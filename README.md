

# Gality Engine

A lightweight, data-driven Visual Novel (Galgame) engine built from scratch in **C++17** using **SFML 3.x**.

Designed with modern systems architecture in mind, Gality bypasses heavy commercial engines to explore low-level engine integration, finite state machines, dynamic layer rendering, and custom state serialization.

---

## Key Features

* **Data-Driven Narrative FSM:** Write complex branching storylines, conditional routing, and global variable updates entirely in plain JSON scripts.
* **Blackboard State Management:** A centralized key-value store managing player choices, favorability scores, and global flags to dynamically trigger multi-ending routes.
* **Layer-Based Renderer & Asset Cache:** Automated z-indexed rendering for backgrounds, character sprites, and UI elements with built-in texture caching.
* **Audio Pipeline:** Stream persistent BGM across scene transitions and trigger low-latency single-shot voice acting (CV) audio.
* **State Serialization (Save/Load):** Fully serializable game snapshots allowing full runtime restoration of node execution pointers and Blackboard state via JSON.
* **Authentic Visual Novel UI:** Smooth frame-independent typewriter text effect, CJK UTF-8/UTF-32 text rendering, and interactive choice menus with hover state detection.

---

## Tech Stack

* **Core Language:** C++17 (`std::optional`, `std::shared_ptr`, `std::function`)
* **Graphics & Audio:** SFML 3.0.2
* **Serialization & Scripting:** `nlohmann/json`
* **Build System:** CMake (3.20+)
* **Package Management:** `vcpkg` (Manifest Mode)

---

## Project Structure

```text
Gality/
├── assets/
│   ├── audio/        # BGM (.ogg) and Voice Acting (.ogg/.wav)
│   ├── bg/           # Background textures (.jpg/.png)
│   ├── characters/   # Transparent character sprites (.png)
│   ├── fonts/        # CJK TrueType fonts (e.g., font.ttf)
│   └── scripts/      # Narrative JSON scripts (demo_long.json)
├── saves/            # Generated JSON save state snapshots
├── src/
│   ├── core/         # Blackboard, AudioManager, SaveManager
│   ├── render/       # DialogueBox, ChoiceUI, LayerRenderer
│   ├── story/        # StoryNode, StoryExecutor, ScriptLoader
│   └── main.cpp      # Event loop & subsystem coordination
├── vcpkg.json        # Dependencies manifest
└── CMakeLists.txt    # Modern CMake configuration

```

---

## Quick Start (Build & Run)

### Prerequisites

* **Compiler:** MSVC (Visual Studio 2022 recommended with C++17 support)
* **Build Tools:** CMake (3.20+)
* **Package Manager:** `vcpkg`

### Build Instructions

1. **Clone the repository:**
```powershell
git clone https://github.com/moli0516/Gality.git
cd Gality

```


2. **Configure CMake with vcpkg integration:**
```powershell
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake"

```


3. **Compile the engine:**
```powershell
cmake --build build --config Debug

```


4. **Ensure Assets & Run:**
Make sure a CJK-compatible TrueType font is placed at `assets/fonts/font.ttf`, then run:
```powershell
.\build\Debug\Gality.exe

```



---

## Controls

| Key / Mouse Action | Function |
| --- | --- |
| **Left Click / Space / Enter** | Advance Dialogue / Complete Typewriter Effect |
| **Mouse Hover + Click** | Select Branching Choice Options |
| **`S` Key** | Save Snapshot to `saves/save1.json` |
| **`L` Key** | Load Snapshot from `saves/save1.json` |

---

## Architecture Overview

Detailed system flow and decoupling strategies are documented in [ARCHITECTURE.md](ARCHITECTURE.md).

```text
[ ScriptLoader ] ---> [ StoryNode Graph ] 
                             |
[ Input Events ] ---> [ StoryExecutor ] <---> [ Blackboard State ]
                             |
                             v
                 +-----------------------+
                 |  syncCurrentNodeState |
                 +-----------------------+
                   /         |         \
                  v          v          v
          [LayerRenderer] [DialogueBox] [AudioManager]

```

---

## Roadmap

* [x] **Phase 1:** Core Logic (FSM Graph, Blackboard State, JSON Script Loader)
* [x] **Phase 2:** Visuals & UI (SFML 3.x Window, Typewriter Text Engine, Layered Renderer, Interactive Choice Buttons)
* [x] **Phase 3:** Audio Engine (SFML Audio BGM Looping & Single-shot Voice Stream Control)
* [x] **Phase 4:** State Persistence (JSON Serialization, Save/Load Snapshot Restoration)