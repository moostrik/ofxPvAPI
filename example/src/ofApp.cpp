#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	
	ofSetLogLevel(OF_LOG_VERBOSE);
	camera.listDevices();
	ofSetLogLevel(OF_LOG_NOTICE);
	
	//	PRE SETUP FEATURES
	
	//  select camera, default is first in list
	//  select camera by deviceID
//	camera.setDeviceID(6002494);
	
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
	// reshape window to fit camera image
	if (camera.getConnectionChange() && camera.isInitialized())
		ofSetWindowShape(MAX(1024, 30 + gui.getWidth() + camera.getWidth()), MAX(768, 20 + camera.getHeight()) );
	
	camera.update();
	fps.set(ofGetFrameRate() + 0.5);
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofClear(128);
	if (camera.isFrameNew()){
		int w = camera.getWidth();
		int h = camera.getHeight();
		int glFormat = ofGetGLInternalFormatFromPixelFormat(camera.getPixelFormat());
		
		if (tex.getWidth() != w || tex.getHeight() != h || tex.getTextureData().glInternalFormat != glFormat) {
			tex.clear();
			tex.allocate(w, h, glFormat);
		}
		
		tex.loadData(camera.getPixels(), w, h, glFormat);
	}
	if (tex.isAllocated()) {
		tex.draw(gui.getWidth() + gui.getPosition().x + 10, gui.getPosition().y);
	}
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
			
		default:
			break;
	}
}

//--------------------------------------------------------------
void ofApp::exit(){
    camera.close();
}
