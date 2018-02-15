#pragma once

#include "ofMain.h"
#include "TexPC.h"

#define GLSL_120(shader)  "#version 120 \n" #shader
#define GLSL_150(shader)  "#version 150 \n" #shader

namespace ofxProsilica {
	
	class WarpTexPC : public TexPC {
	public:
		
		WarpTexPC() {}
		virtual ~WarpTexPC() {;}
		
		bool	setup();
		void	update();
		
		ofTexture& getWarpedTexture() 	{ return warpFbo.getTexture(); }
		ofPolyline& getWarpLine() 		{ return warpLine; }
		
		int 	getWarpWidth()			{ return warpFbo.getWidth(); }
		int 	getWarpHeight()			{ return warpFbo.getHeight(); }
		
		void reloadShader() {invWarpShader.load("invWarp");}
		
	private:		
		ofParameterGroup	warpParameters;
		ofParameter<ofVec2f>*   warpPoints;
		void warpPointListener(ofVec2f& _value) { _value = ofVec2f(int(_value.x * 100.0) / 100.0, int(_value.y * 100.0) / 100.0); }
		
		ofFbo 				warpFbo;
		ofShader			invWarpShader;
		ofPlanePrimitive	warpPlane;
		ofPolyline			warpLine;
		
		void createShader();
		void createShader2();
		void createShader3();
		
	};
}

