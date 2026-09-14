#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class AnimatedSprite {
private:
    sf::Sprite sprite;
    std::vector<sf::IntRect> frames;
    size_t currentFrame = 0;
    float frameDuration = 0.15f;
    float timer = 0.0f;
    bool looping = true;

public:
    AnimatedSprite(const sf::Texture& texture) : sprite(texture) {}

    void addFrame(const sf::IntRect& rect) {
        frames.push_back(rect);
        if (frames.size() == 1) {
            sprite.setTextureRect(frames[0]);
        }
    }

    void update(float deltaTime) {
        if (frames.empty()) return;

        timer += deltaTime;
        if (timer >= frameDuration) {
            timer -= frameDuration;
            currentFrame = (currentFrame + 1) % frames.size();
            sprite.setTextureRect(frames[currentFrame]);
        }
    }

    void draw(sf::RenderTarget& target) {
        target.draw(sprite);
    }
};