#pragma once
#include <unordered_map>
#include <list>
#include <memory>
#include <string>
#include <iostream>
#include <functional>
#include <stdexcept>

template <typename Key, typename Value>
class LRUCache {
private:
    size_t capacity;
    using ListIterator = typename std::list<std::pair<Key, Value>>::iterator;
    std::list<std::pair<Key, Value>> itemsList;
    std::unordered_map<Key, ListIterator> itemsMap;

    // ===== 統計 =====
    mutable size_t hitCount = 0;
    mutable size_t missCount = 0;
    mutable size_t evictionCount = 0;

    // ===== 記憶體估算（可選） =====
    std::function<size_t(const Value&)> sizeEstimator;

public:
    explicit LRUCache(size_t cap) : capacity(cap) {}

    // 設定記憶體估算器（用於統計）
    void setSizeEstimator(std::function<size_t(const Value&)> estimator) {
        sizeEstimator = std::move(estimator);
    }

    bool contains(const Key& key) const {
        return itemsMap.find(key) != itemsMap.end();
    }

    Value get(const Key& key) {
        auto it = itemsMap.find(key);
        if (it == itemsMap.end()) {
            missCount++;
            throw std::runtime_error("Key not found in LRU Cache");
        }
        hitCount++;
        // 將被存取的項目移至 list 最前面 (表示最近使用)
        itemsList.splice(itemsList.begin(), itemsList, it->second);
        return it->second->second;
    }

    void put(const Key& key, const Value& value) {
        auto it = itemsMap.find(key);
        if (it != itemsMap.end()) {
            // 已存在，更新數值並搬移到最前
            it->second->second = value;
            itemsList.splice(itemsList.begin(), itemsList, it->second);
            return;
        }

        // 超過容量上限，剔除最久未使用者 (list 末端)
        if (itemsList.size() >= capacity) {
            auto last = itemsList.back();
            itemsMap.erase(last.first);
            itemsList.pop_back();
            evictionCount++;
            std::cout << "[LRU Cache] Evicted: " << last.first << std::endl;
        }

        // 新增至前端
        itemsList.push_front({key, value});
        itemsMap[key] = itemsList.begin();
    }

    void clear() {
        itemsMap.clear();
        itemsList.clear();
        hitCount = 0;
        missCount = 0;
        evictionCount = 0;
    }

    // ===== 基本查詢 =====
    size_t size() const {
        return itemsList.size();
    }

    size_t getCapacity() const {
        return capacity;
    }

    void setCapacity(size_t newCap) {
        capacity = newCap;
        while (itemsList.size() > capacity) {
            auto last = itemsList.back();
            itemsMap.erase(last.first);
            itemsList.pop_back();
            evictionCount++;
        }
    }

    // ===== 統計查詢 =====
    size_t getHitCount() const { return hitCount; }
    size_t getMissCount() const { return missCount; }
    size_t getEvictionCount() const { return evictionCount; }

    size_t getTotalAccesses() const {
        return hitCount + missCount;
    }

    float getHitRate() const {
        size_t total = getTotalAccesses();
        return (total == 0) ? 0.0f : static_cast<float>(hitCount) / static_cast<float>(total);
    }

    void resetStats() {
        hitCount = 0;
        missCount = 0;
        evictionCount = 0;
    }

    // ===== 記憶體估算（若未設定 estimator，回傳 0） =====
    size_t estimateMemoryBytes() const {
        if (!sizeEstimator) return 0;
        size_t total = 0;
        for (const auto& [key, value] : itemsList) {
            total += sizeEstimator(value);
        }
        return total;
    }

    // ===== 遍歷（供統計用） =====
    template <typename Func>
    void forEach(Func&& func) const {
        for (const auto& [key, value] : itemsList) {
            func(key, value);
        }
    }
};