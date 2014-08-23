/**********************************************************************
 *<
	FILE: selmod.cpp

	DESCRIPTION:  A volumetric selection modifier

	CREATED BY: Rolf Berteig

	HISTORY: 10/21/95


 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "mods.h"

#ifndef NO_MODIFIER_VOLUME_SELECT // JP Morel - July 25th 2002

//#include "iparamm.h"
#include "buildver.h"
#include "iparamm2.h"
#include "sctex.h"
#include "shape.h"
#include "simpobj.h"
#include "tvnode.h"
#include "iColorMan.h"
#include "MaxIcon.h"
#include "modstack.h"
#include "IParticleObjectExt.h"
#include "modsres.h"
#include "resourceOverride.h"


#define MAX_MATID	0xffff


#define REALLYBIGFLOAT	float( 1.0e+37F )

#define MCONTAINER_TVNODE_CLASS_ID Class_ID(0xe27e0f2b, 0x74fad492)
#define CONTAINER_TVNODE_CLASS_ID Class_ID(0xe27e0f2b, 0x74fad481)
#define SELNODE_TVNODE_CLASS_ID Class_ID(0xe27e0f2b, 0x74fad482)

static GenSubObjType SOT_Apparatus(14);
static GenSubObjType SOT_Center(15);
static GenSubObjType SOT_Vertex(1);
static GenSubObjType SOT_Polygon(4);

class SelModData : public LocalModData {
	public:
		int id;
		INode *selfNode;
		SelModData()
			{
			selfNode = NULL;
			}
		SelModData(int i)
			{
			id = i;
			selfNode = NULL;
			}
		~SelModData()
			{
			}	
		LocalModData*	Clone()
			{
			SelModData* d = new SelModData();
			d->id = -1;
			d->selfNode = NULL;

			return d;

			}	


	};

class SelNodeNotify;


class SelMod;

class SelModValidatorClass : public PBValidator
{
public:
class SelMod *mod;
private:
BOOL Validate(PB2Value &v) 
	{
	INode *node = (INode*) v.r;

	if (node->TestForLoop(FOREVER,(ReferenceMaker *) mod)!=REF_SUCCEED) return FALSE;

	return TRUE;

	};
};

// Version info added for MAXr4, where we can now be applied to a PatchMesh and retain identity!
#define SELMOD_VER1 1
#define SELMOD_VER4 4

#define SELMOD_CURRENT_VERSION SELMOD_VER4

class SelMod : public Modifier {	
	public:

		SelModValidatorClass validator;

//		IParamBlock *pblock;		
		IParamBlock2 *pblock2,*pblock2_afr;		
		Control *tmControl;
		Control *posControl;
		DWORD flags;		

		Box3 mcBox;
		INode *SelfNode;
		SelNodeNotify *notify;
		Matrix3 ntm;
		Matrix3 otm;
		TimeValue rt;
		int map,channel;
		int matID;
		int smG;
		BOOL autoFit;
	
		int useAR, level, method,vol,selType,invert;
		float pinch, falloff, bubble;
		Texmap *tmap;
		Box3 bbox;
		ObjectState sos;
		ShapeObject *pathOb;
		PolyShape workShape;

		SimpleParticle *pobj;
		IParticleObjectExt* epobj;

		Mesh *msh;
		SCTex shadeContext;
		INode *targnode;

		Tab<Box2D> boxList;
		Tab<Box3> box3DList;
		Tab<Point3> normList;

		Tab<Point3> uvwList;

		ITrackViewNode *container;

		int version;

		static IObjParam *ip; static SelMod* curMod;
//		static IParamMap *pmapParam, *pmapParam2;
		static MoveModBoxCMode *moveMode;
		static RotateModBoxCMode *rotMode;
		static UScaleModBoxCMode *uscaleMode;
		static NUScaleModBoxCMode *nuscaleMode;
		static SquashModBoxCMode *squashMode;		

		SelMod(BOOL create);
		~SelMod();
		void InitControl(ModContext &mc,Object *obj, TimeValue t);
		Matrix3 CompMatrix(TimeValue t,ModContext *mc, Matrix3 *ntm, BOOL scale=TRUE, BOOL offset=TRUE);
		void DoIcon(PolyLineProc& lp,BOOL sel);
		float PointInVolume(TimeValue t, Point3 pt,float u, float v, float w, Matrix3 &tm,Box3 box, int mindex);
		float DistFromVolume(Point3 pt, Matrix3 &tm, Matrix3 &ctm, Box3 box);
		void SelectVertices (Mesh &mesh, Matrix3 &tm, Matrix3 &ctm, Box3 &box, TimeValue t);
		void SelectVertices (MNMesh &mesh, Matrix3 &tm, Matrix3 &ctm, Box3 &box, TimeValue t);
		void SelectVertices (PatchMesh &mesh, Matrix3 &tm, Matrix3 &ctm, Box3 &box, TimeValue t);
		void SelectFaces (TimeValue t, Mesh &mesh, Matrix3 &tm,Box3 &box);
		void SelectFaces (TimeValue t, MNMesh &mesh, Matrix3 &tm,Box3 &box);
		void SelectPatches (TimeValue t, PatchMesh &mesh, Matrix3 &tm,Box3 &box);

		// From Animatable
		void DeleteThis() { delete this; }
		void GetClassName(TSTR& s) { s= GetString(IDS_RB_VOLSELECT_CLASS); }  
		virtual Class_ID ClassID() { return Class_ID(SELECTOSM_CLASS_ID,0);}
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip,ULONG flags,Animatable *next);
		TCHAR *GetObjectName() {return GetString(IDS_RB_VOLSELECT);}
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 
		int SubNumToRefNum(int subNum);
		BOOL AssignController(Animatable *control,int subAnim);

		ChannelMask ChannelsUsed()  {return OBJ_CHANNELS;}		
		ChannelMask ChannelsChanged() {return SELECT_CHANNEL|SUBSEL_TYPE_CHANNEL|GEOM_CHANNEL|TOPO_CHANNEL;}
		Class_ID InputType() {return defObjectClassID;}
		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		Interval LocalValidity(TimeValue t);
		Interval GetValidity(TimeValue t);

		int NumRefs() {return 4;}
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		int NumSubs() {return 4;}
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		
		// NS: New SubObjType API
		int NumSubObjTypes();
		ISubObjType *GetSubObjType(int i);
		//int GetSubObjectLevel() { return 1;}
//		TimeValue t;


// JBW: direct ParamBlock access is added
		int	NumParamBlocks() { return 2; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { if (i == 0) return pblock2; 
											else if (i == 1) return pblock2_afr; 
											else return NULL;
												} // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) {if (pblock2->ID() == id) return pblock2 ;
													 else if (pblock2_afr->ID() == id) return pblock2_afr ;
													else return  NULL; } // return id'd ParamBlock

		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);

		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		IOResult SaveLocalData(ISave *isave, LocalModData *pld);
		IOResult LoadLocalData(ILoad *iload, LocalModData **pld);


		int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flagst, ModContext *mc);
		void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);			

		void Move(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE);
		void Rotate(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin=FALSE);
		void Scale(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE);
		void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
		void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
		void ActivateSubobjSel(int level, XFormModes& modes);

		void EnableAffectRegion (TimeValue t);
		void DisableAffectRegion (TimeValue t);
		float GetPixel(TimeValue t, Point3 pt,float u, float v,float w);

		void RecurseDepth(float u1, float u2, float &fu,  ShapeObject *s,int Curve,int Piece, int &depth, Point3 fp);
		void PointToPiece(float &tempu,ShapeObject *s,int Curve,int Piece, int depth, Point3 fp);
		float SplineToPoint(Point3 p1, ShapeObject *s);
		float DistToFace(Point3 a, Point3 p1,Point3 p2,Point3 p3, int faceIndex);
		float LineToPoint(Point3 p1, Point3 l1, Point3 l2);
		void RecurseDepthB(float u1, float u2, float &fu,  BezierShape *s,int Curve,int Piece, int &depth, Point3 fp);
		void PointToPieceB(float &tempu,BezierShape *s,int Curve,int Piece, int depth, Point3 fp);

		INode* GetNodeFromModContext(SelModData *smd, int &which);

		BOOL badObject;

		UniformGrid grid;
		void BuildGrid();
		float worldRadius;

	};


static void DoBoxIcon(BOOL sel,float length, PolyLineProc& lp);
static void DoCylinderIcon(BOOL sel,float radius, float height, PolyLineProc& lp);
static void DoSphereIcon(BOOL sel,float radius, PolyLineProc& lp);

enum { vsel_params, vsel_afr };

enum { sel_level, sel_method, sel_type, sel_volume, sel_invert, sel_node, sel_texture, sel_map, sel_map_channel, sel_matid, sel_smGroup, sel_autofit };
// vsel_afr IDs
enum { sel_use_ar, sel_falloff, sel_pinch, sel_bubble };


class SelNodeNotify  : public TVNodeNotify 
{
public:
SelMod *s;
SelNodeNotify(SelMod *smod)
	{
	s = smod;
	}
RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message)
	{
	if(( (message == REFMSG_CHANGE) || 
		 (message == REFMSG_MOUSE_CYCLE_COMPLETED)
		 ) && (!s->badObject))
		{
		s->pblock2->SetValue(sel_method,0,s->method);
		return REF_STOP; 
		}
	return REF_SUCCEED ;
	}
};

//TVNodeNotify:: TVNodeNotify(SelMod *smod)



//--- ClassDescriptor and class vars ---------------------------------

//IParamMap*          SelMod::pmapParam   = NULL;
//IParamMap*          SelMod::pmapParam2   = NULL;
IObjParam*          SelMod::ip          = NULL;
SelMod*				SelMod::curMod      = NULL;
MoveModBoxCMode*    SelMod::moveMode    = NULL;
RotateModBoxCMode*  SelMod::rotMode     = NULL;
UScaleModBoxCMode*  SelMod::uscaleMode  = NULL;
NUScaleModBoxCMode* SelMod::nuscaleMode = NULL;
SquashModBoxCMode*  SelMod::squashMode  = NULL;


//class SelClassDesc:public ClassDesc {
class SelClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new SelMod(!loading);}
	const TCHAR *	ClassName() { return GetString(IDS_RB_VOLSELECT_CLASS); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(SELECTOSM_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFDEFORMATIONS);}
// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("VolumeSelect"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
	};

static SelClassDesc selDesc;
extern ClassDesc* GetSelModDesc() {return &selDesc;}

// JBW: Here follows the new parameter block descriptors.  There are now 3, 
//      two new STATIC ones to hold the old class var parameters, one for the main
//		per-instance parameters.  Apart from all the extra 
//      metadata you see in the definitions, one important new idea is the
//      folding of ParamMap description info into the parameter descriptor and
//      providing a semi-automatic rollout desipaly mechanism.
//      

// Parameter Block definitions

// JBW: First come the position and version independent IDs for each
//      of the blocks and the parameters in each block.  These IDs are used
//	    in subsequent Get/SetValue() parameter access, etc. and for version-independent
//      load and save

// vsel_params param IDs
#define PBLOCK_REF	0
#define TM_REF		1
#define POS_REF		2
#define PBLOCK_AFR_REF	3
// block IDs
/*
enum { vsel_params, vsel_afr };

enum { sel_level, sel_method, sel_type, sel_volume, sel_invert, sel_node, sel_texture, sel_map, sel_map_channel };
// vsel_afr IDs
enum { sel_use_ar, sel_falloff, sel_pinch, sel_bubble };
*/
// JBW: this descriptor defines the main per-instance parameter block.  It is flagged as AUTO_CONSTRUCT which
//      means that the CreateInstance() will automatically create one of these blocks and set it to the reference
//      number given (0 in this case, as seen at the end of the line).

// per instance geosphere block
static ParamBlockDesc2 sel_param_blk ( vsel_params, _T("Parameters"),  0, &selDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//rollout
	IDD_SELECTPARAM, IDS_RB_PARAMETERS, 0, 0, NULL,
	// params
	sel_level,  _T("level"), TYPE_INT, 	P_RESET_DEFAULT, 	IDS_PW_LEVEL, 
		p_default, 		0,	
		p_range, 		0, 2, 
		p_ui, 			TYPE_RADIO, 3,IDC_SEL_OBJECT,IDC_SEL_VERTEX,IDC_SEL_FACE,
		end, 
	sel_method,  _T("method"), TYPE_INT, 	P_RESET_DEFAULT, 	IDS_PW_METHOD, 
		p_default, 		0,	
		p_range, 		0, 2, 
		p_ui, 			TYPE_RADIO, 3,IDC_SEL_REPLACE,IDC_SEL_ADD,IDC_SEL_SUBTRACT,
		end, 
	sel_type,  _T("type"), TYPE_INT, 	P_RESET_DEFAULT, 	IDS_PW_TYPE, 
		p_default, 		0,	
		p_range, 		0, 1, 
		p_ui, 			TYPE_RADIO, 2,IDC_SEL_WINDOW,IDC_SEL_CROSSING,
		end, 


	sel_volume,  _T("volume"), TYPE_INT, 	P_RESET_DEFAULT, 	IDS_PW_VOLUME, 
		p_default, 		0,	
		p_range, 		0, 6, 
		p_ui, 			TYPE_RADIO,  7, IDC_SEL_BOXB, IDC_SEL_SPHEREB, IDC_SEL_CYLINDERB,
							IDC_SEL_MESH_OBJECTB, IDC_SEL_TEXTURE_MAPB, IDC_SEL_MATB, IDC_SEL_SMGROUPB,
		end, 

	sel_invert, 	_T("invert"),		TYPE_BOOL, 		P_RESET_DEFAULT,				IDS_PW_INVERT,
		p_default, 		FALSE, 
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_SEL_INVERT, 
		end, 

	sel_node, 	_T("node"),		TYPE_INODE, 		0,				IDS_PW_NODE,
		p_ui, 			TYPE_PICKNODEBUTTON, 	IDC_SEL_OBJECT_BUTTON, 
		end, 
	sel_texture, 	_T("texture"),		TYPE_TEXMAP, 		0,				IDS_PW_TEXMAP,
		p_ui, 			TYPE_TEXMAPBUTTON, 	IDC_SEL_TEXTURE_BUTTON, 
		end, 
	sel_map,  _T("map"), TYPE_INT, 	P_RESET_DEFAULT, 	IDS_PW_MAP, 
		p_default, 		0,	
		p_range, 		0, 1, 
		p_ui, 			TYPE_RADIO,  2,IDC_MAP_CHAN1,IDC_MAP_CHAN2,
		end, 

	sel_map_channel,  _T("mapChannel"),	TYPE_INT, 	P_RESET_DEFAULT, 	IDS_PW_CHANNEL, 
		p_default, 		1,	
		p_range, 		1, 99, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_INT, IDC_MAP_CHAN,IDC_MAP_CHAN_SPIN,  SPIN_AUTOSCALE,
		end, 

	sel_matid, _T("matID"), TYPE_INT, P_RESET_DEFAULT, IDS_VS_MATID,
		p_default, 1,
		p_range, 1, MAX_MATID,
		p_ui, TYPE_SPINNER, EDITTYPE_INT, IDC_VS_MATID, IDC_VS_MATIDSPIN, 1.0,
		end,

	sel_smGroup, _T("smGroup"), TYPE_INT, P_RESET_DEFAULT, IDS_VS_SMGROUP,
		p_default, 1,
		p_range, 1, 32,
		p_ui, TYPE_SPINNER, EDITTYPE_INT, IDC_VS_SMG, IDC_VS_SMGSPIN, 1.0,
		end,

	sel_autofit, 	_T("autofit"),		TYPE_BOOL, 		P_RESET_DEFAULT,				IDS_PW_AUTOFIT,
		p_default, 		TRUE, 
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_SEL_AUTOFIT, 
		end, 


	end
	);


static ParamBlockDesc2 sel_afr_blk ( vsel_afr, _T("AffectRegion"),  0, &selDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_AFR_REF, 

	//rollout
	IDD_SELMOD_AFFECTREGION, IDS_MS_SOFTSEL, 0, 0, NULL,
	// params
	sel_use_ar, 	_T("UseAffectRegion"),		TYPE_BOOL, 		P_RESET_DEFAULT,		IDS_PW_USE_AR,
		p_default, 		FALSE, 
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_MS_AR_USE, 
		p_enabled,		FALSE,
		p_enable_ctrls,	3, sel_falloff, sel_pinch, sel_bubble,
		end, 

	sel_falloff,  _T("falloff"),	TYPE_FLOAT, 	P_ANIMATABLE|P_RESET_DEFAULT, 	IDS_PW_FALLOFF2, 
		p_default, 		20.0f,	
		p_range, 		0.0f, 9999999999.0f, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FALLOFF,IDC_FALLOFFSPIN, SPIN_AUTOSCALE, 
		p_enabled,		FALSE,
		end, 

	sel_pinch,  _T("pinch"),	TYPE_FLOAT, 	P_ANIMATABLE|P_RESET_DEFAULT, 	IDS_PW_PINCH, 
		p_default, 		0.0f,	
		p_range, 		-10.0f, 10.0f, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_PINCH,IDC_PINCHSPIN, 0.01f, 
		p_enabled,		FALSE,
		end, 

	sel_bubble,  _T("bubble"),	TYPE_FLOAT, 	P_ANIMATABLE|P_RESET_DEFAULT, 	IDS_PW_BUBBLE, 
		p_default, 		0.0f,	
		p_range, 		-10.0f, 10.0f, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_BUBBLE,IDC_BUBBLESPIN, 0.01f, 
		p_enabled,		FALSE,
		end, 


	end
	);

//--- Parameter map/block descriptors -------------------------------

/*
#define PB_LEVEL 0
#define PB_METHOD 1
#define PB_TYPE 2
#define PB_VOLUME 3
#define PB_INVERT 4
#define PB_USE_AR 5
#define PB_FALLOFF 6
#define PB_PINCH 7
#define PB_BUBBLE 8
*/

// Levels
#define SEL_OBJECT		0
#define SEL_VERTEX		1
#define SEL_FACE		2

// Volumes
#define SEL_BOX			0
#define SEL_SPHERE		1
#define SEL_CYLINDER	2
#define SEL_MESH_OBJECT	3
#define SEL_TEXTURE		4
#define SEL_MATID 5
#define SEL_SMG 6

// Methods
#define SEL_REPLACE		0
#define SEL_ADD			1
#define SEL_SUBTRACT	2

// Types
#define SEL_WINDOW		0
#define SEL_CROSSING	1


// Flags
#define CONTROL_FIT		(1<<0)
#define CONTROL_CENTER	(1<<1)
#define CONTROL_UNIFORM	(1<<3)
#define CONTROL_HOLD	(1<<4)
#define CONTROL_INIT	(1<<5)
#define CONTROL_OP		(CONTROL_FIT|CONTROL_CENTER|CONTROL_UNIFORM|CONTROL_CHANGEFROMBOX)

#define CONTROL_USEBOX	(1<<6) // new for version 2. Means that gizmo space is not 0-1 but instead is based on the dimensions of the mod context box
#define CONTROL_CHANGEFROMBOX	(1<<7) // removes the above problem 


//
//
// Parameters

static int levelIDs[] = {IDC_SEL_OBJECT,IDC_SEL_VERTEX,IDC_SEL_FACE};
static int methodIDs[] = {IDC_SEL_REPLACE,IDC_SEL_ADD,IDC_SEL_SUBTRACT};
static int typeIDs[] = {IDC_SEL_WINDOW,IDC_SEL_CROSSING};
//static int volumeIDs[] = {IDC_SEL_BOX,IDC_SEL_SPHERE,IDC_SEL_CYLINDER};
/*
static ParamUIDesc descParam[] = {
	// Selection level
	ParamUIDesc(PB_LEVEL,TYPE_RADIO,levelIDs,3),
	
	// Selection method
	ParamUIDesc(PB_METHOD,TYPE_RADIO,methodIDs,3),
	
	// Selection type
	ParamUIDesc(PB_TYPE,TYPE_RADIO,typeIDs,2),

	// Selection volume
	ParamUIDesc(PB_VOLUME,TYPE_RADIO,volumeIDs,3),

	// Invert
	ParamUIDesc(PB_INVERT,TYPE_SINGLECHEKBOX,IDC_SEL_INVERT),
	};
#define PARAMDESC_LENGH 5

static ParamUIDesc descParam2[] = {
	// Use affect region?
	ParamUIDesc (PB_USE_AR, TYPE_SINGLECHEKBOX, IDC_MS_AR_USE),

	ParamUIDesc (PB_FALLOFF, EDITTYPE_POS_UNIVERSE,
		IDC_FALLOFF, IDC_FALLOFFSPIN,
		0.0f, 9999999999.f, SPIN_AUTOSCALE),

	ParamUIDesc (PB_PINCH, EDITTYPE_UNIVERSE,
		IDC_PINCH, IDC_PINCHSPIN,
		-10.0f, 10.0f, 0.01f),

	ParamUIDesc (PB_BUBBLE, EDITTYPE_UNIVERSE,
		IDC_BUBBLE, IDC_BUBBLESPIN,
		-10.0f, 10.0f, 0.01f),
};
#define PARAMDESC_LENGH2 4

static ParamBlockDescID descVer0[] = {
	{ TYPE_INT, NULL, FALSE, PB_LEVEL },
	{ TYPE_INT, NULL, FALSE, PB_METHOD },
	{ TYPE_INT, NULL, FALSE, PB_TYPE },
	{ TYPE_INT, NULL, FALSE, PB_VOLUME },
	{ TYPE_INT, NULL, FALSE, PB_INVERT }
};
static ParamBlockDescID descVer1[] = {
	{ TYPE_INT, NULL, FALSE, PB_LEVEL },
	{ TYPE_INT, NULL, FALSE, PB_METHOD },
	{ TYPE_INT, NULL, FALSE, PB_TYPE },
	{ TYPE_INT, NULL, FALSE, PB_VOLUME },
	{ TYPE_INT, NULL, FALSE, PB_INVERT },
	{ TYPE_INT, NULL, TRUE, PB_USE_AR },
	{ TYPE_FLOAT, NULL, TRUE, PB_FALLOFF },
	{ TYPE_FLOAT, NULL, TRUE, PB_PINCH },
	{ TYPE_FLOAT, NULL, TRUE, PB_BUBBLE },
};
*/
static ParamBlockDescID descVer0[] = {
	{ TYPE_INT, NULL, FALSE, sel_level },
	{ TYPE_INT, NULL, FALSE, sel_method },
	{ TYPE_INT, NULL, FALSE, sel_type },
	{ TYPE_INT, NULL, FALSE, sel_volume },
	{ TYPE_INT, NULL, FALSE, sel_invert }
};
static ParamBlockDescID descVer1[] = {
	{ TYPE_INT, NULL, FALSE, sel_level },
	{ TYPE_INT, NULL, FALSE, sel_method },
	{ TYPE_INT, NULL, FALSE, sel_type },
	{ TYPE_INT, NULL, FALSE, sel_volume },
	{ TYPE_INT, NULL, FALSE, sel_invert },
	{ TYPE_INT, NULL, TRUE, sel_use_ar },
	{ TYPE_FLOAT, NULL, TRUE, sel_falloff },
	{ TYPE_FLOAT, NULL, TRUE, sel_pinch },
	{ TYPE_FLOAT, NULL, TRUE, sel_bubble },
};

#define PBLOCK_LENGTH 9

static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,5,0)
};
#define NUM_OLDVERSIONS	1

#define CURRENT_VERSION 1
//static ParamVersionDesc curVersion(descVer1, PBLOCK_LENGTH, CURRENT_VERSION);

//--- SelDlgProc -----------------------------------
 
class SelDlgProc : public ParamMap2UserDlgProc {
public:
	SelMod *mod;		
	SelDlgProc(SelMod *m) {mod = m;}		
	BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
	void DeleteThis() {delete this;}		
};

BOOL SelDlgProc::DlgProc (TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {		
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				//case IDC_SEL_SPHERE:				
					//mod->flags |= CONTROL_UNIFORM|CONTROL_HOLD;
					//mod->NotifyDependents(FOREVER,SELECT_CHANNEL,REFMSG_CHANGE);
					//break;

				case IDC_SEL_FIT:
					mod->flags |= CONTROL_FIT|CONTROL_HOLD|CONTROL_INIT;
					mod->NotifyDependents(FOREVER,SELECT_CHANNEL,REFMSG_CHANGE);
					break;

				case IDC_SEL_CENTER:
					mod->flags |= CONTROL_CENTER|CONTROL_HOLD;
					mod->NotifyDependents(FOREVER,SELECT_CHANNEL,REFMSG_CHANGE);
					break;
				
				case IDC_SEL_RESET:
					theHold.Begin();
					mod->ReplaceReference(TM_REF,NULL);
					mod->flags |= CONTROL_FIT|CONTROL_CENTER|CONTROL_INIT;
					theHold.Accept(GetString(IDS_PW_UNDO_RESET));
					mod->NotifyDependents(FOREVER,SELECT_CHANNEL,REFMSG_CHANGE);
					break;
				case IDC_SEL_OBJECT:
				case IDC_SEL_FACE:
				case IDC_SEL_SMGROUPB:
				case IDC_SEL_MATB:
//watje 5-26-99
				case IDC_SEL_TEXTURE_MAPB:
					{
					mod->DisableAffectRegion(t);
					break;
					}


				//case IDC_SEL_REPLACE:
				//case IDC_SEL_ADD:
				//case IDC_SEL_SUBTRACT:
				case IDC_SEL_VERTEX:
				case IDC_SEL_BOXB:
				case IDC_SEL_SPHEREB:
				case IDC_SEL_CYLINDERB:
				case IDC_SEL_MESH_OBJECTB:

					{
					mod->EnableAffectRegion(t);
					break;
					}
		
				}
			break;
		}
	return FALSE;
	}

class AffectRegProc : public ParamMap2UserDlgProc {
public:
	SelMod *em;
	HWND hWnd;
	AffectRegProc () { em = NULL; }
	BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
	void DeleteThis() { }
	void Update(TimeValue t)
		{

		Rect rect;
		GetClientRectP(GetDlgItem(hWnd,IDC_AR_GRAPH),&rect);
		InvalidateRect(hWnd,&rect,FALSE);
		};

};

static AffectRegProc theAffectRegProc;

#define GRAPHSTEPS 20

float AffectRegFunctA(float dist,float falloff,float pinch,float bubble) {
	if (falloff<dist) return 0.0f;
	float u = ((falloff - dist)/falloff);
	float u2 = u*u, s = 1.0f-u;	
	return (3*u*bubble*s + 3*u2*(1.0f-pinch))*s + u*u2;
}

static void DrawCurve (HWND hWnd,HDC hdc) {
	float pinch, falloff, bubble;
	ISpinnerControl *spin = GetISpinner(GetDlgItem(hWnd,IDC_FALLOFFSPIN));
	falloff = spin->GetFVal();
	ReleaseISpinner(spin);	

	spin = GetISpinner(GetDlgItem(hWnd,IDC_PINCHSPIN));
	pinch = spin->GetFVal();
	ReleaseISpinner(spin);

	spin = GetISpinner(GetDlgItem(hWnd,IDC_BUBBLESPIN));
	bubble = spin->GetFVal();
	ReleaseISpinner(spin);	

	TSTR label = FormatUniverseValue(falloff);
	SetWindowText(GetDlgItem(hWnd,IDC_FARLEFTLABEL),label);
	SetWindowText(GetDlgItem(hWnd,IDC_FARRIGHTLABEL),label);

	Rect rect, orect;
	GetClientRectP(GetDlgItem(hWnd,IDC_AR_GRAPH),&rect);
	orect = rect;

	SelectObject(hdc,GetStockObject(NULL_PEN));
	SelectObject(hdc,GetStockObject(WHITE_BRUSH));
	Rectangle(hdc,rect.left,rect.top,rect.right,rect.bottom);	
	SelectObject(hdc,GetStockObject(NULL_BRUSH));
	
	rect.left   += 3;
	rect.right  -= 3;
	rect.top    += 20;
	rect.bottom -= 20;
	
	SelectObject(hdc,CreatePen(PS_DOT,0,GetCustSysColor(COLOR_BTNFACE)));
	MoveToEx(hdc,orect.left,rect.top,NULL);
	LineTo(hdc,orect.right,rect.top);
	MoveToEx(hdc,orect.left,rect.bottom,NULL);
	LineTo(hdc,orect.right,rect.bottom);
	MoveToEx(hdc,(rect.left+rect.right)/2,orect.top,NULL);
	LineTo(hdc,(rect.left+rect.right)/2,orect.bottom);
	DeleteObject(SelectObject(hdc,GetStockObject(BLACK_PEN)));
	
	MoveToEx(hdc,rect.left,rect.bottom,NULL);
	for (int i=0; i<=GRAPHSTEPS; i++) {
		float dist = falloff * float(abs(i-GRAPHSTEPS/2))/float(GRAPHSTEPS/2);		
		float y = AffectRegFunctA(dist,falloff,pinch,bubble);
		int ix = rect.left + int(float(rect.w()-1) * float(i)/float(GRAPHSTEPS));
		int	iy = rect.bottom - int(y*float(rect.h()-2)) - 1;
		if (iy<orect.top) iy = orect.top;
		if (iy>orect.bottom-1) iy = orect.bottom-1;
		LineTo(hdc, ix, iy);
	}
	
	WhiteRect3D(hdc,orect,TRUE);
}

BOOL AffectRegProc::DlgProc (TimeValue t, IParamMap2 *map,
										HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (!em) return FALSE;
	Rect rect;
	TSTR zero;

	switch (msg) {
	case WM_INITDIALOG:
		zero = FormatUniverseValue(0.0f);
		SetWindowText(GetDlgItem(hWnd,IDC_NEARLABEL),zero);
		ShowWindow(GetDlgItem(hWnd,IDC_AR_GRAPH),SW_HIDE);
		this->hWnd = hWnd;
		em->EnableAffectRegion (t);
		break;
		
	case WM_PAINT: {
//		em->EnableAffectRegion (t);
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd,&ps);
		DrawCurve(hWnd,hdc);
		EndPaint(hWnd,&ps);
		return FALSE;
		}

	case CC_SPINNER_CHANGE:
		GetClientRectP(GetDlgItem(hWnd,IDC_AR_GRAPH),&rect);
		InvalidateRect(hWnd,&rect,FALSE);
		return FALSE;
		break;

	default:
		return FALSE;
	}
	return TRUE;
}


//--- SelMod methods -------------------------------

SelMod::SelMod(BOOL create) {
	
	badObject = FALSE;
	validator.mod = this;

	if (create) flags = CONTROL_CENTER|CONTROL_FIT|CONTROL_INIT|CONTROL_USEBOX;
	else flags = 0;
/*
	MakeRefByID(FOREVER, PBLOCK_REF, 
		CreateParameterBlock(descVer1, PBLOCK_LENGTH, CURRENT_VERSION));
	pblock->SetValue (PB_FALLOFF, 0, 20.0f);
*/
	GetSelModDesc()->MakeAutoParamBlocks(this);

	tmControl  = NULL;
	posControl = NULL;
	container = NULL;
	notify = new SelNodeNotify(this);
	version = SELMOD_CURRENT_VERSION;
	pobj = NULL;
	epobj = NULL;
}


SelMod::~SelMod()
{
// mjm - begin - 5.10.99
	if (container && notify)
		{
		ITrackViewNode *tvr = GetCOREInterface()->GetTrackViewRootNode();
		ITrackViewNode *global = tvr->GetNode(Class_ID(0xb27e9f2a, 0x73fad370));
		ITrackViewNode *tvroot = global->GetNode(MCONTAINER_TVNODE_CLASS_ID);
		if (tvroot) 
			{
			int ct = tvroot->NumItems();
			for (int i = 0; i <ct; i++)
				{
				ITrackViewNode *n = tvroot->GetNode(i);
				if (container == n)
					container->UnRegisterTVNodeNotify(notify);
				}
			}
		else
			{
//			ITrackViewNode *tvroot = tvr->GetNode(CONTAINER_TVNODE_CLASS_ID);
			int ct = tvr->NumItems();
			for (int i = 0; i <ct; i++)
				{
				ITrackViewNode *n = tvr->GetNode(i);
				if (container == n)
					container->UnRegisterTVNodeNotify(notify);
				}

			}
		}

	if (notify)
		delete notify;
// mjm - end
}


#define USEBOX_CHUNK	0x0100
#define BOX_CHUNK		0x0110
#define BACKPATCH_CHUNK		0x0120
#define VERSION_CHUNK		0x0130


class SelModPostLoad : public PostLoadCallback {
	public:
		SelMod *n;
		BOOL param2;
		SelModPostLoad(SelMod *ns, BOOL p) {n = ns;param2 = p;}
		void proc(ILoad *iload) {  
			if (n->container != NULL)
				{
				n->container->RegisterTVNodeNotify(n->notify);
				n->container->HideChildren(TRUE);
				}
			if (!param2)
				n->pblock2->SetValue(sel_autofit,0,1);

			delete this; 


			} 
	};



IOResult SelMod::Load(ILoad *iload)
	{
	IOResult res;	
	Modifier::Load(iload);	
	ULONG nb;

	flags &= ~CONTROL_USEBOX;


	BOOL param2 = FALSE;

	// Default for older files
	version = SELMOD_VER1;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case USEBOX_CHUNK:
				flags |= CONTROL_USEBOX;
				flags |= CONTROL_CHANGEFROMBOX;
				flags |= CONTROL_INIT;
				break; 
			case BOX_CHUNK:
				iload->Read(&mcBox,sizeof(mcBox),&nb);
				break; 
			case BACKPATCH_CHUNK:
				ULONG id;
				iload->Read(&id,sizeof(ULONG), &nb);
				if (id!=0xffffffff)
					{
					iload->RecordBackpatch(id,(void**)&container);
					}
				param2 = TRUE;

				break;
			case VERSION_CHUNK:
				res = iload->Read(&version,sizeof(int),&nb);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}

	if (param2) flags &= ~CONTROL_CHANGEFROMBOX;

/*	iload->RegisterPostLoadCallback (
		new ParamBlockPLCB (versions, NUM_OLDVERSIONS, &curVersion, this, PBLOCK_REF));
*/

	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, NUM_OLDVERSIONS, &sel_param_blk, this, PBLOCK_REF);
	iload->RegisterPostLoadCallback(plcb);

	ParamBlock2PLCB* plcb2 = new ParamBlock2PLCB(versions, NUM_OLDVERSIONS, &sel_afr_blk, this, PBLOCK_AFR_REF);
	iload->RegisterPostLoadCallback(plcb2);

	iload->RegisterPostLoadCallback(new SelModPostLoad(this,param2));

	return IO_OK;

//	return IO_OK;
	}

IOResult SelMod::Save(ISave *isave)
	{
	Modifier::Save(isave);
	ULONG nb;

	isave->BeginChunk(BOX_CHUNK);
	isave->Write(&mcBox,sizeof(mcBox),&nb);

	isave->EndChunk();


	ULONG id = isave->GetRefID(container);

	isave->BeginChunk(BACKPATCH_CHUNK);
	isave->Write(&id,sizeof(ULONG),&nb);
	isave->EndChunk();
	isave->BeginChunk (VERSION_CHUNK);
	isave->Write (&version, sizeof(int), &nb);
	isave->EndChunk();

	if (flags&CONTROL_USEBOX) {
		isave->BeginChunk(USEBOX_CHUNK);
		isave->EndChunk();
		}


	return IO_OK;
	}

void SelMod::BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev) {
	this->ip = ip; curMod = this;

	// Add our sub object type
	// TSTR type1( GetString(IDS_RB_APPARATUS));
	// TSTR type2( GetString(IDS_RB_CENTER));
	// const TCHAR *ptype[] = {type1,type2};
	// This call is obsolete. Please see BaseObject::NumSubObjTypes() and BaseObject::GetSubObjType()
	// ip->RegisterSubObjectTypes(ptype, 2);

	// Create sub object editing modes.
	moveMode    = new MoveModBoxCMode(this,ip);
	rotMode     = new RotateModBoxCMode(this,ip);
	uscaleMode  = new UScaleModBoxCMode(this,ip);
	nuscaleMode = new NUScaleModBoxCMode(this,ip);
	squashMode  = new SquashModBoxCMode(this,ip);	
	selDesc.BeginEditParams(ip, this, flags, prev);
	sel_param_blk.SetUserDlgProc(new SelDlgProc(this));
	theAffectRegProc.em = this;
	sel_afr_blk.SetUserDlgProc(&theAffectRegProc);

	sel_param_blk.ParamOption(sel_node,p_validator,&validator);
/*	
	pmapParam = CreateCPParamMap (descParam,PARAMDESC_LENGH,
		pblock, ip, hInstance, MAKEINTRESOURCE(IDD_SELECTPARAM),
		GetString(IDS_RB_PARAMETERS), 0);		
	pmapParam->SetUserDlgProc(new SelDlgProc(this));
	pmapParam2 = CreateCPParamMap (descParam2, PARAMDESC_LENGH2,
		pblock, ip, hInstance, MAKEINTRESOURCE(IDD_MESHSEL_AFFECTREGION),
		GetString (IDS_MS_SOFTSEL), 0);
	theAffectRegProc.em = this;
	pmapParam2->SetUserDlgProc (&theAffectRegProc);
*/

	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);

	// Steve Anderson Aug 4 2003 - Why have these lines been here?  They're causing a bug
	// when the user moves the Volume Select up and down in the stack.  What could be their purpose?
	// They've been here since the start of the file history in 1999.  They weren't in Max 2.5, but were
	// here after the pblock2 conversion in Max 3.0.
	//int l;
	//pblock2->GetValue (sel_level,0,l,FOREVER);
	//pblock2->SetValue (sel_level,0,l);

	
	}
		
void SelMod::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
	{	
	this->ip = NULL; curMod = NULL;

	TimeValue t = ip->GetTime();
	ClearAFlag(A_MOD_BEING_EDITED);
 	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);

	ip->DeleteMode(moveMode);
	ip->DeleteMode(rotMode);
	ip->DeleteMode(uscaleMode);
	ip->DeleteMode(nuscaleMode);
	ip->DeleteMode(squashMode);	
	delete moveMode; moveMode = NULL;
	delete rotMode; rotMode = NULL;
	delete uscaleMode; uscaleMode = NULL;
	delete nuscaleMode; nuscaleMode = NULL;
	delete squashMode; squashMode = NULL;
	selDesc.EndEditParams(ip, this, flags, next);
/*
	DestroyCPParamMap(pmapParam);
	pmapParam = NULL;
	DestroyCPParamMap (pmapParam2);
	pmapParam2 = NULL;
*/
	theAffectRegProc.em = NULL;
	}


class sMyEnumProc : public DependentEnumProc 
	{
      public :
      virtual int proc(ReferenceMaker *rmaker); 
      INodeTab Nodes;              
	};

int sMyEnumProc::proc(ReferenceMaker *rmaker) 
	{ 
	if (rmaker->SuperClassID()==BASENODE_CLASS_ID)    
			{
            Nodes.Append(1, (INode **)&rmaker);                 
			}
     return 0;              
	}


Interval SelMod::LocalValidity(TimeValue t)
	{	
  // aszabo|feb.05.02 If we are being edited, return NEVER 
	// to forces a cache to be built after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;
	return GetValidity(t);
}

//aszabo|feb.06.02 - When LocalValidity is called by ModifyObject,
// it returns NEVER and thus the object channels are marked non valid
// As a result, the mod stack enters and infinite evaluation of the modifier
// ModifyObject now calls GetValidity and CORE calls LocalValidity to
// allow for building a cache on the input of this modifier when it's 
// being edited 
Interval SelMod::GetValidity(TimeValue t)
{
	Interval valid = FOREVER;
	if (tmControl) {
		Matrix3 tm(1);
		tmControl->GetValue(t,&tm,valid,CTRL_RELATIVE);		
		if (posControl) posControl->GetValue(t,&tm,valid,CTRL_RELATIVE);
	}
	int localUseAR=FALSE;
	pblock2->GetValue (sel_level, t, level, FOREVER);
	pblock2->GetValue (sel_method, t, method, FOREVER);
	if ((level==1) && (method==0)) {
		pblock2_afr->GetValue (sel_use_ar, t, useAR, valid);
		localUseAR = useAR;
	}
	if (localUseAR) {
		float f;
		pblock2_afr->GetValue (sel_falloff, t, f, valid);
		pblock2_afr->GetValue (sel_pinch, t, f, valid);
		pblock2_afr->GetValue (sel_bubble, t, f, valid);
	}

	pblock2->GetValue(sel_volume,t,vol,FOREVER);

	if (vol == SEL_TEXTURE) {
		Texmap *ttmap = NULL;
		pblock2->GetValue(sel_texture,t,ttmap,FOREVER);
		if (ttmap != NULL)
			valid &= ttmap->Validity(t);
	}
	if (vol == SEL_MESH_OBJECT) {
		INode *tnode;
		pblock2->GetValue(sel_node,t,tnode,FOREVER);
		if (tnode != NULL) {
			Matrix3 tm = tnode->GetObjectTM(t,&valid);
			ObjectState nos = tnode->EvalWorldState(t);
			if (nos.obj->IsShapeObject()) {
				ShapeObject *pathOb = (ShapeObject*)nos.obj;
				if (!pathOb->NumberOfCurves()) {
					pathOb = NULL;
					}
			
				}

			valid &= nos.obj->ObjectValidity(t);
			valid &= nos.Validity(t);
		    sMyEnumProc dep;              
			EnumDependents(&dep);
			for (int i = 0; i < dep.Nodes.Count(); i++)
				{
				dep.Nodes[i]->GetObjectTM(t,&valid);
				}
			if (nos.obj->IsParticleSystem() )
				valid.Set(t,t);

		}
//		valid.Set(t,t);  //<- THIS IS A HACK TO FORCE AN UPDATE SINCE THERE IS NO WAY THAT I KNOW OF TO GET THE BASE OBJECT VALIDIDTY
	}
	if (!(valid == FOREVER))
		valid.Set(t,t);  //<- THIS IS A HACK TO FORCE AN UPDATE SINCE THERE IS NO WAY THAT I KNOW OF TO GET THE BASE OBJECT VALIDIDTY

	return valid;
}

RefTargetHandle SelMod::Clone(RemapDir& remap) 
	{
	SelMod* newmod = new SelMod(FALSE);	
	newmod->ReplaceReference(PBLOCK_REF,pblock2->Clone(remap));
	newmod->ReplaceReference(PBLOCK_AFR_REF,pblock2_afr->Clone(remap));
	newmod->ReplaceReference(TM_REF,tmControl->Clone(remap));
	if (posControl)
		newmod->ReplaceReference(POS_REF,posControl->Clone(remap));
	newmod->flags = CONTROL_USEBOX;
	newmod->container = NULL;
//watje bug fix 5/24/00 196569
	newmod->mcBox=mcBox;
	newmod->version = version;

	BaseClone(this, newmod, remap);
	return newmod;
	}

static void FixupBox(Box3 &box)
	{
	if (box.IsEmpty()) box.MakeCube(Point3(0,0,0),10.0f);
	for (int i=0; i<3; i++) {
		if (fabs(box.pmax[i]-box.pmin[i])<0.001) {
			float cent = (box.pmax[i]-box.pmin[i])/2.0f;
			box.pmax[i] = cent + 0.0005f;
			box.pmin[i] = cent - 0.0005f;
			}
		}
	}




void SelMod::InitControl(ModContext &mc, Object *obj, TimeValue t)
	{
	Box3 box;
	Matrix3 tm;

	Box3 mcbox = *mc.box;
	FixupBox(mcbox);	

	if (tmControl==NULL) {
		MakeRefByID(FOREVER,TM_REF,NewDefaultMatrix3Controller()); 
		NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE);
		}
	if (posControl==NULL) {
		MakeRefByID(FOREVER,POS_REF,NewDefaultPositionController()); 
		NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE);
		}

	if (flags&(CONTROL_FIT|CONTROL_CENTER)) {
		Point3 zero(0,0,0);
		posControl->SetValue(t,&zero);
		}

	tm = Inverse(CompMatrix(t,&mc,NULL,FALSE));	

//	obj->GetDeformBBox(t,box,&tm,TRUE);
//	box = *(mc.box) * (*(mc.tm)) * tm;
	if (mc.box->IsEmpty()) 
		{
		box.MakeCube(Point3(0,0,0),10.0f);
		box = box * (*(mc.tm)) * tm;
		}
	else box = *(mc.box) * (*(mc.tm)) * tm;

	FixupBox(box);
	box.Scale(1.0000005f);	
	BOOL n3 = theHold.IsSuspended();

	BOOL isSuspended = FALSE;
	if (flags&CONTROL_HOLD) 
		{
		if (theHold.IsSuspended())
			{
			theHold.Resume();
			isSuspended = TRUE;
			}
		theHold.Begin();
		}

	if (flags&CONTROL_INIT) {
		SuspendAnimate();
		AnimateOff();
		}
	
	if (flags&CONTROL_UNIFORM) {
		Matrix3 tm(1), id(1);
		Interval valid;
		tmControl->GetValue(t,&tm,valid,CTRL_RELATIVE);
		float av = 0.0f;
		Point3 s;
		av += Length(tm.GetRow(0));
		av += Length(tm.GetRow(1));
		av += Length(tm.GetRow(2));
		av /= 3.0f;
		s.x = av/Length(tm.GetRow(0));
		s.y = av/Length(tm.GetRow(1));
		s.z = av/Length(tm.GetRow(2));

		SetXFormPacket pckt(s,TRUE,id,tm);
		tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
		}

	if (flags&CONTROL_FIT) {
		Point3 s, w  = box.Width();
		Matrix3 tm(1), id(1);
		Interval valid;
		tmControl->GetValue(t,&tm,valid,CTRL_RELATIVE);
				
		int axis;
		if (vol==SEL_BOX) {
			s.x = w.x==0.0f ? 1.0f : w.x/2.0f;
			s.y = w.y==0.0f ? 1.0f : w.y/2.0f;
			s.z = w.z==0.0f ? 1.0f : w.z/2.0f;
		} else 
		if (vol==SEL_SPHERE) {
			float max = w.x;
			axis = 0;
			if (w.y>max) 
				{
				max = w.y;
				axis = 1;
				}
			if (w.z>max) 
				{
				max = w.z;
				axis = 2;
				}
			if (max==0.0f) max = 1.0f;
			s.x = s.y = s.z = max/2.0f;
		} else {
			if (w.x>w.y) s.x = s.y = w.x/2.0f;
			else s.x = s.y = w.y/2.0f;
			s.z = w.z/2.0f;
			if (s.x==0.0f) s.x = 1.0f;
			if (s.y==0.0f) s.y = 1.0f;
			if (s.z==0.0f) s.z = 1.0f;
			}
		
		if (flags&CONTROL_USEBOX) {
			if (vol == SEL_CYLINDER)
				{
				if (w.x>w.y) 
					{
					s.x /= mcbox.Width().x*0.5f;
					s.y /= mcbox.Width().x*0.5f;
					}
				else{
					s.x /= mcbox.Width().y*0.5f;
					s.y /= mcbox.Width().y*0.5f;
					}
				s.z /= mcbox.Width().z*0.5f;

				}
			else if (vol == SEL_SPHERE)
				{
				if (axis == 0)
					{
					s.x /= mcbox.Width().x*0.5f;
					s.y /= mcbox.Width().x*0.5f;
					s.z /= mcbox.Width().x*0.5f;
					}
				else if (axis == 1)
					{
					s.x /= mcbox.Width().y*0.5f;
					s.y /= mcbox.Width().y*0.5f;
					s.z /= mcbox.Width().y*0.5f;
					}
				else 
					{
					s.x /= mcbox.Width().z*0.5f;
					s.y /= mcbox.Width().z*0.5f;
					s.z /= mcbox.Width().z*0.5f;
					}


				}

			else
				{
				s.x /= mcbox.Width().x*0.5f;
				s.y /= mcbox.Width().y*0.5f;
				s.z /= mcbox.Width().z*0.5f;
				}
			}

		SetXFormPacket pckt(s,TRUE,id,tm);
		tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
		
		// redo the box so our center op works
		tm = Inverse(CompMatrix(t,&mc,NULL,FALSE));	
		obj->GetDeformBBox(t,box,&tm,TRUE);
		FixupBox(box);
		}		

	if (flags&(CONTROL_CENTER|CONTROL_FIT)) {
		Matrix3 tm(1);
		Interval valid;
		tmControl->GetValue(t,&tm,valid,CTRL_RELATIVE);		
		if (!(flags&CONTROL_USEBOX)) {
			SetXFormPacket pckt(VectorTransform(tm,box.Center()));
			tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
		} else {			
			SetXFormPacket pckt(mcbox.Center()-(mcbox.Center()*tm));
			tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);

			//tm.SetTrans(box.Center()*tm);
			//tm.SetTrans((mc.box->Center()*Inverse(tm))-mc.box->Center());
			//SetXFormPacket pckt(tm);
			//tmControl->SetValue(t,&pckt,TRUE,CTRL_ABSOLUTE);
			}
		}
	
	if (flags&CONTROL_HOLD) 
		{
		if (flags&CONTROL_FIT)
			theHold.Accept(GetString(IDS_PW_UNDO_FIT));	
		else theHold.Accept(GetString(IDS_RB_CENTER));	
			

		if (isSuspended)
			{
			theHold.Suspend();
			}

		}

	if ( (flags&CONTROL_INIT) ||(flags&CONTROL_CHANGEFROMBOX) ) {
		mcBox = mcbox;
		flags &= ~CONTROL_CHANGEFROMBOX;
		ResumeAnimate();
		}

	// Turn off everything except the use box flag
	flags &= CONTROL_USEBOX;
	}


float SelMod::GetPixel(TimeValue t, Point3 pt,float u, float v, float w)
	{
	float f = 0.0f;
	if (tmap) {
		shadeContext.scrPos.x = int(u);
		shadeContext.scrPos.y = int(v);
		shadeContext.uvw.x = u;
		shadeContext.uvw.y = v;
		shadeContext.uvw.z = w;
		shadeContext.pt = pt;
		shadeContext.curTime = t;
		AColor c;
		c  = tmap->EvalColor(shadeContext);
		f += c.r * 0.299f;
		f += c.g * 0.587f;
		f += c.b * 0.114f;		
		}
	return f;
	}

#define EPSILON 0.000001f



void SelMod::RecurseDepth(float u1, float u2, float &fu,  ShapeObject *s,int Curve,int Piece, int &depth, Point3 fp)
{


for (int i = 0; i < depth; i++)
	{
	float u = (u1+u2)*.5f;
	float midu = (u2-u1)*.25f;
	float tu1 = u - midu; 
	float tu2 = u + midu;
	Point3 p1, p2;
	p1 = s->InterpPiece3D(rt, Curve, Piece, tu1);
	p2 = s->InterpPiece3D(rt, Curve, Piece, tu2);



	if ( LengthSquared(fp-p1) < LengthSquared(fp-p2) )
		{
		u1 = u1;
		u2 = u;
		}
	else
		{
		u1 = u;
		u2 = u2;
		}

	}
fu = (u2+u1)*0.5f;
}

void SelMod::PointToPiece(float &tempu,ShapeObject *s,int Curve,int Piece, int depth, Point3 fp)

{
//float tu1,tu2,tu3,tu4;
float tu;
float su,eu;
int depth1;

depth1 = depth;

su = 0.0f;
eu = 0.25f;

float fdist = REALLYBIGFLOAT;
float fu = 0.0f;

for (int i = 0; i < 4; i++)
	{
	tu = 0.0f;
	depth = depth1;
	RecurseDepth(su,eu,tu,s,Curve,Piece,depth,fp);
	su += 0.25f;
	eu += 0.25f;
	Point3 dp = s->InterpPiece3D(rt, Curve, Piece, tu);
	float dist = LengthSquared(fp-dp);
	if (dist<fdist)
		{
		fdist = dist;
		fu = tu;
		}
	}


tempu = fu;
//return fu;
}

void SelMod::RecurseDepthB(float u1, float u2, float &fu,  BezierShape *s,int Curve,int Piece, int &depth, Point3 fp)
{


for (int i = 0; i < depth; i++)
	{
	float u = (u1+u2)*.5f;
	float midu = (u2-u1)*.25f;
	float tu1 = u - midu; 
	float tu2 = u + midu;
	Point3 p1, p2;
//	p1 = s->InterpPiece3D(rt, Curve, Piece, tu1);
//	p2 = s->InterpPiece3D(rt, Curve, Piece, tu2);
	p1 = s->splines[Curve]->InterpBezier3D(Piece, tu1);
	p2 = s->splines[Curve]->InterpBezier3D(Piece, tu2);



	if ( LengthSquared(fp-p1) < LengthSquared(fp-p2) )
		{
		u1 = u1;
		u2 = u;
		}
	else
		{
		u1 = u;
		u2 = u2;
		}

	}
fu = (u2+u1)*0.5f;
}



void SelMod::PointToPieceB(float &tempu,BezierShape *s,int Curve,int Piece, int depth, Point3 fp)

{
//float tu1,tu2,tu3,tu4;
float tu;
float su,eu;
int depth1;

depth1 = depth;

su = 0.0f;
eu = 0.25f;

float fdist = REALLYBIGFLOAT;
float fu = 0.0f;

for (int i = 0; i < 4; i++)
	{
	tu = 0.0f;
	depth = depth1;
	RecurseDepthB(su,eu,tu,s,Curve,Piece,depth,fp);
	su += 0.25f;
	eu += 0.25f;
//	Point3 dp = s->InterpPiece3D(rt, Curve, Piece, tu);
	Point3 dp = s->splines[Curve]->InterpBezier3D(Piece, tu);
	float dist = LengthSquared(fp-dp);
	if (dist<fdist)
		{
		fdist = dist;
		fu = tu;
		}
	}


tempu = fu;
//return fu;
}



float SelMod::SplineToPoint(Point3 p1, ShapeObject *s)

{

int rec_depth = 5;

int piece_count = 0;
float fdist = REALLYBIGFLOAT;
int i = 0;




BezierShape bez;

if (!s->NumberOfCurves ()) return 0.0f;
if (s->CanMakeBezier()) 
	{
	s->MakeBezier (rt, bez);
	if (bez.splineCount < 1) return 0.0f;
	} 
else 
	{	// Do dumb conversion to bezier.
	PolyShape pshp;
	s->MakePolyShape (rt, pshp);
	if (pshp.numLines < 1) return 0.0f;
	bez = pshp;	// Basic conversion implemented in operator=.
	}


for (i = 0; i < bez.SplineCount(); i++)
	{
	for (int j = 0; j < bez.splines[i]->Segments(); j++)
		{
		float u;
		PointToPieceB(u,&bez,i,j,rec_depth,p1);
		Point3 dp = bez.splines[i]->InterpBezier3D(j,u);
		float dist = LengthSquared(p1-dp);
		if (dist<fdist)
			{
			fdist = dist;
			}
		}
	}

return (float)sqrt(fdist);
}

/*

float SelMod::SplineToPoint(Point3 p1, ShapeObject *s)

{

int rec_depth = 5;

int piece_count = 0;
float fdist = REALLYBIGFLOAT;
int i = 0;
for (i = 0; i < s->NumberOfCurves(); i++)
	{
	for (int j = 0; j < s->NumberOfPieces(rt,i); j++)
		{
		float u;
		PointToPiece(u,s,i,j,rec_depth,p1);
		Point3 dp = s->InterpPiece3D(rt, i, j, u);
		float dist = LengthSquared(p1-dp);
		if (dist<fdist)
			{
			fdist = dist;
			}
		}
	}

return (float)sqrt(fdist);
}
*/

float SelMod::LineToPoint(Point3 p1, Point3 l1, Point3 l2)
{
Point3 VectorA,VectorB,VectorC;
float Angle;
float dist = 0.0f;
VectorA = l2-l1;
VectorB = p1-l1;
Angle = (float) acos(DotProd(Normalize(VectorA),Normalize(VectorB)));
if (Angle > (3.14f/2.0f))
	{
	dist = Length(p1-l1);
	}
else
	{
	VectorA = l1-l2;
	VectorB = p1-l2;
	Angle = (float)acos(DotProd(Normalize(VectorA),Normalize(VectorB)));
	if (Angle > (3.14f/2.0f))
		{
		dist = Length(p1-l2);
		}
		else
		{
		float hyp;
		hyp = Length(VectorB);
		dist = (float) sin(Angle) * hyp;
//		u = (float) cos(Angle) * hyp;
//		float a = Length(VectorA);
//		u = ((a-u) / a);

		}

	}

return dist;

}


float SelMod::DistToFace(Point3 pa, Point3 p1,Point3 p2,Point3 p3, int faceIndex)
{
//if insed face take distance from plane
//check if intersects triangle
//compute normal

Ray ray;
Point3 n = normList[faceIndex];
ray.dir = -n;
ray.p = pa;
			
// See if the ray intersects the plane (backfaced)
float rn = DotProd(ray.dir,n);
		
// Use a point on the plane to find d
float d = DotProd(p1,n);

// Find the point on the ray that intersects the plane
float a = (d - DotProd(ray.p,n)) / rn;



// The point on the ray and in the plane.
Point3 hp = ray.p + a*ray.dir;

// Compute barycentric coords.
Point3 bry = msh->BaryCoords(faceIndex,hp);

// barycentric coordinates must sum to 1 and each component must
// be in the range 0-1
if ( (bry.x<0.0f || bry.x>1.0f || bry.y<0.0f || bry.y>1.0f || bry.z<0.0f || bry.z>1.0f) ||
     (fabs(bry.x + bry.y + bry.z - 1.0f) > EPSILON) )
	{
//else take distance from closest edge
//find 2 closest points and use that edge
	float closest,d;
	closest = LineToPoint(pa, p1, p2);
	d = LineToPoint(pa, p2, p3);
	if (d < closest) closest = d;
	d = LineToPoint(pa, p3, p1);
	if (d < closest) closest = d;
	return closest;

	}
else  
	{
	return Length(pa-hp);
	}

return 99999999.9f;
}

float SelMod::PointInVolume(TimeValue t, Point3 pt,float u, float v, float w, Matrix3 &tm,Box3 box, int pindex)
	{
	int i;
	float f;
	
	switch (vol) {
	case SEL_BOX:
		pt = pt*tm;
		for (i=0; i<3; i++) if ((pt[i]<-1.0f) || (pt[i]>1.0f)) break;
		if (i==3) return 1.0f;
		else return 0.0f;

	case SEL_SPHERE:
		pt = pt*tm;
		if (Length(pt)<=1.0f) return 1.0f;
		else return 0;

	case SEL_CYLINDER:
		pt = pt*tm;
		if (Sqrt(pt.x*pt.x+pt.y*pt.y)>1.0f) return 0.0f;
		if ((pt.z<-1.0f) || (pt.z>1.0f)) return 0.0f;
		return 1.0f;

	case SEL_MESH_OBJECT:
		{
//		INode *node=NULL;
//is node not null

//		pblock2->GetValue(sel_node,0,node,FOREVER);

		if (targnode != NULL)
			{

//transform point into world space then into the object space


			if (sos.obj->SuperClassID()==SHAPE_CLASS_ID)
				{

//is spline
				pt = pt*otm;

				if (sos.obj->SuperClassID()==SHAPE_CLASS_ID)
					{

					if (useAR)
						{
						rt = t;
						Box3 bboxFalloff = bbox;
						bboxFalloff.EnlargeBy(falloff);
						if (bboxFalloff.Contains(pt))
							{
							float d =  SplineToPoint(pt, pathOb);
							return d;
							}

						}
					return 999999999999.0f;

		
	
					}
				}
			else if (sos.obj->IsParticleSystem() )
				{

				pt = pt * ntm;
				float pdist;
				grid.ClosestPoint(pt,worldRadius,pindex,pdist);
				if (pindex == -1) pdist = REALLYBIGFLOAT;
				return pdist;

				}

			else if (sos.obj->SuperClassID()==GEOMOBJECT_CLASS_ID)
				
//is geom object
				{
					
				pt = pt*otm;
				if (bbox.Contains(pt)  )
					{

//					Setup a ray
///intersect axis  rays with mesh;
//this is a brute force ugly way to do this but will work for now

//do bounding box hit first
					Ray ray;
					Point3 norm;
					float at = 0.0f;

					float dist = 0.0f;

	
// See if we hit the object
					Point3 dir (1.0f,0.0f,0.0f);
					ray.p   = pt;
					ray.p.x -= 9999.9f;
					ray.dir = dir;
					BOOL hit = FALSE;
					int ct = 0;
					float l = 0;
					if (msh == NULL)
						{
						while  ((sos.obj->IntersectRay(rt,ray,at,norm)) && (l<9999.9f))
							{
							Point3 tp = ray.dir * (at+0.1f) ;
							ray.p = ray.p + tp;
							l += at;
							if (l< 9999.9f)
								ct++;
							}
						}
					else
						{
						if (boxList.Count() != 0)
							{
							Point3 dir (-1.0f,0.0f,0.0f);
							ray.p   = pt;
							ray.dir = dir;

							for (int k = 0; k < boxList.Count();  k++)
								{
								Point2 p2;
								p2.x = pt.y;
								p2.y = pt.z;
								Point2 min, max;
								min = boxList[k].min;
								max = boxList[k].max;
								if ( (p2.x <= boxList[k].max.x) && (p2.x >= boxList[k].min.x) &&
									 (p2.y <= boxList[k].max.y) && (p2.y >= boxList[k].min.y) )
									{
									Point3 plist[3];
									int index;
									for (int f = 0; f < 3; f++)
										{
										index = msh->faces[k].v[f];
										plist[f] = msh->verts[index];
										}
//check if all point are to the left
								
									if ( (plist[0].x <= pt.x) ||
										 (plist[1].x <= pt.x) ||
										 (plist[2].x <= pt.x)    )
										{
//check if intersects triangle
//compute normal

										Point3 n = normList[k];
			
		// See if the ray intersects the plane (backfaced)
										float rn = DotProd(ray.dir,n);
		
		// Use a point on the plane to find d
										float d = DotProd(msh->verts[index],n);

		// Find the point on the ray that intersects the plane
										float a = (d - DotProd(ray.p,n)) / rn;



		// The point on the ray and in the plane.
										Point3 hp = ray.p + a*ray.dir;

										int fudge = 0;
										if (plist[0].z == hp.z) fudge++;
										if (plist[1].z == hp.z) fudge++;
										if (plist[2].z == hp.z) fudge++;

										if (fudge > 1) hp.z += 0.001f;


		// Compute barycentric coords.
										Point3 bry = msh->BaryCoords(k,hp);

		// barycentric coordinates must sum to 1 and each component must
		// be in the range 0-1
										if (bry.x<0.0f || bry.x>1.0f || bry.y<0.0f || bry.y>1.0f || bry.z<0.0f || bry.z>1.0f)
											{
											}
										else if (fabs(bry.x + bry.y + bry.z - 1.0f) > EPSILON)
											{
											}
										else if (hp.x < pt.x)
											{
											ct++;
											}
										}
									}
								}
							}
	
						}
	
					l = 0;
					ray.p   = pt;
					dir.x = -1.0f;
					ray.dir = dir;

					if (msh == NULL)
						{
						if (sos.obj->ClassID()!=Class_ID(SPHERE_CLASS_ID,0))  // hack to make sphere work since it report intersections going both way
							{
							while  ((sos.obj->IntersectRay(rt,ray,at,norm)))
								{
								Point3 tp = ray.dir * (at+0.1f) ;
								ray.p = ray.p + tp;
								ct++;
								}
							}	
						}
					else
						{
						}
					if ((ct%2) == 1)
						return 1.0f;
					}
//				}
//				else 
					{
					if ((useAR) &&( msh != NULL))
						{
						Box3 bboxFalloff = bbox;
						bboxFalloff.EnlargeBy(falloff);
						if (bboxFalloff.Contains(pt))
							{
							float closest = -1.0f;
							for (int k = 0; k < boxList.Count();  k++)
								{
								if (box3DList[k].Contains(pt))
									{
//get distance from that face
									int a,b,c;
									a = msh->faces[k].v[0];
									b = msh->faces[k].v[1];
									c = msh->faces[k].v[2];
									float d =  DistToFace(pt, msh->verts[a], msh->verts[b], msh->verts[c],k);
									if ((closest == -1) || (d < closest))
										closest = d;
									}
								}
							if (closest != -1.0f) return closest;
							}
						}

					return 999999999.0f;
					}
				}
//				}
			}


			
		return 0.0f;
		}

	case SEL_TEXTURE:
		f = GetPixel(t,pt,u,v,w);
		if (f >0.99f) f = 1.0f;  // this has to be done since MS VC++ optimizer does not evaluate the same for debug and release
		return f;
	}

	return FALSE;
}

// Returns negative amount if point inside volume, 0 on surface, positive outside.
float SelMod::DistFromVolume(Point3 pt, Matrix3 &tm, Matrix3 & ctm, Box3 box)
	{
	Point3 p = tm*pt;
	int i;
	float max;
	Point3 diff, scale, center;
	for (i=0; i<3; i++) scale[i] = Length (ctm.GetRow (i));

	switch (vol) {
	case SEL_BOX:
		for (i=0; i<3; i++) {
			diff[i] = (float(fabs(p[i])) - 1.0f)*scale[i];
			if (!i || (diff[i]>max)) max = diff[i];
			if (diff[i]<0) diff[i] = 0.0f;
		}
		if (max<0) return max;	// this far inside box.
		return Length(diff);

	case SEL_SPHERE: // Not easy!  Distance to ellipsoid...
		return Length(((p)-Normalize(p))*(scale));	// Is this right though?

	case SEL_CYLINDER:
		if ((p.z < 0.0) || (p.z > 1.0)) 
			return Length(((p)-Normalize(p))*(scale));	// see note above
		else
			{
			p.z = 0;
			return Length(((p)-Normalize(p))*(scale));	// see note above
			}
/*
		max = diff.x = (Sqrt(p.x*p.x+p.y*p.y)-1.0f) * Sqrt(scale.x*scale.x+scale.y*scale.y);	// STEVE: kluge again.
		if ((diff.z = (float(fabs(p.z))-1.0f)*scale.z) > max) max = diff.z;
		if (max<0) return max;
		if (diff.x<=0) diff.x = 0.0f;
		if (diff.z<=0) diff.z = 0.0f; 
		diff.y = 0;
		return Length(diff);
*/
	}

	return 0.0f;
}

void
SelMod::BuildGrid()
{
	if (targnode && (vol == SEL_MESH_OBJECT) && sos.obj && sos.obj->IsParticleSystem() )
		{
		float scale = Length(ntm.GetRow(0));
		if (Length(ntm.GetRow(1)) > scale) scale = Length(ntm.GetRow(1));
		if (Length(ntm.GetRow(2)) > scale) scale = Length(ntm.GetRow(2));
		worldRadius = falloff*scale;
		Tab<Point3> pointList;
		if (pobj)
			{
			int count = pobj->parts.Count();
			float closest=999999999.9f;
			for (int pid = 0; pid < count; pid++)
				{
				TimeValue age  = pobj->ParticleAge(rt,pid);
				TimeValue life = pobj->ParticleLife(rt,pid);
				if (age!=-1)
					{
					Point3 curval = pobj->parts.points[pid];
					pointList.Append(1,&curval,1000);
					}
				}	
			grid.InitializeGrid(50);
			if (pointList.Count()> 0)
				grid.LoadPoints(pointList.Addr(0),pointList.Count());
			}
		else if (epobj)
			{
			int count = epobj->NumParticles();
			float closest=999999999.9f;
			for (int pid = 0; pid < count; pid++)
				{
				TimeValue age  = epobj->GetParticleAgeByIndex(pid);
				if (age!=-1)
					{
					Point3 *curval = epobj->GetParticlePositionByIndex(pid);

					if (curval)
						{
						pointList.Append(1,curval,1000);

						}
					}
				}	
			grid.InitializeGrid(50);
			if (pointList.Count()> 0)
				grid.LoadPoints(pointList.Addr(0),pointList.Count());					
			}

		}
}


void SelMod::SelectVertices (Mesh &mesh, Matrix3 &tm, Matrix3 & ctm, Box3 &box, TimeValue t) {
	if (method==SEL_REPLACE) {
		mesh.vertSel.ClearAll();
		mesh.ClearVSelectionWeights ();
	}

	if ((vol == SEL_MATID) || (vol == SEL_SMG)) {
		BitArray andVerts;
		BitArray orVerts;
		andVerts.SetSize (mesh.numVerts);
		andVerts.SetAll ();
		orVerts.SetSize (mesh.numVerts);
		orVerts.ClearAll ();
		DWORD realSmG = (1<<(smG-1));
		for (int i=0; i<mesh.numFaces; i++) {
			Face & fac = mesh.faces[i];
			BOOL hot = (vol == SEL_MATID) ? (fac.getMatID () == matID) : (fac.smGroup & realSmG);
			if (hot) {
				for (int j=0; j<3; j++) orVerts.Set (fac.v[j]);
			} else {
				for (int j=0; j<3; j++) andVerts.Clear (fac.v[j]);
			}
		}
		andVerts &= orVerts;	// So that verts with no faces won't be in andVerts.
		// If in crossing mode, select verts that touch this material.  Otherwise, select only verts
		// surrounded by this material.
		BitArray *selVerts = (selType==SEL_CROSSING) ? &orVerts : &andVerts;

		float *vsw = mesh.getVSelectionWeights ();	// NULL if none.
		switch (method) {
		case SEL_REPLACE:
		case SEL_ADD:
			mesh.vertSel |= *selVerts;
			if (vsw) {
				for (i=0; i<mesh.numVerts; i++) if ((*selVerts)[i]) vsw[i] = 1.0f;
			}
			break;
		case SEL_SUBTRACT:
			mesh.vertSel &= ~(*selVerts);
			if (vsw) {
				for (i=0; i<mesh.numVerts; i++) if ((*selVerts)[i]) vsw[i] = 0.0f;
			}
			break;
		}
		if (invert) {
			mesh.vertSel = ~mesh.vertSel;
			if (vsw) {
				for (i=0; i<mesh.numVerts; i++) vsw[i] = 1.0f-vsw[i];
			}
		}
		return;
	}
//watje 5-26-99
	if ((useAR) || (vol == SEL_TEXTURE)) mesh.SupportVSelectionWeights();
	float *vsw = mesh.getVSelectionWeights ();	// NULL if none.

	int currentChannel;
	currentChannel = map ? 0 : channel;

	BuildGrid();

	TVFace *tvFace = mesh.mapFaces(currentChannel);
	for (int i=0; i<mesh.getNumVerts(); i++) {
		//get texture verts for that vertex
		//loop through faces find ing matched
		Point3 tv(0.5f,0.5f,0.0f);
		if ((tvFace!= NULL) &&(i<uvwList.Count())) tv = uvwList[i];
		float f = PointInVolume(t, mesh.verts[i],tv.x,tv.y,tv.z,tm,box,i);
		if (f == 1.0f) {
			if (method==SEL_SUBTRACT) {
				mesh.vertSel.Clear(i);
				if (vsw) vsw[i] = 0.0f;
			} else {
				mesh.vertSel.Set(i);
				if (vsw) vsw[i] = 1.0f;
			}
		} else {
			if (vsw) {
				if ((vol != SEL_TEXTURE) && (vol != SEL_MESH_OBJECT)) {
					f = AffectRegFunctA (DistFromVolume (mesh.verts[i], tm, ctm, box), falloff, pinch, bubble);
				} else if (vol != SEL_TEXTURE) f = AffectRegFunctA (f, falloff, pinch, bubble);

				switch (method) {
				case SEL_SUBTRACT:
					vsw[i] = vsw[i] - f;
					if (vsw[i] < 0) vsw[i] = 0.0f;
					break;
				case SEL_ADD:
					vsw[i] += f;
					if (vsw[i] >= 1) vsw[i] = 1.0f;
					break;
				default:
					vsw[i] = f;
					break;
				}
	
				mesh.vertSel.Set (i, vsw[i] == 1.0f);
			}
		}

		if (invert) {
			if (vsw) {
				vsw[i] = 1.0f-vsw[i];
				mesh.vertSel.Set(i, vsw[i] == 1.0f);
			} else mesh.vertSel.Set(i,!mesh.vertSel[i]);
		}
	}
	grid.FreeGrid();
}

void SelMod::SelectVertices (MNMesh &mesh, Matrix3 &tm, Matrix3 & ctm, Box3 &box, TimeValue t) {
	if (method==SEL_REPLACE) {
		mesh.ClearVFlags (MN_SEL);
		mesh.freeVSelectionWeights ();
	}

	if ((vol == SEL_MATID) || (vol == SEL_SMG)) {
		BitArray andVerts;
		BitArray orVerts;
		andVerts.SetSize (mesh.numv);
		andVerts.SetAll ();
		orVerts.SetSize (mesh.numv);
		orVerts.ClearAll ();
		DWORD realSmG = (1<<(smG-1));
		for (int i=0; i<mesh.numf; i++) {
			MNFace & fac = mesh.f[i];
			BOOL hot = (vol == SEL_MATID) ? (fac.material == matID) : (fac.smGroup & realSmG);
			if (hot) {
				for (int j=0; j<fac.deg; j++) orVerts.Set (fac.vtx[j]);
			} else {
				for (int j=0; j<fac.deg; j++) andVerts.Clear (fac.vtx[j]);
			}
		}
		andVerts &= orVerts;	// So that verts with no faces won't be in andVerts.
		// If in crossing mode, select verts that touch this material.  Otherwise, select only verts
		// surrounded by this material.
		BitArray *selVerts = (selType==SEL_CROSSING) ? &orVerts : &andVerts;

		float *vsw = mesh.getVSelectionWeights ();	// NULL if none.
		switch (method) {
		case SEL_REPLACE:
		case SEL_ADD:
			for (i=0; i<mesh.numv; i++) {
				if (!(*selVerts)[i]) continue;
				mesh.v[i].SetFlag (MN_SEL);
				if (vsw) vsw[i] = 1.0f;
			}
			break;
		case SEL_SUBTRACT:
			for (i=0; i<mesh.numv; i++) {
				if (!(*selVerts)[i]) continue;
				mesh.v[i].ClearFlag (MN_SEL);
				if (vsw) vsw[i] = 0.0f;
			}
			break;
		}
		if (invert) {
			for (i=0; i<mesh.numv; i++) {
				mesh.v[i].SetFlag (MN_SEL, !mesh.v[i].GetFlag (MN_SEL));
				if (vsw) vsw[i] = 1.0f - vsw[i];
			}
		}
		return;
	}
//watje 5-26-99
	if ((useAR) || (vol == SEL_TEXTURE)) mesh.SupportVSelectionWeights();
	float *vsw = mesh.getVSelectionWeights ();	// NULL if none.

	int currentChannel;
	currentChannel = map ? 0 : channel;

	BuildGrid();

	bool texmap = false;
	if (mesh.M(currentChannel)) texmap = !mesh.M(currentChannel)->GetFlag (MN_DEAD);
	for (int i=0; i<mesh.numv; i++) {
		//get texture verts for that vertex
		//loop through faces find ing matched
		Point3 tv(0.5f,0.5f,0.0f);
		if (texmap &&(i<uvwList.Count())) tv = uvwList[i];
		float f = PointInVolume (t, mesh.v[i].p, tv.x, tv.y, tv.z, tm, box,i);
		if (f == 1.0f) {
			if (method==SEL_SUBTRACT) {
				mesh.v[i].ClearFlag (MN_SEL);
				if (vsw) vsw[i] = 0.0f;
			} else {
				mesh.v[i].SetFlag (MN_SEL);
				if (vsw) vsw[i] = 1.0f;
			}
		} else {
			if (vsw) {
				if ((vol != SEL_TEXTURE) && (vol != SEL_MESH_OBJECT)) {
					f = AffectRegFunctA (DistFromVolume (mesh.v[i].p, tm, ctm, box), falloff, pinch, bubble);
				} else if (vol != SEL_TEXTURE) f = AffectRegFunctA (f, falloff, pinch, bubble);

				switch (method) {
				case SEL_SUBTRACT:
					vsw[i] = vsw[i] - f;
					if (vsw[i] < 0) vsw[i] = 0.0f;
					break;
				case SEL_ADD:
					vsw[i] += f;
					if (vsw[i] >= 1) vsw[i] = 1.0f;
					break;
				default:
					vsw[i] = f;
					break;
				}
	
				mesh.v[i].SetFlag (MN_SEL, vsw[i] == 1.0f);
			}
		}

		if (invert) {
			if (vsw) {
				vsw[i] = 1.0f-vsw[i];
				mesh.v[i].SetFlag (MN_SEL, vsw[i] == 1.0f);
			} else mesh.v[i].SetFlag (MN_SEL, !mesh.v[i].GetFlag (MN_SEL));
		}
	}
	grid.FreeGrid();
}

void SelMod::SelectVertices (PatchMesh &pmesh, Matrix3 &tm, Matrix3 & ctm, Box3 &box, TimeValue t) {
	if (method==SEL_REPLACE) {
		pmesh.vertSel.ClearAll();
		pmesh.InvalidateVertexWeights();
	}

	if ((vol == SEL_MATID) || (vol == SEL_SMG)) {
		BitArray andVerts;
		BitArray orVerts;
		andVerts.SetSize (pmesh.numVerts);
		andVerts.SetAll ();
		orVerts.SetSize (pmesh.numVerts);
		orVerts.ClearAll ();
		DWORD realSmG = (1<<(smG-1));
		for (int i=0; i<pmesh.numPatches; i++) {
			Patch &p = pmesh.patches[i];
			BOOL hot = (vol == SEL_MATID) ? (p.getMatID() == matID) : (p.smGroup & realSmG);
			if (hot) {
				for (int j=0; j<p.type; j++) orVerts.Set (p.v[j]);
			} else {
				for (int j=0; j<p.type; j++) andVerts.Clear (p.v[j]);
			}
		}
		andVerts &= orVerts;	// So that verts with no faces won't be in andVerts.
		// If in crossing mode, select verts that touch this material.  Otherwise, select only verts
		// surrounded by this material.
		BitArray *selVerts = (selType==SEL_CROSSING) ? &orVerts : &andVerts;

		float *vsw = pmesh.GetVSelectionWeights ();	// NULL if none.
		switch (method) {
		case SEL_REPLACE:
		case SEL_ADD:
			pmesh.vertSel |= *selVerts;
			if (vsw) {
				for (i=0; i<pmesh.numVerts; i++)
					if ((*selVerts)[i])
						vsw[i] = 1.0f;
				}
			break;
		case SEL_SUBTRACT:
			pmesh.vertSel &= ~(*selVerts);
			if (vsw) {
				for (i=0; i<pmesh.numVerts; i++)
					if ((*selVerts)[i])
						vsw[i] = 0.0f;
				}
			break;
			}
		if (invert) {
			pmesh.vertSel = ~pmesh.vertSel;
			if (vsw) {
				for (i=0; i<pmesh.numVerts; i++)
					vsw[i] = 1.0f-vsw[i];
				}
			}
		return;
	}
//watje 5-26-99
	if ((useAR) || (vol == SEL_TEXTURE))
		pmesh.SupportVSelectionWeights();
	float *vsw = pmesh.GetVSelectionWeights ();	// NULL if none.

	int currentChannel;
	currentChannel = map ? 0 : channel;

	BuildGrid();

	TVPatch *tvPatch = pmesh.mapPatches(currentChannel);
	for (int i=0; i<pmesh.getNumVerts(); i++) {
		//get texture verts for that vertex
		//loop through faces finding matched
		Point3 tv(0.5f,0.5f,0.0f);
		if ((tvPatch!= NULL) &&(i<uvwList.Count()))
			tv = uvwList[i];
		float f = PointInVolume(t, pmesh.verts[i].p,tv.x,tv.y,tv.z,tm,box,i);
		if (f == 1.0f) {
			if (method==SEL_SUBTRACT) {
				pmesh.vertSel.Clear(i);
				if (vsw)
					vsw[i] = 0.0f;
			} else {
				pmesh.vertSel.Set(i);
				if (vsw)
					vsw[i] = 1.0f;
			}
		} else {
			if (vsw) {
				if ((vol != SEL_TEXTURE) && (vol != SEL_MESH_OBJECT))
					f = AffectRegFunctA (DistFromVolume (pmesh.verts[i].p, tm, ctm, box), falloff, pinch, bubble);
				else
				if (vol != SEL_TEXTURE)
					f = AffectRegFunctA (f, falloff, pinch, bubble);

				switch (method) {
				case SEL_SUBTRACT:
					vsw[i] = vsw[i] - f;
					if (vsw[i] < 0)
						vsw[i] = 0.0f;
					break;
				case SEL_ADD:
					vsw[i] += f;
					if (vsw[i] >= 1)
						vsw[i] = 1.0f;
					break;
				default:
					vsw[i] = f;
					break;
				}
	
				pmesh.vertSel.Set (i, vsw[i] == 1.0f);
			}
		}

		if (invert) {
			if (vsw) {
				vsw[i] = 1.0f-vsw[i];
				pmesh.vertSel.Set(i, vsw[i] == 1.0f);
			} else
				pmesh.vertSel.Set(i,!pmesh.vertSel[i]);
		}
	}
	grid.FreeGrid();
}

void SelMod::SelectFaces (TimeValue t, Mesh &mesh, Matrix3 &tm, Box3 &box) {
	if (method==SEL_REPLACE) mesh.faceSel.ClearAll();

	if ((vol == SEL_MATID) || (vol == SEL_SMG)) {
		DWORD realSmG = (1<<(smG-1));
		for (int i=0; i<mesh.getNumFaces(); i++) {
			if (vol==SEL_MATID){
				if (mesh.faces[i].getMatID () != matID) continue;
			} else {
				if (mesh.faces[i].getSmGroup () != realSmG) continue;
			}
			if (method == SEL_SUBTRACT) mesh.faceSel.Clear (i);
			else mesh.faceSel.Set (i);
		}
		if (invert) mesh.faceSel = ~mesh.faceSel;

		return;
	}
	int in;

//need to put in a channel spinner
//	TVFace *tvFace = mesh.mapFaces(0);
	int currentChannel;
	currentChannel = map ? 0 : channel;
	
	BuildGrid();

	Point3 *tVerts = mesh.mapVerts(currentChannel);
	TVFace *tvFace = mesh.mapFaces(currentChannel);
	for (int i=0; i<mesh.getNumFaces(); i++) {
		in = 0;
		for (int k=0; k<3; k++) {

			Point3 tv(0.0f,0.0f,0.0f);
			if ((tvFace != NULL) && (vol==SEL_TEXTURE))
				{
				int index = tvFace[i].t[k];
				tv = tVerts[index];
				}
			if (PointInVolume(t,mesh.verts[mesh.faces[i].v[k]],tv.x,tv.y,tv.z,tm,box,mesh.faces[i].v[k])==1.0f) {
//			if (1){
				in++;
				if (selType==SEL_CROSSING) break;
			} else {
				if (selType==SEL_WINDOW) goto nextFace;
				}
			}
		
		if (in) {
			if (method==SEL_SUBTRACT) {
				mesh.faceSel.Clear(i);
			} else {
				mesh.faceSel.Set(i);
				}
			}

		nextFace:;

		if (invert) mesh.faceSel.Set(i,!mesh.faceSel[i]);
		}
	grid.FreeGrid();
	}

void SelMod::SelectFaces (TimeValue t, MNMesh &mesh, Matrix3 &tm, Box3 &box) {
	if (method==SEL_REPLACE) mesh.ClearFFlags (MN_SEL);

	if ((vol == SEL_MATID) || (vol == SEL_SMG)) {
		DWORD realSmG = (1<<(smG-1));
		for (int i=0; i<mesh.numf; i++) {
			if (vol==SEL_MATID) {
				if (mesh.f[i].material != matID) continue;
			} else {
				if (mesh.f[i].smGroup != realSmG) continue;
			}
			if (method == SEL_SUBTRACT) mesh.f[i].ClearFlag (MN_SEL);
			else mesh.f[i].SetFlag (MN_SEL);
		}
		if (invert) {
			for (i=0; i<mesh.numf; i++) mesh.f[i].SetFlag (MN_SEL, !mesh.f[i].GetFlag (MN_SEL));
		}
		return;
	}
	int in;

	int currentChannel;
	currentChannel = map ? 0 : channel;

	BuildGrid();

	UVVert *tVerts = NULL;
	MNMapFace *tvFace = NULL;
	if (mesh.M(currentChannel) && !mesh.M(currentChannel)->GetFlag(MN_DEAD)) {
		tVerts = mesh.M(currentChannel)->v;
		tvFace = mesh.M(currentChannel)->f;
	}

	for (int i=0; i<mesh.numf; i++) {
		in = 0;
		bool outside = false;
		for (int k=0; k<mesh.f[i].deg; k++) {
			Point3 tv(0.0f,0.0f,0.0f);
			if ((tvFace != NULL) && (vol==SEL_TEXTURE)) {
				int index = tvFace[i].tv[k];
				tv = tVerts[index];
			}
			if (PointInVolume (t, mesh.v[mesh.f[i].vtx[k]].p,tv.x,tv.y,tv.z,tm,box,mesh.f[i].vtx[k])==1.0f) {
				in++;
				if (selType==SEL_CROSSING) break;
			} else {
				if (selType==SEL_WINDOW) {
					outside = true;
					break;
				}
			}
		}
		if (in && !outside) {
			if (method==SEL_SUBTRACT) mesh.f[i].ClearFlag (MN_SEL);
			else mesh.f[i].SetFlag (MN_SEL);
		}
		if (invert) mesh.f[i].SetFlag (MN_SEL, !mesh.f[i].GetFlag (MN_SEL));
	}
	grid.FreeGrid();
}

void SelMod::SelectPatches (TimeValue t, PatchMesh &pmesh, Matrix3 &tm, Box3 &box) {
	if (method==SEL_REPLACE)
		pmesh.patchSel.ClearAll();

	if ((vol == SEL_MATID) || (vol == SEL_SMG)) {
		DWORD realSmG = (1<<(smG-1));
		for (int i=0; i<pmesh.getNumPatches(); i++) {
			if (vol==SEL_MATID){
				if (pmesh.patches[i].getMatID () != matID) continue;
			} else {
				if (pmesh.patches[i].smGroup != realSmG) continue;
			}
			if (method == SEL_SUBTRACT)
				pmesh.patchSel.Clear (i);
			else
				pmesh.patchSel.Set (i);
		}
		if (invert)
			pmesh.patchSel = ~pmesh.patchSel;

		return;
	}
	int in;

	BuildGrid();

	int currentChannel;
	currentChannel = map ? 0 : channel;

	PatchTVert *tVerts = pmesh.mapVerts(currentChannel);
	TVPatch *tvPatch = pmesh.mapPatches(currentChannel);
	for (int i=0; i<pmesh.getNumPatches(); i++) {
		in = 0;
		Patch &p = pmesh.patches[i];
		for (int k=0; k<p.type; k++) {

			Point3 tv(0.0f,0.0f,0.0f);
			if ((tvPatch != NULL) && (vol==SEL_TEXTURE))
				{
				int index = tvPatch[i].tv[k];
				tv = tVerts[index].p;
				}
			if (PointInVolume(t,pmesh.verts[pmesh.patches[i].v[k]].p,tv.x,tv.y,tv.z,tm,box,pmesh.patches[i].v[k])==1.0f) {
				in++;
				if (selType==SEL_CROSSING) break;
			} else {
				if (selType==SEL_WINDOW) goto nextFace;
				}
			}
		
		if (in) {
			if (method==SEL_SUBTRACT) {
				pmesh.patchSel.Clear(i);
			} else {
				pmesh.patchSel.Set(i);
				}
			}

		nextFace:;

		if (invert)
			pmesh.patchSel.Set(i,!pmesh.patchSel[i]);
		}
	grid.FreeGrid();
	}


BOOL RecursePipeAndMatch(SelModData *smd, Object *obj)
	{
	SClass_ID		sc;
	IDerivedObject* dobj;
	Object *currentObject = obj;

	if ((sc = obj->SuperClassID()) == GEN_DERIVOB_CLASS_ID)
		{
		dobj = (IDerivedObject*)obj;
		while (sc == GEN_DERIVOB_CLASS_ID)
			{
			for (int j = 0; j < dobj->NumModifiers(); j++)
				{
				ModContext *mc = dobj->GetModContext(j);
				if (mc->localData == smd)
					{
					return TRUE;
					}

				}
			dobj = (IDerivedObject*)dobj->GetObjRef();
			currentObject = (Object*) dobj;
			sc = dobj->SuperClassID();
			}
		}

	int bct = currentObject->NumPipeBranches(FALSE);
	if (bct > 0)
		{
		for (int bi = 0; bi < bct; bi++)
			{
			Object* bobj = currentObject->GetPipeBranch(bi,FALSE);
			if (RecursePipeAndMatch(smd, bobj)) return TRUE;
			}

		}

	return FALSE;
}

INode* SelMod::GetNodeFromModContext(SelModData *smd, int &which)
	{

	int	i;

    sMyEnumProc dep;              
	EnumDependents(&dep);
	for ( i = 0; i < dep.Nodes.Count(); i++)
		{
		INode *node = dep.Nodes[i];
		BOOL found = FALSE;

		if (node)
			{
			Object* obj = node->GetObjectRef();
	
			if ( RecursePipeAndMatch(smd,obj) )
				{
				which = i;
				return node;
				}
			}
		}
	return NULL;
	}



void SelMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node)
	{

#ifndef NO_PATCHES
	if(version >= SELMOD_VER4 && os->obj->IsSubClassOf(patchObjectClassID)) {
		}
	else	// If it's a TriObject, process it
#endif
	if(os->obj->IsSubClassOf(triObjectClassID)) {
	}
	else if (os->obj->IsSubClassOf (polyObjectClassID)) {
	}
	else	// If it can convert to a TriObject, do it
	if(os->obj->CanConvertToType(triObjectClassID)) {
		}
	else
		{
		badObject = TRUE;
		}

	rt = t;
	pblock2->GetValue (sel_volume,t,vol,FOREVER);

	if (mc.localData == NULL)
		{
//add a new inode to trackview
		ITrackViewNode *tvr = GetCOREInterface()->GetTrackViewRootNode();
		ITrackViewNode *global = tvr->GetNode(Class_ID(0xb27e9f2a, 0x73fad370));
		ITrackViewNode *tvroot = global->GetNode(MCONTAINER_TVNODE_CLASS_ID);
		if (!tvroot) 
			{
			ITrackViewNode *mcontainer =  CreateITrackViewNode(TRUE);
			global->AddNode(mcontainer,GetString(IDS_PW_VOLDATA),MCONTAINER_TVNODE_CLASS_ID);
			tvroot = mcontainer;
			}
//add a new a container
		if (container == NULL)
			{
			container = CreateITrackViewNode(TRUE);
			container->HideChildren(TRUE);
			tvroot->AddNode(container,GetString(IDS_PW_VOLDATA),CONTAINER_TVNODE_CLASS_ID);
			}

		SelModData *d  = new SelModData(0);
		mc.localData = d;

		INode *localnode;
		int id;
		localnode = GetNodeFromModContext(d,id);
		d->id = id;

		
		if (localnode)
			{
			container->AddController(localnode->GetTMController(),localnode->GetName(),SELNODE_TVNODE_CLASS_ID);
			container->RegisterTVNodeNotify(notify);
			}
		else
			{
			d->id = -1;
			}

//create a back pointer to the container entry				
//return and call notify again to force another update
		NotifyDependents(FOREVER, OBJ_CHANNELS, REFMSG_CHANGE);

		Interval valid;
		valid.SetEmpty();
		os->obj->UpdateValidity(GEOM_CHAN_NUM,valid); // Have to do this to get it to evaluate
//		os->obj->UpdateValidity(TOPO_CHAN_NUM,valid); // Have to do this to get it to evaluate
		return;
		}
	else if (((SelModData *)mc.localData)->id == -1)
		{
		SelModData *d  = (SelModData *)mc.localData;


//add a new inode to trackview
		ITrackViewNode *tvr = GetCOREInterface()->GetTrackViewRootNode();
		ITrackViewNode *global = tvr->GetNode(Class_ID(0xb27e9f2a, 0x73fad370));
		ITrackViewNode *tvroot = global->GetNode(MCONTAINER_TVNODE_CLASS_ID);
		if (!tvroot) 
			{
			ITrackViewNode *mcontainer =  CreateITrackViewNode(TRUE);
			global->AddNode(mcontainer,GetString(IDS_PW_VOLDATA),MCONTAINER_TVNODE_CLASS_ID);
			tvroot = mcontainer;
			}
//add a new a container
		if (container == NULL)
			{
			container = CreateITrackViewNode(TRUE);
			container->HideChildren(TRUE);
			tvroot->AddNode(container,GetString(IDS_PW_VOLDATA),CONTAINER_TVNODE_CLASS_ID);
			}

		INode *localnode;
		int id;
		localnode = GetNodeFromModContext(d,id);

		if (localnode)
			{
			container->AddController(localnode->GetTMController(),localnode->GetName(),SELNODE_TVNODE_CLASS_ID);
			d->id = id;
			container->RegisterTVNodeNotify(notify);
			}
		else{
			d->id = -1;
			}
//create a back pointer to the container entry				
//return and call notify again to force another update
		NotifyDependents(FOREVER, OBJ_CHANNELS, REFMSG_CHANGE);

		Interval valid;
		valid.SetEmpty();
		os->obj->UpdateValidity(GEOM_CHAN_NUM,valid); // Have to do this to get it to evaluate
//		os->obj->UpdateValidity(TOPO_CHAN_NUM,valid); // Have to do this to get it to evaluate
		if (vol == SEL_MESH_OBJECT) return;

		}

/*
	if (ip != NULL && curMod == this)
		{
		ModContextList mcList;
		INodeTab nodes;
		if (mc.localData == NULL)
			{

			ip->GetModContexts(mcList,nodes);

//add a new inode to trackview
			ITrackViewNode *tvr = ip->GetTrackViewRootNode();
			ITrackViewNode *global = tvr->GetNode(Class_ID(0xb27e9f2a, 0x73fad370));
			ITrackViewNode *tvroot = global->GetNode(MCONTAINER_TVNODE_CLASS_ID);
			if (!tvroot) 
				{
				ITrackViewNode *mcontainer =  CreateITrackViewNode();
				global->AddNode(mcontainer,GetString(IDS_PW_VOLDATA),MCONTAINER_TVNODE_CLASS_ID);
				tvroot = mcontainer;
				}
//add a new a container
//			ITrackViewNode *container = CreateITrackViewNode();
			container = CreateITrackViewNode(TRUE);
			tvroot->AddNode(container,GetString(IDS_PW_VOLDATA),CONTAINER_TVNODE_CLASS_ID);
			for (int i = 0; i < nodes.Count(); i++)
				{
				container->AddController(nodes[i]->GetTMController(),nodes[i]->GetName(),SELNODE_TVNODE_CLASS_ID);
				SelModData *d  = new SelModData(i);
				mcList[i]->localData = d;
				}
			container->RegisterTVNodeNotify(notify);

//create a back pointer to the container entry				
//return and call notify again to force another update
			NotifyDependents(FOREVER, OBJ_CHANNELS, REFMSG_CHANGE);

			Interval valid;
			valid.SetEmpty();
			os->obj->UpdateValidity(GEOM_CHAN_NUM,valid); // Have to do this to get it to evaluate
			os->obj->UpdateValidity(TOPO_CHAN_NUM,valid); // Have to do this to get it to evaluate
			return;
			}
		else 
			{
			SelModData *d  = (SelModData *)mc.localData;

			if ( d->id == -1)
				{
				ip->GetModContexts(mcList,nodes);

//add a new inode to trackview
			ITrackViewNode *tvr = ip->GetTrackViewRootNode();
			ITrackViewNode *global = tvr->GetNode(Class_ID(0xb27e9f2a, 0x73fad370));
			ITrackViewNode *tvroot = global->GetNode(MCONTAINER_TVNODE_CLASS_ID);
			if (!tvroot) 
				{
				ITrackViewNode *mcontainer =  CreateITrackViewNode();
				global->AddNode(mcontainer,GetString(IDS_PW_VOLDATA),MCONTAINER_TVNODE_CLASS_ID);
				tvroot = mcontainer;
				}
//add a new a container
//			ITrackViewNode *container = CreateITrackViewNode();
				container = CreateITrackViewNode(TRUE);
				tvroot->AddNode(container,GetString(IDS_PW_VOLDATA),CONTAINER_TVNODE_CLASS_ID);
				for (int i = 0; i < nodes.Count(); i++)
					{
					container->AddController(nodes[i]->GetTMController(),nodes[i]->GetName(),SELNODE_TVNODE_CLASS_ID);
//					SelModData *d  = new SelModData(i);
//					mcList[i]->localData = d;
					d->id = i;
					}
				container->RegisterTVNodeNotify(notify);
//create a back pointer to the container entry				
//return and call notify again to force another update
				NotifyDependents(FOREVER, OBJ_CHANNELS, REFMSG_CHANGE);

				Interval valid;
				valid.SetEmpty();
				os->obj->UpdateValidity(GEOM_CHAN_NUM,valid); // Have to do this to get it to evaluate
				os->obj->UpdateValidity(TOPO_CHAN_NUM,valid); // Have to do this to get it to evaluate
//				ip->RedrawViews(t);
				if (vol == SEL_MESH_OBJECT) return;
				}
			}



		}
*/
	if ((ip && curMod == this) && (mc.localData == NULL))
		{
		NotifyDependents(FOREVER, OBJ_CHANNELS, REFMSG_CHANGE);
		Interval valid;
		valid.SetEmpty();
		os->obj->UpdateValidity(GEOM_CHAN_NUM,valid); // Have to do this to get it to evaluate
//		os->obj->UpdateValidity(TOPO_CHAN_NUM,valid); // Have to do this to get it to evaluate
		return;
		}


	if (container == NULL)
		{
		if (vol == SEL_MESH_OBJECT) 
			{
			SelModData *d  = (SelModData *) mc.localData;
			if ((d!= NULL) && (d->selfNode !=NULL))
				{
				Interface *ip = GetCOREInterface();
				ITrackViewNode *tvr = ip->GetTrackViewRootNode();
				ITrackViewNode *global = tvr->GetNode(Class_ID(0xb27e9f2a, 0x73fad370));
				ITrackViewNode *tvroot = global->GetNode(MCONTAINER_TVNODE_CLASS_ID);
				if (!tvroot) 
					{
					ITrackViewNode *mcontainer =  CreateITrackViewNode(TRUE);
					global->AddNode(mcontainer,GetString(IDS_PW_VOLDATA),MCONTAINER_TVNODE_CLASS_ID);
					tvroot = mcontainer;
					}
//add a new a container
				container = CreateITrackViewNode(TRUE);
				container->HideChildren(TRUE);
				tvroot->AddNode(container,GetString(IDS_PW_VOLDATA),CONTAINER_TVNODE_CLASS_ID);
				container->AddController(d->selfNode->GetTMController(),d->selfNode->GetName(),SELNODE_TVNODE_CLASS_ID);
				d->id = 0;
				container->RegisterTVNodeNotify(notify);
				}
			else
				return;
			}
		}

	Interval valid = GetValidity(t);
//build local data now
	pblock2->GetValue (sel_level,t,level,FOREVER);
	pblock2->GetValue (sel_method,t,method,FOREVER);
	pblock2->GetValue (sel_type,t,selType,FOREVER);
	pblock2->GetValue (sel_invert,t,invert,FOREVER);
	pblock2->GetValue (sel_map,t,map,FOREVER);
	pblock2->GetValue (sel_matid, t, matID, FOREVER);
	matID--;
	pblock2->GetValue (sel_smGroup, t, smG, FOREVER);
	pblock2->GetValue (sel_map_channel,t,channel,FOREVER);

	pblock2->GetValue (sel_autofit,t,autoFit,FOREVER);

	pblock2_afr->GetValue (sel_use_ar, t, useAR, FOREVER);
	pblock2_afr->GetValue (sel_falloff, t, falloff, FOREVER);
	pblock2_afr->GetValue (sel_pinch, t, pinch, FOREVER);
	pblock2_afr->GetValue (sel_bubble, t, bubble, FOREVER);

	targnode = NULL;
	tmap = NULL;

	TriObject *triObj = NULL;
	PatchObject *patchObj = NULL;
	PolyObject *polyObj = NULL;
	BOOL converted = FALSE;

	// For version 4 and later, we process patch & poly meshes as they are and pass them on.  Earlier
	// versions converted to TriMeshes (done below).  For adding other new types of objects, add
	// them here!
#ifndef NO_PATCHES
	if(version >= SELMOD_VER4 && os->obj->IsSubClassOf(patchObjectClassID)) {
		patchObj = (PatchObject *)os->obj;
		}
	else	// If it's a TriObject, process it
#endif
	if(os->obj->IsSubClassOf(triObjectClassID)) {
		triObj = (TriObject *)os->obj;
	}
	else if (os->obj->IsSubClassOf (polyObjectClassID)) {
		polyObj = (PolyObject *) os->obj;
	}
	else	// If it can convert to a TriObject, do it
	if(os->obj->CanConvertToType(triObjectClassID)) {
		triObj = (TriObject *)os->obj->ConvertToType(t, triObjectClassID);
		converted = TRUE;
		}
	else
		{
		badObject = TRUE;
		if (!tmControl || (flags&CONTROL_OP)) 
			InitControl (mc, os->obj, t);

		return;		// We can't deal with it!
		}

	Mesh &mesh = triObj ? triObj->GetMesh() : *((Mesh *)NULL);

#ifndef NO_PATCHES
	PatchMesh &pmesh = patchObj ? patchObj->GetPatchMesh(t) : *((PatchMesh *)NULL);
#else
	PatchMesh &pmesh = *((PatchMesh *)NULL);
#endif

	TriObject *collapsedtobj = NULL;

	//is texture selection
	if (vol == SEL_TEXTURE) {
		pblock2->GetValue(sel_texture,t,tmap,FOREVER);
		if (tmap) tmap->LoadMapFiles(t);
	    float blur = 0.0f;
		shadeContext.scale = 1.0f;
		shadeContext.duvw  = Point3(blur/10.0f+0.001f,blur/10.0f+0.001f,0.0f);
		shadeContext.dpt = Point3(blur/10.0f+0.001f,blur/10.0f+0.001f,blur/10.0f+0.001f);
		shadeContext.curTime = t;
		if (uvwList.Count() != os->obj->NumPoints())
			uvwList.SetCount(os->obj->NumPoints());
		int currentChannel;
		currentChannel = map ? 0 : channel;
	
		if(triObj) {

			if (uvwList.Count() != mesh.numVerts)
				uvwList.SetCount(mesh.numVerts);

			TVFace *tvFace = mesh.mapFaces(currentChannel);
			Point3 *tVerts = NULL;
			if (tvFace!= NULL) tVerts= mesh.mapVerts(currentChannel);
			Tab<int>ctList;
			ctList.SetCount(mesh.numVerts);

			for (int i=0; i<mesh.getNumVerts(); i++) 
				{
				Point3 tv(0.0f,0.0f,0.0f);
				ctList[i] = 0;
				uvwList[i] = tv;
				}
			if  (tvFace != NULL)
				{
				Point3 tv(0.0f,0.0f,0.0f);
				for (int j = 0; j < mesh.numFaces; j++)
					{
					for (int k = 0; k < 3; k++)
						{
						int index = mesh.faces[j].v[k];
						int tindex = tvFace[j].t[k];
						if (ctList[index] ==0)
							uvwList[index] = tVerts[tindex];
						ctList[index] += 1;
						}
					}
				}
			}
		else
		if (polyObj) {
			MNMesh & mm = polyObj->GetMesh();
			MNMapFace *tvFace = NULL;
			UVVert *tVerts = NULL;
			if (mm.M(currentChannel) && !mm.M(currentChannel)->GetFlag (MN_DEAD)) {
				tvFace = mm.M(currentChannel)->f;
				tVerts= mm.M(currentChannel)->v;
			}
			Tab<int>ctList;
			ctList.SetCount(os->obj->NumPoints());

			for (int i=0; i<mm.numv; i++) {
				Point3 tv(0.0f,0.0f,0.0f);
				ctList[i] = 0;
				uvwList[i] = tv;
			}
			if  (tvFace != NULL) {
				Point3 tv(0.0f,0.0f,0.0f);
				for (int j=0; j<mm.numf; j++) {
					for (int k=0; k<mm.f[j].deg; k++) {
						int index = mm.f[j].vtx[k];
						int tindex = tvFace[j].tv[k];
						if (ctList[index] ==0) uvwList[index] = tVerts[tindex];
						ctList[index] += 1;
					}
				}
			}
		}
		else
		if(patchObj) {
			TVPatch *tvPatch = pmesh.mapPatches(currentChannel);
			PatchTVert *tVerts = NULL;
			if (tvPatch!= NULL)
				tVerts = pmesh.mapVerts(currentChannel);
			Tab<int>ctList;
			ctList.SetCount(os->obj->NumPoints());

			for (int i=0; i<pmesh.getNumVerts(); i++) 
				{
				Point3 tv(0.0f,0.0f,0.0f);
				ctList[i] = 0;
				uvwList[i] = tv;
				}
			if  (tvPatch != NULL)
				{
				Point3 tv(0.0f,0.0f,0.0f);
				for (int j = 0; j < pmesh.numPatches; j++)
					{
					Patch &p = pmesh.patches[j];
					for (int k = 0; k < p.type; k++)
						{
						int index = pmesh.patches[j].v[k];
						int tindex = tvPatch[j].tv[k];
						if (ctList[index] ==0)
							uvwList[index] = tVerts[tindex].p;
						ctList[index] += 1;
						}
					}
				}
			}
		}
	else if ((vol == SEL_MESH_OBJECT) && (level != SEL_OBJECT))
		{
//is object
		pblock2->GetValue(sel_node,t,targnode,FOREVER);
		msh = NULL;
		if (targnode != NULL)
			{
			sos = targnode->EvalWorldState(t);
			sos.obj->GetDeformBBox(rt,bbox);

//is spline
			if (sos.obj->SuperClassID()==SHAPE_CLASS_ID)
				{
				pathOb = NULL;
				pathOb = (ShapeObject*)sos.obj;
				pathOb->MakePolyShape(t, workShape);
				}
			else if (sos.obj->IsParticleSystem())
				{
	//is particle
//				sos.obj->Eval(t);
				pobj = NULL;
				epobj = NULL;
				pobj = (SimpleParticle*) sos.obj->GetInterface(I_SIMPLEPARTICLEOBJ);
				if (pobj)
					pobj->UpdateParticles(t, targnode);
				else
					{
					epobj = (IParticleObjectExt*) sos.obj->GetInterface(PARTICLEOBJECTEXT_INTERFACE);
	
					if (epobj) 
						epobj->UpdateParticles(targnode, t);

					}
//				pobj = (SimpleParticle*)sos.obj;
//				pobj->UpdateParticles(t, targnode);
				}
			else
				{
				//is object

				if (sos.obj->IsSubClassOf(triObjectClassID))
					{
					TriObject *tobj = (TriObject*)sos.obj;
					msh = &tobj->GetMesh();
					}
//collapse it to a mesh
				else
					{
					if (sos.obj->CanConvertToType(triObjectClassID))
						{
//						TriObject *tobj;
						collapsedtobj = (TriObject*) sos.obj->ConvertToType(t,triObjectClassID);
						msh = &collapsedtobj->GetMesh();
						}
					}

				}
			}
		}



	if ((vol== SEL_MESH_OBJECT) && (level != SEL_OBJECT))
		{
//	    sMyEnumProc dep;              
//		EnumDependents(&dep);
//		SelfNode = dep.Nodes[0];

		int id;		
		SelfNode = GetNodeFromModContext((SelModData *) mc.localData,id);

		if (SelfNode)
			{
			Interval iv;
//check if local data present if so use it
			ntm = SelfNode->GetObjectTM(t,&iv);
//		Object *o = SelfNode->GetObjectRef();
//		INode *node;
//		pblock2->GetValue(sel_node,0,node,FOREVER);

//build box hit list
				//is object
			if (msh != NULL)
				{
	
				if (boxList.Count() != msh->numFaces)
					{
					boxList.SetCount(msh->numFaces);
					box3DList.SetCount(msh->numFaces);
					normList.SetCount(msh->numFaces);
					}
//			Point3 p3;
				for (int i = 0; i < msh->numFaces; i++)
					{
					boxList[i].SetEmpty();
					box3DList[i].Init();
					Point3 pnorm[3];
					for (int j = 0; j < 3; j++)
						{
						Point2 p;
						int index = msh->faces[i].v[j];
//						p3 = msh->verts[index];
						pnorm[j] = msh->verts[index];;
						p.x = msh->verts[index].y;
						p.y = msh->verts[index].z;
						boxList[i] += p;
						box3DList[i] += pnorm[j];
						}
					box3DList[i].EnlargeBy(falloff);
					normList[i] = Normalize(pnorm[1]-pnorm[0])^(pnorm[2]-pnorm[1]); 
					}
				}
			


			rt = t;

			if (mc.localData!= NULL)
				{
				SelModData *d  = (SelModData *) mc.localData;
//			Control *c = container->GetController(d->id);
//			ntm.IdentityMatrix();
//			c->GetValue(t,&ntm,valid,CTRL_RELATIVE);
				if (d->selfNode)
					ntm = d->selfNode->GetObjectTM(t,&iv);
				else
					{
					d->selfNode = SelfNode;
					ntm = d->selfNode->GetObjectTM(t,&iv);
					}
//			valid &= c->Validity(t);

				if (targnode != NULL)
					{
//				Matrix3 *tm = os->GetTM();
					otm = targnode->GetObjectTM(t,&iv);
					otm = Inverse(otm);
//				otm = ntm * otm;
					otm = ntm * otm;
					}
				}

			}

		}
	else
		{
		boxList.ZeroCount();
		box3DList.ZeroCount();
		normList.ZeroCount();
		}
//copied the mc box into our box so we cause problems later up in the stack.
	if ( (mc.box!=NULL)  && (flags&CONTROL_CHANGEFROMBOX) ) {
		mcBox.pmin = mc.box->pmin ;
		mcBox.pmax = mc.box->pmax ;
		flags &= ~CONTROL_CHANGEFROMBOX;
		}


	// Prepare the controller and set up mats
	if (!tmControl || (flags&CONTROL_OP)) InitControl (mc, os->obj, t);
	Matrix3 ctm, tm;
	ctm = CompMatrix(t,&mc,NULL,TRUE,FALSE);
	tm = Inverse(ctm);
	
//	Box3 mcbox = *mc.box;
//	FixupBox(mcbox);
//	Box3 mcbox = *mc.box;
	FixupBox(mcBox);

	if(triObj) {
		if (mesh.vertSel.GetSize() != mesh.getNumVerts()) 
			mesh.vertSel.SetSize(triObj->GetMesh().getNumVerts(),1);
		if (mesh.faceSel.GetSize() != mesh.getNumFaces()) 
			mesh.faceSel.SetSize(mesh.getNumFaces(),1);

			
		switch (level) {
			case SEL_OBJECT:
				mesh.selLevel = MESH_OBJECT;
				mesh.ClearDispFlag(DISP_VERTTICKS|DISP_SELVERTS|DISP_SELFACES);
				break;

			case SEL_VERTEX:
				mesh.selLevel = MESH_VERTEX;
				mesh.SetDispFlag(DISP_VERTTICKS|DISP_SELVERTS);
				SelectVertices(triObj->GetMesh(), tm, ctm, mcBox, t);
				break;

			case SEL_FACE:
				mesh.selLevel = MESH_FACE;
				mesh.SetDispFlag(DISP_SELFACES);
				SelectFaces(t, triObj->GetMesh(), tm, mcBox);
				break;
			}
		}
	else
	if (polyObj) {
		MNMesh & mm = polyObj->GetMesh();
		switch (level) {
		case SEL_OBJECT:
			mm.selLevel = MNM_SL_OBJECT;
			mm.ClearDispFlag (MNDISP_VERTTICKS|
				MNDISP_SELVERTS|MNDISP_SELFACES|MNDISP_SELEDGES);
			break;

		case SEL_VERTEX:
			mm.selLevel = MNM_SL_VERTEX;
			mm.SetDispFlag (MNDISP_VERTTICKS|MNDISP_SELVERTS);
			mm.ClearDispFlag (MNDISP_SELFACES|MNDISP_SELEDGES);
			SelectVertices (mm, tm, ctm, mcBox, t);
			break;

		case SEL_FACE:
			mm.selLevel = MNM_SL_FACE;
			mm.SetDispFlag(MNDISP_SELFACES);
			mm.ClearDispFlag (MNDISP_VERTTICKS|MNDISP_SELVERTS|MNDISP_SELEDGES);
			SelectFaces (t, mm, tm, mcBox);
			break;
		}
	}
	else
	if(patchObj) {
		if (pmesh.vertSel.GetSize() != pmesh.getNumVerts()) 
			pmesh.vertSel.SetSize(pmesh.getNumVerts(),1);
		if (pmesh.patchSel.GetSize() != pmesh.getNumPatches()) 
			pmesh.patchSel.SetSize(pmesh.getNumPatches(),1);

			
		switch (level) {
			case SEL_OBJECT:
				pmesh.selLevel = PATCH_OBJECT;
				pmesh.ClearDispFlag(DISP_VERTTICKS|DISP_SELVERTS|DISP_SELPATCHES);
				break;

			case SEL_VERTEX:
				pmesh.selLevel = PATCH_VERTEX;
				pmesh.SetDispFlag(DISP_VERTTICKS|DISP_SELVERTS);
				SelectVertices(pmesh, tm, ctm, mcBox, t);
				break;

			case SEL_FACE:
				pmesh.selLevel = PATCH_PATCH;
				pmesh.SetDispFlag(DISP_SELPATCHES);
				SelectPatches(t, pmesh, tm, mcBox);
				break;
			}
		}

	if (collapsedtobj) collapsedtobj->DeleteThis();

	if(!converted) {
		os->obj->UpdateValidity(SELECT_CHAN_NUM,valid);
		os->obj->UpdateValidity(GEOM_CHAN_NUM,valid); // Have to do this to get it to evaluate
//		os->obj->UpdateValidity(TOPO_CHAN_NUM,valid); // Have to do this to get it to evaluate
		os->obj->UpdateValidity(SUBSEL_TYPE_CHAN_NUM,FOREVER);
		}
	else {
		// Stuff converted object into the pipeline!
//		triObj->SetChannelValidity(TOPO_CHAN_NUM, valid);
		triObj->SetChannelValidity(GEOM_CHAN_NUM, valid);
		triObj->SetChannelValidity(TEXMAP_CHAN_NUM, valid);
		triObj->SetChannelValidity(MTL_CHAN_NUM, valid);
		triObj->SetChannelValidity(SELECT_CHAN_NUM, valid);
		triObj->SetChannelValidity(SUBSEL_TYPE_CHAN_NUM, valid);
		triObj->SetChannelValidity(DISP_ATTRIB_CHAN_NUM, valid);

		os->obj = triObj;
		}
	}


int SelMod::SubNumToRefNum(int subNum)
{
	return subNum;
}

BOOL SelMod::AssignController(Animatable *control,int subAnim)
	{
	ReplaceReference(subAnim,(Control*)control);
	NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE);
	NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	return TRUE;
	}

RefTargetHandle SelMod::GetReference(int i)
	{
	switch (i) {
		case PBLOCK_REF: return pblock2;
		case TM_REF: return tmControl;
		case POS_REF: return posControl;
		case PBLOCK_AFR_REF: return pblock2_afr;
		default: return NULL;
		}
	}

void SelMod::SetReference(int i, RefTargetHandle rtarg)
	{
	switch (i) {
		case PBLOCK_REF: pblock2 = (IParamBlock2*)rtarg; break;
		case TM_REF: tmControl = (Control*)rtarg; break;
		case POS_REF: posControl = (Control*)rtarg; break;
		case PBLOCK_AFR_REF: pblock2_afr = (IParamBlock2*)rtarg; break;
		}
	}

Animatable* SelMod::SubAnim(int i)
	{
	switch (i) {
		case PBLOCK_REF: return pblock2;
		case TM_REF: return tmControl;
		case POS_REF: return posControl;
		case PBLOCK_AFR_REF: return pblock2_afr;
		default: return NULL;
		}
	}

TSTR SelMod::SubAnimName(int i)
	{
	switch (i) {
		case PBLOCK_REF: return TSTR(GetString(IDS_RB_PARAMETERS));
		case TM_REF: return TSTR(GetString(IDS_RB_APPARATUS));
		case POS_REF: return TSTR(GetString(IDS_RB_CENTER));
		case PBLOCK_AFR_REF: return TSTR(GetString (IDS_MS_SOFTSEL));
		default: return TSTR(_T(""));
		}
	}

RefResult SelMod::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
   		PartID& partID, 
   		RefMessage message) 
   	{
	switch (message) {
		case REFMSG_CHANGE:
//			ivalid.SetEmpty();
			// see if this message came from a changing parameter in the pblock,
			// if so, limit rollout update to the changing item and update any active viewport texture
			if (hTarget == pblock2)
				{
				ParamID changing_param = pblock2->LastNotifyParamID();

				sel_param_blk.InvalidateUI(changing_param);
				
				// NS: 5/15/00 Update the StackView
				if(changing_param == sel_level)
				{
					// First pass on the REFMSG_CHANGED, so the modstack gets invalidated
					// and then pass on the REFMSG_NUM_SUBOBJECTTYPES_CHANGED, so that the 
					// StackView gets updated.

					NotifyDependents(changeInt, partID, message);
					NotifyDependents(FOREVER, 0, REFMSG_NUM_SUBOBJECTTYPES_CHANGED);
					return REF_STOP;
				}
				else if (changing_param == sel_texture)
					{

					int m;
//hack alert for some reason materials dep are not updating the stack this forces an update
					pblock2->GetValue (sel_method, GetCOREInterface()->GetTime(), m, FOREVER);
					pblock2->SetValue (sel_method, GetCOREInterface()->GetTime(), m);
					GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
					}
				// notify our dependents that we've changed
//				NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);

//				NotifyChanged();
				}
			if (hTarget == pblock2_afr) 
				{
				ParamID changing_param = pblock2_afr->LastNotifyParamID();

				sel_afr_blk.InvalidateUI(changing_param);
			// notify our dependents that we've changed
//				NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
				}

/*
			if (pmapParam && pmapParam->GetParamBlock()==pblock) {
				pmapParam->Invalidate();
			}
			if (pmapParam2 && pmapParam2->GetParamBlock()==pblock) {
				pmapParam2->Invalidate();
			}
*/
			break;
/*
		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			switch (gpd->index) {				
				case 0:
				default: gpd->dim = defaultDim; break;
				}			
			return REF_STOP; 
			}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			switch (gpn->index) {				
				case 0:
				default: gpn->name = TSTR(_T("")); break;
				}
			return REF_STOP; 
			}
*/
		}
	return REF_SUCCEED;
	}


// --- Gizmo transformations ------------------------------------------

void SelMod::Move(
		TimeValue t, Matrix3& partm, Matrix3& tmAxis, 
		Point3& val, BOOL localOrigin) 
	{	
#ifdef DESIGN_VER
	t=0;
#endif
	if (tmControl == NULL) return;
	if (ip && ip->GetSubObjectLevel()==1) {		
		SetXFormPacket pckt(val,partm,tmAxis);
		tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
	} else {
		Matrix3 ptm = partm;				
		tmControl->GetValue(t,&ptm,FOREVER,CTRL_RELATIVE);
		posControl->SetValue(t,
			VectorTransform(tmAxis*Inverse(ptm),val),TRUE,CTRL_RELATIVE);

		}
	}


void SelMod::Rotate(
		TimeValue t, Matrix3& partm, Matrix3& tmAxis, 
		Quat& val, BOOL localOrigin) 
	{
#ifdef DESIGN_VER
	t=0;
#endif	
	if (tmControl == NULL) return;
	SetXFormPacket pckt(val,localOrigin,partm,tmAxis);
	tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);		
	}

void SelMod::Scale(
		TimeValue t, Matrix3& partm, Matrix3& tmAxis, 
		Point3& val, BOOL localOrigin) 
	{
#ifdef DESIGN_VER
	t=0;
#endif	
	if (tmControl == NULL) return;
	SetXFormPacket pckt(val,localOrigin,partm,tmAxis);
	tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);		
	}


Matrix3 SelMod::CompMatrix(
		TimeValue t,ModContext *mc, Matrix3 *ntm, 
		BOOL scale, BOOL offset)
	{
	Matrix3 tm(1);
	Interval valid;
	if (autoFit)
		{
		Box3 mcbox;
		if (mc && mc->box) mcbox = *mc->box;
		FixupBox(mcbox);

		if (mc && scale && (flags&CONTROL_USEBOX)) {
			tm.Scale(mcbox.Width()/2.0f);
			tm.Translate(mcbox.Center());
			}
		}
	else
		{

//	Box3 mcbox;
//	if (mc && mc->box) mcbox = *mc->box;
		FixupBox(mcBox);

		if (mc && scale && (flags&CONTROL_USEBOX)) {
			tm.Scale(mcBox.Width()/2.0f);
			tm.Translate(mcBox.Center());
			}
		}

	
	if (posControl && offset) {
		Matrix3 tmc(1);
		posControl->GetValue(t,&tmc,valid,CTRL_RELATIVE);		
		tm = tm * tmc;
		}
	if (tmControl) {
		Matrix3 tmc(1);
		tmControl->GetValue(t,&tmc,valid,CTRL_RELATIVE);		
		tm = tm * tmc;
		}

	if (mc && mc->tm) {
		tm = tm * Inverse(*mc->tm);
		}
	if (ntm) {
		tm = tm * *ntm;
		}
	return tm;
	}


void SelMod::DoIcon(PolyLineProc& lp,BOOL sel) {
	switch (vol) {
	case SEL_BOX: DoBoxIcon(sel,2.0f,lp); break;
	case SEL_SPHERE: DoSphereIcon(sel,1.0f,lp); break;
	case SEL_CYLINDER: DoCylinderIcon(sel,1.0f,2.0f,lp); break;
	}
}

static Box3 unitBox(Point3(-0.5f,-0.5f,-0.5f),Point3(0.5f,0.5f,0.5f));
 
int SelMod::HitTest (TimeValue t, INode* inode, int type, int crossing, 
		int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc) 
	{	


	int savedLimits;	
	GraphicsWindow *gw = vpt->getGW();
	HitRegion hr;
	MakeHitRegion(hr,type, crossing,4,p);
	gw->setHitRegion(&hr);
	Matrix3 modmat, ntm = inode->GetObjectTM(t);
	DrawLineProc lp(gw);
		
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
	gw->clearHitCode();
	
	if (ip && ip->GetSubObjectLevel()==1) {
		modmat = CompMatrix(t,mc,&ntm,TRUE,FALSE);
		gw->setTransform(modmat);
		DoIcon(lp,FALSE);
		}

	if (ip && (
		ip->GetSubObjectLevel()==1 ||
		ip->GetSubObjectLevel()==2)) {
		modmat = CompMatrix(t,mc,&ntm);
		gw->setTransform(modmat);
		DrawCenterMark(lp,unitBox);		
		}
	
	gw->setRndLimits(savedLimits);	
	if (gw->checkHitCode()) {
		vpt->LogHit(inode, mc, gw->getHitDistance(), 0, NULL); 
		return 1;
		}
	return 0;
	}

int SelMod::Display(
		TimeValue t, INode* inode, ViewExp *vpt, int flags, 
		ModContext *mc) 
	{	
	GraphicsWindow *gw = vpt->getGW();
	// Transform the gizmo with the node.
#ifdef DESIGN_VER
	TimeValue rt = GetCOREInterface()->GetTime();
	Matrix3 modmat, ntm = inode->GetObjectTM(rt);
#else
	Matrix3 modmat, ntm = inode->GetObjectTM(t);
#endif
	DrawLineProc lp(gw);

	modmat = CompMatrix(t,mc,&ntm,TRUE,FALSE);	
	gw->setTransform(modmat);	
	DoIcon(lp, ip&&ip->GetSubObjectLevel()==1);

	modmat = CompMatrix(t,mc,&ntm);
	gw->setTransform(modmat);
	if (ip && (
		ip->GetSubObjectLevel()==1 ||
		ip->GetSubObjectLevel()==2)) {
		//gw->setColor(LINE_COLOR, (float)1.0, (float)1.0, (float)0.0);
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));
	} else {
		//gw->setColor( LINE_COLOR, (float).85, (float).5, (float)0.0);
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_GIZMOS));
		}
	DrawCenterMark(lp,unitBox);
	return 0;	
	}

void SelMod::GetWorldBoundBox(
		TimeValue t,INode* inode, ViewExp *vpt, Box3& box, 
		ModContext *mc) 
	{
	// Need the correct bound box for proper damage rect calcs.
#ifdef DESIGN_VER
	TimeValue rt = GetCOREInterface()->GetTime();
	Matrix3 modmat, ntm = inode->GetObjectTM(rt);
#else
	Matrix3 modmat, ntm = inode->GetObjectTM(t);	
#endif
	modmat = CompMatrix(t,mc,&ntm,TRUE,FALSE);		
	BoxLineProc bproc(&modmat);
	DoIcon(bproc,FALSE);

	modmat = CompMatrix(t,mc,&ntm);
	DrawCenterMark(bproc,unitBox);

	box = bproc.Box();	
	}

void SelMod::GetSubObjectCenters(
		SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc)
	{	
	Matrix3 modmat, ntm = node->GetObjectTM(t);		
	modmat = CompMatrix(t,mc,&ntm);
	cb->Center(modmat.GetTrans(),0);	
	}

void SelMod::GetSubObjectTMs(
		SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc)
	{
	Matrix3 ntm = node->GetObjectTM(t), modmat;
	modmat = CompMatrix(t,mc,&ntm);
	cb->TM(modmat,0);
	}

void SelMod::ActivateSubobjSel(int level, XFormModes& modes )
	{	
	switch (level) {
		case 1: // Modifier box
			modes = XFormModes(moveMode,rotMode,nuscaleMode,uscaleMode,squashMode,NULL);
			break;
		case 2: // Modifier Center
			modes = XFormModes(moveMode,NULL,NULL,NULL,NULL,NULL);
			break;
		}
	NotifyDependents(FOREVER,PART_DISPLAY,REFMSG_CHANGE);
	}


//----------------------------------------------------------------



#define NUM_SEGS	16

static void DoSphereIcon(BOOL sel,float radius, PolyLineProc& lp)
	{
	float u;
	Point3 pt[3];
	
	if (sel) //lp.SetLineColor(1.0f,1.0f,0.0f);
		lp.SetLineColor(GetUIColor(COLOR_SEL_GIZMOS));
	else //lp.SetLineColor(0.85f,0.5f,0.0f);		
		lp.SetLineColor(GetUIColor(COLOR_GIZMOS));
	
	// XY
	pt[0] = Point3(radius,0.0f,0.0f);
	for (int i=1; i<=NUM_SEGS; i++) {
		u = float(i)/float(NUM_SEGS) * TWOPI;
		pt[1].x = (float)cos(u) * radius;
		pt[1].y = (float)sin(u) * radius;
		pt[1].z = 0.0f;
		lp.proc(pt,2);
		pt[0] = pt[1];
		}

	// YZ	
	pt[0] = Point3(0.0f,radius,0.0f);
	for (i=1; i<=NUM_SEGS; i++) {
		u = float(i)/float(NUM_SEGS) * TWOPI;
		pt[1].y = (float)cos(u) * radius;
		pt[1].z = (float)sin(u) * radius;
		pt[1].x = 0.0f;
		lp.proc(pt,2);
		pt[0] = pt[1];
		}
	
	// ZX	
	pt[0] = Point3(0.0f,0.0f,radius);
	for (i=1; i<=NUM_SEGS; i++) {		
		u = float(i)/float(NUM_SEGS) * TWOPI;
		pt[1].z = (float)cos(u) * radius;
		pt[1].x = (float)sin(u) * radius;
		pt[1].y = 0.0f;
		lp.proc(pt,2);
		pt[0] = pt[1];
		}
	}

static void DoCylinderIcon(BOOL sel,float radius, float height, PolyLineProc& lp)
	{
	float u;
	Point3 pt[5], opt;
	
	if (sel) //lp.SetLineColor(1.0f,1.0f,0.0f);
		lp.SetLineColor(GetUIColor(COLOR_SEL_GIZMOS));
	else //lp.SetLineColor(0.85f,0.5f,0.0f);		
		lp.SetLineColor(GetUIColor(COLOR_GIZMOS));
	
	height *= 0.5f;
		
	opt = Point3(radius,0.0f,-height);
	for (int i=1; i<=NUM_SEGS; i++) {
		u = float(i)/float(NUM_SEGS) * TWOPI;
		pt[0]   = opt;
		
		pt[1].x = (float)cos(u) * radius;
		pt[1].y = (float)sin(u) * radius;
		pt[1].z = -height;
		
		pt[2].x = pt[1].x;
		pt[2].y = pt[1].y;
		pt[2].z = height;

		pt[3]   = opt;
		pt[3].z = height;
		
		lp.proc(pt,4);		
		opt = pt[1];
		}

	}


static int lStart[12] = {0,1,3,2,4,5,7,6,0,1,2,3};
static int lEnd[12]   = {1,3,2,0,5,7,6,4,4,5,6,7};

static void DoBoxIcon(BOOL sel,float length, PolyLineProc& lp)
	{
	Point3 pt[3];
	
	length *= 0.5f;
	Box3 box;
	box.pmin = Point3(-length,-length,-length);
	box.pmax = Point3( length, length, length);

	if (sel) //lp.SetLineColor(1.0f,1.0f,0.0f);
		lp.SetLineColor(GetUIColor(COLOR_SEL_GIZMOS));
	else //lp.SetLineColor(0.85f,0.5f,0.0f);		
		lp.SetLineColor(GetUIColor(COLOR_GIZMOS));
	
	for (int i=0; i<12; i++) {
		pt[0] = box[lStart[i]];
		pt[1] = box[lEnd[i]];
		lp.proc(pt,2);
		}
	}
/*
static int StraightEnables[] = {
	IDC_MS_AR_USE, IDC_FALLOFF_LABEL,
	IDC_PINCH_LABEL, IDC_BUBBLE_LABEL,
	IDC_FARLEFTLABEL, IDC_FARRIGHTLABEL,
	IDC_NEARLABEL
};

static int SpinnerEnables[] = { IDC_FALLOFFSPIN, IDC_PINCHSPIN, IDC_BUBBLESPIN };
*/
void SelMod::DisableAffectRegion (TimeValue t) 
{
//mesh, face/object/ subtract/add
//int selLevel, vol;
//pblock2->GetValue (sel_level, t, selLevel, FOREVER);
//pblock2->GetValue (sel_volume, t, vol, FOREVER);

//if ((vol == 3) || (selLevel == 0)|| (selLevel == 2))
//if ((level == 0)|| (level == 2))
	{
	IParamMap2 *afrmap = pblock2_afr->GetMap();
	afrmap->Enable(sel_use_ar, FALSE);
	afrmap->Enable(sel_falloff, FALSE);
	afrmap->Enable(sel_pinch, FALSE);
	afrmap->Enable(sel_bubble, FALSE);
	}
}

void SelMod::EnableAffectRegion (TimeValue t) 
{
//int selLevel, vol;
int sl;
pblock2->GetValue (sel_level, t, sl, FOREVER);
int v;
pblock2->GetValue (sel_volume, t, v, FOREVER);
BOOL use;

//if ((vol!=3) && (selLevel == 1))
//if ( (level == 1))
//watje 5-26-99
if ( (v < 4 ) && (sl == 1))
	{

	pblock2_afr->GetValue (sel_use_ar, t, use, FOREVER);


	IParamMap2 *afrmap = pblock2_afr->GetMap();
	afrmap->Enable(sel_use_ar, TRUE);
	if (useAR)
		{
		afrmap->Enable(sel_falloff, TRUE);
		afrmap->Enable(sel_pinch, TRUE);
		afrmap->Enable(sel_bubble, TRUE);
		}
	}

//HWND hWnd = pmap->GetHWnd();

//EnableWindow (GetDlgItem (hWnd, IDC_MS_AR_USE), FALSE);
//pmap->Enable(sel_use_ar, TRUE);
//pmap->Enable(sel_falloff, TRUE);
//pmap->Enable(sel_pinch, TRUE);
//pmap->Enable(sel_bubble, TRUE);


}
/*
void SelMod::EnableAffectRegion (TimeValue t) {
//	if (!pmapParam2) return;
//	HWND hWnd = pmapParam2->GetHWnd();

	IParamMap2* pmap = pblock2_afr->GetMap();
	HWND hWnd = pmap->GetHWnd();
	int selLevel, method;
	pblock2->GetValue (sel_level, t, selLevel, FOREVER);
	pblock2->GetValue (sel_method, t, method, FOREVER);
	BOOL enable = (selLevel==1) && (method==0);
	int vol;
	pblock2->GetValue (sel_volume, t, vol, FOREVER);
	if (vol == 3) enable = FALSE;
		
	EnableWindow (GetDlgItem (hWnd, StraightEnables[0]), enable);
	int useAR = FALSE;
	if (enable) pblock2_afr->GetValue (sel_use_ar, t, useAR, FOREVER);
	enable = useAR;

	int i;
	for (i=1; i<7; i++) EnableWindow (GetDlgItem (hWnd, StraightEnables[i]), enable);

//	ISpinnerControl *spin;
//	for (i=0; i<3; i++) {
//	pmap->Enable(sel_falloff, enable);
//	pmap->Enable(sel_pinch, enable);
//	pmap->Enable(sel_bubble, enable);
//		spin = GetISpinner (GetDlgItem (hWnd, SpinnerEnables[i]));
//		spin->Enable (enable);
//		ReleaseISpinner (spin);
//	}
}

*/
#define ID_CHUNK 0x1000
#define NODE_CHUNK 0x1010

IOResult SelMod::SaveLocalData(ISave *isave, LocalModData *pld)
{
SelModData *p;
IOResult	res;
ULONG		nb;

p = (SelModData*)pld;

isave->BeginChunk(ID_CHUNK);
res = isave->Write(&p->id, sizeof(int), &nb);
isave->EndChunk();


ULONG id = isave->GetRefID(p->selfNode);

isave->BeginChunk(NODE_CHUNK);
isave->Write(&id,sizeof(ULONG),&nb);
isave->EndChunk();


return IO_OK;
}

IOResult SelMod::LoadLocalData(ILoad *iload, LocalModData **pld)

{
	IOResult	res;
	ULONG		nb;

	int id;
	SelModData *p= new SelModData();
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case ID_CHUNK:
				iload->Read(&id,sizeof(int), &nb);
				p->id = id;
				break;
			case NODE_CHUNK:
				ULONG id;
				iload->Read(&id,sizeof(ULONG), &nb);
				if (id!=0xffffffff)
					{
					iload->RecordBackpatch(id,(void**)&p->selfNode);
					}
				break;

			}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
		}

	*pld = p;


return IO_OK;

}

int SelMod::NumSubObjTypes() 
{ 
	return 2;
}

ISubObjType *SelMod::GetSubObjType(int i) 
{	
	static bool initialized = false;
	if(!initialized)
	{
		initialized = true;
		SOT_Apparatus.SetName(GetString(IDS_RB_APPARATUS));
		SOT_Center.SetName(GetString(IDS_RB_CENTER));
		SOT_Vertex.SetName(GetString(IDS_RB_VERTEX));
		SOT_Polygon.SetName(GetString(IDS_EM_POLY));

	}

	switch(i)
	{
	case -1:
		{
			int l;
			pblock2->GetValue (sel_level,0,l,FOREVER);
			
			switch(l)
			{
			case 0: return NULL;
			case 1: return &SOT_Vertex;
			case 2:	return &SOT_Polygon;
			}
		}
	case 0:
		return &SOT_Apparatus;
	case 1:
		return &SOT_Center;
	}

	return NULL;
}

#endif // NO_MODIFIER_VOLUME_SELECT
