
#include "painterInterface.h"



void LightMesh::AddMesh(Mesh *m,  Matrix3 basetm,  ViewExp *vpt, BOOL buildVNorms)
	{


	//copy over the vert and face data
	vertsViewSpace.SetCount(m->numVerts);
	for (int i =0; i < m->numVerts; i++)
		vertsViewSpace[i] = m->verts[i];
	
	vertsWorldSpace = vertsViewSpace;
	
	BOOL flip = basetm.Parity();//check to see if the mesh has been mirrored if so flip stuff;
	faces.SetCount(m->numFaces);
	for (i =0; i < m->numFaces; i++)
		{
		faces[i] = m->faces[i];
		if (flip)
			{
			int a = faces[i].v[0];
			int b = faces[i].v[1];
			int c = faces[i].v[2];
			faces[i].v[0] = c;
			faces[i].v[1] = b;
			faces[i].v[2] = a;
			}
		}

	toWorldSpace = basetm;
	toLocalSpace = Inverse(basetm);

	for (i =0; i < m->numVerts; i++)
		vertsWorldSpace[i] = vertsWorldSpace[i]*basetm;


	bb.Init();

//build vnorms and fnorms
	Mesh *mesh;
	mesh = m;
	Face *face;	
	Point3 v0, v1, v2;


//	face = m->faces;
	face = faces.Addr(0);
	

//	fnorms.SetCount(m->getNumFaces());

	GraphicsWindow *gw = vpt->getGW();

	Matrix3 tm;
	vpt->GetAffineTM(tm);
	
	gw->setTransform(Matrix3(1));

	Point3 *verts = vertsViewSpace.Addr(0);
	int vertCount = vertsViewSpace.Count();
	for (i =0; i < vertCount; i++)
		{
		Point3 inPoint,outPoint;
		inPoint = *verts * basetm;
		DWORD flag = gw->transPoint(&inPoint, &outPoint);
		inPoint = inPoint * tm;
		*verts = outPoint ;
		(*verts).z = inPoint.z;
		bb += *verts;
		verts++;
		}

	// Compute face and vertex surface normals
	faceVisible.SetSize(m->getNumFaces());
	faceVisible.ClearAll();
	for (i = 0; i < m->getNumFaces(); i++, face++) 
		{

// Calculate the surface normal
		Point3 norm;
		v0 = vertsViewSpace[face->v[0]];
		v1 = vertsViewSpace[face->v[1]];
		v2 = vertsViewSpace[face->v[2]];
		norm = (v1-v0)^(v2-v1);
		if (norm.z < 0.0f)
			{
			if (face->Hidden())
				faceVisible.Set(i,FALSE);
			else faceVisible.Set(i,TRUE);
			}
		else faceVisible.Set(i,FALSE);
			
		}
	if (buildVNorms)
		{
		face = faces.Addr(0);
		Tab<int> normCount;
		normCount.SetCount(m->numVerts);
		vnorms.SetCount(m->numVerts);
		for (i = 0; i < m->numVerts; i++) 
			{
			vnorms[i] = Point3(0.0f,0.0f,0.0f);
			normCount[i] = 0;
			}
		for (i = 0; i < mesh->getNumFaces(); i++, face++) 
			{

// Calculate the surface normal
			v0 = vertsWorldSpace[face->v[0]];
			v1 = vertsWorldSpace[face->v[1]];
			v2 = vertsWorldSpace[face->v[2]];
			
			for (int j=0; j<3; j++) 
				{		
				vnorms[face->v[j]]+=Normalize((v1-v0)^(v2-v1));
				normCount[face->v[j]]++;
				}
			
			}
		for (i = 0; i < m->numVerts; i++) 
			{
			if (normCount[i]!=0)
				vnorms[i] = Normalize(vnorms[i]/(float)normCount[i]);
			 
			}		
		
		}
	else
		{
		vnorms.ZeroCount();
		vnorms.Resize(0);
		}

	}

void BoundsTree::DeleteTree(Leaf *l)
	{
	Leaf *tlist[4];
	for (int i = 0; i < 4; i++)
		{
		tlist[i] = l->GetQuad(i);
		}
	delete l;
	l = NULL;

	for (i = 0; i < 4; i++)
		{
		if (tlist[i])
			DeleteTree(tlist[i]);
		}
	}

void BoundsTree::LayoutTree(Leaf *l, int depth)
	{
	depth--;
	if (depth == 0) 
		{
		l->SetBottom(TRUE);
		return;
		}

	
	for (int i =0; i < 4; i++)
		{
		l->SetQuad(i, new Leaf(l->GetBounds(i).min,l->GetBounds(i).max));
		}

	LayoutTree(l->GetQuad(0),depth);
	LayoutTree(l->GetQuad(1),depth);
	LayoutTree(l->GetQuad(2),depth);
	LayoutTree(l->GetQuad(3),depth);

	}



void BoundsTree::RecurseTree(Leaf *l, int index, Box2D box, int nodeID)

{
	if (l->IsBottom())
		{
		BoundsList temp;//move this to top no need to recurse if it is a back face
//		Point3 norm = meshList[nodeID]->fnorms[index];
		if (meshList[nodeID]->faceVisible[index])
			{
			temp.faceIndex = index;
			temp.nodeIndex = nodeID;
			l->faceIndex.Append(1,&temp,10);
			}
		}
	else
		{
		if ((l->GetQuad(0)) && (l->InQuad(0,box))) 
			RecurseTree(l->GetQuad(0),index,box,nodeID);
		if ((l->GetQuad(1)) && (l->InQuad(1,box))) 
			RecurseTree(l->GetQuad(1),index,box,nodeID);
		if ((l->GetQuad(2)) && (l->InQuad(2,box))) 
			RecurseTree(l->GetQuad(2),index,box,nodeID);
		if ((l->GetQuad(3)) && (l->InQuad(3,box))) 
			RecurseTree(l->GetQuad(3),index,box,nodeID);

		}
}

void BoundsTree::AddFace(int index, Box2D box, int nodeID)
	{
//if face is not visible no need to add it to the tree

	if ((head) && (meshList[nodeID]->faceVisible[index]))
		{
		if ((head->GetQuad(0)) && (head->InQuad(0,box))) 
			RecurseTree(head->GetQuad(0),index,box,nodeID);
		if ((head->GetQuad(1)) && (head->InQuad(1,box))) 
			RecurseTree(head->GetQuad(1),index,box,nodeID);
		if ((head->GetQuad(2)) && (head->InQuad(2,box))) 
			RecurseTree(head->GetQuad(2),index,box,nodeID);
		if ((head->GetQuad(3)) && (head->InQuad(3,box))) 
			RecurseTree(head->GetQuad(3),index,box,nodeID);
		}	
	}



BOOL BoundsTree::TrimRecurseTree(Leaf *l)
	{
//if is bottom and empty return TRUE
	if (l->IsBottom() && l->faceIndex.Count() ==0)
		return TRUE;
	else if (l->IsBottom() && l->faceIndex.Count() !=0)
		return FALSE;
	else if ( TrimRecurseTree(l->GetQuad(0)) &&
		 TrimRecurseTree(l->GetQuad(1)) &&
		 TrimRecurseTree(l->GetQuad(2)) &&
		 TrimRecurseTree(l->GetQuad(3)) )
		{
//DebugPrint("Leaf trimmed\n");
		Leaf *deadLeaf1 = l->GetQuad(0);
		Leaf *deadLeaf2 = l->GetQuad(1);
		Leaf *deadLeaf3 = l->GetQuad(2);
		Leaf *deadLeaf4 = l->GetQuad(3);

		delete deadLeaf1;
		delete deadLeaf2;
		delete deadLeaf3;
		delete deadLeaf4;
		l->SetQuad(0, NULL);
		l->SetQuad(1, NULL);
		l->SetQuad(2, NULL);
		l->SetQuad(3, NULL);
		l->SetBottom(TRUE);

		if (l->faceIndex.Count() ==0)
			return TRUE;
		else return FALSE;
		}
	else return FALSE;

	}
void BoundsTree::TrimTree()
	{
	TrimRecurseTree(head);
	}

void BoundsTree::BuildMeshData(Mesh *m, Matrix3 basetm,ViewExp *vpt, int index)
	{


	int meshCount = meshList.Count();
	if (index >= meshCount)
		{
		int ct = meshList.Count();
		for (int i = ct; i < (index+1); i++)
			{
			LightMesh *tempMesh = NULL;
			meshList.Append(1,&tempMesh);
			}
		meshList[index] = new LightMesh();
		}
	else
		{
		if (meshList[index]== NULL)
			meshList[index] = new LightMesh();
		}

	meshList[index]->AddMesh(m,basetm,vpt, buildVNorms);
	}


void BoundsTree::BuildQuadTree()
	{

	//layout tree based on depth
	Point2 min;
	min.x = 0.0f;
	min.y = 0.0f;
	Point2 max;
	max.x = 1.0f;
	max.y = 1.0f;

	Box3 bb;
	bb.Init();

	for (int i  = 0; i < meshList.Count(); i++)
		{
		if (meshList[i])
			{
			bb += meshList[i]->bb;
			}
		}

	min.x = bb.pmin.x;
	min.y = bb.pmin.y;

	max.x = bb.pmax.x;
	max.y = bb.pmax.y;

	head = new Leaf(min,max);
	LayoutTree(head, depth);


//now stuff the points into the boudningbox list
	for (int m  = 0; m < meshList.Count(); m++)
		{
		if (meshList[m])
			{
			meshList[m]->boundingBoxList.SetCount(meshList[m]->faces.Count());
			Face *faces = meshList[m]->faces.Addr(0);
			for (i =0; i < meshList[m]->faces.Count(); i++)
				{
				meshList[m]->boundingBoxList[i].SetEmpty();
				for (int j = 0; j < 3; j++)
					{
					int vid = (*faces).v[j];
					Point3 p = meshList[m]->vertsViewSpace[vid];
					Point2 p2;
					p2.x = p.x;		
					p2.y = p.y;
					meshList[m]->boundingBoxList[i] += p2;

					}
				faces++;
				}

	//now go through our tree adding the bounding list 
			for (i =0; i < meshList[m]->faces.Count(); i++)
				{
				AddFace(i,meshList[m]->boundingBoxList[i],m);
				}
			}
		}

	}


#define EPSILON 0.000001f


BOOL BoundsTree::IsInsideMesh(Point3 hitPoint)
{

Leaf *l = head;
int ct = 0;
BOOL hit = FALSE;
if (l == NULL) 
	{
	return FALSE;
	}

while ( (l!=NULL) && (l->IsBottom() == FALSE))
	{
	int id = l->InWhichQuad(hitPoint);
	l = l->GetQuad(id);
	ct++;
	}

ct = 0;

if (l)
	{
	if (l->faceIndex.Count() == 0) 
		return FALSE;
	else
		{
		for (int i = 0; i < l->faceIndex.Count(); i++)
			{
			int faceIndex = l->faceIndex[i].faceIndex;
			int nodeIndex = l->faceIndex[i].nodeIndex;
			Box2D b = meshList[nodeIndex]->boundingBoxList[faceIndex];
			if ( (hitPoint.x >= b.min.x) && (hitPoint.x <= b.max.x) &&
				 (hitPoint.y >= b.min.y) && (hitPoint.y <= b.max.y) )
				{
//now check bary coords
				Point3 a,b,c;
				a = meshList[nodeIndex]->vertsViewSpace[meshList[nodeIndex]->faces[faceIndex].v[0]];
				b = meshList[nodeIndex]->vertsViewSpace[meshList[nodeIndex]->faces[faceIndex].v[1]];
				c = meshList[nodeIndex]->vertsViewSpace[meshList[nodeIndex]->faces[faceIndex].v[2]];
				Point3 az,bz,cz,hitPointZ;
				az = a;
				bz = b;
				cz = c;
				az.z = 0.0f;
				bz.z = 0.0f;
				cz.z = 0.0f;
				hitPointZ = hitPoint;
				hitPointZ.z = 0.0f;
	
				Point3 bry;
				bry = BaryCoords(az, bz, cz, hitPointZ);
//if inside bary find the the z point			
				if (!( (bry.x<0.0f || bry.x>1.0f || bry.y<0.0f || bry.y>1.0f || bry.z<0.0f || bry.z>1.0f) ||
					(fabs(bry.x + bry.y + bry.z - 1.0f) > EPSILON) ))
					{
					float tz = a.z * bry.x + b.z * bry.y + c.z * bry.z; 
					if  (tz < hitPoint.z )  ct++;
					
					}
				}
			}
		}	
	}
if ((ct%2) == 1) return TRUE;
else return FALSE;

}


BOOL BoundsTree::HitQuadTree(IPoint2 m, int &nindex, DWORD &findex, Point3 &p, Point3 &norm, Point3 &bary, float &finalZ, Matrix3 &toWorldTm)
{
//int nindex = 0;
Leaf *l = head;
BOOL hit = FALSE;
Point3 hitPoint(0.0f,0.0f,0.0f);
DWORD smgroup;

hitPoint.x = (float) m.x;
hitPoint.y = (float) m.y;
float z = 0.0f;
Point3 bry;
if (l == NULL) 
	{
	z = 0.0f;
	return FALSE;
	}
int ct = 0;
while ( (l!=NULL) && (l->IsBottom() == FALSE))
	{
	int id = l->InWhichQuad(hitPoint);
	l = l->GetQuad(id);
	ct++;
	}
if (l)
	{
	if (l->faceIndex.Count() == 0) 
		return FALSE;
	else
		{
		for (int i = 0; i < l->faceIndex.Count(); i++)
			{
			int faceIndex = l->faceIndex[i].faceIndex;
			int nodeIndex = l->faceIndex[i].nodeIndex;

			LightMesh *lmesh = meshList[nodeIndex];

			Point3 *tempVerts = lmesh->vertsViewSpace.Addr(0);
			Face *tempFaces = lmesh->faces.Addr(faceIndex);

			Box2D b = lmesh->boundingBoxList[faceIndex];
			if ( (hitPoint.x >= b.min.x) && (hitPoint.x <= b.max.x) &&
				 (hitPoint.y >= b.min.y) && (hitPoint.y <= b.max.y) )
				{
//now check bary coords
				Point3 a,b,c;
				a = tempVerts[tempFaces->v[0]];
				b = tempVerts[tempFaces->v[1]];
				c = tempVerts[tempFaces->v[2]];
				Point3 az,bz,cz,hitPointZ;
				az = a;
				bz = b;
				cz = c;
				az.z = 0.0f;
				bz.z = 0.0f;
				cz.z = 0.0f;
				hitPointZ = hitPoint;
				hitPointZ.z = 0.0f;
	
				Point3 bry;
				bry = BaryCoords(az, bz, cz, hitPointZ);
//if inside bary find the the z point			
				if (!( (bry.x<0.0f || bry.x>1.0f || bry.y<0.0f || bry.y>1.0f || bry.z<0.0f || bry.z>1.0f) ||
					(fabs(bry.x + bry.y + bry.z - 1.0f) > EPSILON) ))
					{
					float tz = a.z * bry.x + b.z * bry.y + c.z * bry.z; 
					if ( (tz > z ) || (hit == FALSE) )  
						{
						z = tz;
						findex = faceIndex;
						nindex = nodeIndex;
						bary = bry;
						finalZ = z;
						smgroup = tempFaces->getSmGroup();
						}
					hit = TRUE;
					}
				}
			}
		}	
	}
if (hit)
	{
	Point3 a,b,c;
	int ia,ib,ic;
	LightMesh *lmesh = meshList[nindex];
	Point3 *tempVerts = lmesh->vertsWorldSpace.Addr(0);
	Face *tempFaces = lmesh->faces.Addr(findex);

	ia = tempFaces->v[0];
	ib = tempFaces->v[1];
	ic = tempFaces->v[2];
	a = tempVerts[ia];
	b = tempVerts[ib];
	c = tempVerts[ic];

	ViewExp *vpt = GetCOREInterface()->GetActiveViewport();

	Ray worldRay;
//	vpt->GetAffineTM(tm);
	vpt->MapScreenToWorldRay((float) m.x, (float) m.y, worldRay);
	GetCOREInterface()->ReleaseViewport(vpt);

//intersect ray with the hit face

		// See if the ray intersects the plane (backfaced)
	norm = Normalize((b-a)^(c-b));

	float rn = DotProd(worldRay.dir,norm);
		
	// Use a point on the plane to find d
	float d = DotProd(a,norm);

	// Find the point on the ray that intersects the plane
	float ta = (d - DotProd(worldRay.p,norm)) / rn;


		// The point on the ray and in the plane.
	p = worldRay.p + ta*worldRay.dir;

		// Compute barycentric coords.
	bary = BaryCoords(a, b, c, p);

	finalZ  = ta;

//	p = a * bary.x +  b * bary.y +  c * bary.z;
	p = p * meshList[nindex]->toLocalSpace;

	norm = VectorTransform(meshList[nindex]->toLocalSpace,norm);
	toWorldTm = meshList[nindex]->toWorldSpace;

	


	}

return hit;

}

Point3 BoundsTree::GetWorldCenter()
{
	Point3 center (0.0f,0.0f,0.0f);
	
	worldBB.Init();
	for (int m  = 0; m < meshList.Count(); m++)
		{
		if (meshList[m])
			{
			Point3 *verts = meshList[m]->vertsWorldSpace.Addr(0);
			for (int i =0; i < meshList[m]->vertsWorldSpace.Count(); i++)
				{
				Point3 p = *verts;
				worldBB +=  p;
				verts++;
				}
			}
		}
	center = worldBB.Center();
	return center;
}

Box3  BoundsTree::GetWorldBounds()
{
return worldBB;
}

Point3 *BoundsTree::GetWorldData(int index, int &count)
{
count = meshList[index]->vertsWorldSpace.Count();
if (count == 0)
	return NULL;
else return meshList[index]->vertsWorldSpace.Addr(0);
}


Point3 *BoundsTree::GetNormals(int index, int &count)
{
count = meshList[index]->vnorms.Count();
if (count == 0)
	return NULL;
else return meshList[index]->vnorms.Addr(0);
}



