#pragma once

#include "ofMain.h"
#include "ofxProsilica.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp{

public:
	void setup();
	void update();
	void draw();
    void exit();
    
    ofxProsilica::ParameterConnector*	cameras;
    int numCameras;
	
	ofxPanel gui;
    ofParameter<int> fps;
    ofParameter<bool> fullScreen;
    ofParameter<bool> drawAll;
    ofParameter<int> drawSelect;
	void fullScreenLisner(bool &_value) { ofSetFullscreen(_value); }
	
	ofTexture* textures;
	
	void keyPressed(int key);
	
	
		
};
