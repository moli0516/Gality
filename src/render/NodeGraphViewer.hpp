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
#include <set>
#include <map>
#include "../story/StoryNode.hpp"
#include "../story/StoryExecutor.hpp"
#include "../core/AssetPack.hpp"

class NodeGraphViewer {
private:
    bool isVisible = false;
    sf::Font font;
    bool hasFont = false;

    std::vector<std::uint8_t> fontDataBuffer;

    // ===== 視圖座標 =====
    sf::Vector2f viewOffset{100.0f, 150.0f};
    float zoomScale = 0.85f;
    bool isPanning = false;
    sf::Vector2i panStartMousePos{0, 0};

    // ===== 節點選擇 =====
    std::string selectedNodeId = "";
    std::shared_ptr<StoryNode> selectedNode = nullptr;
    std::string cachedCurrentNodeId = "";
    std::unordered_map<std::string, sf::Vector2f> nodePositions;
    bool needsLayout = true;

    // ===== 搜尋 =====
    std::string searchQuery = "";
    bool searchActive = false;
    std::unordered_set<std::string> searchResults;

    // ===== 過濾 =====
    bool showDialogue = true;
    bool showChoice = true;
    bool showAction = true;
    bool showCondition = true;

    // ===== 路徑高亮 =====
    std::string pathHighlightTarget = "";
    std::unordered_set<std::string> highlightedPath;

    // ===== 節點卡片尺寸 =====
    const float baseCardWidth = 240.0f;
    const float baseCardHeight = 80.0f;
    const float levelSpacingX = 360.0f;
    const float siblingSpacingY = 120.0f;

    // ===== Live Inspector =====
    enum class EditField { None, Speaker, Text, Bg, Bgm, Cv };
    EditField activeField = EditField::None;
    std::string activeInputBuffer = "";

    sf::FloatRect speakerBoxBounds;
    sf::FloatRect textBoxBounds;
    sf::FloatRect bgBoxBounds;
    sf::FloatRect bgmBoxBounds;
    sf::FloatRect cvBoxBounds;

    // ===== 統計 =====
    size_t totalNodes = 0;
    size_t visibleNodes = 0;
    size_t dialogueCount = 0;
    size_t choiceCount = 0;
    size_t actionCount = 0;
    size_t conditionCount = 0;

    // ------------------------------------------------------------------------
    // 樹狀拓撲佈局
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
                    auto nodeIt = nodeRegistry.find(currId);
                    if (nodeIt == nodeRegistry.end()) continue;
                    const auto& node = nodeIt->second;

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

    // ------------------------------------------------------------------------
    // 更新統計
    // ------------------------------------------------------------------------
    void updateStatistics(const std::unordered_map<std::string, std::shared_ptr<StoryNode>>& nodeRegistry) {
        totalNodes = nodeRegistry.size();
        dialogueCount = 0;
        choiceCount = 0;
        actionCount = 0;
        conditionCount = 0;

        for (const auto& [id, node] : nodeRegistry) {
            switch (node->type) {
                case NodeType::Dialogue:  dialogueCount++; break;
                case NodeType::Choice:    choiceCount++; break;
                case NodeType::Action:    actionCount++; break;
                case NodeType::Condition: conditionCount++; break;
            }
        }
    }

    // ------------------------------------------------------------------------
    // 檢查節點是否通過過濾
    // ------------------------------------------------------------------------
    bool passesFilter(const std::shared_ptr<StoryNode>& node) const {
        switch (node->type) {
            case NodeType::Dialogue:  return showDialogue;
            case NodeType::Choice:    return showChoice;
            case NodeType::Action:    return showAction;
            case NodeType::Condition: return showCondition;
        }
        return true;
    }

    bool passesSearch(const std::string& nodeId, const std::shared_ptr<StoryNode>& node) const {
        if (searchQuery.empty()) return true;
        return searchResults.find(nodeId) != searchResults.end();
    }

    // ------------------------------------------------------------------------
    // 執行搜尋
    // ------------------------------------------------------------------------
    void performSearch(const std::unordered_map<std::string, std::shared_ptr<StoryNode>>& nodeRegistry) {
        searchResults.clear();
        if (searchQuery.empty()) return;

        std::string query = searchQuery;
        std::transform(query.begin(), query.end(), query.begin(), ::tolower);

        for (const auto& [id, node] : nodeRegistry) {
            std::string idLower = id;
            std::transform(idLower.begin(), idLower.end(), idLower.begin(), ::tolower);

            if (idLower.find(query) != std::string::npos) {
                searchResults.insert(id);
                continue;
            }

            std::string speakerLower = node->speaker;
            std::transform(speakerLower.begin(), speakerLower.end(), speakerLower.begin(), ::tolower);
            if (speakerLower.find(query) != std::string::npos) {
                searchResults.insert(id);
                continue;
            }

            std::string textLower = node->text;
            std::transform(textLower.begin(), textLower.end(), textLower.begin(), ::tolower);
            if (textLower.find(query) != std::string::npos) {
                searchResults.insert(id);
            }
        }
    }

    // ------------------------------------------------------------------------
    // 計算路徑
    // ------------------------------------------------------------------------
    void computePath(const std::string& fromId, const std::string& toId,
                     const std::unordered_map<std::string, std::shared_ptr<StoryNode>>& nodeRegistry)
    {
        highlightedPath.clear();
        if (fromId.empty() || toId.empty()) return;

        std::unordered_map<std::string, std::string> parent;
        std::unordered_set<std::string> visited;
        std::queue<std::string> bfsQueue;

        visited.insert(fromId);
        bfsQueue.push(fromId);

        while (!bfsQueue.empty()) {
            std::string curr = bfsQueue.front();
            bfsQueue.pop();

            if (curr == toId) break;

            auto it = nodeRegistry.find(curr);
            if (it == nodeRegistry.end()) continue;
            const auto& node = it->second;

            std::vector<std::string> nexts;
            if (node->defaultNext) nexts.push_back(node->defaultNext->id);
            for (const auto& ch : node->choices) {
                if (ch.nextNode) nexts.push_back(ch.nextNode->id);
            }
            if (!node->thenNodeId.empty()) nexts.push_back(node->thenNodeId);
            if (!node->elseNodeId.empty()) nexts.push_back(node->elseNodeId);

            for (const auto& next : nexts) {
                if (visited.find(next) == visited.end()) {
                    visited.insert(next);
                    parent[next] = curr;
                    bfsQueue.push(next);
                }
            }
        }

        std::string curr = toId;
        while (curr != fromId && parent.count(curr)) {
            highlightedPath.insert(curr);
            curr = parent[curr];
        }
        highlightedPath.insert(fromId);
    }

    // ========================================================================
    // 繪製工具
    // ========================================================================
    static sf::Vector2f evaluateCubicBezier(sf::Vector2f p0, sf::Vector2f p1, sf::Vector2f p2, sf::Vector2f p3, float t) {
        float u = 1.0f - t;
        float tt = t * t;
        float uu = u * u;
        return (uu * u) * p0 + (3.0f * uu * t) * p1 + (3.0f * u * tt) * p2 + (tt * t) * p3;
    }

    static sf::Color getNodeColor(NodeType type) {
        switch (type) {
            case NodeType::Dialogue:  return sf::Color(35, 55, 85, 245);
            case NodeType::Choice:    return sf::Color(55, 40, 80, 245);
            case NodeType::Condition: return sf::Color(70, 55, 25, 245);
            case NodeType::Action:    return sf::Color(30, 65, 60, 245);
        }
        return sf::Color(40, 45, 60, 245);
    }

    static std::string getNodeTypeLabel(NodeType type) {
        switch (type) {
            case NodeType::Dialogue:  return "Dialogue";
            case NodeType::Choice:    return "Choice";
            case NodeType::Condition: return "Condition";
            case NodeType::Action:    return "Action";
        }
        return "Unknown";
    }

    // ========================================================================
    // 文字繪製輔助（修復亂碼）
    // ========================================================================
    void drawText(sf::RenderTarget& target,
                  const std::string& text,
                  float x, float y,
                  unsigned int size,
                  sf::Color color) {
        sf::Text sfText(font);
        sfText.setString(sf::String::fromUtf8(text.begin(), text.end()));
        sfText.setCharacterSize(size);
        sfText.setFillColor(color);
        sfText.setPosition(sf::Vector2f(x, y));
        target.draw(sfText);
    }

    // ========================================================================
    // 節點選擇
    // ========================================================================
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
            float winW = static_cast<float>(windowSize.x) - 380.0f;
            float winH = static_cast<float>(windowSize.y);
            viewOffset.x = (winW * 0.5f) - (worldPos.x + baseCardWidth * 0.5f) * zoomScale;
            viewOffset.y = (winH * 0.5f) - (worldPos.y + baseCardHeight * 0.5f) * zoomScale;
        }
    }

    void handleTextEntered(char32_t unicode) {
        if (!isVisible) return;

        if (searchActive) {
            if (unicode == 13) { searchActive = false; return; }
            if (unicode == 8) {
                if (!searchQuery.empty()) searchQuery.pop_back();
                return;
            }
            if (unicode == 27) {
                searchActive = false;
                searchQuery.clear();
                searchResults.clear();
                return;
            }
            if (unicode >= 32 && unicode < 127) {
                searchQuery += static_cast<char>(unicode);
            }
            return;
        }

        if (activeField == EditField::None) return;

        if (unicode == 13) {
            commitActiveField();
            return;
        }
        if (unicode == 8) {
            if (!activeInputBuffer.empty()) {
                activeInputBuffer.pop_back();
            }
            return;
        }
        if (unicode == 27) {
            activeField = EditField::None;
            return;
        }

        if (unicode >= 32 && unicode < 127) {
            activeInputBuffer += static_cast<char>(unicode);
        } else {
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

            if (keyBtn->code == sf::Keyboard::Key::Slash) {
                searchActive = true;
                searchQuery.clear();
            }

            if (keyBtn->code == sf::Keyboard::Key::Num1) showDialogue = !showDialogue;
            if (keyBtn->code == sf::Keyboard::Key::Num2) showChoice = !showChoice;
            if (keyBtn->code == sf::Keyboard::Key::Num3) showAction = !showAction;
            if (keyBtn->code == sf::Keyboard::Key::Num4) showCondition = !showCondition;
        }
    }

    // ========================================================================
    // 主繪製
    // ========================================================================
    void draw(sf::RenderTarget& target,
              const std::unordered_map<std::string, std::shared_ptr<StoryNode>>& nodeRegistry,
              StoryExecutor& executor,
              std::function<void()> onNodeJumped)
    {
        if (!isVisible || !hasFont) return;

        sf::Vector2f targetSize(static_cast<float>(target.getSize().x), static_cast<float>(target.getSize().y));

        // 全螢幕遮罩
        sf::RectangleShape modalOverlay(targetSize);
        modalOverlay.setFillColor(sf::Color(10, 12, 16, 245));
        target.draw(modalOverlay);

        auto currentNode = executor.getCurrentNode();
        cachedCurrentNodeId = currentNode ? currentNode->id : "";

        updateStatistics(nodeRegistry);

        if (!searchQuery.empty()) {
            performSearch(nodeRegistry);
        }

        if (needsLayout || nodePositions.empty()) {
            computeTreeLayout(nodeRegistry, cachedCurrentNodeId);
            centerOnNode(cachedCurrentNodeId, target.getSize());
        }

        if (!pathHighlightTarget.empty() && !cachedCurrentNodeId.empty()) {
            computePath(cachedCurrentNodeId, pathHighlightTarget, nodeRegistry);
        }

        auto toScreen = [&](sf::Vector2f worldPos) {
            return sf::Vector2f(
                viewOffset.x + worldPos.x * zoomScale,
                viewOffset.y + worldPos.y * zoomScale
            );
        };

        sf::Vector2f cardSize(baseCardWidth * zoomScale, baseCardHeight * zoomScale);

        float viewLeft = 0.0f;
        float viewRight = targetSize.x - 380.0f;
        float viewTop = 0.0f;
        float viewBottom = targetSize.y;

        visibleNodes = 0;

        // 繪製連線
        for (const auto& [id, node] : nodeRegistry) {
            if (!passesFilter(node)) continue;
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
                    if (maxX < viewLeft || minX > viewRight || maxY < viewTop || minY > viewBottom) {
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

            bool isHighlighted = highlightedPath.empty() || (highlightedPath.count(id) > 0);
            sf::Color baseColor = isHighlighted ? sf::Color(255, 255, 255, 200) : sf::Color(80, 90, 110, 100);

            if (node->defaultNext) drawBezierCurve(node->defaultNext->id,
                isHighlighted ? sf::Color(100, 149, 237, 220) : baseColor);
            for (const auto& ch : node->choices) {
                if (ch.nextNode) drawBezierCurve(ch.nextNode->id,
                    isHighlighted ? sf::Color(255, 215, 0, 230) : baseColor);
            }
            if (!node->thenNodeId.empty()) drawBezierCurve(node->thenNodeId,
                isHighlighted ? sf::Color(80, 220, 80, 230) : baseColor);
            if (!node->elseNodeId.empty()) drawBezierCurve(node->elseNodeId,
                isHighlighted ? sf::Color(240, 80, 80, 230) : baseColor);
        }

        // 繪製節點
        for (const auto& [id, node] : nodeRegistry) {
            if (!passesFilter(node)) continue;
            if (!passesSearch(id, node)) continue;
            if (nodePositions.find(id) == nodePositions.end()) continue;

            sf::Vector2f screenPos = toScreen(nodePositions[id]);

            if (screenPos.x + cardSize.x < viewLeft || screenPos.x > viewRight ||
                screenPos.y + cardSize.y < viewTop || screenPos.y > viewBottom) {
                continue;
            }

            visibleNodes++;

            bool isPlaying = (id == cachedCurrentNodeId);
            bool isSelected = (id == selectedNodeId);
            bool isSearchResult = (searchResults.count(id) > 0);
            bool isPathHighlight = (highlightedPath.count(id) > 0);

            sf::Color cardBg = getNodeColor(node->type);

            if (isPlaying) {
                cardBg = sf::Color(35, 100, 60, 255);
            } else if (isSearchResult) {
                cardBg = sf::Color(120, 100, 30, 255);
            } else if (isPathHighlight && !highlightedPath.empty()) {
                cardBg = sf::Color(80, 60, 120, 255);
            } else if (isSelected) {
                cardBg = sf::Color(55, 70, 110, 255);
            }

            sf::RectangleShape card(cardSize);
            card.setPosition(screenPos);
            card.setFillColor(cardBg);
            card.setOutlineThickness(isPlaying ? 2.5f * zoomScale : 1.5f * zoomScale);

            sf::Color outlineColor = sf::Color(70, 80, 100);
            if (isPlaying) outlineColor = sf::Color(80, 255, 120);
            else if (isSearchResult) outlineColor = sf::Color(255, 215, 0);
            else if (isPathHighlight && !highlightedPath.empty()) outlineColor = sf::Color(180, 140, 255);
            else if (isSelected) outlineColor = sf::Color(255, 215, 0);

            card.setOutlineColor(outlineColor);
            target.draw(card);

            // 節點 ID
            drawText(target, id,
                     screenPos.x + 10.0f * zoomScale,
                     screenPos.y + 8.0f * zoomScale,
                     static_cast<unsigned int>(14.0f * zoomScale),
                     isPlaying ? sf::Color(230, 255, 230) : sf::Color::White);

            // 節點類型
            std::string typeStr = "[" + getNodeTypeLabel(node->type) + "]";
            drawText(target, typeStr,
                     screenPos.x + 10.0f * zoomScale,
                     screenPos.y + 30.0f * zoomScale,
                     static_cast<unsigned int>(11.0f * zoomScale),
                     sf::Color(170, 180, 205));

            // Speaker
            if (!node->speaker.empty()) {
                drawText(target, node->speaker,
                         screenPos.x + 10.0f * zoomScale,
                         screenPos.y + 50.0f * zoomScale,
                         static_cast<unsigned int>(11.0f * zoomScale),
                         sf::Color(255, 215, 0));
            }
        }

        drawInspector(target, nodeRegistry, onNodeJumped);
        drawHUD(target);
    }

    // ========================================================================
    // 繪製 Inspector
    // ========================================================================
    void drawInspector(sf::RenderTarget& target,
                       const std::unordered_map<std::string, std::shared_ptr<StoryNode>>& nodeRegistry,
                       std::function<void()>& onNodeJumped)
    {
        sf::Vector2f targetSize(static_cast<float>(target.getSize().x), static_cast<float>(target.getSize().y));

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

        drawText(target, "=== NODE INSPECTOR ===",
                 inspectorX + 20.0f, inspectorY + 15.0f,
                 16, sf::Color(255, 215, 0));

        if (selectedNode) {
            float startY = inspectorY + 55.0f;
            float lineH = 45.0f;

            auto drawInspectorRow = [&](const std::string& label, const std::string& val,
                                        EditField fieldEnum, sf::FloatRect& outBox) {
                drawText(target, label, inspectorX + 20.0f, startY, 13, sf::Color(170, 180, 200));

                bool isEditing = (activeField == fieldEnum);
                std::string displayVal = isEditing ? (activeInputBuffer + "_") : val;

                outBox = sf::FloatRect(sf::Vector2f(inspectorX + 20.0f, startY + 18.0f),
                                       sf::Vector2f(inspectorW - 40.0f, 26.0f));
                sf::RectangleShape box(outBox.size);
                box.setPosition(outBox.position);
                box.setFillColor(isEditing ? sf::Color(35, 45, 65) : sf::Color(24, 28, 38));
                box.setOutlineThickness(1.0f);
                box.setOutlineColor(isEditing ? sf::Color(100, 149, 237) : sf::Color(60, 70, 90));
                target.draw(box);

                drawText(target, displayVal,
                         outBox.position.x + 8.0f, outBox.position.y + 4.0f,
                         13, sf::Color::White);

                startY += lineH + 15.0f;
            };

            std::string idInfoStr = "ID: " + selectedNode->id;
            drawText(target, idInfoStr, inspectorX + 20.0f, startY, 14, sf::Color(100, 220, 140));
            startY += 30.0f;

            drawInspectorRow("Speaker:", selectedNode->speaker, EditField::Speaker, speakerBoxBounds);
            drawInspectorRow("Dialogue Text:", selectedNode->text, EditField::Text, textBoxBounds);
            drawInspectorRow("Background Path:", selectedNode->bg, EditField::Bg, bgBoxBounds);
            drawInspectorRow("BGM Path:", selectedNode->bgm, EditField::Bgm, bgmBoxBounds);
            drawInspectorRow("Voice (CV) Path:", selectedNode->cv, EditField::Cv, cvBoxBounds);

            drawText(target, "[Click field to edit, Enter to commit]",
                     inspectorX + 20.0f, inspectorY + inspectorH - 30.0f,
                     11, sf::Color(130, 140, 160));
        } else {
            drawText(target, "Select a node in the graph",
                     inspectorX + 20.0f, inspectorY + 80.0f, 14, sf::Color(140, 150, 170));
            drawText(target, "to inspect & edit properties.",
                     inspectorX + 20.0f, inspectorY + 105.0f, 14, sf::Color(140, 150, 170));
        }
    }

    // ========================================================================
    // 繪製 HUD
    // ========================================================================
    void drawHUD(sf::RenderTarget& target) {
        sf::Vector2f targetSize(static_cast<float>(target.getSize().x), static_cast<float>(target.getSize().y));

        // 頂部狀態列
        sf::RectangleShape hudBar(sf::Vector2f(targetSize.x, 42.0f));
        hudBar.setFillColor(sf::Color(16, 18, 26, 235));
        hudBar.setOutlineThickness(1.0f);
        hudBar.setOutlineColor(sf::Color(60, 70, 90));
        target.draw(hudBar);

        // 統計文字（純 ASCII）
        std::ostringstream stats;
        stats << "Total: " << totalNodes
              << " | Visible: " << visibleNodes
              << " | D:" << dialogueCount
              << " C:" << choiceCount
              << " A:" << actionCount
              << " Cond:" << conditionCount
              << " | Zoom: " << static_cast<int>(zoomScale * 100.0f) << "%";

        std::string statsStr = stats.str();
        drawText(target, statsStr, 18.0f, 6.0f, 13, sf::Color(220, 225, 240));

        // 搜尋框
        float searchX = 700.0f;
        float searchY = 8.0f;
        float searchW = 300.0f;
        float searchH = 26.0f;

        sf::RectangleShape searchBox(sf::Vector2f(searchW, searchH));
        searchBox.setPosition(sf::Vector2f(searchX, searchY));
        searchBox.setFillColor(searchActive ? sf::Color(40, 50, 70) : sf::Color(24, 28, 38));
        searchBox.setOutlineThickness(1.5f);
        searchBox.setOutlineColor(searchActive ? sf::Color(100, 149, 237) : sf::Color(60, 70, 90));
        target.draw(searchBox);

        std::string searchDisplay = searchQuery.empty() ? "[Press / to search]" : searchQuery;
        if (searchActive) searchDisplay += "_";

        drawText(target, searchDisplay,
                 searchX + 10.0f, searchY + 5.0f,
                 13,
                 searchQuery.empty() ? sf::Color(120, 130, 150) : sf::Color::White);

        // 過濾按鈕
        float filterX = searchX + searchW + 20.0f;
        drawFilterButton(target, filterX, searchY, "1:D", showDialogue, sf::Color(100, 149, 237));
        drawFilterButton(target, filterX + 60.0f, searchY, "2:C", showChoice, sf::Color(255, 215, 0));
        drawFilterButton(target, filterX + 120.0f, searchY, "3:A", showAction, sf::Color(80, 220, 80));
        drawFilterButton(target, filterX + 180.0f, searchY, "4:Cond", showCondition, sf::Color(240, 80, 80));

        // 底部提示
        drawText(target, "Arrows/WASD: Pan | +/-: Zoom | F: Focus | R: Reset | Esc: Close",
                 18.0f, targetSize.y - 20.0f, 12, sf::Color(160, 170, 190));
    }

    void drawFilterButton(sf::RenderTarget& target, float x, float y,
                          const std::string& label, bool enabled, sf::Color color) {
        float w = 55.0f;
        float h = 26.0f;

        sf::RectangleShape box(sf::Vector2f(w, h));
        box.setPosition(sf::Vector2f(x, y));
        box.setFillColor(enabled ? color : sf::Color(40, 45, 55));
        box.setOutlineThickness(1.0f);
        box.setOutlineColor(enabled ? sf::Color(255, 255, 255, 150) : sf::Color(80, 90, 105));
        target.draw(box);

        // ⚠️ 用輔助函式繪製，避免亂碼
        sf::Text text(font);
        text.setString(sf::String::fromUtf8(label.begin(), label.end()));
        text.setCharacterSize(12);
        text.setFillColor(enabled ? sf::Color(20, 20, 30) : sf::Color(120, 130, 150));
        sf::FloatRect tb = text.getLocalBounds();
        text.setPosition(sf::Vector2f(x + (w - tb.size.x) * 0.5f, y + (h - tb.size.y) * 0.5f - 3.0f));
        target.draw(text);
    }

    // ========================================================================
    // 滑鼠點擊
    // ========================================================================
    void handleMouseClick(const sf::Vector2i& mousePos,
                          const std::unordered_map<std::string, std::shared_ptr<StoryNode>>& nodeRegistry,
                          StoryExecutor& executor,
                          std::function<void()> onNodeJumped)
    {
        if (!isVisible) return;
        sf::Vector2f mPos(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

        // 搜尋框
        if (mPos.x >= 700.0f && mPos.x <= 1000.0f && mPos.y >= 8.0f && mPos.y <= 34.0f) {
            searchActive = true;
            return;
        }

        // 過濾按鈕
        float filterX = 1020.0f;
        if (mPos.y >= 8.0f && mPos.y <= 34.0f) {
            if (mPos.x >= filterX && mPos.x <= filterX + 55.0f) { showDialogue = !showDialogue; return; }
            if (mPos.x >= filterX + 60.0f && mPos.x <= filterX + 115.0f) { showChoice = !showChoice; return; }
            if (mPos.x >= filterX + 120.0f && mPos.x <= filterX + 175.0f) { showAction = !showAction; return; }
            if (mPos.x >= filterX + 180.0f && mPos.x <= filterX + 235.0f) { showCondition = !showCondition; return; }
        }

        // Inspector 編輯
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

        // 節點點擊
        auto toScreen = [&](sf::Vector2f worldPos) {
            return sf::Vector2f(
                viewOffset.x + worldPos.x * zoomScale,
                viewOffset.y + worldPos.y * zoomScale
            );
        };

        sf::Vector2f cardSize(baseCardWidth * zoomScale, baseCardHeight * zoomScale);

        for (const auto& [id, node] : nodeRegistry) {
            if (!passesFilter(node)) continue;
            if (nodePositions.find(id) == nodePositions.end()) continue;

            sf::Vector2f screenPos = toScreen(nodePositions[id]);
            sf::FloatRect bounds(screenPos, cardSize);

            if (bounds.contains(mPos)) {
                selectedNodeId = id;
                selectedNode = node;
                activeField = EditField::None;

                if (!cachedCurrentNodeId.empty() && cachedCurrentNodeId != id) {
                    pathHighlightTarget = id;
                    computePath(cachedCurrentNodeId, id, nodeRegistry);
                } else {
                    pathHighlightTarget = "";
                    highlightedPath.clear();
                }

                executor.jumpToNode(node);
                if (onNodeJumped) onNodeJumped();
                break;
            }
        }
    }
};