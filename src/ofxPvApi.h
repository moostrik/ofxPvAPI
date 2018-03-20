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
 	ofxProsilica::TexPC adds a texture to the ParamCam also it adds the
 	flip and rotate functionality
*///--------------------------------------------------------------------
#include "TexPC.h"

/*	--------------------------------------------------------------------
	 ofxProsilica::WarpTexPC adds warp functionality to the TexPC
*///--------------------------------------------------------------------
#include "WarpTexPC.h"
