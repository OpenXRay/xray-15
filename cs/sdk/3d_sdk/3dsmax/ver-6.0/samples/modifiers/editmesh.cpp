/**********************************************************************
 *<
	FILE: editmesh.cpp

	DESCRIPTION:  Edit Mesh OSM

	CREATED BY: Dan Silva & Rolf Berteig
	Heavily modified and now maintained by: Steve Anderson

	HISTORY: created 18 March, 1995

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "mods.h"
#include "MeshDLib.h"
#include "editmesh.h"

// xavier robitaille | 03.02.13 | fixed values for metric units
#ifndef METRIC_UNITS_FIXED_VALUES
#define EDITMESH_NORM_SCALE		20.0f
#else
#define EDITMESH_NORM_SCALE		DFLT_EDIT_MESH_MOD_NORMAL_SCALE
#endif

HWND                EditMeshMod::hSel = NULL;
HWND                EditMeshMod::hGeom = NULL;
HWND                EditMeshMod::hSurf = NULL;
HWND				EditMeshMod::hAR = NULL;
IObjParam*          EditMeshMod::ip      = NULL;

MoveModBoxCMode*    EditMeshMod::moveMode        = NULL;
RotateModBoxCMode*  EditMeshMod::rotMode 	     = NULL;
UScaleModBoxCMode*  EditMeshMod::uscaleMode      = NULL;
NUScaleModBoxCMode* EditMeshMod::nuscaleMode     = NULL;
SquashModBoxCMode*  EditMeshMod::squashMode      = NULL;
SelectModBoxCMode*  EditMeshMod::selectMode      = NULL;
ExtrudeCMode*       EditMeshMod::extrudeMode     = NULL;
BevelCMode * EditMeshMod::bevelMode = NULL;
ChamferCMode *EditMeshMod::chamferMode = NULL;
CreateVertCMode*	EditMeshMod::createVertMode  = NULL;
WeldVertCMode* EditMeshMod::weldVertMode = NULL;
AttachPickMode* EditMeshMod::attachPickMode = NULL;
CreateFaceCMode* EditMeshMod::createFaceMode = NULL;
TurnEdgeCMode* EditMeshMod::turnEdgeMode = NULL;
DivideEdgeCMode* EditMeshMod::divideEdgeMode = NULL;
DivideFaceCMode* EditMeshMod::divideFaceMode = NULL;
CutEdgeCMode * EditMeshMod::cutEdgeMode = NULL;
FlipNormCMode * EditMeshMod::flipMode = NULL;

BOOL  EditMeshMod::inExtrude = FALSE;
BOOL EditMeshMod::inBevel = FALSE;
BOOL EditMeshMod::inChamfer = FALSE;
int EditMeshMod::extType = MESH_EXTRUDE_CLUSTER;
BOOL EditMeshMod::inBuildFace = FALSE;
BOOL EditMeshMod::inCutEdge = FALSE;
BOOL EditMeshMod::faceUIValid = FALSE;
float EditMeshMod::normScale = EDITMESH_NORM_SCALE;
bool EditMeshMod::showVNormals = false;
bool EditMeshMod::showFNormals = false;
BOOL EditMeshMod::ignoreBackfaces = FALSE;
BOOL EditMeshMod::ignoreVisEdge = FALSE;
BOOL EditMeshMod::selByVert = FALSE;
int EditMeshMod::weldBoxSize = DEF_PICKBOX_SIZE;
int EditMeshMod::pickBoxSize = DEF_PICKBOX_SIZE;
Quat EditMeshMod::sliceRot(0.0f,0.0f,0.0f,1.0f);
Point3 EditMeshMod::sliceCenter(0.0f,0.0f,0.0f);
bool EditMeshMod::sliceMode = FALSE;
float EditMeshMod::sliceSize = 100.0f;
bool EditMeshMod::sliceSplit = FALSE;
bool EditMeshMod::cutRefine = TRUE;
BOOL EditMeshMod::rsSel = TRUE;
BOOL EditMeshMod::rsAR = FALSE;
BOOL EditMeshMod::rsGeom = TRUE;
BOOL EditMeshMod::rsSurf = TRUE;
int EditMeshMod::condenseMat = true;
int EditMeshMod::attachMat = ATTACHMAT_IDTOMAT;

static EditMeshClassDesc editMeshDesc;
extern ClassDesc* GetEditMeshModDesc() { return &editMeshDesc; }

void EditMeshClassDesc::ResetClassParams(BOOL fileReset) {
	EditMeshMod::normScale       = EDITMESH_NORM_SCALE;
	EditMeshMod::showVNormals = false;
	EditMeshMod::showFNormals = false;
	EditMeshMod::selByVert       = FALSE;
	EditMeshMod::extType = MESH_EXTRUDE_CLUSTER;
	EditMeshMod::sliceMode = FALSE;
	EditMeshMod::sliceSize = 100.0f;
	EditMeshMod::sliceSplit = FALSE;
	EditMeshMod::cutRefine = TRUE;
	EditMeshMod::rsSel = TRUE;
	EditMeshMod::rsAR = FALSE;
	EditMeshMod::rsGeom = TRUE;
	EditMeshMod::rsSurf = TRUE;
	EditMeshMod::condenseMat = true;
	EditMeshMod::attachMat = ATTACHMAT_IDTOMAT;
	ResetEditMeshUI();
}

EditMeshDeleteEvent delEvent;


/*-------------------------------------------------------------------*/
//
// Per instance data
//

EditMeshData::EditMeshData() {
	flags = 0;
	mod = NULL;
	mesh = NULL;
	tmr = NULL;
	tempData = NULL;
	mValid.SetEmpty ();
	topoValid.SetEmpty ();
	geomValid.SetEmpty ();
	updateMD = FALSE;
	lockInvalidate = FALSE;
}

EditMeshData::EditMeshData(EditMeshData& emc) {
	mdelta   = emc.mdelta;
	for (int i=0; i<3; i++) selSet[i] = emc.selSet[i];
	flags    = emc.flags & EMD_HASDATA;
	mod = NULL;
	mesh = NULL;
	tmr = NULL;
	tempData = NULL;
	mValid.SetEmpty ();
	topoValid.SetEmpty ();
	geomValid.SetEmpty ();
	updateMD = FALSE;
	lockInvalidate = FALSE;
}

EditMeshData::~EditMeshData() {
	if (tempData) delete tempData;
	if (mesh) delete mesh;
	if (tmr) delete tmr;
}

void EditMeshData::Apply (TimeValue t, TriObject *triOb, EditMeshMod *mod) {
	Interval inGeomValid = triOb->ChannelValidity (t, GEOM_CHAN_NUM);
	Interval inTopoValid = triOb->ChannelValidity (t, TOPO_CHAN_NUM);
	Interval inVCValid = triOb->ChannelValidity (t, VERT_COLOR_CHAN_NUM);
	Interval inTexValid = triOb->ChannelValidity (t, TEXMAP_CHAN_NUM);

	if (!GetFlag(EMD_UPDATING_CACHE) && MeshCached(t)) {
		// Just copy the cached mesh.
		triOb->GetMesh().DeepCopy (GetMesh(t),
			PART_GEOM|SELECT_CHANNEL|PART_SUBSEL_TYPE|
			PART_DISPLAY|PART_TOPO|PART_TEXMAP|PART_VERTCOLOR);
		triOb->PointsWereChanged();
	}
	else if (GetFlag(EMD_HASDATA)) {
		// Use the mesh delta.
		triOb->GetMesh().freeVSelectionWeights ();
		lastInVNum = triOb->GetMesh().numVerts;
		lastInFNum = triOb->GetMesh().numFaces;
		if (mdelta.vnum < lastInVNum) mdelta.SetInVNum (lastInVNum);
		else if (mdelta.vnum > lastInVNum) updateMD = TRUE;
		if (mdelta.fnum < lastInFNum) mdelta.SetInFNum (lastInFNum);
		else if (mdelta.fnum > lastInFNum) updateMD = TRUE;
		lastInMVNum.SetCount (mdelta.mapSupport.GetSize());
		for (int i=0; i<mdelta.mapSupport.GetSize(); i++) {
			if (!mdelta.mapSupport[i]) continue;
			if (!triOb->GetMesh().mapSupport(i)) lastInMVNum[i] = lastInVNum;
			else lastInMVNum[i] = triOb->GetMesh().getNumMapVerts (i);
			if (mdelta.map[i].vnum < lastInMVNum[i]) mdelta.map[i].SetInVNum (lastInMVNum[i]);
			else if (mdelta.map[i].vnum > lastInMVNum[i]) updateMD = TRUE;
		}
		mdelta.Apply(triOb->GetMesh());
		selSet[NS_VERTEX].SetSize(triOb->GetMesh().getNumVerts());
		selSet[NS_FACE].SetSize(triOb->GetMesh().getNumFaces());
		selSet[NS_EDGE].SetSize(triOb->GetMesh().getNumFaces()*3);
		triOb->PointsWereChanged();
	}

	triOb->GetMesh().dispFlags = 0;
	bool dispVerts = FALSE;
	if (mod->inBuildFace) dispVerts = TRUE;

	switch (mod->selLevel) {
	case SL_VERTEX:
		dispVerts = TRUE;
		if (mod->showVNormals)
			triOb->GetMesh().displayNormals (MESH_DISP_VERTEX_NORMALS, mod->normScale);
		break;
	case SL_POLY:
	case SL_ELEMENT:
		triOb->GetMesh().SetDispFlag(DISP_SELPOLYS);
		if (mod->showFNormals)
			triOb->GetMesh().displayNormals(MESH_DISP_FACE_NORMALS,mod->normScale);
		break;
	case SL_FACE:
		triOb->GetMesh().SetDispFlag(DISP_SELFACES);
		if (mod->showFNormals)
			triOb->GetMesh().displayNormals(MESH_DISP_FACE_NORMALS,mod->normScale);
		break;
	case SL_EDGE:
		triOb->GetMesh().SetDispFlag(DISP_SELEDGES);
		break;
	}
	if (dispVerts && (!mod->ip || !mod->ip->GetShowEndResult()))
		triOb->GetMesh().SetDispFlag(DISP_VERTTICKS|DISP_SELVERTS);
	triOb->GetMesh().selLevel = meshLevel[mod->selLevel];
	
	// If we have got tvFaces then make sure the flag is
	// set for all faces.
	// SCA 5/2/01: This flag is very obselete.  Do not use.
	//if (triOb->GetMesh().tvFace) {
		//for (int i=0; i<triOb->GetMesh().getNumFaces(); i++) {
			//triOb->GetMesh().faces[i].flags |= HAS_TVERTS;
		//}
	//}

	if (GetFlag(EMD_UPDATING_CACHE)) {
		//MaxAssert(tempData);
		UpdateCache(t, triOb);
		SetFlag(EMD_UPDATING_CACHE,FALSE);
	}

	// Output only changes over time in response to certain changes in input.
	triOb->UpdateValidity (GEOM_CHAN_NUM, inGeomValid & inTopoValid);
	triOb->UpdateValidity (TOPO_CHAN_NUM, inTopoValid);
	triOb->UpdateValidity (SELECT_CHAN_NUM, inTopoValid);
	triOb->UpdateValidity (SUBSEL_TYPE_CHAN_NUM, FOREVER);
	triOb->UpdateValidity (DISP_ATTRIB_CHAN_NUM, inTopoValid);
	int vc=0;
	if (mdelta.mapSupport[0]) {
		triOb->UpdateValidity (VERT_COLOR_CHAN_NUM, inTopoValid & inVCValid);
		vc = 1;
	}
	if (mdelta.mapSupport.NumberSet() > vc) {
		triOb->UpdateValidity (TEXMAP_CHAN_NUM, inTopoValid & inTexValid);
	}

	if (mod->affectRegion && mod->selLevel) {
		triOb->GetMesh().SupportVSelectionWeights ();
		float *vsw = triOb->GetMesh().getVSelectionWeights ();
		if (vsw) {
			Tab<float> *ourvwTable = TempData(t)->VSWeight (mod->useEdgeDist, mod->edgeIts, mod->arIgBack,
				mod->falloff, mod->pinch, mod->bubble);
			if (ourvwTable && ourvwTable->Count()) {
				float *ourvw = ourvwTable->Addr(0);
				memcpy (vsw, ourvw, triOb->GetMesh().getNumVerts()*sizeof(float));
			}
		}
	}
}

void EditMeshData::BeginEdit (TimeValue t) {
	if (GetFlag(EMD_HASDATA)) return;
	GetMesh (t);
	mdelta.InitToMesh (*mesh);
	if (mdelta.vdSupport.GetSize() > VDATA_SELECT)
		mdelta.vdSupport.Clear (VDATA_SELECT);	// Clear out incoming soft selections.
	SetFlag(EMD_HASDATA,TRUE);
}

BitArray EditMeshData::GetSel (int nsl) {
	switch (nsl) {
	case NS_VERTEX: return mdelta.vsel;
	case NS_EDGE: return mdelta.esel;
	}
	return mdelta.fsel;
}

void EditMeshData::AddVertHide (BitArray hideMe, IMeshSelect *mod, TimeValue t) {
	BeginEdit (t);
	if (theHold.Holding()) theHold.Put (new VertexHideRestore (this, (EditMeshMod*)mod));
	hideMe.SetSize (mesh->numVerts);
	mesh->vertHide |= hideMe;
	mesh->InvalidateGeomCache ();
	BitArray nsel = mesh->vertSel & ~hideMe;
	hideMe.SetSize (mdelta.outVNum());
	mdelta.vhide |= hideMe;
	if (nsel.NumberSet() != mesh->vertSel.NumberSet()) SetVertSel (nsel, mod, t);
}

void EditMeshData::ClearVertHide (IMeshSelect *mod, TimeValue t) {
	BeginEdit (t);
	if (theHold.Holding()) theHold.Put (new VertexHideRestore (this, (EditMeshMod*)mod));
	mesh->vertHide.ClearAll ();
	mesh->InvalidateGeomCache ();
	mdelta.vhide.ClearAll ();
}

void EditMeshData::SetVertSel (BitArray &set, IMeshSelect *mod, TimeValue t) {
	EditMeshMod *em = (EditMeshMod *) mod;
	BeginEdit (t);
	if (theHold.Holding()) theHold.Put (new MeshSelectRestore (this, em, SL_VERTEX));
	mesh->vertSel = set;
	mesh->vertSel.SetSize (mesh->numVerts);
	mesh->vertSel &= ~mesh->vertHide;
	mdelta.vsel = mesh->vertSel;
	if (mdelta.vsel.GetSize () != mdelta.outVNum()) {
		mdelta.vsel.SetSize (mdelta.outVNum(), TRUE);
	}
	Invalidate (PART_SELECT);
}

void EditMeshData::SetFaceSel(BitArray &set, IMeshSelect *mod, TimeValue t) {
	EditMeshMod *em = (EditMeshMod *) mod;
	BeginEdit (t);
	if (theHold.Holding()) theHold.Put (new MeshSelectRestore(this, em, SL_FACE));
	mdelta.fsel = set;
	if (set.GetSize () != mdelta.outFNum()) {
		mdelta.fsel.SetSize (mdelta.outFNum(), TRUE);
	}
	GetMesh (t);
	mdelta.SelectFacesByFlags (*mesh, FALSE, FACE_HIDDEN, FACE_HIDDEN);
	mesh->faceSel = mdelta.fsel;
	mesh->faceSel.SetSize (mesh->numFaces);
	Invalidate(PART_SELECT);
}

void EditMeshData::SetEdgeSel(BitArray &set, IMeshSelect *mod, TimeValue t) {
	EditMeshMod *em = (EditMeshMod *) mod;
	BeginEdit (t);
	if (theHold.Holding()) theHold.Put (new MeshSelectRestore (this, em, SL_EDGE));
	mdelta.esel = set;
	if (set.GetSize () != mdelta.outFNum()*3) {
		mdelta.esel.SetSize (mdelta.outFNum()*3, TRUE);
	}
	if (MeshCached(t)) {
		mesh->edgeSel = mdelta.esel;
		mesh->edgeSel.SetSize (mesh->numFaces*3);
		Invalidate(PART_SELECT);
	}
}

void EditMeshData::SetSel (int nsl, BitArray & set, IMeshSelect *mod, TimeValue t) {
	switch (nsl) {
	case NS_VERTEX: SetVertSel (set, mod, t); break;
	case NS_EDGE: SetEdgeSel (set, mod, t); break;
	case NS_FACE: SetFaceSel (set, mod, t); break;
	}
}

class NamedSetSizeChange : public RestoreObj {
public:
	EditMeshData *emd;
	int nsl;
	int oldsize, change;

	NamedSetSizeChange (EditMeshData *e, int n, int old, int inc) { emd=e; nsl=n; oldsize=old; change=inc; }
	void Restore (int isUndo) { emd->selSet[nsl].SetSize (oldsize); }
	void Redo () { emd->selSet[nsl].SetSize (oldsize+change); }
	int GetSize () { return 3*sizeof(int) + sizeof (void *); }
	TSTR Description () { return _T("Named Selection Set Size Change"); }
};

void EditMeshData::ChangeNamedSetSize (int nsl, int oldsize, int change) {
	if (change == 0) return;
	if (selSet[nsl].Count() == 0) return;
	if (theHold.Holding())
		theHold.Put (new NamedSetSizeChange (this, nsl, oldsize, change));
	selSet[nsl].SetSize (oldsize + change);
}

class NamedSetDelete : public RestoreObj {
public:
	EditMeshData *emd;
	int nsl;
	Tab<BitArray *> oldSets;
	BitArray del;

	NamedSetDelete (EditMeshData *em, int n, BitArray &d);
	~NamedSetDelete ();
	void Restore (int isUndo);
	void Redo () { emd->selSet[nsl].DeleteSetElements (del, (nsl==NS_EDGE) ? 3 : 1); }
	int GetSize () { return 3*sizeof(int) + sizeof (void *); }
	TSTR Description () { return _T("Named Selection Set Subset Deletion"); }
};

NamedSetDelete::NamedSetDelete (EditMeshData *em, int n, BitArray &d) {
	emd = em;
	nsl = n;
	del = d;
	oldSets.SetCount (emd->selSet[nsl].Count());
	for (int i=0; i<emd->selSet[nsl].Count(); i++) {
		oldSets[i] = new BitArray;
		(*oldSets[i]) = (*(emd->selSet[nsl].sets[i]));
	}
}

NamedSetDelete::~NamedSetDelete () {
	for (int i=0; i<oldSets.Count(); i++) delete oldSets[i];
}

void NamedSetDelete::Restore (int isUndo) {
	int i, max = oldSets.Count();
	if (emd->selSet[nsl].Count() < max) max = emd->selSet[nsl].Count();
	for (i=0; i<max; i++) *(emd->selSet[nsl].sets[i]) = *(oldSets[i]);
}

void EditMeshData::DeleteNamedSetArray (int nsl, BitArray & del) {
	if (del.NumberSet() == 0) return;
	if (selSet[nsl].Count() == 0) return;
	if (theHold.Holding()) 
		theHold.Put (new NamedSetDelete (this, nsl, del));
	selSet[nsl].DeleteSetElements (del, (nsl==NS_EDGE) ? 3 : 1);
}

void EditMeshData::ApplyMeshDelta (MeshDelta & md, MeshDeltaUser *mdu, TimeValue t) {
	DWORD partsChanged = md.PartsChanged();
	if (updateMD) partsChanged |= (PART_TOPO|PART_GEOM|PART_SELECT|PART_TEXMAP|PART_VERTCOLOR);
	EditMeshMod *mod = (EditMeshMod *) mdu;
	BeginEdit (t);
	if (theHold.Holding()) {
		theHold.Put(new MeshEditRestore (this, mod, md));
	}
	if (updateMD) {
		mdelta.SetInVNum (lastInVNum);
		mdelta.SetInFNum (lastInFNum);
		for (int mp=0; mp<mdelta.mapSupport.GetSize(); mp++) {
			if (!mdelta.mapSupport[mp]) continue;
			mdelta.map[mp].SetInVNum (lastInMVNum[mp]);
		}
		updateMD = FALSE;
	}

	// If there are creates, modify the named selection sets to match:
	int nv;
	if (nv = md.vClone.Count()) {
		ChangeNamedSetSize (NS_VERTEX, mdelta.outVNum(), nv);
	}
	if (md.NumFCreate()) {
		ChangeNamedSetSize (NS_EDGE, mdelta.outFNum()*3, md.NumFCreate()*3);
		ChangeNamedSetSize (NS_FACE, mdelta.outFNum(), md.NumFCreate());
	}

	// If we clone any of mdelta's creates, we'll have to recompute the mesh.
	mdelta.Compose (md);
	if (MeshCached (t)) md.Apply(*mesh);
	Invalidate (partsChanged, TRUE);

	// If there were deletes, change the named selection sets to match:
	if (md.vDelete.NumberSet()) {
		DeleteNamedSetArray (NS_VERTEX, md.vDelete);
	}
	if (md.fDelete.NumberSet()) {
		DeleteNamedSetArray (NS_EDGE, md.fDelete);
		DeleteNamedSetArray (NS_FACE, md.fDelete);
	}

	mod->LocalDataChanged (partsChanged);
}

/*-------------------------------------------------------------------*/		
//
// Edit Mesh Modifier
//


EditMeshMod::EditMeshMod() {
	//if (GetSystemSetting(SYSSET_ENABLE_EDITMESHMOD)) selLevel = SL_VERTEX;
	selLevel = SL_OBJECT;
	affectRegion = FALSE;
	useEdgeDist = 0;
	edgeIts = 1;
	arIgBack = 0;
	falloff = 20.0f;
	pinch = bubble = 0.0f;
	emFlags = 0;
}

EditMeshMod::~EditMeshMod() {
	ClearSetNames();
}

class ScaleVertDeltas : public ModContextEnumProc {
public:
	float f;
	EditMeshMod *mod;
	ScaleVertDeltas(float ff,EditMeshMod *m) {f=ff;mod=m;}
	BOOL proc(ModContext *mc) {
		EditMeshData *emdata = (EditMeshData*)mc->localData;
		if (!emdata) return TRUE;
		if (theHold.Holding()) theHold.Put(new VertexEditRestore(emdata, mod));
		MeshDelta & md = emdata->mdelta;
		for (int i=0; i<md.vMove.Count(); i++) md.vMove[i].dv *= f;
		for (i=0; i<md.vClone.Count(); i++) md.vClone[i].dv *= f;
		return TRUE;
	}
};

void EditMeshMod::RescaleWorldUnits(float f) {
	if (TestAFlag(A_WORK1)) return;
	SetAFlag(A_WORK1);
	ScaleVertDeltas svd(f,this);
	EnumModContexts(&svd);
}

Interval EditMeshMod::LocalValidity(TimeValue t) {
	// Force a cache if being edited.
	// TH 12/20/95 -- Uncommented this code to test fix of failure to cache Extrusions/Lathes
	 if (TestAFlag(A_MOD_BEING_EDITED))  // DONT WANT THIS since we have our own Cache.
	 	return NEVER;
	return FOREVER;
}

RefTargetHandle EditMeshMod::Clone(RemapDir& remap) {
	EditMeshMod* newmod = new EditMeshMod();
	newmod->selLevel = selLevel;
	newmod->affectRegion = affectRegion;
	newmod->falloff = falloff;
	newmod->pinch = pinch;
	newmod->bubble = bubble;
	newmod->useEdgeDist = useEdgeDist;
	newmod->edgeIts = edgeIts;
	newmod->arIgBack = arIgBack;
	for (int i=0; i<3; i++)
	{
		newmod->namedSel[i].SetCount (namedSel[i].Count());
		for (int j=0; j<namedSel[i].Count(); j++) newmod->namedSel[i][j] = new TSTR (*namedSel[i][j]);
		newmod->ids[i].SetCount (ids[i].Count());
		for (j=0; j<ids[i].Count(); j++) newmod->ids[i][j] = ids[i][j];
	}
	BaseClone(this, newmod, remap);
	return(newmod);
}

BOOL EditMeshMod::DependOnTopology(ModContext &mc) {
	EditMeshData *meshData = (EditMeshData*)mc.localData;
	if (meshData) {
		if (meshData->GetFlag(EMD_HASDATA)) {
			return TRUE;
		}
	}
	return FALSE;
}


void EditMeshMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) {
	MaxAssert(os->obj->IsSubClassOf(triObjectClassID));

	TriObject *triOb = (TriObject *)os->obj;
	EditMeshData *meshData;

	if ( !mc.localData ) {
		mc.localData = new EditMeshData();
		meshData = (EditMeshData*)mc.localData;
	} else {
		meshData = (EditMeshData*)mc.localData;
	}
	meshData->SetModifier (this);

	meshData->Apply(t,triOb,this);
}

void EditMeshMod::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc) {
	if ( mc->localData ) {
		EditMeshData *meshData = (EditMeshData*)mc->localData;
		if ( meshData ) meshData->Invalidate(partID,FALSE);
	}
}

void EditMeshMod::SelectSubComponent (HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert) {
	if (selLevel == SL_OBJECT) return;
	if (!ip) return;
	if (sliceMode) return;
	TimeValue t = ip->GetTime();
	ip->ClearCurNamedSelSet();
	EndExtrude(t);

	AdjFaceList *af=NULL;
	AdjEdgeList *ae=NULL;
	BitArray nsel;
	HitRecord *hr;
	EditMeshData *meshData;
	Mesh *mesh;

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	int nd;

	int localSelByVert = selByVert;
	if (cutEdgeMode == ip->GetCommandMode()) localSelByVert = FALSE;
	if (turnEdgeMode == ip->GetCommandMode()) localSelByVert = FALSE;
	if (divideEdgeMode == ip->GetCommandMode()) localSelByVert = FALSE;
	if (divideFaceMode == ip->GetCommandMode()) localSelByVert = FALSE;

	for (nd=0; nd<mcList.Count(); nd++) {
		meshData = (EditMeshData*) mcList[nd]->localData;
		if (meshData==NULL) continue;
		if (!all && (hitRec->modContext->localData != meshData)) continue;
		for (hr=hitRec; hr!=NULL; hr=hr->Next()) if (hr->modContext->localData == meshData) break;
		if (hr==NULL) continue;

		mesh = meshData->GetMesh (t);
		if (selLevel>=SL_POLY) af = meshData->TempData(t)->AdjFList();
		if (localSelByVert) ae = meshData->TempData(t)->AdjEList();
		BaseInterface *msci = mesh->GetInterface (MESHSELECTCONVERT_INTERFACE);

		switch (selLevel) {
		case SL_VERTEX:
			nsel = mesh->vertSel;
			for (; hr != NULL; hr=hr->Next()) {
				if (hr->modContext->localData != meshData) continue;
				nsel.Set (hr->hitInfo, invert ? !nsel[hr->hitInfo] : selected);
				if (!all) break;
			}
			meshData->SetVertSel (nsel, this, t);
			break;

		case SL_EDGE:
			if (msci && selByVert) {
				// Use new improved selection conversion:
				BitArray vhit;
				vhit.SetSize(mesh->numVerts);
				for (hr=hitRec; hr!=NULL; hr=hr->Next()) {
					if (meshData != hr->modContext->localData) continue;
					vhit.Set (hr->hitInfo);
					if (!all) break;
				}
				MeshSelectionConverter *pConv = static_cast<MeshSelectionConverter*>(msci);
				pConv->VertexToEdge(*mesh, vhit, nsel);
				if (invert) nsel ^= mesh->edgeSel;
				else {
					if (selected) nsel |= mesh->edgeSel;
					else nsel = mesh->edgeSel & ~nsel;
				}
			} else {
				nsel = mesh->edgeSel;
				for (; hr != NULL; hr=hr->Next()) {
					if (hr->modContext->localData != meshData) continue;
					if (localSelByVert) {
						DWORDTab & list = ae->list[hr->hitInfo];
						for (int i=0; i<list.Count(); i++) {
							MEdge & me = ae->edges[list[i]];
							for (int j=0; j<2; j++) {
								if (me.f[j] == UNDEFINED) continue;
								DWORD ei = mesh->faces[me.f[j]].GetEdgeIndex (me.v[0], me.v[1]);
								if (ei>2) continue;
								ei += me.f[j]*3;
								nsel.Set (ei, invert ? !mesh->edgeSel[ei] : selected);
							}
						}
					} else {
						nsel.Set (hr->hitInfo, invert ? !nsel[hr->hitInfo] : selected);
					}
					if (!all) break;
				}
			}
			meshData->SetEdgeSel (nsel, this, t);
			break;

		case SL_FACE:
			if (msci && selByVert) {
				// Use new improved selection conversion:
				BitArray vhit;
				vhit.SetSize(mesh->numVerts);
				for (hr=hitRec; hr!=NULL; hr=hr->Next()) {
					if (meshData != hr->modContext->localData) continue;
					vhit.Set (hr->hitInfo);
					if (!all) break;
				}
				MeshSelectionConverter *pConv = static_cast<MeshSelectionConverter*>(msci);
				pConv->VertexToFace (*mesh, vhit, nsel);
				if (invert) nsel ^= mesh->faceSel;
				else {
					if (selected) nsel |= mesh->faceSel;
					else nsel = mesh->faceSel & ~nsel;
				}
			} else {
				nsel = mesh->faceSel;
				for (; hr != NULL; hr=hr->Next()) {
					if (hr->modContext->localData != meshData) continue;
					if (localSelByVert) {
						DWORDTab & list = ae->list[hr->hitInfo];
						for (int i=0; i<list.Count(); i++) {
							MEdge & me = ae->edges[list[i]];
							for (int j=0; j<2; j++) {
								if (me.f[j] == UNDEFINED) continue;
								nsel.Set (me.f[j], invert ? !mesh->faceSel[me.f[j]] : selected);
							}
						}
					} else {
						nsel.Set (hr->hitInfo, invert ? !mesh->faceSel[hr->hitInfo] : selected);
					}
					if (!all) break;
				}
			}
			meshData->SetFaceSel (nsel, this, t);
			break;

		case SL_POLY:
		case SL_ELEMENT:
			if (msci) {
				// Use new improved selection conversion:
				MeshSelectionConverter *pConv = static_cast<MeshSelectionConverter*>(msci);
				BitArray fhit;
				if (selByVert) {
					BitArray vhit;
					vhit.SetSize(mesh->numVerts);
					for (hr=hitRec; hr!=NULL; hr=hr->Next()) {
						if (meshData != hr->modContext->localData) continue;
						vhit.Set (hr->hitInfo);
						if (!all) break;
					}
					if (selLevel == SL_ELEMENT) pConv->VertexToElement(*mesh, af, vhit, nsel);
					else pConv->VertexToPolygon(*mesh, af, vhit, nsel, GetPolyFaceThresh(), ignoreVisEdge?true:false);
				} else {
					fhit.SetSize(mesh->numFaces);
					for (hr=hitRec; hr!=NULL; hr=hr->Next()) {
						if (meshData != hr->modContext->localData) continue;
						fhit.Set (hr->hitInfo);
						if (!all) break;
					}
					if (selLevel == SL_ELEMENT) pConv->FaceToElement (*mesh, af, fhit, nsel);
					else pConv->FaceToPolygon (*mesh, af, fhit, nsel, GetPolyFaceThresh(), ignoreVisEdge?true:false);
				}
			} else {
				// Otherwise we'll take the old approach of converting faces to polygons or elements as we go.
				nsel.SetSize (mesh->getNumFaces());
				nsel.ClearAll ();
				for (; hr != NULL; hr=hr->Next()) {
					if (hr->modContext->localData != meshData) continue;
					if (!localSelByVert) {
						if (selLevel==SL_ELEMENT) mesh->ElementFromFace (hr->hitInfo, nsel, af);
						else mesh->PolyFromFace (hr->hitInfo, nsel, GetPolyFaceThresh(), ignoreVisEdge, af);
					} else {
						DWORDTab & list = ae->list[hr->hitInfo];
						for (int i=0; i<list.Count(); i++) {
							MEdge & me = ae->edges[list[i]];
							for (int j=0; j<2; j++) {
								if (me.f[j] == UNDEFINED) continue;
								if (selLevel==SL_ELEMENT) mesh->ElementFromFace (me.f[j], nsel, af);
								else mesh->PolyFromFace (me.f[j], nsel, GetPolyFaceThresh(), ignoreVisEdge, af);
							}
						}
					}
					if (!all) break;
				}
			}
			if (invert) nsel ^= mesh->faceSel;
			else {
				if (selected) nsel |= mesh->faceSel;
				else nsel = mesh->faceSel & ~nsel;
			}
			meshData->SetFaceSel (nsel, this, t);
		}
	}
	LocalDataChanged ();
	nodes.DisposeTemporary ();
}

void EditMeshMod::ClearSelection(int selLevel) {
	if (selLevel==SL_OBJECT) return;
	if (!ip) return;	
	if (sliceMode) return;

	EndExtrude(ip->GetTime());

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ip->ClearCurNamedSelSet();
	BitArray nsel;

	for (int i=0; i<mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;		
		Mesh *mesh = meshData->GetMesh (ip->GetTime());
		switch (selLevel) {
		case SL_VERTEX:
			nsel.SetSize (mesh->getNumVerts());
			nsel.ClearAll ();
			meshData->SetVertSel (nsel, this, ip->GetTime());
			break;
		case SL_FACE:
		case SL_POLY:
		case SL_ELEMENT:
			nsel.SetSize (mesh->getNumFaces());
			nsel.ClearAll ();
			meshData->SetFaceSel (nsel, this, ip->GetTime());
			break;
		case SL_EDGE:
			nsel.SetSize (mesh->getNumFaces()*3);
			nsel.ClearAll ();
			meshData->SetEdgeSel (nsel, this, ip->GetTime());
			break;
		}
	}
	
	nodes.DisposeTemporary();
	LocalDataChanged ();
}

void EditMeshMod::SelectAll(int selLevel) {
	if (selLevel==SL_OBJECT) return;
	if (!ip) return;	
	if (sliceMode) return;

	EndExtrude(ip->GetTime());

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ip->ClearCurNamedSelSet();

	for (int i=0; i<mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if ( !meshData ) continue;
		Mesh *mesh = meshData->GetMesh (ip->GetTime());
		BitArray sel;

		switch ( selLevel ) {
		case SL_VERTEX:	
			sel.SetSize (mesh->numVerts);
			sel.SetAll ();
			meshData->SetVertSel (sel, this, ip->GetTime());
			break;

		case SL_EDGE:
			sel.SetSize (mesh->numFaces*3);
			sel.SetAll ();
			meshData->SetEdgeSel (sel, this, ip->GetTime());
			break;

		default:
			sel.SetSize (mesh->numFaces);
			sel.SetAll ();
			meshData->SetFaceSel (sel, this, ip->GetTime());
			break;
		}
	}

	nodes.DisposeTemporary();
	LocalDataChanged ();
}

void EditMeshMod::InvertSelection(int selLevel) {
	if ( !ip ) return;
	if (selLevel == SL_OBJECT) return;
	if (sliceMode) return;
	EndExtrude(ip->GetTime());

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ip->ClearCurNamedSelSet();

	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if ( !meshData ) continue;		
		Mesh *mesh = meshData->GetMesh (ip->GetTime());
		BitArray sel;

		switch ( selLevel ) {
		case SL_VERTEX:	
			sel = ~mesh->vertSel;
			meshData->SetVertSel (sel, this, ip->GetTime ());
			break;
	
		case SL_EDGE:
			sel = ~mesh->edgeSel;
			meshData->SetEdgeSel (sel, this, ip->GetTime());
			break;

		default:
			sel = ~mesh->faceSel;
			meshData->SetFaceSel (sel, this, ip->GetTime());
			break;
		}
	}
	
	nodes.DisposeTemporary();
	LocalDataChanged ();
}

void EditMeshMod::InvalidateDistances () {
	if ( !ip ) return;
	if (selLevel == SL_OBJECT) return;
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if ( !meshData ) continue;
		MeshTempData *emt = meshData->tempData;
		if (!emt) continue;
		emt->InvalidateDistances ();
	}
	nodes.DisposeTemporary();
}

class InvalidateAREnumProc : public ModContextEnumProc
{
public:
	BOOL proc (ModContext *mc)
	{
		EditMeshData *meshData = (EditMeshData *) mc->localData;
		if (!meshData) return true;
		MeshTempData *emt = meshData->tempData;
		if (!emt) return true;
		emt->InvalidateAffectRegion ();
		return true;
	}
};

static InvalidateAREnumProc theInvalidateAREnumProc;

void EditMeshMod::InvalidateAffectRegion () {
	if ( !ip ) return;
	if (selLevel == SL_OBJECT) return;
	EnumModContexts (&theInvalidateAREnumProc);
	NotifyDependents (FOREVER, PART_SELECT, REFMSG_CHANGE);
}

DWORD EditMeshMod::GetSelLevel () {
	switch (selLevel) {
	case SL_OBJECT: return IMESHSEL_OBJECT;
	case SL_VERTEX: return IMESHSEL_VERTEX;
	case SL_EDGE: return IMESHSEL_EDGE;
	}
	return IMESHSEL_FACE;
}

void EditMeshMod::SetSelLevel (DWORD sl) {
	switch (sl) {
	case IMESHSEL_OBJECT:
		selLevel = SL_OBJECT;
		break;
	case IMESHSEL_VERTEX:
		selLevel = SL_VERTEX;
		break;
	case IMESHSEL_EDGE:
		selLevel = SL_EDGE;
		break;
	case IMESHSEL_FACE:
		if (selLevel < SL_FACE) selLevel = SL_POLY;	// don't change if we're already in a face mode.
		break;
	}
	if (ip) ip->SetSubObjectLevel(selLevel);
}

void EditMeshMod::LocalDataChanged () {
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	if ( !ip ) return;	
	ip->ClearCurNamedSelSet();
	InvalidateSurfaceUI ();
	InvalidateNumberSelected ();
	UpdateNamedSelDropDown ();
}

void EditMeshMod::LocalDataChanged (DWORD parts) {
	NotifyDependents(FOREVER, parts, REFMSG_CHANGE);
	if (!ip) return;
	ip->ClearCurNamedSelSet();
	InvalidateSurfaceUI ();
	if (parts & SELECT_CHANNEL) InvalidateNumberSelected ();
}


//------Named Selection Sets-----------------------------

// Strategy:
// We need to keep track of named selection sets simultaneously at the
// modifier and localmoddata levels.  Therefore we have the namedSel and ids
// tables in EditMeshMod, plus the GenericNamedSelSetLists in each EditMeshData,
// with its individual tables of names, sets, and ids.  The ids appear to be used 
// instead of names in the localmoddatas -- this saves space and comparison time.

//#define NAMED_SELECTION_DEBUG_PRINT

// Relevant restore objects:
class EMAppendSetRestore : public RestoreObj {
public:
	BitArray set;		
	DWORD id;
	TSTR name;
	GenericNamedSelSetList *setList;		

	EMAppendSetRestore(GenericNamedSelSetList *sl, DWORD idd, TSTR & n) { setList = sl; id=idd; name = n; }
	void Restore(int isUndo) {
		set = *setList->GetSet (id);
		setList->RemoveSet(id);
	}
	void Redo() { setList->AppendSet(set,id,name); }
			
	TSTR Description() { return TSTR(_T("Append Set")); }
};

class EMAppendSetNameRestore : public RestoreObj {
public:		
	TSTR name;
	DWORD id;
	EditMeshMod *et;
	Tab<TSTR*> *sets;
	Tab<DWORD> *ids;

	EMAppendSetNameRestore(EditMeshMod *e,Tab<TSTR*> *s,Tab<DWORD> *i) 
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

class EMDeleteSetRestore : public RestoreObj {
public:
	BitArray set;
	DWORD id;
	TSTR name;
	GenericNamedSelSetList *setList;

	EMDeleteSetRestore(GenericNamedSelSetList *sl,DWORD i, TSTR & n) {
		setList = sl;
		id = i;
		name = n;
		BitArray *ptr = setList->GetSet(id);
		if (ptr) set = *ptr;
	}
	void Restore(int isUndo) { setList->AppendSet(set,id,name); }
	void Redo() { setList->RemoveSet(id); }
	TSTR Description() {return TSTR(_T("Delete Named Set"));}
};

class EMDeleteSetNameRestore : public RestoreObj {
public:
	TSTR name;
	DWORD id;
	EditMeshMod *et;
	Tab<TSTR*> *sets;
	Tab<DWORD> *ids;

	EMDeleteSetNameRestore(Tab<TSTR*> *s, EditMeshMod *e,Tab<DWORD> *i, DWORD id) {
		sets = s;
		et = e;		
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
		if (et->ip) et->ip->NamedSelSetListChanged();
	}
			
	TSTR Description() {return TSTR(_T("Delete Set Name"));}
};

class EMSetNameRestore : public RestoreObj {
public:
	TSTR undo, redo;
	DWORD id;
	Tab<TSTR*> *sets;
	Tab<DWORD> *ids;
	EditMeshMod *et;
	EMSetNameRestore(Tab<TSTR*> *s, EditMeshMod *e,Tab<DWORD> *i,DWORD id) {
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

void EditMeshMod::UpdateSetNames () {
	if ( !ip ) return;
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	for (int i=0; i<mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
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

			GenericNamedSelSetList & gnsl = meshData->selSet[nsl];
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

void EditMeshMod::ClearSetNames() {
	for (int i=0; i<3; i++) {
		for (int j=0; j<namedSel[i].Count(); j++) {
			delete namedSel[i][j];
			namedSel[i][j] = NULL;
		}
	}
}

int EditMeshMod::FindSet(TSTR &setName, int nsl) {
	for (int i=0; i<namedSel[nsl].Count(); i++) if (setName == *namedSel[nsl][i]) return i;
	return -1;
}

DWORD EditMeshMod::AddSet(TSTR &setName, int nsl) {
	DWORD id = 0;
	TSTR *name = new TSTR(setName);
	namedSel[nsl].Append(1,&name);
	// Find an empty id to assign to this set.
	BOOL found = FALSE;
	while (!found) {
		found = TRUE;
		for (int i=0; i<ids[nsl].Count(); i++) {
			if (ids[nsl][i]!=id) continue;
			id++;
			found = FALSE;
		}
	}
	ids[nsl].Append(1,&id);
	return id;
}

void EditMeshMod::RemoveSet (TSTR &setName, int nsl) {
#ifdef NAMED_SELECTION_DEBUG_PRINT
	DebugPrint ("EMM: RemoveSet (%s, %d)\n", setName, nsl);
#endif
	int i;
	if ((i=FindSet(setName,nsl)) < 0) return;
	delete namedSel[nsl][i];
	namedSel[nsl].Delete(i,1);
	ids[nsl].Delete(i,1);
}

void EditMeshMod::ActivateSubSelSet(TSTR &setName) {
#ifdef NAMED_SELECTION_DEBUG_PRINT
	DebugPrint ("EMM: ActivateSubSelSet (%s)\n", setName);
#endif
	if (selLevel == SL_OBJECT) return;
	int nsl = namedSetLevel[selLevel];
	int index = FindSet (setName, nsl);
	if (index<0 || !ip) return;
	EndExtrude(ip->GetTime());	

	theHold.Begin ();
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;		
		meshData->BeginEdit (ip->GetTime());
		BitArray *set = meshData->selSet[nsl].GetSet (ids[nsl][index]);
		if (set) meshData->SetSel (nsl, *set, this, ip->GetTime());
	}
	theHold.Accept (GetString (IDS_DS_SELECT));
	nodes.DisposeTemporary();
	LocalDataChanged ();
	ip->RedrawViews(ip->GetTime());
}

void EditMeshMod::NewSetFromCurSel(TSTR &setName) {
	if (selLevel==SL_OBJECT) return;
	if (!ip) return;

	DWORD id = -1;
	int nsl = namedSetLevel[selLevel];
	int index = FindSet (setName, nsl);
	if (index<0) id = AddSet (setName, nsl);
	else id = ids[nsl][index];
	EndExtrude(ip->GetTime());

	int i;
#ifdef NAMED_SELECTION_DEBUG_PRINT
	DebugPrint ("EditMeshMod::NewSetFromCurSel (%s) - ", setName);
	switch (nsl) {
	case NS_VERTEX: DebugPrint ("Vertex Level\n"); break;
	case NS_EDGE: DebugPrint ("Edge Level\n"); break;
	default: DebugPrint ("Face Level\n"); break;
	}
	DebugPrint ("Modifier list of name sets:\n");
	for (i=0; i<namedSel[nsl].Count(); i++) {
		DebugPrint ("Name: %s; ID: %d\n", *(namedSel[nsl][i]), ids[nsl][i]);
	}
#endif

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	for (i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		meshData->BeginEdit (ip->GetTime());
		BitArray *set;
		if (index>=0 && (set = meshData->selSet[nsl].GetSet(id))) *set = meshData->GetSel(nsl);
		else meshData->selSet[nsl].AppendSet (meshData->GetSel(nsl), id, setName);
#ifdef NAMED_SELECTION_DEBUG_PRINT
		DebugPrint ("\nModContext %d list of name sets:\n", i);
		for (int j=0; j<meshData->selSet[nsl].Count(); j++) {
			DebugPrint ("Name: %s; ID: %d\n", *(meshData->selSet[nsl].names[j]), meshData->selSet[nsl].ids[j]);
		}
#endif
	}
	nodes.DisposeTemporary();
}

void EditMeshMod::RemoveSubSelSet(TSTR &setName) {
	//DebugPrint ("EMM: RemoveSubSelSet (%s)\n", setName);
	int nsl = namedSetLevel[selLevel];
	int index = FindSet (setName, nsl);
	if (index<0 || !ip) return;
	EndExtrude(ip->GetTime());
	
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	DWORD id = ids[nsl][index];
	for (int i=0; i<mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (theHold.Holding ()) theHold.Put (new EMDeleteSetRestore (&(meshData->selSet[nsl]), id, setName));
		meshData->selSet[nsl].RemoveSet(id);
	}
	RemoveSet (setName, nsl);
	ip->ClearCurNamedSelSet();
	nodes.DisposeTemporary();
}

void EditMeshMod::SetupNamedSelDropDown () {
	//DebugPrint ("EMM: SetupNamedSelDropDown\n");
	if (selLevel == SL_OBJECT) return;
	ip->ClearSubObjectNamedSelSets();
	int nsl = namedSetLevel[selLevel];
	for (int i=0; i<namedSel[nsl].Count(); i++) ip->AppendSubObjectNamedSelSet(*namedSel[nsl][i]);
}

void EditMeshMod::UpdateNamedSelDropDown () {
	if (!ip) return;
	if (selLevel == SL_OBJECT) {
		ip->ClearCurNamedSelSet ();
		return;
	}
	// See if this selection matches a named set
	DWORD nsl = namedSetLevel[selLevel];
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts (mcList, nodes);
	BitArray nselmatch;
	nselmatch.SetSize (namedSel[nsl].Count());
	nselmatch.SetAll ();
	int nd, i, foundone=FALSE;
	for (nd=0; nd<mcList.Count(); nd++) {
		EditMeshData *d = (EditMeshData *) mcList[nd]->localData;
		if (!d) continue;
		foundone = TRUE;
		for (i=0; i<nselmatch.GetSize(); i++) {
			if (!nselmatch[i]) continue;
			if (!(*(d->selSet[nsl].sets[i]) == d->GetSel(nsl))) nselmatch.Clear(i);
		}
		if (nselmatch.NumberSet () == 0) break;
	}
	if (foundone && nselmatch.NumberSet ()) {
		for (i=0; i<nselmatch.GetSize(); i++) if (nselmatch[i]) break;
		ip->SetCurNamedSelSet (*(namedSel[nsl][i]));
	} else ip->ClearCurNamedSelSet ();
}

int EditMeshMod::NumNamedSelSets () {
	//DebugPrint ("EMM: NumNamedSelSets()=");
	int nsl = namedSetLevel[selLevel];
	int ret=namedSel[nsl].Count();
	//DebugPrint ("%d\n", ret);
	return ret;
}

TSTR EditMeshMod::GetNamedSelSetName (int i) {
	//DebugPrint ("EMM: GetNamedSelSetName(%d)=", i);
	int nsl = namedSetLevel[selLevel];
	//DebugPrint ("%s\n", *namedSel[nsl][i]);
	return *namedSel[nsl][i];
}

void EditMeshMod::SetNamedSelSetName (int i, TSTR & newName) {
	//DebugPrint ("EMM: SetNamedSelSetName (%d, %s)\n", i, newName);
	int nsl = namedSetLevel[selLevel];

	if (ip) {
		ModContextList mcList;
		INodeTab nodes;
		ip->GetModContexts(mcList,nodes);
		for (int i=0; i<mcList.Count(); i++) {
			EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
			if (!meshData) continue;
			meshData->selSet[nsl].RenameSet (*(namedSel[nsl][i]), newName);
		}
		nodes.DisposeTemporary ();
	}

	*namedSel[nsl][i] = newName;
}

void EditMeshMod::NewSetByOperator (TSTR &newName, Tab<int> &sets, int op) {
	//DebugPrint ("EMM: NewSetByOperator (%s, %d, %d)\n", newName, sets.Count(), op);
	int nsl = namedSetLevel[selLevel];
	DWORD id = AddSet(newName, nsl);
	BOOL delSet = TRUE;

	int i;
#ifdef NAMED_SELECTION_DEBUG_PRINT
	DebugPrint ("EditMeshMod::NewSetByOperator (%s) - ", newName);
	switch (nsl) {
	case NS_VERTEX: DebugPrint ("Vertex Level\n"); break;
	case NS_EDGE: DebugPrint ("Edge Level\n"); break;
	default: DebugPrint ("Face Level\n"); break;
	}
	DebugPrint ("Modifier list of name sets:\n");
	for (i=0; i<namedSel[nsl].Count(); i++) {
		DebugPrint ("Name: %s; ID: %d\n", *(namedSel[nsl][i]), ids[nsl][i]);
	}
#endif

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	for (i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData *)mcList[i]->localData;
		if (!meshData) continue;
		BitArray bits;
		GenericNamedSelSetList & setList = meshData->selSet[nsl];
		bits = setList[sets[0]];
		for (int i=1; i<sets.Count(); i++) {
			switch (op) {
			case NEWSET_MERGE:
				bits |= setList[sets[i]];
				break;
			case NEWSET_INTERSECTION:
				bits &= setList[sets[i]];
				break;
			case NEWSET_SUBTRACT:
				bits &= ~(setList[sets[i]]);
				break;
			}
		}
		if (bits.NumberSet()) delSet = FALSE;

		if (!delSet) setList.AppendSet (bits, id, newName);
#ifdef NAMED_SELECTION_DEBUG_PRINT
		DebugPrint ("\nModContext %d list of name sets:\n", i);
		for (int j=0; j<setList.Count(); j++) {
			DebugPrint ("Name: %s; ID: %d\n", *(setList.names[j]), setList.ids[j]);
		}
#endif
	}
	if (delSet) RemoveSubSelSet(newName);
}

void EditMeshMod::NSCopy() {
	if (selLevel == SL_OBJECT) return;
	int index = SelectNamedSet();
	if (index<0) return;
	if (!ip) return;

	int nsl = namedSetLevel[selLevel];
	MeshNamedSelClip *clip = new MeshNamedSelClip(*namedSel[nsl][index]);

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;

		GenericNamedSelSetList & setList = meshData->selSet[nsl];
		BitArray *bits = new BitArray(*setList.sets[index]);
		clip->sets.Append(1,&bits);
	}
	SetMeshNamedSelClip(clip, namedClipLevel[selLevel]);

	// Enable the paste button
	if (hSel) {
		ICustButton *but;
		but = GetICustButton(GetDlgItem(hSel, IDC_EM_PASTENS));
		but->Enable();
		ReleaseICustButton(but);
	}
}

void EditMeshMod::NSPaste() {
	if (selLevel==SL_OBJECT) return;
	int nsl = namedSetLevel[selLevel];
	MeshNamedSelClip *clip = GetMeshNamedSelClip(namedClipLevel[selLevel]);
	if (!clip) return;
	TSTR name = clip->name;
	if (!GetUniqueSetName(name)) return;

	ModContextList mcList;
	INodeTab nodes;
	theHold.Begin();

	DWORD id = AddSet (name, nsl);
	if (theHold.Holding()) theHold.Put(new EMAppendSetNameRestore(this, &namedSel[nsl], &ids[nsl]));	

	ip->GetModContexts(mcList,nodes);
	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;

		GenericNamedSelSetList & setList = meshData->selSet[nsl];

		if (i>=clip->sets.Count()) {
			BitArray bits;
			setList.InsertSet(bits,id,name);
		} else setList.InsertSet(*clip->sets[i],id,name);
		if (theHold.Holding()) theHold.Put(new EMAppendSetRestore (&setList, id, name));		
	}	
	
	ActivateSubSelSet(name);
	ip->SetCurNamedSelSet(name);
	theHold.Accept(GetString (IDS_TH_PASTE_NAMED_SEL));
	SetupNamedSelDropDown();
}

static INT_PTR CALLBACK PickSetNameDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static TSTR *name;
	ICustEdit *edit;
	TCHAR buf[256];

	switch (msg) {
	case WM_INITDIALOG:
		name = (TSTR*)lParam;
		edit =GetICustEdit(GetDlgItem(hWnd,IDC_SET_NAME));
		edit->SetText(*name);
		ReleaseICustEdit(edit);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			edit =GetICustEdit(GetDlgItem(hWnd,IDC_SET_NAME));
			edit->GetText(buf,256);
			*name = TSTR(buf);
			ReleaseICustEdit(edit);
			EndDialog(hWnd,1);
			break;

		case IDCANCEL:
			EndDialog(hWnd,0);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

BOOL EditMeshMod::GetUniqueSetName(TSTR &name) {
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

int EditMeshMod::SelectNamedSet() {
	Tab<TSTR*> &setList = namedSel[namedSetLevel[selLevel]];
	if (!ip) return FALSE;
	return DialogBoxParam (hInstance, MAKEINTRESOURCE(IDD_SEL_NAMEDSET),
		ip->GetMAXHWnd(), PickSetDlgProc, (LPARAM)&setList);
}

void EditMeshMod::ActivateSubobjSel(int level, XFormModes& modes ) {	
	ModContextList mcList;
	INodeTab nodes;
	int old = selLevel;

	if (!ip) return;
	ip->GetModContexts(mcList,nodes);

	// if we're in sub object selection then we want del key input.
	if (selLevel==0 && level!=0) {
		delEvent.SetEditMeshMod(this);
		ip->RegisterDeleteUser(&delEvent);
	}
	if (selLevel!=0 && level==0) {
		ip->UnRegisterDeleteUser(&delEvent);
	}
	if ((selLevel != level) && ((level<SL_FACE) || (selLevel<SL_FACE))) {
		ExitAllCommandModes (level == SL_OBJECT, level==SL_OBJECT);
	}

	selLevel = level;
	
	if (level!=SL_OBJECT) {
		modes = XFormModes(moveMode,rotMode,nuscaleMode,uscaleMode,squashMode,selectMode);
	}

	if (selLevel != old) {
		// Modify the caches to reflect the new sel level.
		for ( int i = 0; i < mcList.Count(); i++ ) {
			EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
			if ( !meshData ) continue;		
		
			if ( meshData->MeshCached(ip->GetTime()) ) {
				Mesh *mesh = meshData->GetMesh (ip->GetTime());
				mesh->dispFlags = 0;
				mesh->SetDispFlag(levelDispFlags[selLevel]);
				mesh->selLevel = meshLevel[selLevel];
				if ((mesh->selLevel == MESH_FACE) && (inBuildFace)) {
					mesh->SetDispFlag (DISP_VERTTICKS|DISP_SELVERTS);
				}
			}
			meshData->Invalidate(PART_SELECT);
		}

		NotifyDependents (FOREVER, PART_SELECT|PART_SUBSEL_TYPE|PART_DISPLAY, REFMSG_CHANGE);	
		ip->PipeSelLevelChanged();
	}
	
	// Setup named selection sets	
	SetupNamedSelDropDown();
	UpdateNamedSelDropDown();

	nodes.DisposeTemporary();
	if (hSel) RefreshSelType ();
	ip->RedrawViews (ip->GetTime());
}

int EditMeshMod::SubObjectIndex(HitRecord *hitRec) {
	EditMeshData *meshData = (EditMeshData*)hitRec->modContext->localData;

	if ( !meshData ) return 0;
	if ( !ip ) return 0;
	TimeValue t = ip->GetTime();
	FaceClusterList *fc;
	EdgeClusterList *ec;
	DWORD id;

	switch ( selLevel ) {
	case SL_VERTEX:
		// Changed SubObjectIndex to mean the actual index of the vertex,
		// not the nth selected.
		return hitRec->hitInfo;
	
	case SL_FACE:
	case SL_POLY:
	case SL_ELEMENT:
		fc = meshData->TempData(t)->FaceClusters();
		id = (*fc)[hitRec->hitInfo];
		if (id!=UNDEFINED) return id;
		else return 0;

	case SL_EDGE:
		ec = meshData->TempData(t)->EdgeClusters();
		id = (*ec)[hitRec->hitInfo];
		if (id!=UNDEFINED) return id;
		else return 0;

	default:
		return 0;
	}
}

void EditMeshMod::GetSubObjectCenters (SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc) {
	Matrix3 tm = node->GetObjectTM(t);	
	
	if (sliceMode) {
		cb->Center (sliceCenter*tm, 0);
		return;
	}

	MaxAssert(ip);

	if (!mc->localData) return;
	EditMeshData *meshData = (EditMeshData*)mc->localData;		
	Mesh *mesh = meshData->GetMesh (t);
	int i;
	Point3 cent = Point3(0,0,0);
	
	switch (selLevel) {
	case SL_VERTEX:
		int ct;
		for (i=0, ct=0; i<mesh->getNumVerts(); i++) {
			if (!mesh->vertSel[i]) continue;
			cent += mesh->verts[i];
			ct++;
		}
		if (ct) {
			cent /= float(ct);	
			cb->Center(cent*tm,0);
		}
		break;

	case SL_EDGE:
	case SL_FACE:
	case SL_POLY:
	case SL_ELEMENT:
		Tab<Point3> *centers;
		centers = meshData->TempData(t)->ClusterCenters(meshLevel[selLevel]);
		for (i=0; i<centers->Count(); i++) cb->Center((*centers)[i]*tm,i);
		break;

	default:
		cb->Center(tm.GetTrans(),0);
		break;
	}		
}


void EditMeshMod::GetSubObjectTMs (SubObjAxisCallback *cb,
								   TimeValue t,INode *node,ModContext *mc) {
	Matrix3 tm;	
	Matrix3 otm = node->GetObjectTM(t);

	if (sliceMode) {
		Matrix3 rotMatrix(1);
		sliceRot.MakeMatrix (rotMatrix);
		rotMatrix.SetTrans (sliceCenter);
		rotMatrix *= otm;
		cb->TM (rotMatrix, 0);
		return;
	}

	if (!mc->localData ) return;
	EditMeshData *meshData = (EditMeshData*)mc->localData;
	Mesh *mesh = meshData->GetMesh (t);
	int i,j;

	switch (selLevel) {
	case SL_VERTEX:
		if (mesh->vertSel.NumberSet()==0) return;
		if (ip->GetCommandMode()->ID()==CID_SUBOBJMOVE) {
			Tab<Point3> *vnorms;
			vnorms = meshData->TempData(t)->VertexNormals();

			for (i=0,j=0; i<vnorms->Count(); i++) {
				if (!mesh->vertSel[i]) continue;
				Point3 n = (*vnorms)[i];
				n = Normalize(n);
				MatrixFromNormal(n,tm);
				tm = tm * otm;
				tm.SetTrans(mesh->verts[i]*otm);
				cb->TM(tm, j++);
			}
		} else {
			Point3 norm;
			Point3 cent;
			int ct;
			cent = Point3(0,0,0);
			ct=0;

			// Comute average face normal
			norm = AverageSelVertNormal(*mesh);

			// Compute center of selection
			for (i=0; i<mesh->getNumVerts(); i++) {
				if (!mesh->vertSel[i]) continue;
				cent += mesh->verts[i];
				ct++;
			}
			if (ct) cent /= float(ct);
			cent = cent * otm;
			norm = Normalize(VectorTransform(otm,norm));
			Matrix3 mat;
			MatrixFromNormal(norm,mat);
			mat.SetTrans(cent);
			cb->TM(mat,0);
		}
		break;

	case SL_EDGE:
	case SL_FACE:
	case SL_POLY:
	case SL_ELEMENT:
		Tab<Point3> *norms;
		norms = meshData->TempData(t)->ClusterNormals(meshLevel[selLevel]);
		Tab<Point3> *centers;
		centers = meshData->tempData->ClusterCenters(meshLevel[selLevel]);
		for (int i=0; i<norms->Count(); i++) {
			Point3 n = VectorTransform(otm,(*norms)[i]);
			n = Normalize(n);
			MatrixFromNormal(n,tm);
			tm.SetTrans((*centers)[i]*otm);
			cb->TM(tm,i);
		}
		break;
	}
}


void EditMeshMod::DeleteMeshDataTempData()
	{
	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;		
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if ( !meshData ) continue;				
		if ( meshData->tempData ) {
			delete meshData->tempData;
			}
		meshData->tempData = NULL;
		}
	nodes.DisposeTemporary();
	}


void EditMeshMod::CreateMeshDataTempData()
	{
	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;		
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if ( !meshData ) continue;
		meshData->mod = this;
		if ( meshData->tempData ) continue;
		meshData->tempData = new MeshTempData ();
	}
	
	nodes.DisposeTemporary();
}

bool CheckNodeSelection (Interface *ip, INode *inode) {
	if (!ip) return FALSE;
	if (!inode) return FALSE;
	int i, nct = ip->GetSelNodeCount();
	for (i=0; i<nct; i++) if (ip->GetSelNode (i) == inode) return TRUE;
	return FALSE;
}

int EditMeshMod::HitTest(TimeValue t, INode* inode, int type, int crossing, 
		int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc) {
	Interval valid;
	int savedLimits,res = 0;
	GraphicsWindow *gw = vpt->getGW();
	HitRegion hr;

	// Setup GW
	MakeHitRegion(hr,type, crossing, pickBoxSize, p);
	gw->setHitRegion(&hr);
	Matrix3 mat = inode->GetObjectTM(t);
	gw->setTransform(mat);	
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
	if (ignoreBackfaces) gw->setRndLimits(gw->getRndLimits() |  GW_BACKCULL);
	else gw->setRndLimits(gw->getRndLimits() & ~GW_BACKCULL);
	gw->clearHitCode();

	if (sliceMode && CheckNodeSelection (ip, inode)) {
		gw->setTransform (inode->GetObjectTM(t) * (mc->tm?Inverse(*mc->tm):Matrix3(1)));
		Point3 rp[5];
		Matrix3 rotMatrix;
		sliceRot.MakeMatrix (rotMatrix);
		rotMatrix.SetTrans (sliceCenter);
		rp[0] = Point3(-sliceSize,-sliceSize,0.0f)*rotMatrix;
		rp[1] = Point3(-sliceSize,sliceSize,0.0f)*rotMatrix;
		rp[2] = Point3(sliceSize,sliceSize,0.0f)*rotMatrix;
		rp[3] = Point3(sliceSize,-sliceSize,0.0f)*rotMatrix;
		gw->polyline (4, rp, NULL, NULL, TRUE, NULL);
		if (gw->checkHitCode()) {
			vpt->LogHit (inode, mc, gw->getHitDistance(), 0, NULL);
			res = 1;
		}
		gw->setRndLimits (savedLimits);
		return res;
	}

	int localSelByVert = selByVert;
	if (cutEdgeMode == ip->GetCommandMode()) localSelByVert = FALSE;
	if (turnEdgeMode == ip->GetCommandMode()) localSelByVert = FALSE;
	if (divideEdgeMode == ip->GetCommandMode()) localSelByVert = FALSE;
	if (divideFaceMode == ip->GetCommandMode()) localSelByVert = FALSE;

	if ( mc->localData ) {
		EditMeshData *meshData = (EditMeshData*)mc->localData;
		Mesh *mesh = meshData->GetMesh (ip->GetTime());
		SubObjHitList hitList;
		MeshSubHitRec *rec;
		
		DWORD hitLev = hitLevel[selLevel];
		if (inBuildFace || localSelByVert) hitLev = SUBHIT_VERTS;
		if (inCutEdge) hitLev = SUBHIT_EDGES;
		if (hitLev == SUBHIT_VERTS) {
			BitArray oldHide;
			if (ignoreBackfaces) {
				BOOL flip = mat.Parity();
				oldHide = mesh->vertHide;
				BitArray faceBack;
				faceBack.SetSize (mesh->getNumFaces());
				faceBack.ClearAll ();
				for (int i=0; i<mesh->getNumFaces(); i++) {
					DWORD *vv = mesh->faces[i].v;
					IPoint3 A[3];
					for (int j=0; j<3; j++) gw->wTransPoint (&(mesh->verts[vv[j]]), &(A[j]));
					IPoint3 d1 = A[1] - A[0];
					IPoint3 d2 = A[2] - A[0];
					if (flip) {
						if ((d1^d2).z > 0) continue;
					} else {
						if ((d1^d2).z < 0) continue;
					}
					for (j=0; j<3; j++) mesh->vertHide.Set (vv[j]);
					faceBack.Set (i);
				}
				for (i=0; i<mesh->getNumFaces(); i++) {
					if (faceBack[i]) continue;
					DWORD *vv = mesh->faces[i].v;
					for (int j=0; j<3; j++) mesh->vertHide.Clear (vv[j]);
				}
				mesh->vertHide |= oldHide;
			}
			DWORD thisFlags = flags | hitLev;
			if ((selLevel != SL_VERTEX) && localSelByVert) thisFlags |= SUBHIT_USEFACESEL;
			res = mesh->SubObjectHitTest(gw, gw->getMaterial(), &hr, thisFlags, hitList);
			if (ignoreBackfaces) mesh->vertHide = oldHide;
		} else {
			res = mesh->SubObjectHitTest(gw, gw->getMaterial(), &hr, flags|hitLev, hitList);
		}

		rec = hitList.First();
		while (rec) {
			vpt->LogHit(inode,mc,rec->dist,rec->index,NULL);
			rec = rec->Next();
		}
	}

	gw->setRndLimits(savedLimits);	
	return res;
}

int EditMeshMod::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc) {	
	if (!ip) return 0;
	if (!GetFlag (EM_EDITING)) return 0;
	// Set up GW
	GraphicsWindow *gw = vpt->getGW();
	int savedLimits = gw->getRndLimits();
	gw->setRndLimits ((savedLimits & ~GW_ILLUM) | GW_ALL_EDGES);
	Matrix3 tm = inode->GetObjectTM(t) * (mc->tm?Inverse(*mc->tm):Matrix3(1));
	gw->setTransform(tm);

	if (sliceMode && CheckNodeSelection (ip, inode)) {
		// Draw rectangle representing slice plane.
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));

		Point3 rp[5];
		Matrix3 rotMatrix;
		sliceRot.MakeMatrix (rotMatrix);
		rotMatrix.SetTrans (sliceCenter);
		rp[0] = Point3(-sliceSize,-sliceSize,0.0f)*rotMatrix;
		rp[1] = Point3(-sliceSize,sliceSize,0.0f)*rotMatrix;
		rp[2] = Point3(sliceSize,sliceSize,0.0f)*rotMatrix;
		rp[3] = Point3(sliceSize,-sliceSize,0.0f)*rotMatrix;
		gw->polyline (4, rp, NULL, NULL, TRUE, NULL);
	}

	if (inBuildFace && CheckNodeSelection (ip, inode)
		&& (mc->localData == createFaceMode->proc.meshData) && mc->localData) {
		tm = inode->GetObjectTM(t);
		gw->setTransform(tm);
		gw->setColor (LINE_COLOR, GetSubSelColor());
		createFaceMode->proc.DrawEstablishedFace (gw);
	}

	if (ip->GetShowEndResult() && mc->localData && selLevel) {
		tm = inode->GetObjectTM(t);
		gw->setTransform(tm);
		// We need to draw a "gizmo" version of the mesh:
		Point3 colSel=GetSubSelColor();
		Point3 colTicks=GetUIColor (COLOR_VERT_TICKS);
		Point3 colGiz=GetUIColor(COLOR_GIZMOS);
		Point3 colGizSel=GetUIColor(COLOR_SEL_GIZMOS);
		gw->setColor (LINE_COLOR, colGiz);
		EditMeshData *meshData = (EditMeshData*)mc->localData;
		Mesh *mesh = meshData->GetMesh (ip->GetTime());
		AdjEdgeList *ae = meshData->TempData(ip->GetTime())->AdjEList();
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
				if (selLevel < SL_EDGE) continue;
				if (selLevel > SL_FACE) continue;
				es[0] = GW_EDGE_INVIS;
			}

#ifdef MESH_CAGE_BACKFACE_CULLING
			if (backCull && fBackfacing[me.f[0]] && ((me.f[1] == UNDEFINED) || fBackfacing[me.f[1]]))
				continue;
#endif

			if (selLevel == SL_EDGE) {
				if (ae->edges[i].Selected (mesh->faces, meshData->GetEdgeSel())) gw->setColor (LINE_COLOR, colGizSel);
				else gw->setColor (LINE_COLOR, colGiz);
			}
			if (selLevel >= SL_FACE) {
				if (ae->edges[i].AFaceSelected (meshData->GetFaceSel())) gw->setColor (LINE_COLOR, colGizSel);
				else gw->setColor (LINE_COLOR, colGiz);
			}
			rp[0] = mesh->verts[me.v[0]];
			rp[1] = mesh->verts[me.v[1]];
			gw->polyline (2, rp, NULL, NULL, FALSE, es);
		}
		if ((selLevel == SL_VERTEX) || (ip->GetCommandMode() == createFaceMode)) {
			float *ourvw = affectRegion ? meshData->TempData(ip->GetTime())->VSWeight
				(useEdgeDist, edgeIts, arIgBack, falloff, pinch, bubble)->Addr(0) : NULL;
			for (i=0; i<mesh->numVerts; i++) {
				if (meshData->mdelta.vhide[i]) continue;

#ifdef MESH_CAGE_BACKFACE_CULLING
				if (backCull && vBackfacing[i]) continue;
#endif

				if (meshData->mdelta.vsel[i]) gw->setColor (LINE_COLOR, colSel);
				else {
					if (ourvw) gw->setColor (LINE_COLOR, SoftSelectionColor(ourvw[i]));
					else gw->setColor (LINE_COLOR, colTicks);
				}

				if(getUseVertexDots()) gw->marker (&(mesh->verts[i]), VERTEX_DOT_MARKER(getVertexDotType()));
				else gw->marker (&(mesh->verts[i]), PLUS_SIGN_MRKR);
			}
		}
	}
	gw->setRndLimits(savedLimits);
	return 0;	
}

void EditMeshMod::GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc) {
	if (!ip) return;
	box.Init();
	Matrix3 tm = inode->GetObjectTM(t) * (mc->tm?Inverse(*mc->tm):Matrix3(1));
	if (sliceMode && CheckNodeSelection (ip, inode)) {
		Matrix3 rotMatrix;
		sliceRot.MakeMatrix (rotMatrix);
		rotMatrix.SetTrans (sliceCenter);
		rotMatrix *= tm;
		box += Point3(-sliceSize,-sliceSize,0.0f)*rotMatrix;
		box += Point3(-sliceSize,sliceSize,0.0f)*rotMatrix;
		box += Point3(sliceSize,sliceSize,0.0f)*rotMatrix;
		box += Point3(sliceSize,-sliceSize,0.0f)*rotMatrix;
	}

	if (ip->GetShowEndResult() && mc->localData && selLevel) {
		// We need to draw a "gizmo" version of the mesh:
		Matrix3 tm = inode->GetObjectTM(t);
		EditMeshData *meshData = (EditMeshData*)mc->localData;
		if (meshData->MeshCached (ip->GetTime())) {
			Mesh *mesh = meshData->mesh;
			AdjEdgeList *ae = meshData->TempData(ip->GetTime())->AdjEList();
			int i, ect = ae->edges.Count();
			for (i=0; i<ect; i++) {
				box += tm*mesh->verts[ae->edges[i].v[0]];
				box += tm*mesh->verts[ae->edges[i].v[1]];
			}
		}
	}
}

//---------------------------------------------------------------------
// IO

#define SEL_LEVEL_CHUNK 			0x2800
#define NAMEDVSEL_NAMES_CHUNK		0x2805
#define NAMEDFSEL_NAMES_CHUNK		0x2806
#define NAMEDESEL_NAMES_CHUNK		0x2807
#define NAMEDSEL_STRING_CHUNK		0x2809
#define NAMEDSEL_ID_CHUNK			0x2810
#define AR_CHUNK 0x2820
#define FALLOFF_CHUNK 0x2822
#define PINCH_CHUNK 0x2824
#define BUBBLE_CHUNK 0x2826
#define EDIST_CHUNK 0x2828
#define EDGE_ITS_CHUNK 0x282a
#define IG_BACK_CHUNK 0x282c
#define EMOD_FLAGS_CHUNK 0x2830

static int namedSelID[] = {NAMEDVSEL_NAMES_CHUNK,NAMEDFSEL_NAMES_CHUNK,NAMEDESEL_NAMES_CHUNK};

IOResult EditMeshMod::Save(ISave *isave) {
	Modifier::Save(isave);
	Interval valid;
	ULONG nb;	
	short sl = selLevel;
	isave->BeginChunk(SEL_LEVEL_CHUNK);
	isave->Write(&sl,sizeof(short),&nb);
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

	isave->BeginChunk (AR_CHUNK);
	isave->Write (&affectRegion, sizeof(int), &nb);
	isave->EndChunk ();

	isave->BeginChunk (FALLOFF_CHUNK);
	isave->Write (&falloff, sizeof(int), &nb);
	isave->EndChunk ();

	isave->BeginChunk (PINCH_CHUNK);
	isave->Write (&pinch, sizeof(int), &nb);
	isave->EndChunk ();

	isave->BeginChunk (BUBBLE_CHUNK);
	isave->Write (&bubble, sizeof(int), &nb);
	isave->EndChunk ();

	isave->BeginChunk (EDIST_CHUNK);
	isave->Write (&useEdgeDist, sizeof(int), &nb);
	isave->EndChunk ();

	isave->BeginChunk (EDGE_ITS_CHUNK);
	isave->Write (&edgeIts, sizeof(int), &nb);
	isave->EndChunk ();

	isave->BeginChunk (IG_BACK_CHUNK);
	isave->Write (&arIgBack, sizeof(int), &nb);
	isave->EndChunk ();

	isave->BeginChunk (EMOD_FLAGS_CHUNK);
	DWORD keepFlags = emFlags & EM_KEEPFLAGS;
	isave->Write (&keepFlags, sizeof(DWORD), &nb);
	isave->EndChunk();

	return IO_OK;
	}

IOResult EditMeshMod::LoadNamedSelChunk(ILoad *iload,int level)
	{	
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
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

IOResult EditMeshMod::Load(ILoad *iload) {
	Modifier::Load(iload);
	IOResult res;
	ULONG nb;
	int level = -1;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case NAMEDVSEL_NAMES_CHUNK: {				
				res = LoadNamedSelChunk(iload,0);
				break;
				}

			case NAMEDFSEL_NAMES_CHUNK: {
				res = LoadNamedSelChunk(iload,1);
				break;
				}

			case NAMEDESEL_NAMES_CHUNK: {
				res = LoadNamedSelChunk(iload,2);
				break;
				}

			case SEL_LEVEL_CHUNK: {
				short sl;
				res = iload->Read(&sl,sizeof(short),&nb);
				selLevel = sl;
				}
				break;

			case AR_CHUNK:
				res = iload->Read (&affectRegion, sizeof(int), &nb);
#ifdef USE_EMESH_SIMPLE_UI
				// always turn affectRegion off since there is no UI for it
				affectRegion = FALSE;
#endif
				break;

			case FALLOFF_CHUNK:
				res = iload->Read (&falloff, sizeof(float), &nb);
				break;

			case PINCH_CHUNK:
				res = iload->Read (&pinch, sizeof(float), &nb);
				break;

			case BUBBLE_CHUNK:
				res = iload->Read (&bubble, sizeof(float), &nb);
				break;

			case EDIST_CHUNK:
				res = iload->Read (&useEdgeDist, sizeof(int), &nb);
				break;

			case EDGE_ITS_CHUNK:
				res = iload->Read (&edgeIts, sizeof(int), &nb);
				break;

			case IG_BACK_CHUNK:
				res = iload->Read (&arIgBack, sizeof(int), &nb);
				break;

			case EMOD_FLAGS_CHUNK:
				DWORD keepFlags;
				res = iload->Read (&keepFlags, sizeof(DWORD), &nb);
				emFlags = (emFlags & EM_TEMPFLAGS) | (keepFlags & EM_KEEPFLAGS);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

// Save/Load stuff
// Here we define the obselete TopoDelta and VertexDelta classes to make
// backward-compatibility conversion easier.

class VertexDelta {
public:
	Tab<Point3>	deltas;
	BitArray	hide;

	~VertexDelta() {}
	VertexDelta() {}
};

class FaceMap {
public:
	DWORD v[3];
	FaceMap(DWORD fv[3]) {v[0]=fv[0];v[1]=fv[1];v[2]=fv[2];}
	FaceMap(DWORD a,DWORD b,DWORD c) {v[0]=a;v[1]=b;v[2]=c;}
	FaceMap() {v[0]=v[1]=v[2]=UNDEFINED;}
};
typedef Tab<FaceMap> FaceMapTab;

class TopoDelta {
public:				
	DWORDTab cverts;			// Clone verts
	DWORDTab dverts;			// Delete verts		
	Tab<Point3> nverts;			// Add verts
	Tab<Point3> nTverts;			// Add texture verts
	DWORDTab dfaces;			// Delete faces
	Tab<Face> nfaces;			// New faces
	Tab<TVFace> nTVfaces;		// New texture vert faces
	FaceMapTab map;				// Remap faces
	FaceMapTab tmap;			// TVFace remap
	DWORDTab attribs;			// Changes attributes of a face.
	DWORDTab smgroups;			// Changes smooth groups for a face.
	DWORD inputFaces; 			// Number of faces on input mesh
	DWORD inputVerts;			// Number of vertices input mesh has

	TopoDelta () {}
	~TopoDelta() {}
};

class ConvertToMeshDeltaPLCB : public PostLoadCallback {
public:
	EditMeshMod *em;
	EditMeshData *emd;
	TopoDelta td;
	VertexDelta vd;
	BitArray vsel, esel, fsel;

	ConvertToMeshDeltaPLCB(EditMeshMod *m, EditMeshData *d) { em=m; emd=d; }
	void proc(ILoad *iload);
	void MyDebugPrint();
};

#define FLAGS_CHUNK 			0x2740
#define VSEL_CHUNK 				0x2750
#define FSEL_CHUNK	 			0x2755
#define ESEL_CHUNK				0x2756
#define VERT_DELTA_CHUNK 		0x2760
#define VERT_HIDE_CHUNK			0x2761
#define TOPO_DELTA_CHUNK 		0x2765
#define TOPO_NVERTS_CHUNK 		0x2771
#define TOPO_NTVERTS_CHUNK		0x2772
#define TOPO_CVERTS_CHUNK 		0x2770
#define TOPO_NFACES_CHUNK 		0x2775
#define TOPO_NTVFACES_CHUNK 	0x2776
#define TOPO_FACEMAP_CHUNK 		0x2780
#define TOPO_TVFACEMAP_CHUNK 	0x2781
#define TOPO_DVERTS_CHUNK 		0x2790
#define TOPO_DFACES_CHUNK 		0x2800
#define TOPO_INPUTFACES_CHUNK	0x2810
#define TOPO_INPUTVERTS_CHUNK	0x2820
#define TOPO_ATTRIBS_CHUNK		0x2830
#define TOPO_SMGROUPS_CHUNK		0x2840
#define VSELSET_CHUNK			0x2845
#define FSELSET_CHUNK			0x2846
#define ESELSET_CHUNK			0x2847
#define VERT_DELTAZERO_CHUNK   	0x3000
#define TOPO_FACEMAPUNDEF_CHUNK 0x3010
#define TOPO_TVFACEMAPUNDEF_CHUNK 0x3020
#define TOPO_ATTRIBSZERO_CHUNK 0x3030
#define TOPO_SMGROUPSSAME_CHUNK 0x3040
#define MDELTA_CHUNK 0x4000

IOResult EditMeshMod::SaveLocalData(ISave *isave, LocalModData *ld) {
	ULONG nb;
	EditMeshData *em = (EditMeshData *)ld;

	isave->BeginChunk(FLAGS_CHUNK);
	isave->Write(&em->flags,sizeof(DWORD), &nb);
	isave->EndChunk();

	if (em->selSet[NS_VERTEX].Count()) {
		isave->BeginChunk(VSELSET_CHUNK);
		em->selSet[NS_VERTEX].Save(isave);
		isave->EndChunk();
	}
	if (em->selSet[NS_EDGE].Count()) {
		isave->BeginChunk(ESELSET_CHUNK);
		em->selSet[NS_EDGE].Save(isave);
		isave->EndChunk();
	}
	if (em->selSet[NS_FACE].Count()) {
		isave->BeginChunk(FSELSET_CHUNK);
		em->selSet[NS_FACE].Save(isave);
		isave->EndChunk();
	}

	isave->BeginChunk (MDELTA_CHUNK);
	IOResult res = em->mdelta.Save (isave);
	isave->EndChunk ();

	return res;
}

IOResult EditMeshMod::LoadLocalData(ILoad *iload, LocalModData **pld) {
	ULONG nb;
	int n;
	IOResult res;
	EditMeshData *em;
	if (*pld==NULL) *pld =(LocalModData *) new EditMeshData();
	em = (EditMeshData *)*pld;
	em->SetModifier (this);

	ConvertToMeshDeltaPLCB *ctmd = NULL;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case FLAGS_CHUNK:
				res = iload->Read(&em->flags,sizeof(DWORD), &nb);
				break;
			case VSEL_CHUNK:
				if (!ctmd) ctmd = new ConvertToMeshDeltaPLCB (this, em);
			   	res = ctmd->vsel.Load(iload);
				break;
			case FSEL_CHUNK:
				if (!ctmd) ctmd = new ConvertToMeshDeltaPLCB (this, em);
			   	res = ctmd->fsel.Load(iload);
				break;
			case ESEL_CHUNK:
				if (!ctmd) ctmd = new ConvertToMeshDeltaPLCB (this, em);
			   	res = ctmd->esel.Load(iload);
				break;
			
			case VSELSET_CHUNK:
				res = em->selSet[NS_VERTEX].Load(iload);
				break;
			case FSELSET_CHUNK:
				res = em->selSet[NS_FACE].Load(iload);
				break;
			case ESELSET_CHUNK:
				res = em->selSet[NS_EDGE].Load(iload);
				break;

			case MDELTA_CHUNK:
				res = em->mdelta.Load (iload);
				break;

			case VERT_HIDE_CHUNK:
				if (!ctmd) ctmd = new ConvertToMeshDeltaPLCB (this, em);
				res = ctmd->vd.hide.Load(iload);
				break;
			case VERT_DELTA_CHUNK:
				if (!ctmd) ctmd = new ConvertToMeshDeltaPLCB (this, em);
				n = iload->CurChunkLength()/sizeof(Point3);
				ctmd->vd.deltas.SetCount(n);
				res = iload->Read(ctmd->vd.deltas.Addr(0),n*sizeof(Point3),&nb);
				break;
			case VERT_DELTAZERO_CHUNK: {
				if (!ctmd) ctmd = new ConvertToMeshDeltaPLCB (this, em);
				res = iload->Read(&n,sizeof(n),&nb);
				ctmd->vd.deltas.SetCount(n);
				Point3 *pt  = ctmd->vd.deltas.Addr(0);
				Point3 pzero(0,0,0);
				for (int i=0; i<n; i++) pt[i] = pzero;
				}
				break;
			case TOPO_DELTA_CHUNK:
				if (!ctmd) ctmd = new ConvertToMeshDeltaPLCB (this, em);
				while (IO_OK==(res=iload->OpenChunk())) {
					switch(iload->CurChunkID())  {
						case TOPO_NVERTS_CHUNK:
							n = iload->CurChunkLength()/sizeof(Point3);
							ctmd->td.nverts.SetCount(n);
							res = iload->Read(ctmd->td.nverts.Addr(0),n*sizeof(Point3),&nb);
							break;
						case TOPO_NTVERTS_CHUNK:
							n = iload->CurChunkLength()/sizeof(Point3);
							ctmd->td.nTverts.SetCount(n);
							res = iload->Read(ctmd->td.nTverts.Addr(0),n*sizeof(Point3),&nb);
							break;
						case TOPO_CVERTS_CHUNK:
							n = iload->CurChunkLength()/sizeof(DWORD);
							ctmd->td.cverts.SetCount(n);
							res = iload->Read(ctmd->td.cverts.Addr(0),n*sizeof(DWORD),&nb);
							break;
						case TOPO_NFACES_CHUNK:
							n = iload->CurChunkLength()/sizeof(Face);
							ctmd->td.nfaces.SetCount(n);
							res = iload->Read(ctmd->td.nfaces.Addr(0),n*sizeof(Face),&nb);
							break;
						case TOPO_NTVFACES_CHUNK:
							n = iload->CurChunkLength()/sizeof(TVFace);
							ctmd->td.nTVfaces.SetCount(n);
							res = iload->Read(ctmd->td.nTVfaces.Addr(0),n*sizeof(TVFace),&nb);
							break;
						case TOPO_FACEMAP_CHUNK:
							n = iload->CurChunkLength()/sizeof(FaceMap);
							ctmd->td.map.SetCount(n);
							res = iload->Read(ctmd->td.map.Addr(0),n*sizeof(FaceMap),&nb);
							break;
						case TOPO_FACEMAPUNDEF_CHUNK:  {
							res = iload->Read(&n,sizeof(n),&nb);
							ctmd->td.map.SetCount(n);
							FaceMap *fm  = ctmd->td.map.Addr(0);
							for (int i=0; i<n; i++) {
								fm[i].v[0] = fm[i].v[1] = fm[i].v[2] = UNDEFINED;
								}
							}
							break;

						case TOPO_TVFACEMAP_CHUNK:
							n = iload->CurChunkLength()/sizeof(FaceMap);
							ctmd->td.tmap.SetCount(n);
							res = iload->Read(ctmd->td.tmap.Addr(0),n*sizeof(FaceMap),&nb);
							break;

						case TOPO_TVFACEMAPUNDEF_CHUNK:  {
							res = iload->Read(&n,sizeof(n),&nb);
							ctmd->td.tmap.SetCount(n);
							FaceMap *fm  = ctmd->td.tmap.Addr(0);
							for (int i=0; i<n; i++) {
								fm[i].v[0] = fm[i].v[1] = fm[i].v[2] = UNDEFINED;
								}
							}
							break;

						case TOPO_DVERTS_CHUNK:
							n = iload->CurChunkLength()/sizeof(DWORD);
							ctmd->td.dverts.SetCount(n);
							res = iload->Read(ctmd->td.dverts.Addr(0),n*sizeof(DWORD),&nb);
							break;
						case TOPO_DFACES_CHUNK:
							n = iload->CurChunkLength()/sizeof(DWORD);
							ctmd->td.dfaces.SetCount(n);
							res = iload->Read(ctmd->td.dfaces.Addr(0),n*sizeof(DWORD),&nb);
							break;
						case TOPO_ATTRIBS_CHUNK:
							n = iload->CurChunkLength()/sizeof(DWORD);
							ctmd->td.attribs.SetCount(n);
							res = iload->Read(ctmd->td.attribs.Addr(0),n*sizeof(DWORD),&nb);
							break;
						case TOPO_ATTRIBSZERO_CHUNK:
							{
							res = iload->Read(&n,sizeof(n),&nb);
							ctmd->td.attribs.SetCount(n);
							DWORD *dw  = ctmd->td.attribs.Addr(0);
							for (int i=0; i<n; i++)  dw[i] = 0;
							}
							break;
						case TOPO_SMGROUPS_CHUNK:
							n = iload->CurChunkLength()/sizeof(DWORD);
							ctmd->td.smgroups.SetCount(n);
							res = iload->Read(ctmd->td.smgroups.Addr(0),n*sizeof(DWORD),&nb);
							break;
						case TOPO_SMGROUPSSAME_CHUNK:
							{
							DWORD sg;
							res = iload->Read(&n,sizeof(n),&nb);
							res = iload->Read(&sg,sizeof(sg),&nb);
							ctmd->td.smgroups.SetCount(n);
							DWORD *dw  = ctmd->td.smgroups.Addr(0);
							for (int i=0; i<n; i++)  dw[i] = sg;
							}
							break;
						case TOPO_INPUTVERTS_CHUNK:
							res = iload->Read(&ctmd->td.inputVerts,sizeof(DWORD), &nb);
							break;
						case TOPO_INPUTFACES_CHUNK:
							res = iload->Read(&ctmd->td.inputFaces,sizeof(DWORD), &nb);
							break;
						}
					iload->CloseChunk();
					if ( res!=IO_OK) 
					return res;
					}
				if (res==IO_ERROR) return res;
				res = IO_OK;
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}

	if (ctmd) {
		iload->RegisterPostLoadCallback (ctmd);
		SetFlag (EM_SWITCH_SUBOBJ_VERSIONS);
	} else {
		BOOL ret = em->mdelta.CheckOrder ();
#ifdef _DEBUG
		if (!ret) DebugPrint ("Edit Mesh: Repairing MeshDelta internal order on load.\n");
#endif
		ret = em->mdelta.CheckMapFaces ();
#ifdef _DEBUG
		if (!ret) DebugPrint ("Edit Mesh: Repairing MeshDelta mapping faces on load.\n");
#endif
	}
	return IO_OK;
	}


// CCJ 6/19/98
// Return the sub object material assignment interface
// This is used by the node when assigning materials.
// If an open face selection mode is active, the material
// will be assigned to the selected faces only.
// A multi/sub-object material is created and the material
// is assigned to the matierial ID created for the selected
// faces.
void* EditMeshMod::GetInterface(ULONG id) {
	if (id==I_SUBMTLAPI) return (ISubMtlAPI*)this;
	else if (id==I_MESHDELTAUSER) return (MeshDeltaUser*)this;
	else if (id==I_MESHSELECT) return (IMeshSelect*)this;
	else return Modifier::GetInterface(id);
}

// Return a material ID that is currently not used by the object.
// If the current face selection share once single MtlDI that is not
// used by any other faces, you should use it.
MtlID EditMeshMod::GetNextAvailMtlID(ModContext* mc) {
	if (!mc) return 1;
	EditMeshData *d = (EditMeshData*)mc->localData;
	if (!d) return 1;
	Mesh* m = d->GetMesh (ip->GetTime());

	int mtlID = GetSelFaceUniqueMtlID(mc);

	if (mtlID == -1) {
		int i;
		BitArray b;
		mtlID = m->numFaces;
		b.SetSize(m->numFaces, FALSE);
		b.ClearAll();
		for (i=0; i<m->numFaces; i++) {
			int mid = m->faces[i].getMatID();
			if (mid < m->numFaces) {
				b.Set(mid);
				}
			}

		for (i=0; i<m->numFaces; i++) {
			if (!b[i]) {
				mtlID = i;
				break;
				}
			}
		}

	return (MtlID)mtlID;
	}

bool EditMeshMod::HasActiveSelection () {
	if (selLevel == SL_OBJECT) return true;
	ModContextList mcList;
	INodeTab nodes;	
	ip->GetModContexts(mcList,nodes);
	int i;
	for (i=0; i<mcList.Count(); i++) {
		EditMeshData *emd = (EditMeshData *) mcList[i]->localData;
		if (!emd) continue;
		if (emd->GetSel (namedSetLevel[selLevel]).NumberSet() > 0) return true;
	}
	return false;
}

// Indicate if you are active in the modifier panel and have an 
// active face selection
BOOL EditMeshMod::HasFaceSelection(ModContext* mc) {
	// Are we the edited object?
	if (ip == NULL)  return FALSE;

	EditMeshData *d = (EditMeshData*)mc->localData;

	// Do we have local data?
	if (!d) return FALSE;

	// Does the local data have a mesh?
	if (!d->MeshCached (ip->GetTime())) return FALSE;

	// Is Face selection active?
	if (selLevel < SL_FACE) return FALSE;

	return (d->mesh->faceSel.NumberSet() > 0);
}

// Set the selected faces to the specified material ID.
// If bResetUnsel is TRUE, then you should set the remaining
// faces material ID's to 0
void EditMeshMod::SetSelFaceMtlID(ModContext* mc, MtlID id, BOOL bResetUnsel) {

	EditMeshData *d = (EditMeshData*)mc->localData;

	d->BeginEdit (ip->GetTime());
	Mesh *m = d->GetMesh (ip->GetTime());		

	// Can't use the EMD_FLAG here since we will be "Accepted" by the materials editor...
	if (theHold.Holding()) theHold.Put(new FaceChangeRestore(d,this));

	for (int j=0; j<m->getNumFaces(); j++) {			
		if (m->faceSel[j]) {
			m->setFaceMtlIndex(j,(MtlID)id);
			d->mdelta.SetMatID((DWORD)j,id);
			}
		else if (bResetUnsel) {
			m->setFaceMtlIndex(j,(MtlID)0);
			d->mdelta.SetMatID((DWORD)j,0);
		}
	}

	InvalidateSurfaceUI();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
}

// Return the material ID of the selected face(s).
// If multiple faces are selected they should all have the same MtlID -
// otherwise you should return -1.
// If faces other than the selected share the same material ID, then 
// you should return -1.
int EditMeshMod::GetSelFaceUniqueMtlID(ModContext* mc) {
	int	i;
	int	mtlID;

	mtlID = GetSelFaceAnyMtlID(mc);

	EditMeshData *d = (EditMeshData*)mc->localData;
	Mesh* m = d->GetMesh (ip->GetTime());

	if (mtlID == -1) return mtlID;
	for (i=0; i<m->numFaces; i++) {
		if (m->faceSel[i]) continue;
		if (m->faces[i].getMatID() != mtlID) continue;
		mtlID = -1;
		break;
	}

	return mtlID;
}

// Return the material ID of the selected face(s).
// If multiple faces are selected they should all have the same MtlID,
// otherwise you should return -1.
int EditMeshMod::GetSelFaceAnyMtlID(ModContext* mc) {
	int				mtlID = -1;
	BOOL			bGotFirst = FALSE;
	int				i;

	EditMeshData *d = (EditMeshData*)mc->localData;
	Mesh* m = d->GetMesh (ip->GetTime());

	for (i=0; i<m->numFaces; i++) {
		if (!m->faceSel[i]) continue;
		if (bGotFirst) {
			if (mtlID != m->faces[i].getMatID()) {
				mtlID = -1;
				break;
			}
		} else {
			mtlID = m->faces[i].getMatID();
			bGotFirst = TRUE;
		}
	}

	return mtlID;
}

// Return the highest MtlID used by the object.
int EditMeshMod::GetMaxMtlID(ModContext* mc) {
	MtlID mtlID = 0;

	EditMeshData *d  = (EditMeshData*)mc->localData;
	Mesh* m = d->GetMesh (ip->GetTime());

	for (int i=0; i<m->numFaces; i++) mtlID = max(mtlID, m->faces[i].getMatID());
	return mtlID;
}

// Old bits for attribs:
// First 3 bits are edge visibility
// 4th bit is face visibility
// 29,30 and 31 indicate which if any should be applied
#define OLD_ATTRIB_APPLY_EDGE		(1<<31)
#define OLD_ATTRIB_APPLY_FACE		(1<<30)
#define OLD_ATTRIB_APPLY_MATID		(1<<29)
#define OLD_ATTRIB_APPLY_SMGROUP	(1<<28)

// Mat ID takes bit 5-21
#define OLD_ATTRIB_MATID_SHIFT	5
#define OLD_ATTRIB_MATID_MASK	0xffff

void ConvertToMeshDeltaPLCB::MyDebugPrint () {
	DebugPrint ("\nConverting old TopoDelta, VertexDelta to MeshDelta.\n");
	DebugPrint ("Expecting pipeline mesh of %d verts, %d faces\n", td.inputVerts, td.inputFaces);
	int i, num;
	if (num=td.cverts.Count()) {
		DebugPrint ("%d Vertex Clones:\n  ", num);
		for (i=0; i<num; i++) {
			DebugPrint ("%2d ", td.cverts[i]);
			if ((i<num-1) && (i%10==9)) DebugPrint ("\n  ");
		}
		DebugPrint ("\n");
	}
	if (num=td.dverts.Count()) {
		DebugPrint ("%d Vertex Deletes:\n  ", num);
		for (i=0; i<num; i++) {
			DebugPrint ("%2d ", td.dverts[i]);
			if ((i<num-1) && (i%10==9)) DebugPrint ("\n  ");
		}
		DebugPrint ("\n");
	}
	if (num=td.nverts.Count()) {
		DebugPrint ("%d Vertex Creates:\n", num);
		for (i=0; i<num; i++) {
			DebugPrint ("  %7.3f, %7.3f, %7.3f\n", td.nverts[i].x, td.nverts[i].y, td.nverts[i].z);
		}
	}
	if (num=vd.deltas.Count()) {
		DebugPrint ("Vertex Moves:\n");
		for (i=0; i<num; i++) {
			if (vd.deltas[i] == Point3(0,0,0)) continue;
			DebugPrint ("  %d: %7.3f, %7.3f, %7.3f\n", i, vd.deltas[i].x, vd.deltas[i].y, vd.deltas[i].z);
		}
	}
	if (num=td.map.Count()) {
		DebugPrint ("Face Remaps:\n");
		for (i=0; i<num; i++) {
			DebugPrint ("  %d: %3d, %3d, %3d\n", i, td.map[i].v[0], td.map[i].v[1], td.map[i].v[2]);
		}
	}
	if (num=td.nfaces.Count()) {
		DebugPrint ("Face Creates:\n");
		for (i=0; i<num; i++) {
			DebugPrint ("  %3d, %3d, %3d\n", td.nfaces[i].v[0], td.nfaces[i].v[1], td.nfaces[i].v[2]);
		}
	}
}

void ConvertToMeshDeltaPLCB::proc (ILoad *iload) {
	int num, i,j;
	// MyDebugPrint ();
	// Perform operations in same order as old TopoDelta::Apply, etc.
	MeshDelta & md = emd->mdelta;
	md.ClearAllOps ();
	if (emd->flags & EMD_HASDATA) {
		md.SetInVNum (td.inputVerts);
		md.SetInFNum (td.inputFaces);
		md.SetMapNum (2);
		md.mapSupport.ClearAll();
		if (td.nfaces.Count() == td.nTVfaces.Count()) {
			md.mapSupport.Set (1);
			md.map[1].fnum = td.inputFaces;
			// We have no value for md.map[1].vnum -- it's filled in by the first "Apply".
		}
		for (i=0; i<td.cverts.Count(); i++) md.VClone (td.cverts[i]);
		if (num=td.dverts.Count()) {
			// Careful how we apply following -- old way of saying verts 2,3 get deleted is to say 2, 2 get deleted -
			// delete vert 2, then delete vert 2 from what's left.
			// Equally valid old deletion pattern would be 3,2.
			BitArray vd;
			DWORD vdsize = md.vnum + md.vClone.Count();
			vd.SetSize (vdsize);
			for (i=0; i<num; i++) {
				int del = td.dverts[i];
				for (j=0; j<=del; j++) {	// Increment by the number of verts already deleted "below" this one.
					if (vd[j]) del++;
					if (del >= vdsize) break;
				}
				if (j<del) continue;
				vd.Set (del);
			}
			md.VDelete (vd);
		}
		if (td.nverts.Count()) md.VCreate (td.nverts.Addr(0), td.nverts.Count());
		int lastOrigV = md.vnum - md.vDelete.NumberSet();
		int lastCloneV = lastOrigV + md.vClone.Count();
		int numCl = md.vClone.Count();

		if (md.mapSupport[1] && td.nTverts.Count()) md.map[1].VCreate (td.nTverts.Addr(0), td.nTverts.Count());

		if (num=td.dfaces.Count()) {
			// Careful how we apply following -- old way of saying faces 2,3 get deleted is to say 2, 2 get deleted -
			// delete face 2, then delete face 2 from what's left.
			// Equally valid old deletion pattern would be 3,2.
			BitArray fd;
			fd.SetSize (md.fnum);
			for (i=0; i<num; i++) {
				int del = td.dfaces[i];
				for (j=0; j<=del; j++) {	// Increment by the number of faces already deleted "below" this one.
					if (fd[j]) del++;
					if (del >= md.fnum) break;
				}
				if (j<del) continue;
				fd.Set (del);
			}
			md.FDelete (fd);
		}

		if (td.nfaces.Count()) md.FCreate (td.nfaces.Addr(0), td.nfaces.Count());

		if (md.mapSupport[1] && td.nTVfaces.Count()) md.map[1].FCreate (td.nTVfaces.Addr(0), td.nTVfaces.Count());

		FaceRemap temp;
		DWORD *ww = temp.v;
		if (td.map.Count()) {
			// Remaps in original form are indexed by post-deleted verts
			DWORD max = lastCloneV + td.nverts.Count();
			for (i=0; i<td.map.Count(); i++) {
				temp.flags = 0;
				for (j=0; j<3; j++) {
					if ((ww[j] = td.map[i].v[j]) != UNDEFINED) {
						temp.flags |= (1<<j);
						if (ww[j] >= max) ww[j] = 0;	// correct for out-of-range.
						ww[j] = md.VLut (ww[j]);	// Here is the conversion to pre-deleted.
					}
				}
				if (!temp.flags) continue;
				temp.f = md.FLut(i);
				if (temp.f>=md.fnum) {
					MaxAssert (temp.f < md.fnum + (DWORD)md.NumFCreate());
					temp.Apply (md.fCreate[temp.f-md.fnum].face);
				} else md.fRemap.Append (1, &temp, td.map.Count());
			}
		}

		if (md.mapSupport[1] && td.tmap.Count()) {
			for (i=0; i<td.tmap.Count(); i++) {
				temp.flags = 0;
				for (j=0; j<3; j++) {
					if ((ww[j] = td.tmap[i].v[j]) != UNDEFINED) temp.flags |= (1<<j);
				}
				if (!temp.flags) continue;
				temp.f = i;
				if (temp.f>=md.fnum) {
					MaxAssert (temp.f < md.fnum + (DWORD)md.map[1].fCreate.Count());
					temp.Apply (md.fCreate[temp.f-md.fnum].face);
				} else md.map[1].fRemap.Append (1, &temp, td.map.Count());
			}
		}

		if (td.attribs.Count()) {
			FaceChange fcTemp;
			FaceSmooth fsTemp;
			for (i=0; i<td.attribs.Count(); i++) {
				fcTemp.flags = 0;
				fcTemp.val = 0;
				if (td.attribs[i] & OLD_ATTRIB_APPLY_EDGE) {
					fcTemp.flags |= EDGE_ALL;
					fcTemp.val |= (td.attribs[i] & EDGE_ALL);
				}
				if (td.attribs[i] & OLD_ATTRIB_APPLY_FACE) {
					fcTemp.flags |= FACE_HIDDEN;
					fcTemp.val |= (td.attribs[i] & FACE_HIDDEN);
				}
				if (td.attribs[i] & OLD_ATTRIB_APPLY_MATID) {
					fcTemp.flags |= ATTRIB_MATID;
					MtlID mid = (MtlID)(td.attribs[i]>>OLD_ATTRIB_MATID_SHIFT)&OLD_ATTRIB_MATID_MASK;
					fcTemp.val |= mid<<ATTRIB_MATID_SHIFT;
				}
				if (fcTemp.flags) {
					fcTemp.f = md.FLut(i);
					if (fcTemp.f >= md.fnum) {
						MaxAssert (fcTemp.f < md.fnum + (DWORD)md.NumFCreate());
						temp.Apply (md.fCreate[temp.f-md.fnum].face);
					} else md.fChange.Append (1, &fcTemp, td.attribs.Count());
				}
				if (td.attribs[i] & OLD_ATTRIB_APPLY_SMGROUP) {
					fsTemp.f = md.FLut(i);
					fsTemp.mask = ~0x0;
					fsTemp.val = td.smgroups[i];
					if (fsTemp.f >= md.fnum) {
						MaxAssert (fcTemp.f < md.fnum + (DWORD)md.NumFCreate());
						temp.Apply (md.fCreate[temp.f-md.fnum].face);
					} else md.fSmooth.Append (1, &fsTemp, td.attribs.Count());
				}
			}
		}

		for (i=0; i<vd.deltas.Count(); i++) {
			md.Move (i, vd.deltas[i]);
		}
		md.vsel = vsel;
		md.vsel.SetSize (md.outVNum(), TRUE);
		md.vhide = vd.hide;
		md.vhide.SetSize (md.outVNum(), TRUE);
		md.esel = esel;
		md.esel.SetSize (md.outFNum()*3, TRUE);
		md.fsel = fsel;
		md.fsel.SetSize (md.outFNum(), TRUE);

		// Finally, if we're loading up a topodelta version, the subobject indices have changed:
		if (em->GetFlag (EM_SWITCH_SUBOBJ_VERSIONS)) {
			switch (em->selLevel) {
			case 2:
				em->selLevel = SL_POLY;
				break;
			case 3:
				em->selLevel = SL_EDGE;
				break;
			}
			em->ClearFlag (EM_SWITCH_SUBOBJ_VERSIONS);
		}
	}

	//emd->mdelta.MyDebugPrint (FALSE, TRUE);
	delete this;
}

void EditMeshMod::ShowEndResultChanged (BOOL showEndResult) {
	if (!ip) return;
	if (!GetFlag (EM_EDITING)) return;
	NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
}

// Not a real restore object:
TempMoveRestore::TempMoveRestore (Mesh *msh, bool doMaps) {
	init.SetCount (msh->numVerts);
	if (msh->numVerts) memcpy (init.Addr(0), msh->verts, msh->numVerts*sizeof(Point3));

	if (doMaps) {
		maps.SetCount (msh->getNumMaps() + NUM_HIDDENMAPS);
		for (int mp=-NUM_HIDDENMAPS; mp<msh->getNumMaps(); mp++) {
			int nmp = mp + NUM_HIDDENMAPS;
			int mvnum = 0;
			if (msh->mapSupport(mp)) mvnum = msh->getNumMapVerts(mp);
			if (mvnum) {
				maps[nmp] = new Tab<UVVert>;
				maps[nmp]->SetCount (mvnum);
				memcpy (maps[nmp]->Addr(0), msh->mapVerts(mp), mvnum*sizeof(UVVert));
			} else {
				maps[nmp] = NULL;
			}
		}
	} else {
		maps.ZeroCount ();
	}
}

void TempMoveRestore::Restore (Mesh *msh) {
	if (!init.Count()) return;
	int min = (msh->numVerts < init.Count()) ? msh->numVerts : init.Count();
	memcpy (msh->verts, init.Addr(0), min*sizeof(Point3));
	for (int nmp=0; nmp<maps.Count(); nmp++) {
		if (!maps[nmp]) continue;
		int mp = nmp - NUM_HIDDENMAPS;
		if (!msh->mapSupport(mp)) continue;
		memcpy (msh->mapVerts(mp), maps[nmp]->Addr(0), maps[nmp]->Count()*sizeof(UVVert));
	}
}

DWORD TempMoveRestore::ChannelsChanged () {
	DWORD ret = PART_GEOM;
	for (int nmp=0; nmp<maps.Count(); nmp++) {
		if (maps[nmp]) ret |= MapChannelID (nmp-NUM_HIDDENMAPS);
	}
	return ret;
}

static TimeValue dragTime;
static bool dragRestored;
static bool inDragMove=FALSE;

void EditMeshMod::DragMoveInit (TimeValue t, bool doMaps) {
	if (!ip) return;
	dragTime = t;
	inDragMove = TRUE;
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	for (int nd = 0; nd<mcList.Count(); nd++) {
		if (!mcList[nd]->localData) continue;
		EditMeshData *emd = (EditMeshData *) mcList[nd]->localData;
		Mesh *mesh = emd->GetMesh(t);
		emd->tempMove.InitToMesh (*mesh);
		if (emd->tmr) delete emd->tmr;
		emd->tmr = new TempMoveRestore (mesh, doMaps);
	}
	dragRestored = TRUE;
}

void EditMeshMod::DragMoveRestore () {
	if (!ip) return;
	if (dragRestored) return;
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	for (int nd = 0; nd<mcList.Count(); nd++) {
		if (!mcList[nd]->localData) continue;
		EditMeshData *emd = (EditMeshData *) mcList[nd]->localData;
		emd->tempMove.vMove.ZeroCount();
		if (!emd->tmr) continue;
		emd->tmr->Restore (emd->GetMesh(dragTime));
	}
	LocalDataChanged (PART_GEOM);
	if (theHold.Holding ()) theHold.Put (new CueLocalRestore(this));
	dragRestored = TRUE;
}

void EditMeshMod::DragMove (MeshDelta & md, MeshDeltaUserData *mdu) {
	if (!ip) return;
	if (!inDragMove) {
		mdu->ApplyMeshDelta (md, this, ip->GetTime());
		return;
	}

	EditMeshData *emd = (EditMeshData *) mdu;
	Mesh *mesh = emd->GetMesh (dragTime);
	// Only care about vMove:
	for (int i=0; i<md.vMove.Count(); i++) {
		DWORD j = md.vMove[i].vid;
		mesh->verts[j] += md.vMove[i].dv;
	}
	// And the map equivalents:
	for (int mp=-NUM_HIDDENMAPS; mp<md.GetMapNum(); mp++) {
		if (!md.getMapSupport(mp)) continue;
		for (i=0; i<md.Map(mp).vSet.Count(); i++) {
			DWORD j=md.Map(mp).vSet[i].vid;
			mesh->mapVerts(mp)[j] = md.Map(mp).vSet[i].v;
		}
	}
	emd->tempMove = md;
	LocalDataChanged (PART_GEOM);
	dragRestored = FALSE;
}

void EditMeshMod::DragMoveAccept () {
	if (!ip) return;
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	for (int nd = 0; nd<mcList.Count(); nd++) {
		if (!mcList[nd]->localData) continue;
		EditMeshData *emd = (EditMeshData *) mcList[nd]->localData;
		if (!emd->tmr) continue;
		emd->Invalidate (emd->tmr->ChannelsChanged(), FALSE);
		// (Invalidate clears out the mesh, so we don't update it incorrectly with a double-move.)
		emd->ApplyMeshDelta (emd->tempMove, this, dragTime);
		emd->tempMove.ClearAllOps ();
		delete emd->tmr;
		emd->tmr = NULL;
	}
	inDragMove = FALSE;
}
