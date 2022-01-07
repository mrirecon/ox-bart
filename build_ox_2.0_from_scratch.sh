#!/bin/bash

export OX_INSTALL_DIRECTORY=~/tools/orchestra/sdk
BUILD_DIR="ox-bart-build"

mkdir $BUILD_DIR
cd $BUILD_DIR

echo "#################################################################"
echo "setting up dependencies"
git clone git@github.com:mrirecon/ox-bart.git
git clone git@github.com:xianyi/OpenBLAS.git
git clone git@github.com:mrirecon/bart.git

pushd bart
git checkout f071aebf2491491b8a662bfd0cfd951ea63d5bd9
export TOOLBOX_PATH=`pwd`
popd

pushd OpenBLAS
git checkout tags/v0.3.15
export OPENBLAS_PATH=`pwd`
popd
echo "#################################################################"

echo "#################################################################"
echo "TOOLBOX_PATH is $TOOLBOX_PATH"
echo "OPENBLAS_PATH is $OPENBLAS_PATH"
echo "OX_INSTALL_DIRECTORY is $OX_INSTALL_DIRECTORY"
echo "#################################################################"

echo "#################################################################"
echo "building ox-bart"
cd ox-bart
./docker_build.sh
echo "#################################################################"

cd build/BuildOutputs/bin/
ldd ./ScanArchiveToBart
./ScanArchiveToBart
