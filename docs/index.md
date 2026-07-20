# Bunker Run

**Bunker Run** is a small first-person survival game and SDL2 raycasting
engine written in C. Its mission loop is direct: recover every power cell,
control the bunker sentinels with a finite torch, and reach extraction
alive.

## Mission loop

Each infiltration zone has a fixed 16×16 map and deterministic spawn list:
three power cells, two sentinels, one extraction marker, two barrels, and one
pillar. Starting the same zone always restores the same layout and objectives.

1. Select a zone and deploy.
2. Explore while watching health, stamina, and torch battery power.
3. Interact near all three power cells. Each pickup adds torch charge.
4. Aim the torch at sentinels. Sustained light makes them retreat and
   eventually neutralizes them; without direct light they pursue and cause
   contact damage.
5. Interact with extraction after every cell is collected to win. The marker
   remains locked until the objective is complete.

There are no weapons, doors, or audio systems in the current game. The
torch is the player's defense and resource-management tool.

## Infiltration zones

| Zone | Initial lighting | Character |
| --- | --- | --- |
| Classic Maze | Night, torch starts enabled | Dense mixed-material maze |
| Inner Yard | Day, torch starts disabled | Open arena within an outer walkway |
| The Dungeon | Night, torch starts enabled | Repeating bunker chambers and lanes |

All three use deliberate, non-random positions for props, cells, sentinels,
and extraction.

## Gameplay and presentation

- Textured DDA wall rays with distance fog-like shading and darker wall sides
- Alpha-aware, floor-anchored billboard objects rendered back-to-front
- Wall-depth sprite occlusion using one recorded distance per logical column
- Radius-based collision against walls, solid props, and sentinels
- Sentinel line-of-sight pursuit and torch beam response
- Smooth daylight/twilight/night transitions with deep moonlit darkness
- Angle- and distance-aware torch light, battery drain, recharging, and low-power flicker
- POV gloved hands holding a torch, with movement sway, head-bob, and use motion
- Health, stamina, power, cell objective, crosshair, messages, and toggleable minimap
- Title, active mission, pause, extraction-complete, and game-over states
- Fixed 120 Hz simulation updates with clamped frame time

### Depth-correct sprites

The wall caster stores the perpendicular wall distance for every column of the
1280-pixel logical view. Each object sprite is projected into screen space and
drawn a column at a time only when it is nearer than that stored wall depth.
This keeps props, cells, extraction, and sentinels behind walls when they
should be hidden.

### POV movement

Walking and sprinting drive camera head-bob and the hands' sway. Interacting
adds a short view-model motion, and switching off the torch darkens the
hands. These effects are screen-space overlays and do not interfere with world
depth.

## Controls

### Title screen

| Input | Action |
| --- | --- |
| `Up` / `Left` | Previous zone |
| `Down` / `Right` | Next zone |
| `1`, `2`, `3` | Start that zone immediately |
| `Enter` / `Space` | Start selected zone |
| `Esc` / `Q` | Quit |

### Mission

| Input | Action |
| --- | --- |
| `W` / `S` | Move forward / backward |
| `A` / `D` | Strafe left / right |
| Mouse | Look left / right |
| `Left` / `Right` | Keyboard look |
| `Shift` + forward | Sprint while stamina remains |
| `E` / `Space` | Collect a nearby cell or use extraction |
| `F` | Toggle torch |
| `M` | Toggle minimap |
| `N` | Toggle day/night presentation |
| `R` | Restart zone |
| `Esc` | Pause |

The game automatically pauses and releases relative mouse capture when the
window loses focus. On the pause screen, `Esc` or `Enter` resumes, `R`
restarts, and `Q` returns to title. On extraction-complete or game-over
screens, `R` restarts and `Enter` or `Esc` returns to title.

## Display and runtime resilience

The desktop window starts at 1280×720, is resizable and high-DPI aware, and
has a 640×360 minimum size. SDL scales a stable 1280×720 logical render target
to the window, keeping ray count, HUD placement, and aspect ratio consistent.

The loader first resolves `assets/...` from SDL's executable base path, then
falls back to the current working directory. A packaged build must keep the
`assets/` tree beside the executable. Renderer creation tries these modes in
order:

1. Accelerated with VSync
2. Accelerated without VSync
3. Software rendering

This also allows the smoke test to run with SDL's dummy video driver on a
headless Linux machine.

## Requirements

- C99-compatible compiler
- `make` and `pkg-config`
- SDL2 development libraries
- SDL2_image development libraries with PNG support

Ubuntu/Debian:

```bash
sudo apt install build-essential pkg-config libsdl2-dev libsdl2-image-dev
```

macOS with Homebrew:

```bash
brew install pkg-config sdl2 sdl2_image
```

## Build and test

Build the optimized `./game` executable:

```bash
make release
```

The wrapper below can be invoked from any directory and delegates to the same
release target:

```bash
./build.sh
```

Build `./build/debug/game` with AddressSanitizer and UndefinedBehaviorSanitizer:

```bash
make debug
```

Run the title screen, deploy directly into a zone, or view CLI help:

```bash
./game
./game --map 2
./game --help
```

Run one initialization, update, and render cycle, then exit:

```bash
./game --smoke-test
./build/debug/game --smoke-test
```

Headless Linux smoke test:

```bash
SDL_VIDEODRIVER=dummy ./game --smoke-test
```

Build both configurations and run both headless smoke checks:

```bash
make test
```

To run the same smoke initialization, render the selected/default zone, save
a BMP, and exit, pass `--screenshot FILE.bmp`. Combine it with `--map` to
choose the captured zone:

```bash
./game --map 2 --screenshot capture.bmp
```

Clean all generated build outputs:

```bash
make clean
```

Continuous integration builds both release and sanitizer-enabled debug
targets on pushes and pull requests. Tagged releases package the executable
and runtime asset tree for Linux, macOS, and Windows.

## Generated assets

The current runtime loads these project-specific RGBA files:

- `assets/generated/pov_hands_flashlight.png`
- `assets/generated/sentinel.png`
- `assets/generated/power_cell.png`
- `assets/generated/industrial_pillar.png`
- `assets/generated/storage_barrel.png`

Their dimensions, alpha expectations, generation records, and distribution
caveats are maintained in `assets/ASSET_MANIFEST.md` in the repository.

## License and redistribution

The engine source is MIT-licensed. Image provenance is tracked separately.
Legacy texture, sprite, screenshot, and photograph provenance is **not
documented—verify before commercial redistribution**; the software license
must not be treated as an image license. Consult the asset manifest before
shipping a build.

## Repository

[View the source on GitHub](https://github.com/Markkimotho/sdl-c-raycasting)
