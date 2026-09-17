
# Examples

**Ready-to-run examples for Gality Engine.**

Each example is a **complete `.gality` script** you can copy, compile, and
run. They progress from "Hello, world" to a full branching narrative.

---

## Table of Contents

1. [How to Run an Example](#1-how-to-run-an-example)
2. [Example 1: Hello, Gality](#2-example-1-hello-gality)
3. [Example 2: Choices and Branching](#3-example-2-choices-and-branching)
4. [Example 3: Characters and Expressions](#4-example-3-characters-and-expressions)
5. [Example 4: Multi-Slot Scene](#5-example-4-multi-slot-scene)
6. [Example 5: Text Effects Showcase](#6-example-5-text-effects-showcase)
7. [Example 6: Horror Atmosphere](#7-example-6-horror-atmosphere)
8. [Example 7: Romance Scene](#8-example-7-romance-scene)
9. [Example 8: Branching Story with Endings](#9-example-8-branching-story-with-endings)
10. [Full Demo](#10-full-demo)

---

## 1. How to Run an Example

### Step 1: Create the Script

Save the example as `assets/scripts/example.gality`.

### Step 2: Compile

```bash
python -m devtools.scripts.gality_compiler \
    assets/scripts/example.gality \
    assets/scripts/demo_long.json
```

### Step 3: Pack Assets

```bash
python -m devtools.scripts.gality_packer assets data.pak
```

### Step 4: Run

```bash
build_dev.bat
```

Or run the built executable:

```bash
dist-dev\Gality.exe
```

### Troubleshooting

- **"Failed to load script"** — Did you run the compiler?
- **"Missing asset"** — Did you pack assets?
- **Black screen** — Run from a terminal to see errors

See [FAQ § 2](faq.md#2-installation--setup) for more.

### Assets Used

These examples assume you have the standard demo assets:

| Asset | Path |
|---|---|
| Classroom background | `assets/bg/classroom.jpg` |
| Hallway background | `assets/bg/hallway.jpg` |
| Clubroom background | `assets/bg/clubroom.jpg` |
| Rooftop background | `assets/bg/rooftop.jpg` |
| Senior sprites | `assets/characters/senior_*.png` |
| Junior sprites | `assets/characters/junior_*.png` |
| President sprites | `assets/characters/president_*.png` |

If you don't have these, replace paths with assets you do have.

---

## 2. Example 1: Hello, Gality

**The smallest possible script.**

```gality
@start hello

[hello]
bg: assets/bg/classroom.jpg
旁白: 你好，Gality！
-> hello_2

[hello_2]
旁白: 這是一個最小的視覺小說。
-> the_end

[the_end]
旁白: 故事結束。
-> the_end
```

### What It Demonstrates

- `@start` directive
- Node declaration
- Dialogue with `旁白` (narration)
- `->` advance
- Self-looping end node

### What Happens

1. Classroom background appears
2. "你好，Gality！" types out
3. Player advances
4. "這是一個最小的視覺小說。" types out
5. Player advances
6. "故事結束。" types out
7. Game stays on the final node

### Variations

**Add music**:

```gality
[hello]
bg: assets/bg/classroom.jpg
bgm: assets/audio/bgm_daily.ogg
旁白: 音樂開始播放。
-> next
```

**Add weather**:

```gality
[hello]
bg: assets/bg/classroom.jpg
weather: sakura
旁白: 櫻花紛飛。
-> next
```

---

## 3. Example 2: Choices and Branching

**Introduce player agency.**

```gality
@start start

[start]
bg: assets/bg/classroom.jpg
旁白: 放學後，你獨自留在教室。
-> choice_01

[choice_01]
? [繼續看書] -> stay
? [收拾書包回家] -> leave

[stay]
bg: assets/bg/classroom.jpg
旁白: 你沉浸在書本的世界裡。
旁白: 時間悄悄流逝。
-> ending_stay

[leave]
bg: assets/bg/hallway.jpg
trans: wipe_left
duration: 1.0
旁白: 你走出教室，走進走廊。
旁白: 夕陽的餘暉灑在地板上。
-> ending_leave

[ending_stay]
bg: assets/bg/classroom.jpg
旁白: 不知不覺，天已經黑了。
-> the_end

[ending_leave]
bg: assets/bg/hallway.jpg
旁白: 你搭上了回家的電車。
-> the_end

[the_end]
旁白: 故事結束。
-> the_end
```

### What It Demonstrates

- Choice nodes (`?`)
- Branching paths
- Rejoining at a common end
- Transition between backgrounds (`trans: wipe_left`)

### Variations

**Add a third choice**:

```gality
[choice_01]
? [繼續看書] -> stay
? [收拾書包回家] -> leave
? [去屋頂看看] -> rooftop

[rooftop]
bg: assets/bg/rooftop.jpg
trans: dissolve
duration: 1.5
旁白: 你走上了屋頂。
旁白: 微風吹過你的臉頰。
-> the_end
```

**Add text effects**:

```gality
[stay]
旁白: 你<color=#FFB7C5>沉浸在</color>書本的世界裡。
旁白: 時間<shake>悄悄</shake>流逝。
-> ending_stay
```

---

## 4. Example 3: Characters and Expressions

**Show a character with multiple expressions.**

```gality
@start start

[start]
bg: assets/bg/classroom.jpg
char: assets/characters/senior_normal.png
學姐: 你終於來了！
-> reaction_1

[reaction_1]
char: assets/characters/senior_smile.png
學姐: 我等你好久了。
-> reaction_2

[reaction_2]
char: assets/characters/senior_happy.png
學姐: <wave>今天天氣真好！</wave>
-> reaction_3

[reaction_3]
char: assets/characters/senior_surprised.png
shake: 0.3
學姐: <shake>什麼！？</shake>你居然忘記帶書？
-> reaction_4

[reaction_4]
char: assets/characters/senior_pout.png
學姐: 真是的... 你這個人真是的。
-> reaction_5

[reaction_5]
char: assets/characters/senior_smile.png
學姐: 不過... 算了。我們一起看吧。
-> the_end

[the_end]
旁白: 故事結束。
-> the_end
```

### What It Demonstrates

- Single character staging (`char:`)
- Expression switching (5 expressions)
- Rich text tags (`<wave>`, `<shake>`)
- Screen shake (`shake: 0.3`)
- Character persistence

### Expression Chain Pattern

Notice how the character changes expression in each node:

```
normal → smile → happy → surprised → pout → smile
```

This is the **standard pattern** for emotional progression.

---

## 5. Example 4: Multi-Slot Scene

**Show multiple characters with `active_char`.**

```gality
@start start

[start]
bg: assets/bg/clubroom.jpg
trans: diamond
duration: 1.0
char_left: assets/characters/senior_normal.png
char_right: assets/characters/junior_normal.png
active_char: left
bgm: assets/audio/bgm_club.ogg
學姐: 羽月，你覺得這個方案如何？
-> reply_1

[reply_1]
char_left: assets/characters/senior_normal.png
char_right: assets/characters/junior_smug.png
active_char: right
學妹: 哼，勉強還可以。但這裡有個問題。
-> reply_2

[reply_2]
char_left: assets/characters/senior_surprised.png
char_right: assets/characters/junior_smug.png
active_char: left
學姐: <shake>什麼問題！？</shake>
-> reply_3

[reply_3]
char_left: assets/characters/senior_surprised.png
char_right: assets/characters/junior_normal.png
active_char: right
學妹: 記憶體快取策略。你確定 O(1) 查找真的成立嗎？
-> reply_4

[reply_4]
char_left: assets/characters/senior_happy.png
char_right: assets/characters/junior_normal.png
active_char: left
學姐: <wave>這點我早就考慮過了！</wave>
-> president_enters

[president_enters]
bg: assets/bg/clubroom.jpg
char_left: assets/characters/senior_happy.png
char_center: assets/characters/president_arms.png
char_right: assets/characters/junior_normal.png
active_char: center
社長: 我在門外都聽到了。繼續討論。
-> reply_5

[reply_5]
char_left: assets/characters/senior_happy.png
char_center: assets/characters/president_arms.png
char_right: assets/characters/junior_happy.png
active_char: right
學妹: 社長！你終於從機房出來了。
-> reply_6

[reply_6]
char_left: assets/characters/senior_normal.png
char_center: assets/characters/president_smile.png
char_right: assets/characters/junior_happy.png
active_char: center
社長: 嗯。今天討論得很熱烈，我很滿意。
-> the_end

[the_end]
旁白: 故事結束。
-> the_end
```

### What It Demonstrates

- Two-character staging (Left + Right)
- Three-character staging (Left + Center + Right)
- `active_char` switching
- Automatic dimming
- Expression changes on multiple characters
- Character persistence (senior stays visible when president enters)

### Dimming Visualization

```
active_char: left          active_char: right
┌──────────────┐           ┌──────────────┐
│              │           │              │
│  [FULL]      │  [DIM]    │  [DIM]       │  [FULL]
│  學姐        │  學妹     │  學姐        │  學妹
│              │           │              │
└──────────────┘           └──────────────┘
```

The `active_char` speaker is at full brightness. Others are dimmed.

---

## 6. Example 5: Text Effects Showcase

**Demonstrate all rich text tags.**

```gality
@start start

[start]
bg: assets/bg/classroom.jpg
旁白: === Text Effects Showcase ===
-> colors

[colors]
bg: assets/bg/classroom.jpg
旁白: <color=#FF5555>紅色</color> <color=#55FF55>綠色</color> <color=#5555FF>藍色</color> <color=#FFD700>金色</color> <color=#FFB7C5>粉紅</color>
-> nested_colors

[nested_colors]
bg: assets/bg/classroom.jpg
旁白: 巢狀顏色：<color=#FF5555>紅<color=#FFD700>金</color>紅</color>
-> shake

[shake]
bg: assets/bg/classroom.jpg
旁白: 這是<shake>震動</shake>文字，用於強調。
-> wave

[wave]
bg: assets/bg/classroom.jpg
旁白: 這是<wave>波浪</wave>文字，用於活潑場景。
-> rainbow

[rainbow]
bg: assets/bg/classroom.jpg
旁白: 這是<rainbow>彩虹</rainbow>文字，用於奇幻場景。
-> glitch

[glitch]
bg: assets/bg/classroom.jpg
旁白: 這是<glitch>故障</glitch>文字，用於恐怖場景。
-> speed_fast

[speed_fast]
bg: assets/bg/classroom.jpg
旁白: <speed=0.02>這是快速文字，用於日常對話。</speed>
-> speed_slow

[speed_slow]
bg: assets/bg/classroom.jpg
旁白: <speed=0.15>這是慢速文字，用於強調。</speed>
-> pause

[pause]
bg: assets/bg/classroom.jpg
旁白: 這是暫停測試...<w=1.0>一秒過去了...<w=0.5>半秒過去了。
-> combine_1

[combine_1]
bg: assets/bg/classroom.jpg
旁白: <wave><rainbow>波浪 + 彩虹</rainbow></wave>
-> combine_2

[combine_2]
bg: assets/bg/classroom.jpg
旁白: <glitch><speed=0.3>慢速 + 故障</speed></glitch>
-> the_end

[the_end]
旁白: === Showcase 結束 ===
-> the_end
```

### What It Demonstrates

- All color tags
- Nested colors
- `<shake>`, `<wave>`, `<rainbow>`, `<glitch>`
- `<speed=N>` for fast/slow text
- `<w=N>` for pauses
- Combined tags

### Effect Reference

| Tag | Visual Effect |
|---|---|
| `<color=#HEX>` | Text color |
| `<shake>` | Per-character random jitter |
| `<wave>` | Vertical sine wave |
| `<rainbow>` | HSV color cycling |
| `<glitch>` | RGB chromatic aberration |
| `<speed=N>` | Typewriter interval (seconds per char) |
| `<w=N>` | One-time pause (seconds) |

---

## 7. Example 6: Horror Atmosphere

**Create tension with combined effects.**

```gality
@start start

[start]
bg: assets/bg/hallway.jpg
trans: dissolve
duration: 1.5
bgm: assets/audio/bgm_horror.ogg
weather: none
shake: 0.2
旁白: 走廊上，燈光閃爍不定。
-> footsteps

[footsteps]
bg: assets/bg/hallway.jpg
shake: 0.3
旁白: <w=0.5>腳步聲<w=1.0>從遠處傳來...
-> appear

[appear]
bg: assets/bg/hallway.jpg
trans: diamond
duration: 0.5
char: assets/characters/senior_fear.png
shake: 0.5
學姐: <shake>誰... 誰在那裡？</shake>
-> ghost_1

[ghost_1]
bg: assets/bg/hallway.jpg
char: assets/characters/senior_corrupt.png
no_skip: true
wait: 1.0
幽靈: <glitch><speed=0.5>你... 終於... 打開了...</speed></glitch>
-> ghost_2

[ghost_2]
bg: assets/bg/hallway.jpg
char: assets/characters/senior_corrupt.png
shake: 0.8
no_skip: true
wait: 2.0
幽靈: <glitch><rainbow><shake>I'M STILL HERE.</shake></rainbow></glitch>
-> ghost_3

[ghost_3]
bg: assets/bg/hallway.jpg
char: assets/characters/senior_corrupt.png
no_skip: true
wait: 1.5
幽靈: <glitch><speed=0.3>我們... 都是... 同一個... 錯誤...</speed></glitch>
-> blackout

[blackout]
bg: assets/bg/hallway.jpg
trans: dissolve
duration: 0.5
shake: 1.0
旁白: <shake>一切歸於黑暗。</shake>
-> the_end

[the_end]
bg: assets/bg/classroom.jpg
trans: diamond
duration: 1.5
bgm: assets/audio/bgm_daily.ogg
旁白: === 場景結束 ===
-> the_end
```

### What It Demonstrates

- Horror atmosphere via combined effects
- `<glitch>` + `<speed=0.5>` + `no_skip` + `wait`
- Screen shake escalation
- Transition timing
- BGM switching

### The Horror Formula

```
Slow typewriter (<speed=0.5>)
    + RGB glitch (<glitch>)
    + Forced read (no_skip: true)
    + Mandatory pause (wait: N)
    + Screen shake (shake: 0.5-1.0)
    = Terror
```

**Why it works**:
- Player **can't skip** — they must watch
- Typewriter is **painfully slow**
- Every character **flickers**
- Screen **trembles**
- The pause **forces them to sit** with the fear

**Use sparingly** — one or two horror scenes per game.

---

## 8. Example 7: Romance Scene

**Slow, deliberate pacing for emotional moments.**

```gality
@start start

[start]
bg: assets/bg/rooftop.jpg
trans: dissolve
duration: 1.5
bgm: assets/audio/bgm_quiet.ogg
weather: sakura
char: assets/characters/senior_smile.png
旁白: 夕陽染紅了整片天空。
-> hesitation_1

[hesitation_1]
bg: assets/bg/rooftop.jpg
weather: sakura
char: assets/characters/senior_normal.png
學姐: 我...<w=1.0>其實...<w=0.8>一直...
-> hesitation_2

[hesitation_2]
bg: assets/bg/rooftop.jpg
weather: sakura
char: assets/characters/senior_surprised.png
學姐: <w=0.5>想告訴你一件事。
-> confession

[confession]
bg: assets/bg/rooftop.jpg
weather: sakura
char: assets/characters/senior_smile.png
no_skip: true
wait: 2.0
學姐: <speed=0.15>我喜歡你。</speed>
-> freeze

[freeze]
bg: assets/bg/rooftop.jpg
weather: sakura
char: assets/characters/senior_smile.png
no_skip: true
wait: 1.5
旁白: <color=#FFB7C5>時間彷彿靜止了。</color>
-> response

[response]
bg: assets/bg/rooftop.jpg
weather: sakura
char: assets/characters/senior_happy.png
旁白: 我不知道該說什麼。
旁白: 只是靜靜地看著她。
-> ending

[ending]
bg: assets/bg/rooftop.jpg
weather: sakura
char: assets/characters/senior_happy.png
no_skip: true
wait: 2.0
旁白: <rainbow>這一刻，永遠留在我的記憶裡。</rainbow>
-> the_end

[the_end]
bg: assets/bg/rooftop.jpg
weather: sakura
旁白: === 場景結束 ===
-> the_end
```

### What It Demonstrates

- Slow pacing via `<w=N>` pauses
- Slow typewriter (`<speed=0.15>`)
- Long pauses (`wait: 2.0`)
- Sakura weather for romantic atmosphere
- Subtle color (`<color=#FFB7C5>`)
- `no_skip` for dramatic effect

### The Romance Formula

```
Slow typewriter (<speed=0.15>)
    + Long pauses (<w=1.0>)
    + Mandatory freeze (no_skip + wait)
    + Weather (sakura)
    + Soft colors (<color=#FFB7C5>)
    = Emotion
```

**The contrast with horror**:
- Same tools (`<speed>`, `<w>`, `no_skip`, `wait`)
- Different pacing (romance is soft, horror is sharp)
- Different colors (pink vs red)
- Different weather (sakura vs none)

---

## 9. Example 8: Branching Story with Endings

**A complete branching narrative.**

```gality
@start intro

[intro]
bg: assets/bg/classroom.jpg
bgm: assets/audio/bgm_daily.ogg
weather: sakura
旁白: 春天的教室。
-> first_choice

[first_choice]
旁白: 你看到學姐站在窗邊。
-> choice_1

[choice_1]
? [走過去打招呼] -> approach
? [靜靜觀察] -> observe

// =============================================================================
// APPROACH BRANCH
// =============================================================================

[approach]
$ friendliness += 10
bg: assets/bg/classroom.jpg
char: assets/characters/senior_normal.png
學姐: 你來啦。
-> approach_2

[approach_2]
char: assets/characters/senior_smile.png
學姐: 今天天氣真好。
-> approach_3

[approach_3]
char: assets/characters/senior_normal.png
學姐: 我在想一件事...
-> sub_choice

[sub_choice]
? [什麼事？] -> ask
? [等一下，我先走] -> leave

[ask]
$ friendliness += 15
char: assets/characters/senior_surprised.png
學姐: 其實我...
-> ask_2

[ask_2]
char: assets/characters/senior_smile.png
no_skip: true
wait: 1.5
學姐: <speed=0.15>我喜歡你。</speed>
-> true_end

[leave]
$ friendliness -= 10
char: assets/characters/senior_pout.png
學姐: 這樣啊...
-> neutral_end

// =============================================================================
// OBSERVE BRANCH
// =============================================================================

[observe]
bg: assets/bg/classroom.jpg
旁白: 你靜靜地看著她。
旁白: 她的側臉在陽光下...
-> observe_2

[observe_2]
char: assets/characters/senior_surprised.png
學姐: 咦？你什麼時候來的？
-> observe_3

[observe_3]
char: assets/characters/senior_pout.png
學姐: <shake>不要偷看啦！</shake>
-> neutral_end

// =============================================================================
// ENDINGS
// =============================================================================

[true_end]
bg: assets/bg/rooftop.jpg
trans: dissolve
duration: 2.0
weather: sakura
bgm: assets/audio/bgm_quiet.ogg
char: assets/characters/senior_happy.png
no_skip: true
wait: 2.0
旁白: <rainbow>故事迎來了最美好的結局。</rainbow>
-> final

[neutral_end]
bg: assets/bg/hallway.jpg
trans: wipe_left
duration: 1.0
bgm: assets/audio/bgm_daily.ogg
旁白: 也許，還有別的機會。
-> final

[final]
bg: assets/bg/classroom.jpg
旁白: === 故事結束 ===
-> restart_choice

[restart_choice]
? [重新開始] -> intro
? [離開] -> quit

[quit]
旁白: 感謝遊玩。
-> quit
```

### What It Demonstrates

- Multiple branches
- Nested choices
- Variable mutations (`$ friendliness += 10`)
- Multiple endings
- Restart loop

### Branching Diagram

```
                    [intro]
                       │
                  [first_choice]
                  ┌────┴────┐
              approach    observe
                  │           │
              [approach]  [observe]
                  │           │
              [sub_choice]    │
              ┌───┴───┐       │
            ask     leave     │
             │        │       │
         true_end  neutral_end─┘
             │        │
             └───┬────┘
              [final]
                 │
          [restart_choice]
          ┌──────┴──────┐
       intro           quit
```

### Variables Tracking

The example uses `$ friendliness` to track player choices:

| Choice | Effect |
|---|---|
| Approach | `friendliness += 10` |
| Ask | `friendliness += 15` |
| Leave | `friendliness -= 10` |

**Total possible**:
- Best path: 25
- Worst path: 0
- Observe path: 0

**Note**: The example doesn't use `friendliness` for branching. To add
conditional endings:

```gality
[check_friendliness]
IF friendliness >= 20 THEN true_end ELSE neutral_end
```

---

## 10. Full Demo

The complete demo script is at:

```
assets/scripts/main_story_multi.gality
```

It's a **~3000 node visual novel** demonstrating:

- 4 major routes
- 10 distinct endings
- Multi-slot character staging
- Text effects in dramatic moments
- Horror sequences
- Romance sequences
- Branching narrative

### Running the Full Demo

```bash
# Compile
python -m devtools.scripts.gality_compiler \
    assets/scripts/main_story_multi.gality \
    assets/scripts/demo_long.json

# Pack
python -m devtools.scripts.gality_packer assets data.pak

# Run
build_dev.bat
```

### Text Effects Showcase

A separate demo focused on visual effects:

```
assets/scripts/test_fx.gality
```

Try it to see all rich text tags in action.

---

## Appendix: Common Patterns

### Pattern: Multi-Line Sequence

```gality
[node]
旁白: 第一句。
-> node_2

[node_2]
旁白: 第二句。
-> node_3

[node_3]
旁白: 第三句。
-> next
```

### Pattern: Quick Choice

```gality
[node]
旁白: 你應該怎麼做？
-> choice

[choice]
? [做] -> do
? [不做] -> dont

[do]
$ action = 1
-> next

[dont]
$ action = 0
-> next
```

### Pattern: Variable Check

```gality
[check]
IF action >= 1 THEN success ELSE failure

[success]
旁白: 你成功了。
-> next

[failure]
旁白: 你失敗了。
-> next
```

### Pattern: Expression Chain

```gality
[emotion_1]
char: assets/characters/senior_normal.png
旁白: 她思考著。
-> emotion_2

[emotion_2]
char: assets/characters/senior_smile.png
旁白: 她笑了。
-> emotion_3

[emotion_3]
char: assets/characters/senior_happy.png
旁白: 她笑得更燦爛了。
-> next
```

### Pattern: Timed Reveal

```gality
[reveal]
no_skip: true
wait: 2.0
旁白: <speed=0.3>這是一個重要的真相。</speed>
-> next
```

### Pattern: Scene Transition

```gality
[scene_change]
bg: assets/bg/rooftop.jpg
trans: dissolve
duration: 1.5
bgm: assets/audio/bgm_emotional.ogg
旁白: 場景切換到屋頂。
-> next
```

### Pattern: Character Entrance

```gality
[entrance]
trans: wipe_left
duration: 0.5
char: assets/characters/senior_normal.png
shake: 0.3
學姐: <shake>抱歉，我遲到了！</shake>
-> next
```

---

## Related Documentation

- **[Script Language](script_language.md)** — Full DSL reference
- **[Character Guide](character_guide.md)** — Character rendering
- **[UI Guide](ui_guide.md)** — UI customization
- **[Assets Guide](assets.md)** — Asset preparation
- **[Getting Started](getting_started.md)** — Installation

---

**Last updated**: 2026-09-17
