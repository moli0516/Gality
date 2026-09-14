#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>
#include "../core/ConfigManager.hpp"
#include "../core/AssetPack.hpp"

class SettingsUI {
private:
    bool isVisible = false;
    sf::Font font;
    bool hasFont = false;

    // 持久保存字體二進位緩衝區，杜絕生命週期懸空
    std::vector<std::uint8_t> fontDataBuffer;

    sf::RectangleShape backgroundDim;
    sf::RectangleShape panel;
    sf::RectangleShape panelBorder;
    sf::Text titleText;

    struct SliderControl {
        std::string label;
        float* targetValue = nullptr;
        float minVal = 0.0f;
        float maxVal = 100.0f;
        bool isInteger = true;
        sf::FloatRect trackBounds;
        bool isDragging = false;
    };

    struct ToggleControl {
        std::string label;
        bool* targetValue = nullptr;
        sf::FloatRect toggleBounds;
    };

    std::vector<SliderControl> sliders;
    std::vector<ToggleControl> toggles;
    sf::Vector2u currentWindowSize{1280, 720};

    void buildControls() {
        sliders.clear();
        toggles.clear();

        sliders.push_back({"Master Volume", &ConfigManager::config.masterVolume, 0.0f, 100.0f, true, {}});
        sliders.push_back({"BGM Volume", &ConfigManager::config.bgmVolume, 0.0f, 100.0f, true, {}});
        sliders.push_back({"Voice Volume", &ConfigManager::config.voiceVolume, 0.0f, 100.0f, true, {}});
        sliders.push_back({"SFX Volume", &ConfigManager::config.sfxVolume, 0.0f, 100.0f, true, {}});
        sliders.push_back({"Text Speed", &ConfigManager::config.textSpeed, 0.005f, 0.10f, false, {}});
    }

    void recalculateLayout() {
        float winW = static_cast<float>(currentWindowSize.x);
        float winH = static_cast<float>(currentWindowSize.y);

        backgroundDim.setSize(sf::Vector2f(winW, winH));
        backgroundDim.setFillColor(sf::Color(0, 0, 0, 200));

        float panelW = 680.0f;
        float panelH = 500.0f;
        float panelX = (winW - panelW) * 0.5f;
        float panelY = (winH - panelH) * 0.5f;

        panel.setSize(sf::Vector2f(panelW, panelH));
        panel.setPosition(sf::Vector2f(panelX, panelY));
        panel.setFillColor(sf::Color(22, 24, 30, 245));

        panelBorder.setSize(sf::Vector2f(panelW, panelH));
        panelBorder.setPosition(sf::Vector2f(panelX, panelY));
        panelBorder.setFillColor(sf::Color::Transparent);
        panelBorder.setOutlineThickness(2.0f);
        panelBorder.setOutlineColor(sf::Color(90, 100, 130, 200));

        if (hasFont) {
            titleText.setFont(font);
            titleText.setString("SYSTEM CONFIGURATION");
            titleText.setCharacterSize(22);
            titleText.setFillColor(sf::Color(235, 240, 250));
            titleText.setPosition(sf::Vector2f(panelX + 35.0f, panelY + 25.0f));
        }

        float startY = panelY + 80.0f;
        float spacingY = 55.0f;
        float trackW = 240.0f;
        float trackH = 8.0f;
        float trackX = panelX + panelW - trackW - 110.0f;

        for (size_t i = 0; i < sliders.size(); ++i) {
            float rowY = startY + static_cast<float>(i) * spacingY;
            sliders[i].trackBounds = sf::FloatRect(sf::Vector2f(trackX, rowY + 8.0f), sf::Vector2f(trackW, trackH));
        }
    }

    void updateSliderValue(SliderControl& slider, float mouseX) {
        if (!slider.targetValue) return;
        float ratio = (mouseX - slider.trackBounds.position.x) / slider.trackBounds.size.x;
        ratio = std::clamp(ratio, 0.0f, 1.0f);

        float computed = slider.minVal + ratio * (slider.maxVal - slider.minVal);
        if (slider.isInteger) {
            *slider.targetValue = std::round(computed);
        } else {
            *slider.targetValue = computed;
        }
    }

public:
    SettingsUI() : titleText(font) {
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
            recalculateLayout();
            return true;
        }
        return false;
    }

    void setVisible(bool visible) {
        isVisible = visible;
        if (isVisible) {
            recalculateLayout();
        } else {
            ConfigManager::save();
        }
    }

    void toggle() {
        setVisible(!isVisible);
    }

    bool getIsVisible() const {
        return isVisible;
    }

    void onResize(const sf::Vector2u& newSize) {
        currentWindowSize = newSize;
        recalculateLayout();
    }

    bool handleMouseButtonPressed(sf::Mouse::Button button, const sf::Vector2i& mousePos) {
        if (!isVisible) return false;

        if (button == sf::Mouse::Button::Left) {
            sf::Vector2f mPos(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

            for (auto& slider : sliders) {
                sf::FloatRect hitBox(
                    sf::Vector2f(slider.trackBounds.position.x - 10.0f, slider.trackBounds.position.y - 10.0f),
                    sf::Vector2f(slider.trackBounds.size.x + 20.0f, slider.trackBounds.size.y + 20.0f)
                );
                if (hitBox.contains(mPos)) {
                    slider.isDragging = true;
                    updateSliderValue(slider, mPos.x);
                    return true;
                }
            }

            if (panel.getGlobalBounds().contains(mPos)) {
                return true;
            }

            setVisible(false);
            return true;
        }
        return false;
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
    }

    void draw(sf::RenderTarget& target) {
        if (!isVisible || !hasFont) return;

        target.draw(backgroundDim);
        target.draw(panel);
        target.draw(panelBorder);
        target.draw(titleText);

        for (const auto& slider : sliders) {
            sf::Text labelText(font);
            labelText.setString(sf::String::fromUtf8(slider.label.begin(), slider.label.end()));
            labelText.setCharacterSize(16);
            labelText.setFillColor(sf::Color(210, 215, 225));
            labelText.setPosition(sf::Vector2f(panel.getPosition().x + 40.0f, slider.trackBounds.position.y - 5.0f));
            target.draw(labelText);

            sf::RectangleShape track(slider.trackBounds.size);
            track.setPosition(slider.trackBounds.position);
            track.setFillColor(sf::Color(40, 44, 54));
            target.draw(track);

            float currentVal = slider.targetValue ? *slider.targetValue : slider.minVal;
            float normalized = (currentVal - slider.minVal) / (slider.maxVal - slider.minVal);
            normalized = std::clamp(normalized, 0.0f, 1.0f);

            sf::RectangleShape fill(sf::Vector2f(slider.trackBounds.size.x * normalized, slider.trackBounds.size.y));
            fill.setPosition(slider.trackBounds.position);
            fill.setFillColor(sf::Color(100, 149, 237));
            target.draw(fill);

            sf::CircleShape handle(7.0f);
            handle.setOrigin(sf::Vector2f(7.0f, 7.0f));
            handle.setPosition(sf::Vector2f(
                slider.trackBounds.position.x + slider.trackBounds.size.x * normalized,
                slider.trackBounds.position.y + slider.trackBounds.size.y * 0.5f
            ));
            handle.setFillColor(slider.isDragging ? sf::Color::White : sf::Color(220, 225, 235));
            target.draw(handle);

            std::ostringstream ss;
            if (slider.isInteger) {
                ss << static_cast<int>(currentVal) << "%";
            } else {
                ss << std::fixed << std::setprecision(3) << currentVal << "s";
            }
            std::string valStr = ss.str();

            sf::Text valText(font);
            valText.setString(sf::String::fromUtf8(valStr.begin(), valStr.end()));
            valText.setCharacterSize(14);
            valText.setFillColor(sf::Color(160, 170, 185));
            valText.setPosition(sf::Vector2f(slider.trackBounds.position.x + slider.trackBounds.size.x + 18.0f, slider.trackBounds.position.y - 5.0f));
            target.draw(valText);
        }
    }
};