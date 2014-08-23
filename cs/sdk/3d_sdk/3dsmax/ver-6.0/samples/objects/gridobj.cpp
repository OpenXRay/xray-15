/**********************************************************************
 *<
	FILE: gridobj.cpp

	DESCRIPTION:  A simple grid object implementation

	CREATED BY: Peter Watje

	HISTORY: Oct 31, 1998

 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/

#include "prim.h"

// xavier robitaille | 03.02.12 | the plane object had been left out...
#ifndef NO_OBJECT_STANDARD_PRIMITIVES

#include "iparamm2.h"
#include "Simpobj.h"
#include "surf_api.h"

#define A_RENDER			A_PLUGIN1
#define PBLOCK_REF_NO	 0


#define GRIDID 0x81f1dfc, 0x77566f65

class GridObject : public SimpleObject2{
	public:
		// Class vars
		static IObjParam *ip;
		int createMeth;
		Point3 crtPos;		
		static float crtWidth, crtHeight, crtLength;
		static BOOL typeinCreate;
		float RenderWidth,RenderLength;

		GridObject();
		
		// From Object
		int CanConvertToType(Class_ID obtype);
		void GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist);
		Object* ConvertToType(TimeValue t, Class_ID obtype);
		
		// From BaseObject
		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		TCHAR *GetObjectName() { return GetString(IDS_PW_GRID_OBJECT); }

// JBW: direct ParamBlock access is added
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock2; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock2->ID() == id) ? pblock2 : NULL; } // return id'd ParamBlock


		// Animatable methods
		void DeleteThis() { delete this; }
		Class_ID ClassID() { return Class_ID( GRIDID); }  
		
		// From ref
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		IOResult Load(ILoad *iload);

		// From SimpleObject
		void BuildMesh(TimeValue t);
		void BuildGridPatch(TimeValue t, PatchMesh &patch);

#ifndef NO_NURBS
		Object *BuildNURBSPlane(TimeValue t);
#endif

		BOOL OKtoDisplay(TimeValue t);
		void InvalidateUI();
		BOOL HasUVW() ;
		void SetGenUVW(BOOL sw);

		int RenderBegin(TimeValue t, ULONG flags);
		int RenderEnd(TimeValue t);

	};				


float GridObject::crtWidth        = 25.0f; 
float GridObject::crtLength       = 25.0f;
BOOL GridObject::typeinCreate       = FALSE;


#define BOTTOMPIV


//--- ClassDescriptor and class vars ---------------------------------

class GridObjClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) {return new GridObject;}
	const TCHAR *	ClassName() { return GetString(IDS_PW_GRID); }
	SClass_ID		SuperClassID() { return GEOMOBJECT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(GRIDID); }
	const TCHAR* 	Category() { return GetString(IDS_PW_PRIMITIVES);}	
//	void			ResetClassParams(BOOL fileReset);

	const TCHAR*	InternalName() { return _T("meshGrid"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle

	};

static GridObjClassDesc gridObjDesc;

ClassDesc* GetGridobjDesc() { return &gridObjDesc; }

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// class variable for sphere class.
IObjParam *GridObject::ip         = NULL;


//
//
//	Creation method grid_creation_type,


enum {  grid_creation_type,grid_type_in, grid_params };
// grid_creation_type param IDs
enum { grid_create_meth };
// grid_type_in param IDs
enum { grid_ti_pos, grid_ti_length, grid_ti_width};
// grid_param param IDs
enum { grid_length,
	   grid_width,
	   grid_wsegs,
	   grid_lsegs,
	   grid_rsegs,
	   grid_rscale,
	   grid_genuvs,
	   grid_leftover,
	   grid_leftover1,
	 };

// JBW: here are the two static block descriptors.  This form of 
//      descriptor declaration uses a static NParamBlockDesc instance whose constructor
//      uses a varargs technique to walk through all the param definitions.
//      It has the advantage of supporting optional and variable type definitions, 
//      but may generate a tad more code than a simple struct template.  I'd
//      be interested in opinions about this.

//      I'll briefly describe the first definition so you can figure the others.  Note
//      that in certain places where strings are expected, you supply a string resource ID rather than
//      a string at it does the lookup for you as needed.
//
//		line 1: block ID, internal name, local (subanim) name, flags
//																 AUTO_UI here means the rollout will
//																 be automatically created (see BeginEditParams for details)
//      line 2: since AUTO_UI was set, this line gives: 
//				dialog resource ID, rollout title, flag test, appendRollout flags
//		line 3: required info for a parameter:
//				ID, internal name, type, flags, local (subanim) name
//		lines 4-6: optional parameter declaration info.  each line starts with a tag saying what
//              kind of spec it is, in this case default value, value range, and UI info as would
//              normally be in a ParamUIDesc less range & dimension
//	    the param lines are repeated as needed for the number of parameters defined.

// class creation type block
static ParamBlockDesc2 grid_crtype_blk ( grid_creation_type, _T("GridCreationType"), 0, &gridObjDesc, P_CLASS_PARAMS + P_AUTO_UI, 
	//rollout
	IDD_GRIDPARAM1, IDS_PW_CREATIONMETHOD, BEGIN_EDIT_CREATE, 0, NULL,
	// params
	grid_create_meth,  _T("typeinCreationMethod"), 		TYPE_INT, 		0, IDS_PW_CREATIONMETHOD, 	 
		p_default, 		0, 
		p_range, 		0, 1, 
		p_ui, 			TYPE_RADIO, 	2, IDC_CREATECUBE, IDC_CREATEBOX, 
		end, 
	end
	);

// class type-in block
static ParamBlockDesc2 grid_typein_blk ( grid_type_in, _T("GridTypeIn"),  0, &gridObjDesc, P_CLASS_PARAMS + P_AUTO_UI, 
	//rollout
	IDD_GRIDPARAM3, IDS_PW_KEYBOARDENTRY, BEGIN_EDIT_CREATE, APPENDROLL_CLOSED, NULL,
	// params
	grid_ti_pos, 		_T("typeInPos"), 		TYPE_POINT3, 		0, 	IDS_PW_POS,
		p_default, 		Point3(0.0f,0.0f,0.0f), 
		p_range, 		-99999999.0, 99999999.0, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_TI_POSX, IDC_TI_POSXSPIN, IDC_TI_POSY, IDC_TI_POSYSPIN, IDC_TI_POSZ, IDC_TI_POSZSPIN, SPIN_AUTOSCALE, 
		end, 
	grid_ti_length, 	_T("typeInLength"), 	TYPE_FLOAT, 		0, 	IDS_PW_LENGTH, 
		p_default, 		25.0, 
		p_range, 		0.0f, 999999999.9f, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_LENGTHEDIT, IDC_LENSPINNER, SPIN_AUTOSCALE, 
		end, 
	grid_ti_width, 		_T("typeInWidth"), 		TYPE_FLOAT, 		0, 	IDS_PW_WIDTH, 
		p_default, 		25.0, 
		p_range, 		0.0f, 999999999.9f, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_WIDTHEDIT, IDC_WIDTHSPINNER, SPIN_AUTOSCALE, 
		end, 
	end
	);

// JBW: this descriptor defines the main per-instance parameter block.  It is flagged as AUTO_CONSTRUCT which
//      means that the CreateInstance() will automatically create one of these blocks and set it to the reference
//      number given (0 in this case, as seen at the end of the line).

// per instance gridsphere block
static ParamBlockDesc2 grid_param_blk ( grid_params, _T("GridParameters"),  0, &gridObjDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF_NO, 
	//rollout
	IDD_GRIDPARAM2, IDS_PW_PARAMETERS, 0, 0, NULL,
	// params
	grid_length,  _T("length"), 			TYPE_FLOAT, 	P_ANIMATABLE + P_RESET_DEFAULT, 	IDS_PW_LENGTH, 
		p_default, 		0.0,	
		p_ms_default,	25.0,
		p_range, 		0.0f, 999999999.9f, 
		p_dim,			stdWorldDim,
		p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_LENGTHEDIT, IDC_LENSPINNER, SPIN_AUTOSCALE, 
		end, 
	grid_width,  _T("width"), 			TYPE_FLOAT, 	P_ANIMATABLE + P_RESET_DEFAULT, 	IDS_PW_WIDTH, 
		p_default, 		0.0,	
		p_ms_default,	25.0,
		p_range, 		0.0f, 999999999.9f, 
		p_dim,			stdWorldDim,
		p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_WIDTHEDIT, IDC_WIDTHSPINNER, SPIN_AUTOSCALE, 
		end, 
	grid_lsegs, 	_T("lengthsegs"), 			TYPE_INT, 		P_ANIMATABLE, 	IDS_PW_LSEGS, 
		p_default, 		4, 
		p_range, 		1, 1000, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_INT, IDC_LSEGS, IDC_LSEGSPIN, SPIN_AUTOSCALE, 
		end, 
	grid_wsegs, 	_T("widthsegs"), 			TYPE_INT, 		P_ANIMATABLE, 	IDS_PW_WSEGS, 
		p_default, 		4, 
		p_range, 		1, 1000, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_INT, IDC_WSEGS, IDC_WSEGSPIN, SPIN_AUTOSCALE, 
		end, 
#ifndef NO_OUTPUTRENDERER
	grid_rsegs, 	_T("density"), 			TYPE_FLOAT, 		P_ANIMATABLE, 	IDS_PW_RSEG, 
		p_default, 		1.0f, 
		p_range, 		1.0f, 9999999.9f, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_RENDERSEGEDIT, IDC_RENDERSEGSPINNER, SPIN_AUTOSCALE, 
		end, 

	grid_rscale, 	_T("renderScale"), 			TYPE_FLOAT, 		P_ANIMATABLE, 	IDS_PW_SCALE, 
		p_default, 		1.0f, 
		p_range, 		1.0f, 9999999.9f, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_SCALEEDIT, IDC_SCALESPINNER, SPIN_AUTOSCALE, 
		end, 
#endif // NO_OUTPUTRENDERER
	grid_genuvs, 	_T("mapCoords"), 	TYPE_BOOL, 		0,				IDS_PW_MAPPING,
		p_default, 		TRUE, 
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_GENTEXTURE, 
		end, 

/*	grid_leftover, 	_T("oldVersionParam"),		TYPE_FLOAT, 0, 	IDS_PW_LEFTOVER, 
		end, 
	grid_leftover1, 	_T("oldVersionParam"),		TYPE_FLOAT, 0, 	IDS_PW_LEFTOVER, 
		end, 
*/

	end
	);


#define PARAMDESC_LENGH 9


ParamBlockDescID griddescVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, grid_length },
	{ TYPE_FLOAT, NULL, TRUE, grid_width },
	{ TYPE_FLOAT, NULL, TRUE, -1 }, 
	{ TYPE_INT, NULL, TRUE, grid_wsegs }, 
	{ TYPE_INT, NULL, TRUE, grid_lsegs }, 
	{ TYPE_INT, NULL, TRUE, -1 },
	{ TYPE_INT, NULL, FALSE, grid_genuvs } 
	};

ParamBlockDescID griddescVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, grid_length },
	{ TYPE_FLOAT, NULL, TRUE, grid_width },
	{ TYPE_FLOAT, NULL, FALSE, -1 }, 
	{ TYPE_INT, NULL, TRUE, grid_wsegs }, 
	{ TYPE_INT, NULL, TRUE, grid_lsegs }, 
	{ TYPE_INT, NULL, FALSE, -1 },
	{ TYPE_INT, NULL, FALSE, grid_genuvs }, 
	{ TYPE_INT, NULL, TRUE, grid_rsegs } 
	};

ParamBlockDescID griddescVer2[] = {
	{ TYPE_FLOAT, NULL, TRUE, grid_length },
	{ TYPE_FLOAT, NULL, TRUE, grid_width },
	{ TYPE_FLOAT, NULL, FALSE, -1 }, 
	{ TYPE_INT, NULL, TRUE,  grid_wsegs}, 
	{ TYPE_INT, NULL, TRUE, grid_lsegs }, 
	{ TYPE_INT, NULL, FALSE,  -1},
	{ TYPE_INT, NULL, FALSE, grid_genuvs }, 
	{ TYPE_FLOAT, NULL, TRUE, grid_rsegs }, 
	{ TYPE_FLOAT, NULL, TRUE, grid_rscale }, 
	};


#define PBLOCK_LENGTH	9

// Array of old versions
static ParamVersionDesc versions[] = {
 	ParamVersionDesc(griddescVer0,7,1),	
 	ParamVersionDesc(griddescVer1,8,2),	
 	ParamVersionDesc(griddescVer2,9,3),	
	};
#define NUM_OLDVERSIONS	3

//#define CURRENT_VERSION	3
//static ParamVersionDesc curVersion(descVer2,PBLOCK_LENGTH,CURRENT_VERSION);


//--- TypeInDlgProc --------------------------------

class GridObjTypeInDlgProc : public ParamMap2UserDlgProc {
	public:
		GridObject *ob;

		GridObjTypeInDlgProc(GridObject *o) {ob=o;}
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

BOOL GridObjTypeInDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_TI_CREATE: {					
					// We only want to set the value if the object is 
					// not in the scene.
					if (ob->TestAFlag(A_OBJ_CREATING)) {
						ob->pblock2->SetValue(grid_length,0,grid_typein_blk.GetFloat(grid_ti_length));
						ob->pblock2->SetValue(grid_width,0,grid_typein_blk.GetFloat(grid_ti_width));
						}
					ob->typeinCreate = TRUE;
					ob->crtWidth = grid_typein_blk.GetFloat(grid_ti_width);
					ob->crtLength = grid_typein_blk.GetFloat(grid_ti_length);

					ob->crtPos = grid_typein_blk.GetPoint3(grid_ti_pos);
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


GridObject::GridObject()
	{
//	typeinCreate = FALSE;
	GetGridobjDesc()->MakeAutoParamBlocks(this);
	}



int GridObject::RenderBegin(TimeValue t, ULONG flags)
	{
	SetAFlag(A_RENDER);
	float rscale, rseg;
	pblock2->GetValue(grid_rscale,t,rscale,ivalid);
	pblock2->GetValue(grid_rsegs,t,rseg,ivalid);
	if (rscale > 1.0f || rseg > 1.0f) {
		ivalid.SetEmpty();
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}
	return 0;
	}

int GridObject::RenderEnd(TimeValue t)
	{
	ClearAFlag(A_RENDER);
	float rscale, rseg;
	pblock2->GetValue(grid_rscale,t,rscale,ivalid);
	pblock2->GetValue(grid_rsegs,t,rseg,ivalid);
	if (rscale > 1.0f || rseg > 1.0f) {
		ivalid.SetEmpty();
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}
	return 0;

	}


void GridObject::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
	{
	SimpleObject::BeginEditParams(ip, flags, prev);
	this->ip = ip;
	// throw up all the appropriate auto-rollouts
	gridObjDesc.BeginEditParams(ip, this, flags, prev);
	// install a callback for the type in.
	grid_typein_blk.SetUserDlgProc(new GridObjTypeInDlgProc(this));
	if (typeinCreate)
		{
		pblock2->SetValue(grid_length,0,crtLength);
		pblock2->SetValue(grid_width,0,crtWidth);
		typeinCreate = FALSE;
		}

	Interval ivalid;
	int lsegs, wsegs;
	float rseg;
	pblock2->GetValue(grid_lsegs,ip->GetTime(),lsegs,ivalid);
	pblock2->GetValue(grid_wsegs,ip->GetTime(),wsegs,ivalid);
#ifndef NO_OUTPUTRENDERER
	pblock2->GetValue(grid_rsegs,ip->GetTime(),rseg,ivalid);
#else
	rseg = 1.0f;
#endif

	if (lsegs < 1) lsegs = 1;
	if (wsegs < 1) lsegs = 1;
	if (rseg < 1.0f) rseg = 1.0f;

	int tlsegs = (int) ((float) lsegs * rseg);
	int twsegs = (int) ((float) wsegs * rseg);

	int totalCount = tlsegs*twsegs *2;
	IParamMap2 *ipm = pblock2->GetMap();
	if (ipm)
		{
		HWND hWnd = ipm->GetHWnd();

		TSTR string;
		string.printf("%d",totalCount);
	
		SetDlgItemText(hWnd, IDC_TOTALFACES, string);
		}

	}
		
void GridObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
	{
	SimpleObject::EndEditParams(ip, flags, next);
	this->ip = NULL;
	// tear down the appropriate auto-rollouts
	gridObjDesc.EndEditParams(ip, this, flags, next);
	}
// vertices ( a b c d ) are in counter clockwise order when viewd from 
// outside the surface unless bias!=0 in which case they are clockwise
static void MakeQuad(int nverts, Face *f, int a, int b , int c , int d, int sg, int bias) {
	int sm = 1;
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

int griddirection(Point3 *v) {
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


BOOL GridObject::HasUVW() { 
     BOOL genUVs;
     Interval v;
     pblock2->GetValue(grid_genuvs, 0, genUVs, v);
     return genUVs; 
}

void GridObject::SetGenUVW(BOOL sw) {  
     if (sw==HasUVW()) return;
     pblock2->SetValue(grid_genuvs,0, sw);				
}



void GridObject::BuildMesh(TimeValue t)
	{
	int ix,iy,iz,nf,kv,mv,nlayer,topStart,midStart;
	int nverts,wsegs,lsegs,hsegs,nv,nextk,nextm,wsp1;
	int nfaces;
	Point3 va,vb,p;
	float l, w, h;
	int genUVs = 1;
	BOOL bias = 0;
	float rscale, rseg;

	// Start the validity interval at forever and widdle it down.
	ivalid = FOREVER;	
	pblock2->GetValue(grid_lsegs,t,lsegs,ivalid);
	pblock2->GetValue(grid_wsegs,t,wsegs,ivalid);
	pblock2->GetValue(grid_genuvs,t,genUVs,ivalid);
#ifndef NO_OUTPUTRENDERER
	pblock2->GetValue(grid_rscale,t,rscale,ivalid);
	pblock2->GetValue(grid_rsegs,t,rseg,ivalid);
#else
	rscale = rseg = 1.0f;
#endif
	pblock2->GetValue(grid_length,t,l,ivalid);
	pblock2->GetValue(grid_width,t,w,ivalid);

	if (lsegs < 1) lsegs = 1;
	if (wsegs < 1) lsegs = 1;


	bias = 1;
	if (rscale <= 1.0f) rscale = 1.0f;
	if (rseg <= 1.0f) rseg = 1.0f;
//check if rendering and if so scale accordingly	
    if (TestAFlag(A_RENDER))
		{
		lsegs = (int) ((float) lsegs * rseg);
		wsegs = (int) ((float) wsegs * rseg);
		l =  ((float)l * rscale);
		w =  ((float)w * rscale);
		}
	else
		{
		long tlsegs = (long) ((float) lsegs * rseg);
		long twsegs = (long) ((float) wsegs * rseg);

		long totalCount = tlsegs*twsegs *2;
		
		IParamMap2 *ipm = pblock2->GetMap();
		if (ipm)
			{
			HWND hWnd = ipm->GetHWnd();

			TSTR string;
			string.printf("%d",totalCount);

			SetDlgItemText(hWnd, IDC_TOTALFACES, string);
			}

		}

//	LimitValue(lsegs, MIN_SEGMENTS, MAX_SEGMENTS);
//	LimitValue(wsegs, MIN_SEGMENTS, MAX_SEGMENTS);
//	LimitValue(hsegs, 0, MAX_SEGMENTS);

	// Number of verts
      // bottom : (lsegs+1)*(wsegs+1)
	  // top    : (lsegs+1)*(wsegs+1)
	  // sides  : (2*lsegs+2*wsegs)*(hsegs-1)

	// Number of rectangular faces.
      // bottom : (lsegs)*(wsegs)
	  // top    : (lsegs)*(wsegs)
	  // sides  : 2*(hsegs*lsegs)+2*(wsegs*lsegs)
	hsegs = 0;
	if (hsegs == 0)
	{

	wsp1 = wsegs + 1;
	nlayer  =  2*(lsegs+wsegs);
	topStart = (lsegs+1)*(wsegs+1);
	midStart = 2*topStart;

	nverts = (lsegs+1)*(wsegs+1);
	nfaces = 2*(lsegs*wsegs + hsegs*lsegs + wsegs*hsegs);

	BOOL notAllocated;
	notAllocated = mesh.setNumVerts(nverts);
//195843 watje to handle out of memory problem on dense meshes
	if (!notAllocated)
		{
		mesh.setNumVerts(0);
		mesh.setNumFaces(0);
		TSTR buf2 = GetString(IDS_PW_PLANE_ERROR);
		TSTR buf1 = GetString(IDS_PW_PLANE_MEMORY_ERROR);
		int res = MessageBox(GetCOREInterface()->GetMAXHWnd(),buf1,buf2,MB_OK| MB_ICONEXCLAMATION |MB_TASKMODAL);
		return;
		}

	notAllocated = mesh.setNumFaces(nfaces);
//195843 watje to handle out of memory problem on dense meshes
	if (!notAllocated)
		{
		mesh.setNumVerts(0);
		mesh.setNumFaces(0);
		TSTR buf2 = GetString(IDS_PW_PLANE_ERROR);
		TSTR buf1 = GetString(IDS_PW_PLANE_MEMORY_ERROR);
		int res = MessageBox(GetCOREInterface()->GetMAXHWnd(),buf1,buf2,MB_OK| MB_ICONEXCLAMATION |MB_TASKMODAL);
		return;
		}

	nv = 0;
	h = 0.0f;
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


	if (genUVs) {
		int ls = lsegs+1;
		int ws = wsegs+1;
		int hs = hsegs+1;
		int ntverts = ls*hs + hs*ws + ws*ls ;
		notAllocated = mesh.setNumTVerts( ntverts ) ;
//195843 watje to handle out of memory problem on dense meshes
		if (!notAllocated)
			{
			mesh.setNumTVFaces(0);	
			mesh.setNumTVerts( 0 ) ;
			TSTR buf2 = GetString(IDS_PW_PLANE_ERROR);
			TSTR buf1 = GetString(IDS_PW_PLANE_MEMORY_ERROR);
			int res = MessageBox(GetCOREInterface()->GetMAXHWnd(),buf1,buf2,MB_OK| MB_ICONEXCLAMATION |MB_TASKMODAL);
			mesh.InvalidateGeomCache();
			return;
			}

		notAllocated = mesh.setNumTVFaces(nfaces);	
//195843 watje to handle out of memory problem on dense meshes
		if (!notAllocated)
			{
			mesh.setNumTVFaces(0);	
			mesh.setNumTVerts( 0 ) ;
			TSTR buf2 = GetString(IDS_PW_PLANE_ERROR);
			TSTR buf1 = GetString(IDS_PW_PLANE_MEMORY_ERROR);
			int res = MessageBox(GetCOREInterface()->GetMAXHWnd(),buf1,buf2,MB_OK| MB_ICONEXCLAMATION |MB_TASKMODAL);
			mesh.InvalidateGeomCache();
			return;
			}
		
		float uoff = 0.0f,voff=0.0f,umult= 1.0f,vmult=1.0f;

		if ((rscale > 1.0f) && (!TestAFlag(A_RENDER)) )

			{
			uoff= ((rscale -1.0f)/2.0f) / (float)rscale ;
			voff= ((rscale -1.0f)/2.0f) / (float)rscale ;
			umult = 1.0f/(float)rscale;
			vmult = 1.0f/(float)rscale;
			}


		int xbase = 0;
		int ybase = ls*hs;
		int zbase = ls*hs + hs*ws;
	
		float dw = 1.0f/float(wsegs);
		float dl = 1.0f/float(lsegs);
//		float dh = 1.0f/float(hsegs);
		float dh = 1.0f;

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
				mesh.setTVert(nv, u * umult + uoff, v * vmult + voff, 0.0f);
				nv++; u+=dl;
				}
			v += dh;
			}

		v = 0.0f; 
		//Y Axis face
		for (iz =0; iz<hs; iz++) {
			u = 0.0f;
			for (ix =0; ix<ws; ix++) {
				mesh.setTVert(nv, u * umult + uoff, v * vmult + voff, 0.0f);
				nv++; u+=dw;
				}
			v += dh;
			}

		v = 0.0f; 
		for (iy =0; iy<ls; iy++) {
			u = 0.0f; 
			for (ix =0; ix<ws; ix++) {
				mesh.setTVert(nv, u * umult + uoff, v * vmult + voff, 0.0f);
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
			int dir = griddirection(v);
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
			for (ix =0; ix<3; ix++)
				v[ix] = mesh.getVert(nv[ix]);
			int dir = griddirection(v);
			mesh.setFaceMtlIndex(nf,mapDir[dir]);
			}
		}
 
	mesh.InvalidateGeomCache();


	}
	else
	{
	wsp1 = wsegs + 1;
	nlayer  =  2*(lsegs+wsegs);
	topStart = (lsegs+1)*(wsegs+1);
	midStart = 2*topStart;

	nverts = midStart+nlayer*(hsegs-1);
	nfaces = 4*(lsegs*wsegs + hsegs*lsegs + wsegs*hsegs);

	mesh.setNumVerts(nverts);
	mesh.setNumFaces(nfaces);

	nv = 0;

	h = 0.0f;
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
		// do LEFT faces -----------------------
		kv = 0;
		mv = topStart;
		for (ix=0; ix<wsegs; ix++) {
			MAKE_QUAD(kv, kv+1, mv+1, mv, 5, bias);
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
			int dir = griddirection(v);
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
			int dir = griddirection(v);
			mesh.setFaceMtlIndex(nf,mapDir[dir]);
			}
		}
 
	mesh.InvalidateGeomCache();
	}
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

void GridObject::BuildGridPatch(TimeValue t,
		PatchMesh &amesh
		)
	{
	int ix,iy,np,kv;
	int wsegs,lsegs,nv;
	Point3 v,p;
	float l, w;
	int tex;
	
	// Start the validity interval at forever and widdle it down.
	ivalid = FOREVER;
/*
	pblock->GetValue( PB_LENGTH, t, l, ivalid );
	pblock->GetValue( PB_WIDTH, t, w, ivalid );
	pblock->GetValue( PB_LSEGS, t, lsegs, ivalid );
	pblock->GetValue( PB_WSEGS, t, wsegs, ivalid );
	pblock->GetValue( PB_TEXTURE, t, tex, ivalid );
*/
	pblock2->GetValue(grid_lsegs,t,lsegs,ivalid);
	pblock2->GetValue(grid_wsegs,t,wsegs,ivalid);
	pblock2->GetValue(grid_genuvs,t,tex,ivalid);
	pblock2->GetValue(grid_length,t,l,ivalid);
	pblock2->GetValue(grid_width,t,w,ivalid);


	int lv = lsegs + 1;
	int wv = wsegs + 1;

	int nverts = lv * wv;
	int npatches = lsegs * wsegs;
	int nexteriors = npatches * 4 + lsegs * 2 + wsegs * 2;
	int ninteriors = npatches * 4;
	int nvecs = ninteriors + nexteriors;

	amesh.setNumVerts(nverts);
	amesh.setNumTVerts(tex ? nverts : 0);
	amesh.setNumVecs(nvecs);
	amesh.setNumPatches(npatches);
	amesh.setNumTVPatches(tex ? npatches : 0);

	v =  Point3(-w, -l, 0.0f) / 2.0f;   

	float dx = w/wsegs;
	float dy = l/lsegs;
	float fws = (float)wsegs;
	float fls = (float)lsegs;

	// Create the vertices.
	nv = 0;
	p.z = v.z;
	p.y = v.y;
	for(iy=0; iy<=lsegs; iy++) {
		p.x = v.x;
		for (ix=0; ix<=wsegs; ix++) {
			if(tex)
				amesh.setTVert(nv, UVVert((float)ix / fws, (float)iy / fls, 0.0f));
			amesh.verts[nv].flags = PVERT_COPLANAR;
			amesh.setVert(nv++, p);
			p.x += dx;
			}
		p.y += dy;
		}

	// Create patches.
	np = 0;
	int interior = nexteriors;
	int vecRowInc = lsegs * 2;
	int vecColInc = wsegs * 2;
	for(iy=0; iy<lsegs; iy++) {
		kv = iy*(wsegs+1);
		int rv = iy * vecColInc;	// Row vector start
		int cv = vecColInc * lv + iy * 2;	// column vector start
		for (ix=0; ix<wsegs; ix++,++np) {
			Patch &p = amesh.patches[np];
			int a = kv, b = kv+1, c = kv+wsegs+2, d = kv + wsegs + 1;
			int ab = rv, ba = rv+1;
			int bc = cv+vecRowInc, cb = cv + vecRowInc + 1;
			int cd = rv+vecColInc+1, dc = rv+vecColInc;
			int da = cv + 1, ad = cv;
			amesh.MakeQuadPatch(np, a, ab, ba, b, bc, cb, c, cd, dc, d, da, ad, interior, interior+1, interior+2, interior+3, 1);
			if(tex)
				amesh.getTVPatch(np).setTVerts(a,b,c,d);
			// Create the default vectors
			Point3 pa = amesh.getVert(a).p;
			Point3 pb = amesh.getVert(b).p;
			Point3 pc = amesh.getVert(c).p;
			Point3 pd = amesh.getVert(d).p;
			amesh.setVec(ab, pa + (pb - pa) / 3.0f);
			amesh.setVec(ba, pb - (pb - pa) / 3.0f);
			amesh.setVec(bc, pb + (pc - pb) / 3.0f);
			amesh.setVec(cb, pc - (pc - pb) / 3.0f);
			amesh.setVec(cd, pc + (pd - pc) / 3.0f);
			amesh.setVec(dc, pd - (pd - pc) / 3.0f);
			amesh.setVec(da, pd + (pa - pd) / 3.0f);
			amesh.setVec(ad, pa - (pa - pd) / 3.0f);
			kv++;
			cv += vecRowInc;
			rv += 2;
			interior += 4;
			}
		}
	// Verify that we have the right number of parts!
	assert(np==npatches);
	assert(nv==nverts);
	// Finish up patch internal linkages (and bail out if it fails!)
	assert(amesh.buildLinkages());
	// Calculate the interior bezier points on the PatchMesh's patches
	amesh.computeInteriors();
	amesh.InvalidateGeomCache();

/*
	int nverts = 8;
	int nvecs = 48;
	int npatches = 6;
	patch.setNumVerts(nverts);	
	patch.setNumTVerts(textured ? 4 : 0);
	patch.setNumVecs(nvecs);
	patch.setNumPatches(npatches);
//	patch.setNumTVPatches(textured ? npatches : 0);

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
		if(textured) {
//			TVPatch &tp = patch.tvPatches[px];
//			tp.setTVerts(2,3,0,1);
			}
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
	if(textured) {
//		TVPatch &tp = patch.tvPatches[px];
//		tp.setTVerts(0,1,2,3);
		}
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
	if(textured) {
//		TVPatch &tp = patch.tvPatches[px];
//		tp.setTVerts(2,3,0,1);
		}
	ix+=4;
	px++;

	assert(patch.buildLinkages());
	patch.computeInteriors();
	patch.InvalidateGeomCache();
*/
	}

#ifndef NO_NURBS

Object * GridObject::BuildNURBSPlane(TimeValue t)
{

	int wsegs,lsegs;
	float l, w;
	int tex;
	NURBSSet nset;
	
	// Start the validity interval at forever and widdle it down.
	Interval ivalid = FOREVER;
	pblock2->GetValue(grid_lsegs,t,lsegs,ivalid);
	pblock2->GetValue(grid_wsegs,t,wsegs,ivalid);
	pblock2->GetValue(grid_genuvs,t,tex,ivalid);
	pblock2->GetValue(grid_length,t,l,ivalid);
	pblock2->GetValue(grid_width,t,w,ivalid);
	lsegs = 4;
	wsegs = 4;

	NURBSCVSurface *s = new NURBSCVSurface();
	nset.AppendObject(s);
	s->SetNumCVs(lsegs, wsegs);

	s->SetUOrder(4);
	s->SetVOrder(4);
	s->SetNumUKnots(8);
	s->SetNumVKnots(8);
	for (int k = 0; k < 4; k++) {
		s->SetUKnot(k, 0.0);
		s->SetVKnot(k, 0.0);
		s->SetUKnot(k+4, 1.0);
		s->SetVKnot(k+4, 1.0);
	}

	NURBSControlVertex cv;

	Point3 v,p;
	v =  Point3(-w, -l, 0.0f) / 2.0f;   

	float dx = w/(wsegs-1.0f);
	float dy = l/(lsegs-1.0f);
	float fws = (float)wsegs;
	float fls = (float)lsegs;

	// Create the vertices.
	int nv = 0;
	p.z = v.z;
	p.y = v.y;
	for(int iy=0; iy<lsegs; iy++) {
		p.x = v.x;
		for (int ix=0; ix<wsegs; ix++) {
//			if(tex)
//				amesh.setTVert(nv, UVVert((float)ix / fws, (float)iy / fls, 0.0f));
//			amesh.verts[nv].flags = PVERT_COPLANAR;
//			amesh.setVert(nv++, p);

			cv.SetPosition(0,  p);
			char name[20];
			sprintf(name, "%s[%d,%d]", GetString(IDS_PW_CV), ix, iy);
			cv.SetName(name);
			s->SetCV(ix, iy, cv);

			p.x += dx;
			}
		p.y += dy;
		}
/*
	for (int u = 0; u < 4; u++) {
		float up = 100.0f * ((float)u/3.0f);
		for (int v = 0; v < 4; v++) {
			float vp = 100.0f * ((float)v/3.0f);
//			cv.SetPosition(0,  Point3(-150.0f + up, -100.0f + vp, 0.0f));
			cv.SetPosition(0,  Point3(up, vp, 0.0f));
			char name[20];
//			sprintf(name, "%s[%d,%d]", GetString(IDS_CV), u, v);
			cv.SetName(name);
			s->SetCV(u, v, cv);
		}
	}
*/
	if (tex)
		{
		s->SetTextureUVs(0, 0, Point2(1.0f, 0.0f));
		s->SetTextureUVs(0, 1, Point2(0.0f, 0.0f));
		s->SetTextureUVs(0, 2, Point2(1.0f, 1.0f));
		s->SetTextureUVs(0, 3, Point2(0.0f, 1.0f));
		}
	Matrix3 mat;
	mat.IdentityMatrix();
	Object *ob = CreateNURBSObject(NULL, &nset, mat);
	return ob;


/*
	NURBSSet nset;

	Point3 center(0, 0, 0);
	Point3 northAxis(0, 0, 1);
	Point3 refAxis(0, -1, 0);

	float startAngleU = -PI;
	float endAngleU = PI;
	float startAngleV;
	if (hemi)
		startAngleV = 0.0f;
	else
		startAngleV = -HALFPI;
	float endAngleV = HALFPI;
	if (recenter) {
		if (hemi)
			center = Point3(0.0f, 0.0f, 0.0f);
		else
			center = Point3(0.0f, 0.0f, radius);
	}

	NURBSCVSurface *surf = new NURBSCVSurface();
	nset.AppendObject(surf);
	surf->SetGenerateUVs(genUVs);

	if (hemi) {
		surf->SetTextureUVs(0, 0, Point2(0.0f, 0.5f));
		surf->SetTextureUVs(0, 1, Point2(0.0f, 1.0f));
		surf->SetTextureUVs(0, 2, Point2(1.0f, 0.5f));
		surf->SetTextureUVs(0, 3, Point2(1.0f, 1.0f));
	} else {
		surf->SetTextureUVs(0, 0, Point2(0.0f, 0.0f));
		surf->SetTextureUVs(0, 1, Point2(0.0f, 1.0f));
		surf->SetTextureUVs(0, 2, Point2(1.0f, 0.0f));
		surf->SetTextureUVs(0, 3, Point2(1.0f, 1.0f));
	}

	surf->FlipNormals(TRUE);
	surf->Renderable(TRUE);
	char bname[80];
	char sname[80];
	strcpy(bname, GetString(IDS_GEOSPHERE));
	sprintf(sname, "%s%s", bname, GetString(IDS_CT_SURF));
	surf->SetName(sname);
	GenNURBSSphereSurface(radius, center, northAxis, refAxis, 
					startAngleU, endAngleU, startAngleV, endAngleV,
					FALSE, *surf);

	if (hemi) {
		// now create caps on the ends
		NURBSCapSurface *cap0 = new NURBSCapSurface();
		nset.AppendObject(cap0);
		cap0->SetGenerateUVs(genUVs);
		cap0->SetParent(0);
		cap0->SetEdge(0);
		cap0->FlipNormals(TRUE);
		cap0->Renderable(TRUE);
		char sname[80];
		sprintf(sname, "%s%s%", bname, GetString(IDS_CT_CAP));
		cap0->SetName(sname);
	}

	Matrix3 mat;
	mat.IdentityMatrix();
	Object *ob = CreateNURBSObject(NULL, &nset, mat);
	return ob;
*/
}

#endif

Object* GridObject::ConvertToType(TimeValue t, Class_ID obtype)
	{
#ifndef NO_PATCHES
	if (obtype == patchObjectClassID) {
		Interval valid = FOREVER;
		float length, width, height;
		int genUVs;
		pblock2->GetValue(grid_length,t,length,valid);
		pblock2->GetValue(grid_width,t,width,valid);
		height = 0.0f;
		pblock2->GetValue(grid_genuvs,t,genUVs,valid);
		PatchObject *ob = new PatchObject();
		BuildGridPatch(t,ob->patch);
		ob->SetChannelValidity(TOPO_CHAN_NUM,valid);
		ob->SetChannelValidity(GEOM_CHAN_NUM,valid);
		ob->UnlockObject();
		return ob;
    }
#endif
#ifndef NO_NURBS
	if (obtype == EDITABLE_SURF_CLASS_ID) {
		Interval valid = FOREVER;
		Object *ob = BuildNURBSPlane(t);
		ob->SetChannelValidity(TOPO_CHAN_NUM, valid);
		ob->SetChannelValidity(GEOM_CHAN_NUM, valid);
		ob->UnlockObject();
		return ob;

	} 
#endif

    return SimpleObject::ConvertToType(t,obtype);
	}

int GridObject::CanConvertToType(Class_ID obtype)
    {
#ifndef NO_PATCHES
	if (obtype == patchObjectClassID)
		return 1;
#endif
#ifndef NO_NURBS
	if (obtype == EDITABLE_SURF_CLASS_ID)
		return 1;
#endif
    return SimpleObject::CanConvertToType(obtype);
	}


void GridObject::GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist)
{
    Object::GetCollapseTypes(clist, nlist);
#ifndef NO_NURBS
    Class_ID id = EDITABLE_SURF_CLASS_ID;
    TSTR *name = new TSTR(GetString(IDS_SM_NURBS_SURFACE));
    clist.Append(1,&id);
    nlist.Append(1,&name);
#endif
}


class GridObjCreateCallBack: public CreateMouseCallBack {
	GridObject *ob;
	Point3 p0,p1;
	IPoint2 sp0, sp1;
	BOOL square;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		void SetObj(GridObject *obj) { ob = obj; }
	};

int GridObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
	Point3 d;
	if (msg == MOUSE_FREEMOVE)
		{
		vpt->SnapPreview(m,m,NULL, SNAP_IN_3D);
		}
	else if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {
			case 0:

				GetCOREInterface()->SetHideByCategoryFlags(GetCOREInterface()->GetHideByCategoryFlags() & ~HIDE_OBJECTS);

				sp0 = m;
				
				ob->createMeth = grid_crtype_blk.GetInt(grid_create_meth);

				ob->pblock2->SetValue(grid_length,0,0.0f);
				ob->pblock2->SetValue(grid_width,0,0.0f);
				ob->suspendSnap = TRUE;								
				p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
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
				p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
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

				ob->pblock2->SetValue(grid_width,0,float(fabs(d.x)));
				ob->pblock2->SetValue(grid_length,0,float(fabs(d.y)));
//				ob->pmapParam->Invalidate();										

				if (msg==MOUSE_POINT && ob->createMeth) {
					ob->suspendSnap = FALSE;
					return (Length(sp1-sp0)<3)?CREATE_ABORT:CREATE_STOP;					
				} else if (msg==MOUSE_POINT && 
						(Length(sp1-sp0)<3 || Length(d)<0.1f)) {
					return CREATE_ABORT;
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
		return CREATE_ABORT;
		}

	return TRUE;
	}

static GridObjCreateCallBack gridCreateCB;

CreateMouseCallBack* GridObject::GetCreateMouseCallBack() {
	gridCreateCB.SetObj(this);
	return(&gridCreateCB);
	}


BOOL GridObject::OKtoDisplay(TimeValue t) 
	{
	return TRUE;
	}



void GridObject::InvalidateUI() 
	{
	grid_param_blk.InvalidateUI();
	}


RefTargetHandle GridObject::Clone(RemapDir& remap) 
	{
	GridObject* newob = new GridObject();
	newob->ReplaceReference(0,pblock2->Clone(remap));
	newob->ivalid.SetEmpty();	
	BaseClone(this, newob, remap);
	return(newob);
	}



IOResult
GridObject::Load(ILoad *iload) 
{	
	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, NUM_OLDVERSIONS, &grid_param_blk, this, PBLOCK_REF_NO);
	iload->RegisterPostLoadCallback(plcb);
	return IO_OK;
}

#endif // NO_OBJECT_STANDARD_PRIMITIVES

