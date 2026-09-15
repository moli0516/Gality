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

    // 持久保存 BGM 串流二進位緩衝區
    std::vector<std::uint8_t> activeBgmBytes;
    std::vector<std::uint8_t> fadingBgmBytes;

    LRUCache<std::string, std::shared_ptr<sf::SoundBuffer>> voiceCache;

    bool isCrossfading = false;
    float fadeTimer = 0.0f;
    float fadeDuration = 1.5f;

    // 智慧 Ducking
    float duckingFactor = 1.0f;

    // 音量倍率（由 playBGM 設定）
    float currentBgmVolumeMultiplier = 1.0f;
    bool  currentBgmLoop = true;

    float getFinalBGMVolume(float baseVolume) const {
        return baseVolume * currentBgmVolumeMultiplier *
               (ConfigManager::config.bgmVolume / 100.0f) *
               (ConfigManager::config.masterVolume / 100.0f) * duckingFactor;
    }

    float getFinalVoiceVolume() const {
        return 100.0f * (ConfigManager::config.voiceVolume / 100.0f) *
               (ConfigManager::config.masterVolume / 100.0f);
    }

public:
    AudioManager() : voiceCache(20) {}

    void update(float deltaTime) {
        // 智慧 Ducking
        bool isVoicePlaying = (voiceSound && voiceSound->getStatus() == sf::Sound::Status::Playing);
        float targetDucking = isVoicePlaying ? 0.65f : 1.0f;
        duckingFactor += (targetDucking - duckingFactor) * deltaTime * 6.0f;

        // Crossfade
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

    // ============================================================================
    // 播放 BGM（支援音量倍率、循環、淡入時間）
    // ============================================================================
    void playBGM(const std::string& audioPath,
                 float volumeMultiplier = 1.0f,
                 bool loop = true,
                 float fadeTime = 1.5f)
    {
        if (audioPath.empty()) return;

        // 同一路徑、同設定 → 不重啟
        if (audioPath == currentBgmPath &&
            volumeMultiplier == currentBgmVolumeMultiplier &&
            loop == currentBgmLoop) {
            return;
        }

        currentBgmPath = audioPath;
        currentBgmVolumeMultiplier = volumeMultiplier;
        currentBgmLoop = loop;
        fadeDuration = (fadeTime <= 0.0f) ? 0.01f : fadeTime;
        fadeTimer = 0.0f;

        // 若目前已有 BGM 播放，啟動 Crossfade
        bool hasActiveBgm = (activeBgm.getStatus() == sf::SoundStream::Status::Playing);

        if (hasActiveBgm && fadeTime > 0.0f) {
            fadingBgm.stop();
            fadingBgmBytes = std::move(activeBgmBytes);

            if (!fadingBgmBytes.empty()) {
                if (fadingBgm.openFromMemory(fadingBgmBytes.data(), fadingBgmBytes.size())) {
                    fadingBgm.setLooping(true);
                    fadingBgm.play();
                }
            }

            activeBgmBytes.clear();
            bool fromPak = AssetPack::readFileFromPak(audioPath, activeBgmBytes, "data.pak");
            bool loaded = false;

            if (fromPak && !activeBgmBytes.empty()) {
                loaded = activeBgm.openFromMemory(activeBgmBytes.data(), activeBgmBytes.size());
            } else {
                loaded = activeBgm.openFromFile(audioPath);
            }

            if (loaded) {
                activeBgm.setLooping(loop);
                activeBgm.setVolume(0.0f);
                activeBgm.play();
                isCrossfading = true;
            } else {
                std::cerr << "[AudioManager] Failed to open BGM: " << audioPath << std::endl;
                isCrossfading = false;
            }
        } else {
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
                activeBgm.setLooping(loop);
                if (fadeTime > 0.0f) {
                    activeBgm.setVolume(0.0f);
                    activeBgm.play();
                    isCrossfading = true;
                } else {
                    activeBgm.setVolume(getFinalBGMVolume(100.0f));
                    activeBgm.play();
                }
            } else {
                std::cerr << "[AudioManager] Failed to open BGM: " << audioPath << std::endl;
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

    // 播放語音
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
                    std::cerr << "[AudioManager] Failed to load Voice: " << audioPath << std::endl;
                    return;
                }
            } else if (buffer->loadFromFile(audioPath)) {
                voiceCache.put(audioPath, buffer);
            } else {
                return;
            }
        }

        voiceSound = std::make_unique<sf::Sound>(*buffer);
        voiceSound->setVolume(getFinalVoiceVolume());

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

    bool isBGMPlaying() const {
        return activeBgm.getStatus() == sf::SoundStream::Status::Playing;
    }

    const std::string& getCurrentBgmPath() const {
        return currentBgmPath;
    }
};