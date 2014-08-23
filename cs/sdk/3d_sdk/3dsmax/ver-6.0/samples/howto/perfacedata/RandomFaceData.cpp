/*===========================================================================*\
 | 
 |  FILE:	RandomFaceData.cpp
 |			Project to demonstrate custom  per face data
 |			Simply creates random floats and binds them to object faces.
 |			All data is maintained by the modifier.  The modifier will survive
 |			a stack
 | 
 |  AUTH:   Neil Hazzard
 |			Developer Consulting Group
 |			Copyright(c) Discreet 2000
 |
 |  HIST:	Started 26-9-00
 | 
\*===========================================================================*/

#include "PerFaceData.h"
#include "SampleFaceData.h"


#define RANDOM_FACEDATA_CLASS_ID	Class_ID(0x9fee915, 0x175ae972)

#define PBLOCK_REF	0

class RandomFaceDataMod : public Modifier {
	BOOL mDisabled;
	IParamBlock2	*mpParams;	//ref 0
	
public:
	// From Animatable
	TCHAR *GetObjectName() { return GetString(IDS_RANDOM_CLASS_NAME); }

	ChannelMask ChannelsUsed()  { return GEOM_CHANNEL|TOPO_CHANNEL|SELECT_CHANNEL; }
	ChannelMask ChannelsChanged() { return GEOM_CHANNEL|TOPO_CHANNEL; }
	void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
	Class_ID InputType() {return defObjectClassID;}
	Interval LocalValidity(TimeValue t);

	// From BaseObject
	BOOL ChangeTopology() {return FALSE;}	

	void NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index);
	void NotifyPostCollapse(INode *node,Object *obj, IDerivedObject *derObj, int index);

	CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}
	void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);

	Interval GetValidity(TimeValue t);

	//From Animatable
	Class_ID ClassID() {return RANDOM_FACEDATA_CLASS_ID;}		
	SClass_ID SuperClassID() { return OSM_CLASS_ID; }
	void GetClassName(TSTR& s) {s = GetString(IDS_RANDOM_CLASS_NAME);}
	
	RefTargetHandle Clone( RemapDir &remap );
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
		PartID& partID,  RefMessage message) { return REF_SUCCEED; }

	int NumSubs() { return 1; }
	TSTR SubAnimName(int i) { return GetString(IDS_PARAMS); }				
	Animatable* SubAnim(int i) { return mpParams; }
	int NumRefs() { return 1; }
	RefTargetHandle GetReference(int i) { return mpParams; }
	void SetReference(int i, RefTargetHandle rtarg) { mpParams=(IParamBlock2*)rtarg; }

	int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
	IParamBlock2* GetParamBlock(int i) { return mpParams; } // return i'th ParamBlock
	IParamBlock2* GetParamBlockByID(BlockID id) { return (mpParams->ID() == id) ? mpParams : NULL; } // return id'd ParamBlock
	void DeleteThis() { delete this; }		

	RandomFaceDataMod();
};

class RandomFaceDataClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new RandomFaceDataMod();}
	const TCHAR *	ClassName() {return GetString(IDS_RANDOM_CLASS_NAME);}
	SClass_ID		SuperClassID() {return OSM_CLASS_ID;}
	Class_ID		ClassID() {return RANDOM_FACEDATA_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_CATEGORY);}
	const TCHAR*	InternalName() { return _T("RandomFaceDataMod"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
};

static RandomFaceDataClassDesc randomFaceDataDesc;
ClassDesc2* GetRandomFaceDataDesc() {return &randomFaceDataDesc;}

enum { perfacedata_params };

enum { 
	pb_range, pb_seed, pb_collapsable
};

static ParamBlockDesc2 perfacedata_param_blk ( perfacedata_params, _T("params"),  0, &randomFaceDataDesc, 
	P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//rollout
	IDD_RANDOM_DATA, IDS_PARAMS, 0, 0, NULL,
	// params
	pb_seed, 			_T("seed"), 		TYPE_INT, P_ANIMATABLE, IDS_SEED, 
		p_default, 		1,
		p_range, 		0, 999999,
		p_ui, TYPE_SPINNER, EDITTYPE_POS_INT, IDC_SEED, IDC_SEED_SPIN, .5f,
		end,

	pb_range, 			_T("range"), 		TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_RANGE, 
		p_default, 		1.0f, 
		p_range, 		0.0f,1000.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_RANGE, IDC_RANGE_SPIN, SPIN_AUTOSCALE,
		end,

	pb_collapsable, _T("collapsable"), TYPE_INT, 0, IDS_COLLAPSABLE,
		p_default, false,
		p_ui, TYPE_SINGLECHEKBOX, IDC_COLLAPSABLE,
		end,

	end
);

//--- RandomFaceDataMod -------------------------------------------------------
RandomFaceDataMod::RandomFaceDataMod() {
	mDisabled = FALSE;
	randomFaceDataDesc.MakeAutoParamBlocks(this);
}

Interval RandomFaceDataMod::LocalValidity(TimeValue t)
{
	// if being edited, return NEVER forces a cache to be built 
	// after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED)) return NEVER;
	return FOREVER;
}

RefTargetHandle RandomFaceDataMod::Clone(RemapDir& remap)
{
	RandomFaceDataMod* newmod = new RandomFaceDataMod();	
	newmod->ReplaceReference(0,mpParams->Clone(remap));
	BaseClone(this, newmod, remap);

	return(newmod);
}

// Between NotifyPreCollase and NotifyPostCollapse, Modify is
// called by the system.  Lets not be modified during the collapse
void RandomFaceDataMod::NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index) {
	BOOL collapsable;
	mpParams->GetValue (pb_collapsable, TimeValue(0), collapsable, FOREVER);
	if (collapsable) return;

	mDisabled = TRUE;
	TimeValue t = GetCOREInterface()->GetTime();
	NotifyDependents(Interval(t,t),PART_ALL,REFMSG_CHANGE);
}

// We want to survive a collapsed stack so we reapply ourselfs here
void RandomFaceDataMod::NotifyPostCollapse(INode *node,Object *obj, IDerivedObject *derObj, int index) {
	BOOL collapsable;
	mpParams->GetValue (pb_collapsable, TimeValue(0), collapsable, FOREVER);
	if (collapsable) return;

	Object *bo = node->GetObjectRef();
	IDerivedObject *derob = NULL;
	if(bo->SuperClassID() != GEN_DERIVOB_CLASS_ID) {
		derob = CreateDerivedObject(obj);
		node->SetObjectRef(derob);
	}
	else derob = (IDerivedObject*) bo;

	// Add ourselves to the top of the stack
	derob->AddModifier(this,NULL,derob->NumModifiers());
	mDisabled = FALSE;
}


void RandomFaceDataMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node)  {
	if(mDisabled == TRUE) return;

	IFaceDataMgr* pFDMgr = NULL;
	int numFaces = 0;

	// We can work with Face Data in a couple different object types:
	if (os->obj->IsSubClassOf(triObjectClassID)) {
		TriObject *tobj = (TriObject*)os->obj;
		Mesh *mesh = &tobj->GetMesh();
		numFaces = mesh->getNumFaces();
		// Get the face-data manager from the incoming object
		pFDMgr = static_cast<IFaceDataMgr*>(mesh->GetInterface( FACEDATAMGR_INTERFACE ));
	} else if (os->obj->IsSubClassOf(polyObjectClassID)) {
		PolyObject *pobj = (PolyObject*)os->obj;
		MNMesh *mesh = &pobj->GetMesh();
		numFaces = mesh->numf;
		// Get the face-data manager from the incoming object
		pFDMgr = static_cast<IFaceDataMgr*>(mesh->GetInterface( FACEDATAMGR_INTERFACE ));
	}

	if (pFDMgr == NULL) return;

	//Get our parameter
	int seed;
	float range;
	mpParams->GetValue (pb_seed, t, seed, FOREVER);
	mpParams->GetValue (pb_range, t, range, FOREVER);

	SampleFaceData* fdc = dynamic_cast<SampleFaceData*>(pFDMgr->GetFaceDataChan( FACE_MAXSAMPLEUSE_CLSID ));
	if ( fdc == NULL ) {
		// The mesh does not have our face-data channel so we will add it here
		fdc = new SampleFaceData();
		fdc->FacesCreated( 0, numFaces);
		pFDMgr->AddFaceDataChan( fdc );
	}

	Random randomGen;
	randomGen.srand(seed);
	for (int i=0; i<numFaces; i++) {
		fdc->SetValue (i, randomGen.getf(range));
	}
}

void RandomFaceDataMod::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev ) {
	randomFaceDataDesc.BeginEditParams(ip, this, flags, prev);
}

void RandomFaceDataMod::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next) {
	randomFaceDataDesc.EndEditParams(ip, this, flags, next);
}
