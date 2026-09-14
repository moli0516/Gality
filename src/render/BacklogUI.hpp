#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <regex>
#include "../core/AssetPack.hpp"

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
    float maxScrollOffset = 0.0f;
    std::vector<HistoryEntry> historyEntries;

    // 持久保存字體二進位緩衝區，杜絕 FreeType 查表野指標
    std::vector<std::uint8_t> fontDataBuffer;

    // 清除行內標籤 (<color>, <speed>, <shake>, <w>) 確保歷史紀錄排版純淨
    static std::string stripTags(const std::string& input) {
        static const std::regex tagRegex(R"(<[^>]*>)");
        return std::regex_replace(input, tagRegex, "");
    }

    // 基於 UTF-32 與 FreeType Advance 的動態折行演算法
    std::vector<std::u32string> wrapTextU32(const std::u32string& u32Str, float maxLineWidth, unsigned int fontSize) {
        std::vector<std::u32string> lines;
        if (u32Str.empty()) return lines;

        std::u32string currentLine;
        float currentLineWidth = 0.0f;

        for (size_t i = 0; i < u32Str.size(); ++i) {
            char32_t cp = u32Str[i];

            if (cp == U'\n') {
                lines.push_back(currentLine);
                currentLine.clear();
                currentLineWidth = 0.0f;
                continue;
            }

            const auto& glyph = font.getGlyph(cp, fontSize, false);
            float advance = glyph.advance > 0.0f ? glyph.advance : static_cast<float>(fontSize);

            if (currentLineWidth + advance > maxLineWidth && !currentLine.empty()) {
                lines.push_back(currentLine);
                currentLine.clear();
                currentLineWidth = 0.0f;
            }

            currentLine += cp;
            currentLineWidth += advance;
        }

        if (!currentLine.empty()) {
            lines.push_back(currentLine);
        }

        return lines;
    }

public:
    BacklogUI() = default;

    bool loadFont(const std::string& fontPath) {
        fontDataBuffer.clear();
        if (AssetPack::readFileFromPak(fontPath, fontDataBuffer, "data.pak") && !fontDataBuffer.empty()) {
            return font.openFromMemory(fontDataBuffer.data(), fontDataBuffer.size());
        }
        return font.openFromFile(fontPath);
    }

    void setVisible(bool show) {
        visible = show;
        if (show) {
            // 開啟時預設滾動至最底部（最新對話）
            scrollOffset = maxScrollOffset;
        }
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
        scrollOffset = 0.0f;
        maxScrollOffset = 0.0f;
    }

    void handleScroll(float delta) {
        if (!visible) return;
        // delta > 0: 向上滾動檢視舊紀錄；delta < 0: 向下滾動檢視新紀錄
        scrollOffset -= delta * 45.0f;
        if (scrollOffset < 0.0f) scrollOffset = 0.0f;
        if (scrollOffset > maxScrollOffset) scrollOffset = maxScrollOffset;
    }

    void draw(sf::RenderTarget& target) {
        if (!visible) return;

        sf::Vector2f targetSize(static_cast<float>(target.getSize().x), static_cast<float>(target.getSize().y));

        // 1. 全螢幕半透明遮罩背景
        sf::RectangleShape overlay(targetSize);
        overlay.setFillColor(sf::Color(10, 10, 15, 235));
        target.draw(overlay);

        // 2. 標題與操作提示
        sf::Text titleText(font, "=== HISTORY BACKLOG (Scroll to Navigate | Right Click / Esc / H to Close) ===", 18);
        titleText.setFillColor(sf::Color(180, 185, 200));
        titleText.setPosition(sf::Vector2f(50.f, 25.f));
        target.draw(titleText);

        if (historyEntries.empty()) {
            sf::Text emptyText(font, "No history recorded yet.", 18);
            emptyText.setFillColor(sf::Color(120, 120, 130));
            emptyText.setPosition(sf::Vector2f(60.f, 80.f));
            target.draw(emptyText);
            return;
        }

        const float paddingX = 60.0f;
        const float maxLineWidth = targetSize.x - paddingX * 2.0f - 40.0f;
        const unsigned int fontSize = 18;
        const float lineHeight = fontSize * 1.5f;
        const float entrySpacing = 28.0f;
        const float topMargin = 75.0f;
        const float bottomMargin = 40.0f;
        const float visibleAreaHeight = targetSize.y - topMargin - bottomMargin;

        // 3. 測量所有條目尺寸（純 UTF-32 計算，杜絕 ANSI 代碼頁污染）
        struct MeasuredEntry {
            std::u32string speaker;
            std::vector<std::u32string> lines;
            float totalHeight = 0.0f;
        };

        std::vector<MeasuredEntry> measuredEntries;
        measuredEntries.reserve(historyEntries.size());
        float totalDocumentHeight = 0.0f;

        for (const auto& entry : historyEntries) {
            MeasuredEntry me;
            if (!entry.speaker.empty()) {
                sf::String sfSpk = sf::String::fromUtf8(entry.speaker.begin(), entry.speaker.end());
                me.speaker = U"[" + sfSpk.toUtf32() + U"]";
            }

            std::string cleanText = stripTags(entry.text);
            sf::String sfClean = sf::String::fromUtf8(cleanText.begin(), cleanText.end());
            me.lines = wrapTextU32(sfClean.toUtf32(), maxLineWidth, fontSize);

            float h = 0.0f;
            if (!me.speaker.empty()) {
                h += lineHeight;
            }
            h += static_cast<float>(me.lines.size()) * lineHeight;
            h += entrySpacing;

            me.totalHeight = h;
            totalDocumentHeight += h;
            measuredEntries.push_back(std::move(me));
        }

        // 4. 動態邊界鎖定
        maxScrollOffset = std::max(0.0f, totalDocumentHeight - visibleAreaHeight);
        if (scrollOffset > maxScrollOffset) scrollOffset = maxScrollOffset;
        if (scrollOffset < 0.0f) scrollOffset = 0.0f;

        // 5. 視錐裁剪渲染（Frustum Culling）
        float cursorY = topMargin - scrollOffset;

        for (const auto& me : measuredEntries) {
            float entryTop = cursorY;
            float entryBottom = cursorY + me.totalHeight;

            // 位於可視範圍內才進行 GPU 頂點繪製
            if (entryBottom >= topMargin && entryTop <= targetSize.y - bottomMargin) {
                float currentRenderY = cursorY;

                // 發言人名稱
                if (!me.speaker.empty()) {
                    if (currentRenderY >= topMargin - lineHeight && currentRenderY <= targetSize.y - bottomMargin) {
                        sf::Text spkText(font, sf::String(me.speaker), fontSize);
                        spkText.setFillColor(sf::Color(255, 215, 0, 255));
                        spkText.setPosition(sf::Vector2f(paddingX, currentRenderY));
                        target.draw(spkText);
                    }
                    currentRenderY += lineHeight;
                }

                // 正文折行文本
                for (const auto& lineU32 : me.lines) {
                    if (currentRenderY >= topMargin - lineHeight && currentRenderY <= targetSize.y - bottomMargin) {
                        sf::Text lineText(font, sf::String(lineU32), fontSize);
                        lineText.setFillColor(sf::Color(240, 240, 245, 255));
                        lineText.setPosition(sf::Vector2f(paddingX + 20.0f, currentRenderY));
                        target.draw(lineText);
                    }
                    currentRenderY += lineHeight;
                }
            }

            cursorY += me.totalHeight;
        }

        // 6. 右側滾動進度條
        if (maxScrollOffset > 0.0f) {
            float scrollbarWidth = 6.0f;
            float scrollbarTrackX = targetSize.x - 20.0f;
            float scrollbarTrackY = topMargin;
            float scrollbarTrackH = visibleAreaHeight;

            sf::RectangleShape track(sf::Vector2f(scrollbarWidth, scrollbarTrackH));
            track.setPosition(sf::Vector2f(scrollbarTrackX, scrollbarTrackY));
            track.setFillColor(sf::Color(40, 40, 50, 180));
            target.draw(track);

            float thumbH = std::max(30.0f, (visibleAreaHeight / totalDocumentHeight) * scrollbarTrackH);
            float thumbY = scrollbarTrackY + (scrollOffset / maxScrollOffset) * (scrollbarTrackH - thumbH);

            sf::RectangleShape thumb(sf::Vector2f(scrollbarWidth, thumbH));
            thumb.setPosition(sf::Vector2f(scrollbarTrackX, thumbY));
            thumb.setFillColor(sf::Color(100, 149, 237, 220));
            target.draw(thumb);
        }
    }
};