#include "StoryExecutor.hpp"

void StoryExecutor::start(std::shared_ptr<StoryNode> rootNode) {
    currentNode = rootNode;
    processCurrentNode();
}

void StoryExecutor::processCurrentNode() {
    if (!currentNode) return;

    switch (currentNode->type) {
        case NodeType::Action:
            if (currentNode->actionFunc) {
                currentNode->actionFunc(blackboard);
            }
            // 執行完動作後自動流轉到下一個節點
            currentNode = currentNode->defaultNext;
            processCurrentNode();
            break;

        case NodeType::Condition:
            if (currentNode->conditionFunc) {
                currentNode = currentNode->conditionFunc(blackboard);
            } else {
                currentNode = currentNode->defaultNext;
            }
            processCurrentNode();
            break;

        case NodeType::Dialogue:
        case NodeType::Choice:
            // 暫停流轉，等待玩家操作
            break;
    }
}

void StoryExecutor::advance(size_t choiceIndex) {
    if (!currentNode) return;

    if (currentNode->type == NodeType::Dialogue) {
        currentNode = currentNode->defaultNext;
        processCurrentNode();
    } else if (currentNode->type == NodeType::Choice) {
        if (choiceIndex < currentNode->choices.size()) {
            currentNode = currentNode->choices[choiceIndex].nextNode;
            processCurrentNode();
        }
    }
}