#include "SwatchColor.h"
#include <sstream>
#include <cmath>

namespace ofxSwatches {

ofColor SwatchColor::cmykToRgb(float c, float m, float y, float k) {
    c = ofClamp(c, 0.0f, 100.0f) / 100.0f;
    m = ofClamp(m, 0.0f, 100.0f) / 100.0f;
    y = ofClamp(y, 0.0f, 100.0f) / 100.0f;
    k = ofClamp(k, 0.0f, 100.0f) / 100.0f;

    const float r = 255.0f * (1.0f - c) * (1.0f - k);
    const float g = 255.0f * (1.0f - m) * (1.0f - k);
    const float b = 255.0f * (1.0f - y) * (1.0f - k);
    return ofColor((unsigned char)r, (unsigned char)g, (unsigned char)b, 255);
}

glm::vec4 SwatchColor::rgbToCmyk(const ofColor& col, const RichBlackPreset* richBlack) {
    const float r = col.r / 255.0f;
    const float g = col.g / 255.0f;
    const float b = col.b / 255.0f;

    float k = 1.0f - std::max({r, g, b});

    if (k >= 1.0f - 0.0001f) {
        if (richBlack) {
            return glm::vec4(richBlack->c, richBlack->m, richBlack->y, richBlack->k);
        }
        return glm::vec4(0.0f, 0.0f, 0.0f, 100.0f);
    }

    const float c = (1.0f - r - k) / (1.0f - k);
    const float m = (1.0f - g - k) / (1.0f - k);
    const float y = (1.0f - b - k) / (1.0f - k);
    return glm::vec4(c * 100.0f, m * 100.0f, y * 100.0f, k * 100.0f);
}

SwatchColor SwatchColor::fromCMYK(float c, float m, float y, float k, const std::string& n) {
    SwatchColor result;
    result.cmyk100 = glm::vec4(
        ofClamp(c, 0.0f, 100.0f),
        ofClamp(m, 0.0f, 100.0f),
        ofClamp(y, 0.0f, 100.0f),
        ofClamp(k, 0.0f, 100.0f));
    result.color = cmykToRgb(c, m, y, k);
    result.name = n;
    result.type = SwatchColorType::CMYK;
    return result;
}

glm::vec4 SwatchColor::getCMYK() const {
    if (type == SwatchColorType::CMYK) {
        return cmyk100;
    }
    return rgbToCmyk(color);
}

glm::vec3 SwatchColor::getHSB() const {
    return glm::vec3(color.getHue(), color.getSaturation(), color.getBrightness());
}

SwatchColor SwatchColor::fromHSB(float h, float s, float b, const std::string& n) {
    SwatchColor result;
    result.color.setHsb(h, s, b);
    result.name = n;
    result.type = SwatchColorType::RGB;
    return result;
}

std::string SwatchColor::getDisplayName() const {
    if (!name.empty()) return name;

    std::stringstream ss;
    if (type == SwatchColorType::CMYK) {
        const glm::vec4 cmyk = getCMYK();
        ss << "C" << (int)cmyk.r << " M" << (int)cmyk.g << " Y" << (int)cmyk.b << " K" << (int)cmyk.a;
    } else {
        ss << "R" << (int)color.r << " G" << (int)color.g << " B" << (int)color.b;
    }
    return ss.str();
}

SwatchColor SwatchColor::withGreyValuePercent(float targetPercent) const {
    const float current = getGreyValuePercent();
    if (std::abs(current - targetPercent) < 0.01f) return *this;

    const float h = color.getHue();
    const float s = color.getSaturation();
    const float b = color.getBrightness();

    SwatchColor result = *this;
    if (current < 0.001f) {
        const float g = targetPercent / 100.0f * 255.0f;
        result.color.set(g, g, g, color.a);
        result.type = SwatchColorType::RGB;
        return result;
    }

    const float factor = (targetPercent / 100.0f) / (current / 100.0f);
    result.color.setHsb(h, s, ofClamp(b * factor, 0.0f, 255.0f));
    result.color.a = color.a;
    if (type == SwatchColorType::CMYK) {
        result.cmyk100 = rgbToCmyk(result.color);
        result.type = SwatchColorType::CMYK;
    }
    return result;
}

SwatchColor SwatchColor::shiftHue(float degrees) const {
    float h = color.getHue();
    const float s = color.getSaturation();
    const float b = color.getBrightness();

    const float hueShift = (degrees / 360.0f) * 255.0f;
    h = fmod(h + hueShift + 255.0f, 255.0f);

    SwatchColor result;
    result.color.setHsb(h, s, b);
    result.color.a = color.a;
    result.type = type;
    result.cmyk100 = cmyk100;
    return result;
}

SwatchColor SwatchColor::adjustSaturation(float factor) const {
    const float h = color.getHue();
    float s = color.getSaturation();
    const float b = color.getBrightness();

    s = ofClamp(s * factor, 0.0f, 255.0f);

    SwatchColor result;
    result.color.setHsb(h, s, b);
    result.color.a = color.a;
    result.type = type;
    result.cmyk100 = cmyk100;
    return result;
}

SwatchColor SwatchColor::adjustBrightness(float factor) const {
    const float h = color.getHue();
    const float s = color.getSaturation();
    float b = color.getBrightness();

    b = ofClamp(b * factor, 0.0f, 255.0f);

    SwatchColor result;
    result.color.setHsb(h, s, b);
    result.color.a = color.a;
    result.type = type;
    result.cmyk100 = cmyk100;
    return result;
}

SwatchColor SwatchColor::getComplementary() const {
    // Achromatic / near-grey: complementary is black ↔ white by luminance, not hue +180°.
    const float sat = color.getSaturation();
    if (ofxSwatches::isNeutralGrey(color) || sat < 8.f) {
        SwatchColor result;
        result.type = SwatchColorType::RGB;
        result.color.a = color.a;
        if (ofxSwatches::getGreyValuePercent(color) < 50.f) {
            result.color = ofColor(255, 255, 255, color.a);
            result.name = name.empty() ? "White" : name + " (Complementary)";
        } else {
            result.color = ofColor(0, 0, 0, color.a);
            result.name = name.empty() ? "Black" : name + " (Complementary)";
        }
        return result;
    }

    SwatchColor result = shiftHue(180.0f);
    result.name = name.empty() ? "Complementary" : name + " (Complementary)";
    return result;
}

std::vector<SwatchColor> SwatchColor::getTriadic() const {
    std::vector<SwatchColor> results;
    SwatchColor c1 = shiftHue(120.0f);
    c1.name = name.empty() ? "Triadic 1" : name + " (Triadic 1)";
    results.push_back(c1);
    SwatchColor c2 = shiftHue(240.0f);
    c2.name = name.empty() ? "Triadic 2" : name + " (Triadic 2)";
    results.push_back(c2);
    return results;
}

std::vector<SwatchColor> SwatchColor::getTetradic() const {
    std::vector<SwatchColor> results;
    SwatchColor c1 = shiftHue(90.0f);
    c1.name = name.empty() ? "Tetradic 1" : name + " (Tetradic 1)";
    results.push_back(c1);
    SwatchColor c2 = shiftHue(180.0f);
    c2.name = name.empty() ? "Tetradic 2" : name + " (Tetradic 2)";
    results.push_back(c2);
    SwatchColor c3 = shiftHue(270.0f);
    c3.name = name.empty() ? "Tetradic 3" : name + " (Tetradic 3)";
    results.push_back(c3);
    return results;
}

std::vector<SwatchColor> SwatchColor::getAnalogous(float angle) const {
    std::vector<SwatchColor> results;
    SwatchColor c1 = shiftHue(-angle);
    c1.name = name.empty() ? "Analogous 1" : name + " (Analogous 1)";
    results.push_back(c1);
    SwatchColor c2 = shiftHue(angle);
    c2.name = name.empty() ? "Analogous 2" : name + " (Analogous 2)";
    results.push_back(c2);
    return results;
}

std::vector<SwatchColor> SwatchColor::getSplitComplementary() const {
    std::vector<SwatchColor> results;
    SwatchColor c1 = shiftHue(150.0f);
    c1.name = name.empty() ? "Split Comp 1" : name + " (Split Comp 1)";
    results.push_back(c1);
    SwatchColor c2 = shiftHue(210.0f);
    c2.name = name.empty() ? "Split Comp 2" : name + " (Split Comp 2)";
    results.push_back(c2);
    return results;
}

std::vector<SwatchColor> SwatchColor::getMonochromatic(int count) const {
    std::vector<SwatchColor> results;
    const float h = color.getHue();
    const float s = color.getSaturation();
    const float b = color.getBrightness();

    for (int i = 1; i <= count; i++) {
        const float factor = 0.3f + (0.7f * i / (float)count);
        SwatchColor c;
        c.color.setHsb(h, s * factor, ofClamp(b * (0.5f + factor * 0.5f), 0, 255));
        c.color.a = color.a;
        c.type = type;
        c.name = name.empty() ? ("Mono " + std::to_string(i)) : (name + " (Mono " + std::to_string(i) + ")");
        results.push_back(c);
    }
    return results;
}

std::vector<SwatchColor> SwatchColor::generateHarmony(ColorHarmony harmony) const {
    switch (harmony) {
        case ColorHarmony::Complementary:
            return {getComplementary()};
        case ColorHarmony::Triadic:
            return getTriadic();
        case ColorHarmony::Tetradic:
            return getTetradic();
        case ColorHarmony::Analogous:
            return getAnalogous();
        case ColorHarmony::SplitComplementary:
            return getSplitComplementary();
        case ColorHarmony::Monochromatic:
            return getMonochromatic();
    }
    return {};
}

} // namespace ofxSwatches
