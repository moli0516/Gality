#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <algorithm>
#include <unordered_set>
#include <cmath>
#include <cstdlib>
#include "UITheme.hpp"
#include "../core/ConfigManager.hpp"
#include "../core/AssetPack.hpp"

// ============================================================================
// 1. 排版與富文字標籤資料結構
// ============================================================================
struct GlyphStyle {
    sf::Color color = sf::Color::White;
    bool isShaking = false;
    float customSpeed = -1.0f; // < 0 表示繼承全域 ConfigManager 設定
};

struct FormattedGlyph {
    char32_t codepoint = 0;
    sf::Vector2f position{0.0f, 0.0f};
    GlyphStyle style;
    float pauseDuration = 0.0f; // 該字元顯示完後的停頓時間 (秒)
};

// ============================================================================
// 2. DialogueBox 類別實作 (純 SFML 原生向量排版，零 ImGui 依賴)
// ============================================================================
class DialogueBox {
private:
    sf::RectangleShape boxShape;
    sf::RectangleShape nameBoxShape;
    sf::Font font;
    sf::Text nameText;

    // 持久保存字體二進位緩衝區，杜絕 FreeType 野指標光柵化失敗
    std::vector<std::uint8_t> fontDataBuffer;
    std::vector<std::uint8_t> soundDataBuffer;

    std::vector<FormattedGlyph> glyphs;
    size_t totalCharCount = 0;
    size_t visibleCharCount = 0;

    sf::Clock timer;
    float currentPauseRemaining = 0.0f;
    bool isCompleted = false;

    sf::SoundBuffer typeBuffer;
    std::unique_ptr<sf::Sound> typeSound;
    bool hasSound = false;

    DialogueBoxStyle style;

    // ------------------------------------------------------------------------
    // CJK Kinsoku Shori (避頭尾禁則字元集合)
    // ------------------------------------------------------------------------
    static bool isProhibitedAtLineStart(char32_t cp) {
        static const std::unordered_set<char32_t> lineStartProhibited = {
            U'，', U'。', U'！', U'？', U'：', U'；', U'、', U'）', U'」', U'』',
            U'”', U'’', U'>', U'·', U'…', U'—', U'~', U'～',
            U',', U'.', U'!', U'?', U':', U';', U')', U']', U'}'
        };
        return lineStartProhibited.find(cp) != lineStartProhibited.end();
    }

    static bool isProhibitedAtLineEnd(char32_t cp) {
        static const std::unordered_set<char32_t> lineEndProhibited = {
            U'（', U'「', U'『', U'“', U'‘', U'《', U'<', U'(', U'[', U'{'
        };
        return lineEndProhibited.find(cp) != lineEndProhibited.end();
    }

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

    // ------------------------------------------------------------------------
    // Tokenizer & Parser: 解析行內標籤並提取字元屬性
    // ------------------------------------------------------------------------
    struct ParsedChar {
        char32_t codepoint;
        GlyphStyle style;
        float pause = 0.0f;
    };

    std::vector<ParsedChar> parseInlineTags(const std::string& rawText) {
        std::vector<ParsedChar> result;
        sf::String sfRaw = sf::String::fromUtf8(rawText.begin(), rawText.end());
        std::u32string u32Str = sfRaw.toUtf32();

        std::vector<sf::Color> colorStack = { sf::Color::White };
        std::vector<bool> shakeStack = { false };
        std::vector<float> speedStack = { -1.0f };

        size_t idx = 0;
        size_t length = u32Str.size();

        while (idx < length) {
            if (u32Str[idx] == U'<') {
                size_t tagEnd = u32Str.find(U'>', idx);
                if (tagEnd != std::u32string::npos) {
                    // 將 ASCII 標籤直接轉換為 std::string，避開 SFML 3.x sf::U8String 型別限制
                    std::string tag;
                    tag.reserve(tagEnd - idx - 1);
                    for (size_t t = idx + 1; t < tagEnd; ++t) {
                        if (u32Str[t] < 128) {
                            tag += static_cast<char>(u32Str[t]);
                        }
                    }

                    if (tag.rfind("color=", 0) == 0) {
                        std::string hex = tag.substr(6);
                        colorStack.push_back(hexToColor(hex));
                        idx = tagEnd + 1;
                        continue;
                    } else if (tag == "/color") {
                        if (colorStack.size() > 1) colorStack.pop_back();
                        idx = tagEnd + 1;
                        continue;
                    } else if (tag == "shake") {
                        shakeStack.push_back(true);
                        idx = tagEnd + 1;
                        continue;
                    } else if (tag == "/shake") {
                        if (shakeStack.size() > 1) shakeStack.pop_back();
                        idx = tagEnd + 1;
                        continue;
                    } else if (tag.rfind("speed=", 0) == 0) {
                        try {
                            float spd = std::stof(tag.substr(6));
                            speedStack.push_back(spd);
                        } catch (...) {}
                        idx = tagEnd + 1;
                        continue;
                    } else if (tag == "/speed") {
                        if (speedStack.size() > 1) speedStack.pop_back();
                        idx = tagEnd + 1;
                        continue;
                    } else if (tag.rfind("w=", 0) == 0) {
                        float pauseSec = 0.0f;
                        try {
                            pauseSec = std::stof(tag.substr(2));
                        } catch (...) {}

                        if (!result.empty()) {
                            result.back().pause += pauseSec;
                        } else {
                            ParsedChar placeholder;
                            placeholder.codepoint = U' ';
                            placeholder.style.color = sf::Color::Transparent;
                            placeholder.pause = pauseSec;
                            result.push_back(placeholder);
                        }
                        idx = tagEnd + 1;
                        continue;
                    }
                }
            }

            ParsedChar pc;
            pc.codepoint = u32Str[idx];
            pc.style.color = colorStack.back();
            pc.style.isShaking = shakeStack.back();
            pc.style.customSpeed = speedStack.back();
            result.push_back(pc);
            idx++;
        }
        return result;
    }

    // ------------------------------------------------------------------------
    // Layout Shaper Pass: 執行字形測量、避頭尾斷行與座標計算
    // ------------------------------------------------------------------------
    void buildLayout(const std::vector<ParsedChar>& parsedList) {
        glyphs.clear();
        totalCharCount = parsedList.size();
        visibleCharCount = 0;
        isCompleted = false;

        float padding = 25.0f;
        float startX = boxShape.getPosition().x + padding;
        float startY = boxShape.getPosition().y + padding;
        float maxX = boxShape.getPosition().x + boxShape.getSize().x - padding;
        float lineHeight = style.dialogueFontSize * 1.4f;

        float currX = startX;
        float currY = startY;

        for (size_t i = 0; i < parsedList.size(); ++i) {
            const auto& item = parsedList[i];
            char32_t cp = item.codepoint;

            if (cp == U'\n') {
                currX = startX;
                currY += lineHeight;
                continue;
            }

            const auto& glyph = font.getGlyph(cp, style.dialogueFontSize, false);
            float advanceX = glyph.advance;
            if (advanceX <= 0.0f) {
                advanceX = static_cast<float>(style.dialogueFontSize);
            }

            bool shouldWrap = false;

            if (currX + advanceX > maxX) {
                shouldWrap = true;
            } else if (i + 1 < parsedList.size()) {
                char32_t nextCp = parsedList[i + 1].codepoint;
                if (isProhibitedAtLineStart(nextCp)) {
                    const auto& nextGlyph = font.getGlyph(nextCp, style.dialogueFontSize, false);
                    float nextAdvance = (nextGlyph.advance > 0.0f) ? nextGlyph.advance : static_cast<float>(style.dialogueFontSize);
                    if (currX + advanceX + nextAdvance > maxX) {
                        shouldWrap = true;
                    }
                }
            }

            if (!shouldWrap && isProhibitedAtLineEnd(cp)) {
                if (currX + advanceX * 2.0f > maxX) {
                    shouldWrap = true;
                }
            }

            if (shouldWrap) {
                currX = startX;
                currY += lineHeight;
            }

            FormattedGlyph fg;
            fg.codepoint = cp;
            fg.position = sf::Vector2f(currX, currY);
            fg.style = item.style;
            fg.pauseDuration = item.pause;
            glyphs.push_back(fg);

            currX += advanceX;
        }

        totalCharCount = glyphs.size();
    }

public:
    DialogueBox() : nameText(font) {}

    bool loadFont(const std::string& fontPath) {
        fontDataBuffer.clear();
        if (AssetPack::readFileFromPak(fontPath, fontDataBuffer, "data.pak") && !fontDataBuffer.empty()) {
            if (font.openFromMemory(fontDataBuffer.data(), fontDataBuffer.size())) {
                nameText.setFont(font);
                return true;
            }
        }
        if (font.openFromFile(fontPath)) {
            nameText.setFont(font);
            return true;
        }
        return false;
    }

    void loadTypeSound(const std::string& soundPath) {
        soundDataBuffer.clear();
        if (AssetPack::readFileFromPak(soundPath, soundDataBuffer, "data.pak") && !soundDataBuffer.empty()) {
            if (typeBuffer.loadFromMemory(soundDataBuffer.data(), soundDataBuffer.size())) {
                typeSound = std::make_unique<sf::Sound>(typeBuffer);
                hasSound = true;
            }
            return;
        }

        if (typeBuffer.loadFromFile(soundPath)) {
            typeSound = std::make_unique<sf::Sound>(typeBuffer);
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
        
        auto parsedList = parseInlineTags(text);
        buildLayout(parsedList);

        currentPauseRemaining = 0.0f;
        timer.restart();
    }

    void update() {
        if (isCompleted) return;

        float dt = timer.getElapsedTime().asSeconds();

        if (currentPauseRemaining > 0.0f) {
            currentPauseRemaining -= dt;
            timer.restart();
            return;
        }

        float stepInterval = ConfigManager::config.textSpeed;
        if (visibleCharCount < totalCharCount && glyphs[visibleCharCount].style.customSpeed >= 0.0f) {
            stepInterval = glyphs[visibleCharCount].style.customSpeed;
        }

        if (dt >= stepInterval) {
            timer.restart();
            if (visibleCharCount < totalCharCount) {
                if (glyphs[visibleCharCount].pauseDuration > 0.0f) {
                    currentPauseRemaining = glyphs[visibleCharCount].pauseDuration;
                }

                visibleCharCount++;

                if (hasSound && typeSound && visibleCharCount % 2 == 0) {
                    float finalSfxVol = 30.0f * (ConfigManager::config.sfxVolume / 100.0f) * (ConfigManager::config.masterVolume / 100.0f);
                    typeSound->setVolume(finalSfxVol);
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
            currentPauseRemaining = 0.0f;
            isCompleted = true;
            return false;
        }
        return true;
    }

    void draw(sf::RenderTarget& target) {
        target.draw(boxShape);
        if (!nameText.getString().isEmpty()) {
            target.draw(nameBoxShape);
            target.draw(nameText);
        }

        size_t charsToRender = std::min(visibleCharCount, totalCharCount);

        for (size_t i = 0; i < charsToRender; ++i) {
            const auto& fg = glyphs[i];

            sf::Vector2f drawPos = fg.position;

            if (fg.style.isShaking) {
                float ox = (-1.0f + static_cast<float>(rand()) / (RAND_MAX / 2.0f)) * 2.0f;
                float oy = (-1.0f + static_cast<float>(rand()) / (RAND_MAX / 2.0f)) * 2.0f;
                drawPos += sf::Vector2f(ox, oy);
            }

            std::u32string singleCharStr(1, fg.codepoint);
            sf::String sfChar(singleCharStr);

            sf::Text charText(font, sfChar, style.dialogueFontSize);
            charText.setFillColor(fg.style.color);
            charText.setPosition(drawPos);
            target.draw(charText);
        }
    }
};