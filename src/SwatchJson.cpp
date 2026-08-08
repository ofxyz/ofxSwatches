#include "SwatchJson.h"

namespace ofxSwatches {
namespace {

ofJson stopToJson(const SwatchGradientStop& s)
{
    return ofJson{
        {"position", s.position},
        {"color", {s.color.x, s.color.y, s.color.z, s.color.w}},
        {"intensity", s.intensity},
    };
}

SwatchGradientStop stopFromJson(const ofJson& j)
{
    SwatchGradientStop s;
    s.position  = j.value("position", 0.f);
    s.intensity = j.value("intensity", 1.f);
    if (j.contains("color") && j["color"].is_array() && j["color"].size() >= 3) {
        const auto& c = j["color"];
        s.color.x = c[0].get<float>();
        s.color.y = c[1].get<float>();
        s.color.z = c[2].get<float>();
        s.color.w = c.size() >= 4 ? c[3].get<float>() : 1.f;
    }
    return s;
}

void gradientToJson(ofJson& j, const SwatchGradient& g)
{
    j["gradType"]   = static_cast<int>(g.type);
    j["interp"]     = static_cast<int>(g.interp);
    j["spread"]     = static_cast<int>(g.spread);
    j["angle"]      = g.angle;
    j["center"]     = {g.center.x, g.center.y};
    j["innerRadius"] = g.innerRadius;
    j["outerRadius"] = g.outerRadius;
    j["numSteps"]   = g.numSteps;
    ofJson stops = ofJson::array();
    for (const auto& s : g.stops)
        stops.push_back(stopToJson(s));
    j["stops"] = std::move(stops);
}

void gradientFromJson(SwatchGradient& g, const ofJson& j)
{
    g.type   = static_cast<SwatchGradientType>(j.value("gradType", 0));
    g.interp = static_cast<SwatchGradientInterpolation>(j.value("interp", 0));
    g.spread = static_cast<SwatchGradientSpread>(j.value("spread", 0));
    g.angle       = j.value("angle", 90.f);
    g.innerRadius = j.value("innerRadius", 0.f);
    g.outerRadius = j.value("outerRadius", 1.f);
    g.numSteps    = j.value("numSteps", 0);
    if (j.contains("center") && j["center"].is_array() && j["center"].size() >= 2) {
        g.center.x = j["center"][0].get<float>();
        g.center.y = j["center"][1].get<float>();
    }
    g.stops.clear();
    if (j.contains("stops") && j["stops"].is_array()) {
        for (const auto& sj : j["stops"])
            g.stops.push_back(stopFromJson(sj));
    }
    if (g.stops.empty())
        g = SwatchGradient{};
    g.sortStops();
}

} // namespace

void to_json(ofJson& j, const SwatchColor& color) {
    j = ofJson{
        {"name", color.name},
        {"kind", color.kind == SwatchKind::Gradient ? "gradient" : "solid"},
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
    if (color.kind == SwatchKind::Gradient)
        gradientToJson(j, color.gradient);
}

void from_json(const ofJson& j, SwatchColor& color) {
    try {
        color = SwatchColor{};
        color.name = j.value("name", "");
        color.isSpotColor = j.value("spot", false);
        color.spotInkName = j.value("spotInk", "");

        const std::string kindStr = j.value("kind", "solid");
        color.kind = (kindStr == "gradient") ? SwatchKind::Gradient : SwatchKind::Solid;

        if (color.kind == SwatchKind::Gradient) {
            color.type = SwatchColorType::RGB;
            gradientFromJson(color.gradient, j);
            color.refreshPreviewFromGradient();
            // Allow explicit preview RGBA override if present
            if (j.contains("r")) {
                color.color.r = j.value("r", color.color.r);
                color.color.g = j.value("g", color.color.g);
                color.color.b = j.value("b", color.color.b);
                color.color.a = j.value("a", color.color.a);
            }
            return;
        }

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
    settings["version"] = 3;
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
    ofJson j = ofLoadJson(inPath);
    if (j.is_null() || j.empty()) return false;
    return libraryFromJson(library, j);
}

} // namespace ofxSwatches
