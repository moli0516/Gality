#pragma once
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>
#include <iostream>
#include <unordered_map>
#include "Blackboard.hpp"

using json = nlohmann::json;

struct SaveSnapshot {
    std::string currentNodeId;
    std::unordered_map<std::string, int> intFlags;
};

class SaveManager {
public:
    // 儲存狀態至指定 JSON 檔案
    static bool saveGame(const std::string& filePath, const std::string& currentNodeId, const Blackboard& blackboard) {
        try {
            json saveJson;
            saveJson["currentNodeId"] = currentNodeId;
            saveJson["flags"] = blackboard.getAllInts();

            std::ofstream file(filePath);
            if (!file.is_open()) return false;

            file << saveJson.dump(4);
            std::cout << "[SaveManager] Successfully saved to " << filePath << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[SaveManager] Save failed: " << e.what() << std::endl;
            return false;
        }
    }

    // 從 JSON 檔案載入狀態
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

            std::cout << "[SaveManager] Successfully loaded from " << filePath << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[SaveManager] Load failed: " << e.what() << std::endl;
            return false;
        }
    }
};