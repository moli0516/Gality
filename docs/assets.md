
# 🌸 Gality Engine — Script & Asset Authoring Manual

> **Version 1.0**
> *Written for writers, artists, and the occasional engineer who wandered in.*

This manual covers everything you need to author content for Gality Engine — from folder layout to `.gality` DSL to UI theming. No C++ required. No JSON for writers. Just text, images, and a compiler that does the rest.

---

## 1. Project Directory Structure

Before you write a single line of dialogue, place your assets in the right folders. The compiler **will not** find them otherwise.

```
assets/
├── scripts/          ← Your .gality narrative source files
├── bg/               ← Background images (.jpg / .png)
├── characters/       ← Character sprites (.png, transparent bg)
├── audio/            ← BGM (.ogg) + voice/SFX (.ogg / .wav)
├── fonts/            ← font.ttf (CJK-compatible recommended)
├── masks/            ← Transition masks (.png, grayscale)
├── shaders/          ← GLSL fragment shaders (advanced)
└── config/
    ├── ui_theme.json ← UI layout & styling
    └── settings.json ← Runtime preferences
```

### Naming Conventions (Recommended)

| Type | Convention | Example |
|---|---|---|
| Backgrounds | `location_time.jpg` | `classroom_day.jpg`, `rooftop_sunset.jpg` |
| Character sprites | `character_emotion.png` | `senior_happy.png`, `junior_pout.png` |
| BGM | `bgm_mood.ogg` | `bgm_daily.ogg`, `bgm_horror.ogg` |
| Voice lines | `cv_character_XX.ogg` | `cv_01.ogg`, `cv_jun_15.ogg` |
| SFX | `sfx_name.wav` | `typewriter.wav` |

**Consistency saves hours later.** Pick a scheme and stick to it.

---

## 2. The `.gality` Scripting Guide

`.gality` is a **declarative DSL** designed to feel like writing a screenplay. Each `[node_id]` is a scene beat. Each `->` is a transition. Each `$` is a state mutation.

The compiler (`gality_compiler.py`) converts your script into a JSON AST that the engine reads at runtime. **You never touch the JSON.**

---

### 2.1 Entry Point & Node Declaration

Every script must declare **exactly one** entry point with `@start`. Nodes are declared with `[bracketed_ids]`.

```gality
@start node_01

[node_01]
bg: assets/bg/classroom.jpg
bgm: assets/audio/bgm_daily.ogg
weather: sakura
旁白: 放學後的陽光透過窗戶灑在課桌上。
-> node_02
```

**Rules:**
- `@start` must be the **first non-comment line**.
- Node IDs must be **unique** within a script. Duplicates will be flagged by the compiler.
- Every `->` target must resolve to an existing `[node_id]`. Dangling pointers are compile errors.

---

### 2.2 Dialogue & Character Staging

#### Dialogue Format

```
SpeakerName: Dialogue text goes here.
```

- **With speaker:** `學姐: 你終於來了！`
- **Narrator (no speaker):** `旁白: 四月的春風掠過校園。`
- **Bare narration:** Just write the line without a colon.

#### Character Sprites

Use `char:` to stage a character sprite. For multi-character scenes, use slot-specific directives:

| Directive | Slot | Screen Position |
|---|---|---|
| `char_left:` | Left | X = 25% |
| `char_center:` or `char:` | Center | X = 50% |
| `char_right:` | Right | X = 75% |

```gality
[node_02]
bg: assets/bg/clubroom.jpg
char_left: assets/characters/senior_normal.png
char_right: assets/characters/junior_smug.png
active_char: right
cv: assets/audio/cv_jun_03.ogg
學妹: 哼，算你識相。但沒有角色多槽位立繪，畫面空曠得像個未完成品！
-> node_03
```

**`active_char:`** dims non-speaking characters automatically. Values: `left`, `center`, `right`.

#### Voice-Overs

Use `cv:` to trigger a voice clip on the current line.

```gality
[node_03]
char_center: assets/characters/senior_happy.png
cv: assets/audio/cv_01.ogg
學姐: 你終於準備好了嗎？我們的<color=#FF5555>遊戲開發計劃</color>今天必須確定下來。
-> node_04
```

**Audio panning:** Voice is automatically panned left/right based on `active_char`.

---

### 2.3 Rich Text Formatting

Gality supports inline tags for expressive dialogue without markdown hell.

| Tag | Effect | Example |
|---|---|---|
| `<color=#HEX>` | Text color | `<color=#FFB7C5>櫻花</color>` |
| `<shake>` | Per-character shake | `<shake>糟了！</shake>` |
| `<speed=X>` | Override typewriter speed | `<speed=0.05>輕聲細語</speed>` |
| `<w=X>` | Insert pause (seconds) | `等等<w=1.5>...你說什麼？` |

**Nesting works.** Combine freely:

```gality
學姐: 你終於來了！我們提報的<color=#FF5555><shake>自製 2D 遊戲引擎計畫</shake></color>，今天必須敲定架構方向。
```

**CJK Line-Breaking:** Gality implements *Kinsoku Shori* (避頭尾禁則). Punctuation like `，。！？` never lands at line start. Opening brackets never land at line end. **You don't have to think about it.**

---

### 2.4 Presentation & Visual FX Directives

#### Weather Particles

```gality
weather: sakura | rain | snow | none
```

Persistent until changed or the node is overridden.

#### Screen Shake

```gality
shake: 0.3    # subtle
shake: 0.6    # dramatic
```

Scalar from `0.0` (no shake) to `1.0` (violent).

#### Transitions

```gality
trans: diamond | wipe_left | clock | dissolve
duration: 1.5    # seconds
```

| Mask | Effect |
|---|---|
| `diamond` | Radial diamond wipe (classic VN) |
| `wipe_left` | Horizontal slide |
| `clock` | Rotational sweep |
| `dissolve` | Crossfade |

```gality
[node_weather_demo]
bg: assets/bg/sunset.jpg
trans: dissolve
duration: 1.5
weather: rain
shake: 0.5
旁白: 突然一陣狂風暴雨襲來！
-> node_next
```

---

### 2.5 Branching Choices

Choices are declared with `? [Option Text] -> TargetNode`:

```gality
[choice_01]
? [包在我身上吧！今晚就能搞定！] -> act_favor_up
? [感覺好麻煩啊... 可以交給別人嗎？] -> act_favor_down
```

**Every choice becomes a clickable button.** No configuration needed.

**Tip:** Keep option text under ~40 CJK characters for best readability.

---

### 2.6 State Flags & Conditional Logic

Gality uses a global **Blackboard** — a key-value store of integers.

#### Variable Mutation

```gality
[act_favor_up]
$ favorability += 10
$ san_loss += 5
-> node_04_a
```

Supported operators: `+=`, `-=`, `=`.

#### Conditional Jumps

```gality
[cond_check_ending]
IF favorability >= 10 THEN node_good_ending ELSE node_normal_ending
```

**Supported operators:** `>=`, `<=`, `>`, `<`, `==`

**⚠️ Important:** The compiler requires both `THEN` and `ELSE`. For multi-condition chains, use sequential nodes:

```gality
[node_route_selector]
IF san_loss >= 60 THEN node_abyss ELSE node_route_selector_2

[node_route_selector_2]
IF junior_favor >= 40 THEN node_junior ELSE node_route_selector_3

[node_route_selector_3]
IF favorability >= 100 THEN node_senior ELSE node_tech
```

This is the **priority chain pattern**. The first matching condition wins.

---

## 3. UI Customization

Skin the entire UI without recompiling the engine. Edit `assets/config/ui_theme.json`:

```json
{
  "dialogueBox": {
    "posX": 80.0,
    "posY": 740.0,
    "width": 1760.0,
    "height": 280.0,
    "bgColor": [0, 0, 0, 215],
    "borderColor": [255, 255, 255, 100],
    "nameBoxPosX": 80.0,
    "nameBoxPosY": 670.0,
    "nameBoxWidth": 320.0,
    "nameBoxHeight": 60.0,
    "nameBoxBgColor": [40, 40, 90, 235],
    "nameTextColor": [255, 220, 0, 255],
    "dialogueTextColor": [255, 255, 255, 255],
    "nameFontSize": 28,
    "dialogueFontSize": 32
  },
  "choiceUI": {
    "width": 1100.0,
    "height": 70.0,
    "spacing": 25.0,
    "startY": 340.0,
    "fontSize": 28,
    "normalBgColor": [30, 30, 50, 220],
    "hoverBgColor": [70, 70, 130, 240],
    "normalOutlineColor": [100, 100, 180, 255],
    "hoverOutlineColor": [255, 215, 0, 255],
    "textColor": [255, 255, 255, 255]
  }
}
```

**Coordinates are in 1920×1080 logical space.** The engine letterboxes automatically on other aspect ratios.

**Colors are RGBA** `[0-255, 0-255, 0-255, 0-255]`.

---

## 4. Debug Shortcuts & Runtime Diagnostics

Gality ships with **built-in dev tools**. No external debugger needed.

| Key | Action |
|---|---|
| **F1** or **`** (Tilde) | Toggle DebugOverlay (Blackboard editor, Node Jumper, Diagnostics) |
| **F2** | Toggle NodeGraphViewer (DAG tree + live property inspector) |
| **Tab** or **H** | Open/Close History Backlog |
| **Mouse Wheel Up** | Rollback one dialogue step (Time Machine) |
| **Backspace** | Rollback (alternative) |
| **F5 / F9** | Quick Save / Quick Load |
| **K** | Trigger test screen shake |
| **Escape** | Settings / Close modal |

### DebugOverlay Tabs

- **Blackboard**: Inspect and edit every integer variable live
- **Node Jumper**: Search and jump to any node ID
- **Diagnostics**: FPS, frame time, memory
- **Console**: Logs from the engine

**Use these constantly.** They will save you hours.

---

## 5. Building & Distribution

### During Development

Use **hot-reload** — the engine watches `demo_long.json` and reloads the AST without restarting.

Workflow:

1. Edit `.gality`
2. Run `python devtools/scripts/gality_compiler.py assets/scripts/main_story_multi.gality assets/scripts/demo_long.json`
3. Switch to running game — it reloads automatically

### One-Click Release

When you're ready to ship, run:

```powershell
.\build_game.bat
```

This will:

1. Compile `.gality` → JSON AST
2. Pack `assets/` → encrypted `data.pak`
3. Build `Gality.exe` (Release)
4. Assemble standalone bundle in `.\dist\`

**The `dist/` folder is fully self-contained.** Zip it and ship it.

### Manual Build Steps

If you want control over each step:

```bash
# 1. Compile DSL
python devtools/scripts/gality_compiler.py \
    assets/scripts/main_story_multi.gality \
    assets/scripts/demo_long.json

# 2. Pack assets
python devtools/scripts/gality_packer.py assets data.pak

# 3. Build engine
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# 4. Launch
.\build\Release\Gality.exe
```

---

## 6. Common Pitfalls

### ❌ Dangling Node Pointers

```gality
[node_01]
旁白: Hello
-> node_999    # ❌ node_999 doesn't exist
```

**Compiler will flag this.** Always verify targets exist.

### ❌ Missing `ELSE` in Conditionals

```gality
[cond_check]
IF favorability >= 10 THEN node_good    # ❌ Missing ELSE
```

**The compiler requires both branches.** Use the sequential pattern for multi-condition logic.

### ❌ Character Sprite Without Alpha

If your character PNG has a solid background, it will render as a rectangle. **Always use transparent PNG.**

**Tip:** Use `rembg` for AI-powered background removal, then post-process to remove green edges.

### ❌ Oversized Background Images

Backgrounds larger than 1920×1080 waste memory. Resize before adding to `assets/bg/`.

### ❌ Rich Text Tag Mismatch

```gality
學姐: <color=#FF5555>紅色文字    # ❌ Missing </color>
```

**The compiler will flag mismatched tags.**

---

## 7. Quick Reference Card

```
ENTRY POINT:      @start node_01

NODE DECLARATION: [node_id]

DIALOGUE:         Speaker: Text
NARRATION:        旁白: Text

TRANSITIONS:      -> node_next
CHOICES:          ? [Text] -> node_next
CONDITIONALS:     IF var OP val THEN node_a ELSE node_b
MUTATIONS:        $ var += val

STAGING:          char_left / char_center / char_right
                  active_char: left | center | right
                  cv: assets/audio/cv_XX.ogg

VISUAL FX:        trans: diamond | wipe_left | clock | dissolve
                  duration: 1.5
                  weather: sakura | rain | snow | none
                  shake: 0.5

RICH TEXT:        <color=#HEX>...</color>
                  <shake>...</shake>
                  <speed=0.05>...</speed>
                  <w=1.5>

DEBUG:            F1 (overlay) | F2 (graph) | Tab (backlog)
                  F5 (save) | F9 (load) | K (shake test)
```

---

<div align="center">

*Written for the people who make stories real.*

**Gality Engine — Authoring Manual v1.0**

</div>