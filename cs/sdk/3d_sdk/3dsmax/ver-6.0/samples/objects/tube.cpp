/**********************************************************************
 *<
	FILE: tube.cpp

	DESCRIPTION:  A tube object

	CREATED BY: Rolf Berteig

	HISTORY: created 13 September 1994

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "prim.h"

#ifndef NO_OBJECT_STANDARD_PRIMITIVES

#include "iparamm.h"
#include "Simpobj.h"
#include "surf_api.h"

class TubeObject : public SimpleObject, public IParamArray {
	public:
		// Class vars
		static IParamMap *pmapCreate;
		static IParamMap *pmapTypeIn;
		static IParamMap *pmapParam;
		static IObjParam *ip;
		static int dlgSegments, dlgCapSegments, dlgSides;		
		static int dlgCreateMeth;
		static int dlgSmooth;	
		static float dlgRadius2;	
		static Point3 crtPos;		
		static float crtRadius1;
		static float crtRadius2;	
		static float crtHeight;

		TubeObject();
		
		// From Object
		int CanConvertToType(Class_ID obtype);
		Object* ConvertToType(TimeValue t, Class_ID obtype);
		void GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist);
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);
			
		// From BaseObject
		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);		
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		TCHAR *GetObjectName() { return GetString(IDS_RB_TUBE);}

		// Animatable methods		
		void DeleteThis() {delete this;}
		Class_ID ClassID() {return Class_ID(TUBE_CLASS_ID,0);}
		
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

#define MIN_SEGMENTS	1
#define MAX_SEGMENTS	200

#define MIN_SIDES		1
#define MAX_SIDES		200

#define MIN_RADIUS		float(0)
#define MAX_RADIUS		float( 1.0E30)
#define MIN_PIESLICE	float(-1.0E30)
#define MAX_PIESLICE	float( 1.0E30)

#define DEF_SEGMENTS 	18	// 24
#define DEF_SIDES		5	// 1
#define DEF_CAPSEGMENTS	1

#define DEF_RADIUS		(0.1f)
#define DEF_RADIUS2   	(10.0f)

#define SMOOTH_ON		1
#define SMOOTH_SIDES	1
#define SMOOTH_OFF		0


//--- ClassDescriptor and class vars ---------------------------------

class TubeClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new TubeObject; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_TUBE_CLASS);}
	SClass_ID		SuperClassID() { return GEOMOBJECT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(TUBE_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_PRIMITIVES); }
	void            ResetClassParams(BOOL fileReset);
	};

static TubeClassDesc tubeDesc;

ClassDesc* GetTubeDesc() { return &tubeDesc; }

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// class variable for sphere class.
IObjParam *TubeObject::ip         = NULL;
int TubeObject::dlgSegments       = DEF_SEGMENTS;
int TubeObject::dlgCapSegments    = DEF_CAPSEGMENTS;
int TubeObject::dlgSides          = DEF_SIDES;
int TubeObject::dlgCreateMeth     = 1; // create_radius
int TubeObject::dlgSmooth         = SMOOTH_ON;
float TubeObject::dlgRadius2      = DEF_RADIUS2;
IParamMap *TubeObject::pmapCreate = NULL;
IParamMap *TubeObject::pmapTypeIn = NULL;
IParamMap *TubeObject::pmapParam  = NULL;
Point3 TubeObject::crtPos         = Point3(0,0,0);
float TubeObject::crtRadius1      = 0.0f;
float TubeObject::crtRadius2      = 0.0f;
float TubeObject::crtHeight       = 0.0f;

void TubeClassDesc::ResetClassParams(BOOL fileReset)
	{
	TubeObject::dlgSegments    = DEF_SEGMENTS;
	TubeObject::dlgCapSegments = DEF_CAPSEGMENTS;
	TubeObject::dlgSides       = DEF_SIDES;
	TubeObject::dlgCreateMeth  = 1; // create_radius
	TubeObject::dlgSmooth      = SMOOTH_ON;
	TubeObject::dlgRadius2     = DEF_RADIUS2;
	TubeObject::crtPos         = Point3(0,0,0);
	TubeObject::crtRadius1     = 0.0f;
	TubeObject::crtRadius2     = 0.0f;
	TubeObject::crtHeight      = 0.0f;
	}


//--- Parameter map/block descriptors -------------------------------

// Parameter map indices
#define PB_RADIUS		0
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
		IDC_RADIUS2,IDC_RAD2SPINNER,
		MIN_RADIUS,MAX_RADIUS,
		SPIN_AUTOSCALE),	

	// height
	ParamUIDesc(
		PB_TI_HEIGHT,
		EDITTYPE_UNIVERSE,
		IDC_LENGTH,IDC_LENSPINNER,
		float(-1.0E30),float(1.0E30),
		SPIN_AUTOSCALE),	
	};
#define TYPEINDESC_LENGH 4


//
//
// Parameters

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

	// height
	ParamUIDesc(
		PB_HEIGHT,
		EDITTYPE_UNIVERSE,
		IDC_LENGTH,IDC_LENSPINNER,
		float(-1.0E30),float(1.0E30),
		SPIN_AUTOSCALE),	

	// Segments
	ParamUIDesc(
		PB_SEGMENTS,
		EDITTYPE_INT,
		IDC_SEGMENTS,IDC_SEGSPINNER,
		(float)3,(float)MAX_SEGMENTS,
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
	{ TYPE_INT, NULL, TRUE, 7 },
	{ TYPE_FLOAT, NULL, TRUE, 8 },
	{ TYPE_FLOAT, NULL, TRUE, 9 } };

static ParamBlockDescID descVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 },
	{ TYPE_INT, NULL, TRUE, 3 },
	{ TYPE_INT, NULL, TRUE, 4 },
	{ TYPE_INT, NULL, TRUE, 5 },
	{ TYPE_BOOL, NULL, TRUE, 6 },
	{ TYPE_INT, NULL, TRUE, 7 },
	{ TYPE_FLOAT, NULL, TRUE, 8 },
	{ TYPE_FLOAT, NULL, TRUE, 9 },
	{ TYPE_INT, NULL, FALSE, 10 } };

#define PBLOCK_LENGTH	11

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,10,0),
	ParamVersionDesc(descVer1,11,1)	
	};
#define NUM_OLDVERSIONS	2

#define CURRENT_VERSION	1
static ParamVersionDesc curVersion(descVer1,PBLOCK_LENGTH,CURRENT_VERSION);


//--- TypeInDlgProc --------------------------------

class TubeTypeInDlgProc : public ParamMapUserDlgProc {
	public:
		TubeObject *ob;

		TubeTypeInDlgProc(TubeObject *o) {ob=o;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

BOOL TubeTypeInDlgProc::DlgProc(
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





class TubeParamDlgProc : public ParamMapUserDlgProc {
	public:
		TubeObject *so;
		HWND thishWnd;

		TubeParamDlgProc(TubeObject *s) {so=s;thishWnd=NULL;}
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
void TubeParamDlgProc::Update(TimeValue t)
{ if (!thishWnd) return;
  int ison;
  so->pblock->GetValue(PB_SLICEON,t,ison,FOREVER);
  TurnSpinner(thishWnd,IDC_PIESLICESPIN1,ison);
  TurnSpinner(thishWnd,IDC_PIESLICESPIN2,ison);

  EnableWindow(GetDlgItem(thishWnd,IDC_STATICFROM),ison);
  EnableWindow(GetDlgItem(thishWnd,IDC_STATICTO),ison);
}

BOOL TubeParamDlgProc::DlgProc(
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


//--- Tube methods -------------------------------


TubeObject::TubeObject()
	{	
	SetAFlag(A_PLUGIN1);
	MakeRefByID(FOREVER, 0, CreateParameterBlock(descVer1, PBLOCK_LENGTH, CURRENT_VERSION));
	assert(pblock);

	pblock->SetValue(PB_SMOOTH,0,dlgSmooth);
	pblock->SetValue(PB_SEGMENTS,0,dlgSegments);
	pblock->SetValue(PB_CAPSEGMENTS,0,dlgCapSegments);
	pblock->SetValue(PB_SIDES,0,dlgSides);	
	pblock->SetValue(PB_RADIUS,0,crtRadius1);
	pblock->SetValue(PB_RADIUS2,0,crtRadius2);	
	pblock->SetValue(PB_HEIGHT,0,crtHeight);
	
	pblock->SetValue(PB_GENUVS,0,TRUE);
	}

#define NEWMAP_CHUNKID	0x0100

IOResult TubeObject::Load(ILoad *iload) 
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

IOResult TubeObject::Save(ISave *isave)
	{
	if (TestAFlag(A_PLUGIN1)) {
		isave->BeginChunk(NEWMAP_CHUNKID);
		isave->EndChunk();
		}
 	return IO_OK;
	}

void TubeObject::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
	{
	SimpleObject::BeginEditParams(ip,flags,prev);
	this->ip = ip;

	if (pmapCreate && pmapParam && pmapTypeIn) {
		
		// Left over from last Tube ceated		
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
				MAKEINTRESOURCE(IDD_TUBEPARAM1),
				GetString(IDS_RB_CREATIONMETHOD),
				0);

			
			pmapTypeIn = CreateCPParamMap(
				descTypeIn,TYPEINDESC_LENGH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_TUBEPARAM3),
				GetString(IDS_RB_KEYBOARDENTRY),
				APPENDROLL_CLOSED);			
			}

		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_TUBEPARAM2),
			GetString(IDS_RB_PARAMETERS),
			0);
		}

	if(pmapTypeIn) {
		// A callback for the type in.
		pmapTypeIn->SetUserDlgProc(new TubeTypeInDlgProc(this));
		}

	if(pmapParam) {
		// A callback for the type in.
		pmapParam->SetUserDlgProc(new TubeParamDlgProc(this));
		}
	}
		
void TubeObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
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
	pblock->GetValue(PB_RADIUS2,ip->GetTime(),dlgRadius2,FOREVER);		
	}

BOOL TubeObject::HasUVW() { 
	BOOL genUVs;
	Interval v;
	pblock->GetValue(PB_GENUVS, 0, genUVs, v);
	return genUVs; 
	}

void TubeObject::SetGenUVW(BOOL sw) {  
	if (sw==HasUVW()) return;
	pblock->SetValue(PB_GENUVS,0, sw);				

	}

void TubeObject::BuildMesh(TimeValue t)
	{
	Point3 p;
	int ix,na,nb,nc,nd,jx,kx;
	int nf=0,nv=0;
	float delta,ang;	
	int sides,segs,smooth,capsegs, ssides;
	float radius,radius2, temp;
	float sinang,cosang, height;
	float pie1, pie2, totalPie, startAng = 0.0f;
	int doPie = TRUE;	
	int genUVs = TRUE;

	// Start the validity interval at forever and widdle it down.
	ivalid = FOREVER;	
	pblock->GetValue(PB_RADIUS,t,radius,ivalid);
	pblock->GetValue(PB_RADIUS2,t,radius2,ivalid);	
	pblock->GetValue(PB_HEIGHT,t,height,ivalid);
	pblock->GetValue(PB_SEGMENTS,t,segs,ivalid);
	pblock->GetValue(PB_CAPSEGMENTS,t,capsegs,ivalid);
	pblock->GetValue(PB_SIDES,t,ssides,ivalid);
	pblock->GetValue(PB_SMOOTH,t,smooth,ivalid);
	pblock->GetValue(PB_PIESLICE1,t,pie1,ivalid);
	pblock->GetValue(PB_PIESLICE2,t,pie2,ivalid);	
	pblock->GetValue(PB_SLICEON,t,doPie,ivalid);		
	pblock->GetValue(PB_GENUVS,t,genUVs,ivalid);
	LimitValue( radius, MIN_RADIUS, MAX_RADIUS );
	LimitValue( radius2, MIN_RADIUS, MAX_RADIUS );
	LimitValue( segs, MIN_SEGMENTS, MAX_SEGMENTS );
	LimitValue( capsegs, MIN_SEGMENTS, MAX_SEGMENTS );
	LimitValue( ssides, MIN_SIDES, MAX_SIDES );	
	doPie = doPie ? 1 : 0;

	// We do the torus backwards from the cylinder
	temp = -pie1;
	pie1 = -pie2;
	pie2 = temp;	

	// Flip parity when radi are swaped or height is negative
	if ((radius<radius2 || height<0) && !(radius<radius2&&height<0)) {
		temp    = radius;
		radius  = radius2;
		radius2 = temp;
		}	

	// Make pie2 < pie1 and pie1-pie2 < TWOPI
	while (pie1 < pie2) pie1 += TWOPI;
	while (pie1 > pie2+TWOPI) pie1 -= TWOPI;
	if (pie1==pie2) totalPie = TWOPI;
	else totalPie = pie1-pie2;	
	
	if (doPie) {
		segs++;	 // *** O.Z. fix for 240436
		delta    = totalPie/(float)(segs-1);
		startAng = pie2;
	} else {
		delta = (float)2.0*PI/(float)segs;
		}	

	sides = 2*(ssides+capsegs);

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
	mesh.setSmoothFlags(smooth != 0);
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
		
		// Radius 1
		for (jx = 0; jx<ssides; jx++) {
			p.x = radius*cosang;
			p.y = -radius*sinang;
			p.z = float(jx)/float(ssides) * height;
			mesh.setVert(nv++, p);
			}
		
		// Top		
		for (jx = 0; jx<capsegs; jx++) {
			float u = float(jx)/float(capsegs);
			p.x = (u*radius2 + (1.0f-u)*radius) * cosang;
			p.y = -(u*radius2 + (1.0f-u)*radius) * sinang;
			p.z = height;
			mesh.setVert(nv++, p);
			}
		
		// Radius 2
		for (jx = 0; jx<ssides; jx++) {
			p.x = radius2*cosang;
			p.y = -radius2*sinang;
			p.z = (1.0f-float(jx)/float(ssides)) * height;
			mesh.setVert(nv++, p);
			}
		
		// Bottom
		for (jx = 0; jx<capsegs; jx++) {
			float u = float(jx)/float(capsegs);
			p.x = (u*radius + (1.0f-u)*radius2) * cosang;
			p.y = -(u*radius + (1.0f-u)*radius2) * sinang;
			p.z = 0.0f;
			mesh.setVert(nv++, p);
			}
		
		ang += delta;
		}
	
	if (doPie) {
		float averag = (radius + radius2) / 2.0f;
		p.x = averag * (float)cos(startAng);
		p.y = -averag * (float)sin(startAng);
		p.z = height/2.0f;
		mesh.setVert(nv++, p);

		ang -= delta;
		p.x = averag * (float)cos(ang);
		p.y = -averag * (float)sin(ang);
		p.z = height/2.0f;
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
			
			DWORD grp =  0;
			int mtlid = 0;
			if  (kx<ssides) {
				mtlid = 2;
				grp = (1<<1);
				}
			else if (kx<ssides+capsegs) {
				mtlid = 0;
				grp = (1<<2);
				}
			else if (kx<2*ssides+capsegs) {
				mtlid = 3;
				grp = (1<<1);
				}
			else {
				mtlid = 1;
				grp = (1<<2);
				}

			if (!smooth) grp = 0;

			mesh.faces[nf].setEdgeVisFlags(0,1,1);
			mesh.faces[nf].setSmGroup(grp);
			mesh.faces[nf].setMatID(mtlid);
			mesh.faces[nf++].setVerts( na,nc,nb);

			mesh.faces[nf].setEdgeVisFlags(1,1,0);
			mesh.faces[nf].setSmGroup(grp);
			mesh.faces[nf].setMatID(mtlid);
			mesh.faces[nf++].setVerts(na,nd,nc);
			}
	 	}

	if (doPie) {		
		na = nv -2;
		for(ix=0; ix<sides; ++ix) {
			nb = ix;
			nc = (ix==(sides-1))?0:ix+1;
			mesh.faces[nf].setEdgeVisFlags(0,1,0);
			mesh.faces[nf].setSmGroup((1<<0));
			mesh.faces[nf].setMatID(4);
			mesh.faces[nf++].setVerts(na,nc,nb);
			}
		
		na = nv -1;
		jx = sides*(segs-1);
		for(ix=0; ix<sides; ++ix) {
			nb = jx+ix;
			nc = (ix==(sides-1))?jx:nb+1;
			mesh.faces[nf].setEdgeVisFlags(0,1,0);
			mesh.faces[nf].setSmGroup((1<<0));
			mesh.faces[nf].setMatID(5);
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

#define CIRCLE_FACT	0.5517861843f
#define Tang(vv,ii) ((vv)*4+(ii))

inline Point3 operator+(const PatchVert &pv,const Point3 &p)
	{
	return p+pv.p;
	}

void BuildTubePatch(
		PatchMesh &patch, 
		float radius1, float radius2, float height, int genUVs)
	{
	if (radius1<radius2) {
		float temp = radius1;
		radius1 = radius2;
		radius2 = temp;
		}
	int nverts = 16;
	int nvecs = 128;
	int npatches = 16;
	patch.setNumVerts(nverts);	
	patch.setNumTVerts(genUVs ? 25 : 0);
	patch.setNumVecs(nvecs);
	patch.setNumPatches(npatches);
	patch.setNumTVPatches(genUVs ? npatches : 0);
	float ocircleLen = radius1*CIRCLE_FACT;
	float icircleLen = radius2*CIRCLE_FACT;
	float radLen = (radius1-radius2)/3.0f;
	float heightLen = height/3.0f;
	int i;
	DWORD a, b, c, d;

	// Base
	patch.setVert(0, radius1, 0.0f, 0.0f);
	patch.setVert(1, radius2, 0.0f, 0.0f);
	patch.setVert(2, 0.0f, radius1, 0.0f);
	patch.setVert(3, 0.0f, radius2, 0.0f);
	patch.setVert(4, -radius1, 0.0f, 0.0f);
	patch.setVert(5, -radius2, 0.0f, 0.0f);
	patch.setVert(6, 0.0f, -radius1, 0.0f);	
	patch.setVert(7, 0.0f, -radius2, 0.0f);

	// Top
	patch.setVert(8, radius1, 0.0f, height);
	patch.setVert(9, radius2, 0.0f, height);
	patch.setVert(10, 0.0f, radius1, height);
	patch.setVert(11, 0.0f, radius2, height);
	patch.setVert(12, -radius1, 0.0f, height);
	patch.setVert(13, -radius2, 0.0f, height);
	patch.setVert(14, 0.0f, -radius1, height);	
	patch.setVert(15, 0.0f, -radius2, height);

	Point3 ovecs[] = {
		Point3(0.0f, ocircleLen, 0.0f),		
		Point3(-ocircleLen, 0.0f, 0.0f),
		Point3(0.0f, -ocircleLen, 0.0f),
		Point3(ocircleLen, 0.0f, 0.0f),
		};
	Point3 rovecs[] = {
		Point3(0.0f, radLen, 0.0f),		
		Point3(-radLen, 0.0f, 0.0f),
		Point3(0.0f, -radLen, 0.0f),
		Point3(radLen, 0.0f, 0.0f),
		};

	Point3 ivecs[] = {
		Point3(0.0f, icircleLen, 0.0f),
		Point3(icircleLen, 0.0f, 0.0f),
		Point3(0.0f, -icircleLen, 0.0f),
		Point3(-icircleLen, 0.0f, 0.0f),
		};
	Point3 rivecs[] = {
		Point3(0.0f, radLen, 0.0f),
		Point3(radLen, 0.0f, 0.0f),
		Point3(0.0f, -radLen, 0.0f),
		Point3(-radLen, 0.0f, 0.0f),
		};

	// Tangents
	int ix=0;
	for (i=0; i<4; i++) {		
		patch.setVec(ix++,patch.verts[i*2] + ovecs[i]);
		patch.setVec(ix++,patch.verts[i*2] + rovecs[(i+1)%4]);
		patch.setVec(ix++,patch.verts[i*2] + ovecs[(i+2)%4]);
		patch.setVec(ix++,patch.verts[i*2] + Point3(0.0f,0.0f,heightLen));

		patch.setVec(ix++,patch.verts[i*2+1] + ivecs[(4-i)%4]);
		patch.setVec(ix++,patch.verts[i*2+1] + rivecs[((4-i)%4+1)%4]);
		patch.setVec(ix++,patch.verts[i*2+1] + ivecs[((4-i)%4+2)%4]);
		patch.setVec(ix++,patch.verts[i*2+1] + Point3(0.0f,0.0f,heightLen));
		}
	for (i=0; i<4; i++) {		
		patch.setVec(ix++,patch.verts[i*2+8] + ovecs[i]);
		patch.setVec(ix++,patch.verts[i*2+8] + rovecs[(i+1)%4]);
		patch.setVec(ix++,patch.verts[i*2+8] + ovecs[(i+2)%4]);
		patch.setVec(ix++,patch.verts[i*2+8] + Point3(0.0f,0.0f,-heightLen));
															
		patch.setVec(ix++,patch.verts[i*2+9] + ivecs[(4-i)%4]);
		patch.setVec(ix++,patch.verts[i*2+9] + rivecs[((4-i)%4+1)%4]);
		patch.setVec(ix++,patch.verts[i*2+9] + ivecs[((4-i)%4+2)%4]);
		patch.setVec(ix++,patch.verts[i*2+9] + Point3(0.0f,0.0f,-heightLen));
		}	
	
	// Patches
	int px = 0;
	for (i=0; i<4; i++) {
		a = i*2+8;
		b = i*2;
		c = (i*2+2)%8;
		d = (i*2+2)%8+8;
		patch.patches[px].SetType(PATCH_QUAD);
		patch.patches[px].setVerts(a, b, c, d);
		patch.patches[px].setVecs(
			Tang(a,3),Tang(b,3),Tang(b,0),Tang(c,2),
			Tang(c,3),Tang(d,3),Tang(d,2),Tang(a,0));
		patch.patches[px].setInteriors(ix, ix+1, ix+2, ix+3);
		patch.patches[px].smGroup = 1;
//watje 3-17-99 to support patch matids
		patch.patches[px].setMatID(2);

		ix+=4;
		px++;
		}
	for (i=0; i<4; i++) {
		a = (i*2+1+2)%8+8;
		b = (i*2+1+2)%8;
		c = i*2+1;
		d = i*2+1+8;				
		patch.patches[px].SetType(PATCH_QUAD);
		patch.patches[px].setVerts(a, b, c, d);
		patch.patches[px].setVecs(
			Tang(a,3),Tang(b,3),Tang(b,2),Tang(c,0),
			Tang(c,3),Tang(d,3),Tang(d,0),Tang(a,2));
		patch.patches[px].setInteriors(ix, ix+1, ix+2, ix+3);
		patch.patches[px].smGroup = 1;
//watje 3-17-99 to support patch matids
		patch.patches[px].setMatID(3);
		ix+=4;
		px++;
		}
	
	for (i=0; i<4; i++) {
		a = i*2+1;
		b = (i*2+3)%8;
		c = (i*2+2)%8;
		d = (i*2);
		patch.patches[px].SetType(PATCH_QUAD);
		patch.patches[px].setVerts(a, b, c, d);
		patch.patches[px].setVecs(
			Tang(a,0),Tang(b,2),Tang(b,1),Tang(c,1),
			Tang(c,2),Tang(d,0),Tang(d,1),Tang(a,1));
		patch.patches[px].setInteriors(ix, ix+1, ix+2, ix+3);
		patch.patches[px].smGroup = 2;
//watje 3-17-99 to support patch matids
		patch.patches[px].setMatID(1);

		ix+=4;
		px++;
		}
	for (i=0; i<4; i++) {
		a = (i*2+3)%8 + 8;
		b = i*2+1 + 8;
		c = (i*2) + 8;
		d = (i*2+2)%8 + 8;		
		patch.patches[px].SetType(PATCH_QUAD);
		patch.patches[px].setVerts(a, b, c, d);
		patch.patches[px].setVecs(
			Tang(a,2),Tang(b,0),Tang(b,1),Tang(c,1),
			Tang(c,0),Tang(d,2),Tang(d,1),Tang(a,1));
		patch.patches[px].setInteriors(ix, ix+1, ix+2, ix+3);
		patch.patches[px].smGroup = 2;
//watje 3-17-99 to support patch matids
		patch.patches[px].setMatID(0);

		ix+=4;
		px++;
		}
	
	if(genUVs) {
		patch.setTVert(0, UVVert(0.0f, 0.5f, 0.0f));
		patch.setTVert(1, UVVert(0.0f, 0.25f, 0.0f));
		patch.setTVert(2, UVVert(0.25f, 0.5f, 0.0f));
		patch.setTVert(3, UVVert(0.25f, 0.25f, 0.0f));
		patch.setTVert(4, UVVert(0.5f, 0.5f, 0.0f));
		patch.setTVert(5, UVVert(0.5f, 0.25f, 0.0f));
		patch.setTVert(6, UVVert(0.75f, 0.5f, 0.0f));
		patch.setTVert(7, UVVert(0.75f, 0.25f, 0.0f));
		patch.setTVert(8, UVVert(1.0f, 0.5f, 0.0f));
		patch.setTVert(9, UVVert(1.0f, 0.25f, 0.0f));

		patch.setTVert(10, UVVert(0.0f, 0.75f, 0.0f));
		patch.setTVert(11, UVVert(0.0f, 1.0f, 0.0f));
		patch.setTVert(12, UVVert(0.25f, 0.75f, 0.0f));
		patch.setTVert(13, UVVert(0.25f, 1.0f, 0.0f));
		patch.setTVert(14, UVVert(0.5f, 0.75f, 0.0f));
		patch.setTVert(15, UVVert(0.5f, 1.0f, 0.0f));
		patch.setTVert(16, UVVert(0.75f, 0.75f, 0.0f));
		patch.setTVert(17, UVVert(0.75f, 1.0f, 0.0f));
		patch.setTVert(18, UVVert(1.0f, 0.75f, 0.0f));
		patch.setTVert(19, UVVert(1.0f, 1.0f, 0.0f));

		patch.setTVert(20, UVVert(0.0f, 0.0f, 0.0f));
		patch.setTVert(21, UVVert(0.25f, 0.0f, 0.0f));
		patch.setTVert(22, UVVert(0.5f, 0.0f, 0.0f));
		patch.setTVert(23, UVVert(0.75f, 0.0f, 0.0f));
		patch.setTVert(24, UVVert(1.0f, 0.0f, 0.0f));

		patch.getTVPatch(0).setTVerts(10,0,2,12);
		patch.getTVPatch(1).setTVerts(12,2,4,14);
		patch.getTVPatch(2).setTVerts(14,4,6,16);
		patch.getTVPatch(3).setTVerts(16,6,8,18);
		patch.getTVPatch(4).setTVerts(21,3,1,20);
		patch.getTVPatch(5).setTVerts(22,5,3,21);
		patch.getTVPatch(6).setTVerts(23,7,5,22);
		patch.getTVPatch(7).setTVerts(24,9,7,23);
		patch.getTVPatch(8).setTVerts(1,3,2,0);
		patch.getTVPatch(9).setTVerts(3,5,4,2);
		patch.getTVPatch(10).setTVerts(5,7,6,4);
		patch.getTVPatch(11).setTVerts(7,9,8,6);
		patch.getTVPatch(12).setTVerts(13,11,10,12);
		patch.getTVPatch(13).setTVerts(15,13,12,14);
		patch.getTVPatch(14).setTVerts(17,15,14,16);
		patch.getTVPatch(15).setTVerts(19,17,16,18);
		}
			
	assert(patch.buildLinkages());
	patch.computeInteriors();
	patch.InvalidateGeomCache();
	}


#ifndef NO_NURBS

Object *
BuildNURBSTube(float radius1, float radius2, float height,
				BOOL sliceon, float pie1, float pie2, BOOL genUVs)
{
	BOOL flip = FALSE;

	if (radius1 < radius2)
		flip = !flip;

	if (height < 0.0f)
		flip = !flip;

	NURBSSet nset;

	Point3 origin(0,0,0);
	Point3 symAxis(0,0,1);
	Point3 refAxis(0,1,0);
	float startAngle = 0.0f;
	float endAngle = TWOPI;
	pie1 += HALFPI;
	pie2 += HALFPI;
	if (sliceon && pie1 != pie2) {
		float sweep = TWOPI - (pie2-pie1);
		if (sweep > TWOPI) sweep -= TWOPI;
		refAxis = Point3(Point3(1,0,0) * RotateZMatrix(pie2));
		endAngle = sweep;
	}


	// first the main surfaces
	NURBSCVSurface *s0 = new NURBSCVSurface();
	nset.AppendObject(s0);
	NURBSCVSurface *s1 = new NURBSCVSurface();
	nset.AppendObject(s1);

	s0->SetGenerateUVs(genUVs);
	s1->SetGenerateUVs(genUVs);

	s0->SetTextureUVs(0, 0, Point2(0.0f, 0.0f));
	s0->SetTextureUVs(0, 1, Point2(0.0f, 1.0f));
	s0->SetTextureUVs(0, 2, Point2(1.0f, 0.0f));
	s0->SetTextureUVs(0, 3, Point2(1.0f, 1.0f));

	s1->SetTextureUVs(0, 0, Point2(0.0f, 0.0f));
	s1->SetTextureUVs(0, 1, Point2(0.0f, 1.0f));
	s1->SetTextureUVs(0, 2, Point2(1.0f, 0.0f));
	s1->SetTextureUVs(0, 3, Point2(1.0f, 1.0f));

	s0->FlipNormals(!flip);
	s1->FlipNormals(flip);

	s0->Renderable(TRUE);
	s1->Renderable(TRUE);

	char bname[80];
	char sname[80];
	strcpy(bname, GetString(IDS_RB_TUBE));
	sprintf(sname, "%s%s01", bname, GetString(IDS_CT_SURF));
	s0->SetName(sname);

	sprintf(sname, "%s%s02", bname, GetString(IDS_CT_SURF));
	s1->SetName(sname);

	if (sliceon && pie1 != pie2) {
		// since GenNURBSCylinderSurface() returns a surface with more CVs
		// if it's larger we need to generate a single surface and make the
		// other one based on it but scaling the CVs based on the radius
		// ratio.
		float radius = (radius1 > radius2) ? radius1 : radius2;

		GenNURBSCylinderSurface(radius, height, origin, symAxis, refAxis,
						startAngle, endAngle, TRUE, *s0);
		GenNURBSCylinderSurface(radius, height, origin, symAxis, refAxis,
						startAngle, endAngle, TRUE, *s1);

		if (radius1 > radius2) {
			s0->MatID(3);
			s1->MatID(4);
		} else {
			s0->MatID(4);
			s1->MatID(3);
		}

		if (radius1 > radius2) {
			double scale = radius2/radius1;
			Matrix3 mat = ScaleMatrix(Point3(scale, scale, 1.0));
			int numU, numV;
			s1->GetNumCVs(numU, numV);
			for (int u = 0; u < numU; u++) {
				for (int v = 0; v < numV; v++) {
					Point3 pos = s1->GetCV(u, v)->GetPosition(0);
					Point3 npos = pos * mat;
					s1->GetCV(u, v)->SetPosition(0, npos);
				}
			}
		} else {
			double scale = radius1/radius2;
			Matrix3 mat = ScaleMatrix(Point3(scale, scale, 1.0));
			int numU, numV;
			s0->GetNumCVs(numU, numV);
			for (int u = 0; u < numU; u++) {
				for (int v = 0; v < numV; v++) {
					Point3 pos = s0->GetCV(u, v)->GetPosition(0);
					Point3 npos = pos * mat;
					s0->GetCV(u, v)->SetPosition(0, npos);
				}
			}
		}


#define F(s1, s2, s1r, s1c, s2r, s2c) \
		fuse.mSurf1 = (s1); \
		fuse.mSurf2 = (s2); \
		fuse.mRow1 = (s1r); \
		fuse.mCol1 = (s1c); \
		fuse.mRow2 = (s2r); \
		fuse.mCol2 = (s2c); \
		nset.mSurfFuse.Append(1, &fuse);

		NURBSFuseSurfaceCV fuse;

		// next the two caps
		for (int c = 0; c < 2; c++) {
			Point3 cen;
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
				Point3 in_edge, out_edge;
				if (c == 0) {
					in_edge = s0->GetCV(0, v)->GetPosition(0);
					out_edge = s1->GetCV(0, v)->GetPosition(0);
				} else {
					in_edge = s0->GetCV(s0->GetNumUCVs()-1, v)->GetPosition(0);
					out_edge = s1->GetCV(s1->GetNumUCVs()-1, v)->GetPosition(0);
				}
				NURBSControlVertex ncv;
				ncv.SetWeight(0, s0->GetCV(0, v)->GetWeight(0));
				for (int u = 0; u < 4; u++) {
					ncv.SetPosition(0, in_edge + ((out_edge - in_edge)*((float)u/3.0f)));
					s->SetCV(u, v, ncv);
				}
			}
			s->SetGenerateUVs(genUVs);

			s->SetTextureUVs(0, 0, Point2(0.0f, 1.0f));
			s->SetTextureUVs(0, 1, Point2(1.0f, 1.0f));
			s->SetTextureUVs(0, 2, Point2(0.0f, 0.0f));
			s->SetTextureUVs(0, 3, Point2(1.0f, 0.0f));

			if (c == 0) {
				s->FlipNormals(flip);
				s->MatID(2);
			} else {
				s->FlipNormals(!flip);
				s->MatID(1);
			}
			s->Renderable(TRUE);
			sprintf(sname, "%s%s%02d", bname, GetString(IDS_CT_CAP), c+1);
			s->SetName(sname);
		}

		NURBSCVSurface *s2 = (NURBSCVSurface *)nset.GetNURBSObject(2);
		NURBSCVSurface *s3 = (NURBSCVSurface *)nset.GetNURBSObject(3);

		// next the two pie slices
		for (c = 0; c < 2; c++) {
			NURBSCVSurface *s = new NURBSCVSurface();
			nset.AppendObject(s);
			// we'll match the tube in one dimension and the caps in the other.
			s->SetUOrder(s0->GetUOrder());
			int numKnots = s0->GetNumUKnots();
			s->SetNumUKnots(numKnots);
			for (int i = 0; i < numKnots; i++)
				s->SetUKnot(i, s0->GetUKnot(i));

			s->SetVOrder(s2->GetUOrder());
			numKnots = s2->GetNumUKnots();
			s->SetNumVKnots(numKnots);
			for (i = 0; i < numKnots; i++)
				s->SetVKnot(i, s2->GetUKnot(i));

			int s0u, s0v, s1u, s1v, s2u, s2v, s3u, s3v;
			s0->GetNumCVs(s0u, s0v);
			s1->GetNumCVs(s1u, s1v);
			s2->GetNumCVs(s2u, s2v);
			s3->GetNumCVs(s3u, s3v);
			int uNum = s0u, vNum = s2u;
			s->SetNumCVs(uNum, vNum);

			for (int v = 0; v < vNum; v++) {
				for (int u = 0; u < uNum; u++) {
					// we get get the ends from the caps and the edge from the main sheet
					if (u == 0) {  // bottom
						if (c == 0) {
							s->SetCV(u, v, *s2->GetCV(v, 0));
						} else {
							s->SetCV(u, v, *s2->GetCV(v, s2v-1));
						}
					} else if (u == uNum-1) { // top
						if (c == 0) {
							s->SetCV(u, v, *s3->GetCV(v, 0));
						} else {
							s->SetCV(u, v, *s3->GetCV(v, s3v-1));
						}
					} else { // middle
						// get x and y from a cap and z from the main sheet.
						Point3 p;
						if (c == 0)
							p = Point3(s2->GetCV(v, 0)->GetPosition(0).x,
										s2->GetCV(v, 0)->GetPosition(0).y,
										s0->GetCV(u, s0v-1)->GetPosition(0).z);
						else
							p = Point3(s2->GetCV(v, s2v-1)->GetPosition(0).x,
										s2->GetCV(v, s2v-1)->GetPosition(0).y,
										s0->GetCV(u, s0v-1)->GetPosition(0).z);
						NURBSControlVertex ncv;
						ncv.SetPosition(0, p);
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

			if (c == 0) {
				s->FlipNormals(flip);
				s->MatID(6);
			} else {
				s->FlipNormals(!flip);
				s->MatID(5);
			}
			s->Renderable(TRUE);
			sprintf(sname, "%s%s%02d", bname, GetString(IDS_CT_SLICE), c+1);
			s->SetName(sname);
		}

		NURBSCVSurface *s4 = (NURBSCVSurface *)nset.GetNURBSObject(4);
		NURBSCVSurface *s5 = (NURBSCVSurface *)nset.GetNURBSObject(5);

		// Fuse the edges
		for (int v = 0; v < s0->GetNumVCVs(); v++) {
			F(0, 2, 0, v, 0, v);
			F(0, 3, s0->GetNumUCVs()-1, v, 0, v);
			F(1, 2, 0, v, s2->GetNumUCVs()-1, v);
			F(1, 3, s1->GetNumUCVs()-1, v, s3->GetNumUCVs()-1, v);
		}
		// Now the caps
		for (int u = 0; u < s4->GetNumUCVs(); u++) {
			F(4, 0, u, 0, u, 0);
			F(4, 1, u, s4->GetNumVCVs()-1, u, 0);
			F(5, 0, u, 0, u, s0->GetNumVCVs()-1);
			F(5, 1, u, s4->GetNumVCVs()-1, u, s0->GetNumVCVs()-1);
		}
		for (v = 1; v < s4->GetNumVCVs()-1; v++) {
			F(4, 2, 0, v, v, 0);
			F(4, 3, s4->GetNumUCVs()-1, v, v, 0);
			F(5, 2, 0, v, v, s2->GetNumVCVs()-1);
			F(5, 3, s5->GetNumUCVs()-1, v, v, s3->GetNumVCVs()-1);
		}
	} else {
		GenNURBSCylinderSurface(radius1, height, origin, symAxis, refAxis,
						startAngle, endAngle, FALSE, *s0);
		GenNURBSCylinderSurface(radius2, height, origin, symAxis, refAxis,
						startAngle, endAngle, FALSE, *s1);

		if (radius1 > radius2) {
			s0->MatID(3);
			s1->MatID(4);
		} else {
			s0->MatID(4);
			s1->MatID(3);
		}

		char sname[80];
		// now 4 iso curves 
		NURBSIsoCurve *iso_0_0 = new NURBSIsoCurve();
		nset.AppendObject(iso_0_0);
		iso_0_0->SetParent(0);
		iso_0_0->SetDirection(FALSE);
		iso_0_0->SetTrim(FALSE);
		iso_0_0->SetParam(0, 0.0);
		sprintf(sname, "%s%s%02d", bname, GetString(IDS_CT_CAP), 0);
		iso_0_0->SetName(sname);

		NURBSIsoCurve *iso_0_1 = new NURBSIsoCurve();
		nset.AppendObject(iso_0_1);
		iso_0_1->SetParent(0);
		iso_0_1->SetDirection(FALSE);
		iso_0_1->SetTrim(FALSE);
		iso_0_1->SetParam(0, 1.0);
		sprintf(sname, "%s%s%02d", bname, GetString(IDS_CT_CAP), 1);
		iso_0_1->SetName(sname);

		NURBSIsoCurve *iso_1_0 = new NURBSIsoCurve();
		nset.AppendObject(iso_1_0);
		iso_1_0->SetParent(1);
		iso_1_0->SetDirection(FALSE);
		iso_1_0->SetTrim(FALSE);
		iso_1_0->SetParam(0, 0.0);
		sprintf(sname, "%s%s%02d", bname, GetString(IDS_CT_CAP), 2);
		iso_1_0->SetName(sname);

		NURBSIsoCurve *iso_1_1 = new NURBSIsoCurve();
		nset.AppendObject(iso_1_1);
		iso_1_1->SetParent(1);
		iso_1_1->SetDirection(FALSE);
		iso_1_1->SetTrim(FALSE);
		iso_1_1->SetParam(0, 1.0);
		sprintf(sname, "%s%s%02d", bname, GetString(IDS_CT_CAP), 3);
		iso_1_1->SetName(sname);

		// now 2 ruled surfaces
		NURBSRuledSurface *cap0 = new NURBSRuledSurface();
		nset.AppendObject(cap0);
		cap0->SetGenerateUVs(genUVs);
		cap0->SetParent(0, 2);
		cap0->SetParent(1, 4);
		cap0->FlipNormals(!flip);
		cap0->Renderable(TRUE);
		cap0->MatID(2);
		sprintf(sname, "%s%s%02d", bname, GetString(IDS_CT_CAP), 0);
		cap0->SetName(sname);

		NURBSRuledSurface *cap1 = new NURBSRuledSurface();
		nset.AppendObject(cap1);
		cap1->SetGenerateUVs(genUVs);
		cap1->SetParent(0, 3);
		cap1->SetParent(1, 5);
		cap1->FlipNormals(flip);
		cap1->Renderable(TRUE);
		cap1->MatID(1);
		sprintf(sname, "%s%s%02d", bname, GetString(IDS_CT_CAP), 1);
		cap1->SetName(sname);
	}

	Matrix3 mat;
	mat.IdentityMatrix();
	Object *ob = CreateNURBSObject(NULL, &nset, mat);
	return ob;
}

extern Object *BuildNURBSCylinder(float radius, float height,
				BOOL sliceon, float pie1, float pie2, BOOL genUVs);

#endif

Object* TubeObject::ConvertToType(TimeValue t, Class_ID obtype)
	{
#ifndef NO_PATCHES
	if (obtype == patchObjectClassID) {
		Interval valid = FOREVER;
		float radius1, radius2, height;
		int genUVs;
		pblock->GetValue(PB_RADIUS,t,radius1,valid);
		pblock->GetValue(PB_RADIUS2,t,radius2,valid);
		pblock->GetValue(PB_HEIGHT,t,height,valid);
		pblock->GetValue(PB_GENUVS,t,genUVs,valid);
		PatchObject *ob = new PatchObject();
		BuildTubePatch(ob->patch,radius1,radius2,height,genUVs);
		ob->SetChannelValidity(TOPO_CHAN_NUM,valid);
		ob->SetChannelValidity(GEOM_CHAN_NUM,valid);
		ob->UnlockObject();
		return ob;
	}
#endif
#ifndef NO_NURBS
    if (obtype == EDITABLE_SURF_CLASS_ID) {
		Interval valid = FOREVER;
		float radius1, radius2, height, pie1, pie2;
		int sliceon, genUVs;
		pblock->GetValue(PB_RADIUS,t,radius1,valid);
		pblock->GetValue(PB_RADIUS2,t,radius2,valid);
		pblock->GetValue(PB_HEIGHT,t,height,valid);	
		pblock->GetValue(PB_PIESLICE1,t,pie1,valid);	
		pblock->GetValue(PB_PIESLICE2,t,pie2,valid);	
		pblock->GetValue(PB_SLICEON,t,sliceon,valid);
		pblock->GetValue(PB_GENUVS,t,genUVs,valid);
		if (radius1 < 0.0f) radius1 = 0.0f;
		if (radius2 < 0.0f) radius2 = 0.0f;
		Object *ob;
		if (radius1 == 0.0f || radius2 == 0.0f) {
			float radius;
			if (radius1 == 0.0f) radius = radius2;
			else radius = radius1;
			ob = BuildNURBSCylinder(radius, height,
				sliceon, pie1 - HALFPI, pie2 - HALFPI, genUVs);
		} else
			ob = BuildNURBSTube(radius1, radius2, height, sliceon, pie1, pie2, genUVs);
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
		pblock->GetValue(PB_RADIUS,t,radius1,valid);
		pblock->GetValue(PB_RADIUS2,t,radius2,valid);
		pblock->GetValue(PB_HEIGHT,t,height,valid);	
		pblock->GetValue(PB_PIESLICE1,t,pie1,valid);	
		pblock->GetValue(PB_PIESLICE2,t,pie2,valid);	
		pblock->GetValue(PB_SLICEON,t,sliceon,valid);
		pblock->GetValue(PB_GENUVS,t,genUVs,valid);
		pblock->GetValue(PB_SEGMENTS,t,sides,valid);
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
				bool res;
				if (radius1 == 0.0f || radius2 == 0.0f) {
					float radius;
					if (radius1 == 0.0f) radius = radius2;
					else radius = radius1;
					res = cacheptr->createCylinder(height, radius, sides, smooth);
				}
				else
					res = cacheptr->createPipe(height, radius1, radius2, sides);
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

int TubeObject::CanConvertToType(Class_ID obtype)
	{
#ifdef DESIGN_VER
	if(obtype == GENERIC_AMSOLID_CLASS_ID) return 1;
#endif
#ifndef NO_PATCHES
    if(obtype == patchObjectClassID) return 1;
#endif
#ifndef NO_NURBS
    if(obtype == EDITABLE_SURF_CLASS_ID) return 1;
#endif
	if(obtype==defObjectClassID || obtype==triObjectClassID) return 1;
    return SimpleObject::CanConvertToType(obtype);
	}
	

void TubeObject::GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist)
{
    Object::GetCollapseTypes(clist, nlist);
#ifndef NO_NURBS
    Class_ID id = EDITABLE_SURF_CLASS_ID;
    TSTR *name = new TSTR(GetString(IDS_SM_NURBS_SURFACE));
    clist.Append(1,&id);
    nlist.Append(1,&name);
#endif
}


class TubeObjCreateCallBack: public CreateMouseCallBack {
	TubeObject *ob;	
	Point3 p0, p1, p2;
	IPoint2 sp0,sp1,sp2;	
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		void SetObj(TubeObject *obj) { ob = obj; }
	};



int TubeObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
	float r;
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
				ob->pblock->SetValue(PB_RADIUS2,0,0.0f);
				ob->pblock->SetValue(PB_HEIGHT,0,0.1f);
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
				sp1 = m;							   
				#ifdef _3D_CREATE	
					p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				#else	
					p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				#endif
				if (ob->dlgCreateMeth) {	
					// radius	
					r = Length(p1-p0);
					mat.SetTrans(p0);
				} else {
					// diameter
					Point3 center = (p0+p1)/float(2);
					r = Length(center-p0);
					mat.SetTrans(center);  // Modify Node's transform
					}

				if (msg==MOUSE_POINT) {
					ob->suspendSnap = FALSE;
					if (Length(m-sp0)<3 || Length(p1-p0)<0.1f)
						return CREATE_ABORT;
					}
				
				ob->pblock->SetValue(PB_RADIUS,0,r);
				ob->pblock->SetValue(PB_RADIUS2,0,r+0.1f);
				ob->pmapParam->Invalidate();
				
				if (flags&MOUSE_CTRL) {
					float ang = (float)atan2(p1.y-p0.y,p1.x-p0.x);					
					mat.PreRotateZ(ob->ip->SnapAngle(ang));
					}				
				break;
			
			case 2:									
				mat.IdentityMatrix();
				//mat.PreRotateZ(HALFPI);
				sp2 = m;							   
				#ifdef _3D_CREATE	
					p2 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				#else	
					p2 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				#endif

				if (ob->dlgCreateMeth) {	
					// radius	
					r = Length(p2-p0);
					mat.SetTrans(p0);
				} else {
					// diameter
					Point3 center = (p2+p0)/float(2);
					r = Length(center-p0);
					mat.SetTrans(center);  // Modify Node's transform
					}
				
				ob->pblock->SetValue(PB_RADIUS2,0,r);
				ob->pmapParam->Invalidate();
				
				if (flags&MOUSE_CTRL) {
					float ang = (float)atan2(p2.y-p0.y,p2.x-p0.x);					
					mat.PreRotateZ(ob->ip->SnapAngle(ang));
					}
				
				break;					   
			
			case 3:
				{
				float h = vpt->SnapLength(vpt->GetCPDisp(p2,Point3(0,0,1),sp2,m));
				ob->pblock->SetValue(PB_HEIGHT,0,h);
				ob->pmapParam->Invalidate();
				if (msg==MOUSE_POINT) {
					ob->suspendSnap = FALSE;
					return CREATE_STOP;
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


static TubeObjCreateCallBack tubeCreateCB;

CreateMouseCallBack* TubeObject::GetCreateMouseCallBack() {
	tubeCreateCB.SetObj(this);
	return(&tubeCreateCB);
	}

BOOL TubeObject::OKtoDisplay(TimeValue t) 
	{
	return TRUE;
	}


// From ParamArray
BOOL TubeObject::SetValue(int i, TimeValue t, int v) 
	{
	switch (i) {
		case PB_CREATEMETHOD: dlgCreateMeth = v; break;
		}		
	return TRUE;
	}

BOOL TubeObject::SetValue(int i, TimeValue t, float v)
	{
	switch (i) {				
		case PB_TI_RADIUS1: crtRadius1 = v; break;
		case PB_TI_RADIUS2: crtRadius2 = v; break;
		case PB_TI_HEIGHT: crtHeight = v; break;
		}	
	return TRUE;
	}

BOOL TubeObject::SetValue(int i, TimeValue t, Point3 &v) 
	{
	switch (i) {
		case PB_TI_POS: crtPos = v; break;
		}		
	return TRUE;
	}

BOOL TubeObject::GetValue(int i, TimeValue t, int &v, Interval &ivalid) 
	{
	switch (i) {
		case PB_CREATEMETHOD: v = dlgCreateMeth; break;
		}
	return TRUE;
	}

BOOL TubeObject::GetValue(int i, TimeValue t, float &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_RADIUS1: v = crtRadius1; break;
		case PB_TI_RADIUS2: v = crtRadius2; break;
		case PB_TI_HEIGHT: v = crtHeight; break;
		}
	return TRUE;
	}

BOOL TubeObject::GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_POS: v = crtPos; break;		
		}
	return TRUE;
	}


void TubeObject::InvalidateUI() 
	{
	if (pmapParam) pmapParam->Invalidate();
	}

ParamDimension *TubeObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_RADIUS:		return stdWorldDim;
		case PB_RADIUS2:	return stdWorldDim;		
		case PB_HEIGHT:		return stdWorldDim;
		case PB_SEGMENTS:	return stdSegmentsDim;
		case PB_CAPSEGMENTS:return stdSegmentsDim;
		case PB_SIDES:		return stdSegmentsDim;
		case PB_SMOOTH:		return stdNormalizedDim;
		case PB_SLICEON:	return stdNormalizedDim;
		case PB_PIESLICE1:	return stdAngleDim;
		case PB_PIESLICE2:	return stdAngleDim;
		default: return defaultDim;
		}
	}

TSTR TubeObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_RADIUS:		return GetString(IDS_RB_RADIUS1);
		case PB_RADIUS2:	return GetString(IDS_RB_RADIUS2);
		case PB_HEIGHT:		return GetString(IDS_RB_HEIGHT);
		case PB_CAPSEGMENTS:return GetString(IDS_RB_CAPSEGMENTS);
		case PB_SEGMENTS:	return GetString(IDS_RB_SIDES);
		case PB_SIDES:		return GetString(IDS_RB_HEIGHTSEGS);
		case PB_SMOOTH:		return GetString(IDS_RB_SMOOTH);
		case PB_SLICEON:	return GetString(IDS_RB_SLICEON);
		case PB_PIESLICE1:	return GetString(IDS_RB_SLICEFROM);
		case PB_PIESLICE2:	return GetString(IDS_RB_SLICETO);
		default: return TSTR(_T(""));
		}
	}

RefTargetHandle TubeObject::Clone(RemapDir& remap) 
	{
	TubeObject* newob = new TubeObject();	
	newob->ReplaceReference(0,pblock->Clone(remap));	
	newob->ivalid.SetEmpty();	
	BaseClone(this, newob, remap);
	return(newob);
	}

#endif // NO_OBJECT_STANDARD_PRIMITIVES
