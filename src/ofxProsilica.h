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
	the	camera to be found and available if it was just use by an
	(other) application.
	Also tries to reconnect to the camera when connection is lost.
*///--------------------------------------------------------------------
#include "Connector.h"

/*	--------------------------------------------------------------------
	ofxProsilica::ParameterConnector adds ofParameters and an 
	ofParameterGroup to the Cam and Connector, for easy use with ofxGui.
	note: IP settings are not included.
*///--------------------------------------------------------------------
#include "ParameterConnector.h"

/*	--------------------------------------------------------------------
 EXPERIMENTAL, PRONE TO CHANGE
 ofxProsilica::texPC adds a texture to the ParameterConnector
*///--------------------------------------------------------------------
#include "TexPC.h"

/*	--------------------------------------------------------------------
 EXPERIMENTAL, PRONE TO CHANGE
 ofxProsilica::texPC adds warp funcyionality to the TexPC
*///--------------------------------------------------------------------
#include "WarpTexPC.h"
