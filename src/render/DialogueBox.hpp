#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <vector>
#include <regex>
#include <sstream>
#include <memory>
#include <algorithm>
#include "UITheme.hpp"

struct ColoredSegment {
    std::u32string text;
    sf::Color color;
};

class DialogueBox {
private:
    sf::RectangleShape boxShape;
    sf::RectangleShape nameBoxShape;
    sf::Font font;
    sf::Text nameText;
    
    std::vector<ColoredSegment> segments;
    
    size_t totalCharCount = 0;
    size_t visibleCharCount = 0;
    
    sf::Clock timer;
    float charDelay = 0.04f;
    bool isCompleted = false;

    // 💡 修正 1：使用 unique_ptr 確保 SFML 3.x 音效物件的移動與生命週期安全
    sf::SoundBuffer typeBuffer;
    std::unique_ptr<sf::Sound> typeSound;
    bool hasSound = false;

    DialogueBoxStyle style;

    // 💡 修正 2：安全的 C++ HEX 顏色解析
    sf::Color hexToColor(const std::string& hexStr) {
        std::string hex = hexStr;
        if (!hex.empty() && hex[0] == '#') hex = hex.substr(1);
        if (hex.size() != 6) return sf::Color::White;

        unsigned int r = 255, g = 255, b = 255;
        std::stringstream ssR(hex.substr(0, 2));
        std::stringstream ssG(hex.substr(2, 2));
        std::stringstream ssB(hex.substr(4, 2));

        ssR >> std::hex >> r;
        ssG >> std::hex >> g;
        ssB >> std::hex >> b;

        return sf::Color(static_cast<std::uint8_t>(r), static_cast<std::uint8_t>(g), static_cast<std::uint8_t>(b));
    }

public:
    DialogueBox() : nameText(font) {}

    bool loadFont(const std::string& fontPath) {
        return font.openFromFile(fontPath);
    }

    void loadTypeSound(const std::string& soundPath) {
        if (typeBuffer.loadFromFile(soundPath)) {
            typeSound = std::make_unique<sf::Sound>(typeBuffer);
            typeSound->setVolume(30.0f);
            hasSound = true;
        }
    }

    void applyTheme(const DialogueBoxStyle& st) {
        style = st;
        boxShape.setSize(sf::Vector2f(style.width, style.height));
        boxShape.setFillColor(style.bgColor);
        boxShape.setOutlineThickness(2.f);
        boxShape.setOutlineColor(style.borderColor);
        boxShape.setPosition(sf::Vector2f(style.posX, style.posY));

        nameBoxShape.setSize(sf::Vector2f(style.nameBoxWidth, style.nameBoxHeight));
        nameBoxShape.setFillColor(style.nameBoxBgColor);
        nameBoxShape.setPosition(sf::Vector2f(style.nameBoxPosX, style.nameBoxPosY));

        nameText.setCharacterSize(style.nameFontSize);
        nameText.setFillColor(style.nameTextColor);
        nameText.setPosition(sf::Vector2f(style.nameBoxPosX + 15.f, style.nameBoxPosY + 8.f));
    }

    void setText(const std::string& speaker, const std::string& text) {
        nameText.setString(sf::String::fromUtf8(speaker.begin(), speaker.end()));
        segments.clear();
        totalCharCount = 0;
        visibleCharCount = 0;
        isCompleted = false;

        std::regex colorRegex(R"(<color=(#[0-9A-Fa-f]{6})>(.*?)</color>)");
        auto words_begin = std::sregex_iterator(text.begin(), text.end(), colorRegex);
        auto words_end = std::sregex_iterator();

        size_t lastPos = 0;
        for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
            std::smatch match = *i;
            size_t matchPos = match.position();

            if (matchPos > lastPos) {
                std::string raw = text.substr(lastPos, matchPos - lastPos);
                sf::String sfStr = sf::String::fromUtf8(raw.begin(), raw.end());
                std::u32string u32(sfStr.begin(), sfStr.end());
                segments.push_back({u32, sf::Color::White});
                totalCharCount += u32.size();
            }

            std::string hexColor = match[1].str();
            std::string innerText = match[2].str();
            
            sf::String sfStr = sf::String::fromUtf8(innerText.begin(), innerText.end());
            std::u32string u32(sfStr.begin(), sfStr.end());
            segments.push_back({u32, hexToColor(hexColor)});
            totalCharCount += u32.size();

            lastPos = matchPos + match.length();
        }

        if (lastPos < text.size()) {
            std::string raw = text.substr(lastPos);
            sf::String sfStr = sf::String::fromUtf8(raw.begin(), raw.end());
            std::u32string u32(sfStr.begin(), sfStr.end());
            segments.push_back({u32, sf::Color::White});
            totalCharCount += u32.size();
        }

        timer.restart();
    }

    void update() {
        if (isCompleted) return;

        if (timer.getElapsedTime().asSeconds() >= charDelay) {
            timer.restart();
            if (visibleCharCount < totalCharCount) {
                visibleCharCount++;
                if (hasSound && typeSound && visibleCharCount % 2 == 0) {
                    typeSound->play();
                }
            } else {
                isCompleted = true;
            }
        }
    }

    bool onInteract() {
        if (!isCompleted) {
            visibleCharCount = totalCharCount;
            isCompleted = true;
            return false;
        }
        return true;
    }

    // 💡 修正 3：加入精準字元寬度量測與自動換行 (Word Wrap) 渲染邏輯
    void draw(sf::RenderTarget& target) {
        target.draw(boxShape);
        if (!nameText.getString().isEmpty()) {
            target.draw(nameBoxShape);
            target.draw(nameText);
        }

        float padding = 25.f;
        float startX = boxShape.getPosition().x + padding;
        float startY = boxShape.getPosition().y + padding;
        float maxX = boxShape.getPosition().x + boxShape.getSize().x - padding;
        float lineHeight = style.dialogueFontSize * 1.4f;

        float currX = startX;
        float currY = startY;
        size_t drawnChars = 0;

        for (const auto& seg : segments) {
            if (drawnChars >= visibleCharCount) break;

            size_t countToDraw = std::min(seg.text.size(), visibleCharCount - drawnChars);

            for (size_t i = 0; i < countToDraw; ++i) {
                char32_t ch = seg.text[i];
                if (ch == U'\n') {
                    currX = startX;
                    currY += lineHeight;
                    continue;
                }

                sf::String sfChar(ch);
                sf::Text charText(font, sfChar, style.dialogueFontSize);
                charText.setFillColor(seg.color);

                float charWidth = charText.getLocalBounds().size.x;
                if (charWidth == 0.0f) charWidth = style.dialogueFontSize * 0.5f;

                // 自動換行檢查
                if (currX + charWidth > maxX) {
                    currX = startX;
                    currY += lineHeight;
                }

                charText.setPosition(sf::Vector2f(currX, currY));
                target.draw(charText);

                currX += charWidth + 1.0f; // 加上固定 1px 字間距補償
            }

            drawnChars += countToDraw;
        }
    }
};