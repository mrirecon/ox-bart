/* Copyright 2017. The Regents of the University of California.
 * Copyright 2011-2017 General Electric Company. All rights reserved.
 * GE Proprietary and Confidential Information. Only to be distributed with
 * permission from GE. Resulting outputs are not for diagnostic purposes.
 *
 * 2016-2017 Jon Tamir <jtamir@eecs.berkeley.edu>
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
	std::cout << "Usage: " << arg << " [options] --pfile <Pfile> --output <kspace>" << std::endl << std::endl;
	std::cout << "Write <Pfile> data into BART-formatted file <kspace>." << std::endl;
	std::cout << "--fft flags performs an FFT on the data along flags" << std::endl;
	std::cout << "--ifft flags performs an IFFT on the data along flags" << std::endl;
	std::cout << "--fftmod flags performs an FFTMod on the data along flags" << std::endl;
	std::cout << "--weights <file> output channel weights to <file>" << std::endl;
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
        BartWrite();

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
