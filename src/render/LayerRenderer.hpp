#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <string>
#include <memory>
#include <optional>
#include <iostream>
#include "../story/StoryNode.hpp"
#include "../core/LRUCache.hpp"
#include "../core/AssetPack.hpp"

class LayerRenderer {
private:
    LRUCache<std::string, std::shared_ptr<sf::Texture>>* textureCachePtr = nullptr;
    sf::Texture emptyTexture; 
    std::string currentBgPath;
    
    // sf::Sprite 在 SFML 3.x 沒有預設建構子，必須在建構時初始化
    sf::Sprite bgSprite;

    std::map<CharSlot, sf::Sprite> characterSprites;
    std::map<CharSlot, std::string> currentSlotPaths;

    float getSlotCenterX(CharSlot slot, float windowWidth) {
        switch (slot) {
            case CharSlot::Left:   return windowWidth * 0.25f;
            case CharSlot::Center: return windowWidth * 0.50f;
            case CharSlot::Right:  return windowWidth * 0.75f;
        }
        return windowWidth * 0.5f;
    }

    std::shared_ptr<sf::Texture> getOrLoadTexture(const std::string& path) {
        if (!textureCachePtr) return nullptr;
        
        if (textureCachePtr->contains(path)) {
            return textureCachePtr->get(path);
        } else {
            auto tex = std::make_shared<sf::Texture>();
            std::vector<std::uint8_t> bytes;
            if (AssetPack::readFileFromPak(path, bytes, "data.pak") && !bytes.empty()) {
                if (tex->loadFromMemory(bytes.data(), bytes.size())) {
                    tex->setSmooth(true);
                    textureCachePtr->put(path, tex);
                    return tex;
                }
            } else if (tex->loadFromFile(path)) {
                tex->setSmooth(true);
                textureCachePtr->put(path, tex);
                return tex;
            }

            std::cerr << "[LayerRenderer] Failed to load texture: " << path << std::endl;
            return nullptr;
        }
    }

public:
    // 💡 修正 1：在初始化清單中明確綁定 emptyTexture 給 bgSprite
    LayerRenderer() : emptyTexture(), bgSprite(emptyTexture) {
        (void)emptyTexture.resize(sf::Vector2u(1, 1));
    }

    LayerRenderer(LRUCache<std::string, std::shared_ptr<sf::Texture>>& cache) 
        : textureCachePtr(&cache), emptyTexture(), bgSprite(emptyTexture) {
        (void)emptyTexture.resize(sf::Vector2u(1, 1));
    }

    void setCache(LRUCache<std::string, std::shared_ptr<sf::Texture>>& cache) {
        textureCachePtr = &cache;
    }

    void update(float /*dt*/) {}

    void setBackground(const std::string& path) {
        if (path.empty() || path == currentBgPath) return;
        currentBgPath = path;

        auto texPtr = getOrLoadTexture(path);
        if (texPtr) {
            bgSprite.setTexture(*texPtr, true);
        }
    }

    void setBackground(const std::string& path, sf::Vector2u windowSize) {
        setBackground(path);
        auto bounds = bgSprite.getLocalBounds();
        if (bounds.size.x > 0 && bounds.size.y > 0) {
            bgSprite.setScale(sf::Vector2f(
                static_cast<float>(windowSize.x) / bounds.size.x,
                static_cast<float>(windowSize.y) / bounds.size.y
            ));
        }
    }

    void setCharacter(const std::string& path) {
        std::map<CharSlot, std::string> slots;
        if (!path.empty()) {
            slots[CharSlot::Center] = path;
        }
        updateCharacters(slots, CharSlot::Center);
    }

    void updateCharacters(const std::map<CharSlot, std::string>& newSlots, 
                          std::optional<CharSlot> activeSlot) {
        for (auto it = currentSlotPaths.begin(); it != currentSlotPaths.end(); ) {
            if (newSlots.find(it->first) == newSlots.end() || newSlots.at(it->first).empty()) {
                characterSprites.erase(it->first);
                it = currentSlotPaths.erase(it);
            } else {
                ++it;
            }
        }

        for (const auto& [slot, path] : newSlots) {
            if (path.empty() || path == "none" || path == "clear") {
                characterSprites.erase(slot);
                currentSlotPaths.erase(slot);
                continue;
            }

            if (currentSlotPaths[slot] != path) {
                currentSlotPaths[slot] = path;
                auto texPtr = getOrLoadTexture(path);
                if (texPtr) {
                    sf::Sprite sprite(*texPtr);
                    auto bounds = sprite.getLocalBounds();
                    sprite.setOrigin(sf::Vector2f(bounds.size.x * 0.5f, bounds.size.y));
                    
                    // 💡 修正 2：使用 insert_or_assign 避開 std::map 的預設建構限制
                    characterSprites.insert_or_assign(slot, sprite);
                }
            }
        }

        for (auto& [slot, sprite] : characterSprites) {
            if (!activeSlot.has_value() || activeSlot.value() == slot) {
                sprite.setColor(sf::Color(255, 255, 255, 255));
            } else {
                sprite.setColor(sf::Color(140, 140, 150, 220));
            }
        }
    }

    void draw(sf::RenderTarget& target) {
        if (!currentBgPath.empty()) {
            target.draw(bgSprite);
        }
        float windowWidth = static_cast<float>(target.getSize().x);
        float baselineY = static_cast<float>(target.getSize().y);

        for (auto& [slot, sprite] : characterSprites) {
            float posX = getSlotCenterX(slot, windowWidth);
            
            // --- 💡 修復：加入自動縮放邏輯 (Auto Scaling) ---
            // 讀取原始圖片高度
            float currentHeight = sprite.getLocalBounds().size.y;
            if (currentHeight > 0) {
                // 將目標高度設定為視窗高度的 95% (保留一點頂部空間)
                // 如果覺得太大或太小，可以微調 0.95f 這個係數 (例如 0.85f)
                float targetHeight = baselineY * 0.95f; 
                float scale = targetHeight / currentHeight;
                sprite.setScale(sf::Vector2f(scale, scale));
            }

            sprite.setPosition(sf::Vector2f(posX, baselineY));
            target.draw(sprite);
        }
    }
};