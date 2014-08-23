/**********************************************************************
 *<
	FILE: boxobj.cpp

	DESCRIPTION:  A Box object implementation

	CREATED BY: Dan Silva
	MODIFIED BY: Rolf Berteig

	HISTORY: created 13 September 1994

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "prim.h"

#include "iparamm.h"
#include "Simpobj.h"
#include "surf_api.h"
#include "MNMath.h"
#include "PolyObj.h"

class BoxObject : public GenBoxObject, public IParamArray {
	private:
		bool mPolyBoxSmoothingGroupFix;

	public:
		// Class vars
		static IParamMap *pmapCreate;
		static IParamMap *pmapTypeIn;
		static IParamMap *pmapParam;		
		static IObjParam *ip;
		static int dlgLSegs;
		static int dlgWSegs;
		static int dlgHSegs;
		static int createMeth;
		static Point3 crtPos;		
		static float crtWidth, crtHeight, crtLength;

		BoxObject();
		
		// From Object
		int CanConvertToType(Class_ID obtype);
		Object* ConvertToType(TimeValue t, Class_ID obtype);
		void GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist);
		
		// From BaseObject
		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		TCHAR *GetObjectName() { return GetString(IDS_RB_BOX); }
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);

		// Animatable methods
		void DeleteThis() { delete this; }
		Class_ID ClassID() { return Class_ID( BOXOBJ_CLASS_ID, 0); }  
		
		// From ref
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		IOResult Save(ISave *isave);
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
		Object *BuildPolyBox (TimeValue t);
		BOOL OKtoDisplay(TimeValue t);
		void InvalidateUI();
		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);		

		// From GenBoxObject
		void SetParams(float width, float height, float length, int wsegs,int lsegs, 
			int hsegs, BOOL genUV); 
	};				



#define BOTTOMPIV

#define BMIN_LENGTH		float(0)
#define BMAX_LENGTH		float(1.0E30)
#define BMIN_WIDTH		float(0)
#define BMAX_WIDTH		float(1.0E30)
#define BMIN_HEIGHT		float(-1.0E30)
#define BMAX_HEIGHT		float(1.0E30)

#define BDEF_DIM		float(0)
#define BDEF_SEGS		1

#define MIN_SEGMENTS	1
#define MAX_SEGMENTS	200

//--- ClassDescriptor and class vars ---------------------------------

class BoxObjClassDesc:public ClassDesc {
	public:
// xavier robitaille | 03.02.15 | private boxes, spheres and cylinders 
#ifndef NO_OBJECT_STANDARD_PRIMITIVES
	int 			IsPublic() { return 1; }
#else
	int 			IsPublic() { return 0; }
#endif
	void *			Create(BOOL loading = FALSE) {return new BoxObject;}
	const TCHAR *	ClassName() { return GetString(IDS_RB_BOX_CLASS); }
	SClass_ID		SuperClassID() { return GEOMOBJECT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(BOXOBJ_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_PRIMITIVES);}	
	void			ResetClassParams(BOOL fileReset);
	};

static BoxObjClassDesc boxObjDesc;

ClassDesc* GetBoxobjDesc() { return &boxObjDesc; }

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// class variable for sphere class.
IObjParam *BoxObject::ip         = NULL;
int BoxObject::dlgLSegs          = BDEF_SEGS;
int BoxObject::dlgWSegs          = BDEF_SEGS;
int BoxObject::dlgHSegs          = BDEF_SEGS;
IParamMap *BoxObject::pmapCreate = NULL;
IParamMap *BoxObject::pmapTypeIn = NULL;
IParamMap *BoxObject::pmapParam  = NULL;	
Point3 BoxObject::crtPos         = Point3(0,0,0);		
float BoxObject::crtWidth        = 0.0f; 
float BoxObject::crtHeight       = 0.0f;
float BoxObject::crtLength       = 0.0f;
int BoxObject::createMeth        = 0;

void BoxObjClassDesc::ResetClassParams(BOOL fileReset)
	{
	BoxObject::dlgLSegs   = BDEF_SEGS;
	BoxObject::dlgWSegs   = BDEF_SEGS;
	BoxObject::dlgHSegs   = BDEF_SEGS;
	BoxObject::crtWidth   = 0.0f; 
	BoxObject::crtHeight  = 0.0f;
	BoxObject::crtLength  = 0.0f;
	BoxObject::createMeth = 0;
	BoxObject::crtPos     = Point3(0,0,0);
	}

//--- Parameter map/block descriptors -------------------------------

// Parameter block indices
#define PB_LENGTH	0
#define PB_WIDTH	1
#define PB_HEIGHT	2
#define PB_WSEGS	3
#define PB_LSEGS	4
#define PB_HSEGS	5
#define PB_GENUVS	6

// Non-parameter block indices
#define PB_CREATEMETHOD		0
#define PB_TI_POS			1
#define PB_TI_LENGTH		2
#define PB_TI_WIDTH			3
#define PB_TI_HEIGHT		4


//
//
//	Creation method

static int createMethIDs[] = {IDC_CREATEBOX,IDC_CREATECUBE};

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
		float(-1.0E30),float(1.0E30),
		SPIN_AUTOSCALE),
	
	// Length
	ParamUIDesc(
		PB_TI_LENGTH,
		EDITTYPE_UNIVERSE,
		IDC_LENGTHEDIT,IDC_LENSPINNER,
		BMIN_LENGTH,BMAX_LENGTH,
		SPIN_AUTOSCALE),
	
	// Width
	ParamUIDesc(
		PB_TI_WIDTH,
		EDITTYPE_UNIVERSE,
		IDC_WIDTHEDIT,IDC_WIDTHSPINNER,
		BMIN_WIDTH,BMAX_WIDTH,
		SPIN_AUTOSCALE),	

	// Height
	ParamUIDesc(
		PB_TI_HEIGHT,
		EDITTYPE_UNIVERSE,
		IDC_HEIGHTEDIT,IDC_HEIGHTSPINNER,
		BMIN_HEIGHT,BMAX_HEIGHT,
		SPIN_AUTOSCALE),	
	};
#define TYPEINDESC_LENGH 4


//
//
// Parameters

static ParamUIDesc descParam[] = {
	// Length
	ParamUIDesc(
		PB_LENGTH,
		EDITTYPE_UNIVERSE,
		IDC_LENGTHEDIT,IDC_LENSPINNER,
		BMIN_LENGTH,BMAX_LENGTH,
		SPIN_AUTOSCALE),
	
	// Width
	ParamUIDesc(
		PB_WIDTH,
		EDITTYPE_UNIVERSE,
		IDC_WIDTHEDIT,IDC_WIDTHSPINNER,
		BMIN_WIDTH,BMAX_WIDTH,
		SPIN_AUTOSCALE),	

	// Height
	ParamUIDesc(
		PB_HEIGHT,
		EDITTYPE_UNIVERSE,
		IDC_HEIGHTEDIT,IDC_HEIGHTSPINNER,
		BMIN_HEIGHT,BMAX_HEIGHT,
		SPIN_AUTOSCALE),	

	
	// Length Segments
	ParamUIDesc(
		PB_LSEGS,
		EDITTYPE_INT,
		IDC_LSEGS,IDC_LSEGSPIN,
		(float)MIN_SEGMENTS,(float)MAX_SEGMENTS,
		0.1f),
	
	// Width Segments
	ParamUIDesc(
		PB_WSEGS,
		EDITTYPE_INT,
		IDC_WSEGS,IDC_WSEGSPIN,
		(float)MIN_SEGMENTS,(float)MAX_SEGMENTS,
		0.1f),
	
	// Height Segments
	ParamUIDesc(
		PB_HSEGS,
		EDITTYPE_INT,
		IDC_HSEGS,IDC_HSEGSPIN,
		(float)MIN_SEGMENTS,(float)MAX_SEGMENTS,
		0.1f),
	
	// Gen UVs
	ParamUIDesc(PB_GENUVS,TYPE_SINGLECHEKBOX,IDC_GENTEXTURE),			
	};
#define PARAMDESC_LENGH 7


ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 }, 
	{ TYPE_INT, NULL, TRUE, 3 }, 
	{ TYPE_INT, NULL, TRUE, 4 }, 
	{ TYPE_INT, NULL, TRUE, 5 } 
	};

ParamBlockDescID descVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 }, 
	{ TYPE_INT, NULL, TRUE, 3 }, 
	{ TYPE_INT, NULL, TRUE, 4 }, 
	{ TYPE_INT, NULL, TRUE, 5 },
	{ TYPE_INT, NULL, FALSE, 6 } 
	};

#define PBLOCK_LENGTH	7

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,6,0),	
	};
#define NUM_OLDVERSIONS	1

#define CURRENT_VERSION	1
static ParamVersionDesc curVersion(descVer1,PBLOCK_LENGTH,CURRENT_VERSION);


//--- TypeInDlgProc --------------------------------

class BoxTypeInDlgProc : public ParamMapUserDlgProc {
	public:
		BoxObject *ob;

		BoxTypeInDlgProc(BoxObject *o) {ob=o;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

BOOL BoxTypeInDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case CC_SPINNER_CHANGE:
			switch (LOWORD(wParam)) {
				case IDC_LENSPINNER:
				case IDC_WIDTHSPINNER:
				case IDC_HEIGHTSPINNER:
					if (ob->createMeth) {
						ISpinnerControl *spin = (ISpinnerControl*)lParam;
						ob->crtLength = ob->crtWidth = ob->crtHeight =
							spin->GetFVal();
						map->Invalidate();
						}
					break;
				}
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_TI_CREATE: {					
					// We only want to set the value if the object is 
					// not in the scene.
					if (ob->TestAFlag(A_OBJ_CREATING)) {
						ob->pblock->SetValue(PB_LENGTH,0,ob->crtLength);
						ob->pblock->SetValue(PB_WIDTH,0,ob->crtWidth);
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


//--- Box methods -------------------------------


BoxObject::BoxObject() : mPolyBoxSmoothingGroupFix (true)
	{
	MakeRefByID(FOREVER, 0, CreateParameterBlock(descVer1, PBLOCK_LENGTH, CURRENT_VERSION));
	
	pblock->SetValue(PB_LSEGS,0,dlgLSegs);
	pblock->SetValue(PB_WSEGS,0,dlgWSegs);
	pblock->SetValue(PB_HSEGS,0,dlgHSegs);	
	pblock->SetValue(PB_LENGTH,0,crtLength);
	pblock->SetValue(PB_WIDTH,0,crtWidth);
	pblock->SetValue(PB_HEIGHT,0,crtHeight);

	pblock->SetValue(PB_GENUVS,0,TRUE);
	}

const kChunkPolyFix = 0x0100;

IOResult BoxObject::Save (ISave *isave)
	{
	ULONG nb;
	isave->BeginChunk (kChunkPolyFix);
	isave->Write (&mPolyBoxSmoothingGroupFix, sizeof(bool), &nb);
	isave->EndChunk ();
	return IO_OK;
	}

IOResult BoxObject::Load(ILoad *iload)
	{
	iload->RegisterPostLoadCallback(
		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,0));

	// For old Boxes with no kChunkPolyFix, the fix defaults to "off".
	mPolyBoxSmoothingGroupFix = false;

	ULONG nb;
	IOResult res = IO_OK;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
		case kChunkPolyFix:
			iload->Read (&mPolyBoxSmoothingGroupFix, sizeof(bool), &nb);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
	}
	return IO_OK;
	}

void BoxObject::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
	{
	SimpleObject::BeginEditParams(ip,flags,prev);
	this->ip = ip;

	if (pmapCreate && pmapParam) {
		
		// Left over from last Box ceated
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
				MAKEINTRESOURCE(IDD_BOXPARAM1),
				GetString(IDS_RB_CREATIONMETHOD),
				0);

			pmapTypeIn = CreateCPParamMap(
				descTypeIn,TYPEINDESC_LENGH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_BOXPARAM3),
				GetString(IDS_RB_KEYBOARDENTRY),
				APPENDROLL_CLOSED);			
			}

		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_BOXPARAM2),
			GetString(IDS_RB_PARAMETERS),
			0);
		}

	if(pmapTypeIn) {
		// A callback for the type in.
		pmapTypeIn->SetUserDlgProc(new BoxTypeInDlgProc(this));
		}
	}
		
void BoxObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
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
	pblock->GetValue(PB_LSEGS,ip->GetTime(),dlgLSegs,FOREVER);
	pblock->GetValue(PB_WSEGS,ip->GetTime(),dlgWSegs,FOREVER);
	pblock->GetValue(PB_HSEGS,ip->GetTime(),dlgHSegs,FOREVER);	
	}

void BoxObject::SetParams(float width, float height, float length, int wsegs,int lsegs, 
			int hsegs, BOOL genUV) {
	pblock->SetValue(PB_WIDTH,0,width);
	pblock->SetValue(PB_HEIGHT,0,height);
	pblock->SetValue(PB_LENGTH,0,length);
	pblock->SetValue(PB_LSEGS,0,lsegs);
	pblock->SetValue(PB_WSEGS,0,wsegs);
	pblock->SetValue(PB_HSEGS,0,hsegs);		
	pblock->SetValue(PB_GENUVS,0, genUV);	
	} 

// vertices ( a b c d ) are in counter clockwise order when viewd from 
// outside the surface unless bias!=0 in which case they are clockwise
static void MakeQuad(int nverts, Face *f, int a, int b , int c , int d, int sg, int bias) {
	int sm = 1<<sg;
	assert(a<nverts);
	assert(b<nverts);
	assert(c<nverts);
	assert(d<nverts);
	if (bias) {
		f[0].setVerts( b, a, c);
		f[0].setSmGroup(sm);
		f[0].setEdgeVisFlags(1,0,1);
		f[1].setVerts( d, c, a);
		f[1].setSmGroup(sm);
		f[1].setEdgeVisFlags(1,0,1);
	} else {
		f[0].setVerts( a, b, c);
		f[0].setSmGroup(sm);
		f[0].setEdgeVisFlags(1,1,0);
		f[1].setVerts( c, d, a);
		f[1].setSmGroup(sm);
		f[1].setEdgeVisFlags(1,1,0);
		}
	}


#define POSX 0	// right
#define POSY 1	// back
#define POSZ 2	// top
#define NEGX 3	// left
#define NEGY 4	// front
#define NEGZ 5	// bottom

int direction(Point3 *v) {
	Point3 a = v[0]-v[2];
	Point3 b = v[1]-v[0];
	Point3 n = CrossProd(a,b);
	switch(MaxComponent(n)) {
		case 0: return (n.x<0)?NEGX:POSX;
		case 1: return (n.y<0)?NEGY:POSY;
		case 2: return (n.z<0)?NEGZ:POSZ;
		}
	return 0;
	}

// Remap the sub-object material numbers so that the top face is the first one
// The order now is:
// Top / Bottom /  Left/ Right / Front / Back
static int mapDir[6] ={ 3, 5, 0, 2, 4, 1 };

#define MAKE_QUAD(na,nb,nc,nd,sm,b) {MakeQuad(nverts,&(mesh.faces[nf]),na, nb, nc, nd, sm, b);nf+=2;}

BOOL BoxObject::HasUVW() { 
	BOOL genUVs;
	Interval v;
	pblock->GetValue(PB_GENUVS, 0, genUVs, v);
	return genUVs; 
	}

void BoxObject::SetGenUVW(BOOL sw) {  
	if (sw==HasUVW()) return;
	pblock->SetValue(PB_GENUVS,0, sw);				
	}


void BoxObject::BuildMesh(TimeValue t)
	{
	int ix,iy,iz,nf,kv,mv,nlayer,topStart,midStart;
	int nverts,wsegs,lsegs,hsegs,nv,nextk,nextm,wsp1;
	int nfaces;
	Point3 va,vb,p;
	float l, w, h;
	int genUVs = 1;
	BOOL bias = 0;

	// Start the validity interval at forever and widdle it down.
	ivalid = FOREVER;	
	pblock->GetValue(PB_LENGTH,t,l,ivalid);
	pblock->GetValue(PB_WIDTH,t,w,ivalid);
	pblock->GetValue(PB_HEIGHT,t,h,ivalid);
	pblock->GetValue(PB_LSEGS,t,lsegs,ivalid);
	pblock->GetValue(PB_WSEGS,t,wsegs,ivalid);
	pblock->GetValue(PB_HSEGS,t,hsegs,ivalid);
	pblock->GetValue(PB_GENUVS,t,genUVs,ivalid);
	if (h<0.0f) bias = 1;
	
	LimitValue(lsegs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(wsegs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(hsegs, MIN_SEGMENTS, MAX_SEGMENTS);

	// Number of verts
      // bottom : (lsegs+1)*(wsegs+1)
	  // top    : (lsegs+1)*(wsegs+1)
	  // sides  : (2*lsegs+2*wsegs)*(hsegs-1)

	// Number of rectangular faces.
      // bottom : (lsegs)*(wsegs)
	  // top    : (lsegs)*(wsegs)
	  // sides  : 2*(hsegs*lsegs)+2*(wsegs*lsegs)

	wsp1 = wsegs + 1;
	nlayer  =  2*(lsegs+wsegs);
	topStart = (lsegs+1)*(wsegs+1);
	midStart = 2*topStart;

	nverts = midStart+nlayer*(hsegs-1);
	nfaces = 4*(lsegs*wsegs + hsegs*lsegs + wsegs*hsegs);

	mesh.setNumVerts(nverts);
	mesh.setNumFaces(nfaces);
	mesh.InvalidateTopologyCache();

	nv = 0;
	
	vb =  Point3(w,l,h)/float(2);   
	va = -vb;

#ifdef BOTTOMPIV
	va.z = float(0);
	vb.z = h;
#endif

	float dx = w/wsegs;
	float dy = l/lsegs;
	float dz = h/hsegs;

	// do bottom vertices.
	p.z = va.z;
	p.y = va.y;
	for(iy=0; iy<=lsegs; iy++) {
		p.x = va.x;
		for (ix=0; ix<=wsegs; ix++) {
			mesh.setVert(nv++, p);
			p.x += dx;
			}
		p.y += dy;
		}
	
	nf = 0;

	// do bottom faces.
	for(iy=0; iy<lsegs; iy++) {
		kv = iy*(wsegs+1);
		for (ix=0; ix<wsegs; ix++) {
			MAKE_QUAD(kv, kv+wsegs+1, kv+wsegs+2, kv+1, 1, bias);
			kv++;
			}
		}
	assert(nf==lsegs*wsegs*2);

	// do top vertices.
	p.z = vb.z;
	p.y = va.y;
	for(iy=0; iy<=lsegs; iy++) {
		p.x = va.x;
		for (ix=0; ix<=wsegs; ix++) {
			mesh.setVert(nv++, p);
			p.x += dx;
			}
		p.y += dy;
		}

	// do top faces (lsegs*wsegs);
	for(iy=0; iy<lsegs; iy++) {
		kv = iy*(wsegs+1)+topStart;
		for (ix=0; ix<wsegs; ix++) {
			MAKE_QUAD(kv, kv+1, kv+wsegs+2,kv+wsegs+1, 2, bias);
			kv++;
			}
		}
	assert(nf==lsegs*wsegs*4);

	// do middle vertices 
	for(iz=1; iz<hsegs; iz++) {
		
		p.z = va.z + dz * iz;

		// front edge
		p.x = va.x;  p.y = va.y;
		for (ix=0; ix<wsegs; ix++) { mesh.setVert(nv++, p);  p.x += dx;	}

		// right edge
		p.x = vb.x;	  p.y = va.y;
		for (iy=0; iy<lsegs; iy++) { mesh.setVert(nv++, p);  p.y += dy;	}

		// back edge
		p.x =  vb.x;  p.y =  vb.y;
		for (ix=0; ix<wsegs; ix++) { mesh.setVert(nv++, p);	 p.x -= dx;	}

		// left edge
		p.x = va.x;  p.y =  vb.y;
		for (iy=0; iy<lsegs; iy++) { mesh.setVert(nv++, p);	 p.y -= dy;	}
		}

	if (hsegs==1) {
		// do FRONT faces -----------------------
		kv = 0;
		mv = topStart;
		for (ix=0; ix<wsegs; ix++) {
			MAKE_QUAD(kv, kv+1, mv+1, mv, 3, bias);
			kv++;
			mv++;
			}

		// do RIGHT faces.-----------------------
		kv = wsegs;  
		mv = topStart + kv;
		for (iy=0; iy<lsegs; iy++) {
			MAKE_QUAD(kv, kv+wsp1, mv+wsp1, mv, 4, bias);
			kv += wsp1;
			mv += wsp1;
			}	

		// do BACK faces.-----------------------
		kv = topStart - 1;
		mv = midStart - 1;
		for (ix=0; ix<wsegs; ix++) {
			MAKE_QUAD(kv, kv-1, mv-1, mv, 5, bias);
			kv --;
			mv --;
			}

		// do LEFT faces.----------------------
		kv = lsegs*(wsegs+1);  // index into bottom
		mv = topStart + kv;
		for (iy=0; iy<lsegs; iy++) {
			MAKE_QUAD(kv, kv-wsp1, mv-wsp1, mv, 6, bias);
			kv -= wsp1;
			mv -= wsp1;
			}
		}

	else {
		// do front faces.
		kv = 0;
		mv = midStart;
		for(iz=0; iz<hsegs; iz++) {
			if (iz==hsegs-1) mv = topStart;
			for (ix=0; ix<wsegs; ix++) 
				MAKE_QUAD(kv+ix, kv+ix+1, mv+ix+1, mv+ix, 3, bias);
			kv = mv;
			mv += nlayer;
			}

		assert(nf==lsegs*wsegs*4 + wsegs*hsegs*2);
	 
		// do RIGHT faces.-------------------------
		// RIGHT bottom row:
		kv = wsegs; // into bottom layer. 
		mv = midStart + wsegs; // first layer of mid verts


		for (iy=0; iy<lsegs; iy++) {
			MAKE_QUAD(kv, kv+wsp1, mv+1, mv, 4, bias);
			kv += wsp1;
			mv ++;
			}

		// RIGHT middle part:
		kv = midStart + wsegs; 
		for(iz=0; iz<hsegs-2; iz++) {
			mv = kv + nlayer;
			for (iy=0; iy<lsegs; iy++) {
				MAKE_QUAD(kv+iy, kv+iy+1, mv+iy+1, mv+iy, 4, bias);
				}
			kv += nlayer;
			}

		// RIGHT top row:
		kv = midStart + wsegs + (hsegs-2)*nlayer; 
		mv = topStart + wsegs;
		for (iy=0; iy<lsegs; iy++) {
			MAKE_QUAD(kv, kv+1, mv+wsp1, mv, 4, bias);
			mv += wsp1;
			kv++;
			}
		
		assert(nf==lsegs*wsegs*4 + wsegs*hsegs*2 + lsegs*hsegs*2);

		// do BACK faces. ---------------------
		// BACK bottom row:
		kv = topStart - 1;
		mv = midStart + wsegs + lsegs;
		for (ix=0; ix<wsegs; ix++) {
			MAKE_QUAD(kv, kv-1, mv+1, mv, 5, bias);
			kv --;
			mv ++;
			}

		// BACK middle part:
		kv = midStart + wsegs + lsegs; 
		for(iz=0; iz<hsegs-2; iz++) {
			mv = kv + nlayer;
			for (ix=0; ix<wsegs; ix++) {
				MAKE_QUAD(kv+ix, kv+ix+1, mv+ix+1, mv+ix, 5, bias);
				}
			kv += nlayer;
			}

		// BACK top row:
		kv = midStart + wsegs + lsegs + (hsegs-2)*nlayer; 
		mv = topStart + lsegs*(wsegs+1)+wsegs;
		for (ix=0; ix<wsegs; ix++) {
			MAKE_QUAD(kv, kv+1, mv-1, mv, 5, bias);
			mv --;
			kv ++;
			}

		assert(nf==lsegs*wsegs*4 + wsegs*hsegs*4 + lsegs*hsegs*2);

		// do LEFT faces. -----------------
		// LEFT bottom row:
		kv = lsegs*(wsegs+1);  // index into bottom
		mv = midStart + 2*wsegs +lsegs;
		for (iy=0; iy<lsegs; iy++) {
			nextm = mv+1;
			if (iy==lsegs-1) 
				nextm -= nlayer;
			MAKE_QUAD(kv, kv-wsp1, nextm, mv, 6, bias);
			kv -=wsp1;
			mv ++;
			}

		// LEFT middle part:
		kv = midStart + 2*wsegs + lsegs; 
		for(iz=0; iz<hsegs-2; iz++) {
			mv = kv + nlayer;
			for (iy=0; iy<lsegs; iy++) {
				nextm = mv+1;
				nextk = kv+iy+1;
				if (iy==lsegs-1) { 
					nextm -= nlayer;
					nextk -= nlayer;
					}
				MAKE_QUAD(kv+iy, nextk, nextm, mv, 6, bias);
				mv++;
				}
			kv += nlayer;
			}

		// LEFT top row:
		kv = midStart + 2*wsegs + lsegs+ (hsegs-2)*nlayer; 
		mv = topStart + lsegs*(wsegs+1);
		for (iy=0; iy<lsegs; iy++) {
			nextk = kv+1;
			if (iy==lsegs-1) 
				nextk -= nlayer;
			MAKE_QUAD(kv, nextk, mv-wsp1, mv, 6, bias);
			mv -= wsp1;
			kv++;
			}
		}

	if (genUVs) {
		int ls = lsegs+1;
		int ws = wsegs+1;
		int hs = hsegs+1;
		int ntverts = ls*hs + hs*ws + ws*ls ;
		mesh.setNumTVerts( ntverts ) ;
		mesh.setNumTVFaces(nfaces);		

		int xbase = 0;
		int ybase = ls*hs;
		int zbase = ls*hs + hs*ws;
	
		float dw = 1.0f/float(wsegs);
		float dl = 1.0f/float(lsegs);
		float dh = 1.0f/float(hsegs);

		if (w==0.0f) w = .0001f;
		if (l==0.0f) l = .0001f;
		if (h==0.0f) h = .0001f;
		float u,v;

		nv = 0;
		v = 0.0f;
		// X axis face
		for (iz =0; iz<hs; iz++) {
			u = 0.0f; 
			for (iy =0; iy<ls; iy++) {
				mesh.setTVert(nv, u, v, 0.0f);
				nv++; u+=dl;
				}
			v += dh;
			}

		v = 0.0f; 
		//Y Axis face
		for (iz =0; iz<hs; iz++) {
			u = 0.0f;
			for (ix =0; ix<ws; ix++) {
				mesh.setTVert(nv, u, v, 0.0f);
				nv++; u+=dw;
				}
			v += dh;
			}

		v = 0.0f; 
		for (iy =0; iy<ls; iy++) {
			u = 0.0f; 
			for (ix =0; ix<ws; ix++) {
				mesh.setTVert(nv, u, v, 0.0f);
				nv++; u+=dw;
				}
			v += dl;
			}

		assert(nv==ntverts);

		for (nf = 0; nf<nfaces; nf++) {
			Face& f = mesh.faces[nf];
			DWORD* nv = f.getAllVerts();
			Point3 v[3];
			for (ix =0; ix<3; ix++)
				v[ix] = mesh.getVert(nv[ix]);
			int dir = direction(v);
			int ntv[3];
			for (ix=0; ix<3; ix++) {
				int iu,iv;
				switch(dir) {
					case POSX: case NEGX:
						iu = int(((float)lsegs*(v[ix].y-va.y)/l)+.5f); 
						iv = int(((float)hsegs*(v[ix].z-va.z)/h)+.5f);  
						if (dir==NEGX) iu = lsegs-iu;
						ntv[ix] = (xbase + iv*ls + iu);
						break;
					case POSY: case NEGY:
						iu = int(((float)wsegs*(v[ix].x-va.x)/w)+.5f);  
						iv = int(((float)hsegs*(v[ix].z-va.z)/h)+.5f); 
						if (dir==POSY) iu = wsegs-iu;
						ntv[ix] = (ybase + iv*ws + iu);
						break;
					case POSZ: case NEGZ:
						iu = int(((float)wsegs*(v[ix].x-va.x)/w)+.5f);  
						iv = int(((float)lsegs*(v[ix].y-va.y)/l)+.5f); 
						if (dir==NEGZ) iu = wsegs-iu;
						ntv[ix] = (zbase + iv*ws + iu);
						break;
					}
			 	}
			assert(ntv[0]<ntverts);
			assert(ntv[1]<ntverts);
			assert(ntv[2]<ntverts);
			
			mesh.tvFace[nf].setTVerts(ntv[0],ntv[1],ntv[2]);
			mesh.setFaceMtlIndex(nf,mapDir[dir]);
			}
		}
    else {
		mesh.setNumTVerts(0);
		mesh.setNumTVFaces(0);
		for (nf = 0; nf<nfaces; nf++) {
			Face& f = mesh.faces[nf];
			DWORD* nv = f.getAllVerts();
			Point3 v[3];
			for (int ix =0; ix<3; ix++)
				v[ix] = mesh.getVert(nv[ix]);
			int dir = direction(v);
			mesh.setFaceMtlIndex(nf,mapDir[dir]);
			}
		}
 
	mesh.InvalidateTopologyCache();
	}


#define Tang(vv,ii) ((vv)*3+(ii))
inline Point3 operator+(const PatchVert &pv,const Point3 &p)
	{
	return p+pv.p;
	}
inline Point3 operator-(const PatchVert &pv1,const PatchVert &pv2)
	{
	return pv1.p-pv2.p;
	}
inline Point3 operator+(const PatchVert &pv1,const PatchVert &pv2)
	{
	return pv1.p+pv2.p;
	}

void BuildBoxPatch(
		PatchMesh &patch, 
		float width, float length, float height, int textured)
	{
	int nverts = 8;
	int nvecs = 48;
	int npatches = 6;
	patch.setNumVerts(nverts);	
	patch.setNumTVerts(textured ? 4 : 0);
	patch.setNumVecs(nvecs);
	patch.setNumPatches(npatches);
	patch.setNumTVPatches(textured ? npatches : 0);

	float w2 = width/2.0f, w3 = width/3.0f;
	float l2 = length/2.0f, l3 = length/3.0f;
	float h2 = height/2.0f, h3 = height/3.0f;
	int i;
	Point3 v;
	DWORD a, b, c, d;

	patch.setVert(0, -w2, -l2, 0.0f);
	patch.setVert(1,  w2, -l2, 0.0f);
	patch.setVert(2,  w2,  l2, 0.0f);
	patch.setVert(3, -w2,  l2, 0.0f);
	patch.setVert(4, -w2, -l2, height);
	patch.setVert(5,  w2, -l2, height);
	patch.setVert(6,  w2,  l2, height);
	patch.setVert(7, -w2,  l2, height);
	
	if(textured) {
		patch.setTVert(0, UVVert(1,0,0));
		patch.setTVert(1, UVVert(1,1,0));
		patch.setTVert(2, UVVert(0,1,0));
		patch.setTVert(3, UVVert(0,0,0));
		}

	int ix=0;
	for (i=0; i<4; i++) {
		v = (patch.verts[(i+1)%4] - patch.verts[i])/3.0f;
		patch.setVec(ix++,patch.verts[i] + v);
		v = (patch.verts[i+4] - patch.verts[i])/3.0f;
		patch.setVec(ix++,patch.verts[i] + v);
		v = (patch.verts[i==0?3:i-1] - patch.verts[i])/3.0f;
		patch.setVec(ix++,patch.verts[i] + v);
		}
	for (i=0; i<4; i++) {
		v = (patch.verts[(i+1)%4+4] - patch.verts[i+4])/3.0f;
		patch.setVec(ix++,patch.verts[i+4] + v);
		v = (patch.verts[i] - patch.verts[i+4])/3.0f;
		patch.setVec(ix++,patch.verts[i+4] + v);
		v = (patch.verts[i==0?7:i+3] - patch.verts[i+4])/3.0f;
		patch.setVec(ix++,patch.verts[i+4] + v);
		}
	
	int px = 0;
	for (i=0; i<4; i++) {
		Patch &p = patch.patches[px];
		a = i+4;
		b = i;
		c = (i+1)%4;
		d = (i+1)%4+4;
		p.SetType(PATCH_QUAD);
		p.setVerts(a, b, c, d);
		p.setVecs(
			Tang(a,1),Tang(b,1),Tang(b,0),Tang(c,2),
			Tang(c,1),Tang(d,1),Tang(d,2),Tang(a,0));
		p.setInteriors(ix, ix+1, ix+2, ix+3);
		p.smGroup = 1<<px;
		if(textured)
			patch.getTVPatch(px).setTVerts(2,3,0,1);

		ix+=4;
		px++;
		}
	
	a = 0;
	b = 3;
	c = 2;
	d = 1;
	patch.patches[px].SetType(PATCH_QUAD);
	patch.patches[px].setVerts(a, b, c, d);
	patch.patches[px].setVecs(
		Tang(a,2),Tang(b,0),Tang(b,2),Tang(c,0),
		Tang(c,2),Tang(d,0),Tang(d,2),Tang(a,0));
	patch.patches[px].setInteriors(ix, ix+1, ix+2, ix+3);
	patch.patches[px].smGroup = 1<<px;
	if(textured)
		patch.getTVPatch(px).setTVerts(0,1,2,3);
//watje 3-17-99 to support patch matids
	patch.patches[px].setMatID(1);

	ix+=4;
	px++;

	a = 7;
	b = 4;
	c = 5;
	d = 6;
	patch.patches[px].SetType(PATCH_QUAD);
	patch.patches[px].setVerts(a, b, c, d);
	patch.patches[px].setVecs(
		Tang(a,0),Tang(b,2),Tang(b,0),Tang(c,2),
		Tang(c,0),Tang(d,2),Tang(d,0),Tang(a,2));
	patch.patches[px].setInteriors(ix, ix+1, ix+2, ix+3);
	patch.patches[px].smGroup = 1<<px;
	if(textured)
		patch.getTVPatch(px).setTVerts(2,3,0,1);
//watje 3-17-99 to support patch matids
	patch.patches[px].setMatID(0);

	patch.patches[0].setMatID(4);
	patch.patches[1].setMatID(3);
	patch.patches[2].setMatID(5);
	patch.patches[3].setMatID(2);

	ix+=4;
	px++;

	assert(patch.buildLinkages());
	patch.computeInteriors();
	patch.InvalidateGeomCache();
	}


#ifndef NO_NURBS

Object*
BuildNURBSBox(float width, float length, float height, int genUVs)
{
	int cube_faces[6][4] = {{0, 1, 2, 3}, // bottom
							{5, 4, 7, 6}, // top
							{2, 3, 6, 7}, // back
							{1, 0, 5, 4}, // front
							{3, 1, 7, 5}, // left
							{0, 2, 4, 6}};// right
	Point3 cube_verts[8] = {Point3(-0.5, -0.5, 0.0),
							Point3( 0.5, -0.5, 0.0),
							Point3(-0.5,  0.5, 0.0),
							Point3( 0.5,  0.5, 0.0),
							Point3(-0.5, -0.5, 1.0),
							Point3( 0.5, -0.5, 1.0),
							Point3(-0.5,  0.5, 1.0),
							Point3( 0.5,  0.5, 1.0)};
	int faceIDs[6] = { 2, 1, 6, 5, 4, 3 };

	NURBSSet nset;

	for (int face = 0; face < 6; face++) {
		Point3 bl = cube_verts[cube_faces[face][0]];
		Point3 br = cube_verts[cube_faces[face][1]];
		Point3 tl = cube_verts[cube_faces[face][2]];
		Point3 tr = cube_verts[cube_faces[face][3]];

		Matrix3 size;
		size.IdentityMatrix();
		Point3 lwh(width, length, height);
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
				char bname[40];
				char sname[40];
				strcpy(bname, GetString(IDS_RB_BOX));
				sprintf(sname, "%s%s%02dCV%02d", bname, GetString(IDS_CT_SURF), face, r * 4 + c);
				ncv.SetName(sname);
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

		surf->MatID(faceIDs[face]);

		surf->SetTextureUVs(0, 0, Point2(1.0f, 0.0f));
		surf->SetTextureUVs(0, 1, Point2(0.0f, 0.0f));
		surf->SetTextureUVs(0, 2, Point2(1.0f, 1.0f));
		surf->SetTextureUVs(0, 3, Point2(0.0f, 1.0f));

		char bname[40];
		char sname[40];
		strcpy(bname, GetString(IDS_RB_BOX));
		sprintf(sname, "%s%s%02d", bname, GetString(IDS_CT_SURF), face);
		surf->SetName(sname);
	}

#define NF(s1, s2, s1r, s1c, s2r, s2c) \
	fuse.mSurf1 = (s1); \
	fuse.mSurf2 = (s2); \
	fuse.mRow1 = (s1r); \
	fuse.mCol1 = (s1c); \
	fuse.mRow2 = (s2r); \
	fuse.mCol2 = (s2c); \
	nset.mSurfFuse.Append(1, &fuse);

	NURBSFuseSurfaceCV fuse;

	// Bottom(0) to Back (2)
	NF(0, 2, 3, 3, 3, 0);
	NF(0, 2, 2, 3, 2, 0);
	NF(0, 2, 1, 3, 1, 0);
	NF(0, 2, 0, 3, 0, 0);

	// Top(1) to Back (2)
	NF(1, 2, 0, 3, 3, 3);
	NF(1, 2, 1, 3, 2, 3);
	NF(1, 2, 2, 3, 1, 3);
	NF(1, 2, 3, 3, 0, 3);

	// Bottom(0) to Front (3)
	NF(0, 3, 0, 0, 3, 0);
	NF(0, 3, 1, 0, 2, 0);
	NF(0, 3, 2, 0, 1, 0);
	NF(0, 3, 3, 0, 0, 0);

	// Top(1) to Front (3)
	NF(1, 3, 3, 0, 3, 3);
	NF(1, 3, 2, 0, 2, 3);
	NF(1, 3, 1, 0, 1, 3);
	NF(1, 3, 0, 0, 0, 3);

	// Bottom(0) to Left (4)
	NF(0, 4, 3, 0, 3, 0);
	NF(0, 4, 3, 1, 2, 0);
	NF(0, 4, 3, 2, 1, 0);
	NF(0, 4, 3, 3, 0, 0);

	// Top(1) to Left (4)
	NF(1, 4, 0, 0, 3, 3);
	NF(1, 4, 0, 1, 2, 3);
	NF(1, 4, 0, 2, 1, 3);
	NF(1, 4, 0, 3, 0, 3);

	// Bottom(0) to Right (5)
	NF(0, 5, 0, 0, 0, 0);
	NF(0, 5, 0, 1, 1, 0);
	NF(0, 5, 0, 2, 2, 0);
	NF(0, 5, 0, 3, 3, 0);

	// Top(1) to Right (5)
	NF(1, 5, 3, 0, 0, 3);
	NF(1, 5, 3, 1, 1, 3);
	NF(1, 5, 3, 2, 2, 3);
	NF(1, 5, 3, 3, 3, 3);

	// Front (3)  to Right (5)
	NF(3, 5, 3, 1, 0, 1);
	NF(3, 5, 3, 2, 0, 2);

	// Right (5) to Back (2)
	NF(5, 2, 3, 1, 0, 1);
	NF(5, 2, 3, 2, 0, 2);

	// Back (2) to Left (4)
	NF(2, 4, 3, 1, 0, 1);
	NF(2, 4, 3, 2, 0, 2);

	// Left (4) to Front (3)
	NF(4, 3, 3, 1, 0, 1);
	NF(4, 3, 3, 2, 0, 2);


	Matrix3 mat;
	mat.IdentityMatrix();
	Object *obj = CreateNURBSObject(NULL, &nset, mat);
	return obj;
}

#endif



Object* BoxObject::ConvertToType(TimeValue t, Class_ID obtype)
{
#ifndef NO_PATCHES
	if (obtype == patchObjectClassID) {
		Interval valid = FOREVER;
		float length, width, height;
		int genUVs;
		pblock->GetValue(PB_LENGTH,t,length,valid);
		pblock->GetValue(PB_WIDTH,t,width,valid);
		pblock->GetValue(PB_HEIGHT,t,height,valid);
		pblock->GetValue(PB_GENUVS,t,genUVs,valid);
		PatchObject *ob = new PatchObject();
		BuildBoxPatch(ob->patch,width,length,height,genUVs);
		ob->SetChannelValidity(TOPO_CHAN_NUM,valid);
		ob->SetChannelValidity(GEOM_CHAN_NUM,valid);
		ob->UnlockObject();
		return ob;
	}
#endif
#ifndef NO_NURBS
	if (obtype == EDITABLE_SURF_CLASS_ID) {
		Interval valid = FOREVER;
		float length, width, height;
		int genUVs;
		pblock->GetValue(PB_LENGTH,t,length,valid);
		pblock->GetValue(PB_WIDTH,t,width,valid);
		pblock->GetValue(PB_HEIGHT,t,height,valid);
		pblock->GetValue(PB_GENUVS,t,genUVs,valid);
		Object *ob = BuildNURBSBox(width, length, height, genUVs);
		ob->SetChannelValidity(TOPO_CHAN_NUM,valid);
		ob->SetChannelValidity(GEOM_CHAN_NUM,valid);
		ob->UnlockObject();
		return ob;
	}
#endif

	if (obtype == polyObjectClassID) {
		Object *ob = BuildPolyBox (t);
		ob->UnlockObject ();
		return ob;
	}

#ifdef DESIGN_VER
	if (obtype == GENERIC_AMSOLID_CLASS_ID)
	{
		Interval valid = FOREVER;
		float length, width, height;
		pblock->GetValue(PB_LENGTH,t,length,valid);
		pblock->GetValue(PB_WIDTH,t,width,valid);
		pblock->GetValue(PB_HEIGHT,t,height,valid);	
		Point3 p(width, length, height);
		Object* solid = (Object*)CreateInstance(GEOMOBJECT_CLASS_ID, GENERIC_AMSOLID_CLASS_ID);
		assert(solid);
		if(solid)
		{
			IGeomImp* cacheptr = (IGeomImp*)(solid->GetInterface(I_GEOMIMP));
			assert(cacheptr);
			if(cacheptr)
			{
				bool res = cacheptr->createBox(p);
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

int BoxObject::CanConvertToType(Class_ID obtype)
	{
#ifndef NO_PATCHES
	if(obtype == patchObjectClassID) return 1;
#endif
#ifndef NO_NURBS
	if(obtype == EDITABLE_SURF_CLASS_ID) return 1;
#endif
#ifdef DESIGN_VER
	if(obtype == GENERIC_AMSOLID_CLASS_ID) return 1;
#endif
	if(obtype==polyObjectClassID) return 1;
	return SimpleObject::CanConvertToType(obtype);
	}

void BoxObject::GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist)
{
    Object::GetCollapseTypes(clist, nlist);

#ifndef NO_NURBS
    Class_ID id = EDITABLE_SURF_CLASS_ID;
    TSTR *name = new TSTR(GetString(IDS_SM_NURBS_SURFACE));
    clist.Append(1,&id);
    nlist.Append(1,&name);
#endif
}

class BoxObjCreateCallBack: public CreateMouseCallBack {
	BoxObject *ob;
	Point3 p0,p1;
	IPoint2 sp0, sp1;
	BOOL square;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		void SetObj(BoxObject *obj) { ob = obj; }
	};

int BoxObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
	Point3 d;
	if (msg == MOUSE_FREEMOVE)
	{
				vpt->SnapPreview(m,m,NULL, SNAP_IN_3D);
	}

	else if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {
			case 0:
				sp0 = m;
				ob->pblock->SetValue(PB_WIDTH,0,0.0f);
				ob->pblock->SetValue(PB_LENGTH,0,0.0f);
				ob->pblock->SetValue(PB_HEIGHT,0,0.0f);
				ob->suspendSnap = TRUE;								
				p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				p1 = p0 + Point3(.01,.01,.01);
				mat.SetTrans(float(.5)*(p0+p1));				
#ifdef BOTTOMPIV
				{
				Point3 xyz = mat.GetTrans();
				xyz.z = p0.z;
				mat.SetTrans(xyz);
				}
#endif
				break;
			case 1:
				sp1 = m;
				p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				p1.z = p0.z +(float).01; 
				if (ob->createMeth || (flags&MOUSE_CTRL)) {
					mat.SetTrans(p0);
				} else {
					mat.SetTrans(float(.5)*(p0+p1));
#ifdef BOTTOMPIV 					
					Point3 xyz = mat.GetTrans();
					xyz.z = p0.z;
					mat.SetTrans(xyz);					
					}
#endif
				d = p1-p0;
				
				square = FALSE;
				if (ob->createMeth) {
					// Constrain to cube
					d.x = d.y = d.z = Length(d)*2.0f;
				} else 
				if (flags&MOUSE_CTRL) {
					// Constrain to square base
					float len;
					if (fabs(d.x) > fabs(d.y)) len = d.x;
					else len = d.y;
					d.x = d.y = 2.0f * len;
					square = TRUE;
					}

				ob->pblock->SetValue(PB_WIDTH,0,float(fabs(d.x)));
				ob->pblock->SetValue(PB_LENGTH,0,float(fabs(d.y)));
				ob->pblock->SetValue(PB_HEIGHT,0,float(fabs(d.z)));
				ob->pmapParam->Invalidate();										

				if (msg==MOUSE_POINT && ob->createMeth) {
					ob->suspendSnap = FALSE;
					return (Length(sp1-sp0)<3)?CREATE_ABORT:CREATE_STOP;					
				} else if (msg==MOUSE_POINT && 
						(Length(sp1-sp0)<3 || Length(d)<0.1f)) {
					return CREATE_ABORT;
					}
				break;
			case 2:
#ifdef _OSNAP
				p1.z = p0.z + vpt->SnapLength(vpt->GetCPDisp(p0,Point3(0,0,1),sp1,m,TRUE));
#else
				p1.z = p0.z + vpt->SnapLength(vpt->GetCPDisp(p1,Point3(0,0,1),sp1,m));
#endif				
				if (!square) {
					mat.SetTrans(float(.5)*(p0+p1));
#ifdef BOTTOMPIV
					mat.SetTrans(2,p0.z); // set the Z component of translation
#endif
					}

				d = p1-p0;
				if (square) {
					// Constrain to square base
					float len;
					if (fabs(d.x) > fabs(d.y)) len = d.x;
					else len = d.y;
					d.x = d.y = 2.0f * len;					
					}

				ob->pblock->SetValue(PB_WIDTH,0,float(fabs(d.x)));
				ob->pblock->SetValue(PB_LENGTH,0,float(fabs(d.y)));
				ob->pblock->SetValue(PB_HEIGHT,0,float(d.z));
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

static BoxObjCreateCallBack boxCreateCB;

CreateMouseCallBack* BoxObject::GetCreateMouseCallBack() {
	boxCreateCB.SetObj(this);
	return(&boxCreateCB);
	}


BOOL BoxObject::OKtoDisplay(TimeValue t) 
	{
	/*
	float l, w, h;
	pblock->GetValue(PB_LENGTH,t,l,FOREVER);
	pblock->GetValue(PB_WIDTH,t,w,FOREVER);
	pblock->GetValue(PB_HEIGHT,t,h,FOREVER);
	if (l==0.0f || w==0.0f || h==0.0f) return FALSE;
	else return TRUE;
	*/
	return TRUE;
	}


// From ParamArray
BOOL BoxObject::SetValue(int i, TimeValue t, int v) 
	{
	switch (i) {
		case PB_CREATEMETHOD: createMeth = v; break;
		}		
	return TRUE;
	}

BOOL BoxObject::SetValue(int i, TimeValue t, float v)
	{
	switch (i) {				
		case PB_TI_LENGTH: crtLength = v; break;
		case PB_TI_WIDTH:  crtWidth = v; break;
		case PB_TI_HEIGHT: crtHeight = v; break;
		}	
	return TRUE;
	}

BOOL BoxObject::SetValue(int i, TimeValue t, Point3 &v) 
	{
	switch (i) {
		case PB_TI_POS: crtPos = v; break;
		}		
	return TRUE;
	}

BOOL BoxObject::GetValue(int i, TimeValue t, int &v, Interval &ivalid) 
	{
	switch (i) {
		case PB_CREATEMETHOD: v = createMeth; break;
		}
	return TRUE;
	}

BOOL BoxObject::GetValue(int i, TimeValue t, float &v, Interval &ivalid) 
	{	
	switch (i) {				
		case PB_TI_LENGTH: v = crtLength; break;
		case PB_TI_WIDTH:  v = crtWidth; break;
		case PB_TI_HEIGHT: v = crtHeight; break;
		}
	return TRUE;
	}

BOOL BoxObject::GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_POS: v = crtPos; break;		
		}
	return TRUE;
	}


void BoxObject::InvalidateUI() 
	{
	if (pmapParam) pmapParam->Invalidate();
	}

ParamDimension *BoxObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_LENGTH:return stdWorldDim;
		case PB_WIDTH: return stdWorldDim;
		case PB_HEIGHT:return stdWorldDim;
		case PB_WSEGS: return stdSegmentsDim;
		case PB_LSEGS: return stdSegmentsDim;
		case PB_HSEGS: return stdSegmentsDim;		
		default: return defaultDim;
		}
	}

TSTR BoxObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_LENGTH: return TSTR(GetString(IDS_RB_LENGTH));
		case PB_WIDTH:  return TSTR(GetString(IDS_RB_WIDTH));
		case PB_HEIGHT: return TSTR(GetString(IDS_RB_HEIGHT));
		case PB_WSEGS:  return TSTR(GetString(IDS_RB_WSEGS));
		case PB_LSEGS:  return TSTR(GetString(IDS_RB_LSEGS));
		case PB_HSEGS:  return TSTR(GetString(IDS_RB_HSEGS));
		default: return TSTR(_T(""));
		}
	}

RefTargetHandle BoxObject::Clone(RemapDir& remap) 
{
	BoxObject* newob = new BoxObject();
	newob->ReplaceReference(0,pblock->Clone(remap));
	newob->mPolyBoxSmoothingGroupFix = mPolyBoxSmoothingGroupFix;
	newob->ivalid.SetEmpty();	
	BaseClone(this, newob, remap);
	return(newob);
}

void MakePQuad (MNFace *mf, int v1, int v2, int v3, int v4, DWORD smG, MtlID mt, int bias) {
	int vv[4];
	vv[0] = v1;
	vv[1+bias] = v2;
	vv[2] = v3;
	vv[3-bias] = v4;
	mf->MakePoly (4, vv);
	mf->smGroup = smG;
	mf->material = mt;
}

void MakeMapQuad (MNMapFace *mf, int v1, int v2, int v3, int v4, int bias) {
	int vv[4];
	vv[0] = v1;
	vv[1+bias] = v2;
	vv[2] = v3;
	vv[3-bias] = v4;
	mf->MakePoly (4, vv);
}

// NOTE: these separate macros for different surfaces spell out the smoothing
// group and material ID for each surface.
#define MAKE_TOP_QUAD(v1,v2,v3,v4) MakePQuad(mm.F(nf),v1,v2,v3,v4,(1<<1),0,bias)
#define MAKE_BOTTOM_QUAD(v1,v2,v3,v4) MakePQuad(mm.F(nf),v1,v2,v3,v4,(1<<2),1,bias)
#define MAKE_LEFT_QUAD(v1,v2,v3,v4) MakePQuad(mm.F(nf),v1,v2,v3,v4,(1<<6),2,bias)
#define MAKE_RIGHT_QUAD(v1,v2,v3,v4) MakePQuad(mm.F(nf),v1,v2,v3,v4,(1<<4),3,bias)
#define MAKE_FRONT_QUAD(v1,v2,v3,v4) MakePQuad(mm.F(nf),v1,v2,v3,v4,(1<<3),4,bias)
#define MAKE_BACK_QUAD(v1,v2,v3,v4) MakePQuad(mm.F(nf),v1,v2,v3,v4,(1<<5),5,bias)
#define MAKE_MAP_QUAD(v1,v2,v3,v4) if(tf)MakeMapQuad(&(tf[nf]),v1,v2,v3,v4,bias);

// Fix for Max 5.1: PolyObjects had been coming in with the top & bottom smoothing groups reversed.
#define MAKE_TOP_QUAD_FIX(v1,v2,v3,v4) MakePQuad(mm.F(nf),v1,v2,v3,v4,(1<<2),0,bias)
#define MAKE_BOTTOM_QUAD_FIX(v1,v2,v3,v4) MakePQuad(mm.F(nf),v1,v2,v3,v4,(1<<1),1,bias)

Object *BoxObject::BuildPolyBox (TimeValue t) {
	PolyObject *pobj = new PolyObject();
	MNMesh & mm = pobj->mm;
	int wsegs, lsegs, hsegs;
	float l, w, h;
	int genUVs = 1;
	int bias = 0;

	// Start the validity interval at forever and widdle it down.
	Interval gValid, tValid;
	tValid.SetInfinite();
	pblock->GetValue(PB_LSEGS,t,lsegs,tValid);
	pblock->GetValue(PB_WSEGS,t,wsegs,tValid);
	pblock->GetValue(PB_HSEGS,t,hsegs,tValid);
	pblock->GetValue(PB_GENUVS,t,genUVs,tValid);
	LimitValue(lsegs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(wsegs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(hsegs, MIN_SEGMENTS, MAX_SEGMENTS);

	gValid = tValid;
	pblock->GetValue(PB_LENGTH,t,l,gValid);
	pblock->GetValue(PB_WIDTH,t,w,gValid);
	pblock->GetValue(PB_HEIGHT,t,h,gValid);
	if (h<0.0f) bias = 2;

	DWORD tParts = genUVs ? PART_TOPO|PART_TEXMAP : PART_TOPO;
	DWORD otherStuff = OBJ_CHANNELS & ~(PART_GEOM|tParts);
	pobj->SetPartValidity (otherStuff, FOREVER);
	pobj->SetPartValidity (PART_GEOM, gValid);
	pobj->SetPartValidity (tParts, tValid);

	// Number of verts
      // bottom : (lsegs+1)*(wsegs+1)
	  // top    : (lsegs+1)*(wsegs+1)
	  // sides  : (2*lsegs+2*wsegs)*(hsegs-1)

	// Number of rectangular faces.
      // bottom : (lsegs)*(wsegs)
	  // top    : (lsegs)*(wsegs)
	  // sides  : 2*(hsegs*lsegs)+2*(wsegs*lsegs)

	mm.Clear();

	int wsp1 = wsegs + 1;
	int nlayer = 2*(lsegs+wsegs);
	int topStart = (lsegs+1)*(wsegs+1);
	int midStart = 2*topStart;

	int nverts = midStart+nlayer*(hsegs-1);
	int nfaces = 2*(lsegs*wsegs + hsegs*lsegs + wsegs*hsegs);

	mm.setNumVerts(nverts);
	mm.setNumFaces(nfaces);
	mm.InvalidateTopoCache();

	// Do mapping verts first, since they're easy.
	int uvStart[6];
	int ix, iy, iz;
	int nv;
	MNMapFace *tf=NULL;
	if (genUVs) {
		int ls = lsegs+1;
		int ws = wsegs+1;
		int hs = hsegs+1;
		int ntverts = 2*(ls*hs + hs*ws + ws*ls);
		mm.SetMapNum (2);
		mm.M(1)->ClearFlag (MN_DEAD);
		mm.M(1)->setNumFaces (nfaces);
		mm.M(1)->setNumVerts (ntverts);
		UVVert *tv = mm.M(1)->v;
		tf = mm.M(1)->f;

		int xbase = 0;
		int ybase = ls*hs;
		int zbase = ls*hs + hs*ws;

		float dw = 1.0f/float(wsegs);
		float dl = 1.0f/float(lsegs);
		float dh = 1.0f/float(hsegs);

		if (w==0.0f) w = .0001f;
		if (l==0.0f) l = .0001f;
		if (h==0.0f) h = .0001f;
		float u,v;

		// Bottom of box.
		nv = 0;
		uvStart[0] = nv;
		v = 0.0f;
		for (iy =0; iy<ls; iy++) {
			u = 1.0f; 
			for (ix =0; ix<ws; ix++) {
				tv[nv] = UVVert (u, v, 0.0f);
				nv++; u-=dw;
			}
			v += dl;
		}

		// Top of box.
		uvStart[1] = nv;
		v = 0.0f;
		for (iy =0; iy<ls; iy++) {
			u = 0.0f; 
			for (ix =0; ix<ws; ix++) {
				tv[nv] = UVVert (u, v, 0.0f);
				nv++; u+=dw;
			}
			v += dl;
		}

		// Front Face
		uvStart[2] = nv;
		v = 0.0f; 
		for (iz =0; iz<hs; iz++) {
			u = 0.0f;
			for (ix =0; ix<ws; ix++) {
				tv[nv] = UVVert (u, v, 0.0f);
				nv++; u+=dw;
			}
			v += dh;
		}

		// Right Face
		uvStart[3] = nv;
		v = 0.0f;
		for (iz =0; iz<hs; iz++) {
			u = 0.0f; 
			for (iy =0; iy<ls; iy++) {
				tv[nv] = UVVert (u, v, 0.0f);
				nv++; u+=dl;
			}
			v += dh;
		}

		// Back Face
		uvStart[4] = nv;
		v = 0.0f; 
		for (iz =0; iz<hs; iz++) {
			u = 0.0f;
			for (ix =0; ix<ws; ix++) {
				tv[nv] = UVVert (u, v, 0.0f);
				nv++; u+=dw;
			}
			v += dh;
		}

		// Left Face
		uvStart[5] = nv;
		v = 0.0f;
		for (iz =0; iz<hs; iz++) {
			u = 0.0f; 
			for (iy =0; iy<ls; iy++) {
				tv[nv] = UVVert (u, v, 0.0f);
				nv++; u+=dl;
			}
			v += dh;
		}

		assert(nv==ntverts);
	}

	nv = 0;
	
	Point3 vb(w/2.0f,l/2.0f,h);
	Point3 va(-vb.x, -vb.y, 0.0f);

	float dx = w/wsegs;
	float dy = l/lsegs;
	float dz = h/hsegs;

	// do bottom vertices.
	Point3 p;
	p.z = va.z;
	p.y = va.y;
	for (iy=0; iy<=lsegs; iy++) {
		p.x = va.x;
		for (ix=0; ix<=wsegs; ix++) {
			mm.P(nv) = p;
			nv++;
			p.x += dx;
		}
		p.y += dy;
	}

	int kv, nf = 0;

	// do bottom faces.
	// (Note that mapping verts are indexed the same as regular verts on the bottom.)
	for (iy=0; iy<lsegs; iy++) {
		kv = iy*(wsegs+1);
		for (ix=0; ix<wsegs; ix++) {
			if (mPolyBoxSmoothingGroupFix) {
				MAKE_BOTTOM_QUAD_FIX(kv,kv+wsegs+1,kv+wsegs+2,kv+1);
			} else {
				MAKE_BOTTOM_QUAD(kv,kv+wsegs+1,kv+wsegs+2,kv+1);
			}
			MAKE_MAP_QUAD(kv,kv+wsegs+1,kv+wsegs+2,kv+1);
			nf++;
			kv++;
		}
	}
	assert(nf==lsegs*wsegs);

	// do top vertices.
	// (Note that mapping verts are indexed the same as regular verts on the top.)
	p.z = vb.z;
	p.y = va.y;
	for(iy=0; iy<=lsegs; iy++) {
		p.x = va.x;
		for (ix=0; ix<=wsegs; ix++) {
			mm.P(nv) = p;
			p.x += dx;
			nv++;
		}
		p.y += dy;
	}

	// do top faces (lsegs*wsegs);
	for(iy=0; iy<lsegs; iy++) {
		kv = iy*(wsegs+1)+topStart;
		for (ix=0; ix<wsegs; ix++) {
			if (mPolyBoxSmoothingGroupFix) {
				MAKE_TOP_QUAD_FIX (kv,kv+1,kv+wsegs+2,kv+wsegs+1);
			} else {
				MAKE_TOP_QUAD (kv,kv+1,kv+wsegs+2,kv+wsegs+1);
			}
			MAKE_MAP_QUAD(kv,kv+1,kv+wsegs+2,kv+wsegs+1);
			nf++;
			kv++;
		}
	}
	assert(nf==lsegs*wsegs*2);

	// do middle vertices
	for(iz=1; iz<hsegs; iz++) {
		p.z = va.z + dz * iz;
		// front edge
		p.x = va.x;  p.y = va.y;
		for (ix=0; ix<wsegs; ix++) { mm.P(nv)=p; p.x += dx; nv++; }
		// right edge
		p.x = vb.x;	  p.y = va.y;
		for (iy=0; iy<lsegs; iy++) { mm.P(nv)=p; p.y += dy; nv++; }
		// back edge
		p.x =  vb.x;  p.y =  vb.y;
		for (ix=0; ix<wsegs; ix++) { mm.P(nv)=p; p.x -= dx; nv++; }
		// left edge
		p.x = va.x;  p.y =  vb.y;
		for (iy=0; iy<lsegs; iy++) { mm.P(nv)=p; p.y -= dy; nv++; }
	}

	int mv;
	if (hsegs==1) {
		// do FRONT faces -----------------------
		kv = 0;
		mv = topStart;
		for (ix=0; ix<wsegs; ix++) {
			MAKE_FRONT_QUAD (kv, kv+1, mv+1, mv);
			if (tf) {
				int mapv = uvStart[2]+ix;
				MAKE_MAP_QUAD (mapv, mapv+1, mapv+wsegs+2, mapv+wsegs+1);
			}
			nf++;
			kv++;
			mv++;
		}

		// do RIGHT faces.-----------------------
		kv = wsegs;  
		mv = topStart + kv;
		for (iy=0; iy<lsegs; iy++) {
			MAKE_RIGHT_QUAD(kv, kv+wsp1, mv+wsp1, mv);
			if (tf) {
				int mapv = uvStart[3]+iy;
				MAKE_MAP_QUAD (mapv, mapv+1, mapv+lsegs+2, mapv+lsegs+1);
			}
			nf++;
			kv += wsp1;
			mv += wsp1;
		}	

		// do BACK faces.-----------------------
		kv = topStart - 1;
		mv = midStart - 1;
		for (ix=0; ix<wsegs; ix++) {
			MAKE_BACK_QUAD(kv, kv-1, mv-1, mv);
			if (tf) {
				int mapv = uvStart[4]+ix;
				MAKE_MAP_QUAD (mapv, mapv+1, mapv+wsegs+2, mapv+wsegs+1);
			}
			nf++;
			kv --;
			mv --;
		}

		// do LEFT faces.----------------------
		kv = lsegs*(wsegs+1);  // index into bottom
		mv = topStart + kv;
		for (iy=0; iy<lsegs; iy++) {
			MAKE_LEFT_QUAD(kv, kv-wsp1, mv-wsp1, mv);
			if (tf) {
				int mapv = uvStart[5]+iy;
				MAKE_MAP_QUAD (mapv, mapv+1, mapv+lsegs+2, mapv+lsegs+1);
			}
			nf++;
			kv -= wsp1;
			mv -= wsp1;
		}
	} else {
		// do FRONT faces.
		kv = 0;
		mv = midStart;
		for(iz=0; iz<hsegs; iz++) {
			if (iz==hsegs-1) mv = topStart;
			for (ix=0; ix<wsegs; ix++) {
				MAKE_FRONT_QUAD(kv+ix, kv+ix+1, mv+ix+1, mv+ix);
				if (tf) {
					int mapv = uvStart[2] + iz*(wsegs+1) + ix;
					MAKE_MAP_QUAD (mapv, mapv+1, mapv+wsegs+2, mapv+wsegs+1);
				}
				nf++;
			}
			kv = mv;
			mv += nlayer;
		}

		assert(nf==lsegs*wsegs*2 + wsegs*hsegs);
	 
		// do RIGHT faces.-------------------------
		// RIGHT bottom row:
		kv = wsegs; // into bottom layer. 
		mv = midStart + wsegs; // first layer of mid verts


		for (iy=0; iy<lsegs; iy++) {
			MAKE_RIGHT_QUAD(kv, kv+wsp1, mv+1, mv);
			if (tf) {
				int mapv = uvStart[3]+iy;
				MAKE_MAP_QUAD (mapv, mapv+1, mapv+lsegs+2, mapv+lsegs+1);
			}
			nf++;
			kv += wsp1;
			mv ++;
		}

		// RIGHT middle part:
		kv = midStart + wsegs; 
		for(iz=0; iz<hsegs-2; iz++) {
			mv = kv + nlayer;
			for (iy=0; iy<lsegs; iy++) {
				MAKE_RIGHT_QUAD(kv+iy, kv+iy+1, mv+iy+1, mv+iy);
				if (tf) {
					int mapv = uvStart[3] + (iz+1)*(lsegs+1) + iy;
					MAKE_MAP_QUAD (mapv, mapv+1, mapv+lsegs+2, mapv+lsegs+1);
				}
				nf++;
			}
			kv += nlayer;
		}

		// RIGHT top row:
		kv = midStart + wsegs + (hsegs-2)*nlayer; 
		mv = topStart + wsegs;
		for (iy=0; iy<lsegs; iy++) {
			MAKE_RIGHT_QUAD(kv, kv+1, mv+wsp1, mv);
			if (tf) {
				int mapv = uvStart[3] + (hsegs-1)*(lsegs+1) + iy;
				MAKE_MAP_QUAD (mapv, mapv+1, mapv+lsegs+2, mapv+lsegs+1);
			}
			nf++;
			mv += wsp1;
			kv++;
		}
		
		assert(nf==lsegs*wsegs*2 + wsegs*hsegs + lsegs*hsegs);

		// do BACK faces. ---------------------
		// BACK bottom row:
		kv = topStart - 1;
		mv = midStart + wsegs + lsegs;
		for (ix=0; ix<wsegs; ix++) {
			MAKE_BACK_QUAD(kv, kv-1, mv+1, mv);
			if (tf) {
				int mapv = uvStart[4]+ix;
				MAKE_MAP_QUAD (mapv, mapv+1, mapv+wsegs+2, mapv+wsegs+1);
			}
			nf++;
			kv --;
			mv ++;
		}

		// BACK middle part:
		kv = midStart + wsegs + lsegs; 
		for(iz=0; iz<hsegs-2; iz++) {
			mv = kv + nlayer;
			for (ix=0; ix<wsegs; ix++) {
				MAKE_BACK_QUAD(kv+ix, kv+ix+1, mv+ix+1, mv+ix);
				if (tf) {
					int mapv = uvStart[4] + (iz+1)*(wsegs+1) + ix;
					MAKE_MAP_QUAD (mapv, mapv+1, mapv+wsegs+2, mapv+wsegs+1);
				}
				nf++;
			}
			kv += nlayer;
		}

		// BACK top row:
		kv = midStart + wsegs + lsegs + (hsegs-2)*nlayer; 
		mv = topStart + lsegs*(wsegs+1)+wsegs;
		for (ix=0; ix<wsegs; ix++) {
			MAKE_BACK_QUAD(kv, kv+1, mv-1, mv);
			if (tf) {
				int mapv = uvStart[4] + (wsegs+1)*(hsegs-1) + ix;
				MAKE_MAP_QUAD (mapv, mapv+1, mapv+wsegs+2, mapv+wsegs+1);
			}
			nf++;
			mv --;
			kv ++;
		}

		assert(nf==lsegs*wsegs*2 + wsegs*hsegs*2 + lsegs*hsegs);

		// do LEFT faces. -----------------
		// LEFT bottom row:
		kv = lsegs*(wsegs+1);  // index into bottom
		mv = midStart + 2*wsegs +lsegs;
		for (iy=0; iy<lsegs; iy++) {
			int nextm = mv+1;
			if (iy==lsegs-1) nextm -= nlayer;
			MAKE_LEFT_QUAD(kv, kv-wsp1, nextm, mv);
			if (tf) {
				int mapv = uvStart[5]+iy;
				MAKE_MAP_QUAD (mapv, mapv+1, mapv+lsegs+2, mapv+lsegs+1);
			}
			nf++;
			kv -=wsp1;
			mv ++;
		}

		// LEFT middle part:
		kv = midStart + 2*wsegs + lsegs; 
		for(iz=0; iz<hsegs-2; iz++) {
			mv = kv + nlayer;
			for (iy=0; iy<lsegs; iy++) {
				int nextm = mv+1;
				int nextk = kv+iy+1;
				if (iy==lsegs-1) { 
					nextm -= nlayer;
					nextk -= nlayer;
				}
				MAKE_LEFT_QUAD(kv+iy, nextk, nextm, mv);
				if (tf) {
					int mapv = uvStart[5] + (iz+1)*(lsegs+1) + iy;
					MAKE_MAP_QUAD (mapv, mapv+1, mapv+lsegs+2, mapv+lsegs+1);
				}
				nf++;
				mv++;
			}
			kv += nlayer;
		}

		// LEFT top row:
		kv = midStart + 2*wsegs + lsegs+ (hsegs-2)*nlayer; 
		mv = topStart + lsegs*(wsegs+1);
		for (iy=0; iy<lsegs; iy++) {
			int nextk = kv+1;
			if (iy==lsegs-1) nextk -= nlayer;
			MAKE_LEFT_QUAD(kv, nextk, mv-wsp1, mv);
			if (tf) {
				int mapv = uvStart[5] + (hsegs-1)*(lsegs+1) + iy;
				MAKE_MAP_QUAD (mapv, mapv+1, mapv+lsegs+2, mapv+lsegs+1);
			}
			nf++;
			mv -= wsp1;
			kv++;
		}
	}

	mm.InvalidateGeomCache();
	mm.FillInMesh();
	return (Object *) pobj;
}



