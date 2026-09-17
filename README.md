
# 🌸 Gality Engine

**A narrative engine built by engineers, for engineers.**

_C++17 · SFML 3.x · Zero GC · 48 MB footprint · Pure native vector UI_

_「有些東西會永遠留在原始碼裡。」_

[![Version](https://img.shields.io/badge/version-0.3.0-blue.svg)](CHANGELOG.md)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![SFML](https://img.shields.io/badge/SFML-3.x-green.svg)](https://www.sfml-dev.org/)

---

## 📖 What is Gality?

Gality is a **production-grade 2D visual novel & narrative game engine** written from scratch in **C++17** and **SFML 3.x**.

But it's also more than that.

It's an answer to a question most engines ignore:

> **What if the tool you built to tell stories, started telling its own?**

Gality was born from a simple obsession — writing a visual novel engine that doesn't compromise on engineering. No garbage collection. No third-party GUI bloat. No JSON spaghetti for writers. Just clean architecture, tight memory, and a DSL that feels like writing a screenplay.

And then, somewhere along the way, that engine became the story itself.

---

## ✨ Why Gality?

### For Engineers

| Feature | What it actually means |
|---|---|
| **Zero-GC C++17 Core** | No `shared_ptr` in hot paths. RAII everywhere. Sub-100ms cold start. |
| **~48 MB Runtime** | Fits in a browser tab's budget. Fits on a Raspberry Pi. |
| **Pure Native Vector UI** | No Dear ImGui. No Qt. No webview. Just SFML primitives and math. |
| **Generic LRU Cache** | Textures and audio buffers recycled by a template-based eviction policy. |
| **XOR-Encrypted VFS** | `data.pak` archives with `O(1)` offset lookups. No external asset loader. |
| **GLSL PostFX Pipeline** | Dual-pass Gaussian blur, screen shake, smoothstep mask transitions. |
| **Time Machine Rollback** | 128-step state snapshot stack. Rewind narrative AND presentation. |
| **Nine-Slice UI System** | Data-driven nine-slice rendering for any UI element. Zero hardcoding. |
| **FontManager Singleton** | Dynamic font loading with graceful fallback. Missing fonts never crash. |

### For Writers

| Feature | What it actually means |
|---|---|
| **`.gality` DSL** | Write dialogue like a screenplay. Compile to JSON AST. Ship. |
| **Rich Inline Tags** | `<color>`, `<shake>`, `<wave>`, `<rainbow>`, `<glitch>`, `<speed>`, `<w>` |
| **Forced Read System** | `no_skip: true` + `wait: N` — dramatic moments that players **must** experience. |
| **CJK Kinsoku Shori** | 避頭尾禁則字元 — punctuation never lands in the wrong place. |
| **Typewriter Audio** | Every character clicks. Every pause breathes. |
| **Multi-Slot Characters** | Left / Center / Right staging with auto-focus dimming and spatial voice panning. |
| **Data-Driven Theme** | Colors, fonts, scaling, shadows — all in `ui_theme.json`. Hot-reloadable. |
| **Weather System** | Sakura, rain, snow — because mood is a particle system. |
| **VSCode Extension** | Syntax highlighting, linting, go-to-definition, IntelliSense. Included. |

### For Players

- **60 FPS** on integrated graphics
- **Instant save/load** via slot UI (`F5` / `F9`)
- **Rollback** with mouse wheel or `Backspace`
- **History backlog** with `Tab` or `H`
- **Native debug overlay** (`F1`) and **DAG tree viewer** (`F2`) — yes, players can see the story graph
- **Ten endings.** None of them truly happy. All of them earned.

---

## 🆕 What's New in v0.3.0

- **Text Effects System**
  - `<wave>` — vertical sine wave animation
  - `<rainbow>` — HSV color cycling
  - `<glitch>` — RGB chromatic aberration
  - Text shadow (configurable)
  - Text outline (8-direction, optional)
  - `FontManager` with dynamic font loading

- **Forced Read System**
  - `no_skip: true` prevents skipping during typewriter
  - `wait: N` mandates delay after typewriter
  - Skip auto-stops on forced read nodes

- **Nine-Slice UI**
  - `NineSliceSprite` class
  - Data-driven nine-slice in `ui_theme.json`
  - Applied to choice buttons

See [CHANGELOG.md](CHANGELOG.md) for the full history.

---

## 🚀 Quick Start

### Prerequisites

- **C++17** compiler (MSVC 2019+, GCC 10+, Clang 11+)
- **CMake 3.20+**
- **Python 3.8+**
- **vcpkg** (recommended) or Homebrew

### Install Dependencies

**Windows (vcpkg):**
```bash
vcpkg install sfml nlohmann-json
```

**macOS (Homebrew):**
```bash
brew install cmake sfml nlohmann-json
```

### Build from Source

```bash
# 1. Compile the .gality DSL into a JSON AST
python devtools/scripts/gality_compiler.py \
    assets/scripts/main_story_multi.gality \
    assets/scripts/demo_long.json

# 2. Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# 3. Launch
# Windows:
.\build\Release\Gality.exe
# macOS / Linux:
./build/Gality
```

### One-Click Release (Windows)

```bash
.\build_game.bat
```

This will:

1. Compile `.gality` → JSON AST
2. Pack `assets/` → encrypted `data.pak`
3. Build `Gality.exe` (Release)
4. Assemble standalone bundle in `.\dist\`

---

## ✍️ Writing with `.gality`

Gality's DSL is designed so that **writers never touch JSON**.

```gality
@start node_01

[node_01]
bg: assets/bg/classroom.jpg
trans: diamond
duration: 1.5
bgm: assets/audio/bgm_daily.ogg
weather: sakura
旁白: 四月的春風掠過校園，窗外灑下一陣<color=#FFB7C5><shake>櫻花雨</shake></color>。
-> node_02

[node_02]
bg: assets/bg/classroom.jpg
char_center: assets/characters/senior_normal.png
cv: assets/audio/cv_01.ogg
學姐: 你終於來了！我們提報的<color=#FF5555>自製 2D 遊戲引擎計畫</color>，今天必須敲定架構方向。
-> choice_01

[choice_01]
? [包在我身上吧！今晚我就把核心底層跑通！] -> act_favor_up1
? [架構好繁重啊... 我們真不用現成引擎嗎？] -> act_favor_down1

[act_favor_up1]
$ favorability += 10
-> node_10_a

[act_favor_down1]
$ favorability += -5
-> node_10_b

[node_route_selector]
IF san_loss >= 60 THEN node_r_abyss_start ELSE node_route_selector_2
```

### 🎭 Horror Atmosphere Recipe

Combine `<glitch>` + `<speed>` + `no_skip` to force players to experience terror:

```gality
[node_horror_01]
bg: assets/bg/hallway.jpg
bgm: assets/audio/bgm_horror.ogg
shake: 0.3
no_skip: true
wait: 1.0
幽靈: <glitch><speed=0.5>你... 終於... 打開了...</speed></glitch>
-> node_horror_02
```

**Effect stack**:
- Typewriter at 0.5s per char → forces slow reading
- `<glitch>` → RGB splitting + jitter on every char
- `no_skip` → player cannot escape
- `wait: 1.0` → final silence before advancing
- `shake: 0.3` → subtle screen tremble

### What you get

- **Directives**: `@start`, `[node_id]`, `->`, `IF/THEN/ELSE`
- **Blackboard mutations**: `$ var += val`
- **Rich text**: `<color>`, `<shake>`, `<wave>`, `<rainbow>`, `<glitch>`, `<speed>`, `<w>`
- **Transitions**: `diamond`, `wipe_left`, `clock`, `dissolve`
- **Weather**: `sakura`, `rain`, `snow`, `none`
- **Forced read**: `no_skip: true`, `wait: N`

The compiler will flag:

- Dangling `-> node_999` pointers
- Duplicate node IDs
- Mismatched rich text tags

Before you ever run the game.

---

## 📚 Documentation

| Document | Description |
|---|---|
| [Getting Started](docs/getting_started.md) | First steps with Gality |
| [Script Language](docs/script_language.md) | Complete `.gality` DSL reference |
| [UI Guide](docs/ui_guide.md) | Customize `ui_theme.json` |
| [Character Guide](docs/character_guide.md) | Multi-slot character rendering |
| [Assets Guide](docs/assets.md) | Asset pipeline and packing |
| [Architecture](docs/architecture.md) | Engine internals |
| [Performance Tuning](docs/performance_tuning.md) | Optimization guide |
| [Examples](docs/example.md) | Sample projects |
| [CHANGELOG](CHANGELOG.md) | Version history |
| [Contributing](CONTRIBUTING.md) | How to contribute |
| [Code of Conduct](CODE_OF_CONDUCT.md) | Community guidelines |

---

## 🎮 Controls

| Input | Action |
|---|---|
| **Left Click / Enter / Space** | Advance dialogue / Confirm choice |
| **Mouse Wheel Up / Backspace** | Rollback (Time Machine) |
| **Tab / H** | Toggle History Backlog |
| **Escape** | Settings / Close modal |
| **F1 / `~`** | DebugOverlay (Blackboard, Node Jumper, Diagnostics) |
| **F2** | NodeGraphViewer (DAG tree + live inspector) |
| **F5 / F9** | Quick Save / Quick Load |
| **K** | Test screen shake |
| **F12** | Screenshot |

---

## 🧩 VS Code Extension

Gality ships with a dedicated language extension: **`gality-language-support-1.0.0.vsix`**

### Capabilities

- **TextMate syntax highlighting** for `.gality` files
- **Static linter** — flags dangling nodes, duplicate definitions, tag mismatches
- **Go to Definition** (`F12` / `Ctrl+Click`) — jump from `-> node_999` to `[node_999]`
- **IntelliSense** — context-aware suggestions for backgrounds, audio, characters

### Install

```bash
code --install-extension gality-language-support-1.0.0.vsix
```

Or via VS Code UI: **Extensions** → `...` → **Install from VSIX...**

---

## 📁 Repository Structure

```
gality/
├── assets/
│   ├── audio/              # BGM, SFX, voice lines
│   ├── bg/                 # Background images
│   ├── characters/         # Character sprites (RGBA)
│   ├── config/             # ui_theme.json, settings.json
│   ├── fonts/              # TTF fonts (CJK-compatible)
│   ├── masks/              # Transition masks (grayscale)
│   ├── scripts/            # .gality source + compiled JSON
│   ├── shaders/            # blur.frag, transition.frag
│   └── ui/                 # Nine-slice UI textures
├── devtools/
│   ├── scripts/
│   │   ├── gality_compiler.py  # DSL → JSON AST compiler
│   │   └── gality_packer.py    # XOR-encrypted asset packer
│   └── private/                # Developer-only tools
├── docs/                   # Documentation
├── src/
│   ├── core/               # AssetPack, AudioManager, Blackboard, LRUCache, SaveManager, FontManager
│   ├── render/             # LayerRenderer, PostFX, WeatherSystem, NineSliceSprite, all UI
│   └── story/              # StoryExecutor, ScriptLoader, StoryNode
├── build_dev.bat           # Dev build (Windows)
├── build_prod.bat          # Product build (Windows)
├── build_game.sh           # Build script (macOS/Linux)
├── CMakeLists.txt
├── CHANGELOG.md
├── CONTRIBUTING.md
├── CODE_OF_CONDUCT.md
├── LICENSE
├── README.md
└── gality-language-support-1.0.0.vsix
```

---

## 🏗️ Architecture at a Glance

```
┌─────────────────────────────────────────────────────────────┐
│                     Gality Engine Runtime                    │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│   ┌──────────────┐    ┌──────────────┐    ┌──────────────┐ │
│   │  ScriptLoader │───▶│ StoryExecutor│───▶│  Blackboard  │ │
│   │  (JSON AST)   │    │  (FSM loop)  │    │  (variables) │ │
│   └──────────────┘    └──────┬───────┘    └──────────────┘ │
│                              │                              │
│                              ▼                              │
│   ┌──────────────┐    ┌──────────────┐    ┌──────────────┐ │
│   │ LayerRenderer │◀───│  StoryNode   │───▶│  AudioManager │ │
│   │  (sprites)    │    │  (dialogue)  │    │  (BGM + CV)  │ │
│   └──────┬───────┘    └──────────────┘    └──────────────┘ │
│          │                                                  │
│          ▼                                                  │
│   ┌──────────────┐    ┌──────────────┐    ┌──────────────┐ │
│   │  Transition  │───▶│   PostFX     │───▶│   Window     │ │
│   │  (masked)    │    │  (GLSL blur) │    │  (letterbox) │ │
│   └──────────────┘    └──────────────┘    └──────────────┘ │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

- **No global state.** Every subsystem is owned by `GalityApp`.
- **No hidden allocations.** Textures and audio live in `LRUCache`.
- **No magic.** The FSM is a `while` loop. You can read it in one sitting.

---

## 🧪 Testing the Engine

The repository ships with a **~3000 node demo story** that exercises every feature:

- 4 branching routes
- 10 distinct endings
- Hidden rooms (library, infirmary, rooftop, data stream)
- Sanity loss tracking
- Ghost dialogue via `幽靈` speaker
- Multi-slot character staging
- Full CJK text rendering
- **Text effects showcase** (`test_fx.gality`)

Run it:

```bash
.\build_game.bat
.\dist\Gality.exe
```

And try not to fall in love with the 副社長.

---

## 🗺️ Roadmap

### ✅ v0.1.0 — Initial Release (2026-09-14)
- [x] C++17 core, SFML 3.x rendering
- [x] `.gality` DSL + compiler
- [x] Multi-slot character pipeline
- [x] Weather + PostFX + transitions
- [x] Encrypted `data.pak` VFS
- [x] Debug overlay + DAG viewer
- [x] VSCode language extension

### ✅ v0.2.0 — Tooling & Polish (2026-09-15)
- [x] 16:9 aspect ratio support
- [x] Data-driven UI theme (`ui_theme.json`)
- [x] Save/Load system
- [x] Rollback (Time Machine)
- [x] Backlog UI
- [x] Settings UI
- [x] Hot reload (dev build)
- [x] One-click build scripts

### ✅ v0.3.0 — Text Effects & Nine-Slice (2026-09-17)
- [x] Nine-slice UI system
- [x] FontManager singleton
- [x] Text shadow
- [x] Text outline (optional)
- [x] `<wave>`, `<rainbow>`, `<glitch>` animations
- [x] `<speed>`, `<w>` control tags
- [x] Forced read system (`no_skip`, `wait`)
- [x] Custom UI image system

### 🚧 v0.4.0 — Month 2 (In Progress)
- [ ] CG System (gallery, viewer, unlock tracking)
- [ ] Character layered parts (blink, mouth, expression)
- [ ] AI toolchain (translation, TTS, image generation)
- [ ] itch.io release

### 🔮 v0.5.0 — Month 3-6
- [ ] WebAssembly build (pending SFML Emscripten support)
- [ ] Cross-platform CI/CD
- [ ] Plugin system
- [ ] Steam release of demo story

### 🌟 v1.0.0 — Year 1
- [ ] Stable API
- [ ] Comprehensive docs
- [ ] Commercial license
- [ ] Community showcase

---

## 🤝 Contributing

Contributions are welcome — but keep the aesthetic:

- **No third-party GUI frameworks.**
- **No Garbage Collected languages in the runtime.**
- **No JSON in the writer's hands.**

If your patch adds a new dependency, explain why in the PR description.

See [CONTRIBUTING.md](CONTRIBUTING.md) for details.

---

## 📜 License

MIT. See [LICENSE](LICENSE) for details.

---

## 🙏 Acknowledgments

Built with:
- [SFML 3.x](https://www.sfml-dev.org/) — Simple and Fast Multimedia Library
- [nlohmann/json](https://github.com/nlohmann/json) — JSON for Modern C++
- [vcpkg](https://vcpkg.io/) — C++ package manager

---

**Gality Engine**

_Built because the story deserved a real engine._

_And the engine deserved a real story._
