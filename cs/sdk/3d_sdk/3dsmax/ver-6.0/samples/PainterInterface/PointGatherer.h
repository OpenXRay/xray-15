


typedef struct celldata_struct
	{
	Point3  *p;
	int		*count;
	float	*weight;
	float	*str;
	float	*u;

	int	*isMirror;

//	int customPointCount;
//	Point3  *customPoints;
}  CellDataStruct;



typedef struct custompointsdata_struct
	{
	Point3  *p;
}  CustomPointDataStruct;



class CellClass
{
public:
	Tab<int>  whichNode;
	Tab<int>  whichPoint;
};

class PointGatherer
{
public:
	PointGatherer()
		{
		width = 0;
		upperLeft = Point3 (0.0f,0.0f,0.0f);
		}
	~PointGatherer()
		{
		FreeCells(TRUE);
		}

	void CreateCells(int width, Box3 box);
	void FreeCells(BOOL nukeCustomPoints = FALSE);

	void ClearWeights();


	void SetNumberNodes(int ct);
	void SetCount(int index, int ct);
	void Add(Point3 *p, int index, int nodeIndex);

//	SetCustomPoints(int whichNode,int ct, Point3 *points);
	
	float LineToPoint(Point3 p1, Point3 l1, Point3 l2, float &u);

	void AddWeightFromSegment(Point3 a, float radiusA, float strA,
						 Point3 b, float radiusB, float strB, ICurve *pCurve, BOOL additive, BOOL isMirror,
						 float currentLength);

	float *GetWeights(int whichNode,int &ct); 
	float *GetStr(int whichNode,int &ct);
	float *GetU(int whichNode,int &ct);
	int *GetIsMirror(int whichNode,int &ct);

	

	BitArray *GetHits(int whichNode);

	

private:

	Tab<int>		cellElementCount;

	Tab<CellDataStruct> cellData;

	Tab<CellClass*> CellDataXY;
	Tab<CellClass *> CellDataYZ;
	Tab<CellClass *> CellDataXZ;

	Tab<BitArray*> pointGatherHitVerts;

	int width;


	Point3 upperLeft;
	float xOffset, yOffset,zOffset;


	
};
