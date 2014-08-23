/**********************************************************************
 *<
	FILE: rectangl.cpp

	DESCRIPTION:  An rectangular spline object implementation

	CREATED BY: Tom Hudson

	HISTORY: created 23 February 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#include "prim.h" 

#ifndef NO_OBJECT_SHAPES_SPLINES

#include "splshape.h"
#include "iparamm.h"
// This is based on the simple spline object...
#include "simpspl.h"


#define MIN_LENGTH		float(0)
#define MAX_LENGTH		float( 1.0E30)
#define MIN_WIDTH		float(0)
#define MAX_WIDTH		float( 1.0E30)

#define DEF_LENGTH		float(0.0)
#define DEF_WIDTH		float(0.0)

#define CREATE_EDGE 0
#define CREATE_CENTER 1

class EllipseObjCreateCallBack;

class EllipseObject: public SimpleSpline, public IParamArray {			   
		friend class EllipseObjCreateCallBack;

	public:	
		// Class vars
		static IParamMap *pmapCreate;
		static IParamMap *pmapTypeIn;
		static IParamMap *pmapParam;
		static IObjParam *ip;
		static int dlgCreateMeth;
		static Point3 crtPos;		
		static float crtLength;
		static float crtWidth;
		
		void BuildShape(TimeValue t,BezierShape& ashape);

		EllipseObject();
		~EllipseObject();

		//  inherited virtual methods:

		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		TCHAR *GetObjectName() { return GetString(IDS_TH_ELLIPSE); }
		void InitNodeName(TSTR& s) { s = GetString(IDS_TH_ELLIPSE); }		
		Class_ID ClassID() { return Class_ID(ELLIPSE_CLASS_ID,0); }  
		void GetClassName(TSTR& s) { s = TSTR(GetString(IDS_TH_ELLIPSE_CLASS)); }
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

class EllipseObjClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new EllipseObject; }
	const TCHAR *	ClassName() { return GetString(IDS_TH_ELLIPSE_CLASS); }
	SClass_ID		SuperClassID() { return SHAPE_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(ELLIPSE_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_TH_SPLINES);  }
	void			ResetClassParams(BOOL fileReset);
	};

static EllipseObjClassDesc ellipseObjDesc;

ClassDesc* GetEllipseDesc() { return &ellipseObjDesc; }

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// class variable for sphere class.
IParamMap *EllipseObject::pmapCreate = NULL;
IParamMap *EllipseObject::pmapParam  = NULL;
IParamMap *EllipseObject::pmapTypeIn = NULL;
IObjParam *EllipseObject::ip         = NULL;
int EllipseObject::dlgCreateMeth     = CREATE_EDGE;
Point3 EllipseObject::crtPos         = Point3(0,0,0);
float EllipseObject::crtLength       = 0.0f;
float EllipseObject::crtWidth        = 0.0f;

void EllipseObjClassDesc::ResetClassParams(BOOL fileReset)
	{
	EllipseObject::dlgCreateMeth = CREATE_EDGE;
	EllipseObject::crtPos        = Point3(0,0,0);
	EllipseObject::crtLength     = 0.0f;
	EllipseObject::crtWidth      = 0.0f;
	}

// Parameter map indices
#define PB_LENGTH		0
#define PB_WIDTH		1

// Non-parameter block indices
#define PB_CREATEMETHOD		0
#define PB_TI_POS			1
#define PB_TI_LENGTH		2
#define PB_TI_WIDTH			3

//
//
//	Creation method

static int createMethIDs[] = {IDC_CREATEEDGE,IDC_CREATECENTER};

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
	
	// Length
	ParamUIDesc(
		PB_TI_LENGTH,
		EDITTYPE_UNIVERSE,
		IDC_LENGTHEDIT,IDC_LENSPINNER,
		MIN_LENGTH,MAX_LENGTH,
		SPIN_AUTOSCALE),	

	// Width
	ParamUIDesc(
		PB_TI_WIDTH,
		EDITTYPE_UNIVERSE,
		IDC_WIDTHEDIT,IDC_WIDTHSPINNER,
		MIN_WIDTH,MAX_WIDTH,
		SPIN_AUTOSCALE)
			
	};
#define TYPEINDESC_LENGTH 3

//
//
// Parameters

static ParamUIDesc descParam[] = {
	// Length
	ParamUIDesc(
		PB_LENGTH,
		EDITTYPE_UNIVERSE,
		IDC_LENGTHEDIT,IDC_LENSPINNER,
		MIN_LENGTH,MAX_LENGTH,
		SPIN_AUTOSCALE),	
	
	// Width
	ParamUIDesc(
		PB_WIDTH,
		EDITTYPE_UNIVERSE,
		IDC_WIDTHEDIT,IDC_WIDTHSPINNER,
		MIN_WIDTH,MAX_WIDTH,
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

class EllipseTypeInDlgProc : public ParamMapUserDlgProc {
	public:
		EllipseObject *ro;

		EllipseTypeInDlgProc(EllipseObject *d) {ro=d;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

BOOL EllipseTypeInDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_TI_CREATE: {
					if (ro->crtLength==0.0 && ro->crtWidth==0.0) return TRUE;
					
					// Return focus to the top spinner
					SetFocus(GetDlgItem(hWnd, IDC_TI_POSX));
					
					// We only want to set the value if the object is 
					// not in the scene.
					if (ro->TestAFlag(A_OBJ_CREATING)) {
						ro->pblock->SetValue(PB_LENGTH,0,ro->crtLength);
						ro->pblock->SetValue(PB_WIDTH,0,ro->crtWidth);
						}

					Matrix3 tm(1);
					tm.SetTrans(ro->crtPos);
					ro->ip->NonMouseCreate(tm);
					// NOTE that calling NonMouseCreate will cause this
					// object to be deleted. DO NOT DO ANYTHING BUT RETURN.
					return TRUE;	
					}
				}
			break;	
		}
	return FALSE;
	}

void EllipseObject::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev)
	{
	SimpleSpline::BeginEditParams(ip,flags,prev);
	this->ip = ip;

	if (pmapCreate && pmapParam) {
		
		// Left over from last shape ceated
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
				MAKEINTRESOURCE(IDD_ELLIPSEPARAM1),
				GetString(IDS_TH_CREATION_METHOD),
				0);

			pmapTypeIn = CreateCPParamMap(
				descTypeIn,TYPEINDESC_LENGTH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_ELLIPSEPARAM3),
				GetString(IDS_TH_KEYBOARD_ENTRY),
				APPENDROLL_CLOSED);
			}

		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_ELLIPSEPARAM2),
			GetString(IDS_TH_PARAMETERS),
			0);
		}

	if(pmapTypeIn) {
		// A callback for the type in.
		pmapTypeIn->SetUserDlgProc(new EllipseTypeInDlgProc(this));
		}
	}
		
void EllipseObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
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

// Vector length for unit circle
#define CIRCLE_VECTOR_LENGTH 0.5517861843f

static void MakeCircle(BezierShape& ashape, float radius, float xmult, float ymult) {
	float vector = CIRCLE_VECTOR_LENGTH * radius;
	// Delete all points in the existing spline
	Spline3D *spline = ashape.NewSpline();
	// Now add all the necessary points
	Point3 mult = Point3(xmult, ymult, 1.0f);
	for(int ix=0; ix<4; ++ix) {
		float angle = 6.2831853f * (float)ix / 4.0f;
		float sinfac = (float)sin(angle), cosfac = (float)cos(angle);
		Point3 p(cosfac * radius, sinfac * radius, 0.0f);
		Point3 rotvec = Point3(sinfac * vector, -cosfac * vector, 0.0f);
		spline->AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_CURVE,p * mult,(p + rotvec) * mult,(p - rotvec) * mult));
		}
	spline->SetClosed();
	spline->ComputeBezPoints();
	}

void EllipseObject::BuildShape(TimeValue t, BezierShape& ashape) {

	// Start the validity interval at forever and whittle it down.
	ivalid = FOREVER;
	float length;
	float width;
	pblock->GetValue(PB_LENGTH, t, length, ivalid);
	pblock->GetValue(PB_WIDTH, t, width, ivalid);
	LimitValue( length, MIN_LENGTH, MAX_LENGTH );
	LimitValue( width, MIN_WIDTH, MAX_WIDTH );

	// Delete the existing shape and create a new spline in it
	ashape.NewShape();

	// Get parameters from SimpleSpline and place them in the BezierShape
	int steps;
	BOOL optimize,adaptive;
	ipblock->GetValue(IPB_STEPS, t, steps, ivalid);
	ipblock->GetValue(IPB_OPTIMIZE, t, optimize, ivalid);
	ipblock->GetValue(IPB_ADAPTIVE, t, adaptive, ivalid);
	ashape.steps = adaptive ? -1 : steps;
	ashape.optimize = optimize;

	float radius, xmult, ymult;
	if(length < width) {
		radius = width;
		xmult = 1.0f;
		ymult = length / width;
		}
	else
	if(width < length) {
		radius = length;
		xmult = width / length;
		ymult = 1.0f;
		}
	else {
		radius = length;
		xmult = ymult = 1.0f;
		}
	MakeCircle(ashape, radius / 2.0f, xmult, ymult);
	ashape.UpdateSels();	// Make sure it readies the selection set info
	ashape.InvalidateGeomCache();
	}

EllipseObject::EllipseObject() : SimpleSpline() 
	{
	ReadyInterpParameterBlock();		// Build the interpolations parameter block in SimpleSpline
	MakeRefByID(FOREVER, USERPBLOCK, CreateParameterBlock(descVer0, PBLOCK_LENGTH, CURRENT_VERSION));
	assert(pblock);
	
	pblock->SetValue(PB_LENGTH,0,crtLength);
	pblock->SetValue(PB_WIDTH,0,crtWidth);	
 	}

EllipseObject::~EllipseObject()
	{
	DeleteAllRefsFromMe();
	pblock = NULL;
	UnReadyInterpParameterBlock();
	}

class EllipseObjCreateCallBack: public CreateMouseCallBack {
	EllipseObject *ob;
	Point3 p0,p1;
	IPoint2 sp0;
	int createType;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		void SetObj(EllipseObject *obj) { ob = obj; }
	};

int EllipseObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
	Point3 d;
#ifdef _3D_CREATE
	DWORD snapdim = SNAP_IN_3D;
#else
	DWORD snapdim = SNAP_IN_PLANE;
#endif

	if (msg == MOUSE_FREEMOVE)
	{
		#ifdef _OSNAP
			vpt->SnapPreview(m,m,NULL, snapdim);
		#endif
	}
	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {
			case 0:
				sp0 = m;
				ob->suspendSnap = TRUE;
				createType = ob->dlgCreateMeth;
				p0 = vpt->SnapPoint(m,m,NULL,snapdim);
				p1 = p0 + Point3(.01,.01,.0);
				if(createType == CREATE_EDGE)
					mat.SetTrans(float(.5)*(p0+p1));
				else
					mat.SetTrans(p0);
				break;
			case 1: { 
				p1 = vpt->SnapPoint(m,m,NULL,snapdim);
				p1.z = p0.z; 
				d = p1-p0;
				float w = float(fabs(d.x));
				float l = float(fabs(d.y));
				if(flags & MOUSE_CTRL) {
					if(createType == CREATE_EDGE) {
						float ysign = (d.y < 0.0f) ? -1.0f : 1.0f;
						mat.SetTrans(float(.5)*(p0+Point3(p1.x,p0.y+ysign*w,0.0f)));
						}
					else {
						mat.SetTrans(p0);
						w = w * 2.0f;
						}
					ob->pblock->SetValue(PB_LENGTH,0,w);
					ob->pblock->SetValue(PB_WIDTH,0,w);
					}
				else {
					if(createType == CREATE_EDGE)
						mat.SetTrans(float(.5)*(p0+p1));
					else {
						mat.SetTrans(p0);
						w = w * 2.0f;
						l = l * 2.0f;
						}
					ob->pblock->SetValue(PB_WIDTH,0,w);
					ob->pblock->SetValue(PB_LENGTH,0,l);
					}
				ob->pmapParam->Invalidate();
				if (msg==MOUSE_POINT) {
					ob->suspendSnap = FALSE;
					return (Length(m-sp0)<3 || Length(p1-p0)<0.1f) ? CREATE_ABORT: CREATE_STOP;
					}
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

static EllipseObjCreateCallBack ellipseCreateCB;

CreateMouseCallBack* EllipseObject::GetCreateMouseCallBack() {
	ellipseCreateCB.SetObj(this);
	return(&ellipseCreateCB);
	}

//
// Reference Managment:
//

RefTargetHandle EllipseObject::Clone(RemapDir& remap) {
	EllipseObject* newob = new EllipseObject();
	newob->SimpleSplineClone(this);
	newob->ReplaceReference(USERPBLOCK,pblock->Clone(remap));	
	newob->ivalid.SetEmpty();	
	BaseClone(this, newob, remap);
	return(newob);
	}

BOOL EllipseObject::ValidForDisplay(TimeValue t) {
	float length, width;
	pblock->GetValue(PB_LENGTH, t, length, ivalid);
	pblock->GetValue(PB_WIDTH, t, width, ivalid);
	return (length == 0 || width == 0) ? FALSE : TRUE;
	}

ParamDimension *EllipseObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_LENGTH:
		case PB_WIDTH:
			return stdWorldDim;			
		default:
			return defaultDim;
		}
	}

TSTR EllipseObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_LENGTH:
			return TSTR(GetString(IDS_RB_LENGTH));			
		case PB_WIDTH:
			return TSTR(GetString(IDS_RB_WIDTH));
		default:
			return TSTR(_T(""));
		}
	}

// From ParamArray
BOOL EllipseObject::SetValue(int i, TimeValue t, int v) 
	{
	switch (i) {
		case PB_CREATEMETHOD: dlgCreateMeth = v; break;
		}		
	return TRUE;
	}

BOOL EllipseObject::SetValue(int i, TimeValue t, float v)
	{
	switch (i) {				
		case PB_TI_LENGTH: crtLength = v; break;
		case PB_TI_WIDTH: crtWidth = v; break;
		}	
	return TRUE;
	}

BOOL EllipseObject::SetValue(int i, TimeValue t, Point3 &v) 
	{
	switch (i) {
		case PB_TI_POS: crtPos = v; break;
		}		
	return TRUE;
	}

BOOL EllipseObject::GetValue(int i, TimeValue t, int &v, Interval &ivalid) 
	{
	switch (i) {
		case PB_CREATEMETHOD: v = dlgCreateMeth; break;
		}
	return TRUE;
	}

BOOL EllipseObject::GetValue(int i, TimeValue t, float &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_LENGTH: v = crtLength; break;
		case PB_TI_WIDTH: v = crtWidth; break;
		}
	return TRUE;
	}

BOOL EllipseObject::GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_POS: v = crtPos; break;		
		}
	return TRUE;
	}

#endif // NO_OBJECT_SHAPES_SPLINES

