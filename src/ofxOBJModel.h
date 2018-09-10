/*
 *  ofxOBJModel.h
 *
 *  This is a really simple wavefront obj loader/saver.
 *  It's no good for anything complicated, just good for
 *  loading and saving simple 3d meshes for projection mapping.
 *
 *  You can create files this format reads and writes in
 *  Cinema4D, you can name the objects in cinema 4d so you
 *  can reference them here. Each object shows up as an "ObjMesh"
 *  here.
 *
 *  You must make sure in cinema4d that you convert your shapes
 *  to meshes - you do this by selecting the object and pressing
 *  the 'c' key - the key for 'make editable.. It'll only work on
 *  3d objects (including planes) so if you have a path, you'll need
 *  to use the extrude nurbs object, with the extrusion depth set to
 *  zero, then press 'c'.
 *
 */

#pragma once

#include "ofMain.h"
#include "ofxOBJGroup.h"

class ofxOBJModel {
public:

	ofxOBJModel();
	/**
	 * Accessible array of groups in the OBJ file.
	 */
	vector<ofxOBJGroup> groups;

	/**
	 * load an obj file, put in the data/ dir.
	 */
	bool load(string path);
	
	
	/**
	 * Divides any quads into triangles.
	 */
	void triangulateQuads();
	
	void calculateFlatNormals();
	void calculateSmoothNormals();
	/**
	 * save the obj file, relative to data/ dir.
	 * If you call save with no parameter, it saves
	 * to where the obj was loaded
	 */
	bool save(string file = "");

	void swapYZ();
    void createFlatProjectionTexCoords();


	void clear();
	
	ofxOBJGroup &addGroup(ofxOBJGroup group);
	
	ofxOBJFace &addTriangle(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c);
	
	// add a triangle with texcoords
	ofxOBJFace &addTriangle(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c, const glm::vec3 &tca, const glm::vec3 &tcb, const glm::vec3 &tcc);
	
	void centreAroundOrigin();
	
	void translate(const glm::vec3 &t);
    /**
     * This scales the whole model, whilst preserving the normals.
     */
    void scale(float factor);

	/**
	 * Finds the minimum and maximum points in all the meshes.
	 */
	void getBounds(glm::vec3 &minPoint, glm::vec3 &maxPoint);

	/**
	 * Draw all the meshes.
	 */
	void draw(bool drawSolid = true);

	void drawPoints();
	ofVboMesh *getVboMesh();

	ofxOBJGroup *getGroup(string name);

	vector<string> getGroupNames();
	
	void flipNormals();
	
	ofRectangle getTexCoordBounds();
	void createMesh();
	

private:

	void parseFace(ofxOBJFace &face, const string &def, const vector<glm::vec3> &vertices,
				   const vector<glm::vec3> &normals, const vector<glm::vec3> &texCoords);
	
	ofVboMesh mesh;

	glm::vec3 parseCoords(string line);
	string filePath;
};



