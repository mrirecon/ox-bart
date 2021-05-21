# Ox-Bart conversion tools
Library and converter tools for interop between BART and GE Healthcare's Orchestra.

If you can build bart and you can build the orchestra-sdk examples, you should be able to build these tools.

These tools are intended for research purposes only and come with no warranty. These tools
are NOT FOR DIAGNOSTIC USE. See LICENSE for details.

These tools use the GE Orchestra API. The Orchestra SDK is copyright GE Healthcare.

# Ox v.2.01 Requirements
*  Tested with BART v0.7.00 (commit f071aebf2491491b8a662bfd0cfd951ea63d5bd9). Requires some light modification of Makefile (included)
*  Tested with OpenBLAS-0.3.15. Requires building from source within Docker
*  Requires docker

## Building
Starting in Orchestra 2.0.x, we need to build ox-bart within the supplied Docker container.
This is a little tricky because it requires installing BART on HELiOS 6.10, which is quite old.
Therefore, we need to build OpenBLAS from source and change some Makefile paths for Docker. We also have to
update the RedHat repo index to point to GE's open HELiOS repo: http://linux.gehealthcare.com/HELiOS6/HELiOS-6.10/os/x86_64/Packages/.

## Instructions

__Note__: this will overwrite some files in the BART directory. It is recommended to use a clean version of BART and set the TOOLBOX_PATH appropriately

1. Download the Orchestra Docker file and place in Orchestra sdk. It should have a name similar to `cpp-sdk`.
1. Download OpenBLAS source from the website
1. Download and install Docker. Follow the website for all setup instructions
1. Set `TOOLBOX_PATH` to the top-level BART directory, e.g.
1. Set `OX_INSTALL_DIRECTORY` to the top-level Orchestra sdk directory, e.g.
1. Set `OPENBLAS_PATH` to the OpenBLAS source code directory e.g.  
Example: 
```bash
export TOOLBOX_PATH=/home/jtamir/bart
export OX_INSTALL_DIRECTORY=/home/jtamir/orchestra-sdk
export OPENBLAS_PATH=/home/jtamir/OpenBLAS-0.3.15
```

Build the tools with
```bash
./docker_build.sh
```

### Example

The following commands were executed to install from scratch on an Ubuntu 20.04.2 LTS x86_64 install with Linux kernel: 5.8.0-50-generic.

#### Install docker.

We will use ``snap``, but feel free to use your preferred method of installing docker.

```bash
sudo snap install docker
sudo groupadd docker
sudo usermod -aG docker $USER
newgrp docker
sudo chmod 666 /var/run/docker.sock
```

#### Clone required repositories.

We will create a workspace folder in the home directory. Feel free to change the destination directory as desired.

```bash
mkdir ~/Orchestra
cd ~/Orchestra
```

Save ``orchestra-sdk-2.0-1`` to ``~/Orchestra``. ``ls ~/Orchestra/orchestra-sdk-2.0-1`` should list something like:
```
3p  Examples  include  lib  license  build.sh  cpp-sdk.tar.gz  openDockerContainer.sh
```

Clone the repositories.
```bash
cd ~/Orchestra
git clone git@github.com:mrirecon/ox-bart.git
git clone git@github.com:xianyi/OpenBLAS.git
git clone git@github.com:mrirecon/bart.git
sudo chown -R $USER .
```

The ``sudo`` command above may not be needed. Then,

```bash
cd OpenBLAS
git checkout tags/v0.3.15
cd ~/Orchestra
```

Before installing, set the required enviorment variables.

```bash
export TOOLBOX_PATH=~/Orchestra/bart
export OX_INSTALL_DIRECTORY=~/Orchestra/orchestra-sdk-2.0-1
export OPENBLAS_PATH=~/Orchestra/OpenBLAS
```

Finally,

```bash
cd ox-bart
./docker_build.sh
```

You should find binary files located in ``~/Orchestra/ox-bart/bin/``.

# Ox v1.10 Requirements
*  Tested with BART v0.6.00 (e1a4303). Currently does not support CUDA builds (trivial fix)
*  Tested with Ox v1.10 (older versions not currently supported)
*  Requires cmake 3.6 or higher

## Building
Must have BART installed without CUDA

Set `TOOLBOX_PATH` to the top-level BART directory, e.g.
```bash
export TOOLBOX_PATH=/home/jtamir/bart
```

Set `OX_INSTALL_DIRECTORY` to the top-level Orchestra sdk directory, e.g.
```bash
export OX_INSTALL_DIRECTORY=/home/jtamir/orchestra-sdk

```
Build the tools with
```bash
./build.sh
```

The tools and libraries can be found under `bin/` and `lib/`

# Usage

### `PfileToBart`
Use to convert a Pfile to a BART cfl/hdr file.
```bash
Usage: PfileToBart [options] --pfile <Pfile> --output <kspace>

Write <Pfile> data into BART-formatted file <kspace>.
--fft flags performs an FFT on the data along flags
--ifft flags performs an IFFT on the data along flags
--fftmod flags performs an FFTMod on the data along flags
--weights <file> output channel weights to <file>
```

### `ScanArchiveToBart`
Use to convert a ScanArchive to a BART cfl/hdr file.
```bash
Usage: ScanArchiveToBart [options] --file <ScanArchive> --output <kspace>

Write <ScanArchive> data into BART-formatted file <kspace>.
--fft flags performs an FFT on the data along flags
--ifft flags performs an IFFT on the data along flags
--fftmod flags performs an FFTMod on the data along flags
--weights <file> output channel weights to <file>
```

### `BartToDicom`
Takes a Pfile and a bart kspace file, and generates Dicoms using the Ox Dicom chain,
e.g. gradwarp, resizing, etc.

```bash
Usage: BartToDicom [options] --pfile <Pfile> --input <kspace>

Write BART-formatted data in <kspace> to dicom using Pfile info.
--fft flags performs an FFT on the data along flags
--ifft flags performs an IFFT on the data along flags
--fftmod flags performs an FFTMod on the data along flags
--weights <weights> inputs custom channel weights
```


### `NoiseCov`
```bash
Usage: NoiseCov --input <NoiseStatistics> --covar <covar> --noise <noise> --optmat <optmat>

Write noise data and covariance matrix files to BART-formatted data
```


### `CalibrationData`
```bash
Usage: CalibrationData --pfile <Pfile> --input <RawCalibration> --body <body_coil> 

Write body coil pre-scan h5 data into BART-formatted file based on dimensions in <Pfile>
```
