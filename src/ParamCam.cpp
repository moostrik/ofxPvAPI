#include "ParamCam.h"


namespace ofxPvAPI {
	
		//------------------------------------------------------------------------
	void ParamCam::setup() {
		Camera::setup();		
		
		parameters.clear();
		string dID = "unknown";
		if (Camera::getRequestedDeviceID() > 0) { dID = ofToString(Camera::getRequestedDeviceID()); }
		parameters.setName("camera " + dID);
		parameters.add(pActivate.set("activate", false));
		pActivate.addListener(this, &ParamCam::activateListener);
		parameters.add(pDeactivate.set("deactivate", false));
		pDeactivate.addListener(this, &ParamCam::deactivateListener);
		parameters.add(reset.set("reset", false));
		reset.addListener(this, &ParamCam::resetListener);
		parameters.add(printAttributes.set("print features", false));
		printAttributes.addListener(this, &ParamCam::printAttributesListener);
		
		frameRateParameters.setName("framerate");
		frameRateParameters.add(pFrameRate.set("frame rate", 60, 0, 60));
		frameRateParameters.add(pFps.set("fps (actual rate)", 0, 0, 60));
		frameRateParameters.add(pFrameDrop.set("drops per second", 0, 0, 60));
		frameRateParameters.add(pTriggered.set("trigger mode", false));
		latencyParameters.setName("latency (ms)");
		latencyParameters.add(pFrameLatency.set("avgerage", 0, 0, 60));
		latencyParameters.add(pFrameMaxLatency.set("min", 0, 0, 60));
		latencyParameters.add(pFrameMinLatency.set("max", 0, 0, 60));
		frameRateParameters.add(latencyParameters);
		pTriggered.addListener(this, &ParamCam::triggeredListener);
		pFrameRate.addListener(this, &ParamCam::frameRateListener);
		parameters.add(frameRateParameters);
		
		roiParameters.setName("region of interest");
		roiParameters.add(pROIWidth.set("width", 640, 1, 2048));
		pROIWidth.addListener(this, &ParamCam::ROIWidthListener);
		roiParameters.add(pROIHeight.set("height", 480, 1, 2048));
		pROIHeight.addListener(this, &ParamCam::ROIHeightListener);
		roiParameters.add(pROIX.set("x", 0, 0, 1));
		pROIX.addListener(this, &ParamCam::ROIXListener);
		roiParameters.add(pROIY.set("y", 0, 0, 1));
		pROIY.addListener(this, &ParamCam::ROIYListener);
		parameters.add(roiParameters);
		
		exposureParameters.setName("exposure");
		exposureParameters.add(pExposure.set("exposure", 30, 0, 40000));
		pExposure.addListener(this, &ParamCam::exposureListener);
		autoExposureParameters.setName("auto exposure");
		autoExposureParameters.add(pAutoExposure.set("auto", true));
		pAutoExposure.addListener(this, &ParamCam::autoExposureListener);
		autoExposureParameters.add(pAutoExposureOnce.set("auto once", false));
		pAutoExposureOnce.addListener(this, &ParamCam::autoExposureOnceListener);
		autoExposureParameters.add(pAutoExposureTarget.set("target pct", 33, 0, 100));
		pAutoExposureTarget.addListener(this, &ParamCam::autoExposureTargetListener);
		autoExposureParameters.add(pAutoExposureRate.set("rate pct", 10, 1, 100));
		pAutoExposureRate.addListener(this, &ParamCam::autoExposureRateListener);
		autoExposureParameters.add(pAutoExposureAdjustTol.set("tolerance pct", 5, 0, 50));
		pAutoExposureAdjustTol.addListener(this, &ParamCam::autoExposureAdjustTolListener);
		autoExposureParameters.add(pAutoExposureOutliers.set("outliers bp", 0, 0, 1000));
		pAutoExposureOutliers.addListener(this, &ParamCam::autoExposureOutliersListener);
		autoExposureParameters.add(pAutoExposureMinimum.set("minimium μs", 30, 30, 1000000));
		pAutoExposureMinimum.addListener(this, &ParamCam::autoExposureMinimumListener);
		autoExposureParameters.add(pAutoExposureMaximum.set("maximum μs", 40000, 30, 1000000));
		pAutoExposureMaximum.addListener(this, &ParamCam::autoExposureMaximumListener);
		exposureParameters.add(autoExposureParameters);
		parameters.add(exposureParameters);
		
		if(getPixelFormat() != OF_PIXELS_RGB) {
			parameters.add(pGain.set("gain", 0, 0, 30));
			pGain.addListener(this, &ParamCam::gainListener);
		}
		else {
			gainParameters.setName("gain");
			gainParameters.add(pGain.set("gain", 0, 0, 30));
			pGain.addListener(this, &ParamCam::gainListener);
			autoGainParameters.setName("auto gain");
			autoGainParameters.add(pAutoGain.set("auto", false));
			pAutoGain.addListener(this, &ParamCam::autoGainListener);
			autoGainParameters.add(pAutoGainOnce.set("auto once", false));
			pAutoGainOnce.addListener(this, &ParamCam::autoGainOnceListener);
			autoGainParameters.add(pAutoGainTarget.set("target pct", 0, 1, 100));
			pAutoGainTarget.addListener(this, &ParamCam::autoGainTargetListener);
			autoGainParameters.add(pAutoGainRate.set("rate pct", 10, 1, 100));
			pAutoGainRate.addListener(this, &ParamCam::autoGainRateListener);
			autoGainParameters.add(pAutoGainAdjustTol.set("tolerance pct", 5, 0, 50));
			pAutoGainAdjustTol.addListener(this, &ParamCam::autoGainAdjustTolListener);
			autoGainParameters.add(pAutoGainOutliers.set("outliers bp", 0, 0, 1000));
			pAutoGainOutliers.addListener(this, &ParamCam::autoGainOutliersListener);
			autoGainParameters.add(pAutoGainMinimum.set("minimum dB", 0, 0, 30));
			pAutoGainMinimum.addListener(this, &ParamCam::autoGainMinimumListener);
			autoGainParameters.add(pAutoGainMaximum.set("maximum dB", 30, 0, 30));
			pAutoGainMaximum.addListener(this, &ParamCam::autoGainMaximumListener);
			gainParameters.add(autoGainParameters);
			parameters.add(gainParameters);
			
			whiteBalanceParameters.setName("white balance");
			whiteBalanceParameters.add(pWhiteBalanceRed.set("red", 190, 80, 300));
			pWhiteBalanceRed.addListener(this, &ParamCam::WhiteBalanceRedListener);
			whiteBalanceParameters.add(pWhiteBalanceBlue.set("blue", 190, 80, 300));
			pWhiteBalanceBlue.addListener(this, &ParamCam::WhiteBalanceBlueListener);
			autoWhiteBalanceParameters.setName("auto white balance");
			autoWhiteBalanceParameters.add(pAutoWhiteBalance.set("auto", false));
			pAutoWhiteBalance.addListener(this, &ParamCam::autoWhiteBalanceListener);
			autoWhiteBalanceParameters.add(pAutoWhiteBalanceOnce.set("auto once", false));
			pAutoWhiteBalanceOnce.addListener(this, &ParamCam::autoWhiteBalanceOnceListener);
			autoWhiteBalanceParameters.add(pAutoWhiteBalanceRate.set("rate pct", 100, 1, 100));
			pAutoWhiteBalanceRate.addListener(this, &ParamCam::autoWhiteBalanceRateListener);
			autoWhiteBalanceParameters.add(pAutoWhiteBalanceAdjustTol.set("tolerance pct", 5, 0, 50));
			pAutoWhiteBalanceAdjustTol.addListener(this, &ParamCam::autoWhiteBalanceAdjustTolListener);
			whiteBalanceParameters.add(autoWhiteBalanceParameters);
			parameters.add(whiteBalanceParameters);
			
			gammaHueSaturationParameters.setName("gamma hue saturation");
			gammaHueSaturationParameters.add(pGamma.set("gamma", 1.0, 0.5, 1.5));
			pGamma.addListener(this, &ParamCam::gammaListener);
			gammaHueSaturationParameters.add(pHue.set("hue", 0.0, -10.0, 10.0));
			pHue.addListener(this, &ParamCam::hueListener);
			gammaHueSaturationParameters.add(pSaturation.set("saturation", 1.0, 0.0, 2.0));
			pSaturation.addListener(this, &ParamCam::saturationListener);
			parameters.add(gammaHueSaturationParameters);
		}
		
		ipParameters.setName("IP");
		
		ipParameters.add(pCurrentIpPersistent.set("persistent", "no"));
		ipParameters.add(pCurrentIpAdress.set("adress", "0.0.0.0"));
		ipParameters.add(pCurrentIpSubnetMask.set("subnet", "0.0.0.0"));
		ipParameters.add(pCurrentIpGateway.set("gateway", "0.0.0.0"));
		if (bIpParametersEnabled) {
			ipPersistentParameters.setName("persistent info");
			ipPersistentParameters.add(pPersistentIpSwitch.set("persistent (or DHCP)", false));
			pPersistentIpSwitch.addListener(this, &ParamCam::pPersistentIpSwitchListener);
			ipPersistentParameters.add(pPersistentIpAdress.set("adress", "0.0.0.0"));
			ipPersistentParameters.add(pPersistentIpSubnetMask.set("subnet", "0.0.0.0"));
			ipPersistentParameters.add(pPersistentIpGateway.set("gateway", "0.0.0.0"));
			ipPersistentParameters.add(pPersistentIpUpdate.set("update", false));
			pPersistentIpUpdate.addListener(this, &ParamCam::pPersistentIpUpdateListener);
			ipParameters.add(ipPersistentParameters);
		}
		parameters.add(ipParameters);
		
		bLoadFromInterface = true;
		blockListeners = !isActive();
		
		return true;
	}
	
		//------------------------------------------------------------------------
	void ParamCam::update() {
		Camera::update();
		blockListeners = !isActive();
		
		if (isActive()) {
			pFps = getFps();
			pFrameDrop = getFrameDrop();
			pFrameLatency = getLatency();
			pFrameMaxLatency = getMaxLatency();
			pFrameMinLatency = getMinLatency();
			
			if (bLoadFromInterface) {
				bLoadFromInterface = false;
				setAllParametersFromInterface();
			} else {
				updateParametersFromCam();
			}
		}
		else {
			bLoadFromInterface = true;
		}
		
		if (pTriggered) {
			bool bl = blockListeners;
			blockListeners = true;
			pFrameRate = ofGetTargetFrameRate();
			blockListeners = bl;
		}
	}
	
		//------------------------------------------------------------------------
	void ParamCam::updateParametersFromCam() {
		blockListeners = true;
		if (pAutoExposure.get() || pAutoExposureOnce.get()) {
			pExposure.set(getExposure());
		}
		if (pAutoGain.get()) {
			pGain.set(getGain());
		}
		if (pAutoWhiteBalance.get()) {
			pWhiteBalanceRed.set(getWhiteBalanceRed());
			pWhiteBalanceBlue.set(getWhiteBalanceBlue());
		}
		blockListeners = false;
	}
	
		//------------------------------------------------------------------------
	void ParamCam::setAllParametersFromCam() {
		blockListeners = true;
		
		pROIHeight.set(getROIHeight());
		pROIWidth.set(getROIWidth());
		pROIX.set(getROIX());
		pROIY.set(getROIY());
		
		pFrameRate.set(getFrameRate());
		
		pExposure.set(getExposure());
		
		pAutoExposure.set(getAutoExposure());
		pAutoExposureOnce.set(getAutoExposureOnce());
		pAutoExposureAdjustTol.set(getAutoExposureAdjustTol());
		pAutoExposureMinimum.set(getAutoExposureMinimum());
		pAutoExposureMaximum.set(getAutoExposureMaximum());
		pAutoExposureOutliers.set(getAutoExposureOutliers());
		pAutoExposureRate.set(getAutoExposureRate());
		pAutoExposureTarget.set(getAutoExposureTarget());
		
		pGain = getGain();
		if (getPixelFormat() == OF_PIXELS_RGB) {
			pGamma.set(getGamma());
			pHue.set(getHue());
			pSaturation.set(getSaturation());
			
			pGain.set(getGain());
			pAutoGain.set(getAutoGain());
			pAutoGainOnce.set(getAutoGainOnce());
			pAutoGainAdjustTol.set(getAutoGainAdjustTol());
			pAutoGainMinimum.set(getAutoGainMinimum());
			pAutoGainMaximum.set(getAutoGainMaximum());
			pAutoGainOutliers.set(getAutoGainOutliers());
			pAutoGainRate.set(getAutoGainRate());
			pAutoGainTarget.set(getAutoGainTarget());
			
			pWhiteBalanceRed.set(getWhiteBalanceRed());
			pWhiteBalanceBlue.set(getWhiteBalanceBlue());
			pAutoWhiteBalance.set(getAutoWhiteBalance());
			pAutoWhiteBalanceOnce.set(getAutoWhiteBalanceOnce());
			pAutoWhiteBalanceRate.set(getAutoWhiteBalanceRate());
			pAutoWhiteBalanceAdjustTol.set(getAutoWhiteBalanceAdjustTol());
		}
		
		if (getIpPersistent()) { pCurrentIpPersistent.set("yes"); }
		else { pCurrentIpPersistent.set("no"); }
		
		pCurrentIpAdress.set(getCurrentIpAdress());
		pCurrentIpSubnetMask.set(getCurrentIpSubnetMask());
		pCurrentIpGateway.set(getCurrentIpGateway());
		
		pPersistentIpSwitch.set(getIpPersistent());
		pPersistentIpAdress.set(getPersistentIpAdress());
		pPersistentIpSubnetMask.set(getPersistentIpSubnetMask());
		pPersistentIpGateway.set(getPersistentIpGateway());
		
		blockListeners = false;
		
		setParametersRange();
	}
	
	void ParamCam::setAllParametersFromInterface() {
		
		setROIHeight(pROIHeight.get()); // Height before width, because it affects max framerate
		setROIWidth(pROIWidth.get());
		setROIX(pROIX.get());
		setROIY(pROIY.get());
		
		setFrameRate(pFrameRate.get());
		
		setExposure(pExposure.get());
		setAutoExposure(pAutoExposure.get());
		setAutoExposureTarget(pAutoExposureTarget.get());
		setAutoExposureRate(pAutoExposureRate.get());
		setAutoExposureAdjustTol(pAutoExposureAdjustTol.get());
		setAutoExposureMinimum(pAutoExposureMinimum.get());
		setAutoExposureMaximum(pAutoExposureMaximum.get());
		setAutoExposureOutliers(pAutoExposureOutliers.get());
		
		setGain(pGain.get());
		
		if (getPixelFormat() == OF_PIXELS_RGB) {
			setGamma(pGamma.get());
			setHue(pHue.get());
			setSaturation(pSaturation.get());
			
			setGain(pGain.get());
			setAutoGain(pAutoGain.get());
			setAutoGainAdjustTol(pAutoGainAdjustTol.get());
			setAutoGainMinimum(pAutoGainMinimum.get());
			setAutoGainMaximum(pAutoGainMaximum.get());
			setAutoGainOutliers(pAutoGainOutliers.get());
			setAutoGainRate(pAutoGainRate.get());
			setAutoGainTarget(pAutoGainTarget.get());
			
			setWhiteBalanceRed(pWhiteBalanceRed.get());
			setWhiteBalanceBlue(pWhiteBalanceBlue.get());
			setAutoWhiteBalance(pAutoWhiteBalance.get());
			setAutoWhiteBalanceRate(pAutoWhiteBalanceRate.get());
			setAutoWhiteBalanceAdjustTol(pAutoWhiteBalanceAdjustTol.get());
		}
		
		blockListeners = false;
		pCurrentIpPersistent.set(getIpPersistent()? "yes" : "no");
		pCurrentIpAdress.set(getCurrentIpAdress());
		pCurrentIpSubnetMask.set(getCurrentIpSubnetMask());
		pCurrentIpGateway.set(getCurrentIpGateway());
		pPersistentIpSwitch.set(getIpPersistent());
		pPersistentIpAdress.set(getPersistentIpAdress());
		pPersistentIpSubnetMask.set(getPersistentIpSubnetMask());
		pPersistentIpGateway.set(getPersistentIpGateway());
		blockListeners = true;
		
		setParametersRange();
	}
	
		//------------------------------------------------------------------------
	void ParamCam::setParametersRange() {
		
		pFrameRate.setMin(ceil(getFrameRateMin()));
		pFrameRate.setMax(floor(getFrameRateMax()));
		setParameterInItsOwnRange(pFrameRate);
		pFps.setMin(ceil(getFrameRateMin()));
		pFps.setMax(floor(getFrameRateMax()));
		setParameterInItsOwnRange(pFps);
		
		pROIWidth.setMin(getROIWidthMin());
		pROIWidth.setMax(getROIWidthMax());
		setParameterInItsOwnRange(pROIWidth);
		pROIHeight.setMin(getROIHeightMin());
		pROIHeight.setMax(getROIHeightMax());
		setParameterInItsOwnRange(pROIHeight);
		pROIX.setMin(getROIXMin());
		pROIX.setMax(MAX(getROIXMax(),1)); // prevent parameter devide by 0
		setParameterInItsOwnRange(pROIX);
		pROIY.setMin(getROIYMin());
		pROIY.setMax(MAX(getROIYMax(),1)); // prevent parameter devide by 0
		setParameterInItsOwnRange(pROIY);
		
		pExposure.setMin(getExposureMin());
		pExposure.setMax(getExposureMaxForCurrentFrameRate());
		setParameterInItsOwnRange(pExposure);
		pAutoExposureTarget.setMin(getAutoExposureTargetMin());
		pAutoExposureTarget.setMax(getAutoExposureTargetMax());
		setParameterInItsOwnRange(pAutoExposureTarget);
		pAutoExposureAdjustTol.setMin(getAutoExposureAdjustTolMin());
		pAutoExposureAdjustTol.setMax(getAutoExposureAdjustTolMax());
		setParameterInItsOwnRange(pAutoExposureAdjustTol);
		pAutoExposureMinimum.setMin(getAutoExposureMinimumMin());
		pAutoExposureMinimum.setMax(getExposureMaxForCurrentFrameRate());
		setParameterInItsOwnRange(pAutoExposureMinimum);
		pAutoExposureMaximum.setMin(getAutoExposureMaximumMin());
		pAutoExposureMaximum.setMax(getExposureMaxForCurrentFrameRate());
		setParameterInItsOwnRange(pAutoExposureMaximum);
		pAutoExposureOutliers.setMin(getAutoExposureOutliersMin());
		pAutoExposureOutliers.setMax(getAutoExposureOutliersMax());
		setParameterInItsOwnRange(pAutoExposureOutliers);
		pAutoExposureRate.setMin(getAutoExposureRateMin());
		pAutoExposureRate.setMax(getAutoExposureRateMax());
		setParameterInItsOwnRange(pAutoExposureRate);
		
		pGain.setMin(getGainMin());
		pGain.setMax(getGainMax());
		setParameterInItsOwnRange(pGain);
		
		if (getPixelFormat() == OF_PIXELS_RGB) {
			pGamma.setMin(getGammaMin());
			pGamma.setMax(getGammaMax());
			setParameterInItsOwnRange(pGamma);
			pHue.setMin(getHueMin());
			pHue.setMax(getHueMax());
			setParameterInItsOwnRange(pHue);
			pSaturation.setMin(getSaturationMin());
			pSaturation.setMax(getSaturationMax());
			setParameterInItsOwnRange(pSaturation);
			
			pAutoGainTarget.setMin(getAutoGainTargetMin());
			pAutoGainTarget.setMax(getAutoGainTargetMax());
			setParameterInItsOwnRange(pAutoGainTarget);
			pAutoGainAdjustTol.setMin(getAutoGainAdjustTolMin());
			pAutoGainAdjustTol.setMax(getAutoGainAdjustTolMax());
			setParameterInItsOwnRange(pAutoGainAdjustTol);
			pAutoGainMinimum.setMin(getAutoGainMinimumMin());
			pAutoGainMinimum.setMax(getAutoGainMinimumMax());
			setParameterInItsOwnRange(pAutoGainMinimum);
			pAutoGainMaximum.setMin(getAutoGainMaximumMin());
			pAutoGainMaximum.setMax(getAutoGainMaximumMax());
			setParameterInItsOwnRange(pAutoGainMaximum);
			pAutoGainOutliers.setMin(getAutoGainOutliersMin());
			pAutoGainOutliers.setMax(getAutoGainOutliersMax());
			setParameterInItsOwnRange(pAutoGainOutliers);
			pAutoGainRate.setMin(getAutoGainRateMin());
			pAutoGainRate.setMax(getAutoGainRateMax());
			setParameterInItsOwnRange(pAutoGainRate);
			
			pWhiteBalanceRed.setMin(getWhiteBalanceRedMin());
			pWhiteBalanceRed.setMax(getWhiteBalanceRedMax());
			setParameterInItsOwnRange(pWhiteBalanceRed);
			pWhiteBalanceBlue.setMin(getWhiteBalanceBlueMin());
			pWhiteBalanceBlue.setMax(getWhiteBalanceBlueMax());
			setParameterInItsOwnRange(pWhiteBalanceBlue);
			pAutoWhiteBalanceRate.setMin(getAutoWhiteBalanceRateMin());
			pAutoWhiteBalanceRate.setMax(getAutoWhiteBalanceRateMax());
			setParameterInItsOwnRange(pAutoWhiteBalanceRate);
			pAutoWhiteBalanceAdjustTol.setMin(getAutoWhiteBalanceAdjustTolMin());
			pAutoWhiteBalanceAdjustTol.setMax(getAutoWhiteBalanceAdjustTolMax());
			setParameterInItsOwnRange(pAutoWhiteBalanceAdjustTol);
		}
	}
	
	void ParamCam::setParameterInItsOwnRange(ofParameter<int> &_parameter) {
		if (_parameter.get() < _parameter.getMin()) { _parameter.set(_parameter.getMin()); }
		else if (_parameter.get() > _parameter.getMax()) {_parameter.set(_parameter.getMax()); }
		else {
			bool bl = blockListeners;
			blockListeners = true;
			_parameter.set(_parameter.get()); // setParameterInItsOwnRange
			blockListeners = bl;
		}
	}
	
	void ParamCam::setParameterInItsOwnRange(ofParameter<float> &_parameter) {
		if (_parameter.get() < _parameter.getMin()) _parameter.set(_parameter.getMin());
		else if (_parameter.get() > _parameter.getMax()) _parameter.set(_parameter.getMax());
		else {
			bool bl = blockListeners;
			blockListeners = true;
			_parameter.set(_parameter.get()); // setParameterInItsOwnRange
			blockListeners = bl;
		}
	}
	
	
		//-- FRAMES ----------------------------------------------------------
	void ParamCam::triggeredListener(bool &_value)  {
		if (isActive()) {
			bool bT = Camera::getTriggered();
			if (_value != bT) {
				Camera::setTriggered(_value);
			
				pExposure.setMax(getExposureMaxForCurrentFrameRate());
				pAutoExposureMaximum.setMax(Camera::getAutoExposureMaximum());
				blockListeners = true;
				pFrameRate.set(Camera::getFrameRate());
				pExposure.set(Camera::getExposure());
				pAutoExposureMaximum.set(Camera::getAutoExposureMaximum());
				blockListeners = false;
			}
		}
	}
	
	void ParamCam::frameRateListener(float &_value) {
		if (isActive()) {
			_value = int(_value + .5);
			Camera::setFrameRate(_value);
			
			pExposure.setMax(getExposureMaxForCurrentFrameRate());
			pAutoExposureMaximum.setMax(Camera::getAutoExposureMaximum());
			blockListeners = true;
			pExposure.set(Camera::getExposure());
			pAutoExposureMaximum.set(Camera::getAutoExposureMaximum());
			blockListeners = false;
		}
	}
	
		//-- REGION OF INTEREST ----------------------------------------------
	void ParamCam::ROIWidthListener(int &_value) {
		if (isActive()) {
			Camera::setROIWidth(_value);
			
			pROIX.setMax(max(getROIXMax(), 1)); // prevent divide by 0
			setParameterInItsOwnRange(pROIX);
			
			pFrameRate.setMax(floor(getFrameRateMax()));
			setParameterInItsOwnRange(pFrameRate);
			Camera::setFrameRate(pFrameRate.get());
			pFps.setMax(floor(getFrameRateMax()));
			setParameterInItsOwnRange(pFps);
		}
	}
	
	void ParamCam::ROIHeightListener(int &_value) {
		if (isActive()) {
			Camera::setROIHeight(_value);
			
			pROIY.setMax(max(getROIYMax(), 1)); // prevent divide by 0
			setParameterInItsOwnRange(pROIY);
			
			pFrameRate.setMax(floor(getFrameRateMax()));
			setParameterInItsOwnRange(pFrameRate);
			Camera::setFrameRate(pFrameRate.get());
			pFps.setMax(floor(getFrameRateMax()));
			setParameterInItsOwnRange(pFps);
		}
	}
	
	void ParamCam::ROIXListener(int &_value) {
		if (isActive()) {
			Camera::setROIX(_value);
			
			pROIWidth.setMax(max(getROIWidthMax(), 1)); // prevent divide by 0
			setParameterInItsOwnRange(pROIWidth);
		}
	}
	
	void ParamCam::ROIYListener(int &_value) {
		if (isActive()) {
			Camera::setROIY(_value);
			
			pROIHeight.setMax(max(getROIHeightMax(), 1)); // prevent divide by 0
			setParameterInItsOwnRange(pROIHeight);
		}
	}
	
		//-- IP SETTINGS -----------------------------------------------------
	void ParamCam::enableIPSettings() {
		if (isActive()) {
			ofLog(OF_LOG_ERROR, "Camera: %lu: enableIPSettings(): can't enable IP settings while device is active", deviceID);
			return;
		}
		
		bIpParametersEnabled = true;
	}
	
	void ParamCam::pPersistentIpSwitchListener(bool & _value)	{
		if (bIpParametersEnabled) {
			if(!blockListeners) { Camera::setIpPersistent(_value); }
		}
		else {
			ofLog(OF_LOG_WARNING, "Camera: %lu: setIpPersistent(): IP parameters not enabled", deviceID);
		}
	}
	
	void ParamCam::setPersistentIpAdress(string _value)	{
		if (bIpParametersEnabled) {
			Camera::setPersistentIpAdress(_value);
			pPersistentIpAdress.set(_value);
		}
		else {
			ofLog(OF_LOG_WARNING, "Camera: %lu: setPersistentIpAdress(): IP parameters not enabled", deviceID);
		}
	}
	
	void ParamCam::setPersistentIpSubnetMask(string _value) {
		if (bIpParametersEnabled) {
			Camera::setPersistentIpSubnetMask(_value);
			pPersistentIpSubnetMask.set(_value);
		}
		else {
			ofLog(OF_LOG_WARNING, "Camera: %lu: setPersistentIpSubnetMask(): IP parameters not enabled", deviceID);
		}
	}
	
	void ParamCam::setPersistentIpGateway(string _value) {
		if (bIpParametersEnabled) {
			Camera::setPersistentIpGateway(_value);
			pPersistentIpGateway.set(_value);
		}
		else {
			ofLog(OF_LOG_WARNING, "Camera: %lu: setPersistentIpGateway(): IP parameters not enabled", deviceID);
		}
	}
	
	
	void ParamCam::pPersistentIpUpdateListener(bool &_value) {
		if(!blockListeners && _value) {
			Camera::updateIpSettings();
		}
		_value = false;
	}
	
	
		//-- RANGE -----------------------------------------------------------
	
	void ParamCam::inRange(ofParameter<int>& _parameter, int& _value) {
		if ( _value < _parameter.getMin() || _value > _parameter.getMax()) {
			int clampedValue = ofClamp(_value, _parameter.getMin(), _parameter.getMax());
			ofLogWarning("ParamCam") << _parameter.getName() << " out of range, clamping " << _value << " to " << clampedValue;
			_value = clampedValue;
		}
	}
	
	void ParamCam::inRange(ofParameter<float>& _parameter, float& _value) {
		if ( _value < _parameter.getMin() || _value > _parameter.getMax()) {
			int clampedValue = ofClamp(_value, _parameter.getMin(), _parameter.getMax());
			ofLogWarning("ParamCam") << _value << " for parameter " << _parameter.getName() << " out of range, clamping to " << clampedValue;
			_value = clampedValue;
		}
	}
}

