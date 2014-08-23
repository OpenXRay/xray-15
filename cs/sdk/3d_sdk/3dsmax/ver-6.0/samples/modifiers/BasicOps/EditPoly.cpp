/**********************************************************************
 *<
	FILE: EditPoly.cpp

	DESCRIPTION: Edit Poly Modifier

	CREATED BY: Steve Anderson, based on Face Extrude modifier by Berteig, and my own Poly Select modifier.

	HISTORY: created March 2002

 *>	Copyright (c) 2002 Discreet, All Rights Reserved.
 **********************************************************************/

#include "BasicOps.h"
#include "iparamm2.h"
#include "MeshDLib.h"
#include "spline3d.h"
#include "splshape.h"
#include "shape.h"
#include "EditPoly.h"

static GenSubObjType SOT_Vertex(1);
static GenSubObjType SOT_Edge(2);
static GenSubObjType SOT_Border(9);
static GenSubObjType SOT_Face(4);
static GenSubObjType SOT_Element(5);

#define WM_UPDATE_CACHE		(WM_USER+0x28e)

// Polymesh selection toolbar icons - used in select and edit tools.
class PolySelImageHandler {
public:
	HIMAGELIST hImages;

	PolySelImageHandler () : hImages(NULL) { }
	~PolySelImageHandler () { if (hImages) ImageList_Destroy (hImages); }
	HIMAGELIST LoadImages ();
};

PolySelImageHandler thePolySelImageHandler;

HIMAGELIST PolySelImageHandler::LoadImages() {
	if (hImages ) return hImages;

	HBITMAP hBitmap, hMask;
	hImages = ImageList_Create(24, 23, ILC_COLOR|ILC_MASK, 10, 0);
	hBitmap = LoadBitmap(hInstance,MAKEINTRESOURCE (IDB_SELTYPES));
	hMask = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_SELMASK));
	ImageList_Add (hImages, hBitmap, hMask);
	DeleteObject (hBitmap);
	DeleteObject (hMask);
	return hImages;
}

#define IDC_SELVERTEX 0x3260
#define IDC_SELEDGE 0x3261
#define IDC_SELBORDER 0x3262
#define IDC_SELFACE 0x3263
#define IDC_SELELEMENT 0x3264

// SelectRestore --------------------------------------------------

SelectRestore::SelectRestore(EditPolyMod *pMod, EditPolyData *pData) : mpMod(pMod), mpData(pData) {
	mLevel = mpMod->mSelLevel;
	mpData->SetHeld();

	switch (mLevel) {
	case kSelLevObject: DbgAssert(0); break;
	case kSelLevVertex: mUndoSel = mpData->mVertSel; break;
	case kSelLevEdge:
	case kSelLevBorder:
		mUndoSel = mpData->mEdgeSel;
		break;
	default:
		mUndoSel = mpData->mFaceSel;
		break;
	}
}

SelectRestore::SelectRestore(EditPolyMod *pMod, EditPolyData *pData, int sLevel) : mpMod(pMod), mpData(pData), mLevel(sLevel) {
	mpData->SetHeld();

	switch (mLevel) {
	case kSelLevObject: DbgAssert(0); break;
	case kSelLevVertex: mUndoSel = mpData->mVertSel; break;
	case kSelLevEdge:
	case kSelLevBorder:
		mUndoSel = mpData->mEdgeSel; break;
	default:
		mUndoSel = mpData->mFaceSel; break;
	}
}

void SelectRestore::After () {
	switch (mLevel) {			
	case kSelLevVertex:
		mRedoSel = mpData->mVertSel; break;
	case kSelLevEdge:
	case kSelLevBorder:
		mRedoSel = mpData->mEdgeSel; break;
	case kSelLevFace:
	case kSelLevElement:
		mRedoSel = mpData->mFaceSel; break;
	}
}

void SelectRestore::Restore(int isUndo) {
	if (isUndo && mRedoSel.GetSize() == 0) After ();
	switch (mLevel) {
	case kSelLevVertex:
		mpData->mVertSel = mUndoSel; break;
	case kSelLevFace:
	case kSelLevElement:
		mpData->mFaceSel = mUndoSel; break;
	case kSelLevEdge:
	case kSelLevBorder:
		mpData->mEdgeSel = mUndoSel; break;
	}
	mpMod->LocalDataChanged ();
}

void SelectRestore::Redo() {
	switch (mLevel) {		
	case kSelLevVertex:
		mpData->mVertSel = mRedoSel; break;
	case kSelLevFace:
	case kSelLevElement:
		mpData->mFaceSel = mRedoSel; break;
	case kSelLevEdge:
	case kSelLevBorder:
		mpData->mEdgeSel = mRedoSel; break;
	}
	mpMod->LocalDataChanged ();
}

class ShapePickMode : public PickModeCallback, public PickNodeCallback {
	EditPolyMod *mpMod;
	IObjParam *mpInterface;

public:
	ShapePickMode() : mpMod(NULL), mpInterface(NULL) { }
	void SetMod (EditPolyMod *pMod, IObjParam *i) { mpMod = pMod; mpInterface = i; }
	void ClearMod () { mpMod=NULL; mpInterface=NULL; }
	BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
	BOOL Pick(IObjParam *ip,ViewExp *vpt);
	void EnterMode(IObjParam *ip);
	void ExitMode(IObjParam *ip);		

	BOOL Filter(INode *node);
	BOOL RightClick(IObjParam *ip,ViewExp *vpt) {return TRUE;}
	PickNodeCallback *GetFilter() {return this;}
};


//--- ClassDescriptor and class vars ---------------------------------

Tab<PolyOperation *> EditPolyMod::mOpList;
IObjParam *EditPolyMod::mpInterface = NULL;
EditPolyMod *EditPolyMod::mpCurrentEditMod = NULL;
SelectModBoxCMode *EditPolyMod::mpSelectMode = NULL;
bool EditPolyMod::mUpdateCachePosted = FALSE;
IParamMap2 *EditPolyMod::mpDialogType = NULL;
IParamMap2 *EditPolyMod::mpDialogSelect = NULL;
IParamMap2 *EditPolyMod::mpDialogOperation = NULL;
ShapePickMode *EditPolyMod::mpShapePicker = NULL;

class EditPolyModClassDesc : public ClassDesc2 {
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new EditPolyMod; }
	const TCHAR *	ClassName() { return GetString(IDS_EDIT_POLY_MOD); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return kEDIT_MOD_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_MAX_STANDARD);}

	// The following are used by MAX Script and the schematic view:
	const TCHAR *InternalName() { return _T("EditPolyMod"); }
	HINSTANCE HInstance() { return hInstance; }
};

static EditPolyModClassDesc emodDesc;
extern ClassDesc* GetEditPolyModDesc() {return &emodDesc;}

//--- Parameter map/block descriptors -------------------------------

// Parameters
static ParamBlockDesc2 edit_mod_param_blk (kEditPolyParams, _T("Edit Poly Parameters"), 0, &emodDesc,
											   P_AUTO_CONSTRUCT | P_AUTO_UI | P_MULTIMAP, EDIT_PBLOCK_REF,
	// rollout descriptions:
	// (Leave out kEditPolySettings)
	2,
	kEditPolyChoice, IDD_EP_CHOICE, IDS_EP_EDIT_TYPE, 0, 0, NULL,
	kEditPolySelect, IDD_EP_SELECT, IDS_EP_SELECTION, 0, 0, NULL,

	// Parameters
	// Type of edit:
	kEditPolyType, _T("editType"), TYPE_INT, 0, IDS_EP_EDIT_TYPE,
		p_default, 0,
		end,

	// Selection:
	kEPExplicitSelection, _T("explicitSelection"), TYPE_INT, 0, IDS_EP_EXPLICIT_SELECTION,
		p_default, true,
		p_ui, kEditPolySelect, TYPE_RADIO, 2, IDC_EP_SEL_PIPE, IDC_EP_SEL_EXPLICIT,
		end,

	kEPSelByVertex, _T("selectByVertex"), TYPE_INT, P_RESET_DEFAULT, IDS_EP_SELECT_BY_VERTEX,
		p_default, 0,
		p_ui, kEditPolySelect, TYPE_SINGLECHEKBOX, IDC_EP_SEL_BYVERT,
		end,

	kEPIgnoreBackfacing, _T("ignoreBackfacing"), TYPE_INT, P_RESET_DEFAULT, IDS_EP_IGNORE_BACKFACING,
		p_default, 0,
		p_ui, kEditPolySelect, TYPE_SINGLECHEKBOX, IDC_EP_IGNORE_BACKFACING,
		end,

	// Individual edit type parameters
	kEPWeldThreshold, _T("weldThreshold"), TYPE_FLOAT, P_RESET_DEFAULT|P_ANIMATABLE, IDS_EP_WELD_THRESHOLD,
		p_default, 1.0f,
		p_range, 0.0f, BIGFLOAT,
		p_ui, kEditPolySettings, TYPE_SPINNER, EDITTYPE_POS_UNIVERSE, IDC_VW_THRESH, IDC_VW_THRESHSPIN, SPIN_AUTOSCALE,
		end,

	kEPChamferAmount, _T("chamferAmount"), TYPE_FLOAT, P_RESET_DEFAULT|P_ANIMATABLE, IDS_EP_CHAMFER_AMOUNT,
		p_default, 0.0f,
		p_range, 0.0f, BIGFLOAT,
		p_ui, kEditPolySettings, TYPE_SPINNER, EDITTYPE_POS_UNIVERSE, IDC_VCH_AMOUNT, IDC_VCH_AMOUNTSPIN, SPIN_AUTOSCALE,
		end,

	kEPExtrudeHeight, _T("extrudeHeight"), TYPE_FLOAT, P_RESET_DEFAULT|P_ANIMATABLE, IDS_EP_EXTRUDE_HEIGHT,
		p_default, 0.0f,
		p_ui, kEditPolySettings, TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_EXTRUDE_HEIGHT, IDC_EXTRUDE_HEIGHTSPIN, SPIN_AUTOSCALE,
		end,

	kEPExtrudeType, _T("extrudeType"), TYPE_INT, P_RESET_DEFAULT, IDS_EP_EXTRUDE_TYPE,
		p_default, 0,
		p_ui, kEditPolySettings, TYPE_RADIO, 3, IDC_FEX_TYPE_GROUP,
			IDC_FEX_TYPE_LOCAL, IDC_FEX_TYPE_BYFACE,
		end,

	kEPExtrudeWidth, _T("extrudeWidth"), TYPE_FLOAT, P_RESET_DEFAULT|P_ANIMATABLE, IDS_EP_EXTRUDE_WIDTH,
		p_default, 0.1f,
		p_range, 0.0f, BIGFLOAT,
		p_ui, kEditPolySettings, TYPE_SPINNER, EDITTYPE_POS_UNIVERSE,
			IDC_EXTRUDE_WIDTH, IDC_EXTRUDE_WIDTHSPIN, SPIN_AUTOSCALE,
		end,

	kEPBevelHeight, _T("bevelHeight"), TYPE_FLOAT, P_RESET_DEFAULT|P_ANIMATABLE, IDS_EP_BEVEL_HEIGHT,
		p_default, 4.0f,
		p_range, -BIGFLOAT, BIGFLOAT,
		p_ui, kEditPolySettings, TYPE_SPINNER, EDITTYPE_POS_UNIVERSE,
			IDC_EP_BEVEL_HEIGHT, IDC_EP_BEVEL_HEIGHTSPIN, SPIN_AUTOSCALE,
		end,

	kEPBevelOutline, _T("bevelOutline"), TYPE_FLOAT, P_RESET_DEFAULT|P_ANIMATABLE, IDS_EP_BEVEL_OUTLINE,
		p_default, -4.0f,
		p_range, -BIGFLOAT, BIGFLOAT,
		p_ui, kEditPolySettings, TYPE_SPINNER, EDITTYPE_POS_UNIVERSE,
			IDC_EP_BEVEL_OUTLINE, IDC_EP_BEVEL_OUTLINESPIN, SPIN_AUTOSCALE,
		end,

	kEPExtrudeSplineSegments, _T("extrudeAlongSplineSegments"), TYPE_INT, P_RESET_DEFAULT|P_ANIMATABLE, IDS_EP_EXTRUDE_SEGMENTS,
		p_default, 10,
		p_range, 1, 999999,
		p_ui, kEditPolySettings, TYPE_SPINNER, EDITTYPE_POS_INT,
			IDC_EP_EXTRUDE_SEGMENTS, IDC_EP_EXTRUDE_SEGMENTS_SPIN, .5f,
		end,

	kEPExtrudeSplineTaper, _T("extrudeAlongSplineTaper"), TYPE_FLOAT, P_RESET_DEFAULT|P_ANIMATABLE, IDS_EP_EXTRUDE_TAPER,
		p_default, 0.0f,
		p_range, -BIGFLOAT, BIGFLOAT,
		p_ui, kEditPolySettings, TYPE_SPINNER, EDITTYPE_POS_FLOAT,
			IDC_EP_EXTRUDE_TAPER, IDC_EP_EXTRUDE_TAPER_SPIN, SPIN_AUTOSCALE,
		end,

	kEPExtrudeSplineTaperCurve, _T("extrudeAlongSplineTaperCurve"), TYPE_FLOAT, P_RESET_DEFAULT|P_ANIMATABLE, IDS_EP_EXTRUDE_TAPER_CURVE,
		p_default, 0.0f,
		p_range, -BIGFLOAT, BIGFLOAT,
		p_ui, kEditPolySettings, TYPE_SPINNER, EDITTYPE_POS_FLOAT,
			IDC_EP_EXTRUDE_TAPER_CURVE, IDC_EP_EXTRUDE_TAPER_CURVE_SPIN, SPIN_AUTOSCALE,
		end,

	kEPExtrudeSplineTwist, _T("extrudeAlongSplineTwist"), TYPE_FLOAT, P_RESET_DEFAULT|P_ANIMATABLE, IDS_EP_EXTRUDE_TWIST,
		p_default, 0.0f,
		p_dim, stdAngleDim,
		p_range, -36000.0f, 36000.0f,
		p_ui, kEditPolySettings, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_EP_EXTRUDE_TWIST, IDC_EP_EXTRUDE_TWIST_SPIN, .5f,
		end,

	kEPExtrudeSplineRotation, _T("extrudeAlongSplineRotation"), TYPE_FLOAT, P_RESET_DEFAULT|P_ANIMATABLE, IDS_EP_EXTRUDE_ROTATION,
		p_default, 0.0f,
		p_dim, stdAngleDim,
		p_range, -36000.0f, 36000.0f,
		p_ui, kEditPolySettings, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_EP_EXTRUDE_ROTATION, IDC_EP_EXTRUDE_ROTATION_SPIN, .5f,
		end,

	kEPExtrudeSpline, _T("extrudeAlongSplineNode"), TYPE_INODE, 0, IDS_EP_EXTRUDE_SPLINE,
		end,

	kEPExtrudeSplineAlign, _T("extrudeAlongSplineAlign"), TYPE_INT, P_RESET_DEFAULT, IDS_EP_EXTRUDE_ALIGN,
		p_default, 1,
		p_ui, kEditPolySettings, TYPE_SINGLECHEKBOX, IDC_EP_EXTRUDE_ALIGN_NORMAL,
		end,

	end
);


//--- Our UI handlers ---------------------------------
class EditPolyTypeProc : public ParamMap2UserDlgProc {
	EditPolyMod *mpEditPolyMod;

public:
	EditPolyTypeProc () : mpEditPolyMod(NULL) { }
	void SetEditPolyMod (EditPolyMod*e) { mpEditPolyMod = e; }
	BOOL DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd,
		UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() { }
	void UpdateEditType (HWND hWnd);
};

static EditPolyTypeProc theEditPolyTypeProc;

void EditPolyTypeProc::UpdateEditType (HWND hWnd) {
	if (!mpEditPolyMod) return;
	if (!mpEditPolyMod->getParamBlock()) return;

	int editType;
	editType = mpEditPolyMod->GetPolyOperationIndex (GetCOREInterface()->GetTime ());
	SendMessage (GetDlgItem (hWnd, IDC_EMOD_LIST), CB_SETCURSEL, editType, 0);
}

BOOL EditPolyTypeProc::DlgProc (TimeValue t, IParamMap2 *pmap, HWND hWnd,
						   UINT msg, WPARAM wParam, LPARAM lParam) {
	int editType, type;
	HWND hDroplist;

	switch (msg) {
	case WM_INITDIALOG:
		hDroplist = GetDlgItem (hWnd, IDC_EMOD_LIST);
		if (hDroplist) {
			SendMessage (hDroplist, CB_RESETCONTENT, 0, 0);
			for (int i=0; i<mpEditPolyMod->GetNumOperations(); i++) {
				PolyOperation *pop = mpEditPolyMod->GetPolyOperationByIndex (i);
				DbgAssert (pop != NULL);
				SendMessage (hDroplist, CB_ADDSTRING, 0, (LPARAM) pop->Name());
			}
			editType = mpEditPolyMod->GetPolyOperationIndex (t);
			SendMessage (hDroplist, CB_SETCURSEL, editType, 0);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_EMOD_LIST:
			{
			if (HIWORD(wParam) != CBN_SELCHANGE) break;
			hDroplist = GetDlgItem (hWnd, IDC_EMOD_LIST);
			if (!hDroplist) break;
			type = SendMessage (hDroplist, CB_GETCURSEL, 0, 0);
			if (type == CB_ERR) type=0;
			editType = mpEditPolyMod->GetPolyOperationIndex (t);
			if (type == editType) break;

			PolyOperation *pop = mpEditPolyMod->GetPolyOperationByIndex (type);
			if (pop == NULL)
			{
				DbgAssert (0);
				break;
			}

			theHold.Begin ();
			mpEditPolyMod->getParamBlock()->SetValue (kEditPolyType, t, pop->OpID ());
			theHold.Accept (GetString (IDS_EP_PARAMETERS));
			}
			break;

		case IDC_EP_COMMIT:
			theHold.Begin ();
			mpEditPolyMod->CommitToOperation ();
			theHold.Accept (GetString (IDS_EP_COMMIT));
			break;
		}
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

class EditPolySelectProc : public ParamMap2UserDlgProc {
	EditPolyMod *mpMod;

public:
	EditPolySelectProc () : mpMod(NULL) { }
	void SetEditPolyMod (EditPolyMod*e) { mpMod = e; }
	void SetEnables (HWND hWnd);
	void UpdateSelLevelDisplay (HWND hWnd);
	BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
	void DeleteThis() { }
};

static EditPolySelectProc theEditPolySelectProc;

static int butIDs[] = { 0, IDC_SELVERTEX, IDC_SELEDGE, IDC_SELBORDER, IDC_SELFACE, IDC_SELELEMENT };
void EditPolySelectProc::UpdateSelLevelDisplay (HWND hWnd) {
	if (!mpMod) return;
	ICustToolbar *iToolbar = GetICustToolbar(GetDlgItem(hWnd,IDC_EP_SELTYPE));
	ICustButton *but;
	for (int i=1; i<6; i++) {
		but = iToolbar->GetICustButton (butIDs[i]);
		but->SetCheck ((DWORD)i==mpMod->mSelLevel);
		ReleaseICustButton (but);
	}
	ReleaseICustToolbar (iToolbar);
	UpdateWindow(hWnd);
}

void EditPolySelectProc::SetEnables (HWND hParams) {
	if (!mpMod) return;
	int selLevel = mpMod->mSelLevel;

	int explicitSelection;
	mpMod->getParamBlock()->GetValue (kEPExplicitSelection, TimeValue(0), explicitSelection, FOREVER);
	if (explicitSelection) {
		bool soLevel = selLevel ? true : false;
		EnableWindow (GetDlgItem (hParams, IDC_EP_SEL_BYVERT), (selLevel>kSelLevVertex)?true:false);
		EnableWindow (GetDlgItem (hParams, IDC_EP_IGNORE_BACKFACING), soLevel);
		ICustButton *iBut = GetICustButton (GetDlgItem (hParams, IDC_EP_GET_PIPE_SELECTION));
		iBut->Enable (soLevel);
		ReleaseICustButton (iBut);
	} else {
		EnableWindow (GetDlgItem (hParams, IDC_EP_SEL_BYVERT), false);
		EnableWindow (GetDlgItem (hParams, IDC_EP_IGNORE_BACKFACING), false);
		ICustButton *iBut = GetICustButton (GetDlgItem (hParams, IDC_EP_GET_PIPE_SELECTION));
		iBut->Enable (false);
		ReleaseICustButton (iBut);
	}
}

BOOL EditPolySelectProc::DlgProc (TimeValue t, IParamMap2 *map,
										HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (!mpMod) return FALSE;
	ICustToolbar *iToolbar;

	switch (msg) {
	case WM_INITDIALOG:
		iToolbar = GetICustToolbar(GetDlgItem(hWnd,IDC_EP_SELTYPE));
		iToolbar->SetImage (thePolySelImageHandler.LoadImages());
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,0,5,0,5,24,23,24,23,IDC_SELVERTEX));
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,1,6,1,6,24,23,24,23,IDC_SELEDGE));
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,2,7,2,7,24,23,24,23,IDC_SELBORDER));
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,3,8,3,8,24,23,24,23,IDC_SELFACE));
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,4,9,4,9,24,23,24,23,IDC_SELELEMENT));
		ReleaseICustToolbar(iToolbar);

		UpdateSelLevelDisplay (hWnd);
		SetEnables (hWnd);
		break;

	case WM_UPDATE_CACHE:
		mpMod->UpdateCache((TimeValue)wParam);
 		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_SELVERTEX:
			if (mpMod->mSelLevel == kSelLevVertex) mpMod->mpInterface->SetSubObjectLevel (kSelLevObject);
			else mpMod->mpInterface->SetSubObjectLevel (kSelLevVertex);
			break;

		case IDC_SELEDGE:
			if (mpMod->mSelLevel == kSelLevEdge) mpMod->mpInterface->SetSubObjectLevel (kSelLevObject);
			else mpMod->mpInterface->SetSubObjectLevel (kSelLevEdge);
			break;

		case IDC_SELBORDER:
			if (mpMod->mSelLevel == kSelLevBorder) mpMod->mpInterface->SetSubObjectLevel (kSelLevObject);
			else mpMod->mpInterface->SetSubObjectLevel (kSelLevBorder);
			break;

		case IDC_SELFACE:
			if (mpMod->mSelLevel == kSelLevFace) mpMod->mpInterface->SetSubObjectLevel (kSelLevObject);
			else mpMod->mpInterface->SetSubObjectLevel (kSelLevFace);
			break;

		case IDC_SELELEMENT:
			if (mpMod->mSelLevel == kSelLevElement) mpMod->mpInterface->SetSubObjectLevel (kSelLevObject);
			else mpMod->mpInterface->SetSubObjectLevel (kSelLevElement);
			break;

		case IDC_EP_GET_PIPE_SELECTION:
			if (mpMod->mSelLevel == kSelLevObject) break;
			mpMod->AcquirePipeSelection ();
			mpMod->NotifyDependents (FOREVER, PART_SELECT, REFMSG_CHANGE);
			break;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code != TTN_NEEDTEXT) break;
		LPTOOLTIPTEXT lpttt;
		lpttt = (LPTOOLTIPTEXT)lParam;				
		switch (lpttt->hdr.idFrom) {
		case IDC_SELVERTEX:
			lpttt->lpszText = GetString (IDS_EP_VERTEX);
			break;
		case IDC_SELEDGE:
			lpttt->lpszText = GetString (IDS_EP_EDGE);
			break;
		case IDC_SELBORDER:
			lpttt->lpszText = GetString(IDS_EP_BORDER);
			break;
		case IDC_SELFACE:
			lpttt->lpszText = GetString(IDS_EP_FACE);
			break;
		case IDC_SELELEMENT:
			lpttt->lpszText = GetString(IDS_EP_ELEMENT);
			break;
		}
		break;
	
	default: return FALSE;
	}
	return TRUE;
}

class EditPolyOperationProc : public ParamMap2UserDlgProc {
	EditPolyMod *mpMod;

public:
	EditPolyOperationProc () : mpMod(NULL) { }
	void SetEditPolyMod (EditPolyMod*e) { mpMod = e; }
	void UpdateUI (HWND hWnd);
	BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
	void DeleteThis() { }
};

static EditPolyOperationProc theEditPolyOperationProc;

void EditPolyOperationProc::UpdateUI (HWND hWnd) {
	ICustButton *but = GetICustButton (GetDlgItem (hWnd, IDC_EP_EXTRUDE_PICK_SPLINE));
	if (but) {
		INode *splineNode;
		mpMod->getParamBlock()->GetValue (kEPExtrudeSpline, TimeValue(0), splineNode, FOREVER);
		if (splineNode) but->SetText (splineNode->GetName());
		else but->SetText (GetString (IDS_EP_PICK_SPLINE));
		ReleaseICustButton(but);
	}
}

BOOL EditPolyOperationProc::DlgProc (TimeValue t, IParamMap2 *map,
										HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	ICustButton *but;

	switch (msg) {
	case WM_INITDIALOG:
		but = GetICustButton (GetDlgItem (hWnd, IDC_EP_EXTRUDE_PICK_SPLINE));
		if (but) {
			but->SetType (CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);

			INode *splineNode;
			mpMod->getParamBlock()->GetValue (kEPExtrudeSpline, TimeValue(0), splineNode, FOREVER);
			if (splineNode) but->SetText (splineNode->GetName());
			else but->SetText (GetString (IDS_EP_PICK_SPLINE));
			ReleaseICustButton(but);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_EP_EXTRUDE_PICK_SPLINE:
			if (!mpMod) return false;
			if (!mpMod->mpInterface) return false;
			if (!mpMod->mpShapePicker) return false;
			mpMod->mpInterface->SetPickMode (mpMod->mpShapePicker);
			break;
		}
		break;

	default:
		return false;
	}

	return true;
}

//--- EditPolyMod methods -------------------------------

EditPolyMod::EditPolyMod() : mSelLevel(kSelLevObject), mpParams(NULL), mpRefNode(NULL) {
	emodDesc.MakeAutoParamBlocks(this);
	if (mOpList.Count () == 0) InitializeOperationList ();
}

void EditPolyMod::BeginEditParams (IObjParam  *ip, ULONG flags,Animatable *prev) {
	mpInterface = ip;
	mpCurrentEditMod = this;

	// Set all our dialog procs to ourselves as modifier:
	theEditPolyTypeProc.SetEditPolyMod (this);
	theEditPolySelectProc.SetEditPolyMod (this);
	theEditPolyOperationProc.SetEditPolyMod (this);

	// Put up our dialogs:
	mpDialogType = CreateCPParamMap2 (kEditPolyChoice, mpParams, mpInterface, hInstance,
		MAKEINTRESOURCE (IDD_EP_CHOICE), GetString (IDS_EP_EDIT_TYPE), 0, &theEditPolyTypeProc);
	mpDialogSelect = CreateCPParamMap2 (kEditPolySelect, mpParams, mpInterface, hInstance,
		MAKEINTRESOURCE (IDD_EP_SELECT), GetString (IDS_EP_SELECTION), 0, &theEditPolySelectProc);

	mpSelectMode = new SelectModBoxCMode(this,ip);

	// Restore the selection level.
	ip->SetSubObjectLevel(mSelLevel);

	// Update the UI to match the selection level
	SetEnableStates ();
	UpdateSelLevelDisplay ();
	SetNumSelLabel();

	// Put up our operation's dialog, if we've got one:
	UpdateOperationDialog ();

	if (!mpShapePicker) mpShapePicker = new ShapePickMode;
	if (mpShapePicker) mpShapePicker->SetMod (this, ip);

	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);
}

void EditPolyMod::EndEditParams (IObjParam *ip,ULONG flags,Animatable *next) {
	if (mpDialogOperation) {
		DestroyCPParamMap2 (mpDialogOperation);
		mpDialogOperation = NULL;
	}
	if (mpDialogSelect) {
		DestroyCPParamMap2 (mpDialogSelect);
		mpDialogSelect = NULL;
	}
	if (mpDialogType) {
		DestroyCPParamMap2 (mpDialogType);
		mpDialogType = NULL;
	}
	theEditPolyTypeProc.SetEditPolyMod (NULL);
	theEditPolySelectProc.SetEditPolyMod (NULL);
	theEditPolyOperationProc.SetEditPolyMod (NULL);

	ip->DeleteMode(mpSelectMode);
	if (mpSelectMode) delete mpSelectMode;
	mpSelectMode = NULL;

	if (mpShapePicker) mpShapePicker->ClearMod();

	mpInterface = NULL;
	mpCurrentEditMod = NULL;

	TimeValue t = ip->GetTime();
	// NOTE: This flag must be cleared before sending the REFMSG_END_EDIT
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);
}

RefTargetHandle EditPolyMod::Clone(RemapDir& remap) {
	EditPolyMod *mod = new EditPolyMod();
	mod->mSelLevel = mSelLevel;
	mod->ReplaceReference(EDIT_PBLOCK_REF,mpParams->Clone(remap));
	// (don't clone mpRefNode reference.)
	BaseClone(this, mod, remap);
	return mod;
}

void EditPolyMod::ModifyObject (TimeValue t, ModContext &mc, ObjectState *os, INode *node) {
	if (!os->obj->IsSubClassOf (polyObjectClassID)) return;
	PolyObject *pObj = (PolyObject *) os->obj;

	MNMesh &mesh = pObj->GetMesh();
	if (!pObj->GetMesh().GetFlag (MN_MESH_FILLED_IN)) pObj->GetMesh().FillInMesh();
	mesh.ClearSpecifiedNormals();	// We always lose specified normals in this mod.
	mesh.selLevel = meshSelLevel[mSelLevel];
	mesh.dispFlags = levelDispFlags[mSelLevel];

	Interval ourValidity(FOREVER);

	// Make sure we have a valid localModData, with the cached mesh set to this modifier's _input_.
	// (That's important for acquiring the pipeline selection.)
	EditPolyData *pData  = (EditPolyData *) mc.localData;
	if (!pData) mc.localData = pData = new EditPolyData (pObj->GetMesh());
	else {
		pData->ApplyAllOperations (pObj->GetMesh());
		pData->SetCache (pObj->GetMesh());
	}

	// Set the selection if necessary:
	int explicitSelection;
	mpParams->GetValue (kEPExplicitSelection, t, explicitSelection, ourValidity);
	if (explicitSelection) {
		switch (meshSelLevel[mSelLevel]) {
		case MNM_SL_VERTEX:
			pObj->GetMesh().VertexSelect (pData->GetVertSel());
			break;
		case MNM_SL_EDGE:
			pObj->GetMesh().EdgeSelect (pData->GetEdgeSel());
			break;
		case MNM_SL_FACE:
			pObj->GetMesh().FaceSelect (pData->GetFaceSel());
			break;
		}
	}

	PolyOperation *chosenOp = GetPolyOperation(t, ourValidity);

	if (pData->GetCommit()) {
		if ((chosenOp != NULL) && (chosenOp->OpID () != kOpNull)) {
			// We need an instance of our op for each local mod data:
			ConvertPolySelection (mesh, chosenOp->MeshSelectionLevel ());
			PolyOperation *pOp = chosenOp->Clone ();
			pOp->GetValues (mpParams, t, FOREVER);	// Don't affect validity, since we're "freezing" these parameters.
			pOp->RecordSelection (mesh);
			pOp->Do (mesh);

			if (theHold.Holding ())
				theHold.Put (new AddOperationRestoreObj (pData, pOp));
			pData->PushOperation (pOp);
		}
		pData->SetCommit (false);
	} else {
		if (chosenOp != NULL) {
			// Set the MN_USER flags on the right SO level, based on our current selection:
			ConvertPolySelection (mesh, chosenOp->MeshSelectionLevel ());
			chosenOp->GetValues (mpParams, t, ourValidity);
			chosenOp->Do (mesh);
		}
	}

	pObj->UpdateValidity (GEOM_CHAN_NUM, ourValidity);
	pObj->UpdateValidity (TOPO_CHAN_NUM, ourValidity);
	pObj->UpdateValidity (VERT_COLOR_CHAN_NUM, ourValidity);
	pObj->UpdateValidity (TEXMAP_CHAN_NUM, ourValidity);
	pObj->UpdateValidity (SELECT_CHAN_NUM, ourValidity);
}

Interval EditPolyMod::LocalValidity(TimeValue t) {
	if (TestAFlag(A_MOD_BEING_EDITED)) return NEVER;
	return GetValidity (t);
}

Interval EditPolyMod::GetValidity (TimeValue t) {
	// We can't just use mpParams->GetValidity,
	// because the parameters we're not currently using
	// shouldn't influence the validity interval.
	Interval ret = FOREVER;
	PolyOperation *pOp = GetPolyOperation (t, ret);
	if (pOp != NULL) pOp->GetValues (mpParams, t, ret);
	return ret;
}

int EditPolyMod::HitTest (TimeValue t, INode* inode, int type, int crossing, int flags,
						  IPoint2 *p, ViewExp *vpt, ModContext* mc) {
	if (!mSelLevel) return 0;

	Interval valid;
	int savedLimits, res = 0;
	GraphicsWindow *gw = vpt->getGW();
	HitRegion hr;
	
	int selByVert, ignoreBackfacing;
	mpParams->GetValue (kEPSelByVertex, t, selByVert, FOREVER);
	mpParams->GetValue (kEPIgnoreBackfacing, t, ignoreBackfacing, FOREVER);

	// Setup GW
	MakeHitRegion(hr,type, crossing,4,p);
	gw->setHitRegion(&hr);
	Matrix3 mat = inode->GetObjectTM(t);
	gw->setTransform(mat);	
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
	if (ignoreBackfacing) gw->setRndLimits (gw->getRndLimits() | GW_BACKCULL);
	else gw->setRndLimits (gw->getRndLimits() & ~GW_BACKCULL);
	gw->clearHitCode();

	SubObjHitList hitList;
	MeshSubHitRec *rec;	

	if (!mc->localData) return 0;
	EditPolyData *pData = (EditPolyData *) mc->localData;
	MNMesh *pMesh = pData->GetMesh();

	if ((mSelLevel > kSelLevVertex) && selByVert) {
		res = pMesh->SubObjectHitTest(gw, gw->getMaterial(), &hr,
			flags|hitLevel[kSelLevVertex]|SUBHIT_MNUSECURRENTSEL, hitList);
	} else {
		res = pMesh->SubObjectHitTest(gw, gw->getMaterial(), &hr,
			flags|hitLevel[mSelLevel], hitList);
	}

	rec = hitList.First();
	while (rec) {
		vpt->LogHit(inode,mc,rec->dist,rec->index,NULL);
		rec = rec->Next();
	}

	gw->setRndLimits(savedLimits);	
	return res;	
}

int EditPolyMod::Display (TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc) {
	if (!mpInterface->GetShowEndResult ()) return 0;
	if (!mSelLevel) return 0;
	if (!mc->localData) return 0;

	EditPolyData *pData = (EditPolyData *) mc->localData;
	MNMesh *pMesh = pData->GetMesh();
	if (!pMesh) return 0;

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

	int explicitSelection;
	mpParams->GetValue (kEPExplicitSelection, t, explicitSelection, FOREVER);

	Point3 rp[3];
	int i;
	int es[3];
	for (i=0; i<pMesh->nume; i++) {
		if (pMesh->e[i].GetFlag (MN_DEAD|MN_HIDDEN)) continue;
		if (!pMesh->e[i].GetFlag (MN_EDGE_INVIS)) {
			es[0] = GW_EDGE_VIS;
		} else {
			if (mSelLevel < kSelLevEdge) continue;
			if (mSelLevel > kSelLevFace) continue;
			es[0] = GW_EDGE_INVIS;
		}
		bool displayEdgeSel = false;
		switch (meshSelLevel[mSelLevel]) {
		case MNM_SL_EDGE:
			displayEdgeSel = IsEdgeSelected (pData, i);
			break;
		case MNM_SL_FACE:
			displayEdgeSel = IsFaceSelected (pData, pMesh->e[i].f1);
			if (pMesh->e[i].f2>-1) displayEdgeSel |= IsFaceSelected (pData, pMesh->e[i].f2);
			break;
		}
		if (displayEdgeSel) gw->setColor (LINE_COLOR, colGizSel);
		else gw->setColor (LINE_COLOR, colGiz);
		rp[0] = pMesh->v[pMesh->e[i].v1].p;
		rp[1] = pMesh->v[pMesh->e[i].v2].p;
		gw->polyline (2, rp, NULL, NULL, FALSE, es);
	}
	if (mSelLevel == kSelLevVertex) {
		for (i=0; i<pMesh->numv; i++) {
			if (pMesh->v[i].GetFlag (MN_DEAD|MN_HIDDEN)) continue;

			if (IsVertexSelected (pData, i)) gw->setColor (LINE_COLOR, colSel);
			else gw->setColor (LINE_COLOR, colTicks);

			if(getUseVertexDots()) gw->marker (&(pMesh->v[i].p), VERTEX_DOT_MARKER(getVertexDotType()));
			else gw->marker (&(pMesh->v[i].p), PLUS_SIGN_MRKR);
		}
	}
	gw->setRndLimits(savedLimits);
	return 0;	
}

void EditPolyMod::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp *vpt, Box3& box, ModContext *mc) {
	if (!mpInterface) return;
	if (!mpInterface->GetShowEndResult() || !mc->localData) return;
	if (!mSelLevel) return;
	EditPolyData *pData = (EditPolyData *) mc->localData;
	MNMesh *pMesh = pData->GetMesh();
	if (!pMesh) return;
	Matrix3 tm = inode->GetObjectTM(t);
	box = pMesh->getBoundingBox (&tm);
}

void EditPolyMod::GetSubObjectCenters (SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc) {
	if (!mc->localData) return;
	if (mSelLevel == kSelLevObject) return;	// shouldn't happen.
	EditPolyData *pData = (EditPolyData *) mc->localData;
	MNMesh *pMesh = pData->GetMesh();
	if (!pMesh) return;
	Matrix3 tm = node->GetObjectTM(t);

	// For Mesh Select, we merely return the center of the bounding box of the current selection.
	BitArray vTempSel;
	GetVertexTempSelection (pData, vTempSel);
	Box3 box;
	for (int i=0; i<pMesh->numv; i++) {
		if (vTempSel[i]) box += pMesh->v[i].p * tm;
	}
	cb->Center (box.Center(),0);
}

void EditPolyMod::GetSubObjectTMs (SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc) {
	if (!mc->localData) return;
	if (mSelLevel == kSelLevObject) return;	// shouldn't happen.
	EditPolyData *pData = (EditPolyData *) mc->localData;
	MNMesh *pMesh = pData->GetMesh();
	if (!pMesh) return;
	Matrix3 tm = node->GetObjectTM(t);

	// For our selection purposes, we merely return the center of the bounding box of the current selection.
	BitArray vTempSel;
	GetVertexTempSelection (pData, vTempSel);
	Box3 box;
	for (int i=0; i<pMesh->numv; i++) {
		if (vTempSel[i]) box += pMesh->v[i].p * tm;
	}
	Matrix3 ctm(1);
	ctm.SetTrans (box.Center());
	cb->TM (ctm,0);
}

RefTargetHandle EditPolyMod::GetReference(int i) {
	switch (i) {
	case EDIT_PBLOCK_REF: return mpParams;
	case EDIT_NODE_REF: return mpRefNode;
	default: return NULL;
	}
}

void EditPolyMod::SetReference(int i, RefTargetHandle rtarg) {
	switch (i) {
	case EDIT_PBLOCK_REF: mpParams = (IParamBlock2*)rtarg; break;
	case EDIT_NODE_REF: mpRefNode = (INode *)rtarg; break;
	}
}

RefResult EditPolyMod::NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message) {
	if (message == REFMSG_CHANGE) {
		if (hTarget == mpParams) {
			// if this was caused by a NotifyDependents from pblock, LastNotifyParamID()
			// will contain ID to update, else it will be -1 => inval whole rollout
			int pid = mpParams->LastNotifyParamID();
			switch (pid) {
			case kEPExplicitSelection:
				SetEnableStates ();
				break;
			case kEditPolyType:
				UpdateOperationDialog ();
				UpdateOpChoice ();
				break;
			case kEPExtrudeSpline:
				ClearSpline ();
				UpdateOperationDialogParams ();
				break;
			}
		}
		else if (hTarget == mpRefNode) {
			// Do something
		}
	}
	return REF_SUCCEED;
}

// Subobject API
int EditPolyMod::NumSubObjTypes() {
	return 5;
}

ISubObjType *EditPolyMod::GetSubObjType(int i) {
	static bool initialized = false;
	if(!initialized) {
		initialized = true;
		SOT_Vertex.SetName(GetString(IDS_EP_VERTEX));
		SOT_Edge.SetName(GetString(IDS_EP_EDGE));
		SOT_Border.SetName(GetString(IDS_EP_BORDER));
		SOT_Face.SetName(GetString(IDS_EP_FACE));
		SOT_Element.SetName(GetString(IDS_EP_ELEMENT));
	}

	switch(i) {
	case -1:
		if(mSelLevel > 0) 
			return GetSubObjType(mSelLevel-1);
		break;
	case 0:
		return &SOT_Vertex;
	case 1:
		return &SOT_Edge;
	case 2:
		return &SOT_Border;
	case 3:
		return &SOT_Face;
	case 4:
		return &SOT_Element;
	}
	return NULL;
}

void EditPolyMod::ActivateSubobjSel(int level, XFormModes& modes) {
	mSelLevel = level;

	// Fill in modes with our sub-object modes
	if (level!=kSelLevObject) modes = XFormModes(NULL,NULL,NULL,NULL,NULL, mpSelectMode);

	// Update UI
	UpdateSelLevelDisplay ();
	SetEnableStates ();
	SetNumSelLabel ();

	NotifyDependents(FOREVER, PART_SUBSEL_TYPE|PART_DISPLAY, REFMSG_CHANGE);
	if (mpInterface) mpInterface->PipeSelLevelChanged();
	NotifyDependents(FOREVER, SELECT_CHANNEL|DISP_ATTRIB_CHANNEL|SUBSEL_TYPE_CHANNEL, REFMSG_CHANGE);
}

void EditPolyMod::SelectSubComponent (HitRecord *pFirstHit, BOOL selected, BOOL all, BOOL invert) {
	EditPolyData *pData = NULL;
	HitRecord *pHitRec;

	int explicitSelection;
	mpParams->GetValue (kEPExplicitSelection, TimeValue(0), explicitSelection, FOREVER);
	if (!explicitSelection) mpParams->SetValue (kEPExplicitSelection, TimeValue(0), true);

	BitArray nsel;
	int selByVert;
	mpParams->GetValue (kEPSelByVertex, TimeValue(0), selByVert, FOREVER);

	BitArray set;
	switch (mSelLevel) {
	case kSelLevVertex:
		for (pHitRec=pFirstHit; pHitRec!=NULL; pHitRec = pHitRec->Next()) {
			pData = (EditPolyData*) pHitRec->modContext->localData;
			if (!pData) continue;
			if (!pData->GetNewSelection()) pData->SetupNewSelection(MNM_SL_VERTEX);
			if (!pData->GetNewSelection()) {
				DbgAssert(0);
				continue;
			}
			pData->GetNewSelection()->Set (pHitRec->hitInfo,
				invert ? !pData->mVertSel[pHitRec->hitInfo] : selected);
			if (!all) break;
		}
		break;

	case kSelLevEdge:
	case kSelLevBorder:
		for (pHitRec=pFirstHit; pHitRec!=NULL; pHitRec = pHitRec->Next()) {
			pData = (EditPolyData*) pHitRec->modContext->localData;
			if (!pData) continue;
			if (!pData->GetNewSelection()) pData->SetupNewSelection(MNM_SL_EDGE);
			if (!pData->GetNewSelection()) {
				DbgAssert(0);
				continue;
			}

			if (selByVert) {
				Tab<int> ve = pData->GetMesh()->vedg[pHitRec->hitInfo];
				for (int i=0; i<ve.Count(); i++) {
					pData->GetNewSelection()->Set (ve[i], invert ? !pData->mEdgeSel[ve[i]] : selected);
				}
			} else {
				pData->GetNewSelection()->Set (pHitRec->hitInfo, invert ? !pData->mEdgeSel[pHitRec->hitInfo] : selected);
			}
			if (!all) break;
		}
		break;

	case kSelLevFace:
	case kSelLevElement:
		for (pHitRec=pFirstHit; pHitRec!=NULL; pHitRec = pHitRec->Next()) {
			pData = (EditPolyData*) pHitRec->modContext->localData;
			if (!pData) continue;
			if (!pData->GetNewSelection()) pData->SetupNewSelection(MNM_SL_FACE);
			if (!pData->GetNewSelection()) {
				DbgAssert(0);
				continue;
			}

			if (selByVert) {
				Tab<int> vf = pData->GetMesh()->vfac[pHitRec->hitInfo];
				for (int i=0; i<vf.Count(); i++) {
					pData->GetNewSelection()->Set (vf[i], invert ? !pData->mFaceSel[vf[i]] : selected);
				}
			} else {
				pData->GetNewSelection()->Set (pHitRec->hitInfo, invert ? !pData->mFaceSel[pHitRec->hitInfo] : selected);
			}
			if (!all) break;
		}
		break;
	}

	// Apply the "new selections" to all the local mod datas:
	// (Must be after assembling all hits so we don't over-invert normals.)
	bool changeOccurred = false;
	for (pHitRec=pFirstHit; pHitRec!=NULL; pHitRec = pHitRec->Next()) {
		pData = (EditPolyData *) pHitRec->modContext->localData;
		if (!pData->GetNewSelection()) continue;
		if (pData->ApplyNewSelection(this, true, invert?true:false, selected?true:false)) {
			changeOccurred = true;
		}
		if (!all) break;
	}

	if (changeOccurred) LocalDataChanged ();
}

void EditPolyMod::ClearSelection(int selLevel) {
	if (mSelLevel == kSelLevObject) return;	// shouldn't happen
	if (!mpInterface) return;

	int explicitSelection;
	mpParams->GetValue (kEPExplicitSelection, TimeValue(0), explicitSelection, FOREVER);
	if (!explicitSelection) mpParams->SetValue (kEPExplicitSelection, TimeValue(0), true);

	ModContextList list;
	INodeTab nodes;	
	mpInterface->GetModContexts(list,nodes);

	bool changeOccurred = false;
	for (int i=0; i<list.Count(); i++) {
		EditPolyData *pData = (EditPolyData*)list[i]->localData;
		if (!pData) continue;

		pData->SetupNewSelection (meshSelLevel[mSelLevel]);
		pData->GetNewSelection()->ClearAll ();
		if (pData->ApplyNewSelection(this)) changeOccurred = true;
	}
	nodes.DisposeTemporary();
	if (changeOccurred) LocalDataChanged ();
}

void EditPolyMod::SelectAll(int selLevel) {
	if (!mpInterface) return;

	int explicitSelection;
	mpParams->GetValue (kEPExplicitSelection, TimeValue(0), explicitSelection, FOREVER);
	if (!explicitSelection) mpParams->SetValue (kEPExplicitSelection, TimeValue(0), true);

	ModContextList list;
	INodeTab nodes;	
	mpInterface->GetModContexts(list,nodes);

	bool changeOccurred = false;
	for (int i=0; i<list.Count(); i++) {
		EditPolyData *pData = (EditPolyData*)list[i]->localData;
		if (!pData) continue;

		pData->SetupNewSelection (meshSelLevel[mSelLevel]);
		pData->GetNewSelection()->SetAll ();
		if (pData->ApplyNewSelection(this)) changeOccurred = true;
	}
	nodes.DisposeTemporary();
	if (changeOccurred) LocalDataChanged ();
}

void EditPolyMod::InvertSelection(int selLevel) {
	if (!mpInterface) return;

	int explicitSelection;
	mpParams->GetValue (kEPExplicitSelection, TimeValue(0), explicitSelection, FOREVER);
	if (!explicitSelection) mpParams->SetValue (kEPExplicitSelection, TimeValue(0), true);

	ModContextList list;
	INodeTab nodes;	
	mpInterface->GetModContexts(list,nodes);

	bool changeOccurred = false;
	for (int i=0; i<list.Count(); i++) {
		EditPolyData *pData = (EditPolyData*)list[i]->localData;
		if (!pData) continue;

		pData->SetupNewSelection (meshSelLevel[mSelLevel]);
		pData->GetNewSelection()->SetAll ();
		if (pData->ApplyNewSelection(this, true, true)) changeOccurred = true;
	}
	nodes.DisposeTemporary();
	if (changeOccurred) LocalDataChanged ();
}

// From IMeshSelect
DWORD EditPolyMod::GetSelLevel() {
	switch (mSelLevel) {
	case kSelLevObject: return IMESHSEL_OBJECT;
	case kSelLevVertex: return IMESHSEL_VERTEX;
	case kSelLevEdge:
	case kSelLevBorder:
		return IMESHSEL_EDGE;
	}
	return IMESHSEL_FACE;
}

void EditPolyMod::SetSelLevel(DWORD level) {
	// This line protects against changing from border to edge, for instance, when told to switch to edge:
	if (GetSelLevel() == level) return;

	switch (level) {
	case IMESHSEL_OBJECT:
		mSelLevel = kSelLevObject;
		break;
	case IMESHSEL_VERTEX:
		mSelLevel = kSelLevVertex;
		break;
	case IMESHSEL_EDGE:
		mSelLevel = kSelLevEdge;
		break;
	case IMESHSEL_FACE:
		mSelLevel = kSelLevFace;
		break;
	}
	if (mpInterface) mpInterface->SetSubObjectLevel(mSelLevel);
}

void EditPolyMod::LocalDataChanged() {
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	if (mpInterface) SetNumSelLabel();
}

void EditPolyMod::UpdateSelLevelDisplay () {
	if (mpCurrentEditMod != this) return;
	if (!mpDialogSelect) return;
	HWND hWnd = mpDialogSelect->GetHWnd();
	if (!hWnd) return;

	theEditPolySelectProc.UpdateSelLevelDisplay (hWnd);
}

void EditPolyMod::SetEnableStates() {
	if (mpCurrentEditMod != this) return;

	if (!mpDialogSelect) return;
	HWND hWnd = mpDialogSelect->GetHWnd();
	if (!hWnd) return;

	theEditPolySelectProc.SetEnables (hWnd);
}

void EditPolyMod::SetNumSelLabel () {
	if (mpCurrentEditMod != this) return;
	TSTR buf;
	int num = 0, which;
	if (!mpInterface) return;

	if (!mpDialogSelect) return;
	HWND hWnd = mpDialogSelect->GetHWnd();
	if (!hWnd) return;

	ModContextList mcList;
	INodeTab nodes;
	mpInterface->GetModContexts(mcList,nodes);

	for (int i = 0; i < mcList.Count(); i++) {
		EditPolyData *pData = (EditPolyData *)mcList[i]->localData;
		if (!pData) continue;

		switch (mSelLevel) {
		case kSelLevVertex:
			num += pData->GetVertSel().NumberSet();
			if (pData->mVertSel.NumberSet() == 1) {
				for (which=0; which<pData->mVertSel.GetSize(); which++) if (pData->mVertSel[which]) break;
			}
			break;
		case kSelLevEdge:
		case kSelLevBorder:
			num += pData->GetEdgeSel().NumberSet();
			if (pData->GetEdgeSel().NumberSet() == 1) {
				for (which=0; which<pData->GetEdgeSel().GetSize(); which++) if (pData->GetEdgeSel()[which]) break;
			}
			break;
		case kSelLevFace:
		case kSelLevElement:
			num += pData->GetFaceSel().NumberSet();
			if (pData->GetFaceSel().NumberSet() == 1) {
				for (which=0; which<pData->GetFaceSel().GetSize(); which++) if (pData->GetFaceSel()[which]) break;
			}
			break;
		}
	}

	switch (mSelLevel) {
	case kSelLevVertex:
		if (num==1) buf.printf (GetString(IDS_EP_WHICHVERTSEL), which+1);
		else buf.printf(GetString(IDS_EP_NUMVERTSEL),num);
		break;

	case kSelLevEdge:
	case kSelLevBorder:
		if (num==1) buf.printf (GetString(IDS_EP_WHICHEDGESEL), which+1);
		else buf.printf(GetString(IDS_EP_NUMEDGESEL),num);
		break;

	case kSelLevFace:
	case kSelLevElement:
		if (num==1) buf.printf (GetString(IDS_EP_WHICHFACESEL), which+1);
		else buf.printf(GetString(IDS_EP_NUMFACESEL),num);
		break;

	case kSelLevObject:
		buf = GetString (IDS_EP_OBJECTSEL);
		break;
	}

	SetDlgItemText(hWnd,IDC_MS_NUMBER_SEL,buf);
}

void EditPolyMod::UpdateCache(TimeValue t) {
	NotifyDependents(Interval(t,t), PART_GEOM|SELECT_CHANNEL|PART_SUBSEL_TYPE|
		PART_DISPLAY|PART_TOPO, REFMSG_MOD_EVAL);
	mUpdateCachePosted = FALSE;
}

void EditPolyMod::AcquirePipeSelection () {
	if (mSelLevel == kSelLevObject) return;	// shouldn't happen
	if (!mpInterface) return;

	theHold.Begin ();

	int explicitSelection;
	mpParams->GetValue (kEPExplicitSelection, TimeValue(0), explicitSelection, FOREVER);
	if (!explicitSelection) mpParams->SetValue (kEPExplicitSelection, TimeValue(0), true);

	ModContextList list;
	INodeTab nodes;	
	mpInterface->GetModContexts(list,nodes);

	bool changeOccurred = false;
	for (int i=0; i<list.Count(); i++) {
		EditPolyData *pData = (EditPolyData*)list[i]->localData;
		if (!pData) continue;

		int msl = meshSelLevel[mSelLevel];
		pData->SetupNewSelection (msl);
		switch (msl) {
		case MNM_SL_VERTEX:
			pData->GetMesh()->getVertexSel (*(pData->GetNewSelection()));
			break;
		case MNM_SL_EDGE:
			pData->GetMesh()->getEdgeSel (*(pData->GetNewSelection()));
			break;
		case MNM_SL_FACE:
			pData->GetMesh()->getFaceSel (*(pData->GetNewSelection()));
			break;
		}
		if (pData->ApplyNewSelection(this)) changeOccurred = true;
	}
	nodes.DisposeTemporary();

	if (changeOccurred) {
		theHold.Accept (GetString (IDS_EP_GET_PIPE_SELECTION));
		LocalDataChanged ();
	} else {
		theHold.Cancel ();
	}
}

bool EditPolyMod::IsVertexSelected (EditPolyData *pData, int vertexID) {
	int explicitSelection;
	mpParams->GetValue (kEPExplicitSelection, 0, explicitSelection, FOREVER);
	if (explicitSelection) {
		return pData->GetVertSel()[vertexID] ? true : false;
	} else {
		return pData->GetMesh()->v[vertexID].GetFlag (MN_SEL);
	}
}

bool EditPolyMod::IsEdgeSelected (EditPolyData *pData, int edgeID) {
	int explicitSelection;
	mpParams->GetValue (kEPExplicitSelection, 0, explicitSelection, FOREVER);
	if (explicitSelection) {
		return pData->GetEdgeSel()[edgeID] ? true : false;
	} else {
		return pData->GetMesh()->e[edgeID].GetFlag (MN_SEL);
	}
}

bool EditPolyMod::IsFaceSelected (EditPolyData *pData, int faceID) {
	int explicitSelection;
	mpParams->GetValue (kEPExplicitSelection, 0, explicitSelection, FOREVER);
	if (explicitSelection) {
		return pData->GetFaceSel()[faceID] ? true : false;
	} else {
		return pData->GetMesh()->f[faceID].GetFlag (MN_SEL);
	}
}

void EditPolyMod::GetVertexTempSelection (EditPolyData *pData, BitArray & vertexTempSel) {
	int explicitSelection;
	mpParams->GetValue (kEPExplicitSelection, 0, explicitSelection, FOREVER);
	if (explicitSelection) {
		vertexTempSel.SetSize (pData->GetMesh()->numv);
		int i;
		MNMesh *pMesh;
		switch (meshSelLevel[mSelLevel]) {
		case MNM_SL_OBJECT:
			vertexTempSel.SetAll();
			break;
		case MNM_SL_VERTEX:
			vertexTempSel = pData->mVertSel;
			break;
		case MNM_SL_EDGE:
			vertexTempSel.ClearAll();
			pMesh = pData->GetMesh();
			for (i=0; i<pMesh->nume; i++) {
				if (pMesh->e[i].GetFlag (MN_DEAD)) continue;
				if (!pMesh->e[i].GetFlag (MN_SEL)) continue;
				vertexTempSel.Set (pMesh->e[i][0]);
				vertexTempSel.Set (pMesh->e[i][1]);
			}
			break;
		case MNM_SL_FACE:
			vertexTempSel.ClearAll();
			pMesh = pData->GetMesh();
			for (i=0; i<pMesh->numf; i++) {
				if (pMesh->f[i].GetFlag (MN_DEAD)) continue;
				if (!pMesh->f[i].GetFlag (MN_SEL)) continue;
				for (int j=0; j<pMesh->f[i].deg; j++) {
					vertexTempSel.Set (pMesh->e[i][j]);
				}
			}
			break;
		}
	} else {
		vertexTempSel = pData->GetMesh()->VertexTempSel ();
	}
}

void EditPolyMod::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc) {
	if (!mc->localData) return;
	if (partID == PART_SELECT) return;
	((EditPolyData*)mc->localData)->FreeCache();
	if (!mpInterface || (mpCurrentEditMod != this) || mUpdateCachePosted) return;

	if (!mpDialogSelect) return;
	HWND hWnd = mpDialogSelect->GetHWnd();
	if (!hWnd) return;

	TimeValue t = mpInterface->GetTime();
	PostMessage(hWnd,WM_UPDATE_CACHE,(WPARAM)t,0);
	mUpdateCachePosted = true;
}


// IO
const kChunkModifier = 0x80;
const kChunkSelLevel = 0x100;

IOResult EditPolyMod::Save(ISave *isave) {
	IOResult res;
	ULONG nb;

	isave->BeginChunk (kChunkModifier);
	Modifier::Save(isave);
	isave->EndChunk ();

	isave->BeginChunk(kChunkSelLevel);
	res = isave->Write(&mSelLevel, sizeof(mSelLevel), &nb);
	isave->EndChunk();

	return IO_OK;
}

IOResult EditPolyMod::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
		case kChunkModifier:
			res = Modifier::Load(iload);
			break;
		case kChunkSelLevel:
			iload->Read(&mSelLevel, sizeof(mSelLevel), &nb);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

const USHORT kChunkVertexSelection = 0x200;
const USHORT kChunkEdgeSelection = 0x210;
const USHORT kChunkFaceSelection = 0x220;
const USHORT kChunkOperationID = 0x230;
const USHORT kChunkOperation = 0x234;

IOResult EditPolyMod::SaveLocalData(ISave *isave, LocalModData *ld) {
	EditPolyData *pData = (EditPolyData*)ld;

	isave->BeginChunk(kChunkVertexSelection);
	pData->GetVertSel().Save(isave);
	isave->EndChunk();

	isave->BeginChunk(kChunkEdgeSelection);
	pData->GetEdgeSel().Save(isave);
	isave->EndChunk();

	isave->BeginChunk(kChunkFaceSelection);
	pData->GetFaceSel().Save(isave);
	isave->EndChunk();

	// Save the operations - in order!  (Important for reconstructing later.)
	for (PolyOperation *pop = pData->mpOpList; pop != NULL; pop=pop->Next())
	{
		isave->BeginChunk (kChunkOperationID);
		int id = pop->OpID ();
		ULONG nb;
		isave->Write (&id, sizeof(int), &nb);
		isave->EndChunk ();

		isave->BeginChunk (kChunkOperation);
		pop->Save (isave);
		isave->EndChunk ();
	}

	return IO_OK;
}

IOResult EditPolyMod::LoadLocalData(ILoad *iload, LocalModData **pld) {
	EditPolyData *pData = new EditPolyData;
	*pld = pData;
	IOResult res;
	int id;
	ULONG nb;
	PolyOperation *currentOp=NULL, *lastOp=NULL;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
		case kChunkVertexSelection:
			res = pData->GetVertSel().Load(iload);
			break;
		case kChunkEdgeSelection:
			res = pData->GetEdgeSel().Load(iload);
			break;
		case kChunkFaceSelection:
			res = pData->GetFaceSel().Load(iload);
			break;
		case kChunkOperationID:
			res = iload->Read (&id, sizeof(int), &nb);
			currentOp = GetPolyOperationByID(id)->Clone ();
			break;
		case kChunkOperation:
			res = currentOp->Load (iload);
			if (res != IO_OK) break;
			if (lastOp == NULL) pData->mpOpList = currentOp;
			else lastOp->SetNext (currentOp);
			lastOp = currentOp;
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

void EditPolyMod::UpdateOpChoice ()
{
	if (!mpDialogType) return;
	HWND hWnd = mpDialogType->GetHWnd();
	theEditPolyTypeProc.UpdateEditType (hWnd);
}

void EditPolyMod::UpdateOperationDialog () {
	// Preserve the focus, through this rollup-rolldown:
	HWND hFocus = GetFocus ();

	if (mpDialogOperation) {
		DestroyCPParamMap2 (mpDialogOperation);
		mpDialogOperation = NULL;
	}

	TimeValue t = GetCOREInterface()->GetTime ();
	PolyOperation *pop = GetPolyOperation (t, FOREVER);
	if (pop != NULL)
	{
		int dialogID = pop->DialogID ();
		if (dialogID != 0)
			mpDialogOperation = CreateCPParamMap2 (kEditPolySettings, mpParams, mpInterface, hInstance,
				MAKEINTRESOURCE (dialogID), pop->Name (), 0, &theEditPolyOperationProc);
	}
	SetFocus (hFocus);
}

void EditPolyMod::UpdateOperationDialogParams () {
	if (!mpDialogOperation) return;
	HWND hWnd = mpDialogOperation->GetHWnd();
	theEditPolyOperationProc.UpdateUI (hWnd);
}

void EditPolyMod::CommitToOperation ()
{
	TimeValue t = GetCOREInterface()->GetTime ();
	int editType;
	mpParams->GetValue (kEditPolyType, t, editType, FOREVER);

	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts(list,nodes);

	for (int i=0; i<list.Count(); i++) {
		EditPolyData *pData = (EditPolyData*)list[i]->localData;
		if (!pData) continue;
		pData->SetCommit (true);
	}
	UpdateCache (t);

	mpParams->SetValue (kEditPolyType, t, kOpNull);

	// Also, we should clear the animations on all the operation parameters.
	// TODO: How do we do this?
}

void EditPolyMod::ClearSpline ()
{
	for (int i=0; i<mOpList.Count (); i++)
	{
		if (mOpList[i]->OpID () != kOpExtrudeAlongSpline) continue;
		PolyOpExtrudeAlongSpline *pop = (PolyOpExtrudeAlongSpline *) mOpList[i];
		pop->ClearSpline ();
	}
}

void EditPolyMod::ConvertPolySelection (MNMesh & mesh, int msl) {
	int i;
	mesh.ClearVFlags (MN_USER);
	mesh.ClearEFlags (MN_USER);
	mesh.ClearFFlags (MN_USER);
	if (mesh.selLevel == MNM_SL_OBJECT) {
		switch (msl) {
		case MNM_SL_VERTEX:
			for (i=0; i<mesh.numv; i++) mesh.v[i].SetFlag (MN_USER, !mesh.v[i].GetFlag (MN_DEAD));
			break;
		case MNM_SL_EDGE:
			for (i=0; i<mesh.nume; i++) mesh.e[i].SetFlag (MN_USER, !mesh.e[i].GetFlag (MN_DEAD));
			break;
		case MNM_SL_FACE:
			for (i=0; i<mesh.numf; i++) mesh.f[i].SetFlag (MN_USER, !mesh.f[i].GetFlag (MN_DEAD));
			break;
		}
		return;
	}

	mesh.PropegateComponentFlags (msl, MN_USER, mesh.selLevel, MN_SEL);
}

int EditPolyMod::GetPolyOperationIndex (TimeValue t)
{
	int editType;
	mpParams->GetValue (kEditPolyType, t, editType, FOREVER);
	for (int i=0; i<mOpList.Count (); i++)
		if (editType == mOpList[i]->OpID ()) return i;
	return 0;
}

PolyOperation *EditPolyMod::GetPolyOperation (TimeValue t, Interval & valid)
{
	int operation;
	mpParams->GetValue (kEditPolyType, t, operation, valid);
	return GetPolyOperationByID (operation);
}

PolyOperation *EditPolyMod::GetPolyOperationByID (int operationId)
{
	for (int i=0; i<mOpList.Count (); i++)
		if (operationId == mOpList[i]->OpID ()) return mOpList[i];
	return NULL;
}


// EditPolyData -----------------------------------------------------

LocalModData *EditPolyData::Clone() {
	EditPolyData *d = new EditPolyData;
	d->mVertSel = mVertSel;
	d->mFaceSel = mFaceSel;
	d->mEdgeSel = mEdgeSel;

	// Copy the Operation linked list:
	PolyOperation *clone=NULL;
	for (PolyOperation *pop = mpOpList; pop != NULL; pop = pop->Next())
	{
		if (clone == NULL) {
			clone = pop->Clone ();
			d->mpOpList = clone;
		} else {
			clone->SetNext (pop->Clone ());
			clone = clone->Next ();
		}
	}
	return d;
}

void EditPolyData::SynchBitArrays() {
	if (!mpMeshCopy) return;
	if (mVertSel.GetSize() != mpMeshCopy->VNum()) mVertSel.SetSize(mpMeshCopy->VNum(),TRUE);
	if (mFaceSel.GetSize() != mpMeshCopy->FNum()) mFaceSel.SetSize(mpMeshCopy->FNum(),TRUE);
	if (mEdgeSel.GetSize() != mpMeshCopy->ENum()) mEdgeSel.SetSize(mpMeshCopy->ENum(),TRUE);
}

void EditPolyData::SetCache(MNMesh &mesh) {
	if (mpMeshCopy) delete mpMeshCopy;
	mpMeshCopy = new MNMesh;
	mpMeshCopy->CopyBasics (mesh, true);
	SynchBitArrays ();
}

void EditPolyData::FreeCache() {
	if (mpMeshCopy) delete mpMeshCopy;
	mpMeshCopy = NULL;
}

void EditPolyData::SetVertSel(BitArray &set, IMeshSelect *imod, TimeValue t) {
	EditPolyMod *mod = (EditPolyMod *) imod;
	if (theHold.Holding()) theHold.Put (new SelectRestore (mod, this, kSelLevVertex));
	mVertSel = set;
	if (mpMeshCopy) mpMeshCopy->VertexSelect (set);
}

void EditPolyData::SetEdgeSel(BitArray &set, IMeshSelect *imod, TimeValue t) {
	EditPolyMod *mod = (EditPolyMod *) imod;
	if (theHold.Holding()) theHold.Put (new SelectRestore (mod, this, kSelLevEdge));
	mEdgeSel = set;
	if (mpMeshCopy) mpMeshCopy->EdgeSelect (set);
}

void EditPolyData::SetFaceSel(BitArray &set, IMeshSelect *imod, TimeValue t) {
	EditPolyMod *mod = (EditPolyMod *) imod;
	if (theHold.Holding()) theHold.Put (new SelectRestore (mod, this, kSelLevFace));
	mFaceSel = set;
	if (mpMeshCopy) mpMeshCopy->FaceSelect (set);
}

// Apparently have to do something for these methods...
static GenericNamedSelSetList dummyList;
GenericNamedSelSetList & EditPolyData::GetNamedVertSelList ()
{
	return dummyList;
}
GenericNamedSelSetList & EditPolyData::GetNamedEdgeSelList ()
{
	return dummyList;
}
GenericNamedSelSetList & EditPolyData::GetNamedFaceSelList ()
{
	return dummyList;
}

void EditPolyData::SetupNewSelection (int meshSelLevel) {
	SynchBitArrays ();
	if (!mpNewSelection) mpNewSelection = new BitArray;
	switch (meshSelLevel) {
	case MNM_SL_VERTEX:
		mpNewSelection->SetSize (mVertSel.GetSize());
		break;
	case MNM_SL_EDGE:
		mpNewSelection->SetSize (mEdgeSel.GetSize());
		break;
	case MNM_SL_FACE:
		mpNewSelection->SetSize (mFaceSel.GetSize());
		break;
	default:
		delete mpNewSelection;
		mpNewSelection = NULL;
	}
}

bool EditPolyData::ApplyNewSelection (EditPolyMod *pMod, bool keepOld, bool invert, bool select) {
	if (!mpNewSelection) return false;
	if (!pMod) return false;
	if (pMod->mSelLevel == kSelLevObject) return false;

	// Do the conversion to whole-border and whole-element selections:
	if (pMod->mSelLevel == kSelLevBorder) ConvertNewSelectionToBorder ();
	if (pMod->mSelLevel == kSelLevElement) ConvertNewSelectionToElement ();

	if (keepOld) {
		// Find the current selection:
		BitArray *pCurrentSel;
		switch (pMod->mSelLevel) {
		case kSelLevVertex:
			pCurrentSel = &mVertSel;
			break;
		case kSelLevEdge:
		case kSelLevBorder:
			pCurrentSel = &mEdgeSel;
			break;
		case kSelLevFace:
		case kSelLevElement:
			pCurrentSel = &mFaceSel;
			break;
		}

		// Mix it properly with the new selection:
		if (invert) {
			// Bits in result should be set if set in exactly one of current, new selections
			(*mpNewSelection) ^= *pCurrentSel;
		} else {
			if (select) {
				// Result set if set in either of current, new:
				*mpNewSelection |= *pCurrentSel;
			} else {
				// Result set if in current, and _not_ in new:
				*mpNewSelection = ~(*mpNewSelection);
				*mpNewSelection &= *pCurrentSel;
			}
		}
	}

	SelectRestore *pSelectRestore = NULL;
	switch (pMod->mSelLevel) {
	case kSelLevObject:
		return false;
	case kSelLevVertex:
		if (*mpNewSelection == mVertSel) return false;
		if (theHold.Holding()) pSelectRestore = new SelectRestore (pMod, this, kSelLevVertex);
		mVertSel = *mpNewSelection;
		break;
	case kSelLevEdge:
	case kSelLevBorder:
		if (*mpNewSelection == mEdgeSel) return false;
		if (theHold.Holding()) pSelectRestore = new SelectRestore (pMod, this);
		mEdgeSel = *mpNewSelection;
		break;
	case kSelLevFace:
	case kSelLevElement:
		if (*mpNewSelection == mFaceSel) return false;
		if (theHold.Holding()) pSelectRestore = new SelectRestore (pMod, this);
		mFaceSel = *mpNewSelection;
		break;
	}

	if (pSelectRestore) theHold.Put (pSelectRestore);

	delete mpNewSelection;
	mpNewSelection = NULL;

	return true;
}

void EditPolyData::ConvertNewSelectionToBorder () {
	if (!mpNewSelection) return;
	if (!mpMeshCopy) return;
	if (mpNewSelection->GetSize () != mpMeshCopy->nume) {
		// Hey, these should already match!
		// Do an DbgAssert and fix the problem.
		DbgAssert (0);
		mpNewSelection->SetSize (mpMeshCopy->nume, true);
	}

	BitArray visited;
	visited.SetSize (mpMeshCopy->nume);
	visited.ClearAll ();

	Tab<int> stack;
	stack.SetCount (mpMeshCopy->nume);	// avoid reallocation!

	for (int i=0; i<mpMeshCopy->nume; i++) {
		if (mpMeshCopy->e[i].GetFlag (MN_DEAD) || (mpMeshCopy->e[i].f2>-1)) {
			// Not a border edge.
			mpNewSelection->Clear (i);
			continue;
		}
		if (visited[i]) continue;
		if (!(*mpNewSelection)[i]) continue;

		// Ok, we've got one border edge selected.
		// Ensure that the whole border is selected.
		// For efficiency, we do a stack starting with our immediate neighbors.
		stack.SetCount (1);
		stack[0] = i;
		visited.Set(i);

		int ct;
		while (ct = stack.Count()) {
			// Pop an edge off the stack:
			int edge = stack[ct-1];
			stack.Delete (ct-1, 1);

			// Check both its ends for more edges to add to the stack:
			for (int end=0; end<2; end++) {
				int vertex = mpMeshCopy->e[edge][end];
				for (int j=0; j<mpMeshCopy->vedg[vertex].Count(); j++) {
					int enext = mpMeshCopy->vedg[vertex][j];
					if (visited[enext]) continue;
					if (mpMeshCopy->e[enext].f2 > -1) continue;
					visited.Set(enext);
					mpNewSelection->Set (enext);
					stack.Append (1, &enext);
				}
			}
		}
	}
}

void EditPolyData::ConvertNewSelectionToElement () {
	if (!mpNewSelection) return;
	if (!mpMeshCopy) return;
	if (mpNewSelection->GetSize () != mpMeshCopy->numf) {
		// Hey, these should already match!
		// Do an DbgAssert and fix the problem.
		DbgAssert (0);
		mpNewSelection->SetSize (mpMeshCopy->numf, true);
	}

	BitArray visited;
	visited.SetSize (mpMeshCopy->numf);
	visited.ClearAll ();

	Tab<int> stack;
	stack.SetCount (mpMeshCopy->numf);	// avoid reallocation!

	for (int i=0; i<mpMeshCopy->numf; i++) {
		if (mpMeshCopy->f[i].GetFlag (MN_DEAD)) {
			mpNewSelection->Clear (i);
			continue;
		}

		if (visited[i]) continue;
		if (!(*mpNewSelection)[i]) continue;

		// Ok, we've got one face selected.
		// Ensure that the whole element is selected.
		// For efficiency, we do a stack starting with our immediate neighbors.
		stack.SetCount (1);
		stack[0] = i;
		visited.Set(i);

		int ct;
		while (ct = stack.Count()) {
			// Pop an edge off the stack:
			int face = stack[ct-1];
			stack.Delete (ct-1, 1);

			// Check both its ends for more edges to add to the stack:
			for (int corner=0; corner<mpMeshCopy->f[face].deg; corner++) {
				int vertex = mpMeshCopy->f[face].vtx[corner];
				for (int j=0; j<mpMeshCopy->vfac[vertex].Count(); j++) {
					int fnext = mpMeshCopy->vfac[vertex][j];
					if (visited[fnext]) continue;
					visited.Set(fnext);
					mpNewSelection->Set (fnext);
					stack.Append (1, &fnext);
				}
			}
		}
	}
}

void EditPolyData::PushOperation (PolyOperation *pOp)
{
	pOp->SetNext (NULL);
	if (mpOpList == NULL)
		mpOpList = pOp;
	else
	{
		// Otherwise, find the last item in the list, and append this one to the end.
		PolyOperation *pOpRec;
		for (pOpRec=mpOpList; pOpRec->Next () != NULL; pOpRec=pOpRec->Next ());
		pOpRec->SetNext (pOp);
	}
}

PolyOperation *EditPolyData::PopOperation ()
{
	PolyOperation *ret = NULL;
	if (mpOpList == NULL) return ret;
	if (mpOpList->Next () == NULL)
	{
		ret = mpOpList;
		mpOpList = NULL;
		return ret;
	}

	// Find the second-to-last item, and set its next to null.
	// (Do not delete - this is to be used by a restore object, so we'll retain the op there.)
	PolyOperation *pOpRec;
	for (pOpRec=mpOpList; pOpRec->Next()->Next() != NULL; pOpRec=pOpRec->Next ());
	ret = pOpRec->Next();
	pOpRec->SetNext (NULL);
	return ret;
}

void EditPolyData::DeleteAllOperations()
{
	PolyOperation *pOpRec, *pNext=NULL;
	for (pOpRec=mpOpList; pOpRec != NULL; pOpRec=pNext)
	{
		pNext = pOpRec->Next();
		pOpRec->DeleteThis ();
	}
}

void EditPolyData::ApplyAllOperations (MNMesh & mesh)
{
	PolyOperation *pOpRec;
	for (pOpRec=mpOpList; pOpRec != NULL; pOpRec=pOpRec->Next())
	{
		pOpRec->RestoreSelection (mesh);
		pOpRec->Do (mesh);
	}
}

//--- ShapePickMode ---------------------------------

BOOL ShapePickMode::HitTest(IObjParam *ip, HWND hWnd, ViewExp *vpt, IPoint2 m,int flags) {
	if (!mpMod) return false;
	return ip->PickNode(hWnd,m,this) ? TRUE : FALSE;
}

BOOL ShapePickMode::Pick(IObjParam *ip,ViewExp *vpt) {
	if (!mpMod) return false;
	INode *node = vpt->GetClosestHit();
	if (!Filter(node)) return false;
	mpMod->getParamBlock()->SetValue (kEPExtrudeSpline, ip->GetTime(), node);
	return true;
}

void ShapePickMode::EnterMode(IObjParam *ip) {
	HWND hOp = mpMod->mpDialogOperation->GetHWnd();
	ICustButton *but = NULL;
	if (hOp) but = GetICustButton (GetDlgItem (hOp, IDC_EP_EXTRUDE_PICK_SPLINE));
	if (but) {
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
		but = NULL;
	}
}

void ShapePickMode::ExitMode(IObjParam *ip) {
	HWND hOp = mpMod->mpDialogOperation->GetHWnd();
	ICustButton *but = NULL;
	if (hOp) but = GetICustButton (GetDlgItem (hOp, IDC_EP_EXTRUDE_PICK_SPLINE));
	if (but) {
		but->SetCheck(false);
		ReleaseICustButton(but);
		but = NULL;
	}
}

BOOL ShapePickMode::Filter(INode *node) {
	if (!mpMod) return false;
	if (!mpInterface) return false;
	if (!node) return false;

	// Make sure the node does not depend on us
	node->BeginDependencyTest();
	mpMod->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) return false;

	ObjectState os = node->GetObjectRef()->Eval(mpInterface->GetTime());
	if (os.obj->IsSubClassOf(splineShapeClassID)) return true;
	if (os.obj->CanConvertToType(splineShapeClassID)) return true;
	return false;
}

