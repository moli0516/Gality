#pragma once
#include <string>
#include <unordered_map>

class Blackboard {
private:
    std::unordered_map<std::string, int> intVars;
    std::unordered_map<std::string, bool> boolVars;

public:
    Blackboard() = default;

    void setInt(const std::string& key, int value) {
        intVars[key] = value;
    }

    int getInt(const std::string& key, int defaultValue = 0) const {
        auto it = intVars.find(key);
        if (it != intVars.end()) {
            return it->second;
        }
        return defaultValue;
    }

    void addInt(const std::string& key, int delta) {
        intVars[key] = getInt(key, 0) + delta;
    }

    void setBool(const std::string& key, bool value) {
        boolVars[key] = value;
    }

    bool getBool(const std::string& key, bool defaultValue = false) const {
        auto it = boolVars.find(key);
        if (it != boolVars.end()) {
            return it->second;
        }
        return defaultValue;
    }

    // 💾 匯出所有整數旗標 (用於序列化 Save)
    const std::unordered_map<std::string, int>& getAllInts() const {
        return intVars;
    }

    // 📂 批量寫入整數旗標 (用於反序列化 Load)
    void setAllInts(const std::unordered_map<std::string, int>& flags) {
        intVars = flags;
    }
};