#pragma once

#include "SwatchColor.h"
#include "components/paint_components.h"

namespace ofxSwatches {

/// Convert portable swatch gradient ↔ ECS paint model.
ecs::gradient_component toGradientComponent(const SwatchColor& swatch);
SwatchColor fromGradientComponent(const ecs::gradient_component& g);

/// True when two gradients match closely enough to reuse a document paint.
bool gradientsEquivalent(const SwatchGradient& a, const SwatchGradient& b, float eps = 1e-4f);
bool gradientsEquivalent(const ecs::gradient_component& a, const ecs::gradient_component& b,
                         float eps = 1e-4f);

} // namespace ofxSwatches
