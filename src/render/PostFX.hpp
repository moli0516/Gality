#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include <cstdlib>
#include <iostream>

class PostFX {
private:
    sf::Shader blurShader;
    bool shaderLoaded = false;
    float blurRadius = 0.0f;
    float targetBlur = 0.0f;

    // 畫面震動變數
    float shakeTimer = 0.0f;
    float shakeDuration = 0.0f;
    float shakeIntensity = 0.0f;
    sf::Vector2f shakeOffset{0.0f, 0.0f};

public:
    PostFX() {
        if (sf::Shader::isAvailable()) {
            if (blurShader.loadFromFile("assets/shaders/blur.frag", sf::Shader::Type::Fragment)) {
                shaderLoaded = true;
                std::cout << "[PostFX] Blur shader loaded successfully." << std::endl;
            }
        }
    }

    void setBlur(float target) {
        targetBlur = target;
    }

    void triggerShake(float duration = 0.4f, float intensity = 15.0f) {
        shakeDuration = duration;
        shakeTimer = duration;
        shakeIntensity = intensity;
    }

    sf::Vector2f getShakeOffset() const {
        return shakeOffset;
    }

    void update(float deltaTime, sf::Vector2u windowSize) {
        // 平滑漸變 Blur
        blurRadius += (targetBlur - blurRadius) * deltaTime * 10.0f;

        // 更新 Screen Shake Offset
        if (shakeTimer > 0.0f) {
            shakeTimer -= deltaTime;
            float progress = shakeTimer / shakeDuration;
            float currentIntensity = shakeIntensity * progress;

            shakeOffset.x = (-1.0f + static_cast<float>(rand()) / (RAND_MAX / 2.0f)) * currentIntensity;
            shakeOffset.y = (-1.0f + static_cast<float>(rand()) / (RAND_MAX / 2.0f)) * currentIntensity;
        } else {
            shakeOffset = {0.0f, 0.0f};
        }

        if (shaderLoaded) {
            blurShader.setUniform("resolution", sf::Glsl::Vec2(static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)));
            blurShader.setUniform("blurRadius", blurRadius);
        }
    }

    void applyAndDraw(sf::RenderWindow& window, const sf::RenderTexture& sceneBuffer) {
        sf::Sprite sceneSprite(sceneBuffer.getTexture());
        sceneSprite.setPosition(shakeOffset);

        if (shaderLoaded && blurRadius > 0.05f) {
            window.draw(sceneSprite, &blurShader);
        } else {
            window.draw(sceneSprite);
        }
    }
};