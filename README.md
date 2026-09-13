# Gality Visual Novel Engine

**Gality** is a lightweight, high-performance, custom C++17 visual novel engine built on **SFML 3.x**. Designed with a clear modular architecture, memory efficiency, and state-machine-driven narrative flow, Gality provides a seamless pipeline from human-readable narrative scripts to execution.

---

## Key Features

* **C++17 & SFML 3.x Native**: Ultra-fast startup, high rendering efficiency, and minimal CPU/RAM overhead.
* **FSM-Based Execution**: Core story logic driven by a Finite State Machine supporting Dialogue, Choice Branching, Action Execution, and Condition Evaluation[cite: 1, 2].
* **LRU Asset Cache**: Custom Least Recently Used (LRU) cache for textures and audio buffers to prevent memory leaks and manage RAM/VRAM usage efficiently[cite: 1, 2].
* **State Snapshot & Backlog System**: Supports real-time dialogue history tracking and state rollback[cite: 1].
* **Smooth Transitions (Week 7)**:
  * **Visual Crossfade**: Alpha transparency transitions powered by custom `Easing` interpolation algorithms[cite: 1].
  * **BGM Crossfade**: Dual-track background music smooth fade-in / fade-out transitions[cite: 1].
* **Proprietary DSL & Compiler (Week 8)**: Custom `.gality` domain-specific language for rapid scriptwriting, complete with a Python-based compiler to output JSON ASTs[cite: 1, 2].

---

## Project Structure

```

Gality/
├── assets/
│   ├── audio/          # Background music (.ogg) and voice clips (.ogg)
│   ├── bg/             # Scene background images (.jpg / .png)
│   ├── characters/     # Character sprites (.png)
│   ├── fonts/          # TrueType fonts (.ttf)
│   └── scripts/        # DSL (.gality) and compiled JSON (.json) scripts
├── saves/              # Save slot JSON files
├── src/
│   ├── core/           # AudioManager, Blackboard, LRUCache, SaveManager, Easing
│   ├── render/         # DialogueBox, ChoiceUI, LayerRenderer, BacklogUI
│   ├── story/          # ScriptLoader, StoryExecutor, StoryNode
│   └── main.cpp        # Application entry point & main loop
├── CMakeLists.txt
├── gality_compiler.py  # Python compiler translating .gality to .json
├── vcpkg.json          # Dependency management
└── README.md

```

---

## Tech Stack & Dependencies

* **Language**: C++17
* **Graphics & Audio**: SFML 3.x
* **Data Interchange**: `nlohmann/json`
* **Build System**: CMake (3.15+) & vcpkg
* **Script Compiler**: Python 3.x

---

## Getting Started

### Prerequisites

* C++17 compatible compiler (MSVC 2019+, GCC 9+, or Clang 10+)
* CMake (v3.15 or higher)
* Python 3.x (for script compilation)[cite: 1, 2]
* vcpkg package manager

### Building the Engine

1. **Clone the Repository**:
   ```powershell
   git clone https://github.com/moli0516/Gality.git
   cd Gality

   ```

```

2. **Configure with CMake**:
```powershell
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="[path-to-vcpkg]/scripts/buildsystems/vcpkg.cmake"

```

3. **Build the Project**:

```powershell
cmake --build build --config Debug

```

4. **Run Gality**:

```powershell
.\build\Debug\Gality.exe

```

---

## Authoring Scripts (.gality DSL)

Gality allows writers to author stories using a simple, human-readable DSL syntax instead of writing JSON manually.

### DSL Syntax Example (`assets/scripts/demo.gality`)

```text
@start node_01

[node_01]
bg: assets/bg/classroom.jpg
bgm: assets/audio/bgm_daily.ogg
Narrator: The sun shines through the classroom window after school.
-> node_02

[node_02]
char: assets/characters/senior_normal.png
cv: assets/audio/cv_01.ogg
Senior: Are you ready to finalize our game engine architecture today?
-> choice_01

[choice_01]
? [Leave it to me! I'll finish it tonight.] -> act_favor_up
? [Sounds troublesome... Can someone else do it?] -> act_favor_down

[act_favor_up]
$ favorability += 10
-> node_03

[act_favor_down]
$ favorability += -5
-> node_03

[node_03]
IF favorability >= 10 THEN node_good ELSE node_norm

[node_good]
Senior: Reliable as always!
-> END

[node_norm]
Senior: Sigh, stop being so lazy...
-> END

```

### Compiling Scripts to JSON

To compile a `.gality` file into an engine-readable `.json` file:

```powershell
python gality_compiler.py assets/scripts/demo.gality assets/scripts/demo_long.json

```

---

## Controls

* **Left Click / Enter / Space**: Advance dialogue / Confirm choice.
* **Mouse Wheel Up / Tab / H**: Open Dialogue History (Backlog).
* **Mouse Wheel Down / Right Click / Esc**: Close Backlog.
* **S Key**: Quick Save game state to `saves/save1.json`.
* **L Key**: Quick Load game state from `saves/save1.json`.

---

## License

This project is licensed under the MIT License - see the LICENSE file for details.
