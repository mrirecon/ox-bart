/* Copyright 2017. The Regents of the University of California.
 * Copyright 2011-2017 General Electric Company. All rights reserved.
 * GE Proprietary and Confidential Information. Only to be distributed with
 * permission from GE. Resulting outputs are not for diagnostic purposes.
 *
 * 2016-2017 Jon Tamir <jtamir@eecs.berkeley.edu>
 */

#include <boost/make_shared.hpp>
#include <boost/program_options.hpp>

#include <System/Utilities/ProgramOptions.h>

#include "CommandLine.h"
#include <Orchestra/Common/ReconException.h>

using namespace GERecon;

boost::filesystem::path CommandLine::NoiseDataPath()
{
    // Create option to get Pfile path. Additional options can be registered and loaded here.
    boost::program_options::options_description options;

    options.add_options()
        ("input", boost::program_options::value<std::string>(), "Input noise file to process (hdf5).");

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


// Create option for output covariance matrix file name
boost::optional<std::string> CommandLine::CovarOutput()
{
    boost::program_options::options_description options;

    options.add_options()
        ("covar", boost::program_options::value<std::string>(), "Output covariance matrix (BART format)");

    const GESystem::ProgramOptions programOptions;
    programOptions.AddOptions(options);

    return programOptions.Get<std::string>("covar");
}


// Create option for output noise data file name
boost::optional<std::string> CommandLine::NoiseDataOutput()
{
    boost::program_options::options_description options;

    options.add_options()
        ("noise", boost::program_options::value<std::string>(), "Output noise data matrix (BART format)");

    const GESystem::ProgramOptions programOptions;
    programOptions.AddOptions(options);

    return programOptions.Get<std::string>("noise");
}


// Create option for output noise data file name
boost::optional<std::string> CommandLine::OptimalMatrixOutput()
{
    boost::program_options::options_description options;

    options.add_options()
        ("optmat", boost::program_options::value<std::string>(), "Output optimal transform matrix (BART format)");

    const GESystem::ProgramOptions programOptions;
    programOptions.AddOptions(options);

    return programOptions.Get<std::string>("optmat");
}
