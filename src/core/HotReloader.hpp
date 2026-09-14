#pragma once
#include <string>
#include <filesystem>
#include <chrono>
#include <functional>
#include <iostream>

namespace fs = std::filesystem;

class HotReloader {
private:
    std::string scriptPath;
    std::string shaderPath;
    fs::file_time_type lastScriptWriteTime;
    fs::file_time_type lastShaderWriteTime;
    float checkInterval = 1.0f; // 每秒檢測一次檔案修改
    float timer = 0.0f;

public:
    HotReloader(const std::string& script, const std::string& shader = "")
        : scriptPath(script), shaderPath(shader) {
        if (fs::exists(scriptPath)) {
            lastScriptWriteTime = fs::last_write_time(scriptPath);
        }
        if (!shaderPath.empty() && fs::exists(shaderPath)) {
            lastShaderWriteTime = fs::last_write_time(shaderPath);
        }
    }

    void update(float deltaTime, std::function<void()> onScriptReloaded, std::function<void()> onShaderReloaded) {
        timer += deltaTime;
        if (timer < checkInterval) return;
        timer = 0.0f;

        // 檢測 JSON 劇本變更
        try {
            if (fs::exists(scriptPath)) {
                auto currentWriteTime = fs::last_write_time(scriptPath);
                if (currentWriteTime != lastScriptWriteTime) {
                    lastScriptWriteTime = currentWriteTime;
                    std::cout << "[HotReloader] Script file modified. Reloading AST..." << std::endl;
                    if (onScriptReloaded) onScriptReloaded();
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "[HotReloader Error] Script check failed: " << e.what() << std::endl;
        }

        // 檢測 GLSL Shader 變更
        if (!shaderPath.empty()) {
            try {
                if (fs::exists(shaderPath)) {
                    auto currentWriteTime = fs::last_write_time(shaderPath);
                    if (currentWriteTime != lastShaderWriteTime) {
                        lastShaderWriteTime = currentWriteTime;
                        std::cout << "[HotReloader] Shader file modified. Reloading GLSL..." << std::endl;
                        if (onShaderReloaded) onShaderReloaded();
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "[HotReloader Error] Shader check failed: " << e.what() << std::endl;
            }
        }
    }
};