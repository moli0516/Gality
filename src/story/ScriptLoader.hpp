#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <optional>
#include <nlohmann/json.hpp>
#include "StoryNode.hpp"
#include "../core/Blackboard.hpp"
#include "../core/AssetPack.hpp"

using json = nlohmann::json;

class ScriptLoader {
public:
    inline static std::map<std::string, std::shared_ptr<StoryNode>> nodeRegistry;
    inline static std::string defaultStartNodeId = "node_01";

    static std::shared_ptr<StoryNode> parseScriptJson(const json& scriptJson) {
        if (scriptJson.contains("startNode")) {
            defaultStartNodeId = scriptJson["startNode"].get<std::string>();
        } else if (scriptJson.contains("start_node")) {
            defaultStartNodeId = scriptJson["start_node"].get<std::string>();
        } else if (scriptJson.contains("start")) {
            defaultStartNodeId = scriptJson["start"].get<std::string>();
        } else {
            defaultStartNodeId = "node_01";
        }

        nodeRegistry.clear();

        if (!scriptJson.contains("nodes") || !scriptJson["nodes"].is_array()) {
            std::cerr << "[ScriptLoader Error] Invalid JSON AST: missing 'nodes' array." << std::endl;
            return nullptr;
        }

        // --- Pass 1: 建立節點與解析屬性 ---
        for (const auto& item : scriptJson["nodes"]) {
            if (!item.contains("id")) continue;

            auto node = std::make_shared<StoryNode>();
            node->id = item["id"].get<std::string>();

            node->speaker = item.value("speaker", "");
            node->text = item.value("text", "");
            node->bg = item.value("bg", "");
            node->bgm = item.value("bgm", "");
            node->cv = item.value("cv", "");
            node->weather = item.value("weather", "none");
            node->shake = item.value("shake", 0.0f);
            node->nextNodeId = item.value("next", "");

            // 立繪槽位映射解析
            std::map<CharSlot, std::string> slots;
            if (item.contains("char_left")) slots[CharSlot::Left] = item["char_left"];
            
            if (item.contains("char_center")) slots[CharSlot::Center] = item["char_center"];
            else if (item.contains("char")) slots[CharSlot::Center] = item["char"];
            else if (item.contains("character")) slots[CharSlot::Center] = item["character"];
            
            if (item.contains("char_right")) slots[CharSlot::Right] = item["char_right"];
            
            for (auto it = slots.begin(); it != slots.end(); ) {
                if (it->second.empty()) it = slots.erase(it);
                else ++it;
            }
            node->slotTextures = slots;

            if (slots.count(CharSlot::Center)) {
                node->characterSpritePath = slots[CharSlot::Center];
            }

            if (item.contains("active_char")) {
                std::string act = item["active_char"];
                if (act == "left") node->activeSlot = CharSlot::Left;
                else if (act == "center") node->activeSlot = CharSlot::Center;
                else if (act == "right") node->activeSlot = CharSlot::Right;
            }

            node->bgImagePath = node->bg;
            node->bgmPath = node->bgm;
            node->voicePath = node->cv;

            node->transitionMask = item.value("transition_mask", item.value("trans", ""));
            node->transitionDuration = item.value("transition_duration", item.value("duration", 1.0f));

            if (item.contains("choices") && item["choices"].is_array()) {
                for (const auto& ch : item["choices"]) {
                    ChoiceOption opt;
                    opt.text = ch.value("text", "");
                    opt.targetNodeId = ch.value("next", ch.value("target", ""));
                    opt.conditionVar = ch.value("condition_var", "");
                    opt.conditionValue = ch.value("condition_val", 0);
                    node->choices.push_back(opt);
                }
            }

            if (item.contains("mutations") && item["mutations"].is_array()) {
                for (const auto& mutJson : item["mutations"]) {
                    VariableMutation mut;
                    mut.variable = mutJson.value("var", "");
                    mut.op = mutJson.value("op", "");
                    mut.value = mutJson.value("val", 0);
                    node->mutations.push_back(mut);
                }
            } else if (item.contains("flag")) {
                VariableMutation mut;
                mut.variable = item.value("flag", "");
                mut.op = "+=";
                mut.value = item.value("value", 0);
                node->mutations.push_back(mut);
            }

            // 節點型別嚴格推導
            std::string typeStr = item.value("type", "");
            if (typeStr == "action" || !node->mutations.empty()) {
                node->type = NodeType::Action;
            } else if (typeStr == "choice" || !node->choices.empty()) {
                node->type = NodeType::Choice;
            } else if (typeStr == "condition" || (item.contains("condition") && !item["condition"].is_null())) {
                node->type = NodeType::Condition;
            } else {
                node->type = NodeType::Dialogue;
            }

            if (node->type == NodeType::Action) {
                node->actionFunc = [node](Blackboard& bb) {
                    for (const auto& mut : node->mutations) {
                        if (mut.op == "+=") bb.addInt(mut.variable, mut.value);
                        else if (mut.op == "-=") bb.addInt(mut.variable, -mut.value);
                        else if (mut.op == "=") bb.setInt(mut.variable, mut.value);
                    }
                };
            }

            if (node->type == NodeType::Condition && item.contains("condition")) {
                const auto& cond = item["condition"];
                std::string flag = cond.value("var", cond.value("flag", ""));
                std::string op = cond.value("op", ">=");
                int threshold = cond.value("val", cond.value("value", 0));
                std::string tNext = cond.value("then", cond.value("trueNext", ""));
                std::string fNext = cond.value("else", cond.value("falseNext", ""));

                node->thenNodeId = tNext;
                node->elseNodeId = fNext;

                node->conditionFunc = [flag, op, threshold, tNext, fNext](Blackboard& bb) -> std::shared_ptr<StoryNode> {
                    int currentVal = bb.getInt(flag, 0);
                    bool passed = false;
                    
                    if (op == ">=") passed = (currentVal >= threshold);
                    else if (op == "<=") passed = (currentVal <= threshold);
                    else if (op == ">") passed = (currentVal > threshold);
                    else if (op == "<") passed = (currentVal < threshold);
                    else passed = (currentVal == threshold);

                    std::string target = passed ? tNext : fNext;
                    if (nodeRegistry.count(target)) return nodeRegistry[target];
                    return nullptr;
                };
            }

            nodeRegistry[node->id] = node;
        }

        // --- Pass 2: 拓撲鏈接 (Link Graph Pointers) ---
        for (auto& [id, node] : nodeRegistry) {
            if (!node->nextNodeId.empty() && nodeRegistry.count(node->nextNodeId)) {
                node->defaultNext = nodeRegistry[node->nextNodeId];
                node->nextNode = node->defaultNext;
            }
            for (auto& choice : node->choices) {
                if (!choice.targetNodeId.empty() && nodeRegistry.count(choice.targetNodeId)) {
                    choice.nextNode = nodeRegistry[choice.targetNodeId];
                }
            }
        }

        std::cout << "[ScriptLoader] Loaded and linked " << nodeRegistry.size() << " nodes." << std::endl;
        return nodeRegistry.count(defaultStartNodeId) ? nodeRegistry[defaultStartNodeId] : nullptr;
    }

    static std::shared_ptr<StoryNode> loadFromFile(const std::string& filePath) {
        std::string payload;
        if (AssetPack::readTextFile(filePath, payload, "data.pak")) {
            std::istringstream stream(payload);
            json scriptJson;
            try {
                stream >> scriptJson;
                return parseScriptJson(scriptJson);
            } catch (const json::parse_error& e) {
                std::cerr << "[ScriptLoader Error] JSON Parse failed from data.pak: " << e.what() << std::endl;
                return nullptr;
            }
        }

        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "[ScriptLoader Error] Cannot open file: " << filePath << std::endl;
            return nullptr;
        }

        json scriptJson;
        try {
            file >> scriptJson;
            return parseScriptJson(scriptJson);
        } catch (const json::parse_error& e) {
            std::cerr << "[ScriptLoader Error] JSON Parse failed from disk: " << e.what() << std::endl;
            return nullptr;
        }
    }
};