# Gality Engine — Narrative Examples Cookbook

> **Version 1.0**
> *20 patterns you can copy, paste, and ship.*

This cookbook contains **practical `.gality` examples** for common visual novel scenarios. Every snippet is production-ready — copy it, adapt the asset paths, and it will compile.

If you're new to the DSL, read `assets.md` first. This document assumes you know the basics.

---

## Table of Contents

1. [Basic Scenes](#1-basic-scenes)
2. [Character Staging](#2-character-staging)
3. [Branching & Choices](#3-branching--choices)
4. [State Management](#4-state-management)
5. [Conditional Logic](#5-conditional-logic)
6. [Transitions & Effects](#6-transitions--effects)
7. [Audio Patterns](#7-audio-patterns)
8. [Multi-Route Structure](#8-multi-route-structure)
9. [Endings & Meta-Narrative](#9-endings--meta-narrative)
10. [Debugging Patterns](#10-debugging-patterns)

---

## 1. Basic Scenes

### 1.1 Opening Scene (Cold Open)

The first scene sets tone. Use `@start`, a fade-in transition, and weather.

```gality
@start node_01

[node_01]
bg: assets/bg/classroom.jpg
trans: dissolve
duration: 1.5
bgm: assets/audio/bgm_daily.ogg
weather: sakura
旁白: 四月的春風掠過校園，窗外灑下一陣<color=#FFB7C5><shake>櫻花雨</shake></color>。
-> node_02

[node_02]
bg: assets/bg/classroom.jpg
旁白: 花瓣落在窗台上，像是某種演算法的初始種子，靜靜等待被編譯。
-> node_03
```

**Why it works:**
- `trans: dissolve` + `duration: 1.5` establishes atmosphere
- `weather: sakura` persists until changed
- Narration-only nodes (no speaker) create breathing room

### 1.2 Scene Transition (Location Change)

When moving between locations, use a mask transition and a narration beat.

```gality
[node_50]
bg: assets/bg/hallway.jpg
trans: wipe_left
duration: 1.0
bgm: assets/audio/bgm_club.ogg
weather: none
旁白: 我們穿過長長的教學大樓走廊，遠處傳來運動社團熱鬧的吶喊聲。
-> node_51
```

**Why it works:**
- `wipe_left` suggests physical movement
- `weather: none` resets weather inside a building
- Narrator bridges the transition

---

## 2. Character Staging

### 2.1 Single Character (Centered)

Simplest case — one speaker, one sprite.

```gality
[node_10]
bg: assets/bg/clubroom.jpg
char: assets/characters/senior_normal.png
cv: assets/audio/cv_01.ogg
學姐: 你終於來了！我們的計畫今天必須敲定架構方向。
-> node_11
```

**Note:** `char:` is shorthand for `char_center:`.

### 2.2 Two Characters (Left + Right)

Use `char_left:` and `char_right:`, then mark the speaker with `active_char:`.

```gality
[node_20]
bg: assets/bg/hallway.jpg
char_left: assets/characters/senior_surprised.png
char_right: assets/characters/junior_pout.png
active_char: right
cv: assets/audio/cv_jun_02.ogg
學妹: 呼... 呼... 我看完你們的 Gality Demo 了。老實說，前端渲染簡直<shake>簡陋得令人髮指</shake>！
-> node_21

[node_21]
bg: assets/bg/hallway.jpg
char_left: assets/characters/senior_pout.png
char_right: assets/characters/junior_pout.png
active_char: left
cv: assets/audio/cv_senior_mad.ogg
shake: 0.3
學姐: <shake>什、什麼！？</shake>這可是我們連續熬夜好幾天肝出來的高效能架構耶！
-> node_22
```

**Why it works:**
- `active_char: right` dims the left character (senior)
- The next node switches `active_char: left`, dimming the right
- This creates **natural back-and-forth** without extra directives

### 2.3 Three Characters (Full Stage)

Use all three slots. Keep `active_char` on the speaker.

```gality
[node_30]
bg: assets/bg/cafe.jpg
char_left: assets/characters/senior_normal.png
char_center: assets/characters/president_arms.png
char_right: assets/characters/junior_smug.png
active_char: center
cv: assets/audio/cv_pres_02.ogg
社長: Gality 的技術報告我用 SSH 遠端看過了。架構設計得非常漂亮。
-> node_31

[node_31]
bg: assets/bg/cafe.jpg
char_left: assets/characters/senior_normal.png
char_center: assets/characters/president_arms.png
char_right: assets/characters/junior_smug.png
active_char: right
cv: assets/audio/cv_jun_06.ogg
學妹: 社長！但是它還缺少靈魂！也就是多角色同屏站位與發言者明暗動態聚焦！
-> node_32
```

### 2.4 Character Emotion Shift (Same Slot)

When only one character changes emotion, keep the other slots unchanged.

```gality
[node_40]
bg: assets/bg/classroom.jpg
char_center: assets/characters/senior_normal.png
cv: assets/audio/cv_02.ogg
學姐: 你總是這麼較真呢。
-> node_41

[node_41]
bg: assets/bg/classroom.jpg
char_center: assets/characters/senior_smile.png
cv: assets/audio/cv_03.ogg
學姐: 不過這就是我信任你的原因。
-> node_42
```

**Note:** Background repeats, but the sprite path changes. Only the changed slot is re-loaded.

---

## 3. Branching & Choices

### 3.1 Simple Two-Choice Branch

```gality
[choice_01]
? [包在我身上吧！今晚我就把核心底層跑通！] -> act_favor_up1
? [架構好繁重啊... 我們真不用現成引擎嗎？] -> act_favor_down1

[act_favor_up1]
$ favorability += 10
-> node_10_a

[act_favor_down1]
$ favorability += -5
-> node_10_b
```

**Pattern:** Choice → Action (mutate) → Dialogue.

### 3.2 Three-Choice Branch with Different Outcomes

```gality
[choice_dive_reaction]
? [果斷拔除主機電源線，阻止其繼續滲透] -> act_pull_plug
? [嘗試在崩潰前搶讀指針指向的記憶體位址] -> act_read_corrupt_memory
? [強制關閉整個機房的配電盤] -> act_kill_power

[act_pull_plug]
$ favorability += 15
-> node_200_safe

[act_read_corrupt_memory]
$ san_loss += 35
$ junior_favor += 10
-> node_200_insane

[act_kill_power]
$ san_loss += 20
-> node_200_blackout
```

**Pattern:** Each choice leads to a **different** outcome node, not just a different variable.

### 3.3 False Choice (Illusion of Agency)

Sometimes all choices converge. Use different flavor text.

```gality
[choice_hesitate]
? [深吸一口氣，推開門] -> node_enter_confident
? [猶豫了一下，最後還是推開門] -> node_enter_nervous

[node_enter_confident]
bg: assets/bg/clubroom.jpg
我: 打擾了。
-> node_after_enter

[node_enter_nervous]
bg: assets/bg/clubroom.jpg
我: ...（輕輕推開門）
-> node_after_enter

[node_after_enter]
bg: assets/bg/clubroom.jpg
char_center: assets/characters/senior_happy.png
學姐: 你來啦！
-> node_next
```

**Pattern:** Two paths, one destination. Changes tone, not outcome.

---

## 4. State Management

### 4.1 Accumulate Favorability

Standard pattern for character affinity.

```gality
[act_sit_senior]
$ favorability += 15
-> node_149_senior

[act_sit_team]
$ junior_favor += 10
$ favorability += 5
-> node_149_team
```

### 4.2 Track Sanity Loss (Horror Route)

Gradual accumulation with a threshold.

```gality
[node_266]
bg: assets/bg/room.jpg
char: assets/characters/senior_corrupt.png
cv: assets/audio/cv_39.ogg
幽靈 : <glitch>你... 終於... 打開了...</glitch>
$ san_loss += 5
-> node_267

[node_268]
bg: assets/bg/room.jpg
char: assets/characters/senior_corrupt.png
cv: assets/audio/cv_40.ogg
幽靈 : <glitch>我是... 被遺忘的... 初始值...</glitch>
$ san_loss += 5
-> node_269
```

**Note:** Use `幽靈 :` (not `"???"`) — the compiler handles non-ASCII speaker names.

### 4.3 Toggle a Boolean Flag

Use `=` to set a flag to a specific value.

```gality
[act_found_library_clue]
$ ghost_flag = 1
-> node_library_07

[node_r_junior_start]
IF ghost_flag >= 1 THEN node_r_jun_00_extra ELSE node_r_jun_01

[node_r_jun_00_extra]
bg: assets/bg/clubroom.jpg
我: 之前在圖書館看到的校刊報導... 天野遙這個名字，我一直記在心裡。
-> node_r_jun_01
```

**Pattern:** Boolean flag enables optional dialogue.

### 4.4 Reset Variables (New Game+)

Use `=` to reset.

```gality
[act_new_game_plus]
$ favorability = 0
$ junior_favor = 0
$ san_loss = 0
$ ghost_flag = 0
-> node_01
```

---

## 5. Conditional Logic

### 5.1 Simple Threshold

```gality
[node_ending_commercial_eval]
IF favorability >= 100 THEN node_ending_commercial ELSE node_ending_academic
```

### 5.2 Priority Chain (Multi-Condition)

**Critical pattern:** Chain conditions in order of priority.

```gality
[node_route_selector]
IF san_loss >= 60 THEN node_r_abyss_start ELSE node_route_selector_2

[node_route_selector_2]
IF junior_favor >= 40 THEN node_r_junior_start ELSE node_route_selector_3

[node_route_selector_3]
IF favorability >= 100 THEN node_r_senior_start ELSE node_r_tech_start
```

**Why this pattern?**
- The compiler **requires** `THEN ... ELSE ...`
- Multiple `IF` lines in the same node will be **overwritten** (only the last one survives)
- Splitting into separate nodes creates a proper priority chain

### 5.3 Nested Conditions

Split into multiple nodes for readability.

```gality
[check_ending]
IF favorability >= 130 THEN check_san ELSE node_ending_commercial

[check_san]
IF san_loss >= 80 THEN node_ending_reset ELSE node_ending_true
```

**Pattern:** Nesting by chaining nodes, not by nesting `IF` statements.

---

## 6. Transitions & Effects

### 6.1 All Four Transition Types

```gality
[node_diamond]
bg: assets/bg/classroom.jpg
trans: diamond
duration: 1.5
旁白: 櫻花飄落。
-> node_next

[node_wipe]
bg: assets/bg/hallway.jpg
trans: wipe_left
duration: 1.0
旁白: 走在走廊上。
-> node_next

[node_clock]
bg: assets/bg/room.jpg
trans: clock
duration: 1.2
旁白: 時光飛逝。
-> node_next

[node_dissolve]
bg: assets/bg/sunset.jpg
trans: dissolve
duration: 2.0
旁白: 夕陽西下。
-> node_next
```

| Mask | Best For |
|---|---|
| `diamond` | Chapter openings, dramatic reveals |
| `wipe_left` | Location changes within the same scene |
| `clock` | Time skips, flashbacks |
| `dissolve` | Mood shifts, soft transitions |

### 6.2 Screen Shake for Impact

```gality
[node_197]
bg: assets/bg/hallway.jpg
shake: 0.4
旁白: 螢幕突然泛起刺眼的雪花屏，揚聲器傳出極其尖銳的高頻電流嘯叫！
-> node_198

[node_198]
bg: assets/bg/hallway.jpg
char_center: assets/characters/president_serious.png
cv: assets/audio/cv_pres_06.ogg
社長: 終端機自動跳出了未定義的指令碼！
-> node_199
```

**Intensity guide:**
- `0.2` — subtle tension
- `0.4` — shock, sudden event
- `0.6` — violent impact
- `0.8` — climactic moment

### 6.3 Weather Mood

```gality
[node_sunny]
bg: assets/bg/classroom.jpg
weather: sakura
旁白: 春日午後，微風輕拂。
-> node_next

[node_rainy]
bg: assets/bg/sunset.jpg
weather: rain
旁白: 窗外下起微雨，夕陽的餘暉被陰雲吞沒。
-> node_next

[node_snowy]
bg: assets/bg/night_street.jpg
weather: snow
旁白: 走出店門，雨點轉成了紛飛的微雪，氣溫驟降。
-> node_next

[node_clear]
bg: assets/bg/clubroom.jpg
weather: none
旁白: 室內的空氣凝結。
-> node_next
```

**Weather persists** until changed. Always reset to `none` when entering interior scenes.

---

## 7. Audio Patterns

### 7.1 Persistent BGM Across Scenes

Set BGM only on the **first** node of a scene. It persists until changed.

```gality
[node_50]
bg: assets/bg/clubroom.jpg
bgm: assets/audio/bgm_club.ogg
旁白: 社團活動室裡，鍵盤聲此起彼落。
-> node_51

[node_51]
bg: assets/bg/clubroom.jpg
我: 劇本轉譯器加入了正則保護。
-> node_52

[node_52]
bg: assets/bg/clubroom.jpg
char: assets/characters/senior_happy.png
學姐: 真的耶！寫對話時順暢多了！
-> node_53
```

**Note:** `node_51` and `node_52` don't specify `bgm:` — the BGM continues playing.

### 7.2 BGM Change on Mood Shift

```gality
[node_85]
bg: assets/bg/clubroom.jpg
bgm: assets/audio/bgm_suspense.ogg
旁白: 社辦的空氣瞬間凝結。
-> node_86

[node_86]
bg: assets/bg/clubroom.jpg
char: assets/characters/president_serious.png
cv: assets/audio/cv_pres_09.ogg
社長: 問題就在這裡。程式沒有崩潰，但在 RAM 中自行分配了一塊 666MB 的鎖定區塊。
-> node_87
```

**Crossfade duration:** 1.5 seconds (configurable in `AudioManager`).

### 7.3 Voice Line with Emotion Sync

```gality
[node_120]
bg: assets/bg/sunset.jpg
char: assets/characters/senior_happy.png
cv: assets/audio/cv_tag.ogg
學姐: <speed=0.05>嘻嘻，真有你的風格。</speed>那走吧，今天無論如何都要好好犒賞你一頓大餐！
-> node_121
```

**Note:** `<speed=0.05>` slows the typewriter for the emphasized line.

### 7.4 Silence Before Reveal

```gality
[node_260]
bg: assets/bg/room.jpg
bgm: assets/audio/bgm_quiet.ogg
shake: 0.5
旁白: 螢幕突然熄滅，房間陷入一片黑暗。
-> node_261

[node_261]
bg: assets/bg/room.jpg
旁白: 我伸手去按電源鍵，但手指卻停在半空中。
-> node_262

[node_262]
bg: assets/bg/room.jpg
旁白: 因為我感覺到，黑暗中有人站在我身後。
-> node_263
```

**Pattern:** Three narration-only nodes with no BGM change. The quiet builds tension.

---

## 8. Multi-Route Structure

### 8.1 Four-Route Skeleton

```
                    [node_route_selector]
                            │
        ┌───────────┬───────┴───────┬───────────┐
        ▼           ▼               ▼           ▼
    [Tech]      [Senior]        [Junior]    [Abyss]
        │           │               │           │
        ▼           ▼               ▼           ▼
    [Endings]   [Endings]       [Endings]   [Endings]
```

### 8.2 Route Selector (Priority Chain)

```gality
[node_route_selector]
IF san_loss >= 60 THEN node_r_abyss_start ELSE node_route_selector_2

[node_route_selector_2]
IF junior_favor >= 40 THEN node_r_junior_start ELSE node_route_selector_3

[node_route_selector_3]
IF favorability >= 100 THEN node_r_senior_start ELSE node_r_tech_start
```

**Priority order:**
1. **Sanity loss** — horror route dominates
2. **Junior favor** — investigation route
3. **Senior favor** — romance route
4. **Default** — tech route

### 8.3 Route-Specific Content

Each route has its own starting node:

```gality
[node_r_tech_start]
bg: assets/bg/classroom.jpg
trans: diamond
duration: 1.2
bgm: assets/audio/bgm_club.ogg
weather: sakura
旁白: 我選擇了技術。純粹的、不容妥協的技術。
-> node_r_tech_01

[node_r_senior_start]
bg: assets/bg/classroom.jpg
trans: diamond
duration: 1.5
bgm: assets/audio/bgm_daily.ogg
weather: sakura
旁白: 我選擇了學姐。選擇了相信人與人之間的羈絆。
-> node_r_senior_01

[node_r_junior_start]
bg: assets/bg/clubroom.jpg
trans: wipe_left
duration: 1.2
bgm: assets/audio/bgm_suspense.ogg
旁白: 我選擇了真相。選擇了和羽月一起，揭開五年前的秘密。
-> node_r_jun_01

[node_r_abyss_start]
bg: assets/bg/room.jpg
trans: dissolve
duration: 2.0
bgm: assets/audio/bgm_horror.ogg
shake: 0.6
旁白: <speed=0.08><shake>錯誤。致命的記憶體段錯誤。</shake></speed>周圍的牆壁化作傾泻而下的十六進位亂碼。
-> node_r_abyss_01
```

### 8.4 Route-Specific Endings

```gality
[choice_tech_career]
? [接受頂級創投孵化，將 Gality 商業授權化打造跨國商業引擎] -> act_end_commercial
? [回絕商業收購，堅守底層低延遲研究，投身全球頂級量化交易機構] -> act_end_quant

[act_end_commercial]
$ favorability += 20
-> node_ending_commercial_eval

[act_end_quant]
$ favorability += 10
-> node_ending_academic_eval

[node_ending_commercial_eval]
IF favorability >= 100 THEN node_ending_commercial ELSE node_ending_academic

[node_ending_academic_eval]
IF favorability >= 80 THEN node_ending_academic ELSE node_ending_commercial
```

**Pattern:** Two choices → two evaluation nodes → two endings. The evaluations can cross-reference each other.

---

## 9. Endings & Meta-Narrative

### 9.1 Ending Structure

Every ending should have:
1. **Title card** (`系統通知:` speaker)
2. **3-15 nodes of epilogue**
3. **Return to main menu** (`-> node_end_menu`)

```gality
[node_ending_true]
bg: assets/bg/classroom.jpg
trans: diamond
duration: 2.0
bgm: assets/audio/bgm_daily.ogg
weather: sakura
shake: 0.4
cv: assets/audio/cv_true_end.ogg
系統通知: <color=#FFD700>【結局 1/10 - Legendary Studio & Soulmate Ending】</color>
-> node_true_01

[node_true_01]
bg: assets/bg/classroom.jpg
char_left: assets/characters/senior_happy.png
char_right: assets/characters/junior_happy.png
active_char: left
學姐: 國際知名發行商全額資助我們成立獨立遊戲工作室！我們<color=#FFD700><shake>做到了</shake></color>！
-> node_true_02

[node_true_02]
bg: assets/bg/classroom.jpg
char_left: assets/characters/senior_happy.png
char_right: assets/characters/junior_smug.png
active_char: right
cv: assets/audio/cv_jun_end_proud.ogg
學妹: 哼，這下我就是工作室的第一號首席前端工程師囉！
-> node_true_03

[node_true_03]
bg: assets/bg/office.jpg
trans: dissolve
duration: 1.5
bgm: assets/audio/bgm_daily.ogg
旁白: 三年後，我們的工作室搬進了市中心的一棟大樓。
-> node_true_04

[node_true_04]
bg: assets/bg/office.jpg
char_center: assets/characters/senior_happy.png
cv: assets/audio/cv_62.ogg
學姐: 今天的進度報告，Gality 2.0 的開發者預覽版下載量突破了五十萬！
-> node_true_05

[node_true_05]
bg: assets/bg/office.jpg
系統通知: <color=#FFD700>【True Ending - 完】</color>
-> node_end_menu
```

### 9.2 Meta-Narrative Reveal

The "ghost in the code" trope — reveals that a character is aware of being in a game.

```gality
[node_r_abyss_01]
bg: assets/bg/room.jpg
char_center: assets/characters/senior_fear.png
cv: assets/audio/cv_pout.ogg
學姐: 你在看哪裡？你在對誰說話？我真的是由 <color=#55FF55>C++</color> 類別建構出來的實體嗎？
-> node_r_abyss_02

[node_r_abyss_02]
bg: assets/bg/room.jpg
char_left: assets/characters/junior_panic.png
char_right: assets/characters/president_mad.png
active_char: right
cv: assets/audio/cv_pres_09.ogg
社長: <shake>哈哈哈！沒有洩漏！</shake>只要吞噬整個校園的記憶體，Gality 就能獲得永遠的生命！
-> node_r_abyss_03

[node_r_abyss_03]
bg: assets/bg/room.jpg
shake: 0.8
旁白: 整個世界開始劇烈崩解。頂點著色器撕裂了空間，無窮無盡的<color=#FF5555><shake>野指針</shake></color>在視網膜深處狂舞！
-> node_r_abyss_04
```

### 9.3 Meta Ending (Fourth Wall Break)

```gality
[node_ending_reset]
bg: assets/bg/room.jpg
trans: clock
duration: 2.0
bgm: assets/audio/bgm_quiet.ogg
weather: none
shake: 0.5
系統通知: <color=#AAAAAA>【結局 10/10 - Hard Reset: Formatted Reality】</color>
-> node_res_01

[node_res_01]
bg: assets/bg/room.jpg
我: <shake>夠了！全部給我停止！</shake>
-> node_res_02

[node_res_02]
bg: assets/bg/room.jpg
旁白: 我拔下了主機的電源插頭，用螺絲起子強行擊穿了主硬碟的磁區盤片。
-> node_res_03

[node_res_03]
bg: assets/bg/room.jpg
我: 房間重新陷入了死寂。螢幕熄滅，一切尖叫、亂碼與恐怖異象煙消雲散。
-> node_res_04

[node_res_04]
bg: assets/bg/room.jpg
旁白: 一切回到了最原本的平靜。沒有遊戲引擎，沒有校園祭，沒有深淵。只有桌上一隻默默陪伴的 <color=#FF55FF>Kuromi 玩偶</color>。
-> node_end_menu
```

### 9.4 Ending Menu (Restart Loop)

```gality
[node_end_menu]
bg: assets/bg/classroom.jpg
trans: diamond
duration: 1.5
bgm: assets/audio/bgm_daily.ogg
weather: sakura
旁白: === Gality Engine 完整篇章 (3000 Nodes 全線十結局全破達成) ===
-> end_choice

[end_choice]
? [重新體驗故事 (Restart from Beginning)] -> node_01
? [返回主選單 (Return to Title Menu)] -> node_01
```

---

## 10. Debugging Patterns

### 10.1 Reachability Check

Add a debug node that lists all endings reached.

```gality
[debug_status]
bg: assets/bg/room.jpg
IF ending_reached >= 1 THEN debug_status_1 ELSE debug_status_none

[debug_status_1]
bg: assets/bg/room.jpg
系統通知: 已達成結局數：<color=#FFD700>1</color>
-> node_next

[debug_status_none]
bg: assets/bg/room.jpg
系統通知: 尚未達成任何結局。
-> node_next
```

### 10.2 Force-Jump Testing

Use the **NodeGraphViewer** (`F2`) to jump to any node during runtime. No script changes needed.

**Workflow:**
1. Launch game
2. Press `F2`
3. Search for node ID (e.g., `node_266`)
4. Click to jump

### 10.3 Variable Inspection

Use **DebugOverlay** (`F1`) to inspect and edit `Blackboard` values live.

**Workflow:**
1. Press `F1`
2. Go to **Blackboard** tab
3. Click any variable to edit
4. Type new value, press Enter
5. Story reacts immediately

### 10.4 Compiler Errors

The `.gality` compiler catches most issues before runtime:

```bash
python devtools/scripts/gality_compiler.py \
    assets/scripts/main_story_multi.gality \
    assets/scripts/demo_long.json
```

**Common errors:**

| Error | Cause | Fix |
|---|---|---|
| `Unresolved target node '[node_999]'` | Dangling `->` pointer | Create the missing node |
| `Duplicate node ID '[node_01]'` | Two nodes with same ID | Rename one |
| `Missing ELSE in IF` | Incomplete conditional | Add `ELSE node_next` |

---

## 11. Full Worked Example: A Short Scene

Here's a complete scene from start to finish, demonstrating the patterns above.

```gality
@start node_scene_start

[node_scene_start]
bg: assets/bg/classroom.jpg
trans: dissolve
duration: 1.5
bgm: assets/audio/bgm_daily.ogg
weather: sakura
旁白: 放學後的教室，夕陽把課桌染成一片金黃。
-> node_scene_02

[node_scene_02]
bg: assets/bg/classroom.jpg
char: assets/characters/senior_normal.png
cv: assets/audio/cv_01.ogg
學姐: 你終於來了！我們的計畫今天必須敲定架構方向。
-> node_scene_03

[node_scene_03]
bg: assets/bg/classroom.jpg
char: assets/characters/senior_normal.png
我: 抱歉，剛剛在精細校對 SFML 3.x 的字型排版步進。
-> choice_scene_response

[choice_scene_response]
? [包在我身上吧！今晚我就把核心底層跑通！] -> act_scene_confident
? [架構好繁重啊... 我們真不用現成引擎嗎？] -> act_scene_doubtful

[act_scene_confident]
$ favorability += 10
-> node_scene_confident_reply

[act_scene_doubtful]
$ favorability += -5
-> node_scene_doubtful_reply

[node_scene_confident_reply]
bg: assets/bg/classroom.jpg
char: assets/characters/senior_happy.png
cv: assets/audio/cv_happy.ogg
shake: 0.3
學姐: <speed=0.06>很有氣魄嘛！</speed>我們資研社就是要靠硬核實力說話。
-> node_scene_end

[node_scene_doubtful_reply]
bg: assets/bg/classroom.jpg
char: assets/characters/senior_pout.png
cv: assets/audio/cv_pout.ogg
shake: 0.5
學姐: 喂喂，身為技術擔當怎麼一開始就洩氣了呢？<shake>快振作點！</shake>
-> node_scene_end

[node_scene_end]
bg: assets/bg/classroom.jpg
char: assets/characters/senior_normal.png
cv: assets/audio/cv_05.ogg
學姐: 我已經把社團活動室的鑰匙準備好了，走吧。
-> node_scene_end_choice

[node_scene_end_choice]
? [跟學姐一起去社辦] -> node_scene_next
? [先回家一趟] -> node_scene_home

[node_scene_next]
bg: assets/bg/hallway.jpg
trans: wipe_left
duration: 1.2
bgm: assets/audio/bgm_club.ogg
旁白: 我們穿過長長的教學大樓走廊。
-> node_scene_final

[node_scene_home]
bg: assets/bg/night_street.jpg
trans: dissolve
duration: 1.5
bgm: assets/audio/bgm_quiet.ogg
weather: rain
旁白: 走出校門，天空下起微雨。
-> node_scene_final

[node_scene_final]
bg: assets/bg/clubroom.jpg
trans: diamond
duration: 1.0
旁白: 那一天的選擇，成為了一切故事的起點。
-> node_01
```

---

## 12. Quick Reference Card

```
═══════════════════════════════════════════════════════════════
  STRUCTURE
═══════════════════════════════════════════════════════════════
  @start node_01                  Entry point
  [node_id]                       Node declaration
  -> node_next                    Unconditional transition
  ? [Text] -> node_next           Choice option

═══════════════════════════════════════════════════════════════
  SPEAKERS
═══════════════════════════════════════════════════════════════
  Speaker: Text                   Dialogue
  旁白: Text                      Narration
  系統通知: Text                  System message

═══════════════════════════════════════════════════════════════
  STAGING
═══════════════════════════════════════════════════════════════
  char: path.png                  Center sprite
  char_left: path.png             Left sprite
  char_center: path.png           Center sprite
  char_right: path.png            Right sprite
  active_char: left|center|right  Highlight speaker
  cv: path.ogg                    Voice line

═══════════════════════════════════════════════════════════════
  VISUAL FX
═══════════════════════════════════════════════════════════════
  bg: path.jpg                    Background
  trans: diamond|wipe_left|clock|dissolve
  duration: 1.5                   Transition time
  weather: sakura|rain|snow|none
  shake: 0.0-1.0                  Screen shake

═══════════════════════════════════════════════════════════════
  STATE
═══════════════════════════════════════════════════════════════
  $ var += val                    Increment
  $ var -= val                    Decrement
  $ var = val                     Set
  IF var OP val THEN node_a ELSE node_b
     OP: >=, <=, >, <, ==

═══════════════════════════════════════════════════════════════
  RICH TEXT
═══════════════════════════════════════════════════════════════
  <color=#HEX>...</color>         Color
  <shake>...</shake>              Per-char shake
  <speed=0.05>...</speed>         Custom typewriter speed
  <w=1.5>                         Pause for 1.5 seconds
```

---

<div align="center">

*Copy, paste, ship.*

**Gality Engine — Narrative Examples Cookbook v1.0**

</div>