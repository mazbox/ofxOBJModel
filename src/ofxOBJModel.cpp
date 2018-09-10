/*
 *  ofxOBJModel.cpp
 *
 */

#include "ofxOBJModel.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <limits>
#ifndef FLT_MAX
	#include <float.h>
#endif



#pragma mark ofxOBJModel

ofxOBJModel::ofxOBJModel() {

}

void ofxOBJModel::flipNormals() {
	for(int i = 0; i < groups.size(); i++) {
		groups[i].flipNormals();
	}

}

void ofxOBJModel::calculateFlatNormals() {
	for(int i =0; i < groups.size(); i++) {
		groups[i].calculateFlatNormals();
	}
}


// add a triangle with texcoords
ofxOBJFace &ofxOBJModel::addTriangle(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c, const glm::vec3 &tca, const glm::vec3 &tcb, const glm::vec3 &tcc) {
	ofxOBJFace &f = addTriangle(a, b, c);
	f.addTexCoord(tca);
	f.addTexCoord(tcb);
	f.addTexCoord(tcc);
	return f;
}

ofxOBJFace &ofxOBJModel::addTriangle(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c) {
	if(groups.size()==0) {
		addGroup(ofxOBJGroup());
	}
	ofxOBJFace face;
	face.addVertex(a);
	face.addVertex(b);
	face.addVertex(c);
	
	groups[0].addFace(face);
	return groups[0].faces.back();
}

bool ofxOBJModel::load(string path) {
	filePath = path;
	path = ofToDataPath(path, true);

	string line;

	vector<string> defs;

	// this is a list of all points
	// that we can drop after parsing
	vector<glm::vec3> vertices;
	vector<glm::vec3> normals;
	vector<glm::vec3> texCoords;

	// obj file format vertexes are 1-indexed
	vertices.push_back(glm::vec3());
	normals.push_back(glm::vec3());
	texCoords.push_back(glm::vec3());


	ifstream myfile (path.c_str());
	if (myfile.is_open()) {
		while (! myfile.eof()) {
			getline (myfile,line);


			// parse the obj format here.
			//
			// the only things we're interested in is
			// lines beginning with 'g' - this says start of new object
			// lines beginning with 'v ' - coordinate of a vertex
			// lines beginning with 'vn ' - vertex normals
			// lines beginning with 'vt ' - texcoords (either 2 or 3 values)
			// lines beginning with 'f ' - specifies a face of a shape
			// 			we take each number before the slash as the index
			// 			of the vertex to join up to create a face.



			if(line.find("g ")==0) { // new group definition
				string groupName = line.substr(2);
				defs.push_back(line);
			} else if(line.find("o ")==0) {
				string objectName = line.substr(2);
				defs.push_back(line);
			} else if(line.find("v ")==0) { // new vertex
				vertices.push_back(parseCoords(line));
			} else if(line.find("vn ")==0) {
				normals.push_back(parseCoords(line));
			} else if(line.find("vt ")==0) {
				texCoords.push_back(parseCoords(line));
			} else if(line.find("f ")==0) { // face definition
				defs.push_back(line);
			} else {
				//printf("Not parsing '%s\n'", line.c_str());
			}
		}


		myfile.close();

		string nextName = "";
		for(int i = 0; i < defs.size(); i++) {
			if(defs[i].find("g ")==0 || defs[i].find("o ")==0) {
				groups.push_back(ofxOBJGroup(ofTrim(defs[i].substr(2))));
//			} else if(defs[i].find("o ")==0) {
//				nextName = defs[i].substr(2);
				
//				if(nextName[nextName.size()-1]=='\r') {
//					nextName = nextName.substr(0, nextName.size()-1);
					//printf("Next name: '%s'\n", nextName.c_str());
//				}
				
			} else if(defs[i].find("f ")==0) {

				if(groups.size()==0) {
					ofLogWarning("ofxOBJModel") << "Found a face before any groups were created. Adding a blank one";
					groups.push_back(ofxOBJGroup());
				}
				groups.back().faces.push_back(ofxOBJFace(nextName));
				parseFace(groups.back().faces.back(), defs[i], vertices, normals, texCoords);
				nextName = "";
			}
		}





		int numFaces = 0;
		for(int i = 0; i < groups.size(); i++) {
			numFaces += groups[i].faces.size();
		}

		ofLogNotice() <<
		"ofxOBJModel: Successfully loaded "<<path<<"\n-----\nVertices: "
		<< vertices.size() - 1 <<
		"\nGroups: "
		<< groups.size() <<
		"\nNormals: "
		<< normals.size() - 1 <<
		"\nTexCoords: "
		<< texCoords.size()-1 <<
		"\nFaces: "
		<< numFaces <<
		"\n----\n";


        glm::vec3 min, max;
        getBounds(min, max);
		ofLogNotice() << "ofxOBJModel: Size (x,y,z): " <<  (max.x-min.x) << ", " <<  (max.y-min.y) << ", " << (max.z-min.z);
		createMesh();
		
		
		return true;
	} else {
		ofLogError() << "ofxOBJModel: Couldn't find the OBJ file " << path.c_str();
		return false;
	}
}
void ofxOBJModel::clear() {
	groups.clear();
	createMesh();
}

ofxOBJGroup &ofxOBJModel::addGroup(ofxOBJGroup group) {
	groups.push_back(group);
	createMesh();
	return groups.back();
}
void ofxOBJModel::triangulateQuads() {
	
	
	for(int i = 0; i < groups.size(); i++) {
		ofxOBJGroup *group = &groups[i];
		group->triangulateQuads();
	}
	createMesh();
}
void ofxOBJModel::swapYZ() {
    for(int i = 0; i < groups.size(); i++) {
        for(int j = 0; j < groups[i].faces.size(); j++) {
            groups[i].faces[j].swapYZ();
        }
    }
    createMesh();
}
void ofxOBJModel::scale(float factor) {
    for(int i = 0; i < groups.size(); i++) {
        for(int j = 0; j < groups[i].faces.size(); j++) {
            groups[i].faces[j].scale(factor);
        }
    }
    createMesh();
}

void ofxOBJModel::createFlatProjectionTexCoords() {
    glm::vec3 min, max;
    getBounds(min, max);
    ofRectangle rect(min.x, min.y, max.x-min.x, max.y-min.y);
    for(int i = 0; i < groups.size(); i++) {
        for(int j = 0; j < groups[i].faces.size(); j++) {
            groups[i].faces[j].createFlatProjectionTexCoords(rect);
        }
    }
   // swapYZ();
    createMesh();
}
void ofxOBJModel::parseFace(ofxOBJFace &face, const string &def, const vector<glm::vec3> &vertices,
							const vector<glm::vec3> &normals, const vector<glm::vec3> &texCoords) {

	// these are the possible options in an obj
	// f v1 v2 v3 v4
	// f v1/vt1 v2/vt2 v3/vt3
	// f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
	// f v1//vn1 v2//vn2 v3//vn3

	if(def.find("f ")==0) {

		vector<string> indices =  ofSplitString(def.substr(2), " ", true, true);
		for(int i = 0; i < indices.size(); i++) {

			vector<string> parts = ofSplitString(indices[i], "/");
			
			if(parts.size()>=1) {
				face.addVertex(vertices[atoi(parts[0].c_str())]);
			}
			
			if(parts.size()>=2 && parts[1]!="") {
				face.addTexCoord(texCoords[atoi(parts[1].c_str())]);
			}
			
			if(parts.size()>=3) {
				face.addNormal(normals[atoi(parts[2].c_str())]);
			}
		}

	} else {
		ofLogError() << "Not a face string in obj file " << def;
	}

}
bool ofxOBJModel::save(string file) {
	if(file=="") {
		file = filePath;
		if(filePath=="") {
			filePath = "untitled.obj";
		}
	}
	file = ofToDataPath(file, true);
	string contents = "# ofxOBJModel output\r\n\r\n";
	int vertexIndex = 1;
	string faceStrings = "";
	string n = "\r\n";
	for(int i = 0; i < groups.size(); i++) {

		ofxOBJGroup *group = &groups[i];

		contents += n + n + "g " + groups[i].name + n;


		for(int j = 0; j < groups[i].faces.size(); j++) {
			ofxOBJFace *face = &group->faces[j];

			if(face->name!="") {
				contents += "o " + face->name + n;
			}

			bool doingNormals = face->normals.size()==face->vertices.size();
			bool doingTexCoords = face->texCoords.size()==face->vertices.size();


			for(int k = 0; k < face->vertices.size(); k++) {
				glm::vec3 p = face->vertices[k];
				contents += "v " + ofToString(p.x) + " " + ofToString(p.y) + " " + ofToString(p.z) + " " +n;
			}



			if(doingNormals) {
				contents += n;
				for(int k = 0; k < face->vertices.size(); k++) {
					glm::vec3 p = face->normals[k];
					contents += "vn " + ofToString(p.x) + " " + ofToString(p.y) + " " + ofToString(p.z) + " " +n;
				}

			}

			if(doingTexCoords) {
				contents += n;
				for(int k = 0; k < face->vertices.size(); k++) {
					glm::vec3 p = face->texCoords[k];
					contents += "vt " + ofToString(p.x) + " " + ofToString(p.y) + " " + ofToString(p.z) + " " +n;
				}
			}

			if(!doingNormals && !doingTexCoords) {
				contents += "f";
				for(int k = 0; k < face->vertices.size(); k++) {
					contents += " " + ofToString(vertexIndex);
					vertexIndex++;
				}
				contents += n;
			} else if(doingNormals && !doingTexCoords) {
				contents += "f";
				for(int k = 0; k < face->vertices.size(); k++) {
					contents += " " + ofToString(vertexIndex) + "//" + ofToString(vertexIndex);
					vertexIndex++;
				}
				contents += n;

			} else if(doingNormals && doingTexCoords) {
				contents += "f";
				for(int k = 0; k < face->vertices.size(); k++) {
					contents += " " + ofToString(vertexIndex) + "/"+ofToString(vertexIndex)+"/" + ofToString(vertexIndex);
					vertexIndex++;
				}
				contents += n;
			} else if(!doingNormals && doingTexCoords) {
				contents += "f";
				for(int k = 0; k < face->vertices.size(); k++) {
					contents += " " + ofToString(vertexIndex) + "/" + ofToString(vertexIndex);
					vertexIndex++;
				}
				contents += n;
			}





		}

		contents += n;
	}


	ofstream myfile (file.c_str());
	if (myfile.is_open()) {
		myfile << contents;
		myfile.close();
		return true;
	} else {
		ofLogError() << "ofxOBJModel: Could not write to file";
		return false;
	}


}


glm::vec3 ofxOBJModel::parseCoords(string line) {
	glm::vec3 p;
	line = line.substr(line.find(" ")+1);
	vector<string> elements = ofSplitString(line, " ");
	if(elements.size()<2) {
		ofLogError() << "ofxOBJModel: Error line does not have at least 2 coordinates: \"" << line << "\"";
		return p;
	}
	p.x = atof(elements[0].c_str());
	p.y = atof(elements[1].c_str());
	if(elements.size()>=3) {
		p.z = atof(elements[2].c_str());
	}

	return p;
}



void ofxOBJModel::draw(bool drawSolid) {
	mesh.draw();
//	for(int i = 0; i < groups.size(); i++) {
//		groups[i].draw(drawSolid);
//	}
}

void ofxOBJModel::drawPoints() {
	mesh.draw(OF_MESH_POINTS);
}


vector<string> ofxOBJModel::getGroupNames() {
	vector<string> groupNames;
	for(int i = 0; i < groups.size(); i++) {
		groupNames.push_back(groups[i].name);
	}
	return groupNames;
}

void ofxOBJModel::getBounds(glm::vec3 &minPoint, glm::vec3 &maxPoint) {
	minPoint.x = minPoint.y = minPoint.z = FLT_MAX;
	maxPoint.x = maxPoint.y = maxPoint.z = -FLT_MAX;
	for(int i = 0; i < groups.size(); i++) {
		glm::vec3 min, max;
		groups[i].getBounds(min, max);

		if(max.x>maxPoint.x) maxPoint.x = max.x;
		if(max.y>maxPoint.y) maxPoint.y = max.y;
		if(max.z>maxPoint.z) maxPoint.z = max.z;

		if(min.x<minPoint.x) minPoint.x = min.x;
		if(min.y<minPoint.y) minPoint.y = min.y;
		if(min.z<minPoint.z) minPoint.z = min.z;
	}

}


ofxOBJGroup *ofxOBJModel::getGroup(string name) {
	for(int i = 0; i < groups.size(); i++) {
		if(groups[i].name==name) {
			return &groups[i];
		}
	}
	ofLogError() << "\n--------\nWarning Warning!!!! No mesh named '"<<name<<"', the program will probably crash.\nPlease update the OBJ file in cinema 4d to have a mesh called '"<< name << "'";
	return NULL;
}

ofVboMesh *ofxOBJModel::getVboMesh() {
	return &mesh;
}

void ofxOBJModel::createMesh() {

	mesh.clear();

	mesh.setUsage(GL_STATIC_DRAW);
	mesh.setMode(OF_PRIMITIVE_TRIANGLES);
	for(int i = 0; i < groups.size(); i++) {
		ofxOBJGroup *group = &groups[i];
		for(int j = 0; j < group->faces.size(); j++) {
			ofxOBJFace *face = &group->faces[j];
			bool doingNormals = face->normals.size()==face->vertices.size();
			bool doingTexCoords = face->texCoords.size()==face->vertices.size();


			for(int k = 0; k < face->vertices.size(); k++) {
				mesh.addVertex(face->vertices[k]);
				if(doingNormals) {
					mesh.addNormal(face->normals[k]);
				}
				if(doingTexCoords) {
					mesh.addTexCoord(face->texCoords[k]);
				}
			}
		}
	}
}


ofRectangle ofxOBJModel::getTexCoordBounds() {
	bool started = false;
	ofRectangle r;
	for(int i = 0; i < groups.size(); i++) {
		for(int j = 0; j < groups[i].faces.size(); j++) {
			for(int k = 0; k < groups[i].faces[j].texCoords.size(); k++) {
				if(!started) {
					r.set(groups[i].faces[j].texCoords[k].x,groups[i].faces[j].texCoords[k].y, 0, 0);
					started = true;
				} else {
					r.growToInclude(groups[i].faces[j].texCoords[k]);
				}
				
			}
		}
	}
	return r;
}


void ofxOBJModel::calculateSmoothNormals() {
	for(int i = 0; i < groups.size(); i++) {
		groups[i].calculateSmoothNormals();
	}
}



void ofxOBJModel::centreAroundOrigin() {
	glm::vec3 minP, maxP;
	getBounds(minP, maxP);
	glm::vec3 oldCentre(maxP.x + minP.x, maxP.y + minP.y, maxP.z + minP.z);// = maxP - maxP;
	oldCentre /= 2;
	ofLogWarning() << "minP " << minP <<   "    maxp " <<maxP << " old centre " << oldCentre;

	ofLogWarning() << "translating by " << (-oldCentre);
	translate(-oldCentre);
	createMesh();
}





void ofxOBJModel::translate(const glm::vec3 &t) {
	for(int i = 0; i < groups.size(); i++) {
		groups[i].translate(t.x, t.y, t.z);

	}
}


