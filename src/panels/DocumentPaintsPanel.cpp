#include "DocumentPaintsPanel.h"

#include "DocumentPaints.h"
#include "DocumentMetadata.h"
#include "components/paint_components.h"

#include "imgui.h"

namespace ofxSwatches {

void DocumentPaintsPanel::setup(entt::registry* registry)
{
    m_registry = registry;
}

void DocumentPaintsPanel::drawContent()
{
    if (!m_registry) {
        ImGui::TextDisabled("No document registry");
        return;
    }

    auto& reg = *m_registry;
    ofxDocumentKit::pruneDocumentPaints(reg);
    auto& lib = ofxDocumentKit::getOrCreateDocumentPaints(reg);

    if (ImGui::Button("New solid##docPaintsPanel")) {
        ofxDocumentKit::createDocumentSolidPaint(reg, glm::vec4(1.f));
        if (m_onChanged) m_onChanged();
    }
    ImGui::SameLine();
    if (ImGui::Button("New gradient##docPaintsPanel")) {
        ofxDocumentKit::createDocumentGradientPaint(reg, "Gradient");
        if (m_onChanged) m_onChanged();
    }

    ImGui::Separator();
    if (lib.paints.empty()) {
        ImGui::TextDisabled("No document paints — add solids/gradients or promote a swatch");
        return;
    }

    for (entt::entity pe : lib.paints) {
        if (!reg.valid(pe)) continue;
        ImGui::PushID(static_cast<int>(pe));
        if (const auto* sc = reg.try_get<ecs::solid_color_component>(pe)) {
            ImGui::ColorButton("##c", ImVec4(sc->color.x, sc->color.y, sc->color.z, sc->color.w));
            ImGui::SameLine();
            ImGui::TextUnformatted("Solid");
        } else if (const auto* g = reg.try_get<ecs::gradient_component>(pe)) {
            ImGui::Text("%s", g->name.empty() ? "Gradient" : g->name.c_str());
        } else {
            ImGui::TextUnformatted("Paint");
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Remove")) {
            ofxDocumentKit::removeDocumentPaint(reg, pe);
            if (m_onChanged) m_onChanged();
            ImGui::PopID();
            break;
        }
        ImGui::PopID();
    }
}

void DocumentPaintsPanel::draw(const char* title, bool* visible, int extraWindowFlags)
{
    if (visible && !*visible) return;
    if (!ImGui::Begin(title, visible, extraWindowFlags)) {
        ImGui::End();
        return;
    }
    drawContent();
    ImGui::End();
}

} // namespace ofxSwatches
