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


// Create option for output file name
boost::optional<std::string> CommandLine::Output()
{
    boost::program_options::options_description options;

    options.add_options()
        ("output", boost::program_options::value<std::string>(), "BART Output file");

    const GESystem::ProgramOptions programOptions;
    programOptions.AddOptions(options);

    // Check if the command line has a "--output" option
    const boost::optional<std::string> outputOption = programOptions.Get<std::string>("output");

    if(!outputOption)
    {
        throw GERecon::Exception(__SOURCE__, "No output BART file specified! Use '--output' on command line.");
    }

    return programOptions.Get<std::string>("output");
}


// Create option for channel weights
boost::optional<std::string> CommandLine::ChannelWeights()
{
    boost::program_options::options_description options;

    options.add_options()
        ("weights", boost::program_options::value<std::string>(), "Output channel weights to BART file.");

    const GESystem::ProgramOptions programOptions;
    programOptions.AddOptions(options);

    return programOptions.Get<std::string>("weights");
}


// Option for performing IFFT along flags
boost::optional<long> CommandLine::IFFT()
{
    boost::program_options::options_description options;

    options.add_options()
        ("ifft", boost::program_options::value<long>()->default_value(0), "Perform IFFT along flags");

    const GESystem::ProgramOptions programOptions;
    programOptions.AddOptions(options);

    return programOptions.Get<long>("ifft");
}


// Option for performing FFT along flags
boost::optional<long> CommandLine::FFT()
{
    boost::program_options::options_description options;

    options.add_options()
        ("fft", boost::program_options::value<long>()->default_value(0), "Perform FFT along flags");

    const GESystem::ProgramOptions programOptions;
    programOptions.AddOptions(options);

    return programOptions.Get<long>("fft");
}


// Option for performing fftmod along flags
boost::optional<long> CommandLine::FFTMod()
{
    boost::program_options::options_description options;

    options.add_options()
        ("fftmod", boost::program_options::value<long>()->default_value(0), "Perform FFTMod along flags");

    const GESystem::ProgramOptions programOptions;
    programOptions.AddOptions(options);

    return programOptions.Get<long>("fftmod");
}
