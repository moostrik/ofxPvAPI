#pragma once

#include "ofMain.h"
#include "ofxPvApi.h"
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
	
	void keyPressed(int key);
	
	void setIPSettings();
	string persistentIP;
	string persistentSubnet;
	string persistentGateway;
	
	int lGHeight, lCWidth, LCHeight;
};
