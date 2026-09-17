#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <algorithm>
#include <iostream>

class NineSliceSprite {
private:
    std::shared_ptr<sf::Texture> texture;

    // 原始圖片尺寸
    sf::Vector2f textureSize;

    // 九宮格邊界（像素）
    float leftBorder = 0.0f;
    float rightBorder = 0.0f;
    float topBorder = 0.0f;
    float bottomBorder = 0.0f;

    // 九個精靈（使用 std::vector 避免 std::array 的預設建構需求）
    std::vector<sf::Sprite> sprites;

    // 目標尺寸
    sf::Vector2f targetSize{0.0f, 0.0f};

    // 目標位置
    sf::Vector2f targetPosition{0.0f, 0.0f};

    bool initialized = false;

    // ------------------------------------------------------------------------
    // 取得一個全域共用的 1x1 空紋理
    // 用於在尚未呼叫 setTexture 前，合法建構 sf::Sprite
    // ------------------------------------------------------------------------
    static const sf::Texture& getEmptyTexture() {
        static sf::Texture emptyTex;
        static bool texReady = false;
        if (!texReady) {
            (void)emptyTex.resize(sf::Vector2u(1, 1));
            sf::Image img(sf::Vector2u(1, 1), sf::Color::Transparent);
            (void)emptyTex.update(img);
            texReady = true;
        }
        return emptyTex;
    }

    // ------------------------------------------------------------------------
    // 重新計算九個 Sprite 的紋理矩形、縮放與位置
    // ------------------------------------------------------------------------
    void updateSprites() {
        if (!texture || !initialized) return;

        float tw = textureSize.x;
        float th = textureSize.y;
        float tw2 = targetSize.x;
        float th2 = targetSize.y;

        // 檢查目標尺寸是否小於邊界總和
        if (tw2 < leftBorder + rightBorder) tw2 = leftBorder + rightBorder;
        if (th2 < topBorder + bottomBorder) th2 = topBorder + bottomBorder;

        // 九宮格的紋理矩形
        sf::IntRect rects[9] = {
            // 左上、中上、右上
            sf::IntRect({0, 0},
                        {static_cast<int>(leftBorder), static_cast<int>(topBorder)}),
            sf::IntRect({static_cast<int>(leftBorder), 0},
                        {static_cast<int>(tw - leftBorder - rightBorder),
                         static_cast<int>(topBorder)}),
            sf::IntRect({static_cast<int>(tw - rightBorder), 0},
                        {static_cast<int>(rightBorder), static_cast<int>(topBorder)}),

            // 左中、中中、右中
            sf::IntRect({0, static_cast<int>(topBorder)},
                        {static_cast<int>(leftBorder),
                         static_cast<int>(th - topBorder - bottomBorder)}),
            sf::IntRect({static_cast<int>(leftBorder), static_cast<int>(topBorder)},
                        {static_cast<int>(tw - leftBorder - rightBorder),
                         static_cast<int>(th - topBorder - bottomBorder)}),
            sf::IntRect({static_cast<int>(tw - rightBorder), static_cast<int>(topBorder)},
                        {static_cast<int>(rightBorder),
                         static_cast<int>(th - topBorder - bottomBorder)}),

            // 左下、中下、右下
            sf::IntRect({0, static_cast<int>(th - bottomBorder)},
                        {static_cast<int>(leftBorder), static_cast<int>(bottomBorder)}),
            sf::IntRect({static_cast<int>(leftBorder), static_cast<int>(th - bottomBorder)},
                        {static_cast<int>(tw - leftBorder - rightBorder),
                         static_cast<int>(bottomBorder)}),
            sf::IntRect({static_cast<int>(tw - rightBorder),
                         static_cast<int>(th - bottomBorder)},
                        {static_cast<int>(rightBorder), static_cast<int>(bottomBorder)})
        };

        // 目標區域的位置與尺寸
        struct Region {
            sf::Vector2f pos;
            sf::Vector2f size;
        };

        Region regions[9] = {
            // 左上、中上、右上
            {{0.0f, 0.0f}, {leftBorder, topBorder}},
            {{leftBorder, 0.0f}, {tw2 - leftBorder - rightBorder, topBorder}},
            {{tw2 - rightBorder, 0.0f}, {rightBorder, topBorder}},

            // 左中、中中、右中
            {{0.0f, topBorder}, {leftBorder, th2 - topBorder - bottomBorder}},
            {{leftBorder, topBorder}, {tw2 - leftBorder - rightBorder, th2 - topBorder - bottomBorder}},
            {{tw2 - rightBorder, topBorder}, {rightBorder, th2 - topBorder - bottomBorder}},

            // 左下、中下、右下
            {{0.0f, th2 - bottomBorder}, {leftBorder, bottomBorder}},
            {{leftBorder, th2 - bottomBorder}, {tw2 - leftBorder - rightBorder, bottomBorder}},
            {{tw2 - rightBorder, th2 - bottomBorder}, {rightBorder, bottomBorder}}
        };

        for (int i = 0; i < 9; ++i) {
            // 重新綁定紋理
            sprites[i].setTexture(*texture, true);
            sprites[i].setTextureRect(rects[i]);

            float sx = (rects[i].size.x > 0)
                ? regions[i].size.x / static_cast<float>(rects[i].size.x)
                : 1.0f;
            float sy = (rects[i].size.y > 0)
                ? regions[i].size.y / static_cast<float>(rects[i].size.y)
                : 1.0f;

            sprites[i].setScale(sf::Vector2f(sx, sy));
            sprites[i].setPosition(sf::Vector2f(
                targetPosition.x + regions[i].pos.x,
                targetPosition.y + regions[i].pos.y
            ));
        }
    }

public:
    // ------------------------------------------------------------------------
    // 預設建構：使用 std::vector 初始化 9 個 Sprite（綁定空紋理）
    // ------------------------------------------------------------------------
    NineSliceSprite() {
        const sf::Texture& emptyTex = getEmptyTexture();
        sprites.reserve(9);
        for (int i = 0; i < 9; ++i) {
            sprites.emplace_back(emptyTex);
        }
    }

    // ------------------------------------------------------------------------
    // 設定紋理與九宮格邊界
    // ------------------------------------------------------------------------
    void setTexture(std::shared_ptr<sf::Texture> tex,
                    float left, float right, float top, float bottom) {
        texture = tex;
        leftBorder = left;
        rightBorder = right;
        topBorder = top;
        bottomBorder = bottom;

        if (texture) {
            auto size = texture->getSize();
            textureSize = sf::Vector2f(
                static_cast<float>(size.x),
                static_cast<float>(size.y)
            );
        }

        initialized = true;
        updateSprites();
    }

    // 設定目標尺寸
    void setSize(sf::Vector2f size) {
        targetSize = size;
        updateSprites();
    }

    void setSize(float width, float height) {
        setSize(sf::Vector2f(width, height));
    }

    // 設定位置
    void setPosition(sf::Vector2f pos) {
        targetPosition = pos;
        updateSprites();
    }

    void setPosition(float x, float y) {
        setPosition(sf::Vector2f(x, y));
    }

    // 設定顏色（整體染色）
    void setColor(sf::Color color) {
        for (auto& sprite : sprites) {
            sprite.setColor(color);
        }
    }

    // 設定透明度（只改 alpha）
    void setAlpha(std::uint8_t alpha) {
        for (auto& sprite : sprites) {
            sf::Color c = sprite.getColor();
            c.a = alpha;
            sprite.setColor(c);
        }
    }

    // 繪製
    void draw(sf::RenderTarget& target) const {
        if (!initialized) return;
        for (const auto& sprite : sprites) {
            target.draw(sprite);
        }
    }

    // 取得邊界（用於碰撞偵測）
    sf::FloatRect getGlobalBounds() const {
        return sf::FloatRect(targetPosition, targetSize);
    }

    // 是否已初始化
    bool isInitialized() const { return initialized; }
};