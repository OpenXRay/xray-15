#include "TreeViewUtil.h"




class MapChannelPasteClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return FALSE; }
	void *			Create(BOOL loading = FALSE) { return new MapChannelPaste(); }
	const TCHAR *	ClassName() { return GetString(IDS_MAPCHANNELPASTENAME); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return MAPCHANNELPASTE_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("MapChannelPaste"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};



static MapChannelPasteClassDesc MapChannelPasteDesc;
ClassDesc2* GetMapChannelPasteDesc() { return &MapChannelPasteDesc; }


enum { mapchannelpaste_params };


//TODO: Add enums for various parameters
enum { 
	pb_mapid,
	pb_usemap
};



static ParamBlockDesc2 mapchannelpaste_param_blk ( mapchannelpaste_params, _T("params"),  0, &MapChannelPasteDesc, 
	P_AUTO_CONSTRUCT , PBLOCK_REF, 
	// params
	pb_mapid, 			_T("mapID"), 		TYPE_INT, 	0, 	IDS_MAPID, 
		p_default, 		1, 
		p_range, 		0,99, 
		end,

	pb_usemap, 	_T("useMap"),		TYPE_BOOL, 		P_RESET_DEFAULT,				IDS_USEMAP,
		p_default, 		FALSE, 
		end, 

	end
	);

IObjParam *MapChannelPaste::ip			= NULL;


//--- MapChannelPaste -------------------------------------------------------
MapChannelPaste::MapChannelPaste()
{
	MapChannelPasteDesc.MakeAutoParamBlocks(this);

}

MapChannelPaste::~MapChannelPaste()
{
}

/*===========================================================================*\
 |	The validity of the parameters.  First a test for editing is performed
 |  then Start at FOREVER, and intersect with the validity of each item
\*===========================================================================*/
Interval MapChannelPaste::LocalValidity(TimeValue t)
{
	// if being edited, return NEVER forces a cache to be built 
	// after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;  
	//TODO: Return the validity interval of the modifier
	return NEVER;
}


/*************************************************************************************************
*
	Between NotifyPreCollapse and NotifyPostCollapse, Modify is
	called by the system.  NotifyPreCollapse can be used to save any plugin dependant data e.g.
	LocalModData
*
\*************************************************************************************************/

void MapChannelPaste::NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index)
{
	//TODO:  Perform any Pre Stack Collapse methods here
}



/*************************************************************************************************
*
	NotifyPostCollapse can be used to apply the modifier back onto to the stack, copying over the
	stored data from the temporary storage.  To reapply the modifier the following code can be 
	used

	Object *bo = node->GetObjectRef();
	IDerivedObject *derob = NULL;
	if(bo->SuperClassID() != GEN_DERIVOB_CLASS_ID)
	{
		derob = CreateDerivedObject(obj);
		node->SetObjectRef(derob);
	}
	else
		derob = (IDerivedObject*) bo;

	// Add ourselves to the top of the stack
	derob->AddModifier(this,NULL,derob->NumModifiers());

*
\*************************************************************************************************/

void MapChannelPaste::NotifyPostCollapse(INode *node,Object *obj, IDerivedObject *derObj, int index)
{
	//TODO: Perform any Post Stack collapse methods here.

}


/*************************************************************************************************
*
	ModifyObject will do all the work in a full modifier
    This includes casting objects to their correct form, doing modifications
	changing their parameters, etc
*
\************************************************************************************************/


void MapChannelPaste::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) 
{
	//TODO: Add the code for actually modifying the object

	//get th map id
	int mapID;
	pblock->GetValue(pb_mapid,0,mapID,FOREVER);
	mapID = buffer.pasteToChannel;

	BOOL useMap;
	pblock->GetValue(pb_usemap,0,useMap,FOREVER);

		//get the mesh
	Mesh *mesh = NULL;
	MNMesh *mnmesh = NULL;
	PatchMesh *pmesh = NULL;
		
	TriObject *collapsedtobj = NULL;
	if (os->obj->IsSubClassOf(triObjectClassID))
	{
		TriObject *tobj = (TriObject*)os->obj;
		mesh = &tobj->GetMesh();
	}
	else if (os->obj->IsSubClassOf(polyObjectClassID))
	{
		PolyObject *pobj = (PolyObject*)os->obj;
		mnmesh = &pobj->GetMesh();
	}
	else if (os->obj->IsSubClassOf(patchObjectClassID))
	{
		PatchObject *pobj = (PatchObject*)os->obj;
		pmesh = &pobj->patch;
	}
	
//	TriObject *tobj = (TriObject*)os->obj;
//	Mesh &mesh = tobj->GetMesh();		

	if (pmesh)
	{
		
		if ( (buffer.numRealFaces == pmesh->numPatches) && (buffer.patchDeg.Count() == pmesh->numPatches) &&
		     ((buffer.pasteToChannelType == PATCHMAPCHANNEL) || (buffer.pasteToChannelType == CHANNEL_MAP)))
		{
			BOOL gvalid = TRUE;
			for (int i = 0; i < pmesh->numPatches; i++)
			{

				int sDeg, tDeg;
				sDeg = buffer.patchDeg[i];		
				tDeg = 3;
				if (pmesh->patches[i].type == PATCH_QUAD)
					tDeg = 4;

				if (tDeg != sDeg)
					gvalid = FALSE;

			}
			if (gvalid)
			{
				if ((buffer.pasteToChannelType == PATCHMAPCHANNEL) || (buffer.pasteToChannelType == CHANNEL_MAP))
				{
					int numMaps = pmesh->getNumMaps();

					BOOL clear = FALSE;
					if (!pmesh->getMapSupport(mapID))
						{
						pmesh->setMapSupport(mapID, TRUE);
						clear = TRUE;
						}

			//if last channel reduce the number of channels
					if (buffer.pasteToSubID == -50)
						pmesh->setNumMapVerts(mapID, buffer.verts.Count());
					else pmesh->setNumMapVerts(mapID, buffer.w.Count());

					PatchTVert *uvw = pmesh->mapVerts(mapID);
					TVPatch *uvwFace = pmesh->tvPatches[mapID];//mesh->mapFaces(mapID);

					int ct = buffer.verts.Count();

					if (buffer.pasteToSubID != -50)
						ct = buffer.w.Count();

					for (int i = 0; i < ct; i++)
					{
						if (buffer.pasteToSubID == -50)
							uvw[i] = buffer.verts[i];
						else
							{
							if (clear)
								uvw[i].p = Point3(0.0f,0.0f,0.0f);
							uvw[i].p[buffer.pasteToSubID] = buffer.w[i];
							}
					}

					for (int i = 0; i < buffer.numFaces; i++)
					{
						uvwFace[i] = buffer.uvwPatchFaces[i];
					}

					


				}

			}

		}
	}
	else if (mnmesh)
	{
		
		
		if (buffer.numRealFaces == mnmesh->numf)
		{
			BOOL gvalid = TRUE;
			for (int i = 0; i < mnmesh->numf; i++)
			{
				int sDeg, tDeg;
				if (buffer.copyType == POLYMAPCHANNEL) 
					sDeg = buffer.uvwMNFaces[i]->deg;
				else sDeg = buffer.geomMNFaces[i]->deg;
				tDeg = mnmesh->f[i].deg;

				if (tDeg != sDeg)
					gvalid = FALSE;

			}
			if (gvalid)
			{
				if ((buffer.pasteToChannelType == POLYMAPCHANNEL) || (buffer.pasteToChannelType == CHANNEL_MAP))
				{
					int numMaps = mnmesh->numm;

					BOOL clear = FALSE;
					if (mapID >= numMaps)
						{
						mnmesh->SetMapNum(mapID+1);
						mnmesh->InitMap(mapID);
						clear = TRUE;
						}
					
//					MNMap *map = mnmesh->M(mapID);
					MNMapFace *uvwFace = mnmesh->M(mapID)->f;
					if (!uvwFace)
						{
						mnmesh->InitMap(mapID);
						uvwFace = mnmesh->M(mapID)->f;
						clear = TRUE;

						}
		

			//if last channel reduce the number of channels
					if (buffer.pasteToSubID == -50)
					{
						if (buffer.copyType == POLYMESH_GEOM)
							mnmesh->M(mapID)->setNumVerts(buffer.mnVerts.Count());
						else mnmesh->M(mapID)->setNumVerts(buffer.verts.Count());
					}
					else mnmesh->M(mapID)->setNumVerts(buffer.w.Count());

					Point3 *uvw = mnmesh->M(mapID)->v;

					int ct = mnmesh->M(mapID)->numv;//buffer.mnVerts.Count();

					if (buffer.pasteToSubID != -50)
						ct = buffer.w.Count();

					for (int i = 0; i < ct; i++)
					{
						if (buffer.pasteToSubID == -50)
						{
							if (buffer.copyType == POLYMESH_GEOM)
								uvw[i] = buffer.mnVerts[i].p;
							else uvw[i] = buffer.verts[i];
						}
						else
							{
							if (clear)
								uvw[i] = Point3(0.0f,0.0f,0.0f);
							uvw[i][buffer.pasteToSubID] = buffer.w[i];
							}
					}

					if ((buffer.copyType == POLYMESH_GEOM) || (buffer.copyType == POLYMESH_SEL))
					{
						for (int i = 0; i < buffer.numFaces; i++)
						{
							int deg = buffer.geomMNFaces[i]->deg;
							
							uvwFace[i].MakePoly(deg,buffer.geomMNFaces[i]->vtx);
						}
					}
					else
					{
						for (int i = 0; i < buffer.numFaces; i++)
						{
							uvwFace[i] = *buffer.uvwMNFaces[i];
						}

					}

				}
				else if ((buffer.pasteToChannelType == POLYGEOMCHANNEL)|| (buffer.pasteToChannelType == CHANNEL_GEOM))
				{

					int ct = buffer.mnVerts.Count();

					if (buffer.copyType == POLYMESH_MAP)
						ct = buffer.verts.Count();

					if (buffer.pasteToSubID != -50)
						ct = buffer.w.Count();

					if (buffer.pasteToSubID == -50)
					{
						if (buffer.copyType == POLYMESH_GEOM)
							mnmesh->setNumVerts(buffer.mnVerts.Count());
						else mnmesh->setNumVerts(buffer.verts.Count());
					}
					else mnmesh->setNumVerts(buffer.w.Count());

					

					MNVert *verts = mnmesh->v;
					MNFace *geomFace = mnmesh->f;

					


					for (int i = 0; i < ct; i++)
					{
						if (buffer.pasteToSubID == -50)
						{
							if (buffer.copyType == POLYMESH_GEOM)
								verts[i] = buffer.mnVerts[i];
							else verts[i].p = buffer.verts[i];
						}
						else verts[i].p[buffer.pasteToSubID] = buffer.w[i];
					}

					if ((buffer.copyType == POLYMESH_GEOM) || (buffer.copyType == POLYMESH_SEL))
					{
						for (int i = 0; i < buffer.numFaces; i++)
						{
							geomFace[i] = *buffer.geomMNFaces[i];
						}
					}
					else
					{
						for (int i = 0; i < buffer.numFaces; i++)
						{
							geomFace[i].MakePoly(buffer.uvwMNFaces[i]->deg,buffer.uvwMNFaces[i]->tv);
					//		geomFace[i].v[0] = buffer.uvwMNFaces[i].t[0];
					//		geomFace[i].v[1] = buffer.uvwMNFaces[i].t[1];
					//		geomFace[i].v[2] = buffer.uvwMNFaces[i].t[2];
						}

					}

				}

				else if ((buffer.pasteToChannelType == POLYSELCHANNEL) || (buffer.pasteToChannelType == CHANNEL_SEL))
				{
					MNVert *verts = mnmesh->v;
					MNFace *geomFace = mnmesh->f;

					mnmesh->SupportVSelectionWeights();


					

					float *vsw = NULL;

					vsw = mnmesh->getVSelectionWeights ();

					mnmesh->ClearVFlags (MN_SEL);

//					mesh->vertSel.ClearAll();

/*					for (int i = 0; i < buffer.w.Count(); i++)
					{
						if (vsw)
						{
							vsw[i] = buffer.w[i];
							if (vsw[i] >= 1.0f)
								mnmesh->v[i].SetFlag (MN_SEL);
								
						}
					}
*/					

					if ((buffer.copyType == POLYMESH_GEOM) || (buffer.copyType == POLYMESH_SEL))
					{
						for (int i = 0; i < buffer.numFaces; i++)
						{
//							geomFace[i] = *buffer.geomMNFaces[i];
							for (int k = 0; k < geomFace[i].deg; k++)
							{
//								geomFace[i].vtx[k] = buffer.uvwMNFaces[i]->tv[k];
								int id = buffer.geomMNFaces[i]->vtx[k];
								int gid = geomFace[i].vtx[k];
								if (vsw)
								{
									vsw[gid] = buffer.w[id];								
								}
							}

						}
					}
					else
					{
						for (int i = 0; i < buffer.numFaces; i++)
						{
							for (int k = 0; k < geomFace[i].deg; k++)
							{
//								geomFace[i].vtx[k] = buffer.uvwMNFaces[i]->tv[k];
								int id = buffer.uvwMNFaces[i]->tv[k];
								int gid = geomFace[i].vtx[k];
								if (vsw)
								{
									vsw[gid] = buffer.w[id];								
								}
								
							}
						}

					}
					
					for (int i = 0; i < mnmesh->numv; i++)
					{
						if (vsw)
						{							
							if (vsw[i] >= 1.0f)
								mnmesh->v[i].SetFlag (MN_SEL);								
						}
					}	
									
					mnmesh->dispFlags =MNDISP_VERTTICKS |MNDISP_SELVERTS ;
					mnmesh->selLevel = MNM_SL_VERTEX ;
				}

			}
		}
	}
	else if (mesh)
	{

		if (buffer.numFaces == mesh->numFaces)
		{
			if ((buffer.pasteToChannelType == TRIMAPCHANNEL) || (buffer.pasteToChannelType == CHANNEL_MAP))
			{
				int numMaps = mesh->getNumMaps();

				BOOL clear = FALSE;
				if (!mesh->mapSupport(mapID))
					{
					mesh->setMapSupport(mapID, TRUE);
					clear = TRUE;
					}

		//if last channel reduce the number of channels
				if (buffer.pasteToSubID == -50)
					mesh->setNumMapVerts(mapID, buffer.verts.Count());
				else mesh->setNumMapVerts(mapID, buffer.w.Count());

				UVVert *uvw = mesh->mapVerts(mapID);
				TVFace *uvwFace = mesh->mapFaces(mapID);

				int ct = buffer.verts.Count();

				if (buffer.pasteToSubID != -50)
					ct = buffer.w.Count();

				for (int i = 0; i < ct; i++)
				{
					if (buffer.pasteToSubID == -50)
						uvw[i] = buffer.verts[i];
					else
						{
						if (clear)
							uvw[i] = Point3(0.0f,0.0f,0.0f);
						uvw[i][buffer.pasteToSubID] = buffer.w[i];
						}
				}

				if ((buffer.copyType == TRIMESH_GEOM) || (buffer.copyType == TRIMESH_SEL))
				{
					for (int i = 0; i < buffer.numFaces; i++)
					{
						uvwFace[i].t[0] = buffer.geomFaces[i].v[0];
						uvwFace[i].t[1] = buffer.geomFaces[i].v[1];
						uvwFace[i].t[2] = buffer.geomFaces[i].v[2];
					}
				}
				else
				{
					for (int i = 0; i < buffer.numFaces; i++)
					{
						uvwFace[i] = buffer.uvwFaces[i];
					}

				}

			}
			else if ((buffer.pasteToChannelType == TRIGEOMCHANNEL)|| (buffer.pasteToChannelType == CHANNEL_GEOM))
			{

				int ct = buffer.verts.Count();

				if (buffer.pasteToSubID != -50)
					ct = buffer.w.Count();

				if (buffer.pasteToSubID == -50)
					mesh->setNumVerts(buffer.verts.Count());
				else mesh->setNumVerts(buffer.w.Count());

				

				Point3 *verts = mesh->verts;
				Face *geomFace = mesh->faces;

				


				for (int i = 0; i < ct; i++)
				{
					if (buffer.pasteToSubID == -50)
						verts[i] = buffer.verts[i];
					else verts[i][buffer.pasteToSubID] = buffer.w[i];
				}

				if ((buffer.copyType == TRIMESH_GEOM) || (buffer.copyType == TRIMESH_SEL))
				{
					for (int i = 0; i < buffer.numFaces; i++)
					{
						geomFace[i] = buffer.geomFaces[i];
					}
				}
				else
				{
					for (int i = 0; i < buffer.numFaces; i++)
					{
						geomFace[i].v[0] = buffer.uvwFaces[i].t[0];
						geomFace[i].v[1] = buffer.uvwFaces[i].t[1];
						geomFace[i].v[2] = buffer.uvwFaces[i].t[2];
					}

				}

			}
			else if ((buffer.pasteToChannelType == TRISELCHANNEL)|| (buffer.pasteToChannelType == CHANNEL_SEL))
			{


				Point3 *verts = mesh->verts;
				Face *geomFace = mesh->faces;

				mesh->SupportVSelectionWeights();


				

				float *vsw = NULL;

				vsw = mesh->getVSelectionWeights ();

				mesh->vertSel.ClearAll();
				for (int i = 0; i <mesh->vertSel.GetSize(); i++)
				{
					if (vsw)		
						vsw[i] = 0.0f;
				}

/*				for (int i = 0; i < buffer.w.Count(); i++)
				{
					if (vsw)
					{
						vsw[i] = buffer.w[i];
						if (vsw[i] >= 1.0f)
							mesh->vertSel.Set(i);
					}
				}
*/				

				if ((buffer.copyType == TRIMESH_GEOM) || (buffer.copyType == TRIMESH_SEL))
				{
					for (int i = 0; i < buffer.numFaces; i++)
					{
//						geomFace[i] = buffer.geomFaces[i];
						int id = buffer.geomFaces[i].v[0];
						if (vsw)
							vsw[id] = buffer.w[id];
						
						id = buffer.geomFaces[i].v[1];
						if (vsw)
							vsw[id] = buffer.w[id];							

						id = buffer.geomFaces[i].v[1];
						if (vsw)
							vsw[id] = buffer.w[id];							
							

					}
				}
				else
				{
					for (int i = 0; i < buffer.numFaces; i++)
					{
//						geomFace[i].v[0] = buffer.uvwFaces[i].t[0];
//						geomFace[i].v[1] = buffer.uvwFaces[i].t[1];
//						geomFace[i].v[2] = buffer.uvwFaces[i].t[2];

						int id = buffer.uvwFaces[i].t[0];
						int gid = mesh->faces[i].v[0];
						if (vsw)
							vsw[gid] = buffer.w[id];							

						id = buffer.uvwFaces[i].t[1];
						gid = mesh->faces[i].v[1];
						if (vsw)
							vsw[gid] = buffer.w[id];							

						id = buffer.uvwFaces[i].t[2];
						gid = mesh->faces[i].v[2];
						if (vsw)
							vsw[gid] = buffer.w[id];							



					}

				}
				
				for (int i = 0; i < mesh->numVerts; i++)
				{
					if (vsw)
					{
//						vsw[i] = buffer.w[i];
						if (vsw[i] >= 1.0f)
							mesh->vertSel.Set(i);
					}
				}
				
				

				mesh->dispFlags = DISP_VERTTICKS|DISP_SELVERTS;
				mesh->selLevel = MESH_VERTEX;

			}

		}
		mesh->InvalidateTopologyCache();
	}

	

	
	


	Interval iv;
	iv = FOREVER;

	os->obj->PointsWereChanged();

	iv &= os->obj->ChannelValidity (t, VERT_COLOR_CHAN_NUM);
	iv &= os->obj->ChannelValidity (t, TEXMAP_CHAN_NUM);
	iv = iv & os->obj->ChannelValidity(t,GEOM_CHAN_NUM);
	iv = iv & os->obj->ChannelValidity(t,TOPO_CHAN_NUM);
	iv = iv & os->obj->ChannelValidity(t,SELECT_CHAN_NUM);

	os->obj->UpdateValidity(GEOM_CHAN_NUM,iv);	
	os->obj->UpdateValidity (VERT_COLOR_CHAN_NUM, iv);
	os->obj->UpdateValidity(TEXMAP_CHAN_NUM,iv);
	os->obj->UpdateValidity (SELECT_CHAN_NUM, iv);


}


void MapChannelPaste::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);	

	MapChannelPasteDesc.BeginEditParams(ip, this, flags, prev);

}

void MapChannelPaste::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	MapChannelPasteDesc.EndEditParams(ip, this, flags, next);

	TimeValue t = ip->GetTime();
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);
	this->ip = NULL;


}



Interval MapChannelPaste::GetValidity(TimeValue t)
{
	Interval valid = FOREVER;
	//TODO: Return the validity interval of the modifier
	return valid;
}




RefTargetHandle MapChannelPaste::Clone(RemapDir& remap)
{
	MapChannelPaste* newmod = new MapChannelPaste();	
	//TODO: Add the cloning code here
	newmod->ReplaceReference(PBLOCK_REF,pblock->Clone(remap));
	BaseClone(this, newmod, remap);
	return(newmod);
}


//From ReferenceMaker 
RefResult MapChannelPaste::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	//TODO: Add code to handle the various reference changed messages
	return REF_SUCCEED;
}

/****************************************************************************************
*
 	NotifyInputChanged is called each time the input object is changed in some way
 	We can find out how it was changed by checking partID and message
*
\****************************************************************************************/

void MapChannelPaste::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc)
{

}



//From Object
BOOL MapChannelPaste::HasUVW() 
{ 
	//TODO: Return whether the object has UVW coordinates or not
	return TRUE; 
}

void MapChannelPaste::SetGenUVW(BOOL sw) 
{  
	if (sw==HasUVW()) return;
	//TODO: Set the plugin internal value to sw				
}

IOResult MapChannelPaste::Load(ILoad *iload)
{
	//TODO: Add code to allow plugin to load its data
	
	return buffer.Load(iload);
}

IOResult MapChannelPaste::Save(ISave *isave)
{
	//TODO: Add code to allow plugin to save its data
	
	return buffer.Save(isave);
}


int MapChannelPaste::SetMapChannel(int mapID)
{
	if (pblock)
	{
		pblock->SetValue(pb_mapid,0,mapID);
	}
	return 1;
}

int MapChannelPaste::SetUseMapChannel(BOOL use)
{
	if (pblock)
	{
		pblock->SetValue(pb_usemap,0,use);
	}
	
	return 1;

}