#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <memory>
#include <iostream>
#include "../core/LRUCache.hpp"
#include "../core/Easing.hpp"

class LayerRenderer {
private:
    LRUCache<std::string, std::shared_ptr<sf::Texture>> textureCache;
    
    std::string currentBgPath;
    std::string currentCharacterPath;

    // 💡 SFML 3.x：使用 unique_ptr 避開 sf::Sprite 缺乏預設建構函式的限制
    std::unique_ptr<sf::Sprite> bgSpriteCurrent;
    std::unique_ptr<sf::Sprite> bgSpriteOld;
    std::unique_ptr<sf::Sprite> characterSpriteCurrent;
    std::unique_ptr<sf::Sprite> characterSpriteOld;

    float bgFadeTimer = 0.0f;
    float charFadeTimer = 0.0f;
    float fadeDuration = 0.5f; // 0.5秒漸變過渡

    bool isBgFading = false;
    bool isCharFading = false;

    std::shared_ptr<sf::Texture> getOrLoadTexture(const std::string& path) {
        if (path.empty()) return nullptr;

        if (textureCache.contains(path)) {
            return textureCache.get(path);
        }

        auto texture = std::make_shared<sf::Texture>();
        if (!texture->loadFromFile(path)) {
            std::cerr << "[LayerRenderer] Failed to load texture: " << path << std::endl;
            return nullptr;
        }

        textureCache.put(path, texture);
        return texture;
    }

public:
    LayerRenderer() : textureCache(10) {}

    void setBackground(const std::string& imagePath) {
        if (imagePath == currentBgPath) return;

        bgSpriteOld = std::move(bgSpriteCurrent);
        currentBgPath = imagePath;

        if (!imagePath.empty()) {
            auto tex = getOrLoadTexture(imagePath);
            if (tex) {
                // SFML 3.x 建構 sf::Sprite 時傳入 sf::Texture 引用
                bgSpriteCurrent = std::make_unique<sf::Sprite>(*tex);
                isBgFading = true;
                bgFadeTimer = 0.0f;
            }
        } else {
            bgSpriteCurrent.reset();
        }
    }

    void setCharacter(const std::string& imagePath) {
        if (imagePath == currentCharacterPath) return;

        characterSpriteOld = std::move(characterSpriteCurrent);
        currentCharacterPath = imagePath;

        if (!imagePath.empty()) {
            auto tex = getOrLoadTexture(imagePath);
            if (tex) {
                // SFML 3.x 建構 sf::Sprite 時傳入 sf::Texture 引用
                characterSpriteCurrent = std::make_unique<sf::Sprite>(*tex);
                isCharFading = true;
                charFadeTimer = 0.0f;
            }
        } else {
            characterSpriteCurrent.reset();
            isCharFading = true;
            charFadeTimer = 0.0f;
        }
    }

    void update(float deltaTime) {
        // 背景漸變更新
        if (isBgFading) {
            bgFadeTimer += deltaTime;
            float progress = bgFadeTimer / fadeDuration;
            if (progress >= 1.0f) {
                progress = 1.0f;
                isBgFading = false;
                bgSpriteOld.reset(); // 結束過渡，釋放舊 Sprite
            }
            float alpha = Easing::easeOutQuad(progress) * 255.0f;
            if (bgSpriteCurrent) {
                bgSpriteCurrent->setColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(alpha)));
            }
            if (bgSpriteOld) {
                bgSpriteOld->setColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(255.0f - alpha)));
            }
        }

        // 立繪漸變更新
        if (isCharFading) {
            charFadeTimer += deltaTime;
            float progress = charFadeTimer / fadeDuration;
            if (progress >= 1.0f) {
                progress = 1.0f;
                isCharFading = false;
                characterSpriteOld.reset(); // 結束過渡，釋放舊 Sprite
            }
            float alpha = Easing::easeOutQuad(progress) * 255.0f;
            if (characterSpriteCurrent) {
                characterSpriteCurrent->setColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(alpha)));
            }
            if (characterSpriteOld) {
                characterSpriteOld->setColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(255.0f - alpha)));
            }
        }
    }

    void draw(sf::RenderWindow& window) {
        if (isBgFading && bgSpriteOld) {
            window.draw(*bgSpriteOld);
        }
        if (bgSpriteCurrent) {
            window.draw(*bgSpriteCurrent);
        }

        if (isCharFading && characterSpriteOld) {
            window.draw(*characterSpriteOld);
        }
        if (characterSpriteCurrent) {
            window.draw(*characterSpriteCurrent);
        }
    }
};