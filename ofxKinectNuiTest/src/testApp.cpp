/******************************************************************/
/**
 * @file	testApp.cpp
 * @brief	Example for ofxKinectNui addon
 * @note
 * @todo
 * @bug	
 *
 * @author	sadmb
 * @date	Oct. 28, 2011
 */
/******************************************************************/
#include "testApp.h"
#include "ofxKinectNuiDraw.h"

//--------------------------------------------------------------
void testApp::setup() {
	m_mesh.setMode(OF_PRIMITIVE_POINTS);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POINT_SMOOTH); // use circular points instead of square points
	glPointSize(3); // make the points bigger
	

	for(int y = 0; y < cDepthHeight; y += cSkip) {
		for(int x = 0; x < cDepthWidth; x += cSkip) {
			ofColor cur(255,255,255,255);
			m_mesh.addColor(cur);

			ofVec3f pos(x, y, 0);
			m_mesh.addVertex(pos);
		}
	}

	ofSetLogLevel(OF_LOG_VERBOSE);
	
	ofxKinectNui::InitSetting initSetting;
	initSetting.grabDepth = true;
	initSetting.depthResolution = NUI_IMAGE_RESOLUTION::NUI_IMAGE_RESOLUTION_320x240;
	kinect.init(initSetting);
	kinect.open();
//	kinect.open(true); // when you want to use near mode (default is false)

	ofSetVerticalSync(true);

	kinectSource = &kinect;

	ofSetFrameRate(60);
	
}

//--------------------------------------------------------------
void testApp::update() {
	kinectSource->update();
#ifdef USE_TWO_KINECTS
	kinect2.update();
#endif

	const USHORT * pBufferRun = kinect.getDistancePixels().getPixels();
	const USHORT * pBufferEnd = pBufferRun + (cDepthWidth * cDepthHeight);
	int x = 0;
	int y = 0;
	int i = 0;
    while ( pBufferRun < pBufferEnd )
    {
        // discard the portion of the depth that contains only the player index
        USHORT depth = *pBufferRun;
		depth = depth / 10.0;

		// add a particle at the x, y location
		ofVec3f pos(x, y, depth);
		m_mesh.setVertex(i, pos);
		m_mesh.setColor(i, ofColor(255 - depth));

		x+= cSkip;
		if(x > cDepthWidth){
			x = 0;
			y+= cSkip;
			pBufferRun += cSkip * cDepthWidth;
		} else 
		{
			pBufferRun += cSkip;
		}
		i++;
    }
}

//--------------------------------------------------------------
void testApp::draw() {
	ofBackground(100, 100, 100);
	
	m_cam.begin();
	ofScale(1, -1, 1); // flip the y axis 
	//ofRotateY(45);
	ofTranslate(-cDepthWidth/ 2, -cDepthHeight / 2);
	m_mesh.draw();
	m_cam.end();

	
	// draw instructions
	ofSetColor(255, 255, 255);
	stringstream reportStream;
	reportStream << "fps: " << ofGetFrameRate() << "  Kinect Nearmode: " << kinect.isNearmode() << endl
				 << "press 'c' to close the stream and 'o' to open it again, stream is: " << kinect.isOpened() << endl
				 << "press UP and DOWN to change the tilt angle: " << angle << " degrees" << endl
				 << "press LEFT and RIGHT to change the far clipping distance: " << farClipping << " mm" << endl
				 << "press '+' and '-' to change the near clipping distance: " << nearClipping << " mm" << endl
				 << "press 'r' to record and 'p' to playback, record is: " << bRecord << ", playback is: " << bPlayback << endl
				 << "press 'v' to show video only: " << bDrawVideo << ",      press 'd' to show depth + users label only: " << bDrawDepthLabel << endl
				 << "press 's' to show skeleton only: " << bDrawSkeleton << ",   press 'q' to show point cloud sample: " << bDrawCalibratedTexture;
	ofDrawBitmapString(reportStream.str(), 20, 648);
}

//--------------------------------------------------------------


//--------------------------------------------------------------
void testApp::exit() {
	if(calibratedTexture.bAllocated()){
		calibratedTexture.clear();
	}

	if(videoDraw_) {
		videoDraw_->destroy();
		videoDraw_ = NULL;
	}
	if(depthDraw_) {
		depthDraw_->destroy();
		depthDraw_ = NULL;
	}
	if(labelDraw_) {
		labelDraw_->destroy();
		labelDraw_ = NULL;
	}
	if(skeletonDraw_) {
		delete skeletonDraw_;
		skeletonDraw_ = NULL;
	}

	kinect.setAngle(0);
	kinect.close();
	kinect.removeKinectListener(this);
	kinectPlayer.close();
	kinectRecorder.close();


#ifdef USE_TWO_KINECTS
	kinect2.close();
#endif
}

//--------------------------------------------------------------
void testApp::keyPressed (int key) {
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y) {
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
void testApp::kinectPlugged(){
	bPlugged = true;
}

//--------------------------------------------------------------
void testApp::kinectUnplugged(){
	bPlugged = false;
}

//--------------------------------------------------------------
void testApp::startRecording(){

}

//--------------------------------------------------------------
void testApp::stopRecording(){
}

//--------------------------------------------------------------
void testApp::startPlayback(){
}

//--------------------------------------------------------------
void testApp::stopPlayback(){
}
