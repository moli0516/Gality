#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "../story/StoryNode.hpp"
#include "../core/AssetPack.hpp"
#include "UITheme.hpp"

struct OptionButton {
    sf::RectangleShape shape;
    sf::Text text;
    size_t index = 0;
    bool isHovered = false;

    explicit OptionButton(const sf::Font& font) : text(font) {}
};

class ChoiceUI {
private:
    std::vector<OptionButton> buttons;
    sf::Font font;
    std::vector<std::uint8_t> fontDataBuffer;
    ChoiceUIStyle style;

    // 移除 <...> 標籤
    static std::string stripTags(const std::string& input) {
        std::string result;
        bool inTag = false;
        for (char c : input) {
            if (c == '<') {
                inTag = true;
            } else if (c == '>') {
                inTag = false;
            } else if (!inTag) {
                result += c;
            }
        }
        return result;
    }

public:
    ChoiceUI() = default;

    bool loadFont(const std::string& fontPath) {
        fontDataBuffer.clear();
        if (AssetPack::readFileFromPak(fontPath, fontDataBuffer, "data.pak") && !fontDataBuffer.empty()) {
            return font.openFromMemory(fontDataBuffer.data(), fontDataBuffer.size());
        }
        return font.openFromFile(fontPath);
    }

    void applyTheme(const ChoiceUIStyle& s) {
        style = s;
    }

    void setChoices(const std::vector<ChoiceOption>& options) {
        buttons.clear();

        for (size_t i = 0; i < options.size(); ++i) {
            OptionButton btn(font);

            float posX = (1920.f - style.width) * 0.5f;
            float posY = style.startY + static_cast<float>(i) * (style.height + style.spacing);

            btn.shape.setSize(sf::Vector2f(style.width, style.height));
            btn.shape.setPosition(sf::Vector2f(posX, posY));
            btn.shape.setFillColor(style.normalBgColor);
            btn.shape.setOutlineThickness(2.f);
            btn.shape.setOutlineColor(style.normalOutlineColor);

            btn.text.setFont(font);
            btn.text.setCharacterSize(style.fontSize);
            btn.text.setFillColor(style.textColor);

            // 移除標籤
            std::string cleanText = stripTags(options[i].text);
            btn.text.setString(sf::String::fromUtf8(cleanText.begin(), cleanText.end()));

            sf::FloatRect textBounds = btn.text.getLocalBounds();
            btn.text.setPosition(sf::Vector2f(
                posX + (style.width - textBounds.size.x) * 0.5f,
                posY + (style.height - textBounds.size.y) * 0.5f - 6.f
            ));

            btn.index = i;
            buttons.push_back(std::move(btn));
        }
    }

    void updateHover(sf::Vector2f mousePosF) {
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

    int handleMouseClick(sf::Vector2f mousePosF) {
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