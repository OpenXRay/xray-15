/**********************************************************************
 *<
	FILE: surfrev.cpp

	DESCRIPTION:  Shape->patch surfrev modifier

	CREATED BY: Tom Hudson, Dan Silva & Rolf Berteig

	HISTORY: created 11 November, 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#include "mods.h"

#ifndef NO_MODIFIER_LATHE  // JP Morel - July 23th 2002

#include "iparamm.h"
#include "shape.h"
#include "spline3d.h"
#include "splshape.h"
#include "linshape.h"
#include "surf_api.h"
#include "MaxIcon.h"
#include "modsres.h"
#include "resourceOverride.h"


//--- SurfrevMod -----------------------------------------------------------

#define CORE_THRESHOLD	0.01f

#define MIN_DEGREES		0.0f
#define MAX_DEGREES		360.0f

#define MIN_SEGS		3
#define MAX_SEGS		10000

#define DEF_DEGREES 360.0f
#define DEF_SEGS 16
#define DEF_CAPSTART 1
#define DEF_CAPEND 1
#define DEF_CAPTYPE CAPTYPE_MORPH
#define DEF_WELDCORE FALSE
#define DEF_FLIPNORMALS FALSE
#define DEF_MAPPING FALSE
#define DEF_GEN_MATIDS TRUE
#define DEF_USE_SHAPEIDS FALSE
#define DEF_SMOOTH TRUE

#define PATCH_OUTPUT 0
#define MESH_OUTPUT 1
#define NURBS_OUTPUT 2

#define DEF_OUTPUT MESH_OUTPUT //PATCH_OUTPUT

#define X_AXIS 0
#define Y_AXIS 1
#define Z_AXIS 2

#define ALIGN_MIN 0
#define ALIGN_CENTER 1
#define ALIGN_MAX 2

class SurfrevMod;

// Our reference indexes
#define AXISREF 0
#define PBLOCKREF 1

static GenSubObjType SOT_Axis(17);


// Special dialog handling
class SurfrevDlgProc : public ParamMapUserDlgProc {
	private:
		SurfrevMod *mod;
	public:
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void SetMod(SurfrevMod *m) { mod = m; }
		void DeleteThis() {}
	};

class SurfrevMod: public Modifier {
	protected:
		Control *axisControl;

		static IObjParam *ip;
		static SurfrevMod *editMod;
		static MoveModBoxCMode *moveMode;
		static RotateModBoxCMode *rotMode;
		static UScaleModBoxCMode *uscaleMode;
		static NUScaleModBoxCMode *nuscaleMode;
		static SquashModBoxCMode *squashMode;		

		SurfrevDlgProc dlgProc;
		BOOL doAlign;			// Special command flag.  See SurfrevMod::Align
		int alignType;
	public:
		IParamBlock *pblock;

		static IParamMap *pmapParam;
		
		static float dlgDegrees;
		static BOOL dlgWeldCore;
		static BOOL dlgFlipNormals;
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
		void GetClassName(TSTR& s) { s= TSTR(GetString(IDS_TH_LATHE_CLASS)); }  
		virtual Class_ID ClassID() { return Class_ID(SURFREVOSM_CLASS_ID,0);}		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() { return GetString(IDS_TH_LATHE); }
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);


		SurfrevMod();
		virtual ~SurfrevMod();

		ChannelMask ChannelsUsed()  { return PART_GEOM|PART_TOPO; }
		// Possible GOTCHA -- Modifiers like this one, which completely change the type of
		// object (shape -> Mesh or Patch) change ALL channels!  Be sure to tell the system!
		ChannelMask ChannelsChanged() { return PART_ALL; }
		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		Class_ID InputType() {return genericShapeClassID;}
		Interval LocalValidity(TimeValue t);

		// From BaseObject
		int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flagst, ModContext *mc);
		void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);
		
		IParamArray *GetParamBlock() {return pblock;}
		int GetParamBlockIndex(int id) {return id;}

		void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
		void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
		BOOL ChangeTopology() {return TRUE;}

		Matrix3 CompMatrix(TimeValue t, ModContext& mc, Matrix3& ntm, 
			Interval& valid, BOOL needOffset);

		// NS: New SubObjType API
		int NumSubObjTypes();
		ISubObjType *GetSubObjType(int i);

		// Affine transform methods
		void Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE );
		void Rotate( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin=FALSE );
		void Scale( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE );

		int NumRefs() {return 2;}
		void RescaleWorldUnits(float f);
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
		int SubNumToRefNum(int i) { return i; }
		BOOL AssignController(Animatable *control,int subAnim);

 		int NumSubs() { return 2; }  
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);		

		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message);

		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 

		void ActivateSubobjSel(int level, XFormModes& modes);

		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);

		void BuildMeshFromShape(TimeValue t,ModContext &mc, ObjectState * os, Matrix3 &axis, Mesh &mesh);
#ifndef NO_PATCHES
		void BuildPatchFromShape(TimeValue t,ModContext &mc, ObjectState * os, Matrix3 &axis, PatchMesh &pmesh);
#endif

		void SetAxis(TimeValue t,int type);
		void Align(TimeValue t, int type);
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

BOOL SurfrevDlgProc::DlgProc(
		TimeValue t,IParamMap *map,
		HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	ICustButton *but;
	switch (msg) {
		case WM_INITDIALOG:
			but = GetICustButton(GetDlgItem(hWnd,IDC_X));
			but->SetType(CBT_PUSH);
			ReleaseICustButton(but);
			but = GetICustButton(GetDlgItem(hWnd,IDC_Y));
			but->SetType(CBT_PUSH);
			ReleaseICustButton(but);
			but = GetICustButton(GetDlgItem(hWnd,IDC_Z));
			but->SetType(CBT_PUSH);
			ReleaseICustButton(but);
			but = GetICustButton(GetDlgItem(hWnd,IDC_ALIGNMIN));
			but->SetType(CBT_PUSH);
			ReleaseICustButton(but);
			but = GetICustButton(GetDlgItem(hWnd,IDC_ALIGNCENTER));
			but->SetType(CBT_PUSH);
			ReleaseICustButton(but);
			but = GetICustButton(GetDlgItem(hWnd,IDC_ALIGNMAX));
			but->SetType(CBT_PUSH);
			ReleaseICustButton(but);
			EnableWindow(GetDlgItem(hWnd,IDC_USE_SHAPE_IDS),mod->GetGenMatIDs());
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_X:
					mod->SetAxis(t, X_AXIS);
					break;
				case IDC_Y:
					mod->SetAxis(t, Y_AXIS);
					break;
				case IDC_Z:
					mod->SetAxis(t, Z_AXIS);
					break;
				case IDC_ALIGNMIN:
					mod->Align(t, ALIGN_MIN);
					break;
				case IDC_ALIGNCENTER:
					mod->Align(t, ALIGN_CENTER);
					break;
				case IDC_ALIGNMAX:
					mod->Align(t, ALIGN_MAX);
					break;
				case IDC_GEN_MATIDS:
					EnableWindow(GetDlgItem(hWnd,IDC_USE_SHAPE_IDS),mod->GetGenMatIDs());
					break;
				}
			break;
		}
	return FALSE;
	}

class SurfrevClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new SurfrevMod; }
	const TCHAR *	ClassName() { return GetString(IDS_TH_LATHE_CLASS); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return  Class_ID(SURFREVOSM_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFDEFORMATIONS);}
	void			ResetClassParams(BOOL fileReset);
	};

void SurfrevClassDesc::ResetClassParams(BOOL fileReset)
	{
	SurfrevMod::dlgDegrees = DEF_DEGREES;
	SurfrevMod::dlgWeldCore = DEF_WELDCORE;
	SurfrevMod::dlgFlipNormals = DEF_FLIPNORMALS;
	SurfrevMod::dlgSegs = DEF_SEGS;
	SurfrevMod::dlgCapStart = DEF_CAPSTART;
	SurfrevMod::dlgCapEnd = DEF_CAPEND;
	SurfrevMod::dlgCapType = DEF_CAPTYPE;
	SurfrevMod::dlgOutput = DEF_OUTPUT;
	SurfrevMod::dlgMapping = DEF_MAPPING;
	SurfrevMod::dlgGenMatIDs    = DEF_GEN_MATIDS;
	SurfrevMod::dlgUseShapeIDs  = DEF_USE_SHAPEIDS;
	SurfrevMod::dlgSmooth = DEF_SMOOTH;
	}

static SurfrevClassDesc surfrevDesc;
extern ClassDesc* GetSurfrevModDesc() { return &surfrevDesc; }

IObjParam*		SurfrevMod::ip          = NULL;
SurfrevMod *	SurfrevMod::editMod	    = NULL;
MoveModBoxCMode*    SurfrevMod::moveMode    = NULL;
RotateModBoxCMode*  SurfrevMod::rotMode 	   = NULL;
UScaleModBoxCMode*  SurfrevMod::uscaleMode  = NULL;
NUScaleModBoxCMode* SurfrevMod::nuscaleMode = NULL;
SquashModBoxCMode*  SurfrevMod::squashMode  = NULL;

IParamMap *		SurfrevMod::pmapParam = NULL;
float			SurfrevMod::dlgDegrees = DEF_DEGREES;
BOOL			SurfrevMod::dlgWeldCore = DEF_WELDCORE;
BOOL			SurfrevMod::dlgFlipNormals = DEF_FLIPNORMALS;
int				SurfrevMod::dlgSegs = DEF_SEGS;
int				SurfrevMod::dlgCapStart = DEF_CAPSTART;
int				SurfrevMod::dlgCapEnd = DEF_CAPEND;
int				SurfrevMod::dlgCapType = DEF_CAPTYPE;
int				SurfrevMod::dlgOutput = DEF_OUTPUT;
BOOL			SurfrevMod::dlgMapping = DEF_MAPPING;
int				SurfrevMod::dlgGenMatIDs = DEF_GEN_MATIDS;
int				SurfrevMod::dlgUseShapeIDs = DEF_USE_SHAPEIDS;
int				SurfrevMod::dlgSmooth = DEF_SMOOTH;

//--- Parameter map/block descriptors -------------------------------

#define PB_DEGREES		0
#define PB_SEGS			1
#define PB_CAPSTART		2
#define PB_CAPEND		3
#define PB_CAPTYPE		4
#define PB_WELDCORE		5
#define PB_OUTPUT		6
#define PB_MAPPING		7
#define PB_FLIPNORMALS	8
#define PB_GEN_MATIDS	9
#define PB_USE_SHAPEIDS	10
#define PB_SMOOTH		11

//
//
// Parameters

static int captypeIDs[] = {IDC_MORPHCAP,IDC_GRIDCAP};
static int outputIDs[] = {IDC_PATCH,IDC_MESH, IDC_NURBS};

static ParamUIDesc descParam[] = {
	// Degrees
	ParamUIDesc(
		PB_DEGREES,
		EDITTYPE_FLOAT,
		IDC_DEGREES,IDC_DEGREESSPINNER,
		MIN_DEGREES,MAX_DEGREES,
		0.5f),

	// Weld Core
	ParamUIDesc(PB_WELDCORE,TYPE_SINGLECHEKBOX,IDC_WELDCORE),

	// Segments
	ParamUIDesc(
		PB_SEGS,
		EDITTYPE_INT,
		IDC_SEGMENTS,IDC_SEGMENTSPINNER,
		(float)MIN_SEGS, (float)MAX_SEGS,	// CAL-02/14/03: Change upper limit. (FID #816)
		0.5f),

	// Capping start
	ParamUIDesc(PB_CAPSTART,TYPE_SINGLECHEKBOX,IDC_CAPSTART),

	// Capping end
	ParamUIDesc(PB_CAPEND,TYPE_SINGLECHEKBOX,IDC_CAPEND),

	// Cap type
	ParamUIDesc(PB_CAPTYPE,TYPE_RADIO,captypeIDs,2),

	// Output type
	ParamUIDesc(PB_OUTPUT,TYPE_RADIO,outputIDs,3),

	// Generate Texture coords?
	ParamUIDesc(PB_MAPPING,TYPE_SINGLECHEKBOX,IDC_GENMAPPING),

	// Flip Normals
	ParamUIDesc(PB_FLIPNORMALS,TYPE_SINGLECHEKBOX,IDC_FLIPNORMALS),

	// Multi material IDs
	ParamUIDesc(PB_GEN_MATIDS,TYPE_SINGLECHEKBOX,IDC_GEN_MATIDS),

	// Shape material IDs
	ParamUIDesc(PB_USE_SHAPEIDS,TYPE_SINGLECHEKBOX,IDC_USE_SHAPE_IDS),

	// Smoothing
	ParamUIDesc(PB_SMOOTH,TYPE_SINGLECHEKBOX,IDC_SMOOTH)
	};
#define PARAMDESC_LENGTH 12


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
	{ TYPE_INT, NULL, FALSE, 4 },
	{ TYPE_INT, NULL, FALSE, 5 } };

static ParamBlockDescID descVer2[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_INT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, FALSE, 2 },
	{ TYPE_INT, NULL, FALSE, 3 },
	{ TYPE_INT, NULL, FALSE, 4 },
	{ TYPE_INT, NULL, FALSE, 5 },
	{ TYPE_INT, NULL, FALSE, 6 } };

static ParamBlockDescID descVer3[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_INT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, FALSE, 2 },
	{ TYPE_INT, NULL, FALSE, 3 },
	{ TYPE_INT, NULL, FALSE, 4 },
	{ TYPE_INT, NULL, FALSE, 5 },
	{ TYPE_INT, NULL, FALSE, 6 },
	{ TYPE_INT, NULL, FALSE, 7 } };

static ParamBlockDescID descVer4[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_INT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, FALSE, 2 },
	{ TYPE_INT, NULL, FALSE, 3 },
	{ TYPE_INT, NULL, FALSE, 4 },
	{ TYPE_INT, NULL, FALSE, 5 },
	{ TYPE_INT, NULL, FALSE, 6 },
	{ TYPE_INT, NULL, FALSE, 7 },
	{ TYPE_INT, NULL, FALSE, 8 } };

static ParamBlockDescID descVer5[] = {
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
	{ TYPE_INT, NULL, FALSE, 9 },
	{ TYPE_INT, NULL, FALSE, 10 } };

static ParamBlockDescID descVer7[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_INT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, FALSE, 2 },
	{ TYPE_INT, NULL, FALSE, 3 },
	{ TYPE_INT, NULL, FALSE, 4 },
	{ TYPE_INT, NULL, FALSE, 5 },
	{ TYPE_INT, NULL, FALSE, 6 },
	{ TYPE_INT, NULL, FALSE, 7 },
	{ TYPE_INT, NULL, FALSE, 8 },
	{ TYPE_INT, NULL, FALSE, 9 },
	{ TYPE_INT, NULL, FALSE, 10 },
	{ TYPE_INT, NULL, FALSE, 11 } };

#define PBLOCK_LENGTH	12

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,4,0),	
	ParamVersionDesc(descVer1,6,1),
	ParamVersionDesc(descVer2,7,2),	
	ParamVersionDesc(descVer3,8,3),	
	ParamVersionDesc(descVer4,9,4),	
	ParamVersionDesc(descVer5,10,5),	
	ParamVersionDesc(descVer6,11,6)	
	};
#define NUM_OLDVERSIONS	7

// Current version
#define CURRENT_VERSION	7
static ParamVersionDesc curVersion(descVer7,PBLOCK_LENGTH,CURRENT_VERSION);

SurfrevMod::SurfrevMod()
	{

//watje new mapping ver < 4 of this modifier use the old mapping
	ver = 4;

	doAlign = FALSE;

	// Create a parameter block
	MakeRefByID(FOREVER, PBLOCKREF, 
		CreateParameterBlock(descVer7, PBLOCK_LENGTH, CURRENT_VERSION));
	pblock->SetValue(PB_DEGREES, TimeValue(0), dlgDegrees);
	pblock->SetValue(PB_SEGS, TimeValue(0), dlgSegs);
	pblock->SetValue(PB_CAPSTART, TimeValue(0), dlgCapStart);
	pblock->SetValue(PB_CAPEND, TimeValue(0), dlgCapEnd);
	pblock->SetValue(PB_CAPTYPE, TimeValue(0), dlgCapType);
	pblock->SetValue(PB_WELDCORE, TimeValue(0), dlgWeldCore);
	pblock->SetValue(PB_FLIPNORMALS, TimeValue(0), dlgFlipNormals);
	pblock->SetValue(PB_OUTPUT, TimeValue(0), dlgOutput);
	pblock->SetValue(PB_MAPPING, TimeValue(0), dlgMapping);
	pblock->SetValue(PB_GEN_MATIDS, TimeValue(0), dlgGenMatIDs);
	pblock->SetValue(PB_USE_SHAPEIDS, TimeValue(0), dlgUseShapeIDs);
	pblock->SetValue(PB_SMOOTH, TimeValue(0), dlgSmooth);

	// Create a transform controller for the surfrev axis
	MakeRefByID(FOREVER, AXISREF, NewDefaultMatrix3Controller()); 
	NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE);

	// Set the axis controller's matrix aligned with the Y axis
	SetAxis(0, Y_AXIS);

	dlgProc.SetMod(this);
	}

SurfrevMod::~SurfrevMod() {	
	DeleteAllRefsFromMe();
	}

void SurfrevMod::RescaleWorldUnits(float f) {
	if (TestAFlag(A_WORK1))
		return;
	SetAFlag(A_WORK1);
	
	// rescale all our references
	for (int i=0; i<NumRefs(); i++) {
		ReferenceMaker *srm = GetReference(i);
		if (srm) 
			srm->RescaleWorldUnits(f);
		}
	}

RefTargetHandle SurfrevMod::GetReference(int i) 
	{ 
	switch (i) {
		case AXISREF: return axisControl;
		case PBLOCKREF: return pblock;
		default: return NULL;
		}
	}

void SurfrevMod::SetReference(int i, RefTargetHandle rtarg) 
	{ 
	switch (i) {
		case AXISREF: axisControl = (Control*)rtarg; break;
		case PBLOCKREF: pblock = (IParamBlock*)rtarg; break;
		}
	}

BOOL SurfrevMod::AssignController(Animatable *control,int subAnim) {
	if(subAnim != 0)
		return FALSE;
	ReplaceReference(0,(Control*)control);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	return TRUE;
	}

Animatable* SurfrevMod::SubAnim(int i) 
	{ 
	switch (i) {
		case AXISREF: return axisControl;
		case PBLOCKREF: return pblock;		
		default: return NULL;
		}
	}

TSTR SurfrevMod::SubAnimName(int i) 
	{ 
	switch (i) {
		case AXISREF: return TSTR(GetString(IDS_TH_AXIS));
		case PBLOCKREF: return TSTR(GetString(IDS_RB_PARAMETERS));
		default: return TSTR(_T(""));
		}	
	}

Interval SurfrevMod::LocalValidity(TimeValue t)
	{
	// if being edited, return NEVER forces a cache to be built 
	// after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;  
	Interval valid = GetValidity(t);	
	Matrix3 mat;	
	mat.IdentityMatrix();		
	axisControl->GetValue(t,&mat,valid,CTRL_RELATIVE);
	return valid;
	}

RefTargetHandle SurfrevMod::Clone(RemapDir& remap)
	{
	SurfrevMod* newmod = new SurfrevMod();	
	newmod->ReplaceReference(PBLOCKREF,pblock->Clone(remap));
	newmod->ReplaceReference(AXISREF, axisControl->Clone()); 
	BaseClone(this, newmod, remap);
	return(newmod);
	}

//  Move, Rotate, and Scale

void SurfrevMod::Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin ) {
	if ( ip && ip->GetSubObjectLevel()==1 ) {
		SetXFormPacket pckt(val,partm,tmAxis);
		axisControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);		
		}
	}

void SurfrevMod::Rotate( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin ) {
	SetXFormPacket pckt(val,localOrigin,partm,tmAxis);
	axisControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);		
	}

void SurfrevMod::Scale( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin ) {
	SetXFormPacket pckt(val,localOrigin,partm,tmAxis);
	axisControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);		
	}

Matrix3 SurfrevMod::CompMatrix(TimeValue t, ModContext& mc, Matrix3& ntm, 
		Interval& valid, BOOL needOffset) {
	Matrix3 tm;
	
	if (mc.tm) {
		tm = *mc.tm;		
		}
	else 
		tm.IdentityMatrix();	
	Matrix3 mat;
	mat.IdentityMatrix();
	axisControl->GetValue(t,&mat,valid,CTRL_RELATIVE);
	tm = tm*Inverse(mat);		

	return Inverse(tm)*ntm;	
	}

int SurfrevMod::HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc) {
	GraphicsWindow *gw = vpt->getGW();
	Interval valid;
	int savedLimits;	
	HitRegion hr;
	MakeHitRegion(hr,type, crossing,4,p);
	gw->setHitRegion(&hr);
	Matrix3 modmat, ntm = inode->GetObjectTM(t);
		
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
	gw->clearHitCode();
	
	if (ip && ip->GetSubObjectLevel() == 1) {
		modmat = CompMatrix(t,*mc,ntm,valid,FALSE);
		gw->setTransform(modmat);
		Point3 pt[3];
        ObjectState os = inode->EvalWorldState(t);
        ShapeObject *shape = (ShapeObject *)os.obj;
        // Get our axis orientation
        Matrix3 axis(TRUE);
        axisControl->GetValue(t, &axis, FOREVER, CTRL_RELATIVE);
        Box3 bbox;
        Matrix3 iaxis = Inverse(axis);
        shape->GetDeformBBox(t, bbox, &iaxis);
		pt[0] = Point3(0.0f,0.0f,bbox.Min().z);
		pt[1] = Point3(0.0f,0.0f,bbox.Max().z);
		gw->polyline(2, pt, NULL, NULL, 1, NULL);
    }
    
	gw->setRndLimits(savedLimits);	
	if (gw->checkHitCode()) {
		vpt->LogHit(inode, mc, gw->getHitDistance(), 0, NULL); 
		return 1;
		}
	return 0;
	}

int SurfrevMod::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc) {
	Interval valid;
	GraphicsWindow *gw = vpt->getGW();
#ifdef DESIGN_VER
	// Transform the gizmo with the node.
	TimeValue rt = GetCOREInterface()->GetTime();
	Matrix3 modmat, ntm = inode->GetObjectTM(rt);
#else
	Matrix3 modmat, ntm = inode->GetObjectTM(t);
#endif

	modmat = CompMatrix(t,*mc,ntm,valid,FALSE);
	gw->setTransform(modmat);
	if ( ip && ip->GetSubObjectLevel() == 1 ) {		
		gw->setColor( LINE_COLOR, (float)1.0, (float)1.0, (float)0.0);
		}
	Point3 pt[2];
	ObjectState os = inode->EvalWorldState(t);
	ShapeObject *shape = (ShapeObject *)os.obj;
	// Get our axis orientation
	Matrix3 axis(TRUE);
	axisControl->GetValue(t, &axis, FOREVER, CTRL_RELATIVE);
	Box3 bbox;
	Matrix3 iaxis = Inverse(axis);
	shape->GetDeformBBox(t, bbox, &iaxis);
	pt[0] = Point3(0.0f,0.0f,bbox.Min().z);
	pt[1] = Point3(0.0f,0.0f,bbox.Max().z);
	gw->polyline(2, pt, NULL, NULL, 1, NULL);
	
	UpdateUI(t);
	return 0;	
	}

void SurfrevMod::GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc) {
	Interval valid;
	GraphicsWindow *gw = vpt->getGW();
#ifdef DESIGN_VER
	// Need the correct bound box for proper damage rect calcs.
	TimeValue rt = GetCOREInterface()->GetTime();
	Matrix3 modmat, ntm = inode->GetObjectTM(rt);
#else
	Matrix3 modmat, ntm = inode->GetObjectTM(t);
#endif

	ObjectState os = inode->EvalWorldState(t);
	ShapeObject *shape = (ShapeObject *)os.obj;
	// Get our axis orientation
	Matrix3 axis(TRUE);
	axisControl->GetValue(t, &axis, FOREVER, CTRL_RELATIVE);
	Box3 bbox;
	Matrix3 iaxis = Inverse(axis);
	shape->GetDeformBBox(t, bbox, &iaxis);
	modmat = CompMatrix(t,*mc,ntm,valid,FALSE);
	box.Init();
	box = bbox * modmat;
	}

void SurfrevMod::GetSubObjectCenters(
		SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc)
	{
	Interval valid;
	Matrix3 modmat, tm, ntm = node->GetObjectTM(t,&valid);

	if (cb->Type()==SO_CENTER_PIVOT) {
		tm = CompMatrix(t,*mc,ntm,valid,TRUE);
		cb->Center(tm.GetTrans(),0);
	} else {
		modmat = CompMatrix(t,*mc,ntm,valid,FALSE);
		cb->Center(Point3(0,0,0)*modmat,0);
		}
	}

void SurfrevMod::GetSubObjectTMs(
		SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc)
	{
	Interval valid;
	Matrix3 ntm = node->GetObjectTM(t,&valid);
	Matrix3 tm = CompMatrix(t,*mc,ntm,valid,TRUE);
	cb->TM(tm,0);
	}

void SurfrevMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) 
	{	
	if(doAlign) {
		// Get our axis orientation
		Matrix3 axis(TRUE);
		axisControl->GetValue(t, &axis, FOREVER, CTRL_RELATIVE);
		doAlign = FALSE;
		// Let's get the bounds of the object in the current space
		ShapeObject *shape = (ShapeObject *)os->obj;
		Box3 bbox;
		Matrix3 iaxis = Inverse(axis);
		shape->GetDeformBBox(t, bbox, &iaxis);
		// Use the X axis for the alignment
		Point3 pos(0,0,0);
		switch(alignType) {
			case ALIGN_MIN:
				pos = Point3(bbox.pmin.x, bbox.Center().y, bbox.Center().z);
				break;
			case ALIGN_CENTER:
				pos = bbox.Center();
				break;
			case ALIGN_MAX:
				pos = Point3(bbox.pmax.x, bbox.Center().y, bbox.Center().z);
				break;
			}
		axis.SetTrans(pos * axis);
		axisControl->SetValue(t, &SetXFormPacket(axis));
		}
	 		
//DebugPrint("Surfrev modifying object\n");

	// Get our personal validity interval...
	Interval valid = GetValidity(t);
	// and intersect it with the channels we use as input (see ChannelsUsed)
	valid &= os->obj->ChannelValidity(t,TOPO_CHAN_NUM);
	valid &= os->obj->ChannelValidity(t,GEOM_CHAN_NUM);

	// Get the object TM
	Matrix3 objTM(TRUE);
	if(mc.tm)
		objTM = *mc.tm;
	
	// Get our axis orientation
	Matrix3 axis(TRUE);
	axisControl->GetValue(t, &axis, valid, CTRL_RELATIVE);

	int output;
	pblock->GetValue(PB_OUTPUT, TimeValue(0), output, FOREVER);

	// Here is where all the fun stuff happens
	switch (output) {
	case NURBS_OUTPUT:
#ifndef NO_NURBS
		{
		// Here is where all the fun stuff happens -- GenerateLatheSurface fills in the EM,
		// then we stuff the EditableSurface into the pipeline.
		ShapeObject *shape = (ShapeObject *)os->obj;

		BOOL texturing, genMatIds, useShapeIDs;
		pblock->GetValue(PB_MAPPING, TimeValue(0), texturing, FOREVER);
		pblock->GetValue(PB_GEN_MATIDS, TimeValue(0), genMatIds, FOREVER);
		pblock->GetValue(PB_USE_SHAPEIDS, TimeValue(0), useShapeIDs, FOREVER);

		float degrees;
		int capStart, capEnd, capType, segs;
		BOOL weldCore, flipNormals;

		pblock->GetValue(PB_SEGS,t,segs,FOREVER);
		pblock->GetValue(PB_CAPSTART,t,capStart,FOREVER);
		pblock->GetValue(PB_CAPEND,t,capEnd,FOREVER);
		pblock->GetValue(PB_CAPTYPE,t,capType,FOREVER);
		pblock->GetValue(PB_WELDCORE,t,weldCore,FOREVER);
		pblock->GetValue(PB_FLIPNORMALS,t,flipNormals,FOREVER);

		pblock->GetValue(PB_DEGREES,t,degrees,FOREVER);
		LimitValue(degrees, MIN_DEGREES, MAX_DEGREES);

		BOOL suspended = FALSE;
		if (theHold.Holding()) {
			theHold.Suspend();
			suspended = TRUE;
		}
		Object *nobj = CreateNURBSLatheShape(ip, GetString(IDS_TH_LATHE_CLASS),
						t, shape, axis * Inverse(objTM), degrees, capStart, capEnd, capType,
						weldCore, flipNormals, texturing, segs, genMatIds, useShapeIDs);
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
		// BuildPatchFromShape fills in the PatchObject's patch mesh,
		// then we stuff the PatchObject into the pipeline.
		PatchObject *pat = new PatchObject;
		BuildPatchFromShape(t, mc, os, axis * Inverse(objTM), pat->patch);

		pat->SetChannelValidity(TOPO_CHAN_NUM, valid);
		pat->SetChannelValidity(GEOM_CHAN_NUM, valid);
		pat->SetChannelValidity(TEXMAP_CHAN_NUM, valid);
		pat->SetChannelValidity(MTL_CHAN_NUM, valid);
		pat->SetChannelValidity(SELECT_CHAN_NUM, valid);
		pat->SetChannelValidity(SUBSEL_TYPE_CHAN_NUM, valid);
		pat->SetChannelValidity(DISP_ATTRIB_CHAN_NUM, valid);

		os->obj = pat;
		break; }
#endif // NO_PATCHES
	case MESH_OUTPUT: {
		// BuildMeshFromShape fills in the TriObject's mesh,
		// then we stuff the TriObj into the pipeline.
		TriObject *tri = CreateNewTriObject();
		BuildMeshFromShape(t, mc, os, axis * Inverse(objTM), tri->GetMesh());

		tri->SetChannelValidity(TOPO_CHAN_NUM, valid);
		tri->SetChannelValidity(GEOM_CHAN_NUM, valid);
		tri->SetChannelValidity(TEXMAP_CHAN_NUM, valid);
		tri->SetChannelValidity(MTL_CHAN_NUM, valid);
		tri->SetChannelValidity(SELECT_CHAN_NUM, valid);
		tri->SetChannelValidity(SUBSEL_TYPE_CHAN_NUM, valid);
		tri->SetChannelValidity(DISP_ATTRIB_CHAN_NUM, valid);

		os->obj = tri;
		break;}
	}

	os->obj->UnlockObject();
//DebugPrint("Surfrev modification complete!\n");
	}


void SurfrevMod::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
	{
	this->ip = ip;
	editMod = this;

	// Add our sub object type
	// TSTR type1( GetString(IDS_TH_AXIS) );
	// const TCHAR *ptype[] = { type1 };
	// This call is obsolete. Please see BaseObject::NumSubObjTypes() and BaseObject::GetSubObjType()
	// ip->RegisterSubObjectTypes( ptype, 1 );

	// Create sub object editing modes.
	moveMode    = new MoveModBoxCMode(this,ip);
	rotMode     = new RotateModBoxCMode(this,ip);
	uscaleMode  = new UScaleModBoxCMode(this,ip);
	nuscaleMode = new NUScaleModBoxCMode(this,ip);
	squashMode  = new SquashModBoxCMode(this,ip);	
	
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);

	pmapParam = CreateCPParamMap(
		descParam,PARAMDESC_LENGTH,
		pblock,
		ip,
		hInstance,
		MAKEINTRESOURCE(IDD_SURFREVPARAM),
		GetString(IDS_RB_PARAMETERS),
		0);	
	pmapParam->SetUserDlgProc(&dlgProc);
	}

void SurfrevMod::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
	{
	this->ip = NULL;
	editMod = NULL;
	
	TimeValue t = ip->GetTime();

	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);

	DestroyCPParamMap(pmapParam);

	ip->DeleteMode(moveMode);
	ip->DeleteMode(rotMode);
	ip->DeleteMode(uscaleMode);
	ip->DeleteMode(nuscaleMode);
	ip->DeleteMode(squashMode);	
	if ( moveMode ) delete moveMode;
	moveMode = NULL;
	if ( rotMode ) delete rotMode;
	rotMode = NULL;
	if ( uscaleMode ) delete uscaleMode;
	uscaleMode = NULL;
	if ( nuscaleMode ) delete nuscaleMode;
	nuscaleMode = NULL;
	if ( squashMode ) delete squashMode;
	squashMode = NULL;

	// Save these values in class variables so the next object created will inherit them.
	pblock->GetValue(PB_DEGREES,ip->GetTime(),dlgDegrees,FOREVER);
	pblock->GetValue(PB_SEGS,ip->GetTime(),dlgSegs,FOREVER);
	pblock->GetValue(PB_CAPSTART,ip->GetTime(),dlgCapStart,FOREVER);
	pblock->GetValue(PB_CAPEND,ip->GetTime(),dlgCapEnd,FOREVER);
	pblock->GetValue(PB_CAPTYPE,ip->GetTime(),dlgCapType,FOREVER);
	pblock->GetValue(PB_WELDCORE,ip->GetTime(),dlgWeldCore,FOREVER);
	pblock->GetValue(PB_FLIPNORMALS,ip->GetTime(),dlgFlipNormals,FOREVER);
	pblock->GetValue(PB_OUTPUT,ip->GetTime(),dlgOutput,FOREVER);
	pblock->GetValue(PB_MAPPING,ip->GetTime(),dlgMapping,FOREVER);
	pblock->GetValue(PB_GEN_MATIDS,ip->GetTime(),dlgGenMatIDs,FOREVER);
	pblock->GetValue(PB_USE_SHAPEIDS,ip->GetTime(),dlgUseShapeIDs,FOREVER);
	pblock->GetValue(PB_SMOOTH,ip->GetTime(),dlgSmooth,FOREVER);
	}

Interval SurfrevMod::GetValidity(TimeValue t)
	{
	float f;
	int i;
	Interval valid = FOREVER;
	pblock->GetValue(PB_DEGREES,t,f,valid);
	pblock->GetValue(PB_SEGS,t,i,valid);	
	pblock->GetValue(PB_CAPSTART,t,i,valid);
	pblock->GetValue(PB_CAPEND,t,i,valid);
	pblock->GetValue(PB_CAPTYPE,t,i,valid);
	pblock->GetValue(PB_WELDCORE,t,i,valid);
	pblock->GetValue(PB_FLIPNORMALS,t,i,valid);
	pblock->GetValue(PB_OUTPUT,t,i,valid);
	pblock->GetValue(PB_MAPPING,t,i,valid);
	return valid;
	}

void SurfrevMod::ActivateSubobjSel(int level, XFormModes& modes )
	{	
	switch ( level ) {
		case 1: // Axis
			modes = XFormModes(moveMode,rotMode,nuscaleMode,uscaleMode,squashMode,NULL);
			break;
		}
	NotifyDependents(FOREVER,PART_DISPLAY,REFMSG_CHANGE);
	}

RefResult SurfrevMod::NotifyRefChanged(
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

ParamDimension *SurfrevMod::GetParameterDim(int pbIndex)
	{
	switch (pbIndex) {
		case PB_DEGREES: 	return defaultDim; // Note: doesn't use angleDim because the lathe has been storing the angle in degrees not radians. Changing this would invalidate old files.
		case PB_SEGS:		return defaultDim;
		case PB_CAPSTART:	return defaultDim;
		case PB_CAPEND:		return defaultDim;
		case PB_CAPTYPE:	return defaultDim;
		case PB_WELDCORE:	return defaultDim;
		case PB_FLIPNORMALS:	return defaultDim;
		default:			return defaultDim;
		}
	}

TSTR SurfrevMod::GetParameterName(int pbIndex)
	{
	switch (pbIndex) {
		case PB_DEGREES:	return TSTR(GetString(IDS_TH_DEGREES));
		case PB_SEGS:		return TSTR(GetString(IDS_TH_SEGMENTS));
		default:			return TSTR(_T(""));
		}
	}

BOOL SurfrevMod::HasUVW() { 
	BOOL genUVs;
	Interval v;
	pblock->GetValue(PB_MAPPING, 0, genUVs, v);
	return genUVs; 
	}

void SurfrevMod::SetGenUVW(BOOL sw) {  
	if (sw==HasUVW()) return;
	pblock->SetValue(PB_MAPPING,0, sw);				
	}

BOOL SurfrevMod::GetGenMatIDs() {
	int sw;
	Interval v;
	pblock->GetValue(PB_GEN_MATIDS, 0, sw, v);
	return sw;
	}

class SurfrevPostLoadCallback : public PostLoadCallback {
	public:
		ParamBlockPLCB *cb;
		SurfrevPostLoadCallback(ParamBlockPLCB *c) {cb=c;}
		void proc(ILoad *iload);
	};

void SurfrevPostLoadCallback::proc(ILoad *iload) {
	DWORD oldVer = ((SurfrevMod*)(cb->targ))->pblock->GetVersion();
	ReferenceTarget *targ = cb->targ;
	cb->proc(iload);
	if (oldVer<5)			
		((SurfrevMod*)targ)->pblock->SetValue(PB_GEN_MATIDS,0,FALSE);
	if (oldVer<6)
		((SurfrevMod*)targ)->pblock->SetValue(PB_USE_SHAPEIDS,0,FALSE);
	if (oldVer<7)
		((SurfrevMod*)targ)->pblock->SetValue(PB_SMOOTH,0,TRUE);

// russom - 08/08/01
// If file loaded has the lathe output set to NURBS, set it Mesh
#ifdef NO_NURBS
	int output;
	((SurfrevMod*)targ)->pblock->GetValue(PB_OUTPUT, TimeValue(0), output, FOREVER);

	if( output == NURBS_OUTPUT )
		((SurfrevMod*)targ)->pblock->SetValue(PB_OUTPUT, TimeValue(0), MESH_OUTPUT);
#endif

// russom - 08/08/01
// If file loaded has the lathe output set to Patches, set it Mesh
#ifdef NO_PATCHES
	int outputPatch;
	((SurfrevMod*)targ)->pblock->GetValue(PB_OUTPUT, TimeValue(0), outputPatch, FOREVER);

	if( outputPatch == PATCH_OUTPUT )
		((SurfrevMod*)targ)->pblock->SetValue(PB_OUTPUT, TimeValue(0), MESH_OUTPUT);
#endif

	delete this;
	}
#define VERSION_CHUNK 0x100

//watje new mapping ver < 4 of this modifier use the old mapping
IOResult SurfrevMod::Save(ISave *isave)
	{
	ULONG nb;
	Modifier::Save(isave);

	isave->BeginChunk(VERSION_CHUNK);
	isave->Write(&ver, sizeof(ver), &nb);
	isave->EndChunk();
	return IO_OK;
	}


IOResult SurfrevMod::Load(ILoad *iload)
	{
	Modifier::Load(iload);

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
		new SurfrevPostLoadCallback(
			new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,PBLOCKREF)));
	return IO_OK;
	}

class ShapeDims {
	public:
	float minx, maxx, xsize, coreThresh;
	ShapeDims() { minx = maxx = xsize = coreThresh = 0.0f; }
	};

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

void SurfrevMod::BuildMeshFromShape(TimeValue t,ModContext &mc, ObjectState * os, Matrix3 &axis, Mesh &mesh) {
	BOOL texturing;
	pblock->GetValue(PB_MAPPING, TimeValue(0), texturing, FOREVER);
	BOOL genMatIDs;
	pblock->GetValue(PB_GEN_MATIDS, TimeValue(0), genMatIDs, FOREVER);
	BOOL useShapeIDs;
	pblock->GetValue(PB_USE_SHAPEIDS, TimeValue(0), useShapeIDs, FOREVER);
	BOOL smooth;
	pblock->GetValue(PB_SMOOTH, TimeValue(0), smooth, FOREVER);
	
	ShapeObject *shape = (ShapeObject *)os->obj;

	float degrees;
	int levels,capStart,capEnd,capType;
	BOOL weldCore, flipNormals;

	pblock->GetValue(PB_DEGREES,t,degrees,FOREVER);
	pblock->GetValue(PB_SEGS,t,levels,FOREVER);
	pblock->GetValue(PB_CAPSTART,t,capStart,FOREVER);
	pblock->GetValue(PB_CAPEND,t,capEnd,FOREVER);
	pblock->GetValue(PB_CAPTYPE,t,capType,FOREVER);
	pblock->GetValue(PB_WELDCORE,t,weldCore,FOREVER);
	pblock->GetValue(PB_FLIPNORMALS,t,flipNormals,FOREVER);

	LimitValue(degrees, MIN_DEGREES, MAX_DEGREES);
	LimitValue(levels, MIN_SEGS, MAX_SEGS);		// CAL-02/14/03: Change upper limit. (FID #816)

	BOOL fullCircle = (degrees == 360.0f) ? TRUE : FALSE;

	// Get the basic dimension stuff
	float radians = (float)fabs(degrees) * DEG_TO_RAD;

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
	int vertLevels = levels + (fullCircle ? 0 : 1);
	int TVlevels = levels + 1;

	verts = levelVerts * vertLevels;
	tVerts = levelTVerts * TVlevels;
	faces = levelFaces * levels;

	mesh.setNumVerts(verts);
	mesh.setNumFaces(faces);
	if(texturing) {
		mesh.setNumTVerts(tVerts);
		mesh.setNumTVFaces(faces);
		}

	// Set up the rotational axis transform
	Matrix3 iaxis = Inverse(axis);

	// Set up a transform that will back out everything but our rotation
	// about the axis
	Matrix3 itm = Inverse(iaxis * RotateZMatrix(0.0f) * axis);

	// A flag to detect whether this will be an inside-out object
	BOOL inverse = FALSE;

	// Make a bit array which indicates which verts get welded
	BitArray weldFlags(levelVerts);
	weldFlags.ClearAll();

	// Get information on the shapes
	ShapeDims *sDims = new ShapeDims [polys];
	for(poly = 0; poly < polys; ++poly) {
		ShapeDims &dim = sDims[poly];
		PolyLine &line = pShape.lines[poly];
		if(!line.numPts)
			continue;
		int lverts = line.numPts;
		for(int v = 0; v < lverts; ++v) {
			PolyPt &pp = line[v];
			Point3 test = pp.p * iaxis;
			if(v == 0 || test.x < dim.minx)
				dim.minx = test.x;
			if(v == 0 || test.x > dim.maxx)
				dim.maxx = test.x;
			}
		dim.xsize = dim.maxx - dim.minx;
		dim.coreThresh = dim.xsize / 100.0f;
		if(dim.coreThresh > 0.01f)
			dim.coreThresh = 0.01f;
		}

	// Create the vertices!
	int vert = 0;
	Matrix3 mat1, mat2;
	for(int level = 0; level < vertLevels; ++level) {
		int levelBase = vert;
		// Create a matrix for the operation
		Matrix3 tm = iaxis * RotateZMatrix((float)level / (float)levels * radians) * axis;
		if(level == 0)
			mat1 = tm;
		else
		if(level == (vertLevels - 1))
			mat2 = tm;
#ifdef TEST_TAPER
		// For testing -- tapers down toward end
		float scale = 1.0f - (float)level / 2.0f / (float)vertLevels;
#endif //TEST_TAPER
		for(poly = 0; poly < polys; ++poly) {
			ShapeDims &dim = sDims[poly];
			PolyLine &line = pShape.lines[poly];
			if(!line.numPts)
				continue;
			int lverts = line.numPts;
			for(int v = 0; v < lverts; ++v) {
				PolyPt &pp = line[v];
				Point3 test = pp.p * iaxis;
				if(weldCore && level == 0) {
					if(fabs(test.x) < dim.coreThresh)
						weldFlags.Set(vert);
					}
#ifdef TEST_TAPER
				Point3 p = pp.p * scale * tm;
#else
				Point3 p = pp.p * tm;
#endif //TEST_TAPER
				mesh.setVert(vert++, p);
				if(level == 1 && !inverse && (float)fabs(test.x) > dim.coreThresh) {
					Point3 ip = p * itm;
					if(ip.z > 0.0f)
						inverse = TRUE;
					}
				}
			}
		}

	// Flip 'inverse' state if flipNormals is true
	if(flipNormals)
		inverse = 1 - inverse;

	delete [] sDims;

	assert(vert == verts);

	// May need to add the texture verts
	if(texturing) {
		int tvertex = 0;
		for(int level = 0; level < TVlevels; ++level) {
			float tV = (float)level / (float)(TVlevels - 1);
			for(poly = 0; poly < polys; ++poly) {
				PolyLine &line = pShape.lines[poly];
				if(!line.numPts)
					continue;
				int lverts = line.numPts;
				int tp;
				int texPts = line.numPts + (line.IsClosed() ? 1 : 0);
				float cumLen = 0.0f;
				float totLen = line.CurveLength();
				Point3 prevPt = line.pts[0].p;
				for(tp = 0; tp < texPts; ++tp) {
					float tU;
					int ltp = tp % line.numPts;
					if(tp == (texPts - 1))
						tU = 1.0f;
					else {
						Point3 &pt = line.pts[ltp].p;
						cumLen += Length(pt - prevPt);
						tU = cumLen / totLen;
						prevPt = pt;
						}
					mesh.setTVert(tvertex++, UVVert(tU, tV, 0.0f));
					}
				}
			}
		}

	// If capping, do it!
	if(degrees < 360.0f && anyClosed && (capStart || capEnd)) {
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
				}
			int oldFaces = mesh.numFaces;
			capper.CapMesh(mesh, capInfo, inverse, 16, &mat1, genMatIDs ? 1: 0);
			// If texturing, create the texture faces and vertices
			if(texturing)
				MakeMeshCapTexture(mesh, Inverse(mat1), oldFaces, mesh.numFaces);
			}
		if(capEnd) {
			vert = levelVerts * levels;
			for(poly = 0; poly < polys; ++poly) {
				PolyLine &line = pShape.lines[poly];
				if(!line.numPts)
					continue;
				MeshCapPoly &capline = capper[poly];
				int lverts = line.numPts;
				for(int v = 0; v < lverts; ++v)
					capline.SetVert(v, vert++);			// Gives this vert's location in the mesh!
				}
			int oldFaces = mesh.numFaces;
			capper.CapMesh(mesh, capInfo, 1-inverse, 16, &mat2, 0);
			// If texturing, create the texture faces and vertices
			if(texturing)
				MakeMeshCapTexture(mesh, Inverse(mat2), oldFaces, mesh.numFaces);
			}
		}

	// Create the faces!
	int face = 0;
	int baseVert = 0;
	int TVface = 0;
	int baseTVert = 0;
	for(level = 0; level < levels; ++level) {
		for(poly = 0; poly < polys; ++poly) {
			PolyLine &line = pShape.lines[poly];
			if(!line.numPts)
				continue;
			int pieces = line.Segments();
			int closed = line.IsClosed();
			int segVerts = pieces + ((closed) ? 0 : 1);
			int segTVerts = pieces + 1;
			int sm = 0;		// Initial smoothing group
			BOOL firstSmooth = (line.pts[0].flags & POLYPT_SMOOTH) ? TRUE : FALSE;
			for(piece = 0; piece < pieces; ++piece) {
				
				// Get vertex indices
				int v1 = baseVert + piece;
				int v2 = baseVert + ((piece + 1) % segVerts);
				int v3 = (v1 + levelVerts) % verts;
				int v4 = (v2 + levelVerts) % verts;

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
				if(inverse) {
					mesh.faces[face].setEdgeVisFlags(1,1,0);
					mesh.faces[face].setSmGroup(smooth ? smoothGroup : 0);
					if(genMatIDs)
						mesh.faces[face].setMatID(mtl);
					mesh.faces[face++].setVerts(v1, v2, v4);
					mesh.faces[face].setEdgeVisFlags(0,1,1);
					mesh.faces[face].setSmGroup(smooth ? smoothGroup : 0);
					if(genMatIDs)
						mesh.faces[face].setMatID(mtl);
					mesh.faces[face++].setVerts(v1, v4, v3);
					}
				else {
					mesh.faces[face].setEdgeVisFlags(0,1,1);
					mesh.faces[face].setSmGroup(smooth ? smoothGroup : 0);
					if(genMatIDs)
						mesh.faces[face].setMatID(mtl);
					mesh.faces[face++].setVerts(v1, v4, v2);
					mesh.faces[face].setEdgeVisFlags(1,1,0);
					mesh.faces[face].setSmGroup(smooth ? smoothGroup : 0);
					if(genMatIDs)
						mesh.faces[face].setMatID(mtl);
					mesh.faces[face++].setVerts(v1, v3, v4);
					}
//DebugPrint("BV:%d V:%d v1:%d v2:%d v3:%d v4:%d\n",baseVert, vert, v1, v2, v3, v4);
				// Create texture faces, if necessary
				if(texturing) {
					int tv1 = baseTVert + piece;
					int tv2 = baseTVert + piece + 1;
					int tv3 = tv1 + levelTVerts;
					int tv4 = tv2 + levelTVerts;
					if(inverse) {
						mesh.tvFace[TVface++].setTVerts(tv1, tv2, tv4);
						mesh.tvFace[TVface++].setTVerts(tv1, tv4, tv3);
						}
					else {
						mesh.tvFace[TVface++].setTVerts(tv1, tv4, tv2);
						mesh.tvFace[TVface++].setTVerts(tv1, tv3, tv4);
						}
					}
				}
			baseVert += segVerts;
			baseTVert += segTVerts;
			}
		}
	assert(face == faces);

	// If welding, weld all those core points
	if(weldCore && weldFlags.NumberSet()) {
		int verts = mesh.getNumVerts();
		int faces = mesh.getNumFaces();
		int tverts = mesh.getNumTVerts();
		BitArray faceDel(faces);
		faceDel.ClearAll();
		// First go thru and redirect all welded points to the first instance,
		// plus delete the faces which are degenerate
		int outFace = 0;
		for(int i = 0; i < faces; ++i) {
			Face &f = mesh.faces[i];
			DWORD a = f.v[0], b=f.v[1], c=f.v[2];
			if(weldFlags[a % levelVerts])
				a = a % levelVerts;
			if(weldFlags[b % levelVerts])
				b = b % levelVerts;
			if(weldFlags[c % levelVerts])
				c = c % levelVerts;
			if(a != b && b != c && a != c) {
				f.v[0] = a;
				f.v[1] = b;
				f.v[2] = c;
				mesh.faces[outFace++] = f;
				}
			else
				faceDel.Set(i);
			}
		// Also do the texture faces
		if(mesh.tvFace && faceDel.NumberSet()) {
			int outTVFace = 0;
			for(i = 0; i < faces; ++i) {
				if(!faceDel[i])
					mesh.tvFace[outTVFace++] = mesh.tvFace[i];
				}
			}
		mesh.setNumFaces(outFace, TRUE);
		mesh.setNumTVFaces(outFace, TRUE, faces);
		// Tag the verts we need to keep
		BitArray vertDel(verts);
		vertDel.SetAll();
		for(i = 0; i < outFace; ++i) {
			Face &f = mesh.faces[i];
			vertDel.Clear(f.v[0]);
			vertDel.Clear(f.v[1]);
			vertDel.Clear(f.v[2]);
			}
		// Repack the vertex array
		DWORD *vredir = new DWORD[verts];
		int outVert = 0;
		for(i = 0; i < verts; ++i) {
			if(!vertDel[i]) {
				vredir[i] = outVert;
				mesh.verts[outVert++] = mesh.verts[i];
				}
			}
		mesh.setNumVerts(outVert, TRUE);
		// Now repoint the face vertices
		for(i = 0; i < outFace; ++i) {
			Face &f = mesh.faces[i];
			f.v[0] = vredir[f.v[0]];
			f.v[1] = vredir[f.v[1]];
			f.v[2] = vredir[f.v[2]];
			}
		delete [] vredir;
		// Tag the tverts we need to keep
		if(tverts && mesh.tvFace) {
			BitArray tvertDel(tverts);
			tvertDel.SetAll();
			for(i = 0; i < outFace; ++i) {
				TVFace &f = mesh.tvFace[i];
				tvertDel.Clear(f.t[0]);
				tvertDel.Clear(f.t[1]);
				tvertDel.Clear(f.t[2]);
				}
			// Repack the tvertex array
			DWORD *tvredir = new DWORD[tverts];
			int outTVert = 0;
			for(i = 0; i < tverts; ++i) {
				if(!tvertDel[i]) {
					tvredir[i] = outTVert;
					mesh.tVerts[outTVert++] = mesh.tVerts[i];
					}
				}
			mesh.setNumTVerts(outTVert, TRUE);
			// Now repoint the tface vertices
			for(i = 0; i < outFace; ++i) {
				TVFace &f = mesh.tvFace[i];
				f.t[0] = tvredir[f.t[0]];
				f.t[1] = tvredir[f.t[1]];
				f.t[2] = tvredir[f.t[2]];
				}
			delete [] tvredir;
			}
		}

	mesh.InvalidateGeomCache();
	}

void SurfrevMod::SetAxis(TimeValue t, int type) {
	switch(type) {
		case X_AXIS:
			axisControl->SetValue(t, &SetXFormPacket(RotateYMatrix(HALFPI)));
			break;
		case Y_AXIS:
			axisControl->SetValue(t, &SetXFormPacket(RotateXMatrix(-HALFPI)));
			break;
		case Z_AXIS:
			axisControl->SetValue(t, &SetXFormPacket(Matrix3(TRUE)));
			break;
		}
//	pmapParam->Invalidate();
	}

void SurfrevMod::Align(TimeValue t, int type) {
	// Set up a flag to make us align on the next ModifyObject
	doAlign = TRUE;
	alignType = type;
	// Force us to evaluate!
	NotifyDependents(Interval(t,t), 
			PART_GEOM|SELECT_CHANNEL|PART_SUBSEL_TYPE|PART_DISPLAY|PART_TOPO,
		    REFMSG_MOD_EVAL);
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

/* Find the vector length for a circle segment	*/
/* Returns a unit value (radius=1.0)		*/
/* Angle expressed in radians			*/
#ifndef NO_PATCHES
static float
veccalc(float angstep) {
	static float lastin = -9999.0f,lastout;
	if(lastin == angstep)
		return lastout;

	float lo,hi,totdist;
	float sinfac=(float)sin(angstep),cosfac=(float)cos(angstep),test;
	int ix,count;
	Spline3D work;
	Point3 k1((float)cos(0.0f),(float)sin(0.0f),0.0f);
	Point3 k2(cosfac,sinfac,0.0f);

	hi=1.5f;
	lo=0.0f;
	count=200;

	/* Loop thru test vectors */

	loop:
	work.NewSpline();
	test=(hi+lo)/2.0f;
	Point3 out = k1 + Point3(0.0f, test, 0.0f);
	Point3 in = k2 + Point3(sinfac * test, -cosfac * test, 0.0f);

 	work.AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_CURVE,k1,k1,out));
 	work.AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_CURVE,k2,in,k2));

	totdist=0.0f;
	for(ix=0; ix<10; ++ix) {
		Point3 terp = work.InterpBezier3D(0,(float)ix/10.0f);
		totdist += (float)sqrt(terp.x * terp.x + terp.y * terp.y);
		}
	
	totdist /= 10.0f;
	count--;
	if(totdist==1.0f || count<=0)
		goto done;
	if(totdist>1.0f) {
		hi=test;
		goto loop;
		}
	lo=test;
	goto loop;

	done:
	lastin = angstep;
	lastout = test;
	return test;
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

void SurfrevMod::BuildPatchFromShape(TimeValue t,ModContext &mc, ObjectState * os, Matrix3 &axis, PatchMesh &pmesh) {
	ShapeObject *shape = (ShapeObject *)os->obj;

	float degrees;
	int levels,capStart,capEnd,capType;
	BOOL weldCore, flipNormals;

	pblock->GetValue(PB_DEGREES,t,degrees,FOREVER);
	pblock->GetValue(PB_SEGS,t,levels,FOREVER);
	pblock->GetValue(PB_CAPSTART,t,capStart,FOREVER);
	pblock->GetValue(PB_CAPEND,t,capEnd,FOREVER);
	pblock->GetValue(PB_CAPTYPE,t,capType,FOREVER);
	pblock->GetValue(PB_WELDCORE,t,weldCore,FOREVER);
	pblock->GetValue(PB_FLIPNORMALS,t,flipNormals,FOREVER);

	BOOL texturing;
	pblock->GetValue(PB_MAPPING, TimeValue(0), texturing, FOREVER);
	BOOL genMatIDs;
	pblock->GetValue(PB_GEN_MATIDS, TimeValue(0), genMatIDs, FOREVER);
	BOOL useShapeIDs;
	pblock->GetValue(PB_USE_SHAPEIDS, TimeValue(0), useShapeIDs, FOREVER);
	BOOL smooth;
	pblock->GetValue(PB_SMOOTH, TimeValue(0), smooth, FOREVER);

	LimitValue(degrees, MIN_DEGREES, MAX_DEGREES);

	BOOL fullCircle = (degrees == 360.0f) ? TRUE : FALSE;

	// Get the basic dimension stuff
	float radians = (float)fabs(degrees) * DEG_TO_RAD;

	// Get the unit length for the angle per level of our surfrev operation
	float veclen = veccalc(radians / (float)levels);

	// If the shape can convert itself to a BezierShape, have it do so!
	BezierShape bShape;
	if(shape->CanMakeBezier())
		shape->MakeBezier(t, bShape);
	else {
		PolyShape pShape;
		shape->MakePolyShape(t, pShape);
		bShape = pShape;	// UGH -- Convert it from a PolyShape -- not good!
		}
	
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
		BOOL closed = spline->Closed();
		if(closed)
			anyClosed = TRUE;
		levelVerts += spline->KnotCount();
		levelTVerts += (spline->Segments() + 1);
		levelVecs += (spline->Segments() * 2);
		levelPatches += spline->Segments();
		}
	int vertLevels = levels + (fullCircle ? 0 : 1);
	nverts = levelVerts * vertLevels;
	npatches = levelPatches * levels;
	nvecs = levelVecs * vertLevels + levels * levelVerts * 2 + npatches * 4;
	if(texturing) {
		ntverts = levelTVerts * TVlevels;
		ntpatches = npatches;
		}
		
	pmesh.setNumVerts(nverts);
	pmesh.setNumVecs(nvecs);
	pmesh.setNumPatches(npatches);
	pmesh.setNumTVerts(ntverts);
	pmesh.setNumTVPatches(ntpatches);

	// Set up the rotational axis transform
	Matrix3 iaxis = Inverse(axis);

	// Set up a transform that will back out everything but our rotation
	// about the axis
	Matrix3 itm = Inverse(iaxis * RotateZMatrix(0.0f) * axis);

	// A flag to detect whether this will be an inside-out object
	BOOL inverse = FALSE;

	// Get information on the shapes

	ShapeDims *sDims = new ShapeDims [polys];
	for(poly = 0; poly < polys; ++poly) {
		ShapeDims &dim = sDims[poly];
		PolyLine line;
		Spline3D *spline = bShape.splines[poly];
		if(!spline->KnotCount())
			continue;
		spline->MakePolyLine(line);
		int lverts = line.numPts;
		for(int v = 0; v < lverts; ++v) {
			PolyPt &pp = line[v];
			Point3 test = pp.p * iaxis;
			if(v == 0 || test.x < dim.minx)
				dim.minx = test.x;
			if(v == 0 || test.x > dim.maxx)
				dim.maxx = test.x;
			}
		dim.xsize = dim.maxx - dim.minx;
		dim.coreThresh = dim.xsize / 100.0f;
		if(dim.coreThresh > 0.01f)
			dim.coreThresh = 0.01f;
		}

	// Create the vertices!
	int vert = 0;
	Matrix3 mat1, mat2;
	int level;
	for(poly = 0; poly < polys; ++poly) {
		ShapeDims &dim = sDims[poly];
		Spline3D *spline = bShape.splines[poly];
		if(!spline->KnotCount())
			continue;
		int knots = spline->KnotCount();
		for(level = 0; level < vertLevels; ++level) {
			int levelBase = vert;
			// Create a matrix for the operation
			Matrix3 tm = iaxis * RotateZMatrix((float)level / (float)levels * radians) * axis;
			if(level == 0)
				mat1 = tm;
			else
			if(level == (vertLevels - 1))
				mat2 = tm;
			for(knot = 0; knot < knots; ++knot) {
				Point3 p = spline->GetKnotPoint(knot) * tm;
				Point3 test = spline->GetKnotPoint(knot) * iaxis;
				pmesh.setVert(vert++, p);
				// If the first level, check for inversion!
				if(level == 1 && !inverse && (float)fabs(test.x) > dim.coreThresh) {
					Point3 ip = p * itm;
					if(p.z > 0.0f)
						inverse = TRUE;
					}
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
		for(level = 0; level < vertLevels; ++level) {
			// Create a matrix for the operation
			Matrix3 tm = iaxis * RotateZMatrix((float)level / (float)levels * radians) * axis;
			for(seg = 0; seg < segs; ++seg) {
				int seg2 = (seg + 1) % knots;
				if(spline->GetLineType(seg) == LTYPE_CURVE) {
					Point3 p = spline->GetOutVec(seg);
					pmesh.setVec(vec++, p * tm);
					p = spline->GetInVec(seg2);
					pmesh.setVec(vec++, p * tm);
					}
				else {
					Point3 p = spline->InterpBezier3D(seg, 0.333333f);
					pmesh.setVec(vec++, p * tm);
					p = spline->InterpBezier3D(seg, 0.666666f);
					pmesh.setVec(vec++, p * tm);
					}
				}
			}

		// Now, the vectors between the levels
		int baseVec = vec;
		for(level = 0; level < levels; ++level) {
			// Create a matrix for the operation
			Matrix3 rot = RotateZMatrix((float)level / (float)levels * radians);
			Matrix3 rot2 = RotateZMatrix((float)(level+1) / (float)levels * radians);
			for(knot = 0; knot < knots; ++knot) {
				Point3 p = spline->GetKnotPoint(knot);
				// Find the distance from the axis
				Point3 ip = p * iaxis;
				Point3 ipw(ip.x, ip.y, 0.0f);
				float radius = Length(ipw);
				// Get the normal for the vector that's perpendicular to the axis
				Point3 perp = Normalize(ipw ^ Point3(0,0,1));
				// Compute a vector length for this radius
				Point3 theVec = perp * veclen * radius;
				pmesh.setVec(vec++, (ip - theVec) * rot * axis);
				pmesh.setVec(vec++, (ip + theVec) * rot2 * axis);
				}
			}
		}

	// Flip 'inverse' if flipNormals is TRUE
	if(flipNormals)
		inverse = 1 - inverse;

	// Create the patches!
	int np = 0;
	int baseVert = 0;
	int baseVec = 0;
	for(poly = 0; poly < polys; ++poly) {
		Spline3D *spline = bShape.splines[poly];
		if(!spline->KnotCount())
			continue;
		int baseKnot = 0;
		int knots = spline->KnotCount();
		int pverts = knots * vertLevels;
		int segs = spline->Segments();
		int secVecs = segs * 2 * vertLevels;	// Number of vectors on cross-sections
		int segVecs = levels * knots * 2;	// Number of vectors between cross-sections
		int baseVec1 = 0;	// Base vector index for this level
		int baseVec2 = 0;	// Base vector index for between levels
		for(level = 0; level < levels; ++level) {
			int sm = 0;
			BOOL firstSmooth = (spline->GetLineType(0) == LTYPE_CURVE && spline->GetLineType(segs-1) == LTYPE_CURVE && (spline->GetKnotType(0) == KTYPE_AUTO || spline->GetKnotType(0) == KTYPE_BEZIER)) ? TRUE : FALSE;
			for(seg = 0; seg < segs; ++seg, vec += 4) {
				int prevseg = (seg + segs - 1) % segs;
				int seg2 = (seg + 1) % knots;
				int a,b,c,d,ab,ba,bc,cb,cd,dc,da,ad;
				MtlID mtl = useShapeIDs ? spline->GetMatID(seg) : 2;
				a = baseVert + baseKnot + seg;
				b = baseVert + baseKnot + seg2;
				c = baseVert + ((baseKnot + seg2 + knots) % pverts);
				d = baseVert + ((baseKnot + seg + knots) % pverts);
				ab = baseVec + baseVec1 + seg * 2;
				ba = ab + 1;
				bc = baseVec + secVecs + ((baseVec2 + seg2 * 2) % segVecs);
				cb = bc + 1;
				cd = baseVec + ((baseVec1 + seg * 2 + 1 + segs * 2) % secVecs);
				dc = cd - 1;
				da = baseVec + secVecs + ((baseVec2 + seg * 2 + 1) % segVecs);
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
				if(inverse)
					pmesh.MakeQuadPatch(np, a, ab, ba, b, bc, cb, c, cd, dc, d, da, ad, vec, vec+1, vec+2, vec+3, smooth ? smoothGroup : 0);
				else
					pmesh.MakeQuadPatch(np, a, ad, da, d, dc, cd, c, cb, bc, b, ba, ab, vec+3, vec+2, vec+1, vec, smooth ? smoothGroup : 0);
				pmesh.setPatchMtlIndex(np++, genMatIDs ? mtl : 0);
				}
			baseKnot += knots;
			baseVec1 += (segs * 2);
			baseVec2 += (knots * 2);
			}
		baseVert += (knots * vertLevels);
		baseVec += (secVecs + segVecs);
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
			int baseKnot = 0;
			int pknots = spline->Segments() + 1;
			int pverts = pknots * TVlevels;
			int segs = spline->Segments();
			for(level = 0; level < levels; ++level) {
				for(seg = 0; seg < segs; ++seg) {
					int prevseg = (seg + segs - 1) % segs;
					int seg2 = seg + 1;
					int a,b,c,d;
					a = baseTVert + baseKnot + seg;
					b = baseTVert + baseKnot + seg2;
					c = baseTVert + baseKnot + seg2 + pknots;
					d = baseTVert + baseKnot + seg + pknots;
					TVPatch &tp = pmesh.getTVPatch(ntp++);
					if(inverse)
						tp.setTVerts(a, b, c, d);
					else
						tp.setTVerts(a, d, c, b);
					}
				baseKnot += pknots;
				}
			baseTVert += (pknots * TVlevels);
			}
		assert(ntp == ntpatches);
		}

	// If capping, do it!
	if(degrees < 360.0f && anyClosed && (capStart || capEnd)) {
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
			int oldPatches = pmesh.numPatches;
			capper.CapPatchMesh(pmesh, capInfo, inverse, 16, &mat1, genMatIDs ? 1 : 0);
			// If texturing, create the texture patches and vertices
			if(texturing)
				MakePatchCapTexture(pmesh, Inverse(mat1), oldPatches, pmesh.numPatches);
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
			int oldPatches = pmesh.numPatches;
			capper.CapPatchMesh(pmesh, capInfo, 1-inverse, 16, &mat2, 0);
			// If texturing, create the texture patches and vertices
			if(texturing)
				MakePatchCapTexture(pmesh, Inverse(mat2), oldPatches, pmesh.numPatches);
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

//pmesh.Dump();

	pmesh.InvalidateGeomCache();
	}
#endif // NO_PATCHES

int SurfrevMod::NumSubObjTypes() 
{ 
	return 1;
}

ISubObjType *SurfrevMod::GetSubObjType(int i) 
{	
	static bool initialized = false;
	if(!initialized)
	{
		initialized = true;
		SOT_Axis.SetName(GetString(IDS_TH_AXIS));
	}

	switch(i)
	{
	case 0:
		return &SOT_Axis;
	}

	return NULL;
}
#endif // NO_MODIFIER_LATHE