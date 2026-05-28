#include "SwatchListReorder.h"
#include "imgui_internal.h"

namespace ofxSwatches {

SwatchIndexDropResult swatchIndexDragDrop(const char* payloadTag,
                                          int index,
                                          const char* previewLabel,
                                          float rowMinY,
                                          float rowMaxY) {
    SwatchIndexDropResult result;
    result.target = index;

    const float h = rowMaxY - rowMinY;
    if (h < 1.0f) return result;

    ImGui::PushID(index);

    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
        ImGui::SetDragDropPayload(payloadTag, &index, sizeof(int));
        ImGui::TextUnformatted(previewLabel);
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadTag)) {
            IM_ASSERT(payload->DataSize == sizeof(int));
            int dragged = *(const int*)payload->Data;
            if (dragged != index) {
                const float my = ImGui::GetMousePos().y;
                const float mid = rowMinY + h * 0.5f;
                result.accepted = true;
                result.dragged = dragged;
                result.zone = (my < mid) ? SwatchDropZone::Before : SwatchDropZone::After;
            }
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::PopID();
    return result;
}

} // namespace ofxSwatches
