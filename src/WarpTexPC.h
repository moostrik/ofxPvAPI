#pragma once

#include "ofMain.h"
#include "TexPC.h"

namespace ofxProsilica {
	
	class WarpTexPC : public TexPC {
	public:
		
		WarpTexPC() {}
		virtual ~WarpTexPC() {;}
		
		bool	setup(int _width, int _height) { ofLogWarning("ofxProsilica") << "setup with dimensions not yet supported"; setup(); };
		bool	setup(ofTexture _tex = ofTexture());
		void	update();
		void 	draw(int _x, int _y) { draw(0, 0, getWarpTexture().getWidth(), getWarpTexture().getHeight()); }
		void 	draw(int _x, int _y, int _width, int _height) { getWarpTexture().draw(_x, _y, _width, _height); }
		
		ofTexture& getWarpTexture() 		{ return warpFbo.getTexture(); }
		ofPolyline& getWarpLine() 		{ return warpLine; }
		
		int 	getWarpWidth()			{ return warpFbo.getWidth(); }
		int 	getWarpHeight()			{ return warpFbo.getHeight(); }
		
		unsigned char*  getWarpData() 	{ return getWarpPixels().getData(); }
		ofPixels&       getWarpPixels();
		
	private:		
		ofParameterGroup	warpParameters;
		ofParameter<ofVec2f>*   warpPoints;
		void warpPointListener(ofVec2f& _value) { _value = ofVec2f(int(_value.x * 100.0) / 100.0, int(_value.y * 100.0) / 100.0); }
		
		ofFbo 				warpFbo;
		ofShader			invWarpShader;
		ofPlanePrimitive	warpPlane;
		ofPolyline			warpLine;
		
		void createWarpShader();
		void createWS2();
		void createWS3();
		
		ofPixels	pixels;
		bool		pixelsSet;
	};
}

