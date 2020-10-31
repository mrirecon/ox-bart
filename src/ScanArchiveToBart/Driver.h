/* Copyright 2017. The Regents of the University of California.
 * Copyright 2011-2017 General Electric Company. All rights reserved.
 * GE Proprietary and Confidential Information. Only to be distributed with
 * permission from GE. Resulting outputs are not for diagnostic purposes.
 */

#pragma once

#include <string>
#include <sstream>

#include <boost/shared_ptr.hpp>

/**
 * This header defines a list of functions that act as simple
 * recon pipelines (rehearsals) that can be called from the main
 * method in the corresponding source .cpp file.
 *
 * Define any new pipelines here and implement in a new
 * .cpp file. A typical use case would be to copy one
 * of the existing pipelines and modify it for development.
 *
 * The file contains a few helper functions useful for basic
 * pipeline creation and control.
 *
 * Also, note that everything is nested in the GERecon namespace.
 * This is convention that is seen throughout all Orchestra
 * code. Namespaces allow for components/classes to be scoped
 * appropriately. If it lives in Orchestra, it's probably nested
 * somewhere in the GERecon namespace. Example: GERecon::Cartesian2D
 *
 * @author Matt Bingen
 */
namespace GERecon
{
    /**
     * Write a ScanArchive to BART formatted file
     */
    void BartWrite();
}
