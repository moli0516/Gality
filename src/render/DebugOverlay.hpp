#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include "../core/Blackboard.hpp"
#include "../core/AssetPack.hpp"
#include "../story/StoryExecutor.hpp"
#include "../story/StoryNode.hpp"

class DebugOverlay {
private:
    bool isVisible = false;
    sf::Font font;
    bool hasFont = false;

    // 持久保存字體二進位緩衝區，杜絕 FreeType 野指標光柵化失敗
    std::vector<std::uint8_t> fontDataBuffer;

    enum class ActiveTab {
        Blackboard,
        NodeJumper,
        Diagnostics,
        Console
    };
    ActiveTab currentTab = ActiveTab::Blackboard;

    std::vector<std::string> logHistory;
    float scrollOffset = 0.0f;
    float maxScrollOffset = 0.0f;

    sf::RectangleShape panelBg;
    sf::RectangleShape panelBorder;

    struct TabButton {
        ActiveTab tab;
        std::string name;
        sf::FloatRect bounds;
    };
    std::vector<TabButton> tabButtons;

    // --- Node Jumper State ---
    struct NodeEntry {
        std::string id;
        std::string typeDesc;
        std::shared_ptr<StoryNode> nodePtr;
        sf::FloatRect bounds;
    };
    std::vector<NodeEntry> cachedNodeEntries;
    int selectedNodeIndex = 0;
    bool isDraggingScrollbar = false;
    float scrollbarDragStartY = 0.0f;
    float scrollbarDragStartOffset = 0.0f;
    sf::FloatRect scrollbarThumbBounds;
    sf::FloatRect scrollbarTrackBounds;

    // --- Blackboard Interactive Input State ---
    std::string editingVarKey = "";
    std::string inputBuffer = "";
    bool isInputActive = false;
    sf::FloatRect inputBoxBounds;
    sf::FloatRect applyBtnBounds;
    struct BlackboardRow {
        std::string key;
        int value;
        sf::FloatRect bounds;
    };
    std::vector<BlackboardRow> cachedBbRows;

    void updateLayout(const sf::Vector2u& /*winSize*/) {
        float panelW = 640.0f;
        float panelH = 500.0f;
        panelBg.setSize(sf::Vector2f(panelW, panelH));
        panelBg.setPosition(sf::Vector2f(20.0f, 20.0f));
        panelBg.setFillColor(sf::Color(15, 17, 22, 240));

        panelBorder.setSize(sf::Vector2f(panelW, panelH));
        panelBorder.setPosition(sf::Vector2f(20.0f, 20.0f));
        panelBorder.setFillColor(sf::Color::Transparent);
        panelBorder.setOutlineThickness(1.5f);
        panelBorder.setOutlineColor(sf::Color(80, 90, 115, 200));

        tabButtons.clear();
        std::vector<std::pair<ActiveTab, std::string>> tabs = {
            {ActiveTab::Blackboard, "Blackboard"},
            {ActiveTab::NodeJumper, "Node Jumper"},
            {ActiveTab::Diagnostics, "Diagnostics"},
            {ActiveTab::Console, "Console"}
        };

        float tabX = 35.0f;
        float tabY = 32.0f;
        float tabW = 125.0f;
        float tabH = 30.0f;

        for (const auto& [tabEnum, tabName] : tabs) {
            tabButtons.push_back({tabEnum, tabName, sf::FloatRect(sf::Vector2f(tabX, tabY), sf::Vector2f(tabW, tabH))});
            tabX += tabW + 8.0f;
        }

        float trackX = panelBg.getPosition().x + panelBg.getSize().x - 22.0f;
        float trackY = 80.0f;
        float trackH = panelBg.getSize().y - 100.0f;
        scrollbarTrackBounds = sf::FloatRect(sf::Vector2f(trackX, trackY), sf::Vector2f(8.0f, trackH));
    }

    void clampScroll() {
        if (scrollOffset < 0.0f) scrollOffset = 0.0f;
        if (scrollOffset > maxScrollOffset) scrollOffset = maxScrollOffset;
    }

public:
    DebugOverlay() = default;

    bool loadFont(const std::string& fontPath) {
        fontDataBuffer.clear();
        if (AssetPack::readFileFromPak(fontPath, fontDataBuffer, "data.pak") && !fontDataBuffer.empty()) {
            if (font.openFromMemory(fontDataBuffer.data(), fontDataBuffer.size())) {
                hasFont = true;
                updateLayout(sf::Vector2u(1280, 720));
                return true;
            }
        } else if (font.openFromFile(fontPath)) {
            hasFont = true;
            updateLayout(sf::Vector2u(1280, 720));
            return true;
        }
        hasFont = false;
        return false;
    }

    void toggle() {
        isVisible = !isVisible;
        scrollOffset = 0.0f;
        isInputActive = false;
        isDraggingScrollbar = false;
    }

    bool getIsVisible() const {
        return isVisible;
    }

    void log(const std::string& message) {
        logHistory.push_back(message);
        if (logHistory.size() > 100) {
            logHistory.erase(logHistory.begin());
        }
    }

    void handleScroll(float delta) {
        if (!isVisible) return;
        scrollOffset -= delta * 35.0f;
        clampScroll();
    }

    void handleTextEntered(char32_t unicode, Blackboard& blackboard) {
        if (!isVisible || !isInputActive || currentTab != ActiveTab::Blackboard) return;

        if (unicode == 13) { 
            commitInputValue(blackboard);
            return;
        }
        if (unicode == 8) { 
            if (!inputBuffer.empty()) {
                inputBuffer.pop_back();
            }
            return;
        }
        if (unicode == 27) { 
            isInputActive = false;
            return;
        }

        if ((unicode >= '0' && unicode <= '9') || (unicode == '-' && inputBuffer.empty())) {
            if (inputBuffer.size() < 10) {
                inputBuffer += static_cast<char>(unicode);
            }
        }
    }

    bool handleKeyPressed(sf::Keyboard::Key key, StoryExecutor& executor, 
                          const std::unordered_map<std::string, std::shared_ptr<StoryNode>>& nodeIndexMap,
                          std::function<void()> onNodeJumped,
                          Blackboard& blackboard) 
    {
        if (!isVisible) return false;

        if (currentTab == ActiveTab::Blackboard) {
            if (isInputActive && key == sf::Keyboard::Key::Enter) {
                commitInputValue(blackboard);
                return true;
            }
        }

        if (currentTab == ActiveTab::NodeJumper) {
            int totalNodes = static_cast<int>(nodeIndexMap.size());
            if (totalNodes == 0) return false;

            if (key == sf::Keyboard::Key::Up) {
                selectedNodeIndex = std::max(0, selectedNodeIndex - 1);
                ensureNodeVisible(selectedNodeIndex);
                return true;
            }
            if (key == sf::Keyboard::Key::Down) {
                selectedNodeIndex = std::min(totalNodes - 1, selectedNodeIndex + 1);
                ensureNodeVisible(selectedNodeIndex);
                return true;
            }
            if (key == sf::Keyboard::Key::Enter) {
                if (selectedNodeIndex >= 0 && selectedNodeIndex < static_cast<int>(cachedNodeEntries.size())) {
                    const auto& targetEntry = cachedNodeEntries[selectedNodeIndex];
                    if (targetEntry.nodePtr) {
                        executor.jumpToNode(targetEntry.nodePtr);
                        log("[Debug] Jumped to node: " + targetEntry.id);
                        if (onNodeJumped) onNodeJumped();
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool handleMouseClick(const sf::Vector2i& mousePos, StoryExecutor& executor,
                          Blackboard& blackboard,
                          std::function<void()> onNodeJumped) 
    {
        if (!isVisible) return false;
        sf::Vector2f mPos(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

        for (const auto& btn : tabButtons) {
            if (btn.bounds.contains(mPos)) {
                currentTab = btn.tab;
                scrollOffset = 0.0f;
                isInputActive = false;
                return true;
            }
        }

        if (maxScrollOffset > 0.0f) {
            if (scrollbarThumbBounds.contains(mPos)) {
                isDraggingScrollbar = true;
                scrollbarDragStartY = mPos.y;
                scrollbarDragStartOffset = scrollOffset;
                return true;
            }
            if (scrollbarTrackBounds.contains(mPos)) {
                float clickRatio = (mPos.y - scrollbarTrackBounds.position.y) / scrollbarTrackBounds.size.y;
                scrollOffset = clickRatio * maxScrollOffset;
                clampScroll();
                return true;
            }
        }

        if (currentTab == ActiveTab::Blackboard) {
            if (applyBtnBounds.contains(mPos)) {
                commitInputValue(blackboard);
                return true;
            }
            if (inputBoxBounds.contains(mPos)) {
                isInputActive = true;
                return true;
            }
            for (const auto& row : cachedBbRows) {
                if (row.bounds.contains(mPos)) {
                    editingVarKey = row.key;
                    inputBuffer = std::to_string(row.value);
                    isInputActive = true;
                    return true;
                }
            }
        }

        if (currentTab == ActiveTab::NodeJumper) {
            for (size_t i = 0; i < cachedNodeEntries.size(); ++i) {
                if (cachedNodeEntries[i].bounds.contains(mPos)) {
                    selectedNodeIndex = static_cast<int>(i);
                    executor.jumpToNode(cachedNodeEntries[i].nodePtr);
                    log("[Debug] Jumped to node: " + cachedNodeEntries[i].id);
                    if (onNodeJumped) onNodeJumped();
                    return true;
                }
            }
        }

        return panelBg.getGlobalBounds().contains(mPos);
    }

    void handleMouseMove(const sf::Vector2i& mousePos) {
        if (!isVisible || !isDraggingScrollbar || maxScrollOffset <= 0.0f) return;

        float deltaY = static_cast<float>(mousePos.y) - scrollbarDragStartY;
        float trackEffectiveH = scrollbarTrackBounds.size.y - scrollbarThumbBounds.size.y;
        if (trackEffectiveH > 0.0f) {
            float deltaOffset = (deltaY / trackEffectiveH) * maxScrollOffset;
            scrollOffset = scrollbarDragStartOffset + deltaOffset;
            clampScroll();
        }
    }

    void handleMouseRelease() {
        isDraggingScrollbar = false;
    }

private:
    void commitInputValue(Blackboard& blackboard) {
        if (!editingVarKey.empty()) {
            try {
                int newVal = inputBuffer.empty() ? 0 : std::stoi(inputBuffer);
                blackboard.setInt(editingVarKey, newVal);
                log("[Debug] Set " + editingVarKey + " = " + std::to_string(newVal));
            } catch (...) {
                log("[Debug Error] Invalid integer conversion: " + inputBuffer);
            }
        }
        isInputActive = false;
    }

    void ensureNodeVisible(int index) {
        float rowHeight = 24.0f;
        float itemTop = index * rowHeight;
        float itemBottom = itemTop + rowHeight;
        float viewH = panelBg.getSize().y - 120.0f;

        if (itemTop < scrollOffset) {
            scrollOffset = itemTop;
        } else if (itemBottom > scrollOffset + viewH) {
            scrollOffset = itemBottom - viewH;
        }
        clampScroll();
    }

public:
    void draw(sf::RenderTarget& target, Blackboard& blackboard, StoryExecutor& executor,
              const std::unordered_map<std::string, std::shared_ptr<StoryNode>>& nodeIndexMap,
              float fps) 
    {
        if (!isVisible || !hasFont) return;

        target.draw(panelBg);
        target.draw(panelBorder);

        for (const auto& btn : tabButtons) {
            sf::RectangleShape tabShape(btn.bounds.size);
            tabShape.setPosition(btn.bounds.position);
            bool isActive = (btn.tab == currentTab);
            tabShape.setFillColor(isActive ? sf::Color(45, 52, 70) : sf::Color(24, 27, 34));
            tabShape.setOutlineThickness(1.0f);
            tabShape.setOutlineColor(isActive ? sf::Color(100, 149, 237) : sf::Color(60, 65, 80));
            target.draw(tabShape);

            sf::Text tabText(font);
            tabText.setString(sf::String::fromUtf8(btn.name.begin(), btn.name.end()));
            tabText.setCharacterSize(14);
            tabText.setFillColor(isActive ? sf::Color::White : sf::Color(180, 185, 200));
            tabText.setPosition(sf::Vector2f(btn.bounds.position.x + 12.0f, btn.bounds.position.y + 6.0f));
            target.draw(tabText);
        }

        float contentX = panelBg.getPosition().x + 25.0f;
        float contentY = 78.0f;
        float contentW = panelBg.getSize().x - 60.0f;
        float viewH = panelBg.getSize().y - 100.0f;

        if (currentTab == ActiveTab::Blackboard) {
            cachedBbRows.clear();
            const auto& intFlags = blackboard.getAllInts();

            sf::Text editLabel(font);
            editLabel.setCharacterSize(14);
            editLabel.setFillColor(sf::Color(255, 215, 0));
            std::string editHead = "Active Variable: [" + (editingVarKey.empty() ? "None Selected" : editingVarKey) + "]";
            editLabel.setString(sf::String::fromUtf8(editHead.begin(), editHead.end()));
            editLabel.setPosition(sf::Vector2f(contentX, contentY));
            target.draw(editLabel);

            float boxY = contentY + 24.0f;
            inputBoxBounds = sf::FloatRect(sf::Vector2f(contentX, boxY), sf::Vector2f(180.0f, 26.0f));
            sf::RectangleShape inputBox(inputBoxBounds.size);
            inputBox.setPosition(inputBoxBounds.position);
            inputBox.setFillColor(isInputActive ? sf::Color(35, 40, 52) : sf::Color(25, 28, 36));
            inputBox.setOutlineThickness(1.0f);
            inputBox.setOutlineColor(isInputActive ? sf::Color(100, 149, 237) : sf::Color(70, 75, 90));
            target.draw(inputBox);

            std::string displayInput = inputBuffer + (isInputActive ? "_" : "");
            sf::Text inputText(font);
            inputText.setCharacterSize(14);
            inputText.setFillColor(sf::Color::White);
            inputText.setString(sf::String::fromUtf8(displayInput.begin(), displayInput.end()));
            inputText.setPosition(sf::Vector2f(contentX + 8.0f, boxY + 4.0f));
            target.draw(inputText);

            applyBtnBounds = sf::FloatRect(sf::Vector2f(contentX + 190.0f, boxY), sf::Vector2f(70.0f, 26.0f));
            sf::RectangleShape applyBtn(applyBtnBounds.size);
            applyBtn.setPosition(applyBtnBounds.position);
            applyBtn.setFillColor(sf::Color(45, 80, 140));
            applyBtn.setOutlineThickness(1.0f);
            applyBtn.setOutlineColor(sf::Color(100, 149, 237));
            target.draw(applyBtn);

            sf::Text applyText(font);
            applyText.setCharacterSize(13);
            applyText.setFillColor(sf::Color::White);
            applyText.setString("SET");
            applyText.setPosition(sf::Vector2f(applyBtnBounds.position.x + 22.0f, boxY + 5.0f));
            target.draw(applyText);

            float listStartY = boxY + 36.0f;
            sf::Text listHeader(font);
            listHeader.setCharacterSize(13);
            listHeader.setFillColor(sf::Color(160, 170, 185));
            listHeader.setString("Click any row below to load into editor:");
            listHeader.setPosition(sf::Vector2f(contentX, listStartY));
            target.draw(listHeader);

            float rowY = listStartY + 22.0f;
            float rowH = 22.0f;
            float totalHeight = (listStartY - contentY) + static_cast<float>(intFlags.size()) * rowH;
            maxScrollOffset = std::max(0.0f, totalHeight - viewH);

            float cursorY = rowY - scrollOffset;

            for (const auto& [key, val] : intFlags) {
                if (cursorY + rowH >= listStartY && cursorY <= panelBg.getPosition().y + panelBg.getSize().y - 15.0f) {
                    sf::FloatRect rBounds(sf::Vector2f(contentX, cursorY), sf::Vector2f(contentW, rowH));
                    cachedBbRows.push_back({key, val, rBounds});

                    bool isSelected = (key == editingVarKey);
                    if (isSelected) {
                        sf::RectangleShape selBg(sf::Vector2f(contentW, rowH));
                        selBg.setPosition(sf::Vector2f(contentX, cursorY));
                        selBg.setFillColor(sf::Color(45, 60, 90, 160));
                        target.draw(selBg);
                    }

                    sf::Text rowText(font);
                    rowText.setCharacterSize(14);
                    rowText.setFillColor(isSelected ? sf::Color(255, 215, 0) : sf::Color(215, 220, 235));
                    std::string line = (isSelected ? " > " : "   ") + key + " = " + std::to_string(val);
                    rowText.setString(sf::String::fromUtf8(line.begin(), line.end()));
                    rowText.setPosition(sf::Vector2f(contentX, cursorY));
                    target.draw(rowText);
                }
                cursorY += rowH;
            }
        } 
        else if (currentTab == ActiveTab::NodeJumper) {
            cachedNodeEntries.clear();
            auto currentNode = executor.getCurrentNode();
            std::string currentId = currentNode ? currentNode->id : "NULL";

            std::vector<std::pair<std::string, std::shared_ptr<StoryNode>>> sortedNodes(nodeIndexMap.begin(), nodeIndexMap.end());
            std::sort(sortedNodes.begin(), sortedNodes.end(), [](const auto& a, const auto& b) {
                return a.first < b.first;
            });

            sf::Text header(font);
            header.setCharacterSize(14);
            header.setFillColor(sf::Color(255, 215, 0));
            std::ostringstream ss;
            ss << "Active: " << currentId << " | UP/DOWN: Select | ENTER: Jump";
            std::string headStr = ss.str();
            header.setString(sf::String::fromUtf8(headStr.begin(), headStr.end()));
            header.setPosition(sf::Vector2f(contentX, contentY));
            target.draw(header);

            float rowStartY = contentY + 26.0f;
            float rowHeight = 24.0f;
            float totalHeight = static_cast<float>(sortedNodes.size()) * rowHeight;
            maxScrollOffset = std::max(0.0f, totalHeight - (viewH - 30.0f));

            float cursorY = rowStartY - scrollOffset;
            int idx = 0;

            for (const auto& [id, nodePtr] : sortedNodes) {
                sf::FloatRect bounds(sf::Vector2f(contentX, cursorY), sf::Vector2f(contentW - 20.0f, rowHeight));
                cachedNodeEntries.push_back({id, "", nodePtr, bounds});

                if (cursorY + rowHeight >= rowStartY && cursorY <= panelBg.getPosition().y + panelBg.getSize().y - 15.0f) {
                    bool isCursor = (idx == selectedNodeIndex);
                    bool isPlaying = (id == currentId);

                    if (isCursor || isPlaying) {
                        sf::RectangleShape highlight(sf::Vector2f(contentW - 20.0f, rowHeight - 2.0f));
                        highlight.setPosition(sf::Vector2f(contentX, cursorY + 1.0f));
                        if (isCursor && isPlaying) highlight.setFillColor(sf::Color(45, 90, 60, 200));
                        else if (isCursor) highlight.setFillColor(sf::Color(55, 65, 85, 200));
                        else highlight.setFillColor(sf::Color(35, 70, 50, 150));
                        target.draw(highlight);
                    }

                    sf::Text rowText(font);
                    rowText.setCharacterSize(13);
                    if (isPlaying) rowText.setFillColor(sf::Color(120, 240, 140));
                    else if (isCursor) rowText.setFillColor(sf::Color(255, 255, 255));
                    else rowText.setFillColor(sf::Color(190, 195, 210));

                    std::string label = (isPlaying ? " [*] " : "     ") + id + " [" + 
                        (nodePtr->type == NodeType::Dialogue ? "Dialogue" : 
                         nodePtr->type == NodeType::Choice   ? "Choice" : 
                         nodePtr->type == NodeType::Condition ? "Condition" : "Action") + "]";
                    rowText.setString(sf::String::fromUtf8(label.begin(), label.end()));
                    rowText.setPosition(sf::Vector2f(contentX, cursorY + 2.0f));
                    target.draw(rowText);
                }
                cursorY += rowHeight;
                idx++;
            }
        } 
        else if (currentTab == ActiveTab::Diagnostics) {
            maxScrollOffset = 0.0f;
            std::ostringstream ss;
            ss << "Engine Runtime Diagnostics:\n\n"
               << "  Target Refresh Rate : 60.0 FPS\n"
               << "  Current Framerate   : " << std::fixed << std::setprecision(1) << fps << " FPS\n"
               << "  Frame Latency       : " << std::setprecision(3) << (fps > 0.0f ? 1000.0f / fps : 0.0f) << " ms\n"
               << "  Viewport Dimensions : 1280 x 720 (Window Fixed)\n"
               << "  Rendering Pipeline  :\n"
               << "    - Dual-Buffer Offscreen Compositor\n"
               << "    - Easing Grayscale Transition Masking\n"
               << "    - PostFX Multi-Pass Fragment Shaders\n"
               << "    - Rollback State Snapshot Memory Stack\n"
               << "    - Native Pure Vector Subsystem";

            std::string diagStr = ss.str();
            sf::Text text(font);
            text.setCharacterSize(14);
            text.setFillColor(sf::Color(220, 225, 235));
            text.setString(sf::String::fromUtf8(diagStr.begin(), diagStr.end()));
            text.setPosition(sf::Vector2f(contentX, contentY + 10.0f));
            target.draw(text);
        } 
        else if (currentTab == ActiveTab::Console) {
            float rowY = contentY + 10.0f;
            float totalHeight = static_cast<float>(logHistory.size()) * 22.0f;
            maxScrollOffset = std::max(0.0f, totalHeight - viewH);

            float cursorY = rowY - scrollOffset;
            for (const auto& logMsg : logHistory) {
                if (cursorY + 22.0f >= rowY && cursorY <= panelBg.getPosition().y + panelBg.getSize().y - 15.0f) {
                    sf::Text text(font);
                    text.setCharacterSize(13);
                    text.setFillColor(sf::Color(160, 205, 245));
                    text.setString(sf::String::fromUtf8(logMsg.begin(), logMsg.end()));
                    text.setPosition(sf::Vector2f(contentX, cursorY));
                    target.draw(text);
                }
                cursorY += 22.0f;
            }
        }

        if (maxScrollOffset > 0.0f) {
            sf::RectangleShape track(scrollbarTrackBounds.size);
            track.setPosition(scrollbarTrackBounds.position);
            track.setFillColor(sf::Color(30, 33, 42));
            target.draw(track);

            float trackH = scrollbarTrackBounds.size.y;
            float totalContentH = maxScrollOffset + trackH;
            float thumbH = std::max(25.0f, (trackH / totalContentH) * trackH);
            float thumbY = scrollbarTrackBounds.position.y + (scrollOffset / maxScrollOffset) * (trackH - thumbH);

            scrollbarThumbBounds = sf::FloatRect(sf::Vector2f(scrollbarTrackBounds.position.x, thumbY), 
                                                 sf::Vector2f(scrollbarTrackBounds.size.x, thumbH));

            sf::RectangleShape thumb(scrollbarThumbBounds.size);
            thumb.setPosition(scrollbarThumbBounds.position);
            thumb.setFillColor(isDraggingScrollbar ? sf::Color(140, 180, 255) : sf::Color(90, 120, 180));
            target.draw(thumb);
        }
    }
};