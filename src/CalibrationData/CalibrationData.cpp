/* Copyright 2018. The Regents of the University of California.
 * Copyright 2011-2018 General Electric Company. All rights reserved.
 * GE Proprietary and Confidential Information. Only to be distributed with
 * permission from GE. Resulting outputs are not for diagnostic purposes.
 *
 * 2018 Jon Tamir <jtamir@eecs.berkeley.edu>
 */

// orchestra includes
#include <Hdf5/File.h>
#include <Orchestra/Legacy/Pfile.h>
#include <Orchestra/Legacy/PfileReader.h>
#include <Orchestra/Legacy/SliceEntry.h>
#include <Orchestra/Legacy/DicomSeries.h>
#include <Orchestra/Gradwarp/GradwarpPlugin.h>
#include <Orchestra/Cartesian2D/LxControlSource.h>
#include <Orchestra/Asset/Calibration.h>
#include <Orchestra/Calibration/Common/RawFile.h>


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
 * Write RawCalibration data to BART-formatted file
 */
void GERecon::CalibrationData()
{
	GERecon::Trace trace("CalibrationData");

	// Read cal data file from command line
	const boost::filesystem::path calDataPath = CommandLine::CalibrationDataPath();
	const GEHdf5::File calDataFile(calDataPath.string(), GEHdf5::File::ReadOnly);

	// Read Pfile from command line
	const boost::filesystem::path pfilePath = CommandLine::PfilePath();
	const Legacy::PfilePointer pfile = Legacy::Pfile::Create(pfilePath, Legacy::Pfile::AllAvailableAcquisitions, AnonymizationPolicy(AnonymizationPolicy::None));
	Control::ProcessingControlPointer processingControl = pfile->CreateOrchestraProcessingControl();

	// Get file output names
	const boost::optional<std::string> bodyCoilVolOutput = CommandLine::BodyCoilVolOutput();

	// Get body coil image and reformat to match imaging field of view
	const int acqXRes = processingControl->Value<int>("AcquiredXRes");
	const int acqYRes = processingControl->Value<int>("AcquiredYRes");
	const int acqZRes = processingControl->Value<int>("AcquiredZRes");
	const int numSlices = pfile->SliceCount();
	const SliceCorners& sliceCornersStart = pfile->Corners(0);
	const SliceCorners& sliceCornersEnd = pfile->Corners(numSlices - 1);
	const float scanCenter = processingControl->Value<float>("ScanCenter");

	GERecon::Calibration::RawFile rawCalibrationFile(*processingControl, GERecon::Calibration::ThreeD);
	rawCalibrationFile.Load(calDataPath, GERecon::CalibrationFile::ForceLoad);
	GERecon::Calibration::CalibrationData calibrationImageData = rawCalibrationFile.ImageSpaceData(GERecon::Calibration::VolumeImageSpace, acqXRes, acqYRes, acqZRes, sliceCornersStart, sliceCornersEnd, scanCenter);

	long body_coil_dims[DIMS];
	md_singleton_dims(DIMS, body_coil_dims);
	body_coil_dims[READ_DIM] = acqXRes;
	body_coil_dims[PHS1_DIM] = acqYRes;
	body_coil_dims[PHS2_DIM] = acqZRes;

	long flat_dims[1] = { md_calc_size(DIMS, body_coil_dims) };

	_Complex float* cal_data = (_Complex float*)create_cfl(bodyCoilVolOutput->c_str(), DIMS, body_coil_dims);

	md_copy(1, flat_dims, cal_data, calibrationImageData.data(), CFL_SIZE);

	unmap_cfl(DIMS, body_coil_dims, cal_data);
}
