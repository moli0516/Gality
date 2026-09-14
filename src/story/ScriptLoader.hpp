#pragma once
#include <nlohmann/json.hpp>
#include <fstream>
#include <unordered_map>
#include <memory>
#include <iostream>
#include "StoryNode.hpp"

using json = nlohmann::json;

class ScriptLoader {
public:
    static std::shared_ptr<StoryNode> loadFromFile(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Failed to open script file: " << filePath << std::endl;
            return nullptr;
        }

        json scriptJson;
        file >> scriptJson;

        std::unordered_map<std::string, std::shared_ptr<StoryNode>> nodeMap;

        // 第一階段：構建所有節點實例
        for (const auto& item : scriptJson["nodes"]) {
            std::string id = item["id"];
            std::string typeStr = item["type"];
            NodeType type = NodeType::Dialogue;

            if (typeStr == "choice") type = NodeType::Choice;
            else if (typeStr == "condition") type = NodeType::Condition;
            else if (typeStr == "action") type = NodeType::Action;

            auto node = std::make_shared<StoryNode>(id, type);

            if (type == NodeType::Dialogue) {
                node->speaker = item.value("speaker", "");
                node->text = item.value("text", "");
                node->bgImagePath = item.value("bg", "");
                node->characterSpritePath = item.value("character", "");
                node->bgmPath = item.value("bgm", "");
                node->voicePath = item.value("cv", "");

                // 💡 Week 10: 載入 Weather 與 Shake
                node->weather = item.value("weather", "");
                node->shake = item.value("shake", 0.0f);
            } else if (type == NodeType::Action) {
                std::string flag = item["flag"];
                int val = item["value"];
                node->actionFunc = [flag, val](Blackboard& bb) {
                    bb.addInt(flag, val);
                };
            }

            nodeMap[id] = node;
        }

        // 第二階段：建立 Link 關係
        for (const auto& item : scriptJson["nodes"]) {
            std::string id = item["id"];
            auto node = nodeMap[id];

            if (item.contains("next")) {
                std::string nextId = item["next"];
                if (nodeMap.count(nextId)) node->defaultNext = nodeMap[nextId];
            }

            if (item.contains("choices")) {
                for (const auto& c : item["choices"]) {
                    ChoiceOption opt;
                    opt.text = c["text"];
                    std::string targetId = c["next"];
                    if (nodeMap.count(targetId)) opt.nextNode = nodeMap[targetId];
                    node->choices.push_back(opt);
                }
            }

            if (node->type == NodeType::Condition) {
                std::string flag = item["condition"]["flag"];
                int targetVal = item["condition"]["value"];
                std::string trueId = item["condition"]["trueNext"];
                std::string falseId = item["condition"]["falseNext"];

                node->conditionFunc = [flag, targetVal, trueId, falseId, nodeMap](const Blackboard& bb) {
                    if (bb.getInt(flag) >= targetVal) {
                        return nodeMap.at(trueId);
                    }
                    return nodeMap.at(falseId);
                };
            }
        }

        std::string startId = scriptJson.value("startNode", "start");
        return nodeMap.count(startId) ? nodeMap[startId] : nullptr;
    }
};