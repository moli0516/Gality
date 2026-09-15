#pragma once
#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <memory>
#include <cmath>
#include <fstream>
#include "../core/AssetPack.hpp"
#include "../render/WeatherSystem.hpp"

using json = nlohmann::json;

// ============================================================================
// BGM 設定
// ============================================================================
struct TitleBGMConfig {
    std::string path = "";
    float volume = 1.0f;
    float fadeIn = 1.5f;
    bool loop = true;
    bool stopOnStart = true;

    bool isEnabled() const { return !path.empty(); }
};

// ============================================================================
// 按鈕 / 文字 設定
// ============================================================================
struct MenuButtonConfig {
    std::string action;
    std::string text;
    float posX = -1.0f;
    float posY = 0.0f;
    float width = 400.0f;
    float height = 65.0f;
    unsigned int fontSize = 28;

    sf::Color baseColor{20, 20, 40, 200};
    sf::Color hoverColor{60, 60, 120, 240};
    sf::Color outlineColor{100, 100, 200, 255};
    sf::Color hoverOutlineColor{255, 215, 0, 255};
    sf::Color textColor{255, 255, 255, 255};
};

struct MenuTextConfig {
    std::string text;
    float posX = -1.0f;
    float posY = 0.0f;
    unsigned int fontSize = 24;
    sf::Color color{255, 255, 255, 255};
};

struct KenBurnsConfig {
    bool enabled = true;
    float zoomRange = 0.05f;
    float panRange = 30.0f;
    float cycleDuration = 20.0f;
};

struct CodeRainConfig {
    bool enabled = true;
    float intensity = 0.15f;
    sf::Color color{80, 255, 120, 255};
    std::string shaderPath = "assets/shaders/code_rain.frag";
};

// ============================================================================
// TitleMenu
// ============================================================================
class TitleMenu {
private:
    sf::Font font;
    bool hasFont = false;
    std::vector<std::uint8_t> fontBytes;

    // ===== 資料驅動設定 =====
    std::string backgroundPath;
    int darkenAmount = 100;
    KenBurnsConfig kenBurnsConfig;
    CodeRainConfig codeRainConfig;
    bool sakuraEnabled = true;
    int sakuraParticleCount = 60;

    MenuTextConfig titleConfig;
    MenuTextConfig subtitleConfig;
    MenuTextConfig versionConfig;
    std::vector<MenuButtonConfig> buttonConfigs;

    // ===== BGM 設定 =====
    TitleBGMConfig bgmConfig;
    std::string currentBgmPath = "";
    std::function<void(const std::string&, float, bool, float)> onRequestBGM;
    std::function<void()> onStopBGM;

    // ===== 執行時物件 =====
    sf::Text titleText;
    sf::Text subtitleText;
    sf::Text versionText;

    std::shared_ptr<sf::Texture> backgroundTexture;
    std::unique_ptr<sf::Sprite> backgroundSprite;

    sf::Shader codeRainShader;
    bool codeRainLoaded = false;
    sf::RenderTexture codeRainBuffer;

    std::unique_ptr<WeatherSystem> sakuraWeather;

    float kenBurnsTime = 0.0f;

    struct MenuButton {
        sf::RectangleShape shape;
        std::unique_ptr<sf::Text> text;
        std::string action;
        bool isHovered = false;
        MenuButtonConfig config;

        MenuButton() = default;
        MenuButton(const MenuButton&) = delete;
        MenuButton& operator=(const MenuButton&) = delete;
        MenuButton(MenuButton&&) = default;
        MenuButton& operator=(MenuButton&&) = default;
    };
    std::vector<MenuButton> buttons;

    bool visible = true;
    std::unordered_map<std::string, std::function<void()>> actionCallbacks;

    // ========================================================================
    // 顏色 / 文字 解析
    // ========================================================================
    static sf::Color parseColor(const json& arr, sf::Color defaultColor = sf::Color::White) {
        if (arr.is_array() && arr.size() >= 4) {
            return sf::Color(
                static_cast<std::uint8_t>(arr[0].get<int>()),
                static_cast<std::uint8_t>(arr[1].get<int>()),
                static_cast<std::uint8_t>(arr[2].get<int>()),
                static_cast<std::uint8_t>(arr[3].get<int>())
            );
        }
        if (arr.is_array() && arr.size() == 3) {
            return sf::Color(
                static_cast<std::uint8_t>(arr[0].get<int>()),
                static_cast<std::uint8_t>(arr[1].get<int>()),
                static_cast<std::uint8_t>(arr[2].get<int>())
            );
        }
        return defaultColor;
    }

    static MenuTextConfig parseTextConfig(const json& j, const MenuTextConfig& def) {
        MenuTextConfig cfg = def;
        cfg.text     = j.value("text", def.text);
        cfg.posX     = j.value("posX", def.posX);
        cfg.posY     = j.value("posY", def.posY);
        cfg.fontSize = j.value("fontSize", def.fontSize);
        if (j.contains("color")) cfg.color = parseColor(j["color"], def.color);
        return cfg;
    }

    // ========================================================================
    // 預設配置
    // ========================================================================
    void buildDefaultConfig() {
        titleConfig    = {"GALITY ENGINE",  -1.0f, 180.0f,  84, sf::Color::White};
        subtitleConfig = {"",                -1.0f, 300.0f,  20, sf::Color(200, 210, 230)};
        versionConfig  = {"v0.2.0",        1850.0f, 1040.0f, 16, sf::Color(120, 130, 150)};
        backgroundPath = "assets/bg/classroom.jpg";

        buttonConfigs.clear();
        {
            MenuButtonConfig b;
            b.action = "start"; b.text = "START GAME";
            b.posX = -1.0f; b.posY = 460.0f;
            buttonConfigs.push_back(b);
        }
        {
            MenuButtonConfig b;
            b.action = "load"; b.text = "LOAD GAME";
            b.posX = -1.0f; b.posY = 555.0f;
            buttonConfigs.push_back(b);
        }
        {
            MenuButtonConfig b;
            b.action = "settings"; b.text = "SETTINGS";
            b.posX = -1.0f; b.posY = 650.0f;
            buttonConfigs.push_back(b);
        }
        {
            MenuButtonConfig b;
            b.action = "exit"; b.text = "EXIT";
            b.posX = -1.0f; b.posY = 745.0f;
            b.baseColor = sf::Color(20, 20, 40, 200);
            b.hoverColor = sf::Color(140, 50, 50, 240);
            b.hoverOutlineColor = sf::Color(255, 100, 100, 255);
            buttonConfigs.push_back(b);
        }

        // 預設 BGM 為空（靜音）
        bgmConfig = TitleBGMConfig{};
    }

    // ========================================================================
    // JSON 解析
    // ========================================================================
    bool loadConfigFromJson(const json& j) {
        // ===== BGM =====
        if (j.contains("bgm")) {
            if (j["bgm"].is_string()) {
                bgmConfig.path = j["bgm"].get<std::string>();
                bgmConfig.volume = 1.0f;
                bgmConfig.fadeIn = 1.5f;
                bgmConfig.loop = true;
                bgmConfig.stopOnStart = true;
            } else if (j["bgm"].is_object()) {
                const auto& bgm = j["bgm"];
                bgmConfig.path        = bgm.value("path", "");
                bgmConfig.volume      = bgm.value("volume", 1.0f);
                bgmConfig.fadeIn      = bgm.value("fadeIn", 1.5f);
                bgmConfig.loop        = bgm.value("loop", true);
                bgmConfig.stopOnStart = bgm.value("stopOnStart", true);
            }
        } else {
            bgmConfig.path = "";  // 靜音
        }

        // ===== 背景 =====
        if (j.contains("background")) {
            if (j["background"].is_string()) {
                backgroundPath = j["background"].get<std::string>();
            } else if (j["background"].is_object()) {
                const auto& bg = j["background"];
                backgroundPath = bg.value("image", "");

                if (bg.contains("kenBurns")) {
                    const auto& kb = bg["kenBurns"];
                    kenBurnsConfig.enabled       = kb.value("enabled", true);
                    kenBurnsConfig.zoomRange     = kb.value("zoomRange", 0.05f);
                    kenBurnsConfig.panRange      = kb.value("panRange", 30.0f);
                    kenBurnsConfig.cycleDuration = kb.value("cycleDuration", 20.0f);
                }

                darkenAmount = bg.value("darken", 100);
            }
        }

        // ===== 程式碼瀑布 =====
        if (j.contains("codeRain")) {
            const auto& cr = j["codeRain"];
            codeRainConfig.enabled    = cr.value("enabled", true);
            codeRainConfig.intensity  = cr.value("intensity", 0.15f);
            codeRainConfig.shaderPath = cr.value("shader", "assets/shaders/code_rain.frag");
            if (cr.contains("color")) {
                codeRainConfig.color = parseColor(cr["color"], codeRainConfig.color);
            }
        }

        // ===== 櫻花 =====
        if (j.contains("sakura")) {
            const auto& sk = j["sakura"];
            sakuraEnabled       = sk.value("enabled", true);
            sakuraParticleCount = sk.value("particleCount", 60);
        }

        // ===== 文字 =====
        if (j.contains("title"))    titleConfig    = parseTextConfig(j["title"], titleConfig);
        if (j.contains("subtitle")) subtitleConfig = parseTextConfig(j["subtitle"], subtitleConfig);
        if (j.contains("version"))  versionConfig  = parseTextConfig(j["version"], versionConfig);

        // ===== 按鈕 =====
        if (j.contains("buttons") && j["buttons"].is_array()) {
            buttonConfigs.clear();
            for (const auto& btnJson : j["buttons"]) {
                MenuButtonConfig cfg;
                cfg.action   = btnJson.value("action", "");
                cfg.text     = btnJson.value("text", "");
                cfg.posX     = btnJson.value("posX", -1.0f);
                cfg.posY     = btnJson.value("posY", 0.0f);
                cfg.width    = btnJson.value("width", 400.0f);
                cfg.height   = btnJson.value("height", 65.0f);
                cfg.fontSize = btnJson.value("fontSize", 28);

                if (btnJson.contains("baseColor"))         cfg.baseColor         = parseColor(btnJson["baseColor"],         cfg.baseColor);
                if (btnJson.contains("hoverColor"))        cfg.hoverColor        = parseColor(btnJson["hoverColor"],        cfg.hoverColor);
                if (btnJson.contains("outlineColor"))      cfg.outlineColor      = parseColor(btnJson["outlineColor"],      cfg.outlineColor);
                if (btnJson.contains("hoverOutlineColor")) cfg.hoverOutlineColor = parseColor(btnJson["hoverOutlineColor"], cfg.hoverOutlineColor);
                if (btnJson.contains("textColor"))         cfg.textColor         = parseColor(btnJson["textColor"],         cfg.textColor);

                buttonConfigs.push_back(cfg);
            }
        }

        return true;
    }

    // ========================================================================
    // 載入程式碼瀑布 Shader
    // ========================================================================
    void loadCodeRainShader() {
        codeRainLoaded = false;
        if (!codeRainConfig.enabled) return;

        std::string shaderSource;
        bool loaded = false;

        if (AssetPack::readTextFile(codeRainConfig.shaderPath, shaderSource, "data.pak")) {
            loaded = codeRainShader.loadFromMemory(shaderSource, sf::Shader::Type::Fragment);
        }

        if (!loaded) {
            std::string diskPath = codeRainConfig.shaderPath;
            if (diskPath.rfind("assets/", 0) != 0) {
                diskPath = "assets/" + diskPath;
            }
            loaded = codeRainShader.loadFromFile(diskPath, sf::Shader::Type::Fragment);
        }

        if (loaded) {
            codeRainLoaded = true;
            std::cout << "[TitleMenu] Code rain shader loaded: " << codeRainConfig.shaderPath << std::endl;
        } else {
            std::cerr << "[TitleMenu] Failed to load code rain shader: " << codeRainConfig.shaderPath << std::endl;
        }
    }

    // ========================================================================
    // 重建 UI
    // ========================================================================
    void rebuildUI() {
        if (!hasFont) return;

        // ===== 背景 =====
        backgroundSprite.reset();
        backgroundTexture.reset();

        if (!backgroundPath.empty()) {
            backgroundTexture = std::make_shared<sf::Texture>();

            std::vector<std::uint8_t> bytes;
            bool loaded = false;

            if (AssetPack::readFileFromPak(backgroundPath, bytes, "data.pak") && !bytes.empty()) {
                loaded = backgroundTexture->loadFromMemory(bytes.data(), bytes.size());
            }

            if (!loaded) {
                std::string diskPath = backgroundPath;
                if (diskPath.rfind("assets/", 0) != 0) {
                    diskPath = "assets/" + diskPath;
                }
                loaded = backgroundTexture->loadFromFile(diskPath);
            }

            if (loaded) {
                backgroundTexture->setSmooth(true);
                backgroundSprite = std::make_unique<sf::Sprite>(*backgroundTexture);

                auto bounds = backgroundSprite->getLocalBounds();
                if (bounds.size.x > 0 && bounds.size.y > 0) {
                    float scaleX = 1920.0f / bounds.size.x;
                    float scaleY = 1080.0f / bounds.size.y;
                    float scale = std::max(scaleX, scaleY);
                    backgroundSprite->setScale(sf::Vector2f(scale, scale));
                    backgroundSprite->setOrigin(sf::Vector2f(bounds.size.x * 0.5f, bounds.size.y * 0.5f));
                    backgroundSprite->setPosition(sf::Vector2f(960.0f, 540.0f));
                }
                std::cout << "[TitleMenu] Background loaded: " << backgroundPath << std::endl;
            } else {
                std::cerr << "[TitleMenu] Failed to load background: " << backgroundPath << std::endl;
                backgroundTexture.reset();
            }
        }

        // ===== Shader =====
        loadCodeRainShader();
        if (codeRainLoaded) {
            if (!codeRainBuffer.resize(sf::Vector2u(1920, 1080))) {
                std::cerr << "[TitleMenu] Failed to create code rain buffer!" << std::endl;
                codeRainLoaded = false;
            }
        }

        // ===== 櫻花 =====
        if (sakuraEnabled) {
            sakuraWeather = std::make_unique<WeatherSystem>(sf::Vector2u(1920, 1080));
            sakuraWeather->setParticleCount(sakuraParticleCount);
            sakuraWeather->setWeather(WeatherType::Sakura);
        } else {
            sakuraWeather.reset();
        }

        // ===== 標題 =====
        titleText.setFont(font);
        titleText.setString(sf::String::fromUtf8(titleConfig.text.begin(), titleConfig.text.end()));
        titleText.setCharacterSize(titleConfig.fontSize);
        titleText.setFillColor(titleConfig.color);
        {
            sf::FloatRect tb = titleText.getLocalBounds();
            float x = (titleConfig.posX < 0.0f) ? (1920.0f - tb.size.x) * 0.5f : titleConfig.posX;
            titleText.setPosition(sf::Vector2f(x, titleConfig.posY));
        }

        // ===== 副標題 =====
        subtitleText.setFont(font);
        subtitleText.setString(sf::String::fromUtf8(subtitleConfig.text.begin(), subtitleConfig.text.end()));
        subtitleText.setCharacterSize(subtitleConfig.fontSize);
        subtitleText.setFillColor(subtitleConfig.color);
        {
            sf::FloatRect sb = subtitleText.getLocalBounds();
            float x = (subtitleConfig.posX < 0.0f) ? (1920.0f - sb.size.x) * 0.5f : subtitleConfig.posX;
            subtitleText.setPosition(sf::Vector2f(x, subtitleConfig.posY));
        }

        // ===== 版本號 =====
        versionText.setFont(font);
        versionText.setString(sf::String::fromUtf8(versionConfig.text.begin(), versionConfig.text.end()));
        versionText.setCharacterSize(versionConfig.fontSize);
        versionText.setFillColor(versionConfig.color);
        {
            sf::FloatRect vb = versionText.getLocalBounds();
            float x = (versionConfig.posX < 0.0f) ? (1920.0f - vb.size.x) * 0.5f : versionConfig.posX;
            versionText.setPosition(sf::Vector2f(x, versionConfig.posY));
        }

        // ===== 按鈕 =====
        buttons.clear();
        for (const auto& cfg : buttonConfigs) {
            MenuButton btn;
            btn.config = cfg;
            btn.action = cfg.action;

            float x = (cfg.posX < 0.0f) ? (1920.0f - cfg.width) * 0.5f : cfg.posX;

            btn.shape.setSize(sf::Vector2f(cfg.width, cfg.height));
            btn.shape.setPosition(sf::Vector2f(x, cfg.posY));
            btn.shape.setFillColor(cfg.baseColor);
            btn.shape.setOutlineThickness(2.5f);
            btn.shape.setOutlineColor(cfg.outlineColor);

            btn.text = std::make_unique<sf::Text>(font);
            btn.text->setString(sf::String::fromUtf8(cfg.text.begin(), cfg.text.end()));
            btn.text->setCharacterSize(cfg.fontSize);
            btn.text->setFillColor(cfg.textColor);

            sf::FloatRect tb = btn.text->getLocalBounds();
            btn.text->setPosition(sf::Vector2f(
                x + (cfg.width - tb.size.x) * 0.5f,
                cfg.posY + (cfg.height - tb.size.y) * 0.5f - 6.0f
            ));

            buttons.push_back(std::move(btn));
        }
    }

public:
    TitleMenu()
        : titleText(font)
        , subtitleText(font)
        , versionText(font)
    {}

    // ========================================================================
    // 載入字型
    // ========================================================================
    bool loadFont(const std::string& fontPath) {
        fontBytes.clear();
        hasFont = false;

        if (AssetPack::readFileFromPak(fontPath, fontBytes, "data.pak") && !fontBytes.empty()) {
            if (font.openFromMemory(fontBytes.data(), fontBytes.size())) {
                hasFont = true;
            }
        } else if (font.openFromFile(fontPath)) {
            hasFont = true;
        }

        if (hasFont) {
            buildDefaultConfig();
            rebuildUI();
            return true;
        }
        return false;
    }

    // ========================================================================
    // 載入配置
    // ========================================================================
    bool loadConfig(const std::string& configPath = "assets/config/title_menu.json") {
        std::string payload;
        bool fromPak = AssetPack::readTextFile(configPath, payload, "data.pak");

        if (!fromPak) {
            std::ifstream file(configPath);
            if (!file.is_open()) {
                std::cerr << "[TitleMenu] Config not found: " << configPath
                          << " (using defaults)" << std::endl;
                return false;
            }
            std::stringstream ss;
            ss << file.rdbuf();
            payload = ss.str();
        }

        try {
            json j = json::parse(payload);
            loadConfigFromJson(j);
            rebuildUI();

            if (visible && bgmConfig.isEnabled()) {
                playBGM();
            }

            std::cout << "[TitleMenu] Config loaded: " << configPath << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[TitleMenu] Parse error: " << e.what() << std::endl;
            return false;
        }
    }

    // ========================================================================
    // BGM 回呼設定
    // ========================================================================
    void setBGMRequestCallback(std::function<void(const std::string&, float, bool, float)> callback) {
        onRequestBGM = callback;
    }

    void setBGMStopCallback(std::function<void()> callback) {
        onStopBGM = callback;
    }

    // ========================================================================
    // 手動設定 BGM
    // ========================================================================
    void setBGM(const TitleBGMConfig& config) {
        bgmConfig = config;
    }

    const TitleBGMConfig& getBGM() const {
        return bgmConfig;
    }

    // ========================================================================
    // BGM 播放 / 停止
    // ========================================================================
    void playBGM() {
        if (!bgmConfig.isEnabled()) return;
        if (!onRequestBGM) return;

        if (currentBgmPath != bgmConfig.path) {
            currentBgmPath = bgmConfig.path;
            onRequestBGM(bgmConfig.path, bgmConfig.volume, bgmConfig.loop, bgmConfig.fadeIn);
            std::cout << "[TitleMenu] Playing BGM: " << bgmConfig.path << std::endl;
        }
    }

    void stopBGM() {
        if (!bgmConfig.isEnabled()) return;
        if (currentBgmPath.empty()) return;
        if (onStopBGM) onStopBGM();
        currentBgmPath = "";
        std::cout << "[TitleMenu] Stopped BGM" << std::endl;
    }

    void onGameStart() {
        if (bgmConfig.stopOnStart) {
            stopBGM();
        }
    }

    // ========================================================================
    // 動作註冊
    // ========================================================================
    void registerAction(const std::string& action, std::function<void()> callback) {
        actionCallbacks[action] = callback;
    }

    void setVisible(bool show) {
        visible = show;
        if (show) {
            playBGM();
        } else {
            if (bgmConfig.stopOnStart) {
                stopBGM();
            }
        }
    }

    bool isVisible() const { return visible; }

    // ========================================================================
    // 更新（Ken Burns + 粒子）
    // ========================================================================
    void update(float deltaTime) {
        if (!visible) return;

        if (kenBurnsConfig.enabled && backgroundSprite) {
            kenBurnsTime += deltaTime;

            float t = kenBurnsTime / kenBurnsConfig.cycleDuration;
            float phaseX = std::sin(t * 2.0f * 3.14159265f) * 0.5f + 0.5f;
            float phaseY = std::cos(t * 2.0f * 3.14159265f) * 0.5f + 0.5f;
            float phaseZ = std::sin(t * 3.14159265f);

            float baseScale = 1.0f;
            if (backgroundTexture) {
                auto sz = backgroundTexture->getSize();
                float scaleX = 1920.0f / static_cast<float>(sz.x);
                float scaleY = 1080.0f / static_cast<float>(sz.y);
                baseScale = std::max(scaleX, scaleY);
            }
            float zoom = baseScale * (1.0f + phaseZ * kenBurnsConfig.zoomRange);

            float offsetX = (phaseX - 0.5f) * 2.0f * kenBurnsConfig.panRange;
            float offsetY = (phaseY - 0.5f) * 2.0f * kenBurnsConfig.panRange;

            backgroundSprite->setScale(sf::Vector2f(zoom, zoom));
            backgroundSprite->setPosition(sf::Vector2f(960.0f + offsetX, 540.0f + offsetY));
        }

        if (sakuraWeather) {
            sakuraWeather->update(deltaTime);
        }
    }

    // ========================================================================
    // 滑鼠事件
    // ========================================================================
    void updateHover(sf::Vector2f mousePosF) {
        if (!visible) return;
        for (auto& btn : buttons) {
            if (btn.shape.getGlobalBounds().contains(mousePosF)) {
                btn.isHovered = true;
                btn.shape.setFillColor(btn.config.hoverColor);
                btn.shape.setOutlineColor(btn.config.hoverOutlineColor);
            } else {
                btn.isHovered = false;
                btn.shape.setFillColor(btn.config.baseColor);
                btn.shape.setOutlineColor(btn.config.outlineColor);
            }
        }
    }

    bool handleClick(sf::Vector2f mousePosF) {
        if (!visible) return false;
        for (auto& btn : buttons) {
            if (btn.shape.getGlobalBounds().contains(mousePosF)) {
                auto it = actionCallbacks.find(btn.action);
                if (it != actionCallbacks.end() && it->second) {
                    it->second();
                } else {
                    std::cerr << "[TitleMenu] No callback for action: " << btn.action << std::endl;
                }
                return true;
            }
        }
        return false;
    }

    // ========================================================================
    // 繪製
    // ========================================================================
    void draw(sf::RenderTarget& target) {
        if (!visible || !hasFont) return;

        // Layer 1: 背景 + Shader
        if (backgroundSprite && codeRainLoaded) {
            codeRainBuffer.clear(sf::Color::Transparent);
            codeRainBuffer.draw(*backgroundSprite);
            codeRainBuffer.display();

            codeRainShader.setUniform("u_texture", codeRainBuffer.getTexture());
            codeRainShader.setUniform("u_resolution", sf::Glsl::Vec2(1920.0f, 1080.0f));
            codeRainShader.setUniform("u_time", kenBurnsTime);
            codeRainShader.setUniform("u_intensity", codeRainConfig.intensity);
            codeRainShader.setUniform("u_color", sf::Glsl::Vec3(
                codeRainConfig.color.r / 255.0f,
                codeRainConfig.color.g / 255.0f,
                codeRainConfig.color.b / 255.0f
            ));

            sf::Sprite shaderSprite(codeRainBuffer.getTexture());
            target.draw(shaderSprite, &codeRainShader);
        } else if (backgroundSprite) {
            target.draw(*backgroundSprite);
        } else {
            sf::RectangleShape fallback(sf::Vector2f(1920.0f, 1080.0f));
            fallback.setFillColor(sf::Color(40, 45, 60));
            target.draw(fallback);
        }

        // Layer 2: 暗色遮罩
        if (darkenAmount > 0) {
            sf::RectangleShape overlay(sf::Vector2f(1920.0f, 1080.0f));
            overlay.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(darkenAmount)));
            target.draw(overlay);
        }

        // Layer 3: 櫻花
        if (sakuraWeather) {
            sakuraWeather->draw(target);
        }

        // Layer 4: 文字
        if (!titleConfig.text.empty())    target.draw(titleText);
        if (!subtitleConfig.text.empty()) target.draw(subtitleText);
        if (!versionConfig.text.empty())  target.draw(versionText);

        // Layer 5: 按鈕
        for (auto& btn : buttons) {
            target.draw(btn.shape);
            if (btn.text) target.draw(*btn.text);
        }
    }
};