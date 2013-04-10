#pragma once
#include "orthoCamera.h"

//----------------------------------------
orthoCamera::orthoCamera(){
	enableOrtho();
	ofRegisterMouseEvents(this);
	ofRegisterKeyEvents(this);
	reset();
}

//----------------------------------------
orthoCamera::~orthoCamera(){

}

//----------------------------------------
void orthoCamera::begin(ofRectangle viewport){
	ofPushMatrix();
	ofTranslate(ofGetWidth() - 10 + translation.x, ofGetHeight() - 10 + translation.y, zoom);
	ofRotate(-rotation.x, 0, 1, 0);
	ofRotate(rotation.y, 1, 0, 0);
	ofScale(1, -1, -1); // flip the y axis 
}


void orthoCamera::end()
{
	ofPopMatrix();
}

void orthoCamera::reset()
{
	rotation = ofVec2f();
	lastRotation = ofVec2f();
	zoom = 0;
	lastZoom = 0;
	translating = false;
	translation = ofVec2f();
	lastTranslation = ofVec2f();
}

//----------------------------------------
void orthoCamera::mouseDragged(ofMouseEventArgs& mouse)
{
	float curX = ofGetMouseX();
	float curY = ofGetMouseY();

	if(mouse.button == 0)
	{
		// left
		rotation.x = lastRotation.x + (curX - mouseStart.x)  * ROTATION_DAMPENING;
		rotation.y = lastRotation.y + (curY- mouseStart.y)  * ROTATION_DAMPENING ;
	} else if (mouse.button == 2)
	{
		translation.x = lastTranslation.x + (curX - mouseStart.x) * 10;
		translation.y = lastTranslation.y + (curY - mouseStart.y) * 10;
	} else if (mouse.button == 1)
	{
		// right
		zoom = lastZoom + (curY - mouseStart.y) * 10;
	}
}

//----------------------------------------
void orthoCamera::mouseMoved(ofMouseEventArgs& mouse)
{
	
}

//----------------------------------------
void orthoCamera::mousePressed(ofMouseEventArgs& mouse){
	mouseStart.x = ofGetMouseX();
	mouseStart.y = ofGetMouseY();

	unsigned long curTap = ofGetElapsedTimeMillis();
	if(lastTap != 0 && curTap - lastTap < ORTHO_CAM_DBLCLICKTIME){
		reset();
	}
	lastTap = curTap;
}

//----------------------------------------
void orthoCamera::mouseReleased(ofMouseEventArgs& mouse)
{
		lastZoom = zoom;
		lastRotation = rotation;
		lastTranslation = translation;

}

void orthoCamera::keyPressed(ofKeyEventArgs& k)
{
}

void orthoCamera::keyReleased(ofKeyEventArgs& k)
{
	translating = false;
}
