#include "TreeViewUtil.h"




class MapChannelAddClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return new MapChannelAdd(); }
	const TCHAR *	ClassName() { return GetString(IDS_MAPCHANNELADDNAME); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return MAPCHANNELADD_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("MapChannelAdd"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};



static MapChannelAddClassDesc MapChannelAddDesc;
ClassDesc2* GetMapChannelAddDesc() { return &MapChannelAddDesc; }


enum { mapchanneladd_params };


//TODO: Add enums for various parameters
enum { 
	pb_mapid
};



static ParamBlockDesc2 mapchanneladd_param_blk ( mapchanneladd_params, _T("params"),  0, &MapChannelAddDesc, 
	P_AUTO_CONSTRUCT, PBLOCK_REF, 


	end
	);

IObjParam *MapChannelAdd::ip			= NULL;


//--- MapChannelAdd -------------------------------------------------------
MapChannelAdd::MapChannelAdd()
{
	MapChannelAddDesc.MakeAutoParamBlocks(this);

}

MapChannelAdd::~MapChannelAdd()
{
}

/*===========================================================================*\
 |	The validity of the parameters.  First a test for editing is performed
 |  then Start at FOREVER, and intersect with the validity of each item
\*===========================================================================*/
Interval MapChannelAdd::LocalValidity(TimeValue t)
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

void MapChannelAdd::NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index)
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

void MapChannelAdd::NotifyPostCollapse(INode *node,Object *obj, IDerivedObject *derObj, int index)
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


void MapChannelAdd::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) 
{
	//TODO: Add the code for actually modifying the object


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

	//TriObject *tobj = (TriObject*)os->obj;
	//Mesh &mesh = tobj->GetMesh();		

	if (mnmesh)
	{
		int numMaps = mnmesh->numm;
		mnmesh->SetMapNum(numMaps+1);
		mnmesh->InitMap(numMaps);
		
	}
	else if (mesh)
	{
		int numMaps = mesh->getNumMaps();

		mesh->setNumMaps(numMaps+1, TRUE);
	}
	else if (pmesh)
	{
		int numMaps = pmesh->getNumMaps();

		pmesh->setNumMaps(numMaps+1, TRUE);
//		pmesh->setNumMapPatches(numMaps,0);
//		pmesh->setNumMapVerts(numMaps,0);
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


void MapChannelAdd::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);	

	MapChannelAddDesc.BeginEditParams(ip, this, flags, prev);

}

void MapChannelAdd::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	MapChannelAddDesc.EndEditParams(ip, this, flags, next);

	TimeValue t = ip->GetTime();
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);
	this->ip = NULL;


}



Interval MapChannelAdd::GetValidity(TimeValue t)
{
	Interval valid = FOREVER;
	//TODO: Return the validity interval of the modifier
	return valid;
}




RefTargetHandle MapChannelAdd::Clone(RemapDir& remap)
{
	MapChannelAdd* newmod = new MapChannelAdd();	
	//TODO: Add the cloning code here
	newmod->ReplaceReference(PBLOCK_REF,pblock->Clone(remap));
	BaseClone(this, newmod, remap);
	return(newmod);
}


//From ReferenceMaker 
RefResult MapChannelAdd::NotifyRefChanged(
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

void MapChannelAdd::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc)
{

}



//From Object
BOOL MapChannelAdd::HasUVW() 
{ 
	//TODO: Return whether the object has UVW coordinates or not
	return TRUE; 
}

void MapChannelAdd::SetGenUVW(BOOL sw) 
{  
	if (sw==HasUVW()) return;
	//TODO: Set the plugin internal value to sw				
}

IOResult MapChannelAdd::Load(ILoad *iload)
{
	//TODO: Add code to allow plugin to load its data
	
	return IO_OK;
}

IOResult MapChannelAdd::Save(ISave *isave)
{
	//TODO: Add code to allow plugin to save its data
	
	return IO_OK;
}


