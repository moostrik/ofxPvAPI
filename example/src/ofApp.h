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
	
	ofxPvAPI::WarpTexPC	camera;
		
	ofxPanel gui;
	ofParameter<int> fps;
	ofParameter<int> camFps;
	ofParameter<int> camFpsInternal;
	ofParameter<bool> fullScreen;
	ofParameter<bool> drawPixels;
	void fullScreenLisner(bool &_value) { ofSetFullscreen(_value); }
	
	ofTexture tex;
	
	void keyPressed(int key);
	
	vector<float> frameTimes;
};
