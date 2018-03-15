#include "ParameterConnector.h"


namespace ofxPvAPI {
	
		//------------------------------------------------------------------------
	bool ParameterConnector::setup() {
		return initInterface();
	}
	
		//------------------------------------------------------------------------
	bool ParameterConnector::initInterface() {
		Connector::initConnector();
		
		parameters.clear();
		string dID = "unknown";
		if (requestedDeviceID != -1)
			dID = ofToString(requestedDeviceID);
		parameters.setName("camera " + dID);
		parameters.add(reset.set("reset", false));
		reset.addListener(this, &ParameterConnector::resetListener);
		parameters.add(printAttributes.set("print features", false));
		printAttributes.addListener(this, &ParameterConnector::printAttributesListener);
			//		parameters.add(printIpSettings.set("print IP settings", false));
			//		printIpSettings.addListener(this, &ParameterConnector::printIpSettingsListener);
		parameters.add(resetParametersFromCam.set("load", false));
		resetParametersFromCam.addListener(this, &ParameterConnector::resetParametersFromCamListener);
		
		frameRateParameters.setName("framerate");
		frameRateParameters.add(pFps.set("fps", 0, 0, 60));
		frameRateParameters.add(pFrameDrop.set("drops per second", 0, 0, 60));
		frameRateParameters.add(pFrameLatency.set("avg latency (ms)", 0, 0, 60));
		frameRateParameters.add(pFrameMaxLatency.set("max latency (ms)", 0, 0, 60));
		frameRateParameters.add(pFrameMinLatency.set("min latency (ms)", 0, 0, 60));
		frameRateParameters.add(pFixedRate.set("fixed (or trigger)", false));
		pFixedRate.addListener(this, &ParameterConnector::fixedRateListener);
		frameRateParameters.add(pFrameRate.set("frame rate", 30, 2, 60));
		pFrameRate.addListener(this, &ParameterConnector::frameRateListener);
		parameters.add(frameRateParameters);
		
		roiParameters.setName("region of interest");
		roiParameters.add(pROIWidth.set("width", 640, 1, 640));
		pROIWidth.addListener(this, &ParameterConnector::ROIWidthListener);
		roiParameters.add(pROIHeight.set("height", 480, 1, 480));
		pROIHeight.addListener(this, &ParameterConnector::ROIHeightListener);
		roiParameters.add(pROIX.set("x", 0, 0, 1));
		pROIX.addListener(this, &ParameterConnector::ROIXListener);
		roiParameters.add(pROIY.set("y", 0, 0, 1));
		pROIY.addListener(this, &ParameterConnector::ROIYListener);
		parameters.add(roiParameters);
		
		exposureParameters.setName("exposure");
		exposureParameters.add(pExposure.set("exposure", 30, 0, 40000));
		pExposure.addListener(this, &ParameterConnector::exposureListener);
		autoExposureParameters.setName("auto exposure");
		autoExposureParameters.add(pAutoExposure.set("auto", true));
		pAutoExposure.addListener(this, &ParameterConnector::autoExposureListener);
		autoExposureParameters.add(pAutoExposureOnce.set("auto once", false));
		pAutoExposureOnce.addListener(this, &ParameterConnector::autoExposureOnceListener);
		autoExposureParameters.add(pAutoExposureTarget.set("target pct", 33, 0, 100));
		pAutoExposureTarget.addListener(this, &ParameterConnector::autoExposureTargetListener);
		autoExposureParameters.add(pAutoExposureRate.set("rate pct", 10, 1, 100));
		pAutoExposureRate.addListener(this, &ParameterConnector::autoExposureRateListener);
		autoExposureParameters.add(pAutoExposureAdjustTol.set("tolerance pct", 5, 0, 50));
		pAutoExposureAdjustTol.addListener(this, &ParameterConnector::autoExposureAdjustTolListener);
		autoExposureParameters.add(pAutoExposureOutliers.set("outliers 0.01pct", 0, 0, 1000));
		pAutoExposureOutliers.addListener(this, &ParameterConnector::autoExposureOutliersListener);
		autoExposureParameters.add(pAutoExposureMinimum.set("minimium μs", 30, 30, 1000000));
		pAutoExposureMinimum.addListener(this, &ParameterConnector::autoExposureMinimumListener);
		autoExposureParameters.add(pAutoExposureMaximum.set("maximum μs", 40000, 30, 1000000));
		pAutoExposureMaximum.addListener(this, &ParameterConnector::autoExposureMaximumListener);
		exposureParameters.add(autoExposureParameters);
		parameters.add(exposureParameters);
		
		if(getPixelFormat() != OF_PIXELS_RGB) {
			parameters.add(pGain.set("gain", 0, 0, 1));
			pGain.addListener(this, &ParameterConnector::gainListener);
		}
		else {
			gainParameters.setName("gain");
			gainParameters.add(pGain.set("gain", 0, 0, 30));
			pGain.addListener(this, &ParameterConnector::gainListener);
			autoGainParameters.setName("auto gain");
			autoGainParameters.add(pAutoGain.set("auto", false));
			pAutoGain.addListener(this, &ParameterConnector::autoGainListener);
			autoGainParameters.add(pAutoGainOnce.set("auto once", false));
			pAutoGainOnce.addListener(this, &ParameterConnector::autoGainOnceListener);
			autoGainParameters.add(pAutoGainTarget.set("target pct", 0, 1, 100));
			pAutoGainTarget.addListener(this, &ParameterConnector::autoGainTargetListener);
			autoGainParameters.add(pAutoGainRate.set("rate pct", 10, 1, 100));
			pAutoGainRate.addListener(this, &ParameterConnector::autoGainRateListener);
			autoGainParameters.add(pAutoGainAdjustTol.set("tolerance pct", 5, 0, 50));
			pAutoGainAdjustTol.addListener(this, &ParameterConnector::autoGainAdjustTolListener);
			autoGainParameters.add(pAutoGainOutliers.set("outliers 0.01pct", 0, 0, 1000));
			pAutoGainOutliers.addListener(this, &ParameterConnector::autoGainOutliersListener);
			autoGainParameters.add(pAutoGainMinimum.set("minimum dB", 0, 0, 30));
			pAutoGainMinimum.addListener(this, &ParameterConnector::autoGainMinimumListener);
			autoGainParameters.add(pAutoGainMaximum.set("maximum dB", 30, 0, 30));
			pAutoGainMaximum.addListener(this, &ParameterConnector::autoGainMaximumListener);
			gainParameters.add(autoGainParameters);
			parameters.add(gainParameters);
			
			whiteBalanceParameters.setName("white balance");
			whiteBalanceParameters.add(pWhiteBalanceRed.set("red", 190, 80, 300));
			pWhiteBalanceRed.addListener(this, &ParameterConnector::WhiteBalanceRedListener);
			whiteBalanceParameters.add(pWhiteBalanceBlue.set("blue", 190, 80, 300));
			pWhiteBalanceBlue.addListener(this, &ParameterConnector::WhiteBalanceBlueListener);
			autoWhiteBalanceParameters.setName("auto white balance");
			autoWhiteBalanceParameters.add(pAutoWhiteBalance.set("auto", false));
			pAutoWhiteBalance.addListener(this, &ParameterConnector::autoWhiteBalanceListener);
			autoWhiteBalanceParameters.add(pAutoWhiteBalanceOnce.set("auto once", false));
			pAutoWhiteBalanceOnce.addListener(this, &ParameterConnector::autoWhiteBalanceOnceListener);
			autoWhiteBalanceParameters.add(pAutoWhiteBalanceRate.set("rate pct", 100, 1, 100));
			pAutoWhiteBalanceRate.addListener(this, &ParameterConnector::autoWhiteBalanceRateListener);
			autoWhiteBalanceParameters.add(pAutoWhiteBalanceAdjustTol.set("ajust tolerance pct", 1, 1, 100));
			pAutoWhiteBalanceAdjustTol.addListener(this, &ParameterConnector::autoWhiteBalanceAdjustTolListener);
			whiteBalanceParameters.add(autoWhiteBalanceParameters);
			parameters.add(whiteBalanceParameters);
			
			gammaHueSaturationParameters.setName("gamma hue saturation");
			gammaHueSaturationParameters.add(pGamma.set("gamma", 1.0, 0.5, 1.5));
			pGamma.addListener(this, &ParameterConnector::gammaListener);
			gammaHueSaturationParameters.add(pHue.set("hue", 0.0, -10.0, 10.0));
			pHue.addListener(this, &ParameterConnector::hueListener);
			gammaHueSaturationParameters.add(pSaturation.set("saturation", 1.0, 0.0, 2.0));
			pSaturation.addListener(this, &ParameterConnector::saturationListener);
			parameters.add(gammaHueSaturationParameters);
		}
		
		ipParameters.setName("IP");
		ipParameters.add(pIpPersistent.set("persistent", "no"));
		ipParameters.add(pIpAdress.set("adress", "0.0.0.0"));
		ipParameters.add(pIpSubnet.set("subnet", "0.0.0.0"));
		ipParameters.add(pIpGateway.set("gateway", "0.0.0.0"));
		parameters.add(ipParameters);
		
		connectParameters.setName("connector");
		connectParameters.add(doConnect.set("connect", false));
		doConnect.addListener(this, &ParameterConnector::doConnectListener);
		connectParameters.add(doDisconnect.set("disconnect", false));
		doDisconnect.addListener(this, &ParameterConnector::doDisconnectListener);
		autoConnectParameters.setName("auto connector");
		autoConnectParameters.add(doAutoConnect.set("auto connect", false));
		doAutoConnect.addListener(this, &ParameterConnector::doAutoConnectListener);
		autoConnectParameters.add(autoConnectAttempts.set("max attempts", 100, 0, 100));
		autoConnectAttempts.addListener(this, &ParameterConnector::autoConnectAttemptsListener);
		autoConnectParameters.add(autoConnectCounter.set("attempts made", 0, 0, 100));
		autoConnectParameters.add(autoConnectInterval.set("interval", 0, 0, 60));
		connectParameters.add(autoConnectParameters);
		parameters.add(connectParameters);
		
		
		bLoadFromInterface = true;
		blockListeners = false;
		
		return true;
	}
	
		//------------------------------------------------------------------------
	void ParameterConnector::update() {
		Connector::update();
		
		if (bInitialized) {
			pFps = getFps();
			pFrameDrop = getFrameDrop();
			pFrameLatency = getLatency();
			pFrameMaxLatency = getMaxLatency();
			pFrameMinLatency = getMinLatency();
			
			if (!bLoadFromInterface) {
				updateParametersFromCam();
			} else {
				bLoadFromInterface = false;
				setAllParametersFromInterface();
			}
		}
		
		doAutoConnect.set(getAutoConnect());
		if (!bInitialized) {
			autoConnectAttempts.set(getAutoConnectAttempts());
			autoConnectCounter.set(getAutoConnectCounter());
			autoConnectInterval.set(getAutoConnectInterval());
		}
	}
	
		//------------------------------------------------------------------------
	void ParameterConnector::updateParametersFromCam() {
		blockListeners = true;
		if (pAutoExposure.get()) {
			pExposure.set(getExposure());
		}
		if (pAutoGain.get()) {
			pGain.set(getGain());
		}
		if (pAutoWhiteBalance.get()) {
			pWhiteBalanceRed.set(getWhiteBalanceRed());
			pWhiteBalanceBlue.set(getWhiteBalanceBlue());
		}
		
		pFrameRate.setMin(ceil(getFrameRateMin()));
		pFrameRate.setMax(floor(getFrameRateMax()));
		setParameterInItsOwnRange(pFrameRate);
		
		pExposure.setMax(getExposureMaxForCurrentFrameRate());				//  <<
		setParameterInItsOwnRange(pExposure);
		
		blockListeners = false;
		
			//		setParametersRange();
	}
	
		//------------------------------------------------------------------------
	void ParameterConnector::setAllParametersFromCam() {
		blockListeners = true;
		
		pFrameRate.set(getFrameRate());
		pROIWidth.set(getROIWidth());
		pROIHeight.set(getROIHeight());
		pROIX.set(getROIX());
		pROIY.set(getROIY());
		
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
		
		if (getIpPersistent())
			pIpPersistent.set("yes");
		else
			pIpPersistent.set("no");
		
		pIpAdress.set(getIpAdress());
		pIpSubnet.set(getIpSubnet());
		pIpGateway.set(getIpGateway());
		
		blockListeners = false;
		
		setParametersRange();
	}
	
	void ParameterConnector::setAllParametersFromInterface() {
		
		setFrameRate(pFrameRate.get());
		
		setROIWidth(pROIWidth.get());
		setROIHeight(pROIHeight.get());
		setROIX(pROIX.get());
		setROIY(pROIY.get());
		
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
		
		setParametersRange();
	}
	
		//------------------------------------------------------------------------
	void ParameterConnector::setParametersRange() {
		
		pFrameRate.setMin(ceil(getFrameRateMin()));
		pFrameRate.setMax(floor(getFrameRateMax()));
		setParameterInItsOwnRange(pFrameRate);
		
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
	
	void ParameterConnector::setParameterInItsOwnRange(ofParameter<int> &_parameter) {
		if (_parameter.get() < _parameter.getMin()) { _parameter.set(_parameter.getMin()); }
		else if (_parameter.get() > _parameter.getMax()) {_parameter.set(_parameter.getMax()); }
		else { _parameter.set(_parameter.get()); } // dirty way to update, should go
	}
	
	void ParameterConnector::setParameterInItsOwnRange(ofParameter<float> &_parameter) {
		if (_parameter.get() < _parameter.getMin()) _parameter.set(_parameter.getMin());
		if (_parameter.get() > _parameter.getMax()) _parameter.set(_parameter.getMax());
	}
	
	
		//-- FRAMES ----------------------------------------------------------
	
	void ParameterConnector::frameRateListener(float &_value) {
		if (bInitialized) {
			Camera::setFrameRate(_value);
			if (pExposure != getExposure()) pExposure.set(getExposure()); // only if autoexposure = off
		}
	}
	
		//-- REGION OF INTEREST ----------------------------------------------
	void ParameterConnector::ROIWidthListener(int &_value) {
		if (bInitialized) {
			Camera::setROIWidth(_value);
			pROIX.setMax(max(getROIXMax(), 1)); // prevent divide by 0
			if (pROIX != getROIX()) pROIX.set(getROIX());
		}
	}
	
	void ParameterConnector::ROIHeightListener(int &_value) {
		if (bInitialized) {
			Camera::setROIHeight(_value);
			pROIY.setMax(max(getROIYMax(), 1)); // prevent divide by 0
			if (pROIY != getROIY()) pROIY.set(getROIY());
		}
	}
	
	void ParameterConnector::ROIXListener(int &_value) {
		if (bInitialized) {
			Camera::setROIX(_value);
			pROIWidth.setMax(max(getROIWidthMax(), 1)); // prevent divide by 0
			if (pROIWidth != getROIWidth()) pROIWidth.set(getROIWidth());
		}
	}
	
	void ParameterConnector::ROIYListener(int &_value) {
		if (bInitialized) {
			Camera::setROIY(_value);
			pROIHeight.setMax(max(getROIHeightMax(), 1)); // prevent divide by 0
			if (pROIHeight != getROIHeight()) pROIHeight.set(getROIHeight());
		}
	}
	
	
		//-- RANGE -----------------------------------------------------------
	
	void ParameterConnector::inRange(ofParameter<int>& _parameter, int& _value) {
		if ( _value < _parameter.getMin() || _value > _parameter.getMax()) {
			int clampedValue = ofClamp(_value, _parameter.getMin(), _parameter.getMax());
			ofLogWarning("ParameterConnector") << _value << " for parameter " << _parameter.getName() << "out of range, clamping to " << clampedValue;
			_value = clampedValue;
		}
	}
	
	void ParameterConnector::inRange(ofParameter<float>& _parameter, float& _value) {
		if ( _value < _parameter.getMin() || _value > _parameter.getMax()) {
			int clampedValue = ofClamp(_value, _parameter.getMin(), _parameter.getMax());
			ofLogWarning("ParameterConnector") << _value << " for parameter " << _parameter.getName() << "out of range, clamping to " << clampedValue;
			_value = clampedValue;
		}
	}
}

