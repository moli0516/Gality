# Gality Engine - System Architecture Specification

This document details the software architecture, data pipelines, and design patterns used in the **Gality Galgame Engine**. The core design philosophy prioritizes modularity, single-responsibility components, data-driven control flow, and automated state management.

---

## High-Level Architecture Overview

Gality separates storytelling logic from graphics, user input, and state management. The engine is divided into four major sub-systems:

```text
               +----------------------------------+
               |        demo_long.json            |
               +----------------------------------+
                                |
                                v
                       [ ScriptLoader ]
                                |
                                v
                    [ StoryNode Network ]
                                |
                                v
  [ Player Input ] ---> [ StoryExecutor ] <---> [ Blackboard ]
                                |               (State Snapshot)
                                v
               +----------------------------------+
               |         Sync State Event         |
               +----------------------------------+
                 /              |               \
                v               v                v
        [LayerRenderer]   [DialogueBox]    [AudioManager]
        (Texture Cache)   (Typewriter)     (BGM / CV Streaming)
                \               |               /
                 v              v              v
               +----------------------------------+
               |        sf::RenderWindow          |
               +----------------------------------+

```

---

## Core Systems & Component Responsibilities

### 1. Narrative Finite State Machine (FSM)

* **`StoryNode`**: The fundamental atomic unit representing a state in the narrative graph. Nodes are typed (`Dialogue`, `Choice`, `Action`, `Condition`) and contain properties for text, speakers, texture paths, and branching pointers.
* **`ScriptLoader`**: A two-pass parsing system using `nlohmann/json`. Pass 1 constructs all `StoryNode` memory instances. Pass 2 links graph nodes (`defaultNext`, `nextNode` for choices, and dynamic lambda conditions).
* **`StoryExecutor`**: The main execution engine. It evaluates node transitions, executes action lambdas (e.g., modifying favorability), handles conditional branches automatically, and yields execution when encountering interactive states (`Dialogue` or `Choice`).

### 2. State & Memory Management

* **`Blackboard`**: Centralized key-value storage (`std::unordered_map<std::string, int>`) holding global game flags, branching conditions, and character favorability scores.
* **`SaveManager`**: Handles serialization and deserialization. State snapshots write the active `currentNodeId` and full `Blackboard` state to `saves/save1.json`. On load, `SaveManager` restores global flags and relocates the execution pointer via an indexed `nodeIndexMap`.

### 3. Rendering & Presentation Layer

* **`LayerRenderer`**: Manages visual assets (Backgrounds and Character Sprites). Utilizes `std::optional<sf::Sprite>` to support SFML 3.x explicit texture binding and handles runtime texture caching in memory.
* **`DialogueBox`**: Encapsulates text rendering UI, UTF-8 to UTF-32 conversion, and a frame-rate independent typewriter text reveal algorithm using time deltas.
* **`ChoiceUI`**: Dynamically calculates bounds and positions for branching option buttons, handling hover state changes and mouse raycasting.

### 4. Audio Engine

* **`AudioManager`**: Encapsulates audio streams and buffer caching. Uses `sf::Music` for persistent, non-blocking BGM looping across node transitions and `sf::SoundBuffer`/`sf::Sound` for low-latency single-shot Voice Acting (CV) triggers.

---

## Data Flow Pipeline

### Execution & Rendering Loop

1. **Input Phase**: `sf::RenderWindow` captures window polling events (`MouseButtonPressed`, `KeyPressed`). Global shortcuts (`S` for Save, `L` for Load) are intercepted.
2. **Logic Phase**:
* If on a `Dialogue` node, user interaction completes the typewriter effect or calls `StoryExecutor::advance()`.
* If on a `Choice` node, mouse hovering updates button highlights, and clicking triggers `StoryExecutor::advance(choiceIndex)`.


3. **State Evaluation Phase**: `StoryExecutor` updates the node pointer and evaluates transient nodes (`Action`, `Condition`) synchronously until arriving at the next visual node.
4. **Synchronization Phase**: `syncCurrentNodeState()` executes:
* Updates `DialogueBox` with speaker and target text.
* Triggers `LayerRenderer::setBackground()` and `setCharacter()`.
* Calls `AudioManager::playBGM()` (retaining continuous audio if path is unchanged) and `playVoice()`.


5. **Render Phase**: Backbuffer is cleared, followed by sequential drawing of layers: `LayerRenderer` -> `DialogueBox` -> `ChoiceUI`. Window displays frame.

---

## Key Design Patterns Applied

* **Data-Driven Design**: Narrative flow, visual assets, audio paths, and branching logic exist external to code in JSON scripts. Modifying story content requires zero re-compilation.
* **Blackboard Pattern**: Decouples independent game mechanics from narrative logic, allowing conditions to query global state without direct dependencies.
* **Flyweight / Asset Caching**: `LayerRenderer` and `AudioManager` maintain `std::unordered_map` resource caches, preventing duplicate disk reads and reducing I/O latency during scene transitions.