#include "ofApp.h"

// -----------------------------------------------------------------------
void ofApp::setup() {
    ofSetWindowTitle("ofxSwatches — load / save");
    ofSetFrameRate(60);
    ofBackground(40);

    library = ofxSwatches::SwatchLibrary("Pantone Demo");

    // Try to load a previously saved library from bin/data; fall back to defaults.
    if (!ofxSwatches::loadLibrary(library)) {
        buildDefaultLibrary();
    }
}

// -----------------------------------------------------------------------
void ofApp::buildDefaultLibrary() {
    library.clear();

    // CMYK process colours
    library.addCMYK(  0,   0,   0,   0, "Paper");
    library.addCMYK(100,   0,   0,   0, "Cyan");
    library.addCMYK(  0, 100,   0,   0, "Magenta");
    library.addCMYK(  0,   0, 100,   0, "Yellow");
    library.addCMYK(  0,   0,   0, 100, "Black");

    // RGB colour
    library.addColor(ofColor(255, 87, 34), "Deep Orange");

    // HSB colour — hue/saturation/brightness are 0–255 in openFrameworks
    library.addColor(ofxSwatches::SwatchColor::fromHSB(191, 180, 200, "Violet"));
}

// -----------------------------------------------------------------------
void ofApp::update() {}

// -----------------------------------------------------------------------
void ofApp::draw() {
    constexpr int kSwatchW = 130;
    constexpr int kSwatchH = 80;
    constexpr int kPad     = 10;
    constexpr int kTopY    = 58;
    constexpr int kInfoH   = 22;

    const int cols = std::max(1, (ofGetWidth() - kPad) / (kSwatchW + kPad));

    // Header
    ofSetColor(220);
    ofDrawBitmapString(library.libraryName + "  (" + ofToString(library.count()) + " swatches)", kPad, 18);
    ofSetColor(140);
    ofDrawBitmapString("[s] Save   [l] Load   [r] Reset   [h] Add triadic harmony from first colour", kPad, 36);

    for (int i = 0; i < library.count(); i++) {
        const auto* c = library.getColor(i);
        if (!c) continue;

        const int   col = i % cols;
        const int   row = i / cols;
        const float x   = kPad + col * (kSwatchW + kPad);
        const float y   = kTopY + row * (kSwatchH + kInfoH + kPad);

        // Swatch rectangle
        ofSetColor(c->color);
        ofDrawRectangle(x, y, kSwatchW, kSwatchH);

        // Name label — white or black depending on contrast against the swatch colour
        const float     cr       = ofxSwatches::contrastRatio(c->color, ofColor::white);
        const ofColor   labelCol = (cr >= 3.0f) ? ofColor::white : ofColor::black;
        ofSetColor(labelCol);
        ofDrawBitmapString(c->getDisplayName(), x + 4, y + kSwatchH - 6);

        // Type tag + WCAG grey value below the swatch
        ofSetColor(180);
        const std::string tag = (c->type == ofxSwatches::SwatchColorType::CMYK) ? "CMYK" : "RGB ";
        ofDrawBitmapString(tag + "  L:" + ofToString((int)c->getGreyValuePercent()) + "%",
                           x + 4, y + kSwatchH + 14);
    }
}

// -----------------------------------------------------------------------
void ofApp::keyPressed(int key) {
    if (key == 's') {
        // Saves to bin/data/<libraryName>.json
        if (ofxSwatches::saveLibrary(library)) {
            ofLogNotice("example") << "Saved \"" << library.libraryName << "\" ("
                                   << library.count() << " swatches)";
        }
    } else if (key == 'l') {
        // Loads from bin/data/<libraryName>.json
        ofxSwatches::SwatchLibrary tmp(library.libraryName);
        if (ofxSwatches::loadLibrary(tmp)) {
            library = std::move(tmp);
            ofLogNotice("example") << "Loaded \"" << library.libraryName << "\" ("
                                   << library.count() << " swatches)";
        }
    } else if (key == 'h') {
        // Collect non-neutral swatches and pick one at random.
        std::vector<int> candidates;
        for (int i = 0; i < library.count(); i++) {
            if (!library.getColor(i)->isNeutralGrey()) candidates.push_back(i);
        }
        if (!candidates.empty()) {
            library.generateHarmonyFrom(candidates[ofRandom(candidates.size())],
                                        ofxSwatches::ColorHarmony::Triadic);
        }
    } else if (key == 'r') {
        buildDefaultLibrary();
    }
}

// -----------------------------------------------------------------------
void ofApp::keyReleased(int key)                            {}
void ofApp::mouseMoved(int x, int y)                        {}
void ofApp::mouseDragged(int x, int y, int button)          {}
void ofApp::mousePressed(int x, int y, int button)          {}
void ofApp::mouseReleased(int x, int y, int button)         {}
void ofApp::windowResized(int w, int h)                     {}
void ofApp::gotMessage(ofMessage msg)                       {}
void ofApp::dragEvent(ofDragInfo dragInfo)                  {}
