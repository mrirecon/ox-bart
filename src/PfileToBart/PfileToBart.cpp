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
	GERecon::Trace trace("PfiletoBart");

	const long ifft_flags = *CommandLine::IFFT();
	const long fft_flags = *CommandLine::FFT();
	const long fftmod_flags = *CommandLine::FFTMod();

	// Read Pfile from command line
	const boost::filesystem::path pfilePath = CommandLine::PfilePath();

	const Legacy::PfilePointer pfile = Legacy::Pfile::Create(pfilePath, Legacy::Pfile::AllAvailableAcquisitions, AnonymizationPolicy(AnonymizationPolicy::None));

	// get current version of Pfile
	const Legacy::PfileReader pfileReader(pfilePath);
	const float pfileVersion = pfileReader.CurrentRevision();

#if 0
	const Legacy::LxDownloadDataPointer lp = pfile->DownloadData();
	const Legacy::PrescanHeaderStruct& php = lp->PrescanHeader();
	for (int i = 0; i < 20; i++) {
		std::cout << php.rec_std[i] << std::endl;
		std::cout << php.rec_mean[i] << std::endl;
	}
#endif

	// Get acquired k-space dimensions from processingControl. the pfile dimensions may be zipped
	Control::ProcessingControlPointer processingControl;
	if (pfile->IsZEncoded())
		processingControl = pfile->CreateOrchestraProcessingControl();
	else
		processingControl = pfile->CreateOrchestraProcessingControl<Cartesian2D::LxControlSource>();

	const int acqXRes = processingControl->Value<int>("AcquiredXRes");
	const int acqYRes = processingControl->Value<int>("AcquiredYRes");
	const int acqZRes = processingControl->Value<int>("AcquiredZRes");
	const FloatVector channelWeights = processingControl->Value<FloatVector>("ChannelWeights");

	const int numEchoes = pfile->EchoCount();
	const int numChannels = pfile->ChannelCount();
	const int numPhases = pfile->PhaseCount();
	const int numPasses = pfile->PassCount();

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

#if 1
	// FIXME: rare occasions where data size does not match pfile/proccesing control specified size...
	const ComplexFloatMatrix kSpace = pfile->KSpaceData<float>(Legacy::Pfile::PassSlicePair(0, 0), 0, 0);
	long dims1[DIMS];
	BartIO::BartDims(dims1, kSpace);
	dims[0] = dims1[0];
	dims[1] = dims1[1];
	
	
#else
	dims[0] = acqXRes;
	dims[1] = acqYRes;
#endif

	// FIXME: differentiate between passes and phases
	dims[2] = acqZRes;
	dims[3] = numEchoes;
	dims[4] = numChannels;
	dims[5] = numPhases;

	//debug_print_dims(DP_INFO, PFILE_DIMS, dims);

	// copy into bart-formatted ksp array
	_Complex float* ksp2 = (_Complex float*)md_alloc(PFILE_DIMS, dims, CFL_SIZE);

	BartIO::PfileToBart(dims, ksp2, pfile, pfileVersion);

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
