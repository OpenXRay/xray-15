/**********************************************************************
 *<
	FILE: arc.cpp

	DESCRIPTION:  An Arc object implementation

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

#define MIN_ANGLE		float(0.0)
#define MAX_ANGLE		float(360.0)

#define DEF_RADIUS		float(0.0)
#define DEF_START		float(0.0)
#define DEF_END			float(360.0)
#define DEF_REVERSE		FALSE

#define CREATE_EEC 0
#define CREATE_CEE 1

class ArcObjCreateCallBack;

class ArcObject: public SimpleSpline, public IParamArray {			   

	friend class ArcObjCreateCallBack;
	
	public:
		BOOL halfBaked;		// If TRUE, the arc is in the creation phase and needs a special draw
		Point3 hb;			// The halfBaked line endpoint

		// Class vars
		static IParamMap *pmapCreate;
		static IParamMap *pmapTypeIn;
		static IParamMap *pmapParam;
		static IObjParam *ip;
		static int dlgCreateMeth;
		static int dlgPie;
		static BOOL dlgReverse;
		static Point3 crtPos;		
		static float crtRadius;
		static float crtFrom;
		static float crtTo;
		static int crtPie;		// Probable future enhancement
		
		void BuildShape(TimeValue t,BezierShape& ashape);

		ArcObject();
		~ArcObject();

		//  inherited virtual methods:

		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		TCHAR *GetObjectName() { return GetString(IDS_TH_ARC); }
		void InitNodeName(TSTR& s) { s = GetString(IDS_TH_ARC); }		
		Class_ID ClassID() { return Class_ID(ARC_CLASS_ID,0); }  
		void GetClassName(TSTR& s) { s = TSTR(GetString(IDS_TH_ARC_CLASS)); }
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		IOResult Load(ILoad *iload);
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

class ArcObjClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new ArcObject; }
	const TCHAR *	ClassName() { return GetString(IDS_TH_ARC_CLASS); }
	SClass_ID		SuperClassID() { return SHAPE_CLASS_ID; }
   	Class_ID		ClassID() { return Class_ID(ARC_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_TH_SPLINES);  }
	void			ResetClassParams(BOOL fileReset);
	};

static ArcObjClassDesc arcObjDesc;

ClassDesc* GetArcDesc() { return &arcObjDesc; }

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// class variable for arc class.
IParamMap *ArcObject::pmapCreate = NULL;
IParamMap *ArcObject::pmapParam  = NULL;
IParamMap *ArcObject::pmapTypeIn = NULL;
IObjParam *ArcObject::ip         = NULL;
Point3 ArcObject::crtPos         = Point3(0,0,0);
float ArcObject::crtRadius       = 0.0f;
float ArcObject::crtFrom         = 0.0f;
float ArcObject::crtTo           = 360.0f;
int ArcObject::crtPie            = 0;
int ArcObject::dlgCreateMeth = CREATE_EEC;
int ArcObject::dlgPie = 0;
BOOL ArcObject::dlgReverse = FALSE;

void ArcObjClassDesc::ResetClassParams(BOOL fileReset)
	{
	ArcObject::crtPos         = Point3(0,0,0);
	ArcObject::crtRadius      = 0.0f;
	ArcObject::crtFrom        = 0.0f;
	ArcObject::crtTo          = 360.0f;
	ArcObject::crtPie         = 0;
	ArcObject::dlgCreateMeth  = CREATE_EEC;
	ArcObject::dlgPie         = 0;
	ArcObject::dlgReverse     = FALSE;
	}

// Parameter map indices
#define PB_RADIUS		0
#define PB_FROM			1
#define PB_TO			2
#define PB_PIE			3
#define PB_REVERSE		4

// Non-parameter block indices
#define PB_CREATEMETHOD		0
#define PB_TI_POS			1
#define PB_TI_RADIUS		2
#define PB_TI_FROM			3
#define PB_TI_TO			4

// Vector length for unit arc
#define CIRCLE_VECTOR_LENGTH 0.5517861843f

//
//
//	Creation method

static int createMethIDs[] = {IDC_CREATE_EEC,IDC_CREATE_CEE};

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
			
	// From Angle
	ParamUIDesc(
		PB_TI_FROM,
		EDITTYPE_POS_FLOAT,
		IDC_TI_ARCFROM,IDC_TI_ARCFROMSPINNER,
		MIN_ANGLE,MAX_ANGLE,
		SPIN_AUTOSCALE),
			
	// To Angle
	ParamUIDesc(
		PB_TI_TO,
		EDITTYPE_POS_FLOAT,
		IDC_TI_ARCTO,IDC_TI_ARCTOSPINNER,
		MIN_ANGLE,MAX_ANGLE,
		SPIN_AUTOSCALE)
			
	};
#define TYPEINDESC_LENGTH 4

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

	// From Angle
	ParamUIDesc(
		PB_FROM,
		EDITTYPE_FLOAT,
		IDC_ARCFROM,IDC_ARCFROMSPINNER,
		MIN_ANGLE,MAX_ANGLE,
		SPIN_AUTOSCALE),
			
	// To Angle
	ParamUIDesc(
		PB_TO,
		EDITTYPE_FLOAT,
		IDC_ARCTO,IDC_ARCTOSPINNER,
		MIN_ANGLE,MAX_ANGLE,
		SPIN_AUTOSCALE),

	// Pie Slice
	ParamUIDesc(PB_PIE,TYPE_SINGLECHEKBOX,IDC_ARCPIE),	
				
	// Reverse
	ParamUIDesc(PB_REVERSE,TYPE_SINGLECHEKBOX,IDC_REVERSE),	
	};
#define PARAMDESC_LENGTH 5


static ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },		
	{ TYPE_FLOAT, NULL, TRUE, 1 },		
	{ TYPE_FLOAT, NULL, TRUE, 2 },		
	{ TYPE_INT, NULL, FALSE, 3 } };
static ParamBlockDescID descVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },		
	{ TYPE_FLOAT, NULL, TRUE, 1 },		
	{ TYPE_FLOAT, NULL, TRUE, 2 },		
	{ TYPE_INT, NULL, FALSE, 3 },
	{ TYPE_INT, NULL, FALSE, 4} };
#define PBLOCK_LENGTH	5

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,4,0)
	};
#define NUM_OLDVERSIONS	1

// Current version
#define CURRENT_VERSION	1
static ParamVersionDesc curVersion(descVer1,PBLOCK_LENGTH,CURRENT_VERSION);

//--- TypeInDlgProc --------------------------------

class ArcTypeInDlgProc : public ParamMapUserDlgProc {
	public:
		ArcObject *ao;

		ArcTypeInDlgProc(ArcObject *c) {ao=c;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

BOOL ArcTypeInDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_TI_CREATE: {
					if (ao->crtRadius==0.0 || ao->crtFrom == ao->crtTo) return TRUE;
					
					// Return focus to the top spinner
					SetFocus(GetDlgItem(hWnd, IDC_TI_POSX));
					
					// We only want to set the value if the object is 
					// not in the scene.
					if (ao->TestAFlag(A_OBJ_CREATING)) {
						ao->pblock->SetValue(PB_RADIUS,0,ao->crtRadius);
						ao->pblock->SetValue(PB_FROM,0,ao->crtFrom);
						ao->pblock->SetValue(PB_TO,0,ao->crtTo);
						ao->pblock->SetValue(PB_PIE,0,ao->crtPie);
						}

					Matrix3 tm(1);
					tm.SetTrans(ao->crtPos);
					ao->ip->NonMouseCreate(tm);
					// NOTE that calling NonMouseCreate will cause this
					// object to be deleted. DO NOT DO ANYTHING BUT RETURN.
					return TRUE;	
					}
				}
			break;	
		}
	return FALSE;
	}

void ArcObject::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
	{
	SimpleSpline::BeginEditParams(ip,flags,prev);
	this->ip = ip;

	if (pmapCreate && pmapParam) {
		
		// Left over from last arc ceated
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
				MAKEINTRESOURCE(IDD_ARCPARAM1),
				GetString(IDS_TH_CREATION_METHOD),
				0);

			pmapTypeIn = CreateCPParamMap(
				descTypeIn,TYPEINDESC_LENGTH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_ARCPARAM3),
				GetString(IDS_TH_KEYBOARD_ENTRY),
				APPENDROLL_CLOSED);
			}

		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_ARCPARAM2),
			GetString(IDS_TH_PARAMETERS),
			0);
		}

	if(pmapTypeIn) {
		// A callback for the type in.
		pmapTypeIn->SetUserDlgProc(new ArcTypeInDlgProc(this));
		}
	}
		
void ArcObject::EndEditParams( IObjParam *ip,ULONG flags,Animatable *next )
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
	pblock->GetValue(PB_PIE,ip->GetTime(),dlgPie,FOREVER);
	pblock->GetValue(PB_REVERSE,ip->GetTime(),dlgReverse,FOREVER);
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

static void MakeArc(BezierShape& ashape, float radius, float from, float to, ArcObject *ob, BOOL pie, BOOL reverse) {
	// Delete all points in the existing spline
	Spline3D *spline = ashape.NewSpline();
	if(ob->halfBaked) {		// Special construction during creation
		Point3 origin(0,0,0);
		spline->AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_LINE,origin,origin,origin));
		spline->AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_LINE,ob->hb,ob->hb,ob->hb));
		}
	else {
		Point3 origin(0,0,0);
		// Order angles properly
		if(from > to)
			to += TWOPI;
		float totAngle = to - from;
		float vector = veccalc(totAngle / 3.0f) * radius;
		// Now add all the necessary points
		float angStep = totAngle / 3.0f;
		for(int ix=0; ix<4; ++ix) {
			float angle = from + (float)ix * angStep;
			float sinfac = (float)sin(angle), cosfac = (float)cos(angle);
			Point3 p(cosfac * radius, sinfac * radius, 0.0f);
			Point3 rotvec = Point3(sinfac * vector, -cosfac * vector, 0.0f);
			Point3 invec = (ix==0) ? p : p + rotvec;
			Point3 outvec = (ix==3) ? p : p - rotvec;
			spline->AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_CURVE,p,invec,outvec));
			}
		if(pie) {
			spline->AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_CURVE,origin,origin,origin));
			spline->SetClosed();
			}
		}
	spline->ComputeBezPoints();
	if(reverse)
		spline->Reverse(TRUE);
	}

void ArcObject::BuildShape(TimeValue t, BezierShape& ashape) {
	// Start the validity interval at forever and whittle it down.
	ivalid = FOREVER;
	float radius, from, to;
	int pie;
	BOOL reverse;
	pblock->GetValue(PB_RADIUS, t, radius, ivalid);
	pblock->GetValue(PB_FROM, t, from, ivalid);
	pblock->GetValue(PB_TO, t, to, ivalid);
	pblock->GetValue(PB_PIE, t, pie, ivalid);
	pblock->GetValue(PB_REVERSE, t, reverse, ivalid);
	LimitValue( radius, MIN_RADIUS, MAX_RADIUS );
	LimitValue( from, MIN_ANGLE, MAX_ANGLE );
	LimitValue( to, MIN_ANGLE, MAX_ANGLE );

	ashape.NewShape();

	// Get parameters from SimpleSpline and place them in the BezierShape
	int steps;
	BOOL optimize,adaptive;
	ipblock->GetValue(IPB_STEPS, t, steps, ivalid);
	ipblock->GetValue(IPB_OPTIMIZE, t, optimize, ivalid);
	ipblock->GetValue(IPB_ADAPTIVE, t, adaptive, ivalid);
	ashape.steps = adaptive ? -1 : steps;
	ashape.optimize = optimize;

	MakeArc(ashape, radius, from * DEG_TO_RAD, to * DEG_TO_RAD, this, pie, reverse);
	ashape.UpdateSels();	// Make sure it readies the selection set info
	ashape.InvalidateGeomCache();
	}

ArcObject::ArcObject() : SimpleSpline() 
	{
	ReadyInterpParameterBlock();		// Build the interpolations parameter block in SimpleSpline
	MakeRefByID(FOREVER, USERPBLOCK, CreateParameterBlock(descVer1, PBLOCK_LENGTH, CURRENT_VERSION));
	assert(pblock);
	
	pblock->SetValue(PB_RADIUS,0,crtRadius);
	pblock->SetValue(PB_FROM,0,crtFrom);
	pblock->SetValue(PB_TO,0,crtTo);
	pblock->SetValue(PB_PIE,0,dlgPie);
	pblock->SetValue(PB_REVERSE,0,dlgReverse);

	halfBaked = FALSE;
 	}

ArcObject::~ArcObject()
	{
	DeleteAllRefsFromMe();
	pblock = NULL;
	UnReadyInterpParameterBlock();
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

static float ComputeAngle(Point3 pt, Point3 origin) {
	Point3 vec = Normalize(pt - origin);
	// Dot product gives us the angle
	float dot = DotProd(vec, Point3(1,0,0));
	float angle = 0.0f;
	if(dot >= -1.0f && dot < 1.0f)
		angle = (float)-acos(dot);
	if(vec.y > 0.0f)
		angle = -angle;
	if(angle < 0.0f)
		angle += TWOPI;
	return angle;
	}

class ArcObjCreateCallBack: public CreateMouseCallBack {
	ArcObject *ob;
	IPoint2 sp0, sp1, sp2;
	Point3 center, end1, end2, middle;
	int createType;
	float lastAngle, baseAngle, sumAng;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		void SetObj(ArcObject *obj) { ob = obj; }
	};

int ArcObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
	static float r = 0.0f;
	Point3 wp;

#ifdef _3D_CREATE
	DWORD snapdim = SNAP_IN_3D;
#else
	DWORD snapdim = SNAP_IN_PLANE;
#endif

	static int dir;
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
				ob->halfBaked = TRUE;
				sp0 = m;
				createType = ob->dlgCreateMeth;
				wp = vpt->SnapPoint(m,m,NULL,snapdim);
				if(createType == CREATE_CEE) {
					center = end1 = wp;
					ob->hb = Point3(0,0,0);
					}
				else {
					end1 = wp;
					ob->hb = Point3(0,0,0);
					}
				mat.SetTrans(wp); // Set Node's transform
				ob->pblock->SetValue(PB_RADIUS,0,0.01f);
				ob->pmapParam->Invalidate();
				break;
			case 1: 
				sp1 = m;
				wp = vpt->SnapPoint(m,m,NULL,snapdim);
				if ( createType == CREATE_CEE ) {
					ob->hb = wp - center;		// Update half-baked coord
					r = Length(wp - center);
					end1 = wp;
					// Compute the 'from' angle
					float angle = ComputeAngle(end1, center);
					baseAngle = lastAngle = angle;
					ob->pblock->SetValue(PB_FROM,0,angle * RAD_TO_DEG);
					ob->pblock->SetValue(PB_TO,0,angle * RAD_TO_DEG);
					sumAng = 0.0f;
					if(msg==MOUSE_POINT && end1==center)
						return CREATE_ABORT;
					}
				else {
					ob->hb = wp - end1;		// Update half-baked coord
					middle = end2 = wp;
					if(msg==MOUSE_POINT && end1==end2)
						return CREATE_ABORT;
					}
				ob->pblock->SetValue(PB_RADIUS,0,r);
				ob->pmapParam->Invalidate();
				break;
			case 2:
				sp2 = m;
				wp = vpt->SnapPoint(m,m,NULL,snapdim);
				if(createType == CREATE_CEE) {
					ob->halfBaked = FALSE;
					end2 = wp;
					// Compute the 'to' angle
					float angle = ComputeAngle(end2, center);
					float dang = angle - lastAngle;
					if(dang > PI)
						dang -= TWOPI;
					else
					if(dang < -PI)
						dang += TWOPI;
					sumAng += dang;
					if(sumAng < 0.0f) {
						ob->pblock->SetValue(PB_TO,0,baseAngle * RAD_TO_DEG);
						ob->pblock->SetValue(PB_FROM,0,angle * RAD_TO_DEG);
						}
					else {
						ob->pblock->SetValue(PB_FROM,0,baseAngle * RAD_TO_DEG);
						ob->pblock->SetValue(PB_TO,0,angle * RAD_TO_DEG);
						}
					lastAngle = angle;
					}
				else {
					// Need to compute the center.  This is done by getting the perpendicular
					// bisector of the endpoints and intersecting it with the perpendicular
					// bisector of one endpoint and the mouse position.
					Point2 e1 = Point2(end1.x,end1.y);
					Point2 e2 = Point2(end2.x,end2.y);
					Point2 w = Point2(wp.x,wp.y);
					Point2 ec = (e1 + e2) / 2.0f;	// Point between two endpoints
					Point2 v1 = e2 - e1;
					Point2 pv1 = ec + Point2(-v1.y,v1.x);
					Point2 mc = (w + e1) / 2.0f;	// Point between mouse and endpoint 1
					Point2 v2 = e1 - w;
					Point2 pv2 = mc + Point2(-v2.y,v2.x);
					// Now find their intersection
					Point2 c;
					if(FindIntercept(ec,pv1,mc,pv2,c) == 1) {
						center = Point3(c.x,c.y,end1.z);
						r = Length(center - end1);
						mat.SetTrans(center); // Set Node's transform
						ob->halfBaked = FALSE;				
						
						// Now compute the various angles
						float e1Angle = ComputeAngle(end1, center);
						float e2Angle = ComputeAngle(end2, center);
						float wpAngle = ComputeAngle(wp, center);
						// Figure out which is which
						if(e2Angle > e1Angle) {
							if(wpAngle > e1Angle && wpAngle < e2Angle) {
								ob->pblock->SetValue(PB_FROM,0,e1Angle * RAD_TO_DEG);
								ob->pblock->SetValue(PB_TO,0,e2Angle * RAD_TO_DEG);
								}
							else {
								ob->pblock->SetValue(PB_FROM,0,e2Angle * RAD_TO_DEG);
								ob->pblock->SetValue(PB_TO,0,e1Angle * RAD_TO_DEG);
								}
							}
						else
						if(e1Angle > e2Angle) {
							if(wpAngle > e2Angle && wpAngle < e1Angle) {
								ob->pblock->SetValue(PB_FROM,0,e2Angle * RAD_TO_DEG);
								ob->pblock->SetValue(PB_TO,0,e1Angle * RAD_TO_DEG);
								}
							else {
								ob->pblock->SetValue(PB_FROM,0,e1Angle * RAD_TO_DEG);
								ob->pblock->SetValue(PB_TO,0,e2Angle * RAD_TO_DEG);
								}
							}
						else
							r = 0.0f;	// Bad arc!
						}
					else {
						r = 0.0f;
						ob->halfBaked = TRUE;
						}
					ob->pblock->SetValue(PB_RADIUS,0,r);
					}
				ob->pmapParam->Invalidate();
				if (msg==MOUSE_POINT) {
					ob->suspendSnap = FALSE;
					if((Length(sp0-sp1)<3 && Length(sp1-sp2)<3 && Length(sp0-sp2)<3) || r == 0.0f)
						return CREATE_ABORT;
					return (ob->halfBaked) ? CREATE_ABORT : CREATE_STOP;
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

static ArcObjCreateCallBack arcCreateCB;

CreateMouseCallBack* ArcObject::GetCreateMouseCallBack() {
	arcCreateCB.SetObj(this);
	return(&arcCreateCB);
	}

RefTargetHandle ArcObject::Clone(RemapDir& remap) {
	ArcObject* newob = new ArcObject();
	newob->SimpleSplineClone(this);
	newob->ReplaceReference(USERPBLOCK,pblock->Clone(remap));	
	newob->ivalid.SetEmpty();	
	BaseClone(this, newob, remap);
	return(newob);
	}

IOResult ArcObject::Load(ILoad *iload)
	{
	iload->RegisterPostLoadCallback(
		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,USERPBLOCK));
	return SimpleSpline::Load(iload);
	}

BOOL ArcObject::ValidForDisplay(TimeValue t) {
	if(halfBaked)
		return TRUE;
	float radius, from, to;
	pblock->GetValue(PB_RADIUS, t, radius, ivalid);
	pblock->GetValue(PB_FROM, t, from, ivalid);
	pblock->GetValue(PB_TO, t, to, ivalid);
	return (radius == 0.0f || from==to) ? FALSE : TRUE;
	}

ParamDimension *ArcObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_RADIUS:
			return stdWorldDim;			
		case PB_FROM:
		case PB_TO:
			return defaultDim;	// This is an angle but is stored in degrees, not radians
		case PB_PIE:
			return stdNormalizedDim;
		default:
			return defaultDim;
		}
	}

TSTR ArcObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_RADIUS:
			return TSTR(GetString(IDS_TH_RADIUS));			
		case PB_FROM:
			return TSTR(GetString(IDS_TH_FROM));
		case PB_TO:
			return TSTR(GetString(IDS_TH_TO));
		default:
			return TSTR(_T(""));
		}
	}

// From ParamArray
BOOL ArcObject::SetValue(int i, TimeValue t, int v) 
	{
	switch (i) {
		case PB_CREATEMETHOD: dlgCreateMeth = v; break;
		case PB_PIE: dlgPie = v; break;
		case PB_REVERSE: dlgReverse = v; break;
		}		
	return TRUE;
	}

BOOL ArcObject::SetValue(int i, TimeValue t, float v)
	{
	switch (i) {				
		case PB_TI_RADIUS: crtRadius = v; break;
		case PB_TI_FROM: crtFrom = v; break;
		case PB_TI_TO: crtTo = v; break;
		}	
	return TRUE;
	}

BOOL ArcObject::SetValue(int i, TimeValue t, Point3 &v) 
	{
	switch (i) {
		case PB_TI_POS: crtPos = v; break;
		}		
	return TRUE;
	}

BOOL ArcObject::GetValue(int i, TimeValue t, int &v, Interval &ivalid) 
	{
	switch (i) {
		case PB_CREATEMETHOD: v = dlgCreateMeth; break;
		case PB_PIE: v = dlgPie; break;
		case PB_REVERSE: v = dlgReverse; break;
		}
	return TRUE;
	}

BOOL ArcObject::GetValue(int i, TimeValue t, float &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_RADIUS: v = crtRadius; break;
		case PB_TI_FROM: v = crtFrom; break;
		case PB_TI_TO: v = crtTo; break;
		}
	return TRUE;
	}

BOOL ArcObject::GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_POS: v = crtPos; break;		
		}
	return TRUE;
	}

#endif // NO_OBJECT_SHAPES_SPLINES

