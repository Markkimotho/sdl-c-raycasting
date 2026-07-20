# Asset Manifest

This manifest records the asset inventory as of 2026-07-21. Dimensions and
channel information were verified from the files in this directory. Runtime
status reflects the source tree on that date; an asset being present in the
repository does not mean that the game currently loads it.

This document is an inventory, not a license grant. The repository's software
license must not be assumed to establish redistribution rights for every
image.

## Provenance and distribution labels

- **Generated for this project:** Generated specifically for this project via
  OpenAI built-in image generation on 2026-07-21, then postprocessed to RGBA.
  No separate asset-license record is included here; distribution must comply
  with the applicable service terms and the project's release policy.
- **Legacy / undocumented:** **not documented—verify before commercial
  redistribution**. Keep, replace, or distribute these files only after their
  source and rights have been established.

## Current runtime assets

These textures are loaded by `src/textures.c` for wall types 1–5.

| Asset | Purpose | Dimensions | Alpha expectation | Provenance | Distribution caveat |
| --- | --- | ---: | --- | --- | --- |
| `colorstone.png` | Wall texture, type 1 | 64×64 | Opaque RGB; alpha not required | Legacy / undocumented | not documented—verify before commercial redistribution |
| `redbrick.png` | Wall texture, type 2 | 64×64 | Opaque RGB; alpha not required | Legacy / undocumented | not documented—verify before commercial redistribution |
| `purplestone.png` | Wall texture, type 3 | 64×64 | Opaque RGB; alpha not required | Legacy / undocumented | not documented—verify before commercial redistribution |
| `wood.png` | Wall texture, type 4 | 64×64 | Opaque RGB; alpha not required | Legacy / undocumented | not documented—verify before commercial redistribution |
| `rocky.png` | Wall texture, type 5 | 128×128 | Opaque RGB; alpha not required | Legacy / undocumented | not documented—verify before commercial redistribution |

The following legacy file is loaded as the extraction texture. Its source is
opaque RGB; the loader converts pure black pixels to transparent pixels before
creating the SDL texture.

| Asset | Purpose | Dimensions | Alpha expectation | Provenance | Distribution caveat |
| --- | --- | ---: | --- | --- | --- |
| `greenlight.png` | Extraction marker billboard, tinted red while locked and green when ready | 64×64 | Source is opaque RGB with a black matte; runtime black color-keying supplies sprite transparency | Legacy / undocumented | not documented—verify before commercial redistribution |

## Generated runtime assets

| Asset | Purpose | Dimensions | Alpha expectation | Source / provenance | Distribution caveat |
| --- | --- | ---: | --- | --- | --- |
| `generated/pov_hands_flashlight.png` | Screen-space first-person view model: POV gloved hands holding a compact, worn metal flashlight | 1254×1254 | Transparent RGBA required; non-opaque alpha confirmed | Generated specifically for this project via OpenAI built-in image generation on 2026-07-21. Prompt summary: POV gloved hands holding compact worn metal flashlight on chroma background. Postprocessed to RGBA. | No separate asset-license record is included here; verify applicable OpenAI service terms and release policy before distribution. |
| `generated/sentinel.png` | World-space billboard sprite: full-body hostile sentinel for the abandoned-bunker setting | 1254×1254 | Transparent RGBA required; non-opaque alpha confirmed | Generated specifically for this project via OpenAI built-in image generation on 2026-07-21. Prompt summary: full-body abandoned-bunker gas-mask sentinel on chroma background. Postprocessed to RGBA. | No separate asset-license record is included here; verify applicable OpenAI service terms and release policy before distribution. |
| `generated/power_cell.png` | World-space collectible billboard that advances the objective and restores flashlight power | 1254×1254 | Transparent RGBA required; non-opaque alpha confirmed | Generated specifically for this project via OpenAI built-in image generation on 2026-07-21. Prompt summary: rugged upright industrial bunker power cell; scratched gunmetal, worn brass, ribbed rubber, and an amber charge window; isolated on a flat green chroma background. Postprocessed locally to RGBA. | No separate asset-license record is included here; verify applicable OpenAI service terms and release policy before distribution. |
| `generated/industrial_pillar.png` | Solid reinforced bunker support-pillar billboard | 1254×1254 | Transparent RGBA required; non-opaque alpha confirmed | Generated specifically for this project via OpenAI built-in image generation on 2026-07-21. Prompt summary: full-height reinforced underground-bunker support column with chipped concrete, corroded steel bands, bolts, dust, and rust accents on a flat green chroma background. Postprocessed locally to RGBA. | No separate asset-license record is included here; verify applicable OpenAI service terms and release policy before distribution. |
| `generated/storage_barrel.png` | Solid waist-high bunker storage-barrel billboard | 1254×1254 | Transparent RGBA required; non-opaque alpha confirmed | Generated specifically for this project via OpenAI built-in image generation on 2026-07-21. Prompt summary: sealed industrial bunker drum with dented gunmetal steel, reinforced bands, bolts, grime, and muted rust accents on a flat green chroma background. Postprocessed locally to RGBA. | No separate asset-license record is included here; verify applicable OpenAI service terms and release policy before distribution. |

## Other legacy assets

These files are not loaded by the current runtime. They remain potential floor
or alternate-wall textures.

| Asset | Intended or likely purpose | Dimensions | Alpha expectation | Provenance | Distribution caveat |
| --- | --- | ---: | --- | --- | --- |
| `grass.png` | Floor texture candidate | 64×64 | Opaque RGB; alpha not required | Legacy / undocumented | not documented—verify before commercial redistribution |
| `wall1.png` | Alternate wall texture | 64×64 | Opaque RGB; alpha not required | Legacy / undocumented | not documented—verify before commercial redistribution |
| `wall2.png` | Alternate wall texture | 64×64 | Opaque RGB; alpha not required | Legacy / undocumented | not documented—verify before commercial redistribution |
| `wall3.png` | Alternate wall texture | 512×512 | Opaque RGB; alpha not required | Legacy / undocumented | not documented—verify before commercial redistribution |
| `barrel.png` | Replaced legacy barrel sprite | 64×64 | Opaque RGB with black matte; no longer loaded | Legacy / undocumented | not documented—verify before commercial redistribution |
| `pillar.png` | Replaced legacy pillar sprite | 64×64 | Opaque RGB with black matte; no longer loaded | Legacy / undocumented | not documented—verify before commercial redistribution |

## Documentation and promotional media

These files support the README or static project page and are not game-runtime
assets.

| Asset | Purpose | Dimensions | Alpha expectation | Provenance | Distribution caveat |
| --- | --- | ---: | --- | --- | --- |
| `Peek.gif` | Animated gameplay preview used by the project page | 1474×806 | Opaque animation; alpha not required | Legacy / undocumented | not documented—verify before commercial redistribution |
| `Peek 2.gif` | Legacy animated gameplay preview used by the static project page | 1277×756 | Opaque animation; alpha not required | Legacy / undocumented | not documented—verify before commercial redistribution |
| `map.png` | Project-page minimap screenshot | 157×161 | Stored as RGBA but fully opaque; alpha not required | Legacy / undocumented | not documented—verify before commercial redistribution |
| `maze.png` | Project-page gameplay screenshot | 1468×816 | Stored as RGBA but fully opaque; alpha not required | Legacy / undocumented | not documented—verify before commercial redistribution |
| `textures_maze.png` | Project-page textured-maze screenshot | 1273×721 | Stored as RGBA but fully opaque; alpha not required | Legacy / undocumented | not documented—verify before commercial redistribution |
| `retro-computer-desk-arrangement.jpg` | Static-page cover or promotional photograph | 3500×2500 | Opaque JPEG; alpha unavailable and not required | Legacy / undocumented | not documented—verify before commercial redistribution |
| `retro-computer-desk-indoors.jpg` | Alternate promotional photograph | 3500×2500 | Opaque JPEG; alpha unavailable and not required | Legacy / undocumented | not documented—verify before commercial redistribution |
| `gameplay-preview.png` | Current Bunker Run gameplay preview captured from the project renderer | 1280×720 | Stored as RGBA screenshot; transparency is not used | Produced by this project on 2026-07-21 from the map-2 screenshot smoke path | Contains the runtime assets listed above; their individual distribution caveats still apply. |
