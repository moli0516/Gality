#pragma once
#include <memory>
#include <iostream>
#include "StoryNode.hpp"
#include "../core/Blackboard.hpp"

class StoryExecutor {
private:
    std::shared_ptr<StoryNode> currentNode;
    Blackboard& blackboard;

    void processCurrentNode();

public:
    explicit StoryExecutor(Blackboard& bb) : blackboard(bb) {}

    void start(std::shared_ptr<StoryNode> rootNode);
    void advance(size_t choiceIndex = 0);

    std::shared_ptr<StoryNode> getCurrentNode() const { return currentNode; }
    bool isFinished() const { return currentNode == nullptr; }
};