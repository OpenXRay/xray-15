/**********************************************************************
 *<
	FILE: oiltnk.cpp - builds OilTank objects
 	CREATED BY:  Audrey Peterson

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#include "solids.h"
#include "iparamm.h"
#include "Simpobj.h"

static Class_ID OILTNK_CLASS_ID(0x210e642a, 0x22f11ef8);
class OilTnkObject : public SimpleObject, public IParamArray {
	public:
		// Class vars
		static IParamMap *pmapCreate;
		static IParamMap *pmapTypeIn;
		static IParamMap *pmapParam;		
		static IObjParam *ip;
		static float crtRadius, crtCapHeight,crtBlend;
		static float crtHeight,crtSliceFrom,crtSliceTo;
		static int dlgHSegs, dlgSides,crtCenters;
		static int dlgCreateMeth;
		static int dlgSmooth, dlgSlice;
		static Point3 crtPos;
		BOOL increate;
		
		OilTnkObject();		

		// From Object
		int CanConvertToType(Class_ID obtype);
		Object* ConvertToType(TimeValue t, Class_ID obtype);
				
		// From BaseObject
		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);		
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		TCHAR *GetObjectName() { return GetString(IDS_RB_OILTNK); }
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);
				
		// Animatable methods		
		void DeleteThis() { delete this; }
		Class_ID ClassID() { return OILTNK_CLASS_ID; }  		
				
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
#define MIN_SLICE	float(-1.0E30)
#define MAX_SLICE	float( 1.0E30)

#define DEF_SEGMENTS 	1
#define DEF_SIDES		12

#define DEF_RADIUS		float(0.0)
#define DEF_HEIGHT		float(0.01)
#define DEF_FILLET		float(0.01)

#define SMOOTH_ON		1
#define SMOOTH_OFF		0



//--- ClassDescriptor and class vars ---------------------------------

class OilTnkClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new OilTnkObject; }
	const TCHAR *	ClassName() { return GetString(IDS_AP_OILTNK_CLASS); }
	SClass_ID		SuperClassID() { return GEOMOBJECT_CLASS_ID; }
	Class_ID		ClassID() { return OILTNK_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_RB_EXTENDED);  }
	void			ResetClassParams(BOOL fileReset);
	};

static OilTnkClassDesc OilTnkDesc;

ClassDesc* GetOilTnkDesc() { return &OilTnkDesc; }

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// class variables for OilTnk class.
IObjParam *OilTnkObject::ip         = NULL;
int OilTnkObject::dlgHSegs			= DEF_SEGMENTS;
int OilTnkObject::dlgSides          = DEF_SIDES;
int OilTnkObject::dlgCreateMeth     = 1; // create_radius
int OilTnkObject::dlgSmooth         = SMOOTH_ON;
int OilTnkObject::dlgSlice          = 0;
int OilTnkObject::crtCenters          = 0;
IParamMap *OilTnkObject::pmapCreate = NULL;
IParamMap *OilTnkObject::pmapTypeIn = NULL;
IParamMap *OilTnkObject::pmapParam  = NULL;
Point3 OilTnkObject::crtPos         = Point3(0,0,0);
float OilTnkObject::crtRadius       = 0.0f;
float OilTnkObject::crtHeight       = 0.0f;
float OilTnkObject::crtCapHeight    = 0.0f;
float OilTnkObject::crtSliceFrom    = 0.0f;
float OilTnkObject::crtSliceTo    = 0.0f;
float OilTnkObject::crtBlend	= 0.0f;

void OilTnkClassDesc::ResetClassParams(BOOL fileReset)
	{ OilTnkObject::dlgHSegs			= DEF_SEGMENTS;
	  OilTnkObject::dlgSides          = DEF_SIDES;
	  OilTnkObject::crtBlend			=0.0f;
	  OilTnkObject::dlgCreateMeth     = 1; // create_radius
	  OilTnkObject::dlgSmooth         = SMOOTH_ON;
	  OilTnkObject::dlgSlice          = 0;
	  OilTnkObject::crtCenters          = 0;
	  OilTnkObject::crtRadius       = 0.0f;
	  OilTnkObject::crtHeight       = 0.0f;
	  OilTnkObject::crtCapHeight    = 0.0f;
	  OilTnkObject::crtSliceFrom    = 0.0f;
	  OilTnkObject::crtSliceTo    = 0.0f;
	  OilTnkObject::crtPos         = Point3(0,0,0);
	}

//--- Parameter map/block descriptors -------------------------------

// Parameter block indices
#define PB_RADIUS		0
#define PB_CAPHEIGHT	1
#define PB_HEIGHT		2
#define PB_CENTERS		3
#define PB_BLEND		4
#define PB_SIDES		5
#define PB_HSEGS		6
#define PB_SMOOTHON		7
#define PB_SLICEON		8
#define PB_SLICEFROM	9
#define PB_SLICETO		10
#define PB_GENUVS		11

// Non-parameter block indices
#define PB_CREATEMETHOD		0
#define PB_TI_POS			1
#define PB_TI_RADIUS		2
#define PB_TI_CAPHEIGHT		3
#define PB_TI_HEIGHT		4
#define PB_TI_CENTERS		5
#define PB_TI_BLEND			6
// dont worry about the CREATE button here
#define BMIN_HEIGHT		float(0.1)
#define BMAX_HEIGHT		float(1.0E30)
#define BMIN_LENGTH		float(0.1)
#define BMAX_LENGTH		float(1.0E30)
//
//
//	Creation method

static int createMethIDs[] = {IDC_UCYLS_BYDIA,IDC_UCYLS_BYRAD};

static ParamUIDesc descCreate[] = {
	// Diameter/radius
	ParamUIDesc(PB_CREATEMETHOD,TYPE_RADIO,createMethIDs,2)
	};
#define CREATEDESC_LENGTH 1


//
//
// Type in

static int centerIDs[] = {IDC_OT_LENOVERALL,IDC_OT_LENCENTERS};
static int capIDs[] = {IDC_OT_ONECAP,IDC_OT_TWOCAPS};

static ParamUIDesc descTypeIn[] = {
	
	// Position
	ParamUIDesc(
		PB_TI_POS,
		EDITTYPE_UNIVERSE,
		IDC_OT_POSX,IDC_OT_POSXSPIN,
		IDC_OT_POSY,IDC_OT_POSYSPIN,
		IDC_OT_POSZ,IDC_OT_POSZSPIN,
		float(-1.0E30),float(1.0E30),
		SPIN_AUTOSCALE),
	
	// Radius
	ParamUIDesc(
		PB_TI_RADIUS,
		EDITTYPE_UNIVERSE,
		IDC_OT_RADIUS,IDC_OT_RADIUSSPIN,
		MIN_RADIUS,MAX_RADIUS,
		SPIN_AUTOSCALE),
	
	// Cap Height
	ParamUIDesc(
		PB_TI_CAPHEIGHT,
		EDITTYPE_UNIVERSE,
		IDC_OT_CAPHGT,IDC_OT_CAPHGTSPIN,
		BMIN_HEIGHT,BMAX_HEIGHT,
		SPIN_AUTOSCALE),

	// Height
	ParamUIDesc(
		PB_TI_HEIGHT,
		EDITTYPE_UNIVERSE,
		IDC_OT_HEIGHT,IDC_OT_HEIGHTSPIN,
		MIN_HEIGHT,BMAX_HEIGHT,
		SPIN_AUTOSCALE),

	// Radio Buttons - centers
	ParamUIDesc(PB_TI_CENTERS,TYPE_RADIO,centerIDs,2),
	// Blend
	ParamUIDesc(
		PB_TI_BLEND,
		EDITTYPE_UNIVERSE,
		IDC_OT_BLEND,IDC_OT_BLENDSPIN,
		0.0f,BMAX_LENGTH,
		SPIN_AUTOSCALE),

	};
#define TYPEINDESC_LENGTH 6

//
//
// Parameters


// DANGER DANGER DANGER I THINK THIS IS A REDEF BUT I HAVE NO
//EXAMPLES WITH RADIO BUTTONS APPEARING IN BOTH PARAM AND NONPARAM SCREENS

static ParamUIDesc descParam[] = {
	// Radius
	ParamUIDesc(
		PB_RADIUS,
		EDITTYPE_UNIVERSE,
		IDC_OT_RADIUS,IDC_OT_RADIUSSPIN,
		MIN_RADIUS,MAX_RADIUS,
		SPIN_AUTOSCALE),
	
	// Cap Height
	ParamUIDesc(
		PB_CAPHEIGHT,
		EDITTYPE_UNIVERSE,
		IDC_OT_CAPHGT,IDC_OT_CAPHGTSPIN,
		BMIN_HEIGHT,BMAX_HEIGHT,
		SPIN_AUTOSCALE),

	// Height
	ParamUIDesc(
		PB_HEIGHT,
		EDITTYPE_UNIVERSE,
		IDC_OT_HEIGHT,IDC_OT_HEIGHTSPIN,
		MIN_HEIGHT,BMAX_HEIGHT,
		SPIN_AUTOSCALE),

	// Radio Buttons - centers
	ParamUIDesc(PB_CENTERS,TYPE_RADIO,centerIDs,2),

	// Blend
	ParamUIDesc(
		PB_BLEND,
		EDITTYPE_UNIVERSE,
		IDC_OT_BLEND,IDC_OT_BLENDSPIN,
		0.0f,BMAX_LENGTH,
		SPIN_AUTOSCALE),

	// Sides
	ParamUIDesc(
		PB_SIDES,
		EDITTYPE_INT,
		IDC_OT_SIDES,IDC_OT_SIDESSPIN,
		(float)MIN_SIDES,(float)MAX_SIDES,
		0.1f),
	
	// Height Segments
	ParamUIDesc(
		PB_HSEGS,
		EDITTYPE_INT,
		IDC_OT_HGTSEGS,IDC_OT_HGTSEGSSPIN,
		(float)MIN_SEGMENTS,(float)MAX_SEGMENTS,
		0.1f),

	// Gen Smoothing
	ParamUIDesc(PB_SMOOTHON,TYPE_SINGLECHEKBOX,IDC_OT_SMOOTH),			

	// Gen Slice
	ParamUIDesc(PB_SLICEON,TYPE_SINGLECHEKBOX,IDC_OT_SLICEON),			

	// Pie slice from
	ParamUIDesc(
		PB_SLICEFROM,
		EDITTYPE_FLOAT,
		IDC_OT_SLICE1,IDC_OT_SLICE1SPIN,
		MIN_SLICE,MAX_SLICE,		
		0.5f,
		stdAngleDim),

	// Pie slice to
	ParamUIDesc(
		PB_SLICETO,
		EDITTYPE_FLOAT,
		IDC_OT_SLICE2,IDC_OT_SLICE2SPIN,
		MIN_SLICE,MAX_SLICE,		
		0.5f,
		stdAngleDim),

	// Gen UVs
	ParamUIDesc(PB_GENUVS,TYPE_SINGLECHEKBOX,IDC_GENTEXTURE),			

	};
#define PARAMDESC_LENGTH 12


// variable type, NULL, animatable, number
ParamBlockDescID OilTnkdescVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 },
	{ TYPE_INT, NULL, FALSE, 3 }, 
	{ TYPE_FLOAT, NULL, TRUE, 4},
	{ TYPE_INT, NULL, TRUE, 5 }, 
	{ TYPE_INT, NULL, TRUE, 6 },
	{ TYPE_INT, NULL, TRUE, 7 }, 
	{ TYPE_INT, NULL, TRUE, 8 },
	{ TYPE_FLOAT, NULL, TRUE, 9 },
	{ TYPE_FLOAT, NULL, TRUE, 10 },
	{ TYPE_INT, NULL, FALSE, 11 } 
	};

#define PBLOCK_LENGTH	12

#define NUM_OLDVERSIONS	0

#define CURRENT_VERSION	0
static ParamVersionDesc curVersion(OilTnkdescVer0,PBLOCK_LENGTH,CURRENT_VERSION);


//--- TypeInDlgProc --------------------------------

class OilTnkTypeInDlgProc : public ParamMapUserDlgProc {
	public:
		OilTnkObject *ob;

		OilTnkTypeInDlgProc(OilTnkObject *o) {ob=o;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

BOOL OilTnkTypeInDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_OT_CREATE: {
					if (ob->crtRadius==0.0) return TRUE;
					
					// We only want to set the value if the object is 
					// not in the scene.
					if (ob->TestAFlag(A_OBJ_CREATING)) {
						ob->pblock->SetValue(PB_RADIUS,0,ob->crtRadius);
						ob->pblock->SetValue(PB_HEIGHT,0,ob->crtHeight);
						ob->pblock->SetValue(PB_CAPHEIGHT,0,ob->crtCapHeight);
						ob->pblock->SetValue(PB_BLEND,0,ob->crtBlend);
						ob->pblock->SetValue(PB_CENTERS,0,ob->crtCenters);
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

void FixCapLimits(IParamBlock *pblock,TimeValue t,HWND hWnd,BOOL increate)
{ float radius,height,minc,capheight,hh;
  BOOL con;

	pblock->GetValue(PB_CAPHEIGHT,(increate?0:t),capheight,FOREVER);
	pblock->GetValue(PB_RADIUS,t,radius,FOREVER);
	pblock->GetValue(PB_CENTERS,t,con,FOREVER);
	pblock->GetValue(PB_HEIGHT,t,height,FOREVER);
	height=(float)fabs(height);
	minc=0.025f*radius;
	if (con) height+=2.0f*capheight;
	hh=height/2.0f;
	float maxh=(hh>radius?radius:hh)*0.99f;
	if (hWnd)
	{ ISpinnerControl *spin2 = GetISpinner(GetDlgItem(hWnd,IDC_OT_CAPHGTSPIN));
	  spin2->SetLimits(minc,maxh,FALSE);
	  ReleaseISpinner(spin2);
	}
	if (capheight>maxh) pblock->SetValue(PB_CAPHEIGHT,(increate?0:t),maxh);
	if (capheight<minc) pblock->SetValue(PB_CAPHEIGHT,(increate?0:t),minc);
}
void FixBlendLimits(IParamBlock *pblock,TimeValue t,HWND hWnd,BOOL increate)
{ float maxb,height,capheight,blend;
  BOOL con;

	pblock->GetValue(PB_CAPHEIGHT,t,capheight,FOREVER);
	pblock->GetValue(PB_CENTERS,t,con,FOREVER);
	pblock->GetValue(PB_HEIGHT,t,height,FOREVER);
	pblock->GetValue(PB_BLEND,(increate?0:t),blend,FOREVER);
	height=(float)fabs(height);
	if (con) height+=2.0f*capheight;
	maxb=(height-2.0f*capheight)/2.0f;
	if (hWnd)
	{ ISpinnerControl *spin2 = GetISpinner(GetDlgItem(hWnd,IDC_OT_BLENDSPIN));
	  spin2->SetLimits(0.0f,maxb,FALSE);
	  ReleaseISpinner(spin2);
	}
	if (blend>maxb) pblock->SetValue(PB_BLEND,(increate?0:t),maxb);
}

class OilTnkCapValsDlgProc : public ParamMapUserDlgProc {
	public:
		OilTnkObject *ob;

		OilTnkCapValsDlgProc(OilTnkObject *o) {ob=o;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

BOOL OilTnkCapValsDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{ 	switch (msg) {
		case CC_SPINNER_CHANGE:
			switch ( LOWORD(wParam) ) {
				case IDC_OT_CAPHGTSPIN:
					FixCapLimits(ob->pblock,t,hWnd,ob->increate);
			return TRUE;
				case IDC_OT_BLENDSPIN:
					FixBlendLimits(ob->pblock,t,hWnd,ob->increate);
			return TRUE;
				}
		}
	return FALSE;
	}
//--- OilTnk methods -------------------------------

OilTnkObject::OilTnkObject() 
	{
	MakeRefByID(FOREVER, 0, CreateParameterBlock(OilTnkdescVer0, PBLOCK_LENGTH, CURRENT_VERSION));
	
	pblock->SetValue(PB_RADIUS,0,crtRadius);
	pblock->SetValue(PB_CAPHEIGHT,0,crtCapHeight);
	pblock->SetValue(PB_HEIGHT,0,crtHeight);
	pblock->SetValue(PB_CENTERS,0,crtCenters);
	pblock->SetValue(PB_BLEND,0,crtBlend);
	pblock->SetValue(PB_SIDES,0,dlgSides);
	pblock->SetValue(PB_HSEGS,0,dlgHSegs);
	pblock->SetValue(PB_SMOOTHON,0,dlgSmooth);
	pblock->SetValue(PB_SLICEON,0,dlgSlice);
	pblock->SetValue(PB_SLICEFROM,0,crtSliceFrom);
	pblock->SetValue(PB_SLICETO,0,crtSliceTo);

	pblock->SetValue(PB_GENUVS,0, TRUE);				

	increate=FALSE;
	}

IOResult OilTnkObject::Load(ILoad *iload) 
	{
	return IO_OK;
	}


void OilTnkObject::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
	{
	SimpleObject::BeginEditParams(ip,flags,prev);
	this->ip = ip;

	if (pmapCreate && pmapParam) {
		
		// Left over from last OilTnk ceated
		pmapCreate->SetParamBlock(this);
		pmapTypeIn->SetParamBlock(this);
		pmapParam->SetParamBlock(pblock);
	} else {
		
		if (flags&BEGIN_EDIT_CREATE) {
			pmapCreate = CreateCPParamMap(
				descCreate,CREATEDESC_LENGTH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_UREVS1),
				GetString(IDS_RB_CREATE_DIALOG),
				0);

			pmapTypeIn = CreateCPParamMap(
				descTypeIn,TYPEINDESC_LENGTH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_OILTANK2),
				GetString(IDS_RB_KEYBOARDENTRY),
				APPENDROLL_CLOSED);			
			}

		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_OILTANK3),
			GetString(IDS_AP_PARAMETERS),
			0);
		}

	if(pmapTypeIn) {
		// A callback for the type in.
		pmapTypeIn->SetUserDlgProc(new OilTnkTypeInDlgProc(this));
		}
	if(pmapParam) {
		// A callback for the type in.
		pmapParam->SetUserDlgProc(new OilTnkCapValsDlgProc(this));
		}
	}
		
void OilTnkObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
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
	pblock->GetValue(PB_HSEGS,ip->GetTime(),dlgHSegs,FOREVER);
	pblock->GetValue(PB_SIDES,ip->GetTime(),dlgSides,FOREVER);	
	}

/*void BoxObject::SetParams(float radius, float capheight, float height,
						  BOOL centers, BOOL ncaps, float blend,
						  int sides, int hsegs, BOOL smooth, BOOL slice, 
						  float slice1, float slice2, BOOL genUV) {
	pblock->SetValue(PB_RADIUS,0,radius);
	pblock->SetValue(PB_CAPHEIGHT,0,capheight);
	pblock->SetValue(PB_HEIGHT,0,height);
	pblock->SetValue(PB_CENTERS,0,centers);
	pblock->SetValue(PB_BLEND,0,blend);
	pblock->SetValue(PB_SIDES,0,sides);
	pblock->SetValue(PB_HSEGS,0,hsegs);
	pblock->SetValue(PB_SMOOTH,0,smooth);
	pblock->SetValue(PB_SLICEON,0,slice);
	pblock->SetValue(PB_SLICEFROM,0,slice1);
	pblock->SetValue(PB_SLICETO,0,slice2);
	pblock->SetValue(PB_GENUVS,0,genUV);
	} 
*/


void BuildOilTnkMesh(Mesh &mesh,
		int segs, int smooth, int llsegs, int doPie, float radius1, float blend,
		float height, float capheight, float pie1, float pie2, int genUVs)
	{
	Point3 p;		   
	BOOL minush=(height<0.0f);
	if (minush) height=-height;
	int ix,jx,ic = 1;
	int nf=0,nv=0, lsegs,VertexPerLevel,capsegs=(int)(segs/3.0f),csegs=(blend>0?1:0);
	float delta,ang;	
	float totalPie, startAng = 0.0f;	

	if (doPie) doPie = 1;
	else doPie = 0; 

	lsegs = llsegs-1 + 2*capsegs;

	// Make pie2 < pie1 and pie1-pie2 < TWOPI
	while (pie1 < pie2) pie1 += TWOPI;
	while (pie1 > pie2+TWOPI) pie1 -= TWOPI;
	if (pie1==pie2) totalPie = TWOPI;
	else totalPie = pie1-pie2;		
	int nfaces,ntverts,levels=csegs*2+(llsegs-1);
	int capv=segs,sideedge=capsegs+csegs,*edgelstr,*edgelstl,totlevels;
    // capv=vertex in one cap layer
	totlevels=levels+capsegs*2+2;
	int	tvinslice=totlevels+totlevels-2;
	if (doPie) {
		delta    = totalPie/(float)(segs);
		startAng = pie2; capv++;
		VertexPerLevel=segs+2;
		nfaces=2*segs*(levels+1)+(sideedge+llsegs)*4;
		ntverts=tvinslice+2*(segs+1);
		// 2 faces between every 2 vertices, with 2 ends, except in central cap)
	} else {
		delta = (float)2.0*PI/(float)segs;
		VertexPerLevel=segs;
		nfaces=2*segs*(levels+1);
		ntverts=2*(segs+1)+llsegs-1;
	}

	edgelstl=new int[totlevels];
	edgelstr=new int[totlevels];
	int lastlevel=totlevels-1,dcapv=capv-1,dvertper=VertexPerLevel-1;
	edgelstr[0]=0;edgelstl[0]=0;
	edgelstr[1]=1;
	edgelstl[1]=capv;
	for (int i=2;i<=sideedge;i++)
	{ edgelstr[i]=edgelstr[i-1]+capv;
	  edgelstl[i]=edgelstr[i]+dcapv;
	}
	while ((i<lastlevel)&&(i<=totlevels-sideedge))
	{ edgelstr[i]=edgelstr[i-1]+VertexPerLevel;
	  edgelstl[i]=edgelstr[i]+dcapv;
	  i++;
	}
	while (i<lastlevel)
	{ edgelstr[i]=edgelstr[i-1]+capv;
	  edgelstl[i]=edgelstr[i]+dcapv;
	  i++;
	}
	edgelstl[lastlevel]=(edgelstr[lastlevel]=edgelstl[i-1]+((doPie &&(sideedge==1))?2:1));
	int nverts=edgelstl[lastlevel]+1;

	nfaces+=2*segs*(2*capsegs-1);
	if (height<0) delta = -delta;

  mesh.setSmoothFlags(smooth != 0);
  if (radius1==0.0f)
  { mesh.setNumVerts(0);
	  mesh.setNumFaces(0);
	  mesh.setNumTVerts(0);
	  mesh.setNumTVFaces(0);
  }
  else
  {
	mesh.setNumVerts(nverts);
	mesh.setNumFaces(nfaces);
	if (genUVs) 
	{ ntverts+=nverts;
	  mesh.setNumTVerts(ntverts);
	  mesh.setNumTVFaces(nfaces);
	} 
	else 
	{ mesh.setNumTVerts(0);
	  mesh.setNumTVFaces(0);
	}
	
	mesh.setSmoothFlags((smooth != 0) | ((doPie != 0) << 1));
	// bottom vertex 
	mesh.setVert(nv, Point3(0.0f,0.0f,height));
	mesh.setVert(nverts-1, Point3(0.0f,0.0f,0.0f));		
	float ru,cang,sang,botz;//deltacsegs=PI/(2.0f*csegs);
	int msegs=segs,deltaend=nverts-capv-1;
	// Bottom cap vertices
	ang = startAng;	 
	if (!doPie) msegs--;
	float r1sqr=radius1*radius1,chsq=capheight*capheight;
	float tnkrad=(r1sqr+chsq)/(2.0f*capheight);
	float theta=(float)atan(2.0f*radius1*capheight/(r1sqr-chsq)),alpha=(float)acos(1.0f-blend/(2.0f*r1sqr));
	float tangle=theta/capsegs,chrad=tnkrad*(float)sin(theta-alpha),tmpangle;
	float chz=tnkrad*(1.0f-(float)cos(theta-alpha));
	BOOL incap=TRUE;
	for (jx = 0; jx<=msegs; jx++) 
	{ cang=(float)cos(ang);
	  sang=(float)sin(ang);
	  for(ix=1; ix<=sideedge; ix++)
	  {if (ix<=capsegs)
		{ incap=(capsegs==sideedge||ix<capsegs);
		  ru=(incap?tnkrad*(float)sin(tmpangle=(tangle*(float)ix)):chrad);
		  if (jx==0)
		  { p.z = height-(incap?tnkrad*(1.0f-(float)cos(tmpangle)):chz);
		  } else p.z=mesh.verts[edgelstr[ix]].z;
	    }
		else
		{ ru=radius1;
		  if (jx==0)
		  {	p.z=height-(capheight+blend);
		  } else p.z=mesh.verts[edgelstr[ix]].z;
		}
		botz=height-p.z;
		if ((doPie)&&((jx==0)&&(ix==sideedge)))
		{ mesh.setVert(edgelstl[ix]+1,Point3(0.0f,0.0f,p.z));
		  mesh.setVert(edgelstl[lastlevel-ix]+1,Point3(0.0f,0.0f,botz));
		}
		p.x = cang*ru;
		p.y = sang*ru;	
		mesh.setVert(edgelstr[ix]+jx, p);
		mesh.setVert(edgelstr[lastlevel-ix]+jx,Point3(p.x,p.y,botz));
	  }
	  ang += delta;
	}
	//top layer done, now reflect sides down 
	int sidevs,startv=edgelstr[sideedge],deltav;				
	if (llsegs>1)
	{ float topd=mesh.verts[startv].z,sincr=(height-2.0f*(height-topd))/llsegs;
	  for (sidevs=0;sidevs<VertexPerLevel;sidevs++)
	  { p=mesh.verts[startv];
	    deltav=VertexPerLevel;
	    for (ic=1;ic<llsegs;ic++)
	    { p.z =topd-sincr*ic;
	 	  mesh.setVert(startv+deltav, p);
		  deltav+=VertexPerLevel;
	    }
	    startv++;
	  }
	}
	int lasttvl=0,lasttvr=0;
	if (genUVs)
	{ int tvcount=0,nexttv;
	  float udenom=2.0f*radius1;
	  for (i=0;i<=sideedge;i++)
	  {	nexttv=edgelstr[i];
		while (nexttv<=edgelstl[i])
		{ mesh.setTVert(tvcount++,(radius1+mesh.verts[nexttv].x)/udenom,(radius1+mesh.verts[nexttv].y)/udenom,0.0f);
		  nexttv++;
	    }
	  }
	  int iseg,hcount=0,lastedge=(sideedge==1?lastlevel-2:lastlevel-1);
	  float hlevel;
	  for (i=sideedge;i<=lastlevel-sideedge;i++)
	  { hlevel=1.0f-hcount++/(float)llsegs;
		for (iseg=0;iseg<=segs;iseg++)
		 mesh.setTVert(tvcount++,(float)iseg/segs,hlevel,0.0f);
	  }
	  i--;
	  while (i<=lastlevel)
	  {	nexttv=edgelstr[i];
		while (nexttv<=edgelstl[i])
		{ mesh.setTVert(tvcount++,(radius1+mesh.verts[nexttv].x)/udenom,(radius1+mesh.verts[nexttv].y)/udenom,0.0f);
		  nexttv++;
	    }
		i++;
	  }
	  if (doPie)
	  { lasttvl=lasttvr=tvcount;
		float u,v;
		mesh.setTVert(tvcount++,0.0f,1.0f,0.0f);
		for (i=sideedge;i<=sideedge+llsegs;i++)
	    { mesh.setTVert(tvcount++,0.0f,mesh.verts[edgelstl[i]].z/height,0.0f);
		}
		mesh.setTVert(tvcount++,0.0f,0.0f,0.0f);
		for (i=1;i<lastlevel;i++)
		{ u=(float)sqrt(mesh.verts[edgelstl[i]].x*mesh.verts[edgelstl[i]].x+mesh.verts[edgelstl[i]].y*mesh.verts[edgelstl[i]].y)/radius1;
		  v=mesh.verts[edgelstl[i]].z/height;
		  mesh.setTVert(tvcount++,u,v,0.0f);
		  mesh.setTVert(tvcount++,u,v,0.0f);
		}
	  }
	}	
	int lvert=(doPie?segs+1:segs);
    int t0,t1,b0,b1,tvt0=0,tvt1=0,tvb0=1,tvb1=2,fc=0,smoothgr=(smooth?4:0),vseg=segs+1;
	int tvcount=0,lowerside=lastlevel-sideedge,onside=0;
	BOOL ok,wrap;
	// Now make faces ---
	for (int clevel=0;clevel<lastlevel-1;clevel++)
	{ t1=(t0=edgelstr[clevel])+1;
	  b1=(b0=edgelstr[clevel+1])+1;
	  ok=!doPie; wrap=FALSE;
	  if ((clevel>0)&&((doPie)||(onside==1))) {tvt0++;tvt1++;tvb0++,tvb1++;}
	  if (clevel==1) {tvt0=1;tvt1=2;}
	  if (clevel==sideedge)
	    {tvt1+=lvert;tvt0+=lvert;tvb0+=vseg;tvb1+=vseg;onside++;}
	  else if (clevel==lowerside)
	    {tvt1+=vseg;tvt0+=vseg;tvb0+=lvert;tvb1+=lvert;onside++;}
	  while ((b0<edgelstl[clevel+1])||ok)
	  { if (b1==edgelstr[clevel+2]) 
	    { b1=edgelstr[clevel+1]; 
	      t1=edgelstr[clevel];
		  ok=FALSE;wrap=(onside!=1);}
	  if (smooth)
	  { if (blend>0.0f) smoothgr=4;
	    else if ((clevel<sideedge)||(clevel>=lowerside)) 
		  smoothgr=4;
		else smoothgr=8;
	  }
	  if (genUVs) mesh.tvFace[fc].setTVerts(tvt0,tvb0,(wrap?tvb1-segs:tvb1));
		AddFace(&mesh.faces[fc++],t0,b0,b1,0,smoothgr);
	    if (clevel>0)
		{ if (genUVs)
		  { if (wrap) mesh.tvFace[fc].setTVerts(tvt0++,tvb1-segs,tvt1-segs);
			else mesh.tvFace[fc].setTVerts(tvt0++,tvb1,tvt1);
			tvt1++;
		  }
		  AddFace(&mesh.faces[fc++],t0,b1,t1,1,smoothgr);
		  t0++;t1++;
		}
		b0++;b1++;tvb0++,tvb1++;
	  }
	}
	smoothgr=(smooth?4:0);
	t1=(t0=edgelstr[lastlevel-1])+1;b0=edgelstr[lastlevel];
	int lastpt=(doPie?lastlevel-1:lastlevel);
	if ((doPie)||(onside==1)) {tvt0++;tvt1++;tvb0++,tvb1++;}
	if (sideedge==1) {tvt1+=vseg;tvt0+=vseg;tvb0+=lvert;tvb1+=lvert;onside++;}
	while (t0<edgelstl[lastpt])
	  { if ((!doPie)&&(t1==edgelstr[lastlevel]))
	    { t1=edgelstr[lastlevel-1];tvt1-=segs;}
		if (genUVs) mesh.tvFace[fc].setTVerts(tvt0++,tvb0,tvt1++);
		AddFace(&mesh.faces[fc++],t0,b0,t1,1,smoothgr);
		t0++;t1++;
	  }
	int chv=edgelstl[sideedge]+1,botcap=lastlevel-sideedge;
	int chb=edgelstl[botcap]+1,chm0,chm1,last=0,sg0=(smooth?2:0),sg1=(smooth?1:0);
	if (doPie)
	{int topctv=lasttvl+1,tvcount=topctv+llsegs+2;
	  for (i=1;i<=lastlevel;i++)
	  { if (i<=sideedge)
		{ if (genUVs)
		  { mesh.tvFace[fc].setTVerts(tvcount,topctv,lasttvl);lasttvl=tvcount++;
		    mesh.tvFace[fc+1].setTVerts(lasttvr,topctv,tvcount);lasttvr=tvcount++;
		  }
		  AddFace(&mesh.faces[fc++],edgelstl[i],chv,edgelstl[last],(i==1?1:2),sg0);
		  AddFace(&mesh.faces[fc++],edgelstr[last],chv,edgelstr[i],(i==1?3:2),sg1);
		}
	    else if (i<=botcap)
		{ if (genUVs)
		  { topctv++;
			mesh.tvFace[fc].setTVerts(lasttvl,tvcount,topctv);
			mesh.tvFace[fc+1].setTVerts(lasttvl,topctv,topctv-1);lasttvl=tvcount++;
		    mesh.tvFace[fc+2].setTVerts(topctv-1,topctv,tvcount);
		    mesh.tvFace[fc+3].setTVerts(topctv-1,tvcount,lasttvr);lasttvr=tvcount++;
		  }
		  AddFace(&mesh.faces[fc++],edgelstl[last],edgelstl[i],chm1=(edgelstl[i]+1),0,sg0);
	      AddFace(&mesh.faces[fc++],edgelstl[last],chm1,chm0=(edgelstl[last]+1),1,sg0);
		  AddFace(&mesh.faces[fc++],chm0,chm1,edgelstr[i],0,sg1);
	      AddFace(&mesh.faces[fc++],chm0,edgelstr[i],edgelstr[last],1,sg1);
		}
		else
		{if (genUVs)
		  {	if (i==lastlevel) tvcount=topctv+1;
			mesh.tvFace[fc].setTVerts(tvcount,topctv,lasttvl);
			  if (i<lastlevel) lasttvl=tvcount++;
		    mesh.tvFace[fc+1].setTVerts(lasttvr,topctv,tvcount);lasttvr=tvcount++;
		  }
		  AddFace(&mesh.faces[fc++],edgelstl[i],chb,edgelstl[last],(i==lastlevel?3:2),sg0);
	      AddFace(&mesh.faces[fc++],edgelstr[last],chb,edgelstr[i],(i==lastlevel?1:2),sg1);
		}
		last++;
	  }
	}

	if (minush)
	for (i=0;i<nverts;i++) mesh.verts[i].z-=height;
	if (edgelstr) delete []edgelstr;
	if (edgelstl) delete []edgelstl;
	assert(fc==mesh.numFaces);
//	assert(nv==mesh.numVerts);
  }
  mesh.InvalidateTopologyCache();
}

BOOL OilTnkObject::HasUVW() { 
	BOOL genUVs;
	Interval v;
	pblock->GetValue(PB_GENUVS, 0, genUVs, v);
	return genUVs; 
	}

void OilTnkObject::SetGenUVW(BOOL sw) {  
	if (sw==HasUVW()) return;
	pblock->SetValue(PB_GENUVS,0, sw);				
	}

void OilTnkObject::BuildMesh(TimeValue t)
	{	
	int segs, smooth, hsegs;
	float radius,height,pie1, pie2,capheight,blend;
	int doPie, genUVs,con;	

 
	FixCapLimits(pblock,t,(pmapParam?pmapParam->GetHWnd():NULL),increate);
	FixBlendLimits(pblock,t,(pmapParam?pmapParam->GetHWnd():NULL),increate);

	ivalid = FOREVER;
	
	pblock->GetValue(PB_SIDES,t,segs,ivalid);
	pblock->GetValue(PB_CAPHEIGHT,t,capheight,ivalid);
	pblock->GetValue(PB_HSEGS,t,hsegs,ivalid);
	pblock->GetValue(PB_BLEND,t,blend,ivalid);
	pblock->GetValue(PB_RADIUS,t,radius,ivalid);
	pblock->GetValue(PB_CENTERS,t,con,ivalid);
	pblock->GetValue(PB_HEIGHT,t,height,ivalid);
	if (con) height+=2.0f*capheight;
	pblock->GetValue(PB_SMOOTHON,t,smooth,ivalid);	
	pblock->GetValue(PB_SLICEFROM,t,pie1,ivalid);
	pblock->GetValue(PB_SLICETO,t,pie2,ivalid);	
	pblock->GetValue(PB_SLICEON,t,doPie,ivalid);	
	pblock->GetValue(PB_GENUVS,t,genUVs,ivalid);	
	LimitValue(radius, MIN_RADIUS, MAX_RADIUS);
	LimitValue(height, MIN_HEIGHT, MAX_HEIGHT);
	LimitValue(hsegs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(segs, MIN_SIDES, MAX_SIDES);
	LimitValue(smooth, 0, 1);	
				//        sides, smooth, 			  cylrad  fsize	 totalh
	BuildOilTnkMesh(mesh, segs, smooth, hsegs, doPie,radius, blend, height, capheight,pie1, pie2, genUVs);
	}

inline Point3 operator+(const PatchVert &pv,const Point3 &p)
	{
	return p+pv.p;
	}

#define CIRCLE_VECTOR_LENGTH 0.5517861843f


Object* OilTnkObject::ConvertToType(TimeValue t, Class_ID obtype)
	{
		return SimpleObject::ConvertToType(t,obtype);
	}

int OilTnkObject::CanConvertToType(Class_ID obtype)
	{
	if (obtype==triObjectClassID) {
		return 1;
	} else {
		return SimpleObject::CanConvertToType(obtype);
		}
	}

class OilTnkObjCreateCallBack: public CreateMouseCallBack {
	OilTnkObject *ob;	
	Point3 p[2];
	IPoint2 sp0,sp1,sp2;
	float h,r,basech;
	int con;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		void SetObj(OilTnkObject *obj) { ob = obj; }
	};

int OilTnkObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {

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
				p[0] = vpt->SnapPoint(m,m,NULL,snapdim);
				mat.SetTrans(p[0]); // Set Node's transform				
				ob->pblock->GetValue(PB_CENTERS,0,con,FOREVER);
				ob->pblock->SetValue(PB_RADIUS,0,0.01f);
				ob->pblock->SetValue(PB_HEIGHT,0,0.005f);
				ob->pblock->SetValue(PB_CAPHEIGHT,0,0.0025f);
				ob->increate=TRUE;
				break;
			case 1: 
				mat.IdentityMatrix();
				//mat.PreRotateZ(HALFPI);
				sp1 = m;							   
				p[1] = vpt->SnapPoint(m,m,NULL,snapdim);
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
				ob->pblock->SetValue(PB_HEIGHT,0,0.5f*r);
				ob->pblock->SetValue(PB_CAPHEIGHT,0,(basech=0.25f*r));
				ob->pmapParam->Invalidate();

				if (flags&MOUSE_CTRL) {
					float ang = (float)atan2(p[1].y-p[0].y,p[1].x-p[0].x);
					mat.PreRotateZ(ob->ip->SnapAngle(ang));
					}

				if (msg==MOUSE_POINT) {
					if (Length(m-sp0)<3 ||
						Length(p[1]-p[0])<0.1f) {	
						ob->increate=FALSE;
						return CREATE_ABORT;
						}
					}
				break;
			case 2:
				{sp2=m;
				float minh=0.5f*r;
#ifdef _OSNAP
				float tmph = vpt->SnapLength(vpt->GetCPDisp(p[1],Point3(0,0,1),sp1,m,TRUE));
#else
				float tmph = vpt->SnapLength(vpt->GetCPDisp(p[1],Point3(0,0,1),sp1,m));
#endif
				tmph+=(tmph<0?-minh:minh);
				p[1].z=p[0].z;
				h=tmph;
				ob->pblock->SetValue(PB_HEIGHT,0,h);
				ob->pmapParam->Invalidate();
				if (msg==MOUSE_POINT) 
				{ if (Length(m-sp0)<3) {ob->increate=FALSE;
				return CREATE_ABORT;}
				}
				}
				break;
			case 3:
				{
				float ch = vpt->SnapLength(vpt->GetCPDisp(p[1],Point3(0,1,0),sp2,m));
				if (ch<0.0f) ch=0.0f;
				ch+=basech;float rmax=0.99f*r;
				if (ch>=rmax) ch=rmax;
				float blend;
				ob->pblock->GetValue(PB_BLEND,0,blend,FOREVER);
				float chmax=((float)fabs(h)-2.0f*blend)/2.0f;
				float minc=0.025f*r;
				if ((!con)&&(ch>chmax)) ch=(chmax<0?minc:chmax);
				if (ch<minc) ch=minc;
				ob->pblock->SetValue(PB_CAPHEIGHT,0,ch);
				ob->pmapParam->Invalidate();
				if (msg==MOUSE_POINT) {					
					ob->suspendSnap = FALSE;ob->increate=FALSE;
					return CREATE_STOP;
					}
				}
				break;
			}
		}
	else
	if (msg == MOUSE_ABORT) {
		ob->increate=FALSE;		
		return CREATE_ABORT;
		}

	return TRUE;
	}

static OilTnkObjCreateCallBack cylCreateCB;

CreateMouseCallBack* OilTnkObject::GetCreateMouseCallBack() 
	{
	cylCreateCB.SetObj(this);
	return(&cylCreateCB);
	}

BOOL OilTnkObject::OKtoDisplay(TimeValue t) 
	{
	float radius;
	pblock->GetValue(PB_RADIUS,t,radius,FOREVER);
	if (radius==0.0f) return FALSE;
	else return TRUE;
	}


// From ParamArray
BOOL OilTnkObject::SetValue(int i, TimeValue t, int v) 
	{
	switch (i) {
		case PB_CREATEMETHOD: dlgCreateMeth = v; break;
		case PB_TI_CENTERS: crtCenters = v; break;
		}		
	return TRUE;
	}

BOOL OilTnkObject::SetValue(int i, TimeValue t, float v)
	{
	switch (i) {				
		case PB_TI_RADIUS: crtRadius = v; break;
		case PB_TI_CAPHEIGHT: crtCapHeight = v; break;
		case PB_TI_HEIGHT: crtHeight = v; break;
		case PB_TI_BLEND: crtBlend = v; break;
		}	
	return TRUE;
	}

BOOL OilTnkObject::SetValue(int i, TimeValue t, Point3 &v) 
	{
	switch (i) {
		case PB_TI_POS: crtPos = v; break;
		}		
	return TRUE;
	}

BOOL OilTnkObject::GetValue(int i, TimeValue t, int &v, Interval &ivalid) 
	{
	switch (i) {
		case PB_CREATEMETHOD: v = dlgCreateMeth; break;
		case PB_TI_CENTERS: v = crtCenters; break;
		}
	return TRUE;
	}

BOOL OilTnkObject::GetValue(int i, TimeValue t, float &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_RADIUS: v=crtRadius; break;
		case PB_TI_CAPHEIGHT: v=crtCapHeight; break;
		case PB_TI_HEIGHT: v=crtHeight; break;
		case PB_TI_BLEND: v=crtBlend; break;
		}
	return TRUE;
	}

BOOL OilTnkObject::GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_POS: v = crtPos; break;		
		}
	return TRUE;
	}


void OilTnkObject::InvalidateUI() 
	{
	if (pmapParam) pmapParam->Invalidate();
	}

ParamDimension *OilTnkObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_RADIUS: return stdWorldDim;
		case PB_CAPHEIGHT: return stdWorldDim;
		case PB_HEIGHT: return stdWorldDim;
		case PB_BLEND: return stdWorldDim;
		case PB_SIDES: return stdSegmentsDim;
		case PB_HSEGS: return stdSegmentsDim;
		case PB_SLICEFROM: return stdAngleDim;
		case PB_SLICETO: return stdAngleDim;
		default: return defaultDim;
		}
	}

TSTR OilTnkObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_RADIUS: return TSTR(GetString(IDS_RB_RADIUS));
		case PB_CAPHEIGHT: return TSTR(GetString(IDS_RB_CAPHEIGHT));
		case PB_HEIGHT: return TSTR(GetString(IDS_RB_HEIGHT));
		case PB_BLEND: return TSTR(GetString(IDS_RB_BLEND));
		case PB_SIDES: return TSTR(GetString(IDS_RB_SIDES));
		case PB_HSEGS: return TSTR(GetString(IDS_RB_HSEGS));
		case PB_SMOOTHON: return TSTR(GetString(IDS_RB_SMOOTHON));
		case PB_SLICEON: return TSTR(GetString(IDS_RB_SLICEON));
		case PB_SLICEFROM: return TSTR(GetString(IDS_RB_SLICEFROM));
		case PB_SLICETO: return TSTR(GetString(IDS_RB_SLICETO));
		case PB_GENUVS:  return TSTR(GetString(IDS_MXS_GENUVS));
		default: return TSTR(_T(""));
		}
	}

RefTargetHandle OilTnkObject::Clone(RemapDir& remap) 
	{
	OilTnkObject* newob = new OilTnkObject();	
	newob->ReplaceReference(0,pblock->Clone(remap));	
	newob->ivalid.SetEmpty();	
	BaseClone(this, newob, remap);
	return(newob);
	}




