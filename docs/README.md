# Gality Documentation

**Welcome to Gality's documentation.** This is the entry point for all
guides, references, and tutorials.

If you're new, start with **[Getting Started](getting_started.md)** —
it'll take you from zero to your first running scene in about 30 minutes.

---

## Quick Navigation

### 🎯 I want to...


| Goal                        | Document                                                                |
| --------------------------- | ----------------------------------------------------------------------- |
| **Install Gality**          | [Getting Started § 2](getting_started.md#2-installation)               |
| **Write my first scene**    | [Getting Started § 3](getting_started.md#3-your-first-scene)           |
| **Learn the `.gality` DSL** | [Script Language](script_language.md)                                   |
| **Customize the UI**        | [UI Guide](ui_guide.md)                                                 |
| **Add characters**          | [Character Guide](character_guide.md)                                   |
| **Prepare images/audio**    | [Assets Guide](assets.md)                                               |
| **See working examples**    | [Examples](example.md)                                                  |
| **Package for release**     | [Getting Started § 9](getting_started.md#9-packaging-for-distribution) |
| **Understand the engine**   | [Architecture](architecture.md)                                         |
| **Optimize performance**    | [Performance Tuning](performance_tuning.md)                             |
| **Contribute code**         | [CONTRIBUTING](../CONTRIBUTING.md)                                      |
| **Report a security issue** | [SECURITY](../SECURITY.md)                                              |

### 🧑 I am a...


| Role                       | Recommended Path                                                                                                     |
| -------------------------- | -------------------------------------------------------------------------------------------------------------------- |
| **Complete beginner**      | [Getting Started](getting_started.md) → [Script Language](script_language.md) → [Examples](example.md)             |
| **Writer (no coding)**     | [Getting Started](getting_started.md) → [Script Language](script_language.md)                                       |
| **Artist / asset creator** | [Assets Guide](assets.md) → [Character Guide](character_guide.md)                                                   |
| **UI designer**            | [UI Guide](ui_guide.md)                                                                                              |
| **Engine developer**       | [Architecture](architecture.md) → [Performance Tuning](performance_tuning.md) → [CONTRIBUTING](../CONTRIBUTING.md) |

---

## Documentation Index

### Core Guides

These are the primary documents. Read them in order if you're new.


| Document                                  | Purpose                                        | Length    |
| ----------------------------------------- | ---------------------------------------------- | --------- |
| **[Getting Started](getting_started.md)** | Install Gality, write your first scene, run it | ~30 min   |
| **[Script Language](script_language.md)** | Complete`.gality` DSL reference                | Reference |
| **[UI Guide](ui_guide.md)**               | Customize`ui_theme.json`                       | Reference |
| **[Character Guide](character_guide.md)** | Multi-slot character rendering                 | Reference |
| **[Assets Guide](assets.md)**             | Prepare images, audio, fonts                   | Reference |
| **[Examples](example.md)**                | 9 complete runnable examples                   | Tutorial  |

### Engine Documentation

For developers extending or studying the engine.


| Document                                        | Purpose                              |
| ----------------------------------------------- | ------------------------------------ |
| **[Architecture](architecture.md)**             | Engine internals, subsystem overview |
| **[Performance Tuning](performance_tuning.md)** | Optimization techniques              |

### Project Information

About the project itself.


| Document                                     | Purpose                          |
| -------------------------------------------- | -------------------------------- |
| **[Roadmap](roadmap.md)**                    | Release history and future plans |
| **[FAQ](faq.md)**                            | Common questions and answers     |
| **[CHANGELOG](../CHANGELOG.md)**             | Version-by-version history       |
| **[CONTRIBUTING](../CONTRIBUTING.md)**       | How to contribute                |
| **[CODE_OF_CONDUCT](../CODE_OF_CONDUCT.md)** | Community guidelines             |
| **[SECURITY](../SECURITY.md)**               | Security policy                  |

---

## Reading Paths

### Path 1: Complete Beginner

**Never made a visual novel before.**

1. [Getting Started](getting_started.md) — Install and first scene (30 min)
2. [Example 1: Hello, Gality](example.md#2-example-1-hello-gality) — Minimal script
3. [Example 2: Choices](example.md#3-example-2-choices-and-branching) — Branching
4. [Script Language § 4-7](script_language.md#4-dialogue-lines) — Core syntax
5. [UI Guide § 11](ui_guide.md#11-common-customizations) — Basic styling

**Estimated time**: 2-3 hours to feel comfortable.

### Path 2: Writer (No Coding Background)

**You write stories, not code. You want to learn the DSL.**

1. [Getting Started](getting_started.md) — Install and first scene
2. [Script Language](script_language.md) — Full DSL reference
3. [Example 8: Branching Story](example.md#9-example-8-branching-story-with-endings) — Complete narrative
4. [Example 7: Romance Scene](example.md#8-example-7-romance-scene) — Emotional pacing

**Focus on**:

- Dialogue lines (§ 4)
- Choices (§ 5)
- Conditional branching (§ 6)
- Variables (§ 7)
- Rich text tags (§ 8)

**Skip** (unless curious):

- § 11 Grammar Summary
- § 12 Compiler Behavior

### Path 3: Engine Developer

**You want to modify the engine or add features.**

1. [Architecture](architecture.md) — Understand the design
2. [Script Language § 12](script_language.md#12-compiler-behavior) — How the compiler works
3. [Performance Tuning](performance_tuning.md) — Optimization
4. [CONTRIBUTING](../CONTRIBUTING.md) — How to contribute

**Focus on**:

- Subsystem boundaries
- Data flow
- Where to add code
- Testing approach

### Path 4: Asset Creator

**You prepare images, audio, fonts.**

1. [Assets Guide](assets.md) — All formats and specs
2. [Character Guide § 2](character_guide.md#2-sprite-preparation) — Sprite requirements
3. [UI Guide § 7](ui_guide.md#7-nine-slice-images) — Nine-slice UI images

**Focus on**:

- Image sizes and formats
- Green screen removal (rembg)
- Audio formats (OGG, WAV)
- Font licensing

### Path 5: Just Want to Try It

**You don't want to read much. Just run something.**

1. Download a pre-built binary from [Releases](https://github.com/moli0516/Gality/releases)
2. Extract
3. Run `Gality.exe`
4. Play the demo (~30 minutes of content)

Then come back to [Getting Started](getting_started.md) when you want
to make your own.

---

## FAQ Summary

**Full FAQ**: [faq.md](faq.md)

### Most Common Questions

**Q: Do I need C++ to use Gality?**
A: No. You write stories in `.gality` DSL and configure UI in JSON. See
[FAQ § General](faq.md#1-general).

**Q: How is Gality different from Ren'Py?**
A: Gality is text-first, C++-based, and has no GUI editor. Ren'Py is
Python-based with a launcher. See [FAQ § General](faq.md#1-general).

**Q: What platforms are supported?**
A: Windows 10/11 and macOS (Apple Silicon) are tested. Linux should work
but is untested. See [Platform Support](../README.md#-platform-support).

**Q: Can I sell games made with Gality?**
A: Yes. MIT license. See [FAQ § Publishing](faq.md#7-publishing).

**Q: My assets don't show up. What's wrong?**
A: Re-run the packer: `python -m devtools.scripts.gality_packer assets data.pak`.
See [FAQ § Assets](faq.md#5-assets).

**Q: How do I make text appear slowly?**
A: Use `<speed=0.15>slow text</speed>`. See
[Script Language § 8](script_language.md#8-rich-text-tags).

**Q: Where can I ask questions?**
A: [GitHub Discussions](https://github.com/moli0516/Gality/discussions).

---

## Cheat Sheets

### DSL Cheat Sheet

```gality
// Entry point
@start node_01

// Node
[node_01]
bg: assets/bg/classroom.jpg
bgm: assets/audio/bgm_daily.ogg
char: assets/characters/senior_normal.png
cv: assets/audio/cv_01.ogg
weather: sakura
學姐: 你好。
-> node_02

// Choice
[choice]
? [Option A] -> node_a
? [Option B] -> node_b

// Condition
[check]
IF favorability >= 50 THEN good ELSE bad

// Mutation
[action]
$ favorability += 10
-> next

// Rich tags
旁白: <color=#FF5555>red</color>
旁白: <wave>wave</wave>
旁白: <speed=0.1>slow</speed>
旁白: wait<w=1.0>1 second
旁白: <glitch>glitch</glitch>

// Forced read
[dramatic]
no_skip: true
wait: 2.0
旁白: <glitch><speed=0.5>Something... happened...</speed></glitch>
-> next
```

### Debug Shortcuts


| Key                | Action                                              |
| ------------------ | --------------------------------------------------- |
| **F1**             | DebugOverlay (Blackboard, Node Jumper, Diagnostics) |
| **F2**             | NodeGraphViewer (DAG tree)                          |
| **Tab** / **H**    | Toggle Backlog                                      |
| **Mouse Wheel Up** | Rollback                                            |
| **Backspace**      | Rollback                                            |
| **F5** / **F9**    | Quick Save / Load                                   |
| **K**              | Test screen shake                                   |
| **F12**            | Screenshot                                          |
| **Escape**         | Settings                                            |

### UI Theme Quick Reference

```json
{
  "fonts": {
    "default": "regular",
    "available": { "regular": "assets/fonts/font.ttf" }
  },
  "dialogueBox": {
    "posX": 80, "posY": 700,
    "width": 1760, "height": 300,
    "bgColor": [0, 0, 0, 215],
    "dialogueFontSize": 32,
    "textShadow": { "enabled": true, "offsetX": 2, "offsetY": 2 }
  },
  "characterScaling": {
    "single": 0.85, "double": 0.78, "triple": 0.70
  },
  "choiceUI": {
    "width": 1100, "height": 90,
    "startY": 200, "fontSize": 28
  }
}
```

---

## Documentation Status

This table tracks which documents are complete.


| Document                | Status      |
| ----------------------- | ----------- |
| `getting_started.md`    | ✅ Complete |
| `script_language.md`    | ✅ Complete |
| `ui_guide.md`           | ✅ Complete |
| `character_guide.md`    | ✅ Complete |
| `assets.md`             | ✅ Complete |
| `example.md`            | ✅ Complete |
| `roadmap.md`            | ✅ Complete |
| `faq.md`                | ✅ Complete |
| `architecture.md`       | ✅ Complete |
| `performance_tuning.md` | ✅ Complete |

**Want to improve the docs?** See [CONTRIBUTING](../CONTRIBUTING.md).

**Found an error?** [Open an issue](https://github.com/moli0516/Gality/issues).

---

## External Resources

### Community

- **GitHub**: [moli0516/Gality](https://github.com/moli0516/Gality)
- **Discussions**: [GitHub Discussions](https://github.com/moli0516/Gality/discussions)
- **Issues**: [GitHub Issues](https://github.com/moli0516/Gality/issues)

### Dependencies

- [SFML 3.x](https://www.sfml-dev.org/) — Graphics, audio, window
- [nlohmann/json](https://github.com/nlohmann/json) — JSON parsing
- [FreeType](https://freetype.org/) — Font rendering
- [vcpkg](https://vcpkg.io/) — C++ package manager

### Related Tools

- [VS Code](https://code.visualstudio.com/) — Recommended editor
- [rembg](https://github.com/danielgatis/rembg) — Background removal
- [Audacity](https://www.audacityteam.org/) — Audio editing
- [GIMP](https://www.gimp.org/) — Image editing

### Learning Resources

- [Visual Novel Design](https://en.wikipedia.org/wiki/Visual_novel) — Background
- [Ren'Py Documentation](https://www.renpy.org/doc/html/) — Alternative engine reference
- [Ink](https://www.inklestudios.com/ink/) — Narrative scripting language

---

## Contributing to Documentation

Documentation improvements are **highly welcomed**.

### How to Help

1. **Fix typos** — Small PRs are appreciated
2. **Clarify confusing sections** — If you were confused, others will be too
3. **Add examples** — Real-world examples help everyone
4. **Translate** — (Future) Localization is planned

### Style Guide

- **Be concise** — Every sentence should add value
- **Show, don't tell** — Examples > descriptions
- **Link liberally** — Cross-reference related docs
- **Use tables** — Structured info is easier to scan
- **Include code** — Runnable examples beat abstract explanations
- **Test your changes** — Run the code you write

### Submitting Changes

```bash
git checkout -b docs/improve-getting-started
# Edit files
git commit -m "docs: clarify asset pipeline section"
git push origin docs/improve-getting-started
```

Then open a PR. See [CONTRIBUTING](../CONTRIBUTING.md).

---

## Getting Help

**Can't find what you're looking for?**

1. **Search the docs** — Use `Ctrl+F` / `Cmd+F` or VS Code's search
2. **Check the FAQ** — [faq.md](faq.md) has 100+ answered questions
3. **Ask on Discussions** — [GitHub Discussions](https://github.com/moli0516/Gality/discussions)
4. **Open an issue** — [GitHub Issues](https://github.com/moli0516/Gality/issues)

**When asking for help, include**:

- What you're trying to do
- What you tried
- The exact error message
- Your OS and Gality version

---

<div align="center">

**Gality Engine Documentation**

_Write in `.gality`. Customize in JSON. Extend in C++._

_All three layers are yours._

</div>