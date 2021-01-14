#include "Shapes.h"

// Positive-only modulo, because the default fmod() can return negative results
//
float pos_mod(float n, float d) {
	float result = fmod(n, d);
	return result < 0.0 ? result + d : result;
}

// Return the color at the given point (in world space); either a flat diffuse color or part of a texture
//
ofColor Plane::getColorAt(glm::vec3 point) {
	if (!hasTexture) return diffuseColor;
	glm::vec3 relOrigin = position;		// relative origin, in other words, where the texture starts from
	if (!bInfinite) {
		relOrigin = relOrigin + (basis1 * (height / 2)) - (basis2 * (width / 2));	// if it's a finite plane, start drawing from the corner
	}
	return texture.getColor(
		pos_mod(floor(glm::dot((point - relOrigin), (basis2 * 80))), texture.getWidth()),
		texture.getHeight() - 1 - pos_mod(floor(glm::dot((point - relOrigin), (basis1 * 80))), texture.getHeight())
	);
	// ^ This looks like a complete mess but it's actually not too bad: 
	//		basis*80 turns the basis from being 1 OF unit long, which is kind of big, into being 80 times smaller
	//		point - relOrigin is the vector to the intersect point from the relative origin
	//		glm::dot() is how you get subspace coordinates given a basis vector and a vector on the subspace (i.e. the location of the pixels on the image source file)
	//		floor() so we don't accidentally get any overflow
	//		pos_mod() so that the image repeats in a tile pattern
	//		texture.getheight() - 1 so the image isn't upside down
}

// Intersect Ray with Plane  (wrapper on glm::intersectRayPlane)
//
bool Plane::intersect(const Ray &ray, glm::vec3 & point, glm::vec3 & normalAtIntersect) {
	float dist;
	bool hit = glm::intersectRayPlane(ray.p, ray.d, position, this->normal, dist);
	Ray r = ray;
	glm::vec3 poi = r.evalPoint(dist);
	if (!bInfinite) {		// only for finite planes
		glm::vec3 relativePoi = poi - position;						// vector position relative to plane position
		glm::vec3 b1Proj = glm::dot(relativePoi, basis1) * basis1;		// Projection of the relative point of intersection (poi) onto
		glm::vec3 b2Proj = glm::dot(relativePoi, basis2) * basis2;		// the two bases of the plane.

		hit = hit && glm::length(b1Proj) <= height / 2 && glm::length(b2Proj) <= width / 2;		// if either of these projections is too long, the poi must be off the finite plane
	}
	if (hit) {
		point = poi;
		normalAtIntersect = this->normal;
	}
	return (hit);
}

// Intersect Ray with Sphere 
//
bool Sphere::intersect(const Ray &ray, glm::vec3 & point, glm::vec3 & normalAtIntersect) {
	float dist;
	float rad = radius;
	bool hit = glm::intersectRaySphere(ray.p, ray.d, position, glm::pow2(rad), dist);
	if (hit) {
		Ray r = ray;
		point = r.evalPoint(dist);
		normalAtIntersect = glm::normalize(point - position);
	}
	return (hit);
}