#pragma once

#include "ofMain.h"
#include "ofxPvAPI.h"
#include "ofxGui.h"


class ofApp : public ofBaseApp{

public:
	void setup();
	void update();
    void draw();
    void exit();
	
	ofxPvAPI::ParamCam	camera;
		
	ofxPanel gui;
	ofParameter<int> fps;
	ofParameter<bool> fullScreen;
	ofParameter<bool> drawPixels;
	void fullScreenLisner(bool &_value) { ofSetFullscreen(_value); }
	
	ofTexture tex;
	
	void keyPressed(int key);
};
