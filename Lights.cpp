#include "Lights.h"

// Checks if the line segment between the given point and the light is blocked by any of the given SceneObjects
//
bool Light::isBlocked(glm::vec3 surfacePoint, vector<SceneObject *> sceneObjs) {
	glm::vec3 intersectPt, normal;
	Ray ray = Ray(position, glm::normalize(surfacePoint - position));
	for (SceneObject *obj : sceneObjs) {
		if (obj->isVisible && obj->intersect(ray, intersectPt, normal) && glm::distance(intersectPt, position) < glm::distance(surfacePoint, position)) return true;
	}
	return false;
}

// Checks if the line segment between the given point and the spotlight is blocked by any of the given SceneObjects or is outside the light cone
//
bool Spotlight::isBlocked(glm::vec3 surfacePoint, vector<SceneObject *> sceneObjs) {
	glm::vec3 intersectPt, normal;
	glm::vec3 dir = direction;
	float ang = angle;
	Ray ray = Ray(position, glm::normalize(surfacePoint - position));
	if (glm::angle(glm::normalize(dir), glm::normalize(surfacePoint - position)) > glm::radians(ang)) return true;
	for (SceneObject *obj : sceneObjs) {
		if (obj->isVisible && obj->intersect(ray, intersectPt, normal) && glm::distance(intersectPt, position) < glm::distance(surfacePoint, position)) return true;
	}
	return false;
}
