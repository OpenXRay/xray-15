/**********************************************************************
 *<
	FILE: triedui.cpp

	DESCRIPTION: Editable Triangle Mesh Object UI code

	CREATED BY: Rolf Berteig

	HISTORY: created 4 March 1996

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#include "pack1.h"
#include "triobjed.h"
#include "iColorMan.h"
#include "MaxIcon.h"


static INT_PTR CALLBACK SelectDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK AffectRegionDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK SurfaceDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK GeomDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


// xavier robitaille | 03.02.13 | fixed values for metric units
#ifndef METRIC_UNITS_FIXED_VALUES
#define TRIEDUI_WELD_TRESH		0.1f
#else
#define TRIEDUI_WELD_TRESH		DFLT_EDIT_MESH_WELD_TRESHOLD
#endif

static float weldThresh     = TRIEDUI_WELD_TRESH;
static float planarFaceThresh = 45.0f;
static float explodeThresh  = 24.0f;
static float autoEdgeThresh  = 24.0f;
static float autoSmoothThresh = 45.0f;
static int autoEdgeType = 0;	// Set & Clear
static float tessTens       = 25.0f;
static BOOL expObj          = TRUE;
static BOOL edgeTes         = TRUE;
static int sbmParams[2]     = {1,1};
static DWORD sbsParams[3]   = {1,1,0};
static float lastEdgeThresh = 30.0f;
static int selDeltaR		= 10;
static int selDeltaG		= 10;
static int selDeltaB		= 10;
static int selByChannel = 0;
static Point3 selByColor    = Point3(1,1,1);
static int cloneTo = IDC_CLONE_ELEM;
static const int SOFTSEL_EDGEDIST_MIN = 1;
static const int SOFTSEL_EDGEDIST_MAX = 999;

// Local static instance.
	
static GenSubObjType SOT_Vertex(1);
static GenSubObjType SOT_Edge(2);
static GenSubObjType SOT_Face(3);
static GenSubObjType SOT_Poly(4);
static GenSubObjType SOT_Element(5);


void ResetEditableMeshUI() {
	weldThresh     = TRIEDUI_WELD_TRESH;
	planarFaceThresh = 45.0f;
	explodeThresh  = 24.0f;
	autoEdgeThresh = 24.0f;
	autoSmoothThresh = 45.0f;
	autoEdgeType = 0;
	tessTens       = 25.0f;
	expObj         = TRUE;
	edgeTes        = TRUE;
	sbmParams[0]   = 1;
	sbmParams[1]   = 1;
	sbsParams[0]   = 1;
	sbsParams[1]   = 1;
	sbsParams[2]   = 0;
	lastEdgeThresh = 30.0f;
	selDeltaR		= 10;
	selDeltaG		= 10;
	selDeltaB		= 10;
	selByChannel = 0;
	selByColor    = Point3(1,1,1);
	cloneTo = IDC_CLONE_ELEM;
}

class MeshSelImageHandler {
public:
	HIMAGELIST images;
	MeshSelImageHandler () { images = NULL; }
	~MeshSelImageHandler () { if (images) ImageList_Destroy (images); }
	HIMAGELIST LoadImages ();
};

static MeshSelImageHandler theMeshSelImageHandler;

HIMAGELIST MeshSelImageHandler::LoadImages() {
	if (images ) return images;

	HBITMAP hBitmap, hMask;
	images = ImageList_Create(24, 23, ILC_COLOR|ILC_MASK, 10, 0);
	hBitmap = LoadBitmap(hInstance,MAKEINTRESOURCE (IDB_SELTYPES));
	hMask = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_SELMASK));
	ImageList_Add (images, hBitmap, hMask);
	DeleteObject (hBitmap);
	DeleteObject (hMask);
	return images;
}

static int SurfDlgs[] = { 0, IDD_EDTRI_SURF_VERT, IDD_EDTRI_SURF_EDGE,
	IDD_EDTRI_SURF_FACE, IDD_EDTRI_SURF_FACE, IDD_EDTRI_SURF_FACE };

void EditTriObject::UpdateSurfType () {
	if (editObj != this) return;
	if (!hGeom) return;
	if (!hApprox) {
		if (selLevel == SL_OBJECT) {
			HWND hKeyFocus = GetFocus ();
#if !defined(WEBVERSION) && !defined(USE_EMESH_SIMPLE_UI)
			hApprox = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_DISP_APPROX),
									DispApproxDlgProc, GetString(IDS_SCA_SURFACE),
									(LPARAM)this, rsApprox ? 0 : APPENDROLL_CLOSED);
#endif // WEBVERSION
			SetFocus (hKeyFocus);
		}
	} else {
		if (selLevel != SL_OBJECT) {
			rsApprox = IsRollupPanelOpen (hApprox);
			ip->DeleteRollupPage (hApprox);
			hApprox = NULL;
		}
	}
	if (!hSurf) {
		if (selLevel == SL_OBJECT) return;
#ifdef USE_EMESH_SIMPLE_UI
		if ((selLevel == SL_VERTEX) || (selLevel == SL_EDGE)) return;
#endif
		HWND hKeyFocus = GetFocus ();
		hSurf = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(SurfDlgs[selLevel]),
			SurfaceDlgProc, GetString(IDS_SCA_SURFACE), (LPARAM)this, rsSurf ? 0 : APPENDROLL_CLOSED);
		SetFocus (hKeyFocus);
		mtlref->hwnd = hSurf;      
		noderef->hwnd = hSurf;     
		return;
	}
#ifndef USE_EMESH_SIMPLE_UI
	bool wasFace = GetDlgItem (hSurf, IDC_SMOOTH_GRP1) ? TRUE : FALSE;
#else
	bool wasFace = GetDlgItem (hSurf, IDC_SMOOTH_AUTO) ? TRUE : FALSE;
#endif
	bool wasEdge = GetDlgItem (hSurf, IDC_EDGE_VIS) ? TRUE : FALSE;
	bool wasVert = GetDlgItem (hSurf, IDC_VERT_SELCOLOR) ? TRUE : FALSE;
	if (wasVert && (selLevel == SL_VERTEX)) return;
	if (wasEdge && (selLevel == SL_EDGE)) return;
	if (wasFace && (selLevel >= SL_FACE)) return;

	rsSurf = IsRollupPanelOpen (hSurf);
	ip->DeleteRollupPage (hSurf);
	if (selLevel == SL_OBJECT) hSurf = NULL;
#ifdef USE_EMESH_SIMPLE_UI
	else if ((selLevel == SL_VERTEX) || (selLevel == SL_EDGE)) hSurf = NULL;
#endif
	else {
		HWND hFocus = GetFocus ();
		hSurf = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(SurfDlgs[selLevel]),
			SurfaceDlgProc, GetString(IDS_SCA_SURFACE), (LPARAM)this, rsSurf ? 0 : APPENDROLL_CLOSED);
		SetFocus (hFocus);
		mtlref->hwnd = hSurf;      
		noderef->hwnd = hSurf;     
	}
}

void EditTriObject::UpdateSurfaceSpinner (TimeValue t, HWND hWnd, int idSpin) {
	ISpinnerControl *spin = GetISpinner (GetDlgItem (hWnd, idSpin));
	if (!spin) return;

	int num=0;
	float fval;
	DWORD ival;
	bool isInt=false;
	bool indeterminate=false;

	switch (idSpin) {
	case IDC_EM_WEIGHTSPIN:
		fval = GetWeight (t, &num);
		if (fval<0) indeterminate = true;
		if (num==0) indeterminate = true;
		break;

	case IDC_EM_ALPHASPIN:
		fval = GetAlpha (MAP_ALPHA, &num, &indeterminate) * 100.0f;
		break;

	case IDC_MAT_IDSPIN:
	case IDC_MAT_IDSPIN_SEL:                 	
		isInt = true;
		ival = GetMatIndex ();
		if (ival == UNDEFINED) indeterminate = true;
		else ival++;	// Always display one higher.
		num = 1;
		break;
	}

	spin->SetIndeterminate (indeterminate);
	// Set the actual value:
	if (isInt) spin->SetValue (int(ival), FALSE);
	else spin->SetValue (fval, FALSE);
	ReleaseISpinner(spin);
}

static int butIDs[] = { 0, IDC_SELVERTEX, IDC_SELEDGE, IDC_SELFACE, IDC_SELPOLY, IDC_SELELEMENT };
void EditTriObject::RefreshSelType () {
	if (editObj != this) return;
	ICustToolbar *iToolbar = GetICustToolbar(GetDlgItem(hSel,IDC_SELTYPE));
	ICustButton *but;
	for (int i=1; i<6; i++) {
		but = iToolbar->GetICustButton (butIDs[i]);
		but->SetCheck (selLevel==i);
		ReleaseICustButton (but);
	}
	ReleaseICustToolbar(iToolbar);
	ShowNormals ();	// choice of what to show depends on subobject level.
	SetSelDlgEnables();
	SetARDlgEnables();
	SetGeomDlgEnables();
	UpdateSurfType ();
}

static updateNumSel = TRUE;
void EditTriObject::InvalidateNumberSelected () {
	if (editObj != this) return;
	if (!hSel) return;
	InvalidateRect (hSel, NULL, FALSE);
	updateNumSel = TRUE;
}

void EditTriObject::SetNumSelLabel() {	

#ifdef USE_EMESH_SIMPLE_UI
	updateNumSel = FALSE;
	return;
#endif

	static TSTR buf;
	if (!hSel) return;
	if (editObj != this) return;
	if (!updateNumSel) {
		SetDlgItemText (hSel, IDC_NUMSEL_LABEL, buf);
		return;
	}
	updateNumSel = FALSE;

	int num, j;
	switch (selLevel) {
	case SL_OBJECT:
		buf.printf (GetString (IDS_OBJECT_SEL));
		break;

	case SL_VERTEX:
		num = GetMesh().vertSel.NumberSet();
		if (num==1) {
			for (j=0; j<GetMesh().vertSel.GetSize(); j++) if (GetMesh().vertSel[j]) break;
			buf.printf (GetString(IDS_RB_NUMVERTSEL), j+1);
		} else buf.printf (GetString(IDS_RB_NUMVERTSELP), num);
		break;

	case SL_FACE:
	case SL_POLY:
	case SL_ELEMENT:
		num = GetMesh().faceSel.NumberSet();
		if (num==1) {
			for (j=0; j<GetMesh().faceSel.GetSize(); j++) if (GetMesh().faceSel[j]) break;
			buf.printf (GetString(IDS_RB_NUMFACESEL), j+1);
		} else buf.printf(GetString(IDS_RB_NUMFACESELP),num);
		break;

	case SL_EDGE:
		num = GetMesh().edgeSel.NumberSet();
		if (num==1) {
			for (j=0; j<GetMesh().edgeSel.GetSize(); j++) if (GetMesh().edgeSel[j]) break;
			buf.printf (GetString(IDS_RB_NUMEDGESEL), j+1);
		} else buf.printf(GetString(IDS_RB_NUMEDGESELP),num);
		break;
	}

	SetDlgItemText(hSel, IDC_NUMSEL_LABEL, buf);
}

float EditTriObject::GetPolyFaceThresh() {
	return DegToRad (planarFaceThresh);
}

// --- Begin/End Edit Params ---------------------------------

const ActionTableId kEMeshActions = EM_SHORTCUT_ID;
const ActionContextId kEMeshContext = EM_SHORTCUT_ID;
static BOOL oldShowEnd;
static EMeshActionCB *accel=NULL;

void EditTriObject::BeginEditParams (IObjParam  *ip, ULONG flags,Animatable *prev) {
	if (!enabled) return;
#ifdef DESIGN_VER
	//JH 8/11/99
	//Probably throw-away code
	//In the design version we have a compound object called a host composite. It 
	//consists of a host object and several channels which get booleaned into the
	//host. The Hostcomposite provides a shortcut to the parameters of the host 
	//in its own BeginEditParams. This is problematic when the host has subobject
	//types of its own. The HostComposite passes the BEGIN_EDIT_SUPPRESS_SO so we
	//just return from here
	if(flags & BEGIN_EDIT_SUPPRESS_SO)
		return;
#endif
	this->ip = ip;
	editObj = this;
	hSel = ip->AddRollupPage (hInstance, MAKEINTRESOURCE(IDD_EDTRI_SELECT),
		SelectDlgProc, GetString(IDS_SCA_SELECTION), (LPARAM)this, rsSel ? 0 : APPENDROLL_CLOSED);
#ifndef USE_EMESH_SIMPLE_UI
	hAR = ip->AddRollupPage (hInstance, MAKEINTRESOURCE(IDD_EDTRI_AFFECTREGION),
		AffectRegionDlgProc, GetString(IDS_AFFECT_REGION), (LPARAM)this, rsAR ? 0 : APPENDROLL_CLOSED);
#endif
	hGeom = ip->AddRollupPage (hInstance, MAKEINTRESOURCE(IDD_EDTRI_GEOM),
		GeomDlgProc, GetString (IDS_EDIT_GEOM), (LPARAM) this, rsGeom ? 0 : APPENDROLL_CLOSED);

#if !defined(WEBVERSION) && !defined(USE_EMESH_SIMPLE_UI)
	if (selLevel == SL_OBJECT)
		hApprox = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_DISP_APPROX),
									DispApproxDlgProc, GetString(IDS_SCA_SURFACE),
									(LPARAM)this,  rsApprox ? 0 : APPENDROLL_CLOSED);
#endif // WEBVERSION

	if (selLevel == SL_OBJECT) hSurf = NULL;
#ifdef USE_EMESH_SIMPLE_UI
	else if ((selLevel == SL_VERTEX) || (selLevel == SL_EDGE)) hSurf = NULL;
#endif
	else hSurf = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(SurfDlgs[selLevel]),
			SurfaceDlgProc, GetString(IDS_SCA_SURFACE), (LPARAM)this, rsSurf ? 0 : APPENDROLL_CLOSED);

	InvalidateNumberSelected ();

	accel = new EMeshActionCB(this);
	ip->GetActionManager()->ActivateActionTable (accel, kEMeshActions);

	// Create sub object editing modes.
	moveMode       = new MoveModBoxCMode(this,ip);
	rotMode        = new RotateModBoxCMode(this,ip);
	uscaleMode     = new UScaleModBoxCMode(this,ip);
	nuscaleMode    = new NUScaleModBoxCMode(this,ip);
	squashMode     = new SquashModBoxCMode(this,ip);
	selectMode     = new SelectModBoxCMode(this,ip);
	weldVertMode   = new WeldVertCMode(this,ip);
	createVertMode = new CreateVertCMode(this,ip);
	createFaceMode  = new CreateFaceCMode(this,ip);
	divideEdgeMode = new DivideEdgeCMode(this,ip);
	turnEdgeMode   = new TurnEdgeCMode(this,ip);
	extrudeMode    = new ExtrudeCMode(this,ip);
	bevelMode = new BevelCMode (this, ip);
	chamferMode = new ChamferCMode (this, ip);
	divideFaceMode = new DivideFaceCMode (this, ip);
	flipMode       = new FlipNormCMode(this,ip);
	cutEdgeMode   = new CutEdgeCMode (this, ip);

	// SCA 10/29/01: Special handling for pick mode to circumvent defect 293289
	//attachPickMode = new AttachPickMode(this,ip);
	if (!attachPickMode) attachPickMode = new AttachPickMode;
	if (attachPickMode) {
		attachPickMode->eo = this;
		attachPickMode->ip = ip;
	}

	noderef = new SingleRefMakerMeshNode;         //set ref to node
	INode* objNode = GetNode(this);
	if (objNode) {
		noderef->eo = this;
		noderef->SetRef(objNode);                 
	}

	mtlref = new SingleRefMakerMeshMtl;       //set ref for mtl
	mtlref->eo = this;
	if (objNode) {
		Mtl* nodeMtl = objNode->GetMtl();
		mtlref->SetRef(nodeMtl);                        
	}
	      
	// Add our sub object type
	// TSTR type1(GetString(IDS_VERTEX));
	// TSTR type2(GetString (IDS_EDGE));
	// TSTR type3(GetString (IDS_FACE));
	// TSTR type4(GetString (IDS_POLYGON));
	// TSTR type5(GetString (IDS_ELEMENT));
	// const TCHAR *ptype[] = {type1,type2,type3,type4,type5};
	// This call is obsolete. Please see BaseObject::NumSubObjTypes() and BaseObject::GetSubObjType()
	// ip->RegisterSubObjectTypes (ptype, 5);

	// Restore the selection level.
	ip->SetSubObjectLevel(selLevel);

	// We want del key input if in geometry level
	if (selLevel != SL_OBJECT) ip->RegisterDeleteUser(this);

	// Set show end result.
	oldShowEnd = ip->GetShowEndResult();
	ip->SetShowEndResult (GetFlag (ET_DISP_RESULT));

	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);		
	SetAFlag(A_OBJ_BEING_EDITED);	
}

void EditTriObject::EndEditParams (IObjParam *ip, ULONG flags,Animatable *next)
{
	if (!enabled) return;
#ifdef DESIGN_VER
	if(flags & BEGIN_EDIT_SUPPRESS_SO)
		return;
#endif

	if (accel) {
		ip->GetActionManager()->DeactivateActionTable (accel, kEMeshActions);
		delete accel;
		accel = NULL;
	}

	if (hSel) {
		rsSel = IsRollupPanelOpen (hSel);
		ip->DeleteRollupPage (hSel);
		hSel = NULL;
	}
	if (hAR) {
		rsAR = IsRollupPanelOpen (hAR);
		ip->DeleteRollupPage (hAR);
		hAR = NULL;
	}
	if (hGeom) {
		rsGeom = IsRollupPanelOpen (hGeom);
		ip->DeleteRollupPage (hGeom);
		hGeom = NULL;
	}
	if (hApprox) {
		rsApprox = IsRollupPanelOpen (hApprox);
		ip->DeleteRollupPage (hApprox);
		hApprox = NULL;
	}
	if (hSurf) {
		rsSurf = IsRollupPanelOpen (hSurf);
		ip->DeleteRollupPage (hSurf);
		hSurf = NULL;
	}

	// Unregister del key notification
	if (selLevel != SL_OBJECT) ip->UnRegisterDeleteUser(this);

	ExitAllCommandModes ();
	if (moveMode) delete moveMode;
	moveMode = NULL;
	if (rotMode) delete rotMode;
	rotMode = NULL;
	if (uscaleMode) delete uscaleMode;
	uscaleMode = NULL;
	if (nuscaleMode) delete nuscaleMode;
	nuscaleMode = NULL;
	if (squashMode) delete squashMode;
	squashMode = NULL;
	if (selectMode) delete selectMode;
	selectMode = NULL;
	if (weldVertMode) delete weldVertMode;
	weldVertMode = NULL;
	if (createVertMode) delete createVertMode;
	createVertMode = NULL;
	if (createFaceMode) delete createFaceMode;
	createFaceMode = NULL;
	if (divideFaceMode) delete divideFaceMode;
	divideFaceMode = NULL;
	if (divideEdgeMode) delete divideEdgeMode;
	divideEdgeMode = NULL;
	if (turnEdgeMode) delete turnEdgeMode;
	turnEdgeMode = NULL;
	if (extrudeMode) delete extrudeMode;
	extrudeMode = NULL;
	if (bevelMode) delete bevelMode;
	bevelMode = NULL;
	if (chamferMode) delete chamferMode;
	chamferMode = NULL;
	if (flipMode) delete flipMode;
	flipMode = NULL;
	if (cutEdgeMode) delete cutEdgeMode;
	cutEdgeMode = NULL;

	// SCA 10/29/01: Special handling for pick mode to circumvent defect 293289
	//if (attachPickMode) delete attachPickMode;
	//attachPickMode = NULL;
	if (attachPickMode) {
		attachPickMode->eo = NULL;
		attachPickMode->ip = NULL;
	}
	if (noderef) delete noderef;            
	noderef = NULL;                         
	if (mtlref) delete mtlref;             
	mtlref = NULL;                         

	this->ip = NULL;

	if (tempData) {
		delete tempData;
		tempData = NULL;
	}
	if (tempMove) {
		delete tempMove;
		tempMove = NULL;
	}
	editObj = NULL;

	// Reset show end result
	SetFlag (ET_DISP_RESULT, ip->GetShowEndResult());
	ip->SetShowEndResult(oldShowEnd);

	ClearAFlag(A_OBJ_BEING_EDITED);	
}

void EditTriObject::ExitAllCommandModes (bool exSlice, bool exStandardModes) {
	if (exStandardModes) {
		ip->DeleteMode (moveMode);
		ip->DeleteMode (rotMode);
		ip->DeleteMode (uscaleMode);
		ip->DeleteMode (nuscaleMode);
		ip->DeleteMode (squashMode);
		ip->DeleteMode (selectMode);
	}
	ip->DeleteMode (weldVertMode);
	ip->DeleteMode (createVertMode);
	ip->DeleteMode (createFaceMode);	
	ip->DeleteMode (divideFaceMode);
	ip->DeleteMode (divideEdgeMode);	
	ip->DeleteMode (turnEdgeMode);
	ip->DeleteMode (extrudeMode);
	ip->DeleteMode (bevelMode);
	ip->DeleteMode (chamferMode);
	ip->DeleteMode (flipMode);
	ip->DeleteMode (cutEdgeMode);
	ip->ClearPickMode();
	if (exSlice && sliceMode) ExitSliceMode ();
	inCutEdge = FALSE;
	inBuildFace = FALSE;
	inExtrude = FALSE;
	inBevel = FALSE;
	inChamfer = FALSE;
}



// -- Misc. Window procs ----------------------------------------

static int createCurveType   = IDC_CURVE_SMOOTH;
static int curveIgnoreHiddenEdges = TRUE;

static INT_PTR CALLBACK CurveNameDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static TSTR *name = NULL;

	switch (msg) {
	case WM_INITDIALOG:
		name = (TSTR*)lParam;
		SetWindowText (GetDlgItem(hWnd,IDC_CURVE_NAME), name->data());
		CenterWindow (hWnd,GetParent(hWnd));
		SendMessage(GetDlgItem(hWnd,IDC_CURVE_NAME), EM_SETSEL,0,-1);			
		CheckDlgButton(hWnd,createCurveType,TRUE);
		CheckDlgButton(hWnd,IDC_CURVE_IGNOREHIDDEN,curveIgnoreHiddenEdges);
		return FALSE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			name->Resize(GetWindowTextLength(GetDlgItem(hWnd,IDC_CURVE_NAME))+1);
			GetWindowText(GetDlgItem(hWnd,IDC_CURVE_NAME), name->data(), name->length()+1);
			if (IsDlgButtonChecked(hWnd,IDC_CURVE_SMOOTH)) createCurveType = IDC_CURVE_SMOOTH;
			else createCurveType = IDC_CURVE_LINEAR;
			curveIgnoreHiddenEdges = IsDlgButtonChecked(hWnd,IDC_CURVE_IGNOREHIDDEN);
			EndDialog(hWnd,1);
			break;
		
		case IDCANCEL:
			EndDialog(hWnd,0);
			break;
		}
		break;

	default:
		return 0;
	}
	return 1;
}

static BOOL detachToElem = FALSE;
static BOOL detachAsClone = FALSE;

static void SetDetachNameState(HWND hWnd) {
	if (detachToElem) {
		EnableWindow(GetDlgItem(hWnd,IDC_DETACH_NAMELABEL),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_DETACH_NAME),FALSE);
	} else {
		EnableWindow(GetDlgItem(hWnd,IDC_DETACH_NAMELABEL),TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_DETACH_NAME),TRUE);
	}
}

static INT_PTR CALLBACK DetachDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static TSTR *name = NULL;

	switch (msg) {
	case WM_INITDIALOG:
		name = (TSTR*)lParam;
		SetWindowText(GetDlgItem(hWnd,IDC_DETACH_NAME), name->data());
		CenterWindow(hWnd,GetParent(hWnd));
		SendMessage(GetDlgItem(hWnd,IDC_DETACH_NAME), EM_SETSEL,0,-1);
		CheckDlgButton (hWnd, IDC_DETACH_ELEM, detachToElem);
		CheckDlgButton (hWnd, IDC_DETACH_CLONE, detachAsClone);
		if (detachToElem) SetFocus (GetDlgItem (hWnd, IDOK));
		else SetFocus (GetDlgItem (hWnd, IDC_DETACH_NAME));
		SetDetachNameState(hWnd);
		return FALSE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			name->Resize (GetWindowTextLength(GetDlgItem(hWnd,IDC_DETACH_NAME))+1);
			GetWindowText (GetDlgItem(hWnd,IDC_DETACH_NAME),
				name->data(), name->length()+1);
			EndDialog(hWnd,1);
			break;

		case IDC_DETACH_ELEM:
			detachToElem = IsDlgButtonChecked(hWnd,IDC_DETACH_ELEM);
			SetDetachNameState(hWnd);
			break;

		case IDC_DETACH_CLONE:
			detachAsClone = IsDlgButtonChecked (hWnd, IDC_DETACH_CLONE);
			break;

		case IDCANCEL:
			EndDialog(hWnd,0);
			break;
		}
		break;

	default:
		return 0;
	}
	return 1;
}

BOOL GetDetachObjectName (Interface *ip, TSTR &name, BOOL &elem, BOOL &clone) {
	HWND hMax = ip->GetMAXHWnd();
	name = GetString(IDS_OBJECT);
	ip->MakeNameUnique (name);
	if (DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DETACH),
			hMax, DetachDlgProc, (LPARAM)&name)) {
		elem = detachToElem;
		clone = detachAsClone;
		return TRUE;
	} else {
		return FALSE;
	}
}

static void SetCloneNameState(HWND hWnd) {
	switch (cloneTo) {
	case IDC_CLONE_ELEM:
		EnableWindow(GetDlgItem(hWnd,IDC_CLONE_NAME),FALSE);
		break;
	case IDC_CLONE_OBJ:
		EnableWindow(GetDlgItem(hWnd,IDC_CLONE_NAME),TRUE);
		break;
	}
}

static INT_PTR CALLBACK CloneDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static TSTR *name = NULL;

	switch (msg) {
	case WM_INITDIALOG:
		name = (TSTR*)lParam;
		SetWindowText(GetDlgItem(hWnd,IDC_CLONE_NAME), name->data());
		CenterWindow(hWnd, GetParent(hWnd));

		CheckRadioButton (hWnd, IDC_CLONE_OBJ, IDC_CLONE_ELEM, cloneTo);
		if (cloneTo == IDC_CLONE_OBJ) {
			SetFocus(GetDlgItem(hWnd,IDC_CLONE_NAME));
			SendMessage(GetDlgItem(hWnd,IDC_CLONE_NAME), EM_SETSEL,0,-1);
		} else SetFocus (GetDlgItem (hWnd, IDOK));
		SetCloneNameState(hWnd);
		return FALSE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			name->Resize (GetWindowTextLength(GetDlgItem(hWnd,IDC_CLONE_NAME))+1);
			GetWindowText (GetDlgItem(hWnd,IDC_CLONE_NAME),
				name->data(), name->length()+1);
			EndDialog(hWnd,1);
			break;

		case IDCANCEL:
			EndDialog (hWnd, 0);
			break;

		case IDC_CLONE_ELEM:
		case IDC_CLONE_OBJ:
			cloneTo = LOWORD(wParam);
			SetCloneNameState(hWnd);
			break;
		}
		break;

	default:
		return 0;
	}
	return 1;
}

BOOL GetCloneObjectName (Interface *ip, TSTR &name) {
	HWND hMax = ip->GetMAXHWnd();
	name = GetString(IDS_OBJECT);
	if (ip) ip->MakeNameUnique (name);
	DialogBoxParam (hInstance, MAKEINTRESOURCE(IDD_CLONE), hMax, CloneDlgProc, (LPARAM)&name);
	return (cloneTo==IDC_CLONE_OBJ);
}

static INT_PTR CALLBACK ExplodeDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static TSTR *name = NULL;

	switch (msg) {
	case WM_INITDIALOG:
		name = (TSTR*)lParam;
		SetWindowText(GetDlgItem(hWnd,IDC_EXPLODE_NAME), name->data());
		CenterWindow(hWnd, GetParent(hWnd));
		SetFocus(GetDlgItem(hWnd,IDC_EXPLODE_NAME));
		SendMessage(GetDlgItem(hWnd,IDC_EXPLODE_NAME), EM_SETSEL,0,-1);
		return FALSE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			name->Resize (GetWindowTextLength(GetDlgItem(hWnd,IDC_EXPLODE_NAME))+1);
			GetWindowText (GetDlgItem(hWnd,IDC_EXPLODE_NAME),
				name->data(), name->length()+1);
			EndDialog(hWnd,1);
			break;

		case IDCANCEL:
			EndDialog(hWnd,0);
			break;
		}
		break;

	default:
		return 0;
	}
	return 1;
}

BOOL GetExplodeObjectName (HWND hMax, TSTR &name) {
	name = GetString(IDS_OBJECT);
	if (DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_EXPLODE),
		hMax, ExplodeDlgProc, (LPARAM)&name)) {
		return TRUE;
	} else {
		return FALSE;
	}
}

void EditTriObject::SetSelDlgEnables() {
	if (!hSel) return;
	BOOL fac = (selLevel >= SL_FACE);
	BOOL poly = (selLevel == SL_POLY);
	BOOL edg = (selLevel == SL_EDGE);
	BOOL obj = (selLevel == SL_OBJECT);
	BOOL vtx = (selLevel == SL_VERTEX);

	EnableWindow (GetDlgItem (hSel, IDC_SEL_BYVERT), fac||edg);
	EnableWindow (GetDlgItem (hSel, IDC_IGNORE_BACKFACES), !obj);
	EnableWindow (GetDlgItem (hSel,IDC_IGNORE_VISEDGE), poly);
	EnableWindow (GetDlgItem (hSel,IDC_SEL_PT_TEXT), poly);
	ISpinnerControl *spin = GetISpinner(GetDlgItem(hSel,IDC_PLANARSPINNER));
	spin->Enable(poly);
	ReleaseISpinner(spin);

	EnableWindow (GetDlgItem (hSel, IDC_NORMAL_SHOW), vtx||fac);
	// Set value based on sel level.
	if (vtx) CheckDlgButton (hSel, IDC_NORMAL_SHOW, showVNormals);
	if (fac) CheckDlgButton (hSel, IDC_NORMAL_SHOW, showFNormals);
	spin = GetISpinner (GetDlgItem (hSel, IDC_NORMAL_SCALESPIN));
	spin->Enable (vtx||fac);
	ReleaseISpinner (spin);
	EnableWindow (GetDlgItem (hSel, IDC_SCALE_TEXT), vtx||fac);

	ICustButton *but;
	but = GetICustButton (GetDlgItem (hSel, IDC_HIDE));
	but->Enable (vtx||fac);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hSel, IDC_UNHIDEALL));
	but->Enable (vtx||fac);
	ReleaseICustButton (but);

#ifndef USE_EMESH_SIMPLE_UI
	but = GetICustButton (GetDlgItem (hSel, IDC_COPY_NS));
	but->Enable (!obj);
	ReleaseICustButton(but);
	but = GetICustButton (GetDlgItem (hSel, IDC_PASTE_NS));
	but->Enable (!obj && (GetMeshNamedSelClip (namedClipLevel[selLevel]) ? TRUE : FALSE));
	ReleaseICustButton(but);
#endif
}

static INT_PTR CALLBACK SelectDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	EditTriObject *eo = (EditTriObject *) GetWindowLongPtr (hWnd, GWLP_USERDATA);
	ICustToolbar *iToolbar;
	ISpinnerControl *spin;

	switch (msg) {
	case WM_INITDIALOG:
		eo = (EditTriObject*)lParam;
		eo->hSel = hWnd;
		SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);			

		switch (eo->selLevel) {
		case SL_VERTEX:
			CheckDlgButton (hWnd, IDC_NORMAL_SHOW, eo->showVNormals);
			eo->ShowNormals();
			break;
		case SL_FACE:
		case SL_POLY:
		case SL_ELEMENT:
			CheckDlgButton (hWnd, IDC_NORMAL_SHOW, eo->showFNormals);
			eo->ShowNormals();
			break;
		}
		SetupFloatSpinner (hWnd, IDC_NORMAL_SCALESPIN,
			IDC_NORMAL_SCALE, 0.01f, 999999999.0f, eo->normScale, 0.1f);

		spin = GetISpinner(GetDlgItem(hWnd,IDC_PLANARSPINNER));
		spin->SetLimits(0, 180, FALSE);
		spin->SetScale(0.1f);
		spin->LinkToEdit(GetDlgItem(hWnd,IDC_PLANAR), EDITTYPE_FLOAT);
		spin->SetValue(planarFaceThresh,FALSE);
		ReleaseISpinner(spin);

		iToolbar = GetICustToolbar(GetDlgItem(hWnd,IDC_SELTYPE));
		iToolbar->SetImage (theMeshSelImageHandler.LoadImages());
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,0,5,0,5,24,23,24,23,IDC_SELVERTEX));
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,1,6,1,6,24,23,24,23,IDC_SELEDGE));
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,2,7,2,7,24,23,24,23,IDC_SELFACE));
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,3,8,3,8,24,23,24,23,IDC_SELPOLY));
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,4,9,4,9,24,23,24,23,IDC_SELELEMENT));
		ReleaseICustToolbar(iToolbar);
		eo->RefreshSelType();

		CheckDlgButton (hWnd, IDC_SEL_BYVERT, eo->selByVert);
		CheckDlgButton (hWnd, IDC_IGNORE_BACKFACES, eo->ignoreBackfaces);
		CheckDlgButton (hWnd, IDC_IGNORE_VISEDGE, eo->ignoreVisEdge);
		break;

	case CC_SPINNER_CHANGE:
		spin = (ISpinnerControl*)lParam;
		switch (LOWORD(wParam)) {
		case IDC_PLANARSPINNER:
			planarFaceThresh = spin->GetFVal();
			break;

		case IDC_NORMAL_SCALESPIN: 
			eo->normScale = spin->GetFVal();
			if (eo->normScale<.01f) eo->normScale=.01f;
			if (!IsDlgButtonChecked(hWnd,IDC_NORMAL_SHOW)) break;
			eo->ShowNormals (REDRAW_INTERACTIVE);
			break;
		}
		break;

	case CC_SPINNER_BUTTONUP:
	case WM_CUSTEDIT_ENTER:
		switch (LOWORD(wParam)) {
		case IDC_NORMAL_SCALE:
		case IDC_NORMAL_SCALESPIN:
			if (!IsDlgButtonChecked(hWnd,IDC_NORMAL_SHOW)) break;
			eo->ip->RedrawViews(eo->ip->GetTime(),REDRAW_END);
			break;
		}

	case WM_DESTROY:
		// Not sure why... transplanted from surface dialog.  (SCA, Aug 2000)
		eo->ShowNormals();
		break;

	case WM_COMMAND:

		switch (LOWORD(wParam)) {
		case IDC_SELVERTEX:
			if (eo->selLevel == SL_VERTEX) eo->ip->SetSubObjectLevel (SL_OBJECT);
			else eo->ip->SetSubObjectLevel (SL_VERTEX);
			break;

		case IDC_SELEDGE:
			if (eo->selLevel == SL_EDGE) eo->ip->SetSubObjectLevel (SL_OBJECT);
			else eo->ip->SetSubObjectLevel (SL_EDGE);
			break;

		case IDC_SELFACE:
			if (eo->selLevel == SL_FACE) eo->ip->SetSubObjectLevel (SL_OBJECT);
			else eo->ip->SetSubObjectLevel (SL_FACE);
			break;

		case IDC_SELPOLY:
			if (eo->selLevel == SL_POLY) eo->ip->SetSubObjectLevel (SL_OBJECT);
			else eo->ip->SetSubObjectLevel (SL_POLY);
			break;

		case IDC_SELELEMENT:
			if (eo->selLevel == SL_ELEMENT) eo->ip->SetSubObjectLevel (SL_OBJECT);
			else eo->ip->SetSubObjectLevel (SL_ELEMENT);
			break;

		case IDC_SEL_BYVERT:
			eo->selByVert = IsDlgButtonChecked(hWnd,IDC_SEL_BYVERT);
			break;

		case IDC_IGNORE_BACKFACES:
			eo->ignoreBackfaces = IsDlgButtonChecked(hWnd,IDC_IGNORE_BACKFACES);
			break;

		case IDC_IGNORE_VISEDGE:
			eo->ignoreVisEdge = IsDlgButtonChecked(hWnd,IDC_IGNORE_VISEDGE);
			break;

		case IDC_NORMAL_SHOW:
			switch (eo->selLevel) {
			case SL_VERTEX:
				eo->showVNormals = IsDlgButtonChecked (hWnd, IDC_NORMAL_SHOW)?true:false;
				break;
			case SL_FACE:
			case SL_POLY:
			case SL_ELEMENT:
				eo->showFNormals = IsDlgButtonChecked (hWnd, IDC_NORMAL_SHOW)?true:false;
				break;
			}
			eo->ShowNormals();
			break;

		case IDC_HIDE: eo->ButtonOp (MopHide); break;
		case IDC_UNHIDEALL: eo->ButtonOp (MopUnhideAll); break;
		case IDC_COPY_NS: eo->NSCopy(); break;
		case IDC_PASTE_NS: eo->NSPaste(); break;
		}
		break;

	case WM_PAINT:
		if (updateNumSel) eo->SetNumSelLabel ();
		return FALSE;

	case WM_NOTIFY:
		if(((LPNMHDR)lParam)->code != TTN_NEEDTEXT) break;
		LPTOOLTIPTEXT lpttt;
		lpttt = (LPTOOLTIPTEXT)lParam;				
		switch (lpttt->hdr.idFrom) {
		case IDC_SELVERTEX:
			lpttt->lpszText = GetString (IDS_VERTEX);
			break;
		case IDC_SELEDGE:
			lpttt->lpszText = GetString (IDS_EDGE);
			break;
		case IDC_SELFACE:
			lpttt->lpszText = GetString(IDS_FACE);
			break;
		case IDC_SELPOLY:
			lpttt->lpszText = GetString(IDS_POLYGON);
			break;
		case IDC_SELELEMENT:
			lpttt->lpszText = GetString(IDS_ELEMENT);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

#define GRAPHSTEPS 20

static void DrawCurve(HWND hWnd,HDC hdc) {
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
	SetWindowText(GetDlgItem(hWnd,IDC_FARLEFTLABEL), label);
	SetWindowText(GetDlgItem(hWnd,IDC_NEARLABEL), FormatUniverseValue (0.0f));
	SetWindowText(GetDlgItem(hWnd,IDC_FARRIGHTLABEL), label);

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
		float y = AffectRegionFunction(dist,falloff,pinch,bubble);
		int ix = rect.left + int(float(rect.w()-1) * float(i)/float(GRAPHSTEPS));
		int	iy = rect.bottom - int(y*float(rect.h()-2)) - 1;
		if (iy<orect.top) iy = orect.top;
		if (iy>orect.bottom-1) iy = orect.bottom-1;
		LineTo(hdc, ix, iy);
	}
	
	WhiteRect3D(hdc,orect,TRUE);
}

void EditTriObject::SetARDlgEnables () {
	if (!hAR) return;
	ISpinnerControl *spin;
	EnableWindow (GetDlgItem (hAR, IDC_AFFECT_REGION), selLevel);
	bool enable = (selLevel && affectRegion) ? TRUE : FALSE;
	EnableWindow (GetDlgItem (hAR, IDC_E_DIST), enable);
	EnableWindow (GetDlgItem (hAR, IDC_AR_BACK), enable);
	spin = GetISpinner (GetDlgItem (hAR, IDC_E_ITER_SPIN));
	spin->Enable (enable && useEdgeDist);
	ReleaseISpinner (spin);
	spin = GetISpinner (GetDlgItem (hAR, IDC_FALLOFFSPIN));
	spin->Enable (enable);
	ReleaseISpinner (spin);
	spin = GetISpinner (GetDlgItem (hAR, IDC_PINCHSPIN));
	spin->Enable (enable);
	ReleaseISpinner (spin);
	spin = GetISpinner (GetDlgItem (hAR, IDC_BUBBLESPIN));
	spin->Enable (enable);
	ReleaseISpinner (spin);
	EnableWindow (GetDlgItem (hAR, IDC_FALLOFF_LABEL), enable);
	EnableWindow (GetDlgItem (hAR, IDC_PINCH_LABEL), enable);
	EnableWindow (GetDlgItem (hAR, IDC_BUBBLE_LABEL), enable);
}

INT_PTR CALLBACK AffectRegionDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	EditTriObject *em = (EditTriObject*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	ISpinnerControl *spin;
	Rect rect;

	switch (msg) {
	case WM_INITDIALOG:
		em = (EditTriObject*)lParam;
		SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);

		// Set spinners:
		spin = GetISpinner(GetDlgItem(hWnd,IDC_FALLOFFSPIN));
		spin->SetLimits(0.0f,9999999.0f, FALSE);
		spin->SetAutoScale();
		spin->LinkToEdit(GetDlgItem(hWnd,IDC_FALLOFF), EDITTYPE_POS_UNIVERSE);
		spin->SetValue(em->falloff,FALSE);
		spin->SetResetValue (DEF_FALLOFF);
		ReleaseISpinner(spin);

		spin = GetISpinner(GetDlgItem(hWnd,IDC_PINCHSPIN));
		spin->SetLimits(-10.0f,10.0f, FALSE);
		spin->SetScale(0.01f);
		spin->LinkToEdit(GetDlgItem(hWnd,IDC_PINCH), EDITTYPE_FLOAT);
		spin->SetValue(em->pinch,FALSE);
		ReleaseISpinner(spin);

		spin = GetISpinner(GetDlgItem(hWnd,IDC_BUBBLESPIN));
		spin->SetLimits(-10.0f,10.0f, FALSE);
		spin->SetScale(0.01f);
		spin->LinkToEdit(GetDlgItem(hWnd,IDC_BUBBLE), EDITTYPE_FLOAT);
		spin->SetValue(em->bubble,FALSE);
		ReleaseISpinner(spin);
					
		spin = GetISpinner(GetDlgItem(hWnd,IDC_E_ITER_SPIN));
		spin->SetLimits(SOFTSEL_EDGEDIST_MIN, SOFTSEL_EDGEDIST_MAX, FALSE);
		spin->LinkToEdit(GetDlgItem(hWnd,IDC_E_ITER), EDITTYPE_POS_INT);
		spin->SetValue(em->edgeIts,FALSE);
		spin->SetResetValue (1);
		if (!em->affectRegion || !em->useEdgeDist) spin->Disable();
		ReleaseISpinner(spin);

		// Check Checkboxes:
		CheckDlgButton (hWnd, IDC_AFFECT_REGION, em->affectRegion);
		CheckDlgButton(hWnd, IDC_E_DIST, em->useEdgeDist);
		CheckDlgButton (hWnd, IDC_AR_BACK, !em->arIgBack);

		ShowWindow(GetDlgItem(hWnd,IDC_AR_GRAPH),SW_HIDE);
		CenterWindow(hWnd,GetParent(hWnd));
		em->hAR = hWnd;
		em->SetARDlgEnables ();
		break;
		
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd,&ps);
		DrawCurve (hWnd, hdc);
		EndPaint(hWnd,&ps);
		break;
		}

	case CC_SPINNER_CHANGE:
		if (LOWORD(wParam) != IDC_E_ITER_SPIN) {
			GetClientRectP(GetDlgItem(hWnd,IDC_AR_GRAPH),&rect);
			InvalidateRect(hWnd,&rect,FALSE);
		}
		spin = (ISpinnerControl*)lParam;
		switch (LOWORD(wParam)) {
		case IDC_FALLOFFSPIN:
			em->falloff = spin->GetFVal();
			em->InvalidateAffectRegion();
			break;
		case IDC_PINCHSPIN:
			em->pinch = spin->GetFVal();
			em->InvalidateAffectRegion();
			break;
		case IDC_BUBBLESPIN:
			em->bubble = spin->GetFVal();
			em->InvalidateAffectRegion();
			break;
		case IDC_E_ITER_SPIN:
			em->edgeIts = spin->GetIVal ();
			em->InvalidateDistances ();
		}
		em->NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
		em->ip->RedrawViews (em->ip->GetTime(),REDRAW_INTERACTIVE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_AFFECT_REGION:
			em->affectRegion = IsDlgButtonChecked (hWnd,IDC_AFFECT_REGION);
			em->SetARDlgEnables ();
			em->InvalidateDistances ();
			em->NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
			em->ip->RedrawViews(em->ip->GetTime());
			break;

		case IDC_E_DIST:
			em->useEdgeDist = IsDlgButtonChecked (hWnd, IDC_E_DIST);
			spin = GetISpinner (GetDlgItem (hWnd, IDC_E_ITER_SPIN));
			spin->Enable (em->useEdgeDist && em->affectRegion);
			ReleaseISpinner(spin);
			em->InvalidateDistances ();
			em->NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
			em->ip->RedrawViews(em->ip->GetTime());
			break;

		case IDC_AR_BACK:
			em->arIgBack = !IsDlgButtonChecked (hWnd, IDC_AR_BACK);
			em->InvalidateAffectRegion ();
			em->NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
			em->ip->RedrawViews(em->ip->GetTime());
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

void EditTriObject::SetGeomDlgEnables () {
	if (!hGeom) return;
	BOOL edg = (selLevel == SL_EDGE);
	BOOL vtx = (selLevel == SL_VERTEX);
	BOOL fac = (selLevel >= SL_FACE);
	BOOL obj = (selLevel == SL_OBJECT);
	ISpinnerControl *spin;
	ICustButton *but;

#ifndef USE_EMESH_SIMPLE_UI
	but = GetICustButton (GetDlgItem (hGeom, IDC_CREATE));
	but->Enable (vtx||fac);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hGeom, IDC_DETACH));
	but->Enable (!edg);
	if (obj) but->SetText (GetString (IDS_ATTACH_LIST));
	else but->SetText (GetString (IDS_DETACH));
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hGeom, IDC_DIVIDE));
	but->Enable (!obj);
	if (edg||fac) {
		but->SetText (GetString (IDS_DIVIDE));
		but->SetType(CBT_CHECK);
		but->SetHighlightColor(GREEN_WASH);
	} else {
		but->SetText (GetString (IDS_BREAK));
		but->SetType (CBT_PUSH);
	}
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hGeom, IDC_EDGE_TURN));
	but->Enable (edg);
	ReleaseICustButton (but);
	// Align buttons always active.
	but = GetICustButton (GetDlgItem (hGeom, IDC_MAKEPLANAR));
	but->Enable (!obj);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hGeom, IDC_DELETE));
	but->Enable (!obj);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hGeom, IDC_EXTRUDE));
	but->Enable (edg||fac);
	ReleaseICustButton (but);
	spin = GetISpinner (GetDlgItem (hGeom, IDC_EXTRUDESPINNER));
	spin->Enable (edg||fac);
	ReleaseISpinner (spin);
	but = GetICustButton (GetDlgItem (hGeom, IDC_BEVEL));
	but->Enable (!obj);
	if (vtx||edg) but->SetText (GetString (IDS_CHAMFER));
	else but->SetText (GetString (IDS_BEVEL));
	ReleaseICustButton (but);
	spin = GetISpinner (GetDlgItem (hGeom, IDC_OUTLINESPINNER));
	spin->Enable (!obj);
	if (vtx||edg) spin->SetLimits(0.0f, 9999999.0f, FALSE);
	else spin->SetLimits(-9999999.0f, 9999999.0f, FALSE);
	ReleaseISpinner (spin);
	EnableWindow (GetDlgItem (hGeom, IDC_NORMAL_LABEL), fac||edg);
	EnableWindow (GetDlgItem (hGeom, IDC_EXTYPE_A), fac||edg);
	EnableWindow (GetDlgItem (hGeom, IDC_EXTYPE_B), fac||edg);
	but = GetICustButton (GetDlgItem (hGeom, IDC_COLLAPSE));
	but->Enable (!obj);
	ReleaseICustButton (but);
	// It would be nice if Slice Plane were always active, but we can't make it available
	// at the object level, since the transforms won't work.
	but = GetICustButton (GetDlgItem (hGeom, IDC_SLICEPLANE));
	but->Enable (!obj);
	ReleaseICustButton (but);
	EnableWindow (GetDlgItem (hGeom, IDC_SPLIT), !obj);
	but = GetICustButton (GetDlgItem (hGeom, IDC_CUT));
	but->Enable (edg|fac);
	ReleaseICustButton (but);
	EnableWindow (GetDlgItem (hGeom, IDC_REFINE), (edg||fac) && (!sliceMode));

	but = GetICustButton (GetDlgItem (hGeom, IDC_TESSELLATE));
	but->Enable (fac);
	ReleaseICustButton (but);
	spin = GetISpinner (GetDlgItem (hGeom, IDC_TENSIONSPINNER));
	spin->Enable (fac && edgeTes);
	ReleaseISpinner (spin);
	EnableWindow (GetDlgItem (hGeom, IDC_TES_TEXT), fac);
	EnableWindow (GetDlgItem (hGeom, IDC_TES_EDGE), fac);
	EnableWindow (GetDlgItem (hGeom, IDC_TES_CENTER), fac);

	but = GetICustButton (GetDlgItem (hGeom, IDC_EXPLODE));
	but->Enable (fac||obj);
	ReleaseICustButton (but);
	spin = GetISpinner (GetDlgItem (hGeom, IDC_ANGLETHRESHSPIN));
	spin->Enable (fac || obj);
	ReleaseISpinner (spin);
	EnableWindow (GetDlgItem (hGeom, IDC_EXP_TEXT), fac||obj);
	EnableWindow (GetDlgItem (hGeom, IDC_EXP_ELEMENTS), fac||obj);
	EnableWindow (GetDlgItem (hGeom, IDC_EXP_OBJECTS), fac||obj);

	but = GetICustButton (GetDlgItem (hGeom, IDC_WELD));
	but->Enable (vtx);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hGeom, IDC_WELDTOVERT));
	but->Enable (vtx);
	ReleaseICustButton (but);
	EnableWindow (GetDlgItem (hGeom, IDC_PIXELS_TEXT), vtx);
	spin = GetISpinner (GetDlgItem (hGeom, IDC_W_THR_SPIN));
	spin->Enable (vtx);
	ReleaseISpinner (spin);
	spin = GetISpinner (GetDlgItem (hGeom, IDC_T_THR_SPIN));
	spin->Enable (vtx);
	ReleaseISpinner (spin);

	but = GetICustButton (GetDlgItem (hGeom, IDC_SELECT_OPEN));
	but->Enable (edg);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hGeom, IDC_CREATE_CURVE));
	but->Enable (edg);
	ReleaseICustButton (but);
#else
	but = GetICustButton (GetDlgItem (hGeom, IDC_DELETE));
	but->Enable (!obj);
	ReleaseICustButton (but);

	but = GetICustButton (GetDlgItem (hGeom, IDC_DETACH));
	but->Enable (!edg && !obj && !vtx);
	ReleaseICustButton (but);

	but = GetICustButton (GetDlgItem (hGeom, IDC_WELD));
	but->Enable (vtx);
	ReleaseICustButton (but);

	spin = GetISpinner (GetDlgItem (hGeom, IDC_W_THR_SPIN));
	spin->Enable (vtx);
	ReleaseISpinner (spin);
#endif	// USE_EMESH_SIMPLE_UI
}

static INT_PTR CALLBACK GeomDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	EditTriObject *eo = (EditTriObject*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	ISpinnerControl *spin;
	ICustButton *but;
	TSTR name;
	//MeshDelta tmd;

	switch (msg) {
	case WM_INITDIALOG:
		// Record the ETO * in the window's long.
		eo = (EditTriObject*)lParam;
		eo->hGeom = hWnd;
		SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);

#ifndef USE_EMESH_SIMPLE_UI
		// Set up the "depressed" color for the command-mode buttons
		but = GetICustButton(GetDlgItem(hWnd,IDC_CREATE));
		but->SetType(CBT_CHECK);
		but->SetHighlightColor(GREEN_WASH);
		ReleaseICustButton(but);

		but = GetICustButton(GetDlgItem(hWnd,IDC_DIVIDE));
		but->SetType(CBT_CHECK);
		but->SetHighlightColor(GREEN_WASH);
		ReleaseICustButton(but);

		but = GetICustButton(GetDlgItem(hWnd,IDC_EDGE_TURN));
		but->SetType(CBT_CHECK);
		but->SetHighlightColor(GREEN_WASH);
		ReleaseICustButton(but);

		but = GetICustButton(GetDlgItem(hWnd,IDC_EXTRUDE));
		but->SetType(CBT_CHECK);
		but->SetHighlightColor(GREEN_WASH);
		ReleaseICustButton(but);

		but = GetICustButton(GetDlgItem(hWnd,IDC_BEVEL));
		but->SetType(CBT_CHECK);
		but->SetHighlightColor(GREEN_WASH);
		ReleaseICustButton(but);

		but = GetICustButton(GetDlgItem(hWnd,IDC_OBJ_ATTACH));
		but->SetType(CBT_CHECK);
		but->SetHighlightColor(GREEN_WASH);
		ReleaseICustButton(but);					

		but = GetICustButton(GetDlgItem(hWnd,IDC_SLICEPLANE));
		but->SetType(CBT_CHECK);
		but->SetHighlightColor(GREEN_WASH);
		but->SetCheck (eo->sliceMode);
		ReleaseICustButton(but);

		but = GetICustButton(GetDlgItem(hWnd,IDC_CUT));
		but->SetType(CBT_CHECK);
		but->SetHighlightColor(GREEN_WASH);
		ReleaseICustButton(but);

		but = GetICustButton(GetDlgItem(hWnd,IDC_WELDTOVERT));
		but->SetType(CBT_CHECK);
		but->SetHighlightColor(GREEN_WASH);
		ReleaseICustButton(but);

		// Set up spinners
		spin = GetISpinner(GetDlgItem(hWnd,IDC_EXTRUDESPINNER));
		spin->SetLimits(-9999999.0f, 9999999.0f, FALSE);
		spin->LinkToEdit (GetDlgItem (hWnd,IDC_EXTRUDEAMOUNT), EDITTYPE_UNIVERSE);
		spin->SetScale (.1f);
		ReleaseISpinner (spin);

		spin = GetISpinner(GetDlgItem(hWnd,IDC_OUTLINESPINNER));
		spin->SetLimits(-9999999.0f, 9999999.0f, FALSE);
		spin->LinkToEdit (GetDlgItem (hWnd,IDC_OUTLINEAMOUNT), EDITTYPE_UNIVERSE);
		spin->SetScale (.1f);
		ReleaseISpinner (spin);

		spin = GetISpinner(GetDlgItem(hWnd,IDC_TENSIONSPINNER));
		spin->SetLimits(-100,100, FALSE);
		spin->LinkToEdit(GetDlgItem(hWnd,IDC_TENSION), EDITTYPE_FLOAT);
		spin->SetValue(tessTens,FALSE);
		ReleaseISpinner(spin);

		spin = GetISpinner (GetDlgItem (hWnd,IDC_ANGLETHRESHSPIN));
		spin->SetLimits(0, 180, FALSE);
		spin->SetScale(0.1f);
		spin->LinkToEdit(GetDlgItem(hWnd,IDC_ANGLETHRESH), EDITTYPE_FLOAT);
		spin->SetValue(explodeThresh,FALSE);
		ReleaseISpinner(spin);

		spin = GetISpinner (GetDlgItem(hWnd, IDC_W_THR_SPIN));
		spin->SetLimits(0,9999999, FALSE);
		spin->SetAutoScale();
		spin->LinkToEdit(GetDlgItem(hWnd,IDC_W_THR), EDITTYPE_POS_UNIVERSE);
		spin->SetValue(weldThresh,FALSE);
		ReleaseISpinner(spin);

		spin = GetISpinner(GetDlgItem(hWnd,IDC_T_THR_SPIN));
		spin->SetLimits(1,1000, FALSE);
		spin->SetScale(0.1f);
		spin->LinkToEdit(GetDlgItem(hWnd,IDC_T_THR), EDITTYPE_INT);
		spin->SetValue(eo->weldBoxSize,FALSE);
		ReleaseISpinner(spin);

		// Take care of Explode and Tessellation radios:
		if (expObj) {
			CheckDlgButton (hWnd, IDC_EXP_OBJECTS, TRUE);
			CheckDlgButton (hWnd, IDC_EXP_ELEMENTS, FALSE);
		} else {
			CheckDlgButton (hWnd, IDC_EXP_OBJECTS, FALSE);
			CheckDlgButton (hWnd, IDC_EXP_ELEMENTS, TRUE);
		}

		if (edgeTes) {
			CheckDlgButton (hWnd, IDC_TES_EDGE, TRUE);
			CheckDlgButton (hWnd, IDC_TES_CENTER, FALSE);
		} else {
			CheckDlgButton(hWnd,IDC_TES_EDGE,FALSE);
			CheckDlgButton(hWnd,IDC_TES_CENTER,TRUE);				
		}

		if (eo->extType == MESH_EXTRUDE_CLUSTER) {
			CheckRadioButton (hWnd, IDC_EXTYPE_A, IDC_EXTYPE_B, IDC_EXTYPE_A);
		} else {
			CheckRadioButton (hWnd, IDC_EXTYPE_A, IDC_EXTYPE_B, IDC_EXTYPE_B);
		}

		// Set Slice button to be grey if not in Slice Plane mode.
		but = GetICustButton (GetDlgItem (hWnd, IDC_SLICE));
		but->Enable (eo->sliceMode);
		ReleaseICustButton (but);

		// Check boxes as appropriate
		CheckDlgButton(hWnd, IDC_SPLIT, eo->sliceSplit);
		CheckDlgButton(hWnd, IDC_REFINE, eo->cutRefine);
#else
		but = NULL; // get rid of warning without messing code up more

		spin = GetISpinner (GetDlgItem(hWnd, IDC_W_THR_SPIN));
		spin->SetLimits(0,9999999, FALSE);
		spin->SetAutoScale();
		spin->LinkToEdit(GetDlgItem(hWnd,IDC_W_THR), EDITTYPE_POS_UNIVERSE);
		spin->SetValue(weldThresh,FALSE);
		ReleaseISpinner(spin);

		spin = GetISpinner (GetDlgItem(hWnd, IDC_W_THR_SPIN));
		spin->SetLimits(0,9999999, FALSE);
		spin->SetAutoScale();
		spin->LinkToEdit(GetDlgItem(hWnd,IDC_W_THR), EDITTYPE_POS_UNIVERSE);
		spin->SetValue(weldThresh,FALSE);
		ReleaseISpinner(spin);
#endif	// USE_EMESH_SIMPLE_UI

		eo->SetGeomDlgEnables();
		break;

	case CC_SPINNER_BUTTONDOWN:
		switch (LOWORD(wParam)) {
		case IDC_EXTRUDESPINNER:
			eo->BeginExtrude (eo->ip->GetTime());
			break;

		case IDC_OUTLINESPINNER:
			switch (eo->selLevel) {
			case SL_VERTEX:
			case SL_EDGE:
				eo->BeginChamfer (eo->ip->GetTime ());
				break;
			case SL_FACE:
			case SL_POLY:
			case SL_ELEMENT:
				eo->BeginBevel (eo->ip->GetTime (), FALSE);
				break;
			}
			break;
		}
		break;

	case CC_SPINNER_BUTTONUP:
		switch (LOWORD(wParam)) {
		case IDC_EXTRUDESPINNER:
			eo->EndExtrude (eo->ip->GetTime(),HIWORD(wParam));
			eo->ip->RedrawViews (eo->ip->GetTime(), REDRAW_END);
			break;

		case IDC_OUTLINESPINNER:
			switch (eo->selLevel) {
			case SL_VERTEX:
			case SL_EDGE:
				eo->EndChamfer (eo->ip->GetTime (), HIWORD(wParam));
				break;
			case SL_FACE:
			case SL_POLY:
			case SL_ELEMENT:
				eo->EndBevel (eo->ip->GetTime(), HIWORD(wParam));
				break;
			}
			eo->ip->RedrawViews (eo->ip->GetTime(), REDRAW_END);
			break;
		}
		break;

	case CC_SPINNER_CHANGE:
		spin = (ISpinnerControl*)lParam;

		switch (LOWORD(wParam)) {
		case IDC_T_THR_SPIN: eo->weldBoxSize = spin->GetIVal(); break;
		case IDC_W_THR_SPIN: weldThresh = spin->GetFVal (); break;
		case IDC_ANGLETHRESHSPIN: explodeThresh = spin->GetFVal (); break;
		case IDC_TENSIONSPINNER: tessTens = spin->GetFVal (); break;

		case IDC_EXTRUDESPINNER:
			// NOTE: In future, we should use WM_CUSTEDIT_ENTER
			// to complete the keyboard entry, not this enterKey variable.
			// (See Editable Poly for example.)
			bool enterKey;
			enterKey = FALSE;
			if (!HIWORD(wParam) && !eo->inExtrude) {
				enterKey = TRUE;
				eo->BeginExtrude(eo->ip->GetTime());
			}
			eo->Extrude (eo->ip->GetTime(),spin->GetFVal());
			if (enterKey) {
				eo->EndExtrude (eo->ip->GetTime(),TRUE);
				eo->ip->RedrawViews (eo->ip->GetTime(),REDRAW_END);
			} else {
				eo->ip->RedrawViews (eo->ip->GetTime(),REDRAW_INTERACTIVE);
			}
			break;

		case IDC_OUTLINESPINNER:
			// NOTE: In future, we should use WM_CUSTEDIT_ENTER
			// to complete the keyboard entry, not this enterKey variable.
			// (See Editable Poly for example.)
			enterKey = FALSE;
			switch (eo->selLevel) {
			case SL_VERTEX:
			case SL_EDGE:
				if (!HIWORD(wParam) && !eo->inChamfer) {
					enterKey = TRUE;
					eo->BeginChamfer (eo->ip->GetTime ());
				}
				eo->Chamfer (eo->ip->GetTime (), spin->GetFVal ());
				if (enterKey) eo->EndChamfer (eo->ip->GetTime (), TRUE);
				break;

			default:
				if (!HIWORD(wParam) && !eo->inBevel) {
					enterKey = TRUE;
					eo->BeginBevel (eo->ip->GetTime ());
				}
				eo->Bevel (eo->ip->GetTime (), spin->GetFVal ());
				if (enterKey) eo->EndBevel (eo->ip->GetTime (), TRUE);
				break;
			}
			if (enterKey) eo->ip->RedrawViews (eo->ip->GetTime(), REDRAW_END);
			else eo->ip->RedrawViews (eo->ip->GetTime(),REDRAW_INTERACTIVE);
			break;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CREATE: eo->ToggleCommandMode (McmCreate); break;

		case IDC_DETACH:
			if (eo->selLevel == SL_OBJECT) {
				// Really an attach multiple button.
				AttachHitByName proc(eo);
				eo->ip->DoHitByNameDialog(&proc);
			} else eo->ButtonOp (MopDetach);
			break;

		case IDC_DIVIDE:
			switch (eo->selLevel) {
			case SL_OBJECT: break;
			case SL_VERTEX: eo->ButtonOp (MopBreak); break;
			default: eo->ToggleCommandMode (McmDivide); break;
			}
			break;

		case IDC_EDGE_TURN: eo->ToggleCommandMode (McmTurnEdge); break;
		case IDC_ALIGNVIEW: eo->ButtonOp (MopViewAlign); break;
		case IDC_ALIGNCONST: eo->ButtonOp (MopGridAlign); break;
		case IDC_MAKEPLANAR: eo->ButtonOp (MopMakePlanar); break;
		case IDC_DELETE: eo->ButtonOp (MopDelete); break;
		case IDC_EXTRUDE: eo->ToggleCommandMode (McmExtrude); break;

		case IDC_BEVEL:
			switch (eo->selLevel) {
			case SL_VERTEX:
			case SL_EDGE:
				eo->ToggleCommandMode (McmChamfer);
				break;
			case SL_FACE:
			case SL_POLY:
			case SL_ELEMENT:
				eo->ToggleCommandMode (McmBevel);
				break;
			}
			break;

		case IDC_EXTYPE_A:
			eo->extType = MESH_EXTRUDE_CLUSTER;
			break;

		case IDC_EXTYPE_B:
			eo->extType = MESH_EXTRUDE_LOCAL;
			break;

		case IDC_COLLAPSE: eo->ButtonOp (MopCollapse); break;
		case IDC_OBJ_ATTACH: eo->ToggleCommandMode (McmAttach); break;
		case IDC_SLICEPLANE: eo->ToggleCommandMode (McmSlicePlane); break;
		case IDC_CUT: eo->ToggleCommandMode (McmCut); break;
		case IDC_SLICE: eo->ButtonOp (MopSlice); break;

		case IDC_REFINE:
			eo->cutRefine = IsDlgButtonChecked(hWnd,IDC_REFINE) ? TRUE : FALSE;
			break;

		case IDC_SPLIT:
			eo->sliceSplit = IsDlgButtonChecked(hWnd,IDC_SPLIT) ? TRUE : FALSE;
			break;

		case IDC_TESSELLATE: eo->ButtonOp (MopTessellate); break;

		case IDC_TES_EDGE:
			edgeTes = TRUE;
			spin = GetISpinner (GetDlgItem (hWnd,IDC_TENSIONSPINNER));
			spin->Enable();
			ReleaseISpinner(spin);
			break;

		case IDC_TES_CENTER:
			edgeTes = FALSE;
			spin = GetISpinner (GetDlgItem (hWnd,IDC_TENSIONSPINNER));
			spin->Disable();
			ReleaseISpinner(spin);
			break;

		case IDC_EXPLODE: eo->ButtonOp (MopExplode); break;

		case IDC_EXP_OBJECTS:
			expObj = TRUE;
			break;

		case IDC_EXP_ELEMENTS:
			expObj = FALSE;
			break;

		case IDC_WELD: eo->ButtonOp (MopWeld); break;
		case IDC_WELDTOVERT: eo->ToggleCommandMode (McmWeldTarget); break;
		case IDC_REMOVE_ISO_VERTS: eo->ButtonOp (MopRemoveIsolatedVerts); break;
		case IDC_SELECT_OPEN: eo->ButtonOp (MopSelectOpenEdges); break;
		case IDC_CREATE_CURVE: eo->ButtonOp (MopCreateShapeFromEdges); break;
		}
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

static void SetSmoothButtonState (HWND hWnd,DWORD bits,DWORD invalid,DWORD unused=0) {
	for (int i=IDC_SMOOTH_GRP1; i<IDC_SMOOTH_GRP1+32; i++) {
		if ( (unused&(1<<(i-IDC_SMOOTH_GRP1))) ) {
			ShowWindow(GetDlgItem(hWnd,i),SW_HIDE);
			continue;
		}

		if ( (invalid&(1<<(i-IDC_SMOOTH_GRP1))) ) {
			SetWindowText(GetDlgItem(hWnd,i),NULL);
			SendMessage(GetDlgItem(hWnd,i),CC_COMMAND,CC_CMD_SET_STATE,FALSE);
		} else {
			TSTR buf;
			buf.printf(_T("%d"),i-IDC_SMOOTH_GRP1+1);
			SetWindowText(GetDlgItem(hWnd,i),buf);
			SendMessage(GetDlgItem(hWnd,i),CC_COMMAND,CC_CMD_SET_STATE,
				(bits&(1<<(i-IDC_SMOOTH_GRP1)))?TRUE:FALSE);
		}
		InvalidateRect(GetDlgItem(hWnd,i),NULL,TRUE);
	}
}

static INT_PTR CALLBACK SelectBySmoothDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static DWORD *param;
	int i;
	ICustButton *iBut;

	switch (msg) {
	case WM_INITDIALOG:
		param = (DWORD*)lParam;
		for (i=IDC_SMOOTH_GRP1; i<IDC_SMOOTH_GRP1+32; i++)
			SendMessage(GetDlgItem(hWnd,i),CC_COMMAND,CC_CMD_SET_TYPE,CBT_CHECK);
		SetSmoothButtonState(hWnd,param[0],0,param[2]);
		CheckDlgButton(hWnd,IDC_CLEARSELECTION,param[1]);
		CenterWindow(hWnd,GetParent(hWnd));
		break;

	case WM_COMMAND: 
		if (LOWORD(wParam)>=IDC_SMOOTH_GRP1 &&
			LOWORD(wParam)<=IDC_SMOOTH_GRP32) {
			iBut = GetICustButton(GetDlgItem(hWnd,LOWORD(wParam)));				
			int shift = LOWORD(wParam) - IDC_SMOOTH_GRP1;				
			if (iBut->IsChecked()) param[0] |= 1<<shift;
			else param[0] &= ~(1<<shift);		
			ReleaseICustButton(iBut);
			break;
		}

		switch (LOWORD(wParam)) {
		case IDOK:
			param[1] = IsDlgButtonChecked(hWnd,IDC_CLEARSELECTION);					
			EndDialog(hWnd,1);					
			break;					

		case IDCANCEL:
			EndDialog(hWnd,0);
			break;
		}
		break;			

	default: return FALSE;
	}
	return TRUE;
}

void EditTriObject::InvalidateSurfaceUI() {
	if (!hSurf) return;
	faceUIValid = FALSE;
	InvalidateRect (hSurf, NULL, FALSE);
}

class SurfaceSpinRestore : public RestoreObj {
public:
	EditTriObject *eto;
	Tab<float> oldFVals, newFVals;
	int idSpin, num;

	SurfaceSpinRestore () { eto = NULL; }
	SurfaceSpinRestore (EditTriObject *eo, int iSpin);
	void Restore (int isUndo);
	void Redo ();
	int Size () { return sizeof(eto) + 3*sizeof(int) + 2*sizeof(Tab<float>); }
	TSTR Description() { return _T("Surface Spin Restore"); }
};

SurfaceSpinRestore::SurfaceSpinRestore (EditTriObject *eo, int iSpin) {
	eto = eo;
	idSpin = iSpin;

	float *vp;
	switch (idSpin) {
	case IDC_EM_WEIGHTSPIN:
		num = eo->GetMesh().numVerts;
		vp = eo->GetMesh().vertexFloat (VDATA_WEIGHT);
		if (!vp) return;
		oldFVals.SetCount (num);
		memcpy (oldFVals.Addr(0), vp, num*sizeof(float));
		break;

	case IDC_MAT_IDSPIN:
	case IDC_EM_ALPHASPIN:
		num = 0;
		// Handle these changes in normal undo object.
		// (Just use SurfaceSpinRestore to update display.)
		break;
	}
}

void SurfaceSpinRestore::Restore (int isUndo) {
	if (!eto) return;
	float *vp;
	switch (idSpin) {
	case IDC_EM_ALPHASPIN:
	case IDC_MAT_IDSPIN:
		break;

	case IDC_EM_WEIGHTSPIN:
		vp = eto->GetMesh().vertexFloat (VDATA_WEIGHT);
		if (!vp) return;
		if (!newFVals.Count()) {
			newFVals.SetCount (num);
			memcpy (newFVals.Addr(0), vp, num*sizeof(float));
		}
		if (oldFVals.Count()) {
			memcpy (vp, oldFVals.Addr(0), num*sizeof(float));
		} else {
			for (int i=0; i<num; i++) vp[i] = 1.0f;
		}
		break;
	}

	if (eto->hSurf && GetDlgItem (eto->hSurf, idSpin))
		eto->UpdateSurfaceSpinner (eto->ip ? eto->ip->GetTime() : 0, eto->hSurf, idSpin);
	eto->NotifyDependents (FOREVER, PART_GEOM, REFMSG_CHANGE);
}

void SurfaceSpinRestore::Redo () {
	if (!eto) return;
	float *vp;
	switch (idSpin) {
	case IDC_EM_ALPHASPIN:
	case IDC_MAT_IDSPIN:
		break;

	case IDC_EM_WEIGHTSPIN:
		vp = eto->GetMesh().vertexFloat (VDATA_WEIGHT);
		if (!vp) return;
		memcpy (vp, newFVals.Addr(0), num * sizeof(float));
		break;
	}

	if (eto->hSurf && GetDlgItem (eto->hSurf, idSpin))
		eto->UpdateSurfaceSpinner (eto->ip ? eto->ip->GetTime() : 0, eto->hSurf, idSpin);
	eto->NotifyDependents (FOREVER, PART_GEOM, REFMSG_CHANGE);
}

static int autoEdgeTypeIDs[] = { IDC_AE_SETCLEAR, IDC_AE_SET, IDC_AE_CLEAR };

static INT_PTR CALLBACK SurfaceDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	EditTriObject *eo = (EditTriObject*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	ISpinnerControl *spin;
	ICustButton *but;
	IColorSwatch *iCol;
	COLORREF rgb;
	int i;
	bool isFace, isVert;
	static SurfaceSpinRestore *ssr = NULL;

	switch (msg) {
	case WM_INITDIALOG:
		eo = (EditTriObject*)lParam;
		eo->hSurf = hWnd;
		SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);

#ifndef USE_EMESH_SIMPLE_UI

		if (GetDlgItem (hWnd, IDC_SMOOTH_GRP1)) {	// Face stuff:
			// NOTE: the following requires that the smoothing group ID's be sequential!
			isFace = TRUE;
			for (i=IDC_SMOOTH_GRP1; i<IDC_SMOOTH_GRP1+32; i++)
				SendMessage(GetDlgItem(hWnd,i),CC_COMMAND,CC_CMD_SET_TYPE,CBT_CHECK);
			SetupIntSpinner (hWnd, IDC_MAT_IDSPIN, IDC_MAT_ID, 1, MAX_MATID, 1);
			SetupIntSpinner (hWnd, IDC_MAT_IDSPIN_SEL, IDC_MAT_ID_SEL, 1, MAX_MATID, 1);      
			CheckDlgButton(hWnd, IDC_CLEARSELECTION, 1);                                 
			SetupMtlSubNameCombo (hWnd, eo); 

			SetupFloatSpinner (hWnd, IDC_SMOOTH_THRESHSPIN, IDC_SMOOTH_THRESH,
				0.0f, 180.0f, autoSmoothThresh, 0.1f);

			but = GetICustButton(GetDlgItem(hWnd,IDC_NORMAL_FLIPMODE));
			but->SetType(CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(but);

			eo->faceUIValid = FALSE;

			iCol = GetIColorSwatch (GetDlgItem (hWnd, IDC_VERT_COLOR),
				eo->GetFaceColor(), GetString(IDS_RB_VERTEXCOLOR));
			ReleaseIColorSwatch(iCol);

			iCol = GetIColorSwatch (GetDlgItem (hWnd, IDC_VERT_ILLUM),
				eo->GetFaceColor(MAP_SHADING), GetString (IDS_VERTEX_ILLUM));
			ReleaseIColorSwatch (iCol);

			SetupFloatSpinner (hWnd, IDC_EM_ALPHASPIN, IDC_EM_ALPHA, 0.0f, 100.0f, 100.0f, .1f);
			eo->UpdateSurfaceSpinner (eo->ip->GetTime(), hWnd, IDC_EM_ALPHASPIN);
		} else {
			isFace = FALSE;
		}

		if (GetDlgItem (hWnd, IDC_VERT_SELRSPIN)) {	// Vertex stuff:
			isVert = TRUE;
			spin = SetupFloatSpinner (hWnd, IDC_EM_WEIGHTSPIN, IDC_EM_WEIGHT, 0.0f, 9999999.0f, 1.0f, .1f);
			spin->SetAutoScale(TRUE);
			eo->UpdateSurfaceSpinner (eo->ip->GetTime(), hWnd, IDC_EM_WEIGHTSPIN);

			iCol = GetIColorSwatch(GetDlgItem(hWnd,IDC_VERT_COLOR),
				eo->GetVertColor(), GetString(IDS_RB_VERTEXCOLOR));
			ReleaseIColorSwatch(iCol);

			iCol = GetIColorSwatch(GetDlgItem(hWnd,IDC_VERT_ILLUM),
				eo->GetVertColor(MAP_SHADING), GetString(IDS_VERTEX_ILLUM));
			ReleaseIColorSwatch(iCol);

			spin = SetupFloatSpinner (hWnd, IDC_EM_ALPHASPIN, IDC_EM_ALPHA, 0.0f, 100.0f, 100.0f, .1f);
			eo->UpdateSurfaceSpinner (eo->ip->GetTime(), hWnd, IDC_EM_ALPHASPIN);

			SetupIntSpinner (hWnd, IDC_VERT_SELRSPIN, IDC_VERT_SELR, 0, 255, selDeltaR);
			SetupIntSpinner (hWnd, IDC_VERT_SELGSPIN, IDC_VERT_SELG, 0, 255, selDeltaG);
			SetupIntSpinner (hWnd, IDC_VERT_SELBSPIN, IDC_VERT_SELB, 0, 255, selDeltaB);
			rgb = RGB(int(selByColor.x*255.0),int(selByColor.y*255.0),int(selByColor.z*255.0));
			iCol = GetIColorSwatch (GetDlgItem(hWnd,IDC_VERT_SELCOLOR), rgb, GetString(IDS_RB_SELBYCOLOR));
			ReleaseIColorSwatch(iCol);

			CheckDlgButton (hWnd, IDC_EM_SEL_BY_COLOR, (selByChannel==0));
			CheckDlgButton (hWnd, IDC_EM_SEL_BY_ILLUM, (selByChannel==MAP_SHADING));
		} else {
			isVert = FALSE;
		}

		if (GetDlgItem (hWnd, IDC_ANGLETHRESH)) {	// Edge stuff:
			SetupFloatSpinner (hWnd, IDC_ANGLETHRESHSPIN, IDC_ANGLETHRESH,
				0.f, 180.f, autoEdgeThresh, 0.1f);
			for (i=0; i<3; i++) CheckDlgButton (hWnd, autoEdgeTypeIDs[i], (autoEdgeType == i));
		}
#else
		i = 0;
		isVert = FALSE;
		isFace = TRUE;
		SetupFloatSpinner (hWnd, IDC_SMOOTH_THRESHSPIN, IDC_SMOOTH_THRESH,
			0.0f, 180.0f, autoSmoothThresh, 0.1f);

		but = GetICustButton(GetDlgItem(hWnd,IDC_NORMAL_FLIPMODE));
		but->SetType(CBT_CHECK);
		but->SetHighlightColor(GREEN_WASH);
		ReleaseICustButton(but);

		eo->faceUIValid = FALSE;
#endif // USE_EMESH_SIMPLE_UI

		break;

	case WM_PAINT:
		if (eo->faceUIValid) return FALSE;
		if (GetDlgItem (hWnd, IDC_SMOOTH_GRP1)) {
			DWORD invalid, bits;
			bits = eo->GetSelSmoothBits(invalid);
			invalid -= bits;
			SetSmoothButtonState(hWnd,bits,invalid);
		}
		if (GetDlgItem (hWnd, IDC_MAT_IDSPIN)) {
			eo->UpdateSurfaceSpinner (eo->ip->GetTime(), hWnd, IDC_MAT_IDSPIN);
		}
		if (GetDlgItem (hWnd, IDC_MAT_IDSPIN_SEL)) {                                       
			eo->UpdateSurfaceSpinner (eo->ip->GetTime(), hWnd, IDC_MAT_IDSPIN_SEL);
		}  
		if (GetDlgItem (hWnd, IDC_MTLID_NAMES_COMBO)) { 
			ValidateUINameCombo(hWnd, eo);
		}    		
		if (GetDlgItem (hWnd, IDC_VERT_COLOR)) {
			iCol = GetIColorSwatch (GetDlgItem(hWnd,IDC_VERT_COLOR),
				eo->selLevel==SL_VERTEX ? eo->GetVertColor() : eo->GetFaceColor(),
				GetString(IDS_RB_VERTEXCOLOR));
			ReleaseIColorSwatch(iCol);
			iCol = GetIColorSwatch (GetDlgItem (hWnd, IDC_VERT_ILLUM),
				eo->selLevel==SL_VERTEX ? eo->GetVertColor(MAP_SHADING) : eo->GetFaceColor(MAP_SHADING),
				GetString (IDS_VERTEX_ILLUM));
			ReleaseIColorSwatch (iCol);
			eo->UpdateSurfaceSpinner (eo->ip->GetTime(), hWnd, IDC_EM_ALPHASPIN);
		}
		if (GetDlgItem (hWnd, IDC_VERT_SELCOLOR)) {
			rgb = RGB(int(selByColor.x*255.0),int(selByColor.y*255.0),int(selByColor.z*255.0));
			iCol = GetIColorSwatch (GetDlgItem (hWnd, IDC_VERT_SELCOLOR), rgb,
				(selByChannel==0) ? GetString(IDS_RB_SELBYCOLOR) : GetString (IDS_SELBYILLUM));
			ReleaseIColorSwatch (iCol);
		}
		if (GetDlgItem (hWnd, IDC_EM_WEIGHT_LABEL)) {
			eo->UpdateSurfaceSpinner (eo->ip->GetTime (), hWnd, IDC_EM_WEIGHTSPIN);
		}
		eo->faceUIValid = TRUE;
		return FALSE;

	case CC_COLOR_BUTTONDOWN:
		theHold.Begin();
		break;

	case CC_COLOR_BUTTONUP:
		int stringID;
		switch (LOWORD(wParam)) {
		case IDC_VERT_COLOR:
			stringID = IDS_RB_SETVERTCOLOR;
			break;
		case IDC_VERT_ILLUM:
			stringID = IDS_VERTEX_ILLUM;
			break;
		}
		if (HIWORD(wParam)) theHold.Accept (GetString(stringID));
		else theHold.Cancel ();
		break;

	case CC_COLOR_CHANGE:
		iCol = (IColorSwatch*)lParam;
		switch (LOWORD(wParam)) {
		case IDC_VERT_COLOR:
			if (eo->selLevel == SL_VERTEX) eo->SetVertColor (Color(iCol->GetColor()));
			else eo->SetFaceColor (Color(iCol->GetColor()));
			break;
		case IDC_VERT_ILLUM:
			if (eo->selLevel == SL_VERTEX) eo->SetVertColor (Color(iCol->GetColor()), MAP_SHADING);
			else eo->SetFaceColor (Color(iCol->GetColor()), MAP_SHADING);
			break;
		case IDC_VERT_SELCOLOR:
			COLORREF rgb = iCol->GetColor();
			selByColor.x = float(GetRValue(rgb))/255.0f;
			selByColor.y = float(GetGValue(rgb))/255.0f;
			selByColor.z = float(GetBValue(rgb))/255.0f;
			break;
		}
		break;

	case CC_SPINNER_BUTTONDOWN:
		switch (LOWORD(wParam)) {
		case IDC_MAT_IDSPIN:
			theHold.Begin ();
			ssr = new SurfaceSpinRestore (eo, IDC_MAT_IDSPIN);
			break;
		case IDC_EM_WEIGHTSPIN:
			ssr = new SurfaceSpinRestore (eo, IDC_EM_WEIGHTSPIN);
			break;
		case IDC_EM_ALPHASPIN:
			theHold.Begin ();
			ssr = new SurfaceSpinRestore (eo, IDC_EM_ALPHASPIN);
			break;
		}
		break;

	case WM_CUSTEDIT_ENTER:
	case CC_SPINNER_BUTTONUP:
		switch (LOWORD(wParam)) {
		case IDC_MAT_ID:
		case IDC_MAT_IDSPIN:
			eo->ip->RedrawViews(eo->ip->GetTime(),REDRAW_END);
			if (HIWORD(wParam) || msg==WM_CUSTEDIT_ENTER) {
				if (ssr) theHold.Put (ssr);
				theHold.Accept(GetString(IDS_RB_ASSIGNMATID));
			} else {
				if (ssr) ssr->Restore (false);
				if (ssr) delete ssr;
				theHold.Cancel();
			}
			ssr = NULL;
			break;

		case IDC_EM_WEIGHT:
		case IDC_EM_WEIGHTSPIN:
			eo->ip->RedrawViews (eo->ip->GetTime(),REDRAW_END);
			if (HIWORD(wParam) || msg==WM_CUSTEDIT_ENTER) {
				theHold.Begin ();
				if (ssr) theHold.Put (ssr);
				theHold.Accept (GetString(IDS_CHANGEWEIGHT));
			} else {
				ssr->Restore (FALSE);
				delete ssr;
			}
			ssr = NULL;
			break;

		case IDC_EM_ALPHA:
		case IDC_EM_ALPHASPIN:
			eo->ip->RedrawViews (eo->ip->GetTime(),REDRAW_END);
			if (HIWORD(wParam) || msg==WM_CUSTEDIT_ENTER) {
				if (ssr) theHold.Put (ssr);
				theHold.Accept (GetString(IDS_CHANGE_ALPHA));
			} else {
				theHold.Cancel ();
				delete ssr;
			}
			ssr = NULL;
			break;
		}
		break;

	case CC_SPINNER_CHANGE:
		spin = (ISpinnerControl*)lParam;
		switch (LOWORD(wParam)) {
		case IDC_MAT_IDSPIN:
			if (!ssr)
			{
				theHold.Begin ();
				ssr = new SurfaceSpinRestore (eo, IDC_MAT_IDSPIN);
			}
			eo->SetMatIndex(spin->GetIVal()-1);
			break;

		case IDC_VERT_SELRSPIN: selDeltaR = spin->GetIVal(); break;
		case IDC_VERT_SELGSPIN: selDeltaG = spin->GetIVal(); break;
		case IDC_VERT_SELBSPIN: selDeltaB = spin->GetIVal(); break;
		case IDC_ANGLETHRESHSPIN: autoEdgeThresh = spin->GetFVal (); break;
		case IDC_SMOOTH_THRESHSPIN: autoSmoothThresh = spin->GetFVal (); break;

		case IDC_EM_WEIGHTSPIN:
			if (!ssr) {
				theHold.Begin ();
				ssr = new SurfaceSpinRestore (eo, IDC_EM_WEIGHTSPIN);
			}
			eo->SetWeight (eo->ip->GetTime(), spin->GetFVal());
			break;

		case IDC_EM_ALPHASPIN:
			if (!ssr) {
				theHold.Begin ();
				ssr = new SurfaceSpinRestore (eo, IDC_EM_ALPHASPIN);
			}
			eo->SetAlpha (spin->GetFVal()/100.0f);
			break;
		}
		break;

	case WM_COMMAND:
		if (LOWORD(wParam)>=IDC_SMOOTH_GRP1 &&
			LOWORD(wParam)<=IDC_SMOOTH_GRP32) {
			ICustButton *iBut = GetICustButton(GetDlgItem(hWnd,LOWORD(wParam)));
			int bit = iBut->IsChecked() ? 1 : 0;
			int shift = LOWORD(wParam) - IDC_SMOOTH_GRP1;
			eo->SetSelSmoothBits(bit<<shift,1<<shift);
			ReleaseICustButton(iBut);
			break;
		}
		switch (LOWORD(wParam)) {
		case IDC_EDGE_VIS: eo->ButtonOp (MopVisibleEdge); break;
		case IDC_EDGE_INVIS: eo->ButtonOp (MopInvisibleEdge); break;
		case IDC_EDGE_AUTO: eo->ButtonOp (MopAutoEdge); break;

		case IDC_AE_SETCLEAR:
			autoEdgeType = 0;
			break;

		case IDC_AE_SET:
			autoEdgeType = 1;
			break;

		case IDC_AE_CLEAR:
			autoEdgeType = 2;
			break;

		case IDC_NORMAL_FLIP: eo->ButtonOp (MopFlipNormal); break;
		case IDC_NORMAL_UNIFY: eo->ButtonOp (MopUnifyNormal); break;
		case IDC_NORMAL_FLIPMODE: eo->ToggleCommandMode (McmFlipNormalMode); break;

		case IDC_SELECT_BYID:
			ISpinnerControl *spin;
			int mtlID, clearSel;
			spin = GetISpinner(GetDlgItem(hWnd,IDC_MAT_IDSPIN_SEL));
			mtlID = (spin->IsIndeterminate()) ? -1 : spin->GetIVal()-1; 
			clearSel = IsDlgButtonChecked(hWnd,IDC_CLEARSELECTION);
			if (eo->ip) eo->SelectByMat(mtlID, clearSel); 
			ReleaseISpinner(spin);                                                                   
			break;

		case IDC_MTLID_NAMES_COMBO:
			switch(HIWORD(wParam)){
			case CBN_SELENDOK:
				int index, val;
				index = SendMessage(GetDlgItem(hWnd,IDC_MTLID_NAMES_COMBO), CB_GETCURSEL, 0, 0);
				val = SendMessage(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), CB_GETITEMDATA, (WPARAM)index, 0);
				if (index != CB_ERR){
					int clearSel = IsDlgButtonChecked(hWnd,IDC_CLEARSELECTION);
					theHold.Begin();
					if (eo->ip) eo->SelectByMat(val, clearSel);
					theHold.Accept(GetString(IDS_RB_SELECTBYMATID));
					eo->ip->RedrawViews(eo->ip->GetTime(),REDRAW_END);
				}
				break;                                                    
			}
			break;

		case IDC_SELECTBYSMOOTH:
			sbsParams[2] = ~eo->GetUsedSmoothBits();
			if (DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_EM_SELECTBYSMOOTH),
						eo->ip->GetMAXHWnd(), SelectBySmoothDlgProc, (LPARAM)sbsParams)) {
				if (eo->ip) eo->SelectBySmoothGroup(sbsParams[0],(BOOL)sbsParams[1]);
			}
			break;

		case IDC_SMOOTH_CLEAR:
			eo->SetSelSmoothBits(0,0xffffffff);
			break;

		case IDC_SMOOTH_AUTO: eo->ButtonOp (MopAutoSmooth); break;

		case IDC_EM_SEL_BY_COLOR:
			selByChannel = 0;
			eo->faceUIValid = false;
			InvalidateRect (hWnd, NULL, FALSE);
			break;

		case IDC_EM_SEL_BY_ILLUM:
			selByChannel = MAP_SHADING;
			eo->faceUIValid = false;
			InvalidateRect (hWnd, NULL, FALSE);
			break;

		case IDC_VERT_SELBYCOLOR:
			BOOL add, sub;
			add = GetKeyState(VK_CONTROL)<0;
			sub = GetKeyState(VK_MENU)<0;
			eo->SelectVertByColor (selByColor,selDeltaR,selDeltaG,selDeltaB,add,sub, selByChannel);
			break;
		}
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

//
// Customizable Actions for Edit and Editable Mesh, the MeshDelta users:
//

static ActionDescription spActions[] = {
	ID_EM_SELTYPE,
    IDS_EM_SWITCH_SUBLEVELS,
    IDS_EM_SWITCH_SUBLEVELS,
    IDS_EMESH,

	ID_EM_SELTYPE_BACK,
    IDS_EM_PREV_SUBLEVEL,
    IDS_EM_PREV_SUBLEVEL,
    IDS_EMESH,

	ID_EM_SELTYPE_VERTEX,
    IDS_EM_GOTO_VERTEX,
    IDS_EM_GOTO_VERTEX,
    IDS_EMESH,

	ID_EM_SELTYPE_EDGE,
    IDS_EM_GOTO_EDGE,
    IDS_EM_GOTO_EDGE,
    IDS_EMESH,

	ID_EM_SELTYPE_FACE,
    IDS_EM_GOTO_FACE,
    IDS_EM_GOTO_FACE,
    IDS_EMESH,

	ID_EM_SELTYPE_POLYGON,
    IDS_EM_GOTO_POLYGON,
    IDS_EM_GOTO_POLYGON,
    IDS_EMESH,

	ID_EM_SELTYPE_ELEMENT,
    IDS_EM_GOTO_ELEMENT,
    IDS_EM_GOTO_ELEMENT,
    IDS_EMESH,

	ID_EM_SELTYPE_OBJ,
    IDS_EM_GOTO_OBJECT,
    IDS_EM_GOTO_OBJECT,
    IDS_EMESH,

	ID_EM_AUTOSMOOTH,
    IDS_RB_AUTOSMOOTH,
    IDS_RB_AUTOSMOOTH,
    IDS_EMESH,

#ifndef RENDER_VER
	ID_EM_ATTACH,
    IDS_EM_ATTACH_MODE,
    IDS_EM_ATTACH_MODE,
    IDS_EMESH,

	ID_EM_BREAK,
    IDS_RB_BREAKVERTS,
    IDS_RB_BREAKVERTS,
    IDS_EMESH,
#endif //RENDER_VER

	ID_EM_IGBACK,
    IDS_EM_IGNORE_BACK,
    IDS_EM_IGNORE_BACK,
    IDS_EMESH,

#ifndef RENDER_VER
	ID_EM_BEVEL,
    IDS_EM_BEVEL_MODE,
    IDS_EM_BEVEL_MODE,
    IDS_EMESH,

	ID_EM_CREATE,
    IDS_EM_CREATE_MODE,
    IDS_EM_CREATE_MODE,
    IDS_EMESH,

	ID_EM_CUT,
    IDS_EM_CUT_MODE,
    IDS_EM_CUT_MODE,
    IDS_EMESH,

	ID_EM_DIVIDE,
    IDS_EM_DIVIDE_MODE,
    IDS_EM_DIVIDE_MODE,
    IDS_EMESH,

	ID_EM_EXTRUDE,
    IDS_EM_EXTRUDE_MODE,
    IDS_EM_EXTRUDE_MODE,
    IDS_EMESH,
#endif // RENDER_VER

	ID_EM_FLIPNORM,
    IDS_RB_FLIPNORMALS,
    IDS_RB_FLIPNORMALS,
    IDS_EMESH,

#ifndef RENDER_VER
	ID_EM_SS_BACKFACE,
    IDS_EM_SS_BACKFACE,
    IDS_EM_SS_BACKFACE,
    IDS_EMESH,
#endif // RENDER_VER

	ID_EM_UNIFY_NORMALS,
    IDS_EM_UNIFY,
    IDS_EM_UNIFY,
    IDS_EMESH,

	ID_EM_HIDE,
    IDS_EM_HIDE_SELECTED,
    IDS_EM_HIDE_SELECTED,
    IDS_EMESH,

#ifndef RENDER_VER
	ID_EM_EDGE_INVIS,
    IDS_RB_EDGEINVISIBLE,
    IDS_RB_EDGEINVISIBLE,
    IDS_EMESH,
#endif // RENDER_VER

	ID_EM_IGNORE_INVIS,
    IDS_EM_IGNORE_VISEDGE,
    IDS_EM_IGNORE_VISEDGE,
    IDS_EMESH,

#ifndef RENDER_VER
	ID_EM_COLLAPSE,
    IDS_RB_COLLAPSE,
    IDS_RB_COLLAPSE,
    IDS_EMESH,
#endif // RENDER_VER

	ID_EM_SHOWNORMAL,
    IDS_EM_SHOW_NORMALS,
    IDS_EM_SHOW_NORMALS,
    IDS_EMESH,

#ifndef RENDER_VER
	ID_EM_SELOPEN,
    IDS_RB_SELOPENEDGES,
    IDS_RB_SELOPENEDGES,
    IDS_EMESH,

	ID_EM_REMOVE_ISO,
    IDS_RB_DELETEISO,
    IDS_RB_DELETEISO,
    IDS_EMESH,

	ID_EM_SLICEPLANE,
    IDS_EM_SLICEPLANE,
    IDS_EM_SLICEPLANE,
    IDS_EMESH,

	ID_EM_SOFTSEL,
    IDS_AFFECT_REGION,
    IDS_AFFECT_REGION,
    IDS_EMESH,

	ID_EM_SLICE,
    IDS_SCA_SLICE,
    IDS_SCA_SLICE,
    IDS_EMESH,
#endif // RENDER_VER

	ID_EM_DETACH,
    IDS_DETACH,
    IDS_DETACH,
    IDS_EMESH,

#ifndef RENDER_VER
	ID_EM_TURNEDGE,
    IDS_RB_EDGETURN,
    IDS_RB_EDGETURN,
    IDS_EMESH,
#endif // RENDER_VER

	ID_EM_UNHIDE,
    IDS_RB_UNHIDEALLFACES,
    IDS_RB_UNHIDEALLFACES,
    IDS_EMESH,

#ifndef RENDER_VER
	ID_EM_EDGE_VIS,
    IDS_RB_EDGEVISIBLE,
    IDS_RB_EDGEVISIBLE,
    IDS_EMESH,
#endif // RENDER_VER

	ID_EM_SELBYVERT,
    IDS_EM_SELBYVERT,
    IDS_EM_SELBYVERT,
    IDS_EMESH,

#ifndef RENDER_VER
	ID_EM_AUTOEDGE,
    IDS_RB_AUTOEDGE,
    IDS_RB_AUTOEDGE,
    IDS_EMESH,
#endif // RENDER_VER

	ID_EM_WELD,
    IDS_EM_WELD_SEL,
    IDS_EM_WELD_SEL,
    IDS_EMESH,

#ifndef RENDER_VER
	ID_EM_EXPLODE,
    IDS_RB_EXPLODE,
    IDS_RB_EXPLODE,
    IDS_EMESH,

	ID_EM_CHAMFER,
    IDS_EM_CHAMFER_MODE,
    IDS_EM_CHAMFER_MODE,
    IDS_EMESH,

	ID_EM_WELD_TARGET,
    IDS_EM_WELD_TARGET_MODE,
    IDS_EM_WELD_TARGET_MODE,
    IDS_EMESH,

	ID_EM_ATTACH_LIST,
    IDS_EM_ATTACH_LIST,
    IDS_EM_ATTACH_LIST,
    IDS_EMESH,
	
	ID_EM_VIEW_ALIGN,
		IDS_EM_VIEW_ALIGN,
		IDS_EM_VIEW_ALIGN,
		IDS_EMESH,

	ID_EM_GRID_ALIGN,
		IDS_EM_GRID_ALIGN,
		IDS_EM_GRID_ALIGN,
		IDS_EMESH,

	ID_EM_SPLIT,
		IDS_EM_SPLIT,
		IDS_EM_SPLIT,
		IDS_EMESH,

	ID_EM_REFINE_CUTENDS,
		IDS_EM_REFINE_CUTENDS,
		IDS_EM_REFINE_CUTENDS,
		IDS_EMESH,

	ID_EM_COPY_NAMEDSEL,
		IDS_EM_COPY_NAMEDSEL,
		IDS_EM_COPY_NAMEDSEL,
		IDS_EMESH,

	ID_EM_PASTE_NAMEDSEL,
		IDS_EM_PASTE_NAMEDSEL,
		IDS_EM_PASTE_NAMEDSEL,
		IDS_EMESH,

	ID_EM_MAKE_PLANAR,
		IDS_EM_MAKE_PLANAR,
		IDS_EM_MAKE_PLANAR,
		IDS_EMESH,

	ID_EM_VERT_COLOR,
		IDS_EM_VERT_COLOR,
		IDS_EM_VERT_COLOR,
		IDS_EMESH,

	ID_EM_VERT_ILLUM,
		IDS_EM_VERT_ILLUM,
		IDS_EM_VERT_ILLUM,
		IDS_EMESH,
#endif // RENDER_VER

	ID_EM_FLIP_NORMAL_MODE,
		IDS_EM_FLIP_NORMAL_MODE,
		IDS_EM_FLIP_NORMAL_MODE,
		IDS_EMESH,
};

#ifndef RENDER_VER
#define NUM_ACTIONS 51
#else // RENDER_VER
#define NUM_ACTIONS 20
#endif// RENDER_VER

static int actionIDsInSDK[] = {
	MDUID_EM_SELTYPE,
	MDUID_EM_SELTYPE_BACK,
	MDUID_EM_SELTYPE_VERTEX,
	MDUID_EM_SELTYPE_EDGE,
	MDUID_EM_SELTYPE_FACE,
	MDUID_EM_SELTYPE_POLYGON,
	MDUID_EM_SELTYPE_ELEMENT,
	MDUID_EM_SELTYPE_OBJ,
	MDUID_EM_AUTOSMOOTH,
#ifndef RENDER_VER
	MDUID_EM_ATTACH,
	MDUID_EM_BREAK,
#endif // RENDER_VER
	MDUID_EM_IGBACK,
#ifndef RENDER_VER
	MDUID_EM_BEVEL,
	MDUID_EM_CREATE,
	MDUID_EM_CUT,
	MDUID_EM_DIVIDE,
	MDUID_EM_EXTRUDE,
#endif // RENDER_VER
	MDUID_EM_FLIPNORM,
#ifndef RENDER_VER
	MDUID_EM_SS_BACKFACE,
#endif // RENDER_VER
	MDUID_EM_UNIFY_NORMALS,
	MDUID_EM_HIDE,
#ifndef RENDER_VER
	MDUID_EM_EDGE_INVIS,
#endif // RENDER_VER
	MDUID_EM_IGNORE_INVIS,
#ifndef RENDER_VER
	MDUID_EM_COLLAPSE,
#endif // RENDER_VER
	MDUID_EM_SHOWNORMAL,
#ifndef RENDER_VER
	MDUID_EM_SELOPEN,
	MDUID_EM_REMOVE_ISO,
	MDUID_EM_SLICEPLANE,
	MDUID_EM_SOFTSEL,
	MDUID_EM_SLICE,
#endif // RENDER_VER
	MDUID_EM_DETACH,
#ifndef RENDER_VER
	MDUID_EM_TURNEDGE,
#endif // RENDER_VER
	MDUID_EM_UNHIDE,
#ifndef RENDER_VER
	MDUID_EM_EDGE_VIS,
#endif // RENDER_VER
	MDUID_EM_SELBYVERT,
#ifndef RENDER_VER
	MDUID_EM_AUTOEDGE,
#endif // RENDER_VER
	MDUID_EM_WELD,
#ifndef RENDER_VER
	MDUID_EM_EXPLODE,
	MDUID_EM_CHAMFER,
	MDUID_EM_WELD_TARGET,
	MDUID_EM_ATTACH_LIST,							 
	MDUID_EM_VIEW_ALIGN,								 
	MDUID_EM_GRID_ALIGN,							 
	MDUID_EM_SPLIT,										 
	MDUID_EM_REFINE_CUTENDS,
	MDUID_EM_COPY_NAMEDSEL,					 
	MDUID_EM_PASTE_NAMEDSEL,						 
	MDUID_EM_MAKE_PLANAR,					 
	MDUID_EM_VERT_COLOR,							 
	MDUID_EM_VERT_ILLUM,							 
#endif // RENDER_VER
	MDUID_EM_FLIP_NORMAL_MODE,
};

// Following should only be called once.
ActionTable* GetEMeshActions()
{
#ifdef _DEBUG
	int i;
	for (i=0; i<NUM_ACTIONS; i++) {
		if (actionIDsInSDK[i] != spActions[i].mCmdID) break;
	}
	if (i<NUM_ACTIONS) {
		DebugPrint ("EMesh: Please match SDK action ID's with the resource ID's in Editable Mesh.\n");
		DbgAssert(0);
	}
#endif
	TSTR name = GetString(IDS_EMESH);
	HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE (IDR_ACCELERATORS));
	ActionTable* pTab;
	pTab = new ActionTable (kEMeshActions, kEMeshContext, name, hAccel, NUM_ACTIONS, spActions, hInstance);
    GetCOREInterface()->GetActionManager()->RegisterActionContext(kEMeshContext, name.data());

	return pTab;
}

// Following should be _identical_ to that in edmui.cpp; the whole class should be in some core location.
// -sca/1999.03.12
BOOL EMeshActionCB::ExecuteAction (int id) {
	if (!em) return FALSE;
	if (!em->Editing()) return FALSE;
	int val;

	switch (id) {
	case MDUID_EM_SELTYPE:
		int type;
		switch (em->GetEMeshSelLevel()) {
			case EM_SL_FACE:  type = EM_SL_POLYGON;    break;
			case EM_SL_POLYGON:   type = EM_SL_ELEMENT; break;
			case EM_SL_ELEMENT: type = EM_SL_OBJECT;  break;
			case EM_SL_OBJECT: type = EM_SL_VERTEX; break;
			case EM_SL_VERTEX: type = EM_SL_EDGE; break;
			case EM_SL_EDGE: type = EM_SL_FACE; break;
		}
		em->SetEMeshSelLevel (type);
		break;

	case MDUID_EM_SELTYPE_BACK:
		switch (em->GetEMeshSelLevel()) {
			case EM_SL_FACE:  type = EM_SL_EDGE;    break;
			case EM_SL_POLYGON:   type = EM_SL_FACE; break;
			case EM_SL_ELEMENT: type = EM_SL_POLYGON;  break;
			case EM_SL_VERTEX: type = EM_SL_OBJECT; break;
			case EM_SL_OBJECT: type = EM_SL_ELEMENT; break;
			case EM_SL_EDGE: type = EM_SL_VERTEX; break;
		}
		em->SetEMeshSelLevel (type);
		break;

	case MDUID_EM_SELTYPE_VERTEX:
		em->SetEMeshSelLevel (EM_SL_VERTEX);
		break;

	case MDUID_EM_SELTYPE_EDGE:
		em->SetEMeshSelLevel (EM_SL_EDGE);
		break;

	case MDUID_EM_SELTYPE_FACE:
		em->SetEMeshSelLevel (EM_SL_FACE);
		break;

	case MDUID_EM_SELTYPE_POLYGON:
		em->SetEMeshSelLevel (EM_SL_POLYGON);
		break;

	case MDUID_EM_SELTYPE_ELEMENT:
		em->SetEMeshSelLevel (EM_SL_ELEMENT);
		break;

	case MDUID_EM_SELTYPE_OBJ:
		em->SetEMeshSelLevel (EM_SL_OBJECT);
		break;

	case MDUID_EM_AUTOSMOOTH:
		em->ButtonOp (MopAutoSmooth);
		break;

	case MDUID_EM_ATTACH:
		em->ToggleCommandMode (McmAttach);
		break;

	case MDUID_EM_BREAK:
		em->ButtonOp (MopBreak);
		break;

	case MDUID_EM_IGBACK:
		em->GetUIParam (MuiIgBack, val);
		em->SetUIParam (MuiIgBack, !val);
		break;

	case MDUID_EM_BEVEL:
		em->ToggleCommandMode (McmBevel);
		break;

	case MDUID_EM_CHAMFER:
		em->ToggleCommandMode (McmChamfer);
		break;

	case MDUID_EM_CREATE:
		em->ToggleCommandMode (McmCreate);
		break;

	case MDUID_EM_CUT:
		em->ToggleCommandMode (McmCut);
		break;

	case MDUID_EM_DIVIDE:
		em->ToggleCommandMode (McmDivide);
		break;

	case MDUID_EM_EXTRUDE:
		em->ToggleCommandMode (McmExtrude);
		break;

	case MDUID_EM_FLIPNORM:
		em->ButtonOp (MopFlipNormal);
		break;

	case MDUID_EM_SS_BACKFACE:
		em->GetUIParam (MuiSSBack, val);
		em->SetUIParam (MuiSSBack, !val);
		break;

	case MDUID_EM_UNIFY_NORMALS:
		em->ButtonOp (MopUnifyNormal);
		break;

	case MDUID_EM_HIDE:
		em->ButtonOp (MopHide);
		break;

	case MDUID_EM_EDGE_INVIS:
		em->ButtonOp (MopInvisibleEdge);
		break;

	case MDUID_EM_IGNORE_INVIS:
		em->GetUIParam (MuiIgnoreVis, val);
		em->SetUIParam (MuiIgnoreVis, !val);
		break;

	case MDUID_EM_COLLAPSE:
		em->ButtonOp (MopCollapse);
		break;

	case MDUID_EM_SHOWNORMAL:
		em->ButtonOp (MopShowNormal);
		break;

	case MDUID_EM_SELOPEN:
		em->ButtonOp (MopSelectOpenEdges);
		break;

	case MDUID_EM_REMOVE_ISO:
		em->ButtonOp (MopRemoveIsolatedVerts);
		break;

	case MDUID_EM_SLICEPLANE:
		em->ToggleCommandMode (McmSlicePlane);
		break;

	case MDUID_EM_SOFTSEL:
		em->GetUIParam (MuiSoftSel, val);
		em->SetUIParam (MuiSoftSel, !val);
		break;

	case MDUID_EM_SLICE:
		em->ButtonOp (MopSlice);
		break;

	case MDUID_EM_DETACH:
		em->ButtonOp (MopDetach);
		break;

	case MDUID_EM_TURNEDGE:
		em->ToggleCommandMode (McmTurnEdge);
		break;

	case MDUID_EM_UNHIDE:
		em->ButtonOp (MopUnhideAll);
		break;

	case MDUID_EM_EDGE_VIS:
		em->ButtonOp (MopVisibleEdge);
		break;

	case MDUID_EM_SELBYVERT:
		em->GetUIParam (MuiSelByVert, val);
		em->SetUIParam (MuiSelByVert, !val);
		break;

	case MDUID_EM_AUTOEDGE:
		em->ButtonOp (MopAutoEdge);
		break;

	case MDUID_EM_WELD:
		em->ButtonOp (MopWeld);
		break;

	case MDUID_EM_WELD_TARGET:
		em->ToggleCommandMode (McmWeldTarget);
		break;

	case MDUID_EM_EXPLODE:
		em->ButtonOp (MopExplode);
		break;

	case MDUID_EM_FLIP_NORMAL_MODE:
		em->ToggleCommandMode (McmFlipNormalMode);
		break;

	case MDUID_EM_ATTACH_LIST:
		em->ButtonOp (MopAttachList);
		break;

	case MDUID_EM_VIEW_ALIGN:
		em->ButtonOp (MopViewAlign);
		break;

	case MDUID_EM_GRID_ALIGN:
		em->ButtonOp (MopGridAlign);
		break;

	case MDUID_EM_SPLIT:
		em->GetUIParam (MuiSliceSplit, val);
		em->SetUIParam (MuiSliceSplit, !val);
		break;
			
	case MDUID_EM_REFINE_CUTENDS:
		em->GetUIParam (MuiCutRefine, val);
		em->SetUIParam (MuiCutRefine, !val);
		break;

	case MDUID_EM_COPY_NAMEDSEL:
		em->ButtonOp (MopCopyNS);
		break;

	case MDUID_EM_PASTE_NAMEDSEL:
		em->ButtonOp (MopPasteNS);
		break;

	case MDUID_EM_MAKE_PLANAR:
		em->ButtonOp (MopMakePlanar);
		break;

	case MDUID_EM_VERT_COLOR:
		em->ButtonOp (MopEditVertColor);
		break;
	
	case MDUID_EM_VERT_ILLUM:
		em->ButtonOp (MopEditVertIllum);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

//---------------------------------------------------------
//  MeshDeltaUser UI-related methods

void EditTriObject::GetUIParam (meshUIParam uiCode, float & ret) {
	if (!ip) return;
	if (!Editing()) return;

	switch (uiCode) {
	case MuiPolyThresh:
		ret = planarFaceThresh;
		break;
	case MuiFalloff:
		ret = falloff;
		break;
	case MuiPinch:
		ret = pinch;
		break;
	case MuiBubble:
		ret = bubble;
		break;
	case MuiWeldDist:
		ret = weldThresh;
		break;
	case MuiNormalSize:
		ret = normScale;
		break;
	}
}

void EditTriObject::SetUIParam (meshUIParam uiCode, float val) {
	if (!ip) return;
	if (!Editing()) return;
	ISpinnerControl *spin;

	switch (uiCode) {
	case MuiPolyThresh:
		planarFaceThresh = val;
		if (hSel) {
			spin = GetISpinner (GetDlgItem (hSel, IDC_PLANARSPINNER));
			spin->SetValue (val, FALSE);
			ReleaseISpinner (spin);
		}
		break;
	case MuiFalloff:
		falloff = val;
		InvalidateAffectRegion ();
		if (hAR) {
			spin = GetISpinner (GetDlgItem (hAR, IDC_FALLOFFSPIN));
			spin->SetValue (val, FALSE);
			ReleaseISpinner (spin);
		}
		break;
	case MuiPinch:
		pinch = val;
		InvalidateAffectRegion ();
		if (hAR) {
			spin = GetISpinner (GetDlgItem (hAR, IDC_PINCHSPIN));
			spin->SetValue (val, FALSE);
			ReleaseISpinner (spin);
		}
		break;
	case MuiBubble:
		bubble = val;
		InvalidateAffectRegion ();
		if (hAR) {
			spin = GetISpinner (GetDlgItem (hAR, IDC_BUBBLESPIN));
			spin->SetValue (val, FALSE);
			ReleaseISpinner (spin);
		}
		break;
	case MuiWeldDist:
		weldThresh = val;
		if (hGeom) {
			spin = GetISpinner (GetDlgItem (hGeom, IDC_W_THR_SPIN));
			spin->SetValue (val, FALSE);
			ReleaseISpinner (spin);
		}
		break;
	case MuiNormalSize:
		normScale = val;
		if (normScale<.01f) normScale=.01f;
		if (hSel) {
			spin = GetISpinner (GetDlgItem (hSel, IDC_NORMAL_SCALESPIN));
			spin->SetValue (val, FALSE);
			ReleaseISpinner (spin);
		}
		break;
	}
}

void EditTriObject::GetUIParam (meshUIParam uiCode, int & ret) {
	if (!ip) return;
	if (!Editing()) return;

	switch (uiCode) {
	case MuiSelByVert:
		ret = selByVert;
		break;
	case MuiIgBack:
		ret = ignoreBackfaces;
		break;
	case MuiIgnoreVis:
		ret = ignoreVisEdge;
		break;
	case MuiSoftSel:
		ret = affectRegion;
		break;
	case MuiSSUseEDist:
		ret = useEdgeDist;
		break;
	case MuiSSEDist:
		ret = edgeIts;
		break;
	case MuiSSBack:
		ret = arIgBack;
		break;
	case MuiWeldBoxSize:
		ret = weldBoxSize;
		break;
	case MuiExtrudeType:
		ret = extType;
		break;
	case MuiShowVNormals:
		ret = showVNormals;
		break;
	case MuiShowFNormals:
		ret = showFNormals;
		break;
	case MuiSliceSplit:
		ret = sliceSplit;
		break;
	case MuiCutRefine:
		ret = cutRefine;
		break;
	}
}

void EditTriObject::SetUIParam (meshUIParam uiCode, int val) {
	if (!ip) return;
	if (!Editing()) return;
	ISpinnerControl *spin;

	switch (uiCode) {
	case MuiSelByVert:
		selByVert = val ? TRUE : FALSE;
		if (hSel) CheckDlgButton (hSel, IDC_SEL_BYVERT, selByVert);
		break;
	case MuiIgBack:
		ignoreBackfaces = val ? TRUE : FALSE;
		if (hSel) CheckDlgButton (hSel, IDC_IGNORE_BACKFACES, ignoreBackfaces);
		break;
	case MuiIgnoreVis:
		ignoreVisEdge = val ? TRUE : FALSE;
		if (hSel) CheckDlgButton (hSel, IDC_IGNORE_VISEDGE, ignoreVisEdge);
		break;
	case MuiSoftSel:
		affectRegion = val ? TRUE : FALSE;
		if (hAR) {
			CheckDlgButton (hAR, IDC_AFFECT_REGION, affectRegion);
			SetARDlgEnables ();
		}
		break;
	case MuiSSUseEDist:
		useEdgeDist = val ? TRUE : FALSE;
		if (hAR) {
			CheckDlgButton (hAR, IDC_E_DIST, useEdgeDist);
			spin = GetISpinner (GetDlgItem (hAR, IDC_E_ITER_SPIN));
			spin->Enable (useEdgeDist);
			ReleaseISpinner (spin);
		}
		break;
	case MuiSSEDist:
		edgeIts = val;
		if (hAR) {
			spin = GetISpinner (GetDlgItem (hAR, IDC_E_ITER_SPIN));
			spin->SetValue (edgeIts, FALSE);
			ReleaseISpinner (spin);
		}
		break;
	case MuiSSBack:
		arIgBack = val ? TRUE : FALSE;
		if (hAR) CheckDlgButton (hAR, IDC_AR_BACK, !arIgBack); // LAM - 6/18/02 - defect 430856
		break;
	case MuiWeldBoxSize:
		weldBoxSize = val;
		if (hGeom) {
			spin = GetISpinner (GetDlgItem (hGeom, IDC_T_THR_SPIN));
			spin->SetValue (val, FALSE);
			ReleaseISpinner (spin);
		}
		break;
	case MuiExtrudeType:
		extType = val;
		if (hGeom) {
			if (extType == MESH_EXTRUDE_CLUSTER) {
				CheckRadioButton (hGeom, IDC_EXTYPE_A, IDC_EXTYPE_B, IDC_EXTYPE_A);
			} else {
				CheckRadioButton (hGeom, IDC_EXTYPE_A, IDC_EXTYPE_B, IDC_EXTYPE_B);
			}
		}
		break;
	case MuiShowVNormals:
		showVNormals = val?true:false;
		if (hSel && (selLevel == SL_VERTEX))
			CheckDlgButton (hSel, IDC_NORMAL_SHOW, val);
		break;
	case MuiShowFNormals:
		showFNormals = val?true:false;
		if (hSel && (selLevel >= SL_FACE))
			CheckDlgButton (hSel, IDC_NORMAL_SHOW, val);
		break;

	case MuiSliceSplit:
		sliceSplit = val?true:false;
		if (hGeom != NULL)
			CheckDlgButton(hGeom, IDC_SPLIT, sliceSplit);
		break;

	case MuiCutRefine:
		cutRefine = val?true:false;
		if (hGeom != NULL)
			CheckDlgButton(hGeom, IDC_REFINE, cutRefine);
		break;
	}
}

void EditTriObject::ToggleCommandMode(meshCommandMode mode) {
	if (!ip) return;
	if (sliceMode && (mode != McmSlicePlane)) ExitSliceMode ();

	switch (mode) {
	case McmCreate:
		switch (selLevel) {
		case SL_EDGE:
		case SL_OBJECT:
			break;
		case SL_VERTEX:
			if (ip->GetCommandMode()==createVertMode)
				ip->DeleteMode (createVertMode);
			else ip->PushCommandMode(createVertMode);
			break;
		default:
			if (ip->GetCommandMode()==createFaceMode)
				ip->DeleteMode (createFaceMode);
			else ip->PushCommandMode (createFaceMode);
			break;
		}
		break;

	case McmAttach:
		// Need to make sure we exit out of any other special command modes:
		if (ip->GetCommandMode() == weldVertMode) ip->DeleteMode (weldVertMode);
		if (ip->GetCommandMode() == createVertMode) ip->DeleteMode (createVertMode);
		if (ip->GetCommandMode() == createFaceMode) ip->DeleteMode (createFaceMode);
		if (ip->GetCommandMode() == divideFaceMode) ip->DeleteMode (divideFaceMode);
		if (ip->GetCommandMode() == divideEdgeMode) ip->DeleteMode (divideEdgeMode);
		if (ip->GetCommandMode() == turnEdgeMode) ip->DeleteMode (turnEdgeMode);
		if (ip->GetCommandMode() == extrudeMode) ip->DeleteMode (extrudeMode);
		if (ip->GetCommandMode() == bevelMode) ip->DeleteMode (bevelMode);
		if (ip->GetCommandMode() == chamferMode) ip->DeleteMode (chamferMode);
		if (ip->GetCommandMode() == flipMode) ip->DeleteMode (flipMode);
		if (ip->GetCommandMode() == cutEdgeMode) ip->DeleteMode (cutEdgeMode);
		if (sliceMode) ExitSliceMode ();
		ip->SetPickMode (attachPickMode);
		break;

	case McmExtrude:
		if (selLevel < SL_EDGE) break;
		if (ip->GetCommandMode()==extrudeMode)
			ip->DeleteMode(extrudeMode);
		else ip->PushCommandMode (extrudeMode);
		break;

	case McmBevel:
		if (selLevel < SL_FACE) break;
		if (ip->GetCommandMode()==bevelMode)
			ip->DeleteMode (bevelMode);
		else ip->PushCommandMode (bevelMode);
		break;

	case McmChamfer:
		if ((selLevel == SL_OBJECT) || (selLevel > SL_EDGE)) break;
		if (ip->GetCommandMode()==chamferMode)
			ip->DeleteMode (chamferMode);
		else ip->PushCommandMode (chamferMode);
		break;

	case McmSlicePlane:
		if (!selLevel) break;
		if (sliceMode) ExitSliceMode();
		else EnterSliceMode ();
		break;

	case McmCut:
		if (selLevel < SL_EDGE) break;
		if (ip->GetCommandMode()==cutEdgeMode)
			ip->DeleteMode(cutEdgeMode);
		else ip->PushCommandMode(cutEdgeMode);
		break;

	case McmWeldTarget:
		if (selLevel != SL_VERTEX) break;
		if (ip->GetCommandMode()==weldVertMode)
			ip->DeleteMode(weldVertMode);
		else ip->PushCommandMode(weldVertMode);
		break;

	case McmFlipNormalMode:
		if (selLevel < SL_FACE) break;
		if (ip->GetCommandMode()==flipMode)
			ip->DeleteMode(flipMode);
		else ip->PushCommandMode(flipMode);
		break;

	case McmDivide:
		if (selLevel < SL_EDGE) break;
		if (selLevel == SL_EDGE) {
			if (ip->GetCommandMode()==divideEdgeMode)
				ip->DeleteMode(divideEdgeMode);
			else ip->PushCommandMode (divideEdgeMode);
		} else {
			if (ip->GetCommandMode()==divideFaceMode)
				ip->DeleteMode(divideFaceMode);
			else ip->PushCommandMode (divideFaceMode);
		}
		break;

	case McmTurnEdge:
		if (selLevel != SL_EDGE) break;
		if (ip->GetCommandMode()==turnEdgeMode)
			ip->DeleteMode (turnEdgeMode);
		else ip->PushCommandMode (turnEdgeMode);
		break;
	}
}

void EditTriObject::ButtonOp(meshButtonOp opcode) {
	if (!ip) return; // LAM: added 9/3/00
	TSTR name;

	switch (opcode) {
	case MopHide:
		if (selLevel == SL_VERTEX) HideSelectedVerts ();
		else if (selLevel >= SL_FACE) HideSelectedFaces ();
		break;

	case MopUnhideAll:
		if (selLevel == SL_VERTEX) UnhideAllVerts();
		else if (selLevel >= SL_FACE) UnhideAllFaces ();
		break;

	case MopDelete:
		DeleteSelected ();
		break;

	case MopDetach:
		if (!hGeom || !editObj || !ip) break;
		if (selLevel == SL_VERTEX) {
			BOOL elem, asClone;
			TSTR name;
			if (mesh.vertSel.NumberSet() == 0) break;
			if (GetDetachObjectName (ip, name, elem, asClone)) {
				Detach (name, FALSE, !asClone, elem);
			}
		}
		if (selLevel >= SL_FACE) {
			BOOL elem, asClone;
			TSTR name;
			if (mesh.faceSel.NumberSet() == 0) break;
			if (GetDetachObjectName (ip, name, elem, asClone)) {
				Detach (name, TRUE, !asClone, elem);
			}
		}
		break;

	case MopBreak:
		BreakVerts ();
		break;

	case MopViewAlign:
		AlignTo(ALIGN_VIEW);
		break;

	case MopGridAlign:
		AlignTo(ALIGN_CONST);
		break;

	case MopMakePlanar:
		if (selLevel > SL_OBJECT) MakePlanar ();
		break;

	case MopCollapse:
		if (selLevel > SL_OBJECT) Collapse ();
		break;

	case MopTessellate:
		if (selLevel > SL_EDGE) Tessellate (tessTens/400.0f, edgeTes);
		break;

	case MopExplode:
		if ((selLevel==SL_OBJECT) && (GetMesh().numFaces == 0)) break;
		if ((selLevel>SL_EDGE) && (GetMesh().faceSel.NumberSet() == 0)) break;
		if ((selLevel > SL_EDGE) || (selLevel == SL_OBJECT)) {
			if (expObj) {
				name = GetString(IDS_SCA_BASEMESH);
				if (GetExplodeObjectName (ip->GetMAXHWnd(), name)) {
					Explode (DegToRad(explodeThresh), TRUE, name);
				}
			} else {
				Explode (DegToRad (explodeThresh), FALSE, name);
			}
		}
		break;

	case MopSlice:
		if (sliceMode && selLevel) Slice ();
		break;

	case MopWeld:
		if (selLevel == SL_VERTEX) {
			if (!WeldVerts(weldThresh)) {
				TSTR buf1 = GetString(IDS_RB_NOVERTSTOWELD);
				TSTR buf2 = GetString(IDS_RB_WELDVERTS);
				MessageBox (ip->GetMAXHWnd(), buf1, buf2, MB_OK|MB_TASKMODAL);
			}
		}
		break;

	case MopRemoveIsolatedVerts:
		RemoveIsoVerts ();
		break;

	case MopSelectOpenEdges:
		if (selLevel == SL_EDGE) SelectOpenEdges ();
		break;

	case MopCreateShapeFromEdges:
		if (selLevel != SL_EDGE) break;
		if (!GetMesh().edgeSel.NumberSet()) {
			TSTR buf1 = GetString(IDS_RB_CREATECURVE);
			TSTR buf2 = GetString(IDS_RB_NOEDGESSELECTED);
			MessageBox (ip->GetMAXHWnd(),buf2,buf1,MB_ICONEXCLAMATION|MB_OK);
			break;
		}
		name = GetString(IDS_RB_SHAPE);
		ip->MakeNameUnique(name);
		if (DialogBoxParam (hInstance, MAKEINTRESOURCE(IDD_CREATECURVE),
			ip->GetMAXHWnd(), CurveNameDlgProc, (LPARAM)&name)) {
			ModContextList mcList;
			INodeTab nodes;
			ip->GetModContexts (mcList, nodes);
			AdjEdgeList *ae = TempData()->AdjEList();
			theHold.Begin();
			CreateCurveFromMeshEdges (GetMesh(), nodes[0], ip, ae, name,
				(createCurveType==IDC_CURVE_SMOOTH), curveIgnoreHiddenEdges);
			theHold.Accept (GetString(IDS_RB_CREATECURVE));
			nodes.DisposeTemporary();
		}
		break;

	case MopShowNormal:
		switch (selLevel) {
		case SL_VERTEX:
			showVNormals = !showVNormals;
			ShowNormals ();
			if (hSel) CheckDlgButton (hSel, IDC_NORMAL_SHOW, showVNormals);
			break;
		case SL_FACE:
		case SL_POLY:
		case SL_ELEMENT:
			showFNormals = !showFNormals;
			ShowNormals ();
			if (hSel) CheckDlgButton (hSel, IDC_NORMAL_SHOW, showFNormals);
			break;
		}
		break;

	case MopFlipNormal:
		if (selLevel > SL_EDGE) FlipNormals ();
		break;

	case MopUnifyNormal:
		if (selLevel > SL_EDGE) UnifyNormals ();
		break;

	case MopAutoSmooth:
		if (selLevel > SL_EDGE) AutoSmooth (DegToRad (autoSmoothThresh));
		break;

	case MopVisibleEdge:
		if (selLevel == SL_EDGE) SetEdgeVis (TRUE);
		break;

	case MopInvisibleEdge:
		if (selLevel == SL_EDGE) SetEdgeVis (FALSE);
		break;

	case MopAutoEdge:
		if (selLevel == SL_EDGE) AutoEdge (DegToRad (autoEdgeThresh), autoEdgeType);
		break;
//LAM; added 9/3/00
	case MopSelectByID:
		if (selLevel > SL_EDGE) {                                        
			ISpinnerControl *spin;
			int clearSel;
			spin = GetISpinner(GetDlgItem(hSurf,IDC_MAT_IDSPIN_SEL));
			clearSel = IsDlgButtonChecked(hSurf,IDC_CLEARSELECTION);
			SelectByMat(spin->GetIVal()-1,clearSel); 
			ReleaseISpinner(spin);  
		}                                                              
		break;
	case MopSelectBySG:
		if (selLevel > SL_EDGE) {
			sbsParams[2] = ~GetUsedSmoothBits();
			if (DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_EM_SELECTBYSMOOTH),
						ip->GetMAXHWnd(), SelectBySmoothDlgProc, (LPARAM)sbsParams)) {
				SelectBySmoothGroup(sbsParams[0],(BOOL)sbsParams[1]);
			}
		}
		break;
	case MopClearAllSG:
		if (selLevel > SL_EDGE) SetSelSmoothBits(0,0xffffffff);
		break;
	case MopSelectByColor:
		if (selLevel == SL_VERTEX) {
			BOOL add, sub;
			add = GetKeyState(VK_CONTROL)<0;
			sub = GetKeyState(VK_MENU)<0;
			SelectVertByColor (selByColor,selDeltaR,selDeltaG,selDeltaB,add,sub, selByChannel);
		}
		break;
	case MopAttachList:
		if (selLevel == SL_OBJECT) {
			AttachHitByName proc(this);
			ip->DoHitByNameDialog(&proc);
		}
		break;
	case MopCopyNS:
		NSCopy();
		break;
	case MopPasteNS:
		NSPaste();
		break;

	case MopEditVertColor:
		EditVertColor(0, IDC_VERT_COLOR, IDS_RB_VERTEXCOLOR);
		break;

	case MopEditVertIllum:
		EditVertColor(MAP_SHADING, IDC_VERT_ILLUM, IDS_VERTEX_ILLUM);
		break;
	}
}

Object *EditTriObject::CollapseObject () {
	EditTriObject *eto = (EditTriObject *) this->Clone ();
	// NOTE: ideal solution someday would be to set eto->selLevel based on mesh.selLevel,
	// not the other way around as shown here.  This merely creates consistency.
	eto->mesh.selLevel = meshSelLevel[selLevel];

	eto->mesh.ClearSpecifiedNormals ();

	return (Object *)eto;
}

int EditTriObject::NumSubObjTypes() 
{ 
	return 5;
}

ISubObjType *EditTriObject::GetSubObjType(int i) 
{
	static bool initialized = false;
	if(!initialized)
	{
		initialized = true;
		SOT_Vertex.SetName(GetString(IDS_VERTEX));
		SOT_Edge.SetName(GetString(IDS_EDGE));
		SOT_Face.SetName(GetString(IDS_FACE));
		SOT_Poly.SetName(GetString(IDS_POLYGON));
		SOT_Element.SetName(GetString(IDS_ELEMENT));
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

void EditTriObject::EditVertColor(int mp, int ctrlID, int strID )
{
	if (hSurf == NULL || selLevel == SL_OBJECT || selLevel == SL_EDGE)
		return;
	
	HWND hWnd = GetDlgItem (hSurf, ctrlID);
	if (hWnd == NULL) 
		return;
	
	Color col;
	if (selLevel == SL_VERTEX)
		col = GetVertColor(mp);
	else
		col = GetFaceColor(mp);

	IColorSwatch* iCol;
	if ((iCol = GetIColorSwatch (hWnd, col, GetString(strID))) == NULL)
		return;

	iCol->EditThis (TRUE);
	ReleaseIColorSwatch(iCol);
}


//az -  042303  MultiMtl sub/mtl name support
void GetMtlIDList(Mtl *mtl, NumList& mtlIDList)
{
	if (mtl != NULL && mtl->IsMultiMtl()) {
		int subs = mtl->NumSubMtls();
		if (subs <= 0)
			subs = 1;
		for (int i=0; i<subs;i++){
			if(mtl->GetSubMtl(i))
				mtlIDList.Add(i, TRUE);  

		}
	}
}

void GetMeshMtlIDList(EditTriObject *eo, NumList& mtlIDList)
{
	if (eo) {
		int c_numFaces = eo->GetMesh().numFaces;
		for (int i=0; i<c_numFaces; i++) {
			int mid = eo->GetMesh().faces[i].getMatID();
			if (mid != -1){
				mtlIDList.Add(mid, TRUE);
			}
		}
	}
}


INode* GetNode (EditTriObject *eo){
	ModContextList mcList;
	INodeTab nodes;
	eo->ip->GetModContexts (mcList, nodes);
	INode* objnode = nodes.Count() == 1 ? nodes[0]->GetActualINode(): NULL;
	nodes.DisposeTemporary();
	return objnode;
}

BOOL SetupMtlSubNameCombo (HWND hWnd, EditTriObject *eo) {
	INode* singleNode;
	Mtl *nodeMtl;
	
	singleNode = GetNode(eo);
	if(singleNode)
		nodeMtl = singleNode->GetMtl();
	if(singleNode == NULL || nodeMtl == NULL || !nodeMtl->IsMultiMtl()) {    //no UI for cloned nodes, and not MultiMtl 
		SendMessage(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), CB_RESETCONTENT, 0, 0);
		EnableWindow(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), false);
		return false;
	}
	NumList mtlIDList;
	NumList mtlIDMeshList;
	GetMtlIDList(nodeMtl, mtlIDList);
	GetMeshMtlIDList(eo, mtlIDMeshList);
	MultiMtl *nodeMulti = (MultiMtl*) nodeMtl;
	EnableWindow(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), true);
	SendMessage(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), CB_RESETCONTENT, 0, 0);

	for (int i=0; i<mtlIDList.Count(); i++){
		TSTR idname, buf;
		if(mtlIDMeshList.Find(mtlIDList[i]) != -1) {
			nodeMulti->GetSubMtlName(mtlIDList[i], idname); 
			if (idname.isNull())
				idname = GetString(IDS_MTL_NONAME);                                 //az: 042303  - FIGS
			buf.printf(_T("%s - ( %d )"), idname.data(), mtlIDList[i]+1);
			int ith = SendMessage(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), CB_ADDSTRING, 0, (LPARAM)buf.data());
			SendMessage(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), CB_SETITEMDATA, ith, (LPARAM)mtlIDList[i]);
		}
	}
	return true;
}

void UpdateNameCombo (HWND hWnd, ISpinnerControl *spin) {
	int cbcount, sval, cbval;
	sval = spin->GetIVal() - 1;          
	if (!spin->IsIndeterminate()){
		cbcount = SendMessage(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), CB_GETCOUNT, 0, 0);
		if (cbcount > 0){
			for (int index=0; index<cbcount; index++){
				cbval = SendMessage(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), CB_GETITEMDATA, (WPARAM)index, 0);
				if (sval == cbval) {
					SendMessage(GetDlgItem( hWnd, IDC_MTLID_NAMES_COMBO), CB_SETCURSEL, (WPARAM)index, 0);
					return;
				}
			}
		}
	}
	SendMessage(GetDlgItem( hWnd, IDC_MTLID_NAMES_COMBO), CB_SETCURSEL, (WPARAM)-1, 0);	
}


void ValidateUINameCombo (HWND hWnd, EditTriObject *eo) {
	SetupMtlSubNameCombo (hWnd, eo);
	ISpinnerControl *spin;
	spin = GetISpinner(GetDlgItem(hWnd,IDC_MAT_IDSPIN));
	if (spin)
		UpdateNameCombo (hWnd, spin);
	ReleaseISpinner(spin);
}
