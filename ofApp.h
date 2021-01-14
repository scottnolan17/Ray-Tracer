//
//  SJSU CS 116A F2020
//  Ray tracer using Lambert and Blinn-Phong shading, and 
//  Gouraud normal interpolation. Illuminated by movable light 
//  sources (point or spotlight) to render outputimage files. 
//  Can process .obj files into modifiable meshes, or create 
//  and modify geometric shapes.
//
//  - Scott Nolan 2020
//
#pragma once

#include "ofMain.h"
#include "Mesh.h"
#include "Shapes.h"
#include "Lights.h"


// view plane for render camera
// 
class  ViewPlane: public Plane {
public:
	ViewPlane(glm::vec2 p0, glm::vec2 p1) { min = p0; max = p1; }

	ViewPlane() {                         // create reasonable defaults (6x4 aspect)
		min = glm::vec2(-3, -2);
		max = glm::vec2(3, 2);
		position = glm::vec3(0, 0, 5);
		setNormal(glm::vec3(0, 0, 1));      // viewplane currently limited to Z axis orientation
	}

	void setSize(glm::vec2 min, glm::vec2 max) { this->min = min; this->max = max; }
	float getAspect() { return width() / height(); }

	glm::vec3 toWorld(float u, float v);   //   (u, v) --> (x, y, z) [ world space ]

	void draw() {
		ofDrawRectangle(glm::vec3(min.x, min.y, position.z), width(), height());
	}

	
	float width() {
		return (max.x - min.x);
	}
	float height() {
		return (max.y - min.y); 
	}

	// some convenience methods for returning the corners
	//
	glm::vec2 topLeft() { return glm::vec2(min.x, max.y); }
	glm::vec2 topRight() { return max; }
	glm::vec2 bottomLeft() { return min;  }
	glm::vec2 bottomRight() { return glm::vec2(max.x, min.y); }

	//  To define an infinite plane, we just need a point and normal.
	//  The ViewPlane is a finite plane so we need to define the boundaries.
	//  We will define this in terms of min, max  in 2D.  
	//  (in local 2D space of the plane)
	//  ultimately, will want to locate the ViewPlane with RenderCam anywhere
	//  in the scene, so it is easier to define the View rectangle in a local'
	//  coordinate system.
	//
	glm::vec2 min, max;
};


//  render camera  - currently must be z axis aligned (we will improve this in project 4)
//
class RenderCam: public SceneObject {
public:
	RenderCam() {
		position = glm::vec3(0, 0, 10);
		aim = glm::vec3(0, 0, -1);
	}
	Ray getRay(float u, float v);
	void draw() { ofDrawBox(position, 1.0); };
	void drawFrustum();

	glm::vec3 aim;
	ViewPlane view;          // The camera viewplane, this is the view that we will render 
};


class ofApp : public ofBaseApp{

	public:

		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		void rayTrace();
		void drawGrid() { ofDrawGrid(); }
		void drawAxis(glm::vec3);
		bool mouseToDragPlane(int x, int y, glm::vec3 &point);
		bool objSelected() { return (selected.size() ? true : false); };
		ofColor lambert(const glm::vec3&, const glm::vec3&, ofColor);
		ofColor phong(const glm::vec3&, const glm::vec3&, const ofColor, const ofColor, float);

		bool bDrag = false;
		bool bHide = true;
		bool bShowImage = false;

		ofEasyCam  mainCam;
		ofCamera sideCam;
		ofCamera previewCam;
		ofCamera  *theCam;    // set to current camera either mainCam or sideCam

		// set up one render camera to render image throughn
		//
		RenderCam renderCam;
		ofImage image;

		vector<SceneObject *> scene;
		vector<SceneObject *> selected;
		vector<Light *> lights;

		ofxFloatSlider lightFalloff;
		ofxFloatSlider phongPower;
		ofxFloatSlider ambientStrength;
		ofxPanel gui;

		ofxPanel *display;

		int imageWidth = 1200;
		int imageHeight = 800;

		glm::vec3 lastPoint;
};
 