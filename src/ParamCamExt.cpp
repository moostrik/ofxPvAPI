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
		
		// OF won't allow for ofFbo's to be re-allocated with different internal format, so I use a pointer
		flipFbo = new ofFbo;
		flipFbo->allocate(640, 480, GL_R8);
		flipFbo->getTexture().setSwizzle(GL_TEXTURE_SWIZZLE_G, GL_RED);
		flipFbo->getTexture().setSwizzle(GL_TEXTURE_SWIZZLE_B, GL_RED);
		flipFbo->begin();
		ofClear(0);
		flipFbo->end();
		
		flipQuad.getVertices().resize(4, glm::vec3(0.0));
		flipQuad.getTexCoords().resize(4, glm::vec2(0.0));
		flipQuad.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
		
		pixelsSet = false;
		
		createBarrelShader();
		
		distortionParameters.setName("barrel distortion");
		distortionParameters.add(pDoDistortion.set("enable", true));
		distortionParameters.add(pDistortionK1.set("K1", 0.0, -5.0, 5.0));
		distortionParameters.add(pDistortionK2.set("K2", 0.0, -5.0, 5.0));
		parameters.add(distortionParameters);
		
		flipParameters.setName("flip");
		flipParameters.add(pFlipH.set("flip H", false));
		flipParameters.add(pFlipV.set("flip V", false));
		flipParameters.add(pRotate90.set("rotate 90", false));
		pFlipH.addListener(this, &ParamCamExt::pFlipListener);
		pFlipV.addListener(this, &ParamCamExt::pFlipListener);
		pRotate90.addListener(this, &ParamCamExt::pFlipListener);
		parameters.add(flipParameters);
		
		homographyParameters.setName("homography warp");
		homographyParameters.add(pDoHomography.set("enable", true));
		pHomographyPoints = new ofParameter<glm::vec2>[4];
		homographyParameters.add(pHomographyPoints[0].set("up left", glm::vec2(0,0), glm::vec2(-.5,-.5), glm::vec2(0.5,0.5)));
		homographyParameters.add(pHomographyPoints[1].set("up right", glm::vec2(1,0), glm::vec2(0.5,-.5), glm::vec2(1.5,0.5)));
		homographyParameters.add(pHomographyPoints[2].set("down left", glm::vec2(1,1), glm::vec2(0.5,0.5), glm::vec2(1.5,1.5)));
		homographyParameters.add(pHomographyPoints[3].set("down right", glm::vec2(0,1), glm::vec2(-.5,0.5), glm::vec2(0.5,1.5)));
		for (int i=0; i<4; i++) { pHomographyPoints[i].addListener(this, &ParamCamExt::pHomographyPointListener); }
		parameters.add(homographyParameters);
		
//		rotationParameters.setName("rotation");
//		rotationParameters.add(pDoRotation.set("enable", true));
//		rotationParameters.add(pRotation.set("X Y Z", glm::vec3(0,0,0), glm::vec3(-90,-90,-90), glm::vec3(90,90,90)));
//		parameters.add(rotationParameters);
		
		updateFlip();
		updateHomography();
		
		return true;
	}
	
	//--------------------------------------------------------------
	void ParamCamExt::update() {
		ParamCam::update();
		
		if (ParamCam::isFrameNew()){
			int w = ParamCam::getWidth();
			int h = ParamCam::getHeight();
			int glInternalFormat = ofGetGLInternalFormatFromPixelFormat(getPixelFormat());
			if (glInternalFormat == GL_LUMINANCE) glInternalFormat = GL_RGB; // openGL 2.x
			
			int dstWidth = pRotate90? h: w;
			int dstHeight = pRotate90? w: h;
			
			if (flipFbo->getWidth() != dstWidth || flipFbo->getHeight() != dstHeight || flipFbo->getTexture().getTextureData().glInternalFormat != glInternalFormat) {
				flipFbo->clear();
				delete flipFbo;
				flipFbo = new ofFbo;
				flipFbo->allocate(dstWidth, dstHeight, glInternalFormat);
				if (ofIsGLProgrammableRenderer() && glInternalFormat == GL_R8) {
					flipFbo->getTexture().setSwizzle(GL_TEXTURE_SWIZZLE_G, GL_RED);
					flipFbo->getTexture().setSwizzle(GL_TEXTURE_SWIZZLE_B, GL_RED);
				}
				updateFlip();
				updateHomography();
			}
			
			flipFbo->begin();
			ofClear(0);
			Camera::getTexture().bind();
			if (pDoDistortion) {
				barrelShader.begin();
				int mW = getMaxWidth();
				int mH = getMaxHeight();
				float diameter = sqrt(mW * mW + mH * mH);
				float centerX = (mW / 2.0) - pROIX.get();
				float centerY = (mH / 2.0) - pROIY.get();
				barrelShader.setUniform1f("diameter", diameter);
				barrelShader.setUniform2f("center", centerX, centerY);
				barrelShader.setUniform1f("distortion_k1", pDistortionK1.get());
				barrelShader.setUniform1f("distortion_k2", pDistortionK2.get());
			}
			ofPushMatrix();
			if (pDoHomography) {
				ofMultMatrix(homography);
			}
//			if (pDoRotation.get()) {
//				ofTranslate(dstWidth * 0.5, dstHeight * 0.5);
//				ofPushMatrix();
//				ofRotateX(pRotation.get().x);
//				ofRotateY(pRotation.get().y);
//				ofRotateZ(pRotation.get().z);
//				ofTranslate(dstWidth * -0.5, dstHeight * -0.5);
//				flipQuad.draw();
//				ofPopMatrix();
//			}
			flipQuad.draw();
			ofPopMatrix();
			Camera::getTexture().unbind();
			barrelShader.end();
			flipFbo->end();
			
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
	void ParamCamExt::createBarrelShader() {
		string vertexShader, fragmentShader;
		
		if (ofIsGLProgrammableRenderer()) {
			vertexShader = GLSL_150(
									uniform mat4 modelViewProjectionMatrix;
									uniform mat4 textureMatrix;
									
									in vec4  position;
									in vec2  texcoord;
									
									out vec2 texCoordVarying;
									
									void main(){
										gl_Position = modelViewProjectionMatrix * position;
										texCoordVarying = (textureMatrix*vec4(texcoord.x,texcoord.y,0,1)).xy;
									}
									);
			
			fragmentShader = GLSL_150(
									  uniform sampler2DRect tex0;
									  
									  uniform float distortion_k1;
									  uniform float distortion_k2;
									  uniform float diameter;
									  uniform vec2  center;
									  
									  in vec2 texCoordVarying;
									  
									  out vec4 fragColor;
									  
									  vec2 brownConradyDistortion(vec2 uv)
									  {
										  // positive values of K1 give barrel distortion, negative give pincushion
										  float r2 = uv.x*uv.x + uv.y*uv.y;
										  uv *= 1.0 + distortion_k1 * r2 + distortion_k2 * r2 * r2;
										  
										  // tangential distortion (due to off center lens elements)
										  // is not modeled in this function, but if it was, the terms would go here
										  return uv;
									  }
									  
									  void main(){
										  vec2 uv = texCoordVarying.xy;
										  uv -= center;
										  uv /= diameter;
										  uv = brownConradyDistortion(uv);
										  uv *= diameter;
										  uv += center;
										  fragColor = texture( tex0,uv);
									  }
									  );
		}
		else {
			
			vertexShader = GLSL_120(
									void main() {
										gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;
										gl_TexCoord[0] = gl_MultiTexCoord0;
									}
									);
			
			
			fragmentShader = GLSL_120(
									  uniform sampler2DRect tex0;
									  
									  uniform float distortion_k1;
									  uniform float distortion_k2;
									  uniform float diameter;
									  uniform vec2  center;
									  
									  vec2 brownConradyDistortion(vec2 uv)
									  {
										  float r2 = uv.x*uv.x + uv.y*uv.y;
										  uv *= 1.0 + distortion_k1 * r2 + distortion_k2 * r2 * r2;
										  return uv;
									  }
									  
									  void main()
									  {
										  vec2 uv = gl_TexCoord[0].st;
										  uv -= center;
										  uv /= diameter;
										  uv = brownConradyDistortion(uv);
										  uv *= diameter;
										  uv += center;
										  gl_FragColor = texture2DRect(tex0, uv);
									  }
									  );
		}
		
		barrelShader.setupShaderFromSource(GL_VERTEX_SHADER, vertexShader);
		barrelShader.setupShaderFromSource(GL_FRAGMENT_SHADER, fragmentShader);
		barrelShader.bindDefaults();
		barrelShader.linkProgram();
	}
	
	//--------------------------------------------------------------
	void ParamCamExt::updateFlip() {
		int w = getWidth();
		int h = getHeight();
		
		vector<glm::vec2> pts;
		pts.assign(4, glm::vec2(0,0));
		
		if (!pRotate90) {	// NO ROTATION
			if (!pFlipH) {
				if (!pFlipV) {  // NO FLIP
					pts[0] = glm::vec2(0, 0);
					pts[1] = glm::vec2(w, 0);
					pts[2] = glm::vec2(w, h);
					pts[3] = glm::vec2(0, h);
				} else {          // FLIP V
					pts[0] = glm::vec2(0, h);
					pts[1] = glm::vec2(w, h);
					pts[2] = glm::vec2(w, 0);
					pts[3] = glm::vec2(0, 0);
				}
			}
			else {
				if (!pFlipV) {  // FLIP H
					pts[0] = glm::vec2(w, 0);
					pts[1] = glm::vec2(0, 0);
					pts[2] = glm::vec2(0, h);
					pts[3] = glm::vec2(w, h);
				} else {          // FLIP H & V
					pts[0] = glm::vec2(w, h);
					pts[1] = glm::vec2(0, h);
					pts[2] = glm::vec2(0, 0);
					pts[3] = glm::vec2(w, 0);
				}
			}
		}
		else {	// ROTATION
			if (!pFlipH) {
				if (!pFlipV) {  // NO FLIP
					pts[0] = glm::vec2(0, h);
					pts[1] = glm::vec2(0, 0);
					pts[2] = glm::vec2(w, 0);
					pts[3] = glm::vec2(w, h);
				} else {          // FLIP V
					pts[0] = glm::vec2(0, 0);
					pts[1] = glm::vec2(0, h);
					pts[2] = glm::vec2(w, h);
					pts[3] = glm::vec2(w, 0);
				}
			}
			else {
				if (!pFlipV) {  // FLIP H
					pts[0] = glm::vec2(0, h);
					pts[1] = glm::vec2(0, 0);
					pts[2] = glm::vec2(w, 0);
					pts[3] = glm::vec2(w, h);
				} else {          // FLIP H & V
					pts[0] = glm::vec2(0, h);
					pts[1] = glm::vec2(0, 0);
					pts[2] = glm::vec2(w, 0);
					pts[3] = glm::vec2(w, h);
				}
			}
		}
		
		flipQuad.setTexCoord(0, pts[0]);
		flipQuad.setTexCoord(1, pts[1]);
		flipQuad.setTexCoord(2, pts[2]);
		flipQuad.setTexCoord(3, pts[3]);
		
		int dstWidth = w;
		int dstHeight = h;
		
		if (pRotate90) {
			dstWidth = h;
			dstHeight = w;
		}
		flipQuad.setVertex(0, glm::vec3(0,0,0));
		flipQuad.setVertex(1, glm::vec3(dstWidth,0,0));
		flipQuad.setVertex(2, glm::vec3(dstWidth,dstHeight,0));
		flipQuad.setVertex(3, glm::vec3(0,dstHeight,0));
	}
	
	//--------------------------------------------------------------
	
	void ParamCamExt::updateHomography() {
		int w = getWidth();
		int h = getHeight();
		
		glm::vec2 srcCorners[4];
		srcCorners[0] = pHomographyPoints[0].get() * glm::vec2(w,h);
		srcCorners[1] = pHomographyPoints[1].get() * glm::vec2(w,h);
		srcCorners[2] = pHomographyPoints[2].get() * glm::vec2(w,h);
		srcCorners[3] = pHomographyPoints[3].get() * glm::vec2(w,h);
		
		glm::vec2 dstCorners[4];
		dstCorners[0] = glm::vec2(0,0);
		dstCorners[1] = glm::vec2(w,0);
		dstCorners[2] = glm::vec2(w,h);
		dstCorners[3] = glm::vec2(0,h);
		
		homography = findHomography(srcCorners, dstCorners);
	}
	
	ofMatrix4x4 ParamCamExt::findHomography(glm::vec2* src, glm::vec2* dst){
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
		if (pRotate90) {
			maxW = getROIHeightMax();
			maxH = getROIWidthMax();
		} else {
			maxW = getROIHeightMax();
			maxH = getROIWidthMax();
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
		
		
		if (pRotate90) {
			return ofRectangle(oY, oX, oH, oW);
		} else {
			return ofRectangle(oX, oY, oW, oH);
		}
	}
}


