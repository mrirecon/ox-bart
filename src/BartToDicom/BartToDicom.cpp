/* Copyright 2017. The Regents of the University of California.
 * Copyright 2011-2017 General Electric Company. All rights reserved.
 * GE Proprietary and Confidential Information. Only to be distributed with
 * permission from GE. Resulting outputs are not for diagnostic purposes.
 *
 * 2016-2017 Jon Tamir <jtamir@eecs.berkeley.edu>
 */

// orchestra includes
#include <MDArray/Utils.h>

#include <Dicom/Core/MR/Image.h>
#include <Dicom/Core/Network.h>

#include <Orchestra/Cartesian2D/KSpaceTransformer.h>
#include <Orchestra/Cartesian3D/ZTransformer.h>

#include <Orchestra/Gradwarp/GradwarpPlugin.h>

#include <Orchestra/Control/ProcessingControl.h>
#include <Orchestra/Control/CommonTypes.h>

#include <Orchestra/Legacy/DicomSeries.h>
#include <Orchestra/Legacy/Pfile.h>
#include <Orchestra/Legacy/PfileReader.h>

#include <Orchestra/Core/SumOfSquares.h>
#include <Orchestra/Core/Clipper.h>
#include <Orchestra/Core/RotateTranspose.h>

#include <Orchestra/Common/ImageCorners.h>
#include <Orchestra/Common/SliceOrientation.h>
#include <Orchestra/Common/SliceCorners.h>
#include <Orchestra/Common/ReconTrace.h>

// bart includes
#include <assert.h>

#include "misc/mri.h"
#include "misc/misc.h"
#include "misc/mmio.h"
#include "misc/debug.h"

#include "num/multind.h"
#include "num/flpmath.h"
#include "num/fft.h"

// project includes
#include "CommandLine.h"
#include "Driver.h"
#include "BartIO.h"



// Include this to avoid having to type fully qualified names
using namespace GERecon;
using namespace MDArray;


/**
 * Write Pfile data to BART-formatted file
 */
void GERecon::BartToDicom()
{
	GERecon::TracePointer trace = GERecon::Trace::Instance();

	// Get DICOM network, series number, and series description (if specified) from the command line.
	// If not specified, the optionals and pointer will be empty and no attempt will be made to insert
	// the values or store the images.
	const GEDicom::NetworkPointer dicomNetwork = CommandLine::DicomNetwork();
	const boost::optional<int> seriesNumber = CommandLine::SeriesNumber();
	const boost::optional<std::string> seriesDescription = CommandLine::SeriesDescription();
	const boost::optional<std::string> fileNamePrefix = CommandLine::FileNamePrefix();

	const long ifft_flags = *CommandLine::IFFT();
	const long fft_flags = *CommandLine::FFT();
	const long fftmod_flags = *CommandLine::FFTMod();

	// Read Pfile from command line
	const boost::filesystem::path pfilePath = CommandLine::PfilePath();
	const Legacy::PfilePointer pfile = Legacy::Pfile::Create(pfilePath, Legacy::Pfile::AllAvailableAcquisitions, AnonymizationPolicy(AnonymizationPolicy::None));

	// get current version of Pfile
	const Legacy::PfileReader pfileReader(pfilePath);
	const float pfileVersion = pfileReader.CurrentRevision();

	// Get input name
	const boost::optional<std::string> InString = CommandLine::BartInput();

	// load image data from BART file
	long dims[DIMS];
	_Complex float* data = load_cfl(InString->c_str(), DIMS, dims);

	// Get channel weights input name
	const boost::optional<std::string> ChannelWeightsString = CommandLine::ChannelWeights();

	long cdims[DIMS];
	_Complex float* weights = NULL;

	if (ChannelWeightsString)
		weights = load_cfl(ChannelWeightsString->c_str(), DIMS, cdims);


	if (0 != fftmod_flags) {

		trace->ConsoleMsg("bart fftmod %d", fftmod_flags);
		fftmod(PFILE_DIMS, dims, fftmod_flags, data, data);
	}

	if (0 != ifft_flags) {

		trace->ConsoleMsg("bart fft -iu %d", ifft_flags);
		ifftuc(PFILE_DIMS, dims, ifft_flags, data, data);
	}

	if (0 != fft_flags) {
		
		trace->ConsoleMsg("bart fft -u %d", fft_flags);
		fftuc(PFILE_DIMS, dims, fft_flags, data, data);
	}


	std::string fileName = "Image";
	if (fileNamePrefix)
		fileName = fileNamePrefix->c_str();

	// write to dicom
	BartIO::BartToDicom(dims, fileName, seriesNumber, seriesDescription, dicomNetwork, data, pfile, pfileVersion, weights);

	if (NULL != weights)
		unmap_cfl(DIMS, cdims, weights);

	unmap_cfl(DIMS, dims, data);
}

