
#define MESH_CHUNK 0x1010

#define MESH_VERTEXCOUNT_CHUNK 0x1020
#define MESH_FACECOUNT_CHUNK 0x1030

#define MESH_VERTEXDATA_CHUNK 0x1040
#define MESH_FACEDATA_CHUNK 0x1050



class VNormal {
	public:
		Point3 norm;
		DWORD smooth;
		VNormal *next;
		BOOL init;



		VNormal() {smooth=0;next=NULL;init=FALSE;norm=Point3(0,0,0);}
		VNormal(Point3 &n,DWORD s) {next=NULL;init=TRUE;norm=n;smooth=s;}
		~VNormal() {delete next;}
		void AddNormal(Point3 &n,DWORD s);
		Point3 &GetNormal(DWORD s);
		void Normalize();
	};




class Leaf;

class BoundsList
{
public:
	int faceIndex;
	int nodeIndex;
	BoundsList()
		{
		faceIndex = -1;
		nodeIndex = -1;
		}
};

class Leaf
{

private:
	Box2D bounds[4];
	BOOL bottom;
	
public:

	Leaf *quad[4];
	Tab<BoundsList> faceIndex;

	Leaf(Point2 min, Point2 max)
		{
		bottom = FALSE;
		quad[0] = NULL;
		quad[1] = NULL;
		quad[2] = NULL;
		quad[3] = NULL;
		Point2 mid = (min+max)*0.5f;

		bounds[0].min = min;
		bounds[0].max = mid;

		bounds[1].min.x = mid.x;
		bounds[1].min.y = min.y;
		bounds[1].max.x = max.x;
		bounds[1].max.y = mid.y;

		bounds[2].min.x = min.x;
		bounds[2].min.y = mid.y;
		bounds[2].max.x = mid.x;
		bounds[2].max.y = max.y;

		bounds[3].min = mid;
		bounds[3].max = max;

		}
	BOOL InQuad(int i, Box2D box)
		{
		Box2D b = bounds[i];
		if (box.max.x < b.min.x) return FALSE;
		if (box.min.x > b.max.x) return FALSE;

		if (box.max.y < b.min.y) return FALSE;
		if (box.min.y > b.max.y) return FALSE;

		return TRUE;

		}
	int InWhichQuad(Point3 p)
		{
		if ( (p.x >=bounds[0].min.x) && (p.x < bounds[0].max.x) &&
			 (p.y >=bounds[0].min.y) && (p.y < bounds[0].max.y) )
			 return 0;
		else if ( (p.x >=bounds[1].min.x) && (p.x < bounds[1].max.x) &&
			 (p.y >=bounds[1].min.y) && (p.y < bounds[1].max.y) )
			 return 1;
		else if ( (p.x >=bounds[2].min.x) && (p.x < bounds[2].max.x) &&
			 (p.y >=bounds[2].min.y) && (p.y < bounds[2].max.y) )
			 return 2;
		else if ( (p.x >=bounds[3].min.x) && (p.x < bounds[3].max.x) &&
			 (p.y >=bounds[3].min.y) && (p.y < bounds[3].max.y) )
			 return 3;
		return -1;
		}

	Box2D GetBounds(int i) {	if ( (i>=0) && (i<4) ) 
								return bounds[i];
								else return Box2D();}
	
	Leaf *GetQuad(int i) {	if ( (i>=0) && (i<4) ) 
							return quad[i];
							else return NULL;
						}
	void SetQuad(int i, Leaf *l) { if ( (i>=0) && (i<4) ) quad[i] = l;}

	BOOL IsBottom() {return bottom;}
	void SetBottom (BOOL b) { bottom = b;}

};


class LightMesh  //not really light but contains everything we need and no other extra data
{
public:
	Tab<Point3> vertsWorldSpace;
	Tab<Point3> vertsViewSpace;
	Tab<Face> faces;
	Tab<Box2D> boundingBoxList;
	BitArray faceVisible;
	Matrix3 toWorldSpace;
	Matrix3 toLocalSpace;
	Box3 bb;

	Tab<Point3> vnorms;  //these are only created if asked for

	void AddMesh(Mesh *m,  Matrix3 basetm, ViewExp *vpt, BOOL buildVNorms);

};

class BoundsTree
{

private:

	int depth;
	BOOL buildVNorms;
	Tab<LightMesh*> meshList;

	void LayoutTree(Leaf *l, int depth);

	void RecurseTree(Leaf *l, int index, Box2D box, int nodeID);
	void AddFace(int index, Box2D box, int nodeID);
	Box3 worldBB;

public:

	Point3 GetWorldCenter();
	Box3  GetWorldBounds();
	Point3 *GetWorldData(int index,int &count);
	Point3 *GetNormals(int index,int &count);

	Leaf *head;


	void FreeCaches()
		{

		if (head) DeleteTree(head);
		head = NULL;
//delete meshList
		for (int i =0; i < meshList.Count(); i++)
			{
			if (meshList[i])
				{
				delete meshList[i];
				meshList[i] = NULL;
				}
			}


		}

	BoundsTree()
		{
		buildVNorms = FALSE;
		depth = 6;
		head = NULL;
		}

	~BoundsTree()
		{
		FreeCaches();
		}


	void DeleteTree(Leaf *l);

//make sure to free the caches before recreating meshdata
	void BuildMeshData(Mesh *m, Matrix3 basetm,  ViewExp *vpt, int index);
	void BuildQuadTree();



	BOOL TrimRecurseTree(Leaf *l);
	void TrimTree();

	BOOL IsInsideMesh(Point3 hitPoint);


	BOOL HitQuadTree(IPoint2 m, int &nindex, DWORD &findex, Point3 &p, Point3 &norm, Point3 &bary, float &z, Matrix3 &toWorldTm);



	void SetDepth(int d) { depth = d;}
	void SetBuildVNorms(BOOL build) { buildVNorms = build;}

};

