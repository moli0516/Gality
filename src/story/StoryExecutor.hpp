#pragma once
#include <memory>
#include <vector>
#include "StoryNode.hpp"
#include "../core/Blackboard.hpp"
#include "../core/SaveManager.hpp"

class StoryExecutor {
private:
    std::shared_ptr<StoryNode> currentNode;
    Blackboard& blackboard;
    
    // 🔙 Week 5: 狀態快照堆疊 (用於 Rollback)
    std::vector<SaveSnapshot> historyStack;

public:
    explicit StoryExecutor(Blackboard& bb) : blackboard(bb) {}

    void start(std::shared_ptr<StoryNode> rootNode) {
        historyStack.clear();
        currentNode = rootNode;
        evaluateCurrentNode();
    }

    void jumpToNode(std::shared_ptr<StoryNode> node) {
        currentNode = node;
    }

    void advance(int choiceIndex = -1) {
        if (!currentNode) return;

        // 推進前記錄當前快照
        recordSnapshot();

        if (currentNode->type == NodeType::Dialogue) {
            currentNode = currentNode->defaultNext;
        } else if (currentNode->type == NodeType::Choice) {
            if (choiceIndex >= 0 && choiceIndex < static_cast<int>(currentNode->choices.size())) {
                currentNode = currentNode->choices[choiceIndex].nextNode;
            }
        }

        evaluateCurrentNode();
    }

    void recordSnapshot() {
        if (currentNode) {
            historyStack.push_back({currentNode->id, blackboard.getAllInts()});
        }
    }

    // 🔙 倒退回上一個 Dialogue/Choice 節點
    bool rollback(SaveSnapshot& outSnapshot) {
        if (historyStack.empty()) return false;

        outSnapshot = historyStack.back();
        historyStack.pop_back();
        return true;
    }

    void evaluateCurrentNode() {
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
                break;
            }
        }
    }

    std::shared_ptr<StoryNode> getCurrentNode() const {
        return currentNode;
    }

    bool isFinished() const {
        return currentNode == nullptr;
    }
};