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

    // Choice / Branch 下一步連接
    std::vector<ChoiceOption> choices;
    std::shared_ptr<StoryNode> defaultNext;

    // Condition 節點的評估邏輯: 傳入 Blackboard, 回傳該跳轉的目標節點
    std::function<std::shared_ptr<StoryNode>(const Blackboard&)> conditionFunc;

    // Action 節點執行的動作
    std::function<void(Blackboard&)> actionFunc;

    explicit StoryNode(std::string nodeId, NodeType nodeType) 
        : id(std::move(nodeId)), type(nodeType) {}
};