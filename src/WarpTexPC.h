#pragma once

#include "ofMain.h"
#include "TexPC.h"

namespace ofxProsilica {
	
	class WarpTexPC : public TexPC {
	public:
		
		WarpTexPC() {}
		virtual ~WarpTexPC() {;}
		
		bool	setup();
		void	update();
		void 	draw(int _x, int _y) { draw(0, 0, this->getWidth(), this->getHeight()); }
		void 	draw(int _x, int _y, int _width, int _height) { getTexture().draw(_x, _y, _width, _height); }
		
		float 	getWidth()			{ return warpFbo.getWidth(); }
		float 	getHeight()			{ return warpFbo.getHeight(); }
		
		ofTexture& 	getTexture() 	{ return warpFbo.getTexture(); }
		ofPixels&	getPixels();
		ofPolyline& getWarpLine() 	{ return warpLine; }
		
	protected:
		ofParameterGroup	warpParameters;
		ofParameter<ofVec2f>*   warpPoints;
		void warpPointListener(ofVec2f& _value) { warpUpdated = true; _value = ofVec2f(int(_value.x * 100.0) / 100.0, int(_value.y * 100.0) / 100.0); }
		
		ofFbo 				warpFbo;
		ofShader			invWarpShader;
		ofPlanePrimitive	warpPlane;
		ofPolyline			warpLine;
		bool 				warpUpdated;
		
		void createWarpShader();
		void createWS2();
		void createWS3();
		
		ofPixels	pixels;
		bool		pixelsSet;
		
	};
}

