/**********************************************************************
 *<
	FILE: RayMeshIntersect.cpp

	DESCRIPTION:	Appwizard generated plugin

	CREATED BY: 

	HISTORY: 


  fix the closest to point so the radius is split into width/height/depth


 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "RayMeshIntersect.h"



static RayMeshGridIntersectClassDesc rayMeshGridIntersectDesc;
ClassDesc2* GetRayMeshGridIntersectDesc() { return &rayMeshGridIntersectDesc; }


static ParamBlockDesc2 raymeshgridintersect_param_blk ( raymeshgridintersect_params, _T("params"),  0, &rayMeshGridIntersectDesc, 
	P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//rollout
	IDD_PANEL, IDS_PARAMS, 0, 0, NULL,
	// params
	pb_nodelist,	_T("nodeList"), 		TYPE_INODE_TAB, 	0, P_VARIABLE_SIZE, 	IDS_NODELIST, 
		end,
	end
	);


static FPInterfaceDesc raymeshgridintersect_interface(
    RAYMESHGRIDINTERSECT_V1_INTERFACE, _T("rayMeshGridIntersectOps"), 0, &rayMeshGridIntersectDesc, FP_MIXIN,
		grid_free, _T("free"), 0, TYPE_VOID, 0, 0,
		grid_initialize, _T("initialize"), 0, TYPE_VOID, 0, 1,
			_T("gridSize"), 0, TYPE_INT,
		grid_buildgrid, _T("buildGrid"), 0, TYPE_VOID, 0, 0,
		grid_addnode, _T("addNode"), 0, TYPE_VOID, 0, 1,
			_T("addNode"), 0, TYPE_INODE,
		grid_intersectbox, _T("intersectBox"), 0, TYPE_INT, 0, 2,
			_T("min"), 0, TYPE_POINT3,
			_T("max"), 0, TYPE_POINT3,
		grid_intersectsphere, _T("intersectSphere"), 0, TYPE_INT, 0, 2,
			_T("center"), 0, TYPE_POINT3,
			_T("radius"), 0, TYPE_FLOAT,
		grid_intersectray, _T("intersectRay"), 0, TYPE_INT, 0, 3,
			_T("p"), 0, TYPE_POINT3,
			_T("dir"), 0, TYPE_POINT3,
			_T("doudbleSided"), 0, TYPE_BOOL,
		grid_intersectsegment, _T("intersectSegment"), 0, TYPE_INT, 0, 3,
			_T("p1"), 0, TYPE_POINT3,
			_T("p2"), 0, TYPE_POINT3,
			_T("doudbleSided"), 0, TYPE_BOOL,
		grid_gethitface, _T("getHitFace"), 0, TYPE_INT, 0, 1,
			_T("index"), 0, TYPE_INT,
		grid_gethitbary, _T("getHitBary"), 0, TYPE_POINT3, 0, 1,
			_T("index"), 0, TYPE_INT,
		grid_gethitnorm, _T("getHitNormal"), 0, TYPE_POINT3, 0, 1,
			_T("index"), 0, TYPE_INT,
		grid_gethitdist, _T("getHitDist"), 0, TYPE_FLOAT, 0, 1,
			_T("index"), 0, TYPE_INT,
		grid_intersectsegmentdebug, _T("intersectSegmentDebug"), 0, TYPE_INT, 0, 4,
			_T("p1"), 0, TYPE_POINT3,
			_T("p2"), 0, TYPE_POINT3,
			_T("doudbleSided"), 0, TYPE_BOOL,
			_T("gridID"), 0, TYPE_INT,
		grid_getclosesthit, _T("getClosestHit"), 0, TYPE_INT, 0, 0,
		grid_getfarthesthit, _T("getFarthestHit"), 0, TYPE_INT, 0, 0,
		grid_closestface, _T("closestFace"), 0, TYPE_INT, 0, 1,
			_T("p"), 0, TYPE_POINT3,


		grid_getperpdist, _T("getPerpDist"), 0, TYPE_FLOAT, 0, 1,
			_T("index"), 0, TYPE_INT,

		grid_clearstats, _T("ClearStats"), 0, TYPE_VOID, 0, 0,
		grid_printstats, _T("PrintStats"), 0, TYPE_VOID, 0, 0,

      end
      );


void *RayMeshGridIntersectClassDesc::Create(BOOL loading)
		{
		AddInterface(&raymeshgridintersect_interface);
		return new RayMeshGridIntersect;
		}

FPInterfaceDesc* IRayMeshGridIntersect_InterfaceV1::GetDesc()
	{
	 return &raymeshgridintersect_interface;
	}


RayMeshGridIntersect::RayMeshGridIntersect()
{
rayMeshGridIntersectDesc.MakeAutoParamBlocks(this);

}

RayMeshGridIntersect::~RayMeshGridIntersect()
{

}


RefResult RayMeshGridIntersect::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	return REF_SUCCEED;
}


void RayMeshGridIntersect::fnFree()
{
	gridData.FreeGrid();
}

void RayMeshGridIntersect::fnInitialize(int size)
{
	gridData.Initialize(size);
}

void	RayMeshGridIntersect::fnAddMesh(Mesh *msh, Matrix3 tm)
{
	gridData.AddMesh(msh,tm);
}


void RayMeshGridIntersect::fnAddNode(INode *node)
{
	gridData.AddNode(node);
}

void RayMeshGridIntersect::fnBuildGrid()
{
	gridData.BuildGrid();
}



int	RayMeshGridIntersect::fnIntersectBox(Point3 min, Point3 max)
{
	Box3 b;
	b.Init();
	b += min;
	b += max;
	return gridData.IntersectBox(b);
}
int	RayMeshGridIntersect::fnIntersectSphere(Point3 p, float radius)
{
	return gridData.IntersectSphere(p,radius);
}

int	RayMeshGridIntersect::fnIntersectRay(Point3 p, Point3 dir,  BOOL doubleSided)
{
	dir = Normalize(dir);
	return gridData.IntersectRay(p, dir, FALSE, doubleSided);

}

int	RayMeshGridIntersect::fnIntersectSegment(Point3 p1, Point3 p2,  BOOL doubleSided)
{
	Point3 dir = p2 - p1;
	return gridData.IntersectRay(p1, dir, TRUE, doubleSided);

}

int	RayMeshGridIntersect::fnIntersectSegmentDebug(Point3 p1, Point3 p2,  BOOL doubleSided, int whichGrid)
{
	Point3 dir = p2 - p1;
	return gridData.IntersectRay(p1, dir, TRUE, doubleSided,whichGrid);

}

int	RayMeshGridIntersect::fnGetHitFace(int index)
{
	index--;
	return gridData.GetHitFace(index)+1;
}

Point3	RayMeshGridIntersect::fnGetHitBary(int index)
{
	index--;
	return gridData.GetHitBary(index);
}

Point3	RayMeshGridIntersect::fnGetHitNorm(int index)
{
	index--;
	return gridData.GetHitNorm(index);
}

float	RayMeshGridIntersect::fnGetHitDist(int index)
{
	index--;
	return gridData.GetHitDist(index);
}

int	RayMeshGridIntersect::fnGetClosestHit()
{
	return gridData.GetClosestHit()+1;
}

int	RayMeshGridIntersect::fnGetFarthestHit()
{
	return gridData.GetFarthestHit()+1;
}

int	RayMeshGridIntersect::fnClosestFace(Point3 p)
{
	return gridData.ClosestFace(p);
}


float	RayMeshGridIntersect::fnGetPerpDist(int index)
{
	index--;
	return gridData.GetPerpDist(index);
}


void	RayMeshGridIntersect::fnClearStats()
{
	gridData.summedFaces = 0.0f;
	gridData.tally = 0.0f;
}
void	RayMeshGridIntersect::fnPrintStats()
{
	ScriptPrint("Number of faces %0.0f  average number of faces searched %0.0f\n",gridData.numFaces,gridData.summedFaces/gridData.tally);
}


Point3	RayMeshGridIntersect::fnGetHitPoint(int index)
{
	index--;
	return gridData.GetHitPoint(index);
} 

void Grid::FreeGrid()
{

	summedFaces = 0.0f;
	tally = 0.0f;

	for (int i = 0; i < (gridX.Count());i++)
		 if (gridX[i]) delete gridX[i];

	for (i = 0; i < (gridY.Count());i++)
		 if (gridY[i]) delete gridY[i];

	for (i = 0; i < (gridZ.Count());i++)
		 if (gridZ[i]) delete gridZ[i];

	gridX.ZeroCount();
	gridY.ZeroCount();
	gridZ.ZeroCount();

	for (i = 0; i < nodeList.Count(); i++)
		if (nodeList[i]) delete nodeList[i];
	nodeList.ZeroCount();

}

void Grid::Initialize(int width)
{
	this->width = width;

	FreeGrid();
	int size = width*width;
	gridX.SetCount(size);
	gridY.SetCount(size);
	gridZ.SetCount(size);

	for (int i = 0; i < size;i++)
		 gridX[i] = NULL;//new CellList;
	for (i = 0; i < size;i++)
		 gridY[i] = NULL;//= new CellList;
	for (i = 0; i < size;i++)
		 gridZ[i] = NULL;//= new CellList;
}


void Grid::AddMesh(Mesh *msh, Matrix3 tm)
{
	NodeProperties *temp = new NodeProperties;
	temp->node = NULL;

	temp->verts.SetCount(msh->numVerts);
	for (int i = 0; i < msh->numVerts; i++)
		temp->verts[i] = msh->verts[i]*tm;
	temp->faces.SetCount(msh->numFaces);
	for (i = 0; i < msh->numFaces; i++)
		temp->faces[i] = msh->faces[i];
	nodeList.Append(1,&temp,10);
}

void Grid::AddNode(INode *node)
{
	NodeProperties *temp = new NodeProperties;
	temp->node = node;
	nodeList.Append(1,&temp,10);
}


void Grid::ComputeStartPoints(Point3 min, Point3 max, IPoint3 &pStart,IPoint3 &pEnd)
{
	float fwidth = bounds.pmax.x - bounds.pmin.x;
	float fheight = bounds.pmax.y - bounds.pmin.y;
	float fdepth = bounds.pmax.z - bounds.pmin.z;
//compute our grid start x,y,z
	min = min - bounds.pmin;
	max = max - bounds.pmin;
	float wInc, hInc,dInc;
	wInc = fwidth/width;
	hInc = fheight/width;
	dInc = fdepth/width;

	pStart.x = min.x/wInc-1;
	pStart.y = min.y/hInc-1;
	pStart.z = min.z/dInc-1;

	pEnd.x = max.x/wInc+1;
	pEnd.y = max.y/hInc+1;
	pEnd.z = max.z/dInc+1;

/*	
	pStart.x = (int)(float(width-1) * min.x/fwidth);
	pStart.y = (int)(float(width-1) * min.y/fheight);
	pStart.z = (int)(float(width-1) * min.z/fdepth);

//compute our grid end x,y,z
	pEnd.x = (int)(float(width-1) * max.x/fwidth);
	pEnd.y = (int)(float(width-1) * max.y/fheight);
	pEnd.z = (int)(float(width-1) * max.z/fdepth);
*/
//check bounds
	if (pStart.x < 0) 
		{
		pStart.x = 0;
//		assert(0);
		}
	if (pStart.y < 0) 
		{
		pStart.y = 0;
//		assert(0);
		}
	if (pStart.z < 0) 
		{
		pStart.z = 0;
//		assert(0);
		}


	if (pEnd.x >= width) 
		{
		pEnd.x = width-1;
//		assert(0);
		}
	if (pEnd.y >= width) 
		{
		pEnd.y = width-1;
//		assert(0);
		}
	if (pEnd.z >= width) 
		{
		pEnd.z = width-1;
//		assert(0);
		}

}
void Grid::AddFace(int nodeIndex, int faceIndex, Box3 faceBounds, Point3 a, Point3 b, Point3 c)
{
	float dist = Length(faceBounds.Width()) *0.05f;
	if ( (faceBounds.pmax.x- faceBounds.pmin.x) == 0.0f)
		{
		faceBounds.pmax.x += dist;
		faceBounds.pmin.x -= dist;
		}

	if ( (faceBounds.pmax.y- faceBounds.pmin.y) == 0.0f)
		{
		faceBounds.pmax.y += dist;
		faceBounds.pmin.y -= dist;
		}

	if ( (faceBounds.pmax.z- faceBounds.pmin.z) == 0.0f)
		{
		faceBounds.pmax.z += dist;
		faceBounds.pmin.z -= dist;
		}

	IPoint3 pStart, pEnd;
	ComputeStartPoints(faceBounds.pmin, faceBounds.pmax, pStart,pEnd);

//add to gridX
//intersect triangle with out grid and add


	for (int y = pStart.y; y <= pEnd.y; y++)
		{
		for (int x = pStart.x; x <= pEnd.x; x++)
			{
//FIX ADD later intersect the face with our cells and only add those
//check if we have a cell if not create one
			int index = y * width + x;

			if (gridX[index] == NULL)
				gridX[index] = new CellList;
			Cell temp;
			temp.nodeIndex = nodeIndex;
			temp.faceIndex = faceIndex;

			gridX[index]->data.Append(1,&temp,10);		
			}
		}



//add to gridY
	for (y = pStart.y; y <= pEnd.y; y++)
		{
		for (int z = pStart.z; z <= pEnd.z; z++)
			{
//FIX ADD later intersect the face with our cells and only add those
//check if we have a cell if not create one
			int index = y * width + z;

			if (gridY[index] == NULL)
				gridY[index] = new CellList;
			Cell temp;
			temp.nodeIndex = nodeIndex;
			temp.faceIndex = faceIndex;

			gridY[index]->data.Append(1,&temp,100);		
			}
		}

//intersect triangle with out grid and add
//add to gridZ
	for (int x = pStart.x; x <= pEnd.x; x++)
		{
		for (int z = pStart.z; z <= pEnd.z; z++)
			{
//FIX ADD later intersect the face with our cells and only add those
//check if we have a cell if not create one
			int index = x * width + z;

			if (gridZ[index] == NULL)
				gridZ[index] = new CellList;
			Cell temp;
			temp.nodeIndex = nodeIndex;
			temp.faceIndex = faceIndex;

			gridZ[index]->data.Append(1,&temp,100);		
			}
		}

//intersect triangle with out grid and add

}

void Grid::BuildGrid()
{
	//get our mesh face and vertex data	in world space
	TimeValue t = GetCOREInterface()->GetTime();
	
	bounds.Init();

	for (int i = 0; i < nodeList.Count(); i++)
		{
		if (nodeList[i]->node)
			{
			//check to make sure it is a mesh
			Mesh *msh = NULL;
			TriObject *collapsedtobj = NULL;
			ObjectState sos = nodeList[i]->node->EvalWorldState(t);
			if (sos.obj->IsSubClassOf(triObjectClassID))
				{
				TriObject *tobj = (TriObject*)sos.obj;
				msh = &tobj->GetMesh();
				}
//collapse it to a mesh
			else
				{
				if (sos.obj->CanConvertToType(triObjectClassID))
					{
					collapsedtobj = (TriObject*) sos.obj->ConvertToType(t,triObjectClassID);
					msh = &collapsedtobj->GetMesh();
					}
				else 
					{
					nodeList[i]->node = NULL;
					continue;
					}
				}

			//get the tm
			Matrix3 tm = nodeList[i]->node->GetObjectTM(t,&FOREVER);
			//put the vertices in world space
			nodeList[i]->verts.SetCount(msh->numVerts);
			nodeList[i]->faces.SetCount(msh->numFaces);
			nodeList[i]->fnorms.SetCount(msh->numFaces);
//add them to our list
			nodeList[i]->bounds.Init();
			for (int j = 0; j < msh->numVerts; j++)
				{
				nodeList[i]->verts[j] = msh->verts[j] * tm;
				nodeList[i]->bounds += nodeList[i]->verts[j];
				bounds += nodeList[i]->verts[j];
				}
			for (j = 0; j < msh->numFaces; j++)
				{
				nodeList[i]->faces[j] = msh->faces[j];
				Point3 norm(0.0f,0.0f,0.0f);
				Point3 vecA,vecB;
				Point3 a,b,c;
				a = nodeList[i]->verts[msh->faces[j].v[0]];
				b = nodeList[i]->verts[msh->faces[j].v[1]];
				c = nodeList[i]->verts[msh->faces[j].v[2]];
				vecA = Normalize(a - b);
				vecB = Normalize(c - b);
				norm = Normalize(CrossProd(vecA,vecB));
				nodeList[i]->fnorms[j] = norm;

				}
			
			if (collapsedtobj) collapsedtobj->DeleteThis();

			}
		else
			{
//add them to our list
			nodeList[i]->bounds.Init();
			for (int j = 0; j < nodeList[i]->verts.Count(); j++)
				{
				nodeList[i]->bounds += nodeList[i]->verts[j];
				bounds += nodeList[i]->verts[j];
				}
			nodeList[i]->fnorms.SetCount(nodeList[i]->faces.Count());
			for (j = 0; j < nodeList[i]->faces.Count(); j++)
				{
				Point3 norm(0.0f,0.0f,0.0f);
				Point3 vecA,vecB;
				Point3 a,b,c;
				a = nodeList[i]->verts[nodeList[i]->faces[j].v[0]];
				b = nodeList[i]->verts[nodeList[i]->faces[j].v[1]];
				c = nodeList[i]->verts[nodeList[i]->faces[j].v[2]];
				vecA = Normalize(a - b);
				vecB = Normalize(c - b);
				norm = Normalize(CrossProd(vecA,vecB));
				nodeList[i]->fnorms[j] = norm;

				}
			}
		}

	//loop through all our faces
	for (i = 0; i < nodeList.Count(); i++)
		{
//		if (nodeList[i]->node)
			{
			numFaces = nodeList[i]->faces.Count();
			for (int j = 0; j < nodeList[i]->faces.Count(); j++)
				{

				int ia,ib,ic;
				ia = nodeList[i]->faces[j].v[0];
				ib = nodeList[i]->faces[j].v[1];
				ic = nodeList[i]->faces[j].v[2];
				Point3 a,b,c;
				a = nodeList[i]->verts[ia];
				b = nodeList[i]->verts[ib];
				c = nodeList[i]->verts[ic];
				Box3 bb;
				bb.Init();
				bb+= a;
				bb+= b;
				bb+= c;
//				bb.Scale(1.25f);
//add each face
				AddFace(i, j, bb, a, b, c);
				}

			}
		}

	
}


int Grid::IntersectSphere(Point3 p, int radius)
{

	BitArray potentialXHitList,potentialYHitList,potentialZHitList;

	potentialXHitList.SetSize(nodeList[0]->faces.Count());
	potentialYHitList.SetSize(nodeList[0]->faces.Count());
	potentialZHitList.SetSize(nodeList[0]->faces.Count());

	potentialXHitList.ClearAll();
	potentialYHitList.ClearAll();
	potentialZHitList.ClearAll();

//get bounds
	Point3 min, max;
	min.x = p.x - radius;
	min.y = p.y - radius;
	min.z = p.z - radius;

	max.x = p.x + radius;
	max.y = p.y + radius;
	max.z = p.z + radius;

//compute our grid start x,y,z
	IPoint3 pStart, pEnd;

	ComputeStartPoints(min, max, pStart,pEnd);

	for (int y = pStart.y; y <= pEnd.y; y++)
		{
		for (int x = pStart.x; x <= pEnd.x; x++)
			{
			//optimize here see if quad intersects sphere
			//get quad that the box is in
			
			//if so check to see if any of the faces also intersect
			int index = y * width + x;


			if (gridX[index])
				{
				
				CellList *temp = gridX[index];
				for (int i = 0; i < temp->data.Count(); i++)
					{
					int findex = temp->data[i].faceIndex;
					potentialXHitList.Set(findex);
					}

				}
			}
		}

	for (y = pStart.y; y <= pEnd.y; y++)
		{
		for (int x = pStart.z;  x <= pEnd.z; x++)
			{
			//optimize here see if quad intersects sphere
			//get quad that the box is in
			
			//if so check to see if any of the faces also intersect
			int index = y * width + x;


			if (gridY[index])
				{
				
				CellList *temp = gridY[index];
				for (int i = 0; i < temp->data.Count(); i++)
					{
					int findex = temp->data[i].faceIndex;
					potentialYHitList.Set(findex);
					}

				}
			}
		}


	for (y = pStart.x; y <= pEnd.x; y++)
		{
		for (int x = pStart.z; x <= pEnd.z; x++)
			{
			//optimize here see if quad intersects sphere
			//get quad that the box is in
			
			//if so check to see if any of the faces also intersect
			int index = y * width + x;


			if (gridZ[index])
				{
				
				CellList *temp = gridZ[index];
				for (int i = 0; i < temp->data.Count(); i++)
					{
					int findex = temp->data[i].faceIndex;
					potentialZHitList.Set(findex);
					}
				}
			}
		}
	int ct = nodeList[0]->faces.Count();
	float radiusSquared = radius * radius;
	hitList.ZeroCount();
	for (int i = 0; i < ct; i++)
		{
		if (potentialYHitList[i] && potentialYHitList[i] && potentialZHitList[i] )
			{
			//see if face intersects sphere
			//check verts then face
			int a,b,c;
			Point3 pa,pb,pc;
			nodeList[0]->faces[i];
			a = nodeList[0]->faces[i].v[0];
			b = nodeList[0]->faces[i].v[1];
			c = nodeList[0]->faces[i].v[2];
			pa = nodeList[0]->verts[a];
			pb = nodeList[0]->verts[b];
			pc = nodeList[0]->verts[c];
			HitList temp;
			temp.distance = -1.0f;
			float ls = LengthSquared(p-pa);
			float lastls=-1.0f;
			if (ls < radiusSquared) 
				{
				temp.faceIndex = i;
				temp.nodeIndex = 0;
				temp.distance = sqrt(ls);
				lastls = ls;
				}
			ls = LengthSquared(p-pb);
			if (ls < radiusSquared) 
				{
				temp.faceIndex = i;
				temp.nodeIndex = 0;
				if ((lastls==-1.0f) || (ls < lastls))
					{
					temp.distance = sqrt(ls);
					lastls = ls;
					}
				}
			ls = LengthSquared(p-pc);
			if (ls < radiusSquared) 
				{
				temp.faceIndex = i;
				temp.nodeIndex = 0;
				if ((lastls==-1.0f) || (ls < lastls))
					{
					temp.distance = sqrt(ls);
					lastls = ls;
					}
				}
			
			if (lastls != -1.0f) hitList.Append(1,&temp,100);


			
			}
		}


	return hitList.Count();
}


int Grid::IntersectBox(Box3 bbox)
{
	BitArray potentialXHitList,potentialYHitList,potentialZHitList;

	potentialXHitList.SetSize(nodeList[0]->faces.Count());
	potentialYHitList.SetSize(nodeList[0]->faces.Count());
	potentialZHitList.SetSize(nodeList[0]->faces.Count());

	potentialXHitList.ClearAll();
	potentialYHitList.ClearAll();
	potentialZHitList.ClearAll();

//get bounds
	Point3 min, max;
	min = bbox.pmin;
	max = bbox.pmax;

//compute our grid start x,y,z
	IPoint3 pStart, pEnd;

	ComputeStartPoints(min, max, pStart,pEnd);

	for (int y = pStart.y; y <= pEnd.y; y++)
		{
		for (int x = pStart.x; x <= pEnd.x; x++)
			{
			//optimize here see if quad intersects sphere
			//get quad that the box is in
			
			//if so check to see if any of the faces also intersect
			int index = y * width + x;


			if (gridX[index])
				{
				
				CellList *temp = gridX[index];
				for (int i = 0; i < temp->data.Count(); i++)
					{
					int findex = temp->data[i].faceIndex;
					potentialXHitList.Set(findex);
					}

				}
			}
		}

	for (y = pStart.y; y <= pEnd.y; y++)
		{
		for (int x = pStart.z;  x <= pEnd.z; x++)
			{
			//optimize here see if quad intersects sphere
			//get quad that the box is in
			
			//if so check to see if any of the faces also intersect
			int index = y * width + x;


			if (gridY[index])
				{
				
				CellList *temp = gridY[index];
				for (int i = 0; i < temp->data.Count(); i++)
					{
					int findex = temp->data[i].faceIndex;
					potentialYHitList.Set(findex);
					}

				}
			}
		}


	for (y = pStart.x; y <= pEnd.x; y++)
		{
		for (int x = pStart.z; x <= pEnd.z; x++)
			{
			//optimize here see if quad intersects sphere
			//get quad that the box is in
			
			//if so check to see if any of the faces also intersect
			int index = y * width + x;


			if (gridZ[index])
				{
				
				CellList *temp = gridZ[index];
				for (int i = 0; i < temp->data.Count(); i++)
					{
					int findex = temp->data[i].faceIndex;
					potentialZHitList.Set(findex);
					}
				}
			}
		}
	int ct = nodeList[0]->faces.Count();
	hitList.ZeroCount();
	for (int i = 0; i < ct; i++)
		{
		if (potentialYHitList[i] && potentialYHitList[i] && potentialZHitList[i] )
			{
			//see if face intersects sphere
			//check verts then face
			int a,b,c;
			Point3 pa,pb,pc;
			nodeList[0]->faces[i];
			a = nodeList[0]->faces[i].v[0];
			b = nodeList[0]->faces[i].v[1];
			c = nodeList[0]->faces[i].v[2];
			pa = nodeList[0]->verts[a];
			pb = nodeList[0]->verts[b];
			pc = nodeList[0]->verts[c];
			HitList temp;
			temp.faceIndex = i;
			temp.distance = -1.0f;
			float lastls=-1.0f;
			if (bbox.Contains(pa)) 
				{
				lastls = 0.0f;
				}
			if (bbox.Contains(pb)) 
				{
				lastls = 0.0f;
				}
			if (bbox.Contains(pc)) 
				{
				lastls = 0.0f;
				}

			
			if (lastls != -1.0f) hitList.Append(1,&temp,100);


			
			}
		}


	return hitList.Count();

}

/*
Grid::RecurseLine(IPoint3 pStart, IPoint3 pEnd, int l1, int l2, BitArray &potentialHitList)
{
	
	IPoint3 mid = IPoint3(0,0,0);
	mid[l1] = (pStart[l1] + pEnd[l1])/2;
	mid[l2] = (pStart[l2] + pEnd[l2])/2;
	//add this point
	int x,y;
	x = mid[l1];
	y = mid[l2];

	int yLine = y * width;
	int index = yLine +x;

	CellList *temp = NULL;
	if ((l1 == 0) && (l1 ==1))
		temp = gridX[index];
	else if ((l1 == 1) && (l1 ==2))
		temp = gridY[index];
	else if ((l1 == 0) && (l1 ==2) )
		temp = gridZ[index];

	if (temp)
		{
		for (int i = 0; i < temp->data.Count(); i++)
			{
			int findex = temp->data[i].faceIndex;
			potentialHitList.Set(findex);
			}
		}
	
	int p[8];
	p[0] = yLine - width - 1;
	p[1] = yLine - width;
	p[2] = yLine - width + 1;

	p[3] = yLine - 1;
	p[4] = yLine + 1;
	
	p[5] = yLine + width - 1;
	p[6] = yLine + width;
	p[7] = yLine + width + 1;
	int ct = 0;
	for (int i =0; i < 8; i++)
		{
		if ((p[i] >= 0) && (p[i] < (width*width)))
			{
			if (potentialHitList[p[i]]) ct++; 
			}
		}
	//check if it has 2 neighbors if so end this recursion
	if (ct >= 2)
		{
		}
	//else continue
	else
		{
		RecurseLine(pStart, mid, l1, l2, potentialHitList);
		RecurseLine(mid, pEnd, l1, l2, potentialHitList);
		}

}

*/
inline float FindY(float m, float b,float x)
{
return m*x+b;
}

inline float FindX(float im, float b,float y)
{
return (y -b)*im;
}

int Grid::WalkLine(Point3 start, Point3 end, int l1, int l2, BitArray &potentialHitList,Tab<CellList*> &grid)

{
	//get point in grid space

	float fwidth = bounds.pmax.x - bounds.pmin.x;
	float fheight = bounds.pmax.y - bounds.pmin.y;
	float fdepth = bounds.pmax.z - bounds.pmin.z;
//compute our grid start x,y,z
	start = start - bounds.pmin;
	end = end - bounds.pmin;

	IPoint3 iStart, iEnd;

	float wInc, hInc,dInc;
	wInc = fwidth/width;
	hInc = fheight/width;
	dInc = fdepth/width;

	iStart.x = start.x/wInc;
	iStart.y = start.y/hInc;
	iStart.z = start.z/dInc;

	iEnd.x = end.x/wInc;
	iEnd.y = end.y/hInc;
	iEnd.z = end.z/dInc;

//DebugPrint("walkline %d %d %d to %d %d %d \n",iStart.x,iStart.y,iStart.z,
//		   iEnd.x,iEnd.y,iEnd.z
//		   );



//	iStart.x = (int)(float(width-1) * start.x/fwidth);
//	iStart.y = (int)(float(width-1) * start.y/fheight);
//	iStart.z = (int)(float(width-1) * start.z/fdepth);

//compute our grid end x,y,z
//	iEnd.x = (int)(float(width-1) * end.x/fwidth);
//	iEnd.y = (int)(float(width-1) * end.y/fheight);
//	iEnd.z = (int)(float(width-1) * end.z/fdepth);

	float w,h;
	w = bounds.pmax[l1]-bounds.pmin[l1];
	h = bounds.pmax[l2]-bounds.pmin[l2];

	float boundMaxX, boundMinX;
	float boundMaxY, boundMinY;
	boundMaxX = w;
	boundMinX = 0;

	boundMaxY = h;
	boundMinY = 0;

	int iStartX,iStartY,iEndX,iEndY;
	iStartX = iStart[l1];
	iEndX = iEnd[l1];

	iStartY = iStart[l2];
	iEndY = iEnd[l2];


	

	//compute rise
	float startX, startY, endX, endY;
	startX = start[l1];
	startY = start[l2];

	endX = end[l1];
	endY = end[l2];

	if (iEndX < iStartX)
		{
		int temp = iStartX;
		iStartX = iEndX;
		iEndX = temp;
		float ftemp = startX;
		startX = endX;
		startX = ftemp;
		}
	if (iEndY < iStartY)
		{
		int temp = iStartY;
		iStartY = iEndY;
		iEndY = temp;
		float ftemp = startY;
		startY = endY;
		startY = ftemp;
		}

	float rise = endY - startY;
	//computte run
	float run = endX - startX;

	//get line equation y = mx + b
	// x = (y-b)/m
	float slope, b;


	if (run == 0) //vertical line
		{
		if (iStartY < 0) iStartY = 0;
		if (iEndY >= width) iEndY = width-1;
		for (int i = iStartY; i <= iEndY; i++)
			{
			int index = i * width + iStartX;
			if ((index < width*width) && (index >= 0))
				{
				CellList *temp = grid[index];
				if (temp)
					{
					for (int i = 0; i < temp->data.Count(); i++)
						{
						int findex = temp->data[i].faceIndex;
						potentialHitList.Set(findex);
						}
					}
				}
			}
		return 1;
		}
	else if (rise == 0)//horizontal line
		{
		if (iStartX < 0) iStartX = 0;
		if (iEndX >= width) iEndX = width-1;
		if ((iStartY < 0) || (iStartY >= width)) return 0;
		for (int i = iStartX; i <= iEndX; i++)
			{
			int index = iStartY * width + i;
			if ((index < width*width) && (index >= 0))
				{
				CellList *temp = grid[index];
				if (temp)
					{
					for (int i = 0; i < temp->data.Count(); i++)
						{
						int findex = temp->data[i].faceIndex;
						potentialHitList.Set(findex);
						}
					}
				}
			}
		return 1;
		}

	//see what is the major run direction
	//get that major axis distance

	int xDist = iEndX - iStartX;
	int yDist = iEndY - iStartY;


	if (iStartX < 0) iStartX = 0;
	if (iEndX >= width) iEndX = width-1;

	if (iStartY < 0) iStartY = 0;
	if (iEndY >= width) iEndY = width-1;

	if (iStartX >= width) return 0;
	if (iEndX < 0) return 0;

	if (iStartY >= width) return 0;
	if (iEndY < 0) return 0;

	float xInc = w/(width),yInc = h/(width);

	rise = endY - startY;
	//computte run
	run = endX - startX;

	slope = rise/run;
	float islope = 1.0f/slope;
	b = startY - slope*startX;
	

//	if (xDist > yDist)
		{
		float cx,cy;//cy is computed
		cx = iStartX * xInc;

		for (int i = iStartX; i <= iEndX; i++)
			{
			cy = FindY(slope, b, cx);
			int j = cy/yInc;

			if ((j < 0) || (j >= width))
				{
//				i = iEndX;
				}
			else
				{
				int index = j * width+i;
				CellList *temp = grid[index];
				if (temp)
					{
					for (int k = 0; k < temp->data.Count(); k++)
						{
						int findex = temp->data[k].faceIndex;
						potentialHitList.Set(findex);
						}
					}
				}
			cx += xInc;
			}
		}
//	else
		{

		float cx,cy;//cx is computed
		cy = iStartY * yInc;

		for (int i = iStartY; i <= iEndY; i++)
			{
			cx = FindX(islope, b, cy);
			int j = cx/xInc;
			if ((j < 0) || (j >= width))
				{
//				i = iEndY;
				}
			else
				{
				int index = i * width+j;
				CellList *temp = grid[index];
				if (temp)
					{
					for (int k = 0; k < temp->data.Count(); k++)
						{
						int findex = temp->data[k].faceIndex;
						potentialHitList.Set(findex);
						}
					}

				}

			cy += yInc;
			}

		}

/*	

	float dirX,dirY;
	dirX = end[l1]-start[l1];
	dirY = end[l2]-start[l2];
	int quad =0;
	float xInc = w/(width),yInc = h/(width);
	if ((dirX > 0) && (dirY > 0))
		{
		quad = 0;
		}
	else if ((dirX < 0) && (dirY > 0))
		{
		quad = 1;
		xInc *= -1;
		}	
	else if ((dirX < 0) && (dirY < 0))
		{
		quad = 2;
		xInc *= -1;
		yInc *= -1;
		}
	else if ((dirX > 0) && (dirY < 0))
		{
		quad = 3;
		yInc *= -1;
		}
	slope = rise/run;
	float islope = 1.0f/slope;
	b = startY - slope*startX;
	
	float gridX=0.0f,gridY=0.0f;

	

	//check to make sure the start point is in our our grid if not find out where it intersects
	if (quad == 0) //going up and right
		{
		if ( (startX > fwidth) || (endX < 0.0f) ||
			 (startY > fheight) || (endY < 0.0f) 
			)
			return 0;
		if (startX < 0.0f)
			gridX = 0.0f;
		else
			{
			gridX = (iStartX +1) * fabs(xInc);
			}
		if (startY < 0.0f)
			gridY = 0.0f;
		else
			{
			gridY = (iStartY +1) * fabs(yInc);
			}
		}
	else if (quad == 1)
		{
		if ( (startX < 0.0f ) || (endX > fwidth) ||
			 (startY > fheight) || (endY < 0.0f) 
			)
			return 0;
		if (startX > (fwidth-fabs(xInc)))
			gridX = (fwidth-fabs(xInc));
		else
			{
			gridX = (iStartX +1) * fabs(xInc);
			}
		if (startY < 0.0f)
			gridY = 0.0f;
		else
			{
			gridY = (iStartY +1) * fabs(yInc);
			}
		}
	else if (quad == 2)
		{
		if ( (startX < 0.0f ) || (endX > fwidth) ||
			 (startY < 0.0f) || (endY > fheight) 
			)
			return 0;
		if (startX > (fwidth-fabs(xInc)))
			gridX = (fwidth-fabs(xInc));
		else
			{
			gridX = (iStartX +1) * fabs(xInc);
			}

		if (startY < (fheight-fabs(yInc)))
			gridY = fheight-fabs(yInc);
		else
			{
			gridY = (iStartY +1) * fabs(yInc);
			}
		}
	else if (quad == 3)
		{

		if ( (startX > fwidth) || (endX < 0.0f) ||
			 (startY < 0.0f) || (endY > fheight) 
			)
			return 0;

		if (startX < 0.0f)
			gridX = 0.0f;
		else
			{
			gridX = (iStartX +1) * fabs(xInc);
			}

		if (startY < (fheight-fabs(yInc)))
			gridY = fheight-fabs(yInc);
		else
			{
			gridY = (iStartY +1) * fabs(yInc);
			}
		}


	BOOL done = FALSE;
	while (!done)
		{
//see what grid we intersect
		float newY, newX;
		newX = FindX(islope, b,gridY);
		int x,y;

		if ((quad == 0) || (quad == 3))
			{
			if (newX > gridX) //we hit the y grid
				{
				//add the hit cell
				//use old x
				//use hit y
				newY = FindY(slope, b,gridX);
				x = gridX/fabs(xInc);
				y = newY/fabs(yInc);
				//get the index and add our cells
				int index = y * width + x;
				CellList *temp = grid[index];
				if (temp)
					{
					for (int i = 0; i < temp->data.Count(); i++)
						{
						int findex = temp->data[i].faceIndex;
						potentialHitList.Set(findex);
						}
					}
				gridY = y*fabs(yInc);
				}
			else //we hit the x grid
				{
				//add the hit cell
				//use old x
				//use hit y
				x = newX/fabs(xInc);
				y = gridY/fabs(yInc);
				//get the index and add our cells
				int index = y * width + x;
				CellList *temp = grid[index];
				if (temp)
					{
					for (int i = 0; i < temp->data.Count(); i++)
						{
						int findex = temp->data[i].faceIndex;
						potentialHitList.Set(findex);
						}
					}
				gridX = x*fabs(xInc);
				}
			}
		else if ((quad == 1) || (quad == 2))
			{
			if (newX > gridX) //we hit the y grid
				{
				//add the hit cell
				//use old x
				//use hit y
				newY = FindY(slope, b,gridX);
				x = gridX/fabs(xInc);
				y = newY/fabs(yInc);
				//get the index and add our cells
				int index = y * width + x;
				CellList *temp = grid[index];
				if (temp)
					{
					for (int i = 0; i < temp->data.Count(); i++)
						{
						int findex = temp->data[i].faceIndex;
						potentialHitList.Set(findex);
						}
					}
				gridY = y*fabs(yInc);
				}
			else //we hit the x grid
				{
				//add the hit cell
				//use old x
				//use hit y
				x = newX/fabs(xInc);
				y = gridY/fabs(yInc);
				//get the index and add our cells
				int index = y * width + x;
				CellList *temp = grid[index];
				if (temp)
					{
					for (int i = 0; i < temp->data.Count(); i++)
						{
						int findex = temp->data[i].faceIndex;
						potentialHitList.Set(findex);
						}
					}
				gridX = x * fabs(xInc);
				}
			}

		if ((x < 0) || (y < 0) || (x >= width) || (y >= width))
			done = TRUE;
		if ( (x == endX) && (y==endY))
			done = TRUE;

	//check if our grid X or Y are out of bounds or we hit the end cell

		}
*/
	return 1;


	//check to make sure the point is in our our grid if not find out where it intersects
	//if no intersects we are done
	//start walking line
	//look at neighbors

}

int	Grid::IntersectRay(Point3 p, Point3 dir, BOOL segment, BOOL doubleSided, int whichGrid)
{

	BitArray xHitList,yHitList,zHitList;
	BitArray potentialXHitList,potentialYHitList,potentialZHitList;
	Tab<int> xHits;
	Tab<int> yHits;
	Tab<int> zHits;

	xHitList.SetSize(width*width);
	yHitList.SetSize(width*width);
	zHitList.SetSize(width*width);

	xHitList.ClearAll();
	yHitList.ClearAll();
	zHitList.ClearAll();

	potentialXHitList.SetSize(nodeList[0]->faces.Count());
	potentialYHitList.SetSize(nodeList[0]->faces.Count());
	potentialZHitList.SetSize(nodeList[0]->faces.Count());

	potentialXHitList.ClearAll();
	potentialYHitList.ClearAll();
	potentialZHitList.ClearAll();

//get bounds
	Point3 min, max;
	Box3 bbox;
	bbox.Init();
	bbox += p;
	Point3 endP;
	float maxDistance = 0.0f;
	if (segment)
		{
		endP = p + dir;
		maxDistance = Length(p-endP);
		}
	else endP = p + dir*(100000.0f);


	int largestAxis = 0;
	int largestDir = 0;
	float xDir = fabs(endP.x-p.x);
	largestDir = xDir;
	float yDir = fabs(endP.y-p.y);
	float zDir = fabs(endP.z-p.z);

	whichGrid = 0;
	if (yDir > largestDir)
		{
		largestAxis = 1;
		largestDir = yDir;

		}
	if (zDir > largestDir)
		largestAxis = 2;

	if (largestAxis == 0)
		WalkLine(p, endP, 0, 1, potentialXHitList,gridX);
	else if (largestAxis == 1)
		WalkLine(p, endP, 2, 1, potentialXHitList,gridY);
	else
		WalkLine(p, endP, 2, 0, potentialXHitList,gridZ);

//	potentialXHitList = potentialZHitList;
//	potentialYHitList = potentialZHitList;



//	bbox += endP;

//	min = bbox.pmin;
//	max = bbox.pmax;

//compute our grid start x,y,z
//	IPoint3 pStart, pEnd;

//	ComputeStartPoints(min, max, pStart,pEnd);


	//do X grid
	//set start
/*
	int index = pStart.y * width + pStart.x;
	if (gridX[index])
		{
		
		CellList *temp = gridX[index];
		for (int i = 0; i < temp->data.Count(); i++)
			{
			int findex = temp->data[i].faceIndex;
			potentialXHitList.Set(findex);
			}
		}
	//set end
	index = pEnd.y * width + pEnd.x;
	if (gridX[index])
		{
		CellList *temp = gridX[index];
		for (int i = 0; i < temp->data.Count(); i++)
			{
			int findex = temp->data[i].faceIndex;
			potentialXHitList.Set(findex);
			}
		}
//	RecurseLine(pStart, pEnd,0,1, potentialXHitList);

	

	//do Y grid
	//set start
	index = pStart.y * width + pStart.z;
	if (gridY[index])
		{
		
		CellList *temp = gridY[index];
		for (int i = 0; i < temp->data.Count(); i++)
			{
			int findex = temp->data[i].faceIndex;
			potentialYHitList.Set(findex);
			}
		}
	//set end
	index = pEnd.y * width + pEnd.z;
	if (gridY[index])
		{
		CellList *temp = gridY[index];
		for (int i = 0; i < temp->data.Count(); i++)
			{
			int findex = temp->data[i].faceIndex;
			potentialYHitList.Set(findex);
			}
		}
//	RecurseLine(pStart, pEnd,1,2, potentialYHitList);


	//do Z grid
	//set start
	index = pStart.x * width + pStart.z;
	if (gridZ[index])
		{
		
		CellList *temp = gridZ[index];
		for (int i = 0; i < temp->data.Count(); i++)
			{
			int findex = temp->data[i].faceIndex;
			potentialZHitList.Set(findex);
			}
		}
	//set end
	index = pEnd.x * width + pEnd.z;
	if (gridZ[index])
		{
		CellList *temp = gridZ[index];
		for (int i = 0; i < temp->data.Count(); i++)
			{
			int findex = temp->data[i].faceIndex;
			potentialZHitList.Set(findex);
			}
		}
//	RecurseLine(pStart, pEnd,0,2, potentialZHitList);
*/

//now lets check for actual face hits from our potential hit list

	int xhitct = potentialXHitList.NumberSet();
	int yhitct = potentialYHitList.NumberSet();
	int zhitct = potentialZHitList.NumberSet();
	hitList.ZeroCount();

	int ct = nodeList[0]->faces.Count();
	hitList.ZeroCount();
	Point3 nRay = Normalize(dir);

	summedFaces += potentialXHitList.NumberSet();
	tally += 1.0f;


	for (int i = 0; i < ct; i++)
		{
		BOOL phit = FALSE;
		if (whichGrid == -1)
			phit = potentialXHitList[i] && potentialYHitList[i] && potentialZHitList[i] ;
		else if (whichGrid == 0)
		 	phit = potentialXHitList[i];
		else if (whichGrid == 1)
			phit = potentialYHitList[i];
		else if (whichGrid == 2)
			phit = potentialZHitList[i];


		if (phit)
//		if (potentialXHitList[i])
			{
			Point3  n = nodeList[0]->fnorms[i];
			float rn = DotProd(nRay,n);

			if (!doubleSided) //check normal first and toss if not facing
				{
				if (rn < 0.0f) //it facing away skip this face
					continue;
				}

			//now look see if ray intersects face

			BOOL hit = FALSE;
			//intersect ray with the plane
			// See if the ray intersects the plane (backfaced)
//check if intersects triangle
			
		
		// Use a point on the plane to find d
			int ia,ib,ic;
			ia = nodeList[0]->faces[i].v[0];
			ib = nodeList[0]->faces[i].v[1];
			ic = nodeList[0]->faces[i].v[2];

			Point3 pa,pb,pc;
			pa = nodeList[0]->verts[ia];
			pb = nodeList[0]->verts[ib];
			pc = nodeList[0]->verts[ic];

			float d = DotProd(pa,n);

		// Find the point on the ray that intersects the plane
			float a = (d - DotProd(p,n)) / rn;
			if (a<0.0f) 
				continue;
			if ((segment) && (a > maxDistance))
			 	continue;



		// The point on the ray and in the plane.
			Point3 hp = p + a*nRay;

		// Compute barycentric coords.
			Point3 bry = BaryCoords(pa,pb,pc,hp);

		// barycentric coordinates must sum to 1 and each component must
		// be in the range 0-1
			if (bry.x<0.0f || bry.y<0.0f || bry.z<-0.0f ) continue;   // DS 3/8/97 this test is sufficient

//			if (bry.x>=0.0f && bry.x<=1.0f && 
//				bry.y>=0.0f && bry.y<=1.0f &&
//				bry.z>=0.0f && bry.z<=1.0f)
				{
//add to hit list
				HitList temp;
				temp.distance = a;
				temp.faceIndex = i;
				temp.nodeIndex = 0;
				temp.bary = bry;
				temp.fnorm = n;
				hitList.Append(1,&temp,100);
				}
			}
		}




	return hitList.Count();

}


int Grid::GetHitFace(int index)

{
//check hit bounds
	if (hitList.Count() == 0) return -1;
	if ( index < 0) return -1;
	if ( index >= hitList.Count()) return -1;
	
	return hitList[index].faceIndex;

}

Point3	Grid::GetHitBary(int index)
{
//check hit bounds
	if (hitList.Count() == 0) return Point3(0.0f,0.0f,0.0f);
	if ( index < 0) return Point3(0.0f,0.0f,0.0f);
	if ( index >= hitList.Count()) return Point3(0.0f,0.0f,0.0f);
	
	return hitList[index].bary;
}

Point3	Grid::GetHitNorm(int index)
{
//check hit bounds
	if (hitList.Count() == 0) return Point3(0.0f,0.0f,0.0f);
	if ( index < 0) return Point3(0.0f,0.0f,0.0f);
	if ( index >= hitList.Count()) return Point3(0.0f,0.0f,0.0f);
	
	return hitList[index].fnorm;
}
float	Grid::GetHitDist(int index)
{
//check hit bounds
	if (hitList.Count() == 0) return -1.0f;
	if ( index < 0) return -1.0f;
	if ( index >= hitList.Count()) return -1.0f;
	
	return hitList[index].distance;

}


float	Grid::GetPerpDist(int index)
{
//check hit bounds
	if (hitList.Count() == 0) return -1.0f;
	if ( index < 0) return -1.0f;
	if ( index >= hitList.Count()) return -1.0f;
	
	return hitList[index].perpDistance;

}


int		Grid::GetClosestHit()
{
	int hitIndex = -1;
	float closest = 1.0f;

	for (int i = 0; i < hitList.Count(); i++)
		{
		if ((hitIndex == -1) || (hitList[i].distance < closest))
			{
			hitIndex = i;
			closest = hitList[i].distance;
			}
		}

	return hitIndex;

}


Point3	Grid::GetHitPoint(int index)
{
//check hit bounds
	if (hitList.Count() == 0) return Point3(0.0f,0.0f,0.0f);
	if ( index < 0) return Point3(0.0f,0.0f,0.0f);
	if ( index >= hitList.Count()) return Point3(0.0f,0.0f,0.0f);
	
	Point3 bry = hitList[index].bary;
	int fid = hitList[index].faceIndex;

	int a,b,c;
	a = nodeList[0]->faces[fid].v[0];
	b = nodeList[0]->faces[fid].v[1];
	c = nodeList[0]->faces[fid].v[2];

	Point3 pa, pb, pc;
	pa = nodeList[0]->verts[a];
	pb = nodeList[0]->verts[b];
	pc = nodeList[0]->verts[c];
	Point3 pt(0.0f,0.0f,0.0f);

	pt.x = pa.x * bry.x + pb.x * bry.y + pc.x * bry.z; 
	pt.y = pa.y * bry.x + pb.y * bry.y + pc.y * bry.z; 
	pt.z = pa.z * bry.x + pb.z * bry.y + pc.z * bry.z; 

	return pt;

}



int		Grid::GetFarthestHit()
{
	int hitIndex = -1;
	float closest = 1.0f;

	for (int i = 0; i < hitList.Count(); i++)
		{
		if ((hitIndex == -1) || (hitList[i].distance > closest))
			{
			hitIndex = i;
			closest = hitList[i].distance;
			}
		}

	return hitIndex;

}

void	Grid::GetCell(Point3 p, int l1, int l2, int &x, int &y)
{
	//get point in grid space
	float fwidth = bounds.pmax.x - bounds.pmin.x;
	float fheight = bounds.pmax.y - bounds.pmin.y;
	float fdepth = bounds.pmax.z - bounds.pmin.z;
//compute our grid start x,y,z
	p = p - bounds.pmin;


	IPoint3 iStart;

	float wInc, hInc,dInc;
	wInc = fwidth/width;
	hInc = fheight/width;
	dInc = fdepth/width;

	iStart.x = p.x/wInc;
	iStart.y = p.y/hInc;
	iStart.z = p.z/dInc;


	float w,h;
	w = bounds.pmax[l1]-bounds.pmin[l1];
	h = bounds.pmax[l2]-bounds.pmin[l2];

	float boundMaxX, boundMinX;
	float boundMaxY, boundMinY;
	boundMaxX = w;
	boundMinX = 0;

	boundMaxY = h;
	boundMinY = 0;

	x = iStart[l1];

	y = iStart[l2];

}


float LineToPoint( Point3 l1, Point3 l2, Point3 p1)
{
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
		}
		else
		{
		double hyp;
		hyp = Length(VectorB);
		dist =  sin(Angle) * hyp;

		}

	}

return (float) dist;

}


int Grid::GetFacesToProcess(int x,int y, int radius,BitArray &cellsToCheck, Tab<CellList*> &grid)
{

	int startX = x - radius;
	int startY = y - radius;
	int endX = x + radius;
	int endY = y + radius;

	int ct = 0;

	if (startX < 0) 
		{
		startX = 0;
		ct++;
		}	
	if (startY < 0) 
		{
		startY = 0;
		ct++;
		}

	if (endX < 0) 
		{
		endX = 0;
		}	
	if (endY < 0) 
		{
		endY = 0;
		}

	if (endX >= width) 
		{
		endX = width-1;
		ct++;
		}	

	if (endY >= width) 
		{
		endY = width-1;
		ct++;
		}	

	if (startX >= width) 
		{
		startX = width-1;
		}	

	if (startY >= width) 
		{
		startY = width-1;
		}	


	if (radius == 1)
		{
		if ((x >=0) && (x<width) && (y>=0) && (y<width))
			{
		 	int index = y * width +  x;

			if (index < (width*width))
				{
				CellList *temp = grid[index];
				if (temp)
					{
					for (int i = 0; i < temp->data.Count(); i++)
						{
						int findex = temp->data[i].faceIndex;
						cellsToCheck.Set(findex);
						}
					}
				}
			}
		}

	
	for (int ix = startX; ix <= endX; ix++)
		{
		int index = startY * width + ix;

		if (index < (width*width))
			{
			CellList *temp = grid[index];
			if (temp)
				{
				for (int i = 0; i < temp->data.Count(); i++)
					{
					int findex = temp->data[i].faceIndex;
					cellsToCheck.Set(findex);
					}
				}
			}

		index = endY * width + ix;

		if (index < (width*width))
			{
			CellList *temp = grid[index];
			if (temp)
				{
				for (int i = 0; i < temp->data.Count(); i++)
					{
					int findex = temp->data[i].faceIndex;
					cellsToCheck.Set(findex);
					}
				}
			}
		}


	for (int iy = (startY+1); iy <= (endY-1); iy++)
		{
		int index = iy * width + startX;

		if (index < (width*width))
			{
			CellList *temp = grid[index];
			if (temp)
				{
				for (int i = 0; i < temp->data.Count(); i++)
					{
					int findex = temp->data[i].faceIndex;
					cellsToCheck.Set(findex);
					}
				}
			}

		index = iy * width + endX;

		if (index < (width*width))
			{
			CellList *temp = grid[index];
			if (temp)
				{
				for (int i = 0; i < temp->data.Count(); i++)
					{
					int findex = temp->data[i].faceIndex;
					cellsToCheck.Set(findex);
					}
				}
			}



		}

	if (ct== 4) return 1;
	else return 0;
}

int		Grid::ClosestFace(Point3 p)
{
//find the 3 grid entry containing the point
	int xX,yX;
	GetCell(p, 0, 1, xX, yX);

	int xY,yY;
	GetCell(p, 2, 1, xY, yY);

	int xZ,yZ;
	GetCell(p, 2, 0, xZ, yZ);
	
	int radius = 1;

	//set a process list
	BitArray processedFaces;
	processedFaces.SetSize(nodeList[0]->faces.Count());
	processedFaces.ClearAll();
	//clear it



	BOOL done = FALSE;
	hitList.ZeroCount();


	//take our threshold divide into the smallest cell add one
	
	//that is our radius
//start with a 1 radius

	radius = 1;


	tally += 1.0f;

	while (!done)
		{
//find get the one with the fewest entries
//expand it by one
		BitArray cellsToCheckX,cellsToCheckY,cellsToCheckZ;
		cellsToCheckX.SetSize(nodeList[0]->faces.Count());
		cellsToCheckX.ClearAll();

		cellsToCheckY.SetSize(nodeList[0]->faces.Count());
		cellsToCheckY.ClearAll();

		cellsToCheckZ.SetSize(nodeList[0]->faces.Count());
		cellsToCheckZ.ClearAll();

		int x,y;
		x = xX;
		y = yX;


		int ct = 0;
		ct += GetFacesToProcess(xX,yX,radius,cellsToCheckX,gridX);
		ct += GetFacesToProcess(xY,yY,radius,cellsToCheckY,gridY);
		ct += GetFacesToProcess(xZ,yZ,radius,cellsToCheckZ,gridZ);



		if (ct == 3 ) 
			done = TRUE;


		

/*DebugPrint("total %d\n",cellsToCheck.NumberSet());
for (int k = 0; k < cellsToCheck.GetSize(); k++)
	{
	if (cellsToCheck[k]) DebugPrint("face %d\n",k);
	}
*/

		int tallyCount = 0;
		for (int i = 0; i < cellsToCheckX.GetSize(); i++)
			{
			if ((!processedFaces[i]) && (cellsToCheckX[i]&&cellsToCheckX[i]&&cellsToCheckZ[i]))
				{
				processedFaces.Set(i);
				tallyCount++;
				//get the normal of that face
				Point3 n = nodeList[0]->fnorms[i]*-1.0f;

				float rn = 1.0f;// DotProd(n,n);

				//that is the ray we want to check
				//see if we hit it is so mark and go on
			// Use a point on the plane to find d
				int ia,ib,ic;
				ia = nodeList[0]->faces[i].v[0];
				ib = nodeList[0]->faces[i].v[1];
				ic = nodeList[0]->faces[i].v[2];

				Point3 pa,pb,pc;
				pa = nodeList[0]->verts[ia];
				pb = nodeList[0]->verts[ib];
				pc = nodeList[0]->verts[ic];

				float d = DotProd(pa,n);

			// Find the point on the ray that intersects the plane
				float a = (d - DotProd(p,n)) / rn;
//				if (a<0.0f) 
//					continue;



				// The point on the ray and in the plane.
				Point3 hp = p + a*n;

				// Compute barycentric coords.
				Point3 bry = BaryCoords(pa,pb,pc,hp);

				// barycentric coordinates must sum to 1 and each component must
				// be in the range 0-1
				if (bry.x<0.0f || bry.y<0.0f || bry.z<0.0f )
					{
					float d1 = LineToPoint( pa, pb, p); //Dist3DPtToLine(&pa,&pb,&p);
					float d2 = LineToPoint( pb, pc, p); //Dist3DPtToLine(&pb,&pc,&p);
					float d3 = LineToPoint( pc, pa, p); //Dist3DPtToLine(&pc,&pa,&p);
//find distance to the edge
					if (d2 < d1) d1 = d2;
					if (d3 < d1) d1 = d3;
		//add to hit list
					HitList temp;
					temp.distance = fabs(d1);
					temp.perpDistance = a;
					temp.faceIndex = i;
					temp.nodeIndex = 0;
					temp.bary = bry;
					temp.fnorm = n;
					hitList.Append(1,&temp,100);
					done = TRUE;

					}
				else
					{

		//add to hit list
					HitList temp;
					temp.distance = fabs(a);
					temp.perpDistance = a;
					temp.faceIndex = i;
					temp.nodeIndex = 0;
					temp.bary = bry;
					temp.fnorm = n;
					hitList.Append(1,&temp,100);
					done = TRUE;
					}

				}
			}
		
		radius++;

		}

	summedFaces += processedFaces.NumberSet();

	//add our hit face,normal bary etc to our list


	return hitList.Count();
}

/*
int		Grid::ClosestFace(Point3 p,float threshold)
{
//find the 3 grid entry containing the point
	int xX,yX;
	GetCell(p, 0, 1, xX, yX);
	int index = yX * width + xX;
	BitArray potentialXHitList;
	potentialXHitList.SetSize(nodeList[0]->faces.Count());
	potentialXHitList.ClearAll();
	if ((index < width*width) && (index >= 0))
		{
		CellList *temp = gridX[index];
		if (temp)
			{
			for (int i = 0; i < temp->data.Count(); i++)
				{
				int findex = temp->data[i].faceIndex;
				potentialXHitList.Set(findex);
				}
			}
		}

	int xY,yY;
	GetCell(p, 2, 1, xY, yY);
	index = yY * width + xY;
	BitArray potentialYHitList;
	potentialYHitList.SetSize(nodeList[0]->faces.Count());
	potentialYHitList.ClearAll();
	if ((index < width*width) && (index >= 0))
		{
		CellList *temp = gridY[index];
		if (temp)
			{
			for (int i = 0; i < temp->data.Count(); i++)
				{
				int findex = temp->data[i].faceIndex;
				potentialYHitList.Set(findex);
				}
			}
		}


	int xZ,yZ;
	GetCell(p, 2, 0, xZ, yZ);
	index = yZ * width + xZ;
	BitArray potentialZHitList;
	potentialZHitList.SetSize(nodeList[0]->faces.Count());
	potentialZHitList.ClearAll();
	if ((index < width*width) && (index >= 0))
		{
		CellList *temp = gridZ[index];
		if (temp)
			{
			for (int i = 0; i < temp->data.Count(); i++)
				{
				int findex = temp->data[i].faceIndex;
				potentialZHitList.Set(findex);
				}
			}
		}

	int x,y;
	x = xX;
	y = yX;
	Tab<CellList*> grid;
	grid = gridX;


	if (potentialYHitList.NumberSet() < potentialXHitList.NumberSet())
		{
		x = xY;
		y = yY;
		potentialXHitList = potentialYHitList;
		grid = gridY;
		}
	if (potentialZHitList.NumberSet() < potentialXHitList.NumberSet())
		{
		x = xZ;
		y = yZ;
		potentialXHitList = potentialZHitList;
		grid = gridZ;
		}

//DebugPrint("x %d y %d z %d\n",potentialXHitList.NumberSet(),potentialYHitList.NumberSet(),potentialZHitList.NumberSet());


	Tab<int> cellsToCheck;
	
	int radius = 1;

	//set a process list
	BitArray processedFaces;
	processedFaces.SetSize(nodeList[0]->faces.Count());
	processedFaces.ClearAll();
	//clear it



	BOOL done = FALSE;
	hitList.ZeroCount();


	//take our threshold divide into the smallest cell add one
	
	//that is our radius
//compute a radius
	//find our x,y,z cell width
	float fwidth = (bounds.pmax.x - bounds.pmin.x)/width;
	float fheight = (bounds.pmax.y - bounds.pmin.y)/width;
	float fdepth = (bounds.pmax.z - bounds.pmin.z)/width;

	float cellWidth = fheight;
	if (fheight < cellWidth) cellWidth = fheight;
	if (fdepth < cellWidth) cellWidth = fdepth;

	radius = threshold/cellWidth + 2;

//	radius = 1.0f;

//	radius = 1;
//	while (!done)
		{
//find get the one with the fewest entries
//expand it by one
		BitArray cellsToCheck;
		cellsToCheck.SetSize(nodeList[0]->faces.Count());
		cellsToCheck.ClearAll();

		//get center 5f raduis == 1 add it
		if (radius == 1)
			{
			int index = y*width+x;
			if (index < (width*width))
				{
				CellList *temp = grid[index];
				if (temp)
					{
					for (int i = 0; i < temp->data.Count(); i++)
						{
						int findex = temp->data[i].faceIndex;
						cellsToCheck.Set(findex);
						}
					}
				}

			}
		else
			{
			//do top and lower edge
			int startX, endX;
			int startY, endY;
			startX = x - radius;
			endX = x + radius;
			startY = y - radius;
			endY = y + radius;
			int ct = 0;
			if (startX < 0)
				{
				startX = 0;
				ct++;
				}
			if (endX >= width) 
				{
				endX = width-1;
				ct++;
				}
			if (startY < 0) 
				{
				startY = 0;
				ct++;
				}
			if (endY >= width ) 
				{
				endY = width-1;
				ct++;
				}
//			if (ct == 4 ) done = TRUE;
			//do left and right edge;
			for (int cy = (startY); cy <= (endY-1); cy++)
				{

				for (int cx = startX; cx <= endX; cx++)
					{
					int index = cy * width + cx;
					if (index < (width*width))
						{
						CellList *temp = grid[index];
						if (temp)
							{
							for (int i = 0; i < temp->data.Count(); i++)
								{
								int findex = temp->data[i].faceIndex;
								cellsToCheck.Set(findex);
								}
							}
						}

					}
				}
			}

		summedFaces += cellsToCheck.NumberSet();
		tally += 1.0f;


		for (int i = 0; i < cellsToCheck.GetSize(); i++)
			{
//if (i==105) 
//DebugPrint("stop\n");
			if (cellsToCheck[i])
				{
				//get the normal of that face
				Point3 n = nodeList[0]->fnorms[i]*-1.0f;

				float rn = 1.0f;// DotProd(n,n);

				//that is the ray we want to check
				//see if we hit it is so mark and go on
			// Use a point on the plane to find d
				int ia,ib,ic;
				ia = nodeList[0]->faces[i].v[0];
				ib = nodeList[0]->faces[i].v[1];
				ic = nodeList[0]->faces[i].v[2];

				Point3 pa,pb,pc;
				pa = nodeList[0]->verts[ia];
				pb = nodeList[0]->verts[ib];
				pc = nodeList[0]->verts[ic];

				float d = DotProd(pa,n);

			// Find the point on the ray that intersects the plane
				float a = (d - DotProd(p,n)) / rn;
//				if (a<0.0f) 
//					continue;



				// The point on the ray and in the plane.
				Point3 hp = p + a*n;

				// Compute barycentric coords.
				Point3 bry = BaryCoords(pa,pb,pc,hp);

				// barycentric coordinates must sum to 1 and each component must
				// be in the range 0-1
				if (bry.x<0.0f || bry.y<0.0f || bry.z<0.0f )
					{
					float d1 = LineToPoint( pa, pb, p); //Dist3DPtToLine(&pa,&pb,&p);
					float d2 = LineToPoint( pb, pc, p); //Dist3DPtToLine(&pb,&pc,&p);
					float d3 = LineToPoint( pc, pa, p); //Dist3DPtToLine(&pc,&pa,&p);
//find distance to the edge
					if (d2 < d1) d1 = d2;
					if (d3 < d1) d1 = d3;
		//add to hit list
					HitList temp;
					temp.distance = fabs(d1);
					temp.perpDistance = a;
					temp.faceIndex = i;
					temp.nodeIndex = 0;
					temp.bary = bry;
					temp.fnorm = n;
					hitList.Append(1,&temp,100);
					done = TRUE;

					}
				else
					{

		//add to hit list
					HitList temp;
					temp.distance = fabs(a);
					temp.perpDistance = a;
					temp.faceIndex = i;
					temp.nodeIndex = 0;
					temp.bary = bry;
					temp.fnorm = n;
					hitList.Append(1,&temp,100);
					done = TRUE;
					}

				}
			}
		radius++;

		}


	//add our hit face,normal bary etc to our list



	return hitList.Count();
}

*/