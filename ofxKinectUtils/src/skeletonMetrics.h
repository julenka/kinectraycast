#pragma once

#include "SimpleKinect.h"
#include <memory.h>
#include "ofMath.h"


// provides metrics for the currently active skeleton (as determined by SimpleKinect)
class skeletonMetrics
{
public:
	

	skeletonMetrics(shared_ptr<SimpleKinect> k);
	~skeletonMetrics(void);

	// Update must be called before getting any metrics on the skeleton
	void update();

	float distanceFromHead();

	// returns the total length of the arm: length of upper arm + 
	// length of forearm. Value is smoothed
	// units in meters
	float armLength(NUI_HANDEDNESS h);

	// returns distance from shoulder to wrist. Value is smoothed
	// units in meters
	float shoulderToWristLength(NUI_HANDEDNESS h);

	// returns amount of arm extension. 0 is no extension, 1 is maximum extension.
	// compares distance of shoulder to wrist to total arm length
	float armExtension(NUI_HANDEDNESS h);

	float headDistance();

	static ofVec4f fromVector4(Vector4 v) 
	{
		return ofVec4f(v.x, v.y, v.z, v.w);
	}
private:
	shared_ptr<SimpleKinect>	m_pSimpleKinect;
	float						m_armLength[NUI_HANDEDNESS_COUNT];
	float						m_shoulderWristLength[NUI_HANDEDNESS_COUNT];
	float						m_headDistance;
	float						m_smoothFactor;

	
};