#include "ofApp.h"

void ofApp::setup() {
    ofSetWindowTitle("ofxSwatches — Swatches Panel");
    ofSetFrameRate(60);
    ofSetBackgroundAuto(true);

    // Same registry as ofkitty::Runtime::attach(..., registry()) in main.cpp.
    auto& reg = ofkitty::runtime().registry();

    m_libraryEntity = reg.create();
    reg.emplace<ecs::swatch_library_component>(m_libraryEntity, "Plotter CMYK");
    auto& lib = reg.get<ecs::swatch_library_component>(m_libraryEntity);
    lib.addCMYK(0, 0, 0, 0, "Paper");
    lib.addCMYK(100, 0, 0, 0, "Cyan");
    lib.addCMYK(0, 100, 0, 0, "Magenta");
    lib.addCMYK(0, 0, 100, 0, "Yellow");
    lib.addCMYK(0, 0, 0, 100, "Black");

    m_panel.setup<ecs::swatch_library_component>(reg, m_libraryEntity);
    m_panel.setLibraryEnumerator([&reg](const std::function<void(entt::entity, ofxSwatches::SwatchLibrary&)>& fn) {
        auto view = reg.view<ecs::swatch_library_component>();
        for (auto e : view) {
            fn(e, reg.get<ecs::swatch_library_component>(e));
        }
    });

    ofkitty::runtime().registerWindow({
        "Swatches",
        "View",
        true,
        false,
        [this](bool& visible) {
            m_showSwatches = visible;
            if (visible) {
                m_panel.draw("Swatches###example", &m_showSwatches);
            }
        }
    });

    ofkitty::runtime().setEditMode(true);
}

void ofApp::update() {}

void ofApp::draw() {
    ofColor bg(40, 40, 40);
    auto& reg = ofkitty::runtime().registry();
    if (m_libraryEntity != entt::null && reg.valid(m_libraryEntity)) {
        auto& lib = reg.get<ecs::swatch_library_component>(m_libraryEntity);
        if (m_panel.hasColorSelection()) {
            bg = m_panel.getSelectedColor();
        } else if (!lib.empty()) {
            bg = lib.colors[0].color;
        }
    }
    ofBackground(bg);

    const int hint = (bg.getBrightness() > 140) ? 0 : 255;
    ofSetColor(hint);
    ofDrawBitmapStringHighlight("Cmd+E: toggle edit mode | S: save JSON | Swatches panel in View menu", 20, 20);
}

void ofApp::keyPressed(int key) {
    if (key == 's' || key == 'S') {
        auto& lib = ofkitty::runtime().registry().get<ecs::swatch_library_component>(m_libraryEntity);
        ofxSwatches::saveLibrary(lib, ofToDataPath(lib.libraryName + ".json"));
        ofLogNotice("example") << "Saved swatch library";
    }
}
