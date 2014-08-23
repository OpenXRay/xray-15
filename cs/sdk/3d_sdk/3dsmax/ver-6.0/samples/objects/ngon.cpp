/**********************************************************************
 *<
	FILE: ngon.cpp

	DESCRIPTION:  An N-sided spline object implementation

	CREATED BY: Tom Hudson

	HISTORY: created 23 February 1995
			 Fillets added 1/12/97 - Audrey Peterson

 *>	Copyright (c) 1995,1996,1997 All Rights Reserved.
 **********************************************************************/

#include "prim.h" 

#ifndef NO_OBJECT_SHAPES_SPLINES

#include "splshape.h"
#include "iparamm.h"
// This is based on the simple spline object...
#include "simpspl.h"

#define MIN_SIDES		3
#define MAX_SIDES		100

#define MIN_RADIUS		float(0)
#define MAX_RADIUS		float( 1.0E30)

#define MIN_CIRCULAR	0
#define MAX_CIRCULAR	1

#define DEF_SIDES		6//24

#define DEF_RADIUS		float(0.0)

#define CIRCUMSCRIBED	0
#define INSCRIBED		1
#define DEF_SCRIBE		INSCRIBED

#define CIRCULAR_ON		1
#define CIRCULAR_OFF	0

#define DEF_CIRCULAR CIRCULAR_OFF
const float HalfPI=1.570796327f;
#define CIRCLE_VECTOR_LENGTH 0.5517861843f


class NGonObjCreateCallBack;

class NGonObject: public SimpleSpline, public IParamArray {			   
		friend class NGonObjCreateCallBack;

	public:
		// Class vars
		static IParamMap *pmapCreate;
		static IParamMap *pmapTypeIn;
		static IParamMap *pmapParam;
		static IObjParam *ip;
		static int dlgSides;
		static int dlgCreateMeth;
		static int dlgCircular;
		static int dlgScribe;
		static Point3 crtPos;		
		static float crtRadius;
		static int crtScribe;
		static float crtFillet;
		
		void BuildShape(TimeValue t,BezierShape& ashape);

		NGonObject();
		~NGonObject();

		//  inherited virtual methods:

	    IOResult NGonObject::Load(ILoad *iload);
		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		TCHAR *GetObjectName() { return GetString(IDS_TH_NGON); }
		void InitNodeName(TSTR& s) { s = GetString(IDS_TH_NGON); }		
		Class_ID ClassID() { return Class_ID(NGON_CLASS_ID,0); }  
		void GetClassName(TSTR& s) { s = TSTR(GetString(IDS_TH_NGON_CLASS)); }
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

class NGonObjClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new NGonObject; }
	const TCHAR *	ClassName() { return GetString(IDS_TH_NGON_CLASS); }
	SClass_ID		SuperClassID() { return SHAPE_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(NGON_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_TH_SPLINES);  }
	void			ResetClassParams(BOOL fileReset);
	};

static NGonObjClassDesc ngonObjDesc;

ClassDesc* GetNGonDesc() { return &ngonObjDesc; }

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// class variable for ngon class.
IParamMap *NGonObject::pmapCreate = NULL;
IParamMap *NGonObject::pmapParam  = NULL;
IParamMap *NGonObject::pmapTypeIn = NULL;
IObjParam *NGonObject::ip         = NULL;
int NGonObject::dlgSides          = DEF_SIDES;
int NGonObject::dlgCreateMeth     = 1; // create_radius
int NGonObject::dlgCircular       = DEF_CIRCULAR;
int NGonObject::dlgScribe         = DEF_SCRIBE;
Point3 NGonObject::crtPos         = Point3(0,0,0);
float NGonObject::crtRadius       = 0.0f;
float NGonObject::crtFillet       = 0.0f;

void NGonObjClassDesc::ResetClassParams(BOOL fileReset)
	{
	NGonObject::dlgSides          = DEF_SIDES;
	NGonObject::dlgCreateMeth     = 1; // create_radius
	NGonObject::dlgCircular       = DEF_CIRCULAR;
	NGonObject::dlgScribe         = DEF_SCRIBE;
	NGonObject::crtPos            = Point3(0,0,0);
	NGonObject::crtRadius         = 0.0f;
	NGonObject::crtFillet         = 0.0f;
	}

// Parameter map indices
#define PB_RADIUS		0
#define PB_SIDES		1
#define PB_CIRCULAR		2
#define PB_FILLET		3
#define PB_SCRIBE		4

// Non-parameter block indices
#define PB_CREATEMETHOD		0
#define PB_TI_POS			1
#define PB_TI_RADIUS		2
#define PB_TI_FILLET		3

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
		SPIN_AUTOSCALE),
		
	// Fillet
	ParamUIDesc(
		PB_TI_FILLET,
		EDITTYPE_UNIVERSE,
		IDC_FILLET,IDC_FILLETSPINNER,
		MIN_RADIUS,MAX_RADIUS,
		SPIN_AUTOSCALE)
	};

#define TYPEINDESC_LENGTH 3

//
//
// Parameters

static int scribeIDs[] = {IDC_CIRCUMSCRIBED,IDC_INSCRIBED};

static ParamUIDesc descParam[] = {
	// Radius
	ParamUIDesc(
		PB_RADIUS,
		EDITTYPE_UNIVERSE,
		IDC_RADIUS,IDC_RADSPINNER,
		MIN_RADIUS,MAX_RADIUS,
		SPIN_AUTOSCALE),	
	
	ParamUIDesc(PB_SCRIBE,TYPE_RADIO,scribeIDs,2),

	// Sides
	ParamUIDesc(
		PB_SIDES,
		EDITTYPE_INT,
		IDC_SIDES,IDC_SIDESPINNER,
		(float)MIN_SIDES,(float)MAX_SIDES,
		(float)1),
	
	// Circular
	ParamUIDesc(PB_CIRCULAR,TYPE_SINGLECHEKBOX,IDC_OBCIRCULAR),

	// Fillet
	ParamUIDesc(
		PB_FILLET,
		EDITTYPE_UNIVERSE,
		IDC_FILLET,IDC_FILLETSPINNER,
		MIN_RADIUS,MAX_RADIUS,
		SPIN_AUTOSCALE)
	};

#define PARAMDESC_LENGTH 5


static ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },		
	{ TYPE_INT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, FALSE, 2 },
 };
static ParamBlockDescID descVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },		
	{ TYPE_INT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, FALSE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 },
 };
static ParamBlockDescID descVer2[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },		
	{ TYPE_INT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, FALSE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 },
	{ TYPE_INT, NULL, FALSE, 4 },
 };
#define PBLOCK_LENGTH	5

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,3,0),			
	ParamVersionDesc(descVer1,4,1),			
	};
#define NUM_OLDVERSIONS	2

// Current version
#define CURRENT_VERSION	2
static ParamVersionDesc curVersion(descVer2,PBLOCK_LENGTH,CURRENT_VERSION);

class NGonPostLoadCallback : public PostLoadCallback {
	public:
		ParamBlockPLCB *cb;
		NGonPostLoadCallback(ParamBlockPLCB *c) {cb=c;}
		void proc(ILoad *iload) {
			DWORD oldVer = ((NGonObject*)(cb->targ))->pblock->GetVersion();
			ReferenceTarget *targ = cb->targ;
			cb->proc(iload);
			if (oldVer==0) {				
				((NGonObject*)targ)->pblock->SetValue(PB_FILLET,0,0.0f);
				((NGonObject*)targ)->pblock->SetValue(PB_SCRIBE,0,INSCRIBED);
				}
			delete this;
			}
	};
IOResult NGonObject::Load(ILoad *iload)
	{  	iload->RegisterPostLoadCallback(
			new NGonPostLoadCallback(
				new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,USERPBLOCK)));
	return SimpleSpline::Load(iload);
	}

//--- TypeInDlgProc --------------------------------

class NGonTypeInDlgProc : public ParamMapUserDlgProc {
	public:
		NGonObject *no;

		NGonTypeInDlgProc(NGonObject *n) {no=n;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

BOOL NGonTypeInDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_TI_CREATE: {
					if (no->crtRadius==0.0) return TRUE;
					
					// Return focus to the top spinner
					SetFocus(GetDlgItem(hWnd, IDC_TI_POSX));
					
					// We only want to set the value if the object is 
					// not in the scene.
					if (no->TestAFlag(A_OBJ_CREATING)) {
						no->pblock->SetValue(PB_RADIUS,0,no->crtRadius);
						no->pblock->SetValue(PB_FILLET,0,no->crtFillet);
						}

					Matrix3 tm(1);
					tm.SetTrans(no->crtPos);
					no->ip->NonMouseCreate(tm);
					// NOTE that calling NonMouseCreate will cause this
					// object to be deleted. DO NOT DO ANYTHING BUT RETURN.
					return TRUE;	
					}
				}
			break;	
		}
	return FALSE;
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


void NGonObject::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev)
	{
	SimpleSpline::BeginEditParams(ip,flags,prev);
	this->ip = ip;

	if (pmapCreate && pmapParam) {
		
		// Left over from last ngon ceated
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
				MAKEINTRESOURCE(IDD_NGONPARAM1),
				GetString(IDS_TH_CREATION_METHOD),
				0);

			pmapTypeIn = CreateCPParamMap(
				descTypeIn,TYPEINDESC_LENGTH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_NGONPARAM3),
				GetString(IDS_TH_KEYBOARD_ENTRY),
				APPENDROLL_CLOSED);
			}

		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_NGONPARAM2),
			GetString(IDS_TH_PARAMETERS),
			0);
		}

	if(pmapTypeIn) {
		// A callback for the type in.
		pmapTypeIn->SetUserDlgProc(new NGonTypeInDlgProc(this));
		}
	}
		
void NGonObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
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
	pblock->GetValue(PB_SIDES,ip->GetTime(),dlgSides,FOREVER);
	pblock->GetValue(PB_CIRCULAR,ip->GetTime(),dlgCircular,FOREVER);	
	pblock->GetValue(PB_SCRIBE,ip->GetTime(),dlgScribe,FOREVER);	
	}

void NGonObject::BuildShape(TimeValue t, BezierShape& ashape) {

	// Start the validity interval at forever and whittle it down.
	ivalid = FOREVER;
	float radius,fillet;
	int sides,circular,scribe;
	pblock->GetValue(PB_RADIUS, t, radius, ivalid);
	pblock->GetValue(PB_SIDES, t, sides, ivalid);
	pblock->GetValue(PB_CIRCULAR, t, circular, ivalid);
	pblock->GetValue(PB_FILLET, t, fillet, ivalid);
	pblock->GetValue(PB_SCRIBE, t, scribe, ivalid);

	LimitValue( radius, MIN_RADIUS, MAX_RADIUS );
	LimitValue( sides, MIN_SIDES, MAX_SIDES );
	LimitValue( circular, MIN_CIRCULAR, MAX_CIRCULAR );
	LimitValue( fillet, MIN_RADIUS, MAX_RADIUS );
	LimitValue( scribe, CIRCUMSCRIBED, INSCRIBED );
	float vector;

	// If circumscribed, modify the radius
	if(scribe == CIRCUMSCRIBED)
		radius = radius / (float)cos(TWOPI / ((float)sides * 2.0f));

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

	Spline3D *spline = ashape.NewSpline();

	// Determine the vector length if circular
	if(circular)
		vector = veccalc(6.2831853f / (float)sides) * radius;
	else
		vector = 0.0f;

	// Now add all the necessary points
	if ((fillet==0)||(circular))
	{ for(int ix=0; ix<sides; ++ix) 
	  {	float angle = 6.2831853f * (float)ix / (float)sides;
		float sinfac = (float)sin(angle), cosfac = (float)cos(angle);
		Point3 p(cosfac * radius, sinfac * radius, 0.0f);
		Point3 rotvec = Point3(sinfac * vector, -cosfac * vector, 0.0f);
		spline->AddKnot(SplineKnot(circular ? KTYPE_BEZIER : KTYPE_CORNER,LTYPE_CURVE,p,p + rotvec,p - rotvec));
	  }
	  spline->SetClosed();
	  spline->ComputeBezPoints();
	// If it's a linear shape, turn the knots back into beziers -- We used the spline package to compute vectors
	  if(!circular) 
	  {	for(int i = 0; i < sides; ++i)
			spline->SetKnotType(i, KTYPE_BEZIER_CORNER);
	  }
	}
	else
	{ for(int ix=0; ix<sides; ++ix) 
	  {	float angle = 6.2831853f * (float)ix / (float)sides;
	    float theta2= (PI*(sides-2)/sides)/2.0f,fang=angle+theta2,fang2=angle-theta2;
		float f=fillet*(float)tan(HalfPI-theta2);
		float sinfac = (float)sin(angle), cosfac = (float)cos(angle);
		Point3 p(cosfac * radius, sinfac * radius, 0.0f),p1,p2;
		Point3 fvec1=Point3((float)-cos(fang),(float)-sin(fang),0.0f)*f;
		Point3 fvec2=Point3((float)-cos(fang2),(float)-sin(fang2),0.0f)*f;
		p1=p+fvec1;p2=p+fvec2;
		Point3 p1vec=fvec1*CIRCLE_VECTOR_LENGTH,p2vec=fvec2*CIRCLE_VECTOR_LENGTH;
		spline->AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_CURVE,p1,p1 + p1vec,p1 - p1vec));
		spline->AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_CURVE,p2,p2 - p2vec,p2 + p2vec));
	  }
	  spline->SetClosed();
	  spline->ComputeBezPoints();
	}
	ashape.UpdateSels();	// Make sure it readies the selection set info
	ashape.InvalidateGeomCache();
	}

NGonObject::NGonObject() : SimpleSpline() 
	{
	ReadyInterpParameterBlock();		// Build the interpolations parameter block in SimpleSpline
	MakeRefByID(FOREVER, USERPBLOCK, CreateParameterBlock(descVer2, PBLOCK_LENGTH, CURRENT_VERSION));
	assert(pblock);
	
	pblock->SetValue(PB_RADIUS,0,crtRadius);
	pblock->SetValue(PB_FILLET,0,crtFillet);
	pblock->SetValue(PB_SIDES,0,dlgSides);	
	pblock->SetValue(PB_CIRCULAR,0,dlgCircular);
	pblock->SetValue(PB_SCRIBE,0,dlgScribe);
 	}

NGonObject::~NGonObject()
	{
	DeleteAllRefsFromMe();
	pblock = NULL;
	UnReadyInterpParameterBlock();
	}

class NGonObjCreateCallBack: public CreateMouseCallBack {
	NGonObject *ob;
	Point3 p[2];
	IPoint2 sp0,sp1;
	int createType;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		void SetObj(NGonObject *obj) { ob = obj; }
	};

int NGonObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
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
				sp1 = m;							   
				p[1] = vpt->SnapPoint(m,m,NULL,snapdim);
				if ( createType ) {	// radius	
					r = Length(p[1]-p[0]);
					}
				else {// diameter
					Point3 center = (p[0]+p[1])/float(2);
					r = Length(center-p[0]);
					mat.SetTrans(center);  // Modify Node's transform
					}
				ob->pblock->SetValue(PB_RADIUS,0,r);
				ob->pmapParam->Invalidate();
				if (msg==MOUSE_POINT)
				{ ob->suspendSnap = FALSE;
				  if (Length(m-sp0)<3 || Length(p[1]-p[0])<0.1f)
				   return CREATE_ABORT;
				  else	return CREATE_STOP;
				}
				break;
/*			case 2:
				{ float f = vpt->SnapLength(vpt->GetCPDisp(p[1],Point3(0,0,1),sp1,m));
				  ob->pblock->SetValue(PB_FILLET,0,f);
				  ob->pmapParam->Invalidate();
				  if (msg==MOUSE_POINT) 
				  {	ob->suspendSnap = FALSE;
					return CREATE_STOP;
				  }
				}
				break;*/
			}
		}
	else
	if (msg == MOUSE_ABORT) {
		return CREATE_ABORT;
		}

	return TRUE;
	}

static NGonObjCreateCallBack ngonCreateCB;

CreateMouseCallBack* NGonObject::GetCreateMouseCallBack() {
	ngonCreateCB.SetObj(this);
	return(&ngonCreateCB);
	}

//
// Reference Managment:
//

RefTargetHandle NGonObject::Clone(RemapDir& remap) {
	NGonObject* newob = new NGonObject();
	newob->SimpleSplineClone(this);
	newob->ReplaceReference(USERPBLOCK,pblock->Clone(remap));	
	newob->ivalid.SetEmpty();	
	BaseClone(this, newob, remap);
	return(newob);
	}

BOOL NGonObject::ValidForDisplay(TimeValue t) {
	float radius;
	pblock->GetValue(PB_RADIUS, t, radius, ivalid);
	return (radius == 0.0f) ? FALSE : TRUE;
	}

ParamDimension *NGonObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_RADIUS:
			return stdWorldDim;			
		case PB_FILLET:
			return stdWorldDim;			
		case PB_SIDES:
			return stdSegmentsDim;			
		case PB_CIRCULAR:
		case PB_SCRIBE:
			return stdNormalizedDim;			
		default:
			return defaultDim;
		}
	}

TSTR NGonObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_RADIUS:
			return TSTR(GetString(IDS_TH_RADIUS));			
		case PB_FILLET:
			return TSTR(GetString(IDS_AP_FILLET));			
		case PB_SIDES:
			return TSTR(GetString(IDS_TH_SIDES));			
		case PB_CIRCULAR:
			return TSTR(GetString(IDS_TH_CIRCULAR));			
		case PB_SCRIBE:
			return TSTR(GetString(IDS_TH_SCRIBE));			
		default:
			return TSTR(_T(""));
		}
	}

// From ParamArray
BOOL NGonObject::SetValue(int i, TimeValue t, int v) 
	{
	switch (i) {
		case PB_CREATEMETHOD: dlgCreateMeth = v; break;
		}		
	return TRUE;
	}

BOOL NGonObject::SetValue(int i, TimeValue t, float v)
	{
	switch (i) {				
		case PB_TI_RADIUS: crtRadius = v; break;
		case PB_TI_FILLET: crtFillet = v; break;
		}	
	return TRUE;
	}

BOOL NGonObject::SetValue(int i, TimeValue t, Point3 &v) 
	{
	switch (i) {
		case PB_TI_POS: crtPos = v; break;
		}		
	return TRUE;
	}

BOOL NGonObject::GetValue(int i, TimeValue t, int &v, Interval &ivalid) 
	{
	switch (i) {
		case PB_CREATEMETHOD: v = dlgCreateMeth; break;
		}
	return TRUE;
	}

BOOL NGonObject::GetValue(int i, TimeValue t, float &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_RADIUS: v = crtRadius; break;
		case PB_TI_FILLET: v = crtFillet; break;
		}
	return TRUE;
	}

BOOL NGonObject::GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_POS: v = crtPos; break;		
		}
	return TRUE;
	}

#endif // NO_OBJECT_SHAPES_SPLINES

