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
	
	ofxPvAPI::ParamCamWarp	camera;
		
	ofxPanel gui;
	ofParameter<int> fps;
	
	void keyPressed(int key);
};
