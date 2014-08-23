//nee a table of TV faces

#ifndef __TVDATA__H
#define __TVDATA__H

class UnwrapMod;

class UVW_TVFaceOldClass
{
public:
int t[4];
int FaceIndex;
int MatID;
int flags;
Point3 pt[4];
};

class UVW_TVVectorClass
{
public:
//index into the texture list
	int handles[8];
	int interiors[4];
//index into the geometric list
	int vhandles[8];
	int vinteriors[4];

	UVW_TVVectorClass()
		{
		for (int i = 0; i < 4; i++)
			{
			interiors[i] = -1;
			vinteriors[i] = -1;
			}
		for (i = 0; i < 8; i++)
			{
			handles[i] = -1;
			vhandles[i] = -1;
			}

		}


};


class UVW_TVFaceClass
{
public:

//index into the texture vertlist
	int *t;
	int FaceIndex;
	int MatID;
	int flags;
//count of number of vertex currently can only 3 or 4 but with poly objects this will change
	int count;
//index into the geometric vertlist
	int *v;
	UVW_TVVectorClass *vecs;

	UVW_TVFaceClass()
		{
		count = 3;
		flags = 0;
		vecs = NULL;
		t = NULL;
		v = NULL;
		}
	~UVW_TVFaceClass()
		{
		if (vecs) delete vecs;
		vecs = NULL;
		if (t) delete [] t;
		t = NULL;
		if (v) delete [] v;
		t = NULL;

		}
	UVW_TVFaceClass* Clone();
	void DeleteVec();

	ULONG SaveFace(ISave *isave);
	ULONG SaveFace(FILE *file);


	ULONG LoadFace(ILoad *iload);
	ULONG LoadFace(FILE *file);

};



//need a table of TVert pointers
class UVW_TVVertClass
{
public:
	Point3 p;
	float influence;
	BYTE flags;
};

//need a table of edgs
class UVW_TVEdgeDataClass
{
public:
//indices into the vert list
	int a,avec;
	int b,bvec;
	int flags;
	
	Tab<int> faceList;

	UVW_TVEdgeDataClass()
		{
		
		}
	~UVW_TVEdgeDataClass()
		{
		}
};

class UVW_TVEdgeClass
	{
public:
	Tab<UVW_TVEdgeDataClass*> data;
	BYTE flags;
	};

class UVW_ChannelClass
{
public:
	int channel;
	Tab<UVW_TVVertClass> v;
	Tab<UVW_TVFaceClass*> f;
	Tab<UVW_TVEdgeClass*> e;
	Tab<UVW_TVEdgeDataClass*> ePtrList;
	

	Tab<Control*> cont;		
	Tab<Point3> geomPoints;


	ULONG LoadFaces(ILoad *iload);
	ULONG LoadFaces(FILE *file);

	ULONG SaveFaces(ISave *isave);
	ULONG SaveFaces(FILE *file);


	void SetCountFaces(int newct);

	void CloneFaces(Tab<UVW_TVFaceClass*> &t);
	void AssignFaces(Tab<UVW_TVFaceClass*> &t);
	void FreeFaces();
	void Dump();

	void MarkDeadVertices();

	BOOL edgesValid;
	void FreeEdges();
	void BuildEdges();
	
	void AppendEdge(int index1,int vec1, int index2,int vec2, int face, BOOL hidden);

	float LineToPoint(Point3 p1, Point3 l1, Point3 l2);
	int EdgeIntersect(Point3 p, float threshold, int i1, int i2);

};


class VertexLookUpDataClass
{
public:
int index;
int newindex;
Point3 p;

};
class VertexLookUpListClass
{
public:
BitArray sel;
Tab<VertexLookUpDataClass> d;
void addPoint(int a_index, Point3 a);
};


#endif // __UWNRAP__H
