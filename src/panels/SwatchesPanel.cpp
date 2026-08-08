#include "SwatchesPanel.h"
#include "SwatchListReorder.h"
#include "SwatchJson.h"
#include "SwatchDraw.h"
#include "SwatchDocumentBridge.h"
#include "DocumentPaints.h"
#include "ImFonts.h"
#include "IconsFontAwesome5.h"
#include "imgui.h"
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstring>

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
    if (c.isGradient()) {
        char buf[64];
        snprintf(buf, sizeof(buf), "Gradient · %d stops", (int)c.gradient.stops.size());
        return buf;
    }
    if (c.type == SwatchColorType::CMYK) {
        const glm::vec4 v = c.getCMYK();
        char buf[64];
        snprintf(buf, sizeof(buf), "C%.0f  M%.0f  Y%.0f  K%.0f", v.r, v.g, v.b, v.a);
        return buf;
    }
    char buf[48];
    const ofColor pc = c.previewColor();
    snprintf(buf, sizeof(buf), "R%d  G%d  B%d", (int)pc.r, (int)pc.g, (int)pc.b);
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

void SwatchesPanel::setDocumentRegistry(entt::registry* registry) {
    m_documentRegistry = registry;
}

void SwatchesPanel::setColorTarget(DualColorTarget* target) {
    m_colorTarget = target;
}

void SwatchesPanel::setOnColorSelected(std::function<void(const ofColor&)> cb) {
    m_onColorSelected = std::move(cb);
}

void SwatchesPanel::setOnSwatchSelected(std::function<void(const SwatchColor&)> cb) {
    m_onSwatchSelected = std::move(cb);
}

void SwatchesPanel::setOnDocumentPaintsChanged(std::function<void()> cb) {
    m_onDocumentPaintsChanged = std::move(cb);
}

void SwatchesPanel::applySelectedColor(const ofColor& color) {
    applySelectedSwatch(SwatchColor(color));
}

void SwatchesPanel::applySelectedSwatch(const SwatchColor& swatch) {
    m_selectedSwatch = swatch;
    m_hasColorSelection = true;
    if (m_colorTarget)
        m_colorTarget->assignActiveSwatch(swatch);
    if (m_onSwatchSelected)
        m_onSwatchSelected(swatch);
    if (m_onColorSelected)
        m_onColorSelected(swatch.previewColor());
}

float SwatchesPanel::contentWidth() const {
    return std::max(80.f, ImGui::GetContentRegionAvail().x);
}

bool SwatchesPanel::hasValidSelection() const {
    return m_hasColorSelection && m_activeLib && m_selectedColorIndex >= 0
        && m_selectedColorIndex < m_activeLib->count();
}

float SwatchesPanel::swatchFooterReserveHeight() const {
    // Compact footer: count line + selection/hint (+ optional action / contrast).
    float lines = 2.f;
    if (hasValidSelection()) {
        if (m_documentRegistry) lines += 1.f;
        if (m_compareColorIndex >= 0 && m_compareColorIndex < m_activeLib->count())
            lines += 1.f;
    }
    return lines * ImGui::GetTextLineHeightWithSpacing()
        + ImGui::GetStyle().ItemSpacing.y * 2.f;
}

float SwatchesPanel::minSwatchAreaHeight() const {
    // Always leave room for at least ~2 rows of chips so the grid never collapses.
    const float cell = gridSwatchSize();
    return std::max(96.f, cell * 2.f + ImGui::GetStyle().ItemSpacing.y * 3.f);
}

float SwatchesPanel::gridSwatchSize() const {
    constexpr float kGridSpacing = 2.f;
    const float w = contentWidth();
    // Prefer the preferred cell size; shrink only when the panel is very narrow.
    const float preferred = m_gridCellSize;
    if (w >= preferred) {
        const int cols = std::max(1, (int)((w + kGridSpacing) / (preferred + kGridSpacing)));
        const float fit = (w - kGridSpacing * (cols - 1)) / (float)cols;
        return ofClamp(fit, 28.f, 96.f);
    }
    return ofClamp(w, 24.f, preferred);
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
    if (lib.colors[index].isGradient()) {
        if (ImGui::MenuItem("Edit gradient...")) {
            m_showGradientEditor = true;
            m_gradientEditIndex = index;
            m_gradientEdit = lib.colors[index];
            m_gradientEdit.refreshPreviewFromGradient();
        }
    } else if (ImGui::MenuItem("Edit...")) {
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

    drawIconToolbar();
    ImGui::Separator();

    // Swatch grid is primary: always reserve a real height so chips never collapse
    // when tools / document paints / a narrow dock steal vertical space.
    const float footerH = swatchFooterReserveHeight();
    const float minGrid = minSwatchAreaHeight();
    float softReserve = footerH;
    if (m_showColorTools && hasValidSelection())
        softReserve += 8.f; // tools draw below; window can scroll if needed
    if (m_documentRegistry)
        softReserve += ImGui::GetFrameHeightWithSpacing(); // collapsed header

    const float avail = ImGui::GetContentRegionAvail().y;
    const float childH = std::max(minGrid, avail - softReserve);

    if (ImGui::BeginChild("SwatchScroll", ImVec2(0, childH), ImGuiChildFlags_Borders,
                          ImGuiWindowFlags_None)) {
        drawSwatchArea(*m_activeLib);
    }
    ImGui::EndChild();

    if (m_showColorTools) {
        ImGui::Separator();
        if (hasValidSelection())
            drawSelectionTools(*m_activeLib);
        else
            ImGui::TextDisabled("Select a swatch to use colour tools.");
    }

    if (m_documentRegistry) {
        ImGui::Separator();
        drawDocumentPaintsSection();
    }

    ImGui::Separator();
    drawSelectionFooter(*m_activeLib);
    drawColorPickerModal(*m_activeLib);
    drawGradientEditorModal(*m_activeLib);
}

void SwatchesPanel::draw(const char* title, bool* visible, int extraWindowFlags) {
    if (!visible || !*visible) return;

    if (m_firstWindowSize) {
        ImGui::SetNextWindowSize(ImVec2(kDefaultWindowWidth, kDefaultWindowHeight), ImGuiCond_FirstUseEver);
        // Narrow docks are fine — content wraps instead of requiring a wide panel.
        ImGui::SetNextWindowSizeConstraints(ImVec2(160, 220), ImVec2(FLT_MAX, FLT_MAX));
        m_firstWindowSize = false;
    }

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_AlwaysVerticalScrollbar | extraWindowFlags;
    if (!ImGui::Begin(title, visible, flags)) {
        ImGui::End();
        return;
    }

    drawMenuBar();
    drawContent();
    ImGui::End();
}

void SwatchesPanel::selectLibraryByEntity(entt::entity e) {
    if (!m_enumerateLibraries || e == entt::null) return;
    m_enumerateLibraries([&](entt::entity ent, SwatchLibrary& lib) {
        if (ent != e) return;
        m_activeLib = &lib;
        m_activeLibraryEntity = ent;
        m_selectedColorIndex = -1;
        m_hasColorSelection = false;
        if (m_onSelectLibrary) m_onSelectLibrary(ent);
    });
}

void SwatchesPanel::drawLibraryMenu() {
    if (!ImGui::BeginMenu("Library")) return;

    if (m_enumerateLibraries) {
        bool any = false;
        m_enumerateLibraries([&](entt::entity e, SwatchLibrary& lib) {
            any = true;
            const bool selected = (e == m_activeLibraryEntity) || (&lib == m_activeLib);
            if (ImGui::MenuItem(lib.libraryName.c_str(), nullptr, selected))
                selectLibraryByEntity(e);
        });
        if (!any && m_activeLib) {
            ImGui::MenuItem(m_activeLib->libraryName.c_str(), nullptr, true, false);
        }
    } else if (m_activeLib) {
        ImGui::MenuItem(m_activeLib->libraryName.c_str(), nullptr, true, false);
    } else {
        ImGui::MenuItem("(none)", nullptr, false, false);
    }

    ImGui::EndMenu();
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

    drawLibraryMenu();

    if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("Add Color...", nullptr, false, m_activeLib != nullptr)) {
            openColorPicker(false);
        }
        if (ImGui::MenuItem("Add Gradient", nullptr, false, m_activeLib != nullptr)) {
            m_activeLib->addGradient("Gradient");
            m_selectedColorIndex = m_activeLib->count() - 1;
            applySelectedSwatch(m_activeLib->colors.back());
            if (m_onLibraryChanged) m_onLibraryChanged();
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
        if (ImGui::MenuItem("Show Tools", nullptr, m_showColorTools)) {
            m_showColorTools = !m_showColorTools;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Harmony, grey value, and related colour tools");
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Grid", nullptr, m_viewMode == SwatchViewMode::Grid)) {
            m_viewMode = SwatchViewMode::Grid;
        }
        if (ImGui::MenuItem("List", nullptr, m_viewMode == SwatchViewMode::List)) {
            m_viewMode = SwatchViewMode::List;
        }
        if (m_viewMode == SwatchViewMode::Grid) {
            ImGui::Separator();
            ImGui::SetNextItemWidth(120);
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

void SwatchesPanel::drawIconToolbar() {
    if (m_activeLib) {
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("%s", m_activeLib->libraryName.c_str());
        ImGui::SameLine();
    }

    if (ImFonts::IconButtonGhost(ICON_FA_PLUS, "##addColor")) {
        openColorPicker(false);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Add solid color");
    }
    ImGui::SameLine();
    if (ImFonts::IconButtonGhost(ICON_FA_FILL_DRIP, "##addGradient") && m_activeLib) {
        m_activeLib->addGradient("Gradient");
        m_selectedColorIndex = m_activeLib->count() - 1;
        applySelectedSwatch(m_activeLib->colors.back());
        if (m_onLibraryChanged) m_onLibraryChanged();
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Add gradient swatch");
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
        m_viewMode = (m_viewMode == SwatchViewMode::Grid) ? SwatchViewMode::List
                                                          : SwatchViewMode::Grid;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(m_viewMode == SwatchViewMode::Grid ? "Switch to list view"
                                                             : "Switch to grid view");
    }
    ImGui::SameLine();
    if (ImFonts::IconButtonGhost(ICON_FA_SLIDERS_H, "##tools")) {
        m_showColorTools = !m_showColorTools;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(m_showColorTools ? "Hide colour tools (View → Show Tools)"
                                           : "Show colour tools (View → Show Tools)");
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
        {
            const ImVec2 chipMin = ImGui::GetCursorScreenPos();
            if (ImGui::InvisibleButton("##c", ImVec2(sw, sw))) {
                m_selectedColorIndex = i;
                applySelectedSwatch(lib.colors[i]);
                m_harmonyPreview = lib.colors[i].generateHarmony(m_previewHarmony);
            }
            drawSwatchRect(ImGui::GetWindowDrawList(), chipMin, ImGui::GetItemRectMax(),
                           lib.colors[i], 2.f);
            ImGui::GetWindowDrawList()->AddRect(
                chipMin, ImGui::GetItemRectMax(),
                isSelected ? ImGui::GetColorU32(ImGuiCol_CheckMark) : IM_COL32(70, 70, 80, 255),
                2.f, 0, isSelected ? 2.f : 1.f);
        }
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)
            && lib.colors[i].isSolid()) {
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
                applySelectedSwatch(lib.colors[i]);
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
    ImGui::TextDisabled("%d color%s", lib.count(), lib.count() == 1 ? "" : "s");
    if (hasValidSelection()) {
        const auto& sel = lib.colors[m_selectedColorIndex];
        ImGui::TextWrapped("%s  ·  Grey %.1f%%%s",
                           sel.getDisplayName().c_str(),
                           sel.getGreyValuePercent(),
                           sel.isNeutralGrey() ? " (neutral)" : "");
        if (m_colorTarget) {
            ImGui::TextDisabled("Apply to %s", m_colorTarget->activeLabel());
        }
        if (m_documentRegistry) {
            if (ImGui::Button("Add to document paints", ImVec2(-FLT_MIN, 0))) {
                ofxSwatches::ensureDocumentPaintFromSwatch(*m_documentRegistry, sel);
                if (m_onDocumentPaintsChanged) m_onDocumentPaintsChanged();
            }
        }
        if (m_compareColorIndex >= 0 && m_compareColorIndex < lib.count()) {
            const float ratio =
                contrastRatio(sel.previewColor(), lib.colors[m_compareColorIndex].previewColor());
            ImGui::TextWrapped("Contrast vs #%d: %.2f:1 %s", m_compareColorIndex + 1, ratio,
                meetsContrastAA(ratio) ? "(AA ok)" : "(below AA 4.5:1)");
        }
    } else {
        ImGui::TextDisabled("Click a swatch to select");
    }
}

void SwatchesPanel::drawDocumentPaintsSection() {
    if (!m_documentRegistry) return;
    // Collapsed by default — keeps the swatch grid visible in narrow docks.
    if (!ImGui::CollapsingHeader("Document Paints"))
        return;

    auto& reg = *m_documentRegistry;
    ofxDocumentKit::pruneDocumentPaints(reg);
    auto& lib = ofxDocumentKit::getOrCreateDocumentPaints(reg);

    const float w = contentWidth();
    const float gap = ImGui::GetStyle().ItemSpacing.x;
    const bool stack = w < 220.f;
    const float half = stack ? -FLT_MIN : (w - gap) * 0.5f;
    if (ImGui::Button("New solid##swDocPaint", ImVec2(half, 0))) {
        ofxDocumentKit::createDocumentSolidPaint(reg, glm::vec4(1.f));
        if (m_onDocumentPaintsChanged) m_onDocumentPaintsChanged();
    }
    if (!stack) ImGui::SameLine();
    if (ImGui::Button("New gradient##swDocPaint", ImVec2(stack ? -FLT_MIN : half, 0))) {
        ofxDocumentKit::createDocumentGradientPaint(reg, "Gradient");
        if (m_onDocumentPaintsChanged) m_onDocumentPaintsChanged();
    }

    if (lib.paints.empty()) {
        ImGui::TextWrapped("Empty — promote a swatch or create a paint.");
        return;
    }

    for (entt::entity pe : lib.paints) {
        if (!reg.valid(pe)) continue;
        ImGui::PushID(static_cast<int>(pe));

        SwatchColor asSwatch;
        const bool ok = documentPaintToSwatch(reg, pe, asSwatch);
        const std::string label = ok ? asSwatch.getDisplayName() : "Paint";
        const ImVec2 chipMin = ImGui::GetCursorScreenPos();
        const float chip = ImGui::GetFrameHeight();
        ImGui::InvisibleButton("##dpHit", ImVec2(chip, chip));
        const bool clicked = ImGui::IsItemClicked();
        if (ok) {
            drawSwatchRect(ImGui::GetWindowDrawList(), chipMin,
                           ImVec2(chipMin.x + chip, chipMin.y + chip), asSwatch, 2.f);
            ImGui::GetWindowDrawList()->AddRect(
                chipMin, ImVec2(chipMin.x + chip, chipMin.y + chip),
                IM_COL32(80, 80, 90, 255), 2.f);
        }
        ImGui::SameLine();
        if (ImGui::Selectable(label.c_str(), false, ImGuiSelectableFlags_None,
                              ImVec2(ImGui::GetContentRegionAvail().x - 56.f, 0))
            || clicked) {
            applySelectedSwatch(asSwatch);
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("+##addLib") && ok && m_activeLib) {
            m_activeLib->addColor(asSwatch);
            if (m_onLibraryChanged) m_onLibraryChanged();
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Add to swatch library");
        ImGui::SameLine();
        if (ImGui::SmallButton("x##rmDocPaint")) {
            ofxDocumentKit::removeDocumentPaint(reg, pe);
            if (m_onDocumentPaintsChanged) m_onDocumentPaintsChanged();
            ImGui::PopID();
            break;
        }
        ImGui::PopID();
    }
}

void SwatchesPanel::drawSwatchCell(SwatchColor& color, int index, SwatchLibrary& lib, float cellSize) {
    ImGui::PushID(index);

    const bool isSelected = (m_selectedColorIndex == index);
    const ImVec2 cellMin = ImGui::GetCursorScreenPos();
    if (ImGui::InvisibleButton("##sw", ImVec2(cellSize, cellSize))) {
        m_selectedColorIndex = index;
        applySelectedSwatch(color);
        m_harmonyPreview = color.generateHarmony(m_previewHarmony);
    }
    const ImVec2 cellMax = ImGui::GetItemRectMax();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    drawSwatchRect(dl, cellMin, cellMax, color, 2.f);
    const ImU32 border = isSelected ? ImGui::GetColorU32(ImGuiCol_CheckMark)
                                    : IM_COL32(70, 70, 80, 255);
    dl->AddRect(cellMin, cellMax, border, 2.f, 0, isSelected ? 2.5f : 1.25f);

    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        if (color.isSolid()) {
            openColorPicker(true, index);
        } else {
            m_showGradientEditor = true;
            m_gradientEditIndex = index;
            m_gradientEdit = color;
            m_gradientEdit.refreshPreviewFromGradient();
        }
    }

    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("%s", color.getDisplayName().c_str());
        ImGui::TextUnformatted(formatSwatchValues(color).c_str());
        ImGui::Text("Grey: %.1f%%", color.getGreyValuePercent());
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
    const char* harmonyNames[] = {
        "Complementary", "Triadic", "Tetradic", "Analogous", "Split", "Mono"};
    int hIdx = (int)m_previewHarmony;

    ImGui::TextUnformatted("Harmony");
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::Combo("##harmony", &hIdx, harmonyNames, IM_ARRAYSIZE(harmonyNames))) {
        m_previewHarmony = (ColorHarmony)hIdx;
        m_harmonyPreview = source.generateHarmony(m_previewHarmony);
    }
    if (ImGui::Button("Add all", ImVec2(-FLT_MIN, 0))) {
        for (auto& c : m_harmonyPreview) lib.addColor(c);
        if (m_onLibraryChanged) m_onLibraryChanged();
    }

    if (m_harmonyPreview.empty()) {
        m_harmonyPreview = source.generateHarmony(m_previewHarmony);
    }

    if (ImGui::BeginChild("HarmonyPreviews", ImVec2(0, 32), ImGuiChildFlags_None,
                          ImGuiWindowFlags_HorizontalScrollbar)) {
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
    ImGui::Checkbox("Same grey filter", &m_filterSameGrey);
    if (m_filterSameGrey) {
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::DragFloat("Tolerance", &m_sameGreyTolerance, 0.1f, 0.1f, 10.0f, "%.1f");
    }
    if (ImGui::Button("Match grey to compare", ImVec2(-FLT_MIN, 0))
        && m_compareColorIndex >= 0 && m_compareColorIndex < lib.count()) {
        const float target = lib.colors[m_compareColorIndex].getGreyValuePercent();
        if (m_selectedColorIndex >= 0 && m_selectedColorIndex < lib.count()) {
            lib.colors[m_selectedColorIndex] = selected.withGreyValuePercent(target);
            m_selectedSwatch = lib.colors[m_selectedColorIndex];
            if (m_onLibraryChanged) m_onLibraryChanged();
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
            // Preserve gradient entries if user somehow opened solid picker on one.
            if (lib.colors[m_editingIndex].isGradient()) {
                c = lib.colors[m_editingIndex];
            } else {
                lib.colors[m_editingIndex] = c;
            }
            m_selectedColorIndex = m_editingIndex;
            applySelectedSwatch(lib.colors[m_editingIndex]);
        } else {
            lib.addColor(c);
            m_selectedColorIndex = lib.count() - 1;
            applySelectedSwatch(c);
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

void SwatchesPanel::drawGradientEditorModal(SwatchLibrary& lib) {
    if (!m_showGradientEditor)
        return;

    ImGui::SetNextWindowSize(ImVec2(360, 420), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Edit Gradient", &m_showGradientEditor, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    char nameBuf[128];
    strncpy(nameBuf, m_gradientEdit.name.c_str(), sizeof(nameBuf) - 1);
    nameBuf[sizeof(nameBuf) - 1] = '\0';
    if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf)))
        m_gradientEdit.name = nameBuf;

    int type = (int)m_gradientEdit.gradient.type;
    if (ImGui::Combo("Type", &type, "Linear\0Radial\0"))
        m_gradientEdit.gradient.type = (SwatchGradientType)type;
    if (m_gradientEdit.gradient.type == SwatchGradientType::Linear)
        ImGui::DragFloat("Angle", &m_gradientEdit.gradient.angle, 1.f, 0.f, 360.f, "%.0f");

    const ImVec2 rampMin = ImGui::GetCursorScreenPos();
    const float rampH = 28.f;
    ImGui::InvisibleButton("##gradRamp", ImVec2(-FLT_MIN, rampH));
    drawSwatchRect(ImGui::GetWindowDrawList(), rampMin, ImGui::GetItemRectMax(),
                   m_gradientEdit, 3.f);

    ImGui::Separator();
    ImGui::TextUnformatted("Stops");
    for (size_t i = 0; i < m_gradientEdit.gradient.stops.size(); ++i) {
        ImGui::PushID((int)i);
        auto& stop = m_gradientEdit.gradient.stops[i];
        ImGui::SetNextItemWidth(72.f);
        ImGui::DragFloat("##pos", &stop.position, 0.01f, 0.f, 1.f, "%.2f");
        ImGui::SameLine();
        float rgba[4] = {stop.color.x, stop.color.y, stop.color.z, stop.color.w};
        if (ImGui::ColorEdit4("##col", rgba,
                ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar)) {
            stop.color = glm::vec4(rgba[0], rgba[1], rgba[2], rgba[3]);
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("-") && m_gradientEdit.gradient.stops.size() > 2) {
            m_gradientEdit.gradient.stops.erase(m_gradientEdit.gradient.stops.begin()
                                                + (ptrdiff_t)i);
            ImGui::PopID();
            break;
        }
        ImGui::PopID();
    }
    if (ImGui::Button("Add stop") && m_gradientEdit.gradient.stops.size() < 16) {
        m_gradientEdit.gradient.stops.emplace_back(
            0.5f, glm::vec4(0.5f, 0.5f, 0.5f, 1.f));
    }

    m_gradientEdit.kind = SwatchKind::Gradient;
    m_gradientEdit.gradient.sortStops();
    m_gradientEdit.refreshPreviewFromGradient();

    ImGui::Separator();
    const float availW = ImGui::GetContentRegionAvail().x;
    const float spacing = ImGui::GetStyle().ItemSpacing.x;
    const float btnW = (availW - spacing) * 0.5f;
    if (ImGui::Button("Apply", ImVec2(btnW, 0))) {
        if (m_gradientEditIndex >= 0 && m_gradientEditIndex < lib.count()) {
            lib.colors[m_gradientEditIndex] = m_gradientEdit;
            m_selectedColorIndex = m_gradientEditIndex;
            applySelectedSwatch(lib.colors[m_gradientEditIndex]);
            if (m_onLibraryChanged) m_onLibraryChanged();
        }
        m_showGradientEditor = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(btnW, 0)))
        m_showGradientEditor = false;

    ImGui::End();
}

} // namespace ofxSwatches
