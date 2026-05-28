#pragma once

#include "ofMain.h"
#include <vector>

namespace ofxSwatches {

/// WCAG-style sRGB relative luminance, 0–100%.
float getGreyValuePercent(const ofColor& c);

/// Fast perceived grey: 0.299R + 0.587G + 0.114B on 0–255, scaled 0–100%.
float getGreyValueSimple(const ofColor& c);

/// True when R ≈ G ≈ B (neutral grey).
bool isNeutralGrey(const ofColor& c, float channelEpsilon = 2.0f);

/// Desaturated R=G=B at the same grey value %.
ofColor toGreyEquivalent(const ofColor& c);

/// WCAG contrast ratio (1.0–21.0).
float contrastRatio(const ofColor& a, const ofColor& b);

/// Meets WCAG AA for normal text (4.5:1).
bool meetsContrastAA(float ratio);

/// Meets WCAG AAA for normal text (7:1).
bool meetsContrastAAA(float ratio);

/// Optional CIE Lab (L* 0–100, a*, b*).
glm::vec3 getLab(const ofColor& c);

} // namespace ofxSwatches
