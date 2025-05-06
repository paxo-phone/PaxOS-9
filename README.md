# PaxOS 9

![PaxOS 9 Logo](https://github.com/paxo-phone/PaxOS-9/assets/45568523/ddb3b517-605c-41b4-8c1e-c8e5d156431b)

[![PlatformIO CI](https://github.com/paxo-phone/PaxOS-9/actions/workflows/platformio-ci.yml/badge.svg)](https://github.com/paxo-phone/PaxOS-9/actions/workflows/platformio-ci.yml)
[![License: AGPL v3](https://img.shields.io/badge/License-AGPL_v3-blue.svg)](https://www.gnu.org/licenses/agpl-3.0)
[![Discord](https://img.shields.io/discord/747816102250643506?label=Discord&logo=discord)](https://discord.com/invite/MpqbWr3pUG)

**PaxOS 9** is the latest iteration of PaxOS, a lightweight, open-source operating system designed primarily for PaxoPhone devices. It's built using the PlatformIO ecosystem, enabling development and deployment across its target hardware and native simulations for desktop environments.

## Table of Contents

- [What is PaxOS 9?](#what-is-paxos-9)
- [Key Features](#key-features)
- [Target Hardware](#target-hardware)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Cloning the Repository](#cloning-the-repository)
- [Building and Running PaxOS 9](#building-and-running-paxos-9)
  - [1. For PaxoPhone Hardware (ESP32 - `paxo-v5`)](#1-for-paxophone-hardware-esp32---paxo-v5)
    - [Build](#build)
    - [Upload](#upload)
    - [Monitor](#monitor)
  - [2. For Windows (Native Simulation)](#2-for-windows-native-simulation)
    - [Build & Run](#build--run)
    - [Build Only](#build-only)
  - [3. For Linux (Native Simulation)](#3-for-linux-native-simulation)
    - [Build](#build-1)
    - [Run](#run)
  - [4. For macOS (Native Simulation)](#4-for-macos-native-simulation)
    - [Build](#build-2)
    - [Run](#run-1)
- [Running Tests](#running-tests)
- [Contributing](#contributing)
- [Resources](#resources)
- [License](#license)
- [Contributors](#contributors)

## What is PaxOS 9?

PaxOS 9 is an operating system tailored for the PaxoPhone, an ESP32-based device. It aims to provide a functional and responsive user experience on resource-constrained hardware. The OS leverages the Arduino framework for its ESP32 target and includes libraries for graphics (LovyanGFX), touch input (FT6236G), and more. For development and testing, PaxOS 9 can also be compiled and run as a native application on Windows, Linux, and macOS, utilizing SDL2 for graphical simulation.

## Key Features

*   **Lightweight:** Designed to run efficiently on ESP32 microcontrollers.
*   **Open Source:** Distributed under the AGPL-3.0 license, encouraging community contributions.
*   **PlatformIO Based:** Simplifies the build process and dependency management across different platforms.
*   **Hardware Abstraction:** Supports specific hardware components like displays and touchscreens.
*   **Native Simulation:** Allows developers to build, run, and test on desktop environments (Windows, Linux, macOS) using SDL2 for UI simulation.

## Target Hardware

The primary hardware target for PaxOS 9 is the **PaxoPhone V5**, which is based on the **ESP-WROVER-KIT** board (an ESP32 module with PSRAM).

## Getting Started

Follow these steps to get PaxOS 9 up and running on your machine or target device.

### Prerequisites

1.  **Git:** To clone the repository.
2.  **PlatformIO Core CLI:** PaxOS 9 uses PlatformIO for building and managing the project. Follow the official installation guide: [https://docs.platformio.org/page/core/installation.html](https://docs.platformio.org/page/core/installation.html)
3.  **For Native Desktop Builds (Windows, Linux, macOS):**
    *   A C++ compiler (GCC, Clang, or MSVC depending on your OS).
    *   **SDL2 Development Libraries:** Required for graphical simulation.
        *   **Linux (Debian/Ubuntu):** `sudo apt-get install libsdl2-dev`
        *   **macOS (using Homebrew):** `brew install sdl2`
        *   **Windows:** The project includes scripts to help manage SDL2 dependencies (see Windows build section). You might need to ensure MinGW (if using GCC) or MSVC build tools are correctly set up.
    *   **Python:** Required for some pre-build scripts, especially for the Windows environment.
4.  **For Linux Native Builds:**
    *   **NCurses Development Libraries:** `sudo apt-get install libncurses-dev` (or equivalent for your distribution).
    *   **Other Libraries:** You may have to install other libraries depending on your system.

### Cloning the Repository

```bash
git clone https://github.com/paxo-phone/PaxOS-9.git
cd PaxOS-9
```

## Building and Running PaxOS 9

PaxOS 9 uses PlatformIO environments to manage different build configurations. Here are the common commands:

*   To build a specific environment: `pio run -e <environment_name>`
*   To build and upload (for hardware targets): `pio run -e <environment_name> -t upload`
*   To clean build files for an environment: `pio run -e <environment_name> -t clean`

### 1. For PaxoPhone Hardware (ESP32 - `paxo-v5`)

This environment targets the ESP-WROVER-KIT board, which is the basis for the PaxoPhone V5.

#### Build

```bash
pio run -e paxo-v5
```

#### Upload

Connect your PaxoPhone/ESP-WROVER-KIT to your computer via USB. PlatformIO will attempt to auto-detect the port.

```bash
pio run -e paxo-v5 -t upload
```
If you have issues with port detection, you can specify it using the `upload_port` option in `platformio.ini` or via the command line.

#### Monitor

To view serial output from the device (e.g., for debugging):

```bash
pio device monitor -e paxo-v5 -b 115200
```
The baud rate is set to `115200`.

### 2. For Windows (Native Simulation)

This allows you to run PaxOS 9 as a desktop application on Windows for development and testing, using SDL2 for the display.

#### Build & Run

The `windows` environment includes scripts to set up the workspace, copy dependencies (like SDL2), and execute the compiled program.

```bash
pio run -e windows
```
This command will compile the project and then attempt to run the resulting executable. The necessary SDL2 DLLs should be copied to the build directory by the `copy_dependencies.py` script.

#### Build Only

If you only want to compile the project without running it immediately:

```bash
pio run -e windows-build-only
```
The executable will typically be found in `.pio/build/windows/program.exe`.

### 3. For Linux (Native Simulation)

This allows you to run PaxOS 9 as a desktop application on Linux, using SDL2.

#### Build

Ensure you have installed the SDL2 and NCurses development libraries (see [Prerequisites](#prerequisites)).

```bash
pio run -e linux
```

#### Run

After a successful build, you can run the executable:

```bash
.pio/build/linux/program
```

### 4. For macOS (Native Simulation)

This allows you to run PaxOS 9 as a desktop application on macOS, using SDL2.

#### Build

Ensure you have installed SDL2 using Homebrew (see [Prerequisites](#prerequisites)). The build flags are configured to look for SDL2 in common Homebrew installation paths (`/usr/local/include/SDL2`, `/opt/homebrew/include/SDL2`).

```bash
pio run -e macos
```

#### Run

After a successful build, you can run the executable:

```bash
.pio/build/macos/program
```

### For all builds, if you run them by yourself, you must be running them in the same directory as the ./storage/ one. (Contain the sd-card files).

## Running Tests

PaxOS 9 uses GoogleTest for its unit tests. These tests are typically run on a native platform.

To build and execute the tests:

```bash
pio test -e test
```
This command will compile the code with the `test` environment configuration and run the defined tests. The `test` environment is configured for native execution and may ignore certain libraries not relevant for unit testing (e.g., graphics).

## Contributing

Contributions are welcome! If you'd like to contribute to PaxOS 9, please follow these general steps:

1.  Fork the repository.
2.  Create a new branch for your feature or bug fix: `git checkout -b feature/your-feature-name` or `git checkout -b fix/your-bug-fix`.
3.  Make your changes and commit them with clear, descriptive messages.
4.  Ensure your code builds successfully for the relevant environments (especially `paxo-v5` and native ones if applicable).
5.  If you add new functionality, consider adding tests.
6.  Push your branch to your fork: `git push origin feature/your-feature-name`.
7.  Open a Pull Request against the `main` branch of the `paxo-phone/PaxOS-9` repository.

Please check if there's a `CONTRIBUTING.md` file for more specific guidelines.

## Resources

*   **Official Website:** [paxo.fr](https://www.paxo.fr)
*   **Discord Server:** Join our community on [Discord](https://discord.com/invite/MpqbWr3pUG) for discussions, help, and updates.

## License

This project is distributed under the **GNU Affero General Public License v3.0 (AGPL-3.0)**. See the `LICENSE` file for more details.

## Contributors

Thanks to all the amazing people who have contributed to PaxOS 9!

<a href="https://github.com/paxo-phone/PaxOS-9/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=paxo-phone/PaxOS-9" />
</a>
