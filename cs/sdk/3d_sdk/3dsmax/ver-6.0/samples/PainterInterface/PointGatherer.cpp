#include "painterInterface.h"


float PointGatherer::LineToPoint(Point3 p1, Point3 l1, Point3 l2, float &u)
{

if (l1 == l2)
	{
	u = 0.0001f;
	return Length(p1-l1);
	}
Point3 VectorA,VectorB,VectorC;
double Angle;
double dist = 0.0f;
VectorA = l2-l1;
VectorB = p1-l1;
float dot = DotProd(Normalize(VectorA),Normalize(VectorB));
if (dot == 1.0f) dot = 0.99f;
Angle =  acos(dot);
if (Angle > (3.14/2.0))
	{
	dist = Length(p1-l1);
	u = 0.0001f;
	}
else
	{
	VectorA = l1-l2;
	VectorB = p1-l2;
	dot = DotProd(Normalize(VectorA),Normalize(VectorB));
	if (dot == 1.0f) dot = 0.99f;
	Angle = acos(dot);
	if (Angle > (3.14/2.0))
		{
		dist = Length(p1-l2);
		u = 0.9999f;
		}
		else
		{
		double hyp;
		hyp = Length(VectorB);
		dist =  sin(Angle) * hyp;
		double du =  (cos(Angle) * hyp);
		double a = Length(VectorA);
		if ( a== 0.0f)
			return 0.0001f;
		else u = (float)((a-du) / a);

		}

	}

return (float) dist;

}


void PointGatherer::CreateCells(int width, Box3 box)
{
//	FreeCells();
	CellDataXY.SetCount(width*width);
	CellDataYZ.SetCount(width*width);
	CellDataXZ.SetCount(width*width);

	this->width = width;
	int w = width*width;
	for (int i =0; i < w; i++)
		{
		CellDataXY[i] = NULL;
		CellDataYZ[i] = NULL;
		CellDataXZ[i] = NULL;
		}
	upperLeft.x = box.pmin.x;
	upperLeft.y = box.pmin.y;
	upperLeft.z = box.pmin.z;
	xOffset = (box.pmax.x - box.pmin.x)/(width); 
	yOffset = (box.pmax.y - box.pmin.y)/(width); 
	zOffset = (box.pmax.z - box.pmin.z)/(width); 

/*
DebugPrint("width %d upperLeft %f %f %f   offx %f offy %f offz %f\n",
		   width,
		   upperLeft.x,upperLeft.y,upperLeft.z,
		   xOffset,yOffset,zOffset
		   );
*/

}

void PointGatherer::FreeCells(BOOL nukeCustomPoints)
{
	for (int i = 0; i < cellData.Count(); i++)
		{
		if (cellData[i].p)
			{
			delete [] cellData[i].p;
			cellData[i].p = NULL;
			}
		if (cellData[i].count)
			{
			delete [] cellData[i].count;
			cellData[i].count = NULL;
			}
		if (cellData[i].weight)
			{
			delete [] cellData[i].weight;
			cellData[i].weight = NULL;
			}
		if (cellData[i].str)
			{
			delete [] cellData[i].str;
			cellData[i].str = NULL;
			}
		if (cellData[i].u)
			{
			delete [] cellData[i].u;
			cellData[i].u = NULL;
			}


		if (cellData[i].isMirror)
			{
			delete [] cellData[i].isMirror;
			cellData[i].isMirror = NULL;
			}


		if (pointGatherHitVerts[i])
			{
			delete pointGatherHitVerts[i];
			pointGatherHitVerts[i] = NULL;
			}

		}

	int w = width*width;
	for (i =0; i < CellDataXY.Count(); i++)
		{
		if (CellDataXY[i]) delete CellDataXY[i];
		if (CellDataYZ[i]) delete CellDataYZ[i];
		if (CellDataXZ[i]) delete CellDataXZ[i];

		}

	CellDataXY.ZeroCount();
	CellDataYZ.ZeroCount();
	CellDataXZ.ZeroCount();
}

void PointGatherer::ClearWeights()
	{

	

	for (int i = 0; i < cellData.Count(); i++)
		{
		pointGatherHitVerts[i]->ClearAll();
		int pointCount = cellElementCount[i];
		float *weights = cellData[i].weight;
		float *u = cellData[i].u;
		float *str = cellData[i].str;
		int *counts = cellData[i].count;

		BOOL *isMirror = cellData[i].isMirror;
		
		for (int j = 0; j < pointCount; j++)
			{
			*weights = 0.0f;
			*u = 0.0f;
			*str = 0.0f;
			*counts = 0;
			*isMirror = FALSE;
			isMirror++;
			weights++;
			counts++;
			str++;
			u++;
			}
		}
	}

void PointGatherer::SetNumberNodes(int ct)
{
FreeCells();
cellData.SetCount(ct);
cellElementCount.SetCount(ct);
pointGatherHitVerts.SetCount(ct);
for (int i = 0; i < cellData.Count(); i++)
	{
	cellElementCount[i] = 0;
	cellData[i].p = NULL;
	cellData[i].count = NULL;
	cellData[i].weight = NULL;
	cellData[i].u = NULL;
	cellData[i].isMirror = NULL;

	cellData[i].str = NULL;
	pointGatherHitVerts[i] = NULL;
	}
}


void PointGatherer::SetCount(int index, int ct)
	{
	if (cellData[index].p)
		{
		delete [] cellData[index].p;
		cellData[index].p = NULL;
		}
	if (cellData[index].count)
		{
		delete [] cellData[index].count;
		cellData[index].count = NULL;
		}
	if (cellData[index].weight)
		{
		delete [] cellData[index].weight;
		cellData[index].weight = NULL;
		}
	if (cellData[index].str)
		{
		delete [] cellData[index].str;
		cellData[index].str = NULL;
		}
	if (cellData[index].u)
		{
		delete [] cellData[index].u;
		cellData[index].u = NULL;
		}
	if (cellData[index].isMirror)
		{
		delete [] cellData[index].isMirror;
		cellData[index].isMirror = NULL;
		}
	if (pointGatherHitVerts[index])
		{
		delete pointGatherHitVerts[index];
		pointGatherHitVerts[index] = NULL;
		}


	cellElementCount[index] = ct;
	cellData[index].p = new Point3[ct];
	cellData[index].weight = new float[ct];
	cellData[index].str = new float[ct];
	cellData[index].u = new float[ct];
	cellData[index].count = new int[ct];
	cellData[index].isMirror = new BOOL[ct];
	pointGatherHitVerts[index] = new BitArray();
	pointGatherHitVerts[index]->SetSize(ct);
	pointGatherHitVerts[index]->ClearAll();

	}

void PointGatherer::Add(Point3 *p, int index, int nodeIndex)
{

cellData[nodeIndex].p[index] = *p;
cellData[nodeIndex].weight[index] = 0.0f;
cellData[nodeIndex].u[index] = 0.0f;
cellData[nodeIndex].str[index] = 0.0f;
cellData[nodeIndex].count[index] = 0;
cellData[nodeIndex].isMirror[index] = FALSE;

int x,y,z;
x = (p->x - upperLeft.x)/xOffset;
y = (p->y - upperLeft.y)/yOffset;
z = (p->z - upperLeft.z)/zOffset;

if (x >= width) x = width -1;
if (y >= width) y = width -1;
if (z >= width) z = width -1;


if (x < 0) x = 0;
if (y < 0) y = 0;
if (z < 0) z = 0;


if (CellDataXY[y*width+x]==NULL)
	CellDataXY[y*width+x] = new CellClass();
CellDataXY[y*width+x]->whichPoint.Append(1,&index,10);
CellDataXY[y*width+x]->whichNode.Append(1,&nodeIndex,10);

if (CellDataYZ[z*width+y]==NULL)
	CellDataYZ[z*width+y] = new CellClass();
CellDataYZ[z*width+y]->whichPoint.Append(1,&index,10);
CellDataYZ[z*width+y]->whichNode.Append(1,&nodeIndex,10);

if (CellDataXZ[z*width+x]==NULL)
	CellDataXZ[z*width+x] = new CellClass();
CellDataXZ[z*width+x]->whichPoint.Append(1,&index,10);
CellDataXZ[z*width+x]->whichNode.Append(1,&nodeIndex,10);

//DebugPrint("Point %f %f %f added to cell %d %d\n",p->x,p->y,p->z,x,y);

	
}

/*
PointGatherer::SetCustomPoints(int whichNode,int ct, Point3 *points)
{
if ((whichNode < 0) || (whichNode >= cellData.Count())) return 0;
if (cellData[whichNode].customPoints)
	delete [] cellData[whichNode].customPoints;
cellData[whichNode].customPointCount = ct;
if (ct ==0)
	cellData[whichNode].customPoints = NULL;
else
	{
	cellData[whichNode].customPoints = new Point3[ct];
	memcpy(cellData[whichNode].customPoints, points, ct * sizeof(Point3));
	SetCount(whichNode, ct);
	for (int i =0; i < ct; i++)
		{
		Add(&cellData[whichNode].customPoints[i], ct, whichNode);
		}
	}
return 1;
}
*/

void PointGatherer::AddWeightFromSegment(Point3 a, float radiusA, float strA,
									Point3 b, float radiusB, float strB, ICurve *pCurve,
									BOOL additveMode, BOOL isMirror,
									float currentLength)
{

	float segLength = Length(a-b);

//get the bounding box from the segment
	Box3 bounds;

	bounds.Init();

	bounds += a;
	bounds += b;
	if (radiusA > radiusB)
		bounds.EnlargeBy(radiusA);
	else bounds.EnlargeBy(radiusB);
	int startX, endX;
	int startY, endY;
	int startZ, endZ;
	
	startX = (bounds.pmin.x - upperLeft.x)/xOffset;
	startY= (bounds.pmin.y - upperLeft.y)/yOffset;
	startZ= (bounds.pmin.z - upperLeft.z)/zOffset;

	endX = (bounds.pmax.x - upperLeft.x)/xOffset;
	endY= (bounds.pmax.y - upperLeft.y)/yOffset;
	endZ= (bounds.pmax.z - upperLeft.z)/zOffset;

//need to constrains the indices since a brush envelope can exist past our bounds
	if (startX < 0) startX = 0;
	if (startX >=  width) startX = width-1;
	if (endX < 0) endX = 0;
	if (endX >=  width) endX = width-1;

	if (startY < 0) startY = 0;
	if (startY >=  width) startY = width-1;
	if (endY < 0) endY = 0;
	if (endY >=  width) endY = width-1;

	if (startZ < 0) startZ = 0;
	if (startZ >=  width) startZ = width-1;
	if (endZ < 0) endZ = 0;
	if (endZ >=  width) endZ = width-1;


//get all the cells that intersect with the bounding box
//loop through those points adding weight to em if they are within the envelope
	float u = 0.0f;
	float dist = 0.0f;
	float envDist = 0.0f;
/*
//Brute force method checks against every point
	for (int i = 0; i < cellData.Count(); i++)
		{

		for (int j = 0; j < cellElementCount[i]; j++)
			{
			Point3 p = *cellData[i].p[j];

			dist = LineToPoint(p,a,b,u);
				envDist = radiusA + ((radiusB-radiusA) * u);
				if (dist <= envDist)
					{
//DebugPrint("dist %f envdist %f\n",dist,envDist);
					float weight = 1.0f - dist/envDist;
					float str = (strA + ((strB-strA) * u)) * weight;
					float listWeight = cellData[i].weight[j];
					if (weight > listWeight) 
						{
						cellData[i].str[j] = str;
						cellData[i].weight[j] = weight;
						}
					
					}

			}	
		}
*/
//checks against cells first then those points only within the hit cells
//2 d gather
/*
	for (int i = startY; i <= endY; i++)
		{
		int currentCell = i * width;
		
		for (int j = startX; j <= endX; j++)
			{
			if (CellDataXY[currentCell+j])
				{
				for (int k = 0; k < CellDataXY[currentCell+j]->whichPoint.Count(); k++)
					{
					int whichPoint = CellDataXY[currentCell+j]->whichPoint[k];
					int whichNode = CellDataXY[currentCell+j]->whichNode[k];
					Point3 p = *cellData[whichNode].p[whichPoint];
					dist = LineToPoint(p,a,b,u);
					envDist = radiusA + ((radiusB-radiusA) * u);
					if (dist <= envDist)
						{
//DebugPrint("dist %f envdist %f\n",dist,envDist);
						float weight = 1.0f - dist/envDist;
						float str = (strA + ((strB-strA) * u)) * weight;
						float listWeight = cellData[whichNode].weight[whichPoint];
						if (weight > listWeight) 
							{
			 				cellData[whichNode].str[whichPoint] = str;
							cellData[whichNode].weight[whichPoint] = weight;
							}
						
						}
					}
				}
			}
		}
		*/


	for (int i = 0; i < cellData.Count(); i++)
		{
		int* countAmount = cellData[i].count;
		
		memset(countAmount,0,cellElementCount[i]*sizeof(int));
/*		for (int j = 0; j < cellElementCount[i]; j++)
			{
			*countAmount = 0;
			countAmount++;
			}
*/

		}
// 3 d gather
//in xy

	for (i = startY; i <= endY; i++)
		{
		int currentCell = i * width;
		
		for (int j = startX; j <= endX; j++)
			{
			if (CellDataXY[currentCell+j])
				{
				for (int k = 0; k < CellDataXY[currentCell+j]->whichPoint.Count(); k++)
					{
					int whichPoint = CellDataXY[currentCell+j]->whichPoint[k];
					int whichNode = CellDataXY[currentCell+j]->whichNode[k];
					cellData[whichNode].count[whichPoint]++;
					}
				}
			}
		}

//in yz
	for (i = startZ; i <= endZ; i++)
		{
		int currentCell = i * width;
		
		for (int j = startY; j <= endY; j++)
			{
			if (CellDataYZ[currentCell+j])
				{
				for (int k = 0; k < CellDataYZ[currentCell+j]->whichPoint.Count(); k++)
					{
					int whichPoint = CellDataYZ[currentCell+j]->whichPoint[k];
					int whichNode = CellDataYZ[currentCell+j]->whichNode[k];
					cellData[whichNode].count[whichPoint]++;
					}
				}
			}
		}

//in xz
	for ( i = startZ; i <= endZ; i++)
		{
		int currentCell = i * width;
		
		for (int j = startX; j <= endX; j++)
			{
			if (CellDataXZ[currentCell+j])
				{
				for (int k = 0; k < CellDataXZ[currentCell+j]->whichPoint.Count(); k++)
					{
					int whichPoint = CellDataXZ[currentCell+j]->whichPoint[k];
					int whichNode = CellDataXZ[currentCell+j]->whichNode[k];
					cellData[whichNode].count[whichPoint]++;
					}
				}
			}
		}
		
	for ( i = 0; i < cellData.Count(); i++)
		{
		int* countAmount = cellData[i].count;
		for (int j = 0; j < cellElementCount[i]; j++)
			{
			if (*countAmount == 3)
				{

				int whichPoint = j;
				int whichNode =i;
				Point3 p = cellData[whichNode].p[whichPoint];
				dist = LineToPoint(p,a,b,u);
				envDist = radiusA + ((radiusB-radiusA) * u);
				if (dist <= envDist)
					{
					float weight = 1.0f - dist/envDist;
					float str = (strA + ((strB-strA) * u));
					float listWeight = cellData[whichNode].weight[whichPoint];
					if (additveMode)
						{
			 			pointGatherHitVerts[whichNode]->Set(whichPoint,TRUE);
						cellData[whichNode].weight[whichPoint] += weight;
						weight =  pCurve->GetValue(0,1.0f-weight);
						cellData[whichNode].str[whichPoint] += str*weight;
						cellData[whichNode].u[whichPoint] = (currentLength + (u*segLength));

						}
					else if (weight > listWeight) 
						{
			 			pointGatherHitVerts[whichNode]->Set(whichPoint,TRUE);
						cellData[whichNode].weight[whichPoint] = weight;
						weight =  pCurve->GetValue(0,1.0f-weight);
						cellData[whichNode].str[whichPoint] = str*weight;
						cellData[whichNode].u[whichPoint] = (currentLength + (u*segLength));
						}
//if first mirror						
					if ((isMirror) && (cellData[whichNode].isMirror[whichPoint] == NO_MIRRROR))
						cellData[whichNode].isMirror[whichPoint] = MIRRRORED;
					else if ((!isMirror) && (cellData[whichNode].isMirror[whichPoint] == MIRRRORED))
						cellData[whichNode].isMirror[whichPoint] = MIRRROR_SHARED;
//if mirrored and now appliying a non mirror weight

						
					}
				}
			countAmount++;
			}
		}

//DebugPrint("Solving Stroke %f %f %f \n",a.x,a.y,a.z);
//DebugPrint("               %f %f %f \n",b.x,b.y,b.z);

}

float *PointGatherer::GetWeights(int whichNode, int &ct)
	{ 
	if (cellElementCount.Count() == 0) return NULL;

	ct = cellElementCount[whichNode];
	if (ct ==0) return NULL;
	else return cellData[whichNode].weight;
	}

float *PointGatherer::GetStr(int whichNode, int &ct)
	{ 
	if (cellElementCount.Count() == 0) return NULL;

	ct = cellElementCount[whichNode];

	if (ct ==0) return NULL;
	else return cellData[whichNode].str;
	}

BitArray *PointGatherer::GetHits(int whichNode)
	{
	if (cellElementCount.Count() == 0) return NULL;

	int ct = cellElementCount[whichNode];

	if (ct ==0) return NULL;
	else return pointGatherHitVerts[whichNode];

	}

int *PointGatherer::GetIsMirror(int whichNode,int &ct)
	
	{
	if (cellElementCount.Count() == 0) return NULL;

	ct = cellElementCount[whichNode];

	if (ct ==0) return NULL;
	else return cellData[whichNode].isMirror;

	}

float *PointGatherer::GetU(int whichNode,int &ct)
	{
	if (cellElementCount.Count() == 0) return NULL;

	ct = cellElementCount[whichNode];

	if (ct ==0) return NULL;
	else return cellData[whichNode].u;
	}