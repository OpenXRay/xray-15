/**********************************************************************
 *<
	FILE: udyflect.cpp

	DESCRIPTION: Turns Any Mesh Into a DynaFlector

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
static Class_ID UDYNADEFL_CLASS_ID(0x685771aa, 0x678144bd);
static Class_ID UDYNADEFLMOD_CLASS_ID(0xcc400b8, 0x8732e2f);

class UnjPickOperand; 

const float dymin=0.0001f;

class UDynaDeflObject : public DynamModObject {	
	public:		
//		static IParamMap *pmapParam;
		static IObjParam *ip;
		static HWND hSot;
		static HWND hParams;

		int lastrnd;
		TimeValue t;
		INode *custnode;
		TSTR custname;
		UDynaDeflObject();
		~UDynaDeflObject();
		BOOL SupportsDynamics() {return TRUE;}
		Mesh *dmesh;
		int nv,nf;
		VNormal *vnorms;
		Point3 *fnorms;
		Matrix3 tm,ptm,invtm,tmNoTrans,invtmNoTrans;
		Interval tmValid,mValid;
		Point3 dvel;

		static BOOL creating;
		static UnjPickOperand pickCB;

		void ShowName();
		// From Animatable		
		void DeleteThis() {delete this;}		
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);		
		void MapKeys(TimeMap *map,DWORD flags);
		Class_ID ClassID() {return UDYNADEFL_CLASS_ID;}		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_EP_UDYNADEFLECTOR_OBJECT);}
				
		// from object		
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}

		// From BaseObject
//		IParamArray *GetParamBlock() {return pblock;}
//		int GetParamBlockIndex(int id) {return id;}
		
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
		FlectForces ForceData(TimeValue t);
		FlectForces ffdata;	
		TimeValue ctime;

		// Direct paramblock access
		int	NumParamBlocks() { return 1; }
		IParamBlock2* GetParamBlock(int i) { return pblock2; }
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock2->ID() == id) ? pblock2 : NULL; }
	};

class UnjPickOperand : 
		public PickModeCallback,
		public PickNodeCallback {
	public:		
		UDynaDeflObject *po;
		
		UnjPickOperand() {po=NULL;}

		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);

		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);

		BOOL Filter(INode *node);
		
		PickNodeCallback *GetFilter() {return this;}
	};

class CreateUDynaDeflPickNode : public RestoreObj {
	public:   		
		UDynaDeflObject *obj;
		INode *oldn;
		CreateUDynaDeflPickNode(UDynaDeflObject *o, INode *n) {
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

IObjParam *UDynaDeflObject::ip        = NULL;
//IParamMap *UDynaDeflObject::pmapParam = NULL;
HWND       UDynaDeflObject::hSot      = NULL;
HWND       UDynaDeflObject::hParams      = NULL;
BOOL UDynaDeflObject::creating    = FALSE;
UnjPickOperand UDynaDeflObject::pickCB;

class UDynaDeflClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) { return new UDynaDeflObject;}
	const TCHAR *	ClassName() {return GetString(IDS_EP_UDYNADEFLECTOR);}
	SClass_ID		SuperClassID() {return WSM_OBJECT_CLASS_ID; }
	Class_ID		ClassID() {return UDYNADEFL_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_EP_SW_DEFLECTORS);}
	int 			BeginCreate(Interface *i);
	int 			EndCreate(Interface *i);

	// Hardwired name, used by MAX Script as unique identifier
	const TCHAR*	InternalName()				{ return _T("UDynaDeflector"); }
	HINSTANCE		HInstance()					{ return hInstance; }
	};

static UDynaDeflClassDesc UDynaDeflDesc;
ClassDesc* GetUDynaDeflObjDesc() {return &UDynaDeflDesc;}

//--- DeflectMod -----------------------------------------------------

class UDynaDeflField : public CollisionObject {
	public:		
		UDynaDeflField()
		{
//	 		colm = (CollisionMesh*)CreateInstance(REF_MAKER_CLASS_ID, MESH_COLLISION_ID);
			srand(453442);
			for (int i =0;i < 500; i++)
			{
				randomFloat[i] = (float)rand()/(float)RAND_MAX;
			}
			colm=NULL;
		}

		~UDynaDeflField()
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

		float chaos,bounce,bvar,vinher,friction;

		UDynaDeflObject *obj;
		INode *node;
		int badmesh;
		Point3 totalforce,applyat;
		int totalnumber;
		TimeValue curtime;
		BOOL CheckCollision(TimeValue t,Point3 &pos, Point3 &vel, float dt, int index,float *ct,BOOL UpdatePastCollide);
		Object *GetSWObject();
		CollisionMesh *colm;
	};

class UDynaDeflMod : public SimpleWSMMod {
	public:				
		UDynaDeflField deflect;

		UDynaDeflMod() {deflect.curtime=NoAni;}
		UDynaDeflMod(INode *node,UDynaDeflObject *obj);	


		// From Animatable
		void GetClassName(TSTR& s) {s= GetString(IDS_EP_UDYNADEFLECTORMOD);}
		SClass_ID SuperClassID() {return WSM_CLASS_ID;}
		void DeleteThis() {delete this;}
		Class_ID ClassID() { return UDYNADEFLMOD_CLASS_ID;}
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_EP_UDYNADEFLECTORBINDING);}

		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);

		// From SimpleWSMMod		
		Interval GetValidity(TimeValue t);		
		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);
	};

//--- ClassDescriptor and class vars ---------------------------------

class UDynaDeflModClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 0;}
	void *			Create(BOOL loading = FALSE) { return new UDynaDeflMod;}
	const TCHAR *	ClassName() {return GetString(IDS_EP_UDYNADEFLECTORMOD);}
	SClass_ID		SuperClassID() {return WSM_CLASS_ID; }
	Class_ID		ClassID() {return UDYNADEFLMOD_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
	};

static UDynaDeflModClassDesc UDynaDeflModDesc;
ClassDesc* GetUDynaDeflModDesc() {return &UDynaDeflModDesc;}

enum 
{
	udyflectrobj_params, 
}; 

enum 
{
	udyflectrobj_timeon, 
	udyflectrobj_timeoff, 
	udyflectrobj_affects, 
	udyflectrobj_bounce, 
	udyflectrobj_bouncevar,
	udyflectrobj_chaos,
	udyflectrobj_friction,
	udyflectrobj_velocity,
	udyflectrobj_radius,
	udyflectrobj_mass,
	udyflectrobj_massunits,
	udyflectrobj_forcex,
	udyflectrobj_forcey,
	udyflectrobj_forcez,
	udyflectrobj_applyx,
	udyflectrobj_applyy,
	udyflectrobj_applyz,
	udyflectrobj_number,
	udyflectrobj_collider
};

#define PBLOCK_REF_NO	0

static ParamBlockDesc2 udyflector_param_blk 
(	udyflectrobj_params, 
	_T("UDynaflectorParameters"),  
	0, 
	&UDynaDeflDesc, 
	P_AUTO_CONSTRUCT + P_AUTO_UI, 
	PBLOCK_REF_NO, 

	//rollout
	IDD_AP_UDYNADEFL, IDS_EP_PARAMETERS, 0, 0, NULL, 

	// params

	udyflectrobj_timeon,	_T("timeOn"),		TYPE_TIMEVALUE,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_TIMEON,
		p_default,			1,
		p_range,			-999999999, 999999999, 
		p_ui,				TYPE_SPINNER, EDITTYPE_INT,		IDC_EP_TIMEON,				IDC_EP_TIMEONSPIN,	0.1f,
		end,

	udyflectrobj_timeoff,	_T("timeOff"),		TYPE_TIMEVALUE,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_TIMEOFF,
		p_default,			1,
		p_range,			-999999999, 999999999, 
		p_ui,				TYPE_SPINNER, EDITTYPE_INT,		IDC_EP_TIMEOFF,				IDC_EP_TIMEOFFSPIN, 0.1f,
		end,

	udyflectrobj_affects,	_T("affects"),		TYPE_PCNT_FRAC,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_AFFECTS,
		p_default,			1.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_AFFECTS,				IDC_EP_AFFECTSSPIN, 1.0f,
		end,

	udyflectrobj_bounce,	_T("bounce"),		TYPE_FLOAT,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_BOUNCE,
		p_default,			1.0f,
		p_range,			0.0f,65535.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_BOUNCEU,				IDC_EP_BOUNCEUSPIN,			0.01f,
		end,

	udyflectrobj_bouncevar, _T("bounceVar"),	TYPE_PCNT_FRAC,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_BOUNCEVAR,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_BOUNCEUVAR,			IDC_EP_BOUNCEUVARSPIN,		1.0f,
		end,

	udyflectrobj_chaos,		_T("chaos"),		TYPE_PCNT_FRAC,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_CHAOS,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_BOUNCEUCHAOS,		IDC_EP_BOUNCEUCHAOSSPIN,	1.0f,
		end,

	udyflectrobj_friction,	_T("friction"),		TYPE_PCNT_FRAC,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_FRICTION,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_BOUNCEUFRICTION,		IDC_EP_BOUNCEUFRICTIONSPIN, 1.0f,
		end,

	udyflectrobj_velocity,	_T("inheritVelocity"),	TYPE_FLOAT,	P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_INHERIT,
		p_default,			1.0f,
		p_range,			0.0f, 9999999.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_BOUNCEUINHERIT,		IDC_EP_BOUNCEUINHERITSPIN,	SPIN_AUTOSCALE,
		end,

	udyflectrobj_radius,	_T("radius"),		TYPE_FLOAT,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_ICONSIZE,
		p_default,			0.0f,
		p_range,			0.0f,9999999.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_UNIVERSE,	IDC_EP_ICONSIZE,			IDC_EP_ICONSIZESPIN,		SPIN_AUTOSCALE,
		end,

	udyflectrobj_mass,		_T("mass"),			TYPE_FLOAT,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_MASS,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_PARTICLEMASSU,		IDC_EP_PARTICLEMASSUSPIN,		1.0f,
		end,

	udyflectrobj_massunits,	_T("massUnits"),	TYPE_RADIOBTN_INDEX,	0,		IDS_EP_MASSUNITS,
		p_default,			0,
		p_range,			0, 2, 
		p_ui,				TYPE_RADIO,		3,	IDC_EP_MASSGM, IDC_EP_MASSKG, IDC_EP_MASSLBM,
		end,

//	udyflectrobj_forcex,	/*_T("forceInX")*/_T(""),	TYPE_FLOAT,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_FORCEX,
//	deflector forceInX parameter is no longer used (Bayboro 4/25/01)
	udyflectrobj_forcex,	_T(""),	TYPE_FLOAT,		0,	0,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
//		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_BOUNCEVAR,			IDC_EP_BOUNCEVARSPIN, 1.0f,
		end,

//	udyflectrobj_forcey,	/*_T("forceInY")*/_T(""),	TYPE_FLOAT,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_FORCEY,
//	deflector forceInY parameter is no longer used (Bayboro 4/25/01)
	udyflectrobj_forcey,	_T(""),	TYPE_FLOAT,		0,	0,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
//		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_BOUNCEVAR,			IDC_EP_BOUNCEVARSPIN, 1.0f,
		end,

//	udyflectrobj_forcez,	/*_T("forceInZ")*/_T(""),	TYPE_FLOAT,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_FORCEZ,
//	deflector forceInZ parameter is no longer used (Bayboro 4/25/01)
	udyflectrobj_forcez,	_T(""),	TYPE_FLOAT,		0,	0,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
//		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_EP_BOUNCEVAR,			IDC_EP_BOUNCEVARSPIN, 1.0f,
		end,

//	udyflectrobj_applyx,	/*_T("applyAtX")*/_T(""),	TYPE_FLOAT,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_APPLYX,
//	deflector applyAtX parameter is no longer used (Bayboro 4/25/01)
	udyflectrobj_applyx,	_T(""),	TYPE_FLOAT,		0,	0,
		p_default,			0.0f,
		p_range,			0.0f,9999999.0f, 
//		p_ui,				TYPE_SPINNER, EDITTYPE_UNIVERSE,	IDC_EP_ICONSIZE,			IDC_EP_ICONSIZESPIN,	SPIN_AUTOSCALE,
		end,

//	udyflectrobj_applyy,	/*_T("applyAtY")*/_T(""),	TYPE_FLOAT,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_APPLYY,
//	deflector applyAtY parameter is no longer used (Bayboro 4/25/01)
	udyflectrobj_applyy,	_T(""),	TYPE_FLOAT,		0,	0,
		p_default,			0.0f,
		p_range,			0.0f,9999999.0f, 
//		p_ui,				TYPE_SPINNER, EDITTYPE_UNIVERSE,	IDC_EP_ICONSIZE,			IDC_EP_ICONSIZESPIN,	SPIN_AUTOSCALE,
		end,

//	udyflectrobj_applyz,	/*_T("applyAtZ")*/_T(""),	TYPE_FLOAT,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_EP_APPLYZ,
//	deflector applyAtZ parameter is no longer used (Bayboro 4/25/01)
	udyflectrobj_applyz,	_T(""),	TYPE_FLOAT,		0,	0,
		p_default,			0.0f,
		p_range,			0.0f,9999999.0f, 
//		p_ui,				TYPE_SPINNER, EDITTYPE_UNIVERSE,	IDC_EP_ICONSIZE,			IDC_EP_ICONSIZESPIN,	SPIN_AUTOSCALE,
		end,

//	udyflectrobj_number,	/*_T("number")*/_T(""),		TYPE_INT, 		P_ANIMATABLE + P_RESET_DEFAULT, 	IDS_EP_NUMBER, 
//	deflector number parameter is no longer used (Bayboro 4/25/01)
	udyflectrobj_number,	_T(""),		TYPE_INT, 	0,	0,
		p_default, 			20,	
		p_range, 			0, 100,
//		p_ui, 				TYPE_SPINNER, EDITTYPE_INT,			IDC_EP_QUALITY,				IDC_EP_QUALITYSPIN, SPIN_AUTOSCALE, 
		end, 

//watje ref to hold the collision engine
	udyflectrobj_collider,  _T(""),		TYPE_REFTARG, 	0,0, 	//IDS_EP_FRICTION, 
		end, 

	end
);



/*
//--- UnjDefObject Parameter map/block descriptors ------------------

#define PB_TIMEON		0
#define PB_TIMEOFF		1
#define PB_AFFECTS		2
#define PB_BOUNCE		3
#define PB_BOUNCEVAR	4
#define PB_CHAOS		5
#define PB_FRICTION		6
#define PB_INHERIT		7
#define PB_ICONSIZE		8
#define PB_MASS			9
#define PB_MASSUNITS	10
#define PB_FORCEX		11
#define PB_FORCEY		12
#define PB_FORCEZ		13
#define PB_APPLYX		14
#define PB_APPLYY		15
#define PB_APPLYZ		16
#define PB_NUMBER		17

static int massUunitsIDs[] = {IDC_EP_MASSGM,IDC_EP_MASSKG,IDC_EP_MASSLBM};

static ParamUIDesc descUDynaParam[] = {
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
	
	// Icon Size
	ParamUIDesc(
		PB_ICONSIZE,
		EDITTYPE_UNIVERSE,
		IDC_EP_ICONSIZE,IDC_EP_ICONSIZESPIN,
		0.0f, 9999999.0f,
		SPIN_AUTOSCALE),

	// Particle Mass
	ParamUIDesc(
		PB_MASS,
		EDITTYPE_FLOAT,
		IDC_EP_PARTICLEMASSU,IDC_EP_PARTICLEMASSUSPIN,
		0.0f, 9999999.0f,
		SPIN_AUTOSCALE),

	// Particle Mass Units
	ParamUIDesc(PB_MASSUNITS,TYPE_RADIO,massUunitsIDs,3)

	};
*/

//#define PARAMDESC_LENGTH	11

ParamBlockDescID descUDynaVer0[] = {
	{ TYPE_INT, NULL, FALSE, 0 },	//timeon
	{ TYPE_INT, NULL, TRUE, 1 },	//timeoff
	{ TYPE_FLOAT, NULL, TRUE, 2 },//affects
	{ TYPE_FLOAT, NULL, TRUE, 3 },//bounce
	{ TYPE_FLOAT, NULL, TRUE, 4 },//bouncevar
	{ TYPE_FLOAT, NULL, TRUE, 5 },//chaos
	{ TYPE_FLOAT, NULL, TRUE, 6 },//friction
	{ TYPE_FLOAT, NULL, TRUE, 7 },//inherit
	{ TYPE_FLOAT, NULL, FALSE, 8 },//iconsize
	{ TYPE_FLOAT, NULL, TRUE, 9 },//mass
	{ TYPE_INT, NULL, FALSE, 10 },//massunits
	{ TYPE_FLOAT, NULL, FALSE, 11 },//force
	{ TYPE_FLOAT, NULL, FALSE, 12 },
	{ TYPE_FLOAT, NULL, FALSE, 13 },
	{ TYPE_FLOAT, NULL, FALSE, 14 },//apply
	{ TYPE_FLOAT, NULL, FALSE, 15 },
	{ TYPE_FLOAT, NULL, FALSE, 16 },
	{ TYPE_INT, NULL, FALSE, 17 }//total number
};	

#define PBLOCK_LENGTH	18

// Array of old ParamBlock Ed. 1 versions
static ParamVersionDesc usversions[] = 
{
	ParamVersionDesc(descUDynaVer0,18,0),
};

//#define CURRENT_VERSION	0
#define NUM_OLDVERSIONS	1

//--- Universal Deflect object methods -----------------------------------------
class CreateUDynaDeflObjectProc : public MouseCallBack,ReferenceMaker {
	private:
		IObjParam *ip;
		void Init(IObjParam *i) {ip=i;}
		CreateMouseCallBack *createCB;	
		INode *CloudNode;
		UDynaDeflObject *UspObj;
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
		
		CreateUDynaDeflObjectProc()
			{
			ignoreSelectionChange = FALSE;
			}
		int createmethod(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
		int proc( HWND hwnd, int msg, int point, int flag, IPoint2 m );
	};


#define CID_CREATEUDynaDeflObjectMODE	CID_USER + 12

class CreateUDynaDeflObjectMode : public CommandMode {		
	public:		
		CreateUDynaDeflObjectProc proc;
		IObjParam *ip;
		UDynaDeflObject *obj;
		void Begin( IObjCreate *ioc, ClassDesc *desc ) { proc.Begin( ioc, desc ); }
		void End() { proc.End(); }
		void JumpStart(IObjParam *i,UDynaDeflObject*o);

		int Class() {return CREATE_COMMAND;}
		int ID() { return CID_CREATEUDynaDeflObjectMODE; }
		MouseCallBack *MouseProc(int *numPoints) {*numPoints = 10000; return &proc;}
		ChangeForegroundCallback *ChangeFGProc() {return CHANGE_FG_SELECTED;}
		BOOL ChangeFG( CommandMode *oldMode ) { return (oldMode->ChangeFGProc() != CHANGE_FG_SELECTED); }
		void EnterMode() 
		{ GetCOREInterface()->PushPrompt(GetString(IDS_AP_CREATEMODE));
		  SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR)));
		}
		void ExitMode() {GetCOREInterface()->PopPrompt();SetCursor(LoadCursor(NULL, IDC_ARROW));}
		BOOL IsSticky() { return FALSE; }
	};

static CreateUDynaDeflObjectMode theCreateUDynaDeflObjectMode;

RefResult CreateUDynaDeflObjectProc::NotifyRefChanged(
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
						{  theCreateUDynaDeflObjectMode.JumpStart(UspObj->ip,UspObj);
						   createInterface->SetCommandMode(&theCreateUDynaDeflObjectMode);
					    } 
				  else {createInterface->SetStdCommandMode(CID_OBJMOVE);}
				}
#ifdef _OSNAP
				UspObj->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
				UspObj->EndEditParams( (IObjParam*)createInterface, 0, NULL);
				UspObj  = NULL;				
				CloudNode    = NULL;
				CreateNewObject();	
				attachedToNode = FALSE;
				}
			break;		
		}
	return REF_SUCCEED;
	}

void CreateUDynaDeflObjectProc::Begin( IObjCreate *ioc, ClassDesc *desc )
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

void CreateUDynaDeflObjectProc::CreateNewObject()
{
	SuspendSetKeyMode();
  createInterface->GetMacroRecorder()->BeginCreate(cDesc);
	UspObj = (UDynaDeflObject*)cDesc->Create();
	lastPutCount  = theHold.GetGlobalPutCount();
	// Start the edit params process
	if ( UspObj ) {
		UspObj->BeginEditParams( (IObjParam*)createInterface, BEGIN_EDIT_CREATE, NULL );
		UspObj->SetAFlag(A_OBJ_CREATING);
#ifdef _OSNAP
		UspObj->SetAFlag(A_OBJ_LONG_CREATE);
#endif
		}
	ResumeSetKeyMode();
}

//LACamCreationManager::~LACamCreationManager
void CreateUDynaDeflObjectProc::End()
{ if ( UspObj ) 
	{ 
#ifdef _OSNAP
	UspObj->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
	UspObj->EndEditParams( (IObjParam*)createInterface, 
	                    	          END_EDIT_REMOVEUI, NULL);
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

void CreateUDynaDeflObjectMode::JumpStart(IObjParam *i,UDynaDeflObject *o)
	{
	ip  = i;
	obj = o;
	//MakeRefByID(FOREVER,0,svNode);
	obj->BeginEditParams(i,BEGIN_EDIT_CREATE,NULL);
	}

int UDynaDeflClassDesc::BeginCreate(Interface *i)
	{	
	SuspendSetKeyMode();
	IObjCreate *iob = i->GetIObjCreate();
	theCreateUDynaDeflObjectMode.Begin(iob,this);
	iob->PushCommandMode(&theCreateUDynaDeflObjectMode);
	return TRUE;
	}

int UDynaDeflClassDesc::EndCreate(Interface *i)
	{
	ResumeSetKeyMode();
	theCreateUDynaDeflObjectMode.End();
	i->RemoveMode(&theCreateUDynaDeflObjectMode);
	macroRec->EmitScript();  // 10/00
	return TRUE;
	}

int CreateUDynaDeflObjectProc::createmethod(
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
//				UspObj->pmapParam->Invalidate();
				UspObj->pblock2->SetValue(udyflectrobj_radius,0,0.01f);
				udyflector_param_blk.InvalidateUI();
				break;

			case 1: {
				mat.IdentityMatrix();
				sp1 = m;
				p1  = vpt->SnapPoint(m,m,NULL,snapdim);
				Point3 center = (p0+p1)/float(2);
				mat.SetTrans(center);
//				UspObj->pblock->SetValue(PB_ICONSIZE,0,(float)fabs(p1.x-p0.x));
//				UspObj->pmapParam->Invalidate();
				UspObj->pblock2->SetValue(udyflectrobj_radius,0,(float)fabs(p1.x-p0.x));
				udyflector_param_blk.InvalidateUI();

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

static BOOL needToss;

int CreateUDynaDeflObjectProc::proc(HWND hwnd,int msg,int point,int flag,
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
				   		UspObj->EndEditParams( (IObjParam*)createInterface, 0, NULL);
              createInterface->GetMacroRecorder()->EmitScript();
						if (CloudNode) {
							theHold.Suspend();
							DeleteReference(0);
							theHold.Resume();
							}

						// new object
						CreateNewObject();   // creates UniObj
						}

					needToss = theHold.GetGlobalPutCount()!=lastPutCount;

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
		UspObj->EndEditParams( (IObjParam*)createInterface,0,NULL);
		theHold.Cancel();	 // deletes both the Cloudera and target.
		// Bayboro: the following single line is a fix for #262308
		macroRec->Cancel();
		if (needToss) GetSystemSetting(SYSSET_CLEAR_UNDO);
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

BOOL UnjPickOperand::Filter(INode *node)
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

BOOL UnjPickOperand::HitTest(
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

void UDynaDeflObject::ShowName()
{TSTR name; 
 FormatName(name= TSTR(GetString(IDS_EP_ITEMSTR)) + (custnode ? custname : TSTR(GetString(IDS_EP_NONE))));
SetWindowText(GetDlgItem(hParams, IDC_EP_PICKNAME), name);
}

BOOL UnjPickOperand::Pick(IObjParam *ip,ViewExp *vpt)
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
	theHold.Put(new CreateUDynaDeflPickNode(po,node));

//	po->custnode=node;
	if (po->custnode) po->ReplaceReference(CUSTNODE,node,TRUE);
	else po->MakeRefByID(FOREVER,CUSTNODE,node);	
	po->NotifyDependents(FOREVER, 0, REFMSG_CHANGE);
	theHold.Accept(GetString(IDS_EP_USPPICK));
	po->custname = TSTR(node->GetName());
	// Automatically check show result and do one update
	po->ShowName();	
	if (po->creating) {
		theCreateUDynaDeflObjectMode.JumpStart(ip,po);
		ip->SetCommandMode(&theCreateUDynaDeflObjectMode);
		ip->RedrawViews(ip->GetTime());
		return FALSE;
	} else {
		return TRUE;
		}
	}

void UnjPickOperand::EnterMode(IObjParam *ip)
	{
	ICustButton *iBut;
	iBut=GetICustButton(GetDlgItem(po->hParams,IDC_EP_PICKBUTTON));
	if (iBut) iBut->SetCheck(TRUE);
	ReleaseICustButton(iBut);
	GetCOREInterface()->PushPrompt(GetString(IDS_AP_PICKMODE));
	}

void UnjPickOperand::ExitMode(IObjParam *ip)
	{
	ICustButton *iBut;
	iBut=GetICustButton(GetDlgItem(po->hParams,IDC_EP_PICKBUTTON));
	if (iBut) iBut->SetCheck(FALSE);
	ReleaseICustButton(iBut);
    GetCOREInterface()->PopPrompt();
	}

class UDynaDeflObjectDlgProc : public ParamMap2UserDlgProc {
	public:
		UDynaDeflObject *po;

		UDynaDeflObjectDlgProc(UDynaDeflObject *p) {po=p;}
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
		void Update(TimeValue t);
	};

void UDynaDeflObjectDlgProc::Update(TimeValue t)
{	po->ShowName();
	float size;
//	po->pblock->GetValue(PB_ICONSIZE,0,size,FOREVER);
	po->pblock2->GetValue(udyflectrobj_radius,0,size,FOREVER);
	TurnButton(po->hParams,IDC_EP_PICKBUTTON,(size>=0.01f));
}

BOOL UDynaDeflObjectDlgProc::DlgProc(
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
						{  theCreateUDynaDeflObjectMode.JumpStart(po->ip,po);
							po->ip->SetCommandMode(&theCreateUDynaDeflObjectMode);
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

UDynaDeflObject::UDynaDeflObject()
{
//	MakeRefByID(FOREVER, 0, 
//		CreateParameterBlock(descUDynaVer0, PBLOCK_LENGTH, CURRENT_VERSION));
//	assert(pblock);	

	pblock2 = NULL;
	UDynaDeflDesc.MakeAutoParamBlocks(this);
	assert(pblock2);

	int tpf=GetTicksPerFrame();
	int timeoff=100*tpf;

//	pblock->SetValue(PB_TIMEON,0,0);
//	pblock->SetValue(PB_TIMEOFF,0,100*timeoff);
//	pblock->SetValue(PB_AFFECTS,0,100.0f);
//	pblock->SetValue(PB_BOUNCE,0,1.0f);
//	pblock->SetValue(PB_BOUNCEVAR,0,0.0f);
//	pblock->SetValue(PB_CHAOS,0,0.0f);
//	pblock->SetValue(PB_FRICTION,0,0.0f);
//	pblock->SetValue(PB_INHERIT,0,100.0f);
//	pblock->SetValue(PB_MASS,0,1.0f);
//	pblock->SetValue(PB_MASSUNITS,0,0);
//	pblock->SetValue(PB_ICONSIZE,0,0.0f);

	pblock2->SetValue(udyflectrobj_timeon,0,0);
	pblock2->SetValue(udyflectrobj_timeoff,0,timeoff);
	pblock2->SetValue(udyflectrobj_affects,0,1.0f);
	pblock2->SetValue(udyflectrobj_bounce,0,1.0f);
	pblock2->SetValue(udyflectrobj_bouncevar,0,0.0f);
	pblock2->SetValue(udyflectrobj_chaos,0,0.0f);
	pblock2->SetValue(udyflectrobj_friction,0,0.0f);
//	pblock2->SetValue(udyflectrobj_velocity,0,1.0f);
	pblock2->SetValue(udyflectrobj_mass,0,1.0f);
	pblock2->SetValue(udyflectrobj_massunits,0,0);
	pblock2->SetValue(udyflectrobj_radius,0,0.0f);

	ffdata.FlectForce = Zero;
	ffdata.ApplyAt = Zero;
	ffdata.Num = 0;	
	dmesh=NULL;
	vnorms=NULL;
	fnorms=NULL;
	srand(lastrnd=12345);
	t=99999;
	custnode=NULL;
	custname=TSTR(_T(" "));
	nv=0;nf=0;
	ctime=99999;
	macroRec->Disable();

//watje create a new ref to our collision engine
	CollisionMesh *colm = (CollisionMesh*)CreateInstance(REF_MAKER_CLASS_ID, MESH_COLLISION_ID);
	if (colm)
	{
		pblock2->SetValue(udyflectrobj_collider,0,(ReferenceTarget*)colm);
	}
	macroRec->Enable();
}

Modifier *UDynaDeflObject::CreateWSMMod(INode *node)
	{
	return new UDynaDeflMod(node,this);
	}

RefTargetHandle UDynaDeflObject::Clone(RemapDir& remap) 
	{
	UDynaDeflObject* newob = new UDynaDeflObject();	
	newob->ReplaceReference(0,pblock2->Clone(remap));
	if (custnode) 
		newob->ReplaceReference(CUSTNODE,custnode);
	newob->custname=custname;
	newob->dmesh=NULL;
	newob->vnorms=NULL;
	newob->fnorms=NULL;
	BaseClone(this, newob, remap);
	return newob;
	}

void UDynaDeflObject::BeginEditParams(
		IObjParam *ip,ULONG flags,Animatable *prev)
{	this->ip = ip;
	if (!hSot)
		hSot = ip->AddRollupPage( 
			hInstance, 
			MAKEINTRESOURCE(IDD_SW_DESC_BOTH_LEGACY),
			DefaultSOTProc,
			GetString(IDS_EP_TOP), 
			(LPARAM)ip,APPENDROLL_CLOSED);

	SimpleWSMObject2::BeginEditParams(ip,flags,prev);

	if (flags&BEGIN_EDIT_CREATE) 
		creating = TRUE;
	else 
		creating = FALSE;

	UDynaDeflDesc.BeginEditParams(ip, this, flags, prev);
	udyflector_param_blk.SetUserDlgProc(new UDynaDeflObjectDlgProc(this));

/*
	if (pmapParam) {		
		// Left over
		pmapParam->SetParamBlock(pblock);
		SetWindowLongPtr(hSot,GWLP_USERDATA,(LONG_PTR)this);
	} else {		
		hSot = ip->AddRollupPage( 
			hInstance, 
			MAKEINTRESOURCE(IDD_SW_DESC_BOTH),
			DefaultSOTProc,
			GetString(IDS_EP_TOP), 
			(LPARAM)ip,APPENDROLL_CLOSED);

		// Gotta make a new one.
		pmapParam = CreateCPParamMap(
			descUDynaParam,PARAMDESC_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_AP_UDYNADEFL),
			GetString(IDS_EP_PARAMETERS),
			0);
		ip->RegisterDlgWnd(hSot);
		}
		if (pmapParam)
			pmapParam->SetUserDlgProc(new UDynaDeflObjectDlgProc(this));
*/
	}

void UDynaDeflObject::EndEditParams(
		IObjParam *ip, ULONG flags,Animatable *next)
{		
	SimpleWSMObject2::EndEditParams(ip,flags,next);

	UDynaDeflDesc.EndEditParams(ip, this, flags, next);

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
	ip->ClearPickMode();
	this->ip = NULL;
	ip = NULL;
	creating = FALSE;
}

void UDynaDeflObject::MapKeys(TimeMap *map,DWORD flags)
{	Animatable::MapKeys(map,flags);
	TimeValue TempTime;
// remap values
//	pblock->GetValue(PB_TIMEON,0,TempTime,FOREVER);
	pblock2->GetValue(udyflectrobj_timeon,0,TempTime,FOREVER);
	TempTime=map->map(TempTime);
//	pblock->SetValue(PB_TIMEON,0,TempTime);
	pblock2->SetValue(udyflectrobj_timeon,0,TempTime);
//	pblock->GetValue(PB_TIMEOFF,0,TempTime,FOREVER);
	pblock2->GetValue(udyflectrobj_timeoff,0,TempTime,FOREVER);
	TempTime=map->map(TempTime);
//	pblock->SetValue(PB_TIMEOFF,0,TempTime);
	pblock2->SetValue(udyflectrobj_timeoff,0,TempTime);
}  

void UDynaDeflObject::BuildMesh(TimeValue t)
	{
	ivalid = FOREVER;
	float length,l,h2,h3,h4;
//	pblock->GetValue(PB_ICONSIZE,t,length,ivalid);
	pblock2->GetValue(udyflectrobj_radius,t,length,ivalid);
	l=length*0.5f;
	h2=l*0.5f;
	h3=h2*0.15f;
	h4=h2*0.25f;

	mesh.setNumVerts(23);
	mesh.setNumFaces(21);

	mesh.setVert(0,Point3( l, l, l));
	mesh.setVert(1,Point3( l, l,-l));
	mesh.setVert(2,Point3( l,-l, l));
	mesh.setVert(3,Point3( l,-l,-l));
	mesh.setVert(4,Point3(-l, l, l));
	mesh.setVert(5,Point3(-l, l,-l));
	mesh.setVert(6,Point3(-l,-l, l));
	mesh.setVert(7,Point3(-l,-l,-l));

	mesh.setVert( 8, Point3(0.0f,0.0f,l));
	mesh.setVert( 9, Point3(0.0f,  h2,l+h2));
	mesh.setVert(10, Point3(0.0f, -h2,l+h2));
	mesh.setVert(11, Point3(0.0f,  h2+h3,l+h2));
	mesh.setVert(12, Point3(0.0f,  h2,l+h2+h3));
	mesh.setVert(13, Point3(0.0f, -h2,l+h2-h3));
	mesh.setVert(14, Point3(0.0f, -h2+h3,l+h2));

	mesh.setVert(15, Point3(0.0f, h4, -l));
	mesh.setVert(16, Point3(0.0f, h4, -h2-l));
	mesh.setVert(17, Point3(0.0f, h4+h3, -h2-l));
	mesh.setVert(18, Point3(0.0f, 0.0f, -h2-h3-h3-l));
	mesh.setVert(19, Point3(0.0f,-h4-h3, -h2-l));
	mesh.setVert(20, Point3(0.0f,-h4, -h2-l));
	mesh.setVert(21, Point3(0.0f,-h4, -l));
	mesh.setVert(22, Point3(0.0f,0.0f,-h4-l));

	mesh.faces[0].setVerts(1,0,2);
	mesh.faces[0].setEdgeVisFlags(1,1,0);
	mesh.faces[0].setSmGroup(0);
	mesh.faces[1].setVerts(2,3,1);
	mesh.faces[1].setEdgeVisFlags(1,1,0);
	mesh.faces[1].setSmGroup(0);
	mesh.faces[2].setVerts(2,0,4);
	mesh.faces[2].setEdgeVisFlags(1,1,0);
	mesh.faces[2].setSmGroup(1);
	mesh.faces[3].setVerts(4,6,2);
	mesh.faces[3].setEdgeVisFlags(1,1,0);
	mesh.faces[3].setSmGroup(1);
	mesh.faces[4].setVerts(3,2,6);
	mesh.faces[4].setEdgeVisFlags(1,1,0);
	mesh.faces[4].setSmGroup(2);
	mesh.faces[5].setVerts(6,7,3);
	mesh.faces[5].setEdgeVisFlags(1,1,0);
	mesh.faces[5].setSmGroup(2);
	mesh.faces[6].setVerts(7,6,4);
	mesh.faces[6].setEdgeVisFlags(1,1,0);
	mesh.faces[6].setSmGroup(3);
	mesh.faces[7].setVerts(4,5,7);
	mesh.faces[7].setEdgeVisFlags(1,1,0);
	mesh.faces[7].setSmGroup(3);
	mesh.faces[8].setVerts(4,0,1);
	mesh.faces[8].setEdgeVisFlags(1,1,0);
	mesh.faces[8].setSmGroup(4);
	mesh.faces[9].setVerts(1,5,4);
	mesh.faces[9].setEdgeVisFlags(1,1,0);
	mesh.faces[9].setSmGroup(4);
	mesh.faces[10].setVerts(1,3,7);
	mesh.faces[10].setEdgeVisFlags(1,1,0);
	mesh.faces[10].setSmGroup(5);
	mesh.faces[11].setVerts(7,5,1);
	mesh.faces[11].setEdgeVisFlags(1,1,0);
	mesh.faces[11].setSmGroup(5);

	mesh.faces[12].setEdgeVisFlags(1,0,1);
	mesh.faces[12].setSmGroup(1);
	mesh.faces[12].setVerts(8,10,9);	
	mesh.faces[13].setEdgeVisFlags(1,0,1);
	mesh.faces[13].setSmGroup(1);
	mesh.faces[13].setVerts(10,13,14);	
	mesh.faces[14].setEdgeVisFlags(1,0,1);
	mesh.faces[14].setSmGroup(1);
	mesh.faces[14].setVerts(9,12,11);
	mesh.faces[15].setEdgeVisFlags(1,0,1);
	mesh.faces[15].setSmGroup(1);
	mesh.faces[15].setVerts(15,16,22);	
	mesh.faces[16].setEdgeVisFlags(0,0,0);
	mesh.faces[16].setSmGroup(1);
	mesh.faces[16].setVerts(16,20,22);	
	mesh.faces[17].setEdgeVisFlags(1,1,0);
	mesh.faces[17].setSmGroup(1);
	mesh.faces[17].setVerts(20,21,22);	
	mesh.faces[18].setEdgeVisFlags(1,1,0);
	mesh.faces[18].setSmGroup(1);
	mesh.faces[18].setVerts(16,17,18);	
	mesh.faces[19].setEdgeVisFlags(0,0,0);
	mesh.faces[19].setSmGroup(1);
	mesh.faces[19].setVerts(16,18,20);	
	mesh.faces[20].setEdgeVisFlags(1,1,0);
	mesh.faces[20].setSmGroup(1);
	mesh.faces[20].setVerts(18,19,20);	

	mesh.InvalidateGeomCache();
	}

void UDynaDeflObject::InvalidateUI() 
{
//	if (pmapParam) pmapParam->Invalidate();
	udyflector_param_blk.InvalidateUI(pblock2->LastNotifyParamID());
}

ParamDimension *UDynaDeflObject::GetParameterDim(int pbIndex) 
{
	switch (pbIndex) {		
//		case PB_TIMEON:
		case udyflectrobj_timeon:
				 return stdTimeDim;
//		case PB_TIMEOFF:
		case udyflectrobj_timeoff:
				 return stdTimeDim;
//		case PB_AFFECTS:
		case udyflectrobj_affects:
				 return stdPercentDim;
//		case PB_BOUNCEVAR:
		case udyflectrobj_bouncevar:
				 return stdPercentDim;
//		case PB_CHAOS:
		case udyflectrobj_chaos:
				 return stdPercentDim;
		default: return defaultDim;
		}
	}

/*
TSTR UDynaDeflObject::GetParameterName(int pbIndex) 
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
		case PB_ICONSIZE:		return GetString(IDS_EP_ICONSIZE);
		case PB_MASS:			return GetString(IDS_EP_MASS);
		case PB_MASSUNITS:		return GetString(IDS_EP_MASSUNITS);
		default:				return TSTR(_T(""));
		}
	}
*/

//--- DeflectMod methods -----------------------------------------------

UDynaDeflMod::UDynaDeflMod(INode *node,UDynaDeflObject *obj)
	{	
//	MakeRefByID(FOREVER,SIMPWSMMOD_OBREF,obj);
	MakeRefByID(FOREVER,SIMPWSMMOD_NODEREF,node);	
		pblock = NULL;
	obRef=NULL;
	}

Interval UDynaDeflMod::GetValidity(TimeValue t) 
	{
	if (obRef && nodeRef) {
		Interval valid = FOREVER;
		Matrix3 tm;
		float f;		
		UDynaDeflObject *obj = (UDynaDeflObject*)GetWSMObject(t);
		TimeValue TempT;
//		obj->pblock->GetValue(PB_TIMEOFF,t,TempT,valid);
//		obj->pblock->GetValue(PB_AFFECTS,t,f,valid);
//		obj->pblock->GetValue(PB_BOUNCE,t,f,valid);
//		obj->pblock->GetValue(PB_BOUNCEVAR,t,f,valid);
//		obj->pblock->GetValue(PB_CHAOS,t,f,valid);
//		obj->pblock->GetValue(PB_FRICTION,t,f,valid);
//		obj->pblock->GetValue(PB_INHERIT,t,f,valid);
//		obj->pblock->GetValue(PB_MASS,t,f,valid);
		obj->pblock->GetValue(udyflectrobj_timeoff,t,TempT,valid);
		obj->pblock->GetValue(udyflectrobj_affects,t,f,valid);
		obj->pblock->GetValue(udyflectrobj_bounce,t,f,valid);
		obj->pblock->GetValue(udyflectrobj_bouncevar,t,f,valid);
		obj->pblock->GetValue(udyflectrobj_chaos,t,f,valid);
		obj->pblock->GetValue(udyflectrobj_friction,t,f,valid);
		obj->pblock->GetValue(udyflectrobj_velocity,t,f,valid);
		obj->pblock->GetValue(udyflectrobj_mass,t,f,valid);
		tm = nodeRef->GetObjectTM(t,&valid);
		
		return valid;
	} else {
		return FOREVER;
		}
	}

class UDynaDeflDeformer : public Deformer {
	public:		
		Point3 Map(int i, Point3 p) {return p;}
	};
static UDynaDeflDeformer UDynaddeformer;

Deformer& UDynaDeflMod::GetDeformer(
		TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
	{
	return UDynaddeformer;
	}

RefTargetHandle UDynaDeflMod::Clone(RemapDir& remap) 
	{
	UDynaDeflMod *newob = new UDynaDeflMod(nodeRef,(UDynaDeflObject*)obRef);	
	newob->SimpleWSMModClone(this);
	BaseClone(this, newob, remap);
	return newob;
	}

void UDynaDeflMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node)
	{
	ParticleObject *obj = GetParticleInterface(os->obj);
	if (obj) {
		deflect.obj  = (UDynaDeflObject*)GetWSMObject(t);
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
		deflect.totalforce=Zero;
		deflect.applyat=Zero;
		deflect.totalnumber=0;
		TimeValue tmpt=GetCOREInterface()->GetTime();
		if (deflect.obj->ctime!=tmpt)
		{ deflect.obj->ctime=tmpt;
//		  if ((deflect.curtime==NoAni)||(deflect.curtime!=t))
		  deflect.obj->ffdata.FlectForce=deflect.totalforce;
		  deflect.obj->ffdata.ApplyAt=deflect.applyat;
		  deflect.obj->ffdata.Num=deflect.totalnumber;
		}
//		deflect.curtime=NoAni;
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

void AddMesh(UDynaDeflObject *obj,TriObject *triOb,Matrix3 tm,BOOL nottop)
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

Object *UDynaDeflField::GetSWObject()
{ return obj;
}

UDynaDeflObject::~UDynaDeflObject()
{	
	DeleteAllRefsFromMe();
	if(pblock2)
	{
		ReferenceTarget *rt;
		pblock2->GetValue(udyflectrobj_collider,t,rt,FOREVER);
//		if (rt)
//			delete rt;
	}
	pblock2 = NULL;
	if (vnorms) 
		delete[] vnorms;
	if (fnorms) 
		delete[] fnorms;
	if (dmesh) 
		delete dmesh;
}

BOOL UDynaDeflField::CheckCollision(TimeValue t,Point3 &inp,Point3 &vel,float dt,int index,float *ct, BOOL UpdatePastCollide)
{ 	if (badmesh) 
		return(0);

	Point3 iw;

//watje need to get a pointer to out collider engine
//	ReferenceTarget *rt;
//	obj->pblock2->GetValue(udyflectrobj_collider,t,rt,obj->tmValid);
//	colm = (CollisionMesh *) rt;
// bayboro: the proper way to get a pointer to a our collider engine
// is to create CollisionSphere for each UDynaDeflField object
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
		obj->invtm=Inverse(obj->tm);
		obj->invtmNoTrans=Inverse(obj->tmNoTrans);

		obj->pblock2->GetValue(udyflectrobj_bounce,t,bounce,FOREVER);
		if (bounce < 0.0f) bounce = 0.0f;
		obj->pblock2->GetValue(udyflectrobj_bouncevar,t,bvar,FOREVER);
		obj->pblock2->GetValue(udyflectrobj_chaos,t,chaos,FOREVER);
		obj->pblock2->GetValue(udyflectrobj_velocity,t,vinher,FOREVER);
		obj->pblock2->GetValue(udyflectrobj_friction,t,friction,FOREVER);
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
		obj->ptm=obj->custnode->GetObjectTM(t+(TimeValue)dt,&tmpValid);
		obj->dvel=(Zero*obj->ptm-Zero*obj->tm)/dt;
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
					}
					if ((triOb)&&(triOb!=pobj)) triOb->DeleteThis();
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

    if (curtime!=t)
	{	totalforce=Zero;
		applyat=Zero;
		totalnumber=0;
		curtime=t;
/*		obj->ffdata.FlectForce=totalforce;
		obj->ffdata.ApplyAt=applyat;
		obj->ffdata.Num=totalnumber;*/
	}
	
	float K=(float)GetMasterScale(UNITS_CENTIMETERS);
	float stepsize=dt;
	Point3 invel=vel;

	TimeValue startt,endt;
//	obj->pblock->GetValue(PB_TIMEON,t,startt,FOREVER);
//	obj->pblock->GetValue(PB_TIMEOFF,t,endt,FOREVER);
	obj->pblock2->GetValue(udyflectrobj_timeon,t,startt,FOREVER);
	obj->pblock2->GetValue(udyflectrobj_timeoff,t,endt,FOREVER);
	if ((t<startt)||(t>endt))
	{	obj->lastrnd=rand();
		return FALSE;
	}

	float affectsthisportion;
//	obj->pblock->GetValue(PB_AFFECTS,t,affectsthisportion,FOREVER);
	obj->pblock2->GetValue(udyflectrobj_affects,t,affectsthisportion,FOREVER);
//	affectsthisportion *= 0.01f;
    srand(obj->lastrnd);
	if (RND01()>affectsthisportion)
	{	obj->lastrnd=rand();
		return FALSE;
	}

if (!colm)
{
	float pvel;
	Point3 NVrelL,Vrel,VrelL,pos;

	pos=inp*obj->invtm;
	Vrel=vel-obj->dvel;
	pvel=Length(Vrel);
	VrelL=Vrel*obj->invtmNoTrans;
	NVrelL=Normalize(VrelL);
	Ray ray;
	ray.dir=NVrelL;
	ray.p=pos;
	float at;Point3 norm;
	int kfound=RayIntersectP(ray,at,norm,obj->dmesh,obj->vnorms,obj->fnorms);
	if (!kfound)
	{	obj->lastrnd=rand();
		return FALSE;
	}

	Point3 id;

	iw=(id=pos+at*NVrelL)*obj->tm;
	float delta=Length(iw-inp);
	if (delta>dt*pvel)
	{	obj->lastrnd=rand();
		return FALSE;
	}

//	obj->pblock->GetValue(PB_BOUNCE,t,bounce,FOREVER);
//	obj->pblock->GetValue(PB_BOUNCEVAR,t,bvar,FOREVER);
//	obj->pblock->GetValue(PB_CHAOS,t,chaos,FOREVER);
//	obj->pblock->GetValue(PB_INHERIT,t,vinher,FOREVER);
//	obj->pblock->GetValue(PB_FRICTION,t,friction,FOREVER);
//	obj->pblock2->GetValue(udyflectrobj_bounce,t,bounce,FOREVER);
//	obj->pblock2->GetValue(udyflectrobj_bouncevar,t,bvar,FOREVER);
//	obj->pblock2->GetValue(udyflectrobj_chaos,t,chaos,FOREVER);
//	obj->pblock2->GetValue(udyflectrobj_velocity,t,vinher,FOREVER);
//	obj->pblock2->GetValue(udyflectrobj_friction,t,friction,FOREVER);

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
		// Martell 4/14/01: Fix for order of ops bug.
		float ztmp=RND11(); float ytmp=RND11(); float xtmp=RND11();
		Point3 d=Point3(xtmp,ytmp,ztmp);
		Point3 c=Normalize(vel^d);
		RotateOnePoint(&vel.x,&Zero.x,&c.x,theta);
	}
	if (vinher>0.0f)
	{	Point3 dvel=obj->dvel*vinher; 
		vel = vel + friction*dvel + (1-friction)*DotProd(dvel,wnorm)*wnorm;
	}

	if (UpdatePastCollide)
	{	inp = iw + (dt-dti)*vel;
		if (ct) 
			(*ct) = dt;
	}
	else
	{	inp = iw;
		if (ct) 
			(*ct) = dti;
	}
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

//	AddInheritVelocity(bnorm, frict, vel, inheritedVel,vinher, normLen, fricLen);
	// Bayboro: end of the new scheme

	// Bayboro: old scheme
//	vel = bnorm*(bounce*rvariation) + frict*(1.0f-(friction*rchaos)) ;
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
//	vel +=  (inheritedVel * vinher);

	iw = hitpoint;
}

	applyat = iw;

// get physical parameters
	float mass;
	int massunits;
//	obj->pblock->GetValue(PB_MASS,t,mass,FOREVER);
//	obj->pblock->GetValue(PB_MASSUNITS,t,massunits,FOREVER);
	obj->pblock2->GetValue(udyflectrobj_mass,t,mass,FOREVER);
	obj->pblock2->GetValue(udyflectrobj_massunits,t,massunits,FOREVER);
// compensate for units of measure
	switch(massunits)
	{	case 0: mass*=0.001f; break;
		case 1: break;
		case 2: mass*=0.454f; break;
	}
// increment physical property params
// put information into parameter block
	if (t==obj->ctime)
	{ totalnumber+=1;
	  totalforce += (invel-vel)*K*mass/stepsize;
      obj->ffdata.FlectForce += totalforce;
	  obj->ffdata.ApplyAt = applyat;
	  obj->ffdata.Num =+ totalnumber;
	}
	obj->lastrnd=rand();
	return TRUE;
}

RefTargetHandle UDynaDeflObject::GetReference(int i)
{	switch(i) {
//		case PBLK: return(RefTargetHandle)pblock2;
		case PBLK: return SimpleWSMObject2::GetReference(i);
		case CUSTNODE: return (RefTargetHandle)custnode;
		default: return NULL;
		}
	}

void UDynaDeflObject::SetReference(int i, RefTargetHandle rtarg) { 
	switch(i) {
//		case PBLK: pblock2=(IParamBlock2*)rtarg; return;
		case PBLK: SimpleWSMObject2::SetReference(i, rtarg); return;
		case CUSTNODE: custnode = (INode *)rtarg; return;
		}
	}

RefResult UDynaDeflObject::NotifyRefChanged( 
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

IOResult UDynaDeflObject::Save(ISave *isave)
	{
	isave->BeginChunk(USPDEF_CUSTNAME_CHUNK);		
	isave->WriteWString(custname);
	isave->EndChunk();
	return IO_OK;
	}

class UDynaflectObjectLoad : public PostLoadCallback 
{
	public:
		UDynaDeflObject *n;
		UDynaflectObjectLoad(UDynaDeflObject *ns) {n = ns;}
		void proc(ILoad *iload) 
		{  
			ReferenceTarget *rt;
			Interval iv;
			n->pblock2->GetValue(udyflectrobj_collider,0,rt,iv);
			if (rt == NULL)
			{
				CollisionMesh *colm = (CollisionMesh*)CreateInstance(REF_MAKER_CLASS_ID, MESH_COLLISION_ID);
				if (colm)
					n->pblock2->SetValue(udyflectrobj_collider,0,(ReferenceTarget*)colm);
			}
			delete this; 
		} 
};

IOResult UDynaDeflObject::Load(ILoad *iload)
{
	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(usversions, NUM_OLDVERSIONS, &udyflector_param_blk, this, PBLOCK_REF_NO);
	iload->RegisterPostLoadCallback(plcb);
	iload->RegisterPostLoadCallback(new UDynaflectObjectLoad(this));

	custname = TSTR(_T(" "));

	IOResult res = IO_OK;

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
		if (res!=IO_OK)  return res;
	}
	return IO_OK;
}

FlectForces UDynaDeflObject::ForceData(TimeValue t)
{
//	pblock->GetValue(PB_TIMEON,t,ffdata.t1,FOREVER);
//	pblock->GetValue(PB_TIMEOFF,t,ffdata.t2,FOREVER);
	pblock2->GetValue(udyflectrobj_timeon,t,ffdata.t1,FOREVER);
	pblock2->GetValue(udyflectrobj_timeoff,t,ffdata.t2,FOREVER);
	return ffdata;
}
