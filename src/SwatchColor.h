#pragma once

#include "ColorTheory.h"
#include "SwatchGradient.h"
#include "ofMain.h"
#include <string>
#include <vector>

namespace ofxSwatches {

enum class SwatchColorType {
    RGB = 0,
    CMYK = 1
};

enum class ColorHarmony {
    Complementary,
    Triadic,
    Tetradic,
    Analogous,
    SplitComplementary,
    Monochromatic
};

struct RichBlackPreset {
    float c = 60.0f;
    float m = 40.0f;
    float y = 40.0f;
    float k = 100.0f;
};

struct SwatchColor {
    std::string name;
    ofColor color = ofColor::white;
    SwatchColorType type = SwatchColorType::RGB;
    bool isSpotColor = false;
    std::string spotInkName;
    /// Authoritative CMYK when type == CMYK (C,M,Y,K 0–100).
    glm::vec4 cmyk100 = glm::vec4(0.0f);

    /// Solid (default) or portable gradient book entry.
    SwatchKind kind = SwatchKind::Solid;
    SwatchGradient gradient;

    SwatchColor() = default;

    SwatchColor(const ofColor& col, const std::string& n = "")
        : name(n), color(col), type(SwatchColorType::RGB) {}

    SwatchColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255, const std::string& n = "")
        : name(n), color(r, g, b, a), type(SwatchColorType::RGB) {}

    static SwatchColor fromCMYK(float c, float m, float y, float k, const std::string& n = "");
    static SwatchColor fromHSB(float h, float s, float b, const std::string& n = "");
    /// Two-stop linear black→white gradient (or custom colours via optional args).
    static SwatchColor fromGradient(const std::string& n = "Gradient",
                                    const glm::vec4& start = glm::vec4(0.f, 0.f, 0.f, 1.f),
                                    const glm::vec4& end = glm::vec4(1.f, 1.f, 1.f, 1.f));

    bool isGradient() const { return kind == SwatchKind::Gradient; }
    bool isSolid() const { return kind == SwatchKind::Solid; }

    /// Chip / DualColor preview — midpoint sample for gradients.
    ofColor previewColor() const;
    /// Refresh `color` from gradient stops (no-op for solids).
    void refreshPreviewFromGradient();

    glm::vec4 getCMYK() const;
    glm::vec3 getHSB() const;
    std::string getDisplayName() const;

    float getGreyValuePercent() const { return ofxSwatches::getGreyValuePercent(previewColor()); }
    float getGreyValueSimple() const { return ofxSwatches::getGreyValueSimple(previewColor()); }
    bool isNeutralGrey() const { return ofxSwatches::isNeutralGrey(previewColor()); }
    ofColor toGreyEquivalent() const { return ofxSwatches::toGreyEquivalent(previewColor()); }

    SwatchColor withGreyValuePercent(float targetPercent) const;

    operator ofColor() const { return previewColor(); }

    /// Harmony helpers operate on solids only; gradients return empty / self.
    std::vector<SwatchColor> generateHarmony(ColorHarmony harmony) const;
    SwatchColor getComplementary() const;
    std::vector<SwatchColor> getTriadic() const;
    std::vector<SwatchColor> getTetradic() const;
    std::vector<SwatchColor> getAnalogous(float angle = 30.0f) const;
    std::vector<SwatchColor> getSplitComplementary() const;
    std::vector<SwatchColor> getMonochromatic(int count = 5) const;

    SwatchColor shiftHue(float degrees) const;
    SwatchColor adjustSaturation(float factor) const;
    SwatchColor adjustBrightness(float factor) const;

    static ofColor cmykToRgb(float c, float m, float y, float k);
    static glm::vec4 rgbToCmyk(const ofColor& col, const RichBlackPreset* richBlack = nullptr);
};

} // namespace ofxSwatches
