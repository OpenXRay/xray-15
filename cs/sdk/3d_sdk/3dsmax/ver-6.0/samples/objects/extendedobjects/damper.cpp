#include "solids.h"

#include "interpik.h"
#include "texutil.h"
#include "macrorec.h"

#ifndef NO_OBJECT_DAMPER	// russom - 11/19/01

#define MAX_PATH_LENGTH 257
#define MAX_STRING_LENGTH  256
#define TESTLIMIT 100

#define A_RENDER			A_PLUGIN1

const float PIover2=1.570796327f;
const float QuarterPI=0.785398163f;

const int tps = TIME_TICKSPERSEC;
const float INSIDEDIAM=0.0f;
const float PISTONMIN=0.1f;
const float PISTONDIAM=99999999.0f;
const float STOPMAX=1.05f*99999999.0f;
const float BOOTDIAM=0.99f*STOPMAX;

static int controlsInit = FALSE;
const Point3 ZAxis=Point3(0.0f,0.0f,1.0f);
#define fourninthPI (4.0f/9.0f)*PI
#define oneeightPI	(1.0f/18.0f)*PI

static Class_ID DAMPER_CLASS_ID(0x46aa537c, 0x712729f0);

#define PBLK		0
#define CUSTNODE 	1
#define CUSTNODE2 	2
typedef float Matrix4By4[4][4];
typedef float Matrix3By3[3][3];

const float EPSILON=0.0001f;
const Point3 Ones=Point3(1.0f,1.0f,1.0f);

class DamperPickOperand;
class DamperObject;

class DamperObject : public DynamHelperObject {
	public:
		static IParamMap *pmapParam;
		DamperObject();
		~DamperObject();
		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message);
	
		static DamperObject *editOb;
		INode *custnode2,*custnode,*thisnode;
		TSTR custname,custname2;
		static IObjParam *ip;
		Matrix3 S;
		static HWND hParams;
		ICustButton *iPick,*iPick2;

		static BOOL creating;
		static DamperPickOperand pickCB;
		BOOL cancelled;
		int NumRefs() {return 3;}
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		// From BaseObject
		TCHAR *GetObjectName();
		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);		
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);		
		
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);
			
		// Animatable methods		
		Class_ID ClassID() {return DAMPER_CLASS_ID;} 
		void DeleteThis() {delete this;}
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);
		void MapKeys(TimeMap *map,DWORD flags);
		// From ref
		RefTargetHandle Clone(RemapDir& remap = NoRemap());		
		// From SimpleObject
		void MakeSaveVertex(Point3 *SaveVertex,int NvertsPerRing,int nfillets,int nsides,int wtype,TimeValue t);
		void BuildMesh(TimeValue t);

		int RenderBegin(TimeValue t, ULONG flags);
		int RenderEnd(TimeValue t);

		BOOL OKtoDisplay(TimeValue t);
		void InvalidateUI();
		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);
		void ShowName();
		void ShowName2();
		void ShowNames();
		void GetWorldBoundBox(TimeValue t, INode *inode, ViewExp* vpt, Box3& box);

		INode *GetEndNode1();
		INode *GetEndNode2();
		Point3 ApplyAtEnd1(TimeValue t);
		Point3 ApplyAtEnd2(TimeValue t);
		Point3 Force(TimeValue t, TimeValue dt);
};

//--- ClassDescriptor and class vars ---------------------------------

class DamperClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new DamperObject();}
	const TCHAR *	ClassName(); 
	SClass_ID		SuperClassID() {return GEOMOBJECT_CLASS_ID;}
	Class_ID		ClassID() {return DAMPER_CLASS_ID;}
	const TCHAR* 	Category(); 
	void			ResetClassParams(BOOL fileReset);
	int 			BeginCreate(Interface *i);
	int 			EndCreate(Interface *i);
	};

static DamperClassDesc DamperDesc;
ClassDesc* GetDamperDesc() {return &DamperDesc;}

class DamperPickOperand : 
		public PickModeCallback,
		public PickNodeCallback {
	public:		
		DamperObject *po;
		int dodist,repi;

		DamperPickOperand() {po=NULL;}

		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);

		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);

		BOOL RightClick(IObjParam *ip, ViewExp *vpt) { return TRUE; }
		BOOL Filter(INode *node);
		
		PickNodeCallback *GetFilter() {return this;}
	};

IParamMap *DamperObject::pmapParam;
IObjParam *DamperObject::ip    = NULL;
BOOL DamperObject::creating    = FALSE;
HWND DamperObject::hParams		=NULL;
DamperPickOperand DamperObject::pickCB;
DamperObject *DamperObject::editOb	=NULL;

//--- Parameter map/block descriptors -------------------------------

#define SIZEFACTOR (float(TIME_TICKSPERSEC)/120.0f)

#define PB_ENDSMETHOD			0
#define PB_NOREFLENGTH			1

#define PB_RENDERABLE			2
	
#define PB_MOUNTDIA				3
#define PB_MOUNTLEN				4
#define PB_CYLDIA				5
#define PB_CYLLEN				6
#define PB_CYLSIDES				7
#define PB_CYLFILLET1			8
#define PB_CYLFILLET1SEGS		9
#define PB_CYLFILLET2			10
#define PB_CYLFILLET2SEGS		11
#define PB_INSIDEDIA			12
#define PB_SMOOTHCYL			13

#define PB_PISTONDIA			14
#define PB_PISTONLEN			15
#define PB_PISTONSIDES			16
#define PB_SMOOTHPISTON			17

#define PB_ENABLEBOOT			18
#define PB_BOOTDIA1				19
#define PB_BOOTDIA2				20
#define PB_BOOTSIDES			21
#define PB_BOOTFOLDS			22
#define PB_BOOTRESOLUTION		23
#define PB_BOOTSTOPDIA			24
#define PB_BOOTSTOPLEN			25
#define PB_BOOTSETBACK			26
#define PB_BOOTSTOPFILLET		27
#define PB_BOOTSTOPFILLETSEGS	28
#define PB_SMOOTHBOOT			29

#define PB_DYNTYPE				30
#define PB_DYNDRAG				31
#define PB_DYNDRAGUNITS			32
#define PB_SPRINGDIR			33
#define PB_DYNFORCE				34
#define PB_DYNFORCEUNITS		35

#define PB_MAPMEMAPME			36

static int endmethodIDs[] = {IDC_SPRING_REFS,IDC_SPRING_DIMS};

static int dyntypeIDs[] = {IDC_DAMPERON,IDC_ACTUATORON};

static int dragunitsIDs[] = {IDC_DRAGUNITS_LBINSEC,IDC_DRAGUNITS_NEWTMS};

static int forceunitIDs[] = {IDC_FORCEUNITS_LBS,IDC_FORCEUNITS_NEWTS};

static int springdirIDs[] = {IDC_SPRING_COMP,IDC_SPRING_EXT,IDC_SPRING_BOTH};

//
// Parameters

static ParamUIDesc descDamperParam1[] = {

	// End Method
	ParamUIDesc(PB_ENDSMETHOD,TYPE_RADIO,endmethodIDs,2),

	// No reference version Length
	ParamUIDesc(
		PB_NOREFLENGTH,
		EDITTYPE_UNIVERSE,
		IDC_DAMPER_LENGTH,IDC_DAMPER_LENGTHSPIN,
		0.0f,99999999.0f,
		0.1f),	
		
	// IsRenderable?
	ParamUIDesc(PB_RENDERABLE,TYPE_SINGLECHEKBOX,IDC_DAMPER_RENDERTHIS),			

	// Mounting Stud Diameter
	ParamUIDesc(
		PB_MOUNTDIA,
		EDITTYPE_UNIVERSE,
		IDC_STUDDIA,IDC_STUDDIASPIN,
		0.1f,99999999.0f,
		0.1f),	
		
	// Mounting Stud Len
	ParamUIDesc(
		PB_MOUNTLEN,
		EDITTYPE_UNIVERSE,
		IDC_STUDLEN,IDC_STUDLENSPIN,
		0.1f,99999999.0f,
		0.1f),	
		
	// Main Diameter
	ParamUIDesc(
		PB_CYLDIA,
		EDITTYPE_UNIVERSE,
		IDC_CYLDIA,IDC_CYLDIASPIN,
		0.1f,99999999.0f,
		0.1f),	
		
	// Main Len
	ParamUIDesc(
		PB_CYLLEN,
		EDITTYPE_UNIVERSE,
		IDC_CYLLEN,IDC_CYLLENSPIN,
		0.1f,99999999.0f,
		0.1f),	

	// Stud/Cylinder Sides
	ParamUIDesc(
		PB_CYLSIDES,
		EDITTYPE_INT,
		IDC_CYLSIDES,IDC_CYLSIDESSPIN,
		3.0f,100.0f,
		1.0f),	

	// Fillet 1 Size
	ParamUIDesc(
		PB_CYLFILLET1,
		EDITTYPE_UNIVERSE,
		IDC_CYLFILLET1,IDC_CYLFILLET1SPIN,
		0.0f,99999999.0f,
		0.1f),	

	// Fillet 1 Segments
	ParamUIDesc(
		PB_CYLFILLET1SEGS,
		EDITTYPE_INT,
		IDC_FILLET1SEGS,IDC_FILLET1SEGSSPIN,
		0.0f,100.0f,
		1.0f),	
		
	// Fillet 2 Size
	ParamUIDesc(
		PB_CYLFILLET2,
		EDITTYPE_UNIVERSE,
		IDC_CYLFILLET2,IDC_CYLFILLET2SPIN,
		0.0f,99999999.0f,
		0.1f),	

	// Fillet 2 Segments
	ParamUIDesc(
		PB_CYLFILLET2SEGS,
		EDITTYPE_INT,
		IDC_FILLET2SEGS,IDC_FILLET2SEGSSPIN,
		0.0f,100.0f,
		1.0f),	

	// ID
	ParamUIDesc(
		PB_INSIDEDIA,
		EDITTYPE_UNIVERSE,
		IDC_CYLINSIDEDIA,IDC_CYLINSIDEDIASPIN,
		INSIDEDIAM,99999999.0f,
		0.1f),	

	// Smooth Cylinder
	ParamUIDesc(PB_SMOOTHCYL,TYPE_SINGLECHEKBOX,IDC_SMOOTHCYL),			

// Rod Parameters

	// Piston Dia
	ParamUIDesc(
		PB_PISTONDIA,
		EDITTYPE_UNIVERSE,
		IDC_RODDIA,IDC_RODDIASPIN,
		PISTONMIN,PISTONDIAM,
		0.1f),	

	// Piston Length
	ParamUIDesc(
		PB_PISTONLEN,
		EDITTYPE_UNIVERSE,
		IDC_RODLEN,IDC_RODLENSPIN,
		0.1f,99999999.0f,
		0.1f),	

	// Piston Sides
	ParamUIDesc(
		PB_PISTONSIDES,
		EDITTYPE_INT,
		IDC_RODSIDES,IDC_RODSIDESSPIN,
		3.0f,100.0f,
		1.0f),	

	// Smooth Rod
	ParamUIDesc(PB_SMOOTHPISTON,TYPE_SINGLECHEKBOX,IDC_SMOOTHROD),			

// Boot parameters

	// Enable Boot
	ParamUIDesc(PB_ENABLEBOOT,TYPE_SINGLECHEKBOX,IDC_BOOTON),			

	// Minor Dia
	ParamUIDesc(
		PB_BOOTDIA1,
		EDITTYPE_UNIVERSE,
		IDC_BOOTDIA1,IDC_BOOTDIA1SPIN,
		0.0f,BOOTDIAM,
		0.1f),	

	// Major Diameter
	ParamUIDesc(
		PB_BOOTDIA2,
		EDITTYPE_UNIVERSE,
		IDC_BOOTDIA2,IDC_BOOTDIA2SPIN,
		0.0f,BOOTDIAM,
		0.1f),	
		
	// Boot Sides
	ParamUIDesc(
		PB_BOOTSIDES,
		EDITTYPE_INT,
		IDC_FOLDSIDES,IDC_FOLDSIDESSPIN,
		3.0f,100.0f,
		1.0f),	

	// Boot Folds
	ParamUIDesc(
		PB_BOOTFOLDS,
		EDITTYPE_INT,
		IDC_FOLDS,IDC_FOLDSSPIN,
		1.0f,100.0f,
		1.0f),	

	// Boot Resolution
	ParamUIDesc(
		PB_BOOTRESOLUTION,
		EDITTYPE_INT,
		IDC_FOLDRES,IDC_FOLDRESSPIN,
		4.0f,100.0f,
		1.0f),	

	// Stop Dia
	ParamUIDesc(
		PB_BOOTSTOPDIA,
		EDITTYPE_UNIVERSE,
		IDC_BOOTSTOPDIA,IDC_BOOTSTOPDIASPIN,
		0.0f,STOPMAX,
		0.1f),	

	// Stop Len
	ParamUIDesc(
		PB_BOOTSTOPLEN,
		EDITTYPE_UNIVERSE,
		IDC_STOPTHICK,IDC_STOPTHICKSPIN,
		0.0f,99999999.0f,
		0.1f),	

	// Setback
	ParamUIDesc(
		PB_BOOTSETBACK,
		EDITTYPE_UNIVERSE,
		IDC_STOPSETBACK,IDC_STOPSETBACKSPIN,
		0.0f,99999999.0f,
		0.1f),	

	// Stop Fillet
	ParamUIDesc(
		PB_BOOTSTOPFILLET,
		EDITTYPE_UNIVERSE,
		IDC_STOPFILLET,IDC_STOPFILLETSPIN,
		0.0f,99999999.0f,
		0.1f),	

	// Stop Fillet Segments
	ParamUIDesc(
		PB_BOOTSTOPFILLETSEGS,
		EDITTYPE_INT,
		IDC_STOPFILLETSEGS,IDC_STOPFILLETSEGSSPIN,
		0.0f,100.0f,
		1.0f),	

	// Smooth Boot
	ParamUIDesc(PB_SMOOTHBOOT,TYPE_SINGLECHEKBOX,IDC_SMOOTHBOOT),			

//Dynamics Parameters

	// Dyn Object Type
	ParamUIDesc(PB_DYNTYPE,TYPE_RADIO,dyntypeIDs,2),

	// Drag
	ParamUIDesc(
		PB_DYNDRAG,
		EDITTYPE_FLOAT,
		IDC_DRAG,IDC_DRAGSPIN,
		0.0f,99999999.0f,
		0.1f),	

	// Dyn Drag Units
	ParamUIDesc(PB_DYNDRAGUNITS,TYPE_RADIO,dragunitsIDs,2),

	// Dyn Spring Dir
	ParamUIDesc(PB_SPRINGDIR,TYPE_RADIO,springdirIDs,3),
			
	// Force
	ParamUIDesc(
		PB_DYNFORCE,
		EDITTYPE_FLOAT,
		IDC_FORCE,IDC_FORCESPIN,
		-99999999.0f,99999999.0f,
		0.1f),	

	// Dyn Force units
	ParamUIDesc(PB_DYNFORCEUNITS,TYPE_RADIO,forceunitIDs,2),

	// Texture Coords
	ParamUIDesc(PB_MAPMEMAPME,TYPE_SINGLECHEKBOX,IDC_DAMPER_MAPME),			
};

#define DAMPERPARAMSDESC_LENGTH 37

ParamBlockDescID descDamperVer0[] = {
	{ TYPE_INT, NULL, FALSE, 0 }, // End Method
	{ TYPE_FLOAT, NULL, TRUE, 1 }, // No Ref Length
	{ TYPE_INT, NULL, FALSE, 2 }, // Renderable
	{ TYPE_FLOAT, NULL, TRUE, 3 }, // Stud Dia
	{ TYPE_FLOAT, NULL, TRUE, 4 }, // Stud Len
	{ TYPE_FLOAT, NULL, TRUE, 5 }, // Cyl Dia
	{ TYPE_FLOAT, NULL, TRUE, 6 }, // Cyl len
	{ TYPE_INT, NULL, TRUE, 7 }, // Stud/Cyl Sidecount
	{ TYPE_FLOAT, NULL, TRUE, 8 }, // Fillet 1
	{ TYPE_INT, NULL, TRUE, 9 }, // Fillet 1 segments
	{ TYPE_FLOAT, NULL, TRUE, 10 }, // Fillet 2
	{ TYPE_INT, NULL, TRUE, 11 }, // Fillet 2 segments
	{ TYPE_FLOAT, NULL, TRUE, 12 }, // Inside Dia
	{ TYPE_INT, NULL, FALSE, 13 }, // Smooth Cyl

	{ TYPE_FLOAT, NULL, TRUE, 14 }, // Rod Dia
	{ TYPE_FLOAT, NULL, TRUE, 15 }, // Rod Len
	{ TYPE_INT, NULL, TRUE, 16 }, // Rod Side Count
	{ TYPE_INT, NULL, FALSE, 17 }, // Smooth Rod

	{ TYPE_INT, NULL, FALSE, 18 }, // Boot On
	{ TYPE_FLOAT, NULL, TRUE, 19 }, // Boot Dia Small
	{ TYPE_FLOAT, NULL, TRUE, 20 }, // Boot Dia Large
	{ TYPE_INT, NULL, TRUE, 21 },  // Boot Sides
	{ TYPE_INT, NULL, TRUE, 22 },  // Boot Folds
	{ TYPE_INT, NULL, TRUE, 23 },  // Boot Resolution
	{ TYPE_FLOAT, NULL, TRUE, 24 }, // Stop Dia
	{ TYPE_FLOAT, NULL, TRUE, 25 }, // Stop Len
	{ TYPE_FLOAT, NULL, TRUE, 26 }, // Stop Setback
	{ TYPE_FLOAT, NULL, TRUE, 27 }, // Stop Fillet Size
	{ TYPE_INT, NULL, TRUE, 28 },  // Stop Fillet segments
	{ TYPE_INT, NULL, FALSE, 29 },  // Smooth Boot

	{ TYPE_INT, NULL, FALSE, 30 },  // Dynamics Object Type
	{ TYPE_FLOAT, NULL, TRUE, 31 }, // Drag value
	{ TYPE_INT, NULL, FALSE, 32 },  // Drag Units
	{ TYPE_INT, NULL, FALSE, 33 }, // Damper Direction
	{ TYPE_FLOAT, NULL, TRUE, 34 }, // Force value
	{ TYPE_INT, NULL, FALSE, 35 },  // Force Units

	{ TYPE_INT, NULL, FALSE, 36 },  // texture coords
};

#define PBLOCK_DAMPER_LENGTH	37

// Array of old versions
// static ParamVersionDesc versions[] = {ParamVersionDesc(descDamperVer0,24,0)};

#define NUM_OLDVERSIONS	0
#define CURRENT_VERSION	0

// Current version
static ParamVersionDesc curVersion(descDamperVer0,PBLOCK_DAMPER_LENGTH,CURRENT_VERSION);


class CreateMDamperProc : public MouseCallBack,ReferenceMaker {
	private:
		IObjParam *ip;
		void Init(IObjParam *i) {ip=i;}
		CreateMouseCallBack *createCB;	
		INode *CloudNode;
		DamperObject *SSBlizObject;
		int attachedToNode;
		IObjCreate *createInterface;
		ClassDesc *cDesc;
		Matrix3 mat;  // the nodes TM relative to the CP
		Point3 p0,p1;
		IPoint2 sp0, sp1;
		BOOL square;

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
		
		CreateMDamperProc()
			{
			ignoreSelectionChange = FALSE;
			}
//		int createmethod(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
		int proc( HWND hwnd, int msg, int point, int flag, IPoint2 m );
	};


class CreateCPartRestoreNode : public RestoreObj {
	public:   		
		DamperObject *obj;
		TSTR name,namer;
		INode *save,*saver;
		CreateCPartRestoreNode(DamperObject *o) {
			obj = o; name=TSTR(o->custname);
			save=o->custnode;
			}
		void Restore(int isUndo)
		{ if (isUndo) { namer=TSTR(obj->custname);saver=obj->custnode;	}
		  obj->custname = name;
		  obj->custnode=save;
		  if (obj->hParams) obj->ShowName();
			}
		void Redo() 
		{ obj->custname = namer;
		  obj->custnode=saver;
		  if (obj->hParams) obj->ShowName();
		}
		TSTR Description() {return GetString(IDS_AP_COMPICK);}
	};

class CreateCPart2RestoreNode : public RestoreObj {
	public:   		
		DamperObject *obj;
		TSTR name,namer;
		INode *save,*saver;
		CreateCPart2RestoreNode(DamperObject *o) {
			obj = o; name=TSTR(o->custname2);
			save=o->custnode2;
			}
		void Restore(int isUndo)
		{ if (isUndo) { namer=TSTR(obj->custname2);	}
		  obj->custname2 = name;
		  obj->custnode2=save;
		  if (obj->hParams) obj->ShowName2();
			}
		void Redo() 
		{ obj->custname2 = namer;
		  obj->custnode2 = saver;
		  if (obj->hParams) obj->ShowName2();
		}
		TSTR Description() {return GetString(IDS_AP_COMPICK2);}
	};

#define CID_CREATEDAMPERMODE	CID_USER +75

class CreateMDamperMode : public CommandMode {		
	public:		
		CreateMDamperProc proc;
		IObjParam *ip;
		DamperObject *obj;
		void Begin( IObjCreate *ioc, ClassDesc *desc ) { proc.Begin( ioc, desc ); }
		void End() { proc.End(); }
		void JumpStart(IObjParam *i,DamperObject*o);

		int Class() {return CREATE_COMMAND;}
		int ID() { return CID_CREATEDAMPERMODE; }
		MouseCallBack *MouseProc(int *numPoints) {*numPoints = 10000; return &proc;}
		ChangeForegroundCallback *ChangeFGProc() {return CHANGE_FG_SELECTED;}
		BOOL ChangeFG( CommandMode *oldMode ) { return (oldMode->ChangeFGProc() != CHANGE_FG_SELECTED); }
		void EnterMode() 
		{ GetCOREInterface()->PushPrompt(GetString(IDS_AP_CREATEMODE));
		  SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR)));
		}
		void ExitMode() {GetCOREInterface()->PopPrompt();SetCursor(LoadCursor(NULL, IDC_ARROW));}
	};
static CreateMDamperMode theCreateMDamperMode;

void DamperClassDesc::ResetClassParams(BOOL fileReset)
	{
	}

RefResult CreateMDamperProc::NotifyRefChanged(
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
		 	if ( SSBlizObject && CloudNode==hTarget ) {
				// this will set camNode== NULL;
				theHold.Suspend();
				DeleteReference(0);
				theHold.Resume();
				goto endEdit;
				}
			// fall through

		case REFMSG_TARGET_DELETED:		
			if (SSBlizObject && CloudNode==hTarget ) {
				endEdit:
				if (createInterface->GetCommandMode()->ID() == CID_STDPICK) 
				{ if (SSBlizObject->creating) 
						{  theCreateMDamperMode.JumpStart(SSBlizObject->ip,SSBlizObject);
							createInterface->SetCommandMode(&theCreateMDamperMode);
					    } 
				  else {createInterface->SetStdCommandMode(CID_OBJMOVE);}
				}
#ifdef _OSNAP
				SSBlizObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
				SSBlizObject->EndEditParams( (IObjParam*)createInterface, 0, NULL);
				SSBlizObject  = NULL;				
				CloudNode    = NULL;
				CreateNewObject();	
				attachedToNode = FALSE;
				}
			break;		
		}
	return REF_SUCCEED;
	}

void CreateMDamperProc::Begin( IObjCreate *ioc, ClassDesc *desc )
	{
	createInterface = ioc;
	cDesc           = desc;
	attachedToNode  = FALSE;
	createCB        = NULL;
	CloudNode         = NULL;
	SSBlizObject       = NULL;
	CreateNewObject();
	}

void CreateMDamperProc::CreateNewObject()
	{
	createInterface->GetMacroRecorder()->BeginCreate(cDesc);
	SSBlizObject = (DamperObject*)cDesc->Create();
	lastPutCount  = theHold.GetGlobalPutCount();
	
	// Start the edit params process
	if ( SSBlizObject ) {
		SSBlizObject->BeginEditParams( (IObjParam*)createInterface, BEGIN_EDIT_CREATE, NULL );
#ifdef _OSNAP
		SSBlizObject->SetAFlag(A_OBJ_LONG_CREATE);
#endif
		}	
	}

//LACamCreationManager::~LACamCreationManager
void CreateMDamperProc::End()
{ if ( SSBlizObject ) 
	{ 
#ifdef _OSNAP
		SSBlizObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
	SSBlizObject->EndEditParams( (IObjParam*)createInterface, 
	                    	          END_EDIT_REMOVEUI, NULL);
		if ( !attachedToNode ) 
		{	// RB 4-9-96: Normally the hold isn't holding when this 
			// happens, but it can be in certain situations (like a Damper view paste)
			// Things get confused if it ends up with undo...
			createInterface->GetMacroRecorder()->Cancel();
			theHold.Suspend(); 
			delete SSBlizObject;
			SSBlizObject = NULL;
			theHold.Resume();
			if (theHold.GetGlobalPutCount()!=lastPutCount) 
				GetSystemSetting(SYSSET_CLEAR_UNDO);
		}  else if ( CloudNode ) 
		{	theHold.Suspend();
			DeleteReference(0);  // sets cloudNode = NULL
			theHold.Resume();}
	}
}

void CreateMDamperMode::JumpStart(IObjParam *i,DamperObject *o)
	{
	ip  = i;
	obj = o;
	//MakeRefByID(FOREVER,0,svNode);
	obj->BeginEditParams(i,BEGIN_EDIT_CREATE,NULL);
	}

int DamperClassDesc::BeginCreate(Interface *i)
{	
	IObjCreate *iob = i->GetIObjCreate();
	theCreateMDamperMode.Begin(iob,this);
	iob->PushCommandMode(&theCreateMDamperMode);
	return TRUE;
}

int DamperClassDesc::EndCreate(Interface *i)
	{
	theCreateMDamperMode.End();
	i->RemoveMode(&theCreateMDamperMode);
	macroRec->EmitScript();  // 10/00
	return TRUE;
	}

int CreateMDamperProc::proc(HWND hwnd,int msg,int point,int flag,
				IPoint2 m )
{	
	int res=TRUE;	
	ViewExp *vpx = createInterface->GetViewport(hwnd); 
	assert( vpx );

	SuspendSetKeyMode();

	switch ( msg ) {
		case MOUSE_POINT:
			switch ( point ) {
				case 0:
					assert( SSBlizObject );					
					vpx->CommitImplicitGrid(m, flag );
					if ( createInterface->SetActiveViewport(hwnd) ) {
						ResumeSetKeyMode();
						return FALSE;
						}

					if (createInterface->IsCPEdgeOnInView()) { 
						res = FALSE;
						goto done;
						}

					if ( attachedToNode ) {
				   		// send this one on its way
#ifdef _OSNAP
						SSBlizObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
				   		SSBlizObject->EndEditParams( (IObjParam*)createInterface, 0, NULL);
              createInterface->GetMacroRecorder()->EmitScript();
						// Get rid of the reference.
						if (CloudNode) {
							theHold.Suspend();
							DeleteReference(0);
							theHold.Resume();
							}
						// new object
						CreateNewObject();   // creates SSBlizObject
						}

				   	theHold.Begin();	 // begin hold for undo
					mat.IdentityMatrix();

					// link it up
					CloudNode = createInterface->CreateObjectNode( SSBlizObject);
					SSBlizObject->thisnode=CloudNode;
					attachedToNode = TRUE;
					assert( CloudNode );					
					createCB = SSBlizObject->GetCreateMouseCallBack();
					createInterface->SelectNode( CloudNode );
					
					// Reference the new node so we'll get notifications.
					theHold.Suspend();
					MakeRefByID( FOREVER, 0, CloudNode);
					theHold.Resume();
					mat.IdentityMatrix();
				default:				
					if (createCB) {						
						res = createCB->proc(vpx,msg,point,flag,m,mat);
//						macroRec->Disable();   // 10/00
						createInterface->SetNodeTMRelConstPlane(CloudNode,mat);
//						macroRec->Enable();

						if (res==CREATE_ABORT)
							goto abort;
						if (res==CREATE_STOP)
						    theHold.Accept(GetString(IDS_AP_CREATE));	 
						
						createInterface->RedrawViews(createInterface->GetTime());   //DS
						}

					break;
					
				}			
			break;

		case MOUSE_MOVE:
			if (createCB) {				
				res = createCB->proc(vpx,msg,point,flag,m,mat);
//				macroRec->Disable();   // 10/00
				createInterface->SetNodeTMRelConstPlane(CloudNode,mat);
//				macroRec->Enable();
				if (res==CREATE_ABORT) 
					goto abort;
				if (res==CREATE_STOP)
					theHold.Accept(GetString(IDS_AP_CREATE));	// TH
				createInterface->RedrawViews(createInterface->GetTime(),REDRAW_INTERACTIVE);		//DS		
				}
			break;

	case MOUSE_PROPCLICK:
		createInterface->SetStdCommandMode(CID_OBJMOVE);
		break;
		case MOUSE_ABORT: 
	if (createCB)
	{ res = createCB->proc(vpx,msg,point,flag,m,mat);
//		macroRec->Disable();   // 10/00
	  createInterface->SetNodeTMRelConstPlane(CloudNode, mat);
//		macroRec->Enable();
	  if (res==CREATE_ABORT)
	      goto abort;
	  if (res==CREATE_STOP)
	  {
#ifdef _OSNAP
         SSBlizObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
	    theHold.Accept(GetString(IDS_AP_CREATE));	
	  }
	  createInterface->RedrawViews(createInterface->GetTime()); 
		break;
	}
	abort:
		assert( SSBlizObject );
#ifdef _OSNAP
		SSBlizObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
		SSBlizObject->EndEditParams( (IObjParam*)createInterface,0,NULL);
		theHold.Cancel();	 // deletes both the Cloudera and target.
		if (theHold.GetGlobalPutCount()!=lastPutCount) 
					GetSystemSetting(SYSSET_CLEAR_UNDO);
		SSBlizObject=NULL;
		createInterface->RedrawViews(createInterface->GetTime());
		CreateNewObject();	
		attachedToNode = FALSE;
		res = FALSE;
		break;
	
		case MOUSE_FREEMOVE:
			SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR)));
		#ifdef _OSNAP  //PREVIEW SNAP
			if(createCB) {
				res = createCB->proc(vpx,msg,point,flag,m,mat);
			}
			else
			{
				assert( SSBlizObject );					
				createCB = SSBlizObject->GetCreateMouseCallBack();
			}
		#endif
		vpx->TrackImplicitGrid(m);
			break;
	}

	done:
	ResumeSetKeyMode();
	if ((res == CREATE_STOP)||(res==CREATE_ABORT))
		vpx->ReleaseImplicitGrid();
	createInterface->ReleaseViewport(vpx); 
	return res;
}

BOOL DamperObject::OKtoDisplay(TimeValue t) 
	{ return (mesh.getNumVerts()>1);
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

BOOL DamperPickOperand::Filter(INode *node)
{	if (node)
	{	ObjectState os = node->GetObjectRef()->Eval(po->ip->GetTime());
//		if (os.obj->IsParticleSystem() || os.obj->SuperClassID()!=GEOMOBJECT_CLASS_ID) 
//		{	node = NULL;
//			return FALSE;
//		}
		node->BeginDependencyTest();
		po->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
		if(node->EndDependencyTest())
		{	node = NULL;
			return FALSE;
		}
	}

	return node ? TRUE : FALSE;
}

BOOL DamperPickOperand::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
{	INode *node = ip->PickNode(hWnd,m,this);
	
	if (node) 
	{	ObjectState os = node->GetObjectRef()->Eval(ip->GetTime());
//		if ((os.obj->SuperClassID()!=GEOMOBJECT_CLASS_ID)||(!IsGEOM(os.obj)))
//		{	node = NULL;
//			return FALSE;
//		}
		node->BeginDependencyTest();
		po->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
		if(node->EndDependencyTest())
		{	node = NULL;
			return FALSE;
		}
	}
	return node ? TRUE : FALSE;
}

void DamperObject::ShowName()
{ TSTR name=(custnode ? custname : TSTR(GetString(IDS_AP_NONE)));
  SetWindowText(GetDlgItem(hParams, IDC_SPRING_OBJ1NAME), name);
}
void DamperObject::ShowName2()
{ TSTR name=(custnode2 ? custname2 : TSTR(GetString(IDS_AP_NONE)));
  SetWindowText(GetDlgItem(hParams, IDC_SPRING_OBJ2NAME), name);
}
void DamperObject::ShowNames()
{ ShowName();ShowName2();
}

BOOL DamperPickOperand::Pick(IObjParam *ip,ViewExp *vpt)
{	INode *node = vpt->GetClosestHit();
	assert(node);
	INodeTab nodes;
	nodes.SetCount(1);nodes[0]=node;
	theHold.Begin();
	if (dodist)
	{ theHold.Put(new CreateCPart2RestoreNode(po));
	  if (po->custnode2) po->ReplaceReference(CUSTNODE2,node,TRUE);
	  else po->MakeRefByID(FOREVER,CUSTNODE2,node);	
	  po->custname2 = TSTR(node->GetName());
	  // Automatically check show result and do one update
	  po->ShowName2();	
	}
	else
	{ theHold.Put(new CreateCPartRestoreNode(po));
	  if (po->custnode) po->ReplaceReference(CUSTNODE,node,TRUE);
	  else po->MakeRefByID(FOREVER,CUSTNODE,node);	
	  po->custname = TSTR(node->GetName());
	  // Automatically check show result and do one update
	  po->ShowName();	
	}
	theHold.Accept(GetString(IDS_AP_COMPICK));
	po->pmapParam->Invalidate();
	ip->FlashNodes(&nodes);
	nodes.Resize(0);
	po->ivalid.SetEmpty();
	po->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	if (po->creating) {
		theCreateMDamperMode.JumpStart(ip,po);
		ip->SetCommandMode(&theCreateMDamperMode);
		ip->RedrawViews(ip->GetTime());
		return FALSE;
	} else {return TRUE;}
}

void DamperPickOperand::EnterMode(IObjParam *ip)
{	ICustButton *iBut;
	if (dodist) 
	{ iBut=GetICustButton(GetDlgItem(po->hParams,IDC_SPRING_PICKOBJECT2));
	  TurnButton(po->hParams,IDC_SPRING_PICKOBJECT1,FALSE);
	}
	else 
	{ iBut=GetICustButton(GetDlgItem(po->hParams,IDC_SPRING_PICKOBJECT1));
	  TurnButton(po->hParams,IDC_SPRING_PICKOBJECT2,FALSE);
	}
	if (iBut) iBut->SetCheck(TRUE);
	ReleaseICustButton(iBut);
	GetCOREInterface()->PushPrompt(GetString(IDS_AP_PICKMODE));
}

void DamperPickOperand::ExitMode(IObjParam *ip)
{	ICustButton *iBut;
	if (dodist) 
	{ iBut=GetICustButton(GetDlgItem(po->hParams,IDC_SPRING_PICKOBJECT2));
	  TurnButton(po->hParams,IDC_SPRING_PICKOBJECT1,TRUE);
	}
	else 
	{ iBut=GetICustButton(GetDlgItem(po->hParams,IDC_SPRING_PICKOBJECT1));
	  TurnButton(po->hParams,IDC_SPRING_PICKOBJECT2,TRUE);
	}
	if (iBut) iBut->SetCheck(FALSE);
	ReleaseICustButton(iBut);
    GetCOREInterface()->PopPrompt();
}

//-- ParticleDlgProc ------------------------------------------------

class DamperParamDlg : public ParamMapUserDlgProc {
	public:
		DamperObject *po;
		float bootmax,bootmin;

		DamperParamDlg(DamperObject *p) {po=p;m_bUIValid=FALSE;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void Update(TimeValue t);
		void DeleteThis() {delete this;}
		void BootParams(BOOL ison);
		void ActuatorParams(BOOL ison);
		void DamperParams(BOOL ison);
		void BoundParams(BOOL ison);
		void MainChange(TimeValue t);
		void PistonChange(TimeValue t);
		void BootStop(TimeValue t);
		void StopChange(TimeValue t);
		void StopFillet(TimeValue t);
		BOOL m_bUIValid;
		void InvalidateUI (HWND hWnd) { if (hWnd) InvalidateRect (hWnd, NULL, false); 	m_bUIValid = true; 	}
};

void DamperParamDlg::StopChange(TimeValue t)
{ float outdiam,indiam,maxval;
  po->pblock->GetValue(PB_BOOTSTOPDIA,t,outdiam,FOREVER);
  po->pblock->GetValue(PB_BOOTDIA1,t,indiam,FOREVER);
  maxval=0.99f*outdiam;
  BOOL change;
  if (change=(maxval<indiam))
  { po->pblock->SetValue(PB_BOOTDIA1,t,maxval); }
  if (bootmax>maxval) bootmax=maxval;
  FixFSpinnerLimits(po->hParams,IDC_BOOTDIA1SPIN,bootmin,bootmax,change);
}

void DamperParamDlg::BootStop(TimeValue t)
{ float outdiam,indiam,outmin;
  po->pblock->GetValue(PB_BOOTDIA2,t,outdiam,FOREVER);
  po->pblock->GetValue(PB_BOOTDIA1,t,indiam,FOREVER);
  outmin=1.01f*indiam;
  BOOL change;
  if (change=(outdiam<outmin)) po->pblock->SetValue(PB_BOOTDIA2,t,outmin);
  FixFSpinnerLimits(po->hParams,IDC_BOOTDIA2SPIN,outmin,BOOTDIAM,change);
}
void DamperParamDlg::StopFillet(TimeValue t)
{ float stopthick,stopfillet;
  po->pblock->GetValue(PB_BOOTSTOPLEN,t,stopthick,FOREVER);
  po->pblock->GetValue(PB_BOOTSTOPFILLET,t,stopfillet,FOREVER);
  BOOL change;
  if (change=(stopthick<stopfillet)) po->pblock->SetValue(PB_BOOTSTOPFILLET,t,stopthick);
  FixFSpinnerLimits(po->hParams,IDC_STOPFILLETSPIN,0.0f,stopthick,change);
}
void DamperParamDlg::PistonChange(TimeValue t)
{ float outdiam,indiam,stopdiam,minstop;
  po->pblock->GetValue(PB_BOOTDIA1,t,outdiam,FOREVER);
  po->pblock->GetValue(PB_PISTONDIA,t,indiam,FOREVER);
  po->pblock->GetValue(PB_BOOTSTOPDIA,t,stopdiam,FOREVER);
  bootmin=1.01f*indiam;
  BOOL changes,changed;
  if (changed=(outdiam<bootmin)) {po->pblock->SetValue(PB_BOOTDIA1,t,bootmin);outdiam=bootmin;}
  minstop=1.05f*indiam;
  if (changes=(stopdiam<minstop)) po->pblock->SetValue(PB_BOOTSTOPDIA,t,minstop);  
  float maxdiam=0.99f*stopdiam; if (maxdiam<bootmax) bootmax=maxdiam;
  if (outdiam>bootmax) {po->pblock->SetValue(PB_BOOTDIA1,t,bootmax);changed=TRUE;}
  FixFSpinnerLimits(po->hParams,IDC_BOOTDIA1SPIN,bootmin,bootmax,changed);
  FixFSpinnerLimits(po->hParams,IDC_BOOTSTOPDIASPIN,minstop,STOPMAX,changes);
  StopChange(t);
  BootStop(t);
}
void DamperParamDlg::MainChange(TimeValue t)
{ float outdiam,indiam,maxin;BOOL change;
  po->pblock->GetValue(PB_CYLDIA,t,outdiam,FOREVER);
  po->pblock->GetValue(PB_INSIDEDIA,t,indiam,FOREVER);
  maxin=0.99f*outdiam;
  if (change=(maxin<indiam)) po->pblock->SetValue(PB_INSIDEDIA,t,maxin);
  FixFSpinnerLimits(po->hParams,IDC_CYLINSIDEDIASPIN,INSIDEDIAM,maxin,change);
  float maxpiston=0.95f*outdiam,pistondiam;
  po->pblock->GetValue(PB_PISTONDIA,t,pistondiam,FOREVER);
  if (change=(maxpiston<pistondiam)){ pistondiam=maxpiston;po->pblock->SetValue(PB_PISTONDIA,t,maxpiston);}
  FixFSpinnerLimits(po->hParams,IDC_RODDIASPIN,INSIDEDIAM,maxpiston,change);
  float maxboot=0.99f*outdiam,bootdiam;bootmin=1.01f*pistondiam;
  po->pblock->GetValue(PB_BOOTDIA1,t,bootdiam,FOREVER);
  if (change=(maxboot<bootdiam)) po->pblock->SetValue(PB_BOOTDIA1,t,maxboot);
  FixFSpinnerLimits(po->hParams,IDC_BOOTDIA1SPIN,bootmin,bootdiam,change);
  PistonChange(t);
	StopFillet(t);
}

void DamperParamDlg::Update(TimeValue t)
{ if (!po->editOb) return;
  po->ShowNames();
  int emethod;
  po->pblock->GetValue(PB_ENDSMETHOD,0,emethod,FOREVER);
  BoundParams(!emethod);
  int booton;
  po->pblock->GetValue(PB_ENABLEBOOT,t,booton,FOREVER);
  BootParams(booton);
  if (booton)
  { float stopdiam,maindiam,pistondiam;
    po->pblock->GetValue(PB_BOOTSTOPDIA,t,stopdiam,FOREVER);
    po->pblock->GetValue(PB_CYLDIA,t,maindiam,FOREVER);
    bootmax=(stopdiam>maindiam?maindiam:stopdiam);
    po->pblock->GetValue(PB_PISTONDIA,t,pistondiam,FOREVER);
    bootmin=1.01f*pistondiam;
    MainChange(t);
  }
/*  if ((emethod)||(!po->thisnode))
  { po->iPick->Disable();
    po->iPick2->Disable();
  }
  else if (!po->iPick->IsEnabled())
  { po->iPick->Enable(TRUE);
    po->iPick2->Enable(TRUE);
  }*/
}

void DamperParamDlg::BoundParams(BOOL ison)
{ 
	BOOL ok=ison;
	EnableWindow(GetDlgItem(po->hParams,IDC_DAMPERON),ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_ACTUATORON),ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_DAMPER_PINPIN_LABEL), !ison);

	if (ison)
	{ 
		SpinnerOff(po->hParams,IDC_DAMPER_LENGTHSPIN);
		ok=(po->thisnode!=NULL);
		
		int act;
		po->pblock->GetValue(PB_DYNTYPE,0,act,FOREVER);
		ActuatorParams(act);
		DamperParams(!act);
	}
	else
	{ 
		SpinnerOn(po->hParams,IDC_DAMPER_LENGTHSPIN);
		ActuatorParams(FALSE);
		DamperParams(FALSE);
	}
	if ((po->iPick->IsEnabled()!=ok)||(po->iPick2->IsEnabled()!=ok))
	{ 
		TurnButton(po->hParams,IDC_SPRING_PICKOBJECT2,ok);
		TurnButton(po->hParams,IDC_SPRING_PICKOBJECT1,ok);
		EnableWindow(GetDlgItem(po->hParams,IDC_SPRING_OBJ2NAMECAPTION),ison);
		EnableWindow(GetDlgItem(po->hParams,IDC_SPRING_OBJ1NAMECAPTION),ison);
		EnableWindow(GetDlgItem(po->hParams,IDC_SPRING_OBJ2NAME),ison);
		EnableWindow(GetDlgItem(po->hParams,IDC_SPRING_OBJ1NAME),ison);
	}
	// labels have changed, so mark UI as dirty
	//
	m_bUIValid = FALSE;
	
}
void DamperParamDlg::BootParams(BOOL ison)
{ 
	
	EnableWindow(GetDlgItem(po->hParams,IDC_SMOOTHBOOT),ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_BOOT_MINDIA_LABEL    ),ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_BOOT_MAXDIA_LABEL    ),ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_BOOT_SIDES_LABEL     ),ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_BOOT_FOLDS_LABEL     ),ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_BOOT_RESOLUTION_LABEL),ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_BOOT_STOPDIA_LABEL   ),ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_BOOT_STOPTHICK_LABEL ),ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_BOOT_SETBACK_LABEL   ),ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_BOOT_STOPFILLET_LABEL),ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_BOOT_FILLETSEGS_LABEL),ison);
	
	if (ison)
	{ 
		SpinnerOn(po->hParams,IDC_BOOTDIA1SPIN);
		SpinnerOn(po->hParams,IDC_BOOTDIA2SPIN);
		SpinnerOn(po->hParams,IDC_FOLDSIDESSPIN);
		SpinnerOn(po->hParams,IDC_FOLDSSPIN);
		SpinnerOn(po->hParams,IDC_FOLDRESSPIN);
		SpinnerOn(po->hParams,IDC_BOOTSTOPDIASPIN);
		SpinnerOn(po->hParams,IDC_STOPTHICKSPIN);
		SpinnerOn(po->hParams,IDC_STOPSETBACKSPIN);
		SpinnerOn(po->hParams,IDC_STOPFILLETSPIN);
		SpinnerOn(po->hParams,IDC_STOPFILLETSEGSSPIN);
	}
	else
	{ 
		SpinnerOff(po->hParams,IDC_BOOTDIA1SPIN);
		SpinnerOff(po->hParams,IDC_BOOTDIA2SPIN);
		SpinnerOff(po->hParams,IDC_FOLDSIDESSPIN);
		SpinnerOff(po->hParams,IDC_FOLDSSPIN);
		SpinnerOff(po->hParams,IDC_FOLDRESSPIN);
		SpinnerOff(po->hParams,IDC_BOOTSTOPDIASPIN);
		SpinnerOff(po->hParams,IDC_STOPTHICKSPIN);
		SpinnerOff(po->hParams,IDC_STOPSETBACKSPIN);
		SpinnerOff(po->hParams,IDC_STOPFILLETSPIN);
		SpinnerOff(po->hParams,IDC_STOPFILLETSEGSSPIN);
	}

	// labels have changed, so mark UI as dirty
	//
	m_bUIValid = FALSE;

}

void DamperParamDlg::DamperParams(BOOL ison)
{
	EnableWindow(GetDlgItem(po->hParams,IDC_DRAGUNITS_LBINSEC),ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_DRAGUNITS_NEWTMS), ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_SPRING_COMP),	   ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_SPRING_EXT),       ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_SPRING_BOTH),      ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_DAMPER_DRAG_LABEL       ),	 ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_DAMPER_DRAGMEASURE_LABEL),   ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_DAMPER_DAMPERWORKS_LABEL),   ison);
	if (ison)
	{ 
		SpinnerOn(po->hParams,IDC_DRAGSPIN);
	}
	else
	{ 
		SpinnerOff(po->hParams,IDC_DRAGSPIN);
	}

	// labels have changed, so mark UI as dirty
	//
	m_bUIValid = FALSE;
}

void DamperParamDlg::ActuatorParams(BOOL ison)
{

	EnableWindow(GetDlgItem(po->hParams,IDC_FORCEUNITS_LBS),  ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_FORCEUNITS_NEWTS),ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_ACTUATOR_FORCE_LABEL       ), ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_ACTUATOR_FORCEMEASURE_LABEL), ison);
 
	if (ison)
	{ 
		SpinnerOn(po->hParams,IDC_FORCESPIN); 
	}
	else
	{ 
		SpinnerOff(po->hParams,IDC_FORCESPIN);
	}
	// labels have changed, so mark UI as dirty
	//
	m_bUIValid = FALSE;

}

BOOL DamperParamDlg::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{	switch (msg) {
		case WM_INITDIALOG: {
			// initialize the damper param labels by marking the UI as dirty
			//
			m_bUIValid = FALSE; 

			po->iPick = GetICustButton(GetDlgItem(hWnd,IDC_SPRING_PICKOBJECT1));
			po->iPick->SetType(CBT_CHECK);
			po->iPick->SetHighlightColor(GREEN_WASH);
			po->iPick2 = GetICustButton(GetDlgItem(hWnd,IDC_SPRING_PICKOBJECT2));
			po->iPick2->SetType(CBT_CHECK);
			po->iPick2->SetHighlightColor(GREEN_WASH);
			Update(t);
			return FALSE;	// stop default keyboard focus - DB 2/27  
			}
		case WM_DESTROY:
			// Release all our Custom Controls
			ReleaseICustButton(po->iPick);
			ReleaseICustButton(po->iPick2);
			return FALSE;
		case CC_SPINNER_BUTTONUP:
			{ 
			  if (HIWORD(wParam)) 
			  { switch ( LOWORD(wParam) ) 
				{
				 case IDC_CYLDIASPIN: 
				  { MainChange(t);
					po->pmapParam->Invalidate();
				  }
					break;
				 case IDC_BOOTDIA2SPIN: 
				  { BootStop(t);
					po->pmapParam->Invalidate();
				  }
					break;
				 case IDC_STOPFILLETSPIN: 
				  { StopFillet(t);
						po->pmapParam->Invalidate();
				  }
					break;
				 case IDC_BOOTSTOPDIASPIN: 
				 case IDC_RODDIASPIN: 
				  { PistonChange(t);
					po->pmapParam->Invalidate();
				  }
					break;				
				}
			  }
			return TRUE;
			}
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{ case IDC_SPRING_PICKOBJECT1: 
				   {if (po->ip->GetCommandMode()->ID() == CID_STDPICK) 
					{ if (po->creating) 
						{  theCreateMDamperMode.JumpStart(po->ip,po);
							po->ip->SetCommandMode(&theCreateMDamperMode);
						} else {po->ip->SetStdCommandMode(CID_OBJMOVE);}
					} else 
						{ po->pickCB.po = po;	
					     po->pickCB.dodist=0;
						  po->ip->SetPickMode(&po->pickCB);
						}
					break;
					}
			  case IDC_SPRING_PICKOBJECT2: 
				   {if (po->ip->GetCommandMode()->ID() == CID_STDPICK) 
					{ if (po->creating) 
						{  theCreateMDamperMode.JumpStart(po->ip,po);
							po->ip->SetCommandMode(&theCreateMDamperMode);
						} else {po->ip->SetStdCommandMode(CID_OBJMOVE);}
					} else 
						{ po->pickCB.po = po;	
					     po->pickCB.dodist=1;
						  po->ip->SetPickMode(&po->pickCB);
						}
					break;
					}
			  case IDC_SPRING_REFS: 
				  { BoundParams(TRUE);				  
				  }
				  break;				   
			  case IDC_SPRING_DIMS: 
				  { BoundParams(FALSE);
					if (po->ip->GetCommandMode()->ID() == CID_STDPICK) 
					{ if (po->creating) 
						{  theCreateMDamperMode.JumpStart(po->ip,po);
							po->ip->SetCommandMode(&theCreateMDamperMode);
						} else {po->ip->SetStdCommandMode(CID_OBJMOVE);}
					} 
				  }
				  break;			  
			  case IDC_BOOTDIA2: 
				  { BootStop(t);				  
				  }
				  break;				   
			  case IDC_BOOTSTOPDIA: 
			  case IDC_RODDIA: 
				  { PistonChange(t);				  
				  }
				  break;				   
			  case IDC_CYLDIA: 
				  { MainChange(t);				  
				  }
				  break;				   
			  case IDC_BOOTON: 
				  { int ison;po->pblock->GetValue(PB_ENABLEBOOT,0,ison,FOREVER);
					BootParams(ison);
				  }
				  break;				   
			  case IDC_DAMPERON: 
				  { //int ison;po->pblock->GetValue(PB_DYNTYPE,0,ison,FOREVER);
					ActuatorParams(FALSE);
					DamperParams(TRUE);
				  }
				  break;				   
			  case IDC_ACTUATORON: 
				  { //int ison;po->pblock->GetValue(PB_DYNTYPE,0,ison,FOREVER);
					ActuatorParams(TRUE);
					DamperParams(FALSE);
				  }
				  break;
			}
		default:
			// redraw the UI for the dirty labels
			//
			if (!m_bUIValid) InvalidateUI(hWnd);
			return FALSE;
		}
	return TRUE;
}

//--- DamperObject Methods--------------------------------------------

DamperObject::DamperObject()
{	int FToTick=(int)((float)TIME_TICKSPERSEC/(float)GetFrameRate());
	int length = PBLOCK_DAMPER_LENGTH;
	MakeRefByID(FOREVER, 0, CreateParameterBlock(descDamperVer0, length, CURRENT_VERSION));
	assert(pblock);

	pblock->SetValue(PB_ENDSMETHOD,0,1);
	pblock->SetValue(PB_NOREFLENGTH,0,2.0f);
	pblock->SetValue(PB_RENDERABLE,0,1);

	pblock->SetValue(PB_MOUNTDIA,0,0.5f);	
	pblock->SetValue(PB_MOUNTLEN,0,0.2f);	
	pblock->SetValue(PB_CYLDIA,0,1.0f);
	pblock->SetValue(PB_CYLLEN,0,1.0f);
	pblock->SetValue(PB_CYLSIDES,0,8);
	pblock->SetValue(PB_CYLFILLET1,0,0.0f);
	pblock->SetValue(PB_CYLFILLET1SEGS,0,0);
	pblock->SetValue(PB_CYLFILLET2,0,0.0f);
	pblock->SetValue(PB_CYLFILLET2SEGS,0,0);
	pblock->SetValue(PB_INSIDEDIA,0,0.0f);
	pblock->SetValue(PB_SMOOTHCYL,0,1);

	pblock->SetValue(PB_PISTONDIA,0,0.2f);
	pblock->SetValue(PB_PISTONLEN,0,1.0f);
	pblock->SetValue(PB_PISTONSIDES,0,8);
	pblock->SetValue(PB_SMOOTHPISTON,0,1);

	pblock->SetValue(PB_ENABLEBOOT,0,0);
	pblock->SetValue(PB_BOOTDIA1,0,0.25f);
	pblock->SetValue(PB_BOOTDIA2,0,1.0f);
	pblock->SetValue(PB_BOOTSIDES,0,8);
	pblock->SetValue(PB_BOOTFOLDS,0,4);
	pblock->SetValue(PB_BOOTRESOLUTION,0,4);
	pblock->SetValue(PB_BOOTSTOPDIA,0,0.4f);
	pblock->SetValue(PB_BOOTSTOPLEN,0,0.2f);
	pblock->SetValue(PB_BOOTSETBACK,0,0.2f);
	pblock->SetValue(PB_BOOTSTOPFILLET,0,0.0f);
	pblock->SetValue(PB_BOOTSTOPFILLETSEGS,0,0);
	pblock->SetValue(PB_SMOOTHBOOT,0,1);

	pblock->SetValue(PB_DYNTYPE,0,0);
	pblock->SetValue(PB_DYNDRAG,0,0.0f);
	pblock->SetValue(PB_DYNDRAGUNITS,0,0);
	pblock->SetValue(PB_SPRINGDIR,0,2);
	pblock->SetValue(PB_DYNFORCE,0,0.0f);
	pblock->SetValue(PB_DYNFORCEUNITS,0,0);

	pblock->SetValue(PB_MAPMEMAPME,0,TRUE);

	S.IdentityMatrix();
	iPick=NULL;
	iPick2=NULL;
	thisnode=NULL;
	custnode=NULL;
	custnode2=NULL;
	custname=TSTR(_T(" "));
	custname2=TSTR(_T(" "));
}

DamperObject::~DamperObject()
{	DeleteAllRefsFromMe();
}

void DamperObject::BeginEditParams(
		IObjParam *ip, ULONG flags,Animatable *prev)
{	SimpleObject::BeginEditParams(ip,flags,prev);
	this->ip = ip;
	editOb   = this;
	if (flags&BEGIN_EDIT_CREATE) {
		creating = TRUE;
	} else { creating = FALSE; }


	if (pmapParam) 
	{	pmapParam->SetParamBlock(pblock);
	} else 
	{		pmapParam = CreateCPParamMap(
			descDamperParam1,DAMPERPARAMSDESC_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_DAMPER),
			GetString(IDS_AP_DAMPPARAMETERS),
			0);		
		}
	hParams=pmapParam->GetHWnd();
	if (pmapParam) pmapParam->SetUserDlgProc(new DamperParamDlg(this));
}

void DamperObject::EndEditParams(
		IObjParam *ip, ULONG flags,Animatable *next)
{	SimpleObject::EndEditParams(ip,flags,next);
	this->ip = NULL;
	editOb = NULL;
	if (flags&END_EDIT_REMOVEUI) {
		DestroyCPParamMap(pmapParam);
		pmapParam  = NULL;
		hParams=NULL;
		}
	ip->ClearPickMode();
	ip= NULL;
	creating = FALSE;
}

void DamperObject::MapKeys(TimeMap *map,DWORD flags)
{
}  

static TriObject *GetTriObject(TimeValue t,Object *obj,Interval &valid,BOOL &needsDel)
	{	
	needsDel = FALSE;
	if (!obj) return NULL;
	ObjectState os = obj->Eval(t);
	valid &= os.Validity(t);
	if (os.obj->IsSubClassOf(triObjectClassID)) {
		return (TriObject*)os.obj;
	} else {
		if (os.obj->CanConvertToType(triObjectClassID)) {
			Object *oldObj = os.obj;
			TriObject *tobj = (TriObject*)os.obj->ConvertToType(t,triObjectClassID);			
			needsDel = (tobj != oldObj);			
			return tobj;
			}
		}
	return NULL;
	}

void DamperObject::GetWorldBoundBox(
		TimeValue t, INode *inode, ViewExp* vpt, Box3& box)
	{	thisnode=inode;    
		SimpleObject::GetWorldBoundBox(t,inode,vpt,box);
	}  

int DamperObject::RenderBegin(TimeValue t, ULONG flags)
{	SetAFlag(A_RENDER);
	int renderme;
	pblock->GetValue(PB_RENDERABLE,t,renderme,FOREVER);
	if (!renderme) 
	{	MeshInvalid();
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}
	return 0;
}

int DamperObject::RenderEnd(TimeValue t)
{	ClearAFlag(A_RENDER);
	int renderme;
 	pblock->GetValue(PB_RENDERABLE,t,renderme,FOREVER);
	if (!renderme) 
	{	MeshInvalid();
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}
	return 0;
}

INode *DamperObject::GetEndNode1()
{	if (custnode)
		return (custnode);
	else
		return (NULL);
}

INode *DamperObject::GetEndNode2()
{	if (custnode2) 
		return (custnode2);
	else
		return (NULL);
}

Point3 DamperObject::ApplyAtEnd1(TimeValue t)
{	if (custnode)
	{	Matrix3 mat1 = custnode->GetObjTMAfterWSM(t);
		return (mat1.GetRow(3));
	}
	else
		return (Zero);
}

Point3 DamperObject::ApplyAtEnd2(TimeValue t)
{	if (custnode2)
	{	Matrix3 mat2 = custnode2->GetObjTMAfterWSM(t);
		return (mat2.GetRow(3));
	}
	else
		return (Zero);
}

#define COMPRESSION	0
#define EXTENSION	1
#define	BIDIRECT	2

#define POUNDSSECONDSPERINCH	0
#define NEWTONSECONDSPERMETER	1

#define DAMPER		0
#define ACTUATOR	1

#define POUNDS		0
#define NEWTONS		1

Point3 DamperObject::Force(TimeValue t, TimeValue dt)
{	if ((custnode)&&(custnode2))
	{	Matrix3 mat1now, mat2now, mat1before, mat2before;
		mat1now = custnode->GetObjTMAfterWSM(t);
		mat2now = custnode2->GetObjTMAfterWSM(t);

		float lennow, lenbefore;
		Point3 forcedir = (mat1now.GetRow(3) - mat2now.GetRow(3));
		lennow = Length(forcedir);
		if (lennow < 0.01f)
			return (Zero);
		else
			forcedir = Normalize(forcedir);

		int cylindertype;
		pblock->GetValue(PB_DYNTYPE,t,cylindertype,ivalid);
		if (cylindertype == DAMPER)
		{	mat1before = custnode->GetObjTMAfterWSM(t - dt);
			mat2before = custnode2->GetObjTMAfterWSM(t - dt);
			lenbefore = Length(mat1before.GetRow(3) - mat2before.GetRow(3));
			float difflen = lenbefore - lennow;
			if ((float)fabs(difflen) < 0.01f)
				return (Zero);

			int dampertype;
			pblock->GetValue(PB_SPRINGDIR,t,dampertype,ivalid);
			if ((dampertype == COMPRESSION)&&(difflen < 0.0f))
				return (Zero);
			else if ((dampertype == EXTENSION)&&(difflen > 0.0f))
				return (Zero);

			difflen *= GetMeterMult();

			float diffvel = (float)tps * difflen / (float)dt;

			float drag;
			pblock->GetValue(PB_DYNDRAG,t,drag,ivalid);
			int dragunits;
			pblock->GetValue(PB_DYNDRAGUNITS,t,dragunits,ivalid);
			if (dragunits == POUNDSSECONDSPERINCH)
				drag *= 175.55441f;
			return (drag * diffvel * forcedir);
		}
		else // cylindertype must be ACTUATOR
		{	float pushforce;
			pblock->GetValue(PB_DYNFORCE,t,pushforce,ivalid);
			int pushforceunits;
			pblock->GetValue(PB_DYNFORCEUNITS,t,pushforceunits,ivalid);
			if (pushforceunits == POUNDS)
				pushforce *= 4.4591f;
			return (pushforce * forcedir);
		}
	}
	else
		return (Zero);
}

BOOL DamperObject::HasUVW() 
{ 	BOOL genUVs;
	Interval v;
	pblock->GetValue(PB_MAPMEMAPME, 0, genUVs, v);
	return genUVs; 
}

void DamperObject::SetGenUVW(BOOL sw) 
{  	if (sw==HasUVW()) 
		return;
	pblock->SetValue(PB_MAPMEMAPME,0, sw);
}

void DamperObject::BuildMesh(TimeValue t)
{	if (TestAFlag(A_RENDER))
	{	int renderme;
		pblock->GetValue(PB_RENDERABLE,0,renderme,ivalid);
		if (!renderme)
		{	mesh.setNumVerts(0);
			mesh.setNumFaces(0);
			mesh.setNumTVerts(0);
			mesh.setNumTVFaces(0);
			mesh.InvalidateGeomCache();
			return;
		}
	}
	
	if (!ivalid.InInterval(t))
	{	ivalid=FOREVER;
		int createfree;
		pblock->GetValue(PB_ENDSMETHOD,0,createfree,ivalid);
		if ((!createfree)&&((!custnode)||(!custnode2))) 
			createfree = 1;
		if (!createfree) 
			ivalid.SetInstant(t);
		Matrix3 mat1,mat2;
		srand(56576);
		int s1 = rand();
		float Lf = 0.0f; // Lf is the free length of the damper

		Matrix3 Tlocal(1);
		if (createfree) 
			pblock->GetValue(PB_NOREFLENGTH,t,Lf,ivalid);
		else
		{	
			// Martell 4/14/01: Fix for order of ops bug.
			float ztmp=RND11(); float ytmp=RND11(); float xtmp=RND11();
			Point3 RV = Point3(xtmp,ytmp,ztmp);
			mat1 = custnode->GetObjTMAfterWSM(t);
			mat2 = (custnode==custnode2?mat1:custnode2->GetObjTMAfterWSM(t));

			Matrix3 mato1(1), mato2(1);

			Point3 pos1 = custnode->GetObjOffsetPos();
			mato1.PreTranslate(pos1);
			Quat quat1 = custnode->GetObjOffsetRot();
			PreRotateMatrix(mato1, quat1);
			ScaleValue scaleValue1 = custnode->GetObjOffsetScale();
			ApplyScaling(mato1, scaleValue1);
			mato1 = Inverse(mato1);

			Point3 pos2 = custnode2->GetObjOffsetPos();
			mato2.PreTranslate(pos2);
			Quat quat2 = custnode2->GetObjOffsetRot();
			PreRotateMatrix(mato2, quat2);
			ScaleValue scaleValue2 = custnode2->GetObjOffsetScale();
			ApplyScaling(mato2, scaleValue2);
			mato2 = Inverse(mato2);

			S = thisnode->GetObjTMBeforeWSM(t);
//			Point3 P1 = mat1.GetRow(3) + mat1*mato1.GetRow(3),
//				   P2 = mat2.GetRow(3) + mat2*mato2.GetRow(3);
			Point3 P1 = mat1*mato1.GetRow(3),
				   P2 = mat2*mato2.GetRow(3);
			Matrix3 SI = Inverse(S);
			Point3 P0 = SI*P1;
			Matrix3 T1 = mat1;
			T1.NoTrans();
			Point3 RVw = T1 * RV;
			Lf = Length(P2 - P1);
			pblock->SetValue(PB_NOREFLENGTH,0,Lf);
			ivalid.SetInstant(t);
			Point3 Zw;
			if (Lf < 0.01f)
				Zw = Normalize(P1);
			else
				Zw = Normalize(P2 - P1);
			Point3 Xw = Normalize(RVw ^ Zw),
				   Yw = Normalize(Zw ^ Xw);	
			SI.NoTrans();
			Point3 Xs = SI * Xw,
				   Ys = SI * Yw,
				   Zs = SI * Zw;
			Tlocal.SetRow(0,Xs);
			Tlocal.SetRow(1,Ys);
			Tlocal.SetRow(2,Zs);
			Tlocal.SetRow(3,P0);
		}

		float rstud, lstud, rcyl, lcyl, z_endofcyl;
		pblock->GetValue(PB_MOUNTDIA,t,rstud,ivalid);
		rstud *= 0.5f;
		pblock->GetValue(PB_MOUNTLEN,t,lstud,ivalid);
		pblock->GetValue(PB_CYLDIA,t,rcyl,ivalid);
		rcyl *= 0.5f;
		pblock->GetValue(PB_CYLLEN,t,lcyl,ivalid);
		z_endofcyl = lstud + lcyl;

		int cylsides, cylfillet1segs, cylfillet2segs;
		pblock->GetValue(PB_CYLSIDES,t,cylsides,ivalid);
		if (cylsides<3) cylsides = 3;
		pblock->GetValue(PB_CYLFILLET1SEGS,t,cylfillet1segs,ivalid);
		if (cylfillet1segs<0) cylfillet1segs = 0;
		pblock->GetValue(PB_CYLFILLET2SEGS,t,cylfillet2segs,ivalid);
		if (cylfillet2segs<0) cylfillet2segs = 0;

		float cylfillet1, cylfillet2, insidedia;
		pblock->GetValue(PB_CYLFILLET1,t,cylfillet1,ivalid);
		if (cylfillet1<0.0f) cylfillet1 = 0.0f;
		pblock->GetValue(PB_CYLFILLET2,t,cylfillet2,ivalid);
		if (cylfillet2<0.0f) cylfillet2 = 0.0f;

		pblock->GetValue(PB_INSIDEDIA,t,insidedia,ivalid);
		insidedia *= 0.5f;

		int smoothcyl, smoothrod, smoothboot;
		pblock->GetValue(PB_SMOOTHCYL,t,smoothcyl,ivalid);
		pblock->GetValue(PB_SMOOTHPISTON,t,smoothrod,ivalid);

		float roddia, rodlen;
		pblock->GetValue(PB_PISTONDIA,t,roddia,ivalid);
		roddia *= 0.5f;
		pblock->GetValue(PB_PISTONLEN,t,rodlen,ivalid);

		int rodsides;
		pblock->GetValue(PB_PISTONSIDES,t,rodsides,ivalid);
		if (rodsides < 3) rodsides = 3;

		int IsBooted;
		pblock->GetValue(PB_ENABLEBOOT,t,IsBooted,ivalid);

		int bootsides, bootfolds, bootres, bootfilletsegs;
		float bootdia1, bootdia2, bootstopdia, bootstoplen, bootstoploc, bootstopfillet, diffboot; 
		if (IsBooted)
		{	pblock->GetValue(PB_BOOTDIA1,t,bootdia1,ivalid);
			bootdia1 *= 0.5f;
			pblock->GetValue(PB_BOOTDIA2,t,bootdia2,ivalid);
			bootdia2 *= 0.5f;
			diffboot = bootdia2 - bootdia1;
			pblock->GetValue(PB_BOOTSIDES,t,bootsides,ivalid);
			if (bootsides<3) bootsides = 3;
			pblock->GetValue(PB_BOOTFOLDS,t,bootfolds,ivalid);
			if (bootfolds<1) bootfolds = 1;
			pblock->GetValue(PB_BOOTRESOLUTION,t,bootres,ivalid);
			if (bootres<4) bootres = 4;
			pblock->GetValue(PB_BOOTSTOPDIA,t,bootstopdia,ivalid);
			bootstopdia *= 0.5f;
			pblock->GetValue(PB_BOOTSTOPLEN,t,bootstoplen,ivalid);
			pblock->GetValue(PB_BOOTSETBACK,t,bootstoploc,ivalid);
			pblock->GetValue(PB_BOOTSTOPFILLET,t,bootstopfillet,ivalid);
			if (bootstopfillet<0.0f) bootstopfillet = 0.0f;
			pblock->GetValue(PB_BOOTSTOPFILLETSEGS,t,bootfilletsegs,ivalid);
			if (bootfilletsegs<0) bootfilletsegs = 0;
			pblock->GetValue(PB_SMOOTHBOOT,t,smoothboot,ivalid);
		}

		int NvertsPerRingStud = cylsides;
		int NRingsStud = 6 + ((cylfillet1segs>0)?cylfillet1segs:0) 
			               + ((cylfillet2segs>0)?cylfillet2segs:0) 
						   + (insidedia>0.0f?2:0);
//		int NRingsStud = 6 + ((cylfillet1segs>0)&&(cylfillet1>0.0f)?cylfillet1segs:0) 
//			               + ((cylfillet2segs>0)&&(cylfillet2>0.0f)?cylfillet2segs:0) 
//						   + (insidedia>0.0f?2:0);
		int NvertsStud = NRingsStud*NvertsPerRingStud;
		int NtvertsStud = NRingsStud*(NvertsPerRingStud+1);
		int NfacesStud = (NRingsStud-1)*2*NvertsPerRingStud;

		int NvertsPerRingRod = rodsides;
		int NRingsRod = 4;
		int NvertsRod = NRingsRod*NvertsPerRingRod;
		int NtvertsRod = NRingsRod*(NvertsPerRingRod+1);
		int NfacesRod = (NRingsRod-1)*2*NvertsPerRingRod;

		int NvertsPerRingBoot=0, NRingsBoot=0, NvertsBoot=0, NfacesBoot=0, NtvertsBoot;
		if (IsBooted)
		{	NvertsPerRingBoot = bootsides;
			NRingsBoot = 5 + bootres*bootfolds + ((bootfilletsegs>0)?bootfilletsegs:0);
//			NRingsBoot = 5 + bootres*bootfolds + ((bootfilletsegs>0)&&(bootstopfillet>0.0f)?bootfilletsegs:0);
			NvertsBoot = NRingsBoot*NvertsPerRingBoot;
			NtvertsBoot = NRingsBoot*(NvertsPerRingBoot+1);
			NfacesBoot = (NRingsBoot-2)*2*NvertsPerRingBoot;
		}
		float NdivsBoot = (float)bootres*(float)bootfolds;
		int NRingsStop = 4 + ((bootfilletsegs>0)?bootfilletsegs:0);
//		int NRingsStop = 4 + ((bootfilletsegs>0)&&(bootstopfillet>0.0f)?bootfilletsegs:0);

		int Nverts = NvertsStud + NvertsRod + NvertsBoot;
		int Ntverts = NtvertsStud + NtvertsRod + NtvertsBoot;
		int Nfaces = NfacesStud + NfacesRod + NfacesBoot;
		int Ntfaces = Nfaces;

		int mapmenow;
		pblock->GetValue(PB_MAPMEMAPME,0,mapmenow,FOREVER);

		mesh.setNumVerts(Nverts);
		mesh.setNumFaces(Nfaces);

		if (mapmenow)
		{	mesh.setNumTVerts(Nverts + NRingsStud + NRingsRod + NRingsBoot);
			mesh.setNumTVFaces(Nfaces);
		}
		else
		{	mesh.setNumTVerts(0);
			mesh.setNumTVFaces(0);
		}

		int i,j;
		float u, fverts;

		Point3 *RingCoordsStud = new Point3[NvertsPerRingStud];
		assert(RingCoordsStud);
		fverts = (float)NvertsPerRingStud;
		for (i=0; i<NvertsPerRingStud; i++)
		{	u = (float)i/fverts;
			RingCoordsStud[i].z = u;
			u *= TWOPI;
			RingCoordsStud[i].x = (float)cos(u);
			RingCoordsStud[i].y = (float)sin(u);
		}

		Point3 *RingCoordsRod = new Point3[NvertsPerRingRod];
		assert(RingCoordsRod);
		fverts = (float)NvertsPerRingRod;
		for (i=0; i<NvertsPerRingRod; i++)
		{	u = (float)i/fverts;
			RingCoordsRod[i].z = u;
			u *= TWOPI;
			RingCoordsRod[i].x = (float)cos(u);
			RingCoordsRod[i].y = (float)sin(u);
		}

		Point3 *RingCoordsBoot = new Point3[NvertsPerRingBoot];
		assert(RingCoordsBoot);
		if (IsBooted)
		{	fverts = (float)NvertsPerRingBoot;
			for (i=0; i<NvertsPerRingBoot; i++)
			{	u = (float)i/fverts;
				RingCoordsBoot[i].z = u;
				u *= TWOPI;
				RingCoordsBoot[i].x = (float)cos(u);
				RingCoordsBoot[i].y = (float)sin(u);
			}
		}

		int thisvert = 0;
		float thisradius, thisz, fltNverts, thisxtex, thisangle;

		fltNverts = (float)NvertsPerRingStud;

// vertices for CYLINDER AND STUD
		for (i=0 ; i < NRingsStud ; i++)
		{	thisxtex = (float)i/(float)(NRingsStud-1);
			if (i == 0)
			{	thisradius = 0.0f;
				thisz = 0.0f;
			}
			else if (i == 1)
			{	thisradius = rstud;
				thisz = 0.0f;
			}
			else if (i == 2)
			{	thisradius = rstud;
				thisz = lstud;
			}
			else if (i == 3)
			{	thisz = lstud;
				if (cylfillet1segs == 0)
					thisradius = rcyl;
				else
					thisradius = rcyl - cylfillet1;
			}
			else if (i < 4+cylfillet1segs)
			{	thisangle = (float)(i - 3)*PIover2/(float)(cylfillet1segs);
				thisradius = cylfillet1*(float)sin(thisangle) + rcyl - cylfillet1;
				thisz = lstud + cylfillet1 - cylfillet1*(float)cos(thisangle);
			}
			else if (i == 4+cylfillet1segs)
			{	thisradius = rcyl;
				if (cylfillet2segs == 0)
					thisz = lstud + lcyl;
				else
					thisz = lstud + lcyl - cylfillet2;
			}
			else if (i < 5+cylfillet1segs+cylfillet2segs) 
			{	thisangle = (float)(i - cylfillet1segs - 4)*PIover2/(float)(cylfillet2segs);
				thisradius = rcyl - cylfillet2 + cylfillet2*(float)cos(thisangle);
				thisz = lstud + lcyl - cylfillet2 + cylfillet2*(float)sin(thisangle);
			}
			else if (i == 5+cylfillet1segs+cylfillet2segs)
			{	thisradius = insidedia;
				thisz = lstud + lcyl;
			}
			else if (i == 6+cylfillet1segs+cylfillet2segs)
			{	thisradius = insidedia;
				thisz = lstud + cylfillet1;
			}
			else if (i == 7+cylfillet1segs+cylfillet2segs)
			{	thisradius = 0.0f;
				thisz = lstud + cylfillet1;
			}
			for (j=0 ; j < NvertsPerRingStud ; j++)
			{	if (createfree)
					mesh.setVert(thisvert,Point3(thisradius*RingCoordsStud[j].x,
								                 thisradius*RingCoordsStud[j].y,
												 thisz));
				else
					mesh.setVert(thisvert,Tlocal*Point3(thisradius*RingCoordsStud[j].x,
								                        thisradius*RingCoordsStud[j].y,
												        thisz));
				if (mapmenow)
					mesh.tVerts[thisvert] = Point3(thisxtex,RingCoordsStud[j].z,0.0f);
				thisvert++;
			}
			if (mapmenow)
				mesh.tVerts[Nverts+i] = Point3(thisxtex, 0.999f, 0.0f);
		}

// vertices for ROD
		for (i=0 ; i < NRingsRod ; i++)
		{	thisxtex = (float)i/(float)(NRingsRod-1);
			if (i == 0)
			{	thisradius = 0.0f;
				thisz = Lf - rodlen;
			}
			else if (i == 1)
			{	thisradius = roddia;
				thisz = Lf - rodlen;
			}
			else if (i == 2)
			{	thisradius = roddia;
				thisz = Lf;
			}
			else if (i == 3)
			{	thisradius = 0.0f;
				thisz = Lf;
			}
			for (j=0 ; j < NvertsPerRingRod ; j++)
			{	if (createfree)
					mesh.setVert(thisvert,Point3(thisradius*RingCoordsRod[j].x,
								                 thisradius*RingCoordsRod[j].y,
												 thisz));
				else
					mesh.setVert(thisvert,Tlocal*Point3(thisradius*RingCoordsRod[j].x,
								                        thisradius*RingCoordsRod[j].y,
												        thisz));
				if (mapmenow)
					mesh.tVerts[thisvert] = Point3(thisxtex,RingCoordsRod[j].z,0.0f);
				thisvert++;
			}
			if (mapmenow)
				mesh.tVerts[Nverts+NRingsStud+i] = Point3(thisxtex, 0.999f, 0.0f);
		}

// vertices for BOOT
		float multiwavelen, distancealong, effectiveangle;
		if (IsBooted)
		{	for (i=0 ; i < NRingsBoot ; i++) // Stop vertices
			{	thisxtex = (float)i/(float)(NRingsBoot-1);
				if (i == 0)
				{	thisradius = roddia;
					thisz = Lf - bootstoplen - bootstoploc;
				}
				else if (i == 1)
				{	thisradius = bootstopdia;
					thisz = Lf - bootstoplen - bootstoploc;
				}
				else if (i == 2)
				{	thisradius = bootstopdia;
					if (bootfilletsegs == 0)
						thisz = Lf - bootstoploc;
					else
						thisz = Lf - bootstoploc - bootstopfillet;
				}
				else if (i < 3+bootfilletsegs)
				{	thisangle = (float)(i - 2)*PIover2/(float)(bootfilletsegs);
					thisradius = bootstopfillet*(float)cos(thisangle) + bootstopdia - bootstopfillet;
					thisz = Lf - bootstoploc - bootstopfillet + bootstopfillet*(float)sin(thisangle);
				}
				else if (i == 3+bootfilletsegs)
				{	thisradius = roddia;
					thisz = Lf - bootstoploc;
				}
// boot vertices proper here
				else if (i > 3+bootfilletsegs)
				{	multiwavelen = Lf - lstud - lcyl - bootstoploc - bootstoplen;
					distancealong = float(i-bootfilletsegs-4)/NdivsBoot;
					effectiveangle = (float)bootfolds*PI*distancealong;
					thisz = lstud + lcyl + distancealong*multiwavelen;
					thisradius = bootdia1 + (float)fabs(diffboot*(float)sin(effectiveangle));
				}

				for (j=0 ; j < NvertsPerRingBoot ; j++)
				{	if (createfree)
						mesh.setVert(thisvert,Point3(thisradius*RingCoordsBoot[j].x,
									                 thisradius*RingCoordsBoot[j].y,
													 thisz));
					else
						mesh.setVert(thisvert,Tlocal*Point3(thisradius*RingCoordsBoot[j].x,
									                        thisradius*RingCoordsBoot[j].y,
													        thisz));
					if (mapmenow)
						mesh.tVerts[thisvert] = Point3(thisxtex,RingCoordsBoot[j].z,0.0f);
					thisvert++;
				}
				if (mapmenow)
					mesh.tVerts[Nverts+NRingsStud+NRingsRod+i] = Point3(thisxtex, 0.999f, 0.0f);
			}

		}

// set up variables to create faces
		int thisface = 0, v1, v2, v3, v4, v5, v6;
		int rowsoffaces, lastverttest;
		thisvert = 0;

// Faces for Cylinder
		rowsoffaces = NRingsStud - 1;
		lastverttest = NvertsPerRingStud - 1;
		for (i=0 ; i < rowsoffaces ; i++)
			for (j=0 ; j < NvertsPerRingStud ; j++)
			{	v1 = thisvert;
				v2 = ( j < lastverttest ? v1+1 : v1 - lastverttest );
				v3 = v1 + NvertsPerRingStud;
				v4 = v2 + NvertsPerRingStud;
				v5 = (j < lastverttest? v2 : Nverts+i);
				v6 = (j < lastverttest? v4 : Nverts+i+1);
				mesh.faces[thisface].setVerts(v4,v3,v1);
				if (i == 0)
					mesh.faces[thisface].setSmGroup(smoothcyl?1:0);
				else if (i == 1)
					mesh.faces[thisface].setSmGroup(smoothcyl?2:0);
				else if (i == 2)
					mesh.faces[thisface].setSmGroup(smoothcyl?1:0);
				else if (i < 3+cylfillet1segs)
					mesh.faces[thisface].setSmGroup(smoothcyl?2:0);
				else if (i == 3+cylfillet1segs)
					mesh.faces[thisface].setSmGroup(smoothcyl?4:0);
				else if (i < 4+cylfillet1segs+cylfillet2segs)
					mesh.faces[thisface].setSmGroup(smoothcyl?2:0);
				else if (i == 4+cylfillet1segs+cylfillet2segs)
					mesh.faces[thisface].setSmGroup(smoothcyl?1:0);
				else if (i == 5+cylfillet1segs+cylfillet2segs)
					mesh.faces[thisface].setSmGroup(smoothcyl?2:0);
				else if (i == 6+cylfillet1segs+cylfillet2segs)
					mesh.faces[thisface].setSmGroup(smoothcyl?1:0);
				mesh.faces[thisface].setEdgeVisFlags(1,1,0);
				if (i < 2 )
					mesh.faces[thisface].setMatID(0);
				else if (i == 2)
				{	if (rstud > rcyl)
						mesh.faces[thisface].setMatID(0);
					else
						mesh.faces[thisface].setMatID(1);
				}
				else
					mesh.faces[thisface].setMatID(1);
				if (mapmenow)
					mesh.tvFace[thisface].setTVerts(v6,v3,v1);
				thisface++;
				mesh.faces[thisface].setVerts(v1,v2,v4);
				if (i == 0)
					mesh.faces[thisface].setSmGroup(smoothcyl?1:0);
				else if (i == 1)
					mesh.faces[thisface].setSmGroup(smoothcyl?2:0);
				else if (i == 2)
					mesh.faces[thisface].setSmGroup(smoothcyl?1:0);
				else if (i < 3+cylfillet1segs)
					mesh.faces[thisface].setSmGroup(smoothcyl?2:0);
				else if (i == 3+cylfillet1segs)
					mesh.faces[thisface].setSmGroup(smoothcyl?4:0);
				else if (i < 4+cylfillet1segs+cylfillet2segs)
					mesh.faces[thisface].setSmGroup(smoothcyl?2:0);
				else if (i == 4+cylfillet1segs+cylfillet2segs)
					mesh.faces[thisface].setSmGroup(smoothcyl?1:0);
				else if (i == 5+cylfillet1segs+cylfillet2segs)
					mesh.faces[thisface].setSmGroup(smoothcyl?2:0);
				else if (i == 6+cylfillet1segs+cylfillet2segs)
					mesh.faces[thisface].setSmGroup(smoothcyl?1:0);
				mesh.faces[thisface].setEdgeVisFlags(1,1,0);
				if (i < 2 )
					mesh.faces[thisface].setMatID(0);
				else if (i == 2)
				{	if (rstud > rcyl)
						mesh.faces[thisface].setMatID(0);
					else
						mesh.faces[thisface].setMatID(1);
				}
				else
					mesh.faces[thisface].setMatID(1);
				if (mapmenow)
					mesh.tvFace[thisface].setTVerts(v1,v5,v6);
				thisface++;
				thisvert++;
			}

// Faces for Rod 
		rowsoffaces = NRingsRod - 1;
		lastverttest = NvertsPerRingRod - 1;
		thisvert += NvertsPerRingStud;
		for (i=0 ; i < rowsoffaces ; i++)
			for (j=0 ; j < NvertsPerRingRod ; j++)
			{	v1 = thisvert;
				v2 = ( j < lastverttest ? v1+1 : v1 - lastverttest );
				v3 = v1 + NvertsPerRingRod;
				v4 = v2 + NvertsPerRingRod;
				v5 = (j < lastverttest? v2 : Nverts+NRingsStud+i);
				v6 = (j < lastverttest? v4 : Nverts+NRingsStud+i+1);

				mesh.faces[thisface].setVerts(v4,v3,v1);
				if (i == 0)
					mesh.faces[thisface].setSmGroup(smoothrod?1:0);
				else if (i == 1)
					mesh.faces[thisface].setSmGroup(smoothrod?2:0);
				else if (i == 2)
					mesh.faces[thisface].setSmGroup(smoothrod?1:0);
				mesh.faces[thisface].setEdgeVisFlags(1,1,0);
				mesh.faces[thisface].setMatID(2);
				if (mapmenow)
					mesh.tvFace[thisface].setTVerts(v6,v3,v1);
				thisface++;

				mesh.faces[thisface].setVerts(v1,v2,v4);
				if (i == 0)
					mesh.faces[thisface].setSmGroup(smoothrod?1:0);
				else if (i == 1)
					mesh.faces[thisface].setSmGroup(smoothrod?2:0);
				else if (i == 2)
					mesh.faces[thisface].setSmGroup(smoothrod?1:0);
				mesh.faces[thisface].setEdgeVisFlags(1,1,0);
				mesh.faces[thisface].setMatID(2);
				if (mapmenow)
					mesh.tvFace[thisface].setTVerts(v1,v5,v6);
				thisface++;
				thisvert++;
			}

		if (IsBooted)
		{	rowsoffaces = 3 + ((bootfilletsegs>0) ? bootfilletsegs : 0);
//		{	rowsoffaces = 3 + (((bootfilletsegs>0)&&(bootstopfillet>0.0f)) ? bootfilletsegs : 0);
			lastverttest = NvertsPerRingBoot - 1;
			thisvert += NvertsPerRingRod;
			for (i=0 ; i < rowsoffaces ; i++)
				for (j=0 ; j < NvertsPerRingBoot ; j++)
				{	v1 = thisvert;
					v2 = ( j < lastverttest ? v1+1 : v1 - lastverttest );
					v3 = v1 + NvertsPerRingBoot;
					v4 = v2 + NvertsPerRingBoot;
					v5 = (j < lastverttest? v2 : Nverts+NRingsStud+NRingsRod+i);
					v6 = (j < lastverttest? v4 : Nverts+NRingsStud+NRingsRod+i+1);
	
					mesh.faces[thisface].setVerts(v4,v3,v1);
					if (i == 0) //boot end flat
						mesh.faces[thisface].setSmGroup(smoothboot?1:0);
					else if (i == 1) //side cylindrical surface
						mesh.faces[thisface].setSmGroup(smoothboot?2:0);
					else if ((i < 2 + bootfilletsegs)&&(bootfilletsegs == 1)) //hard fillet
						mesh.faces[thisface].setSmGroup(smoothboot?4:0);
					else if ((i < 2 + bootfilletsegs)&&(bootfilletsegs > 1)) //smooth fillet
						mesh.faces[thisface].setSmGroup(smoothboot?2:0);
					else if ((i == 2 + bootfilletsegs)&&(bootfilletsegs == 1)) //end with hard fillet
						mesh.faces[thisface].setSmGroup(smoothboot?1:0);
					else if ((i == 2 + bootfilletsegs)&&(bootfilletsegs > 1)) //end with smooth fillet
						mesh.faces[thisface].setSmGroup(smoothboot?2:0);
					else if ((i == 2 + bootfilletsegs)&&(bootfilletsegs == 0)) //end
						mesh.faces[thisface].setSmGroup(smoothboot?1:0);
					mesh.faces[thisface].setEdgeVisFlags(1,1,0);
					mesh.faces[thisface].setMatID(3);
					if (mapmenow)
						mesh.tvFace[thisface].setTVerts(v6,v3,v1);
					thisface++;
	
					mesh.faces[thisface].setVerts(v1,v2,v4);
					if (i == 0) //boot end flat
						mesh.faces[thisface].setSmGroup(smoothboot?1:0);
					else if (i == 1) //side cylindrical surface
						mesh.faces[thisface].setSmGroup(smoothboot?2:0);
					else if ((i < 2 + bootfilletsegs)&&(bootfilletsegs == 1)) //hard fillet
						mesh.faces[thisface].setSmGroup(smoothboot?4:0);
					else if ((i < 2 + bootfilletsegs)&&(bootfilletsegs > 1)) //smooth fillet
						mesh.faces[thisface].setSmGroup(smoothboot?2:0);
					else if ((i == 2 + bootfilletsegs)&&(bootfilletsegs == 1)) //end with hard fillet
						mesh.faces[thisface].setSmGroup(smoothboot?1:0);
					else if ((i == 2 + bootfilletsegs)&&(bootfilletsegs > 1)) //end with smooth fillet
						mesh.faces[thisface].setSmGroup(smoothboot?2:0);
					else if ((i == 2 + bootfilletsegs)&&(bootfilletsegs == 0)) //end
						mesh.faces[thisface].setSmGroup(smoothboot?1:0);
					mesh.faces[thisface].setEdgeVisFlags(1,1,0);
					mesh.faces[thisface].setMatID(3);
					if (mapmenow)
						mesh.tvFace[thisface].setTVerts(v1,v5,v6);
					thisface++;
					thisvert++;
				}
			rowsoffaces = bootfolds*bootres;
			lastverttest = NvertsPerRingBoot - 1;
			thisvert += NvertsPerRingBoot;
			for (i=0 ; i < rowsoffaces ; i++)
				for (j=0 ; j < NvertsPerRingBoot ; j++)
				{	v1 = thisvert;
					v2 = ( j < lastverttest ? v1+1 : v1 - lastverttest );
					v3 = v1 + NvertsPerRingBoot;
					v4 = v2 + NvertsPerRingBoot;
					v5 = (j < lastverttest? v2 : Nverts+NRingsStud+NRingsRod+NRingsStop+i);
					v6 = (j < lastverttest? v4 : Nverts+NRingsStud+NRingsRod+NRingsStop+i+1);
	
					mesh.faces[thisface].setVerts(v4,v3,v1);
					mesh.faces[thisface].setSmGroup(smoothboot?1:0);
					mesh.faces[thisface].setEdgeVisFlags(1,1,0);
					mesh.faces[thisface].setMatID(4);
					if (mapmenow)
						mesh.tvFace[thisface].setTVerts(v6,v3,v1);
					thisface++;
	
					mesh.faces[thisface].setVerts(v1,v2,v4);
					mesh.faces[thisface].setSmGroup(smoothboot?1:0);
					mesh.faces[thisface].setEdgeVisFlags(1,1,0);
					mesh.faces[thisface].setMatID(4);
					if (mapmenow)
						mesh.tvFace[thisface].setTVerts(v1,v5,v6);
					thisface++;
					thisvert++;
				}
		}

		if (RingCoordsStud) 
			delete[] RingCoordsStud;
		if (RingCoordsRod) 
			delete[] RingCoordsRod;
		if (RingCoordsBoot) 
			delete[] RingCoordsBoot;
	}
	mesh.InvalidateTopologyCache();
	srand( (unsigned)time( NULL ) );
}

class DamperEmitterCreateCallback : public CreateMouseCallBack {
	public:
		DamperObject *ob;
		Point3 p[2];
		IPoint2 sp0, sp1;
		BOOL square;
		void Cleanup();
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
	};

int DamperEmitterCreateCallback::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat)
{	float r;
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
	if (msg==MOUSE_POINT||msg==MOUSE_MOVE)
	{ switch(point)
		{ 	case 0:
				ob->suspendSnap = TRUE;				
				GetCOREInterface()->SetHideByCategoryFlags(
						GetCOREInterface()->GetHideByCategoryFlags() & ~(HIDE_OBJECTS));
				sp0 = m;				
				p[0] = vpt->SnapPoint(m,m,NULL,snapdim);
				mat.SetTrans(p[0]); // Set Node's transform				
				ob->pblock->SetValue(PB_MOUNTDIA,0,0.01f);
				ob->pblock->SetValue(PB_CYLDIA,0,0.01f);
				ob->pblock->SetValue(PB_NOREFLENGTH,0,0.01f);
				ob->pblock->SetValue(PB_CYLLEN,0,0.01f);
				ob->pblock->SetValue(PB_INSIDEDIA,0,0.0f);
				ob->pblock->SetValue(PB_PISTONDIA,0,0.01f);
				ob->pblock->SetValue(PB_PISTONLEN,0,0.01f);
				break;
			case 1: 
				{	mat.IdentityMatrix();
				//mat.PreRotateZ(HALFPI);
				sp1 = m;							   
					p[1] = vpt->SnapPoint(m,m,NULL,snapdim);
					// diameter
					Point3 center = p[0];
					r = Length(p[1]-p[0]);
					mat.SetTrans(center);  // Modify Node's transform
				
				ob->pblock->SetValue(PB_MOUNTDIA,0,r);
				ob->pblock->SetValue(PB_CYLDIA,0,2.0f*r);
				ob->pblock->SetValue(PB_PISTONDIA,0,0.5f*r);
				ob->pblock->SetValue(PB_BOOTDIA1,0,0.7f*r);
				ob->pblock->SetValue(PB_BOOTDIA2,0,1.8f*r);
				ob->pblock->SetValue(PB_BOOTSTOPDIA,0,0.8f*r);
				ob->pmapParam->Invalidate();

				if (flags&MOUSE_CTRL)
				{	float ang = (float)atan2(p[1].y-p[0].y,p[1].x-p[0].x);
					mat.PreRotateZ(ob->ip->SnapAngle(ang));
				}

				if (msg==MOUSE_POINT) 
				{if (Length(m-sp0)<3 ||	Length(p[1]-p[0])<0.1f)
					{return CREATE_ABORT;	}
				}
				break;
				}
			case 2:
				{
#ifdef _OSNAP
				float h = vpt->SnapLength(vpt->GetCPDisp(p[1],Point3(0,0,1),sp1,m,TRUE));
#else
				float h = vpt->SnapLength(vpt->GetCPDisp(p[1],Point3(0,0,1),sp1,m));
#endif
				if (h<0.0f) 
					h = -h;
				ob->pblock->SetValue(PB_NOREFLENGTH,0,h);
				ob->pblock->SetValue(PB_MOUNTLEN,0,0.1f*h);
				ob->pblock->SetValue(PB_CYLLEN,0,0.6f*h);
				ob->pblock->SetValue(PB_PISTONLEN,0,0.5f*h);
				ob->pblock->SetValue(PB_BOOTSTOPLEN,0,0.05f*h);
				ob->pblock->SetValue(PB_BOOTSETBACK,0,0.1f*h);
				ob->pmapParam->Invalidate();
				if (msg==MOUSE_POINT)
				{ ob->suspendSnap = FALSE;
				  if (Length(m-sp0)<3 ) return CREATE_ABORT;	
				  else { TurnButton(ob->hParams,IDC_SPRING_PICKOBJECT1,TRUE);
						TurnButton(ob->hParams,IDC_SPRING_PICKOBJECT2,TRUE);
						return CREATE_STOP;
						}
				}
				}
				break;
			}
	}
	else
	if (msg == MOUSE_ABORT) 
	{ return CREATE_ABORT;}

	return TRUE;
}

static DamperEmitterCreateCallback emitterCallback;

CreateMouseCallBack* DamperObject::GetCreateMouseCallBack() 
	{
	emitterCallback.ob = this;
	return &emitterCallback;
	}

void DamperObject::InvalidateUI()
	{
	if (pmapParam) pmapParam->Invalidate();
	}

ParamDimension *DamperObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_NOREFLENGTH:
		case PB_MOUNTDIA:
		case PB_MOUNTLEN:
		case PB_CYLDIA:
		case PB_CYLLEN:
		case PB_CYLFILLET1:
		case PB_CYLFILLET2:
		case PB_INSIDEDIA:
		case PB_PISTONDIA:
		case PB_PISTONLEN:
		case PB_BOOTDIA1:
		case PB_BOOTDIA2:
		case PB_BOOTSTOPDIA:
		case PB_BOOTSTOPLEN:
		case PB_BOOTSETBACK:
		case PB_BOOTSTOPFILLET:
					return stdWorldDim;
		default:	return defaultDim;
		}
	}

TSTR DamperObject::GetParameterName(int pbIndex)
	{
	switch (pbIndex) {
		case PB_ENDSMETHOD:			return GetString(IDS_AP_ENDSMETHOD);break;
		case PB_NOREFLENGTH:		return GetString(IDS_AP_NOREFDL);break;
		case PB_RENDERABLE:			return GetString(IDS_AP_RENDER);break;

		case PB_MOUNTDIA:			return GetString(IDS_AP_MOUNTDIA);break;
		case PB_MOUNTLEN:			return GetString(IDS_AP_MOUNTLEN);break;
		case PB_CYLDIA:				return GetString(IDS_AP_CYLDIA);break;
		case PB_CYLLEN:				return GetString(IDS_AP_CYLLEN);break;
		case PB_CYLSIDES:			return GetString(IDS_AP_CYLSIDES);break;
		case PB_CYLFILLET1:			return GetString(IDS_AP_CYLFILLET1);break;
		case PB_CYLFILLET1SEGS:		return GetString(IDS_AP_CYLFILLET1SEGS);break;
		case PB_CYLFILLET2:			return GetString(IDS_AP_CYLFILLET2);break;
		case PB_CYLFILLET2SEGS:		return GetString(IDS_AP_CYLFILLET2SEGS);break;
		case PB_INSIDEDIA:			return GetString(IDS_AP_INSIDEDIA);break;
		case PB_SMOOTHCYL:			return GetString(IDS_AP_SMOOTHCYL);break;

		case PB_PISTONDIA:			return GetString(IDS_AP_PISTONDIA);break;
		case PB_PISTONLEN:			return GetString(IDS_AP_PISTONLEN);break;
		case PB_PISTONSIDES:		return GetString(IDS_AP_PISTONSIDES);break;
		case PB_SMOOTHPISTON:		return GetString(IDS_AP_SMOOTHPISTON);break;

		case PB_ENABLEBOOT:			return GetString(IDS_AP_ENABLEBOOT);break;
		case PB_BOOTDIA1:			return GetString(IDS_AP_BOOTDIA1);break;
		case PB_BOOTDIA2:			return GetString(IDS_AP_BOOTDIA2);break;
		case PB_BOOTSIDES:			return GetString(IDS_AP_BOOTSIDES);break;
		case PB_BOOTFOLDS:			return GetString(IDS_AP_BOOTFOLDS);break;
		case PB_BOOTRESOLUTION:		return GetString(IDS_AP_BOOTRESOLUTION);break;
		case PB_BOOTSTOPDIA:		return GetString(IDS_AP_BOOTSTOPDIA);break;
		case PB_BOOTSTOPLEN:		return GetString(IDS_AP_BOOTSTOPLEN);break;
		case PB_BOOTSETBACK:		return GetString(IDS_AP_BOOTSETBACK);break;
		case PB_BOOTSTOPFILLET:		return GetString(IDS_AP_BOOTSTOPFILLET);break;
		case PB_BOOTSTOPFILLETSEGS:	return GetString(IDS_AP_BOOTSTOPFILLETSEGS);break;
		case PB_SMOOTHBOOT:			return GetString(IDS_AP_SMOOTHBOOT);break;

		case PB_DYNTYPE:			return GetString(IDS_AP_DYNTYPE);break;
		case PB_DYNDRAG:			return GetString(IDS_AP_DYNDRAG);break;
		case PB_DYNDRAGUNITS:		return GetString(IDS_AP_DYNDRAGUNITS);break;
		case PB_SPRINGDIR:			return GetString(IDS_AP_DAMPERDIR);break;
		case PB_DYNFORCE:			return GetString(IDS_AP_DYNFORCE);break;
		case PB_DYNFORCEUNITS:		return GetString(IDS_AP_DYNFORCEUNITS);break;
		case PB_MAPMEMAPME:			return GetString(IDS_AP_MAPME);break;

		default: 					return TSTR(_T(""));
		}
	}

//--- Damper particle -----------------------------------------------

RefTargetHandle DamperObject::Clone(RemapDir& remap) 
	{
	DamperObject* newob = new DamperObject();	
	newob->ReplaceReference(0,pblock->Clone(remap));	
	if (custnode) newob->ReplaceReference(CUSTNODE,custnode);
	if (custnode2) newob->ReplaceReference(CUSTNODE2,custnode2);
	newob->custname=custname;
	newob->custname2=custname2;
	newob->ivalid.SetEmpty();	
	int cnt=GetCOREInterface()->GetSelNodeCount();
	if (cnt>0) newob->thisnode=GetCOREInterface()->GetSelNode(0);
	else newob->thisnode=NULL;
	BaseClone(this, newob, remap);
	return newob;
	}

RefTargetHandle DamperObject::GetReference(int i)
{	switch(i) {
		case PBLK: return(RefTargetHandle)pblock;
		case CUSTNODE: return (RefTargetHandle)custnode;
		case CUSTNODE2: return (RefTargetHandle)custnode2;
		default: return NULL;
		}
	}

void DamperObject::SetReference(int i, RefTargetHandle rtarg) { 
	switch(i) {
		case PBLK: pblock=(IParamBlock*)rtarg; return;
		case CUSTNODE: custnode = (INode *)rtarg; return;
		case CUSTNODE2: custnode2 = (INode *)rtarg; return;
		}
	}

RefResult DamperObject::NotifyRefChanged( 
		Interval changeInt,
		RefTargetHandle hTarget, 
		PartID& partID, 
		RefMessage message )
	{				
	switch (message) {		
		case REFMSG_TARGET_DELETED:	
			{ if (hTarget==custnode)
			  { if (theHold.Holding() && !TestAFlag(A_HELD)) 
					theHold.Put(new CreateCPartRestoreNode(this));
			    DeleteReference(CUSTNODE);
				custnode=NULL;cancelled=FALSE;
				custname=TSTR(_T(" "));				
				}
			  if (hTarget==custnode2) 
			  { if (theHold.Holding() && !TestAFlag(A_HELD)) 
					theHold.Put(new CreateCPart2RestoreNode(this));
			    DeleteReference(CUSTNODE2);
				custnode2=NULL;cancelled=FALSE;
			    custname2=TSTR(_T(" "));
			  }
			}
			break;
		case REFMSG_NODE_NAMECHANGE:
			{ if (hTarget==custnode) 
			  { custname = TSTR(custnode->GetName());
			    ShowName();
				cancelled=FALSE;
				}
			  if (hTarget==custnode2) 
			  { custname2 = TSTR(custnode2->GetName());
			    ShowName2();
				cancelled=FALSE;
				}
			  break;
			}
		case REFMSG_CHANGE:
			{ MeshInvalid();
			  if (editOb==this) InvalidateUI();
			  cancelled=FALSE;
			}
			break;
		default: SimpleObject::NotifyRefChanged(changeInt,hTarget,partID,message);
		}
	return REF_SUCCEED;
	}

/*class DamperPostLoadCallback : public PostLoadCallback {
	public:
		ParamBlockPLCB *cb;
		DamperPostLoadCallback(ParamBlockPLCB *c) {cb=c;}
		void proc(ILoad *iload) {
			DWORD oldVer = ((DamperObject*)(cb->targ))->pblock->GetVersion();
			ReferenceTarget *targ = cb->targ;
			cb->proc(iload);
			if (oldVer<1) {	
				((DamperObject*)targ)->pblock->SetValue(PB_SYMMETRY,0,0);
				}
			delete this;
			}
	};*/

#define COM_CUSTNAME_CHUNK	0x0100
#define COM_CUSTNAME2_CHUNK	0x0101
#define COM_CNODE_CHUNK		0x0102

IOResult DamperObject::Save(ISave *isave)
{ 	ULONG nb;
	int refid;

	isave->BeginChunk(COM_CUSTNAME_CHUNK);		
	isave->WriteWString(custname);
	isave->EndChunk();

	isave->BeginChunk(COM_CUSTNAME2_CHUNK);		
	isave->WriteWString(custname2);
	isave->EndChunk();

	refid=isave->GetRefID(thisnode);
	isave->BeginChunk(COM_CNODE_CHUNK);		
	isave->Write(&refid,sizeof(int),&nb);
	isave->EndChunk();

	return IO_OK;
	}

IOResult DamperObject::Load(ILoad *iload)
	{
	ULONG nb;
	IOResult res = IO_OK;
	int cnmtl=0;
	int refid;
	
//	iload->RegisterPostLoadCallback(
			//new DamperPostLoadCallback(
				//new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,0)));
	// Default names
	custname = TSTR(_T(" "));
	custname2 = TSTR(_T(" "));
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case COM_CUSTNAME_CHUNK: {
				TCHAR *buf;
				res=iload->ReadWStringChunk(&buf);
				custname = TSTR(buf);
				break;
				}
			case COM_CUSTNAME2_CHUNK: {
				TCHAR *buf;
				res=iload->ReadWStringChunk(&buf);
				custname2= TSTR(buf);
				break;
				}
			case COM_CNODE_CHUNK: 
			{	res=iload->Read(&refid,sizeof(int),&nb);
			    iload->RecordBackpatch(refid,(void**)&thisnode);
				break; }
			}
		
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}

	return IO_OK;
	}

const TCHAR *DamperClassDesc::ClassName ()	{return GetString(IDS_AP_DAMPER);}
const TCHAR *DamperClassDesc::Category ()	{return GetString(IDS_EP_DYNPRIMS);}
TCHAR *DamperObject::GetObjectName() {return GetString(IDS_AP_DAMPER_OBJECT);}

#endif // NO_OBJECT_DAMPER
