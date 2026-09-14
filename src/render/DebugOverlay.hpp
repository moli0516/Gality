#pragma once
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <functional>
#include "../core/Blackboard.hpp"
#include "../story/StoryExecutor.hpp"
#include "../story/StoryNode.hpp"

class DebugOverlay {
private:
    bool isVisible = false;
    std::vector<std::string> logHistory;

public:
    DebugOverlay() = default;

    void toggle() {
        isVisible = !isVisible;
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

    void draw(sf::RenderWindow& window, Blackboard& blackboard, StoryExecutor& executor, 
              const std::unordered_map<std::string, std::shared_ptr<StoryNode>>& nodeIndexMap,
              std::function<void()> onNodeJumped) 
    {
        if (!isVisible) return;

        ImGui::Begin("Gality Engine Debugger & Inspector", &isVisible, ImGuiWindowFlags_MenuBar);

        if (ImGui::BeginTabBar("DebugTabs")) {

            // -------------------------------------------------------------
            // Tab 1: Blackboard Variables Inspector
            // -------------------------------------------------------------
            if (ImGui::BeginTabItem("Blackboard Flags")) {
                ImGui::Text("Real-time Variable Inspector & Editor");
                ImGui::Separator();

                auto flags = blackboard.getAllInts();
                for (auto& [key, value] : flags) {
                    int currentVal = value;
                    if (ImGui::InputInt(key.c_str(), &currentVal)) {
                        blackboard.setInt(key, currentVal);
                        log("[Debug] Set " + key + " = " + std::to_string(currentVal));
                    }
                }
                ImGui::EndTabItem();
            }

            // -------------------------------------------------------------
            // Tab 2: FSM Node Jumper
            // -------------------------------------------------------------
            if (ImGui::BeginTabItem("Node Jumper")) {
                ImGui::Text("Force Jump to Any Story Node");
                ImGui::Separator();

                auto currentNode = executor.getCurrentNode();
                ImGui::Text("Current Node ID: %s", currentNode ? currentNode->id.c_str() : "NULL");
                ImGui::Separator();

                // ★ 修正：BeginChild 後必須無條件呼叫 EndChild
                ImGui::BeginChild("NodeList", ImVec2(0, 300), true);
                for (const auto& [nodeId, nodePtr] : nodeIndexMap) {
                    bool isCurrent = (currentNode && currentNode->id == nodeId);
                    std::string label = nodeId + " (" + 
                        (nodePtr->type == NodeType::Dialogue ? "Dialogue" : 
                         nodePtr->type == NodeType::Choice   ? "Choice" : 
                         nodePtr->type == NodeType::Condition ? "Condition" : "Action") + ")";
                    
                    if (ImGui::Selectable(label.c_str(), isCurrent)) {
                        executor.jumpToNode(nodePtr);
                        if (onNodeJumped) onNodeJumped();
                        log("[Debug] Jumped to node: " + nodeId);
                    }
                }
                ImGui::EndChild();   // 一定要呼叫

                ImGui::EndTabItem();
            }

            // -------------------------------------------------------------
            // Tab 3: System Performance & Diagnostics
            // -------------------------------------------------------------
            if (ImGui::BeginTabItem("Diagnostics")) {
                ImGui::Text("Framerate: %.1f FPS", ImGui::GetIO().Framerate);
                ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
                ImGui::Separator();
                ImGui::Text("Window Resolution: %dx%d", window.getSize().x, window.getSize().y);
                ImGui::EndTabItem();
            }

            // -------------------------------------------------------------
            // Tab 4: Engine Console Log
            // -------------------------------------------------------------
            if (ImGui::BeginTabItem("Console Log")) {
                if (ImGui::Button("Clear Log")) logHistory.clear();
                ImGui::Separator();

                // ★ 同樣修正
                ImGui::BeginChild("LogScroll", ImVec2(0, 250), true);
                for (const auto& msg : logHistory) {
                    ImGui::TextUnformatted(msg.c_str());
                }
                ImGui::EndChild();   // 一定要呼叫

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
    }
};