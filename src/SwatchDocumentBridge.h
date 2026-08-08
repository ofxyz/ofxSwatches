#pragma once

#include "SwatchColor.h"
#include "DocumentPaints.h"
#include <entt.hpp>

namespace ofxSwatches {

/// Ensure a document paint entity matching @p swatch (solid dedupe or equivalent
/// gradient). Lives here to avoid ofxDocumentKit ↔ ofxSwatches cycles.
entt::entity ensureDocumentPaintFromSwatch(entt::registry& reg, const SwatchColor& swatch);

/// Convert a document paint entity into a portable swatch book entry.
bool documentPaintToSwatch(const entt::registry& reg, entt::entity paint, SwatchColor& out);

} // namespace ofxSwatches
