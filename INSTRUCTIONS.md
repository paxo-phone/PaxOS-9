# Build Instructions

If you are using CLion, please use the [CLion Instructions](#clion-instructions).

## Install dependencies

### Install PlatformIO Core

> https://docs.platformio.org/en/latest/core/installation/methods/installer-script.html#local-download-macos-linux-windows

> https://docs.platformio.org/en/latest/core/installation/shell-commands.html#piocore-install-shell-commands

### Install SDL2

_(If you are on Windows, skip this step)_


```shell
# for linux
sudo apt-get install libsdl2-dev
# for macos using homebrew
brew install sdl2
```

## Clone the repository

```shell
git clone https://github.com/paxo-phone/PaxOS-9.git
```

## Initialize the project

```shell
pio project init
```

## Build

```shell
# Build for ESP32
pio run -e esp32dev

# Build for Windows
pio run -e windows

# Build for Linux
pio run -e linux

# Build for MacOS
pio run -e macos
```

## Run on ESP32

```shell
pio run -t upload -e esp32dev
```

## Run Tests

```shell
pio test -e test
```

# CLion Instructions

## Install dependencies

### Install PlatformIO plugin

* Launch CLion
* Install the ``PlatformIO for CLion`` Plugin by ``JetBrains``
* Restart the IDE

### Install MinGW-w64

Even if CLion comes with a full MinGW installation.\
This is required by PlatformIO, because it uses ``g++`` for compilation.\
You _may_ want to try to add the CLion's MinGW installation to Path, but it's not recommended.

* Go to https://winlibs.com/
* Select the ``Zip archive`` for ``GCC 13.2.0 (with POSIX threads) + LLVM/Clang/LLD/LLDB 17.0.6 + MinGW-w64 11.0.1 (UCRT) - release 4`` for ``Win64``, or [direct download](https://github.com/brechtsanders/winlibs_mingw/releases/download/13.2.0posix-17.0.6-11.0.1-ucrt-r4/winlibs-x86_64-posix-seh-gcc-13.2.0-llvm-17.0.6-mingw-w64ucrt-11.0.1-r4.zip)
* Extract it in ``C:\mingw64`` (So you have the ``bin`` folder at ``C:\mingw64\bin``)
* Add ``C:\mingw64\bin`` to your PATH

## Clone the repository

* On the ``Welcome to CLion`` window, select ``Get from VCS`` (or go to ``File -> New -> Project from Version Control...``)
* Enter the repository URL (``https://github.com/paxo-phone/PaxOS-9.git``)
* Open the project as a ``PlatformIO`` project, not ``CMake`` (this step is very important)
* Wait the IDE to import the project (this can take several minutes)

## Build

Don't forget to re-build the project before running it,\
you can get weird behaviors by not doing so.

* Select the correct target
* Click the ``Build`` button

## Run on ESP32

* Select the ``esp32dev`` target
* Click on the ``Run`` button _(don't forget to build before)_

## Run Tests

* Select the ``test`` target
* Click on the ``Run`` button _(don't forget to build before)_
