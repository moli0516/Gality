
# Frequently Asked Questions

Answers to the most common questions about Gality Engine.

**Can't find your question?** See [Getting Help](#getting-help) at the bottom.

---

## Table of Contents

1. [General](#1-general)
2. [Installation & Setup](#2-installation--setup)
3. [Writing Scripts](#3-writing-scripts)
4. [UI Customization](#4-ui-customization)
5. [Assets](#5-assets)
6. [Engine Development](#6-engine-development)
7. [Publishing](#7-publishing)
8. [Getting Help](#getting-help)

---

## 1. General

### What is Gality?

Gality is a **2D visual novel and narrative game engine** written in
**C++17** and **SFML 3.x**. It's designed for creators who want:

- **Maximum freedom** — everything is data-driven
- **Pure text workflow** — no GUI editor required
- **Lightweight runtime** — ~48 MB, zero GC, 60 FPS
- **Full source access** — MIT licensed, C++ core

See [README](../README.md) for the full introduction.

### Who is Gality for?

- **Writers** who want to use VS Code and text files, not a GUI editor
- **Engineers** who want full control over the rendering pipeline
- **Technical creators** who value transparency over convenience

**Not for**:
- People who want drag-and-drop tools
- People who don't use a command line
- People who need mobile/web deployment today

### Is Gality free?

Yes. Gality is released under the **MIT License**. You can:

- Use it commercially
- Modify it
- Redistribute it
- Sublicense it

The only requirement: keep the copyright notice.

### Is Gality open source?

Yes. Full source code is available at
[github.com/moli0516/Gality](https://github.com/moli0516/Gality).

### How does Gality compare to Ren'Py?

| Aspect | Ren'Py | Gality |
|---|---|---|
| Language | Python-like `.rpy` | `.gality` DSL |
| Runtime | Python + Pygame | C++17 + SFML |
| Size | ~200 MB | ~48 MB |
| GUI editor | Launcher included | None (use VS Code) |
| Ecosystem | 8000+ games, huge community | Personal project |
| Mobile/Web | Yes | Not yet |
| Learning curve | Lower | Lower (if you know CLI) |

**Ren'Py** is more mature. **Gality** is more controllable.

### How does Gality compare to Unity + Naninovel?

Unity + Naninovel is a **GUI-first commercial** solution. Gality is a
**text-first open source** solution.

Unity is right for you if:
- You want GUI tools
- You need mobile/web
- You want asset store content

Gality is right for you if:
- You prefer text files over GUI
- You want Git-friendly version control
- You want to understand every line of the engine

### What's the difference between `build_dev.bat` and `build_prod.bat`?

| Build | Debug Tools | Optimization | Use Case |
|---|---|---|---|
| **Dev** (`build_dev.bat`) | ✅ Included (F1, F2, HotReload) | `RelWithDebInfo` | Development |
| **Prod** (`build_prod.bat`) | ❌ Excluded | `Release` | Distribution |

**Always ship prod builds** to players. Dev builds include tools that allow
cheating and save manipulation.

### Does Gality work offline?

Yes. Gality is a **local desktop application**. It:

- Does not connect to the internet
- Does not phone home
- Does not require accounts

The only network usage is if you run `generate_assets.py` (which calls AI APIs).

### Is Gality a "reinvention of the wheel"?

No. Gality uses proven libraries at every layer:

- **SFML** for graphics and audio
- **nlohmann/json** for parsing
- **FreeType** for fonts
- **vcpkg** for dependencies
- **Git** for version control
- **VS Code** for editing

What Gality builds is the **layer that doesn't exist**: a text-first,
zero-GC, C++ visual novel engine.

See [README § Design Philosophy](../README.md#-design-philosophy) for details.

---

## 2. Installation & Setup

### What are the system requirements?

**Minimum**:
- Windows 10/11 or macOS 11+
- 4 GB RAM
- 500 MB disk space (for engine + demo)
- OpenGL 2.1 compatible GPU

**Recommended**:
- 8 GB RAM
- 2 GB disk space (for your own assets)

See [Platform Support](../README.md#-platform-support) for tested platforms.

### Do I need a C++ compiler?

**No** — unless you want to build from source.

If you download a **pre-built binary**, you only need:

- Python 3.8+ (for the compiler and packer)
- A text editor (VS Code recommended)

If you **build from source**, you need:

- C++17 compiler (MSVC 2019+, GCC 10+, Clang 11+)
- CMake 3.20+
- vcpkg (recommended)

### Do I need to know C++?

**No.** You can write entire visual novels without touching C++.

You write in:
- `.gality` DSL for scripts
- `ui_theme.json` for UI

The C++ core is only for engine developers.

### How do I install VS Code extension?

1. Open VS Code
2. Go to **Extensions** (`Ctrl+Shift+X` / `Cmd+Shift+X`)
3. Click `...` → **Install from VSIX...**
4. Select `gality-language-support-1.0.0.vsix`

Or via command line:

```bash
code --install-extension gality-language-support-1.0.0.vsix
```

### "Failed to load script" — What do I do?

**Cause**: The compiler didn't run, or `demo_long.json` is missing.

**Fix**:
```bash
python -m devtools.scripts.gality_compiler \
    assets/scripts/main_story_multi.gality \
    assets/scripts/demo_long.json
```

Then restart.

### "vcpkg not found" — What do I do?

**Fix** (Windows):
```bash
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg install
```

The `build_dev.bat` script does this automatically if `vcpkg` is missing.

### Can I build on Linux?

**Not officially tested**, but it should work. See
[Platform Support](../README.md#-platform-support).

Try:
```bash
./build_game.sh
```

If it works, please [open a PR](https://github.com/moli0516/Gality/pulls)
to update the platform support table.

### Can I use WSL (Windows Subsystem for Linux)?

**Not recommended**. Gality is a desktop app that needs OpenGL. WSL's
OpenGL support is limited.

Use native Windows or macOS.

### Can I run Gality in a Docker container?

**Theoretically yes, practically no**. Gality needs:

- A window system (X11, Wayland)
- OpenGL acceleration

Docker containers are typically headless. You could use X11 forwarding,
but the performance is poor.

**Not a supported use case.**

---

## 3. Writing Scripts

### What's the syntax for dialogue?

```
SpeakerName: Dialogue text here.
```

Examples:

```gality
旁白: 這是一句旁白。
學姐: 這是學姐的台詞。
: 這句沒有名字框。
```

See [Script Language § 4](script_language.md#4-dialogue-lines) for full syntax.

### Can I have multi-line dialogue in one node?

**Not yet**. Each `speaker: text` line is a separate dialogue event.

To show multiple lines:

```gality
[node]
旁白: 第一句。
-> node_next

[node_next]
旁白: 第二句。
-> node_next2
```

**Future**: Multi-line single-node support is planned.

### How do I create branching choices?

```gality
[choice_01]
? [選項一] -> target_a
? [選項二] -> target_b
```

Each `?` line is a clickable button.

See [Script Language § 5](script_language.md#5-choices).

### How do I check a variable in a condition?

```gality
[check]
IF favorability >= 50 THEN good_route ELSE bad_route
```

Operators: `>=`, `<=`, `>`, `<`, `==`.

**Important**: Both `THEN` and `ELSE` are required.

### Can I use `AND` / `OR` in conditions?

**Not directly**. Use **sequential condition nodes**:

```gality
[check_1]
IF flag_a >= 1 THEN check_2 ELSE fail_route

[check_2]
IF flag_b >= 1 THEN success_route ELSE fail_route
```

The first matching condition wins.

### Can I compare strings?

**No**. Gality's Blackboard only stores integers. Use integers for
everything (0/1 for booleans, numbers for counters).

### How do I stop BGM?

```gality
[node]
bgm: 
旁白: 音樂停止。
-> next
```

An empty value stops the current track.

### How do I remove a character?

```gality
[node]
char_left: 
旁白: 左邊的角色消失了。
-> next
```

An empty value removes the character.

### Can I use `\n` in dialogue?

**No**. Gality handles line breaking automatically based on the dialogue
box width.

If you want a hard line break, split into separate nodes.

### How do I add a pause in the middle of a line?

Use `<w=N>`:

```gality
旁白: 我...<w=1.0>其實...<w=0.5>一直...
```

See [Script Language § 8](script_language.md#8-rich-text-tags).

### How do I make text appear faster or slower?

Use `<speed=N>`:

```gality
旁白: <speed=0.02>快速文字</speed>
旁白: <speed=0.15>慢速文字</speed>
```

- `N` = seconds per character
- Default: `0.04`

### How do I prevent the player from skipping a line?

Use `no_skip: true` and optionally `wait: N`:

```gality
[dramatic]
no_skip: true
wait: 2.0
幽靈: 你必須看完這句話。
-> next
```

See [Script Language § 9](script_language.md#9-node-attributes).

### What's the difference between `<speed>` and `<w>`?

| Tag | Effect |
|---|---|
| `<speed=N>` | Changes typewriter speed for the enclosed text |
| `<w=N>` | Inserts a one-time pause at this point |

You can combine them:

```gality
旁白: <speed=0.1>慢慢說...<w=2.0>然後停頓。</speed>
```

### Can I load a save from a different build?

**Usually yes**, but not guaranteed. Save files store:

- Current node ID
- Blackboard variables
- Presentation state (background, characters, weather)

If the target node no longer exists, the load fails gracefully.

### Why does my character sprite have a green outline?

**Cause**: Green screen wasn't removed properly.

**Fix**: Use `rembg` with alpha matting, then apply green spill removal.
See [Assets Guide § 4](assets.md#4-character-sprites).

### Why is my CJK text rendering as boxes?

**Cause**: Font doesn't include CJK glyphs.

**Fix**: Use a CJK-compatible font. See [Assets Guide § 8](assets.md#8-fonts).

**Recommended**: NaikaiFont, Noto Sans CJK, Source Han Sans.

### The compiler says "dangling node pointer" — What's wrong?

**Cause**: A `->` target doesn't exist.

```gality
[node_01]
旁白: Hello
-> node_999    ← node_999 doesn't exist
```

**Fix**: Either:
1. Create `[node_999]`
2. Change the target to an existing node

The compiler will tell you the exact line number.

### The compiler says "duplicate node ID" — What's wrong?

**Cause**: Two `[node_id]` declarations with the same ID.

**Fix**: Rename one of them.

```gality
[node_01]
...

[node_01]   ← Duplicate!
...
```

### Can I split my script into multiple files?

**Not yet**. Gality compiles a single `.gality` file to a single JSON AST.

**Workaround**: Use a build step to concatenate files before compiling.

**Future**: Multi-file support is on the roadmap.

### Can I use comments?

Yes:

```gality
// This is a comment
# This is also a comment

[node]
bg: assets/bg/classroom.jpg    // Inline comment
旁白: 你好。
-> next
```

Both `//` and `#` work.

---

## 4. UI Customization

### How do I change the dialogue box color?

Edit `assets/config/ui_theme.json`:

```json
"dialogueBox": {
  "bgColor": [40, 20, 60, 220]
}
```

Then repack:

```bash
python -m devtools.scripts.gality_packer assets data.pak
```

See [UI Guide § 4](ui_guide.md#4-dialogue-box).

### How do I use a custom UI image?

1. Prepare a PNG with decorative corners
2. Place it in `assets/ui/`
3. Enable nine-slice:

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

See [UI Guide § 7](ui_guide.md#7-nine-slice-images).

### How do I know what `left/right/top/bottom` values to use?

Measure the corner decoration in your image editor. The values should
cover the **entire corner decoration**.

Use the helper script:

```bash
python devtools/private/measure_bounds.py assets/ui/my_box.png
```

**Tip**: If in doubt, use larger values. Too-large values only waste
stretchable space; too-small values cause distortion.

### My nine-slice image looks stretched — What's wrong?

**Cause**: `left/right/top/bottom` values are too small.

**Fix**: Increase them until the corner decorations are fully preserved.

### Can I have multiple UI themes?

**Not directly**, but you can:

1. Keep multiple `ui_theme.json` files (e.g., `ui_theme_dark.json`)
2. Copy the one you want to `assets/config/ui_theme.json`
3. Repack

**Future**: Runtime theme switching is on the roadmap.

### How do I hide the dialogue box temporarily?

In a script, use the `hide` button or press `H`.

To hide programmatically, the current API is not exposed to scripts.
You can use `H` in-game.

### Can I change the text font size?

Yes, in `ui_theme.json`:

```json
"dialogueBox": {
  "dialogueFontSize": 36,
  "nameFontSize": 30
}
```

### Can I use bold / italic fonts?

Yes, if you have the font files:

1. Place `font_bold.ttf` in `assets/fonts/`
2. Register in `ui_theme.json`:

```json
"fonts": {
  "default": "regular",
  "available": {
    "regular": "assets/fonts/font.ttf",
    "bold": "assets/fonts/font_bold.ttf"
  }
}
```

3. Use in scripts:

```gality
旁白: <font=bold>粗體文字</font>
```

### Can I change the button positions?

Yes, in `ui_theme.json`:

```json
"dialogueButtons": {
  "position": "top-right"
}
```

Values: `bottom-right`, `top-right`, `bottom-left`, `top-left`.

### Can I add custom buttons?

**Not in v0.3.0**. The available button IDs are fixed:

`auto`, `skip`, `backlog`, `save`, `load`, `hide`, `menu`.

**Future**: Custom buttons are on the roadmap.

### How do I disable specific buttons?

Set `enabled: false` in the button list:

```json
"buttons": [
  { "id": "auto", "text": "AUTO", "enabled": true },
  { "id": "skip", "text": "SKIP", "enabled": false }
]
```

Disabled buttons are not rendered.

### How do I change character scaling?

In `ui_theme.json`:

```json
"characterScaling": {
  "single": 0.85,
  "double": 0.78,
  "triple": 0.70
}
```

See [UI Guide § 10](ui_guide.md#10-character-scaling).

### Can I change character slot positions?

**Not in v0.3.0**. Slots are fixed at 25% / 50% / 75% of screen width.

**Future**: Configurable positions are on the roadmap.

### Why does my `ui_theme.json` change not appear?

**Cause**: `ui_theme.json` is packed into `data.pak`.

**Fix**: Repack and restart:

```bash
python -m devtools.scripts.gality_packer assets data.pak
```

**In dev builds**: You can use hot reload (F1 → Reload Theme).

---

## 5. Assets

### What image format should I use for backgrounds?

**JPG** at 85-95% quality, **1920×1080** resolution.

See [Assets Guide § 3](assets.md#3-background-images).

### What image format for character sprites?

**PNG with alpha**, minimum 800×1200, transparent background.

See [Assets Guide § 4](assets.md#4-character-sprites).

### What audio format for BGM?

**OGG Vorbis**, 128-192 kbps, stereo, 1-3 minutes loopable.

See [Assets Guide § 7](assets.md#7-audio).

### What audio format for voice?

**OGG** or **WAV**, 128-192 kbps, 1-10 seconds per line.

### What audio format for SFX?

**WAV**, 44.1 kHz, 16-bit, < 1 second.

### How do I remove green screen from character sprites?

Use `rembg`:

```bash
pip install rembg pillow
```

```python
from PIL import Image
from rembg import remove

img = Image.open("sprite_green.png")
output = remove(
    img,
    alpha_matting=True,
    alpha_matting_foreground_threshold=240,
    alpha_matting_background_threshold=15,
    alpha_matting_erode_size=10,
)
output.save("sprite.png")
```

See [Assets Guide § 4](assets.md#4-character-sprites).

### How do I remove green spill from sprites?

See the Python code in [Assets Guide § 4](assets.md#4-character-sprites).

The short version: replace green channel values that exceed the
red-blue average.

### Why do my assets not appear in-game?

**Cause**: `data.pak` wasn't rebuilt.

**Fix**:
```bash
python -m devtools.scripts.gality_packer assets data.pak
```

Then restart.

### Can I use SVG or vector graphics?

**No**. Gality uses raster images (PNG, JPG) via SFML.

**Workaround**: Rasterize SVGs to PNG at 2x resolution before adding.

### Can I use webp or avif?

**No**. SFML 3.x supports PNG, JPG, BMP, TGA, and a few others. Stick to
**PNG** and **JPG**.

### What's the maximum image size?

**Practically**, keep images under 4096×4096. Larger images consume more
memory without visual benefit.

**Recommended**:
- Backgrounds: 1920×1080
- Character sprites: 800×1200 to 1200×1800
- UI images: 256×256 to 1024×1024

### How do I optimize large assets?

- **JPG**: `jpegoptim --max=90`
- **PNG**: `pngquant --quality=80-95`
- **OGG**: `oggenc -q 6`
- **Fonts**: subset with `pyftsubset` (careful with CJK)

See [Assets Guide § 12](assets.md#12-optimization).

### Can I use fonts from Google Fonts?

Yes, if they're licensed for distribution (most OFL fonts are).

**Recommended for CJK**: Noto Sans CJK, Source Han Sans.

**Not recommended**: Fonts without CJK support (if you have CJK content).

### How do I add a new transition mask?

1. Create a 1920×1080 grayscale PNG
2. White = revealed, black = hidden
3. Save as `assets/masks/my_mask.png`
4. Use in scripts: `trans: my_mask`

See [Assets Guide § 6](assets.md#6-transition-masks).

---

## 6. Engine Development

### Do I need to modify the engine?

**Almost never**. Gality is designed so that everything you'd want to
change is data-driven:

- Story → `.gality` scripts
- UI → `ui_theme.json`
- Assets → files in `assets/`

**You only modify the engine if you want**:

- New node types
- New inline tags
- New rendering effects
- New UI widgets

### What's the architecture?

See [Architecture](architecture.md) for details.

**Short version**:

```
ScriptLoader → StoryExecutor → Blackboard
                    ↓
              LayerRenderer → Window
                    ↓
              AudioManager → Speaker
```

### Where's the entry point?

`src/main.cpp` → `GalityApp::initialize()` → `GalityApp::run()`

### How do I add a new node type?

1. Add enum value to `NodeType` in `StoryNode.hpp`
2. Add parsing in `ScriptLoader.hpp`
3. Add execution in `StoryExecutor.hpp`
4. Add rendering in `main.cpp` if needed

### How do I add a new inline tag?

1. Add parsing in `DialogueBox::parseInlineTags()`
2. Add rendering in `DialogueBox::draw()`
3. Update `gality-language-support` extension (optional)

### How do I add a new UI element?

1. Create a class in `src/render/`
2. Instantiate in `GalityApp`
3. Draw in `GalityApp::render()`
4. Handle events in `GalityApp::handleEvents()`

### How do I use custom shaders?

Place `.frag` files in `assets/shaders/` and load them in C++ code. See
[Assets Guide § 9](assets.md#9-shaders).

### How do I profile performance?

Use the **F1 DebugOverlay** → **Diagnostics** tab. It shows:

- FPS
- Frame time
- Memory usage
- Cache stats

### Why does the game crash on startup?

**Common causes**:
1. Missing `data.pak` — run the packer
2. Missing DLLs — copy SFML DLLs from vcpkg
3. Corrupted `data.pak` — repack
4. Missing font — check `assets/fonts/font.ttf`

Run the game from a terminal to see error output.

### Can I use Gality with a different rendering library?

**Technically yes, but not recommended**. Gality is tightly integrated
with SFML's window, texture, and audio APIs. Replacing SFML means
rewriting most of `src/render/`.

### Can I use Gality without SFML?

**No**. SFML is the core dependency for graphics, window management, and audio.

### Is there a headless mode?

**Not currently**. Gality requires a display for rendering.

**Future**: Headless mode for automated testing is on the roadmap.

### How do I add tests?

Currently, there's no test framework. The demo script serves as a manual
test.

**Future**: Unit tests are on the roadmap.

### How do I contribute?

See [CONTRIBUTING.md](../CONTRIBUTING.md).

**Guidelines**:
- No third-party GUI frameworks
- No GC languages in runtime
- No JSON for writers
- Keep the aesthetic

---

## 7. Publishing

### Can I sell a game made with Gality?

Yes. Gality is MIT licensed. You can:

- Sell your game commercially
- Use any price
- Distribute on Steam, itch.io, etc.
- Keep all revenue

The only requirement: include a copy of Gality's MIT license in your
distribution.

### Do I have to credit Gality?

**Legally, no** (MIT doesn't require attribution in the user-facing game).
**Practically, yes** — it helps the project grow.

**Suggested credit**:

> Made with [Gality Engine](https://github.com/moli0516/Gality)

### Can I rebrand Gality?

Yes, under MIT. But:

- Don't misrepresent the origin
- Don't claim you wrote the engine
- Include the MIT notice

### How do I distribute a game?

1. Build with `build_prod.bat` (Windows) or `build_game.sh` (macOS/Linux)
2. Zip the `dist/` folder
3. Upload to itch.io, Steam, etc.

See [Getting Started § 9](getting_started.md#9-packaging-for-distribution).

### How do I protect my assets?

**You can't fully**. `data.pak` uses XOR obfuscation, not real encryption.

**For real protection**:

- Use OS-level file permissions
- Use dedicated DRM (Steam, Denuvo)
- Accept that determined users will extract your assets

See [SECURITY.md](../SECURITY.md) for details.

### What's the distribution size?

- **Engine runtime**: ~48 MB (with SFML DLLs)
- **Assets**: depends on your content
- **Total**: typically 50 MB - 1 GB for a full visual novel

### Can I ship on Steam?

Yes. But you'll need:

- Steam SDK integration (not built into Gality)
- Achievements, cloud saves, etc. (custom implementation)
- Steam's launch configuration

**Future**: Steam integration is on the roadmap.

### Can I ship on itch.io?

Yes. Simple zip upload. No SDK required.

See [Getting Started § 9](getting_started.md#9-packaging-for-distribution).

### Can I ship on mobile (iOS/Android)?

**Not yet**. Gality doesn't have mobile builds. It's a desktop-first engine.

**Future**: Mobile support is not currently on the roadmap.

### Can I ship on the web (WebAssembly)?

**Not yet**. SFML's Emscripten support is limited. Gality will support
WebAssembly when SFML does.

**Future**: Planned for v0.5.0.

### What platforms does Gality support?

| Platform | Status |
|---|---|
| Windows 10/11 | ✅ Tested |
| macOS (Apple Silicon) | ✅ Tested |
| macOS (Intel) | ⚠️ Untested |
| Linux | ⚠️ Untested |
| WebAssembly | 🚧 Planned |
| Mobile | ❌ Not planned |

See [Platform Support](../README.md#-platform-support).

### How do I sign my release?

Generate a SHA-256 checksum:

```bash
# Windows
certutil -hashfile Gality.zip SHA256

# macOS/Linux
shasum -a 256 Gality.zip
```

Publish the checksum alongside your download.

See [SECURITY.md](../SECURITY.md) for details.

### Do I need to include the source code with my release?

**No**. MIT doesn't require source distribution. You only need to include
the MIT license text.

### Can I modify the engine and sell the modified version?

Yes, under MIT. But:

- You must include the original MIT notice
- You can't claim the original engine as your own
- Your modifications can be under any license you choose

### Can I use Gality for commercial projects with a team?

Yes. MIT allows team use, commercial use, and closed-source distribution.

### Do I need to pay royalties?

**No**. Gality is free for any use.

### Where do I report a security issue?

See [SECURITY.md](../SECURITY.md). **Do not** open a public GitHub issue
for security vulnerabilities.

Email: `[your-email@example.com]`

---

## Getting Help

### Where can I find documentation?

- **[Getting Started](getting_started.md)** — 30 minutes to first game
- **[Script Language](script_language.md)** — Complete DSL reference
- **[UI Guide](ui_guide.md)** — `ui_theme.json` reference
- **[Character Guide](character_guide.md)** — Character rendering
- **[Assets Guide](assets.md)** — Asset preparation
- **[Architecture](architecture.md)** — Engine internals

### Where can I ask questions?

- **GitHub Discussions**: [moli0516/Gality/discussions](https://github.com/moli0516/Gality/discussions)
- **GitHub Issues**: For bugs and feature requests

### How do I report a bug?

Use the [bug report template](https://github.com/moli0516/Gality/issues/new?template=bug_report.md).

Include:
- Steps to reproduce
- Expected vs actual behavior
- Your OS and Gality version
- Error logs (run from a terminal)

### How do I request a feature?

Use the [feature request template](https://github.com/moli0516/Gality/issues/new?template=feature_request.md).

Include:
- The problem you're trying to solve
- Your proposed solution
- Alternatives you considered

### Can I contribute code?

Yes. See [CONTRIBUTING.md](../CONTRIBUTING.md).

**Before you start**, review the [guidelines](../CONTRIBUTING.md#code-style).

### Can I contribute documentation?

Yes. Documentation PRs are especially welcome.

Improve:
- Existing docs (`docs/`)
- README
- CHANGELOG

### Can I translate Gality?

**Not yet**. Localization is on the roadmap.

**However**, you can write your visual novel in any language — the
`.gality` DSL supports UTF-8.

### Is there a Discord server?

**Not yet**. If there's enough interest, one might be created.

For now, use GitHub Discussions.

### Where can I find examples?

- `assets/scripts/main_story_multi.gality` — Demo story (~3000 nodes)
- `assets/scripts/test_fx.gality` — Text effects showcase
- [Example](example.md) — Sample projects

### Can I see screenshots?

See the [README](../README.md) and demo content.

### What's the project's roadmap?

See [Roadmap](roadmap.md) and [CHANGELOG](../CHANGELOG.md).

### How can I support the project?

- ⭐ **Star** the repo on GitHub
- 🐛 **Report** bugs you find
- 📝 **Improve** documentation
- 🎨 **Share** your Gality projects
- 💬 **Spread** the word

---

**Last updated**: 2026-09-17
