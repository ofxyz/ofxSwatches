#include "SwatchPaintConvert.h"

#include <cmath>

namespace ofxSwatches {
namespace {

bool nearEq(float a, float b, float eps) { return std::fabs(a - b) <= eps; }

bool vec4Near(const glm::vec4& a, const glm::vec4& b, float eps)
{
    return nearEq(a.x, b.x, eps) && nearEq(a.y, b.y, eps)
        && nearEq(a.z, b.z, eps) && nearEq(a.w, b.w, eps);
}

bool vec2Near(const glm::vec2& a, const glm::vec2& b, float eps)
{
    return nearEq(a.x, b.x, eps) && nearEq(a.y, b.y, eps);
}

} // namespace

ecs::gradient_component toGradientComponent(const SwatchColor& swatch)
{
    ecs::gradient_component g(swatch.name.empty() ? "Gradient" : swatch.name);
    g.stops.clear();
    for (const auto& s : swatch.gradient.stops) {
        g.stops.emplace_back(s.position, s.color, s.intensity);
    }
    if (g.stops.empty()) {
        g.stops.emplace_back(0.f, glm::vec4(0.f, 0.f, 0.f, 1.f));
        g.stops.emplace_back(1.f, glm::vec4(1.f, 1.f, 1.f, 1.f));
    }
    g.type   = static_cast<ecs::GradientType>(swatch.gradient.type);
    g.interp = static_cast<ecs::GradientInterpolation>(swatch.gradient.interp);
    g.spread = static_cast<ecs::GradientSpread>(swatch.gradient.spread);
    g.angle       = swatch.gradient.angle;
    g.center      = swatch.gradient.center;
    g.innerRadius = swatch.gradient.innerRadius;
    g.outerRadius = swatch.gradient.outerRadius;
    g.numSteps    = swatch.gradient.numSteps;
    g.sortStops();
    return g;
}

SwatchColor fromGradientComponent(const ecs::gradient_component& g)
{
    SwatchColor out;
    out.kind = SwatchKind::Gradient;
    out.name = g.name;
    out.type = SwatchColorType::RGB;
    out.gradient.stops.clear();
    for (const auto& s : g.stops) {
        out.gradient.stops.emplace_back(s.position, s.color, s.intensity);
    }
    if (out.gradient.stops.empty())
        out.gradient = SwatchGradient{};
    out.gradient.type   = static_cast<SwatchGradientType>(g.type);
    out.gradient.interp = static_cast<SwatchGradientInterpolation>(g.interp);
    out.gradient.spread = static_cast<SwatchGradientSpread>(g.spread);
    out.gradient.angle       = g.angle;
    out.gradient.center      = g.center;
    out.gradient.innerRadius = g.innerRadius;
    out.gradient.outerRadius = g.outerRadius;
    out.gradient.numSteps    = g.numSteps;
    out.gradient.sortStops();
    out.refreshPreviewFromGradient();
    return out;
}

bool gradientsEquivalent(const SwatchGradient& a, const SwatchGradient& b, float eps)
{
    if (a.type != b.type || a.interp != b.interp || a.spread != b.spread)
        return false;
    if (a.numSteps != b.numSteps)
        return false;
    if (!nearEq(a.angle, b.angle, eps) || !vec2Near(a.center, b.center, eps)
        || !nearEq(a.innerRadius, b.innerRadius, eps)
        || !nearEq(a.outerRadius, b.outerRadius, eps))
        return false;
    if (a.stops.size() != b.stops.size())
        return false;
    for (size_t i = 0; i < a.stops.size(); ++i) {
        if (!nearEq(a.stops[i].position, b.stops[i].position, eps)
            || !vec4Near(a.stops[i].color, b.stops[i].color, eps)
            || !nearEq(a.stops[i].intensity, b.stops[i].intensity, eps))
            return false;
    }
    return true;
}

bool gradientsEquivalent(const ecs::gradient_component& a, const ecs::gradient_component& b,
                         float eps)
{
    return gradientsEquivalent(fromGradientComponent(a).gradient,
                               fromGradientComponent(b).gradient, eps);
}

} // namespace ofxSwatches
