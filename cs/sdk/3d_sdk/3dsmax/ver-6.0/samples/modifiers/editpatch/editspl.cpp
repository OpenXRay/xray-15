/**********************************************************************
 *<
	FILE: editspl.cpp

	DESCRIPTION:  Edit BezierShape OSM

	CREATED BY: Tom Hudson, Dan Silva & Rolf Berteig

	HISTORY: created 25 April, 1995

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "dllmain.h"
#include "editspl.h"
#include "nsclip.h"
#include "MaxIcon.h"
#include "resource.h"
#include "resourceOverride.h"

INT_PTR CALLBACK SplineSoftSelectDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );

// Uncomment this for boolean debugging
// (Leaves poly fragments without deleting unused or welding them)
//#define DEBUGBOOL 1

// Uncomment this for vert mapper debugging
//#define VMAP_DEBUG 1

// Our temporary prompts last 2 seconds:
#define PROMPT_TIME 2000

// xavier robitaille | 03.02.13 | fixed values for metric units
#ifndef METRIC_UNITS_FIXED_VALUES
#define EDITSPL_WELD_TRESH		0.1f
#else
#define EDITSPL_WELD_TRESH		DFLT_EDIT_SPLINE_MOD_WELD_TRESHOLD
#endif

// in mods.cpp
extern HINSTANCE hInstance;

// forward declarations
//
// HACK!!!!
//
// These should be member functions / member variables, but I'm putting
// this fix in after the SDK freeze for Magma.  After Magma is shipped, these
// should become members.
//
static void hackInvalidateSelUI(HWND hSelPanel);
static void hackInvalidateOpsUI(HWND hOpsPanel);
static BOOL s_bHackSelUIValid = FALSE;
static BOOL s_bHackOpsUIValid = FALSE;

// Handy temporary integer table

class TempIntTab {
	public:
		int count;
		int *tab;
		TempIntTab() { tab = NULL; count = 0; }
		TempIntTab(int ct) { tab = NULL; Alloc(ct); }
		~TempIntTab() { delete [] tab; }
		void Alloc(int ct) { if(tab) delete [] tab; tab = new int [ct]; count=ct; }
		int &operator[](int x) { assert(x>=0 && x<count); return tab[x]; }
		operator int*() { return tab; }
	};

// A handy zero point
static Point3 zeroPoint(0,0,0);

HWND				EditSplineMod::hSelectPanel    = NULL;
HWND				EditSplineMod::hSoftSelPanel   = NULL;
HWND				EditSplineMod::hOpsPanel       = NULL;
HWND				EditSplineMod::hSurfPanel      = NULL;
HWND				EditSplineMod::hSelectBy       = NULL;
BOOL				EditSplineMod::rsSel           = TRUE;
BOOL				EditSplineMod::rsSoftSel       = FALSE;
BOOL				EditSplineMod::rsOps           = TRUE;
BOOL				EditSplineMod::rsSurf          = TRUE;
IObjParam*          EditSplineMod::ip              = NULL;
EditSplineMod*      EditSplineMod::editMod         = NULL;
MoveModBoxCMode*    EditSplineMod::moveMode        = NULL;
RotateModBoxCMode*  EditSplineMod::rotMode 	       = NULL;
UScaleModBoxCMode*  EditSplineMod::uscaleMode      = NULL;
NUScaleModBoxCMode* EditSplineMod::nuscaleMode     = NULL;
SquashModBoxCMode *	EditSplineMod::squashMode      = NULL;
SelectModBoxCMode*  EditSplineMod::selectMode      = NULL;
OutlineCMode*		EditSplineMod::outlineMode     = NULL;
FilletCMode*		EditSplineMod::filletMode      = NULL;
ESChamferCMode*		EditSplineMod::chamferMode     = NULL;
SegBreakCMode*		EditSplineMod::segBreakMode    = NULL;
SegRefineCMode*		EditSplineMod::segRefineMode   = NULL;
CrossInsertCMode*	EditSplineMod::crossInsertMode = NULL;
VertConnectCMode*	EditSplineMod::vertConnectMode = NULL;
VertInsertCMode*	EditSplineMod::vertInsertMode  = NULL;
CreateLineCMode*	EditSplineMod::createLineMode  = NULL;
CrossSectionCMode*	EditSplineMod::crossSectionMode = NULL;
BooleanCMode*		EditSplineMod::booleanMode     = NULL;
TrimCMode*			EditSplineMod::trimMode        = NULL;
ExtendCMode*		EditSplineMod::extendMode      = NULL;
CopyTangentCMode*	EditSplineMod::copyTangentMode = NULL;
PasteTangentCMode*	EditSplineMod::pasteTangentMode = NULL;
BOOL               	EditSplineMod::inOutline       = FALSE;
BOOL               	EditSplineMod::inFillet        = FALSE;
BOOL               	EditSplineMod::inChamfer       = FALSE;
BOOL               	EditSplineMod::inSegBreak      = FALSE;
ISpinnerControl*	EditSplineMod::outlineSpin     = NULL;
ISpinnerControl*	EditSplineMod::filletSpin      = NULL;
ISpinnerControl*	EditSplineMod::chamferSpin     = NULL;
ISpinnerControl*	EditSplineMod::weldSpin        = NULL;
ISpinnerControl*	EditSplineMod::divSpin         = NULL;
ISpinnerControl*	EditSplineMod::crossSpin       = NULL;
ISpinnerControl*	EditSplineMod::matSpin         = NULL;
ISpinnerControl*	EditSplineMod::matSpinSel      = NULL;        
ISpinnerControl*	EditSplineMod::pEndPointAutoConnectWeldSpinner  = NULL;

// CAL-05/23/03: Threshold for extending existing splines when Shift-Move Copy. (FID #827)
ISpinnerControl*	EditSplineMod::pConnectCopyThreshSpinner = NULL;

//2-1-99 watje
ISpinnerControl*	EditSplineMod::selectAreaSpin  = NULL;

ICustButton *		EditSplineMod::iUnion          = NULL;
ICustButton *		EditSplineMod::iSubtraction    = NULL;
ICustButton *		EditSplineMod::iIntersection   = NULL;
ICustButton *		EditSplineMod::iMirrorHorizontal = NULL;
ICustButton *		EditSplineMod::iMirrorVertical   = NULL;
ICustButton *		EditSplineMod::iMirrorBoth     = NULL;
int                 EditSplineMod::boolType        = BOOL_UNION;
int                 EditSplineMod::mirrorType      = MIRROR_HORIZONTAL;
PickSplineAttach	EditSplineMod::pickCB;
BOOL				EditSplineMod::segUIValid		= TRUE;
int					EditSplineMod::condenseMat		= true;
int					EditSplineMod::attachMat		= ATTACHMAT_IDTOMAT;

// Some trim/extend stuff
#define TRIM_PROJECT_VIEW 0
#define TRIM_PROJECT_GRID 1
#define TRIM_PROJECT_3D 2

// Constants.
//
#define eNO_PTS_FOUND			1
#define eNO_EXTEND_CLOSED_SPL	2
#define eCLOSED_SPL_REQ_2_PTS	4
#define mSPLINE_DELETED			8
//#define mSHAPE_DELETED			16

static int g_trimErrorCode = 0;	// See DisplayMessage() for bit meanings.

//watje
RefineConnectCMode*		EditSplineMod::refineConnectMode   = NULL;
BindCMode*		EditSplineMod::bindMode   = NULL;

#define DEFAULT_KNOT_TYPE KTYPE_CORNER		// CAL-02/20/03: default set to 'Linear'

// Select by material parameters
static int sbmParams[2]     = {1,1};

// Checkbox items for rollup pages
static BOOL polyMirrorCopy = 0;
static BOOL mirrorAboutPivot = 0;
static BOOL trimInfinite = 0;
static int detachSameShape = 0;
static int detachCopy = 0;
static int detachReorient = 0;
static int attachReorient = 0;
static int centeredOutline = 0;
static int explodeToObjects = 0;
static int lockedHandles = 0;
static int lockType = IDC_LOCKALIKE;
static BOOL showSelected = FALSE;
static int segDivisions = 1;
static BOOL boundFirst = FALSE;
static BOOL boundLast = FALSE;
static BOOL dontAskRefineConnectOption = FALSE;
static BOOL connectOnly = FALSE;
static BOOL segmentEnd = FALSE;

// The weld threshold
static float weldThreshold = EDITSPL_WELD_TRESH;

// The boolean operation
static int boolOperation = BOOL_UNION;

// The crossing threshold
static float crossThreshold = 0.1f;

// This is a special override value which allows us to hit-test on
// any sub-part of a shape

int splineHitOverride = 0;	// If zero, no override is done

static GenSubObjType SOT_Vertex(10);
static GenSubObjType SOT_Segment(11);
static GenSubObjType SOT_Spline(12);


// Special messages for Select By floater
#define MSG_SUBOBJ_MODE_CHANGE WM_USER+1

void SetSplineHitOverride(int value) {
	splineHitOverride = value;
	}

void ClearSplineHitOverride() {
	splineHitOverride = 0;
	}

/*-------------------------------------------------------------------*/

static HIMAGELIST hSplineImages = NULL;

static void LoadESImages() {
	if (hSplineImages) return;

	HBITMAP hBitmap, hMask;
	hSplineImages = ImageList_Create(24, 23, ILC_COLOR|ILC_MASK, 6, 0);
	hBitmap     = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_SPLINESELTYPES));
	hMask       = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_SPLINESELMASK));
	ImageList_Add(hSplineImages,hBitmap,hMask);
	DeleteObject(hBitmap);
	DeleteObject(hMask);
}

/*-------------------------------------------------------------------*/

// Support for dynamic quad menus
class ESDynamicMenuCallback: public DynamicMenuCallback {
public:
    void MenuItemSelected(int itemId);
    void SetMod(EditSplineMod *es) { this->es = es; }

private:
		EditSplineMod *es;
};

void
ESDynamicMenuCallback::MenuItemSelected(int id)
{
	switch(es->GetSubobjectLevel()) {
		case ES_VERTEX:
			es->SetRememberedVertType((int)id);
			break;
		case ES_SEGMENT:
			es->SetRememberedSegType((int)id);
			break;
		case ES_SPLINE:
			es->SetRememberedPolyType((int)id);
			break;
		}
}

static ESDynamicMenuCallback sMenuCallback;

class ESActionCallback: public ActionCallback {
public:
    IMenu* GetDynamicMenu(int id, HWND hwnd, IPoint2& m);
    void SetMod(EditSplineMod *es) { this->es = es; }

private:
		EditSplineMod *es;
};

IMenu* 
ESActionCallback::GetDynamicMenu(int id, HWND hWnd, IPoint2& m)
{
    sMenuCallback.SetMod(es);
    DynamicMenu menu(&sMenuCallback);

	switch(es->GetSubobjectLevel()) {
		case ES_VERTEX:
			if(es->RememberVertThere(hWnd, m)) {
				int oldType = -1;
				int flags0, flags1, flags2, flags3, flags4;
				flags0 = flags1 = flags2 = flags3 = flags4 = 0;
				switch(es->rememberedData) {
					case KTYPE_AUTO:
						flags1 |= DynamicMenu::kChecked;
						flags0 |= DynamicMenu::kDisabled;
						break;
					case KTYPE_CORNER:
						flags2 |= DynamicMenu::kChecked;
						flags0 |= DynamicMenu::kDisabled;
						break;
					case KTYPE_BEZIER:
						flags3 |= DynamicMenu::kChecked;
						break;
					case KTYPE_BEZIER_CORNER:
						flags4 |= DynamicMenu::kChecked;
						break;
					}
				// CAL-02/19/03: add reset tangent. (FID #827)
				menu.AddItem(flags0, KTYPE_RESET, GetString(IDS_TH_RESET_TANGENTS));
				menu.AddItem(flags1, KTYPE_AUTO, GetString(IDS_TH_SMOOTH));
				menu.AddItem(flags2, KTYPE_CORNER, GetString(IDS_TH_CORNER));
				menu.AddItem(flags3, KTYPE_BEZIER, GetString(IDS_TH_BEZIER));
				menu.AddItem(flags4, KTYPE_BEZIER_CORNER, GetString(IDS_TH_BEZIERCORNER));
				menu.AddItem( DynamicMenu::kSeparator, 0, NULL);
				}
			break;
		case ES_SEGMENT:
			if(es->RememberSegThere(hWnd, m)) {
				int oldType = -1;
				int flags1, flags2;
				flags1 = flags2 = 0;
				switch(es->rememberedData) {
					case LTYPE_CURVE:
						flags1 |= DynamicMenu::kChecked;
						break;
					case LTYPE_LINE:
						flags2 |= DynamicMenu::kChecked;
						break;
					}
				menu.AddItem(flags1, LTYPE_CURVE, GetString(IDS_TH_CURVE));
				menu.AddItem(flags2, LTYPE_LINE, GetString(IDS_TH_LINE));
				menu.AddItem( DynamicMenu::kSeparator, 0, NULL);
				}
			break;
		case ES_SPLINE:
			if(es->RememberPolyThere(hWnd, m)) {
				int oldType = -1;
				int flags1, flags2;
				flags1 = flags2 = 0;
				switch(es->rememberedData) {
					case LTYPE_CURVE:
						flags1 |= DynamicMenu::kChecked;
						break;
					case LTYPE_LINE:
						flags2 |= DynamicMenu::kChecked;
						break;
					}
				menu.AddItem(flags1, LTYPE_CURVE, GetString(IDS_TH_CURVE));
				menu.AddItem(flags2, LTYPE_LINE, GetString(IDS_TH_LINE));
				menu.AddItem( DynamicMenu::kSeparator, 0, NULL);
				}
			break;
		}

    return menu.GetMenu();
}

static ESActionCallback sActionCallback;


// Right-click menu support
class ESRightMenu : public RightClickMenu {
	private:
		EditSplineMod *es;
	public:
		void Init(RightClickMenuManager* manager, HWND hWnd, IPoint2 m);
		void Selected(UINT id);
		void SetMod(EditSplineMod *es) { this->es = es; }
	};

void ESRightMenu::Init(RightClickMenuManager* manager, HWND hWnd, IPoint2 m) {
	switch(es->GetSubobjectLevel()) {
		case ES_VERTEX:
			if(es->RememberVertThere(hWnd, m)) {
				int oldType = -1;
				int flags0, flags1, flags2, flags3, flags4;
				flags0 = flags1 = flags2 = flags3 = flags4 = MF_STRING;
				switch(es->rememberedData) {
					case KTYPE_AUTO:
						flags1 |= MF_CHECKED;
						flags0 |= MF_DISABLED;
						break;
					case KTYPE_CORNER:
						flags2 |= MF_CHECKED;
						flags0 |= MF_DISABLED;
						break;
					case KTYPE_BEZIER:
						flags3 |= MF_CHECKED;
						break;
					case KTYPE_BEZIER_CORNER:
						flags4 |= MF_CHECKED;
						break;
					}
				manager->AddMenu(this, MF_SEPARATOR, 0, NULL);
				// CAL-02/19/03: add reset tangent. (FID #827)
				manager->AddMenu(this, flags0, KTYPE_RESET, GetString(IDS_TH_RESET_TANGENTS));
				manager->AddMenu(this, flags1, KTYPE_AUTO, GetString(IDS_TH_SMOOTH));
				manager->AddMenu(this, flags2, KTYPE_CORNER, GetString(IDS_TH_CORNER));
				manager->AddMenu(this, flags3, KTYPE_BEZIER, GetString(IDS_TH_BEZIER));
				manager->AddMenu(this, flags4, KTYPE_BEZIER_CORNER, GetString(IDS_TH_BEZIERCORNER));
				}
			break;
		case ES_SEGMENT:
			if(es->RememberSegThere(hWnd, m)) {
				int oldType = -1;
				int flags1, flags2;
				flags1 = flags2 = MF_STRING;
				switch(es->rememberedData) {
					case LTYPE_CURVE:
						flags1 |= MF_CHECKED;
						break;
					case LTYPE_LINE:
						flags2 |= MF_CHECKED;
						break;
					}
				manager->AddMenu(this, MF_SEPARATOR, 0, NULL);
				manager->AddMenu(this, flags1, LTYPE_CURVE, GetString(IDS_TH_CURVE));
				manager->AddMenu(this, flags2, LTYPE_LINE, GetString(IDS_TH_LINE));
				}
			break;
		case ES_SPLINE:
			if(es->RememberPolyThere(hWnd, m)) {
				int oldType = -1;
				int flags1, flags2;
				flags1 = flags2 = MF_STRING;
				switch(es->rememberedData) {
					case LTYPE_CURVE:
						flags1 |= MF_CHECKED;
						break;
					case LTYPE_LINE:
						flags2 |= MF_CHECKED;
						break;
					}
				manager->AddMenu(this, MF_SEPARATOR, 0, NULL);
				manager->AddMenu(this, flags1, LTYPE_CURVE, GetString(IDS_TH_CURVE));
				manager->AddMenu(this, flags2, LTYPE_LINE, GetString(IDS_TH_LINE));
				}
			break;
		}
	}

void ESRightMenu::Selected(UINT id) {
	switch(es->GetSubobjectLevel()) {
		case ES_VERTEX:
			es->SetRememberedVertType((int)id);
			break;
		case ES_SEGMENT:
			es->SetRememberedSegType((int)id);
			break;
		case ES_SPLINE:
			es->SetRememberedPolyType((int)id);
			break;
		}
	}

ESRightMenu esMenu;

// Delete interface support

class ESDeleteUser : public EventUser {
	private:
		EditSplineMod *es;
	public:
		void Notify();
		void SetMod(EditSplineMod *es) { this->es = es; }
	};

void ESDeleteUser::Notify() {
	switch(es->GetSubobjectLevel()) {
		case ES_VERTEX:
			es->DoVertDelete();
			break;
		case ES_SEGMENT:
			es->DoSegDelete();
			break;
		case ES_SPLINE:
			es->DoPolyDelete();
			break;
		}
	}

ESDeleteUser esDel;

/*-------------------------------------------------------------------*/

// This function checks the current command mode and resets it to CID_OBJMOVE if
// it's one of our command modes

static
void CancelEditSplineModes(IObjParam *ip) {
	switch(ip->GetCommandMode()->ID()) {
		case CID_OUTLINE:
		case CID_SEGBREAK:
		case CID_SEGREFINE:
		case CID_VERTCONNECT:
		case CID_VERTINSERT:
		case CID_BOOLEAN:
		case CID_CREATELINE:
		case CID_CROSSINSERT:
		case CID_FILLET:
		case CID_CHAMFER:
		case CID_TRIM:
		case CID_EXTEND:
//watje			
		case CID_REFINECONNECT:
		case CID_SPLINEBIND:

		case CID_STDPICK:
			ip->SetStdCommandMode( CID_OBJMOVE );
			break;
		}
	}

// If there isn't just 1 spline selected in the shape, and the boolean mode
// is active, turn it off!
static
void MaybeCancelBooleanMode(EditSplineMod *mod) {
	if(!mod->ip)
		return;
	if(mod->ip->GetCommandMode()->ID() == CID_BOOLEAN) {
		mod->ip->SetStdCommandMode( CID_OBJMOVE );
		}
	}

/*-------------------------------------------------------------------*/

// Vertex-to-knot selection mapper
static BitArray &KnotSelFromVertSel(BitArray &vsel) {
	int knots = vsel.GetSize() / 3;
	static BitArray ksel;
	ksel.SetSize(knots);
	for(int i = 0; i < knots; ++i)
		ksel.Set(i, vsel[i*3+1]);
	return ksel;
	}

/*-------------------------------------------------------------------*/

static
BOOL IsCompatible(BezierShape *shape, int poly, BitArray *VSel, BitArray *SSel) {
	if(poly >= shape->splineCount)
		return FALSE;
	if(VSel) {
		if(shape->splines[poly]->Verts() != VSel->GetSize())
			return FALSE;
		}
	if(SSel) {
		if(shape->splines[poly]->Segments() != SSel->GetSize())
			return FALSE;
		}
	return TRUE;
	}

/*-------------------------------------------------------------------*/		

// Delete all selected segments
// Returns TRUE if the operation results in the polygon being deleted
static int DeleteSelSegs(BezierShape *shape, int poly) {
	Spline3D *spline = shape->splines[poly];
//	BitArray& vsel = shape->vertSel[poly];
//	BitArray& ssel = shape->segSel[poly];
	int segs = spline->Segments();
	int verts = spline->Verts();
	int altered = 0;
	for(int s = segs-1; s >= 0; --s) {
		BitArray& vsel = shape->vertSel[poly];
		BitArray& ssel = shape->segSel[poly];
		if(ssel[s]) {
			altered = 1;
			if(spline->Closed()) {
				if(s == (spline->KnotCount()-1)) {	// deletion at end segment -- Just open the spline!
					delete_at_last_segment:
					altered = 1;
					spline->SetOpen();
					ssel.SetSize(spline->Segments(),1);
					}
				else
				if(s == 0) {			// Delete at first segment
					SplineKnot dupKnot = spline->GetKnot(0);
					spline->AddKnot(dupKnot, -1);
					spline->DeleteKnot(0);
					spline->SetOpen();
					vsel.Rotate(LEFT_BITSHIFT, 3);
					ssel.Rotate(LEFT_BITSHIFT, 1);
					ssel.SetSize(spline->Segments(),1);
					}
				else {					// Deletion somewhere in the middle
					// First, rotate the spline so that the deletion point is the last one!
					int rotations = 0;
					int lastSeg = spline->KnotCount() - 1;
					while(s < (lastSeg)) {
						SplineKnot dupKnot = spline->GetKnot(lastSeg);
						spline->DeleteKnot(lastSeg);
						spline->AddKnot(dupKnot, 0);
						rotations++;
						s++;
						}
					vsel.Rotate(RIGHT_BITSHIFT, rotations*3);
					ssel.Rotate(RIGHT_BITSHIFT, rotations);
					s = lastSeg;
					goto delete_at_last_segment;
					}
				}
			else {
				// If it's the only segment, blow the polygon away!
				if(spline->Segments() == 1) {
					shape->DeleteSpline(poly);
//5-1-99 watje
//					shape->UpdateBindList();
					return TRUE;				// It's TRUE -- We deleted the spline!
					}
				if(s==0) {
					spline->DeleteKnot(0);
					vsel.Shift(LEFT_BITSHIFT, 3);
					vsel.SetSize(spline->Verts(),1);
					ssel.Shift(LEFT_BITSHIFT, 1);
					ssel.SetSize(spline->Segments(),1);
					}
				else
				if(s == (spline->KnotCount()-2)) {
					spline->DeleteKnot(s+1);
					vsel.SetSize(spline->Verts(),1);
					ssel.SetSize(spline->Segments(),1);
					}
				else {
					int i;
					int newPolySize = spline->KnotCount() - s - 1;
					// OK, We're deleting at a point in the middle -- Copy end points off to a new spline, then
					// delete them from the original!
					Spline3D *newSpline = shape->NewSpline();
					int knots = spline->KnotCount();
					for(i = s + 1; i < knots; ++i) {
						SplineKnot dupKnot = spline->GetKnot(i);
						newSpline->AddKnot(dupKnot, -1);
						}
					for(i = knots-1; i > s; --i)
						spline->DeleteKnot(i);
					// Adjust selection data for this spline
					vsel.SetSize(spline->Verts(),1);
					ssel.SetSize(spline->Segments(),1);
					// Don't forget to create a new selection record for this new spline!
					shape->vertSel.Insert(shape->splineCount - 1, newPolySize * 3);
					shape->segSel.Insert(shape->splineCount - 1, newPolySize - 1);
					shape->polySel.Insert(shape->splineCount - 1);
					}
				}
			}
		}
	if(altered)	{
		spline->ComputeBezPoints();
//3-31-99 watje
//		shape->UpdateBindList();
		shape->InvalidateGeomCache();
		}
	return FALSE;	// The poly's still there
	}

/*-------------------------------------------------------------------*/		

// Segment Detach Types:
#define SDT_COPY_MASK (1<<0)
#define SDT_SAME_MASK (1<<1)

#define SDT_DETACH		0	// Detach to other shape
#define SDT_COPY		(SDT_COPY_MASK) 	// Copy to other shape
#define SDT_DETACH_SAME	(SDT_SAME_MASK)		// Detach to same shape
#define SDT_COPY_SAME	(SDT_COPY_MASK | SDT_SAME_MASK)	// Copy to same shape

// Handle the basic mechanics of detaching segments
static int HandleSegDetach(BezierShape *shape, BezierShape *toShape, int type, int poly) {
	Spline3D *spline = shape->splines[poly];
	int segments = spline->Segments();
	int segsSelected = shape->segSel[poly].NumberSet();
	// If all segments selected, copy the whole polygon!
	if(segsSelected == segments) {
		toShape->InsertSpline(spline, toShape->SplineCount());
		}
	else {
		int end = segments;
		for(int seg = 0; seg < end; ++seg) {
			if(shape->segSel[poly][seg]) {
				Spline3D newSpline;
				if(seg == 0 && spline->Closed()) {
					backLoop:
					if(shape->segSel[poly][--end]) {
						SplineKnot addKnot = spline->GetKnot(end);
						newSpline.AddKnot(addKnot, 0);
						goto backLoop;
						}
					}
				SplineKnot addKnot = spline->GetKnot(seg);
				newSpline.AddKnot(addKnot, -1);

				loop:
				seg++;
				if(spline->Closed())
					seg %= segments;
				addKnot = spline->GetKnot(seg);
				newSpline.AddKnot(addKnot, -1);
				if(seg > 0 && seg < end && shape->segSel[poly][seg])
					goto loop;

				// Finish up the spline!
				newSpline.ComputeBezPoints();
				toShape->InsertSpline(&newSpline, toShape->SplineCount());
				// If copying to same shape, select the copied spline's segments (always the last)
				if(type == SDT_COPY_SAME)
					toShape->segSel[toShape->splineCount - 1].SetAll();
//3-31-99 watje
//				shape->UpdateBindList();
				shape->InvalidateGeomCache();
				if(toShape != shape)
					toShape->InvalidateGeomCache();

				// Special termination test for wraparound
				if(seg == 0)
					seg = end;
				}
			}
		}
	// Call the segment delete function
	if(type != SDT_COPY && type != SDT_COPY_SAME)
		return DeleteSelSegs(shape, poly);	// Spline may be deleted
	else {
		// Clear the spline's segment selection set
		if(type == SDT_COPY_SAME)
			shape->segSel[poly].ClearAll();
		}

	return FALSE;	// Spline not deleted
	}

/*-------------------------------------------------------------------*/

static
TSTR detachName;

static
INT_PTR CALLBACK DetachDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	TCHAR tempName[256];
	switch(message) {
		case WM_INITDIALOG:
			SetDlgItemText(hDlg, IDC_DETACH_NAME, detachName);
			SendMessage(GetDlgItem(hDlg, IDC_DETACH_NAME), EM_SETSEL, 0, -1);
			SetFocus(GetDlgItem(hDlg, IDC_DETACH_NAME));
			return FALSE;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
					GetDlgItemText(hDlg, IDC_DETACH_NAME, tempName, 255);
					detachName = TSTR(tempName);
					EndDialog(hDlg, 1);
					return TRUE;
				case IDCANCEL:
					EndDialog(hDlg, 0);
					return TRUE;
			}
		}
	return FALSE;
	}

static
int GetDetachOptions(IObjParam *ip, TSTR& newName) {
	detachName = newName;
	ip->MakeNameUnique(detachName);	
	if(DialogBox(hInstance, MAKEINTRESOURCE(IDD_DETACH), ip->GetMAXHWnd(), (DLGPROC)DetachDialogProc)==1) {
		newName = detachName;
		return 1;
		}
	return 0;
	}

/*-------------------------------------------------------------------*/

static EditSplineClassDesc editSplineDesc;
extern ClassDesc* GetEditSplineModDesc() { return &editSplineDesc; }

void EditSplineClassDesc::ResetClassParams(BOOL fileReset)
	{
	sbmParams[0]   = 1;
	sbmParams[1]   = 1;
	polyMirrorCopy = 0;
	mirrorAboutPivot = 0;
	trimInfinite = 0;
	detachSameShape = 0;
	detachCopy = 0;
	detachReorient = 0;
	segDivisions = 1;
	attachReorient = 0;
	centeredOutline = 0;
	explodeToObjects = 0;
	lockedHandles = 0;
	showSelected = FALSE;
	lockType = IDC_LOCKALIKE;
	weldThreshold = EDITSPL_WELD_TRESH;
	crossThreshold = 0.1f;
	boolOperation = BOOL_UNION;
	EditSplineMod::condenseMat = true;
	EditSplineMod::attachMat = ATTACHMAT_IDTOMAT;
	segmentEnd = FALSE;
	}

/*-------------------------------------------------------------------*/

ShapePointTab::ShapePointTab() {
	polys = 0;
	pUsed = NULL;
	ptab = NULL;
	ktab = NULL;
	ltab = NULL;
	}

ShapePointTab::~ShapePointTab() {
	if(pUsed)
		delete [] pUsed;
	if(ptab)
		delete [] ptab;
	if(ktab)
		delete [] ktab;
	if(ltab)
		delete [] ltab;
	}

void ShapePointTab::Empty() {
	if(pUsed)
		delete [] pUsed;
	pUsed = NULL;
	if(ptab)
		delete [] ptab;
	ptab = NULL;
	if(ktab)
		delete [] ktab;
	ktab = NULL;
	if(ltab)
		delete [] ltab;
	ltab = NULL;
	polys = 0;
	}

void ShapePointTab::Zero() {
	int i;
	for(int poly = 0; poly < polys; ++poly) {
		pUsed[poly] = 0;

		Point3Tab &pt = ptab[poly];
		IntTab &kt = ktab[poly];
		IntTab &st = ltab[poly];

		int points = pt.Count();
		int knots = kt.Count();
		int segments = st.Count();
		Point3 zero(0, 0, 0);

		for(i = 0; i < points; ++i)
			pt[i] = zero;
		for(i = 0; i < knots; ++i)
			kt[i] = 0;
		for(i = 0; i < segments; ++i)
			st[i] = 0;
		}
	}

void ShapePointTab::MakeCompatible(BezierShape& shape,int clear) {
	Point3 zero(0,0,0);
	int izero = 0;
	if(polys == shape.splineCount) {
		for(int i=0; i<polys; ++i) {
			int size = shape.splines[i]->Verts();
			int knots = shape.splines[i]->KnotCount();
			Point3Tab& tab = ptab[i];
			IntTab& kttab = ktab[i];
			IntTab& lttab = ltab[i];
			if(clear) {
				pUsed[i] = 0;
				tab.Delete(0,tab.Count());
				kttab.Delete(0,kttab.Count());
				lttab.Delete(0,lttab.Count());
				}
			if ( tab.Count() > size ) {
				int diff = tab.Count() - size;
				tab.Delete( tab.Count() - diff, diff );
				diff = kttab.Count() - knots;		
				kttab.Delete( kttab.Count() - diff, diff );		
				lttab.Delete( lttab.Count() - diff, diff );		
				}
			else
			if ( tab.Count() < size ) {
				int diff = size - tab.Count();
				tab.Resize( size );
				for ( int j = 0; j < diff; j++ )
					tab.Append(1,&zero);
				diff = knots - kttab.Count();
				kttab.Resize( knots );
				lttab.Resize( knots );
				for ( j = 0; j < diff; j++ ) {
					kttab.Append(1,&izero);
					lttab.Append(1,&izero);
					}
				}
			}
		}
	else {
		if(pUsed)
			delete [] pUsed;
		if(ptab)
			delete [] ptab;
		if(ktab)
			delete [] ktab;
		if(ltab)
			delete [] ltab;
		polys = shape.splineCount;
		pUsed = new int[polys];
		ptab = new Point3Tab[polys];
		ktab = new IntTab[polys];
		ltab = new IntTab[polys];
//		closures.SetSize(polys);
		for(int i=0; i<polys; ++i) {
			pUsed[i] = 0;
			Point3Tab& tab = ptab[i];
			IntTab& kttab = ktab[i];
			IntTab& lttab = ltab[i];
			int size = shape.splines[i]->Verts();
			for ( int j = 0; j < size; j++ )
				tab.Append(1,&zero);
			int knots = shape.splines[i]->KnotCount();
			tab.Resize( size );
			kttab.Resize( knots );
			lttab.Resize( knots );
			for ( j = 0; j < knots; j++ ) {
				kttab.Append(1,&izero);
				lttab.Append(1,&izero);
				}
			}
		}
	}

ShapePointTab& ShapePointTab::operator=(ShapePointTab& from) {
	if(pUsed)
		delete [] pUsed;
	if(ptab)
		delete [] ptab;
	if(ktab)
		delete [] ktab;
	if(ltab)
		delete [] ltab;
	polys = from.polys;
	pUsed = new int[polys];
	ptab = new Point3Tab[polys];
	ktab = new IntTab[polys];
	ltab = new IntTab[polys];
	for(int poly = 0; poly < polys; ++poly) {
		pUsed[poly] = from.pUsed[poly];
		ptab[poly] = from.ptab[poly];
		ktab[poly] = from.ktab[poly];
		ltab[poly] = from.ltab[poly];
		}
//	closures = from.closures;
	return *this;
	}

BOOL ShapePointTab::IsCompatible(BezierShape &shape) {
	if(polys != shape.splineCount)
		return FALSE;
	for(int poly = 0; poly < polys; ++poly) {
		if(ptab[poly].Count() != shape.splines[poly]->Verts())
			return FALSE;
		if(ktab[poly].Count() != shape.splines[poly]->KnotCount())
			return FALSE;
		}
	return TRUE;
	}

void ShapePointTab::RescaleWorldUnits(float f) {
	Matrix3 stm = ScaleMatrix(Point3(f, f, f));
	int i;
	for(int poly = 0; poly < polys; ++poly) {
		Point3Tab &pt = ptab[poly];
		int points = pt.Count();
		for(i = 0; i < points; ++i)
			pt[i] = pt[i] * stm;
		}
	}

#define SPT_POLYDATA_CHUNK	0x1000

IOResult ShapePointTab::Save(ISave *isave) {	
	int i, n;
	ULONG nb;
	isave->BeginChunk(SPT_POLYDATA_CHUNK);
	isave->Write(&polys,sizeof(int),&nb);
	for(int poly = 0; poly < polys; ++poly) {
		n = pUsed[poly];
		isave->Write(&n,sizeof(int),&nb);
		Point3Tab &pptab = ptab[poly];
		int count = pptab.Count();
		isave->Write(&count,sizeof(int),&nb);
		for(i = 0; i < count; ++i)
			isave->Write(&pptab[i],sizeof(Point3),&nb);
		IntTab &iktab = ktab[poly];
		count = iktab.Count();
		isave->Write(&count,sizeof(int),&nb);
		for(i = 0; i < count; ++i)
			isave->Write(&iktab[i],sizeof(int),&nb);
		IntTab &iltab = ltab[poly];
		count = iltab.Count();
		isave->Write(&count,sizeof(int),&nb);
		for(i = 0; i < count; ++i)
			isave->Write(&iltab[i],sizeof(int),&nb);
		}
	isave->EndChunk();
	return IO_OK;
	}

IOResult ShapePointTab::Load(ILoad *iload) {	
	int i, n;
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case SPT_POLYDATA_CHUNK:
				res = iload->Read(&polys,sizeof(int),&nb);
				pUsed = new int[polys];
				ptab = new Point3Tab[polys];
				ktab = new IntTab[polys];
				ltab = new IntTab[polys];
				for(int poly = 0; poly < polys; ++poly) {
					iload->Read(&n,sizeof(int),&nb);
					pUsed[poly] = n;
					Point3Tab &pptab = ptab[poly];
					int count;
					iload->Read(&count,sizeof(int),&nb);
					Point3 workpt;
					for(i = 0; i < count; ++i) {
						iload->Read(&workpt,sizeof(Point3),&nb);
						pptab.Append(1,&workpt);
						}
					IntTab &iktab = ktab[poly];
					iload->Read(&count,sizeof(int),&nb);
					for(i = 0; i < count; ++i) {
						iload->Read(&n,sizeof(int),&nb);
						iktab.Append(1,&n);
						}
					IntTab &iltab = ltab[poly];
					iload->Read(&count,sizeof(int),&nb);
					for(i = 0; i < count; ++i) {
						iload->Read(&n,sizeof(int),&nb);
						iltab.Append(1,&n);
						}
					}
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/

void ShapeVertexDelta::SetSize(BezierShape& shape, BOOL load)
	{
	dtab.MakeCompatible(shape, FALSE);
	
	// Load it if necessary
	if(load) {
		for(int poly = 0; poly < shape.splineCount; ++poly) {
			Spline3D *spline = shape.splines[poly];
			int verts = spline->Verts();
			int knots = spline->KnotCount();
//			dtab.pUsed[poly] = 1;	// ???
			Point3Tab& delta = dtab.ptab[poly];
			IntTab& kdelta = dtab.ktab[poly];
			IntTab& ldelta = dtab.ltab[poly];
			for(int i = 0; i < verts; ++i)
				delta[i] = spline->GetVert(i);
			for(i = 0; i < knots; ++i) {
				kdelta[i] = spline->GetKnotType(i);
				ldelta[i] = spline->GetLineType(i);
				}
//			spline->ComputeBezPoints();
			}
//		shape.GetClosures(dtab.closures);
		}
	}

void ShapeVertexDelta::Apply(BezierShape &shape)
	{
	// This does nothing if the number of verts hasn't changed in the mesh.
	SetSize(shape, FALSE);

	// Apply the deltas
//	shape.SetClosures(dtab.closures);
	for(int poly = 0; poly < shape.splineCount; ++poly) {
//		if(dtab.pUsed[poly]) {
			Spline3D *spline = shape.splines[poly];
			int verts = spline->Verts();
			int knots = spline->KnotCount();
			Point3Tab& delta = dtab.ptab[poly];
			IntTab& kdelta = dtab.ktab[poly];
			IntTab& ldelta = dtab.ltab[poly];
			for(int i = 0; i < verts; ++i)
				spline->SetVert(i,spline->GetVert(i) + delta[i]);
			for(i = 0; i < knots; ++i) {
				spline->SetKnotType(i,spline->GetKnotType(i) ^ kdelta[i]);
				spline->SetLineType(i,spline->GetLineType(i) ^ ldelta[i]);
				}
			spline->ComputeBezPoints();
//			DebugPrint(_T("Poly %d used"),poly);
//			}
//		else
//			DebugPrint(_T("Poly %d not used"),poly);
		}
//watje
//	shape.UpdateBindList();

	shape.InvalidateGeomCache();
	}

// This function applies the current changes to slave handles and their knots, and zeroes everything else
void ShapeVertexDelta::ApplyHandlesAndZero(BezierShape &shape, int handlePoly, int handleVert) {
	// This does nothing if the number of verts hasn't changed in the mesh.
	SetSize(shape, FALSE);

	Point3 zeroPt(0.0f, 0.0f, 0.0f);

	// Apply the deltas	to just the slave handles
	for(int poly = 0; poly < shape.splineCount; ++poly) {
		if(dtab.pUsed[poly]) {
			Spline3D *spline = shape.splines[poly];
			int verts = spline->Verts();
			int knots = spline->KnotCount();
			Point3Tab& delta = dtab.ptab[poly];
			IntTab& kdelta = dtab.ktab[poly];
			IntTab& ldelta = dtab.ltab[poly];
			for(int i = 0; i < verts; ++i) {
				if(delta[i] != zeroPt) {
					if(!((poly == handlePoly) && (i == handleVert)))
						spline->SetVert(i,spline->GetVert(i) + delta[i]);
					else
						delta[i] = zeroPt;
					}
				}

			for(i = 0; i < knots; ++i) {
				if(kdelta[i])
					spline->SetKnotType(i,spline->GetKnotType(i) ^ kdelta[i]);
				if(ldelta[i])
					spline->SetLineType(i,spline->GetLineType(i) ^ ldelta[i]);
				}
//			spline->ComputeBezPoints();
//			DebugPrint(_T("Poly %d used"),poly);
			}
//		else
//			DebugPrint(_T("Poly %d not used"),poly);
		}
	shape.InvalidateGeomCache();
	}


#define SVD_POINTTAB_CHUNK		0x1000

IOResult ShapeVertexDelta::Save(ISave *isave) {
	isave->BeginChunk(SVD_POINTTAB_CHUNK);
	dtab.Save(isave);
	isave->	EndChunk();
	return IO_OK;
	}

IOResult ShapeVertexDelta::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case SVD_POINTTAB_CHUNK:
				res = dtab.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/

EditSplineData::EditSplineData()
	{
	flags = 0;
	tempData = NULL;
	oldShape = NULL;
	}

EditSplineData::EditSplineData(EditSplineData& esc)
	{
	flags = esc.flags;
	tempData = NULL;
	mapper = esc.mapper;
	finalShape = esc.finalShape;
	oldShape = NULL;
	}

EditSplineData::~EditSplineData() {
	if(oldShape) {
		delete oldShape;
		oldShape = NULL;
		}
	}

void EditSplineData::Apply(TimeValue t,SplineShape *splShape,int selLevel,BOOL showVertNumbers,BOOL SVNSelectedOnly)
	{
	// Either just copy it from the existing cache or rebuild from previous level!
	if ( !GetFlag(ESD_UPDATING_CACHE) && tempData && tempData->ShapeCached(t) ) {
		splShape->shape.DeepCopy( 
			tempData->GetShape(t),
			PART_GEOM|SELECT_CHANNEL|PART_SUBSEL_TYPE|PART_DISPLAY|PART_TOPO );		
		splShape->PointsWereChanged();
		}	
	else
	if ( GetFlag(ESD_HASDATA) ) {
		// For old files, which contain exhaustive data to reconstruct the editing process
		// of shapes, we'll have data in the 'changes' table.  If it's there, go ahead and
		// replay the edits, then store the alterations in our new delta format and discard
		// the change table!
		int count = changes.Count();
		if(count) {
//DebugPrint("*** Applying old style (%d) ***\n", count);
			// Store the topology for future reference
			mapper.Build(splShape->shape);
			finalShape = splShape->shape;
			for(int i = 0; i < count; ++i) {
				ModRecord *rec = changes[i];
				// Record the topo flags
				PreUpdateChanges(&splShape->shape);
				BOOL result = rec->Redo(&splShape->shape,0);
				UpdateChanges(&splShape->shape);
				// If we hit one that didn't play back OK, we need to flush the remainder
				if(!result) {
					for(int j = i; j < count; ++j)
						delete changes[j];
					changes.Delete(i, count - i);
					break;
					}
				}
			splShape->PointsWereChanged();
			// Nuke the changes table
			count = changes.Count();
			for(int k = 0; k < count; ++k)
				delete changes[k];
			changes.Delete(0, count);
			changes.Shrink();
			count = 0;
			}
		else {
			// Old way was to just stuff in finalShape.
//			splShape->shape = finalShape;
			// New way is to use some smarts and apply just the things we changed
			BezierShape theShape(finalShape);	// Init with our stored patch
			mapper.ApplyDeltas(splShape->shape, theShape);
			splShape->shape = theShape;			// Stuff it in!
			}
		// Kind of a waste when there's no animation...		
		splShape->UpdateValidity(GEOM_CHAN_NUM,FOREVER);
		splShape->UpdateValidity(TOPO_CHAN_NUM,FOREVER);
		splShape->UpdateValidity(SELECT_CHAN_NUM,FOREVER);
		splShape->UpdateValidity(SUBSEL_TYPE_CHAN_NUM,FOREVER);
		splShape->UpdateValidity(DISP_ATTRIB_CHAN_NUM,FOREVER);		
		}
	else {	// No data yet -- Store initial required data
//DebugPrint("<<<Storing Initial Data>>>\n");
		mapper.Build(splShape->shape);
		finalShape = splShape->shape;
		}
	
	// CAL-03/03/03: enable the display of Bezier handles
	BOOL dispBezier = splShape->shape.dispFlags & DISP_BEZHANDLES;
	splShape->shape.dispFlags = 0;
	switch ( selLevel ) {
		case ES_SPLINE:
			splShape->shape.SetDispFlag(DISP_VERTTICKS|DISP_SELPOLYS);
			break;
		case ES_VERTEX:
			if (showSelected) 
				{
				splShape->shape.SetDispFlag(DISP_VERTTICKS|DISP_SELVERTS|DISP_SELSEGMENTS);
//				shape->dispFlags |= DISP_SELSEGMENTS;
				}
			else
				{
				splShape->shape.SetDispFlag(DISP_VERTTICKS|DISP_SELVERTS);
//				shape->dispFlags &= ~DISP_SELSEGMENTS;
				}

			break;
		case ES_SEGMENT:
			splShape->shape.SetDispFlag(DISP_VERTTICKS|DISP_SELSEGMENTS);			
			break;
		}
	// CAL-03/03/03: enable the display of Bezier handles
	if (dispBezier) splShape->shape.dispFlags |= DISP_BEZHANDLES;

	splShape->showVertNumbers = showVertNumbers;
	splShape->SVNSelectedOnly = SVNSelectedOnly;
	
	splShape->shape.selLevel = shapeLevel[selLevel];
	
	if ( GetFlag(ESD_UPDATING_CACHE) ) {
		assert(tempData);
//DebugPrint("*** Updating edit spline cache ***\n");
		tempData->UpdateCache(splShape);
		SetFlag(ESD_UPDATING_CACHE,FALSE);
		}		
	}

void EditSplineData::Invalidate(PartID part,BOOL shapeValid)
	{
	if ( tempData ) {
		tempData->Invalidate(part,shapeValid);
		}
	}

void EditSplineData::BeginEdit(TimeValue t)
	{
	assert(tempData);
	if ( !GetFlag(ESD_HASDATA) )
		SetFlag(ESD_HASDATA,TRUE);
	}

ESTempData *EditSplineData::TempData(EditSplineMod *mod)
	{
	if ( !tempData ) {
		assert(mod->ip);
		tempData = new ESTempData(mod,this);
		}
	return tempData;
	}

void EditSplineData::RescaleWorldUnits(float f) {
	// Scale the deltas inside the vertex map
	mapper.RescaleWorldUnits(f);
	// Now rescale stuff inside our data structures
	Matrix3 stm = ScaleMatrix(Point3(f, f, f));
	finalShape.Transform(stm);
	}

/*-------------------------------------------------------------------*/		

class ESChangeVertSetRestore : public RestoreObj {
	public:
		ShapeVSel oldset,newset;
		int index;
		NamedVertSelSetList *setList;

		ESChangeVertSetRestore(NamedVertSelSetList *sl,int ix,ShapeVSel *o) {
			setList = sl; index = ix; oldset = *o;
			}   		
		void Restore(int isUndo) {
			newset = *(setList->sets[index]);
			*(setList->sets[index]) = oldset;
			}
		void Redo() {
			*(setList->sets[index]) = newset;
			}
				
		TSTR Description() {return TSTR(_T("Change Vert Set"));}
	};

class ESChangeSegSetRestore : public RestoreObj {
	public:
		ShapeSSel oldset, newset;
		int index;
		NamedSegSelSetList *setList;

		ESChangeSegSetRestore(NamedSegSelSetList *sl,int ix,ShapeSSel *o) {
			setList = sl; index = ix; oldset = *o;
			}   		
		void Restore(int isUndo) {
			newset = *(setList->sets[index]);
			*(setList->sets[index]) = oldset;
			}
		void Redo() {
			*(setList->sets[index]) = newset;
			}
				
		TSTR Description() {return TSTR(_T("Change Seg Set"));}
	};

class ESChangePolySetRestore : public RestoreObj {
	public:
		ShapePSel oldset, newset;
		int index;
		NamedPolySelSetList *setList;

		ESChangePolySetRestore(NamedPolySelSetList *sl,int ix,ShapePSel *o) {
			setList = sl; index = ix; oldset = *o;
			}   		
		void Restore(int isUndo) {
			newset = *(setList->sets[index]);
			*(setList->sets[index]) = oldset;
			}
		void Redo() {
			*(setList->sets[index]) = newset;
			}
				
		TSTR Description() {return TSTR(_T("Change Poly Set"));}
	};


class ESAppendVertSetRestore : public RestoreObj {
	public:
		ShapeVSel set;
		TSTR name;
		NamedVertSelSetList *setList;
		EditSplineMod *mod;

		ESAppendVertSetRestore(NamedVertSelSetList *sl,EditSplineMod *m) {
			setList = sl; mod = m;
			}   		
		void Restore(int isUndo) {
			set  = *setList->sets[setList->Count()-1];
			name = *setList->names[setList->Count()-1];
			setList->DeleteSet(setList->Count()-1);
			if (mod->ip) mod->ip->NamedSelSetListChanged();
			}
		void Redo() {
			setList->AppendSet(set,name);
			if (mod->ip) mod->ip->NamedSelSetListChanged();
			}
				
		TSTR Description() {return TSTR(_T("Append Vert Set"));}
	};

class ESAppendSegSetRestore : public RestoreObj {
	public:
		ShapeSSel set;
		TSTR name;
		NamedSegSelSetList *setList;
		EditSplineMod *mod;

		ESAppendSegSetRestore(NamedSegSelSetList *sl,EditSplineMod *m) {
			setList = sl; mod = m;
			}   		
		void Restore(int isUndo) {
			set  = *setList->sets[setList->Count()-1];
			name = *setList->names[setList->Count()-1];
			setList->DeleteSet(setList->Count()-1);
			if (mod->ip) mod->ip->NamedSelSetListChanged();
			}
		void Redo() {
			setList->AppendSet(set,name);
			if (mod->ip) mod->ip->NamedSelSetListChanged();
			}
				
		TSTR Description() {return TSTR(_T("Append Seg Set"));}
	};

class ESAppendPolySetRestore : public RestoreObj {
	public:
		ShapePSel set;
		TSTR name;
		NamedPolySelSetList *setList;
		EditSplineMod *mod;

		ESAppendPolySetRestore(NamedPolySelSetList *sl,EditSplineMod *m) {
			setList = sl; mod = m;
			}   		
		void Restore(int isUndo) {
			set  = *setList->sets[setList->Count()-1];
			name = *setList->names[setList->Count()-1];
			setList->DeleteSet(setList->Count()-1);
			if (mod->ip) mod->ip->NamedSelSetListChanged();
			}
		void Redo() {
			setList->AppendSet(set,name);
			if (mod->ip) mod->ip->NamedSelSetListChanged();
			}
				
		TSTR Description() {return TSTR(_T("Append Poly Set"));}
	};

class ESDeleteVertSetRestore : public RestoreObj {
	public:
		ShapeVSel set;
		TSTR name;
		int index;
		NamedVertSelSetList *setList;
		EditSplineMod *mod;

		ESDeleteVertSetRestore(int i,NamedVertSelSetList *sl,EditSplineMod *m) {
			setList = sl; mod = m; index = i;
			set  = *setList->sets[index];
			name = *setList->names[index];
			}   		
		void Restore(int isUndo) {
			ShapeVSel *n = new ShapeVSel(set);
			TSTR *nm = new TSTR(name);
			setList->sets.Insert(index,1,&n);
			setList->names.Insert(index,1,&nm);
			if (mod->FindSet(name, ES_VERTEX) < 0) mod->AddSet(name, ES_VERTEX); // mjm - 4.12.99
			if (mod->ip) mod->ip->NamedSelSetListChanged();
			}
		void Redo() {
			setList->DeleteSet(index);
			mod->RemoveSet(name, ES_VERTEX); // mjm - 4.12.99
			if (mod->ip) mod->ip->NamedSelSetListChanged();
			}
				
		TSTR Description() {return TSTR(_T("Delete Vert Set"));}
	};

class ESDeleteSegSetRestore : public RestoreObj {
	public:
		ShapeSSel set;
		TSTR name;
		int index;
		NamedSegSelSetList *setList;
		EditSplineMod *mod;

		ESDeleteSegSetRestore(int i,NamedSegSelSetList *sl,EditSplineMod *m) {
			setList = sl; mod = m; index = i;
			set  = *setList->sets[index];
			name = *setList->names[index];
			}   		
		void Restore(int isUndo) {
			ShapeSSel *n = new ShapeSSel(set);
			TSTR *nm = new TSTR(name);
			setList->sets.Insert(index,1,&n);
			setList->names.Insert(index,1,&nm);
			if (mod->FindSet(name, ES_SEGMENT) < 0) mod->AddSet(name, ES_SEGMENT); // mjm - 4.12.99
			if (mod->ip) mod->ip->NamedSelSetListChanged();
			}
		void Redo() {
			setList->DeleteSet(index);
			mod->RemoveSet(name, ES_SEGMENT); // mjm - 4.12.99
			if (mod->ip) mod->ip->NamedSelSetListChanged();
			}
				
		TSTR Description() {return TSTR(_T("Delete Seg Set"));}
	};

class ESDeletePolySetRestore : public RestoreObj {
	public:
		ShapePSel set;
		TSTR name;
		int index;
		NamedPolySelSetList *setList;
		EditSplineMod *mod;

		ESDeletePolySetRestore(int i,NamedPolySelSetList *sl,EditSplineMod *m) {
			setList = sl; mod = m; index = i;
			set  = *setList->sets[index];
			name = *setList->names[index];
			}   		
		void Restore(int isUndo) {
			ShapePSel *n = new ShapePSel(set);
			TSTR *nm = new TSTR(name);
			setList->sets.Insert(index,1,&n);
			setList->names.Insert(index,1,&nm);
			if (mod->FindSet(name, ES_SPLINE) < 0) mod->AddSet(name, ES_SPLINE); // mjm - 4.12.99
			if (mod->ip) mod->ip->NamedSelSetListChanged();
			}
		void Redo() {
			setList->DeleteSet(index);
			mod->RemoveSet(name, ES_SPLINE); // mjm - 4.12.99
			if (mod->ip) mod->ip->NamedSelSetListChanged();
			}
				
		TSTR Description() {return TSTR(_T("Delete Poly Set"));}
	};

class ESVertSetNameRestore : public RestoreObj {
	public:
		TSTR undo, redo;
		int index;
		NamedVertSelSetList *setList;
		EditSplineMod *mod;
		ESVertSetNameRestore(int i,NamedVertSelSetList *sl,EditSplineMod *m) {
			index = i; setList = sl; mod = m;
			undo = *setList->names[index];
			}

		void Restore(int isUndo) {			
			redo = *setList->names[index];
			*setList->names[index] = undo;
			if (mod->ip) mod->ip->NamedSelSetListChanged();
			}
		void Redo() {
			*setList->names[index] = redo;
			if (mod->ip) mod->ip->NamedSelSetListChanged();
			}
				
		TSTR Description() {return TSTR(_T("Set Vert Set Name"));}
	};

class ESSegSetNameRestore : public RestoreObj {
	public:
		TSTR undo, redo;
		int index;
		NamedSegSelSetList *setList;
		EditSplineMod *mod;
		ESSegSetNameRestore(int i,NamedSegSelSetList *sl,EditSplineMod *m) {
			index = i; setList = sl; mod = m;
			undo = *setList->names[index];
			}

		void Restore(int isUndo) {			
			redo = *setList->names[index];
			*setList->names[index] = undo;
			if (mod->ip) mod->ip->NamedSelSetListChanged();
			}
		void Redo() {
			*setList->names[index] = redo;
			if (mod->ip) mod->ip->NamedSelSetListChanged();
			}
				
		TSTR Description() {return TSTR(_T("Set Seg Set Name"));}
	};

class ESPolySetNameRestore : public RestoreObj {
	public:
		TSTR undo, redo;
		int index;
		NamedPolySelSetList *setList;
		EditSplineMod *mod;
		ESPolySetNameRestore(int i,NamedPolySelSetList *sl,EditSplineMod *m) {
			index = i; setList = sl; mod = m;
			undo = *setList->names[index];
			}

		void Restore(int isUndo) {			
			redo = *setList->names[index];
			*setList->names[index] = undo;
			if (mod->ip) mod->ip->NamedSelSetListChanged();
			}
		void Redo() {
			*setList->names[index] = redo;
			if (mod->ip) mod->ip->NamedSelSetListChanged();
			}
				
		TSTR Description() {return TSTR(_T("Set Poly Set Name"));}
	};

class ESAddSetRestore : public RestoreObj {
	public:
		EditSplineMod *mod;
		TSTR name;
		int selLevel;
		ESAddSetRestore(TSTR &n, int level, EditSplineMod *m) {
			name = n;
			selLevel = level;
			mod = m;
			}
		void Restore(int isUndo) {			
			mod->namedSel[selLevel-1].Delete(mod->namedSel[selLevel-1].Count()-1, 1);
			if (mod->ip) mod->ip->NamedSelSetListChanged();
			}
		void Redo() {
			mod->AddSet(name, selLevel);
			if (mod->ip) mod->ip->NamedSelSetListChanged();
			}
				
		TSTR Description() {return TSTR(_T("Add Set Name"));}
	};

// --------------------------------------------------------------------------------------

class ESVertMapRestore : public RestoreObj {
public:
	BOOL gotRedo;
	ESVertMapper undo;
	ESVertMapper redo;
	EditSplineData *esd;
	
	ESVertMapRestore(EditSplineData *d) {
		undo = d->mapper;
		esd = d;
		gotRedo = FALSE;
		}

	void Restore(int isUndo) {
		if(!gotRedo) {
			gotRedo = TRUE;
			redo = esd->mapper;
			}
		esd->mapper = undo;
		}

	void Redo() {
		esd->mapper = redo;
		}

	int Size() { return 1; }
	void EndHold() { }
	TSTR Description() { return TSTR(_T("ESVertMapRestore")); }
};

// --------------------------------------------------------------------------------------

class FinalShapeRestore : public RestoreObj {
public:
	BOOL gotRedo;
	BezierShape undo;
	BezierShape redo;
	BezierShape *shape;
	
	FinalShapeRestore(BezierShape *s) {
		undo = *s;
		shape = s;
		gotRedo = FALSE;
		}

	void Restore(int isUndo) {
		if(!gotRedo) {
			gotRedo = TRUE;
			redo = *shape;
			}
		*shape = undo;
		}

	void Redo() {
		*shape = redo;
		}

	int Size() { return 1; }
	void EndHold() { }
	TSTR Description() { return TSTR(_T("FinalShapeRestore")); }
};

// --------------------------------------------------------------------------------------

// Edit Spline Vert Mapper methods follow...

ESPolysPV::~ESPolysPV() {
	if(vertMap) {
		delete [] vertMap;
		vertMap = NULL;
		}
	}

void ESPolysPV::Build(Spline3D &spline, int poly) {
	if(vertMap)
		delete [] vertMap;
	verts = spline.Verts();
	vertMap = new ESPolyVert[verts];
	for(int i = 0; i < verts; ++i)
		vertMap[i] = ESPolyVert(poly, i, Point3(0,0,0));
	}

void ESPolysPV::Reset() {
	for(int i = 0; i < verts; ++i)
		vertMap[i].poly = vertMap[i].vert = -1;
	}

ESPolysPV& ESPolysPV::operator=(ESPolysPV &from) {
	if(vertMap)
		delete [] vertMap;
	verts = from.verts;
	vertMap = new ESPolyVert [verts];
	for(int i = 0; i < verts; ++i)
		vertMap[i] = from.vertMap[i];
	return *this;
	}

void ESPolysPV::RescaleWorldUnits(float f) {
	for(int i = 0; i < verts; ++i) {
		vertMap[i].delta *= f;
		}
	}

#define PPV_DATA_CHUNK 0x1000
#define PPV_DATA_CHUNK_R4 0x1010

IOResult ESPolysPV::Save(ISave *isave) {
	ULONG nb;
	isave->BeginChunk(PPV_DATA_CHUNK_R4);
	isave->Write(&verts,sizeof(int),&nb);
	for(int i = 0; i < verts; ++i) {
		isave->Write(&vertMap[i].poly,sizeof(int),&nb);
		isave->Write(&vertMap[i].vert,sizeof(int),&nb);
		isave->Write(&vertMap[i].delta,sizeof(Point3),&nb);
		isave->Write(&vertMap[i].flags,sizeof(DWORD),&nb);
		}
	isave->EndChunk();
	return IO_OK;
	}

IOResult ESPolysPV::Load(ILoad *iload) {
	int i;
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case PPV_DATA_CHUNK:
				res = iload->Read(&verts,sizeof(int),&nb);
				if(vertMap)
					delete [] vertMap;
				vertMap = new ESPolyVert[verts];
				for(i = 0; i < verts; ++i) {
					iload->Read(&vertMap[i].poly,sizeof(int),&nb);
					iload->Read(&vertMap[i].vert,sizeof(int),&nb);
					res = iload->Read(&vertMap[i].delta,sizeof(Point3),&nb);
					vertMap[i].flags = ESP_KNOTTYPE_ALTERED | ESP_LINETYPE_ALTERED;	// Default old ones to this
					}
				iload->SetObsolete();
				break;
			case PPV_DATA_CHUNK_R4:
				res = iload->Read(&verts,sizeof(int),&nb);
				if(vertMap)
					delete [] vertMap;
				vertMap = new ESPolyVert[verts];
				for(i = 0; i < verts; ++i) {
					iload->Read(&vertMap[i].poly,sizeof(int),&nb);
					iload->Read(&vertMap[i].vert,sizeof(int),&nb);
					iload->Read(&vertMap[i].delta,sizeof(Point3),&nb);
					res = iload->Read(&vertMap[i].flags,sizeof(DWORD),&nb);
					}
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

ESVertMapper::~ESVertMapper() {
	if(polyMap) {
		delete [] polyMap;
		polyMap = NULL;
		}
	}

void ESVertMapper::Build(BezierShape &shape) {
	polys = shape.splineCount;
	if(polyMap)
		delete [] polyMap;
	polyMap = new ESPolysPV[polys];
	for(int i = 0; i < polys; ++i)
		polyMap[i].Build(*shape.splines[i], i);
	}

void ESVertMapper::ApplyDeltas(BezierShape &inShape, BezierShape &outShape) {
#ifdef VMAP_DEBUG
	DebugPrint("Updating/Applying Deltas...\n");
#endif
	// Copy the shape's steps value -- Need to preserve them as they change! TH 5/6/99
	outShape.steps = inShape.steps;
	outShape.optimize = inShape.optimize;

	// Now apply deltas to output
	for(int poly = 0; poly < polys; ++poly) {
		ESPolysPV &pmap = polyMap[poly];
		if(poly < inShape.SplineCount()) {
			Spline3D *inSpline = inShape.splines[poly];
			for(int vert = 0; vert < pmap.verts; ++vert) {
				ESPolyVert &pv = pmap.vertMap[vert];
				if(pv.poly >= 0 && pv.poly < outShape.splineCount) {
					Spline3D *outSpline = outShape.splines[pv.poly];
					if(pv.vert >= 0 && pv.vert < outSpline->Verts()) {
						if(vert >= inSpline->Verts())
							outSpline->SetVert(pv.vert, pv.delta);
						else {
							// Apply the position delta
							outSpline->SetVert(pv.vert, inSpline->GetVert(vert) + pv.delta);
							}
						}
					}
				}
			}
		}
	// Post-processing step for knot verts only -- Apply changes to vertex types!
	// This is necessary because changes to vector positions from below have to be constrained
	// for normal bezier knots
	BOOL knotTypeSet = FALSE;
	for(poly = 0; poly < polys; ++poly) {
		ESPolysPV &pmap = polyMap[poly];
		if(poly < inShape.SplineCount()) {
			Spline3D *inSpline = inShape.splines[poly];
			for(int vert = 1; vert < pmap.verts; vert+=3) {
				ESPolyVert &pv = pmap.vertMap[vert];
				if(pv.poly >= 0 && pv.poly < outShape.splineCount) {
					Spline3D *outSpline = outShape.splines[pv.poly];
					if(pv.vert >= 0 && pv.vert < outSpline->Verts()) {
						if(vert < inSpline->Verts()) {
							// If we altered the knot type, shove it in
							if(pv.flags & ESP_KNOTTYPE_ALTERED) {
								int type = outSpline->GetKnotType(pv.vert / 3);
								if(type == KTYPE_BEZIER) {
									outSpline->SetKnotType(pv.vert / 3, outSpline->GetKnotType(pv.vert / 3));	// Looks dumb, but it forces a handle conatrsint to be applied for collinears
									knotTypeSet = TRUE;
									}
								}
							else {	// If we didn't alter the knot type, let the old one apply
								outSpline->SetKnotType(pv.vert / 3, inSpline->GetKnotType(vert / 3));
								knotTypeSet = TRUE;
								}
							// If we didn't alter the line type, let the old one apply
							if(!(pv.flags & ESP_LINETYPE_ALTERED))
								outSpline->SetLineType(pv.vert / 3, inSpline->GetLineType(vert / 3));
							}
						}
					}
				}
			}
		}
#ifdef VMAP_DEBUG
	DebugPrint("Applying Deltas Complete!\n");
#endif
	}

void ESVertMapper::RecordTopologyTags(BezierShape &shape) {
	// First, stuff all -1's into aux fields
	for(int poly = 0; poly < shape.splineCount; ++poly) {
		Spline3D *spline = shape.GetSpline(poly);
		int verts = spline->Verts();
		for(int vert = 0; vert < verts; ++vert)
			spline->SetVertAux(vert, 0, 0xffffffff);
//to track binds
		for(int knot = 0; knot < spline->KnotCount(); ++knot)
			{
			spline->SetAux2(knot, 0xffffffff);
			}
		}
	for(poly = 0; poly < polys; ++poly) {
		ESPolysPV &pmap = polyMap[poly];
		for(int vert = 0; vert < pmap.verts; ++vert) {
			int mappoly = pmap.vertMap[vert].poly;
			int mapvert = pmap.vertMap[vert].vert;
			// If it's still mapped, record it!
			if(mappoly >= 0 && mapvert >= 0 && 
				mapvert < shape.splines[mappoly]->Verts())
				shape.splines[mappoly]->SetVertAux(mapvert, 0, poly<<16 | vert);
			}
		}

	for(poly = 0; poly < shape.splineCount; ++poly) 
		{
		Spline3D *spline = shape.GetSpline(poly);

		for(int knot = 0; knot < spline->KnotCount(); ++knot)
			{
			spline->SetAux2(knot, poly<<16 | knot);

			}
		}
	}

class PrevIndex {
	public:
		TempIntTab prevVert;
		TempIntTab prevPoly;
		PrevIndex() {}
		void Alloc(int ct) { prevVert.Alloc(ct); prevPoly.Alloc(ct); }
	};

void ESVertMapper::RecomputeDeltas(BOOL checkTopology, BezierShape &shape, BezierShape &oldShape) {
#ifdef VMAP_DEBUG
	DebugPrint("Recomputing Deltas...\n");
#endif
	// Hang on to the original indexes
	PrevIndex *indexes = new PrevIndex[polys];
	for(int poly = 0; poly < polys; ++poly) {
		ESPolysPV &pmap = polyMap[poly];
		PrevIndex &index = indexes[poly];
		index.Alloc(pmap.verts);
		for(int vert = 0; vert < pmap.verts; ++vert) {
			index.prevVert[vert] = pmap.vertMap[vert].vert;
			index.prevPoly[vert] = pmap.vertMap[vert].poly;
			}
		}
	// If topology may have changed, fix up the mapping!
	if(checkTopology) {
		// Flush existing mapping
		for(int poly = 0; poly < polys; ++poly)
			polyMap[poly].Reset();
		// Build the new mapping
		int wpolys = shape.splineCount;
		for(int wpoly = 0; wpoly < wpolys; ++wpoly) {
			Spline3D *spline = shape.splines[wpoly];
			int wverts = spline->Verts();
			for(int wvert = 0; wvert < wverts; ++wvert) {
				int tag = spline->GetVertAux(wvert, 0);
				int oldPoly = tag >> 16;
				if(oldPoly >= 0 && oldPoly < polys) {
					int oldVert = tag & 0xffff;
					ESPolysPV &pmap = polyMap[oldPoly];
					if(oldVert >= 0 && oldVert < pmap.verts) {
						pmap.vertMap[oldVert].vert = wvert;
						pmap.vertMap[oldVert].poly = wpoly;
						}
					}
				}
			}
		}
	// Now compute the vertex deltas...
	for(poly = 0; poly < polys; ++poly) {
		ESPolysPV &pmap = polyMap[poly];
		PrevIndex &index = indexes[poly];
		for(int vert = 0; vert < pmap.verts; ++vert) {
			ESPolyVert &map = pmap.vertMap[vert];
			if(map.vert >= 0) {
				int prevVert = index.prevVert[vert];
				int prevPoly = index.prevPoly[vert];
				assert(prevVert >= 0 && prevPoly >= 0);
				Spline3D *newSpline = shape.splines[map.poly];
				Spline3D *oldSpline = oldShape.splines[prevPoly];
				map.delta += (newSpline->GetVert(map.vert) - oldSpline->GetVert(prevVert));
				// If a knot point, check its type for change
				if((map.vert % 3) == 1 && (prevVert % 3) == 1) {
					int oldType = oldSpline->GetKnotType(prevVert / 3);
					int newType = newSpline->GetKnotType(map.vert / 3);
					if(oldType != newType)
						map.flags |= ESP_KNOTTYPE_ALTERED;
					oldType = oldSpline->GetLineType(prevVert / 3);
					newType = newSpline->GetLineType(map.vert / 3);
					if(oldType != newType)
						map.flags |= ESP_LINETYPE_ALTERED;
					}
				}
			}
		}
	delete [] indexes;			
#ifdef VMAP_DEBUG
	DebugPrint("Recomputing Deltas Complete!\n");
#endif
	}

ESVertMapper& ESVertMapper::operator=(ESVertMapper &from) {
	if(polyMap)
		delete [] polyMap;
	polys = from.polys;
	polyMap = new ESPolysPV[polys];
	for(int i = 0; i < polys; ++i)
		polyMap[i] = from.polyMap[i];
	return *this;
	}

void ESVertMapper::RescaleWorldUnits(float f) {
	for(int i = 0; i < polys; ++i)
		polyMap[i].RescaleWorldUnits(f);
	}

#define ESVM_DATA_CHUNK 0x1000
#define ESVM_POLYMAP_CHUNK 0x1010

IOResult ESVertMapper::Save(ISave *isave) {
	ULONG nb;
	isave->BeginChunk(ESVM_DATA_CHUNK);
	isave->Write(&polys,sizeof(int),&nb);
	isave->EndChunk();
	for(int i = 0; i < polys; ++i) {
		isave->BeginChunk(ESVM_POLYMAP_CHUNK);
		polyMap[i].Save(isave);
		isave->	EndChunk();
		}
	return IO_OK;
	}

IOResult ESVertMapper::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	int index = 0;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case ESVM_DATA_CHUNK:
				res = iload->Read(&polys,sizeof(int),&nb);
				if(polyMap)
					delete [] polyMap;
				polyMap = new ESPolysPV[polys];
				break;
			case ESVM_POLYMAP_CHUNK:
				assert(index < polys);
				res = polyMap[index++].Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

// --------------------------------------------------------------------------------------

void EditSplineData::PreUpdateChanges(BezierShape *shape, BOOL checkTopology) {
//DebugPrint("Preupdating changes\n");
	// Store a temporary copy so we can compare after modifying
	if(!oldShape)
		oldShape = new BezierShape(*shape);
	else
		*oldShape = *shape;
	// Now put in our tags
	mapper.RecordTopologyTags(*shape);
	}

// Alter the stored transformation data to match the current edited shape
void EditSplineData::UpdateChanges(BezierShape *shape, BOOL checkTopology, int bindTrackingOptions) {
	if(theHold.Holding()) {
		theHold.Put(new ESVertMapRestore(this));
		theHold.Put(new FinalShapeRestore(&finalShape));
		}
	mapper.RecomputeDeltas(checkTopology, *shape, *oldShape);
	if(checkTopology && bindTrackingOptions) {
		if (bindTrackingOptions == 2)
			shape->UpdateBindList(TRUE);
		else
			shape->UpdateBindList(FALSE);
		}
	finalShape = *shape;
	}

#define ESD_GENERAL_CHUNK		0x1000	// Obsolete as of 10/28/98 (r3)
#define CHANGE_CHUNK			0x1010	// Obsolete as of 10/28/98 (r3)
#define ESD_R3_GENERAL_CHUNK	0x1020
#define VERTMAP_CHUNK			0x1030
#define FINALSHAPE_CHUNK		0x1040

// Named sel set chunks
#define VSELSET_CHUNK		0x1060
#define SSELSET_CHUNK		0x1070
#define PSELSET_CHUNK		0x1080

IOResult EditSplineData::Save(ISave *isave) {	
	ULONG nb;
	isave->BeginChunk(ESD_R3_GENERAL_CHUNK);
	isave->Write(&flags,sizeof(DWORD),&nb);
	isave->EndChunk();
	// Save named sel sets
	if (vselSet.Count()) {
		isave->BeginChunk(VSELSET_CHUNK);
		vselSet.Save(isave);
		isave->EndChunk();
		}
	if (sselSet.Count()) {
		isave->BeginChunk(SSELSET_CHUNK);
		sselSet.Save(isave);
		isave->EndChunk();
		}
	if (pselSet.Count()) {
		isave->BeginChunk(PSELSET_CHUNK);
		pselSet.Save(isave);
		isave->EndChunk();
		}

	isave->BeginChunk(VERTMAP_CHUNK);
	mapper.Save(isave);
	isave->EndChunk();
	isave->BeginChunk(FINALSHAPE_CHUNK);
	finalShape.Save(isave);
	isave->EndChunk();
	return IO_OK;
	}

IOResult EditSplineData::Load(ILoad *iload) {	
	static int chCount;
	IOResult res;
	ULONG nb;
	ModRecord *theChange;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			// The following code is here to load pre-release 3 files.
			case ESD_GENERAL_CHUNK:
				iload->SetObsolete();
				res = iload->Read(&flags,sizeof(DWORD),&nb);
				res = iload->Read(&chCount,sizeof(int),&nb);
				break;
			case CLEARVERTSELRECORD_CHUNK:
				theChange = new ClearVertSelRecord;
				goto load_change;
			case SETVERTSELRECORD_CHUNK:
				theChange = new SetVertSelRecord;
				goto load_change;
			case INVERTVERTSELRECORD_CHUNK:
				theChange = new InvertVertSelRecord;
				goto load_change;
			case CLEARSEGSELRECORD_CHUNK:
				theChange = new ClearSegSelRecord;
				goto load_change;
			case SETSEGSELRECORD_CHUNK:
				theChange = new SetSegSelRecord;
				goto load_change;
			case INVERTSEGSELRECORD_CHUNK:
				theChange = new InvertSegSelRecord;
				goto load_change;
			case CLEARPOLYSELRECORD_CHUNK:
				theChange = new ClearPolySelRecord;
				goto load_change;
			case SETPOLYSELRECORD_CHUNK:
				theChange = new SetPolySelRecord;
				goto load_change;
			case INVERTPOLYSELRECORD_CHUNK:
				theChange = new InvertPolySelRecord;
				goto load_change;
			case VERTSELRECORD_CHUNK:
				theChange = new VertSelRecord;
				goto load_change;
			case SEGSELRECORD_CHUNK:
				theChange = new SegSelRecord;
				goto load_change;
			case POLYSELRECORD_CHUNK:
				theChange = new PolySelRecord;
				goto load_change;
			case POLYCLOSERECORD_CHUNK:
				theChange = new PolyCloseRecord;
				goto load_change;
			case POLYREVERSERECORD_CHUNK:
				theChange = new PolyReverseRecord;
				goto load_change;
			case POLYMIRRORRECORD_CHUNK:
				theChange = new PolyMirrorRecord;
				goto load_change;
			case POLYENDATTACHRECORD_CHUNK:
				theChange = new PolyEndAttachRecord;
				goto load_change;
			case OUTLINERECORD_CHUNK:
				theChange = new OutlineRecord;
				goto load_change;
			case POLYDETACHRECORD_CHUNK:
				theChange = new PolyDetachRecord;
				goto load_change;
			case POLYDELETERECORD_CHUNK:
				theChange = new PolyDeleteRecord;
				goto load_change;
			case VERTMOVERECORD_CHUNK:
				theChange = new VertMoveRecord;
				goto load_change;
			case SEGDELETERECORD_CHUNK:
				theChange = new SegDeleteRecord;
				goto load_change;
			case SEGDETACHRECORD_CHUNK:
				theChange = new SegDetachRecord;
				goto load_change;
			case POLYFIRSTRECORD_CHUNK:
				theChange = new PolyFirstRecord;
				goto load_change;
			case SEGBREAKRECORD_CHUNK:
				theChange = new SegBreakRecord;
				goto load_change;
			case SEGREFINERECORD_CHUNK:
				theChange = new SegRefineRecord;
				goto load_change;
			case VERTBREAKRECORD_CHUNK:
				theChange = new VertBreakRecord;
				goto load_change;
			case VERTCONNECTRECORD_CHUNK:
				theChange = new VertConnectRecord;
				goto load_change;
			case VERTINSERTRECORD_CHUNK:
				theChange = new VertInsertRecord;
				goto load_change;
			case VERTWELDRECORD_CHUNK:
				theChange = new VertWeldRecord;
				goto load_change;
			case BOOLEANRECORD_CHUNK:
				theChange = new BooleanRecord;
				goto load_change;
			case ATTACHRECORD_CHUNK:
				theChange = new AttachRecord;
				goto load_change;
			case VERTCHANGERECORD_CHUNK:
				theChange = new VertChangeRecord;
				goto load_change;
			case SEGCHANGERECORD_CHUNK:
				theChange = new SegChangeRecord;
				goto load_change;
			case POLYCHANGERECORD_CHUNK:
				theChange = new PolyChangeRecord;
				goto load_change;
			case CREATELINERECORD_CHUNK:
				theChange = new CreateLineRecord;
				goto load_change;
			case POLYCOPYRECORD_CHUNK:
				theChange = new PolyCopyRecord;
				goto load_change;
			case SEGCOPYRECORD_CHUNK:
				theChange = new SegCopyRecord;
				goto load_change;
			case VERTDELETERECORD_CHUNK:
				theChange = new VertDeleteRecord;
				// Intentional fall-thru!
				load_change:
				changes.Append(1,&theChange);
				changes[changes.Count()-1]->Load(iload);
				break;
			//
			// The following code is used for post-release 3 files
			//
			case ESD_R3_GENERAL_CHUNK:
				res = iload->Read(&flags,sizeof(DWORD),&nb);
				break;
			// Load named selection sets
			case VSELSET_CHUNK:
				res = vselSet.Load(iload);
				break;
			case PSELSET_CHUNK:
				res = pselSet.Load(iload);
				break;
			case SSELSET_CHUNK:
				res = sselSet.Load(iload);
				break;
			case VERTMAP_CHUNK:
				res = mapper.Load(iload);
				break;
			case FINALSHAPE_CHUNK:
				res = finalShape.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL ClearVertSelRecord::Redo(BezierShape *shape,int reRecord) {
	if(reRecord)
		sel = shape->vertSel;
	shape->vertSel.ClearAll();
	return TRUE;
	}

#define CVSR_SEL_CHUNK 0x1000

IOResult ClearVertSelRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case CVSR_SEL_CHUNK:
				res = sel.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL SetVertSelRecord::Redo(BezierShape *shape,int reRecord) {
	if(reRecord)
		sel = shape->vertSel;
	shape->vertSel.SetAll();
	return TRUE;
	}

#define SVSR_SEL_CHUNK 0x1000

IOResult SetVertSelRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case SVSR_SEL_CHUNK:
				res = sel.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL InvertVertSelRecord::Redo(BezierShape *shape,int reRecord) {
	shape->vertSel.Toggle();
	return TRUE;
	}

IOResult InvertVertSelRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL ClearSegSelRecord::Redo(BezierShape *shape,int reRecord) {
	if(reRecord)
		sel = shape->segSel;
	shape->segSel.ClearAll();
	return TRUE;
	}

#define CSSR_SEL_CHUNK 0x1000

IOResult ClearSegSelRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case CSSR_SEL_CHUNK:
				res = sel.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL SetSegSelRecord::Redo(BezierShape *shape,int reRecord) {
	if(reRecord)
		sel = shape->segSel;
	shape->segSel.SetAll();
	return TRUE;
	}

#define SSSR_SEL_CHUNK 0x1000

IOResult SetSegSelRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case SSSR_SEL_CHUNK:
				res = sel.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL InvertSegSelRecord::Redo(BezierShape *shape,int reRecord) {
	shape->segSel.Toggle();
	return TRUE;
	}

IOResult InvertSegSelRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL ClearPolySelRecord::Redo(BezierShape *shape,int reRecord) {
	if(reRecord)
		sel = shape->polySel;
	shape->polySel.ClearAll();
	return TRUE;
	}

#define CPSR_SEL_CHUNK 0x1000

IOResult ClearPolySelRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case CPSR_SEL_CHUNK:
				res = sel.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL SetPolySelRecord::Redo(BezierShape *shape,int reRecord) {
	if(reRecord)
		sel = shape->polySel;
	shape->polySel.SetAll();
	return TRUE;
	}

#define SPSR_SEL_CHUNK 0x1000

IOResult SetPolySelRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case SPSR_SEL_CHUNK:
				res = sel.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL InvertPolySelRecord::Redo(BezierShape *shape,int reRecord) {
	shape->polySel.Toggle();
	return TRUE;
	}

IOResult InvertPolySelRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL VertSelRecord::Redo(BezierShape *shape,int reRecord) {
	if(!newSel.IsCompatible(*shape))
		return FALSE;
	shape->vertSel = newSel;
	return TRUE;
	}

#define VSR_OLDSEL_CHUNK 0x1000
#define VSR_NEWSEL_CHUNK 0x1010

IOResult VertSelRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case VSR_OLDSEL_CHUNK:
				res = oldSel.Load(iload);
				break;
			case VSR_NEWSEL_CHUNK:
				res = newSel.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL SegSelRecord::Redo(BezierShape *shape,int reRecord) {
	if(!newSel.IsCompatible(*shape))
		return FALSE;
	shape->segSel = newSel;
	return TRUE;
	}

#define SSR_OLDSEL_CHUNK 0x1000
#define SSR_NEWSEL_CHUNK 0x1010

IOResult SegSelRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case SSR_OLDSEL_CHUNK:
				res = oldSel.Load(iload);
				break;
			case SSR_NEWSEL_CHUNK:
				res = newSel.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL PolySelRecord::Redo(BezierShape *shape,int reRecord) {
	if(!newSel.IsCompatible(*shape))
		return FALSE;
	shape->polySel = newSel;
	return TRUE;
	}

#define PSR_OLDSEL_CHUNK 0x1000
#define PSR_NEWSEL_CHUNK 0x1010

IOResult PolySelRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case PSR_OLDSEL_CHUNK:
				res = oldSel.Load(iload);
				break;
			case PSR_NEWSEL_CHUNK:
				res = newSel.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL PolyCloseRecord::Redo(BezierShape *shape,int reRecord) {
	if(poly >= shape->splineCount)
		return FALSE;
	Spline3D *spline = shape->splines[poly];
	spline->SetClosed();
	spline->ComputeBezPoints();
	shape->InvalidateGeomCache();
	shape->vertSel[poly].SetSize(spline->Verts(),1);
	shape->segSel[poly].SetSize(spline->Segments(),1);
	shape->segSel[poly].Clear(spline->Segments()-1);
	shape->polySel.Clear(poly);
	return TRUE;
	}

#define PCR_POLY_CHUNK 0x1000

IOResult PolyCloseRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case PCR_POLY_CHUNK:
				res = iload->Read(&poly,sizeof(int),&nb);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL PolyReverseRecord::Redo(BezierShape *shape,int reRecord) {
	if(poly >= shape->splineCount)
		return FALSE;
	shape->Reverse(poly);
	shape->splines[poly]->ComputeBezPoints();
	shape->InvalidateGeomCache();
	return TRUE;
	}

#define PRR_POLY_CHUNK 0x1000

IOResult PolyReverseRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case PRR_POLY_CHUNK:
				res = iload->Read(&poly,sizeof(int),&nb);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

static void MirrorPoly(BezierShape *shape, int poly, int type, BOOL copy, Point3 *pivot = NULL) {
	if(copy) {
		Spline3D *newSpline = shape->NewSpline();
		*newSpline = *shape->splines[poly];
		shape->polySel.Clear(poly);		// Unselect the old one
		poly = shape->SplineCount() - 1;
		// Don't forget to create a new selection record for this new spline!
		shape->vertSel.Insert(poly, newSpline->KnotCount() * 3);
		shape->segSel.Insert(poly, newSpline->Segments());
		shape->polySel.Insert(poly);
		shape->polySel.Set(poly);	// Select the new one!
		}
	// Now mirror it!
	Spline3D *spline = shape->splines[poly];
	// Find its center
	Box3 bbox;
	bbox.Init();
	for(int k = 0; k < spline->KnotCount(); ++k)
		bbox += spline->GetKnotPoint(k);

	Point3 center;
	if(pivot)
		center = -(*pivot);
	else
		center = bbox.Center();
	for(k = 0; k < spline->KnotCount(); ++k) {
		Point3 knot = spline->GetKnotPoint(k);
		Point3 in = spline->GetInVec(k);
		Point3 out = spline->GetOutVec(k);
		if(type == MIRROR_BOTH || type == MIRROR_HORIZONTAL) {
			knot.x = center.x - (knot.x - center.x);
			in.x = center.x - (in.x - center.x);
			out.x = center.x - (out.x - center.x);
			}
		if(type == MIRROR_BOTH || type == MIRROR_VERTICAL) {
			knot.y = center.y - (knot.y - center.y);
			in.y = center.y - (in.y - center.y);
			out.y = center.y - (out.y - center.y);
			}
		spline->SetKnotPoint(k, knot);
		spline->SetInVec(k, in);
		spline->SetOutVec(k, out);
		}
	spline->ComputeBezPoints();
	shape->InvalidateGeomCache();
	}

BOOL PolyMirrorRecord::Redo(BezierShape *shape,int reRecord) {
	if(poly >= shape->splineCount)
		return FALSE;
	MirrorPoly(shape, poly, type, copy);
	return TRUE;
	}

#define PMR_POLY_CHUNK 0x1000
#define PMR_TYPE_CHUNK 0x1010
#define PMR_COPY_CHUNK 0x1020

IOResult PolyMirrorRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case PMR_POLY_CHUNK:
				res = iload->Read(&poly,sizeof(int),&nb);
				break;
			case PMR_TYPE_CHUNK:
				res = iload->Read(&type,sizeof(int),&nb);
				break;
			case PMR_COPY_CHUNK:
				res = iload->Read(&copy,sizeof(BOOL),&nb);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

static void DoPolyEndAttach(BezierShape *shape, int poly1, int vert1, int poly2, int vert2, BOOL r3Way=TRUE) {
	Spline3D *spline = shape->splines[poly1];
	int knots = spline->KnotCount();
	int verts = spline->Verts();
	int segs = spline->Segments();

	// If connecting endpoints of the same polygon, close it and get rid of vertex 1
	if(poly1 == poly2) {
		spline->SetClosed();
		BitArray& vsel = shape->vertSel[poly1];
		BitArray& ssel = shape->segSel[poly1];
		BOOL bothAuto = (spline->GetKnotType(vert1) == KTYPE_AUTO && spline->GetKnotType(vert2) == KTYPE_AUTO) ? TRUE : FALSE;
		int lastVert = knots - 1;
		Point3 p1 = spline->GetKnotPoint(0);
		Point3 p2 = spline->GetKnotPoint(lastVert);
		Point3 midpoint = (p1 + p2) / 2.0f;
		Point3 p1Delta = midpoint - p1;
		Point3 p2Delta = midpoint - p2;
		// Repoint the knot
		spline->SetKnotPoint(0, midpoint);
		if(!bothAuto) {
			spline->SetKnotType(0, KTYPE_BEZIER_CORNER);
			spline->SetInVec(0, spline->GetInVec(lastVert) + p2Delta);
			spline->SetOutVec(0, spline->GetOutVec(0) + p1Delta);
			}
		spline->DeleteKnot(lastVert);
		// Make the selection sets the right size
		vsel.SetSize(spline->Verts(), 1);
		ssel.SetSize(spline->Segments(), 1);
//3-31-99 watje
//		shape->UpdateBindList();
		spline->ComputeBezPoints();
		shape->InvalidateGeomCache();
		}
	else { 		// Connecting two different splines -- Splice 'em together!
		Spline3D *spline2 = shape->splines[poly2];
		BOOL bothAuto = (spline->GetKnotType(vert1) == KTYPE_AUTO && spline2->GetKnotType(vert2) == KTYPE_AUTO) ? TRUE : FALSE;
		int knots2 = spline2->KnotCount();
		int verts2 = spline2->Verts();
		int segs2 = spline2->Segments();
		// Enlarge the selection sets for the first spline
 		BitArray& vsel = shape->vertSel[poly1];
		BitArray& ssel = shape->segSel[poly1];
 		BitArray& vsel2 = shape->vertSel[poly2];
		BitArray& ssel2 = shape->segSel[poly2];
		
		// Reorder the splines if necessary -- We set them up so that poly 1 is first,
		// ordered so that its connection point is the last knot.  Then we order poly
		// 2 so that its connection point is the first knot.  We then copy the invec
		// of poly 1's last knot to the invec of poly 2's first knot, delete poly 1's
		// last knot and append poly 2 to poly 1.  We then delete poly 2.

		if(vert1 == 0) {
			spline->Reverse();
			vsel.Reverse();
			ssel.Reverse();
			}
		if(vert2 != 0) {
			spline2->Reverse();
			vsel2.Reverse();
			ssel2.Reverse();
			}
		
		int lastVert = knots - 1;
		Point3 p1 = spline->GetKnotPoint(lastVert);
		Point3 p2 = spline2->GetKnotPoint(0);
		Point3 midpoint = (p1 + p2) / 2.0f;
		Point3 p1Delta = midpoint - p1;
		Point3 p2Delta = midpoint - p2;
		if(r3Way)	// TH 4/13/99 -- Make sure it's backward compatible
			spline2->SetKnotPoint(0, midpoint);
		else
			spline->SetKnotPoint(lastVert, midpoint);
		if(!bothAuto) {
			spline2->SetKnotType(0, KTYPE_BEZIER_CORNER);
			spline2->SetInVec(0, spline->GetInVec(lastVert) + p1Delta);
			spline2->SetOutVec(0, spline2->GetOutVec(0) + p2Delta);
			}
		spline->DeleteKnot(lastVert);
		BOOL welded = spline->Append(spline2);

		// Fix up the selection sets
#ifndef DESIGN_VER
		int base = verts - 3;
		vsel.SetSize(spline->Verts(), 1);
		vsel.Set(base+1);
		for(int i = welded ? 6 : 3; i < verts2; ++i)
			vsel.Set(base+i, vsel2[i]);
		base = segs;
		ssel.SetSize(spline->Segments(), 1);
		for(i = welded ? 1 : 0; i < segs2; ++i)
			ssel.Set(base+i, ssel2[i]);
#else
		int new_verts = spline->Verts();
		vsel.SetSize(new_verts, 1);
		vsel.Set(verts - 2);
		for(int i = verts; i < new_verts; ++i)
			vsel.Set(i, vsel2[i-verts+3]);

		int new_segs = spline->Segments();
		ssel.SetSize(new_segs, 1);
		for(i = segs; i < new_segs; ++i)
			ssel.Set(i, ssel2[i-segs]);
#endif

		// Compute bezier handles and get rid of the attachee
		spline->ComputeBezPoints();
		shape->DeleteSpline(poly2);
//3-31-99 watje
//		shape->UpdateBindList();
		}
	}

BOOL PolyEndAttachRecord::Redo(BezierShape *shape,int reRecord) {
	if(!IsCompatible(shape, poly1, &oldVSel1, &oldSSel1))
		return FALSE;
	if(reRecord) {
		oldSpline1 = *(shape->splines[poly1]);
		oldVSel1 = shape->vertSel[poly1];
		oldSSel1 = shape->segSel[poly1];
		if(poly1 != poly2) {
			oldSpline2 = *(shape->splines[poly2]);
			oldVSel2 = shape->vertSel[poly2];
			oldSSel2 = shape->segSel[poly2];
			selected2 = shape->polySel[poly2];
			}
		}
	DoPolyEndAttach(shape, poly1, vert1, poly2, vert2, FALSE);
	return TRUE;
	}

#define PEAR_GENERAL_CHUNK		0x1000
#define PEAR_OLDVSEL1_CHUNK	0x1040
#define PEAR_OLDSSEL1_CHUNK	0x1050
#define PEAR_SPLINE1_CHUNK		0x1060
#define PEAR_OLDVSEL2_CHUNK	0x1070
#define PEAR_OLDSSEL2_CHUNK	0x1080
#define PEAR_SPLINE2_CHUNK		0x1090

IOResult PolyEndAttachRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case PEAR_GENERAL_CHUNK:
				res = iload->Read(&poly1,sizeof(int),&nb);
				res = iload->Read(&vert1,sizeof(int),&nb);
				res = iload->Read(&poly2,sizeof(int),&nb);
				res = iload->Read(&vert2,sizeof(int),&nb);
				res = iload->Read(&selected2,sizeof(int),&nb);
				break;
			case PEAR_OLDVSEL1_CHUNK:
				res = oldVSel1.Load(iload);
				break;
			case PEAR_OLDSSEL1_CHUNK:
				res = oldSSel1.Load(iload);
				break;
			case PEAR_SPLINE1_CHUNK:
				res = oldSpline1.Load(iload);
				break;
			case PEAR_OLDVSEL2_CHUNK:
				res = oldVSel2.Load(iload);
				break;
			case PEAR_OLDSSEL2_CHUNK:
				res = oldSSel2.Load(iload);
				break;
			case PEAR_SPLINE2_CHUNK:
				res = oldSpline2.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

#define CURVELENGTHSTEPS 5
static float CurveLength(Spline3D *spline, int knot, float v1, float v2, float size = 0.0f) {
	float len = 0.0f;
	if(size == 0.0f) {	// Simple curve length
		Point3 p1,p2;
		p1 = spline->InterpBezier3D(knot, v1);
		float step = (v2 - v1) / CURVELENGTHSTEPS;
		float pos;
		int i;
		for(i = 1, pos = step; i < CURVELENGTHSTEPS; ++i, pos += step) {
			p2 = spline->InterpBezier3D(knot, v1+pos);
			len += Length(p2 - p1);
			p1 = p2;
			}
		len += Length(spline->InterpBezier3D(knot, v2) - p1);
		}
	else {	// Need to figure based on displaced location
		int knots = spline->KnotCount();
		int prev = (knot + knots - 1) % knots;
		int next = (knot + 1) % knots;
		float pv = v1 - 0.01f;
		int pk = knot;
		if(pv < 0.0f) {
			if(spline->Closed()) {
				pv += 1.0f;
				pk = prev;
				}
			else
				pv = 0.0f;
			}
		float nv = v1 + 0.01f;
		Point3 direction = Normalize(spline->InterpBezier3D(knot, nv) - spline->InterpBezier3D(pk, pv));
		direction.z = 0.0f;	// Keep it in the XY plane
		Point3 perp = Point3(direction.y * size, -direction.x * size, 0.0f);

		Point3 p1,p2;
		p1 = spline->InterpBezier3D(knot, v1) + perp;	// Got 1st displaced point

		float step = (v2 - v1) / CURVELENGTHSTEPS;
		float pos;
		int i;
		for(i = 1, pos = step; i < CURVELENGTHSTEPS; ++i, pos += step) {
			pv = v1 + pos - 0.01f;
			nv = v1 + pos + 0.01f;
			direction = Normalize(spline->InterpBezier3D(knot, nv) - spline->InterpBezier3D(knot, pv));
			direction.z = 0.0f;	// Keep it in the XY plane
			perp = Point3(direction.y * size, -direction.x * size, 0.0f);

			p2 = spline->InterpBezier3D(knot, v1+pos) + perp;
			len += Length(p2 - p1);
			p1 = p2;
			}
		pv = v2 - 0.01f;
		int nk = knot;
		nv = v2 + 0.01f;
		if(nv > 1.0f) {
			if(spline->Closed()) {
				nv -= 1.0f;
				nk = next;
				}
			else
				nv = 1.0f;
			}
		direction = Normalize(spline->InterpBezier3D(nk, nv) - spline->InterpBezier3D(knot, pv));
		direction.z = 0.0f;	// Keep it in the XY plane
		perp = Point3(direction.y * size, -direction.x * size, 0.0f);

		len += Length((spline->InterpBezier3D(knot, v2) + perp) - p1);
		}
	return len;
	}

static
void OutlineSpline(BezierShape *shape, int poly, float size, int centered, BOOL newType) {
	// IMPORTANT: The 'else' case to 'if(newType)' must be left as is!  This is code
	// which operates only on modification records brought in from release 1.x of MAX.
	// The 'newType' code operates on MAX 2.0 and later.
	if(newType) {
		Spline3D *inSpline = shape->splines[poly];
		Spline3D outSpline;
		// Do some basic calculations that we'll need regardless
		float size1 = (centered) ? size / 2.0f : 0.0f;	// First phase offset
		float size2 = (centered) ? -size / 2.0f : -size;	// Second phase offset
		int knots = inSpline->KnotCount();
		Point3 knot, in, out;
		int i;
		Matrix3 theMatrix;

		// If the input spline is closed, we wind up with two polygons
		if(inSpline->Closed()) {
			Spline3D *outSpline2 = shape->NewSpline();
			// Generate the outline polygons...
			for(i = 0; i < knots; ++i) {
				int prevKnot = (i + knots - 1) % knots;
				float oldInLength = CurveLength(inSpline, prevKnot, 0.5f, 1.0f);
				float oldOutLength = CurveLength(inSpline, i, 0.0f, 0.5f);
				int knotType = inSpline->GetKnotType(i);
				// Determine the angle of the curve at this knot
				// Get vector from interp before knot to interp after knot
				Point3 ko = inSpline->GetKnotPoint(i);
				Point3 bVec = Normalize(inSpline->InterpBezier3D(prevKnot, 0.99f) - ko);
				Point3 fVec = Normalize(inSpline->InterpBezier3D(i, 0.01f) - ko);
				Point3 direction = Normalize(fVec - bVec);
				direction.z = 0.0f;	// Keep it in the XY plane
				// Figure the size multiplier for the crotch angle
				float dot = DotProd(bVec, fVec);
				float angle, wsize1, wsize2;
				if(dot >= -0.9999939f)
					angle = (float)-acos(dot) / 2.0f;
				else
					angle = HALFPI;
				float base1 = size1 / (float)tan(angle);
				float sign1 = (size1 < 0.0f) ? -1.0f : 1.0f;
				wsize1 = (float)sqrt(base1 * base1 + size1 * size1) * sign1;
				float base2 = size2 / (float)tan(angle);
				float sign2 = (size2 < 0.0f) ? -1.0f : 1.0f;
				wsize2 = (float)sqrt(base2 * base2 + size2 * size2) * sign2;

				Point3 perp(direction.y * wsize1, -direction.x * wsize1, 0.0f);
				float newInLength = CurveLength(inSpline, prevKnot, 0.5f, 1.0f, size1);
				float newOutLength = CurveLength(inSpline, i, 0.0f, 0.5f, size1);
				Point3 kn = ko + perp;
				float inMult = newInLength / oldInLength;
				float outMult = newOutLength / oldOutLength;
				SplineKnot k(knotType, LTYPE_CURVE,
					kn, kn + (inSpline->GetInVec(i) - ko) * inMult, kn + (inSpline->GetOutVec(i) - ko) * outMult);
				outSpline.AddKnot(k);
				perp = Point3(direction.y * wsize2, -direction.x * wsize2, 0.0f);
				newInLength = CurveLength(inSpline, prevKnot, 0.5f, 1.0f, size2);
				newOutLength = CurveLength(inSpline, i, 0.0f, 0.5f, size2);
				kn = ko + perp;
				inMult = newInLength / oldInLength;
				outMult = newOutLength / oldOutLength;
				k = SplineKnot(knotType, LTYPE_CURVE,
					kn, kn + (inSpline->GetInVec(i) - ko) * inMult, kn + (inSpline->GetOutVec(i) - ko) * outMult);
				outSpline2->AddKnot(k);
				}
			outSpline.SetClosed();
			outSpline.ComputeBezPoints();
			*inSpline = outSpline;
			outSpline2->SetClosed();
			outSpline2->ComputeBezPoints();
			shape->vertSel.Insert(shape->SplineCount() - 1, knots * 3);
			shape->segSel.Insert(shape->SplineCount() - 1, knots);
			shape->polySel.Insert(shape->SplineCount() - 1);
			}
		else {	// Otherwise, we get one closed polygon
			// Generate the outline polygon...
			for(i = 0; i < knots; ++i) {
				// Determine the angle of the curve at this knot
				// Get vector from interp before knot to interp after knot
				Point3 direction;
				Point3 ko = inSpline->GetKnotPoint(i);
				float oldInLength = (i == 0) ? 1.0f : CurveLength(inSpline, i - 1, 0.5f, 1.0f);
				float oldOutLength = (i == (knots - 1)) ? 1.0f : CurveLength(inSpline, i, 0.0f, 0.5f);
				float wsize1;
				if(i == 0) {
					direction = Normalize(inSpline->InterpBezier3D(i, 0.01f) - ko);
					wsize1 = size1;
					}
				else
				if(i == (knots - 1)) {
					direction = Normalize(ko - inSpline->InterpBezier3D(i-1, 0.99f));
					wsize1=size1;
					}
				else {
					Point3 bVec = Normalize(inSpline->InterpBezier3D(i-1, 0.99f) - ko);
					Point3 fVec = Normalize(inSpline->InterpBezier3D(i, 0.01f) - ko);
					direction = Normalize(fVec - bVec);
					// Figure the size multiplier for the crotch angle
					float dot = DotProd(bVec, fVec);
					if(dot >= -0.9999939f) {
						float angle = (float)-acos(dot) / 2.0f;
						float base1 = size1 / (float)tan(angle);
						float sign1 = (size1 < 0.0f) ? -1.0f : 1.0f;
						wsize1 = (float)sqrt(base1 * base1 + size1 * size1) * sign1;
						}
					}
				direction.z = 0.0f;	// Keep it in the XY plane
				Point3 perp(direction.y * wsize1, -direction.x * wsize1, 0.0f);
				float newInLength = (i == 0) ? 1.0f : CurveLength(inSpline, i - 1, 0.5f, 1.0f, size1);
				float newOutLength = (i == (knots - 1)) ? 1.0f : CurveLength(inSpline, i, 0.0f, 0.5f, size1);
				float inMult = newInLength / oldInLength;
				float outMult = newOutLength / oldOutLength;
				int knotType = inSpline->GetKnotType(i);
				Point3 kn = ko + perp;
				SplineKnot k((i==0 || i==(knots-1)) ? KTYPE_BEZIER_CORNER : knotType, LTYPE_CURVE,
					kn, kn + (inSpline->GetInVec(i) - ko) * inMult, kn + (inSpline->GetOutVec(i) - ko) * outMult);
				outSpline.AddKnot(k);

				}
			for(i = knots - 1; i >= 0; --i) {
				// Determine the angle of the curve at this knot
				// Get vector from interp before knot to interp after knot
				Point3 direction;
				Point3 ko = inSpline->GetKnotPoint(i);
				float oldInLength = (i == 0) ? 1.0f : CurveLength(inSpline, i - 1, 0.5f, 1.0f);
				float oldOutLength = (i == (knots - 1)) ? 1.0f : CurveLength(inSpline, i, 0.0f, 0.5f);
				float wsize2;
				if(i == 0) {
					direction = Normalize(inSpline->InterpBezier3D(i, 0.01f) - ko);
					wsize2 = size2;
					}
				else
				if(i == (knots - 1)) {
					direction = Normalize(ko - inSpline->InterpBezier3D(i-1, 0.99f));
					wsize2 = size2;
					}
				else {
					Point3 bVec = Normalize(inSpline->InterpBezier3D(i-1, 0.99f) - ko);
					Point3 fVec = Normalize(inSpline->InterpBezier3D(i, 0.01f) - ko);
					direction = Normalize(fVec - bVec);
					// Figure the size multiplier for the crotch angle
					float dot = DotProd(bVec, fVec);
					if(dot >= -0.9999939f) {
						float angle = (float)-acos(dot) / 2.0f;
						float base2 = size2 / (float)tan(angle);
						float sign2 = (size2 < 0.0f) ? -1.0f : 1.0f;
						wsize2 = (float)sqrt(base2 * base2 + size2 * size2) * sign2;
						}
					}
				direction.z = 0.0f;	// Keep it in the XY plane
				Point3 perp(direction.y * wsize2, -direction.x * wsize2, 0.0f);
				float newInLength = (i == 0) ? 1.0f : CurveLength(inSpline, i - 1, 0.5f, 1.0f, size2);
				float newOutLength = (i == (knots - 1)) ? 1.0f : CurveLength(inSpline, i, 0.0f, 0.5f, size2);
				float inMult = newInLength / oldInLength;
				float outMult = newOutLength / oldOutLength;
				int knotType = inSpline->GetKnotType(i);
				Point3 kn = ko + perp;
				SplineKnot k((i==0 || i==(knots-1)) ? KTYPE_BEZIER_CORNER : knotType, LTYPE_CURVE,
					kn, kn + (inSpline->GetOutVec(i) - ko) * outMult, kn + (inSpline->GetInVec(i) - ko) * inMult);
				outSpline.AddKnot(k);
				}
			int lastPt = outSpline.KnotCount() - 1;
			outSpline.SetInVec(0, outSpline.GetKnotPoint(0));
			outSpline.SetOutVec(lastPt, outSpline.GetKnotPoint(lastPt));
			outSpline.SetInVec(knots, outSpline.GetKnotPoint(knots));
			outSpline.SetOutVec(knots - 1, outSpline.GetKnotPoint(knots - 1));
			outSpline.SetClosed();
			outSpline.ComputeBezPoints();
			*inSpline = outSpline;
			// Adjust selection data for this spline
			shape->vertSel[poly].SetSize(outSpline.Verts(),1);
			shape->segSel[poly].SetSize(outSpline.Segments(),1);
			}
		}
	else {
		Spline3D *inSpline = shape->splines[poly];
		Spline3D outSpline;
		// Do some basic calculations that we'll need regardless
		float size1 = (centered) ? size / 2.0f : 0.0f;	// First phase offset
		float size2 = (centered) ? -size / 2.0f : -size;	// Second phase offset
		int knots = inSpline->KnotCount();
		Point3 knot, in, out;
		int i;
		Matrix3 theMatrix;

		// If the input spline is closed, we wind up with two polygons
		if(inSpline->Closed()) {
			Spline3D *outSpline2 = shape->NewSpline();
			// Generate the outline polygons...
			for(i = 0; i < knots; ++i) {
				int prevKnot = (i + knots - 1) % knots;
				float oldInLength = CurveLength(inSpline, prevKnot, 0.5f, 1.0f);
				float oldOutLength = CurveLength(inSpline, i, 0.0f, 0.5f);
				int knotType = inSpline->GetKnotType(i);
				// Determine the angle of the curve at this knot
				// Get vector from interp before knot to interp after knot
				Point3 ko = inSpline->GetKnotPoint(i);
				Point3 bVec = Normalize(inSpline->InterpBezier3D(prevKnot, 0.99f) - ko);
				Point3 fVec = Normalize(inSpline->InterpBezier3D(i, 0.01f) - ko);
				Point3 direction = Normalize(fVec - bVec);
				direction.z = 0.0f;	// Keep it in the XY plane
			// Calculate the offset size differently if this knot is a corner.
			// Applies to both Corner and BezCorner.
				Point3 perp;
				perp = Point3(direction.y * size1, -direction.x * size1, 0.0f);
				float newInLength = CurveLength(inSpline, prevKnot, 0.5f, 1.0f, size1);
				float newOutLength = CurveLength(inSpline, i, 0.0f, 0.5f, size1);
				Point3 kn = ko + perp;
				float inMult = newInLength / oldInLength;
				float outMult = newOutLength / oldOutLength;
				SplineKnot k(knotType, LTYPE_CURVE,
					kn, kn + (inSpline->GetInVec(i) - ko) * inMult, kn + (inSpline->GetOutVec(i) - ko) * outMult);
				outSpline.AddKnot(k);
				perp = Point3(direction.y * size2, -direction.x * size2, 0.0f);
				newInLength = CurveLength(inSpline, prevKnot, 0.5f, 1.0f, size2);
				newOutLength = CurveLength(inSpline, i, 0.0f, 0.5f, size2);
				kn = ko + perp;
				inMult = newInLength / oldInLength;
				outMult = newOutLength / oldOutLength;
				k = SplineKnot(knotType, LTYPE_CURVE,
					kn, kn + (inSpline->GetInVec(i) - ko) * inMult, kn + (inSpline->GetOutVec(i) - ko) * outMult);
				outSpline2->AddKnot(k);
				}
			outSpline.SetClosed();
			outSpline.ComputeBezPoints();
			*inSpline = outSpline;
			outSpline2->SetClosed();
			outSpline2->ComputeBezPoints();
			shape->vertSel.Insert(shape->splineCount - 1, knots * 3);
			shape->segSel.Insert(shape->splineCount - 1, knots);
			shape->polySel.Insert(shape->splineCount - 1);
			}
		else {	// Otherwise, we get one closed polygon
			// Now get the spline selection sets
			BitArray& vsel = shape->vertSel[poly];
			BitArray& ssel = shape->segSel[poly];
			
			// Generate the outline polygon...
			for(i = 0; i < knots; ++i) {
				// Determine the angle of the curve at this knot
				// Get vector from interp before knot to interp after knot
				Point3 direction;
				Point3 ko = inSpline->GetKnotPoint(i);
				float oldInLength = (i == 0) ? 1.0f : CurveLength(inSpline, i - 1, 0.5f, 1.0f);
				float oldOutLength = (i == (knots - 1)) ? 1.0f : CurveLength(inSpline, i, 0.0f, 0.5f);
				if(i == 0)
					direction = Normalize(inSpline->InterpBezier3D(i, 0.01f) - ko);
				else
				if(i == (knots - 1))
					direction = Normalize(ko - inSpline->InterpBezier3D(i-1, 0.99f));
				else {
					Point3 bVec = Normalize(inSpline->InterpBezier3D(i-1, 0.99f) - ko);
					Point3 fVec = Normalize(inSpline->InterpBezier3D(i, 0.01f) - ko);
					direction = Normalize(fVec - bVec);
					}
				direction.z = 0.0f;	// Keep it in the XY plane
				Point3 perp(direction.y * size1, -direction.x * size1, 0.0f);
				float newInLength = (i == 0) ? 1.0f : CurveLength(inSpline, i - 1, 0.5f, 1.0f, size1);
				float newOutLength = (i == (knots - 1)) ? 1.0f : CurveLength(inSpline, i, 0.0f, 0.5f, size1);
				float inMult = newInLength / oldInLength;
				float outMult = newOutLength / oldOutLength;
				int knotType = inSpline->GetKnotType(i);
				Point3 kn = ko + perp;
				SplineKnot k((i==0 || i==(knots-1)) ? KTYPE_BEZIER_CORNER : knotType, LTYPE_CURVE,
					kn, kn + (inSpline->GetInVec(i) - ko) * inMult, kn + (inSpline->GetOutVec(i) - ko) * outMult);
				outSpline.AddKnot(k);
				}
			for(i = knots - 1; i >= 0; --i) {
				// Determine the angle of the curve at this knot
				// Get vector from interp before knot to interp after knot
				Point3 direction;
				Point3 ko = inSpline->GetKnotPoint(i);
				float oldInLength = (i == 0) ? 1.0f : CurveLength(inSpline, i - 1, 0.5f, 1.0f);
				float oldOutLength = (i == (knots - 1)) ? 1.0f : CurveLength(inSpline, i, 0.0f, 0.5f);
				if(i == 0)
					direction = Normalize(inSpline->InterpBezier3D(i, 0.01f) - ko);
				else
				if(i == (knots - 1))
					direction = Normalize(ko - inSpline->InterpBezier3D(i-1, 0.99f));
				else {
					Point3 bVec = Normalize(inSpline->InterpBezier3D(i-1, 0.99f) - ko);
					Point3 fVec = Normalize(inSpline->InterpBezier3D(i, 0.01f) - ko);
					direction = Normalize(fVec - bVec);
					}
				direction.z = 0.0f;	// Keep it in the XY plane
				Point3 perp(direction.y * size2, -direction.x * size2, 0.0f);
				float newInLength = (i == 0) ? 1.0f : CurveLength(inSpline, i - 1, 0.5f, 1.0f, size2);
				float newOutLength = (i == (knots - 1)) ? 1.0f : CurveLength(inSpline, i, 0.0f, 0.5f, size2);
				float inMult = newInLength / oldInLength;
				float outMult = newOutLength / oldOutLength;
				int knotType = inSpline->GetKnotType(i);
				Point3 kn = ko + perp;
				SplineKnot k((i==0 || i==(knots-1)) ? KTYPE_BEZIER_CORNER : knotType, LTYPE_CURVE,
					kn, kn + (inSpline->GetOutVec(i) - ko) * outMult, kn + (inSpline->GetInVec(i) - ko) * inMult);
				outSpline.AddKnot(k);
				}
			int lastPt = outSpline.KnotCount() - 1;
			outSpline.SetInVec(0, outSpline.GetKnotPoint(0));
			outSpline.SetOutVec(lastPt, outSpline.GetKnotPoint(lastPt));
			outSpline.SetInVec(knots, outSpline.GetKnotPoint(knots));
			outSpline.SetOutVec(knots - 1, outSpline.GetKnotPoint(knots - 1));
			outSpline.SetClosed();
			outSpline.ComputeBezPoints();
			*inSpline = outSpline;
			// Adjust selection data for this spline
			vsel.SetSize(inSpline->Verts(),1);
			ssel.SetSize(inSpline->Segments(),1);
			}
		}
//3-31-99 watje
//	shape->UpdateBindList();
	shape->InvalidateGeomCache();
	}

/* Find the vector length for a circle segment	*/
/* Returns a unit value (radius=1.0)		*/
/* Angle expressed in radians			*/

static float
veccalc(float angstep) {
	static float lastin = -9999.0f,lastout;
	if(lastin == angstep)
		return lastout;

	float lo,hi,totdist;
	float sinfac=(float)sin(angstep),cosfac=(float)cos(angstep),test;
	int ix,count;
	Spline3D work;
	Point3 k1((float)cos(0.0f),(float)sin(0.0f),0.0f);
	Point3 k2(cosfac,sinfac,0.0f);

	hi=1.5f;
	lo=0.0f;
	count=200;

	/* Loop thru test vectors */

	loop:
	work.NewSpline();
	test=(hi+lo)/2.0f;
	Point3 out = k1 + Point3(0.0f, test, 0.0f);
	Point3 in = k2 + Point3(sinfac * test, -cosfac * test, 0.0f);

 	work.AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_CURVE,k1,k1,out));
 	work.AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_CURVE,k2,in,k2));

	totdist=0.0f;
	for(ix=0; ix<10; ++ix) {
		Point3 terp = work.InterpBezier3D(0,(float)ix/10.0f);
		totdist += (float)sqrt(terp.x * terp.x + terp.y * terp.y);
		}
	
	totdist /= 10.0f;
	count--;
	if(totdist==1.0f || count<=0)
		goto done;
	if(totdist>1.0f) {
		hi=test;
		goto loop;
		}
	lo=test;
	goto loop;

	done:
	lastin = angstep;
	lastout = test;
	return test;
	}

#define ES_FILLET 0
#define ES_CHAMFER 1

static void
FilletOrChamferSpline(BezierShape *shape, int poly, float size, int which) {
	if(size <= 0.0f)
		return;
	Spline3D *inSpline = shape->splines[poly];
	Spline3D outSpline = *inSpline;
	BitArray map = KnotSelFromVertSel(shape->vertSel[poly]);
	// Don't fillet or chamfer ends of open spline!
	if(!outSpline.Closed()) {
		map.Clear(0);
		map.Clear(outSpline.KnotCount() - 1);
		}
	// If nothing to do, exit
	if(!map.NumberSet())
		return;
	for(int i = outSpline.KnotCount()-1; i >= 0; --i) {
		if(map[i]) {
			int prev = (i + outSpline.KnotCount() - 1) % outSpline.KnotCount();
			float ilen = outSpline.SegmentLength(i);
			float prevlen = outSpline.SegmentLength(prev);
			outSpline.RefineSegment(i, size/ilen, SPLINE_INTERP_NORMALIZED);
			outSpline.RefineSegment((i < prev) ? prev+1 : prev, (prevlen - size)/prevlen, SPLINE_INTERP_NORMALIZED);
			outSpline.DeleteKnot((i > 0) ? i+1 : i);
			int k2 = (i > prev) ? i+1 : i;
			int k1 = prev + 1;
			if(which == ES_CHAMFER) {
				outSpline.SetKnotType(k1, KTYPE_BEZIER_CORNER);
				outSpline.SetKnotType(k2, KTYPE_BEZIER_CORNER);
				}
			Point3 k1p = outSpline.GetKnotPoint(k1);
			Point3 k2p = outSpline.GetKnotPoint(k2);
			outSpline.SetRelOutVec(k1, (k2p - k1p) / 3.0f);
			outSpline.SetRelInVec(k2, (k1p - k2p) / 3.0f);
			if(which == ES_FILLET) {
				Point3 ain = Normalize(outSpline.GetRelInVec(k1));
				Point3 aout = Normalize(outSpline.GetRelOutVec(k1));
				float dot1 = DotProd(ain, aout);	//TH 3/3/00
				Point3 bin = Normalize(outSpline.GetRelInVec(k2));
				Point3 bout = Normalize(outSpline.GetRelOutVec(k2));
				float dot2 = DotProd(bin, bout);	//TH 3/3/00
				if(dot1 > -1.0f && dot2 > -1.0f) {	//TH 3/3/00
					float a1 = (float)fabs(-acos(dot1)) - HALFPI;
					float a2 = (float)fabs(-acos(dot2)) - HALFPI;
					// If both angles are < 179 degrees, continue!
					if(a1 < 3.124f && a2 < 3.124f) {
						float a3 = PI - a1 - a2;
						float ab = Length(k1p - k2p);
						float ax = (ab * (float)sin(a2)) / (float)sin(a3);
						float bx = (ax * (float)sin(a1)) / (float)sin(a2);
						float vector = veccalc(a3);
						outSpline.SetRelOutVec(k1, -ain * (vector * bx));
						outSpline.SetRelInVec(k2, -bout * (vector * ax));
						}
					}
				}
			}
		}
	*inSpline = outSpline;
	// Adjust selection data for this spline
	shape->vertSel[poly].SetSize(outSpline.Verts(),1);
	shape->segSel[poly].SetSize(outSpline.Segments(),1);
//3-31-99 watje
//	shape->UpdateBindList();
	shape->InvalidateGeomCache();
	}

BOOL OutlineRecord::Redo(BezierShape *shape, int reRecord) {
	if(poly >= shape->splineCount)
		return FALSE;
	if(reRecord) {
		oldSpline = *(shape->splines[poly]);
		oldVSel = shape->vertSel[poly];
		oldSSel = shape->segSel[poly];
		oldSplineCount = shape->splineCount;
		}
	OutlineSpline(shape, poly, size, centered, newType);
	return TRUE;
	}

#define POR_GENERAL_CHUNK		0x1000
#define POR_OLDVSEL_CHUNK		0x1040
#define POR_OLDSSEL_CHUNK		0x1050
#define POR_SPLINE_CHUNK		0x1060
#define POR_NEWTYPE_CHUNK		0x1070

IOResult OutlineRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	newType = FALSE;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case POR_GENERAL_CHUNK:
				res = iload->Read(&poly,sizeof(int),&nb);
				res = iload->Read(&centered,sizeof(int),&nb);
				res = iload->Read(&size,sizeof(float),&nb);
				res = iload->Read(&oldSplineCount,sizeof(int),&nb);
				break;
			case POR_OLDVSEL_CHUNK:
				res = oldVSel.Load(iload);
				break;
			case POR_OLDSSEL_CHUNK:
				res = oldSSel.Load(iload);
				break;
			case POR_SPLINE_CHUNK:
				res = oldSpline.Load(iload);
				break;
			// If the following chunk is present, it's a MAX 2.0 file and the outlining 
			// is done using a fixed algorithm.  Otherwise, it uses the R1.x code, preserving
			// the shape as it was.
			case POR_NEWTYPE_CHUNK:
				newType = TRUE;
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL PolyDetachRecord::Redo(BezierShape *shape,int reRecord) {
	if(poly >= shape->splineCount)
		return FALSE;
	if(reRecord && !copy) {
		spline = *(shape->splines[poly]);
		oldVSel = shape->vertSel[poly];
		oldSSel = shape->segSel[poly];
		}
	if(!copy)
		shape->DeleteSpline(poly);
	return TRUE;
	}

#define PDETR_GENERAL_CHUNK		0x1000
#define PDETR_OLDVSEL_CHUNK		0x1040
#define PDETR_OLDSSEL_CHUNK		0x1050
#define PDETR_SPLINE_CHUNK		0x1060

IOResult PolyDetachRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case PDETR_GENERAL_CHUNK:
				res = iload->Read(&poly,sizeof(int),&nb);
				res = iload->Read(&copy,sizeof(int),&nb);
				break;
			case PDETR_OLDVSEL_CHUNK:
				res = oldVSel.Load(iload);
				break;
			case PDETR_OLDSSEL_CHUNK:
				res = oldSSel.Load(iload);
				break;
			case PDETR_SPLINE_CHUNK:
				res = spline.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL PolyDeleteRecord::Redo(BezierShape *shape,int reRecord) {
	if(poly >= shape->splineCount)
		return FALSE;
	if(reRecord) {
		spline = *(shape->splines[poly]);
		oldVSel = shape->vertSel[poly];
		oldSSel = shape->segSel[poly];
		}
	shape->DeleteSpline(poly);
	return TRUE;
	}

#define PDELR_GENERAL_CHUNK		0x1000
#define PDELR_OLDVSEL_CHUNK		0x1040
#define PDELR_OLDSSEL_CHUNK		0x1050
#define PDELR_SPLINE_CHUNK		0x1060

IOResult PolyDeleteRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case PDELR_GENERAL_CHUNK:
				res = iload->Read(&poly,sizeof(int),&nb);
				break;
			case PDELR_OLDVSEL_CHUNK:
				res = oldVSel.Load(iload);
				break;
			case PDELR_OLDSSEL_CHUNK:
				res = oldSSel.Load(iload);
				break;
			case PDELR_SPLINE_CHUNK:
				res = spline.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL VertMoveRecord::Redo(BezierShape *shape,int reRecord) {
	if(!delta.IsCompatible(*shape))
		return FALSE;
	delta.Apply(*shape);
	return TRUE;
	}

#define VMR_DELTA_CHUNK		0x1000

IOResult VertMoveRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case VMR_DELTA_CHUNK:
				res = delta.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}


BOOL SegDeleteRecord::Redo(BezierShape *shape,int reRecord) {
	if(poly >= shape->splineCount)
		return FALSE;
	if(reRecord) {
		oldSpline = *(shape->splines[poly]);
		oldVSel = shape->vertSel[poly];
		oldSSel = shape->segSel[poly];
		selected = shape->polySel[poly];
		}
	DeleteSelSegs(shape, poly);
	return TRUE;
	}

#define SDELR_GENERAL_CHUNK		0x1000
#define SDELR_OLDVSEL_CHUNK		0x1040
#define SDELR_OLDSSEL_CHUNK		0x1050
#define SDELR_SPLINE_CHUNK		0x1060

IOResult SegDeleteRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case SDELR_GENERAL_CHUNK:
				res = iload->Read(&poly,sizeof(int),&nb);
				res = iload->Read(&selected,sizeof(int),&nb);
				res = iload->Read(&deleted,sizeof(int),&nb);
				res = iload->Read(&oldSplineCount,sizeof(int),&nb);
				break;
			case SDELR_OLDVSEL_CHUNK:
				res = oldVSel.Load(iload);
				break;
			case SDELR_OLDSSEL_CHUNK:
				res = oldSSel.Load(iload);
				break;
			case SDELR_SPLINE_CHUNK:
				res = oldSpline.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		


BOOL SegDetachRecord::Redo(BezierShape *shape,int reRecord) {
	if(poly >= shape->splineCount)
		return FALSE;
	if(reRecord && !copy) {
		oldSpline = *(shape->splines[poly]);
		oldVSel = shape->vertSel[poly];
		oldSSel = shape->segSel[poly];
		selected = shape->polySel[poly];
		}
	if(!copy)
		DeleteSelSegs(shape, poly);
	return TRUE;
	}

#define SDETR_GENERAL_CHUNK		0x1000
#define SDETR_OLDVSEL_CHUNK		0x1040
#define SDETR_OLDSSEL_CHUNK		0x1050
#define SDETR_SPLINE_CHUNK		0x1060

IOResult SegDetachRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case SDETR_GENERAL_CHUNK:
				res = iload->Read(&poly,sizeof(int),&nb);
				res = iload->Read(&copy,sizeof(int),&nb);
				res = iload->Read(&selected,sizeof(int),&nb);
				res = iload->Read(&deleted,sizeof(int),&nb);
				res = iload->Read(&oldSplineCount,sizeof(int),&nb);
				break;
			case SDETR_OLDVSEL_CHUNK:
				res = oldVSel.Load(iload);
				break;
			case SDETR_OLDSSEL_CHUNK:
				res = oldSSel.Load(iload);
				break;
			case SDETR_SPLINE_CHUNK:
				res = oldSpline.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		


static void BreakSegment(BezierShape *shape, int poly, int segment, float param) {
	Spline3D *spline = shape->splines[poly];
	int knots = spline->KnotCount();
	int verts = spline->Verts();
	int segs = spline->Segments();
	int nextSeg = (segment + 1) % knots;

	Point3 point = spline->InterpBezier3D(segment, param);
	
	Point3 v00 = spline->GetKnotPoint(segment);
	Point3 v30 = spline->GetKnotPoint(nextSeg);
	Point3 v10 = spline->GetOutVec(segment);
	Point3 v20 = spline->GetInVec(nextSeg);
	Point3 v01 = v00 + (v10 - v00) * param;
	Point3 v21 = v20 + (v30 - v20) * param;
	Point3 v11 = v10 + (v20 - v10) * param;
	Point3 v02 = v01 + (v11 - v01) * param;
	Point3 v12 = v11 + (v21 - v11) * param;
	Point3 v03 = v02 + (v12 - v02) * param;

	spline->SetOutVec(segment, v01);
	spline->SetInVec(nextSeg, v21);

	SplineKnot newKnot(KTYPE_BEZIER, LTYPE_CURVE, v03, v02, v12);
	spline->AddKnot(newKnot, nextSeg);
	spline->ComputeBezPoints();

	// Now adjust the spline selection sets
	BitArray& vsel = shape->vertSel[poly];
	BitArray& ssel = shape->segSel[poly];
	vsel.SetSize(verts + 3, 1);
	int where = (segment + 1) * 3;
	vsel.Shift(RIGHT_BITSHIFT, 3, where);
	vsel.Clear(where);
	vsel.Clear(where+1);
	vsel.Clear(where+2);
	ssel.SetSize(segs + 1, 1);
	ssel.Shift(RIGHT_BITSHIFT, 1, segment + 1);
	ssel.Set(segment+1,ssel[segment]);

	// Now break the spline at that vertex!

	knots = spline->KnotCount();
	verts = spline->Verts();

	int k = nextSeg;
	int altered = 0;

	if(1) {
		BitArray& vsel = shape->vertSel[poly];
		BitArray& ssel = shape->segSel[poly];
		int vert = k*3+1;
		if(spline->Closed()) {
			if(k == (knots-1)) {	// Break at end knot
				break_at_last_knot:
				altered = 1;
				SplineKnot dupKnot(spline->GetKnotType(k),spline->GetLineType(k),
					spline->GetKnotPoint(k),spline->GetInVec(k),spline->GetOutVec(k));
				spline->AddKnot(dupKnot, 0);
				knots++;
				verts += 3;
				spline->SetOpen();
				vsel.SetSize(spline->Verts(),1);
				vsel.Shift(RIGHT_BITSHIFT,3);
				vsel.Clear(0);	// New point not selected
				vsel.Clear(1);
				vsel.Clear(2);
				ssel.SetSize(spline->Segments(),1);
				ssel.Shift(RIGHT_BITSHIFT,1);
				ssel.Clear(0);
				}
			else
			if(k == 0) {			// Break at first knot
				altered = 1;
				SplineKnot dupKnot(spline->GetKnotType(0),spline->GetLineType(0),
					spline->GetKnotPoint(0),spline->GetInVec(0),spline->GetOutVec(0));
				spline->AddKnot(dupKnot, -1);
				knots++;
				verts += 3;
				spline->SetOpen();
				vsel.SetSize(spline->Verts(),1);
				vsel.Clear(verts-3);		// New point not selected
				vsel.Clear(verts-2);
				vsel.Clear(verts-1);
				ssel.SetSize(spline->Segments(),1);
				ssel.Clear(knots-1);
				}
			else {					// Break somewhere in the middle
				// First, rotate the spline so that the break point is the last one!
				int rotations = 0;
				int lastKnot = knots - 1;
				while(k < (lastKnot)) {
					SplineKnot dupKnot(spline->GetKnotType(lastKnot),spline->GetLineType(lastKnot),
						spline->GetKnotPoint(lastKnot),spline->GetInVec(lastKnot),spline->GetOutVec(lastKnot));
					spline->DeleteKnot(lastKnot);
					spline->AddKnot(dupKnot, 0);
					rotations++;
					k++;
					}
				vsel.Rotate(RIGHT_BITSHIFT, rotations*3);
				ssel.Rotate(RIGHT_BITSHIFT, rotations);
				k = lastKnot;
				vert = k*3+1;
				goto break_at_last_knot;
				}
			}
		else {
			// Don't do anything if the spline's open and we're breaking at an end vertex!
			if (k == 0 || k == (knots-1)) {
				vsel.Clear(vert);	// Just turn off the selection bit
				goto done;
				}
			int i;
			int newPolySize = knots - k;
			// OK, We're breaking at a point in the middle -- Copy end points off to a new spline, then
			// delete them from the original!
			Spline3D *newSpline = shape->NewSpline();
			for(i = k; i < knots; ++i) {
				SplineKnot dupKnot(spline->GetKnotType(i),spline->GetLineType(i),
					spline->GetKnotPoint(i),spline->GetInVec(i),spline->GetOutVec(i));
				newSpline->AddKnot(dupKnot, -1);
				}
			for(i = knots-1; i > k; --i)
				spline->DeleteKnot(i);
			// Adjust selection data for this spline
			vsel.SetSize(spline->Verts(),1);
			ssel.SetSize(spline->Segments(),1);
			// Don't forget to create a new selection record for this new spline!
			shape->vertSel.Insert(shape->splineCount - 1, newPolySize * 3);
			shape->segSel.Insert(shape->splineCount - 1, newPolySize - 1);
			shape->polySel.Insert(shape->splineCount - 1);
			}
		}
	done:
	if(altered) {
//3-31-99 watje
//		shape->UpdateBindList();
		spline->ComputeBezPoints();
		shape->InvalidateGeomCache();
		}
	}

BOOL SegBreakRecord::Redo(BezierShape *shape,int reRecord) {
	if(poly >= shape->splineCount)
		return FALSE;
	if(reRecord) {
		oldSpline = *(shape->splines[poly]);
		oldVSel = shape->vertSel[poly];
		oldSSel = shape->segSel[poly];
		selected = shape->polySel[poly];
		oldSplineCount = shape->splineCount;
		}
	BreakSegment(shape, poly, segment, param);
	return TRUE;
	}

#define SBRKR_GENERAL_CHUNK		0x1000
#define SBRKR_OLDVSEL_CHUNK		0x1040
#define SBRKR_OLDSSEL_CHUNK		0x1050
#define SBRKR_SPLINE_CHUNK		0x1060

IOResult SegBreakRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case SBRKR_GENERAL_CHUNK:
				res = iload->Read(&poly,sizeof(int),&nb);
				res = iload->Read(&segment,sizeof(int),&nb);
				res = iload->Read(&param,sizeof(float),&nb);
				res = iload->Read(&selected,sizeof(int),&nb);
				res = iload->Read(&oldSplineCount,sizeof(int),&nb);
				break;
			case SBRKR_OLDVSEL_CHUNK:
				res = oldVSel.Load(iload);
				break;
			case SBRKR_OLDSSEL_CHUNK:
				res = oldSSel.Load(iload);
				break;
			case SBRKR_SPLINE_CHUNK:
				res = oldSpline.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

static void RefineSegment(BezierShape *shape, int poly, int segment, float param, BOOL r3Style=FALSE) {
	Spline3D *spline = shape->splines[poly];
	int knots = spline->KnotCount();
	int verts = spline->Verts();
	int segs = spline->Segments();
	int nextSeg = (segment + 1) % knots;
	int insertSeg = (nextSeg == 0) ? -1 : nextSeg;

	// Get the knot points
	Point3 v00 = spline->GetKnotPoint(segment);
	Point3 v30 = spline->GetKnotPoint(nextSeg);

	// Get the knot types
	int type1 = spline->GetKnotType(segment);
	int type2 = spline->GetKnotType(nextSeg);

	// Get the material
	MtlID mtl = spline->GetMatID(segment);

	// Special: If they're refining a line-type segment, force it to be a bezier curve again
	if(spline->GetLineType(segment) == LTYPE_LINE) {
		spline->SetKnotType(segment, KTYPE_BEZIER_CORNER);
		spline->SetKnotType(nextSeg, KTYPE_BEZIER_CORNER);
		spline->SetLineType(segment, LTYPE_CURVE);
		spline->SetOutVec(segment, v00 + (v30 - v00) / 3.0f);
		spline->SetInVec(nextSeg, v30 - (v30 - v00) / 3.0f);
		}

	Point3 point = spline->InterpBezier3D(segment, param);
	
	Point3 v10 = spline->GetOutVec(segment);
	Point3 v20 = spline->GetInVec(nextSeg);
	Point3 v01 = v00 + (v10 - v00) * param;
	Point3 v21 = v20 + (v30 - v20) * param;
	Point3 v11 = v10 + (v20 - v10) * param;
	Point3 v02 = v01 + (v11 - v01) * param;
	Point3 v12 = v11 + (v21 - v11) * param;
	Point3 v03 = v02 + (v12 - v02) * param;

	spline->SetOutVec(segment, v01);
	spline->SetInVec(nextSeg, v21);

	// New for r3: Make the knot type dependent on the bordering knot types
	int newType = KTYPE_BEZIER;
	if(r3Style) {
		if(type1 == KTYPE_CORNER && type2 == KTYPE_CORNER)
			newType = KTYPE_CORNER;
		else
		if((type1 & KTYPE_CORNER) || (type2 & KTYPE_CORNER))
			newType = KTYPE_BEZIER_CORNER;
		else
		if(type1 == KTYPE_AUTO && type2 == KTYPE_AUTO)
			newType = KTYPE_AUTO;
		}
	SplineKnot newKnot(newType, LTYPE_CURVE, v03, v02, v12);
	newKnot.SetMatID(mtl);

	spline->AddKnot(newKnot, insertSeg);

	spline->ComputeBezPoints();
	shape->InvalidateGeomCache();

	// Now adjust the spline selection sets
	BitArray& vsel = shape->vertSel[poly];
	BitArray& ssel = shape->segSel[poly];
	vsel.SetSize(spline->Verts(), 1);
	int where = (segment + 1) * 3;
	vsel.Shift(RIGHT_BITSHIFT, 3, where);
	vsel.Clear(where);
	vsel.Clear(where+1);
	vsel.Clear(where+2);
	ssel.SetSize(spline->Segments(), 1);
	ssel.Shift(RIGHT_BITSHIFT, 1, segment + 1);
	ssel.Set(segment+1,ssel[segment]);
	}

BOOL SegRefineRecord::Redo(BezierShape *shape,int reRecord) {
	if(poly >= shape->splineCount)
		return FALSE;
	if(reRecord) {
		oldSpline = *(shape->splines[poly]);
		oldVSel = shape->vertSel[poly];
		oldSSel = shape->segSel[poly];
		}
	if(segment >= shape->splines[poly]->Segments())
		return FALSE;
	RefineSegment(shape, poly, segment, param);
	return TRUE;
	}

#define SREFR_GENERAL_CHUNK		0x1000
#define SREFR_OLDVSEL_CHUNK		0x1040
#define SREFR_OLDSSEL_CHUNK		0x1050
#define SREFR_SPLINE_CHUNK		0x1060

IOResult SegRefineRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case SREFR_GENERAL_CHUNK:
				res = iload->Read(&poly,sizeof(int),&nb);
				res = iload->Read(&segment,sizeof(int),&nb);
				res = iload->Read(&param,sizeof(float),&nb);
				break;
			case SREFR_OLDVSEL_CHUNK:
				res = oldVSel.Load(iload);
				break;
			case SREFR_OLDSSEL_CHUNK:
				res = oldSSel.Load(iload);
				break;
			case SREFR_SPLINE_CHUNK:
				res = oldSpline.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

#define BEGINNING 0
#define END 1
#define FORWARD 1
#define REVERSE -1

static void ConnectVerts(BezierShape *shape, int poly1, int vert1, int poly2, int vert2, BOOL makeVectors) {
	Spline3D *spline = shape->splines[poly1];
	int knots = spline->KnotCount();
	int verts = spline->Verts();
	int segs = spline->Segments();

	// If connecting endpoints of the same polygon, close it and make the connecting line linear
	if(poly1 == poly2) {
		spline->SetClosed();
		// Make first and last knots beziers
		spline->SetKnotType(0, KTYPE_BEZIER_CORNER);
		int lastKnot = spline->KnotCount() - 1;
		spline->SetKnotType(lastKnot, KTYPE_BEZIER_CORNER);
		spline->SetLineType(lastKnot, LTYPE_CURVE);
		if(makeVectors) {
			Point3 v0 = spline->GetKnotPoint(0);
			Point3 v1 = spline->GetKnotPoint(lastKnot);
			Point3 vec = (v1 - v0) / 3.0f;
			spline->SetInVec(0, v0 + vec);
			spline->SetOutVec(lastKnot, v1 - vec);
			}
		else {
			spline->SetInVec(0, spline->GetKnotPoint(0));
			spline->SetOutVec(lastKnot, spline->GetKnotPoint(lastKnot));
			}
		BitArray& ssel = shape->segSel[poly1];
		ssel.SetSize(spline->Segments(), 1);
		ssel.Set(spline->Segments() - 1, 0);
		}
	else { 		// Connecting two different splines -- Splice 'em together!
		Spline3D *spline2 = shape->splines[poly2];
		int knots2 = spline2->KnotCount();
		int verts2 = spline2->Verts();
		int segs2 = spline2->Segments();
		// Enlarge the selection sets for the first spline
 		BitArray& vsel = shape->vertSel[poly1];
		BitArray& ssel = shape->segSel[poly1];
 		BitArray& vsel2 = shape->vertSel[poly2];
		BitArray& ssel2 = shape->segSel[poly2];

		// Ready the endpoints
		if(makeVectors) {
			Point3 v1, v2, vec;
			if(vert2 == 1)
				v2 = spline2->GetKnotPoint(0);
			else 
				v2 = spline2->GetKnotPoint(knots2 - 1);
			if(vert1 == 1)
				v1 = spline->GetKnotPoint(0);
			else 
				v1 = spline->GetKnotPoint(knots - 1);
			if(vert2 == 1) {
				spline2->SetKnotType(0, KTYPE_BEZIER_CORNER);
				spline2->SetInVec(0, v2 + (v1 - v2) / 3.0f);
				}
			else {
				spline2->SetKnotType(knots2 - 1, KTYPE_BEZIER_CORNER);
				spline2->SetOutVec(knots2 - 1, v2 + (v1 - v2) / 3.0f);
				}
			if(vert1 == 1) {
				spline->SetKnotType(0, KTYPE_BEZIER_CORNER);
				spline->SetInVec(0, v1 + (v2 - v1) / 3.0f);
				}
			else {
				spline->SetKnotType(spline->KnotCount() - 1, KTYPE_BEZIER_CORNER);
				spline->SetOutVec(knots - 1, v1 + (v2 - v1) / 3.0f);
				}
			}
		else {
			if(vert2 == 1) {
				spline2->SetKnotType(0, KTYPE_BEZIER_CORNER);
				spline2->SetInVec(0, spline2->GetKnotPoint(0));
				}
			else {
				spline2->SetKnotType(knots2 - 1, KTYPE_BEZIER_CORNER);
				spline2->SetOutVec(knots2 - 1, spline2->GetKnotPoint(knots2 - 1));
				}
			if(vert1 == 1) {
				spline->SetKnotType(0, KTYPE_BEZIER_CORNER);
				spline->SetInVec(0, spline->GetKnotPoint(0));
				}
			else {
				spline->SetKnotType(spline->KnotCount() - 1, KTYPE_BEZIER_CORNER);
				spline->SetOutVec(knots - 1, spline->GetKnotPoint(knots - 1));
				}
			}
		// Now copy the knots over!
		if(vert2 == 1) {
			if(vert1 == 1) {
				// Forward copy, reversing vectors
				for(int i = 0, first = 1; i < spline2->KnotCount(); ++i, first = 0) {
					SplineKnot k(spline2->GetKnotType(i), (first) ? LTYPE_CURVE : spline2->GetLineType(i - 1),
						spline2->GetKnotPoint(i),spline2->GetOutVec(i), spline2->GetInVec(i));
//watje  216165 6/14/00 since the spline is  flipped need to much around with the hidden point to get them poinitng the rigth way
					if (i != 0)
						{
						if (spline2->GetKnot(i-1).IsHidden())
							k.Hide();
						else k.Unhide();
						}

					spline->AddKnot(k, 0);
					vsel.SetSize(spline->Verts(), 1);
					ssel.SetSize(spline->Segments(), 1);
					vsel.Shift(RIGHT_BITSHIFT, 3);
					ssel.Shift(RIGHT_BITSHIFT, 1);
					vsel.Set(1, vsel2[i*3+1]);
					ssel.Set(0, (first) ? 0 : ssel2[i - 1]);
					}
				}
			else {
				// Simple forward copy
				for(int i = 0, first = 1; i < spline2->KnotCount(); ++i, first = 0) {
					SplineKnot k(spline2->GetKnotType(i), spline2->GetLineType(i),
						spline2->GetKnotPoint(i),spline2->GetInVec(i), spline2->GetOutVec(i));
					spline->AddKnot(k);
					vsel.SetSize(spline->Verts(), 1);
					ssel.SetSize(spline->Segments(), 1);
					vsel.Set((spline->KnotCount() - 1) * 3 + 1, vsel2[i*3+1]);
					ssel.Set(spline->Segments() - 1, (first) ? 0 : ssel2[i - 1]);
					}
				}
			}
		else {
			if(vert1 == 1) {
				// Backward copy
				for(int i = spline2->KnotCount() - 1, first = 1; i >= 0; --i, first = 0) {
					SplineKnot k(spline2->GetKnotType(i), (first) ? LTYPE_CURVE : spline2->GetLineType(i),
						spline2->GetKnotPoint(i),spline2->GetInVec(i), spline2->GetOutVec(i));
					spline->AddKnot(k, 0);
					vsel.SetSize(spline->Verts(), 1);
					ssel.SetSize(spline->Segments(), 1);
					vsel.Shift(RIGHT_BITSHIFT, 3);
					ssel.Shift(RIGHT_BITSHIFT, 1);
					vsel.Set(1, vsel2[i*3+1]);
					ssel.Set(0, (first) ? 0 : ssel2[i]);
					}
				}
			else {
				// Backward copy, reversing vectors
				for(int i = spline2->KnotCount() - 1, first = 1; i >= 0; --i,first = 0) {
					SplineKnot k(spline2->GetKnotType(i), (i == 0) ? LTYPE_CURVE : spline2->GetLineType(i - 1),
						spline2->GetKnotPoint(i),spline2->GetOutVec(i), spline2->GetInVec(i));
//watje  216165 6/14/00 since the spline is  flipped need to much around with the hidden point to get them poinitng the rigth way
					if (i != 0)
						{
						if (spline2->GetKnot(i-1).IsHidden())
							k.Hide();
						else k.Unhide();
						}

					spline->AddKnot(k, -1);
					vsel.SetSize(spline->Verts(), 1);
					ssel.SetSize(spline->Segments(), 1);
					vsel.Set((spline->KnotCount() - 1) * 3 + 1, vsel2[i*3+1]);
					ssel.Set(spline->Segments() - 1, (first) ? 0 : ssel2[i]);
					}
				}
			}
		spline->ComputeBezPoints();
		shape->DeleteSpline(poly2);

		}
//	shape->UpdateBindList();
	}

BOOL VertConnectRecord::Redo(BezierShape *shape,int reRecord) {
	if(!IsCompatible(shape, poly1, &oldVSel1, &oldSSel1))
		return FALSE;
	if(poly1 != poly2 && !IsCompatible(shape, poly2, &oldVSel2, &oldSSel2))
		return FALSE;
	if(reRecord) {
		oldSpline1 = *(shape->splines[poly1]);
		oldVSel1 = shape->vertSel[poly1];
		oldSSel1 = shape->segSel[poly1];
		if(poly1 != poly2) {
			oldSpline2 = *(shape->splines[poly2]);
			oldVSel2 = shape->vertSel[poly2];
			oldSSel2 = shape->segSel[poly2];
			selected2 = shape->polySel[poly2];
			}
		}
	ConnectVerts(shape, poly1, vert1, poly2, vert2, FALSE);
	return TRUE;
	}

#define VCONR_GENERAL_CHUNK		0x1000
#define VCONR_OLDVSEL1_CHUNK	0x1040
#define VCONR_OLDSSEL1_CHUNK	0x1050
#define VCONR_SPLINE1_CHUNK		0x1060
#define VCONR_OLDVSEL2_CHUNK	0x1070
#define VCONR_OLDSSEL2_CHUNK	0x1080
#define VCONR_SPLINE2_CHUNK		0x1090

IOResult VertConnectRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case VCONR_GENERAL_CHUNK:
				res = iload->Read(&poly1,sizeof(int),&nb);
				res = iload->Read(&vert1,sizeof(int),&nb);
				res = iload->Read(&poly2,sizeof(int),&nb);
				res = iload->Read(&vert2,sizeof(int),&nb);
				res = iload->Read(&selected2,sizeof(int),&nb);
				break;
			case VCONR_OLDVSEL1_CHUNK:
				res = oldVSel1.Load(iload);
				break;
			case VCONR_OLDSSEL1_CHUNK:
				res = oldSSel1.Load(iload);
				break;
			case VCONR_SPLINE1_CHUNK:
				res = oldSpline1.Load(iload);
				break;
			case VCONR_OLDVSEL2_CHUNK:
				res = oldVSel2.Load(iload);
				break;
			case VCONR_OLDSSEL2_CHUNK:
				res = oldSSel2.Load(iload);
				break;
			case VCONR_SPLINE2_CHUNK:
				res = oldSpline2.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL VertInsertRecord::Redo(BezierShape *shape,int reRecord) {
	if(poly >= shape->splineCount)
		return FALSE;
	if(reRecord) {
		oldSpline = *(shape->splines[poly]);
		oldVSel = shape->vertSel[poly];
		oldSSel = shape->segSel[poly];
		}
	shape->DeleteSpline(poly);
	shape->InsertSpline(&newSpline, poly);
	shape->vertSel[poly] = newVSel;
	shape->segSel[poly] = newSSel;
	return TRUE;
	}

#define VINSR_GENERAL_CHUNK		0x1000
#define VINSR_OLDVSEL_CHUNK		0x1010
#define VINSR_OLDSSEL_CHUNK		0x1050
#define VINSR_OLDSPLINE_CHUNK	0x1060
#define VINSR_NEWVSEL_CHUNK		0x1070
#define VINSR_NEWSSEL_CHUNK		0x1080
#define VINSR_NEWSPLINE_CHUNK	0x1090

IOResult VertInsertRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case VINSR_GENERAL_CHUNK:
				res = iload->Read(&poly,sizeof(int),&nb);
				break;
			case VINSR_OLDVSEL_CHUNK:
				res = oldVSel.Load(iload);
				break;
			case VINSR_OLDSSEL_CHUNK:
				res = oldSSel.Load(iload);
				break;
			case VINSR_OLDSPLINE_CHUNK:
				res = oldSpline.Load(iload);
				break;
			case VINSR_NEWVSEL_CHUNK:
				res = newVSel.Load(iload);
				break;
			case VINSR_NEWSSEL_CHUNK:
				res = newSSel.Load(iload);
				break;
			case VINSR_NEWSPLINE_CHUNK:
				res = newSpline.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL PolyFirstRecord::Redo(BezierShape *shape,int reRecord) {
	if(poly >= shape->splineCount || !IsCompatible(shape, poly, &oldVSel, &oldSSel))
		return FALSE;
	if(reRecord) {
		oldSpline = *(shape->splines[poly]);
		oldVSel = shape->vertSel[poly];
		oldSSel = shape->segSel[poly];
		}
	shape->MakeFirst(poly, vertex);
	return TRUE;
	}

#define PFR_GENERAL_CHUNK		0x1000
#define PFR_OLDVSEL_CHUNK		0x1010
#define PFR_OLDSSEL_CHUNK		0x1050
#define PFR_OLDSPLINE_CHUNK		0x1060

IOResult PolyFirstRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case PFR_GENERAL_CHUNK:
				res = iload->Read(&poly,sizeof(int),&nb);
				res = iload->Read(&vertex,sizeof(int),&nb);
				break;
			case PFR_OLDVSEL_CHUNK:
				res = oldVSel.Load(iload);
				break;
			case PFR_OLDSSEL_CHUNK:
				res = oldSSel.Load(iload);
				break;
			case PFR_OLDSPLINE_CHUNK:
				res = oldSpline.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

// Break a polygon at all selected vertices
void BreakSplineAtSelVerts(BezierShape *shape, int poly) {
	Spline3D *spline = shape->splines[poly];
	int altered = 0;
	for(int k = spline->KnotCount()-1; k >= 0; --k) {
		BitArray& vsel = shape->vertSel[poly];
		BitArray& ssel = shape->segSel[poly];
		int vert = k*3+1;
		if(vsel[vert]) {
			if(spline->Closed()) {
				if(k == (spline->KnotCount()-1)) {	// Break at end knot
					break_at_last_knot:
					altered = 1;
					SplineKnot dupKnot(spline->GetKnotType(k),spline->GetLineType(k),
						spline->GetKnotPoint(k),spline->GetInVec(k),spline->GetOutVec(k));
					spline->AddKnot(dupKnot, 0);
					spline->SetOpen();
					vsel.Clear(vert);
					vsel.SetSize(spline->Verts(),1);
					vsel.Shift(RIGHT_BITSHIFT,3);
					vsel.Clear(0);	// New point not selected
					vsel.Clear(1);
					vsel.Clear(2);
					ssel.SetSize(spline->Segments(),1);
					ssel.Shift(RIGHT_BITSHIFT,1);
					ssel.Clear(0);
					k++;	// Increment pointer so we don't miss a knot!
					}
				else
				if(k == 0) {			// Break at first knot
					altered = 1;
					SplineKnot dupKnot(spline->GetKnotType(0),spline->GetLineType(0),
						spline->GetKnotPoint(0),spline->GetInVec(0),spline->GetOutVec(0));
					spline->AddKnot(dupKnot, -1);
					spline->SetOpen();
					vsel.Clear(vert);
					vsel.SetSize(spline->Verts(),1);
					vsel.Clear(spline->Verts()-3);		// New point not selected
					vsel.Clear(spline->Verts()-2);
					vsel.Clear(spline->Verts()-1);
					ssel.SetSize(spline->Segments(),1);
					ssel.Clear(spline->KnotCount()-1);
					}
				else {					// Break somewhere in the middle
					// First, rotate the spline so that the break point is the last one!
					int rotations = 0;
					int lastKnot = spline->KnotCount() - 1;
					while(k < (lastKnot)) {
						SplineKnot dupKnot(spline->GetKnotType(lastKnot),spline->GetLineType(lastKnot),
							spline->GetKnotPoint(lastKnot),spline->GetInVec(lastKnot),spline->GetOutVec(lastKnot));
						spline->DeleteKnot(lastKnot);
						spline->AddKnot(dupKnot, 0);
						rotations++;
						k++;
						}
					vsel.Rotate(RIGHT_BITSHIFT, rotations*3);
					ssel.Rotate(RIGHT_BITSHIFT, rotations);
					k = lastKnot;
					vert = k*3+1;
					goto break_at_last_knot;
					}
				}
			else {
				// Don't do anything if the spline's open and we're breaking at an end vertex!
				if (k == 0 || k == (spline->KnotCount()-1)) {
					vsel.Clear(vert);	// Just turn off the selection bit
					continue;
					}
				int i;
				int newPolySize = spline->KnotCount() - k;
				// OK, We're breaking at a point in the middle -- Copy end points off to a new spline, then
				// delete them from the original!
				Spline3D *newSpline = shape->NewSpline();
				for(i = k; i < spline->KnotCount(); ++i) {
					SplineKnot dupKnot(spline->GetKnotType(i),spline->GetLineType(i),
						spline->GetKnotPoint(i),spline->GetInVec(i),spline->GetOutVec(i));
					newSpline->AddKnot(dupKnot, -1);
					}
				for(i = spline->KnotCount()-1; i > k; --i)
					spline->DeleteKnot(i);
				vsel.Clear(vert);	// Deselect the knot
				// Adjust selection data for this spline
				vsel.SetSize(spline->Verts(),1);
				ssel.SetSize(spline->Segments(),1);
				// Don't forget to create a new selection record for this new spline!
				shape->vertSel.Insert(shape->splineCount - 1, newPolySize * 3);
				shape->segSel.Insert(shape->splineCount - 1, newPolySize - 1);
				shape->polySel.Insert(shape->splineCount - 1);
				}
			}
		}
	if(altered) {
//3-31-99 watje
//		shape->UpdateBindList();
		spline->ComputeBezPoints();
		shape->InvalidateGeomCache();
		}
	}

BOOL VertBreakRecord::Redo(BezierShape *shape,int reRecord) {
	if(!IsCompatible(shape, poly, &oldVSel, &oldSSel))
		return FALSE;
	if(reRecord) {
		oldSpline = *(shape->splines[poly]);
		oldVSel = shape->vertSel[poly];
		oldSSel = shape->segSel[poly];
		selected = shape->polySel[poly];
		}
	BreakSplineAtSelVerts(shape, poly);
	return TRUE;
	}

#define VBRKR_GENERAL_CHUNK		0x1000
#define VBRKR_OLDVSEL_CHUNK		0x1040
#define VBRKR_OLDSSEL_CHUNK		0x1050
#define VBRKR_SPLINE_CHUNK		0x1060

IOResult VertBreakRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case VBRKR_GENERAL_CHUNK:
				res = iload->Read(&poly,sizeof(int),&nb);
				res = iload->Read(&selected,sizeof(int),&nb);
				res = iload->Read(&oldSplineCount,sizeof(int),&nb);
				break;
			case VBRKR_OLDVSEL_CHUNK:
				res = oldVSel.Load(iload);
				break;
			case VBRKR_OLDSSEL_CHUNK:
				res = oldSSel.Load(iload);
				break;
			case VBRKR_SPLINE_CHUNK:
				res = oldSpline.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

// Weld a polygon at all selected vertices
// Returns TRUE if weld results in entire spline deletion (degenerate, single vertex poly)

BOOL WeldSplineAtSelVerts(BezierShape *shape, int poly, float thresh) {
	int i;
	Spline3D *spline = shape->splines[poly];
	BitArray& vsel = shape->vertSel[poly];
	BitArray& ssel = shape->segSel[poly];
	int knots = spline->KnotCount();
	BitArray toPrev(knots);
	BitArray toNext(knots);
	toNext.ClearAll();

	// Create weld attachment flag arrays
	for(i = 0; i < knots; ++i) {
		if(vsel[i*3+1]) {
			int next = (i + 1) % knots;
			if(vsel[next*3+1] && Length(spline->GetKnotPoint(i) - spline->GetKnotPoint(next)) <= thresh)
				toNext.Set(i);
			}
		}

	// Now process 'em!
	for(i = knots - 1; i >= 0; --i) {
		if(toNext[i]) {
			int next = (i + 1) % spline->KnotCount();
			if(i == (spline->KnotCount() - 1))
				spline->SetClosed();
			Point3 midpoint = (spline->GetKnotPoint(i) + spline->GetKnotPoint(next)) / 2.0f;
			Point3 nextDelta = midpoint - spline->GetKnotPoint(next);
			Point3 thisDelta = midpoint - spline->GetKnotPoint(i);
			spline->SetKnotPoint(next, spline->GetKnotPoint(next) + nextDelta);
			spline->SetOutVec(next, spline->GetOutVec(next) + nextDelta);
			spline->SetInVec(next, spline->GetInVec(i) + thisDelta);
			if(spline->IsBezierPt(i) || spline->IsBezierPt(next))
				spline->SetKnotType(next, KTYPE_BEZIER_CORNER);
			else
			if(spline->IsCorner(i) || spline->IsCorner(next))
				spline->SetKnotType(next, KTYPE_CORNER);
			spline->DeleteKnot(i);
			}
		}

	// If the polygon is degenerate, blow it away!
	if(spline->KnotCount() < 2) {
		spline->NewSpline();
		shape->DeleteSpline(poly);
		return TRUE;
		}

	// Update the auto points
//	shape->UpdateBindList();
	spline->ComputeBezPoints();
	shape->InvalidateGeomCache();

	// Clear out the selection sets for verts and segments
	vsel.SetSize(spline->Verts(),0);
	vsel.ClearAll();
	ssel.SetSize(spline->Segments(),0);
	ssel.ClearAll();
	return FALSE;
	}

BOOL VertWeldRecord::Redo(BezierShape *shape,int reRecord) {
	if(poly >= shape->splineCount)
		return FALSE;
	if(reRecord) {
		oldSpline = *(shape->splines[poly]);
		oldVSel = shape->vertSel[poly];
		oldSSel = shape->segSel[poly];
		selected = shape->polySel[poly];
		}
	deleted = WeldSplineAtSelVerts(shape, poly, thresh);
	return TRUE;
	}

#define VWELDR_GENERAL_CHUNK		0x1000
#define VWELDR_OLDVSEL_CHUNK		0x1040
#define VWELDR_OLDSSEL_CHUNK		0x1050
#define VWELDR_SPLINE_CHUNK			0x1060

IOResult VertWeldRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case VWELDR_GENERAL_CHUNK:
				res = iload->Read(&poly,sizeof(int),&nb);
				res = iload->Read(&thresh,sizeof(float),&nb);
				res = iload->Read(&selected,sizeof(int),&nb);
				res = iload->Read(&deleted,sizeof(BOOL),&nb);
				break;
			case VWELDR_OLDVSEL_CHUNK:
				res = oldVSel.Load(iload);
				break;
			case VWELDR_OLDSSEL_CHUNK:
				res = oldSSel.Load(iload);
				break;
			case VWELDR_SPLINE_CHUNK:
				res = oldSpline.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

// A handy 2D floating-point box class

class ESMBox2D {
	public:
		BOOL empty;
		Point2 min, max;
		ESMBox2D() { empty = TRUE; }
		void SetEmpty() { empty = TRUE; }
		ESMBox2D& operator+=(const Point2& p);	// expand this box to include p
	};

ESMBox2D& ESMBox2D::operator+=(const Point2& p) {
	if(empty) {
		min = max = p;
		empty = FALSE;
		}
	else {
		if(p.x < min.x) min.x = p.x;
		if(p.x > max.x) max.x = p.x;
		if(p.y < min.y) min.y = p.y;
		if(p.y > max.y) max.y = p.y;
		}
	return *this;
	}

#define FSGN(x) (x < 0.0f ? -1 : 1)

// Determine if the line segments p1-p2 and p3-p4 intersect
// Returns: 0 (no hit) 1 (hit) 2 (parallel)

static int IntSeg(Point2& p1, Point2& p2, Point2& p3, Point2& p4, Point2& icpt) {
	float C1,C2,DENOM;
	float test1,test2;
	int sgnx1,sgny1,sgnx2,sgny2,hitend;

	/* First, simple minmax test */

	ESMBox2D line1, line2;
	line1.SetEmpty();
	line1 += p1;
	line1 += p2;
	line2.SetEmpty();
	line2 += p3;
	line2 += p4;

	if(line1.max.x < line2.min.x || line1.min.x > line2.max.x || line1.max.y < line2.min.y || line1.min.y > line2.max.y)
		return 0;

	/* A1=-dy1 B1=dx1 A2=-dy2 B2=dx2 */

	Point2 d1 = p2 - p1;
	C1= -(p1.y * d1.x - p1.x * d1.y);

	Point2 d2 = p4 - p3;
	C2= -(p3.y * d2.x - p3.x * d2.y);

	DENOM= -d1.y * d2.x + d2.y * d1.x;

	if(DENOM==0.0)		/* Lines parallel!!! */
	 {
	 test1= -p1.x * d2.y + p1.y * d2.x + C2;
	 test2= -p3.x * d1.y + p3.y * d1.x + C1;
	 if(test1==test2)	/* Lines collinear! */
	  {
	  if(p1 == p3)
	   {
	   icpt = p1;
	   hitend=1;
	   sgnx1=FSGN(p2.x - p1.x);
	   sgny1=FSGN(p2.y - p1.y);
	   sgnx2=FSGN(p3.x - p4.x);
	   sgny2=FSGN(p3.y - p4.y);
	   }
	  else
	  if(p1 == p4)
	   {
	   icpt = p1;
	   hitend=1;
	   sgnx1=FSGN(p2.x - p1.x);
	   sgny1=FSGN(p2.y - p1.y);
	   sgnx2=FSGN(p4.x - p3.x);
	   sgny2=FSGN(p4.y - p3.y);
	   }
	  else
	  if(p2 == p3)
	   {
	   icpt = p2;
	   hitend=1;
	   sgnx1=FSGN(p1.x - p2.x);
	   sgny1=FSGN(p1.y - p2.y);
	   sgnx2=FSGN(p3.x - p4.x);
	   sgny2=FSGN(p3.y - p4.y);
	   }
	  else
	  if(p2 == p4)
	   {
	   icpt = p2;
	   hitend=1;
	   sgnx1=FSGN(p1.x - p2.x);
	   sgny1=FSGN(p1.y - p2.y);
	   sgnx2=FSGN(p4.x - p3.x);
	   sgny2=FSGN(p4.y - p3.y);
	   }
	  else
	   hitend=0;

	  if(hitend)
	   {
	   if(sgnx1==sgnx2 && sgny1==sgny2)	/* Hit at endpoint */
	    return(1);
	   }
	  return(2);
	  }
	 return(0);
	 }

	if(p1.x == p2.x)
	 icpt.x = p1.x;
	else
	if(p3.x == p4.x)
	 icpt.x = p3.x;
	else
	icpt.x = (d1.x * C2 - d2.x * C1) / DENOM;

	if(p1.y == p2.y)
	 icpt.y = p1.y;
	else
	if(p3.y == p4.y)
	 icpt.y = p3.y;
	else
	icpt.y = (-C1 * d2.y + C2 * d1.y) / DENOM;

	/* See if it hit line 1 */

	if(p1.x < p2.x)
	 {
	 if(icpt.x < p1.x || icpt.x > p2.x)
	  return(0);
	 }
	else
	 {
	 if(icpt.x < p2.x || icpt.x > p1.x)
	  return(0);
	 }
	if(p1.y < p2.y)
	 {
	 if(icpt.y < p1.y || icpt.y > p2.y)
	  return(0);
	 }
	else
	 {
	 if(icpt.y < p2.y || icpt.y > p1.y)
	  return(0);
	 }

	/* See if it hit line 2 */

	if(p3.x < p4.x)
	 {
	 if(icpt.x < p3.x || icpt.x > p4.x)
	  return(0);
	 }
	else
	 {
	 if(icpt.x < p4.x || icpt.x > p3.x)
	  return(0);
	 }
	if(p3.y < p4.y)
	 {
	 if(icpt.y < p3.y || icpt.y > p4.y)
	  return(0);
	 }
	else
	 {
	 if(icpt.y < p4.y || icpt.y > p3.y)
	  return(0);
	 }

	/* It hits both! */

	return(1);
	}

// Determine where the line segments p1-p2 and p3-p4 intersect
static int FindIntercept(Point2& p1, Point2& p2, Point2& p3, Point2& p4, Point2& icpt) {
	float C1,C2,DENOM;
	Point2 d1,d2;
	float test1,test2;

	d1 = p2 - p1;
	C1= -(p1.y * d1.x - p1.x * d1.y);

	d2 = p4 - p3;
	C2= -(p3.y * d2.x - p3.x * d2.y);

	DENOM= -d1.y * d2.x + d2.y * d1.x;

	if(DENOM==0.0)		/* Lines parallel!!! */
	 {
	 test1 = p1.x * -d2.y + p1.y * d2.x + C2;
	 test2 = p3.x * -d1.y + p3.y * d1.x + C1;
	 if(test1==test2)	/* Lines collinear! */
	  return(2);
	 return(0);
	 }

	if(p1.x == p2.x)
	 icpt.x = p1.x;
	else
	if(p3.x == p4.x)
	 icpt.x = p3.x;
	else
	icpt.x = (d1.x * C2 - d2.x * C1) / DENOM;

	if(p1.y == p2.y)
	 icpt.y = p1.y;
	else
	if(p3.y == p4.y)
	 icpt.y = p3.y;
	else
	icpt.y = (C1 * (-d2.y) - C2 * (-d1.y)) / DENOM;

	return(1);
	}



class ESMTemplate {
	public:
		int points;
		Point2 *pts;
		ESMTemplate(Spline3D *spline);
		~ESMTemplate();
		int Points() { return points; }
		int OriginalSegment(int seg) { return seg / 10; }
//		BOOL Orient();
		void FlagInside(Spline3D *spline, IntTab& flags);
		BOOL SurroundsPoint(Point2& point);
	};

ESMTemplate::ESMTemplate(Spline3D *spline) {
	int segments = spline->Segments();
	points = segments * 10 + 1;
	pts = new Point2[points];
	int point;
	Point3 p3;
	int i,j;
	float pct;
	for(i = 0, point = 0; i < segments; ++i) {
		for(j = 0, pct = 0.0f; j < 10; ++j, pct += 0.1f, ++point) {
			p3 = spline->InterpBezier3D(i, pct);
			pts[point] = Point2(p3.x, p3.y);
			}
		}
	if(spline->Closed())
		pts[point] = pts[0];
	else {
		p3 = spline->InterpBezier3D(segments, 0.0f);
		pts[point] = Point2(p3.x, p3.y);
		}
	}

ESMTemplate::~ESMTemplate() {
	delete [] pts;
	}

static int myrand(int mask) {
	return rand() & mask;
	}

BOOL ESMTemplate::SurroundsPoint(Point2& point) {
	int ix,ix2,outs,xout,yout;
	int hits,cept,trials,odds,evens;
	Point2 point2, point3, point4, where;
		
	evens=odds=0;
	
	/* Do three trials -- use the one with most */

	for(trials=0; trials<3; ++trials)
		{

		surr_try:
		point2.x = point.x + (float)(myrand(63) + 1);		/* Random second point */
		point2.y = point.y + (float)(myrand(63) - 32);
//DebugPrint("Point:%.2f %.2f Point2:%.2f %.2f\n",point.x,point.y,point2.x,point2.y);
		hits=0;
		for(ix=0; ix<(points - 1); ++ix)
			{
			ix2=ix+1;
	
			point3 = pts[ix];
			point4 = pts[ix2];

			/* Check for intercept */

			cept=FindIntercept(point, point2, point3, point4, where);

			switch(cept)
				{
				case 1:
					/* If lines intersect, check to see if it's in segment */
					if(where.x > point.x) {
						outs = 0;
						/* Abort test if ray goes thru endpoint */
						if(where == point3 || where == point4)
							goto surr_try;
	
						/* Figure out major axis of test line */
	
						xout=(fabs(point4.x - point3.x) > fabs(point4.y - point3.y)) ? 2:1;
						yout=(xout==2) ? 1:2;
	
						/* If ray intercepts line, record hit */
	
						if(point3.x < point4.x)
							{
							if(where.x < point3.x || where.x > point4.x)
								outs+=xout;
							}
						else
						if(point3.x > point4.x)
							{
							if(where.x < point4.x || where.x > point3.x)
								outs+=xout;
							}
						else
							outs++;
	
						if(point3.y < point4.y)
							{
							if(where.y < point3.y || where.y > point4.y)
								outs+=yout;
							}
						else
						if(point3.y > point4.y)
							{
							if(where.y < point4.y || where.y > point3.y)
								outs+=yout;
							}
						else
							outs++;
	
						/* If not outside both coords, it hits! */
	
						if(outs<2)
							hits++;
						}
					break;
				case 2:
					goto surr_try;
				case 0:
					break;
				}
			}
	
	/* odd # hits = surrounded! */
	
		if(hits & 1)
			odds++;
		else
			evens++;
		}
	
	/* even # hits = outside! */
	
	if(odds>evens)
		return(TRUE);
	return(FALSE);
	}

void ESMTemplate::FlagInside(Spline3D *spline, IntTab& flags) {
//DebugPrint("Starting FlagInside\n");
	int segs = spline->Segments();
	for(int i = 0; i < segs; ++i) {
		Point3 p = spline->InterpBezier3D(i, 0.5f);	// Use midpoint to determine whether we're inside or out
		flags[i] = flags[i] | SurroundsPoint(Point2(p.x, p.y)) ? POLYINSIDE : POLYOUTSIDE;
//DebugPrint("Seg %d inside:%s outside:%s\n",i, (flags[i] & POLYINSIDE) ? "YES":"NO", (flags[i] & POLYOUTSIDE) ? "YES":"NO");
		}
	}

class BoolHitData {
	public:
		int id;
		int poly;
		int seg;
		float pct;
		Point2 where;
		Point3 force;
		BOOL gotForce;
		BoolHitData() { gotForce = FALSE; }
		BoolHitData(int id, int poly, int seg, float pct, Point2& where)
			{ gotForce = FALSE; this->id = id; this->poly = poly; this->seg = seg; this->pct = pct; this->where = where; }
		int operator==( const BoolHitData& b ) const { 	return (poly==b.poly && seg==b.seg && pct==b.pct && where==b.where); }
	};

class BoolHit {
	public:
		int count;
		BoolHitData *data;
		BoolHit() { data = NULL; }
		~BoolHit() { if(data) delete [] data; }
		void SetSize(int count);
		void Set(Spline3D *spline, int location, int id, int poly, int seg, Point2& where);
		void Sort();
		BOOL GotForcePoint(BoolHitData& h, Point3* force);
		void SetForcePoint(BoolHitData& h, Point3& force);
	};

void BoolHit::SetSize(int count) {
	data = new BoolHitData[count];
	this->count = count;
	for(int i=0; i<count; ++i)
		data[i].gotForce = FALSE;
	}

BOOL BoolHit::GotForcePoint(BoolHitData& h, Point3* force) {
	for(int i = 0; i < count; ++i) {
		if(data[i].id == h.id && data[i].gotForce) {
			*force = data[i].force;
			return TRUE;
			}
		}
	return FALSE;
	}

void BoolHit::SetForcePoint(BoolHitData& h, Point3& force) {
	h.gotForce = TRUE;
	h.force = force;
//DebugPrint("Set force for id# %d to %.4f %.4f\n",h.id,force.x, force.y);
	}

#define CURVELOOPS 15

float Calc2DCurvePct(Spline3D *spline, int seg, Point2& where) {
	int count = CURVELOOPS;
	float low = 0.0f;
	float high = 1.0f;
	float bestL,bestPos;
	while(count--) {
		float range = high - low;
		float step = range / 10.0f;
		float pos;
		int i;

		for(i = 0, pos = low; i <= 10; ++i, pos += step) {
			Point3 p = spline->InterpBezier3D(seg, pos);
			Point2 p1 = Point2(p.x, p.y);
			float l = Length(p1 - where);
			if(i == 0 || l < bestL) {
				bestL = l;
				bestPos = pos;
				}
			}
		low = bestPos - step;
		if(low < 0.0f)
			low = 0.0f;
		high = bestPos + step;
		if(high > 1.0f)
			high = 1.0f;
		}
	return bestPos;
	}

void BoolHit::Set(Spline3D *spline, int location, int id, int poly, int seg, Point2& where) {
	// Find where on the curve the hit occurred, percentage-wise
	float pct = Calc2DCurvePct(spline, seg, where);
//DebugPrint("Hit %d on poly %d, seg:%d at %.4f %.4f was %.4f percent\n",location,poly,seg,where.x,where.y,pct);
//Point3 check = spline->InterpBezier3D(seg,pct);
//DebugPrint("Cross-check: %.4f %.4f\n",check.x,check.y);
	data[location] = BoolHitData(id, poly, seg, pct, where);
	}

void BoolHit::Sort() {
	BoolHitData temp;
	unsigned int srtcnt;
	register unsigned int srt1,srt2,srt3,srt4;

	srtcnt=count+1;

	nxtsc:
	srtcnt/=2;
	if(srtcnt==0) {
		int ix,jx;
		/* Eliminate duplicates */
		ix=0;
		jx=1;
		dup_loop:
	 	if(data[ix] == data[jx])
			jx++;
		else {
			ix++;
			if(ix!=jx)
				data[ix] = data[jx];
			jx++;
			}
		if(jx<count)
			goto dup_loop;
		count=ix+1;
		return;
		}
	srt1=1;
	srt2=count-srtcnt;

	srtsw1:
	srt3=srt1;

	srtsw2:
	srt4=srt3+srtcnt;

	if(data[srt3-1].poly < data[srt4-1].poly)
		goto swap;
	if(data[srt3-1].poly > data[srt4-1].poly)
		goto next;
	if(data[srt3-1].seg < data[srt4-1].seg)
		goto swap;
	if(data[srt3-1].seg > data[srt4-1].seg)
		goto next;
	if(data[srt3-1].pct >= data[srt4-1].pct)
		goto next;

	swap:
	temp = data[srt3-1];
	data[srt3-1] = data[srt4-1];
	data[srt4-1] = temp;
	if(srtcnt<srt3) {
		srt3-=srtcnt;
		goto srtsw2;
		}

	next:
	srt1++;
	if(srt1<=srt2)
		goto srtsw1;
	goto nxtsc;
	}

static void BoundSpline2D(Spline3D *spline, ESMBox2D& box) {
	Point2 p2;
	Point3 p3;
	box.SetEmpty();
	int knots = spline->KnotCount();
	int closed = spline->Closed();
	int lastKnot = knots - 1;
	for(int i = 0; i < knots; ++i) {
		p3 = spline->GetKnotPoint(i);
		p2.x = p3.x;
		p2.y = p3.y;
		box += p2;
		if (i > 0 || closed) {
			p3 = spline->GetInVec(i);
			p2.x = p3.x;
			p2.y = p3.y;
			box += p2;
			}
		if(i < lastKnot || closed) {
			p3 = spline->GetOutVec(i);
			p2.x = p3.x;
			p2.y = p3.y;
			box += p2;
			}
		}
	}

static void BoolCopy(BezierShape *shape, int poly, IntTab& flags, int mask) {
	BitArray &ssel = shape->segSel[poly];
	ssel.ClearAll();
	Spline3D *spline = shape->splines[poly];
	int segs = spline->Segments();
	int i;
	// Mark the segments that don't contain the mask flag for deletion
	for(i = 0; i < segs; ++i) {
		if(!(flags[i] & mask))
			ssel.Set(i);
		}

	// Now delete all those segments that we just flagged!
#ifndef DEBUGBOOL
	DeleteSelSegs(shape, poly);
#endif
	}																

// Weld all polygons in the shape at common endpoints

static BOOL WeldShape(BezierShape *shape) {
	int i,j;
	int lastKnot1,lastKnot2,knots1,knots2;
	Point3 p;
	BOOL attached;
	do {
		attached = FALSE;

		for(i = 0; i < shape->splineCount; ++i) {
			Spline3D *spline1 = shape->splines[i];
			knots1 = spline1->KnotCount();
			lastKnot1 = knots1 - 1;
			p = spline1->GetKnotPoint(0);
			Point2 s1first = Point2(p.x, p.y);
			p = spline1->GetKnotPoint(lastKnot1);
			Point2 s1last = Point2(p.x, p.y);

			for(j = shape->splineCount - 1; j > i; --j) {
				Spline3D *spline2 = shape->splines[j];
				knots2 = spline2->KnotCount();
				lastKnot2 = knots2 - 1;
				p = spline2->GetKnotPoint(0);
				Point2 s2first = Point2(p.x, p.y);
				p = spline2->GetKnotPoint(lastKnot2);
				Point2 s2last = Point2(p.x, p.y);
//DebugPrint("Checking %d/%d: %.4f %.4f - %.4f %.4f / %.4f %.4f - %.4f %.4f\n",i,j,s1first.x,s1first.y,s1last.x,s1last.y,
//	s2first.x,s2first.y,s2last.x,s2last.y);
				if(s1first == s2first) {
					spline2->Reverse();
					spline1->Prepend(spline2);
					shape->DeleteSpline(j);
//DebugPrint("Welded 1!\n");
					attached = TRUE;
					}
				else
				if(s1first == s2last) {
					spline1->Prepend(spline2);
					shape->DeleteSpline(j);
//DebugPrint("Welded 2!\n");
					attached = TRUE;
					}
				else
				if(s1last == s2first) {
					spline1->Append(spline2);
					shape->DeleteSpline(j);
//DebugPrint("Welded 3!\n");
					attached = TRUE;
					}
				else
				if(s1last == s2last) {
					spline2->Reverse();
					spline1->Append(spline2);
					shape->DeleteSpline(j);
//DebugPrint("Welded 4!\n");
					attached = TRUE;
					}
				}
			}
		} while(attached);

	// Now we go thru and make sure the resulting polygons are valid -- That is,
	// each must have coincident first and last points.  These polygons are stitched
	// together at the endpoints and closed.  If others are left, something
	// went hideously wrong.

	int polys = shape->splineCount;
//DebugPrint("Boolean resulted in %d polys\n",polys);
	for(i = 0; i < polys; ++i) {
		Spline3D *spline = shape->splines[i];
		// Make sure the first point is the same as the last, then close it
		int lastKnot = spline->KnotCount() - 1;
		p = spline->GetKnotPoint(0);
		Point2 first = Point2(p.x, p.y);
		p = spline->GetKnotPoint(lastKnot);
		Point2 last = Point2(p.x, p.y);
		if(!(first == last)) {
//DebugPrint("First:%.8f %.8f Last:%.8f %.8f\n",first.x, first.y, last.x, last.y);
			return FALSE;
			}
		spline->SetKnotType(0, KTYPE_BEZIER_CORNER);
		spline->SetInVec(0, spline->GetInVec(lastKnot));
		spline->DeleteKnot(lastKnot);
		shape->splines[i]->SetClosed();
		}

	return TRUE;
	}

// PerformBoolean return codes
#define BOOL_OK 1
#define BOOL_COINCIDENT_VERTEX	-1
#define BOOL_MUST_OVERLAP		-2
#define BOOL_WELD_FAILURE		-3

static int PerformBoolean(BezierShape *shape, int poly1, int poly2, int op, int *newPolyNum) {
	Spline3D *spline1 = shape->splines[poly1];
	int knots1 = spline1->KnotCount();
	Spline3D *spline2 = shape->splines[poly2];
	int knots2 = spline2->KnotCount();
	int i,j;

	// Can't have coincident vertices (why?)
	for(i = 0; i < knots1; ++i) {
		for(j = 0; j < knots2; ++j) {
			if(spline1->GetKnotPoint(i) == spline2->GetKnotPoint(j))
				return BOOL_COINCIDENT_VERTEX;
			}
		}
	
	// Make sure the polygons overlap in 2D
	ESMBox2D bound1, bound2;
	BoundSpline2D(spline1, bound1);
	BoundSpline2D(spline2, bound2);
	if(bound2.min.x > bound1.max.x || bound2.max.x < bound1.min.x ||
	   bound2.max.y < bound1.min.y || bound2.min.y > bound1.max.y) {
		no_overlap:
		return BOOL_MUST_OVERLAP;
		}

	// Load the splines into special 2D objects which contain the entire interpolated spline
	ESMTemplate t1(spline1);
	int points1 = t1.Points();
	int last1 = points1 - 1;
	ESMTemplate t2(spline2);
	int points2 = t2.Points();
	int last2 = points2 - 1;

	// Find the intersections between the two splines
	// This is a two-pass procedure -- Pass 0 counts the number of collisions,
	// and pass 1 records them.

	BoolHit hit;
	for(int pass = 0; pass < 2; ++pass) {
		int overlap = 0;
		int hitID = 0;
		for(i = 0; i < last1; ++i) {
			Point2 i1 = t1.pts[i];
			Point2 i2 = t1.pts[i+1];
			int iseg = t1.OriginalSegment(i);
			for(j = 0; j < last2; ++j) {
				Point2 j1 = t2.pts[j];
				Point2 j2 = t2.pts[j+1];
				int jseg = t2.OriginalSegment(j);

				/* Now compare line endpoints for collision */
	
				if(i1 == j1 || i1 == j2) {
					if(pass == 1) {
						hit.Set(spline1, overlap++, hitID, 0, iseg, i1);
						hit.Set(spline2, overlap++, hitID++, 1, jseg, i1);
						}
					else
						overlap += 2;
					}
				else
				if(i2 == j1 || i2 == j2) {
					if(pass == 1) {
						hit.Set(spline1, overlap++, hitID, 0, iseg, i2);
						hit.Set(spline2, overlap++, hitID++, 1, jseg, i2);
						}
					else
						overlap += 2;
					}
	
				/* Now test line segments themselves for collisions */
		
				Point2 dummy;
				if(IntSeg(i1, i2, j1, j2, dummy)==1)
					{
					Point2 where;
					FindIntercept(i1, i2, j1, j2, where);
					if(pass==1)
						{
						hit.Set(spline1, overlap++, hitID, 0, iseg, where);
						hit.Set(spline2, overlap++, hitID++, 1, jseg, where);
						}
					else
						overlap+=2;
					}
				}
			}
		if(pass==0)
			{
			if(overlap==0)
				goto no_overlap;
			hit.SetSize(overlap);
			}
		}

	// Sort the hits in ascending order and eliminate dupes
	hit.Sort();

	/* Go thru hitlist and adjust percentages for proper splits						*/
	/* This is necessary because the percentages of each cut are in relation		*/
	/* to the original segment, and as each cut is made, the lower percentages		*/
	/* must be adjusted.  Example: Original cuts at 25%, 50% and 75%.  After the	*/
	/* cut at 75% is made, the 50% cut becomes 66.6%.  After the 50% cut is made,	*/
	/* the 25% cut becomes 50%.														*/

	for(i = hit.count - 1; i > 0; --i)
		{
		j = i - 1;
		if(hit.data[j].poly == hit.data[i].poly && hit.data[j].seg == hit.data[i].seg)
		hit.data[i].pct = hit.data[i].pct / hit.data[j].pct;
		}

	// Copy the splines to a work shape
	BezierShape workShape;
	Spline3D *work0, *work1, *work[2];
	work0 = work[0] = workShape.NewSpline();
	*work[0] = *spline1;
	work1 = work[1] = workShape.NewSpline();
	*work[1] = *spline2;
	workShape.UpdateSels();

	/* Go thru hitlist and split up the original polygons */

	for(i = 0; i < hit.count; ++i)
		{
		BoolHitData &h = hit.data[i];
		Point3 force;
		int seg1 = h.seg;
		int seg2 = (h.seg + 1) % work[h.poly]->KnotCount();

		// Split 'em up and make sure the splits in the two polys are at the EXACT same point
		if(h.pct < 0.001) {
			if(hit.GotForcePoint(h, &force))
				workShape.splines[h.poly]->SetKnotPoint(seg1, force);
			else
				hit.SetForcePoint(h, workShape.splines[h.poly]->GetKnotPoint(seg1));
			}
		else
		if(h.pct > 0.999) {
			if(hit.GotForcePoint(h, &force))
				workShape.splines[h.poly]->SetKnotPoint(seg2, force);
			else
				hit.SetForcePoint(h, workShape.splines[h.poly]->GetKnotPoint(seg2));
			}
		else {
			RefineSegment(&workShape, h.poly, seg1, h.pct);
			if(hit.GotForcePoint(h, &force))
				workShape.splines[h.poly]->SetKnotPoint(seg1+1, force);
			else
				hit.SetForcePoint(h, workShape.splines[h.poly]->GetKnotPoint(seg1+1));
			}
		}

	// Set up some flags arrays
	IntTab flags0, flags1;
	IntTab *flags[2];
	flags[0] = &flags0;
	flags[1] = &flags1;
	int segs = work0->Segments();
	flags0.SetCount(segs);
	for(i = 0; i < segs; ++i)
		flags0[i] = 0;
	segs = work1->Segments();
	flags1.SetCount(segs);
	for(i = 0; i < segs; ++i)
		flags1[i] = 0;

	/* Polygons are diced up, now mark each piece	*/
	/* as being inside or outside the other polygon	*/

	t1.FlagInside(work1, flags1);
	t2.FlagInside(work0, flags0);

	/* Now delete spans according to boolean operator	*/
	/* UNION: Delete inside spans on both				*/
	/* SUBTRACTION: Delete pri/inside, sec/outside		*/
	/* INTERSECTION: Delete outside spans on both		*/

	switch(op)
		{
		case BOOL_UNION:
			BoolCopy(&workShape,1,flags1,POLYOUTSIDE);
			BoolCopy(&workShape,0,flags0,POLYOUTSIDE);
			break;
		case BOOL_SUBTRACTION:
			BoolCopy(&workShape,1,flags1,POLYINSIDE);
			BoolCopy(&workShape,0,flags0,POLYOUTSIDE);
			break;
		case BOOL_INTERSECTION:
			BoolCopy(&workShape,1,flags1,POLYINSIDE);
			BoolCopy(&workShape,0,flags0,POLYINSIDE);
			break;
		}

	/* Weld boolean pieces together, if necessary */
	
#ifndef DEBUGBOOL
	if(TRUE /*weldBooleans*/) {
		if(!WeldShape(&workShape))
			return BOOL_WELD_FAILURE;
		}

	// Get rid of the originals
	if(poly1 < poly2) {
		shape->DeleteSpline(poly2);
		shape->DeleteSpline(poly1);
		}
	else {
		shape->DeleteSpline(poly1);
		shape->DeleteSpline(poly2);
		}

	// Add all our new ones
	int oldPolys = shape->splineCount;
	int newPolys = workShape.splineCount;
	for(i = 0; i < newPolys; ++i) {
		Spline3D *poly = shape->NewSpline();
		*poly = *workShape.splines[i];
		}
	shape->UpdateSels();
	if(newPolys == 1) {
		shape->polySel.Set(oldPolys);
		if(newPolyNum)
			*newPolyNum = oldPolys;
		}
	else {
		if(newPolyNum)
			*newPolyNum = -1;
		}
#else
	*shape = workShape;
#endif // DEBUGBOOL
	return BOOL_OK;
	}

BOOL BooleanRecord::Redo(BezierShape *shape,int reRecord) {
	if(poly1 >= shape->splineCount || poly2 >= shape->splineCount)
		return FALSE;
	if(reRecord) {
		oldSpline1 = *(shape->splines[poly1]);
		oldSpline2 = *(shape->splines[poly2]);
		oldVSel1 = shape->vertSel[poly1];
		oldSSel1 = shape->segSel[poly1];
		oldVSel2 = shape->vertSel[poly2];
		oldSSel2 = shape->segSel[poly2];
		}
	PerformBoolean(shape, poly1, poly2, operation, NULL);
	return TRUE;
	}

#define BOOLR_GENERAL_CHUNK		0x1001
#define BOOLR_OLDSPLINE1_CHUNK	0x1010
#define BOOLR_OLDVSEL1_CHUNK	0x1020
#define BOOLR_OLDSSEL1_CHUNK	0x1030
#define BOOLR_OLDSPLINE2_CHUNK	0x1040
#define BOOLR_OLDVSEL2_CHUNK	0x1050
#define BOOLR_OLDSSEL2_CHUNK	0x1060

IOResult BooleanRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case BOOLR_GENERAL_CHUNK:
				res = iload->Read(&poly1,sizeof(int),&nb);
				res = iload->Read(&poly2,sizeof(int),&nb);
				res = iload->Read(&operation,sizeof(int),&nb);
				res = iload->Read(&oldSplineCount,sizeof(int),&nb);
				break;
			case BOOLR_OLDSPLINE1_CHUNK:
				res = oldSpline1.Load(iload);
				break;
			case BOOLR_OLDVSEL1_CHUNK:
				res = oldVSel1.Load(iload);
				break;
			case BOOLR_OLDSSEL1_CHUNK:
				res = oldSSel1.Load(iload);
				break;
			case BOOLR_OLDSPLINE2_CHUNK:
				res = oldSpline2.Load(iload);
				break;
			case BOOLR_OLDVSEL2_CHUNK:
				res = oldVSel2.Load(iload);
				break;
			case BOOLR_OLDSSEL2_CHUNK:
				res = oldSSel2.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}


/*-------------------------------------------------------------------*/		

static void DoAttach(BezierShape *shape, BezierShape *attShape, int mtlOffset) {
	int boff = shape->splineCount;
	for(int i = 0; i < attShape->splineCount; ++i) {
		int index = shape->splineCount;
		Spline3D *spline = shape->NewSpline();
		*spline = *(attShape->splines[i]);
		for(int seg = 0; seg < spline->Segments(); ++seg)
			spline->SetMatID(seg, spline->GetMatID(seg) + mtlOffset);
		shape->vertSel.Insert(shape->splineCount-1,spline->Verts());
		shape->segSel.Insert(shape->splineCount-1,spline->Segments());
		shape->polySel.Insert(shape->splineCount-1);
		shape->vertSel[index] = attShape->vertSel[i];
		shape->segSel[index] = attShape->segSel[i];
		shape->polySel.Set(index, attShape->polySel[i]);
		// Flush all aux data in attached spline (prevents ResolveTopoChanges problems)
		for(int j = 0; j < spline->KnotCount(); ++j)
			spline->SetAux2(j, -1);
		}
//copy over binds + offset
	for (i = 0; i < attShape->bindList.Count();i++)
		{
		attShape->bindList[i].pointSplineIndex += boff;
		attShape->bindList[i].segSplineIndex += boff;
		

		shape->bindList.Append(1,&attShape->bindList[i],1);
		}
	for (i = 0; i < shape->bindList.Count();i++)
		{
		int index = 0;
		int spindex = shape->bindList[i].pointSplineIndex;
		if (shape->bindList[i].isEnd)
				index = shape->splines[spindex]->KnotCount()-1;
		shape->bindList[i].bindPoint = shape->splines[spindex]->GetKnotPoint(index);
		shape->bindList[i].segPoint = shape->splines[spindex]->GetKnotPoint(index);

		}

	}

BOOL AttachRecord::Redo(BezierShape *shape,int reRecord) {
	if(reRecord)
		oldSplineCount = shape->splineCount;
	DoAttach(shape, &attShape, 0);
	return TRUE;
	}

#define ATTR_GENERAL_CHUNK		0x1001
#define ATTR_ATTSHAPE_CHUNK		0x1010

IOResult AttachRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case ATTR_GENERAL_CHUNK:
				res = iload->Read(&oldSplineCount,sizeof(int),&nb);
				break;
			case ATTR_ATTSHAPE_CHUNK:
				res = attShape.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

static void ChangeVertexType(BezierShape *shape, int poly, int vertex, int type) {
	Spline3D *spline = shape->splines[poly];
	BOOL resetKnotTangent = (type == KTYPE_RESET) ? TRUE : FALSE;
	
	// If positive vertex number, do it to just one vertex
	if(vertex >= 0) {
		// CAL-02/19/03: reset knot tangent. (FID #827)
		if (resetKnotTangent) {
			int oldType = spline->GetKnotType(vertex);
			if (oldType == KTYPE_BEZIER || oldType == KTYPE_BEZIER_CORNER) {
				spline->SetKnotType(vertex, KTYPE_AUTO);
				spline->ComputeBezPoints();
				spline->SetKnotType(vertex, oldType);
			}
		} else {
			spline->SetKnotType(vertex, type);
			}
		spline->ComputeBezPoints();
		shape->InvalidateGeomCache();
		return;
		}

	// Otherwise, do it to all selected vertices!
	int knots = spline->KnotCount();
	BitArray &vsel = shape->vertSel[poly];
	for(int i = 0; i < knots; ++i) {
		if(vsel[i*3+1])
			// CAL-02/19/03: reset knot tangent. (FID #827)
			if (resetKnotTangent) {
				int oldType = spline->GetKnotType(i);
				if (oldType == KTYPE_BEZIER || oldType == KTYPE_BEZIER_CORNER) {
					spline->SetKnotType(i, KTYPE_AUTO);
					spline->ComputeBezPoints();
					spline->SetKnotType(i, oldType);
				}
			} else {
				spline->SetKnotType(i, type);
				}
		}
	spline->ComputeBezPoints();
	shape->InvalidateGeomCache();
	}

BOOL VertChangeRecord::Redo(BezierShape *shape,int reRecord) {
	if(poly >= shape->splineCount || (vertex > 0 && vertex >= shape->splines[poly]->KnotCount()))
		return FALSE;
	if(reRecord) {
		oldSpline = *shape->splines[poly];
		}
	ChangeVertexType(shape, poly, vertex, type);
	return TRUE;
	}

#define VCHG_GENERAL_CHUNK		0x1001
#define VCHG_SPLINE_CHUNK		0x1010

IOResult VertChangeRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case VCHG_GENERAL_CHUNK:
				res = iload->Read(&poly,sizeof(int),&nb);
				res = iload->Read(&vertex,sizeof(int),&nb);
				res = iload->Read(&type,sizeof(int),&nb);
				break;
			case VCHG_SPLINE_CHUNK:
				res = oldSpline.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

static void ChangeSegmentType(BezierShape *shape, int poly, int segment, int type) {
	Spline3D *spline = shape->splines[poly];
	
	// If positive segment number, do it to just one segment
	if(segment >= 0) {
		spline->SetLineType(segment, type);
		spline->ComputeBezPoints();
		shape->InvalidateGeomCache();
		return;
		}

	// Otherwise, do it to all selected vertices!
	int segments = spline->Segments();
	BitArray &ssel = shape->segSel[poly];
	for(int i = 0; i < segments; ++i) {
		if(ssel[i])
			spline->SetLineType(i, type);
		}
	spline->ComputeBezPoints();
	shape->InvalidateGeomCache();
	}

BOOL SegChangeRecord::Redo(BezierShape *shape,int reRecord) {
	if(poly >= shape->splineCount || (segment > 0 && segment >= shape->splines[poly]->Segments()))
		return FALSE;
	if(reRecord) {
		oldSpline = *shape->splines[poly];
		}
	ChangeSegmentType(shape, poly, segment, type);
	return TRUE;
	}

#define SCHG_GENERAL_CHUNK		0x1001
#define SCHG_SPLINE_CHUNK		0x1010

IOResult SegChangeRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case SCHG_GENERAL_CHUNK:
				res = iload->Read(&poly,sizeof(int),&nb);
				res = iload->Read(&segment,sizeof(int),&nb);
				res = iload->Read(&type,sizeof(int),&nb);
				break;
			case SCHG_SPLINE_CHUNK:
				res = oldSpline.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

static void ChangePolyType(BezierShape *shape, int poly, int type) {
	Spline3D *spline = shape->splines[poly];
	int i;
	int knots = spline->KnotCount();
	int segments = spline->Segments();
	switch(type) {
		case LTYPE_CURVE:
			for(i = 0; i < knots; ++i)
				spline->SetKnotType(i, KTYPE_AUTO);
			spline->ComputeBezPoints();
			for(i = 0; i < knots; ++i)
				spline->SetKnotType(i, KTYPE_BEZIER);
			break;
		case LTYPE_LINE:
			for(int i = 0; i < knots; ++i)
				spline->SetKnotType(i, KTYPE_CORNER);
			break;
		}
	for(i = 0; i < segments; ++i)
		spline->SetLineType(i, type);
	spline->ComputeBezPoints();
	shape->InvalidateGeomCache();
	}

BOOL PolyChangeRecord::Redo(BezierShape *shape,int reRecord) {
	if(poly >= shape->splineCount)
		return FALSE;
	if(reRecord) {
		oldSpline = *shape->splines[poly];
		}
	ChangePolyType(shape, poly, type);
	return TRUE;
	}

#define PCHG_GENERAL_CHUNK		0x1001
#define PCHG_SPLINE_CHUNK		0x1010

IOResult PolyChangeRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case PCHG_GENERAL_CHUNK:
				res = iload->Read(&poly,sizeof(int),&nb);
				res = iload->Read(&type,sizeof(int),&nb);
				break;
			case PCHG_SPLINE_CHUNK:
				res = oldSpline.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

// Delete all selected vertices
// Returns TRUE if the operation results in the polygon being deleted
int DeleteSelVerts(BezierShape *shape, int poly) {
	Spline3D *spline = shape->splines[poly];
	int segs = spline->Segments();
	int knots = spline->KnotCount();

	// If less than two vertices would remain, blow it away!
	if((knots - shape->vertSel[poly].NumberSet()) < 2) {
		shape->DeleteSpline(poly);
//5-27-99 watje
//		shape->UpdateBindList();

		return TRUE;				// It's TRUE -- We deleted the spline!
		}

	int altered = 0;
	BitArray& vsel = shape->vertSel[poly];
	BitArray& ssel = shape->segSel[poly];

	for(int k = knots-1; k >= 0; --k) {
		if(vsel[k*3+1]) {
			altered = 1;
			spline->DeleteKnot(k);
			}
		}

	if(altered) {
		vsel.SetSize(spline->Verts(),1);
		vsel.ClearAll();
		ssel.SetSize(spline->Segments(),1);
		ssel.ClearAll();
//3-31-99 watje
//		shape->UpdateBindList();
		spline->ComputeBezPoints();
		shape->InvalidateGeomCache();
		}
	return FALSE;	// The poly's still there
	}

BOOL VertDeleteRecord::Redo(BezierShape *shape,int reRecord) {
	if(poly >= shape->splineCount)
		return FALSE;
	if(reRecord) {
		oldSpline = *(shape->splines[poly]);
		oldVSel = shape->vertSel[poly];
		oldSSel = shape->segSel[poly];
		selected = shape->polySel[poly];
		}
	deleted = DeleteSelVerts(shape, poly);
	return TRUE;
	}

#define VDELR_GENERAL_CHUNK		0x1000
#define VDELR_OLDVSEL_CHUNK		0x1040
#define VDELR_OLDSSEL_CHUNK		0x1050
#define VDELR_SPLINE_CHUNK		0x1060

IOResult VertDeleteRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case VDELR_GENERAL_CHUNK:
				res = iload->Read(&poly,sizeof(int),&nb);
				res = iload->Read(&selected,sizeof(int),&nb);
				res = iload->Read(&deleted,sizeof(int),&nb);
				res = iload->Read(&oldSplineCount,sizeof(int),&nb);
				break;
			case VDELR_OLDVSEL_CHUNK:
				res = oldVSel.Load(iload);
				break;
			case VDELR_OLDSSEL_CHUNK:
				res = oldSSel.Load(iload);
				break;
			case VDELR_SPLINE_CHUNK:
				res = oldSpline.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

BOOL CreateLineRecord::Redo(BezierShape *shape,int reRecord) {
	shape->InsertSpline(&spline,shape->splineCount);
	shape->UpdateSels();
	return TRUE;
	}

#define CRELR_SPLINE_CHUNK		0x1000

IOResult CreateLineRecord::Load(ILoad *iload) {
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case CRELR_SPLINE_CHUNK:
				res = spline.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

static void FlushSplineAux(Spline3D *spline) {
	for(int i = 0; i < spline->KnotCount(); ++i) {
		spline->SetKnotAux(i, 1, -1);
		spline->SetInAux(i, 1, -1);
		spline->SetOutAux(i, 1, -1);
		}
	}

static void CopySpline(BezierShape *shape, int poly, BOOL selOriginal, BOOL selCopy) {
	Spline3D *spline0 = shape->splines[poly];
	Spline3D *spline1 = shape->NewSpline();
	int newPoly = shape->SplineCount() - 1;
	*spline1 = *spline0;
	FlushSplineAux(spline1);

	shape->UpdateSels(TRUE);	// Make sure it readies the selection set info
	shape->InvalidateGeomCache();
	shape->polySel.Set(poly, selOriginal);
	shape->polySel.SetSize(*shape);		// Expand the selection array
	shape->polySel.Set(newPoly, selCopy);
	}

BOOL PolyCopyRecord::Redo(BezierShape *shape,int reRecord) {
	if(poly >= shape->splineCount)
		return FALSE;
	CopySpline(shape, poly, selOrig, selCopy);
	return TRUE;
	}

#define PCOPYR_GENERAL_CHUNK		0x1000

IOResult PolyCopyRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case PCOPYR_GENERAL_CHUNK:
				res = iload->Read(&poly,sizeof(int),&nb);
				res = iload->Read(&selOrig,sizeof(BOOL),&nb);
				res = iload->Read(&selCopy,sizeof(BOOL),&nb);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/		

static void CopySegments(BezierShape *shape, int poly, BOOL selOriginal, BOOL selCopy) {
	int oldSplineCount = shape->SplineCount();

	if(shape->segSel[poly].NumberSet()) {
		Spline3D *spline = shape->splines[poly];
		int segments = spline->Segments();
		int knots = spline->KnotCount();
		// If all segments selected, copy the whole polygon!
		if(shape->segSel[poly].NumberSet() == segments) {
			Spline3D *newSpline = shape->NewSpline();
			*newSpline = *spline;
			FlushSplineAux(newSpline);
			}
		else {
			int end = segments;
			for(int seg = 0; seg < end; ++seg) {
				if(shape->segSel[poly][seg]) {
					Spline3D *newSpline = shape->NewSpline();
					if(seg == 0 && spline->Closed()) {
						backLoop:
						if(shape->segSel[poly][--end]) {
							SplineKnot addKnot(spline->GetKnotType(end),spline->GetLineType(end),
								spline->GetKnotPoint(end),spline->GetInVec(end),spline->GetOutVec(end));
							newSpline->AddKnot(addKnot, 0);
							goto backLoop;
							}

						}
					SplineKnot addKnot(spline->GetKnotType(seg),spline->GetLineType(seg),
						spline->GetKnotPoint(seg),spline->GetInVec(seg),spline->GetOutVec(seg));
					newSpline->AddKnot(addKnot, -1);

					loop:
					int knot = (seg + 1) % knots;
					addKnot = SplineKnot(spline->GetKnotType(knot),spline->GetLineType(knot),
						spline->GetKnotPoint(knot),spline->GetInVec(knot),spline->GetOutVec(knot));
					newSpline->AddKnot(addKnot, -1);
					seg = (seg + 1) % segments;
					if(seg > 0 && seg < end && shape->segSel[poly][seg])
						goto loop;

					// Finish up the spline!
					newSpline->ComputeBezPoints();
					// Flush the Aux values in the copy
					FlushSplineAux(newSpline);

					// Special termination test for wraparound
					if(seg == 0)
						seg = end;
					}
				}
			}
		}
	
	if (!selOriginal)
		shape->segSel[poly].ClearAll();	// Deselect the originals
	shape->UpdateSels(TRUE);	// Make sure it readies the selection set info
	shape->InvalidateGeomCache();
	int newSplineCount = shape->SplineCount();
	for(int i = oldSplineCount; i < newSplineCount; ++i) {
		if(selCopy)
			shape->segSel[i].SetAll();
		else
			shape->segSel[i].ClearAll();
		}
	}

BOOL SegCopyRecord::Redo(BezierShape *shape,int reRecord) {
	if(reRecord) {
		oldSplineCount = shape->splineCount;
		oldSSel = shape->segSel;
		}
	int splCount = shape->SplineCount();
	for(int poly = 0; poly < splCount; ++poly)
		CopySegments(shape, poly, FALSE, selCopy);
	return TRUE;
	}

#define SCOPYR_GENERAL_CHUNK		0x1000

IOResult SegCopyRecord::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case SCOPYR_GENERAL_CHUNK:
				res = iload->Read(&selCopy,sizeof(BOOL),&nb);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

/*-------------------------------------------------------------------*/

// CAL-05/23/03: add connecting splines for newly added splines. (FID #827)
static void AddConnectingSplines(BezierShape *shape, int oldSplineCount, BOOL linkToOld, float linkToOldThresh, BitArray &excludeEnds, int knotType)
{
	int newSplineCount = shape->SplineCount();
	float sqrDistThresh = linkToOldThresh * linkToOldThresh;

	for (int poly = oldSplineCount; poly < newSplineCount; poly++) {
		Spline3D *spline = shape->splines[poly];
		int knots = spline->KnotCount();
		for (int k = 0; k < knots; k++) {
			Point3 &p = spline->GetKnotPoint(k);
			SplineKnot newKnot(knotType /* KTYPE_AUTO */, LTYPE_CURVE, p, p, p);
			Spline3D *linkSpline = NULL;
			int where = -1;

			// link to old splines, if there's a spline's end/start knot at the same position.
			if (linkToOld) {
				for (int oldPoly = 0; oldPoly < oldSplineCount; oldPoly++) {
					Spline3D *oldSpline = shape->splines[oldPoly];
					// CAL-06/24/03: skip closed splines.
					if (oldSpline->Closed()) continue;

					int oldKnots = oldSpline->KnotCount();
					if (!excludeEnds[2*oldPoly+1] && oldKnots > 1 &&
						LengthSquared(oldSpline->GetKnotPoint(oldKnots-1) - p) < sqrDistThresh) {
						linkSpline = oldSpline;		where = -1;		// add knot to end
						excludeEnds.Set(2*oldPoly+1);				// not to be used again
						break;
					}
					if (!excludeEnds[2*oldPoly] && oldKnots > 0 &&
						LengthSquared(oldSpline->GetKnotPoint(0) - p) < sqrDistThresh) {
						linkSpline = oldSpline;		where = 0;		// add knot to start
						excludeEnds.Set(2*oldPoly);					// not to be used again
						break;
					}
				}
			}
			// if not connected to old spline, create a new spline and add the first knot.
			if (linkSpline == NULL) {
				linkSpline = shape->NewSpline();
				linkSpline->AddKnot(newKnot);
			}

			newKnot.SetFlag(SPLINEKNOT_ADD_SEL);	// set additional selection to be transformed
			linkSpline->AddKnot(newKnot, where);
			linkSpline->ComputeBezPoints();
			linkSpline->SetOpen();
		}
	}

	shape->UpdateSels(TRUE);	// Make sure it readies the selection set info
	shape->InvalidateGeomCache();
}

/*-------------------------------------------------------------------*/

ShapeRestore::ShapeRestore(EditSplineData* sd, EditSplineMod* mod, BezierShape *shape)
	{
	gotRedo = FALSE;
	esd = sd;
	this->mod = mod;
	oldShape = *shape;
	t = mod->ip->GetTime();
	undoPointList = mod->pointList;
	}

void ShapeRestore::Restore(int isUndo)
	{
	if ( esd->tempData && esd->TempData(mod)->ShapeCached(t) ) {
		BezierShape *shape = esd->TempData(mod)->GetShape(t);
		if(shape) {
			if(isUndo && !gotRedo) {
				newShape = *shape;
				redoPointList = mod->pointList;
				gotRedo = TRUE;
				}
			DWORD selLevel = shape->selLevel;	// Grab this...
			DWORD dispFlags = shape->dispFlags;	// Grab this...
			*shape = oldShape;
			shape->selLevel = selLevel;	// ...and put it back in
			shape->dispFlags = dispFlags;	// ...and put it back in
			mod->pointList = undoPointList;
			mod->InvalidateSurfaceUI();
			shape->InvalidateGeomCache();
			}
		esd->TempData(mod)->Invalidate(PART_GEOM | PART_TOPO | PART_SELECT);
		}
	else
	if ( esd->tempData ) {
		esd->TempData(mod)->Invalidate(PART_GEOM | PART_TOPO | PART_SELECT, FALSE);
		}
	if(isUndo)
		MaybeCancelBooleanMode(mod);
	mod->SelectionChanged();
	mod->NotifyDependents(FOREVER, PART_GEOM | PART_TOPO | PART_SELECT, REFMSG_CHANGE);
	}

void ShapeRestore::Redo()
	{
	if ( esd->tempData && esd->TempData(mod)->ShapeCached(t) ) {
		BezierShape *shape = esd->TempData(mod)->GetShape(t);
		if(shape) {
			DWORD selLevel = shape->selLevel;	// Grab this...
			DWORD dispFlags = shape->dispFlags;	// Grab this...
			*shape = newShape;
			shape->selLevel = selLevel;	// ...and put it back in
			shape->dispFlags = dispFlags;	// ...and put it back in
			shape->InvalidateGeomCache();
			mod->InvalidateSurfaceUI();
			}
		esd->TempData(mod)->Invalidate(PART_GEOM | PART_TOPO | PART_SELECT);
		}
	else
	if ( esd->tempData ) {
		esd->TempData(mod)->Invalidate(PART_GEOM | PART_TOPO | PART_SELECT, FALSE);
		}
	MaybeCancelBooleanMode(mod);
	mod->pointList = redoPointList;

	mod->SelectionChanged();
	mod->NotifyDependents(FOREVER, PART_GEOM | PART_TOPO | PART_SELECT, REFMSG_CHANGE);
	}

//--- Named Selection Set Methods ------------------------------------

int EditSplineMod::FindSet(TSTR &setName,int level)
	{	
	assert(level>0 && level<4);
	for (int i=0; i<namedSel[level-1].Count(); i++) {
		if (setName == *namedSel[level-1][i]) {
			return i;			
			}
		}
	return -1;
	}

void EditSplineMod::AddSet(TSTR &setName,int level)
	{
	assert(level>0 && level<4);
	TSTR *name = new TSTR(setName);
	namedSel[level-1].Append(1,&name);
	}

void EditSplineMod::RemoveSet(TSTR &setName,int level)
	{
	assert(level>0 && level<4);
	int i = FindSet(setName,level);
	if (i>=0) {
		delete namedSel[level-1][i];
		namedSel[level-1].Delete(i,1);
		}
	}

void EditSplineMod::RemoveAllSets()
	{
	ModContextList mcList;
	INodeTab nodes;
	
	if (!ip) return;	
	
	ip->GetModContexts(mcList,nodes);

	for (int i=0; i < mcList.Count(); i++) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if (!shapeData) continue;		
		
		int j;
		for (j=shapeData->vselSet.Count()-1; j>=0; j--) {
			shapeData->vselSet.DeleteSet(j);
			}
		for (j=shapeData->sselSet.Count()-1; j>=0; j--) {
			shapeData->sselSet.DeleteSet(j);
			}
		for (j=shapeData->pselSet.Count()-1; j>=0; j--) {
			shapeData->pselSet.DeleteSet(j);
			}		
		}	
	
	for (int j=0; j<3; j++) {
		for (int i=0; i<namedSel[j].Count(); i++) {
			delete namedSel[j][i];		
			}
		namedSel[j].Resize(0);
		}

	ip->ClearCurNamedSelSet();
	ip->ClearSubObjectNamedSelSets();
	nodes.DisposeTemporary();
	}

// Used by EditPatchMod destructor to free pointers
void EditSplineMod::ClearSetNames()
	{
	for (int i=0; i<3; i++) {
		for (int j=0; j<namedSel[i].Count(); j++) {
			delete namedSel[i][j];
			namedSel[i][j] = NULL;
			}
		}
	}

/*-------------------------------------------------------------------*/

void EditSplineMod::SelectBySegment() {
	int level = GetSubobjectLevel();
	if(level != ES_VERTEX)
		return;
	if ( !ip ) return;	
	ModContextList mcList;
	INodeTab nodes;
	theHold.Begin();
	ip->ClearCurNamedSelSet();
	EndMoveModes(ip->GetTime());
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;		
		BezierShape *shape = shapeData->TempData(this)->GetShape(ip->GetTime());
		if(!shape) continue;
		shapeData->BeginEdit(ip->GetTime());
		shapeData->PreUpdateChanges(shape, FALSE);
		if ( theHold.Holding() ) {
			theHold.Put(new ShapeRestore(shapeData,this,shape));
			}
		for(int poly = 0; poly < shape->SplineCount(); ++poly)
			shape->vertSel[poly] = shape->VertexTempSel(poly, SHAPE_SEGMENT);
		shape->UnselectHiddenVerts();
		shapeData->UpdateChanges(shape, FALSE);
		}
	nodes.DisposeTemporary();
	theHold.Accept(GetString(IDS_DS_SELECT));
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	SelectionChanged();
	}

void EditSplineMod::SelectBySpline() {
	int level = GetSubobjectLevel();
	if(level != ES_VERTEX && level != ES_SEGMENT)
		return;
	if ( !ip ) return;	
	ModContextList mcList;
	INodeTab nodes;
	theHold.Begin();
	ip->ClearCurNamedSelSet();
	EndMoveModes(ip->GetTime());
	ip->GetModContexts(mcList,nodes);
	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;		
		BezierShape *shape = shapeData->TempData(this)->GetShape(ip->GetTime());
		if(!shape) continue;
		shapeData->BeginEdit(ip->GetTime());
		shapeData->PreUpdateChanges(shape, FALSE);
		if ( theHold.Holding() ) {
			theHold.Put(new ShapeRestore(shapeData,this,shape));
			}
		switch(level) {
			case ES_VERTEX: {
				for(int poly = 0; poly < shape->SplineCount(); ++poly)
					shape->vertSel[poly] = shape->VertexTempSel(poly, SHAPE_SPLINE);
				shape->UnselectHiddenVerts();
				}
				break;
			case ES_SEGMENT: {
				shape->segSel.ClearAll();
				for(int poly = 0; poly < shape->SplineCount(); ++poly) {
					if(shape->polySel[poly])
						shape->segSel[poly].SetAll();
					}
				shape->UnselectHiddenSegs();
				}
				break;
			}
		shapeData->UpdateChanges(shape, FALSE);
		}
	theHold.Accept(GetString(IDS_DS_SELECT));
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	SelectionChanged();
	}
	 
void EditSplineMod::ActivateSubSelSet(TSTR &setName)
	{
	ModContextList mcList;
	INodeTab nodes;
	int index = FindSet(setName,selLevel);
	if (index<0 || !ip) return;	
	TimeValue t = ip->GetTime();

	ip->GetModContexts(mcList,nodes);

	theHold.Begin();
	for (int i = 0; i < mcList.Count(); i++) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		shapeData->BeginEdit(ip->GetTime());
		shapeData->PreUpdateChanges(shape, FALSE);
		switch (GetSubobjectLevel()) {
			case ES_VERTEX: {
				int i = shapeData->vselSet.FindSet(setName);
				if (i>=0) {
					if (!shapeData->vselSet[i].IsCompatible(*shape))
						shapeData->vselSet[i].SetSize(*shape, TRUE);
					if(theHold.Holding())
						theHold.Put(new ShapeRestore(shapeData,this,shape));
					shape->vertSel = shapeData->vselSet[i];
					SelectionChanged();
					}
				break;
				}

			case ES_SEGMENT: {
				int i = shapeData->sselSet.FindSet(setName);
				if (i>=0) {
					if (!shapeData->sselSet[i].IsCompatible(*shape))
						shapeData->sselSet[i].SetSize(*shape, TRUE);
					if(theHold.Holding())
						theHold.Put(new ShapeRestore(shapeData,this,shape));
					shape->segSel = shapeData->sselSet[i];
					SelectionChanged();
					}
				break;
				}

			case ES_SPLINE: {
				int i = shapeData->pselSet.FindSet(setName);
				if (i>=0) {
					if (!shapeData->pselSet[i].IsCompatible(*shape))
						shapeData->pselSet[i].SetSize(*shape, TRUE);
					if(theHold.Holding())
						theHold.Put(new ShapeRestore(shapeData,this,shape));
					shape->polySel = shapeData->pselSet[i];
					SelectionChanged();
					}
				break;
				}
			}
		shapeData->UpdateChanges(shape, FALSE);
		if (shapeData->tempData)
			shapeData->TempData(this)->Invalidate(PART_SELECT);
		}
	
	theHold.Accept(GetString(IDS_DS_SELECT));
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
	}

void EditSplineMod::NewSetFromCurSel(TSTR &setName)
	{	
	ModContextList mcList;
	INodeTab nodes;
	if(!ip)
		return;
	TimeValue t = ip->GetTime();
	ip->GetModContexts(mcList,nodes);

	for (int i = 0; i < mcList.Count(); i++) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		shapeData->BeginEdit(ip->GetTime());
		switch (GetSubobjectLevel()) {
			case ES_VERTEX: {
				int j = shapeData->vselSet.FindSet(setName);
				if (j>=0) {
					shapeData->vselSet[j] = shape->vertSel;
				} else {
					shapeData->vselSet.AppendSet(shape->vertSel,setName);
					}
				break;
				}

			case ES_SEGMENT: {
				int j = shapeData->sselSet.FindSet(setName);
				if (j>=0) {
					shapeData->sselSet[j] = shape->segSel;
				} else {
					shapeData->sselSet.AppendSet(shape->segSel,setName);
					}
				break;
				}

			case ES_SPLINE: {
				int j = shapeData->pselSet.FindSet(setName);
				if (j>=0) {
					shapeData->pselSet[j] = shape->polySel;
				} else {
					shapeData->pselSet.AppendSet(shape->polySel,setName);
					}
				break;
				}
			}
		}
	int index = FindSet(setName,selLevel);
	if (index<0) {
		if(theHold.Holding())
			theHold.Put(new ESAddSetRestore(setName, selLevel, this));
		AddSet(setName,selLevel);
		}
	nodes.DisposeTemporary();
	}

void EditSplineMod::RemoveSubSelSet(TSTR &setName)
	{
	ModContextList mcList;
	INodeTab nodes;
	if(!ip)
		return;
	TimeValue t = ip->GetTime();
	ip->GetModContexts(mcList,nodes);

	for (int i = 0; i < mcList.Count(); i++) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		shapeData->BeginEdit(ip->GetTime());
		switch(GetSubobjectLevel()) {
			case ES_VERTEX:
				if (theHold.Holding())
					theHold.Put(new ESDeleteVertSetRestore(i,&shapeData->vselSet,this));
				shapeData->vselSet.DeleteSet(setName);
				break;
			case ES_SEGMENT:
				if (theHold.Holding())
					theHold.Put(new ESDeleteSegSetRestore(i,&shapeData->sselSet,this));
				shapeData->sselSet.DeleteSet(setName);
				break;
			case ES_SPLINE:
				if (theHold.Holding())
					theHold.Put(new ESDeletePolySetRestore(i,&shapeData->pselSet,this));
				shapeData->pselSet.DeleteSet(setName);
				break;
			}
		}
	// Remove the modifier's entry
	RemoveSet(setName,selLevel);
	ip->ClearCurNamedSelSet();
	SetupNamedSelDropDown();
	nodes.DisposeTemporary();
	}

void EditSplineMod::SetupNamedSelDropDown()
	{
	// Setup named selection sets	
	if (selLevel == ES_OBJECT)
		return;
	ip->ClearSubObjectNamedSelSets();
	for (int i=0; i<namedSel[selLevel-1].Count(); i++)
		ip->AppendSubObjectNamedSelSet(*namedSel[selLevel-1][i]);
	}

int EditSplineMod::NumNamedSelSets()
	{
	if(GetSubobjectLevel() == ES_OBJECT)
		return 0;
	return namedSel[selLevel-1].Count();
	}

TSTR EditSplineMod::GetNamedSelSetName(int i)
	{
	return *namedSel[selLevel-1][i];
	}

class ESSelSetNameRestore : public RestoreObj {
	public:
		TSTR undo, redo;
		TSTR *target;
		EditSplineMod *mod;
		ESSelSetNameRestore(EditSplineMod *m, TSTR *t, TSTR &newName) {
			mod = m;
			undo = *t;
			target = t;
			}
		void Restore(int isUndo) {			
			if(isUndo)
				redo = *target;
			*target = undo;
			if (mod->ip)
				mod->ip->NamedSelSetListChanged();
			}
		void Redo() {
			*target = redo;
			if (mod->ip)
				mod->ip->NamedSelSetListChanged();
			}
				
		TSTR Description() {return TSTR(_T("Sel Set Name"));}
	};

void EditSplineMod::SetNamedSelSetName(int index,TSTR &newName)
	{
	if(!ip) return;

	// First do the master name list
	if (theHold.Holding())
		theHold.Put(new ESSelSetNameRestore(this, namedSel[selLevel-1][index], newName));

	// Save the old name so we can change those in the EditSplineData
	TSTR oldName = *namedSel[selLevel-1][index];
	*namedSel[selLevel-1][index] = newName;

	ModContextList mcList;
	INodeTab nodes;
	
	ip->GetModContexts(mcList,nodes);

	for (int i = 0; i < mcList.Count(); i++) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( shapeData ) {
			switch(GetSubobjectLevel()) {
				case ES_VERTEX:
					if (theHold.Holding())
						theHold.Put(new ESVertSetNameRestore(i,&shapeData->vselSet,this));
					shapeData->vselSet.RenameSet(oldName, newName);
					break;
				case ES_SEGMENT:
					if (theHold.Holding())
						theHold.Put(new ESSegSetNameRestore(i,&shapeData->sselSet,this));
					shapeData->sselSet.RenameSet(oldName, newName);
					break;
				case ES_SPLINE:
					if (theHold.Holding())
						theHold.Put(new ESPolySetNameRestore(i,&shapeData->pselSet,this));
					shapeData->pselSet.RenameSet(oldName, newName);
					break;
				}
			}
		}
	nodes.DisposeTemporary();
	}

void EditSplineMod::NewSetByOperator(TSTR &newName,Tab<int> &sets,int op)
	{
	// First do it in the master name list
	if(theHold.Holding())
		theHold.Put(new ESAddSetRestore(newName, selLevel, this));
	AddSet(newName,selLevel);		

	ModContextList mcList;
	INodeTab nodes;
	if(!ip)
		return;
	TimeValue t = ip->GetTime();
	ip->GetModContexts(mcList,nodes);

//	theHold.Begin();
	for (int i = 0; i < mcList.Count(); i++) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		shapeData->BeginEdit(ip->GetTime());
		int poly, j;
		
		switch(GetSubobjectLevel()) {
			case ES_VERTEX: {
				ShapeVSel bits = *(shapeData->vselSet.sets[sets[0]]);
				for(poly = 0; poly < bits.polys; ++poly) {
					for (j=1; j<sets.Count(); j++) {
						ShapeVSel &bit2 = *(shapeData->vselSet.sets[sets[j]]);
						switch (op) {
							case NEWSET_MERGE:
								bits[poly] |= bit2[poly];
								break;

							case NEWSET_INTERSECTION:
								bits[poly] &= bit2[poly];
								break;

							case NEWSET_SUBTRACT:
								bits[poly] &= ~(bit2[poly]);
								break;
							}
						}
					}
				shapeData->vselSet.AppendSet(bits,newName);
				if (theHold.Holding())
					theHold.Put(new ESAppendVertSetRestore(&shapeData->vselSet,this));
				}
				break;
			case ES_SEGMENT: {
				ShapeSSel bits = *(shapeData->sselSet.sets[sets[0]]);
				for(poly = 0; poly < bits.polys; ++poly) {
					for (j=1; j<sets.Count(); j++) {
						ShapeSSel &bit2 = *(shapeData->sselSet.sets[sets[j]]);
						switch (op) {
							case NEWSET_MERGE:
								bits[poly] |= bit2[poly];
								break;

							case NEWSET_INTERSECTION:
								bits[poly] &= bit2[poly];
								break;

							case NEWSET_SUBTRACT:
								bits[poly] &= ~(bit2[poly]);
								break;
							}
						}
					}
				shapeData->sselSet.AppendSet(bits,newName);
				if (theHold.Holding())
					theHold.Put(new ESAppendSegSetRestore(&shapeData->sselSet,this));
				}
				break;
			case ES_SPLINE: {
				ShapePSel bits = *(shapeData->pselSet.sets[sets[0]]);
				for (j=1; j<sets.Count(); j++) {
					ShapePSel &bit2 = *(shapeData->pselSet.sets[sets[j]]);
					switch (op) {
						case NEWSET_MERGE:
							bits.sel |= bit2.sel;
							break;

						case NEWSET_INTERSECTION:
							bits.sel &= bit2.sel;
							break;

						case NEWSET_SUBTRACT:
							bits.sel &= ~(bit2.sel);
							break;
						}
					}
				shapeData->pselSet.AppendSet(bits,newName);
				if (theHold.Holding())
					theHold.Put(new ESAppendPolySetRestore(&shapeData->pselSet,this));
				}
				break;
			}
		}
	}

// Named selection set copy/paste methods follow...

static INT_PTR CALLBACK PickSetNameDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
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

BOOL EditSplineMod::GetUniqueSetName(TSTR &name)
	{
	while (1) {		
		if(FindSet(name, selLevel) < 0)
			break;

		if (!DialogBoxParam(
			hInstance, 
			MAKEINTRESOURCE(IDD_PASTE_NAMEDSET),
			ip->GetMAXHWnd(), 
			PickSetNameDlgProc,
			(LPARAM)&name)) return FALSE;		
		}
	return TRUE;
	}

static INT_PTR CALLBACK PickSetDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{	
	switch (msg) {
		case WM_INITDIALOG:	{
			Tab<TSTR*> &names = *((Tab<TSTR*>*)lParam);
			for (int i=0; i<names.Count(); i++) {
				int pos  = SendDlgItemMessage(hWnd,IDC_NS_LIST,LB_ADDSTRING,0,
					(LPARAM)(TCHAR*)*names[i]);
				SendDlgItemMessage(hWnd,IDC_NS_LIST,LB_SETITEMDATA,pos,i);
				}
			break;
			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_NS_LIST:
					if (HIWORD(wParam)!=LBN_DBLCLK) break;
					// fall through
				case IDOK: {
					int sel = SendDlgItemMessage(hWnd,IDC_NS_LIST,LB_GETCURSEL,0,0);
					if (sel!=LB_ERR) {
						int res =SendDlgItemMessage(hWnd,IDC_NS_LIST,LB_GETITEMDATA,sel,0);
						EndDialog(hWnd,res);
						break;
						}
					// fall through
					}

				case IDCANCEL:
					EndDialog(hWnd,-1);
					break;
				}
			break;

		default:
			return FALSE;
		};
	return TRUE;
	}

int EditSplineMod::SelectNamedSet()
	{
	Tab<TSTR*> names = namedSel[selLevel-1];
	return DialogBoxParam(
		hInstance, 
		MAKEINTRESOURCE(IDD_SEL_NAMEDSET),
		ip->GetMAXHWnd(), 
		PickSetDlgProc,
		(LPARAM)&names);
	}

void EditSplineMod::NSCopy()
	{
	if (selLevel == ES_OBJECT) return;
	int index = SelectNamedSet();
	if(index < 0) return;
	if(!ip) return;

	// Get the name for that index
	int nsl = namedSetLevel[selLevel];
	TSTR setName = *namedSel[nsl][index];

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	for (int i = 0; i < mcList.Count(); i++) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		
		switch (GetSubobjectLevel()) {
			case ES_VERTEX: {
				ShapeNamedVertSelClip *clip = new ShapeNamedVertSelClip(*(shapeData->vselSet.names[index]));
				ShapeVSel *sel = new ShapeVSel(*(shapeData->vselSet.sets[index]));
				clip->sets.Append(1,&sel);
				SetShapeNamedVertSelClip(clip);
				}
				break;
			case ES_SEGMENT: {
				ShapeNamedSegSelClip *clip = new ShapeNamedSegSelClip(*(shapeData->sselSet.names[index]));
				ShapeSSel *sel = new ShapeSSel(*(shapeData->sselSet.sets[index]));
				clip->sets.Append(1,&sel);
				SetShapeNamedSegSelClip(clip);
				}
				break;
			case ES_SPLINE: {
				ShapeNamedPolySelClip *clip = new ShapeNamedPolySelClip(*(shapeData->pselSet.names[index]));
				ShapePSel *sel = new ShapePSel(*(shapeData->pselSet.sets[index]));
				clip->sets.Append(1,&sel);
				SetShapeNamedPolySelClip(clip);
				}
				break;
			}		
		}
	// Enable the paste button
	ICustButton *but = GetICustButton(GetDlgItem(hSelectPanel,IDC_NS_PASTE));
	but->Enable();
	ReleaseICustButton(but);
	}

void EditSplineMod::NSPaste()
	{	
	if (selLevel == ES_OBJECT) return;
	int nsl = namedSetLevel[selLevel];
	if(!ip) return;

	ShapeNamedVertSelClip *vclip;
	ShapeNamedSegSelClip *sclip;
	ShapeNamedPolySelClip *pclip;

	TSTR name;

	switch(GetSubobjectLevel()) {
		case ES_VERTEX:
			vclip = GetShapeNamedVertSelClip();
			if (!vclip) return;
			name = vclip->name;
			break;
		case ES_SEGMENT:
			sclip = GetShapeNamedSegSelClip();
			if (!sclip) return;
			name = sclip->name;
			break;
		case ES_SPLINE:
			pclip = GetShapeNamedPolySelClip();
			if (!pclip) return;
			name = pclip->name;
			break;
		}

	if (!GetUniqueSetName(name)) return;
	
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	theHold.Begin();

	if(theHold.Holding())
		theHold.Put(new ESAddSetRestore(name, selLevel, this));
	AddSet(name, selLevel);

	for (int i = 0; i < mcList.Count(); i++) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		switch(GetSubobjectLevel()) {
			case ES_VERTEX:
				shapeData->vselSet.AppendSet(*vclip->sets[0],name);	
				if ( theHold.Holding() )
					theHold.Put(new ESAppendVertSetRestore(&shapeData->vselSet,this));
				break;
			case ES_SEGMENT:
				shapeData->sselSet.AppendSet(*sclip->sets[0],name);	
				if ( theHold.Holding() )
					theHold.Put(new ESAppendSegSetRestore(&shapeData->sselSet,this));
				break;
			case ES_SPLINE:
				shapeData->pselSet.AppendSet(*pclip->sets[0],name);	
				if ( theHold.Holding() )
					theHold.Put(new ESAppendPolySetRestore(&shapeData->pselSet,this));
				break;
			}
		}

	ActivateSubSelSet(name);
	ip->SetCurNamedSelSet(name);
	theHold.Accept(GetString (IDS_TH_PASTE_NAMED_SEL));
	SetupNamedSelDropDown();
	}

/*-------------------------------------------------------------------*/

void EditSplineMod::EndMoveModes(TimeValue t, BOOL accept) {
	EndOutlineMove(t, accept);
	EndFilletMove(t, accept);
	EndChamferMove(t, accept);
	}

void OutlineCMode::EnterMode()
	{
	if ( es->hOpsPanel ) {
		ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel,IDC_ES_OUTLINE));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
		}
	}

void OutlineCMode::ExitMode()
	{
	if ( es->hOpsPanel ) {
		es->EndOutlineMove(es->ip->GetTime(),TRUE);
		ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel,IDC_ES_OUTLINE));
		but->SetCheck(FALSE);
		ReleaseICustButton(but);
		}
	}

void EditSplineMod::StartOutlineMode()
	{
	if ( !ip ) return;

	if (ip->GetCommandMode() == outlineMode) {
		ip->SetStdCommandMode(CID_OBJMOVE);
		return;
		}
	ip->SetCommandMode(outlineMode);
	}

int OutlineMouseProc::proc( 
		HWND hwnd, 
		int msg, 
		int point, 
		int flags, 
		IPoint2 m )
	{	
	ViewExp *vpt = ip->GetViewport(hwnd);
	switch ( msg ) {
		case MOUSE_PROPCLICK:
			ip->SetStdCommandMode(CID_OBJMOVE);
			break;

		case MOUSE_POINT:						
			if ( point == 0 ) {							
				es->BeginOutlineMove(ip->GetTime());				
				p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				sp0 = m;
			} else {
				float outSize = es->outlineSpin->GetFVal();
				es->EndOutlineMove(ip->GetTime(), (outSize == 0.0f) ? FALSE : TRUE);
				ip->RedrawViews(ip->GetTime(),REDRAW_END);
				}
			break;

		case MOUSE_MOVE: {
			float size = vpt->SnapLength(vpt->GetCPDisp(p0,Point3(0,0,1),sp0,m));
			es->OutlineMove( ip->GetTime(), size);
			es->outlineSpin->SetValue(size, FALSE);
			
			ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
			break;
			}

		case MOUSE_ABORT:
			es->EndOutlineMove(ip->GetTime(),FALSE);			
			ip->RedrawViews(ip->GetTime(),REDRAW_END);
			break;		
		}
	
	if ( vpt ) ip->ReleaseViewport(vpt);
	return TRUE;
	}

void EditSplineMod::BeginOutlineMove(TimeValue t)
	{
	EndOutlineMove(t);
	theHold.Begin();
	inOutline = TRUE;
	}

void EditSplineMod::EndOutlineMove(TimeValue t,BOOL accept)
	{	
	if ( !inOutline ) return;
	if ( !ip ) return;	
	
	if (accept) {
		// Record the changes that happened
		ModContextList mcList;		
		INodeTab nodes;
		ip->GetModContexts(mcList,nodes);
		ClearShapeDataFlag(mcList,ESD_BEENDONE);
		for ( int i = 0; i < mcList.Count(); i++ ) {
			EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
			if ( !shapeData ) continue;
			if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;
			BezierShape *shape = shapeData->TempData(this)->GetShape(t);
			if(!shape) continue;
			shapeData->UpdateChanges(shape);					
			shapeData->TempData(this)->Invalidate(PART_GEOM);
			shapeData->SetFlag(ESD_BEENDONE,TRUE);
			}

		nodes.DisposeTemporary();
		ClearShapeDataFlag(mcList,ESD_BEENDONE);
		theHold.Accept(GetString(IDS_TH_OUTLINE));
	} else {
		theHold.Cancel();
		}
		
	inOutline = FALSE;
	if ( outlineSpin ) {
		outlineSpin->SetValue(0,FALSE);
		}
	}

// mjm - begin - 4.3.99
// removes segments less than tol distance long 
static
bool CleanSpline(Spline3D *spline, double tol)
{
	bool changed(false);
	int segs = spline->Segments();
	if (segs > 0)
	{
		bool *deleteFlags = new bool[segs];

		for (int seg=0; seg<segs; ++seg)
			deleteFlags[seg] = (spline->SegmentLength(seg) < tol) ? true : false;

		for (int knot = segs-1; knot>=0; --knot)
		{
			if (deleteFlags[knot])
			{
				spline->DeleteKnot(knot);
				changed = true;
			}
		}

		// re-cache the spline
		spline->SplineLength();

		if (deleteFlags)
			delete[] deleteFlags;
	}
	return changed;
}

// checks for splines with degenerate segments 
#define CLEAN_TOLERANCE .00001
static
bool CleanShape(BezierShape *shape)
{
	bool changed(false);
	int polys = shape->splineCount;
	for (int poly=0; poly<polys; ++poly)
	{
		Spline3D *spline = shape->splines[poly];
		if ( spline && CleanSpline(spline, CLEAN_TOLERANCE * spline->SplineLength()) )
			changed = true;
	}
	if (changed)
		shape->InvalidateGeomCache();

	return changed;
}
// mjm - end

void EditSplineMod::OutlineMove( TimeValue t, float amount ) 
	{	
	ModContextList mcList;		
	INodeTab nodes;

	if ( !ip ) return;
	theHold.Restore();
	ip->GetModContexts(mcList,nodes);

	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
				
		shapeData->BeginEdit(t);
		shapeData->PreUpdateChanges(shape);

		// mjm - begin - 4.3.99
		if ( CleanShape(shape) )
		{
			shapeData->UpdateChanges(shape);
			shapeData->TempData(this)->Invalidate(PART_TOPO);
		}
		// mjm - end

		int polys = shape->splineCount;
		for(int poly = 0; poly < polys; ++poly) {				
			Spline3D *spline = shape->splines[poly];

			if(shape->polySel[poly]) {
				if ( theHold.Holding() && !TestAFlag(A_HELD) ) {
					shapeData->PreUpdateChanges(shape);
					theHold.Put(new ShapeRestore(shapeData,this,shape));
					}

				// Outline the polygon
				OutlineSpline(shape, poly, amount, centeredOutline, TRUE);
				}
			}
		shapeData->TempData(this)->Invalidate(PART_GEOM);
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}

	// Mark all objects in selection set
	SetAFlag(A_HELD);
	
	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
	}

/*-------------------------------------------------------------------*/

void FilletCMode::EnterMode()
	{
	if ( es->hOpsPanel ) {
		ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel,IDC_ES_FILLET));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
		}
	}

void FilletCMode::ExitMode()
	{
	if ( es->hOpsPanel ) {
		es->EndFilletMove(es->ip->GetTime(),TRUE);
		ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel,IDC_ES_FILLET));
		but->SetCheck(FALSE);
		ReleaseICustButton(but);
		}
	}

void EditSplineMod::StartFilletMode()
	{
	if ( !ip ) return;

	if (ip->GetCommandMode() == filletMode) {
		ip->SetStdCommandMode(CID_OBJMOVE);
		return;
		}
	ip->SetCommandMode(filletMode);
	}

int FilletMouseProc::proc( 
		HWND hwnd, 
		int msg, 
		int point, 
		int flags, 
		IPoint2 m )
	{	
	ViewExp *vpt = ip->GetViewport(hwnd);
	switch ( msg ) {
		case MOUSE_PROPCLICK:
			ip->SetStdCommandMode(CID_OBJMOVE);
			break;

		case MOUSE_POINT:						
			if ( point == 0 ) {							
				es->SetFCLimit();
				es->BeginFilletMove(ip->GetTime());				
				p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				sp0 = m;
			} else {
				float size = es->filletSpin->GetFVal();
				es->EndFilletMove(ip->GetTime(), (size == 0.0f) ? FALSE : TRUE);
				ip->RedrawViews(ip->GetTime(),REDRAW_END);
				}
			break;

		case MOUSE_MOVE: {
			float size = vpt->SnapLength(vpt->GetCPDisp(p0,Point3(0,0,1),sp0,m));
			es->FilletMove( ip->GetTime(), size);
			es->filletSpin->SetValue(size, FALSE);
			ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
			break;
			}

		case MOUSE_ABORT:
			es->EndFilletMove(ip->GetTime(),FALSE);			
			ip->RedrawViews(ip->GetTime(),REDRAW_END);
			break;		
		}
	
	if ( vpt ) ip->ReleaseViewport(vpt);
	return TRUE;
	}

void EditSplineMod::BeginFilletMove(TimeValue t)
	{
	EndMoveModes(t);
	theHold.Begin();
	inFillet = TRUE;
	}

void EditSplineMod::EndFilletMove(TimeValue t,BOOL accept)
	{	
	if ( !inFillet ) return;
	if ( !ip ) return;	
	
	if (accept) {
		// Record the changes that happened
		ModContextList mcList;		
		INodeTab nodes;
		ip->GetModContexts(mcList,nodes);
		ClearShapeDataFlag(mcList,ESD_BEENDONE);
		for ( int i = 0; i < mcList.Count(); i++ ) {
			EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
			if ( !shapeData ) continue;
			if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;
			BezierShape *shape = shapeData->TempData(this)->GetShape(t);
			if(!shape) continue;
			shapeData->UpdateChanges(shape);					
			shapeData->TempData(this)->Invalidate(PART_GEOM);
			shapeData->SetFlag(ESD_BEENDONE,TRUE);
			}

		nodes.DisposeTemporary();
		ClearShapeDataFlag(mcList,ESD_BEENDONE);
		theHold.Accept(GetString(IDS_TH_FILLET));
	} else {
		theHold.Cancel();
		}
		
	inFillet = FALSE;
	if ( filletSpin ) {
		filletSpin->SetValue(0,FALSE);
		}
	}

void EditSplineMod::FilletMove( TimeValue t, float amount ) 
	{	
	LimitValue(amount, 0.0f, FCLimit);

	ModContextList mcList;		
	INodeTab nodes;

	if ( !ip ) return;
	theHold.Restore();
	ip->GetModContexts(mcList,nodes);

	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
				
		shapeData->BeginEdit(t);
		int polys = shape->splineCount;
		for(int poly = 0; poly < polys; ++poly) {				
			Spline3D *spline = shape->splines[poly];

			if(shape->vertSel[poly].NumberSet()) {
				if ( theHold.Holding() && !TestAFlag(A_HELD) ) {
					shapeData->PreUpdateChanges(shape);
					theHold.Put(new ShapeRestore(shapeData,this,shape));
					}

				FilletOrChamferSpline(shape, poly, amount, ES_FILLET);
				}
			}
		shapeData->TempData(this)->Invalidate(PART_GEOM);
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}

	// Mark all objects in selection set
	SetAFlag(A_HELD);
	
	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
	}

/*-------------------------------------------------------------------*/

void ESChamferCMode::EnterMode()
	{
	if ( es->hOpsPanel ) {
		ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel,IDC_ES_CHAMFER));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
		}
	}

void ESChamferCMode::ExitMode()
	{
	if ( es->hOpsPanel ) {
		es->EndChamferMove(es->ip->GetTime(),TRUE);
		ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel,IDC_ES_CHAMFER));
		but->SetCheck(FALSE);
		ReleaseICustButton(but);
		}
	}

void EditSplineMod::StartChamferMode()
	{
	if ( !ip ) return;

	if (ip->GetCommandMode() == chamferMode) {
		ip->SetStdCommandMode(CID_OBJMOVE);
		return;
		}
	ip->SetCommandMode(chamferMode);
	}

int ESChamferMouseProc::proc( 
		HWND hwnd, 
		int msg, 
		int point, 
		int flags, 
		IPoint2 m )
	{	
	ViewExp *vpt = ip->GetViewport(hwnd);
	switch ( msg ) {
		case MOUSE_PROPCLICK:
			ip->SetStdCommandMode(CID_OBJMOVE);
			break;

		case MOUSE_POINT:						
			if ( point == 0 ) {							
				es->SetFCLimit();
				es->BeginChamferMove(ip->GetTime());				
				p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				sp0 = m;
			} else {
				float size = es->chamferSpin->GetFVal();
				es->EndChamferMove(ip->GetTime(), (size == 0.0f) ? FALSE : TRUE);
				ip->RedrawViews(ip->GetTime(),REDRAW_END);
				}
			break;

		case MOUSE_MOVE: {
			float size = vpt->SnapLength(vpt->GetCPDisp(p0,Point3(0,0,1),sp0,m));
			es->ChamferMove( ip->GetTime(), size);
			es->chamferSpin->SetValue(size, FALSE);
			ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
			break;
			}

		case MOUSE_ABORT:
			es->EndChamferMove(ip->GetTime(),FALSE);			
			ip->RedrawViews(ip->GetTime(),REDRAW_END);
			break;		
		}
	
	if ( vpt ) ip->ReleaseViewport(vpt);
	return TRUE;
	}

void EditSplineMod::BeginChamferMove(TimeValue t)
	{
	EndMoveModes(t);
	theHold.Begin();
	inChamfer = TRUE;
	}

void EditSplineMod::EndChamferMove(TimeValue t,BOOL accept)
	{	
	if ( !inChamfer ) return;
	if ( !ip ) return;	
	
	if (accept) {
		// Record the changes that happened
		ModContextList mcList;		
		INodeTab nodes;
		ip->GetModContexts(mcList,nodes);
		ClearShapeDataFlag(mcList,ESD_BEENDONE);
		for ( int i = 0; i < mcList.Count(); i++ ) {
			EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
			if ( !shapeData ) continue;
			if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;
			BezierShape *shape = shapeData->TempData(this)->GetShape(t);
			if(!shape) continue;
			shapeData->UpdateChanges(shape);					
			shapeData->TempData(this)->Invalidate(PART_GEOM);
			shapeData->SetFlag(ESD_BEENDONE,TRUE);
			}

		nodes.DisposeTemporary();
		ClearShapeDataFlag(mcList,ESD_BEENDONE);
		theHold.Accept(GetString(IDS_TH_CHAMFER));
	} else {
		theHold.Cancel();
		}
		
	inChamfer = FALSE;
	if ( chamferSpin ) {
		chamferSpin->SetValue(0,FALSE);
		}
	}

void EditSplineMod::ChamferMove( TimeValue t, float amount ) 
	{	
	LimitValue(amount, 0.0f, FCLimit);
	
	ModContextList mcList;		
	INodeTab nodes;

	if ( !ip ) return;
	theHold.Restore();
	ip->GetModContexts(mcList,nodes);

	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
				
		shapeData->BeginEdit(t);
		int polys = shape->splineCount;
		for(int poly = 0; poly < polys; ++poly) {				
			Spline3D *spline = shape->splines[poly];

			if(shape->vertSel[poly].NumberSet()) {
				if ( theHold.Holding() && !TestAFlag(A_HELD) ) {
					shapeData->PreUpdateChanges(shape);
					theHold.Put(new ShapeRestore(shapeData,this,shape));
					}

				FilletOrChamferSpline(shape, poly, amount, ES_CHAMFER);
				}
			}
		shapeData->TempData(this)->Invalidate(PART_GEOM);
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}

	// Mark all objects in selection set
	SetAFlag(A_HELD);
	
	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
	}

/*-------------------------------------------------------------------*/

void SegBreakCMode::EnterMode()
	{
	if ( es->hOpsPanel ) {
		ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel,IDC_ES_BREAK));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
		}
	}

void SegBreakCMode::ExitMode()
	{
	if ( es->hOpsPanel ) {
		ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel,IDC_ES_BREAK));
		but->SetCheck(FALSE);
		ReleaseICustButton(but);
		}
	}

void EditSplineMod::StartSegBreakMode()
	{
	if ( !ip ) return;

	ip->SetCommandMode(segBreakMode);
	}

/*-------------------------------------------------------------------*/

void CreateLineCMode::EnterMode()
	{
	if ( es->hOpsPanel ) {
		ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel,IDC_ES_CREATELINE));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
		}
	}

void CreateLineCMode::ExitMode()
	{
	if ( es->hOpsPanel ) {
		ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel,IDC_ES_CREATELINE));
		but->SetCheck(FALSE);
		ReleaseICustButton(but);
		}
	}

void EditSplineMod::StartCreateLineMode()
	{
	if ( !ip ) return;

	if (ip->GetCommandMode() == createLineMode) {
		ip->SetStdCommandMode(CID_OBJMOVE);
		return;
		}
	ip->SetCommandMode(createLineMode);
	}

BOOL EditSplineMod::StartCreateLine(BezierShape **shape) {
	ModContextList mcList;		
	INodeTab nodes;
	if ( !ip ) return FALSE;

	ip->GetModContexts(mcList,nodes);
	if(mcList.Count() != 1) {
		nodes.DisposeTemporary();
		return FALSE;
		}
	createShapeData = (EditSplineData*)mcList[0]->localData;
	if ( !createShapeData ) {
		nodes.DisposeTemporary();
		return FALSE;
		}
	// If the mesh isn't yet cache, this will cause it to get cached.
	createShape = createShapeData->TempData(this)->GetShape(ip->GetTime());
	if ( !createShape ) {
		nodes.DisposeTemporary();
		return FALSE;
		}

	theHold.Begin();
	if ( theHold.Holding() ) {
		theHold.Put(new ShapeRestore(createShapeData,this,createShape));
		}

	createNode = nodes[0]->GetActualINode();	
	createTM = nodes[0]->GetObjectTM(ip->GetTime());
	*shape = createShape;
	nodes.DisposeTemporary();
	createShapeData->PreUpdateChanges(createShape);
	return TRUE;
	}

void EditSplineMod::EndCreateLine(BOOL altered) {
	if ( !ip ) return;
	if(altered) {
		createShapeData->BeginEdit(ip->GetTime());
		createShapeData->UpdateChanges(createShape);
		theHold.Accept(GetString(IDS_TH_CREATELINE));
		}
	else
		theHold.End();	// Forget it!
	}

/*-------------------------------------------------------------------*/
// CAL-02/26/03: Add Cross Section mode. (FID #827)

void CrossSectionCMode::EnterMode()
	{
	if ( es->hOpsPanel ) {
		ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel,IDC_ES_CROSS_SECTION));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
		}
	}

void CrossSectionCMode::ExitMode()
	{
	if ( es->hOpsPanel ) {
		ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel,IDC_ES_CROSS_SECTION));
		but->SetCheck(FALSE);
		ReleaseICustButton(but);
		}
	}

void EditSplineMod::StartCrossSectionMode()
	{
	if ( !ip ) return;

	if (ip->GetCommandMode() == crossSectionMode) {
		ip->SetStdCommandMode(CID_OBJMOVE);
		return;
		}
	ip->SetCommandMode(crossSectionMode);
	}

BOOL EditSplineMod::StartCrossSection(EditSplineData **shapeData)
{
	if ( !ip ) return FALSE;
	ModContextList mcList;	
	INodeTab nodes;

	ip->GetModContexts(mcList,nodes);
	if(mcList.Count() != 1) {
		nodes.DisposeTemporary();
		return FALSE;
		}
	createShapeData = (EditSplineData*)mcList[0]->localData;
	if ( !createShapeData ) {
		nodes.DisposeTemporary();
		return FALSE;
		}
	// If the mesh isn't yet cache, this will cause it to get cached.
	createShape = createShapeData->TempData(this)->GetShape(ip->GetTime());
	if ( !createShape ) {
		nodes.DisposeTemporary();
		return FALSE;
		}

	theHold.Begin();
	if ( theHold.Holding() && !TestAFlag(A_HELD) ) {
		theHold.Put(new ShapeRestore(createShapeData,this,createShape));
		SetAFlag(A_HELD);
	}

	*shapeData = createShapeData;
	createShapeData->PreUpdateChanges(createShape);
	nodes.DisposeTemporary();
	return TRUE;
}

void EditSplineMod::EndCrossSection(BOOL acceptUndo)
{
	if ( !ip ) return;
	if (acceptUndo) {
		createShapeData->BeginEdit(ip->GetTime());
		createShapeData->UpdateChanges(createShape);
		theHold.Accept(GetString(IDS_TH_CROSS_SECTION));
	} else {
		theHold.Cancel();
	}
}

/*-------------------------------------------------------------------*/

void SegRefineCMode::EnterMode()
	{
	if ( es->hOpsPanel ) {
		ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel, IDC_ES_REFINE));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);

//2-1-99 watje
		EnableWindow (GetDlgItem (es->hOpsPanel, IDC_ES_RCONNECT),FALSE);
		EnableWindow (GetDlgItem (es->hOpsPanel, IDC_ES_RCLINEAR), FALSE);
		EnableWindow (GetDlgItem (es->hOpsPanel, IDC_ES_RCCLOSED), FALSE);
		EnableWindow (GetDlgItem (es->hOpsPanel, IDC_ES_BINDFIRST), FALSE);
		EnableWindow (GetDlgItem (es->hOpsPanel, IDC_ES_BINDLAST), FALSE);

		}
	}

void SegRefineCMode::ExitMode()
	{
	if ( es->hOpsPanel ) {
		ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel, IDC_ES_REFINE));
		but->SetCheck(FALSE);
		ReleaseICustButton(but);

//2-1-99 watje
		BOOL vType = (es->GetSubobjectLevel() == ES_VERTEX) ? TRUE : FALSE;
		BOOL sType = (es->GetSubobjectLevel() == ES_SEGMENT) ? TRUE : FALSE;

		if ( vType||sType)
			EnableWindow (GetDlgItem (es->hOpsPanel, IDC_ES_RCONNECT),TRUE);
		else EnableWindow (GetDlgItem (es->hOpsPanel, IDC_ES_RCONNECT),FALSE); 

		if ( (es->rConnect) && (vType||sType))
			{
			EnableWindow (GetDlgItem (es->hOpsPanel, IDC_ES_RCLINEAR), TRUE);
			EnableWindow (GetDlgItem (es->hOpsPanel, IDC_ES_RCCLOSED), TRUE);
			EnableWindow (GetDlgItem (es->hOpsPanel, IDC_ES_BINDFIRST), TRUE);
			EnableWindow (GetDlgItem (es->hOpsPanel, IDC_ES_BINDLAST), TRUE);
			}
		else
			{
			EnableWindow (GetDlgItem (es->hOpsPanel, IDC_ES_RCLINEAR), FALSE);
			EnableWindow (GetDlgItem (es->hOpsPanel, IDC_ES_RCCLOSED), FALSE);
			EnableWindow (GetDlgItem (es->hOpsPanel, IDC_ES_BINDFIRST), FALSE);
			EnableWindow (GetDlgItem (es->hOpsPanel, IDC_ES_BINDLAST), FALSE);
			}


		}
	}

void EditSplineMod::StartSegRefineMode()
	{
	if ( !ip ) return;

	if (ip->GetCommandMode() == segRefineMode) {
		ip->SetStdCommandMode(CID_OBJMOVE);
		return;
		}

	segRefineMode->SetType(GetSubobjectLevel()==ES_VERTEX ? REFINE_VERT : REFINE_SEG);
	ip->SetCommandMode(segRefineMode);
	}

/*-------------------------------------------------------------------*/

void CrossInsertCMode::EnterMode()
	{
	if ( es->hOpsPanel ) {
		ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel, IDC_ES_CROSS_INSERT));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
		}
	}

void CrossInsertCMode::ExitMode()
	{
	if ( es->hOpsPanel ) {
		ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel, IDC_ES_CROSS_INSERT));
		but->SetCheck(FALSE);
		ReleaseICustButton(but);
		}
	}

void EditSplineMod::StartCrossInsertMode()
	{
	if ( !ip ) return;

	if (ip->GetCommandMode() == crossInsertMode) {
		ip->SetStdCommandMode(CID_OBJMOVE);
		return;
		}
	ip->SetCommandMode(crossInsertMode);
	}

/*-------------------------------------------------------------------*/

void VertConnectCMode::EnterMode()
	{
	if ( es->hOpsPanel ) {
		ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel,IDC_ES_CONNECT));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
		}
	}

void VertConnectCMode::ExitMode()
	{
	if ( es->hOpsPanel ) {
//		es->EndOutlineMove(es->ip->GetTime(),TRUE);
		ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel,IDC_ES_CONNECT));
		but->SetCheck(FALSE);
		ReleaseICustButton(but);
		}
	}

void EditSplineMod::StartVertConnectMode()
	{
	if ( !ip ) return;

	if (ip->GetCommandMode() == vertConnectMode) {
		ip->SetStdCommandMode(CID_OBJMOVE);
		return;
		}
	ip->SetCommandMode(vertConnectMode);
	}

/*-------------------------------------------------------------------*/

void VertInsertCMode::EnterMode()
	{
	if ( es->hOpsPanel ) {
		if(control >= 0) {
			ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel,control));
			but->SetCheck(TRUE);
			ReleaseICustButton(but);
			}
		}
	}

void VertInsertCMode::ExitMode()
	{
	if ( es->hOpsPanel ) {
		if(control >= 0) {
			ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel,control));
			but->SetCheck(FALSE);
			ReleaseICustButton(but);
			}
		}
	}

void EditSplineMod::StartVertInsertMode(int controlID)
	{
	if ( !ip ) return;

	if (ip->GetCommandMode() == vertInsertMode) {
		ip->SetStdCommandMode(CID_OBJMOVE);
		return;
		}
	vertInsertMode->SetControl(controlID);
	ip->SetCommandMode(vertInsertMode);
	}

/*-------------------------------------------------------------------*/

void BooleanCMode::EnterMode()
	{
	if ( es->hOpsPanel ) {
		ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel,IDC_ES_BOOLEAN));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
		}
	}

void BooleanCMode::ExitMode()
	{
	if ( es->hOpsPanel ) {
//		es->EndOutlineMove(es->ip->GetTime(),TRUE);
		ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel,IDC_ES_BOOLEAN));
		but->SetCheck(FALSE);
		ReleaseICustButton(but);
		}
	}

void EditSplineMod::StartBooleanMode()
	{
	if ( !ip ) return;

	ip->SetCommandMode(booleanMode);
	}

// See if a spline self-intersects
static BOOL SplineSelfIntersects(Spline3D *spline) {
	// Create a 2D template of the spline
	ESMTemplate t(spline);
	int points = t.Points();
	int last = points - 1;
	int last2 = points - 2;
	Point2 where;
	for(int i = 0; i < last2; ++i) {
		Point2 i1 = t.pts[i];
		Point2 i2 = t.pts[i+1];
		for(int j = i + 2; j < last; ++j) {
			if(i==0 && j == last2) continue;	// No comparison with last seg & first!
			if(IntSeg(i1,i2,t.pts[j],t.pts[j+1],where) == 1) {
//DebugPrint("Self-int %d/%d: %.4f %.4f - %.4f %.4f / %.4f %.4f - %.4f %.4f @ %.4f %.4f\n",i,j,i1.x,i1.y,i2.x,i2.y,
//	t.pts[j].x,t.pts[j].y,t.pts[j+1].x,t.pts[j+1].y,where.x,where.y);
				return TRUE;
				}
			}
		}
	return FALSE;
	}

BOOL ValidBooleanPolygon(IObjParam *ip, Spline3D *spline) {
	if(!spline->Closed()) {
		ip->DisplayTempPrompt(GetString(IDS_TH_SELECTCLOSEDSPLINE),PROMPT_TIME);
		return FALSE;
		}
	if(SplineSelfIntersects(spline)) {
		ip->DisplayTempPrompt(GetString(IDS_TH_SPLINESELFINTERSECTS),PROMPT_TIME);
		return FALSE;
		}
	return TRUE;	// It's OK!
	}

// This function checks to see if there is only one polygon selected, and if so, starts up
// the second phase of the boolean operation

BOOL EditSplineMod::BooleanStartUp() {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();

	if ( !ip ) return FALSE;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	int selected = 0;
	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
			
		int polys = shape->splineCount;
		for(int poly = 0; poly < polys; ++poly) {
			if(shape->polySel[poly]) {
				if(selected) {
					ip->DisplayTempPrompt(GetString(IDS_TH_MORETHANONESPLINESEL),PROMPT_TIME);
					return FALSE;
					}
				selected = 1;
				boolShape = shape;
				boolPoly1 = poly;
				}
			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}

	nodes.DisposeTemporary();

	// If no polys selected -- No can do!
	if(!selected) {
		ip->DisplayTempPrompt(GetString(IDS_TH_SELECTONESPLINE),PROMPT_TIME);
		return FALSE;
		}

	// Got one poly selected, make sure it's valid!
	if(!ValidBooleanPolygon(ip, boolShape->splines[boolPoly1]))
		return FALSE;

	// It's kosher, start up the boolean mode!
	StartBooleanMode();

	return TRUE;
	}

/*-------------------------------------------------------------------*/

BOOL PickSplineAttach::Filter(INode *node)
	{
	ModContextList mcList;		
	INodeTab nodes;
	if (node) {
		// Make sure the node does not depend on us
		node->BeginDependencyTest();
		es->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
		if (node->EndDependencyTest()) return FALSE;

		ObjectState os = node->GetObjectRef()->Eval(es->ip->GetTime());
		GeomObject *object = (GeomObject *)os.obj;
		// Make sure it isn't one of the nodes we're editing, for heaven's sake!
		es->ip->GetModContexts(mcList,nodes);
		int numNodes = nodes.Count();
		for(int i = 0; i < numNodes; ++i) {
			if(nodes[i] == node) {
				nodes.DisposeTemporary();
				return FALSE;
				}
			}
		nodes.DisposeTemporary();
		if(object->CanConvertToType(splineShapeClassID))
			return TRUE;
		}

	return FALSE;
	}

BOOL PickSplineAttach::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
	{	
	INode *node = ip->PickNode(hWnd,m,this);
	ModContextList mcList;		
	INodeTab nodes;
	
	if (node) {
		ObjectState os = node->GetObjectRef()->Eval(ip->GetTime());
		GeomObject *object = (GeomObject *)os.obj;
		// Make sure it isn't one of the nodes we're editing, for heaven's sake!
		es->ip->GetModContexts(mcList,nodes);
		int numNodes = nodes.Count();
		for(int i = 0; i < numNodes; ++i) {
			if(nodes[i] == node) {
				nodes.DisposeTemporary();
				return FALSE;
				}
			}
		nodes.DisposeTemporary();
		if(object->CanConvertToType(splineShapeClassID))
			return TRUE;
		}

	return FALSE;
	}

BOOL PickSplineAttach::Pick(IObjParam *ip,ViewExp *vpt)
	{
	if(!ip)
		return FALSE;
	INode *node = vpt->GetClosestHit();
	assert(node);
	GeomObject *object = (GeomObject *)node->GetObjectRef()->Eval(ip->GetTime()).obj;
	if(object->CanConvertToType(splineShapeClassID)) {
		ModContextList mcList;
		INodeTab nodes;
		ip->GetModContexts(mcList,nodes);
		BOOL res = TRUE;
		if (nodes[0]->GetMtl() && node->GetMtl() && (nodes[0]->GetMtl()!=node->GetMtl()))
			res = DoAttachMatOptionDialog(ip, es);
		if(res)	{
			bool canUndo = TRUE;
			es->DoAttach(node, canUndo);
			if (!canUndo)
				GetSystemSetting (SYSSET_CLEAR_UNDO);
			}
		nodes.DisposeTemporary();
		}
	return FALSE;
	}


void PickSplineAttach::EnterMode(IObjParam *ip)
	{
	if ( es->hOpsPanel ) {
		ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel,IDC_ES_ATTACH));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
		}
	}

void PickSplineAttach::ExitMode(IObjParam *ip)
	{
	if ( es->hOpsPanel ) {
		ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel,IDC_ES_ATTACH));
		but->SetCheck(FALSE);
		ReleaseICustButton(but);
		}
	}

HCURSOR PickSplineAttach::GetHitCursor(IObjParam *ip) {
	return LoadCursor(hInstance, MAKEINTRESOURCE(IDC_ATTACHCUR));
	}

static BOOL doingMultiAttach = FALSE;

int EditSplineMod::DoAttach(INode *node, bool & canUndo) {
	if ( !ip ) return 0;

	// Get the shape that's being attached
	BezierShape attShape;
	GeomObject *object = (GeomObject *)node->GetObjectRef()->Eval(ip->GetTime()).obj;
	if(!object->CanConvertToType(splineShapeClassID))
		return 0;
	SplineShape *attSplShape = (SplineShape *)object->ConvertToType(ip->GetTime(),splineShapeClassID);
	if(!attSplShape)
		return 0;
	attShape = attSplShape->shape;
	// Discard the copy it made, if it isn't the same as the object itself
	if(attSplShape != (SplineShape *)object)
		delete attSplShape;
	
	ModContextList mcList;	
	INodeTab nodes;

	ip->GetModContexts(mcList,nodes);

	EditSplineData *shapeData = (EditSplineData*)mcList[0]->localData;
	if ( !shapeData ) {
		nodes.DisposeTemporary();
		return 0;
		}

	// If the mesh isn't yet cached, this will cause it to get cached.
	BezierShape *shape = shapeData->TempData(this)->GetShape(ip->GetTime());
	if(!shape) {
		nodes.DisposeTemporary();
		return 0;
		}
 	shapeData->BeginEdit(ip->GetTime());

	ShapeRestore *theRestore = new ShapeRestore(shapeData,this,shape);

	// Transform the shape for attachment:
	// If reorienting, just translate to align pivots
	// Otherwise, transform to match our transform
	Matrix3 attMat(1);
	if(attachReorient) {
		Matrix3 thisTM = nodes[0]->GetNodeTM(ip->GetTime());
		Matrix3 thisOTMBWSM = nodes[0]->GetObjTMBeforeWSM(ip->GetTime());
		Matrix3 thisPivTM = thisTM * Inverse(thisOTMBWSM);
		Matrix3 otherTM = node->GetNodeTM(ip->GetTime());
		Matrix3 otherOTMBWSM = node->GetObjTMBeforeWSM(ip->GetTime());
		Matrix3 otherPivTM = otherTM * Inverse(otherOTMBWSM);
		Point3 otherObjOffset = node->GetObjOffsetPos();
		attMat = Inverse(otherPivTM) * thisPivTM;
		}
	else {
		attMat = node->GetObjectTM(ip->GetTime()) *
			Inverse(nodes[0]->GetObjectTM(ip->GetTime()));
		}
	attShape.Transform(attMat);

	theHold.Begin();

	// Combine the materials of the two nodes.
	int mat2Offset=0;
	Mtl *m1 = nodes[0]->GetMtl();
	Mtl *m2 = node->GetMtl();
	bool condenseMe = FALSE;
	if (m1 && m2 && (m1 != m2)) {
		if (attachMat==ATTACHMAT_IDTOMAT) {
			int ct=1;
			if (m1->IsMultiMtl())
				ct = m1->NumSubMtls();
			for(int poly = 0; poly < shape->SplineCount(); ++poly) {
				Spline3D *spline = shape->splines[poly];
				for(int i = 0; i < spline->Segments(); ++i) {
					int mtid = spline->GetMatID(i);
					if(mtid >= ct)
						spline->SetMatID(i, mtid % ct);
					}
				}
			FitShapeIDsToMaterial (attShape, m2);
			if (condenseMat) condenseMe = TRUE;
			}
		// the theHold calls here were a vain attempt to make this all undoable.
		// This should be revisited in the future so we don't have to use the SYSSET_CLEAR_UNDO.
		theHold.Suspend ();
		if (attachMat==ATTACHMAT_MATTOID) {
			m1 = FitMaterialToShapeIDs (*shape, m1);
			m2 = FitMaterialToShapeIDs (attShape, m2);
			}

		Mtl *multi = CombineMaterials (m1, m2, mat2Offset);
		if (attachMat == ATTACHMAT_NEITHER) mat2Offset = 0;
		theHold.Resume ();
		// We can't be in face subobject mode, else we screw up the materials:
		DWORD oldSL = shape->selLevel;
		shape->selLevel = SHAPE_OBJECT;
		nodes[0]->SetMtl(multi);
		shape->selLevel = oldSL;
		m1 = multi;
		// canUndo = FALSE;	// DS: 11/15/00 Undo should work now.
		}
	if (!m1 && m2) {
		// We can't be in face subobject mode, else we screw up the materials:
		DWORD oldSL = shape->selLevel;
		shape->selLevel = SHAPE_OBJECT;
		nodes[0]->SetMtl(m2);
		shape->selLevel = oldSL;
		m1 = m2;
		}

	shapeData->PreUpdateChanges(shape);

	// Start a restore object...
	if ( theHold.Holding() )
		theHold.Put(theRestore);

	// Do the attach
	::DoAttach(shape, &attShape, mat2Offset);
	shapeData->UpdateChanges(shape,0);
	shapeData->TempData(this)->Invalidate(PART_TOPO|PART_GEOM);

	// Get rid of the original node
	ip->DeleteNode(node);

	theHold.Accept(GetString(IDS_TH_ATTACH));

	if (m1 && condenseMe) {
		// Following clears undo stack.
		shape = shapeData->TempData(this)->GetShape(ip->GetTime());
		m1 = CondenseMatAssignments (*shape, m1);
		}
	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	if(!doingMultiAttach) {
		SelectionChanged();
		NotifyDependents(FOREVER, PART_TOPO|PART_GEOM, REFMSG_CHANGE);
		ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
		}
	return 1;
	}

void EditSplineMod::MultiAttachObject(INodeTab &nodeTab)
	{
	bool canUndo = TRUE;
	if (nodeTab.Count() > 1)
		theHold.SuperBegin ();
	doingMultiAttach = TRUE;
	for (int i=0; i<nodeTab.Count(); i++)
		DoAttach (nodeTab[i], canUndo);
	doingMultiAttach = FALSE;
	if (nodeTab.Count() > 1)
		theHold.SuperAccept (GetString (IDS_EM_ATTACH_LIST));
	if (!canUndo)
		GetSystemSetting (SYSSET_CLEAR_UNDO);
	SelectionChanged();
	NotifyDependents(FOREVER, PART_TOPO|PART_GEOM, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
	}

/*-------------------------------------------------------------------*/

EditSplineMod::EditSplineMod()
	{
	showVertNumbers = FALSE;
	SVNSelectedOnly = FALSE;
	selLevel = ES_OBJECT;
	insertShape = NULL;	
 	segUIValid = FALSE;
//2-1-99 watje
	useAreaSelect = FALSE;
	areaSelect = 0.1f;
	esFlags = 0;
	// CAL-05/23/03: Add Connecting Splines when Shift-Move Copy. (FID #827)
	connectCopy = FALSE;
	connectCopyThreshold = 0.1f;
	// CAL-02/24/03: Add Cross Section mode. (FID #827)
	newKnotType = DEFAULT_KNOT_TYPE;
	// CAL-03/03/03: copy/paste tangent. (FID #827)
	copyTanLength = FALSE;
	tangentCopied = FALSE;

	mFalloff =  20.0;
	mBubble = 0.0;
	mPinch = 0.0;
	mUseSoftSelections = 0;
	mUseEdgeDists = 0;
	mEdgeDist = 1;

	}

EditSplineMod::~EditSplineMod()
	{
	}

Interval EditSplineMod::LocalValidity(TimeValue t)
	{
	// Force a cache if being edited.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;  			   
	return FOREVER;
	}

RefTargetHandle EditSplineMod::Clone(RemapDir& remap) {
	EditSplineMod* newmod = new EditSplineMod();	
	newmod->showVertNumbers = showVertNumbers;
	newmod->SVNSelectedOnly = SVNSelectedOnly;
	newmod->selLevel = selLevel;
	BaseClone(this, newmod, remap);
	return(newmod);
	}

void EditSplineMod::ClearShapeDataFlag(ModContextList& mcList,DWORD f)
	{
	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *meshData = (EditSplineData*)mcList[i]->localData;
		if ( !meshData ) continue;
		meshData->SetFlag(f,FALSE);
		}
	}

void EditSplineMod::XFormHandles( 
		XFormProc *xproc, 
		TimeValue t, 
		Matrix3& partm, 
		Matrix3& tmAxis,
		int masterObject)
	{	
	ModContextList mcList;		
	INodeTab nodes;
	Matrix3 mat,imat,theMatrix;
	Interval valid;
	int numAxis;
	int masterKnot;
	Point3 oldpt,newpt,oldin,oldout,rel;
	BOOL shiftPressed = FALSE;
	static BOOL wasBroken;
	Point3 theKnot;
	Point3 oldVector;
	Point3 newVector;
	BOOL isInVec;
	float oldLen;
	float newLen;
	float lengthRatio;

	if(lockType != IDC_LOCKALL)
		shiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) ? TRUE : FALSE;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	numAxis = ip->GetNumAxis();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	// Loop thru the objects, doing the master object first
	// If handles aren't locked, we only need to do the master object!
	int objects = lockedHandles ? mcList.Count() : 1;
	for ( int i = 0, object = masterObject; i < objects; i++, object = (object + 1) % objects ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[object]->localData;
		if ( !shapeData ) {
			nodes.DisposeTemporary();
			return;
			}
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;
		
		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);

		// Create a change record for this object and store a pointer to its delta info in this EditSplineData
		if(!TestAFlag(A_HELD)) {
			shapeData->vdelta.SetSize(*shape,FALSE);
			if ( theHold.Holding() ) {
				theHold.Put(new ShapeRestore(shapeData,this,shape));
				}
			shapeData->vdelta.Zero();		// Reset all deltas
			shapeData->ClearHandleFlag();
			wasBroken = FALSE;
			shapeData->PreUpdateChanges(shape);
			}
		else {
			if(wasBroken && !shiftPressed)
				wasBroken = FALSE;
			if(shapeData->DoingHandles())
				shapeData->ApplyHandlesAndZero(*shape);		// Reapply the slave handle deltas
			else
				shapeData->vdelta.Zero();
			}

		// If the master shape, move the handle!
		if(object == masterObject) {
			masterKnot = shape->bezVecVert / 3;
			int poly = shape->bezVecPoly;
			int vert = shape->bezVecVert;
			shapeData->SetHandleFlag(poly, vert);
			Spline3D *spline = shape->splines[poly];
			int primaryKnot = vert / 3;
			isInVec = ((vert % 3) == 0) ? TRUE : FALSE;
			int otherVert = isInVec ? vert + 2 : vert - 2;
			theKnot = spline->GetKnotPoint(primaryKnot);
			Point3Tab &pDeltas = shapeData->vdelta.dtab.ptab[poly];

			int vbase = shape->GetVertIndex(poly, primaryKnot * 3 + 1);
			tmAxis = ip->GetTransformAxis(nodes[object],vbase);
			mat    = nodes[object]->GetObjectTM(t,&valid) * Inverse(tmAxis);
			imat   = Inverse(mat);
			xproc->SetMat(mat);
					
			// XForm the cache vertices
			oldpt = spline->GetVert(vert);
			newpt = xproc->proc(oldpt,mat,imat);
			spline->SetVert(vert,newpt);

			// Move the delta's vertices.
			shapeData->vdelta.SetPoint(poly,vert,newpt - oldpt);

			// If locked handles, turn the movement into a transformation matrix
			if(lockedHandles) {
				if(!wasBroken && shiftPressed)
					wasBroken = TRUE;
				oldVector = oldpt - theKnot;
				newVector = newpt - theKnot;
				oldLen = Length(oldVector);
				newLen = Length(newVector);
				int allNew = (oldLen == 0.0f) ? 1 : 0;		// All new vector?
				lengthRatio = 1.0f;
				if(!allNew)
					lengthRatio = newLen / oldLen;
				Point3 oldNorm = Normalize(oldVector);
				Point3 newNorm = Normalize(newVector);
				theMatrix.IdentityMatrix();
				if(oldNorm != newNorm) {
					// Get a matrix that will transform the old point to the new one
					// Cross product gives us the normal of the rotational axis
					Point3 axis = Normalize(CrossProd(oldNorm, newNorm));
					// Dot product gives us the angle
					float dot = DotProd(oldNorm, newNorm);
					if(dot >= -1.0f && dot < 1.0f) {
						float angle = (float)-acos(dot);

						// Now let's build a matrix that'll do this for us!
						Quat quat = QFromAngAxis(angle, axis);
						quat.MakeMatrix(theMatrix);

						// If need to break the vector, 
						if(shiftPressed && spline->GetKnotType(primaryKnot) == KTYPE_BEZIER) {
							spline->SetKnotType(primaryKnot,KTYPE_BEZIER_CORNER);
							shapeData->vdelta.SetKType(poly,primaryKnot,KTYPE_BEZIER ^ KTYPE_BEZIER_CORNER);
							}
						}
					}
				}
			else {
				// If unlocked and the bezier is non-corner, do its partner on the other side of the knot!
				if(spline->GetKnotType(primaryKnot) == KTYPE_BEZIER) {
					if(shiftPressed) {
						wasBroken = TRUE;
						spline->SetKnotType(primaryKnot,KTYPE_BEZIER_CORNER);
						// Need to record this for undo!
						shapeData->vdelta.SetKType(poly,primaryKnot,KTYPE_BEZIER ^ KTYPE_BEZIER_CORNER);
						}
					// If a bezier smooth knot, do the opposite side!
					if(spline->GetKnotType(primaryKnot) == KTYPE_BEZIER) {
						Point3 oldpt2 = spline->GetVert(otherVert) - pDeltas[otherVert];
						float oldLen2 = Length(theKnot - oldpt2);
						float oldLen1 = Length(theKnot - oldpt);
						if(oldLen1!=0.0f && oldLen2!=0.0f) {
							float ratio = oldLen2 / oldLen1;
							Point3 newpt2 = theKnot - (newpt - theKnot) * ratio;
							// Alter the cache
							spline->SetVert(otherVert,newpt2);
							// Move the delta's vertices.
							shapeData->vdelta.SetPoint(poly,otherVert,newpt2-oldpt2);
							}
						}
					}
				}

			// Really only need to do this if neighbor knots are non-bezier
			spline->ComputeBezPoints();
			shape->InvalidateGeomCache();
			}

		// If doing locked handles, process all of the handles of selected verts!
		if(lockedHandles) {
			int count = 0;
			if(object!=masterObject)
				shapeData->SetHandleFlag(-1, -1);
			for(int poly = 0; poly < shape->splineCount; ++poly) {
				shapeData->vdelta.ClearUsed(poly);
				// Selected vertices - either directly or indirectly through selected faces or edges.
				BitArray sel = shape->VertexTempSel(poly);
				Spline3D *spline = shape->splines[poly];
				int knots = spline->KnotCount();
				Point3Tab &pDeltas = shapeData->vdelta.dtab.ptab[poly];
				for ( int k = 0; k < knots; k++ ) {
					int kvert = k*3+1;
					if(object == masterObject && poly == shape->bezVecPoly && kvert == shape->bezVecVert) {
						shapeData->vdelta.SetUsed(poly);
						continue;
						}
					if ( spline->IsBezierPt(k) && sel[kvert]) {
						shapeData->vdelta.SetUsed(poly);
						theKnot = spline->GetKnotPoint(k);
						int knotType = spline->GetKnotType(k);
						
						if(shiftPressed && knotType == KTYPE_BEZIER) {
							spline->SetKnotType(k,KTYPE_BEZIER_CORNER);
							shapeData->vdelta.SetKType(poly,k,KTYPE_BEZIER ^ KTYPE_BEZIER_CORNER);
							}

						if(isInVec || lockType == IDC_LOCKALL || (!shiftPressed && knotType == KTYPE_BEZIER)) {
							// In vector
							int vert = kvert - 1;
							// XForm the cache vertices
							oldpt = spline->GetVert(vert) - pDeltas[vert];
							oldVector = oldpt - theKnot;
							oldLen = Length(oldVector);
							// If the old vector existed, transform it!
							if(oldLen != 0.0f) {
								Point3 newpt = theKnot + ((oldVector * lengthRatio) * theMatrix);
								// Alter the cache
								spline->SetVert(vert,newpt);
								// Move the delta's vertices.
								shapeData->vdelta.SetPoint(poly,vert,newpt-oldpt);
								}
							}

						if(!isInVec || lockType == IDC_LOCKALL || (!shiftPressed && knotType == KTYPE_BEZIER)) {
							// Out vector
							int vert = kvert + 1;
							// XForm the cache vertices
							oldpt = spline->GetVert(vert) - pDeltas[vert];
							oldVector = oldpt - theKnot;
							oldLen = Length(oldVector);
							// If the old vector existed, transform it!
							if(oldLen != 0.0f) {
								Point3 newpt = theKnot + ((oldVector * lengthRatio) * theMatrix);
								// Alter the cache
								spline->SetVert(vert,newpt);
								// Move the delta's vertices.
								shapeData->vdelta.SetPoint(poly,vert,newpt-oldpt);
								}
							}
						}
					}
				}
			}

		shapeData->UpdateChanges(shape);
		shapeData->TempData(this)->Invalidate(PART_GEOM);
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	
	// Mark all objects in selection set
	SetAFlag(A_HELD);
	
	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
	}

// --------------------------------------------------------------------------------------

class XFormVertsRestore : public RestoreObj {
public:
	TimeValue t;
	BOOL gotRedo;
	BezierShape undo;
	BezierShape redo;
	EditSplineData *esd;
	EditSplineMod *mod;
	
	XFormVertsRestore(EditSplineData *d, EditSplineMod *m, BezierShape *ss) {
		undo.CopyShapeDataFrom(*ss);
		gotRedo = FALSE;
		esd = d;
		mod = m;
		t = mod->ip->GetTime();
		}

	void Restore(int isUndo) {
		if ( esd->tempData && esd->TempData(mod)->ShapeCached(t) ) {
			BezierShape *shape = esd->TempData(mod)->GetShape(t);
			if(shape) {
				if (isUndo) {
					if(!gotRedo) {
						gotRedo = TRUE;
						redo.CopyShapeDataFrom(*shape);
						}
					}
				shape->CopyShapeDataFrom(undo);
				shape->InvalidateGeomCache();
				if (!mod->ip || !mod->ip->AxisTripodLocked()) {	// conditional to prevent updates during drag-moves.
					if (mod->mUseSoftSelections) {
						shape->UpdateVertexDists();
						shape->UpdateEdgeDists();
						shape->UpdateVertexWeights();	
						}
					}
				}
			esd->TempData(mod)->Invalidate(PART_GEOM | PART_TOPO | PART_SELECT);
			}
		else
		if ( esd->tempData ) {
			esd->TempData(mod)->Invalidate(PART_GEOM | PART_TOPO | PART_SELECT, FALSE);
			}
		mod->NotifyDependents(FOREVER, PART_GEOM | PART_TOPO | PART_SELECT, REFMSG_CHANGE);
		}

	void Redo() {
		if ( esd->tempData && esd->TempData(mod)->ShapeCached(t) ) {
			BezierShape *shape = esd->TempData(mod)->GetShape(t);
			if(shape) {
				shape->CopyShapeDataFrom(redo);
				shape->InvalidateGeomCache();
				esd->TempData(mod)->Invalidate(PART_GEOM | PART_TOPO | PART_SELECT);
				if (!mod->ip || !mod->ip->AxisTripodLocked()) {	// conditional to prevent updates during drag-moves.
					if (mod->mUseSoftSelections) {
						shape->UpdateVertexDists();
						shape->UpdateEdgeDists();
						shape->UpdateVertexWeights();	
						}
					}
				}
			}
		else
		if ( esd->tempData ) {
			esd->TempData(mod)->Invalidate(PART_GEOM | PART_TOPO | PART_SELECT, FALSE);
			}
		mod->NotifyDependents(FOREVER, PART_GEOM | PART_TOPO | PART_SELECT, REFMSG_CHANGE);
		}

	int Size() { return 1; }
	void EndHold() { mod->ClearAFlag(A_HELD); }
	TSTR Description() { return TSTR(_T("Generic shape restore")); }
};

// --------------------------------------------------------------------------------------

void EditSplineMod::XFormVerts( 
		XFormProc *xproc, 
		TimeValue t, 
		Matrix3& partm, 
		Matrix3& tmAxis  ) 
	{	
	ModContextList mcList;		
	INodeTab nodes;
	Matrix3 mat,imat;	
	Interval valid;
	int numAxis;
	Point3 oldpt,newpt,oldin,oldout,rel,delta;
	int shiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) ? 1 : 0;
	static BOOL wasBroken;
	static BOOL handleEdit = FALSE;
	static int handleObject;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	numAxis = ip->GetNumAxis();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	if(!TestAFlag(A_HELD)) {
		handleEdit = FALSE;

		// Check all shapes to see if they are altering a bezier vector handle...
		if(selLevel == ES_VERTEX) {
			for ( int i = 0; i < mcList.Count(); i++ ) {
				EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
				if ( !shapeData ) continue;
				if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;
		
				// If the mesh isn't yet cache, this will cause it to get cached.
				BezierShape *shape = shapeData->TempData(this)->GetShape(t);
				if(!shape) continue;
				if(!ip->SelectionFrozen() && shape->bezVecPoly >= 0) {
					// Editing a bezier handle -- Go do it!
					handleEdit = TRUE;
					handleObject = i;
					goto edit_handles;
					}
	 			shapeData->SetFlag(ESD_BEENDONE,TRUE);
				}
			}
		}
	
	// If editing the handles, cut to the chase!
	if(handleEdit) {
		edit_handles:
		XFormHandles(xproc, t, partm, tmAxis, handleObject);
		nodes.DisposeTemporary();
		return;
		}

	// Not doing handles, just plain ol' verts
	ClearShapeDataFlag(mcList,ESD_BEENDONE);	// Clear these out again
	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;
		
		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);

		// Create a change record for this object and store a pointer to its delta info in this EditSplineData
		if(!TestAFlag(A_HELD)) {
			shapeData->vdelta.SetSize(*shape,FALSE);
			shapeData->vdelta.Zero();		// Reset all deltas
			shapeData->PreUpdateChanges(shape);
			if ( theHold.Holding() ) {
				theHold.Put(new XFormVertsRestore(shapeData, this, shape));
				}
			shapeData->ClearHandleFlag();
			wasBroken = FALSE;
			}
		else {
			if(wasBroken)
				shiftPressed = TRUE;
			if(shapeData->DoingHandles())
				shapeData->ApplyHandlesAndZero(*shape);		// Reapply the slave handle deltas
			else
				shapeData->vdelta.Zero();
			}

		// Compute the transforms
		if (numAxis==NUMAXIS_INDIVIDUAL) {
			switch(selLevel) {
				case ES_VERTEX:
				case ES_SEGMENT: {
					int vbase = 1;
					int baseWeight = 0;
					for(int poly = 0; poly < shape->splineCount; ++poly) {
						shapeData->vdelta.ClearUsed(poly);
						// Selected vertices - either directly or indirectly through selected faces or edges.
						BitArray sel = shape->VertexTempSel(poly);
						Spline3D *spline = shape->splines[poly];
						int knots = spline->KnotCount();
						// CAL-05/23/03: transform additional vertices. (FID #827)
						if (GetFlag(ES_ADDED_SELECT)) sel |= shape->VertexFlagSel(poly, SPLINEKNOT_ADD_SEL);

	//watje 5-20-99
						for ( int k = 0; k < knots; k++, vbase+=3 ) {
							BOOL bound = FALSE;
							if ((k==0) && (!spline->Closed()))
								{
								for (int bct = 0; bct < shape->bindList.Count(); bct++)
									{
									if ((shape->bindList[bct].pointSplineIndex == poly) && (!shape->bindList[bct].isEnd))
										bound = TRUE;
									}
								}
							else if ((k==(knots-1)) && (!spline->Closed()))
								{
								for (int bct = 0; bct < shape->bindList.Count(); bct++)
									{
									if ((shape->bindList[bct].pointSplineIndex == poly) && (shape->bindList[bct].isEnd))
										bound = TRUE;
									}
								}

							int vert = k*3+1;
							float vertexWeight;
							if ( sel[vert] ) 
								vertexWeight = 1.0;
							else 
								vertexWeight = shape->VertexWeight( baseWeight+vert );


							if ((fabs(vertexWeight) > 0.0001) && (!bound)) {
								
								shapeData->vdelta.SetUsed(poly);
								tmAxis = ip->GetTransformAxis(nodes[i],vbase);
								mat    = nodes[i]->GetObjectTM(t,&valid) * Inverse(tmAxis);
								imat   = Inverse(mat);
								xproc->SetMat(mat);

								// XForm the cache vertices
								oldpt = spline->GetVert(vert);
								newpt = xproc->proc(oldpt,mat,imat,vertexWeight);
								spline->SetVert(vert,newpt);
								delta = newpt - oldpt;

								// Move the delta's vertices.
								shapeData->vdelta.Move(poly,vert,delta);

								// If it's a bezier knot, also affect its vectors
								if(spline->IsBezierPt(k)) {
									int in = vert - 1;
									int out = vert + 1;

									// XForm the cache vertices
									oldin = spline->GetVert(in);
									float inWeight = shape->VertexWeight( baseWeight+in );
									if ( vertexWeight > .9999 ) 
										inWeight = 1.0;
									else {
										if ( spline->GetKnotType(k) == KTYPE_BEZIER ) {
											spline->SetKnotType(k,KTYPE_BEZIER_CORNER);
											shapeData->vdelta.SetKType(poly,k,KTYPE_BEZIER ^ KTYPE_BEZIER_CORNER);
										}
									}

									spline->SetVert(in,xproc->proc(oldin,mat,imat,inWeight));
									delta = spline->GetVert(in) - oldin;

									// Move the delta's vertices.
									shapeData->vdelta.Move(poly,in,delta);

									// XForm the cache vertices
									oldout = spline->GetVert(out);
									float outWeight = shape->VertexWeight( baseWeight+out );
									if ( vertexWeight > .9999 ) outWeight = 1.0;
									spline->SetVert(out,xproc->proc(oldout,mat,imat,outWeight));

									// Move the delta's vertices.
									shapeData->vdelta.Move(poly,out,spline->GetVert(out) - oldout);
								}
							}
						}

						if(shapeData->vdelta.IsUsed(poly)) {
							spline->ComputeBezPoints();
							shape->InvalidateGeomCache();
						}

						baseWeight += knots*3;

						}
					}
					break;
				case ES_SPLINE: {
					int baseWeight = 0;
					for(int poly = 0; poly < shape->splineCount; ++poly) {
						shapeData->vdelta.ClearUsed(poly);
						// Selected vertices - either directly or indirectly through selected faces or edges.
						BitArray sel = shape->VertexTempSel(poly);
						Spline3D *spline = shape->splines[poly];
						int knots = spline->KnotCount();
						// CAL-05/23/03: transform additional vertices. (FID #827)
						if (GetFlag(ES_ADDED_SELECT)) sel |= shape->VertexFlagSel(poly, SPLINEKNOT_ADD_SEL);
						for ( int k = 0; k < knots; k++ ) {

							int vert = k*3+1;
							float vertexWeight;
							if ( sel[vert] ) 
								vertexWeight = 1.0;
							else 
								vertexWeight = shape->VertexWeight( baseWeight+vert );


							if (fabs(vertexWeight) > 0.0001) {
								
								shapeData->vdelta.SetUsed(poly);
								tmAxis = ip->GetTransformAxis(nodes[i],poly);
								mat    = nodes[i]->GetObjectTM(t,&valid) * Inverse(tmAxis);
								imat   = Inverse(mat);
								xproc->SetMat(mat);
						
								// XForm the cache vertices
								oldpt = spline->GetVert(vert);
								newpt = xproc->proc(oldpt,mat,imat,vertexWeight);
								spline->SetVert(vert,newpt);
								delta = newpt - oldpt;

								// Move the delta's vertices.
								shapeData->vdelta.Move(poly,vert,delta);

								// If it's a bezier knot, also affect its vectors
								if(spline->IsBezierPt(k)) {
									int in = vert - 1;
									int out = vert + 1;

									// XForm the cache vertices
									oldin = spline->GetVert(in);
									float inWeight = shape->VertexWeight( baseWeight+in );
									if ( vertexWeight > .9999 ) 
										inWeight = 1.0;
									else {
										if ( spline->GetKnotType(k) == KTYPE_BEZIER ) {
											spline->SetKnotType(k,KTYPE_BEZIER_CORNER);
											shapeData->vdelta.SetKType(poly,k,KTYPE_BEZIER ^ KTYPE_BEZIER_CORNER);
										}
									}

									spline->SetVert(in,xproc->proc(oldin,mat,imat,inWeight));
									delta = spline->GetVert(in) - oldin;

									// Move the delta's vertices.
									shapeData->vdelta.Move(poly,in,delta);

									// XForm the cache vertices
									oldout = spline->GetVert(out);
									float outWeight = shape->VertexWeight( baseWeight+out );
									if ( vertexWeight > .9999 ) outWeight = 1.0;
									spline->SetVert(out,xproc->proc(oldout,mat,imat,outWeight));

									// Move the delta's vertices.
									shapeData->vdelta.Move(poly,out,spline->GetVert(out) - oldout);
									}
								}

							}
						if(shapeData->vdelta.IsUsed(poly)) {
							spline->ComputeBezPoints();
							shape->InvalidateGeomCache();
							}
						baseWeight += knots*3;

						}
					}
					break;
				}			
			}
		else {
			mat = nodes[i]->GetObjectTM(t,&valid) * Inverse(tmAxis);
			imat = Inverse(mat);
			xproc->SetMat(mat);

			int baseWeight = 0;
			for(int poly = 0; poly < shape->splineCount; ++poly) {
				shapeData->vdelta.ClearUsed(poly);
				// Selected vertices - either directly or indirectly through selected faces or edges.
				BitArray sel = shape->VertexTempSel(poly);
				Spline3D *spline = shape->splines[poly];
				int knots = spline->KnotCount();
				// CAL-05/23/03: transform additional vertices. (FID #827)
				if (GetFlag(ES_ADDED_SELECT)) sel |= shape->VertexFlagSel(poly, SPLINEKNOT_ADD_SEL);
				for ( int k = 0; k < knots; k++ ) {
					int vert = k*3+1;
//watje 5-20-99
					BOOL bound = FALSE;
					if ((k==0) && (!spline->Closed()))
						{
						for (int bct = 0; bct < shape->bindList.Count(); bct++)
							{
							if ((shape->bindList[bct].pointSplineIndex == poly) && (!shape->bindList[bct].isEnd))
								bound = TRUE;
							}
						}
					else if ((k==(knots-1)) && (!spline->Closed()))
						{
						for (int bct = 0; bct < shape->bindList.Count(); bct++)
							{
							if ((shape->bindList[bct].pointSplineIndex == poly) && (shape->bindList[bct].isEnd))
								bound = TRUE;
							}
						}

					float vertexWeight;
					if ( sel[vert] ) 
						vertexWeight = 1.0;
					else 
						vertexWeight = shape->VertexWeight( baseWeight+vert );

					if ((fabs(vertexWeight) > 0.0001) && (!bound)) {

						shapeData->vdelta.SetUsed(poly);

							// XForm the cache vertices
							oldpt = spline->GetVert(vert);
							newpt = xproc->proc(oldpt,mat,imat,vertexWeight);
							spline->SetVert(vert,newpt);
							delta = newpt - oldpt;

							// Move the delta's vertices.
							shapeData->vdelta.Move(poly,vert,delta);

							// If it's a bezier knot, also affect its vectors
							if(spline->IsBezierPt(k)) {
								int in = vert - 1;
								int out = vert + 1;

								// XForm the cache vertices
								oldin = spline->GetVert(in);
								float inWeight = shape->VertexWeight( baseWeight+in );
									if ( vertexWeight > .9999 ) 
										inWeight = 1.0;
									else {
										if ( spline->GetKnotType(k) == KTYPE_BEZIER ) {
											spline->SetKnotType(k,KTYPE_BEZIER_CORNER);
											shapeData->vdelta.SetKType(poly,k,KTYPE_BEZIER ^ KTYPE_BEZIER_CORNER);
										}
									}

								spline->SetVert(in,xproc->proc(oldin,mat,imat,inWeight));

								// Move the delta's vertices.
								shapeData->vdelta.Move(poly,in,spline->GetVert(in) - oldin);

								// XForm the cache vertices
								oldout = spline->GetVert(out);
								float outWeight = shape->VertexWeight( baseWeight+out );
								if ( vertexWeight > .9999 ) outWeight = 1.0;
								spline->SetVert(out,xproc->proc(oldout,mat,imat,outWeight));

								// Move the delta's vertices.
								shapeData->vdelta.Move(poly,out,spline->GetVert(out) - oldout);
								}
							}
					}

				if(shapeData->vdelta.IsUsed(poly)) {
					spline->ComputeBezPoints();
					shape->InvalidateGeomCache();
					}
				baseWeight += knots*3;
				}
			}
		shapeData->UpdateChanges(shape);					
		shapeData->TempData(this)->Invalidate(PART_GEOM);
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	
	// Mark all objects in selection set
	SetAFlag(A_HELD);
	
	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
	}


#if 0 // methods already defined in editpat.cpp
Point3 RotateXForm::proc(Point3& p, Matrix3 &mat, Matrix3 &imat, float wt  ) 
{
}

Point3 ScaleXForm::proc(Point3& p, Matrix3 &mat, Matrix3 &imat, float wt ) 
{
}
#endif // methods already defined in editpat.cpp

void EditSplineMod::Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin )
	{
	MoveXForm proc(val);
	XFormVerts(&proc,t,partm,tmAxis); 	
	}

void EditSplineMod::Rotate( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin )
	{
	RotateXForm proc(val);
	XFormVerts(&proc,t,partm,tmAxis); 	
	}

void EditSplineMod::Scale( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin )
	{
	ScaleXForm proc(val);
	XFormVerts(&proc,t,partm,tmAxis); 	
	}

static IPoint2
ProjectPointI(GraphicsWindow *gw, Point3 fp) {
	IPoint3 out;
	gw->wTransPoint(&fp,&out);
	IPoint2 work;
	work.x = out.x + 1;
	work.y = out.y + 1;
	return work;
	}

static Point2
ProjectPointF(GraphicsWindow *gw, Point3 fp) {
	IPoint3 out;
	gw->wTransPoint(&fp,&out);
	Point2 work;
	work.x = (float)out.x;
	work.y = (float)out.y;
	return work;
	}

class AutoConnectPrompt {
	public:
		AutoConnectPrompt();
		AutoConnectPrompt(bool usingAutoConnect, bool useConfirmationPopUp, float threshold);
		BOOL DoIt(ViewExp *vpt, Matrix3 &tm, Point3 p1, Point3 p2);
	private:
		BOOL mPrompted;
		BOOL mDoIt;
		BOOL mTryToAutoConnect;
		BOOL mUseConfirmation;
		float mThreshold;
	};


#define HITSIZE 6


AutoConnectPrompt::AutoConnectPrompt()
{
	mPrompted = FALSE; 
	mDoIt = FALSE;
	mUseConfirmation = TRUE;
	mTryToAutoConnect = TRUE;
	mThreshold = HITSIZE;
}

AutoConnectPrompt::AutoConnectPrompt(bool tryToAutoConnect, bool useConfirmationPopUp, float threshold)
{
	mPrompted = FALSE; 
	mDoIt = FALSE;
	mUseConfirmation = useConfirmationPopUp;
	mTryToAutoConnect = tryToAutoConnect;
	mThreshold = threshold;
}

BOOL AutoConnectPrompt::DoIt(ViewExp *vpt, Matrix3 &tm, Point3 p1, Point3 p2) {
	GraphicsWindow *gw = vpt->getGW();
	gw->setTransform(tm);

	IPoint2 sp1 = ProjectPointI(gw, p1);
	IPoint2 sp2 = ProjectPointI(gw, p2);

	if (!mTryToAutoConnect)
		return FALSE;
	if(	abs(sp1.x - sp2.x) >= mThreshold || 
		abs(sp1.y - sp2.y) >= mThreshold)
		return FALSE;
	if(mPrompted)
		return mDoIt;
	TSTR s1 = GetString(IDS_TH_CONNECT_COINCIDENT);
	TSTR s2 = GetString(IDS_TH_EDITSPLINE);
	int result = IDYES;
	if (mUseConfirmation) 
		result = MessageBox(GetActiveWindow(), s1, s2, MB_YESNO);
	mDoIt = (result == IDYES) ? TRUE : FALSE;
	mPrompted = TRUE;
	if(!mDoIt)
		theHold.End();
	return mDoIt;
}


// The following is called before the first Move(), Rotate() or Scale() call
void EditSplineMod::TransformStart(TimeValue t) {
	if(!ip)
		return;
	ip->LockAxisTripods(TRUE);
	NotifyDependents(FOREVER, 0, REFMSG_SHAPE_START_CHANGE);
	BOOL shiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) ? TRUE : FALSE;
	if(shiftPressed) {
		// CAL-05/23/03: add supporting splines from original to the copy. (FID #827)
		BOOL addLinks = connectCopy;
		BOOL linkToOld = TRUE;			// TODO: should be parameterized?
		BitArray excludeEnds;
		float linkToOldThresh = connectCopyThreshold;
		int knotType = newKnotType;
		if(selLevel == ES_SEGMENT) {
			ModContextList mcList;		
			INodeTab nodes;
			Interval valid;

			theHold.Begin();
			BOOL needUndo = FALSE;

			ip->GetModContexts(mcList,nodes);
			ClearShapeDataFlag(mcList,ESD_BEENDONE);
			for ( int i = 0; i < mcList.Count(); i++ ) {
				EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
				if ( !shapeData ) continue;
				if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;
				
				// If the mesh isn't yet cache, this will cause it to get cached.
				BezierShape *shape = shapeData->TempData(this)->GetShape(t);
				if(!shape) continue;
				shapeData->PreUpdateChanges(shape);

				// Start a restore object...
				if ( theHold.Holding() ) {
					theHold.Put(new ShapeRestore(shapeData,this,shape));
					}

				// Go thru all polygons -- If any segs selected, copy 'em
				int polys = shape->SplineCount();
				BOOL altered = FALSE;
				// CAL-05/23/03: link the supporting splines to the original splines. (FID #827)
				if (addLinks && linkToOld) {
					excludeEnds.SetSize(2*polys);
					excludeEnds.ClearAll();
					// CAL-08/28/03: collect excludeEnds before calling CopySegments because the selection will be changed. (Defect #509802)
					for(int poly = 0; poly < polys; ++poly) {
						if(shape->segSel[poly].NumberSet()) {
							Spline3D *spline = shape->splines[poly];
							// CAL-05/23/03: exclude ends from the candidates of linking to old splines, if the
							// adjacent segment is selected. (FID #827)
							if (shape->segSel[poly][0]) excludeEnds.Set(2*poly);
							if (shape->segSel[poly][spline->Segments()-1]) excludeEnds.Set(2*poly+1);
							}
						}
					}
				for(int poly = 0; poly < polys; ++poly) {
					if(shape->segSel[poly].NumberSet()) {
						CopySegments(shape, poly, FALSE, TRUE);	// Actually copy it
						altered = needUndo = TRUE;
						}
					}
				// CAL-05/23/03: add supporting splines from original to the copy. (FID #827)
				if (addLinks && altered)
					AddConnectingSplines(shape, polys, linkToOld, linkToOldThresh, excludeEnds, knotType);

				if(altered) {
					shapeData->UpdateChanges(shape);
					shapeData->TempData(this)->Invalidate(PART_TOPO|PART_GEOM);
					}

				shapeData->TempData(this)->Invalidate(PART_GEOM);
				shapeData->SetFlag(ESD_BEENDONE,TRUE);
				}
			nodes.DisposeTemporary();
			if(needUndo)
				theHold.Accept(GetString(IDS_TH_COPY_SEGMENT));
			else
				theHold.End(); 		// Forget it!

			}
		else
		if(selLevel == ES_SPLINE) {
			ModContextList mcList;		
			INodeTab nodes;
			Interval valid;

			if ( !ip ) return;

			theHold.Begin();
			BOOL needUndo = FALSE;

			ip->GetModContexts(mcList,nodes);
			ClearShapeDataFlag(mcList,ESD_BEENDONE);
			for ( int i = 0; i < mcList.Count(); i++ ) {
				EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
				if ( !shapeData ) continue;
				if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;
				
				// If the mesh isn't yet cache, this will cause it to get cached.
				BezierShape *shape = shapeData->TempData(this)->GetShape(t);
				if(!shape) continue;
				shapeData->PreUpdateChanges(shape);
				
				// Start a restore object...
				if ( theHold.Holding() ) {
					theHold.Put(new ShapeRestore(shapeData,this,shape));
					}

				// Go thru all polygons -- If it's selected, copy it
				int polys = shape->SplineCount();
				BOOL altered = FALSE;
				// CAL-05/23/03: link the supporting splines to the original splines. (FID #827)
				if (addLinks && linkToOld) {
					excludeEnds.SetSize(2*polys);
					excludeEnds.ClearAll();
					for(int poly = 0; poly < polys; ++poly) {
						if(shape->polySel[poly]) {
							// CAL-05/23/03: exclude ends from the candidates of linking to old splines, if the
							// spline is selected. (FID #827)
							excludeEnds.Set(2*poly);
							excludeEnds.Set(2*poly+1);
							}
						}
					}
				for(int poly = 0; poly < polys; ++poly) {
					if(shape->polySel[poly]) {
						CopySpline(shape, poly, FALSE, TRUE);	// Actually copy it
						altered = needUndo = TRUE;
						}
					}
				// CAL-05/23/03: add supporting splines from original to the copy. (FID #827)
				if (addLinks && altered)
					AddConnectingSplines(shape, polys, linkToOld, linkToOldThresh, excludeEnds, knotType);

				if(altered) {
					shapeData->UpdateChanges(shape);
					shapeData->TempData(this)->Invalidate(PART_TOPO|PART_GEOM);
					}

				shapeData->TempData(this)->Invalidate(PART_GEOM);
				shapeData->SetFlag(ESD_BEENDONE,TRUE);
				}
			nodes.DisposeTemporary();
			if(needUndo)
				theHold.Accept(GetString(IDS_TH_COPY_SPLINE));
			else
				theHold.End(); 		// Forget it!
			}
		else {
			addLinks = FALSE;
			}
		// CAL-05/23/03: transform additional vertices. (FID #827)
		if (addLinks) SetFlag(ES_ADDED_SELECT);
		}
	}

// The following is called after the user has completed the Move, Rotate or Scale operation and
// the undo object has been accepted.
void EditSplineMod::TransformFinish(TimeValue t) {
	ModContextList mcList;		
	INodeTab nodes;
	Matrix3 mat,imat;	
	Interval valid;
	int numAxis;
	Point3 oldpt,newpt,oldin,oldout,rel,delta;
	AutoConnectPrompt thePrompt(SplineShape::GetUseEndPointAutoConnect()==BST_CHECKED, 
								SplineShape::GetPromptForEndPointAutoConnect()==BST_CHECKED, 
								SplineShape::GetEndPointAutoWeldThreshold());

	if ( !ip ) return;

	ViewExp *vpt = ip->GetViewport(NULL);

	ip->LockAxisTripods(FALSE);

	theHold.Begin();
	BOOL needUndo = FALSE;
	ip->GetModContexts(mcList,nodes);
	numAxis = ip->GetNumAxis();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;
		
		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		BOOL altered = FALSE;
		
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);
		shapeData->PreUpdateChanges(shape);

		Matrix3 nodeTM = nodes[i]->GetObjectTM(t);

		// CAL-05/23/03: clear flags for transformation of additional vertices. (FID #827)
		int polys = shape->SplineCount();
		if (GetFlag(ES_ADDED_SELECT)) {
			for (int poly = 0; poly < polys; poly++) {
				Spline3D *spline = shape->splines[poly];
				int knots = spline->KnotCount();
				for (int k = 0; k < knots; k++) {
					SplineKnot knot = spline->GetKnot(k);
					if (knot.GetFlag(SPLINEKNOT_ADD_SEL)) {
						knot.ClearFlag(SPLINEKNOT_ADD_SEL);
						spline->SetKnot(k, knot);
						}
					}
				}
			}

		changed:
		// Go thru all polygons -- If it's an open poly and it has a selected end vertex, see if
		// its end verts overlap -- if they do, prompt for closure.
		polys = shape->SplineCount();
		for(int poly = 0; poly < polys; ++poly) {
			Spline3D *spline = shape->splines[poly];
			if(!spline->Closed()) {
				int knots = spline->KnotCount();
				BitArray pSel = shape->VertexTempSel(poly);
				if(knots > 2 && (pSel[1] || pSel[(knots-1)*3+1])) {
					Point3 p1 = spline->GetKnotPoint(0);
					Point3 p2 = spline->GetKnotPoint(knots-1);
					if(thePrompt.DoIt(vpt, nodeTM, p1, p2)) {
						// Start a restore object...
						if ( theHold.Holding() )
							theHold.Put(new ShapeRestore(shapeData,this,shape));
						// Do the close
						if(pSel[1]) {
							DoPolyEndAttach(shape, poly, 0, poly, knots-1);
							}
						else {
							DoPolyEndAttach(shape, poly, knots-1, poly, 0);
							}
						shapeData->TempData(this)->Invalidate(PART_TOPO|PART_GEOM);
						altered = needUndo = TRUE;
						}
					}

				}
			}
		// Go thru all polygons -- If an open poly, and it has a selected end vertex, see if its
		// end vertices overlap the end vertex of another open poly.  If so, prompt for connection
		for(int poly1 = 0; poly1 < polys; ++poly1) {
			Spline3D *spline1 = shape->splines[poly1];
			if(!spline1->Closed()) {
				int knots1 = spline1->KnotCount();
				int lastKnot1 = knots1-1;
				int lastSel1 = lastKnot1 * 3 + 1;
				BitArray pSel = shape->VertexTempSel(poly1);
				if(pSel[1] || pSel[lastSel1]) {
					Point3 p1 = spline1->GetKnotPoint(0);
					Point3 p2 = spline1->GetKnotPoint(knots1-1);
					for(int poly2 = 0; poly2 < polys; ++poly2) {
						Spline3D *spline2 = shape->splines[poly2];
						if(poly1 != poly2 && !spline2->Closed()) {
							int knots2 = spline2->KnotCount();
							Point3 p3 = spline2->GetKnotPoint(0);
							Point3 p4 = spline2->GetKnotPoint(knots2-1);
							int vert1, vert2;
							if(pSel[1]) {
								if(thePrompt.DoIt(vpt, nodeTM, p3, p1)) {
									vert1 = 0;
									vert2 = 0;

									attach_it:
									// Start a restore object...
									if ( theHold.Holding() )
										theHold.Put(new ShapeRestore(shapeData,this,shape));
									// Do the attach
									DoPolyEndAttach(shape, poly1, vert1, poly2, vert2);
									shapeData->TempData(this)->Invalidate(PART_TOPO|PART_GEOM);
									altered = needUndo = TRUE;
									goto changed;
									}
								else
								if(thePrompt.DoIt(vpt, nodeTM, p4, p1)) {
									vert1 = 0;
									vert2 = knots2-1;
									goto attach_it;
									}
								}
							if(pSel[lastSel1]) {
								if(thePrompt.DoIt(vpt, nodeTM, p3, p2)) {
									vert1 = knots1-1;
									vert2 = 0;
									goto attach_it;
									}
								else
								if(thePrompt.DoIt(vpt, nodeTM, p4, p2)) {
									vert1 = knots1-1;
									vert2 = knots2-1;
									goto attach_it;
									}
								}
							}
						}
					}
				}
			}
		if(altered)
			shapeData->UpdateChanges(shape);
		shapeData->TempData(this)->Invalidate(PART_GEOM);
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	// CAL-05/23/03: clear flags for transformation of additional vertices. (FID #827)
	if (GetFlag(ES_ADDED_SELECT))		ClearFlag(ES_ADDED_SELECT);

	if(needUndo)
		theHold.Accept(GetString(IDS_TH_POLYCONNECT));
	else
		theHold.End(); 		// Forget it!
	
	nodes.DisposeTemporary();
	SelectionChanged();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	NotifyDependents(FOREVER, 0, REFMSG_SHAPE_END_CHANGE);
	NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
	if ( vpt ) ip->ReleaseViewport(vpt);
	}

// The following is called when the transform operation is cancelled by a right-click and the undo
// has been cancelled.
void EditSplineMod::TransformCancel(TimeValue t) {
//DebugPrint("Transform cancel\n");
	if (ip) ip->LockAxisTripods(FALSE);

	// CAL-05/23/03: clear flags for transformation of additional vertices. (FID #827)
	if (!ip) return;
	ModContextList mcList;		
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;
		
		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;

		int polys = shape->SplineCount();
		if (GetFlag(ES_ADDED_SELECT)) {
			for (int poly = 0; poly < polys; poly++) {
				Spline3D *spline = shape->splines[poly];
				int knots = spline->KnotCount();
				for (int k = 0; k < knots; k++) {
					SplineKnot knot = spline->GetKnot(k);
					if (knot.GetFlag(SPLINEKNOT_ADD_SEL)) {
						knot.ClearFlag(SPLINEKNOT_ADD_SEL);
						spline->SetKnot(k, knot);
						}
					}
				}
			}
		}
	// CAL-05/23/03: clear flags for transformation of additional vertices. (FID #827)
	if (GetFlag(ES_ADDED_SELECT))		ClearFlag(ES_ADDED_SELECT);

	nodes.DisposeTemporary();

	NotifyDependents(FOREVER, PART_ALL, REFMSG_SHAPE_END_CHANGE);
	}

void EditSplineMod::DoBoolean(int poly2)
	{
	ModContextList mcList;	
	INodeTab nodes;
	DWORD *clones = NULL;	
	BOOL altered = FALSE;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);

	theHold.Begin();

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;

		// If the mesh isn't yet cached, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(ip->GetTime());
		if(!shape) continue;

		if(shape == boolShape) {
			shapeData->PreUpdateChanges(shape);
			int newPolyNum;

			// Start a restore object...
			if ( theHold.Holding() ) {
				theHold.Put(new ShapeRestore(shapeData,this,shape));
				}
			int boolStat = PerformBoolean(boolShape, boolPoly1, poly2, boolType, &newPolyNum);
			switch(boolStat) {
				case BOOL_OK:
					altered = TRUE;
					shapeData->UpdateChanges(shape);
					shapeData->TempData(this)->Invalidate(PART_TOPO|PART_GEOM);
					if(newPolyNum >= 0)
						boolPoly1 = newPolyNum;
					else
						CancelEditSplineModes(ip);	// Cancel mode if more than one poly resulted
					break;
				default: {
					TSTR reason;
					switch(boolStat) {
						case BOOL_WELD_FAILURE:
							reason = GetString(IDS_TH_BOOLWELDFAILED);
							break;
						case BOOL_COINCIDENT_VERTEX:
							reason = GetString(IDS_TH_COINCIDENTVERTEX);
							break;
						case BOOL_MUST_OVERLAP:
							reason = GetString(IDS_TH_SPLINESMUSTOVERLAP);
							break;
						}
					ip->DisplayTempPrompt(reason,PROMPT_TIME);
					}
					break;
				}	
			goto done;
			}
		}
			
	// Accept all the outlines so they go on the undo stack
	done:
	if(altered)
		theHold.Accept(GetString(IDS_TH_BOOLEAN));
	else
		theHold.Cancel();

	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	SelectionChanged();
	NotifyDependents(FOREVER, PART_TOPO|PART_GEOM, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}					

void EditSplineMod::DoSegRefine(ViewExp *vpt, BezierShape *rshape, int poly, int seg, IPoint2 p) {
	ModContextList mcList;		
	INodeTab nodes;
	int holdNeeded = 0;
	int altered = 0;
	TimeValue t = ip->GetTime();
	
	Point3 sPoint = vpt->SnapPoint(p,p,NULL,SNAP_IN_3D);

	Point2 fp = Point2((float)p.x, (float)p.y);

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);

	theHold.Begin();

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);

		if(shape == rshape) {
			shapeData->PreUpdateChanges(shape);
			
			// Find the location on the segment where the user clicked
			INode *inode = nodes[i];
			GraphicsWindow *gw = vpt->getGW();
			Matrix3 mat = inode->GetObjectTM(t);
			gw->setTransform(mat);	

			Spline3D *spline = shape->splines[poly];

			// Go thru the segment and narrow down where they hit
			float rez = 0.01f;
			float lorez = 0.0f;
			float hirez = 1.0f;
			// Start with a rough estimate
			HitRegion hr;
			MakeHitRegion(hr, HITTYPE_POINT, 1, 4, &p);
			gw->setHitRegion(&hr);
			float bestParam = shape->FindSegmentPoint(poly, seg, gw, gw->getMaterial(), &hr);
			Point3 pt = spline->InterpBezier3D(seg, bestParam);
			Point2 sp = ProjectPointF(gw, pt);
			float bestDist = 1000.0f;
			int iBestDist = 1000;
			if(bestParam > 0.0f) {
				bestDist = Length(sp - fp);
				iBestDist = (int)bestDist;
				}
			BOOL bestChanged = TRUE;
			while((rez > 0.00005f) || bestChanged) {
				bestChanged = FALSE;
				for(float sample = lorez; sample <= hirez; sample += rez) {
					pt = spline->InterpBezier3D(seg, sample);
					sp = ProjectPointF(gw, pt);
					float dist = Length(sp - fp);
					if(dist < bestDist) {
						int ibd = (int)dist;
						if(ibd < iBestDist) {
							bestChanged = TRUE;
							iBestDist = ibd;
							}
						bestDist = dist;
						bestParam = sample;
						if(bestDist <= 0.5)
							goto got_it;
						}
					}
				lorez = bestParam - rez;
				if(lorez < 0.0f)
					lorez = 0.0f;
				hirez = bestParam + rez;
				if(hirez > 1.0f)
					hirez = 1.0f;
				rez /= 10.0f;
				}

			got_it:
			if(iBestDist < 1000) {			
				altered = holdNeeded = 1;
				if ( theHold.Holding() ) {
					theHold.Put(new ShapeRestore(shapeData,this,shape));
					}
				RefineSegment(shape, poly, seg, bestParam, TRUE);
				shapeData->UpdateChanges(shape);
				shapeData->TempData(this)->Invalidate(PART_TOPO);
				theHold.Accept(GetString(IDS_TH_REFINE));
				}
			else
				theHold.Cancel();
			goto finished;
			}
		}
	finished:
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(t, REDRAW_NORMAL);
	}

static void DivideSegment(BezierShape *shape, int poly, int divisions) {
	Spline3D *spline = shape->splines[poly];
	int segments = spline->Segments();
	// First tag auto points for later fixup 
	for(int k = 0; k < spline->KnotCount(); ++k) {
		int nextKnot = (k + 1) % spline->KnotCount();
		if(spline->GetKnotType(k) == KTYPE_AUTO) {
			if(spline->GetKnotType(nextKnot) == KTYPE_AUTO)
				spline->SetAux3(k, 1111);	// Span to be all auto
			else
				spline->SetAux3(k, 111);		// Span left alone
			}
		else
			spline->SetAux3(k, 222);	// Leave these alone
		}
	// Now convert all auto points to bezier corner
	for(k = 0; k < spline->KnotCount(); ++k) {
		if(spline->GetKnotType(k) == KTYPE_AUTO)
			spline->SetKnotType(k, KTYPE_BEZIER_CORNER);
		}
	// Now refine the curve
	for(int seg = segments - 1; seg >= 0; --seg) {
		if(shape->segSel[poly][seg]) {
			// Perform a recursive division of the segment from the end
			for(int div = divisions; div > 0; --div) {
				float pos = (float)div / (float)(div + 1);
				RefineSegment(shape, poly, seg, pos);
				}
			}
		}
	// Now convert all saved auto points back! 
	int inAuto = FALSE;
	for(k = 0; k < spline->KnotCount(); ++k) {
		switch(spline->GetAux3(k)) {
			case 111:
				inAuto = FALSE;
				spline->SetKnotType(k, KTYPE_AUTO);
				break;
			case 1111:
				inAuto = TRUE;
				spline->SetKnotType(k, KTYPE_AUTO);
				break;
			case 222:
				break;
			default:	// New point
				if(inAuto)
					spline->SetKnotType(k, KTYPE_AUTO);
				break;
			}
		}
	spline->ComputeBezPoints();
	}

void EditSplineMod::DoSegDivide(int divisions) {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	theHold.Begin();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		int altered = 0;
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);
		shapeData->PreUpdateChanges(shape);

		int polys = shape->splineCount;
		for(int poly = polys - 1; poly >= 0; --poly) {
			Spline3D *spline = shape->splines[poly];
			// If any bits are set in the selection set, let's DO IT!!
			if(shape->segSel[poly].NumberSet()) {
				altered = holdNeeded = 1;
				if ( theHold.Holding() )
					theHold.Put(new ShapeRestore(shapeData,this,shape));
				// Call the segment divide function
				DivideSegment(shape, poly, divisions);
				}
			}

		if(altered) {
			shapeData->UpdateChanges(shape);
			shapeData->TempData(this)->Invalidate(PART_TOPO);
			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	
	if(holdNeeded)
		theHold.Accept(GetString(IDS_TH_DIVIDE_SEGMENT));
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOSEGSSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

// Reverse all selected polygons
void EditSplineMod::DoPolyReverse() {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	theHold.Begin();
	MaybeSelectSingleSpline();

	for ( int i = 0; i < mcList.Count(); i++ ) {
		int altered = 0;
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);
		shapeData->PreUpdateChanges(shape);

		int polys = shape->splineCount;
		for(int poly = polys-1; poly >= 0; --poly) {
			if(shape->polySel[poly]) {
				Spline3D *spline = shape->splines[poly];
				altered = holdNeeded = 1;
				// Save the unmodified verts.
				if ( theHold.Holding() ) {
					theHold.Put(new ShapeRestore(shapeData,this,shape));
					}
				shape->Reverse(poly, TRUE);
				}
			}

		if(altered) {
			shapeData->UpdateChanges(shape);
			shapeData->TempData(this)->Invalidate(PART_TOPO);
			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	
	if(holdNeeded)
		theHold.Accept(GetString(IDS_TH_REVERSE_SPLINE));
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOSPLINESSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	SelectionChanged();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

static TSTR explodeName;

static INT_PTR CALLBACK ExplodeDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	TCHAR tempName[256];
	switch (msg) {
	case WM_INITDIALOG:
		SetDlgItemText(hWnd, IDC_ES_EXPLODE_NAME, explodeName);
		CenterWindow(hWnd, GetParent(hWnd));
		SendMessage(GetDlgItem(hWnd,IDC_ES_EXPLODE_NAME), EM_SETSEL,0,-1);
		SetFocus(GetDlgItem(hWnd,IDC_ES_EXPLODE_NAME));
		return FALSE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			GetDlgItemText(hWnd, IDC_ES_EXPLODE_NAME, tempName, 255);
			explodeName = TSTR(tempName);
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

static BOOL GetExplodeObjectName (IObjParam *ip, TSTR &name) {
	explodeName = name;
	ip->MakeNameUnique(explodeName);	
	if (DialogBox(hInstance, MAKEINTRESOURCE(IDD_ES_EXPLODE), ip->GetMAXHWnd(), ExplodeDlgProc)) {
		name = explodeName;
		return TRUE;
		}
	return FALSE;
}

// Explode all selected polygons
void EditSplineMod::DoPolyExplode() {
	ModContextList mcList;		
	INodeTab nodes;

	if ( !ip ) return;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	theHold.Begin();
	MaybeSelectSingleSpline();

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;

		if(shape->polySel.sel.NumberSet()) {
			int altered = 0;
			// If this is the first edit, then the delta arrays will be allocated
			shapeData->BeginEdit(t);
			shapeData->PreUpdateChanges(shape);
			// Save the unmodified verts.
			if ( theHold.Holding() ) {
				theHold.Put(new ShapeRestore(shapeData,this,shape));
				}

			int polys = shape->splineCount;
			for(int poly = polys-1; poly >= 0; --poly) {
				if(shape->polySel[poly]) {
					Spline3D *spline = shape->splines[poly];
					altered = holdNeeded = 1;
					// Select all the vertices
					shape->vertSel[poly].ClearAll();
					for(int i = 1; i < shape->splines[poly]->Verts(); i+=3)
						shape->vertSel[poly].Set(i);
					// Call the vertex break function!
					BreakSplineAtSelVerts(shape, poly);
					}
				}

			if(altered) {
				shapeData->UpdateChanges(shape);
				shapeData->TempData(this)->Invalidate(PART_TOPO);
				}
			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	
	if(holdNeeded)
		theHold.Accept(GetString(IDS_TH_EXPLODE));
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOSPLINESSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	SelectionChanged();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

void EditSplineMod::DoExplodeToObjects() {
	ModContextList mcList;		
	INodeTab nodes;

	if ( !ip ) return;

	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	int dialoged = 0;
	TSTR newName(GetString(IDS_TH_SHAPE));

	theHold.Begin();
	MaybeSelectSingleSpline();

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;

		if(shape->polySel.sel.NumberSet()) {
			// If this is the first edit, then the delta arrays will be allocated
			shapeData->BeginEdit(t);
			shapeData->PreUpdateChanges(shape);
			
			// Save the unmodified verts.
			if ( theHold.Holding() ) {
				theHold.Put(new ShapeRestore(shapeData,this,shape));
				}
			SplineShape *splShape;

			BOOL altered = FALSE;
			int polys = shape->splineCount;
			for(int poly = polys - 1; poly >= 0; --poly) {
				if(shape->polySel[poly]) {
					Spline3D *spline = shape->splines[poly];
					shape->segSel[poly].ClearAll();
					int segs = spline->Segments();
					while(segs > 0) {
						segs--;
						shape->segSel[poly].Set(0);
						if(!dialoged) {
							dialoged = 1;
							if(!GetExplodeObjectName(ip, newName))
								goto bail_out;
							}
						else
							ip->MakeNameUnique(newName);

						altered = holdNeeded = 1;					
						splShape = new SplineShape;
						HandleSegDetach(shape, &splShape->shape, SDT_DETACH, poly);
						splShape->shape.UpdateSels();	// Make sure it readies the selection set info
						splShape->shape.InvalidateGeomCache();
						ObjectState os =
							nodes[0]->GetObjectRef()->Eval(t);
						GeomObject *object = (GeomObject *)os.obj;
						if(object->IsShapeObject()) {
							// Set base parameters in new shape
							splShape->CopyBaseData(*((ShapeObject *)object));
							// And optimize/adaptive
							if(object->IsSubClassOf(splineShapeClassID)) {
								SplineShape *ss = (SplineShape *)object;
								splShape->shape.optimize = ss->shape.optimize;
								splShape->steps = ss->steps;
								splShape->shape.steps = ss->shape.steps;
								}
							}
						INode *newNode = ip->CreateObjectNode(splShape);
						newNode->CopyProperties(nodes[0]);
						newNode->SetName(newName.data());
						Matrix3 tm = nodes[0]->GetObjectTM(t);
						newNode->SetNodeTM(t, tm);	// Use this object's TM.
						newNode->FlagForeground(t);		// WORKAROUND!
						}
					}
				}
			if(altered) {
				shape->UpdateSels();
				shapeData->UpdateChanges(shape);
				}
			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}

	bail_out:
	if(holdNeeded) {
		theHold.Accept(GetString(IDS_TH_EXPLODE));
		}
	else {
		if(!dialoged)
			ip->DisplayTempPrompt(GetString(IDS_TH_NOSPLINESSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	SelectionChanged();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(t, REDRAW_NORMAL);
	}

// Helper class to locate closest points on segments

#define CSP_STEPS 50

class ClosestSegPoint {
	public:
		Spline3D *spline;
		int segment;
		float min;
		float max;
		float step;
		float bestParam;
		int bestIndex;
		float bestDist;
		BOOL gotBestIndex;
		Point3 interps[CSP_STEPS + 1];
		ClosestSegPoint(Spline3D *s, int seg) {
			spline = s;
			segment = seg;
			min = 0.0f;
			max = 1.0f;
			gotBestIndex = FALSE;
			}
		void Interpolate();
		void NextLevel(ClosestSegPoint &other);
		void SetBestIndex(int value) { bestIndex = value; gotBestIndex = TRUE; }
	};

void ClosestSegPoint::Interpolate() {
	int i;
	float param;
	step = (max - min) / (float)CSP_STEPS;
	for(i = 0, param = min; i < (CSP_STEPS + 1); ++i, param += step)
		interps[i] = spline->InterpBezier3D(segment, param);
	gotBestIndex = FALSE;
	}

void ClosestSegPoint::NextLevel(ClosestSegPoint &other) {
	if(!gotBestIndex) {
		bestIndex = -1;
		bestDist = 0.0f;
		for(int i = 0; i < (CSP_STEPS + 1); ++i) {
			for(int j = 0; j < (CSP_STEPS + 1); ++j) {
				float dist = Length(interps[i] - other.interps[j]);
				if(bestIndex < 0 || dist < bestDist) {
					SetBestIndex(i);
					other.SetBestIndex(j);
					bestDist = other.bestDist = dist;
					}
				}
			}
		}
	if(bestIndex == 0) {
		max = min + step;
		}
	else
	if(bestIndex == CSP_STEPS) {
		min = max - step;
		}
	else {
		max = min + (float)(bestIndex + 1) * step;
		min = min + (float)(bestIndex - 1) * step;
		}
	bestParam = (min + max) / 2.0f;
	}

#define CROSS_INSERT_LOOPS 20

void EditSplineMod::DoCrossInsert(ViewExp *vpt, BezierShape *rshape, int poly1, int seg1, int poly2, int seg2, IPoint2 p) {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	Point2 fp = Point2((float)p.x, (float)p.y);

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);

	theHold.Begin();

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);

		if(shape == rshape) {
			shapeData->PreUpdateChanges(shape);
			if ( theHold.Holding() )
				theHold.Put(new ShapeRestore(shapeData,this,shape));

			// Find the location where the segments are closest
			ClosestSegPoint csp1(rshape->GetSpline(poly1),seg1);
			ClosestSegPoint csp2(rshape->GetSpline(poly2),seg2);
			for(int loops = 0; loops < CROSS_INSERT_LOOPS; ++loops) {
				csp1.Interpolate();
				csp2.Interpolate();
				csp1.NextLevel(csp2);
				csp2.NextLevel(csp1);
				}

			// When we get here, we have a pretty good idea of where the best point is
			// Make sure it's within the user's threshold!
			if(csp1.bestDist <= crossThreshold) {
				RefineSegment(shape, poly1, seg1, csp1.bestParam);
				RefineSegment(shape, poly2, seg2, csp2.bestParam);
				shapeData->UpdateChanges(shape);
				shapeData->TempData(this)->Invalidate(PART_TOPO);
				theHold.Accept(GetString(IDS_TH_CROSS_INSERT));
				NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
				ip->RedrawViews(t, REDRAW_NORMAL);
				}
			else {
				ip->DisplayTempPrompt(GetString(IDS_TH_CROSS_NOT_IN_THRESHOLD),PROMPT_TIME);
				theHold.End();
				}
			goto finished;
			}
		}

finished:
	nodes.DisposeTemporary();
	}

void EditSplineMod::DoSegBreak(ViewExp *vpt, BezierShape *rshape, int poly, int seg, IPoint2 p) {
	ModContextList mcList;		
	INodeTab nodes;
	int holdNeeded = 0;
	int altered = 0;
	TimeValue t = ip->GetTime();

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);

	theHold.Begin();

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);

		if(shape == rshape) {
			shapeData->PreUpdateChanges(shape);

			// Find the location on the segment where the user clicked
			INode *inode = nodes[i];
			GraphicsWindow *gw = vpt->getGW();
			HitRegion hr;
			MakeHitRegion(hr, HITTYPE_POINT, 1, 4, &p);
			gw->setHitRegion(&hr);
			Matrix3 mat = inode->GetObjectTM(t);
			gw->setTransform(mat);	
	
			float param = shape->FindSegmentPoint(poly, seg, gw, gw->getMaterial(), &hr);
			
			altered = holdNeeded = 1;
			if ( theHold.Holding() ) {
				theHold.Put(new ShapeRestore(shapeData,this,shape));
				}
			shapeData->UpdateChanges(shape);
			shapeData->TempData(this)->Invalidate(PART_TOPO);
			theHold.Accept(GetString(IDS_TH_SEGBREAK));
			BreakSegment(shape, poly, seg, param);
			goto finished;
			}
		}
	finished:
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(t, REDRAW_NORMAL);
	}

void EditSplineMod::DoVertConnect(ViewExp *vpt, BezierShape *rshape, int poly1, int vert1, int poly2, int vert2) {
	ModContextList mcList;		
	INodeTab nodes;
	int holdNeeded = 0;
	int altered = 0;
	TimeValue t = ip->GetTime();

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);

	theHold.Begin();

	// Verts may be vectors -- Make sure they indicate knots
	vert1 = (vert1 / 3) * 3 + 1;
	vert2 = (vert2 / 3) * 3 + 1;

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);

		if(shape == rshape) {
			shapeData->PreUpdateChanges(shape);
			altered = holdNeeded = 1;
			if ( theHold.Holding() ) {
				theHold.Put(new ShapeRestore(shapeData,this,shape));
				}
			ConnectVerts(shape, poly1, vert1, poly2, vert2, TRUE);
			shapeData->UpdateChanges(shape);
			shapeData->TempData(this)->Invalidate(PART_TOPO);
			theHold.Accept(GetString(IDS_TH_VERTCONNECT));
			goto finished;
			}
		}
	finished:
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(t, REDRAW_NORMAL);
	}

// Start inserting vertices -- Prep the spline object and start a record of the change.
// Return the index of the vertex where insertion will begin
int EditSplineMod::StartVertInsert(ViewExp *vpt, BezierShape *rshape, int poly, int seg, int vert, EditSplineMod **mod) {
	ModContextList mcList;		
	INodeTab nodes;
	int holdNeeded = 0;
	int altered = 0;
	TimeValue t = ip->GetTime();

	if ( !ip ) return -1;

	ip->GetModContexts(mcList,nodes);

	theHold.Begin();

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);

		if(shape == rshape) {
			shapeData->PreUpdateChanges(shape);
			altered = holdNeeded = 1;
			if ( theHold.Holding() ) {
				theHold.Put(new ShapeRestore(shapeData,this,shape));
				}
			insertShape = shape;
			// Insert the points into the spline
			Spline3D *spline = shape->splines[poly];			
			insertSpline = spline;
			insertPoly = poly;
			// If inserting at first vertex, reverse the spline and relocate the insertion point!
			if(vert == 1) {
				spline->Reverse();
				shape->vertSel[poly].Reverse();
				shape->segSel[poly].Reverse();
				vert = (spline->KnotCount() - 1) * 3 + 1;

				}
			else	// If segment, find the insertion vertex
			if(seg >= 0)
				{
				vert = seg * 3 + 1;
				}

			insertNode = nodes[i]->GetActualINode();
			insertTM = nodes[i]->GetObjectTM(ip->GetTime());
			insertVert = vert;
			insertShapeData = shapeData;
			*mod = this;
			shapeData->TempData(this)->Invalidate(PART_TOPO);
			nodes.DisposeTemporary();
			return vert;
			}
		}
	nodes.DisposeTemporary();
	return -1;
	}

void EditSplineMod::EndVertInsert() {
	if(!insertShape)
		return;
	// Save the resulting spline to the restore record
	insertShapeData->UpdateChanges(insertShape);
	theHold.Accept(GetString(IDS_TH_VERTINSERT));
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
	insertShape->InvalidateGeomCache();
	insertShape = NULL;
	}

void EditSplineMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) 
	{		
//Alert(_T("in ModifyObject"));
	assert( os->obj->ClassID() == Class_ID(SPLINESHAPE_CLASS_ID,0) );
//Alert(_T("ModifyObject class ID is OK"));
	
	SplineShape *splShape = (SplineShape *)os->obj;

	if ( !mc.localData )
		mc.localData = new EditSplineData();	// Create it 1st time

	EditSplineData *shapeData = (EditSplineData*)mc.localData;
	shapeData->Apply(t,splShape,selLevel,showVertNumbers,SVNSelectedOnly);
	splShape->InvalidateGeomCache();
	}

void EditSplineMod::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc)
	{
	if ( mc->localData ) {
		EditSplineData *shapeData = (EditSplineData*)mc->localData;
		if ( shapeData ) {
			// The FALSE parameter indicates the the mesh cache itself is
			// invalid in addition to any other caches that depend on the
			// mesh cache.
			shapeData->Invalidate(partID,FALSE);
			}
		}
	}

// Select a subcomponent within our object(s).  WARNING! Because the HitRecord list can
// indicate any of the objects contained within the group of shapes being edited, we need
// to watch for control breaks in the shapeData pointer within the HitRecord!

void EditSplineMod::SelectSubComponent( HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert )
	{
	if ( !ip ) return; 
	ip->ClearCurNamedSelSet();
	TimeValue t = ip->GetTime();
	EndOutlineMove(t);
//watje 
	int i;

	// Keep processing hit records as long as we have them!
	while(hitRec) {	
		EditSplineData *shapeData = (EditSplineData*)hitRec->modContext->localData;
	
		if ( !shapeData )
			return;

		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) return;

		shapeData->BeginEdit(t);
		shapeData->PreUpdateChanges(shape, FALSE);

		if ( theHold.Holding() ) {
			theHold.Put(new ShapeRestore(shapeData,this,shape));
			}

		switch ( selLevel ) {
			case ES_VERTEX: {
				if ( all ) {
					if ( invert ) {
						while( hitRec ) {
							// If the object changes, we're done!
							if(shapeData != (EditSplineData*)hitRec->modContext->localData)
								goto vert_done;
							int poly = ((ShapeHitData *)(hitRec->hitData))->poly;
							int vert = ((ShapeHitData *)(hitRec->hitData))->index;
							if(shape->vertSel[poly][vert])
								shape->vertSel[poly].Clear(vert);
							else
								shape->vertSel[poly].Set(vert);
							hitRec = hitRec->Next();
							}
						}
					else
					if ( selected ) {
						while( hitRec ) {
							// If the object changes, we're done!
							if(shapeData != (EditSplineData*)hitRec->modContext->localData)
								goto vert_done;
							shape->vertSel[((ShapeHitData *)(hitRec->hitData))->poly].Set(((ShapeHitData *)(hitRec->hitData))->index);
							hitRec = hitRec->Next();
							}
						}
					else {
						while( hitRec ) {
							// If the object changes, we're done!
							if(shapeData != (EditSplineData*)hitRec->modContext->localData)
								goto vert_done;
							shape->vertSel[((ShapeHitData *)(hitRec->hitData))->poly].Clear(((ShapeHitData *)(hitRec->hitData))->index);
							hitRec = hitRec->Next();
							}
						}
					}
				else {
					ShapeHitData *hit = (ShapeHitData *)(hitRec->hitData);
					int poly = hit->poly;
					int vert = hit->index;
					if ( invert ) {
						if(shape->vertSel[poly][vert])
							shape->vertSel[poly].Clear(vert);
						else
							shape->vertSel[poly].Set(vert);
						}
					else
					if ( selected ) {
						shape->vertSel[poly].Set(vert);
						}
					else {
						shape->vertSel[poly].Clear(vert);
						}
					hitRec = NULL;	// Reset it so we can exit	
					}

//watje
				if (useAreaSelect)
					{
					float dist = areaSelect * areaSelect;

					for (i = 0; i < shape->splineCount; i++)  //wow managed for nested loops that sucks
						{
						for (int j = 0; j < shape->splines[i]->KnotCount(); j++)
							{
							if (shape->vertSel[i][j*3+1])
								{
								Point3 p = shape->splines[i]->GetKnotPoint(j);
								for (int k = 0; k < shape->splineCount; k++)
									{
									for (int m = 0; m < shape->splines[k]->KnotCount(); m++)
										{
										if ((!shape->vertSel[k][m*3+1]) && (LengthSquared(p - shape->splines[k]->GetKnotPoint(m)) < dist))
											shape->vertSel[k].Set(m*3+1);
										}
									}
	
								}

							}

						}


					}

				vert_done:
				break;
				}

			case ES_SEGMENT: {
				if ( all ) {				
					if ( invert ) {
						while( hitRec ) {
							// If the object changes, we're done!
							if(shapeData != (EditSplineData*)hitRec->modContext->localData)
								goto seg_done;
							int poly = ((ShapeHitData *)(hitRec->hitData))->poly;
							int seg = ((ShapeHitData *)(hitRec->hitData))->index;
							if(shape->segSel[poly][seg])
								shape->segSel[poly].Clear(seg);
							else
								shape->segSel[poly].Set(seg);
							hitRec = hitRec->Next();
							}
						}
					else
					if ( selected ) {
						while( hitRec ) {
							// If the object changes, we're done!
							if(shapeData != (EditSplineData*)hitRec->modContext->localData)
								goto seg_done;
							shape->segSel[((ShapeHitData *)(hitRec->hitData))->poly].Set(((ShapeHitData *)(hitRec->hitData))->index);
							hitRec = hitRec->Next();
							}
						}
					else {
						while( hitRec ) {
							// If the object changes, we're done!
							if(shapeData != (EditSplineData*)hitRec->modContext->localData)
								goto seg_done;
							shape->segSel[((ShapeHitData *)(hitRec->hitData))->poly].Clear(((ShapeHitData *)(hitRec->hitData))->index);
							hitRec = hitRec->Next();
							}
						}
					}
				else {
					int poly = ((ShapeHitData *)(hitRec->hitData))->poly;
					int seg = ((ShapeHitData *)(hitRec->hitData))->index;
					if ( invert ) {
						if(shape->segSel[poly][seg])
							shape->segSel[poly].Clear(seg);
						else
							shape->segSel[poly].Set(seg);
						}
					else
					if ( selected ) {
						shape->segSel[poly].Set(seg);
						}
					else {
						shape->segSel[poly].Clear(seg);
						}	
					hitRec = NULL;	// Reset it so we can exit	
					}
				seg_done:
				break;
				}

			case ES_SPLINE: {
				if ( all ) {				
					if ( invert ) {
						while( hitRec ) {
							// If the object changes, we're done!
							if(shapeData != (EditSplineData*)hitRec->modContext->localData)
								goto poly_done;
							int poly = ((ShapeHitData *)(hitRec->hitData))->poly;
							if(shape->polySel[poly])
								shape->polySel.Clear(poly);
							else
								shape->polySel.Set(poly);
							hitRec = hitRec->Next();
							}
						}
					else
					if ( selected ) {
						while( hitRec ) {
							// If the object changes, we're done!
							if(shapeData != (EditSplineData*)hitRec->modContext->localData)
								goto poly_done;
							shape->polySel.Set(((ShapeHitData *)(hitRec->hitData))->poly);
							hitRec = hitRec->Next();
							}
						}
					else {
						while( hitRec ) {
							// If the object changes, we're done!
							if(shapeData != (EditSplineData*)hitRec->modContext->localData)
								goto poly_done;
							shape->polySel.Clear(((ShapeHitData *)(hitRec->hitData))->poly);
							hitRec = hitRec->Next();
							}
						}
					}
				else {
					int poly = ((ShapeHitData *)(hitRec->hitData))->poly;
					if ( invert ) {
						if(shape->polySel[poly])
							shape->polySel.Clear(poly);
						else
							shape->polySel.Set(poly);
						}
					else
					if ( selected ) {
						shape->polySel.Set(poly);
						}
					else {
						shape->polySel.Clear(poly);
						}	
					hitRec = NULL;	// Reset it so we can exit	
					}
				poly_done:
				break;
				}
			case ES_OBJECT:
			default:
				return;
			}
		shapeData->UpdateChanges(shape, FALSE);
		if ( shapeData->tempData ) {
			shapeData->tempData->Invalidate(PART_SELECT);
			}
		}

	SelectionChanged();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	}

void EditSplineMod::ClearSelection(int selLevel) 
	{
	if(selLevel == ES_OBJECT)
		return;

	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;	
	ip->ClearCurNamedSelSet();
	EndOutlineMove(ip->GetTime());
	
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;		
		BezierShape *shape = shapeData->TempData(this)->GetShape(ip->GetTime());
		if(!shape) continue;

		shapeData->BeginEdit(ip->GetTime());
		shapeData->PreUpdateChanges(shape, FALSE);

		switch ( selLevel ) {
			case ES_VERTEX: {
				if ( theHold.Holding() ) {
					theHold.Put(new ShapeRestore(shapeData,this,shape));
					}
				shape->vertSel.ClearAll();
				break;
				}

			case ES_SEGMENT: {
				if ( theHold.Holding() ) {
					theHold.Put(new ShapeRestore(shapeData,this,shape));
					}
				shape->segSel.ClearAll();
				break;
				}

			case ES_SPLINE: {
				if ( theHold.Holding() ) {
					theHold.Put(new ShapeRestore(shapeData,this,shape));
					}
				shape->polySel.ClearAll();
				break;
				}
			}
		shapeData->UpdateChanges(shape, FALSE);
		if ( shapeData->tempData ) {
			shapeData->TempData(this)->Invalidate(PART_SELECT);
			}
		}
	nodes.DisposeTemporary();
	SelectionChanged();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	}

void EditSplineMod::SelectAll(int selLevel) 
	{
	if(selLevel == ES_OBJECT)
		return;

	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;	
	ip->ClearCurNamedSelSet();
	EndOutlineMove(ip->GetTime());
	
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;		
		BezierShape *shape = shapeData->TempData(this)->GetShape(ip->GetTime());
		if(!shape) continue;
		shapeData->BeginEdit(ip->GetTime());
		shapeData->PreUpdateChanges(shape, FALSE);

		switch ( selLevel ) {
			case ES_VERTEX: {
				if ( theHold.Holding() ) {
					theHold.Put(new ShapeRestore(shapeData,this,shape));
					}
				shape->vertSel.SetAll();
				shape->UnselectHiddenVerts();

				break;
				}

			case ES_SEGMENT: {
				if ( theHold.Holding() ) {
					theHold.Put(new ShapeRestore(shapeData,this,shape));
					}
				shape->segSel.SetAll();
				shape->UnselectHiddenSegs();
				break;
				}

			case ES_SPLINE: {
				if ( theHold.Holding() ) {
					theHold.Put(new ShapeRestore(shapeData,this,shape));
					}
				shape->polySel.SetAll();
				shape->UnselectHiddenSplines();
				break;
				}
			}
		shapeData->UpdateChanges(shape, FALSE);
		if ( shapeData->tempData ) {
			shapeData->TempData(this)->Invalidate(PART_SELECT);
			}
		}
	nodes.DisposeTemporary();
	SelectionChanged();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	}

void EditSplineMod::InvertSelection(int selLevel) 
	{
	if(selLevel == ES_OBJECT)
		return;

	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;	
	ip->ClearCurNamedSelSet();
	EndOutlineMove(ip->GetTime());
	
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;		
		BezierShape *shape = shapeData->TempData(this)->GetShape(ip->GetTime());
		if(!shape) continue;
		shapeData->BeginEdit(ip->GetTime());
		shapeData->PreUpdateChanges(shape, FALSE);

		switch ( selLevel ) {
			case ES_VERTEX: {
				if ( theHold.Holding() ) {
					theHold.Put(new ShapeRestore(shapeData,this,shape));
					}
				shape->vertSel.Toggle();
				shape->UnselectHiddenVerts();
				break;
				}

			case ES_SEGMENT: {
				if ( theHold.Holding() ) {
					theHold.Put(new ShapeRestore(shapeData,this,shape));
					}
				shape->segSel.Toggle();
				shape->UnselectHiddenSegs();
				break;
				}

			case ES_SPLINE: {
				if ( theHold.Holding() ) {
					theHold.Put(new ShapeRestore(shapeData,this,shape));
					}
				shape->polySel.Toggle();
				shape->UnselectHiddenSplines();
				break;
				}
			}
		shapeData->UpdateChanges(shape, FALSE);
		if ( shapeData->tempData ) {
			shapeData->TempData(this)->Invalidate(PART_SELECT);
			}
		}
	nodes.DisposeTemporary();
	SelectionChanged();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	}

void EditSplineMod::MaybeSelectSingleSpline(BOOL makeUndo) {
	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;	
	EndOutlineMove(ip->GetTime());
	
	ip->GetModContexts(mcList,nodes);
	int splines = 0;
	BezierShape *theShape = NULL;
	EditSplineData *theESD = NULL;
	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;		
		BezierShape *shape = shapeData->TempData(this)->GetShape(ip->GetTime());
		if(!shape) continue;
		splines += shape->SplineCount();
		if(splines > 1) {
			nodes.DisposeTemporary();
			return;
			}
		if(shape->polySel[0])
			return;
		if(splines) {
			theShape = shape;
			theESD = shapeData;
			}
		}
	if(theShape) {
		theESD->BeginEdit(ip->GetTime());
		theESD->PreUpdateChanges(theShape, FALSE);
		if(makeUndo)
			theHold.Begin();
		if ( theHold.Holding() )
			theHold.Put(new ShapeRestore(theESD,this,theShape));
		if(makeUndo)
			theHold.Accept(GetString(IDS_DS_SELECT));
		theShape->polySel.Set(0);
		theESD->UpdateChanges(theShape, FALSE);
		if ( theESD->tempData ) {
			theESD->TempData(this)->Invalidate(PART_SELECT);
			}
		}
	nodes.DisposeTemporary();
	}

BOOL EditSplineMod::AnyPolysSelected() {
	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return FALSE;	
	EndOutlineMove(ip->GetTime());
	
	ip->GetModContexts(mcList,nodes);
	int splines = 0;
	BezierShape *theShape = NULL;
	EditSplineData *theESD = NULL;
	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;		
		BezierShape *shape = shapeData->TempData(this)->GetShape(ip->GetTime());
		if(!shape) continue;
		if(shape->polySel.sel.NumberSet()) {
			nodes.DisposeTemporary();
			return TRUE;
			}
		}
	nodes.DisposeTemporary();
	return FALSE;
	}

class MinSegLength {
	public:
		BOOL first;
		float length;
		MinSegLength() { first = TRUE; length = 0.0f; }
		void Include(float l);
	};

void MinSegLength::Include(float l) {
	if(first) {
		length = l;
		first = FALSE;
		return;
		}
	if(l < length)
		length = l;
	}

void EditSplineMod::SetFCLimit() {
	MinSegLength min;
	ModContextList mcList;		
	INodeTab nodes;

	if ( !ip ) return;
	TimeValue t = ip->GetTime();
	ip->GetModContexts(mcList,nodes);

	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
				
		shapeData->BeginEdit(t);
		int polys = shape->splineCount;
		for(int poly = 0; poly < shape->splineCount; ++poly) {
			BitArray map = KnotSelFromVertSel(shape->vertSel[poly]);
			if(map.NumberSet()) {
				Spline3D *spline = shape->splines[poly];
				int segs = spline->Segments();
				// Don't fillet or chamfer ends of open spline!
				if(!spline->Closed()) {
					map.Clear(0);
					map.Clear(segs);
					}
				// If any still on, proceed!
				if(map.NumberSet()) {
					for(int i = 0; i < segs; ++i) {
						if(map[i]) {
							int prev = (i + segs - 1) % segs;
							int next = i + 1;
							float lp = spline->SegmentLength(prev);
							float ln = spline->SegmentLength(i);
							min.Include(map[prev] ? lp * 0.495f : lp * 0.99f);
							min.Include(map[next] ? ln * 0.495f : ln * 0.99f);
							}
						}
					}
				}
			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	FCLimit = min.length;
//	DebugPrint("Fillet/Chamfer limit:%f\n",min.length);
	}

int EditSplineMod::GetSubobjectLevel()
	{
	return selLevel;
	}

void EditSplineMod::SetSubobjectLevel(int level)
	{
	selLevel = level;
	if(hSelectPanel)
		RefreshSelType();
	// Set up or remove the surface properties rollup if needed
	if (ip)
	{
		if (selLevel == ES_SEGMENT || selLevel == ES_SPLINE) {
			if(!hSurfPanel) {
				hSurfPanel = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_EDSPLINE_SURF),
					SplineSurfDlgProc, GetString(IDS_TH_SURFACEPROPERTIES), (LPARAM)this, rsSurf ? 0 : APPENDROLL_CLOSED);
					mtlref->hwnd = hSurfPanel;         
					noderef->hwnd = hSurfPanel;         
				}
			}
		else {
			if(hSurfPanel) {
				rsSurf = IsRollupPanelOpen (hSurfPanel);
				ip->DeleteRollupPage(hSurfPanel);
				hSurfPanel = NULL;
				}
			}
	}

	if(hSurfPanel)
		InvalidateSurfaceUI();
	if(hSelectBy)
		SendMessage(hSelectBy, MSG_SUBOBJ_MODE_CHANGE, 0, 0);
	// Setup named selection sets	
	if (ip)
		SetupNamedSelDropDown();
	}

void EditSplineMod::ActivateSubobjSel(int level, XFormModes& modes )
	{	
	ModContextList mcList;
	INodeTab nodes;
	int old = selLevel;

	if ( !ip ) return;
	ip->GetModContexts(mcList,nodes);

	switch ( level ) {
		case ES_OBJECT:
			// Not imp.
			break;

		case ES_SPLINE:
		case ES_SEGMENT:
		case ES_VERTEX:
			modes = XFormModes(moveMode,rotMode,nuscaleMode,uscaleMode,squashMode,selectMode);
			break;
		}

	if ( level != old ) {
		SetSubobjectLevel(level);
		// Modify the caches to reflect the new sel level.
		for ( int i = 0; i < mcList.Count(); i++ ) {
			EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
			if ( !shapeData ) continue;		
		
			if ( shapeData->tempData && shapeData->TempData(this)->ShapeCached(ip->GetTime()) ) {
				BezierShape *shape = shapeData->TempData(this)->GetShape(ip->GetTime());
				if(!shape) continue;
				shape->dispFlags = 0;
				shape->SetDispFlag(shapeLevelDispFlags[selLevel]);
				shape->selLevel = shapeLevel[selLevel];
				}
			}		

		NotifyDependents( FOREVER, 	PART_SUBSEL_TYPE|PART_DISPLAY, 	REFMSG_CHANGE);
		ip->PipeSelLevelChanged();
		// Update selection UI display, named sel
		SelectionChanged();
		}
	nodes.DisposeTemporary();
	}

int EditSplineMod::SubObjectIndex(HitRecord *hitRec)
	{	
	EditSplineData *shapeData = (EditSplineData*)hitRec->modContext->localData;
	if ( !shapeData ) return 0;
	if ( !ip ) return 0;
	TimeValue t = ip->GetTime();
	switch ( selLevel ) {
		case ES_VERTEX:
		case ES_SEGMENT:
			return ((ShapeHitData *)(hitRec->hitData))->index;
		case ES_SPLINE:
			return ((ShapeHitData *)(hitRec->hitData))->poly;

		default:
			return 0;
		}
	}

BOOL EditSplineMod::DependOnTopology(ModContext &mc)
	{
	EditSplineData *shapeData = (EditSplineData*)mc.localData;
	if (shapeData) {
		if (shapeData->GetFlag(ESD_HASDATA)) {
			return TRUE;
			}
		}
	return FALSE;
	}

void EditSplineMod::GetSubObjectTMs(
		SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc)
	{
	Interval valid;
	if ( mc->localData ) {
		EditSplineData *shapeData = (EditSplineData*)mc->localData;
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) return;

		switch ( selLevel ) {
			case ES_VERTEX: {
				int poly;
				Spline3D *spline;
 				Matrix3 otm = node->GetObjectTM(t,&valid);
				Matrix3 tm = node->GetNodeTM(t,&valid);
				int vbase = 1;
				for(poly = 0; poly < shape->splineCount; ++poly) {
					spline = shape->splines[poly];
					BitArray sel = shape->VertexTempSel(poly);
					for(int i = 1; i < spline->Verts(); i+=3, vbase+=3) {
						// Only display knots' axes
						if(sel[i]) {
							tm.SetTrans(otm * spline->GetVert(i));
							cb->TM(tm, vbase);
							}
						}
					}
				break;
				}
			case ES_SEGMENT: {
				int polys = shape->splineCount;
 				Matrix3 otm = node->GetObjectTM(t,&valid);
				Matrix3 tm = node->GetNodeTM(t,&valid);
				Box3 box;
				for(int poly = 0; poly < polys; ++poly) {
					BitArray sel = shape->VertexTempSel(poly);
					Spline3D *spline = shape->splines[poly];
					for ( int i = 0; i < spline->Verts(); i+=3 ) {
						if ( sel[i] )
							box += spline->GetVert(i);
						}
					}
				tm.SetTrans(otm * box.Center());
				cb->TM(tm, 0);
				break;
				}
			case ES_SPLINE: {
				int polys = shape->splineCount;
 				Matrix3 otm = node->GetObjectTM(t,&valid);
				Matrix3 tm = node->GetNodeTM(t,&valid);
				for(int poly = 0; poly < polys; ++poly) {
					if(shape->polySel[poly]) {
						Box3 box;
						Spline3D *spline = shape->splines[poly];
						for ( int i = 1; i < spline->Verts(); i+=3)
							box += spline->GetVert(i);
						tm.SetTrans(otm * box.Center());
						cb->TM(tm, poly);
						}
					}
				break;
				}
			}
		}
	}

void EditSplineMod::GetSubObjectCenters(
	SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc)
	{
	Interval valid;
	Matrix3 tm = node->GetObjectTM(t,&valid);	
	
	assert(ip);
	if ( mc->localData ) {	
		EditSplineData *shapeData = (EditSplineData*)mc->localData;		
		BezierShape *shape = shapeData->TempData(this)->GetShape(ip->GetTime());
		if(!shape) return;

		switch ( selLevel ) {
			case ES_VERTEX: {
				int polys = shape->splineCount;
				Spline3D *spline;
				Box3 box;
				int vbase = 1;
				for(int poly = 0; poly < polys; ++poly) {
					BitArray sel = shape->VertexTempSel(poly);
					spline = shape->splines[poly];
					for ( int i = 1; i < spline->Verts(); i+=3, vbase+=3 ) {
						if ( sel[i] )
							cb->Center(spline->GetVert(i) * tm, vbase);
						}
					}
				break;
				}
			case ES_SEGMENT: { 
				int polys = shape->splineCount;
				Box3 box;
				BOOL bHasSel = FALSE;
				for(int poly = 0; poly < polys; ++poly) {
					BitArray sel = shape->VertexTempSel(poly);
					Spline3D *spline = shape->splines[poly];
					for ( int i = 0; i < spline->Verts(); i++ ) {
						if ( sel[i] && ((i-1)%3) == 0 ) {
							box += spline->GetVert(i) * tm;
							bHasSel = TRUE;
							}

						}
					}
				if (bHasSel)
					cb->Center(box.Center(),0);

				break;
				}
			case ES_SPLINE: {
				int polys = shape->splineCount;
				for(int poly = 0; poly < polys; ++poly) {
					if(shape->polySel[poly]) {
						Box3 box;
						Spline3D *spline = shape->splines[poly];
						for ( int i = 1; i < spline->Verts(); i+=3)
							box += spline->GetVert(i) * tm;
						cb->Center(box.Center(), poly);
						}
					}
				break;
				}
			
			default:
				cb->Center(tm.GetTrans(),0);
				break;
			}		
		}
	}

void EditSplineMod::DeleteShapeDataTempData()
	{
	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;		
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;				
		if ( shapeData->tempData ) {
			delete shapeData->tempData;
			}
		shapeData->tempData = NULL;
		}
	nodes.DisposeTemporary();
	}


void EditSplineMod::CreateShapeDataTempData()
	{
	ModContextList mcList;
	INodeTab nodes;

	if ( !ip ) return;		
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;				
		if ( !shapeData->tempData ) {
			shapeData->tempData = new ESTempData(this,shapeData);
			}		
		}
	nodes.DisposeTemporary();
	}

//--------------------------------------------------------------
int EditSplineMod::HitTest(TimeValue t, INode* inode, int type, int crossing, 
		int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc) 
	{
	Interval valid;
	int savedLimits,res = 0;
	GraphicsWindow *gw = vpt->getGW();
	HitRegion hr;
	MakeHitRegion(hr,type, crossing,4,p);
	gw->setHitRegion(&hr);
	Matrix3 mat = inode->GetObjectTM(t);
	gw->setTransform(mat);	
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
	gw->clearHitCode();
	
	if ( mc->localData ) {		
		EditSplineData *shapeData = (EditSplineData*)mc->localData;
		BezierShape *shape = shapeData->TempData(this)->GetShape(ip->GetTime());
		if(!shape) return 0;
		SubShapeHitList hitList;
		ShapeSubHitRec *rec;
		res = shape->SubObjectHitTest( gw, gw->getMaterial(), &hr,
			flags|((splineHitOverride) ? shapeHitLevel[splineHitOverride] : shapeHitLevel[selLevel]), hitList );
	
		BOOL anyHits = FALSE;
		rec = hitList.First();
		while( rec ) {
			anyHits = TRUE;
			vpt->LogHit(inode,mc,rec->dist,123456,new ShapeHitData(rec->shape, rec->poly, rec->index));
			rec = rec->Next();
			}
		// If no hits and we're in Vertex mode with Segment-End on, try hits on segments
		if(type == HITTYPE_POINT && !anyHits && !splineHitOverride && GetSubobjectLevel() == SS_VERTEX && segmentEnd) {
			res = shape->SubObjectHitTest( gw, gw->getMaterial(), &hr,
				flags|shapeHitLevel[SS_SEGMENT], hitList );
			
			rec = hitList.First();
			while( rec ) {
				float param = shape->FindSegmentPoint(rec->poly, rec->index, gw, gw->getMaterial(), &hr, PARAM_NORMALIZED);
				int knot = rec->index;
				if(param > 0.5f)	// Inc to next seg if past midpoint
					knot++;
				knot = knot % shape->splines[rec->poly]->KnotCount();
				rec->index = knot * 3 + 1;	// Convert segment index to vertex index
				vpt->LogHit(inode,mc,rec->dist,123456,new ShapeHitData(rec->shape, rec->poly, rec->index));
				rec = rec->Next();
				}
			}
		}

	gw->setRndLimits(savedLimits);	
	return res;
	}

int EditSplineMod::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc) {	
	if (!ip || editMod != this || selLevel==ES_OBJECT) return 0;
	// Set up GW
	GraphicsWindow *gw = vpt->getGW();
	int savedLimits = gw->getRndLimits();
	gw->setRndLimits ((savedLimits & ~GW_ILLUM) | GW_ALL_EDGES);
	Matrix3 tm = inode->GetObjectTM(t) * (mc->tm?Inverse(*mc->tm):Matrix3(1));
	gw->setTransform(tm);

	if (ip->GetShowEndResult() && mc->localData) {
		tm = inode->GetObjectTM(t);
		gw->setTransform(tm);
		// We need to draw a "gizmo" version of the spline:
		EditSplineData *shapeData = (EditSplineData*)mc->localData;
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if (shape) shape->RenderGizmo(gw);	// CAL-10/3/2002: shape could be NULL (313652)
		}
	gw->setRndLimits(savedLimits);
	return 0;	
	}

void EditSplineMod::GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc) {
	if (!ip) return;
	box.Init();
	Matrix3 tm = inode->GetObjectTM(t) * (mc->tm?Inverse(*mc->tm):Matrix3(1));
	if (ip->GetShowEndResult() && mc->localData && selLevel!=ES_OBJECT) {
		// We need to draw a "gizmo" version of the mesh:
		Matrix3 tm = inode->GetObjectTM(t);
		EditSplineData *shapeData = (EditSplineData*)mc->localData;
		if (shapeData) {
			BezierShape *shape = shapeData->TempData(this)->GetShape(t);
			if (shape) {					// CAL-10/3/2002: shape could be NULL (313652)
				box = shape->GetBoundingBox();
				box = box * tm;
				}
			}
		}
	}

void EditSplineMod::ShowEndResultChanged (BOOL showEndResult) {
	if (!ip || editMod != this) return;
	NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	}

//---------------------------------------------------------------------
// UI stuff

class ESModContextEnumProc : public ModContextEnumProc {
	float f;
	public:
		ESModContextEnumProc(float f) { this->f = f; }
		BOOL proc(ModContext *mc);  // Return FALSE to stop, TRUE to continue.
	};

BOOL ESModContextEnumProc::proc(ModContext *mc) {
	EditSplineData *shapeData = (EditSplineData*)mc->localData;
	if ( shapeData )		
		shapeData->RescaleWorldUnits(f);
	return TRUE;
	}

// World scaling
void EditSplineMod::RescaleWorldUnits(float f) {
	if (TestAFlag(A_WORK1))
		return;
	SetAFlag(A_WORK1);
	
	// rescale all our references
	for (int i=0; i<NumRefs(); i++) {
		ReferenceMaker *srm = GetReference(i);
		if (srm) 
			srm->RescaleWorldUnits(f);
		}
	
	// Now rescale stuff inside our data structures
	ESModContextEnumProc proc(f);
	EnumModContexts(&proc);
	NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
	}

static BOOL oldShowEnd;

void EditSplineMod::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
	{
	this->ip = ip;
	editMod = this;
	
	CreateShapeDataTempData();

	segUIValid = FALSE;
	EndOutlineMove(ip->GetTime());
	hSelectPanel = ip->AddRollupPage (hInstance, MAKEINTRESOURCE(IDD_EDSPLINE_SELECT),
		SplineSelectDlgProc, GetString(IDS_TH_SELECTION), (LPARAM)this, rsSel ? 0 : APPENDROLL_CLOSED);
#ifndef RENDER_VER
	hSoftSelPanel = ip->AddRollupPage (hInstance, MAKEINTRESOURCE(IDD_SOFT_SELECTION),
		SplineSoftSelectDlgProc, GetString (IDS_SOFT_SELECTION), (LPARAM) this, rsSoftSel ? 0 : APPENDROLL_CLOSED); // fix this to look like the above AddRollupPage
#endif
	hOpsPanel = ip->AddRollupPage (hInstance, MAKEINTRESOURCE(IDD_EDSPLINE_OPS),
		SplineOpsDlgProc, GetString (IDS_TH_GEOMETRY), (LPARAM) this, rsOps ? 0 : APPENDROLL_CLOSED);

	if (selLevel == ES_SEGMENT || selLevel == ES_SPLINE) {
		hSurfPanel = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_EDSPLINE_SURF),
			SplineSurfDlgProc, GetString(IDS_TH_SURFACEPROPERTIES), (LPARAM)this, rsSurf ? 0 : APPENDROLL_CLOSED);
		}
	else { 
		hSurfPanel = NULL;
	}

	hSelectBy = NULL;
	
	// Create sub object editing modes.
	moveMode        = new MoveModBoxCMode(this,ip);
	rotMode         = new RotateModBoxCMode(this,ip);
	uscaleMode      = new UScaleModBoxCMode(this,ip);
	nuscaleMode     = new NUScaleModBoxCMode(this,ip);
	squashMode      = new SquashModBoxCMode(this,ip);
	selectMode      = new SelectModBoxCMode(this,ip);
	outlineMode     = new OutlineCMode(this,ip);
	filletMode      = new FilletCMode(this,ip);
	chamferMode     = new ESChamferCMode(this,ip);
	segBreakMode    = new SegBreakCMode(this,ip);
	segRefineMode   = new SegRefineCMode(this,ip);
	crossInsertMode = new CrossInsertCMode(this,ip);
	vertConnectMode = new VertConnectCMode(this,ip);
	vertInsertMode  = new VertInsertCMode(this,ip);
	booleanMode     = new BooleanCMode(this,ip);
	trimMode        = new TrimCMode(this,ip);
	extendMode      = new ExtendCMode(this,ip);
	createLineMode  = new CreateLineCMode(this,ip);
	crossSectionMode = new CrossSectionCMode(this,ip);
//watje
	refineConnectMode   = new RefineConnectCMode(this,ip);
	bindMode   = new BindCMode(this,ip);
	// CAL-03/03/03: copy/paste tangent. (FID #827)
	copyTangentMode = new CopyTangentCMode(this,ip);
	pasteTangentMode = new PasteTangentCMode(this,ip);

	// Create reference for MultiMtl name support         
	noderef = new SingleRefMakerSplineMNode;         //set ref to node
	INode* objNode = GetNode(this);
	if (objNode) {
		noderef->es = this;
		noderef->SetRef(objNode);                 
	}
	mtlref = new SingleRefMakerSplineMMtl;       //set ref for mtl
	mtlref->es = this;
	if (objNode) {
		Mtl* nodeMtl = objNode->GetMtl();
		mtlref->SetRef(nodeMtl);                        
	}
	// Add our sub object type
	// TSTR type1( GetString(IDS_TH_VERTEX) );
	// TSTR type2( GetString(IDS_TH_SEGMENT) );
	// TSTR type3( GetString(IDS_TH_SPLINE) );
	// const TCHAR *ptype[] = { type1, type2, type3 };
	// This call is obsolete. Please see BaseObject::NumSubObjTypes() and BaseObject::GetSubObjType()
	// ip->RegisterSubObjectTypes( ptype, 3 );

	// Restore the selection level.
	ip->SetSubObjectLevel(selLevel);
	
	// Setup named selection sets	
	SetupNamedSelDropDown();

	// Update selection UI display
	SelectionChanged();

	// Set show end result.
	oldShowEnd = ip->GetShowEndResult();
	ip->SetShowEndResult (GetFlag (ES_DISP_RESULT));

	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);	
	}
		
void EditSplineMod::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
	{
	EndOutlineMove(ip->GetTime());

	if (hSelectPanel) {
		rsSel = IsRollupPanelOpen (hSelectPanel);
		ip->DeleteRollupPage(hSelectPanel);
		hSelectPanel = NULL;
		}
	if (hOpsPanel) {
		rsOps = IsRollupPanelOpen (hOpsPanel);
		ip->DeleteRollupPage(hOpsPanel);
		hOpsPanel = NULL;
		}
	if (hSurfPanel) {
		rsSurf = IsRollupPanelOpen (hSurfPanel);
		ip->DeleteRollupPage(hSurfPanel);
		hSurfPanel = NULL;
		}
	if(hSelectBy) {
		DestroyWindow(hSelectBy);
		hSelectBy = NULL;
		}
#ifndef RENDER_VER
	if(hSoftSelPanel) {
		rsSoftSel = IsRollupPanelOpen (hSoftSelPanel);
		ip->DeleteRollupPage(hSoftSelPanel);
		hSoftSelPanel = NULL;
		}
#endif

	CancelEditSplineModes(ip);

	TimeValue t = ip->GetTime();
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);
	
	if (noderef) delete noderef;        
	noderef = NULL;                         
	if (mtlref) delete mtlref;             
	mtlref = NULL;                      

	DeleteShapeDataTempData();
	this->ip = NULL;
	editMod = NULL;
	
	ip->DeleteMode(moveMode);
	ip->DeleteMode(rotMode);
	ip->DeleteMode(uscaleMode);
	ip->DeleteMode(nuscaleMode);
	ip->DeleteMode(squashMode);
	ip->DeleteMode(selectMode);
	ip->DeleteMode(outlineMode);
	ip->DeleteMode(filletMode);
	ip->DeleteMode(chamferMode);
	ip->DeleteMode(segBreakMode);
	ip->DeleteMode(segRefineMode);
	ip->DeleteMode(crossInsertMode);
	ip->DeleteMode(vertConnectMode);
	ip->DeleteMode(vertInsertMode);
	ip->DeleteMode(booleanMode);
	ip->DeleteMode(trimMode);
	ip->DeleteMode(extendMode);
	ip->DeleteMode(createLineMode);
	ip->DeleteMode(crossSectionMode);

//	watje
	ip->DeleteMode(refineConnectMode);
	ip->DeleteMode(bindMode);
	// CAL-03/03/03: copy/paste tangent. (FID #827)
	ip->DeleteMode(copyTangentMode);
	ip->DeleteMode(pasteTangentMode);


	if ( moveMode ) delete moveMode;
	moveMode = NULL;
	if ( rotMode ) delete rotMode;
	rotMode = NULL;
	if ( uscaleMode ) delete uscaleMode;
	uscaleMode = NULL;
	if ( nuscaleMode ) delete nuscaleMode;
	nuscaleMode = NULL;
	if ( squashMode ) delete squashMode;
	squashMode = NULL;
	if ( selectMode ) delete selectMode;
	selectMode = NULL;
	if ( outlineMode ) delete outlineMode;
	outlineMode = NULL;
	if ( filletMode ) delete filletMode;
	filletMode = NULL;
	if ( chamferMode ) delete chamferMode;
	chamferMode = NULL;
	if ( segBreakMode ) delete segBreakMode;
	segBreakMode = NULL;
	if ( segRefineMode ) delete segRefineMode;
	segRefineMode = NULL;
	if ( crossInsertMode ) delete crossInsertMode;
	crossInsertMode = NULL;
	if ( vertConnectMode ) delete vertConnectMode;
	vertConnectMode = NULL;
	if ( vertInsertMode ) delete vertInsertMode;
	vertInsertMode = NULL;
	if ( booleanMode ) delete booleanMode;
	booleanMode = NULL;
	if ( trimMode ) delete trimMode;
	trimMode = NULL;
	if ( extendMode ) delete extendMode;
	extendMode = NULL;
	if ( createLineMode ) delete createLineMode;
	createLineMode = NULL;
	if ( crossSectionMode ) delete crossSectionMode;
	crossSectionMode = NULL;

	//watje
	if ( refineConnectMode ) delete refineConnectMode;
	refineConnectMode = NULL;
	if ( bindMode ) delete bindMode;
	bindMode = NULL;
	// CAL-03/03/03: copy/paste tangent. (FID #827)
	if ( copyTangentMode ) delete copyTangentMode;
	copyTangentMode = NULL;
	if ( pasteTangentMode ) delete pasteTangentMode;
	pasteTangentMode = NULL;

	// Reset show end result
	SetFlag (ES_DISP_RESULT, ip->GetShowEndResult());
	ip->SetShowEndResult(oldShowEnd);
	}

// CAL-02/20/03: Add Cross Section. (FID #827)
class SegmentType {
public:
	int a, b;
	bool used;
};

int EditSplineMod::GetPointIndex(const Tab<Point3> &vertList, const Point3 &point) const {
	for (int i = 0; i < vertList.Count(); i++)
		if (vertList[i] == point)
			return i;
	return 0;
};

void EditSplineMod::DoCrossSection(EditSplineData *shapeData, Tab<int> &splineIndices) {
	if ( !ip || shapeData == NULL) return;

	BezierShape *shape = shapeData->TempData(this)->GetShape(ip->GetTime());
	if (shape == NULL) return;

	int polys = splineIndices.Count();
	if (polys <= 1) return;

	theHold.Restore();
	if ( theHold.Holding() && !TestAFlag(A_HELD) ) {
		theHold.Put(new ShapeRestore(shapeData,this,shape));
		SetAFlag(A_HELD);
	}

	int i, j;
	Tab<Spline3D*> selSplines;
	bool sameCount = true;
	int closedA, closedB;
	int countA, countB;
	int kType = newKnotType;
	Point3 p;
	
	for (i = 0; i < polys; i++) {
		Spline3D *spline = shape->splines[splineIndices[i]];
		selSplines.Append(1, &spline, 1);
	}

	countA = selSplines[0]->KnotCount();
	closedA = selSplines[0]->Closed();
	for (int poly = 1; poly < polys; poly++) {
		if (countA != selSplines[poly]->KnotCount()) {
			sameCount = false;
			break;
		}
		if (closedA != selSplines[poly]->Closed()) {
			sameCount = false;
			break;
		}
	}

	if (sameCount) {
		for (i = 0; i < selSplines[0]->KnotCount(); i++) {
			Spline3D *addSpline = shape->NewSpline();
			// Add verts
			for (poly = 0; poly < polys; poly++) {
				Spline3D *spline = selSplines[poly];
				// Now add all the necessary points
				// Next points or knots are added to the spline by calling AddKnot().
				// This allows you to add different types of knots and line segments.
				p = spline->GetKnotPoint(i);
				addSpline->AddKnot(SplineKnot(KTYPE_AUTO, LTYPE_CURVE, spline->GetKnotPoint(i), p, p));
			}
			addSpline->ComputeBezPoints();
			for (j = 0; j < addSpline->KnotCount(); j++)
				addSpline->SetKnotType(j, kType);
			addSpline->ComputeBezPoints();
			addSpline->SetOpen();
		}
	} else {
		Tab<Point3> vertList;
		Tab<SegmentType> segmentList;
		SegmentType sType;

		// Build vert list
		for (poly = 0; poly < polys; poly++) {
			Spline3D *spline = selSplines[poly];
			for (i = 0; i < spline->KnotCount(); i++) {
				p = spline->GetKnotPoint(i);
				vertList.Append(1, &p, 1);
			}
		}

		// Build segment list
		sType.used = false;
		for (poly = 0; poly < polys-1; poly++) {
			Spline3D *splineA = selSplines[poly];
			Spline3D *splineB = selSplines[poly+1];
			
			countA = selSplines[poly]->KnotCount();
			countB = selSplines[poly+1]->KnotCount();
			closedA = selSplines[poly]->Closed();
			closedB = selSplines[poly+1]->Closed();

			if (countA == countB) {
				if (closedA == closedB) {
					for (i = 0; i < countA; i++) {
						sType.a = GetPointIndex(vertList, splineA->GetKnotPoint(i));
						sType.b = GetPointIndex(vertList, splineB->GetKnotPoint(i));
						segmentList.Append(1, &sType, 1);
					}
				} else if (closedA) {
					for (i = 0; i < countA; i++) {
						sType.a = GetPointIndex(vertList, splineA->GetKnotPoint(i));
						sType.b = GetPointIndex(vertList, splineB->GetKnotPoint(i));
						segmentList.Append(1, &sType, 1);
					}
					sType.a = GetPointIndex(vertList, splineA->GetKnotPoint(0));
					sType.b = GetPointIndex(vertList, splineB->GetKnotPoint(countB-1));
					segmentList.Append(1, &sType, 1);
				} else {
					for (i = 0; i < countA; i++) {
						sType.a = GetPointIndex(vertList, splineA->GetKnotPoint(i));
						sType.b = GetPointIndex(vertList, splineB->GetKnotPoint(i));
						segmentList.Append(1, &sType, 1);
					}
					sType.a = GetPointIndex(vertList, splineA->GetKnotPoint(countA-1));
					sType.b = GetPointIndex(vertList, splineB->GetKnotPoint(0));
					segmentList.Append(1, &sType, 1);
				}
			} else {
				float per;
				Tab<float> perListA;
				Tab<float> perListB;
				float dist1, dist2;

				float lengthA = splineA->SplineLength();
				float lengthB = splineB->SplineLength();
				float lengthC = 0.0f;
				
				for (i = 0; i < countA; i++) {
					per = lengthC / lengthA;
					if (per > 1.0f) per = 1.0f;
					perListA.Append(1, &per, 1);
					for (j = 0, per = 0.0f; j < 10; j++, per += 0.1f)	// make 10 sub-sampling
						lengthC += Length(splineA->InterpBezier3D(i, per+0.1f) -
										  splineA->InterpBezier3D(i, per));
				}

				lengthC = 0.0f;
				for (i = 0; i < countB; i++) {
					per = lengthC / lengthB;
					if (per > 1.0f) per = 1.0f;
					perListB.Append(1, &per, 1);
					for (j = 0, per = 0.0f; j < 10; j++, per += 0.1f)	// make 10 sub-sampling
						lengthC += Length(splineB->InterpBezier3D(i, per+0.1f) -
										  splineB->InterpBezier3D(i, per));
				}

				if (closedA != closedB) {
					per = 1.0f;
					if (closedA) {
						perListA.Append(1, &per, 1);
						countA++;
					} else {
						perListB.Append(1, &per, 1);
						countB++;
					}
				}

				if (countA < countB) {
					for (i = 0, j = 0; i < countB; i++) {
						if ((closedA != closedB) && (closedA) && (j == (countA-1)))
							sType.a = GetPointIndex(vertList, splineA->GetKnotPoint(0));
						else
							sType.a = GetPointIndex(vertList, splineA->GetKnotPoint(j));
						if ((closedA != closedB) && (closedB) && (i == (countB-1)))
							sType.b = GetPointIndex(vertList, splineB->GetKnotPoint(0));
						else
							sType.b = GetPointIndex(vertList, splineB->GetKnotPoint(i));
						segmentList.Append(1, &sType, 1);
						// check if need to advance to the next vertex
						if ((j < (countA-1)) && (i < (countB-1))) {
							dist1 = (float)fabs(perListB[i+1] - perListA[j]);
							dist2 = (float)fabs(perListB[i+1] - perListA[j+1]);
							if (dist2 < dist1) j++;
						}
					}
				} else {
					for (i = 0, j = 0; i < countA; i++) {
						if ((closedA != closedB) && (closedA) && (i == (countA-1)))
							sType.a = GetPointIndex(vertList, splineA->GetKnotPoint(0));
						else
							sType.a = GetPointIndex(vertList, splineA->GetKnotPoint(i));
						if ((closedA != closedB) && (closedB) && (j == (countB-1)))
							sType.b = GetPointIndex(vertList, splineB->GetKnotPoint(0));
						else
							sType.b = GetPointIndex(vertList, splineB->GetKnotPoint(j));
						segmentList.Append(1, &sType, 1);
						// check if need to advance to the next vertex
						if ((j < (countB-1)) && (i < (countA-1))) {
							dist1 = (float)fabs(perListA[i+1] - perListB[j]);
							dist2 = (float)fabs(perListA[i+1] - perListB[j+1]);
							if (dist2 < dist1) j++;
						}
					}
				}
			}
		}

		// Build splines
		for (i = 0; i < segmentList.Count(); i++) {
			if (!segmentList[i].used) {
				// Build spline
				Spline3D *addSpline = shape->NewSpline();
				// Add verts - Now add all the necessary points
				p = vertList[segmentList[i].a];
				addSpline->AddKnot(SplineKnot(KTYPE_AUTO, LTYPE_CURVE, vertList[segmentList[i].a], p, p));
				p = vertList[segmentList[i].b];
				addSpline->AddKnot(SplineKnot(KTYPE_AUTO, LTYPE_CURVE, vertList[segmentList[i].b], p, p));
				segmentList[i].used = true;

				for (j = i;;) {
					sType = segmentList[j];
					for (j = 0; j < segmentList.Count(); j++)
						if ((!segmentList[j].used) && (sType.b == segmentList[j].a))
							 break;
					if (j >= segmentList.Count()) break;
					// Next points or knots are added to the spline by calling AddKnot().
					// This allows you to add different types of knots and line segments.
					p = vertList[segmentList[j].b];
					addSpline->AddKnot(SplineKnot(KTYPE_AUTO, LTYPE_CURVE, vertList[segmentList[j].b], p, p));
					segmentList[j].used = true;
				}
	
				addSpline->ComputeBezPoints();
				for (j = 0; j < addSpline->KnotCount(); j++)
					addSpline->SetKnotType(j, kType);
				addSpline->ComputeBezPoints();
				addSpline->SetOpen();
			}
		}
	}

	shape->UpdateSels();	// Make sure it readies the selection set info
	shape->InvalidateGeomCache();

	// TODO: copy the controllers to new knots
	// TODO: update selections (current selection and named selections)

	SelectionChanged();
	// ResolveTopoChanges();
	// FixupControllerTopo(this);

	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	// if(cont.Count())
	//	NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
}

// Vertex Break modifier method
void EditSplineMod::DoVertBreak() {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	theHold.Begin();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		int altered = 0;
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);
		shapeData->PreUpdateChanges(shape);

		int polys = shape->splineCount;
		for(int poly = 0; poly < polys; ++poly) {
			Spline3D *spline = shape->splines[poly];
			// If any bits are set in the selection set, let's DO IT!!
			if(shape->vertSel[poly].NumberSet()) {
				altered = holdNeeded = 1;
				if ( theHold.Holding() ) {
					theHold.Put(new ShapeRestore(shapeData,this,shape));
					}
				// Call the spline break function
				BreakSplineAtSelVerts(shape, poly);
				}
			}

		if(altered) {
			shapeData->UpdateChanges(shape);
			shapeData->TempData(this)->Invalidate(PART_TOPO);
			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	
	if(holdNeeded)
		theHold.Accept(GetString(IDS_TH_VERTBREAK));
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOVALIDVERTSSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

static BOOL NeedsWeld(Spline3D *spline, BitArray sel, float thresh) {
	int knots = spline->KnotCount();
	for(int i = 0; i < knots; ++i) {
		if(sel[i*3+1]) {
			int next = (i + 1) % knots;
			if(sel[next*3+1] && Length(spline->GetKnotPoint(i) - spline->GetKnotPoint(next)) <= thresh)
				return TRUE;
			}
		}
	return FALSE;
	}


//watje
class WeldList
{
public:
	int splineID, knotID;
	Point3 p;
	int used;
};


// Vertex Weld modifier method
void EditSplineMod::DoVertWeld() {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	BOOL holdNeeded = FALSE;
	BOOL hadSelected = FALSE;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	int oldVerts = 0;
	int newVerts = 0;

	theHold.Begin();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		BOOL altered = FALSE;
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);
		shapeData->PreUpdateChanges(shape);

		// Add the original number of knots to the oldVerts field
		for(int poly = 0; poly < shape->splineCount; ++poly)
			oldVerts += shape->splines[poly]->KnotCount();

//watje now move any verts thta are within the weld tolerance onto each other
//build list 
/*
		Tab<WeldList> weldList;
		for(poly = 0; poly < shape->splineCount; ++poly)
			{
			BitArray pSel = shape->VertexTempSel(poly);
			for (int i = 0; i < pSel.GetSize()/3; i++)
				{
				if(pSel[(i*3)+1]) 
					{
					WeldList w;
					w.used = -1;
					w.splineID =poly;
					w.knotID = i;
					w.p = shape->splines[poly]->GetKnotPoint(i);
					weldList.Append(1,&w,1);
					}
				}
			}
		int ct = 0;
		float threshSquared = weldThreshold * weldThreshold;
		for (int wc = 0; wc < weldList.Count(); wc++)
			{
			Point3 p = weldList[wc].p;
			BOOL found = FALSE;
			for (int wc2 = (wc+1); wc2 < weldList.Count(); wc2++)
				{
				Point3 t;
				t = weldList[wc2].p;
				if ((weldList[wc2].used == -1) && (LengthSquared(p-t) <= threshSquared))
					{
					found = TRUE;
					weldList[wc].used = ct;
					weldList[wc2].used = ct;
					}
				}
			if (found) ct++;
			}
		if (ct >0)
			{
			altered = holdNeeded = TRUE;
			for (int i = 0; i < ct; i++)
				{
				Point3 avgPoint(0.0f,0.0f,0.0f);
				int pointCount = 0;
				for (int j = 0; j < weldList.Count(); j++)
					{
					if (i == weldList[j].used)
						{
						avgPoint += weldList[j].p;
						pointCount++;
						}
					}
				avgPoint = avgPoint/(float)pointCount;
				for (j = 0; j < weldList.Count(); j++)
					{
					if (i == weldList[j].used)
						{
						weldList[j].p = avgPoint;
						}
					}

				}
//now put them back in the knot list
			for (i = 0; i < weldList.Count(); i++)
				{
				int polyID = weldList[i].splineID;
				int knotID = weldList[i].knotID;

				shape->splines[polyID]->SetKnotPoint(knotID,weldList[i].p);
				shape->splines[polyID]->ComputeBezPoints();

				}
			shape->InvalidateGeomCache();

			}
*/
		// Poly-to-poly weld loops back here because a weld between polys creates one
		// polygon which may need its two new endpoints welded
		changed:
		int polys = shape->splineCount;
		for(poly = polys - 1; poly >= 0; --poly) {
			Spline3D *spline = shape->splines[poly];
			// If any bits are set in the selection set, let's DO IT!!
			if(shape->vertSel[poly].NumberSet()) {
				hadSelected = TRUE;
				if(NeedsWeld(spline, shape->vertSel[poly], weldThreshold)) {
					altered = holdNeeded = TRUE;
					if ( theHold.Holding() ) {
						theHold.Put(new ShapeRestore(shapeData,this,shape));
						}
					// Call the weld function
					WeldSplineAtSelVerts(shape, poly, weldThreshold);
					}
				}
			}

		// Now check for welds with other polys (endpoints only)
		polys = shape->SplineCount();
		for(int poly1 = 0; poly1 < polys; ++poly1) {
			Spline3D *spline1 = shape->splines[poly1];
			if(!spline1->Closed()) {
				int knots1 = spline1->KnotCount();
				int lastKnot1 = knots1-1;
				int lastSel1 = lastKnot1 * 3 + 1;
				BitArray pSel = shape->VertexTempSel(poly1);
				if(pSel[1] || pSel[lastSel1]) {
					Point3 p1 = spline1->GetKnotPoint(0);
					Point3 p2 = spline1->GetKnotPoint(lastKnot1);
					for(int poly2 = 0; poly2 < polys; ++poly2) {
						Spline3D *spline2 = shape->splines[poly2];
						if(poly1 != poly2 && !spline2->Closed()) {
							int knots2 = spline2->KnotCount();
							int lastKnot2 = knots2-1;
							int lastSel2 = lastKnot2 * 3 + 1;
							Point3 p3 = spline2->GetKnotPoint(0);
							Point3 p4 = spline2->GetKnotPoint(lastKnot2);
							BitArray pSel2 = shape->VertexTempSel(poly2);
							int vert1, vert2;
							if(pSel[1]) {
								if(pSel2[1] && Length(p3 - p1) <= weldThreshold) {
									vert1 = 0;
									vert2 = 0;

									attach_it:
									// Start a restore object...
									if ( theHold.Holding() )
										theHold.Put(new ShapeRestore(shapeData,this,shape));
									// Do the attach
									DoPolyEndAttach(shape, poly1, vert1, poly2, vert2);
									shapeData->TempData(this)->Invalidate(PART_TOPO|PART_GEOM);
									altered = holdNeeded = TRUE;
									goto changed;
									}
								else
								if(pSel2[lastSel2] && Length(p4 - p1) <= weldThreshold) {
									vert1 = 0;
									vert2 = knots2-1;
									goto attach_it;
									}
								}
							if(pSel[lastSel1]) {
								if(pSel2[1] && Length(p3 - p2) <= weldThreshold) {
									vert1 = knots1-1;
									vert2 = 0;
									goto attach_it;
									}
								else
								if(pSel2[lastSel2] && Length(p4 - p2) <= weldThreshold) {
									vert1 = knots1-1;
									vert2 = knots2-1;
									goto attach_it;
									}
								}
							}
						}
					}
				}
			}

		if(altered) {
			shapeData->UpdateChanges(shape);
			shapeData->TempData(this)->Invalidate(PART_TOPO);
			}

		// Add the original number of knots to the oldVerts field
		for(poly = 0; poly < shape->splineCount; ++poly)
			newVerts += shape->splines[poly]->KnotCount();

		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	
	if(holdNeeded) {
		theHold.Accept(GetString(IDS_TH_VERTWELD));
		TSTR s1;
		int welded = oldVerts - newVerts;
		s1.printf(GetString(IDS_TH_VERTWELDRESULT), welded, oldVerts);
		ip->DisplayTempPrompt(s1,PROMPT_TIME);
		}
	else {
		TSTR s1;
		if(!hadSelected)
			s1 = TSTR( GetString(IDS_TH_NOVALIDVERTSSEL) );
		else
			s1 = TSTR( GetString(IDS_TH_NOWELDPERFORMED) );
		ip->DisplayTempPrompt(s1,PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

// Make First modifier method
void EditSplineMod::DoMakeFirst() {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	BOOL holdNeeded = FALSE;
	BOOL hadSelected = FALSE;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	theHold.Begin();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		BOOL altered = FALSE;
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);
		shapeData->PreUpdateChanges(shape);

		int polys = shape->splineCount;
		for(int poly = polys - 1; poly >= 0; --poly) {
			Spline3D *spline = shape->splines[poly];
			int knots = spline->KnotCount();
			// If one bits is set in the selection set, let's DO IT!!
			BitArray &vsel = shape->vertSel[poly];
			if(vsel.NumberSet() == 1) {
				if(spline->Closed()) {
					for(int j = 0; j < knots; ++j) {
						if(vsel[j*3+1])
							break;
						}
					if ( theHold.Holding() ) {
						theHold.Put(new ShapeRestore(shapeData,this,shape));
						}
					shape->MakeFirst(poly, j);
					altered = holdNeeded = TRUE;
					}
				else
				if(vsel[(knots-1)*3+1]) {	// Last vertex?
					if ( theHold.Holding() ) {
						theHold.Put(new ShapeRestore(shapeData,this,shape));
						}
					shape->Reverse(poly);
					altered = holdNeeded = TRUE;
					}
				}
			}

		if(altered) {
			shapeData->UpdateChanges(shape);
			shapeData->TempData(this)->Invalidate(PART_TOPO);
			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	
	if(holdNeeded)
		theHold.Accept(GetString(IDS_TH_MAKEFIRST));
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOVALIDVERTSSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

// Vertex Delete modifier method
void EditSplineMod::DoVertDelete() {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	theHold.Begin();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		int altered = 0;
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);
		shapeData->PreUpdateChanges(shape);

		int polys = shape->splineCount;
		for(int poly = polys - 1; poly >= 0; --poly) {
			Spline3D *spline = shape->splines[poly];
			// If any bits are set in the selection set, let's DO IT!!
			if(shape->vertSel[poly].NumberSet()) {
				altered = holdNeeded = 1;
				if ( theHold.Holding() )
					theHold.Put(new ShapeRestore(shapeData,this,shape));
				// Call the vertex delete function
				DeleteSelVerts(shape, poly);
				}
			}

		if(altered) {
			shapeData->UpdateChanges(shape);
			shapeData->TempData(this)->Invalidate(PART_TOPO);
			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	
	if(holdNeeded)
		theHold.Accept(GetString(IDS_TH_VERTDELETE));
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOVERTSSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}


// Segment Delete modifier method
void EditSplineMod::DoSegDelete() {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	theHold.Begin();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		int altered = 0;
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);
		shapeData->PreUpdateChanges(shape);

		int polys = shape->splineCount;
		for(int poly = polys - 1; poly >= 0; --poly) {
			Spline3D *spline = shape->splines[poly];
			// If any bits are set in the selection set, let's DO IT!!
			if(shape->segSel[poly].NumberSet()) {
				altered = holdNeeded = 1;
				if ( theHold.Holding() ) {
					theHold.Put(new ShapeRestore(shapeData,this,shape));
					}
				// Call the segment delete function
				DeleteSelSegs(shape, poly);
				}
			}

		if(altered) {
			shapeData->UpdateChanges(shape);
			shapeData->TempData(this)->Invalidate(PART_TOPO);
			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	
	if(holdNeeded)
		theHold.Accept(GetString(IDS_TH_SEGDELETE));
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOSEGSSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

// Segment Detach modifier method
void EditSplineMod::DoSegDetach(int sameShape, int copy, int reorient) {
	int holdNeeded = 0;
	int dialoged = 0;
	TSTR newName(GetString(IDS_TH_SHAPE));
	int retain = 1;
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	if(nodes.Count() > 1 && sameShape) {
		ip->DisplayTempPrompt(GetString(IDS_TH_MULTIPLE_NODES),PROMPT_TIME);
		nodes.DisposeTemporary();
		return;
		}
	
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	theHold.Begin();

	// Create a spline shape object
	SplineShape *splShape = NULL;
	BezierShape *outShape = NULL;
	
	if(!sameShape) {
		splShape = new SplineShape;
		outShape = &splShape->shape;
		}	

	int multipleObjects = (mcList.Count() > 1) ? 1 : 0;
	for ( int i = 0; i < mcList.Count(); i++ ) {
		int altered = 0;
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);
		shapeData->PreUpdateChanges(shape);
		if(!outShape)
			outShape = shape;
		
		int detachType;
		if(sameShape) {
			if(copy)
				detachType = SDT_COPY_SAME;
			else
				detachType = SDT_DETACH_SAME;
			}
		else {
			if(copy)
				detachType = SDT_COPY;
			else
				detachType = SDT_DETACH;
			}

		int polys = shape->splineCount;
//watje 5-26-99
		outShape->bindList = shape->bindList;

		for(int poly = polys - 1; poly >= 0; --poly) {
			Spline3D *spline = shape->splines[poly];
			// If any bits are set in the selection set, let's DO IT!!
			int segsSelected = shape->segSel[poly].NumberSet();
			if(segsSelected) {
				if(!dialoged && !sameShape) {
					dialoged = 1;
					if(!GetDetachOptions(ip, newName))
						goto bail_out;
					}
				altered = holdNeeded = 1;
				if ( theHold.Holding() ) {
					theHold.Put(new ShapeRestore(shapeData,this,shape));
					}
				HandleSegDetach(shape, outShape, detachType, poly);
				}
			}

//watje 5-26-99
//		shape->UpdateBindList();

		bail_out:
		if(altered) {
			shapeData->UpdateChanges(shape);
			shapeData->TempData(this)->Invalidate(PART_TOPO);
			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	
	if(holdNeeded) {
		if(!sameShape) {
			ObjectState os =
				nodes[0]->GetObjectRef()->Eval(t);
			GeomObject *object = (GeomObject *)os.obj;
			if(object->IsShapeObject()) {
				// Set base parameters in new shape
				splShape->CopyBaseData(*((ShapeObject *)object));
				// And optimize/adaptive
				if(object->IsSubClassOf(splineShapeClassID)) {
					SplineShape *ss = (SplineShape *)object;
					splShape->shape.optimize = ss->shape.optimize;
					splShape->steps = ss->steps;
					splShape->shape.steps = ss->shape.steps;
					}
				}
//watje 5-26-99
			splShape->shape.UpdateBindList(TRUE);

			INode *newNode = ip->CreateObjectNode(splShape);
			newNode->CopyProperties(nodes[0]);
			newNode->SetName(newName.data());
			if(!multipleObjects) {	// Single input object?
				if(!reorient) {
					Matrix3 tm = nodes[0]->GetObjectTM(t);
					newNode->SetNodeTM(t, tm);	// Use this object's TM.
					}
				}
			else {
				if(!reorient) {
					Matrix3 matrix;
					matrix.IdentityMatrix();
					newNode->SetNodeTM(t, matrix);	// Use identity TM
					}
				}
			newNode->FlagForeground(t);		// WORKAROUND!
			}
//		outShape->UpdateSels();	// Make sure it readies the selection set info
//watje 5-26-99
//		outShape->UpdateBindList();
		outShape->InvalidateGeomCache();
		theHold.Accept(GetString(IDS_TH_SEGDETACH));
		}
	else {
		if(splShape)
			delete splShape;	// Didn't need it after all!
		if(!dialoged)
			ip->DisplayTempPrompt(GetString(IDS_TH_NOSEGSSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	SelectionChanged();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(t, REDRAW_NORMAL);
	}

// Close all selected polygons (that aren't already closed)
void EditSplineMod::DoPolyClose() {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	theHold.Begin();
	MaybeSelectSingleSpline();

	for ( int i = 0; i < mcList.Count(); i++ ) {
		int altered = 0;
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);
		shapeData->PreUpdateChanges(shape);

		int polys = shape->splineCount;
		for(int poly = 0; poly < polys; ++poly) {
			if(shape->polySel[poly]) {
				Spline3D *spline = shape->splines[poly];
				if(!spline->Closed() && spline->KnotCount()>2) {
					altered = holdNeeded = 1;
					// Save the unmodified verts.
					if ( theHold.Holding() ) {
						theHold.Put(new ShapeRestore(shapeData,this,shape));
						}
					spline->SetClosed();
					shape->vertSel[poly].SetSize(spline->Verts(),1);
					shape->segSel[poly].SetSize(spline->Segments(),1);
					shape->segSel[poly].Clear(spline->Segments()-1);
					spline->ComputeBezPoints();
//					shape->UpdateBindList();
					shape->InvalidateGeomCache();
					}
				}
			}

		if(altered) {
			shapeData->UpdateChanges(shape);
			shapeData->TempData(this)->Invalidate(PART_TOPO);
			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	
	if(holdNeeded)
		theHold.Accept(GetString(IDS_TH_CLOSESPLINE));
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOVALIDSPLINESSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	SelectionChanged();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

// Detach all selected polygons
void EditSplineMod::DoPolyDetach(int copy, int reorient) {
	int dialoged = 0;
	TSTR newName(GetString(IDS_TH_SHAPE));
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	theHold.Begin();
	MaybeSelectSingleSpline();

	// Create a spline shape object
	SplineShape *splShape = new SplineShape;

	BOOL setSegs = FALSE;
	 
	int multipleObjects = (mcList.Count() > 1) ? 1 : 0;
	for ( int i = 0; i < mcList.Count(); i++ ) {
		int altered = 0;
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);
		shapeData->PreUpdateChanges(shape);

		int polys = shape->splineCount;
		for(int poly = polys-1; poly >= 0; --poly) {
			if(shape->polySel[poly]) {
				Spline3D *spline = shape->splines[poly];
				if(!dialoged) {
					dialoged = 1;
					if(!GetDetachOptions(ip, newName))
						goto bail_out;
					}
				altered = holdNeeded = 1;
				// Save the unmodified verts.
				if ( theHold.Holding() ) {
					theHold.Put(new ShapeRestore(shapeData,this,shape));
					}
				// If haven't set destination segments, do it now
				if(!setSegs) {
					setSegs = TRUE;
					splShape->shape.steps = shape->steps;
					splShape->shape.optimize = shape->optimize;
					}
				// Copy selected polys to a new output object
				Spline3D *newSpline = splShape->shape.NewSpline();
				*newSpline = *spline;
				if(multipleObjects && !reorient)
					newSpline->Transform(&nodes[i]->GetObjectTM(t));
				if(!copy)
					shape->DeleteSpline(poly);
				}
			}

		bail_out:
//watje 5-26-99
		splShape->shape.bindList = shape->bindList;
//		shape->UpdateBindList();

		if(altered) {
			shapeData->UpdateChanges(shape);
			shapeData->TempData(this)->Invalidate(PART_TOPO);
			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	
	if(holdNeeded) {
		ObjectState os =
			nodes[0]->GetObjectRef()->Eval(t);
		GeomObject *object = (GeomObject *)os.obj;
		if(object->IsShapeObject()) {
			// Set base parameters in new shape
			splShape->CopyBaseData(*((ShapeObject *)object));
			// And optimize/adaptive
			if(object->IsSubClassOf(splineShapeClassID)) {
				SplineShape *ss = (SplineShape *)object;
				splShape->shape.optimize = ss->shape.optimize;
				splShape->steps = ss->steps;
				splShape->shape.steps = ss->shape.steps;
				}
			}
		INode *newNode = ip->CreateObjectNode(splShape);
		newNode->CopyProperties(nodes[0]);
		newNode->SetName(newName.data());
//watje 5-26-99
		splShape->shape.UpdateBindList(TRUE);

		splShape->shape.UpdateSels();	// Make sure it readies the selection set info
		splShape->shape.InvalidateGeomCache();
		if(!multipleObjects) {	// Single input object?
			if(!reorient) {
				Matrix3 tm = nodes[0]->GetObjectTM(t);
				newNode->SetNodeTM(t, tm);	// Use this object's TM.
				}
			}
		else {
			if(!reorient) {
				Matrix3 matrix;
				matrix.IdentityMatrix();
				newNode->SetNodeTM(t, matrix);	// Use identity TM
				}
			}
		newNode->FlagForeground(t);		// WORKAROUND!
		theHold.Accept(GetString(IDS_TH_DETACHSPLINE));
		}
	else {
		delete splShape;	// Didn't need it after all!
		if(!dialoged)
			ip->DisplayTempPrompt(GetString(IDS_TH_NOSPLINESSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	SelectionChanged();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(t,REDRAW_NORMAL);
	}

// Mirror all selected polygons
void EditSplineMod::DoPolyMirror(int type, int copy, BOOL aboutPivot) {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	theHold.Begin();
	MaybeSelectSingleSpline();

	for (int i = 0; i < mcList.Count(); i++ ) {
		Point3 pivot = nodes[i]->GetObjOffsetPos();
		int altered = 0;
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);
		shapeData->PreUpdateChanges(shape);
		
//to handle copying bound vert list watje 4-27-99
		Tab<int> oldSplines;
		Tab<int> newSplines;

		int polys = shape->splineCount;
		for(int poly = 0; poly < polys; ++poly) {
			if(shape->polySel[poly]) {
//to handle copying bound vert list watje 4-27-99
				if (copy)
					{
					oldSplines.Append(1,&poly,1);
					int nsp = shape->SplineCount();
					newSplines.Append(1,&nsp,1);
					}
				Spline3D *spline = shape->splines[poly];
				altered = holdNeeded = 1;
				// Save the unmodified verts.
				if ( theHold.Holding() ) {
					theHold.Put(new ShapeRestore(shapeData,this,shape));
					}
				MirrorPoly(shape, poly, type, copy, mirrorAboutPivot ? &pivot : NULL);
				}
			}

		if(altered) {
//to handle copying bound vert list 4-27-99
			shapeData->UpdateChanges(shape);
			shapeData->TempData(this)->Invalidate(PART_TOPO);
			if (copy)
				{
				for (int m = 0; m < shape->bindList.Count(); m++)
					{
					BOOL foundSeg=FALSE, foundSpline=FALSE;
					for (int j = 0; j < oldSplines.Count(); j++)
						{
						if (oldSplines[j] == shape->bindList[m].pointSplineIndex)
							{
							for (int k = 0; k < oldSplines.Count(); k++)
								{
								if (oldSplines[k] == shape->bindList[m].segSplineIndex)
//match found add to bindlist
									{
									bindShape temp;
									temp = shape->bindList[m];
									temp.segSplineIndex = newSplines[k];
									temp.pointSplineIndex = newSplines[j];
									shape->bindList.Append(1,&temp,1);
									}
								}
							}
						}
					}
				}
			for (int m = 0; m < shape->bindList.Count(); m++)
				{
				int index = 0;
				int spindex = shape->bindList[m].pointSplineIndex;
				Point3 p;
				if (shape->bindList[m].isEnd)
					index = shape->splines[spindex]->KnotCount()-1;
				shape->bindList[m].bindPoint = shape->splines[spindex]->GetKnot(index).Knot();
				shape->bindList[m].segPoint = shape->splines[spindex]->GetKnot(index).Knot();
				}
//			shape->UpdateBindList();


			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	
	if(holdNeeded) {
		theHold.Accept(GetString(IDS_TH_MIRROR));
		}
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOSPLINESSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	SelectionChanged();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(t,REDRAW_NORMAL);
	}

// Delete all selected polygons
void EditSplineMod::DoPolyDelete() {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	theHold.Begin();

	MaybeSelectSingleSpline();

	for ( int i = 0; i < mcList.Count(); i++ ) {
		int altered = 0;
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);
		shapeData->PreUpdateChanges(shape);
		
		int polys = shape->splineCount;
		for(int poly = polys-1; poly >= 0; --poly) {
			if(shape->polySel[poly]) {
				Spline3D *spline = shape->splines[poly];
				altered = holdNeeded = 1;
				// Save the unmodified verts.
				if ( theHold.Holding() ) {
					theHold.Put(new ShapeRestore(shapeData,this,shape));
					}
				shape->DeleteSpline(poly);
				}
			}

		if(altered) {
			shapeData->UpdateChanges(shape);
			shapeData->TempData(this)->Invalidate(PART_TOPO);
			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	
	if(holdNeeded)
		theHold.Accept(GetString(IDS_TH_DELETESPLINE));
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOSPLINESSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	SelectionChanged();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

int EditSplineMod::GetBoolCursorID() {
	switch(boolType) {
		case BOOL_UNION:
			return IDC_BOOLUNION;
		case BOOL_SUBTRACTION:
			return IDC_BOOLSUBTRACTION;
		case BOOL_INTERSECTION:
			return IDC_BOOLINTERSECTION;
		}
	assert(0);
	return IDC_BOOLUNION;
	}

int EditSplineMod::GetBoolMirrString(int type) {
	switch(type) {
		case BOOL_UNION:
			return IDS_TH_UNION;
		case BOOL_SUBTRACTION:
			return IDS_TH_SUBTRACTION;
		case BOOL_INTERSECTION:
			return IDS_TH_INTERSECTION;
		case MIRROR_HORIZONTAL:
			return IDS_TH_MIRROR_H;
		case MIRROR_VERTICAL:
			return IDS_TH_MIRROR_V;
		case MIRROR_BOTH:
			return IDS_TH_MIRROR_BOTH;
		}
	assert(0);
	return IDC_BOOLUNION;
	}

#define IndexIsHandle(ix) (((ix) % 3) == 1 ? FALSE : TRUE)

int EditSplineMod::RememberVertThere(HWND hWnd, IPoint2 m) {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();

	// Initialize so there isn't any remembered shape
	rememberedShape = NULL;

	if ( !ip ) return 0;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	// See if we're over a vertex
	ViewExp *vpt = ip->GetViewport(hWnd);
	GraphicsWindow *gw = vpt->getGW();
	HitRegion hr;
	MakeHitRegion(hr, HITTYPE_POINT, 1, 4, &m);
	gw->setHitRegion(&hr);
	SubShapeHitList hitList;

	int result = 0;
	int knotType = -1;

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		INode *inode = nodes[i];
		Matrix3 mat = inode->GetObjectTM(t);
		gw->setTransform(mat);	
		shape->SubObjectHitTest(gw, gw->getMaterial(), &hr, SUBHIT_SHAPE_VERTS/* | HIT_ABORTONHIT*/, hitList );
		ShapeSubHitRec *hit = hitList.First();
		if(hit) {
			result = 1;
			// Go thru the list and see if we have one that's selected
			// If more than one selected and they're different types, set unknown type
			hit = hitList.First();
			while(hit) {
				if(shape->vertSel[hit->poly][hit->index]) {
					if(shape->SelVertsSameType()) {
						rememberedShape = NULL;
						rememberedData = shape->splines[hit->poly]->GetKnotType(hit->index / 3);
						goto onselect;		// CAL-02/14/03: find the knot type of current selection
						}
					// Selected verts not all the same type!
					rememberedShape = NULL;
					rememberedData = -1;	// Not all the same!
					goto finish;
					}
				hit = hit->Next();
				}
			hit = hitList.First();
			if(IndexIsHandle(hit->index)) {
				result = 0;
				goto finish;
				}
			if(ip->SelectionFrozen())
				goto onselect;		// CAL-02/14/03: find the knot type of current selection
			// If this is the first edit, then the delta arrays will be allocated
			shapeData->BeginEdit(t);
			shapeData->PreUpdateChanges(shape, FALSE);
			theHold.Begin();
			if ( theHold.Holding() )
				theHold.Put(new ShapeRestore(shapeData,this,shape));
			theHold.Accept(GetString(IDS_DS_SELECT));
			// Select just this vertex
			shape->vertSel.ClearAll();
			shape->vertSel[hit->poly].Set(hit->index);
			shapeData->UpdateChanges(shape, FALSE);
			NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
			ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);

			rememberedShape = shape;
			rememberedPoly = hit->poly;
			rememberedIndex = hit->index / 3;	// Convert from bezier index to knot
			rememberedData = shape->splines[hit->poly]->GetKnotType(rememberedIndex);
			SelectionChanged();
			goto finish;
			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}

onselect:
	// CAL-02/14/03: No hit on knot or handle. Find the vertex type of the current selection. (FID #830)
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	for ( i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		int poly, index;
		// find the first selected knot
		for (poly = 0; poly < shape->SplineCount(); poly++) {
			Spline3D *spline = shape->splines[poly];
			for(index = 0; index < spline->KnotCount(); index++)
				if (shape->vertSel[poly][index*3+1]) break;
			if (index < spline->KnotCount()) break;
		}
		// find the knot type of the selected knot(s)
		if (poly < shape->SplineCount()) {	// some knots are selected
			result = 1;
			rememberedShape = NULL;
			if (shape->SelVertsSameType()) {
				rememberedData = shape->splines[poly]->GetKnotType(index);
				if (knotType < 0) {
					knotType = rememberedData;
				} else if (knotType != rememberedData) {
					rememberedData = -1;
					goto finish;
				}
			} else {
				rememberedData = -1;	// Not all the same!
				goto finish;
			}
		}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
	}
	
finish:
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	if ( vpt ) ip->ReleaseViewport(vpt);
	nodes.DisposeTemporary();

	return result;
	}

void EditSplineMod::ChangeSelVerts(int type) {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	BOOL holdNeeded = FALSE;
	BOOL hadSelected = FALSE;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	theHold.Begin();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		BOOL altered = FALSE;
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);
		shapeData->PreUpdateChanges(shape, FALSE);

		int polys = shape->splineCount;
		for(int poly = polys - 1; poly >= 0; --poly) {
			Spline3D *spline = shape->splines[poly];
			// If any bits are set in the selection set, let's DO IT!!
			if(shape->vertSel[poly].NumberSet()) {
				altered = holdNeeded = TRUE;
				if ( theHold.Holding() )
					theHold.Put(new ShapeRestore(shapeData,this,shape));
				// Call the vertex type change function
				ChangeVertexType(shape, poly, -1, type);
				}
			}

		if(altered) {
			shapeData->UpdateChanges(shape, FALSE);
			shapeData->TempData(this)->Invalidate(PART_TOPO);
			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	
	if(holdNeeded)
		theHold.Accept(GetString(IDS_TH_VERTCHANGE));
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOVERTSSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

void EditSplineMod::ChangeRememberedVert(int type) {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		if(shape == rememberedShape) {
			// If this is the first edit, then the delta arrays will be allocated
			shapeData->BeginEdit(t);
			shapeData->PreUpdateChanges(shape, FALSE);

			theHold.Begin();
			if ( theHold.Holding() )
				theHold.Put(new ShapeRestore(shapeData,this,shape));
			// Call the vertex type change function
			ChangeVertexType(shape, rememberedPoly, rememberedIndex, type);
			shapeData->UpdateChanges(shape, FALSE);
			shapeData->TempData(this)->Invalidate(PART_TOPO);
			theHold.Accept(GetString(IDS_TH_VERTCHANGE));
			ClearShapeDataFlag(mcList,ESD_BEENDONE);
			NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
			ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
			return;
 			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	}

void EditSplineMod::SetRememberedVertType(int type) {
	if(rememberedShape)
		ChangeRememberedVert(type);
	else
		ChangeSelVerts(type);
	}


int EditSplineMod::RememberSegThere(HWND hWnd, IPoint2 m) {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();

	// Initialize so there isn't any remembered shape
	rememberedShape = NULL;

	if ( !ip ) return 0;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	// Okay, nothing is selected -- See if we're over a segment
	ViewExp *vpt = ip->GetViewport(hWnd);
	GraphicsWindow *gw = vpt->getGW();
	HitRegion hr;
	MakeHitRegion(hr, HITTYPE_POINT, 1, 4, &m);
	gw->setHitRegion(&hr);
	SubShapeHitList hitList;

	int result = 0;
	int lineType = -1;

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		INode *inode = nodes[i];
		Matrix3 mat = inode->GetObjectTM(t);
		gw->setTransform(mat);	
		shape->SubObjectHitTest(gw, gw->getMaterial(), &hr, SUBHIT_SHAPE_SEGMENTS /*| HIT_ABORTONHIT*/, hitList );
		ShapeSubHitRec *hit = hitList.First();
		if(hit) {
			result = 1;
			// Go thru the list and see if we have one that's selected
			// If more than one selected and they're different types, set unknown type
			hit = hitList.First();
			while(hit) {
				if(shape->segSel[hit->poly][hit->index]) {
					if(shape->SelSegsSameType()) {
						rememberedShape = NULL;
						rememberedData = shape->splines[hit->poly]->GetLineType(hit->index);
						goto onselect;		// CAL-05/29/03: find the line type of current selection
						}
					// Selected segs not all the same type!
					rememberedShape = NULL;
					rememberedData = -1;	// Not all the same!
					goto finish;
					}
				hit = hit->Next();
				}
			if(ip->SelectionFrozen())
				goto onselect;		// CAL-05/29/03: find the line type of current selection
			// If this is the first edit, then the delta arrays will be allocated
			shapeData->BeginEdit(t);
			shapeData->PreUpdateChanges(shape, FALSE);
			hit = hitList.First();
			theHold.Begin();
			if ( theHold.Holding() )
				theHold.Put(new ShapeRestore(shapeData,this,shape));
			theHold.Accept(GetString(IDS_DS_SELECT));
			// Select just this segment
			shape->segSel.ClearAll();
			shape->segSel[hit->poly].Set(hit->index);
			shapeData->UpdateChanges(shape, FALSE);
			NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
			ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);

			rememberedShape = shape;
			rememberedPoly = hit->poly;
			rememberedIndex = hit->index;
			rememberedData = shape->splines[rememberedPoly]->GetLineType(rememberedIndex);
			SelectionChanged();
			goto finish;
			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}

onselect:
	// CAL-05/29/03: No hit on segment. Find the line type of the current selection. (FID #830)
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	for ( i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		int poly, index;
		// find the first selected segment
		for (poly = 0; poly < shape->SplineCount(); poly++) {
			Spline3D *spline = shape->splines[poly];
			for(index = 0; index < spline->Segments(); index++)
				if (shape->segSel[poly][index]) break;
			if (index < spline->Segments()) break;
		}
		// find the line type of the selected segment(s)
		if (poly < shape->SplineCount()) {	// some segments are selected
			result = 1;
			rememberedShape = NULL;
			if (shape->SelSegsSameType()) {
				rememberedData = shape->splines[poly]->GetLineType(index);
				if (lineType < 0) {
					lineType = rememberedData;
				} else if (lineType != rememberedData) {
					rememberedData = -1;
					goto finish;
				}
			} else {
				rememberedData = -1;	// Not all the same!
				goto finish;
			}
		}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
	}

finish:
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	if ( vpt ) ip->ReleaseViewport(vpt);
	nodes.DisposeTemporary();
	return result;
	}

void EditSplineMod::ChangeSelSegs(int type) {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	BOOL holdNeeded = FALSE;
	BOOL hadSelected = FALSE;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	theHold.Begin();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		BOOL altered = FALSE;
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);
		shapeData->PreUpdateChanges(shape, FALSE);

		int polys = shape->splineCount;
		for(int poly = polys - 1; poly >= 0; --poly) {
			Spline3D *spline = shape->splines[poly];
			// If any bits are set in the selection set, let's DO IT!!
			if(shape->segSel[poly].NumberSet()) {
				altered = holdNeeded = TRUE;
				if ( theHold.Holding() )
					theHold.Put(new ShapeRestore(shapeData,this,shape));
				// Call the segment type change function
				ChangeSegmentType(shape, poly, -1, type);
				}
			}

		if(altered) {
			shapeData->UpdateChanges(shape, FALSE);
			shapeData->TempData(this)->Invalidate(PART_TOPO);
			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	
	if(holdNeeded)
		theHold.Accept(GetString(IDS_TH_SEGCHANGE));
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOSEGSSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

void EditSplineMod::ChangeRememberedSeg(int type) {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		if(shape == rememberedShape) {
			// If this is the first edit, then the delta arrays will be allocated
			shapeData->BeginEdit(t);
			shapeData->PreUpdateChanges(shape, FALSE);

			theHold.Begin();
			if ( theHold.Holding() )
				theHold.Put(new ShapeRestore(shapeData,this,shape));
			// Call the segment type change function
			ChangeSegmentType(shape, rememberedPoly, rememberedIndex, type);
			shapeData->UpdateChanges(shape, FALSE);
			shapeData->TempData(this)->Invalidate(PART_TOPO);
			theHold.Accept(GetString(IDS_TH_SEGCHANGE));
			ClearShapeDataFlag(mcList,ESD_BEENDONE);
			NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
			ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
			nodes.DisposeTemporary();
			return;
 			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	nodes.DisposeTemporary();
	}

void EditSplineMod::SetRememberedSegType(int type) {
	if(rememberedShape)
		ChangeRememberedSeg(type);
	else
		ChangeSelSegs(type);
	}

int EditSplineMod::RememberPolyThere(HWND hWnd, IPoint2 m) {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();

	// Initialize so there isn't any remembered shape
	rememberedShape = NULL;

	if ( !ip ) return 0;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	// Okay, nothing is selected -- See if we're over a polygon
	ViewExp *vpt = ip->GetViewport(hWnd);
	GraphicsWindow *gw = vpt->getGW();
	HitRegion hr;
	MakeHitRegion(hr, HITTYPE_POINT, 1, 4, &m);
	gw->setHitRegion(&hr);
	SubShapeHitList hitList;

	int result = 0;
	int lineType = -1;

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		INode *inode = nodes[i];
		Matrix3 mat = inode->GetObjectTM(t);
		gw->setTransform(mat);	
		shape->SubObjectHitTest(gw, gw->getMaterial(), &hr, SUBHIT_SHAPE_POLYS /*| HIT_ABORTONHIT*/, hitList );
		ShapeSubHitRec *hit = hitList.First();
		if(hit) {
			result = 1;
			while(hit) {
				if(shape->polySel[hit->poly]) {
					if(shape->SelSplinesSameType()) {
						rememberedShape = NULL;
						rememberedData = shape->splines[hit->poly]->GetLineType(0);
						rememberedPoly = hit->poly;
						goto onselect;		// CAL-05/29/03: find the line type of current selection
						}
					// Selected segs not all the same type!
					rememberedShape = NULL;
					rememberedData = -1;	// Not all the same!
					rememberedPoly = hit->poly;
					goto finish;
					}
				hit = hit->Next();
				}
			if(ip->SelectionFrozen())
				goto onselect;		// CAL-05/29/03: find the line type of current selection
			// If this is the first edit, then the delta arrays will be allocated
			shapeData->BeginEdit(t);
			shapeData->PreUpdateChanges(shape, FALSE);
			hit = hitList.First();
			theHold.Begin();
			if ( theHold.Holding() )
				theHold.Put(new ShapeRestore(shapeData,this,shape));
			// Select just this poly
			shape->polySel.ClearAll();
			shape->polySel.Set(hit->poly);
			theHold.Accept(GetString(IDS_DS_SELECT));
			shapeData->UpdateChanges(shape, FALSE);
			NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
			ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);

			rememberedShape = shape;
			if(shape->SelSplinesSameType())
				rememberedData = shape->splines[hit->poly]->GetLineType(0);
			else
				rememberedData = -1;
			rememberedPoly = hit->poly;
			SelectionChanged();
			goto finish;
			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}

onselect:
	// CAL-05/29/03: No hit on spline. Find the line type of the current selection. (FID #830)
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	for ( i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		int poly;
		// find the first selected spline
		for (poly = 0; poly < shape->SplineCount(); poly++) {
			if (shape->polySel[poly]) break;
		}
		// find the line type of the selected spline(s)
		if (poly < shape->SplineCount()) {	// some splines are selected
			result = 1;
			rememberedShape = NULL;
			rememberedPoly = poly;
			if (shape->SelSplinesSameType()) {
				rememberedData = shape->splines[poly]->GetLineType(0);
				if (lineType < 0) {
					lineType = rememberedData;
				} else if (lineType != rememberedData) {
					rememberedData = -1;
					goto finish;
				}
			} else {
				rememberedData = -1;	// Not all the same!
				goto finish;
			}
		}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
	}

finish:
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	if ( vpt ) ip->ReleaseViewport(vpt);
	nodes.DisposeTemporary();
	return result;
	}

void EditSplineMod::ChangeSelPolys(int type) {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	BOOL holdNeeded = FALSE;
	BOOL hadSelected = FALSE;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	theHold.Begin();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		BOOL altered = FALSE;
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);
		shapeData->PreUpdateChanges(shape, FALSE);

		int polys = shape->splineCount;
		for(int poly = polys - 1; poly >= 0; --poly) {
			Spline3D *spline = shape->splines[poly];
			// If any bits are set in the selection set, let's DO IT!!
			if(shape->polySel[poly]) {
				altered = holdNeeded = TRUE;
				if ( theHold.Holding() )
					theHold.Put(new ShapeRestore(shapeData,this,shape));
				// Call the polygon type change function
				ChangePolyType(shape, poly, type);
				}
			}

		if(altered) {
			shapeData->UpdateChanges(shape, FALSE);
			shapeData->TempData(this)->Invalidate(PART_TOPO);
			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	
	if(holdNeeded)
		theHold.Accept(GetString(IDS_TH_SPLINECHANGE));
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOSPLINESSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

void EditSplineMod::ChangeRememberedPoly(int type) {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		if(shape == rememberedShape) {
			// If this is the first edit, then the delta arrays will be allocated
			shapeData->BeginEdit(t);
			shapeData->PreUpdateChanges(shape, FALSE);

			theHold.Begin();
			if ( theHold.Holding() )
				theHold.Put(new ShapeRestore(shapeData,this,shape));
			// Call the segment type change function
			ChangePolyType(shape, rememberedPoly, type);
			shapeData->UpdateChanges(shape, FALSE);
			shapeData->TempData(this)->Invalidate(PART_TOPO);
			theHold.Accept(GetString(IDS_TH_SPLINECHANGE));
			ClearShapeDataFlag(mcList,ESD_BEENDONE);
			NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
			ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
			nodes.DisposeTemporary();
			return;
 			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	}

void EditSplineMod::SetRememberedPolyType(int type) {
	if(rememberedShape)
		ChangeRememberedPoly(type);
	else
		ChangeSelPolys(type);
	}

static int butIDs[] = { 0, ES_VERTEX, ES_SEGMENT, ES_SPLINE };

void EditSplineMod::RefreshSelType () {
	ICustToolbar *iToolbar = GetICustToolbar(GetDlgItem(hSelectPanel,IDC_SELTYPE));
	ICustButton *but;
	for (int i=1; i<4; i++) {
		but = iToolbar->GetICustButton (butIDs[i]);
		but->SetCheck (GetSubobjectLevel()==i);
		ReleaseICustButton (but);
	}
	ReleaseICustToolbar(iToolbar);
	SetSelDlgEnables();
	SetOpsDlgEnables();
	SetSoftSelDlgEnables();
	SetSurfDlgEnables();
	SelectionChanged();

}

void EditSplineMod::UpdateSelectDisplay() {	
	TSTR buf;
	int num, j;

	if (!hSelectPanel) return;

	ModContextList mcList;
	INodeTab nodes;
	if ( !ip )
		return;
	ip->GetModContexts(mcList,nodes);

	switch (GetSubobjectLevel()) {
		case ES_OBJECT:
			buf.printf (GetString (IDS_TH_OBJECT_SEL));
			break;

		case ES_VERTEX: {
			num = 0;
			int thePoly = 0;
			BezierShape *theShape = NULL;
			for ( int i = 0; i < mcList.Count(); i++ ) {
				EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
				if ( !shapeData ) continue;		
			
				if ( shapeData->tempData && shapeData->TempData(this)->ShapeCached(ip->GetTime()) ) {
					BezierShape *shape = shapeData->TempData(this)->GetShape(ip->GetTime());
					if(!shape) continue;
					int polys = shape->splineCount;
//watje
					if (showSelected) 
						{
						shape->dispFlags |= DISP_SELSEGMENTS;
						}
					else
						{
						shape->dispFlags &= ~DISP_SELSEGMENTS;
						}

					for(int poly = 0; poly < polys; ++poly) {
						if(shape->splines[poly]->KnotCount() > 0) {
							int thisNum = 0;
							for (j=1; j<shape->vertSel.sel[poly].GetSize(); j+=3) {
								if (shape->vertSel.sel[poly][j])
									thisNum++;
								}
							if(thisNum) {
								num += thisNum;
								thePoly = poly;
								theShape = shape;
								}
							}
						}
					}
				}
			if (num==1) {
				for (j=1; j<theShape->vertSel.sel[thePoly].GetSize(); j+=3)
					if (theShape->vertSel.sel[thePoly][j]) break;
				buf.printf (GetString(IDS_TH_SPLVERTSEL), thePoly+1, (j-1)/3+1);
				}
			else
				buf.printf (GetString(IDS_TH_NUMVERTSELP), num);
			}
			break;

		case ES_SEGMENT: {
			num = 0;
			int thePoly = 0;
			BezierShape *theShape = NULL;
			for ( int i = 0; i < mcList.Count(); i++ ) {
				EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
				if ( !shapeData ) continue;		
			
				if ( shapeData->tempData && shapeData->TempData(this)->ShapeCached(ip->GetTime()) ) {
					BezierShape *shape = shapeData->TempData(this)->GetShape(ip->GetTime());
					if(!shape) continue;
					int polys = shape->splineCount;
					for(int poly = 0; poly < polys; ++poly) {
						if(shape->splines[poly]->KnotCount() > 0) {
							int thisNum = shape->segSel.sel[poly].NumberSet();
							if(thisNum) {
								num += thisNum;
								thePoly = poly;
								theShape = shape;
								}
							}
						}
					}
				}
			if (num==1) {
				for (j=0; j<theShape->segSel.sel[thePoly].GetSize(); j++)
					if (theShape->segSel.sel[thePoly][j]) break;
				buf.printf (GetString(IDS_TH_SPLSEGSEL), thePoly+1, j+1);
				}
			else
				buf.printf (GetString(IDS_TH_NUMSEGSELP), num);
			}
			break;

		case ES_SPLINE: {
			int num = 0;
			int thePoly = 0;
			BezierShape *theShape = NULL;
			for ( int i = 0; i < mcList.Count(); i++ ) {
				EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
				if ( !shapeData ) continue;		
			
				if ( shapeData->tempData && shapeData->TempData(this)->ShapeCached(ip->GetTime()) ) {
					BezierShape *shape = shapeData->TempData(this)->GetShape(ip->GetTime());
					if(!shape) continue;
					int polys = shape->splineCount;
					if(polys) {
						int thisNum = shape->polySel.sel.NumberSet();
						if(thisNum) {
							num += thisNum;
							theShape = shape;
							}
						}
					}
				}
			if (num==1) {
				for (j=0; j<theShape->splineCount; j++)
					if (theShape->polySel[j]) break;
				TSTR oc = theShape->splines[j]->Closed() ? GetString(IDS_TH_CLOSED) : GetString(IDS_TH_OPEN);
				buf.printf (GetString(IDS_TH_NUMSPLINESEL), j+1, oc);
				}	
			else
				buf.printf(GetString(IDS_TH_NUMSPLINESELP),num);
			}
			break;
		}

	SetDlgItemText(hSelectPanel, IDC_NUMSEL_LABEL, buf);

	TSTR string;
	if(GetSubobjectLevel() == ES_SPLINE) {
		int vertCount = 0;
		for ( int i = 0; i < mcList.Count(); i++ ) {
			EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
			if ( !shapeData ) continue;		
		
			if ( shapeData->tempData && shapeData->TempData(this)->ShapeCached(ip->GetTime()) ) {
				BezierShape *shape = shapeData->TempData(this)->GetShape(ip->GetTime());
				if(!shape) continue;
				int polys = shape->splineCount;
				for(int poly = 0; poly < polys; ++poly) {
					if(shape->polySel[poly])
						vertCount += shape->splines[poly]->KnotCount();
					}
				}
			}		
		string.printf(GetString(IDS_TH_SPLINE_VERT_COUNT),vertCount);
		}
	else
		string = _T("");

	nodes.DisposeTemporary();
	SetDlgItemText(hSelectPanel, IDC_SPLINE_VERT_COUNT, string);
	}


// hack... see comment at top of file
//
void hackInvalidateOpsUI(HWND hOpsPanel)
{
	if(hOpsPanel && !s_bHackOpsUIValid) 
	{
		InvalidateRect (hOpsPanel, NULL, FALSE);
		s_bHackOpsUIValid = TRUE;
	}
}

// hack... see comment at top of file
//
void hackInvalidateSelUI(HWND hSelPanel)
{
	if(hSelPanel && !s_bHackSelUIValid) 
	{
		InvalidateRect (hSelPanel, NULL, FALSE);
		s_bHackSelUIValid = TRUE;
	}
}


void EditSplineMod::SetSelDlgEnables() {
	if(!hSelectPanel)
		return;
	
	BOOL vType = (GetSubobjectLevel() == ES_VERTEX) ? TRUE : FALSE;
	BOOL sType = (GetSubobjectLevel() == ES_SEGMENT) ? TRUE : FALSE;
	BOOL splType = (GetSubobjectLevel() == ES_SPLINE) ? TRUE : FALSE;
	BOOL oType =   (GetSubobjectLevel() == SS_OBJECT)  ? TRUE : FALSE;

	EnableWindow (GetDlgItem (hSelectPanel, IDC_SVN_SELECTED), showVertNumbers);

	// can only copy in SO mode
	//
	ICustButton *copyButton = GetICustButton(GetDlgItem(hSelectPanel,IDC_NS_COPY));
	copyButton->Disable();
	if (!oType)
	{
		copyButton->Enable();
	}
	ReleaseICustButton(copyButton);
	
	ICustButton *but = GetICustButton(GetDlgItem(hSelectPanel,IDC_NS_PASTE));
	but->Disable();
	switch(GetSubobjectLevel()) {
		case ES_VERTEX:
			if (GetShapeNamedVertSelClip())
				but->Enable();
			break;
		case ES_SEGMENT:
			if (GetShapeNamedSegSelClip())
				but->Enable();
			break;
		case ES_SPLINE:
			if (GetShapeNamedPolySelClip())
				but->Enable();
			break;
		}
	ReleaseICustButton(but);
//2-1-99
	ISpinnerControl *spin;
	spin = GetISpinner(GetDlgItem(hSelectPanel,IDC_SELECTAREA_SPIN));
	spin->Enable(vType);
	ReleaseISpinner(spin);

	EnableWindow(GetDlgItem(hSelectPanel, IDC_EDSPLINE_SELECT_DISPLAY_LABEL), !oType);
	EnableWindow(GetDlgItem(hSelectPanel, IDC_EDSPLINE_SELECT_NAMEDSEL_LABEL), !oType);
	EnableWindow(GetDlgItem(hSelectPanel, IDC_SHOW_VERTEX_NUMBERS), !oType);

	EnableWindow(GetDlgItem(hSelectPanel, IDC_SELECT_BY), (vType || sType));
	EnableWindow(GetDlgItem(hSelectPanel, IDC_AREA_SELECTION), vType);
	EnableWindow(GetDlgItem(hSelectPanel, IDC_SEGMENT_END),    vType);
	EnableWindow(GetDlgItem(hSelectPanel, IDC_LOCK_HANDLES),   vType);
	EnableWindow(GetDlgItem(hSelectPanel, IDC_LOCKALIKE),      vType);
	EnableWindow(GetDlgItem(hSelectPanel, IDC_LOCKALL),        vType);

	// invalidate UI, labels have changed
	//
	s_bHackSelUIValid = FALSE;
	}

void EditSplineMod::SetSoftSelDlgEnables( HWND hSoftSel ) {
	
	if ( !hSoftSel && !hSoftSelPanel )  // nothing to set
		return;

	if ( !hSoftSel && hSoftSelPanel ) { // user omitted hSoftSel param, use hSoftSelPanel
		hSoftSel = hSoftSelPanel;
	}

	switch ( GetSubobjectLevel() ) {
	case ES_VERTEX:
	case ES_SEGMENT:
	case ES_SPLINE:
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

void EditSplineMod::SetOpsDlgEnables() {
	if(!hOpsPanel)
		return;
	
	assert(ip);

	// Disconnect right-click and delete mechanisms
	ip->GetRightClickMenuManager()->Unregister(&esMenu);
    ip->GetActionManager()->DeactivateActionTable(&sActionCallback, kSplineActions);
	ip->UnRegisterDeleteUser(&esDel);

	BOOL oType = (GetSubobjectLevel() == ES_OBJECT) ? TRUE : FALSE;
	BOOL vType = (GetSubobjectLevel() == ES_VERTEX) ? TRUE : FALSE;
	BOOL sType = (GetSubobjectLevel() == ES_SEGMENT) ? TRUE : FALSE;
	BOOL splType = (GetSubobjectLevel() == ES_SPLINE) ? TRUE : FALSE;
	BOOL ssType = (sType || splType) ? TRUE : FALSE;
	BOOL vsType = (vType || sType) ? TRUE : FALSE;
	
	// Shut off command modes that don't work in the new subobject level

	switch(ip->GetCommandMode()->ID()) {
		case CID_OUTLINE:
		case CID_TRIM:
		case CID_EXTEND:
		case CID_BOOLEAN:
			if(!splType)
				ip->SetStdCommandMode( CID_OBJMOVE );
			break;
		case CID_SEGBREAK:
			if(!sType)
				ip->SetStdCommandMode( CID_OBJMOVE );
			break;
		case CID_SEGREFINE:
//		watje
		case CID_REFINECONNECT:

			if(!vsType)
				ip->SetStdCommandMode( CID_OBJMOVE );
			break;
		case CID_VERTCONNECT:
		case CID_CROSSINSERT:
		case CID_FILLET:
		case CID_CHAMFER:
//		watje
		case CID_SPLINEBIND:
			if(!vType)
				ip->SetStdCommandMode( CID_OBJMOVE );
			break;
		case CID_VERTINSERT:
		case CID_CREATELINE:
			// Any level OK!
			break;
		}

	ICustButton *but;
	ISpinnerControl *spin;
	ICustToolbar *iToolbar;

	// CAL-02/26/03: Turn off Cross Section when multiple instances are selected. (FID #827)
	ModContextList mcList;	
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	nodes.DisposeTemporary();
	BOOL singleInstance = (mcList.Count() == 1) ? TRUE : FALSE;
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ES_CROSS_SECTION));
	but->Enable (singleInstance);
	ReleaseICustButton (but);

//2-1-99 watje
	EnableWindow (GetDlgItem (hOpsPanel, IDC_ES_RCONNECT), sType || vType);
	if (rConnect)
		{
		EnableWindow (GetDlgItem (hOpsPanel, IDC_ES_RCLINEAR), sType || vType);
		EnableWindow (GetDlgItem (hOpsPanel, IDC_ES_RCCLOSED), sType || vType);
		EnableWindow (GetDlgItem (hOpsPanel, IDC_ES_BINDFIRST), sType || vType);
		EnableWindow (GetDlgItem (hOpsPanel, IDC_ES_BINDLAST), sType || vType);
		}
	else
		{
		EnableWindow (GetDlgItem (hOpsPanel, IDC_ES_RCLINEAR), FALSE);
		EnableWindow (GetDlgItem (hOpsPanel, IDC_ES_RCCLOSED), FALSE);
		EnableWindow (GetDlgItem (hOpsPanel, IDC_ES_BINDFIRST), FALSE);
		EnableWindow (GetDlgItem (hOpsPanel, IDC_ES_BINDLAST), FALSE);
		}

//6-16-99 watje
	EnableWindow (GetDlgItem (hOpsPanel, IDC_ES_SHOW_SELECTED), sType || vType);

//watje
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ES_BIND));
	but->Enable (vType);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ES_UNBIND));
	but->Enable (vType);
	ReleaseICustButton (but);

	// CAL-05/23/03: Add Connecting Splines when Shift-Move Copy. (FID #827)
	EnableWindow (GetDlgItem (hOpsPanel, IDC_ES_CONNECT_COPY_STATIC), ssType);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_ES_CONNECT_COPY), ssType);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_ES_DIST_THRESH_LABEL), ssType && connectCopy);
	spin = GetISpinner(GetDlgItem(hOpsPanel, IDC_ES_CONNECT_COPY_THRESHSPINNER));
	spin->Enable(ssType && connectCopy);
	ReleaseISpinner(spin);

	// CAL-03/03/03: copy/paste tangent. (FID #827)
	EnableWindow (GetDlgItem (hOpsPanel, IDC_ES_TANGENT_STATIC), vType);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ES_COPY_TANGENT));
	but->Enable (vType);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ES_PASTE_TANGENT));
	but->Enable (vType && tangentCopied);
	ReleaseICustButton (but);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_ES_COPY_TAN_LENGTH), vType && tangentCopied);

	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ES_FUSE));
	but->Enable (vType);
	ReleaseICustButton (but);

/*2-1-99 watje
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ES_REFINECONNECT));
	but->Enable (sType ||vType);
	ReleaseICustButton (but);
	*/
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ES_CYCLE));
	but->Enable (vType);
	ReleaseICustButton (but);

	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ES_HIDE));
	but->Enable (!oType);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ES_UNHIDE));
	but->Enable (!oType);
	ReleaseICustButton (but);


	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ES_CONNECT));
	but->Enable (vType);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ES_REFINE));
	but->Enable (sType || vType);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ES_BREAK));

	if(sType) {
		but->SetHighlightColor(GREEN_WASH);
		but->SetType(CBT_CHECK);
		}
	else {
		but->SetType(CBT_PUSH);
		}
	but->Enable (vType || sType);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ES_EXPLODE));
	but->Enable (splType);
	ReleaseICustButton (but);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_TO_SPLINES), splType);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_TO_OBJECTS), splType);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_EDSPLINE_OPS_TO_LABEL), splType);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_EDSPLINE_OPS_DISPLAY_LABEL), vType || sType );

	// labels have changed, trigger invalidate
	//
	s_bHackOpsUIValid = FALSE;
	
	
	
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ES_CROSS_INSERT));
	but->Enable (vType);
	ReleaseICustButton (but);
	spin = GetISpinner(GetDlgItem(hOpsPanel,IDC_ES_CROSSTHRESHSPINNER));
	spin->Enable(vType);
	ReleaseISpinner(spin);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ES_MAKEFIRST));
	but->Enable (vType);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ES_WELD));
	but->Enable (vType);
	ReleaseICustButton (but);
	spin = GetISpinner(GetDlgItem(hOpsPanel,IDC_ES_THRESHSPINNER));
	spin->Enable(vType);
	ReleaseISpinner(spin);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ES_REVERSE));
	but->Enable (splType);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ES_CLOSE));
	but->Enable (splType);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ES_SEGDIVIDE));
	but->Enable (sType);
	ReleaseICustButton (but);
	spin = GetISpinner(GetDlgItem(hOpsPanel,IDC_ES_DIVSSPINNER));
	spin->Enable(sType);
	ReleaseISpinner(spin);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ES_DETACH));
	but->Enable (ssType);
	ReleaseICustButton (but);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_ES_SAMESHAPE), sType);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_ES_DETACHREORIENT), ssType);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_ES_DETACHCOPY), ssType);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ES_OUTLINE));
	but->Enable (splType);
	ReleaseICustButton (but);
	spin = GetISpinner(GetDlgItem(hOpsPanel,IDC_ES_OUTLINESPINNER));
	spin->Enable(splType);
	ReleaseISpinner(spin);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_ES_OUTCENTER), splType);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ES_FILLET));
	but->Enable (vType);
	ReleaseICustButton (but);
	spin = GetISpinner(GetDlgItem(hOpsPanel,IDC_ES_FILLETSPINNER));
	spin->Enable(vType);
	ReleaseISpinner(spin);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ES_CHAMFER));
	but->Enable (vType);
	ReleaseICustButton (but);
	spin = GetISpinner(GetDlgItem(hOpsPanel,IDC_ES_CHAMFERSPINNER));
	spin->Enable(vType);
	ReleaseISpinner(spin);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ES_BOOLEAN));
	but->Enable (splType);
	ReleaseICustButton (but);
	iToolbar = GetICustToolbar(GetDlgItem(hOpsPanel,IDC_ES_BOOL_TYPE));
	but = iToolbar->GetICustButton(BOOL_UNION);
	but->Enable (splType);
	ReleaseICustButton (but);
	but = iToolbar->GetICustButton(BOOL_SUBTRACTION);
	but->Enable (splType);
	ReleaseICustButton (but);
	but = iToolbar->GetICustButton(BOOL_INTERSECTION);
	but->Enable (splType);
	ReleaseICustButton (but);
	ReleaseICustToolbar(iToolbar);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ES_MIRROR));
	but->Enable (splType);
	ReleaseICustButton (but);
	iToolbar = GetICustToolbar(GetDlgItem(hOpsPanel,IDC_ES_MIRROR_TYPE));
	but = iToolbar->GetICustButton(MIRROR_HORIZONTAL);
	but->Enable (splType);
	ReleaseICustButton (but);
	but = iToolbar->GetICustButton(MIRROR_VERTICAL);
	but->Enable (splType);
	ReleaseICustButton (but);
	but = iToolbar->GetICustButton(MIRROR_BOTH);
	but->Enable (splType);
	ReleaseICustButton (but);
	ReleaseICustToolbar(iToolbar);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_ES_COPY_MIRROR), splType);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_ES_ABOUT_PIVOT), splType);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ES_TRIM));
	but->Enable (splType);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ES_EXTEND));
	but->Enable (splType);
	ReleaseICustButton (but);
	EnableWindow (GetDlgItem (hOpsPanel, IDC_ES_TRIM_INFINITE), splType);
	but = GetICustButton (GetDlgItem (hOpsPanel, IDC_ES_DELETE));
	but->Enable (!oType);
	ReleaseICustButton (but);
	spin = GetISpinner(GetDlgItem(hOpsPanel,IDC_ES_ENDPOINTAUTOWELD_THRESHSPINNER));
	spin->Enable(SplineShape::GetUseEndPointAutoConnect()==BST_CHECKED);
	ReleaseISpinner(spin);
	// Enable/disable right-click and delete mechanisms
	if(!oType) {			
		esMenu.SetMod(this);
		ip->GetRightClickMenuManager()->Register(&esMenu);
        sActionCallback.SetMod(this);
        ip->GetActionManager()->ActivateActionTable(&sActionCallback, kSplineActions);
		esDel.SetMod(this);
		ip->RegisterDeleteUser(&esDel);
		}
	}

void EditSplineMod::SetSurfDlgEnables() {
	if(!hSurfPanel)
		return;
	
	assert(ip);

	BOOL sType = (GetSubobjectLevel() == ES_SEGMENT) ? TRUE : FALSE;
	BOOL splType = (GetSubobjectLevel() == ES_SPLINE) ? TRUE : FALSE;
	BOOL ssType = (sType || splType) ? TRUE : FALSE;
	
	ICustButton *but;
	ISpinnerControl *spin;

	spin = GetISpinner(GetDlgItem(hSurfPanel,IDC_MAT_IDSPIN));
	spin->Enable(ssType);
	ReleaseISpinner(spin);
	but = GetICustButton (GetDlgItem (hSurfPanel, IDC_SELECT_BYID));
	but->Enable (ssType);
	ReleaseICustButton (but);
	spin = GetISpinner(GetDlgItem(hSurfPanel,IDC_MAT_IDSPIN_SEL));   
	spin->Enable(ssType);
	ReleaseISpinner(spin);
	EnableWindow(GetDlgItem(hSurfPanel,IDC_CLEARSELECTION), TRUE);       

	// invalidate UI, labels have changed
	//
	s_bHackSelUIValid = FALSE;

	}

void EditSplineMod::SelectionChanged() {
	if (hSelectPanel) {
		UpdateSelectDisplay();
		InvalidateRect(hSelectPanel,NULL,FALSE);
		}
	if (selLevel == ES_SEGMENT || GetSubobjectLevel() == ES_SPLINE)
		InvalidateSurfaceUI();
	// Now see if the selection set matches one of the named selections!
	if(ip && (selLevel != ES_OBJECT)) {
		ModContextList mcList;		
		INodeTab nodes;
		TimeValue t = ip->GetTime();
		ip->GetModContexts(mcList,nodes);
		int sublevel = selLevel - 1;
		int dataSet;
		for(int set = 0; set < namedSel[sublevel].Count(); ++set) {
			ClearShapeDataFlag(mcList,ESD_BEENDONE);
			BOOL gotMatch = FALSE;
			for ( int i = 0; i < mcList.Count(); i++ ) {
				EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
				if ( !shapeData ) continue;
				if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;
				BezierShape *shape = shapeData->TempData(this)->GetShape(t);
				if(!shape) continue;
				// See if this shape has the named selection set
				switch(selLevel) {
					case ES_VERTEX: 
						for(dataSet = 0; dataSet < shapeData->vselSet.Count(); ++dataSet) {
							if(*(shapeData->vselSet.names[dataSet]) == *namedSel[sublevel][set]) {
								if(!(*shapeData->vselSet.sets[set] == shape->vertSel))
									goto next_set;
								gotMatch = TRUE;
								break;
								}
							}
						break;
					case ES_SEGMENT:
						for(dataSet = 0; dataSet < shapeData->sselSet.Count(); ++dataSet) {
							if(*(shapeData->sselSet.names[dataSet]) == *namedSel[sublevel][set]) {
								if(!(*shapeData->sselSet.sets[set] == shape->segSel))
									goto next_set;
								gotMatch = TRUE;
								break;
								}
							}
						break;
					case ES_SPLINE:
						for(dataSet = 0; dataSet < shapeData->pselSet.Count(); ++dataSet) {
							if(*(shapeData->pselSet.names[dataSet]) == *namedSel[sublevel][set]) {
								if(!(*shapeData->pselSet.sets[set] == shape->polySel))
									goto next_set;
								gotMatch = TRUE;
								break;
								}
							}
						break;
					}
				shapeData->SetFlag(ESD_BEENDONE,TRUE);
				}
			// If we reach here, we might have a set that matches
			if(gotMatch) {
				ip->SetCurNamedSelSet(*namedSel[sublevel][set]);
				goto namedSelUpdated;
				}
next_set:;
			}
		// No set matches, clear the named selection
		ip->ClearCurNamedSelSet();

		if ( UseSoftSelections() ) {
			UpdateVertexDists();
			UpdateEdgeDists( );
			UpdateVertexWeights();
		}
		else {
			InvalidateVertexWeights();
		}
				

namedSelUpdated:
		nodes.DisposeTemporary();
		ClearShapeDataFlag(mcList,ESD_BEENDONE);
		}
	}

void EditSplineMod::InvalidateSurfaceUI() {
	if(hSurfPanel) {
		InvalidateRect (hSurfPanel, NULL, FALSE);
		segUIValid = FALSE;
		}
	}

// Material editing operations

int EditSplineMod::GetSelMatIndex()
	{
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();

	if ( !ip ) return 0;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	BOOL first = 1;
	int mat=-1;

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;

		for(int poly = 0; poly < shape->splineCount; ++poly) {
			Spline3D *spline = shape->splines[poly];
			BitArray &sel = shape->segSel[poly];
			BOOL polySelected = (GetSubobjectLevel() == ES_SPLINE) ? shape->polySel[poly] : FALSE;
			for (int seg=0; seg < spline->Segments(); ++seg) {
				BOOL segSelected = (GetSubobjectLevel() == ES_SEGMENT) ? sel[seg] : FALSE;
				if (polySelected || segSelected) {
					if (first) {
						first = FALSE;
						mat   = (int)spline->GetMatID(seg);
					} else {
						if ((int)spline->GetMatID(seg) != mat) {
							return -1;
							}
						}
					}
				}
			}
		
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	
	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	return mat;
	}

void EditSplineMod::SetSelMatIndex(int index)
	{
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	theHold.Begin();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		int altered = 0;
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);
		shapeData->PreUpdateChanges(shape, FALSE);

		if ( theHold.Holding() ) {
			theHold.Put(new ShapeRestore(shapeData,this,shape));
			}

		int polys = shape->splineCount;
		for(int poly = 0; poly < polys; ++poly) {
			Spline3D *spline = shape->splines[poly];
			BOOL polySelected = (GetSubobjectLevel() == ES_SPLINE) ? shape->polySel[poly] : FALSE;
			BitArray &sel = shape->segSel[poly];
			for(int seg = 0; seg < spline->Segments(); ++seg) {
				BOOL segSelected = (GetSubobjectLevel() == ES_SEGMENT) ? sel[seg] : FALSE;
				if(polySelected || segSelected) {
					altered = holdNeeded = TRUE;
					spline->SetMatID(seg,(MtlID)index);
					}
				}
			}

		if(altered) {
			shapeData->UpdateChanges(shape, FALSE);
			shapeData->TempData(this)->Invalidate(PART_TOPO);
			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	
	if(holdNeeded)
		theHold.Accept(GetString(IDS_TH_SEGMTLCHANGE));
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOSEGSSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	InvalidateSurfaceUI();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

void EditSplineMod::SelectByMat(int index,BOOL clear)
	{
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	theHold.Begin();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		int altered = 0;
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);
		shapeData->PreUpdateChanges(shape, FALSE);

		if ( theHold.Holding() ) {
			theHold.Put(new ShapeRestore(shapeData,this,shape));
			}

		int polys = shape->splineCount;
		if(GetSubobjectLevel() == ES_SPLINE) {
			BitArray &sel = shape->polySel.sel;
			if(clear)
				sel.ClearAll();
			for(int poly = 0; poly < polys; ++poly) {
				Spline3D *spline = shape->splines[poly];
				for(int seg = 0; seg < spline->Segments(); ++seg) {
					if(spline->GetMatID(seg) != index)
						goto next_spline;
					}
				sel.Set(poly);
next_spline:;
				}
			}
		else {
			for(int poly = 0; poly < polys; ++poly) {
				Spline3D *spline = shape->splines[poly];
				BitArray &sel = shape->segSel[poly];
				if(clear)
					sel.ClearAll();
				for(int seg = 0; seg < spline->Segments(); ++seg) {
					if(spline->GetMatID(seg) == index)
						sel.Set(seg);
					}
				}
			}

		if(altered) {
			shapeData->UpdateChanges(shape, FALSE);
			shapeData->TempData(this)->Invalidate(PART_TOPO);
			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	
	theHold.Accept(GetString(IDS_RB_SELECTBYMATID));

	nodes.DisposeTemporary();
	SelectionChanged();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

// Return TRUE if this modifier is just working on one object
BOOL EditSplineMod::SingleObjectMod() {
	ModContextList mcList;		
	INodeTab nodes;
	if ( !ip ) return FALSE;
	ip->GetModContexts(mcList,nodes);
	nodes.DisposeTemporary();
	return (mcList.Count() == 1) ? TRUE : FALSE;
	}

BezierShape *EditSplineMod::SingleObjectShape(INode **node) {
	ModContextList mcList;		
	INodeTab nodes;
	if ( !ip ) return NULL;
	ip->GetModContexts(mcList,nodes);
	if(mcList.Count() != 1) {
		nodes.DisposeTemporary();
		return NULL;
		}
	EditSplineData *shapeData = (EditSplineData*)mcList[0]->localData;
	if ( !shapeData ) {
		nodes.DisposeTemporary();
		return NULL;
		}
	if(node)
		*node = nodes[0];
	nodes.DisposeTemporary();
	return shapeData->TempData(this)->GetShape(ip->GetTime());
	}

//---------------------------------------------------------------------
// UI stuff

static HIMAGELIST hBoolImages = NULL;

static void LoadBoolImages()
	{
	if ( !hBoolImages ) {
		HBITMAP hBitmap, hMask;
		hBoolImages = ImageList_Create(16, 15, TRUE, 6, 0);
		hBitmap     = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BOOLEANTYPES));
		hMask       = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_MASK_BOOLEANTYPES));
		ImageList_Add(hBoolImages,hBitmap,hMask);
		DeleteObject(hBitmap);
		DeleteObject(hMask);
		}
	}	

void EditSplineMod::SetBooleanButton() {
	iUnion->SetCheck((boolType == BOOL_UNION) ? TRUE : FALSE);
	iSubtraction->SetCheck((boolType == BOOL_SUBTRACTION) ? TRUE : FALSE);
	iIntersection->SetCheck((boolType == BOOL_INTERSECTION) ? TRUE : FALSE);
	}

static HIMAGELIST hMirrorImages = NULL;

static void LoadMirrorImages()
	{
	if ( !hMirrorImages ) {
		HBITMAP hBitmap, hMask;
		hMirrorImages = ImageList_Create(16, 15, TRUE, 6, 0);
		hBitmap     = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_MIRRORTYPES));
		hMask       = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_MASK_MIRRORTYPES));
		ImageList_Add(hMirrorImages,hBitmap,hMask);
		DeleteObject(hBitmap);
		DeleteObject(hMask);
		}
	}	

class ESImageListDestroyer {
	~ESImageListDestroyer() {
		if(hSplineImages)
			ImageList_Destroy(hSplineImages);
		if(hBoolImages)
			ImageList_Destroy(hBoolImages);
		if(hMirrorImages)
			ImageList_Destroy(hMirrorImages);
		}
	};

void EditSplineMod::SetMirrorButton() {
	iMirrorHorizontal->SetCheck((mirrorType == MIRROR_HORIZONTAL) ? TRUE : FALSE);
	iMirrorVertical->SetCheck((mirrorType == MIRROR_VERTICAL) ? TRUE : FALSE);
	iMirrorBoth->SetCheck((mirrorType == MIRROR_BOTH) ? TRUE : FALSE);
	}

// Dialog proc for "Select By" floater

static void EnableSelectByButtons(HWND hDlg, EditSplineMod *es) {
	int level = es->GetSubobjectLevel();
	EnableWindow (GetDlgItem (hDlg, IDC_SEGMENT), level == ES_VERTEX ? TRUE : FALSE);
	EnableWindow (GetDlgItem (hDlg, IDC_SPLINE), (level == ES_VERTEX || level == ES_SEGMENT) ? TRUE : FALSE);
	}

static INT_PTR CALLBACK SelectByDlgProc(
		HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// WIN64 Cleanup: Shuler
	EditSplineMod *es = (EditSplineMod *)GetWindowLongPtr( hDlg, GWLP_USERDATA );
	switch (msg) {
		case WM_INITDIALOG:
		 	es = (EditSplineMod *)lParam;
		 	SetWindowLongPtr( hDlg, GWLP_USERDATA, (LONG_PTR)es );
			es->hSelectBy = hDlg;
			EnableSelectByButtons(hDlg, es);
			CenterWindow(hDlg,GetParent(hDlg));
			break;

		case MSG_SUBOBJ_MODE_CHANGE:
			EnableSelectByButtons(hDlg, es);
			break;

		case WM_CLOSE:
			es->hSelectBy = NULL;
			EndDialog(hDlg, 0);
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_SEGMENT:
					es->SelectBySegment();
					break;
				case IDC_SPLINE:
					es->SelectBySpline();
					break;
				}
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}

// Dialog proc for "Select" rollup

INT_PTR CALLBACK SplineSelectDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
	{
	EditSplineMod *es = (EditSplineMod *)GetWindowLongPtr( hDlg, GWLP_USERDATA );
	ICustToolbar *iToolbar;
	if ( !es && message != WM_INITDIALOG ) return FALSE;
	
	switch ( message ) {
		case WM_INITDIALOG: {
		 	es = (EditSplineMod *)lParam;
		 	SetWindowLongPtr( hDlg, GWLP_USERDATA, (LONG_PTR)es );		 	
		 	es->hSelectPanel = hDlg;
			// Set up the editing level selector
			LoadESImages();
			iToolbar = GetICustToolbar(GetDlgItem(hDlg,IDC_SELTYPE));
			iToolbar->SetImage(hSplineImages);
			iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,0,3,0,3,24,23,24,23,ES_VERTEX));
			iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,1,4,1,4,24,23,24,23,ES_SEGMENT));
			iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,2,5,2,5,24,23,24,23,ES_SPLINE));
			ReleaseICustToolbar(iToolbar);
			es->RefreshSelType();
			CheckDlgButton( hDlg, IDC_LOCK_HANDLES, lockedHandles);
			CheckDlgButton( hDlg, IDC_SEGMENT_END, segmentEnd);
			CheckRadioButton( hDlg, IDC_LOCKALIKE, IDC_LOCKALL, lockType);
			CheckDlgButton( hDlg, IDC_SHOW_VERTEX_NUMBERS, es->showVertNumbers);
			CheckDlgButton( hDlg, IDC_SVN_SELECTED, es->SVNSelectedOnly);
			es->SetSelDlgEnables();

//2-1-99 watje
		 	es->selectAreaSpin = GetISpinner(GetDlgItem(hDlg,IDC_SELECTAREA_SPIN));
			es->selectAreaSpin->SetLimits( 0.0f, 999999.0f, FALSE );
			es->selectAreaSpin->LinkToEdit( GetDlgItem(hDlg,IDC_SELECTAREA), EDITTYPE_UNIVERSE );
			es->selectAreaSpin->SetValue(es->areaSelect,FALSE);
			CheckDlgButton( hDlg, IDC_AREA_SELECTION, es->useAreaSelect);

		 	return TRUE;
			}

		case WM_DESTROY:
//2-1-99 watje
			if ( es->selectAreaSpin ) {
				ReleaseISpinner(es->selectAreaSpin);
				es->selectAreaSpin = NULL;
				}

			// Don't leave in one of our modes!
			es->ip->ClearPickMode();
			CancelEditSplineModes(es->ip);
			return FALSE;
		
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:   			
   			es->ip->RollupMouseMessage(hDlg,message,wParam,lParam);
			return FALSE;		
//2-1-99 watje
		case CC_SPINNER_CHANGE:
			switch ( LOWORD(wParam) ) {
				case IDC_SELECTAREA_SPIN:
					es->areaSelect = es->selectAreaSpin->GetFVal();
					// CAL-03/24/01: update edge distances and redraw
					if ( es->mUseSoftSelections && es->mUseEdgeDists ) {
						es->UpdateEdgeDists();
						es->UpdateVertexWeights();
						es->NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
						es->ip->RedrawViews(es->ip->GetTime(),REDRAW_NORMAL);
					}
					break;
				}


		
		case WM_COMMAND: {
			BOOL needRedraw = FALSE;
			switch ( LOWORD(wParam) ) {				
				case ES_VERTEX:
					if (es->GetSubobjectLevel() == ES_VERTEX)
						es->ip->SetSubObjectLevel (ES_OBJECT);
					else
						es->ip->SetSubObjectLevel (ES_VERTEX);
					if (es->ip->GetShowEndResult())
						needRedraw = TRUE;

// Bug #202091 watje 8-31-99 no need to do a double notify 
//					needRedraw = TRUE;
					break;
				case ES_SEGMENT:
					if (es->GetSubobjectLevel() == ES_SEGMENT)
						es->ip->SetSubObjectLevel (ES_OBJECT);
					else
						es->ip->SetSubObjectLevel (ES_SEGMENT);
					if (es->ip->GetShowEndResult())
						needRedraw = TRUE;
// Bug #202091 watje 8-31-99 no need to do a double notify 
//					needRedraw = TRUE;
					break;
				case ES_SPLINE:
					if (es->GetSubobjectLevel() == ES_SPLINE)
						es->ip->SetSubObjectLevel (ES_OBJECT);
					else
						es->ip->SetSubObjectLevel (ES_SPLINE);
					if (es->ip->GetShowEndResult())
						needRedraw = TRUE;
// Bug #202091 watje 8-31-99 no need to do a double notify 
//					needRedraw = TRUE;
					break;
				case IDC_LOCK_HANDLES:
					lockedHandles = IsDlgButtonChecked( hDlg, IDC_LOCK_HANDLES);
					break;
				case IDC_SEGMENT_END:
					segmentEnd = IsDlgButtonChecked( hDlg, IDC_SEGMENT_END);
					break;
				case IDC_SELECT_BY:
					if(!es->hSelectBy) {
						es->hSelectBy = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_SELECT_BY), es->ip->GetMAXHWnd(), SelectByDlgProc, (LPARAM)es);
						if(es->hSelectBy)
							ShowWindow(es->hSelectBy, SW_SHOWNORMAL);
						}
					break;
				case IDC_SHOW_VERTEX_NUMBERS:
					es->showVertNumbers = IsDlgButtonChecked( hDlg, IDC_SHOW_VERTEX_NUMBERS);
					EnableWindow (GetDlgItem (hDlg, IDC_SVN_SELECTED), es->showVertNumbers);
					needRedraw = TRUE;
					break;
				case IDC_SVN_SELECTED:
					es->SVNSelectedOnly = IsDlgButtonChecked( hDlg, IDC_SVN_SELECTED);
					needRedraw = TRUE;
					break;
				case IDC_LOCKALIKE:
				case IDC_LOCKALL:
					lockType = LOWORD(wParam);
					break;
				case IDC_NS_COPY:
					es->NSCopy();
					break;
				case IDC_NS_PASTE:
					es->NSPaste();
					break;
//2-1-99 watje
				case IDC_AREA_SELECTION:
					es->useAreaSelect = IsDlgButtonChecked( hDlg, IDC_AREA_SELECTION);
					// CAL-03/24/01: update edge distances and redraw
					if ( es->mUseSoftSelections && es->mUseEdgeDists ) {
						es->UpdateEdgeDists();
						es->UpdateVertexWeights();
						needRedraw = TRUE;
					}
					break;

				}
			if(needRedraw) {
				es->NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
				es->ip->RedrawViews(es->ip->GetTime(),REDRAW_NORMAL);
				}
			}
			break;
		case WM_NOTIFY:
			if(((LPNMHDR)lParam)->code == TTN_NEEDTEXT) {
				LPTOOLTIPTEXT lpttt;
				lpttt = (LPTOOLTIPTEXT)lParam;				
				switch (lpttt->hdr.idFrom) {
				case ES_VERTEX:
					lpttt->lpszText = GetString (IDS_TH_VERTEX);
					break;
				case ES_SEGMENT:
					lpttt->lpszText = GetString (IDS_TH_SEGMENT);
					break;
				case ES_SPLINE:
					lpttt->lpszText = GetString(IDS_TH_SPLINE);
					break;
				}
			}
			break;
		default:
			hackInvalidateSelUI(es->hSelectPanel);
			return FALSE;
		}
	
	return FALSE;
	}

// Multiple Attachment handler

class AttachHitByName : public HitByNameDlgCallback {
public:
	EditSplineMod *es;

	AttachHitByName(EditSplineMod *s) {es=s;}
	TCHAR *dialogTitle()	{return GetString(IDS_TH_MULTIATTACH);}
	TCHAR *buttonText() 	{return GetString(IDS_TH_ATTACH);}
	int filter(INode *node);	
	void proc(INodeTab &nodeTab);	
};

int AttachHitByName::filter(INode *node)
	{
	ModContextList mcList;		
	INodeTab nodes;
	if (node) {
		// Make sure the node does not depend on this modifier.
		node->BeginDependencyTest();
		es->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
		if (node->EndDependencyTest()) return FALSE;

		ObjectState os =
			node->GetObjectRef()->Eval(es->ip->GetTime());
		GeomObject *object = (GeomObject *)os.obj;
		// Make sure it isn't one of the nodes we're editing, for heaven's sake!
		es->ip->GetModContexts(mcList,nodes);
		int numNodes = nodes.Count();
		for(int i = 0; i < numNodes; ++i) {
			if(nodes[i] == node) {
				nodes.DisposeTemporary();
				return FALSE;
				}
			}
		nodes.DisposeTemporary();
		if(object->CanConvertToType(splineShapeClassID))
			return TRUE;
		}
	return FALSE;
	}


void AttachHitByName::proc(INodeTab &nodeTab)
	{
	if (!es->ip) return;
	ModContextList mcList;
	INodeTab nodes;
	es->ip->GetModContexts(mcList, nodes);
	BOOL ret = TRUE;
	Mtl *firstMtl = nodes[0]->GetMtl();
	for (int i=0; i<nodeTab.Count(); i++) {
		if(firstMtl) {
			if (nodeTab[i]->GetMtl() && (firstMtl != nodeTab[i]->GetMtl()))
				break;
			}
		else
			firstMtl = nodeTab[i]->GetMtl();
		}
	if (i<nodeTab.Count())
		ret = DoAttachMatOptionDialog(es->ip, es);
	nodes.DisposeTemporary();
	if (!ret)
		return;
	es->MultiAttachObject(nodeTab);
	}


// Dialog proc for "Operations" rollup

INT_PTR CALLBACK SplineOpsDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
	{
	UINT uiVal;
	float fVal;
	ICustButton *but;
	ISpinnerControl *spin;
	EditSplineMod *es = (EditSplineMod *)GetWindowLongPtr( hDlg, GWLP_USERDATA );
	if ( !es && message != WM_INITDIALOG ) return FALSE;
	
	switch ( message ) {
		case WM_INITDIALOG: {
		 	es = (EditSplineMod *)lParam;
		 	SetWindowLongPtr( hDlg, GWLP_USERDATA, (LONG_PTR)es );		 	
			es->hOpsPanel = hDlg;
			but = GetICustButton(GetDlgItem(hDlg,IDC_ES_ATTACH));
			but->SetHighlightColor(GREEN_WASH);
			but->SetType(CBT_CHECK);
			ReleaseICustButton(but);
			CheckDlgButton( hDlg, IDC_ES_ATTACHREORIENT, attachReorient);
			but = GetICustButton(GetDlgItem(hDlg,IDC_ES_CREATELINE));
			but->SetHighlightColor(GREEN_WASH);
			but->SetType(CBT_CHECK);
			ReleaseICustButton(but);
			// CAL-05/23/03: Add Connecting Splines when Shift-Move Copy. (FID #827)
			CheckDlgButton( hDlg, IDC_ES_CONNECT_COPY, es->connectCopy);
			es->pConnectCopyThreshSpinner = GetISpinner(GetDlgItem(hDlg, IDC_ES_CONNECT_COPY_THRESHSPINNER));
			es->pConnectCopyThreshSpinner->SetLimits( 0, 999999, FALSE );
			es->pConnectCopyThreshSpinner->LinkToEdit( GetDlgItem(hDlg, IDC_ES_CONNECT_COPY_THRESH), EDITTYPE_POS_UNIVERSE );
			es->pConnectCopyThreshSpinner->SetValue(es->connectCopyThreshold, FALSE);
			// CAL-02/26/03: Add Cross Section. (FID #827)
			but = GetICustButton(GetDlgItem(hDlg,IDC_ES_CROSS_SECTION));
			but->SetHighlightColor(GREEN_WASH);
			but->SetType(CBT_CHECK);
			ReleaseICustButton(but);
			CheckDlgButton( hDlg, IDC_ES_KTYPE_LINEAR, es->newKnotType == KTYPE_CORNER ? TRUE : FALSE);
			CheckDlgButton( hDlg, IDC_ES_KTYPE_SMOOTH, es->newKnotType == KTYPE_AUTO ? TRUE : FALSE);
			CheckDlgButton( hDlg, IDC_ES_KTYPE_BEZIER, es->newKnotType == KTYPE_BEZIER ? TRUE : FALSE);
			CheckDlgButton( hDlg, IDC_ES_KTYPE_BEZIER_CORNER, es->newKnotType == KTYPE_BEZIER_CORNER ? TRUE : FALSE);
		 	LoadBoolImages();
		 	LoadMirrorImages();
		 	es->outlineSpin = GetISpinner(GetDlgItem(hDlg,IDC_ES_OUTLINESPINNER));
			es->outlineSpin->SetLimits( -999999, 999999, FALSE );
			es->outlineSpin->LinkToEdit( GetDlgItem(hDlg,IDC_ES_OUTLINEWIDTH), EDITTYPE_UNIVERSE );
		 	es->filletSpin = GetISpinner(GetDlgItem(hDlg,IDC_ES_FILLETSPINNER));
			es->filletSpin->SetLimits( 0, 999999, FALSE );
			es->filletSpin->LinkToEdit( GetDlgItem(hDlg,IDC_ES_FILLETWIDTH), EDITTYPE_UNIVERSE );
		 	es->chamferSpin = GetISpinner(GetDlgItem(hDlg,IDC_ES_CHAMFERSPINNER));
			es->chamferSpin->SetLimits( 0, 999999, FALSE );
			es->chamferSpin->LinkToEdit( GetDlgItem(hDlg,IDC_ES_CHAMFERWIDTH), EDITTYPE_UNIVERSE );
			CheckDlgButton( hDlg, IDC_ES_OUTCENTER, centeredOutline);
			but = GetICustButton(GetDlgItem(hDlg,IDC_ES_OUTLINE));
			but->SetHighlightColor(GREEN_WASH);
			but->SetType(CBT_CHECK);
			ReleaseICustButton(but);
			but = GetICustButton(GetDlgItem(hDlg,IDC_ES_FILLET));
			but->SetHighlightColor(GREEN_WASH);
			but->SetType(CBT_CHECK);
			ReleaseICustButton(but);
			but = GetICustButton(GetDlgItem(hDlg,IDC_ES_CHAMFER));
			but->SetHighlightColor(GREEN_WASH);
			but->SetType(CBT_CHECK);
			ReleaseICustButton(but);
			but = GetICustButton(GetDlgItem(hDlg,IDC_ES_BOOLEAN));
			but->SetHighlightColor(GREEN_WASH);
			but->SetType(CBT_CHECK);
			ReleaseICustButton(but);
			but = GetICustButton(GetDlgItem(hDlg,IDC_ES_TRIM));
			but->SetHighlightColor(GREEN_WASH);
			but->SetType(CBT_CHECK);
			ReleaseICustButton(but);
			but = GetICustButton(GetDlgItem(hDlg,IDC_ES_EXTEND));
			but->SetHighlightColor(GREEN_WASH);
			but->SetType(CBT_CHECK);
			ReleaseICustButton(but);
			but = GetICustButton(GetDlgItem(hDlg,IDC_ES_INSERT));
			but->SetHighlightColor(GREEN_WASH);
			but->SetType(CBT_CHECK);
			ReleaseICustButton(but);
		 	ICustToolbar *iToolbar = GetICustToolbar(GetDlgItem(hDlg,IDC_ES_BOOL_TYPE));
			iToolbar->SetImage(hBoolImages);
			iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,0,0,3,3,16,15,24,23,BOOL_UNION));
			iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,1,1,4,4,16,15,24,23,BOOL_SUBTRACTION));
			iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,2,2,5,5,16,15,24,23,BOOL_INTERSECTION));
			es->iUnion        = iToolbar->GetICustButton(BOOL_UNION);
			es->iSubtraction  = iToolbar->GetICustButton(BOOL_SUBTRACTION);
			es->iIntersection = iToolbar->GetICustButton(BOOL_INTERSECTION);
			ReleaseICustToolbar(iToolbar);
			es->SetBooleanButton();
		 	iToolbar = GetICustToolbar(GetDlgItem(hDlg,IDC_ES_MIRROR_TYPE));
			iToolbar->SetImage(hMirrorImages);
			iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,0,0,3,3,16,15,24,23,MIRROR_HORIZONTAL));
			iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,1,1,4,4,16,15,24,23,MIRROR_VERTICAL));
			iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,2,2,5,5,16,15,24,23,MIRROR_BOTH));
			es->iMirrorHorizontal = iToolbar->GetICustButton(MIRROR_HORIZONTAL);
			es->iMirrorVertical   = iToolbar->GetICustButton(MIRROR_VERTICAL);
			es->iMirrorBoth       = iToolbar->GetICustButton(MIRROR_BOTH);
			ReleaseICustToolbar(iToolbar);
			es->SetMirrorButton();
			CheckDlgButton( hDlg, IDC_TO_SPLINES, explodeToObjects ? FALSE : TRUE);
			CheckDlgButton( hDlg, IDC_TO_OBJECTS, explodeToObjects ? TRUE : FALSE);
			CheckDlgButton( hDlg, IDC_ES_COPY_MIRROR, polyMirrorCopy);
			CheckDlgButton( hDlg, IDC_ES_ABOUT_PIVOT, mirrorAboutPivot);
			CheckDlgButton( hDlg, IDC_ES_TRIM_INFINITE, trimInfinite);
			CheckDlgButton( hDlg, IDC_ES_SAMESHAPE, detachSameShape);
			CheckDlgButton( hDlg, IDC_ES_DETACHCOPY, detachCopy);
			CheckDlgButton( hDlg, IDC_ES_DETACHREORIENT, detachReorient);
			EnableWindow(GetDlgItem(hDlg,IDC_ES_DETACHREORIENT),!detachSameShape);
		 	es->divSpin = GetISpinner(GetDlgItem(hDlg,IDC_ES_DIVSSPINNER));
			es->divSpin->SetLimits( 1, 100, FALSE );
			es->divSpin->LinkToEdit( GetDlgItem(hDlg,IDC_ES_DIVISIONS), EDITTYPE_POS_INT );
			es->divSpin->SetValue(segDivisions, FALSE);
			es->divSpin->SetAutoScale();
			but = GetICustButton(GetDlgItem(hDlg,IDC_ES_REFINE));
			but->SetHighlightColor(GREEN_WASH);
			but->SetType(CBT_CHECK);
			ReleaseICustButton(but);
			but = GetICustButton(GetDlgItem(hDlg,IDC_ES_CROSS_INSERT));
			but->SetHighlightColor(GREEN_WASH);
			but->SetType(CBT_CHECK);
			ReleaseICustButton(but);
			but = GetICustButton(GetDlgItem(hDlg,IDC_ES_INSERT));
			but->SetHighlightColor(GREEN_WASH);
			but->SetType(CBT_CHECK);
			ReleaseICustButton(but);
			but = GetICustButton(GetDlgItem(hDlg,IDC_ES_CONNECT));
			but->SetHighlightColor(GREEN_WASH);
			but->SetType(CBT_CHECK);
			ReleaseICustButton(but);
		 	es->weldSpin = GetISpinner(GetDlgItem(hDlg,IDC_ES_THRESHSPINNER));
			es->weldSpin->SetLimits( 0, 999999, FALSE );
			es->weldSpin->LinkToEdit( GetDlgItem(hDlg,IDC_ES_WELDTHRESH), EDITTYPE_UNIVERSE );
			es->weldSpin->SetValue(weldThreshold,FALSE);
		 	es->crossSpin = GetISpinner(GetDlgItem(hDlg,IDC_ES_CROSSTHRESHSPINNER));
			es->crossSpin->SetLimits( 0, 999999, FALSE );
			es->crossSpin->LinkToEdit( GetDlgItem(hDlg,IDC_ES_CROSSTHRESH), EDITTYPE_UNIVERSE );
			es->crossSpin->SetValue(crossThreshold,FALSE);

		 	es->pEndPointAutoConnectWeldSpinner = GetISpinner(GetDlgItem(hDlg,IDC_ES_ENDPOINTAUTOWELD_THRESHSPINNER));
			es->pEndPointAutoConnectWeldSpinner->SetLimits( 0, 999999, FALSE );
			es->pEndPointAutoConnectWeldSpinner->LinkToEdit( GetDlgItem(hDlg,IDC_ES_EndPointAutoWeldThreshold), EDITTYPE_UNIVERSE );
			es->pEndPointAutoConnectWeldSpinner->SetValue(SplineShape::GetEndPointAutoWeldThreshold(),FALSE);

//watje
			es->rConnect = IsDlgButtonChecked( hDlg, IDC_ES_RCONNECT);
			es->smoothRefineConnect = !IsDlgButtonChecked( hDlg, IDC_ES_RCLINEAR);
			es->closedRefineConnect = IsDlgButtonChecked( hDlg, IDC_ES_RCCLOSED);
			es->bindFirst = IsDlgButtonChecked( hDlg, IDC_ES_BINDFIRST);
			es->bindLast = IsDlgButtonChecked( hDlg, IDC_ES_BINDLAST);
			but = GetICustButton(GetDlgItem(hDlg,IDC_ES_BIND));
			but->SetHighlightColor(GREEN_WASH);
			but->SetType(CBT_CHECK);
			ReleaseICustButton(but);
/*2-1-99 watje
			but = GetICustButton(GetDlgItem(hDlg,IDC_ES_REFINECONNECT));
			but->SetHighlightColor(GREEN_WASH);
			but->SetType(CBT_CHECK);
			ReleaseICustButton(but);
*/
			// CAL-03/03/03: copy/paste tangent. (FID #827)
			but = GetICustButton(GetDlgItem(hDlg,IDC_ES_COPY_TANGENT));
			but->SetHighlightColor(GREEN_WASH);
			but->SetType(CBT_CHECK);
			ReleaseICustButton(but);
			but = GetICustButton(GetDlgItem(hDlg,IDC_ES_PASTE_TANGENT));
			but->SetHighlightColor(GREEN_WASH);
			but->SetType(CBT_CHECK);
			ReleaseICustButton(but);
			CheckDlgButton(hDlg, IDC_ES_COPY_TAN_LENGTH, es->copyTanLength);

			but = GetICustButton(GetDlgItem(hDlg,IDC_ES_CYCLE));
			but->SetHighlightColor(GREEN_WASH);
			but->SetType(CBT_PUSH);
			ReleaseICustButton(but);

//6-16-99 watje
			CheckDlgButton( hDlg, IDC_ES_SHOW_SELECTED, showSelected);
			CheckDlgButton( hDlg, IDC_ES_EndPointAutoWeldControl, 
				SplineShape::GetUseEndPointAutoConnect());

		 	es->SetOpsDlgEnables();
			return TRUE;
			}

		case WM_DESTROY:
			if ( es->weldSpin ) {
				ReleaseISpinner(es->weldSpin);
				es->weldSpin = NULL;
				}
			if ( es->crossSpin ) {
				ReleaseISpinner(es->crossSpin);
				es->crossSpin = NULL;
				}
			if ( es->divSpin ) {
				ReleaseISpinner(es->divSpin);
				es->divSpin = NULL;
				}
			if ( es->outlineSpin ) {
				ReleaseISpinner(es->outlineSpin);
				es->outlineSpin = NULL;
				}
			if ( es->filletSpin ) {
				ReleaseISpinner(es->filletSpin);
				es->filletSpin = NULL;
				}
			if ( es->chamferSpin ) {
				ReleaseISpinner(es->chamferSpin);
				es->chamferSpin = NULL;
				}
 			if ( es->iUnion ) {
				ReleaseICustButton(es->iUnion);
				es->iUnion = NULL;
				}
 			if ( es->iSubtraction ) {
				ReleaseICustButton(es->iSubtraction);
				es->iSubtraction = NULL;
				}
 			if ( es->iIntersection ) {
				ReleaseICustButton(es->iIntersection);
				es->iIntersection = NULL;
				}
 			if ( es->iMirrorHorizontal ) {
				ReleaseICustButton(es->iMirrorHorizontal);
				es->iMirrorHorizontal = NULL;
				}
 			if ( es->iMirrorVertical ) {
				ReleaseICustButton(es->iMirrorVertical);
				es->iMirrorVertical = NULL;
				}
 			if ( es->iMirrorBoth ) {
				ReleaseICustButton(es->iMirrorBoth);
				es->iMirrorBoth = NULL;
				}
			// Don't leave in one of our modes!
			es->ip->DeleteMode(es->vertConnectMode);
			es->ip->DeleteMode(es->vertInsertMode);
			es->ip->DeleteMode(es->segRefineMode);
			es->ip->DeleteMode(es->crossInsertMode);
			es->ip->DeleteMode(es->segBreakMode);
			es->ip->DeleteMode(es->outlineMode);
			es->ip->DeleteMode(es->filletMode);
			es->ip->DeleteMode(es->chamferMode);
			es->ip->DeleteMode(es->booleanMode);
			es->ip->DeleteMode(es->createLineMode);
			es->ip->DeleteMode(es->crossSectionMode);

//watje
			es->ip->DeleteMode(es->refineConnectMode);
			es->ip->DeleteMode(es->bindMode);
			// CAL-03/03/03: copy/paste tangent. (FID #827)
			es->ip->DeleteMode(es->copyTangentMode);
			es->ip->DeleteMode(es->pasteTangentMode);

			es->ip->ClearPickMode();
			CancelEditSplineModes(es->ip);
			// Detach our right-menu and delete stuff!
			es->ip->GetRightClickMenuManager()->Unregister(&esMenu);
            es->ip->GetActionManager()->DeactivateActionTable(&sActionCallback, kSplineActions);
			es->ip->UnRegisterDeleteUser(&esDel);
			return FALSE;
		
		case CC_SPINNER_CHANGE:
			switch ( LOWORD(wParam) ) {
				case IDC_ES_OUTLINESPINNER: {
					float outSize = es->outlineSpin->GetFVal();
					if ( !es->inOutline ) {
						es->MaybeSelectSingleSpline(TRUE);
						if(es->AnyPolysSelected())
							es->BeginOutlineMove(es->ip->GetTime());
						}
					if(es->inOutline) {
						es->OutlineMove( es->ip->GetTime(), outSize );
						// If not being dragged, finish it now!
						if(!HIWORD(wParam))
							es->EndOutlineMove(es->ip->GetTime(), (outSize == 0.0f) ? FALSE : TRUE);
						es->ip->RedrawViews(es->ip->GetTime(),REDRAW_INTERACTIVE);					
						}
					}
					break;
				case IDC_ES_FILLETSPINNER: {
					float size = es->filletSpin->GetFVal();
					if ( !es->inFillet ) {
						es->SetFCLimit();
						es->filletSpin->SetLimits( 0.0f, es->FCLimit, FALSE );
						es->BeginFilletMove(es->ip->GetTime());
						}
					es->FilletMove( es->ip->GetTime(), size );
					// If not being dragged, finish it now!
					if(!HIWORD(wParam))
						es->EndFilletMove(es->ip->GetTime(), (size == 0.0f) ? FALSE : TRUE);
					es->ip->RedrawViews(es->ip->GetTime(),REDRAW_INTERACTIVE);					
					}
					break;
				case IDC_ES_CHAMFERSPINNER: {
					float size = es->chamferSpin->GetFVal();
					if ( !es->inChamfer ) {
						es->SetFCLimit();
						es->chamferSpin->SetLimits( 0.0f, es->FCLimit, FALSE );
						es->BeginChamferMove(es->ip->GetTime());
						}
					es->ChamferMove( es->ip->GetTime(), size );
					// If not being dragged, finish it now!
					if(!HIWORD(wParam))
						es->EndChamferMove(es->ip->GetTime(), (size == 0.0f) ? FALSE : TRUE);
					es->ip->RedrawViews(es->ip->GetTime(),REDRAW_INTERACTIVE);					
					}
					break;
				case IDC_ES_DIVSSPINNER:
					segDivisions = es->divSpin->GetIVal();
					break;
				case IDC_ES_THRESHSPINNER:
					weldThreshold = es->weldSpin->GetFVal();
					break;
				// CAL-05/23/03: Add Connecting Splines when Shift-Move Copy. (FID #827)
				case IDC_ES_CONNECT_COPY_THRESHSPINNER:
					es->connectCopyThreshold = es->pConnectCopyThreshSpinner->GetFVal();
					break;
				case IDC_ES_ENDPOINTAUTOWELD_THRESHSPINNER:
					 fVal = es->pEndPointAutoConnectWeldSpinner->GetFVal();
					 SplineShape::SetEndPointAutoWeldThreshold(fVal);
					break;
				case IDC_ES_CROSSTHRESHSPINNER:
					crossThreshold = es->crossSpin->GetFVal();
					break;
				}
			break;

		case CC_SPINNER_BUTTONUP:
			switch( LOWORD(wParam) ) {
				case IDC_ES_OUTLINESPINNER:
					if(es->inOutline) {
						float outSize = es->outlineSpin->GetFVal();
						es->EndOutlineMove(es->ip->GetTime(), (outSize == 0.0f) ? FALSE : TRUE);
						}
					break;
				case IDC_ES_FILLETSPINNER:
					if(es->inFillet) {
						float size = es->filletSpin->GetFVal();
						es->EndFilletMove(es->ip->GetTime(), (size == 0.0f) ? FALSE : TRUE);
						}
					break;
				case IDC_ES_CHAMFERSPINNER:
					if(es->inChamfer) {
						float size = es->chamferSpin->GetFVal();
						es->EndChamferMove(es->ip->GetTime(), (size == 0.0f) ? FALSE : TRUE);
						}
					break;
				}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:   			
   			es->ip->RollupMouseMessage(hDlg,message,wParam,lParam);
			return FALSE;		
		
		case WM_COMMAND:			
			switch ( LOWORD(wParam) ) {				

				case IDC_ES_EndPointAutoWeldControl:
					uiVal = IsDlgButtonChecked( hDlg, IDC_ES_EndPointAutoWeldControl);
					SplineShape::SetUseEndPointAutoConnect(uiVal);
					spin = GetISpinner(GetDlgItem(hDlg,IDC_ES_ENDPOINTAUTOWELD_THRESHSPINNER));
					spin->Enable(SplineShape::GetUseEndPointAutoConnect()==BST_CHECKED);
					ReleaseISpinner(spin);
					break;
//watje
				case IDC_ES_UNBIND:
					es->DoUnBind();
					break;
				case IDC_ES_BIND:
					es->StartBindMode();
					break;
/*2-1-99 watje
				case IDC_ES_REFINECONNECT:
					es->StartRefineConnectMode();
					break;
*/
				case IDC_ES_RCLINEAR:
					{
					es->smoothRefineConnect = !IsDlgButtonChecked( hDlg, IDC_ES_RCLINEAR);
					break;
					}
				case IDC_ES_RCCLOSED:
					{
					es->closedRefineConnect = IsDlgButtonChecked( hDlg, IDC_ES_RCCLOSED);
					break;
					}
				case IDC_ES_BINDFIRST:
					{
					es->bindFirst = IsDlgButtonChecked( hDlg, IDC_ES_BINDFIRST);
					break;
					}
				case IDC_ES_BINDLAST:
					{
					es->bindLast = IsDlgButtonChecked( hDlg, IDC_ES_BINDLAST);
					break;
					}

//2-1-99 watje
				case IDC_ES_RCONNECT:
					{
					es->rConnect = IsDlgButtonChecked( hDlg, IDC_ES_RCONNECT);
					if (es->rConnect)
						{
						EnableWindow (GetDlgItem (hDlg, IDC_ES_RCLINEAR), TRUE);
						EnableWindow (GetDlgItem (hDlg, IDC_ES_RCCLOSED), TRUE);
						EnableWindow (GetDlgItem (hDlg, IDC_ES_BINDFIRST), TRUE);
						EnableWindow (GetDlgItem (hDlg, IDC_ES_BINDLAST), TRUE);
						}
					else
						{
						EnableWindow (GetDlgItem (hDlg, IDC_ES_RCLINEAR), FALSE);
						EnableWindow (GetDlgItem (hDlg, IDC_ES_RCCLOSED), FALSE);
						EnableWindow (GetDlgItem (hDlg, IDC_ES_BINDFIRST), FALSE);
						EnableWindow (GetDlgItem (hDlg, IDC_ES_BINDLAST), FALSE);
						}
					break;
					}


				case IDC_ES_HIDE:
					es->DoHide();
					break;
				case IDC_ES_UNHIDE:
					es->DoUnhide();
					break;
				case IDC_ES_CYCLE:
					es->DoCycleVerts();
					break;
				case IDC_ES_SHOW_SELECTED:
					showSelected = IsDlgButtonChecked( hDlg, IDC_ES_SHOW_SELECTED);
					es->SelectionChanged();
					es->NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
					es->ip->RedrawViews(es->ip->GetTime(),REDRAW_INTERACTIVE);					
					break;


				case IDC_ES_INSERT:
					es->StartVertInsertMode(IDC_ES_INSERT);
					break;
				case IDC_ES_BREAK:
					if(es->GetSubobjectLevel() == ES_VERTEX)
						es->DoVertBreak();
					else
						es->StartSegBreakMode();
					break;
				case IDC_ES_CLOSE:
					es->DoPolyClose();
					break;
				case IDC_ES_DETACH:
					switch(es->GetSubobjectLevel()) {
						case ES_SEGMENT:
							es->DoSegDetach(detachSameShape, detachCopy, detachReorient);
							break;
						case ES_SPLINE:
							es->DoPolyDetach(detachCopy, detachReorient);
							break;
						}
					break;
				case IDC_ES_SAMESHAPE:
					detachSameShape = IsDlgButtonChecked( hDlg, IDC_ES_SAMESHAPE);
					EnableWindow(GetDlgItem(hDlg,IDC_ES_DETACHREORIENT),!detachSameShape);
					break;
				case IDC_ES_DETACHCOPY:
					detachCopy = IsDlgButtonChecked( hDlg, IDC_ES_DETACHCOPY);
					break;
				case IDC_ES_DETACHREORIENT:
					detachReorient = IsDlgButtonChecked( hDlg, IDC_ES_DETACHREORIENT);
					break;
				case IDC_ES_SEGDIVIDE:
					es->DoSegDivide(segDivisions);
					break;
				case IDC_ES_OUTCENTER:
					centeredOutline = IsDlgButtonChecked( hDlg, IDC_ES_OUTCENTER);
					if ( es->inOutline ) {
						es->OutlineMove( es->ip->GetTime(),es->outlineSpin->GetFVal() );
						es->ip->RedrawViews(es->ip->GetTime(),REDRAW_INTERACTIVE);					
						}
					break;
				case IDC_ES_OUTLINE:
					es->StartOutlineMode();
					break;
				case IDC_ES_FILLET:
					es->StartFilletMode();
					break;
				case IDC_ES_CHAMFER:
					es->StartChamferMode();
					break;
				case IDC_ES_BOOLEAN: {
					if(!es->BooleanStartUp()) {
						ICustButton *but = GetICustButton(GetDlgItem(hDlg,IDC_ES_BOOLEAN));
						but->SetCheck(FALSE);
						ReleaseICustButton(but);
						CancelEditSplineModes(es->ip);
						}
					break;
					}
				case BOOL_UNION:
				case BOOL_SUBTRACTION:
				case BOOL_INTERSECTION:
					es->SetBoolOperation(LOWORD(wParam));
					es->SetBooleanButton();
					break;
				case IDC_TO_SPLINES:
					explodeToObjects = FALSE;
					CheckDlgButton( hDlg, IDC_TO_SPLINES, TRUE);
					CheckDlgButton( hDlg, IDC_TO_OBJECTS, FALSE);
					break;
				case IDC_TO_OBJECTS:
					explodeToObjects = TRUE;
					CheckDlgButton( hDlg, IDC_TO_SPLINES, FALSE);
					CheckDlgButton( hDlg, IDC_TO_OBJECTS, TRUE);
					break;
				case IDC_ES_MIRROR:
					es->DoPolyMirror(es->mirrorType,polyMirrorCopy, mirrorAboutPivot);
					break;
				case IDC_ES_COPY_MIRROR:
					polyMirrorCopy = IsDlgButtonChecked( hDlg, IDC_ES_COPY_MIRROR);
					break;
				case IDC_ES_ABOUT_PIVOT:
					mirrorAboutPivot = IsDlgButtonChecked( hDlg, IDC_ES_ABOUT_PIVOT);
					break;
				case IDC_ES_TRIM:
					es->StartTrimMode();
					break;
				case IDC_ES_EXTEND:
					es->StartExtendMode();
					break;
				case IDC_ES_TRIM_INFINITE:
					trimInfinite = IsDlgButtonChecked( hDlg, IDC_ES_TRIM_INFINITE);
					break;
				case IDC_ES_REVERSE:
					es->DoPolyReverse();
					break;
				case IDC_ES_EXPLODE:
					if(explodeToObjects)
						es->DoExplodeToObjects();
					else
						es->DoPolyExplode();
					break;
				case MIRROR_HORIZONTAL:
				case MIRROR_VERTICAL:
				case MIRROR_BOTH:
					es->SetMirrorOperation(LOWORD(wParam));
					es->SetMirrorButton();
					break;
				case IDC_ES_ATTACH: {
					// If the mode is on, turn it off and bail
					if (es->ip->GetCommandMode()->ID() == CID_STDPICK) {
						es->ip->SetStdCommandMode(CID_OBJMOVE);
						return FALSE;
						}
					es->pickCB.es = es;
					es->ip->SetPickMode(&es->pickCB);
					break;
					}
				case IDC_ES_ATTACH_MULTIPLE: {
					AttachHitByName proc(es);
					es->ip->DoHitByNameDialog(&proc);
					break;
					}
				case IDC_ES_ATTACHREORIENT:
					attachReorient = IsDlgButtonChecked( hDlg, IDC_ES_ATTACHREORIENT);
					break;
				case IDC_ES_CREATELINE:
					es->StartCreateLineMode();
					break;
				// CAL-05/23/03: Add Connecting Splines when Shift-Move Copy. (FID #827)
				case IDC_ES_CONNECT_COPY:
					es->connectCopy = IsDlgButtonChecked(hDlg, IDC_ES_CONNECT_COPY);
					BOOL ssType;
					ssType = (es->GetSubobjectLevel() == ES_SEGMENT) || (es->GetSubobjectLevel() == ES_SPLINE);
					EnableWindow (GetDlgItem (hDlg, IDC_ES_DIST_THRESH_LABEL), ssType && es->connectCopy);
					spin = GetISpinner(GetDlgItem(hDlg, IDC_ES_CONNECT_COPY_THRESHSPINNER));
					spin->Enable(ssType && es->connectCopy);
					ReleaseISpinner(spin);
					break;
				// CAL-02/26/03: Add Cross Section. (FID #827)
				case IDC_ES_CROSS_SECTION:
					es->StartCrossSectionMode();
					break;
				case IDC_ES_KTYPE_LINEAR:
					es->newKnotType = KTYPE_CORNER;
					CheckDlgButton( hDlg, IDC_ES_KTYPE_LINEAR, TRUE);
					CheckDlgButton( hDlg, IDC_ES_KTYPE_SMOOTH, FALSE);
					CheckDlgButton( hDlg, IDC_ES_KTYPE_BEZIER, FALSE);
					CheckDlgButton( hDlg, IDC_ES_KTYPE_BEZIER_CORNER, FALSE);
					break;
				case IDC_ES_KTYPE_SMOOTH:
					es->newKnotType = KTYPE_AUTO;
					CheckDlgButton( hDlg, IDC_ES_KTYPE_LINEAR, FALSE);
					CheckDlgButton( hDlg, IDC_ES_KTYPE_SMOOTH, TRUE);
					CheckDlgButton( hDlg, IDC_ES_KTYPE_BEZIER, FALSE);
					CheckDlgButton( hDlg, IDC_ES_KTYPE_BEZIER_CORNER, FALSE);
					break;
				case IDC_ES_KTYPE_BEZIER:
					es->newKnotType = KTYPE_BEZIER;
					CheckDlgButton( hDlg, IDC_ES_KTYPE_LINEAR, FALSE);
					CheckDlgButton( hDlg, IDC_ES_KTYPE_SMOOTH, FALSE);
					CheckDlgButton( hDlg, IDC_ES_KTYPE_BEZIER, TRUE);
					CheckDlgButton( hDlg, IDC_ES_KTYPE_BEZIER_CORNER, FALSE);
					break;
				case IDC_ES_KTYPE_BEZIER_CORNER:
					es->newKnotType = KTYPE_BEZIER_CORNER;
					CheckDlgButton( hDlg, IDC_ES_KTYPE_LINEAR, FALSE);
					CheckDlgButton( hDlg, IDC_ES_KTYPE_SMOOTH, FALSE);
					CheckDlgButton( hDlg, IDC_ES_KTYPE_BEZIER, FALSE);
					CheckDlgButton( hDlg, IDC_ES_KTYPE_BEZIER_CORNER, TRUE);
					break;
//2-1-99 watje
				case IDC_ES_FUSE:
					es->DoVertFuse();
					break;
				case IDC_ES_REFINE:
					if (es->rConnect)
						es->StartRefineConnectMode();
					else es->StartSegRefineMode();
					break;

				// CAL-03/03/03: copy/paste tangent. (FID #827)
				case IDC_ES_COPY_TANGENT:
					es->StartCopyTangentMode();
					break;
				case IDC_ES_PASTE_TANGENT:
					es->StartPasteTangentMode();
					break;
				case IDC_ES_COPY_TAN_LENGTH:
					es->copyTanLength = IsDlgButtonChecked(hDlg, IDC_ES_COPY_TAN_LENGTH);
					break;

				case IDC_ES_CROSS_INSERT:
					es->StartCrossInsertMode();
					break;
				case IDC_ES_DELETE:
					switch(es->GetSubobjectLevel()) {
						case ES_VERTEX:
							es->DoVertDelete();
							break;
						case ES_SEGMENT:
							es->DoSegDelete();
							break;
						case ES_SPLINE:
							es->DoPolyDelete();
							break;
						}
					break;
				case IDC_ES_CONNECT:
					es->StartVertConnectMode();
					break;
				case IDC_ES_WELD:
					es->DoVertWeld();
					break;
				case IDC_ES_MAKEFIRST:
					es->DoMakeFirst();
					break;
				}
			break;

		case WM_NOTIFY:
			if(((LPNMHDR)lParam)->code == TTN_NEEDTEXT) {
				LPTOOLTIPTEXT lpttt;
				lpttt = (LPTOOLTIPTEXT)lParam;
				lpttt->lpszText = GetString(es->GetBoolMirrString(lpttt->hdr.idFrom));
				}
			break;
		default:
			hackInvalidateOpsUI(es->hOpsPanel);
			return FALSE;

		}
	
	return FALSE;
	}

// Dialog proc for "Surface Properties" rollup

INT_PTR CALLBACK SplineSurfDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
	{
	EditSplineMod *es = (EditSplineMod *)GetWindowLongPtr( hDlg, GWLP_USERDATA );
	if ( !es && message != WM_INITDIALOG ) return FALSE;
	
	switch ( message ) {
		case WM_INITDIALOG: {
		 	es = (EditSplineMod *)lParam;
		 	SetWindowLongPtr( hDlg, GWLP_USERDATA, (LONG_PTR)es );		 	
			es->hSurfPanel = hDlg;
			es->matSpin = SetupIntSpinner(hDlg,IDC_MAT_IDSPIN,IDC_MAT_ID,1,MAX_MATID,0);
		 	es->matSpinSel = SetupIntSpinner (hDlg, IDC_MAT_IDSPIN_SEL, IDC_MAT_ID_SEL, 1, MAX_MATID, 0);  
			CheckDlgButton(hDlg, IDC_CLEARSELECTION, 1);                                 
			SetupMtlSubNameCombo (hDlg, es);          
			es->SetSurfDlgEnables();
			return TRUE;
			}

		case WM_DESTROY:
			if( es->matSpin ) {
				ReleaseISpinner(es->matSpin);
				es->matSpin = NULL;
				}
			if( es->matSpinSel ) {                 
				ReleaseISpinner(es->matSpinSel);
				es->matSpinSel = NULL;
				}
			return FALSE;
		
		case CC_SPINNER_CHANGE:
			switch ( LOWORD(wParam) ) {
				case IDC_MAT_IDSPIN: 
					if(HIWORD(wParam))
						break;		// No interactive action
					es->SetSelMatIndex(es->matSpin->GetIVal()-1);
					break;
				}
			break;

		case CC_SPINNER_BUTTONUP:
			switch( LOWORD(wParam) ) {
				case IDC_MAT_IDSPIN:
					es->SetSelMatIndex(es->matSpin->GetIVal()-1);
					es->ip->RedrawViews(es->ip->GetTime(),REDRAW_END);
					break;
				}
			break;

		case WM_PAINT:
			if (!es->segUIValid) {
				// Material index
				int mat = es->GetSelMatIndex();
				if (mat == -1) {
					es->matSpin->SetIndeterminate(TRUE);
					es->matSpinSel->SetIndeterminate(TRUE);    
				} else {
					es->matSpin->SetIndeterminate(FALSE);
					es->matSpin->SetValue(mat+1,FALSE);
					es->matSpinSel->SetIndeterminate(FALSE);       
					es->matSpinSel->SetValue(mat+1,FALSE);         
					}
				if (GetDlgItem (hDlg, IDC_MTLID_NAMES_COMBO)) {    
					ValidateUINameCombo(hDlg, es);                 
				}   

				es->segUIValid = TRUE;
				}
			return FALSE;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:   			
   			es->ip->RollupMouseMessage(hDlg,message,wParam,lParam);
			return FALSE;		
		
		case WM_COMMAND:			
			switch ( LOWORD(wParam) ) {				
				// Material
				case IDC_SELECT_BYID: 
					es->SelectByMat(es->matSpinSel->GetIVal()-1/*index*/, IsDlgButtonChecked(hDlg,IDC_CLEARSELECTION)/*clear*/);
				break;
				case IDC_MTLID_NAMES_COMBO:          
					switch(HIWORD(wParam)){
					case CBN_SELENDOK:
						int index, val;
						index = SendMessage(GetDlgItem(hDlg,IDC_MTLID_NAMES_COMBO), CB_GETCURSEL, 0, 0);
						val = SendMessage(GetDlgItem(hDlg, IDC_MTLID_NAMES_COMBO), CB_GETITEMDATA, (WPARAM)index, 0);
						if (index != CB_ERR){
							es->SelectByMat(val/*index*/, IsDlgButtonChecked(hDlg,IDC_CLEARSELECTION)/*clear*/);
						}
					break;
					}
				break;  	
				}
		break;
		}
	
	return FALSE;
	}

#define OLD_SEL_LEVEL_CHUNK 0x1000	// Old backwards ordering
#define SEL_LEVEL_CHUNK 0x1001
#define VERT_NUM_CHUNK 0x1010
#define FLAGS_CHUNK 0x1020
#define CONNECT_COPY_CHUNK 0x1030
#define KNOT_TYPE_CHUNK 0x1031
#define COPY_TANGENT_CHUNK 0x1032
// Names of named selection sets
#define NAMEDVSEL_NAMES_CHUNK	0x1050
#define NAMEDSSEL_NAMES_CHUNK	0x1060
#define NAMEDPSEL_NAMES_CHUNK	0x1070
#define NAMEDSEL_STRING_CHUNK	0x1080
#define USEAREA_CHUNK			0x1090
#define AREA_CHUNK				0x1100
#define ESM_SOFT_SEL_CHUNK      0x1110

static int namedSelID[] = {
	NAMEDVSEL_NAMES_CHUNK,
	NAMEDSSEL_NAMES_CHUNK,
	NAMEDPSEL_NAMES_CHUNK};

IOResult EditSplineMod::Save(ISave *isave) {
	Modifier::Save(isave);
	Interval valid;
	ULONG nb;
	short sl = selLevel;
	isave->BeginChunk (FLAGS_CHUNK);
	isave->Write (&esFlags, sizeof(DWORD), &nb);
	isave->EndChunk();
	isave->BeginChunk(SEL_LEVEL_CHUNK);
	isave->Write(&sl,sizeof(short),&nb);
	isave->	EndChunk();
	isave->BeginChunk(VERT_NUM_CHUNK);
	isave->Write(&showVertNumbers,sizeof(BOOL), &nb);
	isave->Write(&SVNSelectedOnly,sizeof(BOOL), &nb);
	isave->EndChunk();
	// CAL-05/23/03: Add Connecting Splines when Shift-Move Copy. (FID #827)
	isave->BeginChunk(CONNECT_COPY_CHUNK);
	isave->Write(&connectCopy, sizeof(BOOL), &nb);
	isave->Write(&connectCopyThreshold, sizeof(float), &nb);
	isave->EndChunk();
	// CAL-02/24/03: Add Cross Section mode. (FID #827)
	isave->BeginChunk(KNOT_TYPE_CHUNK);
	isave->Write(&newKnotType, sizeof(int), &nb);
	isave->EndChunk();
	// CAL-02/27/03: copy/paste tangent. (FID #827)
	isave->BeginChunk(COPY_TANGENT_CHUNK);
	isave->Write(&copyTanLength, sizeof(BOOL), &nb);
	isave->EndChunk();
	// Save names of named selection sets
	for (int j=0; j<3; j++) {
		if (namedSel[j].Count()) {
			isave->BeginChunk(namedSelID[j]);			
			for (int i=0; i<namedSel[j].Count(); i++) {
				isave->BeginChunk(NAMEDSEL_STRING_CHUNK);
				isave->WriteWString(*namedSel[j][i]);
				isave->EndChunk();
				}
			isave->EndChunk();
			}
		}

//2-1-99 watje
	if (useAreaSelect)
		{
		isave->BeginChunk(USEAREA_CHUNK);
		isave->EndChunk();
		}
	isave->BeginChunk(AREA_CHUNK);
	isave->Write(&areaSelect,sizeof(float),&nb);
	isave->EndChunk();

	isave->BeginChunk(ESM_SOFT_SEL_CHUNK);
		isave->Write(&mEdgeDist,sizeof(int),&nb);
		isave->Write(&mUseEdgeDists,sizeof(int),&nb);
		isave->Write(&mUseSoftSelections,sizeof(int),&nb);
		isave->Write(&mFalloff,sizeof(float),&nb);
		isave->Write(&mPinch,sizeof(float),&nb);
		isave->Write(&mBubble,sizeof(float),&nb);
	isave->EndChunk();


	return IO_OK;
	}

IOResult EditSplineMod::LoadNamedSelChunk(ILoad *iload,int level)
	{	
	IOResult res;
	
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case NAMEDSEL_STRING_CHUNK: {
				TCHAR *name;
				res = iload->ReadWStringChunk(&name);
				// Set the name in the modifier
				AddSet(TSTR(name),level+1);
				break;
				}
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

IOResult EditSplineMod::Load(ILoad *iload) {
	Modifier::Load(iload);
	IOResult res;
	ULONG nb;

	useAreaSelect = FALSE;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case FLAGS_CHUNK:
				res = iload->Read(&esFlags,sizeof(DWORD),&nb);
				break;
			case OLD_SEL_LEVEL_CHUNK:
				{
				short sl;
				res = iload->Read(&sl,sizeof(short),&nb);
				selLevel = sl;
				switch(selLevel) {
					case 1:
						selLevel = ES_SPLINE;
						break;
					case 3:
						selLevel = ES_VERTEX;
						break;
					}
				}
				break;
			case SEL_LEVEL_CHUNK:
				res = iload->Read(&selLevel,sizeof(int),&nb);
				break;
			case VERT_NUM_CHUNK:
				res = iload->Read(&showVertNumbers,sizeof(BOOL), &nb);
				res = iload->Read(&SVNSelectedOnly,sizeof(BOOL), &nb);
				break;
			// CAL-05/23/03: Add Connecting Splines when Shift-Move Copy. (FID #827)
			case CONNECT_COPY_CHUNK:
				res = iload->Read(&connectCopy, sizeof(BOOL), &nb);
				res = iload->Read(&connectCopyThreshold, sizeof(float), &nb);
				break;
			// CAL-02/24/03: Add Cross Section mode. (FID #827)
			case KNOT_TYPE_CHUNK:
				res = iload->Read(&newKnotType, sizeof(int), &nb);
				break;
			// CAL-02/27/03: copy/paste tangent. (FID #827)
			case COPY_TANGENT_CHUNK:
				res = iload->Read(&copyTanLength, sizeof(BOOL), &nb);
				break;
			case NAMEDVSEL_NAMES_CHUNK: {				
				res = LoadNamedSelChunk(iload,0);
				break;
				}
			case NAMEDSSEL_NAMES_CHUNK: {
				res = LoadNamedSelChunk(iload,1);
				break;
				}
			case NAMEDPSEL_NAMES_CHUNK: {
				res = LoadNamedSelChunk(iload,2);
				break;
				}
//2-1-99 watje
			case USEAREA_CHUNK:
				useAreaSelect = TRUE;
				break;
			case AREA_CHUNK:
				res = iload->Read(&areaSelect,sizeof(float), &nb);
				break;
			case ESM_SOFT_SEL_CHUNK:
				{
				iload->Read(&mEdgeDist,sizeof(int),&nb);
				iload->Read(&mUseEdgeDists,sizeof(int),&nb);
				iload->Read(&mUseSoftSelections,sizeof(int),&nb);
				iload->Read(&mFalloff,sizeof(float),&nb);
				iload->Read(&mPinch,sizeof(float),&nb);
				iload->Read(&mBubble,sizeof(float),&nb);
				}
				break;

			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}


#define EDITSPLINEDATA_CHUNK 0x1000

IOResult EditSplineMod::SaveLocalData(ISave *isave, LocalModData *ld) {
	EditSplineData *es = (EditSplineData *)ld;

	isave->BeginChunk(EDITSPLINEDATA_CHUNK);
	es->Save(isave);
	isave->EndChunk();

	return IO_OK;
	}

IOResult EditSplineMod::LoadLocalData(ILoad *iload, LocalModData **pld) {
	IOResult res;
	EditSplineData *es;
	if (*pld==NULL) {
		*pld =(LocalModData *) new EditSplineData();
		}
	es = (EditSplineData *)*pld;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case EDITSPLINEDATA_CHUNK:
				res = es->Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

ModRecord::~ModRecord() {
	}

//watje

void EditSplineMod::DoBind(int poly1, int vert1, int poly2, int vert2) {

	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	theHold.Begin();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		int altered = 0;
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);
		shapeData->PreUpdateChanges(shape);

/*		int polys = shape->splineCount;
		for(int poly = 0; poly < polys; ++poly) {
			Spline3D *spline = shape->splines[poly];
			// If any bits are set in the selection set, let's DO IT!!
			if(shape->vertSel[poly].NumberSet()) {
*/
		altered = holdNeeded = 1;
		if ( theHold.Holding() ) {
			theHold.Put(new ShapeRestore(shapeData,this,shape));
			}
				// Call the spline break function
		if (vert1 == 1)
			shape->BindKnot(FALSE, vert2, poly2, poly1);
		else shape->BindKnot(TRUE, vert2, poly2, poly1);

		//		shape->splines[poly1]->plineCacheValid = FALSE;
		shape->InvalidateGeomCache();

		if(altered) {
			shapeData->UpdateChanges(shape,FALSE);
			shapeData->TempData(this)->Invalidate(PART_GEOM);
			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	
	if(holdNeeded)
		theHold.Accept(GetString(IDS_PW_BIND));
	else {
//		ip->DisplayTempPrompt(GetString(IDS_TH_NOVALIDVERTSSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
/*
	if ( !ip ) return;

	theHold.Begin();
	if ( theHold.Holding() )
		theHold.Put(new SSRestore(this));
	RecordTopologyTags();

	if (vert1 == 1)
		shape->BindKnot(FALSE, vert2, poly2, poly1);
	else shape->BindKnot(TRUE, vert2, poly2, poly1);
	shape->splines[poly1]->plineCacheValid = FALSE;
	ResolveTopoChanges();

	theHold.Accept(GetResString(IDS_TH_VERTCONNECT));
	SelectionChanged();
	shape.InvalidateGeomCache();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
*/
	}


void EditSplineMod::DoUnBind() {


	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	theHold.Begin();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		int altered = 0;
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);
		shapeData->PreUpdateChanges(shape);

/*		int polys = shape->splineCount;
		for(int poly = 0; poly < polys; ++poly) {
			Spline3D *spline = shape->splines[poly];
			// If any bits are set in the selection set, let's DO IT!!
			if(shape->vertSel[poly].NumberSet()) {
*/
		if ( theHold.Holding() ) {
			theHold.Put(new ShapeRestore(shapeData,this,shape));
			}
				// Call the spline break function
//loop through splines check selecetd knots
		BOOL taltered = FALSE;
		for (int i = 0;  i < shape->vertSel.polys; i++)
			{
			if (shape->vertSel.sel[i][1])
				{
				taltered = shape->UnbindKnot(i,FALSE);
				if (taltered)
					altered = holdNeeded = 1;
				}
			int end = shape->vertSel.sel[i].GetSize() -2;

			if (shape->vertSel.sel[i][end])
				{
				taltered = shape->UnbindKnot(i,TRUE);
				if (taltered)
					altered = holdNeeded = 1;

				}

			}

		if(altered) {
			shapeData->UpdateChanges(shape,FALSE);
			shapeData->TempData(this)->Invalidate(PART_GEOM);
			holdNeeded = TRUE;
			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	
	if(holdNeeded)
		theHold.Accept(GetString(IDS_PW_UNBIND));
	else {
//		ip->DisplayTempPrompt(GetString(IDS_TH_NOVALIDVERTSSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
/*
	if ( !ip ) return;

	theHold.Begin();
	if ( theHold.Holding() )
		theHold.Put(new SSRestore(this));
	RecordTopologyTags();

//loop through splines check selecetd knots
	for (int i = 0;  i < shape.vertSel.polys; i++)
		{
		if (shape->vertSel.sel[i][1])
			{
			shape->UnbindKnot(i,FALSE);
			}
		int end = shape->vertSel.sel[i].GetSize() -2;

		if (shape->vertSel.sel[i][end])
			{
			shape->UnbindKnot(i,TRUE);
			}

		}
	ResolveTopoChanges();

	theHold.Accept(GetResString(IDS_TH_VERTCONNECT));
	SelectionChanged();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);

*/	}

/*-------------------------------------------------------------------*/

void BindCMode::EnterMode()
	{

	if ( ss->hOpsPanel ) {
		ICustButton *but = GetICustButton(GetDlgItem(ss->hOpsPanel,IDC_ES_BIND));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
		}

	}

void BindCMode::ExitMode()
	{

	if ( ss->hOpsPanel ) {
		ICustButton *but = GetICustButton(GetDlgItem(ss->hOpsPanel,IDC_ES_BIND));
		but->SetCheck(FALSE);
		ReleaseICustButton(but);
		}

	}

void EditSplineMod::StartBindMode()
	{

	if ( !ip ) return;

	if (ip->GetCommandMode() == bindMode) {
		ip->SetStdCommandMode(CID_OBJMOVE);
		return;
		}
	ip->SetCommandMode(bindMode);

	}


/*-------------------------------------------------------------------*/
// CAL-03/03/03: copy tangent. (FID #827)

void CopyTangentCMode::EnterMode()
	{
	if ( es->hOpsPanel ) {
		if (!ip) return;

		ModContextList mcList;		
		INodeTab nodes;
		TimeValue t = ip->GetTime();

		ip->GetModContexts(mcList,nodes);
		es->ClearShapeDataFlag(mcList,ESD_BEENDONE);

		for ( int i = 0; i < mcList.Count(); i++ ) {
			EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
			if ( !shapeData ) continue;
			if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

			BezierShape *shape = shapeData->TempData(es)->GetShape(t);
			if(!shape) continue;

			shape->dispFlags |= DISP_BEZHANDLES;
			shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
		nodes.DisposeTemporary();
		es->ClearShapeDataFlag(mcList,ESD_BEENDONE);

		ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel,IDC_ES_COPY_TANGENT));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);

		es->NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
		}
	}

void CopyTangentCMode::ExitMode()
	{
	if ( es->hOpsPanel ) {
		if (!ip) return;

		ModContextList mcList;		
		INodeTab nodes;
		TimeValue t = ip->GetTime();

		ip->GetModContexts(mcList,nodes);
		es->ClearShapeDataFlag(mcList,ESD_BEENDONE);

		for ( int i = 0; i < mcList.Count(); i++ ) {
			EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
			if ( !shapeData ) continue;
			if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

			BezierShape *shape = shapeData->TempData(es)->GetShape(t);
			if(!shape) continue;

			shape->dispFlags &= ~DISP_BEZHANDLES;
			shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
		nodes.DisposeTemporary();
		es->ClearShapeDataFlag(mcList,ESD_BEENDONE);

		ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel,IDC_ES_COPY_TANGENT));
		but->SetCheck(FALSE);
		ReleaseICustButton(but);
		es->SetOpsDlgEnables();

		es->NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
		}
	}

void EditSplineMod::StartCopyTangentMode()
	{
	if ( !ip ) return;

	if (ip->GetCommandMode() == copyTangentMode) {
		ip->SetStdCommandMode(CID_OBJMOVE);
		return;
		}
	ip->SetCommandMode(copyTangentMode);
	}

BOOL EditSplineMod::CopyTangent(BezierShape *shape, int poly, int vert)
	{
	if (!shape) return FALSE;
	
	DbgAssert(poly < shape->SplineCount());
	if (poly >= shape->SplineCount()) return FALSE;

	Spline3D *spline = shape->splines[poly];
	DbgAssert(vert < spline->Verts());
	if (vert >= spline->Verts()) return FALSE;

	if ((vert % 3) == 1) return FALSE;

	copiedTangent = spline->GetVert(vert) - spline->GetKnotPoint(vert/3);
	tangentCopied = TRUE;
	return TRUE;
	}

/*-------------------------------------------------------------------*/
// CAL-03/03/03: paste tangent. (FID #827)

void PasteTangentCMode::EnterMode()
	{
	if ( es->hOpsPanel ) {
		if (!ip) return;

		ModContextList mcList;		
		INodeTab nodes;
		TimeValue t = ip->GetTime();

		ip->GetModContexts(mcList,nodes);
		es->ClearShapeDataFlag(mcList,ESD_BEENDONE);

		for ( int i = 0; i < mcList.Count(); i++ ) {
			EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
			if ( !shapeData ) continue;
			if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

			BezierShape *shape = shapeData->TempData(es)->GetShape(t);
			if(!shape) continue;

			shape->dispFlags |= DISP_BEZHANDLES;
			shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
		nodes.DisposeTemporary();
		es->ClearShapeDataFlag(mcList,ESD_BEENDONE);

		ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel,IDC_ES_PASTE_TANGENT));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);

		es->NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
		}
	}

void PasteTangentCMode::ExitMode()
	{
	if ( es->hOpsPanel ) {
		if (!ip) return;

		ModContextList mcList;		
		INodeTab nodes;
		TimeValue t = ip->GetTime();

		ip->GetModContexts(mcList,nodes);
		es->ClearShapeDataFlag(mcList,ESD_BEENDONE);

		for ( int i = 0; i < mcList.Count(); i++ ) {
			EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
			if ( !shapeData ) continue;
			if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

			BezierShape *shape = shapeData->TempData(es)->GetShape(t);
			if(!shape) continue;

			shape->dispFlags &= ~DISP_BEZHANDLES;
			shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
		nodes.DisposeTemporary();
		es->ClearShapeDataFlag(mcList,ESD_BEENDONE);

		ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel,IDC_ES_PASTE_TANGENT));
		but->SetCheck(FALSE);
		ReleaseICustButton(but);

		es->NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
		}
	}

void EditSplineMod::StartPasteTangentMode()
	{
	if ( !ip ) return;

	if (ip->GetCommandMode() == pasteTangentMode) {
		ip->SetStdCommandMode(CID_OBJMOVE);
		return;
		}
	ip->SetCommandMode(pasteTangentMode);
	}

void EditSplineMod::StartPasteTangent(EditSplineData *shapeData)
{
	if (!ip || !shapeData) return;

	TimeValue t = ip->GetTime();
	BezierShape *shape = shapeData->TempData(this)->GetShape(t);
	if(!shape) return;

	theHold.Begin();
	shapeData->BeginEdit(t);

	// Create a change record for this object and store a pointer to its delta info in this EditSplineData
	if(!TestAFlag(A_HELD)) {
		shapeData->vdelta.SetSize(*shape,FALSE);
		if ( theHold.Holding() )
			theHold.Put(new ShapeRestore(shapeData,this,shape));
		shapeData->vdelta.Zero();		// Reset all deltas
		shapeData->ClearHandleFlag();
		shapeData->PreUpdateChanges(shape);
		SetAFlag(A_HELD);
	}
}

void EditSplineMod::EndPasteTangent(EditSplineData *shapeData)
{
	if (!ip || !shapeData) return;

	TimeValue t = ip->GetTime();
	BezierShape *shape = shapeData->TempData(this)->GetShape(t);
	if(!shape) return;

	shapeData->UpdateChanges(shape);
	shapeData->TempData(this)->Invalidate(PART_GEOM);
	
	theHold.Accept(GetString(IDS_TH_PASTE_TANGENTS));
	
	ip->RedrawViews(t, REDRAW_NORMAL);
}

BOOL EditSplineMod::PasteTangent(BezierShape *shape, int poly, int vert)
{
	if (!shape) return FALSE;

	DbgAssert(poly < shape->SplineCount());
	if (poly >= shape->SplineCount()) return FALSE;

	Spline3D *spline = shape->splines[poly];
	DbgAssert(vert < spline->Verts());
	if (vert >= spline->Verts()) return FALSE;

	if ((vert % 3) == 1) return FALSE;

	BOOL shiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) ? TRUE : FALSE;
	TimeValue t = ip->GetTime();
	int knot = vert / 3;
	Point3 pastedTangent = copiedTangent;
	Point3 knotPt = spline->GetKnotPoint(knot);
	Point3 vertPt1 = spline->GetVert(vert);
	Point3 oldVec1 =  vertPt1 - knotPt;
	float oldLen1 = Length(oldVec1);
	float tanLen = Length(pastedTangent);
	float lengthRatio = 1.0f;
	if (copyTanLength) {
		if (oldLen1 != 0.0f)
			lengthRatio = tanLen / oldLen1;
	} else {
		if (tanLen != 0.0f)
			pastedTangent *= (oldLen1 / tanLen);
	}
	
	spline->SetVert(vert, knotPt + pastedTangent);

	// turn to corner type if shift key is pressed
	if (shiftPressed && spline->GetKnotType(knot) == KTYPE_BEZIER)
		spline->SetKnotType(knot, KTYPE_BEZIER_CORNER);

	if (spline->GetKnotType(knot) == KTYPE_BEZIER) {
		int vert2 = ((vert % 3) == 0) ? vert + 2 : vert - 2;
		Point3 vertPt2 = spline->GetVert(vert2);
		float oldLen2 = Length(knotPt - vertPt2);
		if (oldLen2 != 0.0f) {
			float ratio = (oldLen1 == 0.0f) ? 1.0f : oldLen2 / oldLen1;
			spline->SetVert(vert2, knotPt - ratio * pastedTangent);
		}
	}

	// If locked handles on all type
	// Turn the movement into a transformation matrix and transform all the handles attached to the owner vertex
	if (lockedHandles && (lockType == IDC_LOCKALL) && (spline->GetKnotType(knot) == KTYPE_BEZIER_CORNER)) {
		Point3 oldVec = Normalize(oldVec1);
		Point3 newVec = Normalize(pastedTangent);
		Matrix3 rotMat(1);

		// Get a matrix that will transform the old point to the new one
		// Cross product gives us the normal of the rotational axis
		Point3 axis = Normalize(oldVec ^ newVec);
		// Dot product gives us the angle
		float dot = DotProd(oldVec, newVec);
		float angle = (dot >= -1.0f) ? ((dot <= 1.0f) ? (float)-acos(dot) : 0.0f) : PI;

		// Watch out for cases where the vectors are exactly the opposite --
		// This results in an invalid axis for transformation!
		
		// Now let's build a matrix that'll do this for us!
		if (angle != 0.0f) {
			Quat quat = QFromAngAxis(angle, axis);
			quat.MakeMatrix(rotMat);
		}

		// Process the other handles through the matrix
		int vert2 = ((vert % 3) == 0) ? vert + 2 : vert - 2;
		Point3 vertPt2 = spline->GetVert(vert2);
		Point3 oldVec2 = vertPt2 - knotPt;
		float oldLen2 = Length(oldVec2);
		if (oldLen2 != 0.0f) {
			spline->SetVert(vert2, knotPt + (lengthRatio * oldVec2) * rotMat);
		}
	}

	spline->ComputeBezPoints();
	shape->InvalidateGeomCache();

	NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
	return TRUE;
}

/*-------------------------------------------------------------------*/
/*
void EditSplineMod::DoVertBreak() {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	theHold.Begin();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		int altered = 0;
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		shapeData->RecordTopologyTags(shape);

		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);

		int polys = shape->splineCount;
		for(int poly = 0; poly < polys; ++poly) {
			Spline3D *spline = shape->splines[poly];
			// If any bits are set in the selection set, let's DO IT!!
			if(shape->vertSel[poly].NumberSet()) {
				altered = holdNeeded = 1;
				if ( theHold.Holding() ) {
					theHold.Put(new ShapeRestore(shapeData,this,shape));
					}
				// Call the spline break function
				BreakSplineAtSelVerts(shape, poly);
				}
			}

		if(altered) {
			shapeData->UpdateChanges(shape);
			shapeData->TempData(this)->Invalidate(PART_TOPO);
			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	
	if(holdNeeded)
		theHold.Accept(GetString(IDS_TH_VERTBREAK));
	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOVALIDVERTSSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);

*/

void EditSplineMod::DoHide() {

	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	theHold.Begin();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		int altered = 0;
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);
		shapeData->PreUpdateChanges(shape);

//		altered = holdNeeded = 1;
		if ( theHold.Holding() ) {
			theHold.Put(new ShapeRestore(shapeData,this,shape));
			}

		switch(GetSubobjectLevel()) {
			case ES_VERTEX:
				altered = shape->HideSelectedVerts();
//clear selection set
				shape->vertSel.ClearAll();
				break;
			case ES_SEGMENT:
				altered = shape->HideSelectedSegs();
				shape->segSel.ClearAll();
				break;
			case ES_SPLINE:
				altered = shape->HideSelectedSplines();
				shape->polySel.ClearAll();

				break;
			}



/*
		int polys = shape->splineCount;
		for(int poly = 0; poly < polys; ++poly) {
			Spline3D *spline = shape->splines[poly];
			// If any bits are set in the selection set, let's DO IT!!
			if(shape->vertSel[poly].NumberSet()) {
				altered = holdNeeded = 1;
				if ( theHold.Holding() ) {
					theHold.Put(new ShapeRestore(shapeData,this,shape));
					}
				// Call the spline break function
				BreakSplineAtSelVerts(shape, poly);
				}
			}
*/

		if(altered) {
			shapeData->UpdateChanges(shape);
			shapeData->TempData(this)->Invalidate(PART_TOPO);
			holdNeeded = TRUE;
			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	
	if(holdNeeded)
		theHold.Accept(GetString(IDS_PW_HIDE));
	else {
//		ip->DisplayTempPrompt(GetString(IDS_TH_NOVALIDVERTSSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);



/*
	if ( !ip ) return;

	theHold.Begin();
	if ( theHold.Holding() )
		theHold.Put(new SSRestore(this));
	RecordTopologyTags();

	switch(GetSubobjectLevel()) {
		case ES_VERTEX:
			shape->HideSelectedVerts();
//clear selection set
			shape->vertSel.ClearAll();
			break;
		case ES_SEGMENT:
			shape->HideSelectedSegs();
			shape->segSel.ClearAll();
			break;
		case ES_SPLINE:
			shape->HideSelectedSplines();
			shape->polySel.ClearAll();

			break;
		}

	ResolveTopoChanges();

	theHold.Accept(GetResString(IDS_TH_VERTCONNECT));
	SelectionChanged();
	shape.InvalidateGeomCache();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
*/
	}


void EditSplineMod::DoUnhide() {

	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	theHold.Begin();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		int altered = 0;
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);
		shapeData->PreUpdateChanges(shape);

		altered = 1;
		if ( theHold.Holding() ) {
			theHold.Put(new ShapeRestore(shapeData,this,shape));
			}
		altered = shape->UnhideSegs();

		if(altered) {
			shapeData->UpdateChanges(shape);
			shapeData->TempData(this)->Invalidate(PART_TOPO);
			holdNeeded = TRUE;
			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	
	if(holdNeeded)
//FIX THIS NAME
		theHold.Accept(GetString(IDS_RB_UNHIDEALLFACES));
	else {
//		ip->DisplayTempPrompt(GetString(IDS_TH_NOVALIDVERTSSEL),PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);


/*	if ( !ip ) return;

	theHold.Begin();
	if ( theHold.Holding() )
		theHold.Put(new SSRestore(this));
	RecordTopologyTags();
	shape->UnhideSegs();

	ResolveTopoChanges();

	theHold.Accept(GetResString(IDS_TH_VERTCONNECT));
	SelectionChanged();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
*/
	}


void EditSplineMod::DoCycleVerts() {


	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	theHold.Begin();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		int altered = 0;
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);
		shapeData->PreUpdateChanges(shape);

		if ( theHold.Holding() ) {
			theHold.Put(new ShapeRestore(shapeData,this,shape));
			}
	
		int polys = shape->splineCount;
//check if only one selected cycle to next
//if a group is selected go to first
		int totalSelected = 0;
		for(int poly = 0; poly < polys; ++poly) {
			Spline3D *spline = shape->splines[poly];
			BitArray &sel = shape->vertSel[poly];
			totalSelected += sel.NumberSet();
			}

		if (totalSelected == 1)
			{
//find that one
			altered = 1;

			int s,k;
			Point3 p; 
			float adist = areaSelect * areaSelect;
			for(int poly = 0; poly < polys; ++poly) 
				{
				Spline3D *spline = shape->splines[poly];
				BitArray &sel = shape->vertSel[poly];
				for (int i = 0; i < sel.GetSize(); i++)
					{
					if (sel[i])
						{
						s = poly;
						k = i/3;
						p = spline->GetKnotPoint(k);
						i = sel.GetSize();
						poly = polys;
						}

					}
				}

	//now find next matching
			BOOL done = FALSE;
			int startPoly = s;
			int startKnot = k;

			int knot = ++k;
			poly = s;

			BOOL found = FALSE;
			Spline3D *spline = shape->splines[poly];
			BitArray &sel = shape->vertSel[poly];
			sel.ClearAll();
			BOOL first = TRUE;
			while (!done)
				{
				if ( (knot == startKnot) && (poly == startPoly))
					{
					done = TRUE;
					found = TRUE;
					}
				else if (knot >= spline->KnotCount())
					{
					knot = 0;
					poly++;
					if (poly >= shape->splineCount)
						poly = 0;
					spline = shape->splines[poly];

					}
				if (!done)
					{
					Point3 t = spline->GetKnotPoint(knot);
//2-1-99
					if (!useAreaSelect)
						{
						done = TRUE;
						found = TRUE;
						}
					else if (LengthSquared(p-t) <= adist) 
						{
						done = TRUE;
						found = TRUE;
						}
					else
						{
						knot++;
						}
					}

				}
			if (found)
				{
				BitArray &sel = shape->vertSel[poly];
				sel.Set(knot*3+1);

				}




	

			}
		else if (totalSelected > 0)
			{
//get first selected
			altered = 1;
			BOOL done = FALSE;
			int s,k;
			for(poly = 0; poly < polys; ++poly) {
				Spline3D *spline = shape->splines[poly];
				BitArray &sel = shape->vertSel[poly];
				if ((sel.NumberSet() >0) && (!done))
					{
					for (int i = 0; i < sel.GetSize(); i++)
						{
						if (sel[i])
							{
//2-1/99
							s = poly;
							k = i;
							done = TRUE;
							i = sel.GetSize();
							}
						}
					}
				sel.ClearAll();
				}

			shape->vertSel[s].Set(k);

			}


		if(altered) {
			shapeData->UpdateChanges(shape, FALSE);
			if ( shapeData->tempData ) {
				shapeData->tempData->Invalidate(PART_SELECT);
				}
			holdNeeded = TRUE;
//			shapeData->UpdateChanges(shape);
//			shapeData->TempData(this)->Invalidate(PART_TOPO);
			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	
	if(holdNeeded)
		theHold.Accept(GetString(IDS_PW_CYCLE));
	else {
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	SelectionChanged();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);


}



/*-------------------------------------------------------------------*/


static Point3 ESRefineConnectSegment(BezierShape *shape, int poly, int segment, float param, BOOL apply) {

	Spline3D *spline = shape->splines[poly];
	int knots = spline->KnotCount();
	int verts = spline->Verts();
	int segs = spline->Segments();
	int nextSeg = (segment + 1) % knots;
	int insertSeg = (nextSeg == 0) ? -1 : nextSeg;

	// Get the knot points
	Point3 v00 = spline->GetKnotPoint(segment);
	Point3 v30 = spline->GetKnotPoint(nextSeg);

	// Get the knot types
	int type1 = spline->GetKnotType(segment);
	int type2 = spline->GetKnotType(nextSeg);

	// Get the material
	MtlID mtl = spline->GetMatID(segment);

	// Special: If they're refining a line-type segment, force it to be a bezier curve again


	if(spline->GetLineType(segment) == LTYPE_LINE) {
		spline->SetKnotType(segment, KTYPE_BEZIER_CORNER);
		spline->SetKnotType(nextSeg, KTYPE_BEZIER_CORNER);
		spline->SetLineType(segment, LTYPE_CURVE);
		spline->SetOutVec(segment, v00 + (v30 - v00) / 3.0f);
		spline->SetInVec(nextSeg, v30 - (v30 - v00) / 3.0f);
		}

	if (!apply) param= 0.5f;

	Point3 point = spline->InterpBezier3D(segment, param);
	
	Point3 v10 = spline->GetOutVec(segment);
	Point3 v20 = spline->GetInVec(nextSeg);
	Point3 v01 = v00 + (v10 - v00) * param;
	Point3 v21 = v20 + (v30 - v20) * param;
	Point3 v11 = v10 + (v20 - v10) * param;
	Point3 v02 = v01 + (v11 - v01) * param;
	Point3 v12 = v11 + (v21 - v11) * param;
	Point3 v03 = v02 + (v12 - v02) * param;


	if (apply)
		{

		spline->SetOutVec(segment, v01);
		spline->SetInVec(nextSeg, v21);

	// New for r3: Make the knot type dependent on the bordering knot types
		int newType = KTYPE_BEZIER;
//		if (r3Style) {
		if (TRUE) {
			if(type1 == KTYPE_CORNER && type2 == KTYPE_CORNER)
				newType = KTYPE_CORNER;
			else
			if((type1 & KTYPE_CORNER) || (type2 & KTYPE_CORNER))
				newType = KTYPE_BEZIER_CORNER;
			else
			if(type1 == KTYPE_AUTO && type2 == KTYPE_AUTO)
				newType = KTYPE_AUTO;
			}
		SplineKnot newKnot(newType, LTYPE_CURVE, v03, v02, v12);
		newKnot.SetMatID(mtl);

		spline->AddKnot(newKnot, insertSeg);




		spline->ComputeBezPoints();
		shape->InvalidateGeomCache();

	// Now adjust the spline selection sets
		BitArray& vsel = shape->vertSel[poly];
		BitArray& ssel = shape->segSel[poly];
		vsel.SetSize(spline->Verts(), 1);
		int where = (segment + 1) * 3;
		vsel.Shift(RIGHT_BITSHIFT, 3, where);
		vsel.Clear(where);
		vsel.Clear(where+1);
		vsel.Clear(where+2);
		ssel.SetSize(spline->Segments(), 1);
		ssel.Shift(RIGHT_BITSHIFT, 1, segment + 1);
		ssel.Set(segment+1,ssel[segment]);
		}
	return v03;
/*
	Spline3D *spline = shape->splines[poly];
	int knots = spline->KnotCount();
	int verts = spline->Verts();
	int segs = spline->Segments();
	int nextSeg = (segment + 1) % knots;
	int insertSeg = (nextSeg == 0) ? -1 : nextSeg;

	// Get the knot points
	Point3 v00 = spline->GetKnotPoint(segment);
	Point3 v30 = spline->GetKnotPoint(nextSeg);
	
	// Get the knot types
	int type1 = spline->GetKnotType(segment);
	int type2 = spline->GetKnotType(nextSeg);

	// Get the material
	MtlID mtl = spline->GetMatID(segment);

	// Special: If they're refining a line-type segment, force it to be a bezier curve again
	if(spline->GetLineType(segment) == LTYPE_LINE) {
		spline->SetKnotType(segment, KTYPE_BEZIER_CORNER);
		spline->SetKnotType(nextSeg, KTYPE_BEZIER_CORNER);
		spline->SetLineType(segment, LTYPE_CURVE);
		spline->SetOutVec(segment, v00 + (v30 - v00) / 3.0f);
		spline->SetInVec(nextSeg, v30 - (v30 - v00) / 3.0f);
		}

	if (!apply) param= 0.5f;

	Point3 point = spline->InterpBezier3D(segment, param);
	
	Point3 v10 = spline->GetOutVec(segment);
	Point3 v20 = spline->GetInVec(nextSeg);
	Point3 v01 = v00 + (v10 - v00) * param;
	Point3 v21 = v20 + (v30 - v20) * param;
	Point3 v11 = v10 + (v20 - v10) * param;
	Point3 v02 = v01 + (v11 - v01) * param;
	Point3 v12 = v11 + (v21 - v11) * param;
	Point3 v03 = v02 + (v12 - v02) * param;

	if (apply)
		{
		spline->SetOutVec(segment, v01);
		spline->SetInVec(nextSeg, v21);

	// New for r3: Make the knot type dependent on the bordering knot types
		int newType = KTYPE_BEZIER;
		if(type1 == KTYPE_CORNER && type2 == KTYPE_CORNER)
			newType = KTYPE_CORNER;
		else
		if((type1 & KTYPE_CORNER) || (type2 & KTYPE_CORNER))
			newType = KTYPE_BEZIER_CORNER;
		else
		if(type1 == KTYPE_AUTO && type2 == KTYPE_AUTO)
			newType = KTYPE_AUTO;

		SplineKnot newKnot(newType, LTYPE_CURVE, v03, v02, v12);
		newKnot.SetMatID(mtl);

	// If animating, expand the controller array to accomodate this new vertex
		if(ss && ss->cont.Count()) {
			int base = shape->GetVertIndex(poly, 0);
			if(insertSeg < 0)	// Special flag for last vert
				insertSeg = spline->KnotCount();
			ss->InsertPointConts(base + insertSeg * 3, 3);
			}
		spline->AddKnot(newKnot, insertSeg);
		spline->ComputeBezPoints();
		shape->InvalidateGeomCache();
		

	// Now adjust the spline selection sets
		BitArray& vsel = shape->vertSel[poly];
		BitArray& ssel = shape->segSel[poly];
		vsel.SetSize(spline->Verts(), 1);
		int where = (segment + 1) * 3;
		vsel.Shift(RIGHT_BITSHIFT, 3, where);
		vsel.Clear(where);
		vsel.Clear(where+1);
		vsel.Clear(where+2);
		ssel.SetSize(spline->Segments(), 1);
		ssel.Shift(RIGHT_BITSHIFT, 1, segment + 1);
		ssel.Set(segment+1,ssel[segment]);
		}

	return v03;
*/
	}


static INT_PTR CALLBACK RefineConnectOptionDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// WIN64 Cleanup: Shuler
	switch (msg) {
		case WM_INITDIALOG:
			connectOnly = FALSE;
			CheckDlgButton(hWnd,IDC_DONTSHOWAGAIN,FALSE);
			CenterWindow(hWnd,GetParent(hWnd));
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_CONNECT_ONLY:
					connectOnly = TRUE;
					// Intentional fall-thru!
				case IDC_REFINE:
					dontAskRefineConnectOption = IsDlgButtonChecked(hWnd,IDC_DONTSHOWAGAIN);
					EndDialog(hWnd,1);					
					break;
				}
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}

void EditSplineMod::DoRefineConnect(ViewExp *vpt, BezierShape *rshape, int poly, int seg, IPoint2 p) {
	ModContextList mcList;		
	INodeTab nodes;
	BOOL holdNeeded = FALSE;
	int altered = 0;
	TimeValue t = ip->GetTime();

	Point3 sPoint = vpt->SnapPoint(p,p,NULL,SNAP_IN_3D);

	Point2 fp = Point2((float)p.x, (float)p.y);

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);

	if(nodes.Count() != 1) {
		nodes.DisposeTemporary();
//		assert(0);
		return;
		}

//	if (pointList.Count() != 0)
//		theHold.Accept(GetString(IDS_TH_REFINE));

	theHold.Begin();

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		
		// If this is the first edit, then the delta arrays will be allocated
		if ( theHold.Holding() ) {
			theHold.Put(new ShapeRestore(shapeData,this,shape));
			}
		shapeData->BeginEdit(t);
		shapeData->PreUpdateChanges(shape);

	// Find the location on the segment where the user clicked
		INode *inode = nodes[0];
		GraphicsWindow *gw = vpt->getGW();
		HitRegion hr;
		MakeHitRegion(hr, HITTYPE_POINT, 1, 4, &p);
		gw->setHitRegion(&hr);
		Matrix3 mat = inode->GetObjectTM(t);
		gw->setTransform(mat);	

		// First, look for a vertex under the cursor...
		vpt->ClearSubObjHitList();
		SetSplineHitOverride(SS_VERTEX);
		ip->SubObHitTest(ip->GetTime(),HITTYPE_POINT,ip->GetCrossing(),HIT_ABORTONHIT,&p,vpt);
		ClearSplineHitOverride();

		float param = 0.0f;
		Point3 pt;

		if ( vpt->NumSubObjHits() ) {
			HitLog &hits = vpt->GetSubObjHitList();
			HitRecord *rec = hits.First();
			while(rec) {
				ShapeHitData *hit = ((ShapeHitData *)rec->hitData);
				if(hit->shape == shape && (hit->index % 3) == 1) {
					if(!dontAskRefineConnectOption) {
						DialogBox(
							hInstance, 
							MAKEINTRESOURCE(IDD_REFINE_CONNECT_OPTION),
							ip->GetMAXHWnd(), 
							RefineConnectOptionDlgProc);
						}
					if(connectOnly) {
						pt = shape->splines[hit->poly]->GetKnotPoint(hit->index / 3);
						boundLast = FALSE;
						goto bypass_refine;
						}
					goto proceed;
					}
				rec = rec->Next();
				}
			}

proceed:
			holdNeeded = TRUE;
		
		param = shape->FindSegmentPoint(poly, seg, gw, gw->getMaterial(), &hr);

		if ((bindFirst) && (pointList.Count() ==0) && (!closedRefineConnect))
			{
			pt = ESRefineConnectSegment( shape, poly, seg, param,FALSE);
			startSegRC = seg;
			startSplineRC = poly;
			boundFirst = TRUE;
			}
		else if (bindLast) 
			{
			endSegRC = seg;
			endSplineRC = poly;

//			prevSpline = poly;
//			insertPoint = seg;
			int knots = shape->splines[poly]->KnotCount();
			knotPoint1 = shape->splines[poly]->GetKnot(seg);
			knotPoint2 = shape->splines[poly]->GetKnot((seg + 1) % knots);

			pt = ESRefineConnectSegment( shape, poly, seg, param,TRUE);
			boundLast = TRUE;
			}
		else {
			pt = ESRefineConnectSegment(shape, poly, seg, param,TRUE);
			boundLast = FALSE;
			}

		if (bindFirst && boundFirst)
			{
			if ((poly == startSplineRC) && (seg < startSegRC)) startSegRC++;
			}
bypass_refine:

		pointList.Append(1,&pt,1);

		if(holdNeeded) {
			shapeData->UpdateChanges(shape);
			shapeData->TempData(this)->Invalidate(PART_TOPO);
//		shape->UpdateBindList();

			theHold.Accept(GetString(IDS_TH_REFINE));
			SelectionChanged();
			NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
			ip->RedrawViews(t, REDRAW_NORMAL);
			}
		else
			theHold.End();
		}
	nodes.DisposeTemporary();
	}


void RefineConnectCMode::EnterMode()
	{

	if ( ss->hOpsPanel ) {
		ICustButton *but = GetICustButton(GetDlgItem(ss->hOpsPanel, IDC_ES_REFINE));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
//2-1-99 watje
		EnableWindow (GetDlgItem (ss->hOpsPanel, IDC_ES_RCONNECT),FALSE);
		EnableWindow (GetDlgItem (ss->hOpsPanel, IDC_ES_RCLINEAR), FALSE);
		EnableWindow (GetDlgItem (ss->hOpsPanel, IDC_ES_RCCLOSED), FALSE);
		EnableWindow (GetDlgItem (ss->hOpsPanel, IDC_ES_BINDFIRST), FALSE);
		EnableWindow (GetDlgItem (ss->hOpsPanel, IDC_ES_BINDLAST), FALSE);

		}

	}

void RefineConnectCMode::ExitMode()
	{
//add line now

	if ( ss->hOpsPanel ) {

		if (ss->pointList.Count() > 0) ss->EndRefineConnectMode();

		ICustButton *but = GetICustButton(GetDlgItem(ss->hOpsPanel, IDC_ES_REFINE));
		but->SetCheck(FALSE);
		ReleaseICustButton(but);
//2-1-99 watje
		BOOL vType = (ss->GetSubobjectLevel() == ES_VERTEX) ? TRUE : FALSE;
		BOOL sType = (ss->GetSubobjectLevel() == ES_SEGMENT) ? TRUE : FALSE;

		if ( vType||sType)
			EnableWindow (GetDlgItem (ss->hOpsPanel, IDC_ES_RCONNECT),TRUE);
		else EnableWindow (GetDlgItem (ss->hOpsPanel, IDC_ES_RCONNECT),FALSE); 

		if ( (ss->rConnect) && (vType||sType))
			{
			EnableWindow (GetDlgItem (ss->hOpsPanel, IDC_ES_RCLINEAR), TRUE);
			EnableWindow (GetDlgItem (ss->hOpsPanel, IDC_ES_RCCLOSED), TRUE);
			EnableWindow (GetDlgItem (ss->hOpsPanel, IDC_ES_BINDFIRST), TRUE);
			EnableWindow (GetDlgItem (ss->hOpsPanel, IDC_ES_BINDLAST), TRUE);
			}
		else
			{
			EnableWindow (GetDlgItem (ss->hOpsPanel, IDC_ES_RCLINEAR), FALSE);
			EnableWindow (GetDlgItem (ss->hOpsPanel, IDC_ES_RCCLOSED), FALSE);
			EnableWindow (GetDlgItem (ss->hOpsPanel, IDC_ES_BINDFIRST), FALSE);
			EnableWindow (GetDlgItem (ss->hOpsPanel, IDC_ES_BINDLAST), FALSE);
			}


		}

	}

void EditSplineMod::StartRefineConnectMode()
	{
	if ( !ip ) return;

	if (ip->GetCommandMode() == refineConnectMode) {
		ip->SetStdCommandMode(CID_OBJMOVE);
		return;
		}
	pointList.ZeroCount();

	refineConnectMode->SetType(GetSubobjectLevel()==ES_VERTEX ? REFINE_VERT : REFINE_SEG);
	ip->SetCommandMode(refineConnectMode);

	boundFirst = boundLast = FALSE;
	}


void EditSplineMod::EndRefineConnectMode()
	{

//	if ((bindLast) && (!closedRefineConnect))
//		theHold.Cancel();
//	else theHold.Accept(GetString(IDS_TH_REFINE));


	if (pointList.Count() == 1) return;


	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	theHold.Begin();

	for (int i = 0; i < mcList.Count(); i++ ) {
		Point3 pivot = nodes[i]->GetObjOffsetPos();
		int altered = 0;
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);
		shapeData->PreUpdateChanges(shape);

		altered = holdNeeded = 1;
		// Save the unmodified verts.
		if ( theHold.Holding() ) {
			theHold.Put(new ShapeRestore(shapeData,this,shape));
			}

//get rid of last refine if needed
		if (pointList.Count() > 1)
			{
			if ((startSplineRC == endSplineRC) && (bindFirst && boundFirst) && (bindLast && boundLast))
				{
				if ((startSegRC >= endSegRC) && (startSegRC>0))
					startSegRC--;
				}

			if (bindLast && boundLast)
				{
				shape->splines[endSplineRC]->DeleteKnot(endSegRC+1);
				shape->segSel.sel[endSplineRC].Shift(LEFT_BITSHIFT, 1, endSegRC+1);
				shape->segSel.sel[endSplineRC].SetSize(shape->segSel.sel[endSplineRC].GetSize()-1,TRUE);

				shape->vertSel.sel[endSplineRC].Shift(LEFT_BITSHIFT, 3, endSegRC+2);
				shape->vertSel.sel[endSplineRC].SetSize(shape->vertSel.sel[endSplineRC].GetSize()-3,TRUE);

				int knots = shape->splines[endSplineRC]->KnotCount();
				SplineKnot knotTemp = shape->splines[endSplineRC]->GetKnot(endSegRC);
				int aux = knotTemp.Aux2();
				knotPoint1.SetAux2(aux);

				shape->splines[endSplineRC]->SetKnot(endSegRC,knotPoint1);

				knotTemp = shape->splines[endSplineRC]->GetKnot((endSegRC + 1) % knots);
				aux = knotTemp.Aux2();
				knotPoint2.SetAux2(aux);

				shape->splines[endSplineRC]->SetKnot((endSegRC + 1) % knots,knotPoint2);

				}

			int ct = shape->splineCount;
			Spline3D *newSpline = shape->NewSpline();

			shape->vertSel.Insert(shape->SplineCount() - 1, pointList.Count() * 3);
			if (!closedRefineConnect)
				shape->segSel.Insert(shape->SplineCount() - 1, (pointList.Count()-1));
			else shape->segSel.Insert(shape->SplineCount() - 1, (pointList.Count()));
			shape->polySel.Insert(shape->SplineCount() - 1);

			for	(int j=0; j<pointList.Count();j++)
				{	
				
				if (smoothRefineConnect)  
					{
					SplineKnot k = SplineKnot(KTYPE_AUTO, LTYPE_CURVE,pointList[j],pointList[j],pointList[j]);
					newSpline->AddKnot(k);
					}
					else 
					{
					SplineKnot k = SplineKnot(KTYPE_CORNER, LTYPE_CURVE,pointList[j],pointList[j],pointList[j]);
					newSpline->AddKnot(k);
					}
				}
			if (closedRefineConnect)
				newSpline->SetClosed();
			else newSpline->SetOpen();
			newSpline->ComputeBezPoints();

			shape->InvalidateGeomCache();

			if(altered) {
				shapeData->UpdateChanges(shape);
				shapeData->TempData(this)->Invalidate(PART_TOPO);
				}
			shapeData->SetFlag(ESD_BEENDONE,TRUE);


			if ((bindFirst && boundFirst) && (!closedRefineConnect))
				{
				shape->BindKnot(FALSE , startSegRC,startSplineRC, ct);
				}
			if ((bindLast && boundLast) && (!closedRefineConnect))
				{
				shape->BindKnot(TRUE , endSegRC,endSplineRC, ct);

				}



		

			}

		}
	
	if(holdNeeded) {
		theHold.Accept(GetString(IDS_TH_CREATELINE));
		}
/*	else {
		ip->DisplayTempPrompt(GetString(IDS_TH_NOSPLINESSEL),PROMPT_TIME);
		theHold.End();
		}
*/
	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	SelectionChanged();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(t,REDRAW_NORMAL);



/*
	ModContextList mcList;		
	INodeTab nodes;
	int holdNeeded = 0;
	int altered = 0;
	TimeValue t = ip->GetTime();

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);

	if(nodes.Count() != 1) {
		nodes.DisposeTemporary();
		assert(0);
		return;
		}

	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	theHold.Begin();

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->RecordTopologyTags(shape);
		if ( theHold.Holding() ) {
			theHold.Put(new ShapeRestore(shapeData,this,shape));
			}
		shapeData->BeginEdit(t);

		int ct = shape->splineCount;
		Spline3D *newSpline = shape->NewSpline();
		for	(int j=0; j<pointList.Count();j++)
			{	
				
			if (smoothRefineConnect)  
				{
				SplineKnot k = SplineKnot(KTYPE_AUTO, LTYPE_CURVE,pointList[j],pointList[j],pointList[j]);
				newSpline->AddKnot(k);
				}
				else 
				{
				SplineKnot k = SplineKnot(KTYPE_CORNER, LTYPE_CURVE,pointList[j],pointList[j],pointList[j]);
				newSpline->AddKnot(k);
				}
			}
		if (closedRefineConnect)
			newSpline->SetClosed();
		else newSpline->SetOpen();
			newSpline->ComputeBezPoints();

		if ((bindFirst) && (!closedRefineConnect))
			{
			shape->BindKnot(FALSE , startSegRC,startSplineRC, ct);
			}
		if ((bindLast) && (!closedRefineConnect))
			{
			shape->BindKnot(TRUE , endSegRC,endSplineRC, ct);

			}

		newSpline->ComputeBezPoints();

		shape->vertSel.Insert(shape->SplineCount() - 1, pointList.Count() * 3);
		shape->segSel.Insert(shape->SplineCount() - 1, pointList.Count());
		shape->polySel.Insert(shape->SplineCount() - 1);
		

		shape->InvalidateGeomCache();


		shape->UpdateBindList();


		shapeData->UpdateChanges(shape,TRUE);
		shapeData->TempData(this)->Invalidate(PART_TOPO|PART_GEOM);
		shapeData->SetFlag(ESD_BEENDONE,TRUE);

		}
	theHold.Accept(GetString(IDS_TH_CREATELINE));

	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	SelectionChanged();

	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(t, REDRAW_NORMAL);


*/

	}

/*-------------------------------------------------------------------*/

// Trim/Extend follows

/*-------------------------------------------------------------------*/

void TrimCMode::EnterMode()
	{
	if ( es->hOpsPanel ) {
		ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel,IDC_ES_TRIM));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
		}
	}

void TrimCMode::ExitMode()
	{
	if ( es->hOpsPanel ) {
		es->EndOutlineMove(es->ip->GetTime(),TRUE);
		ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel,IDC_ES_TRIM));
		but->SetCheck(FALSE);
		ReleaseICustButton(but);
		}
	}

void EditSplineMod::StartTrimMode()
	{
	if ( !ip ) return;

	if (ip->GetCommandMode() == trimMode) {
		ip->SetStdCommandMode(CID_OBJMOVE);
		return;
		}
	ip->SetCommandMode(trimMode);
	}

/*-------------------------------------------------------------------*/

void ExtendCMode::EnterMode()
	{
	if ( es->hOpsPanel ) {
		ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel,IDC_ES_EXTEND));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
		}
	}

void ExtendCMode::ExitMode()
	{
	if ( es->hOpsPanel ) {
		es->EndOutlineMove(es->ip->GetTime(),TRUE);
		ICustButton *but = GetICustButton(GetDlgItem(es->hOpsPanel,IDC_ES_EXTEND));
		but->SetCheck(FALSE);
		ReleaseICustButton(but);
		}
	}

void EditSplineMod::StartExtendMode()
	{
	if ( !ip ) return;

	if (ip->GetCommandMode() == extendMode) {
		ip->SetStdCommandMode(CID_OBJMOVE);
		return;
		}
	ip->SetCommandMode(extendMode);
	}

class EditSplineShapeContextCallback : public ShapeContextCallback {
	public:
		EditSplineMod *mod;
		EditSplineShapeContextCallback(EditSplineMod *m) { mod=m; }
		BezierShape *GetShapeContext(ModContext *context);
	};

BezierShape *EditSplineShapeContextCallback::GetShapeContext(ModContext *context) {
	EditSplineData *shapeData = (EditSplineData*)context->localData;
	if ( !mod->ip )
		return NULL;
	if ( !shapeData )
		return NULL;
	return shapeData->TempData(mod)->GetShape(mod->ip->GetTime());
	}

void EditSplineMod::HandleTrimExtend(ViewExp *vpt, ShapeHitData *hit, IPoint2 &m, int trimType) {
	EditSplineShapeContextCallback cb(this);

	ModContextList mcList;		
	INodeTab nodes;
	if ( !ip ) return;
	TimeValue t = ip->GetTime();
	BOOL holdNeeded = FALSE;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	theHold.Begin();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		
		if(shape == hit->shape) {
			// If this is the first edit, then the delta arrays will be allocated
			shapeData->BeginEdit(t);
			shapeData->PreUpdateChanges(shape, FALSE);

			if ( theHold.Holding() )
				theHold.Put(new ShapeRestore(shapeData,this,shape));
			if(shape->PerformTrimOrExtend(ip, vpt, hit, m, cb, trimType, trimInfinite)) {
				holdNeeded = TRUE;
				shapeData->UpdateChanges(shape, FALSE);
				shapeData->TempData(this)->Invalidate(PART_TOPO);
				}
			}
		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	
	if(holdNeeded)
		theHold.Accept(trimType == SHAPE_TRIM ? GetString(IDS_TH_TRIM) : GetString(IDS_TH_EXTEND));
	else
		theHold.End();

	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
	}

/*-------------------------------------------------------------------*/
// --------------------------------------------------------------------
// ISplineSelect and ISplineOps interfaces   (JBW 2/1/99)

void* EditSplineMod::GetInterface(ULONG id) 
{
	switch (id)
	{
		case I_SPLINESELECT: return (ISplineSelect*)this;
		case I_SPLINESELECTDATA: return (ISplineSelectData*)this;
		case I_SPLINEOPS: return (ISplineOps*)this;
		case I_SUBMTLAPI: return (ISubMtlAPI*)this;
	}
	return Modifier::GetInterface(id);
}

void EditSplineMod::StartCommandMode(splineCommandMode mode)
{
	switch (mode)
	{
		case ScmCreateLine:
			if (hOpsPanel != NULL)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_CREATELINE, 0);
			break;
		case ScmAttach:
			if (hOpsPanel != NULL)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_ATTACH, 0);
			break;
		// CAL-02/26/03: Add Cross Section. (FID #827)
		case ScmCrossSection:
			if (hOpsPanel != NULL)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_CROSS_SECTION, 0);
			break;
		case ScmInsert:
			if (hOpsPanel != NULL)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_INSERT, 0);
			break;
		case ScmConnect:
			if (hOpsPanel != NULL && GetSubobjectLevel() == ES_VERTEX)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_CONNECT, 0);
			break;
		case ScmRefine:
			if (hOpsPanel != NULL && (GetSubobjectLevel() == ES_VERTEX || GetSubobjectLevel() == ES_SEGMENT))
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_REFINE, 0);
			break;
		case ScmFillet:
			if (hOpsPanel != NULL && GetSubobjectLevel() == ES_VERTEX)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_FILLET, 0);
			break;
		case ScmChamfer:
			if (hOpsPanel != NULL && GetSubobjectLevel() == ES_VERTEX)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_CHAMFER, 0);
			break;
		case ScmBind:
			if (hOpsPanel != NULL && GetSubobjectLevel() == ES_VERTEX)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_BIND, 0);
			break;
		// CAL-02/03/03: copy/paste tangent. (FID #827)
		case ScmCopyTangent:
			if (hOpsPanel != NULL && GetSubobjectLevel() == SS_VERTEX)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_COPY_TANGENT, 0);
			break;
		case ScmPasteTangent:
			if (hOpsPanel != NULL && GetSubobjectLevel() == SS_VERTEX && tangentCopied)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_PASTE_TANGENT, 0);
			break;
		case ScmRefineConnect:
			if (hOpsPanel != NULL && (GetSubobjectLevel() == ES_VERTEX || GetSubobjectLevel() == ES_SEGMENT))
//watje			PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_REFINECONNECT, 0);
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_REFINE, 0);
			break;
		case ScmOutline:
			if (hOpsPanel != NULL && GetSubobjectLevel() == ES_SPLINE)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_OUTLINE, 0);
			break;
		case ScmTrim:
			if (hOpsPanel != NULL && GetSubobjectLevel() == ES_SPLINE)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_TRIM, 0);
			break;
		case ScmExtend:
			if (hOpsPanel != NULL && GetSubobjectLevel() == ES_SPLINE)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_EXTEND, 0);
			break;
		case ScmCrossInsert:
			if (hOpsPanel != NULL && GetSubobjectLevel() == ES_VERTEX)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_CROSS_INSERT, 0);
			break;
		case ScmBreak:
			if (hOpsPanel != NULL && (GetSubobjectLevel() == ES_SEGMENT || GetSubobjectLevel() == ES_VERTEX))
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_BREAK, 0);
			break;
		case ScmUnion:
			if (hOpsPanel != NULL && GetSubobjectLevel() == ES_SPLINE)
			{
				PostMessage(hOpsPanel, WM_COMMAND, BOOL_UNION, 0);
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_BOOLEAN, 0);
			}
			break;
		case ScmSubtract:
			if (hOpsPanel != NULL && GetSubobjectLevel() == ES_SPLINE)
			{
				PostMessage(hOpsPanel, WM_COMMAND, BOOL_SUBTRACTION, 0);
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_BOOLEAN, 0);
			}
			break;
	}
}

void EditSplineMod::ButtonOp(splineButtonOp opcode)
{
	switch (opcode)
	{
		case SopHide:
			if (hOpsPanel != NULL && GetSubobjectLevel() >= SS_VERTEX) // LAM: added SO level test
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_HIDE, 0);
			break;
		case SopUnhideAll:
			if (hOpsPanel != NULL)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_UNHIDE, 0);
			break;
		case SopDelete:
			if (hOpsPanel != NULL && GetSubobjectLevel() >= ES_VERTEX)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_DELETE, 0);
			break;
		case SopDetach:
			if (hOpsPanel != NULL && (GetSubobjectLevel() == ES_SEGMENT || GetSubobjectLevel() == ES_SPLINE))
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_DETACH, 0);
			break;
		case SopDivide:
			if (hOpsPanel != NULL && GetSubobjectLevel() == ES_SEGMENT)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_SEGDIVIDE, 0);
			break;
		case SopCycle:
			if (hOpsPanel != NULL && GetSubobjectLevel() == ES_VERTEX)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_CYCLE, 0);
			break;
		case SopUnbind:
			if (hOpsPanel != NULL && GetSubobjectLevel() == ES_VERTEX)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_UNBIND, 0);
			break;
		case SopWeld:
			if (hOpsPanel != NULL && GetSubobjectLevel() == ES_VERTEX)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_WELD, 0);
			break;
		case SopMakeFirst:
			if (hOpsPanel != NULL && GetSubobjectLevel() == ES_VERTEX)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_MAKEFIRST, 0);
			break;
		case SopAttachMultiple:
			if (hOpsPanel != NULL)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_ATTACH_MULTIPLE, 0);
			break;
		case SopExplode:
			if (hOpsPanel != NULL && GetSubobjectLevel() == ES_SPLINE)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_EXPLODE, 0);
			break;
		case SopReverse:
			if (hOpsPanel != NULL && GetSubobjectLevel() == ES_SPLINE)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_REVERSE, 0);
			break;
		case SopClose:
			if (hOpsPanel != NULL && GetSubobjectLevel() == ES_SPLINE)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_CLOSE, 0);
			break;
		case SopIntersect:
			if (hOpsPanel != NULL && GetSubobjectLevel() == ES_SPLINE)
			{
				PostMessage(hOpsPanel, WM_COMMAND, BOOL_INTERSECTION, 0);
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_BOOLEAN, 0);
			}
			break;
		case SopMirrorHoriz:
			if (hOpsPanel != NULL && GetSubobjectLevel() == ES_SPLINE)
			{
				PostMessage(hOpsPanel, WM_COMMAND, MIRROR_HORIZONTAL, 0);
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_MIRROR, 0);
			}
			break;
		case SopMirrorVert:
			if (hOpsPanel != NULL && GetSubobjectLevel() == ES_SPLINE)
			{
				PostMessage(hOpsPanel, WM_COMMAND, MIRROR_VERTICAL, 0);
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_MIRROR, 0);
			}
			break;
		case SopMirrorBoth:
			if (hOpsPanel != NULL && GetSubobjectLevel() == ES_SPLINE)
			{
				PostMessage(hOpsPanel, WM_COMMAND, MIRROR_BOTH, 0);
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_MIRROR, 0);
			}
			break;
		case SopSelectByID:
			if (hOpsPanel != NULL && (GetSubobjectLevel() == ES_SEGMENT || GetSubobjectLevel() == ES_SPLINE))
				PostMessage(hOpsPanel, WM_COMMAND, IDC_SELECT_BYID, 0);
			break;
// LAM: added following 9/2/00
		case SopFuse:
			if (hOpsPanel != NULL && GetSubobjectLevel() == SS_VERTEX)
				PostMessage(hOpsPanel, WM_COMMAND, IDC_ES_FUSE, 0);
			break;
	}
}

// LAM: added following 9/3/00 - just stubs for now....
//---------------------------------------------------------
//  UI-related methods - examples of how to do it....

void EditSplineMod::GetUIParam (splineUIParam uiCode, float & ret) {
	if (!ip) return;
	if (!Editing()) return;

//	switch (uiCode) {
//	case MuiPolyThresh:
//		ret = planarFaceThresh;
//		break;
//	case MuiFalloff:
//		ret = falloff;
//		break;
//	}
}

void EditSplineMod::SetUIParam (splineUIParam uiCode, float val) {
	if (!ip) return;
	if (!Editing()) return;
//	ISpinnerControl *spin;

//	switch (uiCode) {
//	case MuiPolyThresh:
//		planarFaceThresh = val;
//		if (hSel) {
//			spin = GetISpinner (GetDlgItem (hSel, IDC_PLANARSPINNER));
//			spin->SetValue (val, FALSE);
//			ReleaseISpinner (spin);
//		}
//		break;
//	case MuiFalloff:
//		falloff = val;
//		InvalidateAffectRegion ();
//		if (hAR) {
//			spin = GetISpinner (GetDlgItem (hAR, IDC_FALLOFFSPIN));
//			spin->SetValue (val, FALSE);
//			ReleaseISpinner (spin);
//		}
//		break;
//	}
}

void EditSplineMod::GetUIParam (splineUIParam uiCode, int & ret) {
	if (!ip) return;
	if (!Editing()) return;

//	switch (uiCode) {
//	case MuiSelByVert:
//		ret = selByVert;
//		break;
//	case MuiIgBack:
//		ret = ignoreBackfaces;
//		break;
//	}
}

void EditSplineMod::SetUIParam (splineUIParam uiCode, int val) {
	if (!ip) return;
	if (!Editing()) return;
//	ISpinnerControl *spin;

//	switch (uiCode) {
//	case MuiSelByVert:
//		selByVert = val ? TRUE : FALSE;
//		if (hSel) CheckDlgButton (hSel, IDC_SEL_BYVERT, selByVert);
//		break;
//	case MuiSoftSel:
//		affectRegion = val ? TRUE : FALSE;
//		if (hAR) {
//			CheckDlgButton (hAR, IDC_AFFECT_REGION, affectRegion);
//			SetARDlgEnables ();
//		}
//		break;
//	case MuiSSUseEDist:
//		useEdgeDist = val ? TRUE : FALSE;
//		if (hAR) {
//			CheckDlgButton (hAR, IDC_E_DIST, useEdgeDist);
//			spin = GetISpinner (GetDlgItem (hAR, IDC_E_ITER_SPIN));
//			spin->Enable (useEdgeDist);
//			ReleaseISpinner (spin);
//		}
//		break;
//	case MuiExtrudeType:
//		extType = val;
//		if (hGeom) {
//			if (extType == MESH_EXTRUDE_CLUSTER) {
//				CheckRadioButton (hGeom, IDC_EXTYPE_A, IDC_EXTYPE_B, IDC_EXTYPE_A);
//			} else {
//				CheckRadioButton (hGeom, IDC_EXTYPE_A, IDC_EXTYPE_B, IDC_EXTYPE_B);
//			}
//		}
//		break;
//	}
}


DWORD EditSplineMod::GetSelLevel()
{
	return GetSubobjectLevel();
}

void EditSplineMod::SetSelLevel(DWORD level)
{	
	SetSubobjectLevel(level);
}

void EditSplineMod::LocalDataChanged()
{
}

MtlID EditSplineMod::GetNextAvailMtlID(ModContext* mc) {
	if(!mc)
		return 1;
	EditSplineData *shapeData = (EditSplineData*)mc->localData;
	if ( !shapeData ) return 1;

	// If the mesh isn't yet cache, this will cause it to get cached.
	BezierShape *shape = shapeData->TempData(this)->GetShape(ip->GetTime());
	if(!shape) return 1;
	
	int mtlID = GetSelFaceUniqueMtlID(mc);

	if (mtlID == -1) {
		int i;
 		
		MtlID min, max;
		BOOL first = TRUE;

		int polys = shape->splineCount;
		for(int poly = 0; poly < polys; ++poly) {
			Spline3D *spline = shape->splines[poly];
			for(int seg = 0; seg < spline->Segments(); ++seg) {
				MtlID thisID = spline->GetMatID(seg);
				if(first) {
					min = max = thisID;
					first = FALSE;
					}
				else
				if(thisID < min)
					min = thisID;
				else
				if(thisID > max)
					max = thisID;
				}
			}
		// If room below, return it
		if(min > 0)
			return min - 1;
		// Build a bit array to find any gaps		
		BitArray b;
		int bits = max - min + 1;
		b.SetSize(bits);
		b.ClearAll();
		for(poly = 0; poly < polys; ++poly) {
			Spline3D *spline = shape->splines[poly];
			for(int seg = 0; seg < spline->Segments(); ++seg)
				b.Set(spline->GetMatID(seg) - min);
			}
		for(i = 0; i < bits; ++i) {
			if(!b[i])
				return (MtlID)(i + min);
			}
		// No gaps!  If room above, return it
		if(max < 65535)
			return max + 1;
		}
	return (MtlID)mtlID;
	}

BOOL EditSplineMod::HasFaceSelection(ModContext* mc) {
	// Are we the edited object?
	if (ip == NULL)  return FALSE;

	EditSplineData *shapeData = (EditSplineData*)mc->localData;
	if ( !shapeData ) return FALSE;

	// If the mesh isn't yet cache, this will cause it to get cached.
	BezierShape *shape = shapeData->TempData(this)->GetShape(ip->GetTime());
	if(!shape) return FALSE;

	// Is Segment selection active?
	if (selLevel == ES_SEGMENT) {
		for(int poly = 0; poly < shape->splineCount; ++poly)
			if(shape->segSel[poly].NumberSet()) return TRUE;
		}
	
	// Is Spline selection active?
	if (selLevel == ES_SPLINE && shape->polySel.sel.NumberSet()) return TRUE;
	
	return FALSE;
	}

void EditSplineMod::SetSelFaceMtlID(ModContext* mc, MtlID id, BOOL bResetUnsel) {
	int altered = 0;
	EditSplineData *shapeData = (EditSplineData*)mc->localData;
	if ( !shapeData ) return;

	// If the mesh isn't yet cache, this will cause it to get cached.
	BezierShape *shape = shapeData->TempData(this)->GetShape(ip->GetTime());
	if(!shape) return;
	
	// If this is the first edit, then the delta arrays will be allocated
	shapeData->BeginEdit(ip->GetTime());
	shapeData->PreUpdateChanges(shape, FALSE);

	if ( theHold.Holding() )
		theHold.Put(new ShapeRestore(shapeData,this,shape));

	int polys = shape->splineCount;
	for(int poly = 0; poly < polys; ++poly) {
		Spline3D *spline = shape->splines[poly];
		BOOL polySelected = (GetSubobjectLevel() == ES_SPLINE) ? shape->polySel[poly] : FALSE;
		BitArray &sel = shape->segSel[poly];
		for(int seg = 0; seg < spline->Segments(); ++seg) {
			BOOL segSelected = (GetSubobjectLevel() == ES_SEGMENT) ? sel[seg] : FALSE;
			if(polySelected || segSelected) {
				altered = TRUE;
				spline->SetMatID(seg, id);
				}
			}
		}

	if(altered)	{
		shapeData->UpdateChanges(shape, FALSE);
		InvalidateSurfaceUI();
		}

	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
	}

int	EditSplineMod::GetSelFaceUniqueMtlID(ModContext* mc) {
	int	mtlID;

	mtlID = GetSelFaceAnyMtlID(mc);
	if (mtlID == -1) return mtlID;

	EditSplineData *shapeData = (EditSplineData*)mc->localData;
	if ( !shapeData ) return 1;

	// If the mesh isn't yet cache, this will cause it to get cached.
	BezierShape *shape = shapeData->TempData(this)->GetShape(ip->GetTime());
	if(!shape) return 1;

	int polys = shape->splineCount;
	for(int poly = 0; poly < polys; ++poly) {
		Spline3D *spline = shape->splines[poly];
		BOOL polySelected = (GetSubobjectLevel() == ES_SPLINE) ? shape->polySel[poly] : FALSE;
		BitArray &sel = shape->segSel[poly];
		for(int seg = 0; seg < spline->Segments(); ++seg) {
			BOOL segSelected = (GetSubobjectLevel() == ES_SEGMENT) ? sel[seg] : FALSE;
			if(polySelected || segSelected)
				continue;
			if(spline->GetMatID(seg) != mtlID)
				continue;
			mtlID = -1;
			}
		}
	return mtlID;
	}

int	EditSplineMod::GetSelFaceAnyMtlID(ModContext* mc) {
	int				mtlID = -1;
	BOOL			bGotFirst = FALSE;

	EditSplineData *shapeData = (EditSplineData*)mc->localData;
	if ( !shapeData ) return 1;

	// If the mesh isn't yet cache, this will cause it to get cached.
	BezierShape *shape = shapeData->TempData(this)->GetShape(ip->GetTime());
	if(!shape) return 1;

	int polys = shape->splineCount;
	for(int poly = 0; poly < polys; ++poly) {
		Spline3D *spline = shape->splines[poly];
		BOOL polySelected = (GetSubobjectLevel() == ES_SPLINE) ? shape->polySel[poly] : FALSE;
		BitArray &sel = shape->segSel[poly];
		for(int seg = 0; seg < spline->Segments(); ++seg) {
			BOOL segSelected = (GetSubobjectLevel() == ES_SEGMENT) ? sel[seg] : FALSE;
			if(!polySelected && !segSelected)
				continue;
			if (bGotFirst) {
				if (mtlID != spline->GetMatID(seg)) {
					mtlID = -1;
					break;
					}
				}
			else {
				mtlID = spline->GetMatID(seg);
				bGotFirst = TRUE;
				}
			}
		}

	return mtlID;
	}

int	EditSplineMod::GetMaxMtlID(ModContext* mc) {
	MtlID mtlID = 0;

	EditSplineData *shapeData = (EditSplineData*)mc->localData;
	if ( !shapeData ) return 1;

	// If the mesh isn't yet cache, this will cause it to get cached.
	BezierShape *shape = shapeData->TempData(this)->GetShape(ip->GetTime());
	if(!shape) return 1;

	int polys = shape->splineCount;
	for(int poly = 0; poly < polys; ++poly) {
		Spline3D *spline = shape->splines[poly];
		for(int seg = 0; seg < spline->Segments(); ++seg)
			mtlID = max(mtlID, spline->GetMatID(seg));
		}

	return mtlID;
	}

//2-1-99 watje

void EditSplineMod::DoVertFuse() {


	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	BOOL holdNeeded = FALSE;
	BOOL hadSelected = FALSE;

	if ( !ip ) return;

	ip->GetModContexts(mcList,nodes);
	ClearShapeDataFlag(mcList,ESD_BEENDONE);

	int oldVerts = 0;
	int newVerts = 0;

	theHold.Begin();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		BOOL altered = FALSE;
		EditSplineData *shapeData = (EditSplineData*)mcList[i]->localData;
		if ( !shapeData ) continue;
		if ( shapeData->GetFlag(ESD_BEENDONE) ) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		BezierShape *shape = shapeData->TempData(this)->GetShape(t);
		if(!shape) continue;
		// If this is the first edit, then the delta arrays will be allocated
		shapeData->BeginEdit(t);
		shapeData->PreUpdateChanges(shape);

		// Add the original number of knots to the oldVerts field
		for(int poly = 0; poly < shape->splineCount; ++poly)
			oldVerts += shape->splines[poly]->KnotCount();

		if ( theHold.Holding() ) {
			theHold.Put(new ShapeRestore(shapeData,this,shape));
			}

//watje now move any verts thta are within the weld tolerance onto each other
//build list 
		Tab<WeldList> weldList;
		for(poly = 0; poly < shape->splineCount; ++poly)
			{
			BitArray pSel = shape->VertexTempSel(poly);
			for (int i = 0; i < pSel.GetSize()/3; i++)
				{
				if(pSel[(i*3)+1]) 
					{
					WeldList w;
					w.used = -1;
					w.splineID =poly;
					w.knotID = i;
					w.p = shape->splines[poly]->GetKnotPoint(i);
					weldList.Append(1,&w,1);
					}
				}
			}
		int ct = weldList.Count();
		if (ct >0)
			{
			altered = holdNeeded = TRUE;
			Point3 avgPoint(0.0f,0.0f,0.0f);
			int pointCount = 0;
			for (int j = 0; j < weldList.Count(); j++)
				{
				avgPoint += weldList[j].p;
				pointCount++;
				}
			avgPoint = avgPoint/(float)pointCount;
			for (j = 0; j < weldList.Count(); j++)
				{
				weldList[j].p = avgPoint;
				}
//now put them back in the knot list
			for (int i = 0; i < weldList.Count(); i++)
				{
				int polyID = weldList[i].splineID;
				int knotID = weldList[i].knotID;
//10-4-99 211610 
//get vec from point to average
				Point3 vec;
				vec = weldList[i].p-shape->splines[polyID]->GetKnotPoint(knotID);
				Point3 ivec,ovec;
				ivec = shape->splines[polyID]->GetInVec(knotID)+vec;
				ovec = shape->splines[polyID]->GetOutVec(knotID)+vec;
				shape->splines[polyID]->SetInVec(knotID,ivec);
				shape->splines[polyID]->SetOutVec(knotID,ovec);



				shape->splines[polyID]->SetKnotPoint(knotID,weldList[i].p);
				shape->splines[polyID]->ComputeBezPoints();

				}
			shape->InvalidateGeomCache();

			}

		if(altered) {
			shapeData->UpdateChanges(shape);
			shapeData->TempData(this)->Invalidate(PART_TOPO);
			}

		// Add the original number of knots to the oldVerts field
		for(poly = 0; poly < shape->splineCount; ++poly)
			newVerts += shape->splines[poly]->KnotCount();

		shapeData->SetFlag(ESD_BEENDONE,TRUE);
		}
	
	if(holdNeeded) {
		theHold.Accept(GetString(IDS_PW_FUSE));
		TSTR s1;
		int welded = oldVerts - newVerts;
		s1.printf(GetString(IDS_PW_FUSE), welded, oldVerts);
		ip->DisplayTempPrompt(s1,PROMPT_TIME);
		}
	else {
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearShapeDataFlag(mcList,ESD_BEENDONE);
	NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);

/*
	TimeValue t = ip->GetTime();
	BOOL holdNeeded = FALSE;
	BOOL hadSelected = FALSE;

	if ( !ip ) return;

	int oldVerts = 0;
	int newVerts = 0;
	BOOL altered = FALSE;

	theHold.Begin();
	if ( theHold.Holding() )
		theHold.Put(new SSRestore(this));
	RecordTopologyTags();
//watje now move any verts thta are within the weld tolerance onto each other
//build list 
	Tab<WeldList> weldList;
	for(int poly = 0; poly < shape.splineCount; ++poly)
		{
		BitArray pSel = shape.VertexTempSel(poly);
		for (int i = 0; i < pSel.GetSize()/3; i++)
			{
			if(pSel[(i*3)+1]) 
				{
				WeldList w;
				w.used = -1;
				w.splineID =poly;
				w.knotID = i;
				w.p = shape.splines[poly]->GetKnotPoint(i);
				weldList.Append(1,&w,1);
				}
			}
		}
	int ct = weldList.Count();
	if (ct >0)
		{
		altered = holdNeeded = TRUE;
		Point3 avgPoint(0.0f,0.0f,0.0f);
		int pointCount = 0;
		for (int j = 0; j < weldList.Count(); j++)
			{
			avgPoint += weldList[j].p;
			pointCount++;
			}
		avgPoint = avgPoint/(float)pointCount;
		for (j = 0; j < weldList.Count(); j++)
			{
			weldList[j].p = avgPoint;
			}
//now put them back in the knot list
		for (int i = 0; i < weldList.Count(); i++)
			{
			int polyID = weldList[i].splineID;
			int knotID = weldList[i].knotID;

			shape.splines[polyID]->SetKnotPoint(knotID,weldList[i].p);
			shape.splines[polyID]->ComputeBezPoints();

			}
		shape.InvalidateGeomCache();

		}


	if(holdNeeded) {
		ResolveTopoChanges();
		theHold.Accept(GetResString(IDS_PW_VERTFUSED));
		}
	else {
		theHold.End();
		}

	SelectionChanged();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
*/
	}

int EditSplineMod::NumSubObjTypes() 
{ 
	return 3;
}

ISubObjType *EditSplineMod::GetSubObjType(int i) 
{	
	static bool initialized = false;
	if(!initialized)
	{
		initialized = true;
		SOT_Vertex.SetName(GetString(IDS_TH_VERTEX));
		SOT_Segment.SetName(GetString(IDS_TH_SEGMENT));
		SOT_Spline.SetName(GetString(IDS_TH_SPLINE));
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
		return &SOT_Segment;
	case 2:
		return &SOT_Spline;
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

void EditSplineMod::ApplySoftSelectionToSpline( BezierShape * pSpline )
{
		// CAL-03/25/01: pass area selection parameters (turn into API calls in 4.y)
		BezierShapeInterface_Ex41* exInterface = (BezierShapeInterface_Ex41*) pSpline->GetInterface(BEZIER_SHAPE_INTERFACE_EX41);
		if ( exInterface ) {
			exInterface->mAreaSelect = areaSelect;
			exInterface->mUseAreaSelect = useAreaSelect;
		}

		pSpline->SetFalloff( mFalloff );
		pSpline->SetBubble( mBubble );
		pSpline->SetPinch( mPinch );
		pSpline->SetEdgeDist( mEdgeDist );
		pSpline->SetUseEdgeDists( mUseEdgeDists );
		pSpline->SetUseSoftSelections( mUseSoftSelections );
}


void EditSplineMod::SetUseSoftSelections( int useSoftSelections )
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

int EditSplineMod::UseSoftSelections()
{
	return mUseSoftSelections;
}

void EditSplineMod::SetUseEdgeDists( int useEdgeDist )
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

int EditSplineMod::UseEdgeDists()
{
	return mUseEdgeDists;
}

void EditSplineMod::InvalidateVertexWeights() {

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts( mcList, nodes );

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData * pSplineData = (EditSplineData*)mcList[i]->localData;
		if ( !pSplineData ) 
			continue;	
		
		BezierShape *pShape = pSplineData->TempData(this)->GetShape(ip->GetTime());
		if(!pShape)
			continue;

		pShape->InvalidateVertexWeights();
	}
}


void EditSplineMod::UpdateVertexWeights () {

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	if ( mUseSoftSelections ) {

		// discover total # of verts in all contexts
		//
		int totalVertexCount = 0;
		int totalVectorCount = 0;
		for ( int i = 0; i < mcList.Count(); i++ ) {
			EditSplineData * pSplineData = (EditSplineData*)mcList[i]->localData;
			if ( !pSplineData ) 
				continue;	
			
			BezierShape * pShape = pSplineData->TempData(this)->GetShape(ip->GetTime());
			if(!pShape)
				continue;

			// CAL-03/25/01: pass area selection parameters (turn into API calls in 4.y)
			BezierShapeInterface_Ex41* exInterface = (BezierShapeInterface_Ex41*) pShape->GetInterface(BEZIER_SHAPE_INTERFACE_EX41);
			if ( exInterface ) {
				exInterface->mAreaSelect = areaSelect;
				exInterface->mUseAreaSelect = useAreaSelect;
			}

			pShape->SetFalloff( mFalloff );
			pShape->SetBubble( mBubble );
			pShape->SetPinch( mPinch );
			pShape->SetEdgeDist( mEdgeDist );
			pShape->SetUseEdgeDists( mUseEdgeDists );
			pShape->SetUseSoftSelections( mUseSoftSelections );

			pShape->UpdateVertexWeights();
		}

	}

}

void EditSplineMod::UpdateVertexDists( ) 
{
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	
	// discover total # of verts in all contexts
	//
	int totalVertexCount = 0;
	int totalVectorCount = 0;
	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditSplineData * pSplineData = (EditSplineData*)mcList[i]->localData;
		if ( !pSplineData ) 
			continue;	
		
		BezierShape * pShape = pSplineData->TempData(this)->GetShape(ip->GetTime());
		if(!pShape)
			continue;

		pShape->UpdateVertexDists();
	}

}

void EditSplineMod::UpdateEdgeDists( ) 
{

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	if ( mUseEdgeDists ) {
		for ( int i = 0; i < mcList.Count(); i++ ) {
			EditSplineData * pSplineData = (EditSplineData*)mcList[i]->localData;
			if ( !pSplineData ) 
				continue;	
			
			BezierShape * pShape = pSplineData->TempData(this)->GetShape(ip->GetTime());
			if(!pShape)
				continue;

			// CAL-03/25/01: pass area selection parameters (turn into API calls in 4.y)
			BezierShapeInterface_Ex41* exInterface = (BezierShapeInterface_Ex41*) pShape->GetInterface(BEZIER_SHAPE_INTERFACE_EX41);
			if ( exInterface ) {
				exInterface->mAreaSelect = areaSelect;
				exInterface->mUseAreaSelect = useAreaSelect;
			}
			
			pShape->UpdateEdgeDists();
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


INT_PTR CALLBACK SplineSoftSelectDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
	{
	EditSplineMod *esm = (EditSplineMod *) GetWindowLongPtr( hDlg, GWLP_USERDATA );
			// WIN64 Cleanup: Shuler
	if ( !esm && message != WM_INITDIALOG ) return FALSE;

	ISpinnerControl *spin;
	Rect rect;

	switch ( message ) {
		case WM_INITDIALOG: {
		 	esm = (EditSplineMod *)lParam;
			SetWindowLongPtr( hDlg, GWLP_USERDATA, (INT_PTR)esm );		 	

			CheckDlgButton( hDlg, IDC_SOFT_SELECTION, esm->UseSoftSelections() );
			CheckDlgButton( hDlg, IDC_EDGE_DIST, esm->UseEdgeDists() );

			SetupIntSpinner   ( hDlg, IDC_EDGE_SPIN, IDC_EDGE, 1, 999, esm->mEdgeDist );
			SetupUniverseSpinner ( hDlg, IDC_FALLOFF_SPIN, IDC_FALLOFF, 0.0, 9999999.0, esm->mFalloff );
			SetupUniverseSpinner ( hDlg, IDC_PINCH_SPIN, IDC_PINCH, -10.0, 10.0, esm->mPinch );
			SetupUniverseSpinner ( hDlg, IDC_BUBBLE_SPIN, IDC_BUBBLE, -10.0, 10.0, esm->mBubble );
		 	esm->SetSoftSelDlgEnables( hDlg );
			return TRUE;
			}

		case WM_DESTROY:
			return FALSE;

		case CC_SPINNER_CHANGE:
			spin = (ISpinnerControl*)lParam;

			switch ( LOWORD(wParam) ) {

				case IDC_EDGE_SPIN:
					{
					int edgeDist = spin->GetIVal();
					esm->mEdgeDist = edgeDist;
					GetClientRectP(GetDlgItem(hDlg,IDC_AR_GRAPH),&rect);
					InvalidateRect(hDlg,&rect,FALSE);
					esm->UpdateEdgeDists();
					esm->UpdateVertexWeights();
					esm->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					esm->ip->RedrawViews(esm->ip->GetTime(),REDRAW_NORMAL);
					}
					break;
				case IDC_FALLOFF_SPIN: 
					{
					float falloff = spin->GetFVal(); 
					esm->mFalloff = falloff;
					GetClientRectP(GetDlgItem(hDlg,IDC_AR_GRAPH),&rect);
					InvalidateRect(hDlg,&rect,FALSE);
					esm->UpdateVertexWeights();
					esm->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					esm->ip->RedrawViews(esm->ip->GetTime(),REDRAW_NORMAL);
					}
					break;
				case IDC_PINCH_SPIN: 
					{
					float pinch = spin->GetFVal(); 
					esm->mPinch = pinch;
					GetClientRectP(GetDlgItem(hDlg,IDC_AR_GRAPH),&rect);
					InvalidateRect(hDlg,&rect,FALSE);
					esm->UpdateVertexWeights();
					esm->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					esm->ip->RedrawViews(esm->ip->GetTime(),REDRAW_NORMAL);
					}
					break;
				case IDC_BUBBLE_SPIN: 
					{
					float bubble = spin->GetFVal(); 
					esm->mBubble = bubble;
					GetClientRectP(GetDlgItem(hDlg,IDC_AR_GRAPH),&rect);
					InvalidateRect(hDlg,&rect,FALSE);
					esm->UpdateVertexWeights();
					esm->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					esm->ip->RedrawViews(esm->ip->GetTime(),REDRAW_NORMAL);
					}
					break;
				}
			break;
		case CC_SPINNER_BUTTONUP:
			switch( LOWORD(wParam) ) {
				// WTF? do these things ever get hit?
				//
				case IDC_BUBBLE_SPIN: 
					esm->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					esm->ip->RedrawViews(esm->ip->GetTime(),REDRAW_NORMAL);
					break;
				case IDC_PINCH_SPIN: 
					esm->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					esm->ip->RedrawViews(esm->ip->GetTime(),REDRAW_NORMAL);
					break;
				case IDC_FALLOFF_SPIN: 
					esm->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					esm->ip->RedrawViews(esm->ip->GetTime(),REDRAW_NORMAL);
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
   			esm->ip->RollupMouseMessage(hDlg,message,wParam,lParam);
			return FALSE;		
		
		case WM_COMMAND:			
			switch ( LOWORD(wParam) ) {	
				// Normals
				case IDC_SOFT_SELECTION:
					{
					// use method called SetUseSoftSelections() and do all of this work there!
					//
					int useSoftSelections = IsDlgButtonChecked(hDlg, IDC_SOFT_SELECTION);
					esm->SetUseSoftSelections( useSoftSelections );
					esm->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					esm->ip->RedrawViews(esm->ip->GetTime(),REDRAW_NORMAL);
					esm->SetSoftSelDlgEnables( hDlg );
					}
					break;
				case IDC_EDGE_DIST:
					{
					int useEdgeDist = IsDlgButtonChecked(hDlg, IDC_EDGE_DIST);
					esm->SetUseEdgeDists( useEdgeDist );
					esm->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					esm->ip->RedrawViews(esm->ip->GetTime(),REDRAW_NORMAL);
					}
					break;
				}
			break;
		}
	
	return FALSE;
	}

//az -  042803  MultiMtl sub/mtl name support
INode* GetNode (EditSplineMod *ep){
		ModContextList mcList;
		INodeTab nodes;
		ep->ip->GetModContexts (mcList, nodes);
		INode* objnode = nodes.Count() == 1 ? nodes[0]->GetActualINode(): NULL;
		nodes.DisposeTemporary();
		return objnode;
	}

void GetMtlIDListSpline(Mtl *mtl, NumList& mtlIDList){
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

void GetESplineMtlIDList(EditSplineMod *es, NumList& mtlIDList)	{
		ModContextList mcList;
		INodeTab nodes;
		if (es) {
			es->ip->GetModContexts (mcList, nodes);
			if (nodes.Count() == 1) {
				EditSplineData *shapeData = (EditSplineData*)mcList[0]->localData;
				BezierShape *shape = shapeData->TempData(es)->GetShape(es->ip->GetTime());
				int polys = shape->splineCount;
				for(int poly = 0; poly < polys; ++poly) {
					Spline3D *spline = shape->splines[poly];
					for(int seg = 0; seg < spline->Segments(); ++seg) {
						int mid = spline->GetMatID(seg);
						if (mid != -1)
							mtlIDList.Add(mid, TRUE);
					}
				}
				nodes.DisposeTemporary();
			}
		}
	}

BOOL SetupMtlSubNameCombo (HWND hWnd, EditSplineMod *es) {
	INode* singleNode;
	Mtl *nodeMtl;
	
	singleNode = GetNode(es);
	if(singleNode)
		nodeMtl = singleNode->GetMtl();
	if(singleNode == NULL || nodeMtl == NULL || !nodeMtl->IsMultiMtl()) {    //no UI for multi, cloned nodes, and not MultiMtl 
		SendMessage(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), CB_RESETCONTENT, 0, 0);
		EnableWindow(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), false);
		return false;
	}
	NumList mtlIDList;
	NumList mtlIDMeshList;
	GetMtlIDListSpline(nodeMtl, mtlIDList);
	GetESplineMtlIDList(es, mtlIDMeshList);
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

void UpdateNameComboSpline (HWND hWnd, ISpinnerControl *spin) {
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


void ValidateUINameCombo (HWND hWnd, EditSplineMod *es) {
	SetupMtlSubNameCombo (hWnd, es);
	ISpinnerControl *spin;
	spin = GetISpinner(GetDlgItem(hWnd,IDC_MAT_IDSPIN));
	if (spin)
		UpdateNameComboSpline (hWnd, spin);
	ReleaseISpinner(spin);
}
