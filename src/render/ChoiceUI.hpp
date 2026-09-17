#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <regex>
#include <iostream>
#include <memory>
#include "../story/StoryNode.hpp"
#include "../core/AssetPack.hpp"
#include "UITheme.hpp"
#include "NineSliceSprite.hpp"

struct OptionButton {
    sf::RectangleShape shape;
    sf::Text text;
    size_t index = 0;
    bool isHovered = false;

    // ⚠️ 九宮格
    NineSliceSprite normalNineSlice;
    NineSliceSprite hoverNineSlice;

    explicit OptionButton(const sf::Font& font) : text(font) {}
};

class ChoiceUI {
private:
    std::vector<OptionButton> buttons;
    sf::Font font;
    std::vector<std::uint8_t> fontDataBuffer;
    ChoiceUIStyle style;

    // ⚠️ 九宮格紋理
    std::shared_ptr<sf::Texture> normalTexture;
    std::shared_ptr<sf::Texture> hoverTexture;

    static std::string stripTags(const std::string& input) {
        static const std::regex tagRegex(R"(<[^>]*>)");
        return std::regex_replace(input, tagRegex, "");
    }

    std::shared_ptr<sf::Texture> loadTexture(const std::string& path) {
        auto tex = std::make_shared<sf::Texture>();
        std::vector<std::uint8_t> bytes;
        if (AssetPack::readFileFromPak(path, bytes, "data.pak") && !bytes.empty()) {
            if (tex->loadFromMemory(bytes.data(), bytes.size())) {
                tex->setSmooth(true);
                return tex;
            }
        }
        if (tex->loadFromFile(path)) {
            tex->setSmooth(true);
            return tex;
        }
        std::cerr << "[ChoiceUI] Failed to load texture: " << path << std::endl;
        return nullptr;
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

        // 載入九宮格紋理
        if (style.normalImage.enabled) {
            normalTexture = loadTexture(style.normalImage.texturePath);
            if (!normalTexture) style.normalImage.enabled = false;
        }
        if (style.hoverImage.enabled) {
            hoverTexture = loadTexture(style.hoverImage.texturePath);
            if (!hoverTexture) style.hoverImage.enabled = false;
        }
    }

    void setChoices(const std::vector<ChoiceOption>& options) {
        buttons.clear();

        for (size_t i = 0; i < options.size(); ++i) {
            OptionButton btn(font);

            float posX = (1920.f - style.width) * 0.5f;
            float posY = style.startY + static_cast<float>(i) * (style.height + style.spacing);

            // 九宮格
            if (style.normalImage.enabled && normalTexture) {
                btn.normalNineSlice.setTexture(
                    normalTexture,
                    style.normalImage.left,
                    style.normalImage.right,
                    style.normalImage.top,
                    style.normalImage.bottom
                );
                btn.normalNineSlice.setSize(sf::Vector2f(style.width, style.height));
                btn.normalNineSlice.setPosition(sf::Vector2f(posX, posY));
            }

            if (style.hoverImage.enabled && hoverTexture) {
                btn.hoverNineSlice.setTexture(
                    hoverTexture,
                    style.hoverImage.left,
                    style.hoverImage.right,
                    style.hoverImage.top,
                    style.hoverImage.bottom
                );
                btn.hoverNineSlice.setSize(sf::Vector2f(style.width, style.height));
                btn.hoverNineSlice.setPosition(sf::Vector2f(posX, posY));
            }

            // 舊的純色方式
            if (!style.normalImage.enabled) {
                btn.shape.setSize(sf::Vector2f(style.width, style.height));
                btn.shape.setPosition(sf::Vector2f(posX, posY));
                btn.shape.setFillColor(style.normalBgColor);
                btn.shape.setOutlineThickness(2.f);
                btn.shape.setOutlineColor(style.normalOutlineColor);
            }

            // 文字
            btn.text.setFont(font);
            btn.text.setCharacterSize(style.fontSize);
            btn.text.setFillColor(style.textColor);

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
            bool hovered = false;

            if (style.normalImage.enabled) {
                hovered = btn.normalNineSlice.getGlobalBounds().contains(mousePosF);
            } else {
                hovered = btn.shape.getGlobalBounds().contains(mousePosF);
            }

            btn.isHovered = hovered;

            if (hovered) {
                if (!style.hoverImage.enabled) {
                    btn.shape.setFillColor(style.hoverBgColor);
                    btn.shape.setOutlineColor(style.hoverOutlineColor);
                }
            } else {
                if (!style.normalImage.enabled) {
                    btn.shape.setFillColor(style.normalBgColor);
                    btn.shape.setOutlineColor(style.normalOutlineColor);
                }
            }
        }
    }

    int handleMouseClick(sf::Vector2f mousePosF) {
        for (size_t i = 0; i < buttons.size(); ++i) {
            bool clicked = false;

            if (style.normalImage.enabled) {
                clicked = buttons[i].normalNineSlice.getGlobalBounds().contains(mousePosF);
            } else {
                clicked = buttons[i].shape.getGlobalBounds().contains(mousePosF);
            }

            if (clicked) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    void draw(sf::RenderTarget& target) {
        for (auto& btn : buttons) {
            if (btn.isHovered && style.hoverImage.enabled && btn.hoverNineSlice.isInitialized()) {
                btn.hoverNineSlice.draw(target);
            } else if (style.normalImage.enabled && btn.normalNineSlice.isInitialized()) {
                btn.normalNineSlice.draw(target);
            } else {
                target.draw(btn.shape);
            }
            target.draw(btn.text);
        }
    }
};