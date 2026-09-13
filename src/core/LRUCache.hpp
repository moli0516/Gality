#pragma once
#include <unordered_map>
#include <list>
#include <memory>
#include <string>
#include <iostream>

template <typename Key, typename Value>
class LRUCache {
private:
    size_t capacity;
    using ListIterator = typename std::list<std::pair<Key, Value>>::iterator;
    std::list<std::pair<Key, Value>> itemsList;
    std::unordered_map<Key, ListIterator> itemsMap;

public:
    explicit LRUCache(size_t cap) : capacity(cap) {}

    bool contains(const Key& key) const {
        return itemsMap.find(key) != itemsMap.end();
    }

    Value get(const Key& key) {
        auto it = itemsMap.find(key);
        if (it == itemsMap.end()) {
            throw std::runtime_error("Key not found in LRU Cache");
        }
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
            std::cout << "[LRU Cache] Evicted asset from memory: " << last.first << std::endl;
        }

        // 新增至前端
        itemsList.push_front({key, value});
        itemsMap[key] = itemsList.begin();
    }

    void clear() {
        itemsMap.clear();
        itemsList.clear();
    }

    size_t size() const {
        return itemsList.size();
    }
};