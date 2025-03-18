# Yet Another GB Emulator

<img src="yage.png?" alt="YAGE running the open-source game Demon3" width="400"/>

YAGE is a classic (DMG) GB emulator. The emulator core is written in C++ with no dependencies on std headers, and is platform-independent. The main platform frontend uses WinAPI & Vulkan for windowing, input & rendering. A second frontend for running the emulator baremetal on a RaspberryPI 4 is currently in development using Zig.

## Features
YAGE is a cycle-accurate GB (DMG) emulator. For the exact workings of the original GB hardware that this project is trying to emulate please consult the [Pandocs](https://gbdev.io/pandocs/) as this would go beyond the scope of this readme. In its current state YAGE emulates all GB systems except for the serial data connection.
The [Portaudio](https://portaudio.com/) library is used for plattform-indepentent audio playback and [Dear imgui](https://github.com/ocornut/imgui) for the UI.
Furthermore full game-state serialization is supported for save-states.

## Build
### Windows
The "Build" folder contains pre-generated VS2022 project files, but any build system can be used in theory.

> :warning: When running the app from Visual Studio, make sure to change the debugging working directory of the YAGEFrontend project to "$(OutDir)" (YAGEFrontend->Right Click->Properties->Debugging->Working Directory). Otherwise the relative path for the main shader file will be incorrect, and the emulator will crash on startup.

The application consists of three projects: front-end (YAGEFrontend), back-end (YAGECore) and unit tests (Tests).
The back-end should be built into a library and then linked by either the front-end or the unit tests.


### RaspberryPi 4
Building the RasPi kernel is very straightforward, as it uses the Zig build toolchain. In the "Build" folder the "build_zig.bat" file needs to be run, which will build both the C++ core and the Zig kernel and link them. 



> :warning: The Kernel is currently using Zig Version 0.13.0 Due to the amount of breaking changes between Zig versions I guarantee that it will not compile with any other version of Zig.

Please note that the Zig frontend is still very much a work-in-progress and development is ongoing for properly supporting crucial systems such as I/O, timing and rendering.

## Dependencies
The front-end is dependent on WinAPI and the [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/).

The back-end (YAGECore) is pure C++ without any external dependencies. It exposes a C API for binding to the Zig kernel.

The unit tests require the [Google Test](https://github.com/google/googletest) framework.

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

[Blargg's test roms](https://github.com/retrio/gb-test-roms)
| Tests    | Result |
| -------- | ------- |
| cpu_instr  | ✅ |
| dmg_sound | ✅ (Except wave ram read/writes) |
| instr_timing    |  ✅  |
| mem_timing | ✅ |
| mem_timing-2 | ✅ |
| oam_bug | ❌ |
| halt_bug | ✅ |

## Roadmap
- Implementation of I/O, DMA graphics writing and timing for Zig kernel
- GBC

