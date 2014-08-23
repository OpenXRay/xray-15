/**********************************************************************
 *<
	FILE: bones.cpp

	DESCRIPTION:  Bone implementation

	HISTORY: created November 11 1994

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "prim.h"
#include <props.h>
#include "ikctrl.h"
#include "decomp.h"
#include "Simpobj.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "IIKSys.h"
#include "macrorec.h"
#include "surf_api.h"
#include "interpik.h" 

#include "modstack.h" // needed for CreateDerivedObject
#include "splshape.h" // needed for SplineShape
#include "istdplug.h"
#include "IAssembly.h"

#ifndef NO_OBJECT_BONE

//classID components for NormalizeSpline;

#define PW_PATCH_TO_SPLINE1 0x1c450e5c
#define PW_PATCH_TO_SPLINE2 0x2e0e0902

// Undefing this returns to the old behavior where the last two bones are deleted.
// When this is defined, the last bone is still deleted but the next to last bone
// is turned into a nub.
#define TURN_LAST_BONE_INTO_NUB

//--- New Bone Object ------------------------------------------------------------------

#define A_HIDE_BONE_OBJECT	(A_PLUGIN1)

class BoneObj : public SimpleObject2 {
	public:
		static IObjParam *ip;

		BoneObj(BOOL loading=FALSE) {GetNewBonesDesc()->MakeAutoParamBlocks(this);}
		
		// Animatable methods		
		void DeleteThis() {delete this;}
		Class_ID ClassID() {return BONE_OBJ_CLASSID;}		
		void BeginEditParams(IObjParam  *ip, ULONG flags, Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next);		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		
		// ReferenceMaker methods:
		void RescaleWorldUnits(float);

		// From Object
#if 0
		int CanConvertToType(Class_ID obtype);
		Object* ConvertToType(TimeValue t, Class_ID obtype);
		void GetCollapseTypes(Tab<Class_ID> &clist, Tab<TSTR*> &nlist);
#endif
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}		
		TCHAR *GetObjectName() {return GetString(IDS_BONE_OBJ_NAME);}
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);
		int	NumParamBlocks() {return 1;}
		IParamBlock2* GetParamBlock(int i) {return pblock2;}
		IParamBlock2* GetParamBlockByID(BlockID id) {return (pblock2->ID() == id) ? pblock2 : NULL;}
		
		// From SimpleObject
		void BuildMesh(TimeValue t);
		BOOL OKtoDisplay(TimeValue t);
		void InvalidateUI();

		void BuildFin(
			float size, float startTaper, float endTaper, 
			DWORD i0, DWORD i1, DWORD i2, DWORD i3, DWORD &curVert, DWORD &curFace);		
	};


IObjParam *BoneObj::ip = NULL;

#define PBLOCK_REF_NO	 0

// block IDs
enum { boneobj_params, };

// boneobj_params IDs
enum { 
	boneobj_width, boneobj_height, boneobj_taper, boneobj_length,
	boneobj_sidefins, boneobj_sidefins_size, boneobj_sidefins_starttaper, boneobj_sidefins_endtaper,
	boneobj_frontfin, boneobj_frontfin_size, boneobj_frontfin_starttaper, boneobj_frontfin_endtaper,
	boneobj_backfin,  boneobj_backfin_size,  boneobj_backfin_starttaper,  boneobj_backfin_endtaper,
	boneobj_genmap };



//------------------------------------------------------

// New bone descriptor. It's not public because the old descriptor handles the creation.
class NewBoneClassDesc: public ClassDesc2 {
	public:
	int 			IsPublic() {return 0;}
	void *			Create(BOOL loading = FALSE) { return new BoneObj(loading); }
	const TCHAR *	ClassName() { return GetString(IDS_BONE_CLASS_NAME); }
	int 			BeginCreate(Interface *i) {return 0;}
	int 			EndCreate(Interface *i) {return 0;}
	SClass_ID		SuperClassID() { return GEOMOBJECT_CLASS_ID; }
	Class_ID		ClassID() { return BONE_OBJ_CLASSID;}
	const TCHAR* 	Category() { return _T("");}
	const TCHAR*	InternalName() {return _T("BoneObj");}	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() {return hInstance;}			// returns owning module handle
	};


// RB 5/12/2000: Old bone descriptor is used just to handle creation.
// The old bone descriptor doesn't actually have the ability to 
// generate an object. However it lets the bone object appear in the
// system section even though it is a geom object.

class BoneClassDesc: public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return 0; }
	const TCHAR *	ClassName() { return GetString(IDS_DB_BONES_CLASS); }
	int 			BeginCreate(Interface *i);
	int 			EndCreate(Interface *i);
	SClass_ID		SuperClassID() { return SYSTEM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(0x001,0); }
	const TCHAR* 	Category() { return _T("");  }
	void			ResetClassParams(BOOL fileReset);
	};

static BoneClassDesc boneDesc;
static NewBoneClassDesc newBoneDesc;

ClassDesc* GetBonesDesc() { return &boneDesc; }
ClassDesc* GetNewBonesDesc() { return &newBoneDesc; }

class BonesCreationManager : public MouseCallBack, ReferenceMaker {
	friend static INT_PTR CALLBACK BoneParamDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	friend class PickAutoBone;
	friend class BoneClassDesc;
	private:
		CreateMouseCallBack *createCB;		
		INode *curNode, *lastNode, *firstChainNode;
		IObjCreate *createInterface;
		ClassDesc *cDesc;
		Matrix3 mat;  // the nodes TM relative to the CP
		IPoint2 lastpt;
		int ignoreSelectionChange;
		BOOL assignIK, assignIKRoot, assignEE; /*, autoLink, copyJP, matchAlign*/
		WNDPROC suspendProc;
		HWND hWnd;
		Object *editBoneObj;
		Tab<INode*> newBoneChain;
		int lastSolverSel;

		int NumRefs() { return 1; }
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		// StdNotifyRefChanged calls this, which can change the partID to new value 
		// If it doesnt depend on the particular message& partID, it should return
		// REF_DONTCARE
	    RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	    	PartID& partID,  RefMessage message);

	public:
		void CreateSpline(int ctKnot, INode* ikNode, int curveType);
		int BoneCount(INode* stJt, INode* endJt);
		void Begin( IObjCreate *ioc, ClassDesc *desc );
		void End();
		
		/*
		void AutoBone(INode *cnode,INode *pnode);
		void DoAutoBone(INode *node);		
		void AssignIKControllers(INode *cnode,INode *pnode,BOOL newParent,Matrix3 constTM);
		*/
		void MakeEndEffector();
		BOOL CreateIKChain(INode* start, INode* end, BOOL makeEE);

		void SetButtonStates(HWND hWnd);
		Object *CreateNewBoneObject();		
		void SetupObjectTM(INode *node);
		void UpdateBoneLength(INode *parNode);
		void UpdateBoneCreationParams();
		void BuildSolverNameList(Tab<TSTR*> &solverNames);
		BOOL HDSolverSelected();

		BonesCreationManager();
		virtual ~BonesCreationManager(); // mike.ts 2002.09.12

		int proc( HWND hwnd, int msg, int point, int flag, IPoint2 m );
	};


//--- Publish bone creation -------------------------------------------------------------------

#define BONE_FP_INTERFACE_ID Interface_ID(0x438aff72, 0xef9675ac)

class BoneFunctionPublish : public FPStaticInterface {
	public:
		DECLARE_DESCRIPTOR(BoneFunctionPublish);

		enum OpID {
			kBoneSegmentCreate,
			kBoneRefine
			};
		
		BEGIN_FUNCTION_MAP
			FN_3(kBoneSegmentCreate, TYPE_INODE, CreateBoneSegment, TYPE_POINT3, TYPE_POINT3, TYPE_POINT3)
			FN_0(kBoneRefine, TYPE_BOOL, RefineBone)
		END_FUNCTION_MAP

		INode *CreateBoneSegment(Point3 startPos, Point3 endPos, Point3 zAxis);
		BOOL RefineBone();
	};
static BoneFunctionPublish theBoneFunctionPublish(
	BONE_FP_INTERFACE_ID, _T("BoneSys"), -1/*IDS_OPS*/, 0, FP_CORE,
	// The first operation, boneCreate:
	BoneFunctionPublish::kBoneSegmentCreate, _T("createBone"), -1/*IDS_OP_CREATE*/, TYPE_INODE, 0, 3,
		// Argument 1, 2, 3:
		_T("startPos"), -1/*IDS_OP_CREATE_A1*/, TYPE_POINT3,
		_T("endPos"), -1/*IDS_OP_CREATE_A2*/, TYPE_POINT3,
		_T("zAxis"), -1, TYPE_POINT3,

		// The second operation, boneRefine:
	BoneFunctionPublish::kBoneRefine, _T("refineBone"), -1/*IDS_OP_CREATE*/, TYPE_BOOL, 0, 0,
	end);



INode *BoneFunctionPublish::CreateBoneSegment(
		Point3 startPos, Point3 endPos, Point3 zAxis)
	{
	TimeValue t = GetCOREInterface()->GetTime();

	// Figure out length and build node TM	
	Point3 xAxis  = endPos-startPos;
	float length = Length(xAxis);
	if (fabs(length) > 0.0f) {
		xAxis = xAxis/length;
	} else {
		xAxis = Point3(1,0,0);
		}
	Point3 yAxis = Normalize(CrossProd(zAxis, xAxis));
	zAxis = Normalize(CrossProd(xAxis,yAxis));
	Matrix3 tm(xAxis, yAxis, zAxis, startPos);

	SuspendAnimate();
	SuspendSetKeyMode();
	AnimateOff();

	// Make the new object and set its length
	BoneObj *newObj = new BoneObj(FALSE);	
	newObj->pblock2->SetValue(boneobj_length, t, length);

	// Make the node for the object
	INode *newNode = GetCOREInterface()->CreateObjectNode(newObj);
	Point3 boneColor = GetUIColor(COLOR_BONES);
	
	// Set node params
	newNode->SetWireColor(RGB(int(boneColor.x*255.0f), int(boneColor.y*255.0f), int(boneColor.z*255.0f) ));
	newNode->SetNodeTM(t, tm);
	newNode->SetBoneNodeOnOff(TRUE, t);
	newNode->SetRenderable(FALSE);
	//newNode->ShowBone(1);
	
	ResumeSetKeyMode();
	ResumeAnimate();

	return newNode;
	}



//--- Publish HD IK assignement ---------------------------------------------------------------

#define HDIK_FP_INTERFACE_ID Interface_ID(0x928ef7a2, 0xd51ff97a)

class HDIKCreateChain : public FPStaticInterface {
	public:
		BOOL inBonesCreation;		

		DECLARE_DESCRIPTOR(HDIKCreateChain);

		enum OpID {
			kIKChainCreate, kIKChainRemove
			};

		BEGIN_FUNCTION_MAP
			FN_3(kIKChainCreate, TYPE_BOOL, CreateIKChain, TYPE_INODE, TYPE_INODE, TYPE_BOOL)
			FN_1(kIKChainRemove, TYPE_BOOL, RemoveIKChain, TYPE_INODE)
		END_FUNCTION_MAP

		BOOL CreateIKChain(INode* start, INode* end, BOOL makeEE);
		BOOL RemoveIKChain(INode* chainNode);
	};


#ifndef NO_HD_IK // removing HDIKSys from FPS, owen brand
static HDIKCreateChain theHDIKCreateChain(
	HDIK_FP_INTERFACE_ID, _T("HDIKSys"), -1/*IDS_OPS*/, 0, FP_CORE,
	// The first operation, ikChainCreate:
	HDIKCreateChain::kIKChainCreate, _T("ikChain"), -1/*IDS_OP_CREATE*/, TYPE_VOID/*TYPE_INODE*/, 0, 3,
		// Argument 1, 2, 3:
		_T("startJoint"), -1/*IDS_OP_CREATE_A1*/, TYPE_INODE,
		_T("endJoint"), -1/*IDS_OP_CREATE_A2*/, TYPE_INODE,		
		_T("createEndEffector"), -1, TYPE_BOOL,

	HDIKCreateChain::kIKChainRemove, _T("RemoveChain"), -1, TYPE_BOOL, 1, 1,
		// Argument 1
		_T("chainNode"), -1/*IDS_OP_CREATE_A1*/, TYPE_INODE,		
	end);
#else
static HDIKCreateChain theHDIKCreateChain;  // still need this static instance
#endif // NO_HD_IK

class InitHDIKCreateChain {
	public:
		InitHDIKCreateChain() {theHDIKCreateChain.inBonesCreation = FALSE;}
	};
static InitHDIKCreateChain theInitHDIKCreateChain;

BOOL HDIKCreateChain::RemoveIKChain(INode* chainNode)
	{
	TimeValue t = GetCOREInterface()->GetTime();

	Control *tmCont = chainNode->GetTMController();
	if (tmCont->ClassID()!=IKSLAVE_CLASSID) return FALSE;
	
	IKSlaveControl  *slave  = (IKSlaveControl*)tmCont;
	IKMasterControl *master = slave->GetMaster();
	if (!master) return FALSE;

	master->RemoveIKChainControllers(t);

	return TRUE;
	}

class AssignHDIKControllersRestore : public RestoreObj {
	public:
		BOOL parity;
		// Undo/redo will change the controller a node carries. However,
		// the motion panel or hierarchy panel may still hold a pointer
		// to it. To circumvent this problem, we temporarily put the
		// task mode to a neutral one, DISPLAY, and later restore the mode.
		// Note that there are two restore objects for each assignHDIK
		// action, one, with parity==0, before and the other,
		// with parith==1, after the action. Therefore, undo starts from
		// parity==1.
		static BOOL hide;
		static int  cptMode;
		AssignHDIKControllersRestore(BOOL p) {parity=p;}
		void Restore(int isUndo) {
			if (parity) {
				cptMode = GetCOREInterface()->GetCommandPanelTaskMode();
				hide = (cptMode == TASK_MODE_MOTION)
				  || (cptMode == TASK_MODE_HIERARCHY);
				if (hide) {
					GetCOREInterface()->SetCommandPanelTaskMode(TASK_MODE_DISPLAY);
					}
			} else {
				if (hide) {
					GetCOREInterface()->SetCommandPanelTaskMode(cptMode);
					hide = FALSE;
					}
				}
			}
		void Redo() {
			if (!parity) {
				cptMode = 
					GetCOREInterface()->GetCommandPanelTaskMode();
				hide = (cptMode == TASK_MODE_MOTION)
				  || (cptMode == TASK_MODE_HIERARCHY);
				if (hide) {
					GetCOREInterface()->SetCommandPanelTaskMode(TASK_MODE_DISPLAY);
					}					
			} else {
				if (hide) {
					GetCOREInterface()->SetCommandPanelTaskMode(cptMode);
					hide = FALSE;
					}
				}			
			}
		TSTR Description() { return TSTR(_T("AssignHDIKControllersRestore")); }
	};

BOOL AssignHDIKControllersRestore::hide = FALSE;
int AssignHDIKControllersRestore::cptMode = 0;

BOOL HDIKCreateChain::CreateIKChain(INode* startJoint, INode* endJoint, BOOL makeEE)
	{
	TimeValue t = GetCOREInterface()->GetTime();
	INode *node = endJoint;
	IKMasterControl *master = NULL;
	IKSlaveControl *slave;

	// First check to make sure everything is replaceable
	while (node) {

		Control *tmCont = node->GetTMController();
		if (!tmCont->IsReplaceable()) {

			// See if it's an HD IK Slave
			if (tmCont->ClassID()==IKSLAVE_CLASSID) {
				slave = (IKSlaveControl*)tmCont;
				
				// If we haven't already decided on a master, use this one.
				if (!master) {
					master = slave->GetMaster();
				} else {
					// We can't assign to a chain which overlaps two different other chains
					if (master != slave->GetMaster()) {
						return FALSE;
						}
					}
			
			} else {
				// It's another non-replaceable controller. Can't assign IK.
				return FALSE;
				}
			}
		
		if (node==startJoint) break;
		node = node->GetParentNode();
		}
	
	// Make sure startJoint is an ancestor of endJoint
	if (node!=startJoint) return FALSE;

	// Create a new master (if needed)
	if (!master) master = CreateIKMasterControl();	
	Matrix3 tm;
	AffineParts parts;
	Point3 angles;

	if (theHold.Holding()) {
		theHold.Put(new AssignHDIKControllersRestore(0));
		}

	SuspendSetKeyMode();

	// Now do the assignment
	node = endJoint;
	while (node) {
		
		// Create a new slave control
		slave = CreateIKSlaveControl(master, node);		
		
		// Get the relative TM and decompose it
		tm    = node->GetNodeTM(t) * Inverse(node->GetParentTM(t));
		decomp_affine(tm, &parts);
		QuatToEuler(parts.q, angles, EULERTYPE_XYZ);
		
		// Init the IK slave using the previous controller's values at the time
		// we're assigning the new chain
		slave->SetInitPos(parts.t);
		slave->SetInitRot(angles);
		
		// Don't turn on rotation DOFs for the end joint
		if (inBonesCreation) {
			// Just turn on Z rotation because that is the axis perpindicular to the
			// construction plane.			
		 	if (node != endJoint)
			  slave->SetDOF(5, TRUE);
			}
		else {
			// Get limit info from last controller
			InitJointData3 posData3, rotData3;
			Control *tmCont = node->GetTMController();
			BOOL newAPI = tmCont->GetIKJoints2(&posData3, &rotData3);
			if (!newAPI) tmCont->GetIKJoints(&posData3, &rotData3);
			if (node == endJoint) {
				rotData3.active[0] =
				  rotData3.active[1] =
				  rotData3.active[2] = FALSE;
				}
			if (newAPI) slave->InitIKJoints2(&posData3, &rotData3);
			else slave->InitIKJoints(&posData3, &rotData3);
			}

		// Plug-in the slave controller
		node->SetTMController(slave);

		// Turn on the bone so joint limits and end effectors are visible
		node->ShowBone(1);

		// Stop after we do the start joint
		if (node==startJoint) break;

		// Step up to next node
		node = node->GetParentNode();
		}

	if (makeEE) {
		// Assign a position end effector to the end joint
		Control *cont = endJoint->GetTMController();
		if (cont && cont->ClassID()==IKSLAVE_CLASSID) {
			slave  = (IKSlaveControl*)cont;
			slave->MakeEE(TRUE,(1<<0), endJoint->GetNodeTM(t).GetTrans(), Quat());
			}
		}

	if (theHold.Holding()) {
		theHold.Put(new AssignHDIKControllersRestore(1));
		}

	ResumeSetKeyMode();

	return TRUE;
	}

///////////////////////////////////////////////////////////////////////////////

#define CID_BONECREATE	CID_USER + 1

class BonesCreateMode : public CommandMode 
{		
	public:
		BonesCreationManager* proc;
		ParamBlockDesc2*      boneobj_param_blk;

		void Begin( IObjCreate *ioc, ClassDesc *desc ) { proc->Begin( ioc, desc ); }
		void End() { proc->End(); }
		int Class() { return CREATE_COMMAND; }
		int ID() { return CID_BONECREATE; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints = 1000000; return proc; }
		ChangeForegroundCallback *ChangeFGProc() { return CHANGE_FG_SELECTED; }
		BOOL ChangeFG( CommandMode *oldMode ) { return (oldMode->ChangeFGProc() != CHANGE_FG_SELECTED); }
		void EnterMode() {}
		void ExitMode() {}
		BOOL IsSticky() { return FALSE; }

		BonesCreateMode();
		virtual ~BonesCreateMode();
};

///////////////////////////////////////////////////////////////////////////////

BonesCreateMode::~BonesCreateMode()
{
	if ( proc )
	{
		delete proc;
		proc = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////

BonesCreateMode::BonesCreateMode()
{
	proc = new BonesCreationManager();
	boneobj_param_blk = new ParamBlockDesc2 
	( 
		boneobj_params, _T("BoneObjectParameters"),  0, &newBoneDesc, P_AUTO_CONSTRUCT+P_AUTO_UI, PBLOCK_REF_NO,

		//rollout
		IDD_NEWBONE, IDS_RB_BONEPARAMS, 0, 0, NULL,

		// params
		boneobj_width, _T("width"), TYPE_WORLD, P_ANIMATABLE, IDS_BONE_WIDTH,
			p_default, 		10.0,	
			p_ms_default,	10.0,
			p_range, 		0.0f, float(1.0E30), 
			p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_BONE_WIDTH, IDC_BONE_WIDTHSPIN, SPIN_AUTOSCALE, 
			end, 

		boneobj_height, _T("height"), TYPE_WORLD, P_ANIMATABLE, IDS_BONE_HEIGHT,
			p_default, 		10.0,	
			p_ms_default,	10.0,
			p_range, 		0.0f, float(1.0E30), 
			p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_BONE_HEIGHT, IDC_BONE_HEIGHTSPIN, SPIN_AUTOSCALE, 
			end, 

		boneobj_taper, _T("taper"), TYPE_PCNT_FRAC, P_ANIMATABLE, IDS_BONE_TAPER,
			p_default, 		0.9,
			p_ms_default,	0.9,
			p_range, 		-float(1.0E30), 100.0, 
			p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_BONE_TAPER, IDC_BONE_TAPERSPIN, 0.1, 
			end, 

		boneobj_length, _T("length"), TYPE_FLOAT, P_RESET_DEFAULT, IDS_BONE_LENGTH,
			p_default, 		0.0,
			p_ms_default,	10.0,
			p_range, 		0.0, float(1.0E30),
			end, 

		// Side Fins
		boneobj_sidefins, 	_T("sidefins"), TYPE_BOOL, P_ANIMATABLE, IDS_BONE_SIDEFINS,
			p_default, 		FALSE, 
			p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_BONE_SIDEFINS, 
			end, 

		boneobj_sidefins_size, _T("sidefinssize"), TYPE_WORLD, P_ANIMATABLE, IDS_BONE_SF_SIZE,
			p_default, 		5.0,
			p_ms_default,	5.0,
			p_range, 		0.0, float(1.0E30),
			p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_BONE_SIDEFINS_SIZE, IDC_BONE_SIDEFINS_SIZESPIN, SPIN_AUTOSCALE, 
			end, 

		boneobj_sidefins_starttaper, _T("sidefinsstarttaper"), TYPE_PCNT_FRAC, P_ANIMATABLE, IDS_BONE_SF_STARTTAPER,
			p_default, 		0.1,
			p_ms_default,	0.1,
			p_range, 		-float(1.0E30), float(1.0E30),
			p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_BONE_SIDEFINS_START_TAPER, IDC_BONE_SIDEFINS_START_TAPERSPIN, 0.1, 
			end, 

		boneobj_sidefins_endtaper, _T("sidefinsendtaper"), TYPE_PCNT_FRAC, P_ANIMATABLE, IDS_BONE_SF_ENDTAPER,
			p_default, 		0.1,
			p_ms_default,	0.1,
			p_range, 		-float(1.0E30), float(1.0E30),
			p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_BONE_SIDEFINS_END_TAPER, IDC_BONE_SIDEFINS_END_TAPERSPIN, 0.1, 
			end, 


		// Front Fin
		boneobj_frontfin, 	_T("frontfin"), TYPE_BOOL, P_ANIMATABLE, IDS_BONE_FRONTFIN,
			p_default, 		FALSE, 
			p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_BONE_FRONTFIN, 
			end, 

		boneobj_frontfin_size, _T("frontfinsize"), TYPE_WORLD, P_ANIMATABLE, IDS_BONE_FF_SIZE,
			p_default, 		5.0,
			p_ms_default,	5.0,
			p_range, 		0.0, float(1.0E30),
			p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_BONE_FRONTFIN_SIZE, IDC_BONE_FRONTFIN_SIZESPIN, SPIN_AUTOSCALE, 
			end, 

		boneobj_frontfin_starttaper, _T("frontfinstarttaper"), TYPE_PCNT_FRAC, P_ANIMATABLE, IDS_BONE_FF_STARTTAPER,
			p_default, 		0.1,
			p_ms_default,	0.1,
			p_range, 		-float(1.0E30), float(1.0E30),
			p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_BONE_FRONTFIN_START_TAPER, IDC_BONE_FRONTFIN_START_TAPERSPIN, 0.1, 
			end, 

		boneobj_frontfin_endtaper, _T("frontfinendtaper"), TYPE_PCNT_FRAC, P_ANIMATABLE, IDS_BONE_FF_ENDTAPER,
			p_default, 		0.1,
			p_ms_default,	0.1,
			p_range, 		-float(1.0E30), float(1.0E30),
			p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_BONE_FRONTFIN_END_TAPER, IDC_BONE_FRONTFIN_END_TAPERSPIN, 0.1, 
			end, 


		// Back Fin
		boneobj_backfin, 	_T("backfin"), TYPE_BOOL, P_ANIMATABLE, IDS_BONE_BACKFIN,
			p_default, 		FALSE, 
			p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_BONE_BACKFIN, 
			end, 

		boneobj_backfin_size, _T("backfinsize"), TYPE_WORLD, P_ANIMATABLE, IDS_BONE_BF_SIZE,
			p_default, 		5.0,
			p_ms_default,	5.0,
			p_range, 		0.0, float(1.0E30),
			p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_BONE_BACKFIN_SIZE, IDC_BONE_BACKFIN_SIZESPIN, SPIN_AUTOSCALE, 
			end, 

		boneobj_backfin_starttaper, _T("backfinstarttaper"), TYPE_PCNT_FRAC, P_ANIMATABLE, IDS_BONE_BF_STARTTAPER,
			p_default, 		0.1,
			p_ms_default,	0.1,
			p_range, 		-float(1.0E30), float(1.0E30),
			p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_BONE_BACKFIN_START_TAPER, IDC_BONE_BACKFIN_START_TAPERSPIN, 0.1, 
			end, 

		boneobj_backfin_endtaper, _T("backfinendtaper"), TYPE_PCNT_FRAC, P_ANIMATABLE, IDS_BONE_BF_ENDTAPER,
			p_default, 		0.1,
			p_ms_default,	0.1,
			p_range, 		-float(1.0E30), float(1.0E30),
			p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_BONE_BACKFIN_END_TAPER, IDC_BONE_BACKFIN_END_TAPERSPIN, 0.1, 
			end, 

		// Generate mapping coords.
		boneobj_genmap, 	_T("genmap"), TYPE_BOOL, P_ANIMATABLE, IDS_BONE_GENMAP,
			p_default, 		FALSE, 
			p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_BONE_GENMAP, 
			end, 
			

		end
	);

}

///////////////////////////////////////////////////////////////////////////////

static BonesCreateMode theBonesCreateMode;

///////////////////////////////////////////////////////////////////////////////


void BoneClassDesc::ResetClassParams(BOOL fileReset)
	{
#ifndef NO_CREATE_IK_ASSIGN
	theBonesCreateMode.proc->assignIK     = FALSE;
	theBonesCreateMode.proc->assignIKRoot = TRUE;
#else // NO_CREATE_IK_ASSIGN
	theBonesCreateMode.proc->assignIK     = FALSE;
	theBonesCreateMode.proc->assignIKRoot = FALSE;
#endif // NO_CREATE_IK_ASSIGN

	theBonesCreateMode.proc->assignEE     = TRUE; 
	theBonesCreateMode.proc->lastSolverSel = -1;
#if 0
	theBonesCreateMode.proc->autoLink     = TRUE;
	theBonesCreateMode.proc->copyJP       = TRUE;
	theBonesCreateMode.proc->matchAlign   = TRUE;
#endif
	}


#if 0

class PickAutoBone : 
		public PickModeCallback,
		public PickNodeCallback {
	public:				
		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);
		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);
		BOOL Filter(INode *node) {return TRUE;}
		PickNodeCallback *GetFilter() {return this;}
	};
static PickAutoBone thePickAutoBone;

BOOL PickAutoBone::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
	{	
	INode *node = ip->PickNode(hWnd,m,this);
	return node ? TRUE : FALSE;
	}

BOOL PickAutoBone::Pick(IObjParam *ip,ViewExp *vpt)
	{
	INode *node = vpt->GetClosestHit();
	theBonesCreateMode.proc.DoAutoBone(node);
	ip->SetCommandMode(&theBonesCreateMode);
	ip->RedrawViews(ip->GetTime());
	return FALSE;
	}

void PickAutoBone::EnterMode(IObjParam *ip)
	{
	ICustButton *but = GetICustButton(GetDlgItem(
		theBonesCreateMode.proc.hWnd,IDC_BONES_AUTOBONE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
	}

void PickAutoBone::ExitMode(IObjParam *ip)
	{
	ICustButton *but = GetICustButton(GetDlgItem(
		theBonesCreateMode.proc.hWnd,IDC_BONES_AUTOBONE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
	}

#endif

// Following enums are copied from class IKCmdOpsImp in
// src\dll\iksys\IKSys.h. They should be promoted to IKCmdOps in
// src\maxsdk\include\IIKsys.h. In order not to touch the API, we
// copy them to here for now. Move to IIKSys.h once API is open.
// - J.Zhao, 3/21/01.
// 
enum OpID {
  IKCmdOps_kIKChainCreate,
  IKCmdOps_kSolverCount,
  IKCmdOps_kSolverName,
  IKCmdOps_kSolverUIName
};


static INT_PTR CALLBACK AssignSplineIKSolverDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){

	static Control *sikc = NULL; // static so that it remembers the controller when it enters for the first time
	static INode *sikc_node = NULL;
	ISpinnerControl* spin1, *spin2;
	TimeValue t = GetCOREInterface()->GetTime();
	int nKnots, splCreate, hlprCreate, linkRoot;
	int curveType, j;
	float hlprSize;
	IParamBlock2 *sikcPB2;
	BOOL checked;

	INode* stJoint=NULL;
	INode* endJoint=NULL;
	IIKChainControl* iikc;

	ICustEdit *iEditBox;
	TCHAR* cur_solver = NULL;
	TCHAR str[256];

	
	switch (msg) {
		case WM_INITDIALOG:
			sikc_node = (INode*)lParam;
			sikc = sikc_node->GetTMController();
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			sikcPB2 = sikc->GetParamBlockByID(IIKChainControl::kParamBlock);



			iikc = (IIKChainControl*)sikc->GetInterface(I_IKCHAINCONTROL);
			if (!iikc) return FALSE;
			stJoint = iikc->StartJoint();
			if(!(stJoint)) return FALSE;
			endJoint = iikc->EndJoint();
			if(!(endJoint)) return FALSE;
			
			//-------------------------------------------------------------------
			//Traverse the bone hierarchy and calculate the number of bones
			// this is equal to our initial knotcount of the spline
			INode* pparent;
			pparent =  endJoint->GetParentNode();
			j = 1;
			while(pparent){
				if (pparent != stJoint) {
					pparent = pparent->GetParentNode();
					j = j + 1;
				}
				else if (pparent == stJoint){
					j = j + 1;
					break;
				}
				else if (pparent->IsRootNode()) { //we have reached the rootnode without crossing stJt, something is wrong
					assert(1);
				}	
			}
			//-------------------------------------------------------------------

			//set nKnots equal to j, calculated above
			sikcPB2->SetValue(IIKChainControl::kSplineKnotCount, t, j);

			
//			EnableWindow(GetDlgItem(hWnd, IDC_BEZ_SPLINE_RAD),		TRUE); //supporting only Bezier Spline for now
//			EnableWindow(GetDlgItem(hWnd, IDC_NURBS_SPLINE_RAD),	FALSE);


			sikcPB2->GetValue(IIKChainControl::kSplineTypeChoice, t, curveType, FOREVER);
			CheckRadioButton(hWnd, IDC_BEZ_SPLINE_RAD, IDC_NURBS_CV_RAD, IDC_BEZ_SPLINE_RAD);
#ifdef NO_NURBS
			if (curveType != 0)
			{
				curveType = 0;
				sikcPB2->SetValue(IIKChainControl::kSplineTypeChoice, t, curveType);
			}
#endif

			if (curveType == 0) //it's Bezier Spline = DEFAULT 
				CheckRadioButton(hWnd, IDC_BEZ_SPLINE_RAD, IDC_NURBS_CV_RAD, IDC_BEZ_SPLINE_RAD);
			else if (curveType == 1)	// //it's  NURBS Point curve
				CheckRadioButton(hWnd, IDC_BEZ_SPLINE_RAD, IDC_NURBS_CV_RAD, IDC_NURBS_POINT_RAD);
			else if (curveType == 2)	// //it's  NURBS CV curve
				CheckRadioButton(hWnd, IDC_BEZ_SPLINE_RAD, IDC_NURBS_CV_RAD, IDC_NURBS_CV_RAD);
			
			
			sikcPB2->GetValue(IIKChainControl::kAutoSplineCreate, t, splCreate, FOREVER);
			SetCheckBox(hWnd, IDC_AUTO_SPL_CHCK, splCreate);

			EnableWindow(GetDlgItem(hWnd, IDC_BEZ_SPLINE_RAD),		splCreate);
#ifndef NO_NURBS
			EnableWindow(GetDlgItem(hWnd, IDC_NURBS_POINT_RAD),		splCreate);
			EnableWindow(GetDlgItem(hWnd, IDC_NURBS_CV_RAD),		splCreate);
#else
			EnableWindow(GetDlgItem(hWnd, IDC_NURBS_POINT_RAD),		FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_NURBS_CV_RAD),		FALSE);
#endif

			if (curveType == 0){
				EnableWindow(GetDlgItem(hWnd, IDC_SPLKNOT_NUM_SPIN),	splCreate);
				EnableWindow(GetDlgItem(hWnd, IDC_SPLKNOT_NUM_EDIT),	splCreate);
			}
			else if (curveType == 1 || curveType == 2){
				EnableWindow(GetDlgItem(hWnd, IDC_SPLKNOT_NUM_SPIN),	FALSE);
				EnableWindow(GetDlgItem(hWnd, IDC_SPLKNOT_NUM_EDIT),	FALSE);
			}

			EnableWindow(GetDlgItem(hWnd, IDC_CREATE_HLPER_CHCK),	splCreate);
			EnableWindow(GetDlgItem(hWnd, IDC_LINK_ROOT_CHCK),		splCreate);

			EnableWindow(GetDlgItem(hWnd, IDC_CENTRAL_MARKER_CHCK), splCreate);
			EnableWindow(GetDlgItem(hWnd, IDC_AXIS_TRIPOD_CHCK),	splCreate);
			EnableWindow(GetDlgItem(hWnd, IDC_CROSS_CHCK),			splCreate);
			EnableWindow(GetDlgItem(hWnd, IDC_BOX_CHCK),			splCreate);
			EnableWindow(GetDlgItem(hWnd, IDC_CONST_SC_SIZE_CHCK),	splCreate);
			EnableWindow(GetDlgItem(hWnd, IDC_DRAW_TOP_CHCK),		splCreate);
			EnableWindow(GetDlgItem(hWnd, IDC_HLPR_SIZE_SPIN),		splCreate);
			EnableWindow(GetDlgItem(hWnd, IDC_HLPR_SIZE_EDIT),		splCreate);

			sikcPB2->GetValue(IIKChainControl::kCreateHelper,t, hlprCreate, FOREVER);
			SetCheckBox(hWnd, IDC_CREATE_HLPER_CHCK, hlprCreate);

			// if hlprCreate = TRUE		all helper related parameters should be un-gray-ed
			// if hlprCreate = FALSE	all helper related parameters should be gray-ed
						
			EnableWindow(GetDlgItem(hWnd, IDC_CENTRAL_MARKER_CHCK), hlprCreate);
			EnableWindow(GetDlgItem(hWnd, IDC_AXIS_TRIPOD_CHCK),	hlprCreate);
			EnableWindow(GetDlgItem(hWnd, IDC_CROSS_CHCK),			hlprCreate);
			EnableWindow(GetDlgItem(hWnd, IDC_BOX_CHCK),			hlprCreate);
			EnableWindow(GetDlgItem(hWnd, IDC_CONST_SC_SIZE_CHCK),	hlprCreate);
			EnableWindow(GetDlgItem(hWnd, IDC_DRAW_TOP_CHCK),		hlprCreate);
			EnableWindow(GetDlgItem(hWnd, IDC_HLPR_SIZE_SPIN),		hlprCreate);
			EnableWindow(GetDlgItem(hWnd, IDC_HLPR_SIZE_EDIT),		hlprCreate);

			sikcPB2->GetValue(IIKChainControl::kLinktoRootNode,t, linkRoot, FOREVER);
			SetCheckBox(hWnd, IDC_LINK_ROOT_CHCK, linkRoot);

			sikcPB2->GetValue(IIKChainControl::kHelperCentermarker,t, checked, FOREVER);
			SetCheckBox(hWnd, IDC_CENTRAL_MARKER_CHCK, checked);

			sikcPB2->GetValue(IIKChainControl::kHelperAxisTripod,t, checked, FOREVER);
			SetCheckBox(hWnd, IDC_AXIS_TRIPOD_CHCK, checked);

			sikcPB2->GetValue(IIKChainControl::kHelperCross,t, checked, FOREVER);
			SetCheckBox(hWnd, IDC_CROSS_CHCK, checked);

			sikcPB2->GetValue(IIKChainControl::kHelperBox,t, checked, FOREVER);
			SetCheckBox(hWnd, IDC_BOX_CHCK, checked);

			sikcPB2->GetValue(IIKChainControl::kHelperScreensize,t, checked, FOREVER);
			SetCheckBox(hWnd, IDC_CONST_SC_SIZE_CHCK, checked);

			sikcPB2->GetValue(IIKChainControl::kHelperDrawontop,t, checked, FOREVER);
			SetCheckBox(hWnd, IDC_DRAW_TOP_CHCK, checked);
			

			sikcPB2->GetValue(IIKChainControl::kSplineKnotCount, t, nKnots, FOREVER);
			spin1 = SetupIntSpinner(hWnd, IDC_SPLKNOT_NUM_SPIN, IDC_SPLKNOT_NUM_EDIT, 2, INT_MAX, nKnots);

			sikcPB2->GetValue(IIKChainControl::kHelpersize, t, hlprSize, FOREVER);
			spin2 = SetupFloatSpinner(hWnd, IDC_HLPR_SIZE_SPIN, IDC_HLPR_SIZE_EDIT, 0, float(INT_MAX), hlprSize);


			cur_solver = sikc_node->GetName();
			iEditBox = GetICustEdit(GetDlgItem (hWnd, IDC_IKNAME_EDIT));
			if(iEditBox) iEditBox->SetText(cur_solver);
			iEditBox->WantReturn(TRUE);

//			ReleaseISpinner(spin1);

		break;

		case WM_CLOSE:
			EndDialog(hWnd, FALSE);
			return FALSE;
			break;

		case WM_COMMAND:

			switch (LOWORD(wParam)) {

				
				case IDC_IKNAME_EDIT:
					iEditBox = GetICustEdit(GetDlgItem (hWnd, IDC_IKNAME_EDIT));
					if(iEditBox) iEditBox->GetText(str, 256);
					sikc_node->SetName(str);
				break;
			
				case IDC_AUTO_SPL_CHCK:
					splCreate = GetCheckBox(hWnd,IDC_AUTO_SPL_CHCK);
					sikcPB2 = sikc->GetParamBlockByID(IIKChainControl::kParamBlock);
					sikcPB2->SetValue(IIKChainControl::kAutoSplineCreate, t, splCreate);
					
					EnableWindow(GetDlgItem(hWnd, IDC_BEZ_SPLINE_RAD),		splCreate);
#ifndef NO_NURBS
					EnableWindow(GetDlgItem(hWnd, IDC_NURBS_POINT_RAD),		splCreate);
					EnableWindow(GetDlgItem(hWnd, IDC_NURBS_CV_RAD),		splCreate);
#endif
					EnableWindow(GetDlgItem(hWnd, IDC_SPLKNOT_NUM_SPIN),	splCreate);
					EnableWindow(GetDlgItem(hWnd, IDC_SPLKNOT_NUM_EDIT),	splCreate);

					EnableWindow(GetDlgItem(hWnd, IDC_CREATE_HLPER_CHCK),	splCreate);
					hlprCreate = GetCheckBox(hWnd,IDC_CREATE_HLPER_CHCK);
					
					if (!splCreate){

						EnableWindow(GetDlgItem(hWnd, IDC_CREATE_HLPER_CHCK),	splCreate);
						EnableWindow(GetDlgItem(hWnd, IDC_LINK_ROOT_CHCK),		splCreate);

						EnableWindow(GetDlgItem(hWnd, IDC_CENTRAL_MARKER_CHCK), splCreate);
						EnableWindow(GetDlgItem(hWnd, IDC_AXIS_TRIPOD_CHCK),	splCreate);
						EnableWindow(GetDlgItem(hWnd, IDC_CROSS_CHCK),			splCreate);
						EnableWindow(GetDlgItem(hWnd, IDC_BOX_CHCK),			splCreate);
						EnableWindow(GetDlgItem(hWnd, IDC_CONST_SC_SIZE_CHCK),	splCreate);
						EnableWindow(GetDlgItem(hWnd, IDC_DRAW_TOP_CHCK),		splCreate);
						EnableWindow(GetDlgItem(hWnd, IDC_HLPR_SIZE_SPIN),		splCreate);
						EnableWindow(GetDlgItem(hWnd, IDC_HLPR_SIZE_EDIT),		splCreate);

					}
					else if(splCreate && hlprCreate){

						EnableWindow(GetDlgItem(hWnd, IDC_CREATE_HLPER_CHCK),	splCreate);
						EnableWindow(GetDlgItem(hWnd, IDC_LINK_ROOT_CHCK),		splCreate);

						EnableWindow(GetDlgItem(hWnd, IDC_CENTRAL_MARKER_CHCK), splCreate);
						EnableWindow(GetDlgItem(hWnd, IDC_AXIS_TRIPOD_CHCK),	splCreate);
						EnableWindow(GetDlgItem(hWnd, IDC_CROSS_CHCK),			splCreate);
						EnableWindow(GetDlgItem(hWnd, IDC_BOX_CHCK),			splCreate);
						EnableWindow(GetDlgItem(hWnd, IDC_CONST_SC_SIZE_CHCK),	splCreate);
						EnableWindow(GetDlgItem(hWnd, IDC_DRAW_TOP_CHCK),		splCreate);
						EnableWindow(GetDlgItem(hWnd, IDC_HLPR_SIZE_SPIN),		splCreate);
						EnableWindow(GetDlgItem(hWnd, IDC_HLPR_SIZE_EDIT),		splCreate);

					}
				break;
			
				case IDC_BEZ_SPLINE_RAD: // means it's a Bezier spline, kSplineTypeChoice = 0
// 					curveType = IsDlgButtonChecked(hWnd,IDC_BEZ_SPLINE_RAD);
					sikcPB2 = sikc->GetParamBlockByID(IIKChainControl::kParamBlock);
					sikcPB2->SetValue(IIKChainControl::kSplineTypeChoice, t, 0);
					EnableWindow(GetDlgItem(hWnd, IDC_SPLKNOT_NUM_SPIN),	TRUE);
					EnableWindow(GetDlgItem(hWnd, IDC_SPLKNOT_NUM_EDIT),	TRUE);
				break;


				case IDC_NURBS_POINT_RAD: // means it's a NURBS Point Curve, kSplineTypeChoice = 1
//					curveType = IsDlgButtonChecked(hWnd,IDC_NURBS_POINT_RAD);
					sikcPB2 = sikc->GetParamBlockByID(IIKChainControl::kParamBlock);
					sikcPB2->SetValue(IIKChainControl::kSplineTypeChoice, t, 1);
					EnableWindow(GetDlgItem(hWnd, IDC_SPLKNOT_NUM_SPIN),	FALSE);
					EnableWindow(GetDlgItem(hWnd, IDC_SPLKNOT_NUM_EDIT),	FALSE);
				break;

				case IDC_NURBS_CV_RAD: // means it's a NURBS CV Curve, kSplineTypeChoice = 2
//					curveType = IsDlgButtonChecked(hWnd,IDC_NURBS_CV_RAD);
					sikcPB2 = sikc->GetParamBlockByID(IIKChainControl::kParamBlock);
					sikcPB2->SetValue(IIKChainControl::kSplineTypeChoice, t, 2);
					EnableWindow(GetDlgItem(hWnd, IDC_SPLKNOT_NUM_SPIN),	FALSE);
					EnableWindow(GetDlgItem(hWnd, IDC_SPLKNOT_NUM_EDIT),	FALSE);
				break;

			
				case IDC_CREATE_HLPER_CHCK:
					hlprCreate = GetCheckBox(hWnd,IDC_CREATE_HLPER_CHCK);
					sikcPB2 = sikc->GetParamBlockByID(IIKChainControl::kParamBlock);
					sikcPB2->SetValue(IIKChainControl::kCreateHelper, t, hlprCreate);

					// if hlprCreate = TRUE		all helper related parameters should be un-gray-ed
					// if hlprCreate = FALSE	all helper related parameters should be gray-ed
						
					EnableWindow(GetDlgItem(hWnd, IDC_LINK_ROOT_CHCK),		hlprCreate);

					EnableWindow(GetDlgItem(hWnd, IDC_CENTRAL_MARKER_CHCK), hlprCreate);
					EnableWindow(GetDlgItem(hWnd, IDC_AXIS_TRIPOD_CHCK),	hlprCreate);
					EnableWindow(GetDlgItem(hWnd, IDC_CROSS_CHCK),			hlprCreate);
					EnableWindow(GetDlgItem(hWnd, IDC_BOX_CHCK),			hlprCreate);
					EnableWindow(GetDlgItem(hWnd, IDC_CONST_SC_SIZE_CHCK),	hlprCreate);
					EnableWindow(GetDlgItem(hWnd, IDC_DRAW_TOP_CHCK),		hlprCreate);
					EnableWindow(GetDlgItem(hWnd, IDC_HLPR_SIZE_SPIN),		hlprCreate);
					EnableWindow(GetDlgItem(hWnd, IDC_HLPR_SIZE_EDIT),		hlprCreate);
						
				break;

				case IDC_LINK_ROOT_CHCK:
					linkRoot = GetCheckBox(hWnd,IDC_LINK_ROOT_CHCK);
					sikcPB2 = sikc->GetParamBlockByID(IIKChainControl::kParamBlock);
					sikcPB2->SetValue(IIKChainControl::kLinktoRootNode, t, linkRoot);
				break;

				
				// below last sub-rollout
				case IDC_CENTRAL_MARKER_CHCK:		
					checked = GetCheckBox(hWnd,IDC_CENTRAL_MARKER_CHCK);
					sikcPB2 = sikc->GetParamBlockByID(IIKChainControl::kParamBlock);
					sikcPB2->SetValue(IIKChainControl::kHelperCentermarker, t, checked);
				break;

				case IDC_AXIS_TRIPOD_CHCK:
					checked = GetCheckBox(hWnd,IDC_AXIS_TRIPOD_CHCK);
					sikcPB2 = sikc->GetParamBlockByID(IIKChainControl::kParamBlock);
					sikcPB2->SetValue(IIKChainControl::kHelperAxisTripod, t, checked);
				break;

				case IDC_CROSS_CHCK:
					checked = GetCheckBox(hWnd,IDC_CROSS_CHCK);
					sikcPB2 = sikc->GetParamBlockByID(IIKChainControl::kParamBlock);
					sikcPB2->SetValue(IIKChainControl::kHelperCross, t, checked);
				break;

				case IDC_BOX_CHCK:
					checked = GetCheckBox(hWnd,IDC_BOX_CHCK);
					sikcPB2 = sikc->GetParamBlockByID(IIKChainControl::kParamBlock);
					sikcPB2->SetValue(IIKChainControl::kHelperBox, t, checked);
				break;

				case IDC_CONST_SC_SIZE_CHCK:
					checked = GetCheckBox(hWnd,IDC_CONST_SC_SIZE_CHCK);
					sikcPB2 = sikc->GetParamBlockByID(IIKChainControl::kParamBlock);
					sikcPB2->SetValue(IIKChainControl::kHelperScreensize, t, checked);
				break;

				case IDC_DRAW_TOP_CHCK:
					checked = GetCheckBox(hWnd,IDC_DRAW_TOP_CHCK);
					sikcPB2 = sikc->GetParamBlockByID(IIKChainControl::kParamBlock);
					sikcPB2->SetValue(IIKChainControl::kHelperDrawontop, t, checked);
				break;

				case ID_SPLIK_OK:
					EndDialog(hWnd,1);
				break;

			}
		break;

		case CC_SPINNER_CHANGE:
			switch (LOWORD(wParam)) {
				case IDC_SPLKNOT_NUM_SPIN:
					sikcPB2 = sikc->GetParamBlockByID(IIKChainControl::kParamBlock);
					spin1 = GetISpinner(GetDlgItem(hWnd, IDC_SPLKNOT_NUM_SPIN));
					if (!spin1) return FALSE;
					nKnots = spin1->GetIVal();
					if (!sikcPB2) return FALSE;
					sikcPB2->SetValue(IIKChainControl::kSplineKnotCount, t, nKnots);

				break;

				case IDC_HLPR_SIZE_SPIN:
					sikcPB2 = sikc->GetParamBlockByID(IIKChainControl::kParamBlock);
					spin2 = GetISpinner(GetDlgItem(hWnd, IDC_HLPR_SIZE_SPIN));
					if (!spin2) return FALSE;
					hlprSize = spin2->GetFVal();
					if (!sikcPB2) return FALSE;
					sikcPB2->SetValue(IIKChainControl::kHelpersize, t, hlprSize);
				break;

			}
		break;

		default:
			return FALSE;

		}
		return TRUE;


}

///////////////////////////////////////////////////////////////////////////////
// BonesCreationManager:: methods

BonesCreationManager::BonesCreationManager()
{
	ignoreSelectionChange = FALSE;
	hWnd         = NULL;

#ifndef NO_CREATE_IK_ASSIGN
	assignIK     = FALSE;
	assignIKRoot = TRUE;
#else // NO_CREATE_IK_ASSIGN
	assignIK     = FALSE;
	assignIKRoot = FALSE;
#endif // NO_CREATE_IK_ASSIGN

	assignEE     = TRUE; 

#if 0
	autoLink     = TRUE;
	copyJP       = TRUE;
	matchAlign   = TRUE;
#endif

	editBoneObj  = NULL;
	lastSolverSel = -1;
	suspendProc = NULL;
}


BonesCreationManager::~BonesCreationManager() // mike.ts 2002.09.12
{
	if ( editBoneObj )
	{
	//editBoneObj->MaybeAutoDelete();
	  editBoneObj->DeleteMe();
	  editBoneObj = NULL;
	}
}

BOOL BonesCreationManager::HDSolverSelected()
	{
#ifndef NO_HD_IK  // russom 07/16/01
	if (hWnd) {
		// 0 is always the HD solver
		HWND hSolverList = GetDlgItem(hWnd, IDC_BONE_IKSOLVER);
		int sel = SendMessage(hSolverList, CB_GETCURSEL, 0, 0);
		return sel==0;
		}
#endif // NO_HD_IK

	return FALSE;
	}

BOOL BonesCreationManager::CreateIKChain(INode* start, INode* end, BOOL makeEE)
	{
	// Find the selected solver in the list
	HWND hSolverList = GetDlgItem(hWnd, IDC_BONE_IKSOLVER);
	int sel = SendMessage(hSolverList, CB_GETCURSEL, 0, 0);
	if (sel<0) return FALSE;

//watje CANCELFIX
	BOOL iret = TRUE;
	
	SuspendSetKeyMode();

#ifdef NO_HD_IK  // russom 07/16/01
	// incrementing sel here does two things:
	// 1. makes sure the sel==0 HD branch is never taken
	// 2. properly sets the index into the solver name list, which is 1-based
	sel++;
#endif  // NO_HD_IK

	if (sel==0) {
		// HD IK
		theHDIKCreateChain.inBonesCreation = TRUE;
		theHDIKCreateChain.CreateIKChain(start, end, makeEE);
		theHDIKCreateChain.inBonesCreation = FALSE;
	} else {
		// New IK solver
		IKCmdOps *ikco = GET_IK_OPS_INTERFACE;
		FPValue internal_name;
		FPParams params(1, TYPE_INT, sel);
		if (ikco) {
			// Get the internal name. The GUI shows the UI name, which is the
			// class name.
			ikco->Invoke(IKCmdOps_kSolverName, internal_name, &params);
			INode *ikNode;  //this is the node which possesses SplineIKChain controller
			ikNode = ikco->CreateIKChain(start, end, internal_name.tstr->data());
			TSTR iname = internal_name.tstr->data();
			if (iname == TSTR(_T("SplineIKSolver"))){ // that means it's a SplineIK Solver
				Control *ikNodeController;
				IParamBlock2 *ikNC;
				int sCreate;
				ikNodeController = ikNode->GetTMController();
				ikNC = ikNodeController->GetParamBlockByID(IIKChainControl::kParamBlock);
				// pop-up the initial dialog box
				//watje CANCELFIX
				iret = DialogBoxParam(hInstance,MAKEINTRESOURCE(IDD_SPLINEIK_ASSIGN_DLG),
				GetCOREInterface()->GetMAXHWnd(),AssignSplineIKSolverDlgProc,(LPARAM)ikNode);	
				
				ikNC->GetValue(IIKChainControl::kAutoSplineCreate, GetCOREInterface()->GetTime(), sCreate, FOREVER);
//watje CANCELFIX
				if ((sCreate) && (iret)) {
					//create a spline using the parameters specified in the above dialog box.
					int nKnots;
					ikNC->GetValue(IIKChainControl::kSplineKnotCount, GetCOREInterface()->GetTime(), nKnots, FOREVER);
					int curveType;
					ikNC->GetValue(IIKChainControl::kSplineTypeChoice, GetCOREInterface()->GetTime(), curveType, FOREVER);
					CreateSpline(nKnots, ikNode, curveType);
				}
			}
		}

		
		}
		
	ResumeSetKeyMode();
	return iret;
	}



void BonesCreationManager::CreateSpline(int ctKnot, INode* ikNode, int curveType)
	{
	// Create a new object through the CreateInstance() API

	Tab<Point3> knotPoints;
	TimeValue t = GetCOREInterface()->GetTime();

	INode* stJoint=NULL;
	INode* endJoint=NULL;
	IDerivedObject *dobj;
	Spline3D *manipSpline;
	SplineShape* sObj;
	INode *spNode;

	Point3 hlprStPos, hlprEndPos;

	float sLength;

	// get the SplineIK parameters for the IKChain
//	IParamBlock2* sikcPB2;
//	sikcPB2 = ikNode->GetTMController()->GetParamBlockByID(IIKChainControl::kParamBlock);

	Control* iikCont = ikNode->GetTMController();
	IIKChainControl* iikc = (IIKChainControl*)iikCont->GetInterface(I_IKCHAINCONTROL);
	stJoint = iikc->StartJoint();
	endJoint = iikc->EndJoint();

	
    //Get the position of the end bone
	hlprEndPos = endJoint->GetNodeTM(t).GetTrans();
	knotPoints.Append(1, &hlprEndPos, 1); // the first knot is placed at the chain end
	

	//---------------------------------------------------------------------------------------------------------
	//Traverse the bone hierarchy and get the position of each bone pivot, travelling from the end bone,
	//up the hierarchy until the start bone is reached. Add a knot at each of these points.
	//The spline will pass through these points

	INode* pparent;
	Point3 pparent_pos;
	pparent =  endJoint->GetParentNode();
	while(pparent){
		if (pparent != stJoint) {
			pparent_pos = pparent->GetNodeTM(t).GetTrans();
			knotPoints.Append(1, &pparent_pos, 1);
			pparent = pparent->GetParentNode();
		}
		else if (pparent == stJoint){
			hlprStPos = stJoint->GetNodeTM(t).GetTrans();
			knotPoints.Append(1, &hlprStPos, 1);
			break;
		}
		else if (pparent->IsRootNode()) { //we have reached the rootnode without crossing stJt, something is wrong
			assert(1);
		}
	}
	//---------------------------------------------------------------------------------------------------------



	//---------------------------------------------
	//reverse the element order of knotPoints; now we start from "startbone"

	Tab<Point3> temp_knotPoints;
	temp_knotPoints.SetCount(knotPoints.Count());

	for (int i = 0; i<knotPoints.Count(); i++){
		temp_knotPoints[i] = knotPoints[knotPoints.Count()-i-1];

	}
	knotPoints = temp_knotPoints;

	//---------------------------------------


	if (curveType == 0){  // This is a Bezier Spline
		sObj = (SplineShape *)GetCOREInterface()->CreateInstance(SHAPE_CLASS_ID , Class_ID(SPLINE3D_CLASS_ID, 0));
		assert(sObj);
		BezierShape &bShape = sObj->GetShape();
		bShape.NewShape();
		manipSpline = bShape.NewSpline();
		
		// Create a derived object that references sObj
		dobj = CreateDerivedObject(sObj);
		// Create a node in the scene that references the derived object
		spNode = GetCOREInterface()->CreateObjectNode(dobj);
		Matrix3 mmat = spNode->GetNodeTM(t);
		Matrix3 inMmat = Inverse(spNode->GetNodeTM(t));
		//Add the knots to the spline
		for (int j = 0; j < knotPoints.Count(); j++){
			Point3 knotPt = knotPoints[j] * inMmat;
			manipSpline->AddKnot(SplineKnot(KTYPE_AUTO, LTYPE_CURVE, knotPt, knotPt, knotPt));
		}

		//create the actual spline
		manipSpline->ComputeBezPoints();
		sObj->InvalidateGeomCache();
		bShape.UpdateSels(); // must have this
		sLength = manipSpline->SplineLength();

	}

#ifndef NO_NURBS
	else if (curveType == 1){  // This is a NURBS Point Curve

		ViewExp *vExp = GetCOREInterface()->GetActiveViewport();
		Matrix3 worldToScreenTm;
		vExp->GetConstructionTM(worldToScreenTm);
		GetCOREInterface()->ReleaseViewport(vExp);

		NURBSSet nset;
		NURBSPointCurve *c = new NURBSPointCurve();
		c->SetNumPts(knotPoints.Count());
		NURBSIndependentPoint pt;
		for (int k = 0; k < knotPoints.Count(); k++) {
			pt.SetPosition(0, knotPoints[k] * Inverse(worldToScreenTm));
			c->SetPoint(k, pt);
		}
		nset.AppendObject(c);
		Matrix3 mat(1);
		Object *obj = CreateNURBSObject((IObjParam*)GetCOREInterface(), &nset, mat);
		dobj = CreateDerivedObject(obj);
		spNode = GetCOREInterface()->CreateObjectNode(dobj);

	}

	else if (curveType == 2){  // This is a NURBS CV Curve

		ViewExp *vExp = GetCOREInterface()->GetActiveViewport();
		Matrix3 worldToScreenTm;
//		vExp->GetAffineTM(worldToScreenTm);
		vExp->GetConstructionTM(worldToScreenTm);
		GetCOREInterface()->ReleaseViewport(vExp);
		
		NURBSSet nset;
		NURBSCVCurve *c = new NURBSCVCurve();
		c->SetNumCVs(knotPoints.Count());
		c->SetOrder(knotPoints.Count());
		c->SetNumKnots(2 * knotPoints.Count());
		for (int k = 0; k < knotPoints.Count(); k++) {
			c->SetKnot(k, 0.0);
			c->SetKnot(k + knotPoints.Count(), 1.0);
		}
		NURBSControlVertex cv;
		cv.SetSelected(TRUE); // make all the CVs selected
		for (int j = 0; j < knotPoints.Count(); j++){
			cv.SetPosition(t, knotPoints[j] * Inverse(worldToScreenTm));
			c->SetCV(j, cv);
		}
		nset.AppendObject(c);
		int nnobj = nset.GetNumObjects();
		Matrix3 mat(1);
		Object *obj = CreateNURBSObject((IObjParam*)GetCOREInterface(), &nset, mat);
		dobj = CreateDerivedObject(obj);
		spNode = GetCOREInterface()->CreateObjectNode(dobj);
	}
#endif

	
	// Name the node and make the name unique.
	TSTR name(_T("SplineIKNode"));
	GetCOREInterface()->MakeNameUnique(name);
	spNode->SetName(name);

	IParamBlock2 *ikCPb;
	ikCPb = ikNode->GetTMController()->GetParamBlockByID(IIKChainControl::kParamBlock);
	ikCPb->SetValue(IIKChainControl::kPickShape, t, spNode); // make the shape the goal of the SplineIK

	if(ctKnot != knotPoints.Count()  && curveType == 0){

		// Create a Normalize Spline modifier
		Modifier *normSpline = (Modifier *)GetCOREInterface()->CreateInstance(
			OSM_CLASS_ID, Class_ID(PW_PATCH_TO_SPLINE1,PW_PATCH_TO_SPLINE2));

		IParamBlock2* iNormSplineBlock = ((Animatable*)normSpline)->GetParamBlockByID(nspline_params);
		assert(iNormSplineBlock);
		float nLength;
		nLength = sLength /(ctKnot-1);

		iNormSplineBlock->SetValue(nspline_length, t, nLength);
		
		// Add the Normalize Spline modifier to the derived object.
		dobj->AddModifier(normSpline);

	}


//	int boneCt = BoneCount(stJoint, endJoint);  // we might need this later
//	int sCount = bShape.SplineCount();


	BOOL hlprChecked;
	int linkRoot;
	ikCPb->GetValue(IIKChainControl::kCreateHelper, t, hlprChecked, FOREVER);
	ikCPb->GetValue(IIKChainControl::kLinktoRootNode, t, linkRoot, FOREVER);

	IParamBlock2* isplikBlock = NULL;
	INode* firstHelperNode = NULL;
	if (hlprChecked){ // apply the modifier and create the associated helpers
		// Create a SplineIKControl modifier
		ISplineIKControl *splineikcont =
			(ISplineIKControl *)GetCOREInterface()->CreateInstance(
				OSM_CLASS_ID, Class_ID(SPLINEIKCONTROL_CLASS_ID));
		isplikBlock = ((Animatable*)splineikcont)->GetParamBlockByID(ISplineIKControl::SplineIKControl_params);
		assert(isplikBlock);

		if (linkRoot){
			//sm_link_types = 1 means "link to root" --- this needs to be thought about, we need to 
			//link the first helper (knot) of the spline to the rootbone
			isplikBlock->SetValue(ISplineIKControl::sm_link_types, t, 1);
		}
		else
			isplikBlock->SetValue(ISplineIKControl::sm_link_types, t, 2); //sm_link_types = 2 means "link to none"
	
// same BOOL variable "checked"	is used for the state of all the subsequent checkboxes

		float hlprSize;
		BOOL checked;
		ikCPb->GetValue(IIKChainControl::kHelperCentermarker, t, checked, FOREVER);
		isplikBlock->SetValue(ISplineIKControl::sm_helper_centermarker, t, checked);

		ikCPb->GetValue(IIKChainControl::kHelperAxisTripod, t, checked, FOREVER);
		isplikBlock->SetValue(ISplineIKControl::sm_helper_axistripod, t, checked);

		ikCPb->GetValue(IIKChainControl::kHelperCross, t, checked, FOREVER);
		isplikBlock->SetValue(ISplineIKControl::sm_helper_cross, t, checked);

		ikCPb->GetValue(IIKChainControl::kHelperBox, t, checked, FOREVER);
		isplikBlock->SetValue(ISplineIKControl::sm_helper_box, t, checked);
	
		ikCPb->GetValue(IIKChainControl::kHelperScreensize, t, checked, FOREVER);
		isplikBlock->SetValue(ISplineIKControl::sm_helper_screensize, t, checked);
	
		ikCPb->GetValue(IIKChainControl::kHelperDrawontop, t, checked, FOREVER);
		isplikBlock->SetValue(ISplineIKControl::sm_helper_drawontop, t, checked);

		ikCPb->GetValue(IIKChainControl::kHelpersize, t, hlprSize, FOREVER);
		isplikBlock->SetValue(ISplineIKControl::sm_helpersize, t, hlprSize);

		
		// Add the SplineIKControl modifier to the derived object.
		dobj->AddModifier(splineikcont,NULL,0);
		//Create the helpers in the modifier -- this is a separate method in the modifier
		splineikcont->CreateHelpers(ctKnot);

		spNode->EvalWorldState(t);

		// get the first helper node
		isplikBlock->GetValue(ISplineIKControl::sm_point_node_list, t, firstHelperNode, FOREVER, 0);
		if (firstHelperNode) {
			ikCPb->SetValue(IIKChainControl::kUpnode, t, firstHelperNode);
		}
	}

	if (linkRoot){
#if 0
		//-----------------------------------------------------------------
		// now we have to position constrain the rootbone to the first helper.
		IPosConstPosition *positionConstraint = (IPosConstPosition*)
			GetCOREInterface()->CreateInstance(
				CTRL_POSITION_CLASS_ID, Class_ID(POSITION_CONSTRAINT_CLASS_ID,0));
		Control* cur_pos =
			stJoint->GetTMController()->GetPositionController();
		positionConstraint->Copy(cur_pos);
		if (hlprChecked) {
			//make the first helper the target of the positionConstraint
			if(firstHelperNode != NULL) {
				positionConstraint->AppendTarget(firstHelperNode);
			}
		} else {
			positionConstraint->AppendTarget(spNode);
			IParamBlock2* pb = positionConstraint->GetParamBlockByID(0);
			pb->SetValue(2, t, TRUE);
		}
		//assign positionConstraint to the start joint of the chain
		stJoint->GetTMController()->SetPositionController(positionConstraint);
		//-----------------------------------------------------------------
#else
		//-----------------------------------------------------------------
		// now we have to path constrain the rootbone to the spline.
		IPathPosition *pathPos = GetIPathConstInterface((ReferenceTarget*)GetCOREInterface()->CreateInstance(CTRL_POSITION_CLASS_ID, Class_ID(PATH_CONTROL_CLASS_ID,0)));
		DbgAssert(pathPos != NULL);
		IParamBlock2* pb = pathPos->GetParamBlock(path_params);
		int refn = pb ? pb->GetControllerRefNum(path_percent) : -1;
		RefTargetHandle perc_ref = (refn >= 0) ? pb->GetReference(refn) : NULL;
		if (perc_ref) {
			int nk = perc_ref->NumKeys();
			for (int i = nk - 1; i >= 0; --i) {
				perc_ref->DeleteKeyAtTime(perc_ref->GetKeyTime(i));
			}
			pb->SetValue(path_percent, 0, 0.0f);
		}
		Control* cur_pos =
			stJoint->GetTMController()->GetPositionController();
		pathPos->Copy(cur_pos);
		pathPos->AppendTarget(spNode);
		//assign path controller to the start joint of the chain
		stJoint->GetTMController()->SetPositionController(pathPos);
		//-----------------------------------------------------------------
#endif
	}

	GetCOREInterface()->RedrawViews(t);
}



int BonesCreationManager::BoneCount(INode* stJt, INode* endJt){ // determines the # of nodes in a hierarchy
	int i = 2; // there are already two nodes in the hierarchy, stJt and endJt
	INode *pparent;
	pparent =  endJt->GetParentNode();
	while(pparent){
		if (pparent != stJt) {
			i = i + 1;
			pparent = pparent->GetParentNode();
		}
		else if (pparent == stJt){
			return i;
		}
		else if (pparent->IsRootNode()) { //we have reached the rootnode without crossing stJt, something is wrong
			return 0;
		}
	}
	return 0;

}

void BonesCreationManager::BuildSolverNameList(Tab<TSTR*> &solverNames)
	{
	IKCmdOps* ikops = GET_IK_OPS_INTERFACE;
	if (ikops == NULL) return;
	FPValue solver_count;
	ikops->Invoke(IKCmdOps_kSolverCount, solver_count, NULL);
	// 1-based index.
	//
	for (int i = 1; i <= solver_count.i; ++i) {
		FPValue ui_name;
		FPParams params(1, TYPE_INT, i);
		// Use UI Name, which is the class name.
		ikops->Invoke(IKCmdOps_kSolverUIName, ui_name, &params);
		TSTR* useName = new TSTR(*ui_name.tstr);
		solverNames.Append(1, &useName);
		}
	}

static INT_PTR CALLBACK BoneParamDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	BonesCreationManager *mgr = (BonesCreationManager*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	switch (msg) {
		case WM_INITDIALOG: {
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			mgr = (BonesCreationManager*)lParam;
			CheckDlgButton(hWnd,IDC_BONES_ASSIGNIK,mgr->assignIK);
			CheckDlgButton(hWnd,IDC_BONES_ASSIGNIKROOT,mgr->assignIKRoot);
			//CheckDlgButton(hWnd,IDC_BONES_CREATEENDEFFECTOR,mgr->assignEE);

#if 0
			CheckDlgButton(hWnd,IDC_BONE_AUTOLINK,mgr->autoLink);
			CheckDlgButton(hWnd,IDC_BONE_COPYIKPARAMS,mgr->copyJP);
			CheckDlgButton(hWnd,IDC_BONE_MATCHALIGNMENT,mgr->matchAlign);			
#endif

			mgr->SetButtonStates(hWnd);
#if 0
			ICustButton *but = GetICustButton(GetDlgItem(hWnd,IDC_BONES_AUTOBONE));
			but->SetType(CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(but);
#endif
			
			// Reset list 
			SendDlgItemMessage(hWnd, IDC_BONE_IKSOLVER, CB_RESETCONTENT, 0, 0);

#ifndef NO_HD_IK // removing HD solver, owen brand
			// add HD solver which is not a plug-in solver
			SendDlgItemMessage(hWnd, IDC_BONE_IKSOLVER, CB_ADDSTRING, 0, 
				(LPARAM)(TCHAR*)GetString(IDS_BONE_IKSOLVER_HD));			
#endif // NO_HD_IK

			// Find a list of all plug-in solvers
			Tab<TSTR*> solverNames;
			mgr->BuildSolverNameList(solverNames);

			// Adding string to list
			for (int i=0; i<solverNames.Count(); i++) {
				SendDlgItemMessage(hWnd, IDC_BONE_IKSOLVER, CB_ADDSTRING, 0, 
					(LPARAM)(TCHAR*)(*(solverNames[i])));				
				}

			// Reselect previous solver selection
			if (mgr->lastSolverSel<0) {
				// Try to default to the HI solver:
				TSTR szSolver(GetString(IDS_HISOLVER));
				for (int i=0; i<solverNames.Count(); i++) {
					if (*(solverNames[i])==szSolver) {
						SendDlgItemMessage(hWnd, IDC_BONE_IKSOLVER, CB_SETCURSEL, i+1, 0);
						break;
						}
					}
				if (i==solverNames.Count()) {
					// Couldn't find HI solver. Default to HD (which is index 0)
					SendDlgItemMessage(hWnd, IDC_BONE_IKSOLVER, CB_SETCURSEL, 0, 0);
					}
			} else {
				SendDlgItemMessage(hWnd, IDC_BONE_IKSOLVER, CB_SETCURSEL, mgr->lastSolverSel, 0);
				}

			// Clean up name list
			for (i=0; i<solverNames.Count(); i++) {
				delete solverNames[i];
				}

			break;
			}
		
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
#if 0
				case IDC_BONE_AUTOLINK:
					mgr->autoLink = 
						IsDlgButtonChecked(hWnd,LOWORD(wParam));
					break;
				case IDC_BONE_COPYIKPARAMS:
					mgr->copyJP =
						IsDlgButtonChecked(hWnd,LOWORD(wParam));
					break;
				case IDC_BONE_MATCHALIGNMENT:
					mgr->matchAlign =
						IsDlgButtonChecked(hWnd,LOWORD(wParam));
					break;

				case IDC_BONES_AUTOBONE:
					if (mgr->createInterface->GetCommandMode()->ID() 
						== CID_STDPICK) {
						mgr->createInterface->SetCommandMode(
							&theBonesCreateMode);
					} else {
						mgr->createInterface->SetPickMode(
							&thePickAutoBone);
						}
					break;
#endif

				case IDC_BONES_ASSIGNIK:
					mgr->assignIK = 
						IsDlgButtonChecked(hWnd,IDC_BONES_ASSIGNIK);
					mgr->SetButtonStates(hWnd);
					break;
				case IDC_BONES_ASSIGNIKROOT:
					mgr->assignIKRoot = 
						IsDlgButtonChecked(hWnd,IDC_BONES_ASSIGNIKROOT);
					mgr->SetButtonStates(hWnd);
					break;

				/*
				case IDC_BONES_CREATEENDEFFECTOR:
					mgr->assignEE = 
						IsDlgButtonChecked(hWnd,IDC_BONES_CREATEENDEFFECTOR);
					mgr->SetButtonStates(hWnd);
					break;
					*/

				case IDC_BONE_IKSOLVER:
					mgr->lastSolverSel = SendMessage(GetDlgItem(hWnd, IDC_BONE_IKSOLVER), CB_GETCURSEL, 0, 0);
					break;
				}
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}

void BonesCreationManager::SetButtonStates(HWND hWnd)
	{
	if (assignIK) {
		EnableWindow(GetDlgItem(hWnd,IDC_BONES_ASSIGNIKROOT),TRUE);
		CheckDlgButton(hWnd,IDC_BONES_ASSIGNIKROOT,assignIKRoot);
		//EnableWindow(GetDlgItem(hWnd,IDC_BONES_CREATEENDEFFECTOR),TRUE);
		//CheckDlgButton(hWnd,IDC_BONES_CREATEENDEFFECTOR,assignEE);
	} else {
		EnableWindow(GetDlgItem(hWnd,IDC_BONES_ASSIGNIKROOT),FALSE);
		CheckDlgButton(hWnd,IDC_BONES_ASSIGNIKROOT,FALSE);
		//EnableWindow(GetDlgItem(hWnd,IDC_BONES_CREATEENDEFFECTOR),FALSE);
		//CheckDlgButton(hWnd,IDC_BONES_CREATEENDEFFECTOR,FALSE);
		}
	}

void BonesCreationManager::Begin( IObjCreate *ioc, ClassDesc *desc )
{
	createInterface = ioc;
	cDesc           = desc;
	createCB        = NULL;
	curNode			= NULL;
	lastNode        = NULL;
	firstChainNode  = NULL;

#ifndef NO_CREATE_IK_ASSIGN
	hWnd = createInterface->AddRollupPage(
											hInstance, 
											MAKEINTRESOURCE(IDD_BONEPARAMS_NEW), 
											BoneParamDlgProc,
											GetString(IDS_BONE_IK_ROLLUP_NAME),
											(LPARAM)this
										);
#else // NO_CREATE_IK_ASSIGN
	hWnd = NULL;
#endif // NO_CREATE_IK_ASSIGN

	theHold.Suspend();

	if ( !editBoneObj ) // mikets 2002.09.12
	{                   // use the same instance of editBoneObject for multiple creations
		editBoneObj = CreateNewBoneObject();
	}

#ifndef NO_CREATE_TASK	// russom - 12/04/01
	editBoneObj->BeginEditParams(ioc, BEGIN_EDIT_CREATE);
#endif

	theHold.Resume();
}

void BonesCreationManager::End()
{
	if (editBoneObj) 
	{
		// Stop editing and dump object
		theHold.Suspend();

#ifndef NO_CREATE_TASK	// russom - 12/04/01
		editBoneObj->EndEditParams(createInterface, END_EDIT_REMOVEUI);
#endif
		//editBoneObj->MaybeAutoDelete();
		//editBoneObj = NULL;
		theHold.Resume();
	}

	createInterface->ClearPickMode();

#ifndef NO_CREATE_IK_ASSIGN
	createInterface->DeleteRollupPage(hWnd);
#endif // NO_CREATE_IK_ASSIGN

	if (curNode) 
	{
		DeleteReference(0);  // sets curNode = NULL	
	}
}


#if 0
void BonesCreationManager::AutoBone(INode *cnode,INode *pnode)
	{	
	// Create child node
	Object *ob = (Object *)createInterface->
		CreateInstance(HELPER_CLASS_ID,Class_ID(BONE_CLASS_ID,0)); 			
	INode *node = createInterface->CreateObjectNode(ob);
	node->ShowBone(1);		
	Matrix3 ntm = cnode->GetNodeTM(0);
	
	// Only match position if so desired
	if (!matchAlign) {
		Point3 pos = ntm.GetTrans();
		ntm = Matrix3(1);
		ntm.SetTrans(pos);
		}

	// Assign IK controllers
	if (assignIK && (pnode || assignIKRoot)) {
		IKMasterControl *master;
		IKSlaveControl *slave;

		// Either use existing master or make a new one
		Control *cont = NULL;
		if (pnode && (cont=pnode->GetTMController()) &&
			cont->ClassID()==IKSLAVE_CLASSID) {
			// Use its master
			slave  = (IKSlaveControl*)cont;
			master = slave->GetMaster();
		} else {
			// Make a new master
			master = CreateIKMasterControl();
			}

		// Create a slave and assign it to the new node
		slave = CreateIKSlaveControl(master,node);
		node->SetTMController(slave);
		}	
	
	// Set the nod TM and link to parent bone
	node->SetNodeTM(0,ntm);
	if (pnode) pnode->AttachChild(node);	

	// Recurse
	int nc = cnode->NumberOfChildren();
	for (int i=nc-1; i>=0; i--) {
		AutoBone(cnode->GetChildNode(i),node);
		}

	// Attach to bone
	if (autoLink) node->AttachChild(cnode);	

	// Copy IK params
	if (copyJP) {
		IKClipObject *clip = NULL;
		Control *src = cnode->GetTMController();
		Control *dst = node->GetTMController();

		// Do position
		if (src->CanCopyIKParams(COPYPASTE_IKPOS)) 
			clip = src->CopyIKParams(COPYPASTE_IKPOS);
		if (clip && dst->CanPasteIKParams(clip,COPYPASTE_IKPOS)) {
			dst->PasteIKParams(clip,COPYPASTE_IKPOS);
			}
		if (clip) clip->DeleteThis();
		clip = NULL;

		// Do Rotation
		if (src->CanCopyIKParams(COPYPASTE_IKROT)) 
			clip = src->CopyIKParams(COPYPASTE_IKROT);
		if (clip && dst->CanPasteIKParams(clip,COPYPASTE_IKROT)) {
			dst->PasteIKParams(clip,COPYPASTE_IKROT);
			}
		if (clip) clip->DeleteThis();
		clip = NULL;
		}

	// Last nodes in the chain become end effectors
	if (!nc) {
		lastNode = node;
		MakeEndEffector();
		lastNode = NULL;
		}
	}

void BonesCreationManager::DoAutoBone(INode *node)
	{
	theHold.Begin();
	AutoBone(node,NULL);
	createInterface->RedrawViews(createInterface->GetTime());
	theHold.Accept(IDS_DS_CREATE);	 
	}

void BonesCreationManager::AssignIKControllers(
		INode *cnode,INode *pnode,BOOL newParent,Matrix3 constTM)
	{
	Point3 pos;
	IKMasterControl *master;
	IKSlaveControl *slave;
	int axis = 0;
	float projX, projY, projZ, best;

	// Find the world axis that's closest to the construction plane's Z axis
	projX = (float)fabs(DotProd(constTM.GetRow(2),Point3(1,0,0)));
	projY = (float)fabs(DotProd(constTM.GetRow(2),Point3(0,1,0)));
	projZ = (float)fabs(DotProd(constTM.GetRow(2),Point3(0,0,1)));
	best  = projX;
	if (projY>best) {
		axis = 1;
		best = projY;
		}
	if (projZ>best) axis = 2;

	if (newParent && assignIKRoot) {
		// Need to create a new master controller
		master = CreateIKMasterControl();
		slave  = CreateIKSlaveControl(master,pnode);
		pos    = pnode->GetNodeTM(0).GetTrans();
		slave->SetInitPos(pos);
		slave->SetDOF(axis+3,TRUE);
		pnode->SetTMController(slave);
	} else {
		// See if the parent has an IK controller
		Control *cont = pnode->GetTMController();
		if (cont->ClassID()==IKSLAVE_CLASSID) {
			// Use its master
			slave  = (IKSlaveControl*)cont;
			master = slave->GetMaster();
		} else {
			// Make a new master
			master = CreateIKMasterControl();
			}
		}

	// Create a slave for the child node
	slave = CreateIKSlaveControl(master,cnode);
	pos   = cnode->GetNodeTM(0).GetTrans();
	slave->SetInitPos(pos);
	slave->SetDOF(axis+3,TRUE);
	cnode->SetTMController(slave);
	}

#endif

void BonesCreationManager::MakeEndEffector()
	{
	if (!lastNode || !assignEE) return;
	IKSlaveControl *slave;
	Control *cont = lastNode->GetTMController();
	if (cont && cont->ClassID()==IKSLAVE_CLASSID) {			
		slave  = (IKSlaveControl*)cont;
	} else return;

	Matrix3 tm = lastNode->GetNodeTM(createInterface->GetTime());
	slave->MakeEE(TRUE,1,tm.GetTrans(),Quat());
	}

Object *BonesCreationManager::CreateNewBoneObject()
	{
	macroRec->BeginCreate(GetNewBonesDesc());

	BoneObj *obj = (BoneObj*)createInterface->
		CreateInstance(GEOMOBJECT_CLASS_ID,BONE_OBJ_CLASSID);

	return obj;
	}


void BonesCreationManager::UpdateBoneLength(INode *parNode)
	{
	Point3 pos(0,0,0);
	int ct=0;

	// Compute the average bone child pos
	for (int i=0; i<parNode->NumberOfChildren(); i++) {
		INode *cnode = parNode->GetChildNode(i);
		if (cnode->GetBoneNodeOnOff()) {
			pos += cnode->GetNodeTM(createInterface->GetTime()).GetTrans();
			ct++;
			}
		}
	
	if (ct) {
		// To get the bone length, we transform the average child position back 
		// into parent space. We then use the distance along the parent X axis
		// for bone length 
		pos /= float(ct);
		Matrix3 ntm = parNode->GetNodeTM(createInterface->GetTime());		
		parNode->SetNodeTM(createInterface->GetTime(), ntm);		
		pos = pos * Inverse(ntm);
		float len = pos.x;
		
		// Plug the new length into the object
		Object *obj = parNode->GetObjectRef();
		if (obj && 
			obj->ClassID()==BONE_OBJ_CLASSID && 
			obj->SuperClassID()==GEOMOBJECT_CLASS_ID) {
			BoneObj *bobj = (BoneObj*)obj;
			bobj->pblock2->SetValue(boneobj_length, TimeValue(0), len);
			}
	} else {
		// No children. Bone should appear as a nub.

		}

	// During creation we are going to set the length directly and keep the stretch factor at 0.
	// After creation, changes in bone length will be accomplished by stretching.
	parNode->ResetBoneStretch(createInterface->GetTime());	

	}

void BonesCreationManager::UpdateBoneCreationParams()
	{
	if (editBoneObj) {
		// This causes the displayed parameters to be copied to the class parameters so the
		// next bone object created will use the updated params.
#ifndef NO_CREATE_TASK	// russom - 12/04/01
		editBoneObj->EndEditParams(createInterface, 0);
		editBoneObj->BeginEditParams(createInterface, BEGIN_EDIT_CREATE);
#endif
		}
	}

void BonesCreationManager::SetupObjectTM(INode *node)
	{	
	//node->SetObjOffsetRot(QFromAngAxis(HALFPI, Point3(0,-1,0)));
	}

void BonesCreationManager::SetReference(int i, RefTargetHandle rtarg) { 
	switch(i) {
		case 0: curNode = (INode *)rtarg; break;
		default: assert(0); 
		}
	}

RefTargetHandle BonesCreationManager::GetReference(int i) { 
	switch(i) {
		case 0: return (RefTargetHandle)curNode;
		default: assert(0); 
		}
	return NULL;
	}


RefResult BonesCreationManager::NotifyRefChanged(
	Interval changeInt, 
	RefTargetHandle hTarget, 
	PartID& partID,  
	RefMessage message) 
	{
	switch (message) {
		case REFMSG_TARGET_SELECTIONCHANGE:
		 	if ( ignoreSelectionChange ) {
				break;
				}
		 	if ( curNode==hTarget ) {
				// this will set curNode== NULL;
				DeleteReference(0);
				goto endEdit;
				}
			// fall through

		case REFMSG_TARGET_DELETED:		
			if ( curNode==hTarget ) {				
				endEdit:
				curNode    = NULL;
				}
			break;		
		}
	return REF_SUCCEED;
	}

static int DSQ(IPoint2 p, IPoint2 q) {
	return (p.x-q.x)*(p.x-q.x)+(p.y-q.y)*(p.y-q.y);
	}


class BonesPicker: public PickNodeCallback {
	BOOL Filter(INode *node) {
		return node->GetBoneNodeOnOff();
#if 0
		Object* obj = node->GetObjectRef();
		if (obj && obj->SuperClassID()==HELPER_CLASS_ID && obj->ClassID()==Class_ID(BONE_CLASS_ID,0))
			return 1;
		return 0;
#endif
		}
	};



static Matrix3 ComputeExtraBranchLinkPosition(TimeValue t,INode *parNode, Matrix3 constTM, BOOL ignoreKids = FALSE )
{
	Point3 averageChildBonePos(0,0,0);
	int ct=0;

	if ( !ignoreKids )
	{
		for (int i=0; i<parNode->NumberOfChildren(); i++) 
		{
			INode *childNode = parNode->GetChildNode(i);
			if (childNode->GetBoneNodeOnOff()) 
			{
				averageChildBonePos += childNode->GetNodeTM(t).GetTrans();
				ct++;
			}
		}
	}

	if (ct>0) 
	{
		// Position the construction plane at the average position of children
		constTM.SetTrans(averageChildBonePos/float(ct));
	} else 
	{		
		// No children. Position the construction plane at the end of the last bone.
		Matrix3 potm = parNode->GetObjectTM(t);		
		Object *obj = parNode->GetObjectRef();
		if (obj->ClassID()==BONE_OBJ_CLASSID) 
		{
			BoneObj *bobj = (BoneObj*)obj;
			float length;
			bobj->pblock2->GetValue(boneobj_length, t, length, FOREVER);
			constTM.SetTrans(potm.GetTrans() + length * potm.GetRow(0));
		}		
	}

	return constTM;
}

static void SetHideBoneObject(INode *node, BOOL onOff)
	{
	if (!node) return;
	Object *obj = node->GetObjectRef();
	if (obj && obj->ClassID()==BONE_OBJ_CLASSID) {
		if (onOff)
			 obj->SetAFlag(A_HIDE_BONE_OBJECT);
		else obj->ClearAFlag(A_HIDE_BONE_OBJECT);
		}
	}

static void MakeNubBone(TimeValue t, INode *nubBone)
	{
	Matrix3 ptm = nubBone->GetParentTM(t);
	Matrix3 ntm = nubBone->GetNodeTM(t);
	

	// Set the nub to have the same transform as its parent (but retain its position)
	ptm.SetTrans(ntm.GetTrans());
	nubBone->SetNodeTM(t, ptm);

	// Plug in a new bone length
	Object *obj = nubBone->GetObjectRef();
	if (obj->ClassID()==BONE_OBJ_CLASSID) {
		BoneObj *bobj = (BoneObj*)obj;
		bobj->ClearAFlag(A_HIDE_BONE_OBJECT);
		float width, height, length;
		bobj->pblock2->GetValue(boneobj_width, t, width, FOREVER);
		bobj->pblock2->GetValue(boneobj_height, t, height, FOREVER);
		length = float(sqrt(width*height));
		bobj->pblock2->SetValue(boneobj_length, t, length);
		}

	}

static void SetParentBoneAlignment(
		TimeValue t, INode *parNode, INode *childNode, Matrix3 &constTM)
	{
	//
	// Setup the alignment on the parent node's TM.
	// The X axis points to the child
	// The Z axis is parallel to the construction plane normal
	// The Y axis is perpindicular to X and Z
	//
	Matrix3 ptm = parNode->GetNodeTM(t);
	Matrix3 ntm = childNode->GetNodeTM(t);
	Point3 xAxis = Normalize(ntm.GetTrans()-ptm.GetTrans());
	Point3 zAxis = constTM.GetRow(2);
	Point3 yAxis = Normalize(CrossProd(zAxis, xAxis));

	// RB 12/14/2000: Will fix 273660. If the bones are being created off the
	// construction plane (3D snap), the Z axis may not be perpindicular to X
	// so we need to orthogonalize.
	zAxis = Normalize(CrossProd(xAxis, yAxis));
	yAxis = Normalize(CrossProd(zAxis, xAxis));

	ptm.SetRow(0, xAxis);
	ptm.SetRow(1, yAxis);
	ptm.SetRow(2, zAxis);

	// Plug in the TMs.
	parNode->SetNodeTM(t, ptm);
	childNode->SetNodeTM(t, ntm);

	}

class HideCatRestore : public RestoreObj {
public:
	HideCatRestore(DWORD undo, DWORD redo)
		: mRestFlag(undo), mRedoFlag(redo) {}
	void Restore(int isUndo);
	void Redo();
	TSTR Description() { return TSTR(_T("Change Hide Category")); }
	DWORD mRestFlag;
	DWORD mRedoFlag;
};

void HideCatRestore::Restore(int isUndo)
{
	GetCOREInterface()->SetHideByCategoryFlags(mRestFlag);
}

void HideCatRestore::Redo()
{
	GetCOREInterface()->SetHideByCategoryFlags(mRedoFlag);
}

///////////////////////////////////////////////////////////////////////////////
// class BoneRefineRestoreObj

class BoneRefineRestoreObj : public RestoreObj
{

	public:

		TimeValue        refineTime;
		TimeValue        undoTime;

		BoneRefineRestoreObj      ( TimeValue tv );

		void      Restore          (int isUndo); // override
		void      Redo             ();           // override
		TSTR      Description      ();           // override
		int		  Size             ();           // override

		void      goToTime         ( TimeValue tv );
};

BoneRefineRestoreObj::BoneRefineRestoreObj ( TimeValue tv )
{
	refineTime = tv;
	undoTime   = TIME_NegInfinity;
}

void BoneRefineRestoreObj::Restore (int isUndo)
{
	if ( isUndo )
	{
	  undoTime = GetCOREInterface()->GetTime();
	}
}

void BoneRefineRestoreObj::Redo()
{
	goToTime(undoTime);
}


TSTR BoneRefineRestoreObj::Description() 
{ 
	return TSTR(_T(" ")); 
}

int	BoneRefineRestoreObj::Size()
{
	return sizeof(BoneRefineRestoreObj);
}

void BoneRefineRestoreObj::goToTime( TimeValue tv )
{
	if ( tv != TIME_NegInfinity )
	{
		Interface* ip = GetCOREInterface();

		if ( tv != ip->GetTime() )
		{
			ip->SetTime(tv,TRUE);
		}
	}
}


///////////////////////////////////////////////////////////////////////////////

int BonesCreationManager::proc(
		HWND hwnd, int msg, int point, int flag, IPoint2 m)				
	{	
	int res;
	INode *newNode,*parNode;	
	BonesPicker bonePick;
	ViewExp *vpx = createInterface->GetViewport(hwnd); 
	assert( vpx );
	Matrix3 constTM;
	vpx->GetConstructionTM(constTM);
	Point3 boneColor = GetUIColor(COLOR_BONES);
	static float constPlaneZOffset = 0.0f;
	TimeValue t = createInterface->GetTime();

	switch (msg) {

		case MOUSE_POINT:
			{
			Object *ob;
			if (point==0) {
				newBoneChain.SetCount(0);
				SuspendAnimate();
				AnimateOff();
				MouseManager* mm = createInterface->GetMouseManager();
				if (mm != NULL) {
				  // This is the additional window proc that intercepts
				  // the mouse event for the manipulator. It interferes
				  // with bone creation. We temporarily suspend it.
				  suspendProc = mm->GetMouseWindProcCallback();
				  mm->SetMouseWindProcCallback(NULL);
				  }

				mat.IdentityMatrix();
				if ( createInterface->SetActiveViewport(hwnd) ) {
					return FALSE;
					}

				// RB 7/28/00: Wait for first mouse up to avoid making small bones.
				res = TRUE;
				goto done;

				}
			else  {
				if (DSQ(m,lastpt)<10) {
					res = TRUE;
					goto done;
					}
				
				theHold.SuperAccept(IDS_DS_CREATE);
				}

			if (createInterface->IsCPEdgeOnInView()) { 
				res = FALSE;
				goto done;
				}

			
			// Make sure the next bone created uses the creation parameters displayed
			// by the bone object UI in the command panel.
			UpdateBoneCreationParams();

			theHold.SuperBegin();	 // begin hold for undo

			DWORD hide_flag = GetCOREInterface()->GetHideByCategoryFlags();
			if (hide_flag & HIDE_BONEOBJECTS) {
				DWORD unhide = hide_flag & (~HIDE_BONEOBJECTS);
				theHold.Begin();
				theHold.Put(new HideCatRestore(hide_flag, unhide));
				theHold.Accept(NULL);
				GetCOREInterface()->SetHideByCategoryFlags(unhide);
			}

			theHold.Begin();
			
			//mat.IdentityMatrix();
			mat = constTM;
#ifdef _3D_CREATE
			mat.SetTrans(constTM*vpx->SnapPoint(m,m,NULL,SNAP_IN_3D));
#else
			//mat.SetTrans(vpx->SnapPoint(m,m,NULL,SNAP_IN_PLANE));
			mat.SetTrans(constTM*vpx->SnapPoint(m,m,NULL,SNAP_IN_PLANE));
#endif
			
			BOOL newParent = FALSE, overB=FALSE;
			if (curNode==NULL) 	{
				constPlaneZOffset = 0.0f;
				INode* overBone = createInterface->PickNode(hwnd,m,&bonePick);
				if (overBone) {

					// Compute the Z distance of the existing bone the user clicked on from
					// the current construction plane. We'll add this offset to every new bone
					// created in this chain.

					// { Commented out by mike.ts 2002.10.18 
					//	 Matrix3 extraBranchTM = ComputeExtraBranchLinkPosition(t,overBone,mat);
					// }
					
					// { Added by mike.ts 2002.10.18
					Matrix3 extraBranchTM = ComputeExtraBranchLinkPosition(t,overBone,mat,TRUE);
					// }

					constPlaneZOffset = (Inverse(constTM) * extraBranchTM.GetTrans()).z;

					// We don't want 'mat' to get offset twice so we'll save and restore it.
					Matrix3 oldMat = mat;
					mat.SetTrans(mat.GetTrans() + constPlaneZOffset*constTM.GetRow(2));

					// Instead of just linking newly created bones to the existing parent,
					// insert an extra link under the existing parent bone and position this
					// extra link at the average child position of the existing parent bone.					
					ob = CreateNewBoneObject();
					parNode = createInterface->CreateObjectNode(ob);
//watje
					for (int i=0; i < GetCOREInterface()->GetNumberDisplayFilters(); i++)
						{
						if (GetCOREInterface()->DisplayFilterIsNodeVisible(i,ob->SuperClassID(), ob->ClassID(), parNode))
							{
							GetCOREInterface()->SetDisplayFilter(i, FALSE );
							}
						}

					newBoneChain.Append(1,&parNode);
					parNode->SetWireColor(RGB(int(boneColor.x*255.0f), int(boneColor.y*255.0f), int(boneColor.z*255.0f) ));
					parNode->SetNodeTM(0,extraBranchTM);					
					macroRec->SetNodeTM(parNode, extraBranchTM);
					
					SetupObjectTM(parNode);

					// { Commented out by mike.ts 2002.10.18 
					//	parNode->SetBoneNodeOnOff(TRUE, t);
					//	parNode->SetRenderable(FALSE);
					//	//parNode->ShowBone(1); // Old bones on by default
					//	overBone->AttachChild(parNode);										
					//	// RB 12/13/2000: Don't want to align the pre-existing bone
					//	//SetParentBoneAlignment(t, overBone, parNode, constTM);
					//	UpdateBoneLength(overBone);
					// }

					// { Added by mike.ts 2002.10.18
					parNode->SetBoneNodeOnOff(FALSE,t); 
					parNode->SetRenderable(FALSE);
					Matrix3 oldOverBoneTM = overBone->GetNodeTM(t);
					overBone->AttachChild(parNode);										
					SetParentBoneAlignment(t, overBone, parNode, constTM);
					overBone->SetNodeTM(t,oldOverBoneTM);
					parNode->SetBoneNodeOnOff(TRUE,t); // mike.ts
					// }

					newParent = TRUE;
					
					if (HDSolverSelected()) {
						firstChainNode = overBone;
					} else {
						firstChainNode = parNode;
						}

					mat = oldMat;

					//parNode = overBone;					
					overB = TRUE;
				} else {
					// Make first node 
					//ob = (Object *)createInterface->
					//	CreateInstance(HELPER_CLASS_ID,Class_ID(BONE_CLASS_ID,0));					
					ob = CreateNewBoneObject();
					parNode = createInterface->CreateObjectNode(ob);
//watje
					for (int i=0; i < GetCOREInterface()->GetNumberDisplayFilters(); i++)
						{
						if (GetCOREInterface()->DisplayFilterIsNodeVisible(i,ob->SuperClassID(), ob->ClassID(), parNode))
							{
							GetCOREInterface()->SetDisplayFilter(i, FALSE );
							}
						}

					newBoneChain.Append(1,&parNode);

					//createInterface->SetNodeTMRelConstPlane(parNode, mat);										
					parNode->SetWireColor(RGB(int(boneColor.x*255.0f), int(boneColor.y*255.0f), int(boneColor.z*255.0f) ));
					parNode->SetNodeTM(0,mat);					
					macroRec->SetNodeTM(parNode, mat);
					
					SetupObjectTM(parNode);
					parNode->SetBoneNodeOnOff(TRUE, t);
					parNode->SetRenderable(FALSE);
					//parNode->ShowBone(1); // Old bones on by default
					newParent = TRUE;

					firstChainNode = parNode;
					}
				//parNode->ShowBone(1);

				// RB 5/10/99
				lastNode = parNode;				

			} else {
				lastNode = parNode = curNode;
				DeleteReference(0);
				}

			// Make new node 
			//ob = (Object *)createInterface->
			//	CreateInstance(HELPER_CLASS_ID,Class_ID(BONE_CLASS_ID,0)); 			
			ob = CreateNewBoneObject();
			newNode = createInterface->CreateObjectNode(ob);			
			newBoneChain.Append(1,&newNode);
			SetupObjectTM(newNode);
			newNode->SetBoneNodeOnOff(TRUE, t);
			newNode->SetWireColor(RGB(int(boneColor.x*255.0f), int(boneColor.y*255.0f), int(boneColor.z*255.0f) ));
			newNode->SetRenderable(FALSE);
			//newNode->ShowBone(1); // Old bones on by default

			//newNode->ShowBone(1);
			
			//createInterface->SetNodeTMRelConstPlane(newNode, mat);
			mat.SetTrans(mat.GetTrans() + constPlaneZOffset*constTM.GetRow(2));			
			newNode->SetNodeTM(0,mat);							
			macroRec->SetNodeTM(newNode, mat);

			parNode->AttachChild(newNode); // make node a child of prev 
			
			// Align the parent bone to point at the child bone
			if (overB && parNode) {
				SetParentBoneAlignment(t, parNode, newNode, constTM);
				UpdateBoneLength(parNode);
				}

			curNode = newNode;
			SetHideBoneObject(curNode, TRUE);

			// Reference the new node so we'll get notifications.
			MakeRefByID( FOREVER, 0, curNode);

#if 0
			// Assign IK controllers
			if (assignIK) 
				AssignIKControllers(curNode,parNode,newParent,constTM);
#endif

			ignoreSelectionChange = TRUE;
			createInterface->SelectNode( curNode);
			ignoreSelectionChange = FALSE;
 			
			createInterface->RedrawViews(t);  
			theHold.Accept(IDS_DS_CREATE);	 
			lastpt = m;
			res = TRUE; 
			}
			break;

		case MOUSE_MOVE:
			// The user can loose capture by switching to another app.
			if (!GetCapture()) {				
				theHold.SuperAccept(IDS_DS_CREATE);
				macroRec->EmitScript();
				return FALSE;
				}
			
			if (curNode) {
				
				// Hide the bone floating under the cursor during creation.
				SetHideBoneObject(curNode, TRUE);
				SetHideBoneObject(lastNode, FALSE);				

#ifdef _3D_CREATE
				mat.SetTrans(constTM*vpx->SnapPoint(m,m,NULL,SNAP_IN_3D));
#else
				//mat.SetTrans(vpx->SnapPoint(m,m,NULL,SNAP_IN_PLANE));
				mat.SetTrans(constTM*vpx->SnapPoint(m,m,NULL,SNAP_IN_PLANE));
#endif
				mat.SetTrans(mat.GetTrans() + constPlaneZOffset*constTM.GetRow(2));				

				//createInterface->SetNodeTMRelConstPlane(curNode, mat);
				curNode->SetNodeTM(t,mat);				
				macroRec->SetNodeTM(curNode, mat);

				if (lastNode) {
					SetParentBoneAlignment(t, lastNode, curNode, constTM);
					UpdateBoneLength(lastNode);
					}
								
				createInterface->RedrawViews(t);
				}
			res = TRUE;
			break;

		case MOUSE_FREEMOVE: {
			INode* overNode = createInterface->PickNode(hwnd,m,&bonePick);
			if (overNode) {
				SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR)));
				}
			else {
				SetCursor(LoadCursor(NULL, IDC_ARROW));
				}
			}
#ifdef _OSNAP
		#ifdef _3D_CREATE
				vpx->SnapPreview(m,m,NULL, SNAP_IN_3D);
		#else
				vpx->SnapPreview(m,m,NULL, SNAP_IN_PLANE);
		#endif
#endif
			break;

// mjm - 3.1.99
		case MOUSE_PROPCLICK:
			// right click while between creations
			createInterface->RemoveMode(NULL);
			break;
// mjm - end

		case MOUSE_ABORT:
			if (curNode) {
				INode *oldCurNode = curNode;
				theHold.SuperCancel(); // this deletes curNode and everything (and sets curNode to NULL)
				curNode = NULL;				

#ifndef TURN_LAST_BONE_INTO_NUB
				// Delete the last two bones
				INode *deletedBone = NULL;
				if (newBoneChain.Count()>2) {										
					deletedBone = newBoneChain[newBoneChain.Count()-2];
					if (deletedBone!=oldCurNode && deletedBone!=firstChainNode) {										
						theHold.Begin();
						if (lastNode==deletedBone) {
							if (lastNode!=firstChainNode) {
								lastNode = lastNode->GetParentNode();
							} else {
								lastNode       = NULL;
								firstChainNode = NULL;
								}
							}
						createInterface->DeleteNode(deletedBone, FALSE);

						// Also select last node
						if (lastNode) {
							ignoreSelectionChange = TRUE;
							createInterface->SelectNode(lastNode);
							ignoreSelectionChange = FALSE;
							}

						theHold.Accept(GetString(IDS_BONE_UNDO_DELETE_BONE));
						}
				} else {
					// If only two or less bones are created, then the whole chain creation is
					// being canceled.
					firstChainNode = NULL;
					}
#else
				if (newBoneChain.Count()>2) {
					// Turn the last bone into a nub					
					INode *nubBone = newBoneChain[newBoneChain.Count()-2];
					MakeNubBone(t, nubBone);
				} else {
					// If only two or less bones are created, then the whole chain creation is
					// being canceled.
					firstChainNode = NULL;
					}
#endif				

				// firstChainNode may have just been deleted if we aborted before
				// the first two nodes have been placed. In this case the creation
				// of the whole bone chain is being canceled.
				//if (firstChainNode==oldCurNode || firstChainNode==deletedBone) {
				//	firstChainNode = NULL;
				//	}
				//curNode = NULL;	 
				
				//MakeEndEffector();
				if (firstChainNode && assignIK) {
					
					// Handle the single node chain case
					if (!lastNode) lastNode = firstChainNode;
					
					// We may not want to assign an IK controller to the first parent
					// if its a child of the scene root.
					BOOL doAssign = TRUE;
					if (!assignIKRoot) {
						if (firstChainNode->GetParentNode()->IsRootNode()) {							
							// Set the first chain node to its child
							if (firstChainNode->NumberOfChildren()) {
								firstChainNode = firstChainNode->GetChildNode(
									firstChainNode->NumberOfChildren()-1);
							} else {
								// We must have only created one bone. Because assignIKRoot is turned off,
								// the result should be that no IK is assigned.
								doAssign = FALSE;
								}
							}
						}

					if (doAssign) {
						theHold.Begin();
//watje CANCELFIX
						BOOL accept = CreateIKChain(firstChainNode, lastNode, assignEE);
						if (accept)
							theHold.Accept(GetString(IDS_BONE_UNDO_ASSIGN_IK));
						else theHold.Cancel();
						}
					//theHDIKCreateChain.CreateIKChain(firstChainNode, lastNode, assignEE);
					//if (assignEE) MakeEndEffector();
					}

				macroRec->EmitScript();
				createInterface->RedrawViews(t); 
				}
			
			if (suspendProc != NULL) {
			  createInterface->GetMouseManager()->SetMouseWindProcCallback(suspendProc);
			  suspendProc = NULL;
			  }
			ResumeAnimate();
			res = FALSE;
			break;
		}
	
	done:
	createInterface->ReleaseViewport(vpx); 
	return res;
	}

int BoneClassDesc::BeginCreate(Interface *i)
	{
	IObjCreate *iob = i->GetIObjCreate();
	
	theBonesCreateMode.Begin( iob, this );
	iob->PushCommandMode( &theBonesCreateMode );
	
	return TRUE;
	}

int BoneClassDesc::EndCreate(Interface *i)
	{
	
	theBonesCreateMode.End();
	i->RemoveMode( &theBonesCreateMode );

	return TRUE;
	}


///////////////////////////////////////////////////////////////////////////////

class BoneObjUserDlgProc;

class PickBoneRefineMode : 
		public PickModeCallback,
		public PickNodeCallback {
	public:		
		BoneObjUserDlgProc *dlg;
		INode*pickedNode;
		float refinePercentage;

		PickBoneRefineMode() {dlg = NULL; pickedNode = NULL; }
		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);
		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);
		BOOL RightClick(IObjParam *ip,ViewExp *vpt)	{return TRUE;}
		BOOL Filter(INode *node);
		PickNodeCallback *GetFilter() {return this;}
	};


class BoneObjUserDlgProc : public ParamMap2UserDlgProc {
	public:
		BoneObj *bo;
		ICustButton *iRefine;
		BOOL inCreate;
		PickBoneRefineMode refineMode;

//		BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		void DeleteThis() {delete this;}
		BoneObjUserDlgProc(BoneObj *b, BOOL ic) {
			bo=b;
			inCreate=ic;
			iRefine=NULL;
			refineMode.dlg=this;}
		~BoneObjUserDlgProc() {ReleaseICustButton(iRefine);iRefine=NULL;}
		INode *GetCurBoneNode();

		static void RefineSegment        ( INode*   parNode, float refinePercentage);
		static void CopyRefinedJointData ( Control* parCont, Control* newCont, BOOL ikSlave );
	};

INode *BoneObjUserDlgProc::GetCurBoneNode()
	{
	if (GetCOREInterface()->GetSelNodeCount()!=1) return NULL;
	INode *selNode = GetCOREInterface()->GetSelNode(0);
	Object *obj = selNode->GetObjectRef();
	if (obj->ClassID()==BONE_OBJ_CLASSID)
		return selNode;
	return NULL;
	}

void BoneObjUserDlgProc::RefineSegment(INode *parNode, float refinePercentage)
{
	// Find the bone node we're refining and get at its object
	Object *obj = parNode->GetObjectRef();
	if (obj->ClassID()!=BONE_OBJ_CLASSID)
		return;
	BoneObj *bobj = (BoneObj*)obj;

	// Make the new object (as a clone of the bone we're refining)
	RemapDir *rmap = NewRemapDir();
	BoneObj *newObj = (BoneObj*)bobj->Clone(*rmap);
	rmap->DeleteThis();

	// Begin holding and turn animating off
	theHold.Begin();
	SuspendAnimate();
	AnimateOff();

	TimeValue t = GetCOREInterface()->GetTime();

	if ( theHold.Holding() ) 
	{
		theHold.Put(new BoneRefineRestoreObj(t));
	}

	float oneMinusRefinePercentage = 1.0f - refinePercentage;


	// Determine the position for the new node.
	float length;
	bobj->pblock2->GetValue(boneobj_length, t, length, FOREVER);
	Matrix3 otm   = parNode->GetObjectTM(t);
	Point3 pos    = otm * Point3(length * refinePercentage, 0.0f, 0.0f);
	Matrix3 newTM = parNode->GetNodeTM(t);
	newTM.SetTrans(pos);

	// Shrink the existing bone object
	bobj->pblock2->SetValue(boneobj_length, t, length * refinePercentage);

	// Setup the bone node
	newObj->pblock2->SetValue(boneobj_length, t, length * oneMinusRefinePercentage);
	INode *newNode = GetCOREInterface()->CreateObjectNode(newObj);
	Point3 boneColor = GetUIColor(COLOR_BONES);
	newNode->SetWireColor(RGB(int(boneColor.x*255.0f), int(boneColor.y*255.0f), int(boneColor.z*255.0f) ));
	newNode->SetNodeTM(t, newTM);
	newNode->SetBoneNodeOnOff(TRUE, t);
	newNode->SetRenderable(FALSE);

	// setup showbone flag 
	if ( parNode->IsBoneShowing() )
	{
		newNode->ShowBone ( parNode->IsBoneOnly() ? 2 : 1 );
	}
	else
	{
		newNode->ShowBone(0);
	}

	// Copy bone params
	newNode->SetBoneAutoAlign(parNode->GetBoneAutoAlign());
	newNode->SetBoneFreezeLen(parNode->GetBoneFreezeLen());
	newNode->SetBoneScaleType(parNode->GetBoneScaleType());
	newNode->SetBoneAxis(parNode->GetBoneAxis());
	newNode->SetBoneAxisFlip(parNode->GetBoneAxisFlip());

	// Copy material and wire frame colors
	newNode->SetMtl(parNode->GetMtl());
	newNode->SetWireColor(parNode->GetWireColor());

	// check the parent for group membership
	BOOL parentIsGroupMember = FALSE;
	if ( IAssembly* iasm = GetAssemblyInterface(parNode) )
	{
		if ( iasm->IsAssemblyMember() )
		{
			parentIsGroupMember = TRUE;
		}
	}


	// Attach children of parent to new node
	if ( int numKids = parNode->NumberOfChildren() ) // if there are any children at all
	{
		// create the list of children _before_ any parent reassignment is done
		INodeTab kids;
		kids.SetCount(numKids);
		
		for (int i=0; i < numKids; i++) 
		{
			kids[i] = parNode->GetChildNode(i);
		}

//		parNode->AttachChild(newNode);
//		if ( parentIsGroupMember ) 
//		{
//			if ( IAssembly* iasm = GetAssemblyInterface(newNode) )
//			{
//				iasm->SetAssemblyMember(TRUE);
//				iasm->SetAssemblyMemberOpen(TRUE);
//			}
//		}

		// now process the nodes from the kids list and feel free to reassign the parent

		for ( i=0; i < numKids; i++ )  
		{
			INode*   cnode       = kids[i];
			Control* childTMCont = cnode->GetTMController();

			// Handle IK slave controllers specially
			if (childTMCont->ClassID()==IKSLAVE_CLASSID) 
			{
				IKSlaveControl *slave  = (IKSlaveControl*)childTMCont;

				// Setup the reference frame of the child slaves so that they
				// are relative to the new parent.
				Matrix3 tm = cnode->GetNodeTM(t) * Inverse(newTM);
				AffineParts parts;
				decomp_affine(tm, &parts);
				Point3 angles;
				QuatToEuler(parts.q, angles, EULERTYPE_XYZ);
				slave->SetInitPos(parts.t);
				slave->SetInitRot(angles);

				newNode->AttachChild(cnode, FALSE);
			} 
			else 
			{
				newNode->AttachChild(cnode);
			}
		}
	}


	// set up the controller properties
	Control *parCont = parNode->GetTMController();

	if ( parCont && !(parCont->ClassID() == IKSLAVE_CLASSID) ) 
	{
		CopyRefinedJointData ( parCont, newNode->GetTMController(), FALSE );
	}

	// Attach new node to parent if it hasn't been attached yet
	parNode->AttachChild(newNode);
	if ( parentIsGroupMember ) 
	{
		if ( IAssembly* iasm = GetAssemblyInterface(newNode) )
		{
			iasm->SetAssemblyMember(TRUE);
			iasm->SetAssemblyMemberOpen(TRUE);
		}
	}

	// re-align and reset stretch
//	parNode->RealignBoneToChild(t);
//	newNode->RealignBoneToChild(t);
	parNode->ResetBoneStretch(t);
	newNode->ResetBoneStretch(t);

	// See if we're inserting in an IK chain

	if ( ((parCont = parNode->GetTMController()) != NULL) && (parCont->ClassID() == IKSLAVE_CLASSID) ) 
	{
		IKSlaveControl  *slave  = (IKSlaveControl*)parCont;
		IKMasterControl *master = slave->GetMaster();

		// Create a new slave control
		slave = CreateIKSlaveControl(master, newNode);		
		
		// Get the relative TM and decompose it
		Matrix3 tm = newNode->GetNodeTM(t) * Inverse(newNode->GetParentTM(t));
		AffineParts parts;
		decomp_affine(tm, &parts);
		Point3 angles;
		QuatToEuler(parts.q, angles, EULERTYPE_XYZ);
		
		// Init the IK slave using the previous controller's values at the time
		// we're assigning the new chain
		slave->SetInitPos(parts.t);
		slave->SetInitRot(angles);
		slave->SetDOF(5, TRUE);

		// Plug in the slave controller
		newNode->SetTMController(slave); 
	}		

	parNode->InvalidateTreeTM();

	if ( parCont && (parCont->ClassID() == IKSLAVE_CLASSID) ) 
	{
	  CopyRefinedJointData ( parCont, newNode->GetTMController(), TRUE );
	}

	ResumeAnimate();
	theHold.Accept(GetString(IDS_BONE_REFINE));
	GetCOREInterface()->RedrawViews(t);
}	

void BoneObjUserDlgProc::CopyRefinedJointData ( Control* parCont, Control* newCont, BOOL ikSlave )
{
	if ( !parCont || !newCont ) return;

	InitJointData2  ijd [2];
	BOOL            got = FALSE;

	if ( ikSlave )
	{
		got = parCont->GetIKJoints2(ijd, ijd+1);
	}
	else
	{
		for ( int k = 0; k < 2; k++ )
		{
			Control* c = ( k ) ? parCont->GetRotationController() : parCont->GetPositionController();
			if ( !c ) continue;

			JointParams* jp = (JointParams*)(c->GetProperty(PROPID_JOINTPARAMS));
			if ( !jp ) continue;

			got = TRUE;

			for ( int i = 0; i < 3; i++ )
			{
			  ijd[k].active  [i] = jp->Active (i);
			  ijd[k].limit   [i] = jp->Limited(i);
			  ijd[k].ease    [i] = jp->Ease   (i);
			  ijd[k].min     [i] = jp->min    [i];
			  ijd[k].max     [i] = jp->max    [i];
			  ijd[k].damping [i] = jp->damping[i];
			}
		}
	}

	if ( !got )
	{
		return;
	}

	for ( int k = 0; k < 2; k++ )
	{
		for ( int i = 0; i < 3; i++ )
		{
		  ijd[k].preferredAngle[i] = 0.f;

		  if ( ijd[k].min[i] > 0.f ) ijd[k].min[i] = 0.f;
		  if ( ijd[k].max[i] < 0.f ) ijd[k].max[i] = 0.f;
		}
	}

    newCont->InitIKJoints2( ijd, ijd+1 );
}



BOOL PickBoneRefineMode::HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
{
	if ( pickedNode = ip->PickNode(hWnd,m) ) 
	{
		if ( pickedNode->IsGroupMember() && !pickedNode->IsOpenGroupMember() )
		{
			return FALSE;
		}

		Object *nobj = pickedNode->GetObjectRef();		
		if (nobj) nobj = nobj->FindBaseObject();
		if (nobj && nobj->ClassID()==BONE_OBJ_CLASSID) {
			
			// Get the bone object's TM
			Matrix3 otm = pickedNode->GetObjectTM(ip->GetTime());
			Point3 mousePoint(m.x, m.y, 0);

			// Find the two ends of the bone in world space
			BoneObj *bobj = (BoneObj*)nobj;
			float length;
			bobj->pblock2->GetValue(boneobj_length, ip->GetTime(), length, FOREVER);
			Point3 pt0 = otm.GetTrans();
			Point3 pt1 = Point3(length, 0.0f, 0.0f) * otm;
			
			// Transform into screen space
			GraphicsWindow *gw = vpt->getGW();
			gw->setTransform(Matrix3(1));
			gw->transPoint(&pt0, &pt0);
			gw->transPoint(&pt1, &pt1);

			// Don't care about Z
			pt0.z = 0.0f;
			pt1.z = 0.0f;

			// Project mouse point onto bone axis
			Point3 baxis = Normalize(pt1-pt0);
			Point3 mvect = mousePoint-pt0;
			refinePercentage = DotProd(mvect, baxis)/Length(pt1-pt0);

			// We may have missed the bone
			if (refinePercentage < 0.0f || refinePercentage > 1.0f) {
				return FALSE;
				}
			}
	}

	return pickedNode? TRUE: FALSE;
}

BOOL PickBoneRefineMode::Pick(IObjParam *ip,ViewExp *vpt)
	{
	if (pickedNode) BoneObjUserDlgProc::RefineSegment(pickedNode,refinePercentage);
	pickedNode = NULL;
	return FALSE;
	}

void PickBoneRefineMode::EnterMode(IObjParam *ip)
	{
	if (dlg && dlg->iRefine) dlg->iRefine->SetCheck(TRUE);
	}

void PickBoneRefineMode::ExitMode(IObjParam *ip)
	{
	if (dlg && dlg->iRefine) dlg->iRefine->SetCheck(FALSE);
	}

BOOL PickBoneRefineMode::Filter(INode *node)
	{
	Object *nobj = node->GetObjectRef();
	if (nobj) {
		nobj = nobj->FindBaseObject();
		if (nobj && nobj->ClassID()==BONE_OBJ_CLASSID) {
			return TRUE;
			}
		}
	return FALSE;
	}


// AG 03/26/200: Removed along with the removal of the "Refine" button from the modify panel
// "Refine" button was moved to the Character > Bone Adjustment Tools
/*

BOOL BoneObjUserDlgProc::DlgProc(
		TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG:			
			iRefine = GetICustButton(GetDlgItem(hWnd, IDC_BONE_REFINE));
			if (!inCreate) iRefine->Enable();
			else iRefine->Disable();
			iRefine->SetType(CBT_CHECK);
			iRefine->SetHighlightColor(GREEN_WASH);
			break;
		
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_BONE_REFINE:
					GetCOREInterface()->SetPickMode(&refineMode);
					return TRUE;
				}
			break;
		}

	return FALSE;
	}
*/

void BoneObj::BeginEditParams(IObjParam  *ip, ULONG flags, Animatable *prev)
	{
	SimpleObject::BeginEditParams(ip, flags, prev);
	this->ip = ip;
	newBoneDesc.BeginEditParams(ip, this, flags, prev);	
//	boneobj_param_blk.SetUserDlgProc(new BoneObjUserDlgProc(this,(flags&BEGIN_EDIT_CREATE)?TRUE:FALSE));
	}

void BoneObj::EndEditParams(IObjParam *ip, ULONG flags, Animatable *next)
	{
	ip->ClearPickMode();
	SimpleObject::EndEditParams(ip, flags, next);
	this->ip = NULL;
	newBoneDesc.EndEditParams(ip, this, flags, next);
	}

RefTargetHandle BoneObj::Clone(RemapDir& remap)
	{
	BoneObj* newob = new BoneObj(FALSE);
	newob->ReplaceReference(0, pblock2->Clone(remap));
	newob->ivalid.SetEmpty();	
	return newob;
	}

#if 0
int BoneObj::CanConvertToType(Class_ID obtype)
	{
	return 0;
	}

Object* BoneObj::ConvertToType(TimeValue t, Class_ID obtype)
	{
	return NULL;
	}

void BoneObj::GetCollapseTypes(Tab<Class_ID> &clist, Tab<TSTR*> &nlist)
	{
	}
#endif

BOOL BoneObj::HasUVW()
	{
	BOOL res;
	pblock2->GetValue(boneobj_genmap, TimeValue(0), res, FOREVER);
	return res;
	}

void BoneObj::SetGenUVW(BOOL sw)
	{
	if (pblock2) {
		pblock2->SetValue(boneobj_genmap,TimeValue(0),sw);
		}
	}

BOOL BoneObj::OKtoDisplay(TimeValue t)
	{
	return !TestAFlag(A_HIDE_BONE_OBJECT);
	}

void BoneObj::InvalidateUI()
	{
	// if this was caused by a NotifyDependents from pblock2, LastNotifyParamID()
	// will contain ID to update, else it will be -1 => inval whole rollout
	theBonesCreateMode.boneobj_param_blk->InvalidateUI(pblock2->LastNotifyParamID());
	}

static void MakeQuadFace(
		Face &f0, Face &f1, DWORD a, DWORD b, DWORD c, DWORD d, DWORD smooth, MtlID matID)
	{
	f0.setVerts (a, d, b);
	f1.setVerts (c, b, d);
	f0.setSmGroup(smooth);
	f1.setSmGroup(smooth);
	f0.setMatID(matID);
	f1.setMatID(matID);
	f0.setEdgeVisFlags(1, 0, 1);
	f1.setEdgeVisFlags(1, 0, 1);
	}



void BoneObj::BuildFin(
		float size, float startTaper, float endTaper,
		DWORD i0, DWORD i1, DWORD i2, DWORD i3,
		DWORD &curVert, DWORD &curFace)
	{
	// Grab the verts of the face we're building the fin off of
	Point3 v0 = mesh.verts[i0];
	Point3 v1 = mesh.verts[i1];
	Point3 v2 = mesh.verts[i2];
	Point3 v3 = mesh.verts[i3];
		
	// Compute two perpindicular vectors along the face
	Point3 horizDir = ((v3+v2)*0.5f) - ((v1+v0)*0.5f);
	Point3 vertDir  = ((v1+v2)*0.5f) - ((v3+v0)*0.5f);
	
	// Normal
	Point3 normal = Normalize(CrossProd(horizDir, vertDir));
	
	// We'll make the border size be 1/3 the height of the face (on the left side)
	float border = Length(v1-v0)/3.0f;
	if (Length(horizDir) < border * 2.0f) {
		border = Length(horizDir) * 0.5f;
		}
	horizDir = Normalize(horizDir);
	vertDir  = Normalize(vertDir);

	// Interpolate along the top and bottom edge to get 4 points
	Point3 p0 = v0 + Normalize(v3-v0) * border;
	Point3 p1 = v1 + Normalize(v2-v1) * border;
	Point3 p2 = v2 + Normalize(v1-v2) * border;
	Point3 p3 = v3 + Normalize(v0-v3) * border;

	// Now drop down vertically to get the final base points
	Point3 bv0 = p0*2.0f/3.0f + p1/3.0f;
	Point3 bv1 = p1*2.0f/3.0f + p0/3.0f;
	Point3 bv2 = p2*2.0f/3.0f + p3/3.0f;
	Point3 bv3 = p3*2.0f/3.0f + p2/3.0f;
	
	// Save start vert index
	int sv = curVert;

	// We'll need edge vectors to taper the end of the fin
	Point3 topEdge = bv2-bv1;
	Point3 botEdge = bv3-bv0;

	// Add base verts to array
	mesh.setVert(curVert++, bv0);
	mesh.setVert(curVert++, bv1);
	mesh.setVert(curVert++, bv2);
	mesh.setVert(curVert++, bv3);
	
	// Extrude out in the direction of the normal (and taper)
	mesh.setVert(curVert++, bv0 + normal*size + botEdge*startTaper);
	mesh.setVert(curVert++, bv1 + normal*size + topEdge*startTaper);
	mesh.setVert(curVert++, bv2 + normal*size - topEdge*endTaper);
	mesh.setVert(curVert++, bv3 + normal*size - botEdge*endTaper);

	// End
	MakeQuadFace(mesh.faces[curFace], mesh.faces[curFace+1], sv+4, sv+5, sv+6, sv+7, (1<<0), 0);
	curFace += 2;

	// Top
	MakeQuadFace(mesh.faces[curFace], mesh.faces[curFace+1], sv+1, sv+2, sv+6, sv+5, (1<<1), 1);
	curFace += 2;

	// Right side
	MakeQuadFace(mesh.faces[curFace], mesh.faces[curFace+1], sv+0, sv+1, sv+5, sv+4, (1<<2), 2);
	curFace += 2;

	// Left side
	MakeQuadFace(mesh.faces[curFace], mesh.faces[curFace+1], sv+7, sv+6, sv+2, sv+3, (1<<3), 3);
	curFace += 2;

	// Bottom
	MakeQuadFace(mesh.faces[curFace], mesh.faces[curFace+1], sv+4, sv+7, sv+3, sv+0, (1<<4), 4);
	curFace += 2;

	}

#define	ALMOST_ONE (0.999f)

void BoneObj::BuildMesh(TimeValue t)
	{
	float width, height, width2, height2, taper, length;
	float endWidth, endHeight, endWidth2, endHeight2;
	float pyrmidHeight;
	float sfSize, sfStartTaper, sfEndTaper; // side fins
	float ffSize, ffStartTaper, ffEndTaper; // front fin
	float bfSize, bfStartTaper, bfEndTaper; // back fin
	BOOL sideFins, frontFin, backFin, genuv;
	int nverts=9, nfaces=14;

	// Get params from pblock
	ivalid = FOREVER;
	pblock2->GetValue(boneobj_width, t, width, ivalid);
	pblock2->GetValue(boneobj_height, t, height, ivalid);
	pblock2->GetValue(boneobj_taper, t, taper, ivalid);
	pblock2->GetValue(boneobj_length, t, length, ivalid);
	pblock2->GetValue(boneobj_genmap, t, genuv, ivalid);

	// Don't actually allow numerical 100% taper (due to shading artifacts at point)
	if (taper > ALMOST_ONE) taper = ALMOST_ONE;

	pblock2->GetValue(boneobj_sidefins, t, sideFins, ivalid);
	if (sideFins) {
		pblock2->GetValue(boneobj_sidefins_size, t, sfSize, ivalid);
		pblock2->GetValue(boneobj_sidefins_starttaper, t, sfStartTaper, ivalid);
		pblock2->GetValue(boneobj_sidefins_endtaper, t, sfEndTaper, ivalid);
		nverts += 16;
		nfaces += 20;
		if (sfStartTaper + sfEndTaper > ALMOST_ONE) {
			float d = (ALMOST_ONE - sfStartTaper - sfEndTaper) * 0.5f;
			sfStartTaper += d;
			sfEndTaper   += d;
			}
		}
	
	pblock2->GetValue(boneobj_frontfin, t, frontFin, ivalid);
	if (frontFin) {
		pblock2->GetValue(boneobj_frontfin_size, t, ffSize, ivalid);
		pblock2->GetValue(boneobj_frontfin_starttaper, t, ffStartTaper, ivalid);
		pblock2->GetValue(boneobj_frontfin_endtaper, t, ffEndTaper, ivalid);
		nverts += 8;
		nfaces += 10;
		if (ffStartTaper + ffEndTaper > ALMOST_ONE) {
			float d = (ALMOST_ONE - ffStartTaper - ffEndTaper) * 0.5f;
			ffStartTaper += d;
			ffEndTaper   += d;
			}
		}
	
	pblock2->GetValue(boneobj_backfin, t, backFin, ivalid);
	if (backFin) {
		pblock2->GetValue(boneobj_backfin_size, t, bfSize, ivalid);
		pblock2->GetValue(boneobj_backfin_starttaper, t, bfStartTaper, ivalid);
		pblock2->GetValue(boneobj_backfin_endtaper, t, bfEndTaper, ivalid);
		nverts += 8;
		nfaces += 10;
		if (bfStartTaper + bfEndTaper > ALMOST_ONE) {
			float d = (ALMOST_ONE - bfStartTaper - bfEndTaper) * 0.5f;
			bfStartTaper += d;
			bfEndTaper   += d;
			}
		}

	// Compute tapers 'n stuff
	endWidth     = width * (1.0f-taper);
	endHeight    = height * (1.0f-taper);
	width2       = width*0.5f;
	height2      = height*0.5f;
	endWidth2    = endWidth*0.5f;
	endHeight2   = endHeight*0.5f;
	pyrmidHeight = (width2+height2)*0.5f;
	

	mesh.setNumVerts(nverts);
	mesh.setNumFaces(nfaces);
	mesh.setSmoothFlags(0);
	mesh.setNumTVerts (0);
	mesh.setNumTVFaces (0);

	mesh.setVert (0, pyrmidHeight, -height2,  width2);
	mesh.setVert (1, pyrmidHeight,  height2,  width2);
	mesh.setVert (2, pyrmidHeight,  height2, -width2);
	mesh.setVert (3, pyrmidHeight, -height2, -width2);

	mesh.setVert (4, length, -endHeight2,  endWidth2);
	mesh.setVert (5, length,  endHeight2,  endWidth2);
	mesh.setVert (6, length,  endHeight2, -endWidth2);
	mesh.setVert (7, length, -endHeight2, -endWidth2);

	// old way with Z up
	/*	
	mesh.setVert (0, pyrmidHeight, -width2, -height2);
	mesh.setVert (1, pyrmidHeight, -width2,  height2);
	mesh.setVert (2, pyrmidHeight,  width2,  height2);
	mesh.setVert (3, pyrmidHeight,  width2, -height2);

	mesh.setVert (4, length, -endWidth2, -endHeight2);
	mesh.setVert (5, length, -endWidth2,  endHeight2);
	mesh.setVert (6, length,  endWidth2,  endHeight2);
	mesh.setVert (7, length,  endWidth2, -endHeight2);
	*/

	mesh.setVert (8, 0.0f,  0.0f, 0.0f);

	// End
	MakeQuadFace(mesh.faces[0], mesh.faces[1], 4, 5, 6, 7, (1<<0), 0);
	
	// Top
	MakeQuadFace(mesh.faces[2], mesh.faces[3], 1, 2, 6, 5, (1<<1), 1);

	// Right side
	MakeQuadFace(mesh.faces[4], mesh.faces[5], 0, 1, 5, 4, (1<<2), 2);

	// Left side
	MakeQuadFace(mesh.faces[6], mesh.faces[7], 7, 6, 2, 3, (1<<3), 3);

	// Bottom
	MakeQuadFace(mesh.faces[8], mesh.faces[9], 4, 7, 3, 0, (1<<4), 4);

	// Start pyrmid
	mesh.faces[10].setVerts (8, 0, 1);
	mesh.faces[10].setSmGroup(1<<5);
	mesh.faces[10].setMatID(5);
	mesh.faces[10].setEdgeVisFlags(1, 1, 1);

	mesh.faces[11].setVerts (8, 1, 2);
	mesh.faces[11].setSmGroup(1<<6);
	mesh.faces[11].setMatID(5);
	mesh.faces[11].setEdgeVisFlags(1, 1, 1);

	mesh.faces[12].setVerts (8, 2, 3);
	mesh.faces[12].setSmGroup(1<<7);
	mesh.faces[12].setMatID(5);
	mesh.faces[12].setEdgeVisFlags(1, 1, 1);

	mesh.faces[13].setVerts (8, 3, 0);
	mesh.faces[13].setSmGroup(1<<8);
	mesh.faces[13].setMatID(5);
	mesh.faces[13].setEdgeVisFlags(1, 1, 1);

	// This is how many verts and faces we've made so far
	DWORD curVert = 9;
	DWORD curFace = 14;

	// Optionally build fins
	if (sideFins) {
		BuildFin(sfSize, sfStartTaper, sfEndTaper, 0, 1, 5, 4, curVert, curFace);
		BuildFin(sfSize, sfStartTaper, sfEndTaper, 2, 3, 7, 6, curVert, curFace);
		}
	if (frontFin) {
		BuildFin(ffSize, ffStartTaper, ffEndTaper, 1, 2, 6, 5, curVert, curFace);
		}
	if (backFin) {
		BuildFin(bfSize, bfStartTaper, bfEndTaper, 3, 0, 4, 7, curVert, curFace);
		}

	if (genuv) {
		mesh.ApplyUVWMap(MAP_FACE,
			1.0f, 1.0f, 1.0f, // tile
			FALSE, FALSE, FALSE, // flip
			FALSE, // cap
			Matrix3(1));
		}

	// Invalidate caches
	mesh.InvalidateTopologyCache();
	}

void BoneObj::RescaleWorldUnits(float f)
{
  if (TestAFlag(A_WORK1)) return;

  SimpleObject2::RescaleWorldUnits(f);
  DbgAssert(TestAFlag(A_WORK1));

  DependentIterator iter(this);
  for (ReferenceMaker* mk = iter.Next(); mk != NULL; mk = iter.Next()) {
	if (mk->SuperClassID() == BASENODE_CLASS_ID) break;
  }
  if (mk != NULL) {
	INode* node = (INode*)mk;
	TimeValue t = GetCOREInterface()->GetTime();
	node->ResetBoneStretch(t);
  }
}

class PickBoneRefineMode;
static PickBoneRefineMode thePickBoneMode;

BOOL BoneFunctionPublish::RefineBone()
	{		
		GetCOREInterface()->SetPickMode(&thePickBoneMode);
		return FALSE;

}

#endif // NO_OBJECT_BONE