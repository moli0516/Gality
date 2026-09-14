
# 🌸 Gality Engine

**Gality** is a high-performance, modern Galgame / Visual Novel engine engineered in **C++17** and **SFML 3.x**. Designed around a data-driven architecture, Gality completely decouples engine logic from content creation, allowing writers and artists to build rich narrative experiences using custom DSL scripts and JSON configurations without writing a single line of C++[cite: 1].

---

## ✨ Features

* **⚡ Ultra-Fast C++17 Core**: Native compiled execution with zero GC pauses, delivering minimal startup overhead and low memory consumption.
* **🎨 Data-Driven Architecture & UI Themes**: Easily skin dialogue boxes, name panels, typography, and choice menus via `assets/config/ui_theme.json`[cite: 1].
* **📜 Human-Friendly `.gality` DSL**: Writers write dialogue, branching logic, flags, and conditions in clean text files compiled down to JSON ASTs via Python[cite: 1].
* **💬 Rich Text & Typewriter Effects**: Native support for inline color tags (`<color=#HEX>`) with automatic word wrapping and audio typing feedback[cite: 1].
* **✨ GPU Visuals & Weather System**: Real-time particle systems for **Sakura**, **Rain**, and **Snow**, alongside Easing transitions for backgrounds and character sprites[cite: 1].
* **🌌 PostFX Shader Pipeline**: Integrated GLSL fragment shaders supporting real-time Gaussian Blur and dynamic screen shaking[cite: 1].
* **📦 Asset Management**: Built-in **LRU (Least Recently Used) Cache** for efficient memory recycling of textures and voice audio buffers.
* **🛠️ ImGui Debugger & Inspector**: In-game hot-key overlay for real-time `Blackboard` variable inspection and FSM Node Jumper.
* **🔐 Distribution Toolchain**: Includes asset encryption (`gality_packer.py`) and standard Release packaging automation (`build_game.bat`)[cite: 1].

---

## 🛠️ Tech Stack & Requirements

* **Language**: C++17
* **Graphics & Windowing**: SFML 3.x
* **GUI / Debugging**: Dear ImGui + imgui-sfml
* **Serialization**: `nlohmann::json`
* **Package Manager**: `vcpkg`
* **Build System**: CMake (3.20+)
* **Toolchain Scripts**: Python 3.8+

---

## 🚀 Getting Started

### 1. Prerequisites

Ensure you have CMake, a C++17 compliant compiler (MSVC 2019+, GCC 10+, or Clang 11+), and Python 3 installed.

```powershell
# Install dependencies via vcpkg
vcpkg install sfml nlohmann-json imgui imgui-sfml

```

### 2. Building from Source

```powershell
# 1. Compile DSL script to JSON
python gality_compiler.py assets/scripts/demo_long.gality assets/scripts/demo_long.json

# 2. Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# 3. Launch Engine
.\build\Release\Gality.exe

```

### 3. One-Click Build & Distribution

To compile the DSL script, pack assets, build the Release executable, and assemble a standalone executable bundle:

```powershell
.\build_game.bat

```

The distributable bundle will be output to the `.\dist` directory.

---

## ✍️ Writer & Creator Workflow

### `.gality` Script Example

```gality
@start node_01

[node_01]
bg: assets/bg/classroom.jpg
bgm: assets/audio/bgm_daily.ogg
weather: sakura
旁白: 放學後的陽光透過窗戶灑在課桌上，窗外飄著陣陣<color=#FFB7C5>櫻花雨</color>。
-> node_02

[node_02]
bg: assets/bg/classroom.jpg
char: assets/characters/senior_normal.png
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


* **Mouse Wheel Up / Tab / H**: Open/Close History Backlog.


* **F1 or `~**`: Toggle **Gality Engine Debugger & Inspector** (ImGui).


* **S / L**: Quick Save / Quick Load current state.



---

## 📁 Repository Structure

```text
├── assets/                  # Media assets, fonts, UI themes, scripts, & shaders
│   ├── config/              # ui_theme.json layout settings
│   ├── scripts/             # .gality source DSL and JSON ASTs
│   └── shaders/             # GLSL fragment shaders
├── devtools / scripts/
│   ├── gality_compiler.py   # DSL to JSON compiler
│   └── gality_packer.py     # Asset encryption and packing tool
├── src/
│   ├── core/                # LRU Cache, Easing, Blackboard, SaveManager
│   ├── render/              # UI, LayerRenderer, Weather, PostFX, TitleMenu
│   └── story/               # FSM Executor, ScriptLoader, StoryNode
├── build_game.bat           # Automated release packaging script
└── CMakeLists.txt           # Build configuration

```

---

## 📜 License

Distributed under the MIT License. See `LICENSE` for more information.
