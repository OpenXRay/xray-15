#include "SkinTools.h"




class PasteSkinWeightsClassDesc :public ClassDesc2 {
	public:
	int 			IsPublic() { return FALSE; }
	void *			Create(BOOL loading = FALSE) { return new PasteSkinWeights(); }
	const TCHAR *	ClassName() { return GetString(IDS_PASTESKINWEIGHTS); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return PASTESKINWEIGHTS_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("PasteSkinWeights"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};



static PasteSkinWeightsClassDesc PasteSkinWeightsDesc;
ClassDesc2* GetPasteSkinWeightsDesc() { return &PasteSkinWeightsDesc; }


enum { pasteskinweights_params };


//TODO: Add enums for various parameters
enum { 
	pb_mapid
};



static ParamBlockDesc2 pasteskinweights_params_param_blk ( pasteskinweights_params, _T("params"),  0, &PasteSkinWeightsDesc, 
	P_AUTO_CONSTRUCT + P_AUTO_UI, 0, 
	//rollout
	IDD_PASTEPANEL, IDS_PARAMS, 0, 0, NULL,


	end
	);


IObjParam *PasteSkinWeights::ip			= NULL;


//--- PasteSkinWeights -------------------------------------------------------
PasteSkinWeights::PasteSkinWeights()
{
	PasteSkinWeightsDesc.MakeAutoParamBlocks(this);

}

PasteSkinWeights::~PasteSkinWeights()
{
	for (int i = 0; i < boneList.Count(); i++)
	{
		if (boneList[i])
			delete boneList[i];
	}
	boneList.ZeroCount();

}

/*===========================================================================*\
 |	The validity of the parameters.  First a test for editing is performed
 |  then Start at FOREVER, and intersect with the validity of each item
\*===========================================================================*/
Interval PasteSkinWeights::LocalValidity(TimeValue t)
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

void PasteSkinWeights::NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index)
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

void PasteSkinWeights::NotifyPostCollapse(INode *node,Object *obj, IDerivedObject *derObj, int index)
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


void PasteSkinWeights::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) 
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


	if (mnmesh)
	{
		int numMaps = mnmesh->numm;
		mnmesh->SetMapNum(numMaps+1);

		int numGFaces = mnmesh->numf;
		int numGVerts = mnmesh->numv;

		numMaps = boneList.Count()/3+1;
		
		mnmesh->SetMapNum(numMaps);
		for (int i = 0; i < boneList.Count(); i++)
		{	
	
			int mapID = i/3;
			int subID = i%3;

			if (subID==0)	//create our face data
			{
				mnmesh->InitMap(mapID);
				mnmesh->M(mapID)->setNumVerts(numGVerts);
//				mnmesh->setNumMapVerts(mapID,numGVerts);
			}
//			TVFace *tvFace = mnmesh->mapFaces(mapID);
			MNMapFace *uvwFace = mnmesh->M(mapID)->f;
			UVVert *tvVerts = mnmesh->M(mapID)->v;//mnmesh->mapVerts(mapID);
			if (subID==0)	//create our face data
			{
				

				
		//copy our original
			//copy our geo faces to the texture faces
				for (int j = 0; j < numGFaces; j++)
				{
					int deg = mnmesh->f[j].deg;
					uvwFace[j].MakePoly(deg,mnmesh->f[j].vtx);
				}	
				for (j = 0; j < numGVerts; j++)
				{
					tvVerts[j] = Point3(0.0f,0.0f,0.0f);
				}
			}
			
			for (int j = 0; j < boneList[i]->weights.Count(); j++)
			{
				int vertIndex = boneList[i]->weights[j].vertIndex;
				float vertWeight = boneList[i]->weights[j].vertWeight;
				tvVerts[vertIndex][subID] = vertWeight;
			}
		}

		
	}
	else if (mesh)
	{
		int numMaps = mesh->getNumMaps();

		int numGFaces = mesh->numFaces;
		int numGVerts = mesh->numVerts;

		numMaps = boneList.Count()/3+1;
		
		mesh->setNumMaps(numMaps, FALSE);
		for (int i = 0; i < boneList.Count(); i++)
		{	
	
			int mapID = i/3;
			int subID = i%3;

			if (subID==0)	//create our face data
			{
				mesh->setMapSupport(mapID);
				mesh->setNumMapVerts(mapID,numGVerts);
			}
			TVFace *tvFace = mesh->mapFaces(mapID);
			UVVert *tvVerts = mesh->mapVerts(mapID);
			if (subID==0)	//create our face data
			{
				

				
		//copy our original
			//copy our geo faces to the texture faces
				for (int j = 0; j < numGFaces; j++)
				{
					for (int k = 0; k < 3; k++)
					{
						tvFace[j].t[k] = mesh->faces[j].v[k];
					}
				}	
				for (j = 0; j < numGVerts; j++)
				{
					tvVerts[j] = Point3(0.0f,0.0f,0.0f);
				}
			}
			
			for (int j = 0; j < boneList[i]->weights.Count(); j++)
			{
				int vertIndex = boneList[i]->weights[j].vertIndex;
				float vertWeight = boneList[i]->weights[j].vertWeight;
				tvVerts[vertIndex][subID] = vertWeight;
			}
				
			

			
		}
	}
	else if (pmesh)
	{
		int numMaps = pmesh->getNumMaps();

		int numGFaces = pmesh->numPatches;
		int numGVerts = pmesh->numVerts+pmesh->numVecs;

		numMaps = boneList.Count()/3+1;
		
		pmesh->setNumMaps(numMaps, FALSE);

		for (int i = 0; i < boneList.Count(); i++)
		{	
	
			int mapID = i/3;
			int subID = i%3;

			if (subID==0)	//create our face data
			{
				pmesh->setMapSupport(mapID);
				pmesh->setNumMapVerts(mapID,numGVerts);
			}

			TVPatch *tvFace = pmesh->tvPatches[mapID];;//TVFace *tvFace = pmesh->mapFaces(mapID);
			PatchTVert *tvVerts = pmesh->mapVerts(mapID);
			if (subID==0)	//create our face data
			{
				

				
		//copy our original
			//copy our geo faces to the texture faces
				for (int j = 0; j < numGFaces; j++)
				{
					int deg = 3;
						if (pmesh->patches[j].type == PATCH_QUAD)
						deg = 4;


					for (int k = 0; k < deg; k++)
					{
						int vindex = pmesh->patches[j].v[k];
						tvFace[j].tv[k] = pmesh->patches[j].v[k];
						tvFace[j].interiors[k] = pmesh->patches[j].interior[k]+pmesh->numVerts;
						tvFace[j].handles[k*2] = pmesh->patches[j].vec[k*2]+pmesh->numVerts;
						tvFace[j].handles[k*2+1] = pmesh->patches[j].vec[k*2+1]+pmesh->numVerts;
					}
				}	
				for (j = 0; j < numGVerts; j++)
				{
					tvVerts[j] = Point3(0.0f,0.0f,0.0f);
				}
			}
			
			for (int j = 0; j < boneList[i]->weights.Count(); j++)
			{
				int vertIndex = boneList[i]->weights[j].vertIndex;
				float vertWeight = boneList[i]->weights[j].vertWeight;
//DebugPrint("bone %d  vert %d weight %f\n",i,vertIndex,vertWeight);
				tvVerts[vertIndex].p[subID] = vertWeight;
			}
				
			

			
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


void PasteSkinWeights::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);	

	PasteSkinWeightsDesc.BeginEditParams(ip, this, flags, prev);

}

void PasteSkinWeights::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	PasteSkinWeightsDesc.EndEditParams(ip, this, flags, next);

	TimeValue t = ip->GetTime();
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);
	this->ip = NULL;


}



Interval PasteSkinWeights::GetValidity(TimeValue t)
{
	Interval valid = FOREVER;
	//TODO: Return the validity interval of the modifier
	return valid;
}




RefTargetHandle PasteSkinWeights::Clone(RemapDir& remap)
{
	PasteSkinWeights* newmod = new PasteSkinWeights();	
	//TODO: Add the cloning code here
	newmod->ReplaceReference(0,pblock->Clone(remap));
	BaseClone(this, newmod, remap);
	return(newmod);
}


//From ReferenceMaker 
RefResult PasteSkinWeights::NotifyRefChanged(
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

void PasteSkinWeights::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc)
{

}



//From Object
BOOL PasteSkinWeights::HasUVW() 
{ 
	//TODO: Return whether the object has UVW coordinates or not
	return TRUE; 
}

void PasteSkinWeights::SetGenUVW(BOOL sw) 
{  
	if (sw==HasUVW()) return;
	//TODO: Set the plugin internal value to sw				
}

IOResult PasteSkinWeights::Load(ILoad *iload)
{
	//TODO: Add code to allow plugin to load its data
	
	return IO_OK;
}

IOResult PasteSkinWeights::Save(ISave *isave)
{
	//TODO: Add code to allow plugin to save its data
	
	return IO_OK;
}


