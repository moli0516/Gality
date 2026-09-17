#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <iostream>

class LocalizationManager {
public:
    static LocalizationManager& instance() {
        static LocalizationManager mgr;
        return mgr;
    }

    void initialize(const std::vector<std::string>& availableLanguages,
                    const std::string& defaultLanguage = "zh-TW") {
        available = availableLanguages;
        currentLanguage = defaultLanguage;

        std::cout << "[Localization] Available languages: ";
        for (const auto& lang : available) {
            std::cout << lang << " ";
        }
        std::cout << std::endl;
        std::cout << "[Localization] Current: " << currentLanguage << std::endl;
    }

    void setLanguage(const std::string& lang) {
        if (std::find(available.begin(), available.end(), lang) != available.end()) {
            currentLanguage = lang;
            std::cout << "[Localization] Language switched to: " << lang << std::endl;
        } else {
            std::cerr << "[Localization] Unsupported language: " << lang << std::endl;
        }
    }

    const std::string& getLanguage() const {
        return currentLanguage;
    }

    const std::vector<std::string>& getAvailableLanguages() const {
        return available;
    }

    bool isOriginalLanguage() const {
        return currentLanguage == "zh-TW";
    }

    int getLanguageIndex() const {
        auto it = std::find(available.begin(), available.end(), currentLanguage);
        if (it != available.end()) {
            return static_cast<int>(std::distance(available.begin(), it));
        }
        return 0;
    }

    std::string getDisplayName(const std::string& lang) const {
        static const std::unordered_map<std::string, std::string> names = {
            {"zh-TW", "繁體中文"},
            {"zh-CN", "简体中文"},
            {"en", "English"},
            {"ja", "日本語"},
            {"ko", "한국어"},
            {"es", "Español"},
            {"fr", "Français"},
            {"de", "Deutsch"},
            {"ru", "Русский"},
        };
        auto it = names.find(lang);
        return (it != names.end()) ? it->second : lang;
    }

private:
    LocalizationManager() = default;
    ~LocalizationManager() = default;
    LocalizationManager(const LocalizationManager&) = delete;
    LocalizationManager& operator=(const LocalizationManager&) = delete;

    std::vector<std::string> available;
    std::string currentLanguage = "zh-TW";
};