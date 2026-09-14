#pragma once
#include <nlohmann/json.hpp>
#include <SFML/Graphics.hpp>
#include <fstream>
#include <iostream>
#include <string>

using json = nlohmann::json;

struct DialogueBoxStyle {
    float posX = 40.f, posY = 480.f;
    float width = 1200.f, height = 200.f;
    sf::Color bgColor{0, 0, 0, 200};
    sf::Color borderColor{255, 255, 255, 100};

    float nameBoxPosX = 40.f, nameBoxPosY = 430.f;
    float nameBoxWidth = 250.f, nameBoxHeight = 45.f;
    sf::Color nameBoxBgColor{40, 40, 90, 220};

    sf::Color nameTextColor{255, 255, 0, 255};
    sf::Color dialogueTextColor{255, 255, 255, 255};
    unsigned int nameFontSize = 22;
    unsigned int dialogueFontSize = 24;
};

struct ChoiceUIStyle {
    float width = 800.f;
    float height = 55.f;
    float spacing = 20.f;
    float startY = 220.f;
    unsigned int fontSize = 22;

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
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "[UITheme] Failed to open theme file: " << filePath << ", using default values." << std::endl;
            return false;
        }

        try {
            json j;
            file >> j;

            if (j.contains("dialogueBox")) {
                const auto& db = j["dialogueBox"];
                dialogueStyle.posX = db.value("posX", 40.f);
                dialogueStyle.posY = db.value("posY", 480.f);
                dialogueStyle.width = db.value("width", 1200.f);
                dialogueStyle.height = db.value("height", 200.f);
                dialogueStyle.bgColor = parseColor(db["bgColor"]);
                dialogueStyle.borderColor = parseColor(db["borderColor"]);

                dialogueStyle.nameBoxPosX = db.value("nameBoxPosX", 40.f);
                dialogueStyle.nameBoxPosY = db.value("nameBoxPosY", 430.f);
                dialogueStyle.nameBoxWidth = db.value("nameBoxWidth", 250.f);
                dialogueStyle.nameBoxHeight = db.value("nameBoxHeight", 45.f);
                dialogueStyle.nameBoxBgColor = parseColor(db["nameBoxBgColor"]);

                dialogueStyle.nameTextColor = parseColor(db["nameTextColor"]);
                dialogueStyle.dialogueTextColor = parseColor(db["dialogueTextColor"]);
                dialogueStyle.nameFontSize = db.value("nameFontSize", 22);
                dialogueStyle.dialogueFontSize = db.value("dialogueFontSize", 24);
            }

            if (j.contains("choiceUI")) {
                const auto& c = j["choiceUI"];
                choiceStyle.width = c.value("width", 800.f);
                choiceStyle.height = c.value("height", 55.f);
                choiceStyle.spacing = c.value("spacing", 20.f);
                choiceStyle.startY = c.value("startY", 220.f);
                choiceStyle.fontSize = c.value("fontSize", 22);

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