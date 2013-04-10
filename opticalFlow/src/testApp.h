#pragma once

// Make sure Windows.h is included before NuiApi.h for using Kinect for Windows
#include <Windows.h>

#include "SimpleKinect.h"
#include "ofMain.h"
#include "kinectView.h"
#include "orthoCamera.h"

class testApp : public ofBaseApp{
	static const int		cSkip = 4;
	static const int		cNumPlayers = 7;
public:
	
	testApp();
    /// <summary>
    /// Destructor
    /// </summary>
    ~testApp();

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

	enum VisualizationMode { 
		Depth = 0, 
		DepthColor, 
		FlowLines, 
		FlowPoints, 
		FlowDzDtPoints,
		VisualizationModeCount };

	
private:
	SimpleKinect			m_simpleKinect;
    
    /// <summary>
    /// Handle new depth data
    /// </summary>
    void                    UpdatePointMeshShowDepth();
	void                    UpdatePointMeshShowColor();
	void                    UpdatePointMeshShowFlow();
	void                    UpdatePointMeshShowFlowDz();
	void					UpdateLineMesh();
	
	orthoCamera				m_cam;
	ofMesh					m_mesh;
	ofMesh					m_artMesh;
	ofMesh					m_lineMesh;
	ofVec3f					m_cameraPos;
	ofColor					m_playerColors[cNumPlayers];
	int						m_cameraVelocity;
	kinectView				m_kinectView;
	
	// settings
	
	VisualizationMode	m_currentVisualizationMode;

};
