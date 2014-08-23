/**********************************************************************
 *<
	FILE: spline.cpp

	DESCRIPTION:  A spline object implementation

	CREATED BY: Tom Hudson

	HISTORY: created 23 February 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/
#include "prim.h"

#ifndef NO_OBJECT_SHAPES_SPLINES

#include "splshape.h"
#include "linshape.h"
#include "iparamm.h"
#include "evrouter.h"
#include "simpspl.h"	// Here for loading old files

// Knot type indexes for creation and a handy lookup table

#define INDEX_CORNER 0
#define INDEX_SMOOTH 1
#define INDEX_BEZIER 2
static int KnotTypeTable[] = { KTYPE_CORNER, KTYPE_AUTO, KTYPE_BEZIER };

class SplineObject;

class SplineObjCreateCallBack: public CreateMouseCallBack {
	SplineObject *ob;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		int override(int mode) { return CLICK_DOWN_POINT; }		// Override the mouse manager's mode!
		void SetObj(SplineObject *obj);
	};

class SplineObject: public SplineShape, public IParamArray {			   
	friend class SplineObjCreateCallBack;
	friend BOOL CALLBACK SplineParamDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
	friend class SplineTypeInDlgProc;
	friend class BackspaceUser;

	public:
		// Class vars
		static IParamMap *pmapCreate;
		static IParamMap *pmapTypeIn;
		static Point3 crtPos;		
		static int crtInitialType;
		static int crtDragType;
		static Spline3D TIspline;	// Type-in construction spline
		static BOOL useTI;			// If TRUE, constructor copies TIspline into our work spline & resets this
		static int dlgSteps;
		static int dlgShapeSteps;
		static BOOL dlgOptimize;
#ifdef DESIGN_VER
        static int crtOrthoInitType;
        static int crtOrthoDragType;
#endif

		// Pointer to the create callback
		SplineObjCreateCallBack *ccb;

		// Flag set when initial creation is complete.
		BOOL createDone;

		// Special flag for loading old files -- Temporarily sets up refs
		BOOL loadingOldFile;

		// Old interpolation parameter block (used when loading old files)
		IParamBlock *nullpblock;	// Old one not used
		IParamBlock *ipblock;	// Interpolation parameter block

		// Flag to suspend snapping -- Used during creation
		BOOL suspendSnap;

		SplineObject();
		~SplineObject();

		//  inherited virtual methods:

		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		TCHAR *GetObjectName() { return GetString(IDS_TH_LINE); }
		void InitNodeName(TSTR& s) { s = GetString(IDS_TH_LINE); }		
		Class_ID ClassID() { return Class_ID(SPLINE3D_CLASS_ID,0); }  
		void GetClassName(TSTR& s) { s = TSTR(GetString(IDS_TH_LINE_CLASS)); }
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		int NumRefs();
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);		
		int RemapRefOnLoad(int iref);

		void DeleteThis();

		// From IParamArray
		BOOL SetValue(int i, TimeValue t, int v);
		BOOL SetValue(int i, TimeValue t, float v);
		BOOL SetValue(int i, TimeValue t, Point3 &v);
		BOOL GetValue(int i, TimeValue t, int &v, Interval &ivalid);
		BOOL GetValue(int i, TimeValue t, float &v, Interval &ivalid);
		BOOL GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid);

		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);

		void AssignSpline(Spline3D *s);
		void FlushTypeInSpline() { TIspline.NewSpline(); useTI = FALSE; }
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		BOOL ValidForDisplay(TimeValue t);

		void CenterPivot(IObjParam *ip);

		// From SplineShape (Editable Spline)
		Object* ConvertToType(TimeValue t, Class_ID obtype);

		// From ISplineSelect, ISplineOps, etc.  These are pure virtual, but SplineShape implements them so bounce to it.  JBW 2/1/99
		void StartCommandMode(splineCommandMode mode) { SplineShape::StartCommandMode(mode); }
		void ButtonOp(splineButtonOp opcode) { SplineShape::ButtonOp(opcode); }
		DWORD GetSelLevel() { return SplineShape::GetSelLevel(); }
		void SetSelLevel(DWORD level) { SplineShape::SetSelLevel(level); }
		void LocalDataChanged() { SplineShape::LocalDataChanged(); }
		BitArray GetVertSel() { return SplineShape::GetVertSel(); }
		BitArray GetSegmentSel() { return SplineShape::GetSegmentSel(); }
		BitArray GetSplineSel() { return SplineShape::GetSplineSel(); }
		void SetVertSel(BitArray &set, ISplineSelect *imod, TimeValue t) { SplineShape::SetVertSel(set, imod, t); }
		void SetSegmentSel(BitArray &set, ISplineSelect *imod, TimeValue t) { SplineShape::SetSegmentSel(set, imod, t); }
		void SetSplineSel(BitArray &set, ISplineSelect *imod, TimeValue t) { SplineShape::SetSplineSel(set, imod, t); }
		GenericNamedSelSetList& GetNamedVertSelList() { return SplineShape::GetNamedVertSelList(); }
		GenericNamedSelSetList& GetNamedSegmentSelList() { return SplineShape::GetNamedSegmentSelList(); }
		GenericNamedSelSetList& GetNamedSplineSelList() { return SplineShape::GetNamedSplineSelList(); }

		// IO
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
	};				

//------------------------------------------------------

class SplineObjClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new SplineObject; }
	const TCHAR *	ClassName() { return GetString(IDS_TH_LINE_CLASS); }
	SClass_ID		SuperClassID() { return SHAPE_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(SPLINE3D_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_TH_SPLINES);  }
	void			ResetClassParams(BOOL fileReset);
	};

static SplineObjClassDesc splineObjDesc;

ClassDesc* GetSplineDesc() { return &splineObjDesc; }

#ifdef DESIGN_VER
class OrthoSplineObjClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) {
                        SplineObject *so = new SplineObject;
                        if (so != NULL)
                            so->shape.dispFlags |= DISP_SPLINES_ORTHOG;
                        return so;
                        }
    const TCHAR *	ClassName() { return GetString(IDS_PRS_ORTHOLINE_CLASS); }
	SClass_ID		SuperClassID() { return SHAPE_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(0x640f7dd1, 0x6b164fdc); }
	const TCHAR* 	Category() { return GetString(IDS_TH_SPLINES);  }
	void			ResetClassParams(BOOL fileReset);
	};

static OrthoSplineObjClassDesc orthoSplineObjDesc;

ClassDesc* GetOrthoSplineDesc() { return &orthoSplineObjDesc; }
#endif

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// class variable for spline class.
int SplineObject::dlgSteps     = DEF_STEPS;
int SplineObject::dlgShapeSteps = DEF_STEPS;
BOOL SplineObject::dlgOptimize = DEF_OPTIMIZE;
IParamMap *SplineObject::pmapCreate = NULL;
IParamMap *SplineObject::pmapTypeIn = NULL;
IObjParam *SplineObject::ip         = NULL;
Point3 SplineObject::crtPos         = Point3(0,0,0);
int SplineObject::crtInitialType    = INDEX_CORNER;
int SplineObject::crtDragType       = INDEX_BEZIER;
#ifdef DESIGN_VER
int SplineObject::crtOrthoInitType  = INDEX_CORNER;
int SplineObject::crtOrthoDragType  = INDEX_CORNER;
#endif

// Type-in creation statics:
Spline3D SplineObject::TIspline;
BOOL SplineObject::useTI            = FALSE;

void SplineObjClassDesc::ResetClassParams(BOOL fileReset)
	{
	SplineObject::crtPos         = Point3(0,0,0);
	SplineObject::crtInitialType = INDEX_CORNER;
	SplineObject::crtDragType    = INDEX_BEZIER;
	SplineObject::useTI          = FALSE;
	SplineObject::dlgSteps       = DEF_STEPS;
	SplineObject::dlgShapeSteps  = DEF_STEPS;
	SplineObject::dlgOptimize    = DEF_OPTIMIZE;
#ifdef DESIGN_VER
    SplineObject::crtOrthoInitType  = INDEX_CORNER;
    SplineObject::crtOrthoDragType  = INDEX_CORNER;
#endif
	}

#ifdef DESIGN_VER
void OrthoSplineObjClassDesc::ResetClassParams(BOOL fileReset)
	{
	SplineObject::crtPos         = Point3(0,0,0);
	SplineObject::crtInitialType = INDEX_CORNER;
	SplineObject::crtDragType    = INDEX_CORNER;
	SplineObject::useTI          = FALSE;
	SplineObject::dlgSteps       = DEF_STEPS;
	SplineObject::dlgShapeSteps  = DEF_STEPS;
	SplineObject::dlgOptimize    = DEF_OPTIMIZE;
    SplineObject::crtOrthoInitType  = INDEX_CORNER;
    SplineObject::crtOrthoDragType  = INDEX_CORNER;
	}
#endif

// Parameter map indices
#define PB_DUMMY		0	// Not actually used!

// Non-parameter block indices
#define PB_INITIALTYPE		0
#define PB_DRAGTYPE			1
#define PB_TI_POS			2
// next two only for DESIGN_VER -- OrthoLine feature
#define PB_O_INITIALTYPE    3
#define PB_O_DRAGTYPE       4

//
//
// Creation Parameters

static int initialTypeIDs[] = {IDC_ICORNER,IDC_ISMOOTH};
static int dragTypeIDs[] = {IDC_DCORNER,IDC_DSMOOTH,IDC_DBEZIER};

static ParamUIDesc descCreate[] = {
	// Initial point type	
	ParamUIDesc(PB_INITIALTYPE,TYPE_RADIO,initialTypeIDs,2),
	// Dragged point type
	ParamUIDesc(PB_DRAGTYPE,TYPE_RADIO,dragTypeIDs,3)
	};
static ParamUIDesc descOrthoCreate[] = {
	// Initial point type	
	ParamUIDesc(PB_O_INITIALTYPE,TYPE_RADIO,initialTypeIDs,2),
	// Dragged point type
	ParamUIDesc(PB_O_DRAGTYPE,TYPE_RADIO,dragTypeIDs,3)
	};
#define CREATEDESC_LENGTH 2

//
//
// Type in

static ParamUIDesc descTypeIn[] = {
	
	// Position
	ParamUIDesc(
		PB_TI_POS,
		EDITTYPE_UNIVERSE,
		IDC_TI_POSX,IDC_TI_POSXSPIN,
		IDC_TI_POSY,IDC_TI_POSYSPIN,
		IDC_TI_POSZ,IDC_TI_POSZSPIN,
		-99999999.0f,99999999.0f,
		SPIN_AUTOSCALE)
	
	};
#define TYPEINDESC_LENGTH 1

//--- TypeInDlgProc --------------------------------

class SplineTypeInDlgProc : public ParamMapUserDlgProc {
	public:
		SplineObject *ob;

		SplineTypeInDlgProc(SplineObject *n) {ob=n;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

BOOL SplineTypeInDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	static Point3 previous;
	switch (msg) {
		case WM_DESTROY:
			ob->suspendSnap = FALSE;
			ob->FlushTypeInSpline();
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_TI_ADDPOINT: {
					// Return focus to the top spinner
					SetFocus(GetDlgItem(hWnd, IDC_TI_POSX));
					ob->suspendSnap = TRUE;
					// If first point, start up the new object
					if(ob->TIspline.KnotCount() == 0) {
						Matrix3 tm(1);
						previous = ob->crtPos;
						SplineObject *newob = (SplineObject *)ob->ip->NonMouseCreate(tm);
						newob->TIspline.AddKnot(SplineKnot(KnotTypeTable[ob->crtInitialType],LTYPE_CURVE,ob->crtPos,ob->crtPos,ob->crtPos));
						// Stuff the spline into the shape
						while(newob->shape.SplineCount())
							newob->shape.DeleteSpline(0);
						Spline3D *spline = newob->shape.NewSpline();
						*spline = newob->TIspline;
						newob->shape.UpdateSels();	// Make sure it readies the selection set info
						newob->InvalidateGeomCache();
						return TRUE;
						}					
					if(ob->TIspline.KnotCount() > 0 && ob->crtPos == previous) {
						ob->ip->DisplayTempPrompt(GetString(IDS_TH_ERRORTWOCONSECUTIVE));
						Beep(800, 100);
						return TRUE;
						}
					previous = ob->crtPos;
					ob->TIspline.AddKnot(SplineKnot(KnotTypeTable[ob->crtInitialType],LTYPE_CURVE,ob->crtPos,ob->crtPos,ob->crtPos));
					// Stuff the revised spline into the shape
					while(ob->shape.SplineCount())
						ob->shape.DeleteSpline(0);
					Spline3D *spline = ob->shape.NewSpline();
					*spline = ob->TIspline;
					ob->shape.UpdateSels();	// Make sure it readies the selection set info
					ob->InvalidateGeomCache();
					ob->NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
					return TRUE;
					}
				case IDC_TI_CLOSE:
					// Return focus to the top spinner
					SetFocus(GetDlgItem(hWnd, IDC_TI_POSX));
					
					if(ob->TIspline.KnotCount() > 1) {
						ob->TIspline.SetClosed();

						common_finish:
						ob->suspendSnap = FALSE;
						ob->TIspline.ComputeBezPoints();
						ob->AssignSpline(&ob->TIspline);
						ob->CenterPivot(ob->ip);

						// Zero out the type-in spline
						ob->FlushTypeInSpline();
						return TRUE;
						}
					else {
						ob->ip->DisplayTempPrompt(GetString(IDS_TH_NEEDTWOSPLINEPOINTS));
						Beep(500, 100);
						}
					break;
				case IDC_TI_FINISH:
					if(ob->TIspline.KnotCount() > 1)
						goto common_finish;
					else {
						ob->ip->DisplayTempPrompt(GetString(IDS_TH_NEEDTWOSPLINEPOINTS));
						Beep(500, 100);
						}
					break;
				}
			break;	
		}
	return FALSE;
	}

void SplineObject::CenterPivot(IObjParam *ip) {
	// Find the center of the knot points
	Point3 composite(0,0,0);
	Spline3D *spline = shape.splines[0];
	int knots = spline->KnotCount();
	for(int i = 0; i < knots; ++i)
		composite += spline->GetKnotPoint(i);
	composite /= (float)knots;
	// Translate the entire spline to this new center
	Matrix3 xlate = TransMatrix(-composite);
	spline->Transform(&xlate);
	InvalidateGeomCache();
	// Set the new matrix center
	Matrix3 tm(1);
	tm.SetTrans(composite);
	// Assign the revised transform
	ip->NonMouseCreateFinish(tm);
	}

void SplineObject::BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev)
	{
	SplineShape::BeginEditParams(ip,flags,prev);
	// This shouldn't be necessary, but the ip is getting hosed after the previous call!
	this->ip = ip;

	if (pmapCreate) {

        // Left over from last one created
        pmapCreate->SetParamBlock(this);
        pmapTypeIn->SetParamBlock(this);
    } else {
		
		// Gotta make a new one.
		if (flags&BEGIN_EDIT_CREATE) {
			pmapCreate = CreateCPParamMap(
                (shape.dispFlags & DISP_SPLINES_ORTHOG) ?
                                                descOrthoCreate : descCreate,
                CREATEDESC_LENGTH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_SPLINEPARAM1),
				GetString(IDS_TH_CREATION_METHOD),
				0);

			pmapTypeIn = CreateCPParamMap(
				descTypeIn,TYPEINDESC_LENGTH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_SPLINEPARAM2),
				GetString(IDS_TH_KEYBOARD_ENTRY),
				APPENDROLL_CLOSED);
			}
		}

    if(pmapTypeIn) {
		// A callback for the type in.
		pmapTypeIn->SetUserDlgProc(new SplineTypeInDlgProc(this));
		}
	}
		
void SplineObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
	{
	if(TIspline.KnotCount()) {
		if(TIspline.KnotCount() == 1) {
			Spline3D *spline = shape.splines[0];
			Point3 p = spline->GetKnotPoint(0);
			spline->AddKnot(SplineKnot(KnotTypeTable[crtInitialType],LTYPE_CURVE,p,p,p));
			shape.UpdateSels();	// Make sure it readies the selection set info
			InvalidateGeomCache();
			}
		CenterPivot(ip);
		suspendSnap = FALSE;
		NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
		FlushTypeInSpline();		
		}

	SplineShape::EndEditParams(ip,flags,next);

	if (flags&END_EDIT_REMOVEUI ) {
		if (pmapCreate) DestroyCPParamMap(pmapCreate);
		if (pmapTypeIn) DestroyCPParamMap(pmapTypeIn);
		pmapTypeIn = NULL;
		pmapCreate = NULL;
		}
	// Save these values in class variables so the next object created will inherit them.
	dlgSteps = steps;
	dlgShapeSteps = shape.steps;
	dlgOptimize = shape.optimize;
	}

SplineObject::SplineObject() : SplineShape() 
	{
	ccb = NULL;
	createDone = FALSE;
	loadingOldFile = FALSE;
	suspendSnap = FALSE;
	nullpblock = NULL;
	ipblock = NULL;
	if(useTI) {
		AssignSpline(&TIspline);
		FlushTypeInSpline();
		}
	steps = dlgSteps;
	shape.steps = dlgShapeSteps;
	shape.optimize = dlgOptimize;
	}

SplineObject::~SplineObject()
	{
	if(ccb)
		ccb->SetObj(NULL);
	DeleteAllRefsFromMe();
	}

class BackspaceUser : public EventUser {
	SplineObject *ob;
	public:
		void Notify();
		void SetObj(SplineObject *obj) { ob = obj; }
	};

void BackspaceUser::Notify() {
	Spline3D *spline = ob->shape.GetSpline(0);
	if(spline->KnotCount() > 2) {
		// Tell the spline we just backspaced to remove the last point
		spline->Create(NULL, -1, 0, 0, IPoint2(0,0), NULL,ob->ip);
		ob->InvalidateGeomCache();
		ob->NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
		ob->ip->RedrawViews(ob->ip->GetTime(),REDRAW_INTERACTIVE);
		}
	}

static BackspaceUser pBack;

void SplineObjCreateCallBack::SetObj(SplineObject *obj) {
	ob = obj;
	}

int SplineObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
	// If the callback is being turned off, punt!
	if(msg == MOUSE_UNINIT)
		return TRUE;
	// If the object we're part of is gone, punt!
	if(!ob)
		return TRUE;

#ifdef _OSNAP
		if(msg == MOUSE_FREEMOVE)
		{
			vpt->SnapPreview(m,m,NULL,SNAP_IN_3D);
			return TRUE;
		}
#endif

	if(ob->createDone) {
		ob->suspendSnap = FALSE;
		return CREATE_STOP;
		}
	// CAL-11/15/2002: When SetMouseProc() is called to change the mouse proc. It will send a MOUSE_ABORT
	//		message with point == 0 to the old mouse proc. We want to treat it as aborting the last knot
	//		and finishing the creation of the spline object. Add a "msg != MOUSE_ABORT" checking. (465673)
	if (point == 0 && msg != MOUSE_ABORT) {
		ob->suspendSnap = TRUE;
		pBack.SetObj(ob);
		backspaceRouter.Register(&pBack);
		ob->shape.NewShape();
#ifdef DESIGN_VER
        if (ob->shape.dispFlags & DISP_SPLINES_ORTHOG)
		    ob->shape.NewSpline(KnotTypeTable[ob->crtOrthoInitType],
                                KnotTypeTable[ob->crtOrthoDragType]);
        else
#endif
		    ob->shape.NewSpline(KnotTypeTable[ob->crtInitialType],
                                KnotTypeTable[ob->crtDragType]);
		ob->FlushTypeInSpline();	// Also get rid of the type-in spline, which we aren't using
		}
	Spline3D *theSpline = ob->shape.GetSpline(0);
	if(!theSpline) {
		assert(0);
		return FALSE;
		}
	int res = theSpline->Create(vpt,msg,point,flags,m,NULL,ob->ip);
	switch(res) {
		case CREATE_ABORT:
			ob->suspendSnap = FALSE;
			ob->createDone = TRUE;
			backspaceRouter.UnRegister(&pBack);
			return res;
		case CREATE_STOP:
			ob->suspendSnap = FALSE;
			ob->createDone = TRUE;
			if(theSpline->KnotCount() == 0) {
				backspaceRouter.UnRegister(&pBack);
				return CREATE_ABORT;
				}
			// Find the center of the knot points
			Point3 composite(0,0,0);
			for(int i = 0; i < theSpline->KnotCount(); ++i)
				composite += theSpline->GetKnotPoint(i);
			composite /= (float)theSpline->KnotCount();
			// Translate the entire spline to this new center
			Matrix3 xlate = TransMatrix(-composite);
			theSpline->Transform(&xlate);
			// Set the new matrix center
			mat.SetTrans(composite * mat);
			backspaceRouter.UnRegister(&pBack);
			break;
		}
	ob->shape.UpdateSels();	// Make sure it readies the selection set info
	ob->InvalidateGeomCache();
	ob->NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	return res;
	}

static SplineObjCreateCallBack splineCreateCB;

CreateMouseCallBack* SplineObject::GetCreateMouseCallBack() {
	splineCreateCB.SetObj(this);
	ccb = &splineCreateCB;
	return(&splineCreateCB);
	}

//
// Reference Managment:
//

RefTargetHandle SplineObject::Clone(RemapDir& remap) {
	SplineObject* newob = new SplineObject();
	newob->SplineShapeClone(this);
	BaseClone(this, newob, remap);
	return(newob);
	}

ParamDimension *SplineObject::GetParameterDim(int pbIndex) 
	{
//	switch (pbIndex) {
//		}
	return defaultDim;
	}

TSTR SplineObject::GetParameterName(int pbIndex) 
	{
//	switch (pbIndex) {
//		}
	return TSTR(_T(""));
	}

void SplineObject::AssignSpline(Spline3D *s) {
	shape.NewShape();
	*(shape.NewSpline()) = *s;
	shape.UpdateSels();
	InvalidateGeomCache();
	}

// From ParamArray
BOOL SplineObject::SetValue(int i, TimeValue t, int v) 
	{
	switch (i) {
		case PB_INITIALTYPE: crtInitialType = v; break;
		case PB_DRAGTYPE: crtDragType = v; break;
#ifdef DESIGN_VER
		case PB_O_INITIALTYPE: crtOrthoInitType = v; break;
		case PB_O_DRAGTYPE: crtOrthoDragType = v; break;
#endif
		}		
	return TRUE;
	}

BOOL SplineObject::SetValue(int i, TimeValue t, float v)
	{
//	switch (i) {				
//		}	
	return TRUE;
	}

BOOL SplineObject::SetValue(int i, TimeValue t, Point3 &v) 
	{
	switch (i) {
		case PB_TI_POS: crtPos = v; break;
		}		
	return TRUE;
	}

BOOL SplineObject::GetValue(int i, TimeValue t, int &v, Interval &ivalid) 
	{
	switch (i) {
		case PB_INITIALTYPE: v = crtInitialType; break;
		case PB_DRAGTYPE: v = crtDragType; break;
#ifdef DESIGN_VER
		case PB_O_INITIALTYPE: v = crtOrthoInitType; break;
		case PB_O_DRAGTYPE: v = crtOrthoDragType; break;
#endif
		}
	return TRUE;
	}

BOOL SplineObject::GetValue(int i, TimeValue t, float &v, Interval &ivalid) 
	{	
//	switch (i) {		
//		}
	return TRUE;
	}

BOOL SplineObject::GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_POS: v = crtPos; break;		
		}
	return TRUE;
	}

#define DISP_NODEFAULTCOLOR (1<<12)
#define COMP_NODEFAULTCOLOR (1<<10)

int SplineObject::Display(TimeValue t, INode *inode, ViewExp *vpt, int flags) {
	if(!ValidForDisplay(t))
		return 0;

	BOOL ticksSet = FALSE;
	if(!drawTicks && suspendSnap) {
		drawTicks = TRUE;
		ticksSet = TRUE;
		}

	SplineShape::Display(t, inode, vpt, flags);

	// Turn off the ticks if we set 'em
	if(ticksSet)
		drawTicks = FALSE;

	return(0);
	}

BOOL SplineObject::ValidForDisplay(TimeValue t) {
	if(shape.SplineCount() == 0)
		return FALSE;
	return (shape.GetSpline(0)->KnotCount() > 1) ? TRUE : FALSE;
	}

// We're based on the SplineShape object, but the conversion to an editable spline
// should turn us into a general SplineShape, not the Line object that we are.
// If asked to turn into a SplineShape, we'll do the conversion, otherwise, we'll
// let the SplineShape code do the work, which may mean just returning a pointer
// to ourself.
Object* SplineObject::ConvertToType(TimeValue t, Class_ID obtype) {
#ifdef OLDCODE // TH 6/24/99 Removed this, it was wreaking havoc in the pipeline
	if (obtype == splineShapeClassID) {
		SplineShape* newob = new SplineShape;
		// Gotta copy the ShapeObject parts
		newob->CopyBaseData(*this);
		theHold.Suspend();	// TH 4/9/99
		*newob = *this;
		theHold.Resume();	// TH 4/9/99
		newob->SetChannelValidity(TOPO_CHAN_NUM,ConvertValidity(t));
		newob->SetChannelValidity(GEOM_CHAN_NUM,ConvertValidity(t));
		return newob;
		}
#endif
	return SplineShape::ConvertToType(t, obtype);
	}

int SplineObject::NumRefs() {
	if(loadingOldFile)
		return 2;
	return SplineShape::NumRefs();
	}

RefTargetHandle SplineObject::GetReference(int i) {
//DebugPrint("GetReference(%d):",i);
	if(loadingOldFile) {
		if(i == 0) {
//DebugPrint("%p [nullpblock]\n",nullpblock);
			return nullpblock;
			}
//DebugPrint("%p [ipblock]\n",ipblock);
		return ipblock;
		}
//DebugPrint("%p [SS]\n",SplineShape::GetReference(i));
	return SplineShape::GetReference(i);
	}

void SplineObject::SetReference(int i, RefTargetHandle rtarg) {
//SClass_ID scid = rtarg->SuperClassID();
//Class_ID cid = rtarg->ClassID();
//DebugPrint("SetReference(%d, %p) SC:%X CID:%X:%X --",i, rtarg, scid, cid.PartA(), cid.PartB());
	if(loadingOldFile) {
		if(i == 0) {
//DebugPrint(" [nullpblock]\n");
			nullpblock = (IParamBlock*)rtarg;
			return;
			}
//DebugPrint(" [ipblock]\n");
		ipblock=(IParamBlock*)rtarg;
//int nparams = ipblock->NumParams();
//DebugPrint("pblock params:%d\n",nparams);
//for(int p = 0; p < nparams; ++p)
//  DebugPrint("Param %d type %d\n", p, ipblock->GetParameterType(p));
		return;					   
		}
//DebugPrint(" [SS]\n");
	SplineShape::SetReference(i, rtarg);
	}		

int SplineObject::RemapRefOnLoad(int iref) {
//DebugPrint("SO RemapRefOnLoad called (%d)",iref);
	if(loadingOldFile) {
//DebugPrint("Loading old, left as is\n");
		return iref;
		}
	return SplineShape::RemapRefOnLoad(iref);
	}

void SplineObject::DeleteThis() {
	if(ccb)
		ccb->SetObj(NULL);
	delete this;
	}

// IO
#define OLD_SPLINE_CHUNK 0x1000
#define NEW_SPLINE_CHUNK 0x2000

IOResult SplineObject::Save(ISave *isave) {
	isave->BeginChunk(NEW_SPLINE_CHUNK);
	SplineShape::Save(isave);
	isave->EndChunk();
	return IO_OK;
	}

class SplinePostLoadCallback : public PostLoadCallback {
	public:
		SplineObject *so;
		SplinePostLoadCallback(SplineObject *s) {so=s;}
		void proc(ILoad *iload) {
			int steps, optimize, adaptive;
			if(so->ipblock) {
				so->ipblock->GetValue(IPB_STEPS, 0, steps, FOREVER);
				so->ipblock->GetValue(IPB_OPTIMIZE, 0, optimize, FOREVER);
				so->ipblock->GetValue(IPB_ADAPTIVE, 0, adaptive, FOREVER);
				so->steps = so->shape.steps = steps;
				so->shape.optimize = optimize;
				if(adaptive)
					so->shape.steps = -1;
				}
			so->DeleteReference(0);
			so->DeleteReference(1);
			so->loadingOldFile = FALSE;
			delete this;
			}
	};

IOResult  SplineObject::Load(ILoad *iload) {
	IOResult res;
	loadingOldFile = FALSE;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case OLD_SPLINE_CHUNK: {	// Pre-2.0 line object chunk
				loadingOldFile = TRUE;
				iload->RegisterPostLoadCallback(new SplinePostLoadCallback(this));
				shape.NewShape();
				Spline3D *s = shape.NewSpline();
				res = s->Load(iload);
				iload->SetObsolete();
				shape.UpdateSels();
				InvalidateGeomCache();
				}
				break;
			case NEW_SPLINE_CHUNK:
				SplineShape::Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

#endif // NO_OBJECT_SHAPES_SPLINES
