#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

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
}

//--------------------------------------------------------------
void ofApp::update(){

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
            prettyContourFinder.findContours(input_image, MIN_CONTOUR_AREA, (kinect.width*kinect.height)/2, 7, false);
//        }
//        else {
//            contourFinder.findContours(input_image);
//        }
    }
    
    // Ripples
    ripples.begin();
//    if (b_learn_background) {
//        ofPushStyle();
//        ofSetColor(0, 0, 0);
//        ofRect(0, 0, kinect.width, kinect.height);
//        ofPopStyle();
//        b_learn_background = false;
//    }
    if(!bCalibrate){
        ofPushStyle();
        ofSetColor(foreground_color);
        for (int i = 0; i < prettyContourFinder.blobs.size(); i++) {
//            ofPolyline blob = contourFinder.getPolyline(i);
            vector<ofPoint> blob = prettyContourFinder.blobs[i].pts;
        
            ofBeginShape();
            for (int j = 0; j < blob.size(); j++) {
                ofVertex(blob[j]);
            }
            ofEndShape();
        }
        ofPopStyle();
    }
    if(bCalibrate) {
        ofPushStyle();
        ofSetColor(0,0,0);
        ofRect(0, 0, ofGetWidth(), ofGetHeight());
    }
    ripples.end();
    ripples.update();
    
//    bounce << ripples;
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    if(bCalibrate) {
        ofPushMatrix();
//        ofTranslate(dx, dy);
//        ofScale(ofGetWidth()/kinect.width*dz,ofGetHeight()/kinect.height*dz);
//        input_image.draw(0,0);
        prettyContourFinder.draw(dx,dy, ofGetWidth()*dz, ofGetHeight()*dz);
//        input_image.draw(dx, dy, PROJECTOR_WIDTH*dz, PROJECTOR_HEIGHT*dz);
//        contourFinder.draw(dx, dy, ofGetWidth()*dz, ofGetHeight()*dz);
        ofPopMatrix();
    }
    else {
        ripples.draw(dx, dy, ofGetWidth()*dz, ofGetHeight()*dz);
    }
    
    stringstream reportStream;
    reportStream << "Framerate: " << ofToString(ofGetFrameRate()) << endl
    << "Frame: " << ofToString(ofGetFrameNum()) << endl
    << "Threshold: " << threshold << endl
    << "Contour area: " << min_contour_area << endl
    << "Damping: " << ripples.damping << endl
    << "Tilt Angle: " << angle << endl
    << "RGB: " << bFreezeRed << bFreezeGreen << bFreezeBlue << endl;
    if(display_feedback) {
        ofDrawBitmapString(reportStream.str(), 100, 600);
    }
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
        case 'b':
            b_learn_background = true;
            break;
        case '=':
            threshold++;
            break;
        case '-':
            threshold--;
            break;
        case 'c':
            bCalibrate = !bCalibrate;
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
        case '1':
            bFreezeRed = !bFreezeRed;
            break;
        case '2':
            bFreezeGreen = !bFreezeGreen;
            break;
        case '3':
            bFreezeBlue = !bFreezeBlue;
            break;
    }
}
