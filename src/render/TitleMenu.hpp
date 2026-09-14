#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>
#include "../core/AssetPack.hpp"

struct MenuButton {
    sf::RectangleShape shape;
    sf::Text text;
    std::function<void()> callback;
    bool isHovered = false;

    explicit MenuButton(const sf::Font& font) : text(font) {}
};

class TitleMenu {
private:
    sf::Font font;
    bool hasFont = false;
    std::vector<std::uint8_t> fontBytes;
    sf::Text titleText;
    std::vector<MenuButton> buttons;
    bool visible = true;

    std::function<void()> onStartCb;
    std::function<void()> onLoadCb;
    std::function<void()> onExitCb;

    void rebuildButtons() {
        if (!hasFont) return;
        buttons.clear();

        std::vector<std::pair<std::string, std::function<void()>>> menuItems = {
            {"START GAME", onStartCb},
            {"LOAD GAME",  onLoadCb},
            {"EXIT",       onExitCb}
        };

        const float btnWidth = 400.f;
        const float btnHeight = 65.f;
        const float posX = (1920.f - btnWidth) * 0.5f;
        const float startY = 460.f;
        const float spacing = 95.f;

        for (size_t i = 0; i < menuItems.size(); ++i) {
            MenuButton btn(font);
            float posY = startY + static_cast<float>(i) * spacing;

            btn.shape.setSize(sf::Vector2f(btnWidth, btnHeight));
            btn.shape.setPosition(sf::Vector2f(posX, posY));
            btn.shape.setFillColor(sf::Color(20, 20, 40, 200));
            btn.shape.setOutlineThickness(2.5f);
            btn.shape.setOutlineColor(sf::Color(100, 100, 200, 255));

            btn.text.setFont(font);
            btn.text.setCharacterSize(28);
            btn.text.setFillColor(sf::Color::White);
            btn.text.setString(menuItems[i].first);

            sf::FloatRect bounds = btn.text.getLocalBounds();
            btn.text.setPosition(sf::Vector2f(
                posX + (btnWidth - bounds.size.x) * 0.5f, 
                posY + (btnHeight - bounds.size.y) * 0.5f - 6.f
            ));
            btn.callback = menuItems[i].second;

            buttons.push_back(std::move(btn));
        }
    }

public:
    TitleMenu() : titleText(font) {}

    bool loadFont(const std::string& fontPath) {
        fontBytes.clear();
        hasFont = false;

        if (AssetPack::readFileFromPak(fontPath, fontBytes, "data.pak") && !fontBytes.empty()) {
            if (font.openFromMemory(fontBytes.data(), fontBytes.size())) {
                hasFont = true;
            }
        } else if (font.openFromFile(fontPath)) {
            hasFont = true;
        }

        if (hasFont) {
            titleText.setFont(font);
            titleText.setString("GALITY ENGINE");
            titleText.setCharacterSize(84);
            titleText.setFillColor(sf::Color::White);
            
            sf::FloatRect bounds = titleText.getLocalBounds();
            titleText.setPosition(sf::Vector2f((1920.f - bounds.size.x) * 0.5f, 200.f));
            rebuildButtons();
            return true;
        }

        return false;
    }

    void initButtons(std::function<void()> onStart, std::function<void()> onLoad, std::function<void()> onExit) {
        onStartCb = onStart;
        onLoadCb = onLoad;
        onExitCb = onExit;
        rebuildButtons();
    }

    void setVisible(bool show) { visible = show; }
    bool isVisible() const { return visible; }

    void updateHover(sf::Vector2f mousePosF) {
        if (!visible) return;
        for (auto& btn : buttons) {
            if (btn.shape.getGlobalBounds().contains(mousePosF)) {
                btn.isHovered = true;
                btn.shape.setFillColor(sf::Color(60, 60, 120, 240));
                btn.shape.setOutlineColor(sf::Color(255, 215, 0, 255));
            } else {
                btn.isHovered = false;
                btn.shape.setFillColor(sf::Color(20, 20, 40, 200));
                btn.shape.setOutlineColor(sf::Color(100, 100, 200, 255));
            }
        }
    }

    bool handleClick(sf::Vector2f mousePosF) {
        if (!visible) return false;
        for (auto& btn : buttons) {
            if (btn.shape.getGlobalBounds().contains(mousePosF)) {
                if (btn.callback) btn.callback();
                return true;
            }
        }
        return false;
    }

    void draw(sf::RenderTarget& target) {
        if (!visible || !hasFont) return;
        target.draw(titleText);
        for (auto& btn : buttons) {
            target.draw(btn.shape);
            target.draw(btn.text);
        }
    }
};