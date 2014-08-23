/**********************************************************************
 *<
	FILE: star.cpp

	DESCRIPTION:  A Spline based star object

	CREATED BY: Christer Janson
	            Autodesk European Developer Support

	HISTORY: Created 22 October 1995
             Based on the file ngon.cpp by Tom Hudson
			 Fillets added 1/12/97 - Audrey Peterson

 *>	Copyright (c) 1995,1996,1997 All Rights Reserved.
 **********************************************************************/

#include "prim.h" 

#ifndef NO_OBJECT_SHAPES_SPLINES

#include "splshape.h"
#include "iparamm.h"
// This is based on the simple spline object...
#include "simpspl.h"

#define MIN_POINTS		3
#define MAX_POINTS		100

#define MIN_RADIUS		float(0)
#define MAX_RADIUS		float( 1.0E30)

#define MIN_DIST		float(-180)
#define MAX_DIST		float(180)

#define DEF_POINTS		6

#define DEF_RADIUS1		float(0.0)
#define DEF_RADIUS2		float(0.0)
#define DEF_DIST		float(0.0)
const float PI180=0.0174532f;
#define CIRCLE_VECTOR_LENGTH 0.5517861843f

class StarObjCreateCallBack;

class StarObject: public SimpleSpline, public IParamArray {			   
		friend class StarObjCreateCallBack;

	public:
		// Class vars
		static IParamMap *pmapTypeIn;
		static IParamMap *pmapParam;
		static IObjParam *ip;
		static int dlgPoints;
		static float dlgDistortion;
		static Point3 crtPos;		
		static float crtRadius1;
		static float crtRadius2;
		static float crtFillet1;
		static float crtFillet2;

		void BuildShape(TimeValue t,BezierShape& ashape);

		StarObject();
		~StarObject();

		//  inherited virtual methods:

		IOResult StarObject::Load(ILoad *iload);
		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		TCHAR *GetObjectName() { return GetString(IDS_TH_STAR); }
		void InitNodeName(TSTR& s) { s = GetString(IDS_TH_STAR); }		
		Class_ID ClassID() { return Class_ID(STAR_CLASS_ID,0); }  
		void GetClassName(TSTR& s) { s = TSTR(GetString(IDS_TH_STAR_CLASS)); }
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

class StarObjClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new StarObject; }
	const TCHAR *	ClassName() { return GetString(IDS_TH_STAR_CLASS); }
	SClass_ID		SuperClassID() { return SHAPE_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(STAR_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_TH_SPLINES);  }
	void			ResetClassParams(BOOL fileReset);
	};

static StarObjClassDesc starObjDesc;

ClassDesc* GetStarDesc() { return &starObjDesc; }

// in ccj.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// class variable for star class.
IParamMap *StarObject::pmapParam  = NULL;
IParamMap *StarObject::pmapTypeIn = NULL;
IObjParam *StarObject::ip         = NULL;
int StarObject::dlgPoints         = DEF_POINTS;
float StarObject::dlgDistortion   = DEF_DIST;
Point3 StarObject::crtPos         = Point3(0,0,0);
float StarObject::crtRadius1      = 0.0f;
float StarObject::crtRadius2      = 0.0f;
float StarObject::crtFillet1      = 0.0f;
float StarObject::crtFillet2      = 0.0f;

void StarObjClassDesc::ResetClassParams(BOOL fileReset)
	{
	StarObject::dlgPoints       = DEF_POINTS;
	StarObject::dlgDistortion   = DEF_DIST;
	StarObject::crtPos          = Point3(0,0,0);
	StarObject::crtRadius1      = 0.0f;
	StarObject::crtRadius2      = 0.0f;
	StarObject::crtFillet1      = 0.0f;
	StarObject::crtFillet2      = 0.0f;
	}

// Parameter map indices
#define PB_RADIUS1		0
#define PB_RADIUS2		1
#define PB_POINTS		2
#define PB_DISTORT		3
#define PB_FILLET1		4
#define PB_FILLET2		5

// Non-parameter block indices
#define PB_TI_POS			0
#define PB_TI_RADIUS1		1
#define PB_TI_RADIUS2		2
#define PB_TI_FILLET1		3
#define PB_TI_FILLET2		4

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
		SPIN_AUTOSCALE),

	// Fillet1
	ParamUIDesc(
		PB_TI_FILLET1,
		EDITTYPE_UNIVERSE,
		IDC_AP_FILLET,IDC_AP_FILLETSPINNER,
		MIN_RADIUS,MAX_RADIUS,
		SPIN_AUTOSCALE),

	// Fillet2
	ParamUIDesc(
		PB_TI_FILLET2,
		EDITTYPE_UNIVERSE,
		IDC_AP_FILLET2,IDC_AP_FILLETSPINNER2,
		MIN_RADIUS,MAX_RADIUS,
		SPIN_AUTOSCALE),
	};
#define TYPEINDESC_LENGTH 5

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
		SPIN_AUTOSCALE),
	
	// Points
	ParamUIDesc(
		PB_POINTS,
		EDITTYPE_INT,
		IDC_SIDES,IDC_SIDESPINNER,
		(float)MIN_POINTS,(float)MAX_POINTS,
		(float)1),
	
	// Distortion
	ParamUIDesc(
		PB_DISTORT,
		EDITTYPE_FLOAT,
		IDC_DISTORT,IDC_DISTORTSPINNER,
		MIN_DIST,MAX_DIST,
		SPIN_AUTOSCALE),

	// Fillet1
	ParamUIDesc(
		PB_FILLET1,
		EDITTYPE_UNIVERSE,
		IDC_AP_FILLET,IDC_AP_FILLETSPINNER,
		MIN_RADIUS,MAX_RADIUS,
		SPIN_AUTOSCALE),

	// Fillet2
	ParamUIDesc(
		PB_FILLET2,
		EDITTYPE_UNIVERSE,
		IDC_AP_FILLET2,IDC_AP_FILLETSPINNER2,
		MIN_RADIUS,MAX_RADIUS,
		SPIN_AUTOSCALE),
	};
#define PARAMDESC_LENGTH 6


static ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },		
	{ TYPE_FLOAT, NULL, TRUE, 1 },		
	{ TYPE_INT, NULL, TRUE, 2 },		
	{ TYPE_FLOAT, NULL, TRUE, 3 },
 };		
static ParamBlockDescID descVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },		
	{ TYPE_FLOAT, NULL, TRUE, 1 },		
	{ TYPE_INT, NULL, TRUE, 2 },		
	{ TYPE_FLOAT, NULL, TRUE, 3 },
	{ TYPE_FLOAT, NULL, TRUE, 4 },	  //Fillet1
	{ TYPE_FLOAT, NULL, TRUE, 5 },    //Fillet2
 };		
#define PBLOCK_LENGTH	6

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,4,0)			
	};
#define NUM_OLDVERSIONS	1

// Current version
#define CURRENT_VERSION	1
static ParamVersionDesc curVersion(descVer1,PBLOCK_LENGTH,CURRENT_VERSION);

//--- TypeInDlgProc --------------------------------

class StarPostLoadCallback : public PostLoadCallback {
	public:
		ParamBlockPLCB *cb;
		StarPostLoadCallback(ParamBlockPLCB *c) {cb=c;}
		void proc(ILoad *iload) {
			DWORD oldVer = ((StarObject*)(cb->targ))->pblock->GetVersion();
			ReferenceTarget *targ = cb->targ;
			cb->proc(iload);
			if (oldVer==0) {				
				((StarObject*)targ)->pblock->SetValue(PB_FILLET1,0,0.0f);
				((StarObject*)targ)->pblock->SetValue(PB_FILLET2,0,0.0f);
				}
			delete this;
			}
	};
IOResult StarObject::Load(ILoad *iload)
	{  	iload->RegisterPostLoadCallback(
			new StarPostLoadCallback(
				new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,USERPBLOCK)));
	return SimpleSpline::Load(iload);
	}


class StarTypeInDlgProc : public ParamMapUserDlgProc {
	public:
		StarObject *so;

		StarTypeInDlgProc(StarObject *s) {so=s;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

BOOL StarTypeInDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_TI_CREATE: {
					if (so->crtRadius1==0.0 && so->crtRadius2==0.0) return TRUE;
					
					// Return focus to the top spinner
					SetFocus(GetDlgItem(hWnd, IDC_TI_POSX));
					
					// We only want to set the value if the object is 
					// not in the scene.
					if (so->TestAFlag(A_OBJ_CREATING)) {
						so->pblock->SetValue(PB_RADIUS1,0,so->crtRadius1);
						so->pblock->SetValue(PB_RADIUS2,0,so->crtRadius2);
						so->pblock->SetValue(PB_FILLET1,0,so->crtFillet1);
						so->pblock->SetValue(PB_FILLET2,0,so->crtFillet2);
						}

					Matrix3 tm(1);
					tm.SetTrans(so->crtPos);
					so->ip->NonMouseCreate(tm);
					// NOTE that calling NonMouseCreate will cause this
					// object to be deleted. DO NOT DO ANYTHING BUT RETURN.
					return TRUE;	
					}
				}
			break;	
		}
	return FALSE;
	}

void StarObject::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev)
	{
	SimpleSpline::BeginEditParams(ip,flags,prev);
	this->ip = ip;

	if (pmapParam) {
		
		// Left over from last star ceated
		pmapTypeIn->SetParamBlock(this);
		pmapParam->SetParamBlock(pblock);
	} else {
		
		// Gotta make a new one.
		if (flags&BEGIN_EDIT_CREATE) {
			pmapTypeIn = CreateCPParamMap(
				descTypeIn,TYPEINDESC_LENGTH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_STARPARAM2),
				GetString(IDS_TH_KEYBOARD_ENTRY),
				APPENDROLL_CLOSED);
			}

		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_STARPARAM1),
			GetString(IDS_TH_PARAMETERS),
			0);
		}

	if(pmapTypeIn) {
		// A callback for the type in.
		pmapTypeIn->SetUserDlgProc(new StarTypeInDlgProc(this));
		}
	}
		
void StarObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
	{
	SimpleSpline::EndEditParams(ip,flags,next);
	this->ip = NULL;

	if (flags&END_EDIT_REMOVEUI ) {
		if (pmapTypeIn) DestroyCPParamMap(pmapTypeIn);
		DestroyCPParamMap(pmapParam);
		pmapParam  = NULL;
		pmapTypeIn = NULL;
		}

	// Save these values in class variables so the next object created will inherit them.
	pblock->GetValue(PB_POINTS,ip->GetTime(),dlgPoints,FOREVER);
	pblock->GetValue(PB_DISTORT,ip->GetTime(),dlgDistortion,FOREVER);	
	}

void StarObject::BuildShape(TimeValue t, BezierShape& ashape) {

	// Start the validity interval at forever and whittle it down.
	ivalid = FOREVER;
	float radius1, radius2, distortion,fillet1,fillet2;
	int points;
	pblock->GetValue(PB_RADIUS1, t, radius1, ivalid);
	pblock->GetValue(PB_RADIUS2, t, radius2, ivalid);
	pblock->GetValue(PB_POINTS, t, points, ivalid);
	pblock->GetValue(PB_DISTORT, t, distortion, ivalid);
	pblock->GetValue(PB_FILLET1, t, fillet1, ivalid);
	pblock->GetValue(PB_FILLET2, t, fillet2, ivalid);

	Point3 p;								// The actual point
	float angle;							// Angle of the current point

	LimitValue( radius1, MIN_RADIUS, MAX_RADIUS );
	LimitValue( radius2, MIN_RADIUS, MAX_RADIUS );
	LimitValue( distortion, MIN_DIST, MAX_DIST );
	LimitValue( points, MIN_POINTS, MAX_POINTS );
	LimitValue( fillet1, MIN_RADIUS, MAX_RADIUS );
	LimitValue( fillet2, MIN_RADIUS, MAX_RADIUS );

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

	float distort=PI180*distortion,PIpts=PI/(float)points;
	// Now add all the necessary points
	for(int ix=0; ix<(2*points); ++ix) 
	{ if (ix%2) 	// Points for radius 1
		{ 	angle = PI * (float)ix / (float)points;
			p.x = (float)cos(angle) * radius1;
			p.y = (float)sin(angle) * radius1;
			p.z = 0.0f;
			if (fillet1>0.0f)
			{ float theta1,theta2,stheta1,stheta2,ctheta1,ctheta2;
			  theta1=angle-PIpts;
			  theta2=angle+PIpts;
			  stheta1=(float)sin(theta1);stheta2=(float)sin(theta2);
			  ctheta1=(float)cos(theta1);ctheta2=(float)cos(theta2);
			  Point3 plast,pnext;
			  plast=Point3(radius2*ctheta1,radius2*stheta1,0.0f);
			  pnext=Point3(radius2*ctheta2,radius2*stheta2,0.0f);
			  Point3 n1=Normalize(plast-p)*fillet1,n2=Normalize(pnext-p)*fillet1;
			  Point3 nk1=n1*CIRCLE_VECTOR_LENGTH,nk2=n2*CIRCLE_VECTOR_LENGTH;
			  Point3 p1=p+n1,p2=p+n2;
			  spline->AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_CURVE,p1,p1+nk1,p1-nk1));
			  spline->AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_CURVE,p2,p2-nk2,p2+nk2));
			} else	spline->AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_CURVE,p,p,p));
		}
		else  // Points for radius 2 with optional angular offset
		{	angle = PIpts * (float)ix + distort;
			p.x = (float)cos(angle) * radius2;
			p.y = (float)sin(angle) * radius2;
			p.z = 0.0f;
			if (fillet2>0.0f)
			{ float theta1,theta2,stheta1,stheta2,ctheta1,ctheta2;
			  theta1=angle-PIpts-distort;
			  theta2=angle+PIpts+distort;
			  stheta1=(float)sin(theta1);stheta2=(float)sin(theta2);
			  ctheta1=(float)cos(theta1);ctheta2=(float)cos(theta2);
			  Point3 plast,pnext;
			  plast=Point3(radius1*ctheta1,radius1*stheta1,0.0f);
			  pnext=Point3(radius1*ctheta2,radius1*stheta2,0.0f);
			  Point3 n1=Normalize(plast-p)*fillet2,n2=Normalize(pnext-p)*fillet2;
			  Point3 nk1=n1*CIRCLE_VECTOR_LENGTH,nk2=n2*CIRCLE_VECTOR_LENGTH;
			  Point3 p1=p+n1,p2=p+n2;
			  spline->AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_CURVE,p1,p1+nk1,p1-nk1));
			  spline->AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_CURVE,p2,p2-nk2,p2+nk2));
			} else	spline->AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_CURVE,p,p,p));
		}
	}
	spline->SetClosed();
	spline->ComputeBezPoints();
	ashape.UpdateSels();	// Make sure it readies the selection set info
	ashape.InvalidateGeomCache();
	}

StarObject::StarObject() : SimpleSpline() 
	{
	ReadyInterpParameterBlock();		// Build the interpolations parameter block in SimpleSpline
	MakeRefByID(FOREVER, USERPBLOCK, CreateParameterBlock(descVer1, PBLOCK_LENGTH, CURRENT_VERSION));
	assert(pblock);
	
	pblock->SetValue(PB_RADIUS1,0,crtRadius1);
	pblock->SetValue(PB_RADIUS2,0,crtRadius2);
	pblock->SetValue(PB_FILLET1,0,crtFillet1);
	pblock->SetValue(PB_FILLET2,0,crtFillet2);
	pblock->SetValue(PB_POINTS,0,dlgPoints);
	pblock->SetValue(PB_DISTORT,0,dlgDistortion);
 	}

StarObject::~StarObject()
	{
	DeleteAllRefsFromMe();
	pblock = NULL;
	UnReadyInterpParameterBlock();
	}

class StarObjCreateCallBack: public CreateMouseCallBack {
	StarObject *ob;
	Point3 p[2];
	IPoint2 cPt,sp1,sp2;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		void SetObj(StarObject *obj) { ob = obj; }
	};

int StarObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
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
				cPt = m;
				p[0] = vpt->SnapPoint(m,m,NULL,snapdim);
				mat.SetTrans(p[0]); // Set Node's transform
				ob->pblock->SetValue(PB_RADIUS1,0,0.01f);
				ob->pblock->SetValue(PB_RADIUS2,0,0.01f);
				ob->pmapParam->Invalidate();
				break;
			case 1: 
				p[1] = vpt->SnapPoint(m,m,NULL,snapdim);
				
				r = Length(p[1]-p[0]);
				
				ob->pblock->SetValue(PB_RADIUS1,0,r);
				ob->pblock->SetValue(PB_RADIUS2,0,r/2.0f);
				ob->pmapParam->Invalidate();

				// If Radius 1 is zero we abort creation
				if ((msg==MOUSE_POINT) && (Length(m-cPt)<3 || Length(p[1]-p[0])<0.1f)) {
					return CREATE_ABORT;
					}

				break;
			case 2: 
				sp1=m;
				p[1] = vpt->SnapPoint(m,m,NULL,snapdim);
				
				r = Length(p[1]-p[0]);
				
				ob->pblock->SetValue(PB_RADIUS2,0,r);
				ob->pmapParam->Invalidate();

				// Radius 2 can be 0, do not abort creation here
				  if (msg==MOUSE_POINT) 
				  {	ob->suspendSnap = FALSE;
					return CREATE_STOP;
				  }
				break;
/*			case 3: 
				{ sp2 = m;							   
				  float f = vpt->SnapLength(vpt->GetCPDisp(p[1],Point3(0,0,1),sp1,m));
				  ob->pblock->SetValue(PB_FILLET1,0,f);
				  ob->pmapParam->Invalidate();
				}
				break;
			case 4: 
				{ float f = vpt->SnapLength(vpt->GetCPDisp(p[1],Point3(0,0,1),sp2,m));
				  ob->pblock->SetValue(PB_FILLET2,0,f);
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

static StarObjCreateCallBack starCreateCB;

CreateMouseCallBack* StarObject::GetCreateMouseCallBack() {
	starCreateCB.SetObj(this);
	return(&starCreateCB);
	}

//
// Reference Managment:
//

RefTargetHandle StarObject::Clone(RemapDir& remap) {
	StarObject* newob = new StarObject();
	newob->SimpleSplineClone(this);
	newob->ReplaceReference(USERPBLOCK,pblock->Clone(remap));	
	newob->ivalid.SetEmpty();	
	BaseClone(this, newob, remap);
	return(newob);
	}

BOOL StarObject::ValidForDisplay(TimeValue t) {
	float radius1, radius2;
	pblock->GetValue(PB_RADIUS1, t, radius1, ivalid);
	pblock->GetValue(PB_RADIUS2, t, radius2, ivalid);
	return (radius1 == 0.0f && radius2 == 0.0f) ? FALSE : TRUE;
	}

ParamDimension *StarObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_RADIUS1:
		case PB_RADIUS2:
		case PB_FILLET1:
		case PB_FILLET2:
			return stdWorldDim;			
		case PB_POINTS:
			return stdSegmentsDim;			
		default:
			return defaultDim;
		}
	}

TSTR StarObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_RADIUS1:
			return TSTR(GetString(IDS_TH_RADIUS1));
		case PB_RADIUS2:
			return TSTR(GetString(IDS_TH_RADIUS2));
		case PB_FILLET1:
			return TSTR(GetString(IDS_AP_FILLET1));
		case PB_FILLET2:
			return TSTR(GetString(IDS_AP_FILLET2));
		case PB_POINTS:
			return TSTR(GetString(IDS_TH_STARPOINTS));
		case PB_DISTORT:
			return TSTR(GetString(IDS_TH_DISTORTION));
		default:
			return TSTR(_T(""));
		}
	}

// From ParamArray
BOOL StarObject::SetValue(int i, TimeValue t, int v) 
	{
	return TRUE;
	}

BOOL StarObject::SetValue(int i, TimeValue t, float v)
	{
	switch (i) {				
		case PB_TI_RADIUS1: crtRadius1 = v; break;
		case PB_TI_RADIUS2: crtRadius2 = v; break;
		case PB_TI_FILLET1: crtFillet1 = v; break;
		case PB_TI_FILLET2: crtFillet2 = v; break;
		}	
	return TRUE;
	}

BOOL StarObject::SetValue(int i, TimeValue t, Point3 &v) 
	{
	switch (i) {
		case PB_TI_POS: crtPos = v; break;
		}		
	return TRUE;
	}

BOOL StarObject::GetValue(int i, TimeValue t, int &v, Interval &ivalid) 
	{
	return TRUE;
	}

BOOL StarObject::GetValue(int i, TimeValue t, float &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_RADIUS1: v = crtRadius1; break;
		case PB_TI_RADIUS2: v = crtRadius2; break;
		case PB_TI_FILLET1: v = crtFillet1; break;
		case PB_TI_FILLET2: v = crtFillet2; break;
		}
	return TRUE;
	}

BOOL StarObject::GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_POS: v = crtPos; break;		
		}
	return TRUE;
	}

#endif // NO_OBJECT_SHAPES_SPLINES

