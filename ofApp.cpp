#include "ofApp.h"


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

// Cast rays out from the camera's perspective to create an image output to a file called raytraced.png
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
				if (obj->isVisible && obj->intersect(ray, intersectPt, normal) ) {
					if (glm::distance(ray.p, intersectPt) < closest) {	// new closest object
						closest = glm::distance(ray.p, intersectPt);
						color = phong(intersectPt, normal, obj->getColorAt(intersectPt), obj->specularColor, phongPower);

					}
				}
			}
			image.setColor(i, imageHeight - j - 1, color);

		}

		cout << "Completed " << (i+1) * imageHeight << " pixels out of " << imageHeight * imageWidth << endl;

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

	Plane *floorPlane = new Plane(glm::vec3(0, -1, 0), glm::vec3(0, 1, 0), 20, 20);
	floorPlane->bInfinite = true;
	floorPlane->setTexture(ofImage("07_wood grain PBR texture _seamless/07_wood grain PBR texture.jpg"));
	Plane *backdropPlane = new Plane(glm::vec3(0, 5, -32), glm::vec3(0, 0, 1), 20, 20);
	backdropPlane->bInfinite = true;
	backdropPlane->setTexture(ofImage("2_Wallpaper PBR texture_seamless/2_Wallpaper PBR texture_seamless_DIFFUSE.jpg"));
	//Plane *picturePlane = new Plane(glm::vec3(-7, 4.5, -12), glm::vec3(0.6, 0.2, 1), ofColor::grey);
	//picturePlane->width = 7.6;
	//picturePlane->height = 4.8;
	//picturePlane->bInfinite = false;
	//picturePlane->setTexture(ofImage("persistenceofmemory1931.jpg"));

	scene.push_back(floorPlane);
	//scene.push_back(picturePlane);
	scene.push_back(backdropPlane);
	scene.push_back(new Sphere(glm::vec3(3, 1.4, -2), 2.5, ofColor::lime));									
	scene.push_back(new Sphere(glm::vec3(-3.5, 1.7, -4), 2, ofColor::magenta));								
	scene.push_back(new Sphere(glm::vec3(0, 2, -8), 1.5, ofColor::crimson));			


	lights.push_back(new Light(glm::vec3(10, 4.5, 12), 1.5));
	lights.push_back(new Spotlight(glm::vec3(-10, 12, 10), 1.5, glm::vec3(10, -12, -10), 10));	// pointed at the origin
	for (Light *l : lights) scene.push_back(l);

	gui.setup();
	gui.add(lightFalloff.setup("Light Falloff", 1.0, 1.0, 10.0));
	gui.add(phongPower.setup("Phong Power", 100, 1, 400));
	gui.add(ambientStrength.setup("Ambient Light Level", 0.3, 0.0, 1.0));

	display = &gui;

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
		if (objSelected() && obj == selected[0]) {	// draw selected objects in a different color
			ofSetColor(ofColor::salmon);
			obj->draw();
			ofSetColor(ofColor::white);
		}
		else obj->draw();
	}

	ofDrawSphere(renderCam.position, 0.5);
	renderCam.drawFrustum();

	theCam->end();

	if (bShowImage) image.draw(glm::vec3(0, 0, 0), ofGetWindowHeight() / 4 * renderCam.view.getAspect(), ofGetWindowHeight() / 4);		// do a sort of picture-in-picture effect to display the rendered image
	else display->draw();
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
	case 'C':		
	case 'c':		// toggle camera control
		if (mainCam.getMouseInputEnabled()) mainCam.disableMouseInput();
		else mainCam.enableMouseInput();
		break;
	case 'D':
	case 'd':		// remove the selected object from the scene
		if (objSelected()) {
			SceneObject *deadObj = (SceneObject *)selected[0];

			vector<SceneObject *>::iterator i = scene.begin();
			while (*i != deadObj && i != scene.end()) i++;	// find the object in the vector
			if (*i == deadObj) scene.erase(i);

			vector<Light *>::iterator l = lights.begin();	// make sure it gets deleted if it's a light source
			while (*l != deadObj && l != lights.end()) l++;		
			if (*l == deadObj) lights.erase(l);
		}
		selected.clear();
		display = &gui;
		break;
	case 'K':
	case 'k':		// add a new spotlight
		lights.push_back(new Spotlight(glm::vec3(0, 0, 0), 1.5, glm::vec3(0, -1, 0), 10));
		scene.push_back(lights.back());
		display = &scene.back()->settings;

		selected.clear();
		selected.push_back(scene.back());
		break;
	case 'L':
	case 'l':		// add a new point light
		lights.push_back(new Light(glm::vec3(0, 0, 0), 1.5));
		scene.push_back(lights.back());
		display = &scene.back()->settings;

		selected.clear();
		selected.push_back(scene.back());
		break;
	case 'P':
	case 'p':		// add a plane to the scene
		scene.push_back(new Plane(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));
		display = &scene.back()->settings;

		selected.clear();
		selected.push_back(scene.back());
		break;
	case 'R':
	case 'r':		// render image
		rayTrace();
		break;
	case 'S':
	case 's':		// add a sphere to the scene
		scene.push_back(new Sphere(glm::vec3(0, 0, 0), 1.5));
		display = &scene.back()->settings;

		selected.clear();
		selected.push_back(scene.back());
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
// reused from skeleton builder project:
void ofApp::mouseDragged(int x, int y, int button) {

	if (objSelected() && bDrag) {
		glm::vec3 point;
		mouseToDragPlane(x, y, point);
		selected[0]->position += (point - lastPoint);
		lastPoint = point;
	}

}

// reused from skeleton builder project:
//
//  This projects the mouse point in screen space (x, y) to a 3D point on a plane
//  normal to the view axis of the camera passing through the point of the selected object.
//  If no object selected, the plane passing through the world origin is used.
//
bool ofApp::mouseToDragPlane(int x, int y, glm::vec3 &point) {
	glm::vec3 p = theCam->screenToWorld(glm::vec3(x, y, 0));
	glm::vec3 d = p - theCam->getPosition();
	glm::vec3 dn = glm::normalize(d);

	float dist;
	glm::vec3 pos;
	if (objSelected()) {
		pos = selected[0]->position;
	}
	else pos = glm::vec3(0, 0, 0);
	if (glm::intersectRayPlane(p, dn, pos, glm::normalize(theCam->getZAxis()), dist)) {
		point = p + dn * dist;
		return true;
	}
	return false;
}

//--------------------------------------------------------------
// reused code from the skeleton builder project:
//
// Provides functionality of single selection and if something is already selected,
// sets up state for translation/rotation of object using mouse.
//
void ofApp::mousePressed(int x, int y, int button) {

	// if we are moving the camera around, don't allow selection
	//
	if (mainCam.getMouseInputEnabled()) return;

	// clear selection list
	//
	selected.clear();

	//
	// test if something selected
	//
	vector<SceneObject *> hits;
	vector<glm::vec3> pts;

	glm::vec3 p = theCam->screenToWorld(glm::vec3(x, y, 0));
	glm::vec3 d = p - theCam->getPosition();
	glm::vec3 dn = glm::normalize(d);

	// check for selection of scene objects
	//
	for (int i = 0; i < scene.size(); i++) {

		glm::vec3 point, norm;

		//  We hit an object
		//
		if (scene[i]->isSelectable && scene[i]->intersect(Ray(p, dn), point, norm)) {
			hits.push_back(scene[i]);
			pts.push_back(point);
		}
	}

	// if we selected more than one, pick nearest
	//
	SceneObject *selectedObj = NULL;
	if (hits.size() > 0) {
		selectedObj = hits[0];
		float nearestDist = std::numeric_limits<float>::infinity();
		for (int n = 0; n < hits.size(); n++) {
			float dist = glm::length(pts[n] - theCam->getPosition());
			if (dist < nearestDist) {
				nearestDist = dist;
				selectedObj = hits[n];
			}
		}
	}
	if (selectedObj) {
		selected.push_back(selectedObj);
		bDrag = true;
		mouseToDragPlane(x, y, lastPoint);
		display = &selected[0]->settings;
	}
	else {
		selected.clear();
		display = &gui;
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	bDrag = false;

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
	Mesh *mesh = new Mesh(glm::vec3(0, 1, 0));
	mesh->readObjFile(dragInfo.files[0]);
	scene.push_back(mesh);
	display = &mesh->settings;

	selected.clear();
	selected.push_back(mesh);
}

