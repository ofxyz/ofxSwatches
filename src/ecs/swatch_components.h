#pragma once

#include "SwatchLibrary.h"
#include <entt/entt.hpp>
#include <string>

// ============================================================================
// ECS COLOUR COMPONENTS
// Wraps ofxSwatches data types as EnTT components.
// Requires ofxEnTTKit (for entt::entity / entt::registry).
//
// Gradients are NOT defined here. The canonical gradient/paint model lives in
// ofxKit (ecs::gradient_component, ecs::solid_color_component, …). ofxSwatches
// owns palettes; ofxKit owns paints.
// ============================================================================

namespace ecs {

/// One ECS entity per palette. Individual swatches live in `colors[]`.
/// Inherits the full SwatchLibrary API (addColor, removeColor, findSameGreyValue, etc.)
/// so it works directly with SwatchesPanel and any ofxSwatches utility.
struct swatch_library_component : public ofxSwatches::SwatchLibrary {
    using ofxSwatches::SwatchLibrary::SwatchLibrary;
};

/// Bind a layer/path entity to a specific swatch in a palette entity.
///
/// Pattern:
///   1. Create a swatch_library_component entity (palette).
///   2. Attach this to the layer/path group entity.
///   3. Resolve stroke/fill ofColor via colorIndex or colorName.
struct swatch_palette_ref_component {
    entt::entity library    = entt::null;
    int          colorIndex = -1;
    std::string  colorName;
};

} // namespace ecs
