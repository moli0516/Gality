# Project Codebase: .

## Directory Structure

```
./
├── assets
│   ├── audio
│   │   ├── bgm_club.ogg
│   │   ├── bgm_daily.ogg
│   │   ├── bgm_quiet.ogg
│   │   ├── cv_01.ogg
│   │   ├── cv_05.ogg
│   │   ├── cv_08.ogg
│   │   ├── cv_10.ogg
│   │   ├── cv_12.ogg
│   │   ├── cv_14.ogg
│   │   ├── cv_16_a.ogg
│   │   ├── cv_16_b.ogg
│   │   ├── cv_19.ogg
│   │   ├── cv_21.ogg
│   │   ├── cv_23.ogg
│   │   ├── cv_25.ogg
│   │   ├── cv_27.ogg
│   │   ├── cv_29.ogg
│   │   ├── cv_32.ogg
│   │   ├── cv_34.ogg
│   │   ├── cv_good_end.ogg
│   │   ├── cv_happy.ogg
│   │   ├── cv_norm_end.ogg
│   │   ├── cv_pout.ogg
│   │   ├── cv_senior_05.ogg
│   │   └── cv_senior_06.ogg
│   ├── bg
│   ├── characters
│   ├── fonts
│   │   └── font.ttf
│   └── scripts
│       └── demo_long.json
├── docs
├── saves
│   └── save1.json
├── src
│   ├── core
│   │   ├── Audiomanager.hpp
│   │   ├── Blackboard.hpp
│   │   ├── LRUCache.hpp
│   │   └── SaveManager.hpp
│   ├── render
│   │   ├── BacklogUI.hpp
│   │   ├── ChoiceUI.hpp
│   │   ├── DialogueBox.hpp
│   │   └── LayerRenderer.hpp
│   ├── story
│   │   ├── ScriptLoader.hpp
│   │   ├── StoryExecutor.hpp
│   │   └── StoryNode.hpp
│   └── main.cpp
├── .gitignore
└── vspkg.json
```

---

## Source Code

### File: `.gitignore`

```text
build/
vcpkg/
code_packer.py
.packerignore
*.png
*.jpg
*.ttf
*.ogg
devtools/
```

---

### File: `vspkg.json`

```json
{
  "$schema": "https://raw.githubusercontent.com/microsoft/vcpkg-tool/main/docs/vcpkg-configuration.schema.json",
  "name": "Gality",
  "version-string": "0.1.0",
  "dependencies": [
    "sfml",
    "nlohmann-json",
    "imgui",
    "imgui-sfml"
  ]
}
```

---

### File: `assets\scripts\demo_long.json`

```json
﻿{
  "startNodeId": "node_01",
  "nodes": [
    {
      "id": "node_01",
      "type": "Dialogue",
      "speaker": "旁白",
      "text": "放學後的陽光透過窗戶灑在課桌上，教室裡只剩下我和學姐兩個人。",
      "bgImagePath": "assets/bg/classroom.jpg",
      "characterSpritePath": "",
      "bgmPath": "assets/audio/bgm_daily.ogg",
      "voicePath": "",
      "next": "node_02"
    },
    {
      "id": "node_02",
      "type": "Dialogue",
      "speaker": "學姐",
      "text": "你終於準備好了嗎？我們等的遊戲引擎開發計劃，今天必須確定下來。",
      "bgImagePath": "assets/bg/classroom.jpg",
      "characterSpritePath": "assets/characters/senior_normal.png",
      "bgmPath": "",
      "voicePath": "assets/audio/cv_01.ogg",
      "next": "node_03"
    },
    {
      "id": "node_03",
      "type": "Dialogue",
      "speaker": "我",
      "text": "抱歉學姐，剛剛在確認 C++17 的 SFML 3.x 資產快取邏輯，耽誤了一點時間。",
      "bgImagePath": "assets/bg/classroom.jpg",
      "characterSpritePath": "assets/characters/senior_normal.png",
      "bgmPath": "",
      "voicePath": "",
      "next": "choice_01"
    },
    {
      "id": "choice_01",
      "type": "Choice",
      "speaker": "",
      "text": "",
      "choices": [
        { "text": "包在我身上吧！今晚就能搞定！", "next": "act_favor_up" },
        { "text": "感覺好麻煩啊... 可以交給別人嗎？", "next": "act_favor_down" }
      ]
    },
    {
      "id": "act_favor_up",
      "type": "Action",
      "actionType": "addInt",
      "key": "favorability",
      "value": 10,
      "next": "node_04_a"
    },
    {
      "id": "act_favor_down",
      "type": "Action",
      "actionType": "addInt",
      "key": "favorability",
      "value": -5,
      "next": "node_04_b"
    },
    {
      "id": "node_04_a",
      "type": "Dialogue",
      "speaker": "學姐",
      "text": "真可靠呢！我就知道沒有看錯你。",
      "bgImagePath": "assets/bg/classroom.jpg",
      "characterSpritePath": "assets/characters/senior_happy.png",
      "bgmPath": "",
      "voicePath": "assets/audio/cv_happy.ogg",
      "next": "node_05"
    },
    {
      "id": "node_04_b",
      "type": "Dialogue",
      "speaker": "學姐",
      "text": "真是的，又在說這種懶散的話了...",
      "bgImagePath": "assets/bg/classroom.jpg",
      "characterSpritePath": "assets/characters/senior_pout.png",
      "bgmPath": "",
      "voicePath": "",
      "next": "node_05"
    },
    {
      "id": "node_05",
      "type": "Dialogue",
      "speaker": "學姐",
      "text": "那麼，我們去社團教室討論具體的系統架構吧。",
      "bgImagePath": "assets/bg/hallway.jpg",
      "characterSpritePath": "assets/characters/senior_normal.png",
      "bgmPath": "assets/audio/bgm_club.ogg",
      "voicePath": "",
      "next": "node_06"
    },
    {
      "id": "node_06",
      "type": "Dialogue",
      "speaker": "旁白",
      "text": "我們沿著走廊走向資研社的社團教室。",
      "bgImagePath": "assets/bg/hallway.jpg",
      "characterSpritePath": "",
      "bgmPath": "",
      "voicePath": "",
      "next": "node_07"
    },
    {
      "id": "node_07",
      "type": "Dialogue",
      "speaker": "我",
      "text": "對了學姐，關於存讀檔與 Backlog 歷史回溯，我已經寫好框架了。",
      "bgImagePath": "assets/bg/clubroom.jpg",
      "characterSpritePath": "",
      "bgmPath": "",
      "voicePath": "",
      "next": "node_08"
    },
    {
      "id": "node_08",
      "type": "Dialogue",
      "speaker": "學姐",
      "text": "真的嗎？那向上滾動滾輪就能看到剛才的對話歷史囉？",
      "bgImagePath": "assets/bg/clubroom.jpg",
      "characterSpritePath": "assets/characters/senior_surprised.png",
      "bgmPath": "",
      "voicePath": "",
      "next": "node_09"
    },
    {
      "id": "node_09",
      "type": "Dialogue",
      "speaker": "我",
      "text": "沒錯，而且使用了 Command 模式與狀態堆疊，隨時支援 Rollback。",
      "bgImagePath": "assets/bg/clubroom.jpg",
      "characterSpritePath": "assets/characters/senior_smile.png",
      "bgmPath": "",
      "voicePath": "",
      "next": "node_10"
    },
    {
      "id": "node_10",
      "type": "Dialogue",
      "speaker": "學姐",
      "text": "聽起來好厲害！那記憶體管理部分呢？",
      "bgImagePath": "assets/bg/clubroom.jpg",
      "characterSpritePath": "assets/characters/senior_normal.png",
      "bgmPath": "",
      "voicePath": "",
      "next": "node_11"
    },
    {
      "id": "node_11",
      "type": "Dialogue",
      "speaker": "我",
      "text": "我實作了 LRU (Least Recently Used) Cache 快取，自動釋放久未使用的紋理。",
      "bgImagePath": "assets/bg/clubroom.jpg",
      "characterSpritePath": "assets/characters/senior_normal.png",
      "bgmPath": "",
      "voicePath": "",
      "next": "node_12"
    },
    {
      "id": "node_12",
      "type": "Dialogue",
      "speaker": "學姐",
      "text": "這下即使圖片再多，RAM 也不會爆掉了呢！讚喔！",
      "bgImagePath": "assets/bg/clubroom.jpg",
      "characterSpritePath": "assets/characters/senior_happy.png",
      "bgmPath": "",
      "voicePath": "",
      "next": "node_13"
    },
    {
      "id": "node_13",
      "type": "Dialogue",
      "speaker": "旁白",
      "text": "不知不覺中，天色漸漸暗了下來。",
      "bgImagePath": "assets/bg/sunset.jpg",
      "characterSpritePath": "",
      "bgmPath": "assets/audio/bgm_quiet.ogg",
      "voicePath": "",
      "next": "node_14"
    },
    {
      "id": "node_14",
      "type": "Dialogue",
      "speaker": "學姐",
      "text": "天黑了呢... 今天的討論就先到這裡吧？",
      "bgImagePath": "assets/bg/sunset.jpg",
      "characterSpritePath": "assets/characters/senior_normal.png",
      "bgmPath": "",
      "voicePath": "",
      "next": "node_15"
    },
    {
      "id": "node_15",
      "type": "Dialogue",
      "speaker": "我",
      "text": "好的學姐，要一起走去車站嗎？",
      "bgImagePath": "assets/bg/sunset.jpg",
      "characterSpritePath": "assets/characters/senior_normal.png",
      "bgmPath": "",
      "voicePath": "",
      "next": "choice_02"
    },
    {
      "id": "choice_02",
      "type": "Choice",
      "speaker": "",
      "text": "",
      "choices": [
        { "text": "邀請學姐一起去 Cafe 喝杯咖啡", "next": "act_cafe" },
        { "text": "直接搭電車回家", "next": "act_home" }
      ]
    },
    {
      "id": "act_cafe",
      "type": "Action",
      "actionType": "addInt",
      "key": "favorability",
      "value": 15,
      "next": "node_16_a"
    },
    {
      "id": "act_home",
      "type": "Action",
      "actionType": "addInt",
      "key": "favorability",
      "value": 0,
      "next": "node_16_b"
    },
    {
      "id": "node_16_a",
      "type": "Dialogue",
      "speaker": "學姐",
      "text": "好啊！我知道車站附近有一家很安靜的 Cafe。",
      "bgImagePath": "assets/bg/sunset.jpg",
      "characterSpritePath": "assets/characters/senior_happy.png",
      "bgmPath": "",
      "voicePath": "",
      "next": "node_17"
    },
    {
      "id": "node_16_b",
      "type": "Dialogue",
      "speaker": "學姐",
      "text": "好吧，那今天就早點休息吧，明天見囉！",
      "bgImagePath": "assets/bg/sunset.jpg",
      "characterSpritePath": "assets/characters/senior_smile.png",
      "bgmPath": "",
      "voicePath": "",
      "next": "node_17"
    },
    {
      "id": "node_17",
      "type": "Dialogue",
      "speaker": "旁白",
      "text": "離開學校後，我們來到了車站前。",
      "bgImagePath": "assets/bg/station.jpg",
      "characterSpritePath": "",
      "bgmPath": "assets/audio/bgm_daily.ogg",
      "voicePath": "",
      "next": "node_18"
    },
    {
      "id": "node_18",
      "type": "Dialogue",
      "speaker": "我",
      "text": "對了，這款 Gality 引擎之後還打算加入音效漸變 Fade-in/out。",
      "bgImagePath": "assets/bg/station.jpg",
      "characterSpritePath": "",
      "bgmPath": "",
      "voicePath": "",
      "next": "node_19"
    },
    {
      "id": "node_19",
      "type": "Dialogue",
      "speaker": "學姐",
      "text": "還有畫面淡出與溶解對吧？那演出效果一定超棒！",
      "bgImagePath": "assets/bg/station.jpg",
      "characterSpritePath": "assets/characters/senior_happy.png",
      "bgmPath": "",
      "voicePath": "",
      "next": "node_20"
    },
    {
      "id": "node_20",
      "type": "Dialogue",
      "speaker": "我",
      "text": "嗯，Week 7 規劃了 Easing 插值函式庫。",
      "bgImagePath": "assets/bg/station.jpg",
      "characterSpritePath": "assets/characters/senior_normal.png",
      "bgmPath": "",
      "voicePath": "",
      "next": "node_21"
    },
    {
      "id": "node_21",
      "type": "Dialogue",
      "speaker": "學姐",
      "text": "那 Week 8 的劇本 DSL 工具鏈呢？",
      "bgImagePath": "assets/bg/station.jpg",
      "characterSpritePath": "assets/characters/senior_normal.png",
      "bgmPath": "",
      "voicePath": "",
      "next": "node_22"
    },
    {
      "id": "node_22",
      "type": "Dialogue",
      "speaker": "我",
      "text": "到時候只要用純文字寫劇本，轉換器就會自動輸出 JSON 囉！",
      "bgImagePath": "assets/bg/station.jpg",
      "characterSpritePath": "assets/characters/senior_smile.png",
      "bgmPath": "",
      "voicePath": "",
      "next": "node_23"
    },
    {
      "id": "node_23",
      "type": "Dialogue",
      "speaker": "學姐",
      "text": "太好了！這樣非工程師也能輕鬆創作故事了呢！",
      "bgImagePath": "assets/bg/station.jpg",
      "characterSpritePath": "assets/characters/senior_happy.png",
      "bgmPath": "",
      "voicePath": "",
      "next": "node_24"
    },
    {
      "id": "node_24",
      "type": "Dialogue",
      "speaker": "旁白",
      "text": "夜幕低垂，街燈陸續亮起。",
      "bgImagePath": "assets/bg/night_street.jpg",
      "characterSpritePath": "",
      "bgmPath": "assets/audio/bgm_quiet.ogg",
      "voicePath": "",
      "next": "node_25"
    },
    {
      "id": "node_25",
      "type": "Dialogue",
      "speaker": "學姐",
      "text": "謝謝你今天陪我討論這麼久。",
      "bgImagePath": "assets/bg/night_street.jpg",
      "characterSpritePath": "assets/characters/senior_smile.png",
      "bgmPath": "",
      "voicePath": "",
      "next": "node_26"
    },
    {
      "id": "node_26",
      "type": "Dialogue",
      "speaker": "我",
      "text": "哪裡，這是我應該做的。",
      "bgImagePath": "assets/bg/night_street.jpg",
      "characterSpritePath": "assets/characters/senior_smile.png",
      "bgmPath": "",
      "voicePath": "",
      "next": "node_27"
    },
    {
      "id": "node_27",
      "type": "Dialogue",
      "speaker": "學姐",
      "text": "對了，你有沒有想過將這個專案放進 CV？",
      "bgImagePath": "assets/bg/night_street.jpg",
      "characterSpritePath": "assets/characters/senior_normal.png",
      "bgmPath": "",
      "voicePath": "",
      "next": "node_28"
    },
    {
      "id": "node_28",
      "type": "Dialogue",
      "speaker": "我",
      "text": "有啊，這展示了 C++17 記憶體與架構設計能力，面試 Quant/Backend 很加分。",
      "bgImagePath": "assets/bg/night_street.jpg",
      "characterSpritePath": "assets/characters/senior_normal.png",
      "bgmPath": "",
      "voicePath": "",
      "next": "node_29"
    },
    {
      "id": "node_29",
      "type": "Dialogue",
      "speaker": "學姐",
      "text": "相信你一定能拿到心儀的 Offer 的！",
      "bgImagePath": "assets/bg/night_street.jpg",
      "characterSpritePath": "assets/characters/senior_happy.png",
      "bgmPath": "",
      "voicePath": "",
      "next": "node_30"
    },
    {
      "id": "node_30",
      "type": "Dialogue",
      "speaker": "我",
      "text": "謝謝學姐的祝福！",
      "bgImagePath": "assets/bg/night_street.jpg",
      "characterSpritePath": "assets/characters/senior_happy.png",
      "bgmPath": "",
      "voicePath": "",
      "next": "node_31"
    },
    {
      "id": "node_31",
      "type": "Dialogue",
      "speaker": "旁白",
      "text": "電車進站的廣播聲響起。",
      "bgImagePath": "assets/bg/station.jpg",
      "characterSpritePath": "",
      "bgmPath": "assets/audio/bgm_daily.ogg",
      "voicePath": "",
      "next": "node_32"
    },
    {
      "id": "node_32",
      "type": "Dialogue",
      "speaker": "學姐",
      "text": "啊，我的車來了。",
      "bgImagePath": "assets/bg/station.jpg",
      "characterSpritePath": "assets/characters/senior_normal.png",
      "bgmPath": "",
      "voicePath": "",
      "next": "node_33"
    },
    {
      "id": "node_33",
      "type": "Dialogue",
      "speaker": "我",
      "text": "回去路上請小心喔。",
      "bgImagePath": "assets/bg/station.jpg",
      "characterSpritePath": "assets/characters/senior_normal.png",
      "bgmPath": "",
      "voicePath": "",
      "next": "node_34"
    },
    {
      "id": "node_34",
      "type": "Dialogue",
      "speaker": "學姐",
      "text": "恩！明天學校見！",
      "bgImagePath": "assets/bg/station.jpg",
      "characterSpritePath": "assets/characters/senior_smile.png",
      "bgmPath": "",
      "voicePath": "",
      "next": "node_35"
    },
    {
      "id": "node_35",
      "type": "Dialogue",
      "speaker": "旁白",
      "text": "看著學姐登上電車的身影，心中充滿了成就感。",
      "bgImagePath": "assets/bg/station.jpg",
      "characterSpritePath": "",
      "bgmPath": "",
      "voicePath": "",
      "next": "node_36"
    },
    {
      "id": "node_36",
      "type": "Dialogue",
      "speaker": "我",
      "text": "回家後繼續優化引擎的渲染管線吧。",
      "bgImagePath": "assets/bg/room.jpg",
      "characterSpritePath": "",
      "bgmPath": "assets/audio/bgm_quiet.ogg",
      "voicePath": "",
      "next": "node_37"
    },
    {
      "id": "node_37",
      "type": "Dialogue",
      "speaker": "旁白",
      "text": "回到家裡，打開房間的燈。",
      "bgImagePath": "assets/bg/room.jpg",
      "characterSpritePath": "",
      "bgmPath": "",
      "voicePath": "",
      "next": "node_38"
    },
    {
      "id": "node_38",
      "type": "Dialogue",
      "speaker": "我",
      "text": "抱著 Kuromi 娃娃休息一下，接著準備程式碼重構。",
      "bgImagePath": "assets/bg/room.jpg",
      "characterSpritePath": "",
      "bgmPath": "",
      "voicePath": "",
      "next": "node_39"
    },
    {
      "id": "node_39",
      "type": "Dialogue",
      "speaker": "旁白",
      "text": "引擎狀態機正在評估最終的結局條件...",
      "bgImagePath": "assets/bg/room.jpg",
      "characterSpritePath": "",
      "bgmPath": "",
      "voicePath": "",
      "next": "cond_check_ending"
    },
    {
      "id": "cond_check_ending",
      "type": "Condition",
      "conditionType": "greaterThan",
      "key": "favorability",
      "value": 10,
      "trueNext": "node_good_ending",
      "falseNext": "node_normal_ending"
    },
    {
      "id": "node_good_ending",
      "type": "Dialogue",
      "speaker": "系統通知",
      "text": "恭喜！學姐好感度達標！解鎖 【True Developer Ending】！",
      "bgImagePath": "assets/bg/classroom.jpg",
      "characterSpritePath": "assets/characters/senior_happy.png",
      "bgmPath": "assets/audio/bgm_daily.ogg",
      "voicePath": "",
      "next": "end_choice"
    },
    {
      "id": "node_normal_ending",
      "type": "Dialogue",
      "speaker": "系統通知",
      "text": "解鎖 【Normal Developer Ending】！請繼續努力提升好感度！",
      "bgImagePath": "assets/bg/classroom.jpg",
      "characterSpritePath": "assets/characters/senior_normal.png",
      "bgmPath": "assets/audio/bgm_quiet.ogg",
      "voicePath": "",
      "next": "end_choice"
    },
    {
      "id": "end_choice",
      "type": "Choice",
      "speaker": "",
      "text": "",
      "choices": [
        { "text": "重新開始遊戲 (Restart)", "next": "node_01" },
        { "text": "退出遊戲 (Exit)", "next": "node_01" }
      ]
    }
  ]
}
```

---

### File: `saves\save1.json`

```json
{
    "currentNodeId": "node_03_a",
    "flags": {
        "favorability": 10
    }
}
```

---

### File: `src\main.cpp`

```cpp
﻿#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <memory>
#include <filesystem>
#include <unordered_map>
#include <functional>

#include "story/ScriptLoader.hpp" 
#include "core/Blackboard.hpp"
#include "story/StoryExecutor.hpp"
#include "render/DialogueBox.hpp"
#include "render/ChoiceUI.hpp"
#include "render/LayerRenderer.hpp"
#include "render/BacklogUI.hpp"
#include "core/AudioManager.hpp"
#include "core/SaveManager.hpp"

namespace fs = std::filesystem;

int main() {
    // 1. 初始化 SFML 視窗 (SFML 3.x 語法)
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(1280, 720)), "Gality Galgame Engine MVP");
    window.setFramerateLimit(60);

    // 2. 確保 saves 資料夾存在
    fs::create_directories("saves");

    // 3. 初始化數據與腳本
    Blackboard blackboard;
    auto rootNode = ScriptLoader::loadFromFile("assets/scripts/demo_long.json");
    if (!rootNode) {
        std::cerr << "Failed to load script!" << std::endl;
        return -1;
    }

    // 建立 Node ID 索引表 (供 Save/Load 快速跳轉)
    std::unordered_map<std::string, std::shared_ptr<StoryNode>> nodeIndexMap;
    std::function<void(std::shared_ptr<StoryNode>)> buildIndex = [&](std::shared_ptr<StoryNode> node) {
        if (!node || nodeIndexMap.count(node->id)) return;
        nodeIndexMap[node->id] = node;
        if (node->defaultNext) buildIndex(node->defaultNext);
        for (auto& choice : node->choices) {
            if (choice.nextNode) buildIndex(choice.nextNode);
        }
    };
    buildIndex(rootNode);

    StoryExecutor executor(blackboard);
    executor.start(rootNode);

    // 4. 初始化所有 UI 與音效模組
    DialogueBox dialogueBox;
    ChoiceUI choiceUI;
    LayerRenderer layerRenderer;
    AudioManager audioManager;
    BacklogUI backlogUI;

    if (!dialogueBox.loadFont("assets/fonts/font.ttf") || 
        !choiceUI.loadFont("assets/fonts/font.ttf") ||
        !backlogUI.loadFont("assets/fonts/font.ttf")) {
        std::cerr << "Failed to load font. Please ensure assets/fonts/font.ttf exists." << std::endl;
        return -1;
    }

    // 5. 定義狀態同步 Lambda
    auto syncCurrentNodeState = [&](bool recordHistory = true) {
        auto currentNode = executor.getCurrentNode();
        if (!currentNode) return;

        if (currentNode->type == NodeType::Dialogue) {
            dialogueBox.setText(currentNode->speaker, currentNode->text);
            
            // 寫入 Backlog 歷史
            if (recordHistory) {
                backlogUI.addEntry(currentNode->speaker, currentNode->text, currentNode->voicePath);
            }
            
            if (!currentNode->bgImagePath.empty()) layerRenderer.setBackground(currentNode->bgImagePath);
            else layerRenderer.setBackground("");

            if (!currentNode->characterSpritePath.empty()) layerRenderer.setCharacter(currentNode->characterSpritePath);
            else layerRenderer.setCharacter("");
            
            if (!currentNode->bgmPath.empty()) audioManager.playBGM(currentNode->bgmPath);
            audioManager.playVoice(currentNode->voicePath);
            
        } else if (currentNode->type == NodeType::Choice) {
            choiceUI.setChoices(currentNode->choices);
        }
    };

    // 初始第一句同步
    syncCurrentNodeState(true);

    // 6. 主迴圈
    while (window.isOpen()) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);

        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            // -------------------------------------------------------------
            // 🔙 Backlog 滾輪事件處理 (SFML 3.x 語法)
            // -------------------------------------------------------------
            if (const auto* wheelEvt = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (wheelEvt->wheel == sf::Mouse::Wheel::Vertical) {
                    if (wheelEvt->delta > 0.0f) {
                        if (!backlogUI.isVisible()) {
                            backlogUI.setVisible(true); // 向上滾動：開啟 Backlog
                        } else {
                            backlogUI.handleScroll(wheelEvt->delta); // 向上滑動清單
                        }
                    } else if (wheelEvt->delta < 0.0f) {
                        if (backlogUI.isVisible()) {
                            backlogUI.handleScroll(wheelEvt->delta); // 向下滑動清單
                        }
                    }
                }
            }

            // -------------------------------------------------------------
            // 🔙 Backlog 開啟狀態下的鍵盤/滑鼠攔截
            // -------------------------------------------------------------
            if (backlogUI.isVisible()) {
                if (const auto* keyBtn = event->getIf<sf::Event::KeyPressed>()) {
                    if (keyBtn->code == sf::Keyboard::Key::Escape || keyBtn->code == sf::Keyboard::Key::Tab) {
                        backlogUI.setVisible(false);
                    }
                    if (keyBtn->code == sf::Keyboard::Key::Up) backlogUI.handleScroll(1.0f);
                    if (keyBtn->code == sf::Keyboard::Key::Down) backlogUI.handleScroll(-1.0f);
                }
                if (const auto* mouseBtn = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseBtn->button == sf::Mouse::Button::Right) {
                        backlogUI.setVisible(false); // 右鍵關閉
                    }
                }
                continue; // Backlog 開啟時，不處理下方遊戲的推進與點擊
            }

            // -------------------------------------------------------------
            // 全局快捷鍵處理 (Backlog 切換、S 存檔、L 讀檔)
            // -------------------------------------------------------------
            auto currentNode = executor.getCurrentNode();

            if (const auto* keyBtn = event->getIf<sf::Event::KeyPressed>()) {
                // 按 Tab 或 H 鍵開關 Backlog (滑鼠滾輪的備用方案)
                if (keyBtn->code == sf::Keyboard::Key::Tab || keyBtn->code == sf::Keyboard::Key::H) {
                    backlogUI.toggle();
                }

                // S 鍵存檔
                if (keyBtn->code == sf::Keyboard::Key::S && currentNode) {
                    SaveManager::saveGame("saves/save1.json", currentNode->id, blackboard);
                }

                // L 鍵讀檔
                if (keyBtn->code == sf::Keyboard::Key::L) {
                    SaveSnapshot snapshot;
                    if (SaveManager::loadGame("saves/save1.json", snapshot)) {
                        if (nodeIndexMap.count(snapshot.currentNodeId)) {
                            blackboard.setAllInts(snapshot.intFlags);
                            executor.jumpToNode(nodeIndexMap[snapshot.currentNodeId]);
                            syncCurrentNodeState(false); // 讀檔不重複錄入 Backlog
                        }
                    }
                }
            }

            if (!currentNode) continue;

            // -------------------------------------------------------------
            // 遊戲劇情邏輯 (Choice & Dialogue 點擊)
            // -------------------------------------------------------------
            if (currentNode->type == NodeType::Choice) {
                if (event->is<sf::Event::MouseMoved>()) {
                    choiceUI.updateHover(mousePos);
                }
                if (const auto* mouseBtn = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseBtn->button == sf::Mouse::Button::Left) {
                        int selectedOption = choiceUI.handleMouseClick(mousePos);
                        if (selectedOption != -1) {
                            if (currentNode->id == "end_choice") {
                                if (selectedOption == 0) {
                                    audioManager.stopBGM();
                                    blackboard = Blackboard();
                                    backlogUI.clear();
                                    executor.start(rootNode);
                                    syncCurrentNodeState(true);
                                    continue;
                                } else if (selectedOption == 1) {
                                    window.close();
                                    continue;
                                }
                            }

                            executor.advance(selectedOption);
                            syncCurrentNodeState(true);
                        }
                    }
                }
            } 
            else if (currentNode->type == NodeType::Dialogue) {
                bool triggerAdvance = false;
                if (const auto* keyBtn = event->getIf<sf::Event::KeyPressed>()) {
                    if (keyBtn->code == sf::Keyboard::Key::Enter || keyBtn->code == sf::Keyboard::Key::Space) {
                        triggerAdvance = true;
                    }
                }
                if (const auto* mouseBtn = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseBtn->button == sf::Mouse::Button::Left) {
                        triggerAdvance = true;
                    }
                }

                if (triggerAdvance) {
                    if (dialogueBox.onInteract()) {
                        audioManager.stopVoice(); 
                        executor.advance();
                        syncCurrentNodeState(true);
                    }
                }
            }
        }

        // 7. 更新與渲染
        dialogueBox.update();

        window.clear(sf::Color(20, 20, 30));
        
        // 底層：背景與立繪
        layerRenderer.draw(window);

        // 中層：對話框與選項 UI
        if (!executor.isFinished()) {
            auto currentNode = executor.getCurrentNode();
            if (currentNode) {
                if (currentNode->type == NodeType::Dialogue) {
                    dialogueBox.draw(window);
                } else if (currentNode->type == NodeType::Choice) {
                    dialogueBox.draw(window);
                    choiceUI.draw(window);
                }
            }
        }

        // 最上層：Backlog 遮罩
        backlogUI.draw(window);

        window.display();
    }

    return 0;
}
```

---

### File: `src\core\Audiomanager.hpp`

```hpp
#pragma once
#include <SFML/Audio.hpp>
#include <string>
#include <memory>
#include <iostream>
#include "LRUCache.hpp"

class AudioManager {
private:
    sf::Music bgmMusic;
    std::unique_ptr<sf::Sound> voiceSound; // 🔊 使用 unique_ptr 以避開 SFML 3.x 缺乏預設建構函式的限制
    std::string currentBgmPath;

    LRUCache<std::string, std::shared_ptr<sf::SoundBuffer>> voiceCache;

public:
    AudioManager() : voiceCache(20) {}

    void playBGM(const std::string& audioPath) {
        if (audioPath.empty() || audioPath == currentBgmPath) return;

        bgmMusic.stop();
        if (bgmMusic.openFromFile(audioPath)) {
            bgmMusic.setLooping(true);
            bgmMusic.play();
            currentBgmPath = audioPath;
        } else {
            std::cerr << "[AudioManager] Failed to load BGM: " << audioPath << std::endl;
        }
    }

    void stopBGM() {
        bgmMusic.stop();
        currentBgmPath.clear();
    }

    void playVoice(const std::string& audioPath) {
        if (audioPath.empty()) return;

        if (voiceSound) {
            voiceSound->stop();
        }

        std::shared_ptr<sf::SoundBuffer> buffer;
        if (voiceCache.contains(audioPath)) {
            buffer = voiceCache.get(audioPath);
        } else {
            buffer = std::make_shared<sf::SoundBuffer>();
            if (buffer->loadFromFile(audioPath)) {
                voiceCache.put(audioPath, buffer);
            } else {
                std::cerr << "[AudioManager] Failed to load Voice: " << audioPath << std::endl;
                return;
            }
        }

        // SFML 3.x 初始化 sf::Sound 時必須帶入 SoundBuffer
        voiceSound = std::make_unique<sf::Sound>(*buffer);
        voiceSound->play();
    }

    void stopVoice() {
        if (voiceSound) {
            voiceSound->stop();
        }
    }
};
```

---

### File: `src\core\Blackboard.hpp`

```hpp
#pragma once
#include <string>
#include <unordered_map>

class Blackboard {
private:
    std::unordered_map<std::string, int> intVars;
    std::unordered_map<std::string, bool> boolVars;

public:
    Blackboard() = default;

    void setInt(const std::string& key, int value) {
        intVars[key] = value;
    }

    int getInt(const std::string& key, int defaultValue = 0) const {
        auto it = intVars.find(key);
        if (it != intVars.end()) {
            return it->second;
        }
        return defaultValue;
    }

    void addInt(const std::string& key, int delta) {
        intVars[key] = getInt(key, 0) + delta;
    }

    void setBool(const std::string& key, bool value) {
        boolVars[key] = value;
    }

    bool getBool(const std::string& key, bool defaultValue = false) const {
        auto it = boolVars.find(key);
        if (it != boolVars.end()) {
            return it->second;
        }
        return defaultValue;
    }

    // 💾 匯出所有整數旗標 (用於序列化 Save)
    const std::unordered_map<std::string, int>& getAllInts() const {
        return intVars;
    }

    // 📂 批量寫入整數旗標 (用於反序列化 Load)
    void setAllInts(const std::unordered_map<std::string, int>& flags) {
        intVars = flags;
    }
};
```

---

### File: `src\core\LRUCache.hpp`

```hpp
#pragma once
#include <unordered_map>
#include <list>
#include <memory>
#include <string>
#include <iostream>

template <typename Key, typename Value>
class LRUCache {
private:
    size_t capacity;
    using ListIterator = typename std::list<std::pair<Key, Value>>::iterator;
    std::list<std::pair<Key, Value>> itemsList;
    std::unordered_map<Key, ListIterator> itemsMap;

public:
    explicit LRUCache(size_t cap) : capacity(cap) {}

    bool contains(const Key& key) const {
        return itemsMap.find(key) != itemsMap.end();
    }

    Value get(const Key& key) {
        auto it = itemsMap.find(key);
        if (it == itemsMap.end()) {
            throw std::runtime_error("Key not found in LRU Cache");
        }
        // 將被存取的項目移至 list 最前面 (表示最近使用)
        itemsList.splice(itemsList.begin(), itemsList, it->second);
        return it->second->second;
    }

    void put(const Key& key, const Value& value) {
        auto it = itemsMap.find(key);
        if (it != itemsMap.end()) {
            // 已存在，更新數值並搬移到最前
            it->second->second = value;
            itemsList.splice(itemsList.begin(), itemsList, it->second);
            return;
        }

        // 超過容量上限，剔除最久未使用者 (list 末端)
        if (itemsList.size() >= capacity) {
            auto last = itemsList.back();
            itemsMap.erase(last.first);
            itemsList.pop_back();
            std::cout << "[LRU Cache] Evicted asset from memory: " << last.first << std::endl;
        }

        // 新增至前端
        itemsList.push_front({key, value});
        itemsMap[key] = itemsList.begin();
    }

    void clear() {
        itemsMap.clear();
        itemsList.clear();
    }

    size_t size() const {
        return itemsList.size();
    }
};
```

---

### File: `src\core\SaveManager.hpp`

```hpp
#pragma once
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>
#include <iostream>
#include <unordered_map>
#include "Blackboard.hpp"

using json = nlohmann::json;

struct SaveSnapshot {
    std::string currentNodeId;
    std::unordered_map<std::string, int> intFlags;
};

class SaveManager {
public:
    // 儲存狀態至指定 JSON 檔案
    static bool saveGame(const std::string& filePath, const std::string& currentNodeId, const Blackboard& blackboard) {
        try {
            json saveJson;
            saveJson["currentNodeId"] = currentNodeId;
            saveJson["flags"] = blackboard.getAllInts();

            std::ofstream file(filePath);
            if (!file.is_open()) return false;

            file << saveJson.dump(4);
            std::cout << "[SaveManager] Successfully saved to " << filePath << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[SaveManager] Save failed: " << e.what() << std::endl;
            return false;
        }
    }

    // 從 JSON 檔案載入狀態
    static bool loadGame(const std::string& filePath, SaveSnapshot& outSnapshot) {
        try {
            std::ifstream file(filePath);
            if (!file.is_open()) return false;

            json saveJson;
            file >> saveJson;

            outSnapshot.currentNodeId = saveJson.value("currentNodeId", "");
            if (saveJson.contains("flags")) {
                outSnapshot.intFlags = saveJson["flags"].get<std::unordered_map<std::string, int>>();
            }

            std::cout << "[SaveManager] Successfully loaded from " << filePath << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[SaveManager] Load failed: " << e.what() << std::endl;
            return false;
        }
    }
};
```

---

### File: `src\render\BacklogUI.hpp`

```hpp
#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <iostream>

struct HistoryEntry {
    std::string speaker;
    std::string text;
    std::string voicePath;
};

class BacklogUI {
private:
    sf::Font font;
    bool visible = false;
    float scrollOffset = 0.0f;
    std::vector<HistoryEntry> historyEntries;

public:
    BacklogUI() = default;

    bool loadFont(const std::string& fontPath) {
        return font.openFromFile(fontPath);
    }

    void setVisible(bool show) {
        visible = show;
        if (show) scrollOffset = 0.0f; // 開啟時重置滾動
    }

    bool isVisible() const {
        return visible;
    }

    void toggle() {
        setVisible(!visible);
    }

    void addEntry(const std::string& speaker, const std::string& text, const std::string& voicePath = "") {
        historyEntries.push_back({speaker, text, voicePath});
    }

    void clear() {
        historyEntries.clear();
    }

    void handleScroll(float delta) {
        if (!visible) return;
        scrollOffset += delta * 30.0f;
        if (scrollOffset < 0.0f) scrollOffset = 0.0f;
    }

    void draw(sf::RenderWindow& window) {
        if (!visible) return;

        // 1. 半透明黑色背景遮罩
        sf::RectangleShape overlay(sf::Vector2f(window.getSize().x, window.getSize().y));
        overlay.setFillColor(sf::Color(0, 0, 0, 220));
        window.draw(overlay);

        // 2. 標題
        sf::Text titleText(font, "=== HISTORY BACKLOG (Scroll to Navigate | Right Click / Esc to Close) ===", 20);
        titleText.setFillColor(sf::Color(200, 200, 200));
        titleText.setPosition(sf::Vector2f(50.f, 30.f));
        window.draw(titleText);

        // 3. 繪製對話列表 (從最新到最舊倒序渲染)
        float startY = 80.f + scrollOffset;
        float lineSpacing = 70.f;

        for (int i = static_cast<int>(historyEntries.size()) - 1; i >= 0; --i) {
            float currentY = startY + (historyEntries.size() - 1 - i) * lineSpacing;
            
            // 超出螢幕範圍不渲染 (Culling)
            if (currentY < 70.f || currentY > window.getSize().y - 50.f) continue;

            std::string displayText = (historyEntries[i].speaker.empty() ? "" : historyEntries[i].speaker + ": ") + historyEntries[i].text;
            sf::Text entryText(font, sf::String::fromUtf8(displayText.begin(), displayText.end()), 18);
            entryText.setFillColor(sf::Color::White);
            entryText.setPosition(sf::Vector2f(70.f, currentY));
            
            window.draw(entryText);
        }
    }
};
```

---

### File: `src\render\ChoiceUI.hpp`

```hpp
#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "../story/StoryNode.hpp"

struct OptionButton {
    sf::RectangleShape shape;
    sf::Text text;
    size_t index;
    bool isHovered = false;

    // 💡 SFML 3.x Text 建構
    OptionButton(const sf::Font& font) : text(font) {}
};

class ChoiceUI {
private:
    std::vector<OptionButton> buttons;
    sf::Font font;

public:
    ChoiceUI() = default;

    bool loadFont(const std::string& fontPath) {
        return font.openFromFile(fontPath);
    }

    void setChoices(const std::vector<ChoiceOption>& options) {
        buttons.clear();
        float buttonWidth = 800.f;
        float buttonHeight = 55.f;
        float spacing = 20.f;
        float startY = 220.f;

        for (size_t i = 0; i < options.size(); ++i) {
            OptionButton btn(font);
            
            float posX = (1280.f - buttonWidth) / 2.f;
            float posY = startY + i * (buttonHeight + spacing);

            btn.shape.setSize(sf::Vector2f(buttonWidth, buttonHeight));
            btn.shape.setPosition(sf::Vector2f(posX, posY));
            btn.shape.setFillColor(sf::Color(30, 30, 50, 220));
            btn.shape.setOutlineThickness(2.f);
            btn.shape.setOutlineColor(sf::Color(100, 100, 180, 255));

            btn.text.setCharacterSize(22);
            btn.text.setFillColor(sf::Color::White);
            btn.text.setString(sf::String::fromUtf8(options[i].text.begin(), options[i].text.end()));
            
            sf::FloatRect textBounds = btn.text.getLocalBounds();
            // 💡 SFML 3.x：使用 size.x 與 size.y
            btn.text.setPosition(sf::Vector2f(
                posX + (buttonWidth - textBounds.size.x) / 2.f,
                posY + (buttonHeight - textBounds.size.y) / 2.f - 5.f
            ));

            btn.index = i;
            buttons.push_back(std::move(btn));
        }
    }

    void updateHover(sf::Vector2i mousePos) {
        sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
        for (auto& btn : buttons) {
            if (btn.shape.getGlobalBounds().contains(mousePosF)) {
                btn.isHovered = true;
                btn.shape.setFillColor(sf::Color(70, 70, 130, 240));
                btn.shape.setOutlineColor(sf::Color(255, 215, 0, 255));
            } else {
                btn.isHovered = false;
                btn.shape.setFillColor(sf::Color(30, 30, 50, 220));
                btn.shape.setOutlineColor(sf::Color(100, 100, 180, 255));
            }
        }
    }

    int handleMouseClick(sf::Vector2i mousePos) {
        sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
        for (size_t i = 0; i < buttons.size(); ++i) {
            if (buttons[i].shape.getGlobalBounds().contains(mousePosF)) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    void draw(sf::RenderWindow& window) {
        for (auto& btn : buttons) {
            window.draw(btn.shape);
            window.draw(btn.text);
        }
    }
};
```

---

### File: `src\render\DialogueBox.hpp`

```hpp
#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class DialogueBox {
private:
    sf::RectangleShape boxShape;
    sf::RectangleShape nameBoxShape;
    sf::Font font;
    sf::Text nameText;
    sf::Text dialogueText;

    std::string fullText;
    std::u32string unicodeText;
    size_t visibleCharCount = 0;
    
    sf::Clock timer;
    float charDelay = 0.04f;
    bool isCompleted = false;

    std::u32string utf8ToUtf32(const std::string& str) {
        // SFML 3.x 原生支援 UTF-8 轉換
        sf::String sfStr = sf::String::fromUtf8(str.begin(), str.end());
        std::u32string u32;
        for (std::size_t i = 0; i < sfStr.getSize(); ++i) {
            u32.push_back(sfStr[i]);
        }
        return u32;
    }

public:
    // 💡 SFML 3.x：Text 必須在建構時綁定 font
    DialogueBox() : nameText(font), dialogueText(font) 
    {
        // 💡 顯式使用 sf::Vector2f 避免 MSVC 推導錯誤
        boxShape.setSize(sf::Vector2f(1200.f, 200.f));
        boxShape.setFillColor(sf::Color(0, 0, 0, 200));
        boxShape.setOutlineThickness(2.f);
        boxShape.setOutlineColor(sf::Color(255, 255, 255, 100));
        boxShape.setPosition(sf::Vector2f(40.f, 480.f));

        nameBoxShape.setSize(sf::Vector2f(250.f, 45.f));
        nameBoxShape.setFillColor(sf::Color(40, 40, 90, 220));
        nameBoxShape.setPosition(sf::Vector2f(40.f, 430.f));

        nameText.setCharacterSize(22);
        nameText.setFillColor(sf::Color::Yellow);
        nameText.setPosition(sf::Vector2f(55.f, 438.f));

        dialogueText.setCharacterSize(24);
        dialogueText.setFillColor(sf::Color::White);
        dialogueText.setPosition(sf::Vector2f(60.f, 500.f));
    }

    bool loadFont(const std::string& fontPath) {
        return font.openFromFile(fontPath);
    }

    void setText(const std::string& speaker, const std::string& text) {
        nameText.setString(sf::String::fromUtf8(speaker.begin(), speaker.end()));
        fullText = text;
        unicodeText = utf8ToUtf32(text);
        visibleCharCount = 0;
        isCompleted = false;
        timer.restart();
    }

    void update() {
        if (isCompleted) return;

        if (timer.getElapsedTime().asSeconds() >= charDelay) {
            timer.restart();
            if (visibleCharCount < unicodeText.size()) {
                visibleCharCount++;
                std::u32string currentSubStr = unicodeText.substr(0, visibleCharCount);
                dialogueText.setString(sf::String(currentSubStr.c_str()));
            } else {
                isCompleted = true;
            }
        }
    }

    bool onInteract() {
        if (!isCompleted) {
            visibleCharCount = unicodeText.size();
            dialogueText.setString(sf::String(unicodeText.c_str()));
            isCompleted = true;
            return false;
        }
        return true;
    }

    void draw(sf::RenderWindow& window) {
        window.draw(boxShape);
        window.draw(nameBoxShape);
        window.draw(nameText);
        window.draw(dialogueText);
    }
};
```

---

### File: `src\render\LayerRenderer.hpp`

```hpp
#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <memory>
#include <iostream>
#include "../core/LRUCache.hpp"

class LayerRenderer {
private:
    // 快取容量上限設為 10 張紋理
    LRUCache<std::string, std::shared_ptr<sf::Texture>> textureCache;
    
    std::string currentBgPath;
    std::string currentCharacterPath;

    sf::Sprite bgSprite;
    sf::Sprite characterSprite;

    std::shared_ptr<sf::Texture> getOrLoadTexture(const std::string& path) {
        if (path.empty()) return nullptr;

        if (textureCache.contains(path)) {
            return textureCache.get(path);
        }

        auto texture = std::make_shared<sf::Texture>();
        if (!texture->loadFromFile(path)) {
            std::cerr << "[LayerRenderer] Failed to load texture: " << path << std::endl;
            return nullptr;
        }

        textureCache.put(path, texture);
        return texture;
    }

public:
    LayerRenderer() : textureCache(10) {}

    void setBackground(const std::string& imagePath) {
        if (imagePath == currentBgPath) return;
        currentBgPath = imagePath;

        auto tex = getOrLoadTexture(imagePath);
        if (tex) {
            bgSprite.setTexture(*tex, true);
        }
    }

    void setCharacter(const std::string& imagePath) {
        if (imagePath == currentCharacterPath) return;
        currentCharacterPath = imagePath;

        if (imagePath.empty()) return;

        auto tex = getOrLoadTexture(imagePath);
        if (tex) {
            characterSprite.setTexture(*tex, true);
        }
    }

    void draw(sf::RenderWindow& window) {
        if (!currentBgPath.empty()) {
            window.draw(bgSprite);
        }
        if (!currentCharacterPath.empty()) {
            window.draw(characterSprite);
        }
    }
};
```

---

### File: `src\story\ScriptLoader.hpp`

```hpp
﻿#pragma once
#include <nlohmann/json.hpp>
#include <fstream>
#include <unordered_map>
#include <memory>
#include <iostream>
#include "StoryNode.hpp"

using json = nlohmann::json;

class ScriptLoader {
public:
    static std::shared_ptr<StoryNode> loadFromFile(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Failed to open script file: " << filePath << std::endl;
            return nullptr;
        }

        json scriptJson;
        file >> scriptJson;

        std::unordered_map<std::string, std::shared_ptr<StoryNode>> nodeMap;

        // 第一階段：構建所有節點實例
        for (const auto& item : scriptJson["nodes"]) {
            std::string id = item["id"];
            std::string typeStr = item["type"];
            NodeType type = NodeType::Dialogue;

            if (typeStr == "choice") type = NodeType::Choice;
            else if (typeStr == "condition") type = NodeType::Condition;
            else if (typeStr == "action") type = NodeType::Action;

            auto node = std::make_shared<StoryNode>(id, type);

            if (type == NodeType::Dialogue) {
                node->speaker = item.value("speaker", "");
                node->text = item.value("text", "");
                node->bgImagePath = item.value("bg", "");
                node->characterSpritePath = item.value("character", "");
                node->bgmPath = item.value("bgm", "");
                node->voicePath = item.value("cv", "");
            } else if (type == NodeType::Action) {
                std::string flag = item["flag"];
                int val = item["value"];
                node->actionFunc = [flag, val](Blackboard& bb) {
                    bb.addInt(flag, val);
                };
            }

            nodeMap[id] = node;
        }

        // 第二階段：建立 Link 關係
        for (const auto& item : scriptJson["nodes"]) {
            std::string id = item["id"];
            auto node = nodeMap[id];

            if (item.contains("next")) {
                std::string nextId = item["next"];
                if (nodeMap.count(nextId)) node->defaultNext = nodeMap[nextId];
            }

            if (item.contains("choices")) {
                for (const auto& c : item["choices"]) {
                    ChoiceOption opt;
                    opt.text = c["text"];
                    std::string targetId = c["next"];
                    if (nodeMap.count(targetId)) opt.nextNode = nodeMap[targetId];
                    node->choices.push_back(opt);
                }
            }

            if (node->type == NodeType::Condition) {
                std::string flag = item["condition"]["flag"];
                int targetVal = item["condition"]["value"];
                std::string trueId = item["condition"]["trueNext"];
                std::string falseId = item["condition"]["falseNext"];

                node->conditionFunc = [flag, targetVal, trueId, falseId, nodeMap](const Blackboard& bb) {
                    if (bb.getInt(flag) >= targetVal) {
                        return nodeMap.at(trueId);
                    }
                    return nodeMap.at(falseId);
                };
            }
        }

        std::string startId = scriptJson.value("startNode", "start");
        return nodeMap.count(startId) ? nodeMap[startId] : nullptr;
    }
};
```

---

### File: `src\story\StoryExecutor.hpp`

```hpp
﻿#pragma once
#include <memory>
#include <vector>
#include "StoryNode.hpp"
#include "../core/Blackboard.hpp"
#include "../core/SaveManager.hpp"

class StoryExecutor {
private:
    std::shared_ptr<StoryNode> currentNode;
    Blackboard& blackboard;
    
    // 🔙 Week 5: 狀態快照堆疊 (用於 Rollback)
    std::vector<SaveSnapshot> historyStack;

public:
    explicit StoryExecutor(Blackboard& bb) : blackboard(bb) {}

    void start(std::shared_ptr<StoryNode> rootNode) {
        historyStack.clear();
        currentNode = rootNode;
        evaluateCurrentNode();
    }

    void jumpToNode(std::shared_ptr<StoryNode> node) {
        currentNode = node;
    }

    void advance(int choiceIndex = -1) {
        if (!currentNode) return;

        // 推進前記錄當前快照
        recordSnapshot();

        if (currentNode->type == NodeType::Dialogue) {
            currentNode = currentNode->defaultNext;
        } else if (currentNode->type == NodeType::Choice) {
            if (choiceIndex >= 0 && choiceIndex < static_cast<int>(currentNode->choices.size())) {
                currentNode = currentNode->choices[choiceIndex].nextNode;
            }
        }

        evaluateCurrentNode();
    }

    void recordSnapshot() {
        if (currentNode) {
            historyStack.push_back({currentNode->id, blackboard.getAllInts()});
        }
    }

    // 🔙 倒退回上一個 Dialogue/Choice 節點
    bool rollback(SaveSnapshot& outSnapshot) {
        if (historyStack.empty()) return false;

        outSnapshot = historyStack.back();
        historyStack.pop_back();
        return true;
    }

    void evaluateCurrentNode() {
        while (currentNode) {
            if (currentNode->type == NodeType::Action) {
                if (currentNode->actionFunc) {
                    currentNode->actionFunc(blackboard);
                }
                currentNode = currentNode->defaultNext;
            } else if (currentNode->type == NodeType::Condition) {
                if (currentNode->conditionFunc) {
                    currentNode = currentNode->conditionFunc(blackboard);
                } else {
                    currentNode = currentNode->defaultNext;
                }
            } else {
                break;
            }
        }
    }

    std::shared_ptr<StoryNode> getCurrentNode() const {
        return currentNode;
    }

    bool isFinished() const {
        return currentNode == nullptr;
    }
};
```

---

### File: `src\story\StoryNode.hpp`

```hpp
﻿#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "../core/Blackboard.hpp"

enum class NodeType {
    Dialogue,   // 顯示對話與角色立繪
    Choice,     // 等待玩家選擇分歧
    Condition,  // 根據條件自動轉向分支
    Action      // 執行指令（如更改 Flag、增加好感度）
};

struct ChoiceOption {
    std::string text;
    std::shared_ptr<struct StoryNode> nextNode;
};

struct StoryNode {
    std::string id;
    NodeType type;

    // Dialogue 節點屬性
    std::string speaker;
    std::string text;
    std::string bgImagePath;
    std::string characterSpritePath;
    std::string bgmPath;
    std::string voicePath;

    // Choice / Branch 下一步連接
    std::vector<ChoiceOption> choices;
    std::shared_ptr<StoryNode> defaultNext;

    // Condition 節點的評估邏輯: 傳入 Blackboard, 回傳該跳轉的目標節點
    std::function<std::shared_ptr<StoryNode>(const Blackboard&)> conditionFunc;

    // Action 節點執行的動作
    std::function<void(Blackboard&)> actionFunc;

    explicit StoryNode(std::string nodeId, NodeType nodeType) 
        : id(std::move(nodeId)), type(nodeType) {}
};
```

---

