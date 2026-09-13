#include "StoryExecutor.hpp"

StoryExecutor::StoryExecutor(Blackboard& bb) : blackboard(bb) {}

void StoryExecutor::start(std::shared_ptr<StoryNode> rootNode) {
    currentNode = rootNode;
    evaluateCurrentNode();
}

void StoryExecutor::jumpToNode(std::shared_ptr<StoryNode> node) {
    currentNode = node;
}

void StoryExecutor::advance(int choiceIndex) {
    if (!currentNode) return;

    if (currentNode->type == NodeType::Dialogue) {
        currentNode = currentNode->defaultNext;
    } else if (currentNode->type == NodeType::Choice) {
        if (choiceIndex >= 0 && choiceIndex < static_cast<int>(currentNode->choices.size())) {
            currentNode = currentNode->choices[choiceIndex].nextNode;
        }
    }

    evaluateCurrentNode();
}

void StoryExecutor::evaluateCurrentNode() {
    while (currentNode) {
        if (currentNode->type == NodeType::Action) {
            if (currentNode->actionFunc) {
                currentNode->actionFunc(blackboard);
            }
            currentNode = currentNode->defaultNext;
        } else if (currentNode->type == NodeType::Condition) {
            if (currentNode->conditionFunc) {
                currentNode = currentNode->conditionFunc(blackboard);
            } else {
                currentNode = currentNode->defaultNext;
            }
        } else {
            break; // Dialogue 或 Choice 節點，停下等待渲染與玩家操作
        }
    }
}

std::shared_ptr<StoryNode> StoryExecutor::getCurrentNode() const {
    return currentNode;
}

bool StoryExecutor::isFinished() const {
    return currentNode == nullptr;
}