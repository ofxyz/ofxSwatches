#pragma once

#include "ofMain.h"
#include "ofxKit.h"
#include "ofxSwatchesAll.h"
#include "components/swatch_components.h"
#include <entt/entt.hpp>

class ofApp : public ofBaseApp {
public:
    void setup() override;
    void update() override;
    void draw() override;
    void keyPressed(int key) override;

    /// Owned registry — must be the same instance passed to ofkitty::Runtime::attach().
    entt::registry& registry() { return m_registry; }

private:
    entt::registry m_registry;
    entt::entity m_libraryEntity = entt::null;
    ofxSwatches::SwatchesPanel m_panel;
    bool m_showSwatches = true;
};
