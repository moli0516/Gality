#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>

#include "core/Blackboard.hpp"
#include "story/StoryExecutor.hpp"
#include "story/ScriptLoader.hpp"
#include "render/DialogueBox.hpp"
#include "render/ChoiceUI.hpp"
#include "render/LayerRenderer.hpp"

int main() {
    sf::RenderWindow window(sf::VideoMode({1280, 720}), "Gality Galgame Engine MVP");
    window.setFramerateLimit(60);

    Blackboard blackboard;
    auto rootNode = ScriptLoader::loadFromFile("assets/scripts/demo.json");
    if (!rootNode) return -1;

    StoryExecutor executor(blackboard);
    executor.start(rootNode);

    // 初始化渲染組件
    DialogueBox dialogueBox;
    ChoiceUI choiceUI;
    LayerRenderer layerRenderer;

    if (!dialogueBox.loadFont("assets/fonts/font.ttf") || !choiceUI.loadFont("assets/fonts/font.ttf")) {
        return -1;
    }

    // 當節點切換時同步對話內容與圖層圖片
    auto syncCurrentNodeState = [&]() {
        auto currentNode = executor.getCurrentNode();
        if (!currentNode) return;

        if (currentNode->type == NodeType::Dialogue) {
            dialogueBox.setText(currentNode->speaker, currentNode->text);
            
            // 更新背景與立繪圖層
            if (!currentNode->bgImagePath.empty()) {
                layerRenderer.setBackground(currentNode->bgImagePath);
            }
            if (!currentNode->characterSpritePath.empty()) {
                layerRenderer.setCharacter(currentNode->characterSpritePath);
            }
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
            if (!currentNode) continue;

            if (currentNode->type == NodeType::Choice) {
                if (event->is<sf::Event::MouseMoved>()) {
                    choiceUI.updateHover(mousePos);
                }
                if (event->is<sf::Event::MouseButtonPressed>()) {
                    const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>();
                    if (mouseEvent && mouseEvent->button == sf::Mouse::Button::Left) {
                        int selectedOption = choiceUI.handleMouseClick(mousePos);
                        if (selectedOption != -1) {
                            executor.advance(selectedOption);
                            syncCurrentNodeState();
                        }
                    }
                }
            } 
            else if (currentNode->type == NodeType::Dialogue) {
                bool triggerAdvance = false;
                if (event->is<sf::Event::KeyPressed>()) {
                    const auto* keyEvent = event->getIf<sf::Event::KeyPressed>();
                    if (keyEvent && (keyEvent->code == sf::Keyboard::Key::Enter || keyEvent->code == sf::Keyboard::Key::Space)) {
                        triggerAdvance = true;
                    }
                }
                if (event->is<sf::Event::MouseButtonPressed>()) {
                    const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>();
                    if (mouseEvent && mouseEvent->button == sf::Mouse::Button::Left) {
                        triggerAdvance = true;
                    }
                }

                if (triggerAdvance) {
                    if (dialogueBox.onInteract()) {
                        executor.advance();
                        syncCurrentNodeState();
                    }
                }
            }
        }

        dialogueBox.update();

        window.clear(sf::Color(20, 20, 30));

        // 渲染堆疊順序 (Render Stack Order)
        // 1. 最底層：背景與角色立繪
        layerRenderer.draw(window);

        // 2. 上層：對話框與 UI
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