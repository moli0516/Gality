#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <algorithm>
#include <unordered_set>
#include <functional>
#include <cmath>
#include <cstdlib>
#include "UITheme.hpp"
#include "NineSliceSprite.hpp"
#include "../core/ConfigManager.hpp"
#include "../core/AssetPack.hpp"

// ============================================================================
// 排版與富文字標籤資料結構
// ============================================================================
struct GlyphStyle {
    sf::Color color = sf::Color::White;
    bool isShaking = false;
    bool isGlitching = false;
    float customSpeed = -1.0f;
};

struct FormattedGlyph {
    char32_t codepoint = 0;
    sf::Vector2f position{0.0f, 0.0f};
    GlyphStyle style;
    float pauseDuration = 0.0f;
    float glitchSeed = 0.0f;
};

// ============================================================================
// DialogueBox
// ============================================================================
class DialogueBox {
public:
    enum class ButtonAction {
        None,
        Auto,
        Skip,
        Backlog,
        Save,
        Load,
        Hide,
        Menu
    };

private:
    // ===== 對話框本體 =====
    sf::RectangleShape boxShape;
    sf::RectangleShape nameBoxShape;
    sf::Font font;
    sf::Text nameText;

    // ⚠️ 九宮格
    NineSliceSprite boxNineSlice;
    NineSliceSprite nameBoxNineSlice;
    std::shared_ptr<sf::Texture> boxTexture;
    std::shared_ptr<sf::Texture> nameBoxTexture;

    std::vector<std::uint8_t> fontDataBuffer;
    std::vector<std::uint8_t> soundDataBuffer;

    std::vector<FormattedGlyph> glyphs;
    size_t totalCharCount = 0;
    size_t visibleCharCount = 0;

    sf::Clock timer;
    sf::Clock glitchClock;
    float currentPauseRemaining = 0.0f;
    bool isCompleted = false;

    sf::SoundBuffer typeBuffer;
    std::unique_ptr<sf::Sound> typeSound;
    bool hasSound = false;

    DialogueBoxStyle style;

    // ===== 功能按鈕 =====
    struct FunctionButton {
        sf::RectangleShape shape;
        std::unique_ptr<sf::Text> text;
        std::string id;
        ButtonAction action = ButtonAction::None;
        bool isHovered = false;
        bool isActive = false;
    };
    std::vector<FunctionButton> functionButtons;
    DialogueButtonsStyle buttonsStyle;
    bool buttonsEnabled = false;

    // ===== 模式狀態 =====
    bool autoMode = false;
    float autoDelay = 1.5f;
    float autoTimer = 0.0f;

    // ===== Skip 按住狀態 =====
    bool skipHeld = false;
    float skipTimer = 0.0f;
    float skipInterval = 0.05f;
    float skipWarmup = 0.0f;
    const float skipWarmupDuration = 0.15f;

    bool hiddenMode = false;

    // ===== 回呼 =====
    std::function<void()> onBacklogCallback;
    std::function<void()> onSaveCallback;
    std::function<void()> onLoadCallback;
    std::function<void()> onMenuCallback;
    std::function<void()> onAdvanceCallback;

    // ------------------------------------------------------------------------
    // 載入紋理輔助
    // ------------------------------------------------------------------------
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
        std::cerr << "[DialogueBox] Failed to load texture: " << path << std::endl;
        return nullptr;
    }

    // ------------------------------------------------------------------------
    // CJK Kinsoku Shori (避頭尾禁則)
    // ------------------------------------------------------------------------
    static bool isProhibitedAtLineStart(char32_t cp) {
        static const std::unordered_set<char32_t> prohibited = {
            U'，', U'。', U'！', U'？', U'：', U'；', U'、', U'）', U'」', U'』',
            U'”', U'’', U'>', U'·', U'…', U'—', U'~', U'～',
            U',', U'.', U'!', U'?', U':', U';', U')', U']', U'}'
        };
        return prohibited.find(cp) != prohibited.end();
    }

    static bool isProhibitedAtLineEnd(char32_t cp) {
        static const std::unordered_set<char32_t> prohibited = {
            U'（', U'「', U'『', U'“', U'‘', U'《', U'<', U'(', U'[', U'{'
        };
        return prohibited.find(cp) != prohibited.end();
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

        return sf::Color(
            static_cast<std::uint8_t>(r),
            static_cast<std::uint8_t>(g),
            static_cast<std::uint8_t>(b)
        );
    }

    // ------------------------------------------------------------------------
    // 解析行內標籤
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
        std::vector<bool> glitchStack = { false };
        std::vector<float> speedStack = { -1.0f };

        size_t idx = 0;
        size_t length = u32Str.size();

        while (idx < length) {
            if (u32Str[idx] == U'<') {
                size_t tagEnd = u32Str.find(U'>', idx);
                if (tagEnd != std::u32string::npos) {
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
                    } else if (tag == "glitch") {
                        glitchStack.push_back(true);
                        idx = tagEnd + 1;
                        continue;
                    } else if (tag == "/glitch") {
                        if (glitchStack.size() > 1) glitchStack.pop_back();
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
            pc.style.isGlitching = glitchStack.back();
            pc.style.customSpeed = speedStack.back();
            result.push_back(pc);
            idx++;
        }
        return result;
    }

    // ------------------------------------------------------------------------
    // 排版
    // ------------------------------------------------------------------------
    void buildLayout(const std::vector<ParsedChar>& parsedList) {
        glyphs.clear();
        totalCharCount = parsedList.size();
        visibleCharCount = 0;
        isCompleted = false;
        autoTimer = 0.0f;

        float padding = 25.0f;
        float startX = boxShape.getPosition().x + padding;
        float startY = boxShape.getPosition().y + padding;
        float maxX = boxShape.getPosition().x + boxShape.getSize().x - padding;
        float lineHeight = style.dialogueFontSize * 1.4f;

        // 若使用九宮格，從 style 取位置
        if (style.backgroundImage.enabled) {
            startX = style.posX + padding;
            startY = style.posY + padding;
            maxX = style.posX + style.width - padding;
        }

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
                    float nextAdvance = (nextGlyph.advance > 0.0f)
                        ? nextGlyph.advance
                        : static_cast<float>(style.dialogueFontSize);
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

            if (item.style.isGlitching) {
                fg.glitchSeed = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            }

            glyphs.push_back(fg);
            currX += advanceX;
        }

        totalCharCount = glyphs.size();
    }

    // ------------------------------------------------------------------------
    // 按鈕建立
    // ------------------------------------------------------------------------
    static ButtonAction stringToAction(const std::string& id) {
        if (id == "auto") return ButtonAction::Auto;
        if (id == "skip") return ButtonAction::Skip;
        if (id == "backlog") return ButtonAction::Backlog;
        if (id == "save") return ButtonAction::Save;
        if (id == "load") return ButtonAction::Load;
        if (id == "hide") return ButtonAction::Hide;
        if (id == "menu") return ButtonAction::Menu;
        return ButtonAction::None;
    }

    void buildFunctionButtons() {
        functionButtons.clear();
        if (!buttonsEnabled) return;

        float panelRight = style.posX + style.width;
        float panelBottom = style.posY;

        float baseX = panelRight;
        float baseY = panelBottom - buttonsStyle.buttonHeight - 8.0f;

        if (buttonsStyle.position == "top-right") {
            baseY = style.posY + 8.0f;
        } else if (buttonsStyle.position == "bottom-left") {
            baseX = style.posX + 8.0f;
        } else if (buttonsStyle.position == "top-left") {
            baseX = style.posX + 8.0f;
            baseY = style.posY + 8.0f;
        }

        size_t enabledCount = 0;
        for (const auto& cfg : buttonsStyle.buttons) {
            if (cfg.enabled) enabledCount++;
        }

        if (enabledCount == 0) return;

        float totalWidth = enabledCount * buttonsStyle.buttonWidth
                         + (enabledCount - 1) * buttonsStyle.spacing;

        float startX = baseX - totalWidth - 8.0f;
        float startY = baseY;

        for (const auto& cfg : buttonsStyle.buttons) {
            if (!cfg.enabled) continue;

            FunctionButton btn;
            btn.id = cfg.id;
            btn.action = stringToAction(cfg.id);

            btn.shape.setSize(sf::Vector2f(buttonsStyle.buttonWidth, buttonsStyle.buttonHeight));
            btn.shape.setPosition(sf::Vector2f(startX, startY));
            btn.shape.setFillColor(buttonsStyle.normalBgColor);
            btn.shape.setOutlineThickness(1.5f);
            btn.shape.setOutlineColor(buttonsStyle.outlineColor);

            btn.text = std::make_unique<sf::Text>(font);
            btn.text->setString(sf::String::fromUtf8(cfg.text.begin(), cfg.text.end()));
            btn.text->setCharacterSize(buttonsStyle.fontSize);
            btn.text->setFillColor(buttonsStyle.textColor);

            sf::FloatRect tb = btn.text->getLocalBounds();
            btn.text->setPosition(sf::Vector2f(
                startX + (buttonsStyle.buttonWidth - tb.size.x) * 0.5f,
                startY + (buttonsStyle.buttonHeight - tb.size.y) * 0.5f - 4.0f
            ));

            functionButtons.push_back(std::move(btn));
            startX += buttonsStyle.buttonWidth + buttonsStyle.spacing;
        }
    }

public:
    DialogueBox() : nameText(font) {}

    // ========================================================================
    // 載入字型
    // ========================================================================
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

    // ========================================================================
    // 套用主題
    // ========================================================================
    void applyTheme(const DialogueBoxStyle& st, const DialogueButtonsStyle& btnStyle) {
        style = st;
        buttonsStyle = btnStyle;
        buttonsEnabled = btnStyle.enabled;

        skipInterval = btnStyle.skipInterval;
        if (skipInterval <= 0.0f) skipInterval = 0.05f;

        // ===== 對話框本體 =====
        if (style.backgroundImage.enabled) {
            boxTexture = loadTexture(style.backgroundImage.texturePath);
            if (boxTexture) {
                boxNineSlice.setTexture(
                    boxTexture,
                    style.backgroundImage.left,
                    style.backgroundImage.right,
                    style.backgroundImage.top,
                    style.backgroundImage.bottom
                );
                boxNineSlice.setSize(sf::Vector2f(style.width, style.height));
                boxNineSlice.setPosition(sf::Vector2f(style.posX, style.posY));
                std::cout << "[DialogueBox] Nine-slice box loaded" << std::endl;
            } else {
                std::cerr << "[DialogueBox] Failed to load box image, falling back to color" << std::endl;
                style.backgroundImage.enabled = false;
            }
        }

        if (!style.backgroundImage.enabled) {
            boxShape.setSize(sf::Vector2f(style.width, style.height));
            boxShape.setFillColor(style.bgColor);
            boxShape.setOutlineThickness(2.f);
            boxShape.setOutlineColor(style.borderColor);
            boxShape.setPosition(sf::Vector2f(style.posX, style.posY));
        }

        // ===== 名字框 =====
        if (style.nameBoxImage.enabled) {
            nameBoxTexture = loadTexture(style.nameBoxImage.texturePath);
            if (nameBoxTexture) {
                nameBoxNineSlice.setTexture(
                    nameBoxTexture,
                    style.nameBoxImage.left,
                    style.nameBoxImage.right,
                    style.nameBoxImage.top,
                    style.nameBoxImage.bottom
                );
                nameBoxNineSlice.setSize(sf::Vector2f(style.nameBoxWidth, style.nameBoxHeight));
                nameBoxNineSlice.setPosition(sf::Vector2f(style.nameBoxPosX, style.nameBoxPosY));
            } else {
                style.nameBoxImage.enabled = false;
            }
        }

        if (!style.nameBoxImage.enabled) {
            nameBoxShape.setSize(sf::Vector2f(style.nameBoxWidth, style.nameBoxHeight));
            nameBoxShape.setFillColor(style.nameBoxBgColor);
            nameBoxShape.setPosition(sf::Vector2f(style.nameBoxPosX, style.nameBoxPosY));
        }

        // ===== 名字文字 =====
        nameText.setCharacterSize(style.nameFontSize);
        nameText.setFillColor(style.nameTextColor);
        nameText.setPosition(sf::Vector2f(style.nameBoxPosX + 15.f, style.nameBoxPosY + 8.f));

        buildFunctionButtons();
    }

    // 向後相容：舊的 applyTheme
    void applyTheme(const DialogueBoxStyle& st) {
        DialogueButtonsStyle defaultBtnStyle;
        applyTheme(st, defaultBtnStyle);
    }

    // ========================================================================
    // 設定文字
    // ========================================================================
    void setText(const std::string& speaker, const std::string& text) {
        nameText.setString(sf::String::fromUtf8(speaker.begin(), speaker.end()));

        auto parsedList = parseInlineTags(text);
        buildLayout(parsedList);

        currentPauseRemaining = 0.0f;
        timer.restart();
    }

    // ========================================================================
    // 更新
    // ========================================================================
    void update() {
        // ⚠️ Skip 按住模式：自動快速推進
        if (skipHeld && !hiddenMode) {
            float dt = timer.getElapsedTime().asSeconds();
            timer.restart();

            visibleCharCount = totalCharCount;
            isCompleted = true;

            skipWarmup += dt;
            if (skipWarmup < skipWarmupDuration) {
                return;
            }

            skipTimer += dt;
            if (skipTimer >= skipInterval) {
                skipTimer = 0.0f;
                if (onAdvanceCallback) {
                    onAdvanceCallback();
                }
            }
            return;
        }

        // 自動模式
        if (isCompleted) {
            if (autoMode && !hiddenMode) {
                autoTimer += timer.getElapsedTime().asSeconds();
                timer.restart();
                if (autoTimer >= autoDelay) {
                    autoTimer = 0.0f;
                    if (onAdvanceCallback) onAdvanceCallback();
                }
            }
            return;
        }

        // 一般打字機效果
        float dt = timer.getElapsedTime().asSeconds();

        if (currentPauseRemaining > 0.0f) {
            currentPauseRemaining -= dt;
            timer.restart();
            return;
        }

        float stepInterval = ConfigManager::config.textSpeed;

        if (visibleCharCount < totalCharCount &&
            glyphs[visibleCharCount].style.customSpeed >= 0.0f) {
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
                    float finalSfxVol = 30.0f
                        * (ConfigManager::config.sfxVolume / 100.0f)
                        * (ConfigManager::config.masterVolume / 100.0f);
                    typeSound->setVolume(finalSfxVol);
                    typeSound->play();
                }
            } else {
                isCompleted = true;
                autoTimer = 0.0f;
            }
        }
    }

    bool onInteract() {
        if (skipHeld) return false;

        if (hiddenMode) {
            hiddenMode = false;
            return false;
        }

        if (!isCompleted) {
            visibleCharCount = totalCharCount;
            currentPauseRemaining = 0.0f;
            isCompleted = true;
            return false;
        }
        return true;
    }

    // ========================================================================
    // 按鈕互動
    // ========================================================================
    void updateButtonHover(sf::Vector2f mousePosF) {
        if (!buttonsEnabled || hiddenMode) return;
        for (auto& btn : functionButtons) {
            btn.isHovered = btn.shape.getGlobalBounds().contains(mousePosF);
        }
    }

    bool handleButtonPress(sf::Vector2f mousePosF) {
        if (!buttonsEnabled || hiddenMode) return false;

        for (auto& btn : functionButtons) {
            if (btn.shape.getGlobalBounds().contains(mousePosF)) {
                switch (btn.action) {
                    case ButtonAction::Auto:
                        autoMode = !autoMode;
                        btn.isActive = autoMode;
                        autoTimer = 0.0f;
                        break;

                    case ButtonAction::Skip:
                        skipHeld = true;
                        skipWarmup = 0.0f;
                        skipTimer = 0.0f;
                        btn.isActive = true;
                        break;

                    case ButtonAction::Backlog:
                        if (onBacklogCallback) onBacklogCallback();
                        break;

                    case ButtonAction::Save:
                        if (onSaveCallback) onSaveCallback();
                        break;

                    case ButtonAction::Load:
                        if (onLoadCallback) onLoadCallback();
                        break;

                    case ButtonAction::Hide:
                        hiddenMode = true;
                        break;

                    case ButtonAction::Menu:
                        if (onMenuCallback) onMenuCallback();
                        break;

                    default:
                        break;
                }
                return true;
            }
        }
        return false;
    }

    void handleButtonRelease() {
        if (skipHeld) {
            skipHeld = false;
            skipTimer = 0.0f;
            skipWarmup = 0.0f;

            for (auto& btn : functionButtons) {
                if (btn.action == ButtonAction::Skip) {
                    btn.isActive = false;
                }
            }
        }
    }

    // ========================================================================
    // 回呼設定
    // ========================================================================
    void setBacklogCallback(std::function<void()> cb) { onBacklogCallback = cb; }
    void setSaveCallback(std::function<void()> cb) { onSaveCallback = cb; }
    void setLoadCallback(std::function<void()> cb) { onLoadCallback = cb; }
    void setMenuCallback(std::function<void()> cb) { onMenuCallback = cb; }
    void setAdvanceCallback(std::function<void()> cb) { onAdvanceCallback = cb; }

    // ========================================================================
    // 狀態存取
    // ========================================================================
    bool isAutoMode() const { return autoMode; }
    bool isSkipHeld() const { return skipHeld; }
    bool isHiddenMode() const { return hiddenMode; }
    bool isTyping() const { return !isCompleted; }

    void setAutoMode(bool enabled) { autoMode = enabled; }
    void setHiddenMode(bool enabled) { hiddenMode = enabled; }
    void setAutoDelay(float seconds) { autoDelay = seconds; }
    void setSkipInterval(float seconds) {
        skipInterval = std::max(0.01f, seconds);
    }

    void stopSkip() {
        skipHeld = false;
        skipTimer = 0.0f;
        skipWarmup = 0.0f;
        for (auto& btn : functionButtons) {
            if (btn.action == ButtonAction::Skip) {
                btn.isActive = false;
            }
        }
    }

    void resetModes() {
        autoMode = false;
        skipHeld = false;
        hiddenMode = false;
        autoTimer = 0.0f;
        skipTimer = 0.0f;
        skipWarmup = 0.0f;
        for (auto& btn : functionButtons) {
            btn.isActive = false;
        }
    }

    // ========================================================================
    // 繪製
    // ========================================================================
    void draw(sf::RenderTarget& target) {
        if (hiddenMode) return;

        // ===== 對話框 =====
        if (style.backgroundImage.enabled && boxNineSlice.isInitialized()) {
            boxNineSlice.draw(target);
        } else {
            target.draw(boxShape);
        }

        // ===== 名字框 =====
        if (!nameText.getString().isEmpty()) {
            if (style.nameBoxImage.enabled && nameBoxNineSlice.isInitialized()) {
                nameBoxNineSlice.draw(target);
            } else {
                target.draw(nameBoxShape);
            }
            target.draw(nameText);
        }

        // ===== 文字 =====
        size_t charsToRender = std::min(visibleCharCount, totalCharCount);
        float time = glitchClock.getElapsedTime().asSeconds();

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

            if (fg.style.isGlitching) {
                float glitchTime = time * 10.0f + fg.glitchSeed * 100.0f;
                float trigger = std::sin(glitchTime * 2.0f);

                if (trigger > 0.7f) {
                    float glitchX = std::sin(glitchTime * 50.0f) * 3.0f;
                    float glitchY = std::cos(glitchTime * 40.0f) * 2.0f;
                    drawPos.x += glitchX;
                    drawPos.y += glitchY;

                    sf::Text redText(font, sfChar, style.dialogueFontSize);
                    redText.setFillColor(sf::Color(255, 0, 0, 128));
                    redText.setPosition(drawPos + sf::Vector2f(-2.0f, 0.0f));
                    target.draw(redText);

                    sf::Text cyanText(font, sfChar, style.dialogueFontSize);
                    cyanText.setFillColor(sf::Color(0, 255, 255, 128));
                    cyanText.setPosition(drawPos + sf::Vector2f(2.0f, 0.0f));
                    target.draw(cyanText);
                }
            }

            sf::Text charText(font, sfChar, style.dialogueFontSize);
            charText.setFillColor(fg.style.color);
            charText.setPosition(drawPos);
            target.draw(charText);
        }

        // ===== 功能按鈕 =====
        drawFunctionButtons(target);
    }

    void drawFunctionButtons(sf::RenderTarget& target) {
        if (!buttonsEnabled) return;

        for (auto& btn : functionButtons) {
            if (btn.isActive) {
                btn.shape.setFillColor(buttonsStyle.activeBgColor);
                btn.shape.setOutlineColor(buttonsStyle.hoverOutlineColor);
            } else if (btn.isHovered) {
                btn.shape.setFillColor(buttonsStyle.hoverBgColor);
                btn.shape.setOutlineColor(buttonsStyle.hoverOutlineColor);
            } else {
                btn.shape.setFillColor(buttonsStyle.normalBgColor);
                btn.shape.setOutlineColor(buttonsStyle.outlineColor);
            }

            target.draw(btn.shape);
            if (btn.text) target.draw(*btn.text);
        }
    }
};