#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	
	ofSetLogLevel(OF_LOG_VERBOSE);
    ofxProsilica::Connector tempConnector;
	vector<ofVideoDevice> devices= tempConnector.listDevices();
    numCameras = devices.size();
	ofSetLogLevel(OF_LOG_NOTICE);
	
	cameras = new ofxProsilica::ParameterConnector[numCameras];
	textures = new ofTexture[numCameras];
	
    gui.setup("settings");
    gui.add(fps.set("FPS", 0, 0, 100));
    gui.add(fullScreen.set("fullscreen (F)", false));
    gui.add(drawAll.set("draw all", false));
    gui.add(drawSelect.set("draw select ", 0, 0, numCameras));
    fullScreen.addListener(this, &ofApp::fullScreenLisner);
    for (int i=0; i<numCameras; i++) {
        cameras[i].setDeviceID(devices[i].id);
        cameras[i].setup();
        gui.add(cameras[i].parameters);
    }
	gui.loadFromFile("settings.xml");
	gui.minimizeAll();
	
	if (numCameras > 0 && drawSelect == 0) {
        drawSelect = 1;
	}
	
	gui.minimizeAll();
}

//--------------------------------------------------------------
void ofApp::update(){
    for (int i=0; i<numCameras; i++) {
        cameras[i].update();
    }
	fps.set(ofGetFrameRate() + 0.5);
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofClear(128);
    for (int i=0; i<numCameras; i++) {
        if (cameras[i].isFrameNew()){
            int w = cameras[i].getWidth();
            int h = cameras[i].getHeight();
            int glFormat = ofGetGLInternalFormatFromPixelFormat(cameras[i].getPixelFormat());
            
            if (textures[i].getWidth() != w || textures[i].getHeight() != h || textures[i].getTextureData().glInternalFormat != glFormat) {
                textures[i].clear();
                textures[i].allocate(w, h, glFormat);
            }
			
			textures[i].loadData(cameras[i].getData(), w, h, glFormat);
        }
        
    }
    if (drawAll.get()) {
        for (int i=0; i<numCameras; i++) {
            textures[i].draw(gui.getWidth() + gui.getPosition().x + 10 + i * 650, gui.getPosition().y);
        }
    }
    else if (drawSelect > 0)  {
        int i = drawSelect - 1;
        int h = ofGetWindowHeight() - 20;
        float ar = textures[i].getWidth() / textures[i].getHeight();
        int w = h * ar;
        textures[i].draw(gui.getWidth() + gui.getPosition().x + 10, gui.getPosition().y, w, h);
        
    }
	gui.draw();
	
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch (key) {
		case ' ':
            
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
    
    for (int i=0; i<numCameras; i++) {
        cameras[i].close();
    }
}
