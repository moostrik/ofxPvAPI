//
//  TexPC.cpp
//  plCam
//
//  Created by PLPLPL.PL on 08/01/16.
//
//

#include "TexPC.h"


namespace ofxProsilica {
	
	bool TexPC::setup(){
		ParameterConnector::setup();
		
		flipParameters.setName("flip");
		flipParameters.add(flipH.set("flip H", false));
		flipParameters.add(flipV.set("flip V", false));
		flipParameters.add(rotate90.set("rotate 90", false));
		
		parameters.add(flipParameters);
		
		ofImage tp("tp.jpg");
		
		//	if (getPixelFormat() == OF_PIXELS_MONO) {
		//		texture.allocate(tp.getWidth(), tp.getHeight(), GL_R8);
		//		flipFbo.allocate(tp.getWidth(), tp.getHeight(), GL_R8);
		//	}
		//	else {
		//		texture.allocate(tp.getWidth(), tp.getHeight(), GL_RGB);
		//		flipFbo.allocate(tp.getWidth(), tp.getHeight(), GL_RGB);
		//	}
		
		
		//	texture.allocate(tp.getWidth(), tp.getHeight(), GL_RGB);
		texture.loadData(tp.getPixels());
		flipFbo.allocate(tp.getWidth(), tp.getHeight(), GL_RGB);
		
		flipFbo.begin();
		ofClear(0);
		texture.draw(0, 0);
		flipFbo.end();
		
		return true;
	}
	
	//--------------------------------------------------------------
	void TexPC::update() {
		ParameterConnector::update();
		
		if (isFrameNew()){
			int w = getWidth();
			int h = getHeight();
			int glFormat = ofGetGLInternalFormatFromPixelFormat(getPixelFormat());
			
			if (texture.getWidth() != w || texture.getHeight() != h || texture.getTextureData().glInternalFormat != glFormat) {
				texture.clear();
				texture.allocate(w, h, glFormat);
			}
			
			texture.loadData(getPixels());
			
			if (rotate90 || flipH || flipV) {
				hasFlip = true;
			}
			else {
				hasFlip = false;
				return;
			}
			
			
			if (rotate90) {
				int t = w;
				w = h;
				h = t;
			}
			
			if (flipFbo.getWidth() != w || flipFbo.getHeight() != h) {
					flipFbo.allocate(w, h, GL_RGB);
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
						pts[0].set(w, 0);
						pts[1].set(w, h);
						pts[2].set(0, h);
						pts[3].set(0, 0);
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
						pts[0].set(0, h);
						pts[1].set(0, 0);
						pts[2].set(w, 0);
						pts[3].set(w, h);
					}
				}
			}
			
			flipFbo.begin();
			ofClear(0);
			texture.draw(pts[0], pts[1], pts[2], pts[3]);
			flipFbo.end();
		}
	}
}

