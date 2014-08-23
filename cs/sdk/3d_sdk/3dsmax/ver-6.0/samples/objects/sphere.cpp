/**********************************************************************
 *<
	FILE: sphere.cpp

	DESCRIPTION:  Sphere object, Revised implementation

	CREATED BY: Rolf Berteig

	HISTORY: created 10/10/95

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "prim.h"

#include "iparamm.h"
#include "Simpobj.h"
#include "surf_api.h"
#include "notify.h"

class SphereObject : public GenSphere, public IParamArray {
	public:			
		// Class vars
		static IParamMap *pmapCreate;
		static IParamMap *pmapTypeIn;
		static IParamMap *pmapParam;
		static int dlgSegments;
		static int dlgCreateMeth;
		static int dlgSmooth,dlgSlice;
		static Point3 crtPos;		
		static float crtRadius,crtSliceFrom,crtSliceTo;
		static BOOL baseToPivot;
		static IObjParam *ip;
		IParamBlock *temppb;
		// mjm - 3.19.99 - ensure accurate matIDs and smoothing groups
		int lastSquash;
		BOOL lastNoHemi;

		SphereObject();		
		~SphereObject();		
		
		// From Object
		int CanConvertToType(Class_ID obtype);
		Object* ConvertToType(TimeValue t, Class_ID obtype);
		void GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist);
		
		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() { return GetString(IDS_RB_SPHERE); }
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);
		BOOL IsParamSurface() {return TRUE;}
		Point3 GetSurfacePoint(TimeValue t, float u, float v,Interval &iv);

		// From GeomObject
		int IntersectRay(TimeValue t, Ray& ray, float& at, Point3& norm);
		
		// Animatable methods		
		void DeleteThis() {delete this;}
		Class_ID ClassID() { return Class_ID(SPHERE_CLASS_ID,0); } 
		
		// From ReferenceTarget
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);
				
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

		// From GenSphere
		void SetParams(float rad, int segs, BOOL smooth=TRUE, BOOL genUV=TRUE,
			 float hemi=0.0f, BOOL squash=FALSE, BOOL recenter=FALSE);
		void PreSaveOld(); 
		void PostSaveOld(); 
	};


// Misc stuff
#define MAX_SEGMENTS	200
#define MIN_SEGMENTS	4

#define MIN_RADIUS		float(0)
#define MAX_RADIUS		float(1.0E30)

#define MIN_SMOOTH		0
#define MAX_SMOOTH		1

#define DEF_SEGMENTS	32	// 16
#define DEF_RADIUS		float(0.0)

#define SMOOTH_ON	1
#define SMOOTH_OFF	0

#define MIN_SLICE	float(-1.0E30)
#define MAX_SLICE	float( 1.0E30)


//--- ClassDescriptor and class vars ---------------------------------

// The class descriptor for sphere
class SphereClassDesc:public ClassDesc {
	public:
// xavier robitaille | 03.02.15 | private boxes, spheres and cylinders 
#ifndef NO_OBJECT_STANDARD_PRIMITIVES
	int 			IsPublic() { return 1; }
#else
	int 			IsPublic() { return 0; }
#endif
	void *			Create(BOOL loading = FALSE) { return new SphereObject; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_SPHERE_CLASS); }
	SClass_ID		SuperClassID() { return GEOMOBJECT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(SPHERE_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_PRIMITIVES); }
	void			ResetClassParams(BOOL fileReset);
	};

static SphereClassDesc sphereDesc;
extern ClassDesc* GetSphereDesc() { return &sphereDesc; }


// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

int SphereObject::dlgSegments       = DEF_SEGMENTS;
int SphereObject::dlgCreateMeth     = 1; // create_radius
int SphereObject::dlgSmooth         = SMOOTH_ON;
IParamMap *SphereObject::pmapCreate = NULL;
IParamMap *SphereObject::pmapParam  = NULL;
IParamMap *SphereObject::pmapTypeIn = NULL;
IObjParam *SphereObject::ip         = NULL;
Point3 SphereObject::crtPos         = Point3(0,0,0);
float SphereObject::crtRadius       = 0.0f;
float SphereObject::crtSliceFrom       = 0.0f;
float SphereObject::crtSliceTo       = 0.0f;
int SphereObject::dlgSlice          = 0;
int SphereObject::baseToPivot	 = FALSE;

void SphereClassDesc::ResetClassParams(BOOL fileReset)
	{
	SphereObject::dlgSegments    = DEF_SEGMENTS;
	SphereObject::dlgCreateMeth  = 1; // create_radius
	SphereObject::dlgSmooth      = SMOOTH_ON;
	SphereObject::crtPos         = Point3(0,0,0);
	SphereObject::crtRadius      = 0.0f;
	SphereObject::dlgSlice          = 0;
	SphereObject::crtSliceFrom    = 0.0f;
	SphereObject::crtSliceTo    = 0.0f;
	SphereObject::baseToPivot	 = FALSE;
	}


//--- Parameter map/block descriptors -------------------------------

// Parameter block indices
#define PB_RADIUS	0
#define PB_SEGS		1
#define PB_SMOOTH	2
#define PB_HEMI		3
#define PB_SQUASH	4
#define PB_RECENTER	5
#define PB_GENUVS	6
#define PB_SLICEON		7
#define PB_SLICEFROM	8
#define PB_SLICETO		9

// Non-parameter block indices
#define PB_CREATEMETHOD		0
#define PB_TI_POS			1
#define PB_TI_RADIUS		2


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
		SPIN_AUTOSCALE)	
	};
#define TYPEINDESC_LENGH 2


//
//
// Parameters

static int squashIDs[] = {IDC_HEMI_CHOP,IDC_HEMI_SQUASH};

static ParamUIDesc descParam[] = {
	// Radius
	ParamUIDesc(
		PB_RADIUS,
		EDITTYPE_UNIVERSE,
		IDC_RADIUS,IDC_RADSPINNER,
		MIN_RADIUS,MAX_RADIUS,
		SPIN_AUTOSCALE),	
	
	// Segments
	ParamUIDesc(
		PB_SEGS,
		EDITTYPE_INT,
		IDC_SEGMENTS,IDC_SEGSPINNER,
		(float)MIN_SEGMENTS,(float)MAX_SEGMENTS,
		0.1f),
	
	// Smooth
	ParamUIDesc(PB_SMOOTH,TYPE_SINGLECHEKBOX,IDC_OBSMOOTH),

	// Hemisphere
	ParamUIDesc(
		PB_HEMI,
		EDITTYPE_FLOAT,
		IDC_HEMISPHERE,IDC_HEMISPHERESPINNER,
		0.0f,1.0f,
		0.005f),

	// Chop/squash
	ParamUIDesc(PB_SQUASH,TYPE_RADIO,squashIDs,2),

	// Gen Slice
	ParamUIDesc(PB_SLICEON,TYPE_SINGLECHEKBOX,IDC_SC_SLICEON),			

	// Pie slice from
	ParamUIDesc(
		PB_SLICEFROM,
		EDITTYPE_FLOAT,
		IDC_SC_SLICE1,IDC_SC_SLICE1SPIN,
		MIN_SLICE,MAX_SLICE,		
		0.5f,
		stdAngleDim),

	// Pie slice to
	ParamUIDesc(
		PB_SLICETO,
		EDITTYPE_FLOAT,
		IDC_SC_SLICE2,IDC_SC_SLICE2SPIN,
		MIN_SLICE,MAX_SLICE,		
		0.5f,
		stdAngleDim),

	// Recenter
	ParamUIDesc(PB_RECENTER,TYPE_SINGLECHEKBOX,IDC_HEMI_RECENTER),

	// Gen UVs
	ParamUIDesc(PB_GENUVS,TYPE_SINGLECHEKBOX,IDC_GENTEXTURE),
	};
#define PARAMDESC_LENGH 10


static ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },		
	{ TYPE_INT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, TRUE, 2 } };

static ParamBlockDescID descVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },	
	{ TYPE_INT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, TRUE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 },
	{ TYPE_INT, NULL, FALSE, 4 },
	{ TYPE_INT, NULL, FALSE, 5 } };

static ParamBlockDescID descVer2[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },	
	{ TYPE_INT, NULL, TRUE, 1 },
	{ TYPE_BOOL, NULL, TRUE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 },
	{ TYPE_INT, NULL, FALSE, 4 },
	{ TYPE_INT, NULL, FALSE, 5 },
	{ TYPE_INT, NULL, FALSE, 6 } };

static ParamBlockDescID descVer3[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },	
	{ TYPE_INT, NULL, TRUE, 1 },
	{ TYPE_BOOL, NULL, TRUE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 },
	{ TYPE_INT, NULL, FALSE, 4 },
	{ TYPE_INT, NULL, FALSE, 5 },
	{ TYPE_INT, NULL, FALSE, 6 },
	{ TYPE_INT, NULL, FALSE, 7 },
	{ TYPE_FLOAT, NULL, TRUE, 8 },
	{ TYPE_FLOAT, NULL, TRUE, 9 },
};

#define PBLOCK_LENGTH	10

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,3,0),
	ParamVersionDesc(descVer1,6,1),
	ParamVersionDesc(descVer2,7,2)
	};
#define NUM_OLDVERSIONS	3

// Current version
#define CURRENT_VERSION	3
static ParamVersionDesc curVersion(descVer3,PBLOCK_LENGTH,CURRENT_VERSION);


//--- TypeInDlgProc --------------------------------

class SphereTypeInDlgProc : public ParamMapUserDlgProc {
	public:
		SphereObject *so;

		SphereTypeInDlgProc(SphereObject *s) {so=s;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

BOOL SphereTypeInDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_TI_CREATE: {
					if (so->crtRadius==0.0) return TRUE;
					
					// We only want to set the value if the object is 
					// not in the scene.
					if (so->TestAFlag(A_OBJ_CREATING)) {
						so->pblock->SetValue(PB_RADIUS,0,so->crtRadius);
						}

					Matrix3 tm(1);
					tm.SetTrans(so->crtPos);
					so->suspendSnap = FALSE;
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


class SphereParamDlgProc : public ParamMapUserDlgProc {
	public:
		SphereObject *so;
		HWND thishWnd;

		SphereParamDlgProc(SphereObject *s) {so=s;thishWnd=NULL;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void Update(TimeValue t);
		void DeleteThis() {delete this;}

//--- ParamDlgProc --------------------------------
		void TurnSpinner(HWND hWnd,int SpinNum,BOOL ison)
			{	
			ISpinnerControl *spin2 = GetISpinner(GetDlgItem(hWnd,SpinNum));
			if (ison) spin2->Enable();else spin2->Disable();
			ReleaseISpinner(spin2);

			};

	};
void SphereParamDlgProc::Update(TimeValue t)
{ if (!thishWnd) return;
  int ison;
  so->pblock->GetValue(PB_SLICEON,t,ison,FOREVER);
  TurnSpinner(thishWnd,IDC_SC_SLICE1SPIN,ison);
  TurnSpinner(thishWnd,IDC_SC_SLICE2SPIN,ison);

  EnableWindow(GetDlgItem(thishWnd,IDC_STATICFROM),ison);
  EnableWindow(GetDlgItem(thishWnd,IDC_STATICTO),ison);
}

BOOL SphereParamDlgProc::DlgProc(
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
					TurnSpinner(hWnd,IDC_SC_SLICE1SPIN,ison);
					TurnSpinner(hWnd,IDC_SC_SLICE2SPIN,ison);
					EnableWindow(GetDlgItem(hWnd,IDC_STATICFROM),ison);
					EnableWindow(GetDlgItem(hWnd,IDC_STATICTO),ison);
					return TRUE;	
					}
				}
			break;	
		}
	return FALSE;
	}



static void NotifyPreSaveOld(void *param, NotifyInfo *info) {
	SphereObject *mt = (SphereObject *)param;
	mt->PreSaveOld();
	}

static void NotifyPostSaveOld(void *param, NotifyInfo *info) {
	SphereObject *mt = (SphereObject *)param;
	mt->PostSaveOld();
	}

//--- Sphere methods -------------------------------


SphereObject::SphereObject() : lastSquash(-1), lastNoHemi(FALSE)
{
	SetAFlag(A_PLUGIN1);
	MakeRefByID(FOREVER, 0, CreateParameterBlock(descVer3, PBLOCK_LENGTH, CURRENT_VERSION));
	assert(pblock);
	temppb = NULL;
	
	pblock->SetValue(PB_RADIUS,0,crtRadius);
	pblock->SetValue(PB_SMOOTH,0,dlgSmooth);
	pblock->SetValue(PB_SEGS,0,dlgSegments);	
	pblock->SetValue(PB_SQUASH,0,0);
	pblock->SetValue(PB_SLICEON,0,dlgSlice);
	pblock->SetValue(PB_SLICEFROM,0,crtSliceFrom);
	pblock->SetValue(PB_SLICETO,0,crtSliceTo);
	pblock->SetValue(PB_GENUVS,0,TRUE);
	pblock->SetValue(PB_RECENTER,0, baseToPivot);				

	RegisterNotification(NotifyPreSaveOld, (void *)this, NOTIFY_FILE_PRE_SAVE_OLD);
	RegisterNotification(NotifyPostSaveOld, (void *)this, NOTIFY_FILE_POST_SAVE_OLD);
}

SphereObject::~SphereObject() {
	UnRegisterNotification(NotifyPreSaveOld, (void *)this, NOTIFY_FILE_PRE_SAVE_OLD);
	UnRegisterNotification(NotifyPostSaveOld, (void *)this, NOTIFY_FILE_POST_SAVE_OLD);
	}

void SphereObject::PreSaveOld() { 
	if (GetSavingVersion()==2000) {
		temppb = pblock;
		pblock =  UpdateParameterBlock(descVer3,PBLOCK_LENGTH,temppb, descVer2,7,2);
		}
	}

void SphereObject::PostSaveOld() { 
	if (temppb) {
		pblock->DeleteThis();
		pblock = temppb;
		temppb = NULL;
		}
	}


#define NEWMAP_CHUNKID	0x0100

IOResult SphereObject::Load(ILoad *iload) 
	{
	ClearAFlag(A_PLUGIN1);

	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {	
			case NEWMAP_CHUNKID:
				SetAFlag(A_PLUGIN1);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}

	iload->RegisterPostLoadCallback(
		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,0));
	return IO_OK;
	}

IOResult SphereObject::Save(ISave *isave)
	{
	if (TestAFlag(A_PLUGIN1)) {
		isave->BeginChunk(NEWMAP_CHUNKID);
		isave->EndChunk();
		}
 	return IO_OK;
	}

void SphereObject::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
	{
	SimpleObject::BeginEditParams(ip,flags,prev);
	this->ip = ip;

	if (pmapCreate && pmapParam) {
		
		// Left over from last sphere ceated
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
				MAKEINTRESOURCE(IDD_SPHEREPARAM1),
				GetString(IDS_RB_CREATIONMETHOD),
				0);

			pmapTypeIn = CreateCPParamMap(
				descTypeIn,TYPEINDESC_LENGH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_SPHEREPARAM3),
				GetString(IDS_RB_KEYBOARDENTRY),
				APPENDROLL_CLOSED);
			}

		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_SPHEREPARAM2),
			GetString(IDS_RB_PARAMETERS),
			0);
		}

	if(pmapTypeIn) {
		// A callback for the type in.
		pmapTypeIn->SetUserDlgProc(new SphereTypeInDlgProc(this));
		}
	if(pmapParam) {
		// A callback for the type in.
		pmapParam->SetUserDlgProc(new SphereParamDlgProc(this));
		}
	}
		
void SphereObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
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
	pblock->GetValue(PB_SEGS,ip->GetTime(),dlgSegments,FOREVER);
	pblock->GetValue(PB_SMOOTH,ip->GetTime(),dlgSmooth,FOREVER);	
	pblock->GetValue(PB_RECENTER,ip->GetTime(),baseToPivot,FOREVER);	
	}

Point3 SphereObject::GetSurfacePoint(
		TimeValue t, float u, float v,Interval &iv)
	{
	float rad;
	pblock->GetValue(PB_RADIUS, t, rad, iv);
	Point3 pos;	
	v -= 0.5f;
	float ar = (float)cos(v*PI);
	pos.x = rad * float(cos(u*TWOPI)) * ar;
	pos.y = rad * float(sin(u*TWOPI)) * ar;
	pos.z = rad * float(sin(v*PI));
	return pos;
	}

void SphereObject::SetParams(float rad, int segs, BOOL smooth, BOOL genUV,
	 float hemi, BOOL squash, BOOL recenter) {
	pblock->SetValue(PB_RADIUS,0, rad);				
	pblock->SetValue(PB_HEMI,0, hemi);				
	pblock->SetValue(PB_SEGS,0, segs);				
	pblock->SetValue(PB_SQUASH,0, squash);				
	pblock->SetValue(PB_SMOOTH,0, smooth);				
	pblock->SetValue(PB_RECENTER,0, recenter);				
	pblock->SetValue(PB_GENUVS,0, genUV);				
	}			   

BOOL SphereObject::HasUVW() { 
	BOOL genUVs;
	Interval v;
	pblock->GetValue(PB_GENUVS, 0, genUVs, v);
	return genUVs; 
	}

void SphereObject::SetGenUVW(BOOL sw) {  
	if (sw==HasUVW()) return;
	pblock->SetValue(PB_GENUVS,0, sw);				
	}

float uval[3]={1.0f,0.0f,1.0f};
void SphereObject::BuildMesh(TimeValue t)
	{
	Point3 p;	
	int ix,na,nb,nc,nd,jx,kx;
	int nf=0,nv=0;
	float delta, delta2;
	float a,alt,secrad,secang,b,c;
	int segs, smooth;
	float radius;
	float hemi;
	BOOL noHemi = FALSE;	
	int squash;
	int recenter;
	BOOL genUVs = TRUE;
	float startAng = 0.0f;
	float pie1,pie2;int doPie;
	if (TestAFlag(A_PLUGIN1)) startAng = HALFPI;

	// Start the validity interval at forever and widdle it down.
	ivalid = FOREVER;
	pblock->GetValue(PB_RADIUS, t, radius, ivalid);
	pblock->GetValue(PB_SEGS, t, segs, ivalid);
	pblock->GetValue(PB_SMOOTH, t, smooth, ivalid);
	pblock->GetValue(PB_HEMI, t, hemi, ivalid);
	pblock->GetValue(PB_SQUASH, t, squash, ivalid);
	pblock->GetValue(PB_RECENTER, t, recenter, ivalid);
	pblock->GetValue(PB_GENUVS, t, genUVs, ivalid);
	pblock->GetValue(PB_SLICEFROM,t,pie1,ivalid);
	pblock->GetValue(PB_SLICETO,t,pie2,ivalid);	
	pblock->GetValue(PB_SLICEON,t,doPie,ivalid);	
	LimitValue(segs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(smooth, MIN_SMOOTH, MAX_SMOOTH);
	LimitValue(radius, MIN_RADIUS, MAX_RADIUS);
	LimitValue(hemi, 0.0f, 1.0f);

	float totalPie(0.0f);
	if (doPie) doPie = 1;
	else doPie = 0;
	if (doPie)
	{ pie2+=startAng;pie1+=startAng;
	  while (pie1 < pie2) pie1 += TWOPI;
	  while (pie1 > pie2+TWOPI) pie1 -= TWOPI;
	  if (pie1==pie2) totalPie = TWOPI;
	  else totalPie = pie1-pie2;	
	}

	if (hemi<0.00001f) noHemi = TRUE;
	if (hemi>=1.0f) hemi = 0.9999f;
	hemi = (1.0f-hemi) * PI;
	float basedelta=2.0f*PI/(float)segs;
	delta2=(doPie?totalPie/(float)segs:basedelta);
	if (!noHemi && squash) {
		delta  = 2.0f*hemi/float(segs-2);
	} else {
		delta  = basedelta;
		}

	int rows;
	if (noHemi || squash) {
		rows = (segs/2-1);
	} else {
		rows = int(hemi/delta) + 1;
		}
	int realsegs=(doPie?segs+2:segs);
	int nverts = rows * realsegs + 2;
	int nfaces = rows * realsegs * 2;
	if (doPie) 
	{ startAng=pie2;segs+=1;
	  if (!noHemi) {nfaces-=2;nverts-=1;}
	}
	mesh.setNumVerts(nverts);
	mesh.setNumFaces(nfaces);
	mesh.setSmoothFlags(smooth != 0);
	int lastvert=nverts-1;

	// mjm - 3.19.99 - ensure accurate matIDs and smoothing groups
	if (lastSquash != squash || lastNoHemi != noHemi)
	{
		lastSquash = squash;
		lastNoHemi = noHemi;
		mesh.InvalidateStrips();
	}

	// Top vertex 
	mesh.setVert(nv, 0.0f, 0.0f, radius);
	nv++;

	// Middle vertices 
	alt=delta;
	for(ix=1; ix<=rows; ix++) {		
		if (!noHemi && ix==rows) alt = hemi;
		a = (float)cos(alt)*radius;		
		secrad = (float)sin(alt)*radius;
		secang = startAng; //0.0f
		for(jx=0; jx<segs; ++jx) {
			b = (float)cos(secang)*secrad;
			c = (float)sin(secang)*secrad;
			mesh.setVert(nv++,b,c,a);
			secang+=delta2;
			}
		if (doPie &&(noHemi ||(ix<rows))) mesh.setVert(nv++,0.0f,0.0f,a);
		alt+=delta;		
		}

	/* Bottom vertex */
	if (noHemi) {
		mesh.setVert(nv++, 0.0f, 0.0f,-radius);
		}
	else {
		a = (float)cos(hemi)*radius;
		mesh.setVert(nv++, 0.0f, 0.0f, a);
		}

	BOOL issliceface;
	// Now make faces 
	if (doPie) segs++;

	// Make top conic cap
	for(ix=1; ix<=segs; ++ix) {
		issliceface=(doPie && (ix>=segs-1));
		nc=(ix==segs)?1:ix+1;
		mesh.faces[nf].setEdgeVisFlags(1,1,1);
		if ((issliceface)&&(ix==segs-1))
		{	mesh.faces[nf].setSmGroup(smooth?4:0);
			mesh.faces[nf].setMatID(2);
		}
		else if ((issliceface)&&(ix==segs))
		{	mesh.faces[nf].setSmGroup(smooth?8:0);
			mesh.faces[nf].setMatID(3);
		}
		else
		{	mesh.faces[nf].setSmGroup(smooth?1:0);
			mesh.faces[nf].setMatID(1); // mjm - 5.5.99 - rollback change - should be fixed in later release
//			mesh.faces[nf].setMatID(0); // mjm - 3.2.99 - was set to 1
		}
		mesh.faces[nf].setVerts(0, ix, nc);
		nf++;
		}

	/* Make midsection */
	int lastrow=rows-1,lastseg=segs-1,almostlast=lastseg-1;
	BOOL weirdpt=doPie && !noHemi,weirdmid=weirdpt && (rows==2);
	for(ix=1; ix<rows; ++ix) {
		jx=(ix-1)*segs+1;
		for(kx=0; kx<segs; ++kx) {
			issliceface=(doPie && (kx>=almostlast));

			na = jx+kx;
			nb = na+segs;
			nb = (weirdmid &&(kx==lastseg)? lastvert:na+segs);
			if ((weirdmid) &&(kx==almostlast)) nc=lastvert; else
			nc = (kx==lastseg)? jx+segs: nb+1;
			nd = (kx==lastseg)? jx : na+1;
			
			mesh.faces[nf].setEdgeVisFlags(1,1,0);

			if ((issliceface)&&((kx==almostlast-2)||(kx==almostlast)))
			{	mesh.faces[nf].setSmGroup(smooth?4:0);
				mesh.faces[nf].setMatID(2);
			}
			else if((issliceface)&&((kx==almostlast-1)||(kx==almostlast+1)))
			{	mesh.faces[nf].setSmGroup(smooth?8:0);
				mesh.faces[nf].setMatID(3);
			}
			else
			{	mesh.faces[nf].setSmGroup(smooth?1:0);
				mesh.faces[nf].setMatID(1); // mjm - 5.5.99 - rollback change - should be fixed in later release
//				mesh.faces[nf].setMatID(0); // mjm - 3.2.99 - was set to 1
			}

			mesh.faces[nf].setVerts(na,nb,nc);
			nf++;

			mesh.faces[nf].setEdgeVisFlags(0,1,1);

			if ((issliceface)&&((kx==almostlast-2)||(kx==almostlast)))
			{	mesh.faces[nf].setSmGroup(smooth?4:0);
				mesh.faces[nf].setMatID(2);
			}
			else if((issliceface)&&((kx==almostlast-1)||(kx==almostlast+1)))
			{	mesh.faces[nf].setSmGroup(smooth?8:0);
				mesh.faces[nf].setMatID(3);
			}
			else
			{	mesh.faces[nf].setSmGroup(smooth?1:0);
				mesh.faces[nf].setMatID(1); // mjm - 5.5.99 - rollback change - should be fixed in later release
//				mesh.faces[nf].setMatID(0); // mjm - 3.2.99 - was set to 1
			}

			mesh.faces[nf].setVerts(na,nc,nd);
			nf++;
			}
	 	}

	// Make bottom conic cap
	na = mesh.getNumVerts()-1;
	int botsegs=(weirdpt?segs-2:segs);
	jx = (rows-1)*segs+1;lastseg=botsegs-1;
	for(ix=0; ix<botsegs; ++ix) {
		issliceface=(doPie && (ix>=botsegs-2));
		nc = ix + jx;
		nb = (!weirdpt && (ix==lastseg)?jx:nc+1);
		mesh.faces[nf].setEdgeVisFlags(1,1,1);

		if ((issliceface)&&(noHemi)&&(ix==botsegs-2))
		{	mesh.faces[nf].setSmGroup(smooth?4:0);
			mesh.faces[nf].setMatID(2);
		}
		else if ((issliceface)&&(noHemi)&&(ix==botsegs-1))
		{	mesh.faces[nf].setSmGroup(smooth?8:0);
			mesh.faces[nf].setMatID(3);
		}
		else if ((!issliceface)&&(noHemi))
		{	mesh.faces[nf].setSmGroup(smooth?1:0);
			mesh.faces[nf].setMatID(1); // mjm - 5.5.99 - rollback change - should be fixed in later release
//			mesh.faces[nf].setMatID(0); // mjm - 3.2.99 - was set to 1
		}
		else if (!noHemi)
		{	mesh.faces[nf].setSmGroup(smooth?2:0);
			mesh.faces[nf].setMatID(0); // mjm - 5.5.99 - rollback change - should be fixed in later release
//			mesh.faces[nf].setMatID(1); // mjm - 3.2.99 - was set to 0
		}
//		else
//		{	mesh.faces[nf].setSmGroup(0);
//			mesh.faces[nf].setMatID(noHemi?1:0); // mjm - 5.5.99 - rollback change - should be fixed in later release
//			mesh.faces[nf].setMatID(noHemi?0:1); // mjm - 3.2.99 - was commented out but set to 1:0
//		}

		mesh.faces[nf].setVerts(na, nb, nc);

		nf++;
		}

	// Put the flat part of the hemisphere at z=0
	if (recenter) {
		float shift = (float)cos(hemi) * radius;
		for (ix=0; ix<mesh.getNumVerts(); ix++) {
			mesh.verts[ix].z -= shift;
			}
		}

	if (genUVs) {
		int tvsegs=segs;
		int tvpts=(doPie?segs+1:segs); 
		int ntverts = (rows+2)*(tvpts+1);
//		if (doPie) {ntverts-=6; if (weirdpt) ntverts-3;}
		mesh.setNumTVerts(ntverts);
		mesh.setNumTVFaces(nfaces);
		nv = 0;
		delta  = basedelta;  // make the texture squash too
		alt = 0.0f; // = delta;
		int dsegs=(doPie?3:0),midsegs=tvpts-dsegs,m1=midsegs+1,t1=tvpts+1;
		for(ix=0; ix < rows+2; ix++) {		
		//	if (!noHemi && ix==rows) alt = hemi;		
			secang = 0.0f; //angle;
			float yang=1.0f-alt/PI;
			for(jx=0; jx <= midsegs; ++jx) {
				mesh.setTVert(nv++, secang/TWOPI, yang, 0.0f);
				secang += delta2;
				}
			for (jx=0;jx<dsegs;jx++) mesh.setTVert(nv++,uval[jx],yang,0.0f);
			alt += delta;		
			}

		nf = 0;dsegs=(doPie?2:0),midsegs=segs-dsegs;
		// Make top conic cap
		for(ix=0; ix<midsegs; ++ix) {
			mesh.tvFace[nf++].setTVerts(ix,ix+t1,ix+t1+1);
		} ix=midsegs+1;int topv=ix+1;
		for (jx=0;jx<dsegs;jx++) 
		{ mesh.tvFace[nf++].setTVerts(topv,ix+t1,ix+t1+1);ix++;
		}
		int cpt;
		/* Make midsection */
		for(ix=1; ix<rows; ++ix) {
			cpt=ix*t1;
			for(kx=0; kx<tvsegs; ++kx) {
				if (kx==midsegs) cpt++;
				na = cpt+kx;
				nb = na+t1;
				nc = nb+1;
				nd = na+1;
				assert(nc<ntverts);
				assert(nd<ntverts);
				mesh.tvFace[nf++].setTVerts(na,nb,nc);
				mesh.tvFace[nf++].setTVerts(na,nc,nd);
				}
			}
		// Make bottom conic cap
		int lastv=rows*t1,jx=lastv+t1;
		if (weirdpt) dsegs=0;
		int j1;
		for (j1=lastv; j1<lastv+midsegs; j1++) {
			mesh.tvFace[nf++].setTVerts(jx,j1+1,j1);jx++;
			}
		j1=lastv+midsegs+1;topv=j1+t1+1;
		for (ix=0;ix<dsegs;ix++) 
		{ mesh.tvFace[nf++].setTVerts(topv,j1+1,j1);j1++;
		}
		assert(nf==nfaces);
		}
	else {
		mesh.setNumTVerts(0);
		mesh.setNumTVFaces(0);
		}

	mesh.InvalidateTopologyCache();
	}

// Triangular patch layout:
//
//   A---> ac ----- ca <---C
//   |                    / 
//   |                  /
//   v    i1    i3    /
//   ab            cb
//
//   |           /
//   |    i2   /
// 
//   ba     bc
//   ^     /
//   |   /
//   | /
//   B
//
// vertices ( a b c d ) are in counter clockwise order when viewed from 
// outside the surface

// Vector length for unit circle
#define CIRCLE_VECTOR_LENGTH 0.5517861843f

static void BuildSpherePatch(PatchMesh& amesh, float radius, int smooth, BOOL textured)
	{
	Point3 p;	
	int np=0,nv=0;
	
	int nverts = 6;
	int nvecs = 48;
	int npatches = 8;
	amesh.setNumVerts(nverts);
	amesh.setNumTVerts(textured ? 13 : 0);
	amesh.setNumVecs(nvecs);
	amesh.setNumPatches(npatches);
	amesh.setNumTVPatches(textured ? npatches : 0);

	Point3 v0(0.0f, 0.0f, radius);		// Top
	Point3 v1(0.0f, 0.0f, -radius);		// Bottom
	Point3 v2(0.0f, -radius, 0.0f);		// Front
	Point3 v3(radius, 0.0f, 0.0f);		// Right
	Point3 v4(0.0f, radius, 0.0f);		// Back
	Point3 v5(-radius, 0.0f, 0.0f);		// Left

	// Create the vertices.
	amesh.verts[0].flags = PVERT_COPLANAR;
	amesh.verts[1].flags = PVERT_COPLANAR;
	amesh.verts[2].flags = PVERT_COPLANAR;
	amesh.verts[3].flags = PVERT_COPLANAR;
	amesh.verts[4].flags = PVERT_COPLANAR;
	amesh.verts[5].flags = PVERT_COPLANAR;
	amesh.setVert(0, v0);
	amesh.setVert(1, v1);
	amesh.setVert(2, v2);
	amesh.setVert(3, v3);
	amesh.setVert(4, v4);
	amesh.setVert(5, v5);

	if(textured) {
		amesh.setTVert(0, UVVert(0.125f,1.0f,0.0f));
		amesh.setTVert(1, UVVert(0.375f,1.0f,0.0f));
		amesh.setTVert(2, UVVert(0.625f,1.0f,0.0f));
		amesh.setTVert(3, UVVert(0.875f,1.0f,0.0f));
		amesh.setTVert(4, UVVert(0.0f,0.5f,0.0f));
		amesh.setTVert(5, UVVert(0.25f,0.5f,0.0f));
		amesh.setTVert(6, UVVert(0.5f,0.5f,0.0f));
		amesh.setTVert(7, UVVert(0.75f,0.5f,0.0f));
		amesh.setTVert(8, UVVert(1.0f,0.5f,0.0f));
		amesh.setTVert(9, UVVert(0.125f,0.0f,0.0f));
		amesh.setTVert(10, UVVert(0.375f,0.0f,0.0f));
		amesh.setTVert(11, UVVert(0.625f,0.0f,0.0f));
		amesh.setTVert(12, UVVert(0.875f,0.0f,0.0f));

		amesh.getTVPatch(0).setTVerts(3,7,8);
		amesh.getTVPatch(1).setTVerts(0,4,5);
		amesh.getTVPatch(2).setTVerts(1,5,6);
		amesh.getTVPatch(3).setTVerts(2,6,7);
		amesh.getTVPatch(4).setTVerts(12,8,7);
		amesh.getTVPatch(5).setTVerts(9,5,4);
		amesh.getTVPatch(6).setTVerts(10,6,5);
		amesh.getTVPatch(7).setTVerts(11,7,6);
		}

	// Create the edge vectors
	float vecLen = CIRCLE_VECTOR_LENGTH * radius;
	Point3 xVec(vecLen, 0.0f, 0.0f);
	Point3 yVec(0.0f, vecLen, 0.0f);
	Point3 zVec(0.0f, 0.0f, vecLen);
	amesh.setVec(0, v0 - yVec);
	amesh.setVec(2, v0 + xVec);
	amesh.setVec(4, v0 + yVec);
	amesh.setVec(6, v0 - xVec);
	amesh.setVec(8, v1 - yVec);
	amesh.setVec(10, v1 + xVec);
	amesh.setVec(12, v1 + yVec);
	amesh.setVec(14, v1 - xVec);
	amesh.setVec(9, v2 - zVec);
	amesh.setVec(16, v2 + xVec);
	amesh.setVec(1, v2 + zVec);
	amesh.setVec(23, v2 - xVec);
	amesh.setVec(11, v3 - zVec);
	amesh.setVec(18, v3 + yVec);
	amesh.setVec(3, v3 + zVec);
	amesh.setVec(17, v3 - yVec);
	amesh.setVec(13, v4 - zVec);
	amesh.setVec(20, v4 - xVec);
	amesh.setVec(5, v4 + zVec);
	amesh.setVec(19, v4 + xVec);
	amesh.setVec(15, v5 - zVec);
	amesh.setVec(22, v5 - yVec);
	amesh.setVec(7, v5 + zVec);
	amesh.setVec(21, v5 + yVec);
	
	// Create the patches
	amesh.MakeTriPatch(np++, 0, 0, 1, 2, 16, 17, 3, 3, 2, 24, 25, 26, smooth);
	amesh.MakeTriPatch(np++, 0, 2, 3, 3, 18, 19, 4, 5, 4, 27, 28, 29, smooth);
	amesh.MakeTriPatch(np++, 0, 4, 5, 4, 20, 21, 5, 7, 6, 30, 31, 32, smooth);
	amesh.MakeTriPatch(np++, 0, 6, 7, 5, 22, 23, 2, 1, 0, 33, 34, 35, smooth);
	amesh.MakeTriPatch(np++, 1, 10, 11, 3, 17, 16, 2, 9, 8, 36, 37, 38, smooth);
	amesh.MakeTriPatch(np++, 1, 12, 13, 4, 19, 18, 3, 11, 10, 39, 40, 41, smooth);
	amesh.MakeTriPatch(np++, 1, 14, 15, 5, 21, 20, 4, 13, 12, 42, 43, 44, smooth);
	amesh.MakeTriPatch(np++, 1, 8, 9, 2, 23, 22, 5, 15, 14, 45, 46, 47, smooth);

	// Create all the interior vertices and make them non-automatic
	float chi = 0.5893534f * radius;

	int interior = 24;
	amesh.setVec(interior++, Point3(chi, -chi, radius)); 
	amesh.setVec(interior++, Point3(chi, -radius, chi)); 
	amesh.setVec(interior++, Point3(radius, -chi, chi)); 

	amesh.setVec(interior++, Point3(chi, chi, radius)); 
	amesh.setVec(interior++, Point3(radius, chi, chi)); 
	amesh.setVec(interior++, Point3(chi, radius, chi)); 

	amesh.setVec(interior++, Point3(-chi, chi, radius)); 
	amesh.setVec(interior++, Point3(-chi, radius, chi)); 
	amesh.setVec(interior++, Point3(-radius, chi, chi)); 

	amesh.setVec(interior++, Point3(-chi, -chi, radius)); 
	amesh.setVec(interior++, Point3(-radius, -chi, chi)); 
	amesh.setVec(interior++, Point3(-chi, -radius, chi)); 

	amesh.setVec(interior++, Point3(chi, -chi, -radius)); 
	amesh.setVec(interior++, Point3(radius, -chi, -chi)); 
	amesh.setVec(interior++, Point3(chi, -radius, -chi)); 

	amesh.setVec(interior++, Point3(chi, chi, -radius)); 
	amesh.setVec(interior++, Point3(chi, radius, -chi)); 
	amesh.setVec(interior++, Point3(radius, chi, -chi)); 

	amesh.setVec(interior++, Point3(-chi, chi, -radius)); 
	amesh.setVec(interior++, Point3(-radius, chi, -chi)); 
	amesh.setVec(interior++, Point3(-chi, radius, -chi)); 

	amesh.setVec(interior++, Point3(-chi, -chi, -radius)); 
	amesh.setVec(interior++, Point3(-chi, -radius, -chi)); 
	amesh.setVec(interior++, Point3(-radius, -chi, -chi)); 

	for(int i = 0; i < 8; ++i)
		amesh.patches[i].SetAuto(FALSE);

	// Finish up patch internal linkages (and bail out if it fails!)
	assert(amesh.buildLinkages());

	// Calculate the interior bezier points on the PatchMesh's patches
	amesh.computeInteriors();
	amesh.InvalidateGeomCache();
	}


#ifndef NO_NURBS

Object *
BuildNURBSSphere(float radius, float hemi, BOOL recenter, BOOL genUVs, BOOL doPie, float pie1, float pie2)
{
	NURBSSet nset;

	Point3 center(0,0,0);
	Point3 northAxis(0,0,1);
	Point3 refAxis(0,-1,0);

	if (recenter)
		center = Point3(0.0, 0.0, -cos((1.0f-hemi) * PI) * radius);

	NURBSCVSurface *surf = new NURBSCVSurface();
	nset.AppendObject(surf);
	surf->SetGenerateUVs(genUVs);

	surf->SetTextureUVs(0, 0, Point2(0.0f, hemi));
	surf->SetTextureUVs(0, 1, Point2(0.0f, 1.0f));
	surf->SetTextureUVs(0, 2, Point2(1.0f, hemi));
	surf->SetTextureUVs(0, 3, Point2(1.0f, 1.0f));

	surf->FlipNormals(TRUE);
	surf->Renderable(TRUE);
	char bname[80];
	char sname[80];
	strcpy(bname, GetString(IDS_RB_SPHERE));
	sprintf(sname, "%s%s", bname, GetString(IDS_CT_SURF));
	surf->SetName(sname);

	float startAngleU = 0.0f;
	float endAngleU = TWOPI;
	pie1 += HALFPI;
	pie2 += HALFPI;
	if (doPie && pie1 != pie2) {
		float sweep = TWOPI - (pie2-pie1);
		if (sweep > TWOPI) sweep -= TWOPI;
		refAxis = Point3(Point3(1,0,0) * RotateZMatrix(pie2));
		endAngleU = sweep;
		if (fabs(endAngleU) < 1e-5) endAngleU = TWOPI;
	}
	if (hemi == 0.0f && (!doPie || endAngleU == TWOPI)) {
		GenNURBSSphereSurface(radius, center, northAxis, Point3(0,-1,0),
						-PI, PI, -HALFPI, HALFPI,
						FALSE, *surf);
	} else if (hemi > 0.0f && (!doPie || endAngleU == TWOPI)) {
		GenNURBSSphereSurface(radius, center, northAxis, Point3(0,-1,0),
						-PI, PI, -HALFPI + (hemi * PI), HALFPI,
						FALSE, *surf);
		// now cap it
		NURBSCapSurface *cap0 = new NURBSCapSurface();
		nset.AppendObject(cap0);
		cap0->SetGenerateUVs(genUVs);
		cap0->SetParent(0);
		cap0->SetEdge(0);
		cap0->FlipNormals(TRUE);
		cap0->Renderable(TRUE);
		char sname[80];
		sprintf(sname, "%s%s%02d", bname, GetString(IDS_CT_CAP), 0);
		cap0->SetName(sname);
	} else {
		float startAngleV = -HALFPI + (hemi * PI);
		float endAngleV = HALFPI;
		GenNURBSSphereSurface(radius, center, northAxis, refAxis,
						startAngleU, endAngleU, startAngleV, endAngleV,
						TRUE, *surf);
#define F(s1, s2, s1r, s1c, s2r, s2c) \
		fuse.mSurf1 = (s1); \
		fuse.mSurf2 = (s2); \
		fuse.mRow1 = (s1r); \
		fuse.mCol1 = (s1c); \
		fuse.mRow2 = (s2r); \
		fuse.mCol2 = (s2c); \
		nset.mSurfFuse.Append(1, &fuse);

		NURBSFuseSurfaceCV fuse;

		// pole(s)
		for (int f = 1; f < surf->GetNumVCVs(); f++) {
			if (hemi <= 0.0f) {
				// south pole
				F(0, 0, 0, 0, 0, f);
			}
			//north pole
			F(0, 0, surf->GetNumUCVs()-1, 0, surf->GetNumUCVs()-1, f);
		}

		NURBSCVSurface *s0 = (NURBSCVSurface*)nset.GetNURBSObject(0);
		int numU, numV;
		s0->GetNumCVs(numU, numV);


		if (doPie && endAngleU > 0.0f && endAngleU < TWOPI) {
			// next the two pie slices
			for (int c = 0; c < 2; c++) {
				NURBSCVSurface *s = new NURBSCVSurface();
				nset.AppendObject(s);
				// we'll be cubic in on direction and match the sphere in the other
				s->SetUOrder(s0->GetUOrder());
				int numKnots = s0->GetNumUKnots();
				s->SetNumUKnots(numKnots);
				for (int i = 0; i < numKnots; i++)
					s->SetUKnot(i, s0->GetUKnot(i));

				s->SetVOrder(4);
				s->SetNumVKnots(8);
				for (i = 0; i < 4; i++) {
					s->SetVKnot(i, 0.0);
					s->SetVKnot(i+4, 1.0);
				}

				s->SetNumCVs(numU, 4);
				for (int v = 0; v < 4; v++) {
					for (int u = 0; u < numU; u++) {
						if (v == 0) { // outside edge
							if (c == 0) {
								s->SetCV(u, v, *s0->GetCV(u, 0));
								F(0, 1, u, 0, u, v);
							} else {
								s->SetCV(u, v, *s0->GetCV(u, numV-1));
								F(0, 2, u, numV-1, u, v);
							}
						} else
						if (v == 3) { // center axis
							Point3 p(0.0f, 0.0f, s0->GetCV(u, 0)->GetPosition(0).z);
							NURBSControlVertex ncv;
							ncv.SetPosition(0, p);
							ncv.SetWeight(0, 1.0f);
							s->SetCV(u, v, ncv);
							F(1, c+1, u, 3, u, v);
						} else {
							Point3 center(0.0f, 0.0f, s0->GetCV(u, 0)->GetPosition(0).z);
							Point3 edge;
							if (c == 0)
								edge = Point3(s0->GetCV(u, 0)->GetPosition(0));
							else
								edge = Point3(s0->GetCV(u, numV-1)->GetPosition(0));
							NURBSControlVertex ncv;
							ncv.SetPosition(0, center + ((edge - center)*(float)v/3.0f));
							ncv.SetWeight(0, 1.0f);
							s->SetCV(u, v, ncv);
						}
					}
				}
				s->SetGenerateUVs(genUVs);

				s->SetTextureUVs(0, 0, Point2(0.0f, 0.0f));
				s->SetTextureUVs(0, 1, Point2(0.0f, 1.0f));
				s->SetTextureUVs(0, 2, Point2(1.0f, 0.0f));
				s->SetTextureUVs(0, 3, Point2(1.0f, 1.0f));

				if (c == 0)
					s->FlipNormals(FALSE);
				else
					s->FlipNormals(TRUE);
				s->Renderable(TRUE);
				sprintf(sname, "%s%s%02d", bname, GetString(IDS_CT_SLICE), c+1);
				s->SetName(sname);
			}
		}

		if (hemi > 0.0f) {
			// Cap -- we will always have slices since we
			// handle the non-slice cases with cap surfaces

			NURBSCVSurface *s = new NURBSCVSurface();
			s->SetGenerateUVs(genUVs);

			s->SetTextureUVs(0, 0, Point2(1.0f, 1.0f));
			s->SetTextureUVs(0, 1, Point2(0.0f, 1.0f));
			s->SetTextureUVs(0, 2, Point2(1.0f, 0.0f));
			s->SetTextureUVs(0, 3, Point2(0.0f, 0.0f));

			s->FlipNormals(TRUE);
			s->Renderable(TRUE);
			sprintf(sname, "%s%s%01", bname, GetString(IDS_CT_CAP));
			s->SetName(sname);
			int cap = nset.AppendObject(s);

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

			s->SetNumCVs(4, numV);

			Point3 bot;
			if (recenter)
				bot = Point3(0,0,0);
			else
				bot = Point3(0.0, 0.0, cos((1.0-hemi) * PI)*radius);
			for (int v = 0; v < numV; v++) {
				Point3 edge = s0->GetCV(0, v)->GetPosition(0);
				double w = s0->GetCV(0, v)->GetWeight(0);
				for (int u = 0; u < 4; u++) {
					NURBSControlVertex ncv;
					ncv.SetPosition(0, bot + ((edge - bot)*((float)u/3.0f)));
					ncv.SetWeight(0, w);
					s->SetCV(u, v, ncv);
					if (u == 3) {
						// fuse the cap to the sphere
						F(cap, 0, 3, v, 0, v);
					}
					if (u == 1 || u == 2) {
						// fuse the ends to the slices
						if (v == 0) {
							F(cap, 1, u, v, 0, u);
						}
						if (v == numV-1) {
							F(cap, 2, u, v, 0, u);
						}
					}
				}

				if (v > 0) {
					// fuse the center degeneracy
					F(cap, cap, 0, 0, 0, v);
				}
			}
		}
	}

	Matrix3 mat;
	mat.IdentityMatrix();
	Object *ob = CreateNURBSObject(NULL, &nset, mat);
	return ob;
}


#endif

Object* SphereObject::ConvertToType(TimeValue t, Class_ID obtype)
	{
#ifndef NO_PATCHES
	if (obtype == patchObjectClassID) {
		Interval valid = FOREVER;
		float radius;
		int smooth, genUVs;
		pblock->GetValue(PB_RADIUS,t,radius,valid);
		pblock->GetValue(PB_SMOOTH,t,smooth,valid);	
		pblock->GetValue(PB_GENUVS,t,genUVs,valid);
		PatchObject *ob = new PatchObject();
		BuildSpherePatch(ob->patch,radius,smooth,genUVs);
		ob->SetChannelValidity(TOPO_CHAN_NUM,valid);
		ob->SetChannelValidity(GEOM_CHAN_NUM,valid);
		ob->UnlockObject();
		return ob;
	} 
#endif
#ifndef NO_NURBS
    if (obtype == EDITABLE_SURF_CLASS_ID) {
		Interval valid = FOREVER;
		float radius, hemi;
		int recenter, genUVs;
		float pie1, pie2;
		BOOL doPie;
		pblock->GetValue(PB_RADIUS,t,radius,valid);
		pblock->GetValue(PB_HEMI,t,hemi,valid);	
		pblock->GetValue(PB_RECENTER,t,recenter,valid);
		pblock->GetValue(PB_GENUVS,t,genUVs,valid);
		pblock->GetValue(PB_SLICEFROM,t,pie1,ivalid);
		pblock->GetValue(PB_SLICETO,t,pie2,ivalid);	
		pblock->GetValue(PB_SLICEON,t,doPie,ivalid);	
		Object *ob = BuildNURBSSphere(radius, hemi, recenter,genUVs, doPie, pie1, pie2);
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
		float radius, hemi;
		int recenter, genUVs, sides;
		float pie1, pie2;
		BOOL doPie;
		pblock->GetValue(PB_RADIUS,t,radius,valid);
		pblock->GetValue(PB_HEMI,t,hemi,valid);	
		pblock->GetValue(PB_SEGS,t,sides,valid);
		pblock->GetValue(PB_RECENTER,t,recenter,valid);
		pblock->GetValue(PB_GENUVS,t,genUVs,valid);
		pblock->GetValue(PB_SLICEFROM,t,pie1,ivalid);
		pblock->GetValue(PB_SLICETO,t,pie2,ivalid);	
		pblock->GetValue(PB_SLICEON,t,doPie,ivalid);	
		int smooth;
		pblock->GetValue(PB_SMOOTH,t,smooth,valid);
		if (radius < 0.0f) radius = 0.0f;
		Object* solid = (Object*)CreateInstance(GEOMOBJECT_CLASS_ID, GENERIC_AMSOLID_CLASS_ID);
		assert(solid);
		if(solid)
		{
			IGeomImp* cacheptr = (IGeomImp*)(solid->GetInterface(I_GEOMIMP));
			assert(cacheptr);
			if(cacheptr)
			{
				bool res = cacheptr->createSphere(radius, sides, smooth);
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

int SphereObject::CanConvertToType(Class_ID obtype)
	{
	if(obtype==defObjectClassID ||	obtype==triObjectClassID) return 1;
#ifndef NO_PATCHES
    if(obtype == patchObjectClassID) return 1;
#endif
#ifndef NO_NURBS
    if(obtype == EDITABLE_SURF_CLASS_ID) return 1;
#endif
#ifdef DESIGN_VER
	if(obtype == GENERIC_AMSOLID_CLASS_ID) return 1;
#endif
	return SimpleObject::CanConvertToType(obtype);
	}


void SphereObject::GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist)
{
    Object::GetCollapseTypes(clist, nlist);
#ifndef NO_NURBS
    Class_ID id = EDITABLE_SURF_CLASS_ID;
    TSTR *name = new TSTR(GetString(IDS_SM_NURBS_SURFACE));
    clist.Append(1,&id);
    nlist.Append(1,&name);
#endif
}

class SphereObjCreateCallBack : public CreateMouseCallBack {
	IPoint2 sp0;
	SphereObject *ob;
	Point3 p0;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
		void SetObj(SphereObject *obj) {ob = obj;}
	};

int SphereObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
	float r;
	Point3 p1,center;

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
			case 0:  // only happens with MOUSE_POINT msg
				ob->pblock->SetValue(PB_RADIUS,0,0.0f);
				ob->suspendSnap = TRUE;				
				sp0 = m;
				#ifdef _3D_CREATE	
					p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				#else	
					p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				#endif
				mat.SetTrans(p0);
				break;
			case 1:
				mat.IdentityMatrix();
				//mat.PreRotateZ(HALFPI);
				#ifdef _3D_CREATE	
					p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				#else	
					p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				#endif
				if (ob->dlgCreateMeth) {
					r = Length(p1-p0);
					mat.SetTrans(p0);
					}
				else {
					center = (p0+p1)/float(2);
					mat.SetTrans(center);
					r = Length(center-p0);
					} 
				ob->pblock->SetValue(PB_RADIUS,0,r);
				ob->pmapParam->Invalidate();

				if (flags&MOUSE_CTRL) {
					float ang = (float)atan2(p1.y-p0.y,p1.x-p0.x);					
					mat.PreRotateZ(ob->ip->SnapAngle(ang));
					}

				if (msg==MOUSE_POINT) {
					ob->suspendSnap = FALSE;
					return (Length(m-sp0)<3 || Length(p1-p0)<0.1f)?CREATE_ABORT:CREATE_STOP;
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

static SphereObjCreateCallBack sphereCreateCB;

CreateMouseCallBack* SphereObject::GetCreateMouseCallBack() 
	{
	sphereCreateCB.SetObj(this);
	return(&sphereCreateCB);
	}


BOOL SphereObject::OKtoDisplay(TimeValue t) 
	{
	float radius;
	pblock->GetValue(PB_RADIUS,t,radius,FOREVER);
	if (radius==0.0f) return FALSE;
	else return TRUE;
	}



// From ParamArray
BOOL SphereObject::SetValue(int i, TimeValue t, int v) 
	{
	switch (i) {
		case PB_CREATEMETHOD: dlgCreateMeth = v; break;
		}		
	return TRUE;
	}

BOOL SphereObject::SetValue(int i, TimeValue t, float v)
	{
	switch (i) {				
		case PB_TI_RADIUS: crtRadius = v; break;
		}	
	return TRUE;
	}

BOOL SphereObject::SetValue(int i, TimeValue t, Point3 &v) 
	{
	switch (i) {
		case PB_TI_POS: crtPos = v; break;
		}		
	return TRUE;
	}

BOOL SphereObject::GetValue(int i, TimeValue t, int &v, Interval &ivalid) 
	{
	switch (i) {
		case PB_CREATEMETHOD: v = dlgCreateMeth; break;
		}
	return TRUE;
	}

BOOL SphereObject::GetValue(int i, TimeValue t, float &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_RADIUS: v = crtRadius; break;
		}
	return TRUE;
	}

BOOL SphereObject::GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_POS: v = crtPos; break;		
		}
	return TRUE;
	}


// From GeomObject
int SphereObject::IntersectRay(
		TimeValue t, Ray& ray, float& at, Point3& norm)
	{
	int smooth, recenter;
	pblock->GetValue(PB_SMOOTH,t,smooth,FOREVER);
	pblock->GetValue(PB_RECENTER,t,recenter,FOREVER);	
	float hemi;
	pblock->GetValue(PB_HEMI,t,hemi,FOREVER);
	if (!smooth || hemi!=0.0f || recenter) {
		return SimpleObject::IntersectRay(t,ray,at,norm);
		}	
	
	float r;
	float a, b, c, ac4, b2, at1, at2;
	float root;
	BOOL neg1, neg2;

	pblock->GetValue(PB_RADIUS,t,r,FOREVER);

	a = DotProd(ray.dir,ray.dir);
	b = DotProd(ray.dir,ray.p) * 2.0f;
	c = DotProd(ray.p,ray.p) - r*r;
	
	ac4 = 4.0f * a * c;
	b2 = b*b;

	if (ac4 > b2) return 0;

	// We want the smallest positive root
	root = float(sqrt(b2-ac4));
	at1 = (-b + root) / (2.0f * a);
	at2 = (-b - root) / (2.0f * a);
	neg1 = at1<0.0f;
	neg2 = at2<0.0f;
	if (neg1 && neg2) return 0;
	else
	if (neg1 && !neg2) at = at2;
	else 
	if (!neg1 && neg2) at = at1;
	else
	if (at1<at2) at = at1;
	else at = at2;
	
	norm = Normalize(ray.p + at*ray.dir);

	return 1;
	}

void SphereObject::InvalidateUI() 
	{
	if (pmapParam) pmapParam->Invalidate();
	}

ParamDimension *SphereObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_RADIUS:
			return stdWorldDim;			
		case PB_HEMI:
			return stdNormalizedDim;
		case PB_SEGS:
			return stdSegmentsDim;			
		case PB_SMOOTH:
			return stdNormalizedDim;			
		case PB_SLICEFROM: return stdAngleDim;
		case PB_SLICETO: return stdAngleDim;
		default:
			return defaultDim;
		}
	}

TSTR SphereObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_RADIUS:
			return TSTR(GetString(IDS_RB_RADIUS));			
		case PB_HEMI:
			return GetString(IDS_RB_HEMISPHERE);
		case PB_SEGS:
			return TSTR(GetString(IDS_RB_SEGS));			
		case PB_SMOOTH:
			return TSTR(GetString(IDS_RB_SMOOTH));			
		case PB_SLICEON: return TSTR(GetString(IDS_AP_SLICEON));
		case PB_SLICEFROM: return TSTR(GetString(IDS_AP_SLICEFROM));
		case PB_SLICETO: return TSTR(GetString(IDS_AP_SLICETO));
		default:
			return TSTR(_T(""));
		}
	}

RefTargetHandle SphereObject::Clone(RemapDir& remap) 
	{
	SphereObject* newob = new SphereObject();	
	newob->ReplaceReference(0,pblock->Clone(remap));
	newob->ivalid.SetEmpty();	
	BaseClone(this, newob, remap);
	return(newob);
	}

