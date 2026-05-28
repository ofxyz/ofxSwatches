#pragma once

#include "SwatchLibrary.h"
#include "ofJson.h"

namespace ofxSwatches {

void to_json(ofJson& j, const SwatchColor& color);
void from_json(const ofJson& j, SwatchColor& color);

ofJson libraryToJson(const SwatchLibrary& library);
bool libraryFromJson(SwatchLibrary& library, const ofJson& j);

bool saveLibrary(const SwatchLibrary& library, const std::string& path = "");
bool loadLibrary(SwatchLibrary& library, const std::string& path = "");

} // namespace ofxSwatches
