/* Copyright 2017. The Regents of the University of California.
 * Copyright 2011-2017 General Electric Company. All rights reserved.
 * GE Proprietary and Confidential Information. Only to be distributed with
 * permission from GE. Resulting outputs are not for diagnostic purposes.
 *
 * 2016-2017 Jon Tamir <jtamir@eecs.berkeley.edu>
 */

// orchestra includes
#include <Orchestra/Legacy/Pfile.h>
#include <Orchestra/Legacy/PfileReader.h>
#include <Orchestra/Legacy/SliceEntry.h>
#include <Orchestra/Legacy/DicomSeries.h>
#include <Orchestra/Gradwarp/GradwarpPlugin.h>
#include <Orchestra/Cartesian2D/LxControlSource.h>

#include <Orchestra/Acquisition/ControlPacket.h>
#include <Orchestra/Acquisition/ControlTypes.h>
#include <Orchestra/Acquisition/Core/ArchiveStorage.h>
#include <Orchestra/Acquisition/DataTypes.h>
#include <Orchestra/Acquisition/FrameControl.h>

#include <Orchestra/Common/ScanArchive.h>
#include <Orchestra/Common/ReconPaths.h>
#if 0
#include <Orchestra/Legacy/LegacyForwardDeclarations.h>
#include <op_prescan.h>
#include <Orchestra/Legacy/LxDownloadData.h>
#endif

// bart includes
#include <assert.h>

#include "misc/mri.h"
#include "misc/misc.h"
#include "misc/mmio.h"
#include "misc/debug.h"

#include "num/multind.h"
#include "num/flpmath.h"
#include "num/fft.h"

#include "BartIO.h"

// project includes
#include "CommandLine.h"
#include "Driver.h"



// Include this to avoid having to type fully qualified names
using namespace GERecon;
using namespace MDArray;


/**
 * Write Pfile data to BART-formatted file
 */
void GERecon::BartWrite()
{
	GERecon::Trace trace("ScanArchiveToBart");

	const long ifft_flags = *CommandLine::IFFT();
	const long fft_flags = *CommandLine::FFT();
	const long fftmod_flags = *CommandLine::FFTMod();
	const unsigned int store_sequential = *CommandLine::SequentialStorage();

	// Read Pfile from command line
	const boost::filesystem::path filePath = CommandLine::ScanArchivePath();
	const ScanArchivePointer scanArchive = ScanArchive::Create(filePath, GESystem::Archive::LoadMode);

	const Legacy::ConstLxDownloadDataPointer downloadData = boost::dynamic_pointer_cast<Legacy::LxDownloadData>(scanArchive->LoadDownloadData());
	const boost::shared_ptr<Legacy::LxControlSource> controlSource = boost::make_shared<Legacy::LxControlSource>(downloadData);
	const Control::ProcessingControlPointer processingControl = controlSource->CreateOrchestraProcessingControl();

	const int acqXRes = processingControl->Value<int>("AcquiredXRes");
	const int acqYRes = processingControl->Value<int>("AcquiredYRes");
	const int acqZRes = processingControl->Value<int>("AcquiredZRes");
	const FloatVector channelWeights = processingControl->Value<FloatVector>("ChannelWeights");

	const int numEchoes = processingControl->Value<int>("NumEchoes");
	const int numChannels = processingControl->Value<int>("NumChannels");
	const int numPhases = processingControl->Value<int>("NumPhases");
	const int numPasses = processingControl->Value<int>("NumPasses");

	debug_printf(DP_DEBUG1, "Pfile dims: \n");
	debug_printf(DP_DEBUG1, "Spatial dims\t%03d %03d %03d\n", acqXRes, acqYRes, acqZRes);
	debug_printf(DP_DEBUG1, "numEchoes\t%03d\n", numEchoes);
	debug_printf(DP_DEBUG1, "numChannels\t%03d\n", numChannels);
	debug_printf(DP_DEBUG1, "numPhases\t%03d\n", numPhases);
	debug_printf(DP_DEBUG1, "numPasses\t%03d\n", numPasses);

	// Get output name
	const boost::optional<std::string> OutString = CommandLine::Output();

	// Get weights output name
	const boost::optional<std::string> ChannelWeightsString = CommandLine::ChannelWeights();

	// load kspace data from Pfile
	long dims[PFILE_DIMS];
	md_singleton_dims(PFILE_DIMS, dims);

	//bool store_sequential = true; // FIXME: test sequential writing

	if (store_sequential) {

		std::cout << "Sequential mode. Storing data sequentially in the output" << std::endl;
		dims[0] = acqXRes;
		dims[1] = acqYRes * acqZRes * numEchoes * numPhases;
		dims[4] = numChannels;
	}
	else {

		// FIXME: differentiate between passes and phases
		dims[0] = acqXRes;
		dims[1] = acqYRes;
		dims[2] = acqZRes;
		dims[3] = numEchoes;
		dims[4] = numChannels;
		dims[5] = numPhases;
	}

	debug_print_dims(DP_INFO, PFILE_DIMS, dims);

	// copy into bart-formatted ksp array
	_Complex float* ksp2 = (_Complex float*)md_alloc(PFILE_DIMS, dims, CFL_SIZE);
	_Complex float* ksp3 = NULL;

	long num_views = BartIO::ScanArchiveToBart(dims, ksp2, scanArchive, store_sequential);

	if (store_sequential && num_views < dims[1]) {

		long dims1[PFILE_DIMS];
		md_select_dims(PFILE_DIMS, ~MD_BIT(1), dims1, dims);
		dims1[1] = num_views;
		long pos[PFILE_DIMS];
		md_set_dims(PFILE_DIMS, pos, 0);
		ksp3 = (_Complex float*)md_alloc(PFILE_DIMS, dims, CFL_SIZE);
		md_copy_block(PFILE_DIMS, pos, dims1, ksp3, dims, ksp2, CFL_SIZE);
		md_free(ksp2);
		ksp2 = ksp3;
	}

	if (0 != fftmod_flags) {

		std::cout << "bart fftmod " << fftmod_flags << std::endl;
		fftmod(PFILE_DIMS, dims, fftmod_flags, ksp2, ksp2);
	}

	if (0 != ifft_flags) {

		std::cout << "bart fft -iu " << ifft_flags << std::endl;
		ifftuc(PFILE_DIMS, dims, ifft_flags, ksp2, ksp2);
	}

	if (0 != fft_flags) {
		
		std::cout << "bart fft -u " << fft_flags << std::endl;
		fftuc(PFILE_DIMS, dims, fft_flags, ksp2, ksp2);
	}


	long odims[DIMS];
	BartIO::FormatBartMRIDims(odims, dims);

	_Complex float* ksp = (_Complex float*)create_cfl(OutString->c_str(), DIMS, odims);

	BartIO::FormatBartMRI(odims, ksp, dims, ksp2);

	md_free(ksp2);

	if (ChannelWeightsString) {

		long cdims[DIMS];
		md_select_dims(DIMS, COIL_FLAG, cdims, odims);

		_Complex float* weights = (_Complex float*)create_cfl(ChannelWeightsString->c_str(), DIMS, cdims);

		for (int currentChannel = 0; currentChannel < numChannels; currentChannel++)
			weights[currentChannel] = channelWeights(currentChannel);

		unmap_cfl(DIMS, cdims, weights);
	}


	unmap_cfl(DIMS, odims, ksp);
}
