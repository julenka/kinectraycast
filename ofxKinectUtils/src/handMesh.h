#pragma once
#include "SimpleKinect.h"
#include "ofMath.h"
#include <set>
#include <queue>
#include "skeletonMetrics.h"

struct ofVec4f_cmp
{
	bool operator()(const ofVec4f v1, const ofVec4f v2) const
	{
		return v1.w > v2.w;
	}
};

struct ofVec4f_cmp2
{
	bool operator()(const ofVec4f v1, const ofVec4f v2) const
	{
		return v1.w < v2.w;
	}
};

typedef shared_ptr<SimpleKinect> SimpleKinectPtr;
typedef set<ofVec4f, ofVec4f_cmp> ofVec4fSet;
typedef priority_queue<ofVec4f, vector<ofVec4f>,  ofVec4f_cmp> ofVec4fQueue;

class handMesh
{
public:
	handMesh(SimpleKinectPtr k);
	~handMesh();
	void											update();
	shared_ptr<ofVec4fSet>							handPoints(NUI_HANDEDNESS h);
	shared_ptr<ofVec4fSet>							fingerPoints(NUI_HANDEDNESS h); 

private:
	SimpleKinectPtr									m_pSimpleKinect;
	shared_ptr<ofVec4fQueue>						m_ppointQueue[NUI_HANDEDNESS_COUNT];
	shared_ptr<ofVec4fSet>							m_pfingerPoints[NUI_HANDEDNESS_COUNT];
	shared_ptr<ofVec4fSet>							m_phandPoints[NUI_HANDEDNESS_COUNT];

	void											updateForHand(NUI_HANDEDNESS h);
	bool*											m_visited[NUI_HANDEDNESS_COUNT];
};

