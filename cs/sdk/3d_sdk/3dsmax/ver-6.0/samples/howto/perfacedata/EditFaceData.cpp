/**********************************************************************
 *<
	FILE: EditFaceData.cpp

	DESCRIPTION:  A modifier for editing Max's sample face data.

	CREATED BY: Steve Anderson

	HISTORY: created 6/27/2001

 *>	Copyright (c) Discreet 2001, All Rights Reserved.
 **********************************************************************/

#include "PerFaceData.h"
#include "SampleFaceData.h"
#include "MeshDLib.h"

#define EDIT_FACEDATA_CLASS_ID Class_ID(0xc78708b, 0x3e4f063a)

// Local static instance.
static GenSubObjType SOT_Face(3);

// selection levels:
#define SEL_OBJECT	0
#define SEL_FACE	1

class EditFaceDataRestore;
class EditFaceDataModData;

class EditFaceDataMod : public Modifier {
	DWORD selLevel;
	bool mCollapsable, mDisabled;
	EditFaceDataModData *mpModDataCollapseCache;

	static IObjParam *ip;
	static EditFaceDataMod *editMod;
	static SelectModBoxCMode *selectMode;
	static BOOL updateCachePosted;
	static HWND hParams;
	static EditFaceDataRestore *efdRestore;

public:
	EditFaceDataMod() : selLevel(SEL_FACE), mCollapsable(false), mDisabled(false), mpModDataCollapseCache(NULL) { }
	~EditFaceDataMod ();

	// From Animatable
	void DeleteThis() { delete this; }
	void GetClassName(TSTR& s) {s = GetString(IDS_EDIT_CLASS_NAME);}  
	virtual Class_ID ClassID() { return EDIT_FACEDATA_CLASS_ID;}		
	RefTargetHandle Clone(RemapDir& remap = NoRemap());
	TCHAR *GetObjectName() {return GetString(IDS_EDIT_CLASS_NAME);}

	// From modifier
	ChannelMask ChannelsUsed()  {return PART_GEOM|PART_TOPO;}
	ChannelMask ChannelsChanged() {return PART_SELECT|PART_TOPO;}
	Class_ID InputType() { return defObjectClassID; }
	void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
	Interval LocalValidity(TimeValue t) { return FOREVER; }
	Interval GetValidity (TimeValue t) { return FOREVER; }
	BOOL DependOnTopology(ModContext &mc) { return TRUE; }

	void NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index);
	void NotifyPostCollapse(INode *node,Object *obj, IDerivedObject *derObj, int index);

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
	void SelectionChanged ();
	void ValueChanged ();

	void ShowEndResultChanged (BOOL showEndResult) { NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE); }

	// IO
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);
	IOResult SaveLocalData(ISave *isave, LocalModData *ld);
	IOResult LoadLocalData(ILoad *iload, LocalModData **pld);

	// NS: New SubObjType API
	int NumSubObjTypes();
	ISubObjType *GetSubObjType(int i);

	int NumRefs() { return 0; }
	RefTargetHandle GetReference(int i) { return NULL; }
	void SetReference(int i, RefTargetHandle rtarg) { }
	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message) { return REF_SUCCEED; }

	void UpdateDialog ();

	void ChangeBegin ();
	void ChangeTo (float value);
	void ChangeFinish (bool accept);

	void ResetSelection ();
	void SetCollapsable (bool clbl) { mCollapsable = clbl; }
	bool GetCollapsable () { return mCollapsable; }
	void SetDialogHandle (HWND hWnd) { hParams = hWnd; }
};

class EditFaceDataModData : public LocalModData {
	friend class EditFaceDataRestore;

private:
	BitArray mFacesAffected;
	BitArray mFaceSel;
	Tab<float> mtNewFaceValues;
	Mesh *mpCacheMesh;
	AdjEdgeList *mpAdjEdge;
	MNMesh *mpCacheMNMesh;
	bool mHeld;	// Used to prevent repeated RestoreObjs on same LocalModData.

public:
	EditFaceDataModData () : mpCacheMesh(NULL), mpCacheMNMesh(NULL), mpAdjEdge(NULL), mHeld(false) { }
	~EditFaceDataModData () { FreeCache (); }

	LocalModData *Clone();

	void SynchSize (int numFaces = -1);
	void ApplyChanges (Mesh & mesh);
	void ApplyChanges (MNMesh & mesh);

	int NumFaces () { return mFaceSel.GetSize(); }
	BitArray & GetFaceSel () { return mFaceSel; }
	bool GetHeld () { return mHeld; }
	void SetHeld (bool held) { mHeld = held; }
	void DescribeSelection (int & numFaces, int & whichFace, float & value, bool & valueDetermined);
	float FaceValue (int faceID);
	bool FaceAffected (int faceID) { return ((faceID>=0)&&(faceID<mFacesAffected.GetSize())&&mFacesAffected[faceID]) ? true : false; }
	void SetFaceValue (int faceID, float value);
	void SetFaceValue (BitArray & faces, float value);
	void ResetFace (int faceID) { if ((faceID>=0)&&(faceID<mFacesAffected.GetSize())) mFacesAffected.Clear(faceID); }
	void ResetFace (BitArray & faces);

	// Cache copies of last mesh modified for display and hit testing.
	void SetCacheMesh (Mesh & mesh);
	void SetCacheMNMesh (MNMesh & mesh);
	Mesh *GetCacheMesh () { return mpCacheMesh; }
	MNMesh *GetCacheMNMesh () { return mpCacheMNMesh; }
	AdjEdgeList *GetEdgeList ();
	void FreeCache ();
};

class SelectRestore : public RestoreObj {
public:
	BitArray usel, rsel;
	BitArray *sel;
	EditFaceDataMod *mod;
	EditFaceDataModData *d;

	SelectRestore(EditFaceDataMod *m, EditFaceDataModData *d);
	void Restore(int isUndo);
	void Redo();
	int Size() { return 1; }
	void EndHold() {d->SetHeld (false);}
	TSTR Description() { return TSTR(_T("SelectRestore")); }
};

class EditFaceDataRestore : public RestoreObj {
public:
	BitArray faces;
	int firstFace;
	Tab<float> before;
	float after;
	BitArray set_before;
	bool set_after, after_called;

	EditFaceDataModData *modData;
	EditFaceDataMod *mod;

	EditFaceDataRestore (EditFaceDataMod *m, EditFaceDataModData *md);
	void After ();
	void Restore(int isUndo);
	void Redo();
	int Size() { return 2*sizeof(void *) + sizeof(float) + 2*sizeof(bool) + 2*sizeof(BitArray) + sizeof(Tab<float>); }
	TSTR Description() {return TSTR(_T("Face Data Edit"));}
};


//--- ClassDescriptor and class vars ---------------------------------

EditFaceDataRestore *EditFaceDataMod::efdRestore = NULL;
IObjParam *EditFaceDataMod::ip              = NULL;
EditFaceDataMod *EditFaceDataMod::editMod         = NULL;
SelectModBoxCMode *EditFaceDataMod::selectMode      = NULL;
BOOL EditFaceDataMod::updateCachePosted = FALSE;
HWND EditFaceDataMod::hParams = NULL;

class EditFaceDataClassDesc:public ClassDesc {
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new EditFaceDataMod; }
	const TCHAR *	ClassName() { return GetString(IDS_EDIT_CLASS_NAME); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return EDIT_FACEDATA_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY);}
};

static EditFaceDataClassDesc editFaceDataDesc;
ClassDesc* GetEditFaceDataDesc() {return &editFaceDataDesc;}

// Table to convert selLevel values to mesh selLevel flags.
const int meshLevel[] = {MESH_OBJECT, MESH_FACE };
// Table to convert selLevel values to MNMesh selLevel flags
const int mnmeshLevel[] = { MNM_SL_OBJECT, MNM_SL_FACE };

// Get Mesh display flags based on selLevel.
const DWORD levelDispFlags[] = { 0, DISP_SELFACES };
// Get MNMesh display flags based on selLevel.
const DWORD mnlevelDispFlags[] = { 0, MNDISP_SELFACES };

// For hit testing...
const int hitLevel[] = { 0, SUBHIT_FACES };
// For hit testing MNMeshes...
const int mnhitLevel[] = { 0, SUBHIT_MNFACES };

//--- EditFaceDataMod methods -------------------------------

EditFaceDataMod::~EditFaceDataMod () {
	if (mpModDataCollapseCache) delete mpModDataCollapseCache;
}

RefTargetHandle EditFaceDataMod::Clone(RemapDir& remap) {
	EditFaceDataMod *mod = new EditFaceDataMod();
	mod->selLevel = selLevel;
	BaseClone(this, mod, remap);
	return mod;
}

void EditFaceDataMod::ModifyObject (TimeValue t, ModContext &mc, ObjectState *os, INode *node) {
	if (mDisabled) return;

	EditFaceDataModData *d  = (EditFaceDataModData*)mc.localData;
	if (!d) mc.localData = d = new EditFaceDataModData();

	if (os->obj->IsSubClassOf(triObjectClassID)) {
		// Access the Mesh:
		TriObject *tobj = (TriObject*)os->obj;
		Mesh & mesh = tobj->GetMesh();

		// Apply our modifier's changes:
		d->ApplyChanges (mesh);

		// Update the cache used for display and hit testing:
		if (!d->GetCacheMesh()) d->SetCacheMesh(mesh);

		// Set display flags according to SO level:
		mesh.dispFlags = 0;
		mesh.SetDispFlag (levelDispFlags[selLevel]);

	} else if (os->obj->IsSubClassOf(polyObjectClassID)) {

		// Access the Mesh:
		PolyObject *pobj = (PolyObject*)os->obj;
		MNMesh & mesh = pobj->GetMesh();

		// Apply our modifier's changes:
		d->ApplyChanges (mesh);

		// Update the cache used for display and hit testing:
		if (!d->GetCacheMNMesh()) d->SetCacheMNMesh(mesh);

		// Set display flags according to SO level:
		mesh.dispFlags = 0;
		mesh.SetDispFlag (mnlevelDispFlags[selLevel]);
	}
}

void EditFaceDataMod::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc) {
	if (!mc->localData) return;
	if (partID == PART_SELECT) return;
	((EditFaceDataModData*)mc->localData)->FreeCache();
}

// Neil Hazzard's clever approach to surviving a stack collapse:
void EditFaceDataMod::NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index) {
	if (mCollapsable) return;

	// Cache a copy of our local mod data:
	// (Does this work for multiple instances?)
	ModContext* pModCtx = derObj->GetModContext( index );
	mpModDataCollapseCache = (EditFaceDataModData*)pModCtx->localData->Clone();

	// Reevaluate with modifier turned off.
	mDisabled = true;
	TimeValue t = GetCOREInterface()->GetTime();
	NotifyDependents(Interval(t,t),PART_ALL,REFMSG_CHANGE);
}

// We want to survive a collapsed stack so we reapply ourselves here
void EditFaceDataMod::NotifyPostCollapse(INode *node,Object *obj, IDerivedObject *derObj, int index) {
	if (mCollapsable) return;

	Object *bo = node->GetObjectRef();
	IDerivedObject *derob = NULL;
	if(bo->SuperClassID() != GEN_DERIVOB_CLASS_ID) {
		derob = CreateDerivedObject(obj);
		node->SetObjectRef(derob);
	} else derob = (IDerivedObject*) bo;

	// Add ourselves to the top of the stack
	derob->AddModifier(this,NULL,derob->NumModifiers());

	// Reinsert our local mod data
	ModContext* mc = derob->GetModContext(derob->NumModifiers()-1);
	mc->localData = mpModDataCollapseCache;
	mpModDataCollapseCache = NULL;

	// Reengage modification:
	mDisabled = false;
}

static INT_PTR CALLBACK FaceDataDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	EditFaceDataMod *em = (EditFaceDataMod *) GetWindowLongPtr (hWnd, GWLP_USERDATA);
	ISpinnerControl *spin;

	switch (msg) {
	case WM_INITDIALOG:
		em = (EditFaceDataMod*) lParam;
		em->SetDialogHandle (hWnd);
		SetWindowLongPtr (hWnd,GWLP_USERDATA,lParam);			

		spin = GetISpinner(GetDlgItem(hWnd,IDC_VALUE_SPIN));
		spin->SetLimits (0.0f, 100.0f, FALSE);
		spin->SetScale (0.1f);
		spin->LinkToEdit (GetDlgItem(hWnd, IDC_VALUE), EDITTYPE_POS_FLOAT);
		spin->SetValue (1.0f, FALSE);
		spin->Disable();
		ReleaseISpinner(spin);

		CheckDlgButton (hWnd, IDC_COLLAPSABLE, em->GetCollapsable());

		em->UpdateDialog ();
		break;

	case CC_SPINNER_BUTTONDOWN:
		switch (LOWORD(wParam)) {
		case IDC_VALUE_SPIN:
			em->ChangeBegin ();
			break;
		}
		break;

	case CC_SPINNER_CHANGE:
		spin = (ISpinnerControl*)lParam;
		switch (LOWORD(wParam)) {
		case IDC_VALUE_SPIN:
			em->ChangeTo (spin->GetFVal());
			break;
		}
		break;

	case CC_SPINNER_BUTTONUP:
		switch (LOWORD(wParam)) {
		case IDC_VALUE_SPIN:
			em->ChangeFinish (HIWORD(wParam) ? true : false);
			break;
		}
		break;

	case WM_CUSTEDIT_ENTER:
		switch (LOWORD(wParam)) {
		case IDC_VALUE:
			em->ChangeFinish (true);
			break;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_RESET_SELECTION:
			em->ResetSelection ();
			break;
		case IDC_COLLAPSABLE:
			em->SetCollapsable (IsDlgButtonChecked (hWnd, IDC_COLLAPSABLE)?true:false);
			break;
		}
		break;

	case WM_PAINT:
		// Good place for call to UpdateDialog, if needed.
		return FALSE;

	default:
		return FALSE;
	}
	return TRUE;
}

void EditFaceDataMod::UpdateDialog () {	
	TSTR buf;
	int numFaces=0, whichFace=0;
	float value = 1.0f;
	bool valueDetermined = true;

	if (!hParams) return;

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	if (selLevel) {
		for (int i = 0; i < mcList.Count(); i++) {
			EditFaceDataModData *meshData = (EditFaceDataModData*)mcList[i]->localData;
			if (!meshData) continue;
			int numSelectedHere;
			meshData->DescribeSelection (numSelectedHere, whichFace, value, valueDetermined);
			numFaces += numSelectedHere;
		}
	}

	ISpinnerControl *spin = GetISpinner (GetDlgItem (hParams, IDC_VALUE_SPIN));
	ICustButton *but = GetICustButton (GetDlgItem (hParams, IDC_RESET_SELECTION));

	switch (selLevel) {
	case SEL_FACE:
		if (numFaces==1) {
			buf.printf (GetString(IDS_FACE_SELECTED), whichFace+1);
		} else {
			if (numFaces) buf.printf (GetString (IDS_FACES_SELECTED), numFaces);
			else buf = GetString (IDS_NO_FACE_SELECTED);
		}
		but->Enable (numFaces);
		spin->Enable (numFaces);
		if (numFaces && valueDetermined) {
			spin->SetIndeterminate (false);
			spin->SetValue (value, FALSE);
		} else {
			spin->SetIndeterminate (true);
		}
		break;

	case SEL_OBJECT:
		buf = GetString (IDS_OBJECT_SELECTED);
		spin->Disable();
		but->Disable ();
		break;
	}

	SetDlgItemText(hParams,IDC_FACE_SELECTED,buf);
	ReleaseISpinner (spin);
	ReleaseICustButton (but);
}

void EditFaceDataMod::BeginEditParams (IObjParam  *ip, ULONG flags,Animatable *prev) {
	this->ip = ip;	
	editMod  = this;

	selectMode = new SelectModBoxCMode(this,ip);

	// Restore the selection level.
	ip->SetSubObjectLevel(selLevel);

	if (!hParams) {
		hParams = ip->AddRollupPage (hInstance, MAKEINTRESOURCE (IDD_FACEDATA_EDIT),
			FaceDataDlgProc, GetString (IDS_EDIT_FACE_DATA), (LPARAM)this, 0);
	} else {
		UpdateDialog ();
	}

	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);	
}

void EditFaceDataMod::EndEditParams (IObjParam *ip,ULONG flags,Animatable *next) {
	if (flags&END_EDIT_REMOVEUI ) {
		if (hParams) {
			ip->DeleteRollupPage(hParams);
			hParams = NULL;
		}
	}

	if (selectMode) {
		ip->DeleteMode(selectMode);
		delete selectMode;
	}
	selectMode = NULL;

	this->ip = NULL;
	editMod  = NULL;

	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);
	ClearAFlag(A_MOD_BEING_EDITED);
}

int EditFaceDataMod::HitTest (TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc) {
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
	gw->clearHitCode();

	if (!mc->localData) return 0;
	EditFaceDataModData *md = (EditFaceDataModData*)mc->localData;
	Mesh *mesh = md->GetCacheMesh ();
	MNMesh *mnmesh = md->GetCacheMNMesh ();

	SubObjHitList hitList;
	if (mesh) {
		res = mesh->SubObjectHitTest(gw, gw->getMaterial(), &hr, flags|hitLevel[selLevel], hitList);
	} else if (mnmesh) {
		res = mnmesh->SubObjectHitTest (gw, gw->getMaterial(), &hr, flags|mnhitLevel[selLevel], hitList);
	}

	MeshSubHitRec *rec = hitList.First();
	while (rec) {
		vpt->LogHit(inode,mc,rec->dist,rec->index,NULL);
		rec = rec->Next();
	}

	gw->setRndLimits(savedLimits);	
	return res;	
}

int EditFaceDataMod::Display (TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc) {
	if (!ip->GetShowEndResult ()) return 0;
	if (!selLevel) return 0;
	if (!mc->localData) return 0;

	EditFaceDataModData *modData = (EditFaceDataModData *) mc->localData;
	Mesh *mesh = modData->GetCacheMesh();
	MNMesh *mnmesh = modData->GetCacheMNMesh();
	AdjEdgeList *mpAdjEdge = modData->GetEdgeList();
	if (!mesh && !mnmesh) return 0;

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

	int i;
	Point3 rp[3];
	int es[3];

	if (mesh) {
		DbgAssert (mpAdjEdge);
		for (i=0; i<mpAdjEdge->edges.Count(); i++) {
			MEdge & me = mpAdjEdge->edges[i];
			if (me.Hidden (mesh->faces)) continue;
			if (me.Visible (mesh->faces)) es[0] = GW_EDGE_VIS;
			else es[0] = GW_EDGE_INVIS;
			if (mpAdjEdge->edges[i].AFaceSelected (modData->GetFaceSel()))
				gw->setColor (LINE_COLOR, colGizSel);
			else gw->setColor (LINE_COLOR, colGiz);
			rp[0] = mesh->verts[me.v[0]];
			rp[1] = mesh->verts[me.v[1]];
			gw->polyline (2, rp, NULL, NULL, FALSE, es);
		}
	} else {
		es[0] = GW_EDGE_VIS;
		if (mnmesh->GetFlag (MN_MESH_FILLED_IN)) {
			for (i=0; i<mnmesh->nume; i++) {
				if (mnmesh->e[i].GetFlag (MN_DEAD)) continue;
				bool hidden = true, sel = false;
				hidden &= mnmesh->f[mnmesh->e[i].f1].GetFlag (MN_HIDDEN);
				sel |= (modData->GetFaceSel()[mnmesh->e[i].f1] ? true : false);
				if (mnmesh->e[i].f2 >= 0) {
					hidden &= mnmesh->f[mnmesh->e[i].f2].GetFlag (MN_HIDDEN);
					sel |= (modData->GetFaceSel()[mnmesh->e[i].f2] ? true : false);
				}
				if (hidden) continue;
				if (sel) gw->setColor (LINE_COLOR, colGizSel);
				else gw->setColor (LINE_COLOR, colGiz);
				rp[0] = mnmesh->P(mnmesh->e[i].v1);
				rp[1] = mnmesh->P(mnmesh->e[i].v1);
				gw->polyline (2, rp, NULL, NULL, FALSE, es);
			}
		} else {
			for (i=0; i<mnmesh->numf; i++) {
				if (mnmesh->f[i].GetFlag (MN_HIDDEN|MN_DEAD)) continue;
				if (modData->GetFaceSel()[i]) gw->setColor (LINE_COLOR, colGizSel);
				else gw->setColor (LINE_COLOR, colGiz);
				for (int j=0; j<mnmesh->f[i].deg; j++) {
					rp[0] = mnmesh->P(mnmesh->f[i].vtx[j]);
					rp[1] = mnmesh->P(mnmesh->f[i].vtx[(j+1)%mnmesh->f[i].deg]);
					gw->polyline (2, rp, NULL, NULL, FALSE, es);
				}
			}
		}
	}
	gw->setRndLimits(savedLimits);
	return 0;	
}

void EditFaceDataMod::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp *vpt, Box3& box, ModContext *mc) {
	if (!ip->GetShowEndResult() || !mc->localData) return;
	if (!selLevel) return;
	EditFaceDataModData *modData = (EditFaceDataModData *) mc->localData;
	Matrix3 tm = inode->GetObjectTM(t);
	if (modData->GetCacheMesh ()) box = modData->GetCacheMesh()->getBoundingBox (&tm);
	else if (modData->GetCacheMNMesh()) box = modData->GetCacheMNMesh()->getBoundingBox(&tm);
}

// Simply return the center of the bounding box of the current selection:
void EditFaceDataMod::GetSubObjectCenters (SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc) {
	if (!mc->localData) return;
	if (selLevel == SEL_OBJECT) return;	// shouldn't happen.
	EditFaceDataModData *modData = (EditFaceDataModData *) mc->localData;

	Mesh *mesh = modData->GetCacheMesh();
	MNMesh *mnmesh = modData->GetCacheMNMesh();
	if (!mesh && !mnmesh) return;

	Matrix3 tm = node->GetObjectTM(t);
	Box3 box;
	if (mesh) {
		BitArray sel = mesh->VertexTempSel ();
		if (!sel.NumberSet()) return;
		for (int i=0; i<mesh->numVerts; i++) if (sel[i]) box += mesh->verts[i] * tm;
	} else {
		int numSel, which;
		float value;
		bool valueDetermined;
		modData->DescribeSelection (numSel, which, value, valueDetermined);
		if (!numSel) return;
		if (numSel==1) {
			for (int j=0; j<mnmesh->f[which].deg; j++) box += mnmesh->P(mnmesh->f[which].vtx[j]) * tm;
		} else {
			for (int i=0; i<mnmesh->numf; i++) {
				if (mnmesh->f[i].GetFlag (MN_DEAD)) continue;
				if (!modData->GetFaceSel()[i]) continue;
				for (int j=0; j<mnmesh->f[i].deg; j++) box += mnmesh->P(mnmesh->f[i].vtx[j]) * tm;
			}
		}
	}
	cb->Center (box.Center(),0);
}

void EditFaceDataMod::GetSubObjectTMs (SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc) {
	if (!mc->localData) return;
	if (selLevel == SEL_OBJECT) return;	// shouldn't happen.
	EditFaceDataModData *modData = (EditFaceDataModData *) mc->localData;

	Mesh *mesh = modData->GetCacheMesh();
	MNMesh *mnmesh = modData->GetCacheMNMesh();
	if (!mesh && !mnmesh) return;

	Matrix3 tm = node->GetObjectTM(t);
	Box3 box;
	if (mesh) {
		BitArray sel = mesh->VertexTempSel ();
		if (!sel.NumberSet()) return;
		for (int i=0; i<mesh->numVerts; i++) if (sel[i]) box += mesh->verts[i] * tm;
	} else {
		int numSel, which;
		float value;
		bool valueDetermined;
		modData->DescribeSelection (numSel, which, value, valueDetermined);
		if (!numSel) return;
		if (numSel==1) {
			for (int j=0; j<mnmesh->f[which].deg; j++) box += mnmesh->P(mnmesh->f[which].vtx[j]) * tm;
		} else {
			for (int i=0; i<mnmesh->numf; i++) {
				if (mnmesh->f[i].GetFlag (MN_DEAD)) continue;
				if (!modData->GetFaceSel()[i]) continue;
				for (int j=0; j<mnmesh->f[i].deg; j++) box += mnmesh->P(mnmesh->f[i].vtx[j]) * tm;
			}
		}
	}
	Matrix3 ctm(1);
	ctm.SetTrans (box.Center());
	cb->TM (ctm,0);
}

void EditFaceDataMod::ActivateSubobjSel(int level, XFormModes& modes) {
	// Set the meshes level
	selLevel = level;

	// Fill in modes with our sub-object modes
	if (level!=SEL_OBJECT) {
		modes = XFormModes(NULL,NULL,NULL,NULL,NULL,selectMode);
	}

	// Update UI
	UpdateDialog ();

	ip->PipeSelLevelChanged();
	NotifyDependents(FOREVER, SELECT_CHANNEL|DISP_ATTRIB_CHANNEL|SUBSEL_TYPE_CHANNEL, REFMSG_CHANGE);
}

void EditFaceDataMod::SelectSubComponent (HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert) {
	EditFaceDataModData *d = NULL, *od = NULL;

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	BitArray nsel;

	for (int nd=0; nd<mcList.Count(); nd++) {
		d = (EditFaceDataModData*) mcList[nd]->localData;
		if (d==NULL) continue;
		HitRecord *hr = hitRec;
		if (!all && (hr->modContext->localData != d)) continue;
		for (; hr!=NULL; hr=hr->Next()) if (hr->modContext->localData == d) break;
		if (hr==NULL) continue;

		Mesh *mesh = d->GetCacheMesh();
		MNMesh *mnmesh = d->GetCacheMNMesh();
		if (!mesh && !mnmesh) continue;
		if (theHold.Holding() && !d->GetHeld()) theHold.Put (new SelectRestore (this, d));

		switch (selLevel) {
		case SEL_FACE:
			nsel = d->GetFaceSel();
			for (; hr != NULL; hr=hr->Next()) {
				if (d != hr->modContext->localData) continue;
				nsel.Set (hr->hitInfo, invert ? !d->GetFaceSel()[hr->hitInfo] : selected);
				if (!all) break;
			}
			d->GetFaceSel() = nsel;
			break;
		}
	}

	nodes.DisposeTemporary ();
	SelectionChanged ();
}

void EditFaceDataMod::ClearSelection(int selLevel) {
	ModContextList list;
	INodeTab nodes;	
	ip->GetModContexts(list,nodes);
	EditFaceDataModData *d;
	for (int i=0; i<list.Count(); i++) {
		d = (EditFaceDataModData*)list[i]->localData;
		if (!d) continue;

		// Check if we have anything selected first:
		switch (selLevel) {
		case SEL_FACE:
			if (!d->GetFaceSel().NumberSet()) continue;
			else break;
		}

		if (theHold.Holding() && !d->GetHeld()) theHold.Put (new SelectRestore (this, d));
		d->SynchSize ();
		switch (selLevel) {
		case SEL_FACE:
			d->GetFaceSel().ClearAll();
			break;
		}
	}
	nodes.DisposeTemporary();
	SelectionChanged ();
}

void EditFaceDataMod::SelectAll(int selLevel) {
	ModContextList list;
	INodeTab nodes;	
	ip->GetModContexts(list,nodes);
	EditFaceDataModData *d;
	for (int i=0; i<list.Count(); i++) {
		d = (EditFaceDataModData*)list[i]->localData;		
		if (!d) continue;
		if (theHold.Holding() && !d->GetHeld()) theHold.Put(new SelectRestore(this,d));
		d->SynchSize();
		switch (selLevel) {
		case SEL_FACE:
			d->GetFaceSel().SetAll ();
			break;
		}
	}
	nodes.DisposeTemporary();
	SelectionChanged ();
}

void EditFaceDataMod::InvertSelection(int selLevel) {
	ModContextList list;
	INodeTab nodes;	
	ip->GetModContexts(list,nodes);
	EditFaceDataModData *d;
	for (int i=0; i<list.Count(); i++) {
		d = (EditFaceDataModData*)list[i]->localData;
		if (!d) continue;
		if (theHold.Holding() && !d->GetHeld()) theHold.Put(new SelectRestore(this,d));
		d->SynchSize();
		switch (selLevel) {
		case SEL_FACE:
			d->GetFaceSel() = ~d->GetFaceSel();
			break;
		}
	}
	nodes.DisposeTemporary();
	SelectionChanged ();
}

void EditFaceDataMod::SelectionChanged () {
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	if (ip && editMod==this) UpdateDialog();
}

void EditFaceDataMod::ValueChanged () {
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	if (ip && editMod==this) UpdateDialog();
}

#define SELLEVEL_CHUNKID		0x0100

IOResult EditFaceDataMod::Save(ISave *isave) {
	IOResult res;
	ULONG nb;
	Modifier::Save(isave);

	isave->BeginChunk(SELLEVEL_CHUNKID);
	res = isave->Write(&selLevel, sizeof(selLevel), &nb);
	isave->EndChunk();

	return res;
}

IOResult EditFaceDataMod::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	Modifier::Load(iload);

	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
		case SELLEVEL_CHUNKID:
			iload->Read(&selLevel, sizeof(selLevel), &nb);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

#define FACESEL_CHUNKID			0x0210
#define FACE_VALUE_CHUNK	0x0220

IOResult EditFaceDataMod::SaveLocalData(ISave *isave, LocalModData *ld) {	
	EditFaceDataModData *d = (EditFaceDataModData*)ld;
	ULONG nb;

	isave->BeginChunk(FACESEL_CHUNKID);
	d->GetFaceSel().Save(isave);
	isave->EndChunk();

	for (int i=0; i<d->NumFaces(); i++) {
		if (!d->FaceAffected(i)) continue;
		isave->BeginChunk (FACE_VALUE_CHUNK);
		float val = d->FaceValue (i);
		isave->Write (&i, sizeof(int), &nb);
		isave->Write (&val, sizeof(float), &nb);
		isave->EndChunk ();
	}

	return IO_OK;
}

IOResult EditFaceDataMod::LoadLocalData(ILoad *iload, LocalModData **pld) {
	EditFaceDataModData *d = new EditFaceDataModData;
	*pld = d;
	IOResult res;
	ULONG nb;
	int faceID;
	float val;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
		case FACESEL_CHUNKID:
			d->GetFaceSel().Load(iload);
			d->SynchSize (d->GetFaceSel().GetSize());
			break;

		case FACE_VALUE_CHUNK:
			if ((res = iload->Read (&faceID, sizeof(int), &nb)) != IO_OK) break;
			if ((res = iload->Read (&val, sizeof(float), &nb)) != IO_OK) break;
			d->SetFaceValue (faceID, val);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

int EditFaceDataMod::NumSubObjTypes() 
{
	return 1;
}

ISubObjType *EditFaceDataMod::GetSubObjType(int i) 
{
	static bool initialized = false;

	if(!initialized) {
		initialized = true;
		SOT_Face.SetName(GetString(IDS_FACE));
	}

	switch(i) {
	case -1:	
		if(GetSubObjectLevel() > 0)
			return &SOT_Face;
		break;
	case 0:
		return &SOT_Face;
	}
	return NULL;
}

void EditFaceDataMod::ChangeBegin () {
	if (selLevel == SEL_OBJECT) return;
	if (efdRestore) {
		delete efdRestore;
		efdRestore = NULL;
	}

	// Find the modcontext with the selected faces.
	// NOTE that as currently written, this modifier won't
	// support setting FaceFloats on more than one node at a time.
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	int numSelected=0, whichFace;
	float value;
	bool valueDetermined=true;
	EditFaceDataModData *relevantMD;
	if (selLevel) {
		for (int i = 0; i < mcList.Count(); i++) {
			EditFaceDataModData *meshData = (EditFaceDataModData*)mcList[i]->localData;
			if (!meshData) continue;
			meshData->DescribeSelection (numSelected, whichFace, value, valueDetermined);
			if (!numSelected) continue;

			relevantMD = meshData;
			break;
		}
	}
	nodes.DisposeTemporary ();
	if (!numSelected) return;

	efdRestore = new EditFaceDataRestore (this, relevantMD);
}

void EditFaceDataMod::ChangeTo (float value) {
	if (selLevel == SEL_OBJECT) return;
	if (!efdRestore) ChangeBegin ();
	efdRestore->modData->SetFaceValue (efdRestore->faces, value);
}

void EditFaceDataMod::ChangeFinish (bool accept) {
	if (selLevel == SEL_OBJECT) return;
	if (!efdRestore) return;
	if (!accept) {
		efdRestore->Restore (false);
		return;
	}
	efdRestore->After ();
	theHold.Begin ();
	theHold.Put (efdRestore);
	theHold.Accept (GetString (IDS_EDIT_FACE_DATA));
	efdRestore = NULL;	// We should no longer have a record of this RestoreObj - it's been passed to theHold.

	ValueChanged ();
}

void EditFaceDataMod::ResetSelection () {
	ModContextList list;
	INodeTab nodes;	
	ip->GetModContexts(list,nodes);
	EditFaceDataModData *d;
	theHold.Begin ();
	for (int i=0; i<list.Count(); i++) {
		d = (EditFaceDataModData*)list[i]->localData;
		if (!d) continue;
		EditFaceDataRestore *efd = new EditFaceDataRestore (this, d);
		d->ResetFace (d->GetFaceSel());
		efd->After ();
		theHold.Put (efd);
	}
	nodes.DisposeTemporary();
	theHold.Accept (GetString (IDS_RESET_SELECTED));
	ValueChanged ();
}


// EditFaceDataModData -----------------------------------------------------

LocalModData *EditFaceDataModData::Clone() {
	EditFaceDataModData *d = new EditFaceDataModData;
	d->mFaceSel = mFaceSel;
	d->mFacesAffected = mFacesAffected;
	d->mtNewFaceValues = mtNewFaceValues;
	return d;
}

void EditFaceDataModData::SynchSize (int numFaces) {
	if (numFaces<0) {
		// We're supposed to get the right size from the cache.
		if (mpCacheMesh) numFaces = mpCacheMesh->numFaces;
		if (mpCacheMNMesh) numFaces = mpCacheMNMesh->numf;
		if (numFaces<0) return;	// do nothing if cache missing.
	}
	mFaceSel.SetSize (numFaces, true);
	mFacesAffected.SetSize (numFaces, true);
	mtNewFaceValues.SetCount (numFaces);
}

void EditFaceDataModData::ApplyChanges (Mesh & mesh) {
	// Make sure we're sized correctly for this mesh.
	// (NOTE: If the user reduces, then increases input number of faces, we lose data.)
	if (mesh.numFaces != mFaceSel.GetSize()) SynchSize (mesh.numFaces);

	// Set the selection:
	mesh.faceSel = mFaceSel;

	// Get the face data manager from the mesh:
	DebugPrint ("EditFaceDataMod: Getting manager from Mesh (0x%08x)\n", &mesh);
	IFaceDataMgr *pFDMgr = static_cast<IFaceDataMgr*>(mesh.GetInterface (FACEDATAMGR_INTERFACE));
	if (pFDMgr == NULL) return;

	SampleFaceData* fdc = dynamic_cast<SampleFaceData*>(pFDMgr->GetFaceDataChan( FACE_MAXSAMPLEUSE_CLSID ));
	if ( fdc == NULL ) {
		// The mesh does not have our sample face-data channel so we will add it here
		fdc = new SampleFaceData();
		fdc->FacesCreated (0, mFaceSel.GetSize());
		pFDMgr->AddFaceDataChan( fdc );
	}

	if (!mFacesAffected.NumberSet ()) return;

	for (int i=0; i<mFacesAffected.GetSize(); i++) {
		if (!mFacesAffected[i]) continue;
		fdc->SetValue (i, mtNewFaceValues[i]);
	}
}

void EditFaceDataModData::ApplyChanges (MNMesh & mesh) {
	// Make sure we're sized correctly for this mesh.
	// (NOTE: If the user reduces, then increases input number of faces, we lose data.)
	if (mesh.numf != mFaceSel.GetSize()) SynchSize (mesh.numf);

	// Set the selection:
	mesh.FaceSelect (mFaceSel);

	// Get the face data manager from the mesh:
	DebugPrint ("EditFaceDataMod: Getting manager from MNMesh (0x%08x)\n", &mesh);
	IFaceDataMgr *pFDMgr = static_cast<IFaceDataMgr*>(mesh.GetInterface (FACEDATAMGR_INTERFACE));
	if (pFDMgr == NULL) return;

	SampleFaceData* fdc = dynamic_cast<SampleFaceData*>(pFDMgr->GetFaceDataChan( FACE_MAXSAMPLEUSE_CLSID ));
	if ( fdc == NULL ) {
		// The mesh does not have our sample face-data channel so we will add it here
		fdc = new SampleFaceData();
		fdc->FacesCreated (0, mFaceSel.GetSize());
		pFDMgr->AddFaceDataChan( fdc );
	}

	if (!mFacesAffected.NumberSet ()) return;

	for (int i=0; i<mFacesAffected.GetSize(); i++) {
		if (!mFacesAffected[i]) continue;
		fdc->SetValue (i, mtNewFaceValues[i]);
	}
}

void EditFaceDataModData::DescribeSelection (int & numFaces, int & whichFace, float & value, bool &valueDetermined) {
	numFaces = 0;
	for (int i=0; i<mFaceSel.GetSize(); i++) {
		if (!mFaceSel[i]) continue;
		if (!numFaces) {
			whichFace = i;
			value = FaceValue (i);
		} else if (valueDetermined) {
			if (value != FaceValue(i)) valueDetermined = false;
		}
		numFaces++;
	}
}

float EditFaceDataModData::FaceValue (int faceID) {
	if (faceID<0) return 0.0f;
	if (faceID>mFacesAffected.GetSize()) return 0.0f;
	if (mFacesAffected[faceID]) return mtNewFaceValues[faceID];
	IFaceDataMgr *pFDMgr = NULL;
	if (mpCacheMesh && (faceID < mpCacheMesh->numFaces)) {
		// Get the face data manager from the mesh:
		pFDMgr = static_cast<IFaceDataMgr*>(mpCacheMesh->GetInterface (FACEDATAMGR_INTERFACE));
	}
	if (mpCacheMNMesh && (faceID < mpCacheMNMesh->numf)) {
		// Get the face data manager from the mesh:
		pFDMgr = static_cast<IFaceDataMgr*>(mpCacheMNMesh->GetInterface (FACEDATAMGR_INTERFACE));
	}
	if (pFDMgr == NULL) return 0.0f;
	SampleFaceData* fdc = dynamic_cast<SampleFaceData*>(pFDMgr->GetFaceDataChan( FACE_MAXSAMPLEUSE_CLSID ));
	if (!fdc) return 0.0f;
	float val;
	if (!fdc->GetValue (faceID, val)) return 0.0f;
	return val;
}

void EditFaceDataModData::SetFaceValue (int faceID, float val) {
	if (faceID<0) return;
	if (faceID>mFacesAffected.GetSize()) return;
	mFacesAffected.Set (faceID);
	mtNewFaceValues[faceID] = val;
}

void EditFaceDataModData::SetFaceValue (BitArray & faces, float val) {
	for (int i=0; i<faces.GetSize(); i++) {
		if (!faces[i]) continue;
		mFacesAffected.Set (i);
		mtNewFaceValues[i] = val;
	}
}

void EditFaceDataModData::ResetFace (BitArray & faces) {
	for (int i=0; i<faces.GetSize(); i++) {
		if (!faces[i]) continue;
		mFacesAffected.Clear (i);
	}
}

void EditFaceDataModData::SetCacheMesh (Mesh & mesh) {
	if (mpCacheMesh) delete mpCacheMesh;
	if (mpCacheMNMesh) {
		delete mpCacheMNMesh;
		mpCacheMNMesh = NULL;
	}
	if (mpAdjEdge) {
		delete mpAdjEdge;
		mpAdjEdge = NULL;
	}

	mpCacheMesh = new Mesh(mesh);
	DebugPrint ("Made a mesh cache for the EditFaceDataModData (0x%08x)\n", mpCacheMesh);
}

void EditFaceDataModData::SetCacheMNMesh (MNMesh & mesh) {
	if (mpCacheMNMesh) delete mpCacheMNMesh;
	if (mpCacheMesh) {
		delete mpCacheMesh;
		mpCacheMesh = NULL;
	}
	if (mpAdjEdge) {
		delete mpAdjEdge;
		mpAdjEdge = NULL;
	}

	mpCacheMNMesh = new MNMesh(mesh);
	DebugPrint ("Made a MNMesh cache for the EditFaceDataModData (0x%08x)\n", mpCacheMNMesh);
}

AdjEdgeList *EditFaceDataModData::GetEdgeList () {
	if (!mpCacheMesh) return NULL;
	if (!mpAdjEdge) mpAdjEdge = new AdjEdgeList(*mpCacheMesh);
	return mpAdjEdge;
}

void EditFaceDataModData::FreeCache () {
	if (mpCacheMesh) {
		delete mpCacheMesh;
		mpCacheMesh = NULL;
	}
	if (mpCacheMNMesh) {
		delete mpCacheMNMesh;
		mpCacheMNMesh = NULL;
	}
	if (mpAdjEdge) {
		delete mpAdjEdge;
		mpAdjEdge = NULL;
	}
}

// SelectRestore --------------------------------------------------

SelectRestore::SelectRestore(EditFaceDataMod *m, EditFaceDataModData *data) {
	mod     = m;
	d       = data;
	d->SetHeld (true);
	usel = d->GetFaceSel();
}

void SelectRestore::Restore(int isUndo) {
	rsel = d->GetFaceSel();
	d->GetFaceSel() = usel;
	mod->SelectionChanged ();
}

void SelectRestore::Redo() {
	d->GetFaceSel() = rsel;
	mod->SelectionChanged ();
}


// EditFaceDataRestore -----------------------------------------------

EditFaceDataRestore::EditFaceDataRestore (EditFaceDataMod *m, EditFaceDataModData *md) {
	modData = md;
	mod = m;
	faces = modData->GetFaceSel();
	firstFace = -1;
	if (faces.NumberSet() == 0) return;
	for (firstFace=0; firstFace<faces.GetSize(); firstFace++) {
		if (faces[firstFace]) break;
	}
	set_before = modData->mFacesAffected;
	before = modData->mtNewFaceValues;
	after_called = false;
}

void EditFaceDataRestore::After () {
	if (after_called) return;
	if (firstFace < 0) return;
	set_after = modData->FaceAffected (firstFace);
	after = modData->FaceValue (firstFace);
	after_called = true;
}

void EditFaceDataRestore::Restore(int isUndo) {
	if (firstFace < 0) return;
	if (isUndo && !after_called) After();
	for (int i=firstFace; i<faces.GetSize(); i++) {
		if (!faces[i]) continue;
		if (!set_before[i]) modData->ResetFace (i);
		else modData->SetFaceValue (i, before[i]);
	}
	mod->ValueChanged ();
}

void EditFaceDataRestore::Redo() {
	if (firstFace < 0) return;
	if (!set_after) modData->ResetFace (faces);
	else modData->SetFaceValue (faces, after);
	mod->ValueChanged ();
}

