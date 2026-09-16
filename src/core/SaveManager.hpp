#pragma once
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>
#include <iostream>
#include <unordered_map>
#include <map>
#include <optional>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include "Blackboard.hpp"
#include "SafeTime.hpp"
#include "../story/StoryNode.hpp"

using json = nlohmann::json;

struct SaveSnapshot {
    std::string currentNodeId;
    std::unordered_map<std::string, int> intFlags;

    std::string bgImagePath;
    std::map<CharSlot, std::string> slotTextures;
    std::optional<CharSlot> activeSlot;
    std::string weather = "none";

    std::string bgmPath;

    std::string timestamp = "";

    void updateTimestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t in_time_t = std::chrono::system_clock::to_time_t(now);
        timestamp = SafeTime::formatLocalTime(in_time_t);
    }
};

class SaveManager {
public:
    static bool saveGame(const std::string& filePath, SaveSnapshot& snapshot) {
        try {
            snapshot.updateTimestamp();

            json saveJson;
            saveJson["currentNodeId"] = snapshot.currentNodeId;
            saveJson["flags"] = snapshot.intFlags;
            saveJson["bgImagePath"] = snapshot.bgImagePath;
            saveJson["weather"] = snapshot.weather;
            saveJson["bgmPath"] = snapshot.bgmPath;
            saveJson["timestamp"] = snapshot.timestamp;

            json slotsJson = json::object();
            for (const auto& [slot, path] : snapshot.slotTextures) {
                if (slot == CharSlot::Left) slotsJson["left"] = path;
                else if (slot == CharSlot::Center) slotsJson["center"] = path;
                else if (slot == CharSlot::Right) slotsJson["right"] = path;
            }
            saveJson["slotTextures"] = slotsJson;

            if (snapshot.activeSlot.has_value()) {
                if (snapshot.activeSlot.value() == CharSlot::Left) saveJson["activeSlot"] = "left";
                else if (snapshot.activeSlot.value() == CharSlot::Center) saveJson["activeSlot"] = "center";
                else if (snapshot.activeSlot.value() == CharSlot::Right) saveJson["activeSlot"] = "right";
            } else {
                saveJson["activeSlot"] = nullptr;
            }

            std::ofstream file(filePath);
            if (!file.is_open()) return false;
            file << saveJson.dump(4);
            std::cout << "[SaveManager] Saved: " << filePath << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[SaveManager] Save failed: " << e.what() << std::endl;
            return false;
        }
    }

    static bool loadGame(const std::string& filePath, SaveSnapshot& outSnapshot) {
        try {
            std::ifstream file(filePath);
            if (!file.is_open()) return false;

            json saveJson;
            file >> saveJson;

            outSnapshot.currentNodeId = saveJson.value("currentNodeId", "");
            if (saveJson.contains("flags")) {
                outSnapshot.intFlags = saveJson["flags"].get<std::unordered_map<std::string, int>>();
            }
            outSnapshot.bgImagePath = saveJson.value("bgImagePath", "");
            outSnapshot.weather = saveJson.value("weather", "none");
            outSnapshot.bgmPath = saveJson.value("bgmPath", "");
            outSnapshot.timestamp = saveJson.value("timestamp", "Unknown Time");

            outSnapshot.slotTextures.clear();
            if (saveJson.contains("slotTextures") && saveJson["slotTextures"].is_object()) {
                const auto& slots = saveJson["slotTextures"];
                if (slots.contains("left")) outSnapshot.slotTextures[CharSlot::Left] = slots["left"].get<std::string>();
                if (slots.contains("center")) outSnapshot.slotTextures[CharSlot::Center] = slots["center"].get<std::string>();
                if (slots.contains("right")) outSnapshot.slotTextures[CharSlot::Right] = slots["right"].get<std::string>();
            }

            if (saveJson.contains("activeSlot") && !saveJson["activeSlot"].is_null()) {
                std::string act = saveJson["activeSlot"].get<std::string>();
                if (act == "left") outSnapshot.activeSlot = CharSlot::Left;
                else if (act == "center") outSnapshot.activeSlot = CharSlot::Center;
                else if (act == "right") outSnapshot.activeSlot = CharSlot::Right;
            } else {
                outSnapshot.activeSlot = std::nullopt;
            }

            return true;
        } catch (const std::exception& e) {
            std::cerr << "[SaveManager] Load failed: " << e.what() << std::endl;
            return false;
        }
    }
};