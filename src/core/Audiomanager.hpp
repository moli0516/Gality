#pragma once
#include <SFML/Audio.hpp>
#include <string>
#include <memory>
#include <iostream>
#include "LRUCache.hpp"
#include "Easing.hpp"

class AudioManager {
private:
    sf::Music activeBgm;
    sf::Music fadingBgm;
    std::unique_ptr<sf::Sound> voiceSound;
    std::string currentBgmPath;

    LRUCache<std::string, std::shared_ptr<sf::SoundBuffer>> voiceCache;

    // BGM 淡入淡出變數
    bool isCrossfading = false;
    float fadeTimer = 0.0f;
    float fadeDuration = 1.5f; // 漸變時間 1.5 秒

public:
    AudioManager() : voiceCache(20) {}

    void update(float deltaTime) {
        if (isCrossfading) {
            fadeTimer += deltaTime;
            float progress = fadeTimer / fadeDuration;

            if (progress >= 1.0f) {
                progress = 1.0f;
                isCrossfading = false;
                fadingBgm.stop();
            }

            float activeVolume = Easing::easeInQuad(progress) * 100.0f;
            float fadingVolume = (1.0f - Easing::easeOutQuad(progress)) * 100.0f;

            activeBgm.setVolume(activeVolume);
            fadingBgm.setVolume(fadingVolume);
        }
    }

    void playBGM(const std::string& audioPath, float fadeTime = 1.5f) {
        if (audioPath.empty() || audioPath == currentBgmPath) return;

        currentBgmPath = audioPath;
        fadeDuration = fadeTime;
        fadeTimer = 0.0f;

        // 將當前播放的音樂轉入舊音樂軌道準備淡出
        if (activeBgm.getStatus() == sf::SoundStream::Status::Playing) {
            fadingBgm.stop();
            // 切換音樂
            if (activeBgm.openFromFile(audioPath)) {
                activeBgm.setLooping(true);
                activeBgm.setVolume(0.0f);
                activeBgm.play();
                isCrossfading = true;
            }
        } else {
            if (activeBgm.openFromFile(audioPath)) {
                activeBgm.setLooping(true);
                activeBgm.setVolume(100.0f);
                activeBgm.play();
            }
        }
    }

    void stopBGM() {
        activeBgm.stop();
        fadingBgm.stop();
        currentBgmPath.clear();
        isCrossfading = false;
    }

    void playVoice(const std::string& audioPath) {
        if (audioPath.empty()) return;

        if (voiceSound) {
            voiceSound->stop();
        }

        std::shared_ptr<sf::SoundBuffer> buffer;
        if (voiceCache.contains(audioPath)) {
            buffer = voiceCache.get(audioPath);
        } else {
            buffer = std::make_shared<sf::SoundBuffer>();
            if (buffer->loadFromFile(audioPath)) {
                voiceCache.put(audioPath, buffer);
            } else {
                std::cerr << "[AudioManager] Failed to load Voice: " << audioPath << std::endl;
                return;
            }
        }

        voiceSound = std::make_unique<sf::Sound>(*buffer);
        voiceSound->play();
    }

    void stopVoice() {
        if (voiceSound) {
            voiceSound->stop();
        }
    }
};