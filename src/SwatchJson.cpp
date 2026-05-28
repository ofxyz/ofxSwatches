#include "SwatchJson.h"

namespace ofxSwatches {

void to_json(ofJson& j, const SwatchColor& color) {
    j = ofJson{
        {"name", color.name},
        {"type", static_cast<int>(color.type)},
        {"spot", color.isSpotColor},
        {"spotInk", color.spotInkName},
        {"r", color.color.r},
        {"g", color.color.g},
        {"b", color.color.b},
        {"a", color.color.a},
    };
    if (color.type == SwatchColorType::CMYK) {
        j["c"] = color.cmyk100.r;
        j["m"] = color.cmyk100.g;
        j["y"] = color.cmyk100.b;
        j["k"] = color.cmyk100.a;
    }
}

void from_json(const ofJson& j, SwatchColor& color) {
    try {
        color.name = j.value("name", "");
        color.isSpotColor = j.value("spot", false);
        color.spotInkName = j.value("spotInk", "");

        const int typeInt = j.value("type", 0);
        const auto type = static_cast<SwatchColorType>(typeInt);

        if (type == SwatchColorType::CMYK && j.contains("c")) {
            const float c = j.value("c", 0.0f);
            const float m = j.value("m", 0.0f);
            const float y = j.value("y", 0.0f);
            const float k = j.value("k", 0.0f);
            color = SwatchColor::fromCMYK(c, m, y, k, color.name);
            color.isSpotColor = j.value("spot", false);
            color.spotInkName = j.value("spotInk", "");
            return;
        }

        color.type = SwatchColorType::RGB;
        color.color.r = j.value("r", 255);
        color.color.g = j.value("g", 255);
        color.color.b = j.value("b", 255);
        color.color.a = j.value("a", 255);
    } catch (const ofJson::exception& e) {
        ofLogWarning("ofxSwatches") << "from_json: " << e.what();
    }
}

ofJson libraryToJson(const SwatchLibrary& library) {
    ofJson settings;
    settings["libName"] = library.libraryName;
    settings["version"] = 2;
    settings["richBlack"] = {
        {"c", library.richBlack.c},
        {"m", library.richBlack.m},
        {"y", library.richBlack.y},
        {"k", library.richBlack.k},
    };
    settings["Swatches"] = ofJson::array();
    for (const auto& c : library.colors) {
        settings["Swatches"].push_back(c);
    }
    return settings;
}

bool libraryFromJson(SwatchLibrary& library, const ofJson& j) {
    if (!j.is_object()) return false;

    library.libraryName = j.value("libName", "Untitled Library");
    if (j.contains("richBlack") && j["richBlack"].is_object()) {
        const auto& rb = j["richBlack"];
        library.richBlack.c = rb.value("c", 60.0f);
        library.richBlack.m = rb.value("m", 40.0f);
        library.richBlack.y = rb.value("y", 40.0f);
        library.richBlack.k = rb.value("k", 100.0f);
    }

    library.colors.clear();
    if (j.contains("Swatches") && j["Swatches"].is_array()) {
        for (const auto& item : j["Swatches"]) {
            SwatchColor c;
            from_json(item, c);
            library.colors.push_back(c);
        }
    }
    return true;
}

bool saveLibrary(const SwatchLibrary& library, const std::string& path) {
    std::string outPath = path;
    if (outPath.empty()) {
        outPath = ofToDataPath(library.libraryName + ".json");
    }
    return ofSavePrettyJson(outPath, libraryToJson(library));
}

bool loadLibrary(SwatchLibrary& library, const std::string& path) {
    std::string inPath = path;
    if (inPath.empty()) {
        inPath = ofToDataPath(library.libraryName + ".json");
    }
    if (!ofFile::doesFileExist(inPath)) {
        ofLogVerbose("ofxSwatches") << "loadLibrary: no file at " << inPath << " (first run?)";
        return false;
    }
    return libraryFromJson(library, ofLoadJson(inPath));
}

} // namespace ofxSwatches
