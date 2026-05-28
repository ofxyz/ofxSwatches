#pragma once

#include "SwatchLibrary.h"
#include <entt/entt.hpp>
#include <functional>
#include <string>
#include <vector>

namespace ofxSwatches {

enum class SwatchViewMode {
    Grid = 0,
    List = 1
};

using LibraryEnumerator = std::function<void(const std::function<void(entt::entity, SwatchLibrary&)>&)>;

class SwatchesPanel {
public:
    static constexpr float kDefaultWindowWidth = 340.f;
    static constexpr float kDefaultWindowHeight = 520.f;

    /// Active library must live on the same registry passed to ofkitty::Runtime::attach().
    void setup(SwatchLibrary* activeLibrary);

    /// Template overload so SwatchesPanel does not need to know the concrete ECS component type.
    /// TLibraryComponent must be a type that inherits from (or is) SwatchLibrary.
    /// Typical usage: panel.setup<ecs::swatch_library_component>(registry, entity);
    template<typename TLibraryComponent>
    void setup(entt::registry& registry, entt::entity libraryEntity) {
        if (libraryEntity == entt::null || !registry.valid(libraryEntity)
            || !registry.all_of<TLibraryComponent>(libraryEntity)) {
            m_activeLib = nullptr;
            m_activeLibraryEntity = entt::null;
            ofLogWarning("ofxSwatches") << "SwatchesPanel::setup: invalid swatch library entity.";
            return;
        }
        m_activeLibraryEntity = libraryEntity;
        m_activeLib = &registry.get<TLibraryComponent>(libraryEntity);
    }
    void setLibraryEnumerator(LibraryEnumerator enumerator);
    void setOnSelectLibrary(std::function<void(entt::entity)> cb);
    void setOnLibraryChanged(std::function<void()> cb);

    void draw(const char* title, bool* visible, int extraWindowFlags = 0);
    void drawContent();
    void drawMenuBar();

    SwatchViewMode getViewMode() const { return m_viewMode; }
    void setViewMode(SwatchViewMode mode) { m_viewMode = mode; }

    ofColor getSelectedColor() const { return m_selectedColor; }
    bool hasColorSelection() const { return m_hasColorSelection; }
    int getSelectedColorIndex() const { return m_selectedColorIndex; }
    entt::entity getActiveLibraryEntity() const { return m_activeLibraryEntity; }

private:
    float contentWidth() const;
    float gridSwatchSize() const;
    float swatchFooterReserveHeight() const;

    void drawLibraryToolbar();
    void drawSelectionTools(SwatchLibrary& lib);
    void drawHarmonyStrip(const SwatchColor& source, SwatchLibrary& lib);
    void drawGreyValueRow(const SwatchColor& selected, SwatchLibrary& lib);
    void drawSwatchArea(SwatchLibrary& lib);
    void drawSwatchGrid(SwatchLibrary& lib);
    void drawSwatchList(SwatchLibrary& lib);
    void drawSelectionFooter(SwatchLibrary& lib);
    void drawColorPickerModal(SwatchLibrary& lib);
    void drawSwatchCell(SwatchColor& color, int index, SwatchLibrary& lib, float cellSize);
    void drawSwatchContextMenu(int index, SwatchLibrary& lib);
    void beginRenameSwatch(SwatchLibrary& lib, int index);
    void drawCmykInputs(SwatchLibrary& lib);
    void syncPickerRgbFromCmyk();

    void openColorPicker(bool editing, int index = -1);
    void deleteSelectedSwatch(SwatchLibrary& lib);

    SwatchLibrary* m_activeLib = nullptr;
    LibraryEnumerator m_enumerateLibraries;
    entt::entity m_activeLibraryEntity = entt::null;

    std::function<void(entt::entity)> m_onSelectLibrary;
    std::function<void()> m_onLibraryChanged;

    SwatchViewMode m_viewMode = SwatchViewMode::Grid;
    float m_gridCellSize = 56.f;
    bool m_firstWindowSize = true;
    bool m_colorPickerSized = false;
    bool m_confirmClearLibrary = false;

    int m_selectedColorIndex = -1;
    ofColor m_selectedColor = ofColor::white;
    bool m_hasColorSelection = false;

    bool m_showColorEditor = false;
    bool m_filterSameGrey = false;
    float m_sameGreyTolerance = 1.0f;
    int m_compareColorIndex = -1;
    bool m_pickerCmykMode = false;

    std::vector<SwatchColor> m_harmonyPreview;
    ColorHarmony m_previewHarmony = ColorHarmony::Complementary;

    float m_newColorRGB[4] = {1, 1, 1, 1};
    float m_newColorCMYK[4] = {0, 0, 0, 0};
    char m_newColorName[128] = "";
    char m_newLibraryName[128] = "New Library";
    bool m_editingExisting = false;
    int m_editingIndex = -1;

    int m_renamingSwatchIndex = -1;
    char m_renameBuf[128] = {};
};

} // namespace ofxSwatches
