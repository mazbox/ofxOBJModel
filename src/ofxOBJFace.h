//
//  ofxOBJFace.h
//  SphereMapper
//
//  Created by Marek Bereza on 15/05/2012.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#pragma once

#include "ofMain.h"


class ofxOBJFace {
public:

	ofxOBJFace(string name = "");

	vector<glm::vec3> vertices;
	vector<glm::vec3> normals;
	vector<glm::vec3> texCoords;

	
	// this doesn't really work yet.
	bool tryToMerge(ofxOBJFace &other);
	
	void draw(bool drawSolid = true);

	void addVertex(const glm::vec3 &vertex);
	void addNormal(const glm::vec3 &normal);
	void addTexCoord(const glm::vec3 &texCoord);
	/**
	 * Changes the direction of all the vertices.
	 * i.e. clockwise to anti-clockwise
	 */
	void flipDirection();

	bool containsVertex(const glm::vec3 &p);
	void swapYZ();
    void createFlatProjectionTexCoords(const ofRectangle &rect);
    void scale(float factor);

	/**
	 * Shifts the points along one
	 */
	void shiftPointsLeft();
	void shiftPointsRight();

	/**
	 * Gives you a rectangle in 2d
	 * of the face.
	 */
	ofRectangle get2DRect();

	/**
	 * Get the minimum and maximum bounding points
	 */
	void getBounds(glm::vec3 &min, glm::vec3 &max);
	void flipNormals();
	
	void calculateFlatNormals();
	string name;
	void merge(ofxOBJFace &other);
	
	// face normal - only calculated when you call calculateFlatNormals()
	glm::vec3 normal;
};
