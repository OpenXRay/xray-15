#include "unwrap.h"

/****



******/


void UnwrapMod::fnRelax(int iteration, float str, BOOL lockEdges, BOOL matchArea)
{
// get the number of polyons
	int faceCount = TVMaps.f.Count();
	if (faceCount == 0) return;

	Tab<Point3> centerList;
	Tab<int> centerCount;
	centerList.SetCount(faceCount);
	centerCount.SetCount(faceCount);

//loop through faces
	Point3 p = Point3(0.0f, 0.0f, 0.0f);
	for (int faceIndex = 0; faceIndex < faceCount; faceIndex++)
		{
//compute center points
		centerList[faceIndex] = p;
		centerCount[faceIndex] = 0;
		}
	
	int vertexCount = TVMaps.v.Count();
	if (vertexCount == 0) return;


	for (faceIndex = 0; faceIndex < faceCount; faceIndex++)
		{

		int pointCount = TVMaps.f[faceIndex]->count;
	
		for (int pointIndex = 0; pointIndex < pointCount; pointIndex++)
			{
			int index =  TVMaps.f[faceIndex]->t[pointIndex];
			p = TVMaps.v[index].p;
			centerList[faceIndex] = centerList[faceIndex] + p;
			centerCount[faceIndex] = centerCount[faceIndex] + 1;
			}
		}

	for (faceIndex = 0; faceIndex < faceCount; faceIndex++)
		{
		centerList[faceIndex] = centerList[faceIndex]/centerCount[faceIndex];
		}
	
//store off orignal points
	Tab<Point3> originalPoints;
	originalPoints.SetCount(vertexCount);
	
	for (int j = 0; j < vertexCount; j++)
		{
		originalPoints[j] = TVMaps.v[j].p;
		}


//set iteration level
	Tab<Point3> deltaPoints;
	deltaPoints.SetCount(vertexCount);

	for (int i = 0; i < iteration; i++)
		{

		for (int j = 0; j < vertexCount; j++)
			{
			deltaPoints[j] = originalPoints[j];
			}
		
		for (faceIndex = 0; faceIndex < faceCount; faceIndex++)
			{
			int edgeCount = TVMaps.f[faceIndex]->count;
	
			for (int pointIndex = 0; pointIndex < edgeCount; pointIndex++)
				{
				int index = TVMaps.f[faceIndex]->t[pointIndex];
				Point3 p = Point3(0.0f, 0.0f, 0.0f);
				p = (centerList[faceIndex] - originalPoints[index]) * str;
				deltaPoints[index] = deltaPoints[index] + p;
				if ( (!(TVMaps.f[faceIndex]->flags & FLAG_DEAD)) &&
						(TVMaps.f[faceIndex]->flags & FLAG_CURVEDMAPPING) &&
						(TVMaps.f[faceIndex]->vecs) )
					{
					index = TVMaps.f[faceIndex]->vecs->handles[pointIndex*2];
					p = (centerList[faceIndex] - originalPoints[index]) * str;
					deltaPoints[index] = deltaPoints[index] + p;

					index = TVMaps.f[faceIndex]->vecs->handles[pointIndex*2+1];
					p = (centerList[faceIndex] - originalPoints[index]) * str;
					deltaPoints[index] = deltaPoints[index] + p;
					
					if (TVMaps.f[faceIndex]->flags & FLAG_INTERIOR)
						{
						index  = TVMaps.f[faceIndex]->vecs->interiors[pointIndex];
						p = (centerList[faceIndex] - originalPoints[index]) * str;
						deltaPoints[index] = deltaPoints[index] + p;
						}

					}
	   

				}
			}		
		
		for (j = 0; j < vertexCount; j++)
			{
			originalPoints[j] = deltaPoints[j];
			}
		}

	BitArray skipVerts;
	skipVerts.SetSize(vertexCount);
	skipVerts.ClearAll();

	TransferSelectionStart(); 

	if (lockEdges)
		{

		BitArray tempFSel;
		tempFSel.SetSize(TVMaps.f.Count());
		tempFSel.ClearAll();

		for (j = 0; j < TVMaps.f.Count(); j++)
			{
			int count = TVMaps.f[j]->count;
			int selCount = 0;
			for (int k = 0; k < count; k++)
				{
				int index = TVMaps.f[j]->t[k];
				if (vsel[index] || TVMaps.v[index].influence > 0.0f)
					selCount++;
				}
			if (selCount == count)
				tempFSel.Set(j);
			}
		for (j = 0; j < TVMaps.ePtrList.Count(); j++)
			{
			int a,b, avec, bvec;
			int fcount;
			fcount = TVMaps.ePtrList[j]->faceList.Count();
			a = TVMaps.ePtrList[j]->a;
			b = TVMaps.ePtrList[j]->b;
			avec = TVMaps.ePtrList[j]->avec;
			bvec = TVMaps.ePtrList[j]->bvec;

			int selCount = 0;
			for (int k =0; k < fcount; k++)
				{
				if (tempFSel[TVMaps.ePtrList[j]->faceList[k]])
					selCount++;
				}
			if ((fcount == 1) || (selCount == 1))
				{
				skipVerts.Set(a);
				skipVerts.Set(b);
				if ((avec >= 0) && (avec < TVMaps.v.Count())) skipVerts.Set(avec);
				if ((bvec >= 0) && (bvec < TVMaps.v.Count())) skipVerts.Set(bvec);
				}

			}
		}
	

	theHold.Begin();
	HoldPoints();
	
	TimeValue t = GetCOREInterface()->GetTime();
	
	for (j = 0; j < vertexCount; j++)
		{
		float weight = 0.0f;
		if (vsel[j])
			weight = 1.0f;
		else  
			weight = TVMaps.v[j].influence;
	
		if (skipVerts[j]) weight = 0.0f;
	
		if (weight > 0.0f) 
			{
			Point3 originalPos = Point3(0.0f, 0.0f, 0.0f);
			originalPos = TVMaps.v[j].p;

			Point3 p = (originalPoints[j] - originalPos) * weight;
		
			originalPos = originalPos + p;
			
			TVMaps.v[j].p = originalPos;
			if (TVMaps.cont[j]) TVMaps.cont[j]->SetValue(t,&TVMaps.v[j].p);
			}
		}



	theHold.Accept(_T(GetString(IDS_PW_MOVE_UVW)));

	TransferSelectionEnd(FALSE,FALSE); 


	NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	InvalidateView();
	if (ip) ip->RedrawViews(ip->GetTime());
}



void UnwrapMod::RelaxPatch(int iteration, float str, BOOL lockEdges)
{

	BOOL clearSel = FALSE;
	if (vsel.NumberSet() == 0) 
	{
		clearSel = TRUE;
		vsel.SetAll();
	}
	
// get the number of polyons
	int faceCount = TVMaps.f.Count();
	if (faceCount == 0) return;

	Tab<Point3> centerList;
	Tab<int> centerCount;
	centerList.SetCount(faceCount);
	centerCount.SetCount(faceCount);

//loop through faces
	Point3 p = Point3(0.0f, 0.0f, 0.0f);
	for (int faceIndex = 0; faceIndex < faceCount; faceIndex++)
		{
//compute center points
		centerList[faceIndex] = p;
		centerCount[faceIndex] = 0;
		}
	
	int vertexCount = TVMaps.v.Count();
	if (vertexCount == 0) return;


	for (faceIndex = 0; faceIndex < faceCount; faceIndex++)
		{

		int pointCount = TVMaps.f[faceIndex]->count;
	
		for (int pointIndex = 0; pointIndex < pointCount; pointIndex++)
			{
			int index =  TVMaps.f[faceIndex]->t[pointIndex];
			p = TVMaps.v[index].p;
			centerList[faceIndex] = centerList[faceIndex] + p;
			centerCount[faceIndex] = centerCount[faceIndex] + 1;
			}
		}

	for (faceIndex = 0; faceIndex < faceCount; faceIndex++)
		{
		centerList[faceIndex] = centerList[faceIndex]/centerCount[faceIndex];
		}
	
//store off orignal points
	Tab<Point3> originalPoints;
	originalPoints.SetCount(vertexCount);
	
	for (int j = 0; j < vertexCount; j++)
		{
		originalPoints[j] = TVMaps.v[j].p;
		}


//set iteration level
	Tab<Point3> deltaPoints;
	deltaPoints.SetCount(vertexCount);

	for (int i = 0; i < iteration; i++)
		{

		for (int j = 0; j < vertexCount; j++)
			{
			deltaPoints[j] = originalPoints[j];
			}
		
		for (faceIndex = 0; faceIndex < faceCount; faceIndex++)
			{
			int edgeCount = TVMaps.f[faceIndex]->count;
	
			for (int pointIndex = 0; pointIndex < edgeCount; pointIndex++)
				{
				int index = TVMaps.f[faceIndex]->t[pointIndex];
				Point3 p = Point3(0.0f, 0.0f, 0.0f);
				p = (centerList[faceIndex] - originalPoints[index]) * str;
				deltaPoints[index] = deltaPoints[index] + p;
				if ( (!(TVMaps.f[faceIndex]->flags & FLAG_DEAD)) &&
						(TVMaps.f[faceIndex]->flags & FLAG_CURVEDMAPPING) &&
						(TVMaps.f[faceIndex]->vecs) )
					{
					index = TVMaps.f[faceIndex]->vecs->handles[pointIndex*2];
					p = (centerList[faceIndex] - originalPoints[index]) * str;
					deltaPoints[index] = deltaPoints[index] + p;

					index = TVMaps.f[faceIndex]->vecs->handles[pointIndex*2+1];
					p = (centerList[faceIndex] - originalPoints[index]) * str;
					deltaPoints[index] = deltaPoints[index] + p;
					
					if (TVMaps.f[faceIndex]->flags & FLAG_INTERIOR)
						{
						index  = TVMaps.f[faceIndex]->vecs->interiors[pointIndex];
						p = (centerList[faceIndex] - originalPoints[index]) * str;
						deltaPoints[index] = deltaPoints[index] + p;
						}

					}
	   

				}
			}		
		
		for (j = 0; j < vertexCount; j++)
			{
			originalPoints[j] = deltaPoints[j];
			}
		}

	BitArray skipVerts;
	skipVerts.SetSize(vertexCount);
	skipVerts.ClearAll();


	if (lockEdges)
		{

		BitArray tempFSel;
		tempFSel.SetSize(TVMaps.f.Count());
		tempFSel.ClearAll();

		for (j = 0; j < TVMaps.f.Count(); j++)
			{
			int count = TVMaps.f[j]->count;
			int selCount = 0;
			for (int k = 0; k < count; k++)
				{
				int index = TVMaps.f[j]->t[k];
				if (vsel[index] || TVMaps.v[index].influence > 0.0f)
					selCount++;
				}
			if (selCount == count)
				tempFSel.Set(j);
			}
		for (j = 0; j < TVMaps.ePtrList.Count(); j++)
			{
			int a,b, avec, bvec;
			int fcount;
			fcount = TVMaps.ePtrList[j]->faceList.Count();
			a = TVMaps.ePtrList[j]->a;
			b = TVMaps.ePtrList[j]->b;
			avec = TVMaps.ePtrList[j]->avec;
			bvec = TVMaps.ePtrList[j]->bvec;

			int selCount = 0;
			for (int k =0; k < fcount; k++)
				{
				if (tempFSel[TVMaps.ePtrList[j]->faceList[k]])
					selCount++;
				}
			if ((fcount == 1) || (selCount == 1))
				{
				skipVerts.Set(a);
				skipVerts.Set(b);
				if ((avec >= 0) && (avec < TVMaps.v.Count())) skipVerts.Set(avec);
				if ((bvec >= 0) && (bvec < TVMaps.v.Count())) skipVerts.Set(bvec);
				}

			}
		}
	

	
	TimeValue t = GetCOREInterface()->GetTime();
	
	for (j = 0; j < vertexCount; j++)
		{
		float weight = 0.0f;
		if (vsel[j])
			weight = 1.0f;
		else  
			weight = TVMaps.v[j].influence;
	
		if (skipVerts[j]) weight = 0.0f;
	
		if (weight > 0.0f) 
			{
			Point3 originalPos = Point3(0.0f, 0.0f, 0.0f);
			originalPos = TVMaps.v[j].p;

			Point3 p = (originalPoints[j] - originalPos) * weight;
		
			originalPos = originalPos + p;
			
			TVMaps.v[j].p = originalPos;
			if (TVMaps.cont[j]) TVMaps.cont[j]->SetValue(t,&TVMaps.v[j].p);
			}
		}

	if (clearSel)
		vsel.ClearAll();


}



Matrix3 UnwrapMod::GetTMFromFace(int faceIndex, BOOL useTVFace)

{
	Matrix3 tm(1);
	int edgeCount = TVMaps.f[faceIndex]->count;
	Point3 centroid = Point3(0.0f,0.0f,0.0f);
	for (int j = 0; j < edgeCount; j++)
		{			
		int index = 0;
		if (useTVFace)
			{
			index = TVMaps.f[faceIndex]->t[j];
			centroid += TVMaps.v[index].p;
			}
		else
			{
			index = TVMaps.f[faceIndex]->v[j];
			centroid += TVMaps.geomPoints[index];
			}

		}
	centroid = centroid/edgeCount;
	
	//get normal use it as a z axis
	Point3 vecA = Point3(0.0f,0.0f,0.0f), vecB = Point3(0.0f,0.0f,0.0f);

	int index = 0;
	Point3 basePoint = Point3(0.0f,0.0f,0.0f);

	if (useTVFace)
		{
		index = TVMaps.f[faceIndex]->t[0];
		basePoint = TVMaps.v[index].p;
		index = TVMaps.f[faceIndex]->t[1];
		vecA = Normalize(TVMaps.v[index].p - basePoint);
		}
	else
		{
		index = TVMaps.f[faceIndex]->v[0];
		basePoint = TVMaps.geomPoints[index];
		index = TVMaps.f[faceIndex]->v[1];
		vecA = Normalize(TVMaps.geomPoints[index] - basePoint);
		}

	for (j = 2; j < edgeCount; j++)
		{
		if (useTVFace)
			{
			index = TVMaps.f[faceIndex]->t[j];
			vecB = Normalize(TVMaps.v[index].p - basePoint);
			}
		else
			{
			index = TVMaps.f[faceIndex]->v[j];
			vecB = Normalize(TVMaps.geomPoints[index] - basePoint);
			}

		if (Length(vecA-vecB) > 0.0001f)
			{
			j = edgeCount;
			}
		}
	Point3 normal = Normalize(CrossProd(vecA,vecB));

	//get an axis from centroid to first vertex use an x axis
	//get a perp and use it as a y axis
	Point3 xVec, yVec, zVec;
	zVec = normal;
	xVec = Normalize(basePoint - centroid);
	yVec = Normalize(CrossProd(zVec, xVec));
	//build matrix from it 
	tm.SetRow(0,xVec);
	tm.SetRow(1,yVec);
	tm.SetRow(2,zVec);
	tm.SetRow(3,centroid);
	return tm;

}


void UnwrapMod::fnFit(int iteration, float str)
{
//get scale
	
	float scale = 1.0f;
	float tvLen = 0.0f, gLen = 0.0f;
//get the edge length in tv space
//get the edge length in obj space
	Matrix3 toWorld(1);

//	MyEnumProc2 dep;              
//	EnumDependents(&dep);
//	INode *selfNode = dep.Nodes[0];
//	toWorld = selfNode->GetObjectTM(GetCOREInterface()->GetTime());

	theHold.Begin();
	HoldPoints();
	TransferSelectionStart(); 

	
	for (int i = 0; i < TVMaps.f.Count(); i++)
		{
		int edgeCount = TVMaps.f[i]->count;
//get geom points transfer in face space
		for (int j = 0; j < edgeCount; j++)
			{
			int a,b;
			a = TVMaps.f[i]->t[j];

			if ((j+1) == edgeCount)
				b = TVMaps.f[i]->t[0];
			else b = TVMaps.f[i]->t[j+1];

			if (vsel[a] && vsel[b])
				{
				tvLen += Length(TVMaps.v[a].p-TVMaps.v[b].p);
	
				a = TVMaps.f[i]->v[j];
				if ((j+1) == edgeCount)
					b = TVMaps.f[i]->v[0];
				else b = TVMaps.f[i]->v[j+1];
				gLen += Length(TVMaps.geomPoints[a]-TVMaps.geomPoints[b]);
				}
			}
		}

//get the scale from this
	scale = gLen/tvLen;

//build center list
//build matrix for each geom face
	Tab<Matrix3> geomFaceTM;
	geomFaceTM.SetCount(TVMaps.f.Count());
	for (i = 0; i < TVMaps.f.Count(); i++)
		{
		geomFaceTM[i] = GetTMFromFace(i, FALSE);
		geomFaceTM[i].Scale(Point3(scale,scale,scale));
		geomFaceTM[i] = Inverse(geomFaceTM[i]);

		}


//loop through faces
//tv
	TimeValue t = GetCOREInterface()->GetTime();
	for (int k = 0; k < iteration;k++)
		{
		Tab<Point3> deltaList;
		deltaList.SetCount(TVMaps.v.Count());
		for (i = 0; i < TVMaps.v.Count(); i++)
			deltaList[i] = Point3(0.0f,0.0f,0.0f);

		for (i = 0; i < TVMaps.f.Count(); i++)
			{
	//get uv matrix
			Matrix3 uvFaceTM = GetTMFromFace(i, TRUE);
			Matrix3 gmFaceTM = geomFaceTM[i];

			int edgeCount = TVMaps.f[i]->count;

			Tab<Point3> geomList;
			Tab<Point3> uvList;
			geomList.SetCount(edgeCount);
			uvList.SetCount(edgeCount);
	//get geom points transfer in face space
			for (int j = 0; j < edgeCount; j++)
				{
				int index = TVMaps.f[i]->v[j];
				geomList[j] = TVMaps.geomPoints[index];
				geomList[j] = geomList[j] * geomFaceTM[i]; //optimize to collapse matrix

//				DebugPrint("%d %d Geom %f %f %f \n",i,j,geomList[j].x,geomList[j].y,geomList[j].z);

				index = TVMaps.f[i]->t[j];
				uvList[j] = TVMaps.v[index].p;
				uvList[j] = uvList[j] * Inverse(uvFaceTM); //optimize to collapse matrix
//				DebugPrint("      Geom %f %f %f UVW %f %f %f \n",geomList[j].x,geomList[j].y,geomList[j].z,uvList[j].x,uvList[j].y,uvList[j].z);
				Point3 vec;
				vec = (geomList[j] - uvList[j])*str;
				vec = VectorTransform(uvFaceTM,vec);
				deltaList[index] += vec;
				}
			}
		for (i = 0; i < TVMaps.v.Count(); i++)
			{
			float weight = 1.0f;
			if (vsel[i])
				weight = 1.0f;
			else  weight = TVMaps.v[i].influence;
			TVMaps.v[i].p += deltaList[i]*weight;
			if (TVMaps.cont[i]) TVMaps.cont[i]->SetValue(t,&TVMaps.v[i].p);
			}
		}

	theHold.Accept(_T(GetString(IDS_PW_RELAX)));

	TransferSelectionEnd(FALSE,FALSE); 

	NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	InvalidateView();
	if (ip) ip->RedrawViews(ip->GetTime());




}



float	UnwrapMod::fnGetRelaxAmount()
{
	return relaxAmount;
}
void	UnwrapMod::fnSetRelaxAmount(float amount)
{
	relaxAmount = amount;
}


int		UnwrapMod::fnGetRelaxIter()
{
	return relaxIteration;
}
void	UnwrapMod::fnSetRelaxIter(int iter)
{
	if (iter < 1) iter = 1;
	relaxIteration = iter;
}


BOOL	UnwrapMod::fnGetRelaxBoundary()
{
	return relaxBoundary;
}
void	UnwrapMod::fnSetRelaxBoundary(BOOL boundary)
{
	relaxBoundary = boundary;
}

BOOL	UnwrapMod::fnGetRelaxSaddle()
{
	return relaxSaddle;
}
void	UnwrapMod::fnSetRelaxSaddle(BOOL saddle)
{
	relaxSaddle = saddle;
}


void	UnwrapMod::fnRelax2()
{

	theHold.Begin();
	HoldPoints();
	RelaxVerts2(relaxAmount, relaxIteration, relaxBoundary, relaxSaddle);
	theHold.Accept(_T(GetString(IDS_PW_RELAX)));
}
void	UnwrapMod::fnRelax2Dialog()
{
//bring up the dialog
	DialogBoxParam(	hInstance,
							MAKEINTRESOURCE(IDD_RELAXDIALOG),
							GetCOREInterface()->GetMAXHWnd(),
//							hWnd,
							UnwrapRelaxFloaterDlgProc,
							(LPARAM)this );


}





//RELAX

#define EPSILON float(1e-04)

typedef Tab<DWORD> DWordTab;

class RelaxModData {
public:
	DWordTab *nbor;	// Array of neighbors for each vert.
	BitArray *vis;	// visibility of edges on path to neighbors.
	int *fnum;		// Number of faces for each vert.
	BitArray sel;		// Selection information.
	int vnum;		// Size of above arrays

	RelaxModData ();
	~RelaxModData () { Clear(); }
	void Clear();
	void SetVNum (int num);
	void MaybeAppendNeighbor(int vert, int index, int &max) {
		for (int k1=0; k1<max; k1++) {
			if (nbor[vert][k1] == index)
				return;
			}
		DWORD dwi = (DWORD)index;
		nbor[vert].Append (1, &dwi, 1);
		max++;
		}
};

RelaxModData::RelaxModData () {
	nbor = NULL;
	vis = NULL;
	fnum = NULL;
	vnum = 0;
}

void RelaxModData::Clear () {
	if (nbor) delete [] nbor;
	nbor = NULL;
	if (vis) delete [] vis;
	vis = NULL;
	if (fnum) delete [] fnum;
	fnum = NULL;
}

void RelaxModData::SetVNum (int num) {
	if (num==vnum) return;
	Clear();
	vnum = num;
	if (num<1) return;
	nbor = new DWordTab[vnum];
	vis = new BitArray[vnum];
	fnum = new int[vnum];
	sel.SetSize (vnum);
}


static void FindVertexAngles(PatchMesh &pm, float *vang) {
	int i;
	for (i=0; i<pm.numVerts + pm.numVecs; i++) vang[i] = 0.0f;
	for (i=0; i<pm.numPatches; i++) {
		Patch &p = pm.patches[i];
		for (int j=0; j<p.type; j++) {
			Point3 d1 = pm.vecs[p.vec[j*2]].p - pm.verts[p.v[j]].p;
			Point3 d2 = pm.vecs[p.vec[((j+p.type-1)%p.type)*2+1]].p - pm.verts[p.v[j]].p;
			float len = LengthSquared(d1);
			if (len == 0) continue;
			d1 /= Sqrt(len);
			len = LengthSquared (d2);
			if (len==0) continue;
			d2 /= Sqrt(len);
			float cs = DotProd (d1, d2);
			if (cs>=1) continue;	// angle of 0
			if (cs<=-1) vang[p.v[j]] += PI;
			else vang[p.v[j]] += (float) acos (cs);
		}
	}
}

static void FindVertexAngles (MNMesh &mm, float *vang) {
	int i;
	for (i=0; i<mm.numv; i++) vang[i] = 0.0f;
	for (i=0; i<mm.numf; i++) {
		int *vv = mm.f[i].vtx;
		int deg = mm.f[i].deg;
		for (int j=0; j<deg; j++) {
			Point3 d1 = mm.v[vv[(j+1)%deg]].p - mm.v[vv[j]].p;
			Point3 d2 = mm.v[vv[(j+deg-1)%deg]].p - mm.v[vv[j]].p;
			float len = LengthSquared(d1);
			if (len == 0) continue;
			d1 /= Sqrt(len);
			len = LengthSquared (d2);
			if (len==0) continue;
			d2 /= Sqrt(len);
			float cs = DotProd (d1, d2);
			// STEVE: What about angles over PI?
			if (cs>=1) continue;	// angle of 0
			if (cs<=-1) vang[vv[j]] += PI;
			else vang[vv[j]] += (float) acos (cs);
		}
	}
}




void	UnwrapMod::RelaxVerts2(float relax, int iter, BOOL boundary, BOOL saddle)

{
	
	RelaxModData *rd = new RelaxModData;


	float wtdRelax; // mjm - 4.8.99



	Tab<float> softSel;
	softSel.SetCount(TVMaps.v.Count());



//get the local data
	ModContextList mcList;		
	INodeTab nodes;

	MeshTopoData *md =NULL;

	if (ip) 
		{
		ip->GetModContexts(mcList,nodes);

		int objects = mcList.Count();
		md = (MeshTopoData*)mcList[0]->localData;
		}
	else return;


	Mesh *mesh = NULL;
	PatchMesh *patch = NULL;
	MNMesh *mnMesh = NULL;
	

	if (md->mesh)
		{
		mesh = new Mesh();
		mesh->setNumVerts(TVMaps.v.Count());

		mesh->vertSel = vsel;

		mesh->SupportVSelectionWeights();
		float *vsw = mesh->getVSelectionWeights ();

		for (int i = 0; i<TVMaps.v.Count(); i++)
			{	
			Point3 p = TVMaps.v[i].p;
			p.z = 0.0f;
			mesh->setVert(i,p);
			vsw[i] = TVMaps.v[i].influence;
			}

		mesh->setNumFaces(TVMaps.f.Count());
		for (i = 0; i < TVMaps.f.Count();i++)
			{
			int ct = 3;
			for (int j = 0; j < ct; j++)
				{
				mesh->faces[i].v[j] = TVMaps.f[i]->t[j];
				}
			if (TVMaps.f[i]->flags&FLAG_HIDDENEDGEA)
				mesh->faces[i].setEdgeVis(0, EDGE_INVIS);
			else mesh->faces[i].setEdgeVis(0, EDGE_VIS);

			if (TVMaps.f[i]->flags&FLAG_HIDDENEDGEB)
				mesh->faces[i].setEdgeVis(1, EDGE_INVIS);
			else mesh->faces[i].setEdgeVis(1, EDGE_VIS);

			if (TVMaps.f[i]->flags&FLAG_HIDDENEDGEC)
				mesh->faces[i].setEdgeVis(2, EDGE_INVIS);
			else mesh->faces[i].setEdgeVis(2, EDGE_VIS);

			}

		if (vsel.NumberSet() == 0)
			mesh->selLevel = MESH_OBJECT;
		else mesh->selLevel = MESH_VERTEX;

		}
	else if (md->mnMesh)
		{
		mnMesh = new MNMesh();

		mnMesh->setNumVerts(TVMaps.v.Count());
		mnMesh->setNumFaces(TVMaps.f.Count());


		mnMesh->InvalidateTopoCache();
		mnMesh->VertexSelect (vsel);

		mnMesh->SupportVSelectionWeights();

		float *vsw = mnMesh->getVSelectionWeights ();

		for (int i = 0; i<TVMaps.v.Count(); i++)
			{	
			Point3 p = TVMaps.v[i].p;
			p.z = 0.0f;
			mnMesh->P(i) = p;
			if (vsel[i])
				mnMesh->v[i].SetFlag (MN_SEL);
			else mnMesh->v[i].ClearFlag (MN_SEL);

			vsw[i] = TVMaps.v[i].influence;
			}

		for (i = 0; i < TVMaps.f.Count();i++)
			{

			int ct = TVMaps.f[i]->count;
			mnMesh->f[i].Init();
			mnMesh->f[i].SetDeg(ct);

			for (int j = 0; j < ct; j++)
				{
				mnMesh->f[i].vtx[j] = TVMaps.f[i]->t[j];
				}
			}

		if (vsel.NumberSet() == 0)
			mnMesh->selLevel = MNM_SL_OBJECT;
		else mnMesh->selLevel = MNM_SL_VERTEX;

		// CAL-09/10/03: These don't seem to be necessary.
		// Besides, the topology & numv might be changed after FillInMesh. (Defect #520585)
		// mnMesh->InvalidateGeomCache();
		// mnMesh->FillInMesh();

		}
	else if (md->patch)
		{
		if (relax > .3f) relax = .3f;
		RelaxPatch(iter,relax,boundary);
		return;
		
		}



	if(mesh) {
		int i, j, max;
		DWORD selLevel = mesh->selLevel;
		// mjm - 4.8.99 - add support for soft selection
		// sca - 4.29.99 - extended soft selection support to cover EDGE and FACE selection levels.
		float *vsw = (selLevel!=MESH_OBJECT) ? mesh->getVSelectionWeights() : NULL;


		if (1) {
			rd->SetVNum (mesh->numVerts);
			for (i=0; i<rd->vnum; i++) {
				rd->fnum[i]=0;
				rd->nbor[i].ZeroCount();
			}
			rd->sel.ClearAll ();
			DWORD *v;
			int k1, k2, origmax;
			for (i=0; i<mesh->numFaces; i++) {
				v = mesh->faces[i].v;
				for (j=0; j<3; j++) {
					if ((selLevel==MESH_FACE) && mesh->faceSel[i]) rd->sel.Set(v[j]);
					if ((selLevel==MESH_EDGE) && mesh->edgeSel[i*3+j]) rd->sel.Set(v[j]);
					if ((selLevel==MESH_EDGE) && mesh->edgeSel[i*3+(j+2)%3]) rd->sel.Set(v[j]);
					origmax = max = rd->nbor[v[j]].Count();
					rd->fnum[v[j]]++;
					for (k1=0; k1<max; k1++) if (rd->nbor[v[j]][k1] == v[(j+1)%3]) break;
					if (k1==max) { rd->nbor[v[j]].Append (1, v+(j+1)%3, 1); max++; }
					for (k2=0; k2<max; k2++) if (rd->nbor[v[j]][k2] == v[(j+2)%3]) break;
					if (k2==max) { rd->nbor[v[j]].Append (1, v+(j+2)%3, 1); max++; }
					if (max>origmax) rd->vis[v[j]].SetSize (max, TRUE);
					if (mesh->faces[i].getEdgeVis (j)) rd->vis[v[j]].Set (k1);
					else if (k1>=origmax) rd->vis[v[j]].Clear (k1);
					if (mesh->faces[i].getEdgeVis ((j+2)%3)) rd->vis[v[j]].Set (k2);
					else if (k2>= origmax) rd->vis[v[j]].Clear (k2);
				}
			}
	// mjm - begin - 4.8.99
	//		if (selLevel==MESH_VERTEX) rd->sel = mesh->vertSel;
			if (selLevel==MESH_VERTEX)
				rd->sel = mesh->vertSel;
			else if (selLevel==MESH_OBJECT)
				rd->sel.SetAll ();
	// mjm - end
		}

		Tab<float> vangles;
		if (saddle) vangles.SetCount (rd->vnum);
		Point3 *hold = new Point3[rd->vnum];
		int act;
		for (int k=0; k<iter; k++) {
			for (i=0; i<rd->vnum; i++) hold[i] = mesh->verts[i];
			if (saddle) mesh->FindVertexAngles (vangles.Addr(0));
			for (i=0; i<rd->vnum; i++) {
	// mjm - begin - 4.8.99
	//			if ((selLevel!=MESH_OBJECT) && (!rd->sel[i])) continue;
				if ( (!rd->sel[i] ) && (!vsw || vsw[i] == 0) ) continue;
	// mjm - end
				if (saddle && (vangles[i] <= 2*PI*.99999f)) continue;
				max = rd->nbor[i].Count();
				if (boundary && (rd->fnum[i] < max)) continue;
				if (max<1) continue;
				Point3 avg(0.0f, 0.0f, 0.0f);
				for (j=0,act=0; j<max; j++) {
					if (!rd->vis[i][j]) continue;
					act++;
					avg += hold[rd->nbor[i][j]];
				}
				if (act<1) continue;
	// mjm - begin - 4.8.99
				wtdRelax = (!rd->sel[i]) ? relax * vsw[i] : relax;
				mesh->verts[i] = hold[i]*(1-wtdRelax) + avg*wtdRelax/((float)act);
	//			triObj->SetPoint (i, hold[i]*(1-relax) + avg*relax/((float)act));
	// mjm - end
			}
		}
		delete [] hold;
	}

	if (mnMesh) {
		int i, j, max;
		MNMesh & mm = *mnMesh;
		float *vsw = (mm.selLevel!=MNM_SL_OBJECT) ? mm.getVSelectionWeights() : NULL;


		if (1) {
			rd->SetVNum (mm.numv);
			for (i=0; i<rd->vnum; i++) {
				rd->fnum[i]=0;
				rd->nbor[i].ZeroCount();
			}
			rd->sel = mm.VertexTempSel ();
			int k1, k2, origmax;
			for (i=0; i<mm.numf; i++) {
				int deg = mm.f[i].deg;
				int *vtx = mm.f[i].vtx;
				for (j=0; j<deg; j++) {
					Tab<DWORD> & nbor = rd->nbor[vtx[j]];
					origmax = max = nbor.Count();
					rd->fnum[vtx[j]]++;
					DWORD va = vtx[(j+1)%deg];
					DWORD vb = vtx[(j+deg-1)%deg];
					for (k1=0; k1<max; k1++) if (nbor[k1] == va) break;
					if (k1==max) { nbor.Append (1, &va, 1); max++; }
					for (k2=0; k2<max; k2++) if (nbor[k2] == vb) break;
					if (k2==max) { nbor.Append (1, &vb, 1); max++; }
				}
			}
		}

		Tab<float> vangles;
		if (saddle) vangles.SetCount (rd->vnum);
		Tab<Point3> hold;
		hold.SetCount (rd->vnum);
		int act;
		for (int k=0; k<iter; k++) {
			for (i=0; i<rd->vnum; i++) hold[i] = mm.P(i);
			if (saddle) FindVertexAngles (mm, vangles.Addr(0));
			for (i=0; i<rd->vnum; i++) {
				if ((!rd->sel[i]) && (!vsw || vsw[i] == 0) ) continue;
				if (saddle && (vangles[i] <= 2*PI*.99999f)) continue;
				max = rd->nbor[i].Count();
				if (boundary && (rd->fnum[i] < max)) continue;
				if (max<1) continue;
				Point3 avg(0.0f, 0.0f, 0.0f);
				for (j=0,act=0; j<max; j++) {
					act++;
					avg += hold[rd->nbor[i][j]];
				}
				if (act<1) continue;
				wtdRelax = (!rd->sel[i]) ? relax * vsw[i] : relax;
				mm.P(i) =  hold[i]*(1-wtdRelax) + avg*wtdRelax/((float)act);
			}
		}
	}
/*
#ifndef NO_PATCHES
	else
	if(patchObj) {
		int i, j, max;
		DWORD selLevel = pmesh.selLevel;
		// mjm - 4.8.99 - add support for soft selection
		// sca - 4.29.99 - extended soft selection support to cover EDGE and FACE selection levels.
		float *vsw = (selLevel!=PATCH_OBJECT) ? pmesh.GetVSelectionWeights() : NULL;

		if (rd->ivalid.InInterval(t) && (pmesh.numVerts != rd->vnum)) {
			// Shouldn't happen, but does with Loft bug and may with other bugs.
			rd->ivalid.SetEmpty ();
		}

		if (!rd->ivalid.InInterval(t)) {
			int vecBase = pmesh.numVerts;
			rd->SetVNum (pmesh.numVerts + pmesh.numVecs);
			for (i=0; i<rd->vnum; i++) {
				rd->fnum[i]=1;		// For patches, this means it's not a boundary
				rd->nbor[i].ZeroCount();
			}
			rd->sel.ClearAll ();
			for (i=0; i<pmesh.numPatches; i++) {
				Patch &p = pmesh.patches[i];
				int vecLimit = p.type * 2;
				for (j=0; j<p.type; j++) {
					PatchEdge &e = pmesh.edges[p.edge[j]];
					BOOL isBoundary = (e.patches.Count() < 2) ? TRUE : FALSE;
					int theVert = p.v[j];
					int nextVert = p.v[(j+1)%p.type];
					int nextVec = p.vec[j*2] + vecBase;
					int nextVec2 = p.vec[j*2+1] + vecBase;
					int prevEdge = (j+p.type-1)%p.type;
					int prevVec = p.vec[prevEdge*2+1] + vecBase;
					int prevVec2 = p.vec[prevEdge*2] + vecBase;
					int theInterior = p.interior[j] + vecBase;
					// Establish selection bits
					if ((selLevel==PATCH_PATCH) && pmesh.patchSel[i]) {
						rd->sel.Set(theVert);
						rd->sel.Set(nextVec);
						rd->sel.Set(prevVec);
						rd->sel.Set(theInterior);
						}
					else
					if ((selLevel==PATCH_EDGE) && pmesh.edgeSel[p.edge[j]]) {
						rd->sel.Set(e.v1);
						rd->sel.Set(e.vec12 + vecBase);
						rd->sel.Set(e.vec21 + vecBase);
						rd->sel.Set(e.v2);
						}
					else
					if ((selLevel==PATCH_VERTEX) && pmesh.vertSel[theVert]) {
						rd->sel.Set(theVert);
						rd->sel.Set(nextVec);
						rd->sel.Set(prevVec);
						rd->sel.Set(theInterior);
						}

					// Set boundary flags if necessary
					if(isBoundary) {
						rd->fnum[theVert] = 0;
						rd->fnum[nextVec] = 0;
						rd->fnum[nextVec2] = 0;
						rd->fnum[nextVert] = 0;
						}

					// First process the verts
					int work = theVert;
					max = rd->nbor[work].Count();
					// Append the neighboring vectors
					rd->MaybeAppendNeighbor(work, nextVec, max);
					rd->MaybeAppendNeighbor(work, prevVec, max);
					rd->MaybeAppendNeighbor(work, theInterior, max);

					// Now process the edge vectors
					work = nextVec;
					max = rd->nbor[work].Count();
					// Append the neighboring points
					rd->MaybeAppendNeighbor(work, theVert, max);
					rd->MaybeAppendNeighbor(work, theInterior, max);
					rd->MaybeAppendNeighbor(work, prevVec, max);
					rd->MaybeAppendNeighbor(work, nextVec2, max);
					rd->MaybeAppendNeighbor(work, p.interior[(j+1)%p.type] + vecBase, max);

					work = prevVec;
					max = rd->nbor[work].Count();
					// Append the neighboring points
					rd->MaybeAppendNeighbor(work, theVert, max);
					rd->MaybeAppendNeighbor(work, theInterior, max);
					rd->MaybeAppendNeighbor(work, nextVec, max);
					rd->MaybeAppendNeighbor(work, prevVec2, max);
					rd->MaybeAppendNeighbor(work, p.interior[(j+p.type-1)%p.type] + vecBase, max);

					// Now append the interior, if not auto
					if(!p.IsAuto()) {
						work = theInterior;
						max = rd->nbor[work].Count();
						// Append the neighboring points
						rd->MaybeAppendNeighbor(work, p.v[j], max);
						rd->MaybeAppendNeighbor(work, nextVec, max);
						rd->MaybeAppendNeighbor(work, nextVec2, max);
						rd->MaybeAppendNeighbor(work, prevVec, max);
						rd->MaybeAppendNeighbor(work, prevVec2, max);
						for(int k = 1; k < p.type; ++k)
							rd->MaybeAppendNeighbor(work, p.interior[(j+k)%p.type] + vecBase, max);
						}
					}
				}
	// mjm - begin - 4.8.99
			if (selLevel==PATCH_VERTEX) {
				for (int i=0; i<pmesh.numVerts; ++i) {
					if (pmesh.vertSel[i]) rd->sel.Set(i);
				}
			}
			else if (selLevel==PATCH_OBJECT) rd->sel.SetAll();
	// mjm - end
			rd->ivalid  = os->obj->ChannelValidity (t, TOPO_CHAN_NUM);
			rd->ivalid &= os->obj->ChannelValidity (t, SUBSEL_TYPE_CHAN_NUM);
			rd->ivalid &= os->obj->ChannelValidity (t, SELECT_CHAN_NUM);
		}

		Tab<float> vangles;
		if (saddle) vangles.SetCount (rd->vnum);
		Point3 *hold = new Point3[rd->vnum];
		int act;
		for (int k=0; k<iter; k++) {
			for (i=0; i<rd->vnum; i++) hold[i] = patchObj->GetPoint(i);
			if (saddle)
				FindVertexAngles(pmesh, vangles.Addr(0));
			for (i=0; i<rd->vnum; i++) {
	// mjm - begin - 4.8.99
	//			if ((selLevel!=MESH_OBJECT) && (!rd->sel[i])) continue;
				if ( (!rd->sel[i] ) && (!vsw || vsw[i] == 0) ) continue;
	// mjm - end
				if (saddle && (i < pmesh.numVerts) && (vangles[i] <= 2*PI*.99999f)) continue;
				max = rd->nbor[i].Count();
				if (boundary && !rd->fnum[i]) continue;
				if (max<1)
					continue;
				Point3 avg(0.0f, 0.0f, 0.0f);
				for (j=0,act=0; j<max; j++) {
					act++;
					avg += hold[rd->nbor[i][j]];
				}
				if (act<1)
					continue;
	// mjm - begin - 4.8.99
				wtdRelax = (!rd->sel[i]) ? relax * vsw[i] : relax;
				patchObj->SetPoint (i, hold[i]*(1-wtdRelax) + avg*wtdRelax/((float)act));
	//			patchObj->SetPoint (i, hold[i]*(1-relax) + avg*relax/((float)act));
	// mjm - end
			}
		}
		delete [] hold;
		patchObj->patch.computeInteriors();
		patchObj->patch.ApplyConstraints();
	}
#endif // NO_PATCHES
*/	

	TimeValue t = ip->GetTime();
	if (mesh) 
		{
		// CAL-09/10/03: Use TVMaps.v.Count() (was mesh->numVerts)
		for (int i = 0; i < TVMaps.v.Count(); i++)
			{
			TVMaps.v[i].p.x = mesh->verts[i].x;
			TVMaps.v[i].p.y = mesh->verts[i].y;
			if (TVMaps.cont[i]) TVMaps.cont[i]->SetValue(t,&TVMaps.v[i].p);
			}
		delete mesh;
		}
	if (mnMesh) 
		{
		// CAL-09/10/03: Use TVMaps.v.Count() (was mnMesh->numv), because numv might be changed after FillInMesh. (Defect #520585)
		for (int i = 0; i < TVMaps.v.Count(); i++)
			{
			TVMaps.v[i].p.x = mnMesh->P(i).x;
			TVMaps.v[i].p.y = mnMesh->P(i).y;
			if (TVMaps.cont[i]) TVMaps.cont[i]->SetValue(t,&TVMaps.v[i].p);
			}

		delete mnMesh;
		}
	if (patch) delete patch;
}


void	UnwrapMod::SetRelaxDialogPos()
	{
	if (relaxWindowPos.length != 0) 
		SetWindowPlacement(relaxHWND,&relaxWindowPos);
	}

void	UnwrapMod::SaveRelaxDialogPos()
	{
	relaxWindowPos.length = sizeof(WINDOWPLACEMENT); 
	GetWindowPlacement(relaxHWND,&relaxWindowPos);
	}


INT_PTR CALLBACK UnwrapRelaxFloaterDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	UnwrapMod *mod = (UnwrapMod*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	//POINTS p = MAKEPOINTS(lParam);	commented out by sca 10/7/98 -- causing warning since unused.
	
	static ISpinnerControl *iAmount = NULL;
	static ISpinnerControl *iIterations = NULL;

	static BOOL bBoundary = TRUE;
	static BOOL bCorner = FALSE;
	static float amount = 1.0f;
	static int iterations = 1;

	switch (msg) {
		case WM_INITDIALOG:
			{


			mod = (UnwrapMod*)lParam;
			mod->relaxHWND = hWnd;

			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);

//save our points

//create relax amount spinner and set value
			iAmount = SetupFloatSpinner(
				hWnd,IDC_RELAX_AMOUNTSPIN,IDC_RELAX_AMOUNT,
				-1.0f,1.0f,mod->relaxAmount);	
			iAmount->SetScale(0.01f);
			amount = mod->relaxAmount;

			iIterations = SetupIntSpinner(
				hWnd,IDC_RELAX_ITERATIONSSPIN,IDC_RELAX_ITERATIONS,
				0,1000,mod->relaxIteration);	
			iIterations->SetScale(1.f);
			iterations = mod->relaxIteration;

//set align cluster
			bBoundary = mod->relaxBoundary;
			bCorner = mod->relaxSaddle;
			CheckDlgButton(hWnd,IDC_BOUNDARY_CHECK,bBoundary);
			CheckDlgButton(hWnd,IDC_CORNERS_CHECK,bCorner);
			
//restore window pos
			mod->SetRelaxDialogPos();
//start the hold begin
			if (!theHold.Holding())
				{
				theHold.SuperBegin();
				theHold.Begin();
				}
//hold the points 
			mod->HoldPoints();

//stitch initial selection
			mod->RelaxVerts2(amount,iterations,bBoundary,bCorner);

			mod->InvalidateView();
			TimeValue t = GetCOREInterface()->GetTime();
			mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(t);

			break;
			}
		case CC_SPINNER_BUTTONDOWN:
			if (LOWORD(wParam) == IDC_UNWRAP_BIASSPIN) 
				{
				}
			break;


		case CC_SPINNER_CHANGE:
			if ( (LOWORD(wParam) == IDC_RELAX_AMOUNTSPIN) ||
				 (LOWORD(wParam) == IDC_RELAX_ITERATIONSSPIN) )
				{
//get align
				amount = iAmount->GetFVal();
				iterations = iIterations->GetIVal();
//call stitch again
				theHold.Restore();
				mod->RelaxVerts2(amount,iterations,bBoundary,bCorner);

				mod->InvalidateView();
				TimeValue t = GetCOREInterface()->GetTime();
				mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
				GetCOREInterface()->RedrawViews(t);
				}
			break;

		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP:
			if ( (LOWORD(wParam) == IDC_RELAX_AMOUNTSPIN) || (LOWORD(wParam) == IDC_RELAX_ITERATIONSSPIN) )
				{
				mod->InvalidateView();
				TimeValue t = GetCOREInterface()->GetTime();
				mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
				GetCOREInterface()->RedrawViews(t);

				}
			break;


		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_APPLY:
					{
					theHold.Accept(_T(GetString(IDS_PW_RELAX)));
					theHold.SuperAccept(_T(GetString(IDS_PW_RELAX)));

					theHold.SuperBegin();
					theHold.Begin();
					mod->HoldPoints();

					mod->RelaxVerts2(amount,iterations,bBoundary,bCorner);
					mod->InvalidateView();
					TimeValue t = GetCOREInterface()->GetTime();
					mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
					GetCOREInterface()->RedrawViews(t);

					break;
					}
				case IDC_APPLYOK:
					{
					theHold.Accept(_T(GetString(IDS_PW_RELAX)));
					theHold.SuperAccept(_T(GetString(IDS_PW_RELAX)));
					mod->SaveRelaxDialogPos();

					ReleaseISpinner(iAmount);
					iAmount = NULL;
					ReleaseISpinner(iIterations);
					iIterations = NULL;

					 EndDialog(hWnd,1);
					
					break;
					}
				case IDC_REVERT:
					{
					theHold.Restore();
					theHold.Cancel();
					theHold.SuperCancel();
				
					mod->InvalidateView();

					ReleaseISpinner(iAmount);
					iAmount = NULL;
					ReleaseISpinner(iIterations);
					iIterations = NULL;
					EndDialog(hWnd,0);

					break;
					}
				case IDC_DEFAULT:
					{
//get bias
					amount = iAmount->GetFVal();
					iterations = iIterations->GetIVal();

					mod->relaxAmount = amount;
					mod->relaxIteration = iterations;

//get align
					bCorner = IsDlgButtonChecked(hWnd,IDC_CORNERS_CHECK);
					bBoundary = IsDlgButtonChecked(hWnd,IDC_BOUNDARY_CHECK);
					mod->relaxBoundary = bBoundary;
					mod->relaxSaddle = bCorner;
//set as defaults
					break;
					}
				case IDC_BOUNDARY_CHECK:
					{
//get align
					bBoundary = IsDlgButtonChecked(hWnd,IDC_BOUNDARY_CHECK);

					theHold.Restore();
					mod->RelaxVerts2(amount,iterations,bBoundary,bCorner);

					mod->InvalidateView();
					TimeValue t = GetCOREInterface()->GetTime();
					mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
					GetCOREInterface()->RedrawViews(t);

					break;
					}

				case IDC_CORNERS_CHECK:
					{
//get align
					bCorner = IsDlgButtonChecked(hWnd,IDC_CORNERS_CHECK);

					theHold.Restore();
					mod->RelaxVerts2(amount,iterations,bBoundary,bCorner);

					mod->InvalidateView();
					TimeValue t = GetCOREInterface()->GetTime();
					mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
					GetCOREInterface()->RedrawViews(t);

					break;
					}

				}
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}
