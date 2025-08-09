# Doom-Style Raycasting Engine

A lightweight C-based raycasting engine inspired by the classic *DOOM*, built with **SDL2** and **SDL2\_image**.

## ✨ Features

* Doom-style 3D rendering
* Map drawing with textured walls
* Basic movement & camera rotation
* Configurable window size
* Proper cleanup on exit (frees textures, destroys renderer/window, calls `SDL_Quit()`)

## 📦 Requirements

Make sure you have:

* **SDL2**
* **SDL2\_image**
* **gcc** (or any C compiler)

**Ubuntu/Debian:**

```bash
sudo apt install libsdl2-dev libsdl2-image-dev
```

**macOS (Homebrew):**

```bash
brew install sdl2 sdl2_image
```
## 🧹 Clean object files

```bash
make clean
```

## 🚀 Build & Run

```bash
make
./game
```

## 🎮 Controls

| Key   | Action                  |
| ----- | ----------------------- |
| W / S | Move forward / backward |
| A / D | Rotate left / right     |
| ESC   | Exit game               |

## 🧹 Cleanup Stage

When you quit the game, it:

* Frees all loaded textures
* Destroys the SDL renderer and window
* Calls `SDL_Quit()` to release resources

## 🛠 Troubleshooting

* **`SDL2/SDL.h: No such file or directory`**
  → Install SDL2 and SDL2\_image, then check include paths. On macOS, you may need:

  ```bash
  make CFLAGS="-I/opt/homebrew/include/SDL2" LDFLAGS="-L/opt/homebrew/lib"
  ```
* **Duplicate symbol linker errors**
  → Ensure shared functions are declared in headers with `extern` and defined in exactly **one** `.c` file.

---
