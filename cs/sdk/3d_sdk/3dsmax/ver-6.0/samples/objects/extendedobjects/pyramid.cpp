/**********************************************************************
 *<
	FILE:pyramid.cpp
	CREATED BY:  Audrey Peterson

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#include "solids.h"

#ifndef NO_OBJECT_STANDARD_PRIMITIVES

#include "iparamm.h"
#include "Simpobj.h"
#include "surf_api.h"

static Class_ID PYRAMID_CLASS_ID(0x76bf318a, 0x4bf37b10);
class PyramidObject : public SimpleObject, public IParamArray {
	public:
		// Class vars
		static IParamMap *pmapCreate;
		static IParamMap *pmapTypeIn;
		static IParamMap *pmapParam;		
		static IObjParam *ip;
		static float crtHeight,crtWidth,crtDepth;
		static int dlgHSegs, dlgDSegs, dlgWSegs;
		static int dlgCreateMeth;
		static Point3 crtPos;		
		
		PyramidObject();		

		// From Object
		int CanConvertToType(Class_ID obtype);
		Object* ConvertToType(TimeValue t, Class_ID obtype);
		void GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist);
				
		// From BaseObject
		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);		
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		TCHAR *GetObjectName() { return GetString(IDS_RB_PYRAMID); }
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);
				
		// Animatable methods		
		void DeleteThis() { delete this; }
		Class_ID ClassID() { return PYRAMID_CLASS_ID; }  		
				
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

class PyramidClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new PyramidObject; }
	const TCHAR *	ClassName() { return GetString(IDS_AP_PYRAMID_CLASS); }
	SClass_ID		SuperClassID() { return GEOMOBJECT_CLASS_ID; }
	Class_ID		ClassID() { return PYRAMID_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_AP_PRIMITIVES);  }
	void			ResetClassParams(BOOL fileReset);
	};

static PyramidClassDesc PyramidDesc;

ClassDesc* GetPyramidDesc() { return &PyramidDesc; }

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// class variables for Pyramid class.
IObjParam *PyramidObject::ip         = NULL;
int PyramidObject::dlgHSegs			= DEF_SEGMENTS;
int PyramidObject::dlgWSegs       = DEF_SIDES;
int PyramidObject::dlgDSegs       = DEF_SIDES;
int PyramidObject::dlgCreateMeth     = 0; // base_apex
IParamMap *PyramidObject::pmapCreate = NULL;
IParamMap *PyramidObject::pmapTypeIn = NULL;
IParamMap *PyramidObject::pmapParam  = NULL;
Point3 PyramidObject::crtPos         = Point3(0,0,0);
float PyramidObject::crtHeight       = 0.0f;
float PyramidObject::crtWidth       = 0.0f;
float PyramidObject::crtDepth       = 0.0f;

void PyramidClassDesc::ResetClassParams(BOOL fileReset)
	{ PyramidObject::dlgHSegs			= DEF_SEGMENTS;
	  PyramidObject::dlgWSegs     = DEF_SIDES;
	  PyramidObject::dlgDSegs     = DEF_SIDES;
	  PyramidObject::dlgCreateMeth     = 0; // base_apex
	  PyramidObject::crtHeight       = 0.0f;
	  PyramidObject::crtWidth       = 0.0f;
	  PyramidObject::crtDepth       = 0.0f;
	  PyramidObject::crtPos         = Point3(0,0,0);
	}

//--- Parameter map/block descriptors -------------------------------

// Parameter block indices
#define PB_WIDTH	0
#define PB_DEPTH	1
#define PB_HEIGHT	2
#define PB_WSEGS	3
#define PB_DSEGS	4
#define PB_HSEGS	5
#define PB_GENUVS	6

// Non-parameter block indices
#define PB_CREATEMETHOD		0
#define PB_TI_POS			1
#define PB_TI_WIDTH			2
#define PB_TI_DEPTH			3
#define PB_TI_HEIGHT		4

#define BMIN_HEIGHT		float(-1.0E30)
#define BMAX_HEIGHT		float(1.0E30)
#define BMIN_LENGTH		float(0.1)
#define BMAX_LENGTH		float(1.0E30)
#define BMIN_WIDTH		float(0)
#define BMAX_WIDTH		float(1.0E30)
//
//
//	Creation method

static int createMethIDs[] = {IDC_PYR_CREATEBASE,IDC_PYR_CREATECENTER};

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
		IDC_PYR_POSX,IDC_PYR_POSXSPIN,
		IDC_PYR_POSY,IDC_PYR_POSYSPIN,
		IDC_PYR_POSZ,IDC_PYR_POSZSPIN,
		float(-1.0E30),float(1.0E30),
		SPIN_AUTOSCALE),
	
	// Width
	ParamUIDesc(
		PB_TI_WIDTH,
		EDITTYPE_UNIVERSE,
		IDC_PYR_WIDTH,IDC_PYR_WIDTHSPIN,
		BMIN_WIDTH,BMAX_WIDTH,
		SPIN_AUTOSCALE),	

	// Depth
	ParamUIDesc(
		PB_TI_DEPTH,
		EDITTYPE_UNIVERSE,
		IDC_PYR_DEPTH,IDC_PYR_DEPTHSPIN,
		BMIN_LENGTH,BMAX_LENGTH,
		SPIN_AUTOSCALE),
	
	// Height
	ParamUIDesc(
		PB_TI_HEIGHT,
		EDITTYPE_UNIVERSE,
		IDC_PYR_HEIGHT,IDC_PYR_HEIGHTSPIN,
		BMIN_HEIGHT,BMAX_HEIGHT,
		SPIN_AUTOSCALE),	
	};
#define TYPEINDESC_LENGTH 4


//
//
// Parameters

static ParamUIDesc descParam[] = {
	// Width
	ParamUIDesc(
		PB_WIDTH,
		EDITTYPE_UNIVERSE,
		IDC_PYR_WIDTH,IDC_PYR_WIDTHSPIN,
		BMIN_WIDTH,BMAX_WIDTH,
		SPIN_AUTOSCALE),	

	// Depth
	ParamUIDesc(
		PB_DEPTH,
		EDITTYPE_UNIVERSE,
		IDC_PYR_DEPTH,IDC_PYR_DEPTHSPIN,
		BMIN_LENGTH,BMAX_LENGTH,
		SPIN_AUTOSCALE),
	
	// Height
	ParamUIDesc(
		PB_HEIGHT,
		EDITTYPE_UNIVERSE,
		IDC_PYR_HEIGHT,IDC_PYR_HEIGHTSPIN,
		BMIN_HEIGHT,BMAX_HEIGHT,
		SPIN_AUTOSCALE),	

	// Width Segments
	ParamUIDesc(
		PB_WSEGS,
		EDITTYPE_INT,
		IDC_PYR_WIDSEGS,IDC_PYR_WIDSEGSSPIN,
		(float)MIN_SEGMENTS,(float)MAX_SEGMENTS,
		0.1f),
	
	// Depth Segments
	ParamUIDesc(
		PB_DSEGS,
		EDITTYPE_INT,
		IDC_PYR_DEPSEGS,IDC_PYR_DEPSEGSSPIN,
		(float)MIN_SEGMENTS,(float)MAX_SEGMENTS,
		0.1f),
	
	// Height Segments
	ParamUIDesc(
		PB_HSEGS,
		EDITTYPE_INT,
		IDC_PYR_HGTSEGS,IDC_PYR_HGTSEGSSPIN,
		(float)MIN_SEGMENTS,(float)MAX_SEGMENTS,
		0.1f),

	// Gen UVs
	ParamUIDesc(PB_GENUVS,TYPE_SINGLECHEKBOX,IDC_GENTEXTURE),			
	};
#define PARAMDESC_LENGTH 7


// variable type, NULL, animatable, number
ParamBlockDescID PyramiddescVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 },
	{ TYPE_INT, NULL, TRUE, 3 }, 
	{ TYPE_INT, NULL, TRUE, 4 },
	{ TYPE_INT, NULL, TRUE, 5 }, 
	{ TYPE_INT, NULL, FALSE, 6 }, 
	};

#define PBLOCK_LENGTH	7

#define NUM_OLDVERSIONS	0

#define CURRENT_VERSION	0
static ParamVersionDesc curVersion(PyramiddescVer0,PBLOCK_LENGTH,CURRENT_VERSION);

//--- TypeInDlgProc --------------------------------

class PyramidTypeInDlgProc : public ParamMapUserDlgProc {
	public:
		PyramidObject *ob;

		PyramidTypeInDlgProc(PyramidObject *o) {ob=o;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

BOOL PyramidTypeInDlgProc::DlgProc(
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
						ob->pblock->SetValue(PB_WIDTH,0,ob->crtWidth);
						ob->pblock->SetValue(PB_DEPTH,0,ob->crtDepth);
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


//--- Pyramid methods -------------------------------

PyramidObject::PyramidObject() 
	{
	MakeRefByID(FOREVER, 0, CreateParameterBlock(PyramiddescVer0, PBLOCK_LENGTH, CURRENT_VERSION));
	
	pblock->SetValue(PB_WSEGS,0,dlgWSegs);
	pblock->SetValue(PB_DSEGS,0,dlgDSegs);	
	pblock->SetValue(PB_HSEGS,0,dlgHSegs);	

	pblock->SetValue(PB_WIDTH,0,crtWidth);
	pblock->SetValue(PB_DEPTH,0,crtDepth);
	pblock->SetValue(PB_HEIGHT,0,crtHeight);

	pblock->SetValue(PB_GENUVS,0,TRUE);
	}

IOResult PyramidObject::Load(ILoad *iload) 
	{
	return IO_OK;
	}


void PyramidObject::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
	{
	SimpleObject::BeginEditParams(ip,flags,prev);
	this->ip = ip;

	if (pmapCreate && pmapParam) {
		
		// Left over from last Pyramid ceated
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
				MAKEINTRESOURCE(IDD_PYRAMID1),
				GetString(IDS_RB_CREATE_DIALOG),
				0);

			pmapTypeIn = CreateCPParamMap(
				descTypeIn,TYPEINDESC_LENGTH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_PYRAMID2),
				GetString(IDS_RB_KEYBOARDENTRY),
				APPENDROLL_CLOSED);			
			}

		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_PYRAMID3),
			GetString(IDS_AP_PARAMETERS),
			0);
		}

	if(pmapTypeIn) {
		// A callback for the type in.
		pmapTypeIn->SetUserDlgProc(new PyramidTypeInDlgProc(this));
		}	
	}
		
void PyramidObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
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
	pblock->GetValue(PB_WSEGS,ip->GetTime(),dlgWSegs,FOREVER);
	pblock->GetValue(PB_DSEGS,ip->GetTime(),dlgDSegs,FOREVER);
	pblock->GetValue(PB_HSEGS,ip->GetTime(),dlgHSegs,FOREVER);	
	}

/*void PyramidObject::SetParams(float rad, float height, int segs, int sides, int capsegs, BOOL smooth, 
	pblock->SetValue(PB_WIDTH,0,width);
	pblock->SetValue(PB_DEPTH,0,depth);
	pblock->SetValue(PB_HEIGHT,0,height);
	pblock->SetValue(PB_WSEGS,0,wsegs);
	pblock->SetValue(PB_DSEGS,0,dsegs);
	pblock->SetValue(PB_HSEGS,0,hsegs);
	pblock->SetValue(PB_GENUVS,0, genUV);
*/

void BuildPyramidMesh(Mesh &mesh,
		int hsegs, int wsegs, int dsegs, float height, 
		float width, float depth, int genUVs)
{	int nf=0,ulevel=wsegs+dsegs+2,totalsegs=2*(wsegs+dsegs);
	int nfaces,ntverts;
	nfaces = 2*totalsegs*hsegs;
	int nverts = totalsegs*hsegs+2;

	mesh.setNumVerts(nverts);
	mesh.setNumFaces(nfaces);
	if (genUVs) 
	{	ntverts = ulevel*hsegs+2+totalsegs;
		mesh.setNumTVerts(ntverts);
		mesh.setNumTVFaces(nfaces);
	} 
	else 
	{	mesh.setNumTVerts(0);
		mesh.setNumTVFaces(0);
	}
	
	Point3 CenterPt=Point3(width/2.0f,depth/2.0f,height);
	mesh.setVert(0,0.0f,0.0f,height);
	mesh.setVert(nverts-1,0.0f,0.0f,0.0f);
	if (genUVs) 
	{	mesh.setTVert(0,0.5f,1.0f,0.0f);
		mesh.setTVert(ntverts-1,0.5f,0.5f,0.0f);
	}
	int i,lpt,nv=1,tnv=1,fc=0,t0,tot0=totalsegs+1,tt0;
	float hwid=width/2.0f,hdep=depth/2.0f;
	float lwidth=width/hsegs,wincr,dincr,ldepth=depth/hsegs,hpos;
	float dstart,wstart,wpos,dpos,v,hdelta=height/hsegs;
	t0=tt0=0;int visflag;
	for (i=1;i<=hsegs;i++)
	{	v = (hpos=height-hdelta*i)/height;
		wincr=(wstart=lwidth*i)/wsegs;
		wstart=wstart/-2.0f;
		dincr=(dstart=ldepth*i)/dsegs;
		dstart=dstart/-2.0f;
		mesh.setVert(nv++,wstart,dstart,hpos);
		wpos=wstart;
		visflag=(i==1?ALLF:0);
		if (genUVs) 
			mesh.setTVert(tnv++,(1.0f-(hwid-wstart)/width),v,0.0f);
		for (lpt=1;lpt<=wsegs;lpt++)
		{	mesh.setVert(nv,wpos+=wincr,dstart,hpos);
			if (genUVs) 
			{	tt0 = tnv-ulevel-1;
				mesh.tvFace[fc].setTVerts((i<2?0:tt0),tnv-1,tnv);
				if (i>1)		
					mesh.tvFace[fc+1].setTVerts(tt0,tnv,tt0+1);
				mesh.setTVert(tnv++,(1.0f-(hwid-wpos)/width),v,0.0f);
			}
			AddFace(&mesh.faces[fc++],(i>1?t0:0),nv-1,nv,visflag,4);
			if (i>1) 
			{	AddFace(&mesh.faces[fc++],t0,nv,t0+1,1,4);
				t0++;
			}
			nv++;
		}
		if (genUVs) 
			mesh.setTVert(tnv++,(1.0f-(hwid-dstart)/depth),v,0.0f);
		dpos=dstart;
		for (lpt=1;lpt<=dsegs;lpt++)
		{	mesh.setVert(nv,wpos,dpos+=dincr,hpos);
			if (genUVs) 
			{	tt0=tnv-ulevel-1;
				mesh.tvFace[fc].setTVerts((i<2?0:tt0),tnv-1,tnv);
				if (i>1)		
					mesh.tvFace[fc+1].setTVerts(tt0,tnv,tt0+1);
				mesh.setTVert(tnv++,(1.0f-(hwid-dpos)/depth),v,0.0f);
			}
			AddFace(&mesh.faces[fc++],(i>1?t0:0),nv-1,nv,visflag,2);
			if (i>1) 
			{	AddFace(&mesh.faces[fc++],t0,nv,t0+1,1,2);
				t0++;
			}
			nv++;
		}
		if (genUVs) 
		{	tnv++; 
			tt0=(tnv-=ulevel)-ulevel-1;
		}
		for (lpt=1;lpt<=wsegs;lpt++)
		{	mesh.setVert(nv,wpos-=wincr,dpos,hpos);
			if (genUVs) 
			{	mesh.tvFace[fc].setTVerts((i<2?0:tt0),tnv-1,tnv);
				if (i>1)		
					mesh.tvFace[fc+1].setTVerts(tt0,tnv,tt0+1);
				tnv++;
				tt0++;
			}
			AddFace(&mesh.faces[fc++],(i<2?0:t0),nv-1,nv,visflag,16);
			if (i>1) 
			{	AddFace(&mesh.faces[fc++],t0,nv,t0+1,1,16);
				t0++;
			}
			nv++;
		}
		if (genUVs)
		{	tnv++;
			tt0=tnv-ulevel-1;
		}
		for (lpt=1;lpt<=dsegs;lpt++)
		{	if (lpt<dsegs) 
				mesh.setVert(nv,wpos,dpos-=dincr,hpos);
			if (genUVs) 
			{	mesh.tvFace[fc].setTVerts((i<2?0:tt0),tnv-1,tnv);
				if (i>1)		
					mesh.tvFace[fc+1].setTVerts(tt0,tnv,tt0+1);
				tnv++;tt0++;
			}
			if (lpt==dsegs)
			{	AddFace(&mesh.faces[fc++],t0,nv-1,t0+1,visflag,32);
				if (i>1) 
				{	AddFace(&mesh.faces[fc++],t0,t0+1,t0-totalsegs+1,1,32);
					t0++;
				}
			}
			else
			{	AddFace(&mesh.faces[fc++],(i<2?0:t0),nv-1,nv,visflag,32);
				if (i>1) 
				{	AddFace(&mesh.faces[fc++],t0,nv,t0+1,1,32);
					t0++;
				}
			}
			if (lpt<dsegs) 
				nv++;
		}
		if (i==1) 
			t0=1;
	}
	// Now make faces ---
	int b0 = nverts-1,
		bt = ntverts-1;
	if (genUVs) 
	{	mesh.setTVert(tnv++,(1.0f-(hwid-mesh.verts[t0].x)/(float)width),(hdep-mesh.verts[t0].y)/(float)depth,0.0f);
	}
	t0++;
	for (i=1;i<=totalsegs;i++)
	{	if (genUVs) 
		{	mesh.tvFace[fc].setTVerts(tnv-1,bt,(i<totalsegs?tnv:tnv-totalsegs));
			mesh.setTVert(tnv++,(1.0f-(hwid-mesh.verts[t0].x)/(float)width),(hdep-mesh.verts[t0].y)/(float)depth,0.0f);
		}
		AddFace(&mesh.faces[fc++],t0-1,b0,(i<totalsegs?t0:t0-totalsegs),ALLF,8);
		t0++;
	}
	if (height<0)
	{	DWORD hold;
		int tedge0,tedge1,tedge2;
		for (i=0;i<fc;i++)
		{	tedge0=(mesh.faces[i].getEdgeVis(0)?EDGE_VIS:EDGE_INVIS);
			tedge1=(mesh.faces[i].getEdgeVis(1)?EDGE_VIS:EDGE_INVIS);
			tedge2=(mesh.faces[i].getEdgeVis(2)?EDGE_VIS:EDGE_INVIS);
			hold=mesh.faces[i].v[0];mesh.faces[i].v[0]=mesh.faces[i].v[2];mesh.faces[i].v[2]=hold;
			mesh.faces[i].setEdgeVisFlags(tedge1,tedge0,tedge2);
 			if (genUVs)
			{	hold=mesh.tvFace[i].t[0];mesh.tvFace[i].t[0]=mesh.tvFace[i].t[2];mesh.tvFace[i].t[2]=hold;
			}
		}
	}
	assert(fc==mesh.numFaces);
//	assert(nv==mesh.numVerts); 
	mesh.InvalidateTopologyCache();
}

BOOL PyramidObject::HasUVW() { 
	BOOL genUVs;
	Interval v;
	pblock->GetValue(PB_GENUVS, 0, genUVs, v);
	return genUVs; 
	}

void PyramidObject::SetGenUVW(BOOL sw) {  
	if (sw==HasUVW()) return;
	pblock->SetValue(PB_GENUVS,0, sw);				
	}

void PyramidObject::BuildMesh(TimeValue t)
	{	
	int hsegs,wsegs,dsegs;
	float height,width,depth;
	int genUVs;	

	// Start the validity interval at forever and widdle it down.
	ivalid = FOREVER;
	
	pblock->GetValue(PB_HSEGS,t,hsegs,ivalid);
	pblock->GetValue(PB_WSEGS,t,wsegs,ivalid);
	pblock->GetValue(PB_DSEGS,t,dsegs,ivalid);
	pblock->GetValue(PB_HEIGHT,t,height,ivalid);
	pblock->GetValue(PB_WIDTH,t,width,ivalid);
	pblock->GetValue(PB_DEPTH,t,depth,ivalid);
	pblock->GetValue(PB_GENUVS,t,genUVs,ivalid);	
	LimitValue(height, MIN_HEIGHT, MAX_HEIGHT);
	LimitValue(width, MIN_HEIGHT, MAX_HEIGHT);
	LimitValue(depth, MIN_HEIGHT, MAX_HEIGHT);
	LimitValue(hsegs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(wsegs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(dsegs, MIN_SEGMENTS, MAX_SEGMENTS);
	
	BuildPyramidMesh(mesh, hsegs, wsegs, dsegs, height, 
		width, depth, genUVs);
	}

inline Point3 operator+(const PatchVert &pv,const Point3 &p)
	{
	return p+pv.p;
	}



#ifndef NO_NURBS

Object*
BuildNURBSPyramid(float width, float depth, float height, int genUVs)
{
	int pyramid_faces[5][4] = { {0, 1, 2, 3}, // bottom
							{2, 3, 4, 4}, // back
							{1, 0, 4, 4}, // front
							{3, 1, 4, 4}, // left
							{0, 2, 4, 4}};// right
	Point3 pyramid_verts[5] = { Point3(-0.5, -0.5, 0.0),
							Point3( 0.5, -0.5, 0.0),
							Point3(-0.5,  0.5, 0.0),
							Point3( 0.5,  0.5, 0.0),
							Point3( 0.0,  0.0, 1.0)};

	NURBSSet nset;

	for (int face = 0; face < 5; face++) {
		Point3 bl = pyramid_verts[pyramid_faces[face][0]];
		Point3 br = pyramid_verts[pyramid_faces[face][1]];
		Point3 tl = pyramid_verts[pyramid_faces[face][2]];
		Point3 tr = pyramid_verts[pyramid_faces[face][3]];

		Matrix3 size;
		size.IdentityMatrix();
		Point3 lwh(width, depth, height);
		size.Scale(lwh);

		bl = bl * size;
		br = br * size;
		tl = tl * size;
		tr = tr * size;

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

		switch(face) {
		case 0: // bottom
			surf->SetTextureUVs(0, 0, Point2(1.0f, 0.0f));
			surf->SetTextureUVs(0, 1, Point2(0.0f, 0.0f));
			surf->SetTextureUVs(0, 2, Point2(1.0f, 1.0f));
			surf->SetTextureUVs(0, 3, Point2(0.0f, 1.0f));
			break;
		default: // sides
			surf->SetTextureUVs(0, 0, Point2(0.5f, 1.0f));
			surf->SetTextureUVs(0, 1, Point2(0.5f, 1.0f));
			surf->SetTextureUVs(0, 2, Point2(0.0f, 0.0f));
			surf->SetTextureUVs(0, 3, Point2(1.0f, 0.0f));
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
	// Fuse the degenerate peaks
	for (int i = 1; i < 5; i++) {
		for (int j = 1; j < 4; j++) {
			F(i, i, 0, 3, j, 3);
		}
	}

	// Fuse the peaks together
	F(1, 2, 0, 3, 0, 3);
	F(1, 3, 0, 3, 0, 3);
	F(1, 4, 0, 3, 0, 3);

	// Bottom(0) to Back (1)
	F(0, 1, 3, 3, 3, 0);
	F(0, 1, 2, 3, 2, 0);
	F(0, 1, 1, 3, 1, 0);
	F(0, 1, 0, 3, 0, 0);

	// Bottom(0) to Front (2)
	F(0, 2, 0, 0, 3, 0);
	F(0, 2, 1, 0, 2, 0);
	F(0, 2, 2, 0, 1, 0);
	F(0, 2, 3, 0, 0, 0);

	// Bottom(0) to Left (3)
	F(0, 3, 3, 0, 3, 0);
	F(0, 3, 3, 1, 2, 0);
	F(0, 3, 3, 2, 1, 0);
	F(0, 3, 3, 3, 0, 0);

	// Bottom(0) to Right (4)
	F(0, 4, 0, 0, 0, 0);
	F(0, 4, 0, 1, 1, 0);
	F(0, 4, 0, 2, 2, 0);
	F(0, 4, 0, 3, 3, 0);

	// Front (2)  to Right (4)
	F(2, 4, 3, 1, 0, 1);
	F(2, 4, 3, 2, 0, 2);

	// Right (4) to Back (1)
	F(4, 1, 3, 1, 0, 1);
	F(4, 1, 3, 2, 0, 2);

	// Back (1) to Left (3)
	F(1, 3, 3, 1, 0, 1);
	F(1, 3, 3, 2, 0, 2);

	// Left (3) to Front (2)
	F(3, 2, 3, 1, 0, 1);
	F(3, 2, 3, 2, 0, 2);

	Matrix3 mat;
	mat.IdentityMatrix();
	Object *obj = CreateNURBSObject(NULL, &nset, mat);
	return obj;
}

#endif

Object* PyramidObject::ConvertToType(TimeValue t, Class_ID obtype)
	{
#ifndef NO_NURBS
	if (obtype == EDITABLE_SURF_CLASS_ID) {
		Interval valid = FOREVER;
		float depth, width, height;
		int genUVs;
		pblock->GetValue(PB_WIDTH,t,width,valid);
		pblock->GetValue(PB_DEPTH,t,depth,valid);
		pblock->GetValue(PB_HEIGHT,t,height,valid);
		pblock->GetValue(PB_GENUVS,t,genUVs,valid);
		Object *ob = BuildNURBSPyramid(width, depth, height, genUVs);
		ob->SetChannelValidity(TOPO_CHAN_NUM,valid);
		ob->SetChannelValidity(GEOM_CHAN_NUM,valid);
		ob->UnlockObject();
		return ob;
	} 
#endif
    return SimpleObject::ConvertToType(t,obtype);
	}

int PyramidObject::CanConvertToType(Class_ID obtype)
	{
#ifndef NO_NURBS
	if (obtype == EDITABLE_SURF_CLASS_ID)
        return 1;
#endif
	if (obtype == triObjectClassID)
		return 1;

    return SimpleObject::CanConvertToType(obtype);
	}


void PyramidObject::GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist)
{
    Object::GetCollapseTypes(clist, nlist);
#ifndef NO_NURBS
    Class_ID id = EDITABLE_SURF_CLASS_ID;
    TSTR *name = new TSTR(GetString(IDS_SM_NURBS_SURFACE));
    clist.Append(1,&id);
    nlist.Append(1,&name);
#endif
}


class PyramidObjCreateCallBack: public CreateMouseCallBack {
	PyramidObject *ob;	
	Point3 p[2],d;
	IPoint2 sp0,sp1;
	BOOL square;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		void SetObj(PyramidObject *obj) { ob = obj; }
	};

int PyramidObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) 
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
				ob->suspendSnap = TRUE;				
				sp0 = m;				
				p[0] = vpt->SnapPoint(m,m,NULL,snapdim);
				p[1] = p[0] + Point3(.01,.01,.01);
				mat.SetTrans(float(.5)*(p[0]+p[1]));				
				ob->pblock->SetValue(PB_WIDTH,0,0.01f);
				ob->pblock->SetValue(PB_DEPTH,0,0.01f);
				ob->pblock->SetValue(PB_HEIGHT,0,0.01f);
				ob->pmapParam->Invalidate();				
				break;
			case 1: 
				sp1 = m;
				p[1] = vpt->SnapPoint(m,m,NULL,snapdim);
				p[1].z = p[0].z +(float).01; 
				if (ob->dlgCreateMeth || (flags&MOUSE_CTRL)) 
				{ mat.SetTrans(p[0]);	} 
				else mat.SetTrans(float(.5)*(p[0]+p[1]));
				d = p[1]-p[0];
				square = FALSE;
				if (flags&MOUSE_CTRL) {
					// Constrain to square base
					float len;
					if (fabs(d.x) > fabs(d.y)) len = d.x;
					else len = d.y;
					d.x = d.y = 2.0f * len;
					square = TRUE;
					}
				ob->pblock->SetValue(PB_WIDTH,0,(float)fabs(d.x));
				ob->pblock->SetValue(PB_DEPTH,0,(float)fabs(d.y));
				ob->pmapParam->Invalidate();				
				if (msg==MOUSE_POINT && (Length(sp1-sp0)<3 || Length(d)<0.1f)) 
				{ return CREATE_ABORT;	}
				break;
			case 2:
#ifdef _OSNAP
				p[1].z = vpt->SnapLength(vpt->GetCPDisp(p[0],Point3(0,0,1),sp1,m,TRUE));
#else
				p[1].z = vpt->SnapLength(vpt->GetCPDisp(p[1],Point3(0,0,1),sp1,m));
#endif
//				mat.SetTrans(2,(p[1].z>0?p[0].z:p[1].z));			
				ob->pblock->SetValue(PB_WIDTH,0,(float)fabs(d.x));
				ob->pblock->SetValue(PB_DEPTH,0,(float)fabs(d.y));
				ob->pblock->SetValue(PB_HEIGHT,0,p[1].z);
				ob->pmapParam->Invalidate();				
				if (msg==MOUSE_POINT) {					
					ob->suspendSnap = FALSE;
					return (Length(m-sp0)<3)?CREATE_ABORT:CREATE_STOP;
					}
				break;

			}
	} else {
		if (msg == MOUSE_ABORT)
			return CREATE_ABORT;
		}
	return 1;
	}

static PyramidObjCreateCallBack cylCreateCB;

CreateMouseCallBack* PyramidObject::GetCreateMouseCallBack() 
	{
	cylCreateCB.SetObj(this);
	return(&cylCreateCB);
	}

BOOL PyramidObject::OKtoDisplay(TimeValue t) 
	{
	float radius;
	pblock->GetValue(PB_WIDTH,t,radius,FOREVER);
	if (radius==0.0f) return FALSE;
	else return TRUE;
	}


// From ParamArray
BOOL PyramidObject::SetValue(int i, TimeValue t, int v) 
	{
	switch (i) {
		case PB_CREATEMETHOD: dlgCreateMeth = v; break;
		}		
	return TRUE;
	}

BOOL PyramidObject::SetValue(int i, TimeValue t, float v)
	{
	switch (i) {				
		case PB_TI_WIDTH: crtWidth = v; break;
		case PB_TI_DEPTH: crtDepth = v; break;
		case PB_TI_HEIGHT: crtHeight = v; break;
		}	
	return TRUE;
	}

BOOL PyramidObject::SetValue(int i, TimeValue t, Point3 &v) 
	{
	switch (i) {
		case PB_TI_POS: crtPos = v; break;
		}		
	return TRUE;
	}

BOOL PyramidObject::GetValue(int i, TimeValue t, int &v, Interval &ivalid) 
	{
	switch (i) {
		case PB_CREATEMETHOD: v = dlgCreateMeth; break;
		}
	return TRUE;
	}

BOOL PyramidObject::GetValue(int i, TimeValue t, float &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_WIDTH: v = crtWidth; break;
		case PB_TI_DEPTH: v = crtDepth; break;
		case PB_TI_HEIGHT: v = crtHeight; break;
		}
	return TRUE;
	}

BOOL PyramidObject::GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_POS: v = crtPos; break;		
		}
	return TRUE;
	}


void PyramidObject::InvalidateUI() 
	{
	if (pmapParam) pmapParam->Invalidate();
	}

ParamDimension *PyramidObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_WIDTH: return stdWorldDim;
		case PB_DEPTH:return stdWorldDim;
		case PB_HEIGHT:return stdWorldDim;
		case PB_WSEGS: return stdSegmentsDim;
		case PB_DSEGS: return stdSegmentsDim;
		case PB_HSEGS: return stdSegmentsDim;
		default: return defaultDim;
		}
	}

TSTR PyramidObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_WIDTH:  return TSTR(GetString(IDS_RB_WIDTH));
		case PB_DEPTH: return TSTR(GetString(IDS_RB_DEPTH));
		case PB_HEIGHT: return TSTR(GetString(IDS_RB_HEIGHT));
		case PB_WSEGS:  return TSTR(GetString(IDS_RB_WSEGS));
		case PB_DSEGS:  return TSTR(GetString(IDS_RB_DSEGS));
		case PB_HSEGS:  return TSTR(GetString(IDS_RB_HSEGS));
		case PB_GENUVS:  return TSTR(GetString(IDS_MXS_GENUVS));
		default: return TSTR(_T(""));		
		}
	}

RefTargetHandle PyramidObject::Clone(RemapDir& remap) 
	{
	PyramidObject* newob = new PyramidObject();	
	newob->ReplaceReference(0,pblock->Clone(remap));	
	newob->ivalid.SetEmpty();	
	BaseClone(this, newob, remap);
	return(newob);
	}

#endif // NO_OBJECT_STANDARD_PRIMITIVES

