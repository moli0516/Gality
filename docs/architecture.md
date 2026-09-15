
# Gality Engine — System Architecture Specification

> **Version 1.0**
> *For engineers who want to understand why, not just how.*

This document describes the software architecture, data pipelines, and design patterns of **Gality Engine**. The design philosophy is built on four pillars:

1. **Data-Driven Design** — Story, assets, and logic live outside the binary.
2. **Single Responsibility** — Every class does exactly one thing.
3. **Zero Hidden State** — The FSM is a `while` loop you can read in one sitting.
4. **No Garbage Collection** — RAII everywhere. Deterministic destruction.

If you've ever read a game engine's source and thought *"why is this 40,000 files?"* — this document is for you.

---

## 1. High-Level Architecture Overview

Gality separates **storytelling logic** from **graphics, input, and state management**. The engine is divided into four subsystems:

```
                    ┌──────────────────────────────┐
                    │      demo_long.json          │
                    │  (Compiled from .gality DSL) │
                    └──────────────┬───────────────┘
                                   │
                                   ▼
                    ┌──────────────────────────────┐
                    │        ScriptLoader          │
                    │  Pass 1: Construct nodes     │
                    │  Pass 2: Link pointers       │
                    └──────────────┬───────────────┘
                                   │
                                   ▼
                    ┌──────────────────────────────┐
                    │    StoryNode Network (DAG)   │
                    │  Dialogue / Choice / Action  │
                    │  / Condition                 │
                    └──────────────┬───────────────┘
                                   │
        ┌──────────────────────────┼──────────────────────────┐
        │                          │                          │
        ▼                          ▼                          ▼
┌───────────────┐          ┌───────────────┐          ┌───────────────┐
│ Player Input  │◀────────▶│ StoryExecutor │◀────────▶│  Blackboard   │
│ (SFML events) │          │  (FSM loop)   │          │  (state)      │
└───────────────┘          └───────┬───────┘          └───────────────┘
                                   │
                                   ▼
                    ┌──────────────────────────────┐
                    │    syncCurrentNodeState()    │
                    │   (Presentation sync point)  │
                    └──────────────┬───────────────┘
                                   │
        ┌──────────────────────────┼──────────────────────────┐
        │                          │                          │
        ▼                          ▼                          ▼
┌───────────────┐          ┌───────────────┐          ┌───────────────┐
│LayerRenderer  │          │ DialogueBox   │          │ AudioManager  │
│(texture cache)│          │ (typewriter)  │          │ (BGM + CV)    │
└───────┬───────┘          └───────┬───────┘          └───────┬───────┘
        │                          │                          │
        └──────────────────────────┼──────────────────────────┘
                                   │
                                   ▼
                    ┌──────────────────────────────┐
                    │      sf::RenderWindow        │
                    │   (letterboxed to 16:9)      │
                    └──────────────────────────────┘
```

### The Single Source of Truth

Every node transition flows through **one function**:

```cpp
void GalityApp::syncCurrentNodeState(bool recordHistory, bool playTransition);
```

This is the **only place** where the following happen:
- `DialogueBox::setText()` is called
- `LayerRenderer::setBackground()` is called
- `LayerRenderer::updateCharacters()` is called
- `AudioManager::playBGM()` is called
- `AudioManager::playVoice()` is called
- `WeatherSystem::setWeather()` is called

**If you want to add a new presentation element, you add it here.** Nowhere else.

---

## 2. Core Systems

### 2.1 Narrative Finite State Machine (FSM)

#### `StoryNode` — The Atomic Unit

A `StoryNode` represents **one state** in the narrative graph. Nodes are typed:

| Type | Purpose | Has Text? | Has Choices? | Has Conditions? |
|---|---|---|---|---|
| **Dialogue** | Display text, wait for input | ✅ | ❌ | ❌ |
| **Choice** | Present branching options | ❌ | ✅ | ❌ |
| **Action** | Mutate Blackboard, pass through | ❌ | ❌ | ❌ |
| **Condition** | Branch based on variables | ❌ | ❌ | ✅ |

**Key fields:**

```cpp
class StoryNode {
public:
    std::string id;
    NodeType type;
    
    // Presentation
    std::string speaker;
    std::string text;
    std::string bgImagePath;
    std::string bgmPath;
    std::string voicePath;
    std::map<CharSlot, std::string> slotTextures;
    std::optional<CharSlot> activeSlot;
    std::string weather;
    float shake;
    std::string transitionMask;
    float transitionDuration;
    
    // Graph topology
    std::vector<ChoiceOption> choices;
    std::shared_ptr<StoryNode> defaultNext;    // Pre-linked during Pass 2
    std::shared_ptr<StoryNode> nextNode;
    
    // Blackboard integration
    std::vector<VariableMutation> mutations;
    std::function<void(Blackboard&)> actionFunc;
    
    // Conditional branching
    std::string thenNodeId;
    std::string elseNodeId;
    std::function<std::shared_ptr<StoryNode>(Blackboard&)> conditionFunc;
};
```

**Design note:** `actionFunc` and `conditionFunc` are lambdas bound at load time. This means the executor never has to switch on `NodeType` in the hot path — it just calls the function.

#### `ScriptLoader` — Two-Pass Construction

The loader reads JSON and builds a fully-linked DAG:

**Pass 1 — Construction:**
- Iterate `nodes[]` array
- Create `shared_ptr<StoryNode>` for each
- Parse all fields into the node
- Bind `actionFunc` lambdas for `Action` nodes
- Bind `conditionFunc` lambdas for `Condition` nodes

**Pass 2 — Linking:**
- Iterate the registry again
- For every `nextNodeId`, resolve to `shared_ptr<StoryNode>`
- For every `ChoiceOption.targetNodeId`, resolve to `shared_ptr<StoryNode>`
- Store resolved pointers in `defaultNext` and `choice.nextNode`

**Why two passes?** Because during Pass 1, not all nodes exist yet. Pass 2 assumes the registry is complete.

#### `StoryExecutor` — The FSM Loop

The executor is intentionally **tiny**. Its entire job is:

```cpp
void StoryExecutor::evaluateCurrentNode() {
    while (currentNode) {
        if (currentNode->type == NodeType::Action) {
            if (currentNode->actionFunc) {
                currentNode->actionFunc(blackboard);
            }
            currentNode = currentNode->defaultNext;
        } else if (currentNode->type == NodeType::Condition) {
            if (currentNode->conditionFunc) {
                currentNode = currentNode->conditionFunc(blackboard);
            } else {
                currentNode = currentNode->defaultNext;
            }
        } else {
            // Dialogue or Choice — stop here, wait for player input
            break;
        }
    }
}
```

**Key insight:** `Action` and `Condition` nodes are **transient**. They never render. They are evaluated synchronously and skipped in the same frame.

This means:
- 100 chained `Action` nodes execute in **microseconds**
- The player **never sees** them
- No visual flicker, no frame skip

---

### 2.2 State Management

#### `Blackboard` — Global Variable Store

```cpp
class Blackboard {
private:
    std::unordered_map<std::string, int> intVars;
    std::unordered_map<std::string, bool> boolVars;

public:
    void setInt(const std::string& key, int value);
    int getInt(const std::string& key, int defaultValue = 0) const;
    void addInt(const std::string& key, int delta);
    
    const std::unordered_map<std::string, int>& getAllInts() const;
    void setAllInts(const std::unordered_map<std::string, int>& flags);
};
```

**Design decisions:**

- **Integer-only by default.** Visual novels rarely need floats. Integers are deterministic and easy to serialize.
- **Bool overload available.** For flags like `has_seen_ending_1`.
- **No type checking.** If a script writes `$ favorability += 10`, the engine trusts it. The compiler catches typos.
- **Snapshot-friendly.** `getAllInts()` returns a copy of the entire map. This is used by `SaveManager` and `StoryExecutor::recordSnapshot()`.

#### `SaveManager` — Deterministic Serialization

Save files are **JSON**, not binary. Why?

1. **Debuggable.** You can open a save in a text editor.
2. **Forward-compatible.** New variables get default values.
3. **Diff-able.** `git diff` on saves actually works.

**Snapshot structure:**

```cpp
struct SaveSnapshot {
    std::string currentNodeId;                          // Where we are
    std::unordered_map<std::string, int> intFlags;      // All Blackboard vars
    std::string bgImagePath;                            // Current BG
    std::map<CharSlot, std::string> slotTextures;       // Current sprites
    std::optional<CharSlot> activeSlot;                 // Who's talking
    std::string weather;                                // Current weather
    std::string bgmPath;                                // Current BGM
    std::string timestamp;                              // For UI display
};
```

**On load:**
1. Restore Blackboard via `setAllInts()`
2. Jump executor to `nodeIndexMap[currentNodeId]`
3. Rebuild presentation state from snapshot

**Time Machine Rollback** uses the same structure but stores snapshots in a `std::vector<SaveSnapshot>` (max 128 entries).

---

### 2.3 Rendering Layer

#### `LayerRenderer` — Stage Manager

```cpp
class LayerRenderer {
private:
    LRUCache<std::string, std::shared_ptr<sf::Texture>>* textureCachePtr;
    sf::Texture emptyTexture;
    sf::Sprite bgSprite;
    std::map<CharSlot, sf::Sprite> characterSprites;
    std::map<CharSlot, std::string> currentSlotPaths;
};
```

**Responsibilities:**
- Load textures via LRU cache
- Scale background to fit 1920×1080
- Scale character sprites to 95% of screen height
- Position character slots at 25% / 50% / 75%
- Apply `active_char` dimming (non-speakers get `140, 140, 150` tint)

**Character auto-scaling:**

```cpp
float targetHeight = baselineY * 0.95f;
float scale = targetHeight / sprite.getLocalBounds().size.y;
sprite.setScale({scale, scale});
sprite.setOrigin({bounds.size.x * 0.5f, bounds.size.y});
sprite.setPosition({slotCenterX, baselineY});
```

This means **any sprite size works**. You can drop in 512×1024 or 2048×4096 sprites without touching code.

#### `DialogueBox` — Text Rendering Engine

This is the **most complex class** in the engine. It handles:

1. **UTF-8 → UTF-32 conversion** (for correct CJK glyph iteration)
2. **Inline tag parsing** (`<color>`, `<shake>`, `<speed>`, `<w>`)
3. **CJK Kinsoku Shori** (避頭尾禁則 line-breaking rules)
4. **Frame-rate independent typewriter** (using `sf::Clock` deltas)
5. **Per-glyph styling** (each character can have its own color, shake flag, speed)

**Data flow:**

```
Raw string
    ↓
parseInlineTags()
    ↓
vector<ParsedChar> (codepoint + style + pause)
    ↓
buildLayout()
    ↓
vector<FormattedGlyph> (codepoint + position + style)
    ↓
draw() (only first N glyphs, based on typewriter progress)
```

**Kinsoku Shori implementation:**

```cpp
static bool isProhibitedAtLineStart(char32_t cp) {
    static const std::unordered_set<char32_t> prohibited = {
        U'，', U'。', U'！', U'？', U'：', U'；', U'、',
        U'）', U'」', U'』', U'”', U'’', U'…', U'—'
    };
    return prohibited.find(cp) != prohibited.end();
}
```

When laying out text, if the next character would be a prohibited-at-start char, we **wrap early**. This prevents ugly line breaks like:

```
學姐說：「你今天來的
，很準時。」
```

Instead:

```
學姐說：「你今天來的，
很準時。」
```

---

### 2.4 Audio Engine

#### `AudioManager` — BGM + CV Streaming

Two audio paths:

| Type | SFML Class | Lifetime | Use Case |
|---|---|---|---|
| **BGM** | `sf::Music` | Persistent, loops | Background music |
| **CV** | `sf::SoundBuffer` + `sf::Sound` | One-shot | Voice lines |

**Critical design decision:** BGM is loaded **from memory**, not from file:

```cpp
std::vector<std::uint8_t> activeBgmBytes;   // Persistent buffer
sf::Music activeBgm;

bool fromPak = AssetPack::readFileFromPak(audioPath, activeBgmBytes);
activeBgm.openFromMemory(activeBgmBytes.data(), activeBgmBytes.size());
```

**Why?** Because `sf::Music` streams asynchronously. If the underlying buffer is freed while streaming, you get a **dangling pointer crash**. By keeping `activeBgmBytes` alive, we guarantee the stream is always valid.

**Crossfading:**

```cpp
sf::Music activeBgm;    // New BGM
sf::Music fadingBgm;    // Old BGM (fading out)
std::vector<std::uint8_t> activeBgmBytes;
std::vector<std::uint8_t> fadingBgmBytes;
```

When switching BGM:
1. Move `activeBgmBytes` → `fadingBgmBytes`
2. Open `fadingBgm` with old bytes, play at full volume
3. Load new bytes into `activeBgmBytes`
4. Open `activeBgm`, start at volume 0
5. Update loop interpolates volumes over `fadeDuration`

**Smart ducking:**

When a voice line plays, BGM volume drops to 65%. When voice ends, BGM returns to 100%. Interpolation is exponential (`factor += (target - factor) * deltaTime * 6.0f`).

**Spatial panning:**

```cpp
float posX = 0.0f;
switch (slot.value()) {
    case CharSlot::Left:   posX = -1.0f; break;
    case CharSlot::Center: posX =  0.0f; break;
    case CharSlot::Right:  posX =  1.0f; break;
}
voiceSound->setPosition({posX, 0.0f, 0.0f});
```

Voice lines pan left/right based on `active_char`. Subtle, but effective.

---

## 3. Data Flow Pipeline

### Frame-by-Frame Execution

```
┌─────────────────────────────────────────────────────────────┐
│ Frame N                                                      │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  1. deltaTime = clock.restart()                             │
│                                                             │
│  2. handleEvents()                                          │
│     ├─ SFML pollEvent()                                     │
│     ├─ UI hit-testing (SaveLoadUI, SettingsUI, etc.)        │
│     ├─ If Dialogue: dialogueBox.onInteract() or advance()   │
│     └─ If Choice: choiceUI.handleMouseClick()               │
│                                                             │
│  3. update(deltaTime)                                       │
│     ├─ hotReloader->update()  (desktop only)                │
│     ├─ audioManager.update()  (crossfade + ducking)         │
│     ├─ layerRenderer.update() (no-op currently)             │
│     ├─ weatherSystem.update() (particle physics)            │
│     ├─ transitionSystem.update() (mask progress)            │
│     ├─ postFX.update() (blur + shake offsets)               │
│     └─ dialogueBox.update() (typewriter progress)           │
│                                                             │
│  4. render()                                                │
│     ├─ sceneBuffer.clear()                                  │
│     ├─ layerRenderer.draw(sceneBuffer)                      │
│     ├─ dialogueBox.draw(sceneBuffer)                        │
│     ├─ choiceUI.draw(sceneBuffer)                           │
│     ├─ weatherSystem.draw(sceneBuffer)                      │
│     ├─ sceneBuffer.display()                                │
│     ├─ transitionSystem.draw(compositeBuffer, sceneBuffer)  │
│     ├─ compositeBuffer.display()                            │
│     ├─ window.clear()                                       │
│     ├─ postFX.applyAndDraw(window, compositeBuffer)         │
│     ├─ titleMenu.draw(window)                               │
│     ├─ saveLoadUI.draw(window)                              │
│     ├─ backlogUI.draw(window)                               │
│     ├─ settingsUI.draw(window)                              │
│     ├─ debugOverlay.draw(window)                            │
│     ├─ nodeGraphViewer.draw(window)                         │
│     └─ window.display()                                     │
│                                                             │
│  5. Goto Frame N+1                                          │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### Node Transition Flow

When the player advances past a `Dialogue` node:

```
Player clicks
    ↓
dialogueBox.onInteract() returns true (typewriter complete)
    ↓
executor.advance() called
    ↓
If Dialogue: currentNode = currentNode->defaultNext
If Choice:   currentNode = currentNode->choices[i].nextNode
    ↓
executor.evaluateCurrentNode() called
    ↓
While currentNode is Action or Condition:
    - Action: execute actionFunc(blackboard), skip
    - Condition: evaluate conditionFunc(blackboard), branch
    ↓
currentNode is now a Dialogue or Choice
    ↓
syncCurrentNodeState() called
    ↓
Presentation layers updated
```

**The entire chain is synchronous.** No promises, no async, no callbacks. Just a `while` loop.

---

## 4. Key Design Patterns

### 4.1 Data-Driven Design

**The problem:** Hard-coded story logic means recompiling for every dialogue change.

**Gality's solution:** Story lives in `.gality` files. Compiler produces JSON. Engine reads JSON at startup.

**Result:** Writers iterate without touching C++. Engine developers iterate without touching story.

### 4.2 Blackboard Pattern

**The problem:** Coupling narrative logic to game state creates spaghetti.

**Gality's solution:** All state lives in `Blackboard`. Conditions and Actions query/mutate it via string keys.

**Result:** New variables require zero engine changes. `$ new_flag += 1` just works.

### 4.3 Flyweight / Asset Caching

**The problem:** Loading the same texture twice wastes RAM and I/O.

**Gality's solution:** `LRUCache<std::string, std::shared_ptr<sf::Texture>>` with capacity 50.

**Result:** Assets are loaded once, shared by pointer, and evicted when unused. Memory stays bounded even with 1000+ unique textures.

**LRU implementation:**

```cpp
template <typename Key, typename Value>
class LRUCache {
private:
    size_t capacity;
    std::list<std::pair<Key, Value>> itemsList;      // MRU at front
    std::unordered_map<Key, ListIterator> itemsMap;  // O(1) lookup

public:
    Value get(const Key& key) {
        auto it = itemsMap.find(key);
        if (it == itemsMap.end()) throw std::runtime_error("Not found");
        itemsList.splice(itemsList.begin(), itemsList, it->second);  // Move to front
        return it->second->second;
    }

    void put(const Key& key, const Value& value) {
        if (itemsList.size() >= capacity) {
            auto last = itemsList.back();
            itemsMap.erase(last.first);   // Evict LRU
            itemsList.pop_back();
        }
        itemsList.push_front({key, value});
        itemsMap[key] = itemsList.begin();
    }
};
```

**Why `std::list` + `std::unordered_map`?**
- `std::list` gives O(1) splice (move to front)
- `std::unordered_map` gives O(1) lookup by key
- Together: O(1) get, put, and evict

### 4.4 Pure Vector UI

**The problem:** Every GUI framework adds 10+ MB and a second rendering pipeline.

**Gality's solution:** All UI is drawn with `sf::RectangleShape`, `sf::Text`, and `sf::VertexArray`.

**Result:**
- **Zero external dependencies** beyond SFML
- **Fully skinnable** via `ui_theme.json`
- **No second renderer** to keep in sync
- **Sub-millisecond UI updates**

---

## 5. Thread Model

**Gality is single-threaded.** By design.

- **No worker threads.** Asset loading is synchronous.
- **No async.** Node transitions block until complete.
- **No locks.** No mutexes, no atomics, no data races.

**Why?** Because a visual novel doesn't need threads. The workload is:

- **~1 KB of text per frame** (dialogue)
- **~5 sprite draws per frame**
- **~200 particles per frame** (weather)
- **1 audio stream** (BGM)
- **0-1 audio buffers** (CV)

This fits comfortably in a 16ms frame budget on **any** CPU made after 2010.

**Cold-start latency:** ~100ms (SFML window creation + JSON parse + texture load).

**Steady-state CPU:** 1-3% on a modern laptop.

---

## 6. Error Handling Philosophy

**Compile-time errors > runtime errors.**

| Error Type | Handled By | When |
|---|---|---|
| Dangling `-> node_999` | Compiler + ScriptLoader | Compile / Load |
| Duplicate node ID | Compiler | Compile |
| Mismatched rich text tags | Compiler | Compile |
| Missing asset file | `AssetPack::readFileFromPak` | Load |
| Missing font | `loadFont()` returns false | Boot |
| Missing `data.pak` | Boot fails with `MessageBox` | Boot |
| Bad Blackboard key | `getInt(key, 0)` returns default | Runtime |

**The engine assumes scripts are valid.** If you feed it broken JSON, it will fail loudly at startup. If you feed it valid JSON with broken story logic, it will run — but the story will be broken.

**This is intentional.** Silent error correction hides bugs.

---

## 7. Performance Characteristics

| Metric | Value | Notes |
|---|---|---|
| **Cold start** | ~100 ms | Window + JSON + first texture |
| **Steady-state CPU** | 1-3% | On i5-8250U, 60 FPS |
| **Memory (idle)** | ~48 MB | Engine + cached textures |
| **Memory (peak)** | ~120 MB | With 50 textures cached |
| **Node transition** | <1 ms | Pointer swap + presentation sync |
| **Typewriter speed** | 25 chars/sec | Configurable via `<speed>` |
| **Rollback cost** | O(1) | Snapshot pop from stack |
| **Save file size** | ~2 KB | JSON, human-readable |

**Bottlenecks to watch:**

1. **Texture loading** — ~10-50 ms per large JPG. Mitigated by LRU cache.
2. **Font rendering** — ~1 ms per 100 glyphs. Mitigated by caching glyph metrics.
3. **PostFX blur** — ~2 ms for 1080p dual-pass. Mitigated by `blurRadius` threshold.

---

## 8. Extension Points

### Adding a New Node Type

1. Add `NodeType::NewType` to `enum class NodeType`
2. Add fields to `StoryNode`
3. Parse in `ScriptLoader::parseScriptJson()`
4. Handle in `StoryExecutor::evaluateCurrentNode()`
5. Render in `GalityApp::render()` if visual

**No other files need changes.** This is the payoff of the sync point design.

### Adding a New Visual Effect

1. Create `NewEffect.hpp` with a `draw(sf::RenderTarget&)` method
2. Add member to `GalityApp`
3. Call `newEffect.update(dt)` in `update()`
4. Call `newEffect.draw(target)` in `render()`

**Effects compose cleanly.** They don't know about each other.

### Adding a New DSL Directive

1. Add parsing in `gality_compiler.py`
2. Add field to output JSON
3. Parse in `ScriptLoader::parseScriptJson()`
4. Handle in appropriate subsystem

**The compiler is 200 lines.** Extending it is trivial.

---

## 9. Anti-Patterns Avoided

| Anti-Pattern | Gality's Approach |
|---|---|
| Global mutable state | Everything lives in `GalityApp` |
| `shared_ptr` everywhere | `shared_ptr` only for node graph, textures |
| Reflection / RTTI | Explicit `enum class NodeType` |
| Virtual dispatch in hot loop | Lambdas bound at load time |
| Second render pipeline for UI | Single SFML pipeline |
| Async asset loading | Synchronous + LRU cache |
| Save file as binary blob | JSON, human-readable |

---

## 10. Summary

**Gality Engine is a visual novel engine that respects your time.**

- **Writers** get a clean DSL and a compiler that catches errors before runtime.
- **Engineers** get a codebase small enough to read in a weekend.
- **Players** get a responsive, 60 FPS experience with rollback and save/load.

**The architecture is boring on purpose.** No clever tricks, no magic. Just:

- A two-pass loader
- A while-loop FSM
- A blackboard of integers
- An LRU cache for textures
- A layer renderer with three slots

**Boring architecture is fast architecture.** And in the case of a visual novel, boring architecture is **correct** architecture.

---

<div align="center">

*Written for the people who read the source before the manual.*

**Gality Engine — System Architecture Specification v1.0**

</div>