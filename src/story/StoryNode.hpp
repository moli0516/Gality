#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

// 前向宣告 Blackboard 避免循環引用
class Blackboard;

// 角色站位列舉
enum class CharSlot {
    Left,
    Center,
    Right
};

// 節點型別列舉
enum class NodeType {
    Dialogue,
    Choice,
    Action,
    Condition
};

// 變數操作運算 (用於 Blackboard)
struct VariableMutation {
    std::string variable;
    std::string op; // "+=", "-=", "="
    int value = 0;
};

// 分支選項定義
class StoryNode;
struct ChoiceOption {
    std::string text;
    std::string targetNodeId;
    std::string conditionVar;
    int conditionValue = 0;
    std::shared_ptr<StoryNode> nextNode = nullptr; // 預先鏈接好的下一個節點
};

class StoryNode {
public:
    std::string id;
    NodeType type = NodeType::Dialogue;

    // 基本文本屬性
    std::string speaker;
    std::string text;

    // 資源路徑 (相容 bg/bgm/cv 與完整命名)
    std::string bg;
    std::string bgm;
    std::string cv;
    std::string bgImagePath;
    std::string bgmPath;
    std::string voicePath;
    std::string characterSpritePath;

    // 環境與特效
    std::string weather = "none";
    float shake = 0.0f;

    // Week 14: 規則遮罩轉場屬性
    std::string transitionMask = "";
    float transitionDuration = 1.0f;

    // 多立繪站位系統
    std::map<CharSlot, std::string> slotTextures;
    CharSlot activeSlot = CharSlot::Center;

    // 分支選項清單
    std::vector<ChoiceOption> choices;

    // Blackboard 數值操作與條件分支
    std::vector<VariableMutation> mutations;
    std::function<void(Blackboard&)> actionFunc = nullptr;

    std::string thenNodeId;
    std::string elseNodeId;
    std::function<std::shared_ptr<StoryNode>(Blackboard&)> conditionFunc = nullptr;

    // 順序流向指針
    std::string nextNodeId;
    std::shared_ptr<StoryNode> defaultNext = nullptr;
    std::shared_ptr<StoryNode> nextNode = nullptr;

    StoryNode() = default;
    StoryNode(const std::string& nodeId, NodeType nodeType)
        : id(nodeId), type(nodeType) {}
};