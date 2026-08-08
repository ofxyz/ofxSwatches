#pragma once

#include "SwatchColor.h"
#include "imgui.h"

namespace ofxSwatches {

/// Draw a solid fill or horizontal gradient ramp into @p dl.
void drawSwatchRect(ImDrawList* dl, ImVec2 min, ImVec2 max, const SwatchColor& swatch,
                    float rounding = 0.f);

/// Convenience for DualColorTarget slots.
void drawSwatchRect(ImDrawList* dl, ImVec2 min, ImVec2 max, SwatchKind kind,
                    const ofColor& solid, const SwatchColor& gradient,
                    float rounding = 0.f);

} // namespace ofxSwatches
