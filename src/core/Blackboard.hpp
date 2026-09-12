#pragma once
#include <string>
#include <unordered_map>

class Blackboard {
private:
    std::unordered_map<std::string, bool> boolFlags;
    std::unordered_map<std::string, int> intFlags;

public:
    Blackboard() = default;

    void setFlag(const std::string& key, bool value) { boolFlags[key] = value; }
    bool getFlag(const std::string& key, bool defaultValue = false) const {
        auto it = boolFlags.find(key);
        return it != boolFlags.end() ? it->second : defaultValue;
    }

    void setInt(const std::string& key, int value) { intFlags[key] = value; }
    int getInt(const std::string& key, int defaultValue = 0) const {
        auto it = intFlags.find(key);
        return it != intFlags.end() ? it->second : defaultValue;
    }

    void addInt(const std::string& key, int delta) {
        intFlags[key] = getInt(key, 0) + delta;
    }
};