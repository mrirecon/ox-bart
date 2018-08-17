/* Copyright 2018. The Regents of the University of California.
 * Copyright 2011-2018 General Electric Company. All rights reserved.
 * GE Proprietary and Confidential Information. Only to be distributed with
 * permission from GE. Resulting outputs are not for diagnostic purposes.
 *
 * 2018 Jon Tamir <jtamir@eecs.berkeley.edu>
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
	std::cout << "Usage: " << arg << "--input <RawCalibration> [--pfile <pfile>] [--body <body_image>]" << std::endl << std::endl;
	std::cout << "Write body coil pre-scan h5 data into BART-formatted files." << std::endl;
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
        CalibrationData();

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
