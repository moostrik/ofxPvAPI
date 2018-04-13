
#include "Camera.h"

namespace ofxPvAPI {
	
	Camera::Camera() :
	bFramesAllocated(false),
	bDeviceActive(false),
	bWaitForDeviceToBecomeAvailable(false),
	waitInterval(2000),
	lastWaitTime(0),
	bIsFrameNew(false),
	bTextureSet(false),
	fps(0),
	frameDrop(0),
	bTriggered(0),
	frameLatency(0),
	frameAvgLatency(0),
	frameMaxLatency(0),
	frameMinLatency(0),
	pixelFormat(OF_PIXELS_MONO),
	deviceID(0),
	requestedDeviceID(0),
	persistentIpAdress(""),
	persistentIpGateway(""),
	persistentIpSubnetMask(""){
		if (!bPvApiInitiated) PvApiInitialize() ;
	}
	
	Camera::~Camera(){
		if( bDeviceActive ) {
			deactivate();
		}
		if (numActiveDevices == 0 && bPvApiInitiated) {	PvApiUnInitialize(); }
	}
	
	
	//----------------------------------------------------------------------------
	//-- API ---------------------------------------------------------------------
	
	bool Camera::bPvApiInitiated = false;
	
	void Camera::PvApiInitialize() {
		
		if( bPvApiInitiated ){
			ofLog(OF_LOG_VERBOSE, "PvAPI already initialized");
		}
		else {
			tPvErr error = PvInitialize();
			if( error == ePvErrSuccess ) {
//				ofSleepMillis(500); // wait for cams to register
				ofLog(OF_LOG_NOTICE, "Camera: PvAPI initialized");
				
			} else {
				ofLog(OF_LOG_ERROR, "Camera: unable to initialize PvAPI");
				logError(error);
				return;
			}
			bPvApiInitiated = true;
		}
		
	}
	
	void Camera::PvApiUnInitialize() {
		if (numActiveDevices == 0 && bPvApiInitiated) {
			PvUnInitialize();
			bPvApiInitiated = false;
			ofLog(OF_LOG_NOTICE, "Camera: PvAPI uninitialized");
		}
	}
	
	
	//----------------------------------------------------------------------------
	//-- OF ----------------------------------------------------------------------
	
	bool Camera::setup() {
		
		if (requestedDeviceID == 0 && numActiveDevices == 0) { // default first camera only
			requestedDeviceID = getFirstAvailableDeviceID();
			if (requestedDeviceID != 0) {
				deviceID = requestedDeviceID;
				ofLog(OF_LOG_NOTICE, "Camera: no camera ID specified, defaulting to camera %lu", deviceID);
			}
		}
		if (isDeviceFound(requestedDeviceID)) {
			deviceID = requestedDeviceID;
			activate();
		}
		
		PvLinkCallbackRegister(plugCallBack, ePvLinkAdd, this);
		PvLinkCallbackRegister(plugCallBack, ePvLinkRemove, this);
	}
	
	void Camera::update() {
		waitOnAvailable();
		
		bIsFrameNew = false;
		
		if (bDeviceActive) {
			if (bTriggered) { triggerFrame(); }
			if ( capuredFrameQueue.size() > 0) {
				size_t frameoffset = (bTriggered)? 0 : 1;
				frameoffset = min(capuredFrameQueue.size(), frameoffset);
				tPvFrame& frame = *capuredFrameQueue[frameoffset];
				float time = ofGetElapsedTimef();
				if (frame.Status == ePvErrSuccess) {
					pixels.setFromExternalPixels((unsigned char *)frame.ImageBuffer, frame.Width, frame.Height, pixelFormat);
					bIsFrameNew = true;
					bTextureSet = false;
					
					float frameTime = *(float*)frame.Context[2];
					timedInt timedLatency;
					timedLatency.time = time;
					frameLatency = (time - frameTime) * 1000;
					timedLatency.value = frameLatency;
					frameRateAndLatencies.push_back(timedLatency);
				}
				else {
					logError(frame.Status);
				}
				capuredFrameQueue.pop_back();
				
				timedInt timedDrops;
				timedDrops.time = time;
				while (capuredFrameQueue.size() > frameoffset) {
					capuredFrameQueue.pop_back();
					timedDrops.value++;
				}
				framesDropped.push_back(timedDrops);
			}
		
			float magicTimeWindow = ofGetElapsedTimef() - (1 - (1.0 / max(fps, 1) / 3.0)); // a little less as one
			
			while (frameRateAndLatencies.size() > 0 && frameRateAndLatencies[0].time < magicTimeWindow) {
				frameRateAndLatencies.pop_front();
			}
			fps = frameRateAndLatencies.size();
			
			frameMaxLatency = 0;
			frameMinLatency = 10000;
			float tL = 0;
			for (int i=0; i<frameRateAndLatencies.size(); i++) {
				tL += frameRateAndLatencies[i].value;
				frameMaxLatency = max(frameMaxLatency, frameRateAndLatencies[i].value);
				frameMinLatency = min(frameMinLatency, frameRateAndLatencies[i].value);
			}
			frameAvgLatency = (tL / frameRateAndLatencies.size());
			
			while (framesDropped.size() > 0 && framesDropped[0].time < magicTimeWindow) { framesDropped.pop_front(); }
			frameDrop = 0;
			for (int i=0; i<framesDropped.size(); i++) { frameDrop += framesDropped[i].value; }
			
		}
	}
	
	
	//----------------------------------------------------------------------------
	//-- DEVICES -----------------------------------------------------------------
	
	int Camera::numActiveDevices = 0;
	
	
	vector<ofVideoDevice> Camera::listDevices(){
		
		vector <ofVideoDevice> devices;
		
		tPvUint32 count, connected;
		unsigned long cameraCount = PvCameraCount();;
		tPvCameraInfoEx* devList = new tPvCameraInfoEx[cameraCount];
		
		
		count = PvCameraListEx( devList, cameraCount, &connected, sizeof(tPvCameraInfoEx) );
		
		if( connected > cameraCount ) {
			ofLog(OF_LOG_VERBOSE, "Camera: more cameras connected than will be listed");
		}
		
		ofLog(OF_LOG_VERBOSE, "Camera: listing available capture devices:");
		for(int i = 0; i < count; i++){
			ofVideoDevice vd;
			vd.id = devList[i].UniqueId;
			vd.deviceName = devList[i].CameraName;
			vd.hardwareName = devList[i].ModelName;
			vd.bAvailable = devList[i].PermittedAccess > 2;
			devices.push_back(vd);
			
			ofLog(OF_LOG_VERBOSE, "%i: %s | model: %s | id: %d | available: %d", i, vd.deviceName.c_str(), vd.hardwareName.c_str(), vd.id, vd.bAvailable);
		}
		
		if (cameraCount == 0)
		ofLog(OF_LOG_VERBOSE, "Camera: no cameras found");
		
		delete[] devList;
		return devices;
	}
	
	bool Camera::isDeviceFound(int _deviceID) {
		vector<ofVideoDevice> deviceList = listDevices();
		bool requestedDeviceFound = false;
		for (int i=0; i<deviceList.size(); i++) {
			if (requestedDeviceID == deviceList[i].id) {
				requestedDeviceFound = true;
				ofLog(OF_LOG_VERBOSE, "Camera: %lu found", requestedDeviceID);
			} else {
				ofLog(OF_LOG_NOTICE, "Camera: %lu not found", requestedDeviceID);
			}
		}
		return requestedDeviceFound;
	}
	
	bool Camera::isDeviceAvailable(int _deviceID) {
		vector<ofVideoDevice> deviceList = listDevices();
		bool requestedDeviceFound = false;
		bool requestedDeviceAvailable = false;
		for (int i=0; i<deviceList.size(); i++) {
			if (requestedDeviceID == deviceList[i].id) {
				requestedDeviceFound = true;
				if (deviceList[i].bAvailable) {
					requestedDeviceAvailable = true;
					ofLog(OF_LOG_VERBOSE, "Camera: %lu found and available", requestedDeviceID);
				} else {
					ofLog(OF_LOG_VERBOSE, "Camera: %lu found, but not available", requestedDeviceID);
				}
			}else {
				ofLog(OF_LOG_VERBOSE, "Camera: %lu not found, hence not available", requestedDeviceID);
			}
		}
		return requestedDeviceFound && requestedDeviceAvailable;
	}
	
	int Camera::getFirstAvailableDeviceID() {
		vector<ofVideoDevice> deviceList = listDevices();
		for (int i=0; i<deviceList.size(); i++) {
			if (deviceList[i].bAvailable) {
				return deviceList[i].id;
			}
		}
		return 0;
	}
	
	
	int Camera::getDeviceIDFromIpAdress(string _IPAdress) {
		deviceID = 0;
		
		tPvCameraInfoEx pInfo;
		tPvIpSettings pIpSettings;
		
		tPvErr error = PvCameraInfoByAddrEx(IPStringToLong(_IPAdress),
											&pInfo,
											&pIpSettings,
											sizeof(pInfo));
		
		if (error == ePvErrSuccess) {
			deviceID = pInfo.UniqueId;
			ofLog(OF_LOG_NOTICE, "Camera: found camera %lu on IP adress %s", deviceID, _IPAdress.c_str());
		}
		else {
			ofLog(OF_LOG_WARNING, "Camera: no camera found on IP adress %s", _IPAdress.c_str());
			logError(error);
		}
		return deviceID;
	}
	
	void Camera::requestDeviceByID(int _deviceID) {
		if (bDeviceActive) {
			ofLog(OF_LOG_ERROR, "Camera: %lu: can not set ID while device is active", deviceID);
		}
		else {
			requestedDeviceID = _deviceID;
		}
	}
	
	
	void Camera::activate() {
		
		if (bDeviceActive) {
			ofLog(OF_LOG_WARNING,"Camera: %lu already active", deviceID);
			return;
		}
		
		if (!isDeviceFound(deviceID)) {
			ofLog(OF_LOG_NOTICE,"Camera: %lu not found", deviceID);
			return;
		}
		
		if (!isDeviceAvailable(deviceID)) {
			bWaitForDeviceToBecomeAvailable = true;
			lastWaitTime = ofGetElapsedTimeMillis();
			ofLog(OF_LOG_NOTICE,"Camera: %lu not available, waiting...", deviceID);
			return;
		}
		
		if (!openCamera()) {
			return;
		}
		if (!setEnumAttribute("PixelFormat", getPvPixelFormat(pixelFormat))) {
			ofLog(OF_LOG_NOTICE,"Camera: %lu PixelFormat not supported", deviceID);
			closeCamera();
			return;
		}
		
		setPacketSizeToMax();
		if (bTriggered) { setEnumAttribute("FrameStartTriggerMode","Software"); }
		else { setEnumAttribute("FrameStartTriggerMode","FixedRate"); }
		setEnumAttribute("AcquisitionMode", "Continuous");
		
		allocateFrames();
		
		startCapture();
		startAcquisition();
		queueFrames();
		
		numActiveDevices++;
		bDeviceActive = true;
		
		ofLog(OF_LOG_NOTICE,"Camera: %lu activated", deviceID);
	}
	
	void Camera::deactivate() {
		
		if (bDeviceActive) {
			bDeviceActive = false;
			clearQueue();
			//		stopAcquisition(); // why ommit?
			stopCapture();
			closeCamera();
			
			deallocateFrames();
			numActiveDevices--;
			
			if (texture.isAllocated()) {
				texture.clear();
			}
			
			fps = 0;
			frameDrop = 0;
			frameLatency = 0;
			frameAvgLatency = 0;
			frameMinLatency = 0;
			frameMaxLatency = 0;
			
			framesDropped.clear();
			frameRateAndLatencies.clear();
		}
		else {
			ofLog(OF_LOG_NOTICE,"Camera: %lu cant't deactivate, as it is not activated", deviceID);
		}
	}
	
	
	void Camera::plugCallBack(void* Context, tPvInterface Interface, tPvLinkEvent Event, unsigned long UniqueId) {
		Camera* cam = (Camera*)Context;
		if (Event == ePvLinkAdd) {
			cam->plugCamera(UniqueId);
		} else if (Event == ePvLinkRemove) {
			cam->unplugCamera(UniqueId);
		}
	}
	
	void Camera::plugCamera(unsigned long _cameraUid) {
		
		cout << "plug " << _cameraUid << endl;
		if (requestedDeviceID == 0  && numActiveDevices == 0) {
			ofLog(OF_LOG_NOTICE, "Camera: no camera ID specified, defaulting to camera %lu", _cameraUid);
			requestedDeviceID = _cameraUid;
		}
		
		if (requestedDeviceID != _cameraUid) {
			return;
		}
		
		ofLog(OF_LOG_NOTICE, "Camera: %lu plugged in", requestedDeviceID);
		deviceID = _cameraUid;
		
		activate();
	}
	
	void Camera::unplugCamera(unsigned long cameraUid) {
		if (cameraUid == deviceID) {
			ofLog(OF_LOG_NOTICE, "Camera: %lu unplugged", requestedDeviceID);
			if (bDeviceActive) {
				deactivate();
			}
		}
	}
	
	void Camera::waitOnAvailable() {
		
		if (bWaitForDeviceToBecomeAvailable && lastWaitTime + waitInterval < ofGetElapsedTimeMillis()) {
			if (!isDeviceFound(deviceID)) {
				bWaitForDeviceToBecomeAvailable = false;
				ofLog(OF_LOG_NOTICE,"Camera: %lu waiting, but not longer found", deviceID);
				return;
			}
			if (isDeviceAvailable(deviceID)) {
				ofLog(OF_LOG_NOTICE,"Camera: %lu available", deviceID);
				activate();
				bWaitForDeviceToBecomeAvailable = false;
				return;
			}
			
			lastWaitTime = ofGetElapsedTimeMillis();
			ofLog(OF_LOG_NOTICE,"Camera: %lu still waiting...", deviceID);
		}
	}
	
	
	bool Camera::openCamera() {
		tPvErr error = PvCameraOpen( deviceID, ePvAccessMaster, &deviceHandle );
		if( error == ePvErrSuccess ){
			ofLog(OF_LOG_VERBOSE, "Camera: %lu opened", deviceID);
			return true;
		} else {
			ofLog(OF_LOG_ERROR, "Camera: %lu failed to open", deviceID);
			logError(error);
			return false;
		}
	}
	
	bool Camera::closeCamera() {
		tPvErr error = PvCameraClose(deviceHandle);
		if( error == ePvErrSuccess ){
			ofLog(OF_LOG_VERBOSE, "Camera: %lu camera closed", deviceID);
			return true;
		} else {
			ofLog(OF_LOG_ERROR, "Camera: %lu failed to close", deviceID);
			logError(error);
			return false;
		}
	}
	
	
	bool Camera::startCapture() {
		
		tPvErr error = PvCaptureStart(deviceHandle);
		if( error == ePvErrSuccess ){
			ofLog(OF_LOG_VERBOSE, "Camera: %lu set to capture mode", deviceID);
			return true;
		} else {
			logError(error);
			ofLog(OF_LOG_ERROR, "Camera: %lu not set to capture mode", deviceID);
			return false;
		}
	}
	
	bool Camera::stopCapture() {
		tPvErr error = PvCaptureEnd(deviceHandle);
		if( error == ePvErrSuccess ){
			ofLog(OF_LOG_VERBOSE, "Camera: %lu stopped capture mode", deviceID);
			return true;
		} else {
			ofLog(OF_LOG_ERROR, "Camera: %lu stopped capture mode", deviceID);
			logError(error);
			return false;
		}
	}
	
	
	bool Camera::startAcquisition() {
		tPvErr error = PvCommandRun(deviceHandle,"AcquisitionStart");
		if( error == ePvErrSuccess ){
			ofLog(OF_LOG_VERBOSE, "Camera: %lu acquisition started", deviceID);
			return true;
		} else {
			ofLog(OF_LOG_ERROR, "Camera: %lu can not start acquisition", deviceID);
			logError(error);
			return false;
		}
	}
	
	bool Camera::stopAcquisition() {
		
		tPvErr error = PvCommandRun(deviceHandle,"AcquisitionStop");
		if( error == ePvErrSuccess ){
			ofLog(OF_LOG_VERBOSE, "Camera: %lu continuous acquisition stopped", deviceID);
			return true;
		} else {
			ofLog(OF_LOG_ERROR, "Camera: %lu can not stop continuous acquisition", deviceID);
			logError(error);
			return false;
		}
	}
	
	bool Camera::abortAcquisition() {
		
		tPvErr error = PvCommandRun(deviceHandle,"AcquisitionAbort");
		if( error == ePvErrSuccess ){
			ofLog(OF_LOG_VERBOSE, "Camera: %lu continuous acquisition aborted", deviceID);
			return true;
		} else {
			ofLog(OF_LOG_ERROR, "Camera: %lu can not abort continuous acquisition", deviceID);
			logError(error);
			return false;
		}
	}
	
	bool Camera::setPacketSizeToMax() {
		tPvErr error = PvCaptureAdjustPacketSize(deviceHandle, getIntAttributeMax("PacketSize"));
		
		if( error == ePvErrSuccess ){
			ofLog(OF_LOG_VERBOSE, "Camera: %lu packet size set to %i", deviceID, getIntAttribute("PacketSize"));
			return true;
		} else {
			ofLog(OF_LOG_ERROR, "Camera: %lu packet is not set", deviceID);
			logError(error);
			return false;
		}
	}
	
	
	//----------------------------------------------------------------------------
	//-- PV FRAMES ---------------------------------------------------------------
	
	int Camera::numPvFrames = 4;
	
	void Camera::allocateFrames() {
		if (!bFramesAllocated) {
			pvFrames = new tPvFrame[numPvFrames];
			if (pvFrames) { memset(pvFrames,0,sizeof(tPvFrame) * numPvFrames); }
			
			for (int i=0; i<numPvFrames; i++) {
				pvFrames[i].Context[0] = this;
				pvFrames[i].Context[1] = new int(i);
				pvFrames[i].Context[2] = new float(0);
			}
		}
		
		unsigned long frameSize = 0;
		tPvErr error = PvAttrUint32Get( deviceHandle, "TotalBytesPerFrame", &frameSize );
		
		int width = getIntAttribute("Width");
		int height = getIntAttribute("Height");
		
		if( error == ePvErrSuccess ){
			for (int i=0; i<numPvFrames; i++) {
				delete (char*)pvFrames[i].ImageBuffer;
				pvFrames[i].ImageBuffer = new char[frameSize];
				pvFrames[i].ImageBufferSize = frameSize;
				pvFrames[i].Width = width;
				pvFrames[i].Height = height;
			}
			bFramesAllocated = true;
		} else {
			ofLog(OF_LOG_ERROR, "Camera: %lu failed to allocate capture buffer", deviceID);
			deallocateFrames();
		}
	}
	
	void Camera::deallocateFrames() {
		if (bFramesAllocated) {
			capuredFrameQueue.clear();
			for (int i=0; i<numPvFrames; i++) {
				delete (char*)pvFrames[i].ImageBuffer;
			}
			delete[] pvFrames;
			bFramesAllocated = false;
		}
	}
	
	void Camera::resizeFrames() {
		clearQueue();
//		stopAcquisition();		// stop Aq and Cap to make width and height register correctly
		stopCapture();			// stop Capture to make width and height register correctly
		allocateFrames();
		startCapture();
		startAcquisition();
		queueFrames();
	}
	
	bool Camera::queueFrames(){
		for (int i=0; i<numPvFrames; i++) {
			tPvErr error = PvCaptureQueueFrame( deviceHandle, &pvFrames[i], frameCallBack);
			if (error != ePvErrSuccess ){
				ofLog(OF_LOG_NOTICE, "Camera: " + ofToString(deviceID) + " failed to queue frame " + ofToString(i));
				logError(error);
				return false;
			}
		}
		ofLog(OF_LOG_VERBOSE, "Camera: " + ofToString(deviceID) + " frames queued");
		return true;
	}
	
	bool Camera::clearQueue() {
		tPvErr error = PvCaptureQueueClear(deviceHandle);
		if (error != ePvErrSuccess ){
			logError(error);
			return false;
		}
		return true;
	}
	
	
	bool Camera::triggerFrame(){
		tPvErr error = PvCommandRun(deviceHandle,"FrameStartTriggerSoftware");
		if( error == ePvErrSuccess ){
			//			ofLog(OF_LOG_VERBOSE, "Camera: %lu frame triggered", deviceID);
			return true;
		} else {
			ofLog(OF_LOG_ERROR, "Camera: %lu can not trigger frame", deviceID);
			logError(error);
			return false;
		}
	}
	
	
	void Camera::frameCallBack(tPvFrame* pFrame){
		Camera* cam = (Camera*)pFrame->Context[0];
		if(cam) {cam->receiveFrame(pFrame); }
	}
	
	void Camera::receiveFrame(tPvFrame* _frame) {
		if (_frame->Status == ePvErrSuccess) {
			PvCaptureQueueFrame(deviceHandle, _frame, frameCallBack);
			float& time = *(float*)_frame->Context[2];
			time = ofGetElapsedTimef();
			capuredFrameQueue.push_front(_frame);
		}
		else if (_frame->Status != ePvErrCancelled) {
			PvCaptureQueueFrame(deviceHandle, _frame, frameCallBack);
			if (_frame->Status != ePvErrBufferTooSmall) { // don't log error on resize;
				logError(_frame->Status);
			}
		}
	}
	
	
	//----------------------------------------------------------------------------
	//-- FRAMES ------------------------------------------------------------------
	
	void Camera::setTriggered(bool _value) {
		bTriggered = _value;
		if (bTriggered) {
			setEnumAttribute("FrameStartTriggerMode","Software");
			setFrameRate(ofGetTargetFrameRate());
		}
		else {
			setEnumAttribute("FrameStartTriggerMode","FixedRate");
		}
		setExposure(min(getExposure(), getExposureMaxForCurrentFrameRate()));
		setAutoExposureRangeFromFrameRate();
	}
	
	void Camera::setFrameRate(float rate) {
		setFloatAttribute("FrameRate", rate);
		setExposure(min(getExposure(), getExposureMaxForCurrentFrameRate()));
		setAutoExposureRangeFromFrameRate();
	}
	
	
	//----------------------------------------------------------------------------
	//-- PIXELS ------------------------------------------------------------------
	
	bool Camera::setPixelFormat(ofPixelFormat _pixelFormat) {
		if (bDeviceActive)
		ofLog(OF_LOG_ERROR, "Camera: %lu: setPixelFormat(): can't set pixel format while device is active", deviceID);
		else {
			if (_pixelFormat == OF_PIXELS_MONO || _pixelFormat == OF_PIXELS_RGB) {
				pixelFormat = _pixelFormat;
				//				T_bResizeFrames = true // should be possible (with lock) right?
				return true;
			}
			else {
				ofLogWarning("Camera") << "setPixelFormat(): requested pixel format " << _pixelFormat << " not supported";
				return false;
			}
		}
		return false;
	}
	
	ofPixelFormat Camera::getOfPixelFormat(string _format) {
		if (_format == "Mono8") { return OF_PIXELS_MONO; }
		else if (_format == "Rgb24") { return OF_PIXELS_RGB; }
		else { ofLogWarning("Camera") << "pixel format not recognized, defaulting to OF_PIXELS_MONO"; }
	}
	
	string Camera::getPvPixelFormat(ofPixelFormat _format) {
		if (_format == OF_PIXELS_MONO) { return "Mono8"; }
		else if (_format == OF_PIXELS_RGB) { return "Rgb24"; }
		else { ofLogWarning("Camera") << "pixel format not recognized, defaulting to Mono8"; }
		
	}
	
	
	//----------------------------------------------------------------------------
	//-- TEXTURE -----------------------------------------------------------------
	
	ofTexture& Camera::getTexture() {
		if (bDeviceActive) {
			int w = getWidth();
			int h = getHeight();
			if (isFrameNew() && !bTextureSet) {				
				int glFormat = ofGetGLInternalFormatFromPixelFormat(getPixelFormat());
				if (texture.isAllocated()) {
					if (texture.getWidth() != w || texture.getHeight() != h || texture.getTextureData().glInternalFormat != glFormat) {
						texture.clear();
					}
				}
				if (!texture.isAllocated()) {
					texture.allocate(w, h, glFormat);
				}
				texture.loadData(getPixels());
				bTextureSet = true;
			}
		}
		return texture;
	}
	
	
	//----------------------------------------------------------------------------
	//-- ATTRIBUTES (ROI) --------------------------------------------------------
	
	void Camera::setROIWidth(int _value) {
		if (getIntAttribute("Width") != _value) {
			setIntAttribute("Width", _value);
			if(getROIX() > getROIXMax()) { setROIX(getROIXMax()); }
			resizeFrames();
		}
	}
	
	void Camera::setROIHeight(int _value) {
		if (getIntAttribute("Height") != _value) {
			setIntAttribute("Height", _value);
			if(getROIY() > getROIYMax()) { setROIY(getROIYMax()); }
			resizeFrames();
		}
	}
	
	void Camera::setROIX(int _value) {
		int maxValue = getROIXMax();
		if (_value > maxValue) {
			_value = maxValue;
		}
		setIntAttribute("RegionX", _value);
	}
	
	void Camera::setROIY(int _value) {
		int maxValue = getROIYMax();
		if (_value > maxValue) {
			_value = maxValue;
		}
		setIntAttribute("RegionY", _value);
		
		//		regionY = (float)getROIY() / getROIYMax();
	}
	
	
	//----------------------------------------------------------------------------
	//-- ATTRIBUTES (GENERAL) ----------------------------------------------------
	
	void Camera::listAttributes(){
		tPvAttrListPtr listPtr;
		unsigned long listLength;
		if (PvAttrList(deviceHandle, &listPtr, &listLength) == ePvErrSuccess) {
			for (int i = 0; i < listLength; i++)
			{
				const char* attributeName = listPtr[i];
				printf("Attribute: %s", attributeName);
				tPvAttributeInfo pInfo;
				PvAttrInfo(deviceHandle, attributeName, &pInfo);
				tPvDatatype pDatatype = pInfo.Datatype;
				
				
				switch (pDatatype) {
					case 0 :
					printf(", Datatype: Unknown \n");
					break;
					case 1 :
					printf(", Datatype: Command \n");
					break;
					case 2 :
					printf(", Datatype: Raw \n");
					break;
					case 3 :
					char pString[128];
					PvAttrStringGet(deviceHandle, attributeName, pString, sizeof(pString), NULL);
					printf(", Datatype: String, Value: %s \n", pString);
					break;
					case 4 :
					char pEnum[128];
					char pEnumRange[2048];
					
					PvAttrEnumGet(deviceHandle, attributeName, pEnum, sizeof(pEnum), NULL);
					PvAttrRangeEnum(deviceHandle, attributeName, pEnumRange, sizeof(pEnum), NULL);
					printf(", Datatype: Enum, Value: %s, Set: %s \n", pEnum, pEnumRange);
					break;
					case 5 :
					tPvUint32 pUIValue, pUIMin, pUIMax;
					PvAttrUint32Get(deviceHandle, attributeName, &pUIValue);
					PvAttrRangeUint32(deviceHandle, attributeName, &pUIMin, &pUIMax);
					printf(", Datatype: Uint32, Value: %li, Min: %li, Max: %li \n", pUIValue, pUIMin, pUIMax);
					break;
					case 6 :
					tPvFloat32 pFValue, pFMin, pFMax;
					PvAttrFloat32Get(deviceHandle, attributeName, &pFValue);
					PvAttrRangeFloat32(deviceHandle, attributeName, &pFMin, &pFMax);
					printf(", Datatype: Float32, Value: %f, Min: %f, Max: %f \n", pFValue, pFMin, pFMax);
					break;
					case 7 :
					tPvInt64 pIValue, pIMin, pIMax;
					PvAttrInt64Get(deviceHandle, attributeName, &pIValue);
					PvAttrRangeInt64(deviceHandle, attributeName, &pIMin, &pIMax);
					printf(", Datatype: Int64, Value: %lld, Min: %lld, Max: %lld \n", pIValue, pIMin, pIMax);
					break;
					case 8 :
					tPvBoolean pBValue;
					PvAttrBooleanGet(deviceHandle, attributeName, &pBValue);
					printf(", Datatype: Boolean, Value: %hhd \n", pBValue);
					break;
					default :
					printf("No Datatype?? \n");
					break;
				}
				
				
				
			}
		}
	}
	
	void Camera::resetAttributes() {
		if (bDeviceActive) {
			setFrameRate(60);
			
			setROIWidth(min(getROIWidthMax(), 640));
			setROIHeight(min(getROIWidthMax(), 480));
			setROIX(getROIXMax() / 2);
			setROIY(getROIYMax() / 2);
			
			setAutoExposureTarget(20); // 20% white
			setAutoExposureRate(getAutoExposureRateMax());
			setAutoExposureAdjustTol(getAutoExposureAdjustTolMin());
			setAutoExposureOutliers(getAutoExposureOutliersMin());
			setAutoExposureRangeFromFrameRate();
			setAutoExposureOnce(true);
			
			setGain(getGainMin());
			
			if(getPixelFormat() == OF_PIXELS_RGB) {
				setAutoGain(false);
				setAutoGainTarget(getAutoGainTargetMin());
				setAutoGainRate(getAutoGainRateMax());
				setAutoGainAdjustTol(getAutoGainAdjustTolMin());
				setAutoGainOutliers(getAutoGainOutliersMin());
				setAutoGainMinimum(getAutoGainMinimumMin());
				setAutoGainMaximum(getAutoGainMaximumMax());
				
				setWhiteBalanceBlue((getWhiteBalanceBlueMax() - getWhiteBalanceBlueMin()) / 2.0 + getWhiteBalanceBlueMin());
				setWhiteBalanceRed((getWhiteBalanceRedMax() - getWhiteBalanceRedMin()) / 2.0 + getWhiteBalanceRedMin());
				setAutoWhiteBalance(false);
				setAutoWhiteBalanceRate(getAutoWhiteBalanceRateMax());
				setAutoWhiteBalanceAdjustTol(getAutoWhiteBalanceAdjustTolMin());
				
				setGamma(1.0);
				setHue(0.0);
				setSaturation(1.0);
			}
			
			ofLog(OF_LOG_VERBOSE,"Camera: attributes set to default");
		}
	}
	
	
	bool Camera::setNormalizedAttribute(string _name, float _value) {
		
		if (!ofInRange(_value, 0.0, 1.0)) {
			ofLog(OF_LOG_NOTICE, "Camera: %lu normalized attribute %s value %f out of range (0.0, 1.0), clamping...", deviceID, _name.c_str(), _value);
			_value = ofClamp(_value, 0.0, 1.0);
		}
		
		tPvAttributeInfo pInfo;
		tPvErr error = PvAttrInfo(deviceHandle, _name.c_str(), &pInfo);
		if (error != ePvErrSuccess) {
			ofLog(OF_LOG_WARNING, "Camera: %lu, Attribute %s Not Found ", deviceID, _name.c_str());
			return 0;
		}
		tPvDatatype pDatatype = pInfo.Datatype;
		
		
		int minI, maxI, attribI;
		float minF, maxF, attribF;
		
		switch (pDatatype) {
			case ePvDatatypeUint32:
			minI = getIntAttributeMin(_name);
			maxI = getIntAttributeMax(_name);
			attribI = _value * (maxI - minI) + minI;
			setIntAttribute(_name, attribI);
			break;
			
			case ePvDatatypeFloat32:
			minF = getIntAttributeMin(_name);
			maxF = getIntAttributeMax(_name);
			attribF = _value * (maxF - minF) + minF;
			setFloatAttribute(_name, attribF);
			ofLog(OF_LOG_NOTICE, "min " + ofToString(minF) + " max " + ofToString(maxF));
			break;
			
			default:
			ofLog(OF_LOG_WARNING, "Camera: Normalized Attribute Not Supported");
			break;
		}
	}
	
	float Camera::getNormalizedAttribute(string _name) {
		tPvAttributeInfo pInfo;
		tPvErr error = PvAttrInfo(deviceHandle, _name.c_str(), &pInfo);
		if (error != ePvErrSuccess) {
			ofLog(OF_LOG_WARNING, "Camera: %lu, Attribute %s Not Found ", deviceID, _name.c_str());
			return 0;
		}
		tPvDatatype pDatatype = pInfo.Datatype;
		
		int minI, maxI, attribI;
		float minF, maxF, attribF;
		
		switch (pDatatype) {
			case ePvDatatypeUint32:
			minI = getIntAttributeMin(_name);
			maxI = getIntAttributeMax(_name);
			attribI = getIntAttribute(_name);
			return float(attribI - minI) / float(maxI - minI);
			break;
			
			case ePvDatatypeFloat32:
			minF = getFloatAttributeMin(_name);
			maxF = getFloatAttributeMax(_name);
			attribF = getFloatAttribute(_name);
			return float(attribF - minF) / float(maxF - minF);
			break;
			
			default:
			ofLog(OF_LOG_WARNING, "Camera: Normalized Attribute Not Supported");
			break;
		}
	}
	
	
	bool Camera::setEnumAttribute(string _name, string _value) {
		tPvErr error = PvAttrEnumSet(deviceHandle, _name.c_str(), _value.c_str());
		
		if (error == ePvErrSuccess) {
//			ofLog(OF_LOG_VERBOSE, "Camera: %lu set %s to %s", deviceID, _name.c_str(), _value.c_str());
			return true;
		}
		
		ofLog(OF_LOG_ERROR, "Camera: %lu failed to set %s to %s", deviceID, _name.c_str(), _value.c_str());
		logError(error);
		return false;
	}
	
	string Camera::getEnumAttribute(string _name) {
		
		char attribute[128];
		tPvErr error = PvAttrEnumGet(deviceHandle, _name.c_str(), attribute, sizeof(attribute), NULL);
		
		if (error != ePvErrSuccess) {
			ofLog(OF_LOG_ERROR, "Camera: %lu failed to get enumeration attribute %s", deviceID, _name.c_str());
			logError(error);
			return "none";
		}
		
//		ofLog(OF_LOG_VERBOSE, "Camera: %lu %s is %s", deviceID, _name.c_str(), attribute);
		return attribute ;
	}
	
	
	bool Camera::setIntAttribute(string _name, int _value) {
		tPvUint32 min, max;
		PvAttrRangeUint32(deviceHandle, _name.c_str(), &min, &max);
		
		if (!ofInRange(_value, min, max)) {
			ofLog(OF_LOG_NOTICE, "Camera: %lu attribute %s value %i out of range (%lu, %lu), clamping...", deviceID, _name.c_str(), _value, min, max);
			_value = ofClamp(_value, min, max);
		}
		
		tPvErr error = PvAttrUint32Set(deviceHandle, _name.c_str(), _value);
		
		if (error == ePvErrSuccess) {
//			ofLog(OF_LOG_VERBOSE, "Camera: %lu set attribute %s to %i in range %lu to %lu", deviceID, _name.c_str(), _value, min, max);
			return true;
		}
		
		
		ofLog(OF_LOG_ERROR, "Camera: %lu failed to set attribute %s to %i", deviceID, _name.c_str(), _value);
		logError(error);
		return false;
	}
	
	int Camera::getIntAttribute(string _name) {
		
		tPvUint32 value;
		tPvErr error = PvAttrUint32Get(deviceHandle, _name.c_str(), &value);
		
		if (error != ePvErrSuccess) {
			ofLog(OF_LOG_ERROR, "Camera: %lu failed to get attribute %s", deviceID, _name.c_str());
			logError(error);
			return 0;
		}
		
//		ofLog(OF_LOG_VERBOSE, "Camera: %lu %s is %lu", deviceID, _name.c_str(), value);
		return value ;
	}
	
	int Camera::getIntAttributeMax(string _name) {
		tPvUint32 t_min, t_max;
		tPvErr error = PvAttrRangeUint32(deviceHandle, _name.c_str(), &t_min, &t_max);
		
		if (error != ePvErrSuccess) {
			ofLog(OF_LOG_ERROR, "Camera: %lu failed to get maximum for attribute %s", deviceID, _name.c_str());
			logError(error);
			return 1;
		}
		
		return t_max;
	}
	
	int Camera::getIntAttributeMin(string _name) {
		tPvUint32 t_min, t_max;
		tPvErr error = PvAttrRangeUint32(deviceHandle, _name.c_str(), &t_min, &t_max);
		
		if (error != ePvErrSuccess) {
			ofLog(OF_LOG_ERROR, "Camera: %lu failed to get minimum for attribute %s", deviceID, _name.c_str());
			logError(error);
			return 0;
		}
		
		return t_min;
	}
	
	
	bool Camera::setFloatAttribute(string _name, float _value) {
		tPvFloat32 min, max;
		PvAttrRangeFloat32(deviceHandle, _name.c_str(), &min, &max);
		if (!ofInRange(_value, min, max)) {
			ofLog(OF_LOG_NOTICE, "Camera: %lu attribute %s value %f out of range (%f, %f), clamping...", deviceID, _name.c_str(), _value, min, max);
			_value = ofClamp(_value, min, max);
		}
		
		tPvErr error = PvAttrFloat32Set(deviceHandle, _name.c_str(), _value);
		
		if (error == ePvErrSuccess) {
//			ofLog(OF_LOG_VERBOSE, "Camera: %lu set attribute %s to %f in range %f to %f", deviceID, _name.c_str(), _value, min, max);
			return true;
		}
		
		ofLog(OF_LOG_ERROR, "Camera: %lu failed to set attribute %s to %f", deviceID, _name.c_str(), _value);
		logError(error);
		return false;
	}
	
	float Camera::getFloatAttribute(string _name) {
		tPvFloat32 value;
		tPvErr error = PvAttrFloat32Get(deviceHandle, _name.c_str(), &value);
		
		if (error != ePvErrSuccess) {
			ofLog(OF_LOG_ERROR, "Camera: %lu failed to get attribute %s", deviceID, _name.c_str());
			logError(error);
			return 0;
		}
		
//		ofLog(OF_LOG_VERBOSE, "Camera: %lu %s is %f", deviceID, _name.c_str(), value);
		return value ;
	}
	
	float Camera::getFloatAttributeMax(string _name) {
		tPvFloat32 t_min, t_max;
		tPvErr error = PvAttrRangeFloat32(deviceHandle, _name.c_str(), &t_min, &t_max);
		
		if (error != ePvErrSuccess) {
			ofLog(OF_LOG_ERROR, "Camera: %lu failed to get maximum for attribute %s", deviceID, _name.c_str());
			logError(error);
			return 1;
		}
		
		return t_max;
		
	}
	
	float Camera::getFloatAttributeMin(string _name) {
		tPvFloat32 t_min, t_max;
		tPvErr error = PvAttrRangeFloat32(deviceHandle, _name.c_str(), &t_min, &t_max);
		
		if (error != ePvErrSuccess) {
			ofLog(OF_LOG_ERROR, "Camera: %lu failed to get minimum for attribute %s", deviceID, _name.c_str());
			logError(error);
			return 0;
		}
		
		return t_min;
	}
	
	
	//----------------------------------------------------------------------------
	//-- IP SETTINGS -------------------------------------------------------------
	
	void Camera::listIpSettings() {
		
		tPvIpSettings ipSettings;
		PvCameraIpSettingsGet(deviceID, &ipSettings);
		
		ofLogNotice("Printing IP Settings");
		
		if(ipSettings.ConfigMode == ePvIpConfigPersistent)
		printf("Config Mode: Persistent");
		if(ipSettings.ConfigMode == ePvIpConfigDhcp)
		printf("Config Mode: DHCP, fallback to AutoIP");
		if(ipSettings.ConfigMode == ePvIpConfigAutoIp)
		printf("Config Mode: AutoIP only");
		printf("Current Ip Address: %s \n", IPLongToString(ipSettings.CurrentIpAddress).c_str());
		printf("Current Ip Subnet Mask: %s \n", IPLongToString(ipSettings.CurrentIpSubnet).c_str());
		printf("Current Ip Gateway: %s \n", IPLongToString(ipSettings.CurrentIpGateway).c_str());
		printf("Persistent Ip Addr: %s \n", IPLongToString(ipSettings.PersistentIpAddr).c_str());
		printf("Persistent Ip Subnet Mask: %s \n", IPLongToString(ipSettings.PersistentIpSubnet).c_str());
		printf("Persistent Ip Gateway: %s \n", IPLongToString(ipSettings.PersistentIpGateway).c_str());
	}
	
	
	void Camera::updateIpSettings() {
#ifdef _WIN32
		ofLogWarning("setPersistentIp not supported for windows");
		return;
#else
		if (bDeviceActive) {
			bool hasChanged = false;
			tPvIpSettings ipSettings;
			PvCameraIpSettingsGet(deviceID, &ipSettings);
			
			if (persistentIpEnable && ipSettings.ConfigMode != ePvIpConfigPersistent) {
				ipSettings.ConfigMode = ePvIpConfigPersistent;
				hasChanged = true;
			}
			else if (!persistentIpEnable && ipSettings.ConfigMode != ePvIpConfigDhcp) {
				ipSettings.ConfigMode = ePvIpConfigDhcp;
				hasChanged = true;
			}
			
			struct in_addr addr, sn, gw;
			if (persistentIpAdress != "" ) {
				if (!inet_pton(AF_INET, persistentIpAdress.c_str(), &addr)) {
					ofLogWarning("Camera: ") << deviceID << ", failed to change IP settings: IP " << persistentIpAdress << " is not a valid adress, ";
					return;
				} else {
					if (ipSettings.PersistentIpAddr != addr.s_addr) {
						ipSettings.PersistentIpAddr = addr.s_addr;
						hasChanged = true;
					}
				}
			}
			if (persistentIpSubnetMask != "" ) {
				if (!inet_pton(AF_INET, persistentIpSubnetMask.c_str(), &sn)) {
					ofLogWarning("Camera: ") << deviceID << ", failed to change IP settings: Subnet Mask " << persistentIpSubnetMask << " is not a valid adress";
					return;
				} else {
					if(ipSettings.PersistentIpSubnet != sn.s_addr ) {
						ipSettings.PersistentIpSubnet = sn.s_addr;
						hasChanged = true;
					}
				}
			}
			if (persistentIpGateway  != "" ) {
				if (!inet_pton(AF_INET, persistentIpGateway.c_str(), &gw)){
					ofLogWarning("Camera: ") << deviceID << ", failed to change IP settings: Gateway " << persistentIpGateway << " is not a valid adress";
					return;
				} else {
					if (ipSettings.PersistentIpGateway != gw.s_addr) {
						ipSettings.PersistentIpGateway = gw.s_addr;
						hasChanged = true;
					}
				}
			}
			
			if (hasChanged) {
				deactivate();
				
				tPvErr error;
				error = PvCameraIpSettingsChange(deviceID, &ipSettings);
				if (error == ePvErrSuccess) {
					ofLogNotice("Camera:") << deviceID << " Camera IP Settings Changed";
				}
				else {
					ofLogNotice("Camera:") << deviceID << " Failed to Change Camera IP Settings";
					logError(error);
				}
				
				activate();
			}
		}
#endif
	}
	
	void Camera::setIpPersistent(bool _enable) {
		persistentIpEnable = _enable;
	}
	
	void Camera::setPersistentIpAdress(string _IpAdress) {
		persistentIpAdress = _IpAdress;
	}
	
	void Camera::setPersistentIpSubnetMask(string _IpSubnet) {
		persistentIpSubnetMask = _IpSubnet;
	}
	
	void Camera::setPersistentIpGateway(string _IpGateway) {
		persistentIpGateway = _IpGateway;
	}
	
	
	bool Camera::getIpPersistent() {
		tPvIpSettings ipSettings;
		PvCameraIpSettingsGet(deviceID, &ipSettings);
		return (ipSettings.ConfigMode == ePvIpConfigPersistent);
	}
	
	string Camera::getCurrentIpAdress() {
		tPvIpSettings ipSettings;
		PvCameraIpSettingsGet(deviceID, &ipSettings);
		return IPLongToString(ipSettings.CurrentIpAddress);
	}
	
	string Camera::getCurrentIpSubnetMask() {
		tPvIpSettings ipSettings;
		PvCameraIpSettingsGet(deviceID, &ipSettings);
		return IPLongToString(ipSettings.CurrentIpSubnet);
	}
	
	string Camera::getCurrentIpGateway() {
		tPvIpSettings ipSettings;
		PvCameraIpSettingsGet(deviceID, &ipSettings);
		return IPLongToString(ipSettings.CurrentIpGateway);
	}
	
	
	string Camera::getPersistentIpAdress() {
		tPvIpSettings ipSettings;
		PvCameraIpSettingsGet(deviceID, &ipSettings);
		return IPLongToString(ipSettings.PersistentIpAddr);
	}
	
	string Camera::getPersistentIpSubnetMask() {
		tPvIpSettings ipSettings;
		PvCameraIpSettingsGet(deviceID, &ipSettings);
		return IPLongToString(ipSettings.PersistentIpSubnet);
	}
	
	string Camera::getPersistentIpGateway() {
		tPvIpSettings ipSettings;
		PvCameraIpSettingsGet(deviceID, &ipSettings);
		return IPLongToString(ipSettings.PersistentIpGateway);
	}
	
	
	unsigned long Camera::IPStringToLong(string _IpAdress) {
		unsigned long addr = inet_addr(_IpAdress.c_str());
		return addr;
	}
	
	string Camera::IPLongToString(unsigned long _IpAdress) {
		struct in_addr addr;
		char *dot_ip;
		
		addr.s_addr = _IpAdress;
		dot_ip = inet_ntoa(addr);
		
		return dot_ip;
	}
	
	
	//----------------------------------------------------------------------------
	//-- ERROR LOGGING------------------------------------------------------------
	
	void Camera::logError(tPvErr _msg) {
		switch (_msg) {
			case ePvErrCameraFault:
			ofLog(OF_LOG_ERROR, "Camera: %lu Unexpected camera fault", deviceID);
			break;
			case ePvErrInternalFault:
			ofLog(OF_LOG_ERROR, "Camera: %lu Unexpected fault in PvApi or driver", deviceID);
			break;
			case ePvErrBadHandle:
			ofLog(OF_LOG_ERROR, "Camera: %lu handle is invalid", deviceID);
			break;
			case ePvErrBadParameter:
			ofLog(OF_LOG_ERROR, "Camera: %lu Bad parameter to API call", deviceID);
			break;
			case ePvErrBadSequence:
			ofLog(OF_LOG_ERROR, "Camera: %lu Sequence of API calls is incorrect", deviceID);
			break;
			case ePvErrNotFound:
			ofLog(OF_LOG_ERROR, "Camera: %lu camera or attribute not found", deviceID);
			break;
			case ePvErrAccessDenied:
			ofLog(OF_LOG_ERROR, "Camera: %lu can not be opened in the specified mode", deviceID);
			break;
			case ePvErrUnplugged:
			ofLog(OF_LOG_WARNING, "Camera: %lu was unplugged", deviceID);
			break;
			case ePvErrInvalidSetup:
			ofLog(OF_LOG_ERROR, "Camera: %lu Setup is invalid (an attribute is invalid)", deviceID);
			break;
			case ePvErrResources:
			ofLog(OF_LOG_ERROR, "Camera: %lu System/network resources or memory not available", deviceID);
			break;
			case ePvErrBandwidth:
			ofLog(OF_LOG_ERROR, "Camera: %lu 1394??? bandwidth not available", deviceID);
			break;
			case ePvErrQueueFull:
			ofLog(OF_LOG_ERROR, "Camera: %lu Too many frames on queue", deviceID);
			break;
			case ePvErrBufferTooSmall:
			ofLog(OF_LOG_WARNING, "Camera: %lu Frame buffer is too small", deviceID);
			break;
			case ePvErrCancelled:
			ofLog(OF_LOG_VERBOSE, "Camera: %lu Frame cancelled by user", deviceID);
			break;
			case ePvErrDataLost:
			ofLog(OF_LOG_WARNING, "Camera: %lu The data for the frame was lost", deviceID);
			break;
			case ePvErrDataMissing:
			ofLog(OF_LOG_VERBOSE, "Camera: %lu Some data in the frame is missing", deviceID);
			break;
			case ePvErrTimeout:
			ofLog(OF_LOG_ERROR, "Camera: %lu Timeout during wait", deviceID);
			break;
			case ePvErrOutOfRange:
			ofLog(OF_LOG_NOTICE, "Camera: %lu Attribute value is out of the expected range", deviceID);
			break;
			case ePvErrWrongType:
			ofLog(OF_LOG_ERROR, "Camera: %lu Attribute is not this type (wrong access function)", deviceID);
			break;
			case ePvErrForbidden:
			ofLog(OF_LOG_ERROR, "Camera: %lu Attribute write forbidden at this time", deviceID);
			break;
			case ePvErrUnavailable:
			ofLog(OF_LOG_ERROR, "Camera: %lu Attribute is not available at this time", deviceID);
			break;
			case ePvErrFirewall:
			ofLog(OF_LOG_ERROR, "Camera: %lu A firewall is blocking the traffic (Windows only)", deviceID);
			break;
			default:
			break;
		}
	}
}

