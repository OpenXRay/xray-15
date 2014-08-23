/**********************************************************************
 *<
	FILE: circle.cpp

	DESCRIPTION:  A Circle object implementation

	CREATED BY: Tom Hudson

	HISTORY: created 29 October 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/
#include "prim.h"

#ifndef NO_OBJECT_SHAPES_SPLINES

#include "splshape.h"
#include "iparamm.h"
// This is based on the simple spline object...
#include "simpspl.h"


#define MIN_RADIUS		float(0)
#define MAX_RADIUS		float( 1.0E30)

#define DEF_RADIUS		float(0.0)

class CircleObjCreateCallBack;

class CircleObject: public SimpleSpline, public IParamArray {			   

	friend class CircleObjCreateCallBack;
	
	public:
		// Class vars
		static IParamMap *pmapCreate;
		static IParamMap *pmapTypeIn;
		static IParamMap *pmapParam;
		static IObjParam *ip;
		static int dlgCreateMeth;
		static Point3 crtPos;		
		static float crtRadius;
		
		void BuildShape(TimeValue t,BezierShape& ashape);

		CircleObject();
		~CircleObject();

		//  inherited virtual methods:

		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		TCHAR *GetObjectName() { return GetString(IDS_TH_CIRCLE); }
		void InitNodeName(TSTR& s) { s = GetString(IDS_TH_CIRCLE); }		
		Class_ID ClassID() { return Class_ID(CIRCLE_CLASS_ID,0); }  
		void GetClassName(TSTR& s) { s = TSTR(GetString(IDS_TH_CIRCLE_CLASS)); }
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		BOOL ValidForDisplay(TimeValue t);

 		// From IParamArray
		BOOL SetValue(int i, TimeValue t, int v);
		BOOL SetValue(int i, TimeValue t, float v);
		BOOL SetValue(int i, TimeValue t, Point3 &v);
		BOOL GetValue(int i, TimeValue t, int &v, Interval &ivalid);
		BOOL GetValue(int i, TimeValue t, float &v, Interval &ivalid);
		BOOL GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid);

		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);

		void InvalidateUI() { if (pmapParam) pmapParam->Invalidate(); }

		// IO
		IOResult Load(ILoad *iload);
	};				

//------------------------------------------------------

class CircleObjClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new CircleObject; }
	const TCHAR *	ClassName() { return GetString(IDS_TH_CIRCLE_CLASS); }
	SClass_ID		SuperClassID() { return SHAPE_CLASS_ID; }
   	Class_ID		ClassID() { return Class_ID(CIRCLE_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_TH_SPLINES);  }
	void			ResetClassParams(BOOL fileReset);
	};

static CircleObjClassDesc circleObjDesc;

ClassDesc* GetCircleDesc() { return &circleObjDesc; }

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// class variable for circle class.
IParamMap *CircleObject::pmapCreate = NULL;
IParamMap *CircleObject::pmapParam  = NULL;
IParamMap *CircleObject::pmapTypeIn = NULL;
IObjParam *CircleObject::ip         = NULL;
Point3 CircleObject::crtPos         = Point3(0,0,0);
float CircleObject::crtRadius       = 0.0f;
int CircleObject::dlgCreateMeth = 1; // create_radius

void CircleObjClassDesc::ResetClassParams(BOOL fileReset)
	{
	CircleObject::crtPos          = Point3(0,0,0);
	CircleObject::crtRadius       = 0.0f;
	CircleObject::dlgCreateMeth   = 1; // create_radius
	}

// Parameter map indices
#define PB_RADIUS		0

// Non-parameter block indices
#define PB_CREATEMETHOD		0
#define PB_TI_POS			1
#define PB_TI_RADIUS		2

// Vector length for unit circle
#define CIRCLE_VECTOR_LENGTH 0.5517861843f

//
//
//	Creation method

static int createMethIDs[] = {IDC_CREATEDIAMETER,IDC_CREATERADIUS};

static ParamUIDesc descCreate[] = {
	// Diameter/radius
	ParamUIDesc(PB_CREATEMETHOD,TYPE_RADIO,createMethIDs,2)
	};
#define CREATEDESC_LENGTH 1

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
		SPIN_AUTOSCALE),
	
	// Radius
	ParamUIDesc(
		PB_TI_RADIUS,
		EDITTYPE_UNIVERSE,
		IDC_RADIUS,IDC_RADSPINNER,
		MIN_RADIUS,MAX_RADIUS,
		SPIN_AUTOSCALE)
			
	};
#define TYPEINDESC_LENGTH 2

//
//
// Parameters

static ParamUIDesc descParam[] = {
	// Radius
	ParamUIDesc(
		PB_RADIUS,
		EDITTYPE_UNIVERSE,
		IDC_RADIUS,IDC_RADSPINNER,
		MIN_RADIUS,MAX_RADIUS,
		SPIN_AUTOSCALE),	
	
	};
#define PARAMDESC_LENGTH 1


static ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },		
	{ TYPE_FLOAT, NULL, TRUE, 1 } };

static ParamBlockDescID descVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 } };		
#define PBLOCK_LENGTH	1

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,2,0)
	};
#define NUM_OLDVERSIONS	1

// Current version
#define CURRENT_VERSION	1
static ParamVersionDesc curVersion(descVer1,PBLOCK_LENGTH,CURRENT_VERSION);

//--- TypeInDlgProc --------------------------------

class CircleTypeInDlgProc : public ParamMapUserDlgProc {
	public:
		CircleObject *co;

		CircleTypeInDlgProc(CircleObject *c) {co=c;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

BOOL CircleTypeInDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_TI_CREATE: {
					if (co->crtRadius==0.0) return TRUE;

					// Return focus to the top spinner
					SetFocus(GetDlgItem(hWnd, IDC_TI_POSX));
					
					// We only want to set the value if the object is 
					// not in the scene.
					if (co->TestAFlag(A_OBJ_CREATING)) {
						co->pblock->SetValue(PB_RADIUS,0,co->crtRadius);
						}

					Matrix3 tm(1);
					tm.SetTrans(co->crtPos);
					co->ip->NonMouseCreate(tm);
					// NOTE that calling NonMouseCreate will cause this
					// object to be deleted. DO NOT DO ANYTHING BUT RETURN.
					return TRUE;	
					}
				}
			break;	
		}
	return FALSE;
	}

void CircleObject::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
	{
	SimpleSpline::BeginEditParams(ip,flags,prev);
	this->ip = ip;

	if (pmapCreate && pmapParam) {
		
		// Left over from last circle ceated
		pmapCreate->SetParamBlock(this);
		pmapTypeIn->SetParamBlock(this);
		pmapParam->SetParamBlock(pblock);
	} else {
		
		// Gotta make a new one.
		if (flags&BEGIN_EDIT_CREATE) {
			pmapCreate = CreateCPParamMap(
				descCreate,CREATEDESC_LENGTH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_CIRCLEPARAM1),
				GetString(IDS_TH_CREATION_METHOD),
				0);

			pmapTypeIn = CreateCPParamMap(
				descTypeIn,TYPEINDESC_LENGTH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_CIRCLEPARAM3),
				GetString(IDS_TH_KEYBOARD_ENTRY),
				APPENDROLL_CLOSED);
			}

		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_CIRCLEPARAM2),
			GetString(IDS_TH_PARAMETERS),
			0);
		}

	if(pmapTypeIn) {
		// A callback for the type in.
		pmapTypeIn->SetUserDlgProc(new CircleTypeInDlgProc(this));
		}
	}
		
void CircleObject::EndEditParams( IObjParam *ip,ULONG flags,Animatable *next )
	{
	SimpleSpline::EndEditParams(ip,flags,next);
	this->ip = NULL;

	if (flags&END_EDIT_REMOVEUI ) {
		if (pmapCreate) DestroyCPParamMap(pmapCreate);
		if (pmapTypeIn) DestroyCPParamMap(pmapTypeIn);
		DestroyCPParamMap(pmapParam);
		pmapParam  = NULL;
		pmapTypeIn = NULL;
		pmapCreate = NULL;
		}

	// Save these values in class variables so the next object created will inherit them.
	}

static void MakeCircle(BezierShape& ashape, float radius) {
	float vector = CIRCLE_VECTOR_LENGTH * radius;
	// Delete all points in the existing spline
	Spline3D *spline = ashape.NewSpline();

	// Now add all the necessary points
	for(int ix=0; ix<4; ++ix) {
		float angle = 6.2831853f * (float)ix / 4.0f;
		float sinfac = (float)sin(angle), cosfac = (float)cos(angle);
		Point3 p(cosfac * radius, sinfac * radius, 0.0f);
		Point3 rotvec = Point3(sinfac * vector, -cosfac * vector, 0.0f);
		spline->AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_CURVE,p,p + rotvec,p - rotvec));
		}
	spline->SetClosed();
	spline->ComputeBezPoints();
	}

void CircleObject::BuildShape(TimeValue t, BezierShape& ashape) {
	// Start the validity interval at forever and whittle it down.
	ivalid = FOREVER;
	float radius;
	pblock->GetValue(PB_RADIUS, t, radius, ivalid);
	LimitValue( radius, MIN_RADIUS, MAX_RADIUS );
	ashape.NewShape();

	// Get parameters from SimpleSpline and place them in the BezierShape
	int steps;
	BOOL optimize,adaptive;
	ipblock->GetValue(IPB_STEPS, t, steps, ivalid);
	ipblock->GetValue(IPB_OPTIMIZE, t, optimize, ivalid);
	ipblock->GetValue(IPB_ADAPTIVE, t, adaptive, ivalid);
	ashape.steps = adaptive ? -1 : steps;
	ashape.optimize = optimize;

	MakeCircle(ashape,radius);
	ashape.UpdateSels();	// Make sure it readies the selection set info
	ashape.InvalidateGeomCache();
	}

CircleObject::CircleObject() : SimpleSpline() 
	{
	ReadyInterpParameterBlock();		// Build the interpolations parameter block in SimpleSpline
	MakeRefByID(FOREVER, USERPBLOCK, CreateParameterBlock(descVer0, PBLOCK_LENGTH, CURRENT_VERSION));
	assert(pblock);
	
	pblock->SetValue(PB_RADIUS,0,crtRadius);
 	}

CircleObject::~CircleObject()
	{
	DeleteAllRefsFromMe();
	pblock = NULL;
	UnReadyInterpParameterBlock();
	}

class CircleObjCreateCallBack: public CreateMouseCallBack {
	CircleObject *ob;
	Point3 p[2];
	IPoint2 sp0;
	Point3 center;
	int createType;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		void SetObj(CircleObject *obj) { ob = obj; }
	};

int CircleObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
	float r;
#ifdef _3D_CREATE
	DWORD snapdim = SNAP_IN_3D;
#else
	DWORD snapdim = SNAP_IN_PLANE;
#endif

#ifdef _OSNAP
	if (msg == MOUSE_FREEMOVE)
	{
			vpt->SnapPreview(m,m,NULL, snapdim);
	}
#endif
	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {
			case 0:
				ob->suspendSnap = TRUE;
				sp0 = m;
				createType = ob->dlgCreateMeth;
				p[0] = vpt->SnapPoint(m,m,NULL,snapdim);
				mat.SetTrans(p[0]); // Set Node's transform
				ob->pblock->SetValue(PB_RADIUS,0,0.01f);
				ob->pmapParam->Invalidate();
				break;
			case 1: 
				p[1] = vpt->SnapPoint(m,m,NULL,snapdim);
				if ( createType ) {	// radius	
					r = Length(p[1]-p[0]);
					center = p[0];
					}
				else {// diameter
					center = (p[0]+p[1]) / 2.0f;
					r = Length(center-p[0]);
					mat.SetTrans(center);  // Modify Node's transform
					}
				ob->pblock->SetValue(PB_RADIUS,0,r);
				ob->pmapParam->Invalidate();
				if (msg==MOUSE_POINT) {
					ob->suspendSnap = FALSE;
					return (Length(m-sp0)<3 || 
						Length(p[1]-p[0])<0.1f) ? CREATE_ABORT: CREATE_STOP;
					}
				break;
			}
		}
	else
	if (msg == MOUSE_ABORT) {
		return CREATE_ABORT;
		}

	return TRUE;
	}

static CircleObjCreateCallBack circleCreateCB;

CreateMouseCallBack* CircleObject::GetCreateMouseCallBack() {
	circleCreateCB.SetObj(this);
	return(&circleCreateCB);
	}

RefTargetHandle CircleObject::Clone(RemapDir& remap) {
	CircleObject* newob = new CircleObject();
	newob->SimpleSplineClone(this);
	newob->ReplaceReference(USERPBLOCK,pblock->Clone(remap));	
	newob->ivalid.SetEmpty();	
	BaseClone(this, newob, remap);
	return(newob);
	}

BOOL CircleObject::ValidForDisplay(TimeValue t) {
	float radius;
	pblock->GetValue(PB_RADIUS, t, radius, ivalid);
	return (radius == 0.0f) ? FALSE : TRUE;
	}

ParamDimension *CircleObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_RADIUS:
			return stdWorldDim;			
		default:
			return defaultDim;
		}
	}

TSTR CircleObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_RADIUS:
			return TSTR(GetString(IDS_TH_RADIUS));			
		default:
			return TSTR(_T(""));
		}
	}

// From ParamArray
BOOL CircleObject::SetValue(int i, TimeValue t, int v) 
	{
	switch (i) {
		case PB_CREATEMETHOD: dlgCreateMeth = v; break;
		}		
	return TRUE;
	}

BOOL CircleObject::SetValue(int i, TimeValue t, float v)
	{
	switch (i) {				
		case PB_TI_RADIUS: crtRadius = v; break;
		}	
	return TRUE;
	}

BOOL CircleObject::SetValue(int i, TimeValue t, Point3 &v) 
	{
	switch (i) {
		case PB_TI_POS: crtPos = v; break;
		}		
	return TRUE;
	}

BOOL CircleObject::GetValue(int i, TimeValue t, int &v, Interval &ivalid) 
	{
	switch (i) {
		case PB_CREATEMETHOD: v = dlgCreateMeth; break;
		}
	return TRUE;
	}

BOOL CircleObject::GetValue(int i, TimeValue t, float &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_RADIUS: v = crtRadius; break;
		}
	return TRUE;
	}

BOOL CircleObject::GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_POS: v = crtPos; break;		
		}
	return TRUE;
	}

IOResult CircleObject::Load(ILoad *iload)
	{
	iload->RegisterPostLoadCallback(
		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,USERPBLOCK));
	return SimpleSpline::Load(iload);
	}

#endif // NO_OBJECT_SHAPES_SPLINES

