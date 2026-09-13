#include <SFML/Graphics.hpp>
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

    // ⏱️ Week 7: 初始化 Delta Clock 用於平滑過渡與動畫更新
    sf::Clock deltaClock;

    // 6. 主迴圈
    while (window.isOpen()) {
        float deltaTime = deltaClock.restart().asSeconds();

        // ⏱️ Week 7: 更新音訊漸變與圖層動畫
        audioManager.update(deltaTime);
        layerRenderer.update(deltaTime);

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
        
        // 底層：背景與立繪 (自動處理 Crossfade 渲染)
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