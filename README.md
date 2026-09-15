# 🌸 Gality Engine

**A narrative engine built by engineers, for engineers.**

*C++17 · SFML 3.x · Zero GC · 48 MB footprint · Pure native vector UI*

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![SFML](https://img.shields.io/badge/SFML-3.x-green.svg)](https://www.sfml-dev.org/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20macOS-lightgrey.svg)]()

*「有些東西會永遠留在原始碼裡。」*

</div>

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


| Feature                   | What it actually means                                                    |
| ------------------------- | ------------------------------------------------------------------------- |
| **Zero-GC C++17 Core**    | No`shared_ptr` in hot paths. RAII everywhere. Sub-100ms cold start.       |
| **~48 MB Runtime**        | Fits in a browser tab's budget. Fits on a Raspberry Pi.                   |
| **Pure Native Vector UI** | No Dear ImGui. No Qt. No webview. Just SFML primitives and math.          |
| **Generic LRU Cache**     | Textures and audio buffers recycled by a template-based eviction policy.  |
| **XOR-Encrypted VFS**     | `data.pak` archives with `O(1)` offset lookups. No external asset loader. |
| **GLSL PostFX Pipeline**  | Dual-pass Gaussian blur, screen shake, smoothstep mask transitions.       |
| **Time Machine Rollback** | 128-step state snapshot stack. Rewind narrative AND presentation.         |

### For Writers


| Feature                   | What it actually means                                                                     |
| ------------------------- | ------------------------------------------------------------------------------------------ |
| **`.gality` DSL**         | Write dialogue like a screenplay. Compile to JSON AST. Ship.                               |
| **Rich Inline Tags**      | `<color=#FFB7C5>`, `<shake>`, `<speed=0.05>`, `<w=1.2>` — expressive without markup hell. |
| **CJK Kinsoku Shori**     | 避頭尾禁則字元 — punctuation never lands in the wrong place.                              |
| **Typewriter Audio**      | Every character clicks. Every pause breathes.                                              |
| **Multi-Slot Characters** | Left / Center / Right staging with auto-focus dimming and spatial voice panning.           |
| **Weather System**        | Sakura, rain, snow — because mood is a particle system.                                   |
| **VSCode Extension**      | Syntax highlighting, linting, go-to-definition, IntelliSense. Included.                    |

### For Players

- **60 FPS** on integrated graphics
- **Instant save/load** via slot UI (`F5` / `F9`)
- **Rollback** with mouse wheel or `Backspace`
- **History backlog** with `Tab` or `H`
- **Native debug overlay** (`F1`) and **DAG tree viewer** (`F2`) — yes, players can see the story graph
- **Ten endings.** None of them truly happy. All of them earned.

---

## 🚀 Quick Start

### Prerequisites

- **C++17** compiler (MSVC 2019+, GCC 10+, Clang 11+)
- **CMake 3.20+**
- **Python 3.8+**
- **vcpkg** (recommended) or Homebrew

### Install Dependencies

**Windows (vcpkg):**

```powershell
vcpkg install sfml nlohmann-json
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

```powershell
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

[node_route_selector_2]
IF junior_favor >= 40 THEN node_r_junior_start ELSE node_route_selector_3

[node_route_selector_3]
IF favorability >= 100 THEN node_r_senior_start ELSE node_r_tech_start
```

### What you get

- **Directives**: `@start`, `[node_id]`, `->`, `IF/THEN/ELSE`
- **Blackboard mutations**: `$ var += val`
- **Rich text**: `<color>`, `<shake>`, `<speed>`, `<w>`
- **Transitions**: `diamond`, `wipe_left`, `clock`, `dissolve`
- **Weather**: `sakura`, `rain`, `snow`, `none`

The compiler will flag:

- Dangling `-> node_999` pointers
- Duplicate node IDs
- Mismatched rich text tags

Before you ever run the game.

---

## 🎮 Controls


| Input                          | Action                                              |
| ------------------------------ | --------------------------------------------------- |
| **Left Click / Enter / Space** | Advance dialogue / Confirm choice                   |
| **Mouse Wheel Up / Backspace** | Rollback (Time Machine)                             |
| **Tab / H**                    | Toggle History Backlog                              |
| **Escape**                     | Settings / Close modal                              |
| **F1 / `~`**                   | DebugOverlay (Blackboard, Node Jumper, Diagnostics) |
| **F2**                         | NodeGraphViewer (DAG tree + live inspector)         |
| **F5 / F9**                    | Quick Save / Quick Load                             |
| **K**                          | Test screen shake                                   |

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
│   └── shaders/            # blur.frag, transition.frag
├── devtools/scripts/
│   ├── gality_compiler.py  # DSL → JSON AST compiler
│   └── gality_packer.py    # XOR-encrypted asset packer
├── src/
│   ├── core/               # AssetPack, AudioManager, Blackboard, LRUCache, SaveManager
│   ├── render/             # LayerRenderer, PostFX, WeatherSystem, all UI
│   └── story/              # StoryExecutor, ScriptLoader, StoryNode
├── build_game.bat          # One-click release (Windows)
├── CMakeLists.txt
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
- Ghost dialogue via `???` speaker
- Multi-slot character staging
- Full CJK text rendering

Run it:

```powershell
.\build_game.bat
.\dist\Gality.exe
```

And try not to fall in love with the副社長.

---

## 🗺️ Roadmap

- [X]  C++17 core, SFML 3.x rendering
- [X]  `.gality` DSL + compiler
- [X]  Multi-slot character pipeline
- [X]  Weather + PostFX + transitions
- [X]  Encrypted `data.pak` VFS
- [X]  Debug overlay + DAG viewer
- [X]  VSCode language extension
- [ ]  WebAssembly build (pending SFML Emscripten support)
- [ ]  Cross-platform CI/CD
- [ ]  Steam release of demo story

---

## 🤝 Contributing

Contributions are welcome — but keep the aesthetic:

- **No third-party GUI frameworks.**
- **No Garbage Collected languages in the runtime.**
- **No JSON in the writer's hands.**

If your patch adds a new dependency, explain why in the PR description.

---

## 📜 License

MIT. See `LICENSE` for details.

---

<div align="center">

**Gality Engine**

*Built because the story deserved a real engine.*

*And the engine deserved a real story.*

</div>
