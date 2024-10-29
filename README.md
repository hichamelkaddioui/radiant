# Radiant

DIY light show

## Getting Started

### Prerequisites

- VSCode (or VSCodium)
- PlatformIO

  > If you're using VSCodium, you can install PlatformIO with the [VSCode Extension](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide). You also have to install [CPP tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) to use PlatformIO.

- A RP2040 or Pico board

### Install

1. [Download the code](https://github.com/hichamelkaddioui/radiant)
2. Plug in your board
3. Plug in LED strips
4. Configure LED pins in `main.cpp`
5. Run `platformio run`

## Design

- Sequences of states are defined with keyframes
- Keyframes can define interpolation method (ease, wave, gate, or linear)
