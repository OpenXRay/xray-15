#include "unwrap.h"


void UnwrapMod::BuildInitialMapping(Mesh *msh)
	{
//build bounding box
	Box3 bbox;
	bbox.Init();
//normalize the length width height
	for (int i = 0; i < TVMaps.f.Count(); i++)
		{
		int pcount = 3;
//	if (TVMaps.f[i].flags & FLAG_QUAD)
		pcount = TVMaps.f[i]->count;
		for (int j = 0; j < pcount; j++)
			{
			bbox += TVMaps.geomPoints[TVMaps.f[i]->v[j]];
			}

		}
	Tab<int> indexList;

	indexList.SetCount(TVMaps.f.Count() *4);
	BitArray usedIndex;
	usedIndex.SetSize(TVMaps.f.Count() *4);
	usedIndex.ClearAll();

	for (i = 0; i < TVMaps.f.Count()*4; i++)
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
				usedIndex.Set(msh->faces[i].v[j]);
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
				int a = msh->faces[i].v[j];
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

void UnwrapMod::GetFaceSelectionFromMesh(ObjectState *os, ModContext &mc, TimeValue t)
	{
	TriObject *tobj = (TriObject*)os->obj;
	MeshTopoData *d  = (MeshTopoData*)mc.localData;
	if (d)
		{
		d->SetFaceSel(tobj->GetMesh().faceSel, this, t);
		UpdateFaceSelection(d->faceSel);
		}
	}


void UnwrapMod::CopySelectionMesh(ObjectState *os, ModContext &mc, int CurrentChannel, TimeValue t)
	{	

		objType = IS_MESH;
		TriObject *tobj = (TriObject*)os->obj;
		MeshTopoData *d  = (MeshTopoData*)mc.localData;
		if (!d) 
			{
			mc.localData = d = new MeshTopoData(tobj->GetMesh());
			d->SetFaceSel(tobj->GetMesh().faceSel, this, t);
			UpdateFaceSelection(d->faceSel);
			

			}
		if ( ((editMod==this) && (!d->GetMesh())) || (updateCache))
			{
			d->SetCache(tobj->GetMesh());
			d->SetFaceSel(tobj->GetMesh().faceSel, this, t);
			updateCache = FALSE;
			UpdateFaceSelection(d->faceSel);
			SyncTVToGeomSelection(d);

			hiddenPolygons.SetSize(tobj->GetMesh().getNumFaces());
			hiddenPolygons.ClearAll();
			for (int i = 0; i < tobj->GetMesh().getNumFaces(); i++)
				{
				if (tobj->GetMesh().faces[i].Hidden())
					hiddenPolygons.Set(i,TRUE);
				}
			}

		BitArray faceSel = d->faceSel;


		faceSel.SetSize(tobj->GetMesh().getNumFaces(),TRUE);

		if ( (ip && (ip->GetSubObjectLevel() > 0) ))
			{

			tobj->GetMesh().faceSel = faceSel;

			if (showVerts)
				{
//select verts based on the current tverts;
				BitArray vertSel;
				vertSel.SetSize(tobj->GetMesh().getNumVerts(),TRUE);
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
							if ((index < vsel.GetSize()) && (vsel[index] ==1) && (sv<tobj->GetMesh().numFaces))
//							if (vsel[index] ==1)
								{
								int findex = tobj->GetMesh().faces[sv].v[j];
//6-29--99 watje
								if ((findex < vertSel.GetSize()) && (findex >=0))
									vertSel.Set(findex,1);
								}
							}
						}
					}
				tobj->GetMesh().vertSel = vertSel;
				tobj->GetMesh().SetDispFlag(DISP_SELFACES|DISP_VERTTICKS|DISP_SELVERTS);
//done++;
				}
			else
				{
				tobj->GetMesh().SetDispFlag(DISP_SELFACES);
				}

//UNFOLD STUFF
			Face *faces = tobj->GetMesh().faces;
			for (int i =0; i < tobj->GetMesh().getNumFaces(); i++)
				{
				if ( (i < hiddenPolygons.GetSize()) && (hiddenPolygons[i]) ) 
					faces[i].Hide();
				else faces[i].Show();
				}

			}

		
		if (!tmControl || (flags&CONTROL_OP) || (flags&CONTROL_INITPARAMS)) 
			InitControl(t);


//if planar mode build vert and face list

		if ( (ip && (ip->GetSubObjectLevel() == 1) ))
			{
			MeshUpdateGData(&tobj->GetMesh(),faceSel);
	
			}

	}


void UnwrapMod::MeshUpdateGData(Mesh *mesh, BitArray faceSel)
	{	


	gverts.d.SetCount(faceSel.GetSize()*4);

	gverts.sel.SetSize(faceSel.GetSize()*4,1);
	gverts.sel.ClearAll();

	Face *tf = mesh->faces;
	Point3 *tp = mesh->verts;

//isolate a vertex list of just the selected faces
	for (int i = 0; i < faceSel.GetSize(); i++)
		{
		if (faceSel[i])
			{
			for (int j = 0; j < 3; j++)
				{
				int index = tf[i].v[j];
				gverts.addPoint(index, tp[index]);
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
			t->t = new int[3];
			t->v = new int[3];
			t->count = 3;
			for (int j = 0; j < 3; j++)
				{
//find indes in our vert array
				t->t[j] = (int)tf[i].v[j];


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




BOOL UnwrapMod::InitializeMeshData(ObjectState *os, int CurrentChannel)
	{

	TriObject *tobj = (TriObject*)os->obj;
				// Apply our mapping
	Mesh &mesh = tobj->GetMesh();




//get channel from mesh
	TVMaps.channel = CurrentChannel;
					
	if ( (mesh.selLevel==MESH_FACE) && (mesh.faceSel.NumberSet() == 0) ) 
		{
		return FALSE;
		}
						
	
//get from mesh based on cahne
	TVFace *tvFace = mesh.mapFaces(CurrentChannel);
	Point3 *tVerts = mesh.mapVerts(CurrentChannel);
	if (mesh.selLevel!=MESH_FACE) 
		{
//copy into our structs
		TVMaps.SetCountFaces(mesh.getNumFaces());
		TVMaps.v.SetCount(mesh.getNumMapVerts (CurrentChannel));
		TVMaps.cont.SetCount(mesh.getNumMapVerts (CurrentChannel));

		vsel.SetSize(mesh.getNumMapVerts (CurrentChannel));
		
		TVMaps.geomPoints.SetCount(mesh.numVerts);

		for (int j=0; j<TVMaps.f.Count(); j++) 
			{
			TVMaps.f[j]->flags = 0;
			TVMaps.f[j]->t = new int[3];
			TVMaps.f[j]->v = new int[3];
			if (tvFace == NULL)
				{
				
				TVMaps.f[j]->t[0] = 0;
				TVMaps.f[j]->t[1] = 0;
				TVMaps.f[j]->t[2] = 0;
				TVMaps.f[j]->FaceIndex = j;
				TVMaps.f[j]->MatID = mesh.faces[j].getMatID();
				TVMaps.f[j]->flags = 0;
				TVMaps.f[j]->count = 3;
				for (int k = 0; k < 3; k++)
					{
					int index = mesh.faces[j].v[k];
					TVMaps.f[j]->v[k] = index;
					TVMaps.geomPoints[index] = mesh.verts[index];
					}

				if (!mesh.faces[j].getEdgeVis(0))
					TVMaps.f[j]->flags |= FLAG_HIDDENEDGEA;
				if (!mesh.faces[j].getEdgeVis(1))
					TVMaps.f[j]->flags |= FLAG_HIDDENEDGEB;
				if (!mesh.faces[j].getEdgeVis(2))
					TVMaps.f[j]->flags |= FLAG_HIDDENEDGEC;

				}
			else
				{
				TVMaps.f[j]->t[0] = tvFace[j].t[0];
				TVMaps.f[j]->t[1] = tvFace[j].t[1];
				TVMaps.f[j]->t[2] = tvFace[j].t[2];
				TVMaps.f[j]->FaceIndex = j;
				TVMaps.f[j]->MatID = mesh.faces[j].getMatID();
				

				TVMaps.f[j]->flags = 0;
				TVMaps.f[j]->count = 3;
				for (int k = 0; k < 3; k++)
					{
					int index = mesh.faces[j].v[k];
					TVMaps.f[j]->v[k] = index;
					TVMaps.geomPoints[index] = mesh.verts[index];
					}

				if (!mesh.faces[j].getEdgeVis(0))
					TVMaps.f[j]->flags |= FLAG_HIDDENEDGEA;
				if (!mesh.faces[j].getEdgeVis(1))
					TVMaps.f[j]->flags |= FLAG_HIDDENEDGEB;
				if (!mesh.faces[j].getEdgeVis(2))
					TVMaps.f[j]->flags |= FLAG_HIDDENEDGEC;
							
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
		TVMaps.SetCountFaces(mesh.getNumFaces());
		TVMaps.v.SetCount(mesh.getNumMapVerts (CurrentChannel));
		TVMaps.cont.SetCount(mesh.getNumMapVerts (CurrentChannel));

		vsel.SetSize(mesh.getNumMapVerts (CurrentChannel));

		TVMaps.geomPoints.SetCount(mesh.numVerts);


		for (int j=0; j<TVMaps.f.Count(); j++) 
			{
			TVMaps.f[j]->flags = 0;
			TVMaps.f[j]->t = new int[3];
			TVMaps.f[j]->v = new int[3];

			if (tvFace == NULL)
				{
				TVMaps.f[j]->t[0] = 0;
				TVMaps.f[j]->t[1] = 0;
				TVMaps.f[j]->t[2] = 0;
				TVMaps.f[j]->FaceIndex = j;
				TVMaps.f[j]->MatID = mesh.faces[j].getMatID();
				TVMaps.f[j]->count = 3;
				if (mesh.faceSel[j])
					TVMaps.f[j]->flags = 0;
				else TVMaps.f[j]->flags = FLAG_DEAD;
				for (int k = 0; k < 3; k++)
					{
					int index = mesh.faces[j].v[k];
					TVMaps.f[j]->v[k] = index;
					TVMaps.geomPoints[index] = mesh.verts[index];
					}

				if (!mesh.faces[j].getEdgeVis(0))
					TVMaps.f[j]->flags |= FLAG_HIDDENEDGEA;
				if (!mesh.faces[j].getEdgeVis(1))
					TVMaps.f[j]->flags |= FLAG_HIDDENEDGEB;
				if (!mesh.faces[j].getEdgeVis(2))
					TVMaps.f[j]->flags |= FLAG_HIDDENEDGEC;
				}
			else
				{
				TVMaps.f[j]->t[0] = tvFace[j].t[0];
				TVMaps.f[j]->t[1] = tvFace[j].t[1];
				TVMaps.f[j]->t[2] = tvFace[j].t[2];
				TVMaps.f[j]->FaceIndex = j;
				TVMaps.f[j]->MatID = mesh.faces[j].getMatID();
				TVMaps.f[j]->count = 3;
				if (mesh.faceSel[j])
					TVMaps.f[j]->flags = 0;
				else TVMaps.f[j]->flags = FLAG_DEAD;
				for (int k = 0; k < 3; k++)
					{
					int index = mesh.faces[j].v[k];
					TVMaps.f[j]->v[k] = index;
					TVMaps.geomPoints[index] = mesh.verts[index];
					}
				if (!mesh.faces[j].getEdgeVis(0))
					TVMaps.f[j]->flags |= FLAG_HIDDENEDGEA;
				if (!mesh.faces[j].getEdgeVis(1))
					TVMaps.f[j]->flags |= FLAG_HIDDENEDGEB;
				if (!mesh.faces[j].getEdgeVis(2))
					TVMaps.f[j]->flags |= FLAG_HIDDENEDGEC;

								
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
				a = TVMaps.f[j]->t[0];
				TVMaps.v[a].flags = 0;
				a = TVMaps.f[j]->t[1];
				TVMaps.v[a].flags = 0;
				a = TVMaps.f[j]->t[2];
				TVMaps.v[a].flags = 0;
				}
			}


		}

	return TRUE;
	}


void UnwrapMod::ApplyMeshMapping(ObjectState *os, int CurrentChannel, TimeValue t)
	{
// is whole mesh
	TriObject *tobj = (TriObject*)os->obj;
// Apply our mapping
	Mesh &mesh = tobj->GetMesh();

	if (!mesh.mapSupport(CurrentChannel)) 
		{
		// allocate texture verts. Setup tv faces into a parallel
		// topology as the regular faces
		if (CurrentChannel >= mesh.getNumMaps ()) mesh.setNumMaps (CurrentChannel+1, TRUE);
			mesh.setMapSupport (CurrentChannel, TRUE);

		TVFace *tvFace = mesh.mapFaces(CurrentChannel);
		for (int k =0; k < mesh.numFaces;k++)
			{
			tvFace[k].t[0] = 0;
			tvFace[k].t[1] = 0;
			tvFace[k].t[2] = 0;
			}
		} 


	
	TVFace *tvFace = mesh.mapFaces(CurrentChannel);
				
	int tvFaceCount =  mesh.numFaces;

	if (mesh.selLevel!=MESH_FACE) 
		{
//copy into mesh struct

		for (int k=0; k<tvFaceCount; k++) 
			{
			if (k < TVMaps.f.Count())
				{
				tvFace[k].t[0] = TVMaps.f[k]->t[0];
				tvFace[k].t[1] = TVMaps.f[k]->t[1];
				tvFace[k].t[2] = TVMaps.f[k]->t[2];
				}
			else 
				{
				tvFace[k].t[0] = 0;
				tvFace[k].t[1] = 0;
				tvFace[k].t[2] = 0;
				}
			}
//match verts
		mesh.setNumMapVerts (CurrentChannel, TVMaps.v.Count());
		Point3 *tVerts = mesh.mapVerts(CurrentChannel);
		for (    k=0; k<TVMaps.v.Count(); k++) 
			{
			tVerts[k] = GetPoint(t,k);
			}

		
		}
	else
		{
//copy into mesh struct
//check if mesh has existing tv faces
		int offset = mesh.getNumMapVerts (CurrentChannel);
		int current = 0;
		for (int k=0; k<tvFaceCount; k++) 
			{
//copy if face is selected
			if (mesh.faceSel[k]==1)
				{
				tvFace[k].t[0] = TVMaps.f[k]->t[0] + offset;
				tvFace[k].t[1] = TVMaps.f[k]->t[1] + offset;
				tvFace[k].t[2] = TVMaps.f[k]->t[2] + offset;
				}
			}
//add our verts
		mesh.setNumMapVerts (CurrentChannel,TVMaps.v.Count()+offset,TRUE);
		Point3 *tVerts = mesh.mapVerts(CurrentChannel);
		for (    k=0; k<TVMaps.v.Count(); k++) 
			tVerts[k+offset] = GetPoint(t,k);

							

		}
	mesh.DeleteIsoMapVerts(CurrentChannel);

	}
