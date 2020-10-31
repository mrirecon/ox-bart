/* Copyright 2017. The Regents of the University of California.
 * Copyright 2011-2017 General Electric Company. All rights reserved.
 * GE Proprietary and Confidential Information. Only to be distributed with
 * permission from GE. Resulting outputs are not for diagnostic purposes.
 */

#pragma once

#include <string>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>


namespace GERecon
{
    /**
     * Class that contains utilties for parsing parameters/values/flags from
     * the command line for usage in simple programs. The class requires the
     * GESystem::ProgramOptions to be initialized after main(...):
     * Example:
     * 
     *   int main(const int argc, const char* const argv[])
     *   {
     *       GESystem::ProgramOptions().SetupCommandLine(argc, argv);
     *      
     *       // code...
     *
     *       return 0;
     *   }
     *
     * @author Matt Bingen
     */
    class CommandLine
    {
    public:

        /**
         * Get the ScanArchive path specified on the command line. If it is not set
         * or does not exist, the function will throw an exception.
         *
         * Usage:
         *   --pfile </path/to/pfile>
         */
        static boost::filesystem::path ScanArchivePath();

        /**
         * Output BART file
         *
         * Usage:
         *   --output <file>
         */
        static boost::optional<std::string> Output();

        /**
         * Output Channel weights in BART format
         *
         * Usage:
         *   --coilweights <file>
         */
        static boost::optional<std::string> ChannelWeights();

        /**
         * IFFT flags
         *
         * Usage:
         *   --ifft <flags>
         */
        static boost::optional<long> IFFT();

        /**
         * FFT flags
         *
         * Usage:
         *   --fft <flags>
         */
	static boost::optional<long> FFT();

        /**
         * FFTMod  flags
         *
         * Usage:
         *   --fftmod <flags>
         */
	static boost::optional<long> FFTMod();

    private:

        /**
         * Constructor - do not allow.
         */
        CommandLine();
    };
}
