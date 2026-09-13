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
#include "core/AudioManager.hpp"
#include "core/SaveManager.hpp" // 💾 引入 SaveManager

namespace fs = std::filesystem;

int main() {
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(1280, 720)), "Gality Galgame Engine MVP");
    window.setFramerateLimit(60);

    // 確保 saves 資料夾存在
    fs::create_directories("saves");

    Blackboard blackboard;
    auto rootNode = ScriptLoader::loadFromFile("assets/scripts/demo_long.json");
    if (!rootNode) {
        std::cerr << "Failed to load script!" << std::endl;
        return -1;
    }

    // 建立 Node ID 索引表，方便讀檔時快速跳轉
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

    DialogueBox dialogueBox;
    ChoiceUI choiceUI;
    LayerRenderer layerRenderer;
    AudioManager audioManager;

    if (!dialogueBox.loadFont("assets/fonts/font.ttf") || !choiceUI.loadFont("assets/fonts/font.ttf")) {
        std::cerr << "Failed to load font." << std::endl;
        return -1;
    }

    auto syncCurrentNodeState = [&]() {
        auto currentNode = executor.getCurrentNode();
        if (!currentNode) return;

        if (currentNode->type == NodeType::Dialogue) {
            dialogueBox.setText(currentNode->speaker, currentNode->text);
            
            if (!currentNode->bgImagePath.empty()) {
                layerRenderer.setBackground(currentNode->bgImagePath);
            } else {
                layerRenderer.setBackground("");
            }

            if (!currentNode->characterSpritePath.empty()) {
                layerRenderer.setCharacter(currentNode->characterSpritePath);
            } else {
                layerRenderer.setCharacter("");
            }
            
            if (!currentNode->bgmPath.empty()) {
                audioManager.playBGM(currentNode->bgmPath);
            }
            audioManager.playVoice(currentNode->voicePath);
            
        } else if (currentNode->type == NodeType::Choice) {
            choiceUI.setChoices(currentNode->choices);
        }
    };

    syncCurrentNodeState();

    while (window.isOpen()) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);

        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            auto currentNode = executor.getCurrentNode();

            // 全局快捷鍵監聽 (S: 存檔 / L: 讀檔)
            if (const auto* keyBtn = event->getIf<sf::Event::KeyPressed>()) {
                // 💾 按 S 鍵快速存檔
                if (keyBtn->code == sf::Keyboard::Key::S && currentNode) {
                    SaveManager::saveGame("saves/save1.json", currentNode->id, blackboard);
                }
                // 📂 按 L 鍵快速讀檔
                if (keyBtn->code == sf::Keyboard::Key::L) {
                    SaveSnapshot snapshot;
                    if (SaveManager::loadGame("saves/save1.json", snapshot)) {
                        if (nodeIndexMap.count(snapshot.currentNodeId)) {
                            blackboard.setAllInts(snapshot.intFlags);
                            executor.jumpToNode(nodeIndexMap[snapshot.currentNodeId]);
                            syncCurrentNodeState();
                            std::cout << "[Engine] State restored to node: " << snapshot.currentNodeId << std::endl;
                        }
                    }
                }
            }

            if (!currentNode) continue;

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
                                    executor.start(rootNode);
                                    syncCurrentNodeState();
                                    continue;
                                } else if (selectedOption == 1) {
                                    window.close();
                                    continue;
                                }
                            }

                            executor.advance(selectedOption);
                            syncCurrentNodeState();
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
                        syncCurrentNodeState();
                    }
                }
            }
        }

        dialogueBox.update();

        window.clear(sf::Color(20, 20, 30));
        layerRenderer.draw(window);

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

        window.display();
    }

    return 0;
}