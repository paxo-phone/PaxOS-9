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
pio run -e esp32

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

## Troubleshooting

### macOS
- If no program is launching at the end of the build when calling `pio run -e macos`, try to run the executable using `.pio/build/macos/program`.
- If you get an error similar to
  ```
  dyld[xxxxx]: Library not loaded: @rpath/libSDL2-2.0.0.dylib
  Referenced from: <xxxxxx> /Users/username/PaxOS-9/.pio/build/macos/program
  Reason: no LC_RPATH's found
  zsh: abort      .pio/build/macos/program
  ```
  1. Get the location of SDL's dynamic library by executing `brew info sdl2`, you should get a path similar to `/opt/homebrew/Cellar/sdl2/2.28.5`
  2. Run `ls {the path you got from the last command}/lib/libSDL2-2.0.0.dylib`. If you get the exact same path as an output you're ready to go. Otherwise install SDL as described above in this document and retry the procedure.
  3. Re-run the command by adding `DYLD_LIBRARY_PATH="{the path you got from step 1}/lib/"` as a prefix before the command like this `DYLD_LIBRARY_PATH="{the path you got from step 1}/lib/ .pio/build/macos/program`

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
