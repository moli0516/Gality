#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <queue>
#include <functional>
#include <algorithm>
#include <cmath>
#include <sstream>
#include "../story/StoryNode.hpp"
#include "../story/StoryExecutor.hpp"
#include "../core/AssetPack.hpp"

class NodeGraphViewer {
private:
    bool isVisible = false;
    sf::Font font;
    bool hasFont = false;

    // 持久保存字體二進位緩衝區，杜絕 FreeType 查表懸空崩潰
    std::vector<std::uint8_t> fontDataBuffer;

    // 視圖座標轉換參數
    sf::Vector2f viewOffset{100.0f, 150.0f};
    float zoomScale = 0.85f;
    bool isPanning = false;
    sf::Vector2i panStartMousePos{0, 0};

    std::string selectedNodeId = "";
    std::shared_ptr<StoryNode> selectedNode = nullptr;
    std::string cachedCurrentNodeId = "";
    std::unordered_map<std::string, sf::Vector2f> nodePositions;
    bool needsLayout = true;

    // 節點卡片標準尺寸
    const float baseCardWidth = 240.0f;
    const float baseCardHeight = 80.0f;
    const float levelSpacingX = 360.0f;
    const float siblingSpacingY = 120.0f;

    // --- Live Inspector State (純 SFML 原生即時屬性檢視與編輯器) ---
    enum class EditField { None, Speaker, Text, Bg, Bgm, Cv };
    EditField activeField = EditField::None;
    std::string activeInputBuffer = "";

    sf::FloatRect speakerBoxBounds;
    sf::FloatRect textBoxBounds;
    sf::FloatRect bgBoxBounds;
    sf::FloatRect bgmBoxBounds;
    sf::FloatRect cvBoxBounds;

    // ------------------------------------------------------------------------
    // 樹狀拓撲演算法 (Hierarchical Tree Layout via Cycle-Safe BFS)
    // ------------------------------------------------------------------------
    void computeTreeLayout(const std::unordered_map<std::string, std::shared_ptr<StoryNode>>& nodeRegistry,
                           const std::string& startNodeId) 
    {
        nodePositions.clear();
        if (nodeRegistry.empty()) return;

        std::string rootId = startNodeId;
        if (rootId.empty() || nodeRegistry.find(rootId) == nodeRegistry.end()) {
            rootId = "node_01";
            if (nodeRegistry.find(rootId) == nodeRegistry.end()) {
                rootId = nodeRegistry.begin()->first;
            }
        }

        std::unordered_map<std::string, int> nodeLevel;
        std::unordered_set<std::string> visited;
        std::queue<std::string> bfsQueue;

        nodeLevel[rootId] = 0;
        visited.insert(rootId);
        bfsQueue.push(rootId);

        while (!bfsQueue.empty()) {
            std::string currId = bfsQueue.front();
            bfsQueue.pop();
            int currLevel = nodeLevel[currId];

            auto it = nodeRegistry.find(currId);
            if (it == nodeRegistry.end()) continue;
            const auto& node = it->second;

            std::vector<std::string> nextChildren;
            if (node->defaultNext) nextChildren.push_back(node->defaultNext->id);
            for (const auto& ch : node->choices) {
                if (ch.nextNode) nextChildren.push_back(ch.nextNode->id);
            }
            if (!node->thenNodeId.empty() && nodeRegistry.count(node->thenNodeId)) {
                nextChildren.push_back(node->thenNodeId);
            }
            if (!node->elseNodeId.empty() && nodeRegistry.count(node->elseNodeId)) {
                nextChildren.push_back(node->elseNodeId);
            }

            for (const auto& childId : nextChildren) {
                if (visited.find(childId) == visited.end()) {
                    visited.insert(childId);
                    nodeLevel[childId] = currLevel + 1;
                    bfsQueue.push(childId);
                }
            }
        }

        int maxTreeLevel = 0;
        for (const auto& [_, lvl] : nodeLevel) {
            if (lvl > maxTreeLevel) maxTreeLevel = lvl;
        }

        for (const auto& [id, _] : nodeRegistry) {
            if (visited.find(id) == visited.end()) {
                maxTreeLevel += 1;
                visited.insert(id);
                nodeLevel[id] = maxTreeLevel;
                bfsQueue.push(id);

                while (!bfsQueue.empty()) {
                    std::string currId = bfsQueue.front();
                    bfsQueue.pop();
                    int currLevel = nodeLevel[currId];
                    const auto& node = nodeRegistry.at(currId);

                    std::vector<std::string> children;
                    if (node->defaultNext) children.push_back(node->defaultNext->id);
                    for (const auto& ch : node->choices) {
                        if (ch.nextNode) children.push_back(ch.nextNode->id);
                    }
                    if (!node->thenNodeId.empty() && nodeRegistry.count(node->thenNodeId)) children.push_back(node->thenNodeId);
                    if (!node->elseNodeId.empty() && nodeRegistry.count(node->elseNodeId)) children.push_back(node->elseNodeId);

                    for (const auto& childId : children) {
                        if (visited.find(childId) == visited.end()) {
                            visited.insert(childId);
                            nodeLevel[childId] = currLevel + 1;
                            if (nodeLevel[childId] > maxTreeLevel) maxTreeLevel = nodeLevel[childId];
                            bfsQueue.push(childId);
                        }
                    }
                }
            }
        }

        std::map<int, std::vector<std::string>> levelBuckets;
        for (const auto& [id, lvl] : nodeLevel) {
            levelBuckets[lvl].push_back(id);
        }

        float startX = 100.0f;
        float startY = 120.0f;

        for (auto& [lvl, list] : levelBuckets) {
            std::sort(list.begin(), list.end());
            for (size_t rank = 0; rank < list.size(); ++rank) {
                float posX = startX + static_cast<float>(lvl) * levelSpacingX;
                float posY = startY + static_cast<float>(rank) * siblingSpacingY;
                nodePositions[list[rank]] = sf::Vector2f(posX, posY);
            }
        }

        needsLayout = false;
    }

    static sf::Vector2f evaluateCubicBezier(sf::Vector2f p0, sf::Vector2f p1, sf::Vector2f p2, sf::Vector2f p3, float t) {
        float u = 1.0f - t;
        float tt = t * t;
        float uu = u * u;
        return (uu * u) * p0 + (3.0f * uu * t) * p1 + (3.0f * u * tt) * p2 + (tt * t) * p3;
    }

    void commitActiveField() {
        if (!selectedNode) return;
        if (activeField == EditField::Speaker) selectedNode->speaker = activeInputBuffer;
        else if (activeField == EditField::Text) selectedNode->text = activeInputBuffer;
        else if (activeField == EditField::Bg) { selectedNode->bg = activeInputBuffer; selectedNode->bgImagePath = activeInputBuffer; }
        else if (activeField == EditField::Bgm) { selectedNode->bgm = activeInputBuffer; selectedNode->bgmPath = activeInputBuffer; }
        else if (activeField == EditField::Cv) { selectedNode->cv = activeInputBuffer; selectedNode->voicePath = activeInputBuffer; }
        activeField = EditField::None;
    }

public:
    NodeGraphViewer() = default;

    bool loadFont(const std::string& fontPath) {
        fontDataBuffer.clear();
        if (AssetPack::readFileFromPak(fontPath, fontDataBuffer, "data.pak") && !fontDataBuffer.empty()) {
            if (font.openFromMemory(fontDataBuffer.data(), fontDataBuffer.size())) {
                hasFont = true;
                return true;
            }
        } else if (font.openFromFile(fontPath)) {
            hasFont = true;
            return true;
        }
        hasFont = false;
        return false;
    }

    void toggle() {
        isVisible = !isVisible;
        if (isVisible) {
            needsLayout = true;
        } else {
            commitActiveField();
        }
    }

    bool getIsVisible() const {
        return isVisible;
    }

    void centerOnNode(const std::string& nodeId, sf::Vector2u windowSize) {
        if (nodePositions.find(nodeId) != nodePositions.end()) {
            sf::Vector2f worldPos = nodePositions[nodeId];
            float winW = static_cast<float>(windowSize.x) - 380.0f; // 扣除右側 Inspector 寬度
            float winH = static_cast<float>(windowSize.y);
            viewOffset.x = (winW * 0.5f) - (worldPos.x + baseCardWidth * 0.5f) * zoomScale;
            viewOffset.y = (winH * 0.5f) - (worldPos.y + baseCardHeight * 0.5f) * zoomScale;
        }
    }

    void handleTextEntered(char32_t unicode) {
        if (!isVisible || activeField == EditField::None) return;

        if (unicode == 13) { // Enter
            commitActiveField();
            return;
        }
        if (unicode == 8) { // Backspace
            if (!activeInputBuffer.empty()) {
                activeInputBuffer.pop_back();
            }
            return;
        }
        if (unicode == 27) { // Escape
            activeField = EditField::None;
            return;
        }

        if (unicode >= 32 && unicode < 127) {
            activeInputBuffer += static_cast<char>(unicode);
        } else {
            // SFML 3.x sf::U8String 跨平臺相容轉型
            sf::String sfChar(std::u32string(1, unicode));
            auto u8Str = sfChar.toUtf8();
            activeInputBuffer.append(reinterpret_cast<const char*>(u8Str.data()), u8Str.size());
        }
    }

    void handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
        if (!isVisible) return;

        if (activeField != EditField::None) {
            if (const auto* keyBtn = event.getIf<sf::Event::KeyPressed>()) {
                if (keyBtn->code == sf::Keyboard::Key::Escape) {
                    activeField = EditField::None;
                    return;
                }
            }
        }

        if (const auto* mouseBtn = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mouseBtn->button == sf::Mouse::Button::Right) {
                isPanning = true;
                panStartMousePos = sf::Mouse::getPosition(window);
            }
        }
        if (const auto* mouseRel = event.getIf<sf::Event::MouseButtonReleased>()) {
            if (mouseRel->button == sf::Mouse::Button::Right) {
                isPanning = false;
            }
        }
        if (event.is<sf::Event::MouseMoved>()) {
            if (isPanning) {
                sf::Vector2i currentPos = sf::Mouse::getPosition(window);
                sf::Vector2i delta = currentPos - panStartMousePos;
                viewOffset.x += static_cast<float>(delta.x);
                viewOffset.y += static_cast<float>(delta.y);
                panStartMousePos = currentPos;
            }
        }

        if (const auto* wheelEvt = event.getIf<sf::Event::MouseWheelScrolled>()) {
            if (wheelEvt->wheel == sf::Mouse::Wheel::Vertical) {
                float zoomFactor = (wheelEvt->delta > 0.0f) ? 1.15f : 0.85f;
                zoomScale = std::clamp(zoomScale * zoomFactor, 0.25f, 3.0f);
            }
        }

        if (const auto* keyBtn = event.getIf<sf::Event::KeyPressed>()) {
            float panSpeed = 60.0f;

            if (keyBtn->code == sf::Keyboard::Key::Left || keyBtn->code == sf::Keyboard::Key::A)  viewOffset.x += panSpeed;
            if (keyBtn->code == sf::Keyboard::Key::Right || keyBtn->code == sf::Keyboard::Key::D) viewOffset.x -= panSpeed;
            if (keyBtn->code == sf::Keyboard::Key::Up || keyBtn->code == sf::Keyboard::Key::W)    viewOffset.y += panSpeed;
            if (keyBtn->code == sf::Keyboard::Key::Down || keyBtn->code == sf::Keyboard::Key::S)  viewOffset.y -= panSpeed;

            if (keyBtn->code == sf::Keyboard::Key::Equal || keyBtn->code == sf::Keyboard::Key::Add) {
                zoomScale = std::clamp(zoomScale * 1.15f, 0.25f, 3.0f);
            }
            if (keyBtn->code == sf::Keyboard::Key::Hyphen || keyBtn->code == sf::Keyboard::Key::Subtract) {
                zoomScale = std::clamp(zoomScale * 0.85f, 0.25f, 3.0f);
            }
            if (keyBtn->code == sf::Keyboard::Key::R) {
                viewOffset = sf::Vector2f(100.0f, 150.0f);
                zoomScale = 0.85f;
            }
            if (keyBtn->code == sf::Keyboard::Key::F) {
                if (!cachedCurrentNodeId.empty()) {
                    centerOnNode(cachedCurrentNodeId, window.getSize());
                }
            }
        }
    }

    void draw(sf::RenderTarget& target, 
              const std::unordered_map<std::string, std::shared_ptr<StoryNode>>& nodeRegistry,
              StoryExecutor& executor,
              std::function<void()> onNodeJumped) 
    {
        if (!isVisible || !hasFont) return;

        sf::Vector2f targetSize(static_cast<float>(target.getSize().x), static_cast<float>(target.getSize().y));

        // 1. 全螢幕暗色遮罩背景
        sf::RectangleShape modalOverlay(targetSize);
        modalOverlay.setFillColor(sf::Color(10, 12, 16, 245));
        target.draw(modalOverlay);

        auto currentNode = executor.getCurrentNode();
        cachedCurrentNodeId = currentNode ? currentNode->id : "";

        if (needsLayout || nodePositions.empty()) {
            computeTreeLayout(nodeRegistry, cachedCurrentNodeId);
            centerOnNode(cachedCurrentNodeId, target.getSize());
        }

        auto toScreen = [&](sf::Vector2f worldPos) {
            return sf::Vector2f(
                viewOffset.x + worldPos.x * zoomScale,
                viewOffset.y + worldPos.y * zoomScale
            );
        };

        sf::Vector2f cardSize(baseCardWidth * zoomScale, baseCardHeight * zoomScale);

        // 2. 繪製平滑三次貝茲曲線連線
        for (const auto& [id, node] : nodeRegistry) {
            if (nodePositions.find(id) == nodePositions.end()) continue;
            sf::Vector2f p1 = toScreen(nodePositions[id]);
            sf::Vector2f p1Out(p1.x + cardSize.x, p1.y + cardSize.y * 0.5f);

            auto drawBezierCurve = [&](const std::string& targetId, sf::Color color) {
                if (!targetId.empty() && nodePositions.find(targetId) != nodePositions.end()) {
                    sf::Vector2f p2 = toScreen(nodePositions[targetId]);
                    sf::Vector2f p2In(p2.x, p2.y + cardSize.y * 0.5f);

                    float minX = std::min(p1Out.x, p2In.x) - 50.0f;
                    float maxX = std::max(p1Out.x, p2In.x) + 50.0f;
                    float minY = std::min(p1Out.y, p2In.y) - 50.0f;
                    float maxY = std::max(p1Out.y, p2In.y) + 50.0f;
                    if (maxX < 0.0f || minX > targetSize.x - 360.0f || maxY < 0.0f || minY > targetSize.y) {
                        return;
                    }

                    sf::Vector2f cp1, cp2;
                    float deltaX = p2In.x - p1Out.x;

                    if (deltaX > 0.0f) {
                        cp1 = sf::Vector2f(p1Out.x + deltaX * 0.5f, p1Out.y);
                        cp2 = sf::Vector2f(p2In.x - deltaX * 0.5f, p2In.y);
                    } else {
                        float loopOffset = 100.0f * zoomScale;
                        cp1 = sf::Vector2f(p1Out.x + loopOffset, p1Out.y + loopOffset);
                        cp2 = sf::Vector2f(p2In.x - loopOffset, p2In.y + loopOffset);
                    }

                    const int segments = 16;
                    sf::VertexArray curve(sf::PrimitiveType::LineStrip, segments + 1);
                    for (int s = 0; s <= segments; ++s) {
                        float t = static_cast<float>(s) / static_cast<float>(segments);
                        curve[s].position = evaluateCubicBezier(p1Out, cp1, cp2, p2In, t);
                        curve[s].color = color;
                    }
                    target.draw(curve);

                    sf::ConvexShape arrow(3);
                    arrow.setPoint(0, p2In);
                    arrow.setPoint(1, sf::Vector2f(p2In.x - 8.0f * zoomScale, p2In.y - 4.5f * zoomScale));
                    arrow.setPoint(2, sf::Vector2f(p2In.x - 8.0f * zoomScale, p2In.y + 4.5f * zoomScale));
                    arrow.setFillColor(color);
                    target.draw(arrow);
                }
            };

            if (node->defaultNext) drawBezierCurve(node->defaultNext->id, sf::Color(100, 149, 237, 210));
            for (const auto& ch : node->choices) {
                if (ch.nextNode) drawBezierCurve(ch.nextNode->id, sf::Color(255, 215, 0, 230));
            }
            if (!node->thenNodeId.empty()) drawBezierCurve(node->thenNodeId, sf::Color(80, 220, 80, 230));
            if (!node->elseNodeId.empty()) drawBezierCurve(node->elseNodeId, sf::Color(240, 80, 80, 230));
        }

        // 3. 繪製樹狀卡片 (Tree Node Cards)
        for (const auto& [id, node] : nodeRegistry) {
            if (nodePositions.find(id) == nodePositions.end()) continue;

            sf::Vector2f screenPos = toScreen(nodePositions[id]);

            if (screenPos.x + cardSize.x < -50.0f || screenPos.x > targetSize.x - 360.0f ||
                screenPos.y + cardSize.y < -50.0f || screenPos.y > targetSize.y + 50.0f) {
                continue;
            }

            bool isPlaying = (id == cachedCurrentNodeId);
            bool isSelected = (id == selectedNodeId);

            sf::Color cardBg = sf::Color(24, 28, 38, 245);
            if (node->type == NodeType::Choice)        cardBg = sf::Color(38, 30, 52, 245);
            else if (node->type == NodeType::Condition) cardBg = sf::Color(46, 38, 20, 245);
            else if (node->type == NodeType::Action)    cardBg = sf::Color(20, 42, 42, 245);

            if (isPlaying) cardBg = sf::Color(35, 90, 50, 250);
            else if (isSelected) cardBg = sf::Color(55, 70, 110, 250);

            sf::RectangleShape card(cardSize);
            card.setPosition(screenPos);
            card.setFillColor(cardBg);
            card.setOutlineThickness(isPlaying ? 2.5f * zoomScale : 1.5f * zoomScale);
            card.setOutlineColor(isPlaying ? sf::Color(80, 255, 120) :
                                 isSelected ? sf::Color(255, 215, 0) : sf::Color(70, 80, 100));
            target.draw(card);

            sf::Text titleText(font, sf::String::fromUtf8(id.begin(), id.end()), static_cast<unsigned int>(14.0f * zoomScale));
            titleText.setFillColor(isPlaying ? sf::Color(230, 255, 230) : sf::Color::White);
            titleText.setPosition(screenPos + sf::Vector2f(10.0f * zoomScale, 8.0f * zoomScale));
            target.draw(titleText);

            std::string typeStr = "[" + std::string(
                node->type == NodeType::Dialogue ? "Dialogue" :
                node->type == NodeType::Choice   ? "Choice"   :
                node->type == NodeType::Condition ? "Condition" : "Action") + "]";
            sf::Text typeText(font, sf::String::fromUtf8(typeStr.begin(), typeStr.end()), static_cast<unsigned int>(11.0f * zoomScale));
            typeText.setFillColor(sf::Color(170, 180, 205));
            typeText.setPosition(screenPos + sf::Vector2f(10.0f * zoomScale, 30.0f * zoomScale));
            target.draw(typeText);

            if (!node->speaker.empty()) {
                sf::Text spkText(font, sf::String::fromUtf8(node->speaker.begin(), node->speaker.end()), static_cast<unsigned int>(11.0f * zoomScale));
                spkText.setFillColor(sf::Color(255, 215, 0));
                spkText.setPosition(screenPos + sf::Vector2f(10.0f * zoomScale, 50.0f * zoomScale));
                target.draw(spkText);
            }
        }

        // ====================================================================
        // 4. Live Node Property Inspector (右側即時屬性檢視與編輯面板)
        // ====================================================================
        float inspectorW = 350.0f;
        float inspectorX = targetSize.x - inspectorW;
        float inspectorY = 42.0f;
        float inspectorH = targetSize.y - 42.0f;

        sf::RectangleShape inspBg(sf::Vector2f(inspectorW, inspectorH));
        inspBg.setPosition(sf::Vector2f(inspectorX, inspectorY));
        inspBg.setFillColor(sf::Color(18, 22, 30, 250));
        inspBg.setOutlineThickness(1.5f);
        inspBg.setOutlineColor(sf::Color(70, 80, 105));
        target.draw(inspBg);

        sf::Text inspTitle(font, "=== NODE INSPECTOR ===", 16);
        inspTitle.setFillColor(sf::Color(255, 215, 0));
        inspTitle.setPosition(sf::Vector2f(inspectorX + 20.0f, inspectorY + 15.0f));
        target.draw(inspTitle);

        if (selectedNode) {
            float startY = inspectorY + 55.0f;
            float lineH = 45.0f;

            auto drawInspectorRow = [&](const std::string& label, const std::string& val, EditField fieldEnum, sf::FloatRect& outBox) {
                sf::Text lbl(font, label, 13);
                lbl.setFillColor(sf::Color(170, 180, 200));
                lbl.setPosition(sf::Vector2f(inspectorX + 20.0f, startY));
                target.draw(lbl);

                bool isEditing = (activeField == fieldEnum);
                std::string displayVal = isEditing ? (activeInputBuffer + "_") : val;

                outBox = sf::FloatRect(sf::Vector2f(inspectorX + 20.0f, startY + 18.0f), sf::Vector2f(inspectorW - 40.0f, 26.0f));
                sf::RectangleShape box(outBox.size);
                box.setPosition(outBox.position);
                box.setFillColor(isEditing ? sf::Color(35, 45, 65) : sf::Color(24, 28, 38));
                box.setOutlineThickness(1.0f);
                box.setOutlineColor(isEditing ? sf::Color(100, 149, 237) : sf::Color(60, 70, 90));
                target.draw(box);

                sf::Text vTxt(font, sf::String::fromUtf8(displayVal.begin(), displayVal.end()), 13);
                vTxt.setFillColor(sf::Color::White);
                vTxt.setPosition(sf::Vector2f(outBox.position.x + 8.0f, outBox.position.y + 4.0f));
                target.draw(vTxt);

                startY += lineH + 15.0f;
            };

            sf::Text idInfo(font, "ID: " + selectedNode->id, 14);
            idInfo.setFillColor(sf::Color(100, 220, 140));
            idInfo.setPosition(sf::Vector2f(inspectorX + 20.0f, startY));
            target.draw(idInfo);
            startY += 30.0f;

            drawInspectorRow("Speaker:", selectedNode->speaker, EditField::Speaker, speakerBoxBounds);
            drawInspectorRow("Dialogue Text:", selectedNode->text, EditField::Text, textBoxBounds);
            drawInspectorRow("Background Path:", selectedNode->bg, EditField::Bg, bgBoxBounds);
            drawInspectorRow("BGM Path:", selectedNode->bgm, EditField::Bgm, bgmBoxBounds);
            drawInspectorRow("Voice (CV) Path:", selectedNode->cv, EditField::Cv, cvBoxBounds);

            sf::Text hint(font, "[Click field to edit, Enter to commit]", 11);
            hint.setFillColor(sf::Color(130, 140, 160));
            hint.setPosition(sf::Vector2f(inspectorX + 20.0f, inspectorY + inspectorH - 30.0f));
            target.draw(hint);
        } else {
            sf::Text noneTxt(font, "Select a node in the graph\nto inspect & edit properties.", 14);
            noneTxt.setFillColor(sf::Color(140, 150, 170));
            noneTxt.setPosition(sf::Vector2f(inspectorX + 20.0f, inspectorY + 80.0f));
            target.draw(noneTxt);
        }

        // ====================================================================
        // 5. 頂層 HUD 狀態列
        // ====================================================================
        sf::RectangleShape hudBar(sf::Vector2f(targetSize.x, 42.0f));
        hudBar.setFillColor(sf::Color(16, 18, 26, 235));
        hudBar.setOutlineThickness(1.0f);
        hudBar.setOutlineColor(sf::Color(60, 70, 90));
        target.draw(hudBar);

        std::ostringstream hudSs;
        hudSs << "NARRATIVE DAG TREE | Zoom: " << static_cast<int>(zoomScale * 100.0f) 
              << "% | [Arrows/WASD] Pan  [+/-] Zoom  [F] Focus Active  [R] Reset  [Esc] Close";
        std::string hudStr = hudSs.str();

        sf::Text hintText(font, sf::String::fromUtf8(hudStr.begin(), hudStr.end()), 14);
        hintText.setFillColor(sf::Color(220, 225, 240));
        hintText.setPosition(sf::Vector2f(18.0f, 11.0f));
        target.draw(hintText);
    }

    void handleMouseClick(const sf::Vector2i& mousePos, 
                          const std::unordered_map<std::string, std::shared_ptr<StoryNode>>& nodeRegistry,
                          StoryExecutor& executor,
                          std::function<void()> onNodeJumped) 
    {
        if (!isVisible) return;
        sf::Vector2f mPos(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

        // 1. 檢測 Inspector 內部的輸入框點擊
        if (selectedNode) {
            commitActiveField(); 
            if (speakerBoxBounds.contains(mPos)) {
                activeField = EditField::Speaker;
                activeInputBuffer = selectedNode->speaker;
                return;
            }
            if (textBoxBounds.contains(mPos)) {
                activeField = EditField::Text;
                activeInputBuffer = selectedNode->text;
                return;
            }
            if (bgBoxBounds.contains(mPos)) {
                activeField = EditField::Bg;
                activeInputBuffer = selectedNode->bg;
                return;
            }
            if (bgmBoxBounds.contains(mPos)) {
                activeField = EditField::Bgm;
                activeInputBuffer = selectedNode->bgm;
                return;
            }
            if (cvBoxBounds.contains(mPos)) {
                activeField = EditField::Cv;
                activeInputBuffer = selectedNode->cv;
                return;
            }
        }

        // 2. 檢測圖譜上的節點點擊
        auto toScreen = [&](sf::Vector2f worldPos) {
            return sf::Vector2f(
                viewOffset.x + worldPos.x * zoomScale,
                viewOffset.y + worldPos.y * zoomScale
            );
        };

        sf::Vector2f cardSize(baseCardWidth * zoomScale, baseCardHeight * zoomScale);

        for (const auto& [id, node] : nodeRegistry) {
            if (nodePositions.find(id) == nodePositions.end()) continue;
            sf::Vector2f screenPos = toScreen(nodePositions[id]);
            sf::FloatRect bounds(screenPos, cardSize);

            if (bounds.contains(mPos)) {
                selectedNodeId = id;
                selectedNode = node;
                activeField = EditField::None;
                executor.jumpToNode(node);
                if (onNodeJumped) onNodeJumped();
                break;
            }
        }
    }
};