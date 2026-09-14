#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <cmath>

enum class WeatherType {
    None,
    Rain,
    Snow,
    Sakura
};

struct Particle {
    sf::Vector2f position;
    sf::Vector2f velocity;
    float size;
    float alpha;
    float rotation;
    float rotationSpeed;
};

class WeatherSystem {
private:
    WeatherType currentType = WeatherType::None;
    std::vector<Particle> particles;
    std::size_t maxParticles = 150;
    sf::Vector2u windowSize;

    void resetParticle(Particle& p) {
        p.position.x = static_cast<float>(rand() % windowSize.x);
        p.position.y = -10.0f;
        p.alpha = 150.0f + rand() % 105;

        if (currentType == WeatherType::Rain) {
            p.velocity = sf::Vector2f(-50.0f, 600.0f + rand() % 200);
            p.size = 2.0f + rand() % 2;
        } else if (currentType == WeatherType::Snow) {
            p.velocity = sf::Vector2f(-20.0f + rand() % 40, 50.0f + rand() % 50);
            p.size = 3.0f + rand() % 4;
        } else if (currentType == WeatherType::Sakura) {
            p.velocity = sf::Vector2f(-40.0f + rand() % 30, 80.0f + rand() % 40);
            p.size = 6.0f + rand() % 4;
            p.rotation = static_cast<float>(rand() % 360);
            p.rotationSpeed = -60.0f + rand() % 120;
        }
    }

public:
    WeatherSystem(sf::Vector2u winSize) : windowSize(winSize) {}

    void setWeather(WeatherType type) {
        currentType = type;
        particles.clear();
        if (type == WeatherType::None) return;

        particles.resize(maxParticles);
        for (auto& p : particles) {
            resetParticle(p);
            p.position.y = static_cast<float>(rand() % windowSize.y); // 初次隨機分佈
        }
    }

    void update(float deltaTime) {
        if (currentType == WeatherType::None) return;

        for (auto& p : particles) {
            p.position += p.velocity * deltaTime;
            p.rotation += p.rotationSpeed * deltaTime;

            // 超出螢幕邊界重置
            if (p.position.y > windowSize.y || p.position.x < -20.0f || p.position.x > windowSize.x + 20.0f) {
                resetParticle(p);
            }
        }
    }

    void draw(sf::RenderTarget& target) {
        if (currentType == WeatherType::None) return;

        for (const auto& p : particles) {
            if (currentType == WeatherType::Rain) {
                sf::RectangleShape drop(sf::Vector2f(p.size, p.size * 8.0f));
                drop.setPosition(p.position);
                drop.setFillColor(sf::Color(180, 200, 255, static_cast<std::uint8_t>(p.alpha)));
                target.draw(drop);
            } else if (currentType == WeatherType::Snow) {
                sf::CircleShape flake(p.size);
                flake.setPosition(p.position);
                flake.setFillColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(p.alpha)));
                target.draw(flake);
            } else if (currentType == WeatherType::Sakura) {
                sf::RectangleShape petal(sf::Vector2f(p.size, p.size * 1.4f));
                petal.setPosition(p.position);
                petal.setRotation(sf::degrees(p.rotation));
                petal.setFillColor(sf::Color(255, 183, 197, static_cast<std::uint8_t>(p.alpha)));
                target.draw(petal);
            }
        }
    }
};