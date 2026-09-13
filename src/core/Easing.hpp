#pragma once
#include <cmath>
#include <algorithm>

class Easing {
public:
    // 線性插值
    static float linear(float t) {
        return std::clamp(t, 0.0f, 1.0f);
    }

    // 淡入 (Ease In)
    static float easeInQuad(float t) {
        t = std::clamp(t, 0.0f, 1.0f);
        return t * t;
    }

    // 淡出 (Ease Out)
    static float easeOutQuad(float t) {
        t = std::clamp(t, 0.0f, 1.0f);
        return t * (2.0f - t);
    }

    // 淡入淡出 (Ease In-Out)
    static float easeInOutCubic(float t) {
        t = std::clamp(t, 0.0f, 1.0f);
        return t < 0.5f ? 4.0f * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
    }
};