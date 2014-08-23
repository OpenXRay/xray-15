/**********************************************************************
 *<
	FILE: udeflect.cpp

	DESCRIPTION: Turns Any Mesh Into a Deflector

	CREATED BY: Audrey Peterson

	HISTORY: 1/97

	PB2 ECP 3/14/00

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/
#include "suprprts.h"
#include "iparamm2.h"
#include "simpmod.h"
#include "simpobj.h"
#include "texutil.h"
#include "interpik.h"
#include "ICollision.h"
#include "macrorec.h"

#define PBLK		0
#define CUSTNODE 	1


static Class_ID UNIDEF_CLASS_ID(0x28497b68, 0);
static Class_ID UNIDEFMOD_CLASS_ID(0x4d456b2d, 0);


class VNormal {
	public:
		Point3 norm;
		DWORD smooth;
		VNormal *next;
		BOOL init;

		VNormal() {smooth=0;next=NULL;init=FALSE;norm=Point3(0,0,0);}
		VNormal(Point3 &n,DWORD s) {next=NULL;init=TRUE;norm=n;smooth=s;}
		~VNormal() {delete next;}
		void AddNormal(Point3 &n,DWORD s);
		Point3 &GetNormal(DWORD s);
		void Normalize();
	};

void VNormal::AddNormal(Point3 &n,DWORD s)
	{
	if (!(s&smooth) && init) {
		if (next) next->AddNormal(n,s);
		else {
			next = new VNormal(n,s);
			}
	} else {
		norm   += n;
		smooth |= s;
		init    = TRUE;
		}
	}

Point3 &VNormal::GetNormal(DWORD s)
	{
	if (smooth&s || !next) return norm;
	else return next->GetNormal(s);	
	}

void VNormal::Normalize()
	{
	VNormal *ptr = next, *prev = this;
	while (ptr) {
		if (ptr->smooth&smooth) {
			norm += ptr->norm;			
			prev->next = ptr->next;
			delete ptr;
			ptr = prev->next;
		} else {
			prev = ptr;
			ptr  = ptr->next;
			}
		}
	norm = ::Normalize(norm);
	if (next) next->Normalize();
	}

class UniPickOperand;

class UniDefObject : public SimpleWSMObject2 {	
	public:		
//		static IParamMap *pmapParam;
		static IObjParam *ip;
		static HWND hSot;
		static HWND hParams;
					
		INode *custnode;
		int lastrnd;
		TimeValue t;
		TSTR custname;
		UniDefObject();
		~UniDefObject();
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
		Class_ID ClassID() {return UNIDEF_CLASS_ID;}		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_AP_UNIDEFLECTOR);}
				
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
		void SetRandSeed(int seed);
	};

class UniPickOperand : 
		public PickModeCallback,
		public PickNodeCallback {
	public:		
		UniDefObject *po;
		
		UniPickOperand() {po=NULL;}

		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);

		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);

		BOOL RightClick(IObjParam *ip, ViewExp *vpt) { return TRUE; }
		BOOL Filter(INode *node);
		
		PickNodeCallback *GetFilter() {return this;}
	};

class CreateUDeflectPickNode : public RestoreObj {
	public:   		
		UniDefObject *obj;
		INode *oldn;
		CreateUDeflectPickNode(UniDefObject *o, INode *n) {
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
	{TSTR name=TSTR(GetString(IDS_AP_OBJECTSTR)) + (oldn ? obj->custname : TSTR(GetString(IDS_AP_NONE)));
	SetWindowText(GetDlgItem(obj->hParams, IDC_PCLOUD_PCUST), name);
		}
			}
		TSTR Description() {return GetString(IDS_AP_UDEFPICK);}
	};

//--- ClassDescriptor and class vars ---------------------------------

IObjParam *UniDefObject::ip        = NULL;
//IParamMap *UniDefObject::pmapParam = NULL;
HWND       UniDefObject::hSot      = NULL;
HWND       UniDefObject::hParams   = NULL;
BOOL	   UniDefObject::creating  = FALSE;
UniPickOperand UniDefObject::pickCB;

class UniDeflectorClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) { return new UniDefObject;}
	const TCHAR *	ClassName() {return GetString(IDS_AP_UNIDEFLECTOR_CLASS);}
	SClass_ID		SuperClassID() {return WSM_OBJECT_CLASS_ID; }
	Class_ID		ClassID() {return UNIDEF_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_EP_SW_DEFLECTORS);}
	int 			BeginCreate(Interface *i);
	int 			EndCreate(Interface *i);

	// Hardwired name, used by MAX Script as unique identifier
	const TCHAR*	InternalName()				{ return _T("UDeflector"); }
	HINSTANCE		HInstance()					{ return hInstance; }
	};

static UniDeflectorClassDesc UnideflectDesc;
ClassDesc* GetUniDefDesc() {return &UnideflectDesc;}

//--- DeflectMod -----------------------------------------------------

class UniDeflectorField : public CollisionObject {
	public:		
		UniDeflectorField()
		{
//	 		colm = (CollisionMesh*)CreateInstance(REF_MAKER_CLASS_ID, MESH_COLLISION_ID);
			srand(8741354);
			for (int i =0;i < 500; i++)
			{
				randomFloat[i] = (float)rand()/(float)RAND_MAX;
			}	
			colm=NULL;
		}


		void DeleteThis() 
			{	 
			if (colm) colm->DeleteThis();
			colm=NULL;
			delete this;

			}

		~UniDeflectorField()
		{
			if (colm) colm->DeleteThis();
			colm=NULL;
		}

		float randomFloat[500];
		CollisionMesh *colm;

		float chaos,bounce,bvar,vinher,friction;

		UniDefObject *obj;
		INode *node;
		int badmesh;
		BOOL CheckCollision(TimeValue t,Point3 &pos, Point3 &vel, float dt, int index,float *ct,BOOL UpdatePastCollide);;
		Object *GetSWObject();
		void SetRandSeed(int seed) { if (obj != NULL) obj->lastrnd = seed; }
	};

class UniDeflectMod : public SimpleWSMMod {
	public:				
		UniDeflectorField deflect;

		UniDeflectMod() {}
		UniDeflectMod(INode *node,UniDefObject *obj);	


		// From Animatable
		void GetClassName(TSTR& s) {s= GetString(IDS_AP_UNIDEFMOD);}
		SClass_ID SuperClassID() {return WSM_CLASS_ID;}
		void DeleteThis() {delete this;}
		Class_ID ClassID() { return UNIDEFMOD_CLASS_ID;}
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_AP_UNIDEFBINDING);}

		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);

		// From SimpleWSMMod		
		Interval GetValidity(TimeValue t);		
		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);
	};

//--- ClassDescriptor and class vars ---------------------------------

class UniDeflectorModClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 0;}
	void *			Create(BOOL loading = FALSE) { return new UniDeflectMod;}
	const TCHAR *	ClassName() {return GetString(IDS_AP_UNIDEFMOD);}
	SClass_ID		SuperClassID() {return WSM_CLASS_ID; }
	Class_ID		ClassID() {return UNIDEFMOD_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
	};

static UniDeflectorModClassDesc UnideflectModDesc;
ClassDesc* GetUniDefModDesc() {return &UnideflectModDesc;}

enum 
{	udeflectrobj_params, 
}; 

enum 
{	udeflectrobj_bounce, 
	udeflectrobj_bouncevar,
	udeflectrobj_chaos,
	udeflectrobj_radius,
	udeflectrobj_friction,
	udeflectrobj_velocity,
	udeflectrobj_collider
};

#define PBLOCK_REF_NO	0

static ParamBlockDesc2 udeflector_param_blk 
(	udeflectrobj_params, 
	_T("UDeflectorParameters"),  
	0, 
	&UnideflectDesc, 
	P_AUTO_CONSTRUCT + P_AUTO_UI, 
	PBLOCK_REF_NO, 

	//rollout
	IDD_AP_UNIVDEFL, IDS_RB_PARAMETERS, 0, 0, NULL, 

	// params
	udeflectrobj_bounce,	_T("bounce"),		TYPE_FLOAT,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_RB_BOUNCE,
		p_default,			1.0f,
		p_range,			0.0f,65535.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_SPHDEFLECT_BOUNCE,		IDC_SPHDEFLECT_BOUNCESPIN, 0.01f,
		end,

	udeflectrobj_bouncevar, _T("bouncevar"),	TYPE_PCNT_FRAC,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_RB_BVAR,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_SPHDEFLECT_BOUNCEVAR,	IDC_SPHDEFLECT_BOUNCEVARSPIN, 1.0f,
		end,

	udeflectrobj_chaos,		_T("chaos"),		TYPE_PCNT_FRAC,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_RB_CHAOS,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_SPHDEFLECT_CHAOS,		IDC_SPHDEFLECT_CHAOSSPIN,	1.0f,
		end,

	udeflectrobj_radius,	_T("radius"),		TYPE_FLOAT,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_AP_ICONSIZE,
		p_default,			0.0f,
		p_ms_default,		10.0f,
		p_range,			0.0f,9999999.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_UNIVERSE,	IDC_SPHDEFLECT_RADIUS,		IDC_SPHDEFLECT_RADIUSSPIN,	SPIN_AUTOSCALE,
		end,

	udeflectrobj_friction,	_T("friction"),		TYPE_PCNT_FRAC,		P_ANIMATABLE + P_RESET_DEFAULT,		IDS_AP_FRICTION,
		p_default,			0.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_SPHDEFLECT_FRICTION,	IDC_SPHDEFLECT_FRICTIONSPIN, 1.0f,
		end,

	udeflectrobj_velocity,	_T("inheritVelocity"),	TYPE_FLOAT,	P_ANIMATABLE + P_RESET_DEFAULT,		IDS_AP_VELOCITY,
		p_default,			1.0f,
		p_range,			0.0f,100.0f, 
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT,		IDC_SPHDEFLECT_VEL,			IDC_SPHDEFLECT_VELSPIN,		0.01f,
		end,

//watje ref to hold the collision engine
	udeflectrobj_collider,  _T(""),		TYPE_REFTARG, 	0, 	0, 
		end, 

	end
);

/*
#define PB_BOUNCE	 0
#define PB_BVAR		 1
#define PB_CHAOS	 2
#define PB_RADIUS	 3
#define PB_FRICTION	 4
#define PB_VELOCITY	 5

static ParamUIDesc descUniParam[] = {
	// Bounce
	ParamUIDesc(
		PB_BOUNCE,
		EDITTYPE_FLOAT,
		IDC_SPHDEFLECT_BOUNCE,IDC_SPHDEFLECT_BOUNCESPIN,
		0.0f, 9999999.0f,
		0.01f),
	
	// Bounce Var
	ParamUIDesc(
		PB_BVAR,
		EDITTYPE_FLOAT,
		IDC_SPHDEFLECT_BOUNCEVAR,IDC_SPHDEFLECT_BOUNCEVARSPIN,
		0.0f,100.0f,
		1.0f,
		stdPercentDim),
	
	// Chaos
	ParamUIDesc(
		PB_CHAOS,
		EDITTYPE_FLOAT,
		IDC_SPHDEFLECT_CHAOS,IDC_SPHDEFLECT_CHAOSSPIN,
		0.0f,100.0f,
		1.0f,
		stdPercentDim),

	// Deflector Friction
	ParamUIDesc(
		PB_FRICTION,
		EDITTYPE_FLOAT,
		IDC_SPHDEFLECT_FRICTION,IDC_SPHDEFLECT_FRICTIONSPIN,
		0.0f, 100.0f,
		1.0f,
		stdPercentDim),

	// Radius
	ParamUIDesc(
		PB_RADIUS,
		EDITTYPE_UNIVERSE,
		IDC_SPHDEFLECT_RADIUS,IDC_SPHDEFLECT_RADIUSSPIN,
		0.0f, 9999999.0f,
		SPIN_AUTOSCALE),

	// Velocity Inheritance
	ParamUIDesc(
		PB_VELOCITY,
		EDITTYPE_FLOAT,
		IDC_SPHDEFLECT_VEL,IDC_SPHDEFLECT_VELSPIN,
		0.0f, 1000.0f,SPIN_AUTOSCALE),
	};
*/

#define PARAMDESC_UNILENGTH	6

ParamBlockDescID descUniVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE,	0 },	//Bounce
	{ TYPE_FLOAT, NULL, FALSE,	1 },	//Bounce Var
	{ TYPE_FLOAT, NULL, TRUE,	2 },	//Chaos
	{ TYPE_FLOAT, NULL, FALSE,	3 },	//Radius
	{ TYPE_FLOAT, NULL, TRUE,	4 },	//Deflector Friction
	{ TYPE_FLOAT, NULL, TRUE,	5 }		//Velocity Inheritance
};

#define PBLOCK_UNILENGTH	6

//#define CURRENT_VERSION	0
#define NUM_OLDVERSIONS	1

// Array of old ParamBlock Ed. 1 versions
static ParamVersionDesc versions[] = 
{
	ParamVersionDesc (descUniVer0, 6, 0),
};


//--- Universal Deflect object methods -----------------------------------------
class CreateUniObjectProc : public MouseCallBack,ReferenceMaker {
	private:
		IObjParam *ip;
		void Init(IObjParam *i) {ip=i;}
		CreateMouseCallBack *createCB;	
		INode *CloudNode;
		UniDefObject *UniObj;
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
		
		CreateUniObjectProc()
			{
			ignoreSelectionChange = FALSE;
			}
		int createmethod(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
		int proc( HWND hwnd, int msg, int point, int flag, IPoint2 m );
	};


#define CID_CREATEUniObjectMODE	CID_USER + 12

class CreateUniObjectMode : public CommandMode {		
	public:		
		CreateUniObjectProc proc;
		IObjParam *ip;
		UniDefObject *obj;
		void Begin( IObjCreate *ioc, ClassDesc *desc ) { proc.Begin( ioc, desc ); }
		void End() { proc.End(); }
		void JumpStart(IObjParam *i,UniDefObject*o);

		int Class() {return CREATE_COMMAND;}
		int ID() { return CID_CREATEUniObjectMODE; }
		MouseCallBack *MouseProc(int *numPoints) {*numPoints = 10000; return &proc;}
		ChangeForegroundCallback *ChangeFGProc() {return CHANGE_FG_SELECTED;}
		BOOL ChangeFG( CommandMode *oldMode ) { return (oldMode->ChangeFGProc() != CHANGE_FG_SELECTED); }
		void EnterMode() 
		{ GetCOREInterface()->PushPrompt(GetString(IDS_AP_CREATEMODE));
		  SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR)));
		}
		void ExitMode() {GetCOREInterface()->PopPrompt();SetCursor(LoadCursor(NULL, IDC_ARROW));}
	};

static CreateUniObjectMode theCreateUniObjectMode;

RefResult CreateUniObjectProc::NotifyRefChanged(
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
		 	if ( UniObj && CloudNode==hTarget ) {
				// this will set camNode== NULL;
				theHold.Suspend();
				DeleteReference(0);
				theHold.Resume();
				goto endEdit;
				}
			// fall through

		case REFMSG_TARGET_DELETED:		
			if (UniObj && CloudNode==hTarget ) {
				endEdit:
				if (createInterface->GetCommandMode()->ID() == CID_STDPICK) 
				{ if (UniObj->creating) 
						{  theCreateUniObjectMode.JumpStart(UniObj->ip,UniObj);
						   createInterface->SetCommandMode(&theCreateUniObjectMode);
					    } 
				  else {createInterface->SetStdCommandMode(CID_OBJMOVE);}
				}
#ifdef _OSNAP
				UniObj->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
#ifndef NO_CREATE_TASK	// russom - 12/04/01
				UniObj->EndEditParams( (IObjParam*)createInterface, 0, NULL);
#endif
				UniObj  = NULL;				
				CloudNode    = NULL;
				CreateNewObject();	
				attachedToNode = FALSE;
				}
			break;		
		}
	return REF_SUCCEED;
	}

void CreateUniObjectProc::Begin( IObjCreate *ioc, ClassDesc *desc )
	{
	ip=(IObjParam*)ioc;
	createInterface = ioc;
	cDesc           = desc;
	attachedToNode  = FALSE;
	createCB        = NULL;
	CloudNode         = NULL;
	UniObj       = NULL;
	dostuff=0;
	CreateNewObject();
	}

void CreateUniObjectProc::CreateNewObject()
	{
	SuspendSetKeyMode();
	createInterface->GetMacroRecorder()->BeginCreate(cDesc);
	UniObj = (UniDefObject*)cDesc->Create();
	lastPutCount  = theHold.GetGlobalPutCount();
	
	// Start the edit params process
	if ( UniObj ) {
#ifndef NO_CREATE_TASK	// russom - 12/04/01
		UniObj->BeginEditParams( (IObjParam*)createInterface, BEGIN_EDIT_CREATE, NULL );
#endif
		UniObj->SetAFlag(A_OBJ_CREATING);
#ifdef _OSNAP
		UniObj->SetAFlag(A_OBJ_LONG_CREATE);
#endif
		}
	ResumeSetKeyMode();
	}

//LACamCreationManager::~LACamCreationManager
void CreateUniObjectProc::End()
{ if ( UniObj ) 
	{
  #ifdef _OSNAP
		UniObj->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
#ifndef NO_CREATE_TASK	// russom - 12/04/01
		UniObj->EndEditParams( (IObjParam*)createInterface, 
	                    	          END_EDIT_REMOVEUI, NULL);
#endif
		if ( !attachedToNode ) 
		{	// RB 4-9-96: Normally the hold isn't holding when this 
			// happens, but it can be in certain situations (like a track view paste)
			// Things get confused if it ends up with undo...
			theHold.Suspend(); 
			delete UniObj;
			UniObj = NULL;
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

void CreateUniObjectMode::JumpStart(IObjParam *i,UniDefObject *o)
	{
	ip  = i;
	obj = o;
	//MakeRefByID(FOREVER,0,svNode);
#ifndef NO_CREATE_TASK	// russom - 12/04/01
	obj->BeginEditParams(i,BEGIN_EDIT_CREATE,NULL);
#endif
	}


int UniDeflectorClassDesc::BeginCreate(Interface *i)
	{
	SuspendSetKeyMode();
	IObjCreate *iob = i->GetIObjCreate();
	theCreateUniObjectMode.Begin(iob,this);
	iob->PushCommandMode(&theCreateUniObjectMode);
	return TRUE;
	}

int UniDeflectorClassDesc::EndCreate(Interface *i)
	{
	ResumeSetKeyMode();
	theCreateUniObjectMode.End();
	i->RemoveMode(&theCreateUniObjectMode);
	macroRec->EmitScript();  // 10/00
	return TRUE;
	}

int CreateUniObjectProc::createmethod(
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
//				UniObj->pblock->SetValue(PB_RADIUS,0,0.01f);
				UniObj->pblock2->SetValue(udeflectrobj_radius,0,0.01f);
//				UniObj->pmapParam->Invalidate();
				udeflector_param_blk.InvalidateUI();
				break;

			case 1: {
				mat.IdentityMatrix();
				sp1 = m;
				p1  = vpt->SnapPoint(m,m,NULL,snapdim);
				Point3 center = (p0+p1)/float(2);
				mat.SetTrans(center);
//				UniObj->pblock->SetValue(PB_RADIUS,0,(float)fabs(p1.x-p0.x));
				UniObj->pblock2->SetValue(udeflectrobj_radius,0,(float)fabs(p1.x-p0.x));
//				UniObj->pmapParam->Invalidate();
				udeflector_param_blk.InvalidateUI();

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

int CreateUniObjectProc::proc(HWND hwnd,int msg,int point,int flag,
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
					assert( UniObj );					
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
						UniObj->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
#ifndef NO_CREATE_TASK	// russom - 12/04/01
				   		UniObj->EndEditParams( (IObjParam*)createInterface, 0, NULL);
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
					CloudNode = createInterface->CreateObjectNode( UniObj);
					attachedToNode = TRUE;
          UniObj->ClearAFlag(A_OBJ_CREATING);
					assert( CloudNode );					
					createCB = NULL;
					createInterface->SelectNode( CloudNode );
					
					// Reference the new node so we'll get notifications.
					theHold.Suspend();
					MakeRefByID( FOREVER, 0, CloudNode);
					theHold.Resume();
					mat.SetTrans(vpx->SnapPoint(m,m,NULL,snapdim));
//						macroRec->Disable();   // 10/00
					createInterface->SetNodeTMRelConstPlane(CloudNode, mat);
//						macroRec->Enable();
					dostuff=1;
					res = TRUE;
					break;
					
				}			
			break;

		case MOUSE_MOVE:
			//mat[3] = vpx->GetPointOnCP(m);
			mat.SetTrans(vpx->SnapPoint(m,m,NULL,snapdim));
				macroRec->Disable();   // 10/2/00
			createInterface->RedrawViews(createInterface->GetTime());	   
				macroRec->Enable();
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
//	  UniObj->BuildEmitter(createInterface->GetTime(),UniObj->);
	  createInterface->RedrawViews(createInterface->GetTime()); 
	  if (result==CREATE_STOP)
	  { res=FALSE;dostuff=0;				
#ifdef _OSNAP
         UniObj->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
	  theHold.Accept(GetString(IDS_DS_CREATE));	} 
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
		assert( UniObj );
#ifdef _OSNAP
		UniObj->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
#ifndef NO_CREATE_TASK	// russom - 12/04/01
		UniObj->EndEditParams( (IObjParam*)createInterface,0,NULL);
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
		if (os.obj->SuperClassID()!=GEOMOBJECT_CLASS_ID) {
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
	
	if ((node)&&(!node->IsGroupHead())) 
	{	ObjectState os = node->GetObjectRef()->Eval(ip->GetTime());
		if ((os.obj->SuperClassID()!=GEOMOBJECT_CLASS_ID)||(!IsGEOM(os.obj))) {
			node = NULL;
			return FALSE;
			}
		}

	return node ? TRUE : FALSE;
	}

void UniDefObject::ShowName()
{	TSTR name; 
	FormatName(name= TSTR(GetString(IDS_AP_ITEMSTR)) + (custnode ? custname : TSTR(GetString(IDS_AP_NONE))));
	SetWindowText(GetDlgItem(hParams, IDC_PCLOUD_PCUST), name);
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
	theHold.Put(new CreateUDeflectPickNode(po,node));

//	po->custnode=node;
	if (po->custnode) po->ReplaceReference(CUSTNODE,node,TRUE);
	else po->MakeRefByID(FOREVER,CUSTNODE,node);	
	po->NotifyDependents(FOREVER, 0, REFMSG_CHANGE);
	theHold.Accept(GetString(IDS_AP_UDEFPICK));
	po->custname = TSTR(node->GetName());
	// Automatically check show result and do one update
	po->ShowName();	
	if (po->creating) {
		theCreateUniObjectMode.JumpStart(ip,po);
		ip->SetCommandMode(&theCreateUniObjectMode);
		ip->RedrawViews(ip->GetTime());
		return FALSE;
	} else {
		return TRUE;
		}
	}

void UniPickOperand::EnterMode(IObjParam *ip)
	{
	ICustButton *iBut;
	iBut=GetICustButton(GetDlgItem(po->hParams,IDC_MTRACK_PICK));
	if (iBut) iBut->SetCheck(TRUE);
	ReleaseICustButton(iBut);
	GetCOREInterface()->PushPrompt(GetString(IDS_AP_PICKMODE));
	}

void UniPickOperand::ExitMode(IObjParam *ip)
	{
	ICustButton *iBut;
	iBut=GetICustButton(GetDlgItem(po->hParams,IDC_MTRACK_PICK));
	if (iBut) iBut->SetCheck(FALSE);
	ReleaseICustButton(iBut);
    GetCOREInterface()->PopPrompt();
	}

class UniObjectDlgProc : public ParamMap2UserDlgProc {
	public:
		UniDefObject *po;

		UniObjectDlgProc(UniDefObject *p) {po=p;}
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void Update(TimeValue t);
		void DeleteThis() {delete this;}
	};

void UniObjectDlgProc::Update(TimeValue t)
{	po->ShowName();
	float size;
//	po->pblock->GetValue(PB_RADIUS,0,size,FOREVER);
	po->pblock2->GetValue(udeflectrobj_radius,0,size,FOREVER);
	TurnButton(po->hParams,IDC_MTRACK_PICK,(size>=0.01f));
}

BOOL UniObjectDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{	switch (msg) {
		case WM_INITDIALOG: {
			ICustButton *iBut = GetICustButton(GetDlgItem(hWnd,IDC_MTRACK_PICK));
			iBut->SetType(CBT_CHECK);
			iBut->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(iBut);
			po->hParams=hWnd;
			Update(t);
			return FALSE;	// stop default keyboard focus - DB 2/27  
			}
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{    case IDC_MTRACK_PICK:
				   { if (po->ip->GetCommandMode()->ID() == CID_STDPICK) 
					{ if (po->creating) 
						{  theCreateUniObjectMode.JumpStart(po->ip,po);
							po->ip->SetCommandMode(&theCreateUniObjectMode);
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

UniDefObject::UniDefObject()
{
//	MakeRefByID(FOREVER, 0, 
//		CreateParameterBlock(descUniVer0, PBLOCK_UNILENGTH, CURRENT_VERSION));
//	assert(pblock);	

	pblock2 = NULL;
	UnideflectDesc.MakeAutoParamBlocks(this);
	assert(pblock2);

//	pblock->SetValue(PB_BOUNCE,0,1.0f);
//	pblock->SetValue(PB_BVAR,0,0.0f);
//	pblock->SetValue(PB_CHAOS,0,0.0f);
//	pblock->SetValue(PB_VELOCITY,0,0.0f);
//	pblock->SetValue(PB_FRICTION,0,0.0f);
//	pblock->SetValue(PB_RADIUS,0,0.0f);

	pblock2->SetValue(udeflectrobj_bounce,0,1.0f);
	pblock2->SetValue(udeflectrobj_bouncevar,0,0.0f);
	pblock2->SetValue(udeflectrobj_chaos,0,0.0f);
//	pblock2->SetValue(udeflectrobj_velocity,0,0.0f);
	pblock2->SetValue(udeflectrobj_friction,0,0.0f);
	pblock2->SetValue(udeflectrobj_radius,0,0.0f);

	dmesh=NULL;
	vnorms=NULL;
	fnorms=NULL;
	srand(lastrnd=12345);
	t=99999;
	custname=TSTR(_T(" "));
	custnode=NULL;
	nv=0;
	nf=0;
	macroRec->Disable();

//watje create a new ref to our collision engine
	CollisionMesh *colm = (CollisionMesh*)CreateInstance(REF_MAKER_CLASS_ID, MESH_COLLISION_ID);
	if (colm)
	{
		pblock2->SetValue(udeflectrobj_collider,0,(ReferenceTarget*)colm);
	}
	macroRec->Enable();
}

UniDefObject::~UniDefObject()
{  	
	DeleteAllRefsFromMe();
 

	pblock2 = NULL;
	if (vnorms) 
		delete[] vnorms;
	if (fnorms) 
		delete[] fnorms;
	if (dmesh) 
		delete dmesh;

}
Modifier *UniDefObject::CreateWSMMod(INode *node)
	{
	return new UniDeflectMod(node,this);
	}

RefTargetHandle UniDefObject::Clone(RemapDir& remap) 
	{
	UniDefObject* newob = new UniDefObject();	
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

void UniDefObject::BeginEditParams(
		IObjParam *ip,ULONG flags,Animatable *prev)
{	this->ip = ip;

	if (!hSot)
		hSot = ip->AddRollupPage( 
			hInstance, 
			MAKEINTRESOURCE(IDD_SW_DESC_LEGACY),
			DefaultSOTProc,
			GetString(IDS_RB_TOP), 
			(LPARAM)ip,APPENDROLL_CLOSED);

	SimpleWSMObject2::BeginEditParams(ip,flags,prev);

	if (flags&BEGIN_EDIT_CREATE) 
		creating = TRUE;
	else 
		creating = FALSE; 
	
	UnideflectDesc.BeginEditParams(ip, this, flags, prev);
	udeflector_param_blk.SetUserDlgProc(new UniObjectDlgProc(this));

	/*
	if (pmapParam) {		
		// Left over
		pmapParam->SetParamBlock(pblock);
	} else {		
		hSot = ip->AddRollupPage( 
			hInstance, 
			MAKEINTRESOURCE(IDD_SW_DESC),
			DefaultSOTProc,
			GetString(IDS_RB_TOP), 
			(LPARAM)ip,APPENDROLL_CLOSED);

		// Gotta make a new one.
		pmapParam = CreateCPParamMap(
			descUniParam,PARAMDESC_UNILENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_AP_UNIVDEFL),
			GetString(IDS_RB_PARAMETERS),
			0);
		}
		if (pmapParam)
			pmapParam->SetUserDlgProc(new UniObjectDlgProc(this));
*/
	}

void UniDefObject::EndEditParams(
		IObjParam *ip, ULONG flags,Animatable *next)
{	SimpleWSMObject2::EndEditParams(ip,flags,next);

	UnideflectDesc.EndEditParams(ip, this, flags, next);

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

void UniDefObject::BuildMesh(TimeValue t)
	{
	ivalid = FOREVER;
	float length;
//	pblock->GetValue(PB_RADIUS,t,length,ivalid);
	pblock2->GetValue(udeflectrobj_radius,t,length,ivalid);
	length/=2.0f;

	mesh.setNumVerts(11);
	mesh.setNumFaces(5);

	mesh.setVert(0,Point3(length,length,0.0f));
	mesh.setVert(1,Point3(-length,length,0.0f));
	mesh.setVert(2,Point3(length,-length,0.0f));
	mesh.setVert(3,Point3(-length,-length,0.0f));
	mesh.setVert(4,Point3(0.0f,0.0f,0.0f));
	mesh.setVert(5,Point3(0.0f,length,length));
	mesh.setVert(6,Point3(0.0f,-length,length));
	mesh.setVert(7,Point3(0.0f,10.0f/8.0f*length,length));
	mesh.setVert(8,Point3(0.0f,length,length*10.0f/8.0f));
	mesh.setVert(9,Point3(0.0f,-length,length*6.0f/8.0f));
	mesh.setVert(10,Point3(0.0f,-6.0f/8.0f*length,length));
	mesh.faces[0].setVerts(0,1,2);
	mesh.faces[0].setEdgeVisFlags(1,0,1);
	mesh.faces[0].setSmGroup(0);
	mesh.faces[1].setVerts(1,3,2);
	mesh.faces[1].setEdgeVisFlags(1,1,0);
	mesh.faces[1].setSmGroup(0);
	mesh.faces[2].setVerts(4,5,6);
	mesh.faces[2].setEdgeVisFlags(1,0,1);
	mesh.faces[2].setSmGroup(0);
	mesh.faces[3].setVerts(5,7,8);
	mesh.faces[3].setEdgeVisFlags(1,0,1);
	mesh.faces[3].setSmGroup(0);
	mesh.faces[4].setVerts(6,9,10);
	mesh.faces[4].setEdgeVisFlags(1,0,1);
	mesh.faces[4].setSmGroup(0);  
	mesh.InvalidateGeomCache();
	}

void UniDefObject::InvalidateUI() 
{
//	if (pmapParam) pmapParam->Invalidate();
	udeflector_param_blk.InvalidateUI(pblock2->LastNotifyParamID());

}

ParamDimension *UniDefObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {		
		case udeflectrobj_bouncevar:
		case udeflectrobj_chaos:
		case udeflectrobj_friction:
				 return stdPercentDim;
		case udeflectrobj_radius:
				 return stdWorldDim;
		default: return defaultDim;
		}
	}

/*
TSTR UniDefObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {				
		case PB_BOUNCE: 	return GetString(IDS_RB_BOUNCE);
		case PB_BVAR:		return GetString(IDS_RB_BVAR);
		case PB_CHAOS:		return GetString(IDS_RB_CHAOS);
		case PB_RADIUS:		return GetString(IDS_AP_ICONSIZE);
		case PB_FRICTION:	return GetString(IDS_AP_FRICTION);
		case PB_VELOCITY:	return GetString(IDS_AP_VELOCITY);
		default: 			return TSTR(_T(""));
		}
	}
*/


//--- DeflectMod methods -----------------------------------------------

UniDeflectMod::UniDeflectMod(INode *node,UniDefObject *obj)
	{	
//	MakeRefByID(FOREVER,SIMPWSMMOD_OBREF,obj);
	MakeRefByID(FOREVER,SIMPWSMMOD_NODEREF,node);	
		pblock = NULL;
	obRef=NULL;
	}

Interval UniDeflectMod::GetValidity(TimeValue t) 
{
	if (obRef && nodeRef) 
	{
		Interval valid = FOREVER;
		Matrix3 tm;
		float f;		
		UniDefObject *obj = (UniDefObject*)GetWSMObject(t);
//		obj->pblock->GetValue(PB_BOUNCE,t,f,valid);
//		obj->pblock->GetValue(PB_CHAOS,t,f,valid);
//		obj->pblock->GetValue(PB_RADIUS,t,f,valid);
//		obj->pblock->GetValue(PB_FRICTION,t,f,valid);
		obj->pblock2->GetValue(udeflectrobj_bounce,t,f,valid);
		obj->pblock2->GetValue(udeflectrobj_chaos,t,f,valid);
		obj->pblock2->GetValue(udeflectrobj_radius,t,f,valid);
		obj->pblock2->GetValue(udeflectrobj_friction,t,f,valid);
		tm = nodeRef->GetObjectTM(t,&valid);
		
		return valid;
	} 
	else 
	{
		return FOREVER;
	}
}

class UniDeflectDeformer : public Deformer {
	public:		
		Point3 Map(int i, Point3 p) {return p;}
	};

static UniDeflectDeformer Uniddeformer;

Deformer& UniDeflectMod::GetDeformer(
		TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
	{
	return Uniddeformer;
	}

RefTargetHandle UniDeflectMod::Clone(RemapDir& remap) 
	{
	UniDeflectMod *newob = new UniDeflectMod(nodeRef,(UniDefObject*)obRef);	
	newob->SimpleWSMModClone(this);
	BaseClone(this, newob, remap);
	return newob;
	}

void UniDeflectMod::ModifyObject(
		TimeValue t, ModContext &mc, ObjectState *os, INode *node)
	{
	ParticleObject *obj = GetParticleInterface(os->obj);
	if (obj) {
		deflect.obj  = (UniDefObject*)GetWSMObject(t);
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

int RayIntersect(Ray& ray, float& at, Point3& norm,Mesh *amesh,VNormal* vnorms,Point3 *fnorms)
	{
	DWORD fi;
	Point3 bary;
	Face *face = amesh->faces;	
	Point3 v0, v1, v2;
	Point3 n, sum, p, bry;
	float d, rn, a;
	Matrix3 vTM(1);
	BOOL first = FALSE;
	fi = 0xFFFFFFFF;

	for (int i=0; i<amesh->getNumFaces(); i++,face++) {
		n = fnorms[i];
		
		// See if the ray intersects the plane (backfaced)
		rn = DotProd(ray.dir,n);
		if (rn > -EPSILON) continue;
		
		// Use a point on the plane to find d
		d = DotProd(amesh->verts[face->v[0]],n);

		// Find the point on the ray that intersects the plane
		a = (d - DotProd(ray.p,n)) / rn;

		// Must be positive...
		if (a < 0.0f) continue;

		// Must be closer than the closest at so far
		if (first) {
			if (a > at) continue;
			}

		// The point on the ray and in the plane.
		p = ray.p + a*ray.dir;

		// Compute barycentric coords.
		bry = amesh->BaryCoords(i,p);

		// barycentric coordinates must sum to 1 and each component must
		// be in the range 0-1
		if (bry.x<0.0f || bry.x>1.0f || bry.y<0.0f || bry.y>1.0f || bry.z<0.0f || bry.z>1.0f) continue;
		if (fabs(bry.x + bry.y + bry.z - 1.0f) > EPSILON) continue;

		// Hit!
		first = TRUE;		
		at    = a;
		fi    = (DWORD)i;		
//		bary.x  = bry.z;
//		bary.y  = bry.x;
//		bary.z  = bry.y;
		bary  = bry;	// DS 3/8/97
		
		// Use interpolated normal instead.
		if (!face->smGroup) {
			norm  = n;
		} else {
			norm = 
				vnorms[face->v[0]].GetNormal(face->smGroup) * bary.x +
				vnorms[face->v[1]].GetNormal(face->smGroup) * bary.y +
				vnorms[face->v[2]].GetNormal(face->smGroup) * bary.z;
			norm = Normalize(norm);
			}
		}

	return first;
	}

void GetVFLst(Mesh* dmesh,VNormal* vnorms,Point3* fnorms)	 
{ int nv=dmesh->getNumVerts();	
  int nf=dmesh->getNumFaces();	
  Face *face = dmesh->faces;
  for (int i=0; i<nv; i++) 
    vnorms[i] = VNormal();
  Point3 v0, v1, v2;
  for (i=0; i<nf; i++,face++) 
  {	// Calculate the surface normal
	v0 = dmesh->verts[face->v[0]];
	v1 = dmesh->verts[face->v[1]];
	v2 = dmesh->verts[face->v[2]];
	fnorms[i] = (v1-v0)^(v2-v1);
	for (int j=0; j<3; j++) 
	   vnorms[face->v[j]].AddNormal(fnorms[i],face->smGroup);
    fnorms[i] = Normalize(fnorms[i]);
  }
  for (i=0; i<nv; i++) 
	vnorms[i].Normalize();
}

void AddMesh(UniDefObject *obj,TriObject *triOb,Matrix3 tm,BOOL nottop)
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

Object *UniDeflectorField::GetSWObject()
{ return obj;
}

BOOL UniDeflectorField::CheckCollision(
		TimeValue t,Point3 &inp, Point3 &vel, float dt, int index,float *ct,BOOL UpdatePastCollide)
{ 	if (badmesh) 
		return(0);

//watje need to get a pointer to out collider engine
//	ReferenceTarget *rt;
//	obj->pblock2->GetValue(udeflectrobj_collider,t,rt,obj->tmValid);
//	colm = (CollisionMesh *) rt;
// bayboro: the proper way to get a pointer to a our collider engine
// is to create CollisionSphere for each SphereDeflectorField object
	if (colm==NULL)
		colm = (CollisionMesh*)CreateInstance(REF_MAKER_CLASS_ID, MESH_COLLISION_ID);

	if (!((obj->mValid.InInterval(t))&&(obj->tmValid.InInterval(t))))
	{
		if (colm) colm->SetNode(t,obj->custnode);
// this lets it set up some pre initialization data
		if (colm) colm->PreFrame(t,(TimeValue) dt);

		obj->tmValid=FOREVER;

		if (obj->custnode == NULL) return FALSE;

		obj->tm=obj->custnode->GetObjectTM(t,&obj->tmValid);
		obj->tmNoTrans=obj->tm;
		obj->tmNoTrans.NoTrans();
		obj->invtm=Inverse(obj->tm);
		obj->invtmNoTrans=Inverse(obj->tmNoTrans);

		obj->pblock2->GetValue(udeflectrobj_bounce,t,bounce,FOREVER);
		if (bounce < 0.0f) bounce = 0.0f;
		obj->pblock2->GetValue(udeflectrobj_bouncevar,t,bvar,FOREVER);
		obj->pblock2->GetValue(udeflectrobj_chaos,t,chaos,FOREVER);
		obj->pblock2->GetValue(udeflectrobj_friction,t,friction,FOREVER);
		if (friction < 0.0f) friction = 0.0f;
		if (friction > 1.0f) friction = 1.0f;
		obj->pblock2->GetValue(udeflectrobj_velocity,t,vinher,FOREVER);

		if (obj->dmesh) 
			delete obj->dmesh;
		obj->dmesh=new Mesh;obj->dmesh->setNumFaces(0);
		if (obj->vnorms) 
		{	delete[] obj->vnorms;obj->vnorms=NULL;}
		if (obj->fnorms) 
		{	delete[] obj->fnorms;obj->fnorms=NULL;}
		obj->nv=(obj->nf=0);
		Interval tmpValid=FOREVER;
		obj->ptm=obj->custnode->GetObjectTM(t,&tmpValid);
		obj->dvel=(Zero*obj->ptm-Zero*obj->tm)/dt;
		Object *pobj; 
		pobj = obj->custnode->EvalWorldState(t).obj;
		obj->mValid=pobj->ObjectValidity(t);
		TriObject *triOb=NULL;
		badmesh=TRUE;
		if ((triOb=IsUseable(pobj,t))!=NULL) 
			AddMesh(obj,triOb,obj->tm,FALSE);
		if ((triOb)&&(triOb!=pobj))
			triOb->DeleteThis();
		if (obj->custnode->IsGroupHead())
		{	for (int ch=0;ch<obj->custnode->NumberOfChildren();ch++)
			{	INode *cnode=obj->custnode->GetChildNode(ch);
				if (cnode->IsGroupMember())
				{	pobj = cnode->EvalWorldState(t).obj;
					if ((triOb=IsUseable(pobj,t))!=NULL)
					{	Matrix3 tm=cnode->GetObjectTM(t,&obj->tmValid);
						obj->mValid=obj->mValid & pobj->ObjectValidity(t);
						AddMesh(obj,triOb,tm,TRUE);
					}
					if ((triOb)&&(triOb!=pobj)) 
						triOb->DeleteThis();
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

	if (colm)
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

//DebugPrint("%d inherited vel %f %f %f\n",t/160,inheritedVel.x,inheritedVel.y,inheritedVel.z);
//DebugPrint("   bounce vec %f %f %f\n",bnorm.x,bnorm.y,bnorm.z);
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
//DebugPrint("   pos %f %f %f\n",inp.x,inp.y,inp.z);
//DebugPrint("   vel %f %f %f\n",vel.x,vel.y,vel.z);
//		vel +=  (inheritedVel * vinher);

		obj->lastrnd=rand();
		return TRUE;
	}
	else
	{

	if (badmesh) 
		return 0;
	float pvel;
	Point3 NVrelL,Vrel,VrelL,pos;
	pos = inp*obj->invtm;
	Vrel = vel-obj->dvel;
	pvel = Length(Vrel);
	VrelL = Vrel*obj->invtmNoTrans;
	NVrelL = Normalize(VrelL);
	Ray ray;
	ray.dir = NVrelL;
	ray.p = pos;
	float at;
	Point3 norm;
	int kfound = RayIntersect(ray,at,norm,obj->dmesh,obj->vnorms,obj->fnorms);
	if (!kfound) 
	{
		obj->lastrnd=rand();
		return 0;
	}

	Point3 id,iw=(id=pos+at*NVrelL)*obj->tm;
	float delta=Length(iw-inp);
	if (delta>dt*pvel) 
	{
		obj->lastrnd=rand();
		return 0;
	}

	float dti=delta/pvel;
	Point3 wnorm=norm*obj->tmNoTrans;
	Point3 c1,Vreln=Vrel/pvel;
	Point3 Vdirbase=Normalize(Vreln);
	Point3 Vt,c2;
	float q1=DotProd(-Vdirbase,wnorm);
	float theta=(float)acos(q1);
	if (theta>=HalfPI) 
		theta-=PI;
	Point3 zero=Zero;
	if (theta<FLOAT_EPSILON) 
		vel=-vel;
	else 
	{	c1=Normalize((-vel)^wnorm);
		c2=Normalize(wnorm^c1);
		vel=-vel;
		Vt=c2*DotProd(c2,vel);
		RotateOnePoint(&vel.x,&zero.x,&c1.x,2*theta);
		vel=vel+friction*Vt;
	}
    srand(obj->lastrnd);
	vel=vel*bounce*(1-bvar*RND01());
	if (!FloatEQ0(chaos))
	{	theta=(HalfPI-theta)*chaos*RND01();
		// Martell 4/14/01: Fix for order of ops bug.
		float ztmp=RND11(); float ytmp=RND11(); float xtmp=RND11();
		Point3 d=Point3(xtmp,ytmp,ztmp);
		Point3 c=Normalize(vel^d);
		RotateOnePoint(&vel.x,&zero.x,&c.x,theta);
	}
	if (vinher>0.0f)
	{	Point3 dvel=obj->dvel*vinher; 
		vel=vel+friction*dvel+(1-friction)*DotProd(dvel,wnorm)*wnorm;
	}

	if (UpdatePastCollide)
	{	inp = iw+(dt-dti)*vel;
		if (ct) 
			(*ct) = dt;
	}
	else
	{	inp = iw;
		if (ct) 
			(*ct) = dti;
	}

	obj->lastrnd=rand();
	return TRUE;

	}
}

RefTargetHandle UniDefObject::GetReference(int i)
{	switch(i) {
		case PBLK: return SimpleWSMObject2::GetReference(i);
		case CUSTNODE: return (RefTargetHandle)custnode;
		default: return NULL;
		}
	}

void UniDefObject::SetReference(int i, RefTargetHandle rtarg) 
{ 
	switch(i) 
	{
//		case PBLK: pblock2=(IParamBlock2*)rtarg; return;
		case PBLK: SimpleWSMObject2::SetReference(i, rtarg); return;
		case CUSTNODE: custnode = (INode *)rtarg; return;
	}
}

RefResult UniDefObject::NotifyRefChanged( 
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
#define UNIDEF_CUSTNAME_CHUNK	0x0100

IOResult UniDefObject::Save(ISave *isave)
	{
	isave->BeginChunk(UNIDEF_CUSTNAME_CHUNK);		
	isave->WriteWString(custname);
	isave->EndChunk();
	return IO_OK;
	}

class UniDeflectObjectLoad : public PostLoadCallback 
{
	public:
		UniDefObject *n;
		UniDeflectObjectLoad(UniDefObject *ns) {n = ns;}
		void proc(ILoad *iload) 
		{  
			ReferenceTarget *rt;
			Interval iv;
			n->pblock2->GetValue(udeflectrobj_collider,0,rt,iv);
			if (rt == NULL)
			{
				CollisionMesh *colm = (CollisionMesh*)CreateInstance(REF_MAKER_CLASS_ID, MESH_COLLISION_ID);
				if (colm)
					n->pblock2->SetValue(udeflectrobj_collider,0,(ReferenceTarget*)colm);
			}

			// Bayboro
			// Parameters Variation, Chaos, Friction and Inherit Vel are readjusted to be
			// consistent with corresponding parameters in Omni- and DynaFlectors.
			if ((HIWORD(iload->GetFileSaveVersion()) < 4000) && 
			    (LOWORD(iload->GetFileSaveVersion()) < 48)) // MAX v.3.9, build 48
			{
				Control *cont, *mult;
				float pcnt = 0.01f;

				int defl_chan[3] = {udeflectrobj_bouncevar, udeflectrobj_chaos, udeflectrobj_friction};
				// do nothing with Variation, Chaos & Friction since it's converted from Shiva automatically
				if (HIWORD(iload->GetFileSaveVersion()) >= 3900)
					for( int i=0; i<3; i++)
					{	cont = n->pblock2->GetController(defl_chan[i]);
						if (cont == NULL)
							n->pblock2->SetValue(defl_chan[i], 0, 0.01f*n->pblock2->GetFloat(defl_chan[i], 0));
						else
						{	if (cont->CanApplyEaseMultCurves())
							{	mult = NewDefaultFloatController();
								mult->SetValue(0, &pcnt);
								cont->AppendMultCurve(mult);
							}
							else
								n->pblock2->SetValue(defl_chan[i], 0, 0.01f*n->pblock2->GetFloat(defl_chan[i], 0));
						}
					}

				cont = n->pblock2->GetController(udeflectrobj_velocity);
				if (cont != NULL) // user's done some manipulations with the parameters; let's rescale
				{	if (cont->CanApplyEaseMultCurves())
					{	mult = NewDefaultFloatController();
						mult->SetValue(0, &pcnt);
						cont->AppendMultCurve(mult);
					}
					else
						n->pblock2->SetValue(udeflectrobj_velocity, 0, 0.01f*n->pblock2->GetFloat(udeflectrobj_velocity, 0));
				}
				else
				{	if ((n->pblock2->GetFloat(udeflectrobj_velocity,0) == 1.0f) &&
						(HIWORD(iload->GetFileSaveVersion()) >= 3900))
					{	; // do nothing since user probably haven't adjusted the parameter and left it as default value
						// therefore it should be left as 1.0 multiplier
					}
					else	// user's done some manipulations with the parameters; let's rescale
						n->pblock2->SetValue(udeflectrobj_velocity, 0, 0.01f*n->pblock2->GetFloat(udeflectrobj_velocity, 0));
				}
			}

			delete this; 
		} 
};

IOResult UniDefObject::Load(ILoad *iload)
{
	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, NUM_OLDVERSIONS, &udeflector_param_blk, this, PBLOCK_REF_NO);
	iload->RegisterPostLoadCallback(plcb);
	
	// Default names
	custname = TSTR(_T(" "));

	IOResult res = IO_OK;

	while (IO_OK==(res=iload->OpenChunk())) 
	{
		switch (iload->CurChunkID()) 
		{
			case UNIDEF_CUSTNAME_CHUNK: 
			{
				TCHAR *buf;
				res=iload->ReadWStringChunk(&buf);
				custname = TSTR(buf);
				break;
			}
		}
		
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
	}
	iload->RegisterPostLoadCallback(new UniDeflectObjectLoad(this));
	return IO_OK;
}

CollisionObject *UniDefObject::GetCollisionObject(INode *node)
{
	UniDeflectorField *gf = new UniDeflectorField;	
	gf->obj  = this;
	gf->node = node;
	gf->obj->tmValid.SetEmpty();
	gf->badmesh = (gf->obj->custnode==NULL);
	gf->obj->dvel=Zero;
	return gf;
}

