
# Character Guide

**Everything you need to know about character rendering in Gality.**

This guide covers:

1. [Overview](#1-overview)
2. [Sprite Preparation](#2-sprite-preparation)
3. [Staging Characters in Scripts](#3-staging-characters-in-scripts)
4. [Multi-Slot Layout](#4-multi-slot-layout)
5. [Active Character & Dimming](#5-active-character--dimming)
6. [Voice Panning](#6-voice-panning)
7. [Scaling Configuration](#7-scaling-configuration)
8. [Expression Management](#8-expression-management)
9. [Advanced Patterns](#9-advanced-patterns)
10. [Troubleshooting](#10-troubleshooting)

---

## 1. Overview

Gality's character system supports:

- **Multi-slot staging** — up to 3 characters on screen (Left / Center / Right)
- **Automatic dimming** — non-speaking characters are dimmed
- **Voice panning** — voice audio is panned to match character position
- **Expression switching** — change sprites between nodes
- **Data-driven scaling** — adjust via `ui_theme.json`
- **Persistent slots** — characters stay until changed

**What Gality does NOT support**:

- ❌ Layered parts (blink, mouth, expression overlays)
- ❌ Skeletal animation
- ❌ 3D characters
- ❌ Custom slot positions (v0.3.0)

**Future**: Layered parts and custom positions are on the roadmap.

---

## 2. Sprite Preparation

### Format Requirements

| Property | Requirement |
|---|---|
| **Format** | PNG with alpha (RGBA) |
| **Background** | Transparent |
| **Size** | 800×1200 minimum, 1200×1800 recommended |
| **Aspect ratio** | 2:3 or 3:4 |
| **File size** | < 1 MB per sprite |

### Why Transparent Background?

Sprites are overlaid on backgrounds. If the PNG has an opaque background,
it renders as a **rectangle**, hiding the background behind it.

**Always use transparent PNG.**

See [Assets Guide § 4](assets.md#4-character-sprites) for full preparation
details.

### Composition Guidelines

**Recommended framing**:
- **Upper body** — from head to waist (most common for VN)
- **Full body** — from head to feet (for standing scenes)
- **Half body** — from head to hips (variation)

**Position**: Character should be **centered horizontally** in the sprite.

**Bottom alignment**: The sprite's bottom edge should be at the
**character's waist or hip level**, not at the feet. Gality aligns the
sprite's bottom edge to the dialogue box top edge.

**Example layout** (1200×1800):

```
┌─────────────────────────┐  y=0
│                         │
│      [Head]             │
│                         │
│                         │
│   [Torso + Arms]        │
│                         │
│                         │
│   [Hips / Waist]        │  y=1200
│                         │  ← Aligns with dialogue box top
└─────────────────────────┘  y=1800
```

**The bottom 600 pixels** may be hidden by the dialogue box.

### Expression Variants

For a character with N expressions, create N separate PNGs:

```
senior_normal.png       ← Default
senior_smile.png        ← Slight smile
senior_happy.png        ← Bright smile
senior_surprised.png    ← Eyes wide, mouth open
senior_pout.png         ← Puffed cheeks, annoyed
senior_fear.png         ← Constricted pupils, cold sweat
senior_corrupt.png      ← Hollow eyes, glitch marks
```

**Naming convention**: `character_emotion.png`

**Consistency**: Generate all expressions in the same session (if AI) or
draw them from the same base (if manual).

See [Assets Guide § 4](assets.md#4-character-sprites) for AI generation tips.

### Slot Optimization

For multi-character scenes, sprites are scaled to 70-85% of screen height.
Consider creating sprite variants for:

- **Close-up** (single character) — higher detail
- **Standard** (two characters) — medium detail
- **Wide** (three characters) — background-friendly

**Or** use one sprite and let Gality scale it. Simpler, but quality may suffer
at extreme scales.

### Checklist

- [ ] PNG with alpha channel
- [ ] Transparent background
- [ ] 1200×1800 recommended (800×1200 minimum)
- [ ] Character centered horizontally
- [ ] Bottom edge at waist level
- [ ] Named `character_emotion.png`
- [ ] Consistent style across expressions

---

## 3. Staging Characters in Scripts

### Single Character

The simplest form — one character on screen:

```gality
[node_01]
bg: assets/bg/classroom.jpg
char: assets/characters/senior_normal.png
學姐: 你終於來了！
-> node_02
```

**`char:` is shorthand for `char_center:`.**

The character appears in the center slot at 85% of screen height.

### Multiple Characters

Use slot-specific directives:

```gality
[node_02]
bg: assets/bg/clubroom.jpg
char_left: assets/characters/senior_normal.png
char_right: assets/characters/junior_normal.png
active_char: left
學姐: 這是我的學妹，羽月。
-> node_03
```

| Directive | Slot | Screen X |
|---|---|---|
| `char_left` | Left | 25% |
| `char_center` / `char` | Center | 50% |
| `char_right` | Right | 75% |

### Three Characters

```gality
[node_03]
bg: assets/bg/clubroom.jpg
char_left: assets/characters/senior_normal.png
char_center: assets/characters/president_normal.png
char_right: assets/characters/junior_normal.png
active_char: center
社長: 三個人都在場。
-> node_04
```

### Changing Expressions

Change the sprite path in a new node:

```gality
[node_04]
char_center: assets/characters/senior_normal.png
學姐: 這是普通表情。
-> node_05

[node_05]
char_center: assets/characters/senior_smile.png
學姐: 這是微笑表情。
-> node_06

[node_06]
char_center: assets/characters/senior_happy.png
學姐: 這是開心表情。
-> node_07
```

**Important**: You must specify the **full path** each time, not just the
filename.

### Removing a Character

Empty value removes the character:

```gality
[node_07]
char_left: 
旁白: 左邊的角色消失了。
-> node_08
```

### Persistence

Characters **persist between nodes** until:

- Changed by another `char_*` directive
- Removed by an empty value
- Scene fully transitions (backgrounds change)

**Example**:

```gality
[node_01]
char_center: assets/characters/senior_normal.png
學姐: 你好。
-> node_02

[node_02]
旁白: 學姐還在那裡。
-> node_03

[node_03]
char_center: assets/characters/senior_smile.png
學姐: 現在我笑了。
-> node_04
```

At `node_02`, the character is still visible even though not specified.

### Combining with Other Directives

```gality
[node_04]
bg: assets/bg/rooftop.jpg
trans: dissolve
duration: 1.5
char_left: assets/characters/senior_normal.png
char_right: assets/characters/junior_normal.png
active_char: right
cv: assets/audio/cv_jun_03.ogg
weather: sakura
學妹: 學姐，這裡好美啊。
-> node_05
```

All directives are independent.

---

## 4. Multi-Slot Layout

### Slot Positions

Gality uses **fixed slot positions**:

| Slot | Screen X | Description |
|---|---|---|
| Left | 25% | Left third of screen |
| Center | 50% | Center |
| Right | 75% | Right third |

For a 1920-pixel-wide canvas:

- Left: X = 480
- Center: X = 960
- Right: X = 1440

### Custom Positions

**Not supported in v0.3.0**. Slots are fixed.

**Future**: Configurable positions are on the roadmap.

**Workaround**: If you need custom positions, modify `LayerRenderer.hpp`
and rebuild.

### Overlap Avoidance

Characters are scaled to avoid overlap:

| Character Count | Scale |
|---|---|
| 1 | 85% of screen height |
| 2 | 78% |
| 3 | 70% |

**Example**: A 1200×1800 sprite at single-character scale:

- Target height: 1080 × 0.85 = **918 pixels**
- Scale factor: 918 / 1800 = **0.51**
- Scaled width: 1200 × 0.51 = **612 pixels**

At double-character scale:

- Target height: 1080 × 0.78 = **842 pixels**
- Scale factor: 842 / 1800 = **0.47**
- Scaled width: 1200 × 0.47 = **564 pixels**

Two characters at 564 pixels wide with 480-pixel slot distance:

- Left character at X = 480, right edge at 480 + 282 = 762
- Right character at X = 1440, left edge at 1440 - 282 = 1158
- Gap: 1158 - 762 = **396 pixels** — no overlap

At triple-character scale:

- Target height: 1080 × 0.70 = **756 pixels**
- Scale factor: 756 / 1800 = **0.42**
- Scaled width: 1200 × 0.42 = **504 pixels**

Three characters with 480-pixel spacing:

- Left: X = 480, right edge at 480 + 252 = 732
- Center: X = 960, left edge at 960 - 252 = 708, right edge at 1212
- Right: X = 1440, left edge at 1440 - 252 = 1188

Left and center overlap: 732 > 708 — **24 pixels overlap**
Center and right overlap: 1212 > 1188 — **24 pixels overlap**

**This is expected**. Slight overlap is common in VN layouts and looks natural.

**To avoid overlap**: Use narrower sprites (e.g., 1000×1800) or accept the
slight overlap.

### Bottom Alignment

All characters align their **bottom edge** to the dialogue box top edge.

If the dialogue box is at Y = 700 (top), characters extend from
Y = 700 - their_scaled_height to Y = 700.

**Example**: A 918-pixel-tall scaled character:

- Top: Y = 700 - 918 = **-218** (extends off-screen at top!)
- Bottom: Y = 700

This is **normal**. Characters are cropped at the top by the window.

**To adjust**: Use `bottomOffset` in `ui_theme.json` (positive lowers, negative raises):

```json
"characterScaling": {
  "bottomOffset": 50
}
```

This shifts all characters down by 50 pixels.

---

## 5. Active Character & Dimming

### The `active_char` Directive

Controls which character is "speaking":

```gality
[node]
char_left: assets/characters/senior_normal.png
char_right: assets/characters/junior_normal.png
active_char: left
學姐: 我在說話。
-> next
```

**Values**: `left`, `center`, `right`

### Visual Effect

| State | Color | Description |
|---|---|---|
| **Active** | `(255, 255, 255, 255)` | Full brightness |
| **Inactive** | `(140, 140, 150, 220)` | Dimmed (~55% brightness, slightly blue) |

**Purpose**: Draws attention to the speaking character.

### Example: Switching Active Speaker

```gality
[node_01]
char_left: assets/characters/senior_normal.png
char_right: assets/characters/junior_normal.png
active_char: left
學姐: 羽月，妳覺得呢？
-> node_02

[node_02]
char_left: assets/characters/senior_normal.png
char_right: assets/characters/junior_normal.png
active_char: right
學妹: 我覺得這個架構有點問題。
-> node_03

[node_03]
char_left: assets/characters/senior_pout.png
char_right: assets/characters/junior_smug.png
active_char: left
學姐: 喂！別那麼囂張！
-> node_04
```

Each node can switch the active character.

### No Active Character

If `active_char` is not specified or set to a nonexistent slot:

- All characters are shown at full brightness
- No dimming

**Use case**: Narration or scene-setting.

### Persistence

`active_char` persists between nodes. If you don't specify it, the previous
value is used.

**Reset to none**:

```gality
[node]
char_left: assets/characters/senior_normal.png
char_right: assets/characters/junior_normal.png
active_char: 
旁白: 兩個人都沒有說話。
-> next
```

### Dimming Customization

Dimming colors are **hardcoded** in `LayerRenderer.hpp`. To customize:

```cpp
// In LayerRenderer::updateCharacters()
if (!activeSlot.has_value() || activeSlot.value() == slot) {
    sprite.setColor(sf::Color(255, 255, 255, 255));  // Active
} else {
    sprite.setColor(sf::Color(140, 140, 150, 220));  // Inactive (dimmed)
}
```

Modify and rebuild.

**Future**: Dimming colors will be configurable in `ui_theme.json`.

---

## 6. Voice Panning

### How It Works

When a voice clip plays (`cv:` directive), the audio is **panned** based on
`active_char`:

| `active_char` | Audio Pan |
|---|---|
| `left` | Left channel louder |
| `center` | Balanced |
| `right` | Right channel louder |

**SFML panning** uses `sf::Sound::setPosition()`. Gality sets:

```cpp
float posX = 0.0f;
if (slot.has_value()) {
    switch (slot.value()) {
        case CharSlot::Left:   posX = -1.0f; break;  // Left
        case CharSlot::Center: posX =  0.0f; break;  // Center
        case CharSlot::Right:  posX =  1.0f; break;  // Right
    }
}
voiceSound->setPosition(sf::Vector3f(posX, 0.0f, 0.0f));
voiceSound->setRelativeToListener(true);
```

### Spatial Effect

The panning is **mild** — not a hard left/right split. It creates a subtle
sense of direction, matching the character's visual position.

**Example**: A character on the left speaking will have their voice slightly
louder in the left ear. This is a standard VN technique.

### When Panning Applies

Panning applies **only to voice clips** (`cv:` directive). It does not apply
to:

- BGM (always centered)
- SFX (always centered)
- Typewriter sound (always centered)

### Disabling Panning

To disable panning, modify `AudioManager.hpp`:

```cpp
// Remove the setPosition call
// voiceSound->setPosition(sf::Vector3f(posX, 0.0f, 0.0f));
voiceSound->setRelativeToListener(true);
```

**Future**: Panning toggle may be added to `settings.json`.

---

## 7. Scaling Configuration

### Configuration

In `assets/config/ui_theme.json`:

```json
"characterScaling": {
  "single": 0.85,
  "double": 0.78,
  "triple": 0.70,
  "bottomOffset": 0.0
}
```

### Fields

| Field | Type | Default | Description |
|---|---|---|---|
| `single` | float | 0.85 | Scale factor for 1 character |
| `double` | float | 0.78 | Scale factor for 2 characters |
| `triple` | float | 0.70 | Scale factor for 3 characters |
| `bottomOffset` | float | 0.0 | Vertical offset in pixels |

### How Scale Works

Scale factors are **relative to screen height** (1080 logical pixels).

**Example**: `single: 0.85`:
- Target height = 1080 × 0.85 = 918 pixels
- A 1800-pixel-tall sprite is scaled by 918/1800 = 0.51
- The sprite's **bottom edge** aligns to the dialogue box top

### Recommended Values

| Style | single | double | triple | Notes |
|---|---|---|---|---|
| **Cinematic** (large) | 0.95 | 0.88 | 0.82 | For close-up scenes |
| **Standard** (default) | 0.85 | 0.78 | 0.70 | Balanced |
| **Wide** (small) | 0.75 | 0.68 | 0.60 | Panoramic |
| **Minimal** | 0.65 | 0.60 | 0.55 | Background-friendly |

### Adjusting for Wide Sprites

If your sprites are wider than usual (e.g., 1600×1800), you may need to
**reduce** the scale factors to avoid overlap:

```json
"characterScaling": {
  "single": 0.85,
  "double": 0.72,
  "triple": 0.62
}
```

### Bottom Offset

**Positive** values move characters **down**:

```json
"bottomOffset": 50
```

**Negative** values move characters **up**:

```json
"bottomOffset": -30
```

**Use cases**:
- Positive: Characters appear at a lower position
- Negative: Characters appear at a higher position

**Range**: -200 to +200 (beyond this, characters may be cut off)

### Applying Changes

After editing `ui_theme.json`:

```bash
python -m devtools.scripts.gality_packer assets data.pak
```

Then restart the game.

**In dev builds**: Hot reload is not currently supported for character scaling.
You must restart.

---

## 8. Expression Management

### Recommended Workflow

**Step 1: Plan expressions**

For each character, decide which expressions are needed:

```
senior:
  - normal (default)
  - smile
  - happy
  - surprised
  - pout (annoyed)
  - fear (scared)
  - corrupt (horror)

junior:
  - normal
  - smile
  - happy
  - surprised
  - pout
  - smug
  - panic
  - corrupt

president:
  - normal
  - arms (crossed)
  - smile
  - serious
  - mad
  - glitch
```

**Step 2: Generate sprites**

Using AI (`generate_assets.py`) or manual drawing.

**Step 3: Verify consistency**

Open all expressions side-by-side. Check:
- Hair color and style
- Face shape
- Uniform details
- Eye color
- Skin tone

**Inconsistencies** are common with AI. Consider regenerating or manual touch-up.

**Step 4: Name and place**

```
assets/characters/
├── senior_normal.png
├── senior_smile.png
├── senior_happy.png
├── senior_surprised.png
├── senior_pout.png
├── senior_fear.png
├── senior_corrupt.png
├── junior_normal.png
├── ...
```

**Step 5: Use in scripts**

```gality
[node]
char: assets/characters/senior_normal.png
學姐: ...
-> next
```

### Expression Sequencing

For animated expression changes, use a chain of nodes:

```gality
[emotion_1]
char: assets/characters/senior_normal.png
旁白: 她先是愣了一下。
-> emotion_2

[emotion_2]
char: assets/characters/senior_surprised.png
學姐: 什麼！？
-> emotion_3

[emotion_3]
char: assets/characters/senior_pout.png
學姐: 你怎麼會這樣說！
-> emotion_4

[emotion_4]
char: assets/characters/senior_smile.png
學姐: 不過...算了吧。
-> next
```

Each node changes the sprite. The player sees each expression as they advance.

### Using `active_char` with Expressions

When changing expressions **while keeping the character active**:

```gality
[node_01]
char_left: assets/characters/senior_normal.png
char_right: assets/characters/junior_normal.png
active_char: left
學姐: 我覺得...
-> node_02

[node_02]
char_left: assets/characters/senior_pout.png
char_right: assets/characters/junior_normal.png
active_char: left
學姐: 算了，不說了。
-> node_03
```

At `node_02`, only the left sprite changes. The right sprite persists.

**Important**: When changing a sprite, always respecify **all** slots that
should remain visible, **or** rely on persistence.

**Persistence works**:

```gality
[node_01]
char_left: assets/characters/senior_normal.png
char_right: assets/characters/junior_normal.png
active_char: left
學姐: 我覺得...
-> node_02

[node_02]
char_left: assets/characters/senior_pout.png
active_char: left
學姐: 算了，不說了。
-> node_03
```

The right sprite **persists** from `node_01` to `node_02`, even though it's
not specified at `node_02`.

### Bulk Sprite Replacement

If you want to update an expression across many nodes:

**Option 1: Manual find-replace**

Use VS Code's find-replace:

```
Find:    senior_normal.png
Replace: senior_happy.png
```

**Option 2: Script-based**

Write a Python script to rewrite `.gality` files.

**Option 3: Sprite file replacement**

Overwrite `senior_normal.png` with new art. All scripts using it will
auto-update.

---

## 9. Advanced Patterns

### Pattern 1: Character Entrance

Make a character appear dramatically:

```gality
[entrance_1]
bg: assets/bg/classroom.jpg
旁白: 門突然打開了。
-> entrance_2

[entrance_2]
bg: assets/bg/classroom.jpg
trans: wipe_left
duration: 0.5
char: assets/characters/senior_surprised.png
shake: 0.3
學姐: 抱歉，我遲到了！
-> entrance_3

[entrance_3]
char: assets/characters/senior_smile.png
學姐: 讓你們久等了。
-> next
```

### Pattern 2: Character Exit

```gality
[exit_1]
char: assets/characters/senior_normal.png
學姐: 我該走了。
-> exit_2

[exit_2]
char: 
旁白: 她轉身離開。
-> exit_3
```

### Pattern 3: Expression Close-Up

```gality
[closeup_1]
char: assets/characters/senior_normal.png
旁白: 她慢慢抬起頭。
-> closeup_2

[closeup_2]
char: assets/characters/senior_corrupt.png
shake: 0.5
no_skip: true
wait: 1.5
學姐: <glitch>你... 看見了嗎？</glitch>
-> next
```

### Pattern 4: Group Scene

```gality
[group]
char_left: assets/characters/senior_normal.png
char_center: assets/characters/president_normal.png
char_right: assets/characters/junior_normal.png
active_char: center
社長: 大家都到齊了。
-> group_dialog_1

[group_dialog_1]
active_char: left
學姐: 社長，關於新專案...
-> group_dialog_2

[group_dialog_2]
active_char: right
學妹: 我已經完成初步架構了。
-> group_dialog_3

[group_dialog_3]
active_char: center
社長: 很好，我們開始吧。
-> next
```

### Pattern 5: Empathy Sequence

Characters react to each other:

```gality
[reaction_1]
char_left: assets/characters/senior_happy.png
char_right: assets/characters/junior_pout.png
active_char: left
學姐: 羽月，我們成功了！
-> reaction_2

[reaction_2]
char_left: assets/characters/senior_happy.png
char_right: assets/characters/junior_surprised.png
active_char: right
學妹: 這... 這也太快了吧！？
-> reaction_3

[reaction_3]
char_left: assets/characters/senior_smile.png
char_right: assets/characters/junior_happy.png
active_char: right
學妹: 不過... 算是不錯啦。
-> next
```

### Pattern 6: Fade Between Expressions

Use transitions to smooth the change:

```gality
[fade_1]
char: assets/characters/senior_normal.png
旁白: 她的表情變得柔和。
-> fade_2

[fade_2]
char: assets/characters/senior_smile.png
trans: dissolve
duration: 0.3
旁白: ...
-> next
```

**Note**: The transition applies to the **background**, not the character.
Character changes are instantaneous.

**To create a fade**: Would require engine modification (not supported in
v0.3.0).

---

## 10. Troubleshooting

### "Character sprite doesn't appear"

**Possible causes**:

1. **File doesn't exist**: Check `assets/characters/senior_normal.png`
2. **Wrong path in script**: Check `char: assets/characters/senior_normal.png`
3. **`data.pak` not rebuilt**: Run the packer
4. **Transparent PNG issue**: Image might be entirely transparent

**Fix**: Run the game from a terminal to see error messages.

### "Character appears as a rectangle"

**Cause**: The PNG has an opaque background.

**Fix**: Remove the background. See [Assets Guide § 4](assets.md#4-character-sprites).

### "Character has green edges"

**Cause**: Green screen not fully removed.

**Fix**: Apply green spill removal. See [Assets Guide § 4](assets.md#4-character-sprites).

### "Character is too small / too large"

**Fix**: Adjust `characterScaling` in `ui_theme.json`. See [Section 7](#7-scaling-configuration).

### "Character is cut off at the top"

**Cause**: Character is taller than the visible area above the dialogue box.

**Fix**:
1. Reduce the scale factor (`single: 0.75` instead of `0.85`)
2. Or adjust `bottomOffset` to move the character up
3. Or crop sprites to be shorter (e.g., waist-up instead of full-body)

### "Characters overlap when 3 are shown"

**Cause**: Sprites are too wide for 480-pixel slot spacing.

**Fix**:
1. Reduce `triple` scale (e.g., `0.60`)
2. Or create narrower sprites
3. Or accept the overlap (it's common)

### "Active character isn't dimming others"

**Possible causes**:

1. `active_char` not specified
2. `active_char` doesn't match a visible slot
3. Dimming colors modified in `LayerRenderer.hpp`

**Fix**: Check the script's `active_char` directive.

### "Voice doesn't pan left/right"

**Cause**: `active_char` not specified, or panning disabled.

**Fix**: Specify `active_char: left` (or right) before the `cv:` directive.

### "Character doesn't persist between nodes"

**Cause**: Another `char_*` directive overwrote it, or an empty value removed it.

**Fix**: Check all nodes between the character's introduction and disappearance.

### "Sprite flickers or stutters"

**Cause**: Sprite is being reloaded every frame.

**Check**: Is the sprite path changing in every node? If so, only change
it when the expression actually changes.

**Bad**:
```gality
[node_01]
char: assets/characters/senior_normal.png
學姐: 第一句。
-> node_02

[node_02]
char: assets/characters/senior_normal.png    ← Redundant
學姐: 第二句。
-> node_03
```

**Good**:
```gality
[node_01]
char: assets/characters/senior_normal.png
學姐: 第一句。
-> node_02

[node_02]
學姐: 第二句。    ← Sprite persists
-> node_03
```

### "Character scaling changed but didn't apply"

**Cause**: `ui_theme.json` was edited but not repacked.

**Fix**:
```bash
python -m devtools.scripts.gality_packer assets data.pak
```

Then restart the game.

### "Font in name box doesn't match character"

**Cause**: Not applicable — name box font is separate from character.

**Fix**: Adjust `nameFont` in `ui_theme.json`.

### "Character appears behind the dialogue box"

**This is expected**. Characters are rendered **before** the dialogue box,
so the dialogue box overlays them.

**The character's lower body is intentionally hidden by the dialogue box.**

---

## Related Documentation

- **[Script Language](script_language.md)** — Full DSL reference
- **[Assets Guide](assets.md)** — Sprite preparation and rembg
- **[UI Guide](ui_guide.md)** — `ui_theme.json` reference (character scaling)
- **[Getting Started](getting_started.md)** — First steps

---

## Quick Reference

```gality
// Single character
char: assets/characters/senior_normal.png

// Multi-slot
char_left: assets/characters/senior_normal.png
char_center: assets/characters/president_normal.png
char_right: assets/characters/junior_normal.png

// Active speaker (dims others)
active_char: left | center | right

// Voice (panned by active_char)
cv: assets/audio/cv_01.ogg

// Remove character
char_left: 
```

```json
// Scaling configuration (ui_theme.json)
"characterScaling": {
  "single": 0.85,
  "double": 0.78,
  "triple": 0.70,
  "bottomOffset": 0.0
}
```

**Slot positions** (fixed):

| Slot | X Position |
|---|---|
| Left | 25% |
| Center | 50% |
| Right | 75% |

---

**Last updated**: 2026-09-17
