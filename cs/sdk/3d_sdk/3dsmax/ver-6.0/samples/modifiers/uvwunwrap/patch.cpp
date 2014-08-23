#include "unwrap.h"



void UnwrapMod::RemoveDeadVerts(PatchMesh *mesh, int channel)
	{
	Tab<Point3> vertList;
	Tab<int> idList;
//copy over vertlist

	int ct = mesh->getNumMapVerts(channel);
	vertList.SetCount(ct);
	PatchTVert *tVerts = mesh->mapVerts(channel);
	for (int i = 0; i < ct; i++)
		vertList[i] = tVerts[i].p;

	BitArray usedList;
	usedList.SetSize(ct);
	TVPatch *tvFace = NULL;
	if (!mesh->getMapSupport(channel))
		{
		return;
		}

	tvFace = mesh->tvPatches[channel];
	if (tvFace == NULL) return;

	for (i =0; i < mesh->numPatches; i++)
		{
		int pcount = 3;
		if (mesh->patches[i].type == PATCH_QUAD) pcount = 4;

		for (int j = 0; j < pcount; j++)
			{
			int index = tvFace[i].tv[j];
			usedList.Set(index);
			if (!(mesh->patches[i].flags & PATCH_LINEARMAPPING))
				{
				if (!(mesh->patches[i].flags & PATCH_AUTO))
					{
					index = tvFace[i].interiors[j];
					if ((index >= 0) && (index < usedList.GetSize())) usedList.Set(index);
					}
				index = tvFace[i].handles[j*2];
				if ((index >= 0) && (index < usedList.GetSize())) usedList.Set(index);
				index = tvFace[i].handles[j*2+1];
				if ((index >= 0) && (index < usedList.GetSize())) usedList.Set(index);

				}
			}
		}
	mesh->setNumMapVerts (channel,usedList.NumberSet(),TRUE);

	int current = 0;
	tVerts = mesh->mapVerts(channel);

	for (i = 0; i < ct; i++)
		{
		if (usedList[i])
			{
			tVerts[current].p = vertList[i];
//now fix up faces
			for (int j = 0; j < mesh->numPatches; j++)
				{
				int pcount = 3;
				if (mesh->patches[j].type == PATCH_QUAD) pcount = 4;

				for (int k = 0; k < pcount; k++)
					{
					int index = tvFace[j].tv[k];
					if (index == i)
						{
						tvFace[j].tv[k] = current;
						}

					index = tvFace[j].interiors[k];
					if ((index >=0) && (index == i))
						{
						tvFace[j].interiors[k] = current;
						}
					index = tvFace[j].handles[k*2];
					if ((index >=0) && (index == i))
						{
						tvFace[j].handles[k*2] = current;
						}
					index = tvFace[j].handles[k*2+1];
					if ((index >=0) && (index == i))
						{
						tvFace[j].handles[k*2+1] = current;
						}


					}

				}
			current++;
			}
		}




	}



void UnwrapMod::BuildInitialMapping(PatchMesh *msh)
	{
	objType = IS_PATCH;
//build bounding box
	Box3 bbox;
	bbox.Init();
//normalize the length width height
	for (int i = 0; i < TVMaps.f.Count(); i++)
		{
		int pcount = 3;
//	if (TVMaps.f[i].flags & FLAG_QUAD) pcount = 4;
		pcount = TVMaps.f[i]->count;
		for (int j = 0; j < pcount; j++)
			{
			bbox += TVMaps.geomPoints[TVMaps.f[i]->v[j]];
			if (TVMaps.f[i]->flags & FLAG_CURVEDMAPPING)
				{
				if (TVMaps.f[i]->vecs)
					{
					bbox += TVMaps.geomPoints[TVMaps.f[i]->vecs->vhandles[j*2]];
					bbox += TVMaps.geomPoints[TVMaps.f[i]->vecs->vhandles[j*2+1]];
					if (TVMaps.f[i]->flags & FLAG_INTERIOR)
						{
						bbox += TVMaps.geomPoints[TVMaps.f[i]->vecs->vinteriors[j]];
						}
					}
				}
				
			}

		}
	Tab<int> indexList;

	indexList.SetCount(TVMaps.f.Count() *16);
	BitArray usedIndex;
	usedIndex.SetSize(TVMaps.f.Count() *16);
	usedIndex.ClearAll();

	for (i = 0; i < TVMaps.f.Count()*16; i++)
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
//		usedIndex.Set(TVMaps.f[i].t[j]);
				usedIndex.Set(msh->patches[i].v[j]);
				if (TVMaps.f[i]->flags & FLAG_CURVEDMAPPING)
					{
					if (TVMaps.f[i]->vecs)
						{
						usedIndex.Set(msh->patches[i].vec[j*2]+msh->numVerts);
						usedIndex.Set(msh->patches[i].vec[j*2+1]+msh->numVerts);
						if (TVMaps.f[i]->flags & FLAG_INTERIOR)
							{
							usedIndex.Set(msh->patches[i].interior[j]+msh->numVerts);
						
							}
						}	
					}

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
//		if (TVMaps.f[i].flags & FLAG_QUAD) pcount = 4;
			pcount = TVMaps.f[i]->count;

			TVMaps.f[i]->flags &= ~FLAG_DEAD;
			for (int j = 0; j < pcount; j++)
				{
				int index;
				int a = msh->patches[i].v[j];
				index = indexList[a];
				TVMaps.f[i]->t[j] = index;
				TVMaps.v[index].p.x =  TVMaps.geomPoints[TVMaps.f[i]->v[j]].x/bbox.Width().x + 0.5f;
				TVMaps.v[index].p.y =  TVMaps.geomPoints[TVMaps.f[i]->v[j]].y/bbox.Width().y + 0.5f;
				TVMaps.v[index].p.z =  TVMaps.geomPoints[TVMaps.f[i]->v[j]].z/bbox.Width().z + 0.5f;;
				TVMaps.v[index].influence =  0.f;
				TVMaps.v[index].flags =  0.f;
				TVMaps.cont[index] = NULL;
				if (TVMaps.f[i]->flags & FLAG_CURVEDMAPPING)
					{
					if (TVMaps.f[i]->vecs)
						{
//					usedIndex.Set(msh->patches[i].vec[j*2]+msh->numVerts);
//					usedIndex.Set(msh->patches[i].vec[j*2+1]+msh->numVerts);
						int index;
						int a = msh->patches[i].vec[j*2]+msh->numVerts;
						index = indexList[a];
						TVMaps.f[i]->vecs->handles[j*2] = index;
						TVMaps.v[index].p.x =  TVMaps.geomPoints[TVMaps.f[i]->vecs->vhandles[j*2]].x/bbox.Width().x + 0.5f;
						TVMaps.v[index].p.y =  TVMaps.geomPoints[TVMaps.f[i]->vecs->vhandles[j*2]].y/bbox.Width().y + 0.5f;
						TVMaps.v[index].p.z =  TVMaps.geomPoints[TVMaps.f[i]->vecs->vhandles[j*2]].z/bbox.Width().z + 0.5f;;
						TVMaps.v[index].influence =  0.f;
						TVMaps.v[index].flags =  0.f;
						TVMaps.cont[index] = NULL;

						a = msh->patches[i].vec[j*2+1]+msh->numVerts;
						index = indexList[a];
						TVMaps.f[i]->vecs->handles[j*2+1] = index;
						TVMaps.v[index].p.x =  TVMaps.geomPoints[TVMaps.f[i]->vecs->vhandles[j*2+1]].x/bbox.Width().x + 0.5f;
						TVMaps.v[index].p.y =  TVMaps.geomPoints[TVMaps.f[i]->vecs->vhandles[j*2+1]].y/bbox.Width().y + 0.5f;
						TVMaps.v[index].p.z =  TVMaps.geomPoints[TVMaps.f[i]->vecs->vhandles[j*2+1]].z/bbox.Width().z + 0.5f;;
						TVMaps.v[index].influence =  0.f;
						TVMaps.v[index].flags =  0.f;
						TVMaps.cont[index] = NULL;


						if (TVMaps.f[i]->flags & FLAG_INTERIOR)
							{
							int index;
							int a = msh->patches[i].interior[j]+msh->numVerts;
							index = indexList[a];
							TVMaps.f[i]->vecs->interiors[j] = index;
							TVMaps.v[index].p.x =  TVMaps.geomPoints[TVMaps.f[i]->vecs->vinteriors[j]].x/bbox.Width().x + 0.5f;
							TVMaps.v[index].p.y =  TVMaps.geomPoints[TVMaps.f[i]->vecs->vinteriors[j]].y/bbox.Width().y + 0.5f;
							TVMaps.v[index].p.z =  TVMaps.geomPoints[TVMaps.f[i]->vecs->vinteriors[j]].z/bbox.Width().z + 0.5f;;
							TVMaps.v[index].influence =  0.f;
							TVMaps.v[index].flags =  0.f;
							TVMaps.cont[index] = NULL;
							
							}
						}	
					}
				}
			}

		}


	}

void UnwrapMod::GetFaceSelectionFromPatch(ObjectState *os, ModContext &mc, TimeValue t)
{
	PatchObject *pobj = (PatchObject*)os->obj;
	MeshTopoData *d  = (MeshTopoData*)mc.localData;
	if (d) 
		{
		d->SetFaceSel(pobj->patch.patchSel, this, t);
		UpdateFaceSelection(d->faceSel);
		}

}


void UnwrapMod::CopySelectionPatch(ObjectState *os, ModContext &mc, int CurrentChannel, TimeValue t)
	{

	objType = IS_PATCH;
	PatchObject *pobj = (PatchObject*)os->obj;
	MeshTopoData *d  = (MeshTopoData*)mc.localData;
	if (!d) 
		{
		mc.localData = d = new MeshTopoData(pobj->patch);
		d->SetFaceSel(pobj->patch.patchSel, this, t);
		UpdateFaceSelection(d->faceSel);

		}
	if (((editMod==this) && (!d->GetPatch())) || (updateCache))
		{
		d->SetCache(pobj->patch);
		d->SetFaceSel(pobj->patch.patchSel, this, t);
		updateCache = FALSE;
		UpdateFaceSelection(d->faceSel);
		SyncTVToGeomSelection(d);

		hiddenPolygons.SetSize(pobj->patch.numPatches);
		hiddenPolygons.ClearAll();
		for (int i = 0; i < pobj->patch.numPatches; i++)
			{
			if (pobj->patch.patches[i].IsHidden())
				hiddenPolygons.Set(i,TRUE);
			}
		}

		
	BitArray faceSel = d->faceSel;
	faceSel.SetSize(pobj->patch.getNumPatches(),TRUE);
	if ( (ip && (ip->GetSubObjectLevel() > 0) ))
		{
		pobj->patch.patchSel = faceSel;
		if (showVerts)
			{
//select verts based on the current tverts;
			BitArray vertSel;
			vertSel.SetSize(pobj->patch.getNumVerts(),TRUE);
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
						if ((index < vsel.GetSize()) && (vsel[index] ==1) && (sv < pobj->patch.numPatches))
//							if (vsel[index] ==1)
							{
							int findex = pobj->patch.patches[sv].v[j];
//6-29--99 watje
							if ((findex < vertSel.GetSize()) && (findex >=0))
								vertSel.Set(findex,1);
							}
						}
					}
				}
			pobj->patch.vertSel = vertSel;

			pobj->patch.SetDispFlag(DISP_SELPATCHES|DISP_LATTICE|DISP_SELVERTS|DISP_VERTTICKS);
			}
		else pobj->patch.SetDispFlag(DISP_SELPATCHES|DISP_LATTICE);

//UNFOLD STUFF
		Patch *patches = pobj->patch.patches;
		for (int i =0; i < pobj->patch.numPatches; i++)
			{
			if ( (i < hiddenPolygons.GetSize()) && (hiddenPolygons[i]) ) 
				patches[i].SetHidden(TRUE);
			else patches[i].SetHidden(FALSE);
			}
		}

//		pobj->patch.SetDispFlag(DISP_SELPATCHES|DISP_LATTICE);

		
	if (!tmControl || (flags&CONTROL_OP) || (flags&CONTROL_INITPARAMS)) 
		InitControl(t);


//if planar mode build vert and face list
	if ( (ip && (ip->GetSubObjectLevel() == 1) ))
		{
		PatchUpdateGData(&pobj->patch,faceSel);
	
		}
	}


void UnwrapMod::PatchUpdateGData(PatchMesh *patch, BitArray faceSel)
	{

	gverts.d.SetCount(faceSel.GetSize()*16);
	gverts.sel.SetSize(faceSel.GetSize()*16,1);
	gverts.sel.ClearAll();
		
	Patch *tf = patch->patches;
	PatchVert *tp = patch->verts;

//isolate a vertex list of just the selected faces
	for (int i = 0; i < faceSel.GetSize(); i++)
		{
		if (faceSel[i])
			{
			int pcount = 3;
			if (tf[i].type == PATCH_QUAD) pcount = 4;
			for (int j = 0; j < pcount; j++)
				{
				int index = tf[i].v[j];
				gverts.addPoint(index, tp[index].p);
				index = tf[i].vec[j*2];
				gverts.addPoint(patch->numVerts+index, patch->vecs[index].p);
				index = tf[i].vec[j*2+1];
				gverts.addPoint(patch->numVerts+index, patch->vecs[index].p);
				if (!(tf[i].flags & PATCH_AUTO))
					{
					index = tf[i].interior[j];
					gverts.addPoint(patch->numVerts+index, patch->vecs[index].p);
					}


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
			int pcount = 3;
			t->flags = 0;
			if (tf[i].type == PATCH_QUAD) 
				{
				pcount = 4;
//						t.flags = FLAG_QUAD;
				}
			t->count = pcount;
			t->flags = TVMaps.f[i]->flags;

			t->t = new int[pcount];
			t->v = new int[pcount];

			t->vecs = NULL;
			UVW_TVVectorClass *tempv = NULL;

			if (!(t->flags & PATCH_LINEARMAPPING))
				{
				tempv = new UVW_TVVectorClass();
				}
			t->vecs = tempv;	 


			for (int j = 0; j < pcount; j++)
				{
//find indes in our vert array
				t->t[j] = (int)tf[i].v[j];
				if (t->vecs)
					{
// do texture points do't need to since they don't exist in this case
					int index;
					if (t->flags & FLAG_INTERIOR)
						{
						index = patch->numVerts+tf[i].interior[j];
						t->vecs->interiors[j] =index;
						}
					index = patch->numVerts+tf[i].vec[j*2];
					t->vecs->handles[j*2] =index;
					index = patch->numVerts+tf[i].vec[j*2+1];
					t->vecs->handles[j*2+1] =index;

					}
				}
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



BOOL UnwrapMod::InitializePatchData(ObjectState *os, int CurrentChannel)
	{

// is whole mesh
	PatchObject *pobj = (PatchObject*)os->obj;
	// Apply our mapping
	PatchMesh &patch = pobj->patch;

					
	if ( (patch.selLevel==PATCH_PATCH) && (patch.patchSel.NumberSet() == 0) ) 
		{
		return FALSE;
		}
	

//loop through all maps
//get channel from mesh
	TVMaps.channel = CurrentChannel;
					
	
//get from mesh based on cahne
	PatchTVert *tVerts = NULL;
	TVPatch *tvFace = NULL;
	if (!patch.getMapSupport(CurrentChannel))
		{
		patch.setNumMaps(CurrentChannel+1);
		}

	tVerts = patch.tVerts[CurrentChannel];
	tvFace = patch.tvPatches[CurrentChannel];



	if (patch.selLevel!=PATCH_PATCH ) 
		{
//copy into our structs
		TVMaps.SetCountFaces(patch.getNumPatches());
		TVMaps.v.SetCount(patch.getNumMapVerts(CurrentChannel));
		TVMaps.cont.SetCount(patch.getNumMapVerts(CurrentChannel));

		vsel.SetSize(patch.getNumMapVerts (CurrentChannel));

		TVMaps.geomPoints.SetCount(patch.getNumVerts()+patch.getNumVecs());

		for (int j=0; j<TVMaps.f.Count(); j++) 
			{
			TVMaps.f[j]->flags = 0;


			int pcount = 3;
			if (patch.patches[j].type == PATCH_QUAD) 
				{
				pcount = 4;
				}

			TVMaps.f[j]->t = new int[pcount];
			TVMaps.f[j]->v = new int[pcount];


			if (tvFace == NULL)
				{
				TVMaps.f[j]->t[0] = 0;
				TVMaps.f[j]->t[1] = 0;
				TVMaps.f[j]->t[2] = 0;
				if (pcount ==4) TVMaps.f[j]->t[3] = 0;
				TVMaps.f[j]->FaceIndex = j;
				TVMaps.f[j]->MatID = patch.getPatchMtlIndex(j);
				TVMaps.f[j]->flags = 0;
				TVMaps.f[j]->count = pcount;

				TVMaps.f[j]->vecs = NULL;
				UVW_TVVectorClass *tempv = NULL;
//new an instance
				if (!(patch.patches[j].flags & PATCH_AUTO))
					TVMaps.f[j]->flags |= FLAG_INTERIOR;

				if (!(patch.patches[j].flags & PATCH_LINEARMAPPING))
					{
					tempv = new UVW_TVVectorClass();
					TVMaps.f[j]->flags |= FLAG_CURVEDMAPPING;
					}
				TVMaps.f[j]->vecs = tempv;	 

				for (int k = 0; k < pcount; k++)
					{
					int index = patch.patches[j].v[k];
					TVMaps.f[j]->v[k] = index;
					TVMaps.geomPoints[index] = patch.verts[index].p;
//do handles and interiors
					//check if linear
					if (!(patch.patches[j].flags & PATCH_LINEARMAPPING))
						{
//do geometric points
						index = patch.patches[j].interior[k];
						TVMaps.f[j]->vecs->vinteriors[k] =patch.getNumVerts()+index;
						index = patch.patches[j].vec[k*2];
						TVMaps.f[j]->vecs->vhandles[k*2] =patch.getNumVerts()+index;
						index = patch.patches[j].vec[k*2+1];
						TVMaps.f[j]->vecs->vhandles[k*2+1] =patch.getNumVerts()+index;
// do texture points do't need to since they don't exist in this case
							
						TVMaps.f[j]->vecs->interiors[k] =0;								
						TVMaps.f[j]->vecs->handles[k*2] =0;
						TVMaps.f[j]->vecs->handles[k*2+1] =0;

						}

					}

				}
			else
				{
				TVMaps.f[j]->t[0] = tvFace[j].tv[0];
				TVMaps.f[j]->t[1] = tvFace[j].tv[1];
				TVMaps.f[j]->t[2] = tvFace[j].tv[2];
				if (pcount ==4) TVMaps.f[j]->t[3] = tvFace[j].tv[3];
				TVMaps.f[j]->FaceIndex = j;
				TVMaps.f[j]->MatID = patch.getPatchMtlIndex(j);

				TVMaps.f[j]->flags = 0;
				TVMaps.f[j]->count = pcount;


				TVMaps.f[j]->vecs = NULL;
				UVW_TVVectorClass *tempv = NULL;

				if (!(patch.patches[j].flags & PATCH_AUTO))
					TVMaps.f[j]->flags |= FLAG_INTERIOR;
//new an instance
				if (!(patch.patches[j].flags & PATCH_LINEARMAPPING))
					{
					BOOL mapLinear = FALSE;		
					for (int tvCount = 0; tvCount < patch.patches[j].type*2; tvCount++)
						{
						if (tvFace[j].handles[tvCount] < 0) mapLinear = TRUE;
						}
					if (!(patch.patches[j].flags & PATCH_AUTO))
						{
						for (tvCount = 0; tvCount < patch.patches[j].type; tvCount++)
							{
							if (tvFace[j].interiors[tvCount] < 0) mapLinear = TRUE;
							}
						}
					if (!mapLinear)
						{
						tempv = new UVW_TVVectorClass();
						TVMaps.f[j]->flags |= FLAG_CURVEDMAPPING;
						}
					}
				TVMaps.f[j]->vecs = tempv;	 


				if ((patch.selLevel==PATCH_PATCH ) && (patch.patchSel[j] == 0))
					TVMaps.f[j]->flags |= FLAG_DEAD;

				for (int k = 0; k < pcount; k++)
					{
					int index = patch.patches[j].v[k];
					TVMaps.f[j]->v[k] = index;
					TVMaps.geomPoints[index] = patch.verts[index].p;
//							TVMaps.f[j].pt[k] = patch.verts[index].p;
//do handles and interiors
							//check if linear
					if (TVMaps.f[j]->flags & FLAG_CURVEDMAPPING)
						{
//do geometric points
						index = patch.patches[j].interior[k];
						TVMaps.f[j]->vecs->vinteriors[k] =patch.getNumVerts()+index;
						index = patch.patches[j].vec[k*2];
						TVMaps.f[j]->vecs->vhandles[k*2] =patch.getNumVerts()+index;
						index = patch.patches[j].vec[k*2+1];
						TVMaps.f[j]->vecs->vhandles[k*2+1] =patch.getNumVerts()+index;
// do texture points do't need to since they don't exist in this case
						if (TVMaps.f[j]->flags & FLAG_INTERIOR)
							{
							index = tvFace[j].interiors[k];
							TVMaps.f[j]->vecs->interiors[k] =index;
							}
						index = tvFace[j].handles[k*2];
						TVMaps.f[j]->vecs->handles[k*2] =index;
						index = tvFace[j].handles[k*2+1];
						TVMaps.f[j]->vecs->handles[k*2+1] =index;

						}

					}



				}

			}
		for (int geomvecs =0; geomvecs < patch.getNumVecs(); geomvecs++)
			{
			TVMaps.geomPoints[geomvecs+patch.getNumVerts()] = patch.vecs[geomvecs].p;
			}

		for (    j=0; j<TVMaps.v.Count(); j++) 
			{
			TVMaps.cont[j] = NULL;
			TVMaps.v[j].flags = 0;
			if (tVerts)
				TVMaps.v[j].p  = tVerts[j];
			else TVMaps.v[j].p  = Point3(0.0f,0.0f,0.0f);
				TVMaps.v[j].influence = 0.0f;
			}
		if (tvFace == NULL) BuildInitialMapping(&patch);
		}
	else
		{

//copy into our structs
		TVMaps.SetCountFaces(patch.getNumPatches());

		TVMaps.v.SetCount(patch.getNumMapVerts (CurrentChannel));
		TVMaps.cont.SetCount(patch.getNumMapVerts (CurrentChannel));

		vsel.SetSize(patch.getNumMapVerts (CurrentChannel));

		TVMaps.geomPoints.SetCount(patch.getNumVerts()+patch.getNumVecs());


		for (int j=0; j<TVMaps.f.Count(); j++) 
			{
			TVMaps.f[j]->flags = 0;

			int pcount = 3;

			if (patch.patches[j].type == PATCH_QUAD) 
				{
				pcount = 4;
				}

			TVMaps.f[j]->t = new int[pcount];
			TVMaps.f[j]->v = new int[pcount];

			if (tvFace == NULL)
				{
				TVMaps.f[j]->t[0] = 0;
				TVMaps.f[j]->t[1] = 0;
				TVMaps.f[j]->t[2] = 0;
				if (pcount == 4) TVMaps.f[j]->t[3] = 0;
				TVMaps.f[j]->FaceIndex = j;
				TVMaps.f[j]->MatID = patch.patches[j].getMatID();
				if (patch.patchSel[j])
					TVMaps.f[j]->flags = 0;
				else TVMaps.f[j]->flags = FLAG_DEAD;
				TVMaps.f[j]->count = pcount;

				TVMaps.f[j]->vecs = NULL;
				UVW_TVVectorClass *tempv = NULL;

				if (!(patch.patches[j].flags & PATCH_AUTO))
					TVMaps.f[j]->flags |= FLAG_INTERIOR;

//new an instance
				if (!(patch.patches[j].flags & PATCH_LINEARMAPPING))
					{
					tempv = new UVW_TVVectorClass();
					TVMaps.f[j]->flags |= FLAG_CURVEDMAPPING;
					}
				TVMaps.f[j]->vecs = tempv;	 


				for (int k = 0; k < pcount; k++)
					{
					int index = patch.patches[j].v[k];
					TVMaps.f[j]->v[k] = index;
					TVMaps.geomPoints[index] = patch.verts[index].p;
//							TVMaps.f[j].pt[k] = patch.verts[index].p;
							//check if linear
					if (!(patch.patches[j].flags & PATCH_LINEARMAPPING))
						{
//do geometric points
						index = patch.patches[j].interior[k];
						TVMaps.f[j]->vecs->vinteriors[k] =patch.getNumVerts()+index;
						index = patch.patches[j].vec[k*2];
						TVMaps.f[j]->vecs->vhandles[k*2] =patch.getNumVerts()+index;
						index = patch.patches[j].vec[k*2+1];
						TVMaps.f[j]->vecs->vhandles[k*2+1] =patch.getNumVerts()+index;
// do texture points do't need to since they don't exist in this case
							
						TVMaps.f[j]->vecs->interiors[k] =0;								
						TVMaps.f[j]->vecs->handles[k*2] =0;
						TVMaps.f[j]->vecs->handles[k*2+1] =0;

						}

					}

				}
			else
				{
				TVMaps.f[j]->t[0] = tvFace[j].tv[0];
				TVMaps.f[j]->t[1] = tvFace[j].tv[1];
				TVMaps.f[j]->t[2] = tvFace[j].tv[2];
				if (pcount == 4) TVMaps.f[j]->t[3] = tvFace[j].tv[3];
				TVMaps.f[j]->FaceIndex = j;
				TVMaps.f[j]->MatID = patch.patches[j].getMatID();
				

				if (patch.patchSel[j])
					TVMaps.f[j]->flags = 0;
				else TVMaps.f[j]->flags = FLAG_DEAD;

				int pcount = 3;
				if (patch.patches[j].type == PATCH_QUAD) 
					{
//							TVMaps.f[j].flags |= FLAG_QUAD;
					pcount = 4;
					}
				TVMaps.f[j]->count = pcount;

				TVMaps.f[j]->vecs = NULL;
				UVW_TVVectorClass *tempv = NULL;
				if (!(patch.patches[j].flags & PATCH_AUTO))
					TVMaps.f[j]->flags |= FLAG_INTERIOR;
//new an instance
				if (!(patch.patches[j].flags & PATCH_LINEARMAPPING))
					{
					BOOL mapLinear = FALSE;		
					for (int tvCount = 0; tvCount < patch.patches[j].type*2; tvCount++)
						{
						if (tvFace[j].handles[tvCount] < 0) mapLinear = TRUE;
						}
					if (!(patch.patches[j].flags & PATCH_AUTO))
						{
						for (tvCount = 0; tvCount < patch.patches[j].type; tvCount++)
							{
							if (tvFace[j].interiors[tvCount] < 0) mapLinear = TRUE;
							}
						}
					if (!mapLinear)
						{
						tempv = new UVW_TVVectorClass();
						TVMaps.f[j]->flags |= FLAG_CURVEDMAPPING;
						}
					}
				TVMaps.f[j]->vecs = tempv;	 


				for (int k = 0; k < pcount; k++)
					{
					int index = patch.patches[j].v[k];
					TVMaps.f[j]->v[k] = index;
					TVMaps.geomPoints[index] = patch.verts[index].p;
//							TVMaps.f[j].pt[k] = patch.verts[index].p;
//do handles and interiors
							//check if linear
					if (TVMaps.f[j]->flags & FLAG_CURVEDMAPPING)
						{
//do geometric points
						index = patch.patches[j].interior[k];
						TVMaps.f[j]->vecs->vinteriors[k] =patch.getNumVerts()+index;
						index = patch.patches[j].vec[k*2];
						TVMaps.f[j]->vecs->vhandles[k*2] =patch.getNumVerts()+index;
						index = patch.patches[j].vec[k*2+1];
						TVMaps.f[j]->vecs->vhandles[k*2+1] =patch.getNumVerts()+index;
// do texture points do't need to since they don't exist in this case
						if (TVMaps.f[j]->flags & FLAG_INTERIOR)
							{
							index = tvFace[j].interiors[k];
							TVMaps.f[j]->vecs->interiors[k] =index;
							}
						index = tvFace[j].handles[k*2];
						TVMaps.f[j]->vecs->handles[k*2] =index;
						index = tvFace[j].handles[k*2+1];
						TVMaps.f[j]->vecs->handles[k*2+1] =index;

						}
					}

				}
			}
		for (j =0; j < patch.getNumVecs(); j++)
			{
			TVMaps.geomPoints[j+patch.getNumVerts()] = patch.vecs[j].p;
			}

		for (j=0; j<TVMaps.v.Count(); j++) 
			{
			TVMaps.cont[j] = NULL;
			TVMaps.v[j].flags = FLAG_DEAD;
			if (tVerts)
				TVMaps.v[j].p  = tVerts[j];
			else TVMaps.v[j].p  = Point3(.0f,0.0f,0.0f);
//check if vertex for this face selected
			TVMaps.v[j].influence = 0.0f;

			}


		if (tvFace == NULL) BuildInitialMapping(&patch);

		for (j=0; j<TVMaps.f.Count(); j++) 
			{
			if (!(TVMaps.f[j]->flags & FLAG_DEAD))
				{
				int a;
				a = TVMaps.f[j]->t[0];
				TVMaps.v[a].flags = 0;
				a = TVMaps.f[j]->t[1];
				TVMaps.v[a].flags = 0;
				a = TVMaps.f[j]->t[2];
				TVMaps.v[a].flags = 0;
				
				if (TVMaps.f[j]->count > 3)
					{
					a = TVMaps.f[j]->t[3];
					TVMaps.v[a].flags = 0;
					}
				if ( (TVMaps.f[j]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[j]->vecs))
					{
					for (int m =0; m < TVMaps.f[j]->count; m++) 
						{
						int hid = TVMaps.f[j]->vecs->handles[m*2];
						TVMaps.v[hid].flags = 0 ;
						hid = TVMaps.f[j]->vecs->handles[m*2+1];
						TVMaps.v[hid].flags = 0 ;
						}
					if (TVMaps.f[j]->flags & FLAG_INTERIOR) 
						{
						for (int m =0; m < TVMaps.f[j]->count; m++) 
							{
							int iid = TVMaps.f[j]->vecs->interiors[m];
							TVMaps.v[iid].flags = 0;
							}

						}


					}
				}
			}

		}


	return TRUE;
	}

void UnwrapMod::ApplyPatchMapping(ObjectState *os, int CurrentChannel, TimeValue t)
	{

// is whole mesh



	PatchObject *pobj = (PatchObject*)os->obj;
	// Apply our mapping
	PatchMesh &patch = pobj->patch;

//fix this get the channels from mesh
//				int NumChannels = 1;

//get channel from mesh
//					int CurrentChannel = 0;
	TVMaps.channel = CurrentChannel;
					
	
//get from mesh 

	if (!patch.getMapSupport(CurrentChannel) )
		{
		patch.setNumMaps (CurrentChannel+1);
/*		TVPatch *tvFace = patch.tvPatches[CurrentChannel];
		for (int k =0; k < patch.numPatches;k++)
			{
			tvFace[k].tv[0] = 0;
			tvFace[k].tv[1] = 0;
			tvFace[k].tv[2] = 0;
			tvFace[k].tv[3] = 0;
			}
*/
		}

	TVPatch *tvFace = patch.tvPatches[CurrentChannel];

					
	int tvFaceCount =  patch.numPatches;

	if (patch.selLevel!=PATCH_PATCH) 
		{
//copy into mesh struct
		if (!tvFace) 
			{
			// Create tvfaces and init to 0
			patch.setNumMapPatches(CurrentChannel,patch.getNumPatches());
			tvFace = patch.tvPatches[CurrentChannel];
			for (int k=0; k<patch.getNumPatches(); k++)
				{	
				for (int j=0; j<TVMaps.f[k]->count; j++) 
					{
					tvFace[k].tv[j] = 0;			
					tvFace[k].interiors[j] = 0;			
					tvFace[k].handles[j*2] = 0;			
					tvFace[k].handles[j*2+1] = 0;			
					}
				}
			}
		for (int k=0; k<tvFaceCount; k++) 
			{
			if (k < TVMaps.f.Count())
				{
				tvFace[k].tv[0] = TVMaps.f[k]->t[0];
				tvFace[k].tv[1] = TVMaps.f[k]->t[1];
				tvFace[k].tv[2] = TVMaps.f[k]->t[2];
				if (TVMaps.f[k]->count == 4) tvFace[k].tv[3] = TVMaps.f[k]->t[3];
				if (TVMaps.f[k]->flags & FLAG_CURVEDMAPPING)
					{
					patch.patches[k].flags &= ~PATCH_LINEARMAPPING;
					if (TVMaps.f[k]->vecs)
						{
						for (int m = 0;m < TVMaps.f[k]->count;m++)
							{
							if (TVMaps.f[k]->flags & FLAG_INTERIOR)
								{
								tvFace[k].interiors[m] = TVMaps.f[k]->vecs->interiors[m];			
								}

							tvFace[k].handles[m*2] = TVMaps.f[k]->vecs->handles[m*2];			
							tvFace[k].handles[m*2+1] = TVMaps.f[k]->vecs->handles[m*2+1];			
							}
						}

					}
				else patch.patches[k].flags |= PATCH_LINEARMAPPING;
				}
			else{
				tvFace[k].tv[0] = 0;
				tvFace[k].tv[1] = 0;
				tvFace[k].tv[2] = 0;
				if (TVMaps.f[k]->count == 4) tvFace[k].tv[3] = 0;
				for (int m = 0;m < TVMaps.f[k]->count;m++)
					{
					tvFace[k].interiors[m] = 0;			
					tvFace[k].handles[m*2] = 0;			
					tvFace[k].handles[m*2+1] = 0;			
					}

				}
			}
//match verts
		patch.setNumMapVerts (CurrentChannel,TVMaps.v.Count());
		PatchTVert *tVerts = patch.tVerts[CurrentChannel];
		for (k=0; k<TVMaps.v.Count(); k++) 
			tVerts[k].p = GetPoint(t,k);
		}
	else
		{
//copy into mesh struct
		if (!tvFace) 
			{
			// Create tvfaces and init to 0
			patch.setNumMapPatches (CurrentChannel,patch.getNumPatches());
			tvFace = patch.tvPatches[CurrentChannel];
			for (int k=0; k<patch.getNumPatches(); k++)
				{	
				for (int j=0; j<TVMaps.f[k]->count; j++) 
					{
					tvFace[k].tv[j] = 0;			
					tvFace[k].interiors[j] = 0;			
					tvFace[k].handles[j*2] = 0;			
					tvFace[k].handles[j*2+1] = 0;			

					}
				}
			}
		int offset = patch.getNumMapVerts (CurrentChannel);
		int current = 0;
		for (int k=0; k<tvFaceCount; k++) 
//						for (int k=0; k<TVMaps.f.Count(); k++) 
			{
//copy if face is selected
			if (patch.patchSel[k])
				{
				tvFace[k].tv[0] = TVMaps.f[k]->t[0]+offset;
				tvFace[k].tv[1] = TVMaps.f[k]->t[1]+offset;
				tvFace[k].tv[2] = TVMaps.f[k]->t[2]+offset;
				if (TVMaps.f[k]->count == 4) tvFace[k].tv[3] = TVMaps.f[k]->t[3]+offset;
				if (TVMaps.f[k]->flags & FLAG_CURVEDMAPPING)
					{
					patch.patches[k].flags &= ~PATCH_LINEARMAPPING;
					if (TVMaps.f[k]->vecs)
						{
						for (int m = 0;m < TVMaps.f[k]->count;m++)
							{
							if (TVMaps.f[k]->flags & FLAG_INTERIOR)
								{
								tvFace[k].interiors[m] = TVMaps.f[k]->vecs->interiors[m]+offset;			
								}
							tvFace[k].handles[m*2] = TVMaps.f[k]->vecs->handles[m*2]+offset;			
							tvFace[k].handles[m*2+1] = TVMaps.f[k]->vecs->handles[m*2+1]+offset;			
							}
						}

					}
				else patch.patches[k].flags |= PATCH_LINEARMAPPING;

//									current++;
				}
			}
//match verts
		patch.setNumMapVerts (CurrentChannel,TVMaps.v.Count()+offset,TRUE);
		PatchTVert *tVerts = patch.tVerts[CurrentChannel];
		for (    k=0; k<TVMaps.v.Count(); k++) 
			tVerts[k+offset].p = GetPoint(t,k);
		
		}
	RemoveDeadVerts(&patch,CurrentChannel);

	}
