/**********************************************************************
 *<
	FILE: cext.cpp - parameterized c-extrusion
	Created by Audrey Peterson

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#include "solids.h"
#include "iparamm.h"
#include "Simpobj.h"

static Class_ID CEXT_CLASS_ID(0x33b1284d, 0x7af0200d);
class CExtObject : public SimpleObject, public IParamArray {
	public:
		// Class vars
		static IParamMap *pmapCreate;
		static IParamMap *pmapTypeIn;
		static IParamMap *pmapParam;		
		static IObjParam *ip;
		static float crtTopLength,crtSideLength,crtBotLength,crtHeight;
		static float crtTopWidth,crtSideWidth,crtBotWidth;
		static int dlgHSegs, dlgTSegs, dlgWSegs,dlgBSegs,dlgSSegs;
		static int dlgCreateMeth;
		static Point3 crtPos;		
		BOOL increate;
		int createmeth;

		CExtObject();		

		// From Object
		int CanConvertToType(Class_ID obtype);
		Object* ConvertToType(TimeValue t, Class_ID obtype);
				
		// From BaseObject
		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);		
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		TCHAR *GetObjectName() { return GetString(IDS_RB_CEXT); }
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);
				
		// Animatable methods		
		void DeleteThis() { delete this; }
		Class_ID ClassID() { return CEXT_CLASS_ID; }  		
				
		// From ref
		RefTargetHandle Clone(RemapDir& remap = NoRemap());		
		IOResult Load(ILoad *iload);
		IOResult Save(ISave* isave);

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

#define DEF_SEGMENTS 	1
#define DEF_SIDES		1

#define DEF_RADIUS		float(0.0)
#define DEF_HEIGHT		float(0.01)
#define DEF_FILLET		float(0.01)

#define SMOOTH_ON		1
#define SMOOTH_OFF		0



//--- ClassDescriptor and class vars ---------------------------------

class CExtClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new CExtObject; }
	const TCHAR *	ClassName() { return GetString(IDS_AP_CEXT_CLASS); }
	SClass_ID		SuperClassID() { return GEOMOBJECT_CLASS_ID; }
	Class_ID		ClassID() { return CEXT_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_RB_EXTENDED);  }
	void			ResetClassParams(BOOL fileReset);
	};

static CExtClassDesc CExtDesc;

ClassDesc* GetCExtDesc() { return &CExtDesc; }

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// class variables for CExt class.
IObjParam *CExtObject::ip         = NULL;
int CExtObject::dlgHSegs    = DEF_SEGMENTS;
int CExtObject::dlgSSegs       = DEF_SEGMENTS;
int CExtObject::dlgTSegs       = DEF_SEGMENTS;
int CExtObject::dlgWSegs       = DEF_SEGMENTS;
int CExtObject::dlgBSegs       = DEF_SEGMENTS;
int CExtObject::dlgCreateMeth     = 0; // create_radius
IParamMap *CExtObject::pmapCreate = NULL;
IParamMap *CExtObject::pmapTypeIn = NULL;
IParamMap *CExtObject::pmapParam  = NULL;
Point3 CExtObject::crtPos         = Point3(0,0,0);
float CExtObject::crtHeight       = 0.0f;
float CExtObject::crtTopLength    = 0.0f;
float CExtObject::crtSideLength   = 0.0f;
float CExtObject::crtBotLength    = 0.0f;
float CExtObject::crtBotWidth     = 0.0f;
float CExtObject::crtSideWidth     = 0.0f;
float CExtObject::crtTopWidth     = 0.0f;

void CExtClassDesc::ResetClassParams(BOOL fileReset)
	{ CExtObject::dlgHSegs			= DEF_SEGMENTS;
	  CExtObject::dlgSSegs       = DEF_SEGMENTS;
	  CExtObject::dlgTSegs       = DEF_SEGMENTS;
	  CExtObject::dlgWSegs       = DEF_SEGMENTS;
	  CExtObject::dlgBSegs       = DEF_SEGMENTS;
	  CExtObject::dlgCreateMeth     = 0; // create_radius
	  CExtObject::crtHeight       = 0.0f;
	  CExtObject::crtTopLength    = 0.0f;
	  CExtObject::crtSideLength   = 0.0f;
	  CExtObject::crtBotLength    = 0.0f;
	  CExtObject::crtBotWidth     = 0.0f;
      CExtObject::crtSideWidth     = 0.0f;
      CExtObject::crtTopWidth     = 0.0f;
	  CExtObject::crtPos         = Point3(0,0,0);
	}

//--- Parameter map/block descriptors -------------------------------

// Parameter block indices
#define PB_TOPLENGTH	0
#define PB_SIDELENGTH	1
#define PB_BOTLENGTH	2
#define PB_TOPWIDTH		3
#define PB_SIDEWIDTH	4
#define PB_BOTWIDTH		5
#define PB_HEIGHT		6
#define PB_TSEGS		7
#define PB_SSEGS		8
#define PB_BSEGS		9
#define PB_WSEGS		10
#define PB_HSEGS		11
#define PB_GENUVS		12

// Non-parameter block indices
#define PB_CREATEMETHOD		0
#define PB_TI_POS			1
#define PB_TI_TOPLENGTH		2
#define PB_TI_SIDELENGTH	3
#define PB_TI_BOTLENGTH		4
#define PB_TI_TOPWIDTH		5
#define PB_TI_SIDEWIDTH		6
#define PB_TI_BOTWIDTH		7
#define PB_TI_HEIGHT		8

#define BMIN_HEIGHT		float(0.1)
#define BMAX_HEIGHT		float(1.0E30)
#define BMIN_LENGTH		float(0.1)
#define BMAX_LENGTH		float(1.0E30)
#define BMIN_WIDTH		float(0)
#define BMAX_WIDTH		float(1.0E30)
//
//
//	Creation method

static int createMethIDs[] = {IDC_UEXTR_CORNER,IDC_UEXTR_CENTER};

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
		IDC_CEXT_POSX,IDC_CEXT_POSXSPIN,
		IDC_CEXT_POSY,IDC_CEXT_POSYSPIN,
		IDC_CEXT_POSZ,IDC_CEXT_POSZSPIN,
		float(-1.0E30),float(1.0E30),
		SPIN_AUTOSCALE),
	
	// Top Length
	ParamUIDesc(
		PB_TI_TOPLENGTH,
		EDITTYPE_UNIVERSE,
		IDC_CEXT_TOPLEN,IDC_CEXT_TOPLENSPIN,
		MIN_HEIGHT,BMAX_LENGTH,
		SPIN_AUTOSCALE),
	
	// Side Length
	ParamUIDesc(
		PB_TI_SIDELENGTH,
		EDITTYPE_UNIVERSE,
		IDC_CEXT_SIDELEN,IDC_CEXT_SIDELENSPIN,
		MIN_HEIGHT,BMAX_WIDTH,
		SPIN_AUTOSCALE),	

	// Bot Length
	ParamUIDesc(
		PB_TI_BOTLENGTH,
		EDITTYPE_UNIVERSE,
		IDC_CEXT_BOTLEN,IDC_CEXT_BOTLENSPIN,
		MIN_HEIGHT,BMAX_HEIGHT,
		SPIN_AUTOSCALE),
		
	// Top Width
	ParamUIDesc(
		PB_TI_TOPWIDTH,
		EDITTYPE_UNIVERSE,
		IDC_CEXT_TOPWID,IDC_CEXT_TOPWIDSPIN,
		BMIN_LENGTH,BMAX_LENGTH,
		SPIN_AUTOSCALE),
	
	// Side Width
	ParamUIDesc(
		PB_TI_SIDEWIDTH,
		EDITTYPE_UNIVERSE,
		IDC_CEXT_SIDEWID,IDC_CEXT_SIDEWIDSPIN,
		BMIN_WIDTH,BMAX_WIDTH,
		SPIN_AUTOSCALE),	

	// Bot Width
	ParamUIDesc(
		PB_TI_BOTWIDTH,
		EDITTYPE_UNIVERSE,
		IDC_CEXT_BOTWID,IDC_CEXT_BOTWIDSPIN,
		BMIN_HEIGHT,BMAX_HEIGHT,
		SPIN_AUTOSCALE),	

	// Height
	ParamUIDesc(
		PB_TI_HEIGHT,
		EDITTYPE_UNIVERSE,
		IDC_CEXT_HEIGHT,IDC_CEXT_HEIGHTSPIN,
		MIN_HEIGHT,BMAX_HEIGHT,
		SPIN_AUTOSCALE),	
	};
#define TYPEINDESC_LENGTH 8


//
//
// Parameters

static ParamUIDesc descParam[] = {
	// Top Length
	ParamUIDesc(
		PB_TOPLENGTH,
		EDITTYPE_UNIVERSE,
		IDC_CEXT_TOPLEN,IDC_CEXT_TOPLENSPIN,
		MIN_HEIGHT,BMAX_LENGTH,
		SPIN_AUTOSCALE),
	
	// Side Length
	ParamUIDesc(
		PB_SIDELENGTH,
		EDITTYPE_UNIVERSE,
		IDC_CEXT_SIDELEN,IDC_CEXT_SIDELENSPIN,
		MIN_HEIGHT,BMAX_WIDTH,
		SPIN_AUTOSCALE),	

	// Bot Length
	ParamUIDesc(
		PB_BOTLENGTH,
		EDITTYPE_UNIVERSE,
		IDC_CEXT_BOTLEN,IDC_CEXT_BOTLENSPIN,
		MIN_HEIGHT,BMAX_HEIGHT,
		SPIN_AUTOSCALE),
		
	// Top Width
	ParamUIDesc(
		PB_TOPWIDTH,
		EDITTYPE_UNIVERSE,
		IDC_CEXT_TOPWID,IDC_CEXT_TOPWIDSPIN,
		BMIN_LENGTH,BMAX_LENGTH,
		SPIN_AUTOSCALE),
	
	// Side Width
	ParamUIDesc(
		PB_SIDEWIDTH,
		EDITTYPE_UNIVERSE,
		IDC_CEXT_SIDEWID,IDC_CEXT_SIDEWIDSPIN,
		BMIN_WIDTH,BMAX_WIDTH,
		SPIN_AUTOSCALE),	

	// Bot Width
	ParamUIDesc(
		PB_BOTWIDTH,
		EDITTYPE_UNIVERSE,
		IDC_CEXT_BOTWID,IDC_CEXT_BOTWIDSPIN,
		BMIN_HEIGHT,BMAX_HEIGHT,
		SPIN_AUTOSCALE),	

	// Height
	ParamUIDesc(
		PB_HEIGHT,
		EDITTYPE_UNIVERSE,
		IDC_CEXT_HEIGHT,IDC_CEXT_HEIGHTSPIN,
		MIN_HEIGHT,BMAX_HEIGHT,
		SPIN_AUTOSCALE),	

	// Top Segments
	ParamUIDesc(
		PB_TSEGS,
		EDITTYPE_INT,
		IDC_CEXT_TSEGS,IDC_CEXT_TSEGSPIN,
		(float)MIN_SEGMENTS,(float)MAX_SEGMENTS,
		0.1f),
	
	// Side Segments
	ParamUIDesc(
		PB_SSEGS,
		EDITTYPE_INT,
		IDC_CEXT_SSEGS,IDC_CEXT_SSEGSPIN,
		(float)MIN_SEGMENTS,(float)MAX_SEGMENTS,
		0.1f),

	// Bot Segments
	ParamUIDesc(
		PB_BSEGS,
		EDITTYPE_INT,
		IDC_CEXT_BSEGS,IDC_CEXT_BSEGSPIN,
		(float)MIN_SEGMENTS,(float)MAX_SEGMENTS,
		0.1f),

	// Width Segments
	ParamUIDesc(
		PB_WSEGS,
		EDITTYPE_INT,
		IDC_CEXT_WSEGS,IDC_CEXT_WSEGSSPIN,
		(float)MIN_SEGMENTS,(float)MAX_SEGMENTS,
		0.1f),
			
	// Height Segments
	ParamUIDesc(
		PB_HSEGS,
		EDITTYPE_INT,
		IDC_CEXT_HSEGS,IDC_CEXT_HSEGSSPIN,
		(float)MIN_SEGMENTS,(float)MAX_SEGMENTS,
		0.1f),

	// Gen UVs
	ParamUIDesc(PB_GENUVS,TYPE_SINGLECHEKBOX,IDC_GENTEXTURE),			
	};
#define PARAMDESC_LENGTH 13


// variable type, NULL, animatable, number
ParamBlockDescID CExtdescVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 }, 
	{ TYPE_FLOAT, NULL, TRUE, 4 },
	{ TYPE_FLOAT, NULL, TRUE, 5 },
	{ TYPE_FLOAT, NULL, TRUE, 6 }, 
	{ TYPE_INT, NULL, TRUE, 7 }, 
	{ TYPE_INT, NULL, TRUE, 8 }, 
	{ TYPE_INT, NULL, TRUE, 9 }, 
	{ TYPE_INT, NULL, TRUE, 10 }, 
	{ TYPE_INT, NULL, TRUE, 11 }, 
	{ TYPE_INT, NULL, FALSE, 12 }, 
	};

#define PBLOCK_LENGTH	13

#define NUM_OLDVERSIONS	0

#define CURRENT_VERSION	0
static ParamVersionDesc curVersion(CExtdescVer0,PBLOCK_LENGTH,CURRENT_VERSION);

void FixTopWidth(IParamBlock *pblock,TimeValue t,HWND hWnd,BOOL increate)
{ float topwidth,botwidth,sidelen;

	pblock->GetValue(PB_SIDELENGTH,t,sidelen,FOREVER);
	pblock->GetValue(PB_TOPWIDTH,(increate?0:t),topwidth,FOREVER);
	pblock->GetValue(PB_BOTWIDTH,t,botwidth,FOREVER);
	float fmax=(float)fabs(fabs(sidelen)-botwidth);	
	if (hWnd)
	{ ISpinnerControl *spin2 = GetISpinner(GetDlgItem(hWnd,IDC_CEXT_TOPWIDSPIN));
	  spin2->SetLimits(BMIN_LENGTH,fmax,FALSE);
	  ReleaseISpinner(spin2);
	}
	if (topwidth>fmax) pblock->SetValue(PB_TOPWIDTH,(increate?0:t),fmax);
}
void FixBotWidth(IParamBlock *pblock,TimeValue t,HWND hWnd,BOOL increate)
{ float sidelen,topwidth,botwidth;

	pblock->GetValue(PB_SIDELENGTH,t,sidelen,FOREVER);
	pblock->GetValue(PB_TOPWIDTH,t,topwidth,FOREVER);
	pblock->GetValue(PB_BOTWIDTH,(increate?0:t),botwidth,FOREVER);
	float fmax=(float)fabs(sidelen)-topwidth;		
	if (hWnd)
    { ISpinnerControl *spin2 = GetISpinner(GetDlgItem(hWnd,IDC_CEXT_BOTWIDSPIN));
	  spin2->SetLimits(BMIN_LENGTH,fmax,FALSE);
	  ReleaseISpinner(spin2);
	}
	if (botwidth>fmax) pblock->SetValue(PB_BOTWIDTH,(increate?0:t),fmax);
}
void FixSideWidth(IParamBlock *pblock,TimeValue t,HWND hWnd,BOOL increate)
{ float toplen,botlen,sidewidth;

	pblock->GetValue(PB_SIDEWIDTH,(increate?0:t),sidewidth,FOREVER);
	pblock->GetValue(PB_TOPLENGTH,t,toplen,FOREVER);
	pblock->GetValue(PB_BOTLENGTH,t,botlen,FOREVER);
	float fmax=(toplen<0.0f?-toplen:toplen)+(botlen<0.0f?-botlen:botlen);										  
	if (hWnd)
	{ ISpinnerControl *spin2 = GetISpinner(GetDlgItem(hWnd,IDC_CEXT_SIDEWIDSPIN));
	  spin2->SetLimits(BMIN_LENGTH,fmax,FALSE);
	  ReleaseISpinner(spin2);
	}
	if (sidewidth>fmax) pblock->SetValue(PB_SIDEWIDTH,(increate?0:t),fmax);
}
//--- TypeInDlgProc --------------------------------
class CExtWidthDlgProc : public ParamMapUserDlgProc {
	public:
		CExtObject *ob;

		CExtWidthDlgProc(CExtObject *o) {ob=o;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

BOOL CExtWidthDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{ 	switch (msg) {
		case CC_SPINNER_CHANGE:
			switch ( LOWORD(wParam) ) {
				case IDC_CEXT_TOPWIDSPIN:
					FixTopWidth(ob->pblock,t,hWnd,ob->increate);
			return TRUE;
				case IDC_CEXT_BOTWIDSPIN:
					FixBotWidth(ob->pblock,t,hWnd,ob->increate);
			return TRUE;
				case IDC_CEXT_SIDEWIDSPIN:
				{  FixSideWidth(ob->pblock,t,hWnd,ob->increate);
				}
			return TRUE;
				}
		}
	return FALSE;
	}

class CExtTypeInDlgProc : public ParamMapUserDlgProc {
	public:
		CExtObject *ob;

		CExtTypeInDlgProc(CExtObject *o) {ob=o;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

BOOL CExtTypeInDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_CEXT_CREATE: {
					if (ob->crtHeight==0.0f) return TRUE;
					
					// We only want to set the value if the object is 
					// not in the scene.
					if (ob->TestAFlag(A_OBJ_CREATING)) {
						ob->pblock->SetValue(PB_HEIGHT,0,ob->crtHeight);
						ob->pblock->SetValue(PB_TOPLENGTH,0,ob->crtTopLength);
						ob->pblock->SetValue(PB_SIDELENGTH,0,ob->crtSideLength);
						ob->pblock->SetValue(PB_BOTLENGTH,0,ob->crtBotLength);
						ob->pblock->SetValue(PB_TOPWIDTH,0,ob->crtTopWidth);
						ob->pblock->SetValue(PB_SIDEWIDTH,0,ob->crtSideWidth);
						ob->pblock->SetValue(PB_BOTWIDTH,0,ob->crtBotWidth);
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


//--- CExt methods -------------------------------

CExtObject::CExtObject() 
	{
	MakeRefByID(FOREVER, 0, CreateParameterBlock(CExtdescVer0, PBLOCK_LENGTH, CURRENT_VERSION));
	
	pblock->SetValue(PB_TSEGS,0,dlgTSegs);
	pblock->SetValue(PB_SSEGS,0,dlgSSegs);
	pblock->SetValue(PB_BSEGS,0,dlgBSegs);	
	pblock->SetValue(PB_WSEGS,0,dlgWSegs);	
	pblock->SetValue(PB_HSEGS,0,dlgHSegs);	

	pblock->SetValue(PB_TOPLENGTH,0,crtTopLength);
	pblock->SetValue(PB_SIDELENGTH,0,crtSideLength);
	pblock->SetValue(PB_BOTLENGTH,0,crtBotLength);
	pblock->SetValue(PB_TOPWIDTH,0,crtTopWidth);
	pblock->SetValue(PB_SIDEWIDTH,0,crtSideWidth);
	pblock->SetValue(PB_BOTWIDTH,0,crtBotWidth);
	pblock->SetValue(PB_HEIGHT,0,crtHeight);

	pblock->SetValue(PB_GENUVS,0,TRUE);

	increate=FALSE;createmeth=0;
	}


void CExtObject::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
	{
	SimpleObject::BeginEditParams(ip,flags,prev);
	this->ip = ip;
	if (pmapCreate && pmapParam) {
		
		// Left over from last CExt ceated
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
				MAKEINTRESOURCE(IDD_UEXTRUSIONS1),
				GetString(IDS_RB_CREATE_DIALOG),
				0);

			pmapTypeIn = CreateCPParamMap(
				descTypeIn,TYPEINDESC_LENGTH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_C_EXTRUSION2),
				GetString(IDS_RB_KEYBOARDENTRY),
				APPENDROLL_CLOSED);			
			}

		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_C_EXTRUSION3),
			GetString(IDS_AP_PARAMETERS),
			0);
		}

	if(pmapTypeIn) {
		// A callback for the type in.
		pmapTypeIn->SetUserDlgProc(new CExtTypeInDlgProc(this));
		}	
	if(pmapParam) {
		// A callback for the type in.
		pmapParam->SetUserDlgProc(new CExtWidthDlgProc(this));
		}	
	}
		
void CExtObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
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
	pblock->GetValue(PB_TSEGS,ip->GetTime(),dlgTSegs,FOREVER);
	pblock->GetValue(PB_SSEGS,ip->GetTime(),dlgSSegs,FOREVER);
	pblock->GetValue(PB_BSEGS,ip->GetTime(),dlgBSegs,FOREVER);	
	pblock->GetValue(PB_WSEGS,ip->GetTime(),dlgWSegs,FOREVER);	
	pblock->GetValue(PB_HSEGS,ip->GetTime(),dlgHSegs,FOREVER);		
}

/*void CExtObject::SetParams(float rad, float height, int segs, int sides, int capsegs, BOOL smooth, 
	pblock->SetValue(PB_TOPLENGTH,0,toplength);
	pblock->SetValue(PB_SIDELENGTH,0,sidelength);
	pblock->SetValue(PB_BOTLENGTH,0,botlength);
	pblock->SetValue(PB_TOPWIDTH,0,topwidth);
	pblock->SetValue(PB_SIDEWIDTH,0,sidewidth);
	pblock->SetValue(PB_BOTWIDTH,0,botwidth);
	pblock->SetValue(PB_TSEGS,0,tsegs);
	pblock->SetValue(PB_SSEGS,0,ssegs);
	pblock->SetValue(PB_BSEGS,0,bsegs);
	pblock->SetValue(PB_WSEGS,0,wsegs);
	pblock->SetValue(PB_HSEGS,0,hsegs);
	pblock->SetValue(PB_GENUVS,0, genUV);
*/

void BuildCExtMesh(Mesh &mesh,
		int hsegs, int tsegs, int ssegs, int bsegs, int wsegs,
		float height, float toplen, float sidelen, float botlen,
		float topwidth, float sidewidth, float botwidth,
		int genUVs, BOOL create)
{	int nf=0;
	int nfaces,ntverts;
	BOOL minush=height<0.0f;
	if (minush) height=-height;
	BOOL minusx=(toplen<0.0f),minusy=(sidelen<0.0f);
	toplen=(float)fabs(toplen);
	botlen=(float)fabs(botlen);
	sidelen=(float)fabs(sidelen);
	 // sides + top/bot
	int VertexPerLevel=2*(wsegs+tsegs+ssegs+bsegs);
	int topverts=(wsegs+1)*(1+tsegs+ssegs+bsegs);
	int nverts=2*topverts+(hsegs+1)*VertexPerLevel;
	nfaces=hsegs*4*(wsegs+bsegs+tsegs+ssegs)+4*wsegs*(tsegs+ssegs+bsegs);

	mesh.setNumVerts(nverts);
	mesh.setNumFaces(nfaces);
	if (genUVs) 
	{ ntverts=nverts+hsegs+1;
	  mesh.setNumTVerts(ntverts);
	  mesh.setNumTVFaces(nfaces);
	} 
	else 
	{ mesh.setNumTVerts(0);
	  mesh.setNumTVFaces(0);
	}
	Point3 p;
	p.x=p.y=p.z=0.0f;
	float minx=(botlen>toplen?0.0f:botlen-toplen),maxx=botlen;
	float xlen=(botlen>toplen?botlen:toplen);
	float xincr,xpos=0.0f,yincr;
	float uvdist=2.0f*(botlen+toplen+sidelen)-2.0f*sidewidth;
	float ystart,xstart;
	float xtv,ytv,ypos,dx=sidewidth/wsegs,tdy=topwidth/wsegs,bdy=botwidth/wsegs;
	int i,j,nv=0,fc=0,dlevel=bsegs+ssegs+tsegs+1,botv=nverts-topverts;
	int tlast,tendcount=ntverts-hsegs-1,tnv=0,bottv=tendcount-topverts;
	tlast=tendcount;
	for (j=0;j<=wsegs;j++)
	{ xstart=0.0f;xincr=(botlen-j*dx)/bsegs;
      yincr=botwidth/wsegs;ystart=j*yincr;
	  for (i=0;i<=bsegs;i++)
	  { mesh.setVert(nv,xpos=xstart+i*xincr,ystart,height);	
	    mesh.setVert(botv,xpos,ystart,0.0f);
	    if (genUVs)
	    { mesh.setTVert(tnv,xtv=(xpos-minx)/xlen,ytv=ystart/sidelen,0.0f);
	      mesh.setTVert(bottv,xtv,1.0f-ytv,0.0f);
	    }
	    nv++;botv++;bottv++;tnv++;
	  }
	  yincr=(sidelen-j*(tdy+bdy))/ssegs;xpos=mesh.verts[nv-1].x;
	  for (i=1;i<=ssegs;i++)
	  { mesh.setVert(nv,xpos,ypos=ystart+i*yincr,height);	
	    mesh.setVert(botv,xpos,ypos,0.0f);
	    if (genUVs)
	    { mesh.setTVert(tnv,xtv,ytv=ypos/sidelen,0.0f);
	      mesh.setTVert(bottv,xtv,1.0f-ytv,0.0f);
	    }
	   nv++;botv++;bottv++;tnv++;
  	  }
 	  xstart=xpos;xincr=(toplen-j*dx)/tsegs;
	  for (i=1;i<=tsegs;i++)
	  { mesh.setVert(nv,xpos=xstart-i*xincr,ypos,height);	
	    mesh.setVert(botv,xpos,ypos,0.0f);
	    if (genUVs)
	    { mesh.setTVert(tnv,xtv=(xpos-minx)/xlen,ytv,0.0f);
	      mesh.setTVert(bottv,xtv,1.0f-ytv,0.0f);
	    }
	    nv++;botv++;bottv++;tnv++;
	  }
	}
	xstart=0.0f;xpos=0.0f;ypos=0.0f;
	float uval=0.0f;
	int refnv=nv;
	xincr=botlen/bsegs;
	for (i=0;i<=bsegs;i++)
	{  mesh.setVert(nv,xpos=xstart+i*xincr,0.0f,height);	
	   if (genUVs) mesh.setTVert(tnv,xtv=(uval=xpos)/uvdist,1.0f,0.0f);
	   nv++;tnv++;
    }
	yincr=sidelen/ssegs;xpos=mesh.verts[nv-1].x;
	for (i=1;i<=ssegs;i++)
	{ mesh.setVert(nv,xpos,ypos+=yincr,height);	
	  if (genUVs) mesh.setTVert(tnv,(uval+=yincr)/uvdist,1.0f,0.0f);
	  nv++;tnv++;
  	}
 	xincr=toplen/tsegs;
	for (i=1;i<=tsegs;i++)
	{ mesh.setVert(nv,xpos-=xincr,ypos,height);	
	  if (genUVs) mesh.setTVert(tnv,(uval+=xincr)/uvdist,1.0f,0.0f);
	  nv++;;tnv++;
	}
	yincr=topwidth/wsegs;
	for (i=1;i<=wsegs;i++)
	{ mesh.setVert(nv,xpos,ypos-=yincr,height);	
	  if (genUVs) mesh.setTVert(tnv,(uval+=yincr)/uvdist,1.0f,0.0f);
	  nv++;;tnv++;
	}
	xincr=(toplen-sidewidth)/tsegs;
	for (i=1;i<=tsegs;i++)
	{ mesh.setVert(nv,xpos+=xincr,ypos,height);	
	  if (genUVs) mesh.setTVert(tnv,(uval+=xincr)/uvdist,1.0f,0.0f);
	  nv++;;tnv++;
	}
	yincr=(sidelen-topwidth-botwidth)/ssegs;
	for (i=1;i<=ssegs;i++)
	{ mesh.setVert(nv,xpos,ypos-=yincr,height);	
	  if (genUVs) mesh.setTVert(tnv,(uval+=yincr)/uvdist,1.0f,0.0f);
	  nv++;;tnv++;
	}
	xincr=(botlen-sidewidth)/bsegs;
	for (i=1;i<=bsegs;i++)
	{ mesh.setVert(nv,xpos-=xincr,ypos,height);	
	  if (genUVs) mesh.setTVert(tnv,(uval+=xincr)/uvdist,1.0f,0.0f);
	  nv++;;tnv++;
	}
	yincr=botwidth/wsegs;
	for (i=1;i<wsegs;i++)
	{ mesh.setVert(nv,xpos,ypos-=yincr,height);	
	  if (genUVs) mesh.setTVert(tnv,(uval+=yincr)/uvdist,1.0f,0.0f);
	  nv++;tnv++;
	}
	if (genUVs) mesh.setTVert(tendcount++,1.0f,1.0f,0.0f);
	float zval,hincr=height/hsegs,zv;
	for (j=0;j<VertexPerLevel;j++)
	{ zval=height;
	  for (i=1;i<=hsegs;i++)
	  {	 zval-=hincr;
	     mesh.setVert(refnv+VertexPerLevel*i,mesh.verts[refnv].x,mesh.verts[refnv].y,zval);
		 if (genUVs) 
		 { mesh.setTVert(refnv+VertexPerLevel*i,mesh.tVerts[refnv].x,zv=zval/height,0.0f);
		   if (j==VertexPerLevel-1) mesh.setTVert(tendcount++,1.0f,zv,0.0f);
		 }
	  }
	refnv++;
	}
	int base=0,top=dlevel,alevel=dlevel-1;
	for (i=0;i<wsegs;i++)
	{ for (j=0;j<alevel;j++)
	 { if (genUVs) 
	   { mesh.tvFace[fc].setTVerts(top,base,base+1);
		 mesh.tvFace[fc+1].setTVerts(top,base+1,top+1);
	   }
	   AddFace(&mesh.faces[fc++],top,base,base+1,0,1);
	   AddFace(&mesh.faces[fc++],top,base+1,top+1,1,1);
	   top++;base++;
  	 } top++;base++;
	}
	base=top+VertexPerLevel;
	tendcount=tlast;
    int b1,smgroup=2,s0=bsegs+1,s1=s0+ssegs,s2=s1+tsegs,s3=s2+wsegs;
	int s4=s3+tsegs,s5=s4+ssegs,s6=s5+bsegs;
	for (i=0;i<hsegs;i++)
	{ for (j=1;j<=VertexPerLevel;j++)
	{ if (genUVs) 
	   { b1=(j<VertexPerLevel?base+1:tendcount+1);
		 mesh.tvFace[fc].setTVerts(top,base,b1);
		 mesh.tvFace[fc+1].setTVerts(top,b1,(j<VertexPerLevel?top+1:tendcount++));
	   }
	   b1=(j<VertexPerLevel?base+1:base-VertexPerLevel+1);
	   smgroup=(j<s0?2:(j<s1?4:(j<s2?2:(j<s3?4:(j<s4?2:(j<s5?4:(j<s6?2:4)))))));
	   AddFace(&mesh.faces[fc++],top,base,b1,0,smgroup);
	   AddFace(&mesh.faces[fc++],top,b1,(j<VertexPerLevel?top+1:top-VertexPerLevel+1),1,smgroup);
	   top++;base++;
  	 } 
	}
	top=base;base=top+alevel;
	base=top+dlevel;
	// Just for Jack
	for (i=0;i<wsegs;i++)
	{ for (j=0;j<alevel;j++)
	 { if (genUVs) 
	   { mesh.tvFace[fc].setTVerts(top,base,base+1);
		 mesh.tvFace[fc+1].setTVerts(top,base+1,top+1);
	   }
	   AddFace(&mesh.faces[fc++],top,base,base+1,0,1);
	   AddFace(&mesh.faces[fc++],top,base+1,top+1,1,1);
	   top++;base++;
  	 } top++;base++;
	}
	if (minusx || minusy || minush)
	{   float centerx=(create?toplen:0),centery=(create?sidelen:0);
		 for (i=0;i<nverts;i++)
	   { if (minusx) mesh.verts[i].x=-mesh.verts[i].x+centerx;
	     if (minusy) mesh.verts[i].y=-mesh.verts[i].y+centery;
	     if (minush) mesh.verts[i].z-=height;
	   }
	   DWORD hold;
	   int tedge;
	   if (minusx!=minusy)
	   for (i=0;i<nfaces;i++)
	   { hold=mesh.faces[i].v[0];mesh.faces[i].v[0]=mesh.faces[i].v[2];mesh.faces[i].v[2]=hold;
	     tedge=mesh.faces[i].getEdgeVis(0);mesh.faces[i].setEdgeVis(0,mesh.faces[i].getEdgeVis(1));
		 mesh.faces[i].setEdgeVis(1,tedge);
	     if (genUVs)
         { hold=mesh.tvFace[i].t[0];mesh.tvFace[i].t[0]=mesh.tvFace[i].t[2];mesh.tvFace[i].t[2]=hold;}
	   }
	}
	assert(fc==mesh.numFaces);
//	assert(nv==mesh.numVerts); */
	mesh.InvalidateTopologyCache();
}

BOOL CExtObject::HasUVW() { 
	BOOL genUVs;
	Interval v;
	pblock->GetValue(PB_GENUVS, 0, genUVs, v);
	return genUVs; 
	}

void CExtObject::SetGenUVW(BOOL sw) {  
	if (sw==HasUVW()) return;
	pblock->SetValue(PB_GENUVS,0, sw);				
	}

void CExtObject::BuildMesh(TimeValue t)
	{	
	int hsegs,tsegs,ssegs,bsegs,wsegs;
	float height,toplen,sidelen,botlen,topwidth,sidewidth,botwidth;
	int genUVs;	

	// Start the validity interval at forever and widdle it down.
	FixTopWidth(pblock,t,(pmapParam?pmapParam->GetHWnd():NULL),increate);
	FixBotWidth(pblock,t,(pmapParam?pmapParam->GetHWnd():NULL),increate);
	FixSideWidth(pblock,t,(pmapParam?pmapParam->GetHWnd():NULL),increate);
	ivalid = FOREVER;
	
	pblock->GetValue(PB_HSEGS,t,hsegs,ivalid);
	pblock->GetValue(PB_TSEGS,t,tsegs,ivalid);
	pblock->GetValue(PB_SSEGS,t,ssegs,ivalid);
	pblock->GetValue(PB_BSEGS,t,bsegs,ivalid);
	pblock->GetValue(PB_WSEGS,t,wsegs,ivalid);
	pblock->GetValue(PB_TOPLENGTH,t,toplen,ivalid);
	pblock->GetValue(PB_SIDELENGTH,t,sidelen,ivalid);
	pblock->GetValue(PB_BOTLENGTH,t,botlen,ivalid);
	pblock->GetValue(PB_TOPWIDTH,t,topwidth,ivalid);
	pblock->GetValue(PB_SIDEWIDTH,t,sidewidth,ivalid);
	pblock->GetValue(PB_BOTWIDTH,t,botwidth,ivalid);
	pblock->GetValue(PB_HEIGHT,t,height,ivalid);
	pblock->GetValue(PB_GENUVS,t,genUVs,ivalid);
	LimitValue(height, MIN_HEIGHT, BMAX_HEIGHT);
	LimitValue(toplen, MIN_HEIGHT, BMAX_LENGTH);
	LimitValue(sidelen, MIN_HEIGHT,BMAX_WIDTH);
	LimitValue(botlen, MIN_HEIGHT, BMAX_HEIGHT);
	LimitValue(topwidth, BMIN_LENGTH,BMAX_LENGTH);
	LimitValue(sidewidth, BMIN_WIDTH,BMAX_WIDTH);
	LimitValue(botwidth, BMIN_HEIGHT,BMAX_HEIGHT);
	LimitValue(hsegs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(tsegs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(ssegs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(bsegs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(wsegs, MIN_SEGMENTS, MAX_SEGMENTS);
	
	BuildCExtMesh(mesh,hsegs,tsegs,ssegs,bsegs,wsegs,height,
		toplen,sidelen,botlen,topwidth,sidewidth,botwidth, genUVs,createmeth);
	}

inline Point3 operator+(const PatchVert &pv,const Point3 &p)
	{
	return p+pv.p;
	}


Object* CExtObject::ConvertToType(TimeValue t, Class_ID obtype)
	{
		return SimpleObject::ConvertToType(t,obtype);
	}

int CExtObject::CanConvertToType(Class_ID obtype)
	{
	if (obtype==triObjectClassID) {
		return 1;
	} else {
		return SimpleObject::CanConvertToType(obtype);
		}
	}

class CExtObjCreateCallBack: public CreateMouseCallBack {
	CExtObject *ob;	
	Point3 p[2],d;
	IPoint2 sp0,sp1,sp2;
	float xwid,l,hd,slen;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		void SetObj(CExtObject *obj) { ob = obj; }
	};

int CExtObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) 
{
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
		case 0:	ob->suspendSnap = TRUE;				
				sp0 = m;				
				p[0] = vpt->SnapPoint(m,m,NULL,snapdim);
				mat.SetTrans(p[0]); // Set Node's transform				
				ob->pblock->SetValue(PB_BOTLENGTH,0,0.01f);
				ob->pblock->SetValue(PB_TOPLENGTH,0,0.01f);
				ob->pblock->SetValue(PB_SIDELENGTH,0,0.01f);
				ob->createmeth=ob->dlgCreateMeth;
				break;
			case 1: 
				mat.IdentityMatrix();
				sp1 = m;
				p[1] = vpt->SnapPoint(m,m,NULL,snapdim);
				d = p[1]-p[0];
				if (flags&MOUSE_CTRL) {
					// Constrain to square base
					float len;
					if (fabs(d.x) > fabs(d.y)) len = d.x;
					else len = d.y;
					d.x = d.y = len;}
				if (!ob->dlgCreateMeth)
				{ mat.SetTrans(p[0]);
				if (flags&MOUSE_CTRL) d.x=(d.y*=2.0f);}
				else 
				{ mat.SetTrans(p[0]-Point3(fabs(d.x),fabs(d.y),fabs(d.z))); 
				  d=2.0f*d;	
				}
				float tmp;
				xwid=(tmp=(float)fabs(d.x));
				slen=(float)fabs(d.y);
				tmp*=0.2f;
				ob->pblock->SetValue(PB_BOTLENGTH,0,d.x);
				ob->pblock->SetValue(PB_TOPLENGTH,0,d.x);
				ob->pblock->SetValue(PB_SIDELENGTH,0,d.y);
				ob->pblock->SetValue(PB_BOTWIDTH,0,tmp);
				ob->pblock->SetValue(PB_TOPWIDTH,0,tmp);
				ob->pblock->SetValue(PB_SIDEWIDTH,0,0.2f*slen);
				ob->pmapParam->Invalidate();
				ob->increate=TRUE;
				ob->createmeth=ob->dlgCreateMeth;
				if (msg==MOUSE_POINT && (Length(sp1-sp0)<3 || Length(d)<0.1f)) 
				{ ob->increate=FALSE;
					return CREATE_ABORT;	}
				break;
			case 2:
				{
				sp2=m;
#ifdef _OSNAP
				float h = vpt->SnapLength(vpt->GetCPDisp(p[1],Point3(0,0,1),sp1,m,TRUE));
#else
				float h = vpt->SnapLength(vpt->GetCPDisp(p[1],Point3(0,0,1),sp1,m));
#endif
				ob->pblock->SetValue(PB_HEIGHT,0,h);
				ob->pmapParam->Invalidate();				
				if (msg==MOUSE_POINT) {	if (Length(m-sp0)<3) {ob->increate=FALSE; return CREATE_ABORT;}}
				break;
				}
			case 3: 
				float f=vpt->SnapLength(vpt->GetCPDisp(p[1],Point3(0,1,0),sp2,m));
				if (f<0.0f) f=0.0f;
				float fmax=slen/2.0f;										  
				if (f>fmax) f=fmax;
				ob->pblock->SetValue(PB_TOPWIDTH,0,f);
				ob->pblock->SetValue(PB_SIDEWIDTH,0,(f>xwid?xwid:f));
				ob->pblock->SetValue(PB_BOTWIDTH,0,f);
				ob->pmapParam->Invalidate();				
				ob->createmeth=ob->dlgCreateMeth;
				if (msg==MOUSE_POINT) 
				{  ob->suspendSnap = FALSE;	
				ob->increate=FALSE;
				   return CREATE_STOP;
				}
				break;
			}
	} else {
		if (msg == MOUSE_ABORT)
		{ob->increate=FALSE;
		return CREATE_ABORT;}
		}
	return 1;
	}

static CExtObjCreateCallBack cylCreateCB;

CreateMouseCallBack* CExtObject::GetCreateMouseCallBack() 
	{
	cylCreateCB.SetObj(this);
	return(&cylCreateCB);
	}

BOOL CExtObject::OKtoDisplay(TimeValue t) 
	{
	float radius;
	pblock->GetValue(PB_BOTLENGTH,t,radius,FOREVER);
	if (radius==0.0f) return FALSE;
	else return TRUE;
	}


// From ParamArray
BOOL CExtObject::SetValue(int i, TimeValue t, int v) 
	{
	switch (i) {
		case PB_CREATEMETHOD: dlgCreateMeth = v; break;
		}		
	return TRUE;
	}

BOOL CExtObject::SetValue(int i, TimeValue t, float v)
	{
	switch (i) {				
		case PB_TI_TOPLENGTH: crtTopLength = v; break;
		case PB_TI_SIDELENGTH: crtSideLength = v; break;
		case PB_TI_BOTLENGTH: crtBotLength = v; break;
		case PB_TI_TOPWIDTH: crtTopWidth = v; break;
		case PB_TI_SIDEWIDTH: crtSideWidth = v; break;
		case PB_TI_BOTWIDTH: crtBotWidth = v; break;
		case PB_TI_HEIGHT: crtHeight = v; break;
		}	
	return TRUE;
	}

BOOL CExtObject::SetValue(int i, TimeValue t, Point3 &v) 
	{
	switch (i) {
		case PB_TI_POS: crtPos = v; break;
		}		
	return TRUE;
	}

BOOL CExtObject::GetValue(int i, TimeValue t, int &v, Interval &ivalid) 
	{
	switch (i) {
		case PB_CREATEMETHOD: v = dlgCreateMeth; break;
		}
	return TRUE;
	}

BOOL CExtObject::GetValue(int i, TimeValue t, float &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_TOPLENGTH: v = crtTopLength; break;
		case PB_TI_SIDELENGTH: v = crtSideLength; break;
		case PB_TI_BOTLENGTH: v = crtBotLength; break;
		case PB_TI_TOPWIDTH: v = crtTopWidth; break;
		case PB_TI_SIDEWIDTH: v = crtSideWidth; break;
		case PB_TI_BOTWIDTH: v = crtBotWidth; break;
		case PB_TI_HEIGHT: v = crtHeight; break;
		}
	return TRUE;
	}

BOOL CExtObject::GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_POS: v = crtPos; break;		
		}
	return TRUE;
	}


void CExtObject::InvalidateUI() 
	{
	if (pmapParam) pmapParam->Invalidate();
	}

ParamDimension *CExtObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_TOPLENGTH:return stdWorldDim;
		case PB_SIDELENGTH:return stdWorldDim;
		case PB_BOTLENGTH:return stdWorldDim;
		case PB_TOPWIDTH: return stdWorldDim;
		case PB_SIDEWIDTH: return stdWorldDim;
		case PB_BOTWIDTH: return stdWorldDim;
		case PB_HEIGHT:return stdWorldDim;
		case PB_TSEGS: return stdSegmentsDim;
		case PB_SSEGS: return stdSegmentsDim;
		case PB_BSEGS: return stdSegmentsDim;
		case PB_WSEGS: return stdSegmentsDim;
		case PB_HSEGS: return stdSegmentsDim;
		default: return defaultDim;
		}
	}

TSTR CExtObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_TOPLENGTH: return TSTR(GetString(IDS_AP_BACKLENGTH));
		case PB_SIDELENGTH: return TSTR(GetString(IDS_RB_SIDELENGTH));
		case PB_BOTLENGTH: return TSTR(GetString(IDS_AP_FRONTLENGTH));
		case PB_TOPWIDTH:  return TSTR(GetString(IDS_AP_BACKWIDTH));
		case PB_SIDEWIDTH:  return TSTR(GetString(IDS_RB_SIDEWIDTH));
		case PB_BOTWIDTH:  return TSTR(GetString(IDS_AP_FRONTWIDTH));
		case PB_HEIGHT: return TSTR(GetString(IDS_RB_HEIGHT));
		case PB_TSEGS:  return TSTR(GetString(IDS_AP_BACKSEGS));
		case PB_SSEGS:  return TSTR(GetString(IDS_RB_SSEGS));
		case PB_BSEGS:  return TSTR(GetString(IDS_AP_FRONTSEGS));
		case PB_WSEGS:  return TSTR(GetString(IDS_RB_WSEGS));
		case PB_HSEGS:  return TSTR(GetString(IDS_RB_HSEGS));
		case PB_GENUVS:  return TSTR(GetString(IDS_MXS_GENUVS));
		default: return TSTR(_T(""));		
		}
	}

RefTargetHandle CExtObject::Clone(RemapDir& remap) 
	{
	CExtObject* newob = new CExtObject();	
	newob->ReplaceReference(0,pblock->Clone(remap));	
	newob->ivalid.SetEmpty();	
	BaseClone(this, newob, remap);
	return(newob);
	}

#define CYTPE_CHUNK			0x0100

IOResult CExtObject::Load(ILoad *iload) 
{
	ULONG nb;
	IOResult res = IO_OK;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {	
			case CYTPE_CHUNK:
				res=iload->Read(&createmeth,sizeof(int),&nb);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}

	return IO_OK;
}
IOResult CExtObject::Save(ISave* isave) 
{	ULONG nb;
	isave->BeginChunk(CYTPE_CHUNK);		
	isave->Write(&createmeth,sizeof(createmeth),&nb);
	isave->EndChunk();
 	return IO_OK;
}



