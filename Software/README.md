# STNODE Software
The software uses `CMAKE` and `arm-none-eabi-gcc` toolchain with the ST SOC `STM32WLxx`

## Structure

- [target](./target/README.md) folder contains STM32WL low level target files
- [lib](./lib/README.md) folder contains SW libraries used by the various applications
- [app](./app/README.md) folder contains SW applicattions


## Requirements

- [CMAKE](https://cmake.org/download/)
- [arm-none-eabi-gcc](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)
- [MinGW](https://osdn.net/projects/mingw/releases/) for `Windows` environments

## Set-up
1. Clone project & checkout `develop` branch
```
$ git clone --branch develop --recurse-submodules https://github.com/TheThingsIndustries/st-node.git
```
2. Set `TOOLCHAIN_PREFIX` in [cross.cmake](./cross.cmake)
3. Install [MinGW](http://mingw.org) for Windows environment and make sure `mingw32-make.exe` can be used from cmd (as a recognized command)

> Note: you can configure TARGET_APP to any [app](./app) you desire.

## Usage (MAC OSX & Linux)

### Debug build commands
```
$ cd st-node/Software/

$ mkdir -p build/debug

$ cd build/debug

$ cmake ../.. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=../../cross.cmake -DTARGET_APP=basic

$ make
```
### Release build commands
```
$ cd st-node/Software/

$ mkdir -p build/release

$ cd build/release

$ cmake ../.. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../../cross.cmake -DTARGET_APP=basic

$ make
```

## Usage (Windows)

### Debug build commands
```
$ cd st-node\Software\

$ mkdir build\debug

$ cd build\debug

$ cmake -G "MinGW Makefiles" ../.. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=../../cross.cmake -DTARGET_APP=basic

$ mingw32-make.exe
```

### Release build commands
```
$ cd st-node\Software\

$ mkdir build\release

$ cd build\release

$ cmake -G "MinGW Makefiles" ../.. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../../cross.cmake -DTARGET_APP=basic

$ mingw32-make.exe
```
