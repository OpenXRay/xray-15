/**********************************************************************
 *<
	FILE: helix.cpp

	DESCRIPTION:  A Helix object implementation

	CREATED BY: Tom Hudson

	HISTORY: created 19 October 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/
#include "prim.h" 

#ifndef NO_OBJECT_SHAPES_SPLINES

#include "splshape.h"
#include "iparamm.h"
// This is based on the simple spline object...
#include "simpshp.h"

// This controls how fine the helix object is
#define POINTS_PER_TURN 40

#define MIN_RADIUS		float(0)
#define MAX_RADIUS		float( 1.0E30)
#define MIN_HEIGHT		float( -1.0E30)
#define MAX_HEIGHT		float( 1.0E30)
#define MIN_TURNS		float(0)
#define MAX_TURNS		float(100)
#define MIN_BIAS		float(-1)
#define MAX_BIAS		float(1)

#define DEF_TURNS		float(1)
#define DEF_BIAS		float(0)

class HelixObjCreateCallBack;

class HelixObject: public SimpleShape, public IParamArray {			   
		friend class HelixObjCreateCallBack;

	public:	
		// Class vars
		static IParamMap *pmapCreate;
		static IParamMap *pmapTypeIn;
		static IParamMap *pmapParam;
		static IObjParam *ip;
		static int dlgCreateMeth;
		static float dlgTurns;
		static float dlgBias;
		static int dlgDirection;
		static Point3 crtPos;		
		static float crtHeight;
		static float crtRadius1;
		static float crtRadius2;

		// Interpolation parameters and validity
		Interval interpValid;
		float radius1, radius2, height, turns, bias, power;
		int direction;
		// Useful pre-computed values
		float totalRadians,deltaRadius;
		float perPiece, lengthOfCurve;
		int numberOfPieces;
						
		void BuildShape(TimeValue t,PolyShape& ashape);

		HelixObject();
		~HelixObject();

		//  inherited virtual methods:

		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		TCHAR *GetObjectName() { return GetString(IDS_TH_HELIX); }
		void InitNodeName(TSTR& s) { s = GetString(IDS_TH_HELIX); }		
		Class_ID ClassID() { return Class_ID(HELIX_CLASS_ID,0); }  
		void GetClassName(TSTR& s) { s = TSTR(GetString(IDS_TH_HELIX_CLASS)); }
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		BOOL ValidForDisplay(TimeValue t);

		// From SimpleShape
		Point3 InterpCurve3D(TimeValue t, int curve, float param, int ptype);
		Point3 TangentCurve3D(TimeValue t, int curve, float param, int ptype);
		float LengthOfCurve(TimeValue t, int curve);

		// Interpolation parameter functions -- This subsystem maintains
		// a separate interval for precomputed values that we maintain
		// for rapid analysis of our procedural curve!
		void ReadyInterpParams(TimeValue t);
		void UpdateInterpParams(TimeValue t);
		
		// Here are some optional methods.
		// You should _really_ implement these, because they just do the bare-minimum job
		// (Chopping your curve up into manageable pieces makes things look better)
		int NumberOfPieces(TimeValue t, int curve);
		Point3 InterpPiece3D(TimeValue t, int curve, int piece, float param, int ptype);
		Point3 TangentPiece3D(TimeValue t, int curve, int piece, float param, int ptype);

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

// Directions
#define DIR_CW	0
#define DIR_CCW	1

//------------------------------------------------------

class HelixObjClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new HelixObject; }
	const TCHAR *	ClassName() { return GetString(IDS_TH_HELIX_CLASS); }
	SClass_ID		SuperClassID() { return SHAPE_CLASS_ID; }
   	Class_ID		ClassID() { return Class_ID(HELIX_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_TH_SPLINES);  }
	void			ResetClassParams(BOOL fileReset);
	};

static HelixObjClassDesc helixObjDesc;

ClassDesc* GetHelixDesc() { return &helixObjDesc; }

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// class variable for helix class.
IParamMap *HelixObject::pmapCreate = NULL;
IParamMap *HelixObject::pmapParam  = NULL;
IParamMap *HelixObject::pmapTypeIn = NULL;
IObjParam *HelixObject::ip         = NULL;
Point3 HelixObject::crtPos         = Point3(0,0,0);
float HelixObject::crtHeight       = 0.0f;
float HelixObject::crtRadius1      = 0.0f;
float HelixObject::crtRadius2      = 0.0f;
int HelixObject::dlgCreateMeth     = 1; // create_radius
float HelixObject::dlgTurns        = DEF_TURNS;
float HelixObject::dlgBias         = DEF_BIAS;
int HelixObject::dlgDirection      = DIR_CW;

void HelixObjClassDesc::ResetClassParams(BOOL fileReset)
	{
	HelixObject::crtPos          = Point3(0,0,0);
	HelixObject::crtHeight       = 0.0f;
	HelixObject::crtRadius1      = 0.0f;
	HelixObject::crtRadius2      = 0.0f;
	HelixObject::dlgCreateMeth   = 1; // create_radius
	HelixObject::dlgTurns        = DEF_TURNS;
	HelixObject::dlgBias         = DEF_BIAS;
	HelixObject::dlgDirection    = DIR_CW;
	}

// Parameter map indices
#define PB_RADIUS1		0
#define PB_RADIUS2		1
#define PB_HEIGHT		2
#define PB_TURNS		3
#define PB_BIAS			4
#define PB_DIRECTION	5

// Non-parameter block indices
#define PB_CREATEMETHOD		0
#define PB_TI_POS			1
#define PB_TI_RADIUS1		2
#define PB_TI_RADIUS2		3
#define PB_TI_HEIGHT		4

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
	
	// Height
	ParamUIDesc(
		PB_TI_HEIGHT,
		EDITTYPE_UNIVERSE,
		IDC_HEIGHT,IDC_HEIGHTSPINNER,
		MIN_HEIGHT,MAX_HEIGHT,
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
#define TYPEINDESC_LENGTH 4

//
//
// Parameters

static int directionIDs[] = {IDC_HELIX_CW,IDC_HELIX_CCW};

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
	
	// Height
	ParamUIDesc(
		PB_HEIGHT,
		EDITTYPE_UNIVERSE,
		IDC_HEIGHTEDIT,IDC_HEIGHTSPINNER,
		MIN_HEIGHT,MAX_HEIGHT,
		SPIN_AUTOSCALE),	
	
	// Turns
	ParamUIDesc(
		PB_TURNS,
		EDITTYPE_POS_FLOAT,
		IDC_TURNSEDIT,IDC_TURNSSPINNER,
		MIN_TURNS,MAX_TURNS,
		SPIN_AUTOSCALE),	
	
	// Bias
	ParamUIDesc(
		PB_BIAS,
		EDITTYPE_FLOAT,
		IDC_BIASEDIT,IDC_BIASSPINNER,
		MIN_BIAS,MAX_BIAS,
		SPIN_AUTOSCALE),
		
	// Direction
	ParamUIDesc(PB_DIRECTION,TYPE_RADIO,directionIDs,2)	
	
	};
#define PARAMDESC_LENGTH 6


static ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },		
	{ TYPE_FLOAT, NULL, TRUE, 1 },		
	{ TYPE_FLOAT, NULL, TRUE, 2 },		
	{ TYPE_FLOAT, NULL, TRUE, 3 },		
	{ TYPE_FLOAT, NULL, TRUE, 4 },		
	{ TYPE_INT, NULL, FALSE, 5 } };
#define PBLOCK_LENGTH	6

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,6,0)			// This is a dummy entry!!!
	};
#define NUM_OLDVERSIONS	0	// No old ones yet!

// Current version
#define CURRENT_VERSION	0
static ParamVersionDesc curVersion(descVer0,PBLOCK_LENGTH,CURRENT_VERSION);

//--- TypeInDlgProc --------------------------------

class HelixTypeInDlgProc : public ParamMapUserDlgProc {
	public:
		HelixObject *dob;

		HelixTypeInDlgProc(HelixObject *d) {dob=d;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

BOOL HelixTypeInDlgProc::DlgProc(
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
						dob->pblock->SetValue(PB_HEIGHT,0,dob->crtHeight);
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

void HelixObject::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
	{
	SimpleShape::BeginEditParams(ip,flags,prev);
	this->ip = ip;

	if (pmapCreate && pmapParam) {
		
		// Left over from last object ceated
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
				MAKEINTRESOURCE(IDD_HELIXPARAM1),
				GetString(IDS_TH_CREATION_METHOD),
				0);

			pmapTypeIn = CreateCPParamMap(
				descTypeIn,TYPEINDESC_LENGTH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_HELIXPARAM3),
				GetString(IDS_TH_KEYBOARD_ENTRY),
				APPENDROLL_CLOSED);
			}

		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_HELIXPARAM2),
			GetString(IDS_TH_PARAMETERS),
			0);
		}

	if(pmapTypeIn) {
		// A callback for the type in.
		pmapTypeIn->SetUserDlgProc(new HelixTypeInDlgProc(this));
		}
	}
		
void HelixObject::EndEditParams( IObjParam *ip,ULONG flags,Animatable *next )
	{
	SimpleShape::EndEditParams(ip,flags,next);
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
	pblock->GetValue(PB_TURNS,ip->GetTime(),dlgTurns,FOREVER);
	pblock->GetValue(PB_BIAS,ip->GetTime(),dlgBias,FOREVER);
	pblock->GetValue(PB_DIRECTION,ip->GetTime(),dlgDirection,FOREVER);
	}

void HelixObject::BuildShape(TimeValue t, PolyShape& ashape) {
	// Set the object's interpolation parameters
	ReadyInterpParams(t);
	// Set the general interval to the one we just got
	ivalid = interpValid;
	ashape.NewShape();
	// Start a PolyLine for our helix...
	PolyLine *line = ashape.NewLine();
	// Compute some helpful stuff...
	int points = (int)(turns * (float)POINTS_PER_TURN);
	if(points == 0)
		points = 1;
	float fPoints = (float)points;
	numberOfPieces = 0;
	for(int i = 0; i <= points; ++i) {
		float pct = (float)i / fPoints;
		float r = radius1 + deltaRadius * pct;
		float hpct = pct;
		if(bias > 0.0f)
			hpct = 1.0f - (float)pow(1.0f - pct, power );
		else
		if(bias < 0.0f)
			hpct = (float)pow(pct, power);
			
		float z = height * hpct;
		float angle = totalRadians * pct;
		float x = r * (float)cos(angle);
		float y = r * (float)sin(angle);
		DWORD flags = POLYPT_SMOOTH;
		if((i % 10) == 0 || i == points) {
			flags |= POLYPT_KNOT;	// Let it know the piece boundary
			if(i > 0)
				numberOfPieces++;
			}
		line->Append(PolyPt(Point3(x,y,z), flags));
		}
	perPiece = 1.0f / (float)numberOfPieces;
	ashape.UpdateSels();	// Make sure it readies the selection set info
	ashape.InvalidateGeomCache(FALSE);
	lengthOfCurve = ashape.lines[0].CurveLength();
	}

HelixObject::HelixObject() : SimpleShape() 
	{
	ReadyGeneralParameters();		// Build the general parameter block in SimpleShape
	MakeRefByID(FOREVER, USERPBLOCK, CreateParameterBlock(descVer0, PBLOCK_LENGTH, CURRENT_VERSION));
	assert(pblock);
	
	pblock->SetValue(PB_RADIUS1,0,crtRadius1);
	pblock->SetValue(PB_RADIUS2,0,crtRadius2);
	pblock->SetValue(PB_HEIGHT,0,crtHeight);
	pblock->SetValue(PB_TURNS,0,dlgTurns);
	pblock->SetValue(PB_BIAS,0,dlgBias);
	pblock->SetValue(PB_DIRECTION,0,dlgDirection);

	interpValid.SetEmpty();
 	}

HelixObject::~HelixObject()
	{
	DeleteAllRefsFromMe();
	pblock = NULL;
	}

class HelixObjCreateCallBack: public CreateMouseCallBack {
	HelixObject *ob;
	Point3 p[3];
	IPoint2 sp0, sp1, sp2;
	Point3 center;
	float r1, ht;
	int createType;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		void SetObj(HelixObject *obj) { ob = obj; }
	};

int HelixObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
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
				sp1 = m; 
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
				ob->pblock->SetValue(PB_RADIUS2,0,r);	// Make this the same for now
				ob->pmapParam->Invalidate();
				r1 = r;
				if (msg==MOUSE_POINT) {
					if(Length(m-sp0)<3 ||
					   Length(p[1]-p[0])<0.1f) {
						return CREATE_ABORT;
						}
					}
				break;
			case 2:
				sp2 = m;
#ifdef _OSNAP
				ht = vpt->SnapLength(vpt->GetCPDisp(p[1],Point3(0,0,1),sp1,m,TRUE));  
#else
				ht = vpt->SnapLength(vpt->GetCPDisp(p[1],Point3(0,0,1),sp1,m));  
#endif
				ob->pblock->SetValue(PB_HEIGHT,0,float(ht));
				ob->pmapParam->Invalidate();
				break;
			case 3:
				r = vpt->SnapLength(vpt->GetCPDisp(p[1],Point3(0,0,1),sp2,m))
						+ r1;
				ob->pblock->SetValue(PB_RADIUS2,0,r);
				ob->pmapParam->Invalidate();
				if (msg==MOUSE_POINT) {
					ob->suspendSnap = FALSE;
					return CREATE_STOP;
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

static HelixObjCreateCallBack helixCreateCB;

CreateMouseCallBack* HelixObject::GetCreateMouseCallBack() {
	helixCreateCB.SetObj(this);
	return(&helixCreateCB);
	}


//
// Reference Managment:
//

RefTargetHandle HelixObject::Clone(RemapDir& remap) {
	HelixObject* newob = new HelixObject();
	newob->SimpleShapeClone(this);
	newob->ReplaceReference(USERPBLOCK,pblock->Clone(remap));	
	newob->ivalid.SetEmpty();	
	BaseClone(this, newob, remap);
	return(newob);
	}

BOOL HelixObject::ValidForDisplay(TimeValue t) {
	float radius1, radius2;
	pblock->GetValue(PB_RADIUS1, t, radius1, ivalid);
	pblock->GetValue(PB_RADIUS2, t, radius2, ivalid);
	LimitValue( radius1, MIN_RADIUS, MAX_RADIUS );
	LimitValue( radius2, MIN_RADIUS, MAX_RADIUS );
	return (radius1 == 0.0f && radius2 == 0.0f) ? FALSE : TRUE;
	}

void HelixObject::ReadyInterpParams(TimeValue t) {
	interpValid = FOREVER;
	pblock->GetValue(PB_RADIUS1,t,radius1,interpValid);
	pblock->GetValue(PB_RADIUS2,t,radius2,interpValid);
	pblock->GetValue(PB_HEIGHT,t,height,interpValid);
	pblock->GetValue(PB_TURNS,t,turns,interpValid);
	pblock->GetValue(PB_BIAS,t,bias,interpValid);
	pblock->GetValue(PB_DIRECTION,t,direction,interpValid);
	LimitValue( radius1, MIN_RADIUS, MAX_RADIUS );
	LimitValue( radius2, MIN_RADIUS, MAX_RADIUS );
	LimitValue( height, MIN_HEIGHT, MAX_HEIGHT );
	LimitValue( turns, MIN_TURNS, MAX_TURNS );
	LimitValue( bias, MIN_BIAS, MAX_BIAS );
	totalRadians = TWOPI * turns * ((direction == DIR_CCW) ? 1.0f : -1.0f);
	deltaRadius = radius2 - radius1;
	float quarterTurns = turns * 4.0f;
	power = 1.0f;
	if(bias > 0.0f)
		power = bias * 9.0f + 1.0f;
	else
	if(bias < 0.0f)
		power = -bias * 9.0f + 1.0f;
	}

void HelixObject::UpdateInterpParams(TimeValue t) {
	if(interpValid.InInterval(t))
		return;
	ReadyInterpParams(t);
	}

Point3 HelixObject::InterpCurve3D(TimeValue t, int curve, float param, int ptype) {
	if(param < 0.0f)
		param = 0.0f;
	else
	if(param > 1.0f)
		param = 1.0f;
	assert(curve==0);
	UpdateShape(t);
	switch(ptype) {
		case PARAM_SIMPLE: {
			float r = radius1 + deltaRadius * param;
			float angle = totalRadians * param;
			float hpct = param;
			if(bias > 0.0f)
				hpct = 1.0f - (float)pow(1.0f - param, power );
			else
			if(bias < 0.0f)
				hpct = (float)pow(param, power);
			return Point3(r * (float)cos(angle),r * (float)sin(angle),height * hpct);
			}
		case PARAM_NORMALIZED:
			return shape.lines[0].InterpCurve3D(param, POLYSHP_INTERP_NORMALIZED);
		}
	assert(0);
	return Point3(0,0,0);
	}

Point3 HelixObject::TangentCurve3D(TimeValue t, int curve, float param, int ptype) {
	assert(curve==0);
	UpdateShape(t);
	float pp = param - perPiece / 100.0f;
	if(pp < 0.0f)
		pp = 0.0f;
	float np = param + perPiece / 100.0f;
	if(np < 0.0f)
		np = 0.0f;
	Point3 prev = InterpCurve3D(t, 0, pp, ptype);
	Point3 next = InterpCurve3D(t, 0, np, ptype);
	return(Normalize(next-prev));
	}

float HelixObject::LengthOfCurve(TimeValue t, int curve) {
	UpdateShape(t);
	return lengthOfCurve;
	}

int HelixObject::NumberOfPieces(TimeValue t, int curve) {
	assert(curve==0);
	UpdateShape(t);
	return numberOfPieces;
	}

Point3 HelixObject::InterpPiece3D(TimeValue t, int curve, int piece, float param, int ptype) {
	assert(curve==0);
	UpdateShape(t);
	return InterpCurve3D(t, 0, (float)piece * perPiece + param * perPiece, ptype);
	}

Point3 HelixObject::TangentPiece3D(TimeValue t, int curve, int piece, float param, int ptype) {
	assert(curve==0);
	UpdateShape(t);
	return TangentCurve3D(t, curve, (float)piece * perPiece + param * perPiece, ptype);
	}

ParamDimension *HelixObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_RADIUS1:
		case PB_RADIUS2:
		case PB_HEIGHT:
			return stdWorldDim;			
		case PB_TURNS:
			return defaultDim;
		case PB_BIAS:
			return defaultDim;
		case PB_DIRECTION:
			return stdNormalizedDim; 
		default:
			return defaultDim;
		}
	}

TSTR HelixObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_RADIUS1:
			return TSTR(GetString(IDS_RB_RADIUS1));
		case PB_RADIUS2:
			return TSTR(GetString(IDS_RB_RADIUS2));
		case PB_HEIGHT:
			return TSTR(GetString(IDS_TH_HEIGHT));
		case PB_TURNS:
			return TSTR(GetString(IDS_TH_TURNS));
		case PB_BIAS:
			return TSTR(GetString(IDS_TH_BIAS));
		case PB_DIRECTION:
			return TSTR(GetString(IDS_TH_DIRECTION));
		default:
			return TSTR(_T(""));
		}
	}

// From ParamArray
BOOL HelixObject::SetValue(int i, TimeValue t, int v) 
	{
	switch (i) {
		case PB_CREATEMETHOD: dlgCreateMeth = v; break;
		}		
	return TRUE;
	}

BOOL HelixObject::SetValue(int i, TimeValue t, float v)
	{
	switch (i) {				
		case PB_TI_RADIUS1: crtRadius1 = v; break;
		case PB_TI_RADIUS2: crtRadius2 = v; break;
		case PB_TI_HEIGHT: crtHeight = v; break;
		}	
	return TRUE;
	}

BOOL HelixObject::SetValue(int i, TimeValue t, Point3 &v) 
	{
	switch (i) {
		case PB_TI_POS: crtPos = v; break;
		}		
	return TRUE;
	}

BOOL HelixObject::GetValue(int i, TimeValue t, int &v, Interval &ivalid) 
	{
	switch (i) {
		case PB_CREATEMETHOD: v = dlgCreateMeth; break;
		}
	return TRUE;
	}

BOOL HelixObject::GetValue(int i, TimeValue t, float &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_RADIUS1: v = crtRadius1; break;
		case PB_TI_RADIUS2: v = crtRadius2; break;
		case PB_TI_HEIGHT: v = crtHeight; break;
		}
	return TRUE;
	}

BOOL HelixObject::GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_POS: v = crtPos; break;		
		}
	return TRUE;
	}

#endif // NO_OBJECT_SHAPES_SPLINES

