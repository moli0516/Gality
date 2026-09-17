
# Asset Guide

**How to prepare images, audio, and fonts for Gality Engine.**

This guide focuses on **asset preparation**. For scripting, see
[Script Language](script_language.md). For UI configuration, see
[UI Guide](ui_guide.md).

---

## Table of Contents

1. [Directory Structure](#1-directory-structure)
2. [Naming Conventions](#2-naming-conventions)
3. [Background Images](#3-background-images)
4. [Character Sprites](#4-character-sprites)
5. [UI Images (Nine-Slice)](#5-ui-images-nine-slice)
6. [Transition Masks](#6-transition-masks)
7. [Audio](#7-audio)
8. [Fonts](#8-fonts)
9. [Shaders](#9-shaders)
10. [Asset Pipeline](#10-asset-pipeline)
11. [AI-Assisted Generation](#11-ai-assisted-generation)
12. [Optimization](#12-optimization)
13. [Troubleshooting](#13-troubleshooting)

---

## 1. Directory Structure

Gality expects assets in a fixed folder layout. The packer and engine
**will not find them otherwise**.

```
assets/
├── scripts/          ← .gality narrative source + compiled JSON
├── bg/               ← Background images (.jpg / .png)
├── characters/       ← Character sprites (.png, transparent)
├── audio/            ← BGM, voice, SFX (.ogg / .wav)
├── fonts/            ← font.ttf (CJK-compatible recommended)
├── masks/            ← Transition masks (.png, grayscale)
├── shaders/          ← GLSL fragment shaders (.frag)
├── ui/               ← Nine-slice UI images (.png, transparent)
└── config/
    ├── ui_theme.json ← UI layout & styling
    └── settings.json ← Runtime preferences
```

### Rules

- **Case-sensitive** on macOS/Linux. Use lowercase folder names.
- **No spaces** in folder names or file names.
- **Relative paths** — always reference assets as `assets/bg/classroom.jpg`
  from scripts.
- **Do not rename folders** — the engine hardcodes these paths.

---

## 2. Naming Conventions

Gality doesn't enforce naming, but consistency saves hours.

| Type | Convention | Example |
|---|---|---|
| Backgrounds | `location_time.jpg` | `classroom_day.jpg`, `rooftop_sunset.jpg` |
| Character sprites | `character_emotion.png` | `senior_happy.png`, `junior_pout.png` |
| BGM | `bgm_mood.ogg` | `bgm_daily.ogg`, `bgm_horror.ogg` |
| Voice lines | `cv_character_XX.ogg` | `cv_01.ogg`, `cv_jun_15.ogg` |
| SFX | `sfx_name.wav` | `typewriter.wav`, `door_close.wav` |
| UI images | `ui_element.png` | `dialogue_box.png`, `choice_hover.png` |
| Masks | `mask_name.png` | `diamond.png`, `wipe_left.png` |

### Why This Matters

- **Script references stay readable**: `bg: assets/bg/classroom_day.jpg`
  is clearer than `bg: assets/bg/IMG_20230912.jpg`
- **Bulk operations work**: `rm assets/bg/classroom_*.jpg` deletes all
  classroom variants
- **AI generation works**: `generate_assets.py` uses these conventions

### Avoid

- ❌ Spaces: `classroom day.jpg` → use `classroom_day.jpg`
- ❌ Uppercase: `ClassRoom.jpg` → use `classroom.jpg`
- ❌ Non-ASCII: `教室.jpg` → use `classroom.jpg` (Chinese in script, not filenames)
- ❌ Version numbers: `classroom_v2.jpg` → overwrite the original instead

---

## 3. Background Images

### Format

| Property | Recommendation |
|---|---|
| **Format** | JPG (smaller) or PNG (if transparency needed) |
| **Size** | **1920×1080** (matches logical canvas) |
| **Aspect ratio** | 16:9 |
| **Color space** | sRGB |
| **File size** | < 500 KB per image |
| **Quality** | JPG at 85-95% |

### Why 1920×1080?

Gality uses a **1920×1080 logical coordinate space**. Images larger than this:

- Waste memory
- Slow down loading
- Get downscaled anyway

Images smaller than this:

- Get upscaled (blurry)
- Look worse than native

**Always resize to 1920×1080 before adding to `assets/bg/`.**

### Creating Backgrounds

**From photos**:
1. Crop to 16:9
2. Resize to 1920×1080
3. Apply anime/cel-shading filter (if desired)
4. Save as JPG (quality 90)

**From scratch**:
1. Create a 1920×1080 canvas
2. Paint or composite
3. Save as JPG

**From AI**:
- Use a 16:9 aspect ratio
- Prompt for "visual novel background"
- Post-process to remove artifacts
- See [Section 11](#11-ai-assisted-generation)

### Tools

| Tool | Platform | Cost |
|---|---|---|
| Photoshop | Win/Mac | Paid |
| GIMP | Win/Mac/Linux | Free |
| Krita | Win/Mac/Linux | Free |
| Aseprite | Win/Mac/Linux | Paid ($20) |
| Clip Studio Paint | Win/Mac | Paid |

### Checklist

- [ ] 1920×1080 resolution
- [ ] 16:9 aspect ratio
- [ ] Under 500 KB (JPG q=90)
- [ ] No watermarks
- [ ] Named per convention

---

## 4. Character Sprites

### Format

| Property | Recommendation |
|---|---|
| **Format** | PNG with alpha (RGBA) |
| **Size** | 800×1200 to 1200×1800 |
| **Aspect ratio** | 2:3 or 3:4 |
| **Background** | **Transparent** |
| **File size** | < 1 MB per sprite |

### Why Transparent Background?

Character sprites are overlaid on backgrounds. If the PNG has a solid
background, it will render as an **opaque rectangle**, hiding the background.

**Always use transparent PNG.**

### Recommended Sprite Dimensions

Gality displays characters at:

- **Single**: 85% of screen height → ~918 pixels
- **Double**: 78% → ~842 pixels
- **Triple**: 70% → ~756 pixels

**Recommended**: Create sprites at **1200×1800** (for single-character
closeups). They will be scaled down as needed.

**Minimum**: 800×1200. Smaller sprites will look blurry when scaled up.

### Making Sprites

**From AI**:
1. Generate with green screen background
2. Use `rembg` to remove background
3. Post-process to remove green spill
4. Save as PNG

**From hand-drawing**:
1. Draw character
2. Remove background (magic wand, alpha channel)
3. Save as PNG with transparency

**From existing art**:
1. Open in Photoshop/GIMP
2. Use magic wand or select subject
3. Refine edges
4. Save as PNG

### Green Screen Removal

If you generate sprites with AI using green backgrounds:

```bash
pip install rembg pillow
```

```python
from PIL import Image
from rembg import remove

input_image = Image.open("sprite_green.png")
output = remove(
    input_image,
    alpha_matting=True,
    alpha_matting_foreground_threshold=240,
    alpha_matting_background_threshold=15,
    alpha_matting_erode_size=10,
)
output.save("sprite.png", "PNG")
```

**Green spill removal** (if edges are tinted green):

```python
import numpy as np
from PIL import Image

img = Image.open("sprite.png").convert("RGBA")
arr = np.array(img).astype(np.float32)
r, g, b, a = arr[..., 0], arr[..., 1], arr[..., 2], arr[..., 3]

rb_avg = (r + b) * 0.5
green_excess = np.maximum(0, g - rb_avg)
g_new = np.clip(g - green_excess * 0.8 * (a > 10), 0, 255)

arr[..., 1] = g_new
Image.fromarray(arr.astype(np.uint8)).save("sprite_clean.png")
```

See `generate_assets.py` for a complete implementation.

### Expression Variants

For a character with N expressions, create N separate PNGs:

```
senior_normal.png
senior_smile.png
senior_happy.png
senior_surprised.png
senior_pout.png
senior_fear.png
senior_corrupt.png
```

**Tip**: Generate all expressions in the same session for consistent style.

### Checklist

- [ ] PNG with alpha
- [ ] Transparent background
- [ ] 800×1200 minimum size
- [ ] No green spill
- [ ] Clean edges (no halo)
- [ ] Named `character_emotion.png`

---

## 5. UI Images (Nine-Slice)

UI elements like the dialogue box and choice buttons can use **nine-slice**
images for decorative borders.

### Format

| Property | Recommendation |
|---|---|
| **Format** | PNG with alpha |
| **Size** | 256×256 to 1024×1024 |
| **Background** | Transparent |
| **Corners** | Decorative, within `left/right/top/bottom` boundaries |
| **Center** | Simple (solid or subtle gradient) |

### Nine-Slice Rules

```
┌─────────┬─────────────────┬─────────┐
│ Corner  │     Edge        │ Corner  │  ← top
├─────────┼─────────────────┼─────────┤
│         │                 │         │
│  Edge   │     Center      │  Edge   │  ← middle
│         │                 │         │
├─────────┼─────────────────┼─────────┤
│ Corner  │     Edge        │ Corner  │  ← bottom
└─────────┴─────────────────┴─────────┘
  ↑                              ↑
 left                          right
```

| Region | Behavior | Design Constraint |
|---|---|---|
| **Corners** | Never stretched | Decoration must fit entirely |
| **Edges** | Stretched along one axis | Only horizontal OR vertical patterns |
| **Center** | Stretched both ways | Must be solid or gradient |

### Design Guidelines

**Corners**:
- ✅ Decorative (frames, ornaments, circuits)
- ✅ Must fit within `left/right/top/bottom` boundaries
- ❌ No content extending into edges

**Edges**:
- ✅ Solid color
- ✅ Horizontal gradient (top/bottom edges)
- ✅ Vertical gradient (left/right edges)
- ❌ Diagonal patterns
- ❌ Complex textures
- ❌ Text

**Center**:
- ✅ Solid color
- ✅ Subtle gradient
- ✅ Uniform pattern
- ❌ Text
- ❌ Complex patterns

### Sizing

The image must be **larger than the target rendering size in the corner regions**.

For a dialogue box that renders at 1760×300 with `left=70, right=70, top=70, bottom=70`:

- Image must be at least 140px wide (70 + 70)
- Image must be at least 140px tall (70 + 70)
- **Recommended**: 512×512 or larger

### Measuring Boundaries

Use the helper script:

```bash
python devtools/private/measure_bounds.py assets/ui/dialogue_box.png
```

Output:
```
assets/ui/dialogue_box.png
  Image size: 943 x 933
  Suggested bounds:
    "left":   20,
    "right":  20,
    "top":    20,
    "bottom": 20
```

**This is a starting point.** Manually verify that the boundaries fully
enclose the corner decorations.

See [UI Guide § 7](ui_guide.md#7-nine-slice-images) for configuration.

### Common Mistake: Boundaries Too Small

If your corner decoration extends 80 pixels inward but you set `left=40`:

- Corner decoration will be **stretched**
- Visible distortion in the final render

**Fix**: Increase `left/right/top/bottom` to fully cover the decoration.

### Common Mistake: Non-Simple Center

If the center region has a complex pattern (e.g., text, intricate design):

- Pattern will be **stretched**
- Looks broken at different sizes

**Fix**: Keep the center simple (solid or gradient).

### Checklist

- [ ] PNG with alpha
- [ ] Transparent background
- [ ] Corner decorations fully within boundaries
- [ ] Center is solid or gradient
- [ ] Edges only vary along one axis
- [ ] Size ≥ 2× `left + right` and `2× top + bottom`
- [ ] Tested at multiple target sizes

---

## 6. Transition Masks

Transition masks control how the background changes between scenes.

### Format

| Property | Recommendation |
|---|---|
| **Format** | PNG, grayscale |
| **Size** | 1920×1080 |
| **Color space** | Grayscale (single channel) |
| **Values** | Black (0) = hidden, White (255) = revealed |

### How It Works

The transition shader uses the mask to determine reveal order:

1. **Start of transition**: `mask_value > threshold` → old scene
2. **End of transition**: `mask_value > threshold` → new scene
3. **Threshold animates** from 0 to 255 over `duration` seconds

### Built-in Masks

Gality ships with 4 masks in `assets/masks/`:

| Mask | Shape |
|---|---|
| `diamond.png` | Radial diamond from center |
| `wipe_left.png` | Horizontal wipe from left |
| `clock.png` | Clock-style circular sweep |
| `dissolve.png` | Uniform fade (uses solid mid-gray) |

### Creating Custom Masks

**Diamond mask** (built-in): A white diamond on black background.

**Wipe mask**: A linear gradient from black (left) to white (right).

**Radial mask**: A radial gradient from white (center) to black (edges).

**Custom pattern**: Any grayscale gradient will work.

### Example: Radial Mask

```python
from PIL import Image, ImageDraw
import numpy as np

W, H = 1920, 1080
cx, cy = W // 2, H // 2

# Create radial gradient
y, x = np.ogrid[:H, :W]
dist = np.sqrt((x - cx) ** 2 + (y - cy) ** 2)
max_dist = np.sqrt(cx ** 2 + cy ** 2)
mask = np.clip(255 * (1 - dist / max_dist), 0, 255).astype(np.uint8)

Image.fromarray(mask, mode="L").save("assets/masks/radial.png")
```

### Using a Mask

In a script:

```gality
[node]
bg: assets/bg/rooftop.jpg
trans: radial
duration: 2.0
旁白: 場景以放射狀切換。
-> next
```

The engine looks for `assets/masks/radial.png`.

### Checklist

- [ ] 1920×1080
- [ ] Grayscale (or convertible to grayscale)
- [ ] Black = hidden, white = revealed
- [ ] Smooth gradients (avoid hard edges unless intended)
- [ ] Named `mask_name.png`

---

## 7. Audio

Gality supports three audio types: **BGM**, **voice (CV)**, and **SFX**.

### BGM (Background Music)

| Property | Recommendation |
|---|---|
| **Format** | OGG Vorbis (`.ogg`) |
| **Bitrate** | 128-192 kbps |
| **Length** | 1-3 minutes |
| **Looping** | Must loop cleanly |
| **Channels** | Stereo |

**Why OGG?**
- Smaller than WAV (10-20x compression)
- Better quality than MP3 at same bitrate
- No licensing issues (unlike MP3 in some contexts)
- Streaming-friendly (Gality streams from memory)

**Looping**: Ensure the end of the track matches the start for seamless loops.

**Recommended tools**:
- Audacity (free)
- FL Studio (paid)
- Ableton Live (paid)
- LMMS (free)

### Voice Lines (CV)

| Property | Recommendation |
|---|---|
| **Format** | OGG (`.ogg`) or WAV (`.wav`) |
| **Bitrate** | 128-192 kbps (OGG) |
| **Sample rate** | 44.1 kHz |
| **Length** | 1-10 seconds per line |
| **Channels** | Mono or stereo |

**Why OGG for voice?**
- Smaller files
- Fine for short clips
- Audio panning works with both formats

**Naming**: `cv_character_XX.ogg`

Examples:
- `cv_01.ogg` (first line)
- `cv_jun_15.ogg` (junior, line 15)
- `cv_senior_happy.ogg` (senior, "happy" variant)

**Normalization**: Normalize all voice lines to the same loudness (e.g., -16 LUFS).

**Recommended tools**:
- Audacity (free)
- Adobe Audition (paid)
- Reaper (free trial)

### SFX (Sound Effects)

| Property | Recommendation |
|---|---|
| **Format** | WAV (`.wav`) |
| **Sample rate** | 44.1 kHz |
| **Bit depth** | 16-bit |
| **Length** | < 1 second (mostly) |
| **Channels** | Mono or stereo |

**Why WAV for SFX?**
- Minimal latency
- No compression artifacts
- Usually short files (size not an issue)

**Common SFX**:
- `typewriter.wav` — for the typewriter effect
- `click.wav` — for UI buttons
- `door_close.wav` — for scene transitions

### Using Audio in Scripts

```gality
[node]
bgm: assets/audio/bgm_daily.ogg
cv: assets/audio/cv_01.ogg
旁白: 背景音樂和配音同時播放。
-> next
```

See [Script Language § 9](script_language.md#9-node-attributes) for details.

### Stopping Audio

```gality
[node]
bgm: 
旁白: 停止 BGM。
-> next
```

An empty value stops the current track.

### Audio Checklist

- [ ] BGM: OGG, loops cleanly
- [ ] Voice: OGG or WAV, normalized loudness
- [ ] SFX: WAV, short duration
- [ ] Named per convention
- [ ] Placed in `assets/audio/`

---

## 8. Fonts

Gality uses FreeType to render TrueType and OpenType fonts.

### Format

| Property | Recommendation |
|---|---|
| **Format** | TTF or OTF |
| **Size** | Any (FreeType handles rasterization) |
| **CJK support** | Required for CJK content |

### CJK Fonts

For Chinese, Japanese, or Korean content, use a **CJK-compatible font**.

**Recommended**:

| Font | License | Coverage |
|---|---|---|
| **NaikaiFont** | OFL | Full CJK |
| **Noto Sans CJK** | OFL | Full CJK (JP/KR/SC/TC) |
| **Source Han Sans** | OFL | Full CJK |
| **Taipei Sans TC** | OFL | Traditional Chinese |

**Non-CJK fonts** (Arial, Helvetica, etc.) **cannot render CJK glyphs**.
They will show boxes or nothing.

### File Location

Place fonts in `assets/fonts/`:

```
assets/fonts/
├── font.ttf           ← Default (referenced by ui_theme.json)
├── font_bold.ttf      ← Optional
└── font_italic.ttf    ← Optional
```

### Registering Fonts

Add to `assets/config/ui_theme.json`:

```json
"fonts": {
  "default": "regular",
  "available": {
    "regular": "assets/fonts/font.ttf",
    "bold": "assets/fonts/font_bold.ttf"
  }
}
```

See [UI Guide § 3](ui_guide.md#3-fonts) for details.

### Verifying Font Support

To test if a font supports CJK:

```python
from PIL import ImageFont

font = ImageFont.truetype("assets/fonts/font.ttf", 32)
# Check if CJK glyph exists
has_cjk = font.getmask("繁").getbbox() is not None
print("Supports CJK:", has_cjk)
```

### Licensing

**Always check font licenses before distributing.**

- **OFL** (SIL Open Font License) — Free to use, modify, redistribute
- **Apache 2.0** — Free with attribution
- **Commercial** — May require purchase for commercial use
- **Bundled** — Some fonts come with restrictions

Gality does not bundle fonts except the demo's default.

### Tools

- **FontForge** — Free, edit fonts
- **Glyphs** — Paid, Mac only
- **FontLab** — Paid, pro tool

### Checklist

- [ ] TTF or OTF format
- [ ] CJK support (if content is CJK)
- [ ] Licensed for distribution
- [ ] Placed in `assets/fonts/`
- [ ] Registered in `ui_theme.json`

---

## 9. Shaders

Gality uses GLSL fragment shaders for post-processing.

### Format

| Property | Recommendation |
|---|---|
| **Language** | GLSL 1.20 (OpenGL 2.1) |
| **File extension** | `.frag` |
| **Uniforms** | Declared at top |

### Built-in Shaders

| Shader | Purpose |
|---|---|
| `blur.frag` | Gaussian blur (used by PostFX) |
| `transition.frag` | Mask-based scene transitions |

**These are loaded from `data.pak`.** If missing, the engine falls back
gracefully.

### Custom Shaders

To add custom shaders:

1. Place `.frag` file in `assets/shaders/`
2. The engine loads all shaders on startup
3. Reference in C++ code (requires engine modification)

**Note**: Adding new shaders **requires modifying the engine's C++ code**.
Unlike scripts, shaders are not data-driven.

### Writing GLSL 1.20

```glsl
#version 120

uniform sampler2D texture;
uniform vec2 resolution;

void main() {
    vec2 uv = gl_TexCoord[0].xy;
    vec4 color = texture2D(texture, uv);
    gl_FragColor = color;
}
```

**Note**: GLSL 1.20 uses `texture2D`, `gl_FragColor`, `gl_TexCoord`.

### Tools

- **RenderDoc** — Debug shaders (Windows)
- **ShaderToy** — Test shaders online
- **VS Code extension**: `slevesque.shader` for syntax highlighting

### Checklist

- [ ] GLSL 1.20 syntax
- [ ] `#version 120` at top
- [ ] Uniforms declared
- [ ] Placed in `assets/shaders/`

---

## 10. Asset Pipeline

### Full Pipeline

```
1. Create asset (image / audio / font)
       ↓
2. Place in correct folder (assets/bg/, assets/audio/, etc.)
       ↓
3. Run packer: python -m devtools.scripts.gality_packer assets data.pak
       ↓
4. Run engine: build_dev.bat or dist-dev/Gality.exe
       ↓
5. Engine reads data.pak
```

### When to Repack

**Every time you change an asset in `assets/`**, you must re-run the packer:

```bash
python -m devtools.scripts.gality_packer assets data.pak
```

**Why?** The engine reads from `data.pak`, not from `assets/` directly.

**Exception**: In dev builds, some assets (like scripts) support hot reload.
But images and audio always require repacking.

### Script Compilation

For `.gality` scripts, you need an extra step:

```bash
python -m devtools.scripts.gality_compiler \
    assets/scripts/main_story_multi.gality \
    assets/scripts/demo_long.json
```

This converts `.gality` → JSON AST, which is then packed.

### Full Build (Windows)

```bash
build_dev.bat
```

This runs:
1. Compiler
2. Packer
3. CMake + build
4. Package assembly

### Incremental Build

For asset-only changes:

```bash
# Just repack
python -m devtools.scripts.gality_packer assets data.pak

# No need to rebuild the engine
```

### Manifest

The packer generates an internal index mapping paths to byte offsets.
You don't need to maintain a manifest manually.

---

## 11. AI-Assisted Generation

Gality supports AI-generated assets via `generate_assets.py`.

### What It Generates

| Type | Model | Post-processing |
|---|---|---|
| Backgrounds | Grok Imagine | Direct save (JPG) |
| Character sprites | Grok Imagine | `rembg` + green spill removal |
| UI images | Grok Imagine | `rembg` + green spill removal |

### Configuration

Edit the prompt dictionaries in `generate_assets.py`:

```python
CHARACTER_PROMPTS = {
    "assets/characters/senior_normal.png": {
        "prompt": "masterpiece, anime visual novel character sprite, ...",
        "aspect_ratio": "3:4"
    },
    # ...
}
```

### Running

```bash
export XAI_API_KEY="xai-your-key-here"
python generate_assets.py
```

### Features

- **Retry with exponential backoff** (2s, 4s, 8s)
- **Structured logging** (writes to `logs/asset_pipeline_*.log`)
- **Skip existing files** (avoid regenerating)
- **Failure tracking** (JSON summary in `logs/last_run_summary.json`)

### Prompt Tips

**Backgrounds**:
- Always specify `16:9` aspect ratio
- Include `visual novel background`
- Include `anime art style`
- Include `no characters`

**Character sprites**:
- Specify `3:4` aspect ratio
- Include `character sprite`
- Include `upper body, centered`
- Include `solid plain chroma key green background`

**UI elements**:
- Specify `1:1` aspect ratio
- Include `9-slice compatible`
- Include `center area plain and empty`
- Include `solid plain chroma key green background`

### Customization

You can extend `generate_assets.py` to:

- Use a different AI provider (Stable Diffusion, Midjourney)
- Add custom post-processing
- Generate different asset types

See the script for examples.

### Limitations

- AI-generated assets may have **inconsistent styles** across generations
- Character sprites may have **subtle differences** between expressions
- Post-processing is **required** for character sprites (rembg)
- Some generations **fail** (retry logic handles this)

### Verification

After generation, verify:

- [ ] Images are the correct aspect ratio
- [ ] Characters have transparent backgrounds
- [ ] No visible green edges
- [ ] Consistent style across variants
- [ ] Files are in correct folders

---

## 12. Optimization

### Image Optimization

**Backgrounds**:
- JPG quality: 85-95%
- Target size: < 500 KB
- Use `jpegoptim` or `mozjpeg` for lossless re-compression

```bash
jpegoptim --max=90 assets/bg/*.jpg
```

**Character sprites**:
- PNG optimize with `pngquant` or `optipng`

```bash
pngquant --quality=80-95 assets/characters/*.png
```

**UI images**:
- Same as character sprites
- Consider reducing resolution if not needed

### Audio Optimization

**BGM**:
- OGG quality: 5-7 (equivalent to ~128-192 kbps)

```bash
oggenc -q 6 input.wav -o output.ogg
```

**Voice**:
- OGG quality: 4-6
- Remove silence at start/end

```bash
# Trim silence with sox
sox input.wav output.wav silence 1 0.1 1% reverse silence 1 0.1 1% reverse
```

**SFX**:
- Keep WAV (short files)
- Downsample if < 22 kHz is acceptable

### Font Optimization

- Subset fonts to include only used glyphs
- Use `pyftsubset` (from fonttools)

```bash
pyftsubset font.ttf --text-file=used_chars.txt --output-file=font_subset.ttf
```

**Note**: For CJK, subsetting is tricky (thousands of glyphs). Consider
keeping the full font if size is acceptable.

### Total Budget

| Category | Target |
|---|---|
| **Backgrounds** | < 500 KB each |
| **Character sprites** | < 1 MB each |
| **UI images** | < 500 KB each |
| **Audio (BGM)** | < 5 MB each |
| **Audio (voice)** | < 100 KB each |
| **Fonts** | < 20 MB total |
| **Total `data.pak`** | **< 500 MB** |

For comparison, typical visual novels are 500 MB - 5 GB. Gality's ~48 MB
runtime leaves plenty of budget for content.

---

## 13. Troubleshooting

### "Missing asset: assets/bg/classroom.jpg"

**Cause**: Asset doesn't exist, or `data.pak` wasn't rebuilt.

**Fix**:
1. Check that the file exists: `ls assets/bg/classroom.jpg`
2. Rebuild the pack: `python -m devtools.scripts.gality_packer assets data.pak`
3. Restart the game.

### "Character sprite has white/green background"

**Cause**: PNG wasn't properly removed of background.

**Fix**:
- Use `rembg` (see [Section 4](#4-character-sprites))
- Manually remove background in Photoshop/GIMP
- Apply green spill removal

### "Background image looks pixelated"

**Cause**: Image is smaller than 1920×1080.

**Fix**: Upscale or regenerate at 1920×1080.

### "Font renders as boxes (CJK)"

**Cause**: Font doesn't include CJK glyphs.

**Fix**: Use a CJK-compatible font (see [Section 8](#8-fonts)).

### "Audio doesn't play"

**Cause**: File format not supported, or file is corrupted.

**Fix**:
- Check format: `.ogg` for BGM/voice, `.wav` for SFX
- Test file in a media player
- Re-encode with `oggenc` or `ffmpeg`

### "UI image looks stretched / distorted"

**Cause**: Nine-slice boundaries too small (see [Section 5](#5-ui-images-nine-slice)).

**Fix**: Increase `left/right/top/bottom` in `ui_theme.json`.

### "Assets not found after repacking"

**Cause**: Path typo, or file in wrong folder.

**Fix**:
1. Check file location: `ls assets/bg/`
2. Check script reference: `bg: assets/bg/classroom.jpg`
3. Re-run packer
4. Check packer output for warnings

### "Large file size in dist/"

**Cause**: Unoptimized assets.

**Fix**:
- Compress JPGs (`jpegoptim`)
- Compress PNGs (`pngquant`)
- Encode audio at lower bitrate
- Remove unused assets

---

## Appendix: Tools Summary

| Task | Tool | Cost |
|---|---|---|
| **Image editing** | Photoshop | Paid |
| | GIMP | Free |
| | Krita | Free |
| **Background removal** | rembg | Free |
| | Photoshop | Paid |
| **Audio editing** | Audacity | Free |
| | Reaper | Free trial |
| **Font editing** | FontForge | Free |
| **Image optimization** | jpegoptim | Free |
| | pngquant | Free |
| **Audio encoding** | oggenc | Free |
| | ffmpeg | Free |
| **AI generation** | generate_assets.py | Free (API cost) |

---

## Related Documentation

- **[Script Language](script_language.md)** — Using assets in scripts
- **[UI Guide](ui_guide.md)** — Configuring UI assets
- **[Character Guide](character_guide.md)** — Character sprite details
- **[Getting Started](getting_started.md)** — Installation and setup

---

**Last updated**: 2026-09-17
