/**********************************************************************
 *<
	FILE: tripatch.cpp

	DESCRIPTION:  A triangular patch object implementation

	CREATED BY: Tom Hudson

	HISTORY: created 6 July 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#include "prim.h"

#ifndef NO_PATCHES

#include "iparamm.h"
#include "surf_api.h"
#include "tessint.h"
#include "MeshDelta.h"

#define TRIPATCH_CLASS_ID 0x1fff

// Parameter block indices
#define PB_LENGTH	0
#define PB_WIDTH	1
#define PB_TEXTURE	2

// Non-parameter block indices
#define PB_TI_POS			0
#define PB_TI_LENGTH		1
#define PB_TI_WIDTH			2

class TriPatchCreateCallBack;

#define BMIN_LENGTH		float(0)
#define BMAX_LENGTH		float(1.0E30)
#define BMIN_WIDTH		float(0)
#define BMAX_WIDTH		float(1.0E30)

#define BDEF_DIM		float(0)

class TriPatchObject: public GeomObject, public IParamArray  {			   
	friend class TriPatchCreateCallBack;
	friend BOOL CALLBACK TriPatchParamDialogProc( HWND hDlg, UINT message, 
		WPARAM wParam, LPARAM lParam );
	
	public:
		// Object parameters		
		IParamBlock *pblock;
		Interval ivalid;
		int creating;

		// Class vars
		static IParamMap *pmapTypeIn;
		static IParamMap *pmapParam;
		static IObjParam *ip;
		static BOOL dlgTexture;
		static Point3 crtPos;		
		static float crtWidth, crtLength;
		static TriPatchObject *editOb;

		// Caches
		PatchMesh patch;

		//  inherited virtual methods for Reference-management
		RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message );
		void BuildPatch(TimeValue t,PatchMesh& amesh);
		void GetBBox(TimeValue t, Matrix3 &tm, Box3& box);

		TriPatchObject();
		~TriPatchObject();

		void InvalidateUI();
		void PatchMeshInvalid() { ivalid.SetEmpty(); }

		//  inherited virtual methods:		

		// From BaseObject
		int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
		void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		Mesh* GetRenderMesh(TimeValue t, INode *inode, View& view, BOOL& needDelete);
		void PrepareMesh(TimeValue t);
		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam *ip, ULONG flags, Animatable *prev );
		void EndEditParams( IObjParam *ip, ULONG flags, Animatable *next );
		TCHAR *GetObjectName() { return GetString(IDS_TH_TRIPATCH); }

		// From Object
		ObjectState Eval(TimeValue time);
		void InitNodeName(TSTR& s) { s = GetString(IDS_TH_TRIPATCH); }		
		Interval ObjectValidity(TimeValue t);
		int CanConvertToType(Class_ID obtype);
		Object* ConvertToType(TimeValue t, Class_ID obtype);
		
		// From GeomObject
		int IntersectRay(TimeValue t, Ray& r, float& at, Point3& norm);
		ObjectHandle CreateTriObjRep(TimeValue t);  // for rendering, also for deformation		
		void GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box );
		void GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vxt, Box3& box );
		void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm=NULL,BOOL useSel=FALSE );

		PatchMesh& GetPatchMesh(TimeValue t);
		void UpdatePatchMesh(TimeValue t);

		// Animatable methods
		void DeleteThis() { delete this; }
		void FreeCaches(); 
		Class_ID ClassID() { return Class_ID( TRIPATCH_CLASS_ID, 0); }  
		void GetClassName(TSTR& s) { s = TSTR(GetString(IDS_TH_TRIPATCHOBJECT_CLASS)); }		
		
		int NumSubs() { return 1; }  
		Animatable* SubAnim(int i) { return pblock; }
		TSTR SubAnimName(int i) { return TSTR(GetString(IDS_TH_PARAMETERS));}		
		int IsKeyable() { return 1;}
		BOOL BypassTreeView() { return FALSE; }

		// From ref
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i) {return pblock;}
		void SetReference(int i, RefTargetHandle rtarg) {pblock=(IParamBlock*)rtarg;}

		// IO
		IOResult Load(ILoad *iload);

		// From IParamArray
		BOOL SetValue(int i, TimeValue t, int v);
		BOOL SetValue(int i, TimeValue t, float v);
		BOOL SetValue(int i, TimeValue t, Point3 &v);
		BOOL GetValue(int i, TimeValue t, int &v, Interval &ivalid);
		BOOL GetValue(int i, TimeValue t, float &v, Interval &ivalid);
		BOOL GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid);

		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);

		// Automatic texture support
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);
	};				

//------------------------------------------------------

class TriPatchClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new TriPatchObject; }
	const TCHAR *	ClassName() { return GetString(IDS_TH_TRI_PATCH_CLASS); }
	SClass_ID		SuperClassID() { return GEOMOBJECT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(TRIPATCH_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_TH_PATCH_GRIDS);  }
	void			ResetClassParams(BOOL fileReset);
	};

static TriPatchClassDesc triPatchDesc;

ClassDesc* GetTriPatchDesc() { return &triPatchDesc; }

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// class variable for tri patch class.
IParamMap *TriPatchObject::pmapParam  = NULL;
IParamMap *TriPatchObject::pmapTypeIn = NULL;
IObjParam *TriPatchObject::ip;
BOOL TriPatchObject::dlgTexture = TRUE;
Point3 TriPatchObject::crtPos         = Point3(0,0,0);		
float TriPatchObject::crtWidth        = 0.0f; 
float TriPatchObject::crtLength       = 0.0f;
TriPatchObject *TriPatchObject::editOb = NULL;

void TriPatchClassDesc::ResetClassParams(BOOL fileReset)
	{
	TriPatchObject::dlgTexture = FALSE;
	TriPatchObject::crtWidth   = 0.0f; 
	TriPatchObject::crtLength  = 0.0f;
	TriPatchObject::crtPos     = Point3(0,0,0);
	}

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
			
	};
#define TYPEINDESC_LENGTH 3

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
		
	// Gen UVs
	ParamUIDesc(PB_TEXTURE,TYPE_SINGLECHEKBOX,IDC_GENTEXTURE),			
	};
#define PARAMDESC_LENGTH 3


static ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },		
	{ TYPE_FLOAT, NULL, TRUE, 1 },
 };
static ParamBlockDescID descVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },		
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, FALSE, 4 },
 };
#define PBLOCK_LENGTH	3

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,2,0)			
	};
#define NUM_OLDVERSIONS	1	

// Current version
#define CURRENT_VERSION	1
static ParamVersionDesc curVersion(descVer1,PBLOCK_LENGTH,CURRENT_VERSION);

//--- TypeInDlgProc --------------------------------

class TriPatchTypeInDlgProc : public ParamMapUserDlgProc {
	public:
		TriPatchObject *ob;

		TriPatchTypeInDlgProc(TriPatchObject *o) {ob=o;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

BOOL TriPatchTypeInDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_TI_CREATE: {					
					if (ob->crtLength==0.0) return TRUE;
					if (ob->crtWidth==0.0) return TRUE;

					// Return focus to the top spinner
					SetFocus(GetDlgItem(hWnd, IDC_TI_POSX));
					
					// We only want to set the value if the object is 
					// not in the scene.
					if (ob->TestAFlag(A_OBJ_CREATING)) {
						ob->pblock->SetValue(PB_LENGTH,0,ob->crtLength);
						ob->pblock->SetValue(PB_WIDTH,0,ob->crtWidth);
						}

					Matrix3 tm(1);
					tm.SetTrans(ob->crtPos);					
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

void TriPatchObject::BeginEditParams( IObjParam *ip, ULONG flags, Animatable *prev )
	{
	editOb = this;
	this->ip = ip;
	
	if (pmapTypeIn && pmapParam) {
		
		// Left over from last shape ceated
		pmapTypeIn->SetParamBlock(this);
		pmapParam->SetParamBlock(pblock);
	} else {
		
		// Gotta make a new one.
		if (flags&BEGIN_EDIT_CREATE) {
			pmapTypeIn = CreateCPParamMap(
				descTypeIn,TYPEINDESC_LENGTH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_TRIPATCHPARAM2),
				GetString(IDS_TH_KEYBOARD_ENTRY),
				APPENDROLL_CLOSED);
			}

		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_TRIPATCHPARAM),
			GetString(IDS_TH_PARAMETERS),
			0);
		}

	if(pmapTypeIn) {
		// A callback for the type in.
		pmapTypeIn->SetUserDlgProc(new TriPatchTypeInDlgProc(this));
		}
	}
		
void TriPatchObject::EndEditParams( IObjParam *ip, ULONG flags, Animatable *next )
	{
	editOb = NULL;
	this->ip = NULL;

	if (flags&END_EDIT_REMOVEUI ) {
		if (pmapTypeIn) DestroyCPParamMap(pmapTypeIn);
		DestroyCPParamMap(pmapParam);
		pmapParam  = NULL;
		pmapTypeIn = NULL;
		}

	// Save these values in class variables so the next object created will inherit them.
	pblock->GetValue(PB_TEXTURE,ip->GetTime(),dlgTexture,FOREVER);	
	}

PatchMesh &TriPatchObject::GetPatchMesh(TimeValue t) {
	UpdatePatchMesh(t);
	return patch;
	}

void TriPatchObject::UpdatePatchMesh(TimeValue t) {
	if ( ivalid.InInterval(t) ) {
		return;
		}
	BuildPatch(t,patch);
	}

void TriPatchObject::FreeCaches() {
	ivalid.SetEmpty();
	patch.FreeAll();
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

#define MAKEVEC(vec, p1, p2) { patch.setVec(vec, p1 + (p2 - p1) / 3.0f); patch.setVec(vec+1, p2 - (p2 - p1) / 3.0f); }

void TriPatchObject::BuildPatch(TimeValue t,PatchMesh& amesh)
	{
	int nverts = 4;
	int nvecs = 16;
	float l, w;
	int tex;
	
	// Start the validity interval at forever and whittle it down.
	ivalid = FOREVER;
	pblock->GetValue( PB_LENGTH, t, l, ivalid );
	pblock->GetValue( PB_WIDTH, t, w, ivalid );
	pblock->GetValue( PB_TEXTURE, t, tex, ivalid );

	amesh.setNumVerts(nverts);
	amesh.setNumTVerts(tex ? nverts : 0);
	amesh.setNumVecs(nvecs);
	amesh.setNumPatches(2);
	amesh.setNumTVPatches(tex ? 2 : 0);

	Point3 v0 = Point3(-w, -l, 0.0f) / 2.0f;   
	Point3 v1 = v0 + Point3(w, 0.0f, 0.0f);
	Point3 v2 = v0 + Point3(w, l, 0.0f);
	Point3 v3 = v0 + Point3(0.0f, l, 0.0f);

	// Create the vertices.
	amesh.verts[0].flags = PVERT_COPLANAR;
	amesh.verts[1].flags = PVERT_COPLANAR;
	amesh.verts[2].flags = PVERT_COPLANAR;
	amesh.verts[3].flags = PVERT_COPLANAR;
	if(tex) {
		amesh.setTVert(0, UVVert(0,0,0));
		amesh.setTVert(1, UVVert(1,0,0));
		amesh.setTVert(2, UVVert(1,1,0));
		amesh.setTVert(3, UVVert(0,1,0));
		}
	amesh.setVert(0, v0);
	amesh.setVert(1, v1);
	amesh.setVert(2, v2);
	amesh.setVert(3, v3);

	// Create the vectors
	MAKEVEC(0, v0, v1);
	MAKEVEC(2, v1, v2);
	MAKEVEC(4, v2, v3);
	MAKEVEC(6, v3, v0);
	MAKEVEC(8, v3, v1);

	// Create patches.
	amesh.MakeTriPatch(0, 0, 0, 1, 1, 9, 8, 3, 6, 7, 10, 11, 12, 1);
	amesh.MakeTriPatch(1, 1, 2, 3, 2, 4, 5, 3, 8, 9, 13, 14, 15, 1);
	Patch &p1 = amesh.patches[0];
	Patch &p2 = amesh.patches[1];
	if(tex) {
		amesh.getTVPatch(0).setTVerts(0,1,3);
		amesh.getTVPatch(1).setTVerts(1,2,3);
		}

	// Finish up patch internal linkages (and bail out if it fails!)
	assert(amesh.buildLinkages());

	// Calculate the interior bezier points on the PatchMesh's patches
	amesh.computeInteriors();

	amesh.InvalidateGeomCache();

	// Tell the PatchMesh it just got changed
	amesh.InvalidateMesh();
	}


TriPatchObject::TriPatchObject() 
	{
	MakeRefByID(FOREVER, 0, CreateParameterBlock(descVer1, PBLOCK_LENGTH, CURRENT_VERSION));

	pblock->SetValue(PB_LENGTH,0,crtLength);
	pblock->SetValue(PB_WIDTH,0,crtWidth);
	pblock->SetValue(PB_TEXTURE,0,dlgTexture);

	ivalid.SetEmpty();
	creating = 0;	
	}

TriPatchObject::~TriPatchObject()
	{
	DeleteAllRefsFromMe();
	pblock = NULL;
	}

class TriPatchCreateCallBack: public CreateMouseCallBack {
	TriPatchObject *ob;
	Point3 p0,p1;
	IPoint2 sp0;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		void SetObj(TriPatchObject *obj) { ob = obj; }
	};

int TriPatchCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
	Point3 d;

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
			case 0:
				sp0 = m;
				ob->creating = 1;	// tell object we're building it so we can disable snapping to itself
				#ifdef _3D_CREATE	
					p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				#else	
					p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				#endif
				ob->pblock->SetValue(PB_WIDTH,0,0.0f);
				ob->pblock->SetValue(PB_LENGTH,0,0.0f);
				p1 = p0 + Point3(0.01f,0.01f,0.0f);
				mat.SetTrans(float(.5)*(p0+p1));				
				break;
			case 1:
				#ifdef _3D_CREATE	
					p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				#else	
					p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				#endif
				mat.SetTrans(float(.5)*(p0+p1));
				d = p1-p0;
				ob->pblock->SetValue(PB_WIDTH,0,float(fabs(d.x)));
				ob->pblock->SetValue(PB_LENGTH,0,float(fabs(d.y)));
				ob->pmapParam->Invalidate();										
				if (msg==MOUSE_POINT) {
					ob->creating = 0;
					return (Length(m-sp0)<3) ? CREATE_ABORT: CREATE_STOP;
					}
				break;
			}
		}
	else
	if (msg == MOUSE_ABORT) {
		ob->creating = 0;
		return CREATE_ABORT;
		}

	return TRUE;
	}

static TriPatchCreateCallBack patchCreateCB;

CreateMouseCallBack* TriPatchObject::GetCreateMouseCallBack() {
	patchCreateCB.SetObj(this);
	return(&patchCreateCB);
	}


// From BaseObject
int TriPatchObject::HitTest(TimeValue t, INode *inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt) {	
	HitRegion hitRegion;
	GraphicsWindow *gw = vpt->getGW();	
	Material *mtl = gw->getMaterial();
   	
	UpdatePatchMesh(t);
	gw->setTransform(inode->GetObjectTM(t));

	MakeHitRegion(hitRegion, type, crossing, 4, p);
	return patch.select( gw, mtl, &hitRegion, flags & HIT_ABORTONHIT );
	}

void TriPatchObject::Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) {
	if(creating)	// If creating this one, don't try to snap to it!
		return;

	Matrix3 tm = inode->GetObjectTM(t);	
	GraphicsWindow *gw = vpt->getGW();	
   	
	UpdatePatchMesh(t);
	gw->setTransform(tm);

	patch.snap( gw, snap, p, tm );
	}

int TriPatchObject::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags) {
	Matrix3 tm;
	GraphicsWindow *gw = vpt->getGW();
	gw->setTransform(inode->GetObjectTM(t));
	UpdatePatchMesh(t);
	if(!(gw->getRndMode() & GW_BOX_MODE)) {
		PrepareMesh(t);
		Mesh& mesh = patch.GetMesh();
		if(mesh.getNumVerts()) {
			mesh.render( gw, inode->Mtls(),
				(flags&USE_DAMAGE_RECT) ? &vpt->GetDammageRect() : NULL, 
				COMP_ALL | (inode->Selected()?COMP_OBJSELECTED:0), inode->NumMtls());	
			}
		}
	patch.render( gw, inode->Mtls(),
		(flags&USE_DAMAGE_RECT) ? &vpt->GetDammageRect() : NULL, 
		COMP_ALL | (inode->Selected()?COMP_OBJSELECTED:0), inode->NumMtls());	
	return(0);
	}

//////////////////////////////////  MESH WELDER ////////////////////
static void
WeldMesh(Mesh *mesh, float thresh)
{
	if (thresh == 0.0f)
		thresh = (float)1e-30; // find only the coincident ones	BitArray vset, eset;
	BitArray vset;
	vset.SetSize(mesh->numVerts);
	vset.SetAll();
	MeshDelta md;
	md.WeldByThreshold(*mesh, vset, thresh);
	md.Apply(*mesh);
}


typedef int (* GTess)(void *obj, SurfaceType type, Matrix3 *otm, Mesh *mesh,
							TessApprox *tess, TessApprox *disp, View *view,
							Mtl* mtl, BOOL dumpMiFile, BOOL splitMesh);
static GTess psGTessFunc = NULL;

// This function get the function to do GAP Tessellation from
// tessint.dll.  This is required because of the link order between
// core.dll and tessint.dll and gmi.dll.  -- Charlie Thaeler
static void
GetGTessFunction()
{
    if (psGTessFunc)
        return;
    // Get the library handle for tessint.dll
    HINSTANCE hInst = NULL;
	hInst = LoadLibraryEx(_T("tessint.dll"), NULL, 0);
    assert(hInst);

    psGTessFunc = (GTess)GetProcAddress(hInst, _T("GapTessellate"));
	assert(psGTessFunc);
}

Mesh* TriPatchObject::GetRenderMesh(TimeValue t, INode *inode, View& view, BOOL& needDelete) {
	UpdatePatchMesh(t);
	TessApprox tess = patch.GetProdTess();
	if (tess.type == TESS_SET) {
		needDelete = FALSE;
		patch.InvalidateMesh(); // force this...
		// temporarlily set the view tess to prod tess
		TessApprox tempTess = patch.GetViewTess();
		patch.SetViewTess(tess);
		PrepareMesh(t);
		patch.SetViewTess(tempTess);
		return &patch.GetMesh();
	} else {
		Mesh *nmesh = new Mesh/*(mesh)*/;
		Matrix3 otm = inode->GetObjectTM(t);

		Box3 bbox;
		GetDeformBBox(t, bbox);
		tess.merge *= Length(bbox.Width())/1000.0f;
		TessApprox disp = patch.GetDispTess();
		disp.merge *= Length(bbox.Width())/1000.0f;

		GetGTessFunction();
		(*psGTessFunc)(&patch, BEZIER_PATCH, &otm, nmesh, &tess, &disp, &view, inode->GetMtl(), FALSE, FALSE);
		if (tess.merge > 0.0f && patch.GetProdTessWeld())
			WeldMesh(nmesh, tess.merge);
		needDelete = TRUE;
		return nmesh;
	}
}

void TriPatchObject::PrepareMesh(TimeValue t) {
	UpdatePatchMesh(t);
	patch.PrepareMesh();
	}

// From GeomObject
int TriPatchObject::IntersectRay(TimeValue t, Ray& r, float& at, Point3& norm) {
	PrepareMesh(t);	// Turn it into a mesh
	return patch.IntersectRay(r, at, norm);
	}

ObjectHandle TriPatchObject::CreateTriObjRep(TimeValue t) {
	TriObject *tri = CreateNewTriObject();
	PrepareMesh(t);	// Turn it into a mesh
	tri->GetMesh() = patch.GetMesh();	// Place it into the TriObject
	return(ObjectHandle(tri));
	}

void TriPatchObject::GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel )
	{
	UpdatePatchMesh(t);
	patch.GetDeformBBox(box, tm, useSel);
	}

void TriPatchObject::GetLocalBoundBox(TimeValue t, INode *inode,ViewExp* vpt,  Box3& box ) {
	GetDeformBBox(t,box);
	}

void TriPatchObject::GetWorldBoundBox(TimeValue t, INode *inode, ViewExp* vpt, Box3& box )
	{
	Box3	patchBox;

	Matrix3 mat = inode->GetObjectTM(t);
	
	GetLocalBoundBox(t,inode,vpt,patchBox);
	box.Init();
	for(int i = 0; i < 8; i++)
		box += mat * patchBox[i];
	}

//
// Reference Managment:
//

//
// Reference Managment:
//

ParamDimension *TriPatchObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_LENGTH:return stdWorldDim;
		case PB_WIDTH: return stdWorldDim;
		default: return defaultDim;
		}
	}

TSTR TriPatchObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_LENGTH: return TSTR(GetString(IDS_RB_LENGTH));
		case PB_WIDTH:  return TSTR(GetString(IDS_RB_WIDTH));
		default: return TSTR(_T(""));
		}
	}

// From ParamArray
BOOL TriPatchObject::SetValue(int i, TimeValue t, int v) 
	{
	return TRUE;
	}

BOOL TriPatchObject::SetValue(int i, TimeValue t, float v)
	{
	switch (i) {				
		case PB_TI_LENGTH: crtLength = v; break;
		case PB_TI_WIDTH:  crtWidth = v; break;
		}	
	return TRUE;
	}

BOOL TriPatchObject::SetValue(int i, TimeValue t, Point3 &v) 
	{
	switch (i) {
		case PB_TI_POS: crtPos = v; break;
		}		
	return TRUE;
	}

BOOL TriPatchObject::GetValue(int i, TimeValue t, int &v, Interval &ivalid) 
	{
	return TRUE;
	}

BOOL TriPatchObject::GetValue(int i, TimeValue t, float &v, Interval &ivalid) 
	{	
	switch (i) {				
		case PB_TI_LENGTH: v = crtLength; break;
		case PB_TI_WIDTH:  v = crtWidth; break;
		}
	return TRUE;
	}

BOOL TriPatchObject::GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_POS: v = crtPos; break;		
		}
	return TRUE;
	}

void TriPatchObject::InvalidateUI() 
	{
	if (pmapParam) pmapParam->Invalidate();
	}

RefResult TriPatchObject::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
   		PartID& partID, 
   		RefMessage message ) 
   	{
	switch (message) {
		case REFMSG_CHANGE:
			PatchMeshInvalid();
			if (editOb==this) InvalidateUI();
			break;

		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			gpd->dim = GetParameterDim(gpd->index);			
			return REF_STOP; 
			}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			gpn->name = GetParameterName(gpn->index);			
			return REF_STOP; 
			}
		}
	return(REF_SUCCEED);
	}



ObjectState TriPatchObject::Eval(TimeValue time){
	return ObjectState(this);
	}

Interval TriPatchObject::ObjectValidity(TimeValue t) {
	UpdatePatchMesh(t);
	return ivalid;	
	}

int TriPatchObject::CanConvertToType(Class_ID obtype) {
	if (obtype==patchObjectClassID || obtype==defObjectClassID || 
		obtype==mapObjectClassID || obtype==triObjectClassID
#ifndef NO_NURBS
		|| obtype==EDITABLE_SURF_CLASS_ID
#endif
        ) {
		return 1;
		}
	if (Object::CanConvertToType (obtype)) return 1;
	if (CanConvertPatchObject (obtype)) return 1;
	return 0;
	}

Object* TriPatchObject::ConvertToType(TimeValue t, Class_ID obtype) {
	if(obtype == patchObjectClassID || obtype == defObjectClassID || obtype == mapObjectClassID) {
		PatchObject *ob;
		UpdatePatchMesh(t);
		ob = new PatchObject();	
		ob->patch = patch;
		ob->SetChannelValidity(TOPO_CHAN_NUM,ObjectValidity(t));
		ob->SetChannelValidity(GEOM_CHAN_NUM,ObjectValidity(t));
		return ob;
		}

	if(obtype == triObjectClassID) {
		TriObject *ob = CreateNewTriObject();
		PrepareMesh(t);
		ob->GetMesh() = patch.GetMesh();
		ob->SetChannelValidity(TOPO_CHAN_NUM,ObjectValidity(t));
		ob->SetChannelValidity(GEOM_CHAN_NUM,ObjectValidity(t));
		return ob;
		}
#ifndef NO_NURBS
	if (obtype==EDITABLE_SURF_CLASS_ID) {
		PatchObject *pob;
		UpdatePatchMesh(t);
		pob = new PatchObject();	
		pob->patch = patch;
		Object *ob = BuildEMObjectFromPatchObject(pob);
		delete pob;
		ob->SetChannelValidity(TOPO_CHAN_NUM, ObjectValidity(t));
		ob->SetChannelValidity(GEOM_CHAN_NUM, ObjectValidity(t));
		return ob;
		}
#endif
	if (Object::CanConvertToType (obtype)) return Object::ConvertToType (t, obtype);

	if (CanConvertPatchObject (obtype)) {
		PatchObject *ob;
		UpdatePatchMesh(t);
		ob = new PatchObject();	
		ob->patch = patch;
		ob->SetChannelValidity(TOPO_CHAN_NUM,ObjectValidity(t));
		ob->SetChannelValidity(GEOM_CHAN_NUM,ObjectValidity(t));
		Object *ret = ob->ConvertToType (t, obtype);
		ob->DeleteThis ();
		return ret;
		}
	return NULL;
	}

RefTargetHandle TriPatchObject::Clone(RemapDir& remap) {
	TriPatchObject* newob = new TriPatchObject();
	newob->ReplaceReference(0,pblock->Clone(remap));
	newob->ivalid.SetEmpty();	
	BaseClone(this, newob, remap);
	return(newob);
	}

#define GRID_MAPPING_CHUNK 0x1000

// IO
class TriPatchPostLoadCallback : public PostLoadCallback {
	public:
		BOOL tex;
		ParamBlockPLCB *cb;
		TriPatchPostLoadCallback(ParamBlockPLCB *c) {cb=c; tex=FALSE;}
		void proc(ILoad *iload) {
			ReferenceTarget *targ = cb->targ;
			cb->proc(iload);
			if (tex) {				
				((TriPatchObject*)targ)->pblock->SetValue(PB_TEXTURE,0,1);
				}
			delete this;
			}
	};

IOResult  TriPatchObject::Load(ILoad *iload) {
	TriPatchPostLoadCallback *plcb = new TriPatchPostLoadCallback(new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,0));
	iload->RegisterPostLoadCallback(plcb);
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case GRID_MAPPING_CHUNK:
				plcb->tex = TRUE;	// Deal with this old switch after loading
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

BOOL TriPatchObject::HasUVW() { 
	BOOL genUVs;
	Interval v;
	pblock->GetValue(PB_TEXTURE, 0, genUVs, v);
	return genUVs; 
	}

void TriPatchObject::SetGenUVW(BOOL sw) {  
	if (sw==HasUVW()) return;
	pblock->SetValue(PB_TEXTURE,0, sw);				
	InvalidateUI();
	}

#endif // NO_PATCHES
