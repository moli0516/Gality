#pragma once
#include <nlohmann/json.hpp>
#include <SFML/Graphics.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "../core/AssetPack.hpp"

using json = nlohmann::json;

// ============================================================================
// 對話按鈕設定
// ============================================================================
struct DialogueButtonConfig {
    std::string id;
    std::string text;
    bool enabled = true;
};

struct DialogueButtonsStyle {
    bool enabled = true;
    std::string position = "bottom-right";
    float buttonWidth = 90.0f;
    float buttonHeight = 36.0f;
    float spacing = 8.0f;
    unsigned int fontSize = 14;
    float skipInterval = 0.05f;

    sf::Color normalBgColor{40, 45, 60, 220};
    sf::Color hoverBgColor{70, 90, 140, 240};
    sf::Color activeBgColor{100, 130, 200, 250};
    sf::Color outlineColor{80, 90, 110, 255};
    sf::Color hoverOutlineColor{140, 180, 255, 255};
    sf::Color textColor{220, 225, 235, 255};

    std::vector<DialogueButtonConfig> buttons;
};

// ============================================================================
// 角色縮放設定
// ============================================================================
struct CharacterScalingStyle {
    float single = 0.85f;
    float doubleSlot = 0.75f;
    float triple = 0.65f;
    float bottomOffset = 15.0f;
};

// ============================================================================
// 對話框 / 選項樣式
// ============================================================================
struct DialogueBoxStyle {
    float posX = 80.f, posY = 620.f;
    float width = 1760.f, height = 280.f;
    sf::Color bgColor{0, 0, 0, 215};
    sf::Color borderColor{255, 255, 255, 100};

    float nameBoxPosX = 80.f, nameBoxPosY = 550.f;
    float nameBoxWidth = 320.f, nameBoxHeight = 60.f;
    sf::Color nameBoxBgColor{40, 40, 90, 235};

    sf::Color nameTextColor{255, 220, 0, 255};
    sf::Color dialogueTextColor{255, 255, 255, 255};
    unsigned int nameFontSize = 28;
    unsigned int dialogueFontSize = 32;
};

struct ChoiceUIStyle {
    float width = 1100.f;
    float height = 70.f;
    float spacing = 25.f;
    float startY = 220.f;
    unsigned int fontSize = 28;

    sf::Color normalBgColor{30, 30, 50, 240};
    sf::Color hoverBgColor{70, 70, 130, 255};
    sf::Color normalOutlineColor{100, 100, 180, 255};
    sf::Color hoverOutlineColor{255, 215, 0, 255};
    sf::Color textColor{255, 255, 255, 255};
};

// ============================================================================
// UITheme
// ============================================================================
class UITheme {
private:
    static sf::Color parseColor(const json& arr) {
        if (arr.is_array() && arr.size() >= 4) {
            return sf::Color(
                static_cast<std::uint8_t>(arr[0].get<int>()),
                static_cast<std::uint8_t>(arr[1].get<int>()),
                static_cast<std::uint8_t>(arr[2].get<int>()),
                static_cast<std::uint8_t>(arr[3].get<int>())
            );
        }
        if (arr.is_array() && arr.size() == 3) {
            return sf::Color(
                static_cast<std::uint8_t>(arr[0].get<int>()),
                static_cast<std::uint8_t>(arr[1].get<int>()),
                static_cast<std::uint8_t>(arr[2].get<int>())
            );
        }
        return sf::Color::White;
    }

    void parseDialogueButtons(const json& j) {
        if (!j.contains("dialogueButtons")) {
            dialogueButtonsStyle.enabled = true;
            dialogueButtonsStyle.buttons = {
                {"auto",    "AUTO", true},
                {"skip",    "SKIP", true},
                {"backlog", "LOG",  true},
                {"hide",    "HIDE", true},
                {"menu",    "MENU", true}
            };
            return;
        }

        const auto& db = j["dialogueButtons"];
        dialogueButtonsStyle.enabled = db.value("enabled", true);
        dialogueButtonsStyle.position = db.value("position", "bottom-right");
        dialogueButtonsStyle.buttonWidth = db.value("buttonWidth", 90.0f);
        dialogueButtonsStyle.buttonHeight = db.value("buttonHeight", 36.0f);
        dialogueButtonsStyle.spacing = db.value("spacing", 8.0f);
        dialogueButtonsStyle.fontSize = db.value("fontSize", 14);
        dialogueButtonsStyle.skipInterval = db.value("skipInterval", 0.05f);

        if (db.contains("normalBgColor"))     dialogueButtonsStyle.normalBgColor = parseColor(db["normalBgColor"]);
        if (db.contains("hoverBgColor"))      dialogueButtonsStyle.hoverBgColor = parseColor(db["hoverBgColor"]);
        if (db.contains("activeBgColor"))     dialogueButtonsStyle.activeBgColor = parseColor(db["activeBgColor"]);
        if (db.contains("outlineColor"))      dialogueButtonsStyle.outlineColor = parseColor(db["outlineColor"]);
        if (db.contains("hoverOutlineColor")) dialogueButtonsStyle.hoverOutlineColor = parseColor(db["hoverOutlineColor"]);
        if (db.contains("textColor"))         dialogueButtonsStyle.textColor = parseColor(db["textColor"]);

        if (db.contains("buttons") && db["buttons"].is_array()) {
            dialogueButtonsStyle.buttons.clear();
            for (const auto& btn : db["buttons"]) {
                DialogueButtonConfig cfg;
                cfg.id = btn.value("id", "");
                cfg.text = btn.value("text", "");
                cfg.enabled = btn.value("enabled", true);
                if (!cfg.id.empty()) {
                    dialogueButtonsStyle.buttons.push_back(cfg);
                }
            }
        }
    }

    void parseCharacterScaling(const json& j) {
        if (!j.contains("characterScaling")) return;

        const auto& cs = j["characterScaling"];
        characterScaling.single = cs.value("single", 0.85f);
        characterScaling.doubleSlot = cs.value("double", 0.75f);
        characterScaling.triple = cs.value("triple", 0.65f);
        characterScaling.bottomOffset = cs.value("bottomOffset", 15.0f);
    }

public:
    DialogueBoxStyle dialogueStyle;
    ChoiceUIStyle choiceStyle;
    DialogueButtonsStyle dialogueButtonsStyle;
    CharacterScalingStyle characterScaling;

    bool loadFromFile(const std::string& filePath) {
        std::string payload;
        bool fromPak = AssetPack::readTextFile(filePath, payload, "data.pak");

        if (!fromPak) {
            std::ifstream file(filePath);
            if (!file.is_open()) {
                std::cerr << "[UITheme] Failed to open: " << filePath << std::endl;
                return false;
            }
            std::stringstream ss;
            ss << file.rdbuf();
            payload = ss.str();
        }

        try {
            json j = json::parse(payload);

            if (j.contains("dialogueBox")) {
                const auto& db = j["dialogueBox"];
                dialogueStyle.posX = db.value("posX", 80.f);
                dialogueStyle.posY = db.value("posY", 620.f);
                dialogueStyle.width = db.value("width", 1760.f);
                dialogueStyle.height = db.value("height", 280.f);
                if (db.contains("bgColor")) dialogueStyle.bgColor = parseColor(db["bgColor"]);
                if (db.contains("borderColor")) dialogueStyle.borderColor = parseColor(db["borderColor"]);

                dialogueStyle.nameBoxPosX = db.value("nameBoxPosX", 80.f);
                dialogueStyle.nameBoxPosY = db.value("nameBoxPosY", 550.f);
                dialogueStyle.nameBoxWidth = db.value("nameBoxWidth", 320.f);
                dialogueStyle.nameBoxHeight = db.value("nameBoxHeight", 60.f);
                if (db.contains("nameBoxBgColor")) dialogueStyle.nameBoxBgColor = parseColor(db["nameBoxBgColor"]);

                if (db.contains("nameTextColor")) dialogueStyle.nameTextColor = parseColor(db["nameTextColor"]);
                if (db.contains("dialogueTextColor")) dialogueStyle.dialogueTextColor = parseColor(db["dialogueTextColor"]);
                dialogueStyle.nameFontSize = db.value("nameFontSize", 28);
                dialogueStyle.dialogueFontSize = db.value("dialogueFontSize", 32);
            }

            if (j.contains("choiceUI")) {
                const auto& c = j["choiceUI"];
                choiceStyle.width = c.value("width", 1100.f);
                choiceStyle.height = c.value("height", 70.f);
                choiceStyle.spacing = c.value("spacing", 25.f);
                choiceStyle.startY = c.value("startY", 220.f);
                choiceStyle.fontSize = c.value("fontSize", 28);

                if (c.contains("normalBgColor")) choiceStyle.normalBgColor = parseColor(c["normalBgColor"]);
                if (c.contains("hoverBgColor")) choiceStyle.hoverBgColor = parseColor(c["hoverBgColor"]);
                if (c.contains("normalOutlineColor")) choiceStyle.normalOutlineColor = parseColor(c["normalOutlineColor"]);
                if (c.contains("hoverOutlineColor")) choiceStyle.hoverOutlineColor = parseColor(c["hoverOutlineColor"]);
                if (c.contains("textColor")) choiceStyle.textColor = parseColor(c["textColor"]);
            }

            parseDialogueButtons(j);
            parseCharacterScaling(j);

            std::cout << "[UITheme] Loaded successfully" << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[UITheme] Parse error: " << e.what() << std::endl;
            return false;
        }
    }
};