//
//  TexPC.cpp
//  plCam
//
//  Created by PLPLPL.PL on 08/01/16.
//
//

#include "WarpTexPC.h"


namespace ofxProsilica {
	
	bool WarpTexPC::setup(){
		TexPC::setup();
		
		warpParameters.setName("warp");
		warpPoints = new ofParameter<ofVec2f>[5];
		warpParameters.add(warpPoints[0].set("p0", ofVec2f(0,0), ofVec2f(0,0), ofVec2f(1,1)));
		warpParameters.add(warpPoints[1].set("p1", ofVec2f(1,0), ofVec2f(0,0), ofVec2f(1,1)));
		warpParameters.add(warpPoints[2].set("p2", ofVec2f(0,1), ofVec2f(0,0), ofVec2f(1,1)));
		warpParameters.add(warpPoints[3].set("p3", ofVec2f(1,1), ofVec2f(0,0), ofVec2f(1,1)));
//		warpParameters.add(warpPoints[4].set("power H V", ofVec2f(1,1), ofVec2f(.5,.5), ofVec2f(2,2)));
		for (int i=0; i<5; i++) { warpPoints[i].addListener(this, &::ofxProsilica::WarpTexPC::warpPointListener); }
		parameters.add(warpParameters);
		
		warpFbo.allocate(getTexture().getWidth(), getTexture().getHeight(), GL_RGB);
		
		warpFbo.begin();
		ofClear(0,0,255);
		texture.draw(0,0);
		warpFbo.end();
		
		createShader();
		
		ofVec2f p1 = warpParameters.get<ofVec2f>("p0");
		ofVec2f p2 = warpParameters.get<ofVec2f>("p1");
		ofVec2f p3 = warpParameters.get<ofVec2f>("p2");
		ofVec2f p4 = warpParameters.get<ofVec2f>("p3");
		
		vector<ofPoint> verts = {p1, p2, p4, p3, p1};
		warpLine = ofPolyline(verts);
		
		
		return true;
	}
	
	//--------------------------------------------------------------
	void WarpTexPC::update() {
		TexPC::update();
		
		if (isFrameNew()){
		
			ofVec2f p1 = warpParameters.get<ofVec2f>("p0");
			ofVec2f p2 = warpParameters.get<ofVec2f>("p1");
			ofVec2f p3 = warpParameters.get<ofVec2f>("p2");
			ofVec2f p4 = warpParameters.get<ofVec2f>("p3");
			
			vector<ofPoint> verts = {p1, p2, p4, p3, p1};
			warpLine = ofPolyline(verts);
			
			float w = max(fabs(p2.x - p1.x), fabs(p4.x - p3.x)) * getWidth();
			float h = max(fabs(p3.y - p1.y), fabs(p4.y - p2.y)) * getHeight();
			
			warpPlane.set(w, h, 16, 16);
			
			if(warpFbo.getWidth() != w || warpFbo.getHeight() != h) {
				warpFbo.allocate(w, h, GL_RGB);
			}
			
			warpFbo.begin();
			ofClear(0);
			invWarpShader.begin();
			TexPC::getTexture().bind();
			invWarpShader.setUniform2f("inputDimensions", getWidth(), getHeight());
			invWarpShader.setUniform2f("upper_left", p1.x, p1.y);
			invWarpShader.setUniform2f("upper_right", p2.x, p2.y);
			invWarpShader.setUniform2f("lower_left", p3.x, p3.y);
			invWarpShader.setUniform2f("lower_right", p4.x, p4.y);
			invWarpShader.setUniform2f("powr", 1, 1);
		
			ofPushMatrix();
			ofTranslate(w/2, h/2);
			warpPlane.draw();
			ofPopMatrix();
			
			TexPC::getTexture().unbind();
			invWarpShader.end();
			warpFbo.end();
		}
	}
	
	//--------------------------------------------------------------
	void WarpTexPC::createShader() {
		if (ofIsGLProgrammableRenderer()) { createShader3(); }
		else {  createShader2(); }
	}
	
	//--------------------------------------------------------------
	void WarpTexPC::createShader2() {
		string fragmentShader, vertexShader;
		vertexShader = GLSL_120(
								varying vec2 texCoordVarying;
								
								void main()
								{
									vec2 texcoord = gl_MultiTexCoord0.xy;
									texCoordVarying = vec2(texcoord.x, texcoord.y);
									gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;;
								}
								);
		
		fragmentShader = GLSL_120(
								  uniform sampler2DRect tex0;
								  uniform vec2 inputDimensions;
								  
								  uniform vec2 upper_left;
								  uniform vec2 upper_right;
								  uniform vec2 lower_left;
								  uniform vec2 lower_right;
								  
								  uniform vec2 powr;
								  
								  varying vec2 texCoordVarying;
								  
								  void main()
								  {
									  vec2 texcoord0 = texCoordVarying;
									  
									  float destX = pow(texcoord0.x, powr.x);
									  float destY = pow(texcoord0.y, powr.y);;
									  
									  float sourceXUp = destX * (upper_right.x - upper_left.x) + upper_left.x;
									  float sourceXDown = destX * (lower_right.x - lower_left.x) + lower_left.x;
									  float sourceX = mix(sourceXUp, sourceXDown, destY);
									  
									  float sourceYLeft = destY * (lower_left.y - upper_left.y) + upper_left.y;
									  float sourceYRight = destY * (lower_right.y - upper_right.y) + upper_right.y;
									  float sourceY = mix(sourceYLeft, sourceYRight, destX);
									  
									  gl_FragColor = texture2DRect(tex0,vec2(sourceX * inputDimensions.x, sourceY * inputDimensions.y));
								  }
								  );
		invWarpShader.setupShaderFromSource(GL_VERTEX_SHADER, vertexShader);
		invWarpShader.setupShaderFromSource(GL_FRAGMENT_SHADER, fragmentShader);
		invWarpShader.linkProgram();
	}
	
	//--------------------------------------------------------------
	void WarpTexPC::createShader3() {
		string fragmentShader, vertexShader;
		vertexShader = GLSL_150(
								uniform mat4 textureMatrix;
								uniform mat4 modelViewProjectionMatrix;
								
								in vec4  position;
								in vec2  texcoord;
								
								out vec2 texCoordVarying;
								
								void main(){
									texCoordVarying = (textureMatrix*vec4(texcoord.x,texcoord.y,0,1)).xy;
									gl_Position = modelViewProjectionMatrix * position;
								}
								);
		
		fragmentShader = GLSL_150(
								  uniform sampler2DRect tex0;
								  uniform vec2 inputDimensions;
								  uniform vec2 upper_left;
								  uniform vec2 upper_right;
								  uniform vec2 lower_left;
								  uniform vec2 lower_right;
								  uniform vec2 powr;
								  
								  in vec2 texCoordVarying;
								  
								  out vec4 fragColor;
								  
								  void main(){
									  float destX = pow(texCoordVarying.x, powr.x);
									  float destY = pow(texCoordVarying.y, powr.y);
									  
									  float sourceXUp = destX * (upper_right.x - upper_left.x) + upper_left.x;
									  float sourceXDown = destX * (lower_right.x - lower_left.x) + lower_left.x;
									  float sourceX = mix(sourceXUp, sourceXDown, destY);
									  
									  float sourceYLeft = destY * (lower_left.y - upper_left.y) + upper_left.y;
									  float sourceYRight = destY * (lower_right.y - upper_right.y) + upper_right.y;
									  float sourceY = mix(sourceYLeft, sourceYRight, destX);
									  
									  fragColor = texture(tex0,vec2(sourceX * inputDimensions.x, sourceY * inputDimensions.y));
								  }
								  );
		invWarpShader.setupShaderFromSource(GL_VERTEX_SHADER, vertexShader);
		invWarpShader.setupShaderFromSource(GL_FRAGMENT_SHADER, fragmentShader);
		invWarpShader.bindDefaults();
		invWarpShader.linkProgram();
	}
}

