# Gality Engine

A lightweight, data-driven Visual Novel (Galgame) engine built from scratch in C++17. Designed as a deep-dive into system architecture, Gality bypasses heavy commercial engines in favor of low-level integration, finite state machines, and dynamic asset rendering.

## Core Features

* **JSON-Driven Narrative FSM:** Write branching storylines, conditional logic, and action triggers entirely in plain JSON. No hardcoded logic required.
* **Blackboard State Management:** A centralized variable manager that tracks player choices, favorability scores, and global flags to trigger multi-ending routes.
* **Dynamic Layer Rendering:** Automated z-indexing and asset caching for backgrounds, character sprites, and UI elements using SFML 3.x.
* **Authentic Visual Novel UI:** Features a smooth typewriter text effect, UTF-8 text parsing, and interactive branching choice menus with hover state detection.
* **Modern C++ Architecture:** Header-heavy, modular design utilizing C++17 features (`std::optional`, `std::shared_ptr`, `std::function`).

## Tech Stack

* **Language:** C++17
* **Graphics & Windowing:** SFML 3.0.2
* **Serialization:** nlohmann/json
* **Build System:** CMake
* **Package Manager:** vcpkg

## Project Structure

**Plaintext**

```
Gality/
├── assets/
│   ├── bg/           # Background images (.jpg/.png)
│   ├── characters/   # Transparent character sprites (.png)
│   ├── fonts/        # CJK TrueType fonts (e.g., font.ttf)
│   └── scripts/      # JSON story scripts
├── src/
│   ├── core/         # Core engine logic (Blackboard.hpp)
│   ├── render/       # UI & Graphics (DialogueBox.hpp, ChoiceUI.hpp, LayerRenderer.hpp)
│   ├── story/        # Narrative FSM (StoryNode.hpp, StoryExecutor.hpp, ScriptLoader.hpp)
│   └── main.cpp      # Engine entry point and main loop
├── vcpkg.json        # vcpkg manifest for dependencies
└── CMakeLists.txt    # Build configuration
```

## Build Instructions (Windows / MSVC)

Gality uses `vcpkg` in manifest mode to automatically download and build dependencies (SFML 3.x, nlohmann\_json).

### Prerequisites

* Visual Studio 2022 (with Desktop development with C++)
* CMake (3.20+)
* vcpkg

### Compilation

1. Clone the repository and ensure your `vcpkg` path is correct.
2. Open PowerShell in the project root directory and configure the project:

**PowerShell**

```
# Replace the path with your actual vcpkg installation path
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake"
```

3. Build the executable:

**PowerShell**

```
cmake --build build
```

4. Run the engine:

**PowerShell**

```
.\build\Debug\Gality.exe
```

*Note: Ensure a valid CJK font is placed at `assets/fonts/font.ttf` before running, otherwise the UI will fail to initialize.*

## Scripting Guide

Story progression is controlled via `assets/scripts/demo.json`. The engine parses four types of nodes:

* `dialogue`: Renders text, speaker names, backgrounds, and character sprites.
* `choice`: Halts execution and presents clickable branching options.
* `action`: Modifies integer or boolean flags in the Blackboard (e.g., adding favorability).
* `condition`: Evaluates Blackboard flags to route the story to different next nodes (e.g., Good End vs. Normal End).

**Example Node:**

**JSON**

```
{
  "id": "node_01",
  "type": "dialogue",
  "speaker": "Senpai",
  "text": "Welcome to the programming club!",
  "bg": "assets/bg/clubroom.jpg",
  "character": "assets/characters/senior.png",
  "next": "choice_01"
}
```

## Roadmap

* [X]  **Week 1:** Core Logic (FSM, Blackboard, JSON Parsing)
* [X]  **Week 2:** Visuals & UI (SFML 3.x Window, Typewriter Effect, Layer Rendering, Choice UI)
* [ ]  **Week 3:** Audio System (BGM loop, Voice Acting integration via SFML Audio or SoLoud)
* [ ]  **Week 4:** Save/Load System (JSON state serialization and snapshot restoration)
