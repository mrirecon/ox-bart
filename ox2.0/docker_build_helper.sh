#!/usr/bin/env bash

# Set paths to Orchestra SDK and BART inside of docker container
export OX_INSTALL_DIRECTORY=/usr/local/orchestra
export TOOLBOX_PATH=/usr/local/bart
export OPENBLAS_PATH=/opt/OpenBLAS
export LD_LIBRARY_PATH=${OPENBLAS_PATH}/lib:${LD_LIBRARY_PATH}

# update repo index
cat ox2.0/yum.conf > /etc/yum.conf

# build OpenBLAS
pushd /usr/local/openblas
make -j 64
make install
popd

# install required HELiOS packages
yum install -y libpng-devel fftw3-devel lapack-devel

# build BART
echo cp ox2.0/bart/src/num/fft.c ${TOOLBOX_PATH}/src/num/fft.c
echo cp ox2.0/bart/Makefile.local ${TOOLBOX_PATH}/
echo cp ox2.0/bart/Makefile ${TOOLBOX_PATH}/
cp ox2.0/bart/src/num/fft.c ${TOOLBOX_PATH}/src/num/fft.c
cp ox2.0/bart/Makefile ${TOOLBOX_PATH}/
cp ox2.0/bart/Makefile.local ${TOOLBOX_PATH}/
pushd ${TOOLBOX_PATH}
make allclean
make
popd


# build ox-bart
rm -rf build
mkdir -p build
pushd build
cmake -DOX_INSTALL_DIRECTORY=/usr/local/orchestra ../src
make -j64
popd
