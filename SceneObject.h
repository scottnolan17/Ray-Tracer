#pragma once

#include "ofxGui.h"
#include "glm/gtx/intersect.hpp"
#include "Ray.h"

// Parent class of spheres, planes, spotlights, point lights, and meshes.
// Must be able to draw itself to the OF view window, and check for intersection with a ray.
// Have a modifiable diffuse color and specular color.

//  Base class for any renderable object in the scene
//
class SceneObject {
public:
	virtual void draw() = 0;    // pure virtual funcs - must be overloaded
	virtual bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) { cout << "SceneObject::intersect" << endl; return false; }
	virtual ofColor getColorAt(glm::vec3 point) { return diffuseColor; }

	// any data common to all scene objects goes here
	glm::vec3 position = glm::vec3(0, 0, 0);

	bool isSelectable = true;
	bool isVisible = true;

	ofxPanel settings;

	ofxColorSlider diffuseColor;
	ofxColorSlider specularColor;


	// reused from skeleton builder project:
	//
	// Generate a rotation matrix that rotates v1 to v2
	// v1, v2 must be normalized
	//
	glm::mat4 rotateToVector(glm::vec3 v1, glm::vec3 v2) {
		if (v1 == v2) return glm::mat4(1.0);
		glm::vec3 axis = glm::cross(v1, v2);
		glm::quat q = glm::angleAxis(glm::angle(v1, v2), glm::normalize(axis));
		return glm::toMat4(q);
	}

};

