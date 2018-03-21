#pragma once

#include "ofMain.h"
#include "PvApi.h"

#include <stdio.h>

#ifdef _WIN32
#include <Ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif


namespace ofxPvAPI {
	
	class Camera {
		
	public :
		Camera();
		virtual ~Camera();
		
		bool			setup();
		void			update();
		
		//-- API -------------------------------------------------------------
	private:
		void			PvApiInitialize();
		void			PvApiUnInitialize();
		static bool		bPvApiInitiated;
		
		//-- LIST & ID -------------------------------------------------------
	public:
		vector<ofVideoDevice> listDevices();
		
		int 			getDeviceID();
		
		void			setDeviceID(int _deviceID) { requestDeviceByID(_deviceID); }
		void			requestDeviceByID(int _deviceID);
		int				getRequestedDeviceID() { return requestedDeviceID; }
		
		int				getDeviceIDFromIpAdress(string _IpAdress);
		int				getFirstAvailableDeviceID();
		
		bool			isDeviceFound(int _deviceID);
		bool			isDeviceAvailable(int _deviceID);
		
	private:
		unsigned long	deviceID;
		unsigned long	requestedDeviceID;
		
		//-- DEVICE ----------------------------------------------------------
	public:
		bool			isActive() { return bDeviceActive; }
		void			activate(); 			// this is taken care of by setup(), but you might want to (de)activate manually
		void			deactivate();
		
	private:
		tPvHandle		deviceHandle;
		bool 			bDeviceActive;
		static int		numActiveDevices;
		
		static void 	plugCallBack(void* Context, tPvInterface Interface, tPvLinkEvent Event, unsigned long UniqueId);
		void			plugCamera(unsigned long cameraUid);
		void			unplugCamera(unsigned long cameraUid);
		
		bool			openCamera();
		bool			closeCamera();
		
		bool			startCapture();
		bool			stopCapture();
		
		bool			startAcquisition();
		bool			stopAcquisition();
		bool			abortAcquisition();
		
		bool			bWaitForDeviceToBecomeAvailable;
		int				waitInterval;
		uint64_t		lastWaitTime;
		
		//-- PV FRAMES -------------------------------------------------------
	private:
		static int		numPvFrames;
		tPvFrame*		pvFrames;
		deque<tPvFrame*>	capuredFrameQueue;
		
		void 			allocateFrames();
		void 			deallocateFrames();
		bool			setupFrames();
		void			resizeFrames();
		
		bool			queueFrames();
		bool			clearQueue();
		
		bool			triggerFrame();
		
		static void 	frameCallBack(tPvFrame* pFrame);
		void			receiveFrame(tPvFrame* _frame);
		
		//-- FRAMES --------------------------------------------------------------
	public:
		bool 			isFrameNew()			{ return bIsFrameNew; }
		
		int 			getFps() 				{ return fps; }
		int 			getFrameDrop() 			{ return frameDrop; }
		int 			getLatency() 			{ return frameLatency; }
		int 			getMaxLatency() 		{ return frameMaxLatency; }
		int 			getMinLatency() 		{ return frameMinLatency; }
		float			getFixedRate()			{ return fixedRate; }
		float			getFrameRate()			{ return getFloatAttribute("FrameRate"); }
		float			getFrameRateMax()		{ return getFloatAttributeMax("FrameRate"); }
		float			getFrameRateMin()		{ return getFloatAttributeMin("FrameRate"); }
		
		void			setFixedRate(bool _value);
		void			setFrameRate(float rate);
		
	private:
		bool			bIsFrameNew;
		deque<float> 	fpsTimes;
		int 			fps;
		int 			frameDrop;
		deque<int> 		framesDropped;
		int 			frameLatency;
		int 			frameMaxLatency;
		int 			frameMinLatency;
		deque<int> 		framesLatencies;
		bool			fixedRate;
		
		//-- PIXELS --------------------------------------------------------------
	public:
		ofPixels&		getPixels()	{ return pixels; }
		float			getWidth()	{ return (pixels.isAllocated())? pixels.getWidth() : 0; } 	// pixels, not ROI
		float			getHeight()	{ return (pixels.isAllocated())? pixels.getHeight() : 0; }	// pixels, not ROI
		
		bool			setPixelFormat(ofPixelFormat _pixelFormat); // for now only before setup
		ofPixelFormat	getPixelFormat() { return pixelFormat; }
		
	private:
		ofPixels		pixels;
		ofPixelFormat	pixelFormat;
		ofPixelFormat	getOfPixelFormat(string _format);
		string			getPvPixelFormat(ofPixelFormat _format);
		
			//-- ATTRIBUTES GENERAL-----------------------------------------------
	public:
		void	videoSettings() { listAttributes();}
		void	listAttributes();
		void	resetAttributes();
		
	private:
		bool	setNormalizedAttribute(string _name, float _value);
		float	getNormalizedAttribute(string _name);
		
		bool	setIntAttribute(string _name, int _value);
		int		getIntAttribute(string _name);
		int		getIntAttributeMax(string _name);
		int		getIntAttributeMin(string _name);
		
		bool	setFloatAttribute(string _name, float _value);
		float	getFloatAttribute(string _name);
		float	getFloatAttributeMax(string _name);
		float	getFloatAttributeMin(string _name);
		
		bool	setEnumAttribute(string _name, string _value);
		string	getEnumAttribute(string _name);
		
			//-- REGION OF INTEREST ----------------------------------------------
	public:
		void	setROIWidth(int _value);
		void	setROIHeight(int _value);
		void	setROIX(int _value);
		void	setROIY(int _value);
		
		int 	getROIWidth()		{ return getIntAttribute("Width"); }
		int 	getROIHeight()		{ return getIntAttribute("Height"); }
		int		getROIX()			{ return getIntAttribute("RegionX"); }
		int		getROIY()			{ return getIntAttribute("RegionY"); }
		
		int 	getROIWidthMin()	{ return getIntAttributeMin("Width"); }
		int 	getROIWidthMax()	{ return getIntAttributeMax("Width"); }
		int 	getROIHeightMin()	{ return getIntAttributeMin("Height"); }
		int 	getROIHeightMax()	{ return getIntAttributeMax("Height"); }
		int		getROIXMin()		{ return getIntAttributeMin("RegionX"); }
		int		getROIXMax()		{ return getIntAttributeMax("RegionX"); }
		int		getROIYMin()		{ return getIntAttributeMin("RegionY"); }
		int		getROIYMax()		{ return getIntAttributeMax("RegionY"); }
		
			//-- EXPOSURE --------------------------------------------------------
	public:
		void	setAutoExposure(bool state)			{ setEnumAttribute("ExposureMode", (state == true)? "Auto": "Manual"); }
		void	setAutoExposureOnce(bool state)		{ setEnumAttribute("ExposureMode", (state == true)? "AutoOnce": "Manual"); }
		bool	getAutoExposure()					{ return (getEnumAttribute("ExposureMode") == "Auto")? true: false; }
		bool	getAutoExposureOnce()				{ return (getEnumAttribute("ExposureMode") == "AutoOnce")? true: false; }
		
		int 	getExposureMaxForCurrentFrameRate() { return fixedRate? 1000000 / getFrameRate() : 200000 / getFrameRate(); }
		void	setAutoExposureRangeFromFrameRate()	{ setAutoExposureMaximum(getExposureMaxForCurrentFrameRate()); setAutoExposureMinimum(getAutoExposureMinimumMin());}
		
		void	setExposure(int _value)				{ setIntAttribute("ExposureValue", _value); }
		
			// Range: [1 - 100] Default: 50 Units: percent
			// The general lightness or darkness of the auto exposure feature; specifically, the target mean histogram level of the image, 0 being black, 100 being white.
		void 	setAutoExposureTarget(int _value)	{ setIntAttribute("ExposureAutoTarget", _value); }
		
			// Range: [1 - 100] Default: 100 Units: percent
			// The rate at which the auto exposure function changes the exposure setting.
		void 	setAutoExposureRate(int _value)		{ setIntAttribute("ExposureAutoRate", _value); }
		
			// Range: [0 – 50] Default: 5 Units: percent
			// Tolerance in variation from ExposureAutoTarget in which the auto exposure algorithm will not respond.
			// Can be used to limit exposure setting changes to only larger variations in scene lighting.
		void	setAutoExposureAdjustTol(int _value) { setIntAttribute("ExposureAutoAdjustTol", _value); }
		
			// Range: [0 - 1000] Default: 0 Units: 0.01% i.e. 1000 = 10%
			// The total pixels from top of the distribution that are ignored by the auto exposure algorithm.
		void 	setAutoExposureOutliers(int _value)	{ setIntAttribute("ExposureAutoOutliers", _value); }
		
			// Range: [Camera dependent - 1000000] Default: 500000 Units: μs
			// The upper bound to the exposure setting in Autoexposure mode. This is useful in situations where frame rate is important.
			// This value would normally be set to something less than 1x10^6/ (desired frame rate).
		void	setAutoExposureMinimum(int _value)	{ setIntAttribute("ExposureAutoMin", _value); }
		void 	setAutoExposureMaximum(int _value)	{ setIntAttribute("ExposureAutoMax", _value); }
		
		int		getExposure()					{ return getIntAttribute("ExposureValue"); }
		int		getAutoExposureAdjustTol()		{ return getIntAttribute("ExposureAutoAdjustTol"); }
		int		getAutoExposureMinimum()		{ return getIntAttribute("ExposureAutoMin"); }
		int		getAutoExposureMaximum()		{ return getIntAttribute("ExposureAutoMax"); }
		int		getAutoExposureOutliers()		{ return getIntAttribute("ExposureAutoOutliers"); }
		int		getAutoExposureRate()			{ return getIntAttribute("ExposureAutoRate"); }
		int		getAutoExposureTarget()			{ return getIntAttribute("ExposureAutoTarget"); }
		
		int		getExposureMin()				{ return getIntAttributeMin("ExposureValue"); }
		int		getExposureMax()				{ return getIntAttributeMax("ExposureValue"); }
		int		getAutoExposureAdjustTolMin()	{ return getIntAttributeMin("ExposureAutoAdjustTol"); }
		int		getAutoExposureAdjustTolMax()	{ return getIntAttributeMax("ExposureAutoAdjustTol"); }
		int		getAutoExposureMinimumMin()		{ return getIntAttributeMin("ExposureAutoMin"); }
		int		getAutoExposureMinimumMax()		{ return getIntAttributeMax("ExposureAutoMin"); }
		int		getAutoExposureMaximumMin()		{ return getIntAttributeMin("ExposureAutoMax"); }
		int		getAutoExposureMaximumMax()		{ return getIntAttributeMax("ExposureAutoMax"); }
		int		getAutoExposureOutliersMin()	{ return getIntAttributeMin("ExposureAutoOutliers"); }
		int		getAutoExposureOutliersMax()	{ return getIntAttributeMax("ExposureAutoOutliers"); }
		int		getAutoExposureRateMin()		{ return getIntAttributeMin("ExposureAutoRate"); }
		int		getAutoExposureRateMax()		{ return getIntAttributeMax("ExposureAutoRate"); }
		int		getAutoExposureTargetMin()		{ return getIntAttributeMin("ExposureAutoTarget"); }
		int		getAutoExposureTargetMax()		{ return getIntAttributeMax("ExposureAutoTarget"); }
		
		
			//-- GAIN ------------------------------------------------------------
		void 	setAutoGain(bool state)			{ setEnumAttribute("GainMode", (state == true)? "Auto": "Manual"); }
		void 	setAutoGainOnce(bool state)		{ setEnumAttribute("GainMode", (state == true)? "AutoOnce": "Manual"); }
		bool 	getAutoGain()					{ return (getEnumAttribute("GainMode") == "Auto")? true: false; }
		bool 	getAutoGainOnce()				{ return (getEnumAttribute("GainMode") == "AutoOnce")? true: false; }
		
		void	setGain(int _value)				{ setIntAttribute("GainValue", _value); }
		void 	setAutoGainAdjustTol(int _value){ setIntAttribute("GainAutoAdjustTol", _value); }
		void 	setAutoGainMinimum(int _value)	{ setIntAttribute("GainAutoMin", _value); }
		void 	setAutoGainMaximum(int _value)	{ setIntAttribute("GainAutoMax", _value); }
		void 	setAutoGainOutliers(int _value) { setIntAttribute("GainAutoOutliers", _value); }
		void 	setAutoGainRate(int _value)		{ setIntAttribute("GainAutoRate", _value); }
		void 	setAutoGainTarget(int _value)	{ setIntAttribute("GainAutoTarget", _value); }
		
		int 	getGain()						{ return getIntAttribute("GainValue"); }
		int 	getAutoGainAdjustTol()			{ return getIntAttribute("GainAutoAdjustTol"); }
		int 	getAutoGainMinimum()			{ return getIntAttribute("GainAutoMin"); }
		int 	getAutoGainMaximum()			{ return getIntAttribute("GainAutoMax"); }
		int 	getAutoGainOutliers()			{ return getIntAttribute("GainAutoOutliers"); }
		int 	getAutoGainRate()				{ return getIntAttribute("GainAutoRate"); }
		int 	getAutoGainTarget()				{ return getIntAttribute("GainAutoTarget"); }
		
		int 	getGainMin()					{ return getIntAttributeMin("GainValue"); }
		int 	getGainMax()					{ return getIntAttributeMax("GainValue"); }
		int 	getAutoGainAdjustTolMin()		{ return getIntAttributeMin("GainAutoAdjustTol"); }
		int 	getAutoGainAdjustTolMax()		{ return getIntAttributeMax("GainAutoAdjustTol"); }
		int 	getAutoGainMinimumMin()			{ return getIntAttributeMin("GainAutoMin"); }
		int 	getAutoGainMinimumMax()			{ return getIntAttributeMax("GainAutoMin"); }
		int 	getAutoGainMaximumMin()			{ return getIntAttributeMin("GainAutoMax"); }
		int 	getAutoGainMaximumMax()			{ return getIntAttributeMax("GainAutoMax"); }
		int 	getAutoGainOutliersMin()		{ return getIntAttributeMin("GainAutoOutliers"); }
		int 	getAutoGainOutliersMax()		{ return getIntAttributeMax("GainAutoOutliers"); }
		int 	getAutoGainRateMin()			{ return getIntAttributeMin("GainAutoRate"); }
		int 	getAutoGainRateMax()			{ return getIntAttributeMax("GainAutoRate"); }
		int 	getAutoGainTargetMin()			{ return getIntAttributeMin("GainAutoTarget"); }
		int 	getAutoGainTargetMax()			{ return getIntAttributeMax("GainAutoTarget"); }
		
			//-- GAMMA HUE STURATION ---------------------------------------------
		void 	setGamma(float _value)			{ setFloatAttribute("Gamma", _value); }
		void 	setHue(float _value)			{ setFloatAttribute("Hue", _value); }
		void 	setSaturation(float _value)		{ setFloatAttribute("Saturation", _value); }
		
		float 	getGamma()						{ return getFloatAttribute("Gamma"); }
		float 	getHue()						{ return getFloatAttribute("Hue"); }
		float 	getSaturation()					{ return getFloatAttribute("Saturation"); }
		
		int 	getGammaMin()					{ return getFloatAttributeMin("Gamma"); }
		int 	getGammaMax()					{ return getFloatAttributeMax("Gamma"); }
		int 	getHueMin()						{ return getFloatAttributeMin("Hue"); }
		int 	getHueMax()						{ return getFloatAttributeMax("Hue"); }
		int 	getSaturationMin()				{ return getFloatAttributeMin("Saturation"); }
		int 	getSaturationMax()				{ return getFloatAttributeMax("Saturation"); }
		
			//-- WHITE BALANCE ---------------------------------------------------
		void 	setAutoWhiteBalance(bool state)	{ setEnumAttribute("WhitebalMode", (state == true)? "Auto": "Manual"); }
		void 	setAutoWhiteBalanceOnce(bool state)	{ setEnumAttribute("WhitebalMode", (state == true)? "AutoOnce": "Manual"); }
		bool 	getAutoWhiteBalance()			{ return (getEnumAttribute("WhitebalMode") == "Auto")? true: false; }
		bool 	getAutoWhiteBalanceOnce()		{ return (getEnumAttribute("WhitebalMode") == "AutoOnce")? true: false; }
		
		void 	setWhiteBalanceBlue(int _value)	{ setIntAttribute("WhitebalValueBlue", _value); }
		void 	setWhiteBalanceRed(int _value)	{ setIntAttribute("WhitebalValueRed", _value); }
		void 	setAutoWhiteBalanceAdjustTol(int _value) { setIntAttribute("WhitebalAutoAdjustTol", _value); }
		void 	setAutoWhiteBalanceRate(int _value) { setIntAttribute("WhitebalAutoRate", _value); }
		
		int 	getWhiteBalanceBlue()			{ return getIntAttribute("WhitebalValueBlue"); }
		int 	getWhiteBalanceRed()			{ return getIntAttribute("WhitebalValueRed"); }
		int 	getAutoWhiteBalanceAdjustTol()	{ return getIntAttribute("WhitebalAutoAdjustTol"); }
		int 	getAutoWhiteBalanceRate()		{ return getIntAttribute("WhitebalAutoRate"); }
		
		int 	getWhiteBalanceBlueMin()		{ return getIntAttributeMin("WhitebalValueBlue"); }
		int 	getWhiteBalanceBlueMax()		{ return getIntAttributeMax("WhitebalValueBlue"); }
		int 	getWhiteBalanceRedMin()			{ return getIntAttributeMin("WhitebalValueRed"); }
		int 	getWhiteBalanceRedMax()			{ return getIntAttributeMax("WhitebalValueRed"); }
		int 	getAutoWhiteBalanceAdjustTolMin(){return getIntAttributeMin("WhitebalAutoAdjustTol"); }
		int 	getAutoWhiteBalanceAdjustTolMax(){return getIntAttributeMax("WhitebalAutoAdjustTol"); }
		int 	getAutoWhiteBalanceRateMin()	{ return getIntAttributeMin("WhitebalAutoRate"); }
		int 	getAutoWhiteBalanceRateMax()	{ return getIntAttributeMax("WhitebalAutoRate"); }
		
			//-- IP SETTINGS -----------------------------------------------------
	public:
		void	setPersistentIp(bool enable);
		void	setPersistentIpAdress(string _IpAdress);
		void	setPersistentIpSubnetMask(string _IpSubnet);
		void	setPersistentIpGateway(string _IpGateway);
		string	getIpAdress();
		string	getIpSubnet();
		string	getIpGateway();
		bool	getIpPersistent();
		void	listIpSettings();
		
	private:
		string			persistentIpAdress;
		string			persistentIpSubnetMask;
		string			persistentIpGateway;
		unsigned long	IPStringToLong(string _IpAdress);
		string			IPLongToString(unsigned long  _IpAdress);
		bool			setPacketSizeToMax();
		
			//-- ERROR LOGGING ---------------------------------------------------
	private:
		void			logError(tPvErr _msg);
		
	};
}

