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

### Install libcurl

_(If you are on Windows, skip this step)_


```shell
# for linux
sudo apt-get install libcurl4-openssl-dev
# for macos using homebrew
brew install curl
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

## Troubleshooting

### macOS
- If you get a popup saying that the program is from an unidentified developer, do `xattr -d com.apple.quarantine program` in the same directory as the `program`.
- If no program is launching at the end of the build when calling `pio run -e macos`, try to run the executable using `.pio/build/macos/program`.
- If you get an error similar to
  ```
  dyld[xxxxx]: Library not loaded: @rpath/libSDL2-2.0.0.dylib
  Referenced from: <xxxxxx> /Users/username/PaxOS-9/.pio/build/macos/program
  Reason: no LC_RPATH's found
  zsh: abort      .pio/build/macos/program
  ```
  Try to do ```DYLD_LIBRARY_PATH="`brew --prefix sdl2`/lib" .pio/build/macos/program``` and if it still doesn't work and you haven't installed SDL2 locally as decribed in the next bullet point (otherwise run ```DYLD_LIBRARY_PATH="`eval echo ~$USER`/sdl2/lib" .pio/build/macos/program```) do:
  1. Get the location of SDL's dynamic library by executing `brew info sdl2`, you should get a path similar to `/opt/homebrew/Cellar/sdl2/2.28.5`
  2. Run `ls {the path you got from the last command}/lib/libSDL2-2.0.0.dylib`. If you get the exact same path as an output you're ready to go. Otherwise install SDL as described above in this document and retry the procedure.
  3. Re-run the command by adding `DYLD_LIBRARY_PATH="{the path you got from step 1}/lib/"` as a prefix before the command like this `DYLD_LIBRARY_PATH="{the path you got from step 1}/lib/ .pio/build/macos/program`
- When compiling the project using pio you might encounter an error where your shell can't find `pio` even if you installed it via the quick-setup. To fix that, execute `pio` directly from the binaries path that is located at `/Users/username/.platformio/penv/bin/pio`. If you have PaxOS9 installed directly in your home (i.e. `/Users/username/PaxOS9`), run pio via `../.platformio/penv/bin/pio`. To build for macOS it would give something similar this `../.platformio/penv/bin/pio run -e macos`.
- If you get the `unknown platform` error when compiling, that probalby means that SDL2 is not installed where it should or even just not installed at all. If you have admin permissions for your machine run `brew install sdl2`. If you don't, then install SDL2 locally by following those steps:
  1. Execute the following command (don't worry it won't hack into your computer)
     ```
     cd `eval echo ~$USER` && rm -rf sdl2-build && rm -rf sdl2 && mkdir sdl2 && git clone https://github.com/libsdl-org/SDL.git -b SDL2 sdl2-build && cd sdl2-build && mkdir build && cd build && ../configure --prefix=`eval echo ~$USER`/sdl2 && make -j`sysctl -n hw.ncpu` && make install && cd `eval echo ~$USER` && rm -rf sdl2-build && echo "Installed SDL at `eval echo ~$USER`/sdl2" && echo "\033[0;32mThe two lines to add to the macos target (\033[1;34mbuild_flags\033[0;32m) of the \033[1;34mplatform.ini\033[0;32m in your PaxOS9 directory are \033[1;33m-I`eval echo ~$USER`/sdl2/include\033[0;32m and \033[1;33m-L`eval echo ~$USER`/sdl2/lib\033[0;32m"
     ```
  2. Follow the instructions that the command gave you (add the two lines). The place where you should put the lines looks like this (the filename is called `platform.ini` and is present in the `PaxOS9` directory):
     ```
     [env:macos]
      platform = native
      lib_deps = 
      	x
      	x
      	x
      test_framework = googletest
      build_flags = 
      	x
      	x
      	x
      	x
      	-I/opt/homebrew/include/SDL2
      	-L/opt/homebrew/lib
      	-->PLACE THE LINES SEPARATED BY A NEW LINE HERE <--
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
