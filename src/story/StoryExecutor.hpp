#pragma once

// ============================================================================
// Standard Library
// ============================================================================
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <iostream>
#include <map>              // ← 新增（std::map）
#include <optional>         // ← 新增（std::optional）
#include <unordered_map>    // ← 新增（std::unordered_map）

// ============================================================================
// Project Headers
// ============================================================================
#include "StoryNode.hpp"
#include "../core/Blackboard.hpp"
#include "../core/SaveManager.hpp"

class StoryExecutor {
private:
    std::shared_ptr<StoryNode> currentNode;
    Blackboard& blackboard;

    // 狀態快照堆疊 (最高保留 128 步歷史)
    std::vector<SaveSnapshot> historyStack;
    const size_t maxHistoryCapacity = 128;

    // 用於記錄當前演出狀態快照
    SaveSnapshot currentPresentationState;

public:
    explicit StoryExecutor(Blackboard& bb) : blackboard(bb) {}

    void start(std::shared_ptr<StoryNode> rootNode) {
        historyStack.clear();
        currentNode = rootNode;
        evaluateCurrentNode();
    }

    void jumpToNode(std::shared_ptr<StoryNode> node) {
        currentNode = node;
        evaluateCurrentNode();
    }

    // 更新當前舞台外觀狀態，以確保 push 時能截取最新幀
    void updatePresentationState(const std::string& bg,
                                const std::map<CharSlot, std::string>& slots,
                                std::optional<CharSlot> activeSlot,
                                const std::string& weather,
                                const std::string& bgm) {
        currentPresentationState.bgImagePath = bg;
        currentPresentationState.slotTextures = slots;
        currentPresentationState.activeSlot = activeSlot;
        currentPresentationState.weather = weather;
        currentPresentationState.bgmPath = bgm;
    }

    void advance(int choiceIndex = -1) {
        if (!currentNode) return;

        // 僅對具備呈現性質的 Dialogue 節點建立可回滾快照
        if (currentNode->type == NodeType::Dialogue) {
            recordSnapshot();
        }

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
            SaveSnapshot snap = currentPresentationState;
            snap.currentNodeId = currentNode->id;
            snap.intFlags = blackboard.getAllInts();

            if (historyStack.size() >= maxHistoryCapacity) {
                historyStack.erase(historyStack.begin());
            }
            historyStack.push_back(std::move(snap));
        }
    }

    // 倒退回上一個有效對話節點
    bool rollback(SaveSnapshot& outSnapshot) {
        if (historyStack.empty()) return false;

        outSnapshot = historyStack.back();
        historyStack.pop_back();

        // 還原變數
        blackboard.setAllInts(outSnapshot.intFlags);
        return true;
    }

    bool canRollback() const {
        return !historyStack.empty();
    }

    // ⚠️ 核心修復：Dialogue / Choice 節點也執行一次 mutations
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
                // Dialogue / Choice：執行一次 mutations 後停止
                if (currentNode->actionFunc) {
                    currentNode->actionFunc(blackboard);
                    // 清空以避免重複執行
                    currentNode->actionFunc = nullptr;
                    currentNode->mutations.clear();
                }
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

    const SaveSnapshot& getCurrentPresentationState() const {
        return currentPresentationState;
    }
};