#pragma once
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "AssetPack.hpp"

struct EngineConfig {
    // 音訊
    float masterVolume = 100.0f;
    float bgmVolume = 80.0f;
    float voiceVolume = 100.0f;
    float sfxVolume = 100.0f;
    float textSpeed = 0.04f;

    // 畫面
    int renderScale = 2;
    bool enablePostFX = true;
    bool enableTransitions = true;
    int particleCount = 150;
    bool enableVsync = true;
    int windowMode = 0;

    // 語言
    std::string language = "zh-TW";
    std::vector<std::string> availableLanguages = {"zh-TW", "en", "ja"};
};

class ConfigManager {
public:
    inline static EngineConfig config;
    inline static std::string configPath = "assets/config/settings.json";

    static void load() {
        std::string payload;
        if (AssetPack::readTextFile(configPath, payload, "data.pak")) {
            try {
                std::istringstream stream(payload);
                nlohmann::json j;
                stream >> j;
                parseJson(j);
                std::cout << "[ConfigManager] Settings loaded from archive." << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "[ConfigManager] Load error: " << e.what() << std::endl;
            }
            return;
        }

        std::ifstream file(configPath);
        if (file.is_open()) {
            try {
                nlohmann::json j;
                file >> j;
                parseJson(j);
                std::cout << "[ConfigManager] Settings loaded." << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "[ConfigManager] Load error: " << e.what() << std::endl;
            }
        }
    }

    static void save() {
        try {
            nlohmann::json j;
            j["masterVolume"]       = config.masterVolume;
            j["bgmVolume"]          = config.bgmVolume;
            j["voiceVolume"]        = config.voiceVolume;
            j["sfxVolume"]          = config.sfxVolume;
            j["textSpeed"]          = config.textSpeed;
            j["renderScale"]        = config.renderScale;
            j["enablePostFX"]       = config.enablePostFX;
            j["enableTransitions"]  = config.enableTransitions;
            j["particleCount"]      = config.particleCount;
            j["enableVsync"]        = config.enableVsync;
            j["windowMode"]         = config.windowMode;
            j["language"]           = config.language;
            j["availableLanguages"] = config.availableLanguages;

            std::ofstream file(configPath);
            file << j.dump(4);
            std::cout << "[ConfigManager] Settings saved." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[ConfigManager] Save error: " << e.what() << std::endl;
        }
    }

private:
    static void parseJson(const nlohmann::json& j) {
        config.masterVolume      = j.value("masterVolume", 100.0f);
        config.bgmVolume         = j.value("bgmVolume", 80.0f);
        config.voiceVolume       = j.value("voiceVolume", 100.0f);
        config.sfxVolume         = j.value("sfxVolume", 100.0f);
        config.textSpeed         = j.value("textSpeed", 0.04f);
        config.renderScale       = j.value("renderScale", 2);
        config.enablePostFX      = j.value("enablePostFX", true);
        config.enableTransitions = j.value("enableTransitions", true);
        config.particleCount     = j.value("particleCount", 150);
        config.enableVsync       = j.value("enableVsync", true);
        config.windowMode        = j.value("windowMode", 0);
        config.language          = j.value("language", "zh-TW");
        config.availableLanguages = j.value("availableLanguages",
            std::vector<std::string>{"zh-TW", "en", "ja"});
    }
};