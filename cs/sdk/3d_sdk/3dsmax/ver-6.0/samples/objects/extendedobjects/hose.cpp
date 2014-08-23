#include "solids.h"
#include "spline3d.h"
#include "iparamm.h"
#include "Simpobj.h"

#include "interpik.h"
#include "texutil.h"
#include "macrorec.h"

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
#define BMIN_LENGTH		float(0.0)

static Class_ID HOSE_CLASS_ID(0x69f96a5d, 0x235c430a);

#define PBLK		0
#define CUSTNODE 	1
#define CUSTNODE2 	2

const float EPSILON=0.0001f;
const Point3 Ones=Point3(1.0f,1.0f,1.0f);

class HosePickOperand;
class HoseObject;

class HoseObject : public SimpleObject, public IParamArray {
	public:
		static IParamMap *pmapParam;
		HoseObject();
		~HoseObject();
		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message);
	
		static HoseObject *editOb;
		INode *custnode2,*custnode,*thisnode;
		TSTR custname,custname2;
		static IObjParam *ip;
		Matrix3 S;
		static HWND hParams;
		ICustButton *iPick,*iPick2;

		static BOOL creating;
		static HosePickOperand pickCB;
		BOOL increate;
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
		Class_ID ClassID() {return HOSE_CLASS_ID;} 
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
};

//--- ClassDescriptor and class vars ---------------------------------

class HoseClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new HoseObject;}
	const TCHAR *	ClassName(); 
	SClass_ID		SuperClassID() {return GEOMOBJECT_CLASS_ID;}
	Class_ID		ClassID() {return HOSE_CLASS_ID;}
	const TCHAR* 	Category(); 
	void			ResetClassParams(BOOL fileReset);
	int 			BeginCreate(Interface *i);
	int 			EndCreate(Interface *i);
	};

static HoseClassDesc HoseDesc;
ClassDesc* GetHoseDesc() {return &HoseDesc;}

class HosePickOperand : 
		public PickModeCallback,
		public PickNodeCallback {
	public:		
		HoseObject *po;
		int dodist,repi;

		HosePickOperand() {po=NULL;dodist=-1;}

		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);

		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);

		BOOL RightClick(IObjParam *ip, ViewExp *vpt) { return TRUE; }
		BOOL Filter(INode *node);
		
		PickNodeCallback *GetFilter() {return this;}
	};

IParamMap *HoseObject::pmapParam;
IObjParam *HoseObject::ip    = NULL;
BOOL HoseObject::creating    = FALSE;
HWND HoseObject::hParams		=NULL;
HosePickOperand HoseObject::pickCB;
HoseObject *HoseObject::editOb	=NULL;

//--- Parameter map/block descriptors -------------------------------

#define SIZEFACTOR (float(TIME_TICKSPERSEC)/120.0f)

#define PB_ENDSMETHOD		0
#define PB_NOREFLENGTH		1
#define PB_SEGMENTS			2
#define PB_SMOOTH			3
#define PB_RENDERABLE		4

#define PB_WIRETYPE			5
#define PB_RNDDIA			6
#define PB_RNDSIDES			7

#define PB_RECTWIDTH		8
#define PB_RECTDEPTH		9
#define PB_RECTFILLET		10
#define PB_RECTFILLETSIDES	11
#define PB_RECTROTANGLE		12

#define PB_DSECWIDTH		13
#define PB_DSECDEPTH		14
#define PB_DSECFILLET		15
#define PB_DSECFILLETSIDES	16
#define PB_DSECRNDSIDES		17
#define PB_DSECROTANGLE		18

#define PB_MAPMEMAPME		19

#define PB_FLEXON			20
#define PB_FLEXSTART		21
#define PB_FLEXSTOP			22
#define PB_FLEXCYCLES		23
#define PB_FLEXDIAMETER		24

#define PB_TENSION1			25
#define PB_TENSION2			26

static int endmethodIDs[] = {IDC_SPRING_REFS,IDC_SPRING_DIMS};

const float WIREMIN = 0.0f;
const float FILLETMIN = 0.0f;

#define ROUND	0
#define RECT	1
#define DSECT	2

static int wiresectionIDs[] = {IDC_SPRING_ROUNDWIRE,IDC_SPRING_RECTWIRE,IDC_SPRING_DSECTIONWIRE};

static int smoothtypeIDs[] = {IDC_SPRING_SMALL,IDC_SPRING_SMNONE,IDC_SPRING_SMSIDES,IDC_SPRING_SMSEGS};

//
// Parameters

static ParamUIDesc descHoseParam1[] = {

	// End Method
	ParamUIDesc(PB_ENDSMETHOD,TYPE_RADIO,endmethodIDs,2),

	// No reference version Length
	ParamUIDesc(
		PB_NOREFLENGTH,
		EDITTYPE_UNIVERSE,
		IDC_SPRING_LENGTH,IDC_SPRING_LENGTHSPIN,
		0.0f,99999999.0f,
		0.1f),	
		
	// Segments along coils
	ParamUIDesc(
		PB_SEGMENTS,
		EDITTYPE_INT,
		IDC_SPRING_COILSEGS,IDC_SPRING_COILSEGSSPIN,
		3.0f,100000.0f,
		1.0f),	
		
	// Smooth Hose
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

	// Generate text coords
	ParamUIDesc(PB_MAPMEMAPME,TYPE_SINGLECHEKBOX,IDC_SPRING_MAPME),			

	// Flex On
	ParamUIDesc(PB_FLEXON,TYPE_SINGLECHEKBOX,IDC_HOSE_FLEXON),			

	// Flex Start Position
	ParamUIDesc(
		PB_FLEXSTART,
		EDITTYPE_FLOAT,
		IDC_HOSE_FLEX1,IDC_HOSE_FLEX1SPIN,
		0.0f,50.0f,
		1.0f,
		stdPercentDim),	

	// Flex Stop Position
	ParamUIDesc(
		PB_FLEXSTOP,
		EDITTYPE_FLOAT,
		IDC_HOSE_FLEX2,IDC_HOSE_FLEX2SPIN,
		50.0f,100.0f,
		1.0f,
		stdPercentDim),	

	// Flex Cycles
	ParamUIDesc(
		PB_FLEXCYCLES,
		EDITTYPE_INT,
		IDC_HOSE_FLEXCYCLES,IDC_HOSE_FLEXCYCLESSPIN,
		1.0f,1000.0f,
		1.0f),	

	// Flex Diameter
	ParamUIDesc(
		PB_FLEXDIAMETER,
		EDITTYPE_FLOAT,
		IDC_HOSE_DIACHANGE,IDC_HOSE_DIACHANGESPIN,
		-50.0f,500.0f,
		1.0f,
		stdPercentDim),	

	// Spline Tension 1
	ParamUIDesc(
		PB_TENSION1,
		EDITTYPE_FLOAT,
		IDC_HOSE_TENSION1,IDC_HOSE_TENSION1SPIN,
		0.0f,1000.0f,
		1.0f),	

	// Spline Tension 2
	ParamUIDesc(
		PB_TENSION2,
		EDITTYPE_FLOAT,
		IDC_HOSE_TENSION2,IDC_HOSE_TENSION2SPIN,
		0.0f,1000.0f,
		1.0f),	
};

#define HOSEPARAMSDESC_LENGTH 27

ParamBlockDescID descHoseVer0[] = {
	{ TYPE_INT, NULL, FALSE, 0 }, // End Method
	{ TYPE_FLOAT, NULL, TRUE, 1 }, // No Ref Length
	{ TYPE_INT, NULL, TRUE, 2 }, // Coil Seg Count
	{ TYPE_INT, NULL, FALSE, 3 }, // Smooth Control
	{ TYPE_INT, NULL, FALSE, 4 }, // Renderable

	{ TYPE_INT, NULL, FALSE, 5 }, // Wire Section Type
	{ TYPE_FLOAT, NULL, TRUE, 6 }, // Round Wire Dia
	{ TYPE_INT, NULL, TRUE, 7 },  // Wire Sides
	{ TYPE_FLOAT, NULL, TRUE, 8 }, // Rect Wire Width
	{ TYPE_FLOAT, NULL, TRUE, 9 }, // Rect Wire Depth
	{ TYPE_FLOAT, NULL, TRUE, 10 }, // Rect Wire Fillet Size
	{ TYPE_INT, NULL, TRUE, 11 }, // Rect Wire Fillet Sides
	{ TYPE_FLOAT, NULL, TRUE, 12 }, // Rect Wire Rot Angle
	{ TYPE_FLOAT, NULL, TRUE, 13 }, // D Wire Width
	{ TYPE_FLOAT, NULL, TRUE, 14 }, // D Wire Depth
	{ TYPE_FLOAT, NULL, TRUE, 15 }, // D Wire Fillet Size
	{ TYPE_INT, NULL, TRUE, 16 }, // D Wire Fillet Sides
	{ TYPE_INT, NULL, TRUE, 17 }, // D Wire Round Sides
	{ TYPE_FLOAT, NULL, TRUE, 18 }, // D Wire Rotation Angle

	{ TYPE_INT, NULL, FALSE, 19 },  // gen tex coords
	{ TYPE_INT, NULL, FALSE, 20 },  // flex section on
	{ TYPE_FLOAT, NULL, TRUE, 21 },  // flex section start
	{ TYPE_FLOAT, NULL, TRUE, 22 },  // flex section stop
	{ TYPE_INT, NULL, TRUE, 23 },  // flex section cycles
	{ TYPE_FLOAT, NULL, TRUE, 24 },  // flex section diameter change

	{ TYPE_FLOAT, NULL, TRUE, 25 },  // tension 1
	{ TYPE_FLOAT, NULL, TRUE, 26 },  // tension 2
};

#define PBLOCK_HOSE_LENGTH	27

// Array of old versions
// static ParamVersionDesc versions[] = {ParamVersionDesc(descHoseVer0,24,0)};

#define NUM_OLDVERSIONS	0
#define CURRENT_VERSION	0

// Current version
static ParamVersionDesc curVersion(descHoseVer0,PBLOCK_HOSE_LENGTH,CURRENT_VERSION);


class CreateMHoseProc : public MouseCallBack,ReferenceMaker {
	private:
		IObjParam *ip;
		void Init(IObjParam *i) {ip=i;}
		CreateMouseCallBack *createCB;	
		INode *CloudNode;
		HoseObject *SSBlizObject;
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
		
		CreateMHoseProc()
			{
			ignoreSelectionChange = FALSE;
			}
//		int createmethod(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
		int proc( HWND hwnd, int msg, int point, int flag, IPoint2 m );
	};


class CreateCPartRestoreNode : public RestoreObj {
	public:   		
		HoseObject *obj;
		TSTR name,namer;
		INode *save,*saver;
		CreateCPartRestoreNode(HoseObject *o) {
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
		HoseObject *obj;
		TSTR name,namer;
		INode *save,*saver;
		CreateCPart2RestoreNode(HoseObject *o) {
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

#define CID_CREATEHOSEMODE	CID_USER +76

class CreateMHoseMode : public CommandMode {		
	public:		
		CreateMHoseProc proc;
		IObjParam *ip;
		HoseObject *obj;
		void Begin( IObjCreate *ioc, ClassDesc *desc ) { proc.Begin( ioc, desc ); }
		void End() { proc.End(); }
		void JumpStart(IObjParam *i,HoseObject*o);

		int Class() {return CREATE_COMMAND;}
		int ID() { return CID_CREATEHOSEMODE; }
		MouseCallBack *MouseProc(int *numPoints) {*numPoints = 10000; return &proc;}
		ChangeForegroundCallback *ChangeFGProc() {return CHANGE_FG_SELECTED;}
		BOOL ChangeFG( CommandMode *oldMode ) { return (oldMode->ChangeFGProc() != CHANGE_FG_SELECTED); }
		void EnterMode() 
		{ GetCOREInterface()->PushPrompt(GetString(IDS_AP_CREATEMODE));
		  SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR)));
		}
		void ExitMode() {GetCOREInterface()->PopPrompt();SetCursor(LoadCursor(NULL, IDC_ARROW));}
	};
static CreateMHoseMode theCreateMHoseMode;

void HoseClassDesc::ResetClassParams(BOOL fileReset)
	{
	}

RefResult CreateMHoseProc::NotifyRefChanged(
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
						{  theCreateMHoseMode.JumpStart(SSBlizObject->ip,SSBlizObject);
							createInterface->SetCommandMode(&theCreateMHoseMode);
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

void CreateMHoseProc::Begin( IObjCreate *ioc, ClassDesc *desc )
	{
	createInterface = ioc;
	cDesc           = desc;
	attachedToNode  = FALSE;
	createCB        = NULL;
	CloudNode         = NULL;
	SSBlizObject       = NULL;
	CreateNewObject();
	}

void CreateMHoseProc::CreateNewObject()
{
	SuspendSetKeyMode();
	createInterface->GetMacroRecorder()->BeginCreate(cDesc);
	SSBlizObject = (HoseObject*)cDesc->Create();
	lastPutCount  = theHold.GetGlobalPutCount();
	
	// Start the edit params process
	if ( SSBlizObject ) {
#ifndef NO_CREATE_TASK	// russom - 12/04/01
		SSBlizObject->BeginEditParams( (IObjParam*)createInterface, BEGIN_EDIT_CREATE, NULL );
#endif
#ifdef _OSNAP
		SSBlizObject->SetAFlag(A_OBJ_LONG_CREATE);
#endif
		}	
	ResumeSetKeyMode();
}

void CreateMHoseProc::End()
{ if ( SSBlizObject ) 
	{ 
 #ifdef _OSNAP
		SSBlizObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
#ifndef NO_CREATE_TASK	// russom - 12/04/01
		SSBlizObject->EndEditParams( (IObjParam*)createInterface, 
	                    	          END_EDIT_REMOVEUI, NULL);
#endif
		if ( !attachedToNode ) 
		{	// RB 4-9-96: Normally the hold isn't holding when this 
			// happens, but it can be in certain situations (like a Hose view paste)
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

void CreateMHoseMode::JumpStart(IObjParam *i,HoseObject *o)
{
	ip  = i;
	obj = o;
	//MakeRefByID(FOREVER,0,svNode);
#ifndef NO_CREATE_TASK	// russom - 12/04/01
	obj->BeginEditParams(i,BEGIN_EDIT_CREATE,NULL);
#endif
}
void FixHoseFillet(IParamBlock *pblock,TimeValue t,HWND hWnd,BOOL increate)
{ float width,height,fillet;

	pblock->GetValue(PB_RECTWIDTH,t,width,FOREVER);
	pblock->GetValue(PB_RECTDEPTH,t,height,FOREVER);
	pblock->GetValue(PB_RECTFILLET,(increate?0:t),fillet,FOREVER);
	float hh=0.5f*(float)fabs(height),ww=0.5f*(float)fabs(width),maxf=(hh>ww?ww:hh);
	if (hWnd)
	{ ISpinnerControl *spin2 = GetISpinner(GetDlgItem(hWnd,IDC_SPRING_RECWIREFILLETSPIN));
	  spin2->SetLimits(BMIN_LENGTH,maxf,FALSE);
	  ReleaseISpinner(spin2);
	}
	if (fillet>maxf) pblock->SetValue(PB_RECTFILLET,(increate?0:t),maxf);
}

int HoseClassDesc::BeginCreate(Interface *i)
{	
	SuspendSetKeyMode();
	IObjCreate *iob = i->GetIObjCreate();
	theCreateMHoseMode.Begin(iob,this);
	iob->PushCommandMode(&theCreateMHoseMode);
	return TRUE;
}

int HoseClassDesc::EndCreate(Interface *i)
{
	ResumeSetKeyMode();
	theCreateMHoseMode.End();
	i->RemoveMode(&theCreateMHoseMode);
	macroRec->EmitScript();  // 10/00
	return TRUE;
}

int CreateMHoseProc::proc(HWND hwnd,int msg,int point,int flag,
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
	if ((res == CREATE_STOP)||(res==CREATE_ABORT))
		vpx->ReleaseImplicitGrid();
	createInterface->ReleaseViewport(vpx); 
	return res;
}
BOOL HoseObject::OKtoDisplay(TimeValue t) 
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

BOOL HosePickOperand::Filter(INode *node)
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

BOOL HosePickOperand::HitTest(
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

void HoseObject::ShowName()
{ TSTR name=(custnode ? custname : TSTR(GetString(IDS_AP_NONE)));
  SetWindowText(GetDlgItem(hParams, IDC_SPRING_OBJ1NAME), name);
}
void HoseObject::ShowName2()
{ TSTR name=(custnode2 ? custname2 : TSTR(GetString(IDS_AP_NONE)));
  SetWindowText(GetDlgItem(hParams, IDC_SPRING_OBJ2NAME), name);
}
void HoseObject::ShowNames()
{ ShowName();ShowName2();
}

BOOL HosePickOperand::Pick(IObjParam *ip,ViewExp *vpt)
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
		theCreateMHoseMode.JumpStart(ip,po);
		ip->SetCommandMode(&theCreateMHoseMode);
		ip->RedrawViews(ip->GetTime());
		return FALSE;
	} else {return TRUE;}
}

void HosePickOperand::EnterMode(IObjParam *ip)
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

void HosePickOperand::ExitMode(IObjParam *ip)
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

class HoseParamDlg : public ParamMapUserDlgProc {
	public:
		HoseObject *po;

		HoseParamDlg(HoseObject *p) {po=p; uiValid = false;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void Update(TimeValue t);
		void BoundParams(BOOL ison);
		void FlexParams(BOOL ison);
		void HoseShape(int type);
		void DeleteThis() {delete this;}
		BOOL uiValid;
		void InvalidateUI (HWND hWnd) 
		{ 
			if (hWnd) InvalidateRect (hWnd, NULL, false); 
			uiValid = true; 
		}
	};

void RoundParams(HWND hwnd,BOOL ison)
{ 
	EnableWindow(GetDlgItem(hwnd,IDC_HOSE_RNDDIA_LABEL),ison);
	EnableWindow(GetDlgItem(hwnd,IDC_HOSE_RNDSIDES_LABEL),ison);
	if (ison)
	{ 
		SpinnerOn(hwnd,IDC_SPRING_RNDWIREDIASPIN);
		SpinnerOn(hwnd,IDC_SPRING_RNDWIRESIDESSPIN);
	}
	else
	{ 
		SpinnerOff(hwnd,IDC_SPRING_RNDWIREDIASPIN);
		SpinnerOff(hwnd,IDC_SPRING_RNDWIRESIDESSPIN);
	}
	
}
void RectParams(HWND hwnd,BOOL ison)
{ 
	EnableWindow(GetDlgItem(hwnd, IDC_HOSE_RECTWIDTH_LABEL),ison);
	EnableWindow(GetDlgItem(hwnd, IDC_HOSE_RECTDEPTH_LABEL),ison);
	EnableWindow(GetDlgItem(hwnd, IDC_HOSE_RECTFILLET_LABEL),ison);
	EnableWindow(GetDlgItem(hwnd, IDC_HOSE_RECTFILLETSEGS_LABEL),ison);
	EnableWindow(GetDlgItem(hwnd, IDC_HOSE_RECTROTATION_LABEL),ison);
	if (ison)
	{ 
		SpinnerOn(hwnd,IDC_SPRING_RECTWIREWIDSPIN);
		SpinnerOn(hwnd,IDC_SPRING_RECWIREDEPTHSPIN);
		SpinnerOn(hwnd,IDC_SPRING_RECWIREFILLETSPIN);
		SpinnerOn(hwnd,IDC_SPRING_RECWIREFILLETSIDESSPIN);
		SpinnerOn(hwnd,IDC_SPRING_RECTROTSPIN);
	}
	else
	{ 
		SpinnerOff(hwnd,IDC_SPRING_RECTWIREWIDSPIN);
		SpinnerOff(hwnd,IDC_SPRING_RECWIREDEPTHSPIN);
		SpinnerOff(hwnd,IDC_SPRING_RECWIREFILLETSPIN);
		SpinnerOff(hwnd,IDC_SPRING_RECWIREFILLETSIDESSPIN);
		SpinnerOff(hwnd,IDC_SPRING_RECTROTSPIN);
	}
}
void DSectParams(HWND hwnd,BOOL ison)
{ 
	EnableWindow(GetDlgItem(hwnd, IDC_HOSE_DSECTWIDTH_LABEL),ison);
	EnableWindow(GetDlgItem(hwnd, IDC_HOSE_DSECTDEPTH_LABEL),ison);
	EnableWindow(GetDlgItem(hwnd, IDC_HOSE_DSECTRS_LABEL),ison);
	EnableWindow(GetDlgItem(hwnd, IDC_HOSE_DSECTFILLET_LABEL),ison);
	EnableWindow(GetDlgItem(hwnd, IDC_HOSE_DSECTFILLETSEGS_LABEL),ison);
	EnableWindow(GetDlgItem(hwnd, IDC_HOSE_DSECTROTATION_LABEL),ison);
	if (ison)
	{ 
		SpinnerOn(hwnd,IDC_SPRING_DWIREWIDSPIN);
		SpinnerOn(hwnd,IDC_SPRING_DWIREDEPTHSPIN);
		SpinnerOn(hwnd,IDC_SPRING_DWIRERNDSIDESSPIN);
		SpinnerOn(hwnd,IDC_SPRING_DWIREFILLETSPIN);
		SpinnerOn(hwnd,IDC_SPRING_DWIREFILLETSIDESSPIN);
		SpinnerOn(hwnd,IDC_SPRING_DROTSPIN);
	}
	else
	{ 
		SpinnerOff(hwnd,IDC_SPRING_DWIREWIDSPIN);
		SpinnerOff(hwnd,IDC_SPRING_DWIREDEPTHSPIN);
		SpinnerOff(hwnd,IDC_SPRING_DWIRERNDSIDESSPIN);
		SpinnerOff(hwnd,IDC_SPRING_DWIREFILLETSPIN);
		SpinnerOff(hwnd,IDC_SPRING_DWIREFILLETSIDESSPIN);
		SpinnerOff(hwnd,IDC_SPRING_DROTSPIN);
	}
}

void HoseParamDlg::HoseShape(int type)
{
  if (type==ROUND) 
  { 
    RoundParams(po->hParams,TRUE);
    RectParams(po->hParams,FALSE);
    DSectParams(po->hParams,FALSE);
  }
  else if (type==RECT)
  { RoundParams(po->hParams,FALSE);
    RectParams(po->hParams,TRUE);
    DSectParams(po->hParams,FALSE);
  }
  else
  { RoundParams(po->hParams,FALSE);
    RectParams(po->hParams,FALSE);
    DSectParams(po->hParams,TRUE);
  }
  uiValid = FALSE;  // labels have changed, so mark UI as dirty
}
void HoseParamDlg::FlexParams(BOOL ison)
{ 
	EnableWindow(GetDlgItem(po->hParams, IDC_HOSE_FLEXSTARTS_LABEL ),ison);
	EnableWindow(GetDlgItem(po->hParams, IDC_HOSE_FLEXENDS_LABEL   ),ison);
	EnableWindow(GetDlgItem(po->hParams, IDC_HOSE_FLEXCYCLES_LABEL ),ison);
	EnableWindow(GetDlgItem(po->hParams, IDC_HOSE_FLEXDIA_LABEL    ),ison);
	if (ison)
	{ 
		SpinnerOn(po->hParams,IDC_HOSE_FLEX1SPIN);
		SpinnerOn(po->hParams,IDC_HOSE_FLEX2SPIN);
		SpinnerOn(po->hParams,IDC_HOSE_FLEXCYCLESSPIN);
		SpinnerOn(po->hParams,IDC_HOSE_DIACHANGESPIN);
	}
	else
	{ 
		SpinnerOff(po->hParams,IDC_HOSE_FLEX1SPIN);
		SpinnerOff(po->hParams,IDC_HOSE_FLEX2SPIN);
		SpinnerOff(po->hParams,IDC_HOSE_FLEXCYCLESSPIN);
		SpinnerOff(po->hParams,IDC_HOSE_DIACHANGESPIN);
	}
	// labels have changed, so mark UI as dirty
	//
	uiValid = FALSE;  
}
void HoseParamDlg::BoundParams(BOOL ison)
{ 
	BOOL ok=ison;
	EnableWindow(GetDlgItem(po->hParams,IDC_HOSE_HEIGHT_LABEL),!ison);
	if (ison)
	{ 
		SpinnerOff(po->hParams,IDC_SPRING_LENGTHSPIN);
		ok=(po->thisnode!=NULL);
	}
	else
	{ 
		SpinnerOn(po->hParams,IDC_SPRING_LENGTHSPIN);
	}
	if ((po->iPick->IsEnabled()!=ok)||(po->iPick2->IsEnabled()!=ok))
	{ 

		if ((po->pickCB.dodist<0)||(!ok))
		{ 
			TurnButton(po->hParams,IDC_SPRING_PICKOBJECT2,ok);
			TurnButton(po->hParams,IDC_SPRING_PICKOBJECT1,ok);
			EnableWindow(GetDlgItem(po->hParams,IDC_SPRING_OBJ2NAME),ok);
			EnableWindow(GetDlgItem(po->hParams,IDC_SPRING_OBJ1NAME),ok);
			EnableWindow(GetDlgItem(po->hParams,IDC_SPRING_OBJ2NAMECAPTION),ok);
			EnableWindow(GetDlgItem(po->hParams,IDC_SPRING_OBJ1NAMECAPTION),ok);
			EnableWindow(GetDlgItem(po->hParams,IDC_HOSE_TENSION1_LABEL),ok);
			EnableWindow(GetDlgItem(po->hParams,IDC_HOSE_TENSION2_LABEL),ok);

			if (ok)
			{ 
				SpinnerOn(po->hParams,IDC_HOSE_TENSION1SPIN);
				SpinnerOn(po->hParams,IDC_HOSE_TENSION2SPIN);
			}
			else 
			{ 
				SpinnerOff(po->hParams,IDC_HOSE_TENSION1SPIN);
				SpinnerOff(po->hParams,IDC_HOSE_TENSION2SPIN);
			}
		}
	}
	// labels have changed, so mark UI as dirty
	//
	uiValid = FALSE;  
}

void HoseParamDlg::Update(TimeValue t)
{ if (!po->editOb) return;
  po->ShowNames();
  int emethod,flexon,hshape;
  po->pblock->GetValue(PB_ENDSMETHOD,0,emethod,FOREVER);
  BoundParams(!emethod);
  po->pblock->GetValue(PB_FLEXON,0,flexon,FOREVER);
  FlexParams(flexon);
  po->pblock->GetValue(PB_WIRETYPE,0,hshape,FOREVER);
  HoseShape(hshape);
}

BOOL HoseParamDlg::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{	
	switch (msg) 
	{
	case WM_INITDIALOG: 
		{
			uiValid = false;  // to redraw the labels properly
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
		/*			  if (HIWORD(wParam)) 
		{ switch ( LOWORD(wParam) ) 
		{
		}
		}*/
		}
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) 
		{ 
		case IDC_SPRING_PICKOBJECT1: 
			{
				if (po->ip->GetCommandMode()->ID() == CID_STDPICK) 
				{ 
					if (po->creating) 
					{  
						theCreateMHoseMode.JumpStart(po->ip,po);
						po->ip->SetCommandMode(&theCreateMHoseMode);
					} 
					else 
					{
						po->ip->SetStdCommandMode(CID_OBJMOVE);
					}
				} 
				else 
				{ 
					po->pickCB.po = po;	
					po->pickCB.dodist=0;
					po->ip->SetPickMode(&po->pickCB);
				}
				break;
			}
		case IDC_SPRING_PICKOBJECT2: 
			{if (po->ip->GetCommandMode()->ID() == CID_STDPICK) 
			{ if (po->creating) 
			{  theCreateMHoseMode.JumpStart(po->ip,po);
			po->ip->SetCommandMode(&theCreateMHoseMode);
						} else {po->ip->SetStdCommandMode(CID_OBJMOVE);}
			} else 
			{ po->pickCB.po = po;	
			po->pickCB.dodist=1;
			po->ip->SetPickMode(&po->pickCB);
			}
			break;
			}			   
		case IDC_HOSE_FLEXON: 
			{ int flexon; po->pblock->GetValue(PB_FLEXON,0,flexon,FOREVER);
			FlexParams(flexon);				  
			}
			break;				   
		case IDC_SPRING_ROUNDWIRE: 
			{ 
				HoseShape(ROUND);
			}
			break;				   
		case IDC_SPRING_RECTWIRE: 
			{ HoseShape(RECT);
			}
			break;				   
		case IDC_SPRING_DSECTIONWIRE: 
			{ HoseShape(DSECT);	
			}
			break;				   
		case IDC_SPRING_REFS: 
			{ BoundParams(TRUE);				  
			}
			break;				   
		case IDC_SPRING_DIMS: 
			{ BoundParams(FALSE);
			if (po->ip->GetCommandMode()->ID() == CID_STDPICK) 
			{ if (po->creating) 
			{ theCreateMHoseMode.JumpStart(po->ip,po);
			po->ip->SetCommandMode(&theCreateMHoseMode);
					  } else {po->ip->SetStdCommandMode(CID_OBJMOVE);}
			}
			}
			break;
		case IDC_SPRING_RECWIREFILLETSPIN: 
			{ FixHoseFillet(po->pblock,t,hWnd,po->increate);				  
			}
			break;				   	
		}
		default:
			if (!uiValid) InvalidateUI (hWnd);  // redraw the UI for the dirty labels
			return FALSE;
		
	}
	return TRUE;
}

//--- HoseObject Methods--------------------------------------------

HoseObject::HoseObject()
{	int FToTick=(int)((float)TIME_TICKSPERSEC/(float)GetFrameRate());
	int length = PBLOCK_HOSE_LENGTH;
	MakeRefByID(FOREVER, 0, CreateParameterBlock(descHoseVer0, length, CURRENT_VERSION));
	assert(pblock);

	pblock->SetValue(PB_ENDSMETHOD,0,1);
	pblock->SetValue(PB_NOREFLENGTH,0,1.0f);
	pblock->SetValue(PB_SEGMENTS,0,45);
	pblock->SetValue(PB_SMOOTH,0,0);
	pblock->SetValue(PB_RENDERABLE,0,1);

	pblock->SetValue(PB_WIRETYPE,0,0);
	pblock->SetValue(PB_RNDDIA,0,0.2f);
	pblock->SetValue(PB_RNDSIDES,0,8);
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

	pblock->SetValue(PB_MAPMEMAPME,0,TRUE);

	pblock->SetValue(PB_FLEXON,0,1);
	pblock->SetValue(PB_FLEXSTART,0,0.1f);
	pblock->SetValue(PB_FLEXSTOP,0,0.9f);
	pblock->SetValue(PB_FLEXCYCLES,0,5);
	pblock->SetValue(PB_FLEXDIAMETER,0,-0.2f);

	pblock->SetValue(PB_TENSION1,0,100.0f);
	pblock->SetValue(PB_TENSION2,0,100.0f);

	S.IdentityMatrix();
	iPick=NULL;
	iPick2=NULL;
	thisnode=NULL;
	custnode=NULL;
	custnode2=NULL;
	custname=TSTR(_T(" "));
	custname2=TSTR(_T(" "));
	increate=FALSE;
}

HoseObject::~HoseObject()
{	DeleteAllRefsFromMe();
}

void HoseObject::BeginEditParams(
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
		descHoseParam1,HOSEPARAMSDESC_LENGTH,
		pblock,
		ip,
		hInstance,
		MAKEINTRESOURCE(IDD_HOSED),
		GetString(IDS_AP_HOSEPARAMS),
		0);		
	}
	hParams=pmapParam->GetHWnd();
	if (pmapParam) pmapParam->SetUserDlgProc(new HoseParamDlg(this));
}

void HoseObject::EndEditParams(
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

void HoseObject::MapKeys(TimeMap *map,DWORD flags)
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

void HoseObject::GetWorldBoundBox(
		TimeValue t, INode *inode, ViewExp* vpt, Box3& box)
	{	thisnode=inode;    
		SimpleObject::GetWorldBoundBox(t,inode,vpt,box);
	}  

void HoseObject::MakeSaveVertex(Point3 *SaveVertex,int NvertsPerRing, int nfillets, int nsides, int wtype, TimeValue t)
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

int HoseObject::RenderBegin(TimeValue t, ULONG flags)
{	SetAFlag(A_RENDER);
	int renderme;
	pblock->GetValue(PB_RENDERABLE,t,renderme,FOREVER);
	if (!renderme) 
	{	MeshInvalid();
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}
	return 0;
}

int HoseObject::RenderEnd(TimeValue t)
{	ClearAFlag(A_RENDER);
	int renderme;
 	pblock->GetValue(PB_RENDERABLE,t,renderme,FOREVER);
	if (!renderme) 
	{	MeshInvalid();
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}
	return 0;
}

#define SMOOTHALL	0
#define SMOOTHNONE	1
#define SMOOTHSIDES	2
#define SMOOTHSEGS	3

BOOL HoseObject::HasUVW() 
{ 	BOOL genUVs;
	Interval v;
	pblock->GetValue(PB_MAPMEMAPME, 0, genUVs, v);
	return genUVs; 
}

void HoseObject::SetGenUVW(BOOL sw) 
{  	if (sw==HasUVW()) 
		return;
	pblock->SetValue(PB_MAPMEMAPME,0, sw);
}

void HoseObject::BuildMesh(TimeValue t)
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
	
	ivalid=FOREVER;
	int createfree;
	pblock->GetValue(PB_ENDSMETHOD,0,createfree,ivalid);
	FixHoseFillet(pblock,t,(pmapParam?pmapParam->GetHWnd():NULL),increate);

	if ((!createfree)&&((!custnode)||(!custnode2))) 
		createfree = 1;
	if (!createfree) 
		ivalid.SetInstant(t);

	Matrix3 mat1,mat2;
	srand(56576);
	int s1 = rand();
	float Lf = 0.0f;
	Matrix3 Tlocal;

	Point3 startvec, endvec, tweenvec, startpoint, endpoint,
		   starty, endy, roty;

	float yangle = 0.0f;

	Spline3D *hosespline=NULL;

	Point3 RV = Zero;

	if (createfree) 
		pblock->GetValue(PB_NOREFLENGTH,t,Lf,ivalid);
	else
	{	
		// Martell 4/14/01: Fix for order of ops bug.
		float ztmp=RND11(); float ytmp=RND11(); float xtmp=RND11();
		RV = Point3(xtmp,ytmp,ztmp);

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

		Matrix3 mat1NT, mat2NT;

		mat1NT = mat1;
		mat2NT = mat2;
		mat1NT.NoTrans();
		mat2NT.NoTrans();

		Point3 P1 = mat1*mato1.GetRow(3),
			   P2 = mat2*mato2.GetRow(3);

		startvec = mat1NT*mato1.GetRow(2);
		endvec = mat2NT*mato2.GetRow(2);
		starty = mat1NT*mato1.GetRow(1);
		endy = mat2NT*mato2.GetRow(1);

		Matrix3 SI = Inverse(S);

		Point3 P0 = SI*P1;
		Matrix3 T1 = mat1;
		T1.NoTrans();
		Point3 RVw = T1 * RV;
		Lf = Length(P2 - P1);
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

// move z-axes of end transforms into local frame
		Matrix3 TlocalInvNT = Tlocal;
		TlocalInvNT.NoTrans();
		TlocalInvNT = Inverse(TlocalInvNT);

		float tenstop, tensbot;
		pblock->GetValue(PB_TENSION1,t,tenstop,ivalid);
		pblock->GetValue(PB_TENSION2,t,tensbot,ivalid);

		startvec = tensbot*(TlocalInvNT*startvec);
		endvec = tenstop*(TlocalInvNT*endvec);

		starty = TlocalInvNT*starty;
		endy = TlocalInvNT*endy;

		yangle = (float)acos(DotProd(starty,endy));

		if (yangle > EPSILON)
			roty = Normalize(starty^endy);
		else
			roty = Zero;

		startpoint = Point3(0.0f, 0.0f, 0.0f);
		endpoint = Point3(0.0f, 0.0f, Lf);

		hosespline = new Spline3D(KTYPE_CORNER, KTYPE_BEZIER, PARM_UNIFORM);
		int test1 = hosespline->AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_CURVE,startpoint,startpoint-startvec,startpoint+startvec),-1);
		int test2 = hosespline->AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_CURVE,endpoint,endpoint+endvec,endpoint-endvec),-1);
		hosespline->ComputeBezPoints();
	}

	int wtype;
	pblock->GetValue(PB_WIRETYPE,0,wtype,ivalid);
	int NvertsPerRing=0,
		nfillets=0,
		nsides=0;
	if (wtype==ROUND)
	{	pblock->GetValue(PB_RNDSIDES,t,NvertsPerRing,ivalid);
		if (NvertsPerRing<3) NvertsPerRing = 3;
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
	int SMOOTH;
	pblock->GetValue(PB_SMOOTH,0,SMOOTH,FOREVER);
	pblock->GetValue(PB_SEGMENTS,t,Segs,ivalid);
	if (Segs<3) Segs = 3;

	Nverts = (Segs + 1)*NvertsPerRing + 2;
	NfacesPerEnd = NvertsPerRing;
	NfacesPerRing = 2* NvertsPerRing;
	Nfaces =  Segs*NfacesPerRing + 2*NfacesPerEnd;
	Point3 *SaveVertex = new Point3[NvertsPerRing];
	assert(SaveVertex);
	MakeSaveVertex(SaveVertex, NvertsPerRing, nfillets, nsides, wtype, t);

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

	int thisvert = 0,
		last = Nverts-1,
		last2 = last-1;
	int lastvpr = NvertsPerRing-1,
		maxseg = Segs+1;

	int flexon, flexn;
	pblock->GetValue(PB_FLEXON,t,flexon,ivalid);
	float flex1, flex2, flexd, flexhere, flexlen, dadjust;
	pblock->GetValue(PB_FLEXSTART,t,flex1,ivalid);
	pblock->GetValue(PB_FLEXSTOP,t,flex2,ivalid);
	pblock->GetValue(PB_FLEXCYCLES,t,flexn,ivalid);
	pblock->GetValue(PB_FLEXDIAMETER,t,flexd,ivalid);

	Point3 ThisPosition;
	Point3 ThisXAxis, ThisYAxis, ThisZAxis;

	for (int i=0; i < maxseg; i++)
	{	float incr = (float)i/(float)Segs;

		if (createfree)
		{	ThisPosition = Point3(0.0f, 0.0f, Lf*incr);
			ThisXAxis = Point3(1.0f,0.0f,0.0f);
			ThisYAxis = Point3(0.0f,1.0f,0.0f);
			ThisZAxis = Point3(0.0f,0.0f,1.0f);
		}
		else
		{	ThisPosition = hosespline->InterpCurve3D(incr);
			ThisZAxis = hosespline->TangentCurve3D(incr);
//			ThisXAxis = Normalize(RV^ThisZAxis);

// new stuff here			
			ThisYAxis = starty;
			if (yangle > EPSILON)
				RotateOnePoint(ThisYAxis,Point3(0.0f,0.0f,0.0f),roty,incr*yangle);
			ThisXAxis = Normalize(ThisYAxis^ThisZAxis);
// end new stuff
			
			ThisYAxis = Normalize(ThisZAxis^ThisXAxis);
		}

		Matrix3 RingTM;
		RingTM.SetRow(0,ThisXAxis);
		RingTM.SetRow(1,ThisYAxis);
		RingTM.SetRow(2,ThisZAxis);
		RingTM.SetRow(3,ThisPosition);

		if (!createfree) 
			RingTM = RingTM * Tlocal;

		if ((incr>flex1)&&(incr<flex2)&&(flexon))
		{	flexlen = flex2 - flex1;
			if (flexlen<0.01f)
				flexlen = 0.01f;
			flexhere = (incr - flex1)/flexlen;
			dadjust = 1.0f+flexd*(1.0f-(float)sin((float)flexn*flexhere*TWOPI+PIover2));
		}
		else
			dadjust = 0.0f;

		for (int j=0; j < NvertsPerRing; j++)
		{	if (mapmenow)
				mesh.tVerts[thisvert] = Point3(0.999999f*incr, (float)j/(float)NvertsPerRing,0.5f);
			if (dadjust != 0.0f)
				mesh.setVert(thisvert, RingTM*(dadjust*SaveVertex[j]));
			else
				mesh.setVert(thisvert, RingTM*SaveVertex[j]);
			thisvert++;
		}

		if (mapmenow)
			mesh.tVerts[Nverts+i] = Point3(0.999999f*incr,0.999f,0.0f);

		if (i == 0)
		{	mesh.setVert(last2,(createfree?ThisPosition:Tlocal*ThisPosition));
			if (mapmenow)
				mesh.tVerts[last2]=Zero;
		}
		else if (i == Segs)
		{	mesh.setVert(last,(createfree?ThisPosition:Tlocal*ThisPosition));
			if (mapmenow)
				mesh.tVerts[last]=Ones;
		}
	}

//	Now, set up the faces
	int thisface = 0, v1, v2, v3, v4, v5, v6;
	v3 = last2;
	for (i=0; i < NvertsPerRing; i++)
	{	v1 = i;
		v2 = (i < lastvpr? v1+1 : v1-lastvpr);
		v5 = (i < lastvpr? v2 : Nverts);
		mesh.faces[thisface].setVerts(v2,v1,v3);
		mesh.faces[thisface].setSmGroup((SMOOTH == 1)?0:1);
		mesh.faces[thisface].setEdgeVisFlags(1,0,0);
		mesh.faces[thisface].setMatID(0);
		if (mapmenow)
			mesh.tvFace[thisface].setTVerts(v5,v1,v3);
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
			mesh.faces[thisface].setVerts(v4,v3,v1);
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
				mesh.tvFace[thisface].setTVerts(v6,v3,v1);
			thisface++;
			mesh.faces[thisface].setVerts(v1,v2,v4);
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
				mesh.tvFace[thisface].setTVerts(v1,v5,v6);
			thisface++;
		}

	int basevert = Segs*NvertsPerRing;

	v3 = Nverts-1;
	for (i = 0; i < NvertsPerRing; i++)
	{	v1 = i + basevert;
		v2 = (i < lastvpr? v1+1 : v1 - lastvpr);
		v5 = (i < lastvpr? v2 : Nverts+Segs);
		mesh.faces[thisface].setVerts(v1,v2,v3);
		mesh.faces[thisface].setSmGroup((SMOOTH == 1)?0:1);
		mesh.faces[thisface].setEdgeVisFlags(1,0,0);
		mesh.faces[thisface].setMatID(2);;
		if (mapmenow)
			mesh.tvFace[thisface].setTVerts(v1,v5,v3);
		thisface++;
	}

	if (SaveVertex) 
		delete[] SaveVertex;

	if (hosespline)
	{	delete hosespline;
	}
	
	mesh.InvalidateTopologyCache();
	srand( (unsigned)time( NULL ) );
}

class HoseEmitterCreateCallback : public CreateMouseCallBack {
	public:
		HoseObject *ob;
		Point3 p[2];
		IPoint2 sp0, sp1;
		BOOL square;
		void Cleanup();
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
	};

int HoseEmitterCreateCallback::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat)
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
				ob->pblock->SetValue(PB_RNDDIA,0,0.01f);
				ob->pblock->SetValue(PB_RECTWIDTH,0,0.01f);
				ob->pblock->SetValue(PB_RECTDEPTH,0,0.01f);
				ob->pblock->SetValue(PB_DSECWIDTH,0,0.01f);
				ob->pblock->SetValue(PB_DSECDEPTH,0,0.01f);
				ob->pblock->SetValue(PB_NOREFLENGTH,0,0.01f);
				ob->increate=TRUE;
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
				
				float tempsize = 2.0f*r;
				ob->pblock->SetValue(PB_RNDDIA,0,tempsize);
				ob->pblock->SetValue(PB_RECTWIDTH,0,tempsize);
				ob->pblock->SetValue(PB_RECTDEPTH,0,tempsize);
				ob->pblock->SetValue(PB_DSECWIDTH,0,tempsize);
				ob->pblock->SetValue(PB_DSECDEPTH,0,tempsize);
				ob->pmapParam->Invalidate();

				if (flags&MOUSE_CTRL)
				{	float ang = (float)atan2(p[1].y-p[0].y,p[1].x-p[0].x);
					mat.PreRotateZ(ob->ip->SnapAngle(ang));
				}

				if (msg==MOUSE_POINT) 
				{if (Length(m-sp0)<3 ||	Length(p[1]-p[0])<0.1f)
					{	ob->increate=FALSE;
						return CREATE_ABORT;	}
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
				ob->pblock->SetValue(PB_NOREFLENGTH,0,h);
				ob->pmapParam->Invalidate();
				if (msg==MOUSE_POINT)
				{ ob->suspendSnap = FALSE;
				if (Length(m-sp0)<3 ) {ob->increate=FALSE;return CREATE_ABORT;	}
				  else { TurnButton(ob->hParams,IDC_SPRING_PICKOBJECT1,TRUE);
						TurnButton(ob->hParams,IDC_SPRING_PICKOBJECT2,TRUE);
						return CREATE_STOP;
						SpinnerOn(ob->hParams,IDC_HOSE_TENSION1SPIN);
						SpinnerOn(ob->hParams,IDC_HOSE_TENSION2SPIN);
						}
				}
				}
				break;
			}
	}
	else
	if (msg == MOUSE_ABORT) 
	{ ob->increate=FALSE;return CREATE_ABORT;}

	return TRUE;
}

static HoseEmitterCreateCallback emitterCallback;

CreateMouseCallBack* HoseObject::GetCreateMouseCallBack() 
	{
	emitterCallback.ob = this;
	return &emitterCallback;
	}

void HoseObject::InvalidateUI()
	{
	if (pmapParam) pmapParam->Invalidate();
	}

ParamDimension *HoseObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_NOREFLENGTH:
		case PB_RNDDIA:
		case PB_RECTWIDTH:
		case PB_RECTDEPTH:
		case PB_RECTFILLET:
		case PB_DSECWIDTH:
		case PB_DSECDEPTH:
		case PB_DSECFILLET:
					return stdWorldDim;
		case PB_RECTROTANGLE:
		case PB_DSECROTANGLE:
					return stdAngleDim;
		case PB_FLEXSTART:
		case PB_FLEXSTOP:
		case PB_FLEXDIAMETER:
					return stdPercentDim;
		default:	
					return defaultDim;
		}
	}

TSTR HoseObject::GetParameterName(int pbIndex)
	{
	switch (pbIndex) {
		case PB_ENDSMETHOD:			return GetString(IDS_AP_ENDSMETHOD);break;
		case PB_NOREFLENGTH:		return GetString(IDS_EP_HOSEHEIGHT);break;
		case PB_SEGMENTS:			return GetString(IDS_EP_SEGMENTS);break;
		case PB_SMOOTH:				return GetString(IDS_AP_SMOOTH);break;
		case PB_RENDERABLE:			return GetString(IDS_EP_RENDERHOSE);break;

		case PB_WIRETYPE:			return GetString(IDS_AP_HOSETYPE);break;
		case PB_RNDDIA:				return GetString(IDS_AP_HOSEDIA);break;
		case PB_RNDSIDES:			return GetString(IDS_AP_HOSESIDES);break;

		case PB_RECTWIDTH:			return GetString(IDS_AP_HOSERECTWIDTH);break;
		case PB_RECTDEPTH:			return GetString(IDS_AP_HOSERECTDEPTH);break;
		case PB_RECTFILLET:			return GetString(IDS_AP_HOSERFILLET);break;
		case PB_RECTFILLETSIDES:	return GetString(IDS_AP_HOSERFILLETSIDES);break;
		case PB_RECTROTANGLE:		return GetString(IDS_AP_HOSERECTROT);break;

		case PB_DSECWIDTH:			return GetString(IDS_AP_HOSEDSECWIDTH);break;
		case PB_DSECDEPTH:			return GetString(IDS_AP_HOSEDSECDEPTH);break;
		case PB_DSECFILLET:			return GetString(IDS_AP_HOSEDFILLET);break;
		case PB_DSECFILLETSIDES:	return GetString(IDS_AP_HOSEDFILLETSIDES);break;
		case PB_DSECRNDSIDES:		return GetString(IDS_AP_HOSEDSECRNDSIDES);break;
		case PB_DSECROTANGLE:		return GetString(IDS_AP_HOSEDSECROT);break;

		case PB_MAPMEMAPME:			return GetString(IDS_AP_HOSEMAPME);break;

		case PB_FLEXON:				return GetString(IDS_AP_FLEXON);break;
		case PB_FLEXSTART:			return GetString(IDS_AP_FLEXSTART);break;
		case PB_FLEXSTOP:			return GetString(IDS_AP_FLEXSTOP);break;
		case PB_FLEXCYCLES:			return GetString(IDS_AP_FLEXCYCLES);break;
		case PB_FLEXDIAMETER:		return GetString(IDS_AP_FLEXDIAMETER);break;

		case PB_TENSION1:			return GetString(IDS_AP_TENSION1);break;
		case PB_TENSION2:			return GetString(IDS_AP_TENSION2);break;

		default: 					return TSTR(_T(""));
		}
	}

//--- Hose particle -----------------------------------------------

RefTargetHandle HoseObject::Clone(RemapDir& remap) 
	{
	HoseObject* newob = new HoseObject();	
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



RefTargetHandle HoseObject::GetReference(int i)
{	switch(i) {
		case PBLK: return(RefTargetHandle)pblock;
		case CUSTNODE: return (RefTargetHandle)custnode;
		case CUSTNODE2: return (RefTargetHandle)custnode2;
		default: return NULL;
		}
	}

void HoseObject::SetReference(int i, RefTargetHandle rtarg) { 
	switch(i) {
		case PBLK: pblock=(IParamBlock*)rtarg; return;
		case CUSTNODE: custnode = (INode *)rtarg; return;
		case CUSTNODE2: custnode2 = (INode *)rtarg; return;
		}
	}

RefResult HoseObject::NotifyRefChanged( 
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

/*class HosePostLoadCallback : public PostLoadCallback {
	public:
		ParamBlockPLCB *cb;
		HosePostLoadCallback(ParamBlockPLCB *c) {cb=c;}
		void proc(ILoad *iload) {
			DWORD oldVer = ((HoseObject*)(cb->targ))->pblock->GetVersion();
			ReferenceTarget *targ = cb->targ;
			cb->proc(iload);
			if (oldVer<1) {	
				((HoseObject*)targ)->pblock->SetValue(PB_SYMMETRY,0,0);
				}
			delete this;
			}
	};*/

#define COM_CUSTNAME_CHUNK	0x0100
#define COM_CUSTNAME2_CHUNK	0x0101
#define COM_CNODE_CHUNK		0x0102

IOResult HoseObject::Save(ISave *isave)
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

IOResult HoseObject::Load(ILoad *iload)
	{
	ULONG nb;
	IOResult res = IO_OK;
	int cnmtl=0;
	int refid;
	
//	iload->RegisterPostLoadCallback(
			//new HosePostLoadCallback(
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

const TCHAR *HoseClassDesc::ClassName ()	{return GetString(IDS_AP_HOSE);}
const TCHAR *HoseClassDesc::Category ()	{return GetString(IDS_RB_EXTENDED);}
TCHAR *HoseObject::GetObjectName() {return GetString(IDS_AP_HOSE_OBJECT);}
