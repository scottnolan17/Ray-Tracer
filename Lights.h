#pragma once

#include "SceneObject.h"

// Spotlights and points lights that project light rays to illuminate the scene.
// Spotlights have a radius slider to choose how wide their cone of light is.
// Both classes have a function to verify if anything is blocking their light toward  a certain point.

//  A light source to light the scene
//
class Light : public SceneObject {
public:
	Light(glm::vec3 pos, float brightness) {
		position = pos;
		settings.setup();
		settings.add(intensity.setup("Light Intensity", brightness, 0.0, 20.0));
		isVisible = false;
	}
	Light() {
		position = glm::vec3(0, 5, 0);
		settings.setup();
		settings.add(intensity.setup("Light Intensity", 1.0, 0.0, 20.0));
		isVisible = true;
	}

	void draw() {
		ofSetColor(ofColor::lightYellow);
		ofDrawSphere(position, 0.3);
		ofSetColor(ofColor::white);
	}
	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) {
		return (glm::intersectRaySphere(ray.p, ray.d, position, 0.3, point, normal));
	}



	virtual bool isBlocked(glm::vec3, vector<SceneObject *>);

	ofxFloatSlider intensity;
};

//	A directional light source that projects its light in a cone
//
class Spotlight : public Light {
public:
	Spotlight(glm::vec3 pos, float brightness, glm::vec3 dir, float ang) {
		position = pos;
		settings.setup();
		settings.add(intensity.setup("Light Intensity", brightness, 0, 20));
		settings.add(direction.setup("Light Direction", dir, glm::vec3(-1, -1, -1), glm::vec3(1, 1, 1)));
		settings.add(angle.setup("Cone Angle", ang, 1, 45));
		isVisible = false;
	}
	Spotlight() {
		position = glm::vec3(0, 5, 0);
		settings.setup();
		settings.add(intensity.setup("Light Intensity", 0, 1.5, 20));
		settings.add(direction.setup("Light Direction", glm::vec3(0, -1, 0), glm::vec3(-1, -1, -1), glm::vec3(1, 1, 1)));
		settings.add(angle.setup("Cone Angle", 10, 1, 45));
		isVisible = true;
	}

	void draw() {
		glm::vec3 dir = direction;
		ofSetColor(ofColor::goldenRod);
		ofDrawSphere(position, 0.3);
		ofDrawArrow(position, position + (glm::normalize(dir) * 0.6), 0.2);
		ofSetColor(ofColor::white);
	}

	//void setDirection(glm::vec3 newDir) { direction = glm::normalize(newDir); }

	bool isBlocked(glm::vec3, vector<SceneObject *>);

	ofxVec3Slider direction;
	ofxFloatSlider angle;
};
