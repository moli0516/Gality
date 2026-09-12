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