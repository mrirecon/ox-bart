# Ox-Bart conversion tools
Library and converter tools for interop between BART and GE Healthcare's Orchestra.

If you can build bart and you can build the orchestra-sdk examples, you should be able to build these tools.

These tools are intended for research purposes only and come with no warranty. These tools
are NOT FOR DIAGNOSTIC USE. See LICENSE for details.

These tools use the GE Orchestra API. The Orchestra SDK is copyright GE Healthcare

## Requirements
*  Tested with BART v0.3.01 and v0.4.01. Currently does not support CUDA builds (trivial fix)
*  Tested with Ox v1.6
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

## Usage

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
