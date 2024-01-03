# Instructions

## Install dependencies

### Install PlatformIO Core

> https://docs.platformio.org/en/latest/core/installation/methods/installer-script.html#local-download-macos-linux-windows

> https://docs.platformio.org/en/latest/core/installation/shell-commands.html#piocore-install-shell-commands

### Install ESP-IDF

> https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/#manual-installation

### Install SDL2

_(If you are on Windows, skip this step)_

```shell
sudo apt-get install libsdl2-dev
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
```

## Run Test

```shell
pio test -e test
```