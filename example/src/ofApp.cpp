#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(30);
	ofSetVerticalSync(true);
	
	ofSetLogLevel(OF_LOG_VERBOSE);
	// wait for PvApi to initiate and for cams to register
	ofSleepMillis(500);
	camera.listDevices();
	ofSetLogLevel(OF_LOG_NOTICE);
	
	//	PRE SETUP FEATURES
	
	//	when deviceID is not provided the camera defaults to first in list (if detected on setup)
	//  select camera by deviceID
	camera.setDeviceID(6055547);
	
	//	select camera by IP
//	camera.setDeviceID(camera.getDeviceIDFromIpAdress("10.0.0.50"));
	
	//	set to color, default is mono (OF_PIXELS_MONO)
//	camera.setPixelFormat(OF_PIXELS_RGB);
	
	//	enable ip settings;
//	camera.enableIPSettings();
	//	use "I", "S" and "G" to set these values:
	persistentIP = "10.0.0.30";
	persistentSubnet = "255.255.0.0";
	persistentGateway = "10.0.0.1";
	
	camera.setup();
	
	gui.setup("settings");
	gui.add(fps.set("FPS", 0, 0, 100));
	gui.add(toggleFullScreen.set("fullScreen (F)", false));
	gui.add(drawNewFrameOnly.set("draw new frame only", false));
	gui.add(camera.getParameters());
	gui.loadFromFile("settings.xml");
	
	toggleFullScreen.addListener(this, &ofApp::toggleFullScreenListener);
	
	// minimize submenus
	for (int i=0; i< gui.getNumControls(); i++) {
		ofxGuiGroup * group  = dynamic_cast<ofxGuiGroup*>(gui.getControl(i));
		if (group) {
			for (int j=0; j< group->getNumControls(); j++) {
				ofxGuiGroup * subgroup = dynamic_cast<ofxGuiGroup*>(group->getControl(j));
				if(subgroup)subgroup->minimizeAll();
			}
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
	
	if (!(!camera.isFrameNew() && drawNewFrameOnly)) {
		if (camera.getTexture().isAllocated()) {
			int x = gui.getWidth() + gui.getPosition().x + 10;
			int y = gui.getPosition().y;
			int w = ofGetWindowWidth() - x - 10;
			int h = w * (camera.getHeight() / camera.getWidth());
			camera.getTexture().draw(x, y, w, h);
		}
	}
	
	gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch (key) {
		case 'I':
			camera.setPersistentIpAdress(persistentIP);
			break;
		case 'S':
			camera.setPersistentIpSubnetMask(persistentSubnet);
			break;
		case 'G':
			camera.setPersistentIpGateway(persistentGateway);
			break;
		case 'F':
			toggleFullScreen.set(!toggleFullScreen.get());
			break;
			
		default:
			break;
	}
}

//--------------------------------------------------------------
void ofApp::exit(){
}
