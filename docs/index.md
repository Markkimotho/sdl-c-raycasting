# SDL C Raycasting Engine


## Overview

**SDL C Raycasting** is a lightweight C-based raycasting engine inspired by the classic Wolfenstein 3D, built with SDL2 and SDL2_image. This project demonstrates fundamental 3D graphics programming using classic raycasting techniques.

## Features

- Wolfenstein 3D-style rendering
- Map drawing with textured walls
- Basic movement and camera rotation
- Configurable window size
- Proper resource cleanup (textures, renderer, window)
- SDL2 and SDL2_image integration

## Requirements

- SDL2 development libraries
- SDL2_image development libraries
- GCC or compatible C compiler

### Installation

**Ubuntu/Debian:**

```bash
sudo apt install libsdl2-dev libsdl2-image-dev
```

**macOS (Homebrew):**

```bash
brew install sdl2 sdl2_image
```

## Building & Running

```bash
# Clean build artifacts
make clean

# Build the project
make

# Run the game
./game
```

For macOS with Homebrew SDL2 not in standard paths:

```bash
make CFLAGS="-I/opt/homebrew/include/SDL2" LDFLAGS="-L/opt/homebrew/lib"
```

## Controls

| Key       | Action                  |
| --------- | ----------------------- |
| `W` / `S` | Move forward / backward |
| `A` / `D` | Rotate left / right     |
| `ESC`     | Exit game               |

## Technical Details

- **Language**: C
- **Graphics Library**: SDL2 + SDL2_image
- **Rendering Method**: Raycasting
- **License**: MIT

## Troubleshooting

**SDL.h not found:** Install SDL2 and SDL2_image development packages, verify include paths

**Linker errors:** Ensure shared functions are declared in headers with `extern` and defined in exactly one `.c` file

## Resources

- [SDL2 Documentation](https://wiki.libsdl.org/)
- [Raycasting Algorithm Reference](https://en.wikipedia.org/wiki/Ray_casting)

## Repository

[View on GitHub](https://github.com/Markkimotho/sdl-c-raycasting)
