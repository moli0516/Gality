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
#include "../core/LocalizationManager.hpp"

using json = nlohmann::json;

class ScriptLoader {
public:
    inline static std::map<std::string, std::shared_ptr<StoryNode>> nodeRegistry;
    inline static std::string defaultStartNodeId = "node_01";

    static std::shared_ptr<StoryNode> parseScriptJson(const json& scriptJson) {
        // ===== 多語言處理 =====
        const std::string currentLang = LocalizationManager::instance().getLanguage();
        json workingJson = scriptJson;

        if (currentLang != "zh-TW") {
            std::string i18nKey = "i18n_" + currentLang;
            if (scriptJson.contains(i18nKey) &&
                scriptJson[i18nKey].contains("nodes")) {
                const auto& i18nNodes = scriptJson[i18nKey]["nodes"];
                std::cout << "[ScriptLoader] Applying i18n: " << currentLang << std::endl;

                for (size_t i = 0; i < workingJson["nodes"].size() &&
                                   i < i18nNodes.size(); ++i) {
                    auto& node = workingJson["nodes"][i];
                    const auto& i18nNode = i18nNodes[i];

                    if (i18nNode.contains("text") &&
                        !i18nNode["text"].get<std::string>().empty()) {
                        node["text"] = i18nNode["text"];
                    }

                    if (i18nNode.contains("choices") && node.contains("choices") &&
                        node["choices"].is_array()) {
                        const auto& i18nChoices = i18nNode["choices"];
                        for (size_t j = 0; j < node["choices"].size() &&
                                           j < i18nChoices.size(); ++j) {
                            if (i18nChoices[j].contains("text")) {
                                node["choices"][j]["text"] = i18nChoices[j]["text"];
                            }
                        }
                    }
                }
            }
        }

        // ===== 解析 start =====
        if (workingJson.contains("startNode")) {
            defaultStartNodeId = workingJson["startNode"].get<std::string>();
        } else if (workingJson.contains("start_node")) {
            defaultStartNodeId = workingJson["start_node"].get<std::string>();
        } else if (workingJson.contains("start")) {
            defaultStartNodeId = workingJson["start"].get<std::string>();
        } else {
            defaultStartNodeId = "node_01";
        }

        nodeRegistry.clear();

        if (!workingJson.contains("nodes") || !workingJson["nodes"].is_array()) {
            std::cerr << "[ScriptLoader Error] Invalid JSON AST." << std::endl;
            return nullptr;
        }

        // --- Pass 1: 建立節點 ---
        for (const auto& item : workingJson["nodes"]) {
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
            node->transitionDuration = item.value("transition_duration",
                                                   item.value("duration", 1.0f));

            node->noSkip = item.value("no_skip", false);
            node->noSkipWait = item.value("wait", 0.0f);

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

            std::string typeStr = item.value("type", "");
            if (typeStr == "action") {
                node->type = NodeType::Action;
            } else if (typeStr == "choice" || !node->choices.empty()) {
                node->type = NodeType::Choice;
            } else if (typeStr == "condition" ||
                       (item.contains("condition") && !item["condition"].is_null())) {
                node->type = NodeType::Condition;
            } else {
                node->type = NodeType::Dialogue;
            }

            if (!node->mutations.empty()) {
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

                node->conditionFunc = [flag, op, threshold, tNext, fNext]
                                      (Blackboard& bb) -> std::shared_ptr<StoryNode> {
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

        // --- Pass 2: 拓撲鏈接 ---
        for (auto& [id, node] : nodeRegistry) {
            if (!node->nextNodeId.empty() && nodeRegistry.count(node->nextNodeId)) {
                node->defaultNext = nodeRegistry[node->nextNodeId];
                node->nextNode = node->defaultNext;
            }
            for (auto& choice : node->choices) {
                if (!choice.targetNodeId.empty() &&
                    nodeRegistry.count(choice.targetNodeId)) {
                    choice.nextNode = nodeRegistry[choice.targetNodeId];
                }
            }
        }

        std::cout << "[ScriptLoader] Loaded " << nodeRegistry.size()
                  << " nodes (lang: " << currentLang << ")" << std::endl;

        return nodeRegistry.count(defaultStartNodeId)
            ? nodeRegistry[defaultStartNodeId] : nullptr;
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
                std::cerr << "[ScriptLoader] JSON Parse failed: "
                          << e.what() << std::endl;
                return nullptr;
            }
        }

        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "[ScriptLoader] Cannot open: " << filePath << std::endl;
            return nullptr;
        }

        json scriptJson;
        try {
            file >> scriptJson;
            return parseScriptJson(scriptJson);
        } catch (const json::parse_error& e) {
            std::cerr << "[ScriptLoader] JSON Parse failed: "
                      << e.what() << std::endl;
            return nullptr;
        }
    }
};