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
		
		ofRectangle	getMaxRectForHomography() { return getOptimalRectForHomography(Camera::getROIWidthMax(), Camera::getROIHeightMax()); }
		ofRectangle	getOptimalRectForHomography(int _width, int _height);
		
	private:
		
		// Barrel Distortion adapted from meshula: https://www.shadertoy.com/view/MlSXR3
		ofParameterGroup	distortionParameters;
		ofParameter<bool>	pDoDistortion;
		ofParameter<float>	pDistortionK1;
		ofParameter<float>	pDistortionK2;
		ofShader 			barrelShader;
		void 				createBarrelShader();
		
		ofParameterGroup	flipParameters;
		ofParameter<bool>	pFlipH;
		ofParameter<bool>	pFlipV;
		ofParameter<bool>	pRotate90;
		ofFbo 				flipFbo;
		ofMesh				flipQuad;
		void				pFlipListener(bool & _value) { updateFlip(); }
		void				updateFlip();
		
//		ofParameterGroup	rotationParameters;
//		ofParameter<bool>	pDoRotation;
//		ofParameter<ofVec3f>pRotation;
		
		// Homography Functions adapted from Arturo Castro: http://www.openframeworks.cc/forum/viewtopic.php?p=22611
		ofParameterGroup	homographyParameters;
		ofParameter<bool>	pDoHomography;
		ofParameter<ofVec2f>* pHomographyPoints;
		void				pHomographyPointListener(ofVec2f& _value) { updateHomography(); }
		void				updateHomography();
		ofMatrix4x4			homography;
		ofMatrix4x4			findHomography(ofVec2f* src, ofVec2f* dst);
		void				gaussian_elimination(float *input, int n);
		
		ofPixels	pixels;
		ofPixels	pixelsRGB;
		bool		pixelsSet;
	};
}

