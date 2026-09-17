#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <memory>
#include <filesystem>
#include <unordered_map>
#include <functional>
#include <optional>
#include <map>
#include <string>
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <chrono>
#include <ctime>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include "core/FontManager.hpp"
#include "core/ConfigManager.hpp"
#include "core/AssetPack.hpp"
#include "core/LocalizationManager.hpp"
#include "story/ScriptLoader.hpp"
#include "core/Blackboard.hpp"
#include "core/LRUCache.hpp"
#include "story/StoryExecutor.hpp"
#include "render/DialogueBox.hpp"
#include "render/ChoiceUI.hpp"
#include "render/LayerRenderer.hpp"
#include "render/BacklogUI.hpp"
#include "render/WeatherSystem.hpp"
#include "render/PostFX.hpp"
#include "render/TransitionSystem.hpp"
#include "render/UITheme.hpp"
#include "render/TitleMenu.hpp"
#include "render/SettingsUI.hpp"
#include "render/SaveLoadUI.hpp"
#include "core/AudioManager.hpp"
#include "core/SaveManager.hpp"

#ifdef GALITY_DEV_BUILD
#include "core/HotReloader.hpp"
#include "render/DebugOverlay.hpp"
#include "render/NodeGraphViewer.hpp"
#endif

namespace fs = std::filesystem;

// ============================================================================
// 建置模式
// ============================================================================
#ifdef GALITY_DEV_BUILD
constexpr bool IS_DEV_BUILD = true;
constexpr const char* BUILD_NAME = "DEVELOP";
#else
constexpr bool IS_DEV_BUILD = false;
constexpr const char* BUILD_NAME = "PRODUCT";
#endif

// ============================================================================
// 邏輯視窗解析度
// ============================================================================
constexpr unsigned int LOGICAL_WIDTH = 1920;
constexpr unsigned int LOGICAL_HEIGHT = 1080;

// ============================================================================
// Letterbox 視口計算
// ============================================================================
sf::View calculateLetterboxView(sf::Vector2u windowSize) {
    float windowRatio = static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);
    float targetRatio = static_cast<float>(LOGICAL_WIDTH) / static_cast<float>(LOGICAL_HEIGHT);

    sf::View view(sf::FloatRect(
        sf::Vector2f(0.f, 0.f),
        sf::Vector2f(static_cast<float>(LOGICAL_WIDTH), static_cast<float>(LOGICAL_HEIGHT))
    ));

    if (windowRatio >= targetRatio) {
        float viewportWidth = targetRatio / windowRatio;
        float viewportX = (1.0f - viewportWidth) * 0.5f;
        view.setViewport(sf::FloatRect(sf::Vector2f(viewportX, 0.0f),
                                        sf::Vector2f(viewportWidth, 1.0f)));
    } else {
        float viewportHeight = windowRatio / targetRatio;
        float viewportY = (1.0f - viewportHeight) * 0.5f;
        view.setViewport(sf::FloatRect(sf::Vector2f(0.0f, viewportY),
                                        sf::Vector2f(1.0f, viewportHeight)));
    }

    return view;
}

// ============================================================================
// 時間戳記輔助
// ============================================================================
std::string getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
#ifdef _WIN32
    localtime_s(&tm_buf, &timestamp);
#else
    localtime_r(&timestamp, &tm_buf);
#endif
    std::ostringstream ss;
    ss << std::put_time(&tm_buf, "%Y%m%d_%H%M%S");
    return ss.str();
}

// ============================================================================
// GalityApp
// ============================================================================
struct GalityApp {
    // ===== 核心 =====
    sf::RenderWindow window;
    sf::View letterboxView;
    sf::RenderTexture sceneBuffer;
    sf::RenderTexture compositeBuffer;

    Blackboard blackboard;
    LRUCache<std::string, std::shared_ptr<sf::Texture>> textureCache;
    StoryExecutor executor;

    // ===== UI =====
    DialogueBox dialogueBox;
    ChoiceUI choiceUI;
    LayerRenderer layerRenderer;
    AudioManager audioManager;
    BacklogUI backlogUI;
    SettingsUI settingsUI;
    SaveLoadUI saveLoadUI;
    WeatherSystem weatherSystem;
    PostFX postFX;
    TransitionSystem transitionSystem;
    TitleMenu titleMenu;
    UITheme uiTheme;

    // ===== 開發版專屬 =====
#ifdef GALITY_DEV_BUILD
    std::unique_ptr<HotReloader> hotReloader;
    DebugOverlay debugOverlay;
    NodeGraphViewer nodeGraphViewer;
#endif

    // ===== 狀態 =====
    std::unordered_map<std::string, std::shared_ptr<StoryNode>> nodeIndexMap;
    std::shared_ptr<StoryNode> rootNode;

    std::string previousBgPath = "";
    std::string currentPlayingBgm = "";
    std::string currentWeatherStr = "none";
    std::map<CharSlot, std::string> currentSlots;
    std::optional<CharSlot> currentActiveSlot = CharSlot::Center;

    sf::Clock deltaClock;
    float currentFps = 60.0f;
    bool initialized = false;
    bool isRunning = true;

    GalityApp()
        : window()
        , letterboxView()
        , sceneBuffer()
        , compositeBuffer()
        , textureCache(50)
        , executor(blackboard)
        , layerRenderer(textureCache)
        , weatherSystem(sf::Vector2u(LOGICAL_WIDTH, LOGICAL_HEIGHT))
        , transitionSystem(sf::Vector2u(LOGICAL_WIDTH, LOGICAL_HEIGHT))
    {}

    // ========================================================================
    // 初始化
    // ========================================================================
    bool initialize() {
        std::cout << "[Boot] Initializing Gality Engine [" << BUILD_NAME << "]" << std::endl;

        // 1. 視窗
#ifdef __EMSCRIPTEN__
        window.create(sf::VideoMode({1280, 720}), "Gality Engine");
#else
        auto videoMode = sf::VideoMode::getDesktopMode();
        if (ConfigManager::config.windowMode == 1) {
            window.create(sf::VideoMode({1280, 720}), "Gality Engine", sf::Style::Default);
        } else {
            window.create(videoMode, "Gality Engine", sf::State::Fullscreen);
        }
#endif
        window.setFramerateLimit(60);
        window.setVerticalSyncEnabled(ConfigManager::config.enableVsync);

        letterboxView = calculateLetterboxView(window.getSize());

        // 2. RenderTexture
        if (!sceneBuffer.resize(sf::Vector2u(LOGICAL_WIDTH, LOGICAL_HEIGHT))) {
            std::cerr << "[Boot Error] Failed to create sceneBuffer!" << std::endl;
            return false;
        }
        if (!compositeBuffer.resize(sf::Vector2u(LOGICAL_WIDTH, LOGICAL_HEIGHT))) {
            std::cerr << "[Boot Error] Failed to create compositeBuffer!" << std::endl;
            return false;
        }

        // 3. 目錄
#ifndef __EMSCRIPTEN__
        fs::create_directories("saves");
        fs::create_directories("screenshots");
        fs::create_directories("assets/config");
        fs::create_directories("assets/masks");
#endif

        // 4. 配置
        ConfigManager::load();

        // 5. 初始化 LocalizationManager
        LocalizationManager::instance().initialize(
            ConfigManager::config.availableLanguages,
            ConfigManager::config.language
        );

        // 6. 載入劇本
        rootNode = ScriptLoader::loadFromFile("assets/scripts/demo_long.json");
        if (!rootNode) {
            std::cerr << "[Boot Error] Failed to load script!" << std::endl;
#ifdef _WIN32
#ifndef __EMSCRIPTEN__
            MessageBoxW(NULL, L"腳本載入失敗", L"錯誤", MB_ICONERROR | MB_OK);
#endif
#endif
            return false;
        }

        for (const auto& [id, nodePtr] : ScriptLoader::nodeRegistry) {
            nodeIndexMap[id] = nodePtr;
        }

        // 7. 字型
        if (!choiceUI.loadFont("assets/fonts/font.ttf") ||
            !backlogUI.loadFont("assets/fonts/font.ttf") ||
            !titleMenu.loadFont("assets/fonts/font.ttf") ||
            !settingsUI.loadFont("assets/fonts/font.ttf") ||
            !saveLoadUI.loadFont("assets/fonts/font.ttf")) {
            std::cerr << "[Boot Error] Failed to load fonts!" << std::endl;
            return false;
        }

#ifdef GALITY_DEV_BUILD
        if (!debugOverlay.loadFont("assets/fonts/font.ttf") ||
            !nodeGraphViewer.loadFont("assets/fonts/font.ttf")) {
            std::cerr << "[Boot Error] Failed to load debug fonts!" << std::endl;
            return false;
        }
        std::cout << "[Boot] Debug tools loaded (DEVELOP build)" << std::endl;
#endif

        // 8. 打字音效
        dialogueBox.loadTypeSound("assets/audio/typewriter.wav");


        // 9. UI 主題
        uiTheme.loadFromFile("assets/config/ui_theme.json");

        // ⚠️ 初始化 FontManager（DialogueBox 依賴它）
        FontManager::instance().initialize(
            uiTheme.fonts.available,
            uiTheme.fonts.defaultName
        );

        // DialogueBox 套用主題（內部會從 FontManager 取得字型）
        dialogueBox.applyTheme(uiTheme.dialogueStyle, uiTheme.dialogueButtonsStyle);

        // 如果 dialogueFont 不是預設，明確設定
        if (!uiTheme.dialogueStyle.dialogueFont.empty()) {
            dialogueBox.setFont(uiTheme.dialogueStyle.dialogueFont);
        }

        choiceUI.applyTheme(uiTheme.choiceStyle);

        layerRenderer.setDialogueBoxReservedHeight(uiTheme.dialogueStyle.height);
        layerRenderer.setScaleFactors(
            uiTheme.characterScaling.single,
            uiTheme.characterScaling.doubleSlot,
            uiTheme.characterScaling.triple,
            uiTheme.characterScaling.bottomOffset
        );

        // 10. HotReloader
#ifdef GALITY_DEV_BUILD
#ifndef __EMSCRIPTEN__
        hotReloader = std::make_unique<HotReloader>(
            "assets/scripts/demo_long.json",
            "assets/shaders/blur.frag"
        );
#endif
#endif

        // ====================================================================
        // 11. SettingsUI 回呼
        // ====================================================================
        settingsUI.setExitCallback([this]() {
            std::cout << "[Settings] Exiting..." << std::endl;
            isRunning = false;
#ifdef __EMSCRIPTEN__
            emscripten_cancel_main_loop();
#endif
            window.close();
        });

        settingsUI.setReturnToTitleCallback([this]() {
            audioManager.stopBGM();
            audioManager.stopVoice();
            settingsUI.setVisible(false);
            titleMenu.setVisible(true);
        });

        // ⚠️ 語言切換回呼
        settingsUI.onLanguageChanged = [this](const std::string& lang) {
            std::cout << "[Main] Language changed to: " << lang << std::endl;

            // 重新載入劇本（會自動套用新語言）
            rootNode = ScriptLoader::loadFromFile("assets/scripts/demo_long.json");
            if (rootNode) {
                nodeIndexMap.clear();
                for (const auto& [id, nodePtr] : ScriptLoader::nodeRegistry) {
                    nodeIndexMap[id] = nodePtr;
                }

                // 如果遊戲正在進行，重新載入當前節點
                auto currentNode = executor.getCurrentNode();
                if (currentNode) {
                    std::string currentId = currentNode->id;
                    if (nodeIndexMap.count(currentId)) {
                        executor.jumpToNode(nodeIndexMap[currentId]);
                        syncCurrentNodeState(false, false);
                    }
                }
            }
        };

        // ====================================================================
        // 12. TitleMenu BGM 回呼
        // ====================================================================
        titleMenu.setBGMRequestCallback([this](const std::string& path, float volume,
                                                bool loop, float fadeIn) {
            audioManager.playBGM(path, volume, loop, fadeIn);
        });

        titleMenu.setBGMStopCallback([this]() {
            audioManager.stopBGM();
        });

        // ====================================================================
        // 13. DialogueBox 按鈕回呼
        // ====================================================================
        dialogueBox.setBacklogCallback([this]() {
            backlogUI.toggle();
        });

        dialogueBox.setSaveCallback([this]() {
            saveLoadUI.open(SaveLoadMode::Save, sf::Vector2u(LOGICAL_WIDTH, LOGICAL_HEIGHT),
                            [this](int slot) { saveToSlot(slot); },
                            [this](int slot) { loadFromSlot(slot); });
        });

        dialogueBox.setLoadCallback([this]() {
            saveLoadUI.open(SaveLoadMode::Load, sf::Vector2u(LOGICAL_WIDTH, LOGICAL_HEIGHT),
                            [this](int slot) { saveToSlot(slot); },
                            [this](int slot) { loadFromSlot(slot); });
        });

        dialogueBox.setMenuCallback([this]() {
            settingsUI.setVisible(true);
        });

        dialogueBox.setAdvanceCallback([this]() {
            audioManager.stopVoice();
            executor.advance();
            syncCurrentNodeState(true, true);
        });

        // ====================================================================
        // 14. TitleMenu 配置與動作
        // ====================================================================
        titleMenu.loadConfig("assets/config/title_menu.json");

        titleMenu.registerAction("start", [this]() {
            titleMenu.onGameStart();
            titleMenu.setVisible(false);
            blackboard = Blackboard();
            backlogUI.clear();
            dialogueBox.resetModes();
            executor.start(rootNode);
            syncCurrentNodeState(true, false);
        });

        titleMenu.registerAction("load", [this]() {
            saveLoadUI.open(SaveLoadMode::Load, sf::Vector2u(LOGICAL_WIDTH, LOGICAL_HEIGHT),
                            [this](int slot) { saveToSlot(slot); },
                            [this](int slot) { loadFromSlot(slot); });
        });

        titleMenu.registerAction("settings", [this]() {
            settingsUI.setVisible(true);
        });

        titleMenu.registerAction("exit", [this]() {
            isRunning = false;
#ifdef __EMSCRIPTEN__
            emscripten_cancel_main_loop();
#endif
            window.close();
        });

        initialized = true;
        std::cout << "[Boot] Initialization complete" << std::endl;
        return true;
    }

    // ========================================================================
    // 截圖
    // ========================================================================
    void captureScreenshot() {
#ifndef __EMSCRIPTEN__
        try {
            fs::create_directories("screenshots");

            sf::Texture screenshot;
            auto winSize = window.getSize();
            if (!screenshot.resize(winSize)) {
                std::cerr << "[Screenshot] Failed to create texture!" << std::endl;
                return;
            }
            screenshot.update(window);

            std::string filename = "screenshots/" + getTimestamp() + ".png";

            if (screenshot.copyToImage().saveToFile(filename)) {
                fs::path fullPath = fs::absolute(filename);
                std::cout << "[Screenshot] Saved: " << fullPath.string() << std::endl;
            } else {
                std::cerr << "[Screenshot] Failed to save: " << filename << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "[Screenshot] Error: " << e.what() << std::endl;
        }
#endif
    }

    // ========================================================================
    // 同步節點狀態
    // ========================================================================
        // ========================================================================
    // 同步節點狀態
    // ========================================================================
    void syncCurrentNodeState(bool recordHistory = true, bool playTransition = true) {
        auto currentNode = executor.getCurrentNode();
        if (!currentNode) return;

        // Skip 停止條件
        if (dialogueBox.isSkipHeld()) {
            if (currentNode->type == NodeType::Choice) {
                dialogueBox.stopSkip();
                std::cout << "[Skip] Stopped at choice: " << currentNode->id << std::endl;
            }

            if (currentNode->id.find("node_ending_") == 0 ||
                currentNode->id.find("node_end_") == 0) {
                dialogueBox.stopSkip();
                std::cout << "[Skip] Stopped at ending: " << currentNode->id << std::endl;
            }
        }

        if (currentNode->type == NodeType::Dialogue) {
            dialogueBox.setText(currentNode->speaker, currentNode->text);

            // 設定強制觀看
            dialogueBox.setForcedRead(
                currentNode->noSkip,
                currentNode->noSkipWait
            );

            if (recordHistory) {
                backlogUI.addEntry(currentNode->speaker, currentNode->text,
                                    currentNode->voicePath);
            }

            // ================================================================
            // ⚠️ 轉場觸發邏輯（支援同背景轉場）
            // ================================================================
            // 判斷是否要觸發轉場：
            //   1. 背景有改變（bgChanged）
            //   2. 創作者明確指定了 trans（hasExplicitTrans）
            // ================================================================
            bool bgChanged = (!currentNode->bgImagePath.empty() &&
                              currentNode->bgImagePath != previousBgPath);
            bool hasExplicitTrans = !currentNode->transitionMask.empty();
            bool shouldTransition = bgChanged || hasExplicitTrans;

            if (shouldTransition) {
                // 只有在「已經有舊畫面」且「轉場功能開啟」時，才觸發轉場
                if (!previousBgPath.empty() &&
                    playTransition &&
                    ConfigManager::config.enableTransitions) {

                    // 快照當前畫面
                    sceneBuffer.clear(sf::Color(20, 20, 30));
                    layerRenderer.draw(sceneBuffer);
                    sceneBuffer.display();

                    // 決定遮罩名稱與時長
                    std::string maskName = currentNode->transitionMask.empty()
                        ? "diamond" : currentNode->transitionMask;
                    float dur = (currentNode->transitionDuration > 0.0f)
                        ? currentNode->transitionDuration : 1.0f;

                    // 觸發轉場
                    transitionSystem.start(sceneBuffer, maskName, dur, 0.15f);

                    std::cout << "[Transition] Triggered: mask=" << maskName
                              << ", duration=" << dur
                              << ", bgChanged=" << bgChanged
                              << ", explicit=" << hasExplicitTrans
                              << std::endl;
                }

                // ⚠️ 只有背景真的改變時，才更新背景
                if (bgChanged) {
                    layerRenderer.setBackground(currentNode->bgImagePath,
                                                 sf::Vector2u(LOGICAL_WIDTH, LOGICAL_HEIGHT));
                    previousBgPath = currentNode->bgImagePath;
                }
            } else if (currentNode->bgImagePath.empty()) {
                layerRenderer.setBackground("");
                previousBgPath = "";
            }

            // ================================================================
            // 角色立繪更新
            // ================================================================
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

            // ================================================================
            // BGM 與語音
            // ================================================================
            if (!currentNode->bgmPath.empty()) {
                currentPlayingBgm = currentNode->bgmPath;
                audioManager.playBGM(currentNode->bgmPath);
            }
            if (!currentNode->voicePath.empty()) {
                audioManager.playVoice(currentNode->voicePath, currentActiveSlot);
            }

            // ================================================================
            // 天氣系統
            // ================================================================
            currentWeatherStr = currentNode->weather;
            if (!currentNode->weather.empty()) {
                if (currentNode->weather == "rain")
                    weatherSystem.setWeather(WeatherType::Rain);
                else if (currentNode->weather == "snow")
                    weatherSystem.setWeather(WeatherType::Snow);
                else if (currentNode->weather == "sakura")
                    weatherSystem.setWeather(WeatherType::Sakura);
                else if (currentNode->weather == "none")
                    weatherSystem.setWeather(WeatherType::None);
            }

            // ================================================================
            // 震動效果
            // ================================================================
            if (currentNode->shake > 0.0f) {
                postFX.triggerShake(currentNode->shake, 20.0f);
            }

            // ================================================================
            // 更新演出狀態（供 Rollback 使用）
            // ================================================================
            executor.updatePresentationState(previousBgPath, currentSlots,
                                             currentActiveSlot,
                                             currentWeatherStr, currentPlayingBgm);

            // 如果處於隱藏模式，恢復顯示
            if (dialogueBox.isHiddenMode()) {
                dialogueBox.setHiddenMode(false);
            }

        } else if (currentNode->type == NodeType::Choice) {
            choiceUI.setChoices(currentNode->choices);
        }
    }

    // ========================================================================
    // Rollback
    // ========================================================================
    void performRollback() {
        SaveSnapshot snapshot;
        if (executor.rollback(snapshot)) {
            if (nodeIndexMap.count(snapshot.currentNodeId)) {
                audioManager.stopVoice();
                executor.jumpToNode(nodeIndexMap[snapshot.currentNodeId]);

                previousBgPath = snapshot.bgImagePath;
                layerRenderer.setBackground(snapshot.bgImagePath,
                                             sf::Vector2u(LOGICAL_WIDTH, LOGICAL_HEIGHT));
                currentSlots = snapshot.slotTextures;
                currentActiveSlot = snapshot.activeSlot;
                layerRenderer.updateCharacters(currentSlots, currentActiveSlot);

                currentWeatherStr = snapshot.weather;
                if (currentWeatherStr == "rain")
                    weatherSystem.setWeather(WeatherType::Rain);
                else if (currentWeatherStr == "snow")
                    weatherSystem.setWeather(WeatherType::Snow);
                else if (currentWeatherStr == "sakura")
                    weatherSystem.setWeather(WeatherType::Sakura);
                else
                    weatherSystem.setWeather(WeatherType::None);

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

                executor.updatePresentationState(previousBgPath, currentSlots,
                                                 currentActiveSlot,
                                                 currentWeatherStr, currentPlayingBgm);
            }
        }
    }

    // ========================================================================
    // 存讀檔
    // ========================================================================
    void saveToSlot(int slotIndex) {
#ifndef __EMSCRIPTEN__
        std::string filePath = "saves/save_slot_" + std::to_string(slotIndex) + ".json";
        if (!executor.getCurrentNode()) return;

        SaveSnapshot snapshot = executor.getCurrentPresentationState();
        snapshot.currentNodeId = executor.getCurrentNode()->id;
        snapshot.intFlags = blackboard.getAllInts();
        SaveManager::saveGame(filePath, snapshot);
        std::cout << "[Save] Slot " << slotIndex << std::endl;
#endif
    }

    void loadFromSlot(int slotIndex) {
#ifndef __EMSCRIPTEN__
        std::string filePath = "saves/save_slot_" + std::to_string(slotIndex) + ".json";
        SaveSnapshot snapshot;
        if (!SaveManager::loadGame(filePath, snapshot)) return;
        if (!nodeIndexMap.count(snapshot.currentNodeId)) return;

        blackboard.setAllInts(snapshot.intFlags);
        executor.jumpToNode(nodeIndexMap[snapshot.currentNodeId]);
        titleMenu.setVisible(false);
        dialogueBox.resetModes();

        previousBgPath = snapshot.bgImagePath;
        layerRenderer.setBackground(snapshot.bgImagePath,
                                     sf::Vector2u(LOGICAL_WIDTH, LOGICAL_HEIGHT));
        currentSlots = snapshot.slotTextures;
        currentActiveSlot = snapshot.activeSlot;
        layerRenderer.updateCharacters(currentSlots, currentActiveSlot);

        currentWeatherStr = snapshot.weather;
        if (currentWeatherStr == "rain")
            weatherSystem.setWeather(WeatherType::Rain);
        else if (currentWeatherStr == "snow")
            weatherSystem.setWeather(WeatherType::Snow);
        else if (currentWeatherStr == "sakura")
            weatherSystem.setWeather(WeatherType::Sakura);
        else
            weatherSystem.setWeather(WeatherType::None);

        if (!snapshot.bgmPath.empty()) {
            currentPlayingBgm = snapshot.bgmPath;
            audioManager.playBGM(snapshot.bgmPath);
        }

        syncCurrentNodeState(false, false);
        auto loadedNode = executor.getCurrentNode();
        if (loadedNode && !loadedNode->voicePath.empty()) {
            audioManager.playVoice(loadedNode->voicePath, currentActiveSlot);
        }
        std::cout << "[Load] Slot " << slotIndex << std::endl;
#endif
    }

    // ========================================================================
    // 事件處理
    // ========================================================================
    void handleEvents() {
        while (const auto event = window.pollEvent()) {
            // ===== 視窗關閉 =====
            if (event->is<sf::Event::Closed>()) {
                isRunning = false;
#ifdef __EMSCRIPTEN__
                emscripten_cancel_main_loop();
#endif
                window.close();
                return;
            }

            // ===== Resize =====
            if (const auto* resizeEvt = event->getIf<sf::Event::Resized>()) {
                letterboxView = calculateLetterboxView(resizeEvt->size);
                settingsUI.onResize(resizeEvt->size);
            }

            // 邏輯滑鼠座標
            sf::Vector2i pixelMousePos = sf::Mouse::getPosition(window);
            sf::Vector2f logicalMousePosF = window.mapPixelToCoords(pixelMousePos, letterboxView);
            sf::Vector2i logicalMousePosI(
                static_cast<int>(logicalMousePosF.x),
                static_cast<int>(logicalMousePosF.y)
            );

            // ===== 全域滑鼠放開事件 =====
            if (const auto* mouseRel = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (mouseRel->button == sf::Mouse::Button::Left) {
                    dialogueBox.handleButtonRelease();
                }
            }

            // ===== 全域快捷鍵 =====
            if (const auto* keyBtn = event->getIf<sf::Event::KeyPressed>()) {
                if (keyBtn->code == sf::Keyboard::Key::F12) {
                    captureScreenshot();
                    continue;
                }

#ifdef GALITY_DEV_BUILD
                if (keyBtn->code == sf::Keyboard::Key::F1 ||
                    keyBtn->code == sf::Keyboard::Key::Grave) {
                    debugOverlay.toggle();
                    continue;
                }

                if (keyBtn->code == sf::Keyboard::Key::F2) {
                    if (debugOverlay.getIsVisible()) {
                        debugOverlay.toggle();
                    }
                    nodeGraphViewer.toggle();
                    continue;
                }
#endif

                if (keyBtn->code == sf::Keyboard::Key::F5) {
                    saveLoadUI.open(SaveLoadMode::Save, sf::Vector2u(LOGICAL_WIDTH, LOGICAL_HEIGHT),
                                    [this](int slot) { saveToSlot(slot); },
                                    [this](int slot) { loadFromSlot(slot); });
                    continue;
                }
                if (keyBtn->code == sf::Keyboard::Key::F9) {
                    saveLoadUI.open(SaveLoadMode::Load, sf::Vector2u(LOGICAL_WIDTH, LOGICAL_HEIGHT),
                                    [this](int slot) { saveToSlot(slot); },
                                    [this](int slot) { loadFromSlot(slot); });
                    continue;
                }
            }

            // =================================================================
            // SettingsUI
            // =================================================================
            if (settingsUI.getIsVisible()) {
                if (const auto* keyBtn = event->getIf<sf::Event::KeyPressed>()) {
                    if (keyBtn->code == sf::Keyboard::Key::Escape) {
                        if (settingsUI.getConfirmVisible()) {
                            settingsUI.closeConfirm();
                        } else {
                            settingsUI.setVisible(false);
                        }
                        continue;
                    }
                }
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

            // =================================================================
            // SaveLoadUI
            // =================================================================
            if (saveLoadUI.isVisible()) {
                if (const auto* keyBtn = event->getIf<sf::Event::KeyPressed>()) {
                    if (keyBtn->code == sf::Keyboard::Key::Escape) {
                        saveLoadUI.close();
                        continue;
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

#ifdef GALITY_DEV_BUILD
            // =================================================================
            // NodeGraphViewer
            // =================================================================
            if (nodeGraphViewer.getIsVisible()) {
                nodeGraphViewer.handleEvent(*event, window);
                if (const auto* keyBtn = event->getIf<sf::Event::KeyPressed>()) {
                    if (keyBtn->code == sf::Keyboard::Key::Escape) {
                        nodeGraphViewer.toggle();
                    }
                }
                if (const auto* mouseBtn = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseBtn->button == sf::Mouse::Button::Left) {
                        nodeGraphViewer.handleMouseClick(logicalMousePosI, nodeIndexMap, executor, [this]() {
                            syncCurrentNodeState(false, false);
                        });
                    }
                }
                continue;
            }

            // =================================================================
            // DebugOverlay
            // =================================================================
            if (debugOverlay.getIsVisible()) {
                if (const auto* keyBtn = event->getIf<sf::Event::KeyPressed>()) {
                    if (keyBtn->code == sf::Keyboard::Key::Escape) {
                        debugOverlay.toggle();
                        continue;
                    }
                }
                if (const auto* mouseBtn = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseBtn->button == sf::Mouse::Button::Left) {
                        debugOverlay.handleMouseClick(logicalMousePosI, executor, blackboard, [this]() {
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
                if (const auto* textEvt = event->getIf<sf::Event::TextEntered>()) {
                    debugOverlay.handleTextEntered(textEvt->unicode, blackboard);
                }
                if (const auto* keyBtn = event->getIf<sf::Event::KeyPressed>()) {
                    if (debugOverlay.handleKeyPressed(keyBtn->code, executor, nodeIndexMap, [this]() {
                        syncCurrentNodeState(false, false);
                    }, blackboard)) {
                        continue;
                    }
                }
                continue;
            }
#endif

            // =================================================================
            // TitleMenu
            // =================================================================
            if (titleMenu.isVisible()) {
                if (const auto* keyBtn = event->getIf<sf::Event::KeyPressed>()) {
                    if (keyBtn->code == sf::Keyboard::Key::Escape) {
                        settingsUI.setVisible(true);
                        continue;
                    }
                }
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

            // =================================================================
            // BacklogUI
            // =================================================================
            if (backlogUI.isVisible()) {
                if (const auto* keyBtn = event->getIf<sf::Event::KeyPressed>()) {
                    if (keyBtn->code == sf::Keyboard::Key::Escape ||
                        keyBtn->code == sf::Keyboard::Key::Tab ||
                        keyBtn->code == sf::Keyboard::Key::H) {
                        backlogUI.setVisible(false);
                    }
                    if (keyBtn->code == sf::Keyboard::Key::Up) backlogUI.handleScroll(1.0f);
                    if (keyBtn->code == sf::Keyboard::Key::Down) backlogUI.handleScroll(-1.0f);
                }
                if (const auto* wheelEvt = event->getIf<sf::Event::MouseWheelScrolled>()) {
                    if (wheelEvt->wheel == sf::Mouse::Wheel::Vertical) {
                        backlogUI.handleScroll(wheelEvt->delta);
                    }
                }
                if (const auto* mouseBtn = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseBtn->button == sf::Mouse::Button::Right) {
                        backlogUI.setVisible(false);
                    }
                }
                continue;
            }

            // =================================================================
            // 遊戲主體
            // =================================================================
            auto currentNode = executor.getCurrentNode();

            // 隱藏模式下，任意輸入恢復
            if (dialogueBox.isHiddenMode()) {
                bool restore = false;
                if (const auto* mouseBtn = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseBtn->button == sf::Mouse::Button::Left) restore = true;
                }
                if (const auto* keyBtn = event->getIf<sf::Event::KeyPressed>()) {
                    if (keyBtn->code == sf::Keyboard::Key::Escape ||
                        keyBtn->code == sf::Keyboard::Key::Enter ||
                        keyBtn->code == sf::Keyboard::Key::Space) {
                        restore = true;
                    }
                }
                if (restore) {
                    dialogueBox.setHiddenMode(false);
                }
                continue;
            }

            if (const auto* keyBtn = event->getIf<sf::Event::KeyPressed>()) {
                if (keyBtn->code == sf::Keyboard::Key::Escape) {
                    settingsUI.setVisible(true);
                    continue;
                }
                if (keyBtn->code == sf::Keyboard::Key::Tab ||
                    keyBtn->code == sf::Keyboard::Key::H) {
                    backlogUI.toggle();
                }
                if (keyBtn->code == sf::Keyboard::Key::Backspace) {
                    performRollback();
                }
#ifdef GALITY_DEV_BUILD
                if (keyBtn->code == sf::Keyboard::Key::K) {
                    if (ConfigManager::config.enablePostFX) {
                        postFX.triggerShake(0.4f, 20.0f);
                    }
                }
#endif
            }

            // 滾輪
            if (const auto* wheelEvt = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (wheelEvt->wheel == sf::Mouse::Wheel::Vertical) {
                    if (wheelEvt->delta > 0.0f) {
                        performRollback();
                    }
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
                                    isRunning = false;
#ifdef __EMSCRIPTEN__
                                    emscripten_cancel_main_loop();
#endif
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
                if (event->is<sf::Event::MouseMoved>()) {
                    dialogueBox.updateButtonHover(logicalMousePosF);
                }

                if (const auto* mouseBtn = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseBtn->button == sf::Mouse::Button::Left) {
                        if (dialogueBox.handleButtonPress(logicalMousePosF)) {
                            continue;
                        }
                    }
                }

                if (dialogueBox.isSkipHeld()) {
                    continue;
                }

                bool triggerAdvance = false;
                if (const auto* keyBtn = event->getIf<sf::Event::KeyPressed>()) {
                    if (keyBtn->code == sf::Keyboard::Key::Enter ||
                        keyBtn->code == sf::Keyboard::Key::Space) {
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
    }

    // ========================================================================
    // 更新
    // ========================================================================
    void update(float deltaTime) {
        if (deltaTime > 0.0f) {
            currentFps = 0.9f * currentFps + 0.1f * (1.0f / deltaTime);
        }

#ifdef GALITY_DEV_BUILD
#ifndef __EMSCRIPTEN__
        if (hotReloader) {
            hotReloader->update(deltaTime,
                [this]() {
                    auto newRoot = ScriptLoader::loadFromFile("assets/scripts/demo_long.json");
                    if (newRoot) {
                        nodeIndexMap.clear();
                        for (const auto& [id, nodePtr] : ScriptLoader::nodeRegistry) {
                            nodeIndexMap[id] = nodePtr;
                        }
                        debugOverlay.log("[HotReloader] Script AST reloaded.");
                    }
                },
                [this]() {
                    postFX = PostFX();
                    debugOverlay.log("[HotReloader] Shaders reloaded.");
                }
            );
        }
#endif
#endif

        titleMenu.update(deltaTime);
        audioManager.update(deltaTime);
        layerRenderer.update(deltaTime);
        weatherSystem.update(deltaTime);
        transitionSystem.update(deltaTime);

        bool shouldBlur = titleMenu.isVisible() || backlogUI.isVisible() ||
                          settingsUI.getIsVisible() || saveLoadUI.isVisible();
#ifdef GALITY_DEV_BUILD
        shouldBlur = shouldBlur || debugOverlay.getIsVisible() ||
                     nodeGraphViewer.getIsVisible();
#endif

        if (!ConfigManager::config.enablePostFX) {
            postFX.setBlur(0.0f);
        } else {
            postFX.setBlur(shouldBlur ? 3.5f : 0.0f);
        }
        postFX.update(deltaTime, sf::Vector2u(LOGICAL_WIDTH, LOGICAL_HEIGHT));
        dialogueBox.update();

        static bool lastVsync = ConfigManager::config.enableVsync;
        if (lastVsync != ConfigManager::config.enableVsync) {
            window.setVerticalSyncEnabled(ConfigManager::config.enableVsync);
            lastVsync = ConfigManager::config.enableVsync;
        }
    }

    // ========================================================================
    // 繪製
    // ========================================================================
    void render() {
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

        compositeBuffer.clear(sf::Color::Black);
        transitionSystem.draw(compositeBuffer, sceneBuffer);
        compositeBuffer.display();

        window.clear(sf::Color::Black);
        window.setView(letterboxView);

        postFX.applyAndDraw(window, compositeBuffer);

        titleMenu.draw(window);
        saveLoadUI.draw(window);
        backlogUI.draw(window);
        settingsUI.draw(window);

#ifdef GALITY_DEV_BUILD
        debugOverlay.draw(window, blackboard, executor, nodeIndexMap, currentFps);
        nodeGraphViewer.draw(window, nodeIndexMap, executor, [this]() {
            syncCurrentNodeState(false, false);
        });
#endif

        window.display();
    }

    // ========================================================================
    // 單幀
    // ========================================================================
    void tick() {
        if (!isRunning || !window.isOpen()) {
#ifdef __EMSCRIPTEN__
            emscripten_cancel_main_loop();
#endif
            return;
        }

        sf::Time dt = deltaClock.restart();
        float deltaTime = dt.asSeconds();
        if (deltaTime <= 0.0f) deltaTime = 1.0f / 60.0f;

        handleEvents();
        update(deltaTime);
        render();
    }

    // ========================================================================
    // 主迴圈
    // ========================================================================
    void run() {
        if (!initialized) return;

#ifdef __EMSCRIPTEN__
        emscripten_set_main_loop_arg([](void* arg) {
            auto* app = static_cast<GalityApp*>(arg);
            app->tick();
        }, this, 0, 1);
#else
        while (window.isOpen() && isRunning) {
            tick();
        }
#endif
    }
};

// ============================================================================
// 主程式入口
// ============================================================================
int main() {
    GalityApp app;

    if (!app.initialize()) {
        std::cerr << "[Fatal] Initialization failed" << std::endl;
        return -1;
    }

    app.run();

    return 0;
}