#pragma once
#include <memory>
#include "StoryNode.hpp"
#include "../core/Blackboard.hpp"

class StoryExecutor {
private:
    std::shared_ptr<StoryNode> currentNode;
    Blackboard& blackboard;

public:
    explicit StoryExecutor(Blackboard& bb);

    void start(std::shared_ptr<StoryNode> rootNode);
    void jumpToNode(std::shared_ptr<StoryNode> node);
    void advance(int choiceIndex = -1);
    void evaluateCurrentNode();

    std::shared_ptr<StoryNode> getCurrentNode() const;
    bool isFinished() const;
};