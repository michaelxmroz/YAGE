# Yet another GameBoy Emulator

YAGE is a classic (DMG) GameBoy emulator. It is written in C++ and consists of a platform-independent backend and a minimal WinAPI & Vulkan frontend for windowing, input & rendering.

## Features
YAGE aims to be a medium-accuracy emulator. For the exact workings of the original GameBoy hardware that this project is trying to emulate please consult the [Pandocs](https://gbdev.io/pandocs/) as this would go beyond the scope of this readme. In it's current state YAGE emulates all GameBoy systems except for the APU (audio) and serial data connection.

Furthermore full game-state serialization is supported for save-states.

## Build
The "Build" folder contains pre-generated VS2019 project files, but any build system can be used in theory.
The application consists of three projects: front-end (GameBoy), back-end (GameBoyCore) and unit tests (Tests).
The back-end should be built into a library and then linked by either the front-end or the unit tests.

## Dependencies
The front-end is dependent on WinAPI and the [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/).

The back-end (GameBoyCore) is pure C++ without any external dependencies.

The unit tests require the [Google Test](https://github.com/google/googletest) framework.

## Usage

The executable should be started with the -file "x" command line parameter, where x is a valid path to a *.rom file. A second supported parameter is -bootrom "x" to provide a [bootrom](https://gbdev.gg8.se/wiki/articles/Gameboy_Bootstrap_ROM) to the emulator, but it is not strictly necessary.

## Controls
Currently the controls are hard-coded.

- w, a, s, d - Arrow keys
- n, m - A, B
- k, l - Start, Select
- p - Pause
- 1 - Save Gamestate
- 2 - Load Gamestate

## Todo's
- Audio Emulation
- GUI
- Multi-platform front-ends
