#include "MeshLoaderObj.h"

using namespace std;

/**
 * \brief Go through the whole string and remove all line breaks (n) and tabs (t)
 * \param str to go through
 */
void RemoveLineBreaks(string& str) {
	for (int i=0; i<str.size(); i++) {
		if (str[i] == '\t' || str[i] == '\n') {
			str.erase(i, 1);
			i--;
		}
	}
}

/**
 *	\brief Load a Mesh from a Obj file
 *	
 *	\param Stream to load a mesh from
 *  \return Mesh if successfull, 0 otherwise
 */
Mesh* MeshLoaderObj::Load(istream& Stream) {
	Mesh* M = new Mesh();

	string Definition;
	mmsm Materials;

	// Material initialize
	M3DVector3f MatAmbient;
	m3dLoadVector3(MatAmbient, 1.0f, 1.0f, 1.0f);
	Material Mat("Proto", MatAmbient);
	Mat.SetTransparency(1.0f);

	DLOG(INFO) << "Start reading Mesh" << endl;

	TriangulationEasy = 0;
	TriangulationHard = 0;

	/* How we read it in: always process one line at a time. First we read the first string on the line. This indicates what type of definition we have. With the switch case we handle every single Definition (Vertices, Comments, Normals, etc.) and process it */
	while(!Stream.eof() && Stream.good()) {
		// When we have empty lines there are \r or \n or maybe \t or whitespaces
		while (Stream.peek() == '\r' || Stream.peek() == '\n' || Stream.peek() == '\t' || Stream.peek() == ' ')
			Stream.ignore(1);

		// Maybe we had some empty lines at the end
		if (Stream.eof())
			break;

		// Special case comments they don't need to have a whitespace after them so we need to handle them separately
		if (Stream.peek() == '#')
			Definition = "#";

		// Special case 2: only g without name is also valid
		else if (Stream.peek() == 'g')
			Definition = "g";
		else
			getline(Stream, Definition, ' ');


		/* PROCESS LINES */

		// Comments -> Ignore
		if (Definition.compare("#") == 0) {
			Stream.ignore(INT_MAX, '\n');
			//DLOG(INFO) << "Found Comment - ignored" << endl;


		// Object Name
		// o object name
		} else if (Definition[0] == 'o') {
			Stream.ignore(INT_MAX, '\n');
			//DLOG(INFO) << "Found Object Name - ignored" << endl;


		// Group Name
		// g group name
		} else if (Definition[0] == 'g') {
			Stream.ignore(INT_MAX, '\n');
			//DLOG(INFO) << "Found Group Name - ignored" << endl;


		// Smooth Shading activate/deactivate
		// s 1
  		// s off
		} else if (Definition[0] == 's') {
			Stream.ignore(INT_MAX, '\n');
			//DLOG(INFO) << "Found Shading Properties - ignored" << endl;


		// Reference External Material File
		// mtllib [external .mtl file name]
		} else if (Definition.compare("mtllib") == 0) {
			string FileName;
			getline(Stream, FileName, '\n');
			//DLOG(INFO) << "Found Reference to external Material file: " << FileName << endl;
			ReadMaterialFile(Materials, FileName);

		// Material Group
		// usemtl [material name]
		} else if (Definition.compare("usemtl") == 0) {
			string MatName;
			getline(Stream, MatName, '\n');
			RemoveLineBreaks(MatName);
			mmsm::iterator it = Materials.find(MatName);
			if (it == Materials.end())
				DLOG(WARNING) << "Couldn't find a material in the material map: " << MatName << endl;
			else
				Mat = it->second;
			DLOG(INFO) << "Apply Material: " << MatName << endl;
			

		// Vertices, with (x,y,z[,w]) coordinates, w is optional.
		// v 0.123 0.234 0.345 1.0
		} else if (Definition.compare("v") == 0) {
			float* v = new float[3];
			ReadVector4fTo3f(v, Stream, true, false);
			M->_vertices.push_back(v);
			//DLOG(INFO) << "Found Vertex: " << v[0] << ", " << v[1] << ", " << v[2] << ", number: " << M->_vertices.size()-1 << endl;


		// Texture coordinates, in (u,v[,w]) coordinates, w is optional.
		// vt 0.500 -1.352 [0.234]
		} else if (Definition.compare("vt") == 0) {
			Stream.ignore(INT_MAX, '\n');
			//DLOG(INFO) << "Found Texture Coordinate - ignored" << endl;


		// Normals in (x,y,z) form; normals might not be unit.
		// vn 0.707 0.000 0.707
		// ignore as we calculate them ourselves so they are in the right order and not for a point but for the whole triangle
		} else if (Definition.compare("vn") == 0) {
			Stream.ignore(INT_MAX, '\n');
			//DLOG(INFO) << "Found Normal - ignored" << endl;


		// Face Definitions 
		// f 1 2 3						-> Vertex index starting with 1, face can contain more than three elements!
		// f 3/1 4/2 5/3					-> Vertex/texture-coordinate
		// f 6/4/1 3/5/3 7/6/5			-> Vertex/texture-coordinate/normal
		// f v1//vn1 v2//vn2 v3//vn3	-> Vertex/normal
		} else if (Definition.compare("f") == 0) {
			ReadFace(Stream, M, Mat);

		// any other keywords -> ignore them
		} else {
			Stream.ignore(INT_MAX, '\n');
			DLOG(INFO) << "Found Unknown Definition " << Definition << endl;
		}
	}




	// Add Texture for all Vertices
	for (VertexVector::iterator it = M->_vertices.begin(); it != M->_vertices.end(); ++it) {
		float* Texture = new float[2];
		Texture[0] = 0;
		Texture[1] = 0;

		M->_textureCoords.push_back(Texture);
	}

	DLOG(INFO) << "Read in " << M->_vertices.size() << " Vertices" << endl;
	DLOG(INFO) << "Read in " << M->_triangles.size() << " Triangles" << endl;
	DLOG(INFO) << "Calculated " << M->_normals.size() << " Normals" << endl;
	DLOG(INFO) << "Read in " << M->_material.size() << " Materials" << endl;
	DLOG(INFO) << "Read in " << M->_textureCoords.size() << " Texture Coordinates" << endl;
	DLOG(INFO) << "TriangulationEasy " << TriangulationEasy << endl;
	DLOG(INFO) << "TriangulationHard " << TriangulationHard << endl;

	if (Stream.bad()) {
		DLOG(WARNING) << "Error reading in Mesh" << endl;
		return 0;
	} else {
		DLOG(INFO) << "Mesh Successfully read in" << endl;
		return M;
	}
}

/**
 * \brief Opens and Reads a Material File and places all material Information in the MaterialMap
 * \param MaterialMap where we place Material Information
 * \param FileName to read the material information from
 */
// TODO:
void MeshLoaderObj::ReadMaterialFile(mmsm& MaterialMap, string& FileName) {
	RemoveLineBreaks(FileName);
	ifstream In(FileName.c_str());

	DLOG(INFO) << "Load Material: " << FileName << endl;
	DLOG(INFO) << "Stream Status " << In.good() << endl;

	string Definition;

	M3DVector3f MatAmbient;
	m3dLoadVector3(MatAmbient, 0.2f, 0.2f, 0.2f);

	Material Mat("Empty", MatAmbient);

	Material PrototypeMat("Proto", MatAmbient);
	PrototypeMat.SetDiffuse(0.8f, 0.8f, 0.8f);

	bool FirstMat = true;
	float r;
	float g;
	float b;

	while(!In.eof() && In.good()) {
		// When we have empty lines there are \r or \n or maybe \t or whitespaces
		while (In.peek() == '\r' || In.peek() == '\n' || In.peek() == '\t' || In.peek() == ' ')
			In.ignore(1);

		// Maybe we had some empty lines at the end
		if (In.eof())
			break;

		// Special case comments they don't need to have a whitespace after them so we need to handle them separately
		if (In.peek() == '#')
			Definition = "#";
		else
			getline(In, Definition, ' ');

		/* PROCESS LINES */

		// Start a new Material, Commit the Old One to the Map
		if (Definition.compare("newmtl") == 0) {
			if (!FirstMat)
				MaterialMap.insert(mmsm::value_type(Mat._name, Mat));
			FirstMat = false;
			Mat = Material(PrototypeMat);
			getline(In, Mat._name, '\n');
			RemoveLineBreaks(Mat._name);
			DLOG(INFO) << "Found Material: " << Mat._name << endl;


		// Found Ambient Color
		} else if (Definition.compare("Ka") == 0) {
			In >> r >> g >> b;
			Mat.SetAmbient(r, g, b);
			//DLOG(INFO) << "  Found Ambient Color (" << r << ", " << g << ", " << b << ")" << endl;
			In.ignore(INT_MAX, '\n');


		// Found Diffuse Color
		} else if (Definition.compare("Kd") == 0) {
			In >> r >> g >> b;
			Mat.SetDiffuse(r, g, b);
			//DLOG(INFO) << "  Found Diffuse Color (" << r << ", " << g << ", " << b << ")" << endl;
			In.ignore(INT_MAX, '\n');


		// Found Specular Color
		} else if (Definition.compare("Ks") == 0) {
			In >> r >> g >> b;
			Mat.SetSpecular(r, g, b, 0);
			//DLOG(INFO) << "  Found Specular Color (" << r << ", " << g << ", " << b << ")" << endl;
			In.ignore(INT_MAX, '\n');


		// Found Alpha
		} else if (Definition.compare("d") == 0 || Definition.compare("Tr") == 0) {
			In >> r;
			Mat.SetTransparency(r);
			//DLOG(INFO) << "  Found Alpha Value " << r << endl;
			In.ignore(INT_MAX, '\n');


		// Found Shininess
		} else if (Definition.compare("Ns") == 0) {
			In >> r;
			Mat.SetSpecular(Mat.GetSpecularLight(), r);
			//DLOG(INFO) << "  Found Shininess Value " << r << endl;
			In.ignore(INT_MAX, '\n');


		// Found Illumination Model
		} else if (Definition.compare("illum") == 0) {
			//DLOG(INFO) << "  Found Illumination Model - ignored " << endl;
			In.ignore(INT_MAX, '\n');

		// Found Texture Map
		} else if (Definition.compare("map_Ka") == 0) {
			//DLOG(INFO) << "  Found Texture Map - ignored" << endl;
			In.ignore(INT_MAX, '\n');


		// Found Comment
		} else if (Definition.compare("#") == 0) {
			//DLOG(INFO) << "  Found Comment - ignored" << endl;
			In.ignore(INT_MAX, '\n');


		// any other keywords -> ignore them
		} else {
			In.ignore(INT_MAX, '\n');
			DLOG(INFO) << "Found Unknown Definition " << Definition << endl;
		}
	}

	// Insert the last material as well
	if (!FirstMat)
		MaterialMap.insert(mmsm::value_type(Mat._name, Mat));

	In.close();
}

/**
 *	\brief Check if this is a Obj file
 *
 *	\param Stream to read a object from
 *	\return true if it ends with .obj, false otherwise
 * TODO: just stupidly returns true
 */
bool MeshLoaderObj::Accept(istream& Stream) {
	return true;
}

/**
 * \brief Reads 3 float numbers from the stream an places them in the array v
 *
 * \param v must be a float array with 3 elements
 * \param Stream where we read from
 */
void MeshLoaderObj::ReadVector3f(float* v, istream& Stream) {
	Stream >> v[0];
	Stream >> v[1];
	Stream >> v[3];
}

/**
 * \brief If OptionalW is false we read in 4 floats. Otherwise we read in 3 floats and check if it is specified (either within or not within brackets) and try to read it. If it is not specified we just set it to 1.0f. At the end we normalize the x, y and z values and can therefore discard w anyway.
 *
 * \param v pointer to an float array with 3 elements
 * \param Stream where we read from
 * \param OptionalW if false we need to read in a w and normalize otherwise we try but set it to 1.0f if we can't find it
 * \param OptionalWInBrackets if true we try to read in in format [1.0] otherwise just format 1.0
 */
void MeshLoaderObj::ReadVector4fTo3f(float* v, istream& Stream, bool OptionalW, bool OptionalWInBrackets) {
	// Read in x to z
	Stream >> v[0];
	Stream >> v[1];
	Stream >> v[2];

	// w is optional and if it is there it has brackets around
	float w;
	if (OptionalW && OptionalWInBrackets) {
		char c;
		Stream >> c;
		if (c != '[') {
			Stream.unget();
			w = 1.0f;
		} else {
			Stream >> w;
			Stream.ignore('\n');
		}
	// w is optional but if there are no brackets around
	} else if (OptionalW && !OptionalWInBrackets) {
		Stream >> w;
		if (Stream.fail()) {
			Stream.clear();
			w = 1.0f;
		}
	} else {
		Stream >> w;
	}

	// normalize so we don't need the w
	v[0] = v[0] / w;
	v[1] = v[1] / w;
	v[2] = v[2] / w;
}

/**
 * \brief Reads in a Triangle. It accepts the following formats:\n
 *  1 2 3						-> Vertex index starting with 1, face can contain more than three elements!\n
 * 3/1 4/2 5/3					-> Vertex/texture-coordinate\n
 * 6/4/1 3/5/3 7/6/5			-> Vertex/texture-coordinate/normal\n
 * v1//vn1 v2//vn2 v3//vn3		-> Vertex/normal\n
 *
 * If it is not a triangle but something of a higher order we rollback the position in the stream to the beginning and
 * set the first entry of the triangle to INT_MIN
 *
 * \param t where we should store the values
 * \param Stream where we read from
 */
void MeshLoaderObj::ReadTriangle(Triangle* t, istream& Stream) {
	int startpos = Stream.tellg();

	Stream >> t->vert1;
	Stream.ignore(INT_MAX, ' ');
	Stream >> t->vert2;
	Stream.ignore(INT_MAX, ' ');
	Stream >> t->vert3;

	// The Vertices Index in the file start with 1 in our array it starts with 0
	t->vert1--;
	t->vert2--;
	t->vert3--;

	// It is Vertex/Texture Vertex/Texture/Normal or Vertex/Normal Format
	if (Stream.peek() == '/') {
		int i;
		Stream.ignore(1);

		// Finish VertexNormal Format
		if (Stream.peek() == '/') {
			Stream.ignore(1);
			Stream >> i;
		// Finish Vertex/Texture
		} else {
			Stream >> i;
			// Finish Vertex/Texture/Normal
			if (Stream.peek() == '/') {
				Stream.ignore(1);
				Stream >> i;
			}
		}
	}

	// If there are spaces or tabs ignore them
	while (Stream.peek() == '\t' || Stream.peek() == ' ')
		Stream.ignore(1);

	if (Stream.peek() != '\n' && Stream.peek() != '\r') {
		//DLOG(WARNING) << "Face is not a Triangle but has more elements: We do Triangulation" << endl;
		Stream.seekg(startpos, ios::beg);
		//Stream.ignore(INT_MAX, '\n');
		t->vert1 = INT_MIN;
	}
}

/** \brief Determines the sign of the argument
 * \param X to determine sign from
 * \return -1 if smaller than 0, 1 otherwise
 */
int Sign(float X){ return(X<0 ? -1 : 1); }

/** \brief One convexity test is to conceptually walk along the polygon edges and check if the sign of the change in x and y changes more than twice. If it does than the polygon is convex. That should also check for complex polygons as well.
 * \param VList List containing the Index of the Vertices
 * \param M Mesh containing the Vertices themselves
 */
bool MeshLoaderObj::IsPolygonConvex(vector<int>& VList, Mesh* M) {
	// If less than 4 points it's always convex
	if (VList.size() < 4)
		return true;

	// count how many times the x or y signs are diffrent
	int XCh = 0;
	int YCh = 0;

//DLOG(INFO) << "order: " << VList.size() << endl;

for (int i=0; i<VList.size(); i++) {
//	DLOG(INFO) << "point (" << VList[i] << "): " << M->_vertices[VList[i]][0] << ", " << M->_vertices[VList[i]][1] << ", " << endl;
}

	// Vector from last to first point
	float* A = new float[2];
	A[0] = M->_vertices[VList[VList.size()-1]][0] - M->_vertices[VList[0]][0];
	A[1] = M->_vertices[VList[VList.size()-1]][1] - M->_vertices[VList[0]][1];

	float* B = new float[2];

	for(int i=0; i<VList.size()-1; ++i) {
		B[0] = M->_vertices[VList[i]][0] - M->_vertices[VList[i+1]][0];
		B[1] = M->_vertices[VList[i]][1] - M->_vertices[VList[i+1]][1];

		if (Sign(A[0]) != Sign(B[0]))
			++XCh;

		if (Sign(A[1]) != Sign(B[1])) 
			++YCh;

		A[0] = B[0];
		A[1] = B[1];
	}

	delete[] A;
	delete[] B;

//DLOG(INFO) << "XCh: " << XCh << endl;
//DLOG(INFO) << "YCh: " << YCh << endl;

	return (XCh <= 2 && YCh <= 2);
}

/** \brief Reads in a lot of points and creates triangles out of them. It only accepts the basic format (like 1 2 3 4 5 6 7) to specify polygons. 
 * TList to add the Triangles we find to
 * Stream to read in the faces from
 * M Mesh to get the Vertices from
 */
bool MeshLoaderObj::Triangulate(vector<Triangle>& TList, istream& Stream, Mesh* M) {
	// Read in all the Vertices Indexes
	vector<int> Vertices;
	int index;
	while (Stream.peek() != '\n') {
		Stream >> index;

		// Vertex index in file start with 1, array start with 0
		index--;

		Vertices.push_back(index);

		// ignore texture coordinates etc.
		if (Stream.peek() == '/') {
			while (Stream.peek() != ' ' && Stream.peek() != '\n')
				Stream.ignore(1);
		}

		while(Stream.peek() == ' ' || Stream.peek() == '\t' || Stream.peek() == '\r')
			Stream.ignore(1);
	}

	// If it is concave it's to hard
	if (!IsPolygonConvex(Vertices, M)) {
		DLOG(WARNING) << "Now it gets tough concave triangulation!" << endl;
		return TriangulateConcave(TList, Vertices, M);
	}

	return Triangulate(TList, Vertices, M);
}

/**
 * Easy Triangulation. Triangulate by finding the highest y-Coordinate, it's two nearest neighbors and drawing the lines. We then add the triangle and remove the highest point. When we have only three points left they are the last triangle and we quit
 * \param TList to add the triangles to
 * \param Vertices index of the Vertices
 * \param M Mesh to get the Vertices
 * \return true if the Triangulation was successful, false otherwise
 */
bool MeshLoaderObj::Triangulate(vector<Triangle>& TList, vector<int> Vertices, Mesh* M) {
	//DLOG(INFO) << "Easy Triangulation for " << Vertices.size() << " points" << endl;
	TriangulationEasy++;

	// TODO: normals correct?
	while (Vertices.size() > 3) {
		// find highest y-coordinate
		float* HighestY = M->_vertices[Vertices[0]];
		float HighestYIndex = 0;
		for (int i = 0; i<Vertices.size(); i++) {
			if (M->_vertices[Vertices[i]][1] > HighestY[1]) {
				HighestY = M->_vertices[Vertices[i]];
				HighestYIndex = i;
			}
		}

		// put highest point at the end
		Vertices.push_back(Vertices[HighestYIndex]);
		Vertices.erase(Vertices.begin()+HighestYIndex);
		HighestYIndex = Vertices.size()-1;

		// find two nearest points
		float* Near1 = M->_vertices[Vertices[0]];
		int Near1Index = 0;
		float Near1Distance = abs(HighestY[0]-Near1[0]) + abs(HighestY[1]-Near1[1]) + abs(HighestY[2]-Near1[2]);
		float* Near2 = M->_vertices[Vertices[1]];
		int Near2Index = 1;
		float Near2Distance = abs(HighestY[0]-Near2[0]) + abs(HighestY[1]-Near2[1]) + abs(HighestY[2]-Near2[2]);
		float* Candidate;
		float CandidateDistance;
		for (int i=2; i<Vertices.size()-1; i++) {
			Candidate = M->_vertices[Vertices[i]];
			CandidateDistance = abs(HighestY[0]-Candidate[0]) + abs(HighestY[1]-Candidate[1]) + abs(HighestY[2]-Candidate[2]);
			// It's the nearest -> replace near point that is further away
			if (CandidateDistance < Near1Distance && CandidateDistance < Near2Distance) {
				if (Near1Distance < Near2Distance) {
					Near2Index = i;
					Near2Distance = CandidateDistance;
				} else {
					Near1Index = i;
					Near1Distance = CandidateDistance;
				}
			// only nearer than near 1 so we replace that one
			} else if (CandidateDistance < Near1Distance) {
				Near1Index = i;
				Near1Distance = CandidateDistance;
			// only nearer than near 2 so we replace that one
			} else if (CandidateDistance < Near2Distance) {
				Near2Index = i;
				Near2Distance = CandidateDistance;
			}
		}

		// build triangle
		TList.push_back(Triangle(Vertices[HighestYIndex], Vertices[Near1Index], Vertices[Near2Index]));

		// remove highest point
		Vertices.erase(Vertices.begin()+HighestYIndex);
	}

	// Add the last triangle
	TList.push_back(Triangle(Vertices[0], Vertices[1], Vertices[2]));

	//DLOG(INFO) << "Triangulation produced " << TList.size() << " triangles" << endl;

	return true;
}

/** \brief A and B are vectors. The function returns the angle between A and B (http://www.euclideanspace.com/maths/algebra/vectors/angleBetween/index.htm)
 * \param A float array with 2 elements, will not be changed
 * \param B float array with 2 elements, will not be changed
 * \return the angle in radians */
double AngleAToB(float* A, float* B) {
	return atan2(B[1], B[0]) - atan2(A[1], A[0]);
}

/** \brief get the Vector between A and B
 * \param A float Array with 3 elements, will not be changed
 * \param B float Array with 3 elements, will not be changed
 * \param Vector float Array with 3 elements, to store the result in
 */
void VectorFromAToB(float* A, float* B, float* Vector) {
	Vector[0] = B[0] - A[0];
	Vector[1] = B[1] - A[1];
	Vector[2] = B[2] - A[2];
}

/** \brief We start with one Vertex. Take the point p before and after. If the inner angle is less than 180 degrees it
it's triangle is inside and we chop it away (otherwise we just go to the next point). We remove the Point p (the others stay!) and have a smaller Shape. We test if it is already concave and otherwise we chop away another piece. 
 * \param TList where w place the triangles we find
 * \param VerticesIndices indices for the Vertices
 * \param M Mesh with the Vertices
 */
bool MeshLoaderObj::TriangulateConcave(vector<Triangle>& TList, vector<int>& VerticesIndices, Mesh* M) {
	TriangulationHard++;

	for (int i=1; i<VerticesIndices.size()-1; i++) {
		float* Vec1 = new float[3];
		float* Vec2 = new float[3];

		VectorFromAToB(M->_vertices[VerticesIndices[i-1]], M->_vertices[VerticesIndices[i]], Vec1);
		VectorFromAToB(M->_vertices[VerticesIndices[i+1]], M->_vertices[VerticesIndices[i]], Vec2);

		// Angle less than 180 degrees (triangle is part of the shape)
		if (AngleAToB(Vec1, Vec2) < PI/2) {
			TList.push_back(Triangle(i-1, i, i+1));
			VerticesIndices.erase(VerticesIndices.begin()+i);
			i--;
			if (IsPolygonConvex(VerticesIndices, M)) {
				return Triangulate(TList, VerticesIndices, M);
			}
		}
	}	

	return true;
}

/** \brief Reads in a face. If it is a regular Triangle it just creates a triangle otherwise it tries to Triangulate the face so that we only have triangles at the end 
 * \param Stream to read face from
 * \param M Mesh to add the triangle
 * \param Mat for the Face
 */
void MeshLoaderObj::ReadFace(istream& Stream, Mesh* M, Material& Mat) {
	Triangle t(0,0,0);
	ReadTriangle(&t, Stream);
	// found a triangle
	if (t.vert1 != INT_MIN) {
		M->_triangles.push_back(t);
		M->_material.push_back(Mat);
		//DLOG(INFO) << "Found Face " << t.vert1 << ", " << t.vert2 << ", " << t.vert3 << endl;
		float* CalculatedNormal = new float[3];
		m3dFindNormal(CalculatedNormal, M->_vertices[t.vert1], M->_vertices[t.vert2], M->_vertices[t.vert3]);
		M->_normals.push_back(CalculatedNormal);
		//DLOG(INFO) << "Calculated Normal " << CalculatedNormal[0] << ", " << CalculatedNormal[1] << ", " << CalculatedNormal[2] << endl;
	// It is something of higher order so we triangulate
	} else {
		vector<Triangle> TList;
		if (!Triangulate(TList, Stream, M)) {
			Stream.clear(ios::badbit);
		} else {
			// Add all Triangles
			for (int i=0; i<TList.size(); i++) {
				t = TList[i];
				M->_triangles.push_back(t);
				M->_material.push_back(Mat);
				//DLOG(INFO) << "Found Triangulated Face " << t.vert1 << ", " << t.vert2 << ", " << t.vert3 << endl;
				float* CalculatedNormal = new float[3];
				m3dFindNormal(CalculatedNormal, M->_vertices[t.vert1], M->_vertices[t.vert2], M->_vertices[t.vert3]);
				M->_normals.push_back(CalculatedNormal);
				//DLOG(INFO) << "Calculated Normal " << CalculatedNormal[0] << ", " << CalculatedNormal[1] << ", " << CalculatedNormal[2] << endl;
			}
		}
	}
}
