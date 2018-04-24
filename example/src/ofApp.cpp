#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(60);
	ofSetVerticalSync(true);
	
	ofSetLogLevel(OF_LOG_VERBOSE);
	// wait for PvApi to initiate and for cams to register
	ofSleepMillis(500);
	camera.listDevices();
	ofSetLogLevel(OF_LOG_NOTICE);
	
	//	PRE SETUP FEATURES
	
	//	when deviceID is not provided the camera defaults to first in list (if detected on setup)
	//  select camera by deviceID
	camera.setDeviceID(6022891);
	
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
	gui.add(drawNewFrameOnly.set("draw new frame only", false));
	gui.add(camera.parameters);
	gui.loadFromFile("settings.xml");
	
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
	
	lGHeight = 0;
	lCWidth = 0;
	LCHeight = 0;
}

//--------------------------------------------------------------
void ofApp::update(){
	camera.update();
	
	fps.set(ofGetFrameRate() + 0.5);
	
	// reshape window to fit camera image and gui
	if (lGHeight != gui.getHeight() || lCWidth != camera.getWidth() || LCHeight != camera.getHeight()) {
		lGHeight = gui.getHeight();
		lCWidth = camera.getWidth();
		LCHeight = camera.getHeight();
		ofSetWindowShape(30 + gui.getWidth() + max(640.f, camera.getWidth()), 20 + max(gui.getHeight(), camera.getHeight()));
	}
	
	
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofClear(128);
	
	if (!(!camera.isFrameNew() && drawNewFrameOnly)) {
		if (camera.getTexture().isAllocated()) {
			camera.getTexture().draw(gui.getWidth() + gui.getPosition().x + 10, gui.getPosition().y);
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
			
		default:
			break;
	}
}

//--------------------------------------------------------------
void ofApp::exit(){
}
