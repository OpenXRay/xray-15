/**********************************************************************
 *<
	FILE: torus.cpp

	DESCRIPTION:  Defines a Test Object Class

	CREATED BY: Dan Silva
	MODIFIED BY: Rolf Berteig

	HISTORY: created 13 September 1994

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "prim.h"

#ifndef NO_OBJECT_STANDARD_PRIMITIVES

#include "iparamm.h"
#include "Simpobj.h"
#include "surf_api.h"

class TorusObject : public SimpleObject, public IParamArray {
	public:
		// Class vars
		static IParamMap *pmapCreate;
		static IParamMap *pmapTypeIn;
		static IParamMap *pmapParam;
		static IObjParam *ip;
		static int dlgSegments, dlgSides;		
		static int dlgCreateMeth;
		static int dlgSmooth;	
		static float dlgRadius2;	
		static Point3 crtPos;		
		static float crtRadius1;
		static float crtRadius2;	
	
		TorusObject();
		
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
		TCHAR *GetObjectName() { return GetString(IDS_RB_TORUS); }

		// Animatable methods		
		void DeleteThis() { delete this; }
		Class_ID ClassID() { return Class_ID( TORUS_CLASS_ID,0); }  
		
		// From ref
		RefTargetHandle Clone(RemapDir& remap = NoRemap());		
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
	};

// segments  = sides
// lsegments = segments

#define MIN_SEGMENTS	3
#define MAX_SEGMENTS	200

#define MIN_SIDES		3
#define MAX_SIDES		200

#define MIN_RADIUS		float(0)
#define MAX_RADIUS		float( 1.0E30)
#define MIN_PIESLICE	float(-1.0E30)
#define MAX_PIESLICE	float( 1.0E30)

#define DEF_SEGMENTS 	24
#define DEF_SIDES		12

#define DEF_RADIUS		(0.1f)
#define DEF_RADIUS2   	(10.0f)

#define SMOOTH_STRIPES	3
#define SMOOTH_ON		2
#define SMOOTH_SIDES	1
#define SMOOTH_OFF		0


//--- ClassDescriptor and class vars ---------------------------------

class TorusClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new TorusObject; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_TORUS_CLASS); }
	SClass_ID		SuperClassID() { return GEOMOBJECT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(TORUS_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_PRIMITIVES); }
	void            ResetClassParams(BOOL fileReset);
	};

static TorusClassDesc torusDesc;

ClassDesc* GetTorusDesc() { return &torusDesc; }

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// class variable for sphere class.
IObjParam *TorusObject::ip         = NULL;
int TorusObject::dlgSegments       = DEF_SEGMENTS;
int TorusObject::dlgSides          = DEF_SIDES;
int TorusObject::dlgCreateMeth     = 1; // create_radius
int TorusObject::dlgSmooth         = SMOOTH_ON;
float TorusObject::dlgRadius2      = DEF_RADIUS2;
IParamMap *TorusObject::pmapCreate = NULL;
IParamMap *TorusObject::pmapTypeIn = NULL;
IParamMap *TorusObject::pmapParam  = NULL;
Point3 TorusObject::crtPos         = Point3(0,0,0);
float TorusObject::crtRadius1      = 0.0f;
float TorusObject::crtRadius2      = DEF_RADIUS2;

void TorusClassDesc::ResetClassParams(BOOL fileReset)
	{
	TorusObject::dlgSegments    = DEF_SEGMENTS;
	TorusObject::dlgSides       = DEF_SIDES;
	TorusObject::dlgCreateMeth  = 1; // create_radius
	TorusObject::dlgSmooth      = SMOOTH_ON;
	TorusObject::dlgRadius2     = DEF_RADIUS2;
	TorusObject::crtPos         = Point3(0,0,0);
	TorusObject::crtRadius1     = 0.0f;
	TorusObject::crtRadius2     = DEF_RADIUS2;
	}


//--- Parameter map/block descriptors -------------------------------

// Parameter map indices
#define PB_RADIUS		0
#define PB_RADIUS2		1
#define PB_ROTATION		2
#define PB_TWIST		3
#define PB_SEGMENTS		4
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
		IDC_RADIUS2,IDC_RAD2SPINNER,
		MIN_RADIUS,MAX_RADIUS,
		SPIN_AUTOSCALE),	
	};
#define TYPEINDESC_LENGH 3


//
//
// Parameters

static int smoothIDs[] = {IDC_SMOOTH_NONE,IDC_SMOOTH_SIDES,IDC_SMOOTH_ALL,IDC_SMOOTH_STRIPES};

static ParamUIDesc descParam[] = {
	// Radius
	ParamUIDesc(
		PB_RADIUS,
		EDITTYPE_UNIVERSE,
		IDC_RADIUS1,IDC_RADSPINNER1,
		MIN_RADIUS,MAX_RADIUS,
		SPIN_AUTOSCALE),	

	// Radius
	ParamUIDesc(
		PB_RADIUS2,
		EDITTYPE_UNIVERSE,
		IDC_RADIUS2,IDC_RAD2SPINNER,
		MIN_RADIUS,MAX_RADIUS,
		SPIN_AUTOSCALE),	

	// Rotation
	ParamUIDesc(
		PB_ROTATION,
		EDITTYPE_FLOAT,
		IDC_TORUS_ROT,IDC_TORUS_ROTSPIN,
		MIN_PIESLICE,MAX_PIESLICE,
		0.5f,
		stdAngleDim),	

	// Twist
	ParamUIDesc(
		PB_TWIST,
		EDITTYPE_FLOAT,
		IDC_TORUS_TWIST,IDC_TORUS_TWISTSPIN,
		MIN_PIESLICE,MAX_PIESLICE,
		0.5f,
		stdAngleDim),	

	// Segments
	ParamUIDesc(
		PB_SEGMENTS,
		EDITTYPE_INT,
		IDC_SEGMENTS,IDC_SEGSPINNER,
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
	ParamUIDesc(PB_SMOOTH,TYPE_RADIO,smoothIDs,4),

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
	{ TYPE_INT, NULL, TRUE, 2 },
	{ TYPE_INT, NULL, TRUE, 3 },
	{ TYPE_INT, NULL, TRUE, 4 } };

static ParamBlockDescID descVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 8 },
	{ TYPE_INT, NULL, TRUE, 2 },
	{ TYPE_INT, NULL, TRUE, 3 },
	{ TYPE_INT, NULL, TRUE, 4 },
	{ TYPE_INT, NULL, TRUE, 5 },
	{ TYPE_FLOAT, NULL, TRUE, 6 },
	{ TYPE_FLOAT, NULL, TRUE, 7 } };

static ParamBlockDescID descVer2[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 8 },
	{ TYPE_FLOAT, NULL, TRUE, 9 },
	{ TYPE_INT, NULL, TRUE, 2 },
	{ TYPE_INT, NULL, TRUE, 3 },
	{ TYPE_INT, NULL, TRUE, 4 },
	{ TYPE_INT, NULL, TRUE, 5 },
	{ TYPE_FLOAT, NULL, TRUE, 6 },
	{ TYPE_FLOAT, NULL, TRUE, 7 } };

static ParamBlockDescID descVer3[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 8 },
	{ TYPE_FLOAT, NULL, TRUE, 9 },
	{ TYPE_INT, NULL, TRUE, 2 },
	{ TYPE_INT, NULL, TRUE, 3 },
	{ TYPE_INT, NULL, TRUE, 4 },
	{ TYPE_INT, NULL, TRUE, 5 },
	{ TYPE_FLOAT, NULL, TRUE, 6 },
	{ TYPE_FLOAT, NULL, TRUE, 7 },
	{ TYPE_INT, NULL, FALSE, 10 } };

#define PBLOCK_LENGTH	11

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,5,0),
	ParamVersionDesc(descVer1,9,1),
	ParamVersionDesc(descVer2,10,2)
	};
#define NUM_OLDVERSIONS	3

// Current version
static ParamVersionDesc curVersion(descVer3,PBLOCK_LENGTH,3);
#define CURRENT_VERSION	3


//--- TypeInDlgProc --------------------------------

class TorusTypeInDlgProc : public ParamMapUserDlgProc {
	public:
		TorusObject *ob;

		TorusTypeInDlgProc(TorusObject *o) {ob=o;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

BOOL TorusTypeInDlgProc::DlgProc(
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
						ob->pblock->SetValue(PB_RADIUS,0,ob->crtRadius1);
						ob->pblock->SetValue(PB_RADIUS2,0,ob->crtRadius2);
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




class TorusParamDlgProc : public ParamMapUserDlgProc {
	public:
		TorusObject *so;
		HWND thishWnd;

		TorusParamDlgProc(TorusObject *s) {so=s;thishWnd=NULL;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void Update(TimeValue t);
		void DeleteThis() {delete this;}

		void TurnSpinner(HWND hWnd,int SpinNum,BOOL ison)
			{	
			ISpinnerControl *spin2 = GetISpinner(GetDlgItem(hWnd,SpinNum));
			if (ison) spin2->Enable();else spin2->Disable();
			ReleaseISpinner(spin2);

			};

	};
void TorusParamDlgProc::Update(TimeValue t)
{ if (!thishWnd) return;
  int ison;
  so->pblock->GetValue(PB_SLICEON,t,ison,FOREVER);
  TurnSpinner(thishWnd,IDC_PIESLICESPIN1,ison);
  TurnSpinner(thishWnd,IDC_PIESLICESPIN2,ison);

  EnableWindow(GetDlgItem(thishWnd,IDC_STATICFROM),ison);
  EnableWindow(GetDlgItem(thishWnd,IDC_STATICTO),ison);
}

BOOL TorusParamDlgProc::DlgProc(
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


//--- Torus methods -------------------------------


TorusObject::TorusObject()
	{
	SetAFlag(A_PLUGIN1);
	MakeRefByID(FOREVER, 0, CreateParameterBlock(descVer3, PBLOCK_LENGTH, CURRENT_VERSION));
	assert(pblock);

	pblock->SetValue(PB_SMOOTH,0,dlgSmooth);
	pblock->SetValue(PB_SEGMENTS,0,dlgSegments);
	pblock->SetValue(PB_SIDES,0,dlgSides);	
	pblock->SetValue(PB_RADIUS,0,crtRadius1);
	pblock->SetValue(PB_RADIUS2,0,crtRadius2);	
	pblock->SetValue(PB_GENUVS,0,TRUE);
	}

#define NEWMAP_CHUNKID	0x0100

IOResult TorusObject::Load(ILoad *iload) 
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

IOResult TorusObject::Save(ISave *isave)
	{
	if (TestAFlag(A_PLUGIN1)) {
		isave->BeginChunk(NEWMAP_CHUNKID);
		isave->EndChunk();
		}
 	return IO_OK;
	}

void TorusObject::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
	{
	SimpleObject::BeginEditParams(ip,flags,prev);
	this->ip = ip;

	if (pmapCreate && pmapParam && pmapTypeIn) {
		
		// Left over from last Torus ceated		
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
				MAKEINTRESOURCE(IDD_TORUSPARAM1),
				GetString(IDS_RB_CREATIONMETHOD),
				0);

			
			pmapTypeIn = CreateCPParamMap(
				descTypeIn,TYPEINDESC_LENGH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_TORUSPARAM3),
				GetString(IDS_RB_KEYBOARDENTRY),
				APPENDROLL_CLOSED);			
			}

		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_TORUSPARAM2),
			GetString(IDS_RB_PARAMETERS),
			0);
		}

	if(pmapTypeIn) {
		// A callback for the type in.
		pmapTypeIn->SetUserDlgProc(new TorusTypeInDlgProc(this));
		}

	if(pmapParam) {
		// A callback for the type in.
		pmapParam->SetUserDlgProc(new TorusParamDlgProc(this));
		}

	}
		
void TorusObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
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
	pblock->GetValue(PB_SMOOTH,ip->GetTime(),dlgSmooth,FOREVER);	
	pblock->GetValue(PB_RADIUS2,ip->GetTime(),dlgRadius2,FOREVER);		
	}


Point3 TorusObject::GetSurfacePoint(
		TimeValue t, float u, float v,Interval &iv)
	{
	float radius,radius2;
	pblock->GetValue(PB_RADIUS,t,radius,iv);
	pblock->GetValue(PB_RADIUS2,t,radius2,iv);	
	float ang = (1.0f-u)*TWOPI, ang2 = v*TWOPI;
	float sinang  = (float)sin(ang);
	float cosang  = (float)cos(ang);		
	float sinang2 = (float)sin(ang2);
	float cosang2 = (float)cos(ang2);
	float rt = radius+radius2*cosang2;
	Point3 p;
	p.x = rt*cosang;
	p.y = -rt*sinang;
	p.z = radius2*sinang2;
	return p;
	}

BOOL TorusObject::HasUVW() { 
	BOOL genUVs;
	Interval v;
	pblock->GetValue(PB_GENUVS, 0, genUVs, v);
	return genUVs; 
	}

void TorusObject::SetGenUVW(BOOL sw) {  
	if (sw==HasUVW()) return;
	pblock->SetValue(PB_GENUVS,0, sw);				
	}


void TorusObject::BuildMesh(TimeValue t)
	{
	Point3 p;
	int ix,na,nb,nc,nd,jx,kx;
	int nf=0,nv=0;
	float delta,ang;
	float delta2,ang2;
	int sides,segs,smooth;
	float radius,radius2, rotation;
	float sinang,cosang, sinang2,cosang2,rt;
	float twist, pie1, pie2, totalPie, startAng = 0.0f;
	int doPie  = TRUE;	
	int genUVs = TRUE;	

	// Start the validity interval at forever and widdle it down.
	ivalid = FOREVER;	
	pblock->GetValue(PB_RADIUS,t,radius,ivalid);
	pblock->GetValue(PB_RADIUS2,t,radius2,ivalid);
	pblock->GetValue(PB_ROTATION,t,rotation,ivalid);
	pblock->GetValue(PB_TWIST,t,twist,ivalid);
	pblock->GetValue(PB_SEGMENTS,t,segs,ivalid);
	pblock->GetValue(PB_SIDES,t,sides,ivalid);
	pblock->GetValue(PB_SMOOTH,t,smooth,ivalid);
	pblock->GetValue(PB_PIESLICE1,t,pie1,ivalid);
	pblock->GetValue(PB_PIESLICE2,t,pie2,ivalid);	
	pblock->GetValue(PB_SLICEON,t,doPie,ivalid);
	pblock->GetValue(PB_GENUVS,t,genUVs,ivalid);
	LimitValue( radius, MIN_RADIUS, MAX_RADIUS );
	LimitValue( radius2, MIN_RADIUS, MAX_RADIUS );
	LimitValue( segs, MIN_SEGMENTS, MAX_SEGMENTS );
	LimitValue( sides, MIN_SIDES, MAX_SIDES );	

    // Convert doPie to a 0 or 1 value since it is used in arithmetic below
    // Controllers can give it non- 0 or 1 values
    doPie = doPie ? 1 : 0;

	// We do the torus backwards from the cylinder
	pie1 = -pie1;
	pie2 = -pie2;

	// Make pie2 < pie1 and pie1-pie2 < TWOPI
	while (pie1 < pie2) pie1 += TWOPI;
	while (pie1 > pie2+TWOPI) pie1 -= TWOPI;
	if (pie1==pie2) totalPie = TWOPI;
	else totalPie = pie1-pie2;	
	
	if (doPie) {
		segs++; //*** O.Z. fix for bug 240436 
		delta    = totalPie/(float)(segs-1);
		startAng = pie2;
	} else {
		delta = (float)2.0*PI/(float)segs;
		}
	
	delta2 = (float)2.0*PI/(float)sides;
	
	if (TestAFlag(A_PLUGIN1)) startAng -= HALFPI;

	int nverts;
	int nfaces;
	if (doPie) {
		nverts = sides*segs + 2;
		nfaces = 2*sides*segs;
	} else {
		nverts = sides*segs;
		nfaces = 2*sides*segs;
		}
	mesh.setNumVerts(nverts);
	mesh.setNumFaces(nfaces);
	mesh.setSmoothFlags(smooth);
	if (genUVs) {
		if (doPie) {
			mesh.setNumTVerts((sides+1)*segs+2);
			mesh.setNumTVFaces(2*sides*segs);
		} else {
			mesh.setNumTVerts((sides+1)*(segs+1));
			mesh.setNumTVFaces(2*sides*segs);
			}
	} else {
		mesh.setNumTVerts(0);
		mesh.setNumTVFaces(0);
		}

	ang = startAng;

	// make verts
	for(ix=0; ix<segs; ix++) {
		sinang = (float)sin(ang);
		cosang = (float)cos(ang);
		ang2 = rotation + twist * float(ix+1)/float(segs);
		for (jx = 0; jx<sides; jx++) {
			sinang2 = (float)sin(ang2);
			cosang2 = (float)cos(ang2);
			rt = radius+radius2*cosang2;
			p.x = rt*cosang;
			p.y = -rt*sinang;
			p.z = radius2*sinang2;	
			mesh.setVert(nv++, p);
			ang2 += delta2;
			}	
		ang += delta;
		}
	
	if (doPie) {
		p.x = radius * (float)cos(startAng);
		p.y = -radius * (float)sin(startAng);
		p.z = 0.0f;
		mesh.setVert(nv++, p);

		ang -= delta;
		p.x = radius * (float)cos(ang);
		p.y = -radius * (float)sin(ang);
		p.z = 0.0f;
		mesh.setVert(nv++, p);
		}
	
	// Make faces

	/* Make midsection */
	for(ix=0; ix<segs-doPie; ++ix) {
		jx=ix*sides;
		for (kx=0; kx<sides; ++kx) {
			na = jx+kx;
			nb = (ix==(segs-1))?kx:na+sides;
			nd = (kx==(sides-1))? jx : na+1;
			nc = nb+nd-na;

			DWORD grp = 0;
			if (smooth==SMOOTH_SIDES) {
				if (kx==sides-1 && (sides&1)) {
					grp = (1<<2);
				} else {
					grp = (kx&1) ? (1<<0) : (1<<1);
					}
			} else 
			if (smooth==SMOOTH_STRIPES) {
				if (ix==segs-1 && (segs&1)) {
					grp = (1<<2);
				} else {
					grp = (ix&1) ? (1<<0) : (1<<1);
					}
			} else 
			if (smooth > 0) {
				grp = 1;
				}

			mesh.faces[nf].setEdgeVisFlags(0,1,1);
			mesh.faces[nf].setSmGroup(grp);
			mesh.faces[nf].setMatID(0);
			mesh.faces[nf++].setVerts( na,nc,nb);

			mesh.faces[nf].setEdgeVisFlags(1,1,0);
			mesh.faces[nf].setSmGroup(grp);
			mesh.faces[nf].setMatID(0);
			mesh.faces[nf++].setVerts(na,nd,nc);
			}
	 	}

	if (doPie) {		
		na = nv -2;
		for(ix=0; ix<sides; ++ix) {
			nb = ix;
			nc = (ix==(sides-1))?0:ix+1;
			mesh.faces[nf].setEdgeVisFlags(0,1,0);
			mesh.faces[nf].setSmGroup((1<<3));
			mesh.faces[nf].setMatID(1);
			mesh.faces[nf++].setVerts(na,nc,nb);
			}
		
		na = nv -1;
		jx = sides*(segs-1);
		for(ix=0; ix<sides; ++ix) {
			nb = jx+ix;
			nc = (ix==(sides-1))?jx:nb+1;
			mesh.faces[nf].setEdgeVisFlags(0,1,0);
			mesh.faces[nf].setSmGroup((1<<3));
			mesh.faces[nf].setMatID(2);
			mesh.faces[nf++].setVerts(na,nb,nc);
			}
		}

	
	// UVWs -------------------
	
	if (genUVs) {
		nv=0;
		for(ix=0; ix<=segs-doPie; ix++) {
			for (jx=0; jx<=sides; jx++) {
				mesh.setTVert(nv++,float(jx)/float(sides),float(ix)/float(segs),0.0f);
				}
			}
		int pie1, pie2;
		if (doPie) {
			pie1 = nv;
			mesh.setTVert(nv++,0.5f,1.0f,0.0f);
			pie2 = nv;
			mesh.setTVert(nv++,0.5f,0.0f,0.0f);
			}				
		
		nf=0;
		for(ix=0; ix<segs-doPie; ix++) {
			na = ix*(sides+1);
			nb = (ix+1)*(sides+1);
			for (jx=0; jx<sides; jx++) {
				mesh.tvFace[nf++].setTVerts(na,nb+1,nb);
				mesh.tvFace[nf++].setTVerts(na,na+1,nb+1);
				na++;
				nb++;
				}
			}
		if (doPie) {						
			for (jx=0; jx<sides; jx++) {
				mesh.tvFace[nf++].setTVerts(pie1,jx+1,jx);				
				}			
			nb = (sides+1)*(segs-1);
			for (jx=0; jx<sides; jx++) {
				mesh.tvFace[nf++].setTVerts(pie2,nb,nb+1);
				nb++;
				}
			}
		}

	mesh.InvalidateTopologyCache();
	}

#define Tang(vv,ii) ((vv)*4+(ii))

inline Point3 operator+(const PatchVert &pv,const Point3 &p)
	{
	return p+pv.p;
	}

#define CIRCLE_FACT8	0.265202f
#define CIRCLE_FACT4	0.5517861843f

void BuildTorusPatch(
		TimeValue t, PatchMesh &patch, 
		float radius1, float radius2, int genUVs)
	{
	int segs = 8, sides = 4;
	int nverts = segs * sides;
	int nvecs = segs*sides*8;
	int npatches = segs * sides;
	patch.setNumVerts(nverts);
	patch.setNumTVerts(genUVs ? (segs + 1) * (sides + 1) : 0);
	patch.setNumVecs(nvecs);
	patch.setNumPatches(npatches);	
	patch.setNumTVPatches(genUVs ? npatches : 0);
	int ix=0, jx=0, kx=sides*segs*4, i, j;
	float ang1 = 0.0f, delta1 = TWOPI/float(segs);
	float ang2 = 0.0f, delta2 = TWOPI/float(sides);	
	float circleLenIn = CIRCLE_FACT8*(radius1-radius2);
	float circleLenOut = CIRCLE_FACT8*(radius1+radius2);
	float circleLenMid = CIRCLE_FACT4*radius2;
	float circleLen;
	float sinang1, cosang1, sinang2, cosang2, rt, u;
	Point3 p, v;
	DWORD a, b, c, d;

	for (i=0; i<segs; i++) {
		sinang1 = (float)sin(ang1);
		cosang1 = (float)cos(ang1);
		ang2 = 0.0f;
		for (j=0; j<sides; j++) {			
			sinang2 = (float)sin(ang2);
			cosang2 = (float)cos(ang2);
			rt = radius1+radius2*cosang2;
			
			// Vertex
			p.x = rt*cosang1;
			p.y = rt*sinang1;
			p.z = radius2*sinang2;	
			patch.setVert(ix, p);
			
			// Tangents			
			u = (cosang2+1.0f)/2.0f;
			circleLen = u*circleLenOut + (1.0f-u)*circleLenIn;

			v.x = -sinang1*circleLen;
			v.y = cosang1*circleLen;
			v.z = 0.0f;
			patch.setVec(jx++,patch.verts[ix] + v);
			
			v.x = sinang1*circleLen;
			v.y = -cosang1*circleLen;
			v.z = 0.0f;
			patch.setVec(jx++,patch.verts[ix] + v);
			
			v.x = -sinang2*cosang1*circleLenMid;
			v.y = -sinang2*sinang1*circleLenMid;
			v.z = cosang2*circleLenMid;
			patch.setVec(jx++,patch.verts[ix] + v);
			
			v.x = sinang2*cosang1*circleLenMid;
			v.y = sinang2*sinang1*circleLenMid;
			v.z = -cosang2*circleLenMid;
			patch.setVec(jx++,patch.verts[ix] + v);			

			// Build the patch
			a = ((i+1)%segs)*sides + (j+1)%sides;
			b = i*sides + (j+1)%sides;
			c = i*sides + j;			
			d = ((i+1)%segs)*sides + j;
			
			patch.patches[ix].SetType(PATCH_QUAD);
			patch.patches[ix].setVerts(a, b, c, d);
			patch.patches[ix].setVecs(
				Tang(a,1),Tang(b,0),Tang(b,3),Tang(c,2),
				Tang(c,0),Tang(d,1),Tang(d,2),Tang(a,3));
			patch.patches[ix].setInteriors(kx, kx+1, kx+2, kx+3);
			patch.patches[ix].smGroup = 1;

			kx += 4;
			ix++;
			ang2 += delta2;
			}		
		ang1 += delta1;
		}	

	if(genUVs) {
		int tv = 0;
		int tvp = 0;
		float fsegs = (float)segs;
		float fsides = (float)sides;
		for (i=0; i<=segs; i++) {
			float u = (float)i / fsegs;
			for (j=0; j<=sides; j++,++tv) {
				float v = (float)j / fsides;
				patch.setTVert(tv, UVVert(1.0f-u, v, 0.0f));
				if(j < sides && i < segs)
					patch.getTVPatch(tvp++).setTVerts(tv, tv+1, tv+sides+2, tv+sides+1);
				}		
			}	
		}
			
	assert(patch.buildLinkages());
	patch.computeInteriors();
	patch.InvalidateGeomCache();
	}



#ifndef NO_NURBS

Object *
BuildNURBSTorus(float radius, float radius2, BOOL sliceon, float pie1, float pie2, BOOL genUVs)
{
	NURBSSet nset;

	Point3 origin(0,0,0);
	Point3 symAxis(0,0,1);
	Point3 refAxis(0,1,0);

	float startAngle = 0.0f;
	float endAngle = TWOPI;
	if (sliceon && pie1 != pie2) {
		float sweep = pie2-pie1;
		if (sweep <= 0.0f) sweep += TWOPI;
		refAxis = Point3(Point3(0,1,0) * RotateZMatrix(pie1));
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

	surf->FlipNormals(TRUE);
	surf->Renderable(TRUE);
	char bname[80];
	char sname[80];
	strcpy(bname, GetString(IDS_RB_TORUS));
	sprintf(sname, "%s%s", bname, GetString(IDS_CT_SURF));
	surf->SetName(sname);

	if (sliceon && pie1 != pie2) {
		GenNURBSTorusSurface(radius, radius2, origin, symAxis, refAxis,
						startAngle, endAngle, -PI, PI, TRUE, *surf);
		// now create caps on the ends
		NURBSCapSurface *cap0 = new NURBSCapSurface();
		nset.AppendObject(cap0);
		cap0->SetGenerateUVs(genUVs);
		cap0->SetParent(0);
		cap0->SetEdge(2);
		cap0->FlipNormals(FALSE);
		cap0->Renderable(TRUE);
		char sname[80];
		sprintf(sname, "%s%s%02d", bname, GetString(IDS_CT_CAP), 0);
		cap0->SetName(sname);

		NURBSCapSurface *cap1 = new NURBSCapSurface();
		nset.AppendObject(cap1);
		cap1->SetGenerateUVs(genUVs);
		cap1->SetParent(0);
		cap1->SetEdge(3);
		cap1->FlipNormals(TRUE);
		cap1->Renderable(TRUE);
		sprintf(sname, "%s%s%02d", bname, GetString(IDS_CT_CAP), 1);
		cap1->SetName(sname);
	} else {
		GenNURBSTorusSurface(radius, radius2, origin, symAxis, refAxis,
						startAngle, endAngle, -PI, PI, FALSE, *surf);
    }


	Matrix3 mat;
	mat.IdentityMatrix();
	Object *ob = CreateNURBSObject(NULL, &nset, mat);
	return ob;
}
#endif


Object* TorusObject::ConvertToType(TimeValue t, Class_ID obtype)
	{
#ifndef NO_PATCHES
	if (obtype == patchObjectClassID) {
		Interval valid = FOREVER;
		float radius1, radius2;
		int genUVs;
		pblock->GetValue(PB_RADIUS,t,radius1,valid);
		pblock->GetValue(PB_RADIUS2,t,radius2,valid);
		pblock->GetValue(PB_GENUVS,t,genUVs,valid);
		PatchObject *ob = new PatchObject();
		BuildTorusPatch(t,ob->patch,radius1,radius2,genUVs);
		ob->SetChannelValidity(TOPO_CHAN_NUM,valid);
		ob->SetChannelValidity(GEOM_CHAN_NUM,valid);
		ob->UnlockObject();
		return ob;
	} 
#endif
#ifndef NO_NURBS
    if (obtype == EDITABLE_SURF_CLASS_ID) {
		Interval valid = FOREVER;
		float radius, radius2, pie1, pie2;
		int sliceon, genUVs;
		pblock->GetValue(PB_RADIUS,t,radius,valid);
		pblock->GetValue(PB_RADIUS2,t,radius2,valid);	
		pblock->GetValue(PB_PIESLICE1,t,pie1,valid);	
		pblock->GetValue(PB_PIESLICE2,t,pie2,valid);	
		pblock->GetValue(PB_SLICEON,t,sliceon,valid);
		pblock->GetValue(PB_GENUVS,t,genUVs,valid);
		Object *ob = BuildNURBSTorus(radius, radius2, sliceon, pie1, pie2, genUVs);
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
		float radius1, radius2, pie1, pie2;
		int sliceon, genUVs, sides, segs;
		pblock->GetValue(PB_RADIUS,t,radius1,valid);
		pblock->GetValue(PB_RADIUS2,t,radius2,valid);
		pblock->GetValue(PB_PIESLICE1,t,pie1,valid);	
		pblock->GetValue(PB_PIESLICE2,t,pie2,valid);	
		pblock->GetValue(PB_SLICEON,t,sliceon,valid);
		pblock->GetValue(PB_GENUVS,t,genUVs,valid);
		pblock->GetValue(PB_SIDES,t,sides,valid);
		pblock->GetValue(PB_SEGMENTS,t,segs,valid);
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
				bool res = cacheptr->createTorus(radius1, radius2, sides, segs, smooth);
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

int TorusObject::CanConvertToType(Class_ID obtype)
	{
	if(obtype==defObjectClassID || obtype==triObjectClassID) return 1;
#ifdef DESIGN_VER
	if(obtype == GENERIC_AMSOLID_CLASS_ID) return 1;
#endif
#ifndef NO_PATCHES
    if(obtype == patchObjectClassID) return 1;
#endif
#ifndef NO_NURBS
    if(obtype==EDITABLE_SURF_CLASS_ID) return 1;
#endif
    return SimpleObject::CanConvertToType(obtype);
	}

void TorusObject::GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist)
{
    Object::GetCollapseTypes(clist, nlist);
#ifndef NO_NURBS
    Class_ID id = EDITABLE_SURF_CLASS_ID;
    TSTR *name = new TSTR(GetString(IDS_SM_NURBS_SURFACE));
    clist.Append(1,&id);
    nlist.Append(1,&name);
#endif
}


class TorusObjCreateCallBack: public CreateMouseCallBack {
	TorusObject *ob;	
	Point3 p0, p1, p2;
	IPoint2 sp0,sp1,sp2;	
	float oldRad2;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		void SetObj(TorusObject *obj) { ob = obj; }
	};



int TorusObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
	float r, r2;
	Point3 center;

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
				oldRad2 = ob->crtRadius2;
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
				sp1 = m;							   
				#ifdef _3D_CREATE	
					p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				#else	
					p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				#endif
				if (ob->dlgCreateMeth) {	
					// radius	
					r = Length(p1-p0) - ob->crtRadius2;
					mat.SetTrans(p0);
				} else {
					// diameter
					Point3 center = (p0+p1)/float(2);
					r = Length(center-p0) - ob->crtRadius2;
					mat.SetTrans(center);  // Modify Node's transform
					}

				if (msg==MOUSE_POINT) {
					ob->suspendSnap = FALSE;
					if (Length(m-sp0)<3 || Length(p1-p0)<0.1f)
						return CREATE_ABORT;
					}
				
				ob->pblock->SetValue(PB_RADIUS,0,r);
				ob->pmapParam->Invalidate();
				
				if (flags&MOUSE_CTRL) {
					float ang = (float)atan2(p1.y-p0.y,p1.x-p0.x);					
					mat.PreRotateZ(ob->ip->SnapAngle(ang));
					}				
				break;
			
			case 2:					
				center = mat.GetTrans();
				mat.IdentityMatrix();
				//mat.PreRotateZ(HALFPI);
				mat.SetTrans(center);

				#ifdef _3D_CREATE	
					p2 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				#else	
					p2 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				#endif

				r   = Length(p1-p0);
				r2  = Length(p2-p0);
//O.Z. *** begin fix for bug #189695
				if (ob->dlgCreateMeth) {	
					ob->crtRadius2 = (float)fabs(r-r2)/2.0f;
					ob->pblock->SetValue(PB_RADIUS,0,(r2+r)/2.0f);
					ob->pblock->SetValue(PB_RADIUS2,0,ob->crtRadius2);
					ob->pmapParam->Invalidate();
				}else{
					Point3 center = (p2+p0)/float(2);
					r = Length(center-p0);
					mat.SetTrans(center);  // Modify Node's transform
					ob->pblock->SetValue(PB_RADIUS,0,r);
				}
//O.Z. *** end fix for bug #189695
				
				if (flags&MOUSE_CTRL) {
					float ang = (float)atan2(p2.y-p0.y,p2.x-p0.x);					
					mat.PreRotateZ(ob->ip->SnapAngle(ang));
					}

				if (msg==MOUSE_POINT) {
					ob->suspendSnap = FALSE;
					return CREATE_STOP;
					}
				break;					   
			}
		}
	else
	if (msg == MOUSE_ABORT) {		
		ob->crtRadius2 = oldRad2;
		return CREATE_ABORT;
		}

	return TRUE;
	}


static TorusObjCreateCallBack torusCreateCB;

CreateMouseCallBack* TorusObject::GetCreateMouseCallBack() {
	torusCreateCB.SetObj(this);
	return(&torusCreateCB);
	}

BOOL TorusObject::OKtoDisplay(TimeValue t) 
	{
	return TRUE;
	}


// From ParamArray
BOOL TorusObject::SetValue(int i, TimeValue t, int v) 
	{
	switch (i) {
		case PB_CREATEMETHOD: dlgCreateMeth = v; break;
		}		
	return TRUE;
	}

BOOL TorusObject::SetValue(int i, TimeValue t, float v)
	{
	switch (i) {				
		case PB_TI_RADIUS1: crtRadius1 = v; break;
		case PB_TI_RADIUS2: crtRadius2 = v; break;
		}	
	return TRUE;
	}

BOOL TorusObject::SetValue(int i, TimeValue t, Point3 &v) 
	{
	switch (i) {
		case PB_TI_POS: crtPos = v; break;
		}		
	return TRUE;
	}

BOOL TorusObject::GetValue(int i, TimeValue t, int &v, Interval &ivalid) 
	{
	switch (i) {
		case PB_CREATEMETHOD: v = dlgCreateMeth; break;
		}
	return TRUE;
	}

BOOL TorusObject::GetValue(int i, TimeValue t, float &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_RADIUS1: v = crtRadius1; break;
		case PB_TI_RADIUS2: v = crtRadius2; break;
		}
	return TRUE;
	}

BOOL TorusObject::GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_POS: v = crtPos; break;		
		}
	return TRUE;
	}


void TorusObject::InvalidateUI() 
	{
	if (pmapParam) pmapParam->Invalidate();
	}

ParamDimension *TorusObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_RADIUS:		return stdWorldDim;
		case PB_RADIUS2:	return stdWorldDim;
		case PB_ROTATION:	return stdAngleDim;
		case PB_TWIST:		return stdAngleDim;
		case PB_SEGMENTS:	return stdSegmentsDim;
		case PB_SIDES:		return stdSegmentsDim;
		case PB_SMOOTH:		return stdNormalizedDim;
		case PB_SLICEON:	return stdNormalizedDim;
		case PB_PIESLICE1:	return stdAngleDim;
		case PB_PIESLICE2:	return stdAngleDim;
		default: return defaultDim;
		}
	}

TSTR TorusObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_RADIUS:		return TSTR(GetString(IDS_RB_RADIUS1));		
		case PB_RADIUS2:	return TSTR(GetString(IDS_RB_RADIUS2));
		case PB_ROTATION:	return TSTR(GetString(IDS_RB_ROTATION2));
		case PB_TWIST:		return TSTR(GetString(IDS_RB_TWIST));
		case PB_SEGMENTS:	return TSTR(GetString(IDS_RB_SEGS));					
		case PB_SIDES:		return TSTR(GetString(IDS_RB_SIDES));
		case PB_SMOOTH:		return TSTR(GetString(IDS_RB_SMOOTH));
		case PB_SLICEON:	return TSTR(GetString(IDS_RB_SLICEON));
		case PB_PIESLICE1:	return TSTR(GetString(IDS_RB_SLICETO));
		case PB_PIESLICE2:	return TSTR(GetString(IDS_RB_SLICEFROM));		
		default: return TSTR(_T(""));
		}
	}

RefTargetHandle TorusObject::Clone(RemapDir& remap) 
	{
	TorusObject* newob = new TorusObject();	
	newob->ReplaceReference(0,pblock->Clone(remap));	
	newob->ivalid.SetEmpty();	
	BaseClone(this, newob, remap);
	return(newob);
	}

#endif // NO_OBJECT_STANDARD_PRIMITIVES

