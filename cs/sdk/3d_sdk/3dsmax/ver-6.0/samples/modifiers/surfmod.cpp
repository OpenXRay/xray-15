/**********************************************************************
 *<
	FILE: surfmod.cpp

	DESCRIPTION:  Varius surface modifiers

	CREATED BY: Rolf Berteig

	HISTORY: 11/07/95

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "mods.h"
#include "iparamm2.h"
//#include "MeshDLib.h"
#include "resourceOverride.h"

#ifndef NO_MODIFIER_MATERIAL

// Version info added for MAXr4, where we can now be applied to a PatchMesh and retain identity!
#define MATMOD_VER1 1
#define MATMOD_VER4 4

#define MATMOD_CURRENT_VERSION MATMOD_VER4

class MatMod : public Modifier {	
	public:
		IParamBlock *pblock;
		static IParamMap *pmapParam;
		int version;

		MatMod();

		// From Animatable
		void DeleteThis() { delete this; }
		void GetClassName(TSTR& s) { s= GetString(IDS_RB_MATMOD); }  
		virtual Class_ID ClassID() { return Class_ID(MATERIALOSM_CLASS_ID,0);}
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip,ULONG flags,Animatable *next);		
		TCHAR *GetObjectName() { return GetString(IDS_RB_MATERIAL3); }
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 

		ChannelMask ChannelsUsed()  {return OBJ_CHANNELS;}
		ChannelMask ChannelsChanged() {return GEOM_CHANNEL|TOPO_CHANNEL;}
		Class_ID InputType() {return defObjectClassID;}
		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		Interval LocalValidity(TimeValue t);
		IParamArray *GetParamBlock() {return pblock;}
		int GetParamBlockIndex(int id) {return id;}

		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i) {return pblock;}
		void SetReference(int i, RefTargetHandle rtarg) {pblock = (IParamBlock*)rtarg;}

		int NumSubs() {return 1;}
		Animatable* SubAnim(int i) {return pblock;}
		TSTR SubAnimName(int i) {return GetString(IDS_RB_PARAMETERS);}

		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);
		// IO
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
	};



//--- ClassDescriptor and class vars ---------------------------------

IParamMap *MatMod::pmapParam = NULL;



class MatClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new MatMod; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_MATERIAL3_CLASS); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(MATERIALOSM_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFSURFACE);}
	};

static MatClassDesc matDesc;
extern ClassDesc* GetMatModDesc() { return &matDesc; }


//--- Parameter map/block descriptors -------------------------------

#define PB_MATID 0

//
//
// Parameters

static ParamUIDesc descParam[] = {
	
	// Material ID
	ParamUIDesc(
		PB_MATID,
		EDITTYPE_INT,
		IDC_MATID,IDC_MATIDSPIN,
		1.0f,(float)0xffff,
		0.1f),	
	};
#define PARAMDESC_LENGH 1


static ParamBlockDescID descVer0[] = {
	{ TYPE_INT, NULL, TRUE, 0 }};
#define PBLOCK_LENGTH	1

#define CURRENT_VERSION	0


//--- MatMod methods -------------------------------


MatMod::MatMod()
	{	
	MakeRefByID(FOREVER, 0, 
		CreateParameterBlock(descVer0, PBLOCK_LENGTH, CURRENT_VERSION));	
	pblock->SetValue(PB_MATID,0,1);
	version = MATMOD_CURRENT_VERSION;
	}

void MatMod::BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev)
	{
	pmapParam = CreateCPParamMap(
		descParam,PARAMDESC_LENGH,
		pblock,
		ip,
		hInstance,
		MAKEINTRESOURCE(IDD_MATERIALPARAM),
		GetString(IDS_RB_PARAMETERS),
		0);		
	}
		
void MatMod::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
	{	
	DestroyCPParamMap(pmapParam);
	pmapParam = NULL;
	}

Interval MatMod::LocalValidity(TimeValue t)
	{
	int i;
	Interval valid = FOREVER;
	pblock->GetValue(PB_MATID,t,i,valid);	
	return valid;
	}

RefTargetHandle MatMod::Clone(RemapDir& remap) 
	{
	MatMod* newmod = new MatMod();	
	newmod->ReplaceReference(0,pblock->Clone(remap));	
	newmod->version = version;
	BaseClone(this, newmod, remap);
	return newmod;
	}

static void DoMaterialSet(TriObject *triOb, int id) {
	BOOL useSel = triOb->GetMesh().selLevel == MESH_FACE;

	for (int i=0; i<triOb->GetMesh().getNumFaces(); i++) {
		if (!useSel || triOb->GetMesh().faceSel[i]) {
			triOb->GetMesh().setFaceMtlIndex(i,(MtlID)id);
			}
		}
	triOb->GetMesh().InvalidateTopologyCache();
	}

void MatMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node)
	{
	Interval valid = FOREVER;
	int id;
	pblock->GetValue(PB_MATID,t,id,valid);	
	id--;
	if (id<0) id = 0;
	if (id>0xffff) id = 0xffff;

	// For version 4 and later, we process patch meshes as they are and pass them on.  Earlier
	// versions converted to TriMeshes (done below).  For adding other new types of objects, add
	// them here!
#ifndef NO_PATCHES
	if(version >= MATMOD_VER4 && os->obj->IsSubClassOf(patchObjectClassID)) {
		PatchObject *patchOb = (PatchObject *)os->obj;
		PatchMesh &pmesh = patchOb->GetPatchMesh(t);
		BOOL useSel = pmesh.selLevel >= PO_PATCH;

		for (int i=0; i<pmesh.getNumPatches(); i++) {
			if (!useSel || pmesh.patchSel[i]) {
				pmesh.setPatchMtlIndex(i,(MtlID)id);
				}
			}
		pmesh.InvalidateGeomCache();	// Do this because there isn't a topo cache in PatchMesh
						
		patchOb->UpdateValidity(TOPO_CHAN_NUM,valid);		
		}
	else
#endif // NO_PATCHES
	// Process PolyObjects
	if(os->obj->IsSubClassOf(polyObjectClassID)) {
		PolyObject *polyOb = (PolyObject *)os->obj;
		MNMesh &mesh = polyOb->GetMesh();
		BOOL useSel = mesh.selLevel == MNM_SL_FACE;

		for (int i=0; i<mesh.numf; i++) {
			if (!useSel || mesh.f[i].GetFlag(MN_SEL)) {
				mesh.f[i].material = (MtlID)id;
			}
		}
		polyOb->UpdateValidity(TOPO_CHAN_NUM,valid);		
	}
	else	// If it's a TriObject, process it
	if(os->obj->IsSubClassOf(triObjectClassID)) {
		TriObject *triOb = (TriObject *)os->obj;
		DoMaterialSet(triOb, id);
		triOb->UpdateValidity(TOPO_CHAN_NUM,valid);		
		}
	else	// Fallback position: If it can convert to a TriObject, do it!
	if(os->obj->CanConvertToType(triObjectClassID)) {
		TriObject  *triOb = (TriObject *)os->obj->ConvertToType(t, triObjectClassID);
		// Now stuff this into the pipeline!
		os->obj = triOb;

		DoMaterialSet(triOb, id);
		triOb->UpdateValidity(TOPO_CHAN_NUM,valid);		
		}
	else
		return;		// Do nothing if it can't convert to triObject
	}

RefResult MatMod::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
   		PartID& partID, 
   		RefMessage message) 
   	{
	switch (message) {
		case REFMSG_CHANGE:
			if (pmapParam && pmapParam->GetParamBlock()==pblock) {
				pmapParam->Invalidate();
				}
			break;

		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			switch (gpd->index) {				
				case 0:
				default: gpd->dim = defaultDim; break;
				}			
			return REF_STOP; 
			}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			switch (gpn->index) {				
				case PB_MATID:	gpn->name = GetString(IDS_RB_MATERIALID); break;
				default:		gpn->name = TSTR(_T("")); break;
				}
			return REF_STOP; 
			}
		}
	return REF_SUCCEED;
	}

#define MATMOD_VERSION_CHUNK	0x1000

IOResult MatMod::Save(ISave *isave) {
	ULONG nb;
	Modifier::Save(isave);
	isave->BeginChunk (MATMOD_VERSION_CHUNK);
	isave->Write (&version, sizeof(int), &nb);
	isave->EndChunk();
	return IO_OK;
	}

IOResult MatMod::Load(ILoad *iload) {
	Modifier::Load(iload);
	IOResult res;
	ULONG nb;
	version = MATMOD_VER1;	// Set default version to old one
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case MATMOD_VERSION_CHUNK:
				res = iload->Read(&version,sizeof(int),&nb);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

#endif

//-------------------------------------------------------------------
//-------------------------------------------------------------------

// Version info added for MAXr4, where we can now be applied to a PatchMesh and retain identity!
#define SMOOTHMOD_VER1 1
#define SMOOTHMOD_VER4 4

#define SMOOTHMOD_CURRENT_VERSION SMOOTHMOD_VER4

// References in SmoothMod:
enum { REF_SMOOTH_PBLOCK };

class SmoothMod : public Modifier {	
public:
	IParamBlock2 *pblock;
	//static IParamMap *pmapParam;
	int version;

	SmoothMod();

	// From Animatable
	void DeleteThis() { delete this; }
	void GetClassName(TSTR& s) { s= GetString(IDS_RB_SMOOTHMOD); }  
	virtual Class_ID ClassID() { return Class_ID(SMOOTHOSM_CLASS_ID,0);}
	void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
	void EndEditParams( IObjParam *ip,ULONG flags,Animatable *next);		
	TCHAR *GetObjectName() { return GetString(IDS_RB_SMOOTH2); }
	CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 

	ChannelMask ChannelsUsed()  {return OBJ_CHANNELS;}
	ChannelMask ChannelsChanged() {return GEOM_CHANNEL|TOPO_CHANNEL;}
	Class_ID InputType() {return defObjectClassID;}
	void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
	Interval LocalValidity(TimeValue t);

	int NumParamBlocks () { return 1; }
	IParamBlock2 *GetParamBlock(int i) {return pblock;}
	IParamBlock2 *GetParamBlockByID (short id) { return (pblock->ID() == id) ? pblock : NULL; }
	//int GetParamBlockIndex(int id) {return id;}

	int NumRefs() {return 1;}
	RefTargetHandle GetReference(int i) {return pblock;}
	void SetReference(int i, RefTargetHandle rtarg) {pblock = (IParamBlock2*)rtarg;}
	IOResult Load(ILoad *iload);

	int NumSubs() {return 1;}
	Animatable* SubAnim(int i) {return pblock;}
	TSTR SubAnimName(int i) {return GetString(IDS_RB_PARAMETERS);}

	RefTargetHandle Clone(RemapDir& remap = NoRemap());
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);
	// IO
	IOResult Save(ISave *isave);
};



//--- ClassDescriptor and class vars ---------------------------------
class SmoothClassDesc : public ClassDesc2 {
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) {return new SmoothMod;}
	const TCHAR *	ClassName() { return GetString(IDS_RB_SMOOTH2_CLASS); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(SMOOTHOSM_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFSURFACE);}
	const TCHAR*	InternalName()				{ return _T("SmoothModifier"); }	// for scripter.
	HINSTANCE		HInstance()					{ return hInstance; }
};

static SmoothClassDesc smoothDesc;
extern ClassDesc* GetSmoothModDesc() { return &smoothDesc; }


//--- Parameter map/block descriptors -------------------------------

// Enumerate parameter blocks:
enum { smooth_params };
// Parameters in the block:
enum { sm_autosmooth, sm_threshold, sm_smoothbits, sm_prevent_indirect };

static ParamBlockDesc2 smooth_param_desc (smooth_params, _T("smoothParams"),
									IDS_RB_PARAMETERS, &smoothDesc,
									P_AUTO_CONSTRUCT | P_AUTO_UI, REF_SMOOTH_PBLOCK,
	// Rollout description:
	IDD_SMOOTHPARAM, IDS_RB_PARAMETERS, 0, 0, NULL,

	// params
	sm_autosmooth, _T("autoSmooth"), TYPE_BOOL, P_RESET_DEFAULT, IDS_RB_AUTOSMOOTH,
#ifdef USE_SIMPLIFIED_SMOOTH_MODIFIER_UI
		p_default, true,
#else
		p_default, false,
#endif
		p_enable_ctrls, 2, sm_threshold, sm_prevent_indirect,
		p_ui, TYPE_SINGLECHEKBOX, IDC_SMOOTH_AUTO,
		end,

	sm_prevent_indirect, _T("preventIndirect"), TYPE_BOOL, P_RESET_DEFAULT, IDS_PREVENT_INDIRECT,
		p_default, false,
		p_ui, TYPE_SINGLECHEKBOX, IDC_SMOOTH_PREVENTINDIRECT,
		end,

	sm_threshold, _T("threshold"), TYPE_ANGLE, P_RESET_DEFAULT|P_ANIMATABLE, IDS_RB_THRESHOLD,
		p_default, PI/6.0f,
		p_range, 0.0f, 180.0f,
		p_ui, TYPE_SPINNER, EDITTYPE_POS_FLOAT,
			IDC_SMOOTH_THRESH, IDC_SMOOTH_THRESHSPIN, .1f,
		end,

	// NOTE That this should be TYPE_DWORD, but that type isn't yet supported in paramblocks.
	sm_smoothbits, _T("smoothingBits"), TYPE_INT, P_RESET_DEFAULT, IDS_VS_SMGROUP,
		p_default, 0,
		// No UI - we handle the UI ourselves.
		end,
	end
);

//--- SmoothModForceAutoSmoothOn --------------------------------

#ifdef USE_SIMPLIFIED_SMOOTH_MODIFIER_UI
class SmoothModForceAutoSmoothOn : public PostLoadCallback {
	SmoothMod* mod;
	int pbRefNum;
public:
	SmoothModForceAutoSmoothOn(SmoothMod* m, int ref)
		: mod(m), pbRefNum(ref) {}

	// Ensure this happens late among PLCBs; in order to properly support really
	// old files, it must execute after the ParamBlock2PLCB has converted the PB
	// to a PB2.
	int Priority() { return 8; }
	void proc(ILoad *iload) {
		IParamBlock2* pb2 = static_cast<IParamBlock2*>(mod->GetReference(pbRefNum));
		if (pb2 != NULL)
			pb2->SetValue(sm_autosmooth, 0, 1);
		delete this;
	}
};
#endif

//--- SmoothDlgProc --------------------------------

class SmoothDlgProc : public ParamMap2UserDlgProc {
	SmoothMod *mod;
	bool uiValid;
public:
	SmoothDlgProc () : mod(NULL), uiValid(false) { }
	BOOL DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd,
		UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() { }
	void Update (HWND hWnd, TimeValue t);
	void SetMod (SmoothMod *m) { mod=m; }
	void Invalidate (HWND hWnd) { uiValid = false; InvalidateRect (hWnd, NULL, false); }
};

static SmoothDlgProc theSmoothDlgProc;

// NOTE: This depends on the IDC_SMOOTH indices being sequential!
void SmoothDlgProc::Update (HWND hWnd, TimeValue t) {
	if (!mod || !mod->pblock || !hWnd) return;

	int autoSmooth;
	mod->pblock->GetValue (sm_autosmooth, t, autoSmooth, FOREVER);

#ifndef USE_SIMPLIFIED_SMOOTH_MODIFIER_UI
	int bits;
	mod->pblock->GetValue (sm_smoothbits, t, bits, FOREVER);
	for (int i=IDC_SMOOTH_GRP1; i<IDC_SMOOTH_GRP1+32; i++) {
		ICustButton *iBut = GetICustButton(GetDlgItem(hWnd,i));
		iBut->SetCheck ((bits & (1<<(i-IDC_SMOOTH_GRP1))) ? true : false);
		iBut->Enable (!autoSmooth);
		ReleaseICustButton (iBut);
	}

	EnableWindow (GetDlgItem (hWnd, IDC_SMOOTH_GROUP_BOX), !autoSmooth);
#endif

	EnableWindow (GetDlgItem (hWnd, IDC_SMOOTH_THRESH_LABEL), autoSmooth);
	uiValid = true;
}

BOOL SmoothDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd,
							UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_INITDIALOG:
#ifndef USE_SIMPLIFIED_SMOOTH_MODIFIER_UI
		{
			for (int i=IDC_SMOOTH_GRP1; i<IDC_SMOOTH_GRP1+32; i++) {
				ICustButton *iBut = GetICustButton(GetDlgItem(hWnd,i));
				iBut->SetType (CBT_CHECK);
				ReleaseICustButton (iBut);
			}
		}
#endif
		uiValid = false;
		break;

	case WM_PAINT:
		if (uiValid) break;
		Update (hWnd, t);
		break;

	case WM_COMMAND:
		if (LOWORD(wParam)>=IDC_SMOOTH_GRP1 &&
			LOWORD(wParam)<=IDC_SMOOTH_GRP32) {
			IParamBlock2 *pblock = (IParamBlock2*)map->GetParamBlock();
			ICustButton *iBut = GetICustButton(GetDlgItem(hWnd,LOWORD(wParam)));
			int bits;
			pblock->GetValue (sm_smoothbits, t, bits, FOREVER);
			int shift = LOWORD(wParam) - IDC_SMOOTH_GRP1;
			if (iBut->IsChecked()) bits |= 1<<shift;
			else bits &= ~(1<<shift);
			theHold.Begin ();
			pblock->SetValue (sm_smoothbits, t, bits);
			theHold.Accept (GetString (IDS_VS_SMGROUP));
			ReleaseICustButton(iBut);
			return REDRAW_VIEWS;
		}
		break;
	
	default:
		return TRUE;
	}
	return FALSE;
}

//--- SmoothMod methods -------------------------------

SmoothMod::SmoothMod() {
	pblock = NULL;
	version = SMOOTHMOD_CURRENT_VERSION;
	smoothDesc.MakeAutoParamBlocks (this);
}

// Old parameter block descriptions:
static ParamBlockDescID descSmoothVer0[] = {
	{ TYPE_INT, NULL, FALSE, sm_autosmooth },
	{ TYPE_FLOAT, NULL, TRUE, sm_threshold },
	{ TYPE_INT, NULL, FALSE, sm_smoothbits }
};

static ParamBlockDescID descSmoothVer1[] = {
	{ TYPE_INT,   NULL, FALSE, sm_autosmooth },
	{ TYPE_FLOAT, NULL, TRUE, sm_threshold },
	{ TYPE_INT,   NULL, FALSE, sm_smoothbits },
	{ TYPE_INT,   NULL, FALSE, sm_prevent_indirect },
};

// Array of old versions
static ParamVersionDesc versionsSmooth[] = {
	ParamVersionDesc(descSmoothVer0,3,0),
	ParamVersionDesc(descSmoothVer1,4,1)
};
#define NUM_OLDSMOOTHVERSIONS	2

#define SMOOTHMOD_VERSION_CHUNK	0x1000

IOResult SmoothMod::Load(ILoad *iload) {
	Modifier::Load(iload);
	iload->RegisterPostLoadCallback(
		new ParamBlock2PLCB(versionsSmooth, NUM_OLDSMOOTHVERSIONS, &smooth_param_desc,
							this, REF_SMOOTH_PBLOCK));
	IOResult res;
	ULONG nb;
	version = SMOOTHMOD_VER1;	// Set default version to old one
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
		case SMOOTHMOD_VERSION_CHUNK:
			res = iload->Read(&version,sizeof(int),&nb);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}

#ifdef USE_SIMPLIFIED_SMOOTH_MODIFIER_UI
	// In Kahn 'Auto smooth' should always be on regardless of what is in the file.
	iload->RegisterPostLoadCallback(
		new SmoothModForceAutoSmoothOn(this, REF_SMOOTH_PBLOCK));
#endif

	return IO_OK;
}

IOResult SmoothMod::Save(ISave *isave) {
	ULONG nb;
	Modifier::Save(isave);
	isave->BeginChunk (SMOOTHMOD_VERSION_CHUNK);
	isave->Write (&version, sizeof(int), &nb);
	isave->EndChunk();
	return IO_OK;
}

void SmoothMod::BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev) {
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	SetAFlag (A_MOD_BEING_EDITED);

	smoothDesc.BeginEditParams (ip, this, flags, prev);
	theSmoothDlgProc.SetMod (this);
	smoothDesc.SetUserDlgProc (&smooth_param_desc, &theSmoothDlgProc);
}

void SmoothMod::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next) {
	ClearAFlag (A_MOD_BEING_EDITED);
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);

	smoothDesc.EndEditParams (ip, this, flags, next);
	theSmoothDlgProc.SetMod (NULL);
}

Interval SmoothMod::LocalValidity(TimeValue t) {
  // aszabo|feb.05.02 If we are being edited, return NEVER 
	// to forces a cache to be built after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;
	float f;
	Interval valid = FOREVER;
	// Only one animatable parameter:
	pblock->GetValue(sm_threshold,t,f,valid);	
	return valid;
}

RefTargetHandle SmoothMod::Clone(RemapDir& remap) {
	SmoothMod* newmod = new SmoothMod();
	newmod->ReplaceReference (REF_SMOOTH_PBLOCK, pblock->Clone(remap));
	newmod->version = version;
	BaseClone(this, newmod, remap);
	return newmod;
}

void SmoothMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node) {
	Interval valid = FOREVER;
	int autoSmooth, bits, prevent;
	float thresh;
	pblock->GetValue(sm_autosmooth, t, autoSmooth, valid);
	if (autoSmooth) {
		pblock->GetValue(sm_threshold, t, thresh, valid);
		pblock->GetValue(sm_prevent_indirect, t, prevent, valid);
	} else {
		pblock->GetValue(sm_smoothbits, t, bits, valid);
	}
	// For version 4 and later, we process patch meshes as they are and pass them on.  Earlier
	// versions converted to TriMeshes (done below).  For adding other new types of objects, add
	// them here!
	bool done = false;
#ifndef NO_PATCHES
	if(version >= MATMOD_VER4 && os->obj->IsSubClassOf(patchObjectClassID)) {
		PatchObject *patchOb = (PatchObject *)os->obj;
		PatchMesh &pmesh = patchOb->GetPatchMesh(t);
		BOOL useSel = pmesh.selLevel >= PO_PATCH;

		if (!autoSmooth) pmesh.AutoSmooth (thresh, useSel, prevent);
		else {
			for (int i=0; i<pmesh.getNumPatches(); i++) {
				if (!useSel || pmesh.patchSel[i]) pmesh.patches[i].smGroup = (DWORD)bits;
			}
		}
		pmesh.InvalidateGeomCache();	// Do this because there isn't a topo cache in PatchMesh
		patchOb->UpdateValidity(TOPO_CHAN_NUM,valid);
		done = true;
	}
#endif // NO_PATCHES
	if (!done && os->obj->IsSubClassOf (polyObjectClassID)) {
		PolyObject *pPolyOb = (PolyObject *)os->obj;
		MNMesh &mesh = pPolyOb->GetMesh();

		BOOL useSel = (mesh.selLevel == MNM_SL_FACE);
		if (autoSmooth) mesh.AutoSmooth (thresh, useSel, prevent);
		else {
			for (int faceIndex=0; faceIndex<mesh.FNum(); faceIndex++) {
				if (!useSel || mesh.F(faceIndex)->GetFlag(MN_SEL)) {
					mesh.F(faceIndex)->smGroup = (DWORD)bits;
				}
			}
		}

		// Luna task 747
		// We need to rebuild the smoothing-group-based normals in the normalspec, if any:
		if (mesh.GetSpecifiedNormals()) {
			mesh.GetSpecifiedNormals()->SetParent (&mesh);
			mesh.GetSpecifiedNormals()->BuildNormals ();
		}

		pPolyOb->UpdateValidity(TOPO_CHAN_NUM,valid);
		done = true;
	}

	TriObject *triOb = NULL;
	if (!done) {
		if (os->obj->IsSubClassOf(triObjectClassID)) triOb = (TriObject *)os->obj;
		else {
			// Convert to triobject if we can.
			if(os->obj->CanConvertToType(triObjectClassID)) {
				TriObject  *triOb = (TriObject *)os->obj->ConvertToType(t, triObjectClassID);

				// We'll need to stuff this back into the pipeline:
				os->obj = triOb;

				// Convert validities:
				Interval objValid = os->obj->ChannelValidity (t, TOPO_CHAN_NUM);
				triOb->SetChannelValidity (TOPO_CHAN_NUM, objValid);
				triOb->SetChannelValidity (GEOM_CHAN_NUM,
					objValid & os->obj->ChannelValidity (t, GEOM_CHAN_NUM));
				triOb->SetChannelValidity (TEXMAP_CHAN_NUM,
					objValid & os->obj->ChannelValidity (t, TEXMAP_CHAN_NUM));
				triOb->SetChannelValidity (VERT_COLOR_CHAN_NUM,
					objValid & os->obj->ChannelValidity (t, VERT_COLOR_CHAN_NUM));
				triOb->SetChannelValidity (DISP_ATTRIB_CHAN_NUM,
					objValid & os->obj->ChannelValidity (t, DISP_ATTRIB_CHAN_NUM));
			}
		}
	}

	if (triOb) {	// one way or another, there's a triobject to smooth.
		Mesh & mesh = triOb->GetMesh();
		BOOL useSel = mesh.selLevel == MESH_FACE;
		if (autoSmooth) mesh.AutoSmooth (thresh, useSel, prevent);
		else {
			for (int i=0; i<mesh.getNumFaces(); i++) {
				if (!useSel || mesh.faceSel[i]) mesh.faces[i].smGroup = (DWORD)bits;
			}
		}
		triOb->GetMesh().InvalidateTopologyCache();
		triOb->UpdateValidity(TOPO_CHAN_NUM,valid);		
	}
}

RefResult SmoothMod::NotifyRefChanged (Interval changeInt, RefTargetHandle hTarget,
									   PartID& partID, RefMessage message) {
	switch (message) {
	case REFMSG_CHANGE:
		if (hTarget == pblock) {
			int idToUpdate = pblock->LastNotifyParamID();
			smooth_param_desc.InvalidateUI (idToUpdate);
			switch (idToUpdate) {
			case -1:
			case sm_smoothbits:
			case sm_autosmooth:
				if (smoothDesc.NumParamMaps() > 0) {
					IParamMap2 *pmap = smoothDesc.GetParamMap(0);
					if (pmap) {
						HWND hWnd = pmap->GetHWnd();
						if (hWnd) theSmoothDlgProc.Invalidate (hWnd);
					}
				}
				break;
			}
		}
		break;
	}
	return REF_SUCCEED;
}


//-----------------------------------------------------------
//-----------------------------------------------------------

// Version info added for MAXr4, where we can now be applied to a PatchMesh and retain identity!
#define NORMALMOD_VER1 1
#define NORMALMOD_VER4 4

#define NORMALMOD_CURRENT_VERSION NORMALMOD_VER4

class NormalMod : public Modifier {	
	public:
		IParamBlock *pblock;
		static IParamMap *pmapParam;
		int version;

		NormalMod();

		// From Animatable
		void DeleteThis() { delete this; }
		void GetClassName(TSTR& s) { s= GetString(IDS_RB_NORMALMOD); }  
		virtual Class_ID ClassID() { return Class_ID(NORMALOSM_CLASS_ID,0);}
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip,ULONG flags,Animatable *next);		
		TCHAR *GetObjectName() { return GetString(IDS_RB_NORMAL); }
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 

		ChannelMask ChannelsUsed()  {return OBJ_CHANNELS;}
		ChannelMask ChannelsChanged() {return PART_TOPO|PART_TEXMAP|PART_VERTCOLOR;}
		Class_ID InputType() {return defObjectClassID;}
		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		Interval LocalValidity(TimeValue t);
		IParamArray *GetParamBlock() {return pblock;}
		int GetParamBlockIndex(int id) {return id;}

		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i) {return pblock;}
		void SetReference(int i, RefTargetHandle rtarg) {pblock = (IParamBlock*)rtarg;}

		int NumSubs() {return 1;}
		Animatable* SubAnim(int i) {return pblock;}
		TSTR SubAnimName(int i) {return GetString(IDS_RB_PARAMETERS);}

		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);
		// IO
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);
	};



//--- ClassDescriptor and class vars ---------------------------------

IParamMap *NormalMod::pmapParam = NULL;



class NormalClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new NormalMod;}
	const TCHAR *	ClassName() { return GetString(IDS_RB_NORMAL_CLASS); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(NORMALOSM_CLASS_ID,0);}
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFSURFACE);}
	};

static NormalClassDesc normalDesc;
extern ClassDesc* GetNormalModDesc() { return &normalDesc; }


//--- Parameter map/block descriptors -------------------------------

#define PB_UNIFY	0
#define PB_FLIP 	1

//
//
// Parameters

static ParamUIDesc descNormParam[] = {
	// Unify
	ParamUIDesc(PB_UNIFY,TYPE_SINGLECHEKBOX,IDC_NORM_UNIFY),

	// Flip
	ParamUIDesc(PB_FLIP,TYPE_SINGLECHEKBOX,IDC_NORM_FLIP),
	};
#define NORMPARAMDESC_LENGH 2


static ParamBlockDescID descNormVer0[] = {
	{ TYPE_INT, NULL, FALSE, 0 },
	{ TYPE_INT, NULL, FALSE, 1 }};
#define NORMPBLOCK_LENGTH	2

#define CURRENT_NORMVERSION	0


//--- NormalMod methods -------------------------------


NormalMod::NormalMod()
	{	
	MakeRefByID(FOREVER, 0, 
		CreateParameterBlock(descNormVer0, NORMPBLOCK_LENGTH, CURRENT_NORMVERSION));	
		pblock->SetValue(PB_FLIP,0,1);
	version = NORMALMOD_CURRENT_VERSION;
#ifdef WEBVERSION	// russom - 04/29/02
	pblock->SetValue(PB_FLIP,0,1);
#endif
	}

#define NORMALMOD_VERSION_CHUNK	0x1000

IOResult NormalMod::Load(ILoad *iload)
	{
	Modifier::Load(iload);
	IOResult res;
	ULONG nb;
	version = NORMALMOD_VER1;	// Set default version to old one
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case NORMALMOD_VERSION_CHUNK:
				res = iload->Read(&version,sizeof(int),&nb);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

IOResult NormalMod::Save(ISave *isave) {
	ULONG nb;
	Modifier::Save(isave);
	isave->BeginChunk (NORMALMOD_VERSION_CHUNK);
	isave->Write (&version, sizeof(int), &nb);
	isave->EndChunk();
	return IO_OK;
	}

void NormalMod::BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev)
	{
	pmapParam = CreateCPParamMap(
		descNormParam,NORMPARAMDESC_LENGH,
		pblock,
		ip,
		hInstance,
		MAKEINTRESOURCE(IDD_NORMALPARAM),
		GetString(IDS_RB_PARAMETERS),
		0);		
	}
		
void NormalMod::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
	{	
	DestroyCPParamMap(pmapParam);
	pmapParam = NULL;
	}

Interval NormalMod::LocalValidity(TimeValue t)
	{	
	return FOREVER;
	}

RefTargetHandle NormalMod::Clone(RemapDir& remap) 
	{
	NormalMod* newmod = new NormalMod();	
	newmod->ReplaceReference(0,pblock->Clone(remap));	
	BaseClone(this, newmod, remap);
	return newmod;
	}

void FlipMeshNormal(Mesh *mesh,DWORD face) {
	mesh->FlipNormal(int(face));
	/*
	DWORD vis  = 0;
	if (mesh->faces[face].flags&EDGE_A) vis |= EDGE_A;
	if (mesh->faces[face].flags&EDGE_B) vis |= EDGE_C;
	if (mesh->faces[face].flags&EDGE_C) vis |= EDGE_B;
	DWORD temp = mesh->faces[face].v[0];
	mesh->faces[face].v[0] = mesh->faces[face].v[1];
	mesh->faces[face].v[1] = temp;				
	mesh->faces[face].flags &= ~EDGE_ALL;
	mesh->faces[face].flags |= vis;
	if (mesh->tvFace) {		
		temp = mesh->tvFace[face].t[0];
		mesh->tvFace[face].t[0] = mesh->tvFace[face].t[1];
		mesh->tvFace[face].t[1] = temp;		
	}
	*/
}

static void DoNormalSet(TriObject *triOb, BOOL unify, BOOL flip) {
	BOOL useSel = triOb->GetMesh().selLevel == MESH_FACE;

	if (unify) {
		triOb->GetMesh().UnifyNormals(useSel);
		triOb->GetMesh().InvalidateTopologyCache ();
		}

	if (flip) {
		for (int i=0; i<triOb->GetMesh().getNumFaces(); i++) {
			if (!useSel || triOb->GetMesh().faceSel[i])
				FlipMeshNormal(&triOb->GetMesh(),(DWORD)i);
			}
		triOb->GetMesh().InvalidateTopologyCache ();
		}
	}

void NormalMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node)
	{
	Interval valid = FOREVER;
	int flip, unify;
	pblock->GetValue(PB_FLIP,t,flip,valid);	
	pblock->GetValue(PB_UNIFY,t,unify,valid);	

	// For version 4 and later, we process patch meshes as they are and pass them on.  Earlier
	// versions converted to TriMeshes (done below).  For adding other new types of objects, add
	// them here!
#ifndef NO_PATCHES
	if(version >= MATMOD_VER4 && os->obj->IsSubClassOf(patchObjectClassID)) {
		PatchObject *patchOb = (PatchObject *)os->obj;
		PatchMesh &pmesh = patchOb->GetPatchMesh(t);
		BOOL useSel = pmesh.selLevel >= PO_PATCH;

		if (unify)
			pmesh.UnifyNormals(useSel);

		if (flip)
			pmesh.FlipPatchNormal(useSel ? -1 : -2);
						
		patchOb->UpdateValidity(TOPO_CHAN_NUM,valid);		
		}
	else	// If it's a TriObject, process it
#endif // NO_PATCHES
	if(os->obj->IsSubClassOf(triObjectClassID)) {
		TriObject *triOb = (TriObject *)os->obj;
		DoNormalSet(triOb, unify, flip);
		triOb->UpdateValidity(TOPO_CHAN_NUM,valid);		
		}

	// Process PolyObjects
	// note: Since PolyObjects must always have the normals alligned they do not 
	// need to support unify and they do not allow normal flips on selected faces
	else if(os->obj->IsSubClassOf(polyObjectClassID)) {
		PolyObject *pPolyOb = (PolyObject *)os->obj;
		MNMesh& mesh = pPolyOb->GetMesh();


		if (flip) {
			// flip selected faces only if entire elements are selected
			if (mesh.selLevel == MNM_SL_FACE) {
				// sca 12/8/2000: Use MNMesh flipping code instead of the code that was here.
				mesh.FlipElementNormals (MN_SEL);
			} else {
				// Flip the entire object if selected elements were not flipped
				for (int i=0; i<mesh.FNum(); i++) {
					mesh.f[i].SetFlag (MN_WHATEVER, !mesh.f[i].GetFlag(MN_DEAD));
				}
				mesh.FlipElementNormals (MN_WHATEVER);
			}

			// Luna task 747:
			// We cannot support specified normals here at this time.
			mesh.ClearSpecifiedNormals ();
		}

		pPolyOb->UpdateValidity(TOPO_CHAN_NUM,valid);		
		}

	else	// Fallback position: If it can convert to a TriObject, do it!
	if(os->obj->CanConvertToType(triObjectClassID)) {
		TriObject  *triOb = (TriObject *)os->obj->ConvertToType(t, triObjectClassID);
		// Now stuff this into the pipeline!
		os->obj = triOb;

		DoNormalSet(triOb, unify, flip);
		triOb->UpdateValidity(TOPO_CHAN_NUM,valid);		
		
		}
	else
		return;		// Do nothing if it can't convert to triObject
	}

RefResult NormalMod::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
   		PartID& partID, 
   		RefMessage message) 
   	{	
	switch (message) {
		case REFMSG_CHANGE:
			if (pmapParam && pmapParam->GetParamBlock()==pblock) {
				pmapParam->Invalidate();
				}
			break;
		}
	return REF_SUCCEED;
	}
