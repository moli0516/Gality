#pragma once

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace AssetPack {
    inline std::string normalizePath(const std::string& input) {
        std::string result = input;
        std::replace(result.begin(), result.end(), '\\', '/');

        while (!result.empty() && result[0] == '.') {
            if (result.size() >= 2 && result[0] == '.' && result[1] == '/') {
                result.erase(0, 2);
            } else if (result == ".") {
                result.clear();
                break;
            } else {
                break;
            }
        }

        while (!result.empty() && result.front() == '/') {
            result.erase(result.begin());
        }

        if (result.rfind("assets/", 0) == 0) {
            result.erase(0, std::string("assets/").size());
        }

        return result;
    }

    inline std::uint16_t readU16LE(const std::vector<std::uint8_t>& data, std::size_t pos) {
        return static_cast<std::uint16_t>(
            static_cast<std::uint16_t>(data[pos]) |
            (static_cast<std::uint16_t>(data[pos + 1]) << 8)
        );
    }

    inline std::uint32_t readU32LE(const std::vector<std::uint8_t>& data, std::size_t pos) {
        return static_cast<std::uint32_t>(
            static_cast<std::uint32_t>(data[pos]) |
            (static_cast<std::uint32_t>(data[pos + 1]) << 8) |
            (static_cast<std::uint32_t>(data[pos + 2]) << 16) |
            (static_cast<std::uint32_t>(data[pos + 3]) << 24)
        );
    }

    inline std::vector<std::uint8_t> xorBytes(const std::vector<std::uint8_t>& data, const std::string& key) {
        std::vector<std::uint8_t> result(data.size());
        const std::size_t keyLen = key.size();

        for (std::size_t i = 0; i < data.size(); ++i) {
            result[i] = static_cast<std::uint8_t>(data[i] ^ static_cast<std::uint8_t>(key[i % keyLen]));
        }

        return result;
    }

    // Global cache for O(1) file offset lookups
    inline std::unordered_map<std::string, std::pair<std::uint32_t, std::uint32_t>> pakIndexCache;
    inline std::size_t pakBlobStart = 0;
    inline bool indexLoaded = false;

    inline void loadIndex(const std::string& pakPath = "data.pak") {
        if (indexLoaded) return;
        std::ifstream pakStream(pakPath, std::ios::binary);
        if (!pakStream.is_open()) return;

        std::uint32_t headerLength = 0;
        if (!pakStream.read(reinterpret_cast<char*>(&headerLength), 4)) return;

        std::vector<std::uint8_t> headerData(headerLength);
        if (!pakStream.read(reinterpret_cast<char*>(headerData.data()), headerLength)) return;

        pakBlobStart = 4 + headerLength;
        std::size_t cursor = 0;
        if (cursor + 4 > headerData.size()) return;
        
        const std::uint32_t fileCount = readU32LE(headerData, cursor);
        cursor += 4;

        for (std::uint32_t i = 0; i < fileCount; ++i) {
            if (cursor + 2 > headerData.size()) break;
            const std::uint16_t pathLength = readU16LE(headerData, cursor);
            cursor += 2;
            
            if (cursor + pathLength > headerData.size()) break;
            const std::string entryPath(reinterpret_cast<const char*>(headerData.data() + cursor), pathLength);
            cursor += pathLength;

            if (cursor + 8 > headerData.size()) break;
            const std::uint32_t offset = readU32LE(headerData, cursor);
            const std::uint32_t size = readU32LE(headerData, cursor + 4);
            cursor += 8;

            pakIndexCache[normalizePath(entryPath)] = {offset, size};
        }
        indexLoaded = true;
    }

    inline bool readFileFromPak(const std::string& assetPath, std::vector<std::uint8_t>& out, const std::string& pakPath = "data.pak") {
        if (assetPath.empty()) return false;
        
        const std::string normalizedAsset = normalizePath(assetPath);
        if (normalizedAsset.empty()) return false;

        loadIndex(pakPath);

        auto it = pakIndexCache.find(normalizedAsset);
        if (it == pakIndexCache.end()) return false;

        std::ifstream pakStream(pakPath, std::ios::binary);
        if (!pakStream.is_open()) return false;

        const std::uint32_t targetOffset = it->second.first;
        const std::uint32_t targetSize = it->second.second;

        // Jump directly to the payload in the archive without loading the rest of the pack
        pakStream.seekg(pakBlobStart + targetOffset, std::ios::beg);
        
        std::vector<std::uint8_t> encryptedData(targetSize);
        if (!pakStream.read(reinterpret_cast<char*>(encryptedData.data()), targetSize)) return false;

        const std::string key = "GalityEngine2026";
        out = xorBytes(encryptedData, key);
        return true;
    }

    inline bool readTextFile(const std::string& assetPath, std::string& out, const std::string& pakPath = "data.pak") {
        std::vector<std::uint8_t> bytes;
        if (!readFileFromPak(assetPath, bytes, pakPath)) return false;
        
        out.assign(reinterpret_cast<const char*>(bytes.data()), bytes.size());
        return true;
    }

    inline bool existsInPak(const std::string& assetPath, const std::string& pakPath = "data.pak") {
        if (assetPath.empty()) return false;
        loadIndex(pakPath);
        return pakIndexCache.find(normalizePath(assetPath)) != pakIndexCache.end();
    }

    inline bool mountFromPak(const std::string& pakPath, const std::string& targetRoot = "assets") {
        loadIndex(pakPath);
        if (pakIndexCache.empty()) return false;

        std::ifstream pakStream(pakPath, std::ios::binary);
        if (!pakStream.is_open()) return false;

        const std::string key = "GalityEngine2026";
        std::filesystem::create_directories(targetRoot);

        for (const auto& [entryPath, range] : pakIndexCache) {
            const std::uint32_t offset = range.first;
            const std::uint32_t size = range.second;

            pakStream.seekg(pakBlobStart + offset, std::ios::beg);
            std::vector<std::uint8_t> encryptedData(size);
            if (!pakStream.read(reinterpret_cast<char*>(encryptedData.data()), size)) continue;

            const std::vector<std::uint8_t> decryptedData = xorBytes(encryptedData, key);
            const std::filesystem::path outputPath = std::filesystem::path(targetRoot) / entryPath;
            std::filesystem::create_directories(outputPath.parent_path());

            std::ofstream outputFile(outputPath, std::ios::binary | std::ios::trunc);
            if (outputFile.is_open()) {
                outputFile.write(reinterpret_cast<const char*>(decryptedData.data()), static_cast<std::streamsize>(decryptedData.size()));
            }
        }
        return true;
    }
}