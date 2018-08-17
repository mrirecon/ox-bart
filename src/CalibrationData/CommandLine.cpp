/* Copyright 2018. The Regents of the University of California.
 * Copyright 2011-2018 General Electric Company. All rights reserved.
 * GE Proprietary and Confidential Information. Only to be distributed with
 * permission from GE. Resulting outputs are not for diagnostic purposes.
 *
 * 2018 Jon Tamir <jtamir@eecs.berkeley.edu>
 */

#include <boost/make_shared.hpp>
#include <boost/program_options.hpp>

#include <System/Utilities/ProgramOptions.h>

#include "CommandLine.h"
#include <Orchestra/Common/ReconException.h>

using namespace GERecon;

boost::filesystem::path CommandLine::PfilePath()
{
    // Create option to get Pfile path. Additional options can be registered and loaded here.
    boost::program_options::options_description options;

    options.add_options()
        ("pfile", boost::program_options::value<std::string>(), "Specify pfile to run.");

    const GESystem::ProgramOptions programOptions;
    programOptions.AddOptions(options);

    // Check if the command line has a "--pfile" option
    const boost::optional<std::string> pfileOption = programOptions.Get<std::string>("pfile");

    if(!pfileOption)
    {
        throw GERecon::Exception(__SOURCE__, "No input Pfile specified! Use '--pfile' on command line.");
    }

    // Get path from string option
    const boost::filesystem::path pfilePath = *pfileOption;

    if(!boost::filesystem::exists(pfilePath))
    {
        throw GERecon::Exception(__SOURCE__, "Pfile [%s] doesn't exist!", pfilePath.string());
    }

    return pfilePath;
}

boost::filesystem::path CommandLine::CalibrationDataPath()
{
    // Create option to get Pfile path. Additional options can be registered and loaded here.
    boost::program_options::options_description options;

    options.add_options()
        ("input", boost::program_options::value<std::string>(), "Input calibration file to process (hdf5).");

    const GESystem::ProgramOptions programOptions;
    programOptions.AddOptions(options);

    // Check if the command line has a "--input" option
    const boost::optional<std::string> inputOption = programOptions.Get<std::string>("input");

    if(!inputOption)
    {
        throw GERecon::Exception(__SOURCE__, "No input file specified! Use '--input' on command line.");
    }

    // Get path from string option
    const boost::filesystem::path inputPath = *inputOption;

    if(!boost::filesystem::exists(inputPath))
    {
        throw GERecon::Exception(__SOURCE__, "Input file [%s] doesn't exist!", inputPath.string());
    }

    return inputPath;
}


// Create option for output body coil file name
boost::optional<std::string> CommandLine::BodyCoilVolOutput()
{
    boost::program_options::options_description options;

    options.add_options()
        ("body", boost::program_options::value<std::string>(), "Output body coil volume (BART format)");

    const GESystem::ProgramOptions programOptions;
    programOptions.AddOptions(options);

    // Check if the command line has a "--body" option
    const boost::optional<std::string> outputOption = programOptions.Get<std::string>("body");

    if(!outputOption)
    {
        throw GERecon::Exception(__SOURCE__, "No output body coil BART file specified! Use '--body' on command line.");
    }

    return programOptions.Get<std::string>("body");
}
