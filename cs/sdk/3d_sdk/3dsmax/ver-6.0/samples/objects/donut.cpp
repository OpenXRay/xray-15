/**********************************************************************
 *<
	FILE: donut.cpp

	DESCRIPTION:  A Donut object implementation

	CREATED BY: Tom Hudson

	HISTORY: created 25 April 1995

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

class DonutObjCreateCallBack;

class DonutObject: public SimpleSpline, public IParamArray {			   

	friend class DonutObjCreateCallBack;
	
	public:
		// Class vars
		static IParamMap *pmapCreate;
		static IParamMap *pmapTypeIn;
		static IParamMap *pmapParam;
		static IObjParam *ip;
		static int dlgCreateMeth;
		static Point3 crtPos;		
		static float crtRadius1, crtRadius2;
		
		void BuildShape(TimeValue t,BezierShape& ashape);

		DonutObject();
		~DonutObject();

		//  inherited virtual methods:

		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		TCHAR *GetObjectName() { return GetString(IDS_TH_DONUT); }
		void InitNodeName(TSTR& s) { s = GetString(IDS_TH_DONUT); }		
		Class_ID ClassID() { return Class_ID(DONUT_CLASS_ID,0); }  
		void GetClassName(TSTR& s) { s = TSTR(GetString(IDS_TH_DONUT_CLASS)); }
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
	};				

//------------------------------------------------------

class DonutObjClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new DonutObject; }
	const TCHAR *	ClassName() { return GetString(IDS_TH_DONUT_CLASS); }
	SClass_ID		SuperClassID() { return SHAPE_CLASS_ID; }
   	Class_ID		ClassID() { return Class_ID(DONUT_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_TH_SPLINES);  }
	void			ResetClassParams(BOOL fileReset);
	};

static DonutObjClassDesc donutObjDesc;

ClassDesc* GetDonutDesc() { return &donutObjDesc; }

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// class variable for donut class.
IParamMap *DonutObject::pmapCreate = NULL;
IParamMap *DonutObject::pmapParam  = NULL;
IParamMap *DonutObject::pmapTypeIn = NULL;
IObjParam *DonutObject::ip         = NULL;
Point3 DonutObject::crtPos         = Point3(0,0,0);
float DonutObject::crtRadius1      = 0.0f;
float DonutObject::crtRadius2      = 0.0f;
int DonutObject::dlgCreateMeth = 1; // create_radius

void DonutObjClassDesc::ResetClassParams(BOOL fileReset)
	{
	DonutObject::crtPos         = Point3(0,0,0);
	DonutObject::crtRadius1     = 0.0f;
	DonutObject::crtRadius2     = 0.0f;
	DonutObject::dlgCreateMeth  = 1; // create_radius
	}

// Parameter map indices
#define PB_RADIUS1		0
#define PB_RADIUS2		1

// Non-parameter block indices
#define PB_CREATEMETHOD		0
#define PB_TI_POS			1
#define PB_TI_RADIUS1		2
#define PB_TI_RADIUS2		3

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
	
	// Radius 1
	ParamUIDesc(
		PB_TI_RADIUS1,
		EDITTYPE_UNIVERSE,
		IDC_RADIUS1,IDC_RAD1SPINNER,
		MIN_RADIUS,MAX_RADIUS,
		SPIN_AUTOSCALE),	

	// Radius 2
	ParamUIDesc(
		PB_TI_RADIUS2,
		EDITTYPE_UNIVERSE,
		IDC_RADIUS2,IDC_RAD2SPINNER,
		MIN_RADIUS,MAX_RADIUS,
		SPIN_AUTOSCALE)
			
	};
#define TYPEINDESC_LENGTH 3

//
//
// Parameters

static ParamUIDesc descParam[] = {
	// Radius 1
	ParamUIDesc(
		PB_RADIUS1,
		EDITTYPE_UNIVERSE,
		IDC_RADIUS1,IDC_RAD1SPINNER,
		MIN_RADIUS,MAX_RADIUS,
		SPIN_AUTOSCALE),	
	
	// Radius 2
	ParamUIDesc(
		PB_RADIUS2,
		EDITTYPE_UNIVERSE,
		IDC_RADIUS2,IDC_RAD2SPINNER,
		MIN_RADIUS,MAX_RADIUS,
		SPIN_AUTOSCALE)	
	
	};
#define PARAMDESC_LENGTH 2


static ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },		
	{ TYPE_FLOAT, NULL, TRUE, 1 } };
#define PBLOCK_LENGTH	2

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,3,0)			// This is a dummy entry!!!
	};
#define NUM_OLDVERSIONS	0	// No old ones yet!

// Current version
#define CURRENT_VERSION	0
static ParamVersionDesc curVersion(descVer0,PBLOCK_LENGTH,CURRENT_VERSION);

//--- TypeInDlgProc --------------------------------

class DonutTypeInDlgProc : public ParamMapUserDlgProc {
	public:
		DonutObject *dob;

		DonutTypeInDlgProc(DonutObject *d) {dob=d;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

BOOL DonutTypeInDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_TI_CREATE: {
					if (dob->crtRadius1==0.0 && dob->crtRadius2==0.0) return TRUE;
					
					// Return focus to the top spinner
					SetFocus(GetDlgItem(hWnd, IDC_TI_POSX));
					
					// We only want to set the value if the object is 
					// not in the scene.
					if (dob->TestAFlag(A_OBJ_CREATING)) {
						dob->pblock->SetValue(PB_RADIUS1,0,dob->crtRadius1);
						dob->pblock->SetValue(PB_RADIUS2,0,dob->crtRadius2);
						}

					Matrix3 tm(1);
					tm.SetTrans(dob->crtPos);
					dob->ip->NonMouseCreate(tm);
					// NOTE that calling NonMouseCreate will cause this
					// object to be deleted. DO NOT DO ANYTHING BUT RETURN.
					return TRUE;	
					}
				}
			break;	
		}
	return FALSE;
	}

void DonutObject::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
	{
	SimpleSpline::BeginEditParams(ip,flags,prev);
	this->ip = ip;

	if (pmapCreate && pmapParam) {
		
		// Left over from last donut ceated
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
				MAKEINTRESOURCE(IDD_DONUTPARAM1),
				GetString(IDS_TH_CREATION_METHOD),
				0);

			pmapTypeIn = CreateCPParamMap(
				descTypeIn,TYPEINDESC_LENGTH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_DONUTPARAM3),
				GetString(IDS_TH_KEYBOARD_ENTRY),
				APPENDROLL_CLOSED);
			}

		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_DONUTPARAM2),
			GetString(IDS_TH_PARAMETERS),
			0);

		
		}

	if(pmapTypeIn) {
		// A callback for the type in.
		pmapTypeIn->SetUserDlgProc(new DonutTypeInDlgProc(this));
		}
	}
		
void DonutObject::EndEditParams( IObjParam *ip,ULONG flags,Animatable *next )
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

void DonutObject::BuildShape(TimeValue t, BezierShape& ashape) {
	// Start the validity interval at forever and whittle it down.
	ivalid = FOREVER;
	float radius1;
	float radius2;
	pblock->GetValue(PB_RADIUS1, t, radius1, ivalid);
	pblock->GetValue(PB_RADIUS2, t, radius2, ivalid);
	LimitValue( radius1, MIN_RADIUS, MAX_RADIUS );
	LimitValue( radius2, MIN_RADIUS, MAX_RADIUS );

	ashape.NewShape();

	// Get parameters from SimpleSpline and place them in the BezierShape
	int steps;
	BOOL optimize,adaptive;
	ipblock->GetValue(IPB_STEPS, t, steps, ivalid);
	ipblock->GetValue(IPB_OPTIMIZE, t, optimize, ivalid);
	ipblock->GetValue(IPB_ADAPTIVE, t, adaptive, ivalid);
	ashape.steps = adaptive ? -1 : steps;
	ashape.optimize = optimize;

	MakeCircle(ashape,radius1);
	MakeCircle(ashape,radius2);
	ashape.UpdateSels();	// Make sure it readies the selection set info
	ashape.InvalidateGeomCache();
	}

DonutObject::DonutObject() : SimpleSpline() 
	{
	ReadyInterpParameterBlock();		// Build the interpolations parameter block in SimpleSpline
	MakeRefByID(FOREVER, USERPBLOCK, CreateParameterBlock(descVer0, PBLOCK_LENGTH, CURRENT_VERSION));
	assert(pblock);
	
	pblock->SetValue(PB_RADIUS1,0,crtRadius1);
	pblock->SetValue(PB_RADIUS2,0,crtRadius2);
 	}

DonutObject::~DonutObject()
	{
	DeleteAllRefsFromMe();
	pblock = NULL;
	UnReadyInterpParameterBlock();
	}

class DonutObjCreateCallBack: public CreateMouseCallBack {
	DonutObject *ob;
	Point3 p[3];
	IPoint2 sp0;
	Point3 center;
	int createType;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		void SetObj(DonutObject *obj) { ob = obj; }
	};

int DonutObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
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
				ob->pblock->SetValue(PB_RADIUS1,0,0.01f);
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
				ob->pblock->SetValue(PB_RADIUS1,0,r);
				ob->pmapParam->Invalidate();
				if (msg==MOUSE_POINT) {
					if(Length(m-sp0)<3 || Length(p[1]-p[0])<0.1f) {
						return CREATE_ABORT;
						}
					}
				break;
			case 2:
				p[2] = vpt->SnapPoint(m,m,NULL,snapdim);
				r = Length(p[2] - center);
				ob->pblock->SetValue(PB_RADIUS2,0,r);
				ob->pmapParam->Invalidate();
				if (msg==MOUSE_POINT) {
					ob->suspendSnap = FALSE;
					return (Length(m-sp0)<3) ? CREATE_ABORT : CREATE_STOP;
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

static DonutObjCreateCallBack donutCreateCB;

CreateMouseCallBack* DonutObject::GetCreateMouseCallBack() {
	donutCreateCB.SetObj(this);
	return(&donutCreateCB);
	}

RefTargetHandle DonutObject::Clone(RemapDir& remap) {
	DonutObject* newob = new DonutObject();
	newob->SimpleSplineClone(this);
	newob->ReplaceReference(USERPBLOCK,pblock->Clone(remap));	
	newob->ivalid.SetEmpty();	
	BaseClone(this, newob, remap);
	return(newob);
	}

BOOL DonutObject::ValidForDisplay(TimeValue t) {
	float radius1, radius2;
	pblock->GetValue(PB_RADIUS1, t, radius1, ivalid);
	pblock->GetValue(PB_RADIUS2, t, radius2, ivalid);
	return (radius1 == 0.0f && radius2 == 0.0f) ? FALSE : TRUE;
	}

ParamDimension *DonutObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_RADIUS1:
		case PB_RADIUS2:
			return stdWorldDim;			
		default:
			return defaultDim;
		}
	}

TSTR DonutObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_RADIUS1:
			return TSTR(GetString(IDS_TH_RADIUS1));
		case PB_RADIUS2:
			return TSTR(GetString(IDS_TH_RADIUS2));
		default:
			return TSTR(_T(""));
		}
	}

// From ParamArray
BOOL DonutObject::SetValue(int i, TimeValue t, int v) 
	{
	switch (i) {
		case PB_CREATEMETHOD: dlgCreateMeth = v; break;
		}		
	return TRUE;
	}

BOOL DonutObject::SetValue(int i, TimeValue t, float v)
	{
	switch (i) {				
		case PB_TI_RADIUS1: crtRadius1 = v; break;
		case PB_TI_RADIUS2: crtRadius2 = v; break;
		}	
	return TRUE;
	}

BOOL DonutObject::SetValue(int i, TimeValue t, Point3 &v) 
	{
	switch (i) {
		case PB_TI_POS: crtPos = v; break;
		}		
	return TRUE;
	}

BOOL DonutObject::GetValue(int i, TimeValue t, int &v, Interval &ivalid) 
	{
	switch (i) {
		case PB_CREATEMETHOD: v = dlgCreateMeth; break;
		}
	return TRUE;
	}

BOOL DonutObject::GetValue(int i, TimeValue t, float &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_RADIUS1: v = crtRadius1; break;
		case PB_TI_RADIUS2: v = crtRadius2; break;
		}
	return TRUE;
	}

BOOL DonutObject::GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_POS: v = crtPos; break;		
		}
	return TRUE;
	}

#endif // NO_OBJECT_SHAPES_SPLINES

