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


namespace ofxProsilica {
	
	class Camera :public ofThread {
		public :
		
		Camera();
		virtual ~Camera();
		
		
		bool			setup();
		void			update();
		bool			isInitialized();
		bool 			isFrameNew(bool _reset = true);
		void			close();
		
		void 			threadedFunction();
		
		//-- DEVICE ----------------------------------------------------------
		vector<ofVideoDevice> listDevices();
		void			setDeviceID(int _deviceID) { requestDeviceByID(_deviceID); }
		void			requestDeviceByID(int _deviceID);
		int 			getDeviceID();
		int             getDeviceIDFromIpAdress(string _IpAdress);
		
		//-- ATTRIBUTES GENERAL-----------------------------------------------
		void            videoSettings() { listAttributes();}
		void			listAttributes();
		void			resetAttributes();
		
		//-- PIXELS ----------------------------------------------------------
//		unsigned char*	getData();
		ofPixels&		getPixels();
//		void			getPixels(ofPixels& _pixels);
		bool			setPixelFormat(ofPixelFormat _pixelFormat);
		ofPixelFormat	getPixelFormat();
		
        //-- WIDTH & HEIGHT --------------------------------------------------
//		float	getWidth()							{ lock(); int w = (T_pixelsOut.isAllocated())? T_pixelsOut.getWidth() : 0; unlock(); return w; } 	// width of the pixels, not the ROI
//		float	getHeight()							{ lock(); int h = (T_pixelsOut.isAllocated())? T_pixelsOut.getHeight() : 0; unlock(); return h;  }	// height of the pixels, not the ROI
		float	getWidth()							{ return (T_pixelsOut.isAllocated())? T_pixelsOut.getWidth() : 0; } 	// width of the pixels, not the ROI
		float	getHeight()							{ return (T_pixelsOut.isAllocated())? T_pixelsOut.getHeight() : 0; }	// height of the pixels, not the ROI
		
        //-- FRAMERATE -------------------------------------------------------
		void	setDesiredFrameRate(int  _framerate){ setFrameRate(_framerate);}
		void	setFrameRate(float rate)			{ setFloatAttribute("FrameRate", rate); }
		float	getFrameRate()						{ return getFloatAttribute("FrameRate"); }
		float	getFrameRateMax()					{ return getFloatAttributeMax("FrameRate"); }
		float	getFrameRateMin()					{ return getFloatAttributeMin("FrameRate"); }
		
		int		getFrameCount() 					{return frameCount; }
		
        //-- REGION OF INTEREST ----------------------------------------------
		void	setROIWidth(int w);
		void 	setROIHeight(int h);
		void    setROIX(int x);
		void    setROIY(int y);
		
		int 	getROIWidth()						{ return getIntAttribute("Width"); }
		int 	getROIHeight()						{ return getIntAttribute("Height"); }
		int		getROIX()							{ return getIntAttribute("RegionX"); }
		int		getROIY()							{ return getIntAttribute("RegionY"); }
		
		int 	getROIWidthMin()					{ return getIntAttributeMin("Width"); }
		int 	getROIWidthMax()					{ return getIntAttributeMax("Width"); }
		int 	getROIHeightMin()					{ return getIntAttributeMin("Height"); }
		int 	getROIHeightMax()					{ return getIntAttributeMax("Height"); }
		int		getROIXMin()						{ return getIntAttributeMin("RegionX"); }
		int		getROIXMax()						{ return getIntAttributeMax("RegionX"); }
		int		getROIYMin()						{ return getIntAttributeMin("RegionY"); }
		int		getROIYMax()						{ return getIntAttributeMax("RegionY"); }
		
        //-- EXPOSURE --------------------------------------------------------
		void	setAutoExposure(bool state)			{ setEnumAttribute("ExposureMode", (state == true)? "Auto": "Manual"); }
		void	setAutoExposureOnce(bool state)		{ setEnumAttribute("ExposureMode", (state == true)? "AutoOnce": "Manual"); }
		bool	getAutoExposure()					{ return (getEnumAttribute("ExposureMode") == "Auto")? true: false; }
		bool	getAutoExposureOnce()				{ return (getEnumAttribute("ExposureMode") == "AutoOnce")? true: false; }
		
		void    setAutoExposureRangeFromFrameRate()	{ setAutoExposureMaximum(getAutoExposureMaxForCurrentFrameRate()); setAutoExposureMinimum(getAutoExposureMinimumMin());}
		
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
		
		int 	getAutoExposureMaxForCurrentFrameRate() { return 1000000 / getFrameRate(); }
		
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
        void            setPersistentIp(bool enable);
        void            setPersistentIpAdress(string _IpAdress);
        void            setPersistentIpSubnetMask(string _IpSubnet);
        void            setPersistentIpGateway(string _IpGateway);
        string          getIpAdress();
        string          getIpSubnet();
        string          getIpGateway();
        bool            getIpPersistent();
        void            listIpSettings();
		
		
		//-- NORMALIZED ATTRIBUTES -------------------------------------------
		//-- (turned out quite impractical) ----------------------------------
/*
		//-- REGION OF INTEREST ----------------------------------------------
		void	setNormalizedROIWidth(float w);
		void	setNormalizedROIHeight(float h);
		void	setNormalizedROIX(float x);
		void	setNormalizedROIY(float y);
		
		float	getNormalizedROIWidth();
		float	getNormalizedROIHeight();
		float	getNormalizedROIX();
		float	getNormalizedROIY();
		
		//-- EXPOSURE --------------------------------------------------------
		void	setNormalizedExposure(float _value)				{ setNormalizedAttribute("ExposureValue", _value); }
		void	setNormalizedAutoExposureAdjustTol(float _value){ setNormalizedAttribute("ExposureAutoAdjustTol", _value); }
		void	setNormalizedAutoExposureMin(float _value)		{ setNormalizedAttribute("ExposureAutoMin", _value); }
		void	setNormalizedAutoExposureMax(float _value)		{ setNormalizedAttribute("ExposureAutoMax", _value); }
		void	setNormalizedAutoExposureOutliers(float _value)	{ setNormalizedAttribute("ExposureAutoOutliers", _value); }
		void	setNormalizedAutoExposureRate(float _value)		{ setNormalizedAttribute("ExposureAutoRate", _value); }
		void	setNormalizedAutoExposureTarget(float _value)	{ setNormalizedAttribute("ExposureAutoTarget", _value); }
		 
		float	getNormalizedExposure()							{ return getNormalizedAttribute("ExposureValue"); }
		float	getNormalizedAutoExposureAdjustTol()			{ return getNormalizedAttribute("ExposureAutoAdjustTol"); }
		float	getNormalizedAutoExposureMin()					{ return getNormalizedAttribute("ExposureAutoMin"); }
		float	getNormalizedAutoExposureMax()					{ return getNormalizedAttribute("ExposureAutoMax"); }
		float	getNormalizedAutoExposureOutliers()				{ return getNormalizedAttribute("ExposureAutoOutliers"); }
		float	getNormalizedAutoExposureRate()					{ return getNormalizedAttribute("ExposureAutoRate"); }
		float	getNormalizedAutoExposureTarget()				{ return getNormalizedAttribute("ExposureAutoTarget"); }
		 
		float	getNormalizedAutoExposureMaxForCurrentFrameRate() { return (float)getAutoExposureMaxForCurrentFrameRate() / getAutoExposureMax(); }
		
		//-- GAIN ------------------------------------------------------------
		void	setNormalizedGain(float _value)					{ setNormalizedAttribute("GainValue", _value); }
		void	setNormalizedAutoGainAdjustTol(float _value)	{ setNormalizedAttribute("GainAutoAdjustTol", _value); }
		void	setNormalizedAutoGainMin(float _value)			{ setNormalizedAttribute("GainAutoMin", _value); }
		void	setNormalizedAutoGainMax(float _value)			{ setNormalizedAttribute("GainAutoMax", _value); }
		void	setNormalizedAutoGainOutliers(float _value)		{ setNormalizedAttribute("GainAutoOutliers", _value); }
		void   	setNormalizedAutoGainRate(float _value)			{ setNormalizedAttribute("GainAutoRate", _value); }
		void	setNormalizedAutoGainTarget(float _value)		{ setNormalizedAttribute("GainAutoTarget", _value); }
		
		float	getNormalizedGain()								{ return getNormalizedAttribute("GainValue"); }
		float	getNormalizedAutoGainAdjustTol()				{ return getNormalizedAttribute("GainAutoAdjustTol"); }
		float	getNormalizedAutoGainMin()						{ return getNormalizedAttribute("GainAutoMin"); }
		float	getNormalizedAutoGainMax()						{ return getNormalizedAttribute("GainAutoMax"); }
		float	getNormalizedAutoGainOutliers()					{ return getNormalizedAttribute("GainAutoOutliers"); }
		float	getNormalizedAutoGainRate()						{ return getNormalizedAttribute("GainAutoRate"); }
		float	getNormalizedAutoGainTarget()					{ return getNormalizedAttribute("GainAutoTarget"); }
		
		//-- GAMMA HUE STURATION ---------------------------------------------
		void 	setNormalizedGamma(float _value)				{ setNormalizedAttribute("Gamma", _value); }
		void 	setNormalizedHue(float _value)					{ setNormalizedAttribute("Hue", _value); }
		void 	setNormalizedSaturation(float _value)			{ setNormalizedAttribute("Saturation", _value); }
		
		float 	getNormalizedGamma()							{ return getNormalizedAttribute("Gamma"); }
		float 	getNormalizedHue()								{ return getNormalizedAttribute("Hue"); }
		float 	getNormalizedSaturation()						{ return getNormalizedAttribute("Saturation"); }
		
		//-- WHITE BALANCE ---------------------------------------------------
		void	setNormalizedWhiteBalanceBlue(float _value)		{ setNormalizedAttribute("WhitebalValueBlue", _value); }
		void	setNormalizedWhiteBalanceRed(float _value)		{ setNormalizedAttribute("WhitebalValueRed", _value); }
		void	setNormalizedAutoWhiteBalanceAdjustTol(float _value) { setNormalizedAttribute("WhitebalAutoAdjustTol", _value); }
		void	setNormalizedAutoWhiteBalanceRate(float _value)	{ setNormalizedAttribute("WhitebalAutoRate", _value); }
		 
		float	getNormalizedWhiteBalanceBlue()					{ return getNormalizedAttribute("WhitebalValueBlue"); }
		float	getNormalizedWhiteBalanceRed()					{ return getNormalizedAttribute("WhitebalValueRed"); }
		float	getNormalizedAutoWhiteBalanceAdjustTol()		{ return getNormalizedAttribute("WhitebalAutoAdjustTol"); }
		float	getNormalizedAutoWhiteBalanceRate()				{ return getNormalizedAttribute("WhitebalAutoRate"); }
*/		
		
		//--------------------------------------------------------------------
		//-- PROTECTED -------------------------------------------------------
	protected:
		void			PvApiInitialize();
		void			PvApiUnInitialize();
		static bool     bPvApiInitiated;
		static int		numCamerasInUse;
		tPvHandle       cameraHandle;
		bool 			bInitialized;
        
        //-- ACQUISITION -----------------------------------------------------
		bool			initCamera(int cameraUid);
		bool			openCamera();
		bool			closeCamera();
		bool			startCapture();
		bool			stopCapture();
		bool			startAcquisition();
		bool			stopAcquisition();
		bool			abortAcquisition();
		bool			queueFrame();
		bool			clearQueue();
        
        //-- DEVICES ---------------------------------------------------------
		unsigned long 	deviceID;
		unsigned long 	requestedDeviceID;
        
        //-- PIXELS & FRAME---------------------------------------------------
		ofPixelFormat	internalPixelFormat;
		ofPixels		framePixels;
		ofPixels		T_pixelsOut;
		ofPixels		ancillaryPixels;
		bool			allocatePixels();
		bool 			T_bNeedsResize;
		
		tPvFrame        cameraFrame;
		bool 			T_bIsFrameNew;
		bool            bWaitingForFrame;
		int 			frameCount;
		
        //-- REGION OF INTEREST-----------------------------------------------
		float			regionX;
		float			regionY;
		
        //-- ATTRIBUTES-------------------------------------------------------
		bool			setNormalizedAttribute(string _name, float _value);
		float			getNormalizedAttribute(string _name);
		
		bool			setIntAttribute(string _name, int _value);
		int				getIntAttribute(string _name);
		int				getIntAttributeMax(string _name);
		int				getIntAttributeMin(string _name);
		
		bool			setFloatAttribute(string _name, float _value);
		float			getFloatAttribute(string _name);
		float			getFloatAttributeMax(string _name);
		float			getFloatAttributeMin(string _name);
		
		bool			setEnumAttribute(string _name, string _value);
		string			getEnumAttribute(string _name);
		
        //-- IP SETTINGS -----------------------------------------------------
        string          persistentIpAdress;
        string          persistentIpSubnetMask;
        string          persistentIpGateway;
        unsigned long   IPStringToLong(string _IpAdress);
        string          IPLongToString(unsigned long  _IpAdress);
        
		bool			setPacketSizeToMax();
		
		void			logError(tPvErr _msg);
		
	};
}
