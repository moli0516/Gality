#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include <iostream>
#include "AssetPack.hpp"

class FontManager {
public:
    static FontManager& instance() {
        static FontManager mgr;
        return mgr;
    }

    void initialize(const std::unordered_map<std::string, std::string>& availableFonts,
                    const std::string& defaultName = "regular")
    {
        defaultFontName = defaultName;

        for (const auto& [name, path] : availableFonts) {
            loadFont(name, path);
        }

        if (fonts.find(defaultFontName) == fonts.end()) {
            std::cerr << "[FontManager] Default font '" << defaultFontName
                      << "' not loaded! Using first available." << std::endl;
            if (!fonts.empty()) {
                defaultFontName = fonts.begin()->first;
            } else {
                std::cerr << "[FontManager] WARNING: No fonts loaded!" << std::endl;
            }
        }

        std::cout << "[FontManager] Loaded " << fonts.size()
                  << " font(s). Default: '" << defaultFontName << "'" << std::endl;
    }

    bool loadFont(const std::string& name, const std::string& path) {
        if (fonts.find(name) != fonts.end()) return true;

        std::vector<std::uint8_t> bytes;
        bool fromPak = AssetPack::readFileFromPak(path, bytes, "data.pak");

        auto fontPtr = std::make_shared<sf::Font>();

        if (fromPak && !bytes.empty()) {
            if (!fontPtr->openFromMemory(bytes.data(), bytes.size())) {
                std::cerr << "[FontManager] Failed to load '" << name
                          << "' from memory: " << path << std::endl;
                return false;
            }
        } else {
            std::ifstream f(path, std::ios::binary | std::ios::ate);
            if (!f.is_open()) {
                std::cout << "[FontManager] Font '" << name
                          << "' not found at '" << path << "' (skipped)" << std::endl;
                return false;
            }

            auto size = f.tellg();
            f.seekg(0, std::ios::beg);
            bytes.resize(static_cast<size_t>(size));
            f.read(reinterpret_cast<char*>(bytes.data()), size);

            if (!fontPtr->openFromMemory(bytes.data(), bytes.size())) {
                std::cerr << "[FontManager] Failed to parse font: " << path << std::endl;
                return false;
            }
        }

        FontEntry entry;
        entry.font = fontPtr;
        entry.binaryBuffer = std::move(bytes);
        fonts[name] = std::move(entry);

        std::cout << "[FontManager] Loaded font '" << name << "' from " << path << std::endl;
        return true;
    }

    const sf::Font& getFont(const std::string& name = "") const {
        std::string key = name.empty() ? defaultFontName : name;

        auto it = fonts.find(key);
        if (it != fonts.end()) return *it->second.font;

        auto defaultIt = fonts.find(defaultFontName);
        if (defaultIt != fonts.end()) return *defaultIt->second.font;

        static sf::Font emptyFont;
        return emptyFont;
    }

    bool hasFont(const std::string& name) const {
        return fonts.find(name) != fonts.end();
    }

    const std::string& getDefaultFontName() const {
        return defaultFontName;
    }

private:
    FontManager() = default;
    ~FontManager() = default;
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;

    struct FontEntry {
        std::shared_ptr<sf::Font> font;
        std::vector<std::uint8_t> binaryBuffer;
    };

    std::unordered_map<std::string, FontEntry> fonts;
    std::string defaultFontName = "regular";
};