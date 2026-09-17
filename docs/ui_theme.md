
# UI Customization Guide

**Gality's UI is entirely data-driven.** Everything you see on screen —
dialogue box, name box, choices, character scaling, buttons — is controlled
by a single JSON file: `ui_theme.json`.

This guide covers:

1. [Where the file lives](#1-where-the-file-lives)
2. [File structure](#2-file-structure)
3. [Fonts](#3-fonts)
4. [Dialogue box](#4-dialogue-box)
5. [Name box](#5-name-box)
6. [Text effects](#6-text-effects)
7. [Nine-slice images](#7-nine-slice-images)
8. [Choice buttons](#8-choice-buttons)
9. [Dialogue buttons](#9-dialogue-buttons)
10. [Character scaling](#10-character-scaling)
11. [Common customizations](#11-common-customizations)
12. [Hot reload (dev build)](#12-hot-reload-dev-build)

---

## 1. Where the File Lives

```
gality/
└── assets/
    └── config/
        └── ui_theme.json    ← This file
```

**Important**: This file is packed into `data.pak`. After editing, you must
re-run the packer:

```bash
python -m devtools.scripts.gality_packer assets data.pak
```

Then restart the game.

---

## 2. File Structure

The file has a flat structure with these top-level keys:

```json
{
  "fonts": { ... },
  "dialogueBox": { ... },
  "characterScaling": { ... },
  "dialogueButtons": { ... },
  "choiceUI": { ... }
}
```

**Everything is optional.** Missing keys use sensible defaults.

**Minimal valid `ui_theme.json`**:

```json
{
  "dialogueBox": {
    "width": 1760,
    "height": 300
  }
}
```

Everything else will use built-in defaults.

---

## 3. Fonts

### Configuration

```json
"fonts": {
  "default": "regular",
  "available": {
    "regular": "assets/fonts/font.ttf",
    "bold": "assets/fonts/font_bold.ttf",
    "italic": "assets/fonts/font_italic.ttf"
  }
}
```

### Fields

| Key | Type | Description |
|---|---|---|
| `default` | string | Which font to use if none specified |
| `available` | object | Map of `name → path` |

### How It Works

- Gality loads each font in `available`
- If a font file is missing, it's **silently skipped** (no crash)
- When the engine requests a font by name, it falls back to `default`
- If `default` itself is missing, the first loaded font is used

### Using Fonts in Nodes

```gality
[node_01]
dialogueFont: bold
nameFont: italic
學姐: 這句台詞用粗體，名字用斜體。
-> next
```

**Or via inline tags**:

```gality
旁白: <font=bold>這是粗體文字。</font>
```

### Font Recommendations

For CJK (Chinese/Japanese/Korean) content:

| Font | License | Notes |
|---|---|---|
| **NaikaiFont-SemiBold** | OFL | Default, good CJK support |
| **Noto Sans CJK** | OFL | Comprehensive glyphs |
| **Source Han Sans** | OFL | Adobe + Google |
| **Taipei Sans TC** | OFL | Taiwan-focused |

**Avoid**:
- Fonts without CJK glyphs (they'll render as boxes)
- Fonts with restrictive licenses (check before distributing)

---

## 4. Dialogue Box

### Configuration

```json
"dialogueBox": {
  "posX": 80,
  "posY": 700,
  "width": 1760,
  "height": 300,
  "bgColor": [0, 0, 0, 215],
  "borderColor": [255, 255, 255, 100],
  "nameBoxPosX": 100,
  "nameBoxPosY": 600,
  "nameBoxWidth": 320,
  "nameBoxHeight": 100,
  "nameBoxBgColor": [40, 40, 90, 235],
  "nameTextColor": [255, 220, 0, 255],
  "dialogueTextColor": [255, 255, 255, 255],
  "nameFontSize": 28,
  "dialogueFontSize": 32,
  "dialogueFont": "regular",
  "nameFont": "regular"
}
```

### Fields

| Key | Type | Default | Description |
|---|---|---|---|
| `posX` | float | 80 | X position (in 1920×1080 coordinate space) |
| `posY` | float | 720 | Y position |
| `width` | float | 1760 | Box width |
| `height` | float | 280 | Box height |
| `bgColor` | [r,g,b,a] | [0,0,0,215] | Background color (fallback if no image) |
| `borderColor` | [r,g,b,a] | [255,255,255,100] | Border color |
| `nameBoxPosX` | float | 80 | Name box X position |
| `nameBoxPosY` | float | 660 | Name box Y position |
| `nameBoxWidth` | float | 320 | Name box width |
| `nameBoxHeight` | float | 60 | Name box height |
| `nameBoxBgColor` | [r,g,b,a] | [40,40,90,235] | Name box background |
| `nameTextColor` | [r,g,b,a] | [255,220,0,255] | Speaker name color |
| `dialogueTextColor` | [r,g,b,a] | [255,255,255,255] | Dialogue text color |
| `nameFontSize` | int | 28 | Speaker name font size |
| `dialogueFontSize` | int | 32 | Dialogue text font size |
| `dialogueFont` | string | "regular" | Font name for dialogue |
| `nameFont` | string | "regular" | Font name for speaker |

### Coordinate System

Gality uses a **1920×1080 logical coordinate space**, regardless of window
size. The engine automatically letterboxes to fit any resolution.

**Reference points**:

```
(0, 0) ────────────────── (1920, 0)
  │                              │
  │      1920 × 1080 canvas      │
  │                              │
  │                              │
  │   ┌──────────────────┐       │
  │   │ Dialogue box     │       │  ← Default: y=720, h=280
  │   └──────────────────┘       │
  │                              │
(0, 1080) ──────────────── (1920, 1080)
```

**Tip**: For a bottom-aligned dialogue box:
```
posY = 1080 - height
```
e.g., `1080 - 300 = 780` for a 300px box.

---

## 5. Name Box

The name box is configured as part of `dialogueBox`. See [Section 4](#4-dialogue-box).

### Positioning

**Common pattern**: Place the name box just above the dialogue box.

```
nameBoxPosY + nameBoxHeight = dialogueBox.posY
```

**Example**:
```json
"dialogueBox": {
  "posY": 700,
  "nameBoxPosY": 600,
  "nameBoxHeight": 100
}
```

`600 + 100 = 700` ✓ — the name box sits directly on top of the dialogue box.

### Hiding the Name Box

To hide it for narration:

```json
"nameBoxHeight": 0
```

Or in the script, leave the speaker empty:

```gality
旁白: 這句沒有名字框。
```

**Actually**, in Gality, `旁白` (narration) is still a speaker. To truly hide:

```gality
: 這是純旁白，沒有名字。
```

(The leading colon with no speaker name hides the name box.)

---

## 6. Text Effects

### Text Shadow

```json
"dialogueBox": {
  "textShadow": {
    "enabled": true,
    "offsetX": 2.0,
    "offsetY": 2.0,
    "color": [0, 0, 0, 180]
  }
}
```

| Field | Description |
|---|---|
| `enabled` | Turn shadow on/off |
| `offsetX`, `offsetY` | Shadow offset in pixels |
| `color` | Shadow color (RGBA) |

**Performance**: 2x draw calls per glyph. Negligible for typical text.

**Recommended**: Keep enabled — it dramatically improves readability.

### Text Outline

```json
"dialogueBox": {
  "textOutline": {
    "enabled": false,
    "thickness": 1.5,
    "color": [0, 0, 0, 255]
  }
}
```

| Field | Description |
|---|---|
| `enabled` | Turn outline on/off |
| `thickness` | Outline width in pixels |
| `color` | Outline color |

**Performance**: 8x draw calls per glyph. **Use sparingly.**

**When to use**:
- ✅ Titles and headings
- ✅ Text over busy backgrounds
- ❌ Long dialogue passages
- ❌ Dialogue with many animated tags

---

## 7. Nine-Slice Images

**Nine-slice** is a technique for stretching UI images without distorting
their corners.

### How It Works

An image is split into 9 regions:

```
┌─────────┬─────────────────┬─────────┐
│ Corner  │     Edge        │ Corner  │
├─────────┼─────────────────┼─────────┤
│         │                 │         │
│  Edge   │     Center      │  Edge   │
│         │                 │         │
├─────────┼─────────────────┼─────────┤
│ Corner  │     Edge        │ Corner  │
└─────────┴─────────────────┴─────────┘
```

- **Corners**: Never stretched (kept as-is)
- **Edges**: Stretched along one axis
- **Center**: Stretched along both axes

### When to Use

Use nine-slice when:
- ✅ Your UI image has decorative corners
- ✅ You want it to fit different sizes
- ✅ You want to avoid distortion

Don't use when:
- ❌ Your image is a simple rectangle
- ❌ Your image is meant for a fixed size

### Configuration

```json
"dialogueBox": {
  "backgroundImage": {
    "enabled": true,
    "texturePath": "assets/ui/dialogue_box.png",
    "left": 70,
    "right": 70,
    "top": 70,
    "bottom": 70
  }
}
```

| Field | Description |
|---|---|
| `enabled` | Turn nine-slice on/off |
| `texturePath` | Path to your UI image |
| `left`, `right`, `top`, `bottom` | How many pixels to reserve for corners |

### Finding the Right Values

**Method 1: Measure manually**

1. Open your image in an editor
2. Measure the corner decoration size in pixels
3. Use those values for `left`, `right`, `top`, `bottom`

**Method 2: Use the helper script**

```bash
python devtools/private/measure_bounds.py assets/ui/dialogue_box.png
```

Output:
```
assets/ui/dialogue_box.png
  Image size: 903 x 893
  Suggested bounds:
    "left":   20,
    "right":  20,
    "top":    20,
    "bottom": 20
```

**Note**: The script finds where non-white pixels start. You may need to
increase these values if your corner decorations extend further.

### Example: Applying Nine-Slice to Choice Buttons

```json
"choiceUI": {
  "normalImage": {
    "enabled": true,
    "texturePath": "assets/ui/choice_normal.png",
    "left": 40,
    "right": 40,
    "top": 15,
    "bottom": 15
  }
}
```

**Test after configuring**: Resize the choice button and check that:
- Corners stay sharp
- Center stretches cleanly
- No visible seams between regions

### Image Format Requirements

- **Format**: PNG with alpha (RGBA)
- **Size**: Larger than `left + right + center` and `top + bottom + center`
- **Background**: Transparent (not white/green)
- **Corners**: Decorative elements must fit inside `left/right/top/bottom`

### Fallback Behavior

If nine-slice is enabled but the image fails to load:
- Gality logs a warning
- Falls back to solid color (`bgColor`)

**The game never crashes.**

---

## 8. Choice Buttons

### Configuration

```json
"choiceUI": {
  "width": 1100,
  "height": 90,
  "spacing": 25,
  "startY": 200,
  "fontSize": 28,
  "normalBgColor": [30, 30, 50, 240],
  "hoverBgColor": [70, 70, 130, 255],
  "normalOutlineColor": [100, 100, 180, 255],
  "hoverOutlineColor": [255, 215, 0, 255],
  "textColor": [255, 255, 255, 255],
  "normalImage": { ... },
  "hoverImage": { ... }
}
```

### Fields

| Key | Type | Default | Description |
|---|---|---|---|
| `width` | float | 1100 | Button width |
| `height` | float | 70 | Button height |
| `spacing` | float | 25 | Vertical space between buttons |
| `startY` | float | 220 | Y position of the first button |
| `fontSize` | int | 28 | Button text size |
| `normalBgColor` | [r,g,b,a] | — | Default button color |
| `hoverBgColor` | [r,g,b,a] | — | Color when hovered |
| `normalOutlineColor` | [r,g,b,a] | — | Default border |
| `hoverOutlineColor` | [r,g,b,a] | — | Hover border |
| `textColor` | [r,g,b,a] | — | Button text color |

### Layout

Buttons are **centered horizontally** and stacked vertically:

```
                ┌──────────────────────┐
                │   Option 1           │  ← startY = 200
                └──────────────────────┘
                          ↕ spacing = 25
                ┌──────────────────────┐
                │   Option 2           │  ← startY + height + spacing
                └──────────────────────┘
                          ↕ spacing
                ┌──────────────────────┐
                │   Option 3           │
                └──────────────────────┘
```

### Recommended Values

**For 1920×1080:**

| Style | width | height | spacing | startY |
|---|---|---|---|---|
| **Compact** | 800 | 70 | 20 | 250 |
| **Standard** | 1100 | 90 | 25 | 200 |
| **Large** | 1400 | 110 | 30 | 180 |

---

## 9. Dialogue Buttons

These are the small buttons in the bottom-right corner (AUTO, SKIP, LOG, etc.).

### Configuration

```json
"dialogueButtons": {
  "enabled": true,
  "position": "bottom-right",
  "buttonWidth": 90,
  "buttonHeight": 36,
  "spacing": 8,
  "fontSize": 14,
  "skipInterval": 0.05,
  "normalBgColor": [40, 45, 60, 220],
  "hoverBgColor": [70, 90, 140, 240],
  "activeBgColor": [100, 130, 200, 250],
  "outlineColor": [80, 90, 110, 255],
  "hoverOutlineColor": [140, 180, 255, 255],
  "textColor": [220, 225, 235, 255],
  "buttons": [
    { "id": "auto",    "text": "AUTO", "enabled": true },
    { "id": "skip",    "text": "SKIP", "enabled": true },
    { "id": "backlog", "text": "LOG",  "enabled": true },
    { "id": "save",    "text": "SAVE", "enabled": false },
    { "id": "load",    "text": "LOAD", "enabled": false },
    { "id": "hide",    "text": "HIDE", "enabled": true },
    { "id": "menu",    "text": "MENU", "enabled": true }
  ]
}
```

### Fields

| Key | Description |
|---|---|
| `enabled` | Turn all buttons on/off |
| `position` | `"bottom-right"`, `"top-right"`, `"bottom-left"`, `"top-left"` |
| `buttonWidth`, `buttonHeight` | Button dimensions |
| `spacing` | Space between buttons |
| `fontSize` | Button text size |
| `skipInterval` | Skip mode advance speed (seconds) |
| Colors | Background and text colors |

### Button IDs

| ID | Function |
|---|---|
| `auto` | Toggle auto-advance |
| `skip` | Hold to skip (stops on choices and forced reads) |
| `backlog` | Open history |
| `save` | Quick save (currently disabled by default) |
| `load` | Quick load (currently disabled by default) |
| `hide` | Hide dialogue box |
| `menu` | Open settings |

**Custom buttons**: Currently, only these IDs are recognized. Custom buttons
require engine modification.

---

## 10. Character Scaling

Control how large characters appear based on how many are on screen.

### Configuration

```json
"characterScaling": {
  "single": 0.85,
  "double": 0.78,
  "triple": 0.70,
  "bottomOffset": 0.0
}
```

### Fields

| Key | Description |
|---|---|
| `single` | Scale when 1 character is on screen |
| `double` | Scale when 2 characters are on screen |
| `triple` | Scale when 3 characters are on screen |
| `bottomOffset` | Vertical offset in pixels (+ lower, - higher) |

### How It Works

- Scale factors are relative to **screen height** (1080 logical pixels)
- `single: 0.85` means the character is 85% of screen height (918 px)
- Characters align their bottom edge to the dialogue box top

### Recommended Values

| Style | single | double | triple |
|---|---|---|---|
| **Cinematic** (close-up) | 0.95 | 0.88 | 0.82 |
| **Standard** (default) | 0.85 | 0.78 | 0.70 |
| **Wide** (panoramic) | 0.75 | 0.68 | 0.60 |

### Slot Positions

Character horizontal positions are **fixed**:

| Slot | Position |
|---|---|
| Left | 25% of screen width |
| Center | 50% |
| Right | 75% |

**These are not configurable** in v0.3.0. If you need custom positions,
modify `LayerRenderer.hpp`.

### Character Dimming

Non-speaking characters are dimmed automatically:

- **Active** character: `sf::Color(255, 255, 255, 255)` (full)
- **Inactive** characters: `sf::Color(140, 140, 150, 220)` (dimmed)

**To disable dimming**: Modify `LayerRenderer.hpp` and remove the dimming
logic. (Or submit a feature request.)

---

## 11. Common Customizations

### Make the Dialogue Box Wider

```json
"dialogueBox": {
  "posX": 40,
  "width": 1840
}
```

### Move the Dialogue Box Higher

```json
"dialogueBox": {
  "posY": 600,
  "height": 400
}
```

### Use a Different Name Box Color

```json
"dialogueBox": {
  "nameBoxBgColor": [80, 20, 20, 235],
  "nameTextColor": [255, 255, 255, 255]
}
```

### Make Choice Buttons Smaller

```json
"choiceUI": {
  "width": 800,
  "height": 70,
  "fontSize": 24
}
```

### Add a Decorative Dialogue Box Image

1. Prepare a nine-slice PNG at `assets/ui/my_box.png`
2. Measure the corner sizes (e.g., 50px each)
3. Configure:

```json
"dialogueBox": {
  "backgroundImage": {
    "enabled": true,
    "texturePath": "assets/ui/my_box.png",
    "left": 50,
    "right": 50,
    "top": 50,
    "bottom": 50
  }
}
```

### Hide Dialogue Buttons

```json
"dialogueButtons": {
  "enabled": false
}
```

### Only Show Specific Buttons

```json
"dialogueButtons": {
  "buttons": [
    { "id": "auto",    "text": "AUTO", "enabled": true },
    { "id": "skip",    "text": "SKIP", "enabled": true },
    { "id": "menu",    "text": "MENU", "enabled": true }
  ]
}
```

Disabled buttons are simply not rendered.

---

## 12. Hot Reload (Dev Build)

In **dev builds** (`build_dev.bat`), you can hot-reload `ui_theme.json`:

1. Open `assets/config/ui_theme.json` in VS Code
2. Edit and save
3. In-game: Press **F1** to open DebugOverlay
4. Click **Reload Theme**

**Note**: Hot reload is only available in dev builds. The product build
requires a restart.

**For faster iteration**:
- Keep the game running
- Edit `ui_theme.json`
- Press F1 → Reload
- See changes immediately

---

## Troubleshooting

### "My UI changes don't appear"

**Cause**: `ui_theme.json` is packed into `data.pak`.

**Fix**:
```bash
python -m devtools.scripts.gality_packer assets data.pak
```

Then restart.

### "My nine-slice image looks stretched"

**Cause**: `left/right/top/bottom` values are too small. Corner decorations
are being stretched.

**Fix**: Increase these values to cover the full corner decoration. See
[Section 7](#7-nine-slice-images).

### "Text is cut off at the bottom"

**Cause**: `dialogueFontSize` is too large for `height`.

**Fix**: Either reduce `dialogueFontSize` or increase `height`.

### "Choice buttons overlap"

**Cause**: `spacing` + `height` exceeds available space.

**Fix**: Reduce `width`, `height`, or `spacing`.

### "Font shows as boxes"

**Cause**: Font doesn't include the required glyphs (e.g., CJK).

**Fix**: Use a CJK-compatible font. See [Section 3](#3-fonts).

---

## Reference: Complete Default

For reference, here's the complete default `ui_theme.json`:

```json
{
  "fonts": {
    "default": "regular",
    "available": {
      "regular": "assets/fonts/font.ttf"
    }
  },
  "dialogueBox": {
    "posX": 80,
    "posY": 700,
    "width": 1760,
    "height": 300,
    "bgColor": [0, 0, 0, 215],
    "borderColor": [255, 255, 255, 100],
    "nameBoxPosX": 100,
    "nameBoxPosY": 600,
    "nameBoxWidth": 320,
    "nameBoxHeight": 100,
    "nameBoxBgColor": [40, 40, 90, 235],
    "nameTextColor": [255, 220, 0, 255],
    "dialogueTextColor": [255, 255, 255, 255],
    "nameFontSize": 28,
    "dialogueFontSize": 32,
    "dialogueFont": "regular",
    "nameFont": "regular",
    "textShadow": {
      "enabled": true,
      "offsetX": 2.0,
      "offsetY": 2.0,
      "color": [0, 0, 0, 180]
    },
    "textOutline": {
      "enabled": false,
      "thickness": 1.5,
      "color": [0, 0, 0, 255]
    }
  },
  "characterScaling": {
    "single": 0.85,
    "double": 0.78,
    "triple": 0.70,
    "bottomOffset": 0.0
  },
  "dialogueButtons": {
    "enabled": true,
    "position": "bottom-right",
    "buttonWidth": 90,
    "buttonHeight": 36,
    "spacing": 8,
    "fontSize": 14,
    "skipInterval": 0.05,
    "buttons": [
      { "id": "auto",    "text": "AUTO", "enabled": true },
      { "id": "skip",    "text": "SKIP", "enabled": true },
      { "id": "backlog", "text": "LOG",  "enabled": true },
      { "id": "save",    "text": "SAVE", "enabled": false },
      { "id": "load",    "text": "LOAD", "enabled": false },
      { "id": "hide",    "text": "HIDE", "enabled": true },
      { "id": "menu",    "text": "MENU", "enabled": true }
    ]
  },
  "choiceUI": {
    "width": 1100,
    "height": 90,
    "spacing": 25,
    "startY": 200,
    "fontSize": 28
  }
}
```

---

**Last updated**: 2026-09-17
