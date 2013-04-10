#pragma once
#include "SimpleKinect.h"
#include "ofnode.h"

class kinectView :
	public ofNode
{
public:
	kinectView(void);
	~kinectView(void);

	void customDraw();
};

