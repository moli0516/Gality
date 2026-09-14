# 🌸 Gality Engine

**Gality** is a high-performance, production-grade 2D Visual Novel & Narrative Game Engine engineered in **C++17** and **SFML 3.x**. Designed around a data-driven architecture, Gality completely decouples engine runtime logic from narrative content creation, delivering zero-GC execution, an ultra-low memory footprint (~48MB), and a 100% pure native vector UI subsystem without any third-party GUI framework overhead.

---

## ✨ Features

* **⚡ Ultra-Fast C++17 Core**: Native compiled execution with zero garbage collection pauses, strict RAII resource ownership, and sub-100ms cold-start latency.
* **🎨 Pure Native Vector UI & Data-Driven Themes**: 100% Dear ImGui-free. Custom-built vector rendering pipeline for title menus, dialogue boxes, choices, settings, save/load slots, and history backlog, skinning seamlessly via `assets/config/ui_theme.json`.
* **📜 Human-Friendly `.gality` DSL**: Writers craft dialogue, branching logic, Blackboard variable mutations, and conditional evaluations in clean text files compiled down to JSON ASTs via `devtools/scripts/gality_compiler.py`.
* **💬 Rich Text & CJK Kinsoku Shori**: Native support for inline tags (`<color=#HEX>`, `<shake>`, `<speed>`, `<w>`), strict CJK line-breaking rules (prohibiting punctuation at line beginnings and openings at line endings), and typewriter audio feedback.
* **🎭 Multi-Slot Character Pipeline**: Dynamic stage management (`CharSlot::Left`, `Center`, `Right`) with auto-scaling, auto-focus dimming, and spatial audio panning.
* **✨ GPU Visuals & Weather System**: Real-time particle systems for **Sakura**, **Rain**, and **Snow**, alongside Easing transitions and smoothstep grayscale mask shaders (`diamond`, `wipe_left`, `clock`, `dissolve`).
* **🌌 PostFX Shader Pipeline**: Integrated GLSL fragment shaders supporting real-time dual-pass Gaussian Blur and dynamic screen shake offsets.
* **📦 Generic LRU Cache & VFS**: Template-based asset recycling for textures and audio buffers, coupled with XOR-encrypted virtual archive packaging (`data.pak`).
* **🛠️ Native Debugger & DAG Viewer**: Built-in hot-key overlay for real-time `Blackboard` variable mutation, node jumping, runtime diagnostics, and interactive narrative tree visualization (`F1` / `F2`).
* **⏪ Time Machine Rollback**: Stack-based execution history (up to 128 steps) allowing seamless state and presentation rewinding via mouse wheel or Backspace.

---

## 🛠️ Tech Stack & Requirements

* **Language Standard**: C++17
* **Graphics & Windowing**: SFML 3.x (`sfml[core,window,graphics,audio]`)
* **Serialization & AST Parsing**: `nlohmann/json`
* **Shader Pipeline**: OpenGL / GLSL fragment shaders
* **Build System**: CMake (3.20+) / MSVC 2019+, GCC 10+, or Clang 11+
* **Toolchain Automation**: Python 3.8+

---

## 🧩 Gality VS Code Extension (`gality-language-support`)

To streamline narrative authoring and eliminate runtime syntax errors, Gality provides a dedicated VS Code extension located directly in the root directory as **`gality-language-support-1.0.0.vsix`**.

### Extension Capabilities

* **TextMate Syntax Highlighting**: Clean color scoping for directives (`@start`), node headers (`[node_id]`), transition flows (`->`), conditional logic (`IF/THEN/ELSE`), Blackboard variable mutations (`$ var += val`), and rich text tags.
* **Static Linter & Dangling Node Detection**: Automatically flags unresolved target nodes (`-> node_999`), duplicate node definitions, and mismatched rich text tags in real-time.
* **Go to Definition (`F12` / `Ctrl + Click`)**: Instantly jump from any transition pointer or conditional branch directly to the target `[node_id]` declaration.
* **IntelliSense Auto-Completion**: Context-aware suggestions for background paths, audio streams, character slots, and transition rules.

### How to Install the `.vsix` Extension

You can install the extension locally in seconds using either the command line or the VS Code UI:

#### Method A: Via Command Line (Recommended)

Open your terminal in the project root and run:

```powershell
code --install-extension gality-language-support-1.0.0.vsix

```

#### Method B: Via VS Code User Interface

1. Launch Visual Studio Code.
2. Open the **Extensions** view (`Ctrl + Shift + X` on Windows/Linux, `Cmd + Shift + X` on macOS).
3. Click on the **`...` (Views and More Actions)** menu at the top-right of the Extensions panel.
4. Select **`Install from VSIX...`**.
5. Browse to your project root directory, select `gality-language-support-1.0.0.vsix`, and click **Install**.

---

## 🚀 Getting Started

### 1. Prerequisites

Ensure you have CMake, a C++17 compliant compiler, and Python 3 installed.

```powershell
# Install dependencies via vcpkg
vcpkg install sfml nlohmann-json

```

### 2. Building from Source

```powershell
# 1. Compile DSL script to JSON AST
python devtools/scripts/gality_compiler.py assets/scripts/main_story_multi.gality assets/scripts/demo_long.json

# 2. Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# 3. Launch Engine
.\build\Release\Gality.exe

```

### 3. One-Click Build & Distribution

To compile the DSL script, pack encrypted assets into `data.pak`, build the Release executable, and assemble a standalone executable bundle under `.\dist`:

```powershell
.\build_game.bat

```

---

## ✍️ Writer & Creator Workflow

### `.gality` Script Example

```gality
@start node_01

[node_01]
bg: assets/bg/classroom.jpg
trans: diamond
duration: 1.5
bgm: assets/audio/bgm_daily.ogg
weather: sakura
旁白: 四月的春風掠過校園，窗外灑下一陣<color=#FFB7C5>櫻花雨</color>。
-> node_02

[node_02]
bg: assets/bg/classroom.jpg
char_center: assets/characters/senior_normal.png
cv: assets/audio/cv_01.ogg
學姐: 你終於準備好了嗎？我們的<color=#FF5555>遊戲開發計劃</color>今天必須確定下來。
-> choice_01

[choice_01]
? [包在我身上吧！今晚就能搞定！] -> act_favor_up
? [感覺好麻煩啊... 可以交給別人嗎？] -> act_favor_down

[act_favor_up]
$ favorability += 10
-> node_next

[cond_check_ending]
IF favorability >= 10 THEN node_good_ending ELSE node_normal_ending

```

---

## 🎮 In-Game Controls & Debug Shortcuts

* **Left Click / Enter / Space**: Advance dialogue / Confirm menu choice.
* **Mouse Wheel Up / Backspace**: Rollback execution state (Time Machine).
* **Tab / H / Wheel Up (in history)**: Open/Close History Backlog.
* **Escape**: System Settings Menu / Close active modal UI.
* **F1 or `~` (Tilde)**: Toggle native **DebugOverlay** (Blackboard, Node Jumper, Diagnostics).
* **F2**: Toggle narrative **NodeGraphViewer** (DAG Tree & Live Inspector).
* **F5 / F9**: Quick Save / Quick Load Slot UI.
* **K**: Trigger test screen shake.

---

## 📁 Repository Structure

```text
├── assets/                  # Media assets, fonts, UI themes, scripts, & shaders
│   ├── config/              # ui_theme.json layout settings
│   ├── scripts/             # .gality source DSL and JSON ASTs
│   └── shaders/             # GLSL fragment shaders
├── devtools/scripts/
│   ├── gality_compiler.py   # DSL to JSON compiler
│   └── gality_packer.py     # Asset encryption and packing tool
├── src/
│   ├── core/                # AssetPack (VFS), AudioManager, Blackboard, LRUCache, SaveManager
│   ├── render/              # UI, LayerRenderer, Weather, PostFX, TitleMenu, NodeGraphViewer
│   └── story/               # FSM Executor, ScriptLoader, StoryNode
├── build_game.bat           # Automated release packaging script
├── CMakeLists.txt           # Build configuration
└── gality-language-support-1.0.0.vsix  # Pre-built VS Code language extension

```

---

## 📜 License

Distributed under the MIT License. See `LICENSE` for more information.