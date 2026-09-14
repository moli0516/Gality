

# Gality Engine 劇本與資產創作手冊 (v1.0)

## 1. 專案目錄結構

創作時只需將素材放入對應的資料夾：

* `assets/scripts/`：存放 `.gality` 劇本原始檔


* `assets/bg/`：背景圖檔 (`.jpg` / `.png`)


* `assets/characters/`：人物立繪 (`.png` 建議去背透明)


* `assets/audio/`：背景音樂 (`.ogg`) 與語音/音效 (`.ogg` / `.wav`)


* `assets/fonts/`：預設字型 `font.ttf`

* `assets/config/ui_theme.json`：UI 介面外觀配置檔



---

## 2. `.gality` 劇本語法指南

### 2.1 劇本開頭與節點定義

每個劇本檔必須以 `@start` 指定起始節點標籤。每個對話區塊使用 `[節點名稱]` 宣告：

```gality
@start node_01

[node_01]
bg: assets/bg/classroom.jpg
bgm: assets/audio/bgm_daily.ogg
weather: sakura
旁白: 放學後的陽光透過窗戶灑在課桌上。
-> node_02

```

---

### 2.2 對話與人物演出 (Dialogue)

* **對話格式**：`角色名稱: 對話內容`。若無角色名稱則為旁白。


* **立繪與語音**：使用 `char:` 指定人物立繪，`cv:` 指定語音檔。


* **富文字顏色標籤**：使用 `<color=#HEX>文字</color>` 進行動態劃重點加色。



```gality
[node_02]
bg: assets/bg/classroom.jpg
char: assets/characters/senior_normal.png
cv: assets/audio/cv_01.ogg
學姐: 你終於準備好了嗎？我們的<color=#FF5555>遊戲開發計劃</color>今天必須確定下來。
-> node_03

```

---

### 2.3 演出特效指令

* **天氣粒子 (`weather`)**：支援 `sakura`（櫻花）、`rain`（下雨）、`snow`（飄雪）、`none`（關閉）。


* **畫面震動 (`shake`)**：設定數值（如 `0.3` 或 `0.6`）觸發震動效果。



```gality
[node_weather_demo]
bg: assets/bg/sunset.jpg
weather: rain
shake: 0.5
旁白: 突然一陣狂風暴雨襲來！
-> node_next

```

---

### 2.4 選擇題與分支 (Choices)

使用 `? [選項文字] -> 目標節點` 建立玩家選擇分歧：

```gality
[choice_01]
? [包在我身上吧！今晚就能搞定！] -> act_favor_up
? [感覺好麻煩啊... 可以交給別人嗎？] -> act_favor_down

```

---

### 2.5 數值變更與條件判斷 (Flags & Conditions)

* **數值變更 (`$`)**：修改 `Blackboard` 全域旗標（例如好感度 `$ favorability += 10`）。


* **條件跳轉 (`IF ... THEN ... ELSE`)**：根據條件自動評估並跳轉結局。



```gality
[act_favor_up]
$ favorability += 10
-> node_04_a

[cond_check_ending]
IF favorability >= 10 THEN node_good_ending ELSE node_normal_ending

```

---

## 3. UI 樣式自訂 (`assets/config/ui_theme.json`)

無需重新編譯專案，修改 `ui_theme.json` 即可自訂對話框與選項視覺：

```json
{
  "dialogueBox": {
    "posX": 40.0,
    "posY": 480.0,
    "width": 1200.0,
    "height": 200.0,
    "bgColor": [0, 0, 0, 200],
    "nameTextColor": [255, 255, 0, 255],
    "dialogueFontSize": 24
  },
  "choiceUI": {
    "width": 800.0,
    "height": 55.0,
    "spacing": 20.0,
    "startY": 220.0,
    "normalBgColor": [30, 30, 50, 220],
    "hoverBgColor": [70, 70, 130, 240]
  }
}

```

---

## 4. 熱插拔除錯捷徑 (Debug Keys)

在遊戲執行時可使用以下快捷鍵：

* **`F1` 或 ``` (波浪鍵)**：開啟內建 Debugger Inspector，可動態調整變數、強制跳轉劇本節點。


* **`Tab` 或 `H**`：開啟 / 關閉歷史對話 Backlog 視窗。


* **`Mouse Wheel Up`**：向上滾動滾輪直接查看歷史歷史對話。


* **`S` 鍵 / `L` 鍵**：快速儲存 / 載入目前進度。



---

## 5. 一鍵編譯與打包發布

完成劇本編寫後，直接雙擊專案根目錄的 **`build_game.bat`** 檔。

系統將自動完成劇本轉譯、資產加密打包與 C++ 執行檔封裝，並於 `dist/` 資料夾中生成綠色免安裝發布包！