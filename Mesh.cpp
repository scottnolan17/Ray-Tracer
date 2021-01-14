#include "Mesh.h"


// Ray box intersection function, as defined in:
//		Amy Williams, Steve Barrus, R.Keith Morley, and Peter Shirley 
//		"An Efficient and Robust Ray-Box Intersection Algorithm" 
//		Journal of graphics tools, 10(1) : 49 - 54, 2005
//
bool intersectRayBox(const Ray &r, float minDist, float maxDist, glm::vec3 bottomCorner, glm::vec3 topCorner) {
	float tmin, tmax, tymin, tymax, tzmin, tzmax;
	glm::vec3 boundingBox[2] = { bottomCorner, topCorner };

	tmin = (boundingBox[r.sign[0]].x - r.p.x) * r.inv_d.x;
	tmax = (boundingBox[1 - r.sign[0]].x - r.p.x) * r.inv_d.x;
	tymin = (boundingBox[r.sign[1]].y - r.p.y) * r.inv_d.y;
	tymax = (boundingBox[1 - r.sign[1]].y - r.p.y) * r.inv_d.y;
	if ((tmin > tymax) || (tymin > tmax))
		return false;
	if (tymin > tmin)
		tmin = tymin;
	if (tymax < tmax)
		tmax = tymax;
	tzmin = (boundingBox[r.sign[2]].z - r.p.z) * r.inv_d.z;
	tzmax = (boundingBox[1 - r.sign[2]].z - r.p.z) * r.inv_d.z;
	if ((tmin > tzmax) || (tzmin > tmax))
		return false;
	if (tzmin > tmin)
		tmin = tzmin;
	if (tzmax < tmax)
		tmax = tzmax;
	return ((tmin < maxDist) && (tmax > minDist));
}

// takes in an obj file and parses it into its vertices and faces. the mesh is cleared and refilled with the new vertices and triangles
//
void Mesh::readObjFile(string fileName) {
	clearMesh();

	FILE *file;
	char s[64];

	file = fopen(fileName.c_str(), "r");

	float x, y, z;
	int i, j, k;

	while (fscanf(file, "%s", s) != EOF) {
		if (s[0] == 'v' && strlen(s) == 1) {		// vertex data: read the next 3 values as floats
			fscanf(file, "%s", s);
			x = stringToFloat(s);
			fscanf(file, "%s", s);
			y = stringToFloat(s);
			fscanf(file, "%s", s);
			z = stringToFloat(s);
			addVertex(x, y, z);
		}
		else if (s[0] == 'f'  && strlen(s) == 1) {	// face (index) data: read the next 3 values as ints
			fscanf(file, "%s", s);
			i = reformatFaceData(s) - 1;	// -1 because it seems like .obj files don't follow the "start counting from 0" rule
			fscanf(file, "%s", s);
			j = reformatFaceData(s) - 1;
			fscanf(file, "%s", s);
			k = reformatFaceData(s) - 1;
			addTriangle(i, j, k);
		}
	}

	cout << "vertices: " << verts.size() << endl;
	cout << "triangles: " << triangles.size() << endl;
	cout << "size: " << sizeof(this) << "kB" << endl;

	fclose(file);

	// make a bounding box so we can find the center of the points
	topCorner = bottomCorner = verts.front();
	for (auto v : verts) {
		if (v.x > topCorner.x) topCorner.x = v.x;
		if (v.y > topCorner.y) topCorner.y = v.y;
		if (v.z > topCorner.z) topCorner.z = v.z;
		if (v.x < bottomCorner.x) bottomCorner.x = v.x;
		if (v.y < bottomCorner.y) bottomCorner.y = v.y;
		if (v.z < bottomCorner.z) bottomCorner.z = v.z;
	}

	glm::vec3 center = (bottomCorner + (topCorner - bottomCorner) / 2);

	for (int i = 0; i < verts.size(); i++) {
		verts[i] -= center;			// recenter the relative origin of the mesh to the center of the points, so that it doesn't look weird when scaling
	}
	bottomCorner -= center;
	topCorner -= center;

	// create the face normals of all the triangles
	for (int i = 0; i < triangles.size(); i++) {				
		triangles[i].normal = glm::normalize(glm::cross(
			(transform(verts[triangles[i].vInd[1]]) - transform(verts[triangles[i].vInd[0]])),
			(transform(verts[triangles[i].vInd[2]]) - transform(verts[triangles[i].vInd[0]]))
		));

	}

	// create the vertex normals for each vertex
	for (int i = 0; i < verts.size(); i++) {					
		int count = 0;
		glm::vec3 avg = glm::vec3(0, 0, 0);

		for (Tri t : triangles) {		// ...yes, this is a very slow algorthm
			if (t.vInd[0] == i || t.vInd[1] == i || t.vInd[2] == i) {
				count++;
				avg += t.normal;
			}
			
		}

		avg.x = avg.x / count;
		avg.y = avg.y / count;
		avg.z = avg.z / count;
		avg = glm::normalize(avg);

		vertNormals.push_back(avg);		// vertex i's normal can be found at vertNormals[i]
	}

}

// intersect ray with the mesh; checks a bounding box first, then each triangle
//
bool Mesh::intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &norm) {
	if (!intersectRayBox(ray, 0, 1000, bottomCorner, topCorner)) return false;
	Ray r = ray;
	glm::vec2 bary, closestBary;
	glm::vec3 closestNorm, vn0, vn1, vn2;	// face normal and vertex normals of the closest triangle
	float dist;
	float closest = 1000;
	for (Tri t : triangles) {
		if (glm::intersectRayTriangle(r.p, r.d, transform(verts[t.vInd[0]]), transform(verts[t.vInd[1]]), transform(verts[t.vInd[2]]), bary, dist) && dist < closest) {
			closest = dist;
			point = r.evalPoint(closest);
			closestBary = bary;
			closestNorm = t.normal;
			vn0 = vertNormals[t.vInd[0]];
			vn1 = vertNormals[t.vInd[1]];
			vn2 = vertNormals[t.vInd[2]];
		}
	}

	norm = (1 - closestBary.x - closestBary.y) * vn0 + closestBary.x * vn1 + closestBary.y * vn2;	// linearly interpolate hit-point normals using vertex normals multiplied by barycentric coordinates

	glm::vec3 rot = rotation;
	norm = glm::rotateX(norm, glm::radians(rot.x));		// rotate the face normal to the correct direction
	norm = glm::rotateY(norm, glm::radians(rot.y));
	norm = glm::rotateZ(norm, glm::radians(rot.z));

	if (closest < 1000) return true; 

	return false;
}

// draw the entire mesh as a wireframe using ofDrawTriangle()
//
void Mesh::draw() {
	topCorner = bottomCorner = transform(verts.front());
	for (auto vertex : verts) {
	glm::vec3 v = transform(vertex);
		if (v.x > topCorner.x) topCorner.x = v.x;
		if (v.y > topCorner.y) topCorner.y = v.y;
		if (v.z > topCorner.z) topCorner.z = v.z;
		if (v.x < bottomCorner.x) bottomCorner.x = v.x;
		if (v.y < bottomCorner.y) bottomCorner.y = v.y;
		if (v.z < bottomCorner.z) bottomCorner.z = v.z;
	}
	
	vector<Tri> selectedTris;
	float size = scale;
	for (auto t : triangles) {
		ofDrawTriangle(transform(verts[t.vInd[0]]), transform(verts[t.vInd[1]]), transform(verts[t.vInd[2]]));
	}

	ofNoFill();
	glm::vec3 center = bottomCorner + (topCorner - bottomCorner) / 2;
	ofDrawBox(center, topCorner.x - bottomCorner.x, topCorner.y - bottomCorner.y, topCorner.z - bottomCorner.z);
	ofFill();
}




