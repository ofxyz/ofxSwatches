#include "ofMain.h"
#include "ofApp.h"
#include <ofxKit.h>

int main() {
    ofGLWindowSettings settings;
    settings.setSize(1280, 720);
    settings.windowMode = OF_WINDOW;
    settings.title = "ofxSwatches Swatch Panel";

    auto window = ofCreateWindow(settings);
    auto app = std::make_shared<ofApp>();
    ofkitty::Runtime::attach(window, app, app->registry());
    ofRunApp(window, std::move(app));
    ofRunMainLoop();
    return 0;
}
