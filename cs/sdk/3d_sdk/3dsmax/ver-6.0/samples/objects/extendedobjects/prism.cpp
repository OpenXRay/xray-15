/**********************************************************************
 *<
	FILE:prism.cpp
	CREATED BY:  Audrey Peterson

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#include "solids.h"

#ifndef NO_OBJECT_PRISM

#include "iparamm.h"
#include "Simpobj.h"
#include "surf_api.h"

static Class_ID PRISM_CLASS_ID(0x63705fac, 0x5c1f553f);
class PrismObject : public SimpleObject, public IParamArray {
	public:
		// Class vars
		static IParamMap *pmapCreate;
		static IParamMap *pmapTypeIn;
		static IParamMap *pmapParam;		
		static IObjParam *ip;
		static float crtHeight,crtSide1,crtSide2,crtSide3;
		static int dlgHSegs, dlgSide1Segs, dlgSide2Segs,dlgSide3Segs;
		static int dlgCreateMeth;
		static Point3 crtPos;
		int isdone;
		
		PrismObject();		

		// From Object
		int CanConvertToType(Class_ID obtype);
		Object* ConvertToType(TimeValue t, Class_ID obtype);
		void GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist);
				
		// From BaseObject
		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);		
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		TCHAR *GetObjectName() { return GetString(IDS_RB_PRISM); }
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);
				
		// Animatable methods		
		void DeleteThis() { delete this; }
		Class_ID ClassID() { return PRISM_CLASS_ID; }  		
				
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

#define DEF_SEGMENTS 	1
#define DEF_SIDES		1

#define DEF_RADIUS		float(0.0)
#define DEF_HEIGHT		float(0.01)
#define DEF_FILLET		float(0.01)


//--- ClassDescriptor and class vars ---------------------------------

class PrismClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new PrismObject; }
	const TCHAR *	ClassName() { return GetString(IDS_AP_PRISM_CLASS); }
	SClass_ID		SuperClassID() { return GEOMOBJECT_CLASS_ID; }
	Class_ID		ClassID() { return PRISM_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_RB_EXTENDED);  }
	void			ResetClassParams(BOOL fileReset);
	};

static PrismClassDesc PrismDesc;

ClassDesc* GetPrismDesc() { return &PrismDesc; }

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// class variables for Prism class.
IObjParam *PrismObject::ip         = NULL;
int PrismObject::dlgHSegs			= DEF_SEGMENTS;
int PrismObject::dlgSide1Segs       = DEF_SIDES;
int PrismObject::dlgSide2Segs       = DEF_SIDES;
int PrismObject::dlgSide3Segs       = DEF_SIDES;
int PrismObject::dlgCreateMeth     = 1; // create_radius
IParamMap *PrismObject::pmapCreate = NULL;
IParamMap *PrismObject::pmapTypeIn = NULL;
IParamMap *PrismObject::pmapParam  = NULL;
Point3 PrismObject::crtPos         = Point3(0,0,0);
float PrismObject::crtHeight       = 0.0f;
float PrismObject::crtSide1       = 0.0f;
float PrismObject::crtSide2       = 0.0f;
float PrismObject::crtSide3       = 0.0f;

void PrismClassDesc::ResetClassParams(BOOL fileReset)
	{ PrismObject::dlgHSegs			= DEF_SEGMENTS;
	  PrismObject::dlgSide1Segs     = DEF_SIDES;
	  PrismObject::dlgSide2Segs     = DEF_SIDES;
	  PrismObject::dlgSide3Segs     = DEF_SIDES;
	  PrismObject::dlgCreateMeth     = 1; // create_radius
	  PrismObject::crtHeight       = 0.0f;
	  PrismObject::crtSide1       = 0.0f;
	  PrismObject::crtSide2       = 0.0f;
	  PrismObject::crtSide3       = 0.0f;
	  PrismObject::crtPos         = Point3(0,0,0);
	}

//--- Parameter map/block descriptors -------------------------------

// Parameter block indices
#define PB_SIDE1		0
#define PB_SIDE2		1
#define PB_SIDE3		2
#define PB_HEIGHT		3
#define PB_S1SEGS		4
#define PB_S2SEGS		5
#define PB_S3SEGS		6
#define PB_HSEGS		7
#define PB_GENUVS		8

// Non-parameter block indices
#define PB_CREATEMETHOD		0
#define PB_TI_POS			1
#define PB_TI_SIDE1			2
#define PB_TI_SIDE2			3
#define PB_TI_SIDE3			4
#define PB_TI_HEIGHT		5

#define BMIN_HEIGHT		float(0.1)
#define BMAX_HEIGHT		float(1.0E30)
#define BMIN_LENGTH		float(0.1)
#define BMAX_LENGTH		float(1.0E30)
//
//
//	Creation method

static int createMethIDs[] = {IDC_PR_CREATEBASE,IDC_PR_CREATEVERTICES};

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
		IDC_PR_POSX,IDC_PR_POSXSPIN,
		IDC_PR_POSY,IDC_PR_POSYSPIN,
		IDC_PR_POSZ,IDC_PR_POSZSPIN,
		float(-1.0E30),float(1.0E30),
		SPIN_AUTOSCALE),
	
	// Side1 Length
	ParamUIDesc(
		PB_TI_SIDE1,
		EDITTYPE_UNIVERSE,
		IDC_PR_SIDE1LEN,IDC_PR_SIDE1LENSPIN,
		BMIN_LENGTH,BMAX_LENGTH,
		SPIN_AUTOSCALE),

	// Side2 Length
	ParamUIDesc(
		PB_TI_SIDE2,
		EDITTYPE_UNIVERSE,
		IDC_PR_SIDE2LEN,IDC_PR_SIDE2LENSPIN,
		BMIN_LENGTH,BMAX_LENGTH,
		SPIN_AUTOSCALE),

	// Side3 Length
	ParamUIDesc(
		PB_TI_SIDE3,
		EDITTYPE_UNIVERSE,
		IDC_PR_SIDE3LEN,IDC_PR_SIDE3LENSPIN,
		BMIN_LENGTH,BMAX_LENGTH,
		SPIN_AUTOSCALE),

	// Height
	ParamUIDesc(
		PB_TI_HEIGHT,
		EDITTYPE_UNIVERSE,
		IDC_PR_HEIGHT,IDC_PR_HEIGHTSPIN,
		MIN_HEIGHT,BMAX_HEIGHT,
		SPIN_AUTOSCALE),	
	};
#define TYPEINDESC_LENGTH 5


//
//
// Parameters

static ParamUIDesc descParam[] = {
	// Side1 Length
	ParamUIDesc(
		PB_SIDE1,
		EDITTYPE_UNIVERSE,
		IDC_PR_SIDE1LEN,IDC_PR_SIDE1LENSPIN,
		BMIN_LENGTH,BMAX_LENGTH,
		SPIN_AUTOSCALE),

	// Side2 Length
	ParamUIDesc(
		PB_SIDE2,
		EDITTYPE_UNIVERSE,
		IDC_PR_SIDE2LEN,IDC_PR_SIDE2LENSPIN,
		BMIN_LENGTH,BMAX_LENGTH,
		SPIN_AUTOSCALE),

	// Side3 Length
	ParamUIDesc(
		PB_SIDE3,
		EDITTYPE_UNIVERSE,
		IDC_PR_SIDE3LEN,IDC_PR_SIDE3LENSPIN,
		BMIN_LENGTH,BMAX_LENGTH,
		SPIN_AUTOSCALE),

	// Height
	ParamUIDesc(
		PB_HEIGHT,
		EDITTYPE_UNIVERSE,
		IDC_PR_HEIGHT,IDC_PR_HEIGHTSPIN,
		MIN_HEIGHT,BMAX_HEIGHT,
		SPIN_AUTOSCALE),	
	
	
	// Side1 Segments
	ParamUIDesc(
		PB_S1SEGS,
		EDITTYPE_INT,
		IDC_PR_SIDE1SEGS,IDC_PR_SIDE1SEGSSPIN,
		(float)MIN_SEGMENTS,(float)MAX_SEGMENTS,
		0.1f),

	// Side2 Segments
	ParamUIDesc(
		PB_S2SEGS,
		EDITTYPE_INT,
		IDC_PR_SIDE2SEGS,IDC_PR_SIDE2SEGSSPIN,
		(float)MIN_SEGMENTS,(float)MAX_SEGMENTS,
		0.1f),

	// Side3 Segments
	ParamUIDesc(
		PB_S3SEGS,
		EDITTYPE_INT,
		IDC_PR_SIDE3SEGS,IDC_PR_SIDE3SEGSSPIN,
		(float)MIN_SEGMENTS,(float)MAX_SEGMENTS,
		0.1f),

	// Height Segments
	ParamUIDesc(
		PB_HSEGS,
		EDITTYPE_INT,
		IDC_PR_HGTSEGS,IDC_PR_HGTSPIN,
		(float)MIN_SEGMENTS,(float)MAX_SEGMENTS,
		0.1f),
	// Gen UVs
	ParamUIDesc(PB_GENUVS,TYPE_SINGLECHEKBOX,IDC_GENTEXTURE),			
	};
#define PARAMDESC_LENGTH 9


// variable type, NULL, animatable, number
ParamBlockDescID PrismdescVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 }, 
	{ TYPE_INT, NULL, TRUE, 4 }, 
	{ TYPE_INT, NULL, TRUE, 5 }, 
	{ TYPE_INT, NULL, TRUE, 6 }, 
	{ TYPE_INT, NULL, TRUE, 7 }, 
	{ TYPE_INT, NULL, FALSE, 8 }, 
	};

#define PBLOCK_LENGTH	9

#define NUM_OLDVERSIONS	0

#define CURRENT_VERSION	0
static ParamVersionDesc curVersion(PrismdescVer0,PBLOCK_LENGTH,CURRENT_VERSION);

void FixSide1(IParamBlock *pblock,TimeValue t,HWND hWnd)
{ float s1len,s2len,s3len,dl;

	pblock->GetValue(PB_SIDE1,t,s1len,FOREVER);
	pblock->GetValue(PB_SIDE2,t,s2len,FOREVER);
	pblock->GetValue(PB_SIDE3,t,s3len,FOREVER);
	dl=0.95f*(s2len+s3len);
	if (hWnd)
    { ISpinnerControl *spin2 = GetISpinner(GetDlgItem(hWnd,IDC_PR_SIDE1LENSPIN));
	  spin2->SetLimits(BMIN_LENGTH,dl,FALSE);
	  ReleaseISpinner(spin2);
	}
	if (s1len>dl) pblock->SetValue(PB_SIDE1,t,dl);
}
void FixSide2(IParamBlock *pblock,TimeValue t,HWND hWnd)
{ float s1len,s2len,s3len,dl;

	pblock->GetValue(PB_SIDE1,t,s1len,FOREVER);
	pblock->GetValue(PB_SIDE2,t,s2len,FOREVER);
	pblock->GetValue(PB_SIDE3,t,s3len,FOREVER);
	dl=0.95f*(s1len+s3len);
	if (hWnd)
    { ISpinnerControl *spin2 = GetISpinner(GetDlgItem(hWnd,IDC_PR_SIDE2LENSPIN));
	  spin2->SetLimits(BMIN_LENGTH,dl,FALSE);
	  ReleaseISpinner(spin2);
	}
	if (s2len>dl) pblock->SetValue(PB_SIDE2,t,dl);
}
void FixSide3(IParamBlock *pblock,TimeValue t,HWND hWnd)
{ float s1len,s2len,s3len,dl;

	pblock->GetValue(PB_SIDE1,t,s1len,FOREVER);
	pblock->GetValue(PB_SIDE2,t,s2len,FOREVER);
	pblock->GetValue(PB_SIDE3,t,s3len,FOREVER);
	dl=0.95f*(s1len+s2len);
	if (hWnd)
    { ISpinnerControl *spin2 = GetISpinner(GetDlgItem(hWnd,IDC_PR_SIDE3LENSPIN));
	  spin2->SetLimits(BMIN_LENGTH,dl,FALSE);
	  ReleaseISpinner(spin2);
	}
	if (s3len>dl) pblock->SetValue(PB_SIDE3,t,dl);
}
//--- TypeInDlgProc --------------------------------

class PrismTypeInDlgProc : public ParamMapUserDlgProc {
	public:
		PrismObject *ob;

		PrismTypeInDlgProc(PrismObject *o) {ob=o;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

BOOL PrismTypeInDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_PR_CREATE: {
					if (ob->crtHeight==0.0f) return TRUE;
					
					// We only want to set the value if the object is 
					// not in the scene.
					if (ob->TestAFlag(A_OBJ_CREATING)) {
						ob->pblock->SetValue(PB_HEIGHT,0,ob->crtHeight);
						ob->pblock->SetValue(PB_SIDE1,0,ob->crtSide1);
						ob->pblock->SetValue(PB_SIDE2,0,ob->crtSide2);
						ob->pblock->SetValue(PB_SIDE3,0,ob->crtSide3);
						}

					Matrix3 tm(1);
					tm.SetTrans(ob->crtPos);
					ob->suspendSnap = FALSE;ob->isdone=TRUE;
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
class PrismSideDlgProc : public ParamMapUserDlgProc {
	public:
		PrismObject *ob;

		PrismSideDlgProc(PrismObject *o) {ob=o;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

BOOL PrismSideDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{ 	switch (msg) {
		case CC_SPINNER_CHANGE:
			switch ( LOWORD(wParam) ) {
				case IDC_PR_SIDE1LENSPIN:
					FixSide1(ob->pblock,t,hWnd);
			return TRUE;
				case IDC_PR_SIDE2LENSPIN:
					FixSide2(ob->pblock,t,hWnd);
			return TRUE;
				case IDC_PR_SIDE3LENSPIN:
					FixSide3(ob->pblock,t,hWnd);
			return TRUE;
				}
		}
	return FALSE;
	}



//--- Prism methods -------------------------------

PrismObject::PrismObject() 
	{
	MakeRefByID(FOREVER, 0, CreateParameterBlock(PrismdescVer0, PBLOCK_LENGTH, CURRENT_VERSION));
	
	pblock->SetValue(PB_S1SEGS,0,dlgSide1Segs);
	pblock->SetValue(PB_S2SEGS,0,dlgSide2Segs);
	pblock->SetValue(PB_S3SEGS,0,dlgSide3Segs);
	pblock->SetValue(PB_HSEGS,0,dlgHSegs);

	pblock->SetValue(PB_SIDE1,0,crtSide1);
	pblock->SetValue(PB_SIDE2,0,crtSide2);
	pblock->SetValue(PB_SIDE3,0,crtSide3);
	pblock->SetValue(PB_HEIGHT,0,crtHeight);
	pblock->SetValue(PB_GENUVS,0, TRUE);				
	isdone=FALSE;
	}

IOResult PrismObject::Load(ILoad *iload) 
	{
	return IO_OK;
	}


void PrismObject::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
	{
	SimpleObject::BeginEditParams(ip,flags,prev);
	this->ip = ip;

	if (pmapCreate && pmapParam) {
		
		// Left over from last Prism ceated
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
				MAKEINTRESOURCE(IDD_PRISM1),
				GetString(IDS_RB_CREATE_DIALOG),
				0);

			pmapTypeIn = CreateCPParamMap(
				descTypeIn,TYPEINDESC_LENGTH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_PRISM2),
				GetString(IDS_RB_KEYBOARDENTRY),
				APPENDROLL_CLOSED);			
			}

		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_PRISM3),
			GetString(IDS_AP_PARAMETERS),
			0);
		}

	if(pmapTypeIn) {
		// A callback for the type in.
		pmapTypeIn->SetUserDlgProc(new PrismTypeInDlgProc(this));
		}	
	if(pmapParam) {
		pmapParam->SetUserDlgProc(new PrismSideDlgProc(this));
		}
	}
		
void PrismObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
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
	pblock->GetValue(PB_S1SEGS,ip->GetTime(),dlgSide1Segs,FOREVER);
	pblock->GetValue(PB_S2SEGS,ip->GetTime(),dlgSide2Segs,FOREVER);	
	pblock->GetValue(PB_S3SEGS,ip->GetTime(),dlgSide3Segs,FOREVER);	
	pblock->GetValue(PB_HSEGS,ip->GetTime(),dlgHSegs,FOREVER);		
	}

/*void PrismObject::SetParams(float rad, float height, int segs, int sides, int capsegs, BOOL smooth, 
	pblock->SetValue(PB_SIDE1,0,side1);
	pblock->SetValue(PB_SIDE2,0,side2);
	pblock->SetValue(PB_SIDE3,0,side3);
	pblock->SetValue(PB_HEIGHT,0,height);
	pblock->SetValue(PB_S1SEGS,0,s1segs);
	pblock->SetValue(PB_S2SEGS,0,s2segs);
	pblock->SetValue(PB_S3SEGS,0,s3segs);
	pblock->SetValue(PB_HSEGS,0,hsegs);
	pblock->SetValue(PB_CSEGS,0,csegs);
	pblock->SetValue(PB_GENUVS,0, genUV);
*/

void BuildPrismMesh(Mesh &mesh,
		int s1segs, int s2segs, int s3segs, int llsegs, 
		float s1len, float s2len, float s3len, float height,
		int genUVs)
{	BOOL minush=(height<0.0f);
	if (minush) height=-height;
	int nf=0,totalsegs=s1segs+s2segs+s3segs;
	float s13len=s1len*s3len;
	if ((s1len<=0.0f)||(s2len<=0.0f)||(s3len<=0.0f))
	{ mesh.setNumVerts(0);
	  mesh.setNumFaces(0);
	  mesh.setNumTVerts(0);
	  mesh.setNumTVFaces(0);
	}
	else
	{ float acvalue=(s2len*s2len-s1len*s1len-s3len*s3len)/(-2.0f*s13len);
	  acvalue=(acvalue<-1.0f?-1.0f:(acvalue>1.0f?acvalue=1.0f:acvalue));
	  float theta=(float)acos(acvalue);
	int nfaces,ntverts;
	nfaces=2*totalsegs*(llsegs+1);
	int nverts=totalsegs*(llsegs+1)+2;

	mesh.setNumVerts(nverts);
	mesh.setNumFaces(nfaces);
	if (genUVs) 
	{ ntverts=nverts+2*totalsegs+llsegs+1;
	  mesh.setNumTVerts(ntverts);
	  mesh.setNumTVFaces(nfaces);
	} 
	else 
	{ mesh.setNumTVerts(0);
	  mesh.setNumTVFaces(0);
	}
	
	Point3 Pt0=Point3(0.0f,0.0f,height),Pt1=Point3(s1len,0.0f,height);
	Point3 Pt2=Point3(s3len*(float)cos(theta),s3len*(float)sin(theta),height);
	Point3 CenterPt=(Pt0+Pt1+Pt2)/3.0f,Mins;
	float maxx;
	if (s1len<Pt2.x) {Mins.x=s1len;maxx=Pt2.x;}
	else {Mins.x=Pt2.x;maxx=s1len;}
	if (maxx<0.0f) maxx=0.0f;
	if (Mins.x>0.0f) Mins.x=0.0f;
	Mins.y=0.0f;
	float xdist=maxx-Mins.x,ydist=Pt2.y;
	if (xdist==0.0f) xdist=0.001f;if (ydist==0.0f) ydist=0.001f;
	mesh.setVert(0,CenterPt);
	mesh.setVert(nverts-1, Point3(CenterPt.x,CenterPt.y,0.0f));
	mesh.setVert(1,Pt0);
	int botstart=ntverts-totalsegs-1,tnv=totalsegs+1;
	float u=0.0f,yval=Pt2.y,bu,tu,tb;
	if (genUVs) 
	{ mesh.setTVert(0,tu=(CenterPt.x-Mins.x)/xdist,tb=(CenterPt.y-Mins.y)/ydist,0.0f);
	  mesh.setTVert(1,bu=(-Mins.x/xdist),0.0f,0.0f);
	  mesh.setTVert(tnv++,0.0f,1.0f,0.0f);
	  mesh.setTVert(botstart++,1.0f-bu,1.0f,0.0f);
	  mesh.setTVert(ntverts-1,1.0f-tu,1.0f-tb,0.0f);
	}
	int i,nv=2;
	float sincr=s1len/s1segs,tdist=s1len+s2len+s3len,udiv=sincr/tdist,pos;
	if (tdist==0.0f) tdist=0.0001f;
	for (i=1;i<s1segs;i++)
	{ mesh.setVert(nv,Point3(pos=sincr*i,0.0f,height));
	  if (genUVs) 
	  { mesh.setTVert(nv,bu=(pos-Mins.x)/xdist,0.0f,0.0f);
	    mesh.setTVert(tnv++,u+=udiv,1.0f,0.0f);
	    mesh.setTVert(botstart++,1.0f-bu,1.0f,0.0f);
	  }
	  nv++;
	}
	mesh.setVert(nv,Pt1);
	if (genUVs)
	{ mesh.setTVert(nv,bu=(Pt1.x-Mins.x)/xdist,0.0f,0.0f);
	  mesh.setTVert(tnv++,u+=udiv,1.0f,0.0f);
	  mesh.setTVert(botstart++,1.0f-bu,1.0f,0.0f);
	}
	Point3 slope=(Pt2-Pt1)/(float)s2segs;
	float ypos,bv;
	nv++;udiv=(s2len/s2segs)/tdist;
	for (i=1;i<s2segs;i++)
	{ mesh.setVert(nv,Point3(pos=(Pt1.x+slope.x*i),ypos=(Pt1.y+slope.y*i),height));
	  if (genUVs) 
	  { mesh.setTVert(nv,bu=(pos-Mins.x)/xdist,bv=(ypos-Mins.y)/ydist,0.0f);
	    mesh.setTVert(tnv++,u+=udiv,1.0f,0.0f);
	    mesh.setTVert(botstart++,1.0f-bu,1.0f-bv,0.0f);
	  }
	  nv++;
	}
	mesh.setVert(nv,Pt2);
	if (genUVs)
	{ mesh.setTVert(nv,bu=(Pt2.x-Mins.x)/xdist,1.0f,0.0f);
	  mesh.setTVert(tnv++,u+=udiv,1.0f,0.0f);
	  mesh.setTVert(botstart++,1.0f-bu,0.0f,0.0f);
	}
	nv++; slope=(Pt0-Pt2)/(float)s3segs;udiv=(s2len/s2segs)/tdist;
	for (i=1;i<s3segs;i++)
	{ mesh.setVert(nv,Point3(pos=(Pt2.x+slope.x*i),ypos=(Pt2.y+slope.y*i),height));
	  if (genUVs) 
	  { mesh.setTVert(nv,bu=(pos-Mins.x)/xdist,bv=(ypos-Mins.y)/ydist,0.0f);
	    mesh.setTVert(tnv++,u+=udiv,1.0f,0.0f);
	    mesh.setTVert(botstart++,1.0f-bu,1.0f-bv,0.0f);
	  }
	  nv++;
	}
	if (genUVs)	mesh.setTVert(tnv++,1.0f,1.0f,0.0f);
	//top layer done, now reflect sides down 
	int sidevs,startv=1,deltav,ic;
	startv=1;
	sincr=height/llsegs;
	Point3 p;
	for (sidevs=0;sidevs<totalsegs;sidevs++)
	{ p=mesh.verts[startv];
	  deltav=totalsegs;
	  for (ic=1;ic<=llsegs;ic++)
	  { p.z =height-sincr*ic;
	    mesh.setVert(startv+deltav, p);
		deltav+=totalsegs;
	  }
	  startv++;
	}
	if (genUVs)
	{ startv=totalsegs+1;
	  int tvseg=totalsegs+1;
	  for (sidevs=0;sidevs<=totalsegs;sidevs++)
	  { p=mesh.tVerts[startv];
	    deltav=tvseg;
	    for (ic=1;ic<=llsegs;ic++)
	    { p.y =1.0f-ic/(float)llsegs;
	      mesh.setTVert(startv+deltav,p);
	      deltav+=tvseg;
	    }
   	   startv++;
	 }
	}
    int fc=0,sidesm=2;
	int last=totalsegs-1;
	// Now make faces ---
	int j,b0=1,b1=2,tb0,tb1,tt0,tt1,t0,t1,ecount=0,s2end=s1segs+s2segs;
	for (i=0;i<totalsegs;i++)
	{ if (genUVs) mesh.tvFace[fc].setTVerts(0,b0,(i<last?b1:1));
	AddFace(&mesh.faces[fc++],0,b0++,(i<last?b1++:1),0,1);
	}
	tt1=(tt0=i+1)+1;t0=1;t1=2;b1=(b0=t0+totalsegs)+1;
	tb1=(tb0=tt1+totalsegs)+1;
	for (i=1;i<=llsegs;i++)
	{ for (j=0;j<totalsegs;j++)
	  { if (genUVs) 
		{ mesh.tvFace[fc].setTVerts(tt0,tb0++,tb1);
		  mesh.tvFace[fc+1].setTVerts(tt0++,tb1++,tt1++);
		}
	    if (j<s1segs) sidesm=2;
		else if (j<s2end) sidesm=4;
		else sidesm=8;
	  AddFace(&mesh.faces[fc++],t0,b0++,(j==last?t1:b1),0,sidesm);
	  if (j<last)
	    AddFace(&mesh.faces[fc++],t0++,b1,t1,1,sidesm);
	  else
	   AddFace(&mesh.faces[fc++],t0++,b1-totalsegs,t1-totalsegs,1,sidesm);
	  t1++;b1++;
	  }
	 tt0++;tt1++;tb0++;tb1++;
	}
	if (genUVs) {tt0=(tt1=ntverts-totalsegs)-1;tb0=ntverts-1;}
	for (i=0;i<totalsegs;i++)
	{ if (genUVs)
	   { mesh.tvFace[fc].setTVerts(tt0++,tb0,(i==last?tt1-totalsegs:tt1));
	     tt1++;
	   }
	AddFace(&mesh.faces[fc++],t0++,b0,(i==last?t1-totalsegs:t1),1,1);
	  t1++;
	}
	if (minush)
	for (i=0;i<nverts;i++) mesh.verts[i].z-=height;
	assert(fc==mesh.numFaces);
//	assert(nv==mesh.numVerts); */
}
	mesh.InvalidateTopologyCache();
	}
BOOL PrismObject::HasUVW() { 
	BOOL genUVs;
	Interval v;
	pblock->GetValue(PB_GENUVS, 0, genUVs, v);
	return genUVs; 
	}

void PrismObject::SetGenUVW(BOOL sw) {  
	if (sw==HasUVW()) return;
	pblock->SetValue(PB_GENUVS,0, sw);				
	}

void PrismObject::BuildMesh(TimeValue t)
	{	
	int hsegs,s1segs,s2segs,s3segs;
	float height,s1len,s2len,s3len;
	int genUVs;	

	if (isdone)
	{  FixSide1(pblock,t,(pmapParam?pmapParam->GetHWnd():NULL));
	   FixSide2(pblock,t,(pmapParam?pmapParam->GetHWnd():NULL));
	   FixSide3(pblock,t,(pmapParam?pmapParam->GetHWnd():NULL));
	}

	// Start the validity interval at forever and widdle it down.
	ivalid = FOREVER;
	
	pblock->GetValue(PB_HSEGS,t,hsegs,ivalid);
	pblock->GetValue(PB_S1SEGS,t,s1segs,ivalid);
	pblock->GetValue(PB_S2SEGS,t,s2segs,ivalid);
	pblock->GetValue(PB_S3SEGS,t,s3segs,ivalid);
	pblock->GetValue(PB_HEIGHT,t,height,ivalid);
	pblock->GetValue(PB_SIDE1,t,s1len,ivalid);
	pblock->GetValue(PB_SIDE2,t,s2len,ivalid);
	pblock->GetValue(PB_SIDE3,t,s3len,ivalid);
	pblock->GetValue(PB_GENUVS,t,genUVs,ivalid);	
	LimitValue(height, MIN_HEIGHT, MAX_HEIGHT);
	LimitValue(s1len, BMIN_HEIGHT, MAX_HEIGHT);
	LimitValue(s2len, BMIN_HEIGHT, MAX_HEIGHT);
	LimitValue(s3len, BMIN_HEIGHT, MAX_HEIGHT);
	LimitValue(hsegs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(s1segs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(s2segs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(s3segs, MIN_SEGMENTS, MAX_SEGMENTS);
	
	BuildPrismMesh(mesh, s1segs, s2segs, s3segs, hsegs, 
		s1len, s2len, s3len, height, genUVs);
	}

inline Point3 operator+(const PatchVert &pv,const Point3 &p)
	{
	return p+pv.p;
	}


#ifndef NO_NURBS

Object*
BuildNURBSPrism(float side1, float side2, float side3, float height, int genUVs)
{
	float s13len=side1*side3;
	float theta = (float)acos((side2*side2 - side1*side1 - side3*side3)/(-2.0f*s13len));

	int prism_faces[5][4] = {{0, 1, 2, 2}, // bottom
							{1, 0, 4, 3}, // front
							{2, 1, 5, 4}, // left
							{0, 2, 3, 5}, // right
							{4, 3, 5, 5}};// top
	Point3 prism_verts[6] ={Point3(0.0f, 0.0f, 0.0f),
							Point3(side1,  0.0f, 0.0f),
							Point3(side3*(float)cos(theta), side3*(float)sin(theta), 0.0f),
							Point3(0.0f, 0.0f, height),
							Point3(side1,  0.0f, height),
							Point3(side3*(float)cos(theta), side3*(float)sin(theta), height)};

	NURBSSet nset;

	for (int face = 0; face < 5; face++) {
		Point3 bl = prism_verts[prism_faces[face][0]];
		Point3 br = prism_verts[prism_faces[face][1]];
		Point3 tl = prism_verts[prism_faces[face][2]];
		Point3 tr = prism_verts[prism_faces[face][3]];

		NURBSCVSurface *surf = new NURBSCVSurface();
		nset.AppendObject(surf);
		surf->SetUOrder(4);
		surf->SetVOrder(4);
		surf->SetNumCVs(4, 4);
		surf->SetNumUKnots(8);
		surf->SetNumVKnots(8);

		Point3 top, bot;
		for (int r = 0; r < 4; r++) {
			top = tl + (((float)r/3.0f) * (tr - tl));
			bot = bl + (((float)r/3.0f) * (br - bl));
			for (int c = 0; c < 4; c++) {
				NURBSControlVertex ncv;
				ncv.SetPosition(0, bot + (((float)c/3.0f) * (top - bot)));
				ncv.SetWeight(0, 1.0f);
				surf->SetCV(r, c, ncv);
			}
		}

		for (int k = 0; k < 4; k++) {
			surf->SetUKnot(k, 0.0);
			surf->SetVKnot(k, 0.0);
			surf->SetUKnot(k + 4, 1.0);
			surf->SetVKnot(k + 4, 1.0);
		}

		surf->Renderable(TRUE);
		surf->SetGenerateUVs(genUVs);
		if (height > 0.0f)
			surf->FlipNormals(TRUE);
		else
			surf->FlipNormals(FALSE);

		float sum = side1 + side2 + side3;
		float s1 = side1/sum;
		float s3 = 1.0f - side3/sum;
		switch(face) {
		case 0:
			surf->SetTextureUVs(0, 0, Point2(0.5f, 0.0f));
			surf->SetTextureUVs(0, 1, Point2(0.5f, 0.0f));
			surf->SetTextureUVs(0, 2, Point2(1.0f, 1.0f));
			surf->SetTextureUVs(0, 3, Point2(0.0f, 1.0f));
			break;
		case 1:
			surf->SetTextureUVs(0, 0, Point2(s1, 1.0f));
			surf->SetTextureUVs(0, 1, Point2(0.0f, 1.0f));
			surf->SetTextureUVs(0, 2, Point2(s1, 0.0f));
			surf->SetTextureUVs(0, 3, Point2(0.0f, 0.0f));
			break;
		case 2:
			surf->SetTextureUVs(0, 0, Point2(s3, 1.0f));
			surf->SetTextureUVs(0, 1, Point2(s1, 1.0f));
			surf->SetTextureUVs(0, 2, Point2(s3, 0.0f));
			surf->SetTextureUVs(0, 3, Point2(s1, 0.0f));
			break;
		case 3:
			surf->SetTextureUVs(0, 0, Point2(1.0f, 1.0f));
			surf->SetTextureUVs(0, 1, Point2(s3, 1.0f));
			surf->SetTextureUVs(0, 2, Point2(1.0f, 0.0f));
			surf->SetTextureUVs(0, 3, Point2(s3, 0.0f));
			break;
		case 4:
			surf->SetTextureUVs(0, 0, Point2(0.5f, 1.0f));
			surf->SetTextureUVs(0, 1, Point2(0.5f, 1.0f));
			surf->SetTextureUVs(0, 2, Point2(1.0f, 0.0f));
			surf->SetTextureUVs(0, 3, Point2(0.0f, 0.0f));
			break;
		}

		char bname[80];
		sprintf(bname, "%s%02d", GetString(IDS_CT_SURF), face);
		surf->SetName(bname);
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
	// Bottom(0) to Front (1)
	F(0, 1, 3, 0, 0, 0);
	F(0, 1, 2, 0, 1, 0);
	F(0, 1, 1, 0, 2, 0);
	F(0, 1, 0, 0, 3, 0);

	// Bottom(0) to Left (2)
	F(0, 2, 3, 0, 3, 0);
	F(0, 2, 3, 1, 2, 0);
	F(0, 2, 3, 2, 1, 0);
	F(0, 2, 3, 3, 0, 0);

	// Bottom(0) to Right (3)
	F(0, 3, 0, 0, 0, 0);
	F(0, 3, 0, 1, 1, 0);
	F(0, 3, 0, 2, 2, 0);
	F(0, 3, 0, 3, 3, 0);

	// Top(4) to Front (1)
	F(4, 1, 3, 0, 3, 3);
	F(4, 1, 2, 0, 2, 3);
	F(4, 1, 1, 0, 1, 3);
	F(4, 1, 0, 0, 0, 3);

	// Top(4) to Left (2)
	F(4, 2, 0, 0, 3, 3);
	F(4, 2, 0, 1, 2, 3);
	F(4, 2, 0, 2, 1, 3);
	F(4, 2, 0, 3, 0, 3);

	// Top(4) to Right (3)
	F(4, 3, 3, 0, 0, 3);
	F(4, 3, 3, 1, 1, 3);
	F(4, 3, 3, 2, 2, 3);
	F(4, 3, 3, 3, 3, 3);

	// Front(1) to Left (2)
	F(1, 2, 0, 1, 3, 1);
	F(1, 2, 0, 2, 3, 2);

	// Left(2) to Right (3)
	F(2, 3, 0, 1, 3, 1);
	F(2, 3, 0, 2, 3, 2);

	// Right(3) to Front (1)
	F(3, 1, 0, 1, 3, 1);
	F(3, 1, 0, 2, 3, 2);

	// Fuse the triangles together
	for (int i = 1; i < 4; i++) {
		F(0, 0, 0, 3, i, 3);
		F(4, 4, 0, 3, i, 3);
	}

	Matrix3 mat;
	mat.IdentityMatrix();
	Object *obj = CreateNURBSObject(NULL, &nset, mat);
	return obj;
}

#endif

Object* PrismObject::ConvertToType(TimeValue t, Class_ID obtype)
	{
#ifndef NO_NURBS
	if (obtype == EDITABLE_SURF_CLASS_ID) {
		Interval valid = FOREVER;
		float side1, side2, side3, height;
		int genUVs;
		pblock->GetValue(PB_SIDE1,t,side1,valid);
		pblock->GetValue(PB_SIDE2,t,side2,valid);
		pblock->GetValue(PB_SIDE3,t,side3,valid);
		pblock->GetValue(PB_HEIGHT,t,height,valid);
		pblock->GetValue(PB_GENUVS,t,genUVs,valid);
		Object *ob = BuildNURBSPrism(side1, side2, side3, height, genUVs);
		ob->SetChannelValidity(TOPO_CHAN_NUM,valid);
		ob->SetChannelValidity(GEOM_CHAN_NUM,valid);
		ob->UnlockObject();
		return ob;
	} 
#endif

    return SimpleObject::ConvertToType(t,obtype);
	}

int PrismObject::CanConvertToType(Class_ID obtype)
	{
#ifndef NO_NURBS
	if (obtype == EDITABLE_SURF_CLASS_ID)
        return 1;
#endif
	if (obtype == triObjectClassID)
		return 1;

    return SimpleObject::CanConvertToType(obtype);
	}

void PrismObject::GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist)
{
    Object::GetCollapseTypes(clist, nlist);
    Class_ID id = EDITABLE_SURF_CLASS_ID;
    TSTR *name = new TSTR(GetString(IDS_SM_NURBS_SURFACE));
    clist.Append(1,&id);
    nlist.Append(1,&name);
}


class PrismObjCreateCallBack: public CreateMouseCallBack {
	PrismObject *ob;	
	Point3 p0,p1,p2,tmp,d;
	IPoint2 sp0,sp1,sp2;
	float l,hd;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		void SetObj(PrismObject *obj) { ob = obj; }
	};

int PrismObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) 
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
		case 0:
			{	ob->suspendSnap = TRUE;				
				sp0 = m;				
				p0 = vpt->SnapPoint(m,m,NULL,snapdim);
				mat.SetTrans(p0); // Set Node's transform				
				ob->pblock->SetValue(PB_SIDE1,0,0.01f);
				ob->pblock->SetValue(PB_SIDE2,0,0.01f);
				ob->pblock->SetValue(PB_SIDE3,0,0.01f);
				ob->pblock->SetValue(PB_HEIGHT,0,0.01f);
				break;}
			case 1: 
			{	mat.IdentityMatrix();
				sp1 = m;
				p1 = vpt->SnapPoint(m,m,NULL,snapdim);
				mat.SetTrans(p0);
				d = p1-p0;
				if (flags&MOUSE_CTRL) {
					// Constrain to square base
					float len;
					if (fabs(d.x) > fabs(d.y)) len = d.x;
					else len = d.y;
					d.x = d.y = 2.0f * len;
					}
				hd=d.x/2.0f,l=(float)sqrt(hd*hd+d.y*d.y);	
				float dl=0.95f*(l+l);
				hd=(float)fabs(d.x);
				if (hd>dl) hd=dl;
				dl=0.95f*(hd+l);
				if (l>dl) l=dl;
				ob->pblock->SetValue(PB_SIDE1,0,hd);
				ob->pblock->SetValue(PB_SIDE2,0,l);
				ob->pblock->SetValue(PB_SIDE3,0,l);
				ob->pmapParam->Invalidate();				
				if (msg==MOUSE_POINT )
				{if (Length(sp1-sp0)<3 || Length(d)<0.1f) 
				  { return CREATE_ABORT;}
				else {tmp=(p2=p0+Point3(hd/2.0f,l,0.0f));} }
				break; }
			case 2:
				if (!ob->dlgCreateMeth) 
				{
#ifdef _OSNAP
				  float h = vpt->SnapLength(vpt->GetCPDisp(p1,Point3(0,0,1),sp1,m,TRUE));
#else
				  float h = vpt->SnapLength(vpt->GetCPDisp(p1,Point3(0,0,1),sp1,m));
#endif
				  ob->pblock->SetValue(PB_HEIGHT,0,h);
				  ob->pmapParam->Invalidate();				
				  if (msg==MOUSE_POINT)
				  {	ob->suspendSnap = FALSE;
					return (Length(m-sp0)<3)?CREATE_ABORT:CREATE_STOP;
				  }
				}
				else
				{ sp2=m;
				  Point3 newpt = vpt->SnapPoint(m,m,NULL,snapdim);
				  p2=tmp+(newpt-p1);
				  d=p2-p0;
				  float l2,dl,l3,l1;
				  l1=hd;
				  l3=(float)sqrt(d.x*d.x+d.y*d.y);
				  Point3 midpt=p0+Point3(hd,0.0f,0.0f);
				  d=p2-midpt;l2=(float)sqrt(d.x*d.x+d.y*d.y);
				  dl=0.95f*(l2+l3);
				  if (l1>dl) 
				  { l1=dl;				  
				    ob->pblock->SetValue(PB_SIDE1,0,(hd=l1));
				  }
				  dl=0.95f*(l1+l3);
				  if (l2>dl) l2=dl;
				  ob->pblock->SetValue(PB_SIDE2,0,l2);
				  dl=0.95f*(l1+l2);
				  if (l3>dl) l3=dl;
				  ob->pblock->SetValue(PB_SIDE3,0,l3);
				  ob->pmapParam->Invalidate();				
				  if (msg==MOUSE_POINT)
			  	  {	if (Length(m-sp0)<3) CREATE_ABORT; }
				}
				break;

			case 3:
#ifdef _OSNAP
				float h = vpt->SnapLength(vpt->GetCPDisp(p1,Point3(0,0,1),sp2,m,TRUE));
#else
				float h = vpt->SnapLength(vpt->GetCPDisp(p1,Point3(0,0,1),sp2,m));
#endif
				ob->pblock->SetValue(PB_HEIGHT,0,h);
				ob->pmapParam->Invalidate();				
				if (msg==MOUSE_POINT) 
				{   ob->suspendSnap = FALSE;ob->isdone=TRUE;
					return CREATE_STOP;
				}
				break;

			}
	} else {
		if (msg == MOUSE_ABORT)
			return CREATE_ABORT;
		}
	return 1;
	}

static PrismObjCreateCallBack cylCreateCB;

CreateMouseCallBack* PrismObject::GetCreateMouseCallBack() 
	{
	cylCreateCB.SetObj(this);
	return(&cylCreateCB);
	}

BOOL PrismObject::OKtoDisplay(TimeValue t) 
	{
	float radius;
	pblock->GetValue(PB_SIDE1,t,radius,FOREVER);
	if (radius==0.0f) return FALSE;
	else return TRUE;
	}


// From ParamArray
BOOL PrismObject::SetValue(int i, TimeValue t, int v) 
	{
	switch (i) {
		case PB_CREATEMETHOD: dlgCreateMeth = v; break;
		}		
	return TRUE;
	}

BOOL PrismObject::SetValue(int i, TimeValue t, float v)
	{
	switch (i) {				
		case PB_TI_SIDE1: crtSide1 = v; break;
		case PB_TI_SIDE2: crtSide2 = v; break;
		case PB_TI_SIDE3: crtSide3 = v; break;
		case PB_TI_HEIGHT: crtHeight = v; break;
		}	
	return TRUE;
	}

BOOL PrismObject::SetValue(int i, TimeValue t, Point3 &v) 
	{
	switch (i) {
		case PB_TI_POS: crtPos = v; break;
		}		
	return TRUE;
	}

BOOL PrismObject::GetValue(int i, TimeValue t, int &v, Interval &ivalid) 
	{
	switch (i) {
		case PB_CREATEMETHOD: v = dlgCreateMeth; break;
		}
	return TRUE;
	}

BOOL PrismObject::GetValue(int i, TimeValue t, float &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_SIDE1: v = crtSide1; break;
		case PB_TI_SIDE2: v = crtSide2; break;
		case PB_TI_SIDE3: v = crtSide3; break;
		case PB_TI_HEIGHT: v = crtHeight; break;
		}
	return TRUE;
	}

BOOL PrismObject::GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_POS: v = crtPos; break;		
		}
	return TRUE;
	}


void PrismObject::InvalidateUI() 
	{
	if (pmapParam) pmapParam->Invalidate();
	}

ParamDimension *PrismObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_SIDE1: return stdWorldDim;
		case PB_SIDE2: return stdWorldDim;
		case PB_SIDE3: return stdWorldDim;
		case PB_HEIGHT: return stdWorldDim;
		case PB_S1SEGS: return stdSegmentsDim;
		case PB_S2SEGS: return stdSegmentsDim;
		case PB_S3SEGS: return stdSegmentsDim;
		case PB_HSEGS: return stdSegmentsDim;
		default: return defaultDim;
		}
	}

TSTR PrismObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_SIDE1: return TSTR(GetString(IDS_RB_SIDE1));
		case PB_SIDE2: return TSTR(GetString(IDS_RB_SIDE2));
		case PB_SIDE3: return TSTR(GetString(IDS_RB_SIDE3));
		case PB_HEIGHT: return TSTR(GetString(IDS_RB_HEIGHT));
		case PB_S1SEGS:  return TSTR(GetString(IDS_RB_S1SEGS));
		case PB_S2SEGS:  return TSTR(GetString(IDS_RB_S2SEGS));
		case PB_S3SEGS:  return TSTR(GetString(IDS_RB_S3SEGS));
		case PB_HSEGS:  return TSTR(GetString(IDS_RB_HSEGS));
		case PB_GENUVS:  return TSTR(GetString(IDS_MXS_GENUVS));
		default: return TSTR(_T(""));		
		}
	}

RefTargetHandle PrismObject::Clone(RemapDir& remap) 
	{
	PrismObject* newob = new PrismObject();	
	newob->ReplaceReference(0,pblock->Clone(remap));	
	newob->ivalid.SetEmpty();	
	BaseClone(this, newob, remap);
	return(newob);
	}

#endif // NO_OBJECT_PRISM


