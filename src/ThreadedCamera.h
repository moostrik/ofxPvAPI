#pragma once

#include "ofMain.h"
#include "PvApi.h"
#include "Camera.h"

#include <stdio.h>

#ifdef _WIN32
#include <Ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif


namespace ofxPvAPI {
	
	enum ofxPvAPIMessageType {
		OFX_PVAPI_FRAME,
		OFX_PVAPI_SETROIX,
		OFX_PVAPI_SETROIY
	};
	
	struct ofxPvAPIMessage {
		ofxPvAPIMessageType type;
		tPvFrame*			frame;
		int					value;
	};
	
	class ThreadedCamera : public ofThread, public Camera {
		
		public :
		
		ThreadedCamera();
		virtual ~ThreadedCamera();
		
			//-- OF --------------------------------------------------------------
	public:
		bool			setup();
		void			close();
		
	private:
		void 			threadedFunction();
		void 			enqueue(ofxPvAPIMessage message);
		void 			dequeue();
		
		queue<ofxPvAPIMessage> messageQueue;
		
		void 			processFrame(tPvFrame* _frame);
		void 			processResize(bool _XorY, int _value);
	
		bool			queueFrames();
		
	public:
		void			onFrameDone(tPvFrame* _frame); // for internal use only, cannot be protected due to callback
		
		bool			initCamera(int cameraUid);
		
			//-- REGION OF INTEREST ----------------------------------------------
		public:
		void			setROIWidth(int _value);
		void			setROIHeight(int _value);
		
		void			setFlipH(bool _value)	{ std::unique_lock<std::mutex> lock(camMutex); flipH = _value; }
		void			getFlipH()				{ std::unique_lock<std::mutex> lock(camMutex); return flipH; }
		void			setFlipV(bool _value)	{ std::unique_lock<std::mutex> lock(camMutex); flipV = _value; }
		void			getFlipV()				{ std::unique_lock<std::mutex> lock(camMutex); return flipV; }
		void			setRotate90(int _value)	{ std::unique_lock<std::mutex> lock(camMutex); rotate90 = _value % 4; }
		void			getRotate90()			{ std::unique_lock<std::mutex> lock(camMutex); return rotate90; }
		
	protected:
		bool 			flipH;
		bool 			flipV;
		int 			rotate90;

	private:
		std::mutex		camMutex;
		std::condition_variable camCondition;
		
		tPvFrame*		pvFramesThreaded;
		int 			pvFrameIterator;
	};
}


