#pragma once
#include "SimpleKinect.h"
#include "ofNode.h"
#include "assert.h"
#include <memory.h>

class skeletonView :
	public ofNode
{
public:
	skeletonView(shared_ptr<SimpleKinect> k);
	~skeletonView(void);

	// Update must be called on skeletonView before draw is called
	void update();

	// to do: assert that update has been called
	void customDraw();

private:
	shared_ptr<SimpleKinect>	m_pSimpleKinect;
	bool			m_updateCalled;

	void			drawBones(const NUI_SKELETON_DATA & skel);
	void			drawJoints(const NUI_SKELETON_DATA & skel);
	void			drawBone(const NUI_SKELETON_DATA & skel, NUI_SKELETON_POSITION_INDEX bone0, NUI_SKELETON_POSITION_INDEX bone1 );
	ofPoint			getJointPoint(const NUI_SKELETON_DATA & skel, NUI_SKELETON_POSITION_INDEX i);
};

