#include "SwatchesPanel.h"
#include "SwatchListReorder.h"
#include "SwatchJson.h"
#include "ImFonts.h"
#include "IconsFontAwesome5.h"
#include "imgui.h"
#include <cmath>
#include <cstring>
#include <algorithm>

namespace ofxSwatches {

namespace {

constexpr ImGuiColorEditFlags kPickerFlags =
    ImGuiColorEditFlags_PickerHueWheel
    | ImGuiColorEditFlags_AlphaBar
    | ImGuiColorEditFlags_DisplayRGB
    | ImGuiColorEditFlags_DisplayHex
    | ImGuiColorEditFlags_InputRGB
    | ImGuiColorEditFlags_Float
    | ImGuiColorEditFlags_NoSidePreview;

std::string formatSwatchValues(const SwatchColor& c) {
    if (c.type == SwatchColorType::CMYK) {
        const glm::vec4 v = c.getCMYK();
        char buf[64];
        snprintf(buf, sizeof(buf), "C%.0f  M%.0f  Y%.0f  K%.0f", v.r, v.g, v.b, v.a);
        return buf;
    }
    char buf[48];
    snprintf(buf, sizeof(buf), "R%d  G%d  B%d", (int)c.color.r, (int)c.color.g, (int)c.color.b);
    return buf;
}

} // namespace

void SwatchesPanel::setup(SwatchLibrary* activeLibrary) {
    m_activeLib = activeLibrary;
}

void SwatchesPanel::setLibraryEnumerator(LibraryEnumerator enumerator) {
    m_enumerateLibraries = std::move(enumerator);
}

void SwatchesPanel::setOnSelectLibrary(std::function<void(entt::entity)> cb) {
    m_onSelectLibrary = std::move(cb);
}

void SwatchesPanel::setOnLibraryChanged(std::function<void()> cb) {
    m_onLibraryChanged = std::move(cb);
}

float SwatchesPanel::contentWidth() const {
    return std::max(120.f, ImGui::GetContentRegionAvail().x);
}

float SwatchesPanel::swatchFooterReserveHeight() const {
    float lines = 1.f; // "N colors in library"
    if (m_hasColorSelection && m_activeLib && m_selectedColorIndex >= 0
        && m_selectedColorIndex < m_activeLib->count()) {
        lines += 1.f;
        if (m_compareColorIndex >= 0 && m_compareColorIndex < m_activeLib->count()) {
            lines += 1.f;
        }
    } else {
        lines += 1.f; // "Click a swatch to select"
    }
    return lines * ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;
}

float SwatchesPanel::gridSwatchSize() const {
    constexpr float kGridSpacing = 2.f;
    const float w = contentWidth();
    const int cols = std::max(1, (int)((w + kGridSpacing) / (m_gridCellSize + kGridSpacing)));
    const float fit = (w - kGridSpacing * (cols - 1)) / (float)cols;
    return ofClamp(fit, 32.f, 96.f);
}

void SwatchesPanel::syncPickerRgbFromCmyk() {
    const SwatchColor c = SwatchColor::fromCMYK(
        m_newColorCMYK[0], m_newColorCMYK[1], m_newColorCMYK[2], m_newColorCMYK[3]);
    m_newColorRGB[0] = c.color.r / 255.f;
    m_newColorRGB[1] = c.color.g / 255.f;
    m_newColorRGB[2] = c.color.b / 255.f;
    m_newColorRGB[3] = c.color.a / 255.f;
}

void SwatchesPanel::drawCmykInputs(SwatchLibrary& /*lib*/) {
    static const char* kLabels[] = {"Cyan", "Magenta", "Yellow", "Key"};
    bool changed = false;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                         ImVec2(ImGui::GetStyle().ItemSpacing.x, 6.f));
    ImGui::SetNextItemWidth(-FLT_MIN);
    for (int i = 0; i < 4; i++) {
        ImGui::PushID(i);
        if (ImGui::SliderFloat(kLabels[i], &m_newColorCMYK[i], 0.f, 100.f, "%.0f%%")) {
            changed = true;
        }
        ImGui::PopID();
    }
    ImGui::PopStyleVar();

    if (changed) {
        syncPickerRgbFromCmyk();
    }

    const SwatchColor preview = SwatchColor::fromCMYK(
        m_newColorCMYK[0], m_newColorCMYK[1], m_newColorCMYK[2], m_newColorCMYK[3]);
    const ImVec4 prev(preview.color.r / 255.f, preview.color.g / 255.f, preview.color.b / 255.f,
                      preview.color.a / 255.f);
    ImGui::Spacing();
    ImGui::ColorButton("##cmykPreview", prev,
                         ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoTooltip,
                         ImVec2(-FLT_MIN, 36.f));
}

void SwatchesPanel::drawSwatchContextMenu(int index, SwatchLibrary& lib) {
    if (ImGui::MenuItem("Edit...")) {
        openColorPicker(true, index);
    }
    if (ImGui::MenuItem("Rename...")) {
        beginRenameSwatch(lib, index);
    }
    if (ImGui::MenuItem("Set as contrast compare")) {
        m_compareColorIndex = index;
    }
    if (ImGui::MenuItem("Duplicate")) {
        SwatchColor dup = lib.colors[index];
        dup.name = lib.colors[index].name + " Copy";
        lib.addColor(dup);
        if (m_onLibraryChanged) m_onLibraryChanged();
    }
    if (ImGui::MenuItem("Delete")) {
        lib.removeColor(index);
        if (m_selectedColorIndex == index) {
            m_hasColorSelection = false;
            m_selectedColorIndex = -1;
        } else if (m_selectedColorIndex > index) {
            m_selectedColorIndex--;
        }
        if (m_compareColorIndex == index) {
            m_compareColorIndex = -1;
        } else if (m_compareColorIndex > index) {
            m_compareColorIndex--;
        }
        if (m_renamingSwatchIndex == index) {
            m_renamingSwatchIndex = -1;
        } else if (m_renamingSwatchIndex > index) {
            m_renamingSwatchIndex--;
        }
        if (m_onLibraryChanged) {
            m_onLibraryChanged();
        }
    }
}

void SwatchesPanel::beginRenameSwatch(SwatchLibrary& lib, int index) {
    if (index < 0 || index >= lib.count()) {
        return;
    }
    m_renamingSwatchIndex = index;
    strncpy(m_renameBuf, lib.colors[index].name.c_str(), sizeof(m_renameBuf) - 1);
    m_renameBuf[sizeof(m_renameBuf) - 1] = '\0';
}

void SwatchesPanel::deleteSelectedSwatch(SwatchLibrary& lib) {
    if (!m_hasColorSelection || m_selectedColorIndex < 0 || m_selectedColorIndex >= lib.count()) {
        return;
    }
    const int idx = m_selectedColorIndex;
    lib.removeColor(idx);
    m_hasColorSelection = false;
    m_selectedColorIndex = -1;
    if (m_compareColorIndex == idx) {
        m_compareColorIndex = -1;
    } else if (m_compareColorIndex > idx) {
        m_compareColorIndex--;
    }
    if (m_onLibraryChanged) {
        m_onLibraryChanged();
    }
}

void SwatchesPanel::openColorPicker(bool editing, int index) {
    m_showColorEditor = true;
    m_editingExisting = editing;
    m_editingIndex = index;
    m_renamingSwatchIndex = -1;
    m_colorPickerSized = false;
    m_pickerCmykMode = false;

    if (editing && index >= 0 && m_activeLib && index < m_activeLib->count()) {
        const auto& c = m_activeLib->colors[index];
        m_newColorRGB[0] = c.color.r / 255.f;
        m_newColorRGB[1] = c.color.g / 255.f;
        m_newColorRGB[2] = c.color.b / 255.f;
        m_newColorRGB[3] = c.color.a / 255.f;
        glm::vec4 cmyk = c.getCMYK();
        m_newColorCMYK[0] = cmyk.r;
        m_newColorCMYK[1] = cmyk.g;
        m_newColorCMYK[2] = cmyk.b;
        m_newColorCMYK[3] = cmyk.a;
        m_pickerCmykMode = (c.type == SwatchColorType::CMYK);
        strncpy(m_newColorName, c.name.c_str(), sizeof(m_newColorName) - 1);
        m_newColorName[sizeof(m_newColorName) - 1] = '\0';
    } else {
        m_newColorRGB[0] = m_newColorRGB[1] = m_newColorRGB[2] = m_newColorRGB[3] = 1.f;
        m_newColorCMYK[0] = m_newColorCMYK[1] = m_newColorCMYK[2] = 0.f;
        m_newColorCMYK[3] = 0.f;
        m_newColorName[0] = '\0';
    }
}

void SwatchesPanel::drawContent() {
    if (!m_activeLib) {
        ImGui::TextDisabled("No active swatch library.");
        return;
    }

    drawLibraryToolbar();
    ImGui::Separator();

    if (m_hasColorSelection && m_selectedColorIndex >= 0 && m_selectedColorIndex < m_activeLib->count()) {
        drawSelectionTools(*m_activeLib);
        ImGui::Separator();
    }

    const float footerH = swatchFooterReserveHeight();
    ImGuiWindowFlags scrollFlags = ImGuiWindowFlags_None;
    if (ImGui::BeginChild("SwatchScroll", ImVec2(0, -footerH), false, scrollFlags)) {
        drawSwatchArea(*m_activeLib);
    }
    ImGui::EndChild();

    drawSelectionFooter(*m_activeLib);
    drawColorPickerModal(*m_activeLib);
}

void SwatchesPanel::draw(const char* title, bool* visible, int extraWindowFlags) {
    if (!visible || !*visible) return;

    if (m_firstWindowSize) {
        ImGui::SetNextWindowSize(ImVec2(kDefaultWindowWidth, kDefaultWindowHeight), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSizeConstraints(ImVec2(260, 280), ImVec2(800, 900));
        m_firstWindowSize = false;
    }

    ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar | extraWindowFlags;
    if (!ImGui::Begin(title, visible, flags)) {
        ImGui::End();
        return;
    }

    drawMenuBar();
    drawContent();
    ImGui::End();
}

void SwatchesPanel::drawMenuBar() {
    if (!ImGui::BeginMenuBar()) return;

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New Library...")) {
            ImGui::OpenPopup("New Library##Swatches");
        }
        if (ImGui::MenuItem("Import JSON...")) {
            SwatchLibrary imported("Imported");
            if (loadLibrary(imported, "")) {
                *m_activeLib = imported;
                if (m_onLibraryChanged) m_onLibraryChanged();
            }
        }
        if (ImGui::MenuItem("Export JSON...", nullptr, false, m_activeLib != nullptr)) {
            saveLibrary(*m_activeLib, "");
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("Add Color...", nullptr, false, m_activeLib != nullptr)) {
            openColorPicker(false);
        }
        if (ImGui::MenuItem("Delete Selected", nullptr, false,
                            m_activeLib != nullptr && m_hasColorSelection)) {
            deleteSelectedSwatch(*m_activeLib);
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Clear Library...", nullptr, false,
                            m_activeLib != nullptr && !m_activeLib->empty())) {
            m_confirmClearLibrary = true;
            ImGui::OpenPopup("Clear Library##Swatches");
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View")) {
        if (ImGui::MenuItem("Grid", nullptr, m_viewMode == SwatchViewMode::Grid)) {
            m_viewMode = SwatchViewMode::Grid;
        }
        if (ImGui::MenuItem("List", nullptr, m_viewMode == SwatchViewMode::List)) {
            m_viewMode = SwatchViewMode::List;
        }
        ImGui::Separator();
        if (m_viewMode == SwatchViewMode::Grid) {
            ImGui::SetNextItemWidth(140);
            ImGui::SliderFloat("Swatch size", &m_gridCellSize, 40.f, 88.f, "%.0f px");
        }
        ImGui::EndMenu();
    }

    ImGui::EndMenuBar();

    if (ImGui::BeginPopupModal("New Library##Swatches", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Name", m_newLibraryName, sizeof(m_newLibraryName));
        if (ImGui::Button("Create")) {
            m_activeLib->libraryName = m_newLibraryName;
            m_activeLib->clear();
            strcpy(m_newLibraryName, "New Library");
            ImGui::CloseCurrentPopup();
            if (m_onLibraryChanged) m_onLibraryChanged();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (m_confirmClearLibrary) {
        ImGui::OpenPopup("Clear Library##Swatches");
        m_confirmClearLibrary = false;
    }
    if (ImGui::BeginPopupModal("Clear Library##Swatches", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("Remove all swatches from this library?");
        if (ImGui::Button("Clear", ImVec2(80, 0)) && m_activeLib) {
            m_activeLib->clear();
            m_selectedColorIndex = -1;
            m_hasColorSelection = false;
            m_compareColorIndex = -1;
            if (m_onLibraryChanged) m_onLibraryChanged();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(80, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void SwatchesPanel::drawLibraryToolbar() {
    std::vector<entt::entity> entities;
    std::vector<std::string> names;
    int selectedCombo = -1;

    if (m_enumerateLibraries) {
        m_enumerateLibraries([&](entt::entity e, SwatchLibrary& lib) {
            entities.push_back(e);
            names.push_back(lib.libraryName);
            if (&lib == m_activeLib || e == m_activeLibraryEntity) {
                selectedCombo = (int)entities.size() - 1;
                m_activeLibraryEntity = e;
            }
        });
    } else if (m_activeLib) {
        names.push_back(m_activeLib->libraryName);
        selectedCombo = 0;
    }

    if (selectedCombo < 0 && !names.empty()) selectedCombo = 0;

    const float w = contentWidth();
    const bool narrow = w < 300.f;

    if (!narrow) {
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("Library");
        ImGui::SameLine();
    }

    if (!names.empty()) {
        std::vector<const char*> cnames;
        for (auto& n : names) cnames.push_back(n.c_str());
        if (narrow) ImGui::TextUnformatted("Library");
        ImGui::SetNextItemWidth(narrow ? -FLT_MIN : std::max(80.f, w - 120.f));
        if (ImGui::Combo("##lib", &selectedCombo, cnames.data(), (int)cnames.size())) {
            if (m_enumerateLibraries && selectedCombo >= 0 && selectedCombo < (int)entities.size()) {
                m_enumerateLibraries([&](entt::entity e, SwatchLibrary& lib) {
                    if (e == entities[selectedCombo]) {
                        m_activeLib = &lib;
                        m_activeLibraryEntity = e;
                        if (m_onSelectLibrary) m_onSelectLibrary(e);
                    }
                });
            }
            m_selectedColorIndex = -1;
            m_hasColorSelection = false;
        }
    }

    if (narrow) ImGui::Spacing();

    if (ImFonts::IconButtonGhost(ICON_FA_PLUS, "##addColor")) {
        openColorPicker(false);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Add color");
    }
    ImGui::SameLine();
    if (!m_hasColorSelection) {
        ImGui::BeginDisabled();
    }
    if (ImFonts::IconButtonGhost(ICON_FA_TRASH, "##deleteSwatch") && m_activeLib) {
        deleteSelectedSwatch(*m_activeLib);
    }
    if (!m_hasColorSelection) {
        ImGui::EndDisabled();
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        ImGui::SetTooltip(m_hasColorSelection ? "Delete selected swatch" : "Select a swatch to delete");
    }
    ImGui::SameLine();
    // Icon shows the view you will switch to (not the current mode).
    const char* viewIcon = (m_viewMode == SwatchViewMode::Grid) ? ICON_FA_LIST : ICON_FA_TH;
    if (ImFonts::IconButtonGhost(viewIcon, "##viewMode")) {
        m_viewMode = (m_viewMode == SwatchViewMode::Grid) ? SwatchViewMode::List : SwatchViewMode::Grid;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(m_viewMode == SwatchViewMode::Grid ? "Switch to list view" : "Switch to grid view");
    }
}

void SwatchesPanel::drawSelectionTools(SwatchLibrary& lib) {
    const auto& selected = lib.colors[m_selectedColorIndex];
    drawGreyValueRow(selected, lib);
    drawHarmonyStrip(selected, lib);
}

void SwatchesPanel::drawSwatchArea(SwatchLibrary& lib) {
    if (lib.empty()) {
        ImGui::TextDisabled("No colors. Use + or Edit > Add Color.");
        return;
    }
    if (m_viewMode == SwatchViewMode::List) {
        drawSwatchList(lib);
    } else {
        drawSwatchGrid(lib);
    }
}

void SwatchesPanel::drawSwatchGrid(SwatchLibrary& lib) {
    const float cellSize = gridSwatchSize();
    constexpr float kGridSpacing = 2.f;
    const float w = contentWidth();
    const int columns = std::max(1, (int)((w + kGridSpacing) / (cellSize + kGridSpacing)));

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(kGridSpacing, kGridSpacing));

    std::vector<int> greyFilter;
    if (m_filterSameGrey && m_hasColorSelection && m_selectedColorIndex >= 0) {
        greyFilter = lib.findSameGreyValue(
            lib.colors[m_selectedColorIndex].getGreyValuePercent(), m_sameGreyTolerance);
    }

    int col = 0;
    for (int i = 0; i < lib.count(); i++) {
        if (m_filterSameGrey && !greyFilter.empty()) {
            if (std::find(greyFilter.begin(), greyFilter.end(), i) == greyFilter.end()) {
                continue;
            }
        }
        if (col > 0) ImGui::SameLine(0, kGridSpacing);
        drawSwatchCell(lib.colors[i], i, lib, cellSize);
        col++;
        if (col >= columns) col = 0;
    }

    ImGui::PopStyleVar();
}

void SwatchesPanel::drawSwatchList(SwatchLibrary& lib) {
    std::vector<int> greyFilter;
    if (m_filterSameGrey && m_hasColorSelection && m_selectedColorIndex >= 0) {
        greyFilter = lib.findSameGreyValue(
            lib.colors[m_selectedColorIndex].getGreyValuePercent(), m_sameGreyTolerance);
    }

    const float sw = ImGui::GetFrameHeight();
    const ImGuiTableFlags tableFlags =
        ImGuiTableFlags_RowBg
        | ImGuiTableFlags_BordersInnerV
        | ImGuiTableFlags_Resizable
        | ImGuiTableFlags_SizingStretchProp;

    if (!ImGui::BeginTable("SwatchList", 5, tableFlags)) {
        return;
    }

    ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 22.f);
    ImGui::TableSetupColumn(" ", ImGuiTableColumnFlags_WidthFixed, sw + 6.f);
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.35f);
    ImGui::TableSetupColumn("Grey", ImGuiTableColumnFlags_WidthFixed, 44.f);
    ImGui::TableSetupColumn("Values", ImGuiTableColumnFlags_WidthStretch, 0.45f);
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableHeadersRow();

    for (int i = 0; i < lib.count(); i++) {
        if (m_filterSameGrey && !greyFilter.empty()) {
            if (std::find(greyFilter.begin(), greyFilter.end(), i) == greyFilter.end()) {
                continue;
            }
        }

        ImGui::TableNextRow();
        ImGui::PushID(i);
        const bool isSelected = (m_selectedColorIndex == i);

        ImGui::TableSetColumnIndex(0);
        ImGui::TextDisabled("%d", i + 1);

        ImGui::TableSetColumnIndex(1);
        ImVec4 colf(lib.colors[i].color.r / 255.f, lib.colors[i].color.g / 255.f,
                    lib.colors[i].color.b / 255.f, lib.colors[i].color.a / 255.f);
        if (ImGui::ColorButton("##c", colf,
                ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_AlphaPreview,
                ImVec2(sw, sw))) {
            m_selectedColorIndex = i;
            m_selectedColor = lib.colors[i].color;
            m_hasColorSelection = true;
            m_harmonyPreview = lib.colors[i].generateHarmony(m_previewHarmony);
        }
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            openColorPicker(true, i);
        }
        if (ImGui::BeginPopupContextItem("##ctxSwatch")) {
            drawSwatchContextMenu(i, lib);
            ImGui::EndPopup();
        }

        ImGui::TableSetColumnIndex(2);
        const bool renaming = (m_renamingSwatchIndex == i);
        if (renaming) {
            ImGui::SetNextItemWidth(-FLT_MIN);
            const bool commit = ImGui::InputText("##rename", m_renameBuf, sizeof(m_renameBuf),
                                                 ImGuiInputTextFlags_EnterReturnsTrue
                                                     | ImGuiInputTextFlags_AutoSelectAll);
            if (ImGui::IsItemActivated()) {
                ImGui::SetKeyboardFocusHere(-1);
            }
            if (commit || ImGui::IsItemDeactivatedAfterEdit()) {
                lib.colors[i].name = m_renameBuf;
                m_renamingSwatchIndex = -1;
                if (m_onLibraryChanged) m_onLibraryChanged();
            }
            if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                m_renamingSwatchIndex = -1;
            }
        } else {
            if (ImGui::Selectable(lib.colors[i].getDisplayName().c_str(), isSelected, 0, ImVec2(0, sw))) {
                m_selectedColorIndex = i;
                m_selectedColor = lib.colors[i].color;
                m_hasColorSelection = true;
                m_harmonyPreview = lib.colors[i].generateHarmony(m_previewHarmony);
            }
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                beginRenameSwatch(lib, i);
            }
            if (ImGui::BeginPopupContextItem("##ctxName")) {
                drawSwatchContextMenu(i, lib);
                ImGui::EndPopup();
            }
        }

        ImGui::TableSetColumnIndex(3);
        ImGui::AlignTextToFramePadding();
        ImGui::Text("%.1f%%", lib.colors[i].getGreyValuePercent());

        ImGui::TableSetColumnIndex(4);
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(formatSwatchValues(lib.colors[i]).c_str());
        if (ImGui::BeginPopupContextItem("##ctxValues")) {
            drawSwatchContextMenu(i, lib);
            ImGui::EndPopup();
        }

        const float y0 = ImGui::GetItemRectMin().y;
        const float y1 = ImGui::GetItemRectMax().y;
        auto drop = swatchIndexDragDrop("OFXSWATCH_IDX", i, lib.colors[i].getDisplayName().c_str(), y0, y1);
        if (drop.accepted) {
            int to = drop.target + (drop.zone == SwatchDropZone::After ? 1 : 0);
            if (drop.dragged < to) to--;
            lib.reorderColor(drop.dragged, ofClamp(to, 0, lib.count() - 1));
            if (m_selectedColorIndex == drop.dragged) m_selectedColorIndex = to;
            if (m_onLibraryChanged) m_onLibraryChanged();
        }

        ImGui::PopID();
    }

    ImGui::EndTable();
}

void SwatchesPanel::drawSelectionFooter(SwatchLibrary& lib) {
    ImGui::TextDisabled("%d colors in library (1 palette entity)", lib.count());
    if (m_hasColorSelection && m_selectedColorIndex >= 0 && m_selectedColorIndex < lib.count()) {
        const auto& sel = lib.colors[m_selectedColorIndex];
        ImGui::Text("%s", sel.getDisplayName().c_str());
        ImGui::SameLine();
        ImGui::TextDisabled("Grey %.1f%%", sel.getGreyValuePercent());
        if (sel.isNeutralGrey()) {
            ImGui::SameLine();
            ImGui::TextDisabled("(neutral)");
        }
        if (m_compareColorIndex >= 0 && m_compareColorIndex < lib.count()) {
            const float ratio = contrastRatio(sel.color, lib.colors[m_compareColorIndex].color);
            ImGui::Text("Contrast vs #%d: %.2f:1 %s", m_compareColorIndex + 1, ratio,
                meetsContrastAA(ratio) ? "(AA ok)" : "(below AA 4.5:1)");
        }
    } else {
        ImGui::TextDisabled("Click a swatch to select");
    }
}

void SwatchesPanel::drawSwatchCell(SwatchColor& color, int index, SwatchLibrary& lib, float cellSize) {
    ImGui::PushID(index);

    const bool isSelected = (m_selectedColorIndex == index);
    if (isSelected) {
        ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyle().Colors[ImGuiCol_CheckMark]);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
    }

    ImVec4 colf(color.color.r / 255.0f, color.color.g / 255.0f,
                color.color.b / 255.0f, color.color.a / 255.0f);
    if (ImGui::ColorButton("##sw", colf,
            ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_AlphaPreview,
            ImVec2(cellSize, cellSize))) {
        m_selectedColorIndex = index;
        m_selectedColor = color.color;
        m_hasColorSelection = true;
        m_harmonyPreview = color.generateHarmony(m_previewHarmony);
    }
    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        openColorPicker(true, index);
    }

    if (isSelected) {
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }

    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("%s", color.getDisplayName().c_str());
        ImGui::Text("Grey: %.1f%%", color.getGreyValuePercent());
        if (color.type == SwatchColorType::CMYK) {
            glm::vec4 cmyk = color.getCMYK();
            ImGui::Text("CMYK: %.0f, %.0f, %.0f, %.0f", cmyk.r, cmyk.g, cmyk.b, cmyk.a);
        } else {
            ImGui::Text("RGB: %d, %d, %d", (int)color.color.r, (int)color.color.g, (int)color.color.b);
        }
        ImGui::EndTooltip();
    }

    const float y0 = ImGui::GetItemRectMin().y;
    const float y1 = ImGui::GetItemRectMax().y;
    auto drop = swatchIndexDragDrop("OFXSWATCH_IDX", index, color.getDisplayName().c_str(), y0, y1);
    if (drop.accepted) {
        int to = drop.target + (drop.zone == SwatchDropZone::After ? 1 : 0);
        if (drop.dragged < to) to--;
        lib.reorderColor(drop.dragged, ofClamp(to, 0, lib.count() - 1));
        if (m_selectedColorIndex == drop.dragged) m_selectedColorIndex = to;
        if (m_onLibraryChanged) m_onLibraryChanged();
    }

    if (ImGui::BeginPopupContextItem("##ctxGrid")) {
        drawSwatchContextMenu(index, lib);
        ImGui::EndPopup();
    }

    ImGui::PopID();
}

void SwatchesPanel::drawHarmonyStrip(const SwatchColor& source, SwatchLibrary& lib) {
    const char* harmonyNames[] = {"Complementary", "Triadic", "Tetradic", "Analogous", "Split", "Mono"};
    int hIdx = (int)m_previewHarmony;

    ImGui::TextUnformatted("Harmony");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(std::max(168.f, contentWidth() * 0.55f));
    if (ImGui::Combo("##harmony", &hIdx, harmonyNames, IM_ARRAYSIZE(harmonyNames))) {
        m_previewHarmony = (ColorHarmony)hIdx;
        m_harmonyPreview = source.generateHarmony(m_previewHarmony);
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("Add all")) {
        for (auto& c : m_harmonyPreview) lib.addColor(c);
        if (m_onLibraryChanged) m_onLibraryChanged();
    }

    if (m_harmonyPreview.empty()) {
        m_harmonyPreview = source.generateHarmony(m_previewHarmony);
    }

    if (ImGui::BeginChild("HarmonyPreviews", ImVec2(0, 32), false, ImGuiWindowFlags_HorizontalScrollbar)) {
        for (size_t i = 0; i < m_harmonyPreview.size(); i++) {
            if (i > 0) ImGui::SameLine();
            ImGui::PushID((int)i + 1000);
            const auto& c = m_harmonyPreview[i];
            ImVec4 colf(c.color.r / 255.0f, c.color.g / 255.0f, c.color.b / 255.0f, 1.0f);
            if (ImGui::ColorButton("##h", colf, ImGuiColorEditFlags_NoTooltip, ImVec2(28, 28))) {
                lib.addColor(c);
                if (m_onLibraryChanged) m_onLibraryChanged();
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s\nClick to add", c.getDisplayName().c_str());
            }
            ImGui::PopID();
        }
    }
    ImGui::EndChild();
}

void SwatchesPanel::drawGreyValueRow(const SwatchColor& selected, SwatchLibrary& lib) {
    ImGui::Text("Grey value: %.1f%%", selected.getGreyValuePercent());

    const float w = contentWidth();
    if (w >= 340.f) {
        ImGui::SameLine();
        ImGui::Checkbox("Same grey filter", &m_filterSameGrey);
        if (m_filterSameGrey) {
            ImGui::SameLine();
            ImGui::SetNextItemWidth(56);
            ImGui::DragFloat("Tol##grey", &m_sameGreyTolerance, 0.1f, 0.1f, 10.0f, "%.1f");
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Match grey to compare") && m_compareColorIndex >= 0
            && m_compareColorIndex < lib.count()) {
            const float target = lib.colors[m_compareColorIndex].getGreyValuePercent();
            if (m_selectedColorIndex >= 0 && m_selectedColorIndex < lib.count()) {
                lib.colors[m_selectedColorIndex] = selected.withGreyValuePercent(target);
                m_selectedColor = lib.colors[m_selectedColorIndex].color;
                if (m_onLibraryChanged) m_onLibraryChanged();
            }
        }
    } else {
        ImGui::Checkbox("Same grey filter", &m_filterSameGrey);
        if (m_filterSameGrey) {
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::DragFloat("Tolerance", &m_sameGreyTolerance, 0.1f, 0.1f, 10.0f, "%.1f");
        }
        if (ImGui::SmallButton("Match grey to compare") && m_compareColorIndex >= 0
            && m_compareColorIndex < lib.count()) {
            const float target = lib.colors[m_compareColorIndex].getGreyValuePercent();
            if (m_selectedColorIndex >= 0 && m_selectedColorIndex < lib.count()) {
                lib.colors[m_selectedColorIndex] = selected.withGreyValuePercent(target);
                m_selectedColor = lib.colors[m_selectedColorIndex].color;
                if (m_onLibraryChanged) m_onLibraryChanged();
            }
        }
    }
}

void SwatchesPanel::drawColorPickerModal(SwatchLibrary& lib) {
    if (!m_showColorEditor) {
        m_colorPickerSized = false;
        return;
    }

    const ImVec2 minSize = m_pickerCmykMode ? ImVec2(360, 400) : ImVec2(380, 520);
    const ImVec2 maxSize(420, 720);
    ImGui::SetNextWindowSizeConstraints(minSize, maxSize);
    if (!m_colorPickerSized) {
        ImGui::SetNextWindowSize(minSize, ImGuiCond_Always);
        m_colorPickerSized = true;
    }

    const ImGuiWindowFlags pickerFlags =
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

    if (!ImGui::Begin("Color Picker", &m_showColorEditor, pickerFlags)) {
        ImGui::End();
        return;
    }

    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputText("Name", m_newColorName, sizeof(m_newColorName));

    if (ImGui::RadioButton("RGB", !m_pickerCmykMode)) {
        m_pickerCmykMode = false;
        m_colorPickerSized = false;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("CMYK", m_pickerCmykMode)) {
        m_pickerCmykMode = true;
        syncPickerRgbFromCmyk();
        m_colorPickerSized = false;
    }

    ImGui::Separator();

    if (m_pickerCmykMode) {
        drawCmykInputs(lib);
    } else {
        const float pickerW = ImGui::GetContentRegionAvail().x;
        ImGui::SetNextItemWidth(pickerW);
        ImGui::ColorPicker4("##wheel", m_newColorRGB, kPickerFlags);
    }

    const SwatchColor previewSw(
        ofColor(m_newColorRGB[0] * 255, m_newColorRGB[1] * 255, m_newColorRGB[2] * 255, m_newColorRGB[3] * 255));
    ImGui::Text("Grey value: %.1f%%", previewSw.getGreyValuePercent());
    if (previewSw.isNeutralGrey()) {
        ImGui::SameLine();
        ImGui::TextDisabled("(neutral)");
    }

    ImGui::Separator();

    const char* applyLabel = m_editingExisting ? "Apply" : "Add to library";
    const float availW = ImGui::GetContentRegionAvail().x;
    const float spacing = ImGui::GetStyle().ItemSpacing.x;
    const float btnW = (availW - spacing) * 0.5f;
    if (ImGui::Button(applyLabel, ImVec2(btnW, 0))) {
        SwatchColor c;
        if (m_pickerCmykMode) {
            c = SwatchColor::fromCMYK(
                m_newColorCMYK[0], m_newColorCMYK[1], m_newColorCMYK[2], m_newColorCMYK[3], m_newColorName);
        } else {
            c.color = ofColor(m_newColorRGB[0] * 255, m_newColorRGB[1] * 255,
                              m_newColorRGB[2] * 255, m_newColorRGB[3] * 255);
            c.name = m_newColorName;
            c.type = SwatchColorType::RGB;
        }
        if (m_editingExisting && m_editingIndex >= 0 && m_editingIndex < lib.count()) {
            lib.colors[m_editingIndex] = c;
            m_selectedColorIndex = m_editingIndex;
            m_selectedColor = c.color;
            m_hasColorSelection = true;
        } else {
            lib.addColor(c);
            m_selectedColorIndex = lib.count() - 1;
            m_selectedColor = c.color;
            m_hasColorSelection = true;
        }
        if (m_onLibraryChanged) m_onLibraryChanged();
        m_showColorEditor = false;
        m_editingExisting = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(btnW, 0))) {
        m_showColorEditor = false;
    }

    ImGui::End();
}

} // namespace ofxSwatches
