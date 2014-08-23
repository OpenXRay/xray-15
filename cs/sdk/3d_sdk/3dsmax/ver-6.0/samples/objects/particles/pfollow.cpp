/**********************************************************************
 *<
	FILE: pathfollow.cpp

	DESCRIPTION: A simple path follow object for particles

	CREATED BY: Audrey Peterson

	HISTORY: 12/96

	EDP revs 7/00 ECP

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/
#include "suprprts.h"
#include "iparamm.h"
#include "simpmod.h"
#include "simpobj.h"
#include "texutil.h"
#include "spline3d.h"
// #include "pod.h" // Bayboro 9/18/01
#include "macrorec.h"

#define PBLK		0
#define CUSTNODE 		1
#define DU 0.001f
static Class_ID PFOLLOW_CLASS_ID(0x7ab83ab5, 0x5e1d34bd);
static Class_ID PFOLLOWMOD_CLASS_ID(0x263e723d, 0x132724e5);

class PFollowPickOperand;

typedef struct {
  BOOL found;
  BOOL FoundOnceAlready;
  Point3 StartPoint;
  Point3 TraverseVector;

//  BOOL donewith;
  TimeValue Tc,Tt;
  Point3 R,TTT1;
  float TChaos,SChaos;
} pathplist;

class PFollowData : public LocalModData {
public:
	Tab<pathplist> psaved;
	int lastseed;
	TimeValue lasttime;
	PFollowData();
	~PFollowData();
	LocalModData *Clone();
};

PFollowData::PFollowData() 
{	psaved.SetCount(0);
//	psaved[0].found=FALSE;
}

PFollowData::~PFollowData() 
{	psaved.Resize(0);
}

LocalModData *PFollowData::Clone () 
{	PFollowData *clone;
	clone = new PFollowData();
	clone->lastseed=lastseed;
	clone->psaved.SetCount(psaved.Count());
	for (int i=0;i<psaved.Count();i++)
	{ clone->psaved=psaved;}
	return(clone);
}

class PFollowMod;

class PFollowObject : public SimpleWSMObject //,IOperatorInterface  // Bayboro 9/18/01
{	
	public:		
		static IParamMap *pmapParam;
		static IObjParam *ip;
		static HWND hSot;
		static HWND hParams;
		BOOL reset;
					
		INode *custnode;
		TSTR custname;
		PFollowObject();
		~PFollowObject();
		static BOOL creating;
		static PFollowPickOperand pickCB;

		void ShowName();
		// From Animatable		
		void DeleteThis() {delete this;}		
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);		
		Class_ID ClassID() {return PFOLLOW_CLASS_ID;}		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_AP_PFOLLOWOBJ);}
		void MapKeys(TimeMap *map,DWORD flags);
				
		// from object		
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}
		
		// From SimpleWSMObject		
		void InvalidateUI();		
		void BuildMesh(TimeValue t);
		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);		
		
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
		// From WSMObject
		Modifier *CreateWSMMod(INode *node);		
		int NumRefs() {return 2;}
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);		
		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message);

//		int NPOpInterface(TimeValue t,ParticleData *part,float dt,INode *node,int index); // Bayboro 9/18/01
//		void* GetInterface(ULONG id); // Bayboro 9/18/01
		void SetUpModifier(TimeValue t,INode *node);
		PFollowMod *mf;
};

class PFollowPickOperand : 
		public PickModeCallback,
		public PickNodeCallback {
	public:		
		PFollowObject *po;
		
		PFollowPickOperand() {po=NULL;}

		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);

		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);

		BOOL RightClick(IObjParam *ip, ViewExp *vpt) { return TRUE; }
		BOOL Filter(INode *node);
		
		PickNodeCallback *GetFilter() {return this;}
	};

//--- ClassDescriptor and class vars ---------------------------------

IObjParam *PFollowObject::ip        = NULL;
IParamMap *PFollowObject::pmapParam = NULL;
HWND       PFollowObject::hSot      = NULL;
HWND       PFollowObject::hParams      = NULL;
BOOL PFollowObject::creating    = FALSE;
PFollowPickOperand PFollowObject::pickCB;

class PFollowClassDesc:public ClassDesc 
{
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) { return new PFollowObject;}
	const TCHAR *	ClassName() {return GetString(IDS_AP_PFOLLOW);}
	SClass_ID		SuperClassID() {return WSM_OBJECT_CLASS_ID; }
	Class_ID		ClassID() {return PFOLLOW_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_EP_SW_FORCES);}
	int 			BeginCreate(Interface *i);
	int 			EndCreate(Interface *i);
};

static PFollowClassDesc PFollowDesc;
ClassDesc* GetPFollowDesc() {return &PFollowDesc;}

//--- DeflectMod -----------------------------------------------------

class PFollowField : public CollisionObject 
{
	public:		
		PFollowObject *obj;
		ParticleObject *partobj;
		INode *node,*pnode;
		Object *pobj;
		Matrix3 tm,ptm;
		Interval tmValid,mValid;
		PFollowData *pd;
		ShapeObject *pathOb;
		int badmesh;
		BOOL CheckCollision(TimeValue t,Point3 &pos, Point3 &vel, float dt,int index,float *ct,BOOL UpdatePastCollide);
//		BOOL CheckCollision2(TimeValue t,Point3 &pos, Point3 &vel, float dt,int index,float *ct,BOOL Unlock,ParticleData *part); // Bayboro 9/18/01
		Object *GetSWObject();
};

class PFollowMod : public SimpleWSMMod 
{
	public:				
		PFollowField deflect;

		PFollowMod() {}
		PFollowMod(INode *node,PFollowObject *obj);	


		// From Animatable
		void GetClassName(TSTR& s) {s= GetString(IDS_AP_PFOLLOWMOD);}
		SClass_ID SuperClassID() {return WSM_CLASS_ID;}
		void DeleteThis() {delete this;}
		Class_ID ClassID() { return PFOLLOWMOD_CLASS_ID;}
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_AP_PATHFOLLOWBINDING);}

		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);

		// From SimpleWSMMod		
		Interval GetValidity(TimeValue t);		
		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);
	};

//--- ClassDescriptor and class vars ---------------------------------

class PFollowModClassDesc:public ClassDesc 
{
	public:
	int 			IsPublic() {return 0;}
	void *			Create(BOOL loading = FALSE) { return new PFollowMod;}
	const TCHAR *	ClassName() {return GetString(IDS_AP_PFOLLOWMOD);}
	SClass_ID		SuperClassID() {return WSM_CLASS_ID; }
	Class_ID		ClassID() {return PFOLLOWMOD_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
};

static PFollowModClassDesc PFollowModDesc;
ClassDesc* GetPFollowModDesc() {return &PFollowModDesc;}
//--- SphereDefObject Parameter map/block descriptors ------------------

#define PB_LIMITRANGE	0
#define PB_RANGEDIST	1
#define PB_SPLINETYPE	2
#define PB_TANGCHAOS	3
#define PB_TANGCHAOSVAR	4
#define PB_TANGDIR		5
#define PB_SPIRCHAOS	6
#define PB_SPIRCHAOSVAR	7
#define PB_SPIRALDIR	8
#define PB_TIMESTART	9
#define PB_TIMETRAVEL	10
#define PB_TIMETRAVVAR	11
#define PB_TIMESTOP		12
#define PB_ICONSIZE		13
#define PB_SEED			14
#define PB_KSPEED		15

//Radio buttons
static int splinetypeIDs[] = {IDC_AP_SPLINEOFFSET,IDC_AP_SPLINEPARALLEL};
#define OFFSET 0
#define PARALLEL 1
static int tangenttypeIDs[] = {IDC_AP_PATHTANIN,IDC_AP_PATHTANOUT,IDC_AP_PATHTANBOTH};
#define CONTRACT 0
#define BOTH 2
static int spiraltypeIDs[] = {IDC_AP_PATHCW,IDC_AP_PATHCCW,IDC_AP_PATHBIDIR};
#define CCW 1
#define BIDIR 2

//Dialogs
static ParamUIDesc descParam[] = {

	// Range Limit On
	ParamUIDesc(PB_LIMITRANGE,TYPE_SINGLECHEKBOX,IDC_AP_RANGELIMITON),

	// Range Distance	
	ParamUIDesc(
		PB_RANGEDIST,
		EDITTYPE_UNIVERSE,
		IDC_AP_RANGELIMIT,IDC_AP_RANGELIMITSPIN,
		0.0f, 9999999.0f,
		0.01f),
	
	// Spline type radio buttons
	ParamUIDesc(PB_SPLINETYPE,TYPE_RADIO,splinetypeIDs,2),
	
	// Range Limit On
	ParamUIDesc(PB_KSPEED,TYPE_SINGLECHEKBOX,IDC_AP_CONSTSPEED),

	// Tangent Chaos
	ParamUIDesc(
		PB_TANGCHAOS,
		EDITTYPE_FLOAT,
		IDC_AP_CHAOSTAN,IDC_AP_CHAOSTANSPIN,
		0.0f,99.0f,
		1.0f,
		stdPercentDim),

	// Tangent Chaos Var
	ParamUIDesc(
		PB_TANGCHAOSVAR,
		EDITTYPE_FLOAT,
		IDC_AP_CHAOSTANVAR,IDC_AP_CHAOSTANVARSPIN,
		0.0f,100.0f,
		1.0f,
		stdPercentDim),

	// Tangent type radio buttons
	ParamUIDesc(PB_TANGDIR,TYPE_RADIO,tangenttypeIDs,3),

	// Spiral Chaos
	ParamUIDesc(
		PB_SPIRCHAOS,
		EDITTYPE_FLOAT,
		IDC_AP_CHAOSSPIRAL,IDC_AP_CHAOSSPIRALSPIN,
		0.0f,99999.0f,
		0.01f),

	// Spiral Chaos Var
	ParamUIDesc(
		PB_SPIRCHAOSVAR,
		EDITTYPE_FLOAT,
		IDC_AP_CHAOSSPIRALVAR,IDC_AP_CHAOSSPIRALVARSPIN,
		0.0f,100.0f,
		1.0f,
		stdPercentDim),

	// Spiral type radio buttons
	ParamUIDesc(PB_SPIRALDIR,TYPE_RADIO,spiraltypeIDs,3),

	// Start time
	ParamUIDesc(
		PB_TIMESTART,
		EDITTYPE_TIME,
		IDC_AP_CAPTURETIME,IDC_AP_CAPTURETIMESPIN,
		-999999999.0f,999999999.0f,
		10.0f),

	// Travel time
	ParamUIDesc(
		PB_TIMETRAVEL,
		EDITTYPE_TIME,
		IDC_AP_TRAVELTIME,IDC_AP_TRAVELTIMESPIN,
		GetTicksPerFrame(),999999999.0f, // the minimum value should be at least one frame
		10.0f),

	// Travel Time Var
	ParamUIDesc(
		PB_TIMETRAVVAR,
		EDITTYPE_FLOAT,
		IDC_AP_SPEEDVAR,IDC_AP_SPEEDVARSPIN,
		0.0f,100.0f,
		1.0f,
		stdPercentDim),

	// Last Frame
	ParamUIDesc(
		PB_TIMESTOP,
		EDITTYPE_TIME,
		IDC_AP_LASTFRAME,IDC_AP_LASTFRAMESPIN,
		-999999999.0f,999999999.0f,
		10.0f),

	// Seed
	ParamUIDesc(
		PB_SEED,
		EDITTYPE_INT,
		IDC_AP_SEED,IDC_AP_SEEDSPIN,
		0.0f,25000.0f,
		1.0f),

	// Icon Size
	ParamUIDesc(
		PB_ICONSIZE,
		EDITTYPE_UNIVERSE,
		IDC_AP_ICONSIZE,IDC_AP_ICONSIZESPIN,
		0.0f, 9999999.0f,
		SPIN_AUTOSCALE),

	};

#define PARAMDESC_LENGTH	16

#define CURRENT_VERSION	0

ParamBlockDescID PFollowdescVer0[] = {
	{ TYPE_INT, NULL, FALSE, 0 },	//Limit Range
	{ TYPE_FLOAT, NULL, TRUE, 1 },	//Range Dist
	{ TYPE_INT, NULL, FALSE, 2 },	//Spline Type
	{ TYPE_FLOAT, NULL, FALSE, 3 },	//Tang Chaos
	{ TYPE_FLOAT, NULL, FALSE, 4 },	//Tang Chaos Var
	{ TYPE_INT, NULL, FALSE, 5 },	//Tang Dir
	{ TYPE_FLOAT, NULL, FALSE, 6 },	//Spiral Chaos
	{ TYPE_FLOAT, NULL, FALSE, 7 },	//Spiral Chaos Var
	{ TYPE_INT, NULL, FALSE, 8 },	//Spiral Dir
	{ TYPE_INT, NULL, FALSE, 9 },	//Time Start
	{ TYPE_INT, NULL, FALSE, 10 },	//Time Travel
	{ TYPE_FLOAT, NULL, FALSE, 11 }, //Time Travel Var
	{ TYPE_INT, NULL, FALSE, 12 },  //Last Frame
	{ TYPE_FLOAT, NULL, FALSE, 13 },  //Icon Size
	{ TYPE_INT, NULL, FALSE, 14 },  //Seed
	{ TYPE_INT, NULL, FALSE, 15 },  //Speed flag
};

#define PBLOCK_PFLENGTH	16

#define CURRENT_VERSION	0

class CreatePFollowObjectProc : public MouseCallBack,ReferenceMaker {
	private:
		IObjParam *ip;
		void Init(IObjParam *i) {ip=i;}
		CreateMouseCallBack *createCB;	
		INode *PFollowNode;
		PFollowObject *PFollowObj;
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
		RefTargetHandle GetReference(int i) { return (RefTargetHandle)PFollowNode; } 
		void SetReference(int i, RefTargetHandle rtarg) { PFollowNode = (INode *)rtarg; }

		// StdNotifyRefChanged calls this, which can change the partID to new value 
		// If it doesnt depend on the particular message& partID, it should return
		// REF_DONTCARE
		BOOL SupportAutoGrid(){return TRUE;}
	    RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	    	PartID& partID,  RefMessage message);
	public:
		void Begin( IObjCreate *ioc, ClassDesc *desc );
		void End();
		
		CreatePFollowObjectProc()
			{
			ignoreSelectionChange = FALSE;
			}
		int createmethod(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
		int proc( HWND hwnd, int msg, int point, int flag, IPoint2 m );
	};


#define CID_CREATEPFollowObjectMODE	CID_USER + 15

class CreatePFollowObjectMode : public CommandMode {		
	public:		
		CreatePFollowObjectProc proc;
		IObjParam *ip;
		PFollowObject *obj;
		void Begin( IObjCreate *ioc, ClassDesc *desc ) { proc.Begin( ioc, desc ); }
		void End() { proc.End(); }
		void JumpStart(IObjParam *i,PFollowObject*o);

		int Class() {return CREATE_COMMAND;}
		int ID() { return CID_CREATEPFollowObjectMODE; }
		MouseCallBack *MouseProc(int *numPoints) {*numPoints = 10000; return &proc;}
		ChangeForegroundCallback *ChangeFGProc() {return CHANGE_FG_SELECTED;}
		BOOL ChangeFG( CommandMode *oldMode ) { return (oldMode->ChangeFGProc() != CHANGE_FG_SELECTED); }
		void EnterMode() 
		{ GetCOREInterface()->PushPrompt(GetString(IDS_AP_CREATEMODE));
		  SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR)));
		}
		void ExitMode() {GetCOREInterface()->PopPrompt();SetCursor(LoadCursor(NULL, IDC_ARROW));}
	};
static CreatePFollowObjectMode theCreatePFollowObjectMode;

RefResult CreatePFollowObjectProc::NotifyRefChanged(
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
		 	if ( PFollowObj && PFollowNode==hTarget ) {
				// this will set camNode== NULL;
				theHold.Suspend();
				DeleteReference(0);
				theHold.Resume();
				goto endEdit;
				}
			// fall through

		case REFMSG_TARGET_DELETED:		
			if (PFollowObj && PFollowNode==hTarget ) {
				endEdit:
				if (createInterface->GetCommandMode()->ID() == CID_STDPICK) 
				{ if (PFollowObj->creating) 
						{  theCreatePFollowObjectMode.JumpStart(PFollowObj->ip,PFollowObj);
							createInterface->SetCommandMode(&theCreatePFollowObjectMode);
					    } 
				  else {createInterface->SetStdCommandMode(CID_OBJMOVE);}
				}
#ifdef _OSNAP
				PFollowObj->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
				PFollowObj->EndEditParams( (IObjParam*)createInterface, 0, NULL);
				PFollowObj  = NULL;				
				PFollowNode    = NULL;
				CreateNewObject();	
				attachedToNode = FALSE;
				}
			break;		
		}
	return REF_SUCCEED;
	}

void CreatePFollowObjectProc::Begin( IObjCreate *ioc, ClassDesc *desc )
	{
	createInterface = ioc;
	cDesc           = desc;
	attachedToNode  = FALSE;
	createCB        = NULL;
	PFollowNode         = NULL;
	PFollowObj       = NULL;
	dostuff=0;
	CreateNewObject();
	}
void CreatePFollowObjectProc::CreateNewObject()
	{
	SuspendSetKeyMode();
	createInterface->GetMacroRecorder()->BeginCreate(cDesc);
	PFollowObj = (PFollowObject*)cDesc->Create();
	lastPutCount  = theHold.GetGlobalPutCount();
	
	// Start the edit params process
	if ( PFollowObj ) {
#ifndef NO_CREATE_TASK	// russom - 12/04/01
		PFollowObj->BeginEditParams( (IObjParam*)createInterface, BEGIN_EDIT_CREATE, NULL );
#endif
#ifdef _OSNAP
		PFollowObj->SetAFlag(A_OBJ_LONG_CREATE);
#endif
		}	
	ResumeSetKeyMode();
	}

//LACamCreationManager::~LACamCreationManager
void CreatePFollowObjectProc::End()
{ if ( PFollowObj ) 
	{
 #ifdef _OSNAP
		PFollowObj->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
#ifndef NO_CREATE_TASK	// russom - 12/04/01
	PFollowObj->EndEditParams( (IObjParam*)createInterface, 
	                    	          END_EDIT_REMOVEUI, NULL);
#endif
		if ( !attachedToNode ) 
		{	// RB 4-9-96: Normally the hold isn't holding when this 
			// happens, but it can be in certain situations (like a track view paste)
			// Things get confused if it ends up with undo...
			createInterface->GetMacroRecorder()->Cancel();
			theHold.Suspend(); 
			delete PFollowObj;
			PFollowObj = NULL;
			theHold.Resume();
			if (theHold.GetGlobalPutCount()!=lastPutCount) 
				GetSystemSetting(SYSSET_CLEAR_UNDO);
		} 
 else if ( PFollowNode ) {
			 // Get rid of the reference.
			theHold.Suspend();
			DeleteReference(0);  // sets camNode = NULL
			theHold.Resume();
			}
	}
}

void CreatePFollowObjectMode::JumpStart(IObjParam *i,PFollowObject *o)
	{
	ip  = i;
	obj = o;
	//MakeRefByID(FOREVER,0,svNode);
#ifndef NO_CREATE_TASK	// russom - 12/04/01
	obj->BeginEditParams(i,BEGIN_EDIT_CREATE,NULL);
#endif
	}


int PFollowClassDesc::BeginCreate(Interface *i)
	{	
	SuspendSetKeyMode();
	IObjCreate *iob = i->GetIObjCreate();
	theCreatePFollowObjectMode.Begin(iob,this);
	iob->PushCommandMode(&theCreatePFollowObjectMode);
	return TRUE;
	}

int PFollowClassDesc::EndCreate(Interface *i)
	{
	ResumeSetKeyMode();
	theCreatePFollowObjectMode.End();
	i->RemoveMode(&theCreatePFollowObjectMode);
	macroRec->EmitScript();  // 10/00
	return TRUE;
	}

int CreatePFollowObjectProc::createmethod(
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
				PFollowObj->pblock->SetValue(PB_ICONSIZE,0,0.01f);
				PFollowObj->pmapParam->Invalidate();
				break;

			case 1: {
				mat.IdentityMatrix();
				sp1 = m;
				p1  = vpt->SnapPoint(m,m,NULL,snapdim);
				Point3 center = (p0+p1)/float(2);
				mat.SetTrans(center);
				PFollowObj->pblock->SetValue(PB_ICONSIZE,0,Length(p1-p0));
				PFollowObj->pmapParam->Invalidate();

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

int CreatePFollowObjectProc::proc(HWND hwnd,int msg,int point,int flag,
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
					assert( PFollowObj );					
					vpx->CommitImplicitGrid(m, flag );
					if ( createInterface->SetActiveViewport(hwnd) ) {
						return FALSE;
						}

					if (createInterface->IsCPEdgeOnInView()) { 
						res = FALSE;
						goto done;
						}

					if ( attachedToNode ) {
#ifdef _OSNAP
						PFollowObj->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
				   		// send this one on its way
				   		PFollowObj->EndEditParams( (IObjParam*)createInterface, 0, NULL);
              createInterface->GetMacroRecorder()->EmitScript();
						if (PFollowNode) {
							theHold.Suspend();
							DeleteReference(0);
							theHold.Resume();
							}

						// new object
						CreateNewObject();   // creates PFollowObj
						}

				   	theHold.Begin();	 // begin hold for undo
					mat.IdentityMatrix();

					// link it up
					PFollowNode = createInterface->CreateObjectNode( PFollowObj);
					attachedToNode = TRUE;
					assert( PFollowNode );					
					createCB = NULL;
					createInterface->SelectNode( PFollowNode );
					
					// Reference the new node so we'll get notifications.
					theHold.Suspend();
					MakeRefByID( FOREVER, 0, PFollowNode);
					theHold.Resume();
					mat.SetTrans(vpx->SnapPoint(m,m,NULL,snapdim));
//					macroRec->Disable();   // 10/00
					createInterface->SetNodeTMRelConstPlane(PFollowNode, mat);
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
	  createInterface->RedrawViews(createInterface->GetTime()); 
	  if (result==CREATE_STOP)
	  { res=FALSE;dostuff=0;				
#ifdef _OSNAP
         PFollowObj->ClearAFlag(A_OBJ_LONG_CREATE);
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
		assert( PFollowObj );
#ifdef _OSNAP
		PFollowObj->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
		PFollowObj->EndEditParams( (IObjParam*)createInterface,0,NULL);
		theHold.Cancel();	 // deletes both the object and target.
		if (theHold.GetGlobalPutCount()!=lastPutCount) 
					GetSystemSetting(SYSSET_CLEAR_UNDO);
		PFollowNode = NULL;			
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

BOOL PFollowPickOperand::Filter(INode *node)
	{
	if (node)
	{ ObjectState os = node->GetObjectRef()->Eval(po->ip->GetTime());
	  if (os.obj->SuperClassID() != SHAPE_CLASS_ID) 
	  {		node = NULL;
			return FALSE;
	  }
	node->BeginDependencyTest();
	po->NotifyDependents (FOREVER, 0, REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) return FALSE;

	}
	return node ? TRUE : FALSE;
	}

BOOL PFollowPickOperand::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
	{	
	INode *node = ip->PickNode(hWnd,m,this);
	
	if (node) 
	{	ObjectState os = node->GetObjectRef()->Eval(ip->GetTime());
		if (os.obj->SuperClassID() != SHAPE_CLASS_ID)
		{	node = NULL;
			return FALSE;
			}
	node->BeginDependencyTest();
	po->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) return FALSE;
	}
	return node ? TRUE : FALSE;
	}

void PFollowObject::ShowName()
{TSTR name=TSTR(GetString(IDS_AP_OBJECTSTR)) + (custnode ? custname : TSTR(GetString(IDS_AP_NONE)));
SetWindowText(GetDlgItem(hParams, IDC_SP_PATHNAME), name);
}

BOOL PFollowPickOperand::Pick(IObjParam *ip,ViewExp *vpt)
	{
	INode *node = vpt->GetClosestHit();
	assert(node);
	INodeTab nodes;
	nodes.SetCount(1);nodes[0]=node;
	ip->FlashNodes(&nodes);
	if (po->custnode) po->ReplaceReference(CUSTNODE,node,TRUE);
	else po->MakeRefByID(FOREVER,CUSTNODE,node);	
	po->custname = TSTR(node->GetName());
	// Automatically check show result and do one update
	po->ShowName();	
	if (po->creating) {
		theCreatePFollowObjectMode.JumpStart(ip,po);
		ip->SetCommandMode(&theCreatePFollowObjectMode);
		ip->RedrawViews(ip->GetTime());
		return FALSE;
	} else {
		return TRUE;
		}
	}

void PFollowPickOperand::EnterMode(IObjParam *ip)
	{
	ICustButton *iBut;
	iBut=GetICustButton(GetDlgItem(po->hParams,IDC_SP_PATHNAMEPICK));
	if (iBut) iBut->SetCheck(TRUE);
	ReleaseICustButton(iBut);
	GetCOREInterface()->PushPrompt(GetString(IDS_AP_PICKMODE));
	}

void PFollowPickOperand::ExitMode(IObjParam *ip)
	{
	ICustButton *iBut;
	iBut=GetICustButton(GetDlgItem(po->hParams,IDC_SP_PATHNAMEPICK));
	if (iBut) iBut->SetCheck(FALSE);
	ReleaseICustButton(iBut);
    GetCOREInterface()->PopPrompt();
	}
class PFollowObjectDlgProc : public ParamMapUserDlgProc {
	public:
		PFollowObject *po;

		PFollowObjectDlgProc(PFollowObject *p) {po=p;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void Update(TimeValue t);
		void DeleteThis() {delete this;}
	};
void PFollowObjectDlgProc::Update(TimeValue t)
{
	po->ShowName();
	int ron;
	po->pblock->GetValue(PB_LIMITRANGE,0,ron,FOREVER);
	if (ron) SpinnerOff(po->hParams,IDC_AP_RANGELIMITSPIN,IDC_AP_RANGELIMIT);
	else SpinnerOn(po->hParams,IDC_AP_RANGELIMITSPIN,IDC_AP_RANGELIMIT);
	EnableWindow(GetDlgItem(po->hParams,IDC_AP_RANGELIMIT_TXT),!ron);
	float size;
	po->pblock->GetValue(PB_ICONSIZE,0,size,FOREVER);
	TurnButton(po->hParams,IDC_SP_PATHNAMEPICK,(size>=0.01f));
}

BOOL PFollowObjectDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{	switch (msg) {
		case WM_INITDIALOG: {
			ICustButton *iBut = GetICustButton(GetDlgItem(hWnd,IDC_SP_PATHNAMEPICK));
			iBut->SetType(CBT_CHECK);
			iBut->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(iBut);
			po->hParams=hWnd;
			Update(t);
			return FALSE;	// stop default keyboard focus - DB 2/27  
			}
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{ case IDC_SP_PATHNAMEPICK:
				   { if (po->ip->GetCommandMode()->ID() == CID_STDPICK) 
					{ if (po->creating) 
						{  theCreatePFollowObjectMode.JumpStart(po->ip,po);
							po->ip->SetCommandMode(&theCreatePFollowObjectMode);
						} else {po->ip->SetStdCommandMode(CID_OBJMOVE);}
					} else 
						{ po->pickCB.po = po;						
						  po->ip->SetPickMode(&po->pickCB);
						}
					break;
				}
			case IDC_AP_RANGELIMITON:
				int ron;
				po->pblock->GetValue(PB_LIMITRANGE,0,ron,FOREVER);
				if (ron) SpinnerOff(hWnd,IDC_AP_RANGELIMITSPIN,IDC_AP_RANGELIMIT);
				else SpinnerOn(hWnd,IDC_AP_RANGELIMITSPIN,IDC_AP_RANGELIMIT);
				EnableWindow(GetDlgItem(hWnd,IDC_AP_RANGELIMIT_TXT),!ron);
				break;
			}
			break;	
		}
	return TRUE;
	}

PFollowObject::PFollowObject()
{	TimeValue tpf=GetTicksPerFrame();
	MakeRefByID(FOREVER, 0, 
		CreateParameterBlock(PFollowdescVer0, PBLOCK_PFLENGTH, CURRENT_VERSION));
	assert(pblock);	
	reset=FALSE;

	pblock->SetValue(PB_LIMITRANGE,0,1);
	pblock->SetValue(PB_RANGEDIST,0,100.0f);
	pblock->SetValue(PB_SPLINETYPE,0,1);
	pblock->SetValue(PB_TANGCHAOS,0,0.0f);
	pblock->SetValue(PB_TANGCHAOSVAR,0,0.0f);
	pblock->SetValue(PB_TANGDIR,0,0);
	pblock->SetValue(PB_SPIRCHAOS,0,0.0f);
	pblock->SetValue(PB_SPIRCHAOSVAR,0,0.0f);
	pblock->SetValue(PB_SPIRALDIR,0,0);
	pblock->SetValue(PB_TIMESTART,0,0);
	pblock->SetValue(PB_TIMETRAVEL,0,30*tpf);
	pblock->SetValue(PB_TIMETRAVVAR,0,0.0f);
	pblock->SetValue(PB_TIMESTOP,0,100*tpf);
	pblock->SetValue(PB_ICONSIZE,0,0.0f);
	pblock->SetValue(PB_SEED,0,12345);
	pblock->SetValue(PB_KSPEED,0,0);
	srand(12345);
	custname = TSTR(_T(" "));
	custnode = NULL;
	mf = NULL;
}

PFollowObject::~PFollowObject()
{ 
	DeleteAllRefsFromMe();
	if (mf)
		delete mf;
	pblock = NULL;
}

Modifier *PFollowObject::CreateWSMMod(INode *node)
{
	return new PFollowMod(node,this);
}

RefTargetHandle PFollowObject::Clone(RemapDir& remap) 
{
	PFollowObject* newob = new PFollowObject();	
	newob->ReplaceReference(0,pblock->Clone(remap));
	if (custnode) 
		newob->ReplaceReference(CUSTNODE,custnode);
	newob->custname=custname;
	BaseClone(this, newob, remap);
	return newob;
}

void PFollowObject::BeginEditParams(
		IObjParam *ip,ULONG flags,Animatable *prev)
{	SimpleWSMObject::BeginEditParams(ip,flags,prev);
	this->ip = ip;
	if (flags&BEGIN_EDIT_CREATE) {
		creating = TRUE;
	} else { creating = FALSE; }

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
			descParam,PARAMDESC_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_SW_PATHFOLLOW),
			GetString(IDS_RB_PARAMETERS),
			0);
		}
		if (pmapParam)
			pmapParam->SetUserDlgProc(new PFollowObjectDlgProc(this));
	}

void PFollowObject::EndEditParams(
		IObjParam *ip, ULONG flags,Animatable *next)
	{		
	SimpleWSMObject::EndEditParams(ip,flags,next);
	this->ip = NULL;

	if (flags&END_EDIT_REMOVEUI ) {		
		// russom - 12/04/01
		// Added NULL ptr detection - used for NO_CREATE_TASK
		if( pmapParam ) {
			DestroyCPParamMap(pmapParam);
			pmapParam = NULL;		
		}
		if( hSot ) {
			ip->DeleteRollupPage(hSot);
		}
	}	
	ip->ClearPickMode();
	ip= NULL;
	creating = FALSE;
	}

void PFollowObject::MapKeys(TimeMap *map,DWORD flags)
{	TimeValue TempTime;
	pblock->GetValue(PB_TIMESTART,0,TempTime,FOREVER);
	TempTime=map->map(TempTime);
	pblock->SetValue(PB_TIMESTART,0,TempTime);
	pblock->GetValue(PB_TIMETRAVEL,0,TempTime,FOREVER);
	TempTime=map->map(TempTime);
	pblock->SetValue(PB_TIMETRAVEL,0,TempTime);
	pblock->GetValue(PB_TIMESTOP,0,TempTime,FOREVER);
	TempTime=map->map(TempTime);
	pblock->SetValue(PB_TIMESTOP,0,TempTime);
}

void PFollowObject::BuildMesh(TimeValue t)
{	ivalid = FOREVER;
	float width;
	pblock->GetValue(PB_ICONSIZE,t,width,ivalid);
//	width  *= 0.5f;

	mesh.setNumVerts(45); //9+9*4
	mesh.setNumFaces(28); //12+16
	mesh.setVert(0, Point3(-width,-width, width));
	mesh.setVert(1, Point3( width,-width, width));
	mesh.setVert(2, Point3( width, width, width));
	mesh.setVert(3, Point3(-width, width, width));
	mesh.setVert(4, Point3(-width,-width, -width));
	mesh.setVert(5, Point3( width,-width, -width));
	mesh.setVert(6, Point3( width, width, -width));
	mesh.setVert(7, Point3(-width, width, -width));
	mesh.setVert(8, Point3(0,0,0));

	mesh.faces[0].setEdgeVisFlags(1,0,1);
	mesh.faces[0].setSmGroup(0);
	mesh.faces[0].setVerts(0,1,3);

	mesh.faces[1].setEdgeVisFlags(1,1,0);
	mesh.faces[1].setSmGroup(0);
	mesh.faces[1].setVerts(1,2,3);

	mesh.faces[2].setEdgeVisFlags(1,1,0);
	mesh.faces[2].setSmGroup(0);
	mesh.faces[2].setVerts(1,0,4);

	mesh.faces[3].setEdgeVisFlags(1,0,1);
	mesh.faces[3].setSmGroup(0);
	mesh.faces[3].setVerts(5,1,4);

	mesh.faces[4].setEdgeVisFlags(0,1,1);
	mesh.faces[4].setSmGroup(0);
	mesh.faces[4].setVerts(5,2,1);

	mesh.faces[5].setEdgeVisFlags(1,1,0);
	mesh.faces[5].setSmGroup(0);
	mesh.faces[5].setVerts(5,6,2);

	mesh.faces[6].setEdgeVisFlags(1,0,1);
	mesh.faces[6].setSmGroup(0);
	mesh.faces[6].setVerts(2,6,3);

	mesh.faces[7].setEdgeVisFlags(1,1,0);
	mesh.faces[7].setSmGroup(0);
	mesh.faces[7].setVerts(6,7,3);

	mesh.faces[8].setEdgeVisFlags(1,1,0);
	mesh.faces[8].setSmGroup(0);
	mesh.faces[8].setVerts(0,3,7);

	mesh.faces[9].setEdgeVisFlags(1,0,1);
	mesh.faces[9].setSmGroup(0);
	mesh.faces[9].setVerts(4,0,7);

	mesh.faces[10].setEdgeVisFlags(0,1,1);
	mesh.faces[10].setSmGroup(0);
	mesh.faces[10].setVerts(6,4,7);

	mesh.faces[11].setEdgeVisFlags(1,1,0);
	mesh.faces[11].setSmGroup(0);
	mesh.faces[11].setVerts(6,5,4);
	Point3 basept[4];
	basept[0]=Point3(0.0f,0.0f,0.0f);
	basept[1]=Point3(0.0f,-0.8f*width,-0.8f*width);
	basept[2]=Point3(0.0f,-0.5f*width,0.4f*width);
	basept[3]=Point3(0.0f,0.6f*width,0.1f*width);
	float r1=0.1f*width,r2=0.2f*width,r3=0.3f*width,r4=0.4f*width;
	float r6=0.6f*width,r8=0.8f*width,r25=0.25f*width;
	int nv=9,fc=12,bnv;
	for (int i=0;i<4;i++)
	{ bnv=nv;
	  mesh.setVert(nv++, Point3(-width,0.0f,0.0f)+basept[i]);
	  mesh.setVert(nv++, Point3(-r8,r1,0.0f)+basept[i]);
	  mesh.setVert(nv++, Point3(-r6,r1,0.0f)+basept[i]);
	  mesh.setVert(nv++, Point3(-r2,r3,r2)+basept[i]);
	  mesh.setVert(nv++, Point3(0.0f,r3,r2)+basept[i]);
	  mesh.setVert(nv++, Point3(r4,r1,r3)+basept[i]);
	  mesh.setVert(nv++, Point3(r6,r1,r3)+basept[i]);
	  mesh.setVert(nv++, Point3(r8,r2,r25)+basept[i]);
	  mesh.setVert(nv++, Point3(width,r3,r2)+basept[i]);
	  mesh.faces[fc].setVerts(bnv,bnv+2,bnv+1);
	  mesh.faces[fc].setEdgeVisFlags(0,1,1);
	  mesh.faces[fc++].setSmGroup(0);
	  mesh.faces[fc].setVerts(bnv+2,bnv+4,bnv+3);
	  mesh.faces[fc].setEdgeVisFlags(0,1,1);
	  mesh.faces[fc++].setSmGroup(0);
	  mesh.faces[fc].setVerts(bnv+4,bnv+5,bnv+6);
	  mesh.faces[fc].setEdgeVisFlags(1,1,0);
	  mesh.faces[fc++].setSmGroup(0);
	  mesh.faces[fc].setVerts(bnv+6,bnv+7,bnv+8);
	  mesh.faces[fc].setEdgeVisFlags(1,1,0);
	  mesh.faces[fc++].setSmGroup(0);
	}
	mesh.InvalidateGeomCache();
}

void PFollowObject::InvalidateUI() 
{
	if (pmapParam) pmapParam->Invalidate();

}

ParamDimension *PFollowObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {		
		case PB_TANGCHAOS:
		case PB_TIMETRAVVAR:
				 return stdPercentDim;
		case PB_TIMESTART:
		case PB_TIMETRAVEL:
		case PB_TIMESTOP:
				 return stdTimeDim;
		case PB_ICONSIZE:
		case PB_RANGEDIST:
				return stdWorldDim;
		default: return defaultDim;
		}
	}

TSTR PFollowObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {				
		case PB_KSPEED		: 	return GetString(IDS_AP_KSPEED);
		case PB_LIMITRANGE: 	return GetString(IDS_AP_RANGEON);
		case PB_RANGEDIST:		return GetString(IDS_AP_RANGE);
		case PB_SPLINETYPE:		return GetString(IDS_AP_SPLINETYPE);
		case PB_TANGCHAOS:		return GetString(IDS_AP_TANGCHAOS);
		case PB_TANGCHAOSVAR:	return GetString(IDS_AP_TANGCHAOSVAR);
		case PB_TANGDIR:		return GetString(IDS_AP_TANGENTDIR);
		case PB_SPIRCHAOS:		return GetString(IDS_AP_SPIRCHAOS);
		case PB_SPIRCHAOSVAR:	return GetString(IDS_AP_SPIRCHAOSVAR);
		case PB_SPIRALDIR:		return GetString(IDS_AP_SPIRALDIR);
		case PB_TIMESTART:		return GetString(IDS_AP_TIMESTART);
		case PB_TIMETRAVEL:		return GetString(IDS_AP_TIMETRAVEL);
		case PB_TIMETRAVVAR:	return GetString(IDS_AP_TRAVELVAR);
		case PB_TIMESTOP:		return GetString(IDS_AP_TIMESTOP);
		case PB_ICONSIZE:		return GetString(IDS_AP_ICONSIZE);
		default: 			return TSTR(_T(""));
		}
	}

//--- DeflectMod methods -----------------------------------------------

PFollowMod::PFollowMod(INode *node,PFollowObject *obj)
{	
//	MakeRefByID(FOREVER,SIMPWSMMOD_OBREF,obj);
	MakeRefByID(FOREVER,SIMPWSMMOD_NODEREF,node);	
	pblock = NULL;
	obRef = NULL;
}

Interval PFollowMod::GetValidity(TimeValue t) 
{ 	
	if (obRef && nodeRef)
	{	Interval valid = FOREVER;
		Matrix3 tm;
		float f;		
		PFollowObject *obj = (PFollowObject*)GetWSMObject(t);
		obj->pblock->GetValue(PB_RANGEDIST,t,f,valid);
		tm = nodeRef->GetObjectTM(t,&valid);
		return valid;
	} else {return FOREVER;	}
}

class PFollowDeformer : public Deformer {
	public:		
		Point3 Map(int i, Point3 p) {return p;}
	};
static PFollowDeformer PFollowdeformer;

Deformer& PFollowMod::GetDeformer(
		TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
{
	return PFollowdeformer;
}

RefTargetHandle PFollowMod::Clone(RemapDir& remap) 
{
	PFollowMod *newob = new PFollowMod(nodeRef,(PFollowObject*)obRef);	
	newob->SimpleWSMModClone(this);
	BaseClone(this, newob, remap);
	return newob;
}

void PFollowMod::ModifyObject(
		TimeValue t, ModContext &mc, ObjectState *os, INode *node)
{
	ParticleObject *obj = GetParticleInterface(os->obj);
	if (deflect.partobj=obj) 
	{
		deflect.obj  = (PFollowObject*)GetWSMObject(t);
		deflect.node = nodeRef;
		deflect.pnode = deflect.obj->custnode;
		deflect.tmValid.SetEmpty();		
		deflect.mValid.SetEmpty();
		deflect.badmesh = (deflect.obj->custnode==NULL);
		deflect.pobj = NULL;
//		if ((t==0)&&(mc.localData!= NULL))	
//		{  
//			delete mc.localData;mc.localData=NULL;	
//		}
		int starttime;
		deflect.obj->pblock->GetValue(PB_TIMESTART,t,starttime,FOREVER);
		if ( ((t==starttime)||(deflect.obj->reset)) && (mc.localData!=NULL)) 
		{	
			delete mc.localData;
			mc.localData = NULL;	
		}
		if (mc.localData == NULL)
		{	
			mc.localData = new PFollowData;
			int seed;
			deflect.obj->pblock->GetValue(PB_SEED,t,seed,FOREVER);
			deflect.pd = (PFollowData *) mc.localData;
			deflect.pd->lastseed=seed;
			deflect.pd->lasttime=t;
		} 
		else 
			deflect.pd = (PFollowData *) mc.localData;
		obj->ApplyCollisionObject(&deflect);
	}
}

static ShapeObject *IsUseable(Object *pobj,TimeValue t)
{ 
	if (pobj->IsShapeObject())
		return (ShapeObject*)pobj;
	else 
		return NULL;
}

Object *PFollowField::GetSWObject()
{ 
	return obj;
}

BOOL PFollowField::CheckCollision(TimeValue t,Point3 &pos, Point3 &vel, float dt,int index,float *ct,BOOL UpdatePastCollide)
{	Point3 invel = vel;
	int achange = 0; // what does this do??

	int starttime,stoptime;
	obj->pblock->GetValue(PB_TIMESTART,t,starttime,FOREVER);
	obj->pblock->GetValue(PB_TIMESTOP,t,stoptime,FOREVER);

// if we're backing up and have particles saved, nuke database
	if ((pd->psaved.Count()>0)&&(pd->lasttime>t))
	{	pd->psaved.Resize(0);
	    if (obj->reset) obj->reset=FALSE;
	}

	pd->lasttime=t; // set the last time to this time

	if ((badmesh)||(t<starttime))
		return FALSE; // if there's no spline, or this is too early, do squat

// if we've changed the controls, start over
	if (obj->reset)
	{	pd->psaved.SetCount(0);
		obj->reset=FALSE;
	}

// if we're here for the first time init randomizer
	if (pd->psaved.Count()<1)
	{	int seed;
		obj->pblock->GetValue(PB_SEED,t,seed,FOREVER);
		pd->lastseed = seed;
	}

/*
// this was the old attempt to save the random number
	if (pd->lasttime>t) 
	{	int seed;
		obj->pblock->GetValue(PB_SEED,t,seed,FOREVER);
		pd->lastseed=seed;
	}
*/

	if (!((mValid.InInterval(t))&&(tmValid.InInterval(t))))
	{	tmValid = FOREVER;
		tm = pnode->GetObjectTM(t,&tmValid);
		pobj = pnode->EvalWorldState(t).obj;
		mValid = pobj->ObjectValidity(t);
		badmesh = (badmesh||((pathOb=IsUseable(pobj,t))==NULL));
//		srand(pd->lastseed);
 	}

	if (badmesh) 
		return FALSE;

	if (pd->psaved.Count()>index)
		if (pd->psaved[index].found)
			if (pd->psaved[index].Tc>t)
				return FALSE;

	if (pathOb->NumberOfCurves() <= 0) return FALSE;

	int splinetype;
 	obj->pblock->GetValue(PB_SPLINETYPE,t,splinetype,FOREVER);

	int limitr;
	obj->pblock->GetValue(PB_LIMITRANGE,t,limitr,FOREVER);

	if (t<=stoptime) // we're in the window

// do this if we haven't seen the particle yet, if the particle is out of range, or if the particle is new
	{	if ((pd->psaved.Count()<=index)||(!pd->psaved[index].found)||(partobj->ParticleAge(t,index)==0))
		{	
		
			Point3 firstpt = pathOb->InterpCurve3D(t,0,0.0f)*tm;
			Point3 R = pos - firstpt;
			float maxrange;
			int traveltime;
			obj->pblock->GetValue(PB_RANGEDIST,t,maxrange,FOREVER);
			if (!limitr && (Length(R)>maxrange)) // if the particle is out of range, abort
				return FALSE;

			int anum = pd->psaved.Count();

			if (anum>index)
				if (!pd->psaved[index].FoundOnceAlready)
					pd->psaved[index].StartPoint = pos;

			if (anum<=index)
			{	pd->psaved.Resize(index+1);
				pd->psaved.SetCount(index+1);
				for (int newnum=anum;newnum<=index;newnum++)  // this sets notfound for all unknown but presupposed particles
				{	pd->psaved[newnum].found = FALSE;
					pd->psaved[newnum].FoundOnceAlready = FALSE;
				}
				pd->psaved[index].StartPoint = pos;
			}

			pd->psaved[index].found = TRUE;

			if (!pd->psaved[index].FoundOnceAlready)
			{	
				float traveltimevar,tangchaos,tangchaosvar,spiralchaos,spiralchaosvar;
				int tdir,sdir;
				obj->pblock->GetValue(PB_TANGCHAOS,t,tangchaos,FOREVER);
				obj->pblock->GetValue(PB_TANGCHAOSVAR,t,tangchaosvar,FOREVER);
				obj->pblock->GetValue(PB_TANGDIR,t,tdir,FOREVER);
				obj->pblock->GetValue(PB_SPIRCHAOS,t,spiralchaos,FOREVER);
				obj->pblock->GetValue(PB_SPIRCHAOSVAR,t,spiralchaosvar,FOREVER);
				obj->pblock->GetValue(PB_SPIRALDIR,t,sdir,FOREVER);
				obj->pblock->GetValue(PB_TIMETRAVEL,t,traveltime,FOREVER);
 				obj->pblock->GetValue(PB_TIMETRAVVAR,t,traveltimevar,FOREVER);

				if (splinetype==OFFSET) 
				{	Point3 TT1,TTT1,T1 = Normalize(pathOb->InterpCurve3D(t, 0, DU)*tm-firstpt);
					pd->psaved[index].TTT1 = (TTT1=Normalize(T1^R));
					TT1 = Normalize(TTT1^T1);
					Matrix4By4 weyw,invweyw;
					weyw[0][0] = T1.x;
					weyw[1][0] = T1.y;
					weyw[2][0] = T1.z;
					weyw[3][0] = 0.0f;
					weyw[0][1] = TT1.x;
					weyw[1][1] = TT1.y;
					weyw[2][1] = TT1.z;
					weyw[3][1] = 0.0f;
					weyw[0][2] = TTT1.x;
					weyw[1][2] = TTT1.y;
					weyw[2][2] = TTT1.z;
					weyw[3][2] = 0.0f;
					weyw[0][3] = firstpt.x;
					weyw[1][3] = firstpt.y;
					weyw[2][3] = firstpt.z;
					weyw[3][3] = 1.0f;
					float aR[4],pout[4];
					MatrixInvert(weyw,invweyw);
					aR[0] = pos.x;
					aR[1] = pos.y;
					aR[2] = pos.z;
					aR[3] = 1.0f;
					Mult4X1(aR,invweyw,pout);
					R.x = pout[0];
					R.y = pout[1];
					R.z = pout[2];
				}

				pd->psaved[index].R = R;
				pd->psaved[index].Tc = t;

				srand(pd->lastseed); // encapsulated around all the RNDs is the save/restore of the randomizer

				if (tdir==CONTRACT) 
					tangchaos = -tangchaos;
				else if (tdir==BOTH) 
					tangchaos *= RNDSign();
				pd->psaved[index].TChaos = tangchaos*(1.0f-RND01()*tangchaosvar);
				if (sdir==CCW) 
					spiralchaos = -spiralchaos;
				else if (sdir==BIDIR) 
					spiralchaos *= RNDSign();
				pd->psaved[index].SChaos = spiralchaos*(1.0f-RND01()*spiralchaosvar);
				pd->psaved[index].Tt = (int)(traveltime*(1.0f+RND11()*traveltimevar));

				pd->lastseed = rand(); // this saves the random sequence for capturing particles
			}
		}
	}
	else // do this if the particle is nonfound and we're too late to capture it
	{	if ((pd->psaved.Count()<=index)||(!pd->psaved[index].found)) 
			return FALSE;
	}

	TimeValue	intime = t,
				endtime = pd->psaved[index].Tc + pd->psaved[index].Tt,
				difftime;

	int timeloops = 0;

	if ((pd->psaved[index].FoundOnceAlready)&&(t>endtime))
	{	timeloops++;
		difftime = t - endtime;
		while (difftime > pd->psaved[index].Tt)
		{	timeloops++;
			difftime -= pd->psaved[index].Tt;
		}
	}

	if (t>stoptime) 
		endtime = stoptime;

	BOOL lettinggo = (t>=endtime),
		 done=FALSE;
	int tpf=GetTicksPerFrame();
	if (lettinggo) 
		t = endtime - tpf;

	float u;
	int KeepSpeedAlongSpline;

// here begins the maintenance update loop
	while (!done)
	{	if (timeloops < 1)
			u = (float)(tpf + t - pd->psaved[index].Tc) / (float)pd->psaved[index].Tt;
		else
			u = (float)(tpf + difftime) / (float)pd->psaved[index].Tt;

		if (u>1.0f) 
			u = 1.0f;

 		obj->pblock->GetValue(PB_KSPEED,t,KeepSpeedAlongSpline,FOREVER);
		Point3 Sut = pathOb->InterpCurve3D(t,0,u,KeepSpeedAlongSpline)*tm;

//for parallel splines we go here
		if (splinetype==PARALLEL)
		{	Point3 Rnow = pd->psaved[index].R*(1.0f+u*pd->psaved[index].TChaos);
			Point3 PTemp = Sut + Rnow;
			if (pd->psaved[index].SChaos!=0.0f)
			{	Point3 T1 = Normalize(pathOb->InterpCurve3D(t, 0, DU)*tm-pathOb->InterpCurve3D(t,0,0.0f,KeepSpeedAlongSpline)*tm);
				float Thetanow = TWOPI*pd->psaved[index].SChaos*u;
				RotateOnePoint(&PTemp.x,&Sut.x,&T1.x,Thetanow);
			}
			pos = PTemp;
		}
//for offset splines we go here
		else 
		{	float cu;
			Point3 Tt = Normalize(pathOb->InterpCurve3D(t,0,((cu=u+DU)>1.0f?1.0f:cu),KeepSpeedAlongSpline)*tm-pathOb->InterpCurve3D(t, 0, ((cu=u-DU)<0.0f?0.0f:cu))*tm);
			Point3 TTt = Normalize(pd->psaved[index].TTT1^Tt);
			Point3 TTTt = Normalize(Tt^TTt);
			Matrix4By4 mx;
			float aR[4],pout[4];
			Point3 Rnow = pd->psaved[index].R*(1.0f+u*pd->psaved[index].TChaos);
			mx[0][0] = Tt.x;
			mx[1][0] = Tt.y;
			mx[2][0] = Tt.z;
			mx[3][0] = 0.0f;
			mx[0][1] = TTt.x;
			mx[1][1] = TTt.y;
			mx[2][1] = TTt.z;
			mx[3][1] = 0.0f;
			mx[0][2] = TTTt.x;
			mx[1][2] = TTTt.y;
			mx[2][2] = TTTt.z;
			mx[3][2] = 0.0f;
			mx[0][3] = Sut.x;
			mx[1][3] = Sut.y;
			mx[2][3] = Sut.z;
			mx[3][3] = 1.0f;
			aR[0] = Rnow.x;
			aR[1] = Rnow.y;
			aR[2] = Rnow.z;
			aR[3] = 1.0f;
			if (pd->psaved[index].SChaos!=0.0f)
			{	Matrix4By4 mrot,mres;
				float Thetanow = TWOPI*pd->psaved[index].SChaos*u;
				mrot[0][0]=mrot[3][3]=1.0f;
				mrot[0][1]=mrot[0][2]=mrot[0][3]=mrot[1][0]=mrot[1][3]=mrot[2][0]=mrot[2][3]=0.0f;
				mrot[3][0]=mrot[3][1]=mrot[3][2]=0.0f;
				mrot[2][2]=mrot[1][1]=(float)cos(Thetanow);mrot[1][2]=-(mrot[2][1]=(float)sin(Thetanow));
				Mult4X4(mx,mrot,mres);
				Mult4X1(aR,mres,pout);
			}
			else 
				Mult4X1(aR,mx,pout);
			pos.x = pout[0];
			pos.y = pout[1];
			pos.z = pout[2];
		}

		if ((lettinggo)&&(t<endtime)) 
			t = endtime;
		else 
			done = TRUE;
	}

	if (timeloops>0)
		pos += (float)(timeloops)*pd->psaved[index].TraverseVector;

// new velocity calculation HERE

	float splinelen = pathOb->LengthOfCurve(t,0);
	Point3 tanVector = pathOb->TangentCurve3D(t,0,u);
	float baseSpeed = splinelen/(float)pd->psaved[index].Tt;
	
	float u1,u2,u3;
	if (u<0.01f)
	{	u1 = 0.0f;
		u2 = 0.01f;
		u3 = 0.02f;
	}
	else if (u>0.99f)
	{	u1 = 0.98f;
		u2 = 0.99f;
		u3 = 1.0f;
	}
	else
	{	u1 = u - 0.01f;
		u2 = u;
		u3 = u + 0.01f;
	}

	Point3 S1, S2, S3;
	S1 = pathOb->InterpCurve3D(t,0,u1,KeepSpeedAlongSpline)*tm;
	S2 = pathOb->InterpCurve3D(t,0,u2,KeepSpeedAlongSpline)*tm;
	S3 = pathOb->InterpCurve3D(t,0,u3,KeepSpeedAlongSpline)*tm;
	Point3 R1, R2;
	R1 = S2 - S1;
	R2 = S3 - S2;
	float lrSq1, lrSq2;
	lrSq1 = LengthSquared(R1);
	lrSq2 = LengthSquared(R2);

	if (lrSq1<0.000001f)
	{	S1 = S2 - 0.001f*tanVector;
		R1 = S2 - S1;
		lrSq1 = LengthSquared(R1);
	}
	if (lrSq2<0.000001f)
	{	S3 = S2 + 0.001f*tanVector;
		R2 = S3 - S2;
		lrSq2 = LengthSquared(R2);
	}

	float ddProd = DotProd(R1,R2);
	double RlRatio = sqrt((ddProd*ddProd)/(lrSq1*lrSq2));
	float theta = 0.0f;
	if (RlRatio < 1.0) {
		if (RlRatio <= -1.0) theta = PI;
		else theta = (float)acos(RlRatio);
	}

	if (theta < 0.0001f)
		vel = baseSpeed * tanVector;
	else
	{	Point3 SpinAxis = Normalize(R1^R2);
		Point3 Rad1 = Normalize(SpinAxis^R1);
		Point3 Rad2 = Normalize(SpinAxis^R2);
		Point3 P1 = 0.5f*(S1+S2);
		Point3 P2 = 0.5f*(S2+S3);

		float denom, paramT; 
		Point3 RadDif = Rad2 - Rad1;
		int maxIndex = MaxComponent(RadDif);
		denom = Rad2[maxIndex] - Rad1[maxIndex];
		paramT = 0.0f;
		if (denom == 0.0f) paramT = 0.0f;
		else paramT = (P1[maxIndex]-P2[maxIndex])/denom;

		if (paramT == 0.0f) {
			vel = baseSpeed * tanVector;
		} else {
			Point3 omega = (baseSpeed/paramT)*SpinAxis;
			Point3 ArcCenter = P1 + paramT*Rad1;
			Point3 EffectiveArc = pos - ArcCenter;
			vel = omega^EffectiveArc;
		}
	}

	if ((lettinggo)&&(!pd->psaved[index].FoundOnceAlready))
	{	pd->psaved[index].TraverseVector = pos - pd->psaved[index].StartPoint;
		pd->psaved[index].FoundOnceAlready = TRUE;
	}

	if (lettinggo)
	{	vel = invel;
		pd->psaved[index].found = FALSE;
	}

	// Bayboro: for compatibility with the new collision engine (3/7/01)
	*ct = dt;

	return TRUE;
}

/*  // Bayboro 9/18/01
BOOL PFollowField::CheckCollision2(TimeValue t,Point3 &pos, Point3 &vel, float dt,int index,float *ct,BOOL Unlock,ParticleData *part)
{	Point3 invel = vel;
	int achange = 0; // what does this do??

	int starttime,stoptime;
	obj->pblock->GetValue(PB_TIMESTART,t,starttime,FOREVER);
	obj->pblock->GetValue(PB_TIMESTOP,t,stoptime,FOREVER);

// if we're backing up and have particles saved, nuke database
	if ((pd->psaved.Count()>0)&&(pd->lasttime>t))
	{	pd->psaved.Resize(0);
	    if (obj->reset) obj->reset=FALSE;
	}

	pd->lasttime=t; // set the last time to this time

	if ((badmesh)||(t<starttime))
		return FALSE; // if there's no spline, or this is too early, do squat

// if we've changed the controls, start over
	if (obj->reset)
	{	pd->psaved.SetCount(0);
		obj->reset=FALSE;
	}

// if we're here for the first time init randomizer
	if (pd->psaved.Count()<1)
	{	int seed;
		obj->pblock->GetValue(PB_SEED,t,seed,FOREVER);
		pd->lastseed = seed;
	}

	if (!((mValid.InInterval(t))&&(tmValid.InInterval(t))))
	{	tmValid = FOREVER;
		tm = pnode->GetObjectTM(t,&tmValid);
		pobj = pnode->EvalWorldState(t).obj;
		mValid = pobj->ObjectValidity(t);
		badmesh = (badmesh||((pathOb=IsUseable(pobj,t))==NULL));
//		srand(pd->lastseed);
 	}

	if (badmesh) 
		return FALSE;

	if (pd->psaved.Count()>index)
		if (pd->psaved[index].found)
			if (pd->psaved[index].Tc>t)
				return FALSE;

	int splinetype;
 	obj->pblock->GetValue(PB_SPLINETYPE,t,splinetype,FOREVER);

	int limitr;
	obj->pblock->GetValue(PB_LIMITRANGE,t,limitr,FOREVER);

	if (t<=stoptime) // we're in the window

// do this if we haven't seen the particle yet, if the particle is out of range, or if the particle is new
	{	if ((pd->psaved.Count()<=index)||(!pd->psaved[index].found)||(part->age == 0.0f))
		{	
		
			Point3 firstpt = pathOb->InterpCurve3D(t,0,0.0f)*tm;
			Point3 R = pos - firstpt;
			float maxrange;
			int traveltime;
			obj->pblock->GetValue(PB_RANGEDIST,t,maxrange,FOREVER);
			if (!limitr && (Length(R)>maxrange)) // if the particle is out of range, abort
				return FALSE;

			int anum = pd->psaved.Count();

			if (anum>index)
				if (!pd->psaved[index].FoundOnceAlready)
					pd->psaved[index].StartPoint = pos;

			if (anum<=index)
			{	pd->psaved.Resize(index+1);
				pd->psaved.SetCount(index+1);
				for (int newnum=anum;newnum<=index;newnum++)  // this sets notfound for all unknown but presupposed particles
				{	pd->psaved[newnum].found = FALSE;
					pd->psaved[newnum].FoundOnceAlready = FALSE;
				}
				pd->psaved[index].StartPoint = pos;
			}

			pd->psaved[index].found = TRUE;

			if (!pd->psaved[index].FoundOnceAlready)
			{	
				float traveltimevar,tangchaos,tangchaosvar,spiralchaos,spiralchaosvar;
				int tdir,sdir;
				obj->pblock->GetValue(PB_TANGCHAOS,t,tangchaos,FOREVER);
				obj->pblock->GetValue(PB_TANGCHAOSVAR,t,tangchaosvar,FOREVER);
				obj->pblock->GetValue(PB_TANGDIR,t,tdir,FOREVER);
				obj->pblock->GetValue(PB_SPIRCHAOS,t,spiralchaos,FOREVER);
				obj->pblock->GetValue(PB_SPIRCHAOSVAR,t,spiralchaosvar,FOREVER);
				obj->pblock->GetValue(PB_SPIRALDIR,t,sdir,FOREVER);
				obj->pblock->GetValue(PB_TIMETRAVEL,t,traveltime,FOREVER);
 				obj->pblock->GetValue(PB_TIMETRAVVAR,t,traveltimevar,FOREVER);

				if (splinetype==OFFSET) 
				{	Point3 TT1,TTT1,T1 = Normalize(pathOb->InterpCurve3D(t, 0, DU)*tm-firstpt);
					pd->psaved[index].TTT1 = (TTT1=Normalize(T1^R));
					TT1 = Normalize(TTT1^T1);
					Matrix4By4 weyw,invweyw;
					weyw[0][0] = T1.x;
					weyw[1][0] = T1.y;
					weyw[2][0] = T1.z;
					weyw[3][0] = 0.0f;
					weyw[0][1] = TT1.x;
					weyw[1][1] = TT1.y;
					weyw[2][1] = TT1.z;
					weyw[3][1] = 0.0f;
					weyw[0][2] = TTT1.x;
					weyw[1][2] = TTT1.y;
					weyw[2][2] = TTT1.z;
					weyw[3][2] = 0.0f;
					weyw[0][3] = firstpt.x;
					weyw[1][3] = firstpt.y;
					weyw[2][3] = firstpt.z;
					weyw[3][3] = 1.0f;
					float aR[4],pout[4];
					MatrixInvert(weyw,invweyw);
					aR[0] = pos.x;
					aR[1] = pos.y;
					aR[2] = pos.z;
					aR[3] = 1.0f;
					Mult4X1(aR,invweyw,pout);
					R.x = pout[0];
					R.y = pout[1];
					R.z = pout[2];
				}

				pd->psaved[index].R = R;
				pd->psaved[index].Tc = t;

				srand(pd->lastseed); // encapsulated around all the RNDs is the save/restore of the randomizer

				if (tdir==CONTRACT) 
					tangchaos = -tangchaos;
				else if (tdir==BOTH) 
					tangchaos *= RNDSign();
				pd->psaved[index].TChaos = tangchaos*(1.0f-RND01()*tangchaosvar);
				if (sdir==CCW) 
					spiralchaos = -spiralchaos;
				else if (sdir==BIDIR) 
					spiralchaos *= RNDSign();
				pd->psaved[index].SChaos = spiralchaos*(1.0f-RND01()*spiralchaosvar);
				if (traveltime < GetTicksPerFrame()) traveltime = GetTicksPerFrame();
				pd->psaved[index].Tt = (int)(traveltime*(1.0f+RND11()*traveltimevar));

				pd->lastseed = rand(); // this saves the random sequence for capturing particles
			}
		}
	}
	else // do this if the particle is nonfound and we're too late to capture it
	{	if ((pd->psaved.Count()<=index)||(!pd->psaved[index].found)) 
			return FALSE;
	}

	TimeValue	intime = t,
				endtime = pd->psaved[index].Tc + pd->psaved[index].Tt,
				difftime;

	int timeloops = 0;

	if ((pd->psaved[index].FoundOnceAlready)&&(t>endtime))
	{	timeloops++;
		difftime = t - endtime;
		while (difftime > pd->psaved[index].Tt)
		{	timeloops++;
			difftime -= pd->psaved[index].Tt;
		}
	}

	if (t>stoptime) 
		endtime = stoptime;

	BOOL lettinggo = (t>=endtime),
		 done=FALSE;
	int tpf=GetTicksPerFrame();
	if (lettinggo) 
		t = endtime - tpf;

	float u;
	int KeepSpeedAlongSpline;

// here begins the maintenance update loop
	while (!done)
	{	if (timeloops < 1)
			u = (float)(tpf + t - pd->psaved[index].Tc) / (float)pd->psaved[index].Tt;
		else
			u = (float)(tpf + difftime) / (float)pd->psaved[index].Tt;

		if (u>1.0f) 
			u = 1.0f;

 		obj->pblock->GetValue(PB_KSPEED,t,KeepSpeedAlongSpline,FOREVER);
		Point3 Sut = pathOb->InterpCurve3D(t,0,u,KeepSpeedAlongSpline)*tm;

//for parallel splines we go here
		if (splinetype==PARALLEL)
		{	Point3 Rnow = pd->psaved[index].R*(1.0f+u*pd->psaved[index].TChaos);
			Point3 PTemp = Sut + Rnow;
			if (pd->psaved[index].SChaos!=0.0f)
			{	Point3 T1 = Normalize(pathOb->InterpCurve3D(t, 0, DU)*tm-pathOb->InterpCurve3D(t,0,0.0f,KeepSpeedAlongSpline)*tm);
				float Thetanow = TWOPI*pd->psaved[index].SChaos*u;
				RotateOnePoint(&PTemp.x,&Sut.x,&T1.x,Thetanow);
			}
			pos = PTemp;
		}
//for offset splines we go here
		else 
		{	float cu;
			Point3 Tt = Normalize(pathOb->InterpCurve3D(t,0,((cu=u+DU)>1.0f?1.0f:cu),KeepSpeedAlongSpline)*tm-pathOb->InterpCurve3D(t, 0, ((cu=u-DU)<0.0f?0.0f:cu))*tm);
			Point3 TTt = Normalize(pd->psaved[index].TTT1^Tt);
			Point3 TTTt = Normalize(Tt^TTt);
			Matrix4By4 mx;
			float aR[4],pout[4];
			Point3 Rnow = pd->psaved[index].R*(1.0f+u*pd->psaved[index].TChaos);
			mx[0][0] = Tt.x;
			mx[1][0] = Tt.y;
			mx[2][0] = Tt.z;
			mx[3][0] = 0.0f;
			mx[0][1] = TTt.x;
			mx[1][1] = TTt.y;
			mx[2][1] = TTt.z;
			mx[3][1] = 0.0f;
			mx[0][2] = TTTt.x;
			mx[1][2] = TTTt.y;
			mx[2][2] = TTTt.z;
			mx[3][2] = 0.0f;
			mx[0][3] = Sut.x;
			mx[1][3] = Sut.y;
			mx[2][3] = Sut.z;
			mx[3][3] = 1.0f;
			aR[0] = Rnow.x;
			aR[1] = Rnow.y;
			aR[2] = Rnow.z;
			aR[3] = 1.0f;
			if (pd->psaved[index].SChaos!=0.0f)
			{	Matrix4By4 mrot,mres;
				float Thetanow = TWOPI*pd->psaved[index].SChaos*u;
				mrot[0][0]=mrot[3][3]=1.0f;
				mrot[0][1]=mrot[0][2]=mrot[0][3]=mrot[1][0]=mrot[1][3]=mrot[2][0]=mrot[2][3]=0.0f;
				mrot[3][0]=mrot[3][1]=mrot[3][2]=0.0f;
				mrot[2][2]=mrot[1][1]=(float)cos(Thetanow);mrot[1][2]=-(mrot[2][1]=(float)sin(Thetanow));
				Mult4X4(mx,mrot,mres);
				Mult4X1(aR,mres,pout);
			}
			else 
				Mult4X1(aR,mx,pout);
			pos.x = pout[0];
			pos.y = pout[1];
			pos.z = pout[2];
		}

		if ((lettinggo)&&(t<endtime)) 
			t = endtime;
		else 
			done = TRUE;
	}

	if (timeloops>0)
		pos += (float)(timeloops)*pd->psaved[index].TraverseVector;

// new velocity calculation HERE

	float splinelen = pathOb->LengthOfCurve(t,0);
	Point3 tanVector = pathOb->TangentCurve3D(t,0,u);
	float baseSpeed = splinelen/(float)pd->psaved[index].Tt;
	
	float u1,u2,u3;
	if (u<0.01f)
	{	u1 = 0.0f;
		u2 = 0.01f;
		u3 = 0.02f;
	}
	else if (u>0.99f)
	{	u1 = 0.98f;
		u2 = 0.99f;
		u3 = 1.0f;
	}
	else
	{	u1 = u - 0.01f;
		u2 = u;
		u3 = u + 0.01f;
	}

	Point3 S1, S2, S3;
	S1 = pathOb->InterpCurve3D(t,0,u1,KeepSpeedAlongSpline)*tm;
	S2 = pathOb->InterpCurve3D(t,0,u2,KeepSpeedAlongSpline)*tm;
	S3 = pathOb->InterpCurve3D(t,0,u3,KeepSpeedAlongSpline)*tm;
	Point3 R1, R2;
	R1 = S2 - S1;
	R2 = S3 - S2;
	float lr1, lr2;
	lr1 = Length(R1);
	lr2 = Length(R2);

	if (lr1<0.001f)
	{	S1 = S2 - 0.001f*tanVector;
		R1 = S2 - S1;
		lr1 = Length(R1);
	}
	if (lr2<0.001f)
	{	S3 = S2 + 0.001f*tanVector;
		R2 = S3 - S2;
		lr2 = Length(R2);
	}

	float theta = (float)acos(DotProd(R1,R2)/(lr1*lr2));
	if (theta < 0.001f)
		vel = baseSpeed * tanVector;
	else
	{	Point3 SpinAxis = Normalize(R1^R2);
		Point3 Rad1 = Normalize(SpinAxis^R1);
		Point3 Rad2 = Normalize(SpinAxis^R2);
		Point3 P1 = 0.5f*(S1+S2);
		Point3 P2 = 0.5f*(S2+S3);

		float denom, paramT;
		denom = Rad2.x - Rad1.x;
		if ((float)fabs(denom)>0.001f)
			paramT = (P1.x - P2.x)/denom;
		else 
		{	denom = Rad2.y - Rad1.y;
			if ((float)fabs(denom)>0.001f)
				paramT = (P1.y - P2.y)/denom;
			else
			{	denom = Rad2.z - Rad1.z;
				if ((float)fabs(denom)>0.001f)
					paramT = (P1.z - P2.z)/denom;
			}
		}
		Point3 omega = (baseSpeed/paramT)*SpinAxis;
		Point3 ArcCenter = P1 + paramT*Rad1;
		Point3 EffectiveArc = pos - ArcCenter;
		vel = omega^EffectiveArc;
	}

	if ((lettinggo)&&(!pd->psaved[index].FoundOnceAlready))
	{	pd->psaved[index].TraverseVector = pos - pd->psaved[index].StartPoint;
		pd->psaved[index].FoundOnceAlready = TRUE;
	}

	if (lettinggo)
	{	vel = invel;
		pd->psaved[index].found = FALSE;
	}

	// Bayboro: for compatibility with the new collision engine (3/7/01)
	*ct = dt;

	return TRUE;
}
*/  // Bayboro 9/18/01

RefTargetHandle PFollowObject::GetReference(int i)
{	switch(i) {
		case PBLK: return(RefTargetHandle)pblock;
		case CUSTNODE: return (RefTargetHandle)custnode;
		default: return NULL;
		}
	}

void PFollowObject::SetReference(int i, RefTargetHandle rtarg) { 
	switch(i) {
		case PBLK: pblock=(IParamBlock*)rtarg; return;
		case CUSTNODE: custnode = (INode *)rtarg; return;
		}
	}

RefResult PFollowObject::NotifyRefChanged( 
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
		case REFMSG_CHANGE:
			{ if (pblock && (pblock==hTarget))
			  { reset=TRUE;
			  }
			  SimpleWSMObject::NotifyRefChanged(changeInt,hTarget,partID,message);
			}
			break;
			}
		default: SimpleWSMObject::NotifyRefChanged(changeInt,hTarget,partID,message);
		}
	return REF_SUCCEED;
	}
#define PFollow_CUSTNAME_CHUNK	0x0100

IOResult PFollowObject::Save(ISave *isave)
	{
	isave->BeginChunk(PFollow_CUSTNAME_CHUNK);		
	isave->WriteWString(custname);
	isave->EndChunk();
	return IO_OK;
	}

IOResult PFollowObject::Load(ILoad *iload)
	{
	IOResult res = IO_OK;
	
	// Default names
	custname = TSTR(_T(" "));

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case PFollow_CUSTNAME_CHUNK: {
				TCHAR *buf;
				res=iload->ReadWStringChunk(&buf);
				custname = TSTR(buf);
				break;
				}
			}
		
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}

	return IO_OK;
	}

/*  // Bayboro 9/18/01
void* PFollowObject::GetInterface(ULONG id) 
{
	switch (id)
	{ 
		case I_NEWPARTOPERATOR: return (IOperatorInterface*)this;
	}
	return Object::GetInterface(id);
}
*/  // Bayboro 9/18/01

void PFollowObject::SetUpModifier(TimeValue t,INode *node)
{
	mf->deflect.obj  = (PFollowObject*)(mf->GetWSMObject(t));
	mf->deflect.partobj = NULL;
	mf->deflect.node = mf->nodeRef;
	mf->deflect.pnode = custnode;
	mf->deflect.tmValid.SetEmpty();		
	mf->deflect.mValid.SetEmpty();
	mf->deflect.badmesh = (custnode==NULL);
	mf->deflect.pobj = NULL;

	int starttime;
	pblock->GetValue(PB_TIMESTART,t,starttime,FOREVER);
	
	if ( ((t==starttime)||(mf->deflect.obj->reset)) && (mf->deflect.pd!=NULL)) 
	{	
		delete mf->deflect.pd;
		mf->deflect.pd = NULL;	
	}
	
	if (mf->deflect.pd == NULL)
	{	
		mf->deflect.pd = new PFollowData;
		int seed;
		pblock->GetValue(PB_SEED,t,seed,FOREVER);
		mf->deflect.pd->lastseed = seed;
		mf->deflect.pd->lasttime = t;
	} 
}

/*  // Bayboro 9/18/01
#define NORMALOP	-1

int PFollowObject::NPOpInterface(TimeValue t,ParticleData *part,float dt,INode *node,int index)
{	
	if (!mf)
		mf = (PFollowMod *)CreateWSMMod(node);
	SetUpModifier(t,node);

	float ct = 0;
	BOOL Unlock = FALSE;
	mf->deflect.CheckCollision2(t,part->position,part->velocity,dt,index,&ct,Unlock,part);

	return (NORMALOP);
}
*/  // Bayboro 9/18/01