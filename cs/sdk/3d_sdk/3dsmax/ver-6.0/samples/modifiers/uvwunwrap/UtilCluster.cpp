#include "unwrap.h"

//these are just debug globals so I can stuff data into to draw
#ifdef DEBUGMODE
//just some pos tabs to draw bounding volumes
extern Tab<float> jointClusterBoxX;
extern Tab<float> jointClusterBoxY;
extern Tab<float> jointClusterBoxW;
extern Tab<float> jointClusterBoxH;


extern float hitClusterBoxX,hitClusterBoxY,hitClusterBoxW,hitClusterBoxH;

extern int currentCluster, subCluster;

//used to turn off the regular display and only show debug display
extern BOOL drawOnlyBounds;
#endif


static int CompTableArea( const void *elem1, const void *elem2 ) {
	ClusterClass **ta = (ClusterClass **)elem1;
	ClusterClass **tb = (ClusterClass **)elem2;

	ClusterClass *a = *ta;
	ClusterClass *b = *tb;

	float aH, bH;
	aH = a->w * a->h;
	bH = b->w * b->h;

	if (aH == bH) return 0;
	else if (aH > bH) return -1;
	else return 1;
	
}

float AreaOfTriangle(Point3 a, Point3 b, Point3 c)

{
float area;
float s;
float al,bl,cl;
al = Length(b-a);
bl = Length(c-b);
cl = Length(a-c);
s = (al+bl+cl)/2.0f; 
area = (float)sqrt((double)(s*(s-al)*(s-bl)*(s-cl))) ;
return area;


}

float AreaOfPolygon(Tab<Point3> &points)

{
float area = 0.0f;

for (int i = 0; i < (points.Count()-1); i++)
	{	
	int j = i + 1;
	area += points[i].x + points[j].y;
	area -= points[j].x - points[i].y;
	}
area *= 0.5f;
return area;


}


//this just though cluster building a bounding box for each face that is in this cluster
void ClusterClass::BuildList(UVW_ChannelClass &TVMaps)
	{
	int ct = faces.Count();
	boundsList.SetCount(ct);
	for (int i = 0; i < ct; i++)
		{
		boundsList[i].Init();
		int faceIndex = faces[i];
		for (int k = 0; k <  TVMaps.f[faceIndex]->count; k++)
			{
//need to put patch handles in here also
			int index = TVMaps.f[faceIndex]->t[k];
			Point3 a = TVMaps.v[index].p;
			boundsList[i] += a;

			if ( (TVMaps.f[faceIndex]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[faceIndex]->vecs))
					{
					int vertIndex = TVMaps.f[faceIndex]->vecs->interiors[k];
					if (vertIndex >=0) 
						{
						Point3 a = TVMaps.v[vertIndex].p;
						boundsList[i] += a;
						}
						
					vertIndex = TVMaps.f[faceIndex]->vecs->handles[k*2];
					if (vertIndex >=0)
						{
						Point3 a = TVMaps.v[vertIndex].p;
						boundsList[i] += a;
						}
						
					vertIndex = TVMaps.f[faceIndex]->vecs->handles[k*2+1];
					if (vertIndex >= 0 )
						{
						Point3 a = TVMaps.v[vertIndex].p;
						boundsList[i] += a;
						}
	
					}

			}
		boundsList[i].pmin.z = -1.0f;
		boundsList[i].pmax.z = 1.0f;
		if ((boundsList[i].pmax.x-boundsList[i].pmin.x) == 0.0f)
			{
			boundsList[i].pmax.x += 0.5f;
			boundsList[i].pmin.x -= 0.5f;
			}

		if ((boundsList[i].pmax.y-boundsList[i].pmin.y) == 0.0f)
			{
			boundsList[i].pmax.y += 0.5f;
			boundsList[i].pmin.y -= 0.5f;
			}

		}
	}

BOOL ClusterClass::DoesIntersect(float x, float y, float w, float h)
	{
	int ct = boundsList.Count();
	Point3 a1,b1,c1,d1;
	float miny, minx, maxx,maxy;
	minx = x;
	miny = y;
	maxx = x+w;
	maxy = y+h;
	
	for (int i = 0; i < ct; i++)
		{

		BOOL intersect = TRUE;
		Box3 b = boundsList[i];
		float bminy, bminx, bmaxx,bmaxy;
		bminx = b.pmin.x;
		bminy = b.pmin.y;
		bmaxx = b.pmax.x;
		bmaxy = b.pmax.y;

		if (minx > bmaxx) 
			intersect = FALSE;
		if (miny > bmaxy) 
			intersect = FALSE;

		if (maxx < bminx) 
			intersect = FALSE;
		if (maxy < bminy) 
			intersect = FALSE;

		if (intersect) return TRUE;
		

		}


	return FALSE;
	}

int ClusterClass::ComputeOpenSpaces(float spacing)
	{

#ifdef DEBUGMODE
	hitClusterBoxX = bounds.pmin.x;
	hitClusterBoxY = bounds.pmin.y;
	hitClusterBoxW = w;
	hitClusterBoxH = h;

	jointClusterBoxX.ZeroCount();
	jointClusterBoxY.ZeroCount();
	jointClusterBoxW.ZeroCount();
	jointClusterBoxH.ZeroCount();
#endif 

	float boundsArea = w*h;

	float perCoverage = surfaceArea/boundsArea;

	if (perCoverage > 0.95f) return 0;

//right now we split the cluster into a 200x200 grid to look for open spots
	int iGridW,iGridH;
	iGridW = 100;
	iGridH = 100;
	float fEdgeLenW = w/(float) (iGridW);
	float fEdgeLenH = h/(float) (iGridH);
		
	int area = iGridW * iGridW;

	BitArray tusedList;
	tusedList.SetSize(area);
	tusedList.ClearAll();

	float fX,fY;
	fY = bounds.pmin.y;
//loop through bitmap marking used areas

	for (int j =0; j < iGridH; j++)
		{
		fX = bounds.pmin.x;

		for (int k =0; k < iGridW; k++)
			{
			if (DoesIntersect(fX-spacing,fY-spacing,fEdgeLenW+spacing,fEdgeLenH+spacing))
				tusedList.Set(j*iGridW+k);
			fX += fEdgeLenW;
			}
		fY += fEdgeLenH;
		}


//now we need to find groups of bounding boxes
	int iX, iY;
	iX = 0;
	iY = 0;

	int iCX, iCY;
	int iDirY, iDirX;
	int iBoxX = 0, iBoxY = 0, iBoxW = 0, iBoxH = 0;

//start at the upp left corners

	iCX = 0;
	iCY = iGridW-1;
	iDirX = 1;
	iDirY = -1;
	iBoxX = 0;
	iBoxY = 0;
	iBoxW = 0;
	iBoxH = 0;


	FindRectDiag(iCX, iCY, iGridW, iGridH, iDirX, iDirY,tusedList, iBoxX, iBoxY, iBoxW, iBoxH);
	if ((iBoxW != 0) && (iBoxH != 0))
		{
		ClusterClass *tempCluster = this;
		SubClusterClass temp;
		temp.x = bounds.pmin.x+ (iBoxX*fEdgeLenW);
		temp.y = bounds.pmin.y+(iBoxY*fEdgeLenH);
		temp.w = iBoxW*fEdgeLenW;
		temp.h = iBoxH*fEdgeLenH;
		openSpaces.Append(1,&temp);
/*
#ifdef DEBUGMODE
		jointClusterBoxX.Append(1,&temp.x);
		jointClusterBoxY.Append(1,&temp.y);
		jointClusterBoxW.Append(1,&temp.w);
		jointClusterBoxH.Append(1,&temp.h);
#endif
*/
		}

//start at the upp right corners
	iCX = iGridW-1;
	iCY = iGridH-1;
	iDirX = -1;
	iDirY = -1;
	iBoxX = 0;
	iBoxY = 0;
	iBoxW = 0;
	iBoxH = 0;
	FindRectDiag(iCX, iCY, iGridW, iGridH, iDirX, iDirY,tusedList, iBoxX, iBoxY, iBoxW, iBoxH);
	if ((iBoxW != 0) && (iBoxH != 0))
		{
		ClusterClass *tempCluster = this;
		SubClusterClass temp;
		temp.x = bounds.pmin.x+ (iBoxX*fEdgeLenW);
		temp.y = bounds.pmin.y+(iBoxY*fEdgeLenH);
		temp.w = iBoxW*fEdgeLenW;
		temp.h = iBoxH*fEdgeLenH;
		openSpaces.Append(1,&temp);
/*
#ifdef DEBUGMODE
		jointClusterBoxX.Append(1,&temp.x);
		jointClusterBoxY.Append(1,&temp.y);
		jointClusterBoxW.Append(1,&temp.w);
		jointClusterBoxH.Append(1,&temp.h);
#endif
*/
		}


//start at the lower left corners
	iCX = 0;
	iCY = 0;
	iDirX = 1;
	iDirY = 1;
	iBoxX = 0;
	iBoxY = 0;
	iBoxW = 0;
	iBoxH = 0;
	FindRectDiag(iCX, iCY, iGridW, iGridH, iDirX, iDirY,tusedList, iBoxX, iBoxY, iBoxW, iBoxH);
	if ((iBoxW != 0) && (iBoxH != 0))
		{
		SubClusterClass temp;
		temp.x = bounds.pmin.x+ (iBoxX*fEdgeLenW);
		temp.y = bounds.pmin.y+(iBoxY*fEdgeLenH);
		temp.w = iBoxW*fEdgeLenW;
		temp.h = iBoxH*fEdgeLenH;
		openSpaces.Append(1,&temp);
/*
#ifdef DEBUGMODE
		jointClusterBoxX.Append(1,&temp.x);
		jointClusterBoxY.Append(1,&temp.y);
		jointClusterBoxW.Append(1,&temp.w);
		jointClusterBoxH.Append(1,&temp.h);
#endif
*/
		}

//start at the lower right corners
	iCX = iGridW-1;
	iCY = 0;
	iDirX = -1;
	iDirY = 1;
	iBoxX = 0;
	iBoxY = 0;
	iBoxW = 0;
	iBoxH = 0;
	FindRectDiag(iCX, iCY, iGridW, iGridH, iDirX, iDirY,tusedList, iBoxX, iBoxY, iBoxW, iBoxH);
	if ((iBoxW != 0) && (iBoxH != 0))
		{
		SubClusterClass temp;
		temp.x = bounds.pmin.x+ (iBoxX*fEdgeLenW);
		temp.y = bounds.pmin.y+(iBoxY*fEdgeLenH);
		temp.w = iBoxW*fEdgeLenW;
		temp.h = iBoxH*fEdgeLenH;
		openSpaces.Append(1,&temp);
/*
#ifdef DEBUGMODE
		jointClusterBoxX.Append(1,&temp.x);
		jointClusterBoxY.Append(1,&temp.y);
		jointClusterBoxW.Append(1,&temp.w);
		jointClusterBoxH.Append(1,&temp.h);
#endif
*/
		}


	//do center
	int iCenterX = iGridW/2;
	int iCenterY = iGridH/2;
	iBoxX = 0;
	iBoxY = 0;
	iBoxW = 0;
	iBoxH = 0;
	FindSquareX(iCenterX, iCenterY,iGridW, iGridH, tusedList, iBoxX, iBoxY, iBoxW, iBoxH);

	if ((iBoxW != 0) && (iBoxH != 0))
		{
		SubClusterClass temp;
		temp.x = bounds.pmin.x+ (iBoxX*fEdgeLenW);
		temp.y = bounds.pmin.y+(iBoxY*fEdgeLenH);
		temp.w = iBoxW*fEdgeLenW;
		temp.h = iBoxH*fEdgeLenH;
		openSpaces.Append(1,&temp);
//we fit so stuff it in the cluster
/*
#ifdef DEBUGMODE
		jointClusterBoxX.Append(1,&temp.x);
		jointClusterBoxY.Append(1,&temp.y);
		jointClusterBoxW.Append(1,&temp.w);
		jointClusterBoxH.Append(1,&temp.h);
#endif
*/
		}

	//now loop through all the remaining points


	for (j =0; j < iGridH; j++)
		{
		for (int k =0; k < iGridW; k++)
			{
			int index = j * iGridW + k;
			if (!tusedList[index])
				{
				iBoxX = 0;
				iBoxY = 0;
				iBoxW = 0;
				iBoxH = 0;
				FindRectDiag(k, j,iGridW, iGridH, 1,1, tusedList, iBoxX, iBoxY, iBoxW, iBoxH);

				if ((iBoxW >2 ) && (iBoxH > 2))
					{
					SubClusterClass temp;
					temp.x = bounds.pmin.x+ (iBoxX*fEdgeLenW);
					temp.y = bounds.pmin.y+(iBoxY*fEdgeLenH);
					temp.w = iBoxW*fEdgeLenW;
					temp.h = iBoxH*fEdgeLenH;
					openSpaces.Append(1,&temp);
//we fit so stuff it in the cluster
/*
#ifdef DEBUGMODE
					jointClusterBoxX.Append(1,&temp.x);
					jointClusterBoxY.Append(1,&temp.y);
					jointClusterBoxW.Append(1,&temp.w);
					jointClusterBoxH.Append(1,&temp.h);
#endif
*/


					}

				}

			}


		}

#ifdef DEBUGMODE
		
	jointClusterBoxX.SetCount(openSpaces.Count());
	jointClusterBoxY.SetCount(openSpaces.Count());
	jointClusterBoxW.SetCount(openSpaces.Count());
	jointClusterBoxH.SetCount(openSpaces.Count());

	for (j = 0; j < openSpaces.Count(); j++)
		{
		jointClusterBoxX[j] = openSpaces[j].x;
		jointClusterBoxY[j] = openSpaces[j].y;
		jointClusterBoxW[j] = openSpaces[j].w;
		jointClusterBoxH[j] = openSpaces[j].h;
		}
#endif
	return 1;
	}


#define MODE_CENTER		0
#define MODE_LEFTTOP	1
#define MODE_RIGHTTOP	2
#define MODE_LEFTBOTTOM 3
#define MODE_RIGHTBOTTOM 4

class WanderClass
	{
public:

	WanderClass(float x, float y, float dirX, float dirY, int mode)
		{
		this->x = x;
		this->y = y;
		this->dirX = dirX;
		this->dirY = dirY;
		this->mode = mode;
		rowHeight = -1.0f;
		}

	float GetX() {
				if (mode == MODE_CENTER)
					return (x - (w *0.5f));
				else if (mode == MODE_LEFTTOP)
					return (x) ;
				else if (mode == MODE_LEFTBOTTOM)
					return (x - w);
				else if (mode == MODE_RIGHTTOP)
					return (x) ;
				else if (mode == MODE_RIGHTBOTTOM)
					return (x - w) ;
				return x;
				};
	float GetY() {
				if (mode == MODE_CENTER)
					return (y - (y *0.5f));
				else if (mode == MODE_RIGHTBOTTOM)
					return (y) ;
				else if (mode == MODE_LEFTBOTTOM)
					return (y );
				else if (mode == MODE_RIGHTTOP)
					return (y - h) ;
				else if (mode == MODE_LEFTTOP)
					return (y - h) ;
				return y;
				};

	void SetW(float w) {this->w = w;}
	void SetH(float h) {this->h = h;}
	void Advance()
		{
		x += w * dirX;
		if (rowHeight < 0.0f)
			rowHeight = h;
		}
	void NextRow()
		{
		if (rowHeight > 0.0f)
			{
			y+= rowHeight*dirY;
			rowHeight = -1.0f;
			}
		}
	void ClearHit() { hit = FALSE; }
	void Hit() { hit = TRUE; }
	BOOL IsHit() { return hit; }

private:
	int mode;
	float x,y;
	float w,h;
	float dirX,dirY;
	float rowHeight;
	BOOL hit;
	
	};


void	ClusterClass::FindSquareX(int x, int y, int w, int h, BitArray &used, int &iretX, int &iretY, int &iretW, int &iretH)
	{	
// start in the w direction one
	int iAmount = 0;


	if (used[y*w+x])  return;

	BOOL done = FALSE;
	while (!done)
		{
		iAmount++;
		
//check if goes out of border
		if ((x-iAmount) < 0)
			done = TRUE;
		if ((y-iAmount) < 0)
			done = TRUE;
		if ((x+iAmount) >=w)
			done = TRUE;
		if ((y+iAmount) >=h)
			done = TRUE;
//now make sure alll pixels are unused
		if (!done)
			{
			int iStartX = x - iAmount;
			int iStartY = y + iAmount;
//top edge
			int iY = y - iAmount;
			int iStart = iStartX + (iY*w);
			for (int i = 0; i < iAmount*2 +1; i++)
				{
				if (used[iStart]) 
					{
					done = TRUE;
					i = iAmount*2 +1;
					}
				iStart++;
				}

//bottom edge
			iY = y + iAmount;
			iStart = iStartX + (iY*w);
			for (i = 0; i < iAmount*2 +1; i++)
				{
				if (used[iStart]) 
					{
					done = TRUE;
					i = iAmount*2 +1;
					}
				iStart++;
				}

//left edge
			iY = y - iAmount;
			iStart = iStartX + (iY*w);
			for (i = 0; i < iAmount*2 +1; i++)
				{
				if (used[iStart]) 
					{
					done = TRUE;
					i = iAmount*2 +1;
					}
				iStart+=w;
				}
//left edge
			iY = y - iAmount;
			iStart = (x+iAmount) + (iY*w);
			for (i = 0; i < iAmount*2 +1; i++)
				{
				if (used[iStart]) 
					{
					done = TRUE;
					i = iAmount*2 +1;
					}
				iStart+=w;
				}
			}


		}
	iAmount--;
	iretX = x - iAmount;
	iretY = y - iAmount;
	iretW = iAmount*2+1;
	iretH = iAmount*2+1;

	for (int i = iretY; i < (iretY+iretH); i++)
		{
		int iStart =  i*w + iretX;
		for (int j = iretX; j < (iretX+iretW); j++)
			{
			used.Set(iStart);
			iStart++;
			}
		}

	}
void	ClusterClass::FindRectDiag(int x, int y, int w, int h, 
								int dirX, int dirY,
								BitArray &used, int &iretX, int &iretY, int &iretW, int &iretH)
	{	
	if (used[y*w+x])  return;

	int iAmount = 0;
	int iXAdditionalAmount = 0;
	int iYAdditionalAmount = 0;
	BOOL done = FALSE;
//start walking a diag till a hit
	int iX, iY;
	iX = x;
	iY = y;
	BOOL bHitSide = FALSE;
	BOOL bHitTop = FALSE;
	int sidecount =0;
	int topcount =0;
	while (!done)
		{
		iAmount++;
		iX = x + (iAmount*dirX);
		iY = y + (iAmount*dirY);

//check if goes out of border
		if (iX < 0)
			{
			done = TRUE;
			bHitSide = TRUE;
			sidecount++;
			}
		if (iY < 0)
			{
			done = TRUE;
			bHitTop = TRUE;
			topcount++;
			}
		if (iX >=w)
			{
			done = TRUE;
			bHitSide = TRUE;
			sidecount++;
			}
		if (iY >=h)
			{
			done = TRUE;
			bHitTop = TRUE;
			topcount++;
			}
//top/bottom edge
		if (!done)
			{
			int iStartX, iEndX;
			iStartX = x;
			iEndX = iX-(dirX);

			int iStartY, iEndY;
			iStartY = y;
			iEndY = iY-(dirY);



			if (iStartX > iEndX)
				{
				int temp = iStartX;
				iStartX = iEndX;
				iEndX = temp;
				}

			if (iStartY > iEndY)
				{
				int temp = iStartY;
				iStartY = iEndY;
				iEndY = temp;
				}


			int iStart = (iY*w) + iStartX ;


			for (int i = iStartX; i < (iEndX+1);i++)
				{
				if (used[iStart]) 
					{
					done = TRUE;
					bHitTop = TRUE;
					topcount++;
					}
//				else 
//					used.Set(iStart);
				iStart++;


				}
			
///left/right edge

			iStart = (iStartY*w) + iX ;
			for (i = iStartY; i < (iEndY+1);i++)
				{
				if (used[iStart]) 
					{
					done = TRUE;
					bHitSide = TRUE;
					sidecount++;
					}
//				else 
//					used.Set(iStart);
				iStart+=w;

				}
			iStart = iY*w+iX;
			if (used[iStart]) 
				{
				done = TRUE;
				bHitSide = TRUE;
				bHitTop = TRUE;
				topcount++;
				sidecount++;
				}
//			else 
//				used.Set(iStart);




			}
		}
	iAmount--;



 	if ((bHitSide) && (bHitTop))
		{
		if (sidecount>topcount)
			{
			bHitSide = TRUE;
			bHitTop = FALSE;
			}
		else
			{
			bHitSide = FALSE;
			bHitTop = TRUE;
			}

		}

	if (iAmount == 0) return;

//backup one and decide whether to go right, down  or quite
//both hit we are done
	if ((bHitSide) && (bHitTop))
		{
//return the new x,y, and w, h
		if (dirX == 1)
			iretX = x; 
		else iretX = x + (iAmount*dirX);
		if (dirY == 1)
			iretY = y;
		else iretY = y + (iAmount*dirY);
		iretW = iAmount;
		iretH = iAmount;
		}
	else 
		{
// start in the w direction one
		BOOL bGoingSide = TRUE;
//			if (bHitTop) dirY = 0;
		if (bHitSide) 
			{
//				dirX = 0;
			bGoingSide = FALSE;
			}

		done = FALSE;

		while (!done)
			{
			if (bGoingSide)
				iXAdditionalAmount++;//=dirX;
			else iYAdditionalAmount++;//=dirY;



			int iStart, iEnd;
			int inc = 0;
			int index = 0;
			if (bGoingSide)
				{
				int startY = y;
				int endY = y + (iAmount*dirY);
				int xPos = x+(iAmount+iXAdditionalAmount) * dirX;
				iStart = (startY * w)   + xPos;
				iEnd =   (endY*w)   +  xPos;
					
				inc = w * dirY;

				index = x + (iAmount*dirX)  + (iXAdditionalAmount *dirX);
				}
			else
				{
				int yPos = y + (iAmount+iYAdditionalAmount)*dirY; 
				iStart = (yPos*w) + x ;
				iEnd = (yPos*w) + x+(iAmount+iXAdditionalAmount)*dirX;


				inc = dirX; 
				index = y + (iAmount*dirY)  + (iYAdditionalAmount *dirY);
				}

//check if goes out of border
			if (index < 0)
				done = TRUE;
			if (index < 0)
				done = TRUE;
			if ((bGoingSide) && (index >=w))
				done = TRUE;
			if ((!bGoingSide) && (index >=h))
				done = TRUE;

			if (!done)
				{
				while (iStart != iEnd)
					{
					if (used[iStart]) 
						{
						done = TRUE;
						iStart = iEnd;
						}
					else
						{

						iStart += inc;
						}
					

					}
				}


			}

		if (bGoingSide)
			iXAdditionalAmount--;
		else iYAdditionalAmount--;

//return the new x,y, and w, h
		 
		if (dirX == 1)
			iretX = x; 
		else iretX = x + (iAmount*dirX) + ((iXAdditionalAmount)*dirX);
		if (dirY == 1)
			iretY = y; 
		else iretY = y + (iAmount*dirY) + ((iYAdditionalAmount)*dirY);
		iretW = iAmount+(iXAdditionalAmount);
		iretH = iAmount+(iYAdditionalAmount);

		}



	for (int i = iretY; i < (iretY+iretH+1); i++)
		{
		int iStart =  i*w + iretX;
		for (int j = iretX; j < (iretX+iretW+1); j++)
			{
			used.Set(iStart);
			iStart++;
			}
		}


	}


BOOL	UnwrapMod::BuildClusterFromTVVertexElement()
	{
	FreeClusterList();

	ModContextList mcList;		
	INodeTab nodes;

	if (!ip) return FALSE;
	ip->GetModContexts(mcList,nodes);

	int objects = mcList.Count();
	TSTR statusMessage;
	if (objects != 0)
		{
		MeshTopoData *md = (MeshTopoData*)mcList[0]->localData;

		if (md == NULL) 
			{
			return FALSE;
			}


		Tab<Point3> objNormList;
		BuildNormals(md,objNormList);


//build normals
		BOOL done = FALSE;
			
//		while (!done)
			{
			BitArray processedVerts;
			processedVerts.SetSize(TVMaps.v.Count());
			processedVerts.ClearAll();

			for (int i =0; i < TVMaps.v.Count(); i++)
				{
				if ((!(TVMaps.v[i].flags & FLAG_DEAD)) && (!processedVerts[i]))
					{
					vsel.ClearAll();
					vsel.Set(i);
					SelectElement();
					processedVerts |= vsel;
					Point3 normal(0.0f,0.0f,0.0f);
					int fct =0;
					
					if (vsel.NumberSet() > 0)
						{
//create a cluster and add it
						ClusterClass *cluster = new ClusterClass();

						BOOL add= FALSE;
						for (int j = 0; j < TVMaps.f.Count(); j++)
							{
							int ct = TVMaps.f[j]->count;
							BOOL hit = FALSE;
							for (int k =0; k < ct; k++)
								{
								int index = TVMaps.f[j]->t[k];
								if (vsel[index]) hit = TRUE;
								}
//add to cluster
							if (hit)
								{
								add = TRUE;
								cluster->faces.Append(1,&j);
								
								normal += objNormList[j];
								fct++;
								}
							}	
						
//add edges that were processed
						cluster->normal = normal/(float)fct;
						if (add)
							clusterList.Append(1,&cluster);
						else delete cluster;
						}
					int per = (i * 100)/TVMaps.v.Count();
					statusMessage.printf("%s %d%%.",GetString(IDS_PW_STATUS_BUILDCLUSTER),per);
					if (Bail(ip,statusMessage))
						{
						return FALSE;
						}

					}									
				}
			}
		}
	return TRUE;

	}

void UnwrapMod::BuildVertexClusterList()
	{
//loop through faces
	int vCount = 0;

	if (vertexClusterList.Count() != TVMaps.v.Count())
		vertexClusterList.SetCount(TVMaps.v.Count());

	for (int i = 0; i < TVMaps.v.Count(); i++)
		vertexClusterList[i] = -1;
	
	if (TVMaps.v.Count()==0) return;
	

	for (i =0; i < TVMaps.f.Count(); i++)
		{
		int ct = TVMaps.f[i]->count;
		for (int j =0; j < ct; j++)
			{
			int vIndex = TVMaps.f[i]->v[j];
			int tIndex = TVMaps.f[i]->t[j];
			vertexClusterList[tIndex] = vIndex;
			if (vIndex > vCount) vCount = vIndex;
//do patch handles also
			if ( (TVMaps.f[i]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[i]->vecs))
					{
					int tvertIndex = TVMaps.f[i]->vecs->interiors[j];
					int vvertIndex = TVMaps.f[i]->vecs->vinteriors[j];
					if (tvertIndex >=0) 
						{
						vertexClusterList[tvertIndex] = vvertIndex;
						if (vvertIndex > vCount) vCount = vvertIndex;

						}
						
					tvertIndex = TVMaps.f[i]->vecs->handles[j*2];
					vvertIndex = TVMaps.f[i]->vecs->vhandles[j*2];
					if (tvertIndex >=0)
						{
						vertexClusterList[tvertIndex] = vvertIndex;
						if (vvertIndex > vCount) vCount = vvertIndex;

						}
						
					tvertIndex = TVMaps.f[i]->vecs->handles[j*2+1];
					vvertIndex = TVMaps.f[i]->vecs->vhandles[j*2+1];

					if (tvertIndex >= 0)
						{
						vertexClusterList[tvertIndex] = vvertIndex;
						if (vvertIndex > vCount) vCount = vvertIndex;
						}
	
					}			
			}
		}
	vCount++;
	Tab<int> tVertexClusterListCounts;
	if (tVertexClusterListCounts.Count() != vCount)
		tVertexClusterListCounts.SetCount(vCount);
	for (i = 0; i < vCount; i++)
		tVertexClusterListCounts[i] = 0;

	for (i = 0; i < vertexClusterList.Count(); i++)
		{
		int vIndex = vertexClusterList[i];
		if ((vIndex < 0) || (vIndex >= tVertexClusterListCounts.Count()))
			{
//			DebugPrint("Error\n");
			}
		else tVertexClusterListCounts[vIndex] += 1;
		}

	if (vertexClusterListCounts.Count() != TVMaps.v.Count())
		vertexClusterListCounts.SetCount(TVMaps.v.Count());

	for (i = 0; i < TVMaps.v.Count(); i++)
		{
		int vIndex  = vertexClusterList[i];
		int ct = tVertexClusterListCounts[vIndex];
		vertexClusterListCounts[i] = ct;
		}
	
	}




void UnwrapMod::FreeClusterList()
	{
	for (int i = 0; i < clusterList.Count(); i++)
		{
		delete clusterList[i];
		}
	clusterList.ZeroCount();
	}

BOOL UnwrapMod::BuildCluster( Tab<Point3> normalList, float threshold,
							 BOOL connected,
							 BOOL cleanUpStrayFaces)
	{
	FreeClusterList();
	BitArray processedFaces;

	Tab<BorderClass> clusterBorder;

	BitArray sel;

	sel.SetSize(TVMaps.f.Count());
	


	processedFaces.SetSize(TVMaps.f.Count());
	processedFaces.ClearAll();

	//check for type
	MeshTopoData *md =NULL;

	if (ip) 
		{
		ModContextList mcList;		
		INodeTab nodes;
		ip->GetModContexts(mcList,nodes);

		int objects = mcList.Count();
		md = (MeshTopoData*)mcList[0]->localData;
		}
	else md = GetModData();

	if (!md) return FALSE;

	float radThreshold = threshold * PI/180.0f;
	TSTR statusMessage;

	
	

	if (md)
		{

		
//		MeshTopoData *md = (MeshTopoData*)mcList[0]->localData;

		if (md == NULL) 
			{
			return FALSE;
			}


		Tab<Point3> objNormList;
		BuildNormals(md,objNormList);

		if (md->faceSel.NumberSet() != 0)
			{
			for (int i = 0; i < md->faceSel.GetSize(); i++)
				{
				if (!md->faceSel[i])
					processedFaces.Set(i);
				}
			}

//build normals

		AdjEdgeList *edges = NULL;
		BOOL deleteEdges = FALSE;
		if ((objType == IS_MESH) && (md->mesh) && (connected))
			{
			edges = new AdjEdgeList(*md->mesh);
			deleteEdges = TRUE;
			}
		if (connected)
			{
			BOOL done = FALSE;
			int currentNormal = 0;

			if (normalList.Count() == 0)
				done = TRUE;
			
			while (!done)
				{
				sel.ClearAll();
				//find the closest normal within the threshold
				float angDist = -1.0f;
				int hitIndex = -1;
				for (int i =0; i < objNormList.Count(); i++)
					{
					if (!processedFaces[i])
						{
						float cangle = acos(DotProd(normalList[currentNormal],objNormList[i]));
						if (((cangle < angDist) || (hitIndex == -1)) && (cangle < radThreshold))
							{
							angDist = cangle;
							hitIndex = i;
							}
						}
					}
				int bail = 0;
				if ( (hitIndex != -1) )
					{
					SelectFacesByGroup( md,sel,hitIndex, normalList[currentNormal], threshold, FALSE,objNormList,
						clusterBorder,
						edges);
//add cluster
					if (sel.NumberSet() > 0)
						{
//create a cluster and add it
						ClusterClass *cluster = new ClusterClass();
					 	BOOL hit = FALSE;
						cluster->normal = normalList[currentNormal];
						for (int j = 0; j < sel.GetSize(); j++)
							{
							if (sel[j] && (!processedFaces[j]))
								{
//add to cluster
								processedFaces.Set(j,TRUE);
								cluster->faces.Append(1,&j);
								hit = TRUE;
								}
							}

						cluster->borderData = clusterBorder;
/*
						int ct = 0;
						for (j = 0; j < clusterBorder.Count(); j++)
							{
							if (clusterInnerFaces[j])
								{
								cluster->connectedFaces[ct] = j;
								ct++;
								}
							}
*/
//add edges that were processed
						if (hit)
							{
							clusterList.Append(1,&cluster);
							bail++;
							}
						else
							{
							delete cluster;
							}
						
						}
					}	
				currentNormal++;
				if (currentNormal >= normalList.Count()) 
					{
					currentNormal = 0;
					if (bail == 0) done = TRUE;
					}


				int per = (processedFaces.NumberSet() * 100)/processedFaces.GetSize();
				statusMessage.printf("%s %d%%.",GetString(IDS_PW_STATUS_BUILDCLUSTER),per);
				if (Bail(ip,statusMessage))
					{
					if (deleteEdges) delete edges;
					return FALSE;
					}


				}
			}
		else
			{	
			for (int i =0; i < normalList.Count(); i++)
				{
				sel.ClearAll();
	
				//find closest face norm
				SelectFacesByNormals(md,sel,normalList[i], threshold, objNormList);
//add cluster
				int numberSet = sel.NumberSet();
				int totalNumberSet = processedFaces.NumberSet();
				if ( numberSet > 0)
					{
//create a cluster and add it
					ClusterClass *cluster = new ClusterClass();
					BOOL hit = FALSE;
					cluster->normal = normalList[i];
					for (int j = 0; j < sel.GetSize(); j++)
						{
						if (sel[j] && (!processedFaces[j]))
							{
//add to cluster
							processedFaces.Set(j,TRUE);
							cluster->faces.Append(1,&j);
							hit = TRUE;
							}
						}
					if (hit)
						clusterList.Append(1,&cluster);
					else delete cluster;
					}

				int per = (i * 100)/normalList.Count();
				statusMessage.printf("%s %d%%.",GetString(IDS_PW_STATUS_BUILDCLUSTER),per);
				if (Bail(ip,statusMessage))
					{
					if (deleteEdges) delete edges;
					return FALSE;
					}

				}
			}
//process the ramaining
		
		if (cleanUpStrayFaces)
			{
//			Tab<int> tempSeedFaces = seedFaces;
			int ct = 0;
			if (seedFaces.Count() > 0) ct = seedFaces[0];
			while ( (processedFaces.NumberSet() != processedFaces.GetSize()) /*&& (ct!= processedFaces.GetSize()) */)
				{
				if (!processedFaces[ct])
					{
					if (connected)
						{
						SelectFacesByGroup( md,sel,ct, objNormList[ct], threshold, FALSE,objNormList,
							clusterBorder,
							edges);
						
						}
					else SelectFacesByNormals(md,sel,objNormList[ct], threshold, objNormList);
//add cluster
					if (sel.NumberSet() > 0)
						{
//create a cluster and add it
						ClusterClass *cluster = new ClusterClass();
						cluster->normal = objNormList[ct];
						BOOL hit = FALSE;
						for (int j = 0; j < sel.GetSize(); j++)
							{
							if (sel[j] && (!processedFaces[j]))
								{
//add to cluster
								processedFaces.Set(j,TRUE);
								cluster->faces.Append(1,&j);
								hit = TRUE;
								}
							}
						if (connected)
							{
							cluster->borderData = clusterBorder;
							}
						if (hit)
							{
							clusterList.Append(1,&cluster);
							}
						else
							{
							delete cluster;
							}
						
						}

					}
				ct++;
				if (ct >= processedFaces.GetSize())
					ct =0;

				int per = (processedFaces.NumberSet() * 100)/processedFaces.GetSize();
				statusMessage.printf("%s %d%%.",GetString(IDS_PW_STATUS_BUILDCLUSTER),per);
				if (Bail(ip,statusMessage))
					{
					if (deleteEdges) delete edges;
					return FALSE;
					}

				}
			}
		if (deleteEdges) delete edges;
		}
	return TRUE;
	}



void	UnwrapMod::NormalizeCluster(float spacing)
	{
	BitArray processedVerts;
	processedVerts.SetSize(TVMaps.v.Count());
	processedVerts.ClearAll();
	for (int i =0; i < clusterList.Count(); i++)
		{
		for (int j = 0; j < clusterList[i]->faces.Count();j++)
			{
			int faceIndex = clusterList[i]->faces[j];
			for (int k = 0; k < TVMaps.f[faceIndex]->count; k++)
				{
				int vertIndex = TVMaps.f[faceIndex]->t[k];
				processedVerts.Set(vertIndex);
				if ( (objType == IS_PATCH) && (TVMaps.f[faceIndex]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[faceIndex]->vecs))
					{
					vertIndex = TVMaps.f[faceIndex]->vecs->interiors[k];
					if ((vertIndex >=0) && (vertIndex < processedVerts.GetSize()))
						processedVerts.Set(vertIndex);
					vertIndex = TVMaps.f[faceIndex]->vecs->handles[k*2];
					if ((vertIndex >=0) && (vertIndex < processedVerts.GetSize()))
						processedVerts.Set(vertIndex);
					vertIndex = TVMaps.f[faceIndex]->vecs->handles[k*2+1];
					if ((vertIndex >=0) && (vertIndex < processedVerts.GetSize()))
						processedVerts.Set(vertIndex);
	
					}
				}
			}
		}
	
	float minx = FLT_MAX,miny = FLT_MAX;
	float maxx = FLT_MIN,maxy = FLT_MIN;
	for (i = 0; i < TVMaps.v.Count(); i++)
		{
		if (processedVerts[i])
			{
			Point3 p = TVMaps.v[i].p;
			if (p.x<minx) minx = p.x;
			if (p.y<miny) miny = p.y;
			if (p.x>maxx) maxx = p.x;
			if (p.y>maxy) maxy = p.y;
			}
		}
	float w,h;
	w = maxx-minx;
	h = maxy-miny;
	gBArea = w *h;

	for (i = 0; i < TVMaps.v.Count(); i++)
		{
		if (processedVerts[i])
			{
			TVMaps.v[i].p.x -= minx;
			TVMaps.v[i].p.y -= miny;
			}
		}
	
	for (i = 0; i < TVMaps.v.Count(); i++)
		{
		float amount = h;
		if (w > h) 
			amount = w;
		if (processedVerts[i])
			{
			TVMaps.v[i].p.x /= amount;
			TVMaps.v[i].p.y /= amount;
			if (TVMaps.cont[i]) TVMaps.cont[i]->SetValue(0,&TVMaps.v[i].p);
			}	
		}
	if (spacing != 0.0f)
		{
		float mid = spacing;
		float scale = 1.0f - (spacing*2.0f);
		for (i = 0; i < TVMaps.v.Count(); i++)
			{
			if (processedVerts[i])
				{
				TVMaps.v[i].p.x *= scale;
				TVMaps.v[i].p.y *= scale;
				TVMaps.v[i].p.x += mid;
				TVMaps.v[i].p.y += mid;
				if (TVMaps.cont[i]) TVMaps.cont[i]->SetValue(0,&TVMaps.v[i].p);
				}	
			}

		}

	}

class TempEdgeList
	{
public:
	Tab<int> edges;
	};

BOOL UnwrapMod::RotateClusters(float &finalArea)
	{
	BitArray usedVerts;
	
	usedVerts.SetSize(TVMaps.v.Count());
	usedVerts.ClearAll();
	float totalArea = 0.0f;

	TSTR statusMessage;
	for (int  i=0; i < clusterList.Count(); i++)
		{
		BuildUsedList(usedVerts ,i);
//compute
		BOOL done = FALSE;

		//build edges lists for this polygon
		Tab<TempEdgeList*> pEdges;
		pEdges.SetCount(clusterList[i]->faces.Count());
		for (int j =0; j < clusterList[i]->faces.Count(); j++)
			{
			pEdges[j] = new TempEdgeList();
			int faceIndex = clusterList[i]->faces[j];
			pEdges[j]->edges.SetCount(TVMaps.f[faceIndex]->count);
			for (int k = 0; k <  TVMaps.f[faceIndex]->count; k++)
				{
				int vID = TVMaps.f[faceIndex]->t[k];
				pEdges[j]->edges[k] = vID;
				}
			}

		Tab<Point3> transformedPoints, bestPoints;
		transformedPoints.SetCount(TVMaps.v.Count());
		bestPoints.SetCount(TVMaps.v.Count());

		for (j =0; j < TVMaps.v.Count(); j++)
			{
			transformedPoints[j] = TVMaps.v[j].p;
			}

		int angle = 0;

		float bestArea = -1.0f;
		int bestAngle = 0;
		float sArea =0.0f;
		float surfaceArea =0.0f;

		for (j =0; j < clusterList[i]->faces.Count(); j++)
			{
			int faceIndex = clusterList[i]->faces[j];
			int faceCount = TVMaps.f[faceIndex]->count;

			int vertID = 0;

			Tab<Point3> areaPoints;
			for (int k = 0; k <  faceCount; k++)
				{
				int vID = TVMaps.f[faceIndex]->t[k];
				Point3 p = transformedPoints[vID];
				areaPoints.Append(1,&p);
				}
//compute the area of the faces
			if (TVMaps.f[faceIndex]->count == 3)
				{
				surfaceArea += AreaOfTriangle(areaPoints[0], areaPoints[1],areaPoints[2]);
				}
			else
				{
				surfaceArea += AreaOfPolygon(areaPoints);
				}
			}

		clusterList[i]->surfaceArea = surfaceArea;


		while (!done)
			{

			float boundsArea =0.0f;
			Box3 bounds;
			bounds.Init();
//transforms the points
			if (angle != 0)
				{
				float fangle = ((float) angle) * 180.0f/PI;
				for (j =0; j < TVMaps.v.Count(); j++)
					{
					if (usedVerts[j])
						{
						float x = TVMaps.v[j].p.x;
						float y = TVMaps.v[j].p.y;
						float tx = (x * cos(fangle)) - (y * sin(fangle));
						float ty = (x * sin(fangle)) + (y * cos(fangle));
						transformedPoints[j].x =  tx;
						transformedPoints[j].y =  ty;
						}
					}
				}

//build the list 
			for (j =0; j < clusterList[i]->faces.Count(); j++)
				{
				int faceIndex = clusterList[i]->faces[j];
				int faceCount = TVMaps.f[faceIndex]->count;
				for (int k = 0; k <  faceCount; k++)
					{
					int vID = TVMaps.f[faceIndex]->t[k];
					Point3 p = transformedPoints[vID];
					bounds += p;
					}
				}

			float w, h;

			h = bounds.pmax.y-bounds.pmin.y;
			w = bounds.pmax.x-bounds.pmin.x;

			
			boundsArea = (w*h);

//compute the area of the bounding box
			if ((surfaceArea/boundsArea) > 0.95f )
				{
				done = TRUE;
				sArea = surfaceArea;
				}
			else
				{
				if ((boundsArea < bestArea) || (bestArea < 0.0f))
					{
					bestArea = boundsArea;
					bestPoints = transformedPoints;
					bestAngle = angle;
					}
				}

			angle+=2;

			if (angle >= 45) 
				{
				done = TRUE;
				sArea = surfaceArea;
				}

			}
		if (bestAngle != 0)
			{
			for (int j = 0; j < usedVerts.GetSize(); j++)
				{
				if (usedVerts[j])
					{	
					Point3 tempPoint;
					TVMaps.v[j].p = bestPoints[j];
					
					if (TVMaps.cont[j]) TVMaps.cont[j]->SetValue(0,&TVMaps.v[j].p);
					}
				}
			}

		for (j =0; j < pEdges.Count(); j++)
			delete pEdges[j];
		totalArea += sArea;

		int per = (i * 100)/clusterList.Count();
		statusMessage.printf("%s %d%%.",GetString(IDS_PW_STATUS_ROTATING),per);
		if (Bail(ip,statusMessage))
			{
			i = clusterList.Count();
			finalArea = totalArea;
			return FALSE;
			}

		}
	finalArea = totalArea;
	return TRUE;
	}

void	UnwrapMod::JoinCluster(ClusterClass *baseCluster, ClusterClass *joinCluster)
	{
//append faces
	int ct= joinCluster->faces.Count();
	for (int i = 0; i < ct; i++)
		{
		baseCluster->faces.Append(1,&joinCluster->faces[i]);
		}
	
	ct = joinCluster->boundsList.Count();
	for (i = 0; i < ct; i++)
		{
		baseCluster->boundsList.Append(1,&joinCluster->boundsList[i]);
		}

	baseCluster->normal = (baseCluster->normal + joinCluster->normal) *0.5f;
//	baseCluster->surfaceArea += joinCluster->surfaceArea;





	}


void	UnwrapMod::JoinCluster(ClusterClass *baseCluster, ClusterClass *joinCluster, float x, float y)
	{

	BitArray usedVerts;
	usedVerts.SetSize(TVMaps.v.Count());
	usedVerts.ClearAll();
	BuildUsedList(usedVerts ,joinCluster);

	int size = usedVerts.GetSize();
	Point3 offset = joinCluster->bounds.pmin;
	offset.x = x - offset.x;
	offset.y = y - offset.y;
	offset.z = 0.0f;
	for (int j =0; j < size; j++)
		{
		if (usedVerts[j])
			{
//need to put patch handles in here also
			TVMaps.v[j].p = TVMaps.v[j].p+offset;
			if (TVMaps.cont[j]) TVMaps.cont[j]->SetValue(0,&TVMaps.v[j].p);
			}
		}


//append faces
	int ct= joinCluster->faces.Count();
	for (int i = 0; i < ct; i++)
		{
		baseCluster->faces.Append(1,&joinCluster->faces[i]);
		}
	


	baseCluster->normal = (baseCluster->normal + joinCluster->normal) *0.5f;

	baseCluster->BuildList(TVMaps);




	}


static int CompSubClusterArea( const void *elem1, const void *elem2 ) {
	SubClusterClass *ta = (SubClusterClass *)elem1;
	SubClusterClass *tb = (SubClusterClass *)elem2;


	float aH, bH;
	aH = ta->w * ta->h;
	bH = tb->w * tb->h;

	if (aH == bH) return 0;
	else if (aH > bH) return -1;
	else return 1;
	
}


BOOL UnwrapMod::CollapseClusters(float spacing)
	{
	TSTR statusMessage;
	int clusterCount = clusterList.Count();
	for (int  i=0; i < clusterCount; i++)
		{
		clusterList[i]->BuildList(TVMaps);
		}

	for (i=0; i < clusterCount; i++)
		{
		clusterList[i]->openSpaces.ZeroCount();
		}

	for (i=0; i < clusterCount; i++)
		{
		currentCluster = i;
		clusterList[i]->ComputeOpenSpaces(spacing);
		subCluster = clusterList[i]->openSpaces.Count()-1;


		int per = (i * 100)/clusterList.Count();
		statusMessage.printf("%s %d%%.",GetString(IDS_PW_STATUS_DEADSPACE),per);
		if (Bail(ip,statusMessage))
			{
			i = clusterList.Count();
			return FALSE;
			}

#ifdef DEBUGMODE
		if (gDebugLevel >=2)
			{
			drawOnlyBounds = TRUE;
			NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
			ip->RedrawViews(ip->GetTime());
			InvalidateView();
			UpdateWindow(hView);
			drawOnlyBounds = FALSE;
			}
#endif
		}


//	Tab<SubClusterClass> subClusterList;




//start at the back and work our way to the front
	for (i=clusterList.Count()-2; i >= 0 ; i--)
		{

		BOOL bSubClusterDone = FALSE;
		int iSubClusterCount = 0;

#ifdef DEBUGMODE
		jointClusterBoxX.SetCount(clusterList[i]->openSpaces.Count());
		jointClusterBoxY.SetCount(clusterList[i]->openSpaces.Count());
		jointClusterBoxW.SetCount(clusterList[i]->openSpaces.Count());
		jointClusterBoxH.SetCount(clusterList[i]->openSpaces.Count());
		for (int jk = 0; jk < clusterList[i]->openSpaces.Count(); jk++)
			{
			jointClusterBoxX[jk] = clusterList[i]->openSpaces[jk].x;
			jointClusterBoxY[jk] = clusterList[i]->openSpaces[jk].y;
			jointClusterBoxW[jk] = clusterList[i]->openSpaces[jk].w;
			jointClusterBoxH[jk] = clusterList[i]->openSpaces[jk].h;
			}
#endif

		while ((!bSubClusterDone) &&  (iSubClusterCount < 4))
			{
			
			clusterList[i]->openSpaces.Sort(CompSubClusterArea);
			tempSubCluster.ZeroCount();



			for (int j = 0; j < clusterList[i]->openSpaces.Count(); j++)
				{
				SubClusterClass subCluster;
				subCluster = clusterList[i]->openSpaces[j];
				for (int k = i+1; k < clusterList.Count(); k++)
					{
//see if any of the smaller cluster fits in our sub cluster
					float clusterW,clusterH;
					clusterW = clusterList[k]->w;
					clusterH = clusterList[k]->h;
					BOOL flip = FALSE;
					BOOL join = FALSE;
					if ( (clusterW< subCluster.w) && (clusterH< subCluster.h))
						join = TRUE;

					if ( (clusterH< subCluster.w) && (clusterW< subCluster.h))
						{
						join = TRUE;
						flip = TRUE;
						float tw = clusterW;
						clusterW = clusterH;
						clusterH = tw;

						}

					if ( join)
						{
						float subX = subCluster.x;
						float subY = subCluster.y;
						float subW = subCluster.w;
						float subH = subCluster.h;

#ifdef DEBUGMODE
						hitClusterBoxX = subX;
						hitClusterBoxY = subY;
						hitClusterBoxW = subW;
						hitClusterBoxH = subH;
#endif

						if (flip)
							FlipSingleCluster(k,spacing);

						JoinCluster(clusterList[i], clusterList[k], subX, subY);	

//split the rect and put the remaining 2 pieces back in the list
						SubClusterClass newSpots[4];
	
						newSpots[0] = SubClusterClass(subX,subY+clusterH,clusterW,subH - clusterH);
						newSpots[1] = SubClusterClass(subX+clusterW,subY,subW-clusterW,subH);

						newSpots[2] = SubClusterClass(subX,subY+clusterH,subW,subH - clusterH);
						newSpots[3] = SubClusterClass(subX+clusterW,subY,subW-clusterW,clusterH);

						float cArea = newSpots[0].w*newSpots[0].h;
						int cIndex = 0;
						if ((newSpots[1].w*newSpots[1].h) > cArea)
							{
							cIndex = 1;
							cArea = newSpots[1].w*newSpots[1].h;
							}
						if ((newSpots[2].w*newSpots[2].h) > cArea)
							{
							cIndex = 2;
							cArea = newSpots[2].w*newSpots[2].h;
							}
						if ((newSpots[3].w*newSpots[3].h) > cArea)
							{
							cIndex = 3;
							cArea = newSpots[3].w*newSpots[3].h;
							}
	
						if (cIndex < 2)
							{
							if ((newSpots[0].h > 0.0f) && (newSpots[0].w > 0.0f))
								tempSubCluster.Append(1,&newSpots[0]);
							if ((newSpots[1].h > 0.0f) && (newSpots[1].w > 0.0f))
								tempSubCluster.Append(1,&newSpots[1]);
							}
						else
							{
							if ((newSpots[2].h > 0.0f) && (newSpots[2].w > 0.0f))
								tempSubCluster.Append(1,&newSpots[2]);
							if ((newSpots[3].h > 0.0f) && (newSpots[3].w > 0.0f))
								tempSubCluster.Append(1,&newSpots[3]);
							}
#ifdef DEBUGMODE
						if (gDebugLevel >= 2)
							{
							currentCluster = i;
							drawOnlyBounds = TRUE;
							NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
							ip->RedrawViews(ip->GetTime());
							InvalidateView();
							UpdateWindow(hView);
							drawOnlyBounds = FALSE;
							}
#endif

						delete clusterList[k];
						clusterList.Delete(k,1);

						k = clusterList.Count();
						}
					}
				}
			clusterList[i]->openSpaces.ZeroCount();
			clusterList[i]->openSpaces = tempSubCluster;
			iSubClusterCount++;
			if (clusterList[i]->openSpaces.Count() == 0) bSubClusterDone = TRUE;

			}

		int per = ((clusterList.Count()-i) * 100)/clusterList.Count();
		statusMessage.printf("%s %d%%.",GetString(IDS_PW_STATUS_COLLAPSING),per);
		if (Bail(ip,statusMessage))
			{
			i = clusterList.Count();
			return FALSE;
			}

		}
	return TRUE;
	}



void UnwrapMod::FlipSingleCluster(int i,float spacing)
	{
	BitArray usedVerts;
	usedVerts.SetSize(TVMaps.v.Count());
	usedVerts.ClearAll();

	BuildUsedList(usedVerts ,i);


	clusterList[i]->bounds.Init();


	for (int j = 0; j < usedVerts.GetSize(); j++)
		{
		if (usedVerts[j])
			{	
			Point3 tempPoint;
			tempPoint = TVMaps.v[j].p;
			TVMaps.v[j].p.x = -tempPoint.y;
			TVMaps.v[j].p.y = tempPoint.x;
			if (TVMaps.cont[j]) TVMaps.cont[j]->SetValue(0,&TVMaps.v[j].p);
			clusterList[i]->bounds += TVMaps.v[j].p;
			}
		}

//build offset
	float x,y;
	x = clusterList[i]->bounds.pmin.x;
	y = clusterList[i]->bounds.pmin.y;
	clusterList[i]->offset.x = x;
	clusterList[i]->offset.y = y;
	clusterList[i]->offset.z = 0.0f;
	
	float tw = 	clusterList[i]->w;
	clusterList[i]->w = clusterList[i]->h;
	clusterList[i]->h = tw;

	clusterList[i]->h = clusterList[i]->h+spacing;
	clusterList[i]->w = clusterList[i]->w+spacing;

	}

void	UnwrapMod::FlipClusters(BOOL flipW,float spacing)
	{

	for (int i=0; i < clusterList.Count(); i++)
		{
		BOOL flip = FALSE;
		if (flipW)
			{
			if (clusterList[i]->h >	clusterList[i]->w)
				flip = TRUE;
			}
		else
			{
			if (clusterList[i]->w >	clusterList[i]->h)
				flip = TRUE;
			}
		if (flip)
			{
			FlipSingleCluster(i,spacing);
			}
		}
	}

float	UnwrapMod::PlaceClusters2(float area)
	{
	BOOL done = FALSE;
	Tab<OpenAreaList> openAreaList;
	float edgeLength = 0.0f;
	edgeLength = sqrt(area);
	float edgeInc = edgeLength*0.01f;
	while (!done)
		{
		openAreaList.ZeroCount();
		//append to stack
		OpenAreaList tempSpot(0,0,edgeLength,edgeLength);
		openAreaList.Append(1,&tempSpot);
		//loop through polys
		BOOL suceeded = TRUE;
		
		for (int i = 0; i < clusterList.Count(); i++)
			{
		//look for hole
			int index = -1;
			float currentArea = 0.0f;
			currentArea = clusterList[i]->w * clusterList[i]->h;
			float bArea = -1.0f;
			for (int j =0; j < openAreaList.Count(); j++)
				{
				if ((clusterList[i]->w<=openAreaList[j].w) && (clusterList[i]->h<=openAreaList[j].h))
					{
					if (((currentArea < bArea) && (currentArea >= 0.0f)) || (bArea < 0.0f))
						{
						index = j;
						bArea = currentArea;
						}
					}
				}
		//if hole add to split stack and delete holw
			if (index < 0)  // failed to find a holw
				{
				suceeded = FALSE; //bail and resize grid
				}
			else
				{
				//now subdivide the area and find the one that is the best fit
				OpenAreaList newSpots[4];
				float x,y,w,h;
				x = openAreaList[index].x;
				y = openAreaList[index].y;
				w = openAreaList[index].w;		
				h = openAreaList[index].h;
				float clusterW, clusterH;
				
				clusterW = clusterList[i]->w;
				clusterH = clusterList[i]->h;
			
				if ((currentArea) > 0.0f)
					{
					newSpots[0] = OpenAreaList(x,y+clusterH,clusterW,h - clusterH);
					newSpots[1] = OpenAreaList(x+clusterW,y,w-clusterW,h);

					newSpots[2] = OpenAreaList(x,y+clusterH,w,h - clusterH);
					newSpots[3] = OpenAreaList(x+clusterW,y,w-clusterW,clusterH);

					float cArea = newSpots[0].area;
					int cIndex = 0;
					if (newSpots[1].area > cArea)
						{
						cIndex = 1;
						cArea = newSpots[1].area;
						}
					if (newSpots[2].area > cArea)
						{
						cIndex = 2;
						cArea = newSpots[2].area;
						}
					if (newSpots[3].area > cArea)
						{
						cIndex = 3;
						cArea = newSpots[3].area;
						}

					if (cIndex < 2)
						{
						openAreaList.Append(1,&newSpots[0]);
						openAreaList.Append(1,&newSpots[1]);
						}
					else
						{
						openAreaList.Append(1,&newSpots[2]);
						openAreaList.Append(1,&newSpots[3]);
						}

				//set the cluster newx, newy
					clusterList[i]->newX = openAreaList[index].x;
					clusterList[i]->newY = openAreaList[index].y;
				//delete that open spot since it is now occuppied
					openAreaList.Delete(index,1);
					}
				}
		
			}	
		if (suceeded)
			{
//move all verts
			done = TRUE;
			}
		else
			{
			edgeLength += edgeInc;

			}

		}

	return edgeLength;

	}


static int CompTable( const void *elem1, const void *elem2 ) {
	ClusterClass **ta = (ClusterClass **)elem1;
	ClusterClass **tb = (ClusterClass **)elem2;

	ClusterClass *a = *ta;
	ClusterClass *b = *tb;

	float aH, bH;
	aH = a->bounds.pmax.y - a->bounds.pmin.y;
	bH = b->bounds.pmax.y - b->bounds.pmin.y;

	if (aH == bH) return 0;
	else if (aH > bH) return -1;
	else return 1;
	
}


BOOL UnwrapMod::LayoutClusters2(float spacing, BOOL rotateClusters, BOOL combineClusters)
	{

	float totalWidth = 0;
//build bounding data

	float area = 0.0f;

	for (int i=0; i < clusterList.Count(); i++)
		{
		if (clusterList[i]->faces.Count() == 0) //check for clusters with no faces
			{
			delete clusterList[i];
			clusterList.Delete(i,1);
			i--;
			}
		else //check for clusters that have no points in them since we can get faces with no points
			{

			int totalPointCount = 0;
			for (int j =0; j < clusterList[i]->faces.Count(); j++)
				{
				int faceIndex = clusterList[i]->faces[j];
				totalPointCount += TVMaps.f[faceIndex]->count;
				}
			if (totalPointCount==0)
				{
				delete clusterList[i];
				clusterList.Delete(i,1);
				i--;
				}
			}

		}
	gSArea = 0.0f;
	gBArea = 0.0f;
	if (rotateClusters)
		{
		if (!RotateClusters(gSArea)) return FALSE;
		}


	for ( i=0; i < clusterList.Count(); i++)
		{

		clusterList[i]->h = 0.0f;
		clusterList[i]->w = 0.0f;


		clusterList[i]->bounds.Init();
		for (int j =0; j < clusterList[i]->faces.Count(); j++)
			{
			int faceIndex = clusterList[i]->faces[j];
			for (int k = 0; k <  TVMaps.f[faceIndex]->count; k++)
				{
//need to put patch handles in here also
				int index = TVMaps.f[faceIndex]->t[k];
				Point3 a = TVMaps.v[index].p;
				clusterList[i]->bounds += a;

				if ((TVMaps.f[faceIndex]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[faceIndex]->vecs))
					{
					index = TVMaps.f[faceIndex]->vecs->handles[k*2];
					a = TVMaps.v[index].p;
					clusterList[i]->bounds += a;

					index = TVMaps.f[faceIndex]->vecs->handles[k*2+1];
					a = TVMaps.v[index].p;
					clusterList[i]->bounds += a;

					if (TVMaps.f[faceIndex]->flags & FLAG_INTERIOR)
						{
						index = TVMaps.f[faceIndex]->vecs->interiors[k];
						a = TVMaps.v[index].p;
						clusterList[i]->bounds += a;
						}
					}
				}
//FIX
			Point3 width = clusterList[i]->bounds.Width();
			Point3 pmin,pmax;
			pmin = clusterList[i]->bounds.pmin;
			pmax = clusterList[i]->bounds.pmax;
			if ( width.x == 0.0f )
				{
				Point3 p = pmin;
				p.x -= 0.0001f;
				clusterList[i]->bounds += p;
				p = pmax;
				p.x += 0.0001f;
				clusterList[i]->bounds += p;

				}
			if ( width.y == 0.0f)
				{
				Point3 p = pmin;
				p.y -= 0.0001f;
				clusterList[i]->bounds += p;
				p = pmax;
				p.y += 0.0001f;
				clusterList[i]->bounds += p;

				}


			}
//build offset
		float x,y;
		x = clusterList[i]->bounds.pmin.x;
		y = clusterList[i]->bounds.pmin.y;
		clusterList[i]->offset.x = x;
		clusterList[i]->offset.y = y;
		clusterList[i]->offset.z = 0.0f;
		if (clusterList[i]->faces.Count() == 0)
			{
			clusterList[i]->h = 0.0f;
			clusterList[i]->w = 0.0f;

			}
		else
			{
			clusterList[i]->h = clusterList[i]->bounds.pmax.y-clusterList[i]->bounds.pmin.y;
			clusterList[i]->w = clusterList[i]->bounds.pmax.x-clusterList[i]->bounds.pmin.x;
			if (clusterList[i]->h == 0.0f) clusterList[i]->h = 0.5f;
			if (clusterList[i]->w == 0.0f) clusterList[i]->w = 0.5f;
			}

//		clusterList[i]->h += clusterList[i]->h*spacing;
//		clusterList[i]->w += clusterList[i]->w*spacing;

		totalWidth +=clusterList[i]->w;
		area += clusterList[i]->h * clusterList[i]->w;
		}


	spacing = sqrt(area)*spacing;

	totalWidth =0.0f;
	area = 0.0f;


	for ( i=0; i < clusterList.Count(); i++)
		{
		clusterList[i]->h = clusterList[i]->h+spacing;
		clusterList[i]->w = clusterList[i]->w+spacing;

		totalWidth +=clusterList[i]->w;
		area += clusterList[i]->h * clusterList[i]->w;

		}



//sort bounding data by area 
	clusterList.Sort(CompTableArea);

	if (combineClusters)
		{
		if (!CollapseClusters(spacing)) return FALSE;
		}
	
//rotate all of them so the the width is the shortest

	FlipClusters(TRUE,spacing);
	float fEdgeLenW = PlaceClusters2(area);

	FlipClusters(FALSE,spacing);
	float fEdgeLenH = PlaceClusters2(area);

	if (fEdgeLenW < fEdgeLenH)
		{
		FlipClusters(TRUE,spacing);
		PlaceClusters2(area);

		}


//now move vertices
	BitArray hitVerts;
	hitVerts.SetSize( TVMaps.v.Count());
	hitVerts.ClearAll();
	for (i=0; i < clusterList.Count(); i++)
		{
		clusterList[i]->offset.x = clusterList[i]->newX-clusterList[i]->offset.x;
		clusterList[i]->offset.y = clusterList[i]->newY-clusterList[i]->offset.y;
		for (int j = 0; j <  clusterList[i]->faces.Count(); j++)
			{
//need to put patch handles in here also
			int findex = clusterList[i]->faces[j];
			for (int m = 0; m <  TVMaps.f[findex]->count; m++)
				{
//need to put patch handles in here also
				int index = TVMaps.f[findex]->t[m];
				if (!hitVerts[index])
					{
					if ((index >=0) && (index < TVMaps.v.Count()))
						{
						TVMaps.v[index].p = TVMaps.v[index].p + clusterList[i]->offset;
						if (TVMaps.cont[index]) TVMaps.cont[index]->SetValue(0,&TVMaps.v[index].p);
						}
				 	hitVerts.Set(index);
					}	

				if ((TVMaps.f[findex]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[findex]->vecs))
					{
					index = TVMaps.f[findex]->vecs->handles[m*2];
					if (!hitVerts[index])
						{
						if ((index >=0) && (index < TVMaps.v.Count()))
							{
							TVMaps.v[index].p = TVMaps.v[index].p + clusterList[i]->offset;
							if (TVMaps.cont[index]) TVMaps.cont[index]->SetValue(0,&TVMaps.v[index].p);
							}
					 	hitVerts.Set(index);
						}

					index = TVMaps.f[findex]->vecs->handles[m*2+1];
					if (!hitVerts[index])
						{
						if ((index >=0) && (index < TVMaps.v.Count()))
							{
							TVMaps.v[index].p = TVMaps.v[index].p + clusterList[i]->offset;
							if (TVMaps.cont[index]) TVMaps.cont[index]->SetValue(0,&TVMaps.v[index].p);
							}
					 	hitVerts.Set(index);
						}


					if (TVMaps.f[findex]->flags & FLAG_INTERIOR)
						{
						index = TVMaps.f[findex]->vecs->interiors[m];
						if (!hitVerts[index])
							{
							if ((index >=0) && (index < TVMaps.v.Count()))
								{
								TVMaps.v[index].p = TVMaps.v[index].p + clusterList[i]->offset;
								if (TVMaps.cont[index]) TVMaps.cont[index]->SetValue(0,&TVMaps.v[index].p);
								}
					 		hitVerts.Set(index);
							}

						}
					}

					
				}
			}
		}

	return TRUE;

	}

BOOL UnwrapMod::LayoutClusters(float spacing,BOOL rotateClusters, BOOL alignWidth, BOOL combineClusters)
	{

		gSArea = 0.0f;
		gBArea = 0.0f;
		if (rotateClusters)
			{
			if (!RotateClusters(gSArea)) return FALSE;
			}

		float totalWidth = 0;
		float area = 0.0f;
//build bounding data
		for (int i=0; i < clusterList.Count(); i++)
			{
			clusterList[i]->bounds.Init();
			for (int j =0; j < clusterList[i]->faces.Count(); j++)
				{
				int faceIndex = clusterList[i]->faces[j];
				for (int k = 0; k <  TVMaps.f[faceIndex]->count; k++)
					{
//need to put patch handles in here also
					int index = TVMaps.f[faceIndex]->t[k];

					Point3 a = TVMaps.v[index].p;
					clusterList[i]->bounds += a;

					if ( (TVMaps.f[faceIndex]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[faceIndex]->vecs))
						{
						if (TVMaps.f[faceIndex]->flags & FLAG_INTERIOR) 
							{

							index = TVMaps.f[faceIndex]->vecs->interiors[k];
							if ((index >=0) && (index < TVMaps.v.Count()))
								{
								Point3 a = TVMaps.v[index].p;
								clusterList[i]->bounds += a;
								}
							}
										
						index = TVMaps.f[faceIndex]->vecs->handles[k*2];
						if ( (index >=0) && (index < TVMaps.v.Count()))
							{
							Point3 a = TVMaps.v[index].p;
							clusterList[i]->bounds += a;
							}
										
						index = TVMaps.f[faceIndex]->vecs->handles[k*2+1];
						if ((index >=0) && (index < TVMaps.v.Count()))
							{
							Point3 a = TVMaps.v[index].p;
							clusterList[i]->bounds += a;
							}
					
						}

					}

				}

			Point3 width = clusterList[i]->bounds.Width();
			Point3 pmin,pmax;
			pmin = clusterList[i]->bounds.pmin;
			pmax = clusterList[i]->bounds.pmax;
			if ( width.x == 0.0f )
				{
				Point3 p = pmin;
				p.x -= 0.0001f;
				clusterList[i]->bounds += p;
				p = pmax;
				p.x += 0.0001f;
				clusterList[i]->bounds += p;

				}
			if ( width.y == 0.0f)
				{
				Point3 p = pmin;
				p.y -= 0.0001f;
				clusterList[i]->bounds += p;
				p = pmax;
				p.y += 0.0001f;
				clusterList[i]->bounds += p;

				}

//build offset
			float x,y;
			x = clusterList[i]->bounds.pmin.x;
			y = clusterList[i]->bounds.pmin.y;
			clusterList[i]->offset.x = x;
			clusterList[i]->offset.y = y;
			clusterList[i]->offset.z = 0.0f;
			clusterList[i]->h = clusterList[i]->bounds.pmax.y-clusterList[i]->bounds.pmin.y;
			clusterList[i]->w = clusterList[i]->bounds.pmax.x-clusterList[i]->bounds.pmin.x;

			if (clusterList[i]->h == 0.0f) clusterList[i]->h = 0.5f;
			if (clusterList[i]->w == 0.0f) clusterList[i]->w = 0.5f;


//			clusterList[i]->h += clusterList[i]->h*spacing;
//			clusterList[i]->w += clusterList[i]->w*spacing;
			area += clusterList[i]->h*clusterList[i]->w;

			totalWidth +=clusterList[i]->w;
			}


		spacing = sqrt(area)*spacing;

		totalWidth =0.0f;
		area = 0.0f;

		for ( i=0; i < clusterList.Count(); i++)
			{
			clusterList[i]->h = clusterList[i]->h+spacing;
			clusterList[i]->w = clusterList[i]->w+spacing;

			totalWidth +=clusterList[i]->w;
			area += clusterList[i]->h * clusterList[i]->w;

			}


		FlipClusters(alignWidth,spacing);



//sort bounding data by height 
		clusterList.Sort(CompTable);

//		BOOL combineClusters = TRUE;
		if (combineClusters)
			{
			if (!CollapseClusters(spacing)) return FALSE;
			}

		totalWidth = 0.0f;
		for ( i=0; i < clusterList.Count(); i++)
			totalWidth += clusterList[i]->w;

//now shuffle the clusters to fit in normalized space
		int split = 2;
		BOOL done = FALSE;

		while (!done)
			{
			Tab<int> splitList;
			splitList.SetCount(split);


			float maxHeight = clusterList[0]->h;
			float maxWidth = totalWidth/(float) split;

			for (int i =0; i < splitList.Count(); i++)
				splitList[i] = -1;

			splitList[0] = 0;
			int index =1;

			int currentChunk = 0;
			
			
			while (index < split)
				{
				float currentWidth = 0.f;
				while ( (currentWidth < maxWidth) && (currentChunk < (clusterList.Count()-1)) )
					{
					currentWidth +=  clusterList[currentChunk]->w;
					currentChunk++;
					}

				splitList[index] = currentChunk;
				index++;
				}
			
			float totalHeight = 0.0f;
			for (i = 0; i <  splitList.Count(); i++)
				{
				if ( (splitList[i] >=0) && (splitList[i] <clusterList.Count()) )
					totalHeight += clusterList[splitList[i]]->h;
				}
			if (totalHeight >= maxWidth)
				{
				done = TRUE;
//now move points to there respective spots
				int start =0;
				Point3 corner(0.0f,0.0f,0.0f);
				for (int i =0; i < splitList.Count(); i++)
					{
					if (splitList[i] >=0)
						{
						int end;
						if (i == splitList.Count()-1)
							end = clusterList.Count();
						else end = splitList[i+1];
						float xOffset =0;
						for (int j = start; j < end; j++)
							{
							Point3 boundingCorner;
							boundingCorner.x = clusterList[j]->bounds.pmin.x;
							boundingCorner.y = clusterList[j]->bounds.pmin.y;
							boundingCorner.z = 0;
							Point3 vec = corner - boundingCorner;
							clusterList[j]->offset = vec;
							clusterList[j]->offset.x += xOffset;
							xOffset += clusterList[j]->w;


							}
						if (i != splitList.Count()-1)
							{
						    start = splitList[i+1];	
							int index = splitList[i];
							corner.y += clusterList[splitList[i]]->h;
							}
						}
					}

				BitArray hitVerts;
				hitVerts.SetSize( TVMaps.v.Count());
				hitVerts.ClearAll();
				for (i=0; i < clusterList.Count(); i++)
					{

					for (int j = 0; j <  clusterList[i]->faces.Count(); j++)
						{
//need to put patch handles in here also
						int findex = clusterList[i]->faces[j];

						for (int m = 0; m <  TVMaps.f[findex]->count; m++)
							{
//need to put patch handles in here also
							int index = TVMaps.f[findex]->t[m];



							if (!hitVerts[index])
								{

								if ((index >=0) && (index < TVMaps.v.Count()))
									{
									TVMaps.v[index].p = TVMaps.v[index].p + clusterList[i]->offset;
									if (TVMaps.cont[index]) TVMaps.cont[index]->SetValue(0,&TVMaps.v[index].p);
									}

							 	hitVerts.Set(index);
								}

							if ( (TVMaps.f[findex]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[findex]->vecs))
									{
									if (TVMaps.f[findex]->flags & FLAG_INTERIOR)
										{
										index = TVMaps.f[findex]->vecs->interiors[m];
										if ((!hitVerts[index]) && (index >=0) && (index < TVMaps.v.Count()))
											{
											TVMaps.v[index].p = TVMaps.v[index].p + clusterList[i]->offset;
											if (TVMaps.cont[index]) TVMaps.cont[index]->SetValue(0,&TVMaps.v[index].p);
											hitVerts.Set(index);
											}
										}
										
									index = TVMaps.f[findex]->vecs->handles[m*2];

									if ((!hitVerts[index]) && (index >=0) && (index < TVMaps.v.Count()))
										{
										TVMaps.v[index].p = TVMaps.v[index].p + clusterList[i]->offset;
										if (TVMaps.cont[index]) TVMaps.cont[index]->SetValue(0,&TVMaps.v[index].p);
										hitVerts.Set(index);
										}
										
									index = TVMaps.f[findex]->vecs->handles[m*2+1];

									if ((!hitVerts[index]) && (index >=0) && (index < TVMaps.v.Count()))
										{
										TVMaps.v[index].p = TVMaps.v[index].p + clusterList[i]->offset;
										if (TVMaps.cont[index]) TVMaps.cont[index]->SetValue(0,&TVMaps.v[index].p);
										hitVerts.Set(index);
										}
					
									}


							}
						}
					}
				}
			else split++;

			}

	return TRUE;

	}

static void UnwrapMatrixFromNormal(Point3& normal, Matrix3& mat)
	{
	Point3 vx;
	vx.z = .0f;
	vx.x = -normal.y;
	vx.y = normal.x;	
	if ( vx.x == .0f && vx.y == .0f ) {
		vx.x = 1.0f;
		}
	mat.SetRow(0,vx);
	mat.SetRow(1,normal^vx);
	mat.SetRow(2,normal);
	mat.SetTrans(Point3(0,0,0));
	mat.NoScale();
	}
	


void	UnwrapMod::Pack(int layoutType, float spacing, BOOL normalize, BOOL rotateClusters, BOOL fillHoles)
	{
	if (!theHold.Holding())
		{
		theHold.SuperBegin();
		theHold.Begin();
		}

	HoldPointsAndFaces();	

	Point3 normal(0.0f,0.0f,1.0f);


	TSTR statusMessage;
	BOOL bContinue = TRUE;

//FIX
	int holdSubMode = fnGetTVSubMode();
	fnSetTVSubMode(TVVERTMODE);

	
	bContinue = BuildClusterFromTVVertexElement();

	BitArray sel;
	sel.SetSize(TVMaps.f.Count());

	gBArea = 0.0f;

	int initialCluster = clusterList.Count();

	if (bContinue)
		{

		for (int i =0; i < clusterList.Count(); i++)
			{
//center all the the clusters
			BitArray usedVerts;
			
			usedVerts.SetSize(TVMaps.v.Count());
			usedVerts.ClearAll();

			for (int j = 0; j <  clusterList[i]->faces.Count(); j++)
				{
				int faceIndex = clusterList[i]->faces[j];
				for (int k = 0; k <  TVMaps.f[faceIndex]->count; k++)
					{
					int index = TVMaps.f[faceIndex]->t[k];
					usedVerts.Set(index);

					if ( (TVMaps.f[faceIndex]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[faceIndex]->vecs))
						{
						if (TVMaps.f[faceIndex]->flags & FLAG_INTERIOR)
							{
							index = TVMaps.f[faceIndex]->vecs->interiors[k];
							if ( (index >=0) && (index < TVMaps.v.Count()))
								{
								usedVerts.Set(index);
								}
							}
				
						index = TVMaps.f[faceIndex]->vecs->handles[k*2];

						if ( (index >=0) && (index < TVMaps.v.Count()))
							{
							usedVerts.Set(index);
							}
										
						index = TVMaps.f[faceIndex]->vecs->handles[k*2+1];

						if ( (index >=0) && (index < TVMaps.v.Count()))
							{
							usedVerts.Set(index);
							}
					
						}
					}				
				}

			Box3 bounds;
			bounds.Init();

			if (i == 75) vsel = usedVerts;

			for (j =0; j < usedVerts.GetSize(); j++)
				{
				if (usedVerts[j])
					{
					bounds += TVMaps.v[j].p;
					}

				}
			Point3 center = bounds.Center();
			for (j =0; j < usedVerts.GetSize(); j++)
				{
				if (usedVerts[j])
					{
					Point3 p = TVMaps.v[j].p - center;
					TVMaps.v[j].p = p;
					if (TVMaps.cont[j]) 
						TVMaps.cont[j]->SetValue(0,&TVMaps.v[j].p);
					}
				}

			}
		


		if (layoutType == 1)
			bContinue = LayoutClusters( spacing, rotateClusters, TRUE,fillHoles);
		else bContinue = LayoutClusters2( spacing, rotateClusters, fillHoles);


//normalize map to 0,0 to 1,1

		if ((bContinue) && (normalize))
			{
			NormalizeCluster();
			}


		}

	CleanUpDeadVertices();

//FIX
	fnSetTVSubMode(holdSubMode);

	if (bContinue)
		{
		theHold.Accept(_T(GetString(IDS_PW_PACK)));
		theHold.SuperAccept(_T(GetString(IDS_PW_PACK)));
		sel.ClearAll();
		fnSelectPolygonsUpdate(&sel, FALSE);
		}
	else
		{
		theHold.Cancel();
		theHold.SuperCancel();
		}

	RebuildEdges();
	NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
	InvalidateView();


#ifdef DEBUGMODE 
	if (gDebugLevel >= 1)
		{
		int finalCluster = clusterList.Count();
		gEdgeHeight = 0.0f;
		gEdgeWidth = 0.0f;
		for (int i =0; i < clusterList.Count(); i++)
			{
			gEdgeHeight += clusterList[i]->h;
			gEdgeWidth += clusterList[i]->w;
				
			}

		ScriptPrint("Surface Area %f bounds area %f  per used %f\n",gSArea,gBArea,gSArea/gBArea); 
		ScriptPrint("Edge Height %f Edge Width %f\n",gEdgeHeight,gEdgeWidth); 
		ScriptPrint("Initial Clusters %d finalClusters %d\n",initialCluster,finalCluster); 
		}

#endif

	FreeClusterList();

	statusMessage.printf("Done, area coverage %3.2f",(gSArea/gBArea)*100.f);
	Bail(ip,statusMessage,0);
	}



void	UnwrapMod::fnPack( int method, float spacing, BOOL normalize, BOOL rotate, BOOL fillHoles)
	{
	Pack(method, spacing, normalize, rotate, fillHoles);
	}

void	UnwrapMod::fnPackNoParams()
	{
	Pack(packMethod, packSpacing, packNormalize, packRotate, packFillHoles);
	}

void	UnwrapMod::fnPackDialog()
	{
//bring up the dialog
	DialogBoxParam(	hInstance,
							MAKEINTRESOURCE(IDD_PACKDIALOG),
							hWnd,
							UnwrapPackFloaterDlgProc,
							(LPARAM)this );
	}

void	UnwrapMod::SetPackDialogPos()
	{
	if (packWindowPos.length != 0) 
		SetWindowPlacement(packHWND,&packWindowPos);
	}

void	UnwrapMod::SavePackDialogPos()
	{
	packWindowPos.length = sizeof(WINDOWPLACEMENT); 
	GetWindowPlacement(packHWND,&packWindowPos);
	}

INT_PTR CALLBACK UnwrapPackFloaterDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	UnwrapMod *mod = (UnwrapMod*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	//POINTS p = MAKEPOINTS(lParam);	commented out by sca 10/7/98 -- causing warning since unused.
	
	static ISpinnerControl *iSpacing = NULL;

	switch (msg) {
		case WM_INITDIALOG:

			{
			mod = (UnwrapMod*)lParam;
			mod->unfoldHWND = hWnd;

			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);

//create spinners and set value
			iSpacing = SetupFloatSpinner(
				hWnd,IDC_UNWRAP_SPACINGSPIN,IDC_UNWRAP_SPACING,
				0.0f,1.0f,mod->packSpacing);	

//set align cluster
			CheckDlgButton(hWnd,IDC_NORMALIZE_CHECK,mod->packNormalize);
			CheckDlgButton(hWnd,IDC_ROTATE_CHECK,mod->packRotate);
			CheckDlgButton(hWnd,IDC_COLLAPSE_CHECK,mod->packFillHoles);

			HWND hMethod = GetDlgItem(hWnd,IDC_COMBO1);
			SendMessage(hMethod, CB_RESETCONTENT, 0, 0);

			SendMessage(hMethod, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_RECURSIVEPACK));
			SendMessage(hMethod, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_LINEARPACK));

			SendMessage(hMethod, CB_SETCURSEL, mod->packMethod, 0L);

			mod->SetPackDialogPos();

			break;
			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_OK:
					{
					mod->SaveFlattenDialogPos();


					int tempMethod;
					float tempSpacing;
					BOOL tempNormalize, tempRotate, tempCollapse;
					tempSpacing = mod->packSpacing;
					tempNormalize = mod->packNormalize;
					tempRotate = mod->packRotate;
					tempCollapse = mod->packFillHoles;
					tempMethod = mod->packMethod;


					mod->packSpacing = iSpacing->GetFVal();

					mod->packNormalize = IsDlgButtonChecked(hWnd,IDC_NORMALIZE_CHECK);
					mod->packRotate = IsDlgButtonChecked(hWnd,IDC_ROTATE_CHECK);
					mod->packFillHoles = IsDlgButtonChecked(hWnd,IDC_COLLAPSE_CHECK); 

					HWND hMethod = GetDlgItem(hWnd,IDC_COMBO1);
					mod->packMethod = SendMessage(hMethod, CB_GETCURSEL, 0L, 0);

					mod->fnPackNoParams();

					mod->packSpacing = tempSpacing;
					mod->packNormalize = tempNormalize;
					mod->packRotate = tempRotate;
					mod->packFillHoles= tempCollapse;
					mod->packMethod = tempMethod;


					ReleaseISpinner(iSpacing);
					iSpacing = NULL;

					EndDialog(hWnd,1);
					
					break;
					}
				case IDC_CANCEL:
					{
				
					mod->SaveFlattenDialogPos();
					ReleaseISpinner(iSpacing);
					iSpacing = NULL;

					EndDialog(hWnd,0);

					break;
					}
				case IDC_DEFAULT:
					{
//get bias
					mod->packSpacing = iSpacing->GetFVal();

//get align
					mod->packNormalize = IsDlgButtonChecked(hWnd,IDC_NORMALIZE_CHECK);
					mod->packRotate = IsDlgButtonChecked(hWnd,IDC_ROTATE_CHECK);
					mod->packFillHoles = IsDlgButtonChecked(hWnd,IDC_COLLAPSE_CHECK); 

					
					HWND hMethod = GetDlgItem(hWnd,IDC_COMBO1);
					mod->packMethod = SendMessage(hMethod, CB_GETCURSEL, 0L, 0);
					
//set as defaults
					break;
					}

				}
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}

