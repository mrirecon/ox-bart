/* Copyright 2017. The Regents of the University of California.
 * Copyright 2011-2017 General Electric Company. All rights reserved.
 * GE Proprietary and Confidential Information. Only to be distributed with
 * permission from GE. Resulting outputs are not for diagnostic purposes.
 *
 * 2017 Jon Tamir <jtamir@eecs.berkeley.edu>
 */

// orchestra includes
#include <Hdf5/File.h>
#include <Orchestra/Legacy/Pfile.h>
#include <Orchestra/Legacy/PfileReader.h>
#include <Orchestra/Legacy/SliceEntry.h>
#include <Orchestra/Legacy/DicomSeries.h>
#include <Orchestra/Gradwarp/GradwarpPlugin.h>
#include <Orchestra/Cartesian2D/LxControlSource.h>


// bart includes
#include <assert.h>

#include "misc/mri.h"
#include "misc/misc.h"
#include "misc/mmio.h"
#include "misc/debug.h"

#include "num/multind.h"
#include "num/flpmath.h"

#include "BartIO.h"

// project includes
#include "CommandLine.h"
#include "Driver.h"



// Include this to avoid having to type fully qualified names
using namespace GERecon;
using namespace MDArray;
using namespace GEHdf5;


/**
 * Write Pfile data to BART-formatted file
 */
void GERecon::NoiseData()
{
	GERecon::Trace trace("NoiseData");

	// Read Noise data file from command line
	const boost::filesystem::path noiseDataPath = CommandLine::NoiseDataPath();

	const GEHdf5::File noiseDataFile(noiseDataPath.string(), GEHdf5::File::ReadOnly);

	// Get file output names
	const boost::optional<std::string> covarOutput = CommandLine::CovarOutput();
	const boost::optional<std::string> noiseDataOutput = CommandLine::NoiseDataOutput();
	const boost::optional<std::string> optimalMatrixOutput = CommandLine::OptimalMatrixOutput();

	int output_count = 0;

	if (covarOutput) {

		const GEHdf5::Dims covDims = noiseDataFile.Dimensions("Data", "Covariance");
		ComplexFloatMatrix covData(covDims[0], covDims[1]);
		noiseDataFile.Read(covData, "Data", "Covariance");

		long cov_dims[DIMS];
		md_singleton_dims(DIMS, cov_dims);
		cov_dims[COIL_DIM] = covDims[0];
		cov_dims[MAPS_DIM] = covDims[1];

		long cov_flat_dims[2];
		cov_flat_dims[0] = covDims[0];
		cov_flat_dims[1] = covDims[1];

		_Complex float* cov_data = (_Complex float*)create_cfl(covarOutput->c_str(), DIMS, cov_dims);

		md_transpose(2, 0, 1, cov_flat_dims, cov_data, cov_flat_dims, covData.data(), CFL_SIZE);

		unmap_cfl(DIMS, cov_dims, cov_data);

		output_count++;
	}


	if (noiseDataOutput) {

		const GEHdf5::Dims noiseDims = noiseDataFile.Dimensions("Data", "NoiseData");
		ComplexDoubleMatrix noiseDataD(noiseDims[0], noiseDims[1]);
		noiseDataFile.Read(noiseDataD, "Data", "NoiseData");

		ComplexFloatMatrix noiseData(noiseDims[1], noiseDims[0]);
		noiseData = MDArray::cast<std::complex<float> >(noiseDataD);

		long noise_dims[DIMS];
		md_singleton_dims(DIMS, noise_dims);
		noise_dims[READ_DIM] = noiseDims[1];
		noise_dims[COIL_DIM] = noiseDims[0];

		long noise_flat_dims[2];
		noise_flat_dims[0] = noise_dims[READ_DIM];
		noise_flat_dims[1] = noise_dims[COIL_DIM];

		_Complex float* noise_data = (_Complex float*)create_cfl(noiseDataOutput->c_str(), DIMS, noise_dims);

		md_copy(2, noise_flat_dims, noise_data, (_Complex float*)noiseData.data(), CFL_SIZE);

		unmap_cfl(DIMS, noise_dims, noise_data);

		output_count++;
	}


	if (optimalMatrixOutput) {

		const GEHdf5::Dims optDims = noiseDataFile.Dimensions("Data", "OptimalTransformation");
		ComplexFloatMatrix optData(optDims[0], optDims[1]);
		noiseDataFile.Read(optData, "Data", "OptimalTransformation");

		long opt_dims[DIMS];
		md_singleton_dims(DIMS, opt_dims);
		opt_dims[COIL_DIM] = optDims[0];
		opt_dims[MAPS_DIM] = optDims[1];

		long opt_flat_dims[2];
		opt_flat_dims[0] = optDims[0];
		opt_flat_dims[1] = optDims[1];

		_Complex float* opt_data = (_Complex float*)create_cfl(optimalMatrixOutput->c_str(), DIMS, opt_dims);

		md_transpose(2, 0, 1, opt_flat_dims, opt_data, opt_flat_dims, optData.data(), CFL_SIZE);

		unmap_cfl(DIMS, opt_dims, opt_data);

		output_count++;
	}


	if (0 == output_count) {

		error("No output options specified!\n");

	}
}
