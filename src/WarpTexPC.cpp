//
//  TexPC.cpp
//  plCam
//
//  Created by PLPLPL.PL on 08/01/16.
//
//

#include "WarpTexPC.h"


namespace ofxProsilica {
	
	bool WarpTexPC::setup(int _width, int _height){
		TexPC::setup();
		
		warpParameters.setName("warp");
		warpPoints = new ofParameter<ofVec2f>[5];
		warpParameters.add(warpPoints[0].set("p0", ofVec2f(0,0), ofVec2f(0,0), ofVec2f(1,1)));
		warpParameters.add(warpPoints[1].set("p1", ofVec2f(1,0), ofVec2f(0,0), ofVec2f(1,1)));
		warpParameters.add(warpPoints[2].set("p2", ofVec2f(0,1), ofVec2f(0,0), ofVec2f(1,1)));
		warpParameters.add(warpPoints[3].set("p3", ofVec2f(1,1), ofVec2f(0,0), ofVec2f(1,1)));
//		warpParameters.add(warpPoints[4].set("power H V", ofVec2f(1,1), ofVec2f(.5,.5), ofVec2f(2,2)));
//		for (int i=0; i<5; i++) { warpPoints[i].addListener(this, &::ofxProsilica::WarpTexPC::warpPointListener); }
		parameters.add(warpParameters);
		
		warpFbo.allocate(_width, _height, GL_RGB);
		
		warpFbo.begin();
		ofClear(0);
		texture.draw(_width, _height);
		warpFbo.end();
		
//		createShader();
		invWarpShader.load("invWarpShader");
		
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
			
			int w = warpFbo.getWidth();
			int h = warpFbo.getHeight();
			
			//		int bestWidth = max(fabs(p2.x - p1.x), fabs(p4.x - p3.x)) * camW;
			//		int bestHeight = max(fabs(p3.y - p1.y), fabs(p4.y - p2.y)) * camH;
			
			warpPlane.set(w, h, 16, 16);
			
			warpFbo.begin();
			ofClear(0);
			invWarpShader.begin();
			TexPC::getTexture().bind();
			invWarpShader.setUniform2f("inputDimentions", getWidth(), getHeight());
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
		if (ofIsGLProgrammableRenderer()) { createShader2(); }
		else { createShader3(); }
		
//		warpQuad.getVertices().resize(4);
//		warpQuad.getTexCoords().resize(4);
//		warpQuad.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
	}
	
	//--------------------------------------------------------------
	void WarpTexPC::createShader2() {
		vertexShader = GLSL120(
							   varying vec2 texCoordVarying;
							   
							   void main()
							   {
								   vec2 texcoord = gl_MultiTexCoord0.xy;
								   
								   // here we move the texture coordinates
								   texCoordVarying = vec2(texcoord.x, texcoord.y);
								   
								   // send the vertices to the fragment shader
								   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;;
							   }
							   );
		
		fragmentShader = GLSL120(
								 uniform sampler2DRect tex0;
								 
								 varying vec2 texCoordVarying;
								 uniform vec2 inputDimentions;
								 
								 uniform vec2 upper_left;
								 uniform vec2 upper_right;
								 uniform vec2 lower_left;
								 uniform vec2 lower_right;
								 
								 uniform vec2 powr;
								 
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
									 
									 gl_FragColor = texture2DRect(tex0,vec2(sourceX * inputDimentions.x, sourceY * inputDimentions.y));
								 }
								 );
		invWarpShader.setupShaderFromSource(GL_VERTEX_SHADER, vertexShader);
		invWarpShader.setupShaderFromSource(GL_FRAGMENT_SHADER, fragmentShader);
		invWarpShader.linkProgram();
	}
	
	//--------------------------------------------------------------
	void WarpTexPC::createShader3() {
		
	}
	
	
	
	
}

