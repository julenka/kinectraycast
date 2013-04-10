#pragma once
// We are going to use sockets api, define here os it isn't defined in Windows.h
#define _WINSOCKAPI_
#include <Windows.h>

// from stdafx.h
#include <stdio.h>
#include <tchar.h>

#include "ofxNetwork.h"
#include "ofMain.h"
#include "ofAppGlutWindow.h"

#include "SimpleKinect.h"

#include "kinectView.h"
#include "orthoCamera.h"
#include "skeletonMetrics.h"
#include "skeletonView.h"
#include "handMesh.h"

#define RAYCAST_CONFIG "data/screenconfig.txt"
#define	DATA_ROOT "data/"

class rayCast : public ofBaseApp{

	static const int		cNumPlayers = 7;
	static const int		cMaxNumPixelsToTrack = 1000;
	static const int		cSkip = 4;

public:
	
	rayCast();
    /// <summary>
    /// Destructor
    /// </summary>
    ~rayCast();

	void setup();
	void update();
	void draw();
		
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	
private:
	shared_ptr<SimpleKinect>	m_pSimpleKinect;
	ofVec4f						m_rayIntersection[NUI_HANDEDNESS_COUNT];
    
    void                    UpdatePointMeshShowDepth();
	// Updates the number of pixels per player and the 
	// locations of each player pixel. Traversed row-wise
	// in depth image
	void					UpdatePlayerPixels();
	// Sorts the player pixels by z order, also updates
	// m_minDepthPerPlayer, m_maxDepthPerPlayer
	void					GetPixelsInZOrder();

	void					DrawHands();
	void					DrawFingers();
	
	// Raycasting
	ofVec4f					IntersectWithScreen(ofVec4f p1, ofVec4f p2);
	void					DrawRay(NUI_HANDEDNESS handedness, ofColor color);
	void					UpdateRay();
	void					UpdatePoint(NUI_HANDEDNESS handedness);

	// Networking
	void					SendRayData();
	int						GetIp(char* ip_out);

	// Screen calibration
	void					DrawScreen();
	void					SaveCalibration();
	void					LoadCalibration();

	// Debug
	void					DrawDebugInfo();

	ofVec4f					GetHandPoint(NUI_HANDEDNESS h, NUI_SKELETON_DATA* skel);
	ofVec4f					m_smoothedFingerPoints[NUI_HANDEDNESS_COUNT];
	float					m_smoothingFactor;

	// members
	orthoCamera				m_cam;

	ofColor					m_playerColors[cNumPlayers];

	int						m_activePlayer;

	
	kinectView				m_kinectView;
	skeletonView			m_skeletonView;
	skeletonMetrics			m_skeletonMetrics;
	handMesh				m_handMesh;
	ofVec3f					m_screenPoints[4];
	int						m_calibrateIndex;
	bool					m_calibrationMode;
	ofPoint					m_screenPos[NUI_HANDEDNESS_COUNT];

	ofxUDPManager			m_udpConnection;
	stringstream			m_debugStream;

	bool					m_useMeshForHand;

	// visualization settings
	bool					m_showHand;
	bool					m_showFingers;
	bool					m_showDepth;
	bool					m_showRay;

	// visualization
	ofMesh					m_mesh;
	ofMesh					m_screenMesh;
	ofMesh					m_handViz[NUI_HANDEDNESS_COUNT];
	ofMesh					m_fingerViz[NUI_HANDEDNESS_COUNT];
};


rayCast::rayCast()
	: m_pSimpleKinect(new SimpleKinect()) , 
	m_skeletonView(m_pSimpleKinect),
	m_handMesh(m_pSimpleKinect),
	m_calibrationMode(false),
	m_skeletonMetrics(m_pSimpleKinect),
	m_showHand(true),
	m_showDepth(false),
	m_showFingers(true),
	m_showRay(true),
	m_useMeshForHand(true)
	
{
	m_calibrateIndex = 0;
	m_smoothingFactor = 0.8f;
}

rayCast::~rayCast(){};

void rayCast::UpdatePointMeshShowDepth()
{
	USHORT* pBufferRun = m_pSimpleKinect->m_pCurrentDepth;
	Vector4* pBufferWorld = m_pSimpleKinect->m_pCurrentWorld;
	HRESULT hr;
    // end pixel is start + width*height - 1
    const USHORT  * pBufferEnd = pBufferRun + (m_pSimpleKinect->cDepthWidth * m_pSimpleKinect->cDepthHeight);
	int x = 0,  y = 0, i = 0;
	LONG xrgb = 0, yrgb = 0;
	BYTE* pColor;
	m_mesh.clear();
	while ( pBufferRun < pBufferEnd )
    {
		USHORT player = NuiDepthPixelToPlayerIndex(*pBufferRun);
		Vector4 world = *pBufferWorld;
		ofVec3f pos(world.x, world.y , world.z );
		float minZ = m_pSimpleKinect->m_minDepthPerPlayer[player];
		float maxZ = m_pSimpleKinect->m_maxDepthPerPlayer[player];
		
		float lightness = 1 - (world.z - minZ) / (maxZ - minZ);

		ofColor baseColor = m_playerColors[player]; 
		// set alpha based on minimum, maximum player id
		if(pos.z != 0)
		{
			m_mesh.addVertex(pos);
			m_mesh.addColor(ofColor(baseColor.r * lightness, baseColor.g * lightness, baseColor.b * lightness));
		}
		// add a particle at the x, y location

		// update indices
		x+= cSkip;
		if(x >= m_pSimpleKinect->cDepthWidth){
			x = 0;
			y+= cSkip;
			pBufferRun += cSkip * m_pSimpleKinect->cDepthWidth;
			pBufferWorld += cSkip * m_pSimpleKinect->cDepthWidth ;
		} else 
		{
			pBufferRun += cSkip;
			pBufferWorld += cSkip;
		}
		i++;
    }

}

ofVec4f	rayCast::GetHandPoint(NUI_HANDEDNESS h, NUI_SKELETON_DATA* skel)
{
	if(m_useMeshForHand)
	{
		ofVec4f tmp = ofVec4f();
		shared_ptr<ofVec4fSet> fingerPts = m_handMesh.fingerPoints(h);
		float count = 0;
		for(ofVec4fSet::iterator it = fingerPts->begin(); it != fingerPts->end() && count < 2; it++)
		{
			ofVec4f v = *it;
			tmp += v;
			count++;
		}
		tmp = tmp / count;
		m_smoothedFingerPoints[h] = m_smoothedFingerPoints[h] * m_smoothingFactor + tmp * (1 - m_smoothingFactor);
		return m_smoothedFingerPoints[h];

	} else
	{
		ofVec4f result = skeletonMetrics::fromVector4(skel->SkeletonPositions[h == NUI_HANDEDNESS_RIGHT ? NUI_SKELETON_POSITION_WRIST_RIGHT : NUI_SKELETON_POSITION_WRIST_LEFT]);
		result *= 1000;
		return result;
	}
	
}

void rayCast::UpdateRay()
{
	NUI_SKELETON_DATA* skel = m_pSimpleKinect->m_pActiveSkeleton;
	if(skel == NULL) return;	
	// get equation for line
	ofVec4f head = skeletonMetrics::fromVector4(skel->SkeletonPositions[NUI_SKELETON_POSITION_HEAD]);
	ofVec4f handRight = GetHandPoint(NUI_HANDEDNESS_RIGHT, skel);
	ofVec4f handLeft = GetHandPoint(NUI_HANDEDNESS_LEFT, skel);

	// I do not remember why I decided to convert all of my units in mm...
	head = head * 1000;

	bool updateRight = true;
	bool updateLeft = true;
	if(head.x == 0 && head.y == 0 && head.z == 0) return;
	if(handRight.x == 0 && handRight.y == 0 && handRight.z == 0) updateRight = false;
	if(handLeft.x == 0 && handLeft.y == 0 && handLeft.z == 0) updateLeft = false;

	if(updateRight) m_rayIntersection[NUI_HANDEDNESS_RIGHT] = IntersectWithScreen(head, handRight);
	if(updateLeft) m_rayIntersection[NUI_HANDEDNESS_LEFT] = IntersectWithScreen(head, handLeft);
}

// Intersects line defined by p1, p2 wiht the xy plane where z = 0
// p1 and p2 must be different
ofVec4f rayCast::IntersectWithScreen(ofVec4f p1, ofVec4f p2)
{
	// compute equation for line
	ofVec4f dir = p1 - p2;
	
	// head p1
	// hand p2
	
	float a = (0 - p1.z) / dir.z;
	ofVec4f result = p1 + dir * a;
	result.w  = 1;	
	return result;
}

void rayCast::UpdatePoint(NUI_HANDEDNESS handedness)
{
	bool invalid = false;
	for(int i = 0; i < 4; i++)
	{
		if(m_screenPoints[i].length() == 0)
		{
			invalid = true;
		}
	}
	if(invalid) return;

	// valid
	float maxx = max(m_screenPoints[1].x, m_screenPoints[2].x);
	float maxy = max(m_screenPoints[0].y, m_screenPoints[1].y);
	float minx = min(m_screenPoints[0].x, m_screenPoints[3].x);
	float miny = min(m_screenPoints[2].y, m_screenPoints[3].y);

	float relx = m_rayIntersection[handedness].x - minx;
	float rely = maxy - m_rayIntersection[handedness].y;

	float xnorm = relx / (maxx - minx);
	float ynorm = rely / (maxy - miny);


	m_screenPos[handedness].x = xnorm ;
	m_screenPos[handedness].y = ynorm;
}

void rayCast::DrawRay(NUI_HANDEDNESS handedness, ofColor c)
{
	NUI_SKELETON_DATA* skel = m_pSimpleKinect->m_pActiveSkeleton;
	if(skel == NULL) return;	
	// get equation for line
	ofVec4f head = skeletonMetrics::fromVector4(skel->SkeletonPositions[NUI_SKELETON_POSITION_HEAD]);
	
	head = head * 1000;

	if(head.x == 0 && head.y == 0 && head.z == 0) return;

	ofPushStyle();
	ofSetLineWidth(3);
	ofSetColor(c);
	ofVec4f v = m_rayIntersection[handedness];
	ofPoint intersect = ofPoint(
		v.x, 
		v.y, 
		v.z);
	ofLine(intersect, ofPoint(head.x, head.y, head.z));
	ofPopStyle();
	// p1 = line at z = 1000
}

void rayCast::DrawScreen()
{
	for(int i = 0; i < 4; i++)
	{
		m_screenMesh.setVertex(i, m_screenPoints[i]);
	}
	ofPushStyle();
	ofSetLineWidth(10);
	m_screenMesh.draw();
	ofPopStyle();
}

// Thank you http://tangentsoft.net/wskfaq/examples/ipaddr.html
int rayCast::GetIp(char* ip_out)
{
    char ac[80];
    if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR) {
        cerr << "GetIp " << WSAGetLastError() <<
                " when getting local host name." << endl;
        return -1;
    }
    cout	 << "Host name is " << ac << "." << endl;

    struct hostent *phe = gethostbyname(ac);
    if (phe == 0) {
        cerr << "GetIp: Bad host lookup." << endl;
        return -1;
    }

	// assume were are using the first address
    for (int i = 0; phe->h_addr_list[i] != 0; ++i) {
        struct in_addr addr;
        memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
		strcpy_s(ip_out, 16, inet_ntoa(addr));
        cout << "Address " << i << ": " << ip_out << endl;
		// return the first address
		return 0;
    }
    
	cerr << "GetIp: no ip address found" << endl;
	return -1;
}

//--------------------------------------------------------------
void rayCast::setup(){
	// set up player colors
	
	ofColor playerColors[] = {
		ofColor(255, 255,255),
		ofColor(255, 0,0),
		ofColor(0, 255,0),
		ofColor(0, 0,255),
		ofColor(255, 255,0),
		ofColor(255, 0,255),
		ofColor(0, 255,255),
		ofColor(255, 255,255)};
	
	for (int i = 0; i < m_pSimpleKinect->cNumPlayers; i++)
	{
		m_playerColors[i] = playerColors[i];
	}
	
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POINT_SMOOTH); // use circular points instead of square points
	glPointSize(2); // make the points bigger
	glLineWidth(20);
	
	// set up depth mesh
	// we're going to load a ton of points into an ofm_mesh
	m_mesh.setMode(OF_PRIMITIVE_POINTS);
	ofColor cur(0,0,0,0);

	for(int i = 0; i < NUI_HANDEDNESS_COUNT; i++)
	{
		m_handViz[i]=ofMesh();
		m_handViz[i].setMode(OF_PRIMITIVE_POINTS);
		m_fingerViz[i]=ofMesh();
		m_fingerViz[i].setMode(OF_PRIMITIVE_POINTS);
	}

	// set up screen mesh
	m_screenMesh.setMode(OF_PRIMITIVE_LINE_LOOP);
	for(int i = 0; i < 4; i++)
	{
		m_screenMesh.addColor(ofColor(0, 255, 0));
		m_screenMesh.addVertex(ofVec3f());
	}

	m_pSimpleKinect->Initialize();
	LoadCalibration();

	char ip[20];
	HRESULT hr = GetIp(ip);

	if(SUCCEEDED(hr)){
		// set up udp!
		m_udpConnection.Create();
		m_udpConnection.Connect(ip,11999);
		m_udpConnection.SetNonBlocking(true);
	}

}

//--------------------------------------------------------------
void rayCast::update(){
	if(!m_pSimpleKinect->ReadyForUpdate())
	{
		return;
	}
    
	m_pSimpleKinect->UpdateDepth();
	m_pSimpleKinect->UpdateWorld();
	m_skeletonView.update();
	m_skeletonMetrics.update();
	m_handMesh.update();
	UpdateRay();
	UpdatePoint(NUI_HANDEDNESS_LEFT);
	UpdatePoint(NUI_HANDEDNESS_RIGHT);
	SendRayData();
}


void rayCast::DrawHands()
{
		
	for(int i = 0; i < NUI_HANDEDNESS_COUNT; i++)
	{
		// draw the hand mesh
		shared_ptr<ofVec4fSet> handPts = m_handMesh.handPoints((NUI_HANDEDNESS)i);
		m_handViz[i].clear();
		for(ofVec4fSet::iterator it = handPts->begin(); it != handPts->end(); it++)
		{
			ofVec4f v = *it;
			m_handViz[i].addColor(ofColor(0, 255, 255));
			m_handViz[i].addVertex(ofVec3f(v.x, v.y, v.z));
		}
		m_handViz[i].draw();
	}
}

void rayCast::DrawFingers()
{
	for(int i = 0; i < NUI_HANDEDNESS_COUNT; i++)
	{
		// draw the hand mesh
		shared_ptr<ofVec4fSet> fingerPts = m_handMesh.fingerPoints((NUI_HANDEDNESS)i);
		m_fingerViz[i].clear();
		for(ofVec4fSet::iterator it = fingerPts->begin(); it != fingerPts->end(); it++)
		{
			ofVec4f v = *it;
			m_fingerViz[i].addColor(ofColor(255, 0, 255));
			m_fingerViz[i].addVertex(ofVec3f(v.x, v.y, v.z));
		}
		m_fingerViz[i].draw();
	}
}

//--------------------------------------------------------------
void rayCast::draw()
{
	ofBackgroundGradient(ofColor::gray, ofColor::black, OF_GRADIENT_CIRCULAR);

	m_cam.begin();
	if(m_showDepth){
		UpdatePointMeshShowDepth();
		m_mesh.draw();}
	m_kinectView.draw();
	m_skeletonView.draw();
	if(m_showRay)
	{
		DrawRay(NUI_HANDEDNESS_RIGHT, ofColor(255, 0, 0));
		DrawRay(NUI_HANDEDNESS_LEFT, ofColor(255, 0, 255));
	}

	DrawScreen();
	
	if(m_showHand) DrawHands();
	if(m_showFingers) DrawFingers();
	m_cam.end();

	DrawDebugInfo();
	
	float swidth = ofGetWidth();
	float sheight = ofGetHeight();

	ofPoint m_displayPos;
	ofPushStyle();
	for(int i = 0; i < NUI_HANDEDNESS_COUNT; i++)
	{
		m_displayPos.x = m_screenPos[i].x * swidth;
		m_displayPos.y = m_screenPos[i].y * sheight;
		m_displayPos.z = m_screenPos[i].z;
		if(i == NUI_HANDEDNESS_RIGHT) ofSetColor(255, 0, 0);
		else ofSetColor(255, 0, 255);
		ofCircle(m_displayPos, 20);
	}
	ofPopStyle();

}

void rayCast::DrawDebugInfo()
{
	// draw fps
	stringstream reportStream;


	reportStream << "c to calibrate" << endl;
	reportStream << "f to show fingers" << endl;
	reportStream << "r to show ray" << endl;
	reportStream << "d to show depth" << endl;
	reportStream << "UNITS ARE IN MM" << endl;
	reportStream << "fps: " << ofGetFrameRate() << endl;
	if(m_calibrationMode) reportStream << "calibration ON" << endl;
	if(!m_calibrationMode) reportStream << "calibration OFF" << endl;
	reportStream << m_debugStream.str();
	ofSetColor(255, 0, 0);
	ofDrawBitmapString(reportStream.str(), 10, 20);
}

void rayCast::SendRayData()
{
	for(int i = 0; i < NUI_HANDEDNESS_COUNT; i++)
	{
		ofPoint p = m_screenPos[i];

		if(p.x >= 0 && p.x <= 1 && 
			p.y >= 0 && p.y <= 1)
		{
			// send the x, y location in screen coordinates
			ostringstream convert;
			convert << i << ":" << p.x << "," << p.y << "," << m_skeletonMetrics.armExtension((NUI_HANDEDNESS)i) << "," << m_skeletonMetrics.headDistance();
			string message = convert.str();
			int sent = m_udpConnection.Send(message.c_str(), message.length());
		}
	}
}

void rayCast::SaveCalibration()
{
	stringstream dataStream;
	for(int i = 0; i < 4; i++)
	{
		dataStream << m_screenPoints[i].x << "," << 
			m_screenPoints[i].y << "," << 
			m_screenPoints[i].z << endl;
	}

	ofBufferToFile(ofToDataPath(RAYCAST_CONFIG), ofBuffer(dataStream));
}

void rayCast::LoadCalibration()
{
	// check if file exists, otherwise set to zero
	ofFile file(ofToDataPath(RAYCAST_CONFIG));
	int i = 0;
	if(file.exists())
	{
		ofBuffer dataBuffer = ofBufferFromFile(ofToDataPath(RAYCAST_CONFIG), false);
		do
		{
			string line = dataBuffer.getNextLine();
			vector<string> parts = ofSplitString(line, ",", true, true);
			ofVec3f	screenPoint;
			screenPoint.x = atof(parts[0].c_str());
			screenPoint.y = atof(parts[1].c_str());
			screenPoint.z = atof(parts[2].c_str());
			ofLog(OF_LOG_NOTICE) << "raycast calibration point: " << screenPoint.x << "," << screenPoint.y << "," << screenPoint.z << endl;
			m_screenPoints[i] = screenPoint;
			i++;
		}while(!dataBuffer.isLastLine());

	} 
	else 
	{
		ofLog(OF_LOG_NOTICE) << "no calibration file found, path is " << ofToDataPath(RAYCAST_CONFIG) << endl;
		for(int i = 0; i < 4; i++)
		{
			m_screenPoints[i] = ofVec3f();
		}

	}

}


//--------------------------------------------------------------
void rayCast::keyPressed(int key)
{
	if(key == 'c') m_calibrationMode = !m_calibrationMode;
	if(key == 'w')
	{
		SaveCalibration();
		m_debugStream << "wrote calibration to " << ofToDataPath(RAYCAST_CONFIG) << endl;
	}
	if(key == 'h') m_showHand = !m_showHand;
	if(key == 'f') m_showFingers = !m_showFingers;
	if(key == 'd') m_showDepth = !m_showDepth;
	if(key == 'r') m_showRay = !m_showRay;
	if(key == ' ') m_useMeshForHand = !m_useMeshForHand;

}

//--------------------------------------------------------------
void rayCast::keyReleased(int key){
}

//--------------------------------------------------------------
void rayCast::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void rayCast::mouseDragged(int x, int y, int button){
	
}

//--------------------------------------------------------------
void rayCast::mousePressed(int x, int y, int button)
{
	if(m_calibrationMode)
	{
		if(m_calibrateIndex == 0)
		{
			for(int i = 0; i < 4; i++)
			{
				m_screenPoints[i] = ofVec3f();
			}
		}

		// IMPORTANT
		// assume that right hand is used for calibrating hte region
		ofVec4f v = m_rayIntersection[NUI_HANDEDNESS_RIGHT];
		float w = v.w;
		m_screenPoints[m_calibrateIndex] = ofVec3f(
			v.x / w,
			v.y / w,
			v.z / w);// ray cast location

		m_calibrateIndex++;
		
		m_calibrateIndex = m_calibrateIndex % 4;
	}
}

//--------------------------------------------------------------
void rayCast::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void rayCast::windowResized(int w, int h){

}

//--------------------------------------------------------------
void rayCast::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void rayCast::dragEvent(ofDragInfo dragInfo){ 

}

//--------------------------------------------------------------
int main(){
	ofAppGlutWindow window; // create a window
	// set width, height, mode (OF_WINDOW or OF_FULLSCREEN)
	ofSetupOpenGL(&window, 1024, 768, OF_WINDOW);
	ofRunApp(new rayCast()); // start the app
}