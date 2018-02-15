#pragma once

#include "ofMain.h"
#include "Connector.h"

namespace ofxProsilica {
	
	class ParameterConnector : public Connector {
	public:
		
		ParameterConnector() {;}
		virtual ~ParameterConnector() {;}
		
		bool	setup();
		void	update();
		
		void	setFrameRate(float _value);
        bool    getShowMe() { return showMe.get(); }
		
		ofParameterGroup	parameters;
		
		ofParameter<bool>	showMe;
		ofParameter<bool>	printAttributes;
		ofParameter<bool>	printIpSettings;
		ofParameter<bool>	reset;
		ofParameter<bool>	resetParametersFromCam;
		
		ofParameter<float>	frameRate;
		
		ofParameter<int>	ROIWidth;
		ofParameter<int>	ROIHeight;
		ofParameter<int>	ROIX;
		ofParameter<int>	ROIY;
		
		ofParameter<int>	exposure;
		ofParameter<bool>	autoExposureOnce;
		ofParameter<bool>	autoExposure;
		ofParameter<int>	autoExposureTarget;
		ofParameter<int>	autoExposureRate;
		ofParameter<int>	autoExposureAdjustTol;
		ofParameter<int>	autoExposureOutliers;
		ofParameter<int>	autoExposureMinimum;
		ofParameter<int>	autoExposureMaximum;
		
		ofParameter<int>	gain;
		ofParameter<bool>	autoGain;
		ofParameter<bool>	autoGainOnce;
		ofParameter<int>	autoGainTarget;
		ofParameter<int>	autoGainRate;
		ofParameter<int>	autoGainAdjustTol;
		ofParameter<int>	autoGainOutliers;
		ofParameter<int>	autoGainMinimum;
		ofParameter<int>	autoGainMaximum;
		
		ofParameter<int>	whiteBalanceRed;
		ofParameter<int>	whiteBalanceBlue;
		ofParameter<bool>	autoWhiteBalance;
		ofParameter<bool>	autoWhiteBalanceOnce;
		ofParameter<int>	autoWhiteBalanceRate;
		ofParameter<int>	autoWhiteBalanceAdjustTol;
		
		ofParameter<float>	gamma;
		ofParameter<float>	hue;
		ofParameter<float>	saturation;
		
		ofParameter<string>	ipPersistent;
		ofParameter<string>	ipAdress;
		ofParameter<string>	ipSubnet;
		ofParameter<string>	ipGateway;
		
		ofParameter<bool>	doConnect;
		ofParameter<bool>	doDisconnect;
		ofParameter<bool>	doAutoConnect;
		ofParameter<int>	autoConnectAttempts;
		ofParameter<int>	autoConnectCounter;
		ofParameter<float>	autoConnectInterval;
		
	private:
		ofParameterGroup	roiParameters;
		ofParameterGroup	exposureParameters;
		ofParameterGroup	autoExposureParameters;
		ofParameterGroup	gainParameters;
		ofParameterGroup	autoGainParameters;
		ofParameterGroup	whiteBalanceParameters;
		ofParameterGroup	autoWhiteBalanceParameters;
		ofParameterGroup	gammaHueSaturationParameters;
		ofParameterGroup	ipParameters;
		ofParameterGroup	connectParameters;
		ofParameterGroup	autoConnectParameters;
		
		void	printAttributesListner(bool & _value)	{ if(!blockParameters && _value) listAttributes(); _value = false; }
		void	printIpSettingsListner(bool & _value)	{ if(!blockParameters && _value) listIpSettings(); _value = false; }
		void	resetListner(bool & _value)				{ if(!blockParameters && _value) { resetAttributes(); setAllParametersFromCam(); } _value = false; }
		void	resetParametersFromCamListner(bool & _value) { if(!blockParameters && _value) setAllParametersFromCam(); _value = false; }
		
		void	frameRateListner(float& _value)			{ if(!blockParameters) setFrameRate(float(int(_value))); }
		
		void	ROIWidthListner(int& _value)			{ if(!blockParameters) setROIWidth(_value); }
		void	ROIHeightListner(int& _value)			{ if(!blockParameters) setROIHeight(_value); }
		void	ROIXListner(int& _value)				{ if(!blockParameters) setROIX(_value); }
		void	ROIYListner(int& _value)				{ if(!blockParameters) setROIY(_value); }
		
		void	exposureListner(int& _value)			{ if(!blockParameters) setExposure(_value); }
		void	autoExposureOnceListner(bool& _value)	{ if(!blockParameters) setAutoExposureOnce(_value); }
		void	autoExposureListner(bool& _value)		{ if(!blockParameters) setAutoExposure(_value); }
		void	autoExposureTargetListner(int& _value)	{ if(!blockParameters) setAutoExposureTarget(_value); }
		void	autoExposureRateListner(int& _value)	{ if(!blockParameters) setAutoExposureRate(_value); }
		void	autoExposureAdjustTolListner(int& _value){if(!blockParameters) setAutoExposureAdjustTol(_value); }
		void	autoExposureOutliersListner(int& _value){ if(!blockParameters) setAutoExposureOutliers(_value); }
		void	autoExposureMinimumListner(int& _value)	{ if(!blockParameters) setAutoExposureMinimum(_value); }
		void	autoExposureMaximumListner(int& _value)	{ if(!blockParameters) setAutoExposureMaximum(_value); }
		
		void	gainListner(int& _value)				{ if(!blockParameters) setGain(_value); }
		void	autoGainListner(bool& _value)			{ if(!blockParameters) setAutoGain(_value); }
		void	autoGainOnceListner(bool& _value)		{ if(!blockParameters) setAutoGainOnce(_value); }
		void	autoGainTargetListner(int& _value)		{ if(!blockParameters) setAutoGainTarget(_value); }
		void	autoGainRateListner(int& _value)		{ if(!blockParameters) setAutoGainRate(_value); }
		void	autoGainAdjustTolListner(int& _value)	{ if(!blockParameters) setAutoGainAdjustTol(_value); }
		void	autoGainOutliersListner(int& _value)	{ if(!blockParameters) setAutoGainOutliers(_value); }
		void	autoGainMinimumListner(int& _value)		{ if(!blockParameters) setAutoGainMinimum(_value); }
		void	autoGainMaximumListner(int& _value)		{ if(!blockParameters) setAutoGainMaximum(_value); }
		
		void	WhiteBalanceRedListner(int& _value)		{ if(!blockParameters) setWhiteBalanceRed(_value); }
		void	WhiteBalanceBlueListner(int& _value)	{ if(!blockParameters) setWhiteBalanceBlue(_value); }
		void	autoWhiteBalanceListner(bool& _value)	{ if(!blockParameters) setAutoWhiteBalance(_value); }
		void	autoWhiteBalanceOnceListner(bool& _value){if(!blockParameters) setAutoWhiteBalanceOnce(_value); }
		void	autoWhiteBalanceRateListner(int& _value){ if(!blockParameters) setAutoWhiteBalanceRate(_value); }
		void	autoWhiteBalanceAdjustTolListner(int& _value) { if(!blockParameters) setAutoWhiteBalanceAdjustTol(_value); }
		
		void	gammaListner(float& _value)				{ if(!blockParameters) setGamma(_value); }
		void	hueListner(float& _value)				{ if(!blockParameters) setHue(_value); }
		void	saturationListner(float& _value)		{ if(!blockParameters) setSaturation(_value); }
		
		void	doConnectListner(bool& _value)			{ if(_value) connect(); _value = false; }
		void	doDisconnectListner(bool& _value)		{ if(_value) disconnect(); doAutoConnect.set(false); _value = false; }
		void	doAutoConnectListner(bool& _value)		{ setAutoConnect(_value); }
		void	autoConnectAttemptsListner(int& _value)	{ if(!blockParameters) setAutoConnectAttempts(_value); }
		
		void	ipPersistentListner(bool& _value)		{ if(!blockParameters) setPersistentIp(_value); }
		
		bool	initInterface();
		void	setAllParametersFromCam();
		void	setAllParametersFromInterface();
		
		void	setParametersRange();
		void	setParameterInItsOwnRange(ofParameter<int> &_parameter);
		void	setParameterInItsOwnRange(ofParameter<float> &_parameter);
		
		bool	bLoadFromInterface;

		bool	blockParameters;
		
	};
}
