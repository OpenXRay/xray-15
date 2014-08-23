#include "TreeViewUtil.h"




class MapChannelDeleteClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return new MapChannelDelete(); }
	const TCHAR *	ClassName() { return GetString(IDS_MAPCHANNELDELETENAME); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return MAPCHANNELDELETE_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("MapChannelDelete"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};



static MapChannelDeleteClassDesc MapChannelDeleteDesc;
ClassDesc2* GetMapChannelDeleteDesc() { return &MapChannelDeleteDesc; }


enum { mapchanneldelete_params };


//TODO: Add enums for various parameters
enum { 
	pb_mapid
};



static ParamBlockDesc2 mapchanneldelete_param_blk ( mapchanneldelete_params, _T("params"),  0, &MapChannelDeleteDesc, 
	P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//rollout
	IDD_MAPCHANNELDELETEPANEL, IDS_PARAMS, 0, 0, NULL,
	// params
	pb_mapid, 			_T("mapID"), 		TYPE_INT, 	0, 	IDS_MAPID, 
		p_default, 		1, 
		p_range, 		0,99, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_INT, IDC_EDIT,	IDC_SPIN, 0.01f, 
		end,


	end
	);

IObjParam *MapChannelDelete::ip			= NULL;


//--- MapChannelDelete -------------------------------------------------------
MapChannelDelete::MapChannelDelete()
{
	MapChannelDeleteDesc.MakeAutoParamBlocks(this);

}

MapChannelDelete::~MapChannelDelete()
{
}

/*===========================================================================*\
 |	The validity of the parameters.  First a test for editing is performed
 |  then Start at FOREVER, and intersect with the validity of each item
\*===========================================================================*/
Interval MapChannelDelete::LocalValidity(TimeValue t)
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

void MapChannelDelete::NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index)
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

void MapChannelDelete::NotifyPostCollapse(INode *node,Object *obj, IDerivedObject *derObj, int index)
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


void MapChannelDelete::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) 
{
	//TODO: Add the code for actually modifying the object

	//get th map id
	int mapID;
	pblock->GetValue(pb_mapid,0,mapID,FOREVER);

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
	

	if (mnmesh)
	{


		int numMaps = mnmesh->numm;

		if (mapID < numMaps)
		{
			mnmesh->M(mapID)->Clear();
			mnmesh->ClearMap(mapID);

		}
	//if last channel reduce the number of channels
			

		if ((numMaps-1) == mapID)
		{
			if (mapID >= 0)
				mnmesh->SetMapNum(mapID);
		}
	}
	else if (mesh)
	{


		int numMaps = mesh->getNumMaps();

		if (mesh->mapSupport(mapID))
		{
			mesh->setNumMapVerts(mapID, 0);
			mesh->setMapSupport(mapID, FALSE);

		}
	//if last channel reduce the number of channels
			

		if ((numMaps-1) == mapID)
		{
			mesh->setNumMaps((numMaps-1), TRUE);
		}
	}
	else if (pmesh)
	{


		int numMaps = pmesh->getNumMaps();

		if (pmesh->getMapSupport(mapID))
		{
//			pmesh->setNumMapVerts(mapID, 0);
//			pmesh->setNumMapPatches(mapID, 0);
			pmesh->setMapSupport(mapID, FALSE);

		}
	//if last channel reduce the number of channels
			

		if ((numMaps-1) == mapID)
		{
			if ((numMaps-1) >= 1)
				pmesh->setNumMaps((numMaps-1), TRUE);
		}
	}


	Interval iv;
	iv = FOREVER;

	os->obj->PointsWereChanged();

	iv &= os->obj->ChannelValidity (t, VERT_COLOR_CHAN_NUM);
	iv &= os->obj->ChannelValidity (t, TEXMAP_CHAN_NUM);
	iv = iv & os->obj->ChannelValidity(t,GEOM_CHAN_NUM);
	iv = iv & os->obj->ChannelValidity(t,TOPO_CHAN_NUM);

	os->obj->UpdateValidity(GEOM_CHAN_NUM,iv);	
	os->obj->UpdateValidity (VERT_COLOR_CHAN_NUM, iv);
	os->obj->UpdateValidity(TEXMAP_CHAN_NUM,iv);

}


void MapChannelDelete::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);	

	MapChannelDeleteDesc.BeginEditParams(ip, this, flags, prev);

}

void MapChannelDelete::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	MapChannelDeleteDesc.EndEditParams(ip, this, flags, next);

	TimeValue t = ip->GetTime();
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);
	this->ip = NULL;


}



Interval MapChannelDelete::GetValidity(TimeValue t)
{
	Interval valid = FOREVER;
	//TODO: Return the validity interval of the modifier
	return valid;
}




RefTargetHandle MapChannelDelete::Clone(RemapDir& remap)
{
	MapChannelDelete* newmod = new MapChannelDelete();	
	//TODO: Add the cloning code here
	newmod->ReplaceReference(PBLOCK_REF,pblock->Clone(remap));
	BaseClone(this, newmod, remap);
	return(newmod);
}


//From ReferenceMaker 
RefResult MapChannelDelete::NotifyRefChanged(
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

void MapChannelDelete::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc)
{

}



//From Object
BOOL MapChannelDelete::HasUVW() 
{ 
	//TODO: Return whether the object has UVW coordinates or not
	return TRUE; 
}

void MapChannelDelete::SetGenUVW(BOOL sw) 
{  
	if (sw==HasUVW()) return;
	//TODO: Set the plugin internal value to sw				
}

IOResult MapChannelDelete::Load(ILoad *iload)
{
	//TODO: Add code to allow plugin to load its data
	
	return IO_OK;
}

IOResult MapChannelDelete::Save(ISave *isave)
{
	//TODO: Add code to allow plugin to save its data
	
	return IO_OK;
}


int MapChannelDelete::SetMapChannel(int mapID)
{
	if (pblock)
	{
		pblock->SetValue(pb_mapid,0,mapID);
	}
	return 1;
}