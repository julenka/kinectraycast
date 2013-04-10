#pragma once
#include "kinectView.h"

kinectView::kinectView(void)
{
}


kinectView::~kinectView(void)
{
}


void kinectView::customDraw()
{
	ofPushStyle();
	ofSetColor(255, 100, 100);
	
	USHORT maxDepth = NuiDepthPixelToDepth(NUI_IMAGE_DEPTH_MAXIMUM);
	USHORT minDepth = NuiDepthPixelToDepth(NUI_IMAGE_DEPTH_MINIMUM);

	// get far plane
	// draw rectangle at far plane
	// max depth: maxDepth
	float verticalFovRad = NUI_CAMERA_DEPTH_NOMINAL_VERTICAL_FOV * 2 * PI / 360;
	float horizFovRad = NUI_CAMERA_DEPTH_NOMINAL_HORIZONTAL_FOV * 2 * PI / 360;
	float halfBackHeight = atan( verticalFovRad) * maxDepth;
	float halfBackWidth = atan( horizFovRad) * maxDepth;
	
	float halfFrontHeight = atan( verticalFovRad) * minDepth;
	float halfFrontWidth =  atan( horizFovRad) * minDepth;

	ofPoint origin = ofPoint();
	ofPoint leftTop = ofPoint(-halfBackWidth, halfBackHeight, maxDepth);
	ofPoint rightTop = ofPoint(halfBackWidth, halfBackHeight, maxDepth);
	ofPoint leftBottom = ofPoint(-halfBackWidth, -halfBackHeight, maxDepth);
	ofPoint rightBottom = ofPoint(halfBackWidth, -halfBackHeight, maxDepth);

	ofLine(origin, leftTop);
	ofLine(origin, rightTop);
	ofLine(origin, leftBottom);
	ofLine(origin, rightBottom);

	ofLine(leftTop, rightTop);
	ofLine(rightTop, rightBottom);
	ofLine(rightBottom, leftBottom);
	ofLine(leftBottom, leftTop);

	ofPoint frontleftTop = ofPoint(-halfFrontWidth, halfFrontHeight, minDepth);
	ofPoint frontrightTop = ofPoint(halfFrontWidth, halfFrontHeight, minDepth);
	ofPoint frontleftBottom = ofPoint(-halfFrontWidth, -halfFrontHeight, minDepth);
	ofPoint frontrightBottom = ofPoint(halfFrontWidth, -halfFrontHeight, minDepth);

	ofLine(frontleftTop, frontrightTop);
	ofLine(frontrightTop, frontrightBottom);
	ofLine(frontrightBottom, frontleftBottom);
	ofLine(frontleftBottom, frontleftTop);

	ofPopStyle();
}