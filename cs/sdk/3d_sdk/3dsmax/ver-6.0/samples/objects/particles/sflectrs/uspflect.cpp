/**********************************************************************
 *<
	FILE: uspflect.cpp

	DESCRIPTION: Turns Any Mesh Into a SpawnFlector

	CREATED BY: Eric Peterson from Audrey's UDeflector code

	PB2 ECP 3/16/00

	HISTORY: 7/97

 **********************************************************************/
#include "sflectr.h"
#include "iparamm2.h"
#include "simpmod.h"
#include "simpobj.h"
#include "texutil.h"
#include "interpik.h"
#include "ICollision.h"

#define PBLK		0
#define CUSTNODE 	1

static Point3 Zero=Point3(0.0f,0.0f,0.0f);
static Class_ID USPAWNDEFL_CLASS_ID(0x19fd4916,0x557f71d9);
static Class_ID USPAWNDEFLMOD_CLASS_ID(0x36350a51,0x5073041f);

class UniPickOperand;

class USpawnDeflObject : public SimpleWSMObject2 {	
	public:		
//		static IParamMap *pmapParam;
		static IObjParam *ip;
		static HWND hSot;
		static HWND hParams;
					
		INode *custnode;
		int lastrnd;
		TimeValue t;
		TSTR custname;
		USpawnDeflObject();
		~USpawnDeflObject();
		BOOL SupportsDynamics() {return FALSE;}
		Mesh *dmesh;
		int nv,nf;
		VNormal *vnorms;
		Point3 *fnorms;
		Matrix3 tm,ptm,invtm,tmNoTrans,invtmNoTrans;
		Interval tmValid,mValid;
		Point3 dvel;

		static BOOL creating;
		static UniPickOperand pickCB;

		void ShowName();
		// From Animatable		
		void DeleteThis() {delete this;}		
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);		
		void MapKeys(TimeMap *map,DWORD flags);
		Class_ID ClassID() {return USPAWNDEFL_CLASS_ID;}		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_EP_USPAWNDEFLECTOR_OBJECT);}
				
		// from object		
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}
		
		// From SimpleWSMObject		
		void InvalidateUI();		
		void BuildMesh(TimeValue t);
		ParamDimension *GetParameterDim(int pbIndex);
//		TSTR GetParameterName(int pbIndex);		
		
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
		// From WSMObject
		Modifier *CreateWSMMod(INode *node);		
		int NumRefs() {return 2;}
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);		
		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message);

		// Direct paramblock access
		int	NumParamBlocks() { return 1; }
		IParamBlock2* GetParamBlock(int i) { return pblock2; }
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock2->ID() == id) ? pblock2 : NULL; }
		CollisionObject *GetCollisionObject(INode *node);
	};

class UniPickOperand : 
		public PickModeCallback,
		public PickNodeCallback {
	public:		
		USpawnDeflObject *po;
		
		UniPickOperand() {po=NULL;}

		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);

		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);

		BOOL Filter(INode *node);
		
		PickNodeCallback *GetFilter() {return this;}
	};

class CreateUSpawnDeflPickNode : public RestoreObj {
	public:   		
		USpawnDeflObject *obj;
		INode *oldn;
		CreateUSpawnDeflPickNode(USpawnDeflObject *o, INode *n) {
			obj = o; oldn=n;
			}
		void Restore(int isUndo) {
			if (obj->custnode) 
			{ obj->custname = TSTR(obj->custnode->GetName());
			}
			else 
			{ obj->custname=TSTR(_T(""));
			}
			obj->ShowName();
			}
		void Redo() 
		{ obj->custname = TSTR(oldn->GetName());
		if (obj->hParams)
	{TSTR name=TSTR(GetString(IDS_EP_OBJECTSTR)) + (oldn ? obj->custname : TSTR(GetString(IDS_EP_NONE)));
	SetWindowText(GetDlgItem(obj->hParams, IDC_EP_PICKNAME), name);
		}
			}
		TSTR Description() {return GetString(IDS_EP_USPPICK);}
	};

//--- ClassDescriptor and class vars ---------------------------------

IObjParam *USpawnDeflObject::ip			= NULL;
//IParamMap *USpawnDeflObject::pmapParam	= NULL;
HWND       USpawnDeflObject::hSot		= NULL;
HWND       USpawnDeflObject::hParams    = NULL;
BOOL USpawnDeflObject::creating			= FALSE;
UniPickOperand USpawnDeflObject::pickCB;

class USpawnDeflClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) { return new USpawnDeflObject;}
	const TCHAR *	ClassName() {return GetString(IDS_EP_USPAWNDEFLECTOR);}
	SClass_ID		SuperClassID() {return WSM_OBJECT_CLASS_ID; }
	Class_ID		ClassID() {return USPAWNDEFL_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_EP_SW_DEFLECTORS);}
	int 			BeginCreate(Interface *i);
	int 			EndCreate(Interface *i);

	// Hardwired name, used by MAX Script as unique identifier
	const TCHAR*	InternalName()				{ return _T("UOmniFlect"); }
	HINSTANCE		HInstance()					{ return hInstance; }
	};

static USpawnDeflClassDesc USpawnDeflDesc;
ClassDesc* GetUSpawnDeflObjDesc() {return &USpawnDeflDesc;}

//--- DeflectMod -----------------------------------------------------

class USpawnDeflField : public CollisionObject {
	public:		
		USpawnDeflField()
		{
//	 		colm = (CollisionMesh*)CreateInstance(REF_MAKER_CLASS_ID, MESH_COLLISION_ID);
			srand(453442);
			for (int i =0;i < 500; i++)
			{
				randomFloat[i] = (float)rand()/(float)RAND_MAX;
			}		
			colm=NULL;
		}

		~USpawnDeflField()
		{
			if (colm) colm->DeleteThis();
			colm=NULL;
		}
		void DeleteThis() 
			{	 
			if (colm) colm->DeleteThis();
			colm=NULL;
			delete this;

			}


		float randomFloat[500];
		CollisionMesh *colm;

		float chaos,bounce,bvar,vinher,friction;

		USpawnDeflObject *obj;
		INode *node;
		int badmesh;
		BOOL CheckCollision(TimeValue t,Point3 &pos, Point3 &vel, float dt, int index,float *ct,BOOL UpdatePastCollide);
		Object *GetSWObject();
		void SetRandSeed(int seed) { if (obj != NULL) obj->lastrnd = seed; }
	};

class USpawnDeflMod : public SimpleWSMMod {
	public:				
		USpawnDeflField deflect;

		USpawnDeflMod() {}
		USpawnDeflMod(INode *node,USpawnDeflObject *obj);	


		// From Animatable
		void GetClassName(TSTR& s) {s= GetString(IDS_EP_USPAWNDEFLECTORMOD);}
		SClass_ID SuperClassID() {return WSM_CLASS_ID;}
		void DeleteThis() {delete this;}
		Class_ID ClassID() { return USPAWNDEFLMOD_CLASS_ID;}
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_EP_USPAWNDEFLECTORBINDING);}

		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);

		// From SimpleWSMMod		
		Interval GetValidity(TimeValue t);		
		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);
	};

//--- ClassDescriptor and class vars ---------------------------------

class USpawnDeflModClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 0;}
	void *			Create(BOOL loading = FALSE) { return new USpawnDeflMod;}
	const TCHAR *	ClassName() {return GetString(IDS_EP_USPAWNDEFLECTORMOD);}
	SClass_ID		SuperClassID() {return WSM_CLASS_ID; }
	Class_ID		ClassID() {return USPAWNDEFLMOD_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
	};

static USpawnDeflModClassDesc USpawnDeflModDesc;
ClassDesc* GetUSpawnDeflModDesc() {return &USpawnDeflModDesc;}

enum 
{
	uspflectrobj_params, 
}; 

enum 
{
	uspflectrobj_timeon, 
	uspflectrobj_timeoff, 
	uspflectrobj_affects, 
	uspflectrobj_bounce, 
	uspflectrobj_bouncevar,
	uspflectrobj_chaos,
	uspflectrobj_friction,
	uspflectrobj_velocity,
	uspflectrobj_refracts,
	uspflectrobj_decel,
	uspflectrobj_decelvar,
	uspflectrobj_refraction,
	uspflectrobj_refractvar,
	uspflectrobj_diffusion,
	uspflectrobj_diffusionvar,
	uspflectrobj_radius,
	uspflectrobj_spawn,
	uspflectrobj_passvel,
	uspflectrobj_passvelvar,
	uspflectrobj_collider
};

#define PBLOCK_REF_NO	0

static ParamBlockDesc2 uspflector_param_blk 
(	uspflectrobj_params, 
	_T("USpawnflectorParameters"),  
	0, 
	&USpawnDeflDesc, 
	P_AUTO_CONSTRUCT + P_AUTO_UI, 
	PBLOCK_REF_NO, 

	//rollout
	IDD_AP_USPAWNDEFL, IDS_EP_PARAMETERS, 0, 0, NULL, 

	// params

	uspflectrobj_timeon,	_T("timeOn"),		TYPE_TIMEVALUE,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_TIMEON,
		p_default,			1,
		p_range,			-999999999, 999999999, 
		p_ui,				TYPE_SPINNER, EDITTYPE_INT,		IDC_EP_TIMEON,				IDC_EP_TIMEONSPIN,	0.1f,
		end,

	uspflectrobj_timeoff,	_T("timeOff"),		TYPE_TIMEVALUE,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_TIMEOFF,
		p_default,			1,
		p_range,			-999999999, 999999999, 
		p_ui,				TYPE_SPINNER, EDITTYPE_INT,		IDC_EP_TIMEOFF,				IDC_EP_TIMEOFFSPIN, 0.1f,
		end,

	uspflectrobj_affects,	_T("affects"),		TYPE_PCNT_FRAC,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_AFFECTS,
		p_default,			1.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_AFFECTS,				IDC_EP_AFFECTSSPIN, 1.0f,
		end,

	uspflectrobj_bounce,	_T("bounce"),		TYPE_FLOAT,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_BOUNCE,
		p_default,			1.0f,
		p_range,			0.0f,65535.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_BOUNCEU,				IDC_EP_BOUNCEUSPIN,			0.01f,
		end,

	uspflectrobj_bouncevar, _T("bounceVar"),	TYPE_PCNT_FRAC,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_BOUNCEVAR,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_BOUNCEUVAR,			IDC_EP_BOUNCEUVARSPIN,		1.0f,
		end,

	uspflectrobj_chaos,		_T("chaos"),		TYPE_PCNT_FRAC,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_CHAOS,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_BOUNCEUCHAOS,		IDC_EP_BOUNCEUCHAOSSPIN,	1.0f,
		end,

	uspflectrobj_friction,	_T("friction"),		TYPE_PCNT_FRAC,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_FRICTION,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_BOUNCEUFRICTION,		IDC_EP_BOUNCEUFRICTIONSPIN, 1.0f,
		end,

	uspflectrobj_velocity,	_T("inheritVelocity"),	TYPE_FLOAT,	P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_INHERIT,
		p_default,			1.0f,
		p_range,			0.0f, 9999999.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_BOUNCEUINHERIT,		IDC_EP_BOUNCEUINHERITSPIN,	SPIN_AUTOSCALE,
		end,

	uspflectrobj_refracts,	_T("refracts"),		TYPE_PCNT_FRAC,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_REFRACTS,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_REFRACTSU,			IDC_EP_REFRACTSUSPIN,		1.0f,
		end,

	uspflectrobj_decel,		_T("deceleration"),	TYPE_FLOAT,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_AP_DECEL,
		p_default,			0.0f,
		p_range,			0.0f,65000.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_DECELU,				IDC_EP_DECELUSPIN,			1.0f,
		end,

	uspflectrobj_decelvar,	_T("decelVar"),	TYPE_PCNT_FRAC,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_AP_DECELVAR,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_DECELUVAR,			IDC_EP_DECELUVARSPIN,		1.0f,
		end,

	uspflectrobj_refraction,_T("refraction"),	TYPE_PCNT_FRAC,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_REFRACTION,
		p_default,			0.0f,
		p_range,			-100.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_REFRACTIONU,			IDC_EP_REFRACTIONUSPIN,		1.0f,
		end,

	uspflectrobj_refractvar,_T("refractionVar"),TYPE_PCNT_FRAC,	P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_REFRACTVAR,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_REFRACTUVAR,			IDC_EP_REFRACTUVARSPIN,		1.0f,
		end,

	uspflectrobj_diffusion,	_T("diffusion"),	TYPE_PCNT_FRAC,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_DIFFUSION,
		p_default,			0.0f,
		p_range,			0.0f,100.0f,	
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_DIFFUSIONU,			IDC_EP_DIFFUSIONUSPIN,		1.0f,
		end,

	uspflectrobj_diffusionvar,	_T("diffusionVar"),TYPE_PCNT_FRAC,	P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_DIFFUSIONVAR,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_DIFFUSIONUVAR,		IDC_EP_DIFFUSIONUVARSPIN,	1.0f,
		end,

	uspflectrobj_radius,	_T("radius"),		TYPE_FLOAT,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_ICONSIZE,
		p_default,			0.0f,
		p_range,			0.0f,9999999.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_UNIVERSE,	IDC_EP_ICONSIZE,			IDC_EP_ICONSIZESPIN,		SPIN_AUTOSCALE,
		end,

	uspflectrobj_spawn,		_T("spawn"),		TYPE_PCNT_FRAC,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_AP_SPAWN,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_SPAWNSONLY,			IDC_EP_SPAWNSONLYSPIN,		1.0f,
		end,

	uspflectrobj_passvel,	_T("passVelocity"),TYPE_FLOAT,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_AP_PASSVEL,
		p_default,			0.0f,
		p_range,			0.0f, 9999999.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_SPAWNONLYDECEL,		IDC_EP_SPAWNONLYDECELSPIN,	SPIN_AUTOSCALE,
		end,

	uspflectrobj_passvelvar,_T("passVelocityVar"),	TYPE_PCNT_FRAC,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_AP_PASSVELVAR,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_SPAWNSONLYDECELVAR,	IDC_EP_SPAWNSONLYDECELVARSPIN, 1.0f,
		end,

//watje ref to hold the collision engine
	uspflectrobj_collider,  _T(""),		TYPE_REFTARG, 	0,0, 	//IDS_EP_FRICTION, 
		end, 

	end
);

		/*
//--- UniDefObject Parameter map/block descriptors ------------------

#define PB_TIMEON		0
#define PB_TIMEOFF		1
#define PB_AFFECTS		2
#define PB_BOUNCE		3
#define PB_BOUNCEVAR	4
#define PB_CHAOS		5
#define PB_FRICTION		6
#define PB_INHERIT		7
#define PB_REFRACTS		8
#define PB_DECEL		9
#define PB_DECELVAR		10
#define PB_REFRACTION	11
#define PB_REFRACTVAR	12
#define PB_DIFFUSION	13
#define PB_DIFFUSIONVAR	14
#define PB_ICONSIZE		15
#define PB_SPAWN		16
#define PB_PASSVEL		17
#define PB_PASSVELVAR	18

static ParamUIDesc descUSpawnParam[] = {
	// Time On
	ParamUIDesc(
		PB_TIMEON,
		EDITTYPE_TIME,
		IDC_EP_TIMEON,IDC_EP_TIMEONSPIN,
		-999999999.0f, 999999999.0f,
		10.0f),
	
	// Time Off
	ParamUIDesc(
		PB_TIMEOFF,
		EDITTYPE_TIME,
		IDC_EP_TIMEOFF,IDC_EP_TIMEOFFSPIN,
		-999999999.0f, 999999999.0f,
		10.0f),
	
	// Affects Percentage
	ParamUIDesc(
		PB_AFFECTS,
		EDITTYPE_FLOAT,
		IDC_EP_AFFECTS,IDC_EP_AFFECTSSPIN,
		0.0f, 100.0f,
		1.0f,
		stdPercentDim),

	// Bounce
	ParamUIDesc(
		PB_BOUNCE,
		EDITTYPE_FLOAT,
		IDC_EP_BOUNCEU,IDC_EP_BOUNCEUSPIN,
		0.0f, 9999999.0f,
		SPIN_AUTOSCALE),
	
	// BounceVar
	ParamUIDesc(
		PB_BOUNCEVAR,
		EDITTYPE_FLOAT,
		IDC_EP_BOUNCEUVAR,IDC_EP_BOUNCEUVARSPIN,
		0.0f, 100.0f,
		1.0f,
		stdPercentDim),
	
	// Chaos
	ParamUIDesc(
		PB_CHAOS,
		EDITTYPE_FLOAT,
		IDC_EP_BOUNCEUCHAOS,IDC_EP_BOUNCEUCHAOSSPIN,
		0.0f, 100.0f,
		1.0f,
		stdPercentDim),
	
	// Friction
	ParamUIDesc(
		PB_FRICTION,
		EDITTYPE_FLOAT,
		IDC_EP_BOUNCEUFRICTION,IDC_EP_BOUNCEUFRICTIONSPIN,
		0.0f, 100.0f,
		0.1f),
	
	// Inherit
	ParamUIDesc(
		PB_INHERIT,
		EDITTYPE_FLOAT,
		IDC_EP_BOUNCEUINHERIT,IDC_EP_BOUNCEUINHERITSPIN,
		0.0f, 9999999.0f,
		SPIN_AUTOSCALE),
	
	// Refracts Percentage
	ParamUIDesc(
		PB_REFRACTS,
		EDITTYPE_FLOAT,
		IDC_EP_REFRACTSU,IDC_EP_REFRACTSUSPIN,
		0.0f, 100.0f,
		1.0f,
		stdPercentDim),

	// Refraction Decel
	ParamUIDesc(
		PB_DECEL,
		EDITTYPE_FLOAT,
		IDC_EP_DECELU,IDC_EP_DECELUSPIN,
		0.0f, 9999999.0f,
		SPIN_AUTOSCALE),
	
	// Refraction Decel Var
	ParamUIDesc(
		PB_DECELVAR,
		EDITTYPE_FLOAT,
		IDC_EP_DECELUVAR,IDC_EP_DECELUVARSPIN,
		0.0f, 100.0f,
		1.0f,
		stdPercentDim),

	// Refraction
	ParamUIDesc(
		PB_REFRACTION,
		EDITTYPE_FLOAT,
		IDC_EP_REFRACTIONU,IDC_EP_REFRACTIONUSPIN,
		-100.0f, 100.0f,
		1.0f,
		stdPercentDim),

	// Refraction Var
	ParamUIDesc(
		PB_REFRACTVAR,
		EDITTYPE_FLOAT,
		IDC_EP_REFRACTUVAR,IDC_EP_REFRACTUVARSPIN,
		0.0f, 100.0f,
		1.0f,
		stdPercentDim),

	// Diffusion
	ParamUIDesc(
		PB_DIFFUSION,
		EDITTYPE_FLOAT,
		IDC_EP_DIFFUSIONU,IDC_EP_DIFFUSIONUSPIN,
		0.0f, 100.0f,
		1.0f,
		stdPercentDim),

	// Diffusion Var
	ParamUIDesc(
		PB_DIFFUSIONVAR,
		EDITTYPE_FLOAT,
		IDC_EP_DIFFUSIONUVAR,IDC_EP_DIFFUSIONUVARSPIN,
		0.0f, 100.0f,
		1.0f,
		stdPercentDim),
	
	// Spawns Percentage
	ParamUIDesc(
		PB_SPAWN,
		EDITTYPE_FLOAT,
		IDC_EP_SPAWNSONLY,IDC_EP_SPAWNSONLYSPIN,
		0.0f, 100.0f,
		1.0f,
		stdPercentDim),

	// Pass Velocity
	ParamUIDesc(
		PB_PASSVEL,
		EDITTYPE_FLOAT,
		IDC_EP_SPAWNONLYDECEL,IDC_EP_SPAWNONLYDECELSPIN,
		0.0f, 9999999.0f,
		SPIN_AUTOSCALE),
	
	// Pass Velocity Var
	ParamUIDesc(
		PB_PASSVELVAR,
		EDITTYPE_FLOAT,
		IDC_EP_SPAWNSONLYDECELVAR,IDC_EP_SPAWNSONLYDECELVARSPIN,
		0.0f, 100.0f,
		1.0f,
		stdPercentDim),

	// Icon Size
	ParamUIDesc(
		PB_ICONSIZE,
		EDITTYPE_UNIVERSE,
		IDC_EP_ICONSIZE,IDC_EP_ICONSIZESPIN,
		0.0f, 9999999.0f,
		SPIN_AUTOSCALE),
	};
*/


//#define PARAMDESC_LENGTH	19

ParamBlockDescID descUSpawnVer0[] = {
	{ TYPE_INT, NULL, FALSE, 0 },	//timeon
	{ TYPE_INT, NULL, TRUE, 1 },	//timeoff
	{ TYPE_FLOAT, NULL, TRUE, 2 },//affects
	{ TYPE_FLOAT, NULL, TRUE, 3 },//bounce
	{ TYPE_FLOAT, NULL, TRUE, 4 },//bouncevar
	{ TYPE_FLOAT, NULL, TRUE, 5 },//chaos
	{ TYPE_FLOAT, NULL, TRUE, 6 },//friction
	{ TYPE_FLOAT, NULL, TRUE, 7 },//inherit
	{ TYPE_FLOAT, NULL, TRUE, 8 },
	{ TYPE_FLOAT, NULL, TRUE, 9 },
	{ TYPE_FLOAT, NULL, TRUE, 10 },
	{ TYPE_FLOAT, NULL, TRUE, 11 },
	{ TYPE_FLOAT, NULL, TRUE, 12 },
	{ TYPE_FLOAT, NULL, TRUE, 13 },
	{ TYPE_FLOAT, NULL, TRUE, 14 },
	{ TYPE_FLOAT, NULL, FALSE, 15 }};

ParamBlockDescID descUSpawnVer1[] = {
	{ TYPE_INT, NULL, FALSE, 0 },	//timeon
	{ TYPE_INT, NULL, TRUE, 1 },	//timeoff
	{ TYPE_FLOAT, NULL, TRUE, 2 },//affects
	{ TYPE_FLOAT, NULL, TRUE, 3 },//bounce
	{ TYPE_FLOAT, NULL, TRUE, 4 },//bouncevar
	{ TYPE_FLOAT, NULL, TRUE, 5 },//chaos
	{ TYPE_FLOAT, NULL, TRUE, 6 },//friction
	{ TYPE_FLOAT, NULL, TRUE, 7 },//inherit
	{ TYPE_FLOAT, NULL, TRUE, 8 },
	{ TYPE_FLOAT, NULL, TRUE, 9 },
	{ TYPE_FLOAT, NULL, TRUE, 10 },
	{ TYPE_FLOAT, NULL, TRUE, 11 },
	{ TYPE_FLOAT, NULL, TRUE, 12 },
	{ TYPE_FLOAT, NULL, TRUE, 13 },
	{ TYPE_FLOAT, NULL, TRUE, 14 },
	{ TYPE_FLOAT, NULL, FALSE, 15 },
	{ TYPE_FLOAT, NULL, TRUE, 16 },
	{ TYPE_FLOAT, NULL, TRUE, 17 },
	{ TYPE_FLOAT, NULL, TRUE, 18 },
};

#define PBLOCK_LENGTH	19

// Array of old ParamBlock Ed. 1 versions
static ParamVersionDesc usversions[] = 
{
	ParamVersionDesc(descUSpawnVer0,16,0),
	ParamVersionDesc(descUSpawnVer1,19,1),
};

#define NUM_OLDVERSIONS	2
//#define CURRENT_VERSION	3

//static ParamVersionDesc ucurVersion(descUSpawnVer1,PBLOCK_LENGTH,CURRENT_VERSION);

//--- Universal Deflect object methods -----------------------------------------
class CreateUSpawnDeflObjectProc : public MouseCallBack,ReferenceMaker {
	private:
		IObjParam *ip;
		void Init(IObjParam *i) {ip=i;}
		CreateMouseCallBack *createCB;	
		INode *CloudNode;
		USpawnDeflObject *UspObj;
		int attachedToNode;
		IObjCreate *createInterface;
		ClassDesc *cDesc;
		Matrix3 mat;  // the nodes TM relative to the CP
		Point3 p0,p1;
		IPoint2 sp0, sp1;
		BOOL square,dostuff;

		int ignoreSelectionChange;

		int lastPutCount;
		void CreateNewObject();	

		int NumRefs() { return 1; }
		RefTargetHandle GetReference(int i) { return (RefTargetHandle)CloudNode; } 
		void SetReference(int i, RefTargetHandle rtarg) { CloudNode = (INode *)rtarg; }

		// StdNotifyRefChanged calls this, which can change the partID to new value 
		// If it doesnt depend on the particular message& partID, it should return
		// REF_DONTCARE
		BOOL SupportAutoGrid(){return TRUE;}
	    RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	    	PartID& partID,  RefMessage message);
	public:
		void Begin( IObjCreate *ioc, ClassDesc *desc );
		void End();
		
		CreateUSpawnDeflObjectProc()
			{
			ignoreSelectionChange = FALSE;
			}
		int createmethod(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
		int proc( HWND hwnd, int msg, int point, int flag, IPoint2 m );
	};


#define CID_CREATEUSpawnDeflObjectMODE	CID_USER + 12

class CreateUSpawnDeflObjectMode : public CommandMode {		
	public:		
		CreateUSpawnDeflObjectProc proc;
		IObjParam *ip;
		USpawnDeflObject *obj;
		void Begin( IObjCreate *ioc, ClassDesc *desc ) { proc.Begin( ioc, desc ); }
		void End() { proc.End(); }
		void JumpStart(IObjParam *i,USpawnDeflObject*o);

		int Class() {return CREATE_COMMAND;}
		int ID() { return CID_CREATEUSpawnDeflObjectMODE; }
		MouseCallBack *MouseProc(int *numPoints) {*numPoints = 10000; return &proc;}
		ChangeForegroundCallback *ChangeFGProc() {return CHANGE_FG_SELECTED;}
		BOOL ChangeFG( CommandMode *oldMode ) { return (oldMode->ChangeFGProc() != CHANGE_FG_SELECTED); }
		void EnterMode() 
		{ GetCOREInterface()->PushPrompt(GetString(IDS_AP_CREATEMODE));
		  SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR)));
		}
		void ExitMode() {GetCOREInterface()->PopPrompt();SetCursor(LoadCursor(NULL, IDC_ARROW));}
	};
static CreateUSpawnDeflObjectMode theCreateUSpawnDeflObjectMode;

RefResult CreateUSpawnDeflObjectProc::NotifyRefChanged(
	Interval changeInt, 
	RefTargetHandle hTarget, 
	PartID& partID,  
	RefMessage message) 
	{
	switch (message) {
		case REFMSG_PRENOTIFY_PASTE:
		case REFMSG_TARGET_SELECTIONCHANGE:
		 	if ( ignoreSelectionChange ) {
				break;
				}
		 	if ( UspObj && CloudNode==hTarget ) {
				// this will set camNode== NULL;
				theHold.Suspend();
				DeleteReference(0);
				theHold.Resume();
				goto endEdit;
				}
			// fall through

		case REFMSG_TARGET_DELETED:		
			if (UspObj && CloudNode==hTarget ) {
				endEdit:
				if (createInterface->GetCommandMode()->ID() == CID_STDPICK) 
				{ if (UspObj->creating) 
						{  theCreateUSpawnDeflObjectMode.JumpStart(UspObj->ip,UspObj);
						   createInterface->SetCommandMode(&theCreateUSpawnDeflObjectMode);
					    } 
				  else {createInterface->SetStdCommandMode(CID_OBJMOVE);}
				}
#ifdef _OSNAP
				UspObj->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
#ifndef NO_CREATE_TASK	// russom - 12/04/01
				UspObj->EndEditParams( (IObjParam*)createInterface, 0, NULL);
#endif
				UspObj  = NULL;				
				CloudNode    = NULL;
				CreateNewObject();	
				attachedToNode = FALSE;
				}
			break;		
		}
	return REF_SUCCEED;
	}

void CreateUSpawnDeflObjectProc::Begin( IObjCreate *ioc, ClassDesc *desc )
	{
	createInterface = ioc;
	cDesc           = desc;
	attachedToNode  = FALSE;
	createCB        = NULL;
	CloudNode         = NULL;
	UspObj       = NULL;
	dostuff=0;
	CreateNewObject();
	}
void CreateUSpawnDeflObjectProc::CreateNewObject()
	{
	SuspendSetKeyMode();
    createInterface->GetMacroRecorder()->BeginCreate(cDesc);
	UspObj = (USpawnDeflObject*)cDesc->Create();
	lastPutCount  = theHold.GetGlobalPutCount();
	
	// Start the edit params process
	if ( UspObj ) {
#ifndef NO_CREATE_TASK	// russom - 12/04/01
		UspObj->BeginEditParams( (IObjParam*)createInterface, BEGIN_EDIT_CREATE, NULL );
#endif
		UspObj->SetAFlag(A_OBJ_CREATING);
#ifdef _OSNAP
		UspObj->SetAFlag(A_OBJ_LONG_CREATE);
#endif
		}	
	ResumeSetKeyMode();
	}

//LACamCreationManager::~LACamCreationManager
void CreateUSpawnDeflObjectProc::End()
{ if ( UspObj ) 
	{
#ifdef _OSNAP
	UspObj->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
#ifndef NO_CREATE_TASK	// russom - 12/04/01
	UspObj->EndEditParams( (IObjParam*)createInterface, 
	                    	          END_EDIT_REMOVEUI, NULL);
#endif
		if ( !attachedToNode ) 
		{	// RB 4-9-96: Normally the hold isn't holding when this 
			// happens, but it can be in certain situations (like a track view paste)
			// Things get confused if it ends up with undo...
			theHold.Suspend(); 
			delete UspObj;
			UspObj = NULL;
			theHold.Resume();
			createInterface->GetMacroRecorder()->Cancel();
			if (theHold.GetGlobalPutCount()!=lastPutCount) 
				GetSystemSetting(SYSSET_CLEAR_UNDO);
		} 
 else if ( CloudNode ) {
			 // Get rid of the reference.
			theHold.Suspend();
			DeleteReference(0);  // sets camNode = NULL
			theHold.Resume();
			}
	}
}

void CreateUSpawnDeflObjectMode::JumpStart(IObjParam *i,USpawnDeflObject *o)
	{
	ip  = i;
	obj = o;
	//MakeRefByID(FOREVER,0,svNode);
#ifndef NO_CREATE_TASK	// russom - 12/04/01
	obj->BeginEditParams(i,BEGIN_EDIT_CREATE,NULL);
#endif
	}


int USpawnDeflClassDesc::BeginCreate(Interface *i)
	{	
	SuspendSetKeyMode();
	IObjCreate *iob = i->GetIObjCreate();
	theCreateUSpawnDeflObjectMode.Begin(iob,this);
	iob->PushCommandMode(&theCreateUSpawnDeflObjectMode);
	return TRUE;
	}

int USpawnDeflClassDesc::EndCreate(Interface *i)
	{
	ResumeSetKeyMode();
	theCreateUSpawnDeflObjectMode.End();
	i->RemoveMode(&theCreateUSpawnDeflObjectMode);
	macroRec->EmitScript();  // 10/00
	return TRUE;
	}

int CreateUSpawnDeflObjectProc::createmethod(
		ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat)
	{	Point3 d;
#ifdef _3D_CREATE
	DWORD snapdim = SNAP_IN_3D;
#else
	DWORD snapdim = SNAP_IN_PLANE;
#endif

#ifdef _OSNAP
	if (msg == MOUSE_FREEMOVE)
	{ vpt->SnapPreview(m,m,NULL, snapdim);
	}
#endif

	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {
			case 0:
				sp0 = m;
				p0  = vpt->SnapPoint(m,m,NULL,snapdim);
				mat.SetTrans(p0);
//				UspObj->pblock->SetValue(PB_ICONSIZE,0,0.01f);
				UspObj->pblock2->SetValue(uspflectrobj_radius,0,0.01f);
//				UspObj->pmapParam->Invalidate();
				uspflector_param_blk.InvalidateUI();
				break;

			case 1: {
				mat.IdentityMatrix();
				sp1 = m;
				p1  = vpt->SnapPoint(m,m,NULL,snapdim);
				Point3 center = (p0+p1)/float(2);
				mat.SetTrans(center);
//				UspObj->pblock->SetValue(PB_ICONSIZE,0,(float)fabs(p1.x-p0.x));
				UspObj->pblock2->SetValue(uspflectrobj_radius,0,(float)fabs(p1.x-p0.x));
//				UspObj->pmapParam->Invalidate();
				uspflector_param_blk.InvalidateUI();

				if (msg==MOUSE_POINT) {
					if (Length(m-sp0)<3) {						
						return CREATE_ABORT;
					} else {
						return CREATE_STOP;
						}
					}
				break;
				}

			}
	} else {
		if (msg == MOUSE_ABORT)
			return CREATE_ABORT;
		}	
	return TRUE;
	}

int CreateUSpawnDeflObjectProc::proc(HWND hwnd,int msg,int point,int flag,
				IPoint2 m )
{	int res;	
	ViewExp *vpx = createInterface->GetViewport(hwnd); 
	assert( vpx );
#ifdef _3D_CREATE
	DWORD snapdim = SNAP_IN_3D;
#else
	DWORD snapdim = SNAP_IN_PLANE;
#endif

	if (!dostuff)
	switch ( msg ) {
		case MOUSE_POINT:
			switch ( point ) {
				case 0:
					assert( UspObj );					
					vpx->CommitImplicitGrid(m, flag );
					if ( createInterface->SetActiveViewport(hwnd) ) {
						return FALSE;
						}

					if (createInterface->IsCPEdgeOnInView()) { 
						res = FALSE;
						goto done;
						}

					if ( attachedToNode ) {
				   		// send this one on its way
#ifdef _OSNAP
						UspObj->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
#ifndef NO_CREATE_TASK	// russom - 12/04/01
				   		UspObj->EndEditParams( (IObjParam*)createInterface, 0, NULL);
#endif
              createInterface->GetMacroRecorder()->EmitScript();
						if (CloudNode) {
							theHold.Suspend();
							DeleteReference(0);
							theHold.Resume();
							}

						// new object
						CreateNewObject();   // creates UniObj
						}

				   	theHold.Begin();	 // begin hold for undo
					mat.IdentityMatrix();

					// link it up
					CloudNode = createInterface->CreateObjectNode( UspObj);
					attachedToNode = TRUE;
          UspObj->ClearAFlag(A_OBJ_CREATING);
					assert( CloudNode );					
					createCB = NULL;
					createInterface->SelectNode( CloudNode );
					
					// Reference the new node so we'll get notifications.
					theHold.Suspend();
					MakeRefByID( FOREVER, 0, CloudNode);
					theHold.Resume();
					mat.SetTrans(vpx->SnapPoint(m,m,NULL,snapdim));
//					macroRec->Disable();   // 10/00
					createInterface->SetNodeTMRelConstPlane(CloudNode, mat);
//					macroRec->Enable();
					dostuff=1;
					res = TRUE;
					break;
					
				}			
			break;

		case MOUSE_MOVE:
			//mat[3] = vpx->GetPointOnCP(m);
			mat.SetTrans(vpx->SnapPoint(m,m,NULL,snapdim));
			createInterface->RedrawViews(createInterface->GetTime());	   
			res = TRUE;
			break;

		case MOUSE_FREEMOVE:
			SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR)));
#ifdef _OSNAP  //PREVIEW SNAP
			res = createmethod(vpx,msg,point,flag,m,mat);
#endif
		vpx->TrackImplicitGrid(m);
			break;

	case MOUSE_PROPCLICK:
		createInterface->SetStdCommandMode(CID_OBJMOVE);
		break;
		case MOUSE_ABORT: goto abort;
		}
	int result; 
	if (dostuff)
	{ result=createmethod(vpx,msg,point,flag,m,mat);
//	  UspObj->BuildEmitter(createInterface->GetTime(),UspObj->);
	  createInterface->RedrawViews(createInterface->GetTime()); 
	  if (result==CREATE_STOP)
	  { res=FALSE;dostuff=0;				
#ifdef _OSNAP
         UspObj->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
	  theHold.Accept(GetString(IDS_EP_CREATE));	} 
	  else if (result==CREATE_ABORT)
	  { dostuff=0;
	    goto abort;}
	}
	done:
	if ((res == CREATE_STOP)||(res==CREATE_ABORT))
		vpx->ReleaseImplicitGrid();
	createInterface->ReleaseViewport(vpx); 
	return res;
	abort:
		assert( UspObj );
#ifdef _OSNAP
		UspObj->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
#ifndef NO_CREATE_TASK	// russom - 12/04/01
		UspObj->EndEditParams( (IObjParam*)createInterface,0,NULL);
#endif
		theHold.Cancel();	 // deletes both the Cloudera and target.
		// Bayboro: the following single line is a fix for #262308
		macroRec->Cancel();
		if (theHold.GetGlobalPutCount()!=lastPutCount) 
					GetSystemSetting(SYSSET_CLEAR_UNDO);
		CloudNode = NULL;			
		createInterface->RedrawViews(createInterface->GetTime()); 
		CreateNewObject();	
		attachedToNode = FALSE;
		res = FALSE;
		goto done;
	}
static BOOL IsGEOM(Object *obj)
{ if (obj!=NULL) 
  { if (obj->SuperClassID()==GEOMOBJECT_CLASS_ID)
    { if (obj->IsSubClassOf(triObjectClassID)) 
        return TRUE;
      else 
	  { if (obj->CanConvertToType(triObjectClassID)) 
	  	return TRUE;			
	  }
	}
  }
  return FALSE;
}

BOOL UniPickOperand::Filter(INode *node)
	{
	if ((node)&&(!node->IsGroupHead())) {
		ObjectState os = node->GetObjectRef()->Eval(po->ip->GetTime());
		if (os.obj->IsParticleSystem() || os.obj->SuperClassID()!=GEOMOBJECT_CLASS_ID) {
			node = NULL;
			return FALSE;
			}
		node->BeginDependencyTest();
		po->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
		if(node->EndDependencyTest()) {
			node = NULL;
			return FALSE;
			}
		}

	return node ? TRUE : FALSE;
	}

BOOL UniPickOperand::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
	{	
	INode *node = ip->PickNode(hWnd,m,this);
	
	if ((node)&&(node->IsGroupHead())) 
	{	ObjectState os = node->GetObjectRef()->Eval(ip->GetTime());
		if ((os.obj->SuperClassID()!=GEOMOBJECT_CLASS_ID)||(!IsGEOM(os.obj))) {
			node = NULL;
			return FALSE;
			}
		node->BeginDependencyTest();
		po->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
		if(node->EndDependencyTest()) {
			node = NULL;
			return FALSE;
		}
	}
	return node ? TRUE : FALSE;
	}

void USpawnDeflObject::ShowName()
{TSTR name; 
 FormatName(name= TSTR(GetString(IDS_EP_ITEMSTR)) + (custnode ? custname : TSTR(GetString(IDS_EP_NONE))));
SetWindowText(GetDlgItem(hParams, IDC_EP_PICKNAME), name);
}

BOOL UniPickOperand::Pick(IObjParam *ip,ViewExp *vpt)
	{BOOL groupflag=0;
	INode *node = vpt->GetClosestHit();
	assert(node);
	INodeTab nodes;
	if (node->IsGroupMember()) 
	{ groupflag=1;
	  while (node->IsGroupMember()) node=node->GetParentNode();
	}
	int subtree=0;
	if (groupflag) MakeGroupNodeList(node,&nodes,subtree,ip->GetTime());
	else{ nodes.SetCount(1);nodes[0]=node;}
	ip->FlashNodes(&nodes);
	theHold.Begin();
	theHold.Put(new CreateUSpawnDeflPickNode(po,node));

//	po->custnode=node;
	if (po->custnode) po->ReplaceReference(CUSTNODE,node,TRUE);
	else po->MakeRefByID(FOREVER,CUSTNODE,node);	
	po->NotifyDependents(FOREVER, 0, REFMSG_CHANGE);
	theHold.Accept(GetString(IDS_EP_USPPICK));
	po->custname = TSTR(node->GetName());
	// Automatically check show result and do one update
	po->ShowName();	
	if (po->creating) {
		theCreateUSpawnDeflObjectMode.JumpStart(ip,po);
		ip->SetCommandMode(&theCreateUSpawnDeflObjectMode);
		ip->RedrawViews(ip->GetTime());
		return FALSE;
	} else {
		return TRUE;
		}
	}

void UniPickOperand::EnterMode(IObjParam *ip)
	{
	ICustButton *iBut;
	iBut=GetICustButton(GetDlgItem(po->hParams,IDC_EP_PICKBUTTON));
	if (iBut) iBut->SetCheck(TRUE);
	ReleaseICustButton(iBut);
	GetCOREInterface()->PushPrompt(GetString(IDS_AP_PICKMODE));
	}

void UniPickOperand::ExitMode(IObjParam *ip)
	{
	ICustButton *iBut;
	iBut=GetICustButton(GetDlgItem(po->hParams,IDC_EP_PICKBUTTON));
	if (iBut) iBut->SetCheck(FALSE);
	ReleaseICustButton(iBut);
    GetCOREInterface()->PopPrompt();
	}

class USpawnDeflObjectDlgProc : public ParamMap2UserDlgProc {
	public:
		USpawnDeflObject *po;

		USpawnDeflObjectDlgProc(USpawnDeflObject *p) {po=p;}
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
		void Update(TimeValue t);
	};
void USpawnDeflObjectDlgProc::Update(TimeValue t)
{	po->ShowName();
	float size;
//	po->pblock->GetValue(PB_ICONSIZE,0,size,FOREVER);
	po->pblock2->GetValue(uspflectrobj_radius,0,size,FOREVER);
	TurnButton(po->hParams,IDC_EP_PICKBUTTON,(size>=0.01f));
}

BOOL USpawnDeflObjectDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{int FToTick=(int)((float)TIME_TICKSPERSEC/(float)GetFrameRate());
	switch (msg) {
		case WM_INITDIALOG: {
			ICustButton *iBut = GetICustButton(GetDlgItem(hWnd,IDC_EP_PICKBUTTON));
			iBut->SetType(CBT_CHECK);
			iBut->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(iBut);
			po->hParams=hWnd;
			Update(t);
			return FALSE;	// stop default keyboard focus - DB 2/27  
			}
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{    case IDC_EP_PICKBUTTON:
				   { if (po->ip->GetCommandMode()->ID() == CID_STDPICK) 
					{ if (po->creating) 
						{  theCreateUSpawnDeflObjectMode.JumpStart(po->ip,po);
							po->ip->SetCommandMode(&theCreateUSpawnDeflObjectMode);
						} else {po->ip->SetStdCommandMode(CID_OBJMOVE);}
					} else 
						{ po->pickCB.po = po;						
						  po->ip->SetPickMode(&po->pickCB);
						}
					return TRUE;
					break;
				}

			}
			break;	
		}
	return FALSE;
	}

USpawnDeflObject::USpawnDeflObject()
{
//	MakeRefByID(FOREVER, 0, 
//		CreateParameterBlock(descUSpawnVer1, PBLOCK_LENGTH, CURRENT_VERSION));
//	assert(pblock);	

	pblock2 = NULL;
	USpawnDeflDesc.MakeAutoParamBlocks(this);
	assert(pblock2);

//	pblock->SetValue(PB_TIMEON,0,0);
//	pblock->SetValue(PB_TIMEOFF,0,100*GetTicksPerFrame());
//	pblock->SetValue(PB_AFFECTS,0,1.0f);
//	pblock->SetValue(PB_BOUNCE,0,1.0f);
//	pblock->SetValue(PB_BOUNCEVAR,0,0.0f);
//	pblock->SetValue(PB_CHAOS,0,0.0f);
//	pblock->SetValue(PB_FRICTION,0,0.0f);
//	pblock->SetValue(PB_INHERIT,0,1.0f);
//	pblock->SetValue(PB_REFRACTS,0,1.0f);
//	pblock->SetValue(PB_DECEL,0,1.0f);
//	pblock->SetValue(PB_DECELVAR,0,0.0f);
//	pblock->SetValue(PB_REFRACTION,0,0.5f);
//	pblock->SetValue(PB_REFRACTVAR,0,0.0f);
//	pblock->SetValue(PB_DIFFUSION,0,0.0f);
//	pblock->SetValue(PB_DIFFUSIONVAR,0,0.0f);
//	pblock->SetValue(PB_SPAWN,0,1.0f);
//	pblock->SetValue(PB_PASSVEL,0,1.0f);
//	pblock->SetValue(PB_PASSVELVAR,0,0);
//	pblock->SetValue(PB_ICONSIZE,0,0.0f);

	pblock2->SetValue(uspflectrobj_timeon,0,0);
	pblock2->SetValue(uspflectrobj_timeoff,0,100*GetTicksPerFrame());
	pblock2->SetValue(uspflectrobj_affects,0,1.0f);
	pblock2->SetValue(uspflectrobj_bounce,0,1.0f);
	pblock2->SetValue(uspflectrobj_bouncevar,0,0.0f);
	pblock2->SetValue(uspflectrobj_chaos,0,0.0f);
	pblock2->SetValue(uspflectrobj_friction,0,0.0f);
//	pblock2->SetValue(uspflectrobj_velocity,0,1.0f);
	pblock2->SetValue(uspflectrobj_refracts,0,1.0f);
	pblock2->SetValue(uspflectrobj_decel,0,1.0f);
	pblock2->SetValue(uspflectrobj_decelvar,0,0.0f);
	pblock2->SetValue(uspflectrobj_refraction,0,0.5f);
	pblock2->SetValue(uspflectrobj_refractvar,0,0.0f);
	pblock2->SetValue(uspflectrobj_diffusion,0,0.0f);
	pblock2->SetValue(uspflectrobj_diffusionvar,0,0.0f);
	pblock2->SetValue(uspflectrobj_spawn,0,1.0f);
	pblock2->SetValue(uspflectrobj_passvel,0,1.0f);
	pblock2->SetValue(uspflectrobj_passvelvar,0,0.0f);
	pblock2->SetValue(uspflectrobj_radius,0,0.0f);

	dmesh=NULL;
	vnorms=NULL;
	fnorms=NULL;
	srand(lastrnd=12345);
	t=99999;
	custname=TSTR(_T(" "));
	custnode=NULL;
	nv=0;nf=0;
	macroRec->Disable();

//watje create a new ref to our collision engine
	CollisionMesh *colm = (CollisionMesh*)CreateInstance(REF_MAKER_CLASS_ID, MESH_COLLISION_ID);
	if (colm)
	{
		pblock2->SetValue(uspflectrobj_collider,0,(ReferenceTarget*)colm);
	}
	macroRec->Enable();
}

USpawnDeflObject::~USpawnDeflObject()
{   
	DeleteAllRefsFromMe();
	if (pblock2)
	{
		ReferenceTarget *rt;
		pblock2->GetValue(uspflectrobj_collider,t,rt,FOREVER);
//		if (rt)
//			delete rt;
	}
	if (vnorms) 
		delete[] vnorms;
	if (fnorms) 
		delete[] fnorms;
	if (dmesh) 
	   delete dmesh;
}

Modifier *USpawnDeflObject::CreateWSMMod(INode *node)
{
	return new USpawnDeflMod(node,this);
}

RefTargetHandle USpawnDeflObject::Clone(RemapDir& remap) 
	{
	USpawnDeflObject* newob = new USpawnDeflObject();	
//	newob->ReplaceReference(0,pblock->Clone(remap));
	newob->ReplaceReference(0,pblock2->Clone(remap));
	if (custnode) newob->ReplaceReference(CUSTNODE,custnode);
	newob->custname=custname;
	newob->dmesh=NULL;
	newob->vnorms=NULL;
	newob->fnorms=NULL;
	BaseClone(this, newob, remap);
	return newob;
	}

void USpawnDeflObject::BeginEditParams(
		IObjParam *ip,ULONG flags,Animatable *prev)
{
	if (!hSot)
		hSot = ip->AddRollupPage( 
			hInstance, 
			MAKEINTRESOURCE(IDD_SW_DESC_LEGACY),
			DefaultSOTProc,
			GetString(IDS_EP_TOP), 
			(LPARAM)ip,APPENDROLL_CLOSED);

	SimpleWSMObject2::BeginEditParams(ip,flags,prev);
	this->ip = ip;

	if (flags&BEGIN_EDIT_CREATE) 
		creating = TRUE;
	else 
		creating = FALSE; 
	
	USpawnDeflDesc.BeginEditParams(ip, this, flags, prev);
	uspflector_param_blk.SetUserDlgProc(new USpawnDeflObjectDlgProc(this));

/*
	if (pmapParam) {		
		// Left over
		pmapParam->SetParamBlock(pblock);
	} else {		
		hSot = ip->AddRollupPage( 
			hInstance, 
			MAKEINTRESOURCE(IDD_SW_DESC),
			DefaultSOTProc,
			GetString(IDS_EP_TOP), 
			(LPARAM)ip,APPENDROLL_CLOSED);

		// Gotta make a new one.
		pmapParam = CreateCPParamMap(
			descUSpawnParam,PARAMDESC_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_AP_USPAWNDEFL),
			GetString(IDS_EP_PARAMETERS),
			0);
		}
		if (pmapParam)
			pmapParam->SetUserDlgProc(new USpawnDeflObjectDlgProc(this));
*/
}

void USpawnDeflObject::EndEditParams(
		IObjParam *ip, ULONG flags,Animatable *next)
{		
	SimpleWSMObject2::EndEditParams(ip,flags,next);

	USpawnDeflDesc.EndEditParams(ip, this, flags, next);

	if (flags&END_EDIT_REMOVEUI ) 
	{		
//		DestroyCPParamMap(pmapParam);
//		pmapParam = NULL;		
		if (hSot)
		{
//			ip->UnRegisterDlgWnd(hSot);
			ip->DeleteRollupPage(hSot);
			hSot = NULL;
		}
	}
	
	ip->ClearPickMode();
	this->ip = NULL;
	ip = NULL;
	creating = FALSE;
}

void USpawnDeflObject::MapKeys(TimeMap *map,DWORD flags)
{	Animatable::MapKeys(map,flags);
	TimeValue TempTime;
// remap values
//	pblock->GetValue(PB_TIMEON,0,TempTime,FOREVER);
	pblock2->GetValue(uspflectrobj_timeon,0,TempTime,FOREVER);
	TempTime=map->map(TempTime);
//	pblock->SetValue(PB_TIMEON,0,TempTime);
	pblock2->SetValue(uspflectrobj_timeon,0,TempTime);
//	pblock->GetValue(PB_TIMEOFF,0,TempTime,FOREVER);
	pblock2->GetValue(uspflectrobj_timeoff,0,TempTime,FOREVER);
	TempTime=map->map(TempTime);
//	pblock->SetValue(PB_TIMEOFF,0,TempTime);
	pblock2->SetValue(uspflectrobj_timeoff,0,TempTime);
}  

void USpawnDeflObject::BuildMesh(TimeValue t)
	{
	ivalid = FOREVER;
	float length,l,r2,r3,r4;
//	pblock->GetValue(PB_ICONSIZE,t,length,ivalid);
	pblock2->GetValue(uspflectrobj_radius,t,length,ivalid);
	l=length*0.5f;
	r2=0.5f*l;
	r3=0.25f*r2;
	r4=0.71f*r3;

	mesh.setNumVerts(28);
	mesh.setNumFaces(22);

	mesh.setVert(0,Point3( l, l, l));
	mesh.setVert(1,Point3( l, l,-l));
	mesh.setVert(2,Point3( l,-l, l));
	mesh.setVert(3,Point3( l,-l,-l));
	mesh.setVert(4,Point3(-l, l, l));
	mesh.setVert(5,Point3(-l, l,-l));
	mesh.setVert(6,Point3(-l,-l, l));
	mesh.setVert(7,Point3(-l,-l,-l));

	mesh.setVert( 8,Point3(0.0f, 0.0f , l      ));//a
	mesh.setVert( 9,Point3(0.0f, 0.0f , l+r2   ));//b
	mesh.setVert(10,Point3(0.0f, -r2  , l-r2   ));//c
	mesh.setVert(11,Point3(0.0f,  r2  , l-r2   ));//d
	mesh.setVert(12,Point3(0.0f,  r4  , l+r4));//b1
	mesh.setVert(13,Point3(0.0f, -r4  , l+r4));//b2
	mesh.setVert(14,Point3(0.0f, -r2  , l-r2+r3));//c1
	mesh.setVert(15,Point3(0.0f,-r2+r3, l-r2   ));//c2
	mesh.setVert(16,Point3(0.0f, r2-r3, l-r2   ));//d1
	mesh.setVert(17,Point3(0.0f,  r2  , l-r2+r3));//d2
	
	mesh.setVert(18,Point3(0.0f, 0.0f ,-l      ));//a
	mesh.setVert(19,Point3(0.0f, 0.0f ,-l-r2   ));//b
	mesh.setVert(20,Point3(0.0f, -r2  ,-l+r2   ));//c
	mesh.setVert(21,Point3(0.0f,  r2  ,-l+r2   ));//d
	mesh.setVert(22,Point3(0.0f,  r4  ,-l-r4));//b1
	mesh.setVert(23,Point3(0.0f, -r4  ,-l-r4));//b2
	mesh.setVert(24,Point3(0.0f, -r2  ,-l+r2-r3));//c1
	mesh.setVert(25,Point3(0.0f,-r2+r3,-l+r2   ));//c2
	mesh.setVert(26,Point3(0.0f, r2-r3,-l+r2   ));//d1
	mesh.setVert(27,Point3(0.0f,  r2  ,-l+r2-r3));//d2

	mesh.faces[0].setVerts(1,0,2);
	mesh.faces[0].setEdgeVisFlags(1,1,0);
	mesh.faces[0].setSmGroup(0);
	mesh.faces[1].setVerts(2,3,1);
	mesh.faces[1].setEdgeVisFlags(1,1,0);
	mesh.faces[1].setSmGroup(0);
	mesh.faces[2].setVerts(2,0,4);
	mesh.faces[2].setEdgeVisFlags(1,1,0);
	mesh.faces[2].setSmGroup(0);
	mesh.faces[3].setVerts(4,6,2);
	mesh.faces[3].setEdgeVisFlags(1,1,0);
	mesh.faces[3].setSmGroup(0);
	mesh.faces[4].setVerts(3,2,6);
	mesh.faces[4].setEdgeVisFlags(1,1,0);
	mesh.faces[4].setSmGroup(0);
	mesh.faces[5].setVerts(6,7,3);
	mesh.faces[5].setEdgeVisFlags(1,1,0);
	mesh.faces[5].setSmGroup(0);
	mesh.faces[6].setVerts(7,6,4);
	mesh.faces[6].setEdgeVisFlags(1,1,0);
	mesh.faces[6].setSmGroup(0);
	mesh.faces[7].setVerts(4,5,7);
	mesh.faces[7].setEdgeVisFlags(1,1,0);
	mesh.faces[7].setSmGroup(0);
	mesh.faces[8].setVerts(4,0,1);
	mesh.faces[8].setEdgeVisFlags(1,1,0);
	mesh.faces[8].setSmGroup(0);
	mesh.faces[9].setVerts(1,5,4);
	mesh.faces[9].setEdgeVisFlags(1,1,0);
	mesh.faces[9].setSmGroup(0);
	mesh.faces[10].setVerts(1,3,7);
	mesh.faces[10].setEdgeVisFlags(1,1,0);
	mesh.faces[10].setSmGroup(0);
	mesh.faces[11].setVerts(7,5,1);
	mesh.faces[11].setEdgeVisFlags(1,1,0);
	mesh.faces[11].setSmGroup(0);

	mesh.faces[12].setEdgeVisFlags(1,0,1);
	mesh.faces[12].setSmGroup(0);
	mesh.faces[12].setVerts(18,21,19);
	mesh.faces[13].setEdgeVisFlags(1,0,1);
	mesh.faces[13].setSmGroup(0);
	mesh.faces[13].setVerts(18,20,19);
	mesh.faces[14].setEdgeVisFlags(1,1,1);
	mesh.faces[14].setSmGroup(0);
	mesh.faces[14].setVerts(18,23,22);
	mesh.faces[15].setEdgeVisFlags(1,1,1);
	mesh.faces[15].setSmGroup(0);
	mesh.faces[15].setVerts(20,24,25);
	mesh.faces[16].setEdgeVisFlags(1,1,1);
	mesh.faces[16].setSmGroup(0);
	mesh.faces[16].setVerts(21,27,26);

	mesh.faces[17].setEdgeVisFlags(1,0,1);
	mesh.faces[17].setSmGroup(0);
	mesh.faces[17].setVerts(8,11,9);
	mesh.faces[18].setEdgeVisFlags(1,0,1);
	mesh.faces[18].setSmGroup(0);
	mesh.faces[18].setVerts(8,10,9);
	mesh.faces[19].setEdgeVisFlags(1,1,1);
	mesh.faces[19].setSmGroup(0);
	mesh.faces[19].setVerts(8,13,12);
	mesh.faces[20].setEdgeVisFlags(1,1,1);
	mesh.faces[20].setSmGroup(0);
	mesh.faces[20].setVerts(10,14,15);
	mesh.faces[21].setEdgeVisFlags(1,1,1);
	mesh.faces[21].setSmGroup(0);
	mesh.faces[21].setVerts(11,17,16);

	mesh.InvalidateGeomCache();
	}

void USpawnDeflObject::InvalidateUI() 
{
//	if (pmapParam) pmapParam->Invalidate();
	uspflector_param_blk.InvalidateUI(pblock2->LastNotifyParamID());
}

ParamDimension *USpawnDeflObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {		
		case uspflectrobj_timeon:		return stdTimeDim;
		case uspflectrobj_timeoff:		return stdTimeDim;
		case uspflectrobj_affects:		return stdPercentDim;
		case uspflectrobj_bouncevar:	return stdPercentDim;
		case uspflectrobj_chaos:		return stdPercentDim;
		case uspflectrobj_refracts:		return stdPercentDim;
		case uspflectrobj_decelvar:		return stdPercentDim;
		case uspflectrobj_refraction:	return stdPercentDim;
		case uspflectrobj_refractvar:	return stdPercentDim;
		case uspflectrobj_diffusion:	return stdPercentDim;
		case uspflectrobj_diffusionvar:	return stdPercentDim;
		case uspflectrobj_spawn:		return stdPercentDim;
		case uspflectrobj_passvelvar:	return stdPercentDim;
//		case PB_TIMEON:			return stdTimeDim;
//		case PB_TIMEOFF:		return stdTimeDim;
//		case PB_AFFECTS:		return stdPercentDim;
//		case PB_BOUNCEVAR:		return stdPercentDim;
//		case PB_CHAOS:			return stdPercentDim;
//		case PB_REFRACTS:		return stdPercentDim;
//		case PB_DECELVAR:		return stdPercentDim;
//		case PB_REFRACTION:		return stdPercentDim;
//		case PB_REFRACTVAR:		return stdPercentDim;
//		case PB_DIFFUSION:		return stdPercentDim;
//		case PB_DIFFUSIONVAR:	return stdPercentDim;
//		case PB_SPAWN:			return stdPercentDim;
//		case PB_PASSVELVAR:		return stdPercentDim;
		default: return defaultDim;
		}
	}

/*
TSTR USpawnDeflObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {				
		case PB_TIMEON:			return GetString(IDS_EP_TIMEON);
		case PB_TIMEOFF:		return GetString(IDS_EP_TIMEOFF);
		case PB_AFFECTS:		return GetString(IDS_EP_AFFECTS);
		case PB_BOUNCE:			return GetString(IDS_EP_BOUNCE);
		case PB_BOUNCEVAR:		return GetString(IDS_EP_BOUNCEVAR);
		case PB_CHAOS:			return GetString(IDS_EP_CHAOS);
		case PB_FRICTION:		return GetString(IDS_EP_FRICTION);
		case PB_INHERIT:		return GetString(IDS_EP_INHERIT);
		case PB_REFRACTS:		return GetString(IDS_EP_REFRACTS);
		case PB_DECEL:			return GetString(IDS_EP_PASSVEL);
		case PB_DECELVAR:		return GetString(IDS_EP_PASSVELVAR);
		case PB_REFRACTION:		return GetString(IDS_EP_REFRACTION);
		case PB_REFRACTVAR:		return GetString(IDS_EP_REFRACTVAR);
		case PB_DIFFUSION:		return GetString(IDS_EP_DIFFUSION);
		case PB_DIFFUSIONVAR:	return GetString(IDS_EP_DIFFUSIONVAR);
		case PB_ICONSIZE:		return GetString(IDS_EP_ICONSIZE);
		case PB_SPAWN:			return GetString(IDS_AP_SPAWN);
		case PB_PASSVEL:		return GetString(IDS_AP_PASSVEL);
		case PB_PASSVELVAR:		return GetString(IDS_AP_PASSVELVAR);
		default:				return TSTR(_T(""));
		}
	}
*/

//--- DeflectMod methods -----------------------------------------------

USpawnDeflMod::USpawnDeflMod(INode *node,USpawnDeflObject *obj)
	{	
//	MakeRefByID(FOREVER,SIMPWSMMOD_OBREF,obj);
	MakeRefByID(FOREVER,SIMPWSMMOD_NODEREF,node);	
		pblock = NULL;
	obRef=NULL;
	}

Interval USpawnDeflMod::GetValidity(TimeValue t) 
	{
	if (obRef && nodeRef) {
		Interval valid = FOREVER;
		Matrix3 tm;
		float f;		
		USpawnDeflObject *obj = (USpawnDeflObject*)GetWSMObject(t);
		TimeValue TempT;
//		obj->pblock->GetValue(PB_TIMEOFF,t,TempT,valid);
//		obj->pblock->GetValue(PB_AFFECTS,t,f,valid);
//		obj->pblock->GetValue(PB_BOUNCE,t,f,valid);
//		obj->pblock->GetValue(PB_BOUNCEVAR,t,f,valid);
//		obj->pblock->GetValue(PB_CHAOS,t,f,valid);
//		obj->pblock->GetValue(PB_FRICTION,t,f,valid);
//		obj->pblock->GetValue(PB_INHERIT,t,f,valid);
//		obj->pblock->GetValue(PB_REFRACTS,t,f,valid);
//		obj->pblock->GetValue(PB_DECEL,t,f,valid);
//		obj->pblock->GetValue(PB_DECELVAR,t,f,valid);
//		obj->pblock->GetValue(PB_REFRACTION,t,f,valid);
//		obj->pblock->GetValue(PB_REFRACTVAR,t,f,valid);
//		obj->pblock->GetValue(PB_DIFFUSION,t,f,valid);
//		obj->pblock->GetValue(PB_DIFFUSIONVAR,t,f,valid);
//		obj->pblock->GetValue(PB_ICONSIZE,t,f,valid);
//		obj->pblock->GetValue(PB_SPAWN,t,f,valid);
//		obj->pblock->GetValue(PB_PASSVEL,t,f,valid);
//		obj->pblock->GetValue(PB_PASSVELVAR,t,f,valid);

		obj->pblock2->GetValue(uspflectrobj_timeoff,t,TempT,valid);
		obj->pblock2->GetValue(uspflectrobj_affects,t,f,valid);
		obj->pblock2->GetValue(uspflectrobj_bounce,t,f,valid);
		obj->pblock2->GetValue(uspflectrobj_bouncevar,t,f,valid);
		obj->pblock2->GetValue(uspflectrobj_chaos,t,f,valid);
		obj->pblock2->GetValue(uspflectrobj_friction,t,f,valid);
		obj->pblock2->GetValue(uspflectrobj_velocity,t,f,valid);
		obj->pblock2->GetValue(uspflectrobj_refracts,t,f,valid);
		obj->pblock2->GetValue(uspflectrobj_decel,t,f,valid);
		obj->pblock2->GetValue(uspflectrobj_decelvar,t,f,valid);
		obj->pblock2->GetValue(uspflectrobj_refraction,t,f,valid);
		obj->pblock2->GetValue(uspflectrobj_refractvar,t,f,valid);
		obj->pblock2->GetValue(uspflectrobj_diffusion,t,f,valid);
		obj->pblock2->GetValue(uspflectrobj_diffusionvar,t,f,valid);
		obj->pblock2->GetValue(uspflectrobj_radius,t,f,valid);
		obj->pblock2->GetValue(uspflectrobj_spawn,t,f,valid);
		obj->pblock2->GetValue(uspflectrobj_passvel,t,f,valid);
		obj->pblock2->GetValue(uspflectrobj_passvelvar,t,f,valid);


		tm = nodeRef->GetObjectTM(t,&valid);
		
		return valid;
	} else {
		return FOREVER;
		}
	}

class USpawnDeflDeformer : public Deformer {
	public:		
		Point3 Map(int i, Point3 p) {return p;}
	};

static USpawnDeflDeformer USpawnddeformer;

Deformer& USpawnDeflMod::GetDeformer(
		TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
	{
	return USpawnddeformer;
	}

RefTargetHandle USpawnDeflMod::Clone(RemapDir& remap) 
	{
	USpawnDeflMod *newob = new USpawnDeflMod(nodeRef,(USpawnDeflObject*)obRef);	
	newob->SimpleWSMModClone(this);
	BaseClone(this, newob, remap);
	return newob;
	}

void USpawnDeflMod::ModifyObject(
		TimeValue t, ModContext &mc, ObjectState *os, INode *node)
	{
	ParticleObject *obj = GetParticleInterface(os->obj);
	if (obj) {
		deflect.obj  = (USpawnDeflObject*)GetWSMObject(t);
		deflect.node = nodeRef;
		deflect.obj->tmValid.SetEmpty();		
		deflect.obj->mValid.SetEmpty();
		deflect.badmesh=(deflect.obj->custnode==NULL);
		if (t<=deflect.obj->t) deflect.obj->lastrnd=12345;
		deflect.obj->t=t;
/*		if (deflect.obj->dmesh) delete deflect.obj->dmesh;
		deflect.obj->dmesh=NULL;
		deflect.obj->vnorms.ZeroCount();deflect.obj->vnorms.Shrink();
		deflect.obj->fnorms.ZeroCount();deflect.obj->fnorms.Shrink();
		deflect.obj->nv=0;deflect.obj->nf=0;
		*/
		deflect.obj->dvel=Zero;
		obj->ApplyCollisionObject(&deflect);
		}
	}
 static TriObject *IsUseable(Object *pobj,TimeValue t)
{ 
  if (pobj->SuperClassID()==GEOMOBJECT_CLASS_ID)
  {	if (pobj->IsSubClassOf(triObjectClassID)) 
      return (TriObject*)pobj;
    else 
	{ if (pobj->CanConvertToType(triObjectClassID)) 
	  	return (TriObject*)pobj->ConvertToType(t,triObjectClassID);			
	}
  }
  return NULL;
}

#define EPSILON	0.0001f

void AddMesh(USpawnDeflObject *obj,TriObject *triOb,Matrix3 tm,BOOL nottop)
{ int lastv=obj->nv,lastf=obj->nf;
  obj->nv+=triOb->GetMesh().getNumVerts();
  obj->nf+=triOb->GetMesh().getNumFaces();
  if (!nottop)
    obj->dmesh->DeepCopy(&triOb->GetMesh(),PART_GEOM|PART_TOPO);
  else
  {obj->dmesh->setNumFaces(obj->nf,obj->dmesh->getNumFaces());
   obj->dmesh->setNumVerts(obj->nv,obj->dmesh->getNumVerts());
   tm=tm*obj->invtm;
   for (int vc=0;vc<triOb->GetMesh().getNumFaces();vc++)
   { obj->dmesh->faces[lastf]=triOb->GetMesh().faces[vc];
     for (int vs=0;vs<3;vs++) 
	   obj->dmesh->faces[lastf].v[vs]+=lastv;
     lastf++;}
  }
   for (int vc=0;vc<triOb->GetMesh().getNumVerts();vc++)
   { if (nottop) obj->dmesh->verts[lastv]=triOb->GetMesh().verts[vc]*tm;
	 else obj->dmesh->verts[lastv]=triOb->GetMesh().verts[vc];
     lastv++;}
}  

Object *USpawnDeflField::GetSWObject()
{ return obj;
}

BOOL USpawnDeflField::CheckCollision(TimeValue t,Point3 &inp,Point3 &vel,float dt,int index,float *ct,BOOL UpdatePastCollide)
{ 	if (badmesh) 
		return(0);

//watje need to get a pointer to out collider engine
//	ReferenceTarget *rt;
//	obj->pblock2->GetValue(uspflectrobj_collider,t,rt,obj->tmValid);
//	colm = (CollisionMesh *) rt;
// bayboro: the proper way to get a pointer to a our collider engine
// is to create CollisionSphere for each USpawnDeflField object
	if (colm==NULL)
		colm = (CollisionMesh*)CreateInstance(REF_MAKER_CLASS_ID, MESH_COLLISION_ID);
	if (!((obj->mValid.InInterval(t))&&(obj->tmValid.InInterval(t))))
	{
		if (colm) colm->SetNode(t,obj->custnode);
// this lets it set up some pre initialization data
		if (colm) colm->PreFrame(t,(TimeValue) dt);

		obj->tmValid=FOREVER;
		obj->tm=obj->custnode->GetObjectTM(t,&obj->tmValid);
		obj->tmNoTrans=obj->tm;
		obj->tmNoTrans.NoTrans();
//		obj->tm.NoScale();
//		obj->tmNoTrans.NoScale();
		obj->invtm=Inverse(obj->tm);
		obj->invtmNoTrans=Inverse(obj->tmNoTrans);

		obj->pblock2->GetValue(uspflectrobj_bounce,t,bounce,FOREVER);
		if (bounce < 0.0f) bounce = 0.0f;
		obj->pblock2->GetValue(uspflectrobj_bouncevar,t,bvar,FOREVER);
		obj->pblock2->GetValue(uspflectrobj_chaos,t,chaos,FOREVER);
		obj->pblock2->GetValue(uspflectrobj_velocity,t,vinher,FOREVER);
		obj->pblock2->GetValue(uspflectrobj_friction,t,friction,FOREVER);
		if (friction < 0.0f) friction = 0.0f;
		if (friction > 1.0f) friction = 1.0f;

//		bvar *= 0.01f;
//		chaos *= 0.01f;
//		vinher *= 0.01f;
//		friction *= 0.01f;

		if (obj->dmesh) delete obj->dmesh;
		obj->dmesh=new Mesh;obj->dmesh->setNumFaces(0);
		if (obj->vnorms) {delete[] obj->vnorms;obj->vnorms=NULL;}
		if (obj->fnorms) {delete[] obj->fnorms;obj->fnorms=NULL;}
		obj->nv=(obj->nf=0);
		Interval tmpValid=FOREVER;
		obj->ptm=obj->custnode->GetObjectTM(t-(TimeValue)dt,&tmpValid);
		obj->dvel=(Zero*obj->tm-Zero*obj->ptm)/dt;
		Object *pobj; 
		pobj = obj->custnode->EvalWorldState(t).obj;
		obj->mValid=pobj->ObjectValidity(t);
		TriObject *triOb=NULL;
		badmesh=TRUE;
		if ((triOb=IsUseable(pobj,t))!=NULL) AddMesh(obj,triOb,obj->tm,FALSE);
		if ((triOb)&&(triOb!=pobj)) triOb->DeleteThis();
		if (obj->custnode->IsGroupHead())
		{	 for (int ch=0;ch<obj->custnode->NumberOfChildren();ch++)
			{   INode *cnode=obj->custnode->GetChildNode(ch);
				if (cnode->IsGroupMember())
				{	pobj = cnode->EvalWorldState(t).obj;
					if ((triOb=IsUseable(pobj,t))!=NULL)
					{	Matrix3 tm=cnode->GetObjectTM(t,&obj->tmValid);
						obj->mValid=obj->mValid & pobj->ObjectValidity(t);
						AddMesh(obj,triOb,tm,TRUE);
						if ((triOb)&&(triOb!=pobj)) triOb->DeleteThis();
					}
				}
			}
		}
		if (obj->nf>0)
		{	obj->vnorms=new VNormal[obj->nv];
			obj->fnorms=new Point3[obj->nf];
			GetVFLst(obj->dmesh,obj->vnorms,obj->fnorms);
			badmesh=FALSE;
		}
 	}
	if (badmesh) 
		return 0;

// test for time limits
	TimeValue startt,endt;
//	obj->pblock->GetValue(PB_TIMEON,t,startt,FOREVER);
//	obj->pblock->GetValue(PB_TIMEOFF,t,endt,FOREVER);
	obj->pblock2->GetValue(uspflectrobj_timeon,t,startt,FOREVER);
	obj->pblock2->GetValue(uspflectrobj_timeoff,t,endt,FOREVER);
	if ((t<startt)||(t>endt))
	{	obj->lastrnd=rand();
		return FALSE;
	}

    srand(obj->lastrnd);
	float pvel,at,TempDP;
	Point3 NVrelL,Vrel,VrelL,pos,norm;

// test for reflection
	float affectsthisportion;
//	obj->pblock->GetValue(PB_AFFECTS,t,affectsthisportion,FOREVER);
	obj->pblock2->GetValue(uspflectrobj_affects,t,affectsthisportion,FOREVER);
//	affectsthisportion *= 0.01f;

if (randomFloat[index%500]<affectsthisportion)
{	
	if (!colm)
	{
		pos=inp*obj->invtm;
		Vrel=vel-obj->dvel;
		pvel=Length(Vrel);
		VrelL=Vrel*obj->invtmNoTrans;
		NVrelL=Normalize(VrelL);
		Ray ray;
		ray.dir=NVrelL;
		ray.p=pos;
		int kfound=RayIntersectP(ray,at,norm,obj->dmesh,obj->vnorms,obj->fnorms);
		if (!kfound)
		{	obj->lastrnd=rand();
			return FALSE;
		}
		Point3 id,iw=(id=pos+at*NVrelL)*obj->tm;
		float delta=Length(iw-inp);
		if (delta>dt*pvel)
		{	obj->lastrnd=rand();
			return FALSE;
		}

		float dti=delta/pvel;
		Point3 wnorm=norm*obj->tmNoTrans;
		Point3 Vt,c2,c1,Vreln=Vrel/pvel;
		Point3 Vdirbase=Normalize(Vreln);
		float q1=DotProd(-Vdirbase,wnorm);
		float theta=(float)acos(q1);
		if (theta>=HalfPI) 
			theta-=PI;
		if (theta<FLOAT_EPSILON)
			vel=-vel;
		else 
		{	c1=Normalize((-vel)^wnorm);
			c2=Normalize(wnorm^c1);
			vel=-vel;
			Vt=c2*DotProd(c2,vel);
			RotateOnePoint(&vel.x,&Zero.x,&c1.x,2*theta);
			vel=vel+friction*Vt;
		}
		vel=vel*bounce*(1-bvar*RND01());
		if (!FloatEQ0(chaos))
		{	theta=(HalfPI-theta)*chaos*RND01();
			Point3 d=Point3(RND11(),RND11(),RND11());
			Point3 c=Normalize(vel^d);
			RotateOnePoint(&vel.x,&Zero.x,&c.x,theta);
		}
		if (vinher>0.0f)
		{	Point3 dvel=obj->dvel*vinher; 
			vel=vel+friction*dvel+(1-friction)*DotProd(dvel,wnorm)*wnorm;
		}
		if (UpdatePastCollide)
		{	inp = iw + (dt-dti)*vel;
			if (ct) (*ct) = dt;
		}
		else
		{	inp = iw;
			if (ct) (*ct) = dti;
		}
		obj->lastrnd=rand();
		return TRUE;
	}
	else
	{
		Point3 hitpoint,bnorm,frict,inheritedVel;
		float at;
//just make the call to the engine to get the hit point and relevant data
		inheritedVel.x = vinher; // to pass information about inherited Velocity (for values < 1.0f)
		BOOL hit = colm->CheckCollision(t,inp,vel,dt,at,hitpoint,bnorm,frict,inheritedVel);

		if (!hit) 
		{	obj->lastrnd=rand();
			return FALSE;
		}

// Remove the part of dt we used to get to the collision point
		float holddt = dt;
		dt -= at;
		
// Reflect the velocity about the XY plane and attenuate with the bounce factor
// and add in inherited motion

		float rvariation = 1.0f;
		float rchaos = 1.0f;
		if (bvar != 0.0f)
		{
			rvariation =1.0f-( bvar * randomFloat[index%500]);
		}
		if (chaos != 0.0f)
		{
			rchaos =1.0f-( chaos * randomFloat[index%500]);
		}

		// Bayboro: New vector calculation scheme
		float normLen = bounce*rvariation*Length(bnorm);
		float fricLen = Length(frict);

		vel = bnorm*(bounce*rvariation) + frict*FricCoef(friction, normLen, fricLen, at);

		if (rchaos < 0.9999f) // randomize vector direction
		{	int indexOffset = index + 33 + abs(t - (int)ceil(dt));
			Point3 axis(randomFloat[(indexOffset)%500],randomFloat[(indexOffset+1)%500],randomFloat[(indexOffset+2)%500]);
			axis = axis - Point3(0.5f,0.5f,0.5f);
			float lsq = LengthSquared(bnorm);
			axis -= bnorm*DotProd(axis,bnorm)/lsq;
			float angle = PI*(1.0f-rchaos)*randomFloat[(index+57)%500];
			Quat quat = QFromAngAxis(angle, axis);
			Matrix3 tm;
			quat.MakeMatrix(tm);
			vel = vel*tm;
			// vector should not go beyond surface
			float dotProd = DotProd(vel,bnorm);
			if ( dotProd < 0.0f)
				vel -= 2*bnorm*dotProd/lsq;
		}

		vel += vinher*inheritedVel;

//		AddInheritVelocity(bnorm, frict, vel, inheritedVel,vinher, normLen, fricLen);
		// Bayboro: end of the new scheme

		// Bayboro: old scheme
//		vel = bnorm*(bounce*rvariation) + frict*(1.0f-(friction*rchaos)) ;
		inp = hitpoint;

		if (UpdatePastCollide)
		{	inp += vel * dt;  //uses up the rest of the time with the new velocity
			if (ct) 
				(*ct) = holddt;
		}
		else
		{	if (ct) 
				(*ct) = at;
		}
//		vel +=  (inheritedVel * vinher);
	
		return TRUE;
	}
}

// test for refraction
	float refracts;
//	obj->pblock->GetValue(PB_REFRACTS,t,refracts,FOREVER);
	obj->pblock2->GetValue(uspflectrobj_refracts,t,refracts,FOREVER);
//	refracts *= 0.01f;
	if (RND01()<refracts)
	{	pos=inp*obj->invtm;
		Vrel=vel-obj->dvel;
		pvel=Length(Vrel);
		VrelL=Vrel*obj->invtmNoTrans;
		NVrelL=Normalize(VrelL);
		Ray ray;
		ray.dir=NVrelL;
		ray.p=pos;
		int kfound=RayIntersectP(ray,at,norm,obj->dmesh,obj->vnorms,obj->fnorms);
		if (!kfound)
		{	obj->lastrnd=rand();
			return FALSE;
		}
		Point3 id,iw=(id=pos+at*NVrelL)*obj->tm;
		float delta=Length(iw-inp);
		if (delta>dt*pvel)
		{	obj->lastrnd=rand();
			return FALSE;
		}
// refraction code
		float refvol,refvar,decel,decelvar;
//		obj->pblock->GetValue(PB_REFRACTION,t,refvol,FOREVER);
//		obj->pblock->GetValue(PB_REFRACTVAR,t,refvar,FOREVER);
//		obj->pblock->GetValue(PB_DECEL,t,decel,FOREVER);
//		obj->pblock->GetValue(PB_DECELVAR,t,decelvar,FOREVER);
//		obj->pblock->GetValue(PB_INHERIT,t,vinher,FOREVER);
//		obj->pblock->GetValue(PB_FRICTION,t,friction,FOREVER);
		obj->pblock2->GetValue(uspflectrobj_refraction,t,refvol,FOREVER);
//		refvol *= 0.01f;
		obj->pblock2->GetValue(uspflectrobj_refractvar,t,refvar,FOREVER);
//		refvar *= 0.01f;
		obj->pblock2->GetValue(uspflectrobj_decel,t,decel,FOREVER);
		obj->pblock2->GetValue(uspflectrobj_decelvar,t,decelvar,FOREVER);
//		decelvar *= 0.01f;
	
		// shoot particle a little bit beyond the point of intersection
		// to avoid "stuck particles" (Bayboro 5/1/2001)
		iw = (pos + (at+1.0f)*NVrelL)*obj->tm;

		float dti=delta/pvel;
		Point3 wnorm=norm*obj->tmNoTrans;
		Point3 Vt,c2,c1,Vreln=Vrel/pvel;
		Point3 Vdirbase=Normalize(Vreln);
		float q1=DotProd(-Vdirbase,wnorm);
		float theta=(float)acos(q1);
		if (theta>=HalfPI) theta-=PI;
		Point3 Drag;
		c1=Normalize((-vel)^wnorm);
		c2=Normalize(wnorm^c1);
		Drag=friction*c2*DotProd(c2,-vel);
// reduce v by decel parameters
		vel*=decel*(1.0f-decelvar*RND01());
// rotate velocity vector
		float maxref,refangle,maxvarref;
	refangle=0.0f;
		if (!FloatEQ0(refvol))
		{	if (refvol>0.0f)
				maxref=-theta;
			else 
				maxref=HalfPI-theta;
			refangle=maxref*(float)fabs(refvol);
			float frefangle=(float)fabs(refangle);
			if (refvol>0.0f)
				maxvarref=HalfPI-theta-frefangle;
			else
				maxvarref=theta-frefangle;
			refangle+=maxvarref*RND11()*refvar;
			Point3 c,d;
			if (theta<0.01f)
			{	
				// Martell 4/14/01: Fix for order of ops bug.
				float ztmp=RND11(); float ytmp=RND11(); float xtmp=RND11();		
				d=Point3(xtmp,ytmp,ztmp);
				c=Normalize(vel^d);
			}
			else
			{	c=Normalize(wnorm^(-vel));
			}
			RotateOnePoint(vel,&Zero.x,&c.x,refangle);
			TempDP=DotProd(vel,wnorm);
			if (TempDP>0.0f) vel=vel-TempDP*wnorm;
		}
		float maxdiff,diffuse,diffvar,diffangle;
//		obj->pblock->GetValue(PB_DIFFUSION,t,diffuse,FOREVER);
//		obj->pblock->GetValue(PB_DIFFUSIONVAR,t,diffvar,FOREVER);
		obj->pblock2->GetValue(uspflectrobj_diffusion,t,diffuse,FOREVER);
//		diffuse *= 0.01f;
		obj->pblock2->GetValue(uspflectrobj_diffusionvar,t,diffvar,FOREVER);
//		diffvar *= 0.01f;
		maxdiff=HalfPI-theta-refangle;
		if (!FloatEQ0(diffuse))
		{	
			// Martell 4/14/01: Fix for order of ops bug.
			float ztmp=RND11(); float ytmp=RND11(); float xtmp=RND11();			
			Point3 d=Point3(xtmp,ytmp,ztmp);
			Point3 c=Normalize(vel^d);
			diffangle=0.5f*maxdiff*diffuse*(1.0f+RND11()*diffvar);
			RotateOnePoint(vel,&Zero.x,&c.x,diffangle);
			TempDP=DotProd(vel,wnorm);
			if (TempDP>0.0f) vel=vel-TempDP*wnorm;
		}
		if (vinher>0.0f)
		{	Point3 dvel=obj->dvel*vinher; 
			vel=vel+friction*dvel+(1.0f-friction)*DotProd(dvel,wnorm)*wnorm;
		}
		vel+=Drag;
		if (UpdatePastCollide)
		{	inp = iw + (dt-dti)*vel;
			if (ct) (*ct) = dt;
		}
		else
		{	inp = iw;
			if (ct) (*ct) = dti;
		}
		obj->lastrnd=rand();
		return TRUE;
	}

// test for spawning only
	float spawnonly;
//	obj->pblock->GetValue(PB_SPAWN,t,spawnonly,FOREVER);
	obj->pblock2->GetValue(uspflectrobj_spawn,t,spawnonly,FOREVER);
//	spawnonly *= 0.01f;
	if (RND01()<spawnonly)
	{	pos=inp*obj->invtm;
		Vrel=vel-obj->dvel;
		pvel=Length(Vrel);
		VrelL=Vrel*obj->invtmNoTrans;
		NVrelL=Normalize(VrelL);
		Ray ray;
		ray.dir=NVrelL;
		ray.p=pos;
		int kfound=RayIntersectP(ray,at,norm,obj->dmesh,obj->vnorms,obj->fnorms);
		if (!kfound)
		{	obj->lastrnd=rand();
			return FALSE;
		}
		Point3 iw=(pos+at*NVrelL)*obj->tm;
		float delta=Length(iw-inp);
		if (delta>dt*pvel)
		{	obj->lastrnd=rand();
			return FALSE;
		}
		float passvel,passvelvar;
		float dti=delta/pvel;
//		obj->pblock->GetValue(PB_PASSVEL,t,passvel,FOREVER);
//		obj->pblock->GetValue(PB_PASSVELVAR,t,passvelvar,FOREVER);
		obj->pblock2->GetValue(uspflectrobj_passvel,t,passvel,FOREVER);
		obj->pblock2->GetValue(uspflectrobj_passvelvar,t,passvelvar,FOREVER);
//		passvelvar *= 0.01f;

		// shoot particle a little bit beyond the point of intersection
		// to avoid "stuck particles" (Bayboro 5/1/2001)
		iw = (pos + (at+1.0f)*NVrelL)*obj->tm;

		vel *= passvel*(1.0f+passvelvar*RND11());
		if (UpdatePastCollide)
		{	inp = iw + (dt-dti)*vel;
			if (ct) (*ct) = dt;
		}
		else
		{	inp = iw;
			if (ct) (*ct) = dti;
		}
		return TRUE;		
	}

// nothing happens
	obj->lastrnd=rand();
	return FALSE;
}

RefTargetHandle USpawnDeflObject::GetReference(int i)
{	switch(i) 
	{
//		case PBLK: return(RefTargetHandle)pblock2;
		case PBLK: return SimpleWSMObject2::GetReference(i);
		case CUSTNODE: return (RefTargetHandle)custnode;
		default: return NULL;
	}
}

void USpawnDeflObject::SetReference(int i, RefTargetHandle rtarg) 
{ 
	switch(i) 
	{
//		case PBLK: pblock2=(IParamBlock2*)rtarg; return;
		case PBLK: SimpleWSMObject2::SetReference(i, rtarg); return;
		case CUSTNODE: custnode = (INode *)rtarg; return;
	}
}

RefResult USpawnDeflObject::NotifyRefChanged( 
		Interval changeInt,
		RefTargetHandle hTarget, 
		PartID& partID, 
		RefMessage message )
	{				
	switch (message) {		
		case REFMSG_TARGET_DELETED:	
			{ if (hTarget==custnode) custnode=NULL;
			}
			break;
		case REFMSG_NODE_NAMECHANGE:
			{ if (hTarget==custnode) 
			  { custname = TSTR(custnode->GetName());
			    ShowName();
				}
			  break;
			}
		default: SimpleWSMObject::NotifyRefChanged(changeInt,hTarget,partID,message);
		}
	return REF_SUCCEED;
	}

#define USPDEF_CUSTNAME_CHUNK	0x0100

IOResult USpawnDeflObject::Save(ISave *isave)
	{
	isave->BeginChunk(USPDEF_CUSTNAME_CHUNK);		
	isave->WriteWString(custname);
	isave->EndChunk();
	return IO_OK;
	}

class USpawnflectObjectLoad : public PostLoadCallback 
{
	public:
		USpawnDeflObject *n;
		USpawnflectObjectLoad(USpawnDeflObject *ns) {n = ns;}
		void proc(ILoad *iload) 
		{  
			ReferenceTarget *rt;
			Interval iv;
			n->pblock2->GetValue(uspflectrobj_collider,0,rt,iv);
			if (rt == NULL)
			{
				CollisionMesh *colm = (CollisionMesh*)CreateInstance(REF_MAKER_CLASS_ID, MESH_COLLISION_ID);
				if (colm)
					n->pblock2->SetValue(uspflectrobj_collider,0,(ReferenceTarget*)colm);
			}
			delete this; 
		} 
};

class USpawnDeflPostLoadCallback : public PostLoadCallback 
	{
	public:

//		ParamBlockPLCB *cb;
//		USpawnDeflPostLoadCallback(ParamBlockPLCB *c) {cb=c;}

		ParamBlock2PLCB *cb;
		USpawnDeflPostLoadCallback(ParamBlock2PLCB *c) {cb=c;}

		void proc(ILoad *iload) 
		{
			DWORD oldVer = ((USpawnDeflObject*)(cb->targ))->pblock2->GetVersion();
			ReferenceTarget *targ = cb->targ;
			cb->proc(iload);
			if (oldVer<1) 
			{	
// 				((USpawnDeflObject*)targ)->pblock->SetValue(PB_SPAWN,0,1.0f);
//				((USpawnDeflObject*)targ)->pblock->SetValue(PB_PASSVEL,0,1.0f);
//				((USpawnDeflObject*)targ)->pblock->SetValue(PB_PASSVELVAR,0,0);
 				((USpawnDeflObject*)targ)->pblock2->SetValue(uspflectrobj_spawn,0,1.0f);
 				((USpawnDeflObject*)targ)->pblock2->SetValue(uspflectrobj_passvel,0,1.0f);
 				((USpawnDeflObject*)targ)->pblock2->SetValue(uspflectrobj_passvelvar,0,0);
			}
			delete this;
		}
	};

IOResult USpawnDeflObject::Load(ILoad *iload)
{
	IOResult res = IO_OK;
	
	iload->RegisterPostLoadCallback(
		new ParamBlock2PLCB(usversions,NUM_OLDVERSIONS,&uspflector_param_blk,this,PBLOCK_REF_NO));
	iload->RegisterPostLoadCallback(new USpawnflectObjectLoad(this));

	custname = TSTR(_T(" "));

	while (IO_OK==(res=iload->OpenChunk())) 
	{
		switch (iload->CurChunkID()) 
		{
			case USPDEF_CUSTNAME_CHUNK: 
			{
				TCHAR *buf;
				res = iload->ReadWStringChunk(&buf);
				custname = TSTR(buf);
				break;
			}
		}
		iload->CloseChunk();
		if (res != IO_OK)  
			return res;
	}
	return IO_OK;
}

CollisionObject *USpawnDeflObject::GetCollisionObject(INode *node)
{
	USpawnDeflField *gf = new USpawnDeflField;	
	gf->obj  = this;
	gf->node = node;
	gf->obj->tmValid.SetEmpty();
	gf->obj->mValid.SetEmpty();
	gf->badmesh=(gf->obj->custnode==NULL);
	gf->obj->dvel=Zero;
	return gf;
}
