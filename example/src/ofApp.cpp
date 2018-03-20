#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(60);
	ofSetVerticalSync(true);
	
	ofSetLogLevel(OF_LOG_VERBOSE);
	camera.listDevices();
	ofSetLogLevel(OF_LOG_NOTICE);
	
	//	PRE SETUP FEATURES
	
	//  select camera, default is first in list
	//  select camera by deviceID
	camera.setDeviceID(6022891);
	
	//	select camera by IP
//	int deviceID = camera.getDeviceIDFromIpAdress("10.0.0.50");
//	camera.setDeviceID(deviceID);
	
	//	set to mono or color, default is OF_PIXELS_MONO
//	camera.setPixelFormat(OF_PIXELS_MONO);
//	camera.setPixelFormat(OF_PIXELS_RGB);
	
	camera.setup();
	
	gui.setup("settings");
	gui.add(fps.set("FPS", 0, 0, 100));
	gui.add(fullScreen.set("fullscreen (F)", false));
	fullScreen.addListener(this, &ofApp::fullScreenLisner);
	gui.add(drawPixels.set("draw pixels (D)", false));
	gui.add(camera.parameters);
	gui.loadFromFile("settings.xml");
	
//	gui.minimizeAll();
	// instead little hack to minimize 'auto' parameters
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
	// reshape window to fit camera image
	
	ofSetWindowShape(30 + gui.getWidth() + max(640.f, camera.getWidth()), 20 + max(gui.getHeight(), camera.getHeight()));
	fps.set(ofGetFrameRate() + 0.5);
	
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofClear(128);
	
//	if (drawPixels.get()) {
	if (camera.isFrameNew()) {
		ofPixels& pix = camera.getPixels();
		ofImage image;
		image.setFromPixels(pix);
		image.draw(gui.getWidth() + gui.getPosition().x + 10, gui.getPosition().y);
	}
//	}
//	else {
//		if (camera.getTexture().isAllocated()) {
//			camera.getTexture().draw(gui.getWidth() + gui.getPosition().x + 10, gui.getPosition().y);
//		}
//	}
	gui.draw();
	
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch (key) {
		case ' ':
			// BE CAREFULL USING THESE
//			camera.setPersistentIpAdress("10.0.0.50");
//			camera.setPersistentIpSubnetMask("255.255.0.0");
//			camera.setPersistentIpGateway("10.0.0.60");
//			camera.setPersistentIp(true);
			break;
		case 'f':
		case 'F':
			fullScreen.set(1-fullScreen.get());
			break;
		case 'd':
		case 'D':
			drawPixels.set(1-drawPixels.get());
			break;
			
		default:
			break;
	}
}

//--------------------------------------------------------------
void ofApp::exit(){
    camera.destroy();
}
