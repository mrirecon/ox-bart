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
         * Get the Pfile path specified on the command line. If it is not set
         * or does not exist, the function will throw an exception.
         *
         * Usage:
         *   --input </path/to/noisePrescan>
         */
        static boost::filesystem::path NoiseDataPath();

        /**
         * Output noise covariance matrix in BART format
         *
         * Usage:
         *   --covar <file>
         */
        static boost::optional<std::string> CovarOutput();


        /**
         * Output noise data in BART format
         *
         * Usage:
         *   --noise <file>
         */
        static boost::optional<std::string> NoiseDataOutput();


        /**
         * Output optimal transform matrix in BART format
         *
         * Usage:
         *   --optmat <file>
         */
        static boost::optional<std::string> OptimalMatrixOutput();

    private:

        /**
         * Constructor - do not allow.
         */
        CommandLine();
    };
}
