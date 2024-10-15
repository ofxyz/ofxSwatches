#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){
	swatches.addCMYK({   0,  0,  0,  0 }, "Paper");
	swatches.addCMYK({ 100,  0,  0,  0 }, "Cyan");
	swatches.addCMYK({   0,100,  0,  0 }, "Magenta");
	swatches.addCMYK({   0,  0,100,  0 }, "Yellow");
	swatches.addCMYK({   0,  0,  0,100 }, "Keycolor");

	ofBackground(swatches.getColor("Paper"));
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

	ofDrawBitmapString("Press [s] to save the swatches.", 25, 25);
	
	for (int i = 0; i < swatches.count(); i++) {
		ofSetColor(swatches.getColor(i));
		ofDrawRectangle(100 + (i * 100), 100, 100, 100);
	}

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == 's') {
		swatches.saveSettings();
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
