#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>
#include <iostream>
#include <optional> // 💡 引入 optional 用於延遲初始化 Sprite

class LayerRenderer {
private:
    std::unordered_map<std::string, sf::Texture> textureCache;
    
    // 💡 SFML 3.x：使用 std::optional 延遲 Sprite 的初始化
    std::optional<sf::Sprite> bgSprite;
    std::optional<sf::Sprite> characterSprite;

    const sf::Texture* getOrLoadTexture(const std::string& path) {
        if (path.empty()) return nullptr;
        auto it = textureCache.find(path);
        if (it != textureCache.end()) return &it->second;

        sf::Texture texture;
        if (!texture.loadFromFile(path)) {
            std::cerr << "[LayerRenderer] Failed to load texture: " << path << std::endl;
            return nullptr;
        }

        textureCache[path] = std::move(texture);
        return &textureCache[path];
    }

public:
    LayerRenderer() = default;

    void setBackground(const std::string& path) {
        if (path.empty()) {
            bgSprite.reset(); // 清空圖層
            return;
        }
        
        const sf::Texture* tex = getOrLoadTexture(path);
        if (tex) {
            // 💡 使用 emplace 直接在 optional 內部建構 sf::Sprite(*tex)
            bgSprite.emplace(*tex);
            bgSprite->setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(tex->getSize().x, tex->getSize().y)));
            
            sf::Vector2u size = tex->getSize();
            float scaleX = 1280.f / static_cast<float>(size.x);
            float scaleY = 720.f / static_cast<float>(size.y);
            bgSprite->setScale(sf::Vector2f(scaleX, scaleY));
        }
    }

    void setCharacter(const std::string& path) {
        if (path.empty()) {
            characterSprite.reset(); // 清空圖層
            return;
        }
        
        const sf::Texture* tex = getOrLoadTexture(path);
        if (tex) {
            // 💡 使用 emplace 綁定紋理
            characterSprite.emplace(*tex);
            characterSprite->setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(tex->getSize().x, tex->getSize().y)));
            
            sf::Vector2u size = tex->getSize();
            float targetHeight = 600.f;
            float scale = targetHeight / static_cast<float>(size.y);
            characterSprite->setScale(sf::Vector2f(scale, scale));

            float scaledWidth = static_cast<float>(size.x) * scale;
            float posX = (1280.f - scaledWidth) / 2.f;
            float posY = 720.f - targetHeight;
            characterSprite->setPosition(sf::Vector2f(posX, posY));
        }
    }

    void draw(sf::RenderWindow& window) {
        // 💡 透過 optional 自動檢查是否有值 (取代舊版的 hasBg)
        if (bgSprite) window.draw(*bgSprite);
        if (characterSprite) window.draw(*characterSprite);
    }
};