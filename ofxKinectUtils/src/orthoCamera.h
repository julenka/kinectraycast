#pragma once
#include "SimpleKinect.h"
#include "ofCamera.h"
#include "ofMath.h"
#include "ofUtils.h"
#include "ofEvents.h"

#define ROTATION_DAMPENING 0.3
#define ORTHO_CAM_DBLCLICKTIME 500

class orthoCamera :public ofCamera
{
public:
	
	orthoCamera(void);
	~orthoCamera(void);

	virtual void begin(ofRectangle viewport = ofGetCurrentViewport());
	virtual void end();

	void mouseDragged(ofMouseEventArgs& mouse);
	void mouseMoved(ofMouseEventArgs& mouse);
	void mousePressed(ofMouseEventArgs& mouse);
	void mouseReleased(ofMouseEventArgs& mouse);

	void keyPressed(ofKeyEventArgs& k);
	void keyReleased(ofKeyEventArgs& k);

	void reset();
private:
	ofVec2f rotation;
	ofVec2f lastRotation;
	
	float	zoom;
	float lastZoom;

	ofVec2f translation;
	ofVec2f lastTranslation;

	ofVec2f mouseStart;

	ULONG lastTap;

	bool translating;
};

