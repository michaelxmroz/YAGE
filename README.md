# Yet Another GB Emulator

![YAGE running the open-source game Demon3](yage.png? "YAGE running the open-source game Demon3")

YAGE is a classic (DMG) GB emulator. It is written in C++ and consists of a platform-independent backend and a minimal WinAPI & Vulkan frontend for windowing, input & rendering.

## Features
YAGE is a cycle-accurate GB (DMG) emulator. For the exact workings of the original GB hardware that this project is trying to emulate please consult the [Pandocs](https://gbdev.io/pandocs/) as this would go beyond the scope of this readme. In its current state YAGE emulates all GB systems except for the serial data connection.
The [Portaudio](https://portaudio.com/) library is used for plattform-indepentent audio playback and [Dear imgui](https://github.com/ocornut/imgui) for the UI.
Furthermore full game-state serialization is supported for save-states.

## Build
The "Build" folder contains pre-generated VS2022 project files, but any build system can be used in theory.

IMPORTANT: When running the app from Visual Studio, make sure to change the debugging working directory of the YAGEFrontend project to "$(OutDir)" (YAGEFrontend->Right Click->Properties->Debugging->Working Directory). Otherwise the relative path for the main shader file will be incorrect, and the emulator will crash on startup.

The application consists of three projects: front-end (YAGEFrontend), back-end (YAGECore) and unit tests (Tests).
The back-end should be built into a library and then linked by either the front-end or the unit tests.

## Dependencies
The front-end is dependent on WinAPI and the [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/).

The back-end (YAGECore) is pure C++ without any external dependencies. It also exposes a C API as long as _CINTERFACE is set.

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
- GBC
- Multi-platform front-ends
