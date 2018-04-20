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
		flipQuad.getVertices().resize(4);
		flipQuad.getTexCoords().resize(4);
		flipQuad.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
		
		pixelsSet = false;
		
		flipParameters.setName("flip");
		flipParameters.add(flipH.set("flip H", false));
		flipParameters.add(flipV.set("flip V", false));
		flipParameters.add(rotate90.set("rotate 90", false));
		parameters.add(flipParameters);
		
		homographyParameters.setName("homography warp");
		homographyParameters.add(pDoHomography.set("enable", true));
		pHomographyPoints = new ofParameter<ofVec2f>[4];
		homographyParameters.add(pHomographyPoints[0].set("up left", ofVec2f(0,0), ofVec2f(-.5,-.5), ofVec2f(0.5,0.5)));
		homographyParameters.add(pHomographyPoints[1].set("up right", ofVec2f(1,0), ofVec2f(0.5,-.5), ofVec2f(1.5,0.5)));
		homographyParameters.add(pHomographyPoints[2].set("down left", ofVec2f(1,1), ofVec2f(0.5,0.5), ofVec2f(1.5,1.5)));
		homographyParameters.add(pHomographyPoints[3].set("down right", ofVec2f(0,1), ofVec2f(-.5,0.5), ofVec2f(0.5,1.5)));
		for (int i=0; i<4; i++) { pHomographyPoints[i].addListener(this, &ParamCamExt::pHomographyPointListener); }
		parameters.add(homographyParameters);
		
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
				updateHomography();
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
			
			
			flipQuad.setVertex(0, ofVec3f(0,0,0));
			flipQuad.setVertex(1, ofVec3f(dstWidth,0,0));
			flipQuad.setVertex(2, ofVec3f(dstWidth,dstHeight,0));
			flipQuad.setVertex(3, ofVec3f(0,dstHeight,0));
			
			flipQuad.setTexCoord(0, pts[0]);
			flipQuad.setTexCoord(1, pts[1]);
			flipQuad.setTexCoord(2, pts[2]);
			flipQuad.setTexCoord(3, pts[3]);
			
			if (ofIsGLProgrammableRenderer() && getPixelFormat() == OF_PIXELS_MONO) {
				flipFbo.begin();
				ofClear(0);
				red2lumShader.begin();
				Camera::getTexture().bind();
				flipQuad.draw();
				ofPushMatrix();
				if (pDoHomography) { ofMultMatrix(homography); }
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
				flipQuad.draw();
				ofPushMatrix();
				if (pDoHomography) { ofMultMatrix(homography); }
				flipQuad.draw();
				ofPopMatrix();
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
	
	//--------------------------------------------------------------
	
	void ParamCamExt::updateHomography() {
		int w = getWidth();
		int h = getHeight();
		
		ofVec2f srcCorners[4];
		srcCorners[0] = pHomographyPoints[0].get() * ofVec2f(w,h);
		srcCorners[1] = pHomographyPoints[1].get() * ofVec2f(w,h);
		srcCorners[2] = pHomographyPoints[2].get() * ofVec2f(w,h);
		srcCorners[3] = pHomographyPoints[3].get() * ofVec2f(w,h);
		
		ofVec2f dstCorners[4];
		dstCorners[0] = ofVec2f(0,0);
		dstCorners[1] = ofVec2f(w,0);
		dstCorners[2] = ofVec2f(w,h);
		dstCorners[3] = ofVec2f(0,h);
		
		homography = findHomography(srcCorners, dstCorners);
	}
	
	ofMatrix4x4 ParamCamExt::findHomography(ofVec2f* src, ofVec2f* dst){
		float homography[16];
		float P[8][9]={
			{-src[0].x, -src[0].y, -1,   0,   0,  0, src[0].x*dst[0].x, src[0].y*dst[0].x, -dst[0].x },
			{  0,   0,  0, -src[0].x, -src[0].y, -1, src[0].x*dst[0].y, src[0].y*dst[0].y, -dst[0].y },
			{-src[1].x, -src[1].y, -1,   0,   0,  0, src[1].x*dst[1].x, src[1].y*dst[1].x, -dst[1].x },
			{  0,   0,  0, -src[1].x, -src[1].y, -1, src[1].x*dst[1].y, src[1].y*dst[1].y, -dst[1].y },
			{-src[2].x, -src[2].y, -1,   0,   0,  0, src[2].x*dst[2].x, src[2].y*dst[2].x, -dst[2].x },
			{  0,   0,  0, -src[2].x, -src[2].y, -1, src[2].x*dst[2].y, src[2].y*dst[2].y, -dst[2].y },
			{-src[3].x, -src[3].y, -1,   0,   0,  0, src[3].x*dst[3].x, src[3].y*dst[3].x, -dst[3].x },
			{  0,   0,  0, -src[3].x, -src[3].y, -1, src[3].x*dst[3].y, src[3].y*dst[3].y, -dst[3].y },
		};
		gaussian_elimination(&P[0][0],9);
		float aux_H[]={ P[0][8],P[3][8],0,P[6][8],P[1][8],P[4][8],0,P[7][8],0,0,1,0,P[2][8],P[5][8],0,1};
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

	ofRectangle ParamCamExt::getOptimalRectForHomography(int _width, int _height) {
		float minX = min((pHomographyPoints[0].get().x, pHomographyPoints[3].get().x), 0.f);
		float maxX = max((pHomographyPoints[1].get().x, pHomographyPoints[2].get().x), 1.f);
		float minY = min((pHomographyPoints[0].get().y, pHomographyPoints[1].get().y), 0.f);
		float maxY = max((pHomographyPoints[2].get().y, pHomographyPoints[3].get().y), 1.f);
		
		float oW = (maxX - minX) * _width;
		float oH = (maxY - minY) * _height;
		float oAR = oH / oW;
		
		float maxW, maxH;
		if (rotate90) {
			maxW = getROIHeightMax() + getROIY();
			maxH = getROIWidthMax() + getROIX();
		} else {
			maxW = getROIWidthMax() + getROIX();
			maxH = getROIHeightMax() + getROIY();
		}
		
		if (oW > maxW) {
			oW = maxW;
			oH = oW * oAR;
		}
		if (oH > maxH) {
			oH = maxH;
			oW = oH / oAR;
		}
		
		float oX = (maxW - oW) / 2.0;
		float oY = (maxH - oH) / 2.0;
		
		
		if (rotate90) {
			return ofRectangle(oY, oX, oH, oW);
		} else {
			return ofRectangle(oX, oY, oW, oH);
		}
	}
}


