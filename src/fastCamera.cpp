
#include "fastCamera.h"

namespace ofxPvAPI {
	
	void fastCamera::update() {
		bIsFrameNew = false;
		
		if (!bDeviceActive) {
			waitForDeviceToBecomeAvailable();
		}
		else {
			tPvFrame* frame = &pvFrames[0];
			tPvErr error = PvCaptureWaitForFrameDone(deviceHandle, frame, 1); // in MiliSeconds
			if( error == ePvErrSuccess ){
				pixels.setFromExternalPixels((unsigned char *)frame->ImageBuffer, frame->Width, frame->Height, pixelFormat);
				bIsFrameNew = true;
				bTextureSet = false;
				queueFrames();
			}
			else if( error != ePvErrTimeout ) {
				logError(error);
				queueFrames();
			}
		}
	}
	
	bool fastCamera::queueFrames(){
		tPvErr error = PvCaptureQueueFrame( deviceHandle, &pvFrames[0], NULL);
		if (error != ePvErrSuccess ){
			ofLog(OF_LOG_NOTICE, "Camera: " + ofToString(deviceID) + " failed to queue frame ");
			logError(error);
			return false;
		}
		ofLog(OF_LOG_VERBOSE, "Camera: " + ofToString(deviceID) + " frames queued");
		return true;
	}
}



