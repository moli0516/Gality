#pragma once
#include <SFML/Audio.hpp>
#include <string>
#include <memory>
#include <optional>
#include <iostream>
#include <vector>
#include "LRUCache.hpp"
#include "Easing.hpp"
#include "ConfigManager.hpp"
#include "AssetPack.hpp"
#include "../story/StoryNode.hpp"

class AudioManager {
private:
    sf::Music activeBgm;
    sf::Music fadingBgm;
    std::unique_ptr<sf::Sound> voiceSound;
    std::string currentBgmPath;

    // 持久保存 BGM 串流二進位緩衝區，杜絕 sf::Music (SoundStream) 非同步解碼時訪問懸空指針
    std::vector<std::uint8_t> activeBgmBytes;
    std::vector<std::uint8_t> fadingBgmBytes;

    LRUCache<std::string, std::shared_ptr<sf::SoundBuffer>> voiceCache;

    bool isCrossfading = false;
    float fadeTimer = 0.0f;
    float fadeDuration = 1.5f;

    // 智慧 Ducking 變數 (當 CV 播放時自動平滑壓低 BGM)
    float duckingFactor = 1.0f; // 1.0f = 正常音量, 0.65f = 壓低 35%

    float getFinalBGMVolume(float baseVolume) const {
        return baseVolume * (ConfigManager::config.bgmVolume / 100.0f) * 
               (ConfigManager::config.masterVolume / 100.0f) * duckingFactor;
    }

    float getFinalVoiceVolume() const {
        return 100.0f * (ConfigManager::config.voiceVolume / 100.0f) * 
               (ConfigManager::config.masterVolume / 100.0f);
    }

public:
    AudioManager() : voiceCache(20) {}

    void update(float deltaTime) {
        // 1. 智慧 Ducking 狀態檢測與平滑插值
        bool isVoicePlaying = (voiceSound && voiceSound->getStatus() == sf::Sound::Status::Playing);
        float targetDucking = isVoicePlaying ? 0.65f : 1.0f;
        duckingFactor += (targetDucking - duckingFactor) * deltaTime * 6.0f;

        // 2. BGM 跨淡入淡出 (Crossfade) 與音量同步
        if (isCrossfading) {
            fadeTimer += deltaTime;
            float progress = fadeTimer / fadeDuration;

            if (progress >= 1.0f) {
                progress = 1.0f;
                isCrossfading = false;
                fadingBgm.stop();
                fadingBgmBytes.clear();
            }

            float activeVolume = Easing::easeInQuad(progress) * 100.0f;
            float fadingVolume = (1.0f - Easing::easeOutQuad(progress)) * 100.0f;

            activeBgm.setVolume(getFinalBGMVolume(activeVolume));
            fadingBgm.setVolume(getFinalBGMVolume(fadingVolume));
        } else if (activeBgm.getStatus() == sf::SoundStream::Status::Playing) {
            activeBgm.setVolume(getFinalBGMVolume(100.0f));
        }

        if (isVoicePlaying) {
            voiceSound->setVolume(getFinalVoiceVolume());
        }
    }

    void playBGM(const std::string& audioPath, float fadeTime = 1.5f) {
        if (audioPath.empty() || audioPath == currentBgmPath) return;

        currentBgmPath = audioPath;
        fadeDuration = (fadeTime <= 0.0f) ? 0.01f : fadeTime;
        fadeTimer = 0.0f;

        // 如果目前已有 BGM 正在播放，且要求漸層，則啟動真實的雙軌交換 Crossfade
        if (activeBgm.getStatus() == sf::SoundStream::Status::Playing && fadeTime > 0.0f) {
            fadingBgm.stop();
            fadingBgmBytes = std::move(activeBgmBytes);

            // 重新在 fadingBgm 上掛載當前即將淡出的音軌
            if (!fadingBgmBytes.empty()) {
                if (fadingBgm.openFromMemory(fadingBgmBytes.data(), fadingBgmBytes.size())) {
                    fadingBgm.setLooping(true);
                    fadingBgm.play();
                }
            }

            // 載入新的 BGM 至 activeBgm
            activeBgmBytes.clear();
            bool fromPak = AssetPack::readFileFromPak(audioPath, activeBgmBytes, "data.pak");
            bool loaded = false;

            if (fromPak && !activeBgmBytes.empty()) {
                loaded = activeBgm.openFromMemory(activeBgmBytes.data(), activeBgmBytes.size());
            } else {
                loaded = activeBgm.openFromFile(audioPath);
            }

            if (loaded) {
                activeBgm.setLooping(true);
                activeBgm.setVolume(0.0f);
                activeBgm.play();
                isCrossfading = true;
            } else {
                std::cerr << "[AudioManager Error] Failed to open BGM: " << audioPath << std::endl;
                isCrossfading = false;
            }
        } else {
            // 直接播放（無漸變或當前無音軌）
            fadingBgm.stop();
            fadingBgmBytes.clear();
            isCrossfading = false;

            activeBgm.stop();
            activeBgmBytes.clear();

            bool fromPak = AssetPack::readFileFromPak(audioPath, activeBgmBytes, "data.pak");
            bool loaded = false;

            if (fromPak && !activeBgmBytes.empty()) {
                loaded = activeBgm.openFromMemory(activeBgmBytes.data(), activeBgmBytes.size());
            } else {
                loaded = activeBgm.openFromFile(audioPath);
            }

            if (loaded) {
                activeBgm.setLooping(true);
                activeBgm.setVolume(getFinalBGMVolume(100.0f));
                activeBgm.play();
            } else {
                std::cerr << "[AudioManager Error] Failed to open BGM: " << audioPath << std::endl;
            }
        }
    }

    void stopBGM() {
        activeBgm.stop();
        fadingBgm.stop();
        activeBgmBytes.clear();
        fadingBgmBytes.clear();
        currentBgmPath.clear();
        isCrossfading = false;
    }

    // 支援 CharSlot 空間立體聲定位 (Spatial Panning)
    void playVoice(const std::string& audioPath, std::optional<CharSlot> slot = std::nullopt) {
        if (audioPath.empty()) return;
        if (voiceSound) voiceSound->stop();

        std::shared_ptr<sf::SoundBuffer> buffer;
        if (voiceCache.contains(audioPath)) {
            buffer = voiceCache.get(audioPath);
        } else {
            buffer = std::make_shared<sf::SoundBuffer>();
            std::vector<std::uint8_t> bytes;
            if (AssetPack::readFileFromPak(audioPath, bytes, "data.pak") && !bytes.empty()) {
                if (buffer->loadFromMemory(bytes.data(), bytes.size())) {
                    voiceCache.put(audioPath, buffer);
                } else {
                    std::cerr << "[AudioManager] Failed to load Voice from archive: " << audioPath << std::endl;
                    return;
                }
            } else if (buffer->loadFromFile(audioPath)) {
                voiceCache.put(audioPath, buffer);
            } else {
                std::cerr << "[AudioManager] Failed to load Voice: " << audioPath << std::endl;
                return;
            }
        }

        voiceSound = std::make_unique<sf::Sound>(*buffer);
        voiceSound->setVolume(getFinalVoiceVolume());

        // 計算 3D 立體聲位置 (X 軸：左 -3.0, 中 0.0, 右 +3.0)
        float posX = 0.0f;
        if (slot.has_value()) {
            switch (slot.value()) {
                case CharSlot::Left:   posX = -1.0f; break;
                case CharSlot::Center: posX =  0.0f; break;
                case CharSlot::Right:  posX =  1.0f; break;
            }
        }

        voiceSound->setPosition(sf::Vector3f(posX, 0.0f, 0.0f));
        voiceSound->setRelativeToListener(true);
        voiceSound->play();
    }

    void stopVoice() {
        if (voiceSound) voiceSound->stop();
    }
};