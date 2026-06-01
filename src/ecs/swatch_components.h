#pragma once

#include "SwatchLibrary.h"
#include <entt/entt.hpp>
#include <algorithm>
#include <string>
#include <vector>

// ============================================================================
// ECS COLOUR COMPONENTS
// Wraps ofxSwatches data types as EnTT components.
// Requires ofxEnTTKit (for entt::entity / entt::registry).
// ============================================================================

namespace ecs {

/// One ECS entity per palette. Individual swatches live in `colors[]`.
/// Inherits the full SwatchLibrary API (addColor, removeColor, findSameGreyValue, etc.)
/// so it works directly with SwatchesPanel and any ofxSwatches utility.
struct swatch_library_component : public ofxSwatches::SwatchLibrary {
    using ofxSwatches::SwatchLibrary::SwatchLibrary;
};

struct GradientStop {
    float                    position = 0.0f;
    ofxSwatches::SwatchColor color;

    GradientStop() = default;
    GradientStop(float pos, const ofxSwatches::SwatchColor& col) : position(pos), color(col) {}
    GradientStop(float pos, const ofColor& col)                  : position(pos), color(col) {}
};

struct color_gradient_component {
    std::string              name = "Untitled Gradient";
    std::vector<GradientStop> stops;

    color_gradient_component() {
        stops.push_back(GradientStop(0.0f, ofColor::black));
        stops.push_back(GradientStop(1.0f, ofColor::white));
    }

    explicit color_gradient_component(const std::string& n) : name(n) {
        stops.push_back(GradientStop(0.0f, ofColor::black));
        stops.push_back(GradientStop(1.0f, ofColor::white));
    }

    ofColor getColorAt(float t) const {
        if (stops.empty()) return ofColor::white;
        if (stops.size() == 1) return stops[0].color.color;

        t = ofClamp(t, 0.0f, 1.0f);
        for (size_t i = 0; i < stops.size() - 1; i++) {
            if (t >= stops[i].position && t <= stops[i + 1].position) {
                float range = stops[i + 1].position - stops[i].position;
                if (range < 0.0001f) return stops[i].color.color;
                float localT = (t - stops[i].position) / range;
                return stops[i].color.color.getLerped(stops[i + 1].color.color, localT);
            }
        }
        return stops.back().color.color;
    }

    void addStop(float position, const ofxSwatches::SwatchColor& color) {
        stops.push_back(GradientStop(position, color));
        sortStops();
    }

    void addStop(float position, const ofColor& color) {
        stops.push_back(GradientStop(position, color));
        sortStops();
    }

    void removeStop(int index) {
        if (stops.size() > 2 && index >= 0 && index < (int)stops.size()) {
            stops.erase(stops.begin() + index);
        }
    }

    void sortStops() {
        std::sort(stops.begin(), stops.end(),
            [](const GradientStop& a, const GradientStop& b) {
                return a.position < b.position;
            });
    }

    void reverse() {
        for (auto& stop : stops) {
            stop.position = 1.0f - stop.position;
        }
        std::reverse(stops.begin(), stops.end());
    }

    int count() const { return (int)stops.size(); }

    static color_gradient_component fromColors(const ofColor& start, const ofColor& end,
                                               const std::string& name = "") {
        color_gradient_component g(name);
        g.stops.clear();
        g.stops.push_back(GradientStop(0.0f, start));
        g.stops.push_back(GradientStop(1.0f, end));
        return g;
    }

    static color_gradient_component fromLibrary(const swatch_library_component& library,
                                                const std::string& name = "") {
        color_gradient_component g(name.empty() ? library.libraryName + " Gradient" : name);
        g.stops.clear();
        int n = library.count();
        if (n == 0) return g;
        if (n == 1) {
            g.stops.push_back(GradientStop(0.0f, library.colors[0]));
            g.stops.push_back(GradientStop(1.0f, library.colors[0]));
        } else {
            for (int i = 0; i < n; i++) {
                float pos = (float)i / (n - 1);
                g.stops.push_back(GradientStop(pos, library.colors[i]));
            }
        }
        return g;
    }
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
