#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include "../core/SaveManager.hpp"
#include "../core/AssetPack.hpp"

namespace fs = std::filesystem;

enum class SaveLoadMode {
    Save,
    Load
};

struct SlotCard {
    int slotIndex = 0;
    sf::RectangleShape box;
    bool hasData = false;
    bool isHovered = false;

    sf::Text titleText;
    sf::Text infoText;

    explicit SlotCard(int idx, const sf::Font& f) 
        : slotIndex(idx), titleText(f), infoText(f) {}
};

class SaveLoadUI {
private:
    bool visible = false;
    SaveLoadMode currentMode = SaveLoadMode::Save;
    sf::Font font;
    bool hasFont = false;

    std::vector<std::uint8_t> fontDataBuffer;

    sf::RectangleShape backgroundDim;
    sf::RectangleShape panel;
    sf::RectangleShape panelBorder;
    sf::Text headerTitle;

    std::vector<SlotCard> slots;
    const int maxSlots = 6;

    std::function<void(int)> onSaveRequested;
    std::function<void(int)> onLoadRequested;

    void ensureSlotsCreated() {
        if (slots.size() != static_cast<size_t>(maxSlots)) {
            slots.clear();
            for (int i = 1; i <= maxSlots; ++i) {
                slots.emplace_back(i, font);
            }
        }
    }

    void refreshSlotData() {
        ensureSlotsCreated();
        for (auto& slot : slots) {
            std::string filePath = "saves/save_slot_" + std::to_string(slot.slotIndex) + ".json";
            SaveSnapshot snap;
            if (fs::exists(filePath) && SaveManager::loadGame(filePath, snap)) {
                slot.hasData = true;
                std::string timeText = snap.timestamp.empty() ? "Unknown Time" : snap.timestamp;
                slot.infoText.setString(sf::String::fromUtf8(timeText.begin(), timeText.end()));
            } else {
                slot.hasData = false;
                slot.infoText.setString("[ Empty Slot ]");
            }
        }
    }

    void recalculateLayout(sf::Vector2u winSize) {
        ensureSlotsCreated();

        float winW = static_cast<float>(winSize.x);
        float winH = static_cast<float>(winSize.y);

        backgroundDim.setSize(sf::Vector2f(winW, winH));
        backgroundDim.setFillColor(sf::Color(0, 0, 0, 215));

        float panelW = 1280.0f;
        float panelH = 780.0f;
        float panelX = (winW - panelW) * 0.5f;
        float panelY = (winH - panelH) * 0.5f;

        panel.setSize(sf::Vector2f(panelW, panelH));
        panel.setPosition(sf::Vector2f(panelX, panelY));
        panel.setFillColor(sf::Color(20, 22, 30, 248));

        panelBorder.setSize(sf::Vector2f(panelW, panelH));
        panelBorder.setPosition(sf::Vector2f(panelX, panelY));
        panelBorder.setFillColor(sf::Color::Transparent);
        panelBorder.setOutlineThickness(2.5f);
        panelBorder.setOutlineColor(sf::Color(90, 105, 140, 200));

        if (hasFont) {
            std::string titleStr = (currentMode == SaveLoadMode::Save) ? "=== SYSTEM SAVE ===" : "=== SYSTEM LOAD ===";
            headerTitle.setFont(font);
            headerTitle.setString(sf::String::fromUtf8(titleStr.begin(), titleStr.end()));
            headerTitle.setCharacterSize(30);
            headerTitle.setFillColor(sf::Color(255, 215, 0));
            headerTitle.setPosition(sf::Vector2f(panelX + 50.0f, panelY + 35.0f));
        }

        float startX = panelX + 50.0f;
        float startY = panelY + 110.0f;
        float cardW = 570.0f;
        float cardH = 180.0f;
        float gapX = 40.0f;
        float gapY = 28.0f;

        for (int i = 0; i < maxSlots; ++i) {
            int col = i % 2;
            int row = i / 2;
            float cx = startX + static_cast<float>(col) * (cardW + gapX);
            float cy = startY + static_cast<float>(row) * (cardH + gapY);

            slots[i].box.setSize(sf::Vector2f(cardW, cardH));
            slots[i].box.setPosition(sf::Vector2f(cx, cy));
            
            if (hasFont) {
                std::string sTitle = "SLOT " + std::to_string(slots[i].slotIndex);
                slots[i].titleText.setFont(font);
                slots[i].titleText.setString(sf::String::fromUtf8(sTitle.begin(), sTitle.end()));
                slots[i].titleText.setCharacterSize(22);
                slots[i].titleText.setPosition(sf::Vector2f(cx + 25.0f, cy + 20.0f));

                slots[i].infoText.setFont(font);
                slots[i].infoText.setCharacterSize(18);
                slots[i].infoText.setPosition(sf::Vector2f(cx + 25.0f, cy + 65.0f));
            }
        }
    }

public:
    SaveLoadUI() : headerTitle(font) {
        ensureSlotsCreated();
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
            headerTitle.setFont(font);
            ensureSlotsCreated();
            for (auto& s : slots) {
                s.titleText.setFont(font);
                s.infoText.setFont(font);
            }
            return true;
        }
        return false;
    }

    void open(SaveLoadMode mode, sf::Vector2u winSize, std::function<void(int)> onSave, std::function<void(int)> onLoad) {
        currentMode = mode;
        onSaveRequested = onSave;
        onLoadRequested = onLoad;
        visible = true;
        ensureSlotsCreated();
        refreshSlotData();
        recalculateLayout(winSize);
    }

    void close() {
        visible = false;
    }

    bool isVisible() const {
        return visible;
    }

    void updateHover(sf::Vector2f mPos) {
        if (!visible) return;

        for (auto& slot : slots) {
            if (slot.box.getGlobalBounds().contains(mPos)) {
                slot.isHovered = true;
                slot.box.setFillColor(sf::Color(50, 60, 85, 240));
                slot.box.setOutlineThickness(2.0f);
                slot.box.setOutlineColor(sf::Color(100, 180, 255));
            } else {
                slot.isHovered = false;
                slot.box.setFillColor(slot.hasData ? sf::Color(30, 35, 48, 220) : sf::Color(25, 28, 36, 180));
                slot.box.setOutlineThickness(1.5f);
                slot.box.setOutlineColor(slot.hasData ? sf::Color(70, 85, 115) : sf::Color(45, 50, 65));
            }
        }
    }

    bool handleClick(sf::Vector2f mPos) {
        if (!visible) return false;

        for (const auto& slot : slots) {
            if (slot.box.getGlobalBounds().contains(mPos)) {
                if (currentMode == SaveLoadMode::Save) {
                    if (onSaveRequested) onSaveRequested(slot.slotIndex);
                } else {
                    if (slot.hasData && onLoadRequested) {
                        onLoadRequested(slot.slotIndex);
                    }
                }
                close();
                return true;
            }
        }

        if (!panel.getGlobalBounds().contains(mPos)) {
            close();
            return true;
        }

        return false;
    }

    void draw(sf::RenderTarget& target) {
        if (!visible || !hasFont) return;

        target.draw(backgroundDim);
        target.draw(panel);
        target.draw(panelBorder);
        target.draw(headerTitle);

        for (const auto& slot : slots) {
            target.draw(slot.box);
            target.draw(slot.titleText);
            
            sf::Text infoCopy = slot.infoText;
            if (slot.hasData) {
                infoCopy.setFillColor(sf::Color(220, 225, 235));
            } else {
                infoCopy.setFillColor(sf::Color(110, 115, 130));
            }
            target.draw(infoCopy);
        }
    }
};