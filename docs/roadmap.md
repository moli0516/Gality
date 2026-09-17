
# Roadmap

**Where Gality is, and where it's going.**

This document tracks the project's release history, current work, and
planned features. It is updated on every release.

---

## Guiding Principles

Every feature must respect these principles:

1. **No GUI-first design** — text files are the source of truth
2. **No runtime GC** — C++17, zero garbage collection
3. **No JSON in writers' hands** — DSL for authors, JSON is intermediate
4. **Data over code** — if it can be in a config file, it should be
5. **Decouple everything** — every subsystem must be replaceable
6. **Proven libraries at the base** — SFML, nlohmann/json, FreeType, vcpkg

Features that violate these principles are rejected, even if popular.

---

## Release History

### ✅ v0.1.0 — Initial Release
**Date**: 2026-09-14

The first public release. Established the core architecture.

**Highlights**:
- C++17 core, SFML 3.x rendering
- `.gality` DSL + Python compiler
- Multi-slot character pipeline
- Weather + PostFX + transitions
- Encrypted `data.pak` VFS
- Debug overlay + DAG viewer
- VSCode language extension
- ~3000 node demo story

### ✅ v0.2.0 — Tooling & Polish
**Date**: 2026-09-15

Focused on creator tools and presentation.

**Highlights**:
- 16:9 aspect ratio support
- Data-driven UI theme (`ui_theme.json`)
- Save/Load system with JSON snapshots
- Time Machine rollback (128-step stack)
- Backlog UI with scroll
- Settings UI (audio, graphics, system)
- Hot reload (dev build)
- One-click build scripts

### ✅ v0.3.0 — Text Effects & Nine-Slice
**Date**: 2026-09-17

Focused on narrative expressiveness and UI flexibility.

**Highlights**:
- Nine-slice UI system
- FontManager singleton with fallback
- Text shadow + optional outline
- Rich inline tags: `<wave>`, `<rainbow>`, `<glitch>`, `<speed>`, `<w>`
- Forced read system (`no_skip` + `wait`)
- Custom UI image support
- 60 FPS verified with all text effects enabled

See [CHANGELOG](../CHANGELOG.md) for complete history.

---

## Current Work

### 🚧 v0.4.0 — CG System & Layered Characters
**Target**: Month 2 (October 2026)

Focus: **Character depth and scene variety.**

**Planned Features**:

#### CG System
- CG gallery UI
- CG viewer with zoom and pan
- CG unlock tracking
- `cg:` directive in DSL

**Why**: Visual novels need CG events (key visual moments). Currently,
Gality can only show backgrounds and character sprites.

**Expected Usage**:

```gality
[climax]
cg: assets/cg/sunset_confession.jpg
旁白: 夕陽下的告白...
-> next
```

#### Layered Character Parts
- Blink controller
- Mouth controller
- Expression layer overlays

**Why**: Currently, characters are static sprites. Layered parts allow
subtle animation without full sprite swaps.

**Expected Usage**:

```gality
[character]
char: assets/characters/senior_normal.png
blink: assets/characters/senior_blink.png
mouth: assets/characters/senior_mouth_open.png
學姐: 你好。
-> next
```

#### AI Toolchain
- DeepL API integration for translation
- Azure TTS integration for voice
- Stable Diffusion API integration for image generation
- GPT-4 API integration for dynamic dialogue

**Why**: Reduce creator workload for localization and asset generation.

#### FX Registry (Deferred to v0.5.0)
- User-defined C++ `.hpp` modules for custom effects

**Status**: Designed but not implemented. Postponed to v0.5.0 due to
scope.

---

## Future Plans

### 🔮 v0.5.0 — Web & Platforms
**Target**: Month 3-6 (Nov 2026 - Jan 2027)

Focus: **Reach.**

**Planned Features**:

#### WebAssembly Build
- Emscripten support
- Browser-native execution
- Single-file HTML distribution

**Dependency**: SFML's Emscripten support. If SFML supports it, Gality
will too.

#### Cross-Platform CI/CD
- GitHub Actions for automated builds
- Windows / macOS / Linux binaries on every release
- Automated testing

#### FX Registry
- `FXBase` abstract class
- `FXRegistry` singleton
- Auto-registration via `REGISTER_FX` macro
- User-defined `.hpp` modules in `src/fx/`
- 3-5 example FX modules

#### Steam Preparation
- Steam SDK integration layer
- Achievement API hooks
- Cloud save support

---

### 🌟 v1.0.0 — Stable API
**Target**: Year 1 (2027)

Focus: **Stability and ecosystem.**

**Planned Features**:

#### Stable API Guarantee
- Frozen public API
- Semantic versioning commitment
- Deprecation policy

#### Comprehensive Documentation
- All planned docs complete
- Video tutorials
- Interactive examples

#### Community Showcase
- Gallery of games made with Gality
- Contributor recognition
- Community-created FX/UI packs

#### Commercial License
- Same MIT license, but with explicit commercial support options
- Priority bug fixes for supporters (optional)

**Note**: Gality is and will remain **MIT licensed**. The "Commercial
License" refers to optional support tiers, not a change in licensing.

---

### ❌ Not Planned

These features have been considered and **rejected**.

#### Mobile (iOS / Android)
**Reason**: Gality is a desktop-first engine. Mobile deployment requires
significant architectural changes (touch input, mobile GPUs, app stores).
Not aligned with the project's focus.

#### GUI Editor
**Reason**: Violates the "no GUI-first design" principle. VS Code + debug
tools are the editor. See [README § Design Philosophy](../README.md#-design-philosophy).

#### Node-based Script Editor
**Reason**: Same as GUI editor. `.gality` DSL is the authoring format.

#### 3D Rendering
**Reason**: Gality is a 2D visual novel engine. 3D rendering is out of scope.

#### Physics Engine
**Reason**: Visual novels don't need physics. Other engines handle this.

#### Real-time Multiplayer
**Reason**: Visual novels are single-player. Networked state would
complicate the save/load architecture.

#### Built-in DRM
**Reason**: DRM is not effective and adds complexity. `data.pak` provides
XOR obfuscation only. See [SECURITY.md](../SECURITY.md).

---

## Feature Requests

Feature requests are welcome via
[GitHub Issues](https://github.com/moli0516/Gality/issues).

**What gets accepted**:

- ✅ Features aligned with guiding principles
- ✅ Features solving real creator problems
- ✅ Features that are scoped reasonably
- ✅ Features the author can maintain

**What gets rejected**:

- ❌ Features that violate principles (GUI editors, mobile-first)
- ❌ Features requiring massive scope (3D engine)
- ❌ Features with low creator demand

**If your request is rejected**: The author will explain why. It's not
personal — it's about project focus.

---

## How to Read This Roadmap

### Status Icons

| Icon | Meaning |
|---|---|
| ✅ | Released and stable |
| 🚧 | In active development |
| 🔮 | Planned for future |
| ⚠️ | At risk (may be delayed) |
| ❌ | Not planned |

### Version Numbering

Gality uses [Semantic Versioning](https://semver.org/):

- **Major** (`1.0.0`): Breaking changes
- **Minor** (`0.4.0`): New features, backward compatible
- **Patch** (`0.3.1`): Bug fixes only

**Pre-1.0**: Minor versions may include breaking changes (with
documentation). After 1.0, the API is stable.

### Timeline Reality

**The author works on Gality in their spare time.**

Timelines are **targets, not promises**. Features may:

- Ship earlier than planned (rare)
- Ship later than planned (common)
- Get split across multiple releases
- Get dropped entirely (if scope explodes)

**Please don't plan business decisions around these dates.**

---

## Contributing to the Roadmap

### Want a feature?

1. **Open an issue**: [Feature request template](https://github.com/moli0516/Gality/issues/new?template=feature_request.md)
2. **Describe the problem**: What creator problem does it solve?
3. **Propose a solution**: How would you implement it?
4. **Discuss**: The author will respond within a week

### Want to implement a feature?

1. **Check the roadmap**: Is the feature listed?
2. **Open a PR**: Follow [CONTRIBUTING.md](../CONTRIBUTING.md)
3. **Reference the roadmap**: Link the feature in the PR description
4. **Wait for review**: The author reviews PRs within 2 weeks

### Want to influence priorities?

The best ways:

- **Use the engine**: Real feedback shapes priorities
- **Report bugs**: Bugs get fixed fast
- **Contribute code**: PRs jump the queue
- **Sponsor**: (Future) Financial support will prioritize specific features

**Popular requests** are more likely to ship, but **principle-aligned
requests** are more likely to be accepted.

---

## Related Documents

- **[CHANGELOG.md](../CHANGELOG.md)** — Complete release history
- **[README.md](../README.md)** — Project overview
- **[CONTRIBUTING.md](../CONTRIBUTING.md)** — How to contribute
- **[FAQ](faq.md)** — Common questions about features

---

**Last updated**: 2026-09-17

**Next update**: When v0.4.0 development begins
