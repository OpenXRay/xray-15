/**********************************************************************
 *<
	FILE: gengon.cpp - builds generalized n-gonal extrusions
	CREATED BY:  Audrey Peterson

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#include "solids.h"
#include "iparamm.h"
#include "Simpobj.h"

static Class_ID GENGON_CLASS_ID(0x49bf599f, 0x35f945ab);
class GengonObject : public SimpleObject, public IParamArray {
	public:
		// Class vars
		static IParamMap *pmapCreate;
		static IParamMap *pmapTypeIn;
		static IParamMap *pmapParam;		
		static IObjParam *ip;
		static float crtRadius,crtFillet;
		static float crtHeight;
		static int dlgHSegs, dlgSides, dlgFSegs,dlgSSegs;
		static int dlgCreateMeth;
		static Point3 crtPos;	
		BOOL increate;
		
		GengonObject();		

		// From Object
		int CanConvertToType(Class_ID obtype);
		Object* ConvertToType(TimeValue t, Class_ID obtype);
				
		// From BaseObject
		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);		
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		TCHAR *GetObjectName() { return GetString(IDS_RB_GENGON); }
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);
				
		// Animatable methods		
		void DeleteThis() { delete this; }
		Class_ID ClassID() { return GENGON_CLASS_ID; }  		
				
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
#define DEF_SIDES		5

#define DEF_RADIUS		float(0.0)
#define DEF_HEIGHT		float(0.01)
#define DEF_FILLET		float(0.01)

#define SMOOTH_ON		1
#define SMOOTH_OFF		0



//--- ClassDescriptor and class vars ---------------------------------

class GengonClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new GengonObject; }
	const TCHAR *	ClassName() { return GetString(IDS_AP_GENGON_CLASS); }
	SClass_ID		SuperClassID() { return GEOMOBJECT_CLASS_ID; }
	Class_ID		ClassID() { return GENGON_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_RB_EXTENDED);  }
	void			ResetClassParams(BOOL fileReset);
	};

static GengonClassDesc GengonDesc;

ClassDesc* GetGengonDesc() { return &GengonDesc; }

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// class variables for Gengon class.
IObjParam *GengonObject::ip         = NULL;
int GengonObject::dlgHSegs			= DEF_SEGMENTS;
int GengonObject::dlgFSegs			= 1;
int GengonObject::dlgSSegs			= 1;
int GengonObject::dlgSides          = DEF_SIDES;
int GengonObject::dlgCreateMeth     = 1; // create_radius
IParamMap *GengonObject::pmapCreate = NULL;
IParamMap *GengonObject::pmapTypeIn = NULL;
IParamMap *GengonObject::pmapParam  = NULL;
Point3 GengonObject::crtPos         = Point3(0,0,0);
float GengonObject::crtRadius       = 0.0f;
float GengonObject::crtHeight       = 0.0f;
float GengonObject::crtFillet       = 0.0f;

void GengonClassDesc::ResetClassParams(BOOL fileReset)
	{ GengonObject::dlgHSegs			= DEF_SEGMENTS;
	  GengonObject::dlgFSegs			= 1;
	  GengonObject::dlgSSegs			= 1;
	  GengonObject::dlgSides          = DEF_SIDES;
	  GengonObject::dlgCreateMeth     = 1; // create_radius
	  GengonObject::crtRadius       = 0.0f;
	  GengonObject::crtHeight       = 0.0f;
	  GengonObject::crtFillet       = 0.0f;
	  GengonObject::crtPos         = Point3(0,0,0);
	}

//--- Parameter map/block descriptors -------------------------------

// Parameter block indices
#define PB_SIDES		0
#define PB_RADIUS		1
#define PB_FILLET		2
#define PB_HEIGHT		3
#define PB_SSEGS		4
#define PB_FSEGS		5
#define PB_HSEGS		6
#define PB_GENUVS		7
#define PB_SMOOTH		8

// Non-parameter block indices
#define PB_CREATEMETHOD		0
#define PB_TI_POS			1
#define PB_TI_SIDES			2
#define PB_TI_RADIUS		3
#define PB_TI_FILLET		4
#define PB_TI_HEIGHT		5

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
static ParamUIDesc descTypeIn[] = {
	
	// Position
	ParamUIDesc(
		PB_TI_POS,
		EDITTYPE_UNIVERSE,
		IDC_GG_POSX,IDC_GG_POSXSPIN,
		IDC_GG_POSY,IDC_GG_POSYSPIN,
		IDC_GG_POSZ,IDC_GG_POSZSPIN,
		float(-1.0E30),float(1.0E30),
		SPIN_AUTOSCALE),
	
	// Sides
	ParamUIDesc(
		PB_TI_SIDES,
		EDITTYPE_INT,
		IDC_GG_SIDES,IDC_GG_SIDESSPIN,
		float(MIN_SIDES),float(MAX_SIDES),
		SPIN_AUTOSCALE),
	
	// Radius
	ParamUIDesc(
		PB_TI_RADIUS,
		EDITTYPE_UNIVERSE,
		IDC_GG_RADIUS,IDC_GG_RADIUSSPIN,
		MIN_RADIUS,MAX_RADIUS,
		SPIN_AUTOSCALE),	

	// Fillet
	ParamUIDesc(
		PB_TI_FILLET,
		EDITTYPE_UNIVERSE,
		IDC_GG_FILLET,IDC_GG_FILLETSPIN,
		0.0f,BMAX_HEIGHT,
		SPIN_AUTOSCALE),
		
	// Height
	ParamUIDesc(
		PB_TI_HEIGHT,
		EDITTYPE_UNIVERSE,
		IDC_GG_HEIGHT,IDC_GG_HEIGHTSPIN,
		MIN_HEIGHT,BMAX_HEIGHT,
		SPIN_AUTOSCALE),
	
};

#define TYPEINDESC_LENGTH 5


//
//
// Parameters

static ParamUIDesc descParam[] = {
	// Sides
	ParamUIDesc(
		PB_SIDES,
		EDITTYPE_INT,
		IDC_GG_SIDES,IDC_GG_SIDESSPIN,
		float(MIN_SIDES),float(MAX_SIDES),
		SPIN_AUTOSCALE),
	
	// Radius
	ParamUIDesc(
		PB_RADIUS,
		EDITTYPE_UNIVERSE,
		IDC_GG_RADIUS,IDC_GG_RADIUSSPIN,
		MIN_RADIUS,MAX_RADIUS,
		SPIN_AUTOSCALE),	

	// Fillet
	ParamUIDesc(
		PB_FILLET,
		EDITTYPE_UNIVERSE,
		IDC_GG_FILLET,IDC_GG_FILLETSPIN,
		0.0f,BMAX_HEIGHT,
		SPIN_AUTOSCALE),
		
	// Height
	ParamUIDesc(
		PB_HEIGHT,
		EDITTYPE_UNIVERSE,
		IDC_GG_HEIGHT,IDC_GG_HEIGHTSPIN,
		MIN_HEIGHT,BMAX_HEIGHT,
		SPIN_AUTOSCALE),

	// Side Segments
	ParamUIDesc(
		PB_SSEGS,
		EDITTYPE_INT,
		IDC_GG_SIDESEGS,IDC_GG_SIDESEGSSPIN,
		(float)MIN_SEGMENTS,(float)MAX_SEGMENTS,
		0.1f),
	
	// Fillet Segments
	ParamUIDesc(
		PB_FSEGS,
		EDITTYPE_INT,
		IDC_GG_FILLETSEGS,IDC_GG_FILLETSEGSSPIN,
		0.0f,(float)MAX_SEGMENTS,
		0.1f),

	// Height Segments
	ParamUIDesc(
		PB_HSEGS,
		EDITTYPE_INT,
		IDC_GG_HGTSEGS,IDC_GG_HGTSEGSSPIN,
		(float)MIN_SEGMENTS,(float)MAX_SEGMENTS,
		0.1f),

	// Gen Smoothing
	ParamUIDesc(PB_SMOOTH,TYPE_SINGLECHEKBOX,IDC_OT_SMOOTH),
	
	// Gen UVs
	ParamUIDesc(PB_GENUVS,TYPE_SINGLECHEKBOX,IDC_GENTEXTURE),			
	};
#define PARAMDESC_LENGTH 9


// variable type, NULL, animatable, number
ParamBlockDescID gengondescVer0[] = {
	{ TYPE_INT, NULL, TRUE, 0 }, 
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 },
	{ TYPE_INT, NULL, TRUE, 4 }, 
	{ TYPE_INT, NULL, TRUE, 5 }, 
	{ TYPE_INT, NULL, TRUE, 6 }, 
	{ TYPE_INT, NULL, FALSE, 7 } 
	};

ParamBlockDescID gengondescVer1[] = {
	{ TYPE_INT, NULL, TRUE, 0 }, 
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 },
	{ TYPE_INT, NULL, TRUE, 4 }, 
	{ TYPE_INT, NULL, TRUE, 5 }, 
	{ TYPE_INT, NULL, TRUE, 6 }, 
	{ TYPE_INT, NULL, FALSE, 7 }, 
	{ TYPE_INT, NULL, FALSE, 8 } 
	};

#define PBLOCK_LENGTH	9

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(gengondescVer0,8,0),	
	};
#define NUM_OLDVERSIONS	1

#define CURRENT_VERSION	1
static ParamVersionDesc curVersion(gengondescVer1,PBLOCK_LENGTH,CURRENT_VERSION);

//--- TypeInDlgProc --------------------------------

void FixGenFilletLimits(IParamBlock *pblock,TimeValue t,HWND hWnd,BOOL increate)
{ float s,fillet,r;
  int sides;

	pblock->GetValue(PB_SIDES,t,sides,FOREVER);
	pblock->GetValue(PB_RADIUS,t,r,FOREVER);
	pblock->GetValue(PB_FILLET,(increate?0:t),fillet,FOREVER);
	s=(float)sqrt(2.0f*r*r*(1-(float)cos(TWOPI/sides)));
	s*=0.499f;
	if (hWnd)
	{ ISpinnerControl *spin2 = GetISpinner(GetDlgItem(hWnd,IDC_GG_FILLETSPIN));
	  spin2->SetLimits(0.0f,s,FALSE);
	  ReleaseISpinner(spin2);
	}
	if (fillet>s) pblock->SetValue(PB_FILLET,(increate?0:t),s);
}
class GengonCapValsDlgProc : public ParamMapUserDlgProc {
	public:
		GengonObject *ob;

		GengonCapValsDlgProc(GengonObject *o) {ob=o;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

BOOL GengonCapValsDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{ 	switch (msg) {
		case CC_SPINNER_CHANGE:
			switch ( LOWORD(wParam) ) {
				case IDC_GG_FILLETSPIN:
					FixGenFilletLimits(ob->pblock,t,hWnd,ob->increate);
			return TRUE;
				}
		}
	return FALSE;
	}

class GengonTypeInDlgProc : public ParamMapUserDlgProc {
	public:
		GengonObject *ob;

		GengonTypeInDlgProc(GengonObject *o) {ob=o;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

BOOL GengonTypeInDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_GG_CREATE: {
					if (ob->crtRadius==0.0f) return TRUE;
					
					// We only want to set the value if the object is 
					// not in the scene.
					if (ob->TestAFlag(A_OBJ_CREATING)) {
						ob->pblock->SetValue(PB_RADIUS,0,ob->crtRadius);
						ob->pblock->SetValue(PB_HEIGHT,0,ob->crtHeight);
						ob->pblock->SetValue(PB_SIDES,0,ob->dlgSides);
						ob->pblock->SetValue(PB_FILLET,0,ob->crtFillet);
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


//--- Gengon methods -------------------------------

GengonObject::GengonObject() 
	{
	MakeRefByID(FOREVER, 0, CreateParameterBlock(gengondescVer1,PBLOCK_LENGTH, CURRENT_VERSION));
	
	pblock->SetValue(PB_SIDES,0,dlgSides);
	pblock->SetValue(PB_RADIUS,0,crtRadius);
	pblock->SetValue(PB_FILLET,0,crtFillet);
	pblock->SetValue(PB_HEIGHT,0,crtHeight);
	pblock->SetValue(PB_SSEGS,0,dlgSSegs);
	pblock->SetValue(PB_FSEGS,0,dlgFSegs);
	pblock->SetValue(PB_HSEGS,0,dlgHSegs);
	pblock->SetValue(PB_GENUVS,0,TRUE);
	increate=FALSE;
	}

IOResult GengonObject::Load(ILoad *iload) 
	{
	iload->RegisterPostLoadCallback(
		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,0));
	return IO_OK;
	}


void GengonObject::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
	{
	SimpleObject::BeginEditParams(ip,flags,prev);
	this->ip = ip;

	if (pmapCreate && pmapParam) {
		
		// Left over from last Gengon ceated
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
				MAKEINTRESOURCE(IDD_UREVS1),
				GetString(IDS_RB_CREATE_DIALOG),
				0);

			pmapTypeIn = CreateCPParamMap(
				descTypeIn,TYPEINDESC_LENGTH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_GENGON2),
				GetString(IDS_RB_KEYBOARDENTRY),
				APPENDROLL_CLOSED);			
			}

		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_GENGON3),
			GetString(IDS_AP_PARAMETERS),
			0);
		}

	if(pmapTypeIn) {
		// A callback for the type in.
		pmapTypeIn->SetUserDlgProc(new GengonTypeInDlgProc(this));
		}	
	}
		
void GengonObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
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
	pblock->GetValue(PB_FSEGS,ip->GetTime(),dlgFSegs,FOREVER);
	pblock->GetValue(PB_SIDES,ip->GetTime(),dlgSides,FOREVER);	
	pblock->GetValue(PB_SSEGS,ip->GetTime(),dlgSSegs,FOREVER);	
	}

/*void GengonObject::SetParams(float rad, float height, int segs, int sides, int capsegs, BOOL smooth, 
	BOOL genUV, BOOL sliceOn, float slice1, float slice2) {
	pblock->SetValue(PB_RADIUS,0,radius);
	pblock->SetValue(PB_HEIGHT,0,height);
	pblock->SetValue(PB_FILLET,0,fillet);
	pblock->SetValue(PB_HSEGS,0,hsegs);
	pblock->SetValue(PB_FSEGS,0,fsegs);
	pblock->SetValue(PB_SIDES,0,sides);
	pblock->SetValue(PB_SSEGS,0,SSegs);
	pblock->SetValue(PB_SMOOTH,0,smooth);
	pblock->SetValue(PB_GENUVS,0,genUV);
	} */

/*void AddFace(Face *f,int a,int b,int c,int evis,int smooth_group)
{ f[0].setSmGroup(smooth_group);
  f[0].setMatID((MtlID)0); 	 //default 
  if (evis==0) f[0].setEdgeVisFlags(1,1,0);
  else if (evis==1) f[0].setEdgeVisFlags(0,1,1);
  else if (evis==2) f[0].setEdgeVisFlags(0,0,1);
  else f[0].setEdgeVisFlags(1,0,1);	
  f[0].setVerts(a,b,c);
}  */

void BuildGengonMesh(Mesh &mesh,
		int segs, int llsegs, int sidesegs, int fsegs,
		float radius1, float fillet, float height, int genUVs,int smooth)
	{
	Point3 p;
	BOOL minush=(height<0.0f);
	if (minush) height=-height;
	int ix,jx,ic = 1,ssegs=sidesegs-1;
	int nf=0,VertexPerLevel,totalsegs=segs+segs*(ssegs+fsegs);
	float delta,ang;	
	float startAng = 0.0f;	

	// Make pie2 < pie1 and pie1-pie2 < TWOPI
	int nfaces,ntverts;
	VertexPerLevel=totalsegs;
	nfaces=2*totalsegs*(llsegs+1);
	int nverts=VertexPerLevel*(llsegs+1)+2;

	mesh.setNumVerts(nverts);
	mesh.setNumFaces(nfaces);
	mesh.setSmoothFlags(smooth);
	if (genUVs) 
	{ ntverts=nverts+2*VertexPerLevel+llsegs+1;
	  mesh.setNumTVerts(ntverts);
	  mesh.setNumTVFaces(nfaces);
	} 
	else 
	{ mesh.setNumTVerts(0);
	  mesh.setNumTVFaces(0);
	}
	
	delta = (float)2.0*PI/(float)segs;
	if (height<0) delta = -delta;
	mesh.setVert(0, Point3(0.0f,0.0f,height));
	mesh.setVert(nverts-1, Point3(0.0f,0.0f,0.0f));
	// fill in fillet 
	float alfa,theta=(segs-2)*PI/(float)segs,theta2=theta/2.0f,k;
	float stepangle,sang,x,y,l1,l2,ax,ay,cx;
	int i,nv=2;
	if (fsegs>0)
	{ alfa=PI-theta;
	  sang=stepangle=alfa/fsegs;
	  k=fillet*(float)sin(theta2);
	  l1=k/(float)tan(alfa/2.0f);
	  l2=fillet*(float)cos(theta2);
	  cx=radius1-l1-l2;
	  ax=l1;ay=-k;	
	  mesh.setVert(1,Point3(cx+ax,ay,height));
	  for (i=1;i<=fsegs;i++)
	  {	x=cx+(float)cos(sang)*ax-(float)sin(sang)*ay;
	    y=(float)sin(sang)*ax+(float)cos(sang)*ay;
		mesh.setVert(nv++,Point3(x,y,height));
		sang+=stepangle;
	  }
	} else mesh.setVert(1,Point3(radius1,0.0f,height));
	ang=delta;
	for (jx = 1; jx<segs; jx++) 
	{ nv+=ssegs;
	  for (ix=0; ix<=fsegs; ix++)
	  {	x=(float)cos(ang)*mesh.verts[ix+1].x-(float)sin(ang)*mesh.verts[ix+1].y;
	    y=(float)sin(ang)*mesh.verts[ix+1].x+(float)cos(ang)*mesh.verts[ix+1].y;
		mesh.setVert(nv++, Point3(x,y,height));
	  }
	  ang += delta;
	}
	// fill in side segments
	int startpt,cpt,sidept,endpt=(startpt=1+fsegs)+sidesegs,lasts=segs-1;
	float deltax,deltay;
	if (sidesegs>1)
	 for (i=1;i<=segs;i++)
	 { cpt=startpt+1;
	   deltax=(mesh.verts[endpt].x-mesh.verts[startpt].x)/sidesegs;
	   deltay=(mesh.verts[endpt].y-mesh.verts[startpt].y)/sidesegs;
	   for (sidept=1;sidept<sidesegs;sidept++)
	   { mesh.verts[cpt].z=height;
		 mesh.verts[cpt].x=mesh.verts[startpt].x+(float)sidept*deltax;
		 mesh.verts[cpt++].y=mesh.verts[startpt].y+(float)sidept*deltay;
	   }
	   endpt=(startpt=cpt+fsegs)+sidesegs;
	   if (i==lasts) endpt-=totalsegs;
	 }
	else 
	{  deltax=mesh.verts[2].x-mesh.verts[1].x;
	   deltay=mesh.verts[2].y-mesh.verts[1].y;
	}
	//top layer done, now reflect sides down 
	int sidevs,startv=1,deltav;
	float sincr=height/llsegs;
	for (sidevs=0;sidevs<VertexPerLevel;sidevs++)
	{ p=mesh.verts[startv];
	  deltav=VertexPerLevel;
	  for (ic=1;ic<=llsegs;ic++)
	  { p.z =height-sincr*ic;
	    mesh.setVert(startv+deltav, p);
	    deltav+=VertexPerLevel;
	  }
	  startv++;
	}
	int tvcount;
	if (genUVs)
	{ float sidedist=(float)sqrt((deltax*deltax)+(deltay*deltay));
	  float sdenom=(fillet+sidedist)*segs,udenom=2.0f*radius1;
	  float fdist=(fsegs>0?(fillet/fsegs)/sdenom:0.0f),sdist=(sidedist/sidesegs)/sdenom;
	  tvcount=0;
	  for (i=0;i<=VertexPerLevel;i++)
	  { mesh.setTVert(tvcount++,(radius1+mesh.verts[i].x)/udenom,(radius1+mesh.verts[i].y)/udenom,0.0f);
	  }
	  int iseg,hcount=0,fcount;
	  float hlevel,u;
	  for (i=0;i<=llsegs;i++)
	  { hlevel=1.0f-hcount++/(float)llsegs;
		mesh.setTVert(tvcount++,u=0.0f,hlevel,0.0f);
		for (iseg=0;iseg<segs;iseg++)
		{ for (fcount=0;fcount<fsegs;fcount++)
		   mesh.setTVert(tvcount++,u+=fdist,hlevel,0.0f);
		  for (fcount=0;fcount<sidesegs;fcount++)
		   mesh.setTVert(tvcount++,u+=sdist,hlevel,0.0f);
		}
		mesh.tVerts[tvcount-1].x=1.0f;
	  }
	  i=nverts-VertexPerLevel-1;
	  while (tvcount<ntverts)
	  { mesh.setTVert(tvcount++,(radius1+mesh.verts[i].x)/udenom,(radius1+mesh.verts[i].y)/udenom,0.0f);
	    i++;
	  }
	}	
	int lvert=segs;
    int fc=0,sidesm=(smooth?2:0);
	int last=VertexPerLevel-1;
	BOOL evengr=TRUE;
	// Now make faces ---
	int j,b0=1,b1=2,tb0,tb1,tt0,tt1,t0,t1,ecount=0;
	for (i=0;i<VertexPerLevel;i++)
	{ if (genUVs) mesh.tvFace[fc].setTVerts(0,b0,(i<last?b1:1));
	AddFace(&mesh.faces[fc++],0,b0++,(i<last?b1++:1),0,smooth);
	}
	tt1=(tt0=i+1)+1;t0=1;t1=2;b1=(b0=t0+VertexPerLevel)+1;
	tb1=(tb0=tt1+VertexPerLevel)+1;
	for (i=1;i<=llsegs;i++)
	{ ecount=0;
	  for(j=0;j<VertexPerLevel;j++)
	  { if (genUVs) 
		{ mesh.tvFace[fc].setTVerts(tt0,tb0++,tb1);
		  mesh.tvFace[fc+1].setTVerts(tt0++,tb1++,tt1++);
		}
		if (fsegs==0) 
		{ if (sidesm)
			{sidesm=(evengr?2:4);
		     if ((j==last)&&(evengr)) sidesm=8;
			}
		  ecount++;if (ecount==sidesegs) {ecount=0;evengr=!evengr;}
	    }
	AddFace(&mesh.faces[fc++],t0,b0++,(j==last?t1:b1),0,sidesm);
	if (j<last)
	  AddFace(&mesh.faces[fc++],t0++,b1,t1,1,sidesm);
	else
	  AddFace(&mesh.faces[fc++],t0++,b1-VertexPerLevel,t1-VertexPerLevel,1,sidesm);
	t1++;b1++;
	  }
	 tt0++;tt1++;tb0++;tb1++;
	}
	if (genUVs) {tt0=(tt1=ntverts-VertexPerLevel)-1;tb0=ntverts-1;}
	for (i=0;i<VertexPerLevel;i++)
	{ if (genUVs)
	   { mesh.tvFace[fc].setTVerts(tt0++,tb0,(i==last?tt1-VertexPerLevel:tt1));
	     tt1++;
	   }
	AddFace(&mesh.faces[fc++],t0++,b0,(i==last?t1-VertexPerLevel:t1),1,smooth);
	  t1++;
	}
	if (minush)
	for (i=0;i<nverts;i++) mesh.verts[i].z-=height;
	assert(fc==mesh.numFaces);
//	assert(nv==mesh.numVerts);
	mesh.InvalidateTopologyCache();
	}

BOOL GengonObject::HasUVW() { 
	BOOL genUVs;
	Interval v;
	pblock->GetValue(PB_GENUVS, 0, genUVs, v);
	return genUVs; 
	}

void GengonObject::SetGenUVW(BOOL sw) {  
	if (sw==HasUVW()) return;
	pblock->SetValue(PB_GENUVS,0, sw);				
	}

void GengonObject::BuildMesh(TimeValue t)
	{	
	int segs, smooth, hsegs, sidesegs,fsegs;
	float radius,height,fillet;
	int genUVs;	

	// Start the validity interval at forever and widdle it down.
    FixGenFilletLimits(pblock,t,(pmapParam?pmapParam->GetHWnd():NULL),increate);
	ivalid = FOREVER;
	
	pblock->GetValue(PB_FSEGS,t,fsegs,ivalid);
	pblock->GetValue(PB_SIDES,t,segs,ivalid);
	pblock->GetValue(PB_HSEGS,t,hsegs,ivalid);
	pblock->GetValue(PB_SSEGS,t,sidesegs,ivalid);
	pblock->GetValue(PB_RADIUS,t,radius,ivalid);
	pblock->GetValue(PB_HEIGHT,t,height,ivalid);
	pblock->GetValue(PB_FILLET,t,fillet,ivalid);
	pblock->GetValue(PB_GENUVS,t,genUVs,ivalid);	
	pblock->GetValue(PB_SMOOTH,t,smooth,ivalid);	
	LimitValue(radius, MIN_RADIUS, MAX_RADIUS);
	LimitValue(height, MIN_HEIGHT, MAX_HEIGHT);
	LimitValue(fsegs, 0, MAX_SEGMENTS);
	LimitValue(hsegs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(sidesegs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(segs, MIN_SIDES, MAX_SIDES);
	LimitValue(smooth, 0, 1);	
	if (fillet==0.0f) fsegs=0;
	
	BuildGengonMesh(mesh,
		segs, hsegs, sidesegs, fsegs,
		radius, fillet, height, genUVs,smooth);
	}

inline Point3 operator+(const PatchVert &pv,const Point3 &p)
	{
	return p+pv.p;
	}

#define CIRCLE_VECTOR_LENGTH 0.5517861843f


Object* GengonObject::ConvertToType(TimeValue t, Class_ID obtype)
	{
		return SimpleObject::ConvertToType(t,obtype);
	}

int GengonObject::CanConvertToType(Class_ID obtype)
	{
	if (obtype==triObjectClassID) {
		return 1;
	} else {
		return SimpleObject::CanConvertToType(obtype);
		}
	}

class GengonObjCreateCallBack: public CreateMouseCallBack {
	GengonObject *ob;	
	Point3 p[2];
	IPoint2 sp0,sp1,sp2;	
	float r,s;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		void SetObj(GengonObject *obj) { ob = obj; }
	};

int GengonObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {

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
				ob->pblock->SetValue(PB_RADIUS,0,0.01f);
				ob->pblock->SetValue(PB_HEIGHT,0,0.01f);
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
				ob->pmapParam->Invalidate();

				if (flags&MOUSE_CTRL) {
					float ang = (float)atan2(p[1].y-p[0].y,p[1].x-p[0].x);
					mat.PreRotateZ(ob->ip->SnapAngle(ang));
					}

				if (msg==MOUSE_POINT) 
				  { if (Length(m-sp0)<3 ||Length(p[1]-p[0])<0.1f)
					{  ob->increate=FALSE;
					return CREATE_ABORT;	}
				  }
				break;
			case 2:
				{sp2=m;
#ifdef _OSNAP
				float h = vpt->SnapLength(vpt->GetCPDisp(p[1],Point3(0,0,1),sp1,m,TRUE));
#else
				float h = vpt->SnapLength(vpt->GetCPDisp(p[1],Point3(0,0,1),sp1,m));
#endif
				ob->pblock->SetValue(PB_HEIGHT,0,h);
				ob->pmapParam->Invalidate();
				if (msg==MOUSE_POINT)
				{ if (Length(m-sp0)<3) 
				{ ob->increate=FALSE;return CREATE_ABORT;}
				  else 
				  { int sides;
					ob->pblock->GetValue(PB_SIDES,0,sides,FOREVER);
					s=(float)sqrt(2.0f*r*r*(1-(float)cos(TWOPI/sides)));
					s*=0.5f;
				  }
				}
				}
				break;
			case 3:
				{
				float fillet = vpt->SnapLength(vpt->GetCPDisp(p[1],Point3(0,1,0),sp2,m));
				if (fillet<0.0f) fillet=0.0f;
				if (fillet>s) fillet=s;
				ob->pblock->SetValue(PB_FILLET,0,fillet);
				ob->pmapParam->Invalidate();
				if (msg==MOUSE_POINT) {					
					ob->suspendSnap = FALSE;
					ob->increate=FALSE;
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

static GengonObjCreateCallBack cylCreateCB;

CreateMouseCallBack* GengonObject::GetCreateMouseCallBack() 
	{
	cylCreateCB.SetObj(this);
	return(&cylCreateCB);
	}

BOOL GengonObject::OKtoDisplay(TimeValue t) 
	{
	float radius;
	pblock->GetValue(PB_RADIUS,t,radius,FOREVER);
	if (radius==0.0f) return FALSE;
	else return TRUE;
	}


// From ParamArray
BOOL GengonObject::SetValue(int i, TimeValue t, int v) 
	{
	switch (i) {
		case PB_CREATEMETHOD: dlgCreateMeth = v; break;
		case PB_TI_SIDES: dlgSides = v; break;
		}		
	return TRUE;
	}

BOOL GengonObject::SetValue(int i, TimeValue t, float v)
	{
	switch (i) {				
		case PB_TI_RADIUS: crtRadius = v; break;
		case PB_TI_HEIGHT: crtHeight = v; break;
		case PB_TI_FILLET: crtFillet = v; break;
		}	
	return TRUE;
	}

BOOL GengonObject::SetValue(int i, TimeValue t, Point3 &v) 
	{
	switch (i) {
		case PB_TI_POS: crtPos = v; break;
		}		
	return TRUE;
	}

BOOL GengonObject::GetValue(int i, TimeValue t, int &v, Interval &ivalid) 
	{
	switch (i) {
		case PB_CREATEMETHOD: v = dlgCreateMeth; break;
		case PB_TI_SIDES: v = dlgSides; break;
		}
	return TRUE;
	}

BOOL GengonObject::GetValue(int i, TimeValue t, float &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_RADIUS: v = crtRadius; break;
		case PB_TI_HEIGHT: v = crtHeight; break;
		case PB_TI_FILLET: v = crtFillet; break;
		}
	return TRUE;
	}

BOOL GengonObject::GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_POS: v = crtPos; break;		
		}
	return TRUE;
	}


void GengonObject::InvalidateUI() 
	{
	if (pmapParam) pmapParam->Invalidate();
	}

ParamDimension *GengonObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_SIDES:return stdSegmentsDim;
		case PB_RADIUS: return stdWorldDim;
		case PB_FILLET: return stdWorldDim;
		case PB_HEIGHT:return stdWorldDim;
		case PB_SSEGS: return stdSegmentsDim;
		case PB_FSEGS: return stdSegmentsDim;
		case PB_HSEGS: return stdSegmentsDim;
		default: return defaultDim;
		}
	}

TSTR GengonObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_SIDES: return TSTR(GetString(IDS_RB_SIDES));
		case PB_RADIUS: return TSTR(GetString(IDS_RB_RADIUS));
		case PB_FILLET: return TSTR(GetString(IDS_RB_FILLET));
		case PB_HEIGHT: return TSTR(GetString(IDS_RB_HEIGHT));
		case PB_SSEGS:  return TSTR(GetString(IDS_RB_SSEGS));
		case PB_FSEGS:  return TSTR(GetString(IDS_RB_FSEGS));
		case PB_HSEGS:  return TSTR(GetString(IDS_RB_HSEGS));
		case PB_GENUVS:  return TSTR(GetString(IDS_MXS_GENUVS));
		case PB_SMOOTH:  return TSTR(GetString(IDS_MXS_SMOOTH));
		default: return TSTR(_T(""));		
		}
	}

RefTargetHandle GengonObject::Clone(RemapDir& remap) 
	{
	GengonObject* newob = new GengonObject();	
	newob->ReplaceReference(0,pblock->Clone(remap));	
	newob->ivalid.SetEmpty();	
	BaseClone(this, newob, remap);
	return(newob);
	}




