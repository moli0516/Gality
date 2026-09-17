
# 🌸 Gality Engine

**A narrative engine with maximum freedom.**

_Write in `.gality`. Customize in JSON. Extend in C++._
_All three layers are yours._

_C++17 · SFML 3.x · Zero GC · 48 MB footprint · Pure native vector UI_

_「有些東西會永遠留在原始碼裡。」_

[![Version](https://img.shields.io/badge/version-0.3.0-blue.svg)](CHANGELOG.md)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![SFML](https://img.shields.io/badge/SFML-3.x-green.svg)](https://www.sfml-dev.org/)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20macOS-blue.svg)](#-platform-support)

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

### For Creators Who Want Control

Most visual novel engines fall into two camps:

1. **GUI-first editors** (Unity + Naninovel, Godot + Dialogic, RouteVN)
2. **GUI-first launchers** (Ren'Py Launcher, VNU)

Gality rejects both.

**Gality is for creators who want to use the tools they already know.**

| Your Tool | What Gality Uses |
|---|---|
| **Editor** | VS Code (not a custom IDE) |
| **Language** | `.gality` DSL (not drag-and-drop) |
| **Debugger** | F1 / F2 (runtime tools, not GUI panels) |
| **Version control** | Git (works because everything is text) |

### For Writers

- **`.gality` DSL** — Write dialogue like a screenplay
- **Rich inline tags** — `<color>`, `<shake>`, `<wave>`, `<rainbow>`, `<glitch>`, `<speed>`, `<w>`
- **Forced read system** — `no_skip` + `wait` for dramatic moments players **must** experience
- **CJK Kinsoku Shori** — 避頭尾禁則字元, automatic
- **Multi-slot characters** — Left / Center / Right staging with auto-dimming
- **Data-driven theme** — All styling in `ui_theme.json`, hot-reloadable
- **Weather system** — Sakura, rain, snow

### For Engineers

- **Zero-GC C++17 core** — No `shared_ptr` in hot paths, RAII everywhere
- **~48 MB runtime** — Fits on a Raspberry Pi
- **Pure native vector UI** — No Dear ImGui, no Qt, no webview
- **Generic LRU cache** — Textures and audio recycled by eviction policy
- **XOR-encrypted VFS** — `data.pak` with O(1) offset lookups
- **GLSL PostFX pipeline** — Blur, screen shake, mask transitions
- **Time Machine rollback** — 128-step state snapshot stack
- **Nine-slice UI system** — Data-driven, zero hardcoding

### For Players

- **60 FPS** on integrated graphics
- **Instant save/load** via slot UI (`F5` / `F9`)
- **Rollback** with mouse wheel or `Backspace`
- **History backlog** with `Tab` or `H`
- **Native debug overlay** (`F1`) and **DAG tree viewer** (`F2`)
- **Ten endings.** None of them truly happy. All of them earned.

---

## 🆕 What's New in v0.3.0

### Text Effects System
- `<wave>` — vertical sine wave animation
- `<rainbow>` — HSV color cycling
- `<glitch>` — RGB chromatic aberration
- `<speed=N>` — per-character typewriter speed
- `<w=N>` — mid-text pause
- Text shadow (configurable)
- Text outline (8-direction, optional)
- `FontManager` with dynamic font loading

### Forced Read System
- `no_skip: true` prevents skipping during typewriter
- `wait: N` mandates delay after typewriter
- Skip auto-stops on forced read nodes

### Nine-Slice UI
- `NineSliceSprite` class (SFML 3.x compatible)
- Data-driven nine-slice in `ui_theme.json`
- Applied to choice buttons

### Horror Atmosphere Recipe

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

See [CHANGELOG.md](CHANGELOG.md) for the full history.

---

## 💡 Design Philosophy

### Decoupling is the Architecture

Gality is built on a principle: **every subsystem is independent**.

- **Engine core** doesn't know about your story
- **Your story** doesn't know about the engine
- **UI theme** is pure data, not code
- **FX** are pluggable C++ modules
- **Tools** are separate Python scripts

This means you can:
- Replace any subsystem with your own
- Test any module in isolation
- Extend without forking
- Understand one part at a time

### No GUI. No Black Boxes.

Gality rejects GUI-first design. Instead:

- **`.gality` DSL** replaces node editors
- **`ui_theme.json`** replaces property panels
- **VS Code extension** replaces drag-and-drop IDEs
- **Python tools** replace build buttons

**Why?**

- **Transparency**: You can read everything
- **Version control**: Git works the way it's supposed to
- **Freedom**: No feature locked behind a GUI button
- **Longevity**: Text files outlive GUIs

If you want drag-and-drop, there are other engines. If you want control, Gality is for you.

### Is Gality Reinventing the Wheel?

**No.** Gality uses proven libraries at every layer:

| Layer | Tool |
|---|---|
| Graphics / Audio | [SFML 3.x](https://www.sfml-dev.org/) |
| JSON Parsing | [nlohmann/json](https://github.com/nlohmann/json) |
| Font Rendering | [FreeType](https://freetype.org/) |
| Dependency Management | [vcpkg](https://vcpkg.io/) |
| Version Control | [Git](https://git-scm.com/) |
| Editor | [VS Code](https://code.visualstudio.com/) |

What Gality builds on top:

- `.gality` DSL — because Ink / Ren'Py don't fit our C++ engine
- C++ story executor — because we need zero GC
- JSON UI theme — because we want pure text workflow
- C++ FX system — because it doesn't exist anywhere

**We're not reinventing. We're building the layer that doesn't exist.**

### Freedom at Every Layer

| Layer | Who | What They Do |
|---|---|---|
| **Layer 1: Write** | Creators | `.gality` DSL + `ui_theme.json` (no C++) |
| **Layer 2: Configure** | Advanced creators | Python tools (`gality_compiler.py`, `gality_packer.py`) |
| **Layer 3: Extend** | Engine developers | Fully open C++17 core |

**Everyone chooses their own depth.**

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

### One-Click Build (Windows)

```bash
.\build_dev.bat    # Develop build (debug tools enabled)
.\build_prod.bat   # Product build (debug tools disabled)
```

The Dev build will:

1. Compile `.gality` → JSON AST
2. Pack `assets/` → encrypted `data.pak`
3. Build `Gality.exe` (RelWithDebInfo)
4. Assemble standalone bundle in `.\dist-dev\`

---

## 🖥️ Platform Support

| Platform | Status | Notes |
|---|---|---|
| **Windows 10/11** | ✅ Tested | MSVC 2022, vcpkg |
| **macOS (Apple Silicon)** | ✅ Tested | Homebrew SFML 3.x |
| **macOS (Intel)** | ⚠️ Untested | Should work, no test report |
| **Linux (Ubuntu 22.04+)** | ⚠️ Untested | `build_game.sh` provided |
| **WebAssembly** | 🚧 Planned | Awaiting SFML Emscripten support |
| **iOS / Android** | ❌ Not planned | Desktop-first engine |

**Want to help?** If you successfully build on an untested platform, please open a PR to update this table.

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

### What You Get

- **Directives**: `@start`, `[node_id]`, `->`, `IF/THEN/ELSE`
- **Blackboard mutations**: `$ var += val`
- **Rich text**: `<color>`, `<shake>`, `<wave>`, `<rainbow>`, `<glitch>`, `<speed>`, `<w>`
- **Transitions**: `diamond`, `wipe_left`, `clock`, `dissolve`
- **Weather**: `sakura`, `rain`, `snow`, `none`
- **Forced read**: `no_skip: true`, `wait: N`

The compiler will flag dangling pointers, duplicate node IDs, and mismatched rich text tags **before you ever run the game**.

---

## 🛠️ Development Workflow

**Gality does not ship with a GUI editor. You don't need one.**

Your IDE is **VS Code** with the official `gality-language-support` extension.

| Task | Tool |
|---|---|
| Write story | VS Code + `.gality` DSL |
| Configure UI | VS Code + `ui_theme.json` |
| Build & run | `build_dev.bat` |
| Debug variables | **F1** DebugOverlay |
| Inspect story graph | **F2** NodeGraphViewer |
| Version control | Git (VS Code built-in) |

### Why No Built-in Editor?

Because we'd rather not build one. Building an editor means:

- Years of engineering
- Constant maintenance
- Users learning yet another tool
- Losing the VS Code ecosystem

Instead, we extend VS Code and expose runtime debugging tools.

**Result**: You get a professional IDE without us shipping one.

---

## 📚 Documentation

**Full documentation index**: [docs/README.md](docs/README.md)

### Getting Started

| Document | Description |
|---|---|
| **[Getting Started](docs/getting_started.md)** | Install Gality, write first scene (30 min) |
| **[Examples](docs/example.md)** | 9 complete runnable examples |

### For Creators

| Document | Description |
|---|---|
| **[Script Language](docs/script_language.md)** | Complete `.gality` DSL reference |
| **[UI Guide](docs/ui_guide.md)** | Customize `ui_theme.json` |
| **[Character Guide](docs/character_guide.md)** | Multi-slot character rendering |
| **[Assets Guide](docs/assets.md)** | Prepare images, audio, fonts |

### For Engine Developers

| Document | Description |
|---|---|
| **[Architecture](docs/architecture.md)** | Engine internals |
| **[Performance Tuning](docs/performance_tuning.md)** | Optimization tips |

### Project Information

| Document | Description |
|---|---|
| **[Roadmap](docs/roadmap.md)** | Release history and future plans |
| **[FAQ](docs/faq.md)** | 100+ common questions |
| **[CHANGELOG](CHANGELOG.md)** | Version history |
| **[Contributing](CONTRIBUTING.md)** | How to contribute |
| **[Code of Conduct](CODE_OF_CONDUCT.md)** | Community guidelines |
| **[Security](SECURITY.md)** | Security policy |

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
├── SECURITY.md
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
.\build_dev.bat
.\dist-dev\Gality.exe
```

And try not to fall in love with the 副社長.

---

## 🗺️ Roadmap

**Full roadmap**: [docs/roadmap.md](docs/roadmap.md)

### ✅ Released

- **v0.1.0** (2026-09-14) — Initial release
- **v0.2.0** (2026-09-15) — Tooling & polish
- **v0.3.0** (2026-09-17) — Text effects & nine-slice

### 🚧 In Progress

- **v0.4.0** (Month 2) — CG system, layered characters, AI toolchain

### 🔮 Planned

- **v0.5.0** (Month 3-6) — WebAssembly, CI/CD, FX registry
- **v1.0.0** (Year 1) — Stable API, comprehensive docs

### ❌ Not Planned

- Mobile (iOS / Android)
- GUI editor
- 3D rendering
- Physics engine
- Multiplayer

See [Roadmap](docs/roadmap.md) for details.

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
- [FreeType](https://freetype.org/) — Font rendering

---

<div align="center">

**Gality Engine**

_Built because the story deserved a real engine._

_And the engine deserved a real story._

</div>
