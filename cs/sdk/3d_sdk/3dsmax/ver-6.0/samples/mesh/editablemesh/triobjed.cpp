/**********************************************************************
 *<
	FILE: triobjed.cpp

	DESCRIPTION:   Editable Triangle Mesh Object

	CREATED BY: Rolf Berteig

	HISTORY: created 4 March 1996

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#include "pack1.h"
#include "MeshNormalSpec.h"
#include "triobjed.h"
#include "nsclip.h"
#include "macrorec.h"
#include "setkeymode.h"

// xavier robitaille | 03.02.13 | fixed values for metric units
#ifndef METRIC_UNITS_FIXED_VALUES
#define TRIOBJED_NORM_SCALE		20.0f
#else
#define TRIOBJED_NORM_SCALE		DFLT_EDIT_MESH_NORMAL_SCALE
#endif

//--- Class descriptor/Class vars -------------------------------

class EditTriObjectClassDesc : public ClassDesc {
	public:
	int 			IsPublic() {return 0;}
	void *			Create(BOOL loading = FALSE) {return new EditTriObject;}
	const TCHAR *	ClassName() {return GetString(IDS_SCA_BASE_MESH);}
	SClass_ID		SuperClassID() {return GEOMOBJECT_CLASS_ID;}
	Class_ID		ClassID() {return Class_ID(EDITTRIOBJ_CLASS_ID,0);}
	const TCHAR* 	Category() {return GetString(IDS_BASE_OBJECTS);}
	void			ResetClassParams(BOOL fileReset);
	int NumActionTables() { return 1; }
	ActionTable*  GetActionTable(int i) { return GetEMeshActions(); }
};

static EditTriObjectClassDesc editTriObjectDesc;
ClassDesc* GetEditTriObjectDesc() {return &editTriObjectDesc;}

HWND				EditTriObject::hSel        = NULL;
HWND				EditTriObject::hAR			= NULL;
HWND				EditTriObject::hGeom        = NULL;
HWND				EditTriObject::hApprox         = NULL;
HWND				EditTriObject::hSurf		= NULL;
Interface *			EditTriObject::ip              = NULL;
EditTriObject *		EditTriObject::editObj = NULL;
MoveModBoxCMode*    EditTriObject::moveMode        = NULL;
RotateModBoxCMode*  EditTriObject::rotMode 	       = NULL;
UScaleModBoxCMode*  EditTriObject::uscaleMode      = NULL;
NUScaleModBoxCMode* EditTriObject::nuscaleMode     = NULL;
SquashModBoxCMode*  EditTriObject::squashMode      = NULL;
SelectModBoxCMode*  EditTriObject::selectMode      = NULL;
WeldVertCMode*      EditTriObject::weldVertMode    = NULL;
CreateVertCMode*	EditTriObject::createVertMode  = NULL;
CreateFaceCMode*	EditTriObject::createFaceMode   = NULL;
TurnEdgeCMode*		EditTriObject::turnEdgeMode    = NULL;
DivideEdgeCMode*	EditTriObject::divideEdgeMode  = NULL;
AttachPickMode*		EditTriObject::attachPickMode  = NULL;
ExtrudeCMode*		EditTriObject::extrudeMode     = NULL;
BevelCMode*			EditTriObject::bevelMode     = NULL;
ChamferCMode * EditTriObject::chamferMode = NULL;
FlipNormCMode*      EditTriObject::flipMode        = NULL;
CutEdgeCMode *      EditTriObject::cutEdgeMode    = NULL;
DivideFaceCMode *	EditTriObject::divideFaceMode = NULL;
float               EditTriObject::normScale       = TRIOBJED_NORM_SCALE;
bool	EditTriObject::showVNormals = false;
bool	EditTriObject::showFNormals = false;
BOOL                EditTriObject::selByVert       = FALSE;
BOOL                EditTriObject::inBuildFace     = FALSE;
BOOL                EditTriObject::inCutEdge     = FALSE;
BOOL                EditTriObject::faceUIValid     = FALSE;
BOOL				EditTriObject::inExtrude    = FALSE;
BOOL				EditTriObject::inBevel = FALSE;
BOOL				EditTriObject::inChamfer = FALSE;
int					EditTriObject::extType = MESH_EXTRUDE_CLUSTER;
BOOL                EditTriObject::ignoreBackfaces = FALSE;
BOOL                EditTriObject::ignoreVisEdge   = FALSE;
BOOL				EditTriObject::rsSel = TRUE;
BOOL				EditTriObject::rsAR = FALSE;
BOOL				EditTriObject::rsGeom = TRUE;
BOOL				EditTriObject::rsApprox = FALSE;
BOOL				EditTriObject::rsSurf = TRUE;
int                 EditTriObject::pickBoxSize     = DEF_PICKBOX_SIZE;
int                 EditTriObject::weldBoxSize     = DEF_PICKBOX_SIZE;
int                 EditTriObject::attachMat       = ATTACHMAT_IDTOMAT;
BOOL                EditTriObject::condenseMat     = true;
bool                EditTriObject::sliceSplit = FALSE;
bool                EditTriObject::cutRefine = TRUE;
Quat               EditTriObject::sliceRot(0.0f,0.0f,0.0f,1.0f);
Point3            EditTriObject::sliceCenter(0.0f,0.0f,0.0f);
bool				EditTriObject::sliceMode = FALSE;
float				EditTriObject::sliceSize = 100.0f;
TempMoveRestore * EditTriObject::tempMove = NULL;

void EditTriObjectClassDesc::ResetClassParams (BOOL fileReset) {
	EditTriObject::normScale = TRIOBJED_NORM_SCALE;
	EditTriObject::showVNormals = false;
	EditTriObject::showFNormals = false;
	EditTriObject::selByVert = FALSE;
	EditTriObject::inBuildFace = FALSE;
	EditTriObject::inCutEdge = FALSE;
	EditTriObject::inExtrude = FALSE;
	EditTriObject::inBevel = FALSE;
	EditTriObject::inChamfer = FALSE;
	EditTriObject::extType = MESH_EXTRUDE_CLUSTER;
	EditTriObject::ignoreBackfaces = FALSE;
	EditTriObject::ignoreVisEdge = FALSE;
	EditTriObject::rsSel = TRUE;
	EditTriObject::rsAR = FALSE;
	EditTriObject::rsGeom = TRUE;
	EditTriObject::rsApprox = TRUE;
	EditTriObject::rsSurf = TRUE;
	EditTriObject::pickBoxSize = DEF_PICKBOX_SIZE;
	EditTriObject::weldBoxSize = DEF_PICKBOX_SIZE;
	EditTriObject::attachMat = ATTACHMAT_IDTOMAT;
	EditTriObject::condenseMat = true;
	EditTriObject::sliceSplit = FALSE;
	EditTriObject::cutRefine = TRUE;
	ResetEditableMeshUI();
}

static int hitLevel[] = {0, SUBHIT_VERTS,SUBHIT_EDGES,SUBHIT_FACES,
								SUBHIT_FACES, SUBHIT_FACES};

//--- EditTriObject methods ----------------------------------

EditTriObject::EditTriObject() {
	tempData = NULL;
	falloff = DEF_FALLOFF;
	pinch = 0.0f;
	bubble = 0.0f;
	selLevel = SL_OBJECT;
	GetMesh().selLevel = MESH_OBJECT;
	GetMesh().dispFlags = 0;
	affectRegion = FALSE;
	edgeIts = 0;
	useEdgeDist = FALSE;
	arIgBack = FALSE;
	arValid.SetEmpty();
	etFlags = 0;
	theHold.Suspend(); // DS 1/16/98
	MakeRefByID (FOREVER, ET_MASTER_CONTROL_REF, NewDefaultMasterPointController());
	theHold.Resume();	// DS 1/16/98
}

EditTriObject::~EditTriObject() {
	if (tempData) delete tempData;
	theHold.Suspend ();
	DeleteAllRefsFromMe ();
	theHold.Resume ();
}

RefTargetHandle EditTriObject::Clone(RemapDir& remap) {
	EditTriObject *ntri = new EditTriObject;
	ntri->mesh = mesh;
	ntri->mDispApprox = mDispApprox;
	ntri->mSubDivideDisplacement = mSubDivideDisplacement;
	ntri->mSplitMesh = mSplitMesh;
	ntri->mDisableDisplacement = mDisableDisplacement;
	for (int i=0; i<cont.Count(); i++) {
		if (cont[i] == NULL) continue;
		ntri->MakeRefByID (FOREVER, ET_VERT_BASE_REF + i, cont[i]->Clone (remap));
	}
	for (i=0; i<3; i++) ntri->selSet[i] = selSet[i];

	ntri->affectRegion = affectRegion;
	ntri->falloff = falloff;
	ntri->pinch = pinch;
	ntri->bubble = bubble;
	ntri->useEdgeDist = useEdgeDist;
	ntri->edgeIts = edgeIts;
	ntri->arIgBack = arIgBack;
	ntri->arValid = arValid;
	ntri->cutRefine = cutRefine;
	ntri->extType = extType;
	ntri->ignoreBackfaces = ignoreBackfaces;
	ntri->ignoreVisEdge = ignoreVisEdge;
	ntri->normScale = normScale;
	ntri->selByVert = selByVert;
	ntri->selLevel = selLevel;
	ntri->showFNormals = showFNormals;
	ntri->showVNormals = showVNormals;
	ntri->sliceSplit = sliceSplit;

	BaseClone(this, ntri, remap);
	return ntri;
}

BOOL EditTriObject::IsSubClassOf(Class_ID classID) {
	return classID==ClassID() || classID==triObjectClassID;	
}

int EditTriObject::GetSubobjectLevel() {
	return selLevel;
}

void EditTriObject::SetSubobjectLevel(int level) {
	selLevel = level;
	switch (selLevel) {
	case SL_OBJECT:
		GetMesh().selLevel = MESH_OBJECT;
		GetMesh().dispFlags = 0;
		break;
	case SL_VERTEX:
		GetMesh().selLevel = MESH_VERTEX;
		if (ip && ip->GetShowEndResult()) GetMesh().dispFlags = 0;
		else GetMesh().dispFlags = DISP_VERTTICKS|DISP_SELVERTS;
		break;
	case SL_EDGE:
		GetMesh().selLevel = MESH_EDGE;
		GetMesh().dispFlags = DISP_SELEDGES;
		break;
	default:
		GetMesh().selLevel = MESH_FACE;
		if (selLevel==SL_FACE) GetMesh().dispFlags = DISP_SELFACES;
		else GetMesh().dispFlags = DISP_SELPOLYS;
		if (inBuildFace) GetMesh().dispFlags |= (DISP_VERTTICKS|DISP_SELVERTS);
		break;
	}
	InvalidateTempData (PART_SUBSEL_TYPE);
	if (hSel) RefreshSelType ();
	InvalidateNumberSelected ();
	if (ip) ip->RedrawViews (ip->GetTime());
}

bool CheckNodeSelection (Interface *ip, INode *inode) {
	if (!ip) return FALSE;
	if (!inode) return FALSE;
	int i, nct = ip->GetSelNodeCount();
	for (i=0; i<nct; i++) if (ip->GetSelNode (i) == inode) return TRUE;
	return FALSE;
}

int EditTriObject::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc) {	
	if (!ip) return 0;
	if (editObj != this) return 0;
	// Set up GW
	GraphicsWindow *gw = vpt->getGW();
	Matrix3 tm = inode->GetObjectTM(t);
	if (!ip->GetShowEndResult()) return 0;
	if (selLevel == SL_OBJECT) return 0;

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

	DWORD savedLimits = gw->getRndLimits();
	gw->setRndLimits((savedLimits & ~GW_ILLUM) | GW_ALL_EDGES);
	gw->setTransform(tm);

	// We need to draw a "gizmo" version of the mesh:
	Point3 colSel=GetSubSelColor();
	Point3 colTicks=GetUIColor (COLOR_VERT_TICKS);
	Point3 colGiz=GetUIColor(COLOR_GIZMOS);
	Point3 colGizSel=GetUIColor(COLOR_SEL_GIZMOS);
	gw->setColor (LINE_COLOR, colGiz);
	Mesh & mesh = GetMesh();
	AdjEdgeList *ae = TempData()->AdjEList ();
	Point3 rp[3];
	int i, ect = ae->edges.Count();

#ifdef MESH_CAGE_BACKFACE_CULLING
	// Figure out backfacing from frontfacing.
	BitArray fBackfacing, vBackfacing;
	bool backCull = (savedLimits & GW_BACKCULL) ? true : false;
	if (backCull)
	{
		fBackfacing.SetSize (mesh.numFaces);
		vBackfacing.SetSize (mesh.numVerts);
		vBackfacing.SetAll ();
		BitArray nonIsoVerts;
		nonIsoVerts.SetSize (mesh.numVerts);

		mesh.checkNormals (false);	// Allocates rVerts.
		RVertex *rv = mesh.getRVertPtr (0);
		BOOL gwFlipped = gw->getFlipped();

		for (i=0; i <mesh.numVerts; i++) {
			rv[i].rFlags = (rv[i].rFlags & ~(GW_PLANE_MASK | RND_MASK | RECT_MASK)) |
				gw->hTransPoint(&(mesh.verts[i]), (IPoint3 *)rv[i].pos);
		}
		for (i=0; i<mesh.numFaces; i++)
		{
			Face & f = mesh.faces[i];
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
		if (me.Hidden (mesh.faces)) continue;
		if (me.Visible (mesh.faces)) {
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
			if (ae->edges[i].Selected (mesh.faces, mesh.edgeSel)) gw->setColor (LINE_COLOR, colGizSel);
			else gw->setColor (LINE_COLOR, colGiz);
		}
		if (selLevel >= SL_FACE) {
			if (ae->edges[i].AFaceSelected (mesh.faceSel)) gw->setColor (LINE_COLOR, colGizSel);
			else gw->setColor (LINE_COLOR, colGiz);
		}
		rp[0] = mesh.verts[me.v[0]];
		rp[1] = mesh.verts[me.v[1]];
		gw->polyline (2, rp, NULL, NULL, FALSE, es);
	}
	if ((selLevel == SL_VERTEX) || (ip->GetCommandMode()==createFaceMode)) {
		float *ourvw = NULL;
		if (affectRegion) {
			ourvw = TempData()->VSWeight (useEdgeDist, edgeIts, arIgBack, falloff, pinch, bubble)->Addr(0);
		}
		for (i=0; i<mesh.numVerts; i++) {
			if (mesh.vertHide[i]) continue;
#ifdef MESH_CAGE_BACKFACE_CULLING
			if (backCull && vBackfacing[i]) continue;
#endif

			if (mesh.vertSel[i]) gw->setColor (LINE_COLOR, colSel);
			else {
				if (ourvw) gw->setColor (LINE_COLOR, SoftSelectionColor(ourvw[i]));
				else gw->setColor (LINE_COLOR, colTicks);
			}

			if(getUseVertexDots()) gw->marker (&(mesh.verts[i]), VERTEX_DOT_MARKER(getVertexDotType()));
			else gw->marker (&(mesh.verts[i]), PLUS_SIGN_MRKR);
		}
	}
	if (inBuildFace && (editObj==this)) {
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_SUBSELECTION));
		createFaceMode->proc.DrawEstablishedFace (gw);
	}

	gw->setRndLimits(savedLimits);
	return 0;	
}

void EditTriObject::GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc) {
	if (!ip || editObj != this) return;
	box.Init();
	Matrix3 tm = inode->GetObjectTM(t);

	if (ip->GetShowEndResult() && (selLevel != SL_OBJECT)) {
		// We need to draw a "gizmo" version of the mesh:
		Mesh & mesh = GetMesh();
		box = mesh.getBoundingBox (&tm);
	}
}

int EditTriObject::Display (TimeValue t, INode* inode, ViewExp *vpt, int flags) {
	TriObject::Display (t, inode, vpt, flags);
	if (!CheckNodeSelection (ip, inode)) return 0;
	GraphicsWindow *gw = vpt->getGW();
	Matrix3 tm = inode->GetObjectTM(t);
	int savedLimits;

	gw->setRndLimits((savedLimits = gw->getRndLimits()) & ~GW_ILLUM);
	gw->setTransform(tm);

	if (sliceMode) {
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

	if (inBuildFace && (editObj==this)) {
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_SUBSELECTION));
		createFaceMode->proc.DrawEstablishedFace (gw);
	}

	gw->setRndLimits(savedLimits);
	return 0;
}

void EditTriObject::GetWorldBoundBox (TimeValue t, INode* inode, ViewExp* vp, Box3& box ) {
	TriObject::GetWorldBoundBox (t, inode, vp, box);
	if (!sliceMode) return;
	if (!CheckNodeSelection (ip, inode)) return;
	Matrix3 tm = inode->GetObjectTM(t);
	Matrix3 rotMatrix;
	sliceRot.MakeMatrix (rotMatrix);
	rotMatrix.SetTrans (sliceCenter);
	rotMatrix *= tm;
	box += Point3(-sliceSize,-sliceSize,0.0f)*rotMatrix;
	box += Point3(-sliceSize,sliceSize,0.0f)*rotMatrix;
	box += Point3(sliceSize,sliceSize,0.0f)*rotMatrix;
	box += Point3(sliceSize,-sliceSize,0.0f)*rotMatrix;
}

void EditTriObject::GetLocalBoundBox (TimeValue t, INode* inode, ViewExp* vp, Box3& box ) {
	TriObject::GetLocalBoundBox (t, inode, vp, box);
	if (!sliceMode) return;
	if (!CheckNodeSelection (ip, inode)) return;
	Matrix3 rotMatrix;
	sliceRot.MakeMatrix (rotMatrix);
	rotMatrix.SetTrans (sliceCenter);
	box += Point3(-sliceSize,-sliceSize,0.0f)*rotMatrix;
	box += Point3(-sliceSize,sliceSize,0.0f)*rotMatrix;
	box += Point3(sliceSize,sliceSize,0.0f)*rotMatrix;
	box += Point3(sliceSize,-sliceSize,0.0f)*rotMatrix;
}

int EditTriObject::HitTest (TimeValue t, INode* inode, int type, int crossing, 
		int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc) {
	Interval valid;
	int savedLimits, res = 0;
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

	SubObjHitList hitList;
	MeshSubHitRec *rec;
	DWORD hitLev = hitLevel[selLevel];
	int localSelByVert = selByVert;
	//SS 4/26/2001: Allow clients to call HitTest when we're not active in the
	// command panel.
	if (ip && cutEdgeMode == ip->GetCommandMode()) localSelByVert = FALSE;
	if (ip && turnEdgeMode == ip->GetCommandMode()) localSelByVert = FALSE;
	if (ip && divideEdgeMode == ip->GetCommandMode()) localSelByVert = FALSE;
	if (ip && divideFaceMode == ip->GetCommandMode()) localSelByVert = FALSE;

	if (inBuildFace || localSelByVert) hitLev = SUBHIT_VERTS;
	else if (inCutEdge) hitLev = SUBHIT_EDGES;

	if (hitLev == SUBHIT_VERTS) {
		BitArray oldHide;
		if (ignoreBackfaces) {
			BOOL flip = mat.Parity();
			oldHide = GetMesh().vertHide;
			BitArray faceBack;
			faceBack.SetSize (GetMesh().numFaces);
			faceBack.ClearAll ();
			for (int i=0; i<GetMesh().numFaces; i++) {
				DWORD *vv = GetMesh().faces[i].v;
				IPoint3 A[3];
				for (int j=0; j<3; j++) gw->wTransPoint (&(GetMesh().verts[vv[j]]), &(A[j]));
				IPoint3 d1 = A[1] - A[0];
				IPoint3 d2 = A[2] - A[0];
				if (flip) {
					if ((d1^d2).z > 0) continue;
				} else {
					if ((d1^d2).z < 0) continue;
				}
				for (j=0; j<3; j++) GetMesh().vertHide.Set (vv[j]);
				faceBack.Set (i);
			}
			for (i=0; i<GetMesh().numFaces; i++) {
				if (faceBack[i]) continue;
				DWORD *vv = GetMesh().faces[i].v;
				for (int j=0; j<3; j++) GetMesh().vertHide.Clear (vv[j]);
			}
			GetMesh().vertHide |= oldHide;
		}
		DWORD thisFlags = flags | hitLev;
		if ((selLevel != SL_VERTEX) && localSelByVert) thisFlags |= SUBHIT_USEFACESEL;
		res = GetMesh().SubObjectHitTest(gw, gw->getMaterial(), &hr, thisFlags, hitList);
		if (ignoreBackfaces) GetMesh().vertHide = oldHide;
	} else {
		res = GetMesh().SubObjectHitTest(gw, gw->getMaterial(), &hr, flags|hitLev, hitList);
	}

	rec = hitList.First();
	while (rec) {
		vpt->LogHit(inode,mc,rec->dist,rec->index,NULL);
		rec = rec->Next();
	}

	gw->setRndLimits(savedLimits);	
	return res;
}

void EditTriObject::SelectSubComponent (HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert) {
	if (selLevel == SL_OBJECT) return;
	if (!ip) return;
	if (sliceMode) return;
	ip->ClearCurNamedSelSet ();
	TimeValue t = ip->GetTime();
	EndExtrude (t, FALSE);	// Necessary?
	EndBevel (t, FALSE);	// Necessary?
	int localSelByVert = selByVert;
	if (cutEdgeMode == ip->GetCommandMode()) localSelByVert = FALSE;
	if (turnEdgeMode == ip->GetCommandMode()) localSelByVert = FALSE;
	if (divideEdgeMode == ip->GetCommandMode()) localSelByVert = FALSE;
	if (divideFaceMode == ip->GetCommandMode()) localSelByVert = FALSE;

	AdjFaceList *af=NULL;
	AdjEdgeList *ae=NULL;
	BitArray nsel;
	HitRecord *hr;
	if (selLevel>=SL_POLY) af = TempData()->AdjFList();
	if (localSelByVert) ae = TempData()->AdjEList();

	BaseInterface *msci = GetMesh().GetInterface (MESHSELECTCONVERT_INTERFACE);

	switch (selLevel) {
	case SL_VERTEX:
		nsel = GetMesh().vertSel;
		nsel.SetSize (GetMesh().numVerts, TRUE);
		for (hr=hitRec; hr!=NULL; hr = hr->Next()) {
			nsel.Set (hr->hitInfo, invert ? !GetMesh().vertSel[hr->hitInfo] : selected);
			if (!all) break;
		}
		SetVertSel (nsel, this, t);
		// JBW: macro-recorder
		macroRecorder->FunctionCall(_T("select"), 1, 0, mr_index, mr_prop, _T("verts"), mr_reftarg, this, mr_bitarray, &GetMesh().vertSel);
		break;

	case SL_EDGE:
		if (msci && localSelByVert) {
			// Use new improved selection conversion:
			BitArray vhit;
			vhit.SetSize(GetMesh().numVerts);
			for (hr=hitRec; hr!=NULL; hr=hr->Next()) {
				vhit.Set (hr->hitInfo);
				if (!all) break;
			}
			MeshSelectionConverter *pConv = static_cast<MeshSelectionConverter*>(msci);
			pConv->VertexToEdge(GetMesh(), vhit, nsel);
			if (invert) nsel ^= GetMesh().edgeSel;
			else {
				if (selected) nsel |= GetMesh().edgeSel;
				else nsel = GetMesh().edgeSel & ~nsel;
			}
		} else {
			nsel = GetMesh().edgeSel;
			for (hr=hitRec; hr != NULL; hr=hr->Next()) {
				if (localSelByVert) {
					DWORDTab & list = ae->list[hr->hitInfo];
					for (int i=0; i<list.Count(); i++) {
						MEdge & me = ae->edges[list[i]];
						for (int j=0; j<2; j++) {
							if (me.f[j] == UNDEFINED) continue;
							DWORD ei = GetMesh().faces[me.f[j]].GetEdgeIndex (me.v[0], me.v[1]);
							if (ei>2) continue;
							ei += me.f[j]*3;
							nsel.Set (ei, invert ? !GetMesh().edgeSel[ei] : selected);
						}
					}
				} else {
					nsel.Set (hr->hitInfo, invert ? !nsel[hr->hitInfo] : selected);
				}
				if (!all) break;
			}
		}
		SetEdgeSel (nsel, this, t);
		// JBW: macro-recorder
		macroRecorder->FunctionCall(_T("select"), 1, 0, mr_index, mr_prop, _T("edges"), mr_reftarg, this, mr_bitarray, &GetMesh().edgeSel);
		break;

	case SL_FACE:
		if (msci && localSelByVert) {
			// Use new improved selection conversion:
			BitArray vhit;
			vhit.SetSize(GetMesh().numVerts);
			for (hr=hitRec; hr!=NULL; hr=hr->Next()) {
				vhit.Set (hr->hitInfo);
				if (!all) break;
			}
			MeshSelectionConverter *pConv = static_cast<MeshSelectionConverter*>(msci);
			pConv->VertexToFace(GetMesh(), vhit, nsel);
			if (invert) nsel ^= GetMesh().faceSel;
			else {
				if (selected) nsel |= GetMesh().faceSel;
				else nsel = GetMesh().faceSel & ~nsel;
			}
		} else {
			nsel = GetMesh().faceSel;
			for (hr=hitRec; hr != NULL; hr = hr->Next()) {
				if (localSelByVert) {
					DWORDTab & list = ae->list[hr->hitInfo];
					for (int i=0; i<list.Count(); i++) {
						MEdge & me = ae->edges[list[i]];
						for (int j=0; j<2; j++) {
							if (me.f[j] == UNDEFINED) continue;
							nsel.Set (me.f[j], invert ? !GetMesh().faceSel[me.f[j]] : selected);
						}
					}
				} else {
					nsel.Set (hr->hitInfo, invert ? !GetMesh().faceSel[hr->hitInfo] : selected);
				}
				if (!all) break;
			}
		}
		SetFaceSel (nsel, this, t);

		// JBW: macro-recorder
		macroRecorder->FunctionCall(_T("select"), 1, 0, mr_index, mr_prop, _T("faces"), mr_reftarg, this, mr_bitarray, &GetMesh().faceSel);
		break;

	case SL_POLY:
	case SL_ELEMENT:
		if (msci) {
			// Use new improved selection conversion:
			MeshSelectionConverter *pConv = static_cast<MeshSelectionConverter*>(msci);
			BitArray fhit;
			if (localSelByVert) {
				BitArray vhit;
				vhit.SetSize(GetMesh().numVerts);
				for (hr=hitRec; hr!=NULL; hr=hr->Next()) {
					vhit.Set (hr->hitInfo);
					if (!all) break;
				}
				if (selLevel == SL_ELEMENT) pConv->VertexToElement(GetMesh(), af, vhit, nsel);
				else pConv->VertexToPolygon(GetMesh(), af, vhit, nsel, GetPolyFaceThresh(), ignoreVisEdge?true:false);
			} else {
				fhit.SetSize(GetMesh().numFaces);
				for (hr=hitRec; hr!=NULL; hr=hr->Next()) {
					fhit.Set (hr->hitInfo);
					if (!all) break;
				}
				if (selLevel == SL_ELEMENT) pConv->FaceToElement (GetMesh(), af, fhit, nsel);
				else pConv->FaceToPolygon (GetMesh(), af, fhit, nsel, GetPolyFaceThresh (), ignoreVisEdge?true:false);
			}
		} else {
			// Otherwise we'll take the old approach of converting faces to polygons or elements as we go.
			nsel.SetSize (GetMesh().getNumFaces ());
			for (hr=hitRec; hr != NULL; hr=hr->Next()) {
				if (!localSelByVert) {
					if (selLevel == SL_ELEMENT) GetMesh().ElementFromFace (hr->hitInfo, nsel, af);
					else GetMesh().PolyFromFace (hr->hitInfo, nsel, GetPolyFaceThresh (), ignoreVisEdge, af);
				} else {
					DWORDTab & list = ae->list[hr->hitInfo];
					for (int i=0; i<list.Count(); i++) {
						MEdge & me = ae->edges[list[i]];
						for (int j=0; j<2; j++) {
							if (me.f[j] == UNDEFINED) continue;
							if (selLevel==SL_ELEMENT) GetMesh().ElementFromFace (me.f[j], nsel, af);
							else GetMesh().PolyFromFace (me.f[j], nsel, GetPolyFaceThresh(), ignoreVisEdge, af);
						}
					}
				}
				if (!all) break;
			}
		}
		if (invert) nsel ^= GetMesh().faceSel;
		else {
			if (selected) nsel |= GetMesh().faceSel;
			else nsel = GetMesh().faceSel & ~nsel;
		}
		SetFaceSel (nsel, this, t);

		// JBW: macro-recorder
		macroRecorder->FunctionCall(_T("select"), 1, 0, mr_index, mr_prop, _T("faces"), mr_reftarg, this, mr_bitarray, &GetMesh().faceSel);
		break;
	}
	LocalDataChanged();
}

void EditTriObject::ClearSelection(int sl) {
	if (sliceMode) return;
	BitArray sel;
	switch (sl) {
	case SL_OBJECT: return;
	case SL_VERTEX:
		sel.SetSize (GetMesh().getNumVerts());
		sel.ClearAll ();
		SetVertSel (sel, this, ip->GetTime());
		break;
	case SL_EDGE:
		sel.SetSize (GetMesh().getNumFaces()*3);
		sel.ClearAll();
		SetEdgeSel (sel, this, ip->GetTime());
		break;
	default:
		sel.SetSize (GetMesh().getNumFaces());
		sel.ClearAll ();
		SetFaceSel (sel, this, ip->GetTime());
		break;
	}
	LocalDataChanged ();
}

void EditTriObject::SelectAll(int sl) {
	if (sl == SL_OBJECT) return;
	if (sliceMode) return;
	BitArray sel;
	switch (sl) {
	case SL_VERTEX: 
		sel.SetSize (GetMesh().numVerts);
		sel.SetAll();
		SetVertSel (sel, this, ip->GetTime());
		break;
	case SL_EDGE:   
		sel.SetSize (GetMesh().numFaces*3);
		sel.SetAll(); 
		SetEdgeSel (sel, this, ip->GetTime());
		break;
	default:
		sel.SetSize (GetMesh().numFaces);
		sel.SetAll(); 
		SetFaceSel (sel, this, ip->GetTime());
		break;
	}
	LocalDataChanged ();
}

void EditTriObject::InvertSelection(int sl) {
	if (sl == SL_OBJECT) return;
	if (sliceMode) return;
	BitArray sel;
	switch (sl) {
	case SL_VERTEX: 
		sel = ~GetMesh().vertSel;
		SetVertSel (sel, this, ip->GetTime());
		break;
	case SL_EDGE:
		sel = ~GetMesh().edgeSel;
		SetEdgeSel (sel, this, ip->GetTime());
		break;
	default:
		sel = ~GetMesh().faceSel; 
		SetFaceSel (sel, this, ip->GetTime());
		break;
	}
	LocalDataChanged ();
}

BOOL EditTriObject::SelectSubAnim(int subNum) {
	if (!subNum) return FALSE;	// cannot select master point controller.
	subNum--;
	if (subNum >= GetMesh().numVerts) return FALSE;

	BOOL add = GetKeyState(VK_CONTROL)<0;
	BOOL sub = GetKeyState(VK_MENU)<0;
	BitArray nvs;

	if (add || sub) {
		nvs = GetMesh().vertSel;
		nvs.SetSize (GetMesh().numVerts, TRUE);
		if (sub) nvs.Clear (subNum);
		else nvs.Set (subNum);
	} else {
		nvs.SetSize (GetMesh().numVerts);
		nvs.ClearAll ();
		nvs.Set (subNum);
	}

	if (ip) SetVertSel (nvs, this, ip->GetTime());
	else SetVertSel (nvs, this, TimeValue(0));
	LocalDataChanged ();
	return TRUE;
}

void EditTriObject::ActivateSubobjSel (int level, XFormModes& modes) {
	// Register or unregister delete key notification
	if (selLevel==SL_OBJECT && level!=SL_OBJECT) ip->RegisterDeleteUser(this);
	if (selLevel!=SL_OBJECT && level==SL_OBJECT) ip->UnRegisterDeleteUser(this);

	if ((selLevel != level) && ((level<SL_FACE) || (selLevel<SL_FACE))) {
		ExitAllCommandModes (level == SL_OBJECT, level==SL_OBJECT);
	}

	// Set the meshes level
	SetSubobjectLevel(level);

	if (level == SL_EDGE) GetMesh().InvalidateEdgeList();

	// Fill in modes with our sub-object modes
	if (level!=SL_OBJECT)
		modes = XFormModes(moveMode,rotMode,nuscaleMode,uscaleMode,squashMode,selectMode);

	// Setup named selection sets
	if (level != SL_OBJECT) {
		GenericNamedSelSetList &set = GetSelSet();
		ip->ClearSubObjectNamedSelSets();
		for (int i=0; i<set.Count(); i++) {
			ip->AppendSubObjectNamedSelSet(*(set.names[i]));
		}
	}
	ip->PipeSelLevelChanged();
	UpdateNamedSelDropDown ();
	InvalidateTempData (PART_TOPO|PART_GEOM|PART_SELECT|PART_SUBSEL_TYPE);
	NotifyDependents(FOREVER, SELECT_CHANNEL|DISP_ATTRIB_CHANNEL|SUBSEL_TYPE_CHANNEL, REFMSG_CHANGE);
}

GenericNamedSelSetList &EditTriObject::GetSelSet() {
	return selSet[namedSetLevel[selLevel]];
}

void EditTriObject::GetSubObjectCenters(SubObjAxisCallback *cb,
										TimeValue t, INode *node,ModContext *mc) {
	Matrix3 tm = node->GetObjectTM(t);

	if (sliceMode) {
		cb->Center (sliceCenter*tm, 0);
		return;
	}

	if (selLevel == SL_OBJECT) return;
	if (selLevel == SL_VERTEX) {
		BitArray sel = GetMesh().VertexTempSel();
		Point3 cent(0,0,0);
		int ct=0;
		for (int i=0; i<GetMesh().getNumVerts(); i++) {
			if (sel[i]) {
				cent += GetMesh().verts[i];
				ct++;
			}
		}
		if (ct) {
			cent /= float(ct);			
			cb->Center(cent*tm,0);
		}
		return;
	}

	Tab<Point3> *centers = TempData()->ClusterCenters((selLevel==SL_EDGE) ? MESH_EDGE : MESH_FACE);
	for (int i=0; i<centers->Count(); i++) cb->Center((*centers)[i]*tm,i);
}

void EditTriObject::GetSubObjectTMs (SubObjAxisCallback *cb,TimeValue t,
		INode *node,ModContext *mc) {	
	Matrix3 tm, otm = node->GetObjectTM(t);

	if (sliceMode) {
		Matrix3 rotMatrix(1);
		sliceRot.MakeMatrix (rotMatrix);
		rotMatrix.SetTrans (sliceCenter);
		rotMatrix *= otm;
		cb->TM (rotMatrix, 0);
		return;
	}

	switch (selLevel) {
	case SL_OBJECT:
		break;

	case SL_VERTEX:
		if (ip->GetCommandMode()->ID()==CID_SUBOBJMOVE) {
			Tab<Point3> * vnorms = TempData()->VertexNormals ();
			Matrix3 otm = node->GetObjectTM(t);			
			for (int i=0; i<vnorms->Count(); i++) {
				if (!GetMesh().vertSel[i]) continue;
				Point3 n = (*vnorms)[i];
				n = Normalize(n);
				MatrixFromNormal (n, tm);
				tm = tm * otm;
				tm.SetTrans(GetMesh().verts[i]*otm);
				cb->TM(tm,i);
			}
		} else {
			if (GetMesh().vertSel.NumberSet()==0) return;
			Point3 norm;
			Point3 cent(0,0,0);
			int ct=0;
			
			// Comute average face normal
			norm = AverageSelVertNormal(GetMesh());

			// Compute center of selection
			for (int i=0; i<GetMesh().getNumVerts(); i++) {
				if (GetMesh().vertSel[i]) {
					cent += GetMesh().verts[i];
					ct++;
				}
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
		int i, ct;
		ct = TempData()->ClusterNormals(MESH_EDGE)->Count();
		for (i=0; i<ct; i++) {
			tm = TempData()->ClusterTM (i) * otm;
			cb->TM(tm,i);
		}
		break;

	default:
		ct = TempData()->ClusterNormals(MESH_FACE)->Count();
		for (i=0; i<ct; i++) {
			tm = TempData()->ClusterTM (i) * otm;
			cb->TM(tm,i);
		}
		break;
	}
}

void EditTriObject::ShowEndResultChanged (BOOL showEndResult) {
	if ((!ip) || (editObj != this)) return;
	if ((selLevel == SL_VERTEX) || inBuildFace) {
		if (showEndResult) GetMesh().dispFlags = 0;
		else GetMesh().dispFlags = DISP_VERTTICKS | DISP_SELVERTS;
	}
	NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
}

// --- Restore objects ---------------------------------------------

// Not a real restore object:
TempMoveRestore::TempMoveRestore (EditTriObject *em, bool useMaps) {
	init.SetCount (em->GetMesh().numVerts);
	if (init.Count()) memcpy (init.Addr(0), em->GetMesh().verts, init.Count()*sizeof(Point3));
	active.SetSize (em->GetMesh().numVerts);
	active.ClearAll ();

	if (useMaps) {
		maps.SetCount (em->GetMesh().numMaps + NUM_HIDDENMAPS);
		for (int mp=-NUM_HIDDENMAPS; mp<em->GetMesh().numMaps; mp++) {
			int nmp = NUM_HIDDENMAPS + mp;
			int mvnum = 0;
			if (em->GetMesh().mapSupport(mp)) mvnum = em->GetMesh().Map(mp).getNumVerts();
			if (mvnum) {
				maps[nmp] = new Tab<UVVert>;
				maps[nmp]->SetCount (mvnum);
				memcpy (maps[nmp]->Addr(0), em->GetMesh().Map(mp).tv, mvnum*sizeof(UVVert));
			} else {
				maps[nmp] = NULL;
			}
		}
	} else {
		maps.ZeroCount ();
	}
}

void TempMoveRestore::Restore (EditTriObject *em) {
	if (!init.Count()) return;
	memcpy (em->GetMesh().verts, init.Addr(0), init.Count()*sizeof(Point3));
	for (int nmp=0; nmp<maps.Count(); nmp++) {
		if (!maps[nmp]) continue;
		int mp = nmp - NUM_HIDDENMAPS;
		if (!em->GetMesh().mapSupport(mp)) continue;
		memcpy (em->GetMesh().Map(mp).tv, maps[nmp]->Addr(0), maps[nmp]->Count()*sizeof(UVVert));
	}
}

DWORD TempMoveRestore::ChannelsChanged () {
	DWORD ret = PART_GEOM;
	for (int nmp=0; nmp<maps.Count(); nmp++) {
		if (!maps[nmp]) continue;
		ret |= MapChannelID (nmp-NUM_HIDDENMAPS);
	}
	return ret;
}

MeshSelRestore::MeshSelRestore(EditTriObject *et) {
	this->et = et;
	selLevel    = et->selLevel;
	switch (selLevel) {
	case SL_VERTEX: undo = et->GetMesh().vertSel; break;
	case SL_EDGE:   undo = et->GetMesh().edgeSel; break;
	default:   undo = et->GetMesh().faceSel; break;
	}
}

MeshSelRestore::MeshSelRestore(EditTriObject *et, int selLev) {
	this->et = et;
	selLevel = selLev;
	switch (selLevel) {
	case SL_VERTEX: undo = et->GetMesh().vertSel; break;
	case SL_EDGE:   undo = et->GetMesh().edgeSel; break;
	default:   undo = et->GetMesh().faceSel; break;
	}
}

void MeshSelRestore::Restore(int isUndo) {
	switch (selLevel) {
	case SL_VERTEX: 
		redo = et->GetMesh().vertSel; 
		et->GetMesh().vertSel = undo;
		break;
	case SL_EDGE:   
		redo = et->GetMesh().edgeSel;
		et->GetMesh().edgeSel = undo;
		break;
	default:
		redo = et->GetMesh().faceSel; 
		et->GetMesh().faceSel = undo;
		break;
	}
	et->LocalDataChanged();
}

void MeshSelRestore::Redo() {
	switch (selLevel) {
	case SL_VERTEX:
		et->GetMesh().vertSel = redo;
		break;
	case SL_EDGE:   			
		et->GetMesh().edgeSel = redo;
		break;
	default:
		et->GetMesh().faceSel = redo;
		break;
	}
	et->LocalDataChanged();
}

MeshVertRestore::MeshVertRestore(EditTriObject *et) {
	this->et = et;
	undo.SetCount(et->GetMesh().getNumVerts());
	for (int i=0; i<et->GetMesh().getNumVerts(); i++) undo[i] = et->GetMesh().verts[i];
	rvData = NULL;
	int vdNum = et->GetMesh().vdSupport.GetSize();
	if (!vdNum) {
		uvData = NULL;
		return;
	}
	uvdSupport = et->GetMesh().vdSupport;
	uvData = new PerData[vdNum];
	for (i=0; i<vdNum; i++) uvData[i] = et->GetMesh().vData[i];
}

void MeshVertRestore::Restore(int isUndo) {
	int i, vdNum;
	if (isUndo) {
		redo.SetCount(et->GetMesh().getNumVerts());
		if (redo.Count()) memcpy (redo.Addr(0), et->GetMesh().verts, redo.Count()*sizeof(Point3));
		vdNum = et->GetMesh().vdSupport.GetSize();
		if (vdNum && !rvData) {
			rvdSupport = et->GetMesh().vdSupport;
			rvData = new PerData[vdNum];
			for (i=0; i<vdNum; i++) rvData[i] = et->GetMesh().vData[i];
		}
	}
	et->GetMesh().setNumVerts(undo.Count());
	if (undo.Count()) memcpy (et->GetMesh().verts, undo.Addr(0), undo.Count()*sizeof(Point3));
	et->GetMesh().setNumVData (vdNum = uvdSupport.GetSize());
	if (vdNum) {
		et->GetMesh().vdSupport = uvdSupport;
		for (i=0; i<vdNum; i++) et->GetMesh().vData[i] = uvData[i];
	}
	et->GetMesh().InvalidateGeomCache();
	et->InvalidateTempData (PART_GEOM);
	et->NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
}

void MeshVertRestore::Redo() {
	int i, vdNum;
	et->GetMesh().setNumVerts(redo.Count());
	if (redo.Count()) memcpy (et->GetMesh().verts, redo.Addr(0), redo.Count()*sizeof(Point3));
	et->GetMesh().setNumVData (vdNum = rvdSupport.GetSize());
	if (vdNum) {
		et->GetMesh().vdSupport = rvdSupport;
		for (i=0; i<vdNum; i++) et->GetMesh().vData[i] = rvData[i];
	}
	et->GetMesh().InvalidateGeomCache();
	et->InvalidateTempData (PART_GEOM);
	et->NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
}

MeshMapVertRestore::MeshMapVertRestore (EditTriObject *et, int mapChan) {
	this->et = et;
	mapChannel = mapChan;
	int numMapVerts = et->GetMesh().Map(mapChannel).getNumVerts();
	undo.SetCount (numMapVerts);
	if (!numMapVerts) return;
	memcpy (undo.Addr(0), et->GetMesh().Map(mapChannel).tv, numMapVerts*sizeof(UVVert));
}

// returns true only if there's a difference.
bool MeshMapVertRestore::After () {
	if (undo.Count() == 0) return false;
	int numMapVerts = et->GetMesh().Map(mapChannel).getNumVerts();
	if (numMapVerts == 0) return false;
	if (undo.Count() < numMapVerts) numMapVerts = undo.Count();
	redo.SetCount (numMapVerts);
	memcpy (redo.Addr(0), et->GetMesh().Map(mapChannel).tv, numMapVerts*sizeof(UVVert));
	for (int i=0; i<numMapVerts; i++) {
		if (undo[i] != redo[i]) break;
	}
	if (i==numMapVerts) return false;
	return true;
}

void MeshMapVertRestore::Restore(int isUndo) {
	if (!redo.Count()) After();
	if (!redo.Count()) return;
	UVVert *mv = et->GetMesh().mapVerts(mapChannel);
	if (!mv) return;
	int numMapVerts = et->GetMesh().getNumMapVerts (mapChannel);
	if (undo.Count() < numMapVerts) numMapVerts = undo.Count();
	if (!numMapVerts) return;
	memcpy (mv, undo.Addr(0), numMapVerts*sizeof(UVVert));
	et->InvalidateTempData (MapChannelID(mapChannel));
	et->NotifyDependents(FOREVER, MapChannelID(mapChannel), REFMSG_CHANGE);
}

void MeshMapVertRestore::Redo() {
	UVVert *mv = et->GetMesh().mapVerts(mapChannel);
	if (!mv) return;
	int numMapVerts = et->GetMesh().getNumMapVerts (mapChannel);
	if (redo.Count() < numMapVerts) numMapVerts = redo.Count();
	if (!numMapVerts) return;
	memcpy (mv, redo.Addr(0), numMapVerts*sizeof(UVVert));
	et->InvalidateTempData (MapChannelID(mapChannel));
	et->NotifyDependents(FOREVER, MapChannelID(mapChannel), REFMSG_CHANGE);
}

MeshTopoRestore::MeshTopoRestore(EditTriObject *et, DWORD chan) {
	this->et = et;
	channels = chan|PART_SELECT;
	if (channels & PART_TOPO) channels |= PART_GEOM;
	umesh.DeepCopy (&(et->GetMesh()), channels);
	umesh.numFaces = et->GetMesh().numFaces;	// protects against PART_VERT_COLOR without PART_TOPO.
	ucont = et->cont;
	undone = FALSE;
}

bool MeshTopoRestore::After () {
	if (undone) return true;

	// check for differences.
	bool diff = false;
	// Only checking select channel and vertex color channel for now.
	if (channels != (channels & (SELECT_CHANNEL|VERTCOLOR_CHANNEL))) diff = true;
	if (!diff && (channels & VERTCOLOR_CHANNEL)) {
		for (int mp=-NUM_HIDDENMAPS; mp<1; mp++) {
			if (!(MapChannelID(mp) && VERTCOLOR_CHANNEL)) continue;
			if (!et->GetMesh().mapSupport (mp) && !umesh.mapSupport(mp)) continue;
			if (umesh.getNumMapVerts(mp) != et->GetMesh().getNumMapVerts(mp)) break;
			UVVert *tv1 = umesh.mapVerts(mp), *tv2 = et->GetMesh().mapVerts(mp);
			for (int i=0; i<umesh.getNumMapVerts(mp); i++) if (tv1[i] != tv2[i]) break;
			if (i<umesh.getNumMapVerts(mp)) break;
			TVFace *tf1 = umesh.mapFaces(mp), *tf2 = et->GetMesh().mapFaces(mp);
			for (i=0; i<umesh.numFaces; i++) {
				for (int k=0; k<3; k++) if (tf1[i].t[k] != tf2[i].t[k]) break;
				if (k<3) break;
			}
			if (i<umesh.numFaces) break;
		}
		if (mp<1) diff = true;
	}

	if (!diff && (channels & SELECT_CHANNEL)) {
		if (!(umesh.vertSel == et->GetMesh().vertSel)) diff = true;
		else if (!(umesh.faceSel == et->GetMesh().faceSel)) diff = true;
		else if (!(umesh.edgeSel == et->GetMesh().edgeSel)) diff = true;
	}

	// Save for redo
	rmesh.DeepCopy (&(et->GetMesh()), channels);
	rmesh.numFaces = et->GetMesh().numFaces;	// protects against PART_VERT_COLOR without PART_TOPO.
	rcont = et->cont;
	undone = true;
	return diff;
}

void MeshTopoRestore::Restore(int isUndo) {
	if (!undone) After ();

	et->GetMesh().DeepCopy (&umesh, channels);
	// RK: 05/07/99 Can't just copy the cont tab, has to call 
	// SetPtCont() on every controller to restore master controller properly	
	// RK:5/11/99 -- Replaced it with much shorter ReplaceContArray()
	et->ReplaceContArray(ucont);
	//et->cont = ucont;	
	
	et->LocalDataChanged (channels);
	et->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
}

void MeshTopoRestore::Redo() {
	if (!undone) return;
	et->GetMesh().DeepCopy (&rmesh, channels);
	// RK: 05/07/99 Can't just copy the cont tab, has to call 
	// SetPtCont() on every controller to restore master controller properly
	// RK:5/11/99 -- Replaced it with much shorter ReplaceContArray()
	et->ReplaceContArray(rcont);
	//et->cont = rcont;	
	et->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	et->LocalDataChanged (channels);
}

MeshVertHideRestore::MeshVertHideRestore(EditTriObject *et) {
	this->et = et;
	undo = et->GetMesh().vertHide;
}

void MeshVertHideRestore::Restore(int isUndo) {
	redo = et->GetMesh().vertHide;
	et->GetMesh().vertHide = undo;
	et->GetMesh().InvalidateGeomCache ();
	et->NotifyDependents(FOREVER, DISP_ATTRIB_CHANNEL, REFMSG_CHANGE);
}

void MeshVertHideRestore::Redo() {
	et->GetMesh().vertHide = redo;
	et->GetMesh().InvalidateGeomCache ();
	et->NotifyDependents(FOREVER, DISP_ATTRIB_CHANNEL, REFMSG_CHANGE);
}

MeshFaceHideRestore::MeshFaceHideRestore(EditTriObject *et) {
	this->et = et;
	undo.SetSize(et->GetMesh().getNumFaces());
	for (int i=0; i<et->GetMesh().getNumFaces(); i++) undo.Set(i,et->GetMesh().faces[i].Hidden());
}

void MeshFaceHideRestore::Restore(int isUndo) {
	redo.SetSize(et->GetMesh().getNumFaces());
	for (int i=0; i<et->GetMesh().getNumFaces(); i++) {
		redo.Set(i,et->GetMesh().faces[i].Hidden());
		et->GetMesh().faces[i].SetHide(undo[i]);
	}
	et->GetMesh().InvalidateTopologyCache();
	et->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
}

void MeshFaceHideRestore::Redo() {
	for (int i=0; i<et->GetMesh().getNumFaces(); i++) et->GetMesh().faces[i].SetHide(redo[i]);
	et->GetMesh().InvalidateTopologyCache();
	et->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
}

MeshFaceMatRestore::MeshFaceMatRestore(EditTriObject *et) {
	this->et = et;
	undo.SetCount(et->GetMesh().getNumFaces());
	for (int i=0; i<et->GetMesh().getNumFaces(); i++) undo[i] = et->GetMesh().faces[i].getMatID();
}

void MeshFaceMatRestore::Restore(int isUndo) {
	redo.SetCount(et->GetMesh().getNumFaces());
	for (int i=0; i<et->GetMesh().getNumFaces(); i++) {
		redo[i] = et->GetMesh().faces[i].getMatID();
		et->GetMesh().faces[i].setMatID(undo[i]);
	}
	et->InvalidateSurfaceUI();
	et->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
}

void MeshFaceMatRestore::Redo() {
	for (int i=0; i<et->GetMesh().getNumFaces(); i++) et->GetMesh().faces[i].setMatID(undo[i]);
	et->InvalidateSurfaceUI();
	et->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
}

//--- Saving/Loading --------------------------------

#define VSELSET_CHUNK			0x2845
#define FSELSET_CHUNK			0x2846
#define ESELSET_CHUNK			0x2847
#define SELSET_SET_CHUNK		0x2849
#define SELSET_NAME_CHUNK		0x2850
#define SELSETS_CHUNK			0x3001
#define VERTCOUNT_CHUNKID		0x3002
#define GENSELSET_ID_CHUNK     0x3003
#define GENSELSET_CHUNK     0x3004
#define AR_CHUNK  0x4020
#define FALLOFF_CHUNK 0x4024
#define PINCH_CHUNK 0x4025
#define BUBBLE_CHUNK 0x4026
#define EDIST_CHUNK 0x402c
#define EDGE_ITS_CHUNK 0x402d
#define IG_BACK_CHUNK 0x4030
#define ETOBJ_FLAGS_CHUNK 0x4034
#define ETOBJ_SELLEVEL_CHUNK 0x4038
#define ETOBJ_REF_VERSION_CHUNK  0x403b

// Old pre-3.0 named selection sets:
class NamedSelSetList {
public:
	Tab<BitArray*> sets;
	Tab<TSTR*>	   names;
	int Count() {return sets.Count();}
	IOResult Load(ILoad *iload);
};

IOResult NamedSelSetList::Load(ILoad *iload) {
	IOResult res;
	BitArray *set=NULL;
	TSTR *name=NULL;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
		case SELSET_SET_CHUNK:
			set = new BitArray();
			res = set->Load(iload);
			sets.Append (1, &set);
			break;

		case SELSET_NAME_CHUNK:
			TCHAR *ptr;				
			iload->ReadWStringChunk(&ptr);
			name = new TSTR(ptr);
			names.Append (1, &name);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

/*  No longer used:
IOResult NamedSelSetList::Save(ISave *isave) {
	for (int i=0; i<sets.Count(); i++) {
		isave->BeginChunk(SELSET_SET_CHUNK);
		sets[i]->Save(isave);
		isave->EndChunk();

		isave->BeginChunk(SELSET_NAME_CHUNK);
		isave->WriteWString(*names[i]);
		isave->EndChunk();
	}
	return IO_OK;
}
*/

IOResult EditTriObject::Load(ILoad *iload) {
	ulong nb;
	IOResult res;
	loadRefVersion = 0;
	bool selLevLoaded = FALSE;

	if (iload->PeekNextChunkID()==SELSETS_CHUNK) {
		NamedSelSetList oldVSelSet, oldESelSet, oldFSelSet;
		iload->OpenChunk();
		while (IO_OK==(res=iload->OpenChunk())) {
			switch(iload->CurChunkID()) {
			case VSELSET_CHUNK:
				res = oldVSelSet.Load(iload);
				break;
			case FSELSET_CHUNK:
				res = oldFSelSet.Load(iload);
				break;
			case ESELSET_CHUNK:
				res = oldESelSet.Load(iload);
				break;
			}
			iload->CloseChunk();
			if (res!=IO_OK) return res;
		}
		iload->CloseChunk();
		int i, ct;
		if (ct=oldVSelSet.Count()) {
			selSet[NS_VERTEX].names.SetCount (ct);
			selSet[NS_VERTEX].sets.SetCount (ct);
			selSet[NS_VERTEX].ids.SetCount (ct);
			for (i=0; i<ct; i++) {
				selSet[NS_VERTEX].names[i] = oldVSelSet.names[i];
				selSet[NS_VERTEX].sets[i] = oldVSelSet.sets[i];
				selSet[NS_VERTEX].ids[i] = (DWORD) i;
				oldVSelSet.names[i] = NULL;
				oldVSelSet.sets[i] = NULL;
			}
		}
		if (ct=oldFSelSet.Count()) {
			selSet[NS_FACE].names.SetCount (ct);
			selSet[NS_FACE].sets.SetCount (ct);
			selSet[NS_FACE].ids.SetCount (ct);
			for (i=0; i<ct; i++) {
				selSet[NS_FACE].names[i] = oldFSelSet.names[i];
				selSet[NS_FACE].sets[i] = oldFSelSet.sets[i];
				selSet[NS_FACE].ids[i] = (DWORD) i;
				oldFSelSet.names[i] = NULL;
				oldFSelSet.sets[i] = NULL;
			}
		}
		if (ct=oldESelSet.Count()) {
			selSet[NS_EDGE].names.SetCount (ct);
			selSet[NS_EDGE].sets.SetCount (ct);
			selSet[NS_EDGE].ids.SetCount (ct);
			for (i=0; i<ct; i++) {
				selSet[NS_EDGE].names[i] = oldESelSet.names[i];
				selSet[NS_EDGE].sets[i] = oldESelSet.sets[i];
				selSet[NS_EDGE].ids[i] = (DWORD) i;
				oldESelSet.names[i] = NULL;
				oldESelSet.sets[i] = NULL;
			}
		}
	}

	while (iload->PeekNextChunkID() == GENSELSET_ID_CHUNK) {
		int which;
		iload->OpenChunk();
		res = iload->Read(&which, sizeof(int), &nb);
		iload->CloseChunk ();
		if (res!=IO_OK) return res;

		if (iload->PeekNextChunkID() != GENSELSET_CHUNK) break;
		iload->OpenChunk ();
		res = selSet[which].Load(iload);
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}

	if (iload->PeekNextChunkID()==VERTCOUNT_CHUNKID) {		
		int ct;
		iload->OpenChunk();
		iload->Read(&ct,sizeof(ct),&nb);
		iload->CloseChunk();
		AllocContArray(ct);
		for (int i=0; i<ct; i++) SetPtCont(i, NULL);
	}

	if (iload->PeekNextChunkID () == AR_CHUNK) {
		iload->OpenChunk ();
		if ((res = iload->Read (&affectRegion, sizeof(int), &nb)) != IO_OK) return res;
#ifdef USE_EMESH_SIMPLE_UI
		// always turn affectRegion off since there is no UI for it
		affectRegion = FALSE;
#endif
		iload->CloseChunk ();
	}

	if (iload->PeekNextChunkID() == FALLOFF_CHUNK) {
		iload->OpenChunk ();
		res = iload->Read (&falloff, sizeof(float), &nb);
		if (res != IO_OK) return res;
		iload->CloseChunk ();
	}

	if (iload->PeekNextChunkID() == PINCH_CHUNK) {
		iload->OpenChunk ();
		res = iload->Read (&pinch, sizeof(float), &nb);
		if (res != IO_OK) return res;
		iload->CloseChunk ();
	}

	if (iload->PeekNextChunkID() == BUBBLE_CHUNK) {
		iload->OpenChunk ();
		res = iload->Read (&bubble, sizeof(float), &nb);
		if (res != IO_OK) return res;
		iload->CloseChunk ();
	}

	if (iload->PeekNextChunkID() == EDIST_CHUNK) {
		iload->OpenChunk ();
		res = iload->Read (&useEdgeDist, sizeof(int), &nb);
		if (res != IO_OK) return res;
		iload->CloseChunk ();
	}

	if (iload->PeekNextChunkID() == EDGE_ITS_CHUNK) {
		iload->OpenChunk ();
		res = iload->Read (&edgeIts, sizeof(int), &nb);
		if (res != IO_OK) return res;
		iload->CloseChunk ();
	}

	if (iload->PeekNextChunkID() == IG_BACK_CHUNK) {
		iload->OpenChunk ();
		res = iload->Read (&arIgBack, sizeof(int), &nb);
		if (res != IO_OK) return res;
		iload->CloseChunk ();
	}

	if (iload->PeekNextChunkID() == ETOBJ_FLAGS_CHUNK) {
		iload->OpenChunk ();
		res = iload->Read (&etFlags, sizeof(DWORD), &nb);
		if (res != IO_OK) return res;
		iload->CloseChunk ();
	}

	if (iload->PeekNextChunkID() == ETOBJ_SELLEVEL_CHUNK) {
		iload->OpenChunk ();
		res = iload->Read (&selLevel, sizeof(DWORD), &nb);
		if (res != IO_OK) return res;
		iload->CloseChunk ();
		selLevLoaded = TRUE;
	}

	if (iload->PeekNextChunkID() == ETOBJ_REF_VERSION_CHUNK) {
		iload->OpenChunk ();
		res = iload->Read(&loadRefVersion,sizeof(int), &nb);
		if (res != IO_OK) return res;
		iload->CloseChunk ();
	}

	IOResult ret = TriObject::Load (iload);
	if (!selLevLoaded) {
		switch (GetMesh().selLevel) {
		case MESH_OBJECT: selLevel = SL_OBJECT; break;
		case MESH_VERTEX: selLevel = SL_VERTEX; break;
		case MESH_EDGE: selLevel = SL_EDGE; break;
		case MESH_FACE: selLevel = SL_POLY; break;
		}
	} else {
		// Otherwise, we consider the EMesh selection level to be "dominant".
		// We propegate it to the Mesh level to make sure everything's consistent.
		switch (selLevel)
		{
		case SL_OBJECT: GetMesh().selLevel = MESH_OBJECT; break;
		case SL_VERTEX: GetMesh().selLevel = MESH_VERTEX; break;
		case SL_EDGE: GetMesh().selLevel = MESH_EDGE; break;
		case SL_FACE:
		case SL_POLY:
		case SL_ELEMENT: GetMesh().selLevel = MESH_FACE; break;
		}
	}

#ifdef USE_EMESH_SIMPLE_UI
	// always set to FALSE since there is no UI associated with it
	DoSubdivisionDisplacment() = FALSE;
#endif

	return ret;
}

IOResult EditTriObject::Save(ISave *isave) {	
	int ct = cont.Count();
	ulong nb;

	for (int j=0; j<3; j++) {
		if (!selSet[j].Count()) continue;
		isave->BeginChunk(GENSELSET_ID_CHUNK);
		isave->Write (&j, sizeof(j), &nb);
		isave->EndChunk ();
		isave->BeginChunk (GENSELSET_CHUNK);
		selSet[j].Save(isave);
		isave->EndChunk();
	}
	
	if (ct) {
		isave->BeginChunk(VERTCOUNT_CHUNKID);
		isave->Write(&ct,sizeof(ct),&nb);
		isave->EndChunk();
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

	isave->BeginChunk (ETOBJ_FLAGS_CHUNK);
	isave->Write (&etFlags, sizeof(DWORD), &nb);
	isave->EndChunk();

	isave->BeginChunk (ETOBJ_SELLEVEL_CHUNK);
	isave->Write (&selLevel, sizeof(DWORD), &nb);
	isave->EndChunk ();

	isave->BeginChunk (ETOBJ_REF_VERSION_CHUNK);
	int refVersion = 1;
	isave->Write (&refVersion, sizeof(int), &nb);
	isave->EndChunk ();

	return TriObject::Save(isave);
}

void EditTriObject::ActivateSubSelSet(TSTR &setName) {
	if (selLevel == SL_OBJECT) return;

	BitArray *sset;
	int nsl = namedSetLevel[selLevel];
	sset = selSet[nsl].GetSet(setName);
	if (sset==NULL) return;
	theHold.Begin ();
	SetSel (nsl, *sset, this, ip->GetTime());
	LocalDataChanged ();
	theHold.Accept (GetString (IDS_EM_SELECT));
}

void EditTriObject::NewSetFromCurSel(TSTR &setName) {
	if (selLevel == SL_OBJECT) return;
	BitArray *sset;
	int nsl = namedSetLevel[selLevel];
	sset = selSet[nsl].GetSet(setName);
	if (sset) *sset = GetSel (nsl);
	else selSet[nsl].AppendSet (GetSel(nsl), 0, setName);
}

void EditTriObject::RemoveSubSelSet(TSTR &setName) {
	GenericNamedSelSetList &set = GetSelSet();
	BitArray *ssel = set.GetSet(setName);
	if (ssel) {
		if (theHold.Holding()) theHold.Put (new DeleteSetRestore (setName, &set, this));
		set.RemoveSet (setName);
	}
	ip->ClearCurNamedSelSet();
}

void EditTriObject::SetupNamedSelDropDown() {
	// Setup named selection sets
	if (selLevel == SL_OBJECT) return;
	GenericNamedSelSetList &set = GetSelSet();
	ip->ClearSubObjectNamedSelSets();
	for (int i=0; i<set.Count(); i++) ip->AppendSubObjectNamedSelSet(*(set.names[i]));
}

void EditTriObject::UpdateNamedSelDropDown () {
	if (!ip) return;
	DWORD nsl = namedSetLevel[selLevel];
	GenericNamedSelSetList & ns = selSet[nsl];
	for (int i=0; i<ns.Count(); i++) {
		if (*(ns.sets[i]) == GetSel (nsl)) break;
	}
	if (i<ns.Count()) ip->SetCurNamedSelSet (*(ns.names[i]));
}

int EditTriObject::NumNamedSelSets() {
	GenericNamedSelSetList &set = GetSelSet();
	return set.Count();
}

TSTR EditTriObject::GetNamedSelSetName(int i) {
	GenericNamedSelSetList &set = GetSelSet();
	return *set.names[i];
}

void EditTriObject::SetNamedSelSetName(int i,TSTR &newName) {
	GenericNamedSelSetList &set = GetSelSet();
	if (theHold.Holding()) theHold.Put(new SetNameRestore(i,&set,this));
	*set.names[i] = newName;
}

void EditTriObject::NewSetByOperator (TSTR &newName,Tab<int> &sets,int op) {
	GenericNamedSelSetList &set = GetSelSet();
	BitArray bits = *set.sets[sets[0]];

	for (int i=1; i<sets.Count(); i++) {
		switch (op) {
		case NEWSET_MERGE:
			bits |= *set.sets[sets[i]];
			break;

		case NEWSET_INTERSECTION:
			bits &= *set.sets[sets[i]];
			break;

		case NEWSET_SUBTRACT:
			bits &= ~(*set.sets[sets[i]]);
			break;
		}
	}
	
	set.AppendSet(bits,0,newName);
	if (theHold.Holding()) theHold.Put(new AppendSetRestore(&set,this));
	if (!bits.NumberSet()) RemoveSubSelSet(newName);
}

void EditTriObject::NSCopy() {
	if (selLevel == SL_OBJECT) return;
	int index = SelectNamedSet();
	if (index<0) return;
	if (!ip) return;
	int nsl = namedSetLevel[selLevel];
	GenericNamedSelSetList & setList = selSet[nsl];
	MeshNamedSelClip *clip = new MeshNamedSelClip(*(setList.names[index]));
	BitArray *bits = new BitArray(*setList.sets[index]);
	clip->sets.Append(1,&bits);
	SetMeshNamedSelClip (clip, namedClipLevel[selLevel]);

	// Enable the paste button
	if (hSel) {
		ICustButton *but;
		but = GetICustButton(GetDlgItem(hSel, IDC_PASTE_NS));
		but->Enable();
		ReleaseICustButton(but);
	}
}

void EditTriObject::NSPaste() {
	if (selLevel==SL_OBJECT) return;
	int nsl = namedSetLevel[selLevel];
	MeshNamedSelClip *clip = GetMeshNamedSelClip(namedClipLevel[selLevel]);
	if (!clip) return;
	TSTR name = clip->name;
	if (!GetUniqueSetName(name)) return;

	theHold.Begin ();
	GenericNamedSelSetList & setList = selSet[nsl];
	setList.AppendSet (*clip->sets[0], 0, name);
	if (theHold.Holding()) theHold.Put(new AppendSetRestore(&setList,this));
	ActivateSubSelSet(name);
	theHold.Accept (GetString (IDS_PASTE_NS));
	ip->SetCurNamedSelSet(name);
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

BOOL EditTriObject::GetUniqueSetName(TSTR &name) {
	while (1) {
		GenericNamedSelSetList & setList = selSet[namedSetLevel[selLevel]];

		BOOL unique = TRUE;
		for (int i=0; i<setList.Count(); i++) {
			if (name==*setList.names[i]) {
				unique = FALSE;
				break;
			}
		}
		if (unique) break;

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
		GenericNamedSelSetList *setList = (GenericNamedSelSetList *)lParam;
		for (int i=0; i<setList->Count(); i++) {
			int pos  = SendDlgItemMessage(hWnd,IDC_NS_LIST,LB_ADDSTRING,0,
				(LPARAM)(TCHAR*)*(setList->names[i]));
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

int EditTriObject::SelectNamedSet() {
	GenericNamedSelSetList & setList = selSet[namedSetLevel[selLevel]];
	return DialogBoxParam (hInstance, MAKEINTRESOURCE(IDD_SEL_NAMEDSET),
		ip->GetMAXHWnd(), PickSetDlgProc, (LPARAM)&setList);
}

class NamedSetSizeIncrease : public RestoreObj {
public:
	EditTriObject *et;
	int nsl;
	int oldsize, increase;
	NamedSetSizeIncrease () { et=NULL; }
	NamedSetSizeIncrease (EditTriObject *eto, int n, int old, int inc) { et=eto; nsl=n; oldsize=old; increase=inc; }
	void Restore (int isUndo) { et->selSet[nsl].SetSize (oldsize); }
	void Redo () { et->selSet[nsl].SetSize (oldsize+increase); }
	int GetSize () { return 3*sizeof(int) + sizeof (void *); }
	TSTR Description () { return _T("Named Selection Set Size Increase"); }
};

void EditTriObject::IncreaseNamedSetSize (int nsl, int oldsize, int increase) {
	if (increase == 0) return;
	if (selSet[nsl].Count() == 0) return;
	if (theHold.Holding())
		theHold.Put (new NamedSetSizeIncrease (this, nsl, oldsize, increase));
	selSet[nsl].SetSize (oldsize + increase);
}

class NamedSetDelete : public RestoreObj {
public:
	EditTriObject *et;
	int nsl;
	Tab<BitArray *> oldSets;
	BitArray del;

	NamedSetDelete (EditTriObject *eto, int n, BitArray &d);
	~NamedSetDelete ();
	void Restore (int isUndo);
	void Redo () { et->selSet[nsl].DeleteSetElements (del, (nsl==NS_EDGE) ? 3 : 1); }
	int GetSize () { return 3*sizeof(int) + sizeof (void *); }
	TSTR Description () { return _T("Named Selection Set Subset Deletion"); }
};

NamedSetDelete::NamedSetDelete (EditTriObject *eto, int n, BitArray &d) {
	et = eto;
	nsl = n;
	del = d;
	oldSets.SetCount (et->selSet[nsl].Count());
	for (int i=0; i<et->selSet[nsl].Count(); i++) {
		oldSets[i] = new BitArray;
		(*oldSets[i]) = (*(et->selSet[nsl].sets[i]));
	}
}

NamedSetDelete::~NamedSetDelete () {
	for (int i=0; i<oldSets.Count(); i++) delete oldSets[i];
}

void NamedSetDelete::Restore (int isUndo) {
	int i, max = oldSets.Count();
	if (et->selSet[nsl].Count() < max) max = et->selSet[nsl].Count();
	for (i=0; i<max; i++) *(et->selSet[nsl].sets[i]) = *(oldSets[i]);
}

void EditTriObject::DeleteNamedSetArray (int nsl, BitArray &del) {
	if (del.NumberSet() == 0) return;
	if (selSet[nsl].Count() == 0) return;
	selSet[nsl].Alphabetize ();
	if (theHold.Holding()) 
		theHold.Put (new NamedSetDelete (this, nsl, del));
	selSet[nsl].DeleteSetElements (del, (nsl==NS_EDGE) ? 3 : 1);
}

void EditTriObject::CreateContArray() {
	if (cont.Count()) return;
	AllocContArray (mesh.getNumVerts());
	for (int i=0; i<cont.Count(); i++) SetPtCont(i, NULL);
}

void EditTriObject::SynchContArray(int nv) {
	int i, cct = cont.Count();
	if (!cct) return;
	if (cct == nv) return;
	if (masterCont) masterCont->SetNumSubControllers(nv, TRUE);
	if (cct>nv) {
		cont.Delete (nv, cct-nv);
		return;
	}
	cont.Resize (nv);
	Control *dummy=NULL;
	for (i=cct; i<nv; i++) cont.Append (1, &dummy);
}

void EditTriObject::AllocContArray(int count) {
	cont.SetCount (count);
	if (masterCont) masterCont->SetNumSubControllers (count);
}

void EditTriObject::ReplaceContArray(Tab<Control *> &nc) {
	AllocContArray (nc.Count());
	for(int i=0; i<nc.Count(); i++) SetPtCont (i, nc[i]);
}

// Current ref version is 1.
// Old version (0) had no MasterPointController, just the array of point controllers themselves.
int EditTriObject::RemapRefOnLoad(int iref) {
	if (loadRefVersion == 0) return iref+1;
	return iref;
}

RefTargetHandle EditTriObject::GetReference(int i) {	
	if (i <= ET_MASTER_CONTROL_REF) return masterCont;
	if (i >= (cont.Count() + ET_VERT_BASE_REF)) return NULL;
	return cont[i - ET_VERT_BASE_REF];
}

void EditTriObject::SetReference(int i, RefTargetHandle rtarg) {
	if(i == ET_MASTER_CONTROL_REF) {
		masterCont = (MasterPointControl*)rtarg;
		if (masterCont) masterCont->SetNumSubControllers(cont.Count());
	} else {
		if(i < (mesh.getNumVerts() + ET_VERT_BASE_REF)) {
			if (!cont.Count()) CreateContArray();
			SetPtCont(i - ET_VERT_BASE_REF, (Control*)rtarg); 
		}
	}
}

TSTR EditTriObject::SubAnimName(int i) {
	if (i == ET_MASTER_CONTROL_REF) return GetString(IDS_MASTERCONT);
	TSTR buf;
	if(i < (cont.Count() + ET_VERT_BASE_REF))
		buf.printf(GetString(IDS_RB_POINTNUM), i+1-ET_VERT_BASE_REF);
	return buf;
}

void EditTriObject::DeletePointConts(BitArray &set) {
	if (!cont.Count()) return;

	BOOL deleted = FALSE;
	Tab<Control*> nc;
	nc.SetCount(cont.Count());
	int ix=0;
	for (int i=0; i<cont.Count(); i++) {
		if (!set[i]) nc[ix++] = cont[i];
		else deleted = TRUE;
	}
	nc.SetCount(ix);
	nc.Shrink();
	ReplaceContArray(nc);
}

void EditTriObject::PlugControllersSel(TimeValue t,BitArray &set) {
	BOOL res = FALSE;
	SetKeyModeInterface *ski = GetSetKeyModeInterface(GetCOREInterface());
	if( !ski || !ski->TestSKFlag(SETKEY_SETTING_KEYS) ) {
		if(!AreWeKeying(t)) { return; }
	}
	for (int i=0; i<GetMesh().getNumVerts(); i++) if (set[i] && PlugControl(t,i)) res = TRUE;
	if (res) NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
}

BOOL EditTriObject::PlugControl(TimeValue t,int i) {
	if(!AreWeKeying(t)) return FALSE;
	if (!cont.Count()) CreateContArray();
	if (cont[i]) return FALSE;

	MakeRefByID (FOREVER, i + ET_VERT_BASE_REF, NewDefaultPoint3Controller());
	SuspendAnimate(); 
	AnimateOff();
	SuspendSetKeyMode();
	theHold.Suspend ();
	if(tempMove) cont[i]->SetValue (t, tempMove->init[i]);
	else cont[i]->SetValue (t, &GetMesh().verts[i]);
	ResumeSetKeyMode();
	cont[i]->SetValue (t, &GetMesh().verts[i]);
	theHold.Resume ();
	ResumeAnimate ();
	masterCont->SetSubController (i, cont[i]); 
	return TRUE;
}

void EditTriObject::SetPtCont (int i, Control *c) {
	cont[i]=c;
	if (masterCont/* && c*/) masterCont->SetSubController(i, c);
}

void EditTriObject::SetPointAnim (TimeValue t, int i, Point3 pt) {
	if (cont.Count() && cont[i]) cont[i]->SetValue(t,&pt);
	else GetMesh().verts[i] = pt;
}

void EditTriObject::InvalidateDistances () {
	InvalidateAffectRegion ();
	if (!tempData) return;
	tempData->InvalidateDistances ();
}

void EditTriObject::InvalidateAffectRegion () {
	arValid = NEVER;
	if (!tempData) return;
	tempData->InvalidateAffectRegion ();
}

ObjectState EditTriObject::Eval(TimeValue time) {	
	if (!geomValid.InInterval(time)) {
		arValid = NEVER;
		geomValid = FOREVER;
		for (int i=0; i<cont.Count(); i++) {
			if (cont[i]) cont[i]->GetValue(time,&GetMesh().verts[i],geomValid);
		}
		InvalidateTempData (PART_GEOM);
	}
	
	if (!arValid.InInterval (time)) {
		arValid = geomValid;
		if (affectRegion) {
			GetMesh().SupportVSelectionWeights ();
			float *vsw = GetMesh().getVSelectionWeights();
			if (vsw) {
				Tab<float> *myVSWTable = TempData()->VSWeight (useEdgeDist, edgeIts, arIgBack,
					falloff, pinch, bubble);
				if (myVSWTable && myVSWTable->Count()) {
					float * myVSW = myVSWTable->Addr(0);
					memcpy (vsw, myVSW, GetMesh().getNumVerts()*sizeof(float));
				}
			}
		} else {
			GetMesh().freeVSelectionWeights ();
		}
	}

	// RB 7/11/97: There's no reason for any of the other intervals
	// to be anything but FOREVER. There are some cases where they
	// can end up as an instant point in time
	topoValid   = FOREVER;
	texmapValid = FOREVER;
	selectValid = FOREVER;
	vcolorValid = FOREVER;

	return ObjectState(this);
}

RefResult EditTriObject::NotifyRefChanged (Interval changeInt, RefTargetHandle hTarget,
										   PartID& partID, RefMessage message) {
	switch (message) {
	case REFMSG_CHANGE:
		geomValid.SetEmpty();
		break;
	}
	return REF_SUCCEED;
}

BOOL EditTriObject::CloneVertCont(int from, int to) {	
	if (cont.Count() && cont[from]) {
		RemapDir *remap = NewRemapDir();
		ReplaceReference(to,remap->CloneRef(cont[from]));
		remap->DeleteThis();
		return TRUE;
	}
	return FALSE;
}

BOOL EditTriObject::AssignController (Animatable *control, int subAnim) {
	ReplaceReference (subAnim, (Control*)control);
	if (subAnim==ET_MASTER_CONTROL_REF) {
		int n = cont.Count();
		masterCont->SetNumSubControllers(n);
		for (int i=0; i<n; i++) if (cont[i]) masterCont->SetSubController(i,cont[i]);
	}
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	return TRUE;
}

#ifdef _SUBMTLASSIGNMENT
// CCJ 1/19/98
// Return the sub object material assignment interface
// This is used by the node when assigning materials.
// If an open face selection mode is active, the material
// will be assigned to the selected faces only.
// A multi/sub-object material is created and the material
// is assigned to the matierial ID created for the selected
// faces.
void* EditTriObject::GetInterface(ULONG id) {
	switch (id)
	{
		case I_SUBMTLAPI: return (ISubMtlAPI*)this;
		case I_MESHSELECT: return (IMeshSelect*)this;
		case I_MESHSELECTDATA: return (IMeshSelectData*)this;
		case I_MESHDELTAUSER: return (MeshDeltaUser*)this;
		case I_MESHDELTAUSERDATA: return (MeshDeltaUserData*)this;
	}
	//JH 3/8/99
	//This previously called Object"s implementation
	//Now that kenny has an implementation in triObject, we can't skip it
	return TriObject::GetInterface(id);
}

// Return a material ID that is currently not used by the object.
// If the current face selection share once single MtlDI that is not
// used by any other faces, you should use it.
MtlID EditTriObject::GetNextAvailMtlID(ModContext* mc) {
	int mtlID = GetSelFaceUniqueMtlID(mc);

	if (mtlID == -1) {
		int i;
		BitArray b;
		mtlID = GetMesh().numFaces;
		b.SetSize(GetMesh().numFaces, FALSE);
		b.ClearAll();
		for (i=0; i<GetMesh().numFaces; i++) {
			int mid = GetMesh().faces[i].getMatID();
			if (mid < GetMesh().numFaces) {
				b.Set(mid);
				}
			}

		for (i=0; i<GetMesh().numFaces; i++) {
			if (!b[i]) {
				mtlID = i;
				break;
				}
			}
		}

	return (MtlID)mtlID;
	}

// Indicate if you are active in the modifier panel and have an 
// active face selection
BOOL EditTriObject::HasFaceSelection(ModContext* mc) {
	// Are we the edited object?
	if (ip == NULL)  return FALSE;
	// Is Face selection active?
	if (selLevel < SL_FACE) return FALSE;
	return (GetMesh().faceSel.NumberSet() > 0);
}

// Set the selected faces to the specified material ID.
// If bResetUnsel is TRUE, then you should set the remaining
// faces material ID's to 0
void EditTriObject::SetSelFaceMtlID(ModContext* mc, MtlID id, BOOL bResetUnsel) {
	if (theHold.Holding() && !TestAFlag(A_HELD)) theHold.Put(new MeshFaceMatRestore(this));

	for (int i=0; i<GetMesh().getNumFaces(); i++) {
		if (GetMesh().faceSel[i]) GetMesh().setFaceMtlIndex(i, id);
		else if (bResetUnsel) GetMesh().setFaceMtlIndex(i, 0);
	}

	InvalidateSurfaceUI();
	GetMesh().InvalidateGeomCache();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
}

// Return the material ID of the selected face(s).
// If multiple faces are selected they should all have the same MtlID -
// otherwise you should return -1.
// If faces other than the selected share the same material ID, then 
// you should return -1.
int EditTriObject::GetSelFaceUniqueMtlID(ModContext* mc) {
	int	i;
	int	mtlID;

	mtlID = GetSelFaceAnyMtlID(mc);

	if (mtlID == -1) return mtlID;
	for (i=0; i<GetMesh().numFaces; i++) {
		if (GetMesh().faceSel[i]) continue;
		if (GetMesh().faces[i].getMatID() != mtlID) continue;
		mtlID = -1;
		break;
	}

	return mtlID;
}

// Return the material ID of the selected face(s).
// If multiple faces are selected they should all have the same MtlID,
// otherwise you should return -1.
int EditTriObject::GetSelFaceAnyMtlID(ModContext* mc) {
	int				mtlID = -1;
	BOOL			bGotFirst = FALSE;
	int				i;

	for (i=0; i<GetMesh().numFaces; i++) {
		if (!GetMesh().faceSel[i]) continue;
		if (bGotFirst) {
			if (mtlID != GetMesh().faces[i].getMatID()) {
				mtlID = -1;
				break;
			}
		} else {
			mtlID = GetMesh().faces[i].getMatID();
			bGotFirst = TRUE;
		}
	}

	return mtlID;
}

// Return the highest MtlID used by the object.
int EditTriObject::GetMaxMtlID(ModContext* mc) {
	MtlID mtlID = 0;
	for (int i=0; i<GetMesh().numFaces; i++) mtlID = max(mtlID, GetMesh().faces[i].getMatID());
	return mtlID;
}

#endif // _SUBMTLASSIGNMENT

void EditTriObject::LocalDataChanged () {
	InvalidateNumberSelected ();
	InvalidateSurfaceUI ();
	InvalidateTempData (PART_SELECT);
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	if (ip) {
		ip->RedrawViews(ip->GetTime());
		UpdateNamedSelDropDown();
	}
}

void EditTriObject::SetSelLevel (DWORD lev) {
	switch (lev) {
	case IMESHSEL_OBJECT:
		selLevel = SL_OBJECT;
		break;
	case IMESHSEL_VERTEX:
		selLevel = SL_VERTEX;
		break;
	case IMESHSEL_EDGE:
		selLevel = SL_EDGE;
		break;
	default:
		if (selLevel < SL_FACE) selLevel = SL_POLY;
		break;
	}
	if (ip) ip->SetSubObjectLevel (selLevel);
	else InvalidateTempData (PART_SUBSEL_TYPE);
}

DWORD EditTriObject::GetSelLevel () {
	switch (selLevel) {
	case SL_OBJECT: return IMESHSEL_OBJECT;
	case SL_VERTEX: return IMESHSEL_VERTEX;
	case SL_EDGE: return IMESHSEL_EDGE;
	}
	return IMESHSEL_FACE;
}

BitArray EditTriObject::GetSel (int nsl) {
	switch (nsl) {
	case NS_VERTEX: return GetMesh().vertSel;
	case NS_EDGE: return GetMesh().edgeSel;
	}
	return GetMesh().faceSel;
}

void EditTriObject::SetVertSel(BitArray &set, IMeshSelect *imod, TimeValue t) {
	if (ip) ip->ClearCurNamedSelSet();
	if (theHold.Holding()) theHold.Put (new MeshSelRestore (this, SL_VERTEX));
	GetMesh().vertSel = set;
	if (GetMesh().vertSel.GetSize() != GetMesh().numVerts) GetMesh().vertSel.SetSize (GetMesh().numVerts, TRUE);
	GetMesh().vertSel &= ~GetMesh().vertHide;
}

void EditTriObject::SetFaceSel(BitArray &set, IMeshSelect *imod, TimeValue t) {
	if (ip) ip->ClearCurNamedSelSet();
	if (theHold.Holding()) theHold.Put (new MeshSelRestore (this, SL_FACE));
	GetMesh().faceSel = set;
	if (GetMesh().faceSel.GetSize() != GetMesh().numFaces) GetMesh().faceSel.SetSize (GetMesh().numFaces, TRUE);
	DeselectHiddenFaces (GetMesh());
}

void EditTriObject::SetEdgeSel(BitArray &set, IMeshSelect *imod, TimeValue t) {
	if (ip) ip->ClearCurNamedSelSet();
	if (theHold.Holding()) theHold.Put (new MeshSelRestore (this, SL_EDGE));
	GetMesh().edgeSel = set;
	if (GetMesh().edgeSel.GetSize() != GetMesh().numFaces*3) GetMesh().edgeSel.SetSize (GetMesh().numFaces*3, TRUE);
	DeselectHiddenEdges (GetMesh());
}

void EditTriObject::SetSel (int nsl, BitArray & set, IMeshSelect *imod, TimeValue t) {
	switch (nsl) {
	case NS_VERTEX: SetVertSel (set, imod, t); return;
	case NS_EDGE: SetEdgeSel (set, imod, t); return;
	}
	SetFaceSel (set, imod, t);
}

void EditTriObject::LocalDataChanged (DWORD parts) {
	BOOL sel = (parts & PART_SELECT) ? TRUE : FALSE;
	BOOL topo = (parts & PART_TOPO) ? TRUE : FALSE;
	InvalidateTempData (parts);
	if (sel) InvalidateNumberSelected ();
	if (topo|sel) InvalidateSurfaceUI ();
	if (topo) SynchContArray(GetMesh().numVerts);
	NotifyDependents(FOREVER, parts, REFMSG_CHANGE);
}

class RemoveNormalsRestoreObj : public RestoreObj
{
public:
	RemoveNormalsRestoreObj (EditTriObject *ob, MeshNormalSpec *pSpec) : mpObj(ob)
	{
		mpSpec = (MeshNormalSpec *) pSpec->CloneInterface();
	}

	void Restore (int isUndo)
	{
		MeshNormalSpec *pSpec = mpObj->GetMesh().GetSpecifiedNormals();
		if (pSpec != NULL) *pSpec = *mpSpec;
	}

	void Redo ()
	{
		mpObj->GetMesh().ClearSpecifiedNormals();
	}

	int Size()
	{
		return 8 + mpSpec->GetNumFaces()*12 + mpSpec->GetNumNormals()*12;
	}

	TSTR Description() {return TSTR(_T("Mesh Specified Normal removal"));}

private:
	EditTriObject *mpObj;
	MeshNormalSpec *mpSpec;
};

void EditTriObject::ClearSpecifiedNormals ()
{
	MeshNormalSpec *pSpec = GetMesh().GetSpecifiedNormals();
	if ((pSpec != NULL) && pSpec->GetNumFaces())
	{
		if (theHold.Holding())
			theHold.Put (new RemoveNormalsRestoreObj(this, pSpec));
		pSpec->ClearAndFree ();
	}
}

void EditTriObject::ApplyMeshDelta (MeshDelta & md, MeshDeltaUser *mdu, TimeValue t) {
	// First, we need to clear out any specified normals that might still be hanging around.
	ClearSpecifiedNormals ();

	if (AreWeKeying(t)) {
		BOOL addedCont = FALSE;
		for (int i=0; i<md.vMove.Count(); i++) if (PlugControl(t, md.vMove[i].vid)) addedCont = TRUE;
		if (addedCont) NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	}
	MeshTopoRestore *tchange = NULL;
	DWORD partsChanged = md.PartsChanged ();
	if (theHold.Holding() && !TestAFlag (A_HELD)) {
		if (partsChanged & (PART_TOPO|PART_TEXMAP|PART_VERTCOLOR)) {
			tchange = new MeshTopoRestore (this, partsChanged);
		} else {
			if (partsChanged & PART_GEOM) theHold.Put (new MeshVertRestore (this));
			if (partsChanged & PART_SELECT) theHold.Put (new MeshSelRestore (this));
		}
	}
	for (int i=0; i<md.vMove.Count(); i++) {
		DWORD j = md.vMove[i].vid;
		Point3 pt = GetMesh().verts[j] + md.vMove[i].dv;
		if (cont.Count() && cont[j]) cont[j]->SetValue (t, &pt);
	}
	int nv;
	if (nv = md.vClone.Count()) {
		SynchContArray (GetMesh().numVerts+nv);
		IncreaseNamedSetSize (NS_VERTEX, GetMesh().numVerts, nv);
	}
	if (md.NumFCreate()) {
		IncreaseNamedSetSize (NS_EDGE, GetMesh().numFaces*3, md.NumFCreate()*3);
		IncreaseNamedSetSize (NS_FACE, GetMesh().numFaces, md.NumFCreate());
	}
	md.Apply (GetMesh());
	if (md.vDelete.NumberSet()) {
		DeletePointConts (md.vDelete);
		NotifyDependents (FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
		DeleteNamedSetArray (NS_VERTEX, md.vDelete);
	}
	if (md.fDelete.NumberSet()) {
		DeleteNamedSetArray (NS_EDGE, md.fDelete);
		DeleteNamedSetArray (NS_FACE, md.fDelete);
	}
	mdu->LocalDataChanged (partsChanged);
	if (tchange) {
		if (tchange->After()) theHold.Put (tchange);
		else delete tchange;
	}
}

MeshTempData *EditTriObject::TempData () {
	if (!tempData) tempData = new MeshTempData(&(GetMesh()));
	return tempData;
}

void EditTriObject::InvalidateTempData (PartID parts) {
	if (!tempMove) {
		if (tempData) tempData->Invalidate (parts);
		if (parts & (PART_TOPO|PART_GEOM|PART_SELECT|PART_SUBSEL_TYPE))
			InvalidateAffectRegion ();
	}
	if (parts & PART_TOPO) GetMesh().InvalidateTopologyCache();
	if (parts & PART_GEOM) GetMesh().InvalidateGeomCache ();
}

static int dragRestored;

void EditTriObject::DragMoveInit (bool doMaps) {
	if (tempMove) delete tempMove;
	tempMove = new TempMoveRestore (this, doMaps);
	dragRestored = TRUE;
}

void EditTriObject::DragMoveRestore () {
	if (!tempMove) return;
	if (dragRestored) return;
	tempMove->Restore (this);
	dragRestored = TRUE;
	LocalDataChanged (PART_GEOM);
}

void EditTriObject::DragMove (MeshDelta & md, MeshDeltaUser *mdu) {
	if (!tempMove) {
		ApplyMeshDelta (md, mdu, ip ? ip->GetTime() : TimeValue(0));
		return;
	}
	// only care about vMove:
	for (int i=0; i<md.vMove.Count(); i++) {
		DWORD j = md.vMove[i].vid;
		tempMove->active.Set (j);
		GetMesh().verts[j] += md.vMove[i].dv;
	}
	// And the map equivalents:
	for (int mp=-NUM_HIDDENMAPS; mp<md.GetMapNum(); mp++) {
		if (!md.getMapSupport (mp)) continue;
		for (i=0; i<md.Map(mp).vSet.Count(); i++) {
			DWORD j = md.Map(mp).vSet[i].vid;
			GetMesh().Map(mp).tv[j] = md.Map(mp).vSet[i].v;
		}
	}
	if (theHold.Holding ()) theHold.Put (new CueLocalRestore(this));
	dragRestored = FALSE;
	mdu->LocalDataChanged (PART_GEOM);
}

void EditTriObject::DragMoveAccept (TimeValue t) {
	if (!tempMove) return;
	if (!tempMove->active.NumberSet()) {
		delete tempMove;
		tempMove = NULL;
		return;
	}
	ClearSpecifiedNormals();
	if (AreWeKeying(t)) {
		BOOL addedCont = FALSE;
		for (int i=0; i<tempMove->active.GetSize(); i++) {
			if (!tempMove->active[i]) continue;
			if (!PlugControl(t,i)) continue;
			addedCont = TRUE;
			if (!GetSetKeyMode())cont[i]->SetValue (0, tempMove->init[i]);
		}
		if (addedCont) NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	}
	if (cont.Count()) {
		for (int i=0; i<tempMove->active.GetSize(); i++) {
			if (!tempMove->active[i]) continue;
			if (cont[i]) cont[i]->SetValue (t, GetMesh().verts + i);
		}
	}
	if (theHold.Holding()) {
		MeshVertRestore *mvr = new MeshVertRestore(this);
		memcpy (mvr->undo.Addr(0), tempMove->init.Addr(0), GetMesh().numVerts*sizeof(Point3));
		theHold.Put (mvr);
		for (int mp=-NUM_HIDDENMAPS; mp<GetMesh().getNumMaps(); mp++) {
			if (!GetMesh().mapSupport(mp)) continue;
			int nmp = mp + NUM_HIDDENMAPS;
			if (tempMove->maps.Count() <= nmp) continue;
			if (tempMove->maps[nmp] == NULL) continue;
			MeshMapVertRestore *mmvr = new MeshMapVertRestore (this, mp);
			int numMapVerts = tempMove->maps[nmp]->Count();
			if (mmvr->undo.Count() < numMapVerts) numMapVerts = mmvr->undo.Count();
			if (!numMapVerts) continue;
			memcpy (mmvr->undo.Addr(0), tempMove->maps[nmp]->Addr(0), numMapVerts*sizeof(UVVert));
			if (mmvr->After()) {
				theHold.Put (mmvr);
			} else {
				delete mmvr;
			}
		}
	}
	if (tempData) {
		tempData->Invalidate (tempMove->ChannelsChanged());
		InvalidateAffectRegion ();
		if (affectRegion) NotifyDependents (FOREVER, PART_SELECT, REFMSG_CHANGE);
	}
	delete tempMove;
	tempMove = NULL;
	dragRestored = TRUE;
}

void EditTriObject::DragMoveClear () {
	if (tempMove) delete tempMove;
	tempMove=NULL;
}
