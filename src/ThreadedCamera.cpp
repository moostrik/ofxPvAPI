
#include "ThreadedCamera.h"

namespace ofxPvAPI {

	void T_FrameDoneCB(tPvFrame* pFrame){
		ThreadedCamera* cam = (ThreadedCamera*)pFrame->Context[0];
		if(cam) {cam->onFrameDone(pFrame); }
	}
	
	ThreadedCamera::ThreadedCamera(){
		
		if (!bPvApiInitiated) PvApiInitialize() ;
		
		// to allocateFrame
		pvFrames = new tPvFrame[numPvFrames];
		if (pvFrames) { memset(pvFrames,0,sizeof(tPvFrame) * numPvFrames); }
		
		for (int i=0; i<numPvFrames; i++) {
			pvFrames[i].Context[0] = this;
			pvFrames[i].Context[1] = new int(i);
			pvFrames[i].Context[2] = new float(0);
		}
	}
	
	ThreadedCamera::~ThreadedCamera(){
		// waitforT
		close();
		if (numCamerasInUse == 0 && bPvApiInitiated)
		PvApiUnInitialize();
	}


	//----------------------------------------------------------------------------
	//-- OF ----------------------------------------------------------------------

	bool ThreadedCamera::setup() {

		
		pvFramesThreaded = new tPvFrame[numPvFrames];
		if (pvFramesThreaded) { memset(pvFramesThreaded,0,sizeof(tPvFrame) * numPvFrames); }
		
		for (int i=0; i<numPvFrames; i++) {
			pvFramesThreaded[i].Context[0] = this;
			pvFramesThreaded[i].Context[1] = new int(i);
			pvFramesThreaded[i].Context[2] = new float(0);
		}
		
		pvFrameIterator = 0;
		rotate90 = true;

		
		vector<ofVideoDevice> deviceList = listDevices();
		if (deviceList.size() == 0) {
			ofLog(OF_LOG_WARNING, "ThreadedCamera: %lu no cameras found", deviceID);
			return false;
		}
		
		if (requestedDeviceID == 0) {
			bool foundAvailableCamera = false;
			for (int i=0; i<deviceList.size(); i++) {
				if (deviceList[i].bAvailable) {
					requestedDeviceID = deviceList[i].id;
					foundAvailableCamera = true;
					break;
				}
			}
			if (foundAvailableCamera){
				ofLog(OF_LOG_NOTICE, "ThreadedCamera: no camera ID specified, defaulting to camera %lu", requestedDeviceID);
			}
			else {
				ofLog(OF_LOG_WARNING, "ThreadedCamera: found no camera available ");
				return false;
			}
		}
		
		bool requestedDeviceFound = false;
		bool requestedDeviceAvailable = false;
		for (int i=0; i<deviceList.size(); i++) {
			if (requestedDeviceID == deviceList[i].id) {
				requestedDeviceFound = true;
				if (deviceList[i].bAvailable) {
					requestedDeviceAvailable = true;
					ofLog(OF_LOG_VERBOSE, "ThreadedCamera: %lu found", requestedDeviceID);
				}
			}
		}
		if (!requestedDeviceFound) {
			ofLog(OF_LOG_WARNING, "ThreadedCamera: %lu not found", requestedDeviceID);
			return false;
		}
		if (!requestedDeviceAvailable) {
			ofLog(OF_LOG_WARNING, "ThreadedCamera: %lu not available", requestedDeviceID);
			return false;
		}
		
		startThread();
		return initCamera(requestedDeviceID);
	}

	void ThreadedCamera::close(){

		if( bInitialized ) {
			
			camCondition.notify_one();
			waitForThread();

			Camera::close();
		}
	}


	void ThreadedCamera::onFrameDone(tPvFrame* _frame) {

		if (_frame->Status == ePvErrSuccess) {
			PvCaptureQueueFrame(cameraHandle, _frame, T_FrameDoneCB);
			float& time = *(float*)_frame->Context[2];
			time = ofGetElapsedTimef();
			
			ofxPvAPIMessage message;
			message.type = OFX_PVAPI_FRAME;
			message.frame = _frame;
			
			enqueue(message);
		}
		else if (_frame->Status != ePvErrCancelled) {
			logError(_frame->Status);
			PvCaptureQueueFrame(cameraHandle, _frame, T_FrameDoneCB);
		}
		else {
			logError(_frame->Status);
		}
	}

	void ThreadedCamera::threadedFunction() {
		while(isThreadRunning()) {
			if(messageQueue.empty()) {
				{
					std::unique_lock<std::mutex> lock(camMutex);
					camCondition.wait(lock, [this]() {
						return !isThreadRunning() || !messageQueue.empty();
					});
				}
			}
			
			dequeue();
		}
	}
	
	void ThreadedCamera::enqueue(ofxPvAPIMessage message) {
		if(!bInitialized) return;
		
		
		std::unique_lock<std::mutex> lock(camMutex);
		messageQueue.push(message);
		camCondition.notify_one();
	}
	
	void ThreadedCamera::dequeue() {
		if(messageQueue.empty()) return;
		
		ofxPvAPIMessage message = messageQueue.front();
		if(message.type == OFX_PVAPI_FRAME) {
			processFrame(message.frame);
		} else if(message.type == OFX_PVAPI_SETROIX) {
			processResize(0, message.value);
		} else if(message.type == OFX_PVAPI_SETROIY) {
			processResize(1, message.value);
		}
		
		messageQueue.pop();
	}
	
	void ThreadedCamera::processResize(bool _XorY, int _value) {
		
		clearQueue();
		stopAcquisition();		// stop Aq and Cap to make width and height register correctly
		stopCapture();
		if (_XorY) { Camera::setROIY(_value); }
		else  { Camera::setROIX(_value); }
		
		allocateFrames();
		startCapture();
		startAcquisition();
		queueFrames();
	}
	
	void ThreadedCamera::processFrame(tPvFrame* _frame) {
		
		if (_frame->Status != ePvErrSuccess) {
			ofLogWarning("ThreadedCamera: processFrame") << " something went wrong state is not ePvErrSuccess";
			return;
		}
		
		pvFrameIterator = (pvFrameIterator + 1) % numPvFrames;


		tPvFrame* resizeFrame = &pvFramesThreaded[pvFrameIterator];

		if (flipV, flipH, rotate90) {
			ofPixels resizePixels;
			resizePixels.setFromPixels((unsigned char *)_frame->ImageBuffer, _frame->Width, _frame->Height, pixelFormat);

			if (resizePixels.isAllocated()) {
				resizePixels.mirror(flipV, flipH);
				resizePixels.rotate90(rotate90);
			}

//			delete (char*)resizeFrame->ImageBuffer;
//			resizeFrame->ImageBuffer = new char[resizePixels.getTotalBytes()];
			resizeFrame->ImageBuffer = resizePixels.getData();


			resizeFrame->Width = resizePixels.getWidth();
			resizeFrame->Height = resizePixels.getHeight();
			resizeFrame->ImageBufferSize = resizePixels.getTotalBytes();
			resizeFrame->Context[2] = _frame->Context[2];
//			resizeFrame->Status = ePvErrSuccess;
		}
		else {
			resizeFrame = _frame;
		}
	
		capuredFrameQueue.push_front(resizeFrame);
	}
	
	
	bool ThreadedCamera::initCamera(int _cameraUid){
		
		deviceID = _cameraUid;
		
		// todo: better handling of failures
		// for exaple: a failure on starting aquisition leaves camera open.
		
		if (!openCamera()) return false;
		if (!setPacketSizeToMax()) return false;
		if (!allocateFrames()) return false;
		if (!startCapture()) return false;
		if (fixedRate) { if (!setEnumAttribute("FrameStartTriggerMode","FixedRate")) return false; }
		else { if (!setEnumAttribute("FrameStartTriggerMode","Software")) return false; }
		if (!setEnumAttribute("AcquisitionMode", "Continuous")) return false;
		if (!setEnumAttribute("PixelFormat", getPvPixelFormat(pixelFormat))) return false;
		if (!startAcquisition()) return false;
		
		
		bInitialized = true;
		triggerFrame();
		numCamerasInUse++;
		ofLog(OF_LOG_NOTICE,"ThreadedCamera: %lu up and running", deviceID);
		
		queueFrames();
		return true;
	}
	
	bool ThreadedCamera::queueFrames(){
		for (int i=0; i<numPvFrames; i++) {
			tPvErr error = PvCaptureQueueFrame( cameraHandle, &pvFrames[i], T_FrameDoneCB);
			if (error != ePvErrSuccess ){
				ofLog(OF_LOG_NOTICE, "ThreadedCamera: " + ofToString(deviceID) + " failed to queue frame " + ofToString(i));
				logError(error);
				return false;
			}
		}
		ofLog(OF_LOG_VERBOSE, "ThreadedCamera: " + ofToString(deviceID) + " frames queued");
		return true;
	}

	//----------------------------------------------------------------------------
	//-- ATTRIBUTES (ROI) --------------------------------------------------------

	void ThreadedCamera::setROIWidth(int _value) {
		if(!Camera::isInitialized()) return;
		
		ofxPvAPIMessage message;
		message.type = OFX_PVAPI_SETROIX;
		message.value = _value;
		
		enqueue(message);
	}

	void ThreadedCamera::setROIHeight(int _value) {
		if(!Camera::isInitialized()) return;
		
		ofxPvAPIMessage message;
		message.type = OFX_PVAPI_SETROIY;
		message.value = _value;
		
		enqueue(message);
	}

}


