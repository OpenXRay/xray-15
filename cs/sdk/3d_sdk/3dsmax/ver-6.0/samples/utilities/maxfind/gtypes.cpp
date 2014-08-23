/****************************************************************************
 MAX File Finder
 Christer Janson
 September 20, 1998
 GTypes.cpp - Some useful data types
 ***************************************************************************/

#include "pch.h"
#include <stdlib.h>
#include <math.h>
#include "gtypes.h"

//***************************************************************************
//
// Point3
//
//****************************************************************************

Point3::Point3()
{
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
}

Point3::Point3(float ix, float iy, float iz)
{
	x = ix;
	y = iy;
	z = iz;
}

Point3 Point3::operator / (float f)
{
	return Point3(x/f, y/f, z/f);
}

Point3& Point3::operator += (Point3 p)
{
	x += p.x;
	y += p.y;
	z += p.z;
	return *this;
}

Point3& Point3::operator /= (float f)
{
	x /= f;
	y /= f;
	z /= f;
	return *this;
}

//***************************************************************************
//
// Box3
//
//****************************************************************************

Box3::Box3()
{
	SetEmpty();
}

Box3::Box3(Point3 pMin, Point3 pMax)
{
	min = pMin;
	max = pMax;
}

void Box3::SetEmpty()
{
	min = Point3( BIGFLOAT,  BIGFLOAT,  BIGFLOAT);
	max = Point3(-BIGFLOAT, -BIGFLOAT, -BIGFLOAT);
}

BOOL Box3::IsEmpty()
{
	BOOL bEmpty = TRUE;

	if (max.x > min.x)
		bEmpty = FALSE;
	if (max.y > min.y)
		bEmpty = FALSE;
	if (max.y > min.y)
		bEmpty = FALSE;
	return bEmpty;
}

void Box3::IncludePoint(Point3 p)
{
	if (p.x < min.x)
		min.x = p.x;
	if (p.y < min.y)
		min.y = p.y;
	if (p.z < min.z)
		min.z = p.z;

	if (p.x > max.x)
		max.x = p.x;
	if (p.y > max.y)
		max.y = p.y;
	if (p.z > max.z)
		max.z = p.z;
}

void Box3::IncludeBox(Box3 b)
{
	IncludePoint(b.min);
	IncludePoint(b.max);
}

BOOL Box3::PointIncluded(Point3 p)
{
	BOOL bInside = FALSE;

	if ((p.x >= min.x) && (p.y >= min.y) && (p.z >= min.z) &&
		(p.x <= max.x) && (p.y <= max.y) && (p.z <= max.z)) {
		bInside = TRUE;
	}

	return bInside;
}

//***************************************************************************
//
// Face
//
//***************************************************************************

// Points p1, p2, & p3 specified in counter clock-wise order
void calcNormal(float v[3][3], float out[3])
{
	float v1[3],v2[3];
	static const int x = 0;
	static const int y = 1;
	static const int z = 2;

	// Calculate two vectors from the three points
	v1[x] = v[0][x] - v[1][x];
	v1[y] = v[0][y] - v[1][y];
	v1[z] = v[0][z] - v[1][z];

	v2[x] = v[1][x] - v[2][x];
	v2[y] = v[1][y] - v[2][y];
	v2[z] = v[1][z] - v[2][z];

	// Take the cross product of the two vectors to get
	// the normal vector which will be stored in out
	out[x] = v1[y]*v2[z] - v1[z]*v2[y];
	out[y] = v1[z]*v2[x] - v1[x]*v2[z];
	out[z] = v1[x]*v2[y] - v1[y]*v2[x];

	// Normalize the vector (shorten length to one)
    //
    float l = (float)sqrt(out[0]*out[0]+out[1]*out[1]+out[2]*out[2]);
	out[0] /= l;
	out[1] /= l;
	out[2] /= l;
}

void Face::CalcNormal(Vertex* vertArray)
{
	float n[3];
	float p[3][3];

	p[0][0] = vertArray[v[0]].pos.x;
	p[0][1] = vertArray[v[0]].pos.y;
	p[0][2] = vertArray[v[0]].pos.z;

	p[1][0] = vertArray[v[1]].pos.x;
	p[1][1] = vertArray[v[1]].pos.y;
	p[1][2] = vertArray[v[1]].pos.z;

	p[2][0] = vertArray[v[2]].pos.x;
	p[2][1] = vertArray[v[2]].pos.y;
	p[2][2] = vertArray[v[2]].pos.z;

	calcNormal(p, n);

	normal.x = n[0];
	normal.y = n[1];
	normal.z = n[2];
}

Point3 Face::Normal()
{
	return normal;
}



//***************************************************************************
//
// TriMesh
//
//***************************************************************************

TriMesh::TriMesh()
{
	numVerts = 0;
	numFaces = 0;
	vertArray = NULL;
	faceArray = NULL;
}

TriMesh::~TriMesh()
{
	delete [] vertArray;
	delete [] faceArray;
}

void TriMesh::SetNumVerts(int n)
{
	delete [] vertArray;

	vertArray = new Vertex[n];
	numVerts = n;
}

void TriMesh::SetNumFaces(int n)
{
	delete [] faceArray;

	faceArray = new Face[n];
	numFaces = n;
}

int TriMesh::GetNumVerts()
{
	return numVerts;
}

int TriMesh::GetNumFaces()
{
	return numFaces;
}

void TriMesh::SetVert(int n, Vertex v)
{
	vertArray[n] = v;
}

void TriMesh::SetVert(int n, Point3 p)
{
	vertArray[n].pos = p;
}

void TriMesh::SetFace(int n, Face f)
{
	faceArray[n] = f;
}

Vertex& TriMesh::GetVert(int n)
{
	return vertArray[n];
}

Face& TriMesh::GetFace(int n)
{
	return faceArray[n];
}

void TriMesh::Normalize(float factor)
{
	float scale = 0.0f;

	if (scale == 0.0f) {
		for (int v = 0; v < numVerts; v++) {
			if (scale > vertArray[v].pos.x || scale > vertArray[v].pos.y || scale > vertArray[v].pos.z)
				scale = (float)max(vertArray[v].pos.x, (float)max(vertArray[v].pos.y, vertArray[v].pos.z));
		}
	}

	for (int v = 0; v < numVerts; v++) {
		vertArray[v].pos.x /= factor;
		vertArray[v].pos.y /= factor;
		vertArray[v].pos.z /= factor;
	}
}

void TriMesh::BuildNormals()
{
#if 1
	int f, v;
	int* numNormals = new int[numVerts];

	for (v = 0; v < numVerts; v++) {
		numNormals[v] = 0;
	}

	for (f = 0; f < numFaces; f++) {
		faceArray[f].CalcNormal(vertArray);
		for (int c = 0; c < 3; c++) {
			int vert = faceArray[f].v[c];
			vertArray[vert].normal += faceArray[f].normal;
			numNormals[vert]++;
		}
	}

	for (v = 0; v < numVerts; v++) {
		vertArray[v].normal /= (float)numNormals[v];
		vertArray[v].normal.normalize();
	}

	delete [] numNormals;
#endif
}