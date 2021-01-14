#pragma once

#include "SceneObject.h"

// A class to handle .obj meshes by rpocessing them into vectors of vertices and triangles with indices.
// Allows for intersection with a ray and drawing in the openframeworks window.
// Maintains a bounding box to help speed up ray intersection.

class Tri {
public:
	Tri(int i0, int i1, int i2) { vInd[0] = i0; vInd[1] = i1; vInd[2] = i2; }
	bool containsIndex(int index) { return (vInd[0] == index || vInd[1] == index || vInd[2] == index); }
	int vInd[3];
	glm::vec3 normal;
};

//  Mesh class, imported from project 1
//
class Mesh : public SceneObject {

private:
	int reformatFaceData(char *str) {	// take in some data that looks like "1102/1304/445" and returns 1102
		int i = 0;
		while (str[i] != '/' && i < strlen(str)) {
			i++;
		}
		str[i] = 0;		// mark any /'s as end of string, so only the first number is read
		return atoi(str);
	}

	float stringToFloat(char *str) {		// fscanf has a hard time with negative signs, so this is necessary.
		if (str[0] != '-') return atof(str);
		else return (atof(str + 1) * -1);
	}

	glm::vec3 transform(glm::vec3 vec) {
		glm::vec3 result = vec;
		glm::vec3 rot = rotation;

		result = glm::rotateX(result, glm::radians(rot.x));
		result = glm::rotateY(result, glm::radians(rot.y));
		result = glm::rotateZ(result, glm::radians(rot.z));

		result *= (float)scale;

		result += position;

		return result;
	}

	glm::vec3 topCorner, bottomCorner;			// used to create a bounding box for the mesh to speed up ray intersection a little bit.

public:
	Mesh(glm::vec3 pos) {
		position = pos;
		settings.setup();
		settings.add(scale.setup("Scale", 1.0, 0.01, 10.0));
		settings.add(rotation.setup("Rotation", glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(360, 360, 360)));
		settings.add(diffuseColor.setup("Diffuse Color", ofColor::grey, ofColor::black, ofColor::white));
		settings.add(specularColor.setup("Specular Color", ofColor::white, ofColor::black, ofColor::white));
	}
	vector<glm::vec3> verts;
	vector<glm::vec3> vertNormals;
	vector<Tri> triangles;
	void addVertex(float x, float y, float z) { verts.push_back(glm::vec3(x, y, z)); }
	void addVertex(glm::vec3 v) { verts.push_back(v); }
	void addTriangle(int i, int j, int k) { triangles.push_back(Tri(i, j, k)); }
	void addTriangle(Tri t) { triangles.push_back(t); }
	int getNumVertices() { return verts.size(); }
	glm::vec3 getVertex(int index) { return transform(verts[index]); }
	void clearMesh() {
		verts.clear();
		triangles.clear();
	}


	void readObjFile(string fileName);
	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal);
	void draw();

	ofxFloatSlider scale;
	ofxVec3Slider rotation;
};
