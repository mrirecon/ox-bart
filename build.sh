#!/bin/bash

set -eu
set -o pipefail

OX_INSTALL_DIRECTORY=${OX_INSTALL_DIRECTORY?="Orchestra SDK directory (OX_INSTALL_DIRECTORY) not set!"}
TOOLBOX_PATH=${TOOLBOX_PATH?="BART directory (TOOLBOX_PATH) not set!"}

export CC=${CC:= gcc-4.9}
export CXX=${CXX:=g++-4.9}

VERBOSE=${VERBOSE:=0}

mkdir -p build
pushd build

cmake -DOX_INSTALL_DIRECTORY=${OX_INSTALL_DIRECTORY} ../src

if [[ ${VERBOSE} -gt "0" ]] ; then
	make -j64 VERBOSE=1
else
	make -j64
fi

popd
