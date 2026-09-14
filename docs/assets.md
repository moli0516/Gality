# 🌸 Gality Engine - Script & Asset Authoring Manual (v1.0)

## 1. Project Directory Structure

Place all raw assets into their corresponding folders prior to compilation:

* `assets/scripts/` : Stores raw `.gality` narrative source scripts.
* `assets/bg/` : Background images (`.jpg` / `.png`).
* `assets/characters/` : Character sprites (`.png` with transparent backgrounds recommended).
* `assets/audio/` : Background music (`.ogg`) and voice/SFX audio (`.ogg` / `.wav`).
* `assets/fonts/` : Default TrueType font (`font.ttf`).
* `assets/config/ui_theme.json` : User interface layout and style configuration file.

---

## 2. `.gality` Scripting Guide

### 2.1 Entry Point & Node Declaration

Every script must define an entry point node using `@start`. Declare narrative blocks using bracketed IDs:

```gality
@start node_01

[node_01]
bg: assets/bg/classroom.jpg
bgm: assets/audio/bgm_daily.ogg
weather: sakura
旁白: 放學後的陽光透過窗戶灑在課桌上。
-> node_02

```

---

### 2.2 Dialogue & Character Staging

* **Dialogue Format**: `Speaker Name: Dialogue Text`. Omit the speaker prefix and colon for narrator text.
* **Character Sprites & Voice**: Use `char:` to stage character textures and `cv:` to trigger voice-over clips.
* **Rich Text Formatting**: Wrap text in `<color=#HEX>Text</color>` for dynamic color highlighting.

```gality
[node_02]
bg: assets/bg/classroom.jpg
char: assets/characters/senior_normal.png
cv: assets/audio/cv_01.ogg
學姐: 你終於準備好了嗎？我們的<color=#FF5555>遊戲開發計劃</color>今天必須確定下來。
-> node_03

```

---

### 2.3 Presentation & Visual FX Directives

* **Weather Particles (`weather`)**: Supports `sakura`, `rain`, `snow`, and `none` (disabled).
* **Screen Shake (`shake`)**: Set an intensity scalar (e.g., `0.3` or `0.6`) to trigger dynamic camera shake.

```gality
[node_weather_demo]
bg: assets/bg/sunset.jpg
weather: rain
shake: 0.5
旁白: 突然一陣狂風暴雨襲來！
-> node_next

```

---

### 2.4 Branching Choices

Create interactive branches using `? [Option Text] -> TargetNode`:

```gality
[choice_01]
? [包在我身上吧！今晚就能搞定！] -> act_favor_up
? [感覺好麻煩啊... 可以交給別人嗎？] -> act_favor_down

```

---

### 2.5 State Flags & Conditional Logic

* **Variable Mutation (`$`)**: Modify global `Blackboard` variables (e.g., character affinity `$ favorability += 10`).
* **Conditional Jumps (`IF ... THEN ... ELSE`)**: Evaluate expressions to determine narrative paths automatically.

```gality
[act_favor_up]
$ favorability += 10
-> node_04_a

[cond_check_ending]
IF favorability >= 10 THEN node_good_ending ELSE node_normal_ending

```

---

## 3. UI Customization (`assets/config/ui_theme.json`)

Skin dialogue boxes and choice menus instantly without recompiling the binary by editing `ui_theme.json`:

```json
{
  "dialogueBox": {
    "posX": 40.0,
    "posY": 480.0,
    "width": 1200.0,
    "height": 200.0,
    "bgColor": [0, 0, 0, 200],
    "nameTextColor": [255, 255, 0, 255],
    "dialogueFontSize": 24
  },
  "choiceUI": {
    "width": 800.0,
    "height": 55.0,
    "spacing": 20.0,
    "startY": 220.0,
    "normalBgColor": [30, 30, 50, 220],
    "hoverBgColor": [70, 70, 130, 240]
  }
}

```

---

## 4. Hot-Reload & Debug Shortcuts

Use the following runtime diagnostics shortcuts during execution:

* **`F1` or ` (Tilde Key)**: Toggle the native Debugger Inspector to modify variables and force-jump nodes.
* **`Tab` or `H`**: Open or close the history backlog overlay.
* **`Mouse Wheel Up`**: Scroll directly upward to inspect past dialogue logs.
* **`F5` / `F9` Keys**: Quick-save or quick-load current session state.

---

## 5. One-Click Build & Distribution

Once writing is complete, double-click the root directory script: **`build_game.bat`**.

The build script automatically executes script transpilation (`gality_compiler.py`), encrypted asset packaging (`gality_packer.py`), and C++ binary compilation, delivering a fully standalone distribution package inside the `dist/` directory!