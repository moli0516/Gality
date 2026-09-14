#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
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
#include "render/DebugOverlay.hpp"
#include "render/WeatherSystem.hpp"
#include "render/PostFX.hpp"
#include "render/UITheme.hpp"
#include "render/TitleMenu.hpp" // 💡 Week 11 TitleMenu
#include "core/AudioManager.hpp"
#include "core/SaveManager.hpp"

namespace fs = std::filesystem;

int main() {
    // 1. 初始化 SFML 視窗
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(1280, 720)), "Gality Galgame Engine MVP");
    window.setFramerateLimit(60);

    // 離屏渲染緩衝區 (SFML 3.x 建構子語法)
    sf::RenderTexture sceneBuffer(sf::Vector2u(1280, 720));

    // 2. 初始化 ImGui-SFML
    if (!ImGui::SFML::Init(window)) {
        std::cerr << "Failed to initialize ImGui-SFML!" << std::endl;
        return -1;
    }

    // 3. 載入中文字型至 ImGui Font Atlas
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF(
        "assets/fonts/font.ttf",
        16.0f,
        nullptr,
        io.Fonts->GetGlyphRangesChineseSimplifiedCommon()
    );
    ImGui::StyleColorsDark();

    // 4. 確保 saves 與 config 資料夾存在
    fs::create_directories("saves");
    fs::create_directories("assets/config");

    // 5. 初始化數據與腳本
    Blackboard blackboard;
    auto rootNode = ScriptLoader::loadFromFile("assets/scripts/demo_long.json");
    if (!rootNode) {
        std::cerr << "Failed to load script!" << std::endl;
        return -1;
    }

    // 建立 Node ID 索引表
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

    // 6. 初始化 UI、音效、TitleMenu、Weather 與 PostFX 模組
    DialogueBox dialogueBox;
    ChoiceUI choiceUI;
    LayerRenderer layerRenderer;
    AudioManager audioManager;
    BacklogUI backlogUI;
    DebugOverlay debugOverlay;
    WeatherSystem weatherSystem(window.getSize());
    PostFX postFX;
    TitleMenu titleMenu; // 💡 Week 11 主選單

    if (!dialogueBox.loadFont("assets/fonts/font.ttf") || 
        !choiceUI.loadFont("assets/fonts/font.ttf") ||
        !backlogUI.loadFont("assets/fonts/font.ttf") ||
        !titleMenu.loadFont("assets/fonts/font.ttf")) {
        std::cerr << "Failed to load font. Please ensure assets/fonts/font.ttf exists." << std::endl;
        return -1;
    }

    // 💡 載入打字音效 (可選)
    dialogueBox.loadTypeSound("assets/audio/typewriter.wav");

    // 💡 載入與套用 Data-Driven UI Theme 配置
    UITheme uiTheme;
    uiTheme.loadFromFile("assets/config/ui_theme.json");
    dialogueBox.applyTheme(uiTheme.dialogueStyle);
    choiceUI.applyTheme(uiTheme.choiceStyle);

    // 7. 定義狀態同步 Lambda
    auto syncCurrentNodeState = [&](bool recordHistory = true) {
        auto currentNode = executor.getCurrentNode();
        if (!currentNode) return;

        if (currentNode->type == NodeType::Dialogue) {
            dialogueBox.setText(currentNode->speaker, currentNode->text);
            
            if (recordHistory) {
                backlogUI.addEntry(currentNode->speaker, currentNode->text, currentNode->voicePath);
            }
            
            if (!currentNode->bgImagePath.empty()) layerRenderer.setBackground(currentNode->bgImagePath);
            else layerRenderer.setBackground("");

            if (!currentNode->characterSpritePath.empty()) layerRenderer.setCharacter(currentNode->characterSpritePath);
            else layerRenderer.setCharacter("");
            
            if (!currentNode->bgmPath.empty()) audioManager.playBGM(currentNode->bgmPath);
            audioManager.playVoice(currentNode->voicePath);

            if (!currentNode->weather.empty()) {
                if (currentNode->weather == "rain") weatherSystem.setWeather(WeatherType::Rain);
                else if (currentNode->weather == "snow") weatherSystem.setWeather(WeatherType::Snow);
                else if (currentNode->weather == "sakura") weatherSystem.setWeather(WeatherType::Sakura);
                else if (currentNode->weather == "none") weatherSystem.setWeather(WeatherType::None);
            }

            if (currentNode->shake > 0.0f) {
                postFX.triggerShake(currentNode->shake, 15.0f);
            }
            
        } else if (currentNode->type == NodeType::Choice) {
            choiceUI.setChoices(currentNode->choices);
        }
    };

    // 💡 綁定 TitleMenu 按鈕動作
    titleMenu.initButtons(
        [&]() { // Start Game
            titleMenu.setVisible(false);
            blackboard = Blackboard();
            backlogUI.clear();
            executor.start(rootNode);
            syncCurrentNodeState(true);
        },
        [&]() { // Load Game
            SaveSnapshot snapshot;
            if (SaveManager::loadGame("saves/save1.json", snapshot)) {
                if (nodeIndexMap.count(snapshot.currentNodeId)) {
                    blackboard.setAllInts(snapshot.intFlags);
                    executor.jumpToNode(nodeIndexMap[snapshot.currentNodeId]);
                    titleMenu.setVisible(false);
                    syncCurrentNodeState(false);
                }
            }
        },
        [&]() { // Exit
            window.close();
        }
    );

    sf::Clock deltaClock;

    // 8. 主迴圈
    while (window.isOpen()) {
        sf::Time dt = deltaClock.restart();
        float deltaTime = dt.asSeconds();

        // 更新 ImGui 與各模組
        ImGui::SFML::Update(window, dt);
        audioManager.update(deltaTime);
        layerRenderer.update(deltaTime);
        weatherSystem.update(deltaTime);
        
        bool shouldBlur = titleMenu.isVisible() || backlogUI.isVisible() || debugOverlay.getIsVisible();
        postFX.setBlur(shouldBlur ? 3.5f : 0.0f);
        postFX.update(deltaTime, window.getSize());

        sf::Vector2i mousePos = sf::Mouse::getPosition(window);

        while (const auto event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);

            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            // F1 / ~ 切換 Debugger
            if (const auto* keyBtn = event->getIf<sf::Event::KeyPressed>()) {
                if (keyBtn->code == sf::Keyboard::Key::Grave || keyBtn->code == sf::Keyboard::Key::F1) {
                    debugOverlay.toggle();
                }
            }

            if (ImGui::GetIO().WantCaptureKeyboard || ImGui::GetIO().WantCaptureMouse) {
                continue;
            }

            // 💡 主選單開頭攔截事件
            if (titleMenu.isVisible()) {
                if (event->is<sf::Event::MouseMoved>()) {
                    titleMenu.updateHover(mousePos);
                }
                if (const auto* mouseBtn = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseBtn->button == sf::Mouse::Button::Left) {
                        titleMenu.handleClick(mousePos);
                    }
                }
                continue;
            }

            // Backlog 滾輪處理
            if (const auto* wheelEvt = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (wheelEvt->wheel == sf::Mouse::Wheel::Vertical) {
                    if (wheelEvt->delta > 0.0f) {
                        if (!backlogUI.isVisible()) {
                            backlogUI.setVisible(true);
                        } else {
                            backlogUI.handleScroll(wheelEvt->delta);
                        }
                    } else if (wheelEvt->delta < 0.0f) {
                        if (backlogUI.isVisible()) {
                            backlogUI.handleScroll(wheelEvt->delta);
                        }
                    }
                }
            }

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
                        backlogUI.setVisible(false);
                    }
                }
                continue;
            }

            // 快捷鍵處理
            auto currentNode = executor.getCurrentNode();

            if (const auto* keyBtn = event->getIf<sf::Event::KeyPressed>()) {
                if (keyBtn->code == sf::Keyboard::Key::Tab || keyBtn->code == sf::Keyboard::Key::H) {
                    backlogUI.toggle();
                }

                if (keyBtn->code == sf::Keyboard::Key::K) {
                    postFX.triggerShake(0.4f, 20.0f);
                }

                if (keyBtn->code == sf::Keyboard::Key::S && currentNode) {
                    SaveManager::saveGame("saves/save1.json", currentNode->id, blackboard);
                }

                if (keyBtn->code == sf::Keyboard::Key::L) {
                    SaveSnapshot snapshot;
                    if (SaveManager::loadGame("saves/save1.json", snapshot)) {
                        if (nodeIndexMap.count(snapshot.currentNodeId)) {
                            blackboard.setAllInts(snapshot.intFlags);
                            executor.jumpToNode(nodeIndexMap[snapshot.currentNodeId]);
                            syncCurrentNodeState(false);
                        }
                    }
                }
            }

            if (!currentNode) continue;

            // 遊戲劇情推進
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
                                    titleMenu.setVisible(true); // 重新返回主選單
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

        // 9. 渲染流程
        dialogueBox.update();

        sceneBuffer.clear(sf::Color(20, 20, 30));
        layerRenderer.draw(sceneBuffer);

        if (!titleMenu.isVisible() && !executor.isFinished()) {
            auto currentNode = executor.getCurrentNode();
            if (currentNode) {
                if (currentNode->type == NodeType::Dialogue) {
                    dialogueBox.draw(sceneBuffer);
                } else if (currentNode->type == NodeType::Choice) {
                    dialogueBox.draw(sceneBuffer);
                    choiceUI.draw(sceneBuffer);
                }
            }
        }
        weatherSystem.draw(sceneBuffer);
        sceneBuffer.display();

        window.clear();
        postFX.applyAndDraw(window, sceneBuffer);

        // 最上層 UI 繪製
        titleMenu.draw(window);
        backlogUI.draw(window);

        debugOverlay.draw(window, blackboard, executor, nodeIndexMap, [&]() {
            syncCurrentNodeState(false);
        });

        ImGui::SFML::Render(window);

        window.display();
    }

    ImGui::SFML::Shutdown();

    return 0;
}