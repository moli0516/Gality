#pragma once
#include <nlohmann/json.hpp>
#include <SFML/Graphics.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "../core/AssetPack.hpp"

using json = nlohmann::json;

struct DialogueBoxStyle {
    float posX = 80.f, posY = 740.f;
    float width = 1760.f, height = 280.f;
    sf::Color bgColor{0, 0, 0, 215};
    sf::Color borderColor{255, 255, 255, 100};

    float nameBoxPosX = 80.f, nameBoxPosY = 670.f;
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
    float startY = 340.f;
    unsigned int fontSize = 28;

    sf::Color normalBgColor{30, 30, 50, 220};
    sf::Color hoverBgColor{70, 70, 130, 240};
    sf::Color normalOutlineColor{100, 100, 180, 255};
    sf::Color hoverOutlineColor{255, 215, 0, 255};
    sf::Color textColor{255, 255, 255, 255};
};

class UITheme {
private:
    static sf::Color parseColor(const json& arr) {
        if (arr.is_array() && arr.size() >= 4) {
            return sf::Color(arr[0].get<uint8_t>(), arr[1].get<uint8_t>(), arr[2].get<uint8_t>(), arr[3].get<uint8_t>());
        }
        return sf::Color::White;
    }

public:
    DialogueBoxStyle dialogueStyle;
    ChoiceUIStyle choiceStyle;

    bool loadFromFile(const std::string& filePath) {
        std::string payload;
        if (AssetPack::readTextFile(filePath, payload, "data.pak")) {
            try {
                std::istringstream stream(payload);
                json j;
                stream >> j;

                if (j.contains("dialogueBox")) {
                    const auto& db = j["dialogueBox"];
                    dialogueStyle.posX = db.value("posX", 80.f);
                    dialogueStyle.posY = db.value("posY", 740.f);
                    dialogueStyle.width = db.value("width", 1760.f);
                    dialogueStyle.height = db.value("height", 280.f);
                    dialogueStyle.bgColor = parseColor(db["bgColor"]);
                    dialogueStyle.borderColor = parseColor(db["borderColor"]);

                    dialogueStyle.nameBoxPosX = db.value("nameBoxPosX", 80.f);
                    dialogueStyle.nameBoxPosY = db.value("nameBoxPosY", 670.f);
                    dialogueStyle.nameBoxWidth = db.value("nameBoxWidth", 320.f);
                    dialogueStyle.nameBoxHeight = db.value("nameBoxHeight", 60.f);
                    dialogueStyle.nameBoxBgColor = parseColor(db["nameBoxBgColor"]);

                    dialogueStyle.nameTextColor = parseColor(db["nameTextColor"]);
                    dialogueStyle.dialogueTextColor = parseColor(db["dialogueTextColor"]);
                    dialogueStyle.nameFontSize = db.value("nameFontSize", 28);
                    dialogueStyle.dialogueFontSize = db.value("dialogueFontSize", 32);
                }

                if (j.contains("choiceUI")) {
                    const auto& c = j["choiceUI"];
                    choiceStyle.width = c.value("width", 1100.f);
                    choiceStyle.height = c.value("height", 70.f);
                    choiceStyle.spacing = c.value("spacing", 25.f);
                    choiceStyle.startY = c.value("startY", 340.f);
                    choiceStyle.fontSize = c.value("fontSize", 28);

                    choiceStyle.normalBgColor = parseColor(c["normalBgColor"]);
                    choiceStyle.hoverBgColor = parseColor(c["hoverBgColor"]);
                    choiceStyle.normalOutlineColor = parseColor(c["normalOutlineColor"]);
                    choiceStyle.hoverOutlineColor = parseColor(c["hoverOutlineColor"]);
                    choiceStyle.textColor = parseColor(c["textColor"]);
                }

                std::cout << "[UITheme] Loaded UI configuration successfully from archive." << std::endl;
                return true;
            } catch (const std::exception& e) {
                std::cerr << "[UITheme] Parse error: " << e.what() << std::endl;
                return false;
            }
        }

        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "[UITheme] Failed to open theme file: " << filePath << ", using 1080p defaults." << std::endl;
            return false;
        }

        try {
            json j;
            file >> j;

            if (j.contains("dialogueBox")) {
                const auto& db = j["dialogueBox"];
                dialogueStyle.posX = db.value("posX", 80.f);
                dialogueStyle.posY = db.value("posY", 740.f);
                dialogueStyle.width = db.value("width", 1760.f);
                dialogueStyle.height = db.value("height", 280.f);
                dialogueStyle.bgColor = parseColor(db["bgColor"]);
                dialogueStyle.borderColor = parseColor(db["borderColor"]);

                dialogueStyle.nameBoxPosX = db.value("nameBoxPosX", 80.f);
                dialogueStyle.nameBoxPosY = db.value("nameBoxPosY", 670.f);
                dialogueStyle.nameBoxWidth = db.value("nameBoxWidth", 320.f);
                dialogueStyle.nameBoxHeight = db.value("nameBoxHeight", 60.f);
                dialogueStyle.nameBoxBgColor = parseColor(db["nameBoxBgColor"]);

                dialogueStyle.nameTextColor = parseColor(db["nameTextColor"]);
                dialogueStyle.dialogueTextColor = parseColor(db["dialogueTextColor"]);
                dialogueStyle.nameFontSize = db.value("nameFontSize", 28);
                dialogueStyle.dialogueFontSize = db.value("dialogueFontSize", 32);
            }

            if (j.contains("choiceUI")) {
                const auto& c = j["choiceUI"];
                choiceStyle.width = c.value("width", 1100.f);
                choiceStyle.height = c.value("height", 70.f);
                choiceStyle.spacing = c.value("spacing", 25.f);
                choiceStyle.startY = c.value("startY", 340.f);
                choiceStyle.fontSize = c.value("fontSize", 28);

                choiceStyle.normalBgColor = parseColor(c["normalBgColor"]);
                choiceStyle.hoverBgColor = parseColor(c["hoverBgColor"]);
                choiceStyle.normalOutlineColor = parseColor(c["normalOutlineColor"]);
                choiceStyle.hoverOutlineColor = parseColor(c["hoverOutlineColor"]);
                choiceStyle.textColor = parseColor(c["textColor"]);
            }

            std::cout << "[UITheme] Loaded UI configuration successfully." << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[UITheme] Parse error: " << e.what() << std::endl;
            return false;
        }
    }
};