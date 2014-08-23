/**********************************************************************
 *<
	FILE: tessmod.cpp

	DESCRIPTION:  A tessellation modifier

	CREATED BY: Rolf Berteig
	(Modified later, MNMesh stuff added, by Steve Anderson)

	HISTORY: 10/22/96
	(Modified 3/21/97)

 *>	Copyright (c) 1994, 1996, 1997, All Rights Reserved.
 **********************************************************************/

#include "mods.h"

#ifndef NO_MODIFIER_TESSELATE // JP Morel - June 28th 2002

#include "resourceOverride.h"
#include "PolyObj.h"
#include "MeshDLib.h"
#include "iparamm.h"

// LocalModData and related classes:

class TessModData : public LocalModData {
public:
	Mesh m;	// Cached output
	MNMesh mm;
	Interval ivalid;
	bool force;
	TessModData () { ivalid.SetEmpty (); force=TRUE; }
	~TessModData () { }
	LocalModData *Clone ();
};

LocalModData *TessModData::Clone () {
	TessModData *foo = new TessModData ();
	foo->ivalid = NEVER;
	foo->force = force;
	foo->m = m;
	foo->mm = m;
	return (LocalModData *) foo;
}

class TessInvalidateEnumProc : public ModContextEnumProc {
	BOOL proc (ModContext *mc);
};

BOOL TessInvalidateEnumProc::proc (ModContext *mc) {
	if (mc->localData == NULL) return TRUE;
	TessModData *msmd = (TessModData *) mc->localData;
	msmd->ivalid.SetEmpty ();
	return TRUE;
}

static TessInvalidateEnumProc theTessInvalidateEnumProc;

class TessForceEnumProc : public ModContextEnumProc {
	BOOL proc (ModContext *mc);
};

BOOL TessForceEnumProc::proc (ModContext *mc) {
	if (mc->localData == NULL) return TRUE;
	TessModData *msmd = (TessModData *) mc->localData;
	msmd->force = TRUE;
	msmd->ivalid.SetEmpty ();
	return TRUE;
}

static TessForceEnumProc theTessForceEnumProc;

static BOOL testEscape=FALSE;

#define DEF_FACE_TYPE 0	// triangle
#define DEF_TESS_TYPE 0 // edge
#define DEF_TENSION 25.0f
#define DEF_ITERS 0	// Really, 1.

// Flags:
#define TESS_ABORT 0x01
#define TESS_INRENDER 0x02

class TessMod : public Modifier, public MeshOpProgress, public FlagUser {	
public:
	IParamBlock *pblock;
	static int LastFaceType, LastType;

	static IObjParam *ip;
	static IParamMap *pmapParam;
	static TessMod *editMod;

	TessMod();

	// From Animatable
	void DeleteThis() { delete this; }
	void GetClassName(TSTR& s) {s = GetString(IDS_RB_TESSMOD);}  
	virtual Class_ID ClassID() { return Class_ID(TESSELLATE_CLASS_ID,0);}
	void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
	void EndEditParams(IObjParam *ip,ULONG flags,Animatable *next);
	RefTargetHandle Clone(RemapDir& remap = NoRemap());
	TCHAR *GetObjectName() {return GetString(IDS_RB_TESSMOD);}
	IOResult Load (ILoad *iload);

	// From modifier
	ChannelMask ChannelsUsed()  {return OBJ_CHANNELS;}
	ChannelMask ChannelsChanged() {return OBJ_CHANNELS-PART_SUBSEL_TYPE;}
	Class_ID InputType() {return mapObjectClassID;}
	void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
	Interval LocalValidity(TimeValue t);

	// From BaseObject
	CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}
	IParamArray *GetParamBlock() {return pblock;}
	int GetParamBlockIndex(int id) {return id;}

	int NumRefs() {return 1;}
	RefTargetHandle GetReference(int i) {return pblock;}
	void SetReference(int i, RefTargetHandle rtarg) {pblock=(IParamBlock*)rtarg;}

	int NumSubs() {return 1;}
	Animatable* SubAnim(int i) {return GetReference(i);}
	TSTR SubAnimName(int i) {return _T("");}

	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
	   PartID& partID, RefMessage message);
	void NotifyInputChanged (Interval changeInt, PartID part,
		RefMessage msg, ModContext *mc);

	int RenderBegin (TimeValue t, ULONG flags);
	int RenderEnd (TimeValue t);

	// MeshOpProgress:
	void SuperInit (int numSteps);
	void Init (int numSteps) {}
	BOOL Progress (int step);
};


//--- ClassDescriptor and class vars ---------------------------------

IObjParam *TessMod::ip = NULL;
IParamMap *TessMod::pmapParam = NULL;
TessMod   *TessMod::editMod = NULL;
int TessMod::LastFaceType = DEF_FACE_TYPE;
int TessMod::LastType = DEF_TESS_TYPE;

// Class to create and destroy the image list we need at startup & exit.
class faceImageHandler {
public:
	HIMAGELIST hFaceImages;
	faceImageHandler () { hFaceImages = NULL; }
	~faceImageHandler () { if (hFaceImages) ImageList_Destroy (hFaceImages); }
	void LoadImages ();
};

void faceImageHandler::LoadImages () {
	if (hFaceImages) return;
	HBITMAP hBitmap, hMask;
	hFaceImages = ImageList_Create(17, 15, ILC_MASK, 3, 0);	// 17 is kluge to center square. -SA
	hBitmap     = LoadBitmap (hInstance,MAKEINTRESOURCE(IDB_FACESELTYPES));
	hMask       = LoadBitmap (hInstance,MAKEINTRESOURCE(IDB_MASK_FACESELTYPES));
	ImageList_Add(hFaceImages, hBitmap, hMask);
	DeleteObject(hBitmap);
	DeleteObject(hMask);
}

static faceImageHandler theFaceImageHandler;

class TessClassDesc:public ClassDesc {
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new TessMod; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_TESSMOD); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(TESSELLATE_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFDEFORMATIONS);}
	void ResetClassParams (BOOL fileReset);
};

void TessClassDesc::ResetClassParams (BOOL fileReset) {
	TessMod::LastType = DEF_TESS_TYPE;
	TessMod::LastFaceType = DEF_FACE_TYPE;
}
static TessClassDesc tessDesc;
ClassDesc* GetTessModDesc() {return &tessDesc;}

//--- Parameter map/block descriptors -------------------------------

#define PB_TYPE			0
#define PB_TENSION		1
#define PB_ITERATIONS	2
#define PB_FACE_TYPE	3
#define PB_UPDATE 4

//
//
// Parameters

static int typeIDs[] = {IDC_TES_EDGE,IDC_TES_CENTER};
static int iterationsIDs[] = {IDC_ITERATIONS1,IDC_ITERATIONS2,IDC_ITERATIONS3,IDC_ITERATIONS4};
static int updateIDs[] = { IDC_TESS_UPDATE_ALWAYS, IDC_TESS_UPDATE_RENDER, IDC_TESS_UPDATE_MANUAL };

static ParamUIDesc descParam[] = {
	ParamUIDesc(PB_TYPE,TYPE_RADIO,typeIDs,2),
	ParamUIDesc (PB_TENSION, EDITTYPE_FLOAT,
		IDC_TENSION,IDC_TENSIONSPIN,
		-100.0f,100.0f, 0.1f),
	ParamUIDesc(PB_ITERATIONS,TYPE_RADIO,iterationsIDs,4),
	ParamUIDesc (PB_UPDATE, TYPE_RADIO, updateIDs, 3),
};
#define PARAMDESC_LENGTH	4

static ParamBlockDescID descVer0[] = {
	{ TYPE_INT,   NULL, FALSE, PB_TYPE },		// Type
	{ TYPE_FLOAT, NULL, TRUE,  PB_TENSION },		// Tension	
	{ TYPE_INT,   NULL, FALSE, PB_ITERATIONS },		// Iterations
};
static ParamBlockDescID descVer1[] = {
	{ TYPE_INT,   NULL, FALSE, PB_TYPE },
	{ TYPE_FLOAT, NULL, TRUE,  PB_TENSION },
	{ TYPE_INT,   NULL, FALSE, PB_ITERATIONS },
	{ TYPE_INT,   NULL, FALSE, PB_FACE_TYPE },
};
static ParamBlockDescID descVer2[] = {
	{ TYPE_INT,   NULL, FALSE, PB_TYPE },
	{ TYPE_FLOAT, NULL, TRUE,  PB_TENSION },
	{ TYPE_INT,   NULL, FALSE, PB_ITERATIONS },
	{ TYPE_INT,   NULL, FALSE, PB_FACE_TYPE },
	{ TYPE_INT,   NULL, FALSE, PB_UPDATE },
};
#define PBLOCK_LENGTH	5

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,3,0),
	ParamVersionDesc(descVer1,4,1),
};
#define NUM_OLDVERSIONS	2

#define CURRENT_VERSION 2
static ParamVersionDesc curVersion (descVer2, PBLOCK_LENGTH, CURRENT_VERSION);

// Undo/redo for tri/poly buttons:
class TriPolyRestore : public RestoreObj {
public:
	int was_poly, is_poly;
	TessMod * tm;

	TriPolyRestore (TessMod *mod);
	void Restore (int isUndo);
	void Redo ();
	int Size () { return (sizeof(TessMod *) + 2*sizeof (int)); }
	TSTR Description() { return TSTR(_T("TessMod Tri/Poly toggle")); }
};

TriPolyRestore::TriPolyRestore (TessMod *mod) {
	tm = mod;
	MaxAssert (tm);
	MaxAssert (tm->pblock);
	tm->pblock->GetValue (PB_FACE_TYPE, TimeValue(0), was_poly, FOREVER);
}

void TriPolyRestore::Restore (int isUndo) {
	if (!tm) return;
	if (!tm->pmapParam) return;
	if (tm->pblock) {
		tm->pblock->GetValue (PB_FACE_TYPE, TimeValue(0), is_poly, FOREVER);
		if (is_poly == was_poly) return;
	}
	ICustButton *iTri, *iPoly;
	iTri = GetICustButton (GetDlgItem (tm->pmapParam->GetHWnd(), IDC_TES_FACE_TRI));
	iPoly = GetICustButton (GetDlgItem (tm->pmapParam->GetHWnd(), IDC_TES_FACE_POLY));
	iTri->SetCheck (!was_poly);
	iPoly->SetCheck (was_poly);
	ReleaseICustButton (iTri);
	ReleaseICustButton (iPoly);
}

void TriPolyRestore::Redo () {
	if (!tm) return;
	if (!tm->pmapParam) return;
	if (tm->pblock) {
		int isNowPoly;
		tm->pblock->GetValue (PB_FACE_TYPE, TimeValue(0), isNowPoly, FOREVER);
		if (is_poly == isNowPoly) return;
	}
	ICustButton *iTri, *iPoly;
	iTri = GetICustButton (GetDlgItem (tm->pmapParam->GetHWnd(), IDC_TES_FACE_TRI));
	iPoly = GetICustButton (GetDlgItem (tm->pmapParam->GetHWnd(), IDC_TES_FACE_POLY));
	iTri->SetCheck (!is_poly);
	iPoly->SetCheck (is_poly);
	ReleaseICustButton (iTri);
	ReleaseICustButton (iPoly);
}

// Face-type dlg user proc
class TessModDlgProc : public ParamMapUserDlgProc {
public:
	TessMod *mod;
	TessModDlgProc () { mod = NULL; }
	BOOL DlgProc (TimeValue t, IParamMap *map, HWND hWnd,
		UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis () { }
};

BOOL TessModDlgProc::DlgProc (TimeValue t, IParamMap *map, HWND hWnd,
							  UINT msg,WPARAM wParam,LPARAM lParam) {
	if (mod==NULL) return FALSE;
//	ICustButton *iTri, *iPoly;
	switch (msg) {
	case WM_COMMAND:
		switch (LOWORD (wParam)) {
		case IDC_TES_FACE_TRI:
			theHold.Begin ();
			theHold.Put (new TriPolyRestore (mod));
// buttons handled by NotifyRefChanged - LAM - 5/7/01
//			iTri = GetICustButton (GetDlgItem (hWnd, IDC_TES_FACE_TRI));
//			iPoly = GetICustButton (GetDlgItem (hWnd, IDC_TES_FACE_POLY));
			mod->pblock->SetValue (PB_FACE_TYPE, t, 0);
//			iTri->SetCheck (1);
//			iPoly->SetCheck (0);
//			ReleaseICustButton (iTri);
//			ReleaseICustButton (iPoly);
			theHold.Accept (GetString (IDS_SA_TRI_POLY_RESTORE));
			break;
		case IDC_TES_FACE_POLY:
			theHold.Begin ();
			theHold.Put (new TriPolyRestore (mod));
// buttons handled by NotifyRefChanged - LAM - 5/7/01
//			iTri = GetICustButton (GetDlgItem (hWnd, IDC_TES_FACE_TRI));
//			iPoly = GetICustButton (GetDlgItem (hWnd, IDC_TES_FACE_POLY));
			mod->pblock->SetValue (PB_FACE_TYPE, t, 1);
//			iTri->SetCheck (0);
//			iPoly->SetCheck (1);
//			ReleaseICustButton (iTri);
//			ReleaseICustButton (iPoly);
			theHold.Accept (GetString (IDS_SA_TRI_POLY_RESTORE));
			break;

		case IDC_TESS_RECALC:
			mod->EnumModContexts (&theTessForceEnumProc);
			mod->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
			// Force a re-evaluation:
			/*int dummy;
			mod->pblock->GetValue (PB_UPDATE, t, dummy, FOREVER);
			mod->pblock->SetValue (PB_UPDATE, t, dummy);*/
			break;
		}
		break;
	}
	return FALSE;
}

static TessModDlgProc theTessModDlgProc;

//--- tessellate mod methods -------------------------------

TessMod::TessMod()
{
	pblock = NULL;
	MakeRefByID (FOREVER, 0,
		CreateParameterBlock(descVer2, PBLOCK_LENGTH, CURRENT_VERSION));
	pblock->SetValue (PB_TENSION, 0, DEF_TENSION);
	pblock->SetValue (PB_TYPE, 0, LastType);
	pblock->SetValue (PB_ITERATIONS, 0, DEF_ITERS);		// "1" iteration
	pblock->SetValue (PB_FACE_TYPE, 0, LastFaceType);
	theFaceImageHandler.LoadImages();
}

void TessMod::BeginEditParams (IObjParam  *ip, ULONG flags,Animatable *prev) {
	this->ip = ip;
	editMod  = this;

	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	SetAFlag(A_MOD_BEING_EDITED);

	pmapParam = CreateCPParamMap (descParam,PARAMDESC_LENGTH,
		pblock, ip, hInstance, MAKEINTRESOURCE(IDD_TESSPARAM),
		GetString(IDS_RB_PARAMETERS), 0);
	theTessModDlgProc.mod = this;
	pmapParam->SetUserDlgProc (&theTessModDlgProc);

	int ft;
	pblock->GetValue (PB_FACE_TYPE, TimeValue(0), ft, FOREVER);
	HWND hParam = pmapParam->GetHWnd();
	ICustButton *iFaceTri = GetICustButton (GetDlgItem (hParam, IDC_TES_FACE_TRI));
	iFaceTri->SetImage(theFaceImageHandler.hFaceImages, 0, 0, 0, 0, 16, 15);
	iFaceTri->SetType (CBT_CHECK);
	iFaceTri->SetCheck (!ft);
	iFaceTri->SetTooltip (TRUE, GetString (IDS_SA_TRIANGLE));
	ReleaseICustButton (iFaceTri);

	ICustButton *iFacePoly = GetICustButton (GetDlgItem (hParam, IDC_TES_FACE_POLY));
	iFacePoly->SetImage(theFaceImageHandler.hFaceImages, 1, 1, 1, 1, 16, 15);
	iFacePoly->SetType (CBT_CHECK);
	iFacePoly->SetCheck (ft);
	iFacePoly->SetTooltip (TRUE, GetString (IDS_SA_POLYGON));
	ReleaseICustButton (iFacePoly);

// mjm - begin - 2.15.99
// copied from TessMod::NotifyRefChanged()
	int type;
	pblock->GetValue (PB_TYPE, TimeValue(0), type, FOREVER);
	ISpinnerControl *spin = GetISpinner (GetDlgItem (hParam, IDC_TENSIONSPIN));
	spin->Enable (!type);
	ReleaseISpinner (spin);
// mjm - end
}

void TessMod::EndEditParams (IObjParam *ip,ULONG flags,Animatable *next) {
	this->ip = NULL;
	editMod  = NULL;

	TimeValue t = ip->GetTime();

	// NOTE: This flag must be cleared before sending the REFMSG_END_EDIT
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);	

	DestroyCPParamMap(pmapParam);
	theTessModDlgProc.mod = NULL;
	pblock->GetValue (PB_FACE_TYPE, t, LastFaceType, FOREVER);
	pblock->GetValue (PB_TYPE, t, LastType, FOREVER);
}

RefTargetHandle TessMod::Clone(RemapDir& remap) {
	TessMod *mod = new TessMod();
	mod->ReplaceReference(0,pblock->Clone(remap));
	BaseClone(this, mod, remap);
	return mod;
}

IOResult TessMod::Load (ILoad *iload) {
	Modifier::Load(iload);
	// Add following when there's a new pblock version.
	iload->RegisterPostLoadCallback(
		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,0));
	return IO_OK;
}

void TessMod::ModifyObject (TimeValue t, ModContext &mc, ObjectState *os, INode *node) {
	if (!os->obj->CanConvertToType (triObjectClassID)) return;
	Object *input = os->obj;

	int faceType;
	pblock->GetValue (PB_FACE_TYPE, t, faceType, FOREVER);

	// Depending on face type, we either want the input to be a poly object or
	// a tri object.  Since this means we would sometimes replace the input
	// object, sometimes not, we explicitly do replace the input for stability.
	TriObject *tobj=NULL;
	PolyObject *pobj=NULL;
	if (faceType) {
		if (os->obj->IsSubClassOf (polyObjectClassID)) pobj = (PolyObject *) os->obj;
		else {
			if (os->obj->CanConvertToType (polyObjectClassID))
				pobj = (PolyObject *) os->obj->ConvertToType (t, polyObjectClassID);
			else {
				tobj = (TriObject *) os->obj->ConvertToType (t, triObjectClassID);
				pobj = (PolyObject *) tobj->ConvertToType (t, polyObjectClassID);
				if (tobj != os->obj) tobj->DeleteThis ();
			}
		}
		if (pobj != os->obj) {
			os->obj = pobj;
			os->obj->UnlockObject ();
		}
	} else {
		if (os->obj->IsSubClassOf (triObjectClassID)) tobj = (TriObject*)os->obj;
		else tobj = (TriObject *) os->obj->ConvertToType (t, triObjectClassID);
		if (tobj != os->obj) {
			os->obj = tobj;
			os->obj->UnlockObject ();
		}
	}

	if (!mc.localData) mc.localData = (LocalModData *) new TessModData;
	TessModData *tmd = (TessModData *) mc.localData;

	if (tmd->ivalid.InInterval (t)) {
		if (faceType) {
			pobj->mm = tmd->mm;
			pobj->mm.PrepForPipeline ();
		} else {
			tobj->GetMesh() = tmd->m;
		}
		os->obj->SetChannelValidity(TOPO_CHAN_NUM, tmd->ivalid);
		os->obj->SetChannelValidity(GEOM_CHAN_NUM, tmd->ivalid);
		os->obj->SetChannelValidity(TEXMAP_CHAN_NUM, tmd->ivalid);
		os->obj->SetChannelValidity(SELECT_CHAN_NUM, tmd->ivalid);
		os->obj->SetChannelValidity (VERT_COLOR_CHAN_NUM, tmd->ivalid);
		return;
	}

	int update;
	pblock->GetValue (PB_UPDATE, t, update, FOREVER);

	bool updateMe = TRUE;
	switch (update) {
	case 1:		// IDC_UPDATE_RENDER
		if (!GetFlag (TESS_INRENDER)) updateMe = FALSE;
		break;

	case 2:		// IDC_UPDATE_MANUAL
		updateMe = FALSE;
		break;
	}
	if (tmd->force) {
		tmd->force = FALSE;
		updateMe = TRUE;
	}

	if (!updateMe) {
		// The object should be considered valid
		tmd->ivalid.SetInstant(t);
		if (faceType) {
			if (tmd->mm.numv)	// Don't overwrite with an empty mesh.
				pobj->mm = tmd->mm;
			pobj->mm.PrepForPipeline ();
		} else {
			if (tmd->m.numVerts)	// Don't overwrite with an empty mesh.
				tobj->GetMesh() = tmd->m;
		}
		os->obj->SetChannelValidity(TOPO_CHAN_NUM, tmd->ivalid);
		os->obj->SetChannelValidity(GEOM_CHAN_NUM, tmd->ivalid);
		os->obj->SetChannelValidity(TEXMAP_CHAN_NUM, tmd->ivalid);
		os->obj->SetChannelValidity(SELECT_CHAN_NUM, tmd->ivalid);
		os->obj->SetChannelValidity (VERT_COLOR_CHAN_NUM, tmd->ivalid);
		return;
	}

	int type, iterations;
	float tens;
	BOOL ignoreSel = FALSE;
	tmd->ivalid = input->ChannelValidity (t, GEOM_CHAN_NUM);
	tmd->ivalid &= input->ChannelValidity (t, TOPO_CHAN_NUM);
	tmd->ivalid &= input->ChannelValidity (t, SELECT_CHAN_NUM);
	tmd->ivalid &= input->ChannelValidity (t, SUBSEL_TYPE_CHAN_NUM);
	tmd->ivalid &= input->ChannelValidity (t, TEXMAP_CHAN_NUM);
	tmd->ivalid &= input->ChannelValidity (t, VERT_COLOR_CHAN_NUM);

	pblock->GetValue(PB_TYPE,t,type,tmd->ivalid);
	pblock->GetValue(PB_ITERATIONS,t,iterations,tmd->ivalid);
	iterations++;

	int order, i;
	HCURSOR hCur; 
	if (faceType == 0) {
		BitArray fsel;
		switch (tobj->GetMesh().selLevel) {
		case MESH_FACE:
			fsel = tobj->GetMesh().faceSel;
			break;
		case MESH_VERTEX:
			fsel.SetSize (tobj->GetMesh().numFaces);
			for (i=0; i<tobj->GetMesh().numFaces; i++) {
				for (int j=0; j<3; j++) {
					if (tobj->GetMesh().vertSel[tobj->GetMesh().faces[i].v[j]]) break;
				}
				if (j<3) fsel.Set(i);
			}
			break;
		case MESH_EDGE:
			fsel.SetSize (tobj->GetMesh().numFaces);
			for (i=0; i<tobj->GetMesh().numFaces; i++) {
				for (int j=0; j<3; j++) {
					if (tobj->GetMesh().edgeSel[i*3+j]) break;
				}
				if (j<3) fsel.Set(i);
			}
			break;
		case MESH_OBJECT:
			fsel.SetSize (tobj->GetMesh().numFaces);
			fsel.SetAll ();
			break;
		}
		order = fsel.NumberSet ();
		for (i=0; i<iterations; i++) order *= (type ? 3 : 4);
		SuperInit (order);
		if (testEscape)
			hCur = SetCursor (LoadCursor (NULL, IDC_WAIT));

		if (type) {
			for (i=0; i<iterations; i++) {
				MeshDelta tmd;
				tmd.DivideFaces (tobj->GetMesh(), fsel, this);
				tmd.Apply (tobj->GetMesh());
				if (GetFlag (TESS_ABORT)) break;
				if (i<iterations-1) {
					int beforeNumFaces = fsel.GetSize();
					fsel.SetSize (tobj->GetMesh().numFaces);
					for (int j=0; j<tmd.fCreate.Count(); j++) {
						if (tmd.fCreate[j].original == UNDEFINED) continue;
						if (fsel[tmd.fCreate[j].original]) fsel.Set (j+beforeNumFaces);
					}
				}
			}
		} else {
			pblock->GetValue(PB_TENSION, t, tens, tmd->ivalid);
			for (i=0; i<iterations; i++) {
				MeshDelta tmd;
				tmd.EdgeTessellate (tobj->GetMesh(), fsel, tens/400.0f, NULL, NULL, this);
				tmd.Apply (tobj->GetMesh());
				if (GetFlag (TESS_ABORT)) break;
				if (i<iterations-1) {
					int beforeNumFaces = fsel.GetSize();
					fsel.SetSize (tobj->GetMesh().numFaces);
					for (int j=0; j<tmd.fCreate.Count(); j++) {
						if (tmd.fCreate[j].original == UNDEFINED) continue;
						if (fsel[tmd.fCreate[j].original]) fsel.Set (j+beforeNumFaces);
					}
				}
			}
		}
		tobj->GetMesh().InvalidateGeomCache ();
		tobj->GetMesh().InvalidateTopologyCache ();
		tmd->m = tobj->GetMesh();
	} else {
		tmd->mm = pobj->GetMesh ();

		// Luna task 747
		// We cannot support specified normals in Tessellate
		tmd->mm.ClearSpecifiedNormals ();

		if (!tmd->mm.GetFlag (MN_MESH_FILLED_IN)) tmd->mm.FillInMesh ();
		tmd->mm.MakeConvex ();
		order = tmd->mm.TargetFacesBySelection (pobj->mm.selLevel);
		//NH 02/14/03 removed additional declaration to solve warnings
		for (i=0; i<iterations; i++) order *= (type ? 3 : 4);
		SuperInit (order);
		if (type) {
			for (i=0; i<iterations; i++) {
				tmd->mm.TessellateByCenters (this);
				tmd->mm.CollapseDeadFaces ();
				if (GetFlag (TESS_ABORT)) break;
			}
		} else {
			pblock->GetValue(PB_TENSION,t,tens,tmd->ivalid);
			tens /= 400.0f;
			for (i=0; i<iterations; i++) {
				tmd->mm.TessellateByEdges (tens, this);
				tmd->mm.CollapseDeadFaces ();
				if (GetFlag (TESS_ABORT)) break;
			}
		}
		pobj->mm = tmd->mm;
		pobj->mm.PrepForPipeline ();
	}
	if (GetFlag (TESS_ABORT)) {
		if (!GetFlag(TESS_INRENDER)) pblock->SetValue (PB_UPDATE, t, 2);
		tmd->ivalid.SetInstant (t);
		ClearFlag (TESS_ABORT);
	}
	if (testEscape)
		SetCursor (hCur);
	os->obj->UnlockObject ();
	os->obj->UpdateValidity (GEOM_CHAN_NUM, tmd->ivalid);
	os->obj->UpdateValidity (TOPO_CHAN_NUM, tmd->ivalid);
	os->obj->UpdateValidity (SELECT_CHAN_NUM, tmd->ivalid);
	os->obj->UpdateValidity (SUBSEL_TYPE_CHAN_NUM, tmd->ivalid);
	os->obj->UpdateValidity (TEXMAP_CHAN_NUM, tmd->ivalid);
	os->obj->UpdateValidity (VERT_COLOR_CHAN_NUM, tmd->ivalid);
}

Interval TessMod::LocalValidity(TimeValue t) {
  // aszabo|feb.05.02 If we are being edited, 
	// return NEVER to forces a cache to be built after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;
		
	Interval iv = FOREVER;
	int type, iterations, faceType;
	float tens;
	pblock->GetValue(PB_TYPE,t,type,iv);	
	pblock->GetValue(PB_ITERATIONS,t,iterations,iv);
	pblock->GetValue (PB_FACE_TYPE, t, faceType, iv);
	if (!type) pblock->GetValue(PB_TENSION,t,tens,iv);
	return iv;
}

RefResult TessMod::NotifyRefChanged (Interval changeInt,RefTargetHandle hTarget,
									 PartID& partID, RefMessage message) {
	int type;
	switch (message) {
		case REFMSG_CHANGE:
			if (editMod==this && pmapParam) {
				pblock->GetValue (PB_TYPE, TimeValue(0), type, FOREVER);
				ISpinnerControl *spin;
				spin = GetISpinner (GetDlgItem (pmapParam->GetHWnd(), IDC_TENSIONSPIN));
				spin->Enable (!type);
				ReleaseISpinner (spin);
				if (pblock->LastNotifyParamNum() == PB_FACE_TYPE) {  // LAM - 5/7/01 - defect 168091
					pblock->GetValue (PB_FACE_TYPE, TimeValue(0), type, FOREVER);
					ICustButton *iTri, *iPoly;
					iTri = GetICustButton (GetDlgItem (pmapParam->GetHWnd(), IDC_TES_FACE_TRI));
					iPoly = GetICustButton (GetDlgItem (pmapParam->GetHWnd(), IDC_TES_FACE_POLY));
					iTri->SetCheck (!type);
					iPoly->SetCheck (type);
					ReleaseICustButton (iTri);
					ReleaseICustButton (iPoly);
				}
				pmapParam->Invalidate();
			}
			EnumModContexts (&theTessInvalidateEnumProc);
			break;

		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			switch (gpd->index) {
				case PB_TENSION: gpd->dim = stdPercentDim; break;				
			}
			return REF_STOP;
		}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;			
			switch (gpn->index) {
				case PB_TENSION: gpn->name = GetString(IDS_TESS_TENSION); break;				
			}
			return REF_STOP; 
		}
	}
	return REF_SUCCEED;
}

void TessMod::NotifyInputChanged (Interval changeInt, PartID part,
									  RefMessage msg, ModContext *mc) {
	if (!mc->localData) return;
	TessModData *tmd = (TessModData *) mc->localData;
	tmd->ivalid = NEVER;
}

int TessMod::RenderBegin(TimeValue t, ULONG flags) {
	SetFlag(TESS_INRENDER);
	if (!pblock) return 0;
	int update;
	pblock->GetValue (PB_UPDATE, t, update, FOREVER);
	if (update == 1 /*IDC_UPDATE_RENDER*/) {
		EnumModContexts (&theTessInvalidateEnumProc);
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}
	return 0;
}

int TessMod::RenderEnd(TimeValue t) {
	ClearFlag(TESS_INRENDER);
	int update;
	pblock->GetValue (PB_UPDATE, t, update, FOREVER);
	if (update == 1 /*IDC_UPDATE_RENDER*/) {
		EnumModContexts (&theTessInvalidateEnumProc);
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}
	return 0;
}

static EscapeChecker myEscapeChecker;

void TessMod::SuperInit (int total) {
	ClearFlag (TESS_ABORT);
	if (total<200) { testEscape=FALSE; return; }
	testEscape = TRUE;
	myEscapeChecker.Setup();
}

BOOL TessMod::Progress(int p) {
	if (!testEscape) return TRUE;
	if (myEscapeChecker.Check()) {
		SetFlag(TESS_ABORT);
		return FALSE;
	}
	else return TRUE;
}


#endif // NO_MODIFIER_TESSELATE 