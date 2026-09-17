# Changelog

All notable changes to Gality Engine will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]https://github.com/moli0516/Gality/compare/v0.3.0...HEAD

### Planned

- CG System (gallery, viewer, unlock tracking)
- Character layered parts (blink, mouth, expression controllers)
- AI toolchain integration (translation, TTS, image generation)
- WebAssembly build (pending SFML Emscripten support)
- Cross-platform CI/CD pipeline
- itch.io release of demo story
- Steam release preparation

## [0.3.0]https://github.com/moli0516/Gality/compare/v0.2.0...v0.3.0 - 2026-09-17

### Added — Text Effects System

- **FontManager** singleton with dynamic font loading and graceful fallback
- **Text shadow** — configurable offset, color, and blending
- **Text outline** — 8-direction copy method (optional, disabled by default)
- **Rich inline tags**:
  - `<wave>` — vertical sine wave animation
  - `<rainbow>` — HSV color cycling
  - `<glitch>` — RGB chromatic aberration + jitter
  - `<speed=N>` — per-character typewriter interval
  - `<w=N>` — mid-text pause
- **Forced read system**:
  - `no_skip: true` node attribute prevents skipping during typewriter
  - `wait: N` node attribute forces mandatory delay after typewriter
  - Skip mode auto-stops on forced read nodes
  - Blinking red indicator in bottom-right corner during forced read

### Added — Nine-Slice UI System

- **`NineSliceSprite`** class with SFML 3.x compatibility
- **Data-driven UI theme** (`ui_theme.json`) now supports nine-slice configuration
- Applied nine-slice to choice buttons in demo
- Dialogue box supports nine-slice (with AI-generated blue circuit board frame)

### Added — Custom UI Image System

- Extended `UITheme` to support per-element image configuration
- `backgroundImage` / `nameBoxImage` for dialogue box
- `normalImage` / `hoverImage` for choice buttons
- Fallback to solid color when image fails to load

### Added — Tooling

- `devtools/private/trim_ui_assets.py` — auto-crop whitespace from AI-generated UI
- `devtools/private/measure_bounds.py` — detect nine-slice boundary candidates
- `devtools/private/crop_name_box.py` — horizontal frame cropping utility
- `devtools/scripts/gality_compiler.py` now parses `no_skip` and `wait` attributes

### Added — Demos

- `assets/scripts/test_day4.gality` — full Day 4 feature test
- `assets/scripts/test_fx.gality` — comprehensive text effects showcase
- `assets/scripts/text_effects_showcase.gality` — narrative effect recipes

### Changed

- `DialogueBox` refactored for full inline tag parsing
- `UITheme` extended with `TextShadowStyle`, `TextOutlineStyle`, `FontConfig`
- `StoryNode` extended with `noSkip`, `noSkipWait` fields
- `ScriptLoader` loads new node attributes
- Migrated all font loading to `FontManager` singleton

### Fixed

- `std::array<sf::Sprite, 9>` default construction issue (SFML 3.x)
- `NineSliceSprite` now uses `std::vector` to avoid default construction
- `localtime` warnings in `SafeTime.hpp`
- `NodeGraphViewer` unused parameter warnings

### Performance

- **60 FPS verified** with all text effects enabled
- Text shadow: 2x draws per glyph
- Text outline: 9x draws per glyph (disabled by default)
- Glitch: up to 3x draws per glitched glyph
- No frame drops during normal dialogue

### Documentation

- `docs/` updates for 16:9 aspect ratio
- Architecture documentation
- Performance tuning guide

## [0.2.0]https://github.com/moli0516/Gality/compare/v0.1.0...v0.2.0 - 2026-09-15

### Added — Core Engine

- Data-driven UI theme (`ui_theme.json`) with full hot reload
- **Save/Load system** with JSON snapshots
- **Time Machine Rollback** (128-step state snapshot stack)
- **Backlog UI** with scrollable history
- **Settings UI** (audio, graphics, system tabs)
- **Hot reload** for scripts and shaders (dev build)
- **Debug overlay** (`F1`) with 4 tabs: Blackboard, Node Jumper, Diagnostics, Console
- **Node graph viewer** (`F2`) with live node editing
- **Weather system** — rain, snow, sakura, none

### Added — Rendering

- 16:9 aspect ratio support
- Letterbox viewport calculation
- Dual-buffer offscreen compositor

### Added — Build System

- `build_dev.bat` — one-click develop build
- `build_prod.bat` — one-click product build
- `build_game.sh` — macOS/Linux build script
- Distribution packaging with runtime DLLs

### Changed

- Refactored `StoryNode` for multi-slot character rendering
- Improved `StoryExecutor` state machine
- README rewritten with feature tables

### Fixed

- Multiple `AudioManager` crossfade fixes
- Configuration persistence

## [0.1.0]https://github.com/moli0516/Gality/releases/tag/v0.1.0 - 2026-09-14

### Added — Initial Release

- **Core engine**: SFML 3.x, C++17
- **`.gality` DSL** + Python compiler (`gality_compiler.py`)
- **Asset packer** (`gality_packer.py`) with XOR encryption
- **Dialogue system** with typewriter effect
- **Choice system** with branching
- **Multi-slot character rendering** (Left / Center / Right)
- **Character dimming** for non-speaking characters
- **Transition system** with 4 mask types (diamond, wipe_left, dissolve, clock)
- **GLSL PostFX pipeline**: dual-pass Gaussian blur, screen shake
- **LRU texture cache** (capacity 50)
- **VSCode language extension** (`gality-language-support-1.0.0.vsix`)
- **~3000 node demo story** with 4 routes, 10 endings
- **Asset pack VFS** with O(1) offset lookups

### Infrastructure

- `vcpkg.json` for dependency management
- CMake 3.20+ build configuration
- GitHub repository initialized

[Unreleased]: https://github.com/moli0516/Gality/compare/v0.3.0...HEAD
[0.3.0]: https://github.com/moli0516/Gality/compare/v0.2.0...v0.3.0
[0.2.0]: https://github.com/moli0516/Gality/compare/v0.1.0...v0.2.0

[0.1.0]: https://github.com/moli0516/Gality/releases/tag/v0.1.0
