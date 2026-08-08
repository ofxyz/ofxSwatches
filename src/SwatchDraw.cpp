#include "SwatchDraw.h"
#include "SwatchPaintConvert.h"

namespace ofxSwatches {
namespace {

ImU32 toU32(const ofColor& c)
{
    return IM_COL32(c.r, c.g, c.b, c.a);
}

ImU32 toU32(const glm::vec4& c)
{
    return IM_COL32((int)(c.x * 255.f), (int)(c.y * 255.f), (int)(c.z * 255.f),
                    (int)(c.w * 255.f));
}

} // namespace

void drawSwatchRect(ImDrawList* dl, ImVec2 min, ImVec2 max, const SwatchColor& swatch,
                    float rounding)
{
    if (!dl)
        return;
    if (swatch.kind != SwatchKind::Gradient) {
        dl->AddRectFilled(min, max, toU32(swatch.previewColor()), rounding);
        return;
    }

    const ecs::gradient_component g = toGradientComponent(swatch);
    const float w = max.x - min.x;
    if (w <= 1.f) {
        dl->AddRectFilled(min, max, toU32(swatch.previewColor()), rounding);
        return;
    }

    // Horizontal ramp approximating the gradient (linear angle ignored for chips).
    const int steps = std::max(8, (int)w);
    for (int i = 0; i < steps; ++i) {
        const float t0 = (float)i / (float)steps;
        const float t1 = (float)(i + 1) / (float)steps;
        const ImU32 c0 = toU32(g.sample(t0));
        const ImU32 c1 = toU32(g.sample(t1));
        const float x0 = min.x + t0 * w;
        const float x1 = min.x + t1 * w;
        dl->AddRectFilledMultiColor(ImVec2(x0, min.y), ImVec2(x1, max.y), c0, c1, c1, c0);
    }
    if (rounding > 0.f) {
        // Soft clip via border only — full round-rect clip is overkill for chips.
        (void)rounding;
    }
}

void drawSwatchRect(ImDrawList* dl, ImVec2 min, ImVec2 max, SwatchKind kind,
                    const ofColor& solid, const SwatchColor& gradient, float rounding)
{
    if (kind == SwatchKind::Gradient)
        drawSwatchRect(dl, min, max, gradient, rounding);
    else
        dl->AddRectFilled(min, max, toU32(solid), rounding);
}

} // namespace ofxSwatches
