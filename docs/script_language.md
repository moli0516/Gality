
# Script Language Reference

**The complete reference for `.gality` DSL.**

This document covers everything you need to write visual novels in Gality.
It is a **technical reference**, not a tutorial. If you're new, start with
[Getting Started](getting_started.md) first.

---

## Table of Contents

1. [File Structure](#1-file-structure)
2. [Directives](#2-directives)
3. [Nodes](#3-nodes)
4. [Dialogue Lines](#4-dialogue-lines)
5. [Choices](#5-choices)
6. [Conditional Branching](#6-conditional-branching)
7. [Variables (Blackboard)](#7-variables-blackboard)
8. [Rich Text Tags](#8-rich-text-tags)
9. [Node Attributes](#9-node-attributes)
10. [Comments](#10-comments)
11. [Grammar Summary](#11-grammar-summary)
12. [Compiler Behavior](#12-compiler-behavior)

---

## 1. File Structure

A `.gality` file is a **plain-text script** with UTF-8 encoding.

### Minimal File

```gality
@start start

[start]
旁白: Hello, world.
-> start
```

### Structure

```
@start <entry_node_id>       ← Entry point (required)

[<node_id>]                  ← Node declaration
<attribute>: <value>         ← Node attributes (optional)
<speaker>: <text>            ← Dialogue line
? [<text>] -> <target>       ← Choice (optional)
-> <next_node_id>            ← Destination (required)

[<another_node>]
...
```

### File Naming

- Extension: `.gality` (required)
- Encoding: UTF-8 (with or without BOM)
- Line endings: LF or CRLF (both work)

---

## 2. Directives

### `@start`

Declares the entry point of the script.

```gality
@start node_01
```

**Rules**:
- Must appear **once**
- Must appear **before any node**
- The target must exist as a node

**Default**: If missing, the compiler uses `node_01`.

### `->` (Advance)

Points to the next node.

```gality
[node_a]
旁白: Some text.
-> node_b
```

**Rules**:
- Appears at the **end of a node**
- Only one `->` per node (except in `IF` branches)
- Target must exist

**Special case**: Self-reference keeps the game on the same node:

```gality
[the_end]
旁白: 故事結束。
-> the_end
```

---

## 3. Nodes

A **node** is the basic unit of a script. Every node has:

- An **ID** (unique within the file)
- **Content** (dialogue, choice, or action)
- **A destination** (`->`)

### Node Declaration

```gality
[node_01]
```

**Rules**:
- Must be enclosed in square brackets `[...]`
- ID can contain: letters, digits, underscore `_`, hyphen `-`
- **Recommended**: `snake_case` (e.g., `node_01`, `choice_after_class`)
- Must be **unique** within the file
- Case-sensitive (`Node_01` ≠ `node_01`)

### Node Types

Gality infers the type from content:

| Type | Detected When | Example |
|---|---|---|
| **Dialogue** | Has `speaker: text` | `學姐: 你好。` |
| **Choice** | Has `? [...]` lines | `? [選項] -> node` |
| **Condition** | Has `IF ... THEN ... ELSE` | `IF flag >= 1 THEN a ELSE b` |
| **Action** | Only has `$ var += N` | `$ favor += 10` |

**You don't declare the type** — Gality figures it out.

### Node IDs

**Valid**:
- `node_01`
- `choice-after-class`
- `chapter_1_intro`
- `N001`

**Invalid**:
- `[node 01]` — no spaces
- `[node.01]` — no dots
- `[節點01]` — no non-ASCII IDs (use ASCII for compatibility)

**Convention**: Use `node_NNN` for sequential nodes, `choice_NNN` for choices.

---

## 4. Dialogue Lines

The most common node content: a speaker and their line.

### Format

```gality
<speaker>: <text>
```

### Examples

**Narration** (no speaker box):

```gality
: 純旁白，沒有名字框。
```

**Named speaker**:

```gality
學姐: 你終於來了！
```

**Narration with label** (shows "旁白" in name box):

```gality
旁白: 春天的風吹過校園。
```

**Speaker with special characters**:

```gality
??? : 誰在說話？
```

### Rules

- **Colon is required**: `:` or `：` (both work)
- **Speaker** must not contain `:` or `：`
- **Text** can contain anything except newline
- **Text** can include rich tags (see [Section 8](#8-rich-text-tags))

### Text Wrapping

- Long text **automatically wraps** to the next line
- Wrapping respects CJK punctuation rules (避頭尾禁則)
- Line breaks in source are **ignored** (except for readability)

**Multi-line text**:

```gality
[node]
旁白: 第一行。
旁白: 第二行。
```

These are **two separate dialogue nodes**? No — they're **one node with two
lines**. The engine shows them sequentially after each advance.

Actually, **each `speaker: text` line is its own sub-node**. To have one
advance show multiple lines, use explicit `\n`:

```gality
[node]
旁白: 第一行。\n第二行。
```

**Note**: Currently, multi-line single nodes are **not supported**. Use
multiple nodes instead.

---

## 5. Choices

Present options to the player.

### Format

```gality
[choice_node]
? [option 1 text] -> target_node_1
? [option 2 text] -> target_node_2
? [option 3 text] -> target_node_3
```

### Rules

- Each choice is on its own line
- Starts with `?`
- Text is enclosed in `[...]`
- Target is prefixed with `->`
- **Minimum 1**, **maximum unlimited** (recommended: 2-5)

### Example

```gality
[choice_01]
? [包在我身上吧！] -> act_accept
? [架構好繁重啊...] -> act_hesitate

[act_accept]
學姐: 太好了！
-> next

[act_hesitate]
學姐: 你怎麼這麼沒信心？
-> next
```

### Choice with Condition

To conditionally show a choice, route through a condition node first:

```gality
[before_choice]
IF has_key >= 1 THEN choice_with_key ELSE choice_without_key

[choice_with_key]
? [打開秘密房間] -> secret_room
? [離開] -> leave

[choice_without_key]
? [離開] -> leave
```

### Choice Text Formatting

Choice text **cannot** include rich tags (currently). Plain text only.

**Special characters**: `[`, `]`, `->` are not allowed inside choice text.

---

## 6. Conditional Branching

Route the story based on variable values.

### Format

```gality
[condition_node]
IF <variable> <op> <value> THEN <then_node> ELSE <else_node>
```

### Operators

| Operator | Meaning |
|---|---|
| `>=` | Greater than or equal |
| `<=` | Less than or equal |
| `>` | Greater than |
| `<` | Less than |
| `==` | Equal to |

### Example

```gality
[route_check]
IF favorability >= 100 THEN good_route ELSE bad_route

[good_route]
旁白: 你走進了好結局。
-> the_end

[bad_route]
旁白: 事情變糟了。
-> the_end
```

### Chained Conditions

For multiple branches, chain condition nodes:

```gality
[route_selector]
IF san_loss >= 60 THEN abyss_route ELSE route_selector_2

[route_selector_2]
IF junior_favor >= 40 THEN junior_route ELSE route_selector_3

[route_selector_3]
IF favorability >= 100 THEN senior_route ELSE tech_route
```

### Supported Values

- **Integer**: `IF flag >= 1 THEN ...`
- **Boolean as int**: `IF has_key >= 1 THEN ...` (1 = true, 0 = false)
- **No strings**: Gality doesn't support string comparison

---

## 7. Variables (Blackboard)

Gality uses a **Blackboard** — a global key-value store for story state.

### Variable Types

- **Integer** (`int`) — numbers, counters, flags
- **Boolean** (`bool`) — true/false (stored as 0/1)

**No strings** — use integers for everything.

### Reading

Read values in conditions:

```gality
IF favorability >= 50 THEN friend_route ELSE stranger_route
```

### Writing

Modify values with `$`:

```gality
[action_node]
$ favorability += 10
-> next
```

### Operators

| Op | Meaning | Example |
|---|---|---|
| `+=` | Add | `$ favor += 10` |
| `-=` | Subtract | `$ san_loss -= 5` |
| `=`  | Assign | `$ has_key = 1` |

### Syntax

```gality
$ <variable_name> <op> <integer>
```

**Rules**:
- Variable names: `snake_case`, ASCII only
- Values: **integers only** (no floats, no strings)
- No spaces between `$` and variable name

### Example: Favorability System

```gality
[good_choice]
$ favorability += 10
-> next

[bad_choice]
$ favorability -= 5
-> next

[check_favor]
IF favorability >= 50 THEN close_ending ELSE distant_ending
```

### Multiple Mutations

Multiple `$` lines execute **in order**:

```gality
[action]
$ favorability += 10
$ san_loss += 5
$ has_met_senior = 1
-> next
```

### Default Values

Uninitialized variables default to **0**.

```gality
[check]
IF never_set >= 1 THEN a ELSE b
```

`never_set` is 0, so this goes to `b`.

### Variable Naming Convention

Gality's demo uses:

| Pattern | Example | Purpose |
|---|---|---|
| `favorability` | `favorability >= 100` | Romance progress |
| `san_loss` | `san_loss >= 60` | Sanity/horror progress |
| `junior_favor` | `junior_favor >= 40` | Secondary character |
| `has_<item>` | `has_key = 1` | Boolean flags |
| `<character>_<stat>` | `senior_affection` | Character-specific |

**You can use any naming scheme** — these are conventions, not rules.

---

## 8. Rich Text Tags

Rich tags let you style text without leaving the script.

### Syntax

```gality
<tag>text</tag>
<tag=value>text</tag>
```

### Supported Tags

#### `<color=#RRGGBB>`

Changes text color.

```gality
旁白: <color=#FF5555>紅色</color>文字。
旁白: <color=#55FF55>綠色</color>文字。
旁白: <color=#5555FF>藍色</color>文字。
```

**Nested colors**:

```gality
旁白: <color=#FF5555>紅<color=#FFD700>金</color>紅</color>
```

#### `<shake>`

Shakes the text.

```gality
旁白: 這是<shake>震動</shake>文字。
```

#### `<wave>`

Vertical sine wave animation.

```gality
旁白: 這是<wave>波浪</wave>文字。
```

#### `<rainbow>`

Cycles through HSV colors.

```gality
旁白: 這是<rainbow>彩虹</rainbow>文字。
```

#### `<glitch>`

RGB chromatic aberration + jitter.

```gality
旁白: 這是<glitch>故障</glitch>文字。
```

#### `<speed=N>`

Changes typewriter speed for the enclosed text.

```gality
旁白: <speed=0.02>快速文字</speed>
旁白: <speed=0.15>慢速文字</speed>
```

- `N` = seconds per character
- Default: `0.04`

#### `<w=N>`

Pauses the typewriter at this point.

```gality
旁白: 我...<w=1.0>其實...<w=0.5>一直...
```

- `N` = pause duration in seconds
- Unlike `<speed>`, `<w>` inserts a one-time pause

#### `<font=name>`

Switches font for the enclosed text.

```gality
旁白: <font=bold>粗體文字</font>
旁白: <font=italic>斜體文字</font>
```

Font must be registered in `ui_theme.json`.

### Combining Tags

Tags can be nested:

```gality
旁白: <wave><rainbow>波浪彩虹</rainbow></wave>
旁白: <glitch><speed=0.3>慢速故障</speed></glitch>
```

**Order matters for visuals**:
- `<wave>` changes position
- `<rainbow>` changes color
- `<glitch>` adds RGB splitting
- All three can combine

### Self-Closing Tags

Currently, **all tags are paired** (`<tag>...</tag>`). Self-closing tags
(`<tag/>`) are not supported.

### Unknown Tags

If the compiler encounters an unknown tag:

- **Compiles with warning**
- Tag is **rendered literally** (visible to player)

This helps catch typos.

---

## 9. Node Attributes

Attributes configure the node's presentation.

### Common Attributes

| Attribute | Type | Example |
|---|---|---|
| `bg` | string | `bg: assets/bg/classroom.jpg` |
| `bgm` | string | `bgm: assets/audio/bgm_daily.ogg` |
| `cv` | string | `cv: assets/audio/cv_01.ogg` |
| `char` | string | `char: assets/characters/senior_normal.png` |
| `char_left` | string | `char_left: assets/characters/senior.png` |
| `char_center` | string | `char_center: assets/characters/president.png` |
| `char_right` | string | `char_right: assets/characters/junior.png` |
| `active_char` | string | `active_char: left` |
| `weather` | string | `weather: rain` |
| `shake` | float | `shake: 0.5` |
| `trans` | string | `trans: diamond` |
| `duration` | float | `duration: 1.5` |
| `no_skip` | bool | `no_skip: true` |
| `wait` | float | `wait: 2.0` |
| `dialogueFont` | string | `dialogueFont: bold` |
| `nameFont` | string | `nameFont: italic` |

### `bg` — Background

```gality
[node]
bg: assets/bg/classroom.jpg
旁白: 這是一間教室。
-> next
```

**Relative path**: Paths are relative to `assets/`.

**When it changes**: The engine detects a new background and triggers a
transition (if `trans` is set).

**To remove background**:

```gality
[node]
bg: 
旁白: 沒有背景。
-> next
```

### `bgm` — Background Music

```gality
[node]
bgm: assets/audio/bgm_daily.ogg
旁白: 播放 BGM。
-> next
```

**Crossfade**: When BGM changes, the engine crossfades automatically (1.5s).

**To stop BGM**:

```gality
[node]
bgm: 
旁白: 停止 BGM。
-> next
```

### `cv` — Character Voice

```gality
[node]
cv: assets/audio/cv_01.ogg
學姐: 這句有配音。
-> next
```

**Playback**: Plays immediately when the line is shown.

**Spatial audio**: The voice is panned based on `active_char` (left/center/right).

### `char` / `char_left` / `char_center` / `char_right`

Show a character sprite.

```gality
[node]
char: assets/characters/senior_normal.png
學姐: 單角色，預設在中央。
-> next
```

**Multi-character**:

```gality
[node]
char_left: assets/characters/senior_normal.png
char_center: assets/characters/president_normal.png
char_right: assets/characters/junior_normal.png
active_char: center
社長: 三個人一起。
-> next
```

**Note**: `char:` is shorthand for `char_center:`.

**Persistence**: Characters persist until changed. Setting `char_left: ` with
empty value removes the left character.

### `active_char`

Controls who is "speaking":

```gality
active_char: left
```

**Values**: `left`, `center`, `right`

**Effect**:
- Active character is shown at full brightness
- Others are dimmed

### `weather`

Sets the weather effect.

| Value | Effect |
|---|---|
| `none` | No weather (default) |
| `rain` | Rain particles |
| `snow` | Snow particles |
| `sakura` | Cherry blossom petals |

```gality
[node]
weather: rain
旁白: 外面下著雨。
-> next
```

**Persistence**: Weather persists until changed.

### `shake`

Triggers screen shake.

```gality
[node]
shake: 0.5
旁白: 地震了！
-> next
```

**Value**: 0.0 (no shake) to 1.0 (strong shake)

**Duration**: 0.4 seconds

### `trans` / `duration` — Transition

Controls how the background changes.

```gality
[node]
bg: assets/bg/rooftop.jpg
trans: dissolve
duration: 1.5
旁白: 場景切換。
-> next
```

**Available transitions**:

| Name | Effect |
|---|---|
| `diamond` | Diamond mask reveal |
| `wipe_left` | Horizontal wipe from right |
| `dissolve` | Fade to black, then reveal |
| `clock` | Clock-style circular reveal |

**Duration**: Seconds (default: 1.0)

**Only triggers when `bg` changes**.

### `no_skip` — Forced Read

Prevents skipping during typewriter.

```gality
[node]
no_skip: true
幽靈: 你必須看完這句話。
-> next
```

**Effect**:
- `no_skip: true` — cannot skip during typewriter
- `no_skip: false` (default) — normal behavior

**Skip button**: Auto-stops when entering a `no_skip` node.

### `wait` — Mandatory Delay

Adds a delay after the typewriter completes.

```gality
[node]
no_skip: true
wait: 2.0
旁白: 打完字後還要等 2 秒。
-> next
```

**Effect**: Player cannot advance until `wait` seconds pass.

**Note**: `wait` works independently of `no_skip`.

### `dialogueFont` / `nameFont`

Override fonts for this node.

```gality
[node]
dialogueFont: bold
nameFont: italic
學姐: 這句用粗體，名字用斜體。
-> next
```

### Attribute Order

Attributes must come **before** the dialogue line:

```gality
// ✅ Correct
[node]
bg: assets/bg/classroom.jpg
char: assets/characters/senior.png
學姐: 你好。
-> next

// ❌ Wrong
[node]
學姐: 你好。
bg: assets/bg/classroom.jpg
-> next
```

---

## 10. Comments

Comments start with `//` or `#`.

```gality
// This is a comment
# This is also a comment

[node]
bg: assets/bg/classroom.jpg    // Inline comment
旁白: 你好。
-> next
```

**Rules**:
- `//` and `#` both work
- Comments extend to end of line
- Comment-only lines are ignored
- Inline comments are allowed

**Not supported**:
- Block comments (`/* ... */`)
- Nested comments

---

## 11. Grammar Summary

```
<file>          ::= <directive> <node>+

<directive>     ::= "@start" <node_id>

<node>          ::= "[" <node_id> "]" <attribute>* <content> "->" <node_id>

<attribute>     ::= <attr_name> ":" <value>

<content>       ::= <dialogue> | <choice> | <condition> | <action>

<dialogue>      ::= <speaker> ":" <text>

<choice>        ::= ("?" "[" <text> "]" "->" <node_id>)+

<condition>     ::= "IF" <var> <op> <int> "THEN" <node_id> "ELSE" <node_id>

<action>        ::= ("$" <var> <mut_op> <int>)+

<speaker>       ::= <any_text_without_colon>

<text>          ::= <any_text_with_optional_rich_tags>

<node_id>       ::= <ASCII_identifier>

<var>           ::= <ASCII_identifier>

<op>            ::= ">=" | "<=" | ">" | "<" | "=="

<mut_op>        ::= "+=" | "-=" | "="

<int>           ::= <integer>
```

---

## 12. Compiler Behavior

The `.gality` compiler (`gality_compiler.py`) reads your script and produces
a JSON AST.

### Compilation Steps

1. **Read file**: UTF-8 decoding
2. **Parse lines**: Identify directives, nodes, attributes
3. **Detect node types**: By content
4. **Validate references**: Check `->` targets exist
5. **Generate AST**: JSON output

### What the Compiler Checks

| Check | Result |
|---|---|
| Duplicate node IDs | ❌ **Error** |
| Dangling `->` targets | ❌ **Error** |
| Missing `@start` | ⚠️ Warning (uses `node_01`) |
| Unknown attributes | ⚠️ Warning |
| Unknown rich tags | ⚠️ Warning |
| Malformed `IF` syntax | ❌ **Error** |
| Malformed `$` syntax | ❌ **Error** |

### Error Messages

```
[GalityCompiler] Error: Duplicate node ID 'node_01' (line 15)
[GalityCompiler] Error: Node 'node_05' references non-existent target 'node_999'
[GalityCompiler] Warning: Unknown attribute 'bgmm' (line 20)
```

### Output

The AST is JSON with this structure:

```json
{
  "start": "node_01",
  "nodes": [
    {
      "id": "node_01",
      "type": "dialogue",
      "speaker": "學姐",
      "text": "你好。",
      "bg": "assets/bg/classroom.jpg",
      "next": "node_02"
    }
  ]
}
```

**You don't need to understand the AST** unless you're extending the compiler.

### Running the Compiler

```bash
python -m devtools.scripts.gality_compiler \
    assets/scripts/my_story.gality \
    assets/scripts/demo_long.json
```

### Compiler Options

Currently, the compiler accepts **positional arguments**:

```
gality_compiler.py <input.gality> <output.json>
```

**No flags or options** in v0.3.0.

---

## Appendix: Complete Example

A small but complete script demonstrating all features:

```gality
@start intro

// ============================================================================
// INTRODUCTION
// ============================================================================

[intro]
bg: assets/bg/classroom.jpg
trans: diamond
duration: 1.5
bgm: assets/audio/bgm_daily.ogg
weather: sakura
char_center: assets/characters/senior_normal.png
旁白: 春天的教室，櫻花紛飛。
-> first_line

[first_line]
char_center: assets/characters/senior_smile.png
cv: assets/audio/cv_01.ogg
學姐: 你終於來了！
-> question

// ============================================================================
// CHOICE
// ============================================================================

[question]
char_center: assets/characters/senior_normal.png
學姐: 準備好了嗎？
-> choice_01

[choice_01]
? [準備好了！] -> act_confident
? [讓我想想...] -> act_hesitate

[act_confident]
$ favorability += 10
char_center: assets/characters/senior_happy.png
學姐: <wave>太好了！</wave>
-> final_check

[act_hesitate]
$ favorability -= 5
char_center: assets/characters/senior_pout.png
學姐: 你怎麼這麼沒信心？
-> final_check

// ============================================================================
// CONDITIONAL BRANCHING
// ============================================================================

[final_check]
IF favorability >= 10 THEN good_end ELSE bad_end

[good_end]
bg: assets/bg/rooftop.jpg
trans: dissolve
duration: 1.5
weather: sakura
char: assets/characters/senior_happy.png
no_skip: true
wait: 1.5
旁白: <color=#FFB7C5>你們一起走向了春天。</color>
-> the_end

[bad_end]
bg: assets/bg/classroom.jpg
weather: rain
char: assets/characters/senior_fear.png
shake: 0.3
旁白: <glitch>她轉身離開了。</glitch>
-> the_end

// ============================================================================
// END
// ============================================================================

[the_end]
bgm: 
旁白: 故事結束。
-> the_end
```

---

## Getting Help

- **Compiler errors**: Check the terminal output
- **Syntax questions**: See [Getting Started](getting_started.md)
- **Runtime issues**: See [UI Guide](ui_guide.md)
- **Bugs**: [GitHub Issues](https://github.com/moli0516/Gality/issues)

---

**Last updated**: 2026-09-17
