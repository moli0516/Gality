#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <functional>
#include <optional>
#include "../core/ConfigManager.hpp"
#include "../core/AssetPack.hpp"

class SettingsUI {
public:
    enum class Tab {
        Audio,
        Graphics,
        System
    };

    // 公開確認對話框控制（main.cpp 需要呼叫）
    void openConfirm(const std::string& message, std::function<void()> onYes) {
        confirmMessage = message;
        confirmYesCallback = onYes;
        confirmVisible = true;
        if (hasFont) {
            confirmText.setString(sf::String::fromUtf8(message.begin(), message.end()));
        }
    }

    void closeConfirm() {
        confirmVisible = false;
        confirmYesCallback = nullptr;
    }

private:
    bool isVisible = false;
    sf::Font font;
    bool hasFont = false;
    std::vector<std::uint8_t> fontDataBuffer;

    // 背景與面板
    sf::RectangleShape backgroundDim;
    sf::RectangleShape panel;
    sf::RectangleShape panelBorder;
    sf::Text titleText;

    // 分頁標籤
    struct TabButton {
        Tab tab = Tab::Audio;
        std::string label;
        sf::FloatRect bounds;
        sf::RectangleShape shape;
        std::unique_ptr<sf::Text> text;

        TabButton() = default;
        TabButton(const TabButton&) = delete;
        TabButton& operator=(const TabButton&) = delete;
        TabButton(TabButton&&) = default;
        TabButton& operator=(TabButton&&) = default;
    };
    std::vector<TabButton> tabButtons;
    Tab currentTab = Tab::Audio;

    // 滑桿
    struct SliderControl {
        std::string label;
        float* targetFloat = nullptr;
        int*   targetInt   = nullptr;
        float minVal = 0.0f;
        float maxVal = 100.0f;
        bool isInteger = true;
        std::string suffix = "";
        Tab tab = Tab::Audio;
        sf::FloatRect trackBounds;
        bool isDragging = false;
    };
    std::vector<SliderControl> sliders;

    // 切換開關
    struct ToggleControl {
        std::string label;
        bool* targetValue = nullptr;
        Tab tab = Tab::Graphics;
        sf::FloatRect toggleBounds;
        bool isHovered = false;
    };
    std::vector<ToggleControl> toggles;

    // 下拉選單
    struct DropdownControl {
        std::string label;
        int* targetValue = nullptr;
        std::vector<std::string> options;
        Tab tab = Tab::Graphics;
        sf::FloatRect bounds;
        bool isHovered = false;
    };
    std::vector<DropdownControl> dropdowns;

    // 動作按鈕
    struct ActionButton {
        std::string label;
        std::function<void()> callback;
        Tab tab = Tab::System;
        sf::FloatRect bounds;
        sf::RectangleShape shape;
        std::unique_ptr<sf::Text> text;
        bool isHovered = false;
        sf::Color baseColor = sf::Color(45, 80, 140);
        sf::Color hoverColor = sf::Color(70, 120, 200);

        ActionButton() = default;
        ActionButton(const ActionButton&) = delete;
        ActionButton& operator=(const ActionButton&) = delete;
        ActionButton(ActionButton&&) = default;
        ActionButton& operator=(ActionButton&&) = default;
    };
    std::vector<ActionButton> actionButtons;

    // 確認對話框
    bool confirmVisible = false;
    std::string confirmMessage;
    std::function<void()> confirmYesCallback;
    sf::RectangleShape confirmBox;
    sf::RectangleShape confirmBorder;
    sf::Text confirmText;
    ActionButton confirmYesBtn;
    ActionButton confirmNoBtn;

    sf::Vector2u currentWindowSize{1920, 1080};
    std::function<void()> onExitCallback;
    std::function<void()> returnToTitleCallback;

    // ========================================================================
    // 輔助：建立 sf::Text
    // ========================================================================
    std::unique_ptr<sf::Text> makeText(const std::string& str, unsigned int size, sf::Color color) {
        auto t = std::make_unique<sf::Text>(font);
        t->setString(sf::String::fromUtf8(str.begin(), str.end()));
        t->setCharacterSize(size);
        t->setFillColor(color);
        return t;
    }

    // ========================================================================
    // 控制項建構
    // ========================================================================
    void buildControls() {
        sliders.clear();
        toggles.clear();
        dropdowns.clear();
        actionButtons.clear();

        // ===== 音訊頁 =====
        sliders.push_back({"Master Volume", &ConfigManager::config.masterVolume, nullptr, 0.0f, 100.0f, true, "%", Tab::Audio, {}});
        sliders.push_back({"BGM Volume",    &ConfigManager::config.bgmVolume,    nullptr, 0.0f, 100.0f, true, "%", Tab::Audio, {}});
        sliders.push_back({"Voice Volume",  &ConfigManager::config.voiceVolume,  nullptr, 0.0f, 100.0f, true, "%", Tab::Audio, {}});
        sliders.push_back({"SFX Volume",    &ConfigManager::config.sfxVolume,    nullptr, 0.0f, 100.0f, true, "%", Tab::Audio, {}});
        sliders.push_back({"Text Speed",    &ConfigManager::config.textSpeed,    nullptr, 0.005f, 0.10f, false, "s", Tab::Audio, {}});

        // ===== 畫面頁 =====
        dropdowns.push_back({
            "Render Scale",
            &ConfigManager::config.renderScale,
            {"1x (Standard)", "2x (Retina)"},
            Tab::Graphics, {}, false
        });

        dropdowns.push_back({
            "Window Mode",
            &ConfigManager::config.windowMode,
            {"Fullscreen", "Windowed"},
            Tab::Graphics, {}, false
        });

        toggles.push_back({"Post-Processing",   &ConfigManager::config.enablePostFX,      Tab::Graphics, {}, false});
        toggles.push_back({"Screen Transitions",&ConfigManager::config.enableTransitions, Tab::Graphics, {}, false});
        toggles.push_back({"Vertical Sync",     &ConfigManager::config.enableVsync,       Tab::Graphics, {}, false});

        sliders.push_back({"Weather Particles", nullptr, &ConfigManager::config.particleCount, 0.0f, 300.0f, true, "", Tab::Graphics, {}});

        // ===== 系統頁 =====
        {
            ActionButton btn;
            btn.label = "Reset to Defaults";
            btn.callback = [this]() {
                if (!confirmVisible) {
                    openConfirm(
                        "Reset all settings to default values?",
                        [this]() {
                            ConfigManager::config = EngineConfig{};
                            ConfigManager::save();
                            closeConfirm();
                            recalculateLayout();
                        }
                    );
                }
            };
            btn.tab = Tab::System;
            btn.baseColor = sf::Color(80, 80, 100);
            btn.hoverColor = sf::Color(110, 110, 140);
            actionButtons.push_back(std::move(btn));
        }

        {
            ActionButton btn;
            btn.label = "Return to Title";
            btn.callback = [this]() {
                if (!confirmVisible) {
                    openConfirm(
                        "Return to the title screen? Unsaved progress will be lost.",
                        [this]() {
                            closeConfirm();
                            if (returnToTitleCallback) returnToTitleCallback();
                        }
                    );
                }
            };
            btn.tab = Tab::System;
            btn.baseColor = sf::Color(60, 90, 60);
            btn.hoverColor = sf::Color(90, 130, 90);
            actionButtons.push_back(std::move(btn));
        }

        {
            ActionButton btn;
            btn.label = "Exit Game";
            btn.callback = [this]() {
                if (!confirmVisible) {
                    openConfirm(
                        "Are you sure you want to exit Gality Engine?",
                        [this]() {
                            ConfigManager::save();
                            closeConfirm();
                            if (onExitCallback) onExitCallback();
                        }
                    );
                }
            };
            btn.tab = Tab::System;
            btn.baseColor = sf::Color(140, 50, 50);
            btn.hoverColor = sf::Color(200, 70, 70);
            actionButtons.push_back(std::move(btn));
        }
    }

    // 更新滑桿值
    void updateSliderValue(SliderControl& slider, float mouseX) {
        float ratio = (mouseX - slider.trackBounds.position.x) / slider.trackBounds.size.x;
        ratio = std::clamp(ratio, 0.0f, 1.0f);
        float computed = slider.minVal + ratio * (slider.maxVal - slider.minVal);

        if (slider.targetInt) {
            *slider.targetInt = static_cast<int>(std::round(computed));
        } else if (slider.targetFloat) {
            *slider.targetFloat = computed;
        }
    }

    void recalculateLayout() {
        float winW = static_cast<float>(currentWindowSize.x);
        float winH = static_cast<float>(currentWindowSize.y);

        backgroundDim.setSize(sf::Vector2f(winW, winH));
        backgroundDim.setFillColor(sf::Color(0, 0, 0, 200));

        float panelW = 780.0f;
        float panelH = 620.0f;
        float panelX = (winW - panelW) * 0.5f;
        float panelY = (winH - panelH) * 0.5f;

        panel.setSize(sf::Vector2f(panelW, panelH));
        panel.setPosition(sf::Vector2f(panelX, panelY));
        panel.setFillColor(sf::Color(22, 24, 30, 248));

        panelBorder.setSize(sf::Vector2f(panelW, panelH));
        panelBorder.setPosition(sf::Vector2f(panelX, panelY));
        panelBorder.setFillColor(sf::Color::Transparent);
        panelBorder.setOutlineThickness(2.0f);
        panelBorder.setOutlineColor(sf::Color(90, 100, 130, 200));

        if (hasFont) {
            titleText.setFont(font);
            titleText.setString("SYSTEM CONFIGURATION");
            titleText.setCharacterSize(24);
            titleText.setFillColor(sf::Color(235, 240, 250));
            titleText.setPosition(sf::Vector2f(panelX + 35.0f, panelY + 22.0f));
        }

        // 分頁標籤
        tabButtons.clear();
        float tabX = panelX + 35.0f;
        float tabY = panelY + 70.0f;
        float tabW = 180.0f;
        float tabH = 44.0f;
        float tabGap = 10.0f;

        std::vector<std::pair<Tab, std::string>> tabDefs = {
            {Tab::Audio,    "Audio"},
            {Tab::Graphics, "Graphics"},
            {Tab::System,   "System"}
        };

        for (auto& [tabEnum, label] : tabDefs) {
            TabButton btn;
            btn.tab = tabEnum;
            btn.label = label;
            btn.bounds = sf::FloatRect(sf::Vector2f(tabX, tabY), sf::Vector2f(tabW, tabH));
            btn.shape.setSize(sf::Vector2f(tabW, tabH));
            btn.shape.setPosition(sf::Vector2f(tabX, tabY));
            btn.shape.setOutlineThickness(1.5f);
            if (hasFont) {
                btn.text = makeText(label, 18, sf::Color::White);
                btn.text->setPosition(sf::Vector2f(tabX + 15.0f, tabY + 10.0f));
            }
            tabButtons.push_back(std::move(btn));
            tabX += tabW + tabGap;
        }

        // =====================================================================
        // 控制項佈局（標籤區 280px，控制項區剩餘空間）
        // =====================================================================
        float contentX = panelX + 45.0f;
        float contentY = tabY + tabH + 40.0f;
        float contentW = panelW - 90.0f;
        float spacingY = 65.0f;

        float labelWidth = 280.0f;
        float controlX = contentX + labelWidth;
        float controlW = contentW - labelWidth;

        // 滑桿
        int sliderIdx = 0;
        for (auto& s : sliders) {
            if (s.tab != currentTab) { sliderIdx++; continue; }
            float rowY = contentY + static_cast<float>(sliderIdx) * spacingY;
            s.trackBounds = sf::FloatRect(
                sf::Vector2f(controlX, rowY + 12.0f),
                sf::Vector2f(controlW - 80.0f, 8.0f)
            );
            sliderIdx++;
        }

        // 下拉選單
        int ddIdx = 0;
        for (auto& d : dropdowns) {
            if (d.tab != currentTab) { ddIdx++; continue; }
            float rowY = contentY + static_cast<float>(ddIdx) * spacingY;
            d.bounds = sf::FloatRect(
                sf::Vector2f(controlX, rowY),
                sf::Vector2f(controlW - 80.0f, 38.0f)
            );
            ddIdx++;
        }

        // 切換開關
        int toggleIdx = static_cast<int>(dropdowns.size());
        for (auto& t : toggles) {
            if (t.tab != currentTab) { toggleIdx++; continue; }
            float rowY = contentY + static_cast<float>(toggleIdx) * spacingY;
            t.toggleBounds = sf::FloatRect(
                sf::Vector2f(controlX, rowY + 6.0f),
                sf::Vector2f(70.0f, 32.0f)
            );
            toggleIdx++;
        }

        // 動作按鈕
        int btnIdx = 0;
        for (auto& b : actionButtons) {
            if (b.tab != currentTab) { btnIdx++; continue; }
            float rowY = contentY + static_cast<float>(btnIdx) * (spacingY + 10.0f);
            b.bounds = sf::FloatRect(
                sf::Vector2f(contentX + 100.0f, rowY),
                sf::Vector2f(contentW - 200.0f, 45.0f)
            );
            b.shape.setSize(b.bounds.size);
            b.shape.setPosition(b.bounds.position);
            b.shape.setFillColor(b.baseColor);
            b.shape.setOutlineThickness(1.5f);
            b.shape.setOutlineColor(sf::Color(255, 255, 255, 60));
            if (hasFont) {
                b.text = makeText(b.label, 18, sf::Color::White);
                sf::FloatRect tb = b.text->getLocalBounds();
                b.text->setPosition(sf::Vector2f(
                    b.bounds.position.x + (b.bounds.size.x - tb.size.x) * 0.5f,
                    b.bounds.position.y + (b.bounds.size.y - tb.size.y) * 0.5f - 4.0f
                ));
            }
            btnIdx++;
        }

        // 確認對話框
        float cbW = 520.0f;
        float cbH = 220.0f;
        float cbX = (winW - cbW) * 0.5f;
        float cbY = (winH - cbH) * 0.5f;

        confirmBox.setSize(sf::Vector2f(cbW, cbH));
        confirmBox.setPosition(sf::Vector2f(cbX, cbY));
        confirmBox.setFillColor(sf::Color(30, 32, 40, 250));

        confirmBorder.setSize(sf::Vector2f(cbW, cbH));
        confirmBorder.setPosition(sf::Vector2f(cbX, cbY));
        confirmBorder.setFillColor(sf::Color::Transparent);
        confirmBorder.setOutlineThickness(2.0f);
        confirmBorder.setOutlineColor(sf::Color(200, 100, 100, 220));

        if (hasFont) {
            confirmText.setFont(font);
            confirmText.setCharacterSize(18);
            confirmText.setFillColor(sf::Color(240, 240, 245));
            confirmText.setPosition(sf::Vector2f(cbX + 30.0f, cbY + 40.0f));
        }

        float btnW = 140.0f;
        float btnH = 44.0f;
        float btnY = cbY + cbH - btnH - 30.0f;

        confirmYesBtn.bounds = sf::FloatRect(sf::Vector2f(cbX + cbW - btnW * 2 - 40.0f, btnY), sf::Vector2f(btnW, btnH));
        confirmYesBtn.shape.setSize(sf::Vector2f(btnW, btnH));
        confirmYesBtn.shape.setPosition(sf::Vector2f(cbX + cbW - btnW * 2 - 40.0f, btnY));
        confirmYesBtn.shape.setFillColor(sf::Color(140, 50, 50));
        confirmYesBtn.shape.setOutlineThickness(1.5f);
        confirmYesBtn.shape.setOutlineColor(sf::Color(255, 150, 150, 100));

        confirmNoBtn.bounds = sf::FloatRect(sf::Vector2f(cbX + cbW - btnW - 30.0f, btnY), sf::Vector2f(btnW, btnH));
        confirmNoBtn.shape.setSize(sf::Vector2f(btnW, btnH));
        confirmNoBtn.shape.setPosition(sf::Vector2f(cbX + cbW - btnW - 30.0f, btnY));
        confirmNoBtn.shape.setFillColor(sf::Color(60, 90, 60));
        confirmNoBtn.shape.setOutlineThickness(1.5f);
        confirmNoBtn.shape.setOutlineColor(sf::Color(150, 200, 150, 100));

        if (hasFont) {
            confirmYesBtn.text = makeText("Yes", 18, sf::Color::White);
            sf::FloatRect yb = confirmYesBtn.text->getLocalBounds();
            confirmYesBtn.text->setPosition(sf::Vector2f(
                confirmYesBtn.bounds.position.x + (btnW - yb.size.x) * 0.5f,
                confirmYesBtn.bounds.position.y + (btnH - yb.size.y) * 0.5f - 4.0f
            ));

            confirmNoBtn.text = makeText("No", 18, sf::Color::White);
            sf::FloatRect nb = confirmNoBtn.text->getLocalBounds();
            confirmNoBtn.text->setPosition(sf::Vector2f(
                confirmNoBtn.bounds.position.x + (btnW - nb.size.x) * 0.5f,
                confirmNoBtn.bounds.position.y + (btnH - nb.size.y) * 0.5f - 4.0f
            ));
        }
    }

public:
    SettingsUI() : titleText(font), confirmText(font) {
        buildControls();
    }

    bool loadFont(const std::string& fontPath) {
        fontDataBuffer.clear();
        hasFont = false;

        if (AssetPack::readFileFromPak(fontPath, fontDataBuffer, "data.pak") && !fontDataBuffer.empty()) {
            if (font.openFromMemory(fontDataBuffer.data(), fontDataBuffer.size())) {
                hasFont = true;
            }
        } else if (font.openFromFile(fontPath)) {
            hasFont = true;
        }

        if (hasFont) {
            buildControls();
            recalculateLayout();
            return true;
        }
        return false;
    }

    void setExitCallback(std::function<void()> cb) {
        onExitCallback = cb;
    }

    void setReturnToTitleCallback(std::function<void()> cb) {
        returnToTitleCallback = cb;
    }

    void setVisible(bool visible) {
        isVisible = visible;
        if (isVisible) {
            recalculateLayout();
        } else {
            ConfigManager::save();
            confirmVisible = false;
        }
    }

    void toggle() {
        setVisible(!isVisible);
    }

    bool getIsVisible() const { return isVisible; }
    bool getConfirmVisible() const { return confirmVisible; }

    void onResize(const sf::Vector2u& newSize) {
        currentWindowSize = newSize;
        recalculateLayout();
    }

    bool handleMouseButtonPressed(sf::Mouse::Button button, const sf::Vector2i& mousePos) {
        if (!isVisible) return false;
        if (button != sf::Mouse::Button::Left) return false;

        sf::Vector2f mPos(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

        // 確認對話框優先
        if (confirmVisible) {
            if (confirmYesBtn.bounds.contains(mPos)) {
                if (confirmYesCallback) confirmYesCallback();
                return true;
            }
            if (confirmNoBtn.bounds.contains(mPos)) {
                closeConfirm();
                return true;
            }
            return true;
        }

        // 分頁切換
        for (auto& tb : tabButtons) {
            if (tb.bounds.contains(mPos)) {
                currentTab = tb.tab;
                recalculateLayout();
                return true;
            }
        }

        // 滑桿拖曳
        for (auto& slider : sliders) {
            if (slider.tab != currentTab) continue;
            sf::FloatRect hitBox(
                sf::Vector2f(slider.trackBounds.position.x - 10.0f, slider.trackBounds.position.y - 12.0f),
                sf::Vector2f(slider.trackBounds.size.x + 20.0f, slider.trackBounds.size.y + 24.0f)
            );
            if (hitBox.contains(mPos)) {
                slider.isDragging = true;
                updateSliderValue(slider, mPos.x);
                return true;
            }
        }

        // 下拉選單切換
        for (auto& d : dropdowns) {
            if (d.tab != currentTab) continue;
            if (d.bounds.contains(mPos)) {
                if (d.targetValue && !d.options.empty()) {
                    *d.targetValue = (*d.targetValue + 1) % static_cast<int>(d.options.size());
                }
                return true;
            }
        }

        // 切換開關
        for (auto& t : toggles) {
            if (t.tab != currentTab) continue;
            if (t.toggleBounds.contains(mPos)) {
                if (t.targetValue) *t.targetValue = !(*t.targetValue);
                return true;
            }
        }

        // 動作按鈕
        for (auto& b : actionButtons) {
            if (b.tab != currentTab) continue;
            if (b.bounds.contains(mPos)) {
                if (b.callback) b.callback();
                return true;
            }
        }

        // 點擊面板外關閉
        if (!panel.getGlobalBounds().contains(mPos)) {
            setVisible(false);
            return true;
        }

        return true;
    }

    void handleMouseButtonReleased(sf::Mouse::Button button) {
        if (!isVisible) return;
        if (button == sf::Mouse::Button::Left) {
            for (auto& slider : sliders) {
                if (slider.isDragging) {
                    slider.isDragging = false;
                    ConfigManager::save();
                }
            }
        }
    }

    void handleMouseMove(const sf::Vector2i& mousePos) {
        if (!isVisible) return;
        sf::Vector2f mPos(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

        for (auto& slider : sliders) {
            if (slider.isDragging) {
                updateSliderValue(slider, mPos.x);
            }
        }

        for (auto& b : actionButtons) {
            b.isHovered = b.bounds.contains(mPos);
        }

        for (auto& t : toggles) {
            t.isHovered = t.toggleBounds.contains(mPos);
        }

        for (auto& d : dropdowns) {
            d.isHovered = d.bounds.contains(mPos);
        }
    }

    void draw(sf::RenderTarget& target) {
        if (!isVisible || !hasFont) return;

        target.draw(backgroundDim);
        target.draw(panel);
        target.draw(panelBorder);
        target.draw(titleText);

        // 分頁標籤
        for (auto& tb : tabButtons) {
            bool active = (tb.tab == currentTab);
            tb.shape.setFillColor(active ? sf::Color(45, 52, 70) : sf::Color(24, 27, 34));
            tb.shape.setOutlineColor(active ? sf::Color(100, 149, 237) : sf::Color(60, 65, 80));
            if (tb.text) tb.text->setFillColor(active ? sf::Color::White : sf::Color(170, 180, 200));
            target.draw(tb.shape);
            if (tb.text) target.draw(*tb.text);
        }

        float contentX = panel.getPosition().x + 45.0f;

        // =====================================================================
        // 滑桿
        // =====================================================================
        for (auto& slider : sliders) {
            if (slider.tab != currentTab) continue;

            sf::Text labelText(font);
            labelText.setString(sf::String::fromUtf8(slider.label.begin(), slider.label.end()));
            labelText.setCharacterSize(16);
            labelText.setFillColor(sf::Color(210, 215, 225));
            labelText.setPosition(sf::Vector2f(contentX, slider.trackBounds.position.y - 5.0f));
            target.draw(labelText);

            sf::RectangleShape track(slider.trackBounds.size);
            track.setPosition(slider.trackBounds.position);
            track.setFillColor(sf::Color(40, 44, 54));
            target.draw(track);

            float val = slider.targetInt ? static_cast<float>(*slider.targetInt)
                                         : (slider.targetFloat ? *slider.targetFloat : slider.minVal);
            float norm = (val - slider.minVal) / (slider.maxVal - slider.minVal);
            norm = std::clamp(norm, 0.0f, 1.0f);

            sf::RectangleShape fill(sf::Vector2f(slider.trackBounds.size.x * norm, slider.trackBounds.size.y));
            fill.setPosition(slider.trackBounds.position);
            fill.setFillColor(sf::Color(100, 149, 237));
            target.draw(fill);

            sf::CircleShape handle(7.0f);
            handle.setOrigin(sf::Vector2f(7.0f, 7.0f));
            handle.setPosition(sf::Vector2f(
                slider.trackBounds.position.x + slider.trackBounds.size.x * norm,
                slider.trackBounds.position.y + slider.trackBounds.size.y * 0.5f
            ));
            handle.setFillColor(slider.isDragging ? sf::Color::White : sf::Color(220, 225, 235));
            target.draw(handle);

            // 數值顯示（整數 / 浮點 + 後綴）
            std::ostringstream ss;
            if (slider.targetInt) {
                ss << *slider.targetInt;
            } else if (slider.isInteger) {
                ss << static_cast<int>(val);
            } else {
                ss << std::fixed << std::setprecision(3) << val;
            }
            ss << slider.suffix;
            std::string valStr = ss.str();

            sf::Text valText(font);
            valText.setString(sf::String::fromUtf8(valStr.begin(), valStr.end()));
            valText.setCharacterSize(14);
            valText.setFillColor(sf::Color(160, 170, 185));
            valText.setPosition(sf::Vector2f(
                slider.trackBounds.position.x + slider.trackBounds.size.x + 15.0f,
                slider.trackBounds.position.y - 5.0f
            ));
            target.draw(valText);
        }

        // =====================================================================
        // 下拉選單
        // =====================================================================
        for (auto& d : dropdowns) {
            if (d.tab != currentTab) continue;

            sf::Text labelText(font);
            labelText.setString(sf::String::fromUtf8(d.label.begin(), d.label.end()));
            labelText.setCharacterSize(16);
            labelText.setFillColor(sf::Color(210, 215, 225));
            labelText.setPosition(sf::Vector2f(contentX, d.bounds.position.y + 8.0f));
            target.draw(labelText);

            sf::RectangleShape box(d.bounds.size);
            box.setPosition(d.bounds.position);
            box.setFillColor(d.isHovered ? sf::Color(45, 55, 75) : sf::Color(30, 34, 44));
            box.setOutlineThickness(1.5f);
            box.setOutlineColor(d.isHovered ? sf::Color(100, 149, 237) : sf::Color(60, 70, 90));
            target.draw(box);

            std::string display = (d.targetValue && *d.targetValue < static_cast<int>(d.options.size()))
                ? d.options[*d.targetValue]
                : "---";
            sf::Text valText(font);
            valText.setString(sf::String::fromUtf8(display.begin(), display.end()));
            valText.setCharacterSize(15);
            valText.setFillColor(sf::Color::White);
            valText.setPosition(sf::Vector2f(d.bounds.position.x + 12.0f, d.bounds.position.y + 9.0f));
            target.draw(valText);

            // 箭頭：用 ASCII "v" 取代 Unicode ▼
            sf::Text arrow(font);
            arrow.setString("v");
            arrow.setCharacterSize(14);
            arrow.setFillColor(sf::Color(150, 160, 180));
            arrow.setPosition(sf::Vector2f(
                d.bounds.position.x + d.bounds.size.x - 20.0f,
                d.bounds.position.y + 12.0f
            ));
            target.draw(arrow);
        }

        // =====================================================================
        // 切換開關
        // =====================================================================
        for (auto& t : toggles) {
            if (t.tab != currentTab) continue;

            sf::Text labelText(font);
            labelText.setString(sf::String::fromUtf8(t.label.begin(), t.label.end()));
            labelText.setCharacterSize(15);
            labelText.setFillColor(sf::Color(210, 215, 225));
            labelText.setPosition(sf::Vector2f(contentX, t.toggleBounds.position.y + 6.0f));
            target.draw(labelText);

            bool on = t.targetValue && *t.targetValue;

            sf::RectangleShape track(t.toggleBounds.size);
            track.setPosition(t.toggleBounds.position);
            track.setFillColor(on ? sf::Color(70, 130, 100) : sf::Color(45, 48, 58));
            track.setOutlineThickness(1.5f);
            track.setOutlineColor(t.isHovered ? sf::Color(150, 180, 220) : sf::Color(80, 90, 105));
            target.draw(track);

            float knobX = on ? (t.toggleBounds.position.x + t.toggleBounds.size.x - 26.0f)
                             : (t.toggleBounds.position.x + 4.0f);
            sf::CircleShape knob(12.0f);
            knob.setFillColor(on ? sf::Color(180, 240, 200) : sf::Color(150, 155, 170));
            knob.setPosition(sf::Vector2f(knobX, t.toggleBounds.position.y + 4.0f));
            target.draw(knob);

            // ON/OFF 狀態文字
            sf::Text stateText(font);
            stateText.setString(on ? "ON" : "OFF");
            stateText.setCharacterSize(13);
            stateText.setFillColor(on ? sf::Color(180, 240, 200) : sf::Color(150, 155, 170));
            stateText.setPosition(sf::Vector2f(
                t.toggleBounds.position.x + t.toggleBounds.size.x + 14.0f,
                t.toggleBounds.position.y + 9.0f
            ));
            target.draw(stateText);
        }

        // =====================================================================
        // 動作按鈕
        // =====================================================================
        for (auto& b : actionButtons) {
            if (b.tab != currentTab) continue;
            b.shape.setFillColor(b.isHovered ? b.hoverColor : b.baseColor);
            target.draw(b.shape);
            if (b.text) target.draw(*b.text);
        }

        // =====================================================================
        // 確認對話框
        // =====================================================================
        if (confirmVisible) {
            sf::RectangleShape overlay(sf::Vector2f(
                static_cast<float>(target.getSize().x),
                static_cast<float>(target.getSize().y)
            ));
            overlay.setFillColor(sf::Color(0, 0, 0, 150));
            target.draw(overlay);

            target.draw(confirmBox);
            target.draw(confirmBorder);
            target.draw(confirmText);
            target.draw(confirmYesBtn.shape);
            if (confirmYesBtn.text) target.draw(*confirmYesBtn.text);
            target.draw(confirmNoBtn.shape);
            if (confirmNoBtn.text) target.draw(*confirmNoBtn.text);
        }
    }
};