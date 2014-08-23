/**********************************************************************
 *<
	FILE: VRMLOpt.cpp

	DESCRIPTION:  Optimize modifier

	CREATED BY: Charles Thaeler

	Based on optmod.cpp by Rolf Berteig

	HISTORY: 3/21/95

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#ifdef ZY_OPT

#include "max.h"
//#include "reslib.h"
#include "resource.h"
#include "iparamm.h"
#include "triangle.h"

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;


/* This is the interface function that converts mesh types so we can call
 * Zijiang Yang's code
 */

void
LODOpt(Mesh &mesh, float pct)
{
	int nverts,
		nfaces,
		new_verts = 0,
		new_faces = 0,
		i;
	LodVERTEX *vlist = NULL,
				*new_vlist = NULL;
	LodTRIANGLE *flist = NULL,
				*new_flist = NULL;
	nverts = mesh.getNumVerts();
	nfaces = mesh.getNumFaces();

	vlist = new LodVERTEX[nverts];
	flist = new LodTRIANGLE[nfaces];

	for (i = 0; i < nverts; i++) {
		Point3 v = mesh.verts[i];
		vlist[i].Set(i, v.x, v.y, v.z);
	}

	for (i = 0; i < nfaces; i++) {
		Face f = mesh.faces[i];
		flist[i].Set(i, f.v[0], f.v[1], f.v[2]);
	}

	LodGenerate(vlist, flist, nverts, nfaces, 
			&new_vlist, &new_flist, &new_verts, &new_faces, pct/100.0f);

	if (nfaces != new_faces) {
		// We got the optimization so rebuild the Mesh
		mesh.setNumVerts(new_verts);
		mesh.setNumFaces(new_faces);
		for (i = 0; i < new_verts; i++) {
			LodVECTOR loc = new_vlist[i].GetLoc();
			mesh.setVert(i, Point3(loc[0], loc[1], loc[2]));
		}
		for (i = 0; i < new_faces; i++) {
			int a = new_flist[i].GetVertID(0),
				b = new_flist[i].GetVertID(1),
				c = new_flist[i].GetVertID(2);
			mesh.faces[i].setVerts(a, b, c);
			mesh.faces[i].setEdgeVisFlags(1, 1, 1);
			mesh.faces[i].setSmGroup(0);
		}
	}

	mesh.InvalidateGeomCache();
	mesh.EnableEdgeList(1);

	delete vlist;
	delete flist;
	delete new_vlist;
	delete new_flist;
}


#define OPT_CLASS_ID1 0x7e7a48b1
#define OPT_CLASS_ID2 0x0

class VRMLOpt : public Modifier, public MeshOpProgress {	
	public:
		IParamBlock *pblock;
		static IParamMap *pmapParam;

		VRMLOpt();

		// From Animatable
		void DeleteThis() { delete this; }
		void GetClassName(TSTR& s) { s= _T("VRML Opt"); }  
		virtual Class_ID ClassID() { return Class_ID(OPT_CLASS_ID1,OPT_CLASS_ID2);}		
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip,ULONG flags,Animatable *next);		
		TCHAR *GetObjectName() { return _T("VRML Opt"); }
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 

		ChannelMask ChannelsUsed()  {return OBJ_CHANNELS;}
		ChannelMask ChannelsChanged() {return OBJ_CHANNELS;}
		Class_ID InputType() {return triObjectClassID;}
		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		Interval LocalValidity(TimeValue t);

		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i) {return pblock;}
		void SetReference(int i, RefTargetHandle rtarg) {pblock = (IParamBlock*)rtarg;}

		int NumSubs() {return 1;}
		Animatable* SubAnim(int i) {return pblock;}
		TSTR SubAnimName(int i) {return _T("Parameters"); } //GetString(IDS_RB_PARAMETERS);}

		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);

		// From MeshOpProgress
		void Init(int total);
		BOOL Progress(int p);
	};



//--- ClassDescriptor and class vars ---------------------------------

IParamMap *VRMLOpt::pmapParam = NULL;



class OptClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new VRMLOpt; }
	const TCHAR *	ClassName() { return _T("VRML Opt"); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(OPT_CLASS_ID1,OPT_CLASS_ID2); }
	const TCHAR* 	Category() { return _T("VRBL PLUGIN"); }
	};

static OptClassDesc optDesc;
extern ClassDesc* GetVRMLOptDesc() { return &optDesc; }


//--- Parameter map/block descriptors -------------------------------

#define PB_OPT_PCT			0


// Map indices
#define MAP_OPT_PCT			0

//
//
// Parameters

static ParamUIDesc descParam[] = {
	// Face thresh
	ParamUIDesc(
		PB_OPT_PCT,
		EDITTYPE_POS_FLOAT,
		IDC_OPT_PCT, IDC_OPT_PCT_SPIN,
		0.0f,100.0f,
		1.0f,
		defaultDim),

	};
#define PARAMDESC_LENGH 1


static ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 }
 };
#define PBLOCK_LENGTH	1


#define CURRENT_VERSION	0


//--- OptDlgProc -------------------------------

class OptDlgProc : public ParamMapUserDlgProc {
	public:
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {}
	};
static OptDlgProc theOptProc;


BOOL OptDlgProc::DlgProc(
		TimeValue t,IParamMap *map,
		HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG:
			map->SetPBlockIndex(MAP_OPT_PCT,PB_OPT_PCT);
			break;
		}
	return FALSE;
	}


//--- Opt methods -------------------------------


VRMLOpt::VRMLOpt()
	{	
	MakeRefByID(FOREVER, 0, 
		CreateParameterBlock(descVer0, PBLOCK_LENGTH, CURRENT_VERSION));	
	pblock->SetValue(PB_OPT_PCT,0, 50.0f); // default to 50%
	}

void VRMLOpt::BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev)
	{
	pmapParam = CreateCPParamMap(
		descParam,PARAMDESC_LENGH,
		pblock,
		ip,
		hInstance,
		MAKEINTRESOURCE(IDD_VRMLOPTPARAM),
		_T("Parameters"), //GetString(IDS_RB_PARAMETERS),
		0);	
	pmapParam->SetUserDlgProc(&theOptProc);
	}
		
void VRMLOpt::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
	{	
	DestroyCPParamMap(pmapParam);
	pmapParam = NULL;
	}

Interval VRMLOpt::LocalValidity(TimeValue t)
	{
	float f;		
	Interval valid = FOREVER;
	pblock->GetValue(PB_OPT_PCT,t,f,valid);
	return valid;
	}

RefTargetHandle VRMLOpt::Clone(RemapDir& remap) 
	{
	VRMLOpt* newmod = new VRMLOpt();	
	newmod->ReplaceReference(0,pblock->Clone(remap));	
	BaseClone(this, newmod, remap);
	return newmod;
	}


void VRMLOpt::Init(int total)
	{
	}

BOOL VRMLOpt::Progress(int p)
	{
	SHORT res = GetAsyncKeyState(VK_ESCAPE);
	if (res&1) return FALSE;
	else return TRUE;
	}

void VRMLOpt::ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node)
	{
	float optpct;
	DWORD flags = 0;
	Interval valid = FOREVER;
	int nv,nf;

	pblock->GetValue(PB_OPT_PCT,t,optpct,valid);
	
	assert(os->obj->IsSubClassOf(triObjectClassID));
	TriObject *triOb = (TriObject *)os->obj;
	nv = triOb->mesh.getNumVerts();
	nf = triOb->mesh.getNumFaces();

	if (optpct!=0.0f) {
		GetAsyncKeyState(VK_ESCAPE); // clear the state
		HCURSOR hCur;
		if (nf > 2000) hCur = SetCursor(LoadCursor(NULL,IDC_WAIT));

		/* Here's where we really do the work */
		LODOpt(triOb->mesh, optpct);

		if (nf > 200) SetCursor(hCur);
	}

	triOb->PointsWereChanged();
	triOb->UpdateValidity(GEOM_CHAN_NUM,valid);
	triOb->UpdateValidity(TOPO_CHAN_NUM,valid);	
}

RefResult VRMLOpt::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
   		PartID& partID, 
   		RefMessage message) 
{
	switch (message) {
	case REFMSG_GET_PARAM_DIM: {
		GetParamDim *gpd = (GetParamDim*)partID;
		switch (gpd->index) {
		case PB_OPT_PCT:	gpd->dim = defaultDim; break;				
		default:			gpd->dim = defaultDim; break;
		}			
		return REF_STOP; 
		}

	case REFMSG_GET_PARAM_NAME: {
		GetParamName *gpn = (GetParamName*)partID;
		switch (gpn->index) {
		case PB_OPT_PCT:	gpn->name = _T("Percent Opt"); break;
		default:				gpn->name = TSTR(_T("")); break;
		}
		return REF_STOP; 
	}
	}
	return REF_SUCCEED;
}

#endif



