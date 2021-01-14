#pragma once

#include "SceneObject.h"

// Simple geopmetric spheres and planes for ray tracing.
// Planes may be infinte or finite, and can have texture applied to their surface.
// Spheres have an adjustable radius.

//  General purpose sphere  (assume parametric)
//
class Sphere : public SceneObject {
public:
	Sphere(glm::vec3 p, float rad, ofColor col = ofColor::grey) {
		position = p;
		settings.setup();
		settings.add(radius.setup("Radius", rad, 0.1, 10.0));
		settings.add(diffuseColor.setup("Diffuse Color", col, ofColor::black, ofColor::white));
		settings.add(specularColor.setup("Specular Color", ofColor::white, ofColor::black, ofColor::white));
	}
	Sphere() {}
	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal); //{
	//	return (glm::intersectRaySphere(ray.p, ray.d, position, radius, point, normal));
	//}
	void draw() {
		ofDrawSphere(position, radius);
	}

	ofxFloatSlider radius;
};

//  General purpose plane 
//
class Plane : public SceneObject {
public:
	Plane(glm::vec3 p, glm::vec3 n, float w = 5, float h = 5, ofColor col = ofColor::grey) {
		position = p;
		setNormal(n);
		settings.setup();
		settings.add(width.setup("Width", w, 1, 30));
		settings.add(height.setup("Height", h, 1, 30));
		settings.add(diffuseColor.setup("Diffuse Color", col, ofColor::black, ofColor::white));
		settings.add(specularColor.setup("Specular Color", ofColor::white, ofColor::black, ofColor::white));
		plane.rotateDeg(90, 1, 0, 0);
	}
	Plane() { }
	bool intersect(const Ray &ray, glm::vec3 & point, glm::vec3 & normal);
	void draw() {
		ofPushMatrix();
		glm::mat4 m = glm::translate(position);
		m *= rotateToVector(glm::vec3(0, 1, 0), normal);
		ofMultMatrix(m);
		plane.setWidth(width);
		plane.setHeight(height);
		plane.setResolution(4, 4);
		plane.drawWireframe();
		ofPopMatrix();

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
	ofxFloatSlider width;
	ofxFloatSlider height;
	bool bInfinite = false;

private:
	bool hasTexture = false;	// no texture by default
	ofImage texture;

	glm::vec3 normal = glm::vec3(0, 1, 0);
	glm::vec3 basis1 = glm::vec3(1, 0, 0);
	glm::vec3 basis2 = glm::vec3(0, 0, 1);
};

