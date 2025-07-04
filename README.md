# Yet Another GB Emulator

[![Build Windows](https://github.com/michaelxmroz/YAGE/actions/workflows/ci.yml/badge.svg)](https://github.com/michaelxmroz/YAGE/actions/workflows/ci.yml)
[![Build Zig Kernel](https://github.com/michaelxmroz/YAGE/actions/workflows/zig-build.yml/badge.svg)](https://github.com/michaelxmroz/YAGE/actions/workflows/zig-build.yml)

<img src="yage.png?" alt="YAGE running the open-source game Demon3" width="400"/>

YAGE is a classic (DMG) GB emulator. The emulator core is written in C++ with no dependencies on std headers, and is platform-independent. The main platform frontend uses WinAPI & Vulkan for windowing, input & rendering. A second frontend for running the emulator baremetal on a RaspberryPI 4 is currently in development using Zig.

## Features
YAGE is a cycle-accurate GB (DMG) emulator. For the exact workings of the original GB hardware that this project is trying to emulate please consult the [Pandocs](https://gbdev.io/pandocs/) as this would go beyond the scope of this readme. In its current state YAGE emulates all GB systems except for the serial data connection.
Furthermore full game-state serialization is supported for save-states.

## Debugger

<img src="debugger.png?" alt="YAGE debugger" width="600"/>

YAGE includes a comprehensive built-in debugger that provides powerful tools for analyzing and debugging ROMs. The debugger is available in debug builds and offers a modern, intuitive interface.

### Key Features

- **Run/Stop/Step**: Step through the emulation execution on a mCycle or tCycle level.
- **Breakpoints**: Break when a specified condition is reached. The debugger supports program counter addresses, memory address writes, opcodes or opcode execution counts as conditions.
- **CPU Inspector**: Real-time display of all CPU registers
- **Instruction Disassembly**: View disassembled instructions around the current PC
- **Memory Viewer**: Interactive hex dump of the entire Game Boy memory space, including color coding of memory regions
- **PPU Inspector**: Real-time view of current PPU mode (HBlank, VBlank, OAM Scan, Drawing) and registers, including interal states such as mode timings and pixel FIFO pipelines.

## Build
### Windows
The "Build/visualstudio" folder contains pre-generated VS2022 project files. For the dependencies to be properly fetched, vcpkg is required:
```
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg integrate install
```
Recent versions of VS2022 come with vcpkg pre-installed. In that case only integrate install needs to be run, if not done previously.
```
./vcpkg integrate install
```
#### Debugging
> :warning: When running the app from Visual Studio, make sure to change the debugging working directory of the YAGEFrontend project to "$(OutDir)" (YAGEFrontend->Right Click->Properties->Debugging->Working Directory). Otherwise the relative path for the main shader file will be incorrect, and the emulator will crash on startup.

The application consists of three projects: front-end (YAGEFrontend), back-end (YAGECore) and unit tests (Tests).
The back-end should be built into a library and then linked by either the front-end or the unit tests.


### RaspberryPi 4
Building the RasPi kernel is very straightforward, as it uses the Zig build toolchain. In the "Build/zig/" folder the "build_zig.bat" file needs to be run, which will build both the C++ core and the Zig kernel and link them. 



> :warning: The Kernel is currently using Zig Version 0.14.0. Due to the amount of breaking changes between Zig versions I can almost guarantee that it will not compile with any other version of Zig.

Please note that the Zig frontend is still very much a work-in-progress and development is ongoing for properly supporting crucial systems such as I/O, timing and rendering.

## Dependencies
The Windows front-end depends on
- [Dear imgui](https://github.com/ocornut/imgui), [Vulkan Headers](https://github.com/KhronosGroup/Vulkan-Headers) and [Volk](https://github.com/zeux/volk/), which are included through submodules and
- [Shaderc](https://github.com/google/shaderc) and [Portaudio](https://portaudio.com/), which are automatically pulled through VS2022's vcpkg integration.

The RasPi 4 kernel depends on [Zig](https://ziglang.org/download/).

The back-end (YAGECore) is pure C++ without any external dependencies. It exposes a C API for binding to the Zig kernel.

The unit tests require the [Google Test](https://github.com/google/googletest) framework, which is automatically pulled through NuGet.

## Usage

All operations can be performed from the main bar UI, but the executable can also be started with the -file "x" command line parameter, where x is a valid path to a *.rom file. A second supported parameter is -bootrom "x" to provide a [bootrom](https://gbdev.gg8.se/wiki/articles/Gameboy_Bootstrap_ROM) to the emulator, but it is not strictly necessary.

## Controls
Currently the default control scheme is as follows, but this can be remapped through the options menu.

- w, a, s, d - Arrow keys
- n, m - A, B
- j, k - Start, Select
- p - Pause
- 1 - Save Gamestate
- 2 - Load Gamestate
- T - Turbo

## Accuracy

YAGE aims to be a highly-accurate GB emulator. For a breakdown of automated test results, and the test suites it is being run against see [the wiki](https://github.com/michaelxmroz/YAGE/wiki/Accuracy).

## Roadmap
- Implementation of I/O, DMA graphics writing and timing for Zig kernel
- GBC

