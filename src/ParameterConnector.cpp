#include "ParameterConnector.h"


namespace ofxProsilica {
	
	//--------------------------------------------------------------
	bool ParameterConnector::setup() {
		return initInterface();
	}
	
	//--------------------------------------------------------------
	bool ParameterConnector::initInterface() {
		Connector::initConnector();
		
		parameters.clear();
		string dID = "unknown";
		if (requestedDeviceID != -1)
			dID = ofToString(requestedDeviceID);
		parameters.setName("camera " + dID);
		parameters.add(reset.set("reset", false));
		reset.addListener(this, &ParameterConnector::resetListner);
		parameters.add(printAttributes.set("print features", false));
		printAttributes.addListener(this, &ParameterConnector::printAttributesListner);
//		parameters.add(printIpSettings.set("print IP settings", false));
//		printIpSettings.addListener(this, &ParameterConnector::printIpSettingsListner);
		parameters.add(resetParametersFromCam.set("load", false));
		resetParametersFromCam.addListener(this, &ParameterConnector::resetParametersFromCamListner);
		
		roiParameters.setName("region of interest");
		roiParameters.add(ROIWidth.set("width", 640, 1, 640));
		ROIWidth.addListener(this, &ParameterConnector::ROIWidthListner);
		roiParameters.add(ROIHeight.set("height", 480, 1, 480));
		ROIHeight.addListener(this, &ParameterConnector::ROIHeightListner);
		roiParameters.add(ROIX.set("x", 0, 0, 1));
		ROIX.addListener(this, &ParameterConnector::ROIXListner);
		roiParameters.add(ROIY.set("y", 0, 0, 1));
		ROIY.addListener(this, &ParameterConnector::ROIYListner);
		parameters.add(roiParameters);
		
		parameters.add(frameRate.set("framerate", 25, 1, 60));
		frameRate.addListener(this, &ParameterConnector::frameRateListner);
		
		exposureParameters.setName("exposure");
		exposureParameters.add(exposure.set("exposure", 30, 0, 40000));
		exposure.addListener(this, &ParameterConnector::exposureListner);
		autoExposureParameters.setName("auto exposure");
		autoExposureParameters.add(autoExposure.set("auto", true));
		autoExposure.addListener(this, &ParameterConnector::autoExposureListner);
		autoExposureParameters.add(autoExposureOnce.set("auto once", false));
		autoExposureOnce.addListener(this, &ParameterConnector::autoExposureOnceListner);
		autoExposureParameters.add(autoExposureTarget.set("target pct", 33, 0, 100));
		autoExposureTarget.addListener(this, &ParameterConnector::autoExposureTargetListner);
		autoExposureParameters.add(autoExposureRate.set("rate pct", 10, 1, 100));
		autoExposureRate.addListener(this, &ParameterConnector::autoExposureRateListner);
		autoExposureParameters.add(autoExposureAdjustTol.set("tolerance pct", 5, 0, 50));
		autoExposureAdjustTol.addListener(this, &ParameterConnector::autoExposureAdjustTolListner);
		autoExposureParameters.add(autoExposureOutliers.set("outliers 0.01pct", 0, 0, 1000));
		autoExposureOutliers.addListener(this, &ParameterConnector::autoExposureOutliersListner);
		autoExposureParameters.add(autoExposureMinimum.set("minimium μs", 30, 30, 1000000));
		autoExposureMinimum.addListener(this, &ParameterConnector::autoExposureMinimumListner);
		autoExposureParameters.add(autoExposureMaximum.set("maximum μs", 40000, 30, 1000000));
		autoExposureMaximum.addListener(this, &ParameterConnector::autoExposureMaximumListner);
		exposureParameters.add(autoExposureParameters);
		parameters.add(exposureParameters);
		
		if(getPixelFormat() != OF_PIXELS_RGB) {
			parameters.add(gain.set("gain", 0, 0, 1));
			gain.addListener(this, &ParameterConnector::gainListner);
		}
		else {
			gainParameters.setName("gain");
			gainParameters.add(gain.set("gain", 0, 0, 30));
			gain.addListener(this, &ParameterConnector::gainListner);
			autoGainParameters.setName("auto gain");
			autoGainParameters.add(autoGain.set("auto", false));
			autoGain.addListener(this, &ParameterConnector::autoGainListner);
			autoGainParameters.add(autoGainOnce.set("auto once", false));
			autoGainOnce.addListener(this, &ParameterConnector::autoGainOnceListner);
			autoGainParameters.add(autoGainTarget.set("target pct", 0, 1, 100));
			autoGainTarget.addListener(this, &ParameterConnector::autoGainTargetListner);
			autoGainParameters.add(autoGainRate.set("rate pct", 10, 1, 100));
			autoGainRate.addListener(this, &ParameterConnector::autoGainRateListner);
			autoGainParameters.add(autoGainAdjustTol.set("tolerance pct", 5, 0, 50));
			autoGainAdjustTol.addListener(this, &ParameterConnector::autoGainAdjustTolListner);
			autoGainParameters.add(autoGainOutliers.set("outliers 0.01pct", 0, 0, 1000));
			autoGainOutliers.addListener(this, &ParameterConnector::autoGainOutliersListner);
			autoGainParameters.add(autoGainMinimum.set("minimum dB", 0, 0, 30));
			autoGainMinimum.addListener(this, &ParameterConnector::autoGainMinimumListner);
			autoGainParameters.add(autoGainMaximum.set("maximum dB", 30, 0, 30));
			autoGainMaximum.addListener(this, &ParameterConnector::autoGainMaximumListner);
			gainParameters.add(autoGainParameters);
			parameters.add(gainParameters);
			
			whiteBalanceParameters.setName("white balance");
			whiteBalanceParameters.add(whiteBalanceRed.set("red", 190, 80, 300));
			whiteBalanceRed.addListener(this, &ParameterConnector::WhiteBalanceRedListner);
			whiteBalanceParameters.add(whiteBalanceBlue.set("blue", 190, 80, 300));
			whiteBalanceBlue.addListener(this, &ParameterConnector::WhiteBalanceBlueListner);
			autoWhiteBalanceParameters.setName("auto white balance");
			autoWhiteBalanceParameters.add(autoWhiteBalance.set("auto", false));
			autoWhiteBalance.addListener(this, &ParameterConnector::autoWhiteBalanceListner);
			autoWhiteBalanceParameters.add(autoWhiteBalanceOnce.set("auto once", false));
			autoWhiteBalanceOnce.addListener(this, &ParameterConnector::autoWhiteBalanceOnceListner);
			autoWhiteBalanceParameters.add(autoWhiteBalanceRate.set("rate pct", 100, 1, 100));
			autoWhiteBalanceRate.addListener(this, &ParameterConnector::autoWhiteBalanceRateListner);
			autoWhiteBalanceParameters.add(autoWhiteBalanceAdjustTol.set("ajust tolerance pct", 1, 1, 100));
			autoWhiteBalanceAdjustTol.addListener(this, &ParameterConnector::autoWhiteBalanceAdjustTolListner);
			whiteBalanceParameters.add(autoWhiteBalanceParameters);
			parameters.add(whiteBalanceParameters);
			
			gammaHueSaturationParameters.setName("gamma hue saturation");
			gammaHueSaturationParameters.add(gamma.set("gamma", 1.0, 0.5, 1.5));
			gamma.addListener(this, &ParameterConnector::gammaListner);
			gammaHueSaturationParameters.add(hue.set("hue", 0.0, -10.0, 10.0));
			hue.addListener(this, &ParameterConnector::hueListner);
			gammaHueSaturationParameters.add(saturation.set("saturation", 1.0, 0.0, 2.0));
			saturation.addListener(this, &ParameterConnector::saturationListner);
			parameters.add(gammaHueSaturationParameters);
		}
		
		ipParameters.setName("IP");
		ipParameters.add(ipPersistent.set("persistent", "no"));
		ipParameters.add(ipAdress.set("adress", "0.0.0.0"));
		ipParameters.add(ipSubnet.set("subnet", "0.0.0.0"));
		ipParameters.add(ipGateway.set("gateway", "0.0.0.0"));
		parameters.add(ipParameters);
		
		connectParameters.setName("connector");
		connectParameters.add(doConnect.set("connect", false));
		doConnect.addListener(this, &ParameterConnector::doConnectListner);
		connectParameters.add(doDisconnect.set("disconnect", false));
		doDisconnect.addListener(this, &ParameterConnector::doDisconnectListner);
		autoConnectParameters.setName("auto connector");
		autoConnectParameters.add(doAutoConnect.set("auto connect", false));
		doAutoConnect.addListener(this, &ParameterConnector::doAutoConnectListner);
		autoConnectParameters.add(autoConnectAttempts.set("max attempts", 100, 0, 100));
		autoConnectAttempts.addListener(this, &ParameterConnector::autoConnectAttemptsListner);
		autoConnectParameters.add(autoConnectCounter.set("attempts made", 0, 0, 100));
		autoConnectParameters.add(autoConnectInterval.set("interval", 0, 0, 60));
		connectParameters.add(autoConnectParameters);
		parameters.add(connectParameters);
		
		
		bLoadFromInterface = true;
		blockParameters = true;
		
		return true;
	}
	
	//--------------------------------------------------------------
	void ParameterConnector::update() {
		Connector::update();
        
        if (bInitialized) {
			if (!bLoadFromInterface) {
				if (Connector::isFrameNew()) {
//					blockParameters = false;
					setAllParametersFromCam(); // this reduces bandwidth, serious drop in framerate  (~10fps) on higher resolutions
				}
			} else {
				bLoadFromInterface = false;
				setAllParametersFromInterface();
			}
		} else {
			;//	bLoadFromInterface = true;  // unfortunately we still get values from camera after connection is lost
		}
		
		doAutoConnect.set(getAutoConnect());
		if (!bInitialized) {
			autoConnectAttempts.set(getAutoConnectAttempts());
			autoConnectCounter.set(getAutoConnectCounter());
			autoConnectInterval.set(getAutoConnectInterval());
		}
	}
	
	//--------------------------------------------------------------
	void ParameterConnector::setAllParametersFromCam() {
		blockParameters = true;
		
		frameRate.set(getFrameRate());
		ROIWidth.set(getROIWidth());
		ROIHeight.set(getROIHeight());
		ROIX.set(getROIX());
		ROIY.set(getROIY());
		
		exposure.set(getExposure());
		
		autoExposure.set(getAutoExposure());
		autoExposureOnce.set(getAutoExposureOnce());
		autoExposureAdjustTol.set(getAutoExposureAdjustTol());
		autoExposureMinimum.set(getAutoExposureMinimum());
		autoExposureMaximum.set(getAutoExposureMaximum());
		autoExposureOutliers.set(getAutoExposureOutliers());
		autoExposureRate.set(getAutoExposureRate());
		autoExposureTarget.set(getAutoExposureTarget());
				
		gain = getGain();
		if (getPixelFormat() == OF_PIXELS_RGB) {
			gamma.set(getGamma());
			hue.set(getHue());
			saturation.set(getSaturation());
			
			gain.set(getGain());
			autoGain.set(getAutoGain());
			autoGainOnce.set(getAutoGainOnce());
			autoGainAdjustTol.set(getAutoGainAdjustTol());
			autoGainMinimum.set(getAutoGainMinimum());
			autoGainMaximum.set(getAutoGainMaximum());
			autoGainOutliers.set(getAutoGainOutliers());
			autoGainRate.set(getAutoGainRate());
			autoGainTarget.set(getAutoGainTarget());
			
			whiteBalanceRed.set(getWhiteBalanceRed());
			whiteBalanceBlue.set(getWhiteBalanceBlue());
			autoWhiteBalance.set(getAutoWhiteBalance());
			autoWhiteBalanceOnce.set(getAutoWhiteBalanceOnce());
			autoWhiteBalanceRate.set(getAutoWhiteBalanceRate());
			autoWhiteBalanceAdjustTol.set(getAutoWhiteBalanceAdjustTol());
		}
		
		if (getIpPersistent())
			ipPersistent.set("yes");
		else
			ipPersistent.set("no");
			
		ipAdress.set(getIpAdress());
		ipSubnet.set(getIpSubnet());
		ipGateway.set(getIpGateway());
		
		blockParameters = false;
		
		setParametersRange();
	}
	
	//--------------------------------------------------------------
	void ParameterConnector::setAllParametersFromInterface() {
		
		setFrameRate(frameRate.get());
        setROIWidth(ROIWidth.get());
        setROIHeight(ROIHeight.get());
        setROIX(ROIX.get());
        setROIY(ROIY.get());
		
        setExposure(exposure.get());
        setAutoExposure(autoExposure.get());
        setAutoExposureTarget(autoExposureTarget.get());
        setAutoExposureRate(autoExposureRate.get());
		setAutoExposureAdjustTol(autoExposureAdjustTol.get());
		setAutoExposureMinimum(autoExposureMinimum.get());
		setAutoExposureMaximum(autoExposureMaximum.get());
        setAutoExposureOutliers(autoExposureOutliers.get());
		
		setGain(gain.get());
		if (getPixelFormat() == OF_PIXELS_RGB) {
			setGamma(gamma.get());
			setHue(hue.get());
			setSaturation(saturation.get());
			
			setGain(gain.get());
			setAutoGain(autoGain.get());
			setAutoGainAdjustTol(autoGainAdjustTol.get());
			setAutoGainMinimum(autoGainMinimum.get());
			setAutoGainMaximum(autoGainMaximum.get());
			setAutoGainOutliers(autoGainOutliers.get());
			setAutoGainRate(autoGainRate.get());
			setAutoGainTarget(autoGainTarget.get());
			
			setWhiteBalanceRed(whiteBalanceRed.get());
			setWhiteBalanceBlue(whiteBalanceBlue.get());
			setAutoWhiteBalance(autoWhiteBalance.get());
			setAutoWhiteBalanceRate(autoWhiteBalanceRate.get());
			setAutoWhiteBalanceAdjustTol(autoWhiteBalanceAdjustTol.get());
		}
		
		setParametersRange();
	}
	
	//--------------------------------------------------------------
	void ParameterConnector::setParametersRange() {
		
		frameRate.setMin(ceil(getFrameRateMin()));
		frameRate.setMax(floor(getFrameRateMax()));
		setParameterInItsOwnRange(frameRate);
		
		ROIWidth.setMin(getROIWidthMin());
		ROIWidth.setMax(getROIWidthMax());
		setParameterInItsOwnRange(ROIWidth);
		ROIHeight.setMin(getROIHeightMin());
		ROIHeight.setMax(getROIHeightMax());
		setParameterInItsOwnRange(ROIHeight);
		ROIX.setMin(getROIXMin());
		ROIX.setMax(MAX(getROIXMax(),1)); // prevent parameter devide by 0
		setParameterInItsOwnRange(ROIX);
		ROIY.setMin(getROIYMin());
		ROIY.setMax(MAX(getROIYMax(),1)); // prevent parameter devide by 0
		setParameterInItsOwnRange(ROIY);
		
		exposure.setMin(getExposureMin());
		exposure.setMax(getAutoExposureMaxForCurrentFrameRate());				//  <<
		setParameterInItsOwnRange(exposure);
		autoExposureTarget.setMin(getAutoExposureTargetMin());
		autoExposureTarget.setMax(getAutoExposureTargetMax());
		setParameterInItsOwnRange(autoExposureTarget);
		autoExposureAdjustTol.setMin(getAutoExposureAdjustTolMin());
		autoExposureAdjustTol.setMax(getAutoExposureAdjustTolMax());
		setParameterInItsOwnRange(autoExposureAdjustTol);
		autoExposureMinimum.setMin(getAutoExposureMinimumMin());
		autoExposureMinimum.setMax(getAutoExposureMaxForCurrentFrameRate());
		setParameterInItsOwnRange(autoExposureMinimum);
		autoExposureMaximum.setMin(getAutoExposureMaximumMin());
		autoExposureMaximum.setMax(getAutoExposureMaxForCurrentFrameRate());
		setParameterInItsOwnRange(autoExposureMaximum);
		autoExposureOutliers.setMin(getAutoExposureOutliersMin());
		autoExposureOutliers.setMax(getAutoExposureOutliersMax());
		setParameterInItsOwnRange(autoExposureOutliers);
		autoExposureRate.setMin(getAutoExposureRateMin());
		autoExposureRate.setMax(getAutoExposureRateMax());
		setParameterInItsOwnRange(autoExposureRate);
		
		gain.setMin(getGainMin());
		gain.setMax(getGainMax());
		setParameterInItsOwnRange(gain);
		if (getPixelFormat() == OF_PIXELS_RGB) {
			gamma.setMin(getGammaMin());
			gamma.setMax(getGammaMax());
			setParameterInItsOwnRange(gamma);
			hue.setMin(getHueMin());
			hue.setMax(getHueMax());
			setParameterInItsOwnRange(hue);
			saturation.setMin(getSaturationMin());
			saturation.setMax(getSaturationMax());
			setParameterInItsOwnRange(saturation);
			
			autoGainTarget.setMin(getAutoGainTargetMin());
			autoGainTarget.setMax(getAutoGainTargetMax());
			setParameterInItsOwnRange(autoGainTarget);
			autoGainAdjustTol.setMin(getAutoGainAdjustTolMin());
			autoGainAdjustTol.setMax(getAutoGainAdjustTolMax());
			setParameterInItsOwnRange(autoGainAdjustTol);
			autoGainMinimum.setMin(getAutoGainMinimumMin());
			autoGainMinimum.setMax(getAutoGainMinimumMax());
			setParameterInItsOwnRange(autoGainMinimum);
			autoGainMaximum.setMin(getAutoGainMaximumMin());
			autoGainMaximum.setMax(getAutoGainMaximumMax());
			setParameterInItsOwnRange(autoGainMaximum);
			autoGainOutliers.setMin(getAutoGainOutliersMin());
			autoGainOutliers.setMax(getAutoGainOutliersMax());
			setParameterInItsOwnRange(autoGainOutliers);
			autoGainRate.setMin(getAutoGainRateMin());
			autoGainRate.setMax(getAutoGainRateMax());
			setParameterInItsOwnRange(autoGainRate);
			
			whiteBalanceRed.setMin(getWhiteBalanceRedMin());
			whiteBalanceRed.setMax(getWhiteBalanceRedMax());
			setParameterInItsOwnRange(whiteBalanceRed);
			whiteBalanceBlue.setMin(getWhiteBalanceBlueMin());
			whiteBalanceBlue.setMax(getWhiteBalanceBlueMax());
			setParameterInItsOwnRange(whiteBalanceBlue);
			autoWhiteBalanceRate.setMin(getAutoWhiteBalanceRateMin());
			autoWhiteBalanceRate.setMax(getAutoWhiteBalanceRateMax());
			setParameterInItsOwnRange(autoWhiteBalanceRate);
			autoWhiteBalanceAdjustTol.setMin(getAutoWhiteBalanceAdjustTolMin());
			autoWhiteBalanceAdjustTol.setMax(getAutoWhiteBalanceAdjustTolMax());
			setParameterInItsOwnRange(autoWhiteBalanceAdjustTol);
		}
	}
	
	//--------------------------------------------------------------
	void ParameterConnector::setParameterInItsOwnRange(ofParameter<int> &_parameter) {
		if (_parameter.get() < _parameter.getMin()) _parameter.set(_parameter.getMin());
		if (_parameter.get() > _parameter.getMax()) _parameter.set(_parameter.getMax());
	}
	
	//--------------------------------------------------------------
	void ParameterConnector::setParameterInItsOwnRange(ofParameter<float> &_parameter) {
		if (_parameter.get() < _parameter.getMin()) _parameter.set(_parameter.getMin());
		if (_parameter.get() > _parameter.getMax()) _parameter.set(_parameter.getMax());
	}
	
	//--------------------------------------------------------------
	void ParameterConnector::setFrameRate(float rate) {
		if (bInitialized) {
			Camera::setFrameRate(rate);
			setAutoExposureRangeFromFrameRate();
			if (getExposure() > getAutoExposureMaxForCurrentFrameRate())
				setExposure(getAutoExposureMaxForCurrentFrameRate());
		}
	}
	
}
