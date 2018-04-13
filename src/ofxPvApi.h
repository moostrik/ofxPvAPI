/*	--------------------------------------------------------------------
	ofxProsilica is an Open Frameworks wrapper for Allied Vision GigE 
	Cameras
*///--------------------------------------------------------------------

/*	--------------------------------------------------------------------
	ofxProsilica::BaseGrabber is modelled after ofVideoGrabber.
*///--------------------------------------------------------------------
#include "Camera.h"

/*	--------------------------------------------------------------------
	ofxProsilica::ParamCam adds ofParameters and an ofParameterGroup to
 	the Camera, for easy use with ofxGui.
	note: IP settings are not included.
*///--------------------------------------------------------------------
#include "ParamCam.h"

/*	--------------------------------------------------------------------
 	ofxProsilica::ParamCamExt adds extended functionality such as flip,
 	rotate90, redToRGB and homography warp
 	note: these functions are performed on the texture. getPixels()
 	gets it's pixels from the texture and is not optimized.
 
*///--------------------------------------------------------------------
#include "ParamCamExt.h"

