/* Copyright 2017. The Regents of the University of California.
 * Copyright 2011-2017 General Electric Company. All rights reserved.
 * GE Proprietary and Confidential Information. Only to be distributed with
 * permission from GE. Resulting outputs are not for diagnostic purposes.
 *
 * 2015 Martin Uecker <martin.uecker@med.uni-goettingen.de>
 * 2016-2017 Jon Tamir <jtamir@eecs.berkeley.edu>
 */

// includes for orchestra

#include <MDArray/Utils.h>

#include <Dicom/MR/Image.h>
#include <Dicom/Network.h>

#include <Orchestra/Cartesian2D/KSpaceTransformer.h>
#include <Orchestra/Cartesian3D/ZTransformer.h>

#include <Orchestra/Gradwarp/GradwarpPlugin.h>

#include <Orchestra/Control/ProcessingControl.h>
#include <Orchestra/Control/CommonTypes.h>

#include <Orchestra/Legacy/DicomSeries.h>
#include <Orchestra/Legacy/Pfile.h>

#include <Orchestra/Core/SumOfSquares.h>
#include <Orchestra/Core/Clipper.h>
#include <Orchestra/Core/RotateTranspose.h>

#include <Orchestra/Common/ImageCorners.h>
#include <Orchestra/Common/SliceOrientation.h>
#include <Orchestra/Common/SliceCorners.h>
#include <Orchestra/Common/ReconTrace.h>

// includes for bart
#include <assert.h>

#include "misc/mri.h"
#include "misc/misc.h"
#include "misc/mmio.h"
#include "misc/debug.h"

#include "num/multind.h"
#include "num/flpmath.h"
#include "num/fft.h"

#include "BartIO.h"


// Include this to avoid having to type fully qualified names
using namespace GERecon;
using namespace MDArray;



/**
 * Copy bart dims from Array dims
 * FIXME: make for generic Array sizes
 */
void BartIO::BartDims(long dims[PFILE_DIMS], const ComplexFloatMatrix& m)
{
	unsigned int rank = m.rank();
	assert(rank <= PFILE_DIMS);

	md_singleton_dims(PFILE_DIMS, dims);

	for (unsigned int i = 0; i < rank; i++)
		dims[i] = m.ubound(i) + 1;
}


/**
 * Tranpose arrays to match bart mri convention, i.e.
 *
 * Input dims:  [Read, Phs1, Phs2, TE, Coil, Phase]
 * Output dims: [Read, Phs1, Phs2, Coil, 1, TE, 1, 1, 1, 1, Phase]
 */
void BartIO::FormatBartMRIDims(long odims[DIMS], const long idims[PFILE_DIMS])
{
	long tdims[DIMS];

	md_set_dims(DIMS, odims, 1);

	// swap COIL_DIM and TE_DIM
	md_transpose_dims(PFILE_DIMS, 3, 4, odims, idims);

	// swap TIME_DIM and TE_DIM
	md_transpose_dims(DIMS, 5, 10, tdims, odims);

	// swap COIL_DIM and MAPS_DIM
	md_transpose_dims(DIMS, 4, 5, odims, tdims);
}

/**
 * Tranpose arrays to match bart mri convention, i.e.
 *
 * Input dims:  [Read, Phs1, Phs2, TE, Coil, Phase]
 * Output dims: [Read, Phs1, Phs2, Coil, 1, TE, 1, 1, 1, 1, Phase]
 */
void BartIO::FormatBartMRI(long odims[DIMS], _Complex float* odata, const long idims[PFILE_DIMS], const _Complex float* idata)
{
	md_set_dims(DIMS, odims, 1);
	md_transpose_dims(PFILE_DIMS, 3, 4, odims, idims);

	md_transpose(PFILE_DIMS, 3, 4, odims, odata, idims, idata, CFL_SIZE); 

	BartIO::FormatBartMRIDims(odims, idims);
}


/**
 * Tranpose arrays to match Orchestra mri convention, i.e.
 *
 * Input dims: [Read, Phs1, Phs2, Coil, 1, TE, 1, 1, 1, 1, Phase]
 * Output dims:  [Read, Phs1, Phs2, TE, Coil, Phase]
 */
void BartIO::FormatOxMRI(long odims[PFILE_DIMS], _Complex float* odata, const long idims[DIMS], const _Complex float* idata)
{
	long tdims[DIMS];

	md_set_dims(DIMS, tdims, 1);
	md_transpose_dims(DIMS, 3, 5, tdims, idims);

	md_transpose(DIMS, 3, 5, tdims, odata, idims, idata, CFL_SIZE); 

	// data are in correct order. Just make odims to match output dims
	md_copy_dims(PFILE_DIMS, odims, tdims);
	odims[4] = tdims[5];
	odims[5] = tdims[10];
}


/*
 * Try to figure out ZIP direction based on raw data energy. No longer needed
 * in DV 26
 */
bool BartIO::get_zip_dir(const long dims[PFILE_DIMS], const Legacy::PfilePointer& pfile)
{
	const long d[1] = { dims[0] * dims[1] };
	const int numSlices = dims[2];

	const ComplexFloatMatrix k0 = pfile->KSpaceData<float>(numSlices / 2, 0, 0, 0);
	const ComplexFloatMatrix k1 = pfile->KSpaceData<float>(numSlices * 3 / 2, 0, 0, 0);

	float v0 = md_zscalar_real(1, d,(const _Complex float*)k0.data(), (const _Complex float*)k0.data());
	float v1 = md_zscalar_real(1, d, (const _Complex float*)k1.data(), (const _Complex float*)k1.data());

	if (0. == v0 && 0. < v1)
		return false;
	else if (0. < v0 && 0. == v1)
		return true;
	else {

		debug_printf(DP_WARN, "Could not auto-detect zip padding. Trying to grab first half...\n");
		return true;
	}
}


/**
 * Extract Pfile data and copy to BART array
 * Assumes nothing about conventions of dimensions.
 * Limited to 6 dimensions because that's what the Pfile contains...
 */
void BartIO::PfileToBart(const long dims[PFILE_DIMS], _Complex float* out, const Legacy::PfilePointer& pfile, const float pfileVersion)
{
	Trace trace("PfileToBart");

	unsigned int N = PFILE_DIMS;

	// FIXME: check compatibility of pfile dimensions
	// FIXME: check phases vs passes

	const int numSlices = dims[2];
	const int numEchoes = dims[3];
	const int numChannels = dims[4];
#if 0
	const int numPhases = dims[5];
#else
	const int numPasses = dims[5];
#endif

	bool zip_on = false;
	bool zip_forward = true;

	const int numZipSlices = pfile->SliceCount();

	if (pfileVersion < 26.) {

		// if ZIP is enabled, we have to figure out what side of the pfile was zero-padded.
		zip_on = numSlices < numZipSlices;
		zip_forward = zip_on ? BartIO::get_zip_dir(dims, pfile) : true;
	}


	// copy into bart format and store in out
#pragma omp parallel for collapse(4)
	for (int currentPass = 0; currentPass < numPasses; currentPass++) {

		for (int currentSlice = 0; currentSlice < numSlices; currentSlice++) {

			for (int currentEcho = 0; currentEcho < numEchoes; currentEcho++) {

				for (int currentChannel = 0; currentChannel < numChannels; currentChannel++) {

					int sl = currentSlice;

					if (!zip_forward)
						sl = numZipSlices - currentSlice - 1;

#if 0
					const ComplexFloatMatrix kSpace = pfile->KSpaceData<float>(sl, currentEcho, currentChannel, currentPhase);
#else
					const ComplexFloatMatrix kSpace = pfile->KSpaceData<float>(Legacy::Pfile::PassSlicePair(currentPass, sl), currentEcho, currentChannel);
#endif

					long dims1[N];
					BartIO::BartDims(dims1, kSpace);
					assert(md_check_compat(N, ~(MD_BIT(0) | MD_BIT(1)), dims, dims1));

					long pos[N];
					md_set_dims(N, pos, 0);
					pos[2] = currentSlice;
					pos[3] = currentEcho;
					pos[4] = currentChannel;
					pos[5] = currentPass;

					md_copy_block(N, pos, dims, out, dims1, kSpace.data(), CFL_SIZE);
				}
			}
		}
	}
}


/*
 * Convert bart array to Ox MDArray::Array
 * Assumes nothing about conventions of dimensions
 */
void BartIO::BartToOx(const long dims[DIMS], Array<std::complex<float>,PFILE_DIMS>& out, const _Complex float* in)
{
#if 0
	std::complex<float>* data = const_cast<std::complex<float>*>(reinterpret_cast<const std::complex<float>*>(in));

	//FIXME: check if columnmajor is necessary?
	out = Array<std::complex<float>,6>(data, shape(dims[READ_DIM], dims[PHS1_DIM], dims[PHS2_DIM], dims[COIL_DIM], dims[TE_DIM], dims[TIME_DIM]), duplicateData);
#else
	md_copy(PFILE_DIMS, dims, out.data(), in, CFL_SIZE);
#endif
}



/*
 * Convert Ox MDArray::Array to bart array
 * Assumes nothing about conventions of dimensions
 */
void BartIO::OxToBart(const long dims[DIMS], _Complex float* out, const Array<std::complex<float>,PFILE_DIMS>& in)
{
	//assert(md_calc_size(DIMS - PFILE_DIMS, dims + PFILE_DIMS) == 1);
	//FIXME: check if columnmajor is necessary?
	md_copy(DIMS, dims, out, in.data(), CFL_SIZE);
}



static int ImageNumber(const int slice, const int echo, const int phase, const Legacy::PfilePointer& pfile)
{
	// Image numbering scheme:
	// P0S0E0, P0S0E1, ... P0S0En, P0S1E0, P0S1E1, ... P0S1En, ... P0SnEn, ...
	// P1S0E0, P1S0E1, ... PnSnEn
	const int slicesPerPhase = pfile->SliceCount() * pfile->EchoCount();
	const int imageNumber = phase * slicesPerPhase + slice * pfile->EchoCount() + echo;

	return imageNumber;
}


/*
 * Apply ZIP and Z-transform
 */
void BartIO::BartZipAndZTransform(const long dims_zip[DIMS], _Complex float* ksp_zip, const long dims[DIMS], const _Complex float* ksp, const bool zip_forward, const Legacy::PfilePointer& pfile)
{
	TracePointer trace = Trace::Instance();

	//debug_print_dims(DP_INFO, DIMS, dims);

	// Create a ProcessingControl from the Legacy Pfile.
	const Control::ProcessingControlPointer processingControl = pfile->CreateOrchestraProcessingControl();

	int numPhases = dims[TIME_DIM];
	int numEchoes = dims[TE_DIM];
	int numChannels = dims[COIL_DIM];

	if (!zip_forward)
		debug_printf(DP_WARN, "Backwards ZIP not yet implemented for BartToDicom!\n");

	trace->ConsoleMsg("Running Z-Transform and Filter");

#pragma omp parallel for collapse(3)
	for (int currentPhase = 0; currentPhase < numPhases; ++currentPhase)
	{

		for (int currentEcho = 0; currentEcho < numEchoes; ++currentEcho)
		{

			for (int currentChannel = 0; currentChannel < numChannels; ++currentChannel)
			{

				Cartesian3D::ZTransformer zTransformer(*processingControl);

				long dims3d[DIMS];
				md_select_dims(DIMS, FFT_FLAGS, dims3d, dims);

				long dims3d_zip[DIMS];
				md_select_dims(DIMS, FFT_FLAGS, dims3d_zip, dims_zip);

				long pos[DIMS];
				md_set_dims(DIMS, pos, 0); 

				ComplexFloatCube acqKSpaceVol(dims[0], dims[1], dims[2]);
				ComplexFloatCube zipKSpaceVol(dims_zip[0], dims_zip[1], dims_zip[2]);

				pos[COIL_DIM] = currentChannel;
				pos[TE_DIM] = currentEcho;
				pos[TIME_DIM] = currentPhase;

				md_copy_block(DIMS, pos, dims3d, acqKSpaceVol.data(), dims, ksp, CFL_SIZE);

				// IFFT in Z direction. Data will be zipped from acquired size.
				zTransformer.Apply(zipKSpaceVol, acqKSpaceVol);

				md_copy_block(DIMS, pos, dims_zip, ksp_zip, dims3d_zip, zipKSpaceVol.data(), CFL_SIZE);
			}
		}
	}
}


/*
 * Write BART ksp file to dicoms using Pfile for auxiliary info
 */
void BartIO::BartToDicom(const long dims[DIMS], const std::string& fileNamePrefix, const boost::optional<int>& seriesNumber, const boost::optional<std::string>& seriesDescription, const GEDicom::NetworkPointer& dicomNetwork, const _Complex float* ksp, const Legacy::PfilePointer& pfile, const float pfileVersion, const _Complex float* channel_weights)
{
	TracePointer trace = Trace::Instance();

	//debug_print_dims(DP_INFO, DIMS, dims);

	// Create a ProcessingControl from the Legacy Pfile.
	const Control::ProcessingControlPointer processingControl = pfile->CreateOrchestraProcessingControl();

	// Pull info directly out of ProcessingControl    
	const int imageXRes = processingControl->Value<int>("ImageXRes");
	const int imageYRes = processingControl->Value<int>("ImageYRes");


	// Create DICOM series to save images into
	const Legacy::DicomSeries dicomSeries(pfile);

	int numAcqSlices = dims[PHS2_DIM];
	int numPhases = dims[TIME_DIM];
	int numEchoes = dims[TE_DIM];
	int numChannels = dims[COIL_DIM];

	FloatVector channelWeights(numChannels);
	

	if (NULL != channel_weights) {

		std::cout << "Using channel weights from command-line" << std::endl;

		for (int currentChannel = 0; currentChannel < numChannels; currentChannel++)
			channelWeights(currentChannel) = __real__ channel_weights[currentChannel];
	}
	else if (pfile->ChannelCount() != numChannels) {

		std::cout << "Using all-ones for channel weights" << std::endl;

		// data is probably coil compressed. let's ignore the Pfile's channel weights
		for (int currentChannel = 0; currentChannel < numChannels; currentChannel++)
			channelWeights(currentChannel) = 1.;
	}
	else {

		std::cout << "Using channel weights from Pfile" << std::endl;
		channelWeights = processingControl->Value<FloatVector>("ChannelWeights");
	}

	int numZipSlices = pfile->SliceCount();

	bool zip_on = false;
	bool zip_forward = true;

	if (pfileVersion < 26.) {

		// if ZIP is enabled, we have to figure out what side of the pfile was zero-padded.
		zip_on = numAcqSlices < numZipSlices;
		zip_forward = zip_on ? BartIO::get_zip_dir(dims, pfile) : true;
	}

	if (!zip_forward)
		debug_printf(DP_WARN, "Backwards ZIP not yet implemented for BartToDicom!\n");

	// check consistency of dimensions
	assert(processingControl->Value<int>("AcquiredXRes") == dims[0]);
	assert(processingControl->Value<int>("AcquiredYRes") == dims[1]);
	assert(processingControl->Value<int>("AcquiredZRes") == numAcqSlices);
	assert(numZipSlices >= numAcqSlices);


	// apply zip in Z direction
	long dims_zip[DIMS];
	md_select_dims(DIMS, ~PHS2_FLAG, dims_zip, dims);
	dims_zip[PHS2_DIM] = numZipSlices;

	trace->ConsoleMsg("Running Z-Transform and Filter");

	// FIXME: wasteful in memory
	_Complex float* ksp_zip = (_Complex float*)md_alloc(DIMS, dims_zip, CFL_SIZE);

	BartIO::BartZipAndZTransform(dims_zip, ksp_zip, dims, ksp, zip_forward, pfile);


	trace->ConsoleMsg("Writing %d slices to Dicom...", numZipSlices);

#pragma omp parallel for collapse(3)
	for(int currentSlice = 0; currentSlice < numZipSlices; ++currentSlice)
	{

		for(int currentPhase = 0; currentPhase < numPhases; ++currentPhase)
		{

			for(int currentEcho = 0; currentEcho < numEchoes; ++currentEcho)
			{

				// initialize these here for OMP parallel threads

				// Storage for transformed image data, thus the image sizes.
				ComplexFloatMatrix imageData(imageXRes, imageYRes);

				Cartesian2D::KSpaceTransformer transformer(*processingControl);

				// Create channel combiner object that will do the channel combining work in channel loop.
				SumOfSquares channelCombiner(channelWeights);

				// Gradwarp Plugin
				GradwarpPlugin gradwarp(*processingControl, TwoDGradwarp, XRMBGradient);   

				long dims0[DIMS];
				md_select_dims(DIMS, READ_FLAG | PHS1_FLAG, dims0, dims_zip);

				long pos[DIMS];
				md_set_dims(DIMS, pos, 0); 

				// Zero out channel combiner buffer for the next set of channels.
				channelCombiner.Reset();

				for(int currentChannel = 0; currentChannel < numChannels; ++currentChannel)
				{
					//trace->ConsoleMsg("Processing Slice[%d of %d], Echo[%d of %d], Channel[%d of %d]", currentSlice+1, numZipSlices, currentEcho+1, numEchoes, currentChannel+1, numChannels);

					// extract single slice of k-space
					ComplexFloatMatrix kSpace0(dims_zip[0], dims_zip[1]);

					pos[PHS2_DIM] = currentSlice;
					pos[COIL_DIM] = currentChannel;
					pos[TE_DIM] = currentEcho;
					pos[TIME_DIM] = currentPhase;

					md_copy_block(DIMS, pos, dims0, kSpace0.data(), dims_zip, ksp_zip, CFL_SIZE);

					// Transform to image space. Data will be zipped from acquired size.
					transformer.Apply(imageData, kSpace0);

					// Accumulate Channel data in channel combiner.
					channelCombiner.Accumulate(imageData, currentChannel);

					//debug_printf(DP_INFO, "slice %d\n", currentSlice);
				}

				ComplexFloatMatrix combinedImage = channelCombiner.GetCombinedImage();

				FloatMatrix magnitudeImage(combinedImage.shape());
				MDArray::ComplexToReal(magnitudeImage, combinedImage, MDArray::MagnitudeData);

				BartIO::OxImageToDicom(magnitudeImage, currentSlice, currentEcho, currentPhase, fileNamePrefix, seriesNumber, seriesDescription, dicomSeries, dicomNetwork, pfile, gradwarp);

			}
		}
	}

	md_free(ksp_zip);

	trace->ConsoleMsg("...done!");
}


/*
 * Write Ox Image dicoms using Pfile for auxiliary info
 */
void BartIO::OxImageToDicom(MDArray::FloatMatrix& magnitudeImage, const int currentSlice, const int currentEcho, const int currentPhase, const std::string& fileNamePrefix, const boost::optional<int>& seriesNumber, const boost::optional<std::string>& seriesDescription, const Legacy::DicomSeries& dicomSeries, const GEDicom::NetworkPointer& dicomNetwork, const Legacy::PfilePointer& pfile, GradwarpPlugin& gradwarp)
{
	// Get information for current slice
	const SliceOrientation& sliceOrientation = pfile->Orientation(currentSlice);
	const SliceCorners& sliceCorners = pfile->Corners(currentSlice);

	gradwarp.Run(magnitudeImage, sliceCorners, currentSlice);

	FloatMatrix rotatedImage = RotateTranspose::Apply<float>(magnitudeImage, sliceOrientation.RotationType(), sliceOrientation.TransposeType());

	Clipper::Apply(rotatedImage, MagnitudeImage);
	ShortMatrix finalImage(rotatedImage.shape());

	finalImage = MDArray::cast<short>(rotatedImage);

	// Create DICOM image
	const int imageNumber = ImageNumber(currentSlice, currentEcho, currentPhase, pfile);
	const ImageCorners imageCorners(sliceCorners, sliceOrientation);
	const GEDicom::MR::ImagePointer dicom = dicomSeries.NewImage(finalImage, imageNumber, imageCorners);

	// Add custom annotation if specified on command line.
	// Note, boost::optional<> types only get inserted if set.
	dicom->Insert<GEDicom::LongString>(0x0008, 0x103E, seriesDescription); // Series description, showed in image browser
	dicom->Insert<GEDicom::LongString>(0x0008, 0x1090, seriesDescription); // Manufacturers model name, showed in annotation
	dicom->Insert<GEDicom::IntegerString>(0x0020, 0x0011, seriesNumber);

	// Save DICOM to file and also store it if network is active
	std::ostringstream strm;
	strm << fileNamePrefix << imageNumber << ".dcm";
	dicom->Save(strm.str());
	dicom->Store(dicomNetwork);
}
