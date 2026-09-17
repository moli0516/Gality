#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

class Blackboard;

enum class CharSlot {
    Left,
    Center,
    Right
};

enum class NodeType {
    Dialogue,
    Choice,
    Action,
    Condition
};

struct VariableMutation {
    std::string variable;
    std::string op;
    int value = 0;
};

class StoryNode;
struct ChoiceOption {
    std::string text;
    std::string targetNodeId;
    std::string conditionVar;
    int conditionValue = 0;
    std::shared_ptr<StoryNode> nextNode = nullptr;
};

class StoryNode {
public:
    std::string id;
    NodeType type = NodeType::Dialogue;

    std::string speaker;
    std::string text;

    std::string bg;
    std::string bgm;
    std::string cv;
    std::string bgImagePath;
    std::string bgmPath;
    std::string voicePath;
    std::string characterSpritePath;

    std::string weather = "none";
    float shake = 0.0f;

    std::string transitionMask = "";
    float transitionDuration = 1.0f;

    std::map<CharSlot, std::string> slotTextures;
    CharSlot activeSlot = CharSlot::Center;

    std::vector<ChoiceOption> choices;

    std::vector<VariableMutation> mutations;
    std::function<void(Blackboard&)> actionFunc = nullptr;

    std::string thenNodeId;
    std::string elseNodeId;
    std::function<std::shared_ptr<StoryNode>(Blackboard&)> conditionFunc = nullptr;

    std::string nextNodeId;
    std::shared_ptr<StoryNode> defaultNext = nullptr;
    std::shared_ptr<StoryNode> nextNode = nullptr;

    // ⚠️ 新增：強制觀看
    bool noSkip = false;
    float noSkipWait = 0.0f;

    StoryNode() = default;
    StoryNode(const std::string& nodeId, NodeType nodeType)
        : id(nodeId), type(nodeType) {}
};