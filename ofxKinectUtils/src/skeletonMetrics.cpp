#include "skeletonMetrics.h"

skeletonMetrics::skeletonMetrics(shared_ptr<SimpleKinect> k)
{
	m_pSimpleKinect = k;
	m_smoothFactor = 0.5f;
	m_headDistance = 0;
	for(int i = 0; i < NUI_HANDEDNESS_COUNT; i++)
	{
		m_armLength[i] = 0;
		m_shoulderWristLength[i]=0;
	}
}

skeletonMetrics::~skeletonMetrics(void)
{
}

void skeletonMetrics::update()
{
	NUI_SKELETON_DATA* skel = m_pSimpleKinect->m_pActiveSkeleton;
	if(skel == NULL) return;

	
	
	ofVec4f shoulderR = fromVector4(skel->SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_RIGHT]);
	ofVec4f elbowR = fromVector4(skel->SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_RIGHT]);
	ofVec4f handR = fromVector4(skel->SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT]);
	ofVec4f shoulderL = fromVector4(skel->SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_LEFT]);
	ofVec4f elbowL = fromVector4(skel->SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_LEFT]);
	ofVec4f handL = fromVector4(skel->SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT]);
	ofVec4f head = fromVector4(skel->SkeletonPositions[NUI_SKELETON_POSITION_HEAD]);

	float armLengthR = (shoulderR - elbowR).length() + (elbowR - handR).length();
	float armLengthL = (shoulderL - elbowL).length() + (elbowL - handL).length();
	float swLengthR = (shoulderR - handR).length();
	float swLengthL = (shoulderL - handL).length();

	// update shoulder wrist length
	m_armLength[NUI_HANDEDNESS_RIGHT] = m_armLength[NUI_HANDEDNESS_RIGHT] * m_smoothFactor + armLengthR * (1 - m_smoothFactor);
	m_armLength[NUI_HANDEDNESS_LEFT] = m_armLength[NUI_HANDEDNESS_LEFT] * m_smoothFactor + armLengthL * (1 - m_smoothFactor);

	m_shoulderWristLength[NUI_HANDEDNESS_RIGHT] = m_shoulderWristLength[NUI_HANDEDNESS_RIGHT] * m_smoothFactor + swLengthR * (1 - m_smoothFactor);
	m_shoulderWristLength[NUI_HANDEDNESS_LEFT] = m_shoulderWristLength[NUI_HANDEDNESS_LEFT] * m_smoothFactor + swLengthL * (1 - m_smoothFactor);

	m_headDistance = m_headDistance * m_smoothFactor + head.z * (1 - m_smoothFactor);
}

float skeletonMetrics::armLength(NUI_HANDEDNESS h)
{
	return m_armLength[h];
}

float skeletonMetrics::shoulderToWristLength(NUI_HANDEDNESS h)
{
	return m_shoulderWristLength[h];
}

float skeletonMetrics::armExtension(NUI_HANDEDNESS h)
{
	if(m_armLength[h] == 0) return 0;

	return min(1, m_shoulderWristLength[h] / m_armLength[h]);
}

float skeletonMetrics::headDistance()
{
	return m_headDistance;
}
