#include "SwatchDocumentBridge.h"
#include "SwatchPaintConvert.h"

#include "components/paint_components.h"

namespace ofxSwatches {

entt::entity ensureDocumentPaintFromSwatch(entt::registry& reg, const SwatchColor& swatch)
{
    if (swatch.kind == SwatchKind::Solid) {
        return ofxDocumentKit::ensureDocumentPaintFromSwatch(reg, swatch.previewColor(),
                                                             swatch.name);
    }

    const ecs::gradient_component want = toGradientComponent(swatch);
    auto& lib = ofxDocumentKit::getOrCreateDocumentPaints(reg);
    for (entt::entity paint : lib.paints) {
        if (!reg.valid(paint))
            continue;
        const auto* g = reg.try_get<ecs::gradient_component>(paint);
        if (!g)
            continue;
        if (!swatch.name.empty() && g->name != swatch.name)
            continue;
        if (gradientsEquivalent(*g, want))
            return paint;
    }

    // Name match alone is not enough if stops differ — create a new paint.
    for (entt::entity paint : lib.paints) {
        if (!reg.valid(paint))
            continue;
        const auto* g = reg.try_get<ecs::gradient_component>(paint);
        if (!g)
            continue;
        if (gradientsEquivalent(*g, want))
            return paint;
    }

    const entt::entity e =
        ofxDocumentKit::createDocumentGradientPaint(reg, want.name);
    if (e != entt::null && reg.valid(e))
        reg.replace<ecs::gradient_component>(e, want);
    return e;
}

bool documentPaintToSwatch(const entt::registry& reg, entt::entity paint, SwatchColor& out)
{
    if (paint == entt::null || !reg.valid(paint))
        return false;
    if (const auto* g = reg.try_get<ecs::gradient_component>(paint)) {
        out = fromGradientComponent(*g);
        return true;
    }
    if (const auto* s = reg.try_get<ecs::solid_color_component>(paint)) {
        out = SwatchColor(ofFloatColor(s->color.x, s->color.y, s->color.z, s->color.w));
        return true;
    }
    return false;
}

} // namespace ofxSwatches
