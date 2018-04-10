#pragma once

#include "ofMain.h"
#include "ParamCam.h"

#define GLSL_120(shader)  "#version 120 \n" #shader
#define GLSL_150(shader)  "#version 150 \n" #shader

namespace ofxPvAPI {
	
	class ParamCamExt : public ParamCam{
	public:
		
		ParamCamExt() {}
		virtual ~ParamCamExt() {;}
		
		bool	setup();
		void	update();
		void 	draw(int _x, int _y) { draw(0, 0, this->getWidth(), this->getHeight()); }
		void 	draw(int _x, int _y, int _width, int _height) { getTexture().draw(_x, _y, _width, _height); }
		
		float	getWidth()			{ return flipFbo.getWidth(); }
		float	getHeight()			{ return flipFbo.getHeight(); }
		
		ofTexture& 	getTexture()	{ return flipFbo.getTexture(); }
		ofPixels&   getPixels();
		
	private:		
		ofParameterGroup	flipParameters;
		ofParameter<bool>   flipH;
		ofParameter<bool>   flipV;
		ofParameter<bool>   rotate90;
	
		ofFbo 		flipFbo;
		ofMesh		quad;
		ofShader 	red2lumShader;
		void 		createRed2LumShader();
		
		ofPixels	pixels;
		ofPixels	pixelsRGB;
		bool		pixelsSet;
	};
}