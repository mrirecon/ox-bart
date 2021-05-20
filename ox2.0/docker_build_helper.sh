#!/usr/bin/env bash

# Set paths to Orchestra SDK and BART inside of docker container
export OX_INSTALL_DIRECTORY=/usr/local/orchestra
export TOOLBOX_PATH=/usr/local/bart
export OPENBLAS_PATH=/opt/OpenBLAS
export LD_LIBRARY_PATH=${OPENBLAS_PATH}/lib:${LD_LIBRARY_PATH}

# update repo index
cat yum.conf > /etc/yum.conf

# build OpenBLAS
pushd /usr/local/openblas
make -j 64
make install
popd

# install required HELiOS packages
yum install -y libpng-devel fftw3-devel lapack-devel

# build BART
pushd ${TOOLBOX_PATH}
rsync -av --progress ox2.0/bart/* .
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
