
# Getting Started with Gality

**Welcome to Gality.** This guide will get you from "just downloaded" to
"running your own visual novel" in about 30 minutes.

**No C++ required.** You don't need to write any engine code. You'll write
your story in `.gality` DSL, customize the look with `ui_theme.json`, and
let Gality handle the rest.

---

## Table of Contents

1. [Prerequisites](#1-prerequisites)
2. [Installation](#2-installation)
3. [Your First Scene](#3-your-first-scene)
4. [Running the Game](#4-running-the-game)
5. [Adding Choices](#5-adding-choices)
6. [Adding Characters](#6-adding-characters)
7. [Adding Backgrounds and Audio](#7-adding-backgrounds-and-audio)
8. [Text Effects](#8-text-effects)
9. [Packaging for Distribution](#9-packaging-for-distribution)
10. [Next Steps](#10-next-steps)

---

## 1. Prerequisites

Before you start, make sure you have:

| Requirement | Version | Why |
|---|---|---|
| **Windows 10/11** or **macOS** | Recent | Gality runs on these |
| **Disk space** | ~2 GB | For dependencies + engine |
| **VS Code** | Latest | Recommended editor |
| **Python** | 3.8+ | For the compiler and packer |
| **Git** | Latest | For version control (recommended) |

**Not required**:
- ❌ C++ compiler (unless you want to build from source)
- ❌ C++ knowledge
- ❌ Any prior game development experience

---

## 2. Installation

### Option A: Download Pre-Built Binary (Easiest)

1. Go to [Releases](https://github.com/moli0516/Gality/releases)
2. Download the latest `Gality-*-windows.zip` or `Gality-*-macos.zip`
3. Extract to a folder (e.g., `C:\Gality\` or `~/Gality/`)
4. Done!

**Skip to [Section 3](#3-your-first-scene).**

### Option B: Build from Source (For Developers)

If you want to modify the engine or use the dev build:

**Windows**:
```bash
git clone https://github.com/moli0516/Gality.git
cd Gality
build_dev.bat
```

**macOS**:
```bash
git clone https://github.com/moli0516/Gality.git
cd Gality
chmod +x build_game.sh
./build_game.sh
```

This will:
1. Compile the demo script
2. Pack assets into `data.pak`
3. Build `Gality.exe` (or `Gality`)
4. Place the output in `dist-dev/` (or `dist/`)

### Install VS Code Extension

For syntax highlighting and IntelliSense:

1. Open VS Code
2. Go to **Extensions** (`Ctrl+Shift+X` / `Cmd+Shift+X`)
3. Click `...` → **Install from VSIX...**
4. Select `gality-language-support-1.0.0.vsix`

Or via command line:
```bash
code --install-extension gality-language-support-1.0.0.vsix
```

---

## 3. Your First Scene

Let's create your first scene.

### Step 1: Navigate to the Scripts Folder

```
gality/
└── assets/
    └── scripts/
        └── my_story.gality    ← Create this file
```

Create a new file called `my_story.gality`.

### Step 2: Write Your First Line

Copy this into the file:

```gality
@start start

[start]
bg: assets/bg/classroom.jpg
旁白: 歡迎來到我的故事！
-> the_end

[the_end]
旁白: 故事結束。
-> the_end
```

**What does this mean?**

| Line | Meaning |
|---|---|
| `@start start` | Tells Gality to begin at the `start` node |
| `[start]` | Defines a node called `start` |
| `bg: ...` | Sets the background image |
| `旁白: ...` | A line of narration (speaker: text) |
| `-> the_end` | Go to the `the_end` node next |
| `[the_end]` | Another node |
| `-> the_end` | Loop back to itself (so the game doesn't crash) |

### Step 3: Understand the Node

Every node has:

- **An ID** in `[...]`
- **Attributes** (optional): `bg:`, `bgm:`, `char:`, etc.
- **Content**: A dialogue line, a choice, or an action
- **A destination**: `-> next_node`

That's it. **The whole DSL works this way.**

---

## 4. Running the Game

### Step 1: Compile Your Script

Open a terminal in the Gality folder:

```bash
python -m devtools.scripts.gality_compiler assets/scripts/my_story.gality assets/scripts/demo_long.json
```

You should see:

```
[GalityCompiler] Successfully compiled 'assets/scripts/my_story.gality' -> 'assets/scripts/demo_long.json'
[GalityCompiler] Generated 2 valid AST nodes. Start node: 'start'
```

**What happened?** Your `.gality` script was compiled into a JSON AST that the
engine can execute.

### Step 2: Pack Assets

```bash
python -m devtools.scripts.gality_packer assets data.pak
```

You should see:

```
[Gality Packer] Packing N assets into data.pak...
[Gality Packer] Pack complete! Output: data.pak
```

### Step 3: Run the Game

```bash
build_dev.bat
```

Or, if already built:

```bash
dist-dev\Gality.exe
```

**You should see**: A classroom background with "歡迎來到我的故事！" typed
out at the bottom. Press **Space** or **click** to advance.

**Congratulations! You've made your first visual novel scene.**

---

## 5. Adding Choices

Choices let the player decide what happens next.

```gality
@start start

[start]
bg: assets/bg/classroom.jpg
旁白: 你看到桌上有一本書。
-> choice_01

[choice_01]
? [翻開書] -> read_book
? [離開教室] -> leave_room

[read_book]
旁白: 書裡寫著一個秘密...
-> the_end

[leave_room]
旁白: 你決定不多管閒事。
-> the_end

[the_end]
旁白: 故事結束。
-> the_end
```

**Key syntax**:

```gality
[choice_node]
? [選項文字] -> target_node
? [另一個選項] -> other_target
```

- Each `?` line is a choice
- `[text]` is what the player sees
- `-> node_id` is where it goes

### Adding Conditions to Choices

You can show/hide choices based on variables:

```gality
[choice_01]
? [進入秘密房間（需要鑰匙）] -> secret_room
? [離開] -> leave_room
```

For conditional choices, use `IF` nodes:

```gality
[before_choice]
IF has_key >= 1 THEN choice_with_key ELSE choice_without_key

[choice_with_key]
? [進入秘密房間] -> secret_room
? [離開] -> leave_room

[choice_without_key]
? [離開] -> leave_room
```

---

## 6. Adding Characters

Show a character on screen:

```gality
[start]
bg: assets/bg/classroom.jpg
char: assets/characters/senior_normal.png
學姐: 你終於來了！
-> next

[next]
char: assets/characters/senior_smile.png
學姐: 我等你好久了。
-> the_end
```

**Character slots** (for multiple characters):

```gality
[multi]
bg: assets/bg/classroom.jpg
char_left: assets/characters/senior_normal.png
char_right: assets/characters/junior_normal.png
active_char: left
學姐: 這是我的學妹，羽月。
-> next

[next]
char_left: assets/characters/senior_normal.png
char_right: assets/characters/junior_smug.png
active_char: right
學妹: 學姐，別亂介紹我！
-> the_end
```

| Slot | Position |
|---|---|
| `char_left` | Left side (25% of screen width) |
| `char_center` | Center (50%) |
| `char_right` | Right side (75%) |

`active_char` controls who is "speaking" (others are dimmed).

**Shortcut**: `char: path` is the same as `char_center: path`.

### Character Scaling

Adjust via `ui_theme.json`:

```json
"characterScaling": {
  "single": 0.85,
  "double": 0.78,
  "triple": 0.70,
  "bottomOffset": 0.0
}
```

See [Character Guide](character_guide.md) for details.

---

## 7. Adding Backgrounds and Audio

### Backgrounds

```gality
[scene]
bg: assets/bg/classroom.jpg
旁白: 這是教室。
-> next

[next]
bg: assets/bg/rooftop.jpg
trans: dissolve
duration: 1.5
旁白: 現在是頂樓。
-> the_end
```

**Available transitions**:

| Name | Effect |
|---|---|
| `diamond` | Diamond mask reveal |
| `wipe_left` | Horizontal wipe |
| `dissolve` | Fade to black and back |
| `clock` | Clock-style circular reveal |

### BGM (Background Music)

```gality
[scene]
bg: assets/bg/classroom.jpg
bgm: assets/audio/bgm_daily.ogg
旁白: 背景音樂開始播放。
-> next

[next]
bgm: assets/audio/bgm_emotional.ogg
旁白: 音樂切換成感傷的旋律。
-> the_end
```

**To stop BGM**:

```gality
[stop]
bgm: 
旁白: 音樂停止了。
-> the_end
```

### Voice (CV)

```gality
[voiced]
char: assets/characters/senior_normal.png
cv: assets/audio/cv_01.ogg
學姐: 這句台詞有配音。
-> the_end
```

### Weather

```gality
[rainy]
bg: assets/bg/hallway.jpg
weather: rain
旁白: 外面下著雨。
-> next

[snowy]
weather: snow
旁白: 雪開始下了。
-> next

[sakura]
weather: sakura
旁白: 櫻花紛飛。
-> next

[clear]
weather: none
旁白: 天氣放晴了。
-> the_end
```

---

## 8. Text Effects

Gality supports rich inline tags inside dialogue text.

### Basic Colors

```gality
旁白: 這是<color=#FF5555>紅色</color>和<color=#55FF55>綠色</color>。
```

### Shake

```gality
旁白: 這是<shake>震動</shake>文字。
```

### Wave (Sine animation)

```gality
旁白: 這是<wave>波浪</wave>文字。
```

### Rainbow (Color cycling)

```gality
旁白: 這是<rainbow>彩虹</rainbow>文字。
```

### Glitch (RGB split)

```gality
旁白: 這是<glitch>故障</glitch>文字。
```

### Speed Control

```gality
旁白: <speed=0.02>快速文字</speed>
旁白: <speed=0.15>慢速文字</speed>
```

### Mid-Text Pause

```gality
旁白: 我...<w=1.0>其實...<w=0.5>一直...
```

### Combining Tags

```gality
旁白: <wave><rainbow>波浪彩虹</rainbow></wave>
旁白: <glitch><speed=0.3>慢速故障</speed></glitch>
```

### Forced Read (Dramatic Moments)

```gality
[dramatic]
bg: assets/bg/room.jpg
no_skip: true
wait: 1.0
幽靈: <glitch><speed=0.5>你... 終於... 打開了...</speed></glitch>
-> next
```

- `no_skip: true` — player cannot skip during typewriter
- `wait: N` — mandatory delay after typewriter finishes

---

## 9. Packaging for Distribution

When your game is ready to share:

### Step 1: Build a Product Version

```bash
build_prod.bat
```

This:
1. Compiles your `.gality` script
2. Packs assets into `data.pak`
3. Builds `Gality.exe` (Release, no debug tools)
4. Assembles a clean bundle in `dist/`

### Step 2: Test the Bundle

```bash
dist\Gality.exe
```

**Important**: Test the `dist/` build, not the `build/` build. They may behave
differently.

### Step 3: Create a Release

1. Zip the `dist/` folder:

```bash
# Windows
Compress-Archive -Path dist\* -DestinationPath Gality-v0.3.0-win64.zip

# macOS
zip -r Gality-v0.3.0-macos.zip dist/
```

2. Upload to:
   - [itch.io](https://itch.io/)
   - [Steam](https://store.steampowered.com/) (requires setup)
   - GitHub Releases

See [Publishing Guide](publishing.md) for details.

---

## 10. Next Steps

You've learned the basics. Now dive deeper:

### Learn the Language

- **[Script Language Reference](script_language.md)** — Complete `.gality` DSL
- **[Text Effects Reference](text_effects.md)** — All inline tags
- **[Example Scripts](example.md)** — Sample projects

### Customize the Look

- **[UI Guide](ui_guide.md)** — `ui_theme.json` reference
- **[Character Guide](character_guide.md)** — Multi-slot rendering
- **[Assets Guide](assets.md)** — Preparing your own art and audio

### Understand the Engine

- **[Architecture](architecture.md)** — Engine internals
- **[Performance Tuning](performance_tuning.md)** — Optimization tips

### Extend the Engine

- **[Contributing](../CONTRIBUTING.md)** — How to contribute
- **[Security Policy](../SECURITY.md)** — Security considerations

---

## Common Issues

### "Failed to load script"

**Cause**: The compiler didn't run, or `demo_long.json` is missing.

**Fix**:
```bash
python -m devtools.scripts.gality_compiler assets/scripts/my_story.gality assets/scripts/demo_long.json
```

### "Missing asset: assets/bg/classroom.jpg"

**Cause**: The asset doesn't exist, or `data.pak` wasn't rebuilt.

**Fix**:
1. Check that the file exists: `ls assets/bg/classroom.jpg`
2. Rebuild the pack: `python -m devtools.scripts.gality_packer assets data.pak`
3. Restart Gality.

### Game shows a black screen

**Cause**: Something failed during initialization.

**Fix**:
1. Open the console (in VS Code: **Terminal** → **New Terminal**)
2. Run `dist-dev\Gality.exe` from the terminal
3. Read the error log

### Text is too fast / too slow

**Cause**: The default text speed is `0.04` seconds per character.

**Fix**: Either:
- Use `<speed=N>` inline tags
- Modify `assets/config/settings.json`:
  ```json
  { "textSpeed": 0.06 }
  ```

### Font renders as boxes (CJK)

**Cause**: The bundled font doesn't include CJK glyphs.

**Fix**:
- Gality ships with `NaikaiFont-SemiBold` by default (supports CJK)
- If you replaced it, use a CJK-compatible font
- See [Assets Guide](assets.md) for font recommendations

### I edited `ui_theme.json` but nothing changed

**Cause**: `ui_theme.json` is packed into `data.pak`, which isn't rebuilt
automatically.

**Fix**:
1. Save `ui_theme.json`
2. Repack: `python -m devtools.scripts.gality_packer assets data.pak`
3. Restart the game.

**Tip**: In dev builds, you can also edit `assets/config/ui_theme.json`
directly and use the hot-reload feature (F1 → Reload).

---

## Getting Help

If you're stuck:

1. **Check the docs** — see [Documentation](../README.md#-documentation)
2. **Search issues** — [GitHub Issues](https://github.com/moli0516/Gality/issues)
3. **Ask** — [GitHub Discussions](https://github.com/moli0516/Gality/discussions)

When asking for help, include:
- What you tried
- The exact error message
- Your `my_story.gality` file (if relevant)
- Your OS and Gality version

---

## Cheat Sheet

Quick reference for common tasks:

| Task | Syntax |
|---|---|
| Start node | `@start node_id` |
| Define node | `[node_id]` |
| Dialogue | `speaker: text` |
| Narration | `旁白: text` |
| Set background | `bg: path` |
| Set BGM | `bgm: path` |
| Set character | `char: path` |
| Multi-character | `char_left:` / `char_center:` / `char_right:` |
| Set active speaker | `active_char: left/center/right` |
| Go to next | `-> node_id` |
| Choice | `? [text] -> node_id` |
| Condition | `IF var >= N THEN a ELSE b` |
| Mutation | `$ var += N` |
| Transition | `trans: diamond` + `duration: 1.5` |
| Weather | `weather: rain` |
| Forced read | `no_skip: true` + `wait: 1.0` |
| Color tag | `<color=#FF0000>text</color>` |
| Animation tag | `<wave>text</wave>` |
| Speed tag | `<speed=0.1>text</speed>` |
| Pause tag | `text<w=1.0>more` |

---

**You're ready.** Go write your story. 🌸

---

**Last updated**: 2026-09-17
