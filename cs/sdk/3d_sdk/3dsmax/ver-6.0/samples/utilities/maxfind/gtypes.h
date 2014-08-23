/****************************************************************************
 MAX File Finder
 Christer Janson
 September 20, 1998
 GTypes.h - Some useful data types
 ***************************************************************************/

#ifndef __GTYPES_H__
#define __GTYPES_H__

#define BIGFLOAT 1.0e30f

class Point3 {
	public:

	Point3();
	Point3(float ix, float iy, float iz);

	Point3	operator / (float f);
	Point3&	operator += (Point3 p);
	Point3&	operator /= (float f);

	float	length()	{ return (float)sqrt(x*x + y*y + z*z); }
	Point3&	normalize()
	{
		float len = length();
		x /= len;
		y /= len;
		z /= len;
		return *this;
	}


	float x, y, z;
};

typedef Point3 Color3;

class Box3 {
	public:
	Box3();
	Box3(Point3 pMin, Point3 pMax);

	void	SetEmpty();
	BOOL	IsEmpty();
	void	IncludePoint(Point3 p);
	void	IncludeBox(Box3 b);
	BOOL	PointIncluded(Point3 p);

	Point3	min;
	Point3	max;
};

class Vertex {
public:
	Point3	pos;
	Point3	normal;
};

class Face {
public:
	int		v[3];

	void	CalcNormal(Vertex* vertArray);
	Point3	Normal();

//private:
	Point3	normal;
};


class TriMesh {
public:
	TriMesh();
	~TriMesh();
	void	SetNumVerts(int n);
	void	SetNumFaces(int n);
	int		GetNumVerts();
	int		GetNumFaces();
	void	SetVert(int n, Vertex p);
	void	SetVert(int n, Point3 p);
	void	SetFace(int n, Face f);
	Vertex&	GetVert(int n);
	Face&	GetFace(int n);
	void	Normalize(float factor = 0.0f);
	void	BuildNormals();

private:
	int		numVerts;
	int		numFaces;
	Vertex*	vertArray;
	Face*	faceArray;
};

#endif // __GTYPES_H__
