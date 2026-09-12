#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class DialogueBox {
private:
    sf::RectangleShape boxShape;
    sf::RectangleShape nameBoxShape;
    sf::Font font;
    sf::Text nameText;
    sf::Text dialogueText;

    std::string fullText;
    std::u32string unicodeText;
    size_t visibleCharCount = 0;
    
    sf::Clock timer;
    float charDelay = 0.04f;
    bool isCompleted = false;

    std::u32string utf8ToUtf32(const std::string& str) {
        // SFML 3.x 原生支援 UTF-8 轉換
        sf::String sfStr = sf::String::fromUtf8(str.begin(), str.end());
        std::u32string u32;
        for (std::size_t i = 0; i < sfStr.getSize(); ++i) {
            u32.push_back(sfStr[i]);
        }
        return u32;
    }

public:
    // 💡 SFML 3.x：Text 必須在建構時綁定 font
    DialogueBox() : nameText(font), dialogueText(font) 
    {
        // 💡 顯式使用 sf::Vector2f 避免 MSVC 推導錯誤
        boxShape.setSize(sf::Vector2f(1200.f, 200.f));
        boxShape.setFillColor(sf::Color(0, 0, 0, 200));
        boxShape.setOutlineThickness(2.f);
        boxShape.setOutlineColor(sf::Color(255, 255, 255, 100));
        boxShape.setPosition(sf::Vector2f(40.f, 480.f));

        nameBoxShape.setSize(sf::Vector2f(250.f, 45.f));
        nameBoxShape.setFillColor(sf::Color(40, 40, 90, 220));
        nameBoxShape.setPosition(sf::Vector2f(40.f, 430.f));

        nameText.setCharacterSize(22);
        nameText.setFillColor(sf::Color::Yellow);
        nameText.setPosition(sf::Vector2f(55.f, 438.f));

        dialogueText.setCharacterSize(24);
        dialogueText.setFillColor(sf::Color::White);
        dialogueText.setPosition(sf::Vector2f(60.f, 500.f));
    }

    bool loadFont(const std::string& fontPath) {
        return font.openFromFile(fontPath);
    }

    void setText(const std::string& speaker, const std::string& text) {
        nameText.setString(sf::String::fromUtf8(speaker.begin(), speaker.end()));
        fullText = text;
        unicodeText = utf8ToUtf32(text);
        visibleCharCount = 0;
        isCompleted = false;
        timer.restart();
    }

    void update() {
        if (isCompleted) return;

        if (timer.getElapsedTime().asSeconds() >= charDelay) {
            timer.restart();
            if (visibleCharCount < unicodeText.size()) {
                visibleCharCount++;
                std::u32string currentSubStr = unicodeText.substr(0, visibleCharCount);
                dialogueText.setString(sf::String(currentSubStr.c_str()));
            } else {
                isCompleted = true;
            }
        }
    }

    bool onInteract() {
        if (!isCompleted) {
            visibleCharCount = unicodeText.size();
            dialogueText.setString(sf::String(unicodeText.c_str()));
            isCompleted = true;
            return false;
        }
        return true;
    }

    void draw(sf::RenderWindow& window) {
        window.draw(boxShape);
        window.draw(nameBoxShape);
        window.draw(nameText);
        window.draw(dialogueText);
    }
};