#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	
	ofSetLogLevel(OF_LOG_VERBOSE);
	camera.listDevices();
	ofSetLogLevel(OF_LOG_NOTICE);
	
	//	set size for warp output
	camera.setup(512,512);
	
	gui.setup("settings");
	gui.add(fps.set("FPS", 0, 0, 100));
	gui.add(fullScreen.set("fullscreen (F)", false));
	fullScreen.addListener(this, &ofApp::fullScreenLisner);
	gui.add(camera.parameters);
	gui.loadFromFile("settings.xml");
	
	for (int i=0; i< gui.getNumControls(); i++) {
		ofxGuiGroup * group  = dynamic_cast<ofxGuiGroup*>(gui.getControl(i));
		if (group) {
			group->minimizeAll();
		}
	}
}

//--------------------------------------------------------------
void ofApp::update(){
	camera.update();
	fps.set(ofGetFrameRate() + 0.5);
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofClear(128);
	
	int startW = gui.getWidth() + gui.getPosition().x + 10;
	float tW = camera.getTexture().getWidth();
	float tH = camera.getTexture().getHeight();
	float tAr = tW / tH;
	float wW = camera.getWarpWidth();
	float wH = camera.getWarpHeight();
	float wAr = wW / wH;
	float scale = ofGetWindowHeight() / (tH + wH + 30);
	
	int x,y,w,h;
	
	x = gui.getWidth() + gui.getPosition().x + 10;
	y = gui.getPosition().y;
	w = tW * scale;
	h = tH * scale;
	
	camera.getTexture().draw(x,y,w,h);
	
	ofPushMatrix();
	ofTranslate(x, y);
	ofScale(w, h);
	ofPushStyle();
	ofSetColor(64, 0, 0);
	ofSetLineWidth(2);
	camera.getWarpLine().draw();
	ofSetColor(255, 255, 255);
	ofSetLineWidth(1);
	camera.getWarpLine().draw();
	ofPopStyle();
	ofPopMatrix();
	
	y += h + 10;
	w = wW * scale;
	h = wH * scale;
	
	camera.getWarpedTexture().draw(x,y,w,h);
	
	gui.draw();
	
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch (key) {
		case 'f':
		case 'F':
			fullScreen.set(1-fullScreen.get());
			break;
			
		default:
			break;
	}
}

//--------------------------------------------------------------
void ofApp::exit(){
    camera.close();
}
