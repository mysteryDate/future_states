#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    mode = 1;

    display_feedback = true;
    
    // Kinect
    b_learn_background = false;
    kinect.init();
    kinect.open();
    input_image.allocate(kinect.width, kinect.height);
    background.allocate(kinect.width, kinect.height);
    background.set(0);
    threshold = 10;
    contourFinder.setMinArea(MIN_CONTOUR_AREA);
    min_contour_area = 1400;
    
    angle = 17;
    kinect.setCameraTiltAngle(angle);
    
    // Ripples
    ofEnableAlphaBlending();
    ripples.allocate(kinect.width, kinect.height);
    bounce.allocate(kinect.width, kinect.height);
    ripples.damping = 0.97;
    fbo.allocate(kinect.width, kinect.height, GL_RGBA);
    fbo.begin();
    ofClear(255,255,255, 0);
    fbo.end();
    
    bCalibrate = true;
    dx = 0;
    dy = 0;
    dz = 1;
    
    min_brightness = 90;
    ofBackground(0, 0, 0);
    
    red = 0;
    green = 0;
    blue = 0;
    redFrame = 0;
    greenFrame = 0;
    blueFrame = 0;
    
    // Tempo
    bEvaluateTempo = true;
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    // Is it a beat?
    float intPart = 0;
    float beatProximity = modf((ofGetElapsedTimef() - lastBeat) / tempo, &intPart);

    // Colors
    int frame = ofGetFrameNum();
//    int hue = ofMap(frame % 200, 0, 200, 0, 255);

    
    if(!bFreezeRed) {
        redFrame++;
    }
    if(!bFreezeGreen) {
        greenFrame++;
    }
    if(!bFreezeBlue) {
        blueFrame++;
    }
    red =   ofMap(sin(2 * PI * redFrame/ 233), -1, 1, 0, 255);
    green = ofMap(sin(2 * PI * greenFrame/127), -1, 1, 0, 255);
    blue =  ofMap(sin(2 * PI * blueFrame/179), -1, 1, 0, 255);
//    foreground_color = ofColor::fromHsb(hue, 127, 127);
    foreground_color = ofColor(red, green, blue);
    background_color = ofColor(1 - red, 1 - green, 1- blue);
    
    float brightness = 1;

    if (beatProximity > 0.8) {
        brightness = cos(2 * PI * (beatProximity - 1) / 0.4) + 2;
    }
    if (beatProximity < 0.2) {
        brightness = cos(2 * PI * (beatProximity) / 0.4) + 2;
    }
    foreground_color *= (brightness * brightness);

    // Kinect
    kinect.update();
    if (kinect.isFrameNew()) {
        input_image.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
        if(b_learn_background) {
            background = input_image;
            b_learn_background = false;
        }
        input_image -= background;
        // Take only stuff that's significantly different than the background
        if(true){
            unsigned char *pix = input_image.getPixels();
            for (int i = 0; i < input_image.getHeight() * input_image.getWidth(); i++) {
                if(pix[i] > threshold)
                    pix[i] = 255;
                else
                    pix[i] = 0;
            }
        }
        input_image.flagImageChanged();
//        if(bCalibrate) {
            prettyContourFinder.findContours(input_image, min_contour_area, (kinect.width*kinect.height)/2, 7, false);
//        }
//        else {
//            contourFinder.findContours(input_image);
//        }
    }
    
    if(mode == 2){
        updateRipples();
    }
    else {
        ofPushStyle();
        ofSetColor(0,0,0);
        ofRect(0, 0, ofGetWidth(), ofGetHeight());
        ofPopStyle();
    }
    
    if (bEvaluateTempo) {
        evaluateTempo();
    }
}

//--------------------------------------------------------------
void ofApp::updateRipples() {
    
    ripples.begin();
    ofPushStyle();
    ofSetColor(foreground_color);
    for (int i = 0; i < prettyContourFinder.blobs.size(); i++) {
        vector<ofPoint> blob = prettyContourFinder.blobs[i].pts;
        
        ofBeginShape();
        for (int j = 0; j < blob.size(); j++) {
            ofVertex(blob[j]);
        }
        ofEndShape();
    }
    ofPopStyle();
    
    ripples.end();
    ripples.update();
}

//--------------------------------------------------------------
void ofApp::evaluateTempo() {
    
    tempo = 0;
    
    for (int ii = 1; ii < beats.size(); ii++) {
        float t = beats[ii] - beats[ii - 1];
        tempo += t;
    }
    
    tempo /= (beats.size() - 1);
    
    beats.clear();
    bEvaluateTempo = false;
    
    lastBeat = ofGetElapsedTimef();
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    if(mode == 1) {
        ofPushMatrix();
//        ofTranslate(dx, dy);
//        ofScale(ofGetWidth()/kinect.width*dz,ofGetHeight()/kinect.height*dz);
//        input_image.draw(0,0);
        prettyContourFinder.draw(dx,dy, ofGetWidth()*dz, ofGetHeight()*dz);
//        input_image.draw(dx, dy, PROJECTOR_WIDTH*dz, PROJECTOR_HEIGHT*dz);
//        contourFinder.draw(dx, dy, ofGetWidth()*dz, ofGetHeight()*dz);
        ofPopMatrix();
    }
    else if (mode == 2) {
        ripples.draw(dx, dy, ofGetWidth()*dz, ofGetHeight()*dz);
//        input_image.draw(0, 0, ofGetWidth()*dz, ofGetHeight()*dz);
    }
    else if (mode == 3) {
        easyCam.begin();
        drawPointCloud();
        easyCam.end();
    }
    
    stringstream reportStream;
    reportStream << "Framerate: " << ofToString(ofGetFrameRate()) << endl
    << "Frame: " << ofToString(ofGetFrameNum()) << endl
    << "Threshold: " << threshold << endl
    << "Contour area: " << min_contour_area << endl
    << "Damping: " << ripples.damping << endl
    << "Tilt Angle: " << angle << endl
    << "RGB: " << red << ", " << green << ", " << blue << endl
//    << "Tempo: " << (1.0 / tempo) * 60 << "bpm" << endl;
    << (ofGetElapsedTimef() - lastBeat) / tempo << endl;
    if(display_feedback) {
        ofDrawBitmapString(reportStream.str(), 100, ofGetHeight() - 200);
    }
}

//--------------------------------------------------------------
void ofApp::drawPointCloud() {
    int w = 640;
    int h = 480;
    ofMesh mesh;
    mesh.setMode(OF_PRIMITIVE_POINTS);
    int step = 2;
    for(int y = 0; y < h; y += step) {
        for(int x = 0; x < w; x += step) {
            float dist = kinect.getDistanceAt(x, y);
            if(dist > 0) {
                mesh.addColor(foreground_color * ofMap(dist, 0, 3000, 2, 0));
                mesh.addVertex(kinect.getWorldCoordinateAt(x, y));
            }
        }
    }
    glPointSize(3);
    ofPushMatrix();
    // the projected points are 'upside down' and 'backwards'
    ofScale(1, -1, -1);
    ofTranslate(0, 0, -1000); // center the points a bit
    ofEnableDepthTest();
    mesh.drawVertices();
    ofDisableDepthTest();
    ofPopMatrix();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    switch(key) {
        case ' ':
            display_feedback = !display_feedback;
            break;
        case 'f':
            ofToggleFullscreen();
            break;
        case 'v':
            b_learn_background = true;
            break;
        case 'V':
            background.set(0);
            break;
        case '=':
            threshold++;
            break;
        case '-':
            threshold--;
            break;
        case 'D':
            ripples.damping += 0.01;
            break;
        case 'd':
            ripples.damping -= 0.01;
            break;
        case 'M':
            min_contour_area+=10;
            break;
        case 'm':
            min_contour_area-=10;
            break;
        case OF_KEY_UP:
            dy--;
            break;
        case OF_KEY_DOWN:
            dy++;
            break;
        case OF_KEY_LEFT:
            dx--;
            break;
        case OF_KEY_RIGHT:
            dx++;
            break;
        case 'Z':
            dz += 0.01;
            break;
        case 'z':
            dz -= 0.01;
            break;
        case 'T':
            angle++;
            if(angle>30) angle=30;
            kinect.setCameraTiltAngle(angle);
            break;
        case 't':
            angle--;
            if(angle<-30) angle=-30;
            kinect.setCameraTiltAngle(angle);
            break;
        case 'r':
            bFreezeRed = !bFreezeRed;
            break;
        case 'g':
            bFreezeGreen = !bFreezeGreen;
            break;
        case 'b':
            bFreezeBlue = !bFreezeBlue;
            break;
        case 'j':
            beats.push_back(ofGetElapsedTimef());
            if (beats.size() == 4) {
                bEvaluateTempo = true;
            }
            break;
        case '1':
            mode = 1;
            break;
        case '2':
            mode = 2;
            break;
        case '3':
            mode = 3;
            break;
    }
}
