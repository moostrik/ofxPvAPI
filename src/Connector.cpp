#include "Connector.h"


namespace ofxProsilica {
	
	bool Connector::setup() {
		return initConnector();
	}
	
	bool Connector::initConnector() {
		connectAttempts = 100;
		connectCounter = 0;
		connectInterval = 2; // in seconds
		
		bAutoConnect = true;
		nextConnectAttemptTime = ofGetElapsedTimef() + connectInterval;
		
		bWasInitialized = false;
		
		connect();
		return true;
	}
	
	void Connector::update(){
		bWasInitialized = bInitialized;
		Camera::update();
		if (!bInitialized)
			autoConnect();
	}
	
	void Connector::autoConnect() {
		if (bAutoConnect) {
			if (connectAttempts !=0 && connectCounter > connectAttempts) {
				bAutoConnect = false;
				ofLogNotice("Connector") << requestedDeviceID << " Stopped Trying to Connect after " << connectCounter << " Times";
			} else {
				if (nextConnectAttemptTime < ofGetElapsedTimef()) {
					connectCounter++;
					connectInterval += connectInterval;
					if (connectInterval > 64) connectInterval = 64;
					nextConnectAttemptTime = ofGetElapsedTimef() + connectInterval;
					
					if (connect()) {
						connectCounter = 0;
						connectInterval = 2;
					} else {
						ofLogNotice("Connector") << requestedDeviceID << " Trying again in " << connectInterval << " seconds";
					}
				}
			}
		}
	}
	
	bool Connector::connect() {
		if (!bInitialized) {
			ofLogNotice("Connector") << requestedDeviceID << " Trying to (re)connect to camera";
			listDevices();
			
			if (Camera::setup()) {
				ofLogVerbose("Connector") << requestedDeviceID << " Connection succeeded";
				return true;
			} else {
				ofLogNotice("Connector") << requestedDeviceID << " Connection failed";
				return false;
			}
		}
	}
	
	void Connector::disconnect() {
		if (bInitialized) {
			Camera::close();
			ofLogNotice("Connector") << requestedDeviceID << " Disconnected";
			connectCounter = 0;
			connectInterval = 2;
		}
	}
	
}
