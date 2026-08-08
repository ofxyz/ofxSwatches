#include "DualColorChips.h"
#include "SwatchDraw.h"

#include "IconsFontAwesome5.h"
#include "imgui.h"

#include <algorithm>
#include <cfloat>

namespace ofxSwatches {
namespace {

ImU32 contrastingBorder(const ofColor& c) {
    const float lum = (0.299f * c.r + 0.587f * c.g + 0.114f * c.b) / 255.f;
    return lum > 0.55f ? IM_COL32(40, 40, 45, 255) : IM_COL32(230, 230, 235, 255);
}

void drawChip(ImDrawList* dl, ImVec2 min, ImVec2 max, const DualColorTarget& target,
              ColorSlot slot, bool active, const char* label) {
    const ofColor color = target.preview(slot);
    drawSwatchRect(dl, min, max, target.kind(slot), target.color(slot),
                   target.gradient(slot), 2.f);
    // Checker for transparent / low alpha solids.
    if (target.kind(slot) == SwatchKind::Solid && color.a < 250) {
        const ImU32 a = IM_COL32(180, 180, 180, 90);
        const ImU32 b = IM_COL32(90, 90, 90, 90);
        const float s = 4.f;
        for (float y = min.y; y < max.y; y += s) {
            for (float x = min.x; x < max.x; x += s) {
                const int ix = (int)((x - min.x) / s);
                const int iy = (int)((y - min.y) / s);
                dl->AddRectFilled(
                    ImVec2(x, y),
                    ImVec2(std::min(x + s, max.x), std::min(y + s, max.y)),
                    ((ix + iy) & 1) ? a : b);
            }
        }
        drawSwatchRect(dl, min, max, target.kind(slot), target.color(slot),
                       target.gradient(slot), 2.f);
    }
    const ImU32 border = active ? IM_COL32(70, 160, 255, 255) : contrastingBorder(color);
    dl->AddRect(min, max, border, 2.f, 0, active ? 2.5f : 1.25f);
    if (label && ImGui::IsMouseHoveringRect(min, max)) {
        if (target.kind(slot) == SwatchKind::Gradient)
            ImGui::SetTooltip("%s (gradient)", label);
        else
            ImGui::SetTooltip("%s", label);
    }
}

bool chipClicked(ImVec2 min, ImVec2 max, ImGuiMouseButton button = 0) {
    return ImGui::IsMouseHoveringRect(min, max)
        && ImGui::IsMouseClicked(button);
}

bool chipDoubleClicked(ImVec2 min, ImVec2 max) {
    return ImGui::IsMouseHoveringRect(min, max)
        && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
}

} // namespace

void drawDualColorChips(DualColorTarget& target, ImVec2 size,
                        const DualColorChipsOpts& opts) {
    ImGui::PushID("##dualColorChips");

    const float w = std::max(18.f, size.x);
    const float h = std::max(18.f, size.y);
    ImGui::InvisibleButton("##dualHit", ImVec2(w, h));
    const bool hovered = ImGui::IsItemHovered();
    const ImVec2 origin = ImGui::GetItemRectMin();

    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Secondary (back) bottom-right; Primary (front) top-left — AI / PS layout.
    const float chip = std::min(w, h) * 0.62f;
    const float inset = std::max(1.f, chip * 0.08f);
    const ImVec2 backMin(origin.x + w - chip - inset, origin.y + h - chip - inset);
    const ImVec2 backMax(backMin.x + chip, backMin.y + chip);
    const ImVec2 frontMin(origin.x + inset, origin.y + inset);
    const ImVec2 frontMax(frontMin.x + chip, frontMin.y + chip);

    const bool primaryActive = target.active == ColorSlot::Primary;
    drawChip(dl, backMin, backMax, target, ColorSlot::Secondary, !primaryActive,
             target.secondaryLabel);
    drawChip(dl, frontMin, frontMax, target, ColorSlot::Primary, primaryActive,
             target.primaryLabel);

    // Tiny swap affordance top-right (scaled glyph so it stays inside the disc).
    const float swapR = std::max(5.f, chip * 0.18f);
    const ImVec2 swapC(origin.x + w - swapR - 1.f, origin.y + swapR + 1.f);
    dl->AddCircleFilled(swapC, swapR, IM_COL32(45, 48, 55, 220), 12);
    dl->AddCircle(swapC, swapR, IM_COL32(180, 185, 195, 255), 12, 1.f);
    {
        ImFont* font = ImGui::GetFont();
        const float glyphSz = swapR * 1.35f;
        const ImVec2 ts = font->CalcTextSizeA(glyphSz, FLT_MAX, 0.f, ICON_FA_EXCHANGE_ALT);
        dl->AddText(font, glyphSz,
                    ImVec2(swapC.x - ts.x * 0.5f, swapC.y - ts.y * 0.5f),
                    IM_COL32(220, 220, 225, 255), ICON_FA_EXCHANGE_ALT);
    }
    const ImVec2 swapMin(swapC.x - swapR, swapC.y - swapR);
    const ImVec2 swapMax(swapC.x + swapR, swapC.y + swapR);
    if (ImGui::IsMouseHoveringRect(swapMin, swapMax)) {
        ImGui::SetTooltip("Swap %s / %s", target.primaryLabel, target.secondaryLabel);
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            target.swapColors();
    } else if (chipClicked(frontMin, frontMax)) {
        target.setActive(ColorSlot::Primary);
    } else if (chipClicked(backMin, backMax)) {
        target.setActive(ColorSlot::Secondary);
    }

    if (opts.enablePicker) {
        static ColorSlot s_pickerSlot = ColorSlot::Primary;
        if (chipDoubleClicked(frontMin, frontMax)) {
            target.setActive(ColorSlot::Primary);
            s_pickerSlot = ColorSlot::Primary;
            ImGui::OpenPopup("##dualChipPicker");
        } else if (chipDoubleClicked(backMin, backMax)) {
            target.setActive(ColorSlot::Secondary);
            s_pickerSlot = ColorSlot::Secondary;
            ImGui::OpenPopup("##dualChipPicker");
        }

        if (ImGui::BeginPopup("##dualChipPicker")) {
            ImGui::TextUnformatted(target.label(s_pickerSlot));
            if (target.kind(s_pickerSlot) == SwatchKind::Gradient)
                ImGui::TextDisabled("Editing RGB converts this slot to a solid.");
            ofColor& c = target.color(s_pickerSlot);
            float col[4] = {c.r / 255.f, c.g / 255.f, c.b / 255.f, c.a / 255.f};
            if (ImGui::ColorPicker4("##pick", col,
                    ImGuiColorEditFlags_AlphaBar
                        | ImGuiColorEditFlags_DisplayRGB
                        | ImGuiColorEditFlags_InputRGB)) {
                c.set(col[0] * 255.f, col[1] * 255.f, col[2] * 255.f, col[3] * 255.f);
                target.assignSolid(s_pickerSlot, c);
                target.setActive(s_pickerSlot);
            }
            ImGui::EndPopup();
        }
    }

    if (opts.onOpenOptions && hovered
        && ImGui::IsMouseClicked(ImGuiMouseButton_Right)
        && !ImGui::IsMouseHoveringRect(swapMin, swapMax)) {
        opts.onOpenOptions();
    }

    ImGui::PopID();
}

} // namespace ofxSwatches
