#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetFrameRate(60);
    ofSetWindowTitle("BounceBall");
    ofSetVerticalSync(true);
    ofBackground(10, 10, 10);
    ofDisableArbTex();
    
    camera.setPosition(ofVec3f(0, -4.f, -10.f));
    camera.lookAt(ofVec3f(0, 0, 0), ofVec3f(0, -1, 0));
    camera.setDistance(10);
    camera.setNearClip(0.1);
    camera.setFarClip(300);
    
    world.setup();
    world.setCamera(&camera);
    
    anisotropy = 4.;
    
    float fboDiv = 4.f;
    
    fbo.allocate((float)ofGetWidth() / fboDiv, (float)ofGetHeight() / fboDiv, GL_RGB, 4);
    fbo.begin();{
        ofClear(0, 0, 0, 255);
        ofSetColor(255, 243, 160, 255);
        ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
        ofSetColor(120, 140, 150, 255);
        
        int numIterations = 4;
        float inc = (float)fbo.getWidth() / ((float)numIterations);
        for(int i = 0; i < numIterations; i++){
            float tx = (float)i*inc + inc;
            float ty = (float)i*inc + inc;
            
            ofSetColor(238, 255, 209, 255);
            ofSetLineWidth(3.5);
            if(i % 2 == 0) ofSetLineWidth(0.5);
            
            ofDrawLine(tx, 0, tx, fbo.getHeight());
            ofDrawLine(0, ty, fbo.getWidth(), ty);
        }
    }fbo.end();
    
    ofSetLineWidth(3);
    
    fbo.getTexture().setTextureMinMagFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
    
    fbo.getTexture().bind();
    glGenerateMipmap(fbo.getTexture().texData.textureTarget);
    fbo.getTexture().unbind();
    
    fboSphere.allocate(256, 256, GL_RGB, 4);
    fboSphere.begin();{
        ofColor(0, 0, 0, 255);
        ofSetColor(255, 158, 209);
        ofDrawRectangle(0, 0, fboSphere.getWidth(), fboSphere.getHeight());
        ofSetColor(112, 30, 73, 255);
        ofSetLineWidth(5);
        ofDrawLine(0, 0, fboSphere.getWidth(), fboSphere.getHeight());
        ofDrawLine(0, fboSphere.getHeight(), fboSphere.getWidth(), 0);
    }fboSphere.end();
    
    ofSetLineWidth(1);
    
    omesh = ofMesh::plane(20, 20, 20, 20, OF_PRIMITIVE_TRIANGLES);
    ofQuaternion rquat;
    rquat.makeRotate(30, 20, 0, 0);
    ofSeedRandom();
    float rseed = ofRandom(0, 10000);
    vector<glm::vec3>& verts = omesh.getVertices();
    for(int i = 0; i < verts.size(); i++){
        verts[i] = rquat * verts[i];
        verts[i].y = ofSignedNoise(verts[i].x*0.02, verts[i].y*0.02 + verts[i].z*0.02, ofGetElapsedTimef()*0.1 + rseed)*10;
    }
    
    vector<glm::vec2>& tcoods = omesh.getTexCoords();
    for(int i = 0; i < tcoods.size(); i++){
        tcoods[i].x *= 4.f;
        tcoods[i].y *= 4.f;
    }
    mesh = omesh;
    
    bulletMesh = shared_ptr<ofxBulletTriMeshShape>( new ofxBulletTriMeshShape() );
    bulletMesh->create(world.world, mesh, ofVec3f(0, 0, 0), 0.f, ofVec3f(-10000, -10000, -10000), ofVec3f(10000, 10000, 10000));
    bulletMesh->add();
    bulletMesh->enableKinematic();
    bulletMesh->setActivationState(DISABLE_DEACTIVATION);
    
    bDrawDebug = false;
    bSpacebar = false;
    bDrawFbos = false;
    bAnimated = false;

}

//--------------------------------------------------------------
void ofApp::update(){
    if(bSpacebar){
        shared_ptr<ofxBulletSphere> sphere( new ofxBulletSphere() );
        float trad = fabs(sin( ofGetElapsedTimef() )*5);
        sphere->create(world.world, ofVec3f(cos( ofGetElapsedTimef()*10. )*trad, -6, sin(ofGetElapsedTimef()*10)*trad), 1., 0.75);
        sphere->add();
        bulletSpheres.push_back(sphere);
        bSpacebar = false;
    }
    
    for(int i = 0; i < bulletSpheres.size(); i++){
        ofVec3f spos = bulletSpheres[i]->getPosition();
        if(spos.y > 5){
            bulletSpheres.erase(bulletSpheres.begin() + i);
            break;
        }
    }
    
    if(bAnimated){
        vector<glm::vec3>& verts = mesh.getVertices();
        vector<glm::vec3>& overts = omesh.getVertices();
        for(int i = 0; i < verts.size(); i++){
            verts[i].y = ofSignedNoise(verts[i].x*0.025, verts[i].y*0.025 + verts[i].z*0.025, ofGetElapsedTimef()*0.75)*3;
        }
        bulletMesh->updateMesh(world.world, mesh);
    }
    
    world.update(ofGetLastFrameTime(), 12);
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofEnableDepthTest();
    ofBackgroundGradient(ofColor(247, 210, 230), ofColor(191, 118, 157), OF_GRADIENT_CIRCULAR);
    
    camera.begin();
    
    ofSetLineWidth(1.f);
    if(bDrawDebug) world.drawDebug();
    else{
        ofSetColor(255, 255, 255, 255);
        fbo.getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
        fbo.getTexture().bind();
        glSamplerParameterf(fbo.getTexture().texData.textureTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
        mesh.draw();
        fbo.getTexture().unbind();
        
        fboSphere.getTexture().bind();
        for(int i = 0; i < bulletSpheres.size(); i++){
            bulletSpheres[i]->draw();
        }
        fboSphere.getTexture().unbind();
    }
    
    if(bDrawDebug){
        ofDrawAxis(0.5);
    }
    
    camera.end();
    
    ofDisableDepthTest();
    
    ofSetColor(255);
    if(bDrawFbos){
        fbo.draw(ofGetWidth() - 20 - fbo.getWidth(), 20);
        fboSphere.draw(ofGetWidth() - 20 - fboSphere.getWidth(), 40 + fbo.getHeight());
    }
    
    ofSetColor(255, 255, 255);
    stringstream ss;
    ss << "framerate: " << ofToString(ofGetFrameRate(), 0) << endl;
    ss << "draw debug (d): " << ofToString(bDrawDebug, 0) << endl;
    ss << "draw textures (e): " << ofToString(bDrawFbos, 0) << endl;
    ss << "add sphere (spacebar): " << ofToString(bulletSpheres.size(), 0) << endl;
    ss << "animated (a): " << ofToString(bAnimated, 0) << endl;
    ofDrawBitmapString(ss.str().c_str(), 20, 20);
}

//--------------------------------------------------------------
void ofApp::exit(){
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch (key){
        case ' ':
            bSpacebar = true;
            break;
        case 'd':
            bDrawDebug = !bDrawDebug;
            break;
        case 'e':
            bDrawFbos = !bDrawFbos;
        case 127:
            bulletSpheres.clear();
            break;
        case 'a':
            bAnimated = !bAnimated;
            break;
                default:
                    break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
