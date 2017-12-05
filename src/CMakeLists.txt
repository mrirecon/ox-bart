cmake_minimum_required(VERSION 3.6)

project(OxBART)

# Uncomment for optional debugging
#set(CMAKE_VERBOSE_MAKEFILE ON)

# TOOLBOX_PATH should be set to the BART path
message("BART Path:" $ENV{TOOLBOX_PATH})
if(DEFINED ENV{TOOLBOX_PATH})
	message(STATUS "BART TOOLBOX_PATH defined")
	set(TOOLBOX_PATH "$ENV{TOOLBOX_PATH}")
	execute_process(COMMAND ${TOOLBOX_PATH}/bart version
		RESULT_VARIABLE bart_out)
	if(NOT ${bart_out} MATCHES "0")
		message(FATAL_ERROR "BART Execution Error: ${bart_out}")
	endif()
else()
	message(FATAL_ERROR "BART TOOLBOX_PATH environment variable undefined!")
endif()

# TOPDIR is the root of the source tree
set(OX_INSTALL_DIRECTORY "" CACHE PATH "Install directory of the GE Orchestra SDK")
set(TOPDIR "${OX_INSTALL_DIRECTORY}/include")
set(LARGE_LIBRARY_TYPE STATIC)

message("Configuring GE SDK Recon Build")
set(PRODUCT EXTERNAL_SDK)

# Include recon libraries exported from the SDK CMake build
set(RECON_LIBRARIES_INCLUDE_FILE ${OX_INSTALL_DIRECTORY}/lib/ReconLibraries.cmake)
if(EXISTS ${RECON_LIBRARIES_INCLUDE_FILE})
    include (${RECON_LIBRARIES_INCLUDE_FILE})
else()
    message("Could not find ${RECON_LIBRARIES_INCLUDE_FILE}")
    message(FATAL_ERROR "Verify that the CMake OX_INSTALL_DIRECTORY option is set correctly")
endif()

# Include SDK build configuration
include (${TOPDIR}/recon/SDK/product.cmake)

# Include CMakeLists.txt for each rehearsal project
add_subdirectory (BartIO)
add_subdirectory (PfileToBart)
add_subdirectory (BartToDicom)
add_subdirectory (NoiseCov)
