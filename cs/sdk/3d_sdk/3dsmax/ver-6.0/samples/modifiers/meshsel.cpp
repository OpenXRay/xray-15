/**********************************************************************
 *<
	FILE: meshsel.cpp

	DESCRIPTION:  A selection modifier for meshes

	CREATED BY: Rolf Berteig

	HISTORY: 10/23/96

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "mods.h"
#include "iparamm2.h"
#include "MeshDLib.h"
#include "namesel.h"
#include "nsclip.h"
#include "istdplug.h"
#include "iColorMan.h"
#include "MaxIcon.h"
#include "modsres.h"
#include "resourceOverride.h"


// Local static instance.
static GenSubObjType SOT_Vertex(1);
static GenSubObjType SOT_Edge(2);
static GenSubObjType SOT_Face(3);
static GenSubObjType SOT_Poly(4);
static GenSubObjType SOT_Element(5);

// Named selection set levels:
#define NS_VERTEX 0
#define NS_EDGE 1
#define NS_FACE 2
static int namedSetLevel[] = { NS_VERTEX, NS_VERTEX, NS_EDGE, NS_FACE, NS_FACE, NS_FACE };
static int namedClipLevel[] = { CLIP_VERT, CLIP_VERT, CLIP_EDGE, CLIP_FACE, CLIP_FACE, CLIP_FACE };

#define WM_UPDATE_CACHE		(WM_USER+0x287)

static MeshSelImageHandler theMeshSelImageHandler;

// MeshSelMod flags:
#define MS_DISP_END_RESULT 0x01

// MeshSelMod References:
#define REF_PBLOCK 0

class MeshSelMod : public Modifier, public IMeshSelect, public FlagUser {
public:
	DWORD selLevel;
	Tab<TSTR*> namedSel[3];
	Tab<DWORD> ids[3];
	IParamBlock2 *pblock;

	static IObjParam *ip;
	static MeshSelMod *editMod;
	static SelectModBoxCMode *selectMode;
	static BOOL updateCachePosted;

	MeshSelMod();

	// From Animatable
	void DeleteThis() { delete this; }
	void GetClassName(TSTR& s) {s = GetString(IDS_RB_MESHSELMOD);}  
	virtual Class_ID ClassID() { return Class_ID(MESHSELECT_CLASS_ID,0);}		
	RefTargetHandle Clone(RemapDir& remap = NoRemap());
	TCHAR *GetObjectName() {return GetString(IDS_RB_MESHSELMOD);}
	void *GetInterface (ULONG id) { if (id==I_MESHSELECT) return (IMeshSelect *) this; else return Modifier::GetInterface(id); }

	// From modifier
	ChannelMask ChannelsUsed()  {return PART_GEOM|PART_TOPO;}
	ChannelMask ChannelsChanged() {return PART_SELECT|PART_SUBSEL_TYPE|PART_TOPO;} // RB 5/27/97: Had to include topo channel because in edge select mode this modifier turns on hidden edges -- which may cause the edge list to be rebuilt, which is effectively a change in topology since the edge list is now part of the topo channel.
	Class_ID InputType() {return triObjectClassID;}
	void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
	Interval LocalValidity(TimeValue t);
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
	
	// NS: New SubObjType API
	int NumSubObjTypes();
	ISubObjType *GetSubObjType(int i);

	// From IMeshSelect
	DWORD GetSelLevel();
	void SetSelLevel(DWORD level);
	void LocalDataChanged();

	// IO
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);
	IOResult LoadNamedSelChunk(ILoad *iload,int level);
	IOResult SaveLocalData(ISave *isave, LocalModData *ld);
	IOResult LoadLocalData(ILoad *iload, LocalModData **pld);

	int NumParamBlocks () { return 1; }
	IParamBlock2 *GetParamBlock (int i) { return pblock; }
	IParamBlock2 *GetParamBlockByID (short id) { return (pblock->ID() == id) ? pblock : NULL; }

	int NumRefs() { return 1; }
	RefTargetHandle GetReference(int i) { return pblock; }
	void SetReference(int i, RefTargetHandle rtarg) { pblock = (IParamBlock2 *) rtarg; }

	int NumSubs() {return 1;}
	Animatable* SubAnim(int i) { return GetReference(i); }
	TSTR SubAnimName(int i) {return GetString (IDS_RB_PARAMETERS);}

	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
	   PartID& partID, RefMessage message);

	void UpdateSelLevelDisplay ();
	void SelectFrom(int from);
	void SetEnableStates();
	void SelectOpenEdges();
	void SelectByMatID(int id);
	void SetNumSelLabel();
	void UpdateCache(TimeValue t);
	void InvalidateVDistances ();
	void SetSoftSelEnables ();
	void InvalidateDialogElement (int elem);

	// Local methods for handling named selection sets
	int FindSet(TSTR &setName,int level);		
	DWORD AddSet(TSTR &setName,int level);
	void RemoveSet(TSTR &setName,int level);
	void UpdateSetNames ();	// Reconciles names with MeshSelData.
	void ClearSetNames();
	void NSCopy();
	void NSPaste();
	BOOL GetUniqueSetName(TSTR &name);
	int SelectNamedSet();
	void UpdateNamedSelDropDown ();
};

class MeshSelData : public LocalModData, public IMeshSelectData {
private:
	// Temp data used for soft selections, adjacent edge / face lists.
	MeshTempData *temp;
	Interval vdValid;	// validity interval of vertex distances from selection.

public:
	// LocalModData
	void* GetInterface(ULONG id) { if (id == I_MESHSELECTDATA) return(IMeshSelectData*)this; else return LocalModData::GetInterface(id); }

	// Selection sets
	BitArray vertSel;
	BitArray faceSel;
	BitArray edgeSel;

	// Lists of named selection sets
	GenericNamedSelSetList vselSet;
	GenericNamedSelSetList fselSet;
	GenericNamedSelSetList eselSet;

	BOOL held;
	Mesh *mesh;

	MeshSelData(Mesh &mesh);
	MeshSelData() { held=0; mesh=NULL; vdValid.SetEmpty(); temp=NULL; }
	~MeshSelData() { FreeCache(); }
	LocalModData *Clone();
	Mesh *GetMesh() {return mesh;}
	AdjEdgeList *GetAdjEdgeList ();
	AdjFaceList *GetAdjFaceList ();
	void SetCache(Mesh &mesh);
	void FreeCache();
	void SynchBitArrays();

	void SelVertByFace();
	void SelVertByEdge();
	void SelFaceByVert();
	void SelFaceByEdge();
	void SelPolygonByVert (float thresh, int igVis);
	void SelPolygonByEdge (float thresh, int igVis);
	void SelElementByVert ();
	void SelElementByEdge ();
	void SelEdgeByVert();
	void SelEdgeByFace();

	// From IMeshSelectData:
	BitArray GetVertSel() { return vertSel; }
	BitArray GetFaceSel() { return faceSel; }
	BitArray GetEdgeSel() { return edgeSel; }

	void SetVertSel(BitArray &set, IMeshSelect *imod, TimeValue t);
	void SetFaceSel(BitArray &set, IMeshSelect *imod, TimeValue t);
	void SetEdgeSel(BitArray &set, IMeshSelect *imod, TimeValue t);

	GenericNamedSelSetList & GetNamedVertSelList () { return vselSet; }
	GenericNamedSelSetList & GetNamedEdgeSelList () { return eselSet; }
	GenericNamedSelSetList & GetNamedFaceSelList () { return fselSet; }
	GenericNamedSelSetList & GetNamedSel (int nsl) {
		if (nsl==NS_VERTEX) return vselSet;
		if (nsl==NS_EDGE) return eselSet;
		return fselSet;
	}

	void InvalidateVDistances () { vdValid.SetEmpty (); }
	void GetWeightedVertSel (int nv, float *sel, TimeValue t, TriObject *tobj,
		float falloff, float pinch, float bubble, int edgeDist, bool ignoreBackfacing,
		Interval & eDistValid);
};

class SelectRestore : public RestoreObj {
public:
	BitArray usel, rsel;
	BitArray *sel;
	MeshSelMod *mod;
	MeshSelData *d;
	int level;

	SelectRestore(MeshSelMod *m, MeshSelData *d);
	SelectRestore(MeshSelMod *m, MeshSelData *d, int level);
	void Restore(int isUndo);
	void Redo();
	int Size() { return 1; }
	void EndHold() {d->held=FALSE;}
	TSTR Description() { return TSTR(_T("SelectRestore")); }
};

class AddSetRestore : public RestoreObj {
public:
	BitArray set;		
	DWORD id;
	TSTR name;
	GenericNamedSelSetList *setList;

	AddSetRestore (GenericNamedSelSetList *sl, DWORD i, TSTR & n) : setList(sl), id(i), name(n) { }
	void Restore(int isUndo) {
		set  = *setList->GetSet (id);
		setList->RemoveSet(id);
	}
	void Redo() { setList->InsertSet (set, id, name); }

	TSTR Description() {return TSTR(_T("Add NS Set"));}
};

class AddSetNameRestore : public RestoreObj {
public:		
	TSTR name;
	DWORD id;
	int index;	// location in list.

	MeshSelMod *et;
	Tab<TSTR*> *sets;
	Tab<DWORD> *ids;

	AddSetNameRestore (MeshSelMod *e, DWORD idd, Tab<TSTR*> *s,Tab<DWORD> *i) : et(e), sets(s), ids(i), id(idd) { }
	void Restore(int isUndo);
	void Redo();
	TSTR Description() {return TSTR(_T("Add Set Name"));}
};

void AddSetNameRestore::Restore(int isUndo) {
	int sct = sets->Count();
	for (index=0; index<sct; index++) if ((*ids)[index] == id) break;
	if (index >= sct) return;

	name = *(*sets)[index];
	delete (*sets)[index];
	sets->Delete (index, 1);
	ids->Delete (index, 1);
	if (et->ip) et->ip->NamedSelSetListChanged();
}

void AddSetNameRestore::Redo() {
	TSTR *nm = new TSTR(name);
	if (index < sets->Count()) {
		sets->Insert (index, 1, &nm);
		ids->Insert (index, 1, &id);
	} else {
		sets->Append (1, &nm);
		ids->Append (1, &id);
	}
	if (et->ip) et->ip->NamedSelSetListChanged();
}

class DeleteSetRestore : public RestoreObj {
public:
	BitArray set;
	DWORD id;
	TSTR name;
	GenericNamedSelSetList *setList;

	DeleteSetRestore(GenericNamedSelSetList *sl,DWORD i, TSTR & n) {
		setList = sl; 
		id = i;
		BitArray *ptr = setList->GetSet(id);
		if (ptr) set = *ptr;
		name = n;
	}   		
	void Restore(int isUndo) { setList->InsertSet(set,id,name); }
	void Redo() { setList->RemoveSet(id); }
	TSTR Description() {return TSTR(_T("Delete Set"));}
};

class DeleteSetNameRestore : public RestoreObj {
public:		
	TSTR name;
	DWORD id;
	MeshSelMod *et;
	Tab<TSTR*> *sets;
	Tab<DWORD> *ids;

	DeleteSetNameRestore(Tab<TSTR*> *s,MeshSelMod *e,Tab<DWORD> *i, DWORD id) {
		sets = s; et = e;
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
	DWORD id;
	Tab<TSTR*> *sets;
	Tab<DWORD> *ids;
	MeshSelMod *et;
	SetNameRestore(Tab<TSTR*> *s,MeshSelMod *e,Tab<DWORD> *i,DWORD id) {
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
#define SEL_FACE	3
#define SEL_POLY	4
#define SEL_ELEMENT 5

IObjParam *MeshSelMod::ip              = NULL;
MeshSelMod *MeshSelMod::editMod         = NULL;
SelectModBoxCMode *MeshSelMod::selectMode      = NULL;
BOOL MeshSelMod::updateCachePosted = FALSE;

class MeshSelClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new MeshSelMod; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_MESHSELMOD); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(MESHSELECT_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFDEFORMATIONS);}
	const TCHAR * InternalName () { return _T("MeshSelect"); }
	HINSTANCE HInstance () { return hInstance; }
};

static MeshSelClassDesc meshSelDesc;
ClassDesc* GetMeshSelModDesc() {return &meshSelDesc;}

class SelectProc : public ParamMap2UserDlgProc {
public:
	MeshSelMod *mod;
	SelectProc () { mod = NULL; }
	void SetEnables (HWND hWnd);
	void UpdateSelLevelDisplay (HWND hWnd);
	BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
	void DeleteThis() { }
};

static SelectProc theSelectProc;

class SoftSelProc : public ParamMap2UserDlgProc {
public:
	MeshSelMod *mod;
	SoftSelProc () { mod = NULL; }
	void SetEnables (HWND hWnd);
	void DrawCurve (HWND hWnd);
	BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
	void DeleteThis() { }
};

static SoftSelProc theSoftSelProc;

// Table to convert selLevel values to mesh selLevel flags.
const int meshLevel[] = {MESH_OBJECT, MESH_VERTEX, MESH_EDGE, 
MESH_FACE, MESH_FACE, MESH_FACE};

// Get display flags based on selLevel.
const DWORD levelDispFlags[] = {0,DISP_VERTTICKS|DISP_SELVERTS,
DISP_SELEDGES,DISP_SELFACES,DISP_SELPOLYS,DISP_SELPOLYS};

// For hit testing...
const int hitLevel[] = {0,SUBHIT_VERTS,SUBHIT_EDGES,SUBHIT_FACES,SUBHIT_FACES,SUBHIT_FACES};

// Parameter block IDs:
// Blocks themselves:
enum { ms_pblock };
// Parameter maps:
enum { ms_map_main, ms_map_softsel };
// Parameters in first block:
enum { ms_use_softsel, ms_use_edist, ms_edist, ms_affect_backfacing,
		ms_falloff, ms_pinch, ms_bubble, ms_by_vertex, ms_ignore_backfacing,
		ms_matid, ms_ignore_visible, ms_planar_threshold };

static ParamBlockDesc2 meshsel_softsel_desc ( ms_pblock,
									_T("meshSelectSoftSelection"),
									IDS_MS_SOFTSEL, &meshSelDesc,
									P_AUTO_CONSTRUCT | P_AUTO_UI | P_MULTIMAP,
									REF_PBLOCK,
	//rollout descriptions
	2,
	ms_map_main, IDD_MESH_SELECT, IDS_MS_PARAMS, 0, 0, NULL,
#ifndef RENDER_VER
	ms_map_softsel, IDD_MESHSEL_SOFTSEL, IDS_MS_SOFTSEL, 0, APPENDROLL_CLOSED, NULL,
#endif

	// params
	ms_use_softsel, _T("useSoftSelection"), TYPE_BOOL, P_RESET_DEFAULT, IDS_MS_USE_SS,
		p_default, FALSE,
		p_ui, ms_map_softsel, TYPE_SINGLECHEKBOX, IDC_MS_USE_SS,
		end,

	ms_use_edist, _T("softselUseEdgeDistance"), TYPE_BOOL, P_RESET_DEFAULT, IDS_USE_EDIST,
		p_default, FALSE, // Preserve selection
		p_ui, ms_map_softsel, TYPE_SINGLECHEKBOX, IDC_MS_USE_E_DIST,
		end,

	ms_edist, _T("softselEdgeDist"), TYPE_INT, P_ANIMATABLE|P_RESET_DEFAULT, IDS_EDGE_DIST,
		p_default, 1,
		p_range, 1, 9999999,
		p_ui, ms_map_softsel, TYPE_SPINNER, EDITTYPE_POS_INT, IDC_MS_E_DIST, IDC_MS_E_DISTSPIN, .2f,
		end,

	ms_affect_backfacing, _T("softselAffectBackfacing"), TYPE_BOOL, P_RESET_DEFAULT, IDS_AFFECT_BACKFACING,
		p_default, true,
		p_ui, ms_map_softsel, TYPE_SINGLECHEKBOX, IDC_MS_SS_BACKFACING,
		end,

	ms_falloff, _T("softselFalloff"), TYPE_WORLD, P_ANIMATABLE|P_RESET_DEFAULT, IDS_PW_FALLOFF,
		p_default, 20.0f,
		p_range, 0.0f, 999999.f,
		p_ui, ms_map_softsel, TYPE_SPINNER, EDITTYPE_POS_FLOAT,
			IDC_FALLOFF, IDC_FALLOFFSPIN, SPIN_AUTOSCALE,
		end,

	ms_pinch, _T("softselPinch"), TYPE_FLOAT, P_ANIMATABLE|P_RESET_DEFAULT, IDS_PW_PINCH,
		p_default, 0.0f,
		p_range, -999999.f, 999999.f,
		p_ui, ms_map_softsel, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_PINCH, IDC_PINCHSPIN, SPIN_AUTOSCALE,
		end,

	ms_bubble, _T("softselBubble"), TYPE_FLOAT, P_ANIMATABLE|P_RESET_DEFAULT, IDS_PW_BUBBLE,
		p_default, 0.0f,
		p_range, -999999.f, 999999.f,
		p_ui, ms_map_softsel, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_BUBBLE, IDC_BUBBLESPIN, SPIN_AUTOSCALE,
		end,

	ms_by_vertex, _T("byVertex"), TYPE_BOOL, P_RESET_DEFAULT, IDS_BY_VERTEX,
		p_default, false,
		p_ui, ms_map_main, TYPE_SINGLECHEKBOX, IDC_MS_SEL_BYVERT,
		end,

	ms_ignore_backfacing, _T("ignoreBackfacing"), TYPE_BOOL, P_RESET_DEFAULT, IDS_IGNORE_BACKFACING,
		p_default, false,
		p_ui, ms_map_main, TYPE_SINGLECHEKBOX, IDC_MS_IGNORE_BACKFACES,
		end,

	ms_matid, _T("materialID"), TYPE_INT, P_TRANSIENT|P_RESET_DEFAULT, IDS_RB_MATERIALID,
		p_default, 1,
		p_range, 1, 65535,
		p_ui, ms_map_main, TYPE_SPINNER, EDITTYPE_INT,
			IDC_MS_MATID, IDC_MS_MATIDSPIN, .5f,
		end,

	ms_ignore_visible, _T("ignoreVisibleEdges"), TYPE_BOOL, P_RESET_DEFAULT, IDS_IGNORE_VISIBLE,
		p_default, false,
		p_ui, ms_map_main, TYPE_SINGLECHEKBOX, IDC_MS_IGNORE_VISEDGE,
		end,

	ms_planar_threshold, _T("planarThreshold"), TYPE_ANGLE, P_RESET_DEFAULT, IDS_RB_THRESHOLD,
		p_default, PI/4.0f,	// Default value for angles has to be in radians.
		p_range, 0.0f, 180.0f,	// but range given in degrees.
		p_ui, ms_map_main, TYPE_SPINNER, EDITTYPE_POS_FLOAT,
			IDC_MS_PLANAR, IDC_MS_PLANARSPINNER, .1f,
		end,

	end
);

//--- MeshSel mod methods -------------------------------

MeshSelMod::MeshSelMod() {
	selLevel = SEL_OBJECT;
	pblock = NULL;
	meshSelDesc.MakeAutoParamBlocks (this);
}

RefTargetHandle MeshSelMod::Clone(RemapDir& remap) {
	MeshSelMod *mod = new MeshSelMod();
	mod->selLevel = selLevel;
	mod->ReplaceReference (0, pblock->Clone(remap));
	for (int i=0; i<3; i++)
	{
		mod->namedSel[i].SetCount (namedSel[i].Count());
		for (int j=0; j<namedSel[i].Count(); j++) mod->namedSel[i][j] = new TSTR (*namedSel[i][j]);
		mod->ids[i].SetCount (ids[i].Count());
		for (j=0; j<ids[i].Count(); j++) mod->ids[i][j] = ids[i][j];
	}
	BaseClone(this, mod, remap);
	return mod;
}

Interval MeshSelMod::LocalValidity(TimeValue t) 
{ 
  // aszabo|feb.05.02 If we are being edited, return NEVER 
	// to forces a cache to be built after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;
	return GetValidity(t); 
}

Interval MeshSelMod::GetValidity (TimeValue t) {
	Interval ret = FOREVER;
	int useAR;
	pblock->GetValue (ms_use_softsel, t, useAR, ret);
	if (!useAR) return ret;
	int useEDist, eDist;
	pblock->GetValue (ms_use_edist, t, useEDist, ret);
	if (useEDist) pblock->GetValue (ms_edist, t, eDist, ret);
	float f, p, b;
	pblock->GetValue (ms_falloff, t, f, ret);
	pblock->GetValue (ms_pinch, t, p, ret);
	pblock->GetValue (ms_bubble, t, b, ret);
	return ret;
}

void MeshSelData::GetWeightedVertSel (int nv, float *wvs, TimeValue t, TriObject *tobj,
									  float falloff, float pinch, float bubble, int edist, bool ignoreBackfacing,
									  Interval & edistValid) {
	if (!temp) temp = new MeshTempData;
	temp->SetMesh (&(tobj->GetMesh()));
	temp->InvalidateAffectRegion ();	// have to do, or it might remember last time's falloff, etc.

	if (!vdValid.InInterval (t)) {
		temp->InvalidateDistances ();
		vdValid = tobj->ChannelValidity (t,GEOM_CHAN_NUM);
		vdValid &= tobj->ChannelValidity (t,TOPO_CHAN_NUM);
		if (edist) vdValid &= edistValid;
	}

	Tab<float> *vwTable = temp->VSWeight (edist, edist, ignoreBackfacing,
		falloff, pinch, bubble);
	int min = (nv<vwTable->Count()) ? nv : vwTable->Count();
	if (min) memcpy (wvs, vwTable->Addr(0), min*sizeof(float));

	// zero out any accidental leftovers:
	for (int i=min; i<nv; i++) wvs[i] = 0.0f;
}

void MeshSelMod::ModifyObject (TimeValue t, ModContext &mc, ObjectState *os, INode *node) {
	if (!os->obj->IsSubClassOf(triObjectClassID)) return;
	TriObject *tobj = (TriObject*)os->obj;
	MeshSelData *d  = (MeshSelData*)mc.localData;
	if (!d) mc.localData = d = new MeshSelData(tobj->GetMesh());

	BitArray vertSel = d->vertSel;
	BitArray faceSel = d->faceSel;
	BitArray edgeSel = d->edgeSel;
	vertSel.SetSize(tobj->GetMesh().getNumVerts(),TRUE);
	faceSel.SetSize(tobj->GetMesh().getNumFaces(),TRUE);
	edgeSel.SetSize(tobj->GetMesh().getNumFaces()*3,TRUE);
	tobj->GetMesh().vertSel = vertSel;
	tobj->GetMesh().faceSel = faceSel;
	tobj->GetMesh().edgeSel = edgeSel;
	tobj->GetMesh().selLevel = meshLevel[selLevel];

	int useAR;
	Interval outValid;
	outValid = tobj->ChannelValidity (t, SELECT_CHAN_NUM);
	pblock->GetValue (ms_use_softsel, t, useAR, outValid);

	if (useAR) {
		float bubble, pinch, falloff;
		pblock->GetValue (ms_falloff, t, falloff, outValid);
		pblock->GetValue (ms_pinch, t, pinch, outValid);
		pblock->GetValue (ms_bubble, t, bubble, outValid);

		int useEDist, edist=0, affectBackfacing;
		Interval edistValid = FOREVER;
		pblock->GetValue (ms_use_edist, t, useEDist, edistValid);
		if (useEDist) {
			pblock->GetValue (ms_edist, t, edist, edistValid);
			outValid &= edistValid;
		}

		pblock->GetValue (ms_affect_backfacing, t, affectBackfacing, outValid);

		tobj->GetMesh().SupportVSelectionWeights ();
		float *vs = tobj->GetMesh().getVSelectionWeights ();
		int nv = tobj->GetMesh().getNumVerts();
		d->GetWeightedVertSel (nv, vs, t, tobj, falloff, pinch, bubble,
			edist, !affectBackfacing, edistValid);
	} else {
		tobj->GetMesh().ClearVSelectionWeights ();
	}

	// Update the cache used for display, hit-testing:
	if (!d->GetMesh()) d->SetCache(tobj->GetMesh());
	else *(d->GetMesh()) = tobj->GetMesh();

	// Set display flags - but be sure to turn off vertex display in stack result if
	// "Show End Result" is turned on - we want the user to just see the Mesh Select
	// level vertices (from the Display method).
	tobj->GetMesh().dispFlags = 0;
	if ((selLevel != SEL_VERTEX) || !ip || !ip->GetShowEndResult())
		tobj->GetMesh().SetDispFlag (levelDispFlags[selLevel]);
	tobj->SetChannelValidity (SELECT_CHAN_NUM, outValid);
}

void MeshSelMod::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc) {
	if (!mc->localData) return;
	if (partID == PART_SELECT) return;
	((MeshSelData*)mc->localData)->FreeCache();
	if (!ip || (editMod!=this) || updateCachePosted) return;

	if (!meshSelDesc.NumParamMaps ()) return;
	IParamMap2 *pmap = meshSelDesc.GetParamMap (ms_map_main);
	if (!pmap) return;
	HWND hWnd = pmap->GetHWnd();
	if (!hWnd) return;
	TimeValue t = ip->GetTime();
	PostMessage(hWnd,WM_UPDATE_CACHE,(WPARAM)t,0);
	updateCachePosted = TRUE;
}

void MeshSelMod::UpdateCache(TimeValue t) {
	NotifyDependents(Interval(t,t), PART_GEOM|SELECT_CHANNEL|PART_SUBSEL_TYPE|
		PART_DISPLAY|PART_TOPO, REFMSG_MOD_EVAL);
	updateCachePosted = FALSE;
}

class MSInvalidateDistanceEnumProc : public ModContextEnumProc {
public:
	MSInvalidateDistanceEnumProc () { }
	BOOL proc (ModContext *mc);
};

BOOL MSInvalidateDistanceEnumProc::proc (ModContext *mc) {
	if (!mc->localData) return true;
	MeshSelData *msd = (MeshSelData *) mc->localData;
	msd->InvalidateVDistances ();
	return true;
}

static MSInvalidateDistanceEnumProc theMSInvalidateDistanceEnumProc;

void MeshSelMod::InvalidateVDistances () {
	EnumModContexts (&theMSInvalidateDistanceEnumProc);
}

void MeshSelMod::UpdateSelLevelDisplay () {
	if (theSelectProc.mod != this) return;
	if (!meshSelDesc.NumParamMaps ()) return;
	IParamMap2 *pmap = meshSelDesc.GetParamMap (ms_map_main);
	if (!pmap) return;
	HWND hWnd = pmap->GetHWnd();
	if (!hWnd) return;
	theSelectProc.UpdateSelLevelDisplay (hWnd);
}

static int butIDs[] = { 0, IDC_SELVERTEX, IDC_SELEDGE, IDC_SELFACE, IDC_SELPOLY, IDC_SELELEMENT };
void SelectProc::UpdateSelLevelDisplay (HWND hWnd) {
	if (!mod) return;
	ICustToolbar *iToolbar = GetICustToolbar(GetDlgItem(hWnd,IDC_MS_SELTYPE));
	ICustButton *but;
	for (int i=1; i<6; i++) {
		but = iToolbar->GetICustButton (butIDs[i]);
		but->SetCheck ((DWORD)i==mod->selLevel);
		ReleaseICustButton (but);
	}
	ReleaseICustToolbar (iToolbar);
	UpdateWindow(hWnd);
}

static bool oldShowEnd;

void MeshSelMod::BeginEditParams (IObjParam  *ip, ULONG flags,Animatable *prev) {
	this->ip = ip;	
	editMod  = this;
	UpdateSetNames ();

	// Use our classdesc2 to put up our parammap2 maps:
	meshSelDesc.BeginEditParams (ip, this, flags, prev);
	theSelectProc.mod = this;
	meshSelDesc.SetUserDlgProc (&meshsel_softsel_desc, ms_map_main, &theSelectProc);
	theSoftSelProc.mod = this;
	meshSelDesc.SetUserDlgProc (&meshsel_softsel_desc, ms_map_softsel, &theSoftSelProc);

	selectMode = new SelectModBoxCMode(this,ip);

	// Add our sub object types
	// TSTR type1(GetString(IDS_RB_VERTEX));
	// TSTR type2(GetString(IDS_RB_EDGE));
	// TSTR type3(GetString(IDS_RB_FACE));
	// TSTR type4(GetString(IDS_EM_POLY));
	// TSTR type5(GetString(IDS_EM_ELEMENT));
	// const TCHAR *ptype[] = {type1, type2, type3, type4, type5};
	// This call is obsolete. Please see BaseObject::NumSubObjTypes() and BaseObject::GetSubObjType()
	// ip->RegisterSubObjectTypes(ptype, 5);

	// Restore the selection level.
	ip->SetSubObjectLevel(selLevel);

	SetEnableStates ();
	UpdateSelLevelDisplay ();
	SetNumSelLabel();

	// Set show end result.
	oldShowEnd = ip->GetShowEndResult() ? TRUE : FALSE;
	ip->SetShowEndResult (GetFlag (MS_DISP_END_RESULT));

	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);	
}

void MeshSelMod::EndEditParams (IObjParam *ip,ULONG flags,Animatable *next) {
	meshSelDesc.EndEditParams (ip, this, flags, next);
	theSelectProc.mod = NULL;
	theSoftSelProc.mod = NULL;

	ip->DeleteMode(selectMode);
	if (selectMode) delete selectMode;
	selectMode = NULL;

	// Reset show end result
	SetFlag (MS_DISP_END_RESULT, ip->GetShowEndResult() ? TRUE : FALSE);
	ip->SetShowEndResult(oldShowEnd);

	this->ip = NULL;
	editMod  = NULL;

	TimeValue t = ip->GetTime();
	// aszabo|feb.05.02 This flag must be cleared before sending the REFMSG_END_EDIT
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);
}

int MeshSelMod::HitTest (TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc) {
	Interval valid;
	int savedLimits, res = 0;
	GraphicsWindow *gw = vpt->getGW();
	HitRegion hr;
	
	int selByVert, ignoreBackfaces;
	pblock->GetValue (ms_by_vertex, t, selByVert, FOREVER);
	pblock->GetValue (ms_ignore_backfacing, t, ignoreBackfaces, FOREVER);

	// Setup GW
	MakeHitRegion(hr,type, crossing,4,p);
	gw->setHitRegion(&hr);
	Matrix3 mat = inode->GetObjectTM(t);
	gw->setTransform(mat);	
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
	if (ignoreBackfaces) gw->setRndLimits(gw->getRndLimits() |  GW_BACKCULL);
	else gw->setRndLimits(gw->getRndLimits() & ~GW_BACKCULL);
	gw->clearHitCode();
		
	SubObjHitList hitList;
	MeshSubHitRec *rec;	

	if (!mc->localData || !((MeshSelData*)mc->localData)->GetMesh()) return 0;

	DWORD hitFlags;
	if (selByVert) {
		hitFlags = SUBHIT_VERTS;
		if (selLevel > SEL_VERTEX) hitFlags |= SUBHIT_USEFACESEL;
	} else {
		hitFlags = hitLevel[selLevel];
	}

	Mesh &mesh = *((MeshSelData*)mc->localData)->GetMesh();

	// cache backfacing vertices as hidden:
	BitArray oldHide;
	if ((hitFlags & SUBHIT_VERTS) && ignoreBackfaces) {
		BOOL flip = mat.Parity();
		oldHide = mesh.vertHide;
		BitArray faceBack;
		faceBack.SetSize (mesh.getNumFaces());
		faceBack.ClearAll ();
		for (int i=0; i<mesh.getNumFaces(); i++) {
			DWORD *vv = mesh.faces[i].v;
			IPoint3 A[3];
			for (int j=0; j<3; j++) gw->wTransPoint (&(mesh.verts[vv[j]]), &(A[j]));
			IPoint3 d1 = A[1] - A[0];
			IPoint3 d2 = A[2] - A[0];
			if (flip) {
				if ((d1^d2).z > 0) continue;
			} else {
				if ((d1^d2).z < 0) continue;
			}
			for (j=0; j<3; j++) mesh.vertHide.Set (vv[j]);
			faceBack.Set (i);
		}
		for (i=0; i<mesh.getNumFaces(); i++) {
			if (faceBack[i]) continue;
			DWORD *vv = mesh.faces[i].v;
			for (int j=0; j<3; j++) mesh.vertHide.Clear (vv[j]);
		}
		mesh.vertHide |= oldHide;
	}

	res = mesh.SubObjectHitTest(gw, gw->getMaterial(), &hr, flags|hitFlags, hitList);

	if ((hitFlags & SUBHIT_VERTS) && ignoreBackfaces) mesh.vertHide = oldHide;

	rec = hitList.First();
	while (rec) {
		vpt->LogHit(inode,mc,rec->dist,rec->index,NULL);
		rec = rec->Next();
	}

	gw->setRndLimits(savedLimits);	
	return res;	
}

int MeshSelMod::Display (TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc) {
	if (!ip->GetShowEndResult ()) return 0;
	if (!selLevel) return 0;
	if (!mc->localData) return 0;

	MeshSelData *modData = (MeshSelData *) mc->localData;
	Mesh *mesh = modData->GetMesh();
	if (!mesh) return 0;

	// Set up GW
	GraphicsWindow *gw = vpt->getGW();
	Matrix3 tm = inode->GetObjectTM(t);
	int savedLimits;
	gw->setRndLimits((savedLimits = gw->getRndLimits()) & ~GW_ILLUM);
	gw->setTransform(tm);

	// We need to draw a "gizmo" version of the mesh:
	Point3 colSel=GetSubSelColor();
	Point3 colTicks=GetUIColor (COLOR_VERT_TICKS);
	Point3 colGiz=GetUIColor(COLOR_GIZMOS);
	Point3 colGizSel=GetUIColor(COLOR_SEL_GIZMOS);
	gw->setColor (LINE_COLOR, colGiz);

	AdjEdgeList *ae = modData->GetAdjEdgeList ();
	Point3 rp[3];
	int i, ect = ae->edges.Count();

#ifdef MESH_CAGE_BACKFACE_CULLING
	// Figure out backfacing from frontfacing.
	BitArray fBackfacing, vBackfacing;
	bool backCull = (savedLimits & GW_BACKCULL) ? true : false;
	if (backCull)
	{
		fBackfacing.SetSize (mesh->numFaces);
		vBackfacing.SetSize (mesh->numVerts);
		vBackfacing.SetAll ();
		BitArray nonIsoVerts;
		nonIsoVerts.SetSize (mesh->numVerts);

		mesh->checkNormals (false);	// Allocates rVerts.
		RVertex *rv = mesh->getRVertPtr (0);
		BOOL gwFlipped = gw->getFlipped();

		for (i=0; i <mesh->numVerts; i++) {
			rv[i].rFlags = (rv[i].rFlags & ~(GW_PLANE_MASK | RND_MASK | RECT_MASK)) |
				gw->hTransPoint(&(mesh->verts[i]), (IPoint3 *)rv[i].pos);
		}
		for (i=0; i<mesh->numFaces; i++)
		{
			Face & f = mesh->faces[i];
			fBackfacing.Set (i, hIsFacingBack (rv[f.v[0]].pos, rv[f.v[1]].pos, rv[f.v[2]].pos, gwFlipped));
			for (int j=0; j<3; j++) nonIsoVerts.Set (f.v[j]);
			if (fBackfacing[i]) continue;
			for (j=0; j<3; j++) vBackfacing.Clear (f.v[j]);
		}
		vBackfacing &= nonIsoVerts;	// so isolated vertices aren't labeled as backfacing.
	}
#endif

	int es[3];
	for (i=0; i<ect; i++) {
		MEdge & me = ae->edges[i];
		if (me.Hidden (mesh->faces)) continue;
		if (me.Visible (mesh->faces)) {
			es[0] = GW_EDGE_VIS;
		} else {
			if (selLevel < SEL_EDGE) continue;
			if (selLevel > SEL_FACE) continue;
			es[0] = GW_EDGE_INVIS;
		}

#ifdef MESH_CAGE_BACKFACE_CULLING
		if (backCull && fBackfacing[me.f[0]] && ((me.f[1] == UNDEFINED) || fBackfacing[me.f[1]]))
			continue;
#endif

		if (selLevel == SEL_EDGE) {
			if (ae->edges[i].Selected (mesh->faces, modData->GetEdgeSel())) gw->setColor (LINE_COLOR, colGizSel);
			else gw->setColor (LINE_COLOR, colGiz);
		}
		if (selLevel >= SEL_FACE) {
			if (ae->edges[i].AFaceSelected (modData->GetFaceSel())) gw->setColor (LINE_COLOR, colGizSel);
			else gw->setColor (LINE_COLOR, colGiz);
		}
		rp[0] = mesh->verts[me.v[0]];
		rp[1] = mesh->verts[me.v[1]];
		gw->polyline (2, rp, NULL, NULL, FALSE, es);
	}
	if (selLevel == SEL_VERTEX) {
		float *ourvw = NULL;
		int affectRegion=FALSE;
		if (pblock) pblock->GetValue (ms_use_softsel, t, affectRegion, FOREVER);
		if (affectRegion) ourvw = mesh->getVSelectionWeights ();
		for (i=0; i<mesh->numVerts; i++) {
			if (mesh->vertHide[i]) continue;
#ifdef MESH_CAGE_BACKFACE_CULLING
			if (backCull && vBackfacing[i]) continue;
#endif

			if (modData->GetVertSel()[i]) gw->setColor (LINE_COLOR, colSel);
			else {
				if (ourvw) gw->setColor (LINE_COLOR, SoftSelectionColor (ourvw[i]));
				else gw->setColor (LINE_COLOR, colTicks);
			}

			if(getUseVertexDots()) gw->marker (&(mesh->verts[i]), VERTEX_DOT_MARKER(getVertexDotType()));
			else gw->marker (&(mesh->verts[i]), PLUS_SIGN_MRKR);
		}
	}
	gw->setRndLimits(savedLimits);
	return 0;	
}

void MeshSelMod::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp *vpt, Box3& box, ModContext *mc) {
	if (!ip->GetShowEndResult() || !mc->localData) return;
	if (!selLevel) return;
	MeshSelData *modData = (MeshSelData *) mc->localData;
	Mesh *mesh = modData->GetMesh();
	if (!mesh) return;
	Matrix3 tm = inode->GetObjectTM(t);
	box = mesh->getBoundingBox (&tm);
}

void MeshSelMod::GetSubObjectCenters (SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc) {
	if (!mc->localData) return;
	if (selLevel == SEL_OBJECT) return;	// shouldn't happen.
	MeshSelData *modData = (MeshSelData *) mc->localData;
	Mesh *mesh = modData->GetMesh();
	if (!mesh) return;
	Matrix3 tm = node->GetObjectTM(t);

	// For Mesh Select, we merely return the center of the bounding box of the current selection.
	BitArray sel = mesh->VertexTempSel ();
	if (!sel.NumberSet()) return;
	Box3 box;
	for (int i=0; i<mesh->numVerts; i++) if (sel[i]) box += mesh->verts[i] * tm;
	cb->Center (box.Center(),0);
}

void MeshSelMod::GetSubObjectTMs (SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc) {
	if (!mc->localData) return;
	if (selLevel == SEL_OBJECT) return;	// shouldn't happen.
	MeshSelData *modData = (MeshSelData *) mc->localData;
	Mesh *mesh = modData->GetMesh();
	if (!mesh) return;
	Matrix3 tm = node->GetObjectTM(t);

	// For Mesh Select, we merely return the center of the bounding box of the current selection.
	BitArray sel = mesh->VertexTempSel ();
	if (!sel.NumberSet()) return;
	Box3 box;
	for (int i=0; i<mesh->numVerts; i++) if (sel[i]) box += mesh->verts[i] * tm;
	Matrix3 ctm(1);
	ctm.SetTrans (box.Center());
	cb->TM (ctm,0);
}

void MeshSelMod::ActivateSubobjSel(int level, XFormModes& modes) {
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
	SetSoftSelEnables ();

	// Setup named selection sets	
	SetupNamedSelDropDown();

	// Invalidate the weighted vertex caches
	InvalidateVDistances ();

	NotifyDependents(FOREVER, PART_SUBSEL_TYPE|PART_DISPLAY, REFMSG_CHANGE);
	ip->PipeSelLevelChanged();
	NotifyDependents(FOREVER, SELECT_CHANNEL|DISP_ATTRIB_CHANNEL|SUBSEL_TYPE_CHANNEL, REFMSG_CHANGE);
}

DWORD MeshSelMod::GetSelLevel () {
	switch (selLevel) {
	case SEL_OBJECT: return IMESHSEL_OBJECT;
	case SEL_VERTEX: return IMESHSEL_VERTEX;
	case SEL_EDGE:
		return IMESHSEL_EDGE;
	}
	return IMESHSEL_FACE;
}

void MeshSelMod::SetSelLevel(DWORD sl) {
	switch (sl) {
	case IMESHSEL_OBJECT:
		selLevel = SEL_OBJECT;
		break;
	case IMESHSEL_VERTEX:
		selLevel = SEL_VERTEX;
		break;
	case IMESHSEL_EDGE:
		selLevel = SEL_EDGE;
		break;
	case IMESHSEL_FACE:
		// Don't change if we're already in a face level:
		if (GetSelLevel()==IMESHSEL_FACE) break;
		selLevel = SEL_POLY;
		break;
	}
	if (ip && editMod==this) ip->SetSubObjectLevel(selLevel);
}

void MeshSelMod::LocalDataChanged() {
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	if (ip && editMod==this) {
		SetNumSelLabel();
		UpdateNamedSelDropDown ();
	}
	InvalidateVDistances ();
}

void MeshSelMod::UpdateNamedSelDropDown () {
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
		MeshSelData *d = (MeshSelData *) mcList[nd]->localData;
		if (!d) continue;
		foundone = TRUE;
		switch (selLevel) {
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
		default:
			for (i=0; i<nselmatch.GetSize(); i++) {
				if (!nselmatch[i]) continue;
				if (!(*(d->fselSet.sets[i]) == d->faceSel)) nselmatch.Clear(i);
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

void MeshSelMod::SelectSubComponent (HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert) {
	MeshSelData *d = NULL, *od = NULL;

	ip->ClearCurNamedSelSet();

	int selByVert, ignoreVisEdge;
	float planarThresh;
	TimeValue t = ip->GetTime();
	pblock->GetValue (ms_by_vertex, t, selByVert, FOREVER);
	pblock->GetValue (ms_ignore_visible, t, ignoreVisEdge, FOREVER);
	pblock->GetValue (ms_planar_threshold, t, planarThresh, FOREVER);

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	int nd;
	BitArray nsel;
	AdjEdgeList *ae = NULL;
	AdjFaceList *af = NULL;
	Mesh *mesh;

	for (nd=0; nd<mcList.Count(); nd++) {
		d = (MeshSelData*) mcList[nd]->localData;
		if (d==NULL) continue;
		HitRecord *hr = hitRec;
		if (!all && (hr->modContext->localData != d)) continue;
		for (; hr!=NULL; hr=hr->Next()) if (hr->modContext->localData == d) break;
		if (hr==NULL) continue;

		mesh = d->GetMesh();
		if (!mesh) continue;
		BaseInterface *msci = mesh->GetInterface (MESHSELECTCONVERT_INTERFACE);

		if (selByVert) {
			ae = d->GetAdjEdgeList ();
			if (!ae) continue;
		}

		switch (selLevel) {
		case SEL_VERTEX:
			nsel = d->vertSel;
			for (; hr != NULL; hr = hr->Next()) {
				if (d != hr->modContext->localData) continue;
				nsel.Set (hr->hitInfo, invert ? !nsel[hr->hitInfo] : selected);
				if (!all) break;
			}
			d->SetVertSel (nsel, this, t);
			break;

		case SEL_EDGE:
			if (msci && selByVert) {
				// Use new improved selection conversion:
				BitArray vhit;
				vhit.SetSize(mesh->numVerts);
				for (hr=hitRec; hr!=NULL; hr=hr->Next()) {
					if (d != hr->modContext->localData) continue;
					vhit.Set (hr->hitInfo);
					if (!all) break;
				}
				MeshSelectionConverter *pConv = static_cast<MeshSelectionConverter*>(msci);
				pConv->VertexToEdge(*mesh, vhit, nsel);
				if (invert) nsel ^= d->edgeSel;
				else {
					if (selected) nsel |= d->edgeSel;
					else nsel = d->edgeSel & ~nsel;
				}
			} else {
				nsel = d->edgeSel;
				for (; hr != NULL; hr=hr->Next()) {
					if (d != hr->modContext->localData) continue;
					if (selByVert) {
						DWORDTab & list = ae->list[hr->hitInfo];
						for (int i=0; i<list.Count(); i++) {
							MEdge & me = ae->edges[list[i]];
							for (int j=0; j<2; j++) {
								if (me.f[j] == UNDEFINED) continue;
								DWORD ei = mesh->faces[me.f[j]].GetEdgeIndex (me.v[0], me.v[1]);
								if (ei>2) continue;
								ei += me.f[j]*3;
								nsel.Set (ei, invert ? !d->edgeSel[ei] : selected);
							}
						}
					} else {
						nsel.Set (hr->hitInfo, invert ? !d->edgeSel[hr->hitInfo] : selected);
					}
					if (!all) break;
				}
			}
			d->SetEdgeSel (nsel, this, t);
			break;

		case SEL_FACE:
			if (msci && selByVert) {
				// Use new improved selection conversion:
				BitArray vhit;
				vhit.SetSize(mesh->numVerts);
				for (hr=hitRec; hr!=NULL; hr=hr->Next()) {
					if (d != hr->modContext->localData) continue;
					vhit.Set (hr->hitInfo);
					if (!all) break;
				}
				MeshSelectionConverter *pConv = static_cast<MeshSelectionConverter*>(msci);
				pConv->VertexToFace (*mesh, vhit, nsel);
				if (invert) nsel ^= d->faceSel;
				else {
					if (selected) nsel |= d->faceSel;
					else nsel = d->faceSel & ~nsel;
				}
			} else {
				nsel = d->faceSel;
				for (; hr != NULL; hr=hr->Next()) {
					if (d != hr->modContext->localData) continue;
					if (selByVert) {
						DWORDTab & list = ae->list[hr->hitInfo];
						for (int i=0; i<list.Count(); i++) {
							MEdge & me = ae->edges[list[i]];
							for (int j=0; j<2; j++) {
								if (me.f[j] == UNDEFINED) continue;
								nsel.Set (me.f[j], invert ? !d->faceSel[me.f[j]] : selected);
							}
						}
					} else {
						nsel.Set (hr->hitInfo, invert ? !d->faceSel[hr->hitInfo] : selected);
					}
					if (!all) break;
				}
			}
			d->SetFaceSel (nsel, this, t);
			break;

		case SEL_POLY:
		case SEL_ELEMENT:
			af = d->GetAdjFaceList ();
			if (msci) {
				// Use new improved selection conversion:
				MeshSelectionConverter *pConv = static_cast<MeshSelectionConverter*>(msci);
				if (selByVert) {
					BitArray vhit;
					vhit.SetSize(mesh->numVerts);
					for (hr=hitRec; hr!=NULL; hr=hr->Next()) {
						if (d != hr->modContext->localData) continue;
						vhit.Set (hr->hitInfo);
						if (!all) break;
					}
					if (selLevel == SEL_ELEMENT) pConv->VertexToElement(*mesh, af, vhit, nsel);
					else pConv->VertexToPolygon(*mesh, af, vhit, nsel, planarThresh, ignoreVisEdge?true:false);
				} else {
					BitArray fhit;
					fhit.SetSize(mesh->numFaces);
					for (hr=hitRec; hr!=NULL; hr=hr->Next()) {
						if (d != hr->modContext->localData) continue;
						fhit.Set (hr->hitInfo);
						if (!all) break;
					}
					if (selLevel == SEL_ELEMENT) pConv->FaceToElement (*mesh, af, fhit, nsel);
					else pConv->FaceToPolygon (*mesh, af, fhit, nsel, planarThresh, ignoreVisEdge?true:false);
				}
			} else {
				// Otherwise we'll take the old approach of converting faces to polygons or elements as we go.
				nsel.SetSize (mesh->numFaces);
				nsel.ClearAll ();
				for (; hr != NULL; hr=hr->Next()) {
					if (d != hr->modContext->localData) continue;
					if (!selByVert) {
						if (selLevel==SEL_ELEMENT)
							mesh->ElementFromFace (hr->hitInfo, nsel, af);
						else
							mesh->PolyFromFace (hr->hitInfo, nsel, planarThresh, ignoreVisEdge, af);
					} else {
						DWORDTab & list = ae->list[hr->hitInfo];
						for (int i=0; i<list.Count(); i++) {
							MEdge & me = ae->edges[list[i]];
							for (int j=0; j<2; j++) {
								if (me.f[j] == UNDEFINED) continue;
								if (selLevel==SEL_ELEMENT)
									mesh->ElementFromFace (me.f[j], nsel, af);
								else
									mesh->PolyFromFace (me.f[j], nsel, planarThresh, ignoreVisEdge, af);
							}
						}
					}
					if (!all) break;
				}
			}

			if (invert) nsel ^= d->faceSel;
			else if (selected) nsel |= d->faceSel;
			else nsel = d->faceSel & ~nsel;

			d->SetFaceSel (nsel, this, t);
			break;
		}
	}

	nodes.DisposeTemporary ();
	LocalDataChanged ();
}

void MeshSelMod::ClearSelection(int selLevel) {
	ModContextList list;
	INodeTab nodes;	
	ip->GetModContexts(list,nodes);
	MeshSelData *d;
	for (int i=0; i<list.Count(); i++) {
		d = (MeshSelData*)list[i]->localData;
		if (!d) continue;

		// Check if we have anything selected first:
		switch (selLevel) {
		case SEL_VERTEX:
			if (!d->vertSel.NumberSet()) continue;
			else break;
		case SEL_FACE:
		case SEL_POLY:
		case SEL_ELEMENT:
			if (!d->faceSel.NumberSet()) continue;
			else break;
		case SEL_EDGE:
			if (!d->edgeSel.NumberSet()) continue;
			else break;
		}

		if (theHold.Holding() && !d->held) theHold.Put (new SelectRestore (this, d));
		d->SynchBitArrays ();
		switch (selLevel) {
		case SEL_VERTEX:
			d->vertSel.ClearAll ();
			break;
		case SEL_FACE:
		case SEL_POLY:
		case SEL_ELEMENT:
			d->faceSel.ClearAll();
			break;
		case SEL_EDGE:
			d->edgeSel.ClearAll ();
			break;
		}
	}
	nodes.DisposeTemporary();
	LocalDataChanged ();
}

void MeshSelMod::SelectAll(int selLevel) {
	ModContextList list;
	INodeTab nodes;	
	ip->GetModContexts(list,nodes);
	MeshSelData *d;
	for (int i=0; i<list.Count(); i++) {
		d = (MeshSelData*)list[i]->localData;		
		if (!d) continue;
		if (theHold.Holding() && !d->held) theHold.Put(new SelectRestore(this,d));
		d->SynchBitArrays();
		switch (selLevel) {
		case SEL_VERTEX:
			d->vertSel.SetAll ();
			break;
		case SEL_FACE:
		case SEL_POLY:
		case SEL_ELEMENT:
			d->faceSel.SetAll ();
			break;
		case SEL_EDGE:
			d->edgeSel.SetAll ();
			break;
		}
	}
	nodes.DisposeTemporary();
	LocalDataChanged ();
}

void MeshSelMod::InvertSelection(int selLevel) {
	ModContextList list;
	INodeTab nodes;	
	ip->GetModContexts(list,nodes);
	MeshSelData *d;
	for (int i=0; i<list.Count(); i++) {
		d = (MeshSelData*)list[i]->localData;
		if (!d) continue;
		if (theHold.Holding() && !d->held) theHold.Put(new SelectRestore(this,d));
		d->SynchBitArrays();
		switch (selLevel) {
		case SEL_VERTEX:
			d->vertSel = ~d->vertSel;
			break;
		case SEL_FACE:
		case SEL_POLY:
		case SEL_ELEMENT:
			d->faceSel = ~d->faceSel;
			break;
		case SEL_EDGE:
			d->edgeSel = ~d->edgeSel;
			break;
		}
	}
	nodes.DisposeTemporary();
	LocalDataChanged ();
}

void MeshSelMod::SelectByMatID(int id) {
	BOOL add = GetKeyState(VK_CONTROL)<0 ? TRUE : FALSE;
	BOOL sub = GetKeyState(VK_MENU)<0 ? TRUE : FALSE;
	theHold.Begin();
	ModContextList list;
	INodeTab nodes;	
	ip->GetModContexts(list,nodes);
	MeshSelData *d;
	for (int i=0; i<list.Count(); i++) {
		d = (MeshSelData*)list[i]->localData;		
		if (!d) continue;
		if (!d->held) theHold.Put(new SelectRestore(this,d));
		d->SynchBitArrays();
		if (!add && !sub) d->faceSel.ClearAll();
		Mesh *mesh = d->GetMesh();
		if (!mesh) continue;
		for (int i=0; i<mesh->numFaces; i++) {
			if (mesh->faces[i].getMatID()==(MtlID)id) {
				if (sub) d->faceSel.Clear(i);
				else d->faceSel.Set(i);
			}
		}
	}
	nodes.DisposeTemporary();
	theHold.Accept(GetString (IDS_RB_SELECTBYMATID));

	LocalDataChanged ();
	ip->RedrawViews(ip->GetTime());
}

void MeshSelMod::SelectOpenEdges() {
	theHold.Begin();
	ModContextList list;
	INodeTab nodes;
	ip->GetModContexts(list,nodes);
	MeshSelData *d;
	for (int i=0; i<list.Count(); i++) {
		d = (MeshSelData*)list[i]->localData;		
		if (!d) continue;
		if (!d->mesh) continue;

		if (!d->held) theHold.Put(new SelectRestore(this,d));
		d->SynchBitArrays();

		d->edgeSel.ClearAll ();
		AdjEdgeList *ae = d->GetAdjEdgeList ();
		if (!ae) continue;
		for (int j=0; j<ae->edges.Count(); j++) {
			MEdge *me = &(ae->edges[j]);
			if (me->f[0]==UNDEFINED) {
				int wh = d->mesh->faces[me->f[1]].GetEdgeIndex (me->v[0], me->v[1]);
				d->edgeSel.Set (me->f[1]*3+wh);
			}
			if (me->f[1]==UNDEFINED) {
				int wh = d->mesh->faces[me->f[0]].GetEdgeIndex (me->v[0], me->v[1]);
				d->edgeSel.Set (me->f[0]*3+wh);
			}
		}
	}
	nodes.DisposeTemporary();
	theHold.Accept(GetString (IDS_EM_SELECT_OPEN));
	LocalDataChanged ();
	ip->RedrawViews(ip->GetTime());
}

void MeshSelMod::SelectFrom(int from) {
	ModContextList list;
	INodeTab nodes;
	ip->GetModContexts(list,nodes);
	MeshSelData *d;

	int ignoreVisEdge;
	float planarThresh;
	pblock->GetValue (ms_ignore_visible, TimeValue(0), ignoreVisEdge, FOREVER);
	pblock->GetValue (ms_planar_threshold, TimeValue(0), planarThresh, FOREVER);

	theHold.Begin();
	for (int i=0; i<list.Count(); i++) {
		d = (MeshSelData*)list[i]->localData;
		if (!d) continue;

		if (!d->held) theHold.Put(new SelectRestore(this,d));
		d->SynchBitArrays();

		switch (selLevel) {
		case SEL_VERTEX: 
			if (from==SEL_FACE) d->SelVertByFace();
			else d->SelVertByEdge();
			break;
		case SEL_FACE:
			if (from==SEL_VERTEX) d->SelFaceByVert();
			else d->SelFaceByEdge();
			break;
		case SEL_POLY:
			if (from==SEL_VERTEX) d->SelPolygonByVert(planarThresh, ignoreVisEdge);
			else d->SelPolygonByEdge(planarThresh, ignoreVisEdge);
			break;
		case SEL_ELEMENT:
			if (from==SEL_VERTEX) d->SelElementByVert();
			else d->SelElementByEdge();
			break;
		case SEL_EDGE:
			if (from==SEL_VERTEX) d->SelEdgeByVert();
			else d->SelEdgeByFace();
			break;
		}
	}
	theHold.Accept(GetString(IDS_DS_SELECT));
	nodes.DisposeTemporary();
	LocalDataChanged ();
	ip->RedrawViews(ip->GetTime());
}

// old-style parameter block for loading old scenes:
static ParamBlockDescID descVer0[] = {
	{ TYPE_INT, NULL, TRUE, ms_use_softsel },
	{ TYPE_FLOAT, NULL, TRUE, ms_falloff },
	{ TYPE_FLOAT, NULL, TRUE, ms_pinch },
	{ TYPE_FLOAT, NULL, TRUE, ms_bubble }
};
#define PBLOCK_LENGTH	4

static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,4,0)
};
#define NUM_OLDVERSIONS	1

#define SELLEVEL_CHUNKID		0x0100
#define FLAGS_CHUNKID			0x0240
#define VERTSEL_CHUNKID			0x0200
#define FACESEL_CHUNKID			0x0210
#define EDGESEL_CHUNKID			0x0220
#define VERSION_CHUNKID			0x0230
static int currentVersion = 4;

#define NAMEDVSEL_NAMES_CHUNK	0x2805
#define NAMEDFSEL_NAMES_CHUNK	0x2806
#define NAMEDESEL_NAMES_CHUNK	0x2807
#define NAMEDSEL_STRING_CHUNK	0x2809
#define NAMEDSEL_ID_CHUNK		0x2810

#define VSELSET_CHUNK			0x2845
#define FSELSET_CHUNK			0x2846
#define ESELSET_CHUNK			0x2847

static int namedSelID[] = {NAMEDVSEL_NAMES_CHUNK,NAMEDESEL_NAMES_CHUNK,NAMEDFSEL_NAMES_CHUNK, NAMEDFSEL_NAMES_CHUNK, NAMEDFSEL_NAMES_CHUNK};

IOResult MeshSelMod::Save(ISave *isave) {
	IOResult res;
	ULONG nb;
	Modifier::Save(isave);

	isave->BeginChunk(SELLEVEL_CHUNKID);
	res = isave->Write(&selLevel, sizeof(selLevel), &nb);
	isave->EndChunk();

	isave->BeginChunk (VERSION_CHUNKID);
	res = isave->Write (&currentVersion, sizeof(int), &nb);
	isave->EndChunk();

	DWORD flags = ExportFlags ();
	isave->BeginChunk (FLAGS_CHUNKID);
	res = isave->Write (&flags, sizeof(DWORD), &nb);
	isave->EndChunk();

	for (int j=0; j<3; j++) {
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

	return res;
}

IOResult MeshSelMod::LoadNamedSelChunk(ILoad *iload,int level) {	
	IOResult res;
	DWORD ix=0;
	ULONG nb;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
		case NAMEDSEL_STRING_CHUNK: {
			TCHAR *name;
			res = iload->ReadWStringChunk(&name);
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

IOResult MeshSelMod::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	int version = 2;
	DWORD flags;

	Modifier::Load(iload);

	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
		case SELLEVEL_CHUNKID:
			iload->Read(&selLevel, sizeof(selLevel), &nb);
			break;

		case VERSION_CHUNKID:
			iload->Read (&version, sizeof(version), &nb);
			break;

		case FLAGS_CHUNKID:
			iload->Read (&flags, sizeof(DWORD), &nb);
			ImportFlags (flags);
			break;

		case NAMEDVSEL_NAMES_CHUNK:
			res = LoadNamedSelChunk(iload,0);
			break;

		case NAMEDESEL_NAMES_CHUNK:
			res = LoadNamedSelChunk(iload,1);
			break;

		case NAMEDFSEL_NAMES_CHUNK:
			res = LoadNamedSelChunk(iload,2);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	if (version<3) {
		if ((selLevel>1) && (selLevel<4)) selLevel = 5-selLevel;	// switched faces, edges in 3.0
	}
	iload->RegisterPostLoadCallback (
		new ParamBlock2PLCB (versions, NUM_OLDVERSIONS, &meshsel_softsel_desc, this, REF_PBLOCK));
	return IO_OK;
}

IOResult MeshSelMod::SaveLocalData(ISave *isave, LocalModData *ld) {	
	MeshSelData *d = (MeshSelData*)ld;

	isave->BeginChunk(VERTSEL_CHUNKID);
	d->vertSel.Save(isave);
	isave->EndChunk();

	isave->BeginChunk(FACESEL_CHUNKID);
	d->faceSel.Save(isave);
	isave->EndChunk();

	isave->BeginChunk(EDGESEL_CHUNKID);
	d->edgeSel.Save(isave);
	isave->EndChunk();
	
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
	if (d->fselSet.Count()) {
		isave->BeginChunk(FSELSET_CHUNK);
		d->fselSet.Save(isave);
		isave->EndChunk();
	}

	return IO_OK;
}

IOResult MeshSelMod::LoadLocalData(ILoad *iload, LocalModData **pld) {
	MeshSelData *d = new MeshSelData;
	*pld = d;
	IOResult res;	
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
		case VERTSEL_CHUNKID:
			d->vertSel.Load(iload);
			break;
		case FACESEL_CHUNKID:
			d->faceSel.Load(iload);
			break;
		case EDGESEL_CHUNKID:
			d->edgeSel.Load(iload);
			break;

		case VSELSET_CHUNK:
			res = d->vselSet.Load(iload);
			break;
		case FSELSET_CHUNK:
			res = d->fselSet.Load(iload);
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

void MeshSelMod::SetEnableStates() {
	if (!meshSelDesc.NumParamMaps ()) return;
	IParamMap2 *pmap = meshSelDesc.GetParamMap (ms_map_main);
	if (!pmap) return;
	HWND hWnd = pmap->GetHWnd();
	if (!hWnd) return;
	theSelectProc.SetEnables (hWnd);
}

void SelectProc::SetEnables (HWND hParams) {
	if (!mod) return;
	int selLevel = mod->selLevel;
	ICustButton *but;
	ISpinnerControl *spin;

	bool obj = (selLevel == SEL_OBJECT) ? TRUE : FALSE;
	bool vert = (selLevel == SEL_VERTEX) ? TRUE : FALSE;
	bool edge = (selLevel == SEL_EDGE) ? TRUE : FALSE;
	bool poly = (selLevel == SEL_POLY) ? true : false;
	bool face = (selLevel == SEL_FACE)||(selLevel == SEL_ELEMENT)||poly ? TRUE : FALSE;

	EnableWindow (GetDlgItem (hParams, IDC_MS_SEL_BYVERT), edge||face);
	//EnableWindow (GetDlgItem (hParams, IDC_MS_IGNORE_BACKFACES), edge||face);
	EnableWindow (GetDlgItem (hParams, IDC_MS_IGNORE_VISEDGE), poly);
	EnableWindow (GetDlgItem (hParams, IDC_MS_PLANAR_TEXT), poly);
	spin = GetISpinner (GetDlgItem (hParams, IDC_MS_PLANARSPINNER));
	spin->Enable (poly);
	ReleaseISpinner (spin);

	but = GetICustButton (GetDlgItem (hParams, IDC_MS_GETVERT));
	but->Enable (face||edge);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hParams, IDC_MS_GETEDGE));
	but->Enable (vert||face);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hParams, IDC_MS_GETFACE));
	but->Enable (vert||edge);
	ReleaseICustButton (but);

	EnableWindow (GetDlgItem (hParams, IDC_MS_SELBYMAT_BOX), face);
	EnableWindow (GetDlgItem (hParams, IDC_MS_SELBYMAT_TEXT), face);
	but = GetICustButton (GetDlgItem (hParams, IDC_MS_SELBYMAT));
	but->Enable (face);
	ReleaseICustButton (but);
	spin = GetISpinner (GetDlgItem (hParams, IDC_MS_MATIDSPIN));
	spin->Enable (face);
	ReleaseISpinner (spin);

	but = GetICustButton (GetDlgItem (hParams, IDC_MS_COPYNS));
	but->Enable (!obj);
	ReleaseICustButton(but);
	but = GetICustButton (GetDlgItem (hParams,IDC_MS_PASTENS));
	but->Enable (!obj && GetMeshNamedSelClip (namedClipLevel[selLevel]));
	ReleaseICustButton(but);

	but = GetICustButton (GetDlgItem (hParams, IDC_MS_SELOPEN));
	but->Enable (edge);
	ReleaseICustButton(but);
}

BOOL SelectProc::DlgProc (TimeValue t, IParamMap2 *map,
										HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (!mod) return FALSE;
	ICustToolbar *iToolbar;
#ifndef RENDER_VER
	int matid;
#endif

	switch (msg) {
	case WM_INITDIALOG:
		iToolbar = GetICustToolbar(GetDlgItem(hWnd,IDC_MS_SELTYPE));
		iToolbar->SetImage (theMeshSelImageHandler.LoadImages());
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,0,5,0,5,24,23,24,23,IDC_SELVERTEX));
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,1,6,1,6,24,23,24,23,IDC_SELEDGE));
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,2,7,2,7,24,23,24,23,IDC_SELFACE));
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,3,8,3,8,24,23,24,23,IDC_SELPOLY));
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,4,9,4,9,24,23,24,23,IDC_SELELEMENT));
		ReleaseICustToolbar(iToolbar);

		UpdateSelLevelDisplay (hWnd);
		SetEnables (hWnd);
		break;

	case WM_UPDATE_CACHE:
		mod->UpdateCache((TimeValue)wParam);
 		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_MS_SELBYMAT:
#ifndef RENDER_VER
			mod->pblock->GetValue (ms_matid, t, matid, FOREVER);
			mod->SelectByMatID(matid-1);
#endif // RENDER_VER
			break;

		case IDC_SELVERTEX:
			if (mod->selLevel == SEL_VERTEX) mod->ip->SetSubObjectLevel (SEL_OBJECT);
			else mod->ip->SetSubObjectLevel (SEL_VERTEX);
			break;

		case IDC_SELEDGE:
			if (mod->selLevel == SEL_EDGE) mod->ip->SetSubObjectLevel (SEL_OBJECT);
			else mod->ip->SetSubObjectLevel (SEL_EDGE);
			break;

		case IDC_SELFACE:
			if (mod->selLevel == SEL_FACE) mod->ip->SetSubObjectLevel (SEL_OBJECT);
			else mod->ip->SetSubObjectLevel (SEL_FACE);
			break;

		case IDC_SELPOLY:
			if (mod->selLevel == SEL_POLY) mod->ip->SetSubObjectLevel (SEL_OBJECT);
			else mod->ip->SetSubObjectLevel (SEL_POLY);
			break;

		case IDC_SELELEMENT:
			if (mod->selLevel == SEL_ELEMENT) mod->ip->SetSubObjectLevel (SEL_OBJECT);
			else mod->ip->SetSubObjectLevel (SEL_ELEMENT);
			break;

#ifndef RENDER_VER
		case IDC_MS_SELOPEN: mod->SelectOpenEdges(); break;

		case IDC_MS_GETVERT: mod->SelectFrom(SEL_VERTEX); break;
		case IDC_MS_GETEDGE: mod->SelectFrom(SEL_EDGE); break;
		case IDC_MS_GETFACE: mod->SelectFrom(SEL_FACE); break;

		case IDC_MS_COPYNS:  mod->NSCopy();  break;
		case IDC_MS_PASTENS: mod->NSPaste(); break;
#endif // RENDER_VER
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code != TTN_NEEDTEXT) break;
		LPTOOLTIPTEXT lpttt;
		lpttt = (LPTOOLTIPTEXT)lParam;				
		switch (lpttt->hdr.idFrom) {
		case IDC_SELVERTEX:
			lpttt->lpszText = GetString (IDS_RB_VERTEX);
			break;
		case IDC_SELEDGE:
			lpttt->lpszText = GetString (IDS_RB_EDGE);
			break;
		case IDC_SELFACE:
			lpttt->lpszText = GetString(IDS_RB_FACE);
			break;
		case IDC_SELPOLY:
			lpttt->lpszText = GetString(IDS_EM_POLY);
			break;
		case IDC_SELELEMENT:
			lpttt->lpszText = GetString(IDS_EM_ELEMENT);
			break;
		}
		break;
	
	default: return FALSE;
	}
	return TRUE;
}

#define GRAPHSTEPS 20

void SoftSelProc::DrawCurve (HWND hWnd) {
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd,&ps);

	float pinch, falloff, bubble;
	ISpinnerControl *spin = GetISpinner(GetDlgItem(hWnd,IDC_FALLOFFSPIN));
	falloff = spin->GetFVal();
	ReleaseISpinner(spin);	

	spin = GetISpinner(GetDlgItem(hWnd,IDC_PINCHSPIN));
	pinch = spin->GetFVal();
	ReleaseISpinner(spin);

	spin = GetISpinner(GetDlgItem(hWnd,IDC_BUBBLESPIN));
	bubble = spin->GetFVal();
	ReleaseISpinner(spin);	

	TSTR label = FormatUniverseValue(falloff);
	SetWindowText(GetDlgItem(hWnd,IDC_FARLEFTLABEL),label);
	SetWindowText(GetDlgItem(hWnd,IDC_FARRIGHTLABEL),label);
	label = FormatUniverseValue (0.0f);
	SetWindowText(GetDlgItem(hWnd,IDC_NEARLABEL), label);

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
		float y = AffectRegionFunction (dist,falloff,pinch,bubble);
		int ix = rect.left + int(float(rect.w()-1) * float(i)/float(GRAPHSTEPS));
		int	iy = rect.bottom - int(y*float(rect.h()-2)) - 1;
		if (iy<orect.top) iy = orect.top;
		if (iy>orect.bottom-1) iy = orect.bottom-1;
		LineTo(hdc, ix, iy);
	}
	
	WhiteRect3D(hdc,orect,TRUE);
	EndPaint(hWnd,&ps);
}

void SoftSelProc::SetEnables (HWND hWnd) {
	bool obj = !mod || (mod->GetSelLevel() == SEL_OBJECT);
	EnableWindow (GetDlgItem (hWnd, IDC_MS_AR_USE), !obj);

	IParamBlock2 *pblock = mod->GetParamBlock (0);
	int softSel, useEDist;
	mod->pblock->GetValue (ms_use_softsel, TimeValue(0), softSel, FOREVER);
	mod->pblock->GetValue (ms_use_edist, TimeValue(0), useEDist, FOREVER);

	bool enable = (!obj && softSel) ? TRUE : FALSE;
	ISpinnerControl *spin;
	EnableWindow (GetDlgItem (hWnd, IDC_MS_USE_E_DIST), enable);
	EnableWindow (GetDlgItem (hWnd, IDC_MS_SS_BACKFACING), enable);
	spin = GetISpinner (GetDlgItem (hWnd, IDC_MS_E_DISTSPIN));
	spin->Enable (enable && useEDist);
	ReleaseISpinner (spin);
	spin = GetISpinner (GetDlgItem (hWnd, IDC_FALLOFFSPIN));
	spin->Enable (enable);
	ReleaseISpinner (spin);
	spin = GetISpinner (GetDlgItem (hWnd, IDC_PINCHSPIN));
	spin->Enable (enable);
	ReleaseISpinner (spin);
	spin = GetISpinner (GetDlgItem (hWnd, IDC_BUBBLESPIN));
	spin->Enable (enable);
	ReleaseISpinner (spin);
	EnableWindow (GetDlgItem (hWnd, IDC_FALLOFF_LABEL), enable);
	EnableWindow (GetDlgItem (hWnd, IDC_PINCH_LABEL), enable);
	EnableWindow (GetDlgItem (hWnd, IDC_BUBBLE_LABEL), enable);
}

BOOL SoftSelProc::DlgProc (TimeValue t, IParamMap2 *map,
										HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (!mod) return FALSE;
	Rect rect;

	switch (msg) {
	case WM_INITDIALOG:
		ShowWindow(GetDlgItem(hWnd,IDC_AR_GRAPH),SW_HIDE);
		SetEnables (hWnd);
		break;
		
	case WM_PAINT:
		DrawCurve(hWnd);
		return FALSE;

	case CC_SPINNER_CHANGE:
		GetClientRectP(GetDlgItem(hWnd,IDC_AR_GRAPH),&rect);
		InvalidateRect(hWnd,&rect,FALSE);
		return FALSE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_MS_AR_USE:
		case IDC_MS_USE_E_DIST:
			SetEnables (hWnd);
			break;
		}
		return false;

	default:
		return FALSE;
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

BOOL MeshSelMod::GetUniqueSetName(TSTR &name) {
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

// MeshSelData -----------------------------------------------------

LocalModData *MeshSelData::Clone() {
	MeshSelData *d = new MeshSelData;
	d->vertSel = vertSel;
	d->faceSel = faceSel;
	d->edgeSel = edgeSel;
	d->vselSet = vselSet;
	d->eselSet = eselSet;
	d->fselSet = fselSet;
	return d;
}

MeshSelData::MeshSelData(Mesh &mesh) {
	vertSel = mesh.vertSel;
	faceSel = mesh.faceSel;
	edgeSel = mesh.edgeSel;
	held = FALSE;
	this->mesh = NULL;
	vdValid = NEVER;
	temp = NULL;
}

void MeshSelData::SynchBitArrays() {
	if (!mesh) return;
	if (vertSel.GetSize() != mesh->getNumVerts()) vertSel.SetSize(mesh->getNumVerts(),TRUE);
	if (faceSel.GetSize() != mesh->getNumFaces()) faceSel.SetSize(mesh->getNumFaces(),TRUE);
	if (edgeSel.GetSize() != mesh->getNumFaces()*3) edgeSel.SetSize(mesh->getNumFaces()*3,TRUE);
}

AdjEdgeList *MeshSelData::GetAdjEdgeList () {
	if (!mesh) return NULL;
	if (!temp) temp = new MeshTempData;
	temp->SetMesh (mesh);
	return temp->AdjEList ();
}

AdjFaceList *MeshSelData::GetAdjFaceList () {
	if (!mesh) return NULL;
	if (!temp) temp = new MeshTempData;
	temp->SetMesh (mesh);
	return temp->AdjFList ();
}

void MeshSelData::SetCache(Mesh &mesh) {
	if (this->mesh) delete this->mesh;
	this->mesh = new Mesh(mesh);
	if (temp) temp->Invalidate (PART_TOPO);
	SynchBitArrays ();
}

void MeshSelData::FreeCache() {
	if (mesh) delete mesh;
	mesh = NULL;
	if (temp) delete temp;
	temp = NULL;
}

void MeshSelData::SetVertSel(BitArray &set, IMeshSelect *imod, TimeValue t) {
	MeshSelMod *mod = (MeshSelMod *) imod;
	if (theHold.Holding()) theHold.Put (new SelectRestore (mod, this, SEL_VERTEX));
	vertSel = set;
	if (mesh) mesh->vertSel = set;
	InvalidateVDistances ();
}

void MeshSelData::SetFaceSel(BitArray &set, IMeshSelect *imod, TimeValue t) {
	MeshSelMod *mod = (MeshSelMod *) imod;
	if (theHold.Holding()) theHold.Put (new SelectRestore (mod, this, SEL_FACE));
	faceSel = set;
	if (mesh) mesh->faceSel = set;
	InvalidateVDistances ();
}

void MeshSelData::SetEdgeSel(BitArray &set, IMeshSelect *imod, TimeValue t) {
	MeshSelMod *mod = (MeshSelMod *) imod;
	if (theHold.Holding()) theHold.Put (new SelectRestore (mod, this, SEL_EDGE));
	edgeSel = set;
	if (mesh) mesh->edgeSel = set;
	InvalidateVDistances ();
}

void MeshSelData::SelVertByFace() {
	DbgAssert (mesh);
	if (!mesh) return;
	for (int i=0; i<mesh->getNumFaces(); i++) {
		if (!faceSel[i]) continue;
		for (int j=0; j<3; j++) vertSel.Set (mesh->faces[i].v[j]);
	}
}

void MeshSelData::SelVertByEdge() {
	DbgAssert (mesh);
	if (!mesh) return;
	for (int i=0; i<mesh->getNumFaces(); i++) {		
		for (int j=0; j<3; j++) {
			if (!edgeSel[i*3+j]) continue;
			vertSel.Set(mesh->faces[i].v[j],TRUE);
			vertSel.Set(mesh->faces[i].v[(j+1)%3],TRUE);
		}
	}
}

void MeshSelData::SelFaceByVert() {
	DbgAssert (mesh);
	if (!mesh) return;
	for (int i=0; i<mesh->getNumFaces(); i++) {
		for (int j=0; j<3; j++) if (vertSel[mesh->faces[i].v[j]]) break;
		if (j<3) faceSel.Set(i);
	}
}

void MeshSelData::SelFaceByEdge() {
	DbgAssert (mesh);
	if (!mesh) return;
	for (int i=0; i<mesh->getNumFaces(); i++) {
		for (int j=0; j<3; j++) if (edgeSel[i*3+j]) break;
		if (j<3) faceSel.Set(i);
	}
}

void MeshSelData::SelPolygonByVert (float thresh, int igVis) {
	DbgAssert (mesh);
	if (!mesh) return;
	BitArray nfsel;
	nfsel.SetSize (mesh->getNumFaces ());
	for (int i=0; i<mesh->getNumFaces(); i++) {
		for (int j=0; j<3; j++) if (vertSel[mesh->faces[i].v[j]]) break;
		if (j==3) continue;
		mesh->PolyFromFace (i, nfsel, thresh, igVis, GetAdjFaceList ());
	}
	faceSel |= nfsel;
}

void MeshSelData::SelPolygonByEdge (float thresh, int igVis) {
	DbgAssert (mesh);
	if (!mesh) return;
	BitArray nfsel;
	nfsel.SetSize (mesh->getNumFaces ());
	for (int i=0; i<mesh->getNumFaces(); i++) {
		for (int j=0; j<3; j++) if (edgeSel[i*3+j]) break;
		if (j==3) continue;
		mesh->PolyFromFace (i, nfsel, thresh, igVis, GetAdjFaceList());
	}
	faceSel |= nfsel;
}

void MeshSelData::SelElementByVert () {
	DbgAssert (mesh);
	if (!mesh) return;
	BitArray nfsel;
	nfsel.SetSize (mesh->getNumFaces ());
	for (int i=0; i<mesh->getNumFaces(); i++) {
		for (int j=0; j<3; j++) if (vertSel[mesh->faces[i].v[j]]) break;
		if (j==3) continue;
		mesh->ElementFromFace (i, nfsel, GetAdjFaceList());
	}
	faceSel |= nfsel;
}

void MeshSelData::SelElementByEdge () {
	DbgAssert (mesh);
	if (!mesh) return;
	BitArray nfsel;
	nfsel.SetSize (mesh->getNumFaces ());
	for (int i=0; i<mesh->getNumFaces(); i++) {
		for (int j=0; j<3; j++) if (edgeSel[i*3+j]) break;
		if (j==3) continue;
		mesh->ElementFromFace (i, nfsel, GetAdjFaceList());
	}
	faceSel |= nfsel;
}

void MeshSelData::SelEdgeByVert() {
	DbgAssert (mesh);
	if (!mesh) return;
	for (int i=0; i<mesh->getNumFaces(); i++) {
		for (int j=0; j<3; j++) {
			if (vertSel[mesh->faces[i].v[j]]) edgeSel.Set(i*3+j);
			if (vertSel[mesh->faces[i].v[(j+1)%3]]) edgeSel.Set(i*3+j);
		}
	}
}

void MeshSelData::SelEdgeByFace() {
	DbgAssert (mesh);
	if (!mesh) return;
	for (int i=0; i<mesh->getNumFaces(); i++) {
		if (!faceSel[i]) continue;
		for (int j=0; j<3; j++) edgeSel.Set(i*3+j);
	}
}


// SelectRestore --------------------------------------------------

SelectRestore::SelectRestore(MeshSelMod *m, MeshSelData *data) {
	mod     = m;
	level   = mod->selLevel;
	d       = data;
	d->held = TRUE;
	switch (level) {
	case SEL_OBJECT: DbgAssert(0); break;
	case SEL_VERTEX: usel = d->vertSel; break;
	case SEL_EDGE:
		usel = d->edgeSel;
		break;
	default:
		usel = d->faceSel;
		break;
	}
}

SelectRestore::SelectRestore(MeshSelMod *m, MeshSelData *data, int sLevel) {
	mod     = m;
	level   = sLevel;
	d       = data;
	d->held = TRUE;
	switch (level) {
	case SEL_OBJECT: DbgAssert(0); break;
	case SEL_VERTEX: usel = d->vertSel; break;
	case SEL_EDGE:
		usel = d->edgeSel; break;
	default:
		usel = d->faceSel; break;
	}
}

void SelectRestore::Restore(int isUndo) {
	if (isUndo) {
		switch (level) {			
		case SEL_VERTEX:
			rsel = d->vertSel; break;
		case SEL_FACE: 
		case SEL_POLY:
		case SEL_ELEMENT:
			rsel = d->faceSel; break;
		case SEL_EDGE:
			rsel = d->edgeSel; break;
		}
	}
	switch (level) {		
	case SEL_VERTEX:
		d->vertSel = usel; break;
	case SEL_FACE:
	case SEL_POLY:
	case SEL_ELEMENT:
		d->faceSel = usel; break;
	case SEL_EDGE:
		d->edgeSel = usel; break;
	}
	mod->LocalDataChanged ();
}

void SelectRestore::Redo() {
	switch (level) {		
	case SEL_VERTEX:
		d->vertSel = rsel; break;
	case SEL_FACE:
	case SEL_POLY:
	case SEL_ELEMENT:
		d->faceSel = rsel; break;
	case SEL_EDGE:
		d->edgeSel = rsel; break;
	}
	mod->LocalDataChanged ();
}


//--- Named selection sets -----------------------------------------

int MeshSelMod::FindSet(TSTR &setName, int nsl) {
	for (int i=0; i<namedSel[nsl].Count(); i++) {
		if (setName == *namedSel[nsl][i]) return i;
	}
	return -1;
}

DWORD MeshSelMod::AddSet(TSTR &setName,int nsl) {
	DWORD id = 0;
	TSTR *name = new TSTR(setName);
	int nsCount = namedSel[nsl].Count();

	// Find an empty id to assign to this set.
	BOOL found = FALSE;
	while (!found) {
		found = TRUE;
		for (int i=0; i<ids[nsl].Count(); i++) {
			if (ids[nsl][i]!=id) continue;
			id++;
			found = FALSE;
			break;
		}
	}

	// Find location in alphabetized list:
	for (int pos=0; pos<nsCount; pos++) if (setName < *(namedSel[nsl][pos])) break;
	if (pos == nsCount) {
		namedSel[nsl].Append(1,&name);
		ids[nsl].Append(1,&id);
	} else {
		namedSel[nsl].Insert (pos, 1, &name);
		ids[nsl].Insert (pos, 1, &id);
	}

	return id;
}

void MeshSelMod::RemoveSet(TSTR &setName,int nsl) {
	int i = FindSet(setName,nsl);
	if (i<0) return;
	delete namedSel[nsl][i];
	namedSel[nsl].Delete(i,1);
	ids[nsl].Delete(i,1);
}

void MeshSelMod::UpdateSetNames () {
	if (!ip) return;

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	for (int i=0; i<mcList.Count(); i++) {
		MeshSelData *meshData = (MeshSelData*)mcList[i]->localData;
		if ( !meshData ) continue;
		for (int nsl=0; nsl<3; nsl++) {
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
			// Check for old, unnamed or misnamed sets with ids.
			for (k=0; k<gnsl.Count(); k++) {
				for (j=0; j<ids[nsl].Count(); j++) if (ids[nsl][j] == gnsl.ids[k]) break;
				if (j == ids[nsl].Count()) continue;
				if (gnsl.names[k] && !(*(gnsl.names[k]) == *(namedSel[nsl][j]))) {
					delete gnsl.names[k];
					gnsl.names[k] = NULL;
				}
				if (gnsl.names[k]) continue;
				gnsl.names[k] = new TSTR(*(namedSel[nsl][j]));
			}
			gnsl.Alphabetize ();

			// Now check lists against each other, adding any missing elements.
			for (j=0; j<gnsl.Count(); j++) {
				if (j>= namedSel[nsl].Count()) {
					TSTR *nname = new TSTR(*gnsl.names[j]);
					DWORD nid = gnsl.ids[j];
					namedSel[nsl].Append (1, &nname);
					ids[nsl].Append (1, &nid);
					continue;
				}
				if (*(gnsl.names[j]) == *(namedSel[nsl][j])) continue;
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

void MeshSelMod::ClearSetNames() {
	for (int i=0; i<3; i++) {
		for (int j=0; j<namedSel[i].Count(); j++) {
			delete namedSel[i][j];
			namedSel[i][j] = NULL;
		}
	}
}

void MeshSelMod::ActivateSubSelSet(TSTR &setName) {
	ModContextList mcList;
	INodeTab nodes;
	int nsl = namedSetLevel[selLevel];
	int index = FindSet (setName, nsl);	
	if (index<0 || !ip) return;

	theHold.Begin ();
	ip->GetModContexts(mcList,nodes);

	for (int i = 0; i < mcList.Count(); i++) {
		MeshSelData *meshData = (MeshSelData*)mcList[i]->localData;
		if (!meshData) continue;
		if (theHold.Holding() && !meshData->held) theHold.Put(new SelectRestore(this,meshData));

		BitArray *set = NULL;

		switch (nsl) {
		case NS_VERTEX:
			set = meshData->vselSet.GetSet(ids[nsl][index]);
			if (set) {
				if (set->GetSize()!=meshData->vertSel.GetSize()) {
					set->SetSize(meshData->vertSel.GetSize(),TRUE);
				}
				meshData->SetVertSel (*set, this, ip->GetTime());
			}
			break;

		case NS_FACE:
			set = meshData->fselSet.GetSet(ids[nsl][index]);
			if (set) {
				if (set->GetSize()!=meshData->faceSel.GetSize()) {
					set->SetSize(meshData->faceSel.GetSize(),TRUE);
				}
				meshData->SetFaceSel (*set, this, ip->GetTime());
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
	theHold.Accept (GetString (IDS_DS_SELECT));
	ip->RedrawViews(ip->GetTime());
}

void MeshSelMod::NewSetFromCurSel(TSTR &setName) {
	ModContextList mcList;
	INodeTab nodes;
	DWORD id = -1;
	int nsl = namedSetLevel[selLevel];
	int index = FindSet(setName, nsl);
	if (index<0) {
		id = AddSet(setName, nsl);
		if (theHold.Holding()) theHold.Put (new AddSetNameRestore (this, id, &namedSel[nsl], &ids[nsl]));
	} else id = ids[nsl][index];

	ip->GetModContexts(mcList,nodes);

	for (int i = 0; i < mcList.Count(); i++) {
		MeshSelData *meshData = (MeshSelData*)mcList[i]->localData;
		if (!meshData) continue;
		
		BitArray *set = NULL;

		switch (nsl) {
		case NS_VERTEX:	
			if (index>=0 && (set = meshData->vselSet.GetSet(id))) {
				*set = meshData->vertSel;
			} else {
				meshData->vselSet.InsertSet(meshData->vertSel,id, setName);
				if (theHold.Holding()) theHold.Put (new AddSetRestore (&(meshData->vselSet), id, setName));
			}
			break;

		case NS_FACE:
			if (index>=0 && (set = meshData->fselSet.GetSet(id))) {
				*set = meshData->faceSel;
			} else {
				meshData->fselSet.InsertSet(meshData->faceSel,id, setName);
				if (theHold.Holding()) theHold.Put (new AddSetRestore (&(meshData->fselSet), id, setName));
			}
			break;

		case NS_EDGE:
			if (index>=0 && (set = meshData->eselSet.GetSet(id))) {
				*set = meshData->edgeSel;
			} else {
				meshData->eselSet.InsertSet(meshData->edgeSel,id, setName);
				if (theHold.Holding()) theHold.Put (new AddSetRestore (&(meshData->eselSet), id, setName));
			}
			break;
		}
	}
	nodes.DisposeTemporary();
}

void MeshSelMod::RemoveSubSelSet(TSTR &setName) {
	int nsl = namedSetLevel[selLevel];
	int index = FindSet (setName, nsl);
	if (index<0 || !ip) return;		

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	DWORD id = ids[nsl][index];

	for (int i = 0; i < mcList.Count(); i++) {
		MeshSelData *meshData = (MeshSelData*)mcList[i]->localData;
		if (!meshData) continue;		

		switch (nsl) {
		case NS_VERTEX:	
			if (theHold.Holding()) theHold.Put(new DeleteSetRestore(&meshData->vselSet,id,setName));
			meshData->vselSet.RemoveSet(id);
			break;

		case NS_FACE:
			if (theHold.Holding()) theHold.Put(new DeleteSetRestore(&meshData->fselSet,id,setName));
			meshData->fselSet.RemoveSet(id);
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

void MeshSelMod::SetupNamedSelDropDown() {
	if (selLevel == SEL_OBJECT) return;
	ip->ClearSubObjectNamedSelSets();
	int nsl = namedSetLevel[selLevel];
	for (int i=0; i<namedSel[nsl].Count(); i++)
		ip->AppendSubObjectNamedSelSet(*namedSel[nsl][i]);
	UpdateNamedSelDropDown ();
}

int MeshSelMod::NumNamedSelSets() {
	int nsl = namedSetLevel[selLevel];
	return namedSel[nsl].Count();
}

TSTR MeshSelMod::GetNamedSelSetName(int i) {
	int nsl = namedSetLevel[selLevel];
	return *namedSel[nsl][i];
}

void MeshSelMod::SetNamedSelSetName(int i,TSTR &newName) {
	int nsl = namedSetLevel[selLevel];
	if (theHold.Holding()) theHold.Put(new SetNameRestore(&namedSel[nsl],this,&ids[nsl],ids[nsl][i]));
	*namedSel[nsl][i] = newName;
}

void MeshSelMod::NewSetByOperator(TSTR &newName,Tab<int> &sets,int op) {
	int nsl = namedSetLevel[selLevel];
	DWORD id = AddSet(newName, nsl);
	if (theHold.Holding())
		theHold.Put (new AddSetNameRestore (this, id, &namedSel[nsl], &ids[nsl]));

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	for (int i = 0; i < mcList.Count(); i++) {
		MeshSelData *meshData = (MeshSelData*)mcList[i]->localData;
		if (!meshData) continue;
	
		BitArray bits;
		GenericNamedSelSetList *setList;

		switch (nsl) {
		case NS_VERTEX: setList = &meshData->vselSet; break;
		case NS_FACE: setList = &meshData->fselSet; break;			
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
		setList->InsertSet (bits, id, newName);
		if (theHold.Holding()) theHold.Put (new AddSetRestore(setList, id, newName));
	}
}

void MeshSelMod::NSCopy() {
	int index = SelectNamedSet();
	if (index<0) return;
	if (!ip) return;

	int nsl = namedSetLevel[selLevel];
	MeshNamedSelClip *clip = new MeshNamedSelClip(*namedSel[nsl][index]);

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	for (int i = 0; i < mcList.Count(); i++) {
		MeshSelData *meshData = (MeshSelData*)mcList[i]->localData;
		if (!meshData) continue;

		GenericNamedSelSetList *setList;
		switch (nsl) {
		case NS_VERTEX: setList = &meshData->vselSet; break;				
		case NS_FACE: setList = &meshData->fselSet; break;			
		case NS_EDGE: setList = &meshData->eselSet; break;			
		}		

		BitArray *bits = new BitArray(*setList->sets[index]);
		clip->sets.Append(1,&bits);
	}
	SetMeshNamedSelClip(clip, namedClipLevel[selLevel]);
	
	// Enable the paste button
	if (!meshSelDesc.NumParamMaps ()) return;
	IParamMap2 *pmap = meshSelDesc.GetParamMap (ms_map_main);
	if (!pmap) return;
	HWND hWnd = pmap->GetHWnd();
	if (!hWnd) return;
	ICustButton *but;
	but = GetICustButton(GetDlgItem(hWnd, IDC_MS_PASTENS));
	but->Enable();
	ReleaseICustButton(but);
}

void MeshSelMod::NSPaste() {
	int nsl = namedSetLevel[selLevel];
	MeshNamedSelClip *clip = GetMeshNamedSelClip(namedClipLevel[selLevel]);
	if (!clip) return;	
	TSTR name = clip->name;
	if (!GetUniqueSetName(name)) return;

	ModContextList mcList;
	INodeTab nodes;
	theHold.Begin();

	DWORD id = AddSet (name, nsl);	
	if (theHold.Holding())
		theHold.Put(new AddSetNameRestore(this, id, &namedSel[nsl], &ids[nsl]));	

	ip->GetModContexts(mcList,nodes);
	for (int i = 0; i < mcList.Count(); i++) {
		MeshSelData *meshData = (MeshSelData*)mcList[i]->localData;
		if (!meshData) continue;

		GenericNamedSelSetList *setList;
		switch (nsl) {
		case NS_VERTEX: setList = &meshData->vselSet; break;
		case NS_EDGE: setList = &meshData->eselSet; break;
		case NS_FACE: setList = &meshData->fselSet; break;
		}
				
		if (i>=clip->sets.Count()) {
			BitArray bits;
			setList->InsertSet(bits,id,name);
		} else setList->InsertSet(*clip->sets[i],id,name);
		if (theHold.Holding()) theHold.Put (new AddSetRestore (setList, id, name));
	}	
	
	ActivateSubSelSet(name);
	ip->SetCurNamedSelSet(name);
	theHold.Accept(GetString (IDS_TH_PASTE_NAMED_SEL));
	SetupNamedSelDropDown();
}

int MeshSelMod::SelectNamedSet() {
	Tab<TSTR*> &setList = namedSel[namedSetLevel[selLevel]];
	if (!ip) return false;
	return DialogBoxParam (hInstance, MAKEINTRESOURCE(IDD_SEL_NAMEDSET),
		ip->GetMAXHWnd(), PickSetDlgProc, (LPARAM)&setList);
}

void MeshSelMod::SetNumSelLabel() {	
	TSTR buf;
	int num = 0, which;

	if (!meshSelDesc.NumParamMaps ()) return;
	IParamMap2 *pmap = meshSelDesc.GetParamMap (ms_map_main);
	if (!pmap) return;
	HWND hParams = pmap->GetHWnd();
	if (!hParams) return;

	ModContextList mcList;
	INodeTab nodes;

	ip->GetModContexts(mcList,nodes);
	for (int i = 0; i < mcList.Count(); i++) {
		MeshSelData *meshData = (MeshSelData*)mcList[i]->localData;
		if (!meshData) continue;

		switch (selLevel) {
		case SEL_VERTEX:
			num += meshData->vertSel.NumberSet();
			if (meshData->vertSel.NumberSet() == 1) {
				for (which=0; which<meshData->vertSel.GetSize(); which++) if (meshData->vertSel[which]) break;
			}
			break;
		case SEL_FACE:
		case SEL_POLY:
		case SEL_ELEMENT:
			num += meshData->faceSel.NumberSet();
			if (meshData->faceSel.NumberSet() == 1) {
				for (which=0; which<meshData->faceSel.GetSize(); which++) if (meshData->faceSel[which]) break;
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
	case SEL_VERTEX:			
		if (num==1) buf.printf (GetString(IDS_EM_WHICHVERTSEL), which+1);
		else buf.printf(GetString(IDS_RB_NUMVERTSELP),num);
		break;

	case SEL_FACE:
	case SEL_POLY:
	case SEL_ELEMENT:
		if (num==1) buf.printf (GetString(IDS_EM_WHICHFACESEL), which+1);
		else buf.printf(GetString(IDS_RB_NUMFACESELP),num);
		break;

	case SEL_EDGE:
		if (num==1) buf.printf (GetString(IDS_EM_WHICHEDGESEL), which+1);
		else buf.printf(GetString(IDS_RB_NUMEDGESELP),num);
		break;

	case SEL_OBJECT:
		buf = GetString (IDS_EM_OBJECT_SEL);
		break;
	}

	SetDlgItemText(hParams,IDC_MS_NUMBER_SEL,buf);
}

RefResult MeshSelMod::NotifyRefChanged (Interval changeInt, RefTargetHandle hTarget, 
   		PartID& partID, RefMessage message) {
	if ((message ==REFMSG_CHANGE) && (hTarget == pblock)) {
		// if this was caused by a NotifyDependents from pblock, LastNotifyParamID()
		// will contain ID to update, else it will be -1 => inval whole rollout
		int pid = pblock->LastNotifyParamID();
		InvalidateDialogElement (pid);
		if ((pid == ms_edist) || (pid == ms_use_edist)) {
			// Because the technique we use to measure vertex distance has changed, we
			// need to invalidate the currently cached distances.
			InvalidateVDistances ();
		}
	}
	return(REF_SUCCEED);
}

int MeshSelMod::NumSubObjTypes() 
{ 
	return 5;
}

ISubObjType *MeshSelMod::GetSubObjType(int i) 
{

	static bool initialized = false;
	if(!initialized)
	{
		initialized = true;
		SOT_Vertex.SetName(GetString(IDS_RB_VERTEX));
		SOT_Edge.SetName(GetString(IDS_RB_EDGE));
		SOT_Face.SetName(GetString(IDS_RB_FACE));
		SOT_Poly.SetName(GetString(IDS_EM_POLY));
		SOT_Element.SetName(GetString(IDS_EM_ELEMENT));
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
		return &SOT_Face;
	case 3:
		return &SOT_Poly;
	case 4:
		return &SOT_Element;
	}
	return NULL;
}

void MeshSelMod::SetSoftSelEnables () {
#ifndef RENDER_VER
	if (!pblock) return;
	if (!meshSelDesc.NumParamMaps ()) return;
	IParamMap2 *pmap = meshSelDesc.GetParamMap (ms_map_softsel);
	if (!pmap) return;
	HWND hWnd = pmap->GetHWnd();
	if (!hWnd) return;
	theSoftSelProc.SetEnables(hWnd);
#endif
}

void MeshSelMod::InvalidateDialogElement (int elem) {
	if (!pblock) return;
	if (!meshSelDesc.NumParamMaps ()) return;
	IParamMap2 *pmap = meshSelDesc.GetParamMap (ms_map_main);
	if (pmap) pmap->Invalidate (elem);

#ifndef RENDER_VER
	pmap = meshSelDesc.GetParamMap (ms_map_softsel);
	if (pmap) pmap->Invalidate (elem);

	if (editMod != this) return;
	HWND hWnd = pmap->GetHWnd();
	if (!hWnd) return;

	switch (elem) {
	case ms_use_softsel:
	case ms_use_edist:
		theSoftSelProc.SetEnables (hWnd);
	}
#endif
}
