/**********************************************************************
 *<
	FILE: extrude.cpp

	DESCRIPTION:  Shape->patch extruder

	CREATED BY: Tom Hudson, Dan Silva & Rolf Berteig

	HISTORY: created 10 November, 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#include "mods.h"
#include "resourceOverride.h"

#include "iparamm.h"
#include "shape.h"
#include "spline3d.h"
#include "splshape.h"
#include "linshape.h"
#include "surf_api.h"

//JH 10/28/02 Removing references to solid modeler, the features been removed
#ifdef XXDESIGN_VER
#include "..\..\..\include\igeomimp.h"
//bool BuildSolidFromShape(TimeValue t,ModContext &mc, ObjectState * os, IGeomImp* igeom);
#endif
//--- ExtrudeMod -----------------------------------------------------------

#define MIN_AMOUNT		float(-1.0E30)
#define MAX_AMOUNT		float(1.0E30)

#define DEF_AMOUNT 0.0f
#define DEF_SEGS 1
#define DEF_CAPSTART 1
#define DEF_CAPEND 1
#define DEF_CAPTYPE CAPTYPE_MORPH
#define DEF_MAPPING FALSE
#define DEF_GEN_MATIDS TRUE
#define DEF_USE_SHAPEIDS FALSE
#define DEF_SMOOTH TRUE

#define PATCH_OUTPUT 0
#define MESH_OUTPUT 1
#define NURBS_OUTPUT 2
#define AMSOLID_OUTPUT 3
#define DEF_OUTPUT MESH_OUTPUT //PATCH_OUTPUT

class ExtrudeMod;

// Special dialog handling
class ExtrudeDlgProc : public ParamMapUserDlgProc {
	private:
		ExtrudeMod *mod;
	public:
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void SetMod(ExtrudeMod *m) { mod = m; }
		void DeleteThis() {}
	};

class ExtrudeMod: public Modifier {
	
	protected:
		static IObjParam *ip;
		
	public:
		IParamBlock *pblock;

		static IParamMap *pmapParam;
		static ExtrudeMod *editMod;

		ExtrudeDlgProc dlgProc;

		static float dlgAmount;
		static int dlgSegs;
		static int dlgCapStart;
		static int dlgCapEnd;
		static int dlgCapType;
		static int dlgOutput;
		static int dlgMapping;
		static int dlgGenMatIDs;
		static int dlgUseShapeIDs;
		static int dlgSmooth;

		// From Animatable
		void DeleteThis() { delete this; }
		void GetClassName(TSTR& s) { s= TSTR(_T("ExtrudeMod")); }  
		virtual Class_ID ClassID() { return Class_ID(EXTRUDEOSM_CLASS_ID,0);}		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() { return GetString(IDS_DB_EXTRUDE); }
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		ExtrudeMod();
		virtual ~ExtrudeMod();

		ChannelMask ChannelsUsed()  { return PART_GEOM|PART_TOPO; }
		// Possible GOTCHA -- Modifiers like this one, which completely change the type of
		// object (shape -> Mesh or Patch) change ALL channels!  Be sure to tell the system!
		ChannelMask ChannelsChanged() { return PART_ALL; }
		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		Class_ID InputType() {return genericShapeClassID;}
		Interval LocalValidity(TimeValue t);

		// From BaseObject
		BOOL ChangeTopology() {return TRUE;}
		IParamArray *GetParamBlock() {return pblock;}
		int GetParamBlockIndex(int id) {return id;}

		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i) {return pblock;}
		void SetReference(int i, RefTargetHandle rtarg) {pblock=(IParamBlock*)rtarg;}

 		int NumSubs() { return 1; }  
		Animatable* SubAnim(int i) { return pblock; }
		TSTR SubAnimName(int i) { return TSTR(GetString(IDS_RB_PARAMETERS));}		

		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message);

		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 

		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);

		void BuildMeshFromShape(TimeValue t,ModContext &mc, ObjectState * os, Mesh &mesh, BOOL simple=FALSE);
#ifndef NO_PATCHES
		void BuildPatchFromShape(TimeValue t,ModContext &mc, ObjectState * os, PatchMesh &pmesh);
#endif
#ifdef XXDESIGN_VER
		bool BuildAMSolidFromShape(TimeValue t,ModContext &mc, ObjectState * os, IGeomImp* igeom);
#endif

		void UpdateUI(TimeValue t) {}
		Interval GetValidity(TimeValue t);
		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);

		// Automatic texture support
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);

		// Parameter access
		BOOL GetGenMatIDs();

//watje new mapping ver < 4 of this modifier use the old mapping
		int ver;

	};

BOOL ExtrudeDlgProc::DlgProc(
		TimeValue t,IParamMap *map,
		HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG:
			EnableWindow(GetDlgItem(hWnd,IDC_USE_SHAPE_IDS),mod->GetGenMatIDs());
#ifdef XXDESIGN_VER
			//JH 6/24/99
			//In the design version we may be outputting a solid
			//If so we don't allow changing output types
			int output;
			mod->pblock->GetValue(5/*PB_OUTPUT*/, TimeValue(0), output, FOREVER);
			if(output == AMSOLID_OUTPUT)
			{
			EnableWindow(GetDlgItem(hWnd,IDC_PATCH),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_MESH), FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_NURBS),FALSE);
			}
#endif
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_GEN_MATIDS:
					EnableWindow(GetDlgItem(hWnd,IDC_USE_SHAPE_IDS),mod->GetGenMatIDs());
					break;
				}
			break;
		}
	return FALSE;
	}

class ExtrudeClassDesc:public ClassDesc {
	public:
#ifdef MODIFIER_EXTRUDE_PRIVATE
	int 			IsPublic() { return 0; }
#else
	int 			IsPublic() { return 1; }
#endif
	void *			Create(BOOL loading = FALSE) { return new ExtrudeMod; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_EXTRUDE_CLASS); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return  Class_ID(EXTRUDEOSM_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFDEFORMATIONS);}
	void			ResetClassParams(BOOL fileReset);
	};

static ExtrudeClassDesc extrudeDesc;
extern ClassDesc* GetExtrudeModDesc() { return &extrudeDesc; }

IObjParam*		ExtrudeMod::ip          = NULL;
IParamMap *		ExtrudeMod::pmapParam = NULL;
ExtrudeMod *	ExtrudeMod::editMod = NULL;
float			ExtrudeMod::dlgAmount = DEF_AMOUNT;
int				ExtrudeMod::dlgSegs = DEF_SEGS;
int				ExtrudeMod::dlgCapStart = DEF_CAPSTART;
int				ExtrudeMod::dlgCapEnd = DEF_CAPEND;
int				ExtrudeMod::dlgCapType = DEF_CAPTYPE;
int				ExtrudeMod::dlgOutput = DEF_OUTPUT;
int				ExtrudeMod::dlgMapping = DEF_MAPPING;
int				ExtrudeMod::dlgGenMatIDs = DEF_GEN_MATIDS;
int				ExtrudeMod::dlgUseShapeIDs = DEF_USE_SHAPEIDS;
int				ExtrudeMod::dlgSmooth = DEF_SMOOTH;

void ExtrudeClassDesc::ResetClassParams(BOOL fileReset)
	{
	ExtrudeMod::dlgAmount       = DEF_AMOUNT;
	ExtrudeMod::dlgSegs		    = DEF_SEGS;
	ExtrudeMod::dlgCapStart     = DEF_CAPSTART;
	ExtrudeMod::dlgCapEnd       = DEF_CAPEND;
	ExtrudeMod::dlgCapType      = DEF_CAPTYPE;
	ExtrudeMod::dlgOutput       = DEF_OUTPUT;
	ExtrudeMod::dlgMapping      = DEF_MAPPING;
	ExtrudeMod::dlgGenMatIDs    = DEF_GEN_MATIDS;
	ExtrudeMod::dlgUseShapeIDs  = DEF_USE_SHAPEIDS;
	ExtrudeMod::dlgSmooth		= DEF_SMOOTH;
	}

//--- Parameter map/block descriptors -------------------------------

#define PB_AMOUNT		0
#define PB_SEGS			1
#define PB_CAPSTART		2
#define PB_CAPEND		3
#define PB_CAPTYPE		4
#define PB_OUTPUT		5
#define PB_MAPPING		6
#define PB_GEN_MATIDS	7
#define PB_USE_SHAPEIDS	8
#define PB_SMOOTH		9

//
//
// Parameters

static int captypeIDs[] = {IDC_MORPHCAP,IDC_GRIDCAP};
static int outputIDs[] = {IDC_PATCH,IDC_MESH,IDC_NURBS};

static ParamUIDesc descParam[] = {
	// Amount
	ParamUIDesc(
		PB_AMOUNT,
		EDITTYPE_UNIVERSE,
		IDC_AMOUNT,IDC_AMOUNTSPINNER,
		MIN_AMOUNT,MAX_AMOUNT,
		0.5f),

	// Segments
	ParamUIDesc(
		PB_SEGS,
		EDITTYPE_INT,
		IDC_SEGMENTS,IDC_SEGMENTSPINNER,
		1.0f,100.0f,
		0.5f),

	// Capping start
	ParamUIDesc(PB_CAPSTART,TYPE_SINGLECHEKBOX,IDC_CAPSTART),

	// Capping end
	ParamUIDesc(PB_CAPEND,TYPE_SINGLECHEKBOX,IDC_CAPEND),

	// Cap type
	ParamUIDesc(PB_CAPTYPE,TYPE_RADIO,captypeIDs,2),

	// Output type
	ParamUIDesc(PB_OUTPUT,TYPE_RADIO,outputIDs,3),

	// Texture coords
	ParamUIDesc(PB_MAPPING,TYPE_SINGLECHEKBOX,IDC_GENMAPPING),

	// Multi material IDs
	ParamUIDesc(PB_GEN_MATIDS,TYPE_SINGLECHEKBOX,IDC_GEN_MATIDS),

	// Shape material IDs
	ParamUIDesc(PB_USE_SHAPEIDS,TYPE_SINGLECHEKBOX,IDC_USE_SHAPE_IDS),

	// Smoothing
	ParamUIDesc(PB_SMOOTH,TYPE_SINGLECHEKBOX,IDC_SMOOTH)
	};
#define PARAMDESC_LENGTH 10


static ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_INT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, FALSE, 2 },
	{ TYPE_INT, NULL, FALSE, 3 } };

static ParamBlockDescID descVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_INT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, FALSE, 2 },
	{ TYPE_INT, NULL, FALSE, 3 },
	{ TYPE_INT, NULL, FALSE, 4 } };

static ParamBlockDescID descVer2[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_INT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, FALSE, 2 },
	{ TYPE_INT, NULL, FALSE, 3 },
	{ TYPE_INT, NULL, FALSE, 4 },
	{ TYPE_INT, NULL, FALSE, 5 } };

static ParamBlockDescID descVer3[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_INT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, FALSE, 2 },
	{ TYPE_INT, NULL, FALSE, 3 },
	{ TYPE_INT, NULL, FALSE, 4 },
	{ TYPE_INT, NULL, FALSE, 5 },
	{ TYPE_INT, NULL, FALSE, 6 } };

static ParamBlockDescID descVer4[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_INT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, FALSE, 2 },
	{ TYPE_INT, NULL, FALSE, 3 },
	{ TYPE_INT, NULL, FALSE, 4 },
	{ TYPE_INT, NULL, FALSE, 5 },
	{ TYPE_INT, NULL, FALSE, 6 },
	{ TYPE_INT, NULL, FALSE, 7 } };

static ParamBlockDescID descVer5[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_INT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, FALSE, 2 },
	{ TYPE_INT, NULL, FALSE, 3 },
	{ TYPE_INT, NULL, FALSE, 4 },
	{ TYPE_INT, NULL, FALSE, 5 },
	{ TYPE_INT, NULL, FALSE, 6 },
	{ TYPE_INT, NULL, FALSE, 7 },
	{ TYPE_INT, NULL, FALSE, 8 } };

static ParamBlockDescID descVer6[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_INT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, FALSE, 2 },
	{ TYPE_INT, NULL, FALSE, 3 },
	{ TYPE_INT, NULL, FALSE, 4 },
	{ TYPE_INT, NULL, FALSE, 5 },
	{ TYPE_INT, NULL, FALSE, 6 },
	{ TYPE_INT, NULL, FALSE, 7 },
	{ TYPE_INT, NULL, FALSE, 8 },
	{ TYPE_INT, NULL, FALSE, 9 } };

#define PBLOCK_LENGTH	10

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,4,0),
	ParamVersionDesc(descVer1,5,1),	
	ParamVersionDesc(descVer2,6,2),	
	ParamVersionDesc(descVer3,7,3),	
	ParamVersionDesc(descVer4,8,4),
	ParamVersionDesc(descVer5,9,5)	
	};
#define NUM_OLDVERSIONS	6

// Current version
#define CURRENT_VERSION	6
static ParamVersionDesc curVersion(descVer6,PBLOCK_LENGTH,CURRENT_VERSION);

ExtrudeMod::ExtrudeMod()
	{
//watje new mapping ver < 4 of this modifier use the old mapping
	ver = 4;

	MakeRefByID(FOREVER, 0, 
		CreateParameterBlock(descVer6, PBLOCK_LENGTH, CURRENT_VERSION));
	pblock->SetValue(PB_AMOUNT, TimeValue(0), dlgAmount);
	pblock->SetValue(PB_SEGS, TimeValue(0), dlgSegs);
	pblock->SetValue(PB_CAPSTART, TimeValue(0), dlgCapStart);
	pblock->SetValue(PB_CAPEND, TimeValue(0), dlgCapEnd);
	pblock->SetValue(PB_CAPTYPE, TimeValue(0), dlgCapType);
	pblock->SetValue(PB_OUTPUT, TimeValue(0), dlgOutput);
	pblock->SetValue(PB_MAPPING, TimeValue(0), dlgMapping);
	pblock->SetValue(PB_GEN_MATIDS, TimeValue(0), dlgGenMatIDs);
	pblock->SetValue(PB_USE_SHAPEIDS, TimeValue(0), dlgUseShapeIDs);
	pblock->SetValue(PB_SMOOTH, TimeValue(0), dlgSmooth);
	dlgProc.SetMod(this);
	}

ExtrudeMod::~ExtrudeMod()
	{	
	}

Interval ExtrudeMod::LocalValidity(TimeValue t)
	{
	// if being edited, return NEVER forces a cache to be built 
	// after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;  
	Interval valid = GetValidity(t);	
	return valid;
	}

RefTargetHandle ExtrudeMod::Clone(RemapDir& remap)
	{
	ExtrudeMod* newmod = new ExtrudeMod();	
	newmod->ReplaceReference(0,pblock->Clone(remap));
	BaseClone(this, newmod, remap);
	return(newmod);
	}

void ExtrudeMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) 
	{	
//DebugPrint("Extrude modifying object\n");

	// Get our personal validity interval...
	Interval valid = GetValidity(t);
	// and intersect it with the channels we use as input (see ChannelsUsed)
	valid &= os->obj->ChannelValidity(t,TOPO_CHAN_NUM);
	valid &= os->obj->ChannelValidity(t,GEOM_CHAN_NUM);
	
	int output;
	pblock->GetValue(PB_OUTPUT, TimeValue(0), output, FOREVER);

	switch (output) {
	case NURBS_OUTPUT:
#ifndef NO_NURBS
		{
		// Here is where all the fun stuff happens -- GenerateExtrudeSurface fills in the EM,
		// then we stuff the EditableSurface into the pipeline.
		ShapeObject *shape = (ShapeObject *)os->obj;
			float amount;

		BOOL texturing, genMatIds, useShapeIDs;
		pblock->GetValue(PB_MAPPING, TimeValue(0), texturing, FOREVER);
		pblock->GetValue(PB_GEN_MATIDS, TimeValue(0), genMatIds, FOREVER);
		pblock->GetValue(PB_USE_SHAPEIDS, TimeValue(0), useShapeIDs, FOREVER);

		int levels,capStart,capEnd,capType;

		pblock->GetValue(PB_SEGS,t,levels,FOREVER);
		if (levels<1) levels = 1;
		pblock->GetValue(PB_CAPSTART,t,capStart,FOREVER);
		pblock->GetValue(PB_CAPEND,t,capEnd,FOREVER);
		pblock->GetValue(PB_CAPTYPE,t,capType,FOREVER);

		pblock->GetValue(PB_AMOUNT,t,amount,FOREVER);
		LimitValue(amount, -1000000.0f, 1000000.0f);


		BOOL suspended = FALSE;
		if (theHold.Holding()) {
			theHold.Suspend();
			suspended = TRUE;
		}
		Object *nobj = CreateNURBSExtrudeShape(ip, GetString(IDS_RB_EXTRUDE), t, shape, amount,
								capStart, capEnd, capType, texturing, genMatIds, useShapeIDs);
		if (suspended) {
			theHold.Resume();
		}

        // We only set geom validity because we preserve animation on clone
        // and copying other cahnnels causes problems -- SCM 9/2/97
		nobj->SetChannelValidity(GEOM_CHAN_NUM, valid);

		os->obj = nobj;
		break;}
#endif
#ifndef NO_PATCHES
	case PATCH_OUTPUT: {
		// Here is where all the fun stuff happens -- BuildPatchFromShape fills in the PatchObject's patch mesh,
		// then we stuff the PatchObject into the pipeline.
		PatchObject *pat = new PatchObject;
		BuildPatchFromShape(t, mc, os, pat->patch);

		pat->SetChannelValidity(TOPO_CHAN_NUM, valid);
		pat->SetChannelValidity(GEOM_CHAN_NUM, valid);
		pat->SetChannelValidity(TEXMAP_CHAN_NUM, valid);
		pat->SetChannelValidity(MTL_CHAN_NUM, valid);
		pat->SetChannelValidity(SELECT_CHAN_NUM, valid);
		pat->SetChannelValidity(SUBSEL_TYPE_CHAN_NUM, valid);
		pat->SetChannelValidity(DISP_ATTRIB_CHAN_NUM, valid);

		os->obj = pat;
		break;}
#endif // NO_PATCHES
	case MESH_OUTPUT: {
		// Here is where all the fun stuff happens -- BuildMeshFromShape fills in the TriObject's mesh,
		// then we stuff the TriObj into the pipeline.
		TriObject *tri = CreateNewTriObject();
		BuildMeshFromShape(t, mc, os, tri->GetMesh());

		tri->SetChannelValidity(TOPO_CHAN_NUM, valid);
		tri->SetChannelValidity(GEOM_CHAN_NUM, valid);
		tri->SetChannelValidity(TEXMAP_CHAN_NUM, valid);
		tri->SetChannelValidity(MTL_CHAN_NUM, valid);
		tri->SetChannelValidity(SELECT_CHAN_NUM, valid);
		tri->SetChannelValidity(SUBSEL_TYPE_CHAN_NUM, valid);
		tri->SetChannelValidity(DISP_ATTRIB_CHAN_NUM, valid);

		os->obj = tri;
		break; }
#ifdef XXDESIGN_VER
	case AMSOLID_OUTPUT: {
		//Create an extrusion solid using Facet Modeler
		Object* solid = (Object*)CreateInstance(GEOMOBJECT_CLASS_ID, GENERIC_AMSOLID_CLASS_ID);
		assert(solid);
		if(solid)
		{
			IGeomImp* cacheptr = (IGeomImp*)(solid->GetInterface(I_GEOMIMP));
			assert(cacheptr);
			if(cacheptr)
			{
				bool res = BuildAMSolidFromShape(t, mc, os, cacheptr);
				solid->ReleaseInterface(I_GEOMIMP, cacheptr);
				if(!res)
				{
					valid.SetInstant(t);
//					assert(!cacheptr->isNull());
				}
				for(int i=0; i<NUM_OBJ_CHANS;i++)
					solid->SetChannelValidity(i, valid);
				os->obj = solid;
			}

		}

		break; }
#endif
	}

	os->obj->UnlockObject();
	}


void ExtrudeMod::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
	{
	this->ip = ip;
	editMod = this;

	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);

	pmapParam = CreateCPParamMap(
		descParam,PARAMDESC_LENGTH,
		pblock,
		ip,
		hInstance,
		MAKEINTRESOURCE(IDD_EXTRUDEPARAM),
		GetString(IDS_RB_PARAMETERS),
		0);	
	pmapParam->SetUserDlgProc(&dlgProc);
	}

void ExtrudeMod::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
	{
	this->ip = NULL;
	editMod = NULL;
	
	TimeValue t = ip->GetTime();

	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);

	DestroyCPParamMap(pmapParam);

	// Save these values in class variables so the next object created will inherit them.
	pblock->GetValue(PB_AMOUNT,ip->GetTime(),dlgAmount,FOREVER);
	pblock->GetValue(PB_SEGS,ip->GetTime(),dlgSegs,FOREVER);
	if (dlgSegs<1) dlgSegs = 1;
	pblock->GetValue(PB_CAPSTART,ip->GetTime(),dlgCapStart,FOREVER);
	pblock->GetValue(PB_CAPEND,ip->GetTime(),dlgCapEnd,FOREVER);
	pblock->GetValue(PB_CAPTYPE,ip->GetTime(),dlgCapType,FOREVER);
	pblock->GetValue(PB_OUTPUT,ip->GetTime(),dlgOutput,FOREVER);
	pblock->GetValue(PB_MAPPING,ip->GetTime(),dlgMapping,FOREVER);
	pblock->GetValue(PB_GEN_MATIDS,ip->GetTime(),dlgGenMatIDs,FOREVER);
	pblock->GetValue(PB_USE_SHAPEIDS,ip->GetTime(),dlgUseShapeIDs,FOREVER);
	pblock->GetValue(PB_SMOOTH,ip->GetTime(),dlgSmooth,FOREVER);
	}

Interval ExtrudeMod::GetValidity(TimeValue t)
	{
	float f;
	int i;
	Interval valid = FOREVER;
	pblock->GetValue(PB_AMOUNT,t,f,valid);
	pblock->GetValue(PB_SEGS,t,i,valid);	
	pblock->GetValue(PB_CAPSTART,t,i,valid);
	pblock->GetValue(PB_CAPEND,t,i,valid);
	pblock->GetValue(PB_CAPTYPE,t,i,valid);
	return valid;
	}

RefResult ExtrudeMod::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
   		PartID& partID, 
   		RefMessage message ) 
   	{
	switch (message) {
		case REFMSG_CHANGE:
			if ((editMod==this) && pmapParam) pmapParam->Invalidate();
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

ParamDimension *ExtrudeMod::GetParameterDim(int pbIndex)
	{
	switch (pbIndex) {
		case PB_AMOUNT: 	return stdWorldDim;
		case PB_SEGS:		return defaultDim;
		case PB_CAPSTART:	return defaultDim;
		case PB_CAPEND:		return defaultDim;
		case PB_CAPTYPE:	return defaultDim;
		default:			return defaultDim;
		}
	}

TSTR ExtrudeMod::GetParameterName(int pbIndex)
	{
	switch (pbIndex) {
		case PB_AMOUNT:		return TSTR(GetString(IDS_TH_AMOUNT));
		case PB_SEGS:		return TSTR(GetString(IDS_TH_SEGMENTS));
		default:			return TSTR(_T(""));
		}
	}

BOOL ExtrudeMod::HasUVW() { 
	BOOL genUVs;
	Interval v;
	pblock->GetValue(PB_MAPPING, 0, genUVs, v);
	return genUVs; 
	}

void ExtrudeMod::SetGenUVW(BOOL sw) {  
	if (sw==HasUVW()) return;
	pblock->SetValue(PB_MAPPING,0, sw);				
	}

BOOL ExtrudeMod::GetGenMatIDs() {
	int sw;
	Interval v;
	pblock->GetValue(PB_GEN_MATIDS, 0, sw, v);
	return sw;
	}

class ExtrudePostLoadCallback : public PostLoadCallback {
	public:
		ParamBlockPLCB *cb;
		ExtrudePostLoadCallback(ParamBlockPLCB *c) {cb=c;}
		void proc(ILoad *iload);
	};

void ExtrudePostLoadCallback::proc(ILoad *iload) {
	DWORD oldVer = ((ExtrudeMod*)(cb->targ))->pblock->GetVersion();
	ReferenceTarget *targ = cb->targ;
	cb->proc(iload);
	if (oldVer<4)
		((ExtrudeMod*)targ)->pblock->SetValue(PB_GEN_MATIDS,0,FALSE);
	if (oldVer<5)
		((ExtrudeMod*)targ)->pblock->SetValue(PB_USE_SHAPEIDS,0,FALSE);
	if (oldVer<6)
		((ExtrudeMod*)targ)->pblock->SetValue(PB_SMOOTH,0,TRUE);

// russom - 11/19/01
// If file loaded has the lathe output set to NURBS, set it Mesh
#ifdef NO_NURBS
	int output;
	((ExtrudeMod*)targ)->pblock->GetValue(PB_OUTPUT, TimeValue(0), output, FOREVER);

	if( output == NURBS_OUTPUT )
		((ExtrudeMod*)targ)->pblock->SetValue(PB_OUTPUT, TimeValue(0), MESH_OUTPUT);
#endif

// russom - 11/19/01
// If file loaded has the lathe output set to Patches, set it Mesh
#ifdef NO_PATCHES
	int outputPatch;
	((ExtrudeMod*)targ)->pblock->GetValue(PB_OUTPUT, TimeValue(0), outputPatch, FOREVER);

	if( outputPatch == PATCH_OUTPUT )
		((ExtrudeMod*)targ)->pblock->SetValue(PB_OUTPUT, TimeValue(0), MESH_OUTPUT);
#endif

	delete this;
	}

#define VERSION_CHUNK 0x100

//watje new mapping ver < 4 of this modifier use the old mapping
IOResult ExtrudeMod::Save(ISave *isave)
	{
	ULONG nb;
	Modifier::Save(isave);

	isave->BeginChunk(VERSION_CHUNK);
	isave->Write(&ver, sizeof(ver), &nb);
	isave->EndChunk();
	return IO_OK;
	}

IOResult ExtrudeMod::Load(ILoad *iload)
	{
	Modifier::Load(iload);

//watje new mapping ver < 4 of this modifier use the old mapping
	IOResult res;
	ULONG nb;
	ver = 3;
	while (IO_OK==(res=iload->OpenChunk())) 
		{
		if (iload->CurChunkID() == VERSION_CHUNK)  
			{
			iload->Read(&ver, sizeof(ver), &nb);
			}
		iload->CloseChunk();
		if (res!=IO_OK) return res;

		}

	iload->RegisterPostLoadCallback(
		new ExtrudePostLoadCallback(
			new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,0)));
	return IO_OK;
	}

static void MakeMeshCapTexture(Mesh &mesh, Matrix3 &itm, int fstart, int fend) {
	if(fstart == fend)
		return;
	// Find out which verts are used by the cap
	BitArray capVerts(mesh.numVerts);
	capVerts.ClearAll();
	for(int i = fstart; i < fend; ++i) {
		Face &f = mesh.faces[i];
		capVerts.Set(f.v[0]);
		capVerts.Set(f.v[1]);
		capVerts.Set(f.v[2]);
		}
	// Minmax the verts involved in X/Y axis and total them
	Box3 bounds;
	int numCapVerts = 0;
	int numCapFaces = fend - fstart;
	IntTab capIndexes;
	capIndexes.SetCount(mesh.numVerts);
	int baseTVert = mesh.getNumTVerts();
	for(i = 0; i < mesh.numVerts; ++i) {
		if(capVerts[i]) {
			capIndexes[i] = baseTVert + numCapVerts++;
			bounds += mesh.verts[i] * itm;
			}
		}
	mesh.setNumTVerts(baseTVert + numCapVerts, TRUE);
	Point3 s(1.0f / bounds.Width().x, 1.0f / bounds.Width().y, 0.0f);
	Point3 t(-bounds.Min().x, -bounds.Min().y, 0.0f);
	// Do the TVerts
	for(i = 0; i < mesh.numVerts; ++i) {
		if(capVerts[i])
			mesh.setTVert(baseTVert++, ((mesh.verts[i] * itm) + t) * s);
		}
	// Do the TVFaces
	for(i = fstart; i < fend; ++i) {
		Face &f = mesh.faces[i];
		mesh.tvFace[i] = TVFace(capIndexes[f.v[0]], capIndexes[f.v[1]], capIndexes[f.v[2]]);
		}
	}

void ExtrudeMod::BuildMeshFromShape(TimeValue t,ModContext &mc, ObjectState * os, Mesh &mesh, BOOL simple) {
	BOOL texturing;
	pblock->GetValue(PB_MAPPING, TimeValue(0), texturing, FOREVER);
	BOOL genMatIDs;
	pblock->GetValue(PB_GEN_MATIDS, TimeValue(0), genMatIDs, FOREVER);
	BOOL useShapeIDs;
	pblock->GetValue(PB_USE_SHAPEIDS, TimeValue(0), useShapeIDs, FOREVER);
	BOOL smooth;
	pblock->GetValue(PB_SMOOTH, TimeValue(0), smooth, FOREVER);

	ShapeObject *shape = (ShapeObject *)os->obj;

	float amount;
	int levels,capStart,capEnd,capType;

	pblock->GetValue(PB_AMOUNT,t,amount,FOREVER);
	if(simple) {
		levels = 1;
		capStart = capEnd = FALSE;
		}
	else {
		pblock->GetValue(PB_SEGS,t,levels,FOREVER);
		if (levels<1) levels = 1;
		pblock->GetValue(PB_CAPSTART,t,capStart,FOREVER);
		pblock->GetValue(PB_CAPEND,t,capEnd,FOREVER);
		}
	pblock->GetValue(PB_CAPTYPE,t,capType,FOREVER);

	LimitValue(amount, -1000000.0f, 1000000.0f);

	// Get the basic dimension stuff
	float zSize = (float)fabs(amount);
	float baseZ = 0.0f;
	if(amount < 0.0f)
		baseZ = amount;

	// Make the shape convert itself to a PolyShape -- This makes our mesh conversion MUCH easier!
	
	PolyShape pShape;
	shape->MakePolyShape(t, pShape);
	ShapeHierarchy hier;
	pShape.OrganizeCurves(t, &hier);
	// Need to flip the reversed curves in the shape!
	pShape.Reverse(hier.reverse);

	int polys = pShape.numLines;
	int levelVerts = 0, levelFaces = 0, levelTVerts = 0;
	int verts = 0, faces = 0, tVerts = 0;
	int poly, piece;

	BOOL anyClosed = FALSE;
	for(poly = 0; poly < polys; ++poly) {
		PolyLine &line = pShape.lines[poly];
		if(!line.numPts)
			continue;
		if(line.IsClosed()) {
			anyClosed = TRUE;
			levelTVerts++;
			}
		levelVerts += line.numPts;
		levelTVerts += line.numPts;
		levelFaces += (line.Segments() * 2);
		}

	int vertsPerLevel = levelVerts;
	int numLevels = levels;

	verts = levelVerts * (levels + 1);
	tVerts = levelTVerts * (levels + 1);
	faces = levelFaces * levels;

	mesh.setNumVerts(verts);
	mesh.setNumFaces(faces);
	if(texturing) {
		mesh.setNumTVerts(tVerts);
		mesh.setNumTVFaces(faces);
		}

	// Create the vertices!
	int vert = 0;
	int tvertex = 0;
	int level;
	Point3 offset1, offset2;
	for(poly = 0; poly < polys; ++poly) {
		PolyLine &line = pShape.lines[poly];
		if(!line.numPts)
			continue;
		if(texturing) {
//DebugPrint("Texture Verts:\n");
			int tp;
			int texPts = line.numPts + (line.IsClosed() ? 1 : 0);
			float *texPt = new float [texPts];
			float lastPt = (float)(texPts - 1);
			float cumLen = 0.0f;
			float totLen = line.CurveLength();
			Point3 prevPt = line.pts[0].p;
			for(tp = 0; tp < texPts; ++tp) {
				int ltp = tp % line.numPts;
				if(tp == (texPts - 1))
					texPt[tp] = 1.0f;
				else {
					Point3 &pt = line.pts[ltp].p;
					cumLen += Length(pt - prevPt);
					texPt[tp] = cumLen / totLen;
					prevPt = pt;
					}
				}
			float flevels = (float)levels;
			for(level = 0; level <= levels; ++level) {
				float tV = (float)level / flevels;
				for(tp = 0; tp < texPts; ++tp) {
					mesh.setTVert(tvertex++, UVVert(texPt[tp], tV, 0.0f));
					}
				}
			delete [] texPt;
			}
		int lverts = line.numPts;
		for(level = 0; level <= levels; ++level) {
			Point3 offset = Point3(0.0f, 0.0f, baseZ + (float)level / (float)levels * zSize);
			if(level == 0)
				offset1 = offset;
			else
			if(level == levels)
				offset2 = offset;
			for(int v = 0; v < lverts; ++v) {
				line.pts[v].aux = vert;			// Gives the capper this vert's location in the mesh!
				mesh.setVert(vert++, line.pts[v].p + offset);
				}
			}
		}
	assert(vert == verts);

	// If capping, do it!
	if(anyClosed && (capStart || capEnd)) {
		MeshCapInfo capInfo;
		pShape.MakeCap(t, capInfo, capType);
		// Build information for capping
		MeshCapper capper(pShape);
		if(capStart) {
			vert = 0;
			for(poly = 0; poly < polys; ++poly) {
				PolyLine &line = pShape.lines[poly];
				if(!line.numPts)
					continue;
				MeshCapPoly &capline = capper[poly];
				int lverts = line.numPts;
				for(int v = 0; v < lverts; ++v)
					capline.SetVert(v, vert++);			// Gives this vert's location in the mesh!
				vert += lverts * levels;
				}
			// Create a work matrix for grid capping
			Matrix3 gridMat = TransMatrix(offset1);
			int oldFaces = mesh.numFaces;
			capper.CapMesh(mesh, capInfo, TRUE, 16, &gridMat, genMatIDs ? -1 : 0);
			// If texturing, create the texture faces and vertices
			if(texturing)
				MakeMeshCapTexture(mesh, Inverse(gridMat), oldFaces, mesh.numFaces);
			}
		if(capEnd) {
			int baseVert = 0;
			for(poly = 0; poly < polys; ++poly) {
				PolyLine &line = pShape.lines[poly];
				if(!line.numPts)
					continue;
				MeshCapPoly &capline = capper[poly];
				int lverts = line.numPts;
				vert = baseVert + lverts * levels;
				for(int v = 0; v < lverts; ++v)
					capline.SetVert(v, vert++);			// Gives this vert's location in the mesh!
				baseVert += lverts * (levels + 1);
				}
			// Create a work matrix for grid capping
			Matrix3 gridMat = TransMatrix(offset2);
			int oldFaces = mesh.numFaces;
			capper.CapMesh(mesh, capInfo, FALSE, 16, &gridMat, genMatIDs ? -1 : 0);
			// If texturing, create the texture faces and vertices
			if(texturing)
				MakeMeshCapTexture(mesh, Inverse(gridMat), oldFaces, mesh.numFaces);
			}
		}

	// Create the faces!
	int face = 0;
	int TVface = 0;
	int baseVert = 0;
	int baseTVert = 0;
	for(poly = 0; poly < polys; ++poly) {
		PolyLine &line = pShape.lines[poly];
		if(!line.numPts)
			continue;
		int pieces = line.Segments();
		int closed = line.IsClosed();
		int segVerts = pieces + ((closed) ? 0 : 1);
		int segTVerts = pieces + 1;
		for(level = 0; level < levels; ++level) {
			int sm = 0;		// Initial smoothing group
			BOOL firstSmooth = (line.pts[0].flags & POLYPT_SMOOTH) ? TRUE : FALSE;
			for(piece = 0; piece < pieces; ++piece) {
				int v1 = baseVert + piece;
				int v2 = baseVert + ((piece + 1) % segVerts);
				int v3 = v1 + segVerts;
				int v4 = v2 + segVerts;
				// If the vertex is not smooth, go to the next group!
				BOOL thisSmooth = line.pts[piece].flags & POLYPT_SMOOTH;
				MtlID mtl = useShapeIDs ? line.pts[piece].GetMatID() : 2;
				if(piece > 0 && !thisSmooth) {
					sm++;
					if(sm > 2)
						sm = 1;
					}
				DWORD smoothGroup = 1 << sm;
				// Advance to the next smoothing group right away
				if(sm == 0)
					sm++;
				// Special case for smoothing from first segment
				if(piece == 1 && thisSmooth)
					smoothGroup |= 1;
				// Special case for smoothing from last segment
				if((piece == pieces - 1) && firstSmooth)
					smoothGroup |= 1;
				mesh.faces[face].setEdgeVisFlags(1,1,0);
				mesh.faces[face].setSmGroup(smooth ? smoothGroup : 0);
				mesh.faces[face].setMatID(genMatIDs ? mtl : 0);
				mesh.faces[face++].setVerts(v1, v2, v4);
				mesh.faces[face].setEdgeVisFlags(0,1,1);
				mesh.faces[face].setSmGroup(smooth ? smoothGroup : 0);
				mesh.faces[face].setMatID(genMatIDs ? mtl : 0);
				mesh.faces[face++].setVerts(v1, v4, v3);
//DebugPrint("BV:%d V:%d v1:%d v2:%d v3:%d v4:%d\n",baseVert, vert, v1, v2, v3, v4);
				if(texturing) {
					int tv1 = baseTVert + piece;
					int tv2 = tv1 + 1;
					int tv3 = tv1 + segTVerts;
					int tv4 = tv2 + segTVerts;
					mesh.tvFace[TVface++].setTVerts(tv1, tv2, tv4);
					mesh.tvFace[TVface++].setTVerts(tv1, tv4, tv3);
					}
				}
			baseVert += segVerts;
			baseTVert += segTVerts;
			}
		baseVert += segVerts;	// Increment to next poly start (skips last verts of this poly)
		baseTVert += segTVerts;
		}
	assert(face == faces);

	mesh.InvalidateGeomCache();
	}

static void MakePatchCapTexture(PatchMesh &pmesh, Matrix3 &itm, int pstart, int pend) {
	if(pstart == pend)
		return;

	// Find out which verts are used by the cap
	BitArray capVerts(pmesh.numVerts);
	capVerts.ClearAll();
	for(int i = pstart; i < pend; ++i) {
		Patch &p = pmesh.patches[i];
		capVerts.Set(p.v[0]);
		capVerts.Set(p.v[1]);
		capVerts.Set(p.v[2]);
		if(p.type == PATCH_QUAD)
			capVerts.Set(p.v[3]);
		}
	// Minmax the verts involved in X/Y axis and total them
	Box3 bounds;
	int numCapVerts = 0;
	int numCapPatches = pend - pstart;
	IntTab capIndexes;
	capIndexes.SetCount(pmesh.numVerts);
	int baseTVert = pmesh.getNumTVerts();
	for(i = 0; i < pmesh.numVerts; ++i) {
		if(capVerts[i]) {
			capIndexes[i] = baseTVert + numCapVerts++;
			bounds += pmesh.verts[i].p * itm;
			}
		}
	pmesh.setNumTVerts(baseTVert + numCapVerts, TRUE);
	Point3 s(1.0f / bounds.Width().x, 1.0f / bounds.Width().y, 0.0f);
	Point3 t(-bounds.Min().x, -bounds.Min().y, 0.0f);
	// Do the TVerts
	for(i = 0; i < pmesh.numVerts; ++i) {
		if(capVerts[i])
			pmesh.setTVert(baseTVert++, ((pmesh.verts[i].p * itm) + t) * s);
		}
	// Do the TVPatches
	for(i = pstart; i < pend; ++i) {
		Patch &p = pmesh.patches[i];
		TVPatch &tp = pmesh.getTVPatch(i);
		if(p.type == PATCH_TRI)
			tp.setTVerts(capIndexes[p.v[0]], capIndexes[p.v[1]], capIndexes[p.v[2]]);
		else
			tp.setTVerts(capIndexes[p.v[0]], capIndexes[p.v[1]], capIndexes[p.v[2]], capIndexes[p.v[3]]);
		}
	}

// Quad patch layout
//
//   A---> ad ----- da <---D
//   |                     |
//   |                     |
//   v                     v
//   ab    i1       i4     dc
//
//   |                     |
//   |                     |
// 
//   ba    i2       i3     cd
//   ^					   ^
//   |                     |
//   |                     |
//   B---> bc ----- cb <---C
//
// vertices ( a b c d ) are in counter clockwise order when viewed from 
// outside the surface

#ifndef NO_PATCHES
void ExtrudeMod::BuildPatchFromShape(TimeValue t,ModContext &mc, ObjectState * os, PatchMesh &pmesh) {
	ShapeObject *shape = (ShapeObject *)os->obj;

	float amount;
	int levels,capStart,capEnd,capType;

	pblock->GetValue(PB_AMOUNT,t,amount,FOREVER);
	pblock->GetValue(PB_SEGS,t,levels,FOREVER);
	if (levels<1) levels = 1;
	pblock->GetValue(PB_CAPSTART,t,capStart,FOREVER);
	pblock->GetValue(PB_CAPEND,t,capEnd,FOREVER);
	pblock->GetValue(PB_CAPTYPE,t,capType,FOREVER);

	BOOL texturing;
	pblock->GetValue(PB_MAPPING, TimeValue(0), texturing, FOREVER);
	BOOL genMatIDs;
	pblock->GetValue(PB_GEN_MATIDS, TimeValue(0), genMatIDs, FOREVER);
	BOOL useShapeIDs;
	pblock->GetValue(PB_USE_SHAPEIDS, TimeValue(0), useShapeIDs, FOREVER);
	BOOL smooth;
	pblock->GetValue(PB_SMOOTH, TimeValue(0), smooth, FOREVER);

	LimitValue(amount, -1000000.0f, 1000000.0f);

	// Get the basic dimension stuff
	float zSize = (float)fabs(amount);
	float baseZ = 0.0f;
	if(amount < 0.0f)
		baseZ = amount;

	// If the shape can convert itself to a BezierShape, have it do so!
	BezierShape bShape;
	if(shape->CanMakeBezier())
		shape->MakeBezier(t, bShape);
	else {
		PolyShape pShape;
		shape->MakePolyShape(t, pShape);
		bShape = pShape;	// UGH -- Convert it from a PolyShape -- not good!
		}
	
//DebugPrint("Extrude organizing shape\n");
	ShapeHierarchy hier;
	bShape.OrganizeCurves(t, &hier);
	// Need to flip the reversed polys...
	bShape.Reverse(hier.reverse);
	// ...and tell the hierarchy they're no longer reversed!
	hier.reverse.ClearAll();

	// Our shapes are now organized for patch-making -- Let's do the sides!
	int polys = bShape.splineCount;
	int poly, knot;
	int levelVerts = 0, levelVecs = 0, levelPatches = 0, nverts = 0, nvecs = 0, npatches = 0;
	int TVlevels = levels + 1, levelTVerts = 0, ntverts = 0, ntpatches = 0;
	BOOL anyClosed = FALSE;
	for(poly = 0; poly < polys; ++poly) {
		Spline3D *spline = bShape.splines[poly];
		if(!spline->KnotCount())
			continue;
		if(spline->Closed())
			anyClosed = TRUE;
		levelVerts += spline->KnotCount();
		levelTVerts += (spline->Segments() + 1);
		levelVecs += (spline->Segments() * 2);
		levelPatches += spline->Segments();
		}
	nverts = levelVerts * (levels + 1);
	npatches = levelPatches * levels;
	nvecs = (levelVecs * (levels + 1)) + levels * levelVerts * 2 + npatches * 4;
	if(texturing) {
		ntverts = levelTVerts * TVlevels;
		ntpatches = npatches;
		}

	pmesh.setNumVerts(nverts);
	pmesh.setNumVecs(nvecs);
	pmesh.setNumPatches(npatches);
	pmesh.setNumTVerts(ntverts);
	pmesh.setNumTVPatches(ntpatches);

	// Create the vertices!
	int vert = 0;
	int level;
	Point3 offset1, offset2;
	for(poly = 0; poly < polys; ++poly) {
		Spline3D *spline = bShape.splines[poly];
		if(!spline->KnotCount())
			continue;
		int knots = spline->KnotCount();
		for(level = 0; level <= levels; ++level) {
			Point3 offset = Point3(0.0f, 0.0f, baseZ + (float)level / (float)levels * zSize);
			if(level == 0)
				offset1 = offset;
			else
			if(level == levels)
				offset2 = offset;
			for(knot = 0; knot < knots; ++knot) {
				Point3 p = spline->GetKnotPoint(knot);
				pmesh.setVert(vert++, p + offset);
				}
			}
		}
	assert(vert == nverts);

	// Maybe create the texture vertices
	if(texturing) {
		int tvert = 0;
		int level;
		for(poly = 0; poly < polys; ++poly) {
			Spline3D *spline = bShape.splines[poly];
			if(!spline->KnotCount())
				continue;
			// Make it a polyline
			PolyLine pline;
			spline->MakePolyLine(pline, 10);
			int knots = spline->KnotCount();
			for(level = 0; level < TVlevels; ++level) {
				float tV = (float)level / (float)(TVlevels - 1);
				int lverts = pline.numPts;
				int tp = 0;
				int texPts = spline->Segments() + 1;
				float cumLen = 0.0f;
				float totLen = pline.CurveLength();
				Point3 prevPt = pline.pts[0].p;
				int plix = 0;
				while(tp < texPts) {
					Point3 &pt = pline[plix].p;
					cumLen += Length(pt - prevPt);
					prevPt = pt;
					if(pline[plix].flags & POLYPT_KNOT) {
						float tU;
						if(tp == (texPts - 1))
							tU = 1.0f;
						else
							tU = cumLen / totLen;
						pmesh.setTVert(tvert++, UVVert(tU, tV, 0.0f));
						tp++;
						}
					plix = (plix + 1) % pline.numPts;
					}
				}
			}
		assert(tvert == ntverts);
		}

	// Create the vectors!
	int seg;
	int vec = 0;
	for(poly = 0; poly < polys; ++poly) {
		Spline3D *spline = bShape.splines[poly];
		if(!spline->KnotCount())
			continue;
		int segs = spline->Segments();
		int knots = spline->KnotCount();
		// First, the vectors on each level
		for(level = 0; level <= levels; ++level) {
			Point3 offset = Point3(0.0f, 0.0f, baseZ + (float)level / (float)levels * zSize);
			for(seg = 0; seg < segs; ++seg) {
				int seg2 = (seg + 1) % knots;
				if(spline->GetLineType(seg) == LTYPE_CURVE) {
					Point3 p = spline->GetOutVec(seg);
					pmesh.setVec(vec++, p + offset);
					p = spline->GetInVec(seg2);
					pmesh.setVec(vec++, p + offset);
					}
				else {
					Point3 p = spline->InterpBezier3D(seg, 0.333333f);
					pmesh.setVec(vec++, p + offset);
					p = spline->InterpBezier3D(seg, 0.666666f);
					pmesh.setVec(vec++, p + offset);
					}
				}
			}
		// Now, the vectors between the levels
		int baseVec = vec;
		for(level = 0; level < levels; ++level) {
			Point3 offsetA = Point3(0.0f, 0.0f, baseZ + (float)level / (float)levels * zSize);
			Point3 offsetB = Point3(0.0f, 0.0f, baseZ + (float)(level + 1) / (float)levels * zSize);
			Point3 offset1 = offsetA + (offsetB - offsetA) * 0.333333333f;
			Point3 offset2 = offsetA + (offsetB - offsetA) * 0.666666666f;
			for(knot = 0; knot < knots; ++knot) {
				Point3 p = spline->GetKnotPoint(knot);
				pmesh.setVec(vec++, p + offset1);
				pmesh.setVec(vec++, p + offset2);
				}
			}
		}

	// Create the patches!
	int np = 0;
	int baseVert = 0;
	int baseVec = 0;
	for(poly = 0; poly < polys; ++poly) {
		Spline3D *spline = bShape.splines[poly];
		if(!spline->KnotCount())
			continue;
		int knots = spline->KnotCount();
		int segs = spline->Segments();
		int baseVec1 = baseVec;	// Base vector index for this level
		int baseVec2 = baseVec + segs * 2 * (levels + 1);	// Base vector index for between levels
		for(level = 0; level < levels; ++level) {
			int sm = 0;
			BOOL firstSmooth = (spline->GetLineType(0) == LTYPE_CURVE && spline->GetLineType(segs-1) == LTYPE_CURVE && (spline->GetKnotType(0) == KTYPE_AUTO || spline->GetKnotType(0) == KTYPE_BEZIER)) ? TRUE : FALSE;
			for(seg = 0; seg < segs; ++seg, vec += 4) {
				int prevseg = (seg + segs - 1) % segs;
				int seg2 = (seg + 1) % knots;
				int a,b,c,d,ab,ba,bc,cb,cd,dc,da,ad;
				MtlID mtl = useShapeIDs ? spline->GetMatID(seg) : 2;
				a = baseVert + seg;
				b = baseVert + seg2;
				c = b + knots;
				d = a + knots;
				ab = baseVec1 + seg * 2;
				ba = ab + 1;
				bc = baseVec2 + seg2 * 2;
				cb = bc + 1;
				cd = ba + (segs * 2);
				dc = ab + (segs * 2);
				da = baseVec2 + seg * 2 + 1;
				ad = da - 1;
//DebugPrint("Making patch %d: %d (%d %d) %d (%d %d) %d (%d %d) %d (%d %d)\n",np, a, ab, ba, b, bc, cb, c, cd, dc, d, da, ad);
				// If the vertex is not smooth, go to the next group!
				if(seg > 0 && !(spline->GetLineType(prevseg) == LTYPE_CURVE && spline->GetLineType(seg) == LTYPE_CURVE && (spline->GetKnotType(seg) == KTYPE_AUTO || spline->GetKnotType(seg) == KTYPE_BEZIER))) {
					sm++;
					if(sm > 2)
						sm = 1;
					}
				DWORD smoothGroup = 1 << sm;
				if(seg == segs - 1 && firstSmooth) {
					smoothGroup |= 1;
					}
				pmesh.MakeQuadPatch(np, a, ab, ba, b, bc, cb, c, cd, dc, d, da, ad, vec, vec+1, vec+2, vec+3, smooth ? smoothGroup : 0);
				pmesh.setPatchMtlIndex(np++, genMatIDs ? mtl : 0);
				}
			baseVert += knots;
			baseVec1 += (segs * 2);
			baseVec2 += (knots * 2);
			}
		baseVert += knots;
		baseVec += (segs * 2 * (levels + 1) + knots * 2 * levels);
		}
	assert(vec == nvecs);
	assert(np == npatches);

 	// Maybe create the texture patches!
	if(texturing) {
		int ntp = 0;
		int baseTVert = 0;
		for(poly = 0; poly < polys; ++poly) {
			Spline3D *spline = bShape.splines[poly];
			if(!spline->KnotCount())
				continue;
			int pknots = spline->Segments() + 1;
			int pverts = pknots * TVlevels;
			int segs = spline->Segments();
			for(level = 0; level < levels; ++level) {
				for(seg = 0; seg < segs; ++seg) {
					int prevseg = (seg + segs - 1) % segs;
					int seg2 = seg + 1;
					int a,b,c,d;
					a = baseTVert + seg;
					b = baseTVert + seg2;
					c = b + pknots;
					d = a + pknots;
					TVPatch &tp = pmesh.getTVPatch(ntp++);
					tp.setTVerts(a, b, c, d);
					
					}
				baseTVert += pknots;
				}
			baseTVert += pknots;
			}
		assert(ntp == ntpatches);
		}

	// If capping, do it!
	if(anyClosed && (capStart || capEnd)) {
		PatchCapInfo capInfo;
		bShape.MakeCap(t, capInfo);

		// Build information for capping
		PatchCapper capper(bShape);
		if(capStart) {
			vert = 0;
			int baseVec = 0;
			for(poly = 0; poly < polys; ++poly) {
				Spline3D *spline = bShape.splines[poly];
				if(!spline->KnotCount())
					continue;
				PatchCapPoly &capline = capper[poly];
				int lverts = spline->KnotCount();
				for(int v = 0; v < lverts; ++v)
					capline.SetVert(v, vert++);			// Gives this vert's location in the mesh!
				vert += lverts * levels;
				vec = baseVec;
				int lvecs = spline->Segments() * 2;
				for(v = 0; v < lvecs; ++v)
					capline.SetVec(v, vec++);			// Gives this vec's location in the mesh!
				baseVec += lvecs * (levels + 1) + spline->KnotCount() * levels * 2;
				}
			// Create a work matrix for capping
			Matrix3 mat = TransMatrix(offset1);
			int oldPatches = pmesh.numPatches;
			capper.CapPatchMesh(pmesh, capInfo, TRUE, 16, &mat, genMatIDs ? -1 : 0);
			// If texturing, create the texture patches and vertices
			if(texturing)
				MakePatchCapTexture(pmesh, Inverse(mat), oldPatches, pmesh.numPatches);
			}
		if(capEnd) {
			int baseVert = 0;
			int baseVec = 0;
			for(poly = 0; poly < polys; ++poly) {
				Spline3D *spline = bShape.splines[poly];
				if(!spline->KnotCount())
					continue;
				PatchCapPoly &capline = capper[poly];
				int lverts = spline->KnotCount();
				int vert = baseVert + lverts * levels;
				for(int v = 0; v < lverts; ++v)
					capline.SetVert(v, vert++);			// Gives this vert's location in the mesh!
				baseVert += lverts * (levels + 1);
				int lvecs = spline->Segments()*2;
				int vec = baseVec + lvecs * levels;
				for(v = 0; v < lvecs; ++v)
					capline.SetVec(v, vec++);			// Gives this vec's location in the mesh!
				baseVec += lvecs * (levels + 1) + spline->KnotCount() * levels * 2;
				}
			// Create a work matrix for grid capping
			Matrix3 mat = TransMatrix(offset2);
			int oldPatches = pmesh.numPatches;
			capper.CapPatchMesh(pmesh, capInfo, FALSE, 16, &mat, genMatIDs ? -1 : 0);
			// If texturing, create the texture patches and vertices
			if(texturing)
				MakePatchCapTexture(pmesh, Inverse(mat), oldPatches, pmesh.numPatches);
			}
		}

	//watje new mapping
	if(texturing)
		{
		if (ver < 4)
			{
			for (int i = 0; i < pmesh.numPatches; i++)
				pmesh.patches[i].flags |= PATCH_LINEARMAPPING;
			}
		}


	// Ready the patch representation!
	assert(pmesh.buildLinkages());
	pmesh.computeInteriors();
	}
#endif // NO_PATCHES

#ifdef XXDESIGN_VER
#define MIN_EXT 1.0E-3f
bool ExtrudeMod::BuildAMSolidFromShape(TimeValue t,ModContext &mc, ObjectState * os, IGeomImp* igeom) 
{
	ShapeObject *shape = (ShapeObject *)os->obj;

	float amount;

	pblock->GetValue(PB_AMOUNT,t,amount,FOREVER);

	BOOL texturing;
	pblock->GetValue(PB_MAPPING, TimeValue(0), texturing, FOREVER);
	BOOL genMatIDs;
	pblock->GetValue(PB_GEN_MATIDS, TimeValue(0), genMatIDs, FOREVER);
	BOOL useShapeIDs;
	pblock->GetValue(PB_USE_SHAPEIDS, TimeValue(0), useShapeIDs, FOREVER);
	BOOL smooth;
	pblock->GetValue(PB_SMOOTH, TimeValue(0), smooth, FOREVER);

	LimitValue(amount, -1000000.0f, 1000000.0f);
	if(fabs(amount)<MIN_EXT)
		amount = MIN_EXT;

	// Get the basic dimension stuff
	float zSize = (float)fabs(amount);
	float baseZ = 0.0f;
	if(amount < 0.0f)
		baseZ = amount;

	// If the shape can convert itself to a BezierShape, have it do so!
	BezierShape bShape;
	PolyShape pShape;
	shape->MakePolyShape(t, pShape, PSHAPE_BUILTIN_STEPS, true);
	ShapeHierarchy hier;
	pShape.OrganizeCurves(t, &hier);
	// Need to flip the reversed curves in the shape!
	pShape.Reverse(hier.reverse);

	//Now add the extrusion for each polygon
	bool res = false;//assume the best

	//make a solid to hold our work
	Object* solid = (Object*)CreateInstance(GEOMOBJECT_CLASS_ID, GENERIC_AMSOLID_CLASS_ID);
	IGeomImp* workgeom = NULL;
	assert(solid);
	if(solid)
	{
		workgeom = (IGeomImp*)(solid->GetInterface(I_GEOMIMP));
		assert(workgeom);
	}

	for(int i=0;i<pShape.numLines; i++)
	{
		PolyLine poly = pShape.lines[i];
		if(poly.numPts)
			poly.pts[0].aux = poly.flags;
		bool localres  = workgeom->createExtrusion(poly.pts, poly.numPts, amount, smooth, genMatIDs, useShapeIDs);

		if(localres)
		{
			//need to figure out how deeply nested this shape is
			//order is this value with zero meaning not nested
			HierarchyEntry *entry = hier.hier.FindEntry(i);
			assert(entry);
			int order = 0;
			while(entry && (entry->parent != (HierarchyEntry *)-1))
			{
				entry = entry->parent;
				++order;
			}

			if(order%2)
				igeom->operator-=(*workgeom);
			else
				igeom->operator+=(*workgeom);
		}
		res |= localres;
	}

	//do some cleanup
	if(workgeom)
	{
		assert(solid);
		solid->ReleaseInterface(I_GEOMIMP, workgeom);
		solid->DeleteMe();
	}
	return res;
}
#endif
