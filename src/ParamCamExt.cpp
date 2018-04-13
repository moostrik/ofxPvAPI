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
		
		pixelsSet = false;
		
		if (ofIsGLProgrammableRenderer()) { createRed2LumShader(); }
		
		fhInited = false;
		
		// bug in OF won't allow for ofFbo's to be re-allocated with different internal format so default to RGB
		flipFbo.allocate(640, 480, GL_RGB);
		flipFbo.begin();
		ofClear(0);
		flipFbo.end();
		
		flipQuad.getVertices().resize(4);
		flipQuad.getTexCoords().resize(4);
		flipQuad.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
		
		flipParameters.setName("flip");
		flipParameters.add(flipH.set("flip H", false));
		flipParameters.add(flipV.set("flip V", false));
		flipParameters.add(rotate90.set("rotate 90", false));
		flipH.addListener(this, &ParamCamExt::pFlipListener);
		flipV.addListener(this, &ParamCamExt::pFlipListener);
		rotate90.addListener(this, &ParamCamExt::pFlipListener);
		parameters.add(flipParameters);
		
		homographyParameters.setName("homography warp");
		pHomographyPoints = new ofParameter<ofVec2f>[4];
		homographyParameters.add(pHomographyPoints[0].set("up left", ofVec2f(0,0), ofVec2f(-.5,-.5), ofVec2f(0.5,0.5)));
		homographyParameters.add(pHomographyPoints[1].set("up right", ofVec2f(1,0), ofVec2f(0.5,-.5), ofVec2f(1.5,0.5)));
		homographyParameters.add(pHomographyPoints[2].set("down left", ofVec2f(1,1), ofVec2f(0.5,0.5), ofVec2f(1.5,1.5)));
		homographyParameters.add(pHomographyPoints[3].set("down right", ofVec2f(0,1), ofVec2f(-.5,1.5), ofVec2f(0.5,1.5)));
		for (int i=0; i<4; i++) { pHomographyPoints[i].addListener(this, &ParamCamExt::pHomographyPointListener); }
		parameters.add(homographyParameters);
		
		return true;
	}
	
	void ParamCamExt::update() {
		ParamCam::update();
		
		if (ParamCam::isFrameNew()){
			int w = (rotate90)? ParamCam::getWidth(): ParamCam::getHeight();
			int h = (rotate90)? ParamCam::getHeight(): ParamCam::getWidth();
			int glFormat = ofGetGLInternalFormatFromPixelFormat(getPixelFormat());
			// bug in OF won't allow for ofFbo's to be re-allocated with different internal format
			glFormat = GL_RGB;
			
			if (!fhInited || flipFbo.getWidth() != w || flipFbo.getHeight() != h || flipFbo.getTexture().getTextureData().glInternalFormat != glFormat) {
				fhInited = true;
				flipFbo.allocate(w, h, glFormat);
				updateFlip();
				updateHomography();
			}
			
			if (ofIsGLProgrammableRenderer() && getPixelFormat() == OF_PIXELS_MONO) {
				flipFbo.begin();
				ofClear(0);
				red2lumShader.begin();
				Camera::getTexture().bind();
				ofPushMatrix();
				ofMultMatrix(homography);
				flipQuad.draw();
				ofPopMatrix();
				Camera::getTexture().unbind();
				red2lumShader.end();
				flipFbo.end();
			}
			else {
				flipFbo.begin();
				ofClear(0);
				Camera::getTexture().bind();
				ofPushMatrix();
				ofMultMatrix(homography);
				flipQuad.draw();
				ofPopMatrix();
				Camera::getTexture().unbind();
				flipFbo.end();
			}
			
			pixelsSet = false;
		}
	}
	
	//----------------------------------------------------------------------------
	//-- PIXELS ------------------------------------------------------------------
	
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
	
	
	//----------------------------------------------------------------------------
	//-- SHADER ------------------------------------------------------------------
	
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
	
	
	//----------------------------------------------------------------------------
	//-- FLIP --------------------------------------------------------------------
	
	void ParamCamExt::updateFlip() {
		int w = ParamCam::getWidth();
		int h = ParamCam::getHeight();
		
		if (!rotate90) {	// NO ROTATION
			flipQuad.setVertex(0, ofVec3f(0,0,0));
			flipQuad.setVertex(1, ofVec3f(w,0,0));
			flipQuad.setVertex(2, ofVec3f(w,h,0));
			flipQuad.setVertex(3, ofVec3f(0,h,0));
			if (!flipH) {
				if (!flipV) {	// NO FLIP
					flipQuad.setTexCoord(0, ofVec2f(0, 0) );
					flipQuad.setTexCoord(0, ofVec2f(w, 0) );
					flipQuad.setTexCoord(0, ofVec2f(w, h) );
					flipQuad.setTexCoord(0, ofVec2f(0, h) );
				}
				else {			// FLIP V
					flipQuad.setTexCoord(0, ofVec2f(0, h) );
					flipQuad.setTexCoord(0, ofVec2f(w, h) );
					flipQuad.setTexCoord(0, ofVec2f(w, 0) );
					flipQuad.setTexCoord(0, ofVec2f(0, 0) );
				}
			}
			else {
				if (!flipV) {	// FLIP H
					flipQuad.setTexCoord(0, ofVec2f(w, 0) );
					flipQuad.setTexCoord(0, ofVec2f(0, 0) );
					flipQuad.setTexCoord(0, ofVec2f(0, h) );
					flipQuad.setTexCoord(0, ofVec2f(w, h) );
				}
				else {			// FLIP H & V
					flipQuad.setTexCoord(0, ofVec2f(w, h) );
					flipQuad.setTexCoord(0, ofVec2f(0, h) );
					flipQuad.setTexCoord(0, ofVec2f(0, 0) );
					flipQuad.setTexCoord(0, ofVec2f(w, 0) );
				}
			}
		}
		else {				// ROTATION
			flipQuad.setVertex(0, ofVec3f(0,0,0));
			flipQuad.setVertex(1, ofVec3f(h,0,0));
			flipQuad.setVertex(2, ofVec3f(h,w,0));
			flipQuad.setVertex(3, ofVec3f(0,w,0));
			if (!flipH) {
				if (!flipV) {	// NO FLIP
					flipQuad.setTexCoord(0, ofVec2f(0, h) );
					flipQuad.setTexCoord(0, ofVec2f(0, 0) );
					flipQuad.setTexCoord(0, ofVec2f(w, 0) );
					flipQuad.setTexCoord(0, ofVec2f(w, h) );
				}
				else {			// FLIP V
					flipQuad.setTexCoord(0, ofVec2f(0, 0) );
					flipQuad.setTexCoord(0, ofVec2f(0, h) );
					flipQuad.setTexCoord(0, ofVec2f(w, h) );
					flipQuad.setTexCoord(0, ofVec2f(w, 0) );
				}
			}
			else {
				if (!flipV) {	// FLIP H
					flipQuad.setTexCoord(0, ofVec2f(w, h) );
					flipQuad.setTexCoord(0, ofVec2f(w, 0) );
					flipQuad.setTexCoord(0, ofVec2f(0, 0) );
					flipQuad.setTexCoord(0, ofVec2f(0, h) );
				}
				else {			// FLIP H & V
					flipQuad.setTexCoord(0, ofVec2f(w, 0) );
					flipQuad.setTexCoord(0, ofVec2f(w, h) );
					flipQuad.setTexCoord(0, ofVec2f(0, h) );
					flipQuad.setTexCoord(0, ofVec2f(0, 0) );
				}
			}
		}
	}
	
	
	//----------------------------------------------------------------------------
	//-- HOMOGRAPHY -- Functions adapted from Arturo Castro : --------------------
	
	void ParamCamExt::updateHomography() {
		int w = getWidth();
		int h = getHeight();
		
		ofVec2f originalCorners[4];
		originalCorners[0] = ofVec2f(0,0);
		originalCorners[1] = ofVec2f(w,0);
		originalCorners[2] = ofVec2f(w,h);
		originalCorners[3] = ofVec2f(0,h);
		
		ofVec2f distortedCorners[4];
		distortedCorners[0] = pHomographyPoints[0].get() * ofVec2f(w,h);
		distortedCorners[1] = pHomographyPoints[1].get() * ofVec2f(w,h);
		distortedCorners[2] = pHomographyPoints[2].get() * ofVec2f(w,h);
		distortedCorners[3] = pHomographyPoints[3].get() * ofVec2f(w,h);
		
		homography = findHomography(originalCorners, distortedCorners);
	}
	
	ofMatrix4x4 ParamCamExt::findHomography(ofVec2f* src, ofVec2f* dst){
		float homography[16];
		float P[8][9]={
			{ -src[0].x, -src[0].y, -1, 0, 0, 0, src[0].x*dst[0].x, src[0].y*dst[0].x, -dst[0].x },
			{ 0, 0, 0, -src[0].x, -src[0].y, -1, src[0].x*dst[0].y, src[0].y*dst[0].y, -dst[0].y },
			{ -src[1].x, -src[1].y, -1, 0, 0, 0, src[1].x*dst[1].x, src[1].y*dst[1].x, -dst[1].x },
			{ 0, 0, 0, -src[1].x, -src[1].y, -1, src[1].x*dst[1].y, src[1].y*dst[1].y, -dst[1].y },
			{ -src[2].x, -src[2].y, -1, 0, 0, 0, src[2].x*dst[2].x, src[2].y*dst[2].x, -dst[2].x },
			{ 0, 0, 0, -src[2].x, -src[2].y, -1, src[2].x*dst[2].y, src[2].y*dst[2].y, -dst[2].y },
			{ -src[3].x, -src[3].y, -1, 0, 0, 0, src[3].x*dst[3].x, src[3].y*dst[3].x, -dst[3].x },
			{ 0, 0, 0, -src[3].x, -src[3].y, -1, src[3].x*dst[3].y, src[3].y*dst[3].y, -dst[3].y },
		};
		gaussian_elimination(&P[0][0],9);
		float aux_H[]={ P[0][8], P[3][8], 0, P[6][8], P[1][8], P[4][8], 0, P[7][8], 0, 0, 1, 0, P[2][8], P[5][8], 0, 1 };
		for(int i=0;i<16;i++) { homography[i] = aux_H[i]; }
		return ofMatrix4x4(homography);
	}
	
	void ParamCamExt::gaussian_elimination(float *input, int n){
		float * A = input;
		int i = 0;
		int j = 0;
		int m = n-1;
		while (i < m && j < n){
			int maxi = i;
			for(int k = i+1; k<m; k++){
				if(fabs(A[k*n+j]) > fabs(A[maxi*n+j])){
					maxi = k;
				}
			}
			if (A[maxi*n+j] != 0){
				if(i!=maxi)
					for(int k=0;k<n;k++){
						float aux = A[i*n+k];
						A[i*n+k]=A[maxi*n+k];
						A[maxi*n+k]=aux;
					}
				float A_ij=A[i*n+j];
				for(int k=0;k<n;k++){
					A[i*n+k]/=A_ij;
				}
				for(int u = i+1; u< m; u++){
					float A_uj = A[u*n+j];
					for(int k=0;k<n;k++){
						A[u*n+k]-=A_uj*A[i*n+k];
					}
				}
				i++;
			}
			j++;
		}
		
		for(int i=m-2;i>=0;i--){
			for(int j=i+1;j<n-1;j++){
				A[i*n+m]-=A[i*n+j]*A[j*n+m];
				A[i*n+j]=0;
			}
		}
	}
}


