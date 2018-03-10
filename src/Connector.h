#pragma once

#include "ofMain.h"
#include "Camera.h"

namespace ofxPvAPI {
	
	class Connector : public Camera {
	public:
		
		Connector() {;}
		virtual ~Connector() {;}
		
		bool	setup();
		void	update();
		bool	connect();
		void	disconnect();
		
		void	setAutoConnect(bool _value)			{ bAutoConnect = _value; if(!_value){ connectCounter = 0;  connectInterval = 2;} }
		void	setAutoConnectAttempts(int _value)	{ connectAttempts = _value; }
		
		bool	getAutoConnect()					{ return bAutoConnect; }
		int		getAutoConnectAttempts()			{ return connectAttempts; }
		int		getAutoConnectCounter()				{ return connectCounter; }
		float	getAutoConnectInterval()			{ return connectInterval; }
		
		bool	getConnectionChange()				{ return (bWasInitialized != bInitialized); }
		bool	getConnected()						{ return (bWasInitialized != bInitialized && bInitialized); }
		bool	getDisConnected()					{ return (bWasInitialized != bInitialized && !bInitialized); }
		
		
	protected:
		bool	initConnector();
		void	autoConnect();
		
		bool	bAutoConnect;
		int		connectAttempts;
		int		connectCounter;
		float	connectInterval;
		float	nextConnectAttemptTime;
		
		bool	bWasInitialized;
		
	};
}
