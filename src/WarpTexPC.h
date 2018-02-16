#pragma once

#include "ofMain.h"
#include "TexPC.h"

namespace ofxProsilica {
	
	class WarpTexPC : public TexPC {
	public:
		
		WarpTexPC() {}
		virtual ~WarpTexPC() {;}
		
		bool	setup(ofTexture _tex = ofTexture());
		void	update();
		
		ofTexture& getWarpedTexture() 	{ return warpFbo.getTexture(); }
		ofPolyline& getWarpLine() 		{ return warpLine; }
		
		int 	getWarpWidth()			{ return warpFbo.getWidth(); }
		int 	getWarpHeight()			{ return warpFbo.getHeight(); }
		
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
		
	};
}

