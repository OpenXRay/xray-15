#include "solids.h"

#include "interpik.h"
#include "texutil.h"
#include "macrorec.h"

#ifndef NO_OBJECT_SPRING	// russom - 11/19/01

#define MAX_PATH_LENGTH 257
#define MAX_STRING_LENGTH  256
#define TESTLIMIT 100

#define A_RENDER			A_PLUGIN1

const float PIover2=1.570796327f;
const float QuarterPI=0.785398163f;
static int controlsInit = FALSE;
const Point3 ZAxis=Point3(0.0f,0.0f,1.0f);
#define fourninthPI (4.0f/9.0f)*PI
#define oneeightPI	(1.0f/18.0f)*PI

static Class_ID SPRING_CLASS_ID(0x7ba3009a, 0x76b61f8e);

#define PBLK		0
#define CUSTNODE 	1
#define CUSTNODE2 	2

const float EPSILON=0.0001f;
const Point3 Ones=Point3(1.0f,1.0f,1.0f);

class SpringPickOperand;
class SpringObject;

class SpringObject : public DynamHelperObject {
	public:
		static IParamMap *pmapParam;
		SpringObject();
		~SpringObject();
		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message);
	
		static SpringObject *editOb;
		INode *custnode2,*custnode,*thisnode;
		TSTR custname,custname2;
		static IObjParam *ip;
		Matrix3 S;
		static HWND hParams;
		ICustButton *iPick,*iPick2;

		static BOOL creating;
		static SpringPickOperand pickCB;
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
		Class_ID ClassID() {return SPRING_CLASS_ID;} 
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

class SpringClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new SpringObject;}
	const TCHAR *	ClassName(); 
	SClass_ID		SuperClassID() {return GEOMOBJECT_CLASS_ID;}
	Class_ID		ClassID() {return SPRING_CLASS_ID;}
	const TCHAR* 	Category(); 
	void			ResetClassParams(BOOL fileReset);
	int 			BeginCreate(Interface *i);
	int 			EndCreate(Interface *i);
	};

static SpringClassDesc SpringDesc;
ClassDesc* GetSpringDesc() {return &SpringDesc;}

class SpringPickOperand : 
		public PickModeCallback,
		public PickNodeCallback {
	public:		
		SpringObject *po;
		int dodist,repi;

		SpringPickOperand() {po=NULL;dodist=-1;}

		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);

		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);

		BOOL RightClick(IObjParam *ip, ViewExp *vpt) { return TRUE; }
		BOOL Filter(INode *node);
		
		PickNodeCallback *GetFilter() {return this;}
	};

IParamMap *SpringObject::pmapParam;
IObjParam *SpringObject::ip    = NULL;
BOOL SpringObject::creating    = FALSE;
HWND SpringObject::hParams		=NULL;
SpringPickOperand SpringObject::pickCB;
SpringObject *SpringObject::editOb	=NULL;

//--- Parameter map/block descriptors -------------------------------

#define SIZEFACTOR (float(TIME_TICKSPERSEC)/120.0f)

#define PB_ENDSMETHOD		0
#define PB_NOREFLENGTH		1
#define PB_OD				2
#define PB_COILS			3
#define PB_COILDIR			4
#define PB_MANUALSEGSET		5
#define PB_SEGSPERTURN		6
#define PB_SEGMENTS			7
#define PB_SMOOTH			8
#define PB_RENDERABLE		9

#define PB_WIRETYPE			10
#define PB_RNDDIA			11
#define PB_RNDSIDES			12

#define PB_RECTWIDTH		13
#define PB_RECTDEPTH		14
#define PB_RECTFILLET		15
#define PB_RECTFILLETSIDES	16
#define PB_RECTROTANGLE		17

#define PB_DSECWIDTH		18
#define PB_DSECDEPTH		19
#define PB_DSECFILLET		20
#define PB_DSECFILLETSIDES	21
#define PB_DSECRNDSIDES		22
#define PB_DSECROTANGLE		23

#define PB_FREELENGTH		24
#define PB_KVALUE			25
#define PB_KVALUNITS		26
#define PB_SPRINGDIR		27

#define PB_MAPMEMAPME		28
#define PB_NONLINEAR		29

static int endmethodIDs[] = {IDC_SPRING_REFS,IDC_SPRING_DIMS};

static int coildirIDs[] = {IDC_SPRING_CCWDIR,IDC_SPRING_CWDIR};

static int segmethodIDs[] = {IDC_SPRING_AUTOSEGS,IDC_SPRING_MANUALSEGS};
const float WIREMIN=0.0f;
const float FILLETMIN=0.0f;
#define ROUND	0
#define RECT	1
#define DSECT	2

static int wiresectionIDs[] = {IDC_SPRING_ROUNDWIRE,IDC_SPRING_RECTWIRE,IDC_SPRING_DSECTIONWIRE};

static int kvalunitIDs[] = {IDC_SPRING_UNITS_LBIN,IDC_SPRING_UNITS_NM};

static int springdirIDs[] = {IDC_SPRING_COMP,IDC_SPRING_EXT,IDC_SPRING_BOTH};

static int smoothtypeIDs[] = {IDC_SPRING_SMALL,IDC_SPRING_SMNONE,IDC_SPRING_SMSIDES,IDC_SPRING_SMSEGS};

//
// Parameters

static ParamUIDesc descSpringParam1[] = {

	// End Method
	ParamUIDesc(PB_ENDSMETHOD,TYPE_RADIO,endmethodIDs,2),

	// No reference version Length
	ParamUIDesc(
		PB_NOREFLENGTH,
		EDITTYPE_UNIVERSE,
		IDC_SPRING_LENGTH,IDC_SPRING_LENGTHSPIN,
		0.0f,99999999.0f,
		0.1f),	
		
	// Outside Diameter
	ParamUIDesc(
		PB_OD,
		EDITTYPE_UNIVERSE,
		IDC_SPRING_OD,IDC_SPRING_ODSPIN,
		0.1f,99999999.0f,
		0.1f),	
		
	// Coils
	ParamUIDesc(
		PB_COILS,
		EDITTYPE_FLOAT,
		IDC_SPRING_COILS,IDC_SPRING_COILSSPIN,
		0.5f,99999999.0f,
		0.25f),	
		
	// Coil Direction
	ParamUIDesc(PB_COILDIR,TYPE_RADIO,coildirIDs,2),

	// Segmentation method
	ParamUIDesc(PB_MANUALSEGSET,TYPE_RADIO,segmethodIDs,2),

	// Segments per turn
	ParamUIDesc(
		PB_SEGSPERTURN,
		EDITTYPE_INT,
		IDC_SPRING_COILSEGSPERTURN,IDC_SPRING_COILSEGSPERTURNSPIN,
		3.0f,100.0f,
		1.0f),	

	// Segments along coils
	ParamUIDesc(
		PB_SEGMENTS,
		EDITTYPE_INT,
		IDC_SPRING_COILSEGS,IDC_SPRING_COILSEGSSPIN,
		3.0f,100000.0f,
		1.0f),	
		
	// Smooth Spring
	ParamUIDesc(PB_SMOOTH,TYPE_RADIO,smoothtypeIDs,4),			

	// IsRenderable?
	ParamUIDesc(PB_RENDERABLE,TYPE_SINGLECHEKBOX,IDC_SPRING_RENDERTHIS),			


	// Wire Type
	ParamUIDesc(PB_WIRETYPE,TYPE_RADIO,wiresectionIDs,3),

	// Wire Diameter
	ParamUIDesc(
		PB_RNDDIA,
		EDITTYPE_UNIVERSE,
		IDC_SPRING_RNDWIREDIA,IDC_SPRING_RNDWIREDIASPIN,
		WIREMIN,99999999.0f,
		0.1f),	

	// Sides around wire
	ParamUIDesc(
		PB_RNDSIDES,
		EDITTYPE_INT,
		IDC_SPRING_RNDWIRESIDES,IDC_SPRING_RNDWIRESIDESSPIN,
		3.0f,100.0f,
		1.0f),	
		
	// Wire Width
	ParamUIDesc(
		PB_RECTWIDTH,
		EDITTYPE_UNIVERSE,
		IDC_SPRING_RECTWIREWID,IDC_SPRING_RECTWIREWIDSPIN,
		WIREMIN,99999999.0f,
		0.1f),	

	// Wire Depth
	ParamUIDesc(
		PB_RECTDEPTH,
		EDITTYPE_UNIVERSE,
		IDC_SPRING_RECWIREDEPTH,IDC_SPRING_RECWIREDEPTHSPIN,
		WIREMIN,99999999.0f,
		0.1f),	

	// Fillet Size
	ParamUIDesc(
		PB_RECTFILLET,
		EDITTYPE_UNIVERSE,
		IDC_SPRING_RECWIREFILLET,IDC_SPRING_RECWIREFILLETSPIN,
		0.0f,99999999.0f,
		0.1f),	

	// Fillet Sides
	ParamUIDesc(
		PB_RECTFILLETSIDES,
		EDITTYPE_INT,
		IDC_SPRING_RECWIREFILLETSIDES,IDC_SPRING_RECWIREFILLETSIDESSPIN,
		0.0f,100.0f,
		1.0f),	

	// Rect Wire Rotation Angle
	ParamUIDesc(
		PB_RECTROTANGLE,
		EDITTYPE_FLOAT,
		IDC_SPRING_RECTROT,IDC_SPRING_RECTROTSPIN,
		-99999999.0f,99999999.0f,
		1.0f,
		stdAngleDim),	

		
	// D Wire Width
	ParamUIDesc(
		PB_DSECWIDTH,
		EDITTYPE_UNIVERSE,
		IDC_SPRING_DWIREWID,IDC_SPRING_DWIREWIDSPIN,
		WIREMIN,99999999.0f,
		0.1f),	

	// D Wire Depth
	ParamUIDesc(
		PB_DSECDEPTH,
		EDITTYPE_UNIVERSE,
		IDC_SPRING_DWIREDEPTH,IDC_SPRING_DWIREDEPTHSPIN,
		0.0f,99999999.0f,
		0.1f),	

	// D Fillet Size
	ParamUIDesc(PB_DSECFILLET,
		EDITTYPE_UNIVERSE,
		IDC_SPRING_DWIREFILLET,IDC_SPRING_DWIREFILLETSPIN,
		0.0f,99999999.0f,
		0.1f),	

	// D Fillet Sides
	ParamUIDesc(
		PB_DSECFILLETSIDES,
		EDITTYPE_INT,
		IDC_SPRING_DWIREFILLETSIDES,IDC_SPRING_DWIREFILLETSIDESSPIN,
		0.0f,100.0f,
		1.0f),	
		
	// D Round Sides
	ParamUIDesc(
		PB_DSECRNDSIDES,
		EDITTYPE_INT,
		IDC_SPRING_DWIRERNDSIDES,IDC_SPRING_DWIRERNDSIDESSPIN,
		2.0f,100.0f,
		1.0f),	

	// D Wire Rotation Angle
	ParamUIDesc(
		PB_DSECROTANGLE,
		EDITTYPE_FLOAT,
		IDC_SPRING_DROT,IDC_SPRING_DROTSPIN,
		-99999999.0f,99999999.0f,
		1.0f,
		stdAngleDim),	


	// Dyn Free Len
	ParamUIDesc(
		PB_FREELENGTH,
		EDITTYPE_UNIVERSE,
		IDC_SPRING_L,IDC_SPRING_LSPIN,
		0.0f,99999999.0f,
		0.1f),	

	// Dyn K Value
	ParamUIDesc(
		PB_KVALUE,
		EDITTYPE_UNIVERSE,
		IDC_SPRING_K,IDC_SPRING_KSPIN,
		0.0f,99999999.0f,
		0.1f),	

	// Dyn Spring Constant Units
	ParamUIDesc(PB_KVALUNITS,TYPE_RADIO,kvalunitIDs,2),

	// Dyn Spring Dir
	ParamUIDesc(PB_SPRINGDIR,TYPE_RADIO,springdirIDs,3),

	// Generate text coords
	ParamUIDesc(PB_MAPMEMAPME,TYPE_SINGLECHEKBOX,IDC_SPRING_MAPME),			
			
	// Nonlinear Response
	ParamUIDesc(PB_NONLINEAR,TYPE_SINGLECHEKBOX,IDC_SPRING_NL),			
			
};

#define SPRINGPARAMSDESC_LENGTH 30

ParamBlockDescID descSpringVer0[] = {
	{ TYPE_INT, NULL, FALSE, 0 }, // End Method
	{ TYPE_FLOAT, NULL, TRUE, 1 }, // No Ref Length
	{ TYPE_FLOAT, NULL, TRUE, 2 }, // OD
	{ TYPE_FLOAT, NULL, TRUE, 3 }, // Coil Count
	{ TYPE_INT, NULL, FALSE, 4 }, // Coil Dir
	{ TYPE_INT, NULL, FALSE, 5 }, // Segmentation Method
	{ TYPE_INT, NULL, TRUE, 6 }, // Coil Segs per turn Count
	{ TYPE_INT, NULL, TRUE, 7 }, // Coil Seg Count
	{ TYPE_INT, NULL, FALSE, 8 }, // Smooth Control
	{ TYPE_INT, NULL, FALSE, 9 }, // Renderable

	{ TYPE_INT, NULL, FALSE, 10 }, // Wire Section Type
	{ TYPE_FLOAT, NULL, TRUE, 11 }, // Round Wire Dia
	{ TYPE_INT, NULL, TRUE, 12 },  // Wire Sides
	{ TYPE_FLOAT, NULL, TRUE, 13 }, // Rect Wire Width
	{ TYPE_FLOAT, NULL, TRUE, 14 }, // Rect Wire Depth
	{ TYPE_FLOAT, NULL, TRUE, 15 }, // Rect Wire Fillet Size
	{ TYPE_INT, NULL, TRUE, 16 }, // Rect Wire Fillet Sides
	{ TYPE_FLOAT, NULL, TRUE, 17 }, // Rect Wire Rot Angle
	{ TYPE_FLOAT, NULL, TRUE, 18 }, // D Wire Width
	{ TYPE_FLOAT, NULL, TRUE, 19 }, // D Wire Depth
	{ TYPE_FLOAT, NULL, TRUE, 20 }, // D Wire Fillet Size
	{ TYPE_INT, NULL, TRUE, 21 }, // D Wire Fillet Sides
	{ TYPE_INT, NULL, TRUE, 22 }, // D Wire Round Sides
	{ TYPE_FLOAT, NULL, TRUE, 23 }, // D Wire Rotation Angle

	{ TYPE_FLOAT, NULL, TRUE, 24 }, // Free Length
	{ TYPE_FLOAT, NULL, TRUE, 25 },  // K Value
	{ TYPE_INT, NULL, FALSE, 26 }, // K Value Units
	{ TYPE_INT, NULL, FALSE, 27 },  // Spring Direction

	{ TYPE_INT, NULL, FALSE, 28 },  // gen tex coords
	{ TYPE_INT, NULL, FALSE, 29 },  // nonlinear response
};

#define PBLOCK_SPRING_LENGTH	30

// Array of old versions
// static ParamVersionDesc versions[] = {ParamVersionDesc(descSpringVer0,24,0)};

#define NUM_OLDVERSIONS	0
#define CURRENT_VERSION	0

// Current version
static ParamVersionDesc curVersion(descSpringVer0,PBLOCK_SPRING_LENGTH,CURRENT_VERSION);


class CreateMSpringProc : public MouseCallBack,ReferenceMaker {
	private:
		IObjParam *ip;
		void Init(IObjParam *i) {ip=i;}
		CreateMouseCallBack *createCB;	
		INode *CloudNode;
		SpringObject *SSBlizObject;
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
		
		CreateMSpringProc()
			{
			ignoreSelectionChange = FALSE;
			}
//		int createmethod(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
		int proc( HWND hwnd, int msg, int point, int flag, IPoint2 m );
	};


class CreateCPartRestoreNode : public RestoreObj {
	public:   		
		SpringObject *obj;
		TSTR name,namer;
		INode *save,*saver;
		CreateCPartRestoreNode(SpringObject *o) {
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
		SpringObject *obj;
		TSTR name,namer;
		INode *save,*saver;
		CreateCPart2RestoreNode(SpringObject *o) {
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

#define CID_CREATESPRINGMODE	CID_USER +75

class CreateMSpringMode : public CommandMode {		
	public:		
		CreateMSpringProc proc;
		IObjParam *ip;
		SpringObject *obj;
		void Begin( IObjCreate *ioc, ClassDesc *desc ) { proc.Begin( ioc, desc ); }
		void End() { proc.End(); }
		void JumpStart(IObjParam *i,SpringObject*o);

		int Class() {return CREATE_COMMAND;}
		int ID() { return CID_CREATESPRINGMODE; }
		MouseCallBack *MouseProc(int *numPoints) {*numPoints = 10000; return &proc;}
		ChangeForegroundCallback *ChangeFGProc() {return CHANGE_FG_SELECTED;}
		BOOL ChangeFG( CommandMode *oldMode ) { return (oldMode->ChangeFGProc() != CHANGE_FG_SELECTED); }
		void EnterMode() 
		{ GetCOREInterface()->PushPrompt(GetString(IDS_AP_CREATEMODE));
		  SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR)));
		}
		void ExitMode() {GetCOREInterface()->PopPrompt();SetCursor(LoadCursor(NULL, IDC_ARROW));}
	};
static CreateMSpringMode theCreateMSpringMode;

void SpringClassDesc::ResetClassParams(BOOL fileReset)
	{
	}

RefResult CreateMSpringProc::NotifyRefChanged(
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
						{  theCreateMSpringMode.JumpStart(SSBlizObject->ip,SSBlizObject);
							createInterface->SetCommandMode(&theCreateMSpringMode);
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

void CreateMSpringProc::Begin( IObjCreate *ioc, ClassDesc *desc )
	{
	createInterface = ioc;
	cDesc           = desc;
	attachedToNode  = FALSE;
	createCB        = NULL;
	CloudNode         = NULL;
	SSBlizObject       = NULL;
	CreateNewObject();
	}

void CreateMSpringProc::CreateNewObject()
{
	SuspendSetKeyMode();
	SSBlizObject = (SpringObject*)cDesc->Create();
	lastPutCount  = theHold.GetGlobalPutCount();
	
    macroRec->BeginCreate(cDesc);  // JBW 3/30/99

	// Start the edit params process
	if ( SSBlizObject ) {
		SSBlizObject->BeginEditParams( (IObjParam*)createInterface, BEGIN_EDIT_CREATE, NULL );
#ifdef _OSNAP
		SSBlizObject->SetAFlag(A_OBJ_LONG_CREATE);
#endif
	}
	ResumeSetKeyMode();
}

void CreateMSpringProc::End()
{ 
	if ( SSBlizObject ) 
	{ 
 #ifdef _OSNAP
		SSBlizObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
		SSBlizObject->EndEditParams( (IObjParam*)createInterface, 
	                    	          END_EDIT_REMOVEUI, NULL);
		if ( !attachedToNode ) 
		{	// RB 4-9-96: Normally the hold isn't holding when this 
			// happens, but it can be in certain situations (like a Spring view paste)
			// Things get confused if it ends up with undo...
			theHold.Suspend(); 
			delete SSBlizObject;
			SSBlizObject = NULL;
			theHold.Resume();
			if (theHold.GetGlobalPutCount()!=lastPutCount) 
				GetSystemSetting(SYSSET_CLEAR_UNDO);
			macroRec->Cancel();  // JBW 3/30/99
		}  else if ( CloudNode ) 
		{	theHold.Suspend();
			DeleteReference(0);  // sets cloudNode = NULL
			theHold.Resume();}
	}
}

void CreateMSpringMode::JumpStart(IObjParam *i,SpringObject *o)
	{
	ip  = i;
	obj = o;
	//MakeRefByID(FOREVER,0,svNode);
	obj->BeginEditParams(i,BEGIN_EDIT_CREATE,NULL);
	}

int SpringClassDesc::BeginCreate(Interface *i)
{	
	SuspendSetKeyMode();
	IObjCreate *iob = i->GetIObjCreate();
	theCreateMSpringMode.Begin(iob,this);
	iob->PushCommandMode(&theCreateMSpringMode);
	return TRUE;
}

int SpringClassDesc::EndCreate(Interface *i)
{
	ResumeSetKeyMode();
	theCreateMSpringMode.End();
	i->RemoveMode(&theCreateMSpringMode);
	macroRec->EmitScript();  // JBW 3/30/99
	return TRUE;
}

int CreateMSpringProc::proc(HWND hwnd,int msg,int point,int flag,
				IPoint2 m )
{	int res=TRUE;	
	ViewExp *vpx = createInterface->GetViewport(hwnd); 
	assert( vpx );

	switch ( msg ) {
		case MOUSE_POINT:
			switch ( point ) {
				case 0:
					assert( SSBlizObject );					
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
						SSBlizObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
				   		SSBlizObject->EndEditParams( (IObjParam*)createInterface, 0, NULL);
						macroRec->EmitScript();  // JBW 3/30/99

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
					SuspendSetKeyMode();
					CloudNode = createInterface->CreateObjectNode( SSBlizObject);
					ResumeSetKeyMode();
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
						SuspendSetKeyMode();
						createInterface->SetNodeTMRelConstPlane(CloudNode,mat);
						ResumeSetKeyMode();

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
				SuspendSetKeyMode();
				createInterface->SetNodeTMRelConstPlane(CloudNode,mat);
				ResumeSetKeyMode();
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
	  createInterface->SetNodeTMRelConstPlane(CloudNode, mat);
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
//		macroRec->Cancel();  // JBW 3/30/99
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
	if ((res == CREATE_STOP)||(res==CREATE_ABORT))
		vpx->ReleaseImplicitGrid();
	createInterface->ReleaseViewport(vpx); 
	return res;
}
BOOL SpringObject::OKtoDisplay(TimeValue t) 
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

BOOL SpringPickOperand::Filter(INode *node)
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

BOOL SpringPickOperand::HitTest(
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

void SpringObject::ShowName()
{ TSTR name=(custnode ? custname : TSTR(GetString(IDS_AP_NONE)));
  SetWindowText(GetDlgItem(hParams, IDC_SPRING_OBJ1NAME), name);
}
void SpringObject::ShowName2()
{ TSTR name=(custnode2 ? custname2 : TSTR(GetString(IDS_AP_NONE)));
  SetWindowText(GetDlgItem(hParams, IDC_SPRING_OBJ2NAME), name);
}
void SpringObject::ShowNames()
{ ShowName();ShowName2();
}

BOOL SpringPickOperand::Pick(IObjParam *ip,ViewExp *vpt)
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
		theCreateMSpringMode.JumpStart(ip,po);
		ip->SetCommandMode(&theCreateMSpringMode);
		ip->RedrawViews(ip->GetTime());
		return FALSE;
	} else {return TRUE;}
}

void SpringPickOperand::EnterMode(IObjParam *ip)
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

void SpringPickOperand::ExitMode(IObjParam *ip)
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
	dodist=-1;
    GetCOREInterface()->PopPrompt();
}

//-- ParticleDlgProc ------------------------------------------------

class SpringParamDlg : public ParamMapUserDlgProc {
	public:
		SpringObject *po;

		SpringParamDlg(SpringObject *p) {po=p; m_bUIValid = false;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void Update(TimeValue t);
		void BootParams(BOOL ison);
		void AutoSegs(BOOL ison);
		void WireType(int type,TimeValue t);
		void BoundParams(BOOL ison);
		void DeleteThis() {delete this;}
		void FixSpringDiameter(int type,TimeValue t);
		void CheckFillet(int type,TimeValue t);
		BOOL m_bUIValid;
		void InvalidateUI (HWND hWnd) 
		{ 
			if (hWnd) InvalidateRect (hWnd, NULL, false); 
			m_bUIValid = true; 
		}
	};
void SpringParamDlg::CheckFillet(int type,TimeValue t)
{ float depth,fillet;
  if (type==RECT)
  { float width;
    po->pblock->GetValue(PB_RECTWIDTH,t,width,FOREVER);
    po->pblock->GetValue(PB_RECTDEPTH,t,depth,FOREVER);
	if (width<depth) depth=width;
  }
  else
  { po->pblock->GetValue(PB_DSECDEPTH,t,depth,FOREVER);
  }
  int spinnum=(type==RECT?PB_RECTFILLET:PB_DSECFILLET);
  po->pblock->GetValue(spinnum,t,fillet,FOREVER);
  depth*=0.5f;
  BOOL change;
  if (change=(fillet>depth))
  {	 po->pblock->SetValue(spinnum,t,depth);
  }
  if (po->hParams)
  FixFSpinnerLimits(po->hParams,(type==RECT?IDC_SPRING_RECWIREFILLETSPIN:IDC_SPRING_DWIREFILLETSPIN),FILLETMIN,depth,change);
}
void SpringParamDlg::FixSpringDiameter(int type,TimeValue t)
{ float mdiam,diam;
  po->pblock->GetValue(PB_OD,t,mdiam,FOREVER);
  int spinnum=(type==ROUND?PB_RNDDIA:(type==RECT?PB_RECTWIDTH:PB_DSECWIDTH));
  po->pblock->GetValue(spinnum,t,diam,FOREVER);
  BOOL change;
  if (change=(diam>=mdiam))
  { 
	  SuspendSetKeyMode();
	  po->pblock->SetValue(spinnum,t,mdiam);
	  ResumeSetKeyMode();
  }
  FixFSpinnerLimits(po->hParams,(type==ROUND?IDC_SPRING_RNDWIREDIASPIN:(type==RECT?IDC_SPRING_RECTWIREWIDSPIN:IDC_SPRING_DWIREWIDSPIN)),WIREMIN,mdiam,change);
  if (type!=ROUND) CheckFillet(type,t);
}
void SpringParamDlg::BoundParams(BOOL ison)
{ 
	BOOL ok=ison;

	EnableWindow(GetDlgItem(po->hParams,IDC_SPRING_UNITS_LBIN),ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_SPRING_UNITS_NM),ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_SPRING_COMP),ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_SPRING_EXT),ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_SPRING_BOTH),ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_SPRING_NL),ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_SPRING_RELHEIGHT_LABEL),ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_SPRING_CONSTK_LABEL   ),ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_SPRING_CONSTIN_LABEL  ),ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_SPRING_WORKSIN_LABEL  ),ison);  

	EnableWindow(GetDlgItem(po->hParams,IDC_SPRING_FREEHEIGHT_LABEL),!ison);

	if (ison)
	{ 
		SpinnerOff(po->hParams,IDC_SPRING_LENGTHSPIN);
		SpinnerOn(po->hParams,IDC_SPRING_LSPIN);
		SpinnerOn(po->hParams,IDC_SPRING_KSPIN);
		ok=(po->thisnode!=NULL);
	}
	else
	{ 
		SpinnerOn(po->hParams,IDC_SPRING_LENGTHSPIN);
		SpinnerOff(po->hParams,IDC_SPRING_LSPIN);
		SpinnerOff(po->hParams,IDC_SPRING_KSPIN);
	}

	if ((po->iPick->IsEnabled()!=ok)||(po->iPick2->IsEnabled()!=ok))
	{ 
		if ((po->pickCB.dodist<0)||(!ok))
		{ 
			EnableWindow(GetDlgItem(po->hParams,IDC_SPRING_OBJ2NAME),ok);
			EnableWindow(GetDlgItem(po->hParams,IDC_SPRING_OBJ1NAME),ok);
			EnableWindow(GetDlgItem(po->hParams,IDC_SPRING_OBJ2NAMECAPTION),ok);
			EnableWindow(GetDlgItem(po->hParams,IDC_SPRING_OBJ1NAMECAPTION),ok);
			TurnButton(po->hParams,IDC_SPRING_PICKOBJECT2,ok);
			TurnButton(po->hParams,IDC_SPRING_PICKOBJECT1,ok);
		}
	}
	// labels have changed, so mark UI as dirty
    //
    m_bUIValid = FALSE;
}

void SpringParamDlg::AutoSegs(BOOL ison)
{ 

	EnableWindow(GetDlgItem(po->hParams,IDC_SPRING_SEGSTURN_LABEL   ),!ison);
	EnableWindow(GetDlgItem(po->hParams,IDC_SPRING_MANSEGMENTS_LABEL),ison);
	if (ison)
	{ 
		SpinnerOff(po->hParams,IDC_SPRING_COILSEGSPERTURNSPIN);
		SpinnerOn(po->hParams,IDC_SPRING_COILSEGSSPIN);
	}
	else 
	{ 
		SpinnerOn(po->hParams,IDC_SPRING_COILSEGSPERTURNSPIN);
		SpinnerOff(po->hParams,IDC_SPRING_COILSEGSSPIN);
	}
	// labels have changed, so mark UI as dirty
    //
    m_bUIValid = FALSE;
}

void RoundOff(HWND hwnd)
{ 
	EnableWindow(GetDlgItem(hwnd,IDC_SPRING_RNDDIA_LABEL),FALSE);
	EnableWindow(GetDlgItem(hwnd,IDC_SPRING_RNDSIDES_LABEL),FALSE);
	SpinnerOff(hwnd,IDC_SPRING_RNDWIREDIASPIN);
	SpinnerOff(hwnd,IDC_SPRING_RNDWIRESIDESSPIN);
}
void RoundOn(HWND hwnd)
{ 
	EnableWindow(GetDlgItem(hwnd,IDC_SPRING_RNDDIA_LABEL),TRUE);
	EnableWindow(GetDlgItem(hwnd,IDC_SPRING_RNDSIDES_LABEL),TRUE);
	SpinnerOn(hwnd,IDC_SPRING_RNDWIREDIASPIN);
	SpinnerOn(hwnd,IDC_SPRING_RNDWIRESIDESSPIN);
}
void RectOff(HWND hwnd)
{ 
	EnableWindow(GetDlgItem(hwnd, IDC_SPRING_RECTWIDTH_LABEL),     FALSE);
	EnableWindow(GetDlgItem(hwnd, IDC_SPRING_RECTDEPTH_LABEL),     FALSE);
	EnableWindow(GetDlgItem(hwnd, IDC_SPRING_RECTFILLET_LABEL),    FALSE);
	EnableWindow(GetDlgItem(hwnd, IDC_SPRING_RECTFILLETSEGS_LABEL),FALSE);
	EnableWindow(GetDlgItem(hwnd, IDC_SPRING_RECTROTATION_LABEL),  FALSE);
	
	SpinnerOff(hwnd,IDC_SPRING_RECTWIREWIDSPIN);
	SpinnerOff(hwnd,IDC_SPRING_RECWIREDEPTHSPIN);
	SpinnerOff(hwnd,IDC_SPRING_RECWIREFILLETSPIN);
	SpinnerOff(hwnd,IDC_SPRING_RECWIREFILLETSIDESSPIN);
	SpinnerOff(hwnd,IDC_SPRING_RECTROTSPIN);
}
void RectOn(HWND hwnd)
{ 
	EnableWindow(GetDlgItem(hwnd, IDC_SPRING_RECTWIDTH_LABEL),     TRUE);
	EnableWindow(GetDlgItem(hwnd, IDC_SPRING_RECTDEPTH_LABEL),     TRUE);
	EnableWindow(GetDlgItem(hwnd, IDC_SPRING_RECTFILLET_LABEL),    TRUE);
	EnableWindow(GetDlgItem(hwnd, IDC_SPRING_RECTFILLETSEGS_LABEL),TRUE);
	EnableWindow(GetDlgItem(hwnd, IDC_SPRING_RECTROTATION_LABEL),  TRUE);
	
	SpinnerOn(hwnd,IDC_SPRING_RECTWIREWIDSPIN);
	SpinnerOn(hwnd,IDC_SPRING_RECWIREDEPTHSPIN);
	SpinnerOn(hwnd,IDC_SPRING_RECWIREFILLETSPIN);
	SpinnerOn(hwnd,IDC_SPRING_RECWIREFILLETSIDESSPIN);
	SpinnerOn(hwnd,IDC_SPRING_RECTROTSPIN);
}
void DSectOff(HWND hwnd)
{ 
	EnableWindow(GetDlgItem(hwnd, IDC_SPRING_DSECTWIDTH_LABEL),     FALSE);
	EnableWindow(GetDlgItem(hwnd, IDC_SPRING_DSECTDEPTH_LABEL),     FALSE);
	EnableWindow(GetDlgItem(hwnd, IDC_SPRING_DSECTRNDSIDES_LABEL),  FALSE);
	EnableWindow(GetDlgItem(hwnd, IDC_SPRING_DSECTFILLET_LABEL),    FALSE);
	EnableWindow(GetDlgItem(hwnd, IDC_SPRING_DSECTFILLETSEGS_LABEL),FALSE);
	EnableWindow(GetDlgItem(hwnd, IDC_SPRING_DSECTROTATION_LABEL),  FALSE);
	
	SpinnerOff(hwnd,IDC_SPRING_DWIREWIDSPIN);
	SpinnerOff(hwnd,IDC_SPRING_DWIREDEPTHSPIN);
	SpinnerOff(hwnd,IDC_SPRING_DWIRERNDSIDESSPIN);
	SpinnerOff(hwnd,IDC_SPRING_DWIREFILLETSPIN);
	SpinnerOff(hwnd,IDC_SPRING_DWIREFILLETSIDESSPIN);
	SpinnerOff(hwnd,IDC_SPRING_DROTSPIN);
   
}
void DSectOn(HWND hwnd)
{ 
	EnableWindow(GetDlgItem(hwnd, IDC_SPRING_DSECTWIDTH_LABEL),     TRUE);
	EnableWindow(GetDlgItem(hwnd, IDC_SPRING_DSECTDEPTH_LABEL),     TRUE);
	EnableWindow(GetDlgItem(hwnd, IDC_SPRING_DSECTRNDSIDES_LABEL),  TRUE);
	EnableWindow(GetDlgItem(hwnd, IDC_SPRING_DSECTFILLET_LABEL),    TRUE);
	EnableWindow(GetDlgItem(hwnd, IDC_SPRING_DSECTFILLETSEGS_LABEL),TRUE);
	EnableWindow(GetDlgItem(hwnd, IDC_SPRING_DSECTROTATION_LABEL),  TRUE);
	
	SpinnerOn(hwnd,IDC_SPRING_DWIREWIDSPIN);
	SpinnerOn(hwnd,IDC_SPRING_DWIREDEPTHSPIN);
	SpinnerOn(hwnd,IDC_SPRING_DWIRERNDSIDESSPIN);
	SpinnerOn(hwnd,IDC_SPRING_DWIREFILLETSPIN);
	SpinnerOn(hwnd,IDC_SPRING_DWIREFILLETSIDESSPIN);
	SpinnerOn(hwnd,IDC_SPRING_DROTSPIN);
}
void SpringParamDlg::WireType(int type,TimeValue t)
{ if (type==ROUND)
  { RoundOn(po->hParams);
	RectOff(po->hParams);
	DSectOff(po->hParams);
  }
  else if (type==RECT)
  { RoundOff(po->hParams);
	RectOn(po->hParams);
	DSectOff(po->hParams);
  }
  else
  { RoundOff(po->hParams);
	RectOff(po->hParams);
	DSectOn(po->hParams);
  }
  FixSpringDiameter(type,t);
  
  // labels have changed, so mark UI as dirty
  //
  m_bUIValid = FALSE;
}

void SpringParamDlg::Update(TimeValue t)
{ if (!po->editOb) return;
  po->ShowNames();
  int emethod;po->pblock->GetValue(PB_ENDSMETHOD,0,emethod,FOREVER);
  BoundParams(!emethod);
  int manualsegs;po->pblock->GetValue(PB_MANUALSEGSET,0,manualsegs,FOREVER);
  AutoSegs(manualsegs);
  int wire;po->pblock->GetValue(PB_WIRETYPE,0,wire,FOREVER);
  WireType(wire,t);
 /*float length; po->pblock->GetValue(PB_NOREFLENGTH,0,length,FOREVER);
  if (length<0.01f) 
  if ((emethod)||(!po->thisnode))
  { po->iPick->Disable();
    po->iPick2->Disable();
  }
  else if (!po->iPick->IsEnabled())
  { po->iPick->Enable(TRUE);
    po->iPick2->Enable(TRUE);
  }*/
}

BOOL SpringParamDlg::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{	switch (msg) {
		case WM_INITDIALOG: {
			// initialize the spring param labels by marking the UI as dirty
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
				 case IDC_SPRING_ODSPIN: 
				  { int wtype;
					po->pblock->GetValue(PB_WIRETYPE,t,wtype,FOREVER);
					FixSpringDiameter(wtype,t);
				  }
					break;
			  case IDC_SPRING_RECTWIREWIDSPIN: 
			  case IDC_SPRING_RECWIREDEPTHSPIN: 
				  { CheckFillet(RECT,t);
				  }
				  break;				  
			  case IDC_SPRING_DWIREWIDSPIN: 
			  case IDC_SPRING_DWIREDEPTHSPIN: 
				  { CheckFillet(DSECT,t);
				  }
				  break;				  			  
				}
			  }
			}
			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
			  case IDC_SPRING_RECTWIREWIDSPIN: 
			  case IDC_SPRING_RECWIREDEPTHSPIN: 
			  case IDC_SPRING_RECTWIREWID: 
			  case IDC_SPRING_RECWIREDEPTH: 		  
				  { CheckFillet(RECT,t);
				  }
				  break;				  
			  case IDC_SPRING_DWIREWIDSPIN: 
			  case IDC_SPRING_DWIREDEPTHSPIN: 
			  case IDC_SPRING_DWIREWID: 
			  case IDC_SPRING_DWIREDEPTH: 
				  { CheckFillet(DSECT,t);
				  }
				  break;				  			  
			  case IDC_SPRING_PICKOBJECT1: 
				   {if (po->ip->GetCommandMode()->ID() == CID_STDPICK) 
					{ if (po->creating) 
						{  theCreateMSpringMode.JumpStart(po->ip,po);
							po->ip->SetCommandMode(&theCreateMSpringMode);
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
						{  theCreateMSpringMode.JumpStart(po->ip,po);
							po->ip->SetCommandMode(&theCreateMSpringMode);
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
					  { theCreateMSpringMode.JumpStart(po->ip,po);
						 po->ip->SetCommandMode(&theCreateMSpringMode);
					  } else {po->ip->SetStdCommandMode(CID_OBJMOVE);}
					}
				  }
				  break;
			  case IDC_SPRING_OD:
				  { int wtype;
					po->pblock->GetValue(PB_WIRETYPE,t,wtype,FOREVER);
					FixSpringDiameter(wtype,t);
				  }
				  break;
			  case IDC_SPRING_AUTOSEGS: 
				  { AutoSegs(FALSE);
				  }
				  break;
			  case IDC_SPRING_MANUALSEGS: 
				  { AutoSegs(TRUE);
				  }
				  break;				  
			  case IDC_SPRING_ROUNDWIRE: 
				  { WireType(ROUND,t);
				  }
				  break;				  
			  case IDC_SPRING_RECTWIRE: 
				  { WireType(RECT,t);
				  }
				  break;				  
			  case IDC_SPRING_DSECTIONWIRE: 
				  { WireType(DSECT,t);
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

//--- SpringObject Methods--------------------------------------------

SpringObject::SpringObject()
{	int FToTick=(int)((float)TIME_TICKSPERSEC/(float)GetFrameRate());
	int length = PBLOCK_SPRING_LENGTH;
	MakeRefByID(FOREVER, 0, CreateParameterBlock(descSpringVer0, length, CURRENT_VERSION));
	assert(pblock);

	pblock->SetValue(PB_ENDSMETHOD,0,1);
	pblock->SetValue(PB_NOREFLENGTH,0,1.0f);
	pblock->SetValue(PB_OD,0,1.0f);	
	pblock->SetValue(PB_COILS,0,1.0f);	
	pblock->SetValue(PB_COILDIR,0,0);	
	pblock->SetValue(PB_MANUALSEGSET,0,0);	
	pblock->SetValue(PB_SEGSPERTURN,0,16);
	pblock->SetValue(PB_SEGMENTS,0,16);
	pblock->SetValue(PB_SMOOTH,0,0);
	pblock->SetValue(PB_RENDERABLE,0,1);

	pblock->SetValue(PB_WIRETYPE,0,0);
	pblock->SetValue(PB_RNDDIA,0,0.2f);
	pblock->SetValue(PB_RNDSIDES,0,6);
	pblock->SetValue(PB_RECTWIDTH,0,0.2f);
	pblock->SetValue(PB_RECTDEPTH,0,0.2f);
	pblock->SetValue(PB_RECTFILLET,0,0.0f);
	pblock->SetValue(PB_RECTFILLETSIDES,0,0);
	pblock->SetValue(PB_RECTROTANGLE,0,0.0f);
	pblock->SetValue(PB_DSECWIDTH,0,0.2f);
	pblock->SetValue(PB_DSECDEPTH,0,0.2f);
	pblock->SetValue(PB_DSECFILLET,0,0.0f);
	pblock->SetValue(PB_DSECFILLETSIDES,0,0);
	pblock->SetValue(PB_DSECRNDSIDES,0,4);
	pblock->SetValue(PB_DSECROTANGLE,0,0.0f);

	pblock->SetValue(PB_FREELENGTH,0,1.0f);
	pblock->SetValue(PB_KVALUE,0,1.0f);
	pblock->SetValue(PB_KVALUNITS,0,0);
	pblock->SetValue(PB_SPRINGDIR,0,0);
	pblock->SetValue(PB_MAPMEMAPME,0,TRUE);
	pblock->SetValue(PB_NONLINEAR,0,0);

	S.IdentityMatrix();
	iPick=NULL;
	iPick2=NULL;
	thisnode=NULL;
	custnode=NULL;
	custnode2=NULL;
	custname=TSTR(_T(" "));
	custname2=TSTR(_T(" "));
}

SpringObject::~SpringObject()
{	DeleteAllRefsFromMe();
}

void SpringObject::BeginEditParams(
		IObjParam *ip, ULONG flags,Animatable *prev)
{	SimpleObject::BeginEditParams(ip,flags,prev);
	this->ip = ip;
	editOb   = this;
	if (flags&BEGIN_EDIT_CREATE) {
		creating = TRUE;
	} else { creating = FALSE; }

	if (pmapParam) 
	{	pmapParam->SetParamBlock(pblock);
	}
	else 
	{	pmapParam = CreateCPParamMap(
		descSpringParam1,SPRINGPARAMSDESC_LENGTH,
		pblock,
		ip,
		hInstance,
		MAKEINTRESOURCE(IDD_SPRING),
		GetString(IDS_AP_PARAMS),
		0);		
	}
	hParams=pmapParam->GetHWnd();
	if (pmapParam) pmapParam->SetUserDlgProc(new SpringParamDlg(this));
}

void SpringObject::EndEditParams(
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

void SpringObject::MapKeys(TimeMap *map,DWORD flags)
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

#define BLVERTS 11
#define BLFACES 9

void SpringObject::GetWorldBoundBox(
		TimeValue t, INode *inode, ViewExp* vpt, Box3& box)
	{	thisnode=inode;    
		SimpleObject::GetWorldBoundBox(t,inode,vpt,box);
	}  

void SpringObject::MakeSaveVertex(Point3 *SaveVertex,int NvertsPerRing, int nfillets, int nsides, int wtype, TimeValue t)
{	if (wtype == ROUND)
	{	float ang = TWOPI/(float)NvertsPerRing,diar;
		pblock->GetValue(PB_RNDDIA,t,diar,ivalid);
		diar *= 0.5f;
		for (int i=0; i<NvertsPerRing; i++)
		{	float u = (float)(i+1)*ang;
			SaveVertex[i] = Point3(diar*(float)cos(u),diar*(float)sin(u),0.0f);
		}
	}
	else if (wtype == RECT)
	{	int savevertcnt = 0;
		int qtrverts = 1 + nfillets;
		int hlfverts = 2*qtrverts;
		int thrverts = qtrverts + hlfverts;
		float Wr, Dr, Zfr;
		pblock->GetValue(PB_RECTWIDTH,t,Wr,ivalid);
		Wr *= 0.5f;
		pblock->GetValue(PB_RECTDEPTH,t,Dr,ivalid);
		Dr *= 0.5f;
		pblock->GetValue(PB_RECTFILLET,t,Zfr,ivalid);
		if (Zfr<0.0f) Zfr = 0.0f;
		if (nfillets>0)
		{	float WmZ = Wr-Zfr,
				  DmZ = Dr-Zfr;
			float ZmW = -WmZ,
				  ZmD = -DmZ;
			SaveVertex[0                ] = Point3(Wr , DmZ, 0.0f);
			SaveVertex[nfillets         ] = Point3(WmZ, Dr , 0.0f);
			SaveVertex[qtrverts         ] = Point3(ZmW, Dr , 0.0f);
			SaveVertex[qtrverts+nfillets] = Point3(-Wr, DmZ, 0.0f);
			SaveVertex[hlfverts         ] = Point3(-Wr, ZmD, 0.0f);
			SaveVertex[hlfverts+nfillets] = Point3(ZmW, -Dr, 0.0f);
			SaveVertex[thrverts         ] = Point3(WmZ, -Dr, 0.0f);
			SaveVertex[thrverts+nfillets] = Point3(Wr , ZmD, 0.0f);

			if (nfillets > 1)
			{	float ang = PIover2/(float)nfillets;
				savevertcnt = 1;
				for (int i=0; i<nfillets-1; i++)
				{	float u = (float)(i+1)*ang;
				    float cu = Zfr*(float)cos(u),
						  su = Zfr*(float)sin(u);
					SaveVertex[savevertcnt         ] = Point3(WmZ+cu, DmZ+su, 0.0f);
					SaveVertex[savevertcnt+qtrverts] = Point3(ZmW-su, DmZ+cu, 0.0f);
					SaveVertex[savevertcnt+hlfverts] = Point3(ZmW-cu, ZmD-su, 0.0f);
					SaveVertex[savevertcnt+thrverts] = Point3(WmZ+su, ZmD-cu, 0.0f);
					savevertcnt++;
				}
			}
		}
		else
		{	SaveVertex[savevertcnt]=Point3(Wr,Dr,0.0f);
			savevertcnt++;
			SaveVertex[savevertcnt]=Point3(-Wr,Dr,0.0f);
			savevertcnt++;
			SaveVertex[savevertcnt]=Point3(-Wr,-Dr,0.0f);
			savevertcnt++;
			SaveVertex[savevertcnt]=Point3(Wr,-Dr,0.0f);
			savevertcnt++;
		}
	}
	else
	{	int savevertcnt = 0;
		float Wr, Dr, Zfr;
		float sang = PI/(float)nsides;

		pblock->GetValue(PB_DSECWIDTH,t,Wr,ivalid);
		Wr*=0.5f;
		pblock->GetValue(PB_DSECDEPTH,t,Dr,ivalid);
		Dr*=0.5f;
		pblock->GetValue(PB_DSECFILLET,t,Zfr,ivalid);
		if (Zfr<0.0f) Zfr = 0.0f;

		float LeftCenter = Dr-Wr;

		if (nfillets > 0)
		{	float DmZ = Dr-Zfr,
				  ZmD = -DmZ,
				  WmZ = Wr-Zfr;
			int oneqtrverts = 1+nfillets;
			int threeqtrverts = oneqtrverts+1+nsides;

			SaveVertex[0                     ] = Point3(Wr        , DmZ,0.0f);
			SaveVertex[nfillets              ] = Point3(WmZ       , Dr ,0.0f);
			SaveVertex[oneqtrverts           ] = Point3(LeftCenter, Dr ,0.0f);
			SaveVertex[oneqtrverts+nsides    ] = Point3(LeftCenter,-Dr ,0.0f);
			SaveVertex[threeqtrverts         ] = Point3(WmZ       ,-Dr ,0.0f);
			SaveVertex[threeqtrverts+nfillets] = Point3(Wr        , ZmD,0.0f);

			if (nfillets > 1)
			{	float ang = PIover2/(float)nfillets;
				savevertcnt = 1;
				for (int i=0; i < nfillets-1; i++)
				{	float u = (float)(i+1)*ang;
				    float cu = Zfr*(float)cos(u),
						  su = Zfr*(float)sin(u);
					SaveVertex[savevertcnt              ] = Point3(WmZ+cu, DmZ+su, 0.0f);
					SaveVertex[savevertcnt+threeqtrverts] = Point3(WmZ+su, ZmD-cu, 0.0f);
					savevertcnt++;
				}
			}
			savevertcnt = 1+oneqtrverts;
			for (int i=0; i < nsides-1; i++)
			{	float u = (float)(i+1)*sang;
			    float cu = Dr*(float)cos(u),
					  su = Dr*(float)sin(u);
				SaveVertex[savevertcnt] = Point3(LeftCenter-su,cu,0.0f);
				savevertcnt++;
			}
		}
		else
		{	SaveVertex[savevertcnt]=Point3(Wr,        Dr,0.0f);
			savevertcnt++;
			SaveVertex[savevertcnt]=Point3(LeftCenter,Dr,0.0f);
			savevertcnt++;
			for (int i=0; i<nsides-1; i++)
			{	float u = (float)(i+1)*sang;
			    float cu = Dr*(float)cos(u),
					  su = Dr*(float)sin(u);
				SaveVertex[savevertcnt] = Point3(LeftCenter-su, cu, 0.0f);
				savevertcnt++;
			}
			SaveVertex[savevertcnt] = Point3(LeftCenter,-Dr,0.0f);
			savevertcnt++;
			SaveVertex[savevertcnt] = Point3(Wr,        -Dr,0.0f);
			savevertcnt++;
		}
	}
	float rotangle, cosu, sinu, tempx, tempy;
	if (wtype == RECT)
	{	pblock->GetValue(PB_RECTROTANGLE,t,rotangle,ivalid);
		if (rotangle != 0.0f)
		{	cosu = (float)cos(rotangle);
			sinu = (float)sin(rotangle);
			for (int m=0; m<NvertsPerRing ; m++)
			{	tempx = SaveVertex[m].x*cosu - SaveVertex[m].y*sinu;
				tempy = SaveVertex[m].x*sinu + SaveVertex[m].y*cosu;
				SaveVertex[m].x = tempx;
				SaveVertex[m].y = tempy;
			}
		}
	}
	else if (wtype == DSECT)
	{	pblock->GetValue(PB_DSECROTANGLE,t,rotangle,ivalid);
		if (rotangle != 0.0f)
		{	cosu = (float)cos(rotangle);
			sinu = (float)sin(rotangle);
			for (int m=0; m<NvertsPerRing ; m++)
			{	tempx = SaveVertex[m].x*cosu - SaveVertex[m].y*sinu;
				tempy = SaveVertex[m].x*sinu + SaveVertex[m].y*cosu;
				SaveVertex[m].x = tempx;
				SaveVertex[m].y = tempy;
			}
		}
	}

}

int SpringObject::RenderBegin(TimeValue t, ULONG flags)
{	SetAFlag(A_RENDER);
	int renderme;
	pblock->GetValue(PB_RENDERABLE,t,renderme,FOREVER);
	if (!renderme) 
	{	MeshInvalid();
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}
	return 0;
}

int SpringObject::RenderEnd(TimeValue t)
{	ClearAFlag(A_RENDER);
	int renderme;
 	pblock->GetValue(PB_RENDERABLE,t,renderme,FOREVER);
	if (!renderme) 
	{	MeshInvalid();
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}
	return 0;
}

INode *SpringObject::GetEndNode1()
{	if (custnode)
		return (custnode);
	else
		return (NULL);
}

INode *SpringObject::GetEndNode2()
{	if (custnode2) 
		return (custnode2);
	else
		return (NULL);
}

Point3 SpringObject::ApplyAtEnd1(TimeValue t)
{	if (custnode)
	{	Matrix3 mat1 = custnode->GetObjTMAfterWSM(t);
		return (mat1.GetRow(3));
	}
	else
		return (Zero);
}

Point3 SpringObject::ApplyAtEnd2(TimeValue t)
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

#define POUNDSPERINCH	0
#define NEWTONPERMETER	1

Point3 SpringObject::Force(TimeValue t, TimeValue dt)
{	if ((custnode)&&(custnode2))
	{	Matrix3 mat1, mat2;
		mat1 = custnode->GetObjTMAfterWSM(t);
		mat2 = custnode2->GetObjTMAfterWSM(t);
		float len;
		Point3 forcedir = (mat1.GetRow(3)-mat2.GetRow(3));
		len = Length(forcedir);
		if (len < 0.01f)
			return (Zero);
		else
		{	forcedir = Normalize(forcedir);
			float freelen, kval, difflen;
			int springtype, kvalunits;
			pblock->GetValue(PB_FREELENGTH,t,freelen,ivalid);
			pblock->GetValue(PB_KVALUE,t,kval,ivalid);
			pblock->GetValue(PB_KVALUNITS,t,kvalunits,ivalid);
			pblock->GetValue(PB_SPRINGDIR,t,springtype,ivalid);
			if ((len < freelen)&&(springtype == EXTENSION))
				return (Zero);
			else if ((len > freelen)&&(springtype == COMPRESSION))
				return (Zero);
			else 
			{	difflen = freelen - len;
				difflen *= GetMeterMult();
				if (kvalunits == POUNDSPERINCH)
					kval *= 175.55441f;
				forcedir *= (kval*difflen);
				int isnonlinear;
				pblock->GetValue(PB_NONLINEAR,0,isnonlinear,FOREVER);
				if (((isnonlinear)&&(springtype == COMPRESSION)&&(len < freelen))||
					((isnonlinear)&&(springtype == BIDIRECT)&&(len < freelen)))
				{	int wiretype;
					float wiresize, coils;
					pblock->GetValue(PB_WIRETYPE,0,wiretype,FOREVER);
					if (wiretype==ROUND)
						pblock->GetValue(PB_RNDDIA,0,wiresize,FOREVER);
					else if (wiretype==RECT)
						pblock->GetValue(PB_RECTDEPTH,t,wiresize,ivalid);
					else
						pblock->GetValue(PB_DSECDEPTH,t,wiresize,ivalid);
					pblock->GetValue(PB_COILS,t,coils,ivalid);
					if (coils < 0.5f) 
						coils = 0.5f;
					float solidheight = wiresize * coils;
					if (len > solidheight)
						forcedir += forcedir*(freelen - len)/(len - solidheight);
				}
				else if (((isnonlinear)&&(springtype == BIDIRECT)&&(len > freelen))||
						((isnonlinear)&&(springtype == EXTENSION)&&(len > freelen)))
				{	int wiretype;
					float wirewidth, springdia;
					pblock->GetValue(PB_WIRETYPE,0,wiretype,FOREVER);
					if (wiretype==ROUND)
						pblock->GetValue(PB_RNDDIA,t,wirewidth,ivalid);
					else if (wiretype==RECT)
						pblock->GetValue(PB_RECTWIDTH,t,wirewidth,ivalid);
					else
						pblock->GetValue(PB_DSECWIDTH,t,wirewidth,ivalid);
					pblock->GetValue(PB_OD,t,springdia,ivalid);
					float factor = 0.001f*wirewidth/springdia;
					if (len > freelen)
						forcedir *= (float)exp((len - freelen)*factor);
				}
				return (forcedir);  //returns a force in Newtons!!
			}
		}
	}
	else
		return (Zero);
}

#define SMOOTHALL	0
#define SMOOTHNONE	1
#define SMOOTHSIDES	2
#define SMOOTHSEGS	3

BOOL SpringObject::HasUVW() 
{ 	BOOL genUVs;
	Interval v;
	pblock->GetValue(PB_MAPMEMAPME, 0, genUVs, v);
	return genUVs; 
}

void SpringObject::SetGenUVW(BOOL sw) 
{  	if (sw==HasUVW()) 
		return;
	pblock->SetValue(PB_MAPMEMAPME,0, sw);
}

void SpringObject::BuildMesh(TimeValue t)
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
	
//	if (!ivalid.InInterval(t))
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
		float Lf = 0.0f;
		Matrix3 Tlocal;
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
//			Point3 P1 = mat1.GetRow(3),
//				   P2 = mat2.GetRow(3);
			Point3 P1 = mat1*mato1.GetRow(3),
				   P2 = mat2*mato2.GetRow(3);
			Matrix3 SI = Inverse(S);
			Point3 P0 = SI*P1;
			Matrix3 T1 = mat1;
			T1.NoTrans();
			Point3 RVw = T1 * RV;
			Lf = Length(P2 - P1);
//			pblock->SetValue(PB_NOREFLENGTH,0,Lf);
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
		int wtype;
		pblock->GetValue(PB_WIRETYPE,0,wtype,ivalid);
		int NvertsPerRing=0,
			nfillets=0,
			nsides=0;
		if (wtype==ROUND)
		{	pblock->GetValue(PB_RNDSIDES,t,NvertsPerRing,ivalid);
			if (NvertsPerRing < 3) NvertsPerRing = 3;
		}
		else if (wtype==RECT)
		{	pblock->GetValue(PB_RECTFILLETSIDES,t,nfillets,ivalid);
			if (nfillets<0) nfillets = 0;
			NvertsPerRing=(nfillets>0?8+4*(nfillets-1):4);
		}
		else
		{	pblock->GetValue(PB_DSECFILLETSIDES,t,nfillets,ivalid);
			if (nfillets<0) nfillets = 0;
			pblock->GetValue(PB_DSECRNDSIDES,t,nsides,ivalid);
			if (nsides<2) nsides = 2;
			int nsm1 = nsides-1;
			NvertsPerRing=(nfillets>0?6+nsm1+2*(nfillets-1):4+nsm1);
		}
		int Nverts=0,NfacesPerEnd,NfacesPerRing,Nfaces=0,Segs;

		float C,OD;
		int SMOOTH;
		pblock->GetValue(PB_OD,t,OD,ivalid);if (OD<0.1f) OD=0.1f;
		pblock->GetValue(PB_COILS,t,C,ivalid);
		if (C < 0.5f) 
			C = 0.5f;
		pblock->GetValue(PB_SMOOTH,0,SMOOTH,FOREVER);

		int manualsegs;
		pblock->GetValue(PB_MANUALSEGSET,t,manualsegs,ivalid);
		if (manualsegs)
		{
			pblock->GetValue(PB_SEGMENTS,t,Segs,ivalid);
			if (Segs < 3) Segs = 3;
		}
		else
		{	pblock->GetValue(PB_SEGSPERTURN,t,Segs,ivalid);
			if (Segs<3) Segs = 3;
			Segs = (int)((float)Segs*C);
		}

		Nverts = (Segs + 1)*NvertsPerRing + 2;
		NfacesPerEnd = NvertsPerRing;
		NfacesPerRing = 2* NvertsPerRing;
		Nfaces =  Segs*NfacesPerRing + 2*NfacesPerEnd;
		Point3 *SaveVertex = new Point3[NvertsPerRing];
		assert(SaveVertex);
		MakeSaveVertex(SaveVertex, NvertsPerRing, nfillets, nsides, wtype, t);

		int clockwise;
		pblock->GetValue(PB_COILDIR,t,clockwise,ivalid);

		float HelixAngle = (float)atan(Lf/(C*PI*OD));  
		if (clockwise) HelixAngle = -HelixAngle;

		mesh.setNumVerts(Nverts);
		mesh.setNumFaces(Nfaces);

		int mapmenow;
		pblock->GetValue(PB_MAPMEMAPME,0,mapmenow,FOREVER);

		if (mapmenow)
		{	mesh.setNumTVerts(Nverts+Segs+1);
			mesh.setNumTVFaces(Nfaces);
		}
		else
		{	mesh.setNumTVerts(0);
			mesh.setNumTVFaces(0);
		}

		float hod = 0.5f*OD;
		int thisvert = 0,
			last = Nverts-1,
			last2 = last-1;
		int lastvpr = NvertsPerRing-1,
			maxseg = Segs+1;

		for (int i=0; i < maxseg; i++)
		{	float incr = (float)i/(float)Segs;
			float ThisAngle = C*TWOPI*incr;
			if (clockwise) ThisAngle = -ThisAngle;
			float ThisHeight = Lf*incr;
			float ThisPositionX = hod*(float)cos(ThisAngle);
			float ThisPositionY = hod*(float)sin(ThisAngle);
			Point3 ThisXAxis = Normalize(Point3(ThisPositionX, ThisPositionY, 0.0f));
//			if (clockwise) ThisXAxis = -ThisXAxis;
			Point3 ThisYAxis = ZAxis;
			Point3 pt = Zero;
			RotateOnePoint(&ThisYAxis.x,&pt.x,&ThisXAxis.x,HelixAngle);
			Point3 ThisZAxis = Normalize(ThisXAxis^ThisYAxis);
//			ThisYAxis = Normalize(ThisZAxis^ThisXAxis);
			Matrix3 RingTM;
			RingTM.SetRow(0,ThisXAxis);
			RingTM.SetRow(1,ThisYAxis);
			RingTM.SetRow(2,ThisZAxis);
			Point3 mainpos=Point3(ThisPositionX, ThisPositionY, ThisHeight);
			RingTM.SetRow(3,mainpos);
			if (!createfree) 
				RingTM = RingTM * Tlocal;

			for (int j=0; j < NvertsPerRing; j++)
			{	if (mapmenow)
					mesh.tVerts[thisvert]=Point3(0.999999f*incr, (float)j/(float)NvertsPerRing,0.5f);
				mesh.setVert(thisvert, RingTM*SaveVertex[j]);
				thisvert++;
			}
			if (mapmenow)
				mesh.tVerts[Nverts+i]=Point3(0.999999f*incr,0.999f,0.0f);

			if (i == 0)
			{	mesh.setVert(last2,(createfree?mainpos:Tlocal*mainpos));
				if (mapmenow)
					mesh.tVerts[last2]=Zero;
			}
			else if (i == Segs)
			{	mesh.setVert(last,(createfree?mainpos:Tlocal*mainpos));
				if (mapmenow)
					mesh.tVerts[last]=Ones;
			}
		}

//		Now, set up the faces
		int thisface = 0, v1, v2, v3, v4, v5, v6;
		v3 = last2;
		for (i=0; i < NvertsPerRing; i++)
		{	v1 = i;
			v2 = (i < lastvpr? v1+1 : v1-lastvpr);
			v5 = (i < lastvpr? v2 : Nverts);
			if (clockwise)
				mesh.faces[thisface].setVerts(v2,v1,v3);
			else
				mesh.faces[thisface].setVerts(v1,v2,v3);
			mesh.faces[thisface].setSmGroup((SMOOTH == 1)?0:1);
			mesh.faces[thisface].setEdgeVisFlags(1,0,0);
			mesh.faces[thisface].setMatID(0);
			if (mapmenow)
				if (clockwise)
					mesh.tvFace[thisface].setTVerts(v5,v1,v3);
				else
					mesh.tvFace[thisface].setTVerts(v1,v5,v3);
			thisface++;
		}

		for (i=0; i < Segs; i++)
			for (int j=0; j < NvertsPerRing; j++)
			{	v1 = i*NvertsPerRing + j;
				v2 = (j < lastvpr? v1+1 : v1 - lastvpr );
				v3 = v1 + NvertsPerRing;
				v4 = v2 + NvertsPerRing;
				v5 = (j < lastvpr? v2 : Nverts+i);
				v6 = (j < lastvpr? v4 : Nverts+i+1);
				if (clockwise)
					mesh.faces[thisface].setVerts(v4,v3,v1);
				else
					mesh.faces[thisface].setVerts(v1,v3,v4);
//				mesh.faces[thisface].setSmGroup(SMOOTH?2:0);
				switch (SMOOTH)
				{	case SMOOTHALL:		
						mesh.faces[thisface].setSmGroup(2); break;
					case SMOOTHNONE:	
						mesh.faces[thisface].setSmGroup(0); break;
					case SMOOTHSIDES:
						if (j == 0)
						{	mesh.faces[thisface].setSmGroup(2); break;}
						else if ((j % 2) == 0)
						{	mesh.faces[thisface].setSmGroup(4); break;}
						else
						{	mesh.faces[thisface].setSmGroup(8); break;}
					case SMOOTHSEGS:
						if (i == 0)
						{	mesh.faces[thisface].setSmGroup(2); break;}
						else if ((i % 2) == 0)
						{	mesh.faces[thisface].setSmGroup(4); break;}
						else
						{	mesh.faces[thisface].setSmGroup(8); break;}
				}
				mesh.faces[thisface].setEdgeVisFlags(1,1,0);
				mesh.faces[thisface].setMatID(1);\
				if (mapmenow)
					if (clockwise)
						mesh.tvFace[thisface].setTVerts(v6,v3,v1);
					else
						mesh.tvFace[thisface].setTVerts(v1,v3,v6);
				thisface++;
				if (clockwise)
					mesh.faces[thisface].setVerts(v1,v2,v4);
				else
					mesh.faces[thisface].setVerts(v4,v2,v1);
//				mesh.faces[thisface].setSmGroup(SMOOTH?2:0);
				switch (SMOOTH)
				{	case SMOOTHALL:		
						mesh.faces[thisface].setSmGroup(2); break;
					case SMOOTHNONE:	
						mesh.faces[thisface].setSmGroup(0); break;
					case SMOOTHSIDES:
						if (j == 0)
						{	mesh.faces[thisface].setSmGroup(2); break;}
						else if ((j % 2) == 0)
						{	mesh.faces[thisface].setSmGroup(4); break;}
						else
						{	mesh.faces[thisface].setSmGroup(8); break;}
					case SMOOTHSEGS:
						if (i == 0)
						{	mesh.faces[thisface].setSmGroup(2); break;}
						else if ((i % 2) == 0)
						{	mesh.faces[thisface].setSmGroup(4); break;}
						else
						{	mesh.faces[thisface].setSmGroup(8); break;}
				}
				mesh.faces[thisface].setEdgeVisFlags(1,1,0);
				mesh.faces[thisface].setMatID(1);
				if (mapmenow)
					if (clockwise)
						mesh.tvFace[thisface].setTVerts(v1,v5,v6);
					else
						mesh.tvFace[thisface].setTVerts(v6,v5,v1);
				thisface++;
			}
		int basevert = Segs*NvertsPerRing;

		v3 = Nverts-1;
		for (i = 0; i < NvertsPerRing; i++)
		{	v1 = i + basevert;
			v2 = (i < lastvpr? v1+1 : v1 - lastvpr);
			v5 = (i < lastvpr? v2 : Nverts+Segs);
			if (clockwise)
				mesh.faces[thisface].setVerts(v1,v2,v3);
			else
				mesh.faces[thisface].setVerts(v2,v1,v3);
			mesh.faces[thisface].setSmGroup((SMOOTH == 1)?0:1);
			mesh.faces[thisface].setEdgeVisFlags(1,0,0);
			mesh.faces[thisface].setMatID(2);;
			if (mapmenow)
				if (clockwise)
					mesh.tvFace[thisface].setTVerts(v1,v5,v3);
				else
					mesh.tvFace[thisface].setTVerts(v5,v1,v3);
			thisface++;
		}

		if (SaveVertex) 
			delete[] SaveVertex;
	}
	mesh.InvalidateTopologyCache();
	srand( (unsigned)time( NULL ) );
}

class SpringEmitterCreateCallback : public CreateMouseCallBack {
	public:
		SpringObject *ob;
		Point3 p[2];
		IPoint2 sp0, sp1;
		BOOL square;
		void Cleanup();
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
	};

int SpringEmitterCreateCallback::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat)
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
				SuspendSetKeyMode();
				ob->pblock->SetValue(PB_OD,0,0.01f);
				ob->pblock->SetValue(PB_NOREFLENGTH,0,0.01f);
				ob->pblock->SetValue(PB_RNDDIA,0,0.01f);
				ResumeSetKeyMode();
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
				
				float tempsize = 0.3f*r;
				SuspendSetKeyMode();
				ob->pblock->SetValue(PB_OD,0,2.0f*r);
				ob->pblock->SetValue(PB_RNDDIA,0,tempsize);
				ob->pblock->SetValue(PB_RECTWIDTH,0,tempsize);
				ob->pblock->SetValue(PB_RECTDEPTH,0,tempsize);
				ob->pblock->SetValue(PB_DSECWIDTH,0,tempsize);
				ob->pblock->SetValue(PB_DSECDEPTH,0,tempsize);
				ob->pmapParam->Invalidate();
				ResumeSetKeyMode();

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
				if (h<0.0f) h*=-1.0f;
				SuspendSetKeyMode();
				ob->pblock->SetValue(PB_NOREFLENGTH,0,h);
				ResumeSetKeyMode();

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

static SpringEmitterCreateCallback emitterCallback;

CreateMouseCallBack* SpringObject::GetCreateMouseCallBack() 
	{
	emitterCallback.ob = this;
	return &emitterCallback;
	}

void SpringObject::InvalidateUI()
	{
	if (pmapParam) pmapParam->Invalidate();
	}

ParamDimension *SpringObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_NOREFLENGTH:
		case PB_OD:
		case PB_RNDDIA:
		case PB_RECTWIDTH:
		case PB_RECTDEPTH:
		case PB_RECTFILLET:
		case PB_DSECWIDTH:
		case PB_DSECDEPTH:
		case PB_DSECFILLET:
		case PB_FREELENGTH:
					return stdWorldDim;
		case PB_RECTROTANGLE:
		case PB_DSECROTANGLE:
					return stdAngleDim;
		default:	
					return defaultDim;
		}
	}

TSTR SpringObject::GetParameterName(int pbIndex)
	{
	switch (pbIndex) {
		case PB_ENDSMETHOD:			return GetString(IDS_AP_ENDSMETHOD);break;
		case PB_NOREFLENGTH:		return GetString(IDS_AP_NOREFLENGTH);break;
		case PB_OD:					return GetString(IDS_AP_OD);break;
		case PB_COILS:				return GetString(IDS_AP_COILS);break;
		case PB_COILDIR:			return GetString(IDS_AP_COILDIR);break;
		case PB_MANUALSEGSET:		return GetString(IDS_AP_SEGMENTMETHOD);break;
		case PB_SEGSPERTURN:		return GetString(IDS_AP_SEGSPERTURN);break;
		case PB_SEGMENTS:			return GetString(IDS_AP_SEGMENTS);break;
		case PB_SMOOTH:				return GetString(IDS_AP_SMOOTH);break;
		case PB_RENDERABLE:			return GetString(IDS_AP_RENDER);break;

		case PB_WIRETYPE:			return GetString(IDS_AP_WIRETYPE);break;
		case PB_RNDDIA:				return GetString(IDS_AP_RNDDIA);break;
		case PB_RNDSIDES:			return GetString(IDS_AP_RNDSIDES);break;

		case PB_RECTWIDTH:			return GetString(IDS_AP_RECTWIDTH);break;
		case PB_RECTDEPTH:			return GetString(IDS_AP_RECTDEPTH);break;
		case PB_RECTFILLET:			return GetString(IDS_AP_RFILLET);break;
		case PB_RECTFILLETSIDES:	return GetString(IDS_AP_RFILLETSIDES);break;
		case PB_RECTROTANGLE:		return GetString(IDS_AP_RECTROT);break;

		case PB_DSECWIDTH:			return GetString(IDS_AP_DSECWIDTH);break;
		case PB_DSECDEPTH:			return GetString(IDS_AP_DSECDEPTH);break;
		case PB_DSECFILLET:			return GetString(IDS_AP_DFILLET);break;
		case PB_DSECFILLETSIDES:	return GetString(IDS_AP_DFILLETSIDES);break;
		case PB_DSECRNDSIDES:		return GetString(IDS_AP_DSECRNDSIDES);break;
		case PB_DSECROTANGLE:		return GetString(IDS_AP_DSECROT);break;

		case PB_FREELENGTH:			return GetString(IDS_AP_FREELENGTH);break;
		case PB_KVALUE:				return GetString(IDS_AP_KVALUE);break;
		case PB_KVALUNITS:			return GetString(IDS_AP_KVALUNITS);break;
		case PB_SPRINGDIR:			return GetString(IDS_AP_SPRINGDIR);break;
		case PB_MAPMEMAPME:			return GetString(IDS_AP_MAPME);break;
		case PB_NONLINEAR:			return GetString(IDS_AP_NL);break;
		default: 					return TSTR(_T(""));
		}
	}

//--- Spring particle -----------------------------------------------

RefTargetHandle SpringObject::Clone(RemapDir& remap) 
	{
	SpringObject* newob = new SpringObject();	
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

static float findmappos(float curpos)
{ float mappos;

  return(mappos=((mappos=curpos)<0.0f?0.0f:(mappos>1.0f?1.0f:mappos)));
}



RefTargetHandle SpringObject::GetReference(int i)
{	switch(i) {
		case PBLK: return(RefTargetHandle)pblock;
		case CUSTNODE: return (RefTargetHandle)custnode;
		case CUSTNODE2: return (RefTargetHandle)custnode2;
		default: return NULL;
		}
	}

void SpringObject::SetReference(int i, RefTargetHandle rtarg) { 
	switch(i) {
		case PBLK: pblock=(IParamBlock*)rtarg; return;
		case CUSTNODE: custnode = (INode *)rtarg; return;
		case CUSTNODE2: custnode2 = (INode *)rtarg; return;
		}
	}

RefResult SpringObject::NotifyRefChanged( 
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
			{ SimpleObject::NotifyRefChanged(changeInt,hTarget,partID,message);
			  cancelled=FALSE;
			}
			break;
		default: SimpleObject::NotifyRefChanged(changeInt,hTarget,partID,message);
		}
	return REF_SUCCEED;
	}

/*class SpringPostLoadCallback : public PostLoadCallback {
	public:
		ParamBlockPLCB *cb;
		SpringPostLoadCallback(ParamBlockPLCB *c) {cb=c;}
		void proc(ILoad *iload) {
			DWORD oldVer = ((SpringObject*)(cb->targ))->pblock->GetVersion();
			ReferenceTarget *targ = cb->targ;
			cb->proc(iload);
			if (oldVer<1) {	
				((SpringObject*)targ)->pblock->SetValue(PB_SYMMETRY,0,0);
				}
			delete this;
			}
	};*/

#define COM_CUSTNAME_CHUNK	0x0100
#define COM_CUSTNAME2_CHUNK	0x0101
#define COM_CNODE_CHUNK		0x0102

IOResult SpringObject::Save(ISave *isave)
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

IOResult SpringObject::Load(ILoad *iload)
	{
	ULONG nb;
	IOResult res = IO_OK;
	int cnmtl=0;
	int refid;
	
//	iload->RegisterPostLoadCallback(
			//new SpringPostLoadCallback(
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

const TCHAR *SpringClassDesc::ClassName ()	{return GetString(IDS_AP_SPRING);}
const TCHAR *SpringClassDesc::Category ()	{return GetString(IDS_EP_DYNPRIMS);}
TCHAR *SpringObject::GetObjectName() {return GetString(IDS_AP_SPRING_OBJECT);}

#endif // NO_OBJECT_SPRING