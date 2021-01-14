#pragma once
#include "ofMain.h"

//  General Purpose Ray class 
//
class Ray {
public:
	Ray(glm::vec3 p, glm::vec3 d) {
		this->p = p;
		this->d = d;
		inv_d = glm::vec3(1 / d.x, 1 / d.y, 1 / d.z);
		sign[0] = (inv_d.x < 0);
		sign[1] = (inv_d.y < 0);
		sign[2] = (inv_d.z < 0);
	}
	void draw(float t) { ofDrawLine(p, p + t * d); }

	glm::vec3 evalPoint(float t) {
		return (p + t * d);
	}

	glm::vec3 p, d, inv_d;
	int sign[3];
};