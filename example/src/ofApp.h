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
	
	ofxPvAPI::ParamCamExt	camera;
		
	ofxPanel gui;
	ofParameter<int> fps;
	ofParameter<bool> drawNewFrameOnly;
	ofParameter<bool> toggleFullScreen;
	
	void toggleFullScreenListener(bool& _value) { ofSetFullscreen(_value); }
	
	void keyPressed(int key);
	
	void setIPSettings();
	string persistentIP;
	string persistentSubnet;
	string persistentGateway;
};
