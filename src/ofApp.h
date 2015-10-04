#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofxKinect.h"
#include "ofxRipples.h"
#include "ofxBounce.h"
#include "ofUtils.h"

#include <math.h>

#define PROJECTOR_WIDTH 1024
#define PROJECTOR_HEIGHT 768

#define MIN_CONTOUR_AREA 1400

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
    
    // Kinect variables
    ofxKinect   kinect;
    ofxCvGrayscaleImage     input_image;
    ofxCvGrayscaleImage     background;
    int                     threshold;
    bool b_learn_background;
    int angle;
    ofxCv::ContourFinder    contourFinder;
    ofxCvContourFinder      prettyContourFinder;
    int min_contour_area;
    
    // Ripples
    ofxRipples  ripples;
    ofxBounce   bounce;
    int         damping;
    ofFbo       fbo;
    void        drawRipples();
    
    // Colors
    ofColor foreground_color;
    ofColor background_color;
    int min_brightness;
		
    bool bCalibrate;
    int dx;
    int dy;
    float dz;
    
    bool display_feedback;
    
    int red;
    int green;
    int blue;
    bool bFreezeRed;
    bool bFreezeGreen;
    bool bFreezeBlue;
    int redFrame;
    int greenFrame;
    int blueFrame;
    
    // Metronome
    float tempo;
    float lastBeat;
    bool bEvaluateTempo;
    vector<float> beats;
    void evaluateTempo();
};
