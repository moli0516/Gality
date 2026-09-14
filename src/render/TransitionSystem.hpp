#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <filesystem>
#include "../core/AssetPack.hpp"

class TransitionSystem {
private:
    sf::Shader shader;
    sf::Texture maskTexture;
    std::vector<std::uint8_t> maskBufferBytes;
    sf::RenderTexture oldSceneBuffer;
    
    bool isTransitioning = false;
    float currentProgress = 0.0f;
    float duration = 1.0f;
    float softness = 0.15f;
    bool shaderLoaded = false;

public:
    TransitionSystem(sf::Vector2u windowSize) {
        std::string shaderText;
        if (AssetPack::readTextFile("shaders/transition.frag", shaderText, "data.pak")) {
            if (!shader.loadFromMemory(shaderText, sf::Shader::Type::Fragment)) {
                std::cerr << "[TransitionSystem] Error: Failed to load transition shader from archive!" << std::endl;
            } else {
                shaderLoaded = true;
                std::cout << "[TransitionSystem] Transition shader compiled successfully from archive." << std::endl;
            }
        } else if (shader.loadFromFile("assets/shaders/transition.frag", sf::Shader::Type::Fragment)) {
            shaderLoaded = true;
            std::cout << "[TransitionSystem] Transition shader compiled successfully from disk." << std::endl;
        } else {
            std::cerr << "[TransitionSystem] Error: Failed to load transition shader!" << std::endl;
        }

        if (!oldSceneBuffer.resize(windowSize)) {
            std::cerr << "[TransitionSystem] Failed to allocate oldSceneBuffer!" << std::endl;
        }
    }

    // 觸發轉場：快照當前場景，並載入遮罩
    void start(const sf::RenderTexture& currentScene, const std::string& maskNameOrPath, float transitionDuration = 1.0f, float edgeSoftness = 0.15f) {
        if (!shaderLoaded) return;

        // 1. 快照當前舊畫面
        oldSceneBuffer.clear(sf::Color::Transparent);
        sf::Sprite oldSprite(currentScene.getTexture());
        oldSceneBuffer.draw(oldSprite);
        oldSceneBuffer.display();

        // 2. 正規化遮罩路徑 (支援傳入 "diamond" 或 "assets/masks/diamond.png")
        std::string fullMaskPath = maskNameOrPath;
        if (fullMaskPath.find('/') == std::string::npos && fullMaskPath.find('\\') == std::string::npos) {
            fullMaskPath = "masks/" + maskNameOrPath + ".png";
        }

        // 3. 載入遮罩 (優先自 PAK，回退自硬碟)
        maskBufferBytes.clear();
        bool maskLoaded = false;
        if (AssetPack::readFileFromPak(fullMaskPath, maskBufferBytes, "data.pak") && !maskBufferBytes.empty()) {
            maskLoaded = maskTexture.loadFromMemory(maskBufferBytes.data(), maskBufferBytes.size());
        }
        
        if (!maskLoaded) {
            std::string diskPath = fullMaskPath;
            if (diskPath.rfind("assets/", 0) != 0) {
                diskPath = "assets/" + diskPath;
            }
            maskLoaded = maskTexture.loadFromFile(diskPath);
        }

        if (!maskLoaded) {
            std::cerr << "[TransitionSystem] Failed to load mask: " << fullMaskPath << std::endl;
            return;
        }

        maskTexture.setSmooth(true);

        // 4. 設定 Shader 靜態 Uniforms
        shader.setUniform("u_oldTexture", oldSceneBuffer.getTexture());
        shader.setUniform("u_maskTexture", maskTexture);
        shader.setUniform("u_softness", edgeSoftness);

        duration = (transitionDuration <= 0.0f) ? 0.01f : transitionDuration;
        softness = edgeSoftness;
        currentProgress = 0.0f;
        isTransitioning = true;
    }

    void update(float deltaTime) {
        if (!isTransitioning) return;

        currentProgress += deltaTime / duration;
        if (currentProgress >= 1.0f) {
            currentProgress = 1.0f;
            isTransitioning = false;
        }

        shader.setUniform("u_progress", currentProgress);
    }

    bool isActive() const {
        return isTransitioning;
    }

    void draw(sf::RenderTarget& target, const sf::RenderTexture& newScene) {
        if (!isTransitioning || !shaderLoaded) {
            sf::Sprite spr(newScene.getTexture());
            target.draw(spr);
            return;
        }

        shader.setUniform("u_newTexture", newScene.getTexture());
        sf::Sprite renderSprite(newScene.getTexture());
        target.draw(renderSprite, &shader);
    }
};