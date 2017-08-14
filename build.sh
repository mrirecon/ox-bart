#!/bin/bash

set -eu
set -o pipefail

OX_INSTALL_DIRECTORY=${OX_INSTALL_DIRECTORY?="Ox SDK directory not set!"}
TOOLBOX_PATH=${TOOLBOX_PATH?="BART TOOLBOX_PATH not set!"}

VERBOSE=${VERBOSE:=0}

export CC=gcc-4.9
export CXX=g++-4.9

mkdir -p build
pushd build

cmake -DOX_INSTALL_DIRECTORY=${OX_INSTALL_DIRECTORY} ../src

if [[ ${VERBOSE} -gt "0" ]] ; then
	make -j64 VERBOSE=1
else
	make -j64
fi

popd
