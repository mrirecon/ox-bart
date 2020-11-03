/* Copyright 2017. The Regents of the University of California.
 * Copyright 2011-2017 General Electric Company. All rights reserved.
 * GE Proprietary and Confidential Information. Only to be distributed with
 * permission from GE. Resulting outputs are not for diagnostic purposes.
 */

#pragma once

#include <MDArray/MDArray.h>
#include "misc/mri.h"

#define PFILE_DIMS 6


namespace GEDicom
{
    class Network;
    typedef boost::shared_ptr<Network> NetworkPointer;
}

namespace GERecon
{
	namespace Legacy
	{
		class Pfile;
		typedef boost::shared_ptr<Pfile> PfilePointer;
	}


	namespace BartIO
	{

		/**
		 * Copy bart dims from Array dims
		 * FIXME: make for generic Array sizes
		 */
		void BartDims(long dims[PFILE_DIMS], const MDArray::ComplexFloatMatrix& m);


		/**
		 * Tranpose dimension arrays to match bart mri convention
		 *
		 * @param odims Output dims: [Read, Phs1, Phs2, Coil, 1, TE, 1, 1, 1, 1, Phase]
		 * @param idims Input dims:  [Read, Phs1, Phs2, TE, Coil, Phase]
		 */
		void FormatBartMRIDims(long odims[DIMS], const long idims[PFILE_DIMS]);


		/**
		 * Tranpose data and dimension arrays to match bart mri convention
		 *
		 * @param odims Output dims: [Read, Phs1, Phs2, Coil, 1, TE, 1, 1, 1, 1, Phase]
		 * @param odata output data after transposing
		 * @param idims Input dims:  [Read, Phs1, Phs2, TE, Coil, Phase]
		 * @param idata input data
		 */
		void FormatBartMRI(long odims[DIMS], _Complex float* odata, const long idims[PFILE_DIMS], const _Complex float* idata);


		/**
		 * Tranpose dimension and data arrays to match Orchestra mri convention
		 *
		 * @param odims Output dims:  [Read, Phs1, Phs2, TE, Coil, Phase]
		 * @param odata output data after transposing
		 * @param idims Input dims: [Read, Phs1, Phs2, Coil, 1, TE, 1, 1, 1, 1, Phase]
		 * @param idata input data
		 */
		void FormatOxMRI(long odims[PFILE_DIMS], _Complex float* odata, const long idims[DIMS], const _Complex float* idata);


		/*
		 * Try to figure out ZIP direction based on raw data energy. No longer needed
		 * in DV 26
		 */
		bool get_zip_dir(const long dims[PFILE_DIMS], const Legacy::PfilePointer& pfile);

		/**
		 * Extract ScanArchive data and copy to BART array
		 * Assumes nothing about conventions of dimensions.
		 */
		long ScanArchiveToBart(const long dims[PFILE_DIMS], _Complex float* out, const ScanArchivePointer scanArchive, bool store_sequential);

		/**
		 * Extract Pfile data and copy to BART array
		 * Assumes nothing about conventions of dimensions.
		 * Limited to 6 dimensions because that's what the Pfile contains...
		 */
		void PfileToBart(const long dims[PFILE_DIMS], _Complex float* out, const Legacy::PfilePointer& pfile, const float pfileVersion = 0.);


		/**
		 * Convert BART array to Orchestra array
		 */
		void BartToOx(const long dims[PFILE_DIMS], MDArray::Array<std::complex<float>,PFILE_DIMS>& out, const _Complex float* in);


		/**
		 * Convert Orchestra array to BART array
		 */
		void OxToBart(const long dims[PFILE_DIMS], _Complex float* out, const MDArray::Array<std::complex<float>,PFILE_DIMS>& in);


		/**
		 * Apply ZIP and Z transformer to BART kspace data
		 */
		void BartZipAndZTransform(const long dims_zip[DIMS], _Complex float* ksp_zip, const long dims[DIMS], const _Complex float* ksp, const bool zip_forward, const Legacy::PfilePointer& pfile);


		/**
		 * Write BART file to dicoms using pfile info
		 */
		void BartToDicom(const long dims[DIMS], const std::string& fileNamePrefix, const boost::optional<int>& seriesNumber, const boost::optional<std::string>& seriesDescription, const GEDicom::NetworkPointer& dicomNetwork, const _Complex float* ksp, const Legacy::PfilePointer& pfile, const float pfileVersion = 0., const _Complex float* channel_weights = NULL);


/*
 * Write Ox Image dicoms using Pfile for auxiliary info
 */
		void OxImageToDicom(MDArray::FloatMatrix& magnitudeImage, const int currentSlice, const int currentEcho, const int currentPhase, const std::string& fileNamePrefix, const boost::optional<int>& seriesNumber, const boost::optional<std::string>& seriesDescription, const Legacy::DicomSeries& dicomSeries, const GEDicom::NetworkPointer& dicomNetwork, const Legacy::PfilePointer& pfile, GradwarpPlugin& gradwarp);

	}
}
