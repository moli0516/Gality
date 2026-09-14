#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <memory>
#include <filesystem>
#include <unordered_map>
#include <functional>

#ifdef _WIN32
#include <windows.h>
#endif

#include "core/ConfigManager.hpp"
#include "core/AssetPack.hpp"
#include "story/ScriptLoader.hpp" 
#include "core/Blackboard.hpp"
#include "core/LRUCache.hpp"
#include "core/HotReloader.hpp"
#include "story/StoryExecutor.hpp"
#include "render/DialogueBox.hpp"
#include "render/ChoiceUI.hpp"
#include "render/LayerRenderer.hpp"
#include "render/BacklogUI.hpp"
#include "render/DebugOverlay.hpp"
#include "render/NodeGraphViewer.hpp"
#include "render/WeatherSystem.hpp"
#include "render/PostFX.hpp"
#include "render/TransitionSystem.hpp"
#include "render/UITheme.hpp"
#include "render/TitleMenu.hpp"
#include "render/SettingsUI.hpp"
#include "render/SaveLoadUI.hpp"
#include "core/AudioManager.hpp"
#include "core/SaveManager.hpp"

namespace fs = std::filesystem;

// 1080p 邏輯視窗解析度基準
constexpr unsigned int LOGICAL_WIDTH = 1920;
constexpr unsigned int LOGICAL_HEIGHT = 1080;

// Letterbox 視口計算工具函式 (防超寬/非 16:9 螢幕畫面拉伸)
sf::View calculateLetterboxView(sf::Vector2u windowSize) {
    float windowRatio = static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);
    float targetRatio = static_cast<float>(LOGICAL_WIDTH) / static_cast<float>(LOGICAL_HEIGHT);

    sf::View view(sf::FloatRect(sf::Vector2f(0.f, 0.f), sf::Vector2f(static_cast<float>(LOGICAL_WIDTH), static_cast<float>(LOGICAL_HEIGHT))));

    if (windowRatio >= targetRatio) {
        // 螢幕更寬 (Pillarbox: 左右黑邊)
        float viewportWidth = targetRatio / windowRatio;
        float viewportX = (1.0f - viewportWidth) * 0.5f;
        view.setViewport(sf::FloatRect(sf::Vector2f(viewportX, 0.0f), sf::Vector2f(viewportWidth, 1.0f)));
    } else {
        // 螢幕更高 (Letterbox: 上下黑邊)
        float viewportHeight = windowRatio / targetRatio;
        float viewportY = (1.0f - viewportHeight) * 0.5f;
        view.setViewport(sf::FloatRect(sf::Vector2f(0.0f, viewportY), sf::Vector2f(1.0f, viewportHeight)));
    }

    return view;
}

int main() {
    // 1. 初始化 1080p 全螢幕視窗與 Letterbox 視圖
    auto videoMode = sf::VideoMode::getDesktopMode();
    sf::RenderWindow window(videoMode, "Gality Engine", sf::State::Fullscreen);
    window.setFramerateLimit(60);

    sf::View letterboxView = calculateLetterboxView(window.getSize());

    // 離屏緩衝區維持 1920x1080 原始畫質
    sf::RenderTexture sceneBuffer(sf::Vector2u(LOGICAL_WIDTH, LOGICAL_HEIGHT));
    sf::RenderTexture compositeBuffer(sf::Vector2u(LOGICAL_WIDTH, LOGICAL_HEIGHT));

    // 2. 建立目錄與載入配置
    fs::create_directories("saves");
    fs::create_directories("assets/config");
    fs::create_directories("assets/masks");

    ConfigManager::load();

    // 3. 初始化黑板與快取
    Blackboard blackboard;
    LRUCache<std::string, std::shared_ptr<sf::Texture>> textureCache(50); 

    auto rootNode = ScriptLoader::loadFromFile("assets/scripts/demo_long.json");
    if (!rootNode) {
#ifdef _WIN32
        MessageBoxW(NULL, L"Cannot open script file: assets/scripts/demo_long.json", L"Gality Engine Error", MB_ICONERROR | MB_OK);
#else
        std::cerr << "Failed to load script: assets/scripts/demo_long.json" << std::endl;
#endif
        return -1;
    }

    std::unordered_map<std::string, std::shared_ptr<StoryNode>> nodeIndexMap;
    for (const auto& [id, nodePtr] : ScriptLoader::nodeRegistry) {
        nodeIndexMap[id] = nodePtr;
    }

    StoryExecutor executor(blackboard);

    // 4. 初始化子系統與原生 UI 模組 (純向量渲染，零 ImGui 依賴)
    DialogueBox dialogueBox;
    ChoiceUI choiceUI;
    LayerRenderer layerRenderer(textureCache); 
    AudioManager audioManager;
    BacklogUI backlogUI;
    DebugOverlay debugOverlay;
    NodeGraphViewer nodeGraphViewer;
    SettingsUI settingsUI;
    SaveLoadUI saveLoadUI;
    WeatherSystem weatherSystem(sf::Vector2u(LOGICAL_WIDTH, LOGICAL_HEIGHT));
    PostFX postFX;
    TransitionSystem transitionSystem(sf::Vector2u(LOGICAL_WIDTH, LOGICAL_HEIGHT));
    TitleMenu titleMenu;

    HotReloader hotReloader("assets/scripts/demo_long.json", "assets/shaders/blur.frag");

    if (!dialogueBox.loadFont("assets/fonts/font.ttf") || 
        !choiceUI.loadFont("assets/fonts/font.ttf") ||
        !backlogUI.loadFont("assets/fonts/font.ttf") ||
        !titleMenu.loadFont("assets/fonts/font.ttf") ||
        !settingsUI.loadFont("assets/fonts/font.ttf") ||
        !saveLoadUI.loadFont("assets/fonts/font.ttf") ||
        !debugOverlay.loadFont("assets/fonts/font.ttf") ||
        !nodeGraphViewer.loadFont("assets/fonts/font.ttf")) {
#ifdef _WIN32
        MessageBoxW(NULL, L"Failed to load required font: assets/fonts/font.ttf", L"Gality Engine Error", MB_ICONERROR | MB_OK);
#else
        std::cerr << "Failed to load font: assets/fonts/font.ttf" << std::endl;
#endif
        return -1;
    }

    dialogueBox.loadTypeSound("assets/audio/typewriter.wav");

    UITheme uiTheme;
    uiTheme.loadFromFile("assets/config/ui_theme.json");
    dialogueBox.applyTheme(uiTheme.dialogueStyle);
    choiceUI.applyTheme(uiTheme.choiceStyle);

    std::string previousBgPath = "";
    std::string currentPlayingBgm = "";
    std::string currentWeatherStr = "none";
    std::map<CharSlot, std::string> currentSlots;
    std::optional<CharSlot> currentActiveSlot = CharSlot::Center;

    // 5. 狀態同步 Lambda (適配 1080p 邏輯尺寸)
    auto syncCurrentNodeState = [&](bool recordHistory = true, bool playTransition = true) {
        auto currentNode = executor.getCurrentNode();
        if (!currentNode) return;

        if (currentNode->type == NodeType::Dialogue) {
            dialogueBox.setText(currentNode->speaker, currentNode->text);
            
            if (recordHistory) {
                backlogUI.addEntry(currentNode->speaker, currentNode->text, currentNode->voicePath);
            }
            
            if (!currentNode->bgImagePath.empty() && currentNode->bgImagePath != previousBgPath) {
                if (!previousBgPath.empty() && playTransition) {
                    sceneBuffer.clear(sf::Color(20, 20, 30));
                    layerRenderer.draw(sceneBuffer);
                    sceneBuffer.display();

                    std::string maskName = currentNode->transitionMask.empty() ? "diamond" : currentNode->transitionMask;
                    float dur = (currentNode->transitionDuration > 0.0f) ? currentNode->transitionDuration : 1.0f;
                    transitionSystem.start(sceneBuffer, maskName, dur, 0.15f);
                }
                
                layerRenderer.setBackground(currentNode->bgImagePath, sf::Vector2u(LOGICAL_WIDTH, LOGICAL_HEIGHT));
                previousBgPath = currentNode->bgImagePath;
            } else if (currentNode->bgImagePath.empty()) {
                layerRenderer.setBackground("");
                previousBgPath = "";
            }

            if (!currentNode->slotTextures.empty()) {
                currentSlots = currentNode->slotTextures;
                currentActiveSlot = currentNode->activeSlot;
                layerRenderer.updateCharacters(currentSlots, currentActiveSlot);
            } else if (!currentNode->characterSpritePath.empty()) {
                currentSlots.clear();
                currentSlots[CharSlot::Center] = currentNode->characterSpritePath;
                currentActiveSlot = CharSlot::Center;
                layerRenderer.setCharacter(currentNode->characterSpritePath);
            } else {
                currentSlots.clear();
                currentActiveSlot = std::nullopt;
                layerRenderer.setCharacter("");
            }
            
            if (!currentNode->bgmPath.empty()) {
                currentPlayingBgm = currentNode->bgmPath;
                audioManager.playBGM(currentNode->bgmPath);
            }
            if (!currentNode->voicePath.empty()) {
                audioManager.playVoice(currentNode->voicePath, currentActiveSlot);
            }

            currentWeatherStr = currentNode->weather;
            if (!currentNode->weather.empty()) {
                if (currentNode->weather == "rain") weatherSystem.setWeather(WeatherType::Rain);
                else if (currentNode->weather == "snow") weatherSystem.setWeather(WeatherType::Snow);
                else if (currentNode->weather == "sakura") weatherSystem.setWeather(WeatherType::Sakura);
                else if (currentNode->weather == "none") weatherSystem.setWeather(WeatherType::None);
            }

            if (currentNode->shake > 0.0f) {
                postFX.triggerShake(currentNode->shake, 20.0f);
            }

            executor.updatePresentationState(previousBgPath, currentSlots, currentActiveSlot, currentWeatherStr, currentPlayingBgm);
            
        } else if (currentNode->type == NodeType::Choice) {
            choiceUI.setChoices(currentNode->choices);
        }
    };

    // 6. 時光機 Rollback
    auto performRollback = [&]() {
        SaveSnapshot snapshot;
        if (executor.rollback(snapshot)) {
            if (nodeIndexMap.count(snapshot.currentNodeId)) {
                audioManager.stopVoice();
                executor.jumpToNode(nodeIndexMap[snapshot.currentNodeId]);

                previousBgPath = snapshot.bgImagePath;
                layerRenderer.setBackground(snapshot.bgImagePath, sf::Vector2u(LOGICAL_WIDTH, LOGICAL_HEIGHT));
                currentSlots = snapshot.slotTextures;
                currentActiveSlot = snapshot.activeSlot;
                layerRenderer.updateCharacters(currentSlots, currentActiveSlot);

                currentWeatherStr = snapshot.weather;
                if (currentWeatherStr == "rain") weatherSystem.setWeather(WeatherType::Rain);
                else if (currentWeatherStr == "snow") weatherSystem.setWeather(WeatherType::Snow);
                else if (currentWeatherStr == "sakura") weatherSystem.setWeather(WeatherType::Sakura);
                else weatherSystem.setWeather(WeatherType::None);

                if (!snapshot.bgmPath.empty() && snapshot.bgmPath != currentPlayingBgm) {
                    currentPlayingBgm = snapshot.bgmPath;
                    audioManager.playBGM(snapshot.bgmPath);
                }

                auto restoredNode = executor.getCurrentNode();
                if (restoredNode) {
                    dialogueBox.setText(restoredNode->speaker, restoredNode->text);
                    dialogueBox.onInteract();
                    if (!restoredNode->voicePath.empty()) {
                        audioManager.playVoice(restoredNode->voicePath, currentActiveSlot);
                    }
                }

                executor.updatePresentationState(previousBgPath, currentSlots, currentActiveSlot, currentWeatherStr, currentPlayingBgm);
                return true;
            }
        }
        return false;
    };

    auto saveToSlot = [&](int slotIndex) {
        std::string filePath = "saves/save_slot_" + std::to_string(slotIndex) + ".json";
        if (!executor.getCurrentNode()) return;

        SaveSnapshot snapshot = executor.getCurrentPresentationState();
        snapshot.currentNodeId = executor.getCurrentNode()->id;
        snapshot.intFlags = blackboard.getAllInts();
        SaveManager::saveGame(filePath, snapshot);
    };

    auto loadFromSlot = [&](int slotIndex) {
        std::string filePath = "saves/save_slot_" + std::to_string(slotIndex) + ".json";
        SaveSnapshot snapshot;
        if (!SaveManager::loadGame(filePath, snapshot)) {
            return;
        }

        if (!nodeIndexMap.count(snapshot.currentNodeId)) {
            return;
        }

        blackboard.setAllInts(snapshot.intFlags);
        executor.jumpToNode(nodeIndexMap[snapshot.currentNodeId]);
        titleMenu.setVisible(false);

        previousBgPath = snapshot.bgImagePath;
        layerRenderer.setBackground(snapshot.bgImagePath, sf::Vector2u(LOGICAL_WIDTH, LOGICAL_HEIGHT));
        currentSlots = snapshot.slotTextures;
        currentActiveSlot = snapshot.activeSlot;
        layerRenderer.updateCharacters(currentSlots, currentActiveSlot);

        currentWeatherStr = snapshot.weather;
        if (currentWeatherStr == "rain") weatherSystem.setWeather(WeatherType::Rain);
        else if (currentWeatherStr == "snow") weatherSystem.setWeather(WeatherType::Snow);
        else if (currentWeatherStr == "sakura") weatherSystem.setWeather(WeatherType::Sakura);
        else weatherSystem.setWeather(WeatherType::None);

        if (!snapshot.bgmPath.empty()) {
            currentPlayingBgm = snapshot.bgmPath;
            audioManager.playBGM(snapshot.bgmPath);
        }

        syncCurrentNodeState(false, false);
        auto loadedNode = executor.getCurrentNode();
        if (loadedNode && !loadedNode->voicePath.empty()) {
            audioManager.playVoice(loadedNode->voicePath, currentActiveSlot);
        }
    };

    titleMenu.initButtons(
        [&]() {
            titleMenu.setVisible(false);
            blackboard = Blackboard();
            backlogUI.clear();
            executor.start(rootNode);
            syncCurrentNodeState(true, false);
        },
        [&]() {
            saveLoadUI.open(SaveLoadMode::Load, sf::Vector2u(LOGICAL_WIDTH, LOGICAL_HEIGHT), saveToSlot, loadFromSlot);
        },
        [&]() {
            window.close();
        }
    );

    sf::Clock deltaClock;
    float currentFps = 60.0f;

    // 7. 主遊戲迴圈
    while (window.isOpen()) {
        sf::Time dt = deltaClock.restart();
        float deltaTime = dt.asSeconds();
        if (deltaTime > 0.0f) {
            currentFps = 0.9f * currentFps + 0.1f * (1.0f / deltaTime);
        }

        hotReloader.update(deltaTime,
            [&]() {
                auto newRoot = ScriptLoader::loadFromFile("assets/scripts/demo_long.json");
                if (newRoot) {
                    nodeIndexMap.clear();
                    for (const auto& [id, nodePtr] : ScriptLoader::nodeRegistry) {
                        nodeIndexMap[id] = nodePtr;
                    }
                    debugOverlay.log("[HotReloader] Script AST reloaded successfully.");
                }
            },
            [&]() {
                postFX = PostFX();
                debugOverlay.log("[HotReloader] GLSL Shaders reloaded successfully.");
            }
        );

        // 💡 關鍵修復：將滑鼠物理座標無損映射到 1080p 邏輯空間
        sf::Vector2i pixelMousePos = sf::Mouse::getPosition(window);
        sf::Vector2f logicalMousePosF = window.mapPixelToCoords(pixelMousePos, letterboxView);
        sf::Vector2i logicalMousePosI(static_cast<int>(logicalMousePosF.x), static_cast<int>(logicalMousePosF.y));

        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (const auto* resizeEvt = event->getIf<sf::Event::Resized>()) {
                letterboxView = calculateLetterboxView(resizeEvt->size);
            }

            if (const auto* textEvt = event->getIf<sf::Event::TextEntered>()) {
                debugOverlay.handleTextEntered(textEvt->unicode, blackboard);
                nodeGraphViewer.handleTextEntered(textEvt->unicode);
            }

            if (nodeGraphViewer.getIsVisible()) {
                nodeGraphViewer.handleEvent(*event, window);
                if (const auto* keyBtn = event->getIf<sf::Event::KeyPressed>()) {
                    if (keyBtn->code == sf::Keyboard::Key::Escape || keyBtn->code == sf::Keyboard::Key::F2) {
                        nodeGraphViewer.toggle();
                    }
                }
                if (const auto* mouseBtn = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseBtn->button == sf::Mouse::Button::Left) {
                        nodeGraphViewer.handleMouseClick(logicalMousePosI, nodeIndexMap, executor, [&]() {
                            syncCurrentNodeState(false, false);
                        });
                    }
                }
                continue;
            }

            if (const auto* keyBtn = event->getIf<sf::Event::KeyPressed>()) {
                if (keyBtn->code == sf::Keyboard::Key::Grave || keyBtn->code == sf::Keyboard::Key::F1) {
                    debugOverlay.toggle();
                }

                if (keyBtn->code == sf::Keyboard::Key::F2) {
                    nodeGraphViewer.toggle();
                }

                if (debugOverlay.getIsVisible()) {
                    if (debugOverlay.handleKeyPressed(keyBtn->code, executor, nodeIndexMap, [&]() {
                        syncCurrentNodeState(false, false);
                    }, blackboard)) {
                        continue;
                    }
                }

                if (keyBtn->code == sf::Keyboard::Key::Escape) {
                    if (debugOverlay.getIsVisible()) {
                        debugOverlay.toggle();
                    } else if (backlogUI.isVisible()) {
                        backlogUI.setVisible(false);
                    } else {
                        settingsUI.toggle();
                    }
                }

                if (keyBtn->code == sf::Keyboard::Key::T) {
                    transitionSystem.start(sceneBuffer, "diamond", 1.2f, 0.15f);
                }

                if (keyBtn->code == sf::Keyboard::Key::F5) {
                    saveLoadUI.open(SaveLoadMode::Save, sf::Vector2u(LOGICAL_WIDTH, LOGICAL_HEIGHT), saveToSlot, loadFromSlot);
                }

                if (keyBtn->code == sf::Keyboard::Key::F9) {
                    saveLoadUI.open(SaveLoadMode::Load, sf::Vector2u(LOGICAL_WIDTH, LOGICAL_HEIGHT), saveToSlot, loadFromSlot);
                }
            }

            if (saveLoadUI.isVisible()) {
                if (const auto* keyBtn = event->getIf<sf::Event::KeyPressed>()) {
                    if (keyBtn->code == sf::Keyboard::Key::Escape) {
                        saveLoadUI.close();
                    }
                }
                if (event->is<sf::Event::MouseMoved>()) {
                    saveLoadUI.updateHover(logicalMousePosF);
                }
                if (const auto* mouseBtn = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseBtn->button == sf::Mouse::Button::Left) {
                        saveLoadUI.handleClick(logicalMousePosF);
                    }
                }
                continue;
            }

            if (debugOverlay.getIsVisible()) {
                if (const auto* mouseBtn = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseBtn->button == sf::Mouse::Button::Left) {
                        debugOverlay.handleMouseClick(logicalMousePosI, executor, blackboard, [&]() {
                            syncCurrentNodeState(false, false);
                        });
                    }
                }
                if (const auto* mouseRel = event->getIf<sf::Event::MouseButtonReleased>()) {
                    if (mouseRel->button == sf::Mouse::Button::Left) {
                        debugOverlay.handleMouseRelease();
                    }
                }
                if (event->is<sf::Event::MouseMoved>()) {
                    debugOverlay.handleMouseMove(logicalMousePosI);
                }
                if (const auto* wheelEvt = event->getIf<sf::Event::MouseWheelScrolled>()) {
                    if (wheelEvt->wheel == sf::Mouse::Wheel::Vertical) {
                        debugOverlay.handleScroll(wheelEvt->delta);
                    }
                }
                continue;
            }

            if (settingsUI.getIsVisible()) {
                if (const auto* mouseBtn = event->getIf<sf::Event::MouseButtonPressed>()) {
                    settingsUI.handleMouseButtonPressed(mouseBtn->button, logicalMousePosI);
                }
                if (const auto* mouseRel = event->getIf<sf::Event::MouseButtonReleased>()) {
                    settingsUI.handleMouseButtonReleased(mouseRel->button);
                }
                if (event->is<sf::Event::MouseMoved>()) {
                    settingsUI.handleMouseMove(logicalMousePosI);
                }
                continue;
            }

            if (titleMenu.isVisible()) {
                if (event->is<sf::Event::MouseMoved>()) {
                    titleMenu.updateHover(logicalMousePosF);
                }
                if (const auto* mouseBtn = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseBtn->button == sf::Mouse::Button::Left) {
                        titleMenu.handleClick(logicalMousePosF);
                    }
                }
                continue;
            }

            if (const auto* wheelEvt = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (wheelEvt->wheel == sf::Mouse::Wheel::Vertical) {
                    if (backlogUI.isVisible()) {
                        backlogUI.handleScroll(wheelEvt->delta);
                    } else {
                        if (wheelEvt->delta > 0.0f) {
                            performRollback();
                        }
                    }
                }
            }

            if (backlogUI.isVisible()) {
                if (const auto* keyBtn = event->getIf<sf::Event::KeyPressed>()) {
                    if (keyBtn->code == sf::Keyboard::Key::Tab || keyBtn->code == sf::Keyboard::Key::H) {
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

            auto currentNode = executor.getCurrentNode();

            if (const auto* keyBtn = event->getIf<sf::Event::KeyPressed>()) {
                if (keyBtn->code == sf::Keyboard::Key::Tab || keyBtn->code == sf::Keyboard::Key::H) {
                    backlogUI.toggle();
                }

                if (keyBtn->code == sf::Keyboard::Key::Backspace) {
                    performRollback();
                }

                if (keyBtn->code == sf::Keyboard::Key::K) {
                    postFX.triggerShake(0.4f, 20.0f);
                }
            }

            if (!currentNode) continue;

            if (currentNode->type == NodeType::Choice) {
                if (event->is<sf::Event::MouseMoved>()) {
                    choiceUI.updateHover(logicalMousePosF);
                }
                if (const auto* mouseBtn = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseBtn->button == sf::Mouse::Button::Left) {
                        int selectedOption = choiceUI.handleMouseClick(logicalMousePosF);
                        if (selectedOption != -1) {
                            if (currentNode->id == "end_choice") {
                                if (selectedOption == 0) {
                                    audioManager.stopBGM();
                                    titleMenu.setVisible(true);
                                    continue;
                                } else if (selectedOption == 1) {
                                    window.close();
                                    continue;
                                }
                            }

                            executor.advance(selectedOption);
                            syncCurrentNodeState(true, true);
                        }
                    }
                }
            } else if (currentNode->type == NodeType::Dialogue) {
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
                        syncCurrentNodeState(true, true);
                    }
                }
            }
        }

        audioManager.update(deltaTime);
        layerRenderer.update(deltaTime);
        weatherSystem.update(deltaTime);
        transitionSystem.update(deltaTime);

        bool shouldBlur = titleMenu.isVisible() || backlogUI.isVisible() || debugOverlay.getIsVisible() || nodeGraphViewer.getIsVisible() || settingsUI.getIsVisible() || saveLoadUI.isVisible();
        postFX.setBlur(shouldBlur ? 3.5f : 0.0f);
        postFX.update(deltaTime, sf::Vector2u(LOGICAL_WIDTH, LOGICAL_HEIGHT));
        dialogueBox.update();

        // 8. 繪製邏輯 1080p 場景至 sceneBuffer
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

        // 9. 透過 TransitionSystem 混色至 compositeBuffer
        compositeBuffer.clear(sf::Color::Black);
        transitionSystem.draw(compositeBuffer, sceneBuffer);
        compositeBuffer.display();

        // 10. 螢幕 Letterbox 投影輸出
        window.clear(sf::Color::Black);
        window.setView(letterboxView);

        // 繪製 PostFX 主場景
        postFX.applyAndDraw(window, compositeBuffer);

        // 11. 頂層 Native Vector UIs 繪製 (維持 1080p 邏輯視圖)
        titleMenu.draw(window);
        saveLoadUI.draw(window);
        backlogUI.draw(window);
        settingsUI.draw(window);
        debugOverlay.draw(window, blackboard, executor, nodeIndexMap, currentFps);
        nodeGraphViewer.draw(window, nodeIndexMap, executor, [&]() {
            syncCurrentNodeState(false, false);
        });

        window.display();
    }

    return 0;
}