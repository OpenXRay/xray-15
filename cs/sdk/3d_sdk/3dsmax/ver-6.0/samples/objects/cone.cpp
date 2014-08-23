/**********************************************************************
 *<
	FILE: cone.cpp

	DESCRIPTION:  Cone object

	CREATED BY: Rolf Berteig

	HISTORY: created November 11 1994

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "prim.h"

#ifndef NO_OBJECT_STANDARD_PRIMITIVES

#include "polyobj.h"
#include "iparamm.h"
#include "Simpobj.h"
#include "surf_api.h"


class ConeObject : public SimpleObject, public IParamArray {
	public:
		// Class vars
		static IParamMap *pmapCreate;
		static IParamMap *pmapTypeIn;
		static IParamMap *pmapParam;		
		static IObjParam *ip;
		static int dlgSegments, dlgSides, dlgCapSegments;
		static int dlgCreateMeth;
		static int dlgSmooth;
		static Point3 crtPos;		
		static float crtRadius1;
		static float crtRadius2;
		static float crtHeight;
		
		ConeObject();
		
		// From Object
		int CanConvertToType(Class_ID obtype);
		Object* ConvertToType(TimeValue t, Class_ID obtype);
		Object* BuildPoly (TimeValue t);
		void GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist);
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);
		BOOL IsParamSurface() {return TRUE;}
		Point3 GetSurfacePoint(TimeValue t, float u, float v,Interval &iv);
		
		// From BaseObject
		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);		
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		TCHAR *GetObjectName() { return GetString(IDS_RB_CONE); }
				
		// Animatable methods		
		void DeleteThis() { delete this; }
		Class_ID ClassID() { return Class_ID(CONE_CLASS_ID,0); }  		
				
		// From ref
		RefTargetHandle Clone(RemapDir& remap = NoRemap());		
		IOResult Load(ILoad *iload);

		// From IParamArray
		BOOL SetValue(int i, TimeValue t, int v);
		BOOL SetValue(int i, TimeValue t, float v);
		BOOL SetValue(int i, TimeValue t, Point3 &v);
		BOOL GetValue(int i, TimeValue t, int &v, Interval &ivalid);
		BOOL GetValue(int i, TimeValue t, float &v, Interval &ivalid);
		BOOL GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid);

		// From SimpleObject
		void BuildMesh(TimeValue t);
		BOOL OKtoDisplay(TimeValue t);
		void InvalidateUI();
		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);
	};

#define MIN_SEGMENTS	1
#define MAX_SEGMENTS	200

#define MIN_SIDES		3
#define MAX_SIDES		200

#define MIN_RADIUS		float(0)
#define MAX_RADIUS		float( 1.0E30)
#define MIN_HEIGHT		float(-1.0E30)
#define MAX_HEIGHT		float( 1.0E30)
#define MIN_PIESLICE	float(-1.0E30)
#define MAX_PIESLICE	float( 1.0E30)

#define DEF_SEGMENTS 	5
#define DEF_SIDES		24

#define DEF_RADIUS		float(0.0)
#define DEF_HEIGHT		float(0.01)

#define SMOOTH_ON		1
#define SMOOTH_OFF		0



//--- ClassDescriptor and class vars ---------------------------------

class ConeClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new ConeObject; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_CONE_CLASS); }
	SClass_ID		SuperClassID() { return GEOMOBJECT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(CONE_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_PRIMITIVES); }
	void			ResetClassParams(BOOL fileReset);
	};

static ConeClassDesc coneDesc;

ClassDesc* GetConeDesc() { return &coneDesc; }

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// class variables for Cone class.
IObjParam *ConeObject::ip         = NULL;
int ConeObject::dlgSegments       = DEF_SEGMENTS;
int ConeObject::dlgCapSegments    = 1;
int ConeObject::dlgSides          = DEF_SIDES;
int ConeObject::dlgCreateMeth     = 1; // create_radius
int ConeObject::dlgSmooth         = SMOOTH_ON;
IParamMap *ConeObject::pmapCreate = NULL;
IParamMap *ConeObject::pmapTypeIn = NULL;
IParamMap *ConeObject::pmapParam  = NULL;
Point3 ConeObject::crtPos         = Point3(0,0,0);
float ConeObject::crtRadius1      = 0.0f;
float ConeObject::crtRadius2      = 0.0f;
float ConeObject::crtHeight       = 0.0f;

void ConeClassDesc::ResetClassParams(BOOL fileReset)
	{
	ConeObject::dlgSegments     = DEF_SEGMENTS;
	ConeObject::dlgCapSegments  = 1;
	ConeObject::dlgSides        = DEF_SIDES;
	ConeObject::dlgCreateMeth   = 1; // create_radius
	ConeObject::dlgSmooth       = SMOOTH_ON;
	ConeObject::crtRadius1      = 0.0f;
	ConeObject::crtRadius2      = 0.0f;
	ConeObject::crtHeight       = 0.0f;
	ConeObject::crtPos          = Point3(0,0,0);
	}


//--- Parameter map/block descriptors -------------------------------

// Parameter map indices
#define PB_RADIUS1		0
#define PB_RADIUS2		1
#define PB_HEIGHT		2
#define PB_SEGMENTS		3
#define PB_CAPSEGMENTS	4
#define PB_SIDES		5
#define PB_SMOOTH		6
#define PB_SLICEON		7
#define PB_PIESLICE1	8
#define PB_PIESLICE2	9
#define PB_GENUVS		10

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
#define CREATEDESC_LENGH 1


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
		IDC_RADIUS1,IDC_RADSPINNER1,
		MIN_RADIUS,MAX_RADIUS,
		SPIN_AUTOSCALE),
	
	// Radius 2
	ParamUIDesc(
		PB_TI_RADIUS2,
		EDITTYPE_UNIVERSE,
		IDC_RADIUS2,IDC_RADSPINNER2,
		MIN_RADIUS,MAX_RADIUS,
		SPIN_AUTOSCALE),

	// Height 
	ParamUIDesc(
		PB_TI_HEIGHT,
		EDITTYPE_UNIVERSE,
		IDC_LENGTH,IDC_LENSPINNER,
		MIN_HEIGHT,MAX_HEIGHT,		
		SPIN_AUTOSCALE)	
	};
#define TYPEINDESC_LENGH 4


//
//
// Parameters

static ParamUIDesc descParam[] = {
	// Radius 1
	ParamUIDesc(
		PB_RADIUS1,
		EDITTYPE_UNIVERSE,
		IDC_RADIUS1,IDC_RADSPINNER1,
		MIN_RADIUS,MAX_RADIUS,
		SPIN_AUTOSCALE),	
	
	// Radius 2
	ParamUIDesc(
		PB_RADIUS2,
		EDITTYPE_UNIVERSE,
		IDC_RADIUS2,IDC_RADSPINNER2,
		MIN_RADIUS,MAX_RADIUS,
		SPIN_AUTOSCALE),	

	// Height 
	ParamUIDesc(
		PB_HEIGHT,
		EDITTYPE_UNIVERSE,
		IDC_LENGTH,IDC_LENSPINNER,
		MIN_HEIGHT,MAX_HEIGHT,		
		SPIN_AUTOSCALE),	
	
	// Circle Segments
	ParamUIDesc(
		PB_SEGMENTS,
		EDITTYPE_INT,
		IDC_SEGMENTS,IDC_SEGSPINNER,
		(float)MIN_SEGMENTS,(float)MAX_SEGMENTS,
		0.1f),
	
	// Cap Segments
	ParamUIDesc(
		PB_CAPSEGMENTS,
		EDITTYPE_INT,
		IDC_CAPSEGMENTS,IDC_CAPSEGSPINNER,
		(float)MIN_SEGMENTS,(float)MAX_SEGMENTS,
		0.1f),

	// Sides
	ParamUIDesc(
		PB_SIDES,
		EDITTYPE_INT,
		IDC_SIDES,IDC_SIDESPINNER,
		(float)MIN_SIDES,(float)MAX_SIDES,
		0.1f),
	
	// Smooth
	ParamUIDesc(PB_SMOOTH,TYPE_SINGLECHEKBOX,IDC_OBSMOOTH),
	
	// Slice on
	ParamUIDesc(PB_SLICEON,TYPE_SINGLECHEKBOX,IDC_SLICEON),	

	// Pie slice from
	ParamUIDesc(
		PB_PIESLICE1,
		EDITTYPE_FLOAT,
		IDC_PIESLICE1,IDC_PIESLICESPIN1,
		MIN_PIESLICE,MAX_PIESLICE,		
		0.5f,
		stdAngleDim),	

	// Pie slice to
	ParamUIDesc(
		PB_PIESLICE2,
		EDITTYPE_FLOAT,
		IDC_PIESLICE2,IDC_PIESLICESPIN2,
		MIN_PIESLICE,MAX_PIESLICE,		
		0.5f,
		stdAngleDim),		
	
	// Gen UVs
	ParamUIDesc(PB_GENUVS,TYPE_SINGLECHEKBOX,IDC_GENTEXTURE),
	};
#define PARAMDESC_LENGH 11

static ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 },
	{ TYPE_INT, NULL, TRUE, 3 },	
	{ TYPE_INT, NULL, TRUE, 4 },
	{ TYPE_INT, NULL, TRUE, 5 },
	{ TYPE_INT, NULL, TRUE, 6 },
	{ TYPE_FLOAT, NULL, TRUE, 7 },
	{ TYPE_FLOAT, NULL, TRUE, 8 } };

static ParamBlockDescID descVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 },
	{ TYPE_INT, NULL, TRUE, 3 },
	{ TYPE_INT, NULL, TRUE, 9 },
	{ TYPE_INT, NULL, TRUE, 4 },
	{ TYPE_INT, NULL, TRUE, 5 },
	{ TYPE_INT, NULL, TRUE, 6 },
	{ TYPE_FLOAT, NULL, TRUE, 7 },
	{ TYPE_FLOAT, NULL, TRUE, 8 } };

static ParamBlockDescID descVer2[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 },
	{ TYPE_INT, NULL, TRUE, 3 },
	{ TYPE_INT, NULL, TRUE, 9 },
	{ TYPE_INT, NULL, TRUE, 4 },
	{ TYPE_BOOL, NULL, TRUE, 5 },
	{ TYPE_INT, NULL, TRUE, 6 },
	{ TYPE_FLOAT, NULL, TRUE, 7 },
	{ TYPE_FLOAT, NULL, TRUE, 8 },
	{ TYPE_INT, NULL, FALSE, 10 } };
#define PBLOCK_LENGTH	11


// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,9,0),	
	ParamVersionDesc(descVer1,10,0),	
	};
#define NUM_OLDVERSIONS	2

// Current version
#define CURRENT_VERSION	2
static ParamVersionDesc curVersion(descVer2,PBLOCK_LENGTH,CURRENT_VERSION);



//--- TypeInDlgProc --------------------------------

class ConeTypeInDlgProc : public ParamMapUserDlgProc {
	public:
		ConeObject *ob;

		ConeTypeInDlgProc(ConeObject *o) {ob=o;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}

	};

BOOL ConeTypeInDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_TI_CREATE: {
					if (ob->crtRadius1==0.0) return TRUE;
					
					// We only want to set the value if the object is 
					// not in the scene.
					if (ob->TestAFlag(A_OBJ_CREATING)) {
						ob->pblock->SetValue(PB_RADIUS1,0,ob->crtRadius1);
						ob->pblock->SetValue(PB_RADIUS2,0,ob->crtRadius2);
						ob->pblock->SetValue(PB_HEIGHT,0,ob->crtHeight);
						}

					Matrix3 tm(1);
					tm.SetTrans(ob->crtPos);
					ob->suspendSnap = FALSE;
					ob->ip->NonMouseCreate(tm);
					// NOTE that calling NonMouseCreate will cause this
					// object to be deleted. DO NOT DO ANYTHING BUT RETURN.
					return TRUE;	
					}
				}
			break;	
		}
	return FALSE;
	}


//--- ParamDlgProc --------------------------------


class ConeParamDlgProc : public ParamMapUserDlgProc {
	public:
		ConeObject *so;
		HWND thishWnd;

		ConeParamDlgProc(ConeObject *s) {so=s;thishWnd=NULL;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void Update(TimeValue t);
		void DeleteThis() {delete this;}

		void TurnSpinner(HWND hWnd,int SpinNum,BOOL ison)
			{	ISpinnerControl *spin2 = GetISpinner(GetDlgItem(hWnd,SpinNum));
			if (ison) spin2->Enable();else spin2->Disable();
			ReleaseISpinner(spin2);

			};
	};
void ConeParamDlgProc::Update(TimeValue t)
{ if (!thishWnd) return;
  int ison;
  so->pblock->GetValue(PB_SLICEON,t,ison,FOREVER);
  TurnSpinner(thishWnd,IDC_PIESLICESPIN1,ison);
  TurnSpinner(thishWnd,IDC_PIESLICESPIN2,ison);

  EnableWindow(GetDlgItem(thishWnd,IDC_STATICFROM),ison);
  EnableWindow(GetDlgItem(thishWnd,IDC_STATICTO),ison);
}

BOOL ConeParamDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{ thishWnd=hWnd;
	switch (msg) {
		case WM_INITDIALOG:
			Update(t);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_SC_SLICEON: {
					int ison;
					so->pblock->GetValue(PB_SLICEON,t,ison,FOREVER);
					TurnSpinner(hWnd,IDC_PIESLICESPIN1,ison);
					TurnSpinner(hWnd,IDC_PIESLICESPIN2,ison);
					EnableWindow(GetDlgItem(hWnd,IDC_STATICFROM),ison);
					EnableWindow(GetDlgItem(hWnd,IDC_STATICTO),ison);
					return TRUE;	
					}
				}
			break;	
		}
	return FALSE;
	}



//--- Cone methods -------------------------------

ConeObject::ConeObject() 
	{
	MakeRefByID(FOREVER, 0, CreateParameterBlock(descVer2, PBLOCK_LENGTH, CURRENT_VERSION));
	assert(pblock);
		
	pblock->SetValue(PB_SMOOTH,0,dlgSmooth);
	pblock->SetValue(PB_SEGMENTS,0,dlgSegments);
	pblock->SetValue(PB_CAPSEGMENTS,0,dlgCapSegments);
	pblock->SetValue(PB_SIDES,0,dlgSides);
	pblock->SetValue(PB_HEIGHT,0,crtHeight);
	pblock->SetValue(PB_RADIUS1,0,crtRadius1);
	pblock->SetValue(PB_RADIUS2,0,crtRadius2);
	pblock->SetValue(PB_GENUVS,0,TRUE);
	}

IOResult ConeObject::Load(ILoad *iload) 
	{	
	iload->RegisterPostLoadCallback(
		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,0));
	return IO_OK;
	}


void ConeObject::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
	{
	SimpleObject::BeginEditParams(ip,flags,prev);
	this->ip = ip;

	if (pmapCreate && pmapParam) {
		
		// Left over from last Cone ceated
		pmapCreate->SetParamBlock(this);
		pmapTypeIn->SetParamBlock(this);
		pmapParam->SetParamBlock(pblock);
	} else {
		
		// Gotta make a new one.
		if (flags&BEGIN_EDIT_CREATE) {
			pmapCreate = CreateCPParamMap(
				descCreate,CREATEDESC_LENGH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_CONEPARAM1),
				GetString(IDS_RB_CREATIONMETHOD),
				0);

			pmapTypeIn = CreateCPParamMap(
				descTypeIn,TYPEINDESC_LENGH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_CONEPARAM3),
				GetString(IDS_RB_KEYBOARDENTRY),
				APPENDROLL_CLOSED);			
			}

		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_CONEPARAM2),
			GetString(IDS_RB_PARAMETERS),
			0);
		}

	if(pmapTypeIn) {
		// A callback for the type in.
		pmapTypeIn->SetUserDlgProc(new ConeTypeInDlgProc(this));
		}

	if(pmapParam) {
		// A callback for the type in.
		pmapParam->SetUserDlgProc(new ConeParamDlgProc(this));
		}

	}
		
void ConeObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
	{		
	SimpleObject::EndEditParams(ip,flags,next);
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
	pblock->GetValue(PB_SEGMENTS,ip->GetTime(),dlgSegments,FOREVER);
	pblock->GetValue(PB_CAPSEGMENTS,ip->GetTime(),dlgCapSegments,FOREVER);
	pblock->GetValue(PB_SMOOTH,ip->GetTime(),dlgSmooth,FOREVER);	
	}


// In cyl.cpp
extern void BuildCylinderMesh(Mesh &mesh,
		int segs, int smooth, int llsegs, int capsegs, int doPie,
		float radius1, float radius2, float height, float pie1, float pie2,
		int genUVs);

extern void BuildCylinderPoly (MNMesh & mesh,
		int segs, int smooth, int lsegs, int capsegs, int doPie,
		float radius1, float radius2, float height, float pie1, float pie2,
		int genUVs);


BOOL ConeObject::HasUVW() { 
	BOOL genUVs;
	Interval v;
	pblock->GetValue(PB_GENUVS, 0, genUVs, v);
	return genUVs; 
	}

void ConeObject::SetGenUVW(BOOL sw) {  
	if (sw==HasUVW()) return;
	pblock->SetValue(PB_GENUVS,0, sw);				
	}

Point3 ConeObject::GetSurfacePoint(
		TimeValue t, float u, float v,Interval &iv)
	{
	float radius1, radius2, height;
	pblock->GetValue(PB_RADIUS1,t,radius1,iv);
	pblock->GetValue(PB_RADIUS2,t,radius2,iv);
	pblock->GetValue(PB_HEIGHT,t,height,iv);
	Point3 p;
	float sn = -(float)cos(u*TWOPI);
	float cs = (float)sin(u*TWOPI);
	p.x = (1.0f-v)*radius1*cs + v*radius2*cs;
	p.y = (1.0f-v)*radius1*sn + v*radius2*sn;
	p.z = height * v;
	return p;
	}

Object *ConeObject::BuildPoly (TimeValue t) {
	int segs, smooth, llsegs, capsegs;
	float radius1, radius2, height, pie1, pie2;
	int doPie, genUVs;

	// Start the validity interval at forever and widdle it down.
	Interval tvalid = FOREVER;
	
	pblock->GetValue(PB_SIDES,t,segs,tvalid);
	pblock->GetValue(PB_SEGMENTS,t,llsegs,tvalid);
	pblock->GetValue(PB_CAPSEGMENTS,t,capsegs,tvalid);
	pblock->GetValue(PB_SMOOTH,t,smooth,tvalid);	
	pblock->GetValue(PB_SLICEON,t,doPie,tvalid);	
	pblock->GetValue(PB_GENUVS,t,genUVs,tvalid);
	Interval gvalid = tvalid;
	pblock->GetValue(PB_RADIUS1,t,radius1,gvalid);
	pblock->GetValue(PB_RADIUS2,t,radius2,gvalid);
	pblock->GetValue(PB_HEIGHT,t,height,gvalid);
	pblock->GetValue(PB_PIESLICE1,t,pie1,gvalid);
	pblock->GetValue(PB_PIESLICE2,t,pie2,gvalid);	
	LimitValue(radius1, MIN_RADIUS, MAX_RADIUS);
	LimitValue(radius2, MIN_RADIUS, MAX_RADIUS);
	LimitValue(height, MIN_HEIGHT, MAX_HEIGHT);
	LimitValue(llsegs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(segs, MIN_SIDES, MAX_SIDES);
	LimitValue(capsegs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(smooth, 0, 1);	
	
	PolyObject *pobj = new PolyObject();
	MNMesh & mesh = pobj->GetMesh();
	BuildCylinderPoly (mesh, segs, smooth, llsegs, capsegs, doPie,
		radius1, radius2, height, pie1, pie2, genUVs);
	pobj->SetChannelValidity(TOPO_CHAN_NUM,tvalid);
	pobj->SetChannelValidity(GEOM_CHAN_NUM,gvalid);
	return pobj;
}

void ConeObject::BuildMesh(TimeValue t)
	{	
	int segs,llsegs,smooth,capsegs;
	float radius1,radius2,height,pie1, pie2;
	int doPie, genUVs;

	// Start the validity interval at forever and widdle it down.
	ivalid = FOREVER;
	
	pblock->GetValue(PB_SIDES,t,segs,ivalid);
	pblock->GetValue(PB_SEGMENTS,t,llsegs,ivalid);
	pblock->GetValue(PB_CAPSEGMENTS,t,capsegs,ivalid);
	pblock->GetValue(PB_RADIUS1,t,radius1,ivalid);
	pblock->GetValue(PB_RADIUS2,t,radius2,ivalid);
	pblock->GetValue(PB_HEIGHT,t,height,ivalid);
	pblock->GetValue(PB_SMOOTH,t,smooth,ivalid);	
	pblock->GetValue(PB_PIESLICE1,t,pie1,ivalid);
	pblock->GetValue(PB_PIESLICE2,t,pie2,ivalid);	
	pblock->GetValue(PB_SLICEON,t,doPie,ivalid);	
	pblock->GetValue(PB_GENUVS,t,genUVs,ivalid);
	LimitValue(radius1, MIN_RADIUS, MAX_RADIUS);
	LimitValue(radius2, MIN_RADIUS, MAX_RADIUS);
	LimitValue(height, MIN_HEIGHT, MAX_HEIGHT);
	LimitValue(llsegs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(segs, MIN_SIDES, MAX_SIDES);
	LimitValue(capsegs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(smooth, 0, 1);	
	
	BuildCylinderMesh(mesh,
		segs, smooth, llsegs, capsegs, doPie,
		radius1, radius2, height, pie1, pie2, genUVs);
	}

// In cyl,cpp
extern void BuildCylinderPatch(
		TimeValue t, PatchMesh &patch, 
		float radius1, float radius2, float height, int genUVs);




#ifndef NO_NURBS

Object *
BuildNURBSCone(float radius1, float radius2, float height, BOOL sliceon, float pie1, float pie2, BOOL genUVs)
{
	BOOL flip = FALSE;

    if (radius1 == 0.0f)
        radius1 = 0.001f;
    if (radius2 == 0.0f)
        radius2 = 0.001f;

	if (height < 0.0f)
		flip = !flip;

	NURBSSet nset;

	Point3 origin(0,0,0);
	Point3 symAxis(0,0,1);
	Point3 refAxis(0,1,0);

	float startAngle = 0.0f;
	float endAngle = TWOPI;
	if (sliceon && pie1 != pie2) {
		float sweep = TWOPI - (pie2-pie1);
		if (sweep > TWOPI) sweep -= TWOPI;
		refAxis = Point3(Point3(1,0,0) * RotateZMatrix(pie2));
		endAngle = sweep;
	}


	// first the main surface
	NURBSCVSurface *surf = new NURBSCVSurface();
	nset.AppendObject(surf);
	surf->SetGenerateUVs(genUVs);

	surf->SetTextureUVs(0, 0, Point2(0.0f, 0.0f));
	surf->SetTextureUVs(0, 1, Point2(0.0f, 1.0f));
	surf->SetTextureUVs(0, 2, Point2(1.0f, 0.0f));
	surf->SetTextureUVs(0, 3, Point2(1.0f, 1.0f));

	surf->FlipNormals(!flip);
	surf->Renderable(TRUE);
	char bname[80];
	char sname[80];
	strcpy(bname, GetString(IDS_RB_CONE));
	sprintf(sname, "%s%s", bname, GetString(IDS_CT_SURF));

	if (sliceon && pie1 != pie2) {
		surf->SetName(sname);
		GenNURBSConeSurface(radius1, radius2, height, origin, symAxis, refAxis,
						startAngle, endAngle, TRUE, *surf);

#define F(s1, s2, s1r, s1c, s2r, s2c) \
		fuse.mSurf1 = (s1); \
		fuse.mSurf2 = (s2); \
		fuse.mRow1 = (s1r); \
		fuse.mCol1 = (s1c); \
		fuse.mRow2 = (s2r); \
		fuse.mCol2 = (s2c); \
		nset.mSurfFuse.Append(1, &fuse);

		NURBSFuseSurfaceCV fuse;

		NURBSCVSurface *s0 = (NURBSCVSurface*)nset.GetNURBSObject(0);

		Point3 cen;

		// next the two caps
		for (int c = 0; c < 2; c++) {
			if (c == 0) {
				cen = Point3(0,0,0);
			} else {
				cen = Point3(0.0f, 0.0f, height);
			}
			NURBSCVSurface *s = new NURBSCVSurface();
			nset.AppendObject(s);
			// we'll be cubic in on direction and match the sphere in the other
			s->SetUOrder(4);
			s->SetNumUKnots(8);
			for (int i = 0; i < 4; i++) {
				s->SetUKnot(i, 0.0);
				s->SetUKnot(i+4, 1.0);
			}

			s->SetVOrder(s0->GetVOrder());
			s->SetNumVKnots(s0->GetNumVKnots());
			for (i = 0; i < s->GetNumVKnots(); i++)
				s->SetVKnot(i, s0->GetVKnot(i));

			int numU, numV;
			s0->GetNumCVs(numU, numV);
			s->SetNumCVs(4, numV);

			for (int v = 0; v < numV; v++) {
				Point3 edge;
				if (c == 0)
					edge = s0->GetCV(0, v)->GetPosition(0);
				else
					edge = s0->GetCV(numU-1, v)->GetPosition(0);
				double w = s0->GetCV(0, v)->GetWeight(0);
				for (int u = 0; u < 4; u++) {
					NURBSControlVertex ncv;
					ncv.SetPosition(0, cen + ((edge - cen)*((float)u/3.0f)));
					ncv.SetWeight(0, w);
					s->SetCV(u, v, ncv);
				}
			}
			s->SetGenerateUVs(genUVs);

			s->SetTextureUVs(0, 0, Point2(1.0f, 1.0f));
			s->SetTextureUVs(0, 1, Point2(0.0f, 1.0f));
			s->SetTextureUVs(0, 2, Point2(1.0f, 0.0f));
			s->SetTextureUVs(0, 3, Point2(0.0f, 0.0f));

			if (c == 0)
				s->FlipNormals(!flip);
			else
				s->FlipNormals(flip);
			s->Renderable(TRUE);
			sprintf(sname, "%s%s%02d", bname, GetString(IDS_CT_CAP), c+1);
			s->SetName(sname);
		}

		NURBSCVSurface *s1 = (NURBSCVSurface *)nset.GetNURBSObject(1);
		NURBSCVSurface *s2 = (NURBSCVSurface *)nset.GetNURBSObject(2);

		// next the two pie slices
		for (c = 0; c < 2; c++) {
			if (c == 0)
				cen = Point3(0,0,0);
			else
				cen = Point3(0.0f, 0.0f, height);
			NURBSCVSurface *s = new NURBSCVSurface();
			nset.AppendObject(s);
			// we'll match the cylinder in on dimention and the caps in the other.
			s->SetUOrder(s0->GetUOrder());
			int numKnots = s0->GetNumUKnots();
			s->SetNumUKnots(numKnots);
			for (int i = 0; i < numKnots; i++)
				s->SetUKnot(i, s0->GetUKnot(i));

			s->SetVOrder(s1->GetUOrder());
			numKnots = s1->GetNumUKnots();
			s->SetNumVKnots(numKnots);
			for (i = 0; i < numKnots; i++)
				s->SetVKnot(i, s1->GetUKnot(i));

			int s0u, s0v, s1u, s1v, s2u, s2v;
			s0->GetNumCVs(s0u, s0v);
			s1->GetNumCVs(s1u, s1v);
			s2->GetNumCVs(s2u, s2v);
			int uNum = s0u, vNum = s1u;
			s->SetNumCVs(uNum, vNum);
			for (int v = 0; v < vNum; v++) {
				for (int u = 0; u < uNum; u++) {
					// we get get the ends from the caps and the edge from the main sheet
					if (u == 0) {  // bottom
						if (c == 0) {
							s->SetCV(u, v, *s1->GetCV(v, 0));
							F(1, 3, v, 0, u, v);
						} else {
							s->SetCV(u, v, *s1->GetCV(v, s1v-1));
							F(1, 4, v, s1v-1, u, v);
						}
					} else if (u == uNum-1) { // top
						if (c == 0) {
							s->SetCV(u, v, *s2->GetCV(v, 0));
							F(2, 3, v, 0, u, v);
						} else {
							s->SetCV(u, v, *s2->GetCV(v, s2v-1));
							F(2, 4, v, s2v-1, u, v);
						}
					} else { // middle
						if (v == vNum-1) { // outer edge
							if (c == 0) {
								s->SetCV(u, v, *s0->GetCV(u, 0));
								F(0, 3, u, 0, u, v);
							} else {
								s->SetCV(u, v, *s0->GetCV(u, s0v-1));
								F(0, 4, u, s0v-1, u, v);
							}
						} else { // inside
							float angle;
							if (c == 0) angle = pie2;
							else angle = pie1;
							float hrad1 = radius1 * (float)v / (float)(vNum-1);
							float hrad2 = radius2 * (float)v / (float)(vNum-1);
							float rad = hrad1 + ((hrad2 - hrad1) * (float)u / (float)(uNum-1));
							NURBSControlVertex ncv;
							ncv.SetPosition(0, Point3(rad, 0.0f, height * (float)u / (float)(uNum-1)) * RotateZMatrix(angle));
							ncv.SetWeight(0, 1.0f);
							s->SetCV(u, v, ncv);
						}
					}
				}
			}
			s->SetGenerateUVs(genUVs);

			s->SetTextureUVs(0, 0, Point2(0.0f, 0.0f));
			s->SetTextureUVs(0, 1, Point2(0.0f, 1.0f));
			s->SetTextureUVs(0, 2, Point2(1.0f, 0.0f));
			s->SetTextureUVs(0, 3, Point2(1.0f, 1.0f));

			if (c == 0)
				s->FlipNormals(!flip);
			else
				s->FlipNormals(flip);
			s->Renderable(TRUE);
			sprintf(sname, "%s%s%02d", bname, GetString(IDS_CT_SLICE), c+1);
			s->SetName(sname);
		}

		NURBSCVSurface *s3 = (NURBSCVSurface *)nset.GetNURBSObject(3);
		NURBSCVSurface *s4 = (NURBSCVSurface *)nset.GetNURBSObject(4);

		// now fuse up the rest
		// Fuse the edges
		for (int v = 0; v < s0->GetNumVCVs(); v++) {
			F(0, 1, 0, v, s1->GetNumUCVs()-1, v);
			F(0, 2, s0->GetNumUCVs()-1, v, s2->GetNumUCVs()-1, v);
		}

		// Fuse the cap centers
		for (v = 1; v < s1->GetNumVCVs(); v++) {
			F(1, 1, 0, 0, 0, v);
			F(2, 2, 0, 0, 0, v);
		}

		// Fuse the core
		for (int u = 0; u < s3->GetNumUCVs(); u++) {
			F(3, 4, u, 0, u, 0);
		}
	} else {
		GenNURBSConeSurface(radius1, radius2, height, origin, symAxis, refAxis,
					startAngle, endAngle, FALSE, *surf);

		// now create caps on the ends
		if (radius1 != 0.0) {
			NURBSCapSurface *cap0 = new NURBSCapSurface();
			nset.AppendObject(cap0);
			cap0->SetGenerateUVs(genUVs);
			cap0->SetParent(0);
			cap0->SetEdge(0);
			cap0->FlipNormals(!flip);
			cap0->Renderable(TRUE);
			char sname[80];
			sprintf(sname, "%s%s%02d", bname, GetString(IDS_CT_CAP), 0);
			cap0->SetName(sname);
		}

		if (radius2 != 0.0) {
			NURBSCapSurface *cap1 = new NURBSCapSurface();
			nset.AppendObject(cap1);
			cap1->SetGenerateUVs(genUVs);
			cap1->SetParent(0);
			cap1->SetEdge(1);
			cap1->FlipNormals(flip);
			cap1->Renderable(TRUE);
			sprintf(sname, "%s%s%02d", bname, GetString(IDS_CT_CAP), 1);
			cap1->SetName(sname);
		}
	}

	Matrix3 mat;
	mat.IdentityMatrix();
	Object *ob = CreateNURBSObject(NULL, &nset, mat);
	return ob;
}

#endif



Object* ConeObject::ConvertToType(TimeValue t, Class_ID obtype)
	{
#ifndef NO_PATCHES
	if (obtype == patchObjectClassID) {
		Interval valid = FOREVER;
		float radius1, radius2, height;
		int genUVs;
		pblock->GetValue(PB_RADIUS1,t,radius1,valid);
		pblock->GetValue(PB_RADIUS2,t,radius2,valid);
		pblock->GetValue(PB_HEIGHT,t,height,valid);	
		pblock->GetValue(PB_GENUVS,t,genUVs,valid);
		if (radius1 < 0.0f) radius1 = 0.0f;
		if (radius2 < 0.0f) radius2 = 0.0f;
		PatchObject *ob = new PatchObject();
		BuildCylinderPatch(t,ob->patch,radius1,radius2,height,genUVs);
		ob->SetChannelValidity(TOPO_CHAN_NUM,valid);
		ob->SetChannelValidity(GEOM_CHAN_NUM,valid);
		ob->UnlockObject();
		return ob;
	} 
#endif

	if (obtype == polyObjectClassID) {
		Object *ob = BuildPoly (t);
		ob->UnlockObject ();
		return ob;
	}

#ifndef NO_NURBS
    if (obtype == EDITABLE_SURF_CLASS_ID) {
		Interval valid = FOREVER;
		float radius1, radius2, height, pie1, pie2;
		int sliceon, genUVs;
		pblock->GetValue(PB_RADIUS1,t,radius1,valid);
		pblock->GetValue(PB_RADIUS2,t,radius2,valid);
		pblock->GetValue(PB_HEIGHT,t,height,valid);	
		pblock->GetValue(PB_PIESLICE1,t,pie1,valid);	
		pblock->GetValue(PB_PIESLICE2,t,pie2,valid);	
		pblock->GetValue(PB_SLICEON,t,sliceon,valid);
		pblock->GetValue(PB_GENUVS,t,genUVs,valid);
		if (radius1 < 0.0f) radius1 = 0.0f;
		if (radius2 < 0.0f) radius2 = 0.0f;
		Object *ob = BuildNURBSCone(radius1, radius2, height, sliceon, pie1, pie2, genUVs);
		ob->SetChannelValidity(TOPO_CHAN_NUM,valid);
		ob->SetChannelValidity(GEOM_CHAN_NUM,valid);
		ob->UnlockObject();
		return ob;
		
	}
#endif

#ifdef DESIGN_VER
	if (obtype == GENERIC_AMSOLID_CLASS_ID)
	{
		Interval valid = FOREVER;
		float radius1, radius2, height, pie1, pie2;
		int sliceon, genUVs, sides;
		pblock->GetValue(PB_RADIUS1,t,radius1,valid);
		pblock->GetValue(PB_RADIUS2,t,radius2,valid);
		pblock->GetValue(PB_HEIGHT,t,height,valid);	
		pblock->GetValue(PB_PIESLICE1,t,pie1,valid);	
		pblock->GetValue(PB_PIESLICE2,t,pie2,valid);	
		pblock->GetValue(PB_SLICEON,t,sliceon,valid);
		pblock->GetValue(PB_GENUVS,t,genUVs,valid);
		pblock->GetValue(PB_SIDES,t,sides,valid);
		int smooth;
		pblock->GetValue(PB_SMOOTH,t,smooth,valid);
		if (radius1 < 0.0f) radius1 = 0.0f;
		if (radius2 < 0.0f) radius2 = 0.0f;
		Object* solid = (Object*)CreateInstance(GEOMOBJECT_CLASS_ID, GENERIC_AMSOLID_CLASS_ID);
		assert(solid);
		if(solid)
		{
			IGeomImp* cacheptr = (IGeomImp*)(solid->GetInterface(I_GEOMIMP));
			assert(cacheptr);
			if(cacheptr)
			{
				bool res = cacheptr->createCone(height, radius1, radius2, sides, smooth);
				solid->ReleaseInterface(I_GEOMIMP, cacheptr);
				if(res)
					return solid;
				else 
				{
					solid->DeleteMe();
				}
			}
		}
		return NULL;
	}
#endif
	return SimpleObject::ConvertToType(t,obtype);
}

int ConeObject::CanConvertToType(Class_ID obtype)
	{
	if(obtype==defObjectClassID || obtype==mapObjectClassID || obtype==triObjectClassID)
		return 1;

#ifndef NO_PATCHES
    if(obtype == patchObjectClassID) return 1;
#endif
#ifndef NO_NURBS
    if(obtype == EDITABLE_SURF_CLASS_ID) return 1;
#endif
#ifdef DESIGN_VER
	if(obtype == GENERIC_AMSOLID_CLASS_ID) return 1;
#endif
	if (obtype == polyObjectClassID) return 1;

    return SimpleObject::CanConvertToType(obtype);
	}

void ConeObject::GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist)
{
    Object::GetCollapseTypes(clist, nlist);
#ifndef NO_NURBS
    Class_ID id = EDITABLE_SURF_CLASS_ID;
    TSTR *name = new TSTR(GetString(IDS_SM_NURBS_SURFACE));
    clist.Append(1,&id);
    nlist.Append(1,&name);
#endif
}


class ConeObjCreateCallBack: public CreateMouseCallBack {
	ConeObject *ob;	
	Point3 p[4];
	IPoint2 sp0,sp1,sp2,sp3;	
	float r1;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		void SetObj(ConeObject *obj) { ob = obj; }
	};

int ConeObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
	float r;

	#ifdef _OSNAP
	if (msg == MOUSE_FREEMOVE)
	{
		#ifdef _3D_CREATE
			vpt->SnapPreview(m,m,NULL, SNAP_IN_3D);
		#else
			vpt->SnapPreview(m,m,NULL, SNAP_IN_PLANE);
		#endif
	}
	#endif

	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {
			case 0:
				ob->suspendSnap = TRUE;				
				sp0 = m;				
				#ifdef _3D_CREATE	
					p[0] = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				#else	
					p[0] = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				#endif
				mat.SetTrans(p[0]); // Set Node's transform				
				ob->pblock->SetValue(PB_RADIUS1,0,0.01f);
				ob->pblock->SetValue(PB_RADIUS2,0,0.01f);
				ob->pblock->SetValue(PB_HEIGHT,0,0.01f);
				break;
			case 1: 
				mat.IdentityMatrix();
				//mat.PreRotateZ(HALFPI);
				sp1 = m;							   
				#ifdef _3D_CREATE	
					p[1] = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				#else	
					p[1] = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				#endif
				if (ob->dlgCreateMeth) {	
					// radius	
					r = Length(p[1]-p[0]);
					mat.SetTrans(p[0]);
				} else {
					// diameter
					Point3 center = (p[0]+p[1])/float(2);
					r = Length(center-p[0]);
					mat.SetTrans(center);  // Modify Node's transform
					}
				
				ob->pblock->SetValue(PB_RADIUS1,0,r);
				ob->pblock->SetValue(PB_RADIUS2,0,r+1.0f);
				ob->pmapParam->Invalidate();
				r1 = r;

				if (flags&MOUSE_CTRL) {
					float ang = (float)atan2(p[1].y-p[0].y,p[1].x-p[0].x);
					mat.PreRotateZ(ob->ip->SnapAngle(ang));
					}

				if (msg==MOUSE_POINT) {
					if (Length(m-sp0)<3 || 
						Length(p[1]-p[0]) < 0.1f) {						
						return CREATE_ABORT;
						}
					}
				break;
			
			case 2: {
				sp2 = m;
#ifdef _OSNAP
				float h = vpt->SnapLength(vpt->GetCPDisp(p[1],Point3(0,0,1),sp1,m,TRUE));
#else
				float h = vpt->SnapLength(vpt->GetCPDisp(p[1],Point3(0,0,1),sp1,m));
#endif
				ob->pblock->SetValue(PB_HEIGHT,0,h);
				ob->pmapParam->Invalidate();				
				}
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

static ConeObjCreateCallBack coneCreateCB;

CreateMouseCallBack* ConeObject::GetCreateMouseCallBack() 
	{
	coneCreateCB.SetObj(this);
	return(&coneCreateCB);
	}

BOOL ConeObject::OKtoDisplay(TimeValue t) 
	{
	float radius1, radius2;
	pblock->GetValue(PB_RADIUS1,t,radius1,FOREVER);
	pblock->GetValue(PB_RADIUS2,t,radius2,FOREVER);
	if (radius1==0.0f && radius2==0.0f) return FALSE;
	else return TRUE;
	}


// From ParamArray
BOOL ConeObject::SetValue(int i, TimeValue t, int v) 
	{
	switch (i) {
		case PB_CREATEMETHOD: dlgCreateMeth = v; break;
		}		
	return TRUE;
	}

BOOL ConeObject::SetValue(int i, TimeValue t, float v)
	{
	switch (i) {				
		case PB_TI_RADIUS1: crtRadius1 = v; break;
		case PB_TI_RADIUS2: crtRadius2 = v; break;
		case PB_TI_HEIGHT: crtHeight = v; break;
		}	
	return TRUE;
	}

BOOL ConeObject::SetValue(int i, TimeValue t, Point3 &v) 
	{
	switch (i) {
		case PB_TI_POS: crtPos = v; break;
		}		
	return TRUE;
	}

BOOL ConeObject::GetValue(int i, TimeValue t, int &v, Interval &ivalid) 
	{
	switch (i) {
		case PB_CREATEMETHOD: v = dlgCreateMeth; break;
		}
	return TRUE;
	}

BOOL ConeObject::GetValue(int i, TimeValue t, float &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_RADIUS1: v = crtRadius1; break;
		case PB_TI_RADIUS2: v = crtRadius2; break;
		case PB_TI_HEIGHT: v = crtHeight; break;
		}
	return TRUE;
	}

BOOL ConeObject::GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_POS: v = crtPos; break;		
		}
	return TRUE;
	}


void ConeObject::InvalidateUI() 
	{
	if (pmapParam) pmapParam->Invalidate();
	}

ParamDimension *ConeObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_RADIUS1:
		case PB_RADIUS2: 		return stdWorldDim;
		case PB_HEIGHT: 		return stdWorldDim;
		case PB_SEGMENTS: 		return stdSegmentsDim;
		case PB_CAPSEGMENTS:	return stdSegmentsDim;
		case PB_SIDES: 			return stdSegmentsDim;
		case PB_SMOOTH: 		return stdNormalizedDim;
		case PB_SLICEON: 		return stdNormalizedDim;
		case PB_PIESLICE1: 		return stdAngleDim;
		case PB_PIESLICE2: 		return stdAngleDim;
		default: return defaultDim;
		}
	}

TSTR ConeObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_RADIUS1:	 	return GetString(IDS_RB_RADIUS1);
		case PB_RADIUS2: 		return GetString(IDS_RB_RADIUS2);
		case PB_HEIGHT: 		return GetString(IDS_RB_HEIGHT);
		case PB_SEGMENTS:		return GetString(IDS_RB_CIRCLESEGMENTS);
		case PB_CAPSEGMENTS:	return GetString(IDS_RB_CAPSEGMENTS);
		case PB_SIDES: 			return GetString(IDS_RB_SIDES);
		case PB_SMOOTH: 		return GetString(IDS_RB_SMOOTH);
		case PB_SLICEON: 		return GetString(IDS_RB_SLICEON);
		case PB_PIESLICE1: 		return GetString(IDS_RB_SLICEFROM);
		case PB_PIESLICE2: 		return GetString(IDS_RB_SLICETO);
		//case PB_GENUVS:			return GetString(IDS_RB_GENTEXCOORDS);
		default: return TSTR(_T(""));
		}
	}

RefTargetHandle ConeObject::Clone(RemapDir& remap) 
	{
	ConeObject* newob = new ConeObject();	
	newob->ReplaceReference(0,pblock->Clone(remap));	
	newob->ivalid.SetEmpty();	
	BaseClone(this, newob, remap);
	return(newob);
	}

#endif // NO_OBJECT_STANDARD_PRIMITIVES

