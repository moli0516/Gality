#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <iostream>

struct HistoryEntry {
    std::string speaker;
    std::string text;
    std::string voicePath;
};

class BacklogUI {
private:
    sf::Font font;
    bool visible = false;
    float scrollOffset = 0.0f;
    std::vector<HistoryEntry> historyEntries;

public:
    BacklogUI() = default;

    bool loadFont(const std::string& fontPath) {
        return font.openFromFile(fontPath);
    }

    void setVisible(bool show) {
        visible = show;
        if (show) scrollOffset = 0.0f; // 開啟時重置滾動
    }

    bool isVisible() const {
        return visible;
    }

    void toggle() {
        setVisible(!visible);
    }

    void addEntry(const std::string& speaker, const std::string& text, const std::string& voicePath = "") {
        historyEntries.push_back({speaker, text, voicePath});
    }

    void clear() {
        historyEntries.clear();
    }

    void handleScroll(float delta) {
        if (!visible) return;
        scrollOffset += delta * 30.0f;
        if (scrollOffset < 0.0f) scrollOffset = 0.0f;
    }

    void draw(sf::RenderWindow& window) {
        if (!visible) return;

        // 1. 半透明黑色背景遮罩
        sf::RectangleShape overlay(sf::Vector2f(window.getSize().x, window.getSize().y));
        overlay.setFillColor(sf::Color(0, 0, 0, 220));
        window.draw(overlay);

        // 2. 標題
        sf::Text titleText(font, "=== HISTORY BACKLOG (Scroll to Navigate | Right Click / Esc to Close) ===", 20);
        titleText.setFillColor(sf::Color(200, 200, 200));
        titleText.setPosition(sf::Vector2f(50.f, 30.f));
        window.draw(titleText);

        // 3. 繪製對話列表 (從最新到最舊倒序渲染)
        float startY = 80.f + scrollOffset;
        float lineSpacing = 70.f;

        for (int i = static_cast<int>(historyEntries.size()) - 1; i >= 0; --i) {
            float currentY = startY + (historyEntries.size() - 1 - i) * lineSpacing;
            
            // 超出螢幕範圍不渲染 (Culling)
            if (currentY < 70.f || currentY > window.getSize().y - 50.f) continue;

            std::string displayText = (historyEntries[i].speaker.empty() ? "" : historyEntries[i].speaker + ": ") + historyEntries[i].text;
            sf::Text entryText(font, sf::String::fromUtf8(displayText.begin(), displayText.end()), 18);
            entryText.setFillColor(sf::Color::White);
            entryText.setPosition(sf::Vector2f(70.f, currentY));
            
            window.draw(entryText);
        }
    }
};