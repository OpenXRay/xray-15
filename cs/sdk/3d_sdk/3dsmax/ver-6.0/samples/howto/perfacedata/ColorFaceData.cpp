/*===========================================================================*\
 | 
 |  FILE:	ColorFaceData.cpp
 |			Project to demonstrate custom  per face data
 |			Turns face data created by other local modifiers into visible colors.
 | 
 |  AUTH:   Steve Anderson
 |			Copyright(c) Discreet 2000
 |
 |  HIST:	Started February the 20th, 2002.
 | 
\*===========================================================================*/

#include "PerFaceData.h"
#include "SampleFaceData.h"

#define DATA_TO_COLOR_CLASS_ID	Class_ID(0x47866a3f, 0xc8a75b8)

#define PBLOCK_REF	0

class FaceDataToColorMod : public Modifier {
	IParamBlock2 *mpParams;
	static IParamMap2 *mpMap;
	static IObjParam *mpInterface;

	// Data structures for surviving "Collapse" process.
	bool mDisabled;

public:
	// Constructor
	FaceDataToColorMod();

	// From Animatable
	TCHAR *GetObjectName() { return GetString(IDS_COLOR_CLASS_NAME); }

	ChannelMask ChannelsUsed()  { return TOPO_CHANNEL; }
	ChannelMask ChannelsChanged() { return VERTCOLOR_CHANNEL; }
	void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
	Class_ID InputType() { return defObjectClassID; }
	Interval LocalValidity(TimeValue t);

	// From BaseObject
	BOOL ChangeTopology() {return FALSE;}	

	void NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index);
	void NotifyPostCollapse(INode *node,Object *obj, IDerivedObject *derObj, int index);

	CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}
	void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);

	Interval GetValidity(TimeValue t);

	// Loading/Saving
	//IOResult Load(ILoad *iload);
	//IOResult Save(ISave *isave);

	//IOResult LoadLocalData(ILoad *iload, LocalModData **pld) ;
	//IOResult SaveLocalData(ISave *isave, LocalModData *ld);

	//From Animatable
	Class_ID ClassID() {return DATA_TO_COLOR_CLASS_ID;}		
	SClass_ID SuperClassID() { return OSM_CLASS_ID; }
	void GetClassName(TSTR& s) {s = GetString(IDS_COLOR_CLASS_NAME);}
	
	RefTargetHandle Clone( RemapDir &remap );
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
		PartID& partID,  RefMessage message);

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

	void DisplayColors ();
};

class DataToColorClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new FaceDataToColorMod();}
	const TCHAR *	ClassName() {return GetString(IDS_COLOR_CLASS_NAME);}
	SClass_ID		SuperClassID() {return OSM_CLASS_ID;}
	Class_ID		ClassID() {return DATA_TO_COLOR_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_CATEGORY);}
	const TCHAR*	InternalName() { return _T("FaceDataToColorMod"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
};

static DataToColorClassDesc dataToColorDesc;
ClassDesc2* GetDataToColorDesc() {return &dataToColorDesc;}

class DataToColorDlgProc : public ParamMap2UserDlgProc {
	FaceDataToColorMod *mpMod;
public:
	DataToColorDlgProc () : mpMod(NULL) { }
	BOOL DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd,
		UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() { }
	void SetMod (FaceDataToColorMod *pMod) { mpMod = pMod; }
};

static DataToColorDlgProc theDlgProc;

BOOL DataToColorDlgProc::DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd,
							  UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_INITDIALOG:
		break;
		
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_DISPLAY_NOW:
			if (mpMod) mpMod->DisplayColors ();
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

enum { data_to_color_params };

enum { 
	pb_channel, pb_red, pb_green, pb_blue, pb_collapsable
};

static ParamBlockDesc2 data_to_color_param_blk ( data_to_color_params,
												_T("params"),  0, &dataToColorDesc, 
	P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	//rollout
	IDD_DATA_TO_COLOR, IDS_PARAMS, 0, 0, NULL,

	// params
	pb_channel, _T("colorChannel"), TYPE_INT, 0, IDS_COLOR_CHANNEL, 
		p_default, 0,
		p_ui, TYPE_RADIO, 2, IDC_COLOR, IDC_ILLUM, 
		end,

	pb_red, _T("redMultiplier"), TYPE_FLOAT, P_ANIMATABLE, IDS_RED_MULT,
		p_default, 0.0f,
		p_range, 0.0f, 1000.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_RED, IDC_RED_SPIN, SPIN_AUTOSCALE,
		end,

	pb_green, _T("greenMultiplier"), TYPE_FLOAT, P_ANIMATABLE, IDS_GREEN_MULT,
		p_default, 1.0f,
		p_range, 0.0f, 1000.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_GREEN, IDC_GREEN_SPIN, SPIN_AUTOSCALE,
		end,

	pb_blue, _T("blueMultiplier"), TYPE_FLOAT, P_ANIMATABLE, IDS_BLUE_MULT,
		p_default, 0.0f,
		p_range, 0.0f, 1000.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_BLUE, IDC_BLUE_SPIN, SPIN_AUTOSCALE,
		end,

	pb_collapsable, _T("collapsable"), TYPE_INT, 0, IDS_COLLAPSABLE,
		p_default, false,
		p_ui, TYPE_SINGLECHEKBOX, IDC_COLLAPSABLE,
		end,

	end
);

IObjParam *FaceDataToColorMod::mpInterface = NULL;
IParamMap2 *FaceDataToColorMod::mpMap = NULL;

//--- FaceDataToColorMod -------------------------------------------------------

FaceDataToColorMod::FaceDataToColorMod() : mDisabled(false), mpParams(NULL) {
	dataToColorDesc.MakeAutoParamBlocks(this);
}

Interval FaceDataToColorMod::LocalValidity(TimeValue t)
{
	// if being edited, return NEVER forces a cache to be built 
	// after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED)) return NEVER;

	Interval ret(FOREVER);
	float val;
	mpParams->GetValue (pb_red, t, val, ret);
	mpParams->GetValue (pb_green, t, val, ret);
	mpParams->GetValue (pb_blue, t, val, ret);
	return ret;
}

RefTargetHandle FaceDataToColorMod::Clone(RemapDir& remap)
{
	FaceDataToColorMod* newmod = new FaceDataToColorMod();	
	newmod->ReplaceReference(0,mpParams->Clone(remap));
	BaseClone(this, newmod, remap);

	return(newmod);
}

// Neil Hazzard's clever approach to surviving a stack collapse:
void FaceDataToColorMod::NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index) {
	BOOL collapsable;
	mpParams->GetValue (pb_collapsable, TimeValue(0), collapsable, FOREVER);
	if (collapsable) return;

	// Reevaluate with modifier turned off.
	mDisabled = true;
	TimeValue t = GetCOREInterface()->GetTime();
	NotifyDependents(Interval(t,t),PART_ALL,REFMSG_CHANGE);
}

// We want to survive a collapsed stack so we reapply ourselves here
void FaceDataToColorMod::NotifyPostCollapse(INode *node,Object *obj, IDerivedObject *derObj, int index) {
	BOOL collapsable;
	mpParams->GetValue (pb_collapsable, TimeValue(0), collapsable, FOREVER);
	if (collapsable) return;

	Object *bo = node->GetObjectRef();
	IDerivedObject *derob = NULL;
	if(bo->SuperClassID() != GEN_DERIVOB_CLASS_ID) {
		derob = CreateDerivedObject(obj);
		node->SetObjectRef(derob);
	} else derob = (IDerivedObject*) bo;

	// Add ourselves to the top of the stack
	derob->AddModifier(this,NULL,derob->NumModifiers());

	// Reengage modification:
	mDisabled = false;
}

void FaceDataToColorMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) {
	if (mDisabled) return;

	IFaceDataMgr* pFDMgr = NULL;

	// We can work with Face Data in a couple different object types:
	TriObject *pTri = NULL;
	PolyObject *pPoly = NULL;
	Mesh *pMesh = NULL;
	MNMesh *pMNMesh = NULL;
	int numFaces = 0;

	if (os->obj->IsSubClassOf(triObjectClassID)) {
		pTri = (TriObject*)os->obj;
		pMesh = &(pTri->GetMesh());
		numFaces = pMesh->getNumFaces();
		// Get the face-data manager from the incoming object
		pFDMgr = static_cast<IFaceDataMgr*>(pMesh->GetInterface( FACEDATAMGR_INTERFACE ));
	} else if (os->obj->IsSubClassOf(polyObjectClassID)) {
		pPoly = (PolyObject*)os->obj;
		pMNMesh = &pPoly->GetMesh();
		numFaces = pMNMesh->numf;
		// Get the face-data manager from the incoming object
		pFDMgr = static_cast<IFaceDataMgr*>(pMNMesh->GetInterface( FACEDATAMGR_INTERFACE ));
	}

	if (pFDMgr == NULL) return;

	//Get our parameters
	int channel;
	float r, g, b;
	Interval ourValidity = os->obj->ChannelValidity (t, TOPO_CHAN_NUM);
	mpParams->GetValue (pb_channel, t, channel, ourValidity);
	mpParams->GetValue (pb_red, t, r, ourValidity);
	mpParams->GetValue (pb_green, t, g, ourValidity);
	mpParams->GetValue (pb_blue, t, b, ourValidity);

	// Get at our SampleFaceData:
	SampleFaceData *pFaceData = dynamic_cast<SampleFaceData *>(pFDMgr->GetFaceDataChan (FACE_MAXSAMPLEUSE_CLSID));

	// Apply the colors - different code depending on object type:
	if (pMesh) {
		pMesh->setMapSupport (-channel, true);
		pMesh->setNumMapVerts (-channel, 3*numFaces);
		TVFace *pFace = pMesh->mapFaces(-channel);
		VertColor *pColor = pMesh->mapVerts (-channel);

		int maxFaceData = pFaceData ? pFaceData->Count() : 0;

		for (int i=0; i<numFaces; i++) {
			for (int j=0; j<3; j++) {
				int k = i*3+j;
				pFace[i].t[j] = k;
				if (i<maxFaceData) {
					float fdValue = pFaceData->data[i];
					VertColor val;
					val.x = r*fdValue;
					if (val.x < 0) val.x = 0.0f;
					if (val.x > 1) val.x = 1.0f;
					val.y = g*fdValue;
					if (val.y < 0) val.y = 0.0f;
					if (val.y > 1) val.y = 1.0f;
					val.z = b*fdValue;
					if (val.z < 0) val.z = 0.0f;
					if (val.z > 1) val.z = 1.0f;
					pColor[k] = val;
				} else pColor[k] = VertColor(0,0,0);
			}
		}
	}

	if (pMNMesh) {
		if ((channel == 0) && (pMNMesh->numm == 0)) pMNMesh->SetMapNum (1);
		pMNMesh->M(-channel)->ClearFlag (MN_DEAD);
		pMNMesh->M(-channel)->setNumFaces (numFaces);

		// Precount the number of map vertices we need:
		int numColors = 0;
		for (int i=0; i<numFaces; i++) {
			if (pMNMesh->f[i].GetFlag (MN_DEAD)) continue;
			numColors += pMNMesh->f[i].deg;
		}
		pMNMesh->M(-channel)->setNumVerts (numColors);

		MNMapFace *pFace = pMNMesh->M(-channel)->f;
		VertColor *pColor = pMNMesh->M(-channel)->v;

		int maxFaceData = pFaceData ? pFaceData->Count() : 0;

		int k=0;
		for (i=0; i<numFaces; i++) {
			if (pMNMesh->f[i].GetFlag (MN_DEAD)) continue;
			pFace[i].SetSize (pMNMesh->f[i].deg);
			for (int j=0; j<pMNMesh->f[i].deg; j++) {
				pFace[i].tv[j] = k;
				if (i<maxFaceData) {
					float fdValue = pFaceData->data[i];
					VertColor val;
					val.x = r*fdValue;
					if (val.x < 0) val.x = 0.0f;
					if (val.x > 1) val.x = 1.0f;
					val.y = g*fdValue;
					if (val.y < 0) val.y = 0.0f;
					if (val.y > 1) val.y = 1.0f;
					val.z = b*fdValue;
					if (val.z < 0) val.z = 0.0f;
					if (val.z > 1) val.z = 1.0f;
					pColor[k] = val;
				} else pColor[k] = VertColor(0,0,0);
				k++;
			}
		}
	}

	os->obj->SetChannelValidity (VERT_COLOR_CHAN_NUM, ourValidity);
}

void FaceDataToColorMod::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	mpInterface = ip;
	theDlgProc.SetMod (this);
	mpMap = CreateCPParamMap2 (data_to_color_params, mpParams, ip, hInstance,
		MAKEINTRESOURCE (IDD_DATA_TO_COLOR), GetString (IDS_PARAMS),
		0, &theDlgProc);
}

void FaceDataToColorMod::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	theDlgProc.SetMod (NULL);
	if (mpMap) {
		DestroyCPParamMap2 (mpMap);
		mpMap = NULL;
	}
	mpInterface = NULL;
}

//From ReferenceMaker 
RefResult FaceDataToColorMod::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) {
	return REF_SUCCEED;
}

void FaceDataToColorMod::DisplayColors () {
	if (!mpInterface) return;

	int channel = 0;
	if (mpParams) mpParams->GetValue (pb_channel, TimeValue(0), channel, FOREVER);

	ModContextList mcList;
	INodeTab nodes;
	mpInterface->GetModContexts(mcList,nodes);

	for (int nd = 0; nd<nodes.Count(); nd++) {
		nodes[nd]->SetCVertMode (true);
		nodes[nd]->SetShadeCVerts (true);
		if (channel) nodes[nd]->SetVertexColorType (nvct_illumination);
		else nodes[nd]->SetVertexColorType (nvct_color);
		//nodes[nd]->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	}

	// KLUGE: above notify dependents call only seems to set a limited refresh region.  Result looks bad.
	// So we do this instead:
	NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	mpInterface->RedrawViews (mpInterface->GetTime());
}

