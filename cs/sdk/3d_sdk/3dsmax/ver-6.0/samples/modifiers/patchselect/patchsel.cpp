/**********************************************************************
 *<
	FILE: PatchSel.cpp

	DESCRIPTION:  A selection modifier for patches

	CREATED BY: Steve Anderson

	HISTORY: 11/17/1999

 *>	Copyright (c) 1999, All Rights Reserved.
 **********************************************************************/

#include "Max.h"
#include "resource.h"
#include "namesel.h"
#include "nsclip.h"
#include "MeshDLib.h"
#include "MaxIcon.h"

TCHAR *GetString(int id);
ClassDesc *GetPatchSelModDesc();
HINSTANCE hInstance;
int controlsInit = FALSE;
INT_PTR CALLBACK SoftSelectDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );

static GenSubObjType SOT_Handle(39);
static GenSubObjType SOT_Vertex(6);
static GenSubObjType SOT_Edge(7);
static GenSubObjType SOT_Patch(8);
static GenSubObjType SOT_Element(5);

/** public functions **/
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
	hInstance = hinstDLL;

	if ( !controlsInit ) {
		controlsInit = TRUE;
		
		// jaguar controls
		InitCustomControls(hInstance);

#ifdef OLD3DCONTROLS
		// initialize 3D controls
		Ctl3dRegister(hinstDLL);
		Ctl3dAutoSubclass(hinstDLL);
#endif
		
		// initialize Chicago controls
		InitCommonControls();
	}

	switch(fdwReason) {
	case DLL_PROCESS_ATTACH: break;
	case DLL_THREAD_ATTACH: break;
	case DLL_THREAD_DETACH: break;
	case DLL_PROCESS_DETACH: break;
	}
	return(TRUE);
}


//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *LibDescription() {
	return GetString(IDS_PS_DEFMODS);
}

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses() {return 1;}

__declspec( dllexport ) ClassDesc* LibClassDesc(int i) {
	switch(i) {
	case 0: return GetPatchSelModDesc();
	default: return 0;
	}
}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG LibVersion() { return VERSION_3DSMAX; }

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer() { return 1; }

INT_PTR CALLBACK DefaultSOTProc (HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	IObjParam *ip = (IObjParam*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
		break;

	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MOUSEMOVE:
		if (ip) ip->RollupMouseMessage(hWnd,msg,wParam,lParam);
		return FALSE;

	default:
		return FALSE;
	}
	return TRUE;
}

TCHAR *GetString(int id) {
	static TCHAR buf[256];
	if (hInstance) return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

#define PATCHSELECT_CLASS_ID Class_ID(0x19592732, 0x3407a7a)

// These are values for selLevel.
#define EP_OBJECT	0
#define EP_VERTEX	1
#define EP_EDGE		2
#define EP_PATCH	3
#define EP_ELEMENT	4
#define EP_HANDLE	5
#define EP_LEVELS	6

// Named selection set levels:
#define NS_VERTEX 0
#define NS_EDGE 1
#define NS_PATCH 2	// Patch and Element
#define NS_HANDLE 3
#define NS_LEVELS 4		// number of selection levels
static int namedSetLevel[] = { NS_VERTEX, NS_VERTEX, NS_EDGE, NS_PATCH, NS_PATCH, NS_HANDLE };
static int namedClipLevel[] = { CLIP_P_VERT, CLIP_P_VERT, CLIP_P_EDGE, CLIP_P_PATCH, CLIP_P_PATCH, CLIP_P_HANDLE };

#define WM_UPDATE_CACHE		(WM_USER+0x288)

class PatchSelMod : public Modifier, public IPatchSelect {	
public:
	DWORD selLevel;
	Tab<TSTR*> namedSel[NS_LEVELS];
	Tab<DWORD> ids[NS_LEVELS];

	static IObjParam *ip;
	static PatchSelMod *editMod;
	static BOOL rsSoftSel;	// rollup states (FALSE = rolled up)
	static BOOL selByVert;
	static BOOL ignoreBackfaces;
	static HWND hParams, hSoftSelPanel;
	static SelectModBoxCMode *selectMode;
	static BOOL updateCachePosted;

	PatchSelMod();

	// From Animatable
	void DeleteThis() { delete this; }
	void GetClassName(TSTR& s) {s = GetString(IDS_PS_PATCHSELMOD);}  
	virtual Class_ID ClassID() { return PATCHSELECT_CLASS_ID;}		
	RefTargetHandle Clone(RemapDir& remap = NoRemap());
	TCHAR *GetObjectName() {return GetString(IDS_PS_PATCHSELMOD);}
	void *GetInterface(ULONG id) { if (id==I_PATCHSELECT) return (IPatchSelect*)this; else return Modifier::GetInterface(id); }

	// From modifier
	ChannelMask ChannelsUsed()  {return PART_GEOM|PART_TOPO;}
	ChannelMask ChannelsChanged() {return PART_SELECT|PART_SUBSEL_TYPE|PART_TOPO;} // RB 5/27/97: Had to include topo channel because in edge select mode this modifier turns on hidden edges -- which may cause the edge list to be rebuilt, which is effectively a change in topology since the edge list is now part of the topo channel.
	Class_ID InputType() {return patchObjectClassID;}
	void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
	Interval LocalValidity(TimeValue t) { return GetValidity(t); }
	Interval GetValidity (TimeValue t);
	BOOL DependOnTopology(ModContext &mc) {return TRUE;}

	// From BaseObject
	CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 
	void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
	void EndEditParams(IObjParam *ip,ULONG flags,Animatable *next);		
	int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
	int Display(TimeValue t, INode* inode, ViewExp *vpt, int flagst, ModContext *mc);
	void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);
	void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
	void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);

	void ActivateSubobjSel(int level, XFormModes& modes);
	void SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert=FALSE);
	void ClearSelection(int selLevel);
	void SelectAll(int selLevel);
	void InvertSelection(int selLevel);
	void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc);

	// NS: New SubObjType API
	int NumSubObjTypes();
	ISubObjType *GetSubObjType(int i);

	void ShowEndResultChanged (BOOL showEndResult) { NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE); }

	BOOL SupportsNamedSubSels() {return TRUE;}
	void ActivateSubSelSet(TSTR &setName);
	void NewSetFromCurSel(TSTR &setName);
	void RemoveSubSelSet(TSTR &setName);
	void SetupNamedSelDropDown();
	int NumNamedSelSets();
	TSTR GetNamedSelSetName(int i);
	void SetNamedSelSetName(int i,TSTR &newName);
	void NewSetByOperator(TSTR &newName,Tab<int> &sets,int op);
	void UpdateNamedSelDropDown ();

	// CAL-03/11/03: implement IPatchSelect interface
	// From IPatchSelect
	DWORD GetSelLevel();
	void SetSelLevel(DWORD level);
	void LocalDataChanged();
	
	// IO
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);
	IOResult LoadNamedSelChunk(ILoad *iload,int level);
	IOResult SaveLocalData(ISave *isave, LocalModData *ld);
	IOResult LoadLocalData(ILoad *iload, LocalModData **pld);

	int NumRefs() { return 0; }
	RefTargetHandle GetReference(int i) { return NULL; }
	void SetReference(int i, RefTargetHandle rtarg) { }

	int NumSubs() {return 0;}
	Animatable* SubAnim(int i) { return NULL; }
	TSTR SubAnimName(int i) {return _T("");}

	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
	   PartID& partID, RefMessage message);

	void UpdateSelLevelDisplay ();
	void SelectFrom(int from);
	void SelectOpenEdges ();
	void SetEnableStates();
	void SelectByMatID(int id);
	void SetNumSelLabel();		
	void UpdateCache(TimeValue t);

	// Local methods for handling named selection sets
	int FindSet(TSTR &setName,int level);		
	DWORD AddSet(TSTR &setName,int level);
	void RemoveSet(TSTR &setName,int level);
	void UpdateSetNames ();	// Reconciles names with PatchSelData.
	void ClearSetNames();
	void NSCopy();
	void NSPaste();
	BOOL GetUniqueSetName(TSTR &name);
	int SelectNamedSet();
	
	ParamDimension *GetParameterDim (int paramID);
	TSTR GetParameterName (int paramID);

	// soft selection support
	//
 	void SetSoftSelDlgEnables( HWND hSoftSel = NULL );
    float mFalloff, mPinch, mBubble;
	int   mEdgeDist, mUseEdgeDists, mAffectBackface, mUseSoftSelections;

	int  UseEdgeDists();
	void SetUseEdgeDists( int useSoftSelections );
	int  UseSoftSelections();
	void SetUseSoftSelections( int useSoftSelections );
	void InvalidateVertexWeights();

	void ApplySoftSelectionToPatch( PatchMesh * pPatch );
	void UpdateVertexDists();
	void UpdateEdgeDists( );
	void UpdateVertexWeights();

	Point3 VertexNormal( PatchMesh * pPatch, int vIndex );
};

HWND PatchSelMod::hSoftSelPanel   = NULL;
BOOL PatchSelMod::rsSoftSel       = FALSE;

class PatchSelData : public LocalModData, public IPatchSelectData {
public:
	// LocalModData
	void* GetInterface(ULONG id) { if (id == I_PATCHSELECTDATA) return (IPatchSelectData*)this; else return LocalModData::GetInterface(id); }

	// Selection sets
	BitArray vecSel;	// CAL-06/10/03: (FID #1914)
	BitArray vertSel;
	BitArray patchSel;
	BitArray edgeSel;

	// Lists of named selection sets
	GenericNamedSelSetList hselSet;		// CAL-06/10/03: (FID #1914)
	GenericNamedSelSetList vselSet;
	GenericNamedSelSetList pselSet;
	GenericNamedSelSetList eselSet;

	BOOL held;
	PatchMesh *pmesh;

	PatchSelData (PatchMesh &pmesh);
	PatchSelData() { held=0; pmesh=NULL; }
	~PatchSelData() { FreeCache(); }
	LocalModData *Clone();
	PatchMesh *GetMesh() {return pmesh;}
	void SetCache(PatchMesh &pmesh);
	void FreeCache();
	void SynchBitArrays();

	BitArray SelVertByPatch();
	BitArray SelVertByEdge();
	BitArray SelPatchByVert();
	BitArray SelPatchByEdge();
	BitArray SelEdgeByVert();
	BitArray SelEdgeByPatch();
	BitArray SelElementByVert();
	BitArray SelElementByPatch();
	BitArray SelElementByEdge();
	BitArray SelHandleByVert();		// CAL-06/10/03: (FID #1914)
	BitArray SelHandleByPatch();	// CAL-06/10/03: (FID #1914)
	BitArray SelHandleByEdge();		// CAL-06/10/03: (FID #1914)

	BitArray TempVSel (PatchMesh & pm, DWORD slevel);
	BitArray GetSel (int nsl) { if (nsl==NS_HANDLE) return vecSel; else if (nsl==NS_VERTEX) return vertSel; else if (nsl==NS_EDGE) return edgeSel; else return patchSel; }

	// CAL-03/11/03: implement IPatchSelectData interface
	// From IPatchSelectData:
	BitArray GetVecSel() { return vecSel; }		// CAL-06/10/03: (FID #1914)
	BitArray GetVertSel() { return vertSel; }
	BitArray GetEdgeSel() { return edgeSel; }
	BitArray GetPatchSel() { return patchSel; }

	void SetVecSel(BitArray &set, IPatchSelect *imod, TimeValue t);		// CAL-06/10/03: (FID #1914)
	void SetVertSel(BitArray &set, IPatchSelect *imod, TimeValue t);
	void SetEdgeSel(BitArray &set, IPatchSelect *imod, TimeValue t);
	void SetPatchSel(BitArray &set, IPatchSelect *imod, TimeValue t);

	GenericNamedSelSetList & GetNamedVecSelList () { return hselSet; }	// CAL-06/10/03: (FID #1914)
	GenericNamedSelSetList & GetNamedVertSelList () { return vselSet; }
	GenericNamedSelSetList & GetNamedEdgeSelList () { return eselSet; }
	GenericNamedSelSetList & GetNamedPatchSelList () { return pselSet; }
	
	GenericNamedSelSetList & GetNamedSel (int nsl) {
		if (nsl==NS_HANDLE) return hselSet;
		if (nsl==NS_VERTEX) return vselSet;
		if (nsl==NS_EDGE) return eselSet;
		return pselSet;		// Patches and Elements
	}
};

class PatchSelectRestore : public RestoreObj {
public:
	BitArray usel, rsel;
	BitArray *sel;
	PatchSelMod *mod;
	PatchSelData *d;
	int level;

	PatchSelectRestore(PatchSelMod *m, PatchSelData *d);
	PatchSelectRestore(PatchSelMod *m, PatchSelData *d, int level);
	void Restore(int isUndo);
	void Redo();
	int Size() { return 1; }
	void EndHold() {d->held=FALSE;}
	TSTR Description() { return TSTR(_T("SelectRestore")); }
};

class AppendSetRestore : public RestoreObj {
public:
	BitArray set;		
	DWORD id;
	GenericNamedSelSetList *setList;
	TSTR name;

	AppendSetRestore(GenericNamedSelSetList *sl, DWORD i, TSTR & n) {setList = sl; id = i; name = n; }
	void Restore(int isUndo) {
		set  = *setList->GetSet (id);		
		setList->RemoveSet(id);			
	}
	void Redo() { setList->InsertSet (set, id, name); }

	TSTR Description() {return TSTR(_T("Append Set"));}
};

class AppendSetNameRestore : public RestoreObj {
	public:		
		TSTR name;
		DWORD id;
		PatchSelMod *et;
		Tab<TSTR*> *sets;
		Tab<DWORD> *ids;

		AppendSetNameRestore(PatchSelMod *e,Tab<TSTR*> *s,Tab<DWORD> *i) 
			{et = e; sets = s; ids = i;}
		void Restore(int isUndo) {			
			name = *(*sets)[sets->Count()-1];
			id   = (*ids)[sets->Count()-1];
			delete (*sets)[sets->Count()-1];
			sets->Delete(sets->Count()-1,1);			
			if (et->ip) et->ip->NamedSelSetListChanged();
			}
		void Redo() {
			TSTR *nm = new TSTR(name);
			sets->Append(1,&nm);
			ids->Append(1,&id);
			if (et->ip) et->ip->NamedSelSetListChanged();
			}
				
		TSTR Description() {return TSTR(_T("Append Set Name"));}
	};

class DeleteSetRestore : public RestoreObj {
public:
	BitArray set;
	DWORD id;
	TSTR name;
	//int index;
	GenericNamedSelSetList *setList;		

	DeleteSetRestore(GenericNamedSelSetList *sl,DWORD i, TSTR & n) {
		setList = sl; 
		id = i;
		BitArray *ptr = setList->GetSet(id);
		if (ptr) set = *ptr;
		name = n;
	}   		
	void Restore(int isUndo) {
		setList->InsertSet(set,id,name);
	}
	void Redo() {
		setList->RemoveSet(id);
	}
			
	TSTR Description() {return TSTR(_T("Delete Set"));}
};

class DeleteSetNameRestore : public RestoreObj {
public:		
	TSTR name;
	//int index;		
	DWORD id;
	PatchSelMod *et;
	Tab<TSTR*> *sets;
	Tab<DWORD> *ids;

	DeleteSetNameRestore(Tab<TSTR*> *s,PatchSelMod *e,Tab<DWORD> *i, DWORD id) {
		sets = s; et = e; //index = i;			
		this->id = id;
		ids = i;
		int index = -1;
		for (int j=0; j<sets->Count(); j++) {
			if ((*ids)[j]==id) {
				index = j;
				break;
				}
			}
		if (index>=0) {
			name = *(*sets)[index];
			}
		}   		
	void Restore(int isUndo) {			
		TSTR *nm = new TSTR(name);			
		//sets->Insert(index,1,&nm);
		sets->Append(1,&nm);
		ids->Append(1,&id);
		if (et->ip) et->ip->NamedSelSetListChanged();
		}
	void Redo() {
		int index = -1;
		for (int j=0; j<sets->Count(); j++) {
			if ((*ids)[j]==id) {
				index = j;
				break;
				}
			}
		if (index>=0) {
			sets->Delete(index,1);
			ids->Delete(index,1);
			}
		//sets->Delete(index,1);
		if (et->ip) et->ip->NamedSelSetListChanged();
		}
			
	TSTR Description() {return TSTR(_T("Delete Set Name"));}
};

class SetNameRestore : public RestoreObj {
public:
	TSTR undo, redo;
	//int index;
	DWORD id;
	Tab<TSTR*> *sets;
	Tab<DWORD> *ids;
	PatchSelMod *et;
	SetNameRestore(Tab<TSTR*> *s,PatchSelMod *e,Tab<DWORD> *i,DWORD id) {
		//index = i; 
		this->id = id;
		ids = i;
		sets = s; et = e;
		int index = -1;
		for (int j=0; j<sets->Count(); j++) {
			if ((*ids)[j]==id) {
				index = j;
				break;
				}
			}
		if (index>=0) {
			undo = *(*sets)[index];
			}			
		}

	void Restore(int isUndo) {
		int index = -1;
		for (int j=0; j<sets->Count(); j++) {
			if ((*ids)[j]==id) {
				index = j;
				break;
				}
			}
		if (index>=0) {
			redo = *(*sets)[index];
			*(*sets)[index] = undo;
			}			
		if (et->ip) et->ip->NamedSelSetListChanged();
		}
	void Redo() {
		int index = -1;
		for (int j=0; j<sets->Count(); j++) {
			if ((*ids)[j]==id) {
				index = j;
				break;
			}
		}
		if (index>=0) {
			*(*sets)[index] = redo;
		}
		if (et->ip) et->ip->NamedSelSetListChanged();
	}
			
	TSTR Description() {return TSTR(_T("Set Name"));}
};


//--- ClassDescriptor and class vars ---------------------------------

#define SEL_OBJECT	0
#define SEL_VERTEX	1
#define SEL_EDGE	2
#define SEL_PATCH	3
#define SEL_ELEMENT	4
#define	SEL_HANDLE	5

IObjParam *PatchSelMod::ip              = NULL;
PatchSelMod *PatchSelMod::editMod         = NULL;
HWND PatchSelMod::hParams         = NULL;
BOOL PatchSelMod::selByVert       = FALSE;
BOOL PatchSelMod::ignoreBackfaces = FALSE;
SelectModBoxCMode *PatchSelMod::selectMode      = NULL;
BOOL PatchSelMod::updateCachePosted = FALSE;

static int lastMatID = 1;

class PatchSelClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new PatchSelMod; }
	const TCHAR *	ClassName() { return GetString(IDS_PS_PATCHSELMOD); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() {
		return PATCHSELECT_CLASS_ID;
	}
	const TCHAR* 	Category() { return GetString(IDS_PS_CATEGORY);}
};

static PatchSelClassDesc patchSelDesc;
ClassDesc* GetPatchSelModDesc() {return &patchSelDesc;}

static INT_PTR CALLBACK PatchSelectProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Table to convert selLevel values to patch selLevel flags.
const int patchLevel[] = {PATCH_OBJECT,PATCH_VERTEX,PATCH_EDGE,PATCH_PATCH,PATCH_PATCH,PATCH_HANDLE};

// Get display flags based on selLevel.
const DWORD patchLevelDispFlags[] = {0,DISP_VERTTICKS|DISP_SELVERTS,DISP_SELEDGES,DISP_SELPATCHES,DISP_SELPATCHES,DISP_VERTTICKS|DISP_SELHANDLES};

// For hit testing...
static int patchHitLevel[] = {0,SUBHIT_PATCH_VERTS | SUBHIT_PATCH_VECS,SUBHIT_PATCH_EDGES,SUBHIT_PATCH_PATCHES,SUBHIT_PATCH_PATCHES,SUBHIT_PATCH_VECS};

//--- MeshSel mod methods -------------------------------

PatchSelMod::PatchSelMod() {
	selLevel = SEL_OBJECT;

	mFalloff =  20.0;
	mBubble = 0.0;
	mPinch = 0.0;
	mUseSoftSelections = 0;
	mUseEdgeDists = 0;
	mEdgeDist = 1;
	mAffectBackface = 1;
}

RefTargetHandle PatchSelMod::Clone(RemapDir& remap) {
	PatchSelMod *mod = new PatchSelMod();
	mod->selLevel = selLevel;
	for (int i=0; i<NS_LEVELS; i++)
	{
		mod->namedSel[i].SetCount (namedSel[i].Count());
		for (int j=0; j<namedSel[i].Count(); j++) mod->namedSel[i][j] = new TSTR (*namedSel[i][j]);
		mod->ids[i].SetCount (ids[i].Count());
		for (j=0; j<ids[i].Count(); j++) mod->ids[i][j] = ids[i][j];
	}
	BaseClone(this, mod, remap);
	return mod;
}

Interval PatchSelMod::GetValidity (TimeValue t) {
	return FOREVER;
}

BitArray PatchSelData::TempVSel (PatchMesh & pm, DWORD selLevel) {
	BitArray vsel;
	vsel.SetSize (pm.numVerts);

	int i, j;
	switch (selLevel) {
	case SEL_OBJECT:
		vsel.SetAll ();
		break;

	// CAL-06/10/03: add handle sub-object mode. (FID #1914)
	case SEL_HANDLE:
		vsel.ClearAll ();
		for (i=0; i<pm.numVecs; i++) {
			if (!vecSel[i]) continue;
			int v = pm.vecs[i].vert;
			if (v >= 0)	vsel.Set(v);
		}
		break;

	case SEL_VERTEX:
		vsel = vertSel;
		break;

	case SEL_EDGE:
		vsel.ClearAll ();
		for (i=0; i<pm.numEdges; i++) {
			if (!edgeSel[i]) continue;
			vsel.Set (pm.edges[i].v1);
			vsel.Set (pm.edges[i].v2);
		}
		break;

	default:
		vsel.ClearAll ();
		for (i=0; i<pm.numPatches; i++) {
			if (!patchSel[i]) continue;
			if (pm.patches[i].type == PATCH_TRI) {
				for (j=0; j<3; j++) vsel.Set (pm.patches[i].v[j]);
			} else {
				for (j=0; j<4; j++) vsel.Set (pm.patches[i].v[j]);
			}
		}
		break;
	}
	return vsel;
}

void PatchSelMod::ModifyObject (TimeValue t, ModContext &mc, ObjectState *os, INode *node) {
	if (!os->obj->IsSubClassOf(patchObjectClassID)) return;
	PatchObject *pobj = (PatchObject*)os->obj;
	PatchSelData *d  = (PatchSelData*)mc.localData;
	if (!d) mc.localData = d = new PatchSelData(pobj->patch);
	if ((editMod==this) && (!d->GetMesh())) d->SetCache(pobj->patch);

	BitArray vecSel = d->vecSel;
	BitArray vertSel = d->vertSel;
	BitArray patchSel = d->patchSel;
	BitArray edgeSel = d->edgeSel;
	vecSel.SetSize(pobj->patch.getNumVecs(),TRUE);
	vertSel.SetSize(pobj->patch.getNumVerts(),TRUE);
	patchSel.SetSize(pobj->patch.getNumPatches(),TRUE);
	edgeSel.SetSize(pobj->patch.getNumEdges(),TRUE);
	pobj->patch.vecSel = vecSel;
	pobj->patch.vertSel = vertSel;
	pobj->patch.patchSel = patchSel;
	pobj->patch.edgeSel = edgeSel;

	if (d->GetMesh()) {
		// Keep the cache up to date if it exists.
		d->GetMesh()->vecSel = vecSel;
		d->GetMesh()->vertSel = vertSel;
		d->GetMesh()->patchSel = patchSel;
		d->GetMesh()->edgeSel = edgeSel;
	}

	ApplySoftSelectionToPatch( &pobj->patch );	
	SetSoftSelDlgEnables();
	pobj->patch.dispFlags = 0;
	pobj->patch.SetDispFlag (patchLevelDispFlags[selLevel]);
	pobj->patch.selLevel = patchLevel[selLevel];
	pobj->SetChannelValidity (SELECT_CHAN_NUM, FOREVER);
}

void PatchSelMod::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc) {
	if (!mc->localData) return;
	if (partID == PART_SELECT) return;
	((PatchSelData*)mc->localData)->FreeCache();
	if (ip && hParams && editMod==this && !updateCachePosted) {
		TimeValue t = ip->GetTime();
		PostMessage(hParams,WM_UPDATE_CACHE,(WPARAM)t,0);
		updateCachePosted = TRUE;
	}
}

void PatchSelMod::UpdateCache(TimeValue t) {
	NotifyDependents(Interval(t,t), PART_GEOM|SELECT_CHANNEL|PART_SUBSEL_TYPE|
		PART_DISPLAY|PART_TOPO, REFMSG_MOD_EVAL);
	updateCachePosted = FALSE;
}

static int butIDs[] = { 0, EP_VERTEX, EP_EDGE, EP_PATCH, EP_ELEMENT, EP_HANDLE };
void PatchSelMod::UpdateSelLevelDisplay () {
	ICustToolbar *iToolbar = GetICustToolbar(GetDlgItem(hParams,IDC_PS_SELTYPE));
	ICustButton *but;
	for (int i=1; i<EP_LEVELS; i++) {
		but = iToolbar->GetICustButton (butIDs[i]);
		but->SetCheck ((DWORD)i==selLevel);
		ReleaseICustButton (but);
	}
	ReleaseICustToolbar (iToolbar);
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	UpdateWindow(hParams);
	ip->RedrawViews(ip->GetTime());
}

void PatchSelMod::BeginEditParams (IObjParam  *ip, ULONG flags,Animatable *prev) {
	this->ip = ip;	
	editMod  = this;
	UpdateSetNames ();

	hParams = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_PATCH_SELECT),
		PatchSelectProc, GetString(IDS_PS_PARAMS), (LPARAM)this);
	hSoftSelPanel = ip->AddRollupPage (hInstance, MAKEINTRESOURCE(IDD_SOFT_SELECTION),
		SoftSelectDlgProc, GetString (IDS_SOFT_SELECTION), (LPARAM) this, rsSoftSel ? 0 : APPENDROLL_CLOSED);

	selectMode = new SelectModBoxCMode(this,ip);

	// Add our sub object type
	// TSTR type1(GetString(IDS_PS_VERTEX));
	// TSTR type2(GetString(IDS_PS_EDGE));
	// TSTR type3(GetString(IDS_PS_PATCH));
	// TSTR type4(GetString(IDS_PS_ELEMENT));
	// const TCHAR *ptype[] = {type1, type2, type3, type4 };
	// This call is obsolete. Please see BaseObject::NumSubObjTypes() and BaseObject::GetSubObjType()
	// ip->RegisterSubObjectTypes(ptype, 4);

	// Restore the selection level.
	ip->SetSubObjectLevel(selLevel);

	SetEnableStates ();
	UpdateSelLevelDisplay ();
	SetNumSelLabel();

	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);	
}

void PatchSelMod::EndEditParams (IObjParam *ip,ULONG flags,Animatable *next) {
	if (hParams) ip->DeleteRollupPage(hParams);
	hParams = NULL;

	if (hSoftSelPanel) {
		rsSoftSel = IsRollupPanelOpen (hSoftSelPanel);
		ip->DeleteRollupPage(hSoftSelPanel);
		hSoftSelPanel = NULL;
	}

	ip->DeleteMode(selectMode);
	if (selectMode) delete selectMode;
	selectMode = NULL;

	this->ip = NULL;
	editMod  = NULL;

	TimeValue t = ip->GetTime();
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);
}

int PatchSelMod::HitTest (TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc) {
	Interval valid;
	int savedLimits, res = 0;
	GraphicsWindow *gw = vpt->getGW();
	HitRegion hr;
	
	// Setup GW
	MakeHitRegion(hr,type, crossing,4,p);
	gw->setHitRegion(&hr);
	Matrix3 mat = inode->GetObjectTM(t);
	gw->setTransform(mat);	
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
	if (ignoreBackfaces) {
		gw->setRndLimits(gw->getRndLimits() |  GW_BACKCULL);
		flags |= SUBHIT_PATCH_IGNORE_BACKFACING;
	} else {
		gw->setRndLimits(gw->getRndLimits() & ~GW_BACKCULL);
		if (flags & SUBHIT_PATCH_IGNORE_BACKFACING)
			flags -= SUBHIT_PATCH_IGNORE_BACKFACING;
	}
	gw->clearHitCode();
	SubPatchHitList hitList;
	PatchSubHitRec *rec;

	if (!mc->localData) return 0;
	PatchMesh *patch = ((PatchSelData*)mc->localData)->GetMesh();
	if (!patch) return 0;

	if (selLevel>SEL_VERTEX && selByVert) {
		res = patch->SubObjectHitTest(gw, gw->getMaterial(), &hr,
			flags|patchHitLevel[SEL_VERTEX], hitList);
	} else {
		res = patch->SubObjectHitTest(gw, gw->getMaterial(), &hr,
			flags|patchHitLevel[selLevel], hitList);
	}

	rec = hitList.First();
	while (rec) {
		vpt->LogHit(inode,mc,rec->dist,123456,new PatchHitData(rec->patch, rec->index, rec->type));
		rec = rec->Next();
	}

	gw->setRndLimits(savedLimits);	
	return res;	
}

int PatchSelMod::Display (TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc) {
	return 0;
}

void PatchSelMod::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp *vpt, Box3& box, ModContext *mc) {
	box.Init();
}

void PatchSelMod::GetSubObjectCenters (SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc) {
	if (!mc->localData) return;
	if (selLevel == SEL_OBJECT) return;	// shouldn't happen.
	PatchSelData *modData = (PatchSelData *) mc->localData;
	PatchMesh *pmesh = modData->GetMesh();
	if (!pmesh) return;
	Matrix3 tm = node->GetObjectTM(t);

	// For Patch Select, we merely return the center of the bounding box of the current selection.
	BitArray sel = modData->TempVSel(*pmesh, selLevel);
	if (!sel.NumberSet()) return;
	Box3 box;
	for (int i=0; i<pmesh->numVerts; i++) if (sel[i]) box += pmesh->verts[i].p * tm;
	cb->Center (box.Center(),0);
}

void PatchSelMod::GetSubObjectTMs (SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc) {
	if (!mc->localData) return;
	if (selLevel == SEL_OBJECT) return;	// shouldn't happen.
	PatchSelData *modData = (PatchSelData *) mc->localData;
	PatchMesh *pmesh = modData->GetMesh();
	if (!pmesh) return;
	Matrix3 tm = node->GetObjectTM(t);

	// For Patch Select, we merely return the center of the bounding box of the current selection.
	BitArray sel = modData->TempVSel(*pmesh, selLevel);
	if (!sel.NumberSet()) return;
	Box3 box;
	for (int i=0; i<pmesh->numVerts; i++) if (sel[i]) box += pmesh->verts[i].p * tm;
	Matrix3 ctm(1);
	ctm.SetTrans (box.Center());
	cb->TM (ctm,0);
}

void PatchSelMod::ActivateSubobjSel(int level, XFormModes& modes) {
	// Set the meshes level
	selLevel = level;

	// Fill in modes with our sub-object modes
	if (level!=SEL_OBJECT) {
		modes = XFormModes(NULL,NULL,NULL,NULL,NULL,selectMode);
	}

	// Update UI
	UpdateSelLevelDisplay ();
	SetEnableStates ();
	SetNumSelLabel ();

	// Setup named selection sets	
	SetupNamedSelDropDown();
	UpdateNamedSelDropDown ();

	NotifyDependents(FOREVER, PART_SUBSEL_TYPE|PART_DISPLAY, REFMSG_CHANGE);
	ip->PipeSelLevelChanged();
	NotifyDependents(FOREVER, SELECT_CHANNEL|DISP_ATTRIB_CHANNEL|SUBSEL_TYPE_CHANNEL, REFMSG_CHANGE);
}

void PatchSelMod::UpdateNamedSelDropDown () {
	if (!ip) return;
	if (selLevel == SEL_OBJECT) {
		ip->ClearCurNamedSelSet ();
		return;
	}
	// See if this selection matches a named set
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts (mcList, nodes);
	BitArray nselmatch;
	nselmatch.SetSize (namedSel[namedSetLevel[selLevel]].Count());
	nselmatch.SetAll ();
	int nd, i, foundone=FALSE;
	for (nd=0; nd<mcList.Count(); nd++) {
		PatchSelData *d = (PatchSelData *) mcList[nd]->localData;
		if (!d) continue;
		foundone = TRUE;
		switch (selLevel) {
		// CAL-06/10/03: add handle sub-object mode. (FID #1914)
		case SEL_HANDLE:
			for (i=0; i<nselmatch.GetSize(); i++) {
				if (!nselmatch[i]) continue;
				if (!(*(d->hselSet.sets[i]) == d->vecSel)) nselmatch.Clear(i);
			}
			break;
		case SEL_VERTEX:
			for (i=0; i<nselmatch.GetSize(); i++) {
				if (!nselmatch[i]) continue;
				if (!(*(d->vselSet.sets[i]) == d->vertSel)) nselmatch.Clear(i);
			}
			break;
		case SEL_EDGE:
			for (i=0; i<nselmatch.GetSize(); i++) {
				if (!nselmatch[i]) continue;
				if (!(*(d->eselSet.sets[i]) == d->edgeSel)) nselmatch.Clear(i);
			}
			break;
		default:	// Patch and Element
			for (i=0; i<nselmatch.GetSize(); i++) {
				if (!nselmatch[i]) continue;
				if (!(*(d->pselSet.sets[i]) == d->patchSel)) nselmatch.Clear(i);
			}
			break;
		}
		if (nselmatch.NumberSet () == 0) break;
	}
	if (foundone && nselmatch.NumberSet ()) {
		for (i=0; i<nselmatch.GetSize(); i++) if (nselmatch[i]) break;
		ip->SetCurNamedSelSet (*(namedSel[namedSetLevel[selLevel]][i]));
	} else ip->ClearCurNamedSelSet ();
}

DWORD PatchSelMod::GetSelLevel () {
	switch (selLevel) {
	case SEL_HANDLE:	return PO_HANDLE;
	case SEL_VERTEX:	return PO_VERTEX;
	case SEL_EDGE:		return PO_EDGE;
	case SEL_PATCH:		return PO_PATCH;
	case SEL_ELEMENT:	return PO_ELEMENT;
	default:			return PO_OBJECT;
	}
}

void PatchSelMod::SetSelLevel(DWORD sl) {
	switch (sl) {
	case PO_OBJECT:
		selLevel = SEL_OBJECT;
		break;
	// CAL-06/10/03: add handle sub-object mode. (FID #1914)
	case PO_HANDLE:
		selLevel = SEL_HANDLE;
		break;
	case PO_VERTEX:
		selLevel = SEL_VERTEX;
		break;
	case PO_EDGE:
		selLevel = SEL_EDGE;
		break;
	case PO_PATCH:
		selLevel = SEL_PATCH;
		break;
	case PO_ELEMENT:
		selLevel = SEL_ELEMENT;
		break;
	}
	if (ip && editMod==this) ip->SetSubObjectLevel(selLevel);
}

void PatchSelMod::LocalDataChanged() {
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	if (ip && editMod==this) {
		SetNumSelLabel();
		UpdateNamedSelDropDown ();
	}
}

void PatchSelMod::SelectSubComponent (HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert) {
	PatchSelData *d = NULL, *od = NULL;

	ip->ClearCurNamedSelSet();

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	TimeValue t = ip->GetTime();
	int nd;
	BitArray nsel;
	AdjFaceList *al = NULL;
	PatchMesh *pmesh;

	for (nd=0; nd<mcList.Count(); nd++) {
		d = (PatchSelData*) mcList[nd]->localData;
		if (d==NULL) continue;
		HitRecord *hr = hitRec;
		if (!all && (hr->modContext->localData != d)) continue;
		for (; hr!=NULL; hr=hr->Next()) if (hr->modContext->localData == d) break;
		if (hr==NULL) continue;

		if ((selLevel > SEL_VERTEX) && selByVert) {
			// Gather BitArray of vertices that were hit:
			BitArray vhit;
			vhit.SetSize (d->pmesh->numVerts);
			vhit.ClearAll ();
			for (; hr != NULL; hr = hr->Next()) {
				if (d != hr->modContext->localData) continue;
				PatchHitData *phd = (PatchHitData *) hr->hitData;
				if (phd->type == PATCH_HIT_VERTEX) vhit.Set (phd->index, TRUE);
				if (!all) break;
			}
			// Use that to modify edge or face selection:
			// CAL-06/10/03: add handle sub-object mode. (FID #1914)
			if (selLevel == SEL_HANDLE) {
				nsel = d->vecSel;
				for (int i=0; i<d->pmesh->numVecs; i++) {
					int v = pmesh->vecs[i].vert;
					if (v < 0 || !vhit[v]) continue;
					nsel.Set (i, invert ? !nsel[i] : selected);
				}
				d->SetVecSel (nsel, this, t);
			} else
			if (selLevel == SEL_EDGE) {
				nsel = d->edgeSel;
				for (int i=0; i<d->pmesh->numEdges; i++) {
					if (!vhit[d->pmesh->edges[i].v1] && !vhit[d->pmesh->edges[i].v2]) continue;
					nsel.Set (i, invert ? !nsel[i] : selected);
				}
				d->SetEdgeSel (nsel, this, t);
			} else
			if (selLevel == SEL_PATCH) {
				nsel = d->patchSel;
				for (int i=0; i<d->pmesh->numPatches; i++) {
					for (int j=0; j<d->pmesh->patches[i].type; j++) {
						if (vhit[d->pmesh->patches[i].v[j]]) break;
					}
					if (j == d->pmesh->patches[i].type) continue;
					nsel.Set (i, invert ? !nsel[i] : selected);
				}
				d->SetPatchSel (nsel, this, t);
			} else {	// Element
				pmesh = d->GetMesh();
				if (!pmesh) break;
				nsel = d->patchSel;
				BitArray mask(d->patchSel.GetSize());
				mask.ClearAll();
				for (int i=0; i<d->vertSel.GetSize(); i++) {
					if(vhit[i]) {
						PatchVert &v = pmesh->verts[i];
						for(int j = 0; j < v.patches.Count(); ++j) {
							if(!mask[v.patches[j]])
								mask |= pmesh->GetElement(v.patches[j]);
						}
					}
				}
				if( invert )
					nsel ^= mask;
				else
				if ( selected )
					nsel |= mask;
				else
					nsel &= ~mask;
				d->SetPatchSel (nsel, this, t);
			}
			continue;
		}

		switch (selLevel) {
		// CAL-06/10/03: add handle sub-object mode. (FID #1914)
		case SEL_HANDLE:
			nsel = d->vecSel;
			for (; hr != NULL; hr = hr->Next()) {
				if (d != hr->modContext->localData) continue;
				PatchHitData *phd = (PatchHitData *) hr->hitData;
				if (phd->type == PATCH_HIT_VECTOR || phd->type == PATCH_HIT_INTERIOR) {
					nsel.Set (phd->index, invert ? !nsel[phd->index] : selected);
				}
				if (!all) break;
			}
			d->SetVecSel (nsel, this, t);
			break;

		case SEL_VERTEX:
			nsel = d->vertSel;
			for (; hr != NULL; hr = hr->Next()) {
				if (d != hr->modContext->localData) continue;
				PatchHitData *phd = (PatchHitData *) hr->hitData;
				if (phd->type == PATCH_HIT_VERTEX) {
					nsel.Set (phd->index, invert ? !nsel[phd->index] : selected);
				}
				if (!all) break;
			}
			d->SetVertSel (nsel, this, t);
			break;

		case SEL_EDGE:
			nsel = d->edgeSel;
			for (; hr != NULL; hr = hr->Next()) {
				if (d != hr->modContext->localData) continue;
				PatchHitData *phd = (PatchHitData *) hr->hitData;
				nsel.Set (phd->index, invert ? !nsel[phd->index] : selected);
				if (!all) break;
			}
			d->SetEdgeSel (nsel, this, t);
			break;

		case SEL_PATCH:
			pmesh = d->GetMesh();
			if (!pmesh) break;
			nsel = d->patchSel;
			for (; hr != NULL; hr=hr->Next()) {
				if (d != hr->modContext->localData) continue;
				PatchHitData *phd = (PatchHitData *) hr->hitData;
				nsel.Set (phd->index, invert ? !pmesh->patchSel[phd->index] : selected);
				if (!all) break;
			}
			d->SetPatchSel (nsel, this, t);
			break;
		case SEL_ELEMENT:
			pmesh = d->GetMesh();
			if (!pmesh) break;
			nsel = d->patchSel;
			BitArray mask(nsel.GetSize());
			mask.ClearAll();
			for (; hr != NULL; hr=hr->Next()) {
				if (d != hr->modContext->localData) continue;
				PatchHitData *phd = (PatchHitData *) hr->hitData;
				if(!mask[phd->index])
					mask |= pmesh->GetElement(phd->index);
				if (!all) break;
			}
			if( invert )
				nsel ^= mask;
			else
			if ( selected )
				nsel |= mask;
			else
				nsel &= ~mask;
			d->SetPatchSel (nsel, this, t);
			break;
		}
	}

	nodes.DisposeTemporary ();
	LocalDataChanged ();
}

void PatchSelMod::ClearSelection(int selLevel) {
	ModContextList list;
	INodeTab nodes;	
	ip->GetModContexts(list,nodes);
	PatchSelData *d;
	BitArray nsel;
	for (int i=0; i<list.Count(); i++) {
		d = (PatchSelData*)list[i]->localData;
		if (!d) continue;

		switch (selLevel) {
		// CAL-06/10/03: add handle sub-object mode. (FID #1914)
		case SEL_HANDLE:
			if (!d->vecSel.NumberSet()) break;
			nsel.SetSize (d->vecSel.GetSize());
			nsel.ClearAll ();
			d->SetVecSel (nsel, this, ip->GetTime());
			break;

		case SEL_VERTEX:
			if (!d->vertSel.NumberSet()) break;
			nsel.SetSize (d->vertSel.GetSize());
			nsel.ClearAll ();
			d->SetVertSel (nsel, this, ip->GetTime());
			break;

		case SEL_PATCH:
		case SEL_ELEMENT:
			if (!d->patchSel.NumberSet()) break;
			nsel.SetSize (d->patchSel.GetSize());
			nsel.ClearAll ();
			d->SetPatchSel (nsel, this, ip->GetTime());
			break;

		case SEL_EDGE:
			if (!d->edgeSel.NumberSet()) break;
			nsel.SetSize (d->edgeSel.GetSize());
			nsel.ClearAll ();
			d->SetEdgeSel (nsel, this, ip->GetTime());
			break;
		}
	}
	nodes.DisposeTemporary();
	LocalDataChanged ();
}

void PatchSelMod::SelectAll(int selLevel) {
	ModContextList list;
	INodeTab nodes;	
	ip->GetModContexts(list,nodes);
	PatchSelData *d;
	BitArray nsel;
	for (int i=0; i<list.Count(); i++) {
		d = (PatchSelData*)list[i]->localData;		
		if (!d) continue;
		switch (selLevel) {
		// CAL-06/10/03: add handle sub-object mode. (FID #1914)
		case SEL_HANDLE:
			nsel.SetSize (d->vecSel.GetSize());
			nsel.SetAll();
			d->SetVecSel (nsel, this, ip->GetTime());
			break;
		case SEL_VERTEX:
			nsel.SetSize (d->vertSel.GetSize());
			nsel.SetAll();
			d->SetVertSel (nsel, this, ip->GetTime());
			break;
		case SEL_PATCH:
		case SEL_ELEMENT:
			nsel.SetSize (d->patchSel.GetSize());
			nsel.SetAll();
			d->SetPatchSel (nsel, this, ip->GetTime());
			break;
		case SEL_EDGE:
			nsel.SetSize (d->edgeSel.GetSize());
			nsel.SetAll();
			d->SetEdgeSel (nsel, this, ip->GetTime());
			break;
		}
	}
	nodes.DisposeTemporary();
	LocalDataChanged ();
}

void PatchSelMod::InvertSelection(int selLevel) {
	ModContextList list;
	INodeTab nodes;	
	ip->GetModContexts(list,nodes);
	PatchSelData *d;
	for (int i=0; i<list.Count(); i++) {
		d = (PatchSelData*)list[i]->localData;
		if (!d) continue;
		switch (selLevel) {
		// CAL-06/10/03: add handle sub-object mode. (FID #1914)
		case SEL_HANDLE:
			d->SetVecSel (~d->vecSel, this, ip->GetTime());
			break;
		case SEL_VERTEX:
			d->SetVertSel (~d->vertSel, this, ip->GetTime());
			break;
		case SEL_PATCH:
		case SEL_ELEMENT:
			d->SetPatchSel (~d->patchSel, this, ip->GetTime());
			break;
		case SEL_EDGE:
			d->SetEdgeSel (~d->edgeSel, this, ip->GetTime());
			break;
		}
	}
	nodes.DisposeTemporary();
	LocalDataChanged ();
}

void PatchSelMod::SelectByMatID(int id) {
	BOOL add = GetKeyState(VK_CONTROL)<0 ? TRUE : FALSE;
	BOOL sub = GetKeyState(VK_MENU)<0 ? TRUE : FALSE;
	theHold.Begin();
	ModContextList list;
	INodeTab nodes;	
	ip->GetModContexts(list,nodes);
	PatchSelData *d;
	BitArray nsel;
	for (int i=0; i<list.Count(); i++) {
		d = (PatchSelData*)list[i]->localData;		
		if (!d) continue;
		nsel = d->patchSel;
		if (!add && !sub) nsel.ClearAll();
		PatchMesh *pmesh = d->GetMesh();
		if (!pmesh) continue;
		if(selLevel == SEL_PATCH) {
			for (int i=0; i<pmesh->numPatches; i++) {
				if (pmesh->patches[i].getMatID()==(MtlID)id) {
					if (sub) nsel.Clear(i);
					else nsel.Set(i);
				}
			}
		}
		else {		// Elements
			BitArray mask(d->patchSel.GetSize());
			mask.ClearAll();
			for (int i=0; i<pmesh->numPatches; i++) {
				if(!mask[i] && pmesh->patches[i].getMatID()==(MtlID)id) {
					mask |= pmesh->GetElement(i);
				}
			}
			if(sub)	nsel &= ~mask;
			else nsel |= mask;
		}
		d->SetPatchSel (nsel, this, ip->GetTime ());
	}
	nodes.DisposeTemporary();
	theHold.Accept(GetString (IDS_PS_SELECTBYMATID));
	LocalDataChanged ();
	ip->RedrawViews(ip->GetTime());
}

void PatchSelMod::SelectOpenEdges() {
	theHold.Begin();
	ModContextList list;
	INodeTab nodes;
	ip->GetModContexts(list,nodes);
	PatchSelData *d;
	BitArray nsel;
	for (int i=0; i<list.Count(); i++) {
		d = (PatchSelData*)list[i]->localData;		
		if (!d) continue;
		if (!d->pmesh) continue;

		nsel.SetSize (d->pmesh->numEdges);
		nsel.ClearAll ();

		for (int j=0; j<d->pmesh->numEdges; j++) {
			PatchEdge & pe = d->pmesh->edges[j];
			if (pe.patches.Count() < 2) nsel.Set (j);	// TH 3/24/00
		}
		d->SetEdgeSel (nsel, this, ip->GetTime());
	}
	nodes.DisposeTemporary();
	theHold.Accept(GetString (IDS_PS_SELECT_OPEN));
	LocalDataChanged ();
	ip->RedrawViews(ip->GetTime());
}

void PatchSelMod::SelectFrom(int from) {
	ModContextList list;
	INodeTab nodes;
	ip->GetModContexts(list,nodes);
	PatchSelData *d;
	theHold.Begin();
	for (int i=0; i<list.Count(); i++) {
		d = (PatchSelData*)list[i]->localData;
		if (!d) continue;

		switch (selLevel) {
		// CAL-06/10/03: add handle sub-object mode. (FID #1914)
		case SEL_HANDLE:
			if (from==SEL_VERTEX) d->SetVecSel (d->SelHandleByVert(), this, ip->GetTime());
			else if (from==SEL_EDGE) d->SetVecSel (d->SelHandleByEdge(), this, ip->GetTime());
			else d->SetVecSel (d->SelHandleByPatch(), this, ip->GetTime());
			break;
		case SEL_VERTEX: 
			if (from>=SEL_PATCH) d->SetVertSel (d->SelVertByPatch(), this, ip->GetTime());
			else d->SetVertSel (d->SelVertByEdge(), this, ip->GetTime());
			break;
		case SEL_PATCH:
			if (from==SEL_VERTEX) d->SetPatchSel (d->SelPatchByVert(), this, ip->GetTime());
			else d->SetPatchSel (d->SelPatchByEdge(), this, ip->GetTime());
			break;
		case SEL_ELEMENT:
			if (from==SEL_VERTEX) d->SetPatchSel (d->SelElementByVert(), this, ip->GetTime());
			else if (from==SEL_EDGE) d->SetPatchSel (d->SelElementByEdge(), this, ip->GetTime());
			else d->SetPatchSel (d->SelElementByPatch(), this, ip->GetTime());
			break;
		case SEL_EDGE:
			if (from==SEL_VERTEX) d->SetEdgeSel (d->SelEdgeByVert(), this, ip->GetTime());
			else d->SetEdgeSel (d->SelEdgeByPatch(), this, ip->GetTime());
			break;
		}
	}
	theHold.Accept(GetString(IDS_PS_SELECTION));
	nodes.DisposeTemporary();
	LocalDataChanged ();
	ip->RedrawViews(ip->GetTime());
}

#define SELLEVEL_CHUNKID		0x0100
#define VECSEL_CHUNKID			0x0110
#define VERTSEL_CHUNKID			0x0200
#define FACESEL_CHUNKID			0x0210
#define EDGESEL_CHUNKID			0x0220
#define VERSION_CHUNKID        0x0230
#define FLAGS_CHUNKID 0x0240
static int currentVersion = 3;

#define NAMEDVSEL_NAMES_CHUNK	0x2805
#define NAMEDFSEL_NAMES_CHUNK	0x2806
#define NAMEDESEL_NAMES_CHUNK	0x2807
#define NAMEDHSEL_NAMES_CHUNK	0x2808
#define NAMEDSEL_STRING_CHUNK	0x2809
#define NAMEDSEL_ID_CHUNK		0x2810

#define HSELSET_CHUNK			0x2844
#define VSELSET_CHUNK			0x2845
#define FSELSET_CHUNK			0x2846
#define ESELSET_CHUNK			0x2847

#define SOFT_SELECTION_CHUNK	0x2875

static int namedSelID[] = {NAMEDVSEL_NAMES_CHUNK,NAMEDESEL_NAMES_CHUNK,NAMEDFSEL_NAMES_CHUNK,NAMEDHSEL_NAMES_CHUNK};

IOResult PatchSelMod::Save(ISave *isave) {
	IOResult res;
	ULONG nb;
	Modifier::Save(isave);
	isave->BeginChunk(SELLEVEL_CHUNKID);
	res = isave->Write(&selLevel, sizeof(selLevel), &nb);
	isave->EndChunk();
	isave->BeginChunk (VERSION_CHUNKID);
	res = isave->Write (&currentVersion, sizeof(int), &nb);
	isave->EndChunk();

	for (int j=0; j<NS_LEVELS; j++) {
		if (namedSel[j].Count()) {
			isave->BeginChunk(namedSelID[j]);			
			for (int i=0; i<namedSel[j].Count(); i++) {
				isave->BeginChunk(NAMEDSEL_STRING_CHUNK);
				isave->WriteWString(*namedSel[j][i]);
				isave->EndChunk();

				isave->BeginChunk(NAMEDSEL_ID_CHUNK);
				isave->Write(&ids[j][i],sizeof(DWORD),&nb);
				isave->EndChunk();
				}
			isave->EndChunk();
			}
		}

	isave->BeginChunk (SOFT_SELECTION_CHUNK);
		res = isave->Write ( &mFalloff, sizeof(float), &nb );
		res = isave->Write ( &mBubble, sizeof(float), &nb );
		res = isave->Write ( &mPinch, sizeof(float), &nb );
		res = isave->Write ( &mUseSoftSelections, sizeof(int), &nb );
		res = isave->Write ( &mUseEdgeDists, sizeof(int), &nb );
		res = isave->Write ( &mEdgeDist, sizeof(int), &nb );
		res = isave->Write ( &mAffectBackface, sizeof(int), &nb );
	isave->EndChunk();

	return res;
	}

IOResult PatchSelMod::LoadNamedSelChunk(ILoad *iload,int level) {	
	IOResult res;
	DWORD ix=0;
	ULONG nb;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
		case NAMEDSEL_STRING_CHUNK: {
			TCHAR *name;
			res = iload->ReadWStringChunk(&name);
			//AddSet(TSTR(name),level+1);
			TSTR *newName = new TSTR(name);
			namedSel[level].Append(1,&newName);				
			ids[level].Append(1,&ix);
			ix++;
			break;
			}
		case NAMEDSEL_ID_CHUNK:
			iload->Read(&ids[level][ids[level].Count()-1],sizeof(DWORD), &nb);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

IOResult PatchSelMod::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	int version = 2;
	Modifier::Load(iload);
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
		case SELLEVEL_CHUNKID:
			iload->Read(&selLevel, sizeof(selLevel), &nb);
			break;

		case VERSION_CHUNKID:
			iload->Read (&version, sizeof(selLevel), &nb);
			break;

		case NAMEDVSEL_NAMES_CHUNK:
			res = LoadNamedSelChunk(iload,NS_VERTEX);
			break;

		case NAMEDESEL_NAMES_CHUNK:
			res = LoadNamedSelChunk(iload,NS_EDGE);
			break;

		case NAMEDFSEL_NAMES_CHUNK:
			res = LoadNamedSelChunk(iload,NS_PATCH);
			break;

		case NAMEDHSEL_NAMES_CHUNK:
			res = LoadNamedSelChunk(iload,NS_HANDLE);
			break;

		case SOFT_SELECTION_CHUNK:
			res = iload->Read( &mFalloff, sizeof(float), &nb );
			res = iload->Read( &mBubble, sizeof(float), &nb );
			res = iload->Read( &mPinch, sizeof(float), &nb );
			res = iload->Read( &mUseSoftSelections, sizeof(int), &nb );
			res = iload->Read( &mUseEdgeDists, sizeof(int), &nb );
			res = iload->Read( &mEdgeDist, sizeof(int), &nb );
			res = iload->Read( &mAffectBackface, sizeof(int), &nb );
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	if (version<3) {
		if ((selLevel>1) && (selLevel<4)) selLevel = 5-selLevel;	// switched patches, edges in 3.0
	}
	return IO_OK;
}

IOResult PatchSelMod::SaveLocalData(ISave *isave, LocalModData *ld) {	
	PatchSelData *d = (PatchSelData*)ld;

	// CAL-06/10/03: add handle sub-object mode. (FID #1914)
	isave->BeginChunk(VECSEL_CHUNKID);
	d->vecSel.Save(isave);
	isave->EndChunk();

	isave->BeginChunk(VERTSEL_CHUNKID);
	d->vertSel.Save(isave);
	isave->EndChunk();

	isave->BeginChunk(FACESEL_CHUNKID);
	d->patchSel.Save(isave);
	isave->EndChunk();

	isave->BeginChunk(EDGESEL_CHUNKID);
	d->edgeSel.Save(isave);
	isave->EndChunk();
	
	// CAL-06/10/03: add handle sub-object mode. (FID #1914)
	if (d->hselSet.Count()) {
		isave->BeginChunk(HSELSET_CHUNK);
		d->hselSet.Save(isave);
		isave->EndChunk();
		}
	if (d->vselSet.Count()) {
		isave->BeginChunk(VSELSET_CHUNK);
		d->vselSet.Save(isave);
		isave->EndChunk();
		}
	if (d->eselSet.Count()) {
		isave->BeginChunk(ESELSET_CHUNK);
		d->eselSet.Save(isave);
		isave->EndChunk();
		}
	if (d->pselSet.Count()) {
		isave->BeginChunk(FSELSET_CHUNK);
		d->pselSet.Save(isave);
		isave->EndChunk();
		}

	return IO_OK;
	}

IOResult PatchSelMod::LoadLocalData(ILoad *iload, LocalModData **pld) {
	PatchSelData *d = new PatchSelData;
	*pld = d;
	IOResult res;	
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			// CAL-06/10/03: add handle sub-object mode. (FID #1914)
			case VECSEL_CHUNKID:
				d->vecSel.Load(iload);
				break;
			case VERTSEL_CHUNKID:
				d->vertSel.Load(iload);
				break;
			case FACESEL_CHUNKID:
				d->patchSel.Load(iload);
				break;
			case EDGESEL_CHUNKID:
				d->edgeSel.Load(iload);
				break;

			// CAL-06/10/03: add handle sub-object mode. (FID #1914)
			case HSELSET_CHUNK:
				res = d->hselSet.Load(iload);
				break;
			case VSELSET_CHUNK:
				res = d->vselSet.Load(iload);
				break;
			case FSELSET_CHUNK:
				res = d->pselSet.Load(iload);
				break;
			case ESELSET_CHUNK:
				res = d->eselSet.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}



// Window Procs ------------------------------------------------------

void PatchSelMod::SetEnableStates() {
	ICustButton *but;
	ISpinnerControl *spin;
	EnableWindow (GetDlgItem (hParams, IDC_PS_SEL_BYVERT), selLevel && (selLevel != SEL_VERTEX));
	EnableWindow (GetDlgItem (hParams, IDC_PS_IGNORE_BACKFACES), selLevel>SEL_VERTEX);

	but = GetICustButton (GetDlgItem (hParams, IDC_PS_GETVERT));
	but->Enable (selLevel && (selLevel != SEL_VERTEX));
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hParams, IDC_PS_GETEDGE));
	but->Enable (selLevel && (selLevel != SEL_EDGE));
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hParams, IDC_PS_GETPATCH));
	but->Enable (selLevel && (selLevel != SEL_PATCH));
	ReleaseICustButton (but);

	EnableWindow (GetDlgItem (hParams, IDC_PS_SELBYMAT_BOX), selLevel >= SEL_PATCH);
	EnableWindow (GetDlgItem (hParams, IDC_PS_SELBYMAT_TEXT), selLevel >= SEL_PATCH);
	but = GetICustButton (GetDlgItem (hParams, IDC_PS_SELBYMAT));
	but->Enable (selLevel >= SEL_PATCH);
	ReleaseICustButton (but);
	spin = GetISpinner (GetDlgItem (hParams, IDC_PS_MATIDSPIN));
	spin->Enable (selLevel >= SEL_PATCH);
	ReleaseISpinner (spin);

	but = GetICustButton (GetDlgItem (hParams, IDC_PS_COPYNS));
	but->Enable (selLevel);
	ReleaseICustButton(but);
	but = GetICustButton (GetDlgItem (hParams,IDC_PS_PASTENS));
	but->Enable (selLevel &&
		(GetPatchNamedSelClip (namedClipLevel[selLevel]) ? TRUE : FALSE));
	ReleaseICustButton (but);

	but = GetICustButton (GetDlgItem (hParams, IDC_PS_SELOPEN));
	but->Enable (selLevel==SEL_EDGE);
	ReleaseICustButton (but);
}

class PatchSelImageHandler {
public:
	HIMAGELIST images;
	PatchSelImageHandler () { images = NULL; }
	~PatchSelImageHandler () { if (images) ImageList_Destroy (images); }
	HIMAGELIST LoadImages ();
};

HIMAGELIST PatchSelImageHandler::LoadImages () {
	if (images) return images;

	HBITMAP hBitmap, hMask;
	images = ImageList_Create(24, 23, ILC_COLOR|ILC_MASK, 10, 0);
	hBitmap = LoadBitmap (hInstance, MAKEINTRESOURCE(IDB_PATCHSELTYPES));
	hMask = LoadBitmap (hInstance, MAKEINTRESOURCE(IDB_PATCHSELMASK));
	ImageList_Add (images, hBitmap, hMask);
	DeleteObject(hBitmap);
	DeleteObject(hMask);
	return images;
}

static PatchSelImageHandler thePatchSelImageHandler;

static INT_PTR CALLBACK PatchSelectProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {	
	PatchSelMod *mod = (PatchSelMod*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	ICustToolbar *iToolbar;
	ISpinnerControl *spin;

	switch (msg) {
	case WM_INITDIALOG:
		mod = (PatchSelMod*)lParam;
		mod->hParams = hWnd;
		SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);

		iToolbar = GetICustToolbar(GetDlgItem(hWnd,IDC_PS_SELTYPE));
		iToolbar->SetImage (thePatchSelImageHandler.LoadImages());
		// CAL-06/10/03: add handle sub-object mode. (FID #1914)
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,0,5,0,5,24,23,24,23,EP_VERTEX));
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,1,6,1,6,24,23,24,23,EP_HANDLE));
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,2,7,2,7,24,23,24,23,EP_EDGE));
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,3,8,3,8,24,23,24,23,EP_PATCH));
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,4,9,4,9,24,23,24,23,EP_ELEMENT));
		ReleaseICustToolbar(iToolbar);
		mod->UpdateSelLevelDisplay ();

		spin = GetISpinner(GetDlgItem(hWnd,IDC_PS_MATIDSPIN));
		spin->SetLimits(1, 65535, FALSE);
		spin->SetScale(0.1f);
		spin->LinkToEdit(GetDlgItem(hWnd,IDC_PS_MATID), EDITTYPE_INT);
		spin->SetValue(lastMatID,FALSE);
		ReleaseISpinner(spin);

		CheckDlgButton(hWnd,IDC_PS_SEL_BYVERT,mod->selByVert);
		CheckDlgButton(hWnd,IDC_PS_IGNORE_BACKFACES,mod->ignoreBackfaces);
		mod->SetEnableStates();
		break;

	case WM_UPDATE_CACHE:
		mod->UpdateCache((TimeValue)wParam);
 		break;

	case WM_DESTROY:
		spin = GetISpinner(GetDlgItem(hWnd,IDC_PS_MATIDSPIN));
		lastMatID = spin->GetIVal();
		ReleaseISpinner(spin);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_PS_SELBYMAT:
			spin = GetISpinner(GetDlgItem(hWnd,IDC_PS_MATIDSPIN));
			mod->SelectByMatID(spin->GetIVal()-1);
			ReleaseISpinner(spin);
			break;

		// CAL-06/10/03: add handle sub-object mode. (FID #1914)
		case EP_HANDLE:
			if (mod->selLevel == SEL_HANDLE) mod->ip->SetSubObjectLevel (SEL_OBJECT);
			else mod->ip->SetSubObjectLevel (SEL_HANDLE);
			break;

		case EP_VERTEX:
			if (mod->selLevel == SEL_VERTEX) mod->ip->SetSubObjectLevel (SEL_OBJECT);
			else mod->ip->SetSubObjectLevel (SEL_VERTEX);
			break;

		case EP_EDGE:
			if (mod->selLevel == SEL_EDGE) mod->ip->SetSubObjectLevel (SEL_OBJECT);
			else mod->ip->SetSubObjectLevel (SEL_EDGE);
			break;

		case EP_PATCH:
			if (mod->selLevel == SEL_PATCH) mod->ip->SetSubObjectLevel (SEL_OBJECT);
			else mod->ip->SetSubObjectLevel (SEL_PATCH);
			break;

		case EP_ELEMENT:
			if (mod->selLevel == SEL_ELEMENT) mod->ip->SetSubObjectLevel (SEL_OBJECT);
			else mod->ip->SetSubObjectLevel (SEL_ELEMENT);
			break;

		case IDC_PS_SEL_BYVERT:
			mod->selByVert = IsDlgButtonChecked(hWnd,IDC_PS_SEL_BYVERT);
			break;

		case IDC_PS_IGNORE_BACKFACES:
			mod->ignoreBackfaces = IsDlgButtonChecked(hWnd,IDC_PS_IGNORE_BACKFACES);
			break;

		case IDC_PS_GETVERT: mod->SelectFrom(SEL_VERTEX); break;
		case IDC_PS_GETEDGE: mod->SelectFrom(SEL_EDGE); break;
		case IDC_PS_GETPATCH: mod->SelectFrom(SEL_PATCH); break;

		case IDC_PS_SELOPEN: mod->SelectOpenEdges(); break;

		case IDC_PS_COPYNS:  mod->NSCopy();  break;
		case IDC_PS_PASTENS: mod->NSPaste(); break;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code != TTN_NEEDTEXT) break;
		LPTOOLTIPTEXT lpttt;
		lpttt = (LPTOOLTIPTEXT)lParam;				
		switch (lpttt->hdr.idFrom) {
		// CAL-06/10/03: add handle sub-object mode. (FID #1914)
		case EP_HANDLE:
			lpttt->lpszText = GetString (IDS_PS_HANDLE);
			break;
		case EP_VERTEX:
			lpttt->lpszText = GetString (IDS_PS_VERTEX);
			break;
		case EP_EDGE:
			lpttt->lpszText = GetString (IDS_PS_EDGE);
			break;
		case EP_PATCH:
			lpttt->lpszText = GetString (IDS_PS_PATCH);
			break;
		case EP_ELEMENT:
			lpttt->lpszText = GetString (IDS_PS_ELEMENT);
			break;
		}
		break;
	
	default: return FALSE;
	}
	return TRUE;
}

static INT_PTR CALLBACK PickSetNameDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static TSTR *name;

	switch (msg) {
		case WM_INITDIALOG: {
			name = (TSTR*)lParam;
			ICustEdit *edit =GetICustEdit(GetDlgItem(hWnd,IDC_SET_NAME));
			edit->SetText(*name);
			ReleaseICustEdit(edit);
			break;
			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK: {
					ICustEdit *edit =GetICustEdit(GetDlgItem(hWnd,IDC_SET_NAME));
					TCHAR buf[256];
					edit->GetText(buf,256);
					*name = TSTR(buf);
					ReleaseICustEdit(edit);
					EndDialog(hWnd,1);
					break;
					}

				case IDCANCEL:
					EndDialog(hWnd,0);
					break;
				}
			break;

		default:
			return FALSE;
		};
	return TRUE;
	}

BOOL PatchSelMod::GetUniqueSetName(TSTR &name) {
	while (1) {				
		Tab<TSTR*> &setList = namedSel[namedSetLevel[selLevel]];

		BOOL unique = TRUE;
		for (int i=0; i<setList.Count(); i++) {
			if (name==*setList[i]) {
				unique = FALSE;
				break;
			}
		}
		if (unique) break;

		if (!ip) return FALSE;
		if (!DialogBoxParam (hInstance, MAKEINTRESOURCE(IDD_PASTE_NAMEDSET),
			ip->GetMAXHWnd(), PickSetNameDlgProc, (LPARAM)&name)) return FALSE;
		if (!ip) return FALSE;
	}
	return TRUE;
}

static INT_PTR CALLBACK PickSetDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:	{
		Tab<TSTR*> *setList = (Tab<TSTR*>*)lParam;
		for (int i=0; i<setList->Count(); i++) {
			int pos  = SendDlgItemMessage(hWnd,IDC_NS_LIST,LB_ADDSTRING,0,
				(LPARAM)(TCHAR*)*(*setList)[i]);
			SendDlgItemMessage(hWnd,IDC_NS_LIST,LB_SETITEMDATA,pos,i);
		}
		break;
	}

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_NS_LIST:
			if (HIWORD(wParam)!=LBN_DBLCLK) break;
			// fall through

		case IDOK:
			int sel;
			sel = SendDlgItemMessage(hWnd,IDC_NS_LIST,LB_GETCURSEL,0,0);
			if (sel!=LB_ERR) {
				int res =SendDlgItemMessage(hWnd,IDC_NS_LIST,LB_GETITEMDATA,sel,0);
				EndDialog(hWnd,res);
				break;
			}
			// fall through

		case IDCANCEL:
			EndDialog(hWnd,-1);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

// PatchSelData -----------------------------------------------------

LocalModData *PatchSelData::Clone() {
	PatchSelData *d = new PatchSelData;
	d->vecSel = vecSel;
	d->vertSel = vertSel;
	d->patchSel = patchSel;
	d->edgeSel = edgeSel;
	d->hselSet = hselSet;
	d->vselSet = vselSet;
	d->pselSet = pselSet;
	d->eselSet = eselSet;
	held = FALSE;
	pmesh = NULL;
	return d;
}

PatchSelData::PatchSelData(PatchMesh &pmesh) {
	vecSel = pmesh.vecSel;
	vertSel = pmesh.vertSel;
	patchSel = pmesh.patchSel;
	edgeSel = pmesh.edgeSel;
	held = FALSE;
	this->pmesh = NULL;
}

void PatchSelData::SynchBitArrays()
{
	if (pmesh) {
		vecSel.SetSize(pmesh->vecSel.GetSize(),TRUE);
		vertSel.SetSize(pmesh->vertSel.GetSize(),TRUE);
		patchSel.SetSize(pmesh->patchSel.GetSize(),TRUE);
		edgeSel.SetSize(pmesh->edgeSel.GetSize(),TRUE);
	}
}

void PatchSelData::SetCache(PatchMesh &pmesh) {
	if (this->pmesh) delete this->pmesh;
	this->pmesh = new PatchMesh(pmesh);
	SynchBitArrays ();
}

void PatchSelData::FreeCache() {
	if (pmesh) delete pmesh;
	pmesh = NULL;
}

// CAL-06/10/03: add handle sub-object mode. (FID #1914)
void PatchSelData::SetVecSel(BitArray &set, IPatchSelect *imod, TimeValue t) {
	PatchSelMod *mod = (PatchSelMod *) imod;
	if (theHold.Holding()) theHold.Put (new PatchSelectRestore (mod, this, SEL_HANDLE));
	vecSel = set;
	if (pmesh) pmesh->vecSel = set;
}

void PatchSelData::SetVertSel(BitArray &set, IPatchSelect *imod, TimeValue t) {
	PatchSelMod *mod = (PatchSelMod *) imod;
	if (theHold.Holding()) theHold.Put (new PatchSelectRestore (mod, this, SEL_VERTEX));
	vertSel = set;
	if (pmesh) pmesh->vertSel = set;
}

void PatchSelData::SetEdgeSel(BitArray &set, IPatchSelect *imod, TimeValue t) {
	PatchSelMod *mod = (PatchSelMod *) imod;
	if (theHold.Holding()) theHold.Put (new PatchSelectRestore (mod, this, SEL_EDGE));
	edgeSel = set;
	if (pmesh) pmesh->edgeSel = set;
}

void PatchSelData::SetPatchSel(BitArray &set, IPatchSelect *imod, TimeValue t) {
	PatchSelMod *mod = (PatchSelMod *) imod;
	if (theHold.Holding()) theHold.Put (new PatchSelectRestore (mod, this, SEL_PATCH));
	patchSel = set;
	if (pmesh) pmesh->patchSel = set;
}

BitArray PatchSelData::SelVertByPatch() {
	BitArray nsel = vertSel;
	DbgAssert (pmesh);
	if (!pmesh) return nsel;
	for (int i=0; i<pmesh->getNumPatches(); i++) {
		if (patchSel[i]) {
			for (int j=0; j<pmesh->patches[i].type; j++) {
				nsel.Set (pmesh->patches[i].v[j]);
			}
		}
	}
	return nsel;
}

BitArray PatchSelData::SelVertByEdge() {
	BitArray nsel = vertSel;
	DbgAssert (pmesh);
	if (!pmesh) return nsel;
	for (int i=0; i<pmesh->getNumEdges(); i++) {		
		if (edgeSel[i]) {
			nsel.Set(pmesh->edges[i].v1,TRUE);
			nsel.Set(pmesh->edges[i].v2,TRUE);
		}
	}
	return nsel;
}

BitArray PatchSelData::SelPatchByVert() {
	BitArray nsel = patchSel;
	DbgAssert (pmesh);
	if (!pmesh) return nsel;
	for (int i=0; i<pmesh->getNumPatches(); i++) {
		Patch &p = pmesh->patches[i];
		for (int j=0; j<p.type; j++) {
			if (vertSel[pmesh->patches[i].v[j]]) {
				nsel.Set(i);
				break;
			}
		}
	}
	return nsel;
}

BitArray PatchSelData::SelPatchByEdge() {
	BitArray nsel = patchSel;
	DbgAssert (pmesh);
	if (!pmesh) return nsel;
	for (int i=0; i<pmesh->getNumEdges(); i++) {
		if (!edgeSel[i]) continue;
// TH 3/24/00
		for(int j = 0; j < pmesh->edges[i].patches.Count(); ++j)
			nsel.Set (pmesh->edges[i].patches[j]);
	}
	return nsel;
}

BitArray PatchSelData::SelEdgeByVert() {
	BitArray nsel = edgeSel;
	DbgAssert (pmesh);
	if (!pmesh) return nsel;
	for (int i=0; i<pmesh->getNumEdges(); i++) {
		if (vertSel[pmesh->edges[i].v1]) nsel.Set(i);
		if (vertSel[pmesh->edges[i].v2]) nsel.Set(i);
	}
	return nsel;
}

BitArray PatchSelData::SelEdgeByPatch() {
	BitArray nsel = edgeSel;
	DbgAssert (pmesh);
	if (!pmesh) return nsel;
	for (int i=0; i<pmesh->getNumEdges(); i++) {
		if (nsel[i]) continue;
// TH 3/24/00
		for(int j = 0; j < pmesh->edges[i].patches.Count(); ++j) {
			if (patchSel[pmesh->edges[i].patches[j]]) {
				nsel.Set(i);
				continue;
			}
		}
	}
	return nsel;
}

BitArray PatchSelData::SelElementByVert() {
	DbgAssert (pmesh);
	if (!pmesh) return patchSel;
	BitArray mask(patchSel.GetSize());
	mask.ClearAll();
	for (int i=0; i<pmesh->getNumVerts(); i++) {
		if(vertSel[i]) {
			PatchVert &v = pmesh->verts[i];
			for(int j = 0; j < v.patches.Count(); ++j) {
				if(!mask[v.patches[j]])
					mask |= pmesh->GetElement(v.patches[j]);
			}
		}
	}
	return patchSel | mask;
}

BitArray PatchSelData::SelElementByEdge() {
	DbgAssert (pmesh);
	if (!pmesh) return patchSel;
	BitArray mask(patchSel.GetSize());
	mask.ClearAll();
	for (int i=0; i<pmesh->getNumEdges(); i++) {
		if(edgeSel[i]) {
			PatchEdge &e = pmesh->edges[i];
			for(int j = 0; j < e.patches.Count(); ++j) {
				if(!mask[e.patches[j]])
					mask |= pmesh->GetElement(e.patches[j]);
			}
		}
	}
	return patchSel | mask;
}

BitArray PatchSelData::SelElementByPatch() {
	DbgAssert (pmesh);
	if (!pmesh) return patchSel;
	BitArray mask(patchSel.GetSize());
	mask.ClearAll();
	for (int i=0; i<pmesh->getNumPatches(); i++) {
		if(patchSel[i]) {
			if(!mask[i])
				mask |= pmesh->GetElement(i);
		}
	}
	return patchSel | mask;
}

// CAL-06/10/03: add handle sub-object mode. (FID #1914)
BitArray PatchSelData::SelHandleByVert() {
	BitArray nsel = vecSel;
	DbgAssert (pmesh);
	if (!pmesh) return nsel;
	for (int i=0; i<pmesh->getNumVerts(); i++) {
		if(vertSel[i]) {
			PatchVert &v = pmesh->verts[i];
			for(int j = 0; j < v.vectors.Count(); ++j) {
				nsel.Set(v.vectors[j]);
			}
		}
	}
	return nsel;
}

// CAL-06/10/03: add handle sub-object mode. (FID #1914)
BitArray PatchSelData::SelHandleByEdge() {
	BitArray nsel = vecSel;
	DbgAssert (pmesh);
	if (!pmesh) return nsel;
	for (int i=0; i<pmesh->getNumVerts(); i++) {
		PatchVert &v = pmesh->verts[i];
		for (int e = 0; e < v.edges.Count(); e++) if (edgeSel[v.edges[e]]) break;
		if (e < v.edges.Count()) {
			for(int j = 0; j < v.vectors.Count(); ++j) {
				nsel.Set(v.vectors[j]);
			}
		}
	}
	return nsel;
}

// CAL-06/10/03: add handle sub-object mode. (FID #1914)
BitArray PatchSelData::SelHandleByPatch() {
	BitArray nsel = vecSel;
	DbgAssert (pmesh);
	if (!pmesh) return nsel;
	for (int i=0; i<pmesh->getNumVerts(); i++) {
		PatchVert &v = pmesh->verts[i];
		for (int p = 0; p < v.patches.Count(); p++) if (patchSel[v.patches[p]]) break;
		if (p < v.patches.Count()) {
			for(int j = 0; j < v.vectors.Count(); ++j) {
				nsel.Set(v.vectors[j]);
			}
		}
	}
	return nsel;
}

// PatchSelectRestore --------------------------------------------------

PatchSelectRestore::PatchSelectRestore(PatchSelMod *m, PatchSelData *data) {
	mod     = m;
	level   = mod->selLevel;
	d       = data;
	d->held = TRUE;
	switch (level) {
	case SEL_OBJECT: MaxAssert(0); break;
	case SEL_HANDLE: usel = d->vecSel; break;	// CAL-06/10/03: (FID #1914)
	case SEL_VERTEX: usel = d->vertSel; break;
	case SEL_EDGE: usel = d->edgeSel; break;
	default: usel = d->patchSel; break;
	}
}

PatchSelectRestore::PatchSelectRestore(PatchSelMod *m, PatchSelData *data, int sLevel) {
	mod     = m;
	level   = sLevel;
	d       = data;
	d->held = TRUE;
	switch (level) {
	case SEL_OBJECT: MaxAssert(0); break;
	case SEL_HANDLE: usel = d->vecSel; break;	// CAL-06/10/03: (FID #1914)
	case SEL_VERTEX: usel = d->vertSel; break;
	case SEL_EDGE: usel = d->edgeSel; break;
	default: usel = d->patchSel; break;
	}
}

void PatchSelectRestore::Restore(int isUndo) {
	if (isUndo) {
		switch (level) {
		case SEL_HANDLE: rsel = d->vecSel; break;	// CAL-06/10/03: (FID #1914)
		case SEL_VERTEX: rsel = d->vertSel; break;
		case SEL_PATCH:
		case SEL_ELEMENT: rsel = d->patchSel; break;
		case SEL_EDGE: rsel = d->edgeSel; break;
		}
	}
	switch (level) {
	case SEL_HANDLE: d->vecSel = usel; break;		// CAL-06/10/03: (FID #1914)
	case SEL_VERTEX: d->vertSel = usel; break;
	case SEL_PATCH:
	case SEL_ELEMENT: d->patchSel = usel; break;
	case SEL_EDGE: d->edgeSel = usel; break;
	}
	mod->NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	mod->SetNumSelLabel();
	mod->UpdateNamedSelDropDown ();
}

void PatchSelectRestore::Redo() {
	switch (level) {
	case SEL_HANDLE: d->vecSel = rsel; break;		// CAL-06/10/03: (FID #1914)
	case SEL_VERTEX: d->vertSel = rsel; break;
	case SEL_PATCH:
	case SEL_ELEMENT: d->patchSel = rsel; break;
	case SEL_EDGE: d->edgeSel = rsel; break;
	}
	mod->NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	mod->SetNumSelLabel();
	mod->UpdateNamedSelDropDown ();
}


//--- Named selection sets -----------------------------------------

int PatchSelMod::FindSet(TSTR &setName, int level) {
	for (int i=0; i<namedSel[level].Count(); i++) {
		if (setName == *namedSel[level][i]) return i;
	}
	return -1;
}

DWORD PatchSelMod::AddSet(TSTR &setName,int level) {
	DWORD id = 0;
	TSTR *name = new TSTR(setName);
	namedSel[level].Append(1,&name);
	BOOL found = FALSE;
	while (!found) {
		found = TRUE;
		for (int i=0; i<ids[level].Count(); i++) {
			if (ids[level][i]!=id) continue;
			id++;
			found = FALSE;
			break;
		}
	}
	ids[level].Append(1,&id);
	return id;
}

void PatchSelMod::RemoveSet(TSTR &setName,int level) {
	int i = FindSet(setName,level);
	if (i<0) return;
	delete namedSel[level][i];
	namedSel[level].Delete(i,1);
	ids[level].Delete(i,1);
}

void PatchSelMod::UpdateSetNames () {
	if (!ip) return;
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	for (int i=0; i<mcList.Count(); i++) {
		PatchSelData *meshData = (PatchSelData*)mcList[i]->localData;
		if ( !meshData ) continue;
		for (int nsl=0; nsl<NS_LEVELS; nsl++) {
			// Make sure the namedSel array is in alpha order.
			// (Crude bubble sort since we expect that it will be.)
			int j, k, kmax = namedSel[nsl].Count();
			for (k=1; k<kmax; k++) {
				if (*(namedSel[nsl][k-1]) < *(namedSel[nsl][k])) continue;
				for (j=0; j<k-1; j++) {
					if (*(namedSel[nsl][j]) > *(namedSel[nsl][k])) break;
				}
				// j now represents the point at which k should be inserted.
				TSTR *hold = namedSel[nsl][k];
				DWORD dhold = ids[nsl][k];
				int j2;
				for (j2=k; j2>j; j2--) {
					namedSel[nsl][j2] = namedSel[nsl][j2-1];
					ids[nsl][j2] = ids[nsl][j2-1];
				}
				namedSel[nsl][j] = hold;
				ids[nsl][j] = dhold;
			}

			GenericNamedSelSetList & gnsl = meshData->GetNamedSel(nsl);
			// Check for old, unnamed sets with ids.
			for (k=0; k<gnsl.Count(); k++) {
				if (gnsl.names[k]) continue;
				for (j=0; j<ids[nsl].Count(); j++) if (ids[nsl][j] == gnsl.ids[k]) break;
				if (j == ids[nsl].Count()) continue;
				gnsl.names[j] = new TSTR(*(namedSel[nsl][j]));
			}
			gnsl.Alphabetize ();

			// Now check lists against each other, adding any missing elements.
			for (j=0; j<gnsl.Count(); j++) {
				if (*(gnsl.names[j]) == *(namedSel[nsl][j])) continue;
				if (j>= namedSel[nsl].Count()) {
					TSTR *nname = new TSTR(*gnsl.names[j]);
					DWORD nid = gnsl.ids[j];
					namedSel[nsl].Append (1, &nname);
					ids[nsl].Append (1, &nid);
					continue;
				}
				if (*(gnsl.names[j]) > *(namedSel[nsl][j])) {
					BitArray baTemp;
					gnsl.InsertSet (j, baTemp, ids[nsl][j], *(namedSel[nsl][j]));
					continue;
				}
				// Otherwise:
				TSTR *nname = new TSTR(*gnsl.names[j]);
				DWORD nid = gnsl.ids[j];
				namedSel[nsl].Insert (j, 1, &nname);
				ids[nsl].Insert (j, 1, &nid);
			}
			for (; j<namedSel[nsl].Count(); j++) {
				BitArray baTemp;
				gnsl.AppendSet (baTemp, ids[nsl][j], *(namedSel[nsl][j]));
			}
		}
	}

	nodes.DisposeTemporary();
}

void PatchSelMod::ClearSetNames() {
	for (int i=0; i<NS_LEVELS; i++) {
		for (int j=0; j<namedSel[i].Count(); j++) {
			delete namedSel[i][j];
			namedSel[i][j] = NULL;
		}
	}
}

void PatchSelMod::ActivateSubSelSet(TSTR &setName) {
	ModContextList mcList;
	INodeTab nodes;
	int nsl = namedSetLevel[selLevel];
	int index = FindSet (setName, nsl);	
	if (index<0 || !ip) return;

	theHold.Begin ();
	ip->GetModContexts(mcList,nodes);

	for (int i = 0; i < mcList.Count(); i++) {
		PatchSelData *meshData = (PatchSelData*)mcList[i]->localData;
		if (!meshData) continue;
		if (theHold.Holding() && !meshData->held) theHold.Put(new PatchSelectRestore(this,meshData));

		BitArray *set = NULL;

		switch (nsl) {
		// CAL-06/10/03: add handle sub-object mode. (FID #1914)
		case NS_HANDLE:
			set = meshData->hselSet.GetSet(ids[nsl][index]);
			if (set) {
				if (set->GetSize()!=meshData->vecSel.GetSize()) {
					set->SetSize(meshData->vecSel.GetSize(),TRUE);
				}
				meshData->SetVecSel (*set, this, ip->GetTime());
			}
			break;

		case NS_VERTEX:
			set = meshData->vselSet.GetSet(ids[nsl][index]);
			if (set) {
				if (set->GetSize()!=meshData->vertSel.GetSize()) {
					set->SetSize(meshData->vertSel.GetSize(),TRUE);
				}
				meshData->SetVertSel (*set, this, ip->GetTime());
			}
			break;

		case NS_PATCH:	// Patch and Element
			set = meshData->pselSet.GetSet(ids[nsl][index]);
			if (set) {
				if (set->GetSize()!=meshData->patchSel.GetSize()) {
					set->SetSize(meshData->patchSel.GetSize(),TRUE);
				}
				meshData->SetPatchSel (*set, this, ip->GetTime());
			}
			break;

		case NS_EDGE:
			set = meshData->eselSet.GetSet(ids[nsl][index]);
			if (set) {
				if (set->GetSize()!=meshData->edgeSel.GetSize()) {
					set->SetSize(meshData->edgeSel.GetSize(),TRUE);
				}
				meshData->SetEdgeSel (*set, this, ip->GetTime());
			}
			break;
		}
	}
	
	nodes.DisposeTemporary();
	LocalDataChanged ();
	theHold.Accept (GetString (IDS_PS_SELECTION));
	ip->RedrawViews(ip->GetTime());
}

void PatchSelMod::NewSetFromCurSel(TSTR &setName) {
	ModContextList mcList;
	INodeTab nodes;
	DWORD id = -1;
	int nsl = namedSetLevel[selLevel];
	int index = FindSet(setName, nsl);
	if (index<0) id = AddSet(setName, nsl);
	else id = ids[nsl][index];

	ip->GetModContexts(mcList,nodes);

	for (int i = 0; i < mcList.Count(); i++) {
		PatchSelData *meshData = (PatchSelData*)mcList[i]->localData;
		if (!meshData) continue;
		
		BitArray *set = NULL;

		switch (nsl) {
		// CAL-06/10/03: add handle sub-object mode. (FID #1914)
		case NS_HANDLE:	
			if (index>=0 && (set = meshData->hselSet.GetSet(id))) {
				*set = meshData->vecSel;
			} else meshData->hselSet.InsertSet(meshData->vecSel,id, setName);
			break;

		case NS_VERTEX:	
			if (index>=0 && (set = meshData->vselSet.GetSet(id))) {
				*set = meshData->vertSel;
			} else meshData->vselSet.InsertSet(meshData->vertSel,id, setName);
			break;

		case NS_PATCH:	// Patch and Element
			if (index>=0 && (set = meshData->pselSet.GetSet(id))) {
				*set = meshData->patchSel;
			} else meshData->pselSet.InsertSet(meshData->patchSel,id, setName);
			break;

		case NS_EDGE:
			if (index>=0 && (set = meshData->eselSet.GetSet(id))) {
				*set = meshData->edgeSel;
			} else meshData->eselSet.InsertSet(meshData->edgeSel,id, setName);
			break;
		}
	}	
	nodes.DisposeTemporary();
}

void PatchSelMod::RemoveSubSelSet(TSTR &setName) {
	int nsl = namedSetLevel[selLevel];
	int index = FindSet (setName, nsl);
	if (index<0 || !ip) return;		

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	DWORD id = ids[nsl][index];

	for (int i = 0; i < mcList.Count(); i++) {
		PatchSelData *meshData = (PatchSelData*)mcList[i]->localData;
		if (!meshData) continue;		

		switch (nsl) {
			// CAL-06/10/03: add handle sub-object mode. (FID #1914)
			case NS_HANDLE:	
				if (theHold.Holding()) theHold.Put(new DeleteSetRestore(&meshData->hselSet,id,setName));
				meshData->hselSet.RemoveSet(id);
				break;

			case NS_VERTEX:	
				if (theHold.Holding()) theHold.Put(new DeleteSetRestore(&meshData->vselSet,id,setName));
				meshData->vselSet.RemoveSet(id);
				break;

			case NS_PATCH:	// Patch and Element
				if (theHold.Holding()) theHold.Put(new DeleteSetRestore(&meshData->pselSet,id,setName));
				meshData->pselSet.RemoveSet(id);
				break;

			case NS_EDGE:
				if (theHold.Holding()) theHold.Put(new DeleteSetRestore(&meshData->eselSet,id,setName));
				meshData->eselSet.RemoveSet(id);
				break;
			}		
		}
	
	if (theHold.Holding()) theHold.Put(new DeleteSetNameRestore(&(namedSel[nsl]),this,&(ids[nsl]),id));
	RemoveSet (setName, nsl);
	ip->ClearCurNamedSelSet();
	nodes.DisposeTemporary();
}

void PatchSelMod::SetupNamedSelDropDown() {
	if (selLevel == SEL_OBJECT) return;
	ip->ClearSubObjectNamedSelSets();
	int nsl = namedSetLevel[selLevel];
	for (int i=0; i<namedSel[nsl].Count(); i++)
		ip->AppendSubObjectNamedSelSet(*namedSel[nsl][i]);
}

int PatchSelMod::NumNamedSelSets() {
	int nsl = namedSetLevel[selLevel];
	return namedSel[nsl].Count();
}

TSTR PatchSelMod::GetNamedSelSetName(int i) {
	int nsl = namedSetLevel[selLevel];
	return *namedSel[nsl][i];
}

void PatchSelMod::SetNamedSelSetName(int i,TSTR &newName) {
	int nsl = namedSetLevel[selLevel];
	if (theHold.Holding()) theHold.Put(new SetNameRestore(&namedSel[nsl],this,&ids[nsl],ids[nsl][i]));
	*namedSel[nsl][i] = newName;
}

void PatchSelMod::NewSetByOperator(TSTR &newName,Tab<int> &sets,int op) {
	ModContextList mcList;
	INodeTab nodes;
	
	int nsl = namedSetLevel[selLevel];
	DWORD id = AddSet(newName, nsl);
	if (theHold.Holding()) theHold.Put(new AppendSetNameRestore(this,&namedSel[nsl],&ids[nsl]));

	BOOL delSet = TRUE;
	ip->GetModContexts(mcList,nodes);
	for (int i = 0; i < mcList.Count(); i++) {
		PatchSelData *meshData = (PatchSelData*)mcList[i]->localData;
		if (!meshData) continue;
	
		BitArray bits;
		GenericNamedSelSetList *setList;

		switch (nsl) {
		case NS_HANDLE: setList = &meshData->hselSet; break;	// CAL-06/10/03: (FID #1914)
		case NS_VERTEX: setList = &meshData->vselSet; break;
		case NS_PATCH: setList = &meshData->pselSet; break;	// Patch and Element
		case NS_EDGE:   setList = &meshData->eselSet; break;			
		}		

		bits = (*setList)[sets[0]];

		for (int i=1; i<sets.Count(); i++) {
			switch (op) {
			case NEWSET_MERGE:
				bits |= (*setList)[sets[i]];
				break;

			case NEWSET_INTERSECTION:
				bits &= (*setList)[sets[i]];
				break;

			case NEWSET_SUBTRACT:
				bits &= ~((*setList)[sets[i]]);
				break;
			}
		}
		if (bits.NumberSet()) delSet = FALSE;

		if (!delSet) setList->InsertSet (bits, id, newName);
		if (theHold.Holding()) theHold.Put(new AppendSetRestore(setList, id, newName));
	}
	if (delSet) RemoveSubSelSet(newName);
}

void PatchSelMod::NSCopy() {
	int index = SelectNamedSet();
	if (index<0) return;
	if (!ip) return;

	int nsl = namedSetLevel[selLevel];
	PatchNamedSelClip *clip = new PatchNamedSelClip(*namedSel[nsl][index]);

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	for (int i = 0; i < mcList.Count(); i++) {
		PatchSelData *meshData = (PatchSelData*)mcList[i]->localData;
		if (!meshData) continue;

		GenericNamedSelSetList *setList;
		switch (nsl) {
		case NS_HANDLE: setList = &meshData->hselSet; break;	// CAL-06/10/03: (FID #1914)
		case NS_VERTEX: setList = &meshData->vselSet; break;				
		case NS_PATCH: setList = &meshData->pselSet; break;	// Patch and Element
		case NS_EDGE: setList = &meshData->eselSet; break;			
		}		

		BitArray *bits = new BitArray(*setList->GetSet(ids[nsl][index]));
		clip->sets.Append(1,&bits);
	}
	SetPatchNamedSelClip(clip, namedClipLevel[selLevel]);
	
	// Enable the paste button
	ICustButton *but;
	but = GetICustButton(GetDlgItem(hParams,IDC_PS_PASTENS));
	but->Enable();
	ReleaseICustButton(but);
}

void PatchSelMod::NSPaste() {
	int nsl = namedSetLevel[selLevel];
	PatchNamedSelClip *clip = GetPatchNamedSelClip(namedClipLevel[selLevel]);
	if (!clip) return;	
	TSTR name = clip->name;
	if (!GetUniqueSetName(name)) return;

	ModContextList mcList;
	INodeTab nodes;
	theHold.Begin();

	DWORD id = AddSet (name, nsl);	
	if (theHold.Holding()) theHold.Put(new AppendSetNameRestore(this, &namedSel[nsl], &ids[nsl]));	

	ip->GetModContexts(mcList,nodes);
	for (int i = 0; i < mcList.Count(); i++) {
		PatchSelData *meshData = (PatchSelData*)mcList[i]->localData;
		if (!meshData) continue;

		GenericNamedSelSetList *setList;
		switch (nsl) {
		case NS_HANDLE: setList = &meshData->hselSet; break;	// CAL-06/10/03: (FID #1914)
		case NS_VERTEX: setList = &meshData->vselSet; break;
		case NS_EDGE: setList = &meshData->eselSet; break;
		case NS_PATCH: setList = &meshData->pselSet; break;	// Patch and Element
		}
				
		if (i>=clip->sets.Count()) {
			BitArray bits;
			setList->InsertSet(bits,id,name);
		} else setList->InsertSet(*clip->sets[i],id,name);
		if (theHold.Holding()) theHold.Put (new AppendSetRestore (setList, id, name));		
	}	
	
	ActivateSubSelSet(name);
	ip->SetCurNamedSelSet(name);
	theHold.Accept(GetString (IDS_PS_PASTE_NS));
	SetupNamedSelDropDown();
}

int PatchSelMod::SelectNamedSet() {
	Tab<TSTR*> &setList = namedSel[namedSetLevel[selLevel]];
	if (!ip) return FALSE;
	return DialogBoxParam (hInstance, MAKEINTRESOURCE(IDD_SEL_NAMEDSET),
		ip->GetMAXHWnd(), PickSetDlgProc, (LPARAM)&setList);
}

void PatchSelMod::SetNumSelLabel() {	
	TSTR buf;
	int num = 0, which;

	if (!hParams) return;

	ModContextList mcList;
	INodeTab nodes;

	ip->GetModContexts(mcList,nodes);
	for (int i = 0; i < mcList.Count(); i++) {
		PatchSelData *meshData = (PatchSelData*)mcList[i]->localData;
		if (!meshData) continue;

		switch (selLevel) {
		// CAL-06/10/03: add handle sub-object mode. (FID #1914)
		case SEL_HANDLE:
			num += meshData->vecSel.NumberSet();
			if (meshData->vecSel.NumberSet() == 1) {
				for (which=0; which<meshData->vecSel.GetSize(); which++) if (meshData->vecSel[which]) break;
			}
			break;
		case SEL_VERTEX:
			num += meshData->vertSel.NumberSet();
			if (meshData->vertSel.NumberSet() == 1) {
				for (which=0; which<meshData->vertSel.GetSize(); which++) if (meshData->vertSel[which]) break;
			}
			break;
		case SEL_PATCH:
		case SEL_ELEMENT:
			num += meshData->patchSel.NumberSet();
			if (meshData->patchSel.NumberSet() == 1) {
				for (which=0; which<meshData->patchSel.GetSize(); which++) if (meshData->patchSel[which]) break;
			}
			break;
		case SEL_EDGE:
			num += meshData->edgeSel.NumberSet();
			if (meshData->edgeSel.NumberSet() == 1) {
				for (which=0; which<meshData->edgeSel.GetSize(); which++) if (meshData->edgeSel[which]) break;
			}
			break;
		}
	}

	switch (selLevel) {
	// CAL-06/10/03: add handle sub-object mode. (FID #1914)
	case SEL_HANDLE:
		if (num==1) buf.printf (GetString(IDS_PS_NUMHANDLESEL), which+1);
		else buf.printf(GetString(IDS_PS_NUMHANDLESELP),num);
		break;

	case SEL_VERTEX:			
		if (num==1) buf.printf (GetString(IDS_PS_NUMVERTSEL), which+1);
		else buf.printf(GetString(IDS_PS_NUMVERTSELP),num);
		break;

	case SEL_PATCH:
	case SEL_ELEMENT:
		if (num==1) buf.printf (GetString(IDS_PS_NUMPATCHSEL), which+1);
		else buf.printf(GetString(IDS_PS_NUMPATCHSELP),num);
		break;

	case SEL_EDGE:
		if (num==1) buf.printf (GetString(IDS_PS_NUMEDGESEL), which+1);
		else buf.printf(GetString(IDS_PS_NUMEDGESELP),num);
		break;

	case SEL_OBJECT:
		buf = GetString (IDS_PS_OBJECT_SEL);
		break;
	}

	SetDlgItemText(hParams,IDC_PS_NUMBER_SEL,buf);
}

RefResult PatchSelMod::NotifyRefChanged (Interval changeInt, RefTargetHandle hTarget, 
   		PartID& partID, RefMessage message) {
	return(REF_SUCCEED);
}

int PatchSelMod::NumSubObjTypes() 
{ 
	return 5;
}

ISubObjType *PatchSelMod::GetSubObjType(int i) 
{

	static bool initialized = false;
	if(!initialized)
	{
		initialized = true;
		// CAL-06/10/03: add handle sub-object mode. (FID #1914)
		SOT_Handle.SetName(GetString(IDS_PS_HANDLE));
		SOT_Vertex.SetName(GetString(IDS_PS_VERTEX));
		SOT_Edge.SetName(GetString(IDS_PS_EDGE));
		SOT_Patch.SetName(GetString(IDS_PS_PATCH));
		SOT_Element.SetName(GetString(IDS_PS_ELEMENT));
	}

	switch(i)
	{
	case -1:
		if(GetSubObjectLevel() > 0)
			return GetSubObjType(GetSubObjectLevel()-1);
		break;
	case 0:
		return &SOT_Vertex;
	case 1:
		return &SOT_Edge;
	case 2:
		return &SOT_Patch;
	case 3:
		return &SOT_Element;
	// CAL-06/10/03: add handle sub-object mode. (FID #1914)
	case 4:
		return &SOT_Handle;
	}
	return NULL;
}


static float AffectRegionFunct( float dist, float falloff, float pinch, float bubble) 
{
	float u = ((falloff - dist)/falloff);
	float u2 = u*u;
	float s = 1.0f-u;	

	return (3*u*bubble*s + 3*u2*(1.0f-pinch))*s + u*u2;
}

void PatchSelMod::ApplySoftSelectionToPatch( PatchMesh * pPatch )
{
	bool calcVertexWeights = false;

	if ( patchLevel[selLevel] != pPatch->VertexWeightSelectLevel() ) {
		pPatch->selLevel = patchLevel[selLevel];
		calcVertexWeights = true;	
	}

	if ( mFalloff != pPatch->Falloff() ) {
		pPatch->SetFalloff( mFalloff );
		calcVertexWeights = true;	
	}

	if ( mBubble != pPatch->Bubble() ) {
		pPatch->SetFalloff( mFalloff );
		calcVertexWeights = true;	
	}

	if ( mPinch != pPatch->Pinch() ) {
		pPatch->SetPinch( mPinch );
		calcVertexWeights = true;	
	}

	if ( mUseEdgeDists != pPatch->UseEdgeDists() ) {
		pPatch->SetUseEdgeDists( mUseEdgeDists );
		calcVertexWeights = true;	
	}

	if ( mEdgeDist != pPatch->EdgeDist() ) {
		pPatch->SetEdgeDist( mEdgeDist );
		calcVertexWeights = true;	
	}

	if ( mAffectBackface != pPatch->AffectBackface() ) {
		pPatch->SetAffectBackface( mAffectBackface );
		calcVertexWeights = true;	
	}

	if ( mUseSoftSelections != pPatch->UseSoftSelections() ) {
		pPatch->SetUseSoftSelections( mUseSoftSelections );
		// calcVertexWeights = true; /* not necessary since SetUseSoftSelections(..) will calc the vertex weights for us */	
	}

	if ( calcVertexWeights ) {
		pPatch->UpdateVertexDists();
		pPatch->UpdateEdgeDists();
		pPatch->UpdateVertexWeights();
	}
}

void PatchSelMod::SetSoftSelDlgEnables( HWND hSoftSel ) {
	
	if ( !hSoftSel && !hSoftSelPanel )  // nothing to set
		return;

	if ( !hSoftSel && hSoftSelPanel ) { // user omitted hSoftSel param, use hSoftSelPanel
		hSoftSel = hSoftSelPanel;
	}

	switch ( selLevel ) {
	case EP_VERTEX:
	case EP_EDGE:
	case EP_PATCH:
	case EP_ELEMENT:
		{
			int useSoftSelections = UseSoftSelections();

			EnableWindow( GetDlgItem( hSoftSel, IDC_SOFT_SELECTION ), TRUE );
			EnableWindow( GetDlgItem( hSoftSel, IDC_EDGE_DIST ), useSoftSelections );
			EnableWindow( GetDlgItem( hSoftSel, IDC_AFFECT_BACKFACING ), useSoftSelections );
			EnableWindow( GetDlgItem( hSoftSel, IDC_EDGE ),    useSoftSelections );
			EnableWindow( GetDlgItem( hSoftSel, IDC_EDGE_SPIN ),    useSoftSelections );
			EnableWindow( GetDlgItem( hSoftSel, IDC_FALLOFF ), useSoftSelections );
			EnableWindow( GetDlgItem( hSoftSel, IDC_FALLOFF_LABEL ), useSoftSelections );
			EnableWindow( GetDlgItem( hSoftSel, IDC_FALLOFF_SPIN ), useSoftSelections );
			EnableWindow( GetDlgItem( hSoftSel, IDC_PINCH ),   useSoftSelections );
			EnableWindow( GetDlgItem( hSoftSel, IDC_PINCH_LABEL ),   useSoftSelections );
			EnableWindow( GetDlgItem( hSoftSel, IDC_PINCH_SPIN ),   useSoftSelections );
			EnableWindow( GetDlgItem( hSoftSel, IDC_BUBBLE ),  useSoftSelections );
			EnableWindow( GetDlgItem( hSoftSel, IDC_BUBBLE_LABEL ),  useSoftSelections );
			EnableWindow( GetDlgItem( hSoftSel, IDC_BUBBLE_SPIN ),  useSoftSelections );
		}
		break;

	default:
		{
			EnableWindow( GetDlgItem( hSoftSel, IDC_SOFT_SELECTION ), FALSE );
			EnableWindow( GetDlgItem( hSoftSel, IDC_EDGE_DIST ), FALSE );
			EnableWindow( GetDlgItem( hSoftSel, IDC_AFFECT_BACKFACING ), FALSE );
			EnableWindow( GetDlgItem( hSoftSel, IDC_EDGE ),    FALSE );
			EnableWindow( GetDlgItem( hSoftSel, IDC_EDGE_SPIN ),    FALSE );
			EnableWindow( GetDlgItem( hSoftSel, IDC_FALLOFF ), FALSE );
			EnableWindow( GetDlgItem( hSoftSel, IDC_FALLOFF_LABEL ), FALSE );
			EnableWindow( GetDlgItem( hSoftSel, IDC_FALLOFF_SPIN ), FALSE );
			EnableWindow( GetDlgItem( hSoftSel, IDC_PINCH ),   FALSE );
			EnableWindow( GetDlgItem( hSoftSel, IDC_PINCH_LABEL ),   FALSE );
			EnableWindow( GetDlgItem( hSoftSel, IDC_PINCH_SPIN ),   FALSE );
			EnableWindow( GetDlgItem( hSoftSel, IDC_BUBBLE ),  FALSE );
			EnableWindow( GetDlgItem( hSoftSel, IDC_BUBBLE_LABEL ),  FALSE );
			EnableWindow( GetDlgItem( hSoftSel, IDC_BUBBLE_SPIN ),  FALSE );
		}
		break;

	}
}


void PatchSelMod::SetUseSoftSelections( int useSoftSelections )
{
	if ( mUseSoftSelections == useSoftSelections ) return;

	mUseSoftSelections = useSoftSelections;
	if ( useSoftSelections ) {
		UpdateVertexDists();
		UpdateEdgeDists( );
		UpdateVertexWeights();
	}
	else {
		InvalidateVertexWeights();
	}
}

int PatchSelMod::UseSoftSelections()
{
	return mUseSoftSelections;
}

void PatchSelMod::SetUseEdgeDists( int useEdgeDist )
{
	if ( useEdgeDist == mUseEdgeDists ) return;

	mUseEdgeDists = useEdgeDist;
	if ( useEdgeDist ) {
		UpdateVertexDists();
		UpdateEdgeDists( );
		UpdateVertexWeights();
	}
	else {
		UpdateVertexWeights();
	}
}

int PatchSelMod::UseEdgeDists()
{
	return mUseEdgeDists;
}

void PatchSelMod::InvalidateVertexWeights() {

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts( mcList, nodes );

	for ( int i = 0; i < mcList.Count(); i++ ) {
		PatchSelData * pPatchData = (PatchSelData*)mcList[i]->localData;
		if ( !pPatchData ) 
			continue;	
		
		PatchMesh *pPatch = pPatchData->GetMesh();
		if(!pPatch)
			continue;

		pPatch->InvalidateVertexWeights();
	}
}

Point3 PatchSelMod::VertexNormal( PatchMesh * pPatch, int vIndex ) 
{

	PatchVert * pVert = pPatch->getVertPtr( vIndex );

	int vecCount = pVert->vectors.Count();
	if ( vecCount < 2 ) { assert(0); return Point3(0.0, 0.0, 1.0); }

	int vecIndex = pVert->vectors[0];
	PatchVec * pVec0 = pPatch->getVecPtr( vecIndex );

	vecIndex = pVert->vectors[1];
	PatchVec * pVec1 = pPatch->getVecPtr( vecIndex );

	Point3 deltaVec = Normalize( pVec0->p - pVert->p )^Normalize( pVec1->p - pVert->p );
	return deltaVec;
}

void PatchSelMod::UpdateVertexWeights () {

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	if ( mUseSoftSelections ) {

		// discover total # of verts in all contexts
		//
		int totalVertexCount = 0;
		int totalVectorCount = 0;
		for ( int i = 0; i < mcList.Count(); i++ ) {
			PatchSelData * pPatchData = (PatchSelData*)mcList[i]->localData;
			if ( !pPatchData ) 
				continue;	
			
			PatchMesh * pPatch = pPatchData->GetMesh();
			if(!pPatch)
				continue;

			pPatch->selLevel = patchLevel[selLevel];
			pPatch->SetFalloff( mFalloff );
			pPatch->SetBubble( mBubble );
			pPatch->SetPinch( mPinch );
			pPatch->SetUseEdgeDists( mUseEdgeDists );
			pPatch->SetEdgeDist( mEdgeDist );
			pPatch->SetAffectBackface( mAffectBackface );
			pPatch->SetUseSoftSelections( mUseSoftSelections );

			pPatch->UpdateVertexWeights();
		}

	}

}

void PatchSelMod::UpdateVertexDists( ) 
{
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	
	// discover total # of verts in all contexts
	//
	int totalVertexCount = 0;
	int totalVectorCount = 0;
	for ( int i = 0; i < mcList.Count(); i++ ) {
		PatchSelData * pPatchData = (PatchSelData*)mcList[i]->localData;
		if ( !pPatchData ) 
			continue;	
		
		PatchMesh * pPatch = pPatchData->GetMesh();
		if(!pPatch)
			continue;

		pPatch->selLevel = patchLevel[selLevel];
		pPatch->UpdateVertexDists();
	}

}

void PatchSelMod::UpdateEdgeDists( ) 
{

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	if ( mUseEdgeDists ) {
		for ( int i = 0; i < mcList.Count(); i++ ) {
			PatchSelData * pPatchData = (PatchSelData*)mcList[i]->localData;
			if ( !pPatchData ) 
				continue;	
			
			PatchMesh * pPatch = pPatchData->GetMesh();
			if(!pPatch)
				continue;

			pPatch->selLevel = patchLevel[selLevel];
			pPatch->UpdateEdgeDists();
		}
	}

}

#define GRAPHSTEPS 20
static void DrawCurve (HWND hWnd,HDC hdc) {

	float pinch, falloff, bubble;
	ISpinnerControl *spin = GetISpinner(GetDlgItem(hWnd,IDC_FALLOFF_SPIN));
	falloff = spin->GetFVal();
	ReleaseISpinner(spin);	

	spin = GetISpinner(GetDlgItem(hWnd,IDC_PINCH_SPIN));
	pinch = spin->GetFVal();
	ReleaseISpinner(spin);

	spin = GetISpinner(GetDlgItem(hWnd,IDC_BUBBLE_SPIN));
	bubble = spin->GetFVal();
	ReleaseISpinner(spin);	

	TSTR label = FormatUniverseValue(falloff);
	SetWindowText(GetDlgItem(hWnd,IDC_FARLEFTLABEL),label);
	SetWindowText(GetDlgItem(hWnd,IDC_FARRIGHTLABEL),label);

	Rect rect, orect;
	GetClientRectP(GetDlgItem(hWnd,IDC_AR_GRAPH),&rect);
	orect = rect;

	SelectObject(hdc,GetStockObject(NULL_PEN));
	SelectObject(hdc,GetStockObject(WHITE_BRUSH));
	Rectangle(hdc,rect.left,rect.top,rect.right,rect.bottom);	
	SelectObject(hdc,GetStockObject(NULL_BRUSH));
	
	rect.left   += 3;
	rect.right  -= 3;
	rect.top    += 20;
	rect.bottom -= 20;
	
	SelectObject(hdc,CreatePen(PS_DOT,0,GetCustSysColor(COLOR_BTNFACE)));
	MoveToEx(hdc,orect.left,rect.top,NULL);
	LineTo(hdc,orect.right,rect.top);
	MoveToEx(hdc,orect.left,rect.bottom,NULL);
	LineTo(hdc,orect.right,rect.bottom);
	MoveToEx(hdc,(rect.left+rect.right)/2,orect.top,NULL);
	LineTo(hdc,(rect.left+rect.right)/2,orect.bottom);
	DeleteObject(SelectObject(hdc,GetStockObject(BLACK_PEN)));
	
	MoveToEx(hdc,rect.left,rect.bottom,NULL);
	for (int i=0; i<=GRAPHSTEPS; i++) {
		float dist = falloff * float(abs(i-GRAPHSTEPS/2))/float(GRAPHSTEPS/2);		
		float y = AffectRegionFunct( dist, falloff, pinch, bubble );
		int ix = rect.left + int(float(rect.w()-1) * float(i)/float(GRAPHSTEPS));
		int	iy = rect.bottom - int(y*float(rect.h()-2)) - 1;
		if (iy<orect.top) iy = orect.top;
		if (iy>orect.bottom-1) iy = orect.bottom-1;
		LineTo(hdc, ix, iy);
	}
	
	WhiteRect3D(hdc,orect,TRUE);
}

INT_PTR CALLBACK SoftSelectDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
	{
	PatchSelMod *pEpm = (PatchSelMod *)GetWindowLongPtr( hDlg, GWLP_USERDATA );
			// WIN64 Cleanup: Shuler
	if ( !pEpm && message != WM_INITDIALOG ) return FALSE;

	ISpinnerControl *spin;
	Rect rect;

	switch ( message ) {
		case WM_INITDIALOG: {
		 	pEpm = (PatchSelMod *)lParam;
			SetWindowLongPtr( hDlg, GWLP_USERDATA, (INT_PTR)pEpm );		 	

			CheckDlgButton( hDlg, IDC_SOFT_SELECTION, pEpm->UseSoftSelections() );
			CheckDlgButton( hDlg, IDC_EDGE_DIST, pEpm->UseEdgeDists() );
			CheckDlgButton( hDlg, IDC_AFFECT_BACKFACING, pEpm->mAffectBackface );


			SetupIntSpinner   ( hDlg, IDC_EDGE_SPIN, IDC_EDGE, 1, 999, pEpm->mEdgeDist );
			SetupFloatSpinner ( hDlg, IDC_FALLOFF_SPIN, IDC_FALLOFF, 0.0, 9999999.0, pEpm->mFalloff );
			SetupFloatSpinner ( hDlg, IDC_PINCH_SPIN, IDC_PINCH, -10.0, 10.0, pEpm->mPinch );
			SetupFloatSpinner ( hDlg, IDC_BUBBLE_SPIN, IDC_BUBBLE, -10.0, 10.0, pEpm->mBubble );
		 	pEpm->SetSoftSelDlgEnables( hDlg );
			return TRUE;
			}

		case WM_DESTROY:
			return FALSE;

		case CC_SPINNER_CHANGE:
			spin = (ISpinnerControl*)lParam;

			switch ( LOWORD(wParam) ) {

				case IDC_EDGE_SPIN: 
					pEpm->mEdgeDist = spin->GetIVal(); 
					GetClientRectP(GetDlgItem(hDlg,IDC_AR_GRAPH),&rect);
					InvalidateRect(hDlg,&rect,FALSE);
					pEpm->UpdateEdgeDists();
					pEpm->UpdateVertexWeights();
					pEpm->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					pEpm->ip->RedrawViews(pEpm->ip->GetTime(),REDRAW_NORMAL);
					break;
				case IDC_FALLOFF_SPIN: 
					pEpm->mFalloff = spin->GetFVal(); 
					GetClientRectP(GetDlgItem(hDlg,IDC_AR_GRAPH),&rect);
					InvalidateRect(hDlg,&rect,FALSE);
					pEpm->UpdateVertexWeights();
					pEpm->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					pEpm->ip->RedrawViews(pEpm->ip->GetTime(),REDRAW_NORMAL);
					break;
				case IDC_PINCH_SPIN: 
					pEpm->mPinch = spin->GetFVal(); 
					GetClientRectP(GetDlgItem(hDlg,IDC_AR_GRAPH),&rect);
					InvalidateRect(hDlg,&rect,FALSE);
					pEpm->UpdateVertexWeights();
					pEpm->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					pEpm->ip->RedrawViews(pEpm->ip->GetTime(),REDRAW_NORMAL);
					break;
				case IDC_BUBBLE_SPIN: 
					pEpm->mBubble = spin->GetFVal(); 
					GetClientRectP(GetDlgItem(hDlg,IDC_AR_GRAPH),&rect);
					InvalidateRect(hDlg,&rect,FALSE);
					pEpm->UpdateVertexWeights();
					pEpm->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					pEpm->ip->RedrawViews(pEpm->ip->GetTime(),REDRAW_NORMAL);
					break;
				}
			break;
		case CC_SPINNER_BUTTONUP:
			switch( LOWORD(wParam) ) {
				// WTF? do these things ever get hit?
				//
				case IDC_BUBBLE_SPIN: 
					pEpm->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					pEpm->ip->RedrawViews(pEpm->ip->GetTime(),REDRAW_NORMAL);
					break;
				case IDC_PINCH_SPIN: 
					pEpm->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					pEpm->ip->RedrawViews(pEpm->ip->GetTime(),REDRAW_NORMAL);
					break;
				case IDC_FALLOFF_SPIN: 
					pEpm->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					pEpm->ip->RedrawViews(pEpm->ip->GetTime(),REDRAW_NORMAL);
					break;
				}
			break;

		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hDlg,&ps);
				DrawCurve(hDlg,hdc);
				EndPaint(hDlg,&ps);
			}
			return FALSE;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:   			
   			pEpm->ip->RollupMouseMessage(hDlg,message,wParam,lParam);
			return FALSE;		
		
		case WM_COMMAND:			
			switch ( LOWORD(wParam) ) {	
				// Normals
				case IDC_SOFT_SELECTION:
					{
					// use method called SetUseSoftSelections() and do all of this work there!
					//
					int useSoftSelections = IsDlgButtonChecked(hDlg, IDC_SOFT_SELECTION);
					pEpm->SetUseSoftSelections( useSoftSelections );
					pEpm->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					pEpm->ip->RedrawViews(pEpm->ip->GetTime(),REDRAW_NORMAL);
		 			pEpm->SetSoftSelDlgEnables( hDlg );
					}
					break;
				case IDC_EDGE_DIST:
					{
					int useEdgeDist = IsDlgButtonChecked(hDlg, IDC_EDGE_DIST);
					pEpm->SetUseEdgeDists( useEdgeDist );
					pEpm->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					pEpm->ip->RedrawViews(pEpm->ip->GetTime(),REDRAW_NORMAL);
					}
					break;
				case IDC_AFFECT_BACKFACING:
					pEpm->mAffectBackface = IsDlgButtonChecked(hDlg, IDC_AFFECT_BACKFACING);					
					pEpm->UpdateVertexWeights();
					pEpm->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					pEpm->ip->RedrawViews(pEpm->ip->GetTime(),REDRAW_NORMAL);
					break;
				}
			break;
		}
	
	return FALSE;
}

