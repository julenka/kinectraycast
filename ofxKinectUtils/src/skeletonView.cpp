#include "skeletonView.h"


skeletonView::skeletonView(shared_ptr<SimpleKinect> k)
{
	m_pSimpleKinect = k;
	m_updateCalled = false;
}


skeletonView::~skeletonView(void)
{
}

void skeletonView::update()
{
	m_updateCalled = true;

	// update the skeleton if it hasn't been updated
	m_pSimpleKinect->UpdateSkeleton();
}

void skeletonView::customDraw()
{
	// check if update has been called
	if(!m_updateCalled) return;
	bool first = true;
	ofPushStyle();

	NUI_SKELETON_FRAME skel = m_pSimpleKinect->m_curSkelFrame;
	// starter: draw the head and the hand
	for ( int i = 0 ; i < NUI_SKELETON_COUNT; i++ )
    {
		NUI_SKELETON_TRACKING_STATE trackingState = skel.SkeletonData[i].eTrackingState;

        if ( trackingState == NUI_SKELETON_TRACKED )
        {
            // We're tracking the skeleton, draw it
			ofPushStyle();
			
			if(first) ofSetColor(255, 0, 255, 255);
			else ofSetColor(200);

			ofSetLineWidth(3);
			drawBones(skel.SkeletonData[i]);
			ofPopStyle();

			// drawJoints(skel.SkeletonData[i]);


			first = false;
        }
    }
	// draw all of the joints
	// draw all of hte bones

	ofPopStyle();
	m_updateCalled = false;
}

	void skeletonView::drawBones(const NUI_SKELETON_DATA & skel)
	{

		 // Render Torso
		drawBone( skel, NUI_SKELETON_POSITION_HEAD, NUI_SKELETON_POSITION_SHOULDER_CENTER );
		drawBone( skel, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_LEFT );
		drawBone( skel, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_RIGHT );
		drawBone( skel, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SPINE );
		drawBone( skel, NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_HIP_CENTER );
		drawBone( skel, NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_LEFT );
		drawBone( skel, NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_RIGHT );

		// Left Arm
		drawBone( skel, NUI_SKELETON_POSITION_SHOULDER_LEFT, NUI_SKELETON_POSITION_ELBOW_LEFT );
		drawBone( skel, NUI_SKELETON_POSITION_ELBOW_LEFT, NUI_SKELETON_POSITION_WRIST_LEFT );
		drawBone( skel, NUI_SKELETON_POSITION_WRIST_LEFT, NUI_SKELETON_POSITION_HAND_LEFT );

		// Right Arm
		drawBone( skel, NUI_SKELETON_POSITION_SHOULDER_RIGHT, NUI_SKELETON_POSITION_ELBOW_RIGHT );
		drawBone( skel, NUI_SKELETON_POSITION_ELBOW_RIGHT, NUI_SKELETON_POSITION_WRIST_RIGHT );
		drawBone( skel, NUI_SKELETON_POSITION_WRIST_RIGHT, NUI_SKELETON_POSITION_HAND_RIGHT );

		// Left Leg
		drawBone( skel, NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_KNEE_LEFT );
		drawBone( skel, NUI_SKELETON_POSITION_KNEE_LEFT, NUI_SKELETON_POSITION_ANKLE_LEFT );
		drawBone( skel, NUI_SKELETON_POSITION_ANKLE_LEFT, NUI_SKELETON_POSITION_FOOT_LEFT );

		// Right Leg
		drawBone( skel, NUI_SKELETON_POSITION_HIP_RIGHT, NUI_SKELETON_POSITION_KNEE_RIGHT );
		drawBone( skel, NUI_SKELETON_POSITION_KNEE_RIGHT, NUI_SKELETON_POSITION_ANKLE_RIGHT );
		drawBone( skel, NUI_SKELETON_POSITION_ANKLE_RIGHT, NUI_SKELETON_POSITION_FOOT_RIGHT );
		
		
	}

	ofPoint skeletonView::getJointPoint(const NUI_SKELETON_DATA & skel, NUI_SKELETON_POSITION_INDEX i)
	{
		ofPoint jointPoint;
		Vector4 jointPos;
		jointPos = skel.SkeletonPositions[i];
		
		jointPoint.x = jointPos.x / jointPos.w * 1000;
		jointPoint.y = jointPos.y / jointPos.w * 1000;
		jointPoint.z = jointPos.z / jointPos.w * 1000;
		return jointPoint;
	}

	void skeletonView::drawBone(const NUI_SKELETON_DATA & skel, NUI_SKELETON_POSITION_INDEX bone0, NUI_SKELETON_POSITION_INDEX bone1 )
	{



		// do nothing for now.
		NUI_SKELETON_POSITION_TRACKING_STATE bone0State = skel.eSkeletonPositionTrackingState[bone0];
		NUI_SKELETON_POSITION_TRACKING_STATE bone1State = skel.eSkeletonPositionTrackingState[bone1];

		// If we can't find either of these joints, exit
		if ( bone0State == NUI_SKELETON_POSITION_NOT_TRACKED || bone1State == NUI_SKELETON_POSITION_NOT_TRACKED )
		{
			return;
		}
    
		// Don't draw if both points are inferred
		if ( bone0State == NUI_SKELETON_POSITION_INFERRED && bone1State == NUI_SKELETON_POSITION_INFERRED )
		{
			return;
		}
		ofPoint p0 = getJointPoint(skel, bone0);
		ofPoint p1 = getJointPoint(skel, bone1);
		ofLine(p0, p1);

	}
	
	void skeletonView::drawJoints(const NUI_SKELETON_DATA & skel)
	{    
	ofPoint jointPoint;
	Vector4 jointPos;
	// Draw the joints in a different color
    for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; i++ )
    {
        ofPoint jointPoint = getJointPoint(skel, (NUI_SKELETON_POSITION_INDEX)i);

		if(skel.eSkeletonPositionTrackingState[i] == NUI_SKELETON_POSITION_INFERRED)
		{
			// draw gray sphere
			ofColor(100);
			ofSphere(jointPoint, 30);
		} 
		else if (skel.eSkeletonPositionTrackingState[i] == NUI_SKELETON_POSITION_TRACKED)
		{
			// draw green sphere
			ofColor(0,255,0,255);
			ofSphere(jointPoint, 10);
			
		}
    }

	}