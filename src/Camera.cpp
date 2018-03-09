#include "Camera.h"

namespace ofxProsilica {
	
	bool Camera::bPvApiInitiated = false;
    int Camera::numCamerasInUse = 0;
	
    
	//--------------------------------------------------------------------
    
	Camera::Camera() :
	bInitialized(false),
	T_bIsFrameNew(false),
	T_bNeedsResize(false),
	bWaitingForFrame(false),
	frameCount(0),
	internalPixelFormat(OF_PIXELS_MONO),
	deviceID(0),
	requestedDeviceID(0),
	regionX(0.5),
	regionY(0.5),
	persistentIpAdress(""),
    persistentIpGateway(""),
    persistentIpSubnetMask("0.0.0.0") {
		
		if (!bPvApiInitiated) PvApiInitialize() ;
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
				ofSleepMillis(500); // Need some time to actually be initialized;
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
	
	
	//--------------------------------------------------------------------
	
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
	
	bool Camera::initCamera(int _cameraUid){
		
		deviceID = _cameraUid;
		
        if (!openCamera()) return false;
		if (!setPacketSizeToMax()) return false;
		if (!startCapture()) return false;
		if (!setEnumAttribute("FrameStartTriggerMode","FixedRate")) return false;
		if (!setEnumAttribute( "AcquisitionMode", "Continuous")) return false;
		
		string pf = "Mono8";
		if (internalPixelFormat == OF_PIXELS_RGB) pf = "Rgb24";
		if (!setEnumAttribute("PixelFormat", pf)) return false;
		
		if (!allocatePixels()) return false;
		if (!startAcquisition()) return false;
		
		bInitialized = true;
		frameCount = 0;
		numCamerasInUse++;
		ofLog(OF_LOG_NOTICE,"Camera: %lu up and running", deviceID);
		
		startThread(true);
		return true;
	}
	
	void Camera::threadedFunction(){
		while(isThreadRunning()) {
//			bIsFrameNew = false;
			if (bInitialized) {
				
				bool bResize = false;
				lock();
				if (T_bNeedsResize) {
					T_bNeedsResize = false;
					clearQueue();
					allocatePixels();
				}
				unlock();
				
				if( !bWaitingForFrame ) {
					bWaitingForFrame = queueFrame();
				}
				
				tPvErr error = PvCaptureWaitForFrameDone(cameraHandle, &cameraFrame, 4); // in MiliSeconds
				if (error == ePvErrTimeout) {
					
				} else if(error == ePvErrSuccess ){
					lock();
					if (!T_bNeedsResize) {
						T_pixelsOut = framePixels;
						T_bIsFrameNew = true;
						frameCount++;
					}
					unlock();
					
					bWaitingForFrame = false;
				} else if (error == ePvErrUnplugged) {
					ofLogWarning("Camera " + ofToString(deviceID) + " connection lost");
					close();
				} else {
					logError(error);
					close();
				}
			}
			sleep(4);
		}
	}
	
	void Camera::update() {
		
	}
	
	bool Camera::isInitialized() {
         return bInitialized;
    }
    
	bool Camera::isFrameNew(bool _reset){
		lock();
		bool B = T_bIsFrameNew;
		if (_reset) {
			T_bIsFrameNew = false;
		}
		unlock();
		return B;
	}
	
	void Camera::close(){
		
		if( bInitialized ) {
			waitForThread();
			// stop the streaming
			clearQueue();
			//stopAcquisition();
			stopCapture();
			closeCamera();
			
			framePixels.clear();
			
			bInitialized = false;
			numCamerasInUse--;
			
			ofLog(OF_LOG_NOTICE, "Camera: %lu closed", deviceID);
		}
	}
	
	
	//--------------------------------------------------------------------
	//-- ACQUISITION -----------------------------------------------------
	
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
			ofLog(OF_LOG_VERBOSE, "Camera: %lu continuous acquisition started", deviceID);
			return true;
		} else {
			ofLog(OF_LOG_ERROR, "Camera: %lu can not start continuous acquisition", deviceID);
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
			// WHYWHYWHYWHYWHYWHY
			PvCaptureEnd(cameraHandle) ;
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
			// WHYWHYWHYWHYWHYWHY
			PvCaptureEnd(cameraHandle) ;
			ofLog(OF_LOG_ERROR, "Camera: %lu can not abort continuous acquisition", deviceID);
			logError(error);
			return false;
		}
	}
	
	bool Camera::queueFrame(){
		tPvErr error = PvCaptureQueueFrame( cameraHandle, &cameraFrame, NULL);
		if (error != ePvErrSuccess ){
			ofLog(OF_LOG_NOTICE, "Camera " + ofToString(deviceID) + " failed to queue frame buffer -> no worries just try again");
			logError(error);
			return false;
		}
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
	
    
	//--------------------------------------------------------------------
	//-- DEVICES ---------------------------------------------------------
	
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
        tPvIpSettings   pIpSettings;
        
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
	
	
	//--------------------------------------------------------------------
	//-- PIXELS ----------------------------------------------------------
	
	bool Camera::allocatePixels() {
		
//		clearQueue();
		
		framePixels.clear();
		
		unsigned long frameSize = 0;
		tPvErr error = PvAttrUint32Get( cameraHandle, "TotalBytesPerFrame", &frameSize );
		
		int width = getIntAttribute("Width");
		int height = getIntAttribute("Height");
		
		framePixels.allocate(width, height, internalPixelFormat);
		
		ancillaryPixels.allocate(1, 1, 1);
		ancillaryPixels.set(0);
		
		if( error == ePvErrSuccess ){
			cameraFrame.ImageBuffer = framePixels.getData();
			cameraFrame.ImageBufferSize = frameSize;
			
			// don't really get this one, but it prevents rare crash on startup
			cameraFrame.AncillaryBuffer = ancillaryPixels.getData();
			cameraFrame.AncillaryBufferSize = 0;
		} else {
			ofLog(OF_LOG_ERROR, "Camera: %lu failed to allocate capture buffer", deviceID);
			return false;
		}
		
		return true;
	}
    
	ofPixels& Camera::getPixels(){
		return T_pixelsOut;
	}
	
//	void Camera::getPixels(ofPixels& _pixels){
//		lock();
//		_pixels = T_pixelsOut;
//		unlock();
//	}
	
//	unsigned char * Camera::getData(){
//		return pixels.getData();
//	}
	
	bool Camera::setPixelFormat(ofPixelFormat _pixelFormat) {
		if (bInitialized)
			ofLog(OF_LOG_ERROR, "Camera: %lu: setPixelFormat(): can't set pixel format while grabber is running", deviceID);
		else {
			if (_pixelFormat == OF_PIXELS_MONO || _pixelFormat == OF_PIXELS_RGB) {
				internalPixelFormat = _pixelFormat;
				return true;
			}
			else {
				ofLogWarning("Camera") << "setPixelFormat(): requested pixel format " << _pixelFormat << " not supported";
				return false;
			}
		}
		return false;
	}
	
	ofPixelFormat Camera::getPixelFormat() {
		return internalPixelFormat;
	}
    
	
	//--------------------------------------------------------------------
	//-- ATTRIBUTES (GENERAL) --------------------------------------------
	
	void Camera::listAttributes(){
		tPvAttrListPtr    listPtr;
		unsigned long     listLength;
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
						printf(", Datatype: Int64, Value: %lld, Min: %lld, Max: %lld  \n", pIValue, pIMin, pIMax);
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
			setFrameRate(25);  // i'm in Europe and like to use lightbulbs
			
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
    
	
	//--------------------------------------------------------------------
	
	void Camera::setROIWidth(int _value) {
		if (getIntAttribute("Width") != _value) {
			lock();
			T_bNeedsResize = true;
			setIntAttribute("Width", _value);
			if(getROIX() > getROIXMax()) { setROIX(getROIXMax()); }
			regionX = (float)getROIX() / getROIXMax();
			unlock();
		}
	}
	
	void Camera::setROIHeight(int _value) {
		if (getIntAttribute("Height") != _value) {
			lock();
			T_bNeedsResize = true;
			setIntAttribute("Height", _value);
			if(getROIY() > getROIYMax()) { setROIY(getROIYMax()); }
			regionY = (float)getROIY() / getROIYMax();
			unlock();
		}
	}

	void Camera::setROIX(int _value) {
		int maxValue = getROIXMax();
		if (_value > maxValue) {
			_value = maxValue;
		}
		setIntAttribute("RegionX", _value);
		
		regionX = (float)getROIX() / getROIXMax();
	}

	void Camera::setROIY(int _value) {
		int maxValue = getROIYMax();
		if (_value > maxValue) {
			_value = maxValue;
		}
		setIntAttribute("RegionY", _value);
		
		regionY = (float)getROIY() / getROIYMax();
	}
	
	/*	
	 void Camera::setNormalizedROIWidth(float w){
		setNormalizedAttribute ("Width", w);
		float rx = regionX * (1 - w);
		setNormalizedAttribute ("RegionX", rx);
		allocatePixels();
	 }
	 
	 float Camera::getNormalizedROIWidth(){
		return getNormalizedAttribute("Width");
	 }
	 
	 void Camera::setNormalizedROIHeight(float h){
		setNormalizedAttribute ("Height", h);
		float ry = regionY * (1 - h);
		setNormalizedAttribute ("RegionY", ry);
		allocatePixels();
	 }
	 
	 float Camera::getNormalizedROIHeight(){
		return getNormalizedAttribute("Height");
	 }
	 
	 void Camera::setNormalizedROIX(float x){
		regionX = x;
		float w = getNormalizedAttribute("Width");
		float rx = regionX * (1 - w);
		setNormalizedAttribute ("RegionX", rx);
	 }
	 
	 float Camera::getNormalizedROIX(){
		return regionX;
	 }
	 
	 void Camera::setNormalizedROIY(float y){
		regionY  = y;
		float h = getNormalizedAttribute("Height");
		float ry = regionY * (1 - h);
		setNormalizedAttribute ("RegionY", ry);
	 }
	 
	 float Camera::getNormalizedROIY(){
		return regionY;
	 }
	 */
	
	//--------------------------------------------------------------------
	//-- ATTRIBUTES (PROTECTED) ------------------------------------------
	
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
			ofLog(OF_LOG_VERBOSE, "Camera: %lu set %s to %s", deviceID, _name.c_str(), _value.c_str());
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
			ofLog(OF_LOG_VERBOSE, "Camera: %lu set attribute %s to %i in range %lu to %lu", deviceID, _name.c_str(), _value, min, max);
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
			ofLog(OF_LOG_VERBOSE, "Camera: %lu set attribute %s to %f in range %f to %f", deviceID, _name.c_str(), _value, min, max);
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
	
	
	//--------------------------------------------------------------------
	//-- IP SETTINGS -----------------------------------------------------
    
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
                ofLogWarning("Camera: ") << deviceID << ", IP Adress " << persistentIpAdress <<  " is not valid";
                return;
            }
            if (!inet_pton(AF_INET, persistentIpSubnetMask.c_str(), &sn)) {
                ofLogWarning("Camera: ") << deviceID << ", Subnet Mask " << persistentIpSubnetMask <<  " is not valid";
                return;
            }
            if (!inet_pton(AF_INET, persistentIpGateway.c_str(), &gw));{
                ofLogWarning("Camera: ") << deviceID << ", Gatway " << persistentIpGateway <<  " is not valid";
                //    return false;
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
    
    string Camera::IPLongToString(unsigned long  _IpAdress) {
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
	
	
	//--------------------------------------------------------------------
	//-- ERROR LOGGING----------------------------------------------------
	
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
				
				close();
				
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
				ofLog(OF_LOG_ERROR, "Camera: %lu: Frame buffer is too small", deviceID);
				break;
			case ePvErrCancelled:
				ofLog(OF_LOG_ERROR, "Camera: %lu: Frame cancelled by user", deviceID);
				break;
			case ePvErrDataLost:
				ofLog(OF_LOG_ERROR, "Camera: %lu The data for the frame was lost", deviceID);
				break;
			case ePvErrDataMissing:
				ofLog(OF_LOG_ERROR, "Camera: %lu Some data in the frame is missing", deviceID);
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
