#include "handMesh.h"

handMesh::handMesh(SimpleKinectPtr k)
{
	m_pSimpleKinect = k;
	for(int i = 0; i < NUI_HANDEDNESS_COUNT; i++)
	{
		m_pfingerPoints[i] = shared_ptr<ofVec4fSet>(new ofVec4fSet());
		m_phandPoints[i] = shared_ptr<ofVec4fSet>(new ofVec4fSet());
		m_ppointQueue[i] = shared_ptr<ofVec4fQueue>(new ofVec4fQueue());
		m_visited[i] = new bool[SimpleKinect::cDepthHeight * SimpleKinect::cDepthWidth];
	}
}

handMesh::~handMesh()
{
	for(int i = 0; i < NUI_HANDEDNESS_COUNT; i++)
	{
		delete m_visited[i];
	}
}

// assumed m_pSimpleKinect.update() has been called
void handMesh::update()
{
	updateForHand(NUI_HANDEDNESS_RIGHT);
	updateForHand(NUI_HANDEDNESS_LEFT);
}

void handMesh::updateForHand(NUI_HANDEDNESS h)
{
	NUI_SKELETON_DATA* skel = m_pSimpleKinect->m_pActiveSkeleton;
	if(skel == NULL) return;
	ofVec4f hand = skeletonMetrics::fromVector4(skel->SkeletonPositions[ h == NUI_HANDEDNESS_RIGHT ? NUI_SKELETON_POSITION_HAND_RIGHT : NUI_SKELETON_POSITION_HAND_LEFT]);
	ofVec4f wrist = skeletonMetrics::fromVector4(skel->SkeletonPositions[ h == NUI_HANDEDNESS_RIGHT ? NUI_SKELETON_POSITION_WRIST_RIGHT : NUI_SKELETON_POSITION_WRIST_LEFT]);
	ofVec4f hw = wrist - hand;
	hand *= 1000;
	wrist *= 1000;
	m_phandPoints[h]->clear();
	

	// world to screen
	float depthX, depthY;
	NuiTransformSkeletonToDepthImage(
		skel->SkeletonPositions[ h == NUI_HANDEDNESS_RIGHT ? NUI_SKELETON_POSITION_WRIST_RIGHT : NUI_SKELETON_POSITION_WRIST_LEFT], 
		&depthX, 
		&depthY,
		NUI_IMAGE_RESOLUTION_640x480);
	if(!((int)depthY > 0 && (int)depthY < SimpleKinect::cDepthHeight &&
	(int)depthX > 0 && (int)depthX < SimpleKinect::cDepthWidth))
		return;

	Vector4* pWorld = m_pSimpleKinect->m_pCurrentWorld;
	
	m_phandPoints[h]->clear();
	m_pfingerPoints[h]->clear();
	int regionsize = 60;
	//LARGE_INTEGER t1, t2, f;
	//QueryPerformanceCounter(&t1);
	//QueryPerformanceFrequency(&f);
	ofVec4f cur;

	for(int y = depthY - regionsize; y < depthY + regionsize; y+=2){
		for(int x = depthX - regionsize; x < depthX + regionsize; x+=2){
			if(x < 0 || x >= SimpleKinect::cDepthWidth || y < 0 || y >= SimpleKinect::cDepthHeight) continue;

			Vector4& tmp = pWorld[(int)y * SimpleKinect::cDepthWidth + (int)x];
			cur.set(tmp.x, tmp.y, tmp.z, 1000);

			//p.w = 1000;
			//cout << "("<<hand.x << "," << hand.y << "," << hand.z << ","<<hand.w<<")"<<":("<< p.x<<","<<p.y<<","<<p.z<<","<<p.w<<")" << hand.distance(p) << endl;

			if(hand.distance(cur) < 100)
			{
				ofVec4f ph = cur - hand;
				cur.w = ph.dot(hw);
				m_ppointQueue[h]->push(cur);
				//cout <<"("<< p.x<<","<<p.y<<","<<p.z<<","<<p.w<<")"<<endl;
			}
		}
	}
	//QueryPerformanceCounter(&t2);
	//cout << "dif1: " << (t2.QuadPart - t1.QuadPart) / (double)f.QuadPart;

	int count = 0;
	while(!m_ppointQueue[h]->empty())
	{
			ofVec4f t = m_ppointQueue[h]->top();

			if(count < 5){
				m_pfingerPoints[h]->insert(t);
			}
			//m_phandPoints[h]->insert(t);
			m_ppointQueue[h]->pop();
			count++;
	}
	//QueryPerformanceCounter(&t1);
	//cout << "dif2: " <<(t1.QuadPart - t2.QuadPart) / (double)f.QuadPart;
	//cout<<"priority queue size: " << count <<endl;
}

shared_ptr<ofVec4fSet> handMesh::handPoints(NUI_HANDEDNESS h)
{
	return m_phandPoints[h];
}

shared_ptr<ofVec4fSet> handMesh::fingerPoints(NUI_HANDEDNESS h)
{
	return m_pfingerPoints[h];
}