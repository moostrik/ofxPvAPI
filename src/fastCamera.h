
// experimental class with emphasis on low latency at the cost of horizontal tearing and pixel dubbing

#pragma once

#include "ofMain.h"
#include "Camera.h"

namespace ofxPvAPI {
	
	class fastCamera : public Camera {
	public:
		fastCamera() { numPvFrames = 1; bWaitingForFrame = false;}
		void update() override;
		bool queueFrames() override;
	private:
		bool bWaitingForFrame;
	};
	
}
