#pragma once

#include "ofMain.h"
#include "ParameterConnector.h"

namespace ofxProsilica {
	
	class TexPC : public ParameterConnector {
	public:
		
		TexPC() {}
		virtual ~TexPC() {;}
		
		bool	setup();
		void	update();
		
		ofTexture& getTexture() { return (hasFlip)? flipFbo.getTexture(): texture; }
		
	private:		
		ofParameterGroup	flipParameters;
		ofParameter<bool>   flipH;
		ofParameter<bool>   flipV;
		ofParameter<bool>   rotate90;
		
		ofFbo flipFbo;
		
		bool hasFlip;
		
	protected:
		ofTexture texture;
	};
}
