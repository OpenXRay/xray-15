#include "unwrap.h"



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

void UnwrapMod::BuildNormals(MeshTopoData *md, Tab<Point3> &normList)
	{

	if (md == NULL) 
		{
		return;
		}


	if ((objType == IS_MESH) && (!md->mesh))
		{
		updateCache = TRUE;
		NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
		}
	else if ((objType == IS_PATCH) && (!md->patch))
		{
		updateCache = TRUE;
		NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
		}
	else if ((objType == IS_MNMESH) && (!md->mnMesh))
		{
		updateCache = TRUE;
		NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
		}

	if(md->mesh)
		{
		normList.SetCount(md->mesh->numFaces);
		for (int i =0; i < md->mesh->numFaces; i++)
			{
//build normal
			Point3 p[3];
			for (int j =0; j < 3; j++)
				{
				p[j] = md->mesh->verts[md->mesh->faces[i].v[j]];
				}
			Point3 vecA,vecB, norm;
			vecA = Normalize(p[1] - p[0]);
			vecB = Normalize(p[2] - p[0]);
			norm = Normalize(CrossProd(vecA,vecB));


			normList[i] = norm;
			}
		}
	else if (md->mnMesh)
		{
		normList.SetCount(md->mnMesh->numf);
		for (int i =0; i < md->mnMesh->numf; i++)
			{
			Point3 norm =  md->mnMesh->GetFaceNormal (i, TRUE);
			normList[i] = norm;
			}
		}
	else if (md->patch)
		{
		normList.SetCount(md->patch->numPatches);
		for (int i =0; i < md->patch->numPatches; i++)
			{
			Point3 norm(0.0f,0.0f,0.0f);
			for (int j=0; j < md->patch->patches[i].type; j++)
				{
				Point3 vecA, vecB, p;
				p = md->patch->verts[md->patch->patches[i].v[j]].p;
				vecA = md->patch->vecs[md->patch->patches[i].vec[j*2]].p;
				if (j==0)
					vecB = md->patch->vecs[md->patch->patches[i].vec[md->patch->patches[i].type*2-1]].p;
				else vecB = md->patch->vecs[md->patch->patches[i].vec[j*2-1]].p;
				vecA = Normalize(p - vecA);
				vecB = Normalize(p - vecB);
				norm += Normalize(CrossProd(vecA,vecB));
				}
			normList[i] = Normalize(norm/(float)md->patch->patches[i].type);
			}
		}
	}






void UnwrapMod::PlanarMapNoScale(Point3 gNormal)

	{
	AlignMap();

//add vertices to our internal vertex list filling in dead spots where appropriate
	int ct = 0;  

//get align normal
//get fit data

	PlanarTM.IdentityMatrix();
	Interval v;
	Point3 identScale(1.0f,1.0f,1.0f);
	if (scaleControl) scaleControl->GetValue(0,&gScale,v);
	PlanarTM.SetScale(identScale);
	if (rotateControl) rotateControl->GetValue(0,&gRotate,v);
	PlanarTM.RotateZ(gRotate);
	if (offsetControl) offsetControl->GetValue(0,&gOffset,v);
	PlanarTM.SetTrans(gOffset);

	ComputeSelectedFaceData();



	Matrix3 gtm;
	UnwrapMatrixFromNormal(gNormal,gtm);

	gtm = Inverse(gtm);

	DeleteVertsFromFace(gfaces);

//unselect all verts
	for (int j=0;j<TVMaps.v.Count();j++)
		{
		if (vsel[j])
			{
			vsel.Clear(j);
			}
		}

//build available list
	Tab<int> alist;
	alist.ZeroCount();

	for (j=0;j<TVMaps.v.Count();j++)
		{
		if (TVMaps.v[j].flags & FLAG_DEAD)
//dead veretx found copy new vert in and note the place
			{
			alist.Append(1,&j,1);
			}
		}


	for (int i = 0; i < gverts.d.Count(); i++)
		{
		BOOL found = FALSE;
		if (gverts.sel[i])
			{
			if (ct < alist.Count() )
				{
				j = alist[ct];
				TVMaps.v[j].flags = 0;
				TVMaps.v[j].influence = 0.0f;
			
				Point3 tp = gverts.d[i].p - gCenter;
				tp = tp * gtm;
				tp.z = 0.0f;

				TVMaps.v[j].p = tp;
				int vcount = vsel.GetSize();
				vsel.Set(j);

				if (TVMaps.cont[j]) TVMaps.cont[j]->SetValue(0,&tp,CTRL_ABSOLUTE);
				gverts.d[i].newindex = j;
				ct++;

				}
			else
				{
				UVW_TVVertClass tempv;

				Point3 tp = gverts.d[i].p - gCenter;
				tp = tp * gtm;
				tp.z = 0.0f;
				tempv.p = tp;


				tempv.flags = 0;
				tempv.influence = 0.0f;
				gverts.d[i].newindex = TVMaps.v.Count();
				TVMaps.v.Append(1,&tempv,1);

				vsel.SetSize(TVMaps.v.Count(), 1);
				vsel.Set(TVMaps.v.Count()-1);

				Control* c;
				c = NULL;
				TVMaps.cont.Append(1,&c,1);
				if (TVMaps.cont[TVMaps.v.Count()-1]) 
					TVMaps.cont[TVMaps.v.Count()-1]->SetValue(0,&TVMaps.v[TVMaps.v.Count()-1].p,CTRL_ABSOLUTE);
				}
			}

		}
//now copy our face data over
	for (i = 0; i < gfaces.Count(); i++)
		{
		int ct = gfaces[i]->FaceIndex;
		TVMaps.f[ct]->flags = gfaces[i]->flags;
		TVMaps.f[ct]->flags |= FLAG_SELECTED;
		int pcount = 3;
		pcount = gfaces[i]->count;
		for (int j = 0; j < pcount; j++)
			{
			int index = gfaces[i]->t[j];
//find spot in our list
			TVMaps.f[ct]->t[j] = gverts.d[index].newindex;
			if ((TVMaps.f[ct]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[ct]->vecs))
				{
				index = gfaces[i]->vecs->handles[j*2];
//find spot in our list
				TVMaps.f[ct]->vecs->handles[j*2] = gverts.d[index].newindex;

				index = gfaces[i]->vecs->handles[j*2+1];
//find spot in our list
				TVMaps.f[ct]->vecs->handles[j*2+1] = gverts.d[index].newindex;

				if (TVMaps.f[ct]->flags & FLAG_INTERIOR)
					{
					index = gfaces[i]->vecs->interiors[j];
//find spot in our list
					TVMaps.f[ct]->vecs->interiors[j] = gverts.d[index].newindex;
					}
	
				}
			}
		}
		
	CleanUpDeadVertices();

	TVMaps.edgesValid= FALSE;
	}






void	UnwrapMod::BuildUsedListFromSelection(BitArray &usedVerts)
	{
	usedVerts.SetSize(TVMaps.v.Count());
	usedVerts.ClearAll();

	for (int j =0; j < TVMaps.f.Count(); j++)
		{
		int faceIndex = j;
		if (TVMaps.f[faceIndex]->flags & FLAG_SELECTED)
			{
			for (int k = 0; k <  TVMaps.f[faceIndex]->count; k++)
				{
//ne	ed to put patch handles in here also
				int index = TVMaps.f[faceIndex]->t[k];
				usedVerts.Set(index);
				if ((TVMaps.f[faceIndex]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[faceIndex]->vecs))
					{
					index = TVMaps.f[faceIndex]->vecs->handles[k*2];
					if ((index >= 0) && (index < usedVerts.GetSize()))
						usedVerts.Set(index);

					index = TVMaps.f[faceIndex]->vecs->handles[k*2+1];
					if ((index >= 0) && (index < usedVerts.GetSize()))
						usedVerts.Set(index);
	
					if (TVMaps.f[faceIndex]->flags & FLAG_INTERIOR) 
						{
						index = TVMaps.f[faceIndex]->vecs->interiors[k];
						if ((index >= 0) && (index < usedVerts.GetSize()))
							usedVerts.Set(index);
						}
					}
				}
			}
		}

	}

void	UnwrapMod::BuildUsedList(BitArray &usedVerts, ClusterClass *cluster)
	{
	usedVerts.SetSize(TVMaps.v.Count());
	usedVerts.ClearAll();

	for (int j =0; j < cluster->faces.Count(); j++)
		{
		int faceIndex = cluster->faces[j];
		for (int k = 0; k <  TVMaps.f[faceIndex]->count; k++)
			{
//need to put patch handles in here also
			int index = TVMaps.f[faceIndex]->t[k];
			usedVerts.Set(index);
			if ((TVMaps.f[faceIndex]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[faceIndex]->vecs))
				{
				index = TVMaps.f[faceIndex]->vecs->handles[k*2];
				if ((index >= 0) && (index < usedVerts.GetSize()))
					usedVerts.Set(index);

				index = TVMaps.f[faceIndex]->vecs->handles[k*2+1];
				if ((index >= 0) && (index < usedVerts.GetSize()))
					usedVerts.Set(index);

				if (TVMaps.f[faceIndex]->flags & FLAG_INTERIOR) 
					{
					index = TVMaps.f[faceIndex]->vecs->interiors[k];
					if ((index >= 0) && (index < usedVerts.GetSize()))
						usedVerts.Set(index);
					}
				}
			}
		}

	}

void	UnwrapMod::BuildUsedList(BitArray &usedVerts, int clusterIndex)
	{


	ClusterClass *cluster = clusterList[clusterIndex];

	BuildUsedList(usedVerts, cluster);

	}



Point3*	UnwrapMod::fnGetNormal(int faceIndex)
	{
	//check for type
	ModContextList mcList;		
	INodeTab nodes;
	
	Point3 norm(0.0f,0.0f,0.0f);

	n = norm;
	if (!ip) return &n;
	ip->GetModContexts(mcList,nodes);

	int objects = mcList.Count();

	

	faceIndex--;


	if (objects != 0)
		{
		MeshTopoData *md = (MeshTopoData*)mcList[0]->localData;

		if (md == NULL) 
			{
			return NULL;
			}


		Tab<Point3> objNormList;
		BuildNormals(md,objNormList);
		if ((faceIndex >= 0) && (faceIndex < objNormList.Count()))
			norm = objNormList[faceIndex];
		else
			{
			faceIndex = 0;
			int ct = 1;
			for (int i =0; i < md->faceSel.GetSize(); i++)
				{
				if (md->faceSel[i])
					{
					faceIndex = i;
					norm = objNormList[faceIndex];
					TSTR normstr;
					normstr.printf("norm%d = Point3 %f %f %f",ct,norm.x,norm.y,norm.z);
					ct++;
					macroRecorder->ScriptString(normstr);
					macroRecorder->EmitScript();
					}
				}
			}
		}
	n = norm;
	return &n;
	}



void	UnwrapMod::fnUnfoldSelectedPolygons(int unfoldMethod, BOOL normalize)
	{		
// flatten selected polygons
	BailStart();
	BitArray *polySel = fnGetSelectedPolygons();
	BitArray holdPolySel;
	if (polySel == NULL) 
		return;
	if (TVMaps.f.Count() == 0) return;


	if (!theHold.Holding())
		{
		theHold.SuperBegin();
		theHold.Begin();
		}

	holdPolySel.SetSize(polySel->GetSize());
	holdPolySel = *polySel;

	HoldPointsAndFaces();	

	Point3 normal(0.0f,0.0f,1.0f);

	BitArray oldSel = *fnGetSelectedPolygons();

	Tab<Point3> mapNormal;
	mapNormal.SetCount(0);

	BOOL bContinue = BuildCluster( mapNormal, 5.0f, TRUE, TRUE);
	TSTR statusMessage;

	BitArray sel;
	sel.SetSize(TVMaps.f.Count());

	if (bContinue)
		{
			
		for (int i =0; i < clusterList.Count(); i++)
			{
			sel.ClearAll();
			for (int j = 0; j < clusterList[i]->faces.Count();j++)
				sel.Set(clusterList[i]->faces[j]);
			fnSelectPolygonsUpdate(&sel, FALSE);
			PlanarMapNoScale(clusterList[i]->normal);

			int per = (i * 100)/clusterList.Count();
			statusMessage.printf("%s %d%%.",GetString(IDS_PW_STATUS_MAPPING),per);
			if (Bail(ip,statusMessage))
				{
				i = clusterList.Count();
				bContinue =  FALSE;
				}

			}


		if ( (bContinue) && (clusterList.Count() > 1) )
			{

			if (!ip) return;
			ModContextList mcList;		
			INodeTab nodes;
			ip->GetModContexts(mcList,nodes);

			int objects = mcList.Count();

			MeshTopoData *md = (MeshTopoData*)mcList[0]->localData;

			if (md == NULL) 
				{
				theHold.Cancel();
				theHold.SuperCancel();
				return;
				}

			Tab<Point3> objNormList;
			BuildNormals(md,objNormList);

//remove internal edges
			BitArray *selectedPolygons = fnGetSelectedPolygons();
			Tab<int> clusterGroups;
			clusterGroups.SetCount(TVMaps.f.Count());
			for (i =0; i < clusterGroups.Count(); i++)
				{
				clusterGroups[i] = -1;
				}
		//loop through all tagged edges and remove any that onely have one edhes selected
			for (i = 0; i < clusterList.Count(); i++)
				{
				for (int j = 0; j < clusterList[i]->faces.Count(); j++)
					{
					int faceIndex = clusterList[i]->faces[j];
					clusterGroups[faceIndex] = i;
					}
				}
			BitArray processedClusters;
			processedClusters.SetSize(clusterList.Count());
			processedClusters.ClearAll();

			Tab<BorderClass> edgesToBeProcessed;

			BOOL done = FALSE;
			int currentCluster = 0;

			processedClusters.Set(0);
			clusterList[0]->newX = 0.0f;
			clusterList[0]->newY = 0.0f;
//		clusterList[0]->angle = 0.0f;
			for (int i = 0; i < clusterList[0]->borderData.Count(); i++)
				{
				int outerFaceIndex = clusterList[0]->borderData[i].outerFace;
				int connectedClusterIndex = clusterGroups[outerFaceIndex];
				if ((connectedClusterIndex != 0) && (connectedClusterIndex != -1))
					{
					edgesToBeProcessed.Append(1,&clusterList[0]->borderData[i]);
					}
				}
			BitArray seedFaceList;
			seedFaceList.SetSize(clusterGroups.Count());
			seedFaceList.ClearAll();
			for (i = 0; i < seedFaces.Count(); i++)
				{
				seedFaceList.Set(seedFaces[i]);
				}

			while (!done)
				{
				Tab<int> clustersJustProcessed;
				clustersJustProcessed.ZeroCount();
				done = TRUE;

				int edgeToAlign = -1;
				float angDist = PI*2;
				if (unfoldMethod == 1)
					angDist =  PI*2;
				else if (unfoldMethod == 2) angDist = 0;
				for (i = 0; i < edgesToBeProcessed.Count(); i++)
					{
					int outerFace = edgesToBeProcessed[i].outerFace;
					int connectedClusterIndex = clusterGroups[outerFace];
					if (!processedClusters[connectedClusterIndex])
						{
						int innerFaceIndex = edgesToBeProcessed[i].innerFace;
						int outerFaceIndex = edgesToBeProcessed[i].outerFace;
//get angle
						Point3 innerNorm, outerNorm;
						innerNorm = objNormList[innerFaceIndex];
						outerNorm = objNormList[outerFaceIndex];
						float dot = DotProd(innerNorm,outerNorm);

						float angle = 0.0f;

						if (dot == -1.0f)
							angle = PI;
						else if (dot == 1.0f)
							angle = 0.f;						
						else angle = acos(dot);

						if (unfoldMethod == 1)
							{
							if (seedFaceList[outerFaceIndex])
								angle = 0.0f;
							if (angle < angDist)
								{
								angDist = angle;
								edgeToAlign = i;
								}
							}

						else if (unfoldMethod == 2)
							{
							if (seedFaceList[outerFaceIndex])
								angle = 180.0f;
							if (angle > angDist)
								{
								angDist = angle;
								edgeToAlign = i;
								}
							}

						}
					}
				if (edgeToAlign != -1)
					{
					int innerFaceIndex = edgesToBeProcessed[edgeToAlign].innerFace;
					int outerFaceIndex = edgesToBeProcessed[edgeToAlign].outerFace;
					int edgeIndex = edgesToBeProcessed[edgeToAlign].edge;
					

					int connectedClusterIndex = clusterGroups[outerFaceIndex];

					seedFaceList.Set(outerFaceIndex, FALSE);

					processedClusters.Set(connectedClusterIndex);
					clustersJustProcessed.Append(1,&connectedClusterIndex);
					AlignCluster(i,connectedClusterIndex,innerFaceIndex, outerFaceIndex,edgeIndex);
					done = FALSE;
					}

//build new cluster list
				for (int j = 0; j < clustersJustProcessed.Count(); j++)
					{
					int clusterIndex = clustersJustProcessed[j];
					for (int i = 0; i < clusterList[clusterIndex]->borderData.Count(); i++)
						{
						int outerFaceIndex = clusterList[clusterIndex]->borderData[i].outerFace;
						int connectedClusterIndex = clusterGroups[outerFaceIndex];
						if ((!processedClusters[connectedClusterIndex]) && (connectedClusterIndex != 0) && (connectedClusterIndex != -1))
							{
							edgesToBeProcessed.Append(1,&clusterList[clusterIndex]->borderData[i]);
							}
						}
					}
				}
			}

		vsel.SetSize(TVMaps.v.Count());
		vsel.ClearAll();
		for (i = 0; i < clusterList.Count(); i++)
			{
			for (int j =0; j < clusterList[i]->faces.Count(); j++)
				{
				int faceIndex = clusterList[i]->faces[j];
				for (int k =0; k < TVMaps.f[faceIndex]->count; k++)
					{
					int vertexIndex = TVMaps.f[faceIndex]->t[k];
					vsel.Set(vertexIndex);
					}
				}
			}
//now weld the verts
		if (normalize)
			{
			NormalizeCluster();
			}

		float tempWeld = weldThreshold;
		weldThreshold = 0.001f;
		WeldSelected(FALSE);
		weldThreshold = tempWeld;


		}

	


	FreeClusterList();

	if (bContinue)
		{	
		theHold.Accept(_T(GetString(IDS_PW_PLANARMAP)));
		theHold.SuperAccept(_T(GetString(IDS_PW_PLANARMAP)));

		fnSelectPolygonsUpdate(&holdPolySel, FALSE);
		theHold.Suspend();
		fnSyncTVSelection();
		theHold.Resume();
		}
	else
		{
		theHold.Cancel();
		theHold.SuperCancel();
		}

	RebuildEdges();

	theHold.Suspend();
	fnSyncGeomSelection();
	theHold.Resume();


	NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
	InvalidateView();


	}

void UnwrapMod::AlignCluster(int baseCluster, int moveCluster, int innerFaceIndex, int outerFaceIndex,int edgeIndex)
	{
//get edges that are coincedent
	int vInner[2];
	int vOuter[2];

	int vInnerVec[2];
	int vOuterVec[2];


	int ct = 0;
	int vct = 0;
	for (int i = 0; i < TVMaps.f[innerFaceIndex]->count; i++)
		{
		int innerIndex = TVMaps.f[innerFaceIndex]->v[i];
		for (int j = 0; j < TVMaps.f[outerFaceIndex]->count; j++)
			{
			int outerIndex = TVMaps.f[outerFaceIndex]->v[j];
			if (innerIndex == outerIndex)
				{
				vInner[ct] = TVMaps.f[innerFaceIndex]->t[i];


				vOuter[ct] = TVMaps.f[outerFaceIndex]->t[j];
				ct++;
				}

			}

		}

	vInnerVec[0] = -1;
	vInnerVec[1] = -1;
	vOuterVec[0] = -1;
	vOuterVec[1] = -1;
	ct = 0;

	if ( (TVMaps.f[innerFaceIndex]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[innerFaceIndex]->vecs) &&
		 (TVMaps.f[outerFaceIndex]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[outerFaceIndex]->vecs) 
		)
		{
		for (i = 0; i < TVMaps.f[innerFaceIndex]->count*2; i++)
			{
			int innerIndex = TVMaps.f[innerFaceIndex]->vecs->vhandles[i];
			for (int j = 0; j < TVMaps.f[outerFaceIndex]->count*2; j++)
				{
				int outerIndex = TVMaps.f[outerFaceIndex]->vecs->vhandles[j];
				if (innerIndex == outerIndex)
					{
					int vec = TVMaps.f[innerFaceIndex]->vecs->handles[i];
					vInnerVec[ct] = vec;


					vec = TVMaps.f[outerFaceIndex]->vecs->handles[j];
					vOuterVec[ct] = vec;
					ct++;
					}

				}

			}
		}



//get  align vector
	Point3 pInner[2];
	Point3 pOuter[2];

	pInner[0] = TVMaps.v[vInner[0]].p;
	pInner[1] = TVMaps.v[vInner[1]].p;

	pOuter[0] = TVMaps.v[vOuter[0]].p;
	pOuter[1] = TVMaps.v[vOuter[1]].p;

	Point3 offset = pInner[0] - pOuter[0];

	Point3 vecA, vecB;
	vecA = Normalize(pInner[1] - pInner[0]);
	vecB = Normalize(pOuter[1] - pOuter[0]);
	float dot = DotProd(vecA,vecB);

	float angle = 0.0f;
	if (dot == -1.0f)
		angle = PI;
	else if (dot == 1.0f)
		angle = 0.f;
	else angle = acos(dot);

	if ((_isnan(angle)) || (!_finite(angle)))
		angle = 0.0f;
//		DebugPrint("Stop\n");

//	angle = acos(dot);
	
//DebugPrint("angle %f dot %f \n",angle, dot);
/*
DebugPrint("   VecA %f %f %f \n",vecA.x,vecA.y,vecA.z);
DebugPrint("   VecB %f %f %f \n",vecB.x,vecB.y,vecB.z);
*/

 
	Matrix3 tempMat(1);	
	tempMat.RotateZ(angle); 
	Point3 vecC = VectorTransform(tempMat,vecB);




	float negAngle = -angle;
	Matrix3 tempMat2(1);	
	tempMat2.RotateZ(negAngle); 
	Point3 vecD = VectorTransform(tempMat2,vecB);

	float la,lb;
	la = Length(vecA-vecC);
	lb = Length(vecA-vecD);
	if (la > lb)
		angle = negAngle;
	
	clusterList[moveCluster]->newX = offset.x;
	clusterList[moveCluster]->newY = offset.y;
//build vert list
//move those verts
	BitArray processVertList;
	processVertList.SetSize(TVMaps.v.Count());
	processVertList.ClearAll();
	for (i =0; i < clusterList[moveCluster]->faces.Count(); i++)
		{
		int faceIndex = clusterList[moveCluster]->faces[i];
		for (int j =0; j < TVMaps.f[faceIndex]->count; j++)
			{
			int vertexIndex = TVMaps.f[faceIndex]->t[j];
			processVertList.Set(vertexIndex);


			if ( (objType == IS_PATCH) && (TVMaps.f[faceIndex]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[faceIndex]->vecs))
				{
				int vertIndex;
					
				if (TVMaps.f[faceIndex]->flags & FLAG_INTERIOR)
					{
					vertIndex = TVMaps.f[faceIndex]->vecs->interiors[j];
					if ((vertIndex >=0) && (vertIndex < processVertList.GetSize()))
						processVertList.Set(vertIndex);
					}

				vertIndex = TVMaps.f[faceIndex]->vecs->handles[j*2];
				if ((vertIndex >=0) && (vertIndex < processVertList.GetSize()))
					processVertList.Set(vertIndex);
				vertIndex = TVMaps.f[faceIndex]->vecs->handles[j*2+1];
				if ((vertIndex >=0) && (vertIndex < processVertList.GetSize()))
					processVertList.Set(vertIndex);
	
				}


			}
		}
	for (i = 0; i < processVertList.GetSize(); i++)
		{
		if (processVertList[i])
			{
//DebugPrint("%d ",i);
			Point3 p = TVMaps.v[i].p;
//move to origin

			p -= pOuter[0];

//rotate
			Matrix3 mat(1);	

			mat.RotateZ(angle); 

		 	p = p * mat;
//move to anchor point			
			p += pInner[0];

			TVMaps.v[i].p = p;
			if (TVMaps.cont[i]) TVMaps.cont[i]->SetValue(0,&TVMaps.v[i].p);

			}
		}

	if ((vInnerVec[0] != -1) &&	(vInnerVec[1] != -1) && (vOuterVec[0] != -1) && (vOuterVec[1] != -1))
		{
		TVMaps.v[vOuterVec[0]].p = TVMaps.v[vInnerVec[0]].p;
		if (TVMaps.cont[vOuterVec[0]]) TVMaps.cont[vOuterVec[0]]->SetValue(0,&TVMaps.v[vInnerVec[0]].p);

		TVMaps.v[vOuterVec[1]].p = TVMaps.v[vInnerVec[1]].p;
		if (TVMaps.cont[vOuterVec[1]]) TVMaps.cont[vOuterVec[1]]->SetValue(0,&TVMaps.v[vInnerVec[1]].p);

		}



	}


void	UnwrapMod::fnUnfoldSelectedPolygonsNoParams()
	{
	fnUnfoldSelectedPolygons(unfoldMethod, unfoldNormalize);
	}		

void	UnwrapMod::fnUnfoldSelectedPolygonsDialog()
	{
//bring up the dialog
	DialogBoxParam(	hInstance,
							MAKEINTRESOURCE(IDD_UNFOLDDIALOG),
							GetCOREInterface()->GetMAXHWnd(),
//							hWnd,
							UnwrapUnfoldFloaterDlgProc,
							(LPARAM)this );


	}

void	UnwrapMod::SetUnfoldDialogPos()
	{
	if (unfoldWindowPos.length != 0) 
		SetWindowPlacement(unfoldHWND,&unfoldWindowPos);
	}

void	UnwrapMod::SaveUnfoldDialogPos()
	{
	unfoldWindowPos.length = sizeof(WINDOWPLACEMENT); 
	GetWindowPlacement(unfoldHWND,&unfoldWindowPos);
	}



INT_PTR CALLBACK UnwrapUnfoldFloaterDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	UnwrapMod *mod = (UnwrapMod*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	//POINTS p = MAKEPOINTS(lParam);	commented out by sca 10/7/98 -- causing warning since unused.

	switch (msg) {
		case WM_INITDIALOG:
			{

			mod = (UnwrapMod*)lParam;
			mod->unfoldHWND = hWnd;

			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);

			HWND hMethod = GetDlgItem(hWnd,IDC_METHOD_COMBO);
			SendMessage(hMethod, CB_RESETCONTENT, 0, 0);

			SendMessage(hMethod, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_FARTHESTFACE));
			SendMessage(hMethod, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_CLOSESTFACE));

			SendMessage(hMethod, CB_SETCURSEL, mod->unfoldMethod, 0L);

//set normalize cluster
			CheckDlgButton(hWnd,IDC_NORMALIZE_CHECK,mod->unfoldNormalize);

			break;
			}


		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_OK:
					{
					mod->SaveUnfoldDialogPos();


					BOOL tempNormalize;
					int tempMethod;

					tempNormalize = mod->unfoldNormalize;
					tempMethod = mod->unfoldMethod;

					HWND hMethod = GetDlgItem(hWnd,IDC_METHOD_COMBO);
					mod->unfoldMethod = SendMessage(hMethod, CB_GETCURSEL, 0L, 0);


					mod->unfoldNormalize = IsDlgButtonChecked(hWnd,IDC_NORMALIZE_CHECK);

					mod->fnUnfoldSelectedPolygonsNoParams();

					mod->unfoldNormalize = tempNormalize;
					mod->unfoldMethod = tempMethod;


					EndDialog(hWnd,1);
					
					break;
					}
				case IDC_CANCEL:
					{
				
					mod->SaveFlattenDialogPos();

					EndDialog(hWnd,0);

					break;
					}
				case IDC_DEFAULT:
					{

//get align
					mod->unfoldNormalize = IsDlgButtonChecked(hWnd,IDC_NORMALIZE_CHECK);
					HWND hMethod = GetDlgItem(hWnd,IDC_METHOD_COMBO);
					mod->unfoldMethod = SendMessage(hMethod, CB_GETCURSEL, 0L, 0);

					break;
					}

				}
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}




void	UnwrapMod::fnHideSelectedPolygons()
	{
	if (hiddenPolygons.GetSize() != TVMaps.f.Count() )
		{
		hiddenPolygons.SetSize(TVMaps.f.Count());
		}
	//check for type
	ModContextList mcList;		
	INodeTab nodes;

	if (!ip) return;
	ip->GetModContexts(mcList,nodes);

	int objects = mcList.Count();

	


	if (objects != 0)
		{

		
		MeshTopoData *md = (MeshTopoData*)mcList[0]->localData;	

		if (md == NULL) 
			{
			return;
			}


		hiddenPolygons |= md->faceSel;
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		ip->RedrawViews(ip->GetTime());
		InvalidateView();
		}

	}
void	UnwrapMod::fnUnhideAllPolygons()
	{
	if (hiddenPolygons.GetSize() != TVMaps.f.Count() )
		{
		hiddenPolygons.SetSize(TVMaps.f.Count());
		}

	hiddenPolygons.ClearAll();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
	InvalidateView();
	}

void	UnwrapMod::fnSetSeedFace()
	{

	seedFaces.ZeroCount();
	for (int i =0; i < TVMaps.f.Count(); i++)
		{
		if (TVMaps.f[i]->flags & FLAG_SELECTED)
			seedFaces.Append(1,&i);

		}
		
	}
void UnwrapMod::fnShowVertexConnectionList()
	{
	if (showVertexClusterList)
		showVertexClusterList = FALSE;
	else showVertexClusterList = TRUE;

	NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
	InvalidateView();

	}


BOOL	UnwrapMod::fnGetShowConnection()
	{
	return showVertexClusterList;
	}

void	UnwrapMod::fnSetShowConnection(BOOL show)
	{
	showVertexClusterList = show;
	NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
	InvalidateView();
	}




