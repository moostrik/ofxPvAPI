#pragma once

#include "ofMain.h"
#include "ParamCam.h"

#define GLSL_120(shader)  "#version 120 \n" #shader
#define GLSL_150(shader)  "#version 150 \n" #shader

namespace ofxPvAPI {
	
	class ParamCamExt : public ParamCam{
		
	public:
		ParamCamExt() {}
		virtual	~ParamCamExt() {;}
		
		bool		setup();
		void		update();
		void 		draw(int _x, int _y) { draw(0, 0, this->getWidth(), this->getHeight()); }
		void 		draw(int _x, int _y, int _width, int _height) { getTexture().draw(_x, _y, _width, _height); }
		
		float		getWidth()		{ return flipFbo.getWidth(); }
		float		getHeight()		{ return flipFbo.getHeight(); }
		
		ofTexture&	getTexture()	{ return flipFbo.getTexture(); }
		ofPixels&	getPixels();
		
	private:
		bool		fhInited;
		
		//-- PIXELS ------------------------------------------------------------------
		ofPixels		pixels;
		ofPixels		pixelsRGB;
		bool			pixelsSet;
		
		//-- SHADER ------------------------------------------------------------------
		ofShader 		red2lumShader;
		void 			createRed2LumShader();
		
		//-- FLIP --------------------------------------------------------------------
		ofFbo 				flipFbo;
		ofMesh				flipQuad;
		void				updateFlip();
		
		ofParameterGroup	flipParameters;
		ofParameter<bool>	flipH;
		ofParameter<bool>	flipV;
		ofParameter<bool>	rotate90;
		void				pFlipListener(bool& _value) { updateFlip(); }
		
		//-- HOMOGRAPHY -- Functions adapted from Arturo Castro : --------------------
		//-- http://www.openframeworks.cc/forum/viewtopic.php?p=22611 ----------------
		
		ofParameterGroup	homographyParameters;
		ofParameter<ofVec2f>* pHomographyPoints;
		void				pHomographyPointListener(ofVec2f& _value) { updateHomography(); }
		
		void				updateHomography();
		ofMatrix4x4			homography;
		ofMatrix4x4			findHomography(ofVec2f* src, ofVec2f* dst);
		void				gaussian_elimination(float *input, int n);
	};
}

