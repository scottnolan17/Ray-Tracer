//
//  SJSU CS 116A F2020
//  Ray tracer using Lambert and Blinn-Phong shading
//  illuminated by fixed light sources to render output
//  image files. Currently only geometric shapes.
//
//  - Scott Nolan 2020
//
#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "glm/gtx/intersect.hpp"

//  General Purpose Ray class 
//
class Ray {
public:
	Ray(glm::vec3 p, glm::vec3 d) { this->p = p; this->d = d; }
	void draw(float t) { ofDrawLine(p, p + t * d); }

	glm::vec3 evalPoint(float t) {
		return (p + t * d);
	}

	glm::vec3 p, d;
};

//  Base class for any renderable object in the scene
//
class SceneObject {
public: 
	virtual void draw() = 0;    // pure virtual funcs - must be overloaded
	virtual bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) { cout << "SceneObject::intersect" << endl; return false; }
	virtual ofColor getColorAt(glm::vec3 point) { return diffuseColor; }
	
	// any data common to all scene objects goes here
	glm::vec3 position = glm::vec3(0, 0, 0);

	// material properties (we will ultimately replace this with a Material class - TBD)
	//
	ofColor diffuseColor = ofColor::grey;    // default colors - can be changed.
	ofColor specularColor = ofColor::lightGray;
};

//  A light source to light the scene
//
class Light : public SceneObject {
public:
	Light(glm::vec3 pos, float brightness) { position = pos; intensity = brightness; }
	Light() { position = glm::vec3(0, 5, 0); intensity = 1.0; }

	void draw() { 
		ofSetColor(ofColor::lightYellow);
		ofDrawSphere(position, 0.3);  
		ofSetColor(ofColor::white);
	}
	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) { return false; }  // don't want to include the light sources in a rendering

	virtual bool isBlocked(glm::vec3, vector<SceneObject *>);

	float intensity = 1.0;
};

//	A directional light source that projects its light in a cone
//
class Spotlight : public Light {
public:
	Spotlight(glm::vec3 pos, glm::vec3 dir, float brightness, float coneAngle) { position = pos; direction = glm::normalize(dir); intensity = brightness; angle = coneAngle; }
	Spotlight() { position = glm::vec3(0, 5, 0); direction = glm::vec3(0, -1, 0), intensity = 1.0; angle = 45;  }

	void draw() {
		ofSetColor(ofColor::goldenRod);
		ofDrawSphere(position, 0.3);
		ofDrawArrow(position, position + (direction * 0.6), 0.2);
		ofSetColor(ofColor::white);
	}
	
	void setDirection(glm::vec3 newDir) { direction = glm::normalize(newDir); }

	bool isBlocked(glm::vec3, vector<SceneObject *>);

	glm::vec3 direction;
	float angle;
};

//  General purpose sphere  (assume parametric)
//
class Sphere: public SceneObject {
public:
	Sphere(glm::vec3 p, float r, ofColor diffuse = ofColor::lightGray) { position = p; radius = r; diffuseColor = diffuse; }
	Sphere() {}
	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal); //{
	//	return (glm::intersectRaySphere(ray.p, ray.d, position, radius, point, normal));
	//}
	void draw()  { 
		ofDrawSphere(position, radius); 
	}

	float radius = 1.0;
};

//  Mesh class (will complete later- this will be a refinement of Mesh from Project 1)
//
class Mesh : public SceneObject {
	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) { return false;  }
	void draw() { }
};


//  General purpose plane 
//
class Plane: public SceneObject {
public:
	Plane(glm::vec3 p, glm::vec3 n, ofColor diffuse = ofColor::darkOliveGreen, float w = 20, float h = 20 ) {
		position = p; 
		setNormal(n);
		width = w;
		height = h;
		diffuseColor = diffuse;
		plane.rotateDeg(90, 1, 0, 0);
	}
	Plane() { }
	bool intersect(const Ray &ray, glm::vec3 & point, glm::vec3 & normal);
	void draw() {
		plane.setPosition(position);
		plane.setWidth(width);
		plane.setHeight(height);
		plane.setResolution(4, 4);
		plane.drawWireframe();

		ofSetColor(ofColor::red);
		ofDrawArrow(position, position + normal);
		ofSetColor(ofColor::white);
		ofDrawArrow(position, position + basis1);
		ofDrawArrow(position, position + basis2);
	}
	void setTexture(ofImage image) {
		texture = image;
		hasTexture = true;
	}
	ofImage getTexture() {
		return texture;
	}
	void setNormal(glm::vec3 norm) {
		normal = glm::normalize(norm);

		// create a pair of vectors linearly independent from the normal and each other...
		glm::vec3 x1 = glm::normalize(glm::vec3(-normal.y, normal.x, 0));	
		if (glm::length(glm::vec3(-normal.y, normal.x, 0)) == 0) x1 = glm::normalize(glm::vec3(0, -normal.z, normal.y));
		glm::vec3 x2 = glm::normalize(glm::vec3(normal.z, 0, -normal.x));
		if (glm::length(glm::vec3(normal.z, 0, -normal.x)) == 0) x2 = glm::normalize(glm::vec3(0, -normal.z, normal.y));

		// Use Gram-Schmidt to make them into orthogonal bases. Do this once and store it in the plane's structure rather than every time a ray is cast.
		basis1 = x1 - (glm::dot(x1, normal) * normal);
		basis2 = x2 - (glm::dot(x2, normal) * normal) - (glm::dot(x2, basis1) * basis1);
		basis1 = glm::normalize(basis1);
		basis2 = glm::normalize(basis2);


	}
	glm::vec3 getNormal() {
		return normal;
	}

	ofColor getColorAt(glm::vec3);

	ofPlanePrimitive plane;
	float width = 20;
	float height = 20;
	bool bInfinite = true;

private:
	bool hasTexture = false;	// no texture by default
	ofImage texture;

	glm::vec3 normal = glm::vec3(0, 1, 0);
	glm::vec3 basis1 = glm::vec3(1, 0, 0);
	glm::vec3 basis2 = glm::vec3(0, 0, 1);
};

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
		ofColor lambert(const glm::vec3&, const glm::vec3&, ofColor);
		ofColor phong(const glm::vec3&, const glm::vec3&, const ofColor, const ofColor, float);

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
		vector<Light *> lights;

		ofxFloatSlider lightIntensity;
		ofxFloatSlider lightFalloff;
		ofxFloatSlider phongPower;
		ofxFloatSlider ambientStrength;
		ofxPanel gui;

		int imageWidth = 1200;
		int imageHeight = 800;
};
 