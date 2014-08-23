/**********************************************************************
 *<
	FILE: sspflect.cpp

	DESCRIPTION: Enhanced Deflector

	CREATED BY: Eric Peterson from Audrey Peterson's sspflector code

    PB2 ECP 3/19/00

	HISTORY: 7/97

 **********************************************************************/
#include "sflectr.h"
#include "iparamm2.h"
#include "simpmod.h"
#include "simpobj.h"
#include "texutil.h"
#include "icollision.h"

static Class_ID SSPAWNDEFL_CLASS_ID(0x656107ca, 0x1f284a6f);
static Class_ID SSPAWNDEFLMOD_CLASS_ID(0x72a61178, 0x21b407d9);

class SSpawnDeflObject : public SimpleWSMObject2 {	
	public:		
//		static IParamMap *pmapParam;
		static IObjParam *ip;
		static HWND hSot;
					
		int lastrnd;
		TimeValue t;
		SSpawnDeflObject();
		~SSpawnDeflObject();
		BOOL SupportsDynamics() {return FALSE;}

		// From Animatable		
		void DeleteThis() {delete this;}		
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);		
		void MapKeys(TimeMap *map,DWORD flags);
		Class_ID ClassID() {return SSPAWNDEFL_CLASS_ID;}		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_EP_SSPAWNDEFLECTOR_OBJECT);}
				
		// from object		
		CreateMouseCallBack* GetCreateMouseCallBack();		
		
		// From SimpleWSMObject		
		void InvalidateUI();		
		void BuildMesh(TimeValue t);
		ParamDimension *GetParameterDim(int pbIndex);
//		TSTR GetParameterName(int pbIndex);		
		
		// From WSMObject
		Modifier *CreateWSMMod(INode *node);		

		// Direct paramblock access
		int	NumParamBlocks() { return 1; }
		IParamBlock2* GetParamBlock(int i) { return pblock2; }
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock2->ID() == id) ? pblock2 : NULL; }

		// from ref
		IOResult Load(ILoad *iload);
		CollisionObject *GetCollisionObject(INode *node);
	};

//--- ClassDescriptor and class vars ---------------------------------

IObjParam *SSpawnDeflObject::ip        = NULL;
//IParamMap *SSpawnDeflObject::pmapParam = NULL;
HWND       SSpawnDeflObject::hSot      = NULL;

class SSpawnDeflClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) { return new SSpawnDeflObject;}
	const TCHAR *	ClassName() {return GetString(IDS_EP_SSPAWNDEFLECTOR);}
	SClass_ID		SuperClassID() {return WSM_OBJECT_CLASS_ID; }
	Class_ID		ClassID() {return SSPAWNDEFL_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_EP_SW_DEFLECTORS);}

	// Hardwired name, used by MAX Script as unique identifier
	const TCHAR*	InternalName()				{ return _T("SOmniFlect"); }
	HINSTANCE		HInstance()					{ return hInstance; }
	};

static SSpawnDeflClassDesc SSpawnDeflDesc;
ClassDesc* GetSSpawnDeflObjDesc() {return &SSpawnDeflDesc;}

//--- DeflectMod -----------------------------------------------------

class SSpawnDeflField : public CollisionObject {
	public:		
//watje
		SSpawnDeflField()
		{
			srand(9376435);
			for (int i =0;i < 500; i++)
			{
				randomFloat[i] = (float)rand()/(float)RAND_MAX;
			}
			cols=NULL;
		}

		~SSpawnDeflField()
		{
			if (cols) cols->DeleteThis();
			cols=NULL;
		}

		void DeleteThis() 
			{	 
			if (cols) cols->DeleteThis();
			cols=NULL;
			delete this;

			}


//watje moved these here since they don't need to be accessed for every particle
		float radius,chaos,bounce,bvar,vinher,friction;
//using a table of random floats now
		float randomFloat[500];

		SSpawnDeflObject *obj;
		INode *node;
		Matrix3 tm, invtm,tp;
		Interval tmValid;
		Point3 Vc,Vcp;
		BOOL CheckCollision(TimeValue t,Point3 &pos, Point3 &vel, float dt, int index,float *ct,BOOL UpdatePastCollide);
		Object *GetSWObject();
		void SetRandSeed(int seed) { if (obj != NULL) obj->lastrnd = seed; }
		CollisionSphere *cols;
	};

class SSpawnDeflMod : public SimpleWSMMod {
	public:				
		SSpawnDeflField deflect;

		SSpawnDeflMod() {}
		SSpawnDeflMod(INode *node,SSpawnDeflObject *obj);		

		// From Animatable
		void GetClassName(TSTR& s) {s= GetString(IDS_EP_SSPAWNDEFLECTORMOD);}
		SClass_ID SuperClassID() {return WSM_CLASS_ID;}
		void DeleteThis() {delete this;}
		Class_ID ClassID() { return SSPAWNDEFLMOD_CLASS_ID;}
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_EP_SSPAWNDEFLECTORBINDING);}

		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);

		// From SimpleWSMMod		
		Interval GetValidity(TimeValue t);		
		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);
	};

//--- ClassDescriptor and class vars ---------------------------------

class SSpawnDeflModClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 0;}
	void *			Create(BOOL loading = FALSE) { return new SSpawnDeflMod;}
	const TCHAR *	ClassName() {return GetString(IDS_EP_SSPAWNDEFLECTORMOD);}
	SClass_ID		SuperClassID() {return WSM_CLASS_ID; }
	Class_ID		ClassID() {return SSPAWNDEFLMOD_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
	};

static SSpawnDeflModClassDesc SSpawnDeflModDesc;
ClassDesc* GetSSpawnDeflModDesc() {return &SSpawnDeflModDesc;}

enum 
{	sspflectrobj_params, 
}; 

enum 
{
	sspflectrobj_timeon,
	sspflectrobj_timeoff,
	sspflectrobj_affects,
	sspflectrobj_bounce, 
	sspflectrobj_bouncevar,
	sspflectrobj_chaos,
	sspflectrobj_velocity,
	sspflectrobj_refracts,
	sspflectrobj_decel,
	sspflectrobj_decelvar,
	sspflectrobj_refraction,
	sspflectrobj_refractvar,
	sspflectrobj_diffusion,
	sspflectrobj_diffusionvar,
	sspflectrobj_radius,
	sspflectrobj_spawn,
	sspflectrobj_passvel,
	sspflectrobj_passvelvar,
	sspflectrobj_friction,
	sspflectrobj_collider
};

#define PBLOCK_REF_NO	0

static ParamBlockDesc2 sspflector_param_blk 
(	sspflectrobj_params, 
	_T("SSpawnflectorParameters"),  
	0, 
	&SSpawnDeflDesc, 
	P_AUTO_CONSTRUCT + P_AUTO_UI, 
	PBLOCK_REF_NO, 

	//rollout
	IDD_AP_SSPAWNDEFL, IDS_EP_PARAMETERS, 0, 0, NULL, 

	// params
	sspflectrobj_timeon,	_T("timeOn"),		TYPE_TIMEVALUE,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_TIMEON,
		p_default,			1,
		p_range,			-999999999, 999999999, 
		p_ui,				TYPE_SPINNER, EDITTYPE_INT,		IDC_EP_TIMEON,				IDC_EP_TIMEONSPIN,	0.1f,
		end,

	sspflectrobj_timeoff,	_T("timeOff"),		TYPE_TIMEVALUE,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_TIMEOFF,
		p_default,			1,
		p_range,			-999999999, 999999999, 
		p_ui,				TYPE_SPINNER, EDITTYPE_INT,		IDC_EP_TIMEOFF,				IDC_EP_TIMEOFFSPIN, 0.1f,
		end,

	sspflectrobj_affects,	_T("affects"),	TYPE_PCNT_FRAC,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_AFFECTS,
		p_default,			1.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_AFFECTS,				IDC_EP_AFFECTSSPIN, 1.0f,
		end,

	sspflectrobj_bounce,	_T("bounce"),		TYPE_FLOAT,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_BOUNCE,
		p_default,			1.0f,
		p_range,			0.0f,65535.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_BOUNCE,				IDC_EP_BOUNCESPIN, 0.01f,
		end,

	sspflectrobj_bouncevar, _T("bounceVar"),	TYPE_PCNT_FRAC,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_BOUNCEVAR,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_BOUNCEVAR,			IDC_EP_BOUNCEVARSPIN, 1.0f,
		end,

	sspflectrobj_chaos,		_T("chaos"),		TYPE_PCNT_FRAC,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_CHAOS,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_CHAOS,				IDC_EP_CHAOSSPIN,		1.0f,
		end,

	sspflectrobj_velocity,	_T("inheritVelocity"),	TYPE_FLOAT,	P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_INHERIT,
		p_default,			1.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_INHERIT,				IDC_EP_INHERITSPIN,		SPIN_AUTOSCALE,
		end,

	sspflectrobj_refracts,	_T("refracts"),		TYPE_PCNT_FRAC,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_REFRACTS,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_REFRACTS,			IDC_EP_REFRACTSSPIN,	1.0f,
		end,

	sspflectrobj_decel,		_T("deceleration"),	TYPE_FLOAT,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_AP_DECEL,
		p_default,			0.0f,
		p_range,			0.0f,65000.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_DECEL,				IDC_EP_DECELSPIN,			1.0f,
		end,

	sspflectrobj_decelvar,	_T("decelVar"),	TYPE_PCNT_FRAC,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_AP_DECELVAR,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_DECELVAR,			IDC_EP_DECELVARSPIN,		1.0f,
		end,

	sspflectrobj_refraction,_T("refraction"),	TYPE_PCNT_FRAC,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_REFRACTION,
		p_default,			0.0f,
		p_range,			-100.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_REFRACTION,			IDC_EP_REFRACTIONSPIN,		1.0f,
		end,

	sspflectrobj_refractvar,_T("refractionVar"),TYPE_PCNT_FRAC,	P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_REFRACTVAR,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_REFRACTVAR,			IDC_EP_REFRACTVARSPIN,		1.0f,
		end,

	sspflectrobj_diffusion,	_T("diffusion"),	TYPE_PCNT_FRAC,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_DIFFUSION,
		p_default,			0.0f,
		p_range,			0.0f,100.0f,	
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_DIFFUSION,			IDC_EP_DIFFUSIONSPIN,		1.0f,
		end,

	sspflectrobj_diffusionvar,	_T("diffusionVar"),TYPE_PCNT_FRAC,	P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_DIFFUSIONVAR,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_DIFFUSIONVAR,		IDC_EP_DIFFUSIONVARSPIN,	1.0f,
		end,

	sspflectrobj_radius,	_T("radius"),		TYPE_FLOAT,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_RADIUS,
		p_default,			0.0f,
		p_range,			0.0f,9999999.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_UNIVERSE,	IDC_EP_ICONSIZE,			IDC_EP_ICONSIZESPIN,	SPIN_AUTOSCALE,
		end,

	sspflectrobj_spawn,		_T("spawn"),		TYPE_PCNT_FRAC,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_AP_SPAWN,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_SPAWNSONLY,			IDC_EP_SPAWNSONLYSPIN,		1.0f,
		end,

	sspflectrobj_passvel,	_T("passVelocity"),TYPE_FLOAT,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_PASSVEL,
		p_default,			0.0f,
		p_range,			0.0f, 9999999.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_SPAWNONLYDECEL,		IDC_EP_SPAWNONLYDECELSPIN,	SPIN_AUTOSCALE,
		end,

	sspflectrobj_passvelvar,_T("passVelocityVar"),	TYPE_PCNT_FRAC,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_PASSVELVAR,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_SPAWNSONLYDECELVAR,	IDC_EP_SPAWNSONLYDECELVARSPIN, 1.0f,
		end,

	sspflectrobj_friction,	_T("friction"),		TYPE_PCNT_FRAC,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_FRICTION,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_FRICTION,			IDC_EP_FRICTIONSPIN,	1.0f,
		end,

//watje ref to hold the collision engine
	sspflectrobj_collider,  _T(""),		TYPE_REFTARG, 	0,0, 	//IDS_EP_FRICTION, 
		end, 
	end
);

/*
//--- SphereFlectorObject Parameter map/block descriptors ------------------

#define PB_TIMEON		0
#define PB_TIMEOFF		1
#define PB_AFFECTS		2
#define PB_BOUNCE		3
#define PB_BOUNCEVAR	4
#define PB_CHAOS		5
#define PB_INHERIT		6
#define PB_REFRACTS		7
#define PB_DECEL		8
#define PB_DECELVAR		9
#define PB_REFRACTION	10
#define PB_REFRACTVAR	11
#define PB_DIFFUSION	12
#define PB_DIFFUSIONVAR	13
#define PB_RADIUS		14
#define PB_SPAWN		15
#define PB_PASSVEL		16
#define PB_PASSVELVAR	17

static ParamUIDesc descSSpawnDeflParam[] = {
	// Start Time
	ParamUIDesc(
		PB_TIMEON,
		EDITTYPE_TIME,
		IDC_EP_TIMEON,IDC_EP_TIMEONSPIN,
		-999999999.0f, 999999999.0f,
		10.0f),
	
	// Stop Time
	ParamUIDesc(
		PB_TIMEOFF,
		EDITTYPE_TIME,
		IDC_EP_TIMEOFF,IDC_EP_TIMEOFFSPIN,
		-999999999.0f, 999999999.0f,
		10.0f),
	
	// Affects
	ParamUIDesc(
		PB_AFFECTS,
		EDITTYPE_FLOAT,
		IDC_EP_AFFECTS,IDC_EP_AFFECTSSPIN,
		0.0f,100.0f,
		1.0f,
		stdPercentDim),
		
	// Bounce
	ParamUIDesc(
		PB_BOUNCE,
		EDITTYPE_FLOAT,
		IDC_EP_BOUNCE,IDC_EP_BOUNCESPIN,
		0.0f, 9999999.0f,
		SPIN_AUTOSCALE),
	
	// BounceVar
	ParamUIDesc(
		PB_BOUNCEVAR,
		EDITTYPE_FLOAT,
		IDC_EP_BOUNCEVAR,IDC_EP_BOUNCEVARSPIN,
		0.0f, 100.0f,
		1.0f,
		stdPercentDim),
	
	// Chaos
	ParamUIDesc(
		PB_CHAOS,
		EDITTYPE_FLOAT,
		IDC_EP_CHAOS,IDC_EP_CHAOSSPIN,
		0.0f, 100.0f,
		1.0f,
		stdPercentDim),
	
	// Inherit
	ParamUIDesc(
		PB_INHERIT,
		EDITTYPE_FLOAT,
		IDC_EP_INHERIT,IDC_EP_INHERITSPIN,
		0.0f, 9999999.0f,
		SPIN_AUTOSCALE),

	// Refracts Percentage
	ParamUIDesc(
		PB_REFRACTS,
		EDITTYPE_FLOAT,
		IDC_EP_REFRACTS,IDC_EP_REFRACTSSPIN,
		0.0f, 100.0f,
		1.0f,
		stdPercentDim),

	// Refraction Decel
	ParamUIDesc(
		PB_DECEL,
		EDITTYPE_FLOAT,
		IDC_EP_DECEL,IDC_EP_DECELSPIN,
		0.0f, 9999999.0f,
		SPIN_AUTOSCALE),
	
	// Refraction Decel Var
	ParamUIDesc(
		PB_DECELVAR,
		EDITTYPE_FLOAT,
		IDC_EP_DECELVAR,IDC_EP_DECELVARSPIN,
		0.0f, 100.0f,
		1.0f,
		stdPercentDim),

	// Refraction
	ParamUIDesc(
		PB_REFRACTION,
		EDITTYPE_FLOAT,
		IDC_EP_REFRACTION,IDC_EP_REFRACTIONSPIN,
		-100.0f, 100.0f,
		1.0f,
		stdPercentDim),

	// Refraction Var
	ParamUIDesc(
		PB_REFRACTVAR,
		EDITTYPE_FLOAT,
		IDC_EP_REFRACTVAR,IDC_EP_REFRACTVARSPIN,
		0.0f, 100.0f,
		1.0f,
		stdPercentDim),

	// Diffusion
	ParamUIDesc(
		PB_DIFFUSION,
		EDITTYPE_FLOAT,
		IDC_EP_DIFFUSION,IDC_EP_DIFFUSIONSPIN,
		0.0f, 100.0f,
		1.0f,
		stdPercentDim),

	// Diffusion Var
	ParamUIDesc(
		PB_DIFFUSIONVAR,
		EDITTYPE_FLOAT,
		IDC_EP_DIFFUSIONVAR,IDC_EP_DIFFUSIONVARSPIN,
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

	// Radius
	ParamUIDesc(
		PB_RADIUS,
		EDITTYPE_UNIVERSE,
		IDC_EP_ICONSIZE,IDC_EP_ICONSIZESPIN,
		0.0f, 9999999.0f,
		SPIN_AUTOSCALE),

	};

*/
//#define PARAMDESC_LENGTH	19

ParamBlockDescID SSpawnDefldescVer0[] = {
	{ TYPE_INT, NULL, FALSE, 0 },	
	{ TYPE_INT, NULL, TRUE, 1 },	
	{ TYPE_FLOAT, NULL, TRUE, 2 },	
	{ TYPE_FLOAT, NULL, TRUE, 3 },
	{ TYPE_FLOAT, NULL, TRUE, 4 },
	{ TYPE_FLOAT, NULL, TRUE, 5 },
	{ TYPE_FLOAT, NULL, TRUE, 6 },
	{ TYPE_FLOAT, NULL, TRUE, 7 },
	{ TYPE_FLOAT, NULL, TRUE, 8 },
	{ TYPE_FLOAT, NULL, TRUE, 9 },
	{ TYPE_FLOAT, NULL, TRUE, 10 },
	{ TYPE_FLOAT, NULL, TRUE, 11 },
	{ TYPE_FLOAT, NULL, TRUE, 12 },
	{ TYPE_FLOAT, NULL, TRUE, 13 },
	{ TYPE_FLOAT, NULL, TRUE, 14 }};
	
ParamBlockDescID SSpawnDefldescVer1[] = {
	{ TYPE_INT, NULL, FALSE, 0 },	
	{ TYPE_INT, NULL, TRUE, 1 },	
	{ TYPE_FLOAT, NULL, TRUE, 2 },	
	{ TYPE_FLOAT, NULL, TRUE, 3 },
	{ TYPE_FLOAT, NULL, TRUE, 4 },
	{ TYPE_FLOAT, NULL, TRUE, 5 },
	{ TYPE_FLOAT, NULL, TRUE, 6 },
	{ TYPE_FLOAT, NULL, TRUE, 7 },
	{ TYPE_FLOAT, NULL, TRUE, 8 },
	{ TYPE_FLOAT, NULL, TRUE, 9 },
	{ TYPE_FLOAT, NULL, TRUE, 10 },
	{ TYPE_FLOAT, NULL, TRUE, 11 },
	{ TYPE_FLOAT, NULL, TRUE, 12 },
	{ TYPE_FLOAT, NULL, TRUE, 13 },
	{ TYPE_FLOAT, NULL, TRUE, 14 },
	{ TYPE_FLOAT, NULL, TRUE, 15 },
	{ TYPE_FLOAT, NULL, TRUE, 16 },
	{ TYPE_FLOAT, NULL, TRUE, 17 },
};	

#define PBLOCK_LENGTH	19

static ParamVersionDesc ssversions[] = 
{
	ParamVersionDesc(SSpawnDefldescVer0,15,0),
	ParamVersionDesc(SSpawnDefldescVer1,18,1),
};

#define NUM_OLDVERSIONS	2
//#define CURRENT_VERSION	3

//static ParamVersionDesc scurVersion(SSpawnDefldescVer1,PBLOCK_LENGTH,CURRENT_VERSION);


//--- Deflect object methods -----------------------------------------

SSpawnDeflObject::SSpawnDeflObject()
{
//	MakeRefByID(FOREVER, 0, 
//		CreateParameterBlock(SSpawnDefldescVer1, PBLOCK_LENGTH, CURRENT_VERSION));
//	assert(pblock);	

	pblock2 = NULL;
	SSpawnDeflDesc.MakeAutoParamBlocks(this);
	assert(pblock2);

//	pblock->SetValue(PB_TIMEON,0,0);
//	pblock->SetValue(PB_TIMEOFF,0,100*GetTicksPerFrame());
//	pblock->SetValue(PB_AFFECTS,0,1.0f);
//	pblock->SetValue(PB_BOUNCE,0,1.0f);
//	pblock->SetValue(PB_BOUNCEVAR,0,0.0f);
//	pblock->SetValue(PB_CHAOS,0,0.0f);
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

	pblock2->SetValue(sspflectrobj_timeon,0,0);
	pblock2->SetValue(sspflectrobj_timeoff,0,100*GetTicksPerFrame());
	pblock2->SetValue(sspflectrobj_affects,0,1.0f);
	pblock2->SetValue(sspflectrobj_bounce,0,1.0f);
	pblock2->SetValue(sspflectrobj_bouncevar,0,0.0f);
	pblock2->SetValue(sspflectrobj_chaos,0,0.0f);
//	pblock2->SetValue(sspflectrobj_velocity,0,1.0f);
	pblock2->SetValue(sspflectrobj_refracts,0,1.0f);
	pblock2->SetValue(sspflectrobj_decel,0,1.0f);
	pblock2->SetValue(sspflectrobj_decelvar,0,0.0f);
	pblock2->SetValue(sspflectrobj_refraction,0,0.5f);
	pblock2->SetValue(sspflectrobj_refractvar,0,0.0f);
	pblock2->SetValue(sspflectrobj_diffusion,0,0.0f);
	pblock2->SetValue(sspflectrobj_diffusionvar,0,0.0f);
	pblock2->SetValue(sspflectrobj_spawn,0,1.0f);
	pblock2->SetValue(sspflectrobj_passvel,0,1.0f);
	pblock2->SetValue(sspflectrobj_passvelvar,0,0.0f);
	pblock2->SetValue(sspflectrobj_friction,0,0.0f);
	pblock2->SetValue(sspflectrobj_radius,0,0.0f);

	srand(lastrnd=12345);
	t=99999;
	macroRec->Disable();
//watje create a new ref to our collision engine
	CollisionSphere *cols = (CollisionSphere*)CreateInstance(REF_MAKER_CLASS_ID, SPHERICAL_COLLISION_ID);
	if (cols)
	{
		pblock2->SetValue(sspflectrobj_collider,0,(ReferenceTarget*)cols);
	}
	macroRec->Enable();
}

Modifier *SSpawnDeflObject::CreateWSMMod(INode *node)
	{
	return new SSpawnDeflMod(node,this);
	}

RefTargetHandle SSpawnDeflObject::Clone(RemapDir& remap) 
	{
	SSpawnDeflObject* newob = new SSpawnDeflObject();	
	newob->ReplaceReference(0,pblock->Clone(remap));
	BaseClone(this, newob, remap);
	return newob;
	}

void SSpawnDeflObject::BeginEditParams(
		IObjParam *ip,ULONG flags,Animatable *prev)
{
	if (!hSot)
		hSot = ip->AddRollupPage( 
			hInstance, 
			MAKEINTRESOURCE(IDD_SW_DESC_LEGACY),
			DefaultSOTProc,
			GetString(IDS_EP_TOP), 
			(LPARAM)ip,APPENDROLL_CLOSED);

	SimpleWSMObject::BeginEditParams(ip,flags,prev);
	this->ip = ip;

	SSpawnDeflDesc.BeginEditParams(ip, this, flags, prev);
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
			descSSpawnDeflParam,PARAMDESC_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_AP_SSPAWNDEFL),
			GetString(IDS_EP_PARAMETERS),
			0);
		}
*/
	}

void SSpawnDeflObject::EndEditParams(
		IObjParam *ip, ULONG flags,Animatable *next)
{		
	SimpleWSMObject::EndEditParams(ip,flags,next);
	this->ip = NULL;

	SSpawnDeflDesc.EndEditParams(ip, this, flags, next);

	if (flags&END_EDIT_REMOVEUI ) 
	{		
//		DestroyCPParamMap(pmapParam);
//		pmapParam = NULL;		
		if (hSot)
		{
			ip->DeleteRollupPage(hSot);
			hSot = NULL;
		}	
	}
}

void SSpawnDeflObject::MapKeys(TimeMap *map,DWORD flags)
{	Animatable::MapKeys(map,flags);
	TimeValue TempTime;
// remap values
//	pblock->GetValue(PB_TIMEON,0,TempTime,FOREVER);
	pblock2->GetValue(sspflectrobj_timeon,0,TempTime,FOREVER);
	TempTime=map->map(TempTime);
//	pblock->SetValue(PB_TIMEON,0,TempTime);
	pblock2->SetValue(sspflectrobj_timeon,0,TempTime);
//	pblock->GetValue(PB_TIMEOFF,0,TempTime,FOREVER);
	pblock2->GetValue(sspflectrobj_timeoff,0,TempTime,FOREVER);
	TempTime=map->map(TempTime);
//	pblock->SetValue(PB_TIMEOFF,0,TempTime);
	pblock2->SetValue(sspflectrobj_timeoff,0,TempTime);
}  

void SSpawnDeflObject::BuildMesh(TimeValue t)
{	ivalid = FOREVER;
	float length,r2,r3,r,r4;
//	pblock->GetValue(PB_RADIUS,t,length,ivalid);
	pblock2->GetValue(sspflectrobj_radius,t,length,ivalid);
	float u;
	#define NUM_SEGS	24
	 r=length;
	 r2=0.5f*r;
	 r3=0.25f*r2;
	 r4=0.71f*r3;

	mesh.setNumVerts(3*NUM_SEGS+21);
	mesh.setNumFaces(3*NUM_SEGS+10);

	for (int i=0; i<NUM_SEGS; i++)
	{	u=float(i)/float(NUM_SEGS) * TWOPI;
		mesh.setVert(i, Point3((float)cos(u) * length, (float)sin(u) * length, 0.0f));
		mesh.setVert(i+NUM_SEGS, Point3(0.0f, (float)cos(u) * length, (float)sin(u) * length));
		mesh.setVert(i+2*NUM_SEGS, Point3((float)cos(u) * length, 0.0f, (float)sin(u) * length));
	}
//	for (i=0; i<NUM_SEGS; i++)
//	{	u=float(i)/float(NUM_SEGS) * TWOPI;
//	}
//	for (i=0; i<NUM_SEGS; i++)
//	{	u=float(i)/float(NUM_SEGS) * TWOPI;
//	}		
	mesh.setVert(3*NUM_SEGS, Point3(0.0f, 0.0f, 0.0f));

	mesh.setVert(73,Point3(0.0f,0.0f  ,r      ));//a //juncture on sphere
	mesh.setVert(74,Point3(0.0f,0.0f  ,r+r2   ));//b //end third prong
	mesh.setVert(75,Point3(0.0f,-r2   ,r-r2   ));//c // end second prong
	mesh.setVert(76,Point3(0.0f, r2   ,r-r2   ));//d //end second prong
//	mesh.setVert(77,Point3(0.0f, r4   ,r+r2+r4));//b1 //lone arrow head
//	mesh.setVert(78,Point3(0.0f,-r4   ,r+r2+r4));//b2 //lone arrow head
	mesh.setVert(77,Point3(0.0f, r4   ,r+r4   ));//b1 //lone arrow head
	mesh.setVert(78,Point3(0.0f,-r4   ,r+r4   ));//b2 //lone arrow head
	mesh.setVert(79,Point3(0.0f,-r2   ,r-r2+r3));//c1
	mesh.setVert(80,Point3(0.0f,-r2+r3,r-r2   ));//c2
	mesh.setVert(81,Point3(0.0f, r2-r3,r-r2   ));//d1
	mesh.setVert(82,Point3(0.0f, r2   ,r-r2+r3));//d2
	
	mesh.setVert(83,Point3(0.0f,0.0f  ,-r      ));//a //juncture on sphere
	mesh.setVert(84,Point3(0.0f,0.0f  ,-r-r2   ));//b //end third prong
	mesh.setVert(85,Point3(0.0f,-r2   ,-r+r2   ));//c // end second prong
	mesh.setVert(86,Point3(0.0f, r2   ,-r+r2   ));//d //end second prong
//	mesh.setVert(87,Point3(0.0f, r4   ,-r-r2-r4));//b1 //lone arrow head
//	mesh.setVert(88,Point3(0.0f,-r4   ,-r-r2-r4));//b2 //lone arrow head
	mesh.setVert(87,Point3(0.0f, r4   ,-r-r4   ));//b1 //lone arrow head
	mesh.setVert(88,Point3(0.0f,-r4   ,-r-r4   ));//b2 //lone arrow head
	mesh.setVert(89,Point3(0.0f,-r2   ,-r+r2-r3));//c1
	mesh.setVert(90,Point3(0.0f,-r2+r3,-r+r2   ));//c2
	mesh.setVert(91,Point3(0.0f, r2-r3,-r+r2   ));//d1
	mesh.setVert(92,Point3(0.0f, r2   ,-r+r2-r3));//d2
	
	for (i=0; i<3*NUM_SEGS; i++)
	{	int i1 = i+1;
		if (i1%NUM_SEGS==0) i1 -= NUM_SEGS;
		mesh.faces[i].setEdgeVisFlags(1,0,0);
		mesh.faces[i].setSmGroup(0);
		mesh.faces[i].setVerts(i,i1,3*NUM_SEGS);
	}

	mesh.faces[72].setEdgeVisFlags(1,0,1);
	mesh.faces[72].setSmGroup(0);
	mesh.faces[72].setVerts(73,75,76);
	mesh.faces[73].setEdgeVisFlags(1,1,1);
	mesh.faces[73].setSmGroup(0);
	mesh.faces[73].setVerts(73,74,74);
	mesh.faces[74].setEdgeVisFlags(1,1,1);
	mesh.faces[74].setSmGroup(0);
//	mesh.faces[74].setVerts(74,78,77);
	mesh.faces[74].setVerts(73,78,77);
	mesh.faces[75].setEdgeVisFlags(1,1,1);
	mesh.faces[75].setSmGroup(0);
	mesh.faces[75].setVerts(75,79,80);
	mesh.faces[76].setEdgeVisFlags(1,1,1);
	mesh.faces[76].setSmGroup(0);
	mesh.faces[76].setVerts(76,81,82);

	mesh.faces[77].setEdgeVisFlags(1,0,1);
	mesh.faces[77].setSmGroup(0);
	mesh.faces[77].setVerts(83,85,86);
	mesh.faces[78].setEdgeVisFlags(1,0,1);
	mesh.faces[78].setSmGroup(0);
	mesh.faces[78].setVerts(83,84,84);
	mesh.faces[79].setEdgeVisFlags(1,1,1);
	mesh.faces[79].setSmGroup(0);
//	mesh.faces[79].setVerts(84,88,87);
	mesh.faces[79].setVerts(83,88,87);
	mesh.faces[80].setEdgeVisFlags(1,1,1);
	mesh.faces[80].setSmGroup(0);
	mesh.faces[80].setVerts(85,89,90);
	mesh.faces[81].setEdgeVisFlags(1,1,1);
	mesh.faces[81].setSmGroup(0);
	mesh.faces[81].setVerts(86,91,92);

	mesh.InvalidateGeomCache();
}

class SSpawnDeflObjCreateCallback : public CreateMouseCallBack {
	public:
		SSpawnDeflObject *ob;	
		Point3 p0, p1;
		IPoint2 sp0, sp1;
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
	};

int SSpawnDeflObjCreateCallback::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat)
{
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
	
	if (msg==MOUSE_POINT||msg==MOUSE_MOVE)
	{	switch(point)
		{	case 0:
				sp0 = m;
				#ifdef _3D_CREATE	
					p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				#else	
					p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				#endif
				mat.SetTrans(p0);
//				ob->pblock->SetValue(PB_RADIUS,0,0.01f);
				ob->pblock2->SetValue(sspflectrobj_radius,0,0.01f);
//				ob->pmapParam->Invalidate();
				sspflector_param_blk.InvalidateUI();
				break;
			case 1:
			{	sp1 = m;
				#ifdef _3D_CREATE	
					p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				#else	
					p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				#endif
//				ob->pblock->SetValue(PB_RADIUS,0,Length(p1-p0));
				ob->pblock2->SetValue(sspflectrobj_radius,0,Length(p1-p0));
//				ob->pmapParam->Invalidate();
				sspflector_param_blk.InvalidateUI();

				if (msg==MOUSE_POINT)
				{	if (Length(m-sp0)<3) 
						return CREATE_ABORT;
					else
						return CREATE_STOP;
				}
				break;
			}
		}
	}
	else
		if (msg == MOUSE_ABORT)	return CREATE_ABORT;
	return TRUE;
}

static SSpawnDeflObjCreateCallback SSpawnDeflCreateCB;

CreateMouseCallBack* SSpawnDeflObject::GetCreateMouseCallBack()
	{
	SSpawnDeflCreateCB.ob = this;
	return &SSpawnDeflCreateCB;
	}

void SSpawnDeflObject::InvalidateUI() 
{
	sspflector_param_blk.InvalidateUI(pblock2->LastNotifyParamID());
//	if (pmapParam) pmapParam->Invalidate();
}


ParamDimension *SSpawnDeflObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {		
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

		case sspflectrobj_timeon:			 
		case sspflectrobj_timeoff:			return stdTimeDim;
		case sspflectrobj_affects:		
		case sspflectrobj_bouncevar:		
		case sspflectrobj_chaos:			
		case sspflectrobj_refracts:		
		case sspflectrobj_decelvar:		
		case sspflectrobj_refraction:		
		case sspflectrobj_refractvar:		
		case sspflectrobj_diffusion:		
		case sspflectrobj_diffusionvar:	
		case sspflectrobj_spawn:			
		case sspflectrobj_passvelvar:		return stdPercentDim;
		default:							return defaultDim;
		}
	}

/*
TSTR SSpawnDeflObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {				
		case PB_TIMEON:			return GetString(IDS_EP_TIMEON);
		case PB_TIMEOFF:		return GetString(IDS_EP_TIMEOFF);
		case PB_AFFECTS:		return GetString(IDS_EP_AFFECTS);
		case PB_BOUNCE:			return GetString(IDS_EP_BOUNCE);
		case PB_BOUNCEVAR:		return GetString(IDS_EP_BOUNCEVAR);
		case PB_CHAOS:			return GetString(IDS_EP_CHAOS);
		case PB_INHERIT:		return GetString(IDS_EP_INHERIT);
		case PB_REFRACTS:		return GetString(IDS_EP_REFRACTS);
		case PB_DECEL:			return GetString(IDS_EP_PASSVEL);
		case PB_DECELVAR:		return GetString(IDS_EP_PASSVELVAR);
		case PB_REFRACTION:		return GetString(IDS_EP_REFRACTION);
		case PB_REFRACTVAR:		return GetString(IDS_EP_REFRACTVAR);
		case PB_DIFFUSION:		return GetString(IDS_EP_DIFFUSION);
		case PB_DIFFUSIONVAR:	return GetString(IDS_EP_DIFFUSIONVAR);
		case PB_RADIUS:			return GetString(IDS_EP_RADIUS);
		case PB_SPAWN:			return GetString(IDS_AP_SPAWN);
		case PB_PASSVEL:		return GetString(IDS_AP_PASSVEL);
		case PB_PASSVELVAR:		return GetString(IDS_AP_PASSVELVAR);
		default: 				return TSTR(_T(""));
		}
	}
*/

//--- DeflectMod methods -----------------------------------------------

SSpawnDeflMod::SSpawnDeflMod(INode *node,SSpawnDeflObject *obj)
	{	
	//MakeRefByID(FOREVER,SIMPWSMMOD_OBREF,obj);
	MakeRefByID(FOREVER,SIMPWSMMOD_NODEREF,node);	
	pblock = NULL;
	obRef = NULL;
	}

Interval SSpawnDeflMod::GetValidity(TimeValue t) 
	{
	if (obRef && nodeRef) {
		Interval valid = FOREVER;
		Matrix3 tm;
		float f;		
		SSpawnDeflObject *obj = (SSpawnDeflObject*)GetWSMObject(t);
//		obj->pblock->GetValue(PB_TIMEOFF,t,f,valid);
//		obj->pblock->GetValue(PB_AFFECTS,t,f,valid);
//		obj->pblock->GetValue(PB_BOUNCE,t,f,valid);
//		obj->pblock->GetValue(PB_BOUNCEVAR,t,f,valid);
//		obj->pblock->GetValue(PB_CHAOS,t,f,valid);
//		obj->pblock->GetValue(PB_INHERIT,t,f,valid);
//		obj->pblock->GetValue(PB_REFRACTS,t,f,valid);
//		obj->pblock->GetValue(PB_DECEL,t,f,valid);
//		obj->pblock->GetValue(PB_DECELVAR,t,f,valid);
//		obj->pblock->GetValue(PB_REFRACTION,t,f,valid);
//		obj->pblock->GetValue(PB_REFRACTVAR,t,f,valid);
//		obj->pblock->GetValue(PB_DIFFUSION,t,f,valid);
//		obj->pblock->GetValue(PB_DIFFUSIONVAR,t,f,valid);
//		obj->pblock->GetValue(PB_RADIUS,t,f,valid);
//		obj->pblock->GetValue(PB_SPAWN,t,f,valid);
//		obj->pblock->GetValue(PB_PASSVEL,t,f,valid);
//		obj->pblock->GetValue(PB_PASSVELVAR,t,f,valid);

		obj->pblock2->GetValue(sspflectrobj_timeoff,t,f,valid);
		obj->pblock2->GetValue(sspflectrobj_affects,t,f,valid);
		obj->pblock2->GetValue(sspflectrobj_bounce,t,f,valid);
		obj->pblock2->GetValue(sspflectrobj_bouncevar,t,f,valid);
		obj->pblock2->GetValue(sspflectrobj_chaos,t,f,valid);
		obj->pblock2->GetValue(sspflectrobj_velocity,t,f,valid);
		obj->pblock2->GetValue(sspflectrobj_refracts,t,f,valid);
		obj->pblock2->GetValue(sspflectrobj_decel,t,f,valid);
		obj->pblock2->GetValue(sspflectrobj_decelvar,t,f,valid);
		obj->pblock2->GetValue(sspflectrobj_refraction,t,f,valid);
		obj->pblock2->GetValue(sspflectrobj_refractvar,t,f,valid);
		obj->pblock2->GetValue(sspflectrobj_diffusion,t,f,valid);
		obj->pblock2->GetValue(sspflectrobj_diffusionvar,t,f,valid);
		obj->pblock2->GetValue(sspflectrobj_radius,t,f,valid);
		obj->pblock2->GetValue(sspflectrobj_spawn,t,f,valid);
		obj->pblock2->GetValue(sspflectrobj_passvel,t,f,valid);
		obj->pblock2->GetValue(sspflectrobj_passvelvar,t,f,valid);
		obj->pblock2->GetValue(sspflectrobj_friction,t,f,valid);
		tm=nodeRef->GetObjectTM(t,&valid);
		return valid;
	} else {
		return FOREVER;
		}
	}

class SSpawnDeflDeformer : public Deformer {
	public:		
		Point3 Map(int i, Point3 p) {return p;}
	};
static SSpawnDeflDeformer SSpawnDeflddeformer;

Deformer& SSpawnDeflMod::GetDeformer(
		TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
	{
	return SSpawnDeflddeformer;
	}

RefTargetHandle SSpawnDeflMod::Clone(RemapDir& remap) 
{	SSpawnDeflMod *newob = new SSpawnDeflMod(nodeRef,(SSpawnDeflObject*)obRef);	
	newob->SimpleWSMModClone(this);
	BaseClone(this, newob, remap);
	return newob;
}


void SSpawnDeflMod::ModifyObject(
		TimeValue t, ModContext &mc, ObjectState *os, INode *node)
	{
	ParticleObject *obj = GetParticleInterface(os->obj);
	if (obj) {
		deflect.obj  = (SSpawnDeflObject*)GetWSMObject(t);
		deflect.node = nodeRef;
		deflect.tmValid.SetEmpty();		
		if (t<=deflect.obj->t) deflect.obj->lastrnd=12345;
		deflect.obj->t=t;
		obj->ApplyCollisionObject(&deflect);
		}
	}

Object *SSpawnDeflField::GetSWObject()
{	return obj;
}

SSpawnDeflObject::~SSpawnDeflObject()
{
	DeleteAllRefsFromMe();
	if (pblock2)
	{
		ReferenceTarget *rt;
		pblock2->GetValue(sspflectrobj_collider,t,rt,FOREVER);
//		if (rt)
//			delete rt;
	}
}

BOOL SSpawnDeflField::CheckCollision(TimeValue t,Point3 &pos,Point3 &vel,float dt,int index,float *ct,BOOL UpdatePastCollide)
{
//watje need to get a pointer to out collider engine
//	ReferenceTarget *rt;
//	obj->pblock2->GetValue(sspflectrobj_collider,t,rt,tmValid);
//	cols = (CollisionSphere *) rt;
// bayboro: the proper way to get a pointer to a our collider engine
// is to create CollisionSphere for each SSpawnDeflField object
	if (cols==NULL)
		cols = (CollisionSphere*)CreateInstance(REF_MAKER_CLASS_ID, SPHERICAL_COLLISION_ID);
	
	if (!tmValid.InInterval(t))
	{	tmValid=FOREVER;
		tm = node->GetObjectTM(t,&tmValid);

//watje moved the get values so they would not get called on every particle
		obj->pblock2->GetValue(sspflectrobj_bounce,t,bounce,FOREVER);
//		if (bounce<0.001f) 
//			bounce+=0.001f;
		obj->pblock2->GetValue(sspflectrobj_bouncevar,t,bvar,FOREVER);
		obj->pblock2->GetValue(sspflectrobj_chaos,t,chaos,FOREVER);
		obj->pblock2->GetValue(sspflectrobj_friction,t,friction,FOREVER);
		obj->pblock2->GetValue(sspflectrobj_radius,t,radius,FOREVER);
		obj->pblock2->GetValue(sspflectrobj_velocity,t,vinher,FOREVER);
		if (friction < 0.0f) friction = 0.0f;
		if (friction > 1.0f) friction = 1.0f;
		if (bounce < 0.0f) bounce = 0.0f;

//		bvar *= 0.01f;
//		chaos *= 0.01f;
//		friction *= 0.01f;
//		vinher *= 0.01f;

//		radius*=0.5f;

//watje load up the engine
		if (cols)
		{	macroRec->Disable();
			// AnimateOff/On sequence to avoid "pseudo" animated radius for collision engine (Bayboro 6/15/01)
			int animateOn = Animating();
			if (animateOn) AnimateOff();
			Control* cont = obj->pblock2->GetController(sspflectrobj_radius);
			if (cont) cols->pblock->SetController(collisionsphere_radius, 0, cont, FALSE);
			else cols->SetRadius(t, radius);
			cols->SetNode(t,node);
			if (animateOn) AnimateOn();
// this lets it set up some pre initialization data
			cols->PreFrame(t,(TimeValue) dt);
			macroRec->Enable();
		}
		
		invtm = Inverse(tm);
		Interval tmpValid = FOREVER;
		tp = node->GetObjectTM(t,&tmpValid);
		Vc = Zero;
		Vcp = Zero*tp*invtm;
	}

	TimeValue startt,endt;
//	obj->pblock->GetValue(PB_TIMEON,t,startt,FOREVER);
//	obj->pblock->GetValue(PB_TIMEOFF,t,endt,FOREVER);
	obj->pblock2->GetValue(sspflectrobj_timeon,t,startt,FOREVER);
	obj->pblock2->GetValue(sspflectrobj_timeoff,t,endt,FOREVER);
	if ((t<startt)||(t>endt))
	{	obj->lastrnd=rand();
		return FALSE;
	}

    srand(obj->lastrnd);

// Main reflection / refraction loop starts here
	float rsquare,rplus,rminus,TempDP;
	Point3 p,vr,Vdt,Vrel,Vreln;

// test for reflection
	float affectsthisportion;
//	obj->pblock->GetValue(PB_AFFECTS,t,affectsthisportion,FOREVER);
	obj->pblock2->GetValue(sspflectrobj_affects,t,affectsthisportion,FOREVER);
//	affectsthisportion *= 0.01f;

if (randomFloat[index%500]<affectsthisportion) 
{	
	if (!cols)
	{
//		obj->pblock->GetValue(PB_BOUNCE,t,bounce,FOREVER);
//		obj->pblock->GetValue(PB_BOUNCEVAR,t,bvar,FOREVER);
//		obj->pblock->GetValue(PB_CHAOS,t,chaos,FOREVER);
//		obj->pblock->GetValue(PB_INHERIT,t,vinher,FOREVER);
		
//		obj->pblock->GetValue(PB_RADIUS,t,radius,FOREVER);

//		obj->pblock2->GetValue(sspflectrobj_bounce,t,bounce,FOREVER);
//		obj->pblock2->GetValue(sspflectrobj_bouncevar,t,bvar,FOREVER);
//		obj->pblock2->GetValue(sspflectrobj_chaos,t,chaos,FOREVER);
//		obj->pblock2->GetValue(sspflectrobj_velocity,t,vinher,FOREVER);
		
//		obj->pblock2->GetValue(sspflectrobj_radius,t,radius,FOREVER);
		p=pos*invtm; 
		vr=VectorTransform(invtm,vel);
		Vdt=(Vcp-Vc)/dt;
		Vrel=vr-Vdt;
		rsquare = radius*radius;
		rplus = radius;
		rminus = radius;
		Vreln = Normalize(Vrel);
		if (LengthSquared(p-Vc)>=(rminus*rminus)) //outside
		{	Point3 P1;
			P1=p+dt*Vrel; //second particle position
			if (!(LengthSquared(P1-Vc)<(rplus*rplus)))
			{	float Dist,Dist1;
				Dist=DotProd(Vreln,(Vc-p));
				if (Dist<0.0f)
				{	obj->lastrnd=rand();
					return FALSE;
				}
				Dist1=DotProd(-Vreln,(Vc-P1));
				if (Dist1<0.0f)
				{	obj->lastrnd=rand();
					return FALSE;
				}
				Point3 P10=P1-p,Pc=Vc-p;
				float gamma=(float)acos(DotProd(P10,Pc)/(Length(P10)*Length(Pc)));
				float Dist2=Length(Pc)*(float)cos(HalfPI-gamma);
				if (Dist2>radius)
				{	obj->lastrnd=rand();
					return FALSE;
				}
			}
			float A,B,C,omega,omega1,omegaend,a2,ptmp,c,d;
			A=LengthSquared(Vrel);
			B=2.0f*DotProd(p,Vrel)-2.0f*DotProd(Vrel,Vc);
			C=(ptmp=LengthSquared(p))+(c=LengthSquared(Vc))-(d=2.0f*DotProd(p,Vc))-rsquare;
			omegaend=B*B-4.0f*A*C;
			if (omegaend<0.0f) 
				omegaend=0.0f;
			else 
				omegaend = (float)sqrt(omegaend);
			a2=2.0f*A;
			omega1=(-B+omegaend)/a2;
			omega=(-B-omegaend)/a2;
			if (((omega1>0.0f)&&(omega1<omega))||((omega<0.0f)&&(omega1>omega))) omega=omega1;
			float fdt=1.1f*dt;
			if ((omega>fdt)||(omega<-fdt))
			{	obj->lastrnd=rand();
				return FALSE;
			}
// solve for collision params starts here
			Point3 XI,r,n;
			XI=p+omega*vr;
			Point3 Ci=Vc+omega*Vdt;
			r=Normalize(XI-Ci);
			float q1=DotProd(-Vreln,r);
			float theta=(float)acos(q1);
			if (theta>=HalfPI) theta-=PI;
			float v[4];
			if (theta<FLOAT_EPSILON)
				vr=-vr;
			else 
			{	n=Normalize((-Vreln)^r);
				vr=-vr;
				memcpy(v,vr,row3size);v[3]=1.0f;
				RotateOnePoint(v,&Zero.x,&n.x,2*theta);
				memcpy(vr,v,row3size);
			}
			vr=vr*bounce*(1-bvar*RND01());
			if (!FloatEQ0(chaos))
			{	
				theta=(HalfPI-theta)*chaos*RND01();
				// Martell 4/14/01: Fix for order of ops bug.
				float ztmp=RND11(); float ytmp=RND11(); float xtmp=RND11();
				Point3 d=Point3(xtmp,ytmp,ztmp);
				Point3 c=Normalize(vr^d);
				memcpy(v,vr,row3size);v[3]=1.0f;
				RotateOnePoint(v,&Zero.x,&c.x,theta);
				memcpy(vr,v,row3size);
			}
			if ((vinher>0.0f)&&(t>0)){vr=vr+DotProd(Vdt*vinher,r)*r;}
			pos=XI+(dt-omega)*vr;
		}
		else //inside
		{	Point3 P1;
			P1=p+dt*Vrel;
			if (LengthSquared(P1-Vc)<(rplus*rplus))
			{	obj->lastrnd=rand();
				return FALSE;
			}
			float A,B,C,omega,omega1,omegaend,a2,ptmp,c,d;
			A=LengthSquared(Vrel);
			B=2.0f*DotProd(p,Vrel)-2.0f*DotProd(Vrel,Vc);
			C=(ptmp=LengthSquared(p))+(c=LengthSquared(Vc))-(d=2.0f*DotProd(p,Vc))-rsquare;
			omegaend=B*B-4.0f*A*C;
			if (omegaend<0.0f) omegaend=0.0f;
			else omegaend=(float)sqrt(omegaend);
			a2=2.0f*A;
			omega1=(-B+omegaend)/a2;
			omega=(-B-omegaend)/a2;
			if (((omega1>0.0f)&&(omega1<omega))||((omega<0.0f)&&(omega1>omega))) omega=omega1;
			float fdt=1.1f*dt;
			if ((omega>fdt)||(omega<-fdt))
			{	obj->lastrnd=rand();
				return FALSE;
			}
			Point3 XI,r,n;
			XI=p+omega*vr;
			Point3 Ci=Vc+omega*Vdt;
			r=Normalize(XI-Ci);
			float q1=DotProd(Vreln,r);
			float theta=(float)acos(q1);
			float v[4];
			if (theta<FLOAT_EPSILON)
				vr=-vr;
			else 
			{	n=Normalize(Vreln^r);
				vr=-vr;
				memcpy(v,vr,row3size);v[3]=1.0f;
				RotateOnePoint(v,&Zero.x,&n.x,2*theta);
				memcpy(vr,v,row3size);
			}
			vr=vr*bounce*(1-bvar*RND01());
			if (!FloatEQ0(chaos))
			{	theta=(HalfPI-theta)*chaos*RND01();
				// Martell 4/14/01: Fix for order of ops bug.
				float ztmp=RND11(); float ytmp=RND11(); float xtmp=RND11();
				Point3 d=Point3(xtmp,ytmp,ztmp);
				Point3 c=Normalize(vr^d);
				memcpy(v,vr,row3size);v[3]=1.0f;
				RotateOnePoint(v,&Zero.x,&c.x,theta);
				memcpy(vr,v,row3size);
			}
			if ((vinher>0.0f)&&(t>0)){vr=vr+DotProd(Vdt*vinher,r)*r;}
			if (UpdatePastCollide)
			{	pos = XI + (dt-omega)*vr;
				if (ct) (*ct) = dt;
			}
			else
			{	pos = XI;
				if (ct) (*ct) = omega;
			}
		}
		pos = pos*tm;
		vel = VectorTransform(tm,vr);
		obj->lastrnd=rand();
		return TRUE;
	}
	else
	{
		Point3 hitpoint,bnorm,frict,inheritedVel;
		float at;
//just make the call to the engine to get the hit point and relevant data
		inheritedVel.x = vinher; // to pass information about inherited Velocity (for values < 1.0f)
		BOOL hit = cols->CheckCollision(t,pos,vel,dt,at,hitpoint,bnorm,frict,inheritedVel);

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
//		vel = bnorm*(bounce*rvariation) + frict*(1.0f-(friction*rchaos)) + (inheritedVel * vinher);
		
		pos = hitpoint;

		if (UpdatePastCollide)
		{	pos += vel * dt;  //uses up the rest of the time with the new velocity
				if (ct) (*ct) = holddt;
		}
		else
		{	if (ct) (*ct) = at;
		}

		return TRUE;
	}
}

// test for refraction
	float refracts;
//	obj->pblock->GetValue(PB_REFRACTS,t,refracts,FOREVER);
	obj->pblock2->GetValue(sspflectrobj_refracts,t,refracts,FOREVER);
//	refracts *= 0.01f;
	if (RND01()<refracts)
	{	float refvol,refvar,decel,decelvar;
//		obj->pblock->GetValue(PB_REFRACTION,t,refvol,FOREVER);
//		obj->pblock->GetValue(PB_REFRACTVAR,t,refvar,FOREVER);
//		obj->pblock->GetValue(PB_DECEL,t,decel,FOREVER);
//		obj->pblock->GetValue(PB_DECELVAR,t,decelvar,FOREVER);
//		obj->pblock->GetValue(PB_INHERIT,t,vinher,FOREVER);
//		obj->pblock->GetValue(PB_RADIUS,t,radius,FOREVER);
		obj->pblock2->GetValue(sspflectrobj_refraction,t,refvol,FOREVER);
//		refvol *= 0.01f;
		obj->pblock2->GetValue(sspflectrobj_refractvar,t,refvar,FOREVER);
//		refvar *= 0.01f;
		obj->pblock2->GetValue(sspflectrobj_decel,t,decel,FOREVER);
		obj->pblock2->GetValue(sspflectrobj_decelvar,t,decelvar,FOREVER);
//		decelvar *= 0.01f;
		obj->pblock2->GetValue(sspflectrobj_velocity,t,vinher,FOREVER);
//		vinher *= 0.01f;
		obj->pblock2->GetValue(sspflectrobj_radius,t,radius,FOREVER);
		p=pos*invtm; 
		vr=VectorTransform(invtm,vel);
		Vdt=(Vcp-Vc)/dt;
		Vrel=vr-Vdt;
		rsquare=radius*radius;
		rplus=radius;rminus=radius;
		Vreln=Normalize(Vrel);
		if (LengthSquared(p-Vc)>=(rminus*rminus)) //outside
		{	Point3 P1;
			P1=p+dt*Vrel; //second particle position
			if (!(LengthSquared(P1-Vc)<(rplus*rplus)))
			{	float Dist,Dist1;
				Dist=DotProd(Vreln,(Vc-p));
				if (Dist<0.0f)
				{	obj->lastrnd=rand();
					return FALSE;
				}
				Dist1=DotProd(-Vreln,(Vc-P1));
				if (Dist1<0.0f)
				{	obj->lastrnd=rand();
					return FALSE;
				}
				Point3 P10=P1-p,Pc=Vc-p;
				float gamma=(float)acos(DotProd(P10,Pc)/(Length(P10)*Length(Pc)));
				float Dist2=Length(Pc)*(float)cos(HalfPI-gamma);
				if (Dist2>radius)
				{	obj->lastrnd=rand();
					return FALSE;
				}
			}
			float A,B,C,omega,omega1,omegaend,a2,ptmp,c,d;
			A=LengthSquared(Vrel);
			B=2.0f*DotProd(p,Vrel)-2.0f*DotProd(Vrel,Vc);
			C=(ptmp=LengthSquared(p))+(c=LengthSquared(Vc))-(d=2.0f*DotProd(p,Vc))-rsquare;
			omegaend=B*B-4.0f*A*C;
			if (omegaend<0.0f) omegaend=0.0f;
			else omegaend=(float)sqrt(omegaend);
			a2=2.0f*A;
			omega1=(-B+omegaend)/a2;
			omega=(-B-omegaend)/a2;
			if (((omega1>0.0f)&&(omega1<omega))||((omega<0.0f)&&(omega1>omega))) omega=omega1;
			float fdt=1.1f*dt;
			if ((omega>fdt)||(omega<-fdt))
			{	obj->lastrnd=rand();
				return FALSE;
			}
// solve for collision params starts here
			Point3 XI,r,n;
			XI=p+omega*vr;
			// to guarantee that particle is inside now (Bayboro 4/30/01)
			XI = Vc + Normalize(XI-Vc)*0.9999f*rminus;

			Point3 Ci=Vc+omega*Vdt;
			r=Normalize(XI-Ci);
			float q1=DotProd(-Vreln,r);
			float theta=(float)acos(q1);
			if (theta>=HalfPI) theta-=PI;
// refraction solution
// reduce v by decel parameters
			vr*=decel*(1.0f-decelvar*RND01());
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
					c=Normalize(vr^d);
				}
				else
				{	c=Normalize(r^(-vr));
				}
				RotateOnePoint(vr,&Zero.x,&c.x,refangle);
				if ((TempDP=DotProd(vr,r))>0.0f) vr=vr-TempDP*r;
			}
			float maxdiff,diffuse,diffvar,diffangle;
//			obj->pblock->GetValue(PB_DIFFUSION,t,diffuse,FOREVER);
//			obj->pblock->GetValue(PB_DIFFUSIONVAR,t,diffvar,FOREVER);
			obj->pblock2->GetValue(sspflectrobj_diffusion,t,diffuse,FOREVER);
//			diffuse *= 0.01f;
			obj->pblock2->GetValue(sspflectrobj_diffusionvar,t,diffvar,FOREVER);
//			diffvar *= 0.01f;
			maxdiff=HalfPI-theta-refangle;
			if (!FloatEQ0(diffuse))
			{	
				// Martell 4/14/01: Fix for order of ops bug.
				float ztmp=RND11(); float ytmp=RND11(); float xtmp=RND11();				
				Point3 d=Point3(xtmp,ytmp,ztmp);
				Point3 c=Normalize(vr^d);
				diffangle=0.5f*maxdiff*diffuse*(1.0f+RND11()*diffvar);
				RotateOnePoint(vr,&Zero.x,&c.x,diffangle);
				if (TempDP=(DotProd(vr,r))>0.0f) vr=vr-TempDP*r;
			}
			if ((vinher>0.0f)&&(t>0)){vr=vr+DotProd(Vdt*vinher,r)*r;}
			if (UpdatePastCollide)
			{	pos = XI + (dt-omega)*vr;
				if (ct) (*ct) = dt;
			}
			else
			{	pos = XI;
				if (ct) (*ct) = omega;
			}
		}
		else //inside
		{	Point3 P1;
			P1=p+dt*Vrel;
			if (LengthSquared(P1-Vc)<(rplus*rplus))
			{	obj->lastrnd=rand();
				return FALSE;
			}
			float A,B,C,omega,omega1,omegaend,a2,ptmp,c,d;
			A=LengthSquared(Vrel);
			B=2.0f*DotProd(p,Vrel)-2.0f*DotProd(Vrel,Vc);
			C=(ptmp=LengthSquared(p))+(c=LengthSquared(Vc))-(d=2.0f*DotProd(p,Vc))-rsquare;
			omegaend=B*B-4.0f*A*C;
			if (omegaend<0.0f) omegaend=0.0f;
			else omegaend=(float)sqrt(omegaend);
			a2=2.0f*A;
			omega1=(-B+omegaend)/a2;
			omega=(-B-omegaend)/a2;
			if (((omega1>0.0f)&&(omega1<omega))||((omega<0.0f)&&(omega1>omega))) omega=omega1;
			float fdt=1.1f*dt;
			if ((omega>fdt)||(omega<-fdt))
			{	obj->lastrnd=rand();
				return FALSE;
			}
			Point3 XI,r,n;
			XI=p+omega*vr;
			// to guarantee that particle is outside now (Bayboro 4/30/01)
			XI = Vc + Normalize(XI-Vc)*1.0001f*rplus;
			Point3 Ci=Vc+omega*Vdt;
			r=Normalize(XI-Ci);
			float q1=DotProd(Vreln,r);
			if (q1>1.0f) q1=1.0f;
			if (q1<-1.0f) q1=-1.0f;
			float theta=(float)acos(q1);
// refraction solution
// reduce v by decel parameters
			vr *= decel*(1.0f-decelvar*RND01());
// rotate velocity vector
			float maxref,refangle,maxvarref;
			refangle=0.0f;
			if (!FloatEQ0(refvol))
			{	if (refvol>0.0f)
					maxref=HalfPI-theta;
				else 
					maxref=-theta;
				refangle=maxref*(float)fabs(refvol);
				float frefangle=(float)fabs(refangle);
				if (refvol>0.0f)
					maxvarref=theta-frefangle;
				else
					maxvarref=HalfPI-theta-frefangle;
				refangle+=maxvarref*RND11()*refvar;
				Point3 c,d;
				if (theta<0.01f)
				{	
					// Martell 4/14/01: Fix for order of ops bug.
					float ztmp=RND11(); float ytmp=RND11(); float xtmp=RND11();			
					d=Point3(xtmp,ytmp,ztmp);
					c=Normalize(vr^d);
				}
				else
				{	c=Normalize(r^vr);
				}
				RotateOnePoint(vr,&Zero.x,&c.x,refangle);
				if ((TempDP=DotProd(vr,r))<0.0f) vr=vr-TempDP*r;
			}
			float maxdiff,diffuse,diffvar,diffangle;
//			obj->pblock->GetValue(PB_DIFFUSION,t,diffuse,FOREVER);
//			obj->pblock->GetValue(PB_DIFFUSIONVAR,t,diffvar,FOREVER);
			obj->pblock2->GetValue(sspflectrobj_diffusion,t,diffuse,FOREVER);
//			diffuse *= 0.01f;
			obj->pblock2->GetValue(sspflectrobj_diffusionvar,t,diffvar,FOREVER);
//			diffvar *= 0.01f;
			maxdiff=HalfPI-theta-refangle;
			if (!FloatEQ0(diffuse))
			{	
				// Martell 4/14/01: Fix for order of ops bug.
				float ztmp=RND11(); float ytmp=RND11(); float xtmp=RND11();				
				Point3 d=Point3(xtmp,ytmp,ztmp);
				Point3 c=Normalize(vr^d);
				diffangle=0.5f*maxdiff*diffuse*(1.0f+RND11()*diffvar);
				RotateOnePoint(vr,&Zero.x,&c.x,diffangle);
				if (TempDP=(DotProd(vr,r))<0.0f) vr=vr-TempDP*r;
			}
			if ((vinher>0.0f)&&(t>0)){vr=vr+DotProd(Vdt*vinher,r)*r;}
			if (UpdatePastCollide)
			{	pos = XI + (dt-omega)*vr;
				if (ct) (*ct) = dt;
			}
			else
			{	pos = XI;
				if (ct) (*ct) = omega;
			}
		}
		pos = pos*tm;
		vel = VectorTransform(tm,vr);
		obj->lastrnd=rand();
		return TRUE;
	}

// test for spawns only
	float spawnsonly;
//	obj->pblock->GetValue(PB_SPAWN,t,spawnsonly,FOREVER);
	obj->pblock2->GetValue(sspflectrobj_spawn,t,spawnsonly,FOREVER);
//	spawnsonly *= 0.01f;

	if (RND01()<spawnsonly)
//	{	obj->pblock->GetValue(PB_RADIUS,t,radius,FOREVER);
	{	obj->pblock2->GetValue(sspflectrobj_radius,t,radius,FOREVER);
		p=pos*invtm; 
		vr=VectorTransform(invtm,vel);
		Vdt=(Vcp-Vc)/dt;
		Vrel=vr-Vdt;
		rsquare=radius*radius;
		rplus=radius;rminus=radius;
		Vreln=Normalize(Vrel);
		if (LengthSquared(p-Vc)>=(rminus*rminus)) //outside
		{	Point3 P1;
			P1=p+dt*Vrel; //second particle position
			if (!(LengthSquared(P1-Vc)<(rplus*rplus)))
			{	float Dist,Dist1;
				Dist=DotProd(Vreln,(Vc-p));
				if (Dist<0.0f)
				{	obj->lastrnd=rand();
					return FALSE;
				}
				Dist1=DotProd(-Vreln,(Vc-P1));
				if (Dist1<0.0f)
				{	obj->lastrnd=rand();
					return FALSE;
				}
				Point3 P10=P1-p,Pc=Vc-p;
				float gamma=(float)acos(DotProd(P10,Pc)/(Length(P10)*Length(Pc)));
				float Dist2=Length(Pc)*(float)cos(HalfPI-gamma);
				if (Dist2>radius)
				{	obj->lastrnd=rand();
					return FALSE;
				}
			}
			float A,B,C,omega,omega1,omegaend,a2,ptmp,c,d;
			A=LengthSquared(Vrel);
			B=2.0f*DotProd(p,Vrel)-2.0f*DotProd(Vrel,Vc);
			C=(ptmp=LengthSquared(p))+(c=LengthSquared(Vc))-(d=2.0f*DotProd(p,Vc))-rsquare;
			omegaend=B*B-4.0f*A*C;
			if (omegaend<0.0f) omegaend=0.0f;
			else omegaend=(float)sqrt(omegaend);
			a2=2.0f*A;
			omega1=(-B+omegaend)/a2;
			omega=(-B-omegaend)/a2;
			if (((omega1>0.0f)&&(omega1<omega))||((omega<0.0f)&&(omega1>omega))) omega=omega1;
			float fdt=1.1f*dt;
			if ((omega>fdt)||(omega<-fdt))
			{	obj->lastrnd=rand();
				return FALSE;
			}
// solve for collision params starts here
			Point3 XI;
			XI=p+omega*vr;
			// to guarantee that particle is inside now (Bayboro 4/30/01)
			XI = Vc + Normalize(XI-Vc)*0.9999f*rminus;
			float passvel,passvelvar;
//			obj->pblock->GetValue(PB_PASSVEL,t,passvel,FOREVER);
//			obj->pblock->GetValue(PB_PASSVELVAR,t,passvelvar,FOREVER);
			obj->pblock2->GetValue(sspflectrobj_passvel,t,passvel,FOREVER);
			obj->pblock2->GetValue(sspflectrobj_passvelvar,t,passvelvar,FOREVER);
//			passvelvar *= 0.01f;
			vr*=passvel*(1.0f+passvelvar*RND11());
			pos=XI+(dt-omega)*vr;
		}
		else //inside
		{	Point3 P1;
			P1=p+dt*Vrel;
			if (LengthSquared(P1-Vc)<(rplus*rplus))
			{	obj->lastrnd=rand();
				return FALSE;
			}
			float A,B,C,omega,omega1,omegaend,a2,ptmp,c,d;
			A=LengthSquared(Vrel);
			B=2.0f*DotProd(p,Vrel)-2.0f*DotProd(Vrel,Vc);
			C=(ptmp=LengthSquared(p))+(c=LengthSquared(Vc))-(d=2.0f*DotProd(p,Vc))-rsquare;
			omegaend=B*B-4.0f*A*C;
			if (omegaend<0.0f) omegaend=0.0f;
			else omegaend=(float)sqrt(omegaend);
			a2=2.0f*A;
			omega1=(-B+omegaend)/a2;
			omega=(-B-omegaend)/a2;
			if (((omega1>0.0f)&&(omega1<omega))||((omega<0.0f)&&(omega1>omega))) omega=omega1;
			float fdt=1.1f*dt;
			if ((omega>fdt)||(omega<-fdt))
			{	obj->lastrnd=rand();
				return FALSE;
			}
// solve for collision params starts here
			Point3 XI;
			XI=p+omega*vr;
			// to guarantee that particle is outside now (Bayboro 4/30/01)
			XI = Vc + Normalize(XI-Vc)*1.0001f*rplus;
			float passvel,passvelvar;
//			obj->pblock->GetValue(PB_PASSVEL,t,passvel,FOREVER);
//			obj->pblock->GetValue(PB_PASSVELVAR,t,passvelvar,FOREVER);
			obj->pblock2->GetValue(sspflectrobj_passvel,t,passvel,FOREVER);
			obj->pblock2->GetValue(sspflectrobj_passvelvar,t,passvelvar,FOREVER);
//			passvelvar *= 0.01f;
			vr *= passvel*(1.0f+passvelvar*RND11());
			if (UpdatePastCollide)
			{	pos = XI + (dt-omega)*vr;
				if (ct) (*ct) = dt;
			}
			else
			{	pos = XI;
				if (ct) (*ct) = omega;
			}
		}
		pos = pos*tm;
		vel = VectorTransform(tm,vr);
		obj->lastrnd=rand();
		return TRUE;
	}

// nothing happens
	obj->lastrnd=rand();
	return FALSE;
}

class SSpflectObjectLoad : public PostLoadCallback 
{
	public:
		SSpawnDeflObject *n;
		SSpflectObjectLoad(SSpawnDeflObject *ns) {n = ns;}
		void proc(ILoad *iload) 
		{  
			ReferenceTarget *rt;
			Interval iv;
			n->pblock2->GetValue(sspflectrobj_collider,0,rt,iv);
			if (rt == NULL)
			{
				CollisionPlane *cols = (CollisionPlane*)CreateInstance(REF_MAKER_CLASS_ID, SPHERICAL_COLLISION_ID);
				if (cols)
					n->pblock2->SetValue(sspflectrobj_collider,0,(ReferenceTarget*)cols);
			}
			delete this; 
		} 
};

class SSpawnDeflPostLoadCallback : public PostLoadCallback {
	public:
//		ParamBlockPLCB *cb;
//		SSpawnDeflPostLoadCallback(ParamBlockPLCB *c) {cb=c;}

		ParamBlock2PLCB *cb;
		SSpawnDeflPostLoadCallback(ParamBlock2PLCB *c) {cb=c;}

		void proc(ILoad *iload) {
//			DWORD oldVer = ((SSpawnDeflObject*)(cb->targ))->pblock->GetVersion();
			DWORD oldVer = ((SSpawnDeflObject*)(cb->targ))->pblock2->GetVersion();
			ReferenceTarget *targ = cb->targ;
			cb->proc(iload);
			if (oldVer<1) {	
 //				((SSpawnDeflObject*)targ)->pblock->SetValue(PB_SPAWN,0,1.0f);
 //				((SSpawnDeflObject*)targ)->pblock->SetValue(PB_PASSVEL,0,1.0f);
 //				((SSpawnDeflObject*)targ)->pblock->SetValue(PB_PASSVELVAR,0,0);
 				((SSpawnDeflObject*)targ)->pblock2->SetValue(sspflectrobj_spawn,0,1.0f);
 				((SSpawnDeflObject*)targ)->pblock2->SetValue(sspflectrobj_passvel,0,1.0f);
 				((SSpawnDeflObject*)targ)->pblock2->SetValue(sspflectrobj_passvelvar,0,0);
				}
			delete this;
			}
	};

IOResult SSpawnDeflObject::Load(ILoad *iload) 
	{	
	// This is the callback that corrects for any older versions
	// of the parameter block structure found in the MAX file 
	// being loaded.
	iload->RegisterPostLoadCallback(
		new ParamBlock2PLCB(ssversions,NUM_OLDVERSIONS,&sspflector_param_blk,this,PBLOCK_REF_NO));
	iload->RegisterPostLoadCallback(new SSpflectObjectLoad(this));
	return IO_OK;
	}

CollisionObject *SSpawnDeflObject::GetCollisionObject(INode *node)
{
	SSpawnDeflField *gf = new SSpawnDeflField;	
	gf->obj  = this;
	gf->node = node;
	gf->tmValid.SetEmpty();
	return gf;
}
