#!/bin/bash
# Get paths to orchestra SDK and BART
OX_INSTALL_DIRECTORY=${OX_INSTALL_DIRECTORY?="Orchestra SDK directory (OX_INSTALL_DIRECTORY) not set!"}
TOOLBOX_PATH=${TOOLBOX_PATH?="BART directory (TOOLBOX_PATH) not set!"}
OPENBLAS_PATH=${OPENBLAS_PATH?="OpenBLAS source (OPENBLAS_PATH) not set!"}

# Get name of current directory
CUR_DIR=$(cd $(dirname $0); pwd -P)
CONTAINER=cpp-sdk

# Load SDK docker if the image does not exists
INSPECT="$(docker inspect --format='{{.Config.Image}}' $CONTAINER)"
if [[ "$INSPECT" != sha* ]]
then
    docker load -i $OX_INSTALL_DIRECTORY/cpp-sdk.tar.gz
fi

# Run the cpp-sdk docker container
# "scl enable devtoolset-6" changes the default GNU compiler to gcc6 inside the docker
docker run --rm -it \
       -v ${CUR_DIR}:/home/sdkuser \
       -v ${OX_INSTALL_DIRECTORY}:/usr/local/orchestra \
       -v ${TOOLBOX_PATH}:/usr/local/bart \
       -v ${OPENBLAS_PATH}:/usr/local/openblas \
       -w /home/sdkuser \
       -p 8889:8889 \
       -u 0 \
       $CONTAINER scl enable devtoolset-6 /home/sdkuser/ox2.0/docker_build_helper.sh
