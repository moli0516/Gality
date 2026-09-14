#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "../story/StoryNode.hpp"
#include "UITheme.hpp"

struct OptionButton {
    sf::RectangleShape shape;
    sf::Text text;
    size_t index;
    bool isHovered = false;

    OptionButton(const sf::Font& font) : text(font) {}
};

class ChoiceUI {
private:
    std::vector<OptionButton> buttons;
    sf::Font font;
    ChoiceUIStyle style;

public:
    ChoiceUI() = default;

    bool loadFont(const std::string& fontPath) {
        return font.openFromFile(fontPath);
    }

    void applyTheme(const ChoiceUIStyle& s) {
        style = s;
    }

    void setChoices(const std::vector<ChoiceOption>& options) {
        buttons.clear();

        for (size_t i = 0; i < options.size(); ++i) {
            OptionButton btn(font);
            
            float posX = (1280.f - style.width) / 2.f;
            float posY = style.startY + i * (style.height + style.spacing);

            btn.shape.setSize(sf::Vector2f(style.width, style.height));
            btn.shape.setPosition(sf::Vector2f(posX, posY));
            btn.shape.setFillColor(style.normalBgColor);
            btn.shape.setOutlineThickness(2.f);
            btn.shape.setOutlineColor(style.normalOutlineColor);

            btn.text.setCharacterSize(style.fontSize);
            btn.text.setFillColor(style.textColor);
            btn.text.setString(sf::String::fromUtf8(options[i].text.begin(), options[i].text.end()));
            
            sf::FloatRect textBounds = btn.text.getLocalBounds();
            btn.text.setPosition(sf::Vector2f(
                posX + (style.width - textBounds.size.x) / 2.f,
                posY + (style.height - textBounds.size.y) / 2.f - 5.f
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
                btn.shape.setFillColor(style.hoverBgColor);
                btn.shape.setOutlineColor(style.hoverOutlineColor);
            } else {
                btn.isHovered = false;
                btn.shape.setFillColor(style.normalBgColor);
                btn.shape.setOutlineColor(style.normalOutlineColor);
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

    void draw(sf::RenderTarget& target) {
        for (auto& btn : buttons) {
            target.draw(btn.shape);
            target.draw(btn.text);
        }
    }
};