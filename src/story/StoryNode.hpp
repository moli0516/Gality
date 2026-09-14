#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "../core/Blackboard.hpp"

enum class NodeType {
    Dialogue,   // 顯示對話與角色立繪
    Choice,     // 等待玩家選擇分歧
    Condition,  // 根據條件自動轉向分支
    Action      // 執行指令（如更改 Flag、增加好感度）
};

struct ChoiceOption {
    std::string text;
    std::shared_ptr<struct StoryNode> nextNode;
};

struct StoryNode {
    std::string id;
    NodeType type;

    // Dialogue 節點屬性
    std::string speaker;
    std::string text;
    std::string bgImagePath;
    std::string characterSpritePath;
    std::string bgmPath;
    std::string voicePath;

    // 💡 Week 10 天氣與震動屬性
    std::string weather; 
    float shake = 0.0f;

    // Choice / Branch 下一步連接
    std::vector<ChoiceOption> choices;
    std::shared_ptr<StoryNode> defaultNext;

    std::function<std::shared_ptr<StoryNode>(const Blackboard&)> conditionFunc;
    std::function<void(Blackboard&)> actionFunc;

    explicit StoryNode(std::string nodeId, NodeType nodeType) 
        : id(std::move(nodeId)), type(nodeType) {}
};