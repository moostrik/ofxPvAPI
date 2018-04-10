//
//  ParamCamExt.cpp
//  plCam
//
//  Created by PLPLPL.PL on 08/01/16.
//
//

#include "ParamCamExt.h"


namespace ofxPvAPI {
	
	bool ParamCamExt::setup(){
		ParamCam::setup();
		
		// bug in OF won't allow for ofFbo's to be re-allocated with different internal format so default to RGB
		flipFbo.allocate(640, 480, GL_RGB);
		flipFbo.begin();
		ofClear(0);
		flipFbo.end();
		
		if (ofIsGLProgrammableRenderer()) { createRed2LumShader(); }
		quad.getVertices().resize(4);
		quad.getTexCoords().resize(4);
		quad.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
		
		pixelsSet = false;
		
		flipParameters.setName("flip");
		flipParameters.add(flipH.set("flip H", false));
		flipParameters.add(flipV.set("flip V", false));
		flipParameters.add(rotate90.set("rotate 90", false));
		
		parameters.add(flipParameters);
		
		return true;
	}
	
	//--------------------------------------------------------------
	void ParamCamExt::update() {
		ParamCam::update();
		
		if (ParamCam::isFrameNew()){
			int w = ParamCam::getWidth();
			int h = ParamCam::getHeight();
			int glFormat = ofGetGLInternalFormatFromPixelFormat(getPixelFormat());
			
			int dstWidth = w;
			int dstHeight = h;
			
			if (rotate90) {
				dstWidth = h;
				dstHeight = w;
			}
			
			// bug in OF won't allow for ofFbo's to be re-allocated with different internal format
			glFormat = GL_RGB;
			if (flipFbo.getWidth() != dstWidth || flipFbo.getHeight() != dstHeight || flipFbo.getTexture().getTextureData().glInternalFormat != glFormat) {
				flipFbo.allocate(dstWidth, dstHeight, glFormat);
			}
			
			vector<ofPoint> pts;
			pts.assign(4, ofPoint(0,0));
			
			if (!rotate90) {	// NO ROTATION
				if (!flipH) {
					if (!flipV) {  // NO FLIP
						pts[0].set(0, 0);
						pts[1].set(w, 0);
						pts[2].set(w, h);
						pts[3].set(0, h);
					}
					else {          // FLIP V
						pts[0].set(0, h);
						pts[1].set(w, h);
						pts[2].set(w, 0);
						pts[3].set(0, 0);
					}
				}
				else {
					if (!flipV) {  // FLIP H
						pts[0].set(w, 0);
						pts[1].set(0, 0);
						pts[2].set(0, h);
						pts[3].set(w, h);
					}
					else {          // FLIP H & V
						pts[0].set(w, h);
						pts[1].set(0, h);
						pts[2].set(0, 0);
						pts[3].set(w, 0);
					}
				}
			}
			else {	// ROTATION
				if (!flipH) {
					if (!flipV) {  // NO FLIP
						pts[0].set(0, h);
						pts[1].set(0, 0);
						pts[2].set(w, 0);
						pts[3].set(w, h);
					}
					else {          // FLIP V
						pts[0].set(0, 0);
						pts[1].set(0, h);
						pts[2].set(w, h);
						pts[3].set(w, 0);
					}
				}
				else {
					if (!flipV) {  // FLIP H
						pts[0].set(w, h);
						pts[1].set(w, 0);
						pts[2].set(0, 0);
						pts[3].set(0, h);
					}
					else {          // FLIP H & V
						pts[0].set(w, 0);
						pts[1].set(w, h);
						pts[2].set(0, h);
						pts[3].set(0, 0);
					}
				}
			}
			
			
			quad.setVertex(0, ofVec3f(0,0,0));
			quad.setVertex(1, ofVec3f(dstWidth,0,0));
			quad.setVertex(2, ofVec3f(dstWidth,dstHeight,0));
			quad.setVertex(3, ofVec3f(0,dstHeight,0));
			
			quad.setTexCoord(0, pts[0]);
			quad.setTexCoord(1, pts[1]);
			quad.setTexCoord(2, pts[2]);
			quad.setTexCoord(3, pts[3]);
			
			if (ofIsGLProgrammableRenderer() && getPixelFormat() == OF_PIXELS_MONO) {
				flipFbo.begin();
				ofClear(0);
				red2lumShader.begin();
				Camera::getTexture().bind();
				quad.draw();
				Camera::getTexture().unbind();
				red2lumShader.end();
				flipFbo.end();
			}
			else {
				flipFbo.begin();
				ofClear(0);
				Camera::getTexture().bind();
				quad.draw();
				Camera::getTexture().unbind();
				flipFbo.end();
			}
			
			pixelsSet = false;
		}
	}
	
	//--------------------------------------------------------------
	ofPixels& ParamCamExt::getPixels() {
		if (!pixelsSet) {
			ofTextureData& texData = this->getTexture().getTextureData();
			
			int numChannels = 1;
			int readFormat = GL_RED;
			if (getPixelFormat() != OF_PIXELS_MONO) {
				numChannels = 3;
				readFormat = GL_RGB;
			}
			
			if (pixels.getWidth() != texData.width || pixels.getHeight() != texData.height || pixels.getNumChannels() != numChannels) {
				pixels.allocate(texData.width, texData.height, numChannels);
			}
			
			ofSetPixelStoreiAlignment(GL_PACK_ALIGNMENT, texData.width, 1, numChannels);
			glBindTexture(texData.textureTarget, texData.textureID);
			glGetTexImage(texData.textureTarget, 0, readFormat, GL_UNSIGNED_BYTE, pixels.getData());
			glBindTexture(texData.textureTarget, 0);
			
			pixelsSet = true;
		}
		return pixels;
	}

	//--------------------------------------------------------------
	void ParamCamExt::createRed2LumShader() {
		string vertexShader, fragmentShader;
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
								   
								   in vec2 texCoordVarying;
								   
								   out vec4 fragColor;
								   
								   void main(){
									   float red = texture(tex0, texCoordVarying).x;
									   fragColor = vec4(red, red, red, 1.0);
								   }
								   );
		
		red2lumShader.setupShaderFromSource(GL_VERTEX_SHADER, vertexShader);
		red2lumShader.setupShaderFromSource(GL_FRAGMENT_SHADER, fragmentShader);
		red2lumShader.bindDefaults();
		red2lumShader.linkProgram();
	}
	
}

