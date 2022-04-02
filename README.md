# PrimeApp 

## Introduction

GUI application for camera handling and NTA-related image processing. 

## Usage
### Prerequisites

If you just need the app, you can download it on
the Releases page. To run it you'll need Visual C++ Redistributable installed on your PC.

Additionally, you need to have
Python 3 installed, along with some requirements listed int requirements.txt

## Build
### Prerequisites

This app depends on proprietary PVCam SDK. To build it you need to download and install it from the Photometrics
website. It only supports Windows.

### Building
These build instructions are for a release build and assume you have Cmake installed, have vcpkg in path-to-vcpkg,
and running x64-windows. Adjust accordingly.

```
git clone https://github.com/morgunovmi/PrimeApp.git
cd PrimeApp
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=path-to-vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows ..
cmake --build . --config=Release
../bin/Release/App.exe
```