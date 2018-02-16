#pragma once

#include "ofMain.h"
#include "ParameterConnector.h"

#define GLSL_120(shader)  "#version 120 \n" #shader
#define GLSL_150(shader)  "#version 150 \n" #shader

namespace ofxProsilica {
	
	class TexPC : public ParameterConnector {
	public:
		
		TexPC() {}
		virtual ~TexPC() {;}
		
		bool	setup(int _width, int _height) { ofLogWarning("ofxProsilica") << "setup with dimensions not yet supported"; setup(); };
		bool	setup(ofTexture _tex = ofTexture());
		void	update();
		void 	draw(int _x, int _y) { draw(0, 0, getTexture().getWidth(), getTexture().getHeight()); }
		void 	draw(int _x, int _y, int _width, int _height) { getTexture().draw(_x, _y, _width, _height); }
		
		ofTexture& getTexture() 	{ return flipFbo.getTexture(); }
		unsigned char*  getData() 	{ return getPixels().getData(); }
		ofPixels&       getPixels();
		
		float	getWidth()			{ return flipFbo.getWidth(); }
		float	getHeight()			{ return flipFbo.getHeight(); }
		
	private:		
		ofParameterGroup	flipParameters;
		ofParameter<bool>   flipH;
		ofParameter<bool>   flipV;
		ofParameter<bool>   rotate90;
	
		ofTexture 	internalTexture;
		ofFbo 		flipFbo;
		ofMesh		quad;
		ofShader 	red2lumShader;
		void 		createRed2LumShader();
		
		ofPixels	pixels;
		bool		pixelsSet;
	};
}
