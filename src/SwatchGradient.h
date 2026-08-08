#pragma once

#include "ofMain.h"
#include <string>
#include <vector>

namespace ofxSwatches {

/// Portable gradient payload for swatch books (mirrors ecs::gradient_component).
enum class SwatchGradientType { Linear = 0, Radial };
enum class SwatchGradientInterpolation { RGB = 0, HSV, OkLab };
enum class SwatchGradientSpread { Pad = 0, Repeat, Mirror };

struct SwatchGradientStop {
    float     position  = 0.f;
    glm::vec4 color { 0.f, 0.f, 0.f, 1.f }; // normalized RGBA
    float     intensity = 1.f;

    SwatchGradientStop() = default;
    SwatchGradientStop(float pos, const glm::vec4& col, float intens = 1.f)
        : position(pos), color(col), intensity(intens) {}
};

struct SwatchGradient {
    std::vector<SwatchGradientStop> stops;
    SwatchGradientType          type   = SwatchGradientType::Linear;
    SwatchGradientInterpolation interp = SwatchGradientInterpolation::RGB;
    SwatchGradientSpread        spread = SwatchGradientSpread::Pad;
    float     angle = 90.f;
    glm::vec2 center { 0.5f, 0.5f };
    float     innerRadius = 0.f;
    float     outerRadius = 1.f;
    int       numSteps = 0;

    SwatchGradient() {
        stops.push_back(SwatchGradientStop(0.f, glm::vec4(0.f, 0.f, 0.f, 1.f)));
        stops.push_back(SwatchGradientStop(1.f, glm::vec4(1.f, 1.f, 1.f, 1.f)));
    }

    void sortStops();
};

enum class SwatchKind { Solid = 0, Gradient = 1 };

} // namespace ofxSwatches
