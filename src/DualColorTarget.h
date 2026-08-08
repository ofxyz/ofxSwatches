#pragma once

#include "SwatchColor.h"
#include "ofMain.h"

#include <utility>

namespace ofxSwatches {

/// Which of the two colour slots receives swatch / picker edits.
enum class ColorSlot { Primary, Secondary };

/// One Fill/Stroke (or FG/BG) slot: solid or portable gradient swatch.
struct PaintSlot {
    SwatchKind  kind = SwatchKind::Solid;
    ofColor     solid {255, 255, 255, 255};
    SwatchColor gradientSwatch;

    ofColor preview() const
    {
        if (kind == SwatchKind::Gradient)
            return gradientSwatch.previewColor();
        return solid;
    }

    void assignSolid(const ofColor& c)
    {
        kind  = SwatchKind::Solid;
        solid = c;
        gradientSwatch = SwatchColor{};
    }

    void assign(const SwatchColor& s)
    {
        if (s.isGradient()) {
            kind           = SwatchKind::Gradient;
            gradientSwatch = s;
            solid          = s.previewColor();
        } else {
            assignSolid(s.previewColor());
        }
    }

    bool isGradient() const { return kind == SwatchKind::Gradient; }
};

/// Generic dual colour target (Illustrator Fill/Stroke or Photoshop FG/BG).
/// `primary` / `secondary` hold the solid (or gradient preview) ofColor used by
/// ColorEdit and legacy toolbar code; kind + gradientSwatch track full paints.
struct DualColorTarget {
    ofColor primary {255, 255, 255, 255};
    ofColor secondary {0, 0, 0, 255};
    SwatchKind primaryKind {SwatchKind::Solid};
    SwatchKind secondaryKind {SwatchKind::Solid};
    SwatchColor primaryGradient;
    SwatchColor secondaryGradient;

    ColorSlot active {ColorSlot::Primary};
    const char* primaryLabel {"Primary"};
    const char* secondaryLabel {"Secondary"};

    DualColorTarget() = default;
    DualColorTarget(const ofColor& prim, const ofColor& sec,
                    ColorSlot act = ColorSlot::Primary)
        : primary(prim), secondary(sec), active(act) {}

    ofColor& color(ColorSlot slot)
    {
        return slot == ColorSlot::Primary ? primary : secondary;
    }
    const ofColor& color(ColorSlot slot) const
    {
        return slot == ColorSlot::Primary ? primary : secondary;
    }

    SwatchKind& kind(ColorSlot slot)
    {
        return slot == ColorSlot::Primary ? primaryKind : secondaryKind;
    }
    const SwatchKind& kind(ColorSlot slot) const
    {
        return slot == ColorSlot::Primary ? primaryKind : secondaryKind;
    }

    SwatchColor& gradient(ColorSlot slot)
    {
        return slot == ColorSlot::Primary ? primaryGradient : secondaryGradient;
    }
    const SwatchColor& gradient(ColorSlot slot) const
    {
        return slot == ColorSlot::Primary ? primaryGradient : secondaryGradient;
    }

    ofColor preview(ColorSlot slot) const
    {
        if (kind(slot) == SwatchKind::Gradient)
            return gradient(slot).previewColor();
        return color(slot);
    }

    ofColor& activeColor() { return color(active); }
    const ofColor& activeColor() const { return color(active); }
    ofColor activePreview() const { return preview(active); }

    void setActive(ColorSlot slot) { active = slot; }

    void toggleActive()
    {
        active = (active == ColorSlot::Primary) ? ColorSlot::Secondary
                                                : ColorSlot::Primary;
    }

    void swapColors()
    {
        std::swap(primary, secondary);
        std::swap(primaryKind, secondaryKind);
        std::swap(primaryGradient, secondaryGradient);
    }

    void assignSolid(ColorSlot slot, const ofColor& c)
    {
        color(slot) = c;
        kind(slot)  = SwatchKind::Solid;
        gradient(slot) = SwatchColor{};
    }

    void assignSwatch(ColorSlot slot, const SwatchColor& s)
    {
        if (s.isGradient()) {
            kind(slot)     = SwatchKind::Gradient;
            gradient(slot) = s;
            color(slot)    = s.previewColor();
        } else {
            assignSolid(slot, s.previewColor());
        }
    }

    void assignActiveSwatch(const SwatchColor& s) { assignSwatch(active, s); }

    /// After ColorEdit mutates ofColor, call to break a gradient link.
    void solidifyIfEdited(ColorSlot slot, const ofColor& previousPreview)
    {
        if (kind(slot) != SwatchKind::Gradient)
            return;
        if (color(slot) != previousPreview)
            assignSolid(slot, color(slot));
    }

    SwatchColor activeAsSwatch() const
    {
        if (kind(active) == SwatchKind::Gradient)
            return gradient(active);
        return SwatchColor(color(active));
    }

    const char* activeLabel() const
    {
        return active == ColorSlot::Primary ? primaryLabel : secondaryLabel;
    }

    const char* label(ColorSlot slot) const
    {
        return slot == ColorSlot::Primary ? primaryLabel : secondaryLabel;
    }
};

} // namespace ofxSwatches
