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

    sf::Sprite bgSprite;

    std::map<CharSlot, sf::Sprite> characterSprites;
    std::map<CharSlot, std::string> currentSlotPaths;

    // ===== 對話框佔用的高度（僅用於參考，不再限制立繪大小） =====
    float dialogueBoxReservedHeight = 280.0f;

    // ===== 縮放係數（以畫面高度為基準） =====
    float scaleSingle = 0.85f;
    float scaleDouble = 0.78f;
    float scaleTriple = 0.70f;
    float characterBottomOffset = 0.0f;

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

    float getScaleFactor() const {
        size_t count = characterSprites.size();
        if (count >= 3) return scaleTriple;
        if (count == 2) return scaleDouble;
        if (count == 1) return scaleSingle;
        return scaleSingle;
    }

public:
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

    // ===== 設定對話框保留高度（保留 API，但僅供參考） =====
    void setDialogueBoxReservedHeight(float height) {
        dialogueBoxReservedHeight = height;
    }

    float getDialogueBoxReservedHeight() const {
        return dialogueBoxReservedHeight;
    }

    // ===== 設定縮放係數 =====
    void setScaleFactors(float single, float doubleSlot, float triple, float bottomOffset = 0.0f) {
        scaleSingle = single;
        scaleDouble = doubleSlot;
        scaleTriple = triple;
        characterBottomOffset = bottomOffset;
    }

    float getScaleSingle() const { return scaleSingle; }
    float getScaleDouble() const { return scaleDouble; }
    float getScaleTriple() const { return scaleTriple; }
    float getCharacterBottomOffset() const { return characterBottomOffset; }

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
        // 移除不再使用的槽位
        for (auto it = currentSlotPaths.begin(); it != currentSlotPaths.end(); ) {
            if (newSlots.find(it->first) == newSlots.end() || newSlots.at(it->first).empty()) {
                characterSprites.erase(it->first);
                it = currentSlotPaths.erase(it);
            } else {
                ++it;
            }
        }

        // 新增或更新槽位
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
                    // 原點設在底部中心
                    sprite.setOrigin(sf::Vector2f(bounds.size.x * 0.5f, bounds.size.y));
                    characterSprites.insert_or_assign(slot, sprite);
                }
            }
        }

        // 更新 activeSlot 的暗調
        for (auto& [slot, sprite] : characterSprites) {
            if (!activeSlot.has_value() || activeSlot.value() == slot) {
                sprite.setColor(sf::Color(255, 255, 255, 255));
            } else {
                sprite.setColor(sf::Color(140, 140, 150, 220));
            }
        }
    }

    // ===== 主繪製 =====
    void draw(sf::RenderTarget& target) {
        // ===== 背景 =====
        if (!currentBgPath.empty()) {
            target.draw(bgSprite);
        }

        float windowWidth = static_cast<float>(target.getSize().x);
        float windowHeight = static_cast<float>(target.getSize().y);

        // ⚠️ 以「畫面高度」為基準（非對話框上方空間）
        float scaleFactor = getScaleFactor();

        // ===== 角色立繪 =====
        for (auto& [slot, sprite] : characterSprites) {
            float posX = getSlotCenterX(slot, windowWidth);

            float currentHeight = sprite.getLocalBounds().size.y;
            if (currentHeight > 0) {
                // 目標高度 = 畫面高度 × 縮放係數
                float targetHeight = windowHeight * scaleFactor;
                float scale = targetHeight / currentHeight;
                sprite.setScale(sf::Vector2f(scale, scale));
            }

            // 底部對齊畫面底部（讓角色進入對話框）
            sprite.setPosition(sf::Vector2f(posX, windowHeight + characterBottomOffset));
            target.draw(sprite);
        }
    }
};