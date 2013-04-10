#include "testApp.h"

testApp::testApp()
{
	m_cameraPos = ofVec3f(0,0,-10);
	m_cameraVelocity = 0;
	m_currentVisualizationMode = VisualizationMode::Depth;
}

testApp::~testApp()
{
	// don't need to delete m_simple Kinect because descrctor for object gets called when this object
	// gets destroyed
}

//--------------------------------------------------------------
void testApp::setup(){
	ofColor playerColors[] = {
		ofColor(255, 0,0),
		ofColor(0, 255,0),
		ofColor(0, 0,255),
		ofColor(255, 255,0),
		ofColor(255, 0,255),
		ofColor(0, 255,255),
		ofColor(255, 255,255)};
	// set up player colors
	for (int i = 0; i < m_simpleKinect.cNumPlayers; i++)
	{
		m_playerColors[i] = playerColors[i];
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POINT_SMOOTH); // use circular points instead of square points
	glPointSize(3); // make the points bigger
	glLineWidth(2);
	// set camera properties
	
	// we're going to load a ton of points into an ofm_mesh
	m_mesh.setMode(OF_PRIMITIVE_POINTS);
	
	m_artMesh.setMode(OF_PRIMITIVE_POINTS);

	m_lineMesh.setMode(OF_PRIMITIVE_LINES);

	ofColor cur(0,0,0,0);

	for(int y = 0; y < m_simpleKinect.cDepthHeight; y += cSkip) {
		for(int x = 0; x < m_simpleKinect.cDepthWidth; x += cSkip) {
			m_mesh.addColor(cur);
			ofVec3f pos(0, 0, 0);
			m_mesh.addVertex(pos);
		}
	}

	for(int y = m_simpleKinect.cOpticalFlowWindowSize / 2; y < m_simpleKinect.cDepthHeight; y += m_simpleKinect.cOpticalFlowWindowSize) {
		for(int x = m_simpleKinect.cOpticalFlowWindowSize / 2; x < m_simpleKinect.cDepthWidth; x += m_simpleKinect.cOpticalFlowWindowSize) {
			m_lineMesh.addColor(cur);
			m_lineMesh.addColor(cur);

			ofVec3f pos1(x - m_simpleKinect.cOpticalFlowWindowSize / 2, y - m_simpleKinect.cOpticalFlowWindowSize / 2, 0);
			ofVec3f pos2(x + m_simpleKinect.cOpticalFlowWindowSize / 2, y + m_simpleKinect.cOpticalFlowWindowSize / 2, 0);

			m_lineMesh.addVertex(pos1);
			m_lineMesh.addVertex(pos2);
		}
	}

	m_simpleKinect.Initialize();
}

//--------------------------------------------------------------
void testApp::update(){
	if(!m_simpleKinect.ReadyForUpdate())
	{
		return;
	}
    
	m_simpleKinect.UpdateDepth();
	m_simpleKinect.UpdateWorld();

	if(m_artMesh.getVertices().size() > 0)
	{
		m_artMesh.removeVertex(0);
		m_artMesh.removeColor(0);
	}

	switch(m_currentVisualizationMode)
	{
		case VisualizationMode::Depth:
			break;
		case VisualizationMode::DepthColor:
			m_simpleKinect.UpdateColor();
			break;
		case VisualizationMode::FlowLines:
		case VisualizationMode::FlowPoints:
			m_simpleKinect.UpdateOpticFlow();
			break;
		case VisualizationMode::FlowDzDtPoints:
			m_simpleKinect.UpdateOpticFlowDz();
			break;
	}
}



//--------------------------------------------------------------
void testApp::draw()
{
	ofBackgroundGradient(ofColor::gray, ofColor::black, OF_GRADIENT_CIRCULAR);
	// even points can overlap with each other, let's avoid that
	m_cam.begin();
	switch(m_currentVisualizationMode)
	{
		case VisualizationMode::Depth:
			UpdatePointMeshShowDepth();
			m_mesh.draw();
		break;
		case VisualizationMode::DepthColor:
			UpdatePointMeshShowColor();
			m_mesh.draw();
		break;
		case VisualizationMode::FlowLines:
			UpdateLineMesh();
			m_lineMesh.draw();
		break;
		case VisualizationMode::FlowPoints:
			UpdatePointMeshShowFlow();
			m_mesh.draw();
			m_artMesh.draw();
		break;
		case VisualizationMode::FlowDzDtPoints:
			UpdatePointMeshShowFlowDz();
			m_mesh.draw();
			m_artMesh.draw();
			break;
	}
	
	m_kinectView.draw();

	m_cam.end();


	// draw fps
	stringstream reportStream;
	reportStream << "UNITS ARE IN CM" << endl;
	reportStream << "fps: " << ofGetFrameRate();

	ofSetColor(255, 0, 0);
	ofDrawBitmapString(reportStream.str(), 10, 20);
}

void testApp::UpdatePointMeshShowDepth()
{
	USHORT* pBufferRun = m_simpleKinect.m_pCurrentDepth;
	Vector4* pBufferWorld = m_simpleKinect.m_pCurrentWorld;
	HRESULT hr;

    // end pixel is start + width*height - 1
    const USHORT  * pBufferEnd = pBufferRun + (m_simpleKinect.cDepthWidth * m_simpleKinect.cDepthHeight);
	LONG* pBufferDepthToColor = m_simpleKinect.m_pDepthToColorCoordinates;
	int x = 0,  y = 0, i = 0;
	LONG xrgb = 0, yrgb = 0;
	BYTE* pColor;
    
	while ( pBufferRun < pBufferEnd )
    {
        // discard the portion of the depth that contains only the player index
		USHORT player = NuiDepthPixelToPlayerIndex(*pBufferRun);
		Vector4 world = *pBufferWorld;

				
		if(player == 0)
		{
			// todo: convert to function
			m_mesh.setColor(i, ofColor(255 - 255 * ((world.z) - m_simpleKinect.cDepthMinimum) / (m_simpleKinect.cDepthMaximum - m_simpleKinect.cDepthMinimum) ));
		} else 
		{
			m_mesh.setColor(i, m_playerColors[player]);
		}
		
		// add a particle at the x, y location
		ofVec3f pos(world.x, world.y , world.z );
		m_mesh.setVertex(i, pos);

		// update indices
		x+= cSkip;
		if(x >= m_simpleKinect.cDepthWidth){
			x = 0;
			y+= cSkip;
			pBufferRun += cSkip * m_simpleKinect.cDepthWidth;
			pBufferDepthToColor += cSkip * m_simpleKinect.cDepthWidth * 2;
			pBufferWorld += cSkip * m_simpleKinect.cDepthWidth ;
		} else 
		{
			pBufferRun += cSkip;
			pBufferDepthToColor += cSkip * 2;
			pBufferWorld += cSkip;
		}
		i++;
    }

}

void testApp::UpdatePointMeshShowColor()
{
	USHORT* pBufferRun = m_simpleKinect.m_pCurrentDepth;
	Vector4* pBufferWorld = m_simpleKinect.m_pCurrentWorld;
	HRESULT hr;

    // end pixel is start + width*height - 1
    const USHORT  * pBufferEnd = pBufferRun + (m_simpleKinect.cDepthWidth * m_simpleKinect.cDepthHeight);
	LONG* pBufferDepthToColor = m_simpleKinect.m_pDepthToColorCoordinates;
	int x = 0,  y = 0, i = 0;
	LONG xrgb = 0, yrgb = 0;
	BYTE* pColor;
    
	while ( pBufferRun < pBufferEnd )
    {
		Vector4 world = *pBufferWorld;

		xrgb = pBufferDepthToColor[0];
		yrgb = pBufferDepthToColor[1];

		int offset = ((yrgb * m_simpleKinect.cDepthWidth + xrgb) * m_simpleKinect.cBytesPerPixel);
		if(offset >= m_simpleKinect.cDepthWidth * m_simpleKinect.cDepthHeight * m_simpleKinect.cBytesPerPixel)
		{
			return;
		}

		pColor = m_simpleKinect.m_pCurrentRGB + offset;

		m_mesh.setColor(i, ofColor(pColor[2], pColor[1], pColor[0]));

		// add a particle at the x, y location
		ofVec3f pos(world.x, world.y , world.z );
		m_mesh.setVertex(i, pos);

		// update indices
		x+=  cSkip;
		if(x >= m_simpleKinect.cDepthWidth){
			x = 0;
			y+= cSkip;
			pBufferRun += cSkip * m_simpleKinect.cDepthWidth;
			pBufferDepthToColor += cSkip * m_simpleKinect.cDepthWidth * 2;
			pBufferWorld += cSkip * m_simpleKinect.cDepthWidth ;
		} else 
		{
			pBufferRun += cSkip;
			pBufferDepthToColor += cSkip * 2;
			pBufferWorld += cSkip;
		}
		i++;
    }

}

void testApp::UpdatePointMeshShowFlow()
{
	int x = 0,  y = 0, i = 0;
	int opticalFlowCols = m_simpleKinect.cDepthWidth / m_simpleKinect.cOpticalFlowWindowSize;
	int opticalFlowRows = m_simpleKinect.cDepthHeight / m_simpleKinect.cOpticalFlowWindowSize;
	int numVertices = m_mesh.getNumVertices();
    int ofX=0,ofY=0;
	
	while ( i < numVertices )
    {
		USHORT player = NuiDepthPixelToPlayerIndex(m_simpleKinect.m_pCurrentDepth[y * m_simpleKinect.cDepthWidth + x]);
        // discard the portion of the depth that contains only the player index
		Vector4 world = m_simpleKinect.m_pCurrentWorld[y * m_simpleKinect.cDepthWidth + x];

		// get magnitude of optic flow at this pixel
		// set color based on magnitude
		ofX= x / (m_simpleKinect.cOpticalFlowWindowSize);
		ofY = y / (m_simpleKinect.cOpticalFlowWindowSize);
		LONG* opticFlow = m_simpleKinect.m_pOpticFlow + (ofY * opticalFlowCols + ofX) * 3;

		LONG dx = opticFlow[0];
		LONG dy = opticFlow[1];
		LONG dz = opticFlow[2];

		LONG magnitude = sqrtl(dx * dx + dy * dy + dz * dz);
		magnitude = magnitude;
		m_mesh.setColor(i, ofColor(magnitude / 70.0f * 255));

				if(sqrtl(dx * dx + dy * dy) > 50 && player > 0)
				{
					if(world.z > 0){
						m_artMesh.addColor(ofColor(0, 255, 0, 0.01f));
						m_artMesh.addVertex(ofVec3f(world.x, world.y, world.z));
					}
				}


		// add a particle at the x, y location
		ofVec3f pos(world.x, world.y , world.z );
		m_mesh.setVertex(i, pos);

		// update indices
		x+= cSkip;
		if(x > m_simpleKinect.cDepthWidth)
		{
			x = 0;
			y+= cSkip;
		}
		i++;
    }

}



void testApp::UpdatePointMeshShowFlowDz()
{
	int x = 0,  y = 0, i = 0;
	int numVertices = m_mesh.getNumVertices();

	while ( i < numVertices )
    {
		USHORT player = NuiDepthPixelToPlayerIndex(m_simpleKinect.m_pCurrentDepth[y * m_simpleKinect.cDepthWidth + x]);

        // discard the portion of the depth that contains only the player index
		Vector4 world = m_simpleKinect.m_pCurrentWorld[y * m_simpleKinect.cDepthWidth + x];

		LONG* opticFlow = m_simpleKinect.m_pOpticFlowDz + y * m_simpleKinect.cDepthWidth + x;

		if(m_mesh.getColor(i).g != 255)
		{
			if(player > 0)
			{
				long flow = abs(*opticFlow);
				if(flow > 100)
				{
					if(world.z > 0){
						m_artMesh.addColor(ofColor(0, 255, 0, 0.01));
						m_artMesh.addVertex(ofVec3f(world.x, world.y, world.z));
					}
				}

				m_mesh.setColor(i, ofColor((flow / 100.0f) * 255));
			} else
			{
				m_mesh.setColor(i, ofColor(0));
			}
		}

		// add a particle at the x, y location
		ofVec3f pos(world.x, world.y , world.z );
		m_mesh.setVertex(i, pos);

		// update indices
		x+= cSkip;
		if(x > m_simpleKinect.cDepthWidth)
		{
			x = 0;
			y+= cSkip;
		}
		i++;
    }

}

void testApp::UpdateLineMesh()
{
	LONG* pOpticFlow = m_simpleKinect.m_pOpticFlow;

	int x = m_simpleKinect.cOpticalFlowWindowSize / 2,  y = m_simpleKinect.cOpticalFlowWindowSize / 2, i = 0;
	int numVertices = m_lineMesh.getNumVertices();
	//LONG maxFlowDistance = 0, minFlowDistance = LONG_MAX;
	// TODO: Use world coordiantes!
	while ( i < numVertices)
    {
		Vector4 curWorld = m_simpleKinect.m_pCurrentWorld[y * m_simpleKinect.cDepthWidth + x];

		LONG dx = pOpticFlow[0];		
		LONG dy = pOpticFlow[1];
		LONG dz = pOpticFlow[2];
		
		if(curWorld.z > 0)
		{
			ofVec3f pos1(curWorld.x - dx, curWorld.y - dy, curWorld.z - dz);
			ofVec3f pos2(curWorld.x + dx, curWorld.y + dy, curWorld.z + dz);

			m_lineMesh.setVertex(i, pos1);
			m_lineMesh.setVertex(i + 1, pos2);
		
			double flowDistance = sqrtl(dx * dx + dy * dy + dz * dz);

			m_lineMesh.setColor(i, ofColor(255 * flowDistance / 7.0f));
			m_lineMesh.setColor(i + 1, ofColor(255 * flowDistance / 7.0f));	
			
		
		} else 
		{
			m_lineMesh.setVertex(i, ofVec3f(0,0,0));
			m_lineMesh.setVertex(i + 1, ofVec3f(0,0,0));
		}


		// update indices
		x+= m_simpleKinect.cOpticalFlowWindowSize;
		if(x >= m_simpleKinect.cDepthWidth)
		{
			x = m_simpleKinect.cOpticalFlowWindowSize / 2;
			y+= m_simpleKinect.cOpticalFlowWindowSize;
		} 

		pOpticFlow+=3;
		i+=2;
    }
	//ofLog() << "zmax: " << maxFlowDistance << " zmin: " << minFlowDistance;
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	int nextVal = 0;
	switch(key)
	{
		case ' ':
			nextVal = (m_currentVisualizationMode + 1) % VisualizationMode::VisualizationModeCount;
			m_currentVisualizationMode = static_cast<VisualizationMode>(nextVal);
			m_artMesh.clear();
			break;
	}
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
	m_cameraVelocity = 0;
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}