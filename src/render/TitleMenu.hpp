#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>

struct MenuButton {
    sf::RectangleShape shape;
    sf::Text text;
    std::function<void()> callback;
    bool isHovered = false;

    MenuButton(const sf::Font& font) : text(font) {}
};

class TitleMenu {
private:
    sf::Font font;
    sf::Text titleText;
    std::vector<MenuButton> buttons;
    bool visible = true;

public:
    TitleMenu() : titleText(font) {}

    bool loadFont(const std::string& fontPath) {
        if (!font.openFromFile(fontPath)) return false;

        titleText.setFont(font);
        titleText.setString("GALITY ENGINE");
        titleText.setCharacterSize(60);
        titleText.setFillColor(sf::Color::White);
        titleText.setPosition(sf::Vector2f(440.f, 150.f));
        return true;
    }

    void initButtons(std::function<void()> onStart, std::function<void()> onLoad, std::function<void()> onExit) {
        buttons.clear();
        std::vector<std::pair<std::string, std::function<void()>>> menuItems = {
            {"START GAME", onStart},
            {"LOAD GAME",  onLoad},
            {"EXIT",       onExit}
        };

        for (size_t i = 0; i < menuItems.size(); ++i) {
            MenuButton btn(font);
            float posX = 490.f;
            float posY = 320.f + i * 70.f;

            btn.shape.setSize(sf::Vector2f(300.f, 50.f));
            btn.shape.setPosition(sf::Vector2f(posX, posY));
            btn.shape.setFillColor(sf::Color(20, 20, 40, 200));
            btn.shape.setOutlineThickness(2.f);
            btn.shape.setOutlineColor(sf::Color(100, 100, 200, 255));

            btn.text.setCharacterSize(22);
            btn.text.setFillColor(sf::Color::White);
            btn.text.setString(menuItems[i].first);

            sf::FloatRect bounds = btn.text.getLocalBounds();
            btn.text.setPosition(sf::Vector2f(posX + (300.f - bounds.size.x) / 2.f, posY + (50.f - bounds.size.y) / 2.f - 4.f));
            btn.callback = menuItems[i].second;

            buttons.push_back(btn);
        }
    }

    void setVisible(bool show) { visible = show; }
    bool isVisible() const { return visible; }

    void updateHover(sf::Vector2i mousePos) {
        if (!visible) return;
        sf::Vector2f mp(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
        for (auto& btn : buttons) {
            if (btn.shape.getGlobalBounds().contains(mp)) {
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

    bool handleClick(sf::Vector2i mousePos) {
        if (!visible) return false;
        sf::Vector2f mp(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
        for (auto& btn : buttons) {
            if (btn.shape.getGlobalBounds().contains(mp)) {
                if (btn.callback) btn.callback();
                return true;
            }
        }
        return false;
    }

    void draw(sf::RenderTarget& target) {
        if (!visible) return;
        target.draw(titleText);
        for (auto& btn : buttons) {
            target.draw(btn.shape);
            target.draw(btn.text);
        }
    }
};