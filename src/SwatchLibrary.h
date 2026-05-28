#pragma once

#include "SwatchColor.h"
#include <algorithm>
#include <string>
#include <vector>

namespace ofxSwatches {

struct SwatchLibrary {
    std::string libraryName = "Untitled Library";
    std::vector<SwatchColor> colors;
    RichBlackPreset richBlack;

    SwatchLibrary() = default;
    explicit SwatchLibrary(const std::string& name) : libraryName(name) {}

    void addColor(const SwatchColor& color) { colors.push_back(color); }

    void addColor(const ofColor& color, const std::string& name = "") {
        colors.push_back(SwatchColor(color, name));
    }

    void addCMYK(float c, float m, float y, float k, const std::string& name = "") {
        colors.push_back(SwatchColor::fromCMYK(c, m, y, k, name));
    }

    void removeColor(int index) {
        if (index >= 0 && index < (int)colors.size()) {
            colors.erase(colors.begin() + index);
        }
    }

    void removeColor(const std::string& name) {
        colors.erase(
            std::remove_if(colors.begin(), colors.end(),
                [&name](const SwatchColor& c) { return c.name == name; }),
            colors.end());
    }

    SwatchColor* getColor(int index) {
        if (index >= 0 && index < (int)colors.size()) return &colors[index];
        return nullptr;
    }

    const SwatchColor* getColor(int index) const {
        if (index >= 0 && index < (int)colors.size()) return &colors[index];
        return nullptr;
    }

    SwatchColor* getByName(const std::string& name) {
        for (auto& c : colors) {
            if (c.name == name) return &c;
        }
        return nullptr;
    }

    const SwatchColor* getByName(const std::string& name) const {
        for (const auto& c : colors) {
            if (c.name == name) return &c;
        }
        return nullptr;
    }

    bool hasColor(const std::string& name) const { return getByName(name) != nullptr; }

    int count() const { return (int)colors.size(); }
    bool empty() const { return colors.empty(); }
    void clear() { colors.clear(); }

    void generateHarmonyFrom(int sourceIndex, ColorHarmony harmony) {
        if (sourceIndex < 0 || sourceIndex >= (int)colors.size()) return;
        for (auto& c : colors[sourceIndex].generateHarmony(harmony)) {
            colors.push_back(c);
        }
    }

    std::vector<int> findSameGreyValue(float percent, float tolerance = 1.0f) const;

    void reorderColor(int fromIndex, int toIndex);
};

} // namespace ofxSwatches
