/* Copyright 2017. The Regents of the University of California.
 * Copyright 2011-2017 General Electric Company. All rights reserved.
 * GE Proprietary and Confidential Information. Only to be distributed with
 * permission from GE. Resulting outputs are not for diagnostic purposes.
 *
 * 2017 Jon Tamir <jtamir@eecs.berkeley.edu>
 */


#include <iostream>
#include <exception>

#include <System/Utilities/ProgramOptions.h>

#include "Driver.h"

extern "C" {
#include "num/init.h"
}

using namespace GERecon;

static void print_usage(const char* arg)
{
	std::cout << "Usage: " << arg << "--input <NoiseStatistics> [--covar <covar>] [--noise <noise>] [--optmat <optmat>]" << std::endl << std::endl;
	std::cout << "Write <NoiseStatistics> h5 data into BART-formatted files." << std::endl;
	std::cout << "Specify one or more outputs." << std::endl;
}

    
/*****************************************************************
 ** Main function that calls the specific recon pipeline to run **
 ******************************************************************/
int main(const int argc, const char* const argv[])
{
    GESystem::ProgramOptions().SetupCommandLine(argc, argv);

    // initialize BART
    num_init();

    try
    {
        NoiseData();

        return 0;
    }
    catch( std::exception& e )
    {
        std::cout << "Runtime Exception! " << e.what() << std::endl;
	print_usage(argv[0]);
    }
    catch( ... )
    {
        std::cout << "Unknown Runtime Exception!" << std::endl;
	print_usage(argv[0]);
    }

    return -1;
}
