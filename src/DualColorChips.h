#pragma once

#include "DualColorTarget.h"

#include "imgui.h"

#include <functional>

namespace ofxSwatches {

struct DualColorChipsOpts {
    /// Double-click a chip to open an ImGui colour picker popup.
    bool enablePicker {true};
    /// Right-click the control (e.g. stroke width popup).
    std::function<void()> onOpenOptions;
};

/// Classic stacked dual swatches (front = Primary, back = Secondary).
/// Click selects the active slot; double-click opens a picker when enabled.
void drawDualColorChips(DualColorTarget& target, ImVec2 size,
                        const DualColorChipsOpts& opts = {});

} // namespace ofxSwatches
