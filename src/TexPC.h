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
		
		bool	setup(ofTexture _tex = ofTexture());
		void	update();
		
		ofTexture& getTexture() { return (useFbo)? flipFbo.getTexture() : texture; }
		
	private:		
		ofParameterGroup	flipParameters;
		ofParameter<bool>   flipH;
		ofParameter<bool>   flipV;
		ofParameter<bool>   rotate90;
		
		ofTexture 	texture;
		ofFbo 		flipFbo;
		bool 		useFbo;
		ofMesh		quad;
		ofShader 	red2lumShader;
		void 		createRed2LumShader();
	};
}
