#pragma once

#include <entt/entt.hpp>
#include <functional>

namespace ofxSwatches {

/// Optional Document Paints section for SwatchesPanel / standalone use.
/// Requires ofxDocumentKit document paints on the registry.
class DocumentPaintsPanel {
public:
    void setup(entt::registry* registry);
    void setOnChanged(std::function<void()> cb) { m_onChanged = std::move(cb); }

    /// Draw library contents (no window). Call from SwatchesPanel or a host panel.
    void drawContent();

    /// Standalone ImGui window.
    void draw(const char* title, bool* visible, int extraWindowFlags = 0);

    entt::registry* registry() const { return m_registry; }

private:
    entt::registry* m_registry = nullptr;
    std::function<void()> m_onChanged;
};

} // namespace ofxSwatches
