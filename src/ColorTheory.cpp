#include "ColorTheory.h"
#include <algorithm>
#include <cmath>

namespace ofxSwatches {

namespace {

float srgbChannelToLinear(float c) {
    c /= 255.0f;
    if (c <= 0.04045f) return c / 12.92f;
    return std::pow((c + 0.055f) / 1.055f, 2.4f);
}

float relativeLuminance(const ofColor& c) {
    const float r = srgbChannelToLinear(c.r);
    const float g = srgbChannelToLinear(c.g);
    const float b = srgbChannelToLinear(c.b);
    return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}

} // namespace

float getGreyValuePercent(const ofColor& c) {
    return ofClamp(relativeLuminance(c) * 100.0f, 0.0f, 100.0f);
}

float getGreyValueSimple(const ofColor& c) {
    const float y = (0.299f * c.r + 0.587f * c.g + 0.114f * c.b) / 255.0f;
    return ofClamp(y * 100.0f, 0.0f, 100.0f);
}

bool isNeutralGrey(const ofColor& c, float channelEpsilon) {
    return std::abs((float)c.r - c.g) <= channelEpsilon
        && std::abs((float)c.g - c.b) <= channelEpsilon
        && std::abs((float)c.r - c.b) <= channelEpsilon;
}

ofColor toGreyEquivalent(const ofColor& c) {
    const float pct = getGreyValuePercent(c) / 100.0f;
    const unsigned char g = (unsigned char)ofClamp(pct * 255.0f, 0.0f, 255.0f);
    return ofColor(g, g, g, c.a);
}

float contrastRatio(const ofColor& a, const ofColor& b) {
    float l1 = relativeLuminance(a);
    float l2 = relativeLuminance(b);
    if (l1 < l2) std::swap(l1, l2);
    return (l1 + 0.05f) / (l2 + 0.05f);
}

bool meetsContrastAA(float ratio) {
    return ratio >= 4.5f;
}

bool meetsContrastAAA(float ratio) {
    return ratio >= 7.0f;
}

glm::vec3 getLab(const ofColor& c) {
    float r = srgbChannelToLinear(c.r);
    float g = srgbChannelToLinear(c.g);
    float b = srgbChannelToLinear(c.b);

    float x = r * 0.4124564f + g * 0.3575761f + b * 0.1804375f;
    float y = r * 0.2126729f + g * 0.7151522f + b * 0.0721750f;
    float z = r * 0.0193339f + g * 0.1191920f + b * 0.9503041f;

    auto f = [](float t) {
        const float d = 6.0f / 29.0f;
        if (t > d * d * d) return std::cbrt(t);
        return t / (3.0f * d * d) + 4.0f / 29.0f;
    };

    const float refX = 0.95047f;
    const float refY = 1.00000f;
    const float refZ = 1.08883f;

    x = f(x / refX);
    y = f(y / refY);
    z = f(z / refZ);

    const float L = ofClamp(116.0f * y - 16.0f, 0.0f, 100.0f);
    const float a = 500.0f * (x - y);
    const float bLab = 200.0f * (y - z);
    return glm::vec3(L, a, bLab);
}

} // namespace ofxSwatches
