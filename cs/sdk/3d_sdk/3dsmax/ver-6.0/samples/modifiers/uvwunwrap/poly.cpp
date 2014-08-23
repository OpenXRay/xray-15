#include "unwrap.h"

void UnwrapMod::BuildInitialMapping(MNMesh *msh)
	{

//build bounding box
	Box3 bbox;
	bbox.Init();

	int vertCount = 0;
//normalize the length width height
	for (int i = 0; i < TVMaps.f.Count(); i++)
		{
		int pcount = 3;
		pcount = TVMaps.f[i]->count;
		vertCount += pcount;
		for (int j = 0; j < pcount; j++)
			{
			bbox += TVMaps.geomPoints[TVMaps.f[i]->v[j]];
			}

		}

	Tab<int> indexList;

	indexList.SetCount(vertCount);
	BitArray usedIndex;
	usedIndex.SetSize(vertCount);
	usedIndex.ClearAll();

	for (i = 0; i < vertCount; i++)
		indexList[i] = -1;

	for (i = 0; i < TVMaps.f.Count(); i++)
		{
		if (!(TVMaps.f[i]->flags & FLAG_DEAD))
			{
			int pcount = 3;
//		if (TVMaps.f[i].flags & FLAG_QUAD) pcount = 4;
			pcount = TVMaps.f[i]->count;
			for (int j = 0; j < pcount; j++)
				{
				usedIndex.Set(msh->f[i].vtx[j]);
				}
			}

		}

	int ct = 0;
	for (i = 0; i < usedIndex.GetSize(); i++)
		{
		if (usedIndex[i])
			indexList[i] = ct++;

		}

	TVMaps.v.SetCount(usedIndex.NumberSet());
	TVMaps.cont.SetCount(usedIndex.NumberSet());
	vsel.SetSize(usedIndex.NumberSet());

//watje 10-19-99 bug 213437  to prevent a divide by 0 which gives you a huge u,v, or w value
	if (bbox.Width().x == 0.0f) bbox += Point3(0.5f,0.0f,0.0f);
	if (bbox.Width().y == 0.0f) bbox += Point3(0.0f,0.5f,0.0f);
	if (bbox.Width().z == 0.0f) bbox += Point3(0.0f,0.0f,0.5f);

	for (i = 0; i < TVMaps.f.Count(); i++)
		{
		if (!(TVMaps.f[i]->flags & FLAG_DEAD))
			{
			int pcount = 3;
//		if (TVMaps.f[i].flags & FLAG_QUAD)	pcount = 4;
			pcount = TVMaps.f[i]->count;
			TVMaps.f[i]->flags &= ~FLAG_DEAD;
			for (int j = 0; j < pcount; j++)
				{
				int index;
				int a = msh->f[i].vtx[j];
				index = indexList[a];
				TVMaps.f[i]->t[j] = index;
				TVMaps.v[index].p.x =  TVMaps.geomPoints[TVMaps.f[i]->v[j]].x/bbox.Width().x + 0.5f;
				TVMaps.v[index].p.y =  TVMaps.geomPoints[TVMaps.f[i]->v[j]].y/bbox.Width().y + 0.5f;
				TVMaps.v[index].p.z =  TVMaps.geomPoints[TVMaps.f[i]->v[j]].z/bbox.Width().z + 0.5f;
				TVMaps.v[index].influence =  0.f;
				TVMaps.v[index].flags =  0.f;
				TVMaps.cont[index] = NULL;
		
				}

			}
		}

	}

void UnwrapMod::GetFaceSelectionFromMNMesh(ObjectState *os, ModContext &mc, TimeValue t)
	{
		PolyObject *tobj = (PolyObject*)os->obj;

		MeshTopoData *d  = (MeshTopoData*)mc.localData;
		if (d) 
			{
			BitArray s;
			tobj->GetMesh().getFaceSel(s);
			d->SetFaceSel(s, this, t);
			UpdateFaceSelection(d->faceSel);
			}

	}


void UnwrapMod::CopySelectionMNMesh(ObjectState *os, ModContext &mc, int CurrentChannel, TimeValue t)
	{	

		objType = IS_MNMESH;
		PolyObject *tobj = (PolyObject*)os->obj;

		MeshTopoData *d  = (MeshTopoData*)mc.localData;
		if (!d) 
			{
			mc.localData = d = new MeshTopoData(tobj->GetMesh());
			BitArray s;
			tobj->GetMesh().getFaceSel(s);
			d->SetFaceSel(s, this, t);
			UpdateFaceSelection(d->faceSel);

			}
		if ( ((editMod==this) && (!d->GetMNMesh())) || (updateCache))
			{
			d->SetCache(tobj->GetMesh());
			BitArray s;
			tobj->GetMesh().getFaceSel(s);
			d->SetFaceSel(s, this, t);
			updateCache = FALSE;
			UpdateFaceSelection(d->faceSel);
			SyncTVToGeomSelection(d);

			hiddenPolygons.SetSize(tobj->GetMesh().numf);
			hiddenPolygons.ClearAll();
			for (int i = 0; i < tobj->GetMesh().numf; i++)
				{
				if (tobj->GetMesh().f[i].GetFlag(MN_HIDDEN))
					hiddenPolygons.Set(i,TRUE);
				}
			}

		BitArray faceSel = d->faceSel;


		faceSel.SetSize(tobj->GetMesh().FNum(),TRUE);

		if ( (ip && (ip->GetSubObjectLevel() > 0) ))
			{

			tobj->GetMesh().FaceSelect(faceSel);

			if (showVerts)
				{
//select verts based on the current tverts;
				BitArray vertSel;
				vertSel.SetSize(tobj->GetMesh().VNum(),TRUE);
				vertSel.ClearAll();
				for(int sv = 0; sv < TVMaps.f.Count();sv++)
					{
					if (!(TVMaps.f[sv]->flags & FLAG_DEAD))
						{
						int pcount = 3;
//						if (TVMaps.f[sv].flags & FLAG_QUAD) pcount = 4;
						pcount = TVMaps.f[sv]->count;
						for (int j = 0; j < pcount ; j++)
							{
							int index = TVMaps.f[sv]->t[j];
//6-29--99 watje
							if ((index < vsel.GetSize()) && (vsel[index] ==1) && (sv<tobj->GetMesh().FNum()))
//							if (vsel[index] ==1)
								{
								int findex = tobj->GetMesh().f[sv].vtx[j];
//6-29--99 watje
								if ((findex < vertSel.GetSize()) && (findex >=0))
									vertSel.Set(findex,1);
								}
							}
						}
					}
				tobj->GetMesh().VertexSelect(vertSel);
				tobj->GetMesh().SetDispFlag(MNDISP_SELFACES|MNDISP_VERTTICKS|MNDISP_SELVERTS);
//done++;
				}
			else
				{
				tobj->GetMesh().SetDispFlag(MNDISP_SELFACES);
//done++;
				}
//UNFOLD STUFF
			MNFace *faces = tobj->GetMesh().f;
			for (int i =0; i < tobj->GetMesh().numf; i++)
				{
				if ( (i < hiddenPolygons.GetSize()) && (hiddenPolygons[i]) ) 
					faces[i].SetFlag(MN_HIDDEN);
				else faces[i].ClearFlag(MN_HIDDEN);
				}

			}

		
		if (!tmControl || (flags&CONTROL_OP) || (flags&CONTROL_INITPARAMS)) 
			InitControl(t);


//if planar mode build vert and face list

		if ( (ip && (ip->GetSubObjectLevel() == 1) ))
			{
			PolyUpdateGData(&tobj->GetMesh(), faceSel);
	
			}


	}



void UnwrapMod::PolyUpdateGData(MNMesh *mnMesh, BitArray faceSel)
	{

	int count = 0;
	for (int i = 0; i < mnMesh->numf; i++)
		count += mnMesh->f[i].deg;
	gverts.d.SetCount(count);

	gverts.sel.SetSize(count,1);

	gverts.sel.ClearAll();

	MNFace *tf = mnMesh->f;
	MNVert *tp = mnMesh->v;

//isolate a vertex list of just the selected faces
	for ( i = 0; i < faceSel.GetSize(); i++)
		{
		if (faceSel[i])
			{
			int ct = tf[i].deg;
			for (int j = 0; j < ct; j++)
				{
				int index = tf[i].vtx[j];
				gverts.addPoint(index, tp[index].p);
				}

			}
		}
//build new tv faces
	int ct = gfaces.Count();
	for (i =0; i < ct; i++)
		{
		if (gfaces[i]->vecs) delete gfaces[i]->vecs;
		gfaces[i]->vecs = NULL;
		if (gfaces[i]->t) delete [] gfaces[i]->t;
		gfaces[i]->t = NULL;
		if (gfaces[i]->v) delete [] gfaces[i]->v;
		gfaces[i]->v = NULL;

		delete gfaces[i];
		gfaces[i] = NULL;
		}
	gfaces.SetCount(faceSel.NumberSet());
	for (i =0; i < faceSel.NumberSet(); i++) gfaces[i] = NULL;

	ct = 0;
	for (i = 0; i < faceSel.GetSize(); i++)
		{
		if (faceSel[i])
			{
			UVW_TVFaceClass *t = new UVW_TVFaceClass;
			t->FaceIndex = i;
			t->flags = 0;
			int fct = tf[i].deg;
			t->t = new int[fct];
			t->v = new int[fct];
			t->count = fct;
			for (int j = 0; j < fct; j++)
				{
//find indes in our vert array
				t->t[j] = (int)tf[i].vtx[j];


				}


			t->flags = TVMaps.f[i]->flags;
			if (gfaces[ct]) gfaces[ct]->DeleteVec();
			if ((gfaces[ct]) && (gfaces[ct]->t)) 
				{
				delete [] gfaces[ct]->t;
				gfaces[ct]->t = NULL;
				}
			if ((gfaces[ct]) && (gfaces[ct]->v)) 
				{
				delete [] gfaces[ct]->v;
				gfaces[ct]->v = NULL;
				}
			if (gfaces[ct]) delete gfaces[ct];
			gfaces[ct++] = t;


			}
		}

	}

BOOL UnwrapMod::InitializeMNMeshData(ObjectState *os, int CurrentChannel)
	{

	PolyObject *tobj = (PolyObject*)os->obj;
				// Apply our mapping
	MNMesh &mesh = tobj->GetMesh();




//get channel from mesh
	TVMaps.channel = CurrentChannel;
				
	BitArray s;
	mesh.getFaceSel(s);

	if ( (mesh.selLevel==MNM_SL_FACE) && (s == 0) ) 
		{
		return FALSE;
		}
						
	
//get from mesh based on cahne
	int numMaps = mesh.MNum ();
	MNMapFace *tvFace=NULL;
	Point3 *tVerts = NULL;
	if (CurrentChannel >= numMaps) 
		{
		}
	else
		{
		tvFace = mesh.M(CurrentChannel)->f;
		tVerts = mesh.M(CurrentChannel)->v;
		}

	if (mesh.selLevel!=MNM_SL_FACE) 
		{
//copy into our structs
		TVMaps.SetCountFaces(mesh.FNum());
		if (tVerts)
			{
			TVMaps.v.SetCount(mesh.M(CurrentChannel)->VNum());
			TVMaps.cont.SetCount(mesh.M(CurrentChannel)->VNum());
			vsel.SetSize(mesh.M(CurrentChannel)->VNum());
			}
		else
			{
			TVMaps.v.SetCount(mesh.VNum());
			TVMaps.cont.SetCount(mesh.VNum());
			vsel.SetSize(mesh.VNum());
			}

		
		TVMaps.geomPoints.SetCount(mesh.VNum());

		for (int j=0; j<TVMaps.f.Count(); j++) 
			{
			TVMaps.f[j]->flags = 0;
			int fct;
			fct = mesh.f[j].deg;

			if (mesh.f[j].GetFlag(MN_DEAD)) fct = 0;

			TVMaps.f[j]->t = new int[fct];
			TVMaps.f[j]->v = new int[fct];
			if (tvFace == NULL)
				{
				for (int k=0; k < fct;k++)
					TVMaps.f[j]->t[k] = 0;
				TVMaps.f[j]->FaceIndex = j;
				TVMaps.f[j]->MatID = mesh.f[j].material;
				TVMaps.f[j]->flags = 0;
				TVMaps.f[j]->count = fct;
				for (k = 0; k < fct; k++)
					{
					int index = mesh.f[j].vtx[k];
					TVMaps.f[j]->v[k] = index;
					TVMaps.geomPoints[index] = mesh.v[index].p;
					}
				}
			else
				{
				for (int k=0; k < fct;k++)
					TVMaps.f[j]->t[k] = tvFace[j].tv[k];
				TVMaps.f[j]->FaceIndex = j;
				TVMaps.f[j]->MatID = mesh.f[j].material;
				

				TVMaps.f[j]->flags = 0;
				TVMaps.f[j]->count = fct;
				for (k = 0; k < fct; k++)
					{
					int index = mesh.f[j].vtx[k];
					TVMaps.f[j]->v[k] = index;
					TVMaps.geomPoints[index] = mesh.v[index].p;
					}

							
				}
			}
		for (    j=0; j<TVMaps.v.Count(); j++) 
			{
			TVMaps.cont[j] = NULL;
			TVMaps.v[j].flags = 0;
			if (tVerts)
				TVMaps.v[j].p  = tVerts[j];
			else TVMaps.v[j].p  = Point3(.0f,0.0f,0.0f);
			TVMaps.v[j].influence = 0.0f;
			}
		if (tvFace == NULL) BuildInitialMapping(&mesh);
		}
	else
		{
//copy into our structs
		TVMaps.SetCountFaces(mesh.FNum());

		int tvVertCount = 0;
		if (CurrentChannel < numMaps) 
			{
			tvVertCount = mesh.M(CurrentChannel)->VNum();
			}

		TVMaps.v.SetCount(tvVertCount);
		TVMaps.cont.SetCount(tvVertCount);

		vsel.SetSize(tvVertCount);

		TVMaps.geomPoints.SetCount(mesh.VNum());
		
		BitArray bs;
		mesh.getFaceSel(bs);

		for (int j=0; j<TVMaps.f.Count(); j++) 
			{
			TVMaps.f[j]->flags = 0;

			int fct;
			fct = mesh.f[j].deg;

			if (mesh.f[j].GetFlag(MN_DEAD)) fct = 0;


			TVMaps.f[j]->t = new int[fct];
			TVMaps.f[j]->v = new int[fct];

			if (tvFace == NULL)
				{
				for (int k=0; k < fct;k++)
					TVMaps.f[j]->t[k] = 0;
				TVMaps.f[j]->FaceIndex = j;
				TVMaps.f[j]->MatID = mesh.f[j].material;
				TVMaps.f[j]->count = fct;
				if (bs[j])
					TVMaps.f[j]->flags = 0;
				else TVMaps.f[j]->flags = FLAG_DEAD;
				for (k = 0; k < fct; k++)
					{
					int index = mesh.f[j].vtx[k];
					TVMaps.f[j]->v[k] = index;
					TVMaps.geomPoints[index] = mesh.v[index].p;
					}
				}
			else
				{
				for (int k=0; k < fct;k++)
					TVMaps.f[j]->t[k] = tvFace[j].tv[k];
				TVMaps.f[j]->FaceIndex = j;
				TVMaps.f[j]->MatID = mesh.f[j].material;
				TVMaps.f[j]->count = fct;
				if (bs[j])
					TVMaps.f[j]->flags = 0;
				else TVMaps.f[j]->flags = FLAG_DEAD;
				for ( k = 0; k < fct; k++)
					{
					int index = mesh.f[j].vtx[k];
					TVMaps.f[j]->v[k] = index;
					TVMaps.geomPoints[index] = mesh.v[index].p;
					}
								
				}
			}
		for (j=0; j<TVMaps.v.Count(); j++) 
			{
			TVMaps.cont[j] = NULL;
			TVMaps.v[j].flags = FLAG_DEAD;
			if (tVerts)
				TVMaps.v[j].p  = tVerts[j];
			else TVMaps.v[j].p  = Point3(0.0f,0.0f,0.0f);
//check if vertex for this face selected
			TVMaps.v[j].influence = 0.0f;

			}
		if (tvFace == NULL) BuildInitialMapping(&mesh);
		for (j=0; j<TVMaps.f.Count(); j++) 
			{
			if (TVMaps.f[j]->flags != FLAG_DEAD)
				{
				int a;
				for (int k =0 ; k < TVMaps.f[j]->count;k++)
					{
					a = TVMaps.f[j]->t[k];
					TVMaps.v[a].flags = 0;
					}
/*				a = TVMaps.f[j]->t[1];
				TVMaps.v[a].flags = 0;
				a = TVMaps.f[j]->t[2];
				TVMaps.v[a].flags = 0;
*/
				}
			}


		}

	return TRUE;
	}


void UnwrapMod::ApplyMNMeshMapping(ObjectState *os, int CurrentChannel, TimeValue t)
	{

// is whole mesh
	PolyObject *tobj = (PolyObject*)os->obj;
// Apply our mapping
	MNMesh &mesh = tobj->GetMesh();

//	if (CurrentChannel >= mesh.MNum()) 
//		{
		// allocate texture verts. Setup tv faces into a parallel
		// topology as the regular faces
	int numMaps = mesh.MNum ();
	if (CurrentChannel >= numMaps) 
			{
			mesh.SetMapNum(CurrentChannel+1);
			mesh.InitMap(CurrentChannel);
			}
//		mesh.setMapSupport (CurrentChannel, TRUE);
//		} 


	
	MNMapFace *tvFace = mesh.M(CurrentChannel)->f;

	if (!tvFace)
		{
		mesh.InitMap(CurrentChannel);
		tvFace = mesh.M(CurrentChannel)->f;

		}
				
	int tvFaceCount =  mesh.FNum();

	if (mesh.selLevel!=MNM_SL_FACE) 
		{
//copy into mesh struct

		for (int k=0; k<tvFaceCount; k++) 
			{
			if (k < TVMaps.f.Count())
				{
				for (int m = 0; m< TVMaps.f[k]->count; m++) 
					tvFace[k].tv[m] = TVMaps.f[k]->t[m];
//				tvFace[k].tv[1] = TVMaps.f[k]->t[1];
//				tvFace[k].tv[2] = TVMaps.f[k]->t[2];
				}
			else 
				{
				for (int m = 0; m< TVMaps.f[k]->count; m++) 
					tvFace[k].tv[m] = 0;
//				tvFace[k].t[1] = 0;
//				tvFace[k].t[2] = 0;
				}
			}
//match verts
		mesh.M(CurrentChannel)->setNumVerts( TVMaps.v.Count());
		Point3 *tVerts = mesh.M(CurrentChannel)->v;
		for (    k=0; k<TVMaps.v.Count(); k++) 
			{
			tVerts[k] = GetPoint(t,k);
			}

		
		}
	else
		{
//copy into mesh struct
//check if mesh has existing tv faces
		int offset = mesh.M(CurrentChannel)->VNum();
		int current = 0;
		BitArray s;
		mesh.getFaceSel(s);
		for (int k=0; k<tvFaceCount; k++) 
			{
//copy if face is selected
			if (s[k]==1)
				{
				for (int m = 0; m< TVMaps.f[k]->count; m++) 
					tvFace[k].tv[m] = TVMaps.f[k]->t[m] + offset;
//				tvFace[k].t[1] = TVMaps.f[k]->t[1] + offset;
//				tvFace[k].t[2] = TVMaps.f[k]->t[2] + offset;
				}
			}
//add our verts
//		mesh.M(CurrentChannel)->setNumVerts(TVMaps.v.Count()+offset,TRUE);
//		Point3 *tVerts = mesh.mapVerts(CurrentChannel);
		for (    k=0; k<TVMaps.v.Count(); k++) 
			mesh.M(CurrentChannel)->NewVert( GetPoint(t,k));
//			tVerts[k+offset] = GetPoint(t,k);

							

		}
	mesh.M(CurrentChannel)->CollapseDeadVerts(mesh.f);

	}
