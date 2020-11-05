#include "ofApp.h"


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
		relOrigin = relOrigin + (basis1 * height / 2) - (basis2 * width / 2);	// if it's a finite plane, start drawing from the corner
	}
	return texture.getColor(
		pos_mod(floor(glm::dot( (point - relOrigin), (basis2*80) )), texture.getWidth()),
		texture.getHeight() - 1 - pos_mod(floor(glm::dot( (point - relOrigin), (basis1*80) )), texture.getHeight())
);
	// ^ This looks like a complete mess but it's actually not too bad: 
	//		basis*80 turns the basis from being 1 OF unit long, which is kind of big, into being 80 times smaller
	//		point - relOrigin is the vector to the intersect point from the relative origin
	//		glm::dot() is how you get subspace coordinates given a basis vector and a vector on the subspace (i.e. the location of the pixels on the image source file)
	//		floor() so we don't accidentally get any overflow
	//		pos_mod() so that the image repeats in a tile pattern
	//		texture.getheight() - 1 so the image isn't upside down
}

// Intersect Ray with Plane  (wrapper on glm::intersect*
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
	bool hit = glm::intersectRaySphere(ray.p, ray.d, position, glm::pow2(radius), dist);
	if (hit) {
		Ray r = ray;
		point = r.evalPoint(dist);
		normalAtIntersect = glm::normalize(point - position);
	}
	return (hit);
}


// Convert (u, v) to (x, y, z) 
// We assume u,v is in [0, 1]
//
glm::vec3 ViewPlane::toWorld(float u, float v) {
	float w = width();
	float h = height();
	return (glm::vec3((u * w) + min.x, (v * h) + min.y, position.z));
}

// Get a ray from the current camera position to the (u, v) position on
// the ViewPlane
//
Ray RenderCam::getRay(float u, float v) {
	glm::vec3 pointOnPlane = view.toWorld(u, v);
	return(Ray(position, glm::normalize(pointOnPlane - position)));
}

// Draw the viewplane as a rectangle
//
void RenderCam::drawFrustum() {
	view.draw();
	Ray r1 = getRay(0, 0);
	Ray r2 = getRay(0, 1);
	Ray r3 = getRay(1, 1);
	Ray r4 = getRay(1, 0);
	float dist = glm::length((view.toWorld(0, 0) - position));
	r1.draw(dist);
	r2.draw(dist);
	r3.draw(dist);
	r4.draw(dist);
}

// Cast rays out from the camera's perspective to create an image output to a file called...
//
void ofApp::rayTrace() {
	float u, v;
	glm::vec3 intersectPt, normal;

	for (int i = 0; i < imageWidth; i++) {
		for (int j = 0; j < imageHeight; j++) {
			u = (i + 0.5) / imageWidth;
			v = (j + 0.5) / imageHeight;

			Ray ray = renderCam.getRay(u, v);
			float distance = numeric_limits<float>::infinity();
			float closest = distance;
			ofColor color = ofColor::darkGray;	// default to dark grey if no objects are hit by the ray

			for (SceneObject *obj : scene) {
				if (obj->intersect(ray, intersectPt, normal)) {
					if (glm::distance(ray.p, intersectPt) < closest) {	// new closest object
						closest = glm::distance(ray.p, intersectPt);
						color = phong(intersectPt, normal, obj->getColorAt(intersectPt), obj->specularColor, phongPower);

					}
				}
			}
			image.setColor(i, imageHeight - j - 1, color);
		}
	}

	image.update();

	image.save("raytraced.png");
	cout << "ray trace successful: output saved as bin/data/raytraced.png" << endl;
	bShowImage = true;

}

// Apply a shiny Blinn-Phong shader effect to a color, given the point on the scene object, the normal, the unshaded color (diffuse), the highlight color (specular), and the strength of the effect
//
ofColor ofApp::phong(const glm::vec3 &p, const glm::vec3 &norm, const ofColor diffuse, const ofColor specular, float power) {	ofColor result = lambert(p, norm, diffuse);	for (Light *l : lights) {
		if (!l->isBlocked(p + (norm * 0.01), scene)) {
			result += specular * (l->intensity / lightFalloff) * glm::pow(max((float)0, glm::dot(norm, glm::normalize(l->position - p + renderCam.position - p))), power);
		}
	}	return result;}

// Apply a matte Lambert shading effect to a color, given the point on the scene object, the normal, and the unshaded color (diffuse)
//
ofColor ofApp::lambert(const glm::vec3 &p, const glm::vec3 &norm, const ofColor diffuse) {
	ofColor result = diffuse * (ambientStrength);	// ambient light level
	for (Light *l : lights) {
		if (!l->isBlocked(p + (norm * 0.01), scene)) {
			result += diffuse * (l->intensity / lightFalloff) * max((float)0, glm::dot(norm, glm::normalize(l->position - p)));
		}
	}
	return result;
}

// Checks if the line segment between the given point and the light is blocked by any of the given SceneObjects
//
bool Light::isBlocked(glm::vec3 surfacePoint, vector<SceneObject *> sceneObjs) {
	glm::vec3 intersectPt, normal;
	Ray ray = Ray(position, glm::normalize(surfacePoint-position));
	for (SceneObject *obj : sceneObjs) {
		if (obj->intersect(ray, intersectPt, normal) && glm::distance(intersectPt, position) < glm::distance(surfacePoint, position) ) return true; 
	}
	return false;
}

// Checks if the line segment between the given point and the spotlight is blocked by any of the given SceneObjects or is outside the light cone
//
bool Spotlight::isBlocked(glm::vec3 surfacePoint, vector<SceneObject *> sceneObjs) {
	glm::vec3 intersectPt, normal;
	Ray ray = Ray(position, glm::normalize(surfacePoint - position));
	if (glm::angle(direction, glm::normalize(surfacePoint - position)) > glm::radians(angle)) return true;
	for (SceneObject *obj : sceneObjs) {

		if (obj->intersect(ray, intersectPt, normal) && glm::distance(intersectPt, position) < glm::distance(surfacePoint, position)) return true;
	}
	return false;
}

//--------------------------------------------------------------
void ofApp::setup() {
	image.allocate(imageWidth, imageHeight, OF_IMAGE_COLOR);
	
	mainCam.setDistance(30);
	mainCam.lookAt(glm::vec3(0, 0, 0));
	sideCam.setPosition(glm::vec3(renderCam.position.z * -1, renderCam.position.y, renderCam.position.x));
	sideCam.lookAt(renderCam.aim);
	previewCam.setPosition(renderCam.position);
	previewCam.lookAt(renderCam.aim);

	theCam = &mainCam;

	ofSetBackgroundColor(ofColor::black);

	Plane *floorPlane = new Plane(glm::vec3(0, -1, 0), glm::vec3(0.05, 1, 0.1), ofColor::lightBlue);
	floorPlane->setTexture(ofImage("07_wood grain PBR texture _seamless/07_wood grain PBR texture.jpg"));
	Plane *backdropPlane = new Plane(glm::vec3(0, 5, -32), glm::vec3(0, 0, 1), ofColor::grey);
	backdropPlane->setTexture(ofImage("26_white bricks texture-seamless.jpg"));
	//Plane *picturePlane = new Plane(glm::vec3(-7, 4.5, -12), glm::vec3(0.6, 0.2, 1), ofColor::grey);
	//picturePlane->width = 7.6;
	//picturePlane->height = 4.8;
	//picturePlane->bInfinite = false;
	//picturePlane->setTexture(ofImage("persistenceofmemory1931.jpg"));

	scene.push_back(floorPlane);
	//scene.push_back(picturePlane);
	scene.push_back(backdropPlane);
	//scene.push_back(new Plane(glm::vec3(0, -1, 0), glm::vec3(0, 1, 0), ofColor::lightBlue));	// openframeworks seems to have implemented
	scene.push_back(new Sphere(glm::vec3(3, 1.4, -2), 2.5, ofColor::chartreuse));					// a million different colors; might as well
	scene.push_back(new Sphere(glm::vec3(-3.5, 1.7, -4), 2, ofColor::darkMagenta));				// use some of the fun ones...
	scene.push_back(new Sphere(glm::vec3(0, 2, -8), 2.2, ofColor::orangeRed));
	//scene.push_back(new Sphere(glm::vec3(-7, 4, -7), 1.5, ofColor::ivory));
	//scene.push_back(new Sphere(glm::vec3(130, 70, -245), 8, ofColor::coral));
	//scene.push_back(new Sphere(glm::vec3(0, 1.5, 0), 2.5, ofColor::chartreuse));					


	//lights.push_back(new Light(glm::vec3(-10, 8, 10), 1.0));
	lights.push_back(new Light(glm::vec3(10, 4.5, 12), 1.0));
	//lights.push_back(new Light(glm::vec3(-10, 3, -6), 1.0));
	lights.push_back(new Spotlight(glm::vec3(-10, 12, 10), glm::vec3(10, -9, -10), 1.5, 20.0));	// pointed at the origin
	//lights.push_back(new Spotlight(glm::vec3(4, 7, 10), glm::vec3(-6, -10, -12), 1.0, 20.0));
	//lights.push_back(new Spotlight(glm::vec3(-8, 3, 10), glm::vec3(8, -2, -10), 1.0, 25.0));
	//lights.push_back(new Spotlight(glm::vec3(0, 15, 3), glm::vec3(0.2, -3, -1.4), 1.1, 20.0));
	//lights.push_back(new Spotlight(glm::vec3(-4, 0, 8), glm::vec3(5, 1, -8), 1.0, 20.0));
	//lights.push_back(new Spotlight(glm::vec3(4, 0, 8), glm::vec3(-5, 1, -8), 1.0, 20.0));
	for (Light *l: lights) scene.push_back(l);

	gui.setup();
	gui.add(lightIntensity.setup("Light Intensity", 1.0, 0.0, 20.0));
	gui.add(lightFalloff.setup("Light Falloff", 1.0, 1.0, 10.0));
	gui.add(phongPower.setup("Phong Power", 100, 1, 400));
	gui.add(ambientStrength.setup("Ambient Light Level", 0.3, 0.0, 1.0));

}

//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
void ofApp::draw() {
	theCam->begin();

	drawAxis(glm::vec3(0, 0, 0));
	ofSetColor(ofColor::white);
	for (SceneObject *obj : scene) {
		obj->draw();
	}
	ofDrawSphere(renderCam.position, 0.5);
	renderCam.drawFrustum();

	theCam->end();

	if (bShowImage) image.draw(glm::vec3(0, 0, 0), ofGetWindowHeight() / 4 * renderCam.view.getAspect(), ofGetWindowHeight() / 4);		// do a sort of picture-in-picture effect to display the rendered image
	else gui.draw();
}

void ofApp::drawAxis(glm::vec3 pos) {

	// X axis
	ofSetColor(ofColor::red);
	ofDrawLine(pos, glm::vec3(pos.x + 5, pos.y, pos.z));

	// Y axis
	ofSetColor(ofColor::green);
	ofDrawLine(pos, glm::vec3(pos.x, pos.y + 5, pos.z));

	// Z axis
	ofSetColor(ofColor::blue);
	ofDrawLine(pos, glm::vec3(pos.x, pos.y, pos.z + 5));
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	switch (key) {
	case '1':		// switch to front cam
		theCam = &previewCam;
		break;
	case '2':		// switch to side cam
		theCam = &sideCam;
		break;
	case '3':		// switch to easycam
		theCam = &mainCam;
		break;
	case 'r':		// render image
		for (Light *l : lights) l->intensity = lightIntensity;
		rayTrace();
		break;
	case ' ':		// toggle image overlay
		bShowImage = !bShowImage;
		break;

	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}

