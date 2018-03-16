
#include "Camera.h"

namespace ofxPvAPI {
	
	bool Camera::bPvApiInitiated = false;
	int Camera::numCamerasInUse = 0;
	int Camera::numPvFrames = 5;
	
	//--------------------------------------------------------------------
	
	Camera::Camera() :
	bInitialized(false),
	bIsFrameNew(false),
	fps(0),
	frameDrop(0),
	fixedRate(0),
	pixelFormat(OF_PIXELS_MONO),
	deviceID(0),
	requestedDeviceID(0),
	persistentIpAdress(""),
	persistentIpGateway(""),
	T_bResizeFrames(false),
	T_bChangeRate(false),
	persistentIpSubnetMask("0.0.0.0"){
		
		if (!bPvApiInitiated) PvApiInitialize() ;
		
		// to allocateFrame
		pvFrames = new tPvFrame[numPvFrames];
		if (pvFrames) { memset(pvFrames,0,sizeof(tPvFrame) * numPvFrames); }
		
		for (int i=0; i<numPvFrames; i++) {
			pvFrames[i].Context[0] = this;
			pvFrames[i].Context[1] = new int(i);
			pvFrames[i].Context[2] = new float(0);
		}
		
		lastIdentifier = numPvFrames - 1;
	}
	
	Camera::~Camera(){
		close();
		if (numCamerasInUse == 0 && bPvApiInitiated)
		PvApiUnInitialize();
	}
	
	void Camera::PvApiInitialize() {
		
		if( bPvApiInitiated ){
			ofLog(OF_LOG_VERBOSE, "PvAPI already initialized");
		}
		else {
			tPvErr error = PvInitialize();
			if( error == ePvErrSuccess ) {
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
		if (numCamerasInUse == 0 && bPvApiInitiated) {
			PvUnInitialize();
			bPvApiInitiated = false;
			ofLog(OF_LOG_NOTICE, "Camera: PvAPI uninitialized");
		}
	}
	
	
	//----------------------------------------------------------------------------
	//-- DEVICES -----------------------------------------------------------------
	
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
		if (bInitialized)
		ofLog(OF_LOG_ERROR, "Camera: %lu: setDeviceID(): can't set ID while grabber is running", deviceID);
		else {
			requestedDeviceID = _deviceID;
		}
	}
	
	int Camera::getDeviceID() {
		return deviceID;
	}
	
	
	//----------------------------------------------------------------------------
	//-- OF ----------------------------------------------------------------------
	
	bool Camera::setup() {
		vector<ofVideoDevice> deviceList = listDevices();
		if (deviceList.size() == 0) {
			ofLog(OF_LOG_WARNING, "Camera: %lu no cameras found", deviceID);
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
				ofLog(OF_LOG_NOTICE, "Camera: no camera ID specified, defaulting to camera %lu", requestedDeviceID);
			}
			else {
				ofLog(OF_LOG_WARNING, "Camera: found no camera available ");
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
					ofLog(OF_LOG_VERBOSE, "Camera: %lu found", requestedDeviceID);
				}
			}
		}
		if (!requestedDeviceFound) {
			ofLog(OF_LOG_WARNING, "Camera: %lu not found", requestedDeviceID);
			return false;
		}
		if (!requestedDeviceAvailable) {
			ofLog(OF_LOG_WARNING, "Camera: %lu not available", requestedDeviceID);
			return false;
		}
		
		return initCamera(requestedDeviceID);
	}
	
	void Camera::threadedFunction(){
		while(isThreadRunning()) {
			
			if (bInitialized) {
				lock();
				if (T_bResizeFrames) {
					T_bResizeFrames = false;
					clearQueue();
					stopAcquisition();		// without these pvFrame width and height don't register correctly
					stopCapture();			// without these pvFrame width and height don't register correctly
					allocateFrames();
					startCapture();			// without these pvFrame width and height don't register correctly
					startAcquisition();		// without these pvFrame width and height don't register correctly
				}
				unlock();
				
				// make two 
				for (int i=0; i<numPvFrames; i++) {
					tPvFrame& frame = pvFrames[i];
					tPvErr error = PvCaptureWaitForFrameDone(cameraHandle, &frame, 1);
					if (frame.Status == ePvErrTimeout) {
						;
					}
					else if (frame.Status == ePvErrSuccess) {

						lock();
						ofPixels* newPixels = new ofPixels;
						if (!T_bResizeFrames) {
//							float& time = *(float*)frame.Context[2];
//							time = ofGetElapsedTimef();
//							newPixels->setFromPixels((unsigned char *)frame.ImageBuffer, frame.Width, frame.Height, pixelFormat);
						}
						unlock();
//
						PvCaptureQueueFrame(cameraHandle, &frame, NULL);
//
						if (newPixels->isAllocated()) {
//							newPixels->mirror(flipV, flipH);
//							newPixels->rotate90(rotate90);
//
//							capuredFrames.push_front(newPixels);
//
//							size_t maxSize = (fixedRate)? 2 : 1;
//							if (capuredFrames.size() > maxSize) {
//								delete capuredFrames[capuredFrames.size()-1];
//								capuredFrames.pop_back();
////								cout << "deleted" << endl;
//							}
						}
						else {
							delete newPixels;
						}
//					}
//					else if (frame.Status != ePvErrCancelled) {
//						logError(frame.Status);
//						PvCaptureQueueFrame(cameraHandle, &frame, NULL);
//						triggerFrame();
//					}
//					else {
//						logError(frame.Status);
//						PvCaptureQueueFrame(cameraHandle, &frame, NULL);
//						triggerFrame();
					}
				}
			}
			sleep(1);
		}
	}
	
	void Camera::update() {
		
		bIsFrameNew = false;
		
		if (bInitialized) {
			if (!fixedRate) { triggerFrame(); }
			
			lock();
			if ( capuredFrames.size() > 0) {
				size_t frameoffset = (fixedRate)? 1 : 0;
				frameoffset = min(capuredFrames.size(), frameoffset);
//				pixels.setFromPixels(&capuredFrames[frameoffset]);
				if ( capuredFrames.size() > 0) {
					frameoffset = min(capuredFrames.size(), frameoffset);
					pixels = *capuredFrames[frameoffset];
				}
//				tPvFrame& frame = *capuredFrameQueue[frameoffset];
				
				//				cout << i << " " << frame.Width << " " << frame.Height << " " << frame.ImageBufferSize << endl;
				//				cout << i << " " << frame.Width << " " << getIntAttribute("Width") << endl;
				
				float time = ofGetElapsedTimef();
				float frameTime = time; // init with current time 
				
//				if (frame.Status == ePvErrSuccess) { // the state can be changed since added
//					pixels.setFromExternalPixels((unsigned char *)frame.ImageBuffer, frame.Width, frame.Height, pixelFormat);
//					frameTime = *(float*)frame.Context[2];
					bIsFrameNew = true;
//				}
//				else {
//					ofLogWarning("Camera") << " croocked frame";
//				}
//
				fpsTimes.push_back(time);
//				framesLatencies.push_back((time - frameTime) * 1000);
//
//				capuredFrameQueue.pop_back();
//
//				int df = 0;
//				while (capuredFrameQueue.size() > frameoffset) {
//					capuredFrameQueue.pop_back();
//					df++;
//				}
//				framesDropped.push_back(df);
			}
			
			unlock();
		}
		
		float timeWindow = ofGetElapsedTimef() - (1 - (1.0 / max(fps, 1) / 3.0));
		
		while (fpsTimes.size() > 0 && fpsTimes.at(0) < timeWindow) {
			fpsTimes.pop_front();
//			framesDropped.pop_front();
//			framesLatencies.pop_front();
		}
		fps = fpsTimes.size();
		
		frameDrop = 0;
		for (int i=0; i<framesDropped.size(); i++) {
			frameDrop += framesDropped[i];
		}
		
		frameMaxLatency = 0;
		frameMinLatency = 10000;
		float tL = 0;
		for (int i=0; i<framesLatencies.size(); i++) {
			tL += framesLatencies[i];
			frameMaxLatency = max(frameMaxLatency, framesLatencies[i]);
			frameMinLatency = min(frameMinLatency, framesLatencies[i]);
		}
		frameLatency = (tL / framesLatencies.size());
	}
	
	void Camera::close(){
		
		if( bInitialized ) {
			waitForThread();
			
			clearQueue();
			stopAcquisition();
			stopCapture();
			closeCamera();
			
			for (int i=0; i<numPvFrames; i++) {
				delete (char*)pvFrames[i].ImageBuffer;
			}
			
			bInitialized = false;
			numCamerasInUse--;
			
			ofLog(OF_LOG_NOTICE, "Camera: %lu closed", deviceID);
		}
	}
	
	
	//----------------------------------------------------------------------------
	//-- ACQUISITION -------------------------------------------------------------
	
	bool Camera::initCamera(int _cameraUid){
		
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
		ofLog(OF_LOG_NOTICE,"Camera: %lu up and running", deviceID);
		
		queueFrames();
		startThread(true);
		return true;
	}
	
	
	bool Camera::openCamera() {
		tPvErr error = PvCameraOpen( deviceID, ePvAccessMaster, &cameraHandle );
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
		tPvErr error = PvCameraClose(cameraHandle);
		if( error == ePvErrSuccess ){
			ofLog(OF_LOG_VERBOSE, "Camera: %lu camera closed", deviceID);
			bInitialized = false;
			return true;
		} else {
			ofLog(OF_LOG_ERROR, "Camera: %lu failed to close", deviceID);
			logError(error);
			return false;
		}
	}
	
	
	bool Camera::startCapture() {
		
		tPvErr error = PvCaptureStart(cameraHandle);
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
		tPvErr error = PvCaptureEnd(cameraHandle);
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
		tPvErr error = PvCommandRun(cameraHandle,"AcquisitionStart");
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
		
		tPvErr error = PvCommandRun(cameraHandle,"AcquisitionStop");
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
		
		tPvErr error = PvCommandRun(cameraHandle,"AcquisitionAbort");
		if( error == ePvErrSuccess ){
			ofLog(OF_LOG_VERBOSE, "Camera: %lu continuous acquisition aborted", deviceID);
			return true;
		} else {
			ofLog(OF_LOG_ERROR, "Camera: %lu can not abort continuous acquisition", deviceID);
			logError(error);
			return false;
		}
	}
	
	
	//----------------------------------------------------------------------------
	//-- PV FRAMES ---------------------------------------------------------------
	
	bool Camera::allocateFrames() {
		
		unsigned long frameSize = 0;
		tPvErr error = PvAttrUint32Get( cameraHandle, "TotalBytesPerFrame", &frameSize );
		
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
			ofLog(OF_LOG_VERBOSE, "Camera: %lu allocate capture buffer to %i, %i", deviceID, width, height);
		} else {
			ofLog(OF_LOG_ERROR, "Camera: %lu failed to allocate capture buffer", deviceID);
			return false;
		}
		
		return true;
	}
	
	bool Camera::deallocateFrames() {
		//		for (int i=0; i<2; i++) {
		//			delete (char*)pvFrames[i].ImageBuffer;
		//			pvFrames[i].ImageBuffer = new char[frameSize];
		//			pvFrames[i].ImageBufferSize = frameSize;
		//			pvFrames[i].Width = width;
		//			pvFrames[i].Height = height;
		//			bPvFrameNew[i] = false;
		//		}
	}
	
	
	bool Camera::queueFrames(){
		for (int i=0; i<numPvFrames; i++) {
			tPvErr error = PvCaptureQueueFrame( cameraHandle, &pvFrames[i], NULL);
			if (error != ePvErrSuccess ){
				ofLog(OF_LOG_NOTICE, "Camera: " + ofToString(deviceID) + " failed to queue frame " + ofToString(i));
				logError(error);
				return false;
			}
		}
		ofLog(OF_LOG_VERBOSE, "Camera: " + ofToString(deviceID) + " frames queued");
		lastIdentifier = numPvFrames - 1;
		return true;
	}
	
	bool Camera::clearQueue() {
		tPvErr error = PvCaptureQueueClear(cameraHandle);
		if (error != ePvErrSuccess ){
			logError(error);
			return false;
		}
		return true;
	}
	
	
	bool Camera::triggerFrame(){
		tPvErr error = PvCommandRun(cameraHandle,"FrameStartTriggerSoftware");
		if( error == ePvErrSuccess ){
//						ofLog(OF_LOG_VERBOSE, "Camera: %lu frame triggered", deviceID);
			return true;
		} else {
			ofLog(OF_LOG_ERROR, "Camera: %lu can not trigger frame", deviceID);
			logError(error);
			return false;
		}
	}
	
	
//	void Camera::onFrameDone(tPvFrame* _frame) {
////		lock();
//
//		int identifier = *(int*)_frame->Context[1];
//		int expectedIdentifier = (lastIdentifier + 1) % numPvFrames;
//		if (expectedIdentifier != identifier) {
//			ofLogWarning("Camera") << "frame " << expectedIdentifier << " not in queue " << identifier;
//		}
//		lastIdentifier = identifier;
//
//		if (_frame->Status == ePvErrSuccess) {
//			PvCaptureQueueFrame(cameraHandle, _frame, FrameDoneCB);
//			float& time = *(float*)_frame->Context[2];
//			time = ofGetElapsedTimef();
//			capuredFrameQueue.push_front(_frame);
//		}
//		else if (_frame->Status != ePvErrCancelled) {
//			logError(_frame->Status);
//			PvCaptureQueueFrame(cameraHandle, _frame, FrameDoneCB);
//		}
//		else {
//			logError(_frame->Status);
//		}
////		unlock();
//	}
	
	
		//----------------------------------------------------------------------------
		//-- FRAMES --------------------------------------------------------------
	void Camera::setFixedRate(bool _value) {
		fixedRate = _value;
		
//		T_bChangeRate
		
//		clearQueue();
//		stopAcquisition();
		
		if (fixedRate) {
			setEnumAttribute("FrameStartTriggerMode","FixedRate");
			
		}
		else {
			setEnumAttribute("FrameStartTriggerMode","Software");
//			triggerFrame();
		}
		
//		startAcquisition();
//		queueFrames();
//		unlock();
	}
	
	void Camera::setFrameRate(float rate) {
		setFloatAttribute("FrameRate", rate);
		setExposure(min(getExposure(), getExposureMaxForCurrentFrameRate()));
		setAutoExposureRangeFromFrameRate();
	}
	
	
	//----------------------------------------------------------------------------
	//-- PIXELS ------------------------------------------------------------------
	
	bool Camera::setPixelFormat(ofPixelFormat _pixelFormat) {
		if (bInitialized)
		ofLog(OF_LOG_ERROR, "Camera: %lu: setPixelFormat(): can't set pixel format while grabber is running", deviceID);
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
	//-- ATTRIBUTES (ROI) --------------------------------------------------------
	
	void Camera::setROIWidth(int _value) {
		if (getIntAttribute("Width") != _value) {
			lock();	// lock neccesary?
			T_bResizeFrames = true;
			setIntAttribute("Width", _value);
			if(getROIX() > getROIXMax()) { setROIX(getROIXMax()); }
			unlock();
		}
	}
	
	void Camera::setROIHeight(int _value) {
		if (getIntAttribute("Height") != _value) {
			lock();	// lock neccesary?
			T_bResizeFrames = true;
			setIntAttribute("Height", _value);
			if(getROIY() > getROIYMax()) { setROIY(getROIYMax()); }
			//			regionY = (float)getROIY() / getROIYMax();
			unlock();
		}
	}
	
	void Camera::setROIX(int _value) {
		int maxValue = getROIXMax();
		if (_value > maxValue) {
			_value = maxValue;
		}
		setIntAttribute("RegionX", _value);
		
		//		regionX = (float)getROIX() / getROIXMax();
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
		if (PvAttrList(cameraHandle, &listPtr, &listLength) == ePvErrSuccess) {
			for (int i = 0; i < listLength; i++)
			{
				const char* attributeName = listPtr[i];
				printf("Attribute: %s", attributeName);
				tPvAttributeInfo pInfo;
				PvAttrInfo(cameraHandle, attributeName, &pInfo);
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
					PvAttrStringGet(cameraHandle, attributeName, pString, sizeof(pString), NULL);
					printf(", Datatype: String, Value: %s \n", pString);
					break;
					case 4 :
					char pEnum[128];
					char pEnumRange[2048];
					
					PvAttrEnumGet(cameraHandle, attributeName, pEnum, sizeof(pEnum), NULL);
					PvAttrRangeEnum(cameraHandle, attributeName, pEnumRange, sizeof(pEnum), NULL);
					printf(", Datatype: Enum, Value: %s, Set: %s \n", pEnum, pEnumRange);
					break;
					case 5 :
					tPvUint32 pUIValue, pUIMin, pUIMax;
					PvAttrUint32Get(cameraHandle, attributeName, &pUIValue);
					PvAttrRangeUint32(cameraHandle, attributeName, &pUIMin, &pUIMax);
					printf(", Datatype: Uint32, Value: %li, Min: %li, Max: %li \n", pUIValue, pUIMin, pUIMax);
					break;
					case 6 :
					tPvFloat32 pFValue, pFMin, pFMax;
					PvAttrFloat32Get(cameraHandle, attributeName, &pFValue);
					PvAttrRangeFloat32(cameraHandle, attributeName, &pFMin, &pFMax);
					printf(", Datatype: Float32, Value: %f, Min: %f, Max: %f \n", pFValue, pFMin, pFMax);
					break;
					case 7 :
					tPvInt64 pIValue, pIMin, pIMax;
					PvAttrInt64Get(cameraHandle, attributeName, &pIValue);
					PvAttrRangeInt64(cameraHandle, attributeName, &pIMin, &pIMax);
					printf(", Datatype: Int64, Value: %lld, Min: %lld, Max: %lld \n", pIValue, pIMin, pIMax);
					break;
					case 8 :
					tPvBoolean pBValue;
					PvAttrBooleanGet(cameraHandle, attributeName, &pBValue);
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
		if (bInitialized) {
			setFrameRate(25); // i'm in Europe and like to use lightbulbs
			
			setROIWidth(getROIWidthMax());
			setROIHeight(getROIHeightMax());
			setROIX(getROIXMin());
			setROIY(getROIYMin());
			
			setAutoExposureTarget(33); // 33% white
			setAutoExposureRate(10);
			setAutoExposureAdjustTol(5);
			setAutoExposureOutliers(getAutoExposureOutliersMin());
			setAutoExposureRangeFromFrameRate();
			setAutoExposureOnce(true);
			
			setGain(getGainMin()); // should this be 0 or 1?
			
			if(getPixelFormat() == OF_PIXELS_RGB) {
				setAutoGain(false);
				setAutoGainTarget(getAutoGainTargetMin());
				setAutoGainRate(10);
				setAutoGainAdjustTol(5);
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
		tPvErr error = PvAttrInfo(cameraHandle, _name.c_str(), &pInfo);
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
		tPvErr error = PvAttrInfo(cameraHandle, _name.c_str(), &pInfo);
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
		tPvErr error = PvAttrEnumSet(cameraHandle, _name.c_str(), _value.c_str());
		
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
		tPvErr error = PvAttrEnumGet(cameraHandle, _name.c_str(), attribute, sizeof(attribute), NULL);
		
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
		PvAttrRangeUint32(cameraHandle, _name.c_str(), &min, &max);
		
		if (!ofInRange(_value, min, max)) {
			ofLog(OF_LOG_NOTICE, "Camera: %lu attribute %s value %i out of range (%lu, %lu), clamping...", deviceID, _name.c_str(), _value, min, max);
			_value = ofClamp(_value, min, max);
		}
		
		tPvErr error = PvAttrUint32Set(cameraHandle, _name.c_str(), _value);
		
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
		tPvErr error = PvAttrUint32Get(cameraHandle, _name.c_str(), &value);
		
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
		tPvErr error = PvAttrRangeUint32(cameraHandle, _name.c_str(), &t_min, &t_max);
		
		if (error != ePvErrSuccess) {
			ofLog(OF_LOG_ERROR, "Camera: %lu failed to get maximum for attribute %s", deviceID, _name.c_str());
			logError(error);
			return 1;
		}
		
		return t_max;
	}
	
	int Camera::getIntAttributeMin(string _name) {
		tPvUint32 t_min, t_max;
		tPvErr error = PvAttrRangeUint32(cameraHandle, _name.c_str(), &t_min, &t_max);
		
		if (error != ePvErrSuccess) {
			ofLog(OF_LOG_ERROR, "Camera: %lu failed to get minimum for attribute %s", deviceID, _name.c_str());
			logError(error);
			return 0;
		}
		
		return t_min;
	}
	
	
	bool Camera::setFloatAttribute(string _name, float _value) {
		tPvFloat32 min, max;
		PvAttrRangeFloat32(cameraHandle, _name.c_str(), &min, &max);
		if (!ofInRange(_value, min, max)) {
			ofLog(OF_LOG_NOTICE, "Camera: %lu attribute %s value %f out of range (%f, %f), clamping...", deviceID, _name.c_str(), _value, min, max);
			_value = ofClamp(_value, min, max);
		}
		
		tPvErr error = PvAttrFloat32Set(cameraHandle, _name.c_str(), _value);
		
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
		tPvErr error = PvAttrFloat32Get(cameraHandle, _name.c_str(), &value);
		
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
		tPvErr error = PvAttrRangeFloat32(cameraHandle, _name.c_str(), &t_min, &t_max);
		
		if (error != ePvErrSuccess) {
			ofLog(OF_LOG_ERROR, "Camera: %lu failed to get maximum for attribute %s", deviceID, _name.c_str());
			logError(error);
			return 1;
		}
		
		return t_max;
		
	}
	
	float Camera::getFloatAttributeMin(string _name) {
		tPvFloat32 t_min, t_max;
		tPvErr error = PvAttrRangeFloat32(cameraHandle, _name.c_str(), &t_min, &t_max);
		
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
	
	string Camera::getIpAdress() {
		tPvIpSettings ipSettings;
		PvCameraIpSettingsGet(deviceID, &ipSettings);
		return IPLongToString(ipSettings.CurrentIpAddress);
	}
	
	string Camera::getIpSubnet() {
		tPvIpSettings ipSettings;
		PvCameraIpSettingsGet(deviceID, &ipSettings);
		return IPLongToString(ipSettings.CurrentIpSubnet);
	}
	
	string Camera::getIpGateway() {
		tPvIpSettings ipSettings;
		PvCameraIpSettingsGet(deviceID, &ipSettings);
		return IPLongToString(ipSettings.CurrentIpGateway);
	}
	
	bool Camera::getIpPersistent() {
		tPvIpSettings ipSettings;
		tPvErr error;
		error = PvCameraIpSettingsGet(deviceID, &ipSettings);
		if (error != ePvErrSuccess ){
			logError(error);
			return false;
		}
		if (ipSettings.ConfigMode == ePvIpConfigPersistent)
		return true;
		return false;
	}
	
	
	void Camera::setPersistentIp(bool enable) {
		
#ifdef _WIN32
		ofLogWarning("setPersistentIp not supported for windows");
		return;
#else
		tPvIpSettings ipSettings;
		PvCameraIpSettingsGet(deviceID, &ipSettings);
		if (!enable) {
			clearQueue();
			//stopAcquisition();
			stopCapture();
			closeCamera();
			bInitialized = false;
			
			ipSettings.ConfigMode = ePvIpConfigDhcp;
			
		}
		else {
			
			struct in_addr addr, sn, gw;
			
			if (!inet_pton(AF_INET, persistentIpAdress.c_str(), &addr)) {
				ofLogWarning("Camera: ") << deviceID << ", IP Adress " << persistentIpAdress << " is not valid";
				return;
			}
			if (!inet_pton(AF_INET, persistentIpSubnetMask.c_str(), &sn)) {
				ofLogWarning("Camera: ") << deviceID << ", Subnet Mask " << persistentIpSubnetMask << " is not valid";
				return;
			}
			if (!inet_pton(AF_INET, persistentIpGateway.c_str(), &gw));{
				ofLogWarning("Camera: ") << deviceID << ", Gatway " << persistentIpGateway << " is not valid";
			}
			
			clearQueue();
			//stopAcquisition();
			stopCapture();
			closeCamera();
			bInitialized = false;
			
			ipSettings.ConfigMode = ePvIpConfigPersistent;
			ipSettings.PersistentIpAddr = addr.s_addr;
			ipSettings.PersistentIpSubnet = sn.s_addr;
			ipSettings.PersistentIpGateway = gw.s_addr;
		}
		
		tPvErr error;
		error = PvCameraIpSettingsChange(deviceID, &ipSettings);
		if (error == ePvErrSuccess) {
			ofLogNotice("Camera:") << deviceID << " Camera IP Settings Changed";
		}
		else {
			ofLogNotice("Camera:") << deviceID << " Failed to Change Camera IP Settings";
			logError(error);
		}
		
		ofLogNotice("Camera:") << deviceID << " Reinitializing...";
		initCamera(deviceID);
		
#endif
	}
	
	void Camera::setPersistentIpAdress(string _IpAdress) {
#ifdef _WIN32
		ofLogWarning("setPersistentIpAdress not supported for windows");
		return;
#endif
		persistentIpAdress = _IpAdress;
	}
	
	void Camera::setPersistentIpSubnetMask(string _IpSubnet) {
#ifdef _WIN32
		ofLogWarning("setPersistentIpSubnetMask not supported for windows");
		return;
#endif
		persistentIpSubnetMask = _IpSubnet;
	}
	
	void Camera::setPersistentIpGateway(string _IpGateway) {
#ifdef _WIN32
		ofLogWarning("setPersistentIpGateway not supported for windows");
		return;
#endif
		persistentIpGateway = _IpGateway;
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
	
	
	bool Camera::setPacketSizeToMax() {
		tPvErr error = PvCaptureAdjustPacketSize(cameraHandle, getIntAttributeMax("PacketSize"));
		
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
			ofLog(OF_LOG_ERROR, "Camera: %lu was unplugged", deviceID);
			
			close(); // should not close, but should close cam
			
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
			ofLog(OF_LOG_NOTICE, "Camera: %lu Frame cancelled by user", deviceID);
			break;
			case ePvErrDataLost:
			ofLog(OF_LOG_ERROR, "Camera: %lu The data for the frame was lost", deviceID);
			break;
			case ePvErrDataMissing:
			ofLog(OF_LOG_NOTICE, "Camera: %lu Some data in the frame is missing", deviceID);
			break;
			case ePvErrTimeout:
			ofLog(OF_LOG_ERROR, "Camera: %lu Timeout during wait", deviceID);
			break;
			case ePvErrOutOfRange:
			ofLog(OF_LOG_ERROR, "Camera: %lu Attribute value is out of the expected range", deviceID);
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

