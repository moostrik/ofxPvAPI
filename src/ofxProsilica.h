/*	--------------------------------------------------------------------
	ofxProsilica is an Open Frameworks wrapper for Allied Vision GigE 
	Cameras
*///--------------------------------------------------------------------

/*	--------------------------------------------------------------------
	ofxProsilica::BaseGrabber is modelled after ofVideoGrabber.
*///--------------------------------------------------------------------
#include "Camera.h"

/*	--------------------------------------------------------------------
	ofxProsilica::Connector deals with the (long) time it can take for 
	the	camera to be found and available if it was just used by an
	application.
	Also reconnects to the camera when connection is lost.
*///--------------------------------------------------------------------
#include "Connector.h"

/*	--------------------------------------------------------------------
	ofxProsilica::PConnector adds ofParameters and an
	ofParameterGroup to the Cam and Connector, for easy use with ofxGui.
	note: IP settings are not included.
*///--------------------------------------------------------------------
#include "ParameterConnector.h"

/*	--------------------------------------------------------------------
 	ofxProsilica::TPConnector adds a texture to the PConnector
 	also it adds the flip and rotate functionality
*///--------------------------------------------------------------------
#include "TexPC.h"

/*	--------------------------------------------------------------------
	 ofxProsilica::WTPConnector adds warp functionality to the TPC
*///--------------------------------------------------------------------
#include "WarpTexPC.h"
