/**********************************************************************
 *<
	FILE: cyl.cpp

	DESCRIPTION:  Cylinder object, Revised implementation

	CREATED BY: Rolf Berteig

	HISTORY: created November 11 1994

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "prim.h"

#include "iparamm.h"
#include "Simpobj.h"
#include "surf_api.h"
#include "PolyObj.h"

class CylinderObject : public GenCylinder, public IParamArray {
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
		static float crtRadius;
		static float crtHeight;
		
		CylinderObject();		

		// From Object
		int CanConvertToType(Class_ID obtype);
		Object* ConvertToType(TimeValue t, Class_ID obtype);
		void GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist);
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);
		BOOL IsParamSurface() {return TRUE;}
		Point3 GetSurfacePoint(TimeValue t, float u, float v,Interval &iv);
		
		// From BaseObject
		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);		
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		TCHAR *GetObjectName() { return GetString(IDS_RB_CYLINDER); }
				
		// Animatable methods		
		void DeleteThis() { delete this; }
		Class_ID ClassID() { return Class_ID( CYLINDER_CLASS_ID,0); }  		
				
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
		Object *BuildPoly (TimeValue t);
		BOOL OKtoDisplay(TimeValue t);
		void InvalidateUI();
		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);		

		// From GenCylinder
		void SetParams(float rad, float height, int segs, int sides, int capsegs=1, BOOL smooth=TRUE, 
			BOOL genUV=TRUE, BOOL sliceOn= FALSE, float slice1 = 0.0f, float slice2 = 0.0f);
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

#define DEF_SEGMENTS 	5	// 1
#define DEF_SIDES		18	// 24

#define DEF_RADIUS		float(0.0)
#define DEF_HEIGHT		float(0.01)

#define SMOOTH_ON		1
#define SMOOTH_OFF		0



//--- ClassDescriptor and class vars ---------------------------------

class CylClassDesc:public ClassDesc {
	public:
// xavier robitaille | 03.02.15 | private boxes, spheres and cylinders 
#ifndef NO_OBJECT_STANDARD_PRIMITIVES
	int 			IsPublic() { return 1; }
#else
	int 			IsPublic() { return 0; }
#endif
	void *			Create(BOOL loading = FALSE) { return new CylinderObject; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_CYLINDER_CLASS); }
	SClass_ID		SuperClassID() { return GEOMOBJECT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(CYLINDER_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_PRIMITIVES);  }
	void			ResetClassParams(BOOL fileReset);
	};

static CylClassDesc cylDesc;

ClassDesc* GetCylinderDesc() { return &cylDesc; }

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// class variables for Cylinder class.
IObjParam *CylinderObject::ip         = NULL;
int CylinderObject::dlgSegments       = DEF_SEGMENTS;
int CylinderObject::dlgCapSegments    = 1;
int CylinderObject::dlgSides          = DEF_SIDES;
int CylinderObject::dlgCreateMeth     = 1; // create_radius
int CylinderObject::dlgSmooth         = SMOOTH_ON;
IParamMap *CylinderObject::pmapCreate = NULL;
IParamMap *CylinderObject::pmapTypeIn = NULL;
IParamMap *CylinderObject::pmapParam  = NULL;
Point3 CylinderObject::crtPos         = Point3(0,0,0);
float CylinderObject::crtRadius       = 0.0f;
float CylinderObject::crtHeight       = 0.0f;

void CylClassDesc::ResetClassParams(BOOL fileReset)
	{
	CylinderObject::dlgSegments     = DEF_SEGMENTS;
	CylinderObject::dlgCapSegments  = 1;
	CylinderObject::dlgSides        = DEF_SIDES;
	CylinderObject::dlgCreateMeth   = 1; // create_radius
	CylinderObject::dlgSmooth       = SMOOTH_ON;
	CylinderObject::crtRadius       = 0.0f;
	CylinderObject::crtHeight       = 0.0f;
	CylinderObject::crtPos          = Point3(0,0,0);
	}

//--- Parameter map/block descriptors -------------------------------

// Parameter map indices
#define PB_RADIUS		0
#define PB_HEIGHT		1
#define PB_SEGMENTS		2
#define PB_CAPSEGMENTS	3
#define PB_SIDES		4
#define PB_SMOOTH		5
#define PB_SLICEON		6
#define PB_PIESLICE1	7
#define PB_PIESLICE2	8
#define PB_GENUVS		9

// Non-parameter block indices
#define PB_CREATEMETHOD		0
#define PB_TI_POS			1
#define PB_TI_RADIUS		2
#define PB_TI_HEIGHT		3


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
	
	// Radius
	ParamUIDesc(
		PB_TI_RADIUS,
		EDITTYPE_UNIVERSE,
		IDC_RADIUS,IDC_RADSPINNER,
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
#define TYPEINDESC_LENGH 3


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
		IDC_CAPSEGMENTS,IDC_CAPSEGSPIN,
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
#define PARAMDESC_LENGH 10


static ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, TRUE, 2 },
	{ TYPE_INT, NULL, TRUE, 3 },
	{ TYPE_INT, NULL, TRUE, 4 } };

static ParamBlockDescID descVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, TRUE, 2 },	
	{ TYPE_INT, NULL, TRUE, 3 },
	{ TYPE_INT, NULL, TRUE, 4 },
	{ TYPE_INT, NULL, TRUE, 5 },
	{ TYPE_FLOAT, NULL, TRUE, 6 },
	{ TYPE_FLOAT, NULL, TRUE, 7 } };

static ParamBlockDescID descVer2[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, TRUE, 2 },
	{ TYPE_INT, NULL, TRUE, 8 },
	{ TYPE_INT, NULL, TRUE, 3 },
	{ TYPE_INT, NULL, TRUE, 4 },
	{ TYPE_INT, NULL, TRUE, 5 },
	{ TYPE_FLOAT, NULL, TRUE, 6 },
	{ TYPE_FLOAT, NULL, TRUE, 7 } };

static ParamBlockDescID descVer3[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, TRUE, 2 },
	{ TYPE_INT, NULL, TRUE, 8 },
	{ TYPE_INT, NULL, TRUE, 3 },
	{ TYPE_BOOL, NULL, TRUE, 4 },
	{ TYPE_INT, NULL, TRUE, 5 },
	{ TYPE_FLOAT, NULL, TRUE, 6 },
	{ TYPE_FLOAT, NULL, TRUE, 7 },
	{ TYPE_INT, NULL, FALSE, 9 } };

#define PBLOCK_LENGTH	10

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,5,0),
	ParamVersionDesc(descVer1,8,1),
	ParamVersionDesc(descVer2,9,2)
	};
#define NUM_OLDVERSIONS	3

// Current version
#define CURRENT_VERSION	3
static ParamVersionDesc curVersion(descVer3,PBLOCK_LENGTH,CURRENT_VERSION);



//--- TypeInDlgProc --------------------------------

class CylTypeInDlgProc : public ParamMapUserDlgProc {
	public:
		CylinderObject *ob;

		CylTypeInDlgProc(CylinderObject *o) {ob=o;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

BOOL CylTypeInDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_TI_CREATE: {
					if (ob->crtRadius==0.0) return TRUE;
					
					// We only want to set the value if the object is 
					// not in the scene.
					if (ob->TestAFlag(A_OBJ_CREATING)) {
						ob->pblock->SetValue(PB_RADIUS,0,ob->crtRadius);
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



class BaseParamDlgProc : public ParamMapUserDlgProc {
	public:
		CylinderObject *so;
		HWND thishWnd;

		BaseParamDlgProc(CylinderObject *s) {so=s;thishWnd=NULL;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void Update(TimeValue t);
		void DeleteThis() {delete this;}

		void TurnSpinner(HWND hWnd,int SpinNum,BOOL ison)
			{	ISpinnerControl *spin2 = GetISpinner(GetDlgItem(hWnd,SpinNum));
			if (ison) spin2->Enable();else spin2->Disable();
			ReleaseISpinner(spin2);

			};

	};
void BaseParamDlgProc::Update(TimeValue t)
{ if (!thishWnd) return;
  int ison;
  so->pblock->GetValue(PB_SLICEON,t,ison,FOREVER);
  TurnSpinner(thishWnd,IDC_PIESLICESPIN1,ison);
  TurnSpinner(thishWnd,IDC_PIESLICESPIN2,ison);

  EnableWindow(GetDlgItem(thishWnd,IDC_STATICFROM),ison);
  EnableWindow(GetDlgItem(thishWnd,IDC_STATICTO),ison);
}

BOOL BaseParamDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{ thishWnd=hWnd;
	switch (msg) {
		case WM_INITDIALOG:
			Update(t);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_SLICEON: {
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



//--- Cylinder methods -------------------------------

CylinderObject::CylinderObject() 
	{
	MakeRefByID(FOREVER, 0, CreateParameterBlock(descVer3, PBLOCK_LENGTH, CURRENT_VERSION));
	assert(pblock);
		
	pblock->SetValue(PB_SMOOTH,0,dlgSmooth);
	pblock->SetValue(PB_SEGMENTS,0,dlgSegments);
	pblock->SetValue(PB_CAPSEGMENTS,0,dlgCapSegments);
	pblock->SetValue(PB_SIDES,0,dlgSides);
	pblock->SetValue(PB_HEIGHT,0,crtHeight);
	pblock->SetValue(PB_RADIUS,0,crtRadius);	
	pblock->SetValue(PB_GENUVS,0,TRUE);
	}

IOResult CylinderObject::Load(ILoad *iload) 
	{
	iload->RegisterPostLoadCallback(
		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,0));
	return IO_OK;
	}


void CylinderObject::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
	{
	SimpleObject::BeginEditParams(ip,flags,prev);
	this->ip = ip;

	if (pmapCreate && pmapParam) {
		
		// Left over from last cylinder ceated
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
				MAKEINTRESOURCE(IDD_CYLINDERPARAM1),
				GetString(IDS_RB_CREATIONMETHOD),
				0);

			pmapTypeIn = CreateCPParamMap(
				descTypeIn,TYPEINDESC_LENGH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_CYLINDERPARAM3),
				GetString(IDS_RB_KEYBOARDENTRY),
				APPENDROLL_CLOSED);			
			}

		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_CYLINDERPARAM2),
			GetString(IDS_RB_PARAMETERS),
			0);
		}

	if(pmapTypeIn) {
		// A callback for the type in.
		pmapTypeIn->SetUserDlgProc(new CylTypeInDlgProc(this));
		}

	if(pmapParam) {
		// A callback for the type in.
		pmapParam->SetUserDlgProc(new BaseParamDlgProc(this));
		}

	}
		
void CylinderObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
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

void CylinderObject::SetParams(float rad, float height, int segs, int sides, int capsegs, BOOL smooth, 
	BOOL genUV, BOOL sliceOn, float slice1, float slice2) {
	pblock->SetValue(PB_RADIUS,0,rad);
	pblock->SetValue(PB_HEIGHT,0,height);
	pblock->SetValue(PB_SEGMENTS,0,segs);
	pblock->SetValue(PB_SIDES,0,sides);
	pblock->SetValue(PB_SLICEON,0, sliceOn);
	pblock->SetValue(PB_PIESLICE1,0,slice1);
	pblock->SetValue(PB_PIESLICE2,0,slice2);
	pblock->SetValue(PB_GENUVS,0,genUV);
	}

Point3 CylinderObject::GetSurfacePoint(
		TimeValue t, float u, float v,Interval &iv)
	{
	float radius, height;
	pblock->GetValue(PB_RADIUS,t,radius,iv);
	pblock->GetValue(PB_HEIGHT,t,height,iv);
	Point3 p;
	p.x = (float)cos(u*TWOPI)*radius;
	p.y = (float)sin(u*TWOPI)*radius;
	p.z = height * v;
	return p;
	}

// Cone also uses this build function
void BuildCylinderPoly (MNMesh & mesh, int segs, int smooth, int lsegs,
						int capsegs, int doPie, float radius1, float radius2,
						float height, float pie1, float pie2, int genUVs) {
	Point3 p;
	int ix,jx;
	int nf=0,nv=0;//, lsegs;
	float delta,ang, u;	
	float totalPie, startAng = 0.0f;	

	if (doPie) doPie = 1;
	else doPie = 0; 

	//lsegs = llsegs-1 + 2*capsegs;

	// Make pie2 < pie1 and pie1-pie2 < TWOPI
	while (pie1 < pie2) pie1 += TWOPI;
	while (pie1 > pie2+TWOPI) pie1 -= TWOPI;
	if (pie1==pie2) totalPie = TWOPI;
	else totalPie = pie1-pie2;	

	if (doPie) {
		delta    = totalPie/(float)(segs-1);
		startAng = (height<0) ? pie1 : pie2;	// mjm - 2.16.99
	} else {
		delta = (float)2.0*PI/(float)segs;
	}

	if (height<0) delta = -delta;

	int nverts;
	int nfaces;
	if (doPie) {
		// Number of vertices is height segments times (segs "perimeter" verts + 1 "central" vert)
		nverts = (segs+1)*(lsegs+1);
		// Faces to fill these rows...
		nfaces = (segs+1)*lsegs;
		// Plus faces for the caps:
		if (capsegs>1) {
			nverts += 2*(capsegs-1)*segs;
			nfaces += 2*capsegs*(segs-1);
		} else {
			nfaces += 2;
		}
	} else {
		// Number of vertices is the segments times the height segments:
		nverts = segs*(lsegs+1);
		// Faces to fill these rows...
		nfaces = segs*lsegs;
		// Plus faces for the caps:
		if (capsegs>1) {
			nverts += 2*(capsegs-1)*segs + 2;	// two pole vertices.
			nfaces += 2*capsegs*segs;
		} else {
			nfaces += 2;
		}
	}
	mesh.setNumVerts(nverts);
	mesh.setNumFaces(nfaces);

	// Do sides first.
	nv=0;
	nf=0;
	int vv[4];
	int segss = segs+doPie;
	for (ix=0; ix<lsegs+1; ix++) {
		float   u = float(ix)/float(lsegs);
		float rad = (radius1*(1.0f-u) + radius2*u);
		p.z = height*((float)ix/float(lsegs));
		ang = startAng;
		for (jx=0; jx<segss; jx++) {
			if (jx==segs) {
				p.x=p.y=0.0f;
			} else {
				p.x = cosf(ang)*rad;
				p.y = sinf(ang)*rad;
			}
			mesh.v[nv].p = p;
			nv++;
			ang += delta;
		}
		if (ix) {
			for (jx=0; jx<segss; jx++) {
				vv[0] = (ix-1)*segss + jx;
				vv[1] = (ix-1)*segss + (jx+1)%segss;
				vv[2] = vv[1] + segss;
				vv[3] = vv[0] + segss;
				mesh.f[nf].MakePoly (4, vv);
				if ((jx==segs-1) && doPie) {
					mesh.f[nf].material = 4;
					mesh.f[nf].smGroup = (1<<2);
				} else {
					if (jx==segs) {
						mesh.f[nf].material = 3;
						mesh.f[nf].smGroup = (1<<1);
					} else {
						mesh.f[nf].material = 2;
						mesh.f[nf].smGroup = (1<<3);
					}
				}
				nf++;
			}
		}
	}

	// Caps:
	float rad;
	if (capsegs == 1) {
		mesh.f[nf].SetAlloc (segss);
		mesh.f[nf].deg = segss;
		for (jx=0; jx<segss; jx++) mesh.f[nf].vtx[jx] = segss-1-jx;
		mesh.f[nf].smGroup = 1;
		mesh.f[nf].material = 1;
		if (doPie) mesh.RetriangulateFace (nf);
		else mesh.BestConvexDiagonals (nf);
		nf++;
		mesh.f[nf].SetAlloc (segss);
		mesh.f[nf].deg = segss;
		for (jx=0,ix=segss*lsegs; jx<segss; jx++, ix++) mesh.f[nf].vtx[jx] = ix;
		mesh.f[nf].smGroup = 1;
		mesh.f[nf].material = 0;
		if (doPie) mesh.RetriangulateFace (nf);
		else mesh.BestConvexDiagonals (nf);
		nf++;
	} else {
		// Do Bottom Cap:
		int startv = nv;
		for (ix=1; ix<capsegs; ix++) {
			p.z = 0.0f;
			u   = float(capsegs-ix)/float(capsegs);
			ang = startAng;
			rad = radius1*u;
			for (jx = 0; jx<segs; jx++) {			
				p.x = (float)cos(ang)*rad;
				p.y = (float)sin(ang)*rad;	
				mesh.v[nv].p = p;
				nv++;
				ang += delta;
			}
			if (ix==1) {
				// First row of faces:
				for (jx=0; jx<segs-doPie; jx++) {
					vv[0] = (jx+1)%segs;
					vv[1] = jx;
					vv[2] = startv+jx;
					vv[3] = startv+(jx+1)%segs;
					mesh.f[nf].MakePoly (4, vv);
					mesh.f[nf].smGroup = 1;
					mesh.f[nf].material = 1;
					nf++;
				}
			} else {
				for (jx=0; jx<segs-doPie; jx++) {
					vv[0] = startv + segs*(ix-2) + (jx+1)%segs;
					vv[1] = startv + segs*(ix-2) + jx;
					vv[2] = vv[1] + segs;
					vv[3] = vv[0] + segs;
					mesh.f[nf].MakePoly (4, vv);
					mesh.f[nf].smGroup = 1;
					mesh.f[nf].material = 1;
					nf++;
				}
			}
		}
		if (!doPie) {
			mesh.v[nv].p = Point3(0,0,0);
			nv++;
		}
		// Last row of faces:
		for (jx=0; jx<segs-doPie; jx++) {
			vv[0] = startv + segs*(capsegs-2) + (jx+1)%segs;
			vv[1] = startv + segs*(capsegs-2) + jx;
			vv[2] = doPie ? segs : nv-1;
			mesh.f[nf].MakePoly (3, vv);
			mesh.f[nf].smGroup = 1;
			mesh.f[nf].material = 1;
			nf++;
		}

		// Do top cap:
		startv = nv;
		for (ix=1; ix<capsegs; ix++) {
			p.z = height;
			u   = float(capsegs-ix)/float(capsegs);
			ang = startAng;
			rad = radius2*u;
			for (jx = 0; jx<segs; jx++) {			
				p.x = (float)cos(ang)*rad;
				p.y = (float)sin(ang)*rad;	
				mesh.v[nv].p = p;
				nv++;
				ang += delta;
			}
			if (ix==1) {
				// First row of faces:
				for (jx=0; jx<segs-doPie; jx++) {
					vv[1] = segss*lsegs + (jx+1)%segs;
					vv[0] = segss*lsegs + jx;
					vv[3] = startv+jx;
					vv[2] = startv+(jx+1)%segs;
					mesh.f[nf].MakePoly (4, vv);
					mesh.f[nf].smGroup = 1;
					mesh.f[nf].material = 0;
					nf++;
				}
			} else {
				for (jx=0; jx<segs-doPie; jx++) {
					vv[1] = startv + segs*(ix-2) + (jx+1)%segs;
					vv[0] = startv + segs*(ix-2) + jx;
					vv[2] = vv[1] + segs;
					vv[3] = vv[0] + segs;
					mesh.f[nf].MakePoly (4, vv);
					mesh.f[nf].smGroup = 1;
					mesh.f[nf].material = 0;
					nf++;
				}
			}
		}
		if (!doPie) {
			mesh.v[nv].p = Point3(0.0f,0.0f,height);
			nv++;
		}
		// Last row of faces:
		for (jx=0; jx<segs-doPie; jx++) {
			vv[1] = startv + segs*(capsegs-2) + (jx+1)%segs;
			vv[0] = startv + segs*(capsegs-2) + jx;
			vv[2] = doPie ? segss*(lsegs+1)-1 : nv-1;
			mesh.f[nf].MakePoly (3, vv);
			mesh.f[nf].smGroup = 1;
			mesh.f[nf].material = 0;
			nf++;
		}
	}

	if (genUVs) {
		UVWMapper mapper;
		mapper.cap = TRUE;
		mapper.type = MAP_CYLINDRICAL;
		mapper.uflip = 0;
		mapper.vflip = 0;
		mapper.wflip = 0;
		mapper.utile = 1.0f;
		mapper.vtile = 1.0f;
		mapper.wtile = 1.0f;
		mapper.tm.IdentityMatrix();
		Matrix3 tm(1);
		float r = fabsf(radius1) > fabsf(radius2) ? fabsf(radius1) : fabsf(radius2);
		float h = height;
		if (r==0.0f) r = 1.0f;
		else r = 1.0f/r;
		if (h==0.0f) h = 1.0f;
		else h = 1.0f/h;
		mapper.tm.Scale(Point3(r,r,h));
		mapper.tm.RotateZ(HALFPI);
		mapper.tm.SetTrans(Point3(0.0f,0.0f,-0.5f));
		mesh.ApplyMapper (mapper, 1);
	}

	DbgAssert (nf==mesh.numf);
	DbgAssert (nv==mesh.numv);
	mesh.InvalidateGeomCache();
	mesh.FillInMesh ();
}

void BuildCylinderMesh(Mesh &mesh,
		int segs, int smooth, int llsegs, int capsegs, int doPie,
		float radius1, float radius2, float height, float pie1, float pie2,
		int genUVs)
	{
	Point3 p;
	int ix,na,nb,nc,nd,jx,kx, ic = 1;
	int nf=0,nv=0, lsegs;
	float delta,ang, u;	
	float totalPie, startAng = 0.0f;	

	if (doPie) doPie = 1;
	else doPie = 0; 

	lsegs = llsegs-1 + 2*capsegs;

	// Make pie2 < pie1 and pie1-pie2 < TWOPI
	while (pie1 < pie2) pie1 += TWOPI;
	while (pie1 > pie2+TWOPI) pie1 -= TWOPI;
	if (pie1==pie2) totalPie = TWOPI;
	else totalPie = pie1-pie2;	

	if (doPie) {
		segs++; //*** O.Z. fix for bug 240436 
		delta    = totalPie/(float)(segs-1);
		startAng = (height<0) ? pie1 : pie2;	// mjm - 2.16.99
//		startAng = pie2;						// mjm - 2.16.99
	} else {
		delta = (float)2.0*PI/(float)segs;
		}

	if (height<0) delta = -delta;

	int nverts;
	int nfaces;
	if (doPie) {
		nverts = (segs+1)*(lsegs);		
		nfaces = 2*(segs+1)*(lsegs-1) + 2*(segs-1);		
	} else {
		nverts = 2+segs*(lsegs);
		nfaces = 2*segs*(lsegs);	
		}
	mesh.setNumVerts(nverts);
	mesh.setNumFaces(nfaces);
	mesh.setSmoothFlags((smooth != 0) | ((doPie != 0) << 1));
	if (0/*genUVs*/) {
		mesh.setNumTVerts(nverts);
		mesh.setNumTVFaces(nfaces);
	} else {
		mesh.setNumTVerts(0);
		mesh.setNumTVFaces(0);
		}
	
	// bottom vertex 
	mesh.setVert(nv, Point3(0.0,0.0,0.0));
	//if (genUVs) mesh.setTVert(nv,0.5f,1.0f,0.0f);
	nv++;
		
	// Bottom cap vertices	
	for(ix=0; ix<capsegs; ix++) {
		
		// Put center vertices all the way up
		if (ix && doPie) {
			p.z = height*((float)ic/float(lsegs-1));
			p.x = p.y = 0.0f;
			mesh.setVert(nv, p);
			//if (genUVs) mesh.setTVert(nv,0.5f, (float)ic/float(lsegs-1), 0.0f);
			nv++;
			ic++;
			}
		
		p.z = 0.0f;
		u   = float(ix+1)/float(capsegs);
		ang = startAng;
		for (jx = 0; jx<segs; jx++) {			
			p.x = (float)cos(ang)*radius1*u;
			p.y = (float)sin(ang)*radius1*u;	
			mesh.setVert(nv, p);
			//if (genUVs) mesh.setTVert(nv,float(jx)/float(segs),1.0f-u,0.0f);
			nv++;
			ang += delta;
			}	
		}

	// Middle vertices 
	for(ix=1; ix<llsegs; ix++) {
			
		// Put center vertices all the way up
		if (doPie) {
			p.z = height*((float)ic/float(lsegs-1));
			p.x = p.y = 0.0f;
			mesh.setVert(nv, p);
			//if (genUVs) mesh.setTVert(nv,0.5f, (float)ic/float(lsegs-1), 0.0f);
			nv++;
			ic++;
			}
		
		float   u = float(ix)/float(llsegs);
		float rad = (radius1*(1.0f-u) + radius2*u);
		p.z = height*((float)ix/float(llsegs));
		ang = startAng;
		for (jx = 0; jx<segs; jx++) {
			p.x = (float)cos(ang)*rad;
			p.y = (float)sin(ang)*rad;
			mesh.setVert(nv, p);
			//if (genUVs) mesh.setTVert(nv,float(jx)/float(segs),(float)ix/float(llsegs),0.0f);
			nv++;
			ang += delta;
			}	
		}

	// Top cap vertices	
	for(ix=0; ix<capsegs; ix++) {
		
		// Put center vertices all the way up
		if (doPie) {
			p.z = height*((float)ic/float(lsegs-1));
			p.x = p.y = 0.0f;
			mesh.setVert(nv, p);
			//if (genUVs) mesh.setTVert(nv,0.5f, (float)ic/float(lsegs-1), 0.0f);
			ic++;
			nv++;
			}
		
		p.z = height;
		u   = 1.0f-float(ix)/float(capsegs);
		ang = startAng;
		for (jx = 0; jx<segs; jx++) {			
			p.x = (float)cos(ang)*radius2*u;		
			p.y = (float)sin(ang)*radius2*u;	
			mesh.setVert(nv, p);
			//if (genUVs) mesh.setTVert(nv,float(jx)/float(segs),u,0.0f);
			nv++;
			ang += delta;
			}	
		}

	/* top vertex */
	if (!doPie) {
		mesh.setVert(nv, (float)0.0, (float)0.0, height);
		//if (genUVs) mesh.setTVert(nv,0.5f,0.0f,0.0f);
		nv++;
		}	

	// Now make faces ---

	// Make bottom cap		

	for(ix=1; ix<=segs - doPie; ++ix) {
		nc=(ix==segs)?1:ix+1;
		if (doPie && ix==1) 
			 mesh.faces[nf].setEdgeVisFlags(capsegs>1,1,1);
		else if (doPie && ix==segs - doPie) 
			 mesh.faces[nf].setEdgeVisFlags(1,1,0);
		else mesh.faces[nf].setEdgeVisFlags(capsegs>1,1,capsegs>1);
		mesh.faces[nf].setSmGroup(1);
		mesh.faces[nf].setVerts(0,nc,ix);
		mesh.faces[nf].setMatID(1);
		//if (genUVs) mesh.tvFace[nf].setTVerts(0,nc,ix);
		nf++;
		}

	/* Make midsection */
	for(ix=0; ix<lsegs-1; ++ix) {
		if (doPie) {
			jx = ix*(segs+1);
		} else {
			jx=ix*segs+1;
			}
				
		for(kx=0; kx<segs+doPie; ++kx) {			
			DWORD grp = 0;
			int mtlid;
			BOOL inSlice = FALSE;

			if (kx==0 && doPie) {
				mtlid = 3;
				grp = (1<<1);
				inSlice = TRUE;
			} else 
			if (kx==segs) {
				mtlid = 4;
				grp = (1<<2);
				inSlice = TRUE;
			} else
			if (ix < capsegs-1 || ix >= capsegs+llsegs-1) {
				grp = 1;
				mtlid = (ix<capsegs-1)?0:1;
			} else	{		
				mtlid = 2;
				if (smooth) {				
					grp = (1<<3);	
					}			
				}

			na = jx+kx;
			nb = na+segs+doPie;
			nc = (kx==(segs+doPie-1))? jx+segs+doPie: nb+1;
			nd = (kx==(segs+doPie-1))? jx : na+1;			
			mesh.faces[nf].setEdgeVisFlags(0,!inSlice,1);
			mesh.faces[nf].setSmGroup(grp);
			mesh.faces[nf].setVerts(na,nc,nb);
			mesh.faces[nf].setMatID(mtlid);
			//if (genUVs) mesh.tvFace[nf].setTVerts(na,nc,nb);
			nf++;
			mesh.faces[nf].setEdgeVisFlags(!inSlice,1,0);
			mesh.faces[nf].setSmGroup(grp);
			mesh.faces[nf].setVerts(na,nd,nc);
			mesh.faces[nf].setMatID(mtlid);
			//if (genUVs) mesh.tvFace[nf].setTVerts(na,nd,nc);
			nf++;
			}
	 	}

	//Make Top cap 			
	if (doPie) {		
		na = (lsegs-1)*(segs+1);	
		jx = na + 1;
	} else {
		na = mesh.getNumVerts()-1;	
		jx = (lsegs-1)*segs+1;
		}
	for(ix=0; ix<segs-doPie; ++ix) {
		nb = jx+ix;
		nc = (ix==segs-1)? jx: nb+1;		
		if (doPie && ix==0) 
			 mesh.faces[nf].setEdgeVisFlags(1,1,0);
		else if (doPie && ix==segs-doPie-1) 
			 mesh.faces[nf].setEdgeVisFlags(capsegs>1,1,1);
		else mesh.faces[nf].setEdgeVisFlags(capsegs>1,1,capsegs>1);		
		mesh.faces[nf].setSmGroup( 1);
		mesh.faces[nf].setVerts(na,nb,nc);
		mesh.faces[nf].setMatID(0);
		//if (genUVs) mesh.tvFace[nf].setTVerts(na,nb,nc);
		nf++;
		}

	if (genUVs) {
		Matrix3 tm(1);
		float r = fabs(radius1) > fabs(radius2) ? (float)fabs(radius1) : (float)fabs(radius2);
		float h = height;
		if (r==0.0f) r = 1.0f;
		else r = 1.0f/r;
		if (h==0.0f) h = 1.0f;
		else h = 1.0f/h;
		tm.Scale(Point3(r,r,h));
		tm.RotateZ(HALFPI);
		tm.SetTrans(Point3(0.0f,0.0f,-0.5f));
		mesh.ApplyUVWMap(MAP_CYLINDRICAL,
			1.0f, 1.0f, 1.0f,
			0, 0, 0, 0,
			tm);
		}

	assert(nf==mesh.numFaces);
	assert(nv==mesh.numVerts);
	mesh.InvalidateTopologyCache();
	}

BOOL CylinderObject::HasUVW() { 
	BOOL genUVs;
	Interval v;
	pblock->GetValue(PB_GENUVS, 0, genUVs, v);
	return genUVs; 
	}

void CylinderObject::SetGenUVW(BOOL sw) {  
	if (sw==HasUVW()) return;
	pblock->SetValue(PB_GENUVS,0, sw);				

	}

Object *CylinderObject::BuildPoly (TimeValue t) {
	int segs, smooth, llsegs, capsegs;
	float radius,height,pie1, pie2;
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
	pblock->GetValue(PB_RADIUS,t,radius,gvalid);
	pblock->GetValue(PB_HEIGHT,t,height,gvalid);
	pblock->GetValue(PB_PIESLICE1,t,pie1,gvalid);
	pblock->GetValue(PB_PIESLICE2,t,pie2,gvalid);	
	LimitValue(radius, MIN_RADIUS, MAX_RADIUS);
	LimitValue(height, MIN_HEIGHT, MAX_HEIGHT);
	LimitValue(llsegs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(capsegs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(segs, MIN_SIDES, MAX_SIDES);
	LimitValue(smooth, 0, 1);	

	PolyObject *pobj = new PolyObject();
	MNMesh & mesh = pobj->GetMesh();
	BuildCylinderPoly (mesh, segs, smooth, llsegs, capsegs, doPie,
		radius, radius, height, pie1, pie2, genUVs);
	pobj->SetChannelValidity(TOPO_CHAN_NUM,tvalid);
	pobj->SetChannelValidity(GEOM_CHAN_NUM,gvalid);
	return pobj;
}

void CylinderObject::BuildMesh(TimeValue t)
	{	
	int segs, smooth, llsegs, capsegs;
	float radius,height,pie1, pie2;
	int doPie, genUVs;	

	// Start the validity interval at forever and widdle it down.
	ivalid = FOREVER;
	
	pblock->GetValue(PB_SIDES,t,segs,ivalid);
	pblock->GetValue(PB_SEGMENTS,t,llsegs,ivalid);
	pblock->GetValue(PB_CAPSEGMENTS,t,capsegs,ivalid);
	pblock->GetValue(PB_RADIUS,t,radius,ivalid);
	pblock->GetValue(PB_HEIGHT,t,height,ivalid);
	pblock->GetValue(PB_SMOOTH,t,smooth,ivalid);	
	pblock->GetValue(PB_PIESLICE1,t,pie1,ivalid);
	pblock->GetValue(PB_PIESLICE2,t,pie2,ivalid);	
	pblock->GetValue(PB_SLICEON,t,doPie,ivalid);	
	pblock->GetValue(PB_GENUVS,t,genUVs,ivalid);	
	LimitValue(radius, MIN_RADIUS, MAX_RADIUS);
	LimitValue(height, MIN_HEIGHT, MAX_HEIGHT);
	LimitValue(llsegs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(capsegs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(segs, MIN_SIDES, MAX_SIDES);
	LimitValue(smooth, 0, 1);	
	
	BuildCylinderMesh(mesh,
		segs, smooth, llsegs, capsegs, doPie,
		radius, radius, height, pie1, pie2, genUVs);
	}

inline Point3 operator+(const PatchVert &pv,const Point3 &p)
	{
	return p+pv.p;
	}

#define CIRCLE_VECTOR_LENGTH 0.5517861843f

void BuildCylinderPatch(
		TimeValue t, PatchMesh &patch, 
		float radius1, float radius2, float height, int genUVs)
	{
	int nverts = 10;
	int nvecs = 80;
	int npatches = 12;
	patch.setNumVerts(nverts);
	patch.setNumTVerts(genUVs ? 12 : 0);
	patch.setNumVecs(nvecs);
	patch.setNumPatches(npatches);	
	patch.setNumTVPatches(genUVs ? 12 : 0);

	// Center of base
	patch.setVert(0, 0.0f, 0.0f, 0.0f);
	
	// Base
	patch.setVert(1, radius1, 0.0f, 0.0f);
	patch.setVert(2, 0.0f, radius1, 0.0f);
	patch.setVert(3, -radius1, 0.0f, 0.0f);
	patch.setVert(4, 0.0f, -radius1, 0.0f);

	// Center of top
	patch.setVert(5, 0.0f, 0.0f, height);

	// Top
	patch.setVert(6, radius2, 0.0f, height);
	patch.setVert(7, 0.0f, radius2, height);
	patch.setVert(8, -radius2, 0.0f, height);
	patch.setVert(9, 0.0f, -radius2, height);

	// Tangents	
	Point3 vecs[] = {
		Point3(0.0f, CIRCLE_VECTOR_LENGTH,  0.0f),
		Point3(-CIRCLE_VECTOR_LENGTH, 0.0f, 0.0f),
		Point3(0.0f, -CIRCLE_VECTOR_LENGTH,  0.0f),
		Point3(CIRCLE_VECTOR_LENGTH, 0.0f, 0.0f)};
	
	float len = 1.0f/3.0f;
	Point3 vecs2[] = {
		Point3(0.0f, len,  0.0f),
		Point3(-len, 0.0f, 0.0f),
		Point3(0.0f, -len,  0.0f),
		Point3( len, 0.0f, 0.0f)};
	
	float rr = (radius1-radius2)/3.0f;
	float hh = height/3.0f;
	Point3 hvecs1[] = {
		Point3( -rr, 0.0f, hh),
		Point3( 0.0f, -rr, hh),
		Point3( rr, 0.0f, hh),
		Point3( 0.0f, rr, hh)};
	Point3 hvecs2[] = {
		Point3( rr, 0.0f, -hh),
		Point3( 0.0f, rr, -hh),
		Point3( -rr, 0.0f, -hh),
		Point3( 0.0f, -rr, -hh)};

	int ix=0;
	for (int j=0; j<4; j++) {
		patch.setVec(ix++,patch.verts[0] + vecs2[j]*radius1);
		}
	for (int i=0; i<4; i++) {		
		patch.setVec(ix++,patch.verts[i+1] + vecs[(i)%4]*radius1);
		patch.setVec(ix++,patch.verts[i+1] + vecs2[(1+i)%4]*radius1);
		patch.setVec(ix++,patch.verts[i+1] + vecs[(2+i)%4]*radius1);		
		patch.setVec(ix++,patch.verts[i+1] + hvecs1[i]);
		}
	for (j=0; j<4; j++) {
		patch.setVec(ix++,patch.verts[5] + vecs2[j]*radius2);
		}
	for (i=0; i<4; i++) {
		patch.setVec(ix++,patch.verts[i+6] + vecs[(i)%4]*radius2);
		patch.setVec(ix++,patch.verts[i+6] + vecs2[(1+i)%4]*radius2);
		patch.setVec(ix++,patch.verts[i+6] + vecs[(2+i)%4]*radius2);		
		patch.setVec(ix++,patch.verts[i+6] + hvecs2[i]);
		}	
	
#define Tang(vv,ii) ((vv)*4+(ii))
	
	// Build the patches
	int interior = 40;
	for (i=0; i<4; i++) {
		patch.patches[i].SetType(PATCH_QUAD);
		patch.patches[i].setVerts(i+6, i+1, (i+1)%4+1, (i+1)%4+6);		
		patch.patches[i].setVecs(
			Tang(i+6,3), Tang(i+1,3), 
			Tang(i+1,0), Tang((i+1)%4+1,2), 
			Tang((i+1)%4+1,3), Tang((i+1)%4+6,3), 
			Tang((i+1)%4+6,2), Tang(i+6,0));
		patch.patches[i].setInteriors(interior,interior+1,interior+2,interior+3);
		patch.patches[i].smGroup = 1;
//watje 3-17-99 to support patch matids
		patch.patches[i].setMatID(2);

		interior += 4;
		}
	
	for (i=0; i<4; i++) {
		patch.patches[i+4].SetType(PATCH_TRI);
		patch.patches[i+4].setVerts(0,(i+1)%4+1,i+1);
		patch.patches[i+4].setVecs(
			Tang(0,i),Tang((i+1)%4+1,1),
			Tang((i+1)%4+1,2),Tang(i+1,0),
			Tang(i+1,1),Tang(0,(i+3)%4));
		patch.patches[i+4].setInteriors(interior,interior+1,interior+2);
		patch.patches[i].smGroup = 2;
//watje 3-17-99 to support patch matids
		patch.patches[i+4].setMatID(1);

		interior += 3;		
		}

	for (i=0; i<4; i++) {
		patch.patches[i+8].SetType(PATCH_TRI);
		patch.patches[i+8].setVerts(5,i+6,(i+1)%4+6);
		patch.patches[i+8].setVecs(
			Tang(5,(i+3)%4),Tang(i+6,1),
			Tang(i+6,0),Tang((i+1)%4+6,2),
			Tang((i+1)%4+6,1),Tang(5,i));			
		patch.patches[i+8].setInteriors(interior,interior+1,interior+2);
		patch.patches[i].smGroup = 2;
//watje 3-17-99 to support patch matids
		patch.patches[i+8].setMatID(0);

		interior += 3;
		}
//232207
//needed to move this befoe the mapper since the mapper now uses the edge list
	assert(patch.buildLinkages());
	if(genUVs) {
		Matrix3 tm(1);
		float r = fabs(radius1) > fabs(radius2) ? (float)fabs(radius1) : (float)fabs(radius2);
		float h = height;
		if (r==0.0f) r = 1.0f;
		else r = 1.0f/r;
		if (h==0.0f) h = 1.0f;
		else h = 1.0f/h;
		tm.Scale(Point3(r,r,h));
		tm.RotateZ(HALFPI);
		tm.SetTrans(Point3(0.0f,0.0f,-0.5f));
		patch.ApplyUVWMap(MAP_CYLINDRICAL,
			1.0f, 1.0f, 1.0f,
			0, 0, 0, 0,
			tm);
		}
	

	patch.computeInteriors();
	patch.InvalidateGeomCache();
	}


#ifndef NO_NURBS

Object *
BuildNURBSCylinder(float radius, float height, BOOL sliceon, float pie1, float pie2, BOOL genUVs)
{
	BOOL flip = FALSE;

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
	strcpy(bname, GetString(IDS_RB_CYLINDER));
	sprintf(sname, "%s%s", bname, GetString(IDS_CT_SURF));
	surf->SetName(sname);

	if (sliceon && pie1 != pie2) {
		GenNURBSCylinderSurface(radius, height, origin, symAxis, refAxis,
						startAngle, endAngle, TRUE, *surf);

		NURBSCVSurface *s0 = (NURBSCVSurface*)nset.GetNURBSObject(0);

		Point3 cen;
		// next the two caps
		for (int c = 0; c < 2; c++) {
			if (c == 0)
				cen = Point3(0,0,0);
			else
				cen = Point3(0.0f, 0.0f, height);
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

		// now the pie slices
#define F(s1, s2, s1r, s1c, s2r, s2c) \
		fuse.mSurf1 = (s1); \
		fuse.mSurf2 = (s2); \
		fuse.mRow1 = (s1r); \
		fuse.mCol1 = (s1c); \
		fuse.mRow2 = (s2r); \
		fuse.mCol2 = (s2c); \
		nset.mSurfFuse.Append(1, &fuse);

		NURBSFuseSurfaceCV fuse;

		NURBSCVSurface *s1 = (NURBSCVSurface*)nset.GetNURBSObject(1);
		NURBSCVSurface *s2 = (NURBSCVSurface*)nset.GetNURBSObject(2);

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
							// get x and y from a cap and z from the main sheet.
							Point3 p;
							if (c == 0)
								p = Point3(s1->GetCV(v, 0)->GetPosition(0).x,
											s1->GetCV(v, 0)->GetPosition(0).y,
											s0->GetCV(u, v)->GetPosition(0).z);
							else
								p = Point3(s1->GetCV(v, s1v-1)->GetPosition(0).x,
											s1->GetCV(v, s1v-1)->GetPosition(0).y,
											s0->GetCV(u, v)->GetPosition(0).z);
							NURBSControlVertex ncv;
							ncv.SetPosition(0, p);
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

		NURBSCVSurface *s3 = (NURBSCVSurface*)nset.GetNURBSObject(3);
		NURBSCVSurface *s4 = (NURBSCVSurface*)nset.GetNURBSObject(4);

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
		GenNURBSCylinderSurface(radius, height, origin, symAxis, refAxis,
						startAngle, endAngle, FALSE, *surf);

		// now create caps on the ends
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

	Matrix3 mat;
	mat.IdentityMatrix();
	Object *ob = CreateNURBSObject(NULL, &nset, mat);
	return ob;
}

#endif

Object* CylinderObject::ConvertToType(TimeValue t, Class_ID obtype)
	{
#ifndef NO_PATCHES
	if (obtype == patchObjectClassID) {
		Interval valid = FOREVER;
		float radius, height;
		int genUVs;
		pblock->GetValue(PB_RADIUS,t,radius,valid);
		pblock->GetValue(PB_HEIGHT,t,height,valid);	
		pblock->GetValue(PB_GENUVS,t,genUVs,valid);
		PatchObject *ob = new PatchObject();
		BuildCylinderPatch(t,ob->patch,radius,radius,height,genUVs);
		ob->SetChannelValidity(TOPO_CHAN_NUM,valid);
		ob->SetChannelValidity(GEOM_CHAN_NUM,valid);
		ob->UnlockObject();
		return ob;
	} 
#endif
#ifndef NO_NURBS
    if (obtype == EDITABLE_SURF_CLASS_ID) {
		Interval valid = FOREVER;
		float radius, height, pie1, pie2;
		int sliceon, genUVs;
		pblock->GetValue(PB_RADIUS,t,radius,valid);
		pblock->GetValue(PB_HEIGHT,t,height,valid);	
		pblock->GetValue(PB_PIESLICE1,t,pie1,valid);	
		pblock->GetValue(PB_PIESLICE2,t,pie2,valid);	
		pblock->GetValue(PB_SLICEON,t,sliceon,valid);
		pblock->GetValue(PB_GENUVS,t,genUVs,valid);
		Object *ob = BuildNURBSCylinder(radius, height, sliceon, pie1, pie2, genUVs);
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
		int sides;
		float radius, height;
		pblock->GetValue(PB_RADIUS,t,radius,valid);
		pblock->GetValue(PB_HEIGHT,t,height,valid);	
		pblock->GetValue(PB_SIDES,t,sides,valid);	
		int smooth;
		pblock->GetValue(PB_SMOOTH,t,smooth,valid);
		Object* solid = (Object*)CreateInstance(GEOMOBJECT_CLASS_ID, GENERIC_AMSOLID_CLASS_ID);
		assert(solid);
		if(solid)
		{
			IGeomImp* cacheptr = (IGeomImp*)(solid->GetInterface(I_GEOMIMP));
			assert(cacheptr);
			if(cacheptr)
			{
				bool res = cacheptr->createCylinder(height, radius,sides, smooth);
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

	if (obtype == polyObjectClassID) {
		Object *ob = BuildPoly (t);
		ob->UnlockObject ();
		return ob;
	}
	return SimpleObject::ConvertToType(t,obtype);
}

int CylinderObject::CanConvertToType(Class_ID obtype)
	{
	if(obtype==defObjectClassID || obtype==triObjectClassID) return 1;
#ifndef NO_PATCHES
    if(obtype == patchObjectClassID) return 1;
#endif
#ifndef NO_NURBS
    if(obtype == EDITABLE_SURF_CLASS_ID) return 1;
#endif
#ifdef DESIGN_VER
	if (obtype == GENERIC_AMSOLID_CLASS_ID) return 1;
#endif
	if (obtype == polyObjectClassID) return 1;

	return SimpleObject::CanConvertToType(obtype);
}

void CylinderObject::GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist)
{
    Object::GetCollapseTypes(clist, nlist);
#ifndef NO_NURBS
    Class_ID id = EDITABLE_SURF_CLASS_ID;
    TSTR *name = new TSTR(GetString(IDS_SM_NURBS_SURFACE));
    clist.Append(1,&id);
    nlist.Append(1,&name);
#endif
}

class CylinderObjCreateCallBack: public CreateMouseCallBack {
	CylinderObject *ob;	
	Point3 p[2];
	IPoint2 sp0,sp1;	
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		void SetObj(CylinderObject *obj) { ob = obj; }
	};

int CylinderObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
	float r;
	if (msg == MOUSE_FREEMOVE)
	{
		#ifdef _OSNAP
			#ifdef _3D_CREATE
				vpt->SnapPreview(m,m,NULL, SNAP_IN_3D);
			#else
				vpt->SnapPreview(m,m,NULL, SNAP_IN_PLANE);
			#endif
		#endif
	}
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
				ob->pblock->SetValue(PB_RADIUS,0,0.01f);
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
				
				ob->pblock->SetValue(PB_RADIUS,0,r);
				ob->pmapParam->Invalidate();

				if (flags&MOUSE_CTRL) {
					float ang = (float)atan2(p[1].y-p[0].y,p[1].x-p[0].x);
					mat.PreRotateZ(ob->ip->SnapAngle(ang));
					}

				if (msg==MOUSE_POINT) {
					if (Length(m-sp0)<3 ||
						Length(p[1]-p[0])<0.1f) {						
						return CREATE_ABORT;
						}
					}
				break;
			case 2:
				{
#ifdef _OSNAP
				float h = vpt->SnapLength(vpt->GetCPDisp(p[1],Point3(0,0,1),sp1,m,TRUE));
#else
				float h = vpt->SnapLength(vpt->GetCPDisp(p[1],Point3(0,0,1),sp1,m));
#endif
				ob->pblock->SetValue(PB_HEIGHT,0,h);
				ob->pmapParam->Invalidate();
				if (msg==MOUSE_POINT) {					
					ob->suspendSnap = FALSE;
					return (Length(m-sp0)<3)?CREATE_ABORT:CREATE_STOP;
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

static CylinderObjCreateCallBack cylCreateCB;

CreateMouseCallBack* CylinderObject::GetCreateMouseCallBack() 
	{
	cylCreateCB.SetObj(this);
	return(&cylCreateCB);
	}

BOOL CylinderObject::OKtoDisplay(TimeValue t) 
	{
	float radius;
	pblock->GetValue(PB_RADIUS,t,radius,FOREVER);
	if (radius==0.0f) return FALSE;
	else return TRUE;
	}


// From ParamArray
BOOL CylinderObject::SetValue(int i, TimeValue t, int v) 
	{
	switch (i) {
		case PB_CREATEMETHOD: dlgCreateMeth = v; break;
		}		
	return TRUE;
	}

BOOL CylinderObject::SetValue(int i, TimeValue t, float v)
	{
	switch (i) {				
		case PB_TI_RADIUS: crtRadius = v; break;
		case PB_TI_HEIGHT: crtHeight = v; break;
		}	
	return TRUE;
	}

BOOL CylinderObject::SetValue(int i, TimeValue t, Point3 &v) 
	{
	switch (i) {
		case PB_TI_POS: crtPos = v; break;
		}		
	return TRUE;
	}

BOOL CylinderObject::GetValue(int i, TimeValue t, int &v, Interval &ivalid) 
	{
	switch (i) {
		case PB_CREATEMETHOD: v = dlgCreateMeth; break;
		}
	return TRUE;
	}

BOOL CylinderObject::GetValue(int i, TimeValue t, float &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_RADIUS: v = crtRadius; break;
		case PB_TI_HEIGHT: v = crtHeight; break;
		}
	return TRUE;
	}

BOOL CylinderObject::GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_POS: v = crtPos; break;		
		}
	return TRUE;
	}


void CylinderObject::InvalidateUI() 
	{
	if (pmapParam) pmapParam->Invalidate();
	}

ParamDimension *CylinderObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_RADIUS: 	return stdWorldDim;
		case PB_HEIGHT: 	return stdWorldDim;
		case PB_SEGMENTS: 	return stdSegmentsDim;
		case PB_CAPSEGMENTS:return stdSegmentsDim;
		case PB_SIDES:		return stdSegmentsDim;
		case PB_SMOOTH:		return stdNormalizedDim;
		case PB_SLICEON:	return stdNormalizedDim;
		case PB_PIESLICE1:	return stdAngleDim;
		case PB_PIESLICE2:	return stdAngleDim;
		default: return defaultDim;
		}
	}

TSTR CylinderObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_RADIUS:		return GetString(IDS_RB_RADIUS);
		case PB_HEIGHT:		return GetString(IDS_RB_HEIGHT);
		case PB_SEGMENTS:	return GetString(IDS_RB_CIRCLESEGMENTS);
		case PB_CAPSEGMENTS:return GetString(IDS_RB_CAPSEGMENTS);
		case PB_SIDES:		return GetString(IDS_RB_SIDES);
		case PB_SMOOTH:		return GetString(IDS_RB_SMOOTH);
		case PB_SLICEON:	return GetString(IDS_RB_SLICEON);
		case PB_PIESLICE1:	return GetString(IDS_RB_SLICEFROM);
		case PB_PIESLICE2:	return GetString(IDS_RB_SLICETO);
		//case PB_GENUVS:		return GetString(IDS_RB_GENTEXCOORDS);
		default: return TSTR(_T(""));		
		}
	}

RefTargetHandle CylinderObject::Clone(RemapDir& remap) 
	{
	CylinderObject* newob = new CylinderObject();	
	newob->ReplaceReference(0,pblock->Clone(remap));	
	newob->ivalid.SetEmpty();	
	BaseClone(this, newob, remap);
	return(newob);
	}

