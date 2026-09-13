#pragma once
#include <SFML/Audio.hpp>
#include <string>
#include <unordered_map>
#include <iostream>
#include <optional>

class AudioManager {
private:
    // BGM 播放器 (串流模式)
    std::optional<sf::Music> bgm;
    std::string currentBgmPath;

    // CV / SFX 緩衝區快取
    std::unordered_map<std::string, sf::SoundBuffer> soundBuffers;
    std::optional<sf::Sound> voiceSound;

public:
    AudioManager() = default;

    // 播放背景音樂 (支援跨節點連續播放，不中斷)
    void playBGM(const std::string& path) {
        if (path.empty() || path == currentBgmPath) return;

        bgm.emplace(); // 重新建構 Music 實例
        if (!bgm->openFromFile(path)) {
            std::cerr << "[AudioManager] Failed to load BGM: " << path << std::endl;
            bgm.reset();
            return;
        }
        
        bgm->setLooping(true);
        bgm->setVolume(40.f); // BGM 音量設低一點避免蓋過語音
        bgm->play();
        currentBgmPath = path;
    }

    void stopBGM() {
        if (bgm) bgm->stop();
        currentBgmPath = "";
    }

    // 播放角色語音 (單次觸發)
    void playVoice(const std::string& path) {
        if (path.empty()) {
            stopVoice(); // 如果新節點沒有語音，停止上一句未播完的語音
            return;
        }

        auto it = soundBuffers.find(path);
        if (it == soundBuffers.end()) {
            sf::SoundBuffer buffer;
            if (!buffer.loadFromFile(path)) {
                std::cerr << "[AudioManager] Failed to load Voice: " << path << std::endl;
                return;
            }
            soundBuffers[path] = std::move(buffer);
        }

        // 綁定 Buffer 並播放
        voiceSound.emplace(soundBuffers[path]);
        voiceSound->setVolume(100.f);
        voiceSound->play();
    }

    void stopVoice() {
        if (voiceSound) voiceSound->stop();
    }
};