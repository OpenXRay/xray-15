/**********************************************************************
 *<
	FILE: BoolObj.cpp

	DESCRIPTION:  A Boolean object

	CREATED BY: Rolf Berteig

	HISTORY: created 21 August 1995

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "prim.h"

#ifndef NO_OBJECT_BOOL

#include "buildver.h"
#include "istdplug.h"
#include "MaxIcon.h"

#define CID_CREATEBOOLMODE		0x7F26A6B1

#define REF_OP1		0
#define REF_OP2		1
#define REF_CONT1	2
#define REF_CONT2	3

// Flag bits
#define BOOL_OB1SEL			(1<<0)
#define BOOL_OB2SEL			(1<<1)
#define BOOL_ANYSEL			(BOOL_OB1SEL|BOOL_OB2SEL)

#define BOOL_DISPRESULT		(1<<2)
#define BOOL_DISPHIDDEN		(1<<12)

#define BOOL_UPDATEALWAYS	(1<<3)
#define BOOL_UPDATERENDER	(1<<4)
#define BOOL_UPDATEMANUAL	(1<<5)
#define BOOL_UPDATESELECT	(1<<11)

#define BOOL_UNION			(1<<6)
#define BOOL_INTERSECTION	(1<<7)
#define BOOL_DIFFERENCEA	(1<<8)
#define BOOL_DIFFERENCEB	(1<<9)
#define BOOL_OPERATION		(BOOL_UNION|BOOL_INTERSECTION|BOOL_DIFFERENCEA|BOOL_DIFFERENCEB)

#define BOOL_OPTIMIZE		(1<<10)

#define BOOL_FIRSTUPDATE	(1<<13)

#define BOOL_INRENDER		(1<<14)

#define BOOL_ABORTED		(1<<15)

#define BOOL_NEEDSUPDATE	(1<<16)


// When more than this number of faces is being booleaned an hour glass
// cursor is put up.
#define LOTSOFACES	2000

static GenSubObjType SOT_Operands(20);

class PickOperand;

class BoolObject : public IBoolObject, public MeshOpProgress {
		
		RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message );
				
	public:
		TSTR opaName, opbName;
		Object *ob1, *ob2;
		Control *tm1, *tm2;
		Matrix3 obOffset;
		DWORD flags;
		Interval ivalid;
		Mesh mesh;		

		static IObjParam *ip;
		static HWND hParams1;
		static HWND hParams2;		
		static int addOppMethod;
		static Matrix3 createTM;
		static PickOperand pickCB;
		static BOOL creating;
		static BoolObject *editOb;
		static DWORD lastOp;
		static int extractCopy;
		static MoveModBoxCMode *moveMode;
		static RotateModBoxCMode *rotMode;
		static UScaleModBoxCMode *uscaleMode;
		static NUScaleModBoxCMode *nuscaleMode;
		static SquashModBoxCMode *squashMode;
		static SelectModBoxCMode *selectMode;

		BoolObject();
		~BoolObject();
		
		void SetupUI1();
		void SetupUI2(BOOL useName=FALSE);
		void SetExtractButtonState();

		void SetFlag(DWORD mask) { flags|=mask; }
		void ClearFlag(DWORD mask) { flags &= ~mask; }
		int TestFlag(DWORD mask) { return(flags&mask?1:0); }
		DWORD BoolOp(int &order);

		void SetOperandA (TimeValue t, INode *node);
		void SetOperandB (TimeValue t, INode *node, INode *boolNode,
			int addOpMethod=0, int matMergeMethod=0, bool *canUndo=NULL);
		BOOL UpdateMesh(TimeValue t,BOOL force=FALSE,BOOL sel=FALSE);
		Object *GetPipeObj(TimeValue t,int which);
		Matrix3 GetOpTM(TimeValue t,int which,Interval *iv=NULL);
		void Invalidate() {ivalid.SetEmpty();}
		void ExtractOperand(int which);

		// From BaseObject
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}
		TCHAR *GetObjectName() { return GetString(IDS_RB_BOOLEAN); }

		// For sub-object selection
		void Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE);
		void Rotate( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin=FALSE);
		void Scale( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE);
		
		int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext* mc);
		
		void SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert);
		void ClearSelection(int selLevel);

		int SubObjectIndex(HitRecord *hitRec);
		void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
		void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);

		void ActivateSubobjSel(int level, XFormModes& modes);
	
		// NS: New SubObjType API
		int NumSubObjTypes();
		ISubObjType *GetSubObjType(int i);

		// From Object		
		void InitNodeName(TSTR& s) {s = GetString(IDS_RB_BOOLEAN);}
		Interval ObjectValidity(TimeValue t);
		int CanConvertToType(Class_ID obtype);
		Object* ConvertToType(TimeValue t, Class_ID obtype);
        BOOL PolygonCount(TimeValue t, int& numFaces, int& numVerts);
		ObjectState Eval(TimeValue time);
		int NumPipeBranches(bool selected);
		Object *GetPipeBranch(int i, bool selected);
		INode *GetBranchINode(TimeValue t,INode *node,int i, bool selected);
		int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
		void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel);
		void GetLocalBoundBox(TimeValue t, INode *inode,ViewExp* vpt, Box3& box);
		void GetWorldBoundBox(TimeValue t, INode *inode, ViewExp* vpt, Box3& box);

		// From GeomObject		
		ObjectHandle CreateTriObjRep(TimeValue t) {return NULL;}
		int IntersectRay(TimeValue t, Ray& r, float& at, Point3& norm);
		Mesh* GetRenderMesh(TimeValue t, INode *inode, View& view, BOOL& needDelete);

		// Animatable methods
		Class_ID ClassID() {return Class_ID(BOOLOBJ_CLASS_ID,0);}  
		void GetClassName(TSTR& s) {s = GetString(IDS_RB_BOOLEAN_CLASS);}
		void DeleteThis() {delete this;}				
		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);
		int RenderBegin(TimeValue t, ULONG flags);
		int RenderEnd(TimeValue t);

		int NumSubs();
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int subNum);

		// From ref
		RefTargetHandle Clone(RemapDir& remap);
		int NumRefs() {return 4;}
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		// From MeshOpProgress
		void Init(int total);
		BOOL Progress(int p);

		// From IBoolObject
		BOOL GetOperandSel(int which);
		void SetOperandSel(int which,BOOL sel);
		int GetBoolOp();
		void SetBoolOp(int op);
		int GetBoolCutType () { return 0; }
		void SetBoolCutType (int ct) { }
		BOOL GetDisplayResult();
		void SetDisplayResult(BOOL onOff);
		BOOL GetShowHiddenOps();
		void SetShowHiddenOps(BOOL onOff);
		int GetUpdateMode();
		void SetUpdateMode(int mode);
		BOOL GetOptimize();
		void SetOptimize(BOOL onOff);
	};				

class PickOperand : 
		public PickModeCallback,
		public PickNodeCallback {
	public:		
		BoolObject *bo;
		
		PickOperand() {bo=NULL;}

		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);

		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);

		BOOL Filter(INode *node);

		BOOL RightClick(IObjParam *ip,ViewExp *vpt)	{return TRUE;}
		
		PickNodeCallback *GetFilter() {return this;}
	};

IObjParam *BoolObject::ip                   = NULL;
HWND BoolObject::hParams1                   = NULL;
HWND BoolObject::hParams2                   = NULL;
int BoolObject::addOppMethod                = IDC_TARG_MOVE;
DWORD BoolObject::lastOp                    = BOOL_DIFFERENCEA;
BOOL BoolObject::creating                   = FALSE;
BoolObject *BoolObject::editOb              = NULL;
MoveModBoxCMode*    BoolObject::moveMode    = NULL;
RotateModBoxCMode*  BoolObject::rotMode 	= NULL;
UScaleModBoxCMode*  BoolObject::uscaleMode  = NULL;
NUScaleModBoxCMode* BoolObject::nuscaleMode = NULL;
SquashModBoxCMode*  BoolObject::squashMode  = NULL;
SelectModBoxCMode*  BoolObject::selectMode  = NULL;
Matrix3 BoolObject::createTM;
PickOperand BoolObject::pickCB;
int BoolObject::extractCopy = FALSE;

class BoolObjClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return FALSE;}
	void *			Create(BOOL loading = FALSE) {return new BoolObject;}
	const TCHAR *	ClassName() { return GetString(IDS_RB_BOOLEAN_CLASS); }
	SClass_ID		SuperClassID() { return GEOMOBJECT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(BOOLOBJ_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_COMPOUNDOBJECTS);}
	BOOL			OkToCreate(Interface *i);
	int 			BeginCreate(Interface *i);
	int 			EndCreate(Interface *i);
	void			ResetClassParams(BOOL fileReset);
	};

void BoolObjClassDesc::ResetClassParams(BOOL fileReset)
	{
	BoolObject::addOppMethod = IDC_TARG_MOVE;
	BoolObject::lastOp       = BOOL_DIFFERENCEA;
	}

BOOL BoolObjClassDesc::OkToCreate(Interface *i)
	{
	if (i->GetSelNodeCount()!=1) return FALSE;
	
	ObjectState os = i->GetSelNode(0)->GetObjectRef()->Eval(i->GetTime());
	if (os.obj->IsParticleSystem() ||
		os.obj->SuperClassID()!=GEOMOBJECT_CLASS_ID) {
		return FALSE;
		}

	return TRUE;	
	}

static BoolObjClassDesc boolObjDesc;

ClassDesc* GetBoolObjDesc() { return &boolObjDesc; }

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

//----------------------------------------------------------------------

class CreateBoolProc : public MouseCallBack {
	public:
		IObjParam *ip;
		void Init(IObjParam *i) {ip=i;}
		int proc( 
			HWND hWnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
	};

int CreateBoolProc::proc( 
		HWND hWnd, 
		int msg, 
		int point, 
		int flags, 
		IPoint2 m ) 
	{
	switch (msg) {
		case MOUSE_POINT:
			ip->SetActiveViewport(hWnd);
			break;
		case MOUSE_FREEMOVE:
			SetCursor(LoadCursor(NULL,IDC_ARROW));
			break;
// mjm - 3.1.99
		case MOUSE_PROPCLICK:
			// right click while between creations
			ip->RemoveMode(NULL);
			break;
// mjm - end
		}	
	return TRUE;
	}

class CreateBoolMode : public CommandMode, ReferenceMaker {		
	public:		
		CreateBoolProc proc;
		INode *node, *svNode;
		IObjParam *ip;
		BoolObject *obj;

		void Begin(INode *n,IObjParam *i);
		void End(IObjParam *i);
		void JumpStart(IObjParam *i,BoolObject *o);

		int Class() {return CREATE_COMMAND;}
		int ID() { return CID_CREATEBOOLMODE; }
		MouseCallBack *MouseProc(int *numPoints) {*numPoints = 1; return &proc;}
		ChangeForegroundCallback *ChangeFGProc() {return CHANGE_FG_SELECTED;}
		BOOL ChangeFG(CommandMode *oldMode) {return TRUE;}
		void EnterMode() {/*MakeRefByID(FOREVER,0,svNode);*/}
		void ExitMode() {/*DeleteAllRefsFromMe();*/}
		
		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i) {return node;}
		void SetReference(int i, RefTargetHandle rtarg) {node = (INode*)rtarg;}
	    RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	    	PartID& partID,  RefMessage message);		
	};
static CreateBoolMode theCreateBoolMode;

RefResult CreateBoolMode::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
		PartID& partID,
		RefMessage message)
	{
	switch (message) {		
		case REFMSG_TARGET_SELECTIONCHANGE:		
		case REFMSG_TARGET_DELETED:			
			if (ip) ip->StopCreating();
			break;

		}
	return REF_SUCCEED;
	}

class CreateBoolRestore : public RestoreObj {
	public:   		
		void Restore(int isUndo) {
			if (theCreateBoolMode.ip) {
				// Jump out of boolean create mode.
				theCreateBoolMode.ip->SetStdCommandMode(CID_OBJMOVE);
				}
			}	
		void Redo() {}
		TSTR Description() {return TSTR(_T("Create Boolean"));}
	};

// Sending the REFMSG_NOTIFY_PASTE message notifies the modify
// panel that the Node's object reference has changed when
// undoing or redoing.
class CreateBoolNotify : public RestoreObj {
	public:   		
		BoolObject *obj;
		BOOL which;
		CreateBoolNotify(BoolObject *o, BOOL w) {
			obj = o; which = w;
			}
		void Restore(int isUndo) {
			if (which) {
				obj->NotifyDependents(FOREVER,0,REFMSG_NOTIFY_PASTE);
				}
			}	
		void Redo() {
			if (!which) {
				obj->NotifyDependents(FOREVER,0,REFMSG_NOTIFY_PASTE);
				}
			}
		TSTR Description() {return TSTR(_T("Create Bool Notify"));}
	};

void CreateBoolMode::Begin(INode *n,IObjParam *i) 
	{
	MakeRefByID(FOREVER,0,n);
	svNode = node;
	assert(node);
	ip = i;
	proc.Init(ip);

	theHold.Begin();
	theHold.Put(new CreateBoolRestore);

	obj = new BoolObject;

	theHold.Put(new CreateBoolNotify(obj,1));

	obj->createTM = node->GetObjectTM(i->GetTime());
	obj->SetOperandA (i->GetTime(), node);
	node->SetObjectRef(obj);
	
	theHold.Put(new CreateBoolNotify(obj,0));

	obj->BeginEditParams(i,BEGIN_EDIT_CREATE,NULL);
	}

void CreateBoolMode::End(IObjParam *i)
	{
	svNode = node;
	if (obj) obj->EndEditParams(i,END_EDIT_REMOVEUI,NULL);
	DeleteAllRefsFromMe();
	ip  = NULL;
	obj = NULL;
	}

void CreateBoolMode::JumpStart(IObjParam *i,BoolObject *o)
	{
	ip  = i;
	obj = o;
	//MakeRefByID(FOREVER,0,svNode);
	obj->BeginEditParams(i,BEGIN_EDIT_CREATE,NULL);
	}


int BoolObjClassDesc::BeginCreate(Interface *i)
	{	
	SuspendSetKeyMode();
	assert(i->GetSelNodeCount()==1);

	theCreateBoolMode.Begin(i->GetSelNode(0),(IObjParam*)i);
	i->PushCommandMode(&theCreateBoolMode);
	return TRUE;
	}

int BoolObjClassDesc::EndCreate(Interface *i)
	{
	ResumeSetKeyMode();
	theCreateBoolMode.End((IObjParam*)i);
	i->RemoveMode(&theCreateBoolMode);
	return TRUE;
	}

//----------------------------------------------------------------------


BoolObject::BoolObject()
	{	
	obOffset  = Matrix3(1);
	ob1 = ob2 = NULL;
	tm1 = tm2 = NULL;
	flags = 0;
	ivalid.SetEmpty();
	SetFlag(BOOL_UPDATEALWAYS|BOOL_DISPRESULT|
		BOOL_OPTIMIZE|BOOL_FIRSTUPDATE|lastOp);
	}

BoolObject::~BoolObject()
	{
	DeleteAllRefsFromMe();
	}


DWORD BoolObject::BoolOp(int &order) 
	{
	order = 0;
	switch (flags&BOOL_OPERATION) {
		case BOOL_UNION: 		return MESHBOOL_UNION;
		case BOOL_INTERSECTION:	return MESHBOOL_INTERSECTION;
		case BOOL_DIFFERENCEA:	return MESHBOOL_DIFFERENCE;
		case BOOL_DIFFERENCEB:
			order = 1;
			return MESHBOOL_DIFFERENCE;			
		}
	return 0;
	}

void BoolObject::ExtractOperand(int which)
	{
	if (creating) return;

	// Compute a node TM for the new object
	assert(ip);
	ModContextList list;
	INodeTab nodes;	
	ip->GetModContexts(list,nodes);
	Matrix3 tm = nodes[0]->GetObjectTM(ip->GetTime());
	Matrix3 tmOp = GetOpTM(ip->GetTime(),which);
	Object *obj = which ? ob2 : ob1;
	if (!obj) return;
	tm = tmOp * tm;

	// Clone the object if specified
	if (extractCopy) obj = (Object*)obj->Clone();

	// Create the new node
	INode *node = ip->CreateObjectNode(obj);

	// Set the node TM.
	SuspendAnimate();
	AnimateOff();
	node->SetNodeTM(0,tm);
	ResumeAnimate();

	nodes.DisposeTemporary();
	}


class SetOperandRestore : public RestoreObj {	
	public:
		BoolObject *bo;
		TSTR uname, rname;
		int which;

		SetOperandRestore(BoolObject *b,int w) 
			{bo=b;uname=(w?bo->opbName:bo->opaName);}   		
		void Restore(int isUndo) {
			if (which) {
				rname = bo->opbName;
				bo->opbName = uname;
			} else {
				rname = bo->opaName;
				bo->opaName = uname;
				}
			if (bo->hParams2 && bo->editOb==bo) bo->SetupUI2(TRUE);
			}
		void Redo() {
			if (which) {				
				bo->opbName = rname;
			} else {				
				bo->opaName = rname;
				}
			if (bo->hParams2 && bo->editOb==bo) bo->SetupUI2(TRUE);
			}		
	};

void BoolObject::SetOperandA (TimeValue t, INode *node) {
	Object *obj = node->GetObjectRef();
	theHold.Put(new SetOperandRestore(this,0));
	opaName = TSTR(_T("B_")) + TSTR(node->GetName());

	// Plug in the object
	ReplaceReference (0, obj);
	// Make a new controller
	ReplaceReference (2, NewDefaultMatrix3Controller());	

	ivalid.SetEmpty();
	theHold.Accept(GetString(IDS_RB_PICKOPERAND));
}

void BoolObject::SetOperandB (TimeValue t, INode *node, INode *boolNode,
							 int addOpMethod, int matMergeMethod, bool *canUndo) {
	BOOL delnode = FALSE;
	Matrix3 oppTm = node->GetObjectTM(t);
	Matrix3 boolTm = boolNode->GetObjectTM(t);
	Object *obj = node->GetObjectRef();

	switch (addOppMethod) {
	case IDC_TARG_REFERENCE:
		obj = MakeObjectDerivedObject(obj);
		break;
	case IDC_TARG_COPY:
		obj = (Object*)obj->Clone();
		break;
	case IDC_TARG_MOVE:
		delnode = TRUE;
		break;
	}
	
	theHold.Put(new SetOperandRestore(this,1));

	opbName = TSTR(_T("B_")) + TSTR(node->GetName());

	// Plug in the object
	ReplaceReference(1,obj);
	
	// Grab the TM controller from the node and make a copy of it
	Control *opCont = node->GetTMController();
	opCont          = (Control*)opCont->Clone();
	
	// Get the object offset
	obOffset = node->GetObjTMBeforeWSM(t) * Inverse(node->GetNodeTM(t));

	// Adjust the trasform to be relative to us.
	opCont->ChangeParents(t,node->GetParentTM(t),boolTm,oppTm);
	ReplaceReference(3,opCont);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);

	ivalid.SetEmpty();
	if (delnode) ip->DeleteNode(node);
	
	theHold.Accept(GetString(IDS_RB_PICKOPERAND));
}		

Object *BoolObject::GetPipeObj(TimeValue t,int which)
	{
	ObjectState os;
	if (which==0) {
		if (ob1) {
			os = ob1->Eval(t);
			return os.obj;
		} else {
			return NULL;
			}
	} else {
		if (ob2) {
			os = ob2->Eval(t);
			return os.obj;
		} else {
			return NULL;
			}
		}
	return os.obj;
	}

Matrix3 BoolObject::GetOpTM(TimeValue t,int which,Interval *iv)
	{
	Matrix3 tm(1);
	Interval valid, *v;
	if (iv) v = iv;
	else v = &valid;

	if (which==0) {
		if (tm1) {
			tm1->GetValue(t,&tm,*v,CTRL_RELATIVE);
			}
	} else {
		if (tm2) {			
			tm2->GetValue(t,&tm,*v,CTRL_RELATIVE);
			tm = obOffset * tm;
			}
		}
	return tm;
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

int BoolObject::RenderBegin(TimeValue t, ULONG flags)
	{
	SetFlag(BOOL_INRENDER);
	if (TestFlag(BOOL_UPDATERENDER)) {
		ivalid.SetEmpty();
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		}
	return 0;
	}

int BoolObject::RenderEnd(TimeValue t)
	{
	ClearFlag(BOOL_INRENDER);	
	return 0;
	}

void BoolObject::Init(int total)
	{
	}

BOOL BoolObject::Progress(int p)
	{
	SHORT res = GetAsyncKeyState(VK_ESCAPE);
	if (res&1) {
		SetFlag(BOOL_ABORTED);
		return FALSE;
		}
	else return TRUE;
	}

BOOL BoolObject::UpdateMesh(TimeValue t,BOOL force,BOOL sel)
	{
	if ((!ivalid.InInterval(t) || TestFlag(BOOL_NEEDSUPDATE)) &&
		(TestFlag(BOOL_UPDATEALWAYS) || 
		(TestFlag(BOOL_UPDATESELECT)&sel) || 
		(TestFlag(BOOL_UPDATERENDER)&&TestFlag(BOOL_INRENDER)) ||
		force)) {
		
		ClearFlag(BOOL_NEEDSUPDATE);

		// Build the boolean result mesh
		ivalid = FOREVER;
		BOOL needsDel1, needsDel2;

		TriObject *tob1, *tob2;
		tob1 = GetTriObject(t,ob1,ivalid,needsDel1);
		tob2 = GetTriObject(t,ob2,ivalid,needsDel2);

		if (tob1 && tob2) {
			Matrix3 mat1 = GetOpTM(t,0,&ivalid);
			Matrix3 mat2 = GetOpTM(t,1,&ivalid);
			int order, op, res, faces = tob1->GetMesh().getNumFaces() + tob2->GetMesh().getNumFaces();
			HCURSOR hCur;
			if (faces > LOTSOFACES) hCur = SetCursor(LoadCursor(NULL,IDC_WAIT));
			GetAsyncKeyState(VK_ESCAPE);
			ClearFlag(BOOL_ABORTED);
			op = BoolOp(order);
			if (order) {
				res = CalcBoolOp(
					mesh, tob2->GetMesh(), tob1->GetMesh(),
					op, this, &mat2, &mat1, -1, TestFlag(BOOL_OPTIMIZE));
			} else {
				res = CalcBoolOp(
					mesh, tob1->GetMesh(), tob2->GetMesh(),
					op, this, &mat1, &mat2, -1, TestFlag(BOOL_OPTIMIZE));
				}
			if (faces > LOTSOFACES) SetCursor(hCur);

			if (!res) {
				// Boolean Failed!!!
				if (ip) {
					if (TestFlag(BOOL_ABORTED)) {
						ip->DisplayTempPrompt(GetString(IDS_RB_BOOLEANABORTED),500);
					} else {
						ip->DisplayTempPrompt(GetString(IDS_RB_INVALIDBOOLEAN),500);
						}
					}
				if (TestFlag(BOOL_ABORTED)) {
					// Put in manual update mode.
					ClearFlag(BOOL_UPDATEALWAYS|BOOL_UPDATERENDER|BOOL_UPDATEMANUAL|BOOL_UPDATESELECT);
					SetFlag(BOOL_UPDATEMANUAL);					
					if (ip && editOb==this) SetupUI2();
					}
				if (TestFlag(BOOL_FIRSTUPDATE)) {
					// Turn on show operands.
					ClearFlag(BOOL_DISPRESULT);
					}
				
				//RB: 2/29/96:
				// If the boolean op failed don't set the validity to be empty.
				// this will just cause it to re-evaluate over and over again.
				// When the user changes it in some way which may allow the boolen
				// to succeed, that change should invalidate the interval.
				//ivalid.SetEmpty();
			} else {
				if (ip) ip->DisplayTempPrompt(GetString(IDS_RB_BOOLEACOMPLETED),500);
				ClearFlag(BOOL_FIRSTUPDATE);
				mesh.InvalidateEdgeList();
				mesh.InvalidateGeomCache();
				mesh.InvalidateTopologyCache();
				}
			}
		
		//if (needsDel1 && tob1->IsObjectLocked()==0) tob1->DeleteThis();
		//if (needsDel2 && tob2->IsObjectLocked()==0) tob2->DeleteThis();
		if (needsDel1) tob1->DeleteThis();
		if (needsDel2) tob2->DeleteThis();
	} else {
		// RB 4-2-96: We must be set to manual update.
		// The object should be considered valid
		if (!ivalid.InInterval(t)) {			
			ivalid.SetInstant(t);
			SetFlag(BOOL_NEEDSUPDATE);
			}		
		}

	return ivalid.InInterval(t);
	}

Interval BoolObject::ObjectValidity(TimeValue t)
	{
	UpdateMesh(t);
	if (ivalid.Empty()) return Interval(t,t);
	else return ivalid;
	}

int BoolObject::CanConvertToType(Class_ID obtype)
	{
	if (obtype==defObjectClassID||obtype==triObjectClassID||obtype==mapObjectClassID) {
		if (ob1 && ob2) return 1;
		else if (ob1) return ob1->CanConvertToType(obtype);
		else if (ob2) return ob2->CanConvertToType(obtype);
		else return 0;
		}
	return Object::CanConvertToType(obtype);
	}

Object* BoolObject::ConvertToType(TimeValue t, Class_ID obtype)
	{
	if (obtype==defObjectClassID||obtype==triObjectClassID||obtype==mapObjectClassID) {
		if (ob1 && ob2) {
			TriObject *triob;
			UpdateMesh(t);
			triob = CreateNewTriObject();
			triob->GetMesh() = mesh;
			triob->SetChannelValidity(TOPO_CHAN_NUM,ObjectValidity(t));
			triob->SetChannelValidity(GEOM_CHAN_NUM,ObjectValidity(t));
			return triob;
		} else {
			// RB 4-11-96:
			// There was a bug where if one of the operands was a tri-object
			// it would convert itself to a tri-object by just returning itself (as it should)
			// The problem is that there are places in the system where the
			// the system would think it needed to delete the tri-object
			// becuase it was not equal to the boolean object. In other words,
			// it thinks that the boolean convert itself to a tri-object and
			// therefore the tri-object was a temporary object.
			// So what this code does is clone the tri-object in this case
			// so that the boolean object will always return a temporary
			// object.
			Object *obj = NULL;
			if (ob1) {
				obj = ob1->ConvertToType(t,obtype);
				if (obj && (obj==ob1 || obj->IsObjectLocked())) {
					return (Object*)obj->Clone();
				} else {
					return obj;
					}
				}
			if (ob2) {
				obj = ob2->ConvertToType(t,obtype);
				if (obj && (obj==ob2 || obj->IsObjectLocked())) {
					return (Object*)obj->Clone();
				} else {
					return obj;
					}
				}			
			return NULL;
			}
	} else {
		return Object::ConvertToType(t,obtype);
		}
	}

BOOL 
BoolObject::PolygonCount(TimeValue t, int& numFaces, int& numVerts) 
{
    UpdateMesh(t);
    numFaces = mesh.getNumFaces();
    numVerts = mesh.getNumVerts();
    return TRUE;
}

ObjectState BoolObject::Eval(TimeValue time)
	{
	return ObjectState(this);
	}


void BoolObject::SetupUI1()
	{
	CheckRadioButton(hParams1,IDC_TARG_REFERENCE,IDC_TARG_INSTANCE,addOppMethod);
	}

void BoolObject::SetupUI2(BOOL useName)
	{	
	HWND hList = GetDlgItem(hParams2,IDC_BOOL_OPERANDS);
	SendMessage(hList,LB_RESETCONTENT,0,0);
	TSTR name = TSTR(_T("A: ")) + ((ob1||useName) ? opaName : TSTR(_T("")));
	SendMessage(hList,LB_ADDSTRING,0,(LPARAM)(const TCHAR*)name);
	name = TSTR(_T("B: ")) + ((ob2||useName) ? opbName : TSTR(_T("")));
	SendMessage(hList,LB_ADDSTRING,0,(LPARAM)(const TCHAR*)name);
	if (flags&BOOL_OB1SEL) {
		SendMessage(hList,LB_SETSEL,TRUE,0);
		}
	if (flags&BOOL_OB2SEL) {
		SendMessage(hList,LB_SETSEL,TRUE,1);
		}

	ICustEdit *edit = GetICustEdit(GetDlgItem(hParams2,IDC_BOOL_ANAME));
	edit->SetText(opaName);
	ReleaseICustEdit(edit);

	edit = GetICustEdit(GetDlgItem(hParams2,IDC_BOOL_BNAME));
	edit->SetText(opbName);
	ReleaseICustEdit(edit);

	int order;
	switch (BoolOp(order)) {
		case MESHBOOL_UNION:
			CheckRadioButton(hParams2,IDC_BOOL_UNION,IDC_BOOL_DIFFERENCEBA,IDC_BOOL_UNION);
			break;
		case MESHBOOL_INTERSECTION:
			CheckRadioButton(hParams2,IDC_BOOL_UNION,IDC_BOOL_DIFFERENCEBA,IDC_BOOL_INTERSECTION);
			break;
		case MESHBOOL_DIFFERENCE:
			if (order) {
				CheckRadioButton(hParams2,IDC_BOOL_UNION,IDC_BOOL_DIFFERENCEBA,IDC_BOOL_DIFFERENCEBA);
			} else {
				CheckRadioButton(hParams2,IDC_BOOL_UNION,IDC_BOOL_DIFFERENCEBA,IDC_BOOL_DIFFERENCEAB);
				}
			break;
		}	
	
	CheckDlgButton(hParams2,IDC_BOOL_DISPRESULT,TestFlag(BOOL_DISPRESULT));
	CheckDlgButton(hParams2,IDC_BOOL_DISPOPS,!TestFlag(BOOL_DISPRESULT));	

	if (TestFlag(BOOL_UPDATERENDER)) CheckDlgButton(hParams2,IDC_BOOL_UPDATERENDER,TRUE);
	else CheckDlgButton(hParams2,IDC_BOOL_UPDATERENDER,FALSE);
	if (TestFlag(BOOL_UPDATEMANUAL)) CheckDlgButton(hParams2,IDC_BOOL_UPDATEMANUAL,TRUE);
	else CheckDlgButton(hParams2,IDC_BOOL_UPDATEMANUAL,FALSE);
	if (TestFlag(BOOL_UPDATEALWAYS)) CheckDlgButton(hParams2,IDC_BOOL_UPDATEALWAYS,TRUE);
	else CheckDlgButton(hParams2,IDC_BOOL_UPDATEALWAYS,FALSE);
	if (TestFlag(BOOL_UPDATESELECT)) CheckDlgButton(hParams2,IDC_BOOL_UPDATESELECT,TRUE);
	else CheckDlgButton(hParams2,IDC_BOOL_UPDATESELECT,FALSE);	
			
	CheckDlgButton(hParams2,IDC_BOOL_OPTIMIZE,TestFlag(BOOL_OPTIMIZE));
	CheckDlgButton(hParams2,IDC_BOOL_SHOWHIDDENOPS,TestFlag(BOOL_DISPHIDDEN));

	ICustButton *iBut = GetICustButton(GetDlgItem(hParams2,IDC_BOOL_RECALC));
	if (!TestFlag(BOOL_UPDATEALWAYS)) {
		iBut->Enable();
	} else {
		iBut->Disable();
		}
	ReleaseICustButton(iBut);		
	
	SetExtractButtonState();
	}

void BoolObject::SetExtractButtonState()
	{
	ICustButton *iBut = GetICustButton(GetDlgItem(hParams2,IDC_BOOL_EXTRACTOP));
	if (!creating && (flags & (BOOL_OB1SEL|BOOL_OB2SEL))) {
		iBut->Enable();
		EnableWindow(GetDlgItem(hParams2,IDC_BOOL_EXTRACT_INTANCE),TRUE);
		EnableWindow(GetDlgItem(hParams2,IDC_BOOL_EXTRACT_COPY),TRUE);
	} else {
		iBut->Disable();
		EnableWindow(GetDlgItem(hParams2,IDC_BOOL_EXTRACT_INTANCE),FALSE);
		EnableWindow(GetDlgItem(hParams2,IDC_BOOL_EXTRACT_COPY),FALSE);
		}
	ReleaseICustButton(iBut);	

	CheckDlgButton(hParams2,IDC_BOOL_EXTRACT_INTANCE,!extractCopy);
	CheckDlgButton(hParams2,IDC_BOOL_EXTRACT_COPY,extractCopy);
	}

BOOL PickOperand::Filter(INode *node)
	{
	if (node) {
		ObjectState os = node->GetObjectRef()->Eval(bo->ip->GetTime());
		if (os.obj->IsParticleSystem() || 
			os.obj->SuperClassID()!=GEOMOBJECT_CLASS_ID) {
			node = NULL;
			return FALSE;
			}

		node->BeginDependencyTest();
		bo->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
		if(node->EndDependencyTest()) {
			node = NULL;
			return FALSE;
			}		
		}

	return node ? TRUE : FALSE;
	}

BOOL PickOperand::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
	{	
	INode *node = ip->PickNode(hWnd,m,this);
	
	if (node) {
		ObjectState os = node->GetObjectRef()->Eval(ip->GetTime());
		if (os.obj->SuperClassID()!=GEOMOBJECT_CLASS_ID) {
			node = NULL;
			return FALSE;
			}

		node->BeginDependencyTest();
		bo->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
		if(node->EndDependencyTest()) {
			node = NULL;
			return FALSE;
			}		
		}

	return node ? TRUE : FALSE;
	}

BOOL PickOperand::Pick(IObjParam *ip,ViewExp *vpt)
	{
	INode *node = vpt->GetClosestHit();
	assert(node);

	ModContextList mcList;
	INodeTab nodes;
	INode *ourNode = NULL;
	if (!bo->creating) {
		// Grab the node for the first intance of us.
		ip->GetModContexts(mcList,nodes);
		if (nodes.Count()) ourNode = nodes[0];
	}

	theHold.Begin();
	bo->SetOperandB (ip->GetTime(), node, ourNode, 1);		
	theHold.Accept(IDS_DS_CREATE);

	nodes.DisposeTemporary ();

	// Automatically check show result and do one update
	bo->SetFlag(BOOL_DISPRESULT);
	bo->SetFlag(BOOL_FIRSTUPDATE);
	CheckRadioButton(bo->hParams2,IDC_BOOL_DISPRESULT,IDC_BOOL_DISPOPS,IDC_BOOL_DISPRESULT);
	bo->UpdateMesh(ip->GetTime(),TRUE);
	bo->SetupUI2();	
	
	if (bo->creating) {
		theCreateBoolMode.JumpStart(ip,bo);
		ip->SetCommandMode(&theCreateBoolMode);
		ip->RedrawViews(ip->GetTime());
		return FALSE;
	} else {
		return TRUE;
	}
}

void PickOperand::EnterMode(IObjParam *ip)
	{
	ICustButton *iBut = GetICustButton(GetDlgItem(bo->hParams1,IDC_PICK_BOOLOPERAND));
	if (iBut) iBut->SetCheck(TRUE);
	ReleaseICustButton(iBut);
	}

void PickOperand::ExitMode(IObjParam *ip)
	{
	ICustButton *iBut = GetICustButton(GetDlgItem(bo->hParams1,IDC_PICK_BOOLOPERAND));
	if (iBut) iBut->SetCheck(FALSE);
	ReleaseICustButton(iBut);
	}


static INT_PTR CALLBACK BoolParamDlgProc1( 
		HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
	BoolObject *bo = (BoolObject*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	if (!bo && message!=WM_INITDIALOG) return FALSE;

	switch (message) {
		case WM_INITDIALOG: {
			bo = (BoolObject*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			bo->hParams1 = hWnd;			
			
			ICustButton *iBut = GetICustButton(GetDlgItem(hWnd,IDC_PICK_BOOLOPERAND));
			iBut->SetType(CBT_CHECK);
			iBut->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(iBut);

			bo->SetupUI1();
			return FALSE;	// stop default keyboard focus - DB 2/27
			}

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_PICK_BOOLOPERAND:
					if (bo->ip->GetCommandMode()->ID() == CID_STDPICK) {
						if (bo->creating) {
							theCreateBoolMode.JumpStart(bo->ip,bo);
							bo->ip->SetCommandMode(&theCreateBoolMode);
						} else {
							bo->ip->SetStdCommandMode(CID_OBJMOVE);
							}
					} else {
						bo->pickCB.bo = bo;						
						bo->ip->SetPickMode(&bo->pickCB);
						}
					break;

				case IDC_TARG_REFERENCE:
				case IDC_TARG_COPY:
				case IDC_TARG_MOVE:
				case IDC_TARG_INSTANCE:
					bo->addOppMethod = LOWORD(wParam);
					break;				
				}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			bo->ip->RollupMouseMessage(hWnd,message,wParam,lParam);
			return FALSE;
		
		default:
			return FALSE;
		}
	return TRUE;
	}

static INT_PTR CALLBACK BoolParamDlgProc2( 
		HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
	BoolObject *bo = (BoolObject*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	if (!bo && message!=WM_INITDIALOG) return FALSE;

	switch (message) {
		case WM_INITDIALOG: {
			bo = (BoolObject*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			bo->hParams2 = hWnd;			
			bo->SetupUI2();
			return FALSE;	// DB 2/27
			}

		
		case WM_CUSTEDIT_ENTER: {
			ICustEdit *edit;
			TCHAR buf[256];
			
			switch (LOWORD(wParam)) {
				case IDC_BOOL_ANAME:
					edit = GetICustEdit(GetDlgItem(hWnd,IDC_BOOL_ANAME));					
					edit->GetText(buf,256);
					bo->opaName = TSTR(buf);					
					if (bo->ob1) bo->ob1->NotifyDependents(FOREVER,PART_ALL,REFMSG_NODE_NAMECHANGE,TREE_VIEW_CLASS_ID);
					break;
					
				case IDC_BOOL_BNAME:
					edit = GetICustEdit(GetDlgItem(hWnd,IDC_BOOL_BNAME));					
					edit->GetText(buf,256);
					bo->opbName = TSTR(buf);
					if (bo->ob2) bo->ob2->NotifyDependents(FOREVER,PART_ALL,REFMSG_NODE_NAMECHANGE,TREE_VIEW_CLASS_ID);
					break;					
				}
			
			bo->SetupUI2();			
			break;
			}

		case WM_COMMAND:
			switch(LOWORD(wParam)) {				
				case IDC_BOOL_RECALC:
					bo->ivalid.SetEmpty();
					bo->UpdateMesh(bo->ip->GetTime(),TRUE);
					bo->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
					bo->ip->RedrawViews(bo->ip->GetTime());
					break;

				case IDC_BOOL_DISPOPS:
				case IDC_BOOL_DISPRESULT:
					if (IsDlgButtonChecked(hWnd,IDC_BOOL_DISPRESULT)) {						
						bo->SetFlag(BOOL_DISPRESULT);
					} else {
						bo->ClearFlag(BOOL_DISPRESULT);
						}
					bo->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
					bo->ip->RedrawViews(bo->ip->GetTime());
					break;
				
				case IDC_BOOL_UPDATEALWAYS:
					bo->ClearFlag(BOOL_UPDATEALWAYS|BOOL_UPDATERENDER|BOOL_UPDATEMANUAL|BOOL_UPDATESELECT);
					bo->SetFlag(BOOL_UPDATEALWAYS);					
					bo->ip->RedrawViews(bo->ip->GetTime());
					bo->SetupUI2();
					break;

				case IDC_BOOL_UPDATESELECT:
					bo->ClearFlag(BOOL_UPDATEALWAYS|BOOL_UPDATERENDER|BOOL_UPDATEMANUAL|BOOL_UPDATESELECT);
					bo->SetFlag(BOOL_UPDATESELECT);					
					bo->ip->RedrawViews(bo->ip->GetTime());
					bo->SetupUI2();
					break;				

				case IDC_BOOL_UPDATERENDER:
					bo->ClearFlag(BOOL_UPDATEALWAYS|BOOL_UPDATERENDER|BOOL_UPDATEMANUAL|BOOL_UPDATESELECT);
					bo->SetFlag(BOOL_UPDATERENDER);					
					bo->SetupUI2();
					break;

				case IDC_BOOL_UPDATEMANUAL:
					bo->ClearFlag(BOOL_UPDATEALWAYS|BOOL_UPDATERENDER|BOOL_UPDATEMANUAL|BOOL_UPDATESELECT);
					bo->SetFlag(BOOL_UPDATEMANUAL);					
					bo->SetupUI2();
					break;
				
				case IDC_BOOL_UNION:
					bo->ClearFlag(BOOL_OPERATION);
					bo->SetFlag(BOOL_UNION);
					bo->Invalidate();
					bo->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
					bo->ip->RedrawViews(bo->ip->GetTime());
					break;
				case IDC_BOOL_INTERSECTION:
					bo->ClearFlag(BOOL_OPERATION);
					bo->SetFlag(BOOL_INTERSECTION);
					bo->Invalidate();
					bo->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
					bo->ip->RedrawViews(bo->ip->GetTime());
					break;
				case IDC_BOOL_DIFFERENCEAB:
					bo->ClearFlag(BOOL_OPERATION);
					bo->SetFlag(BOOL_DIFFERENCEA);
					bo->Invalidate();
					bo->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
					bo->ip->RedrawViews(bo->ip->GetTime());
					break;
				case IDC_BOOL_DIFFERENCEBA:
					bo->ClearFlag(BOOL_OPERATION);
					bo->SetFlag(BOOL_DIFFERENCEB);
					bo->Invalidate();
					bo->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
					bo->ip->RedrawViews(bo->ip->GetTime());
					break;

				case IDC_BOOL_OPTIMIZE:
					if (IsDlgButtonChecked(hWnd,IDC_BOOL_OPTIMIZE)) {
						bo->SetFlag(BOOL_OPTIMIZE);
					} else {
						bo->ClearFlag(BOOL_OPTIMIZE);
						}
					bo->Invalidate();
					bo->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
					bo->ip->RedrawViews(bo->ip->GetTime());
					break;
				
				case IDC_BOOL_SHOWHIDDENOPS:
					if (IsDlgButtonChecked(hWnd,IDC_BOOL_SHOWHIDDENOPS)) {
						bo->SetFlag(BOOL_DISPHIDDEN);
					} else {
						bo->ClearFlag(BOOL_DISPHIDDEN);
						}
					bo->Invalidate();
					bo->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
					bo->ip->RedrawViews(bo->ip->GetTime());
					break;
				

				case IDC_BOOL_OPERANDS:
					if (HIWORD(wParam)==LBN_SELCHANGE) {
						bo->flags &= ~BOOL_ANYSEL;
						
						if (SendMessage((HWND)lParam,LB_GETSEL,0,0)) {
							bo->flags |= BOOL_OB1SEL;
							}
							
						if (SendMessage((HWND)lParam,LB_GETSEL,1,0)) {
							bo->flags |= BOOL_OB2SEL;
							}
						bo->NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
						bo->NotifyDependents(FOREVER,(PartID) bo,REFMSG_BRANCHED_HISTORY_CHANGED);
						bo->ip->RedrawViews(bo->ip->GetTime());
						bo->SetExtractButtonState();
						}
					break;

				case IDC_BOOL_EXTRACT_INTANCE:
					bo->extractCopy = FALSE;
					break;
				case IDC_BOOL_EXTRACT_COPY:
					bo->extractCopy = TRUE;
					break;
				case IDC_BOOL_EXTRACTOP:
					theHold.Begin();
					if (bo->flags&BOOL_OB1SEL) bo->ExtractOperand(0);
					if (bo->flags&BOOL_OB2SEL) bo->ExtractOperand(1);
					theHold.Accept(GetString(IDS_RB_EXTRACTOP));
					bo->ip->RedrawViews(bo->ip->GetTime());
					break;
				}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			bo->ip->RollupMouseMessage(hWnd,message,wParam,lParam);
			return FALSE;
		
		default:
			return FALSE;
		}
	return TRUE;
	}

void BoolObject::BeginEditParams(
		IObjParam *ip, ULONG flags,Animatable *prev)
	{	
	this->ip = ip;	
	editOb   = this;

	if (flags&BEGIN_EDIT_CREATE) {
		creating = TRUE;
	} else {
		creating = FALSE;
		// Create sub object editing modes.
		moveMode       = new MoveModBoxCMode(this,ip);
		rotMode        = new RotateModBoxCMode(this,ip);
		uscaleMode     = new UScaleModBoxCMode(this,ip);
		nuscaleMode    = new NUScaleModBoxCMode(this,ip);
		squashMode     = new SquashModBoxCMode(this,ip);
		selectMode     = new SelectModBoxCMode(this,ip);
		
		// Add our sub object type
		// TSTR type(GetString(IDS_RB_OPERANDS));
		// const TCHAR *ptype[] = {type};
		// This call is obsolete. Please see BaseObject::NumSubObjTypes() and BaseObject::GetSubObjType()
		// 	ip->RegisterSubObjectTypes(ptype, 1);
		}

	if (!hParams1) {
		hParams1 = ip->AddRollupPage( 
				hInstance, 
				MAKEINTRESOURCE(IDD_BOOLPARAM1),
				BoolParamDlgProc1, 
				GetString(IDS_RB_PICKBOOLEAN), 
				(LPARAM)this);		
		ip->RegisterDlgWnd(hParams1);
		hParams2 = ip->AddRollupPage( 
				hInstance, 
				MAKEINTRESOURCE(IDD_BOOLPARAM2),
				BoolParamDlgProc2, 
				GetString(IDS_RB_PARAMETERS), 
				(LPARAM)this);		
		ip->RegisterDlgWnd(hParams1);
	} else {
		SetWindowLongPtr(hParams1,GWLP_USERDATA,(LONG_PTR)this);
		SetWindowLongPtr(hParams2,GWLP_USERDATA,(LONG_PTR)this);
		SetupUI1();
		SetupUI2();
		}
	}

void BoolObject::EndEditParams(
		IObjParam *ip, ULONG flags,Animatable *next)
	{	
	editOb = NULL;

	if (flags&END_EDIT_REMOVEUI) {
		ip->UnRegisterDlgWnd(hParams1);
		ip->DeleteRollupPage(hParams1);
		hParams1 = NULL;
		ip->UnRegisterDlgWnd(hParams2);
		ip->DeleteRollupPage(hParams2);
		hParams2 = NULL;
	} else {
		SetWindowLongPtr(hParams1,GWLP_USERDATA,(LONG)NULL);
		SetWindowLongPtr(hParams2,GWLP_USERDATA,(LONG)NULL);
		}	
	
	if (!creating) {
		ip->DeleteMode(moveMode);
		ip->DeleteMode(rotMode);
		ip->DeleteMode(uscaleMode);
		ip->DeleteMode(nuscaleMode);
		ip->DeleteMode(squashMode);
		ip->DeleteMode(selectMode);
		if ( moveMode ) delete moveMode;
		moveMode = NULL;
		if ( rotMode ) delete rotMode;
		rotMode = NULL;
		if ( uscaleMode ) delete uscaleMode;
		uscaleMode = NULL;
		if ( nuscaleMode ) delete nuscaleMode;
		nuscaleMode = NULL;
		if ( squashMode ) delete squashMode;
		squashMode = NULL;
		if ( selectMode ) delete selectMode;
		selectMode = NULL;	
	} else {
		lastOp = BOOL_OPERATION&this->flags;
		}

	ip->ClearPickMode();
	ip = NULL;
	creating = FALSE;
	}

int BoolObject::NumPipeBranches(bool selected) 
	{
	int num=0;
	if ((!selected || TestFlag(BOOL_OB1SEL)) && ob1) num++;
	if ((!selected || TestFlag(BOOL_OB2SEL)) && ob2) num++;
	return num;
	}

Object *BoolObject::GetPipeBranch(int i, bool selected) 
	{
	if (i) return ob2;	
	if (ob1 && (!selected || TestFlag(BOOL_OB1SEL))) return ob1;
	return ob2;
	}

INode *BoolObject::GetBranchINode(TimeValue t,INode *node,int i, bool selected)
	{
	if(!selected)
		return CreateINodeTransformed(node,GetOpTM(t,i));
	
	assert(i<2);
	int index = 0;
	if (i) index = 1;
	else if (TestFlag(BOOL_OB1SEL)) index = 0;
	else index = 1;
	return CreateINodeTransformed(node,GetOpTM(t,index));	
	}

int BoolObject::NumSubs()
	{
	return 4;
	}

Animatable* BoolObject::SubAnim(int i)
	{
	switch (i) {
		case 0:  return ob1;
		case 1:  return tm1;
		case 2:	 return ob2;
		case 3:	 return tm2;
		default: return NULL;
		}	
	}

TSTR BoolObject::SubAnimName(int i)
	{	
	switch (i) {
		case 0: return opaName;//GetString(IDS_RB_OPERANDA);
		case 1: return GetString(IDS_RB_OPERANDATRANSFORM);
		case 2: return opbName;//GetString(IDS_RB_OPERANDB);
		case 3: return GetString(IDS_RB_OPERANDBTRANSFORM);
		}
	return _T("Error");
	}

int BoolObject::SubNumToRefNum(int subNum)
	{
	switch (subNum) {
		case 0:  return REF_OP1;
		case 1:  return REF_CONT1;
		case 2:	 return REF_OP2;
		case 3:	 return REF_CONT2;
		default: return -1;
		}	
	}

RefTargetHandle BoolObject::GetReference(int i)
	{
	switch (i) {
		case REF_OP1: 	return ob1;
		case REF_OP2: 	return ob2;
		case REF_CONT1:	return tm1;
		case REF_CONT2:	return tm2;
		default:        return NULL;
		}
	}

void BoolObject::SetReference(int i, RefTargetHandle rtarg)
	{
	switch (i) {
		case REF_OP1: 	 
			ob1 = (Object*)rtarg;
			/*
			if (rtarg==NULL) {				
				if (editOb==this) {
					SetupUI2();
					}
				}
			*/
			break;

		case REF_OP2: 	 
			ob2 = (Object*)rtarg;  
			/*
			if (rtarg==NULL) {
				if (editOb==this) {
					SetupUI2();
					}
				}
			*/
			break;

		case REF_CONT1:	 tm1 = (Control*)rtarg; break;
		case REF_CONT2:	 tm2 = (Control*)rtarg; break;
		}
	}

RefTargetHandle BoolObject::Clone(RemapDir& remap)
	{
	BoolObject *obj = new BoolObject;
	if (ob1) obj->ReplaceReference(REF_OP1,remap.CloneRef(ob1));
	if (ob2) obj->ReplaceReference(REF_OP2,remap.CloneRef(ob2));
	if (tm1) obj->ReplaceReference(REF_CONT1,remap.CloneRef(tm1));
	if (tm2) obj->ReplaceReference(REF_CONT2,remap.CloneRef(tm2));
	obj->flags = flags;
	obj->opaName = opaName;
	obj->opbName = opbName;
	BaseClone(this, obj, remap);
	return obj;
	}

int BoolObject::IntersectRay(
		TimeValue t, Ray& r, float& at, Point3& norm)
	{
	if (TestFlag(BOOL_DISPRESULT)) {
		UpdateMesh(t);
		return mesh.IntersectRay(r,at,norm);
	} else {
		return 0;
		}
	}

Mesh* BoolObject::GetRenderMesh(
		TimeValue t, INode *inode, View& view, BOOL& needDelete)
	{	
	if (!ob1 || !ob2) {
		if (ob1) {
			Object *obj =
				GetPipeObj(t,0);
			if (obj && obj->SuperClassID()==GEOMOBJECT_CLASS_ID) {
				return ((GeomObject*)obj)->
					GetRenderMesh(t,inode,view,needDelete);
				}
			}
		if (ob2) {
			Object *obj =
				GetPipeObj(t,1);
			if (obj && obj->SuperClassID()==GEOMOBJECT_CLASS_ID) {
				return ((GeomObject*)obj)->
					GetRenderMesh(t,inode,view,needDelete);
				}
			}
		}
	UpdateMesh(t);
	needDelete = FALSE;
	return &mesh;	
	}

int BoolObject::HitTest(
		TimeValue t, INode* inode, int type, int crossing, int flags, 
		IPoint2 *p, ViewExp *vpt)
	{
	int res = 0;
	if (TestFlag(BOOL_DISPRESULT) && ob1 && ob2) {
		UpdateMesh(t,FALSE,inode->Selected());
		HitRegion hitRegion;
		GraphicsWindow *gw = vpt->getGW();	
		Material *mtl = gw->getMaterial();		
		gw->setTransform(inode->GetObjectTM(t));
		MakeHitRegion(hitRegion, type, crossing, 4, p);
		res = mesh.select(gw, mtl, &hitRegion, flags & HIT_ABORTONHIT);
		if (res) return res;
	} else {
		Object *ob;
		if (ob=GetPipeObj(t,0)) {
			INodeTransformed n(inode,GetOpTM(t,0));
			res = ob->HitTest(t,&n,type,crossing,flags,p,vpt);
			if (res) return res;
			}
		if (ob=GetPipeObj(t,1)) {
			INodeTransformed n(inode,GetOpTM(t,1));
			res = ob->HitTest(t,&n,type,crossing,flags,p,vpt);
			if (res) return res;
			}
		}
	return res;
	}

void BoolObject::Snap(
		TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt)
	{
	if (TestFlag(BOOL_DISPRESULT) && ob1 && ob2) {
		UpdateMesh(t,FALSE,inode->Selected());
		Matrix3 tm = inode->GetObjectTM(t);	
		GraphicsWindow *gw = vpt->getGW();		
		gw->setTransform(tm);
		mesh.snap( gw, snap, p, tm );
	} else {	
		Object *ob;
		if (ob=GetPipeObj(t,0)) {
			INodeTransformed n(inode,GetOpTM(t,0));
			ob->Snap(t,&n,snap,p,vpt);
			}	
		if (ob=GetPipeObj(t,1)) {
			INodeTransformed n(inode,GetOpTM(t,1));
			ob->Snap(t,&n,snap,p,vpt);
			}
		}
	}

#define DRAW_A (1<<1)
#define DRAW_B (1<<2)

int BoolObject::
		Display(TimeValue t, INode* inode, ViewExp *vpt, int flags)
	{	
	int disp = 0;
	GraphicsWindow *gw = vpt->getGW();
	BOOL showHidden = TestFlag(BOOL_DISPHIDDEN);

	if (TestFlag(BOOL_DISPRESULT) && ob1 && ob2) {
		UpdateMesh(t,FALSE,inode->Selected());		
		gw->setTransform(inode->GetObjectTM(t));
		mesh.render(gw, inode->Mtls(),(flags&USE_DAMAGE_RECT)?&vpt->GetDammageRect():NULL, COMP_ALL, inode->NumMtls());
	
		// Show hidden ops?
		if (showHidden && TestFlag(BOOL_DIFFERENCEA)) {
			disp = DRAW_B;
			}
		if (showHidden && TestFlag(BOOL_DIFFERENCEB)) {
			disp = DRAW_A;
			}
		if (showHidden && TestFlag(BOOL_INTERSECTION)) {
			disp = DRAW_B|DRAW_A;
			}
	} else {
		disp = 1;		
		}
			
	if (disp) {	
		Object *ob;
		
		DWORD rlim = gw->getRndLimits();

		if (disp&DRAW_A || disp&DRAW_B) {
			if (!(rlim&GW_ILLUM)) return 0;
			gw->setRndLimits(GW_WIREFRAME|GW_EDGES_ONLY|GW_BACKCULL|(rlim&GW_Z_BUFFER?GW_Z_BUFFER:0));
			if (inode->NumMtls()) {
				Point3 c = inode->Mtls()->Kd;
				gw->setColor(LINE_COLOR,c.x,c.y,c.z);
			} else {
				gw->setColor(LINE_COLOR,0.7f,0.7f,0.7f);
				}
			}

		if ((disp==1 || disp&DRAW_A) && (ob=GetPipeObj(t,0))) {
			 INodeTransformed n(inode,GetOpTM(t,0));
			 if (inode->Selected()) {
				 if (TestFlag(BOOL_OB1SEL)) {
					vpt->getGW()->setColor(LINE_COLOR,1.0f,0.0f,0.0f);
				 } else {
					Point3 selClr = GetUIColor(COLOR_SELECTION); 
					vpt->getGW()->setColor( LINE_COLOR, selClr.x, selClr.y, selClr.z);
				 	}
				}
			 ob->Display(t,&n,vpt,flags);
			 }
		if ((disp==1 || disp&DRAW_B) && (ob=GetPipeObj(t,1))) {
			INodeTransformed n(inode,GetOpTM(t,1));
			if (inode->Selected()) {
				if (TestFlag(BOOL_OB2SEL)) {
					vpt->getGW()->setColor(LINE_COLOR,1.0f,0.0f,0.0f);
				 } else {
					vpt->getGW()->setColor( LINE_COLOR, GetSelColor());
				 	}
				}
			ob->Display(t,&n,vpt,flags);
			}
		
		if (disp&DRAW_A || disp&DRAW_B) {
			gw->setRndLimits(rlim);
			}
		}
	return 0;
	}

void BoolObject::GetDeformBBox(
		TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel)
	{
	Box3 abox;
	abox.Init();
	box.Init();
	if (TestFlag(BOOL_DISPRESULT) && ob1 && ob2) {
		UpdateMesh(t);
		box = mesh.getBoundingBox(tm);
	} else {
		Object *ob;
		if (ob=GetPipeObj(t,0)) {				
			if (tm) {
				Matrix3 mat = GetOpTM(t,0) * *tm;
				ob->GetDeformBBox(t,abox,&mat,useSel);
			} else ob->GetDeformBBox(t,abox,NULL,useSel);
			box += abox;
			}
		if (ob=GetPipeObj(t,1)) {
			if (tm) {
				Matrix3 mat = GetOpTM(t,1) * *tm;
				ob->GetDeformBBox(t,abox,&mat,useSel);
			} else ob->GetDeformBBox(t,abox,NULL,useSel);
			box += abox;
			}
		}
	}

void BoolObject::GetLocalBoundBox(
		TimeValue t, INode *inode,ViewExp* vpt, Box3& box) 
	{
	Box3 abox;
	abox.Init();
	box.Init();
	if (TestFlag(BOOL_DISPRESULT) && ob1 && ob2) {
		UpdateMesh(t,FALSE,inode->Selected());
		box = mesh.getBoundingBox();	
	} else {
		Object *ob;
		if (ob=GetPipeObj(t,0)) {
			INodeTransformed n(inode,GetOpTM(t,0));
			ob->GetLocalBoundBox(t,&n,vpt,abox);
			if (!abox.IsEmpty()) abox = abox * GetOpTM(t,0);
			box += abox;
			}
		if (ob=GetPipeObj(t,1)) {
			INodeTransformed n(inode,GetOpTM(t,1));
			ob->GetLocalBoundBox(t,&n,vpt,abox);
			if (!abox.IsEmpty()) abox = abox * GetOpTM(t,1);
			box += abox;
			}
		}
	}

void BoolObject::GetWorldBoundBox(
		TimeValue t, INode *inode, ViewExp* vpt, Box3& box)
	{
	Box3 abox;
	int disp = 0;
	abox.Init();
	box.Init();
	BOOL showHidden = TestFlag(BOOL_DISPHIDDEN);

	if (TestFlag(BOOL_DISPRESULT) && ob1 && ob2) {
		UpdateMesh(t,FALSE,inode->Selected());
		Matrix3 mat = inode->GetObjectTM(t);	
		box = mesh.getBoundingBox();
		if (!box.IsEmpty()) box = box * mat;

		// Show hidden ops?
		if (showHidden && TestFlag(BOOL_DIFFERENCEA)) {
			disp = DRAW_B;
			}
		if (showHidden && TestFlag(BOOL_DIFFERENCEB)) {
			disp = DRAW_A;
			}
		if (showHidden && TestFlag(BOOL_INTERSECTION)) {
			disp = DRAW_B|DRAW_A;
			}
	} else {
		disp = 1;
		}

	if (disp) {	
		Object *ob;
		
		if ((disp==1 || disp&DRAW_A) && (ob=GetPipeObj(t,0))) {		
			INodeTransformed n(inode,GetOpTM(t,0));
			ob->GetWorldBoundBox(t,&n,vpt,abox);
			box += abox;
			}
		
		if ((disp==1 || disp&DRAW_B) && (ob=GetPipeObj(t,1))) {		
			INodeTransformed n(inode,GetOpTM(t,1));
			ob->GetWorldBoundBox(t,&n,vpt,abox);
			box += abox;
			}
		}
	}

RefResult BoolObject::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
		PartID& partID, 
		RefMessage message ) 
	{
	switch (message) {
		case REFMSG_SELECT_BRANCH:
			if (hTarget==ob1 || hTarget==ob2) {
				ClearFlag(BOOL_OB1SEL|BOOL_OB2SEL);
				if (hTarget==ob1) SetFlag(BOOL_OB1SEL);
				if (hTarget==ob2) SetFlag(BOOL_OB2SEL);
				NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
				NotifyDependents(FOREVER,(PartID) this,REFMSG_BRANCHED_HISTORY_CHANGED);
				}
			break;

		case REFMSG_CHANGE:
			ivalid.SetEmpty();
			break;
		}
	return REF_SUCCEED;
	}




//--Subobject Selection-------------------------------------------------------------


void BoolObject::Move(
		TimeValue t, Matrix3& partm, Matrix3& tmAxis, 
		Point3& val, BOOL localOrigin)
	{
#ifdef DESIGN_VER
	t=0;
#endif
	if (TestFlag(BOOL_OB1SEL) && tm1) {
		SetXFormPacket pckt(val,partm,tmAxis);
		tm1->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
		}
	if (TestFlag(BOOL_OB2SEL) && tm2) {
		SetXFormPacket pckt(val,partm,tmAxis);
		tm2->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
		}
	}

void BoolObject::Rotate(
		TimeValue t, Matrix3& partm, Matrix3& tmAxis, 
		Quat& val, BOOL localOrigin)
	{
#ifdef DESIGN_VER
	t=0;
#endif
	if (TestFlag(BOOL_OB1SEL) && tm1) {
		SetXFormPacket pckt(val,localOrigin,partm,tmAxis);
		tm1->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
		}
	if (TestFlag(BOOL_OB2SEL) && tm2) {
		SetXFormPacket pckt(val,localOrigin,partm,tmAxis);
		tm2->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
		}
	}

void BoolObject::Scale(
		TimeValue t, Matrix3& partm, Matrix3& tmAxis, 
		Point3& val, BOOL localOrigin)
	{
#ifdef DESIGN_VER
	t=0;
#endif
	if (TestFlag(BOOL_OB1SEL) && tm1) {
		SetXFormPacket pckt(val,localOrigin,partm,tmAxis);
		tm1->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
		}
	if (TestFlag(BOOL_OB2SEL) && tm2) {
		SetXFormPacket pckt(val,localOrigin,partm,tmAxis);
		tm2->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
		}
	}

int BoolObject::HitTest(
		TimeValue t, INode* inode, int type, int crossing, 
		int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc)
	{	
	int res = 0;
	Object *ob;
	if ((ob=GetPipeObj(t,0)) &&
		!(flags&HIT_SELONLY && !TestFlag(BOOL_OB1SEL)) &&
		!(flags&HIT_UNSELONLY && TestFlag(BOOL_OB1SEL)) ) {
		
		INodeTransformed n(inode,GetOpTM(t,0));
		
		if (ob->HitTest(t,&n,type,crossing,flags,p,vpt)) {
			vpt->LogHit(inode,mc,0,0,NULL);
			res = TRUE;
			if (flags & HIT_ABORTONHIT) return TRUE;
			}		
		}
	if ((ob=GetPipeObj(t,1)) &&
		!(flags&HIT_SELONLY && !TestFlag(BOOL_OB2SEL)) &&
		!(flags&HIT_UNSELONLY && TestFlag(BOOL_OB2SEL)) ) {
		
		INodeTransformed n(inode,GetOpTM(t,1));
		
		if (ob->HitTest(t,&n,type,crossing,flags,p,vpt)) {
			vpt->LogHit(inode,mc,0,1,NULL);
			res = TRUE;			
			}		
		}
	
	return res;
	}

int BoolObject::Display(
		TimeValue t, INode* inode, ViewExp *vpt, 
		int flags, ModContext* mc)
	{
	/*
	if (TestFlag(BOOL_DISPRESULT)) {
		
		if (TestFlag(BOOL_OB1SEL) && ob1) {
			INodeTransformed *tnode = 
				CreateINodeTransformed(inode,GetOpTM(t,0));
			ob1->Display(t,tnode,vpt,flags,mc);
			tnode->DisposeTemporary();
			}

		if (TestFlag(BOOL_OB2SEL) && ob2) {
			INodeTransformed *tnode = 
				CreateINodeTransformed(inode,GetOpTM(t,1));
			ob1->Display(t,tnode,vpt,flags,mc);
			tnode->DisposeTemporary();
			}		
		}
		*/
	return 0;
	}

void BoolObject::SelectSubComponent(
		HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert)
	{
	while (hitRec) {
		if (hitRec->hitInfo) {
			if (selected) SetFlag(BOOL_OB2SEL);
			else ClearFlag(BOOL_OB2SEL);
		} else {
			if (selected) SetFlag(BOOL_OB1SEL);
			else ClearFlag(BOOL_OB1SEL);
			}
		if (all) hitRec = hitRec->Next();
		else break;
		}
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	NotifyDependents(FOREVER,(PartID)this,REFMSG_BRANCHED_HISTORY_CHANGED);
	if (ip) SetupUI2();
	}

void BoolObject::ClearSelection(int selLevel)
	{
	ClearFlag(BOOL_OB1SEL|BOOL_OB2SEL);
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	if (ip) SetupUI2();
	}

int BoolObject::SubObjectIndex(HitRecord *hitRec)
	{
	return hitRec->hitInfo;
	}

void BoolObject::GetSubObjectCenters(
		SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc)
	{
	Matrix3 tm;
	if (TestFlag(BOOL_OB1SEL)) {
		tm = GetOpTM(t,0) * node->GetObjectTM(t);
		cb->Center(tm.GetTrans(),0);
		}
	if (TestFlag(BOOL_OB2SEL)) {
		tm = GetOpTM(t,1) * node->GetObjectTM(t);
		cb->Center(tm.GetTrans(),1);
		}
	}

void BoolObject::GetSubObjectTMs(
		SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc)
	{
	Matrix3 tm;
	if (TestFlag(BOOL_OB1SEL)) {
		tm = GetOpTM(t,0) * node->GetObjectTM(t);
		cb->TM(tm,0);
		}
	if (TestFlag(BOOL_OB2SEL)) {
		tm = GetOpTM(t,1) * node->GetObjectTM(t);
		cb->TM(tm,1);
		}
	}

void BoolObject::ActivateSubobjSel(int level, XFormModes& modes)
	{
	if (level) {
		modes = XFormModes(moveMode,rotMode,nuscaleMode,uscaleMode,squashMode,selectMode);
		NotifyDependents(
			FOREVER, 
			PART_SUBSEL_TYPE|PART_DISPLAY, 
			REFMSG_CHANGE);		
		ip->PipeSelLevelChanged();
		}
	}


#define BOOL_FLAGS_CHUNK	0x0100
#define BOOL_OPANAME_CHUNK	0x0110
#define BOOL_OPBNAME_CHUNK	0x0120
#define BOOL_OFFSET_CHUNK	0x0130

IOResult BoolObject::Save(ISave *isave)
	{
	ULONG nb;

	isave->BeginChunk(BOOL_FLAGS_CHUNK);		
	isave->Write(&flags,sizeof(flags),&nb);
	isave->EndChunk();
	
	isave->BeginChunk(BOOL_OPANAME_CHUNK);		
	isave->WriteWString(opaName);
	isave->EndChunk();
	
	isave->BeginChunk(BOOL_OPBNAME_CHUNK);		
	isave->WriteWString(opbName);
	isave->EndChunk();
	
	isave->BeginChunk(BOOL_OFFSET_CHUNK);		
	obOffset.Save(isave);
	isave->EndChunk();	

	return IO_OK;
	}


IOResult BoolObject::Load(ILoad *iload)
	{
	ULONG nb;
	IOResult res = IO_OK;
	
	// Default names
	opaName = GetString(IDS_RB_OPERAND);
	opbName = GetString(IDS_RB_OPERAND);

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case BOOL_OFFSET_CHUNK:
				obOffset.Load(iload);
				break;

			case BOOL_FLAGS_CHUNK:
				res=iload->Read(&flags,sizeof(flags),&nb);
				break;
			
			case BOOL_OPANAME_CHUNK: {
				TCHAR *buf;
				res=iload->ReadWStringChunk(&buf);
				opaName = TSTR(buf);
				break;
				}

			case BOOL_OPBNAME_CHUNK: {
				TCHAR *buf;
				res=iload->ReadWStringChunk(&buf);
				opbName = TSTR(buf);
				break;
				}
			}
		
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}

	Invalidate();
	return IO_OK;
	}



BOOL BoolObject::GetOperandSel(int which)
	{
	if (which) 
		 return TestFlag(BOOL_OB2SEL);
	else return TestFlag(BOOL_OB1SEL);
	}

void BoolObject::SetOperandSel(int which,BOOL sel)
	{
	if (which)
		 SetFlag(BOOL_OB2SEL);
	else SetFlag(BOOL_OB1SEL);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
}

int BoolObject::GetBoolOp()
	{
	if (TestFlag(BOOL_UNION)) return BOOLOP_UNION;
	if (TestFlag(BOOL_INTERSECTION)) return BOOLOP_INTERSECTION;
	if (TestFlag(BOOL_DIFFERENCEA)) return BOOLOP_SUB_AB;
	if (TestFlag(BOOL_DIFFERENCEB)) return BOOLOP_SUB_BA;
	return 0;
	}

void BoolObject::SetBoolOp(int op)
	{
	ClearFlag(BOOL_OPERATION);
	switch (op) {
		case BOOLOP_UNION:
			SetFlag(BOOL_UNION); break;
		case BOOLOP_INTERSECTION:
			SetFlag(BOOL_INTERSECTION); break;
		case BOOLOP_SUB_AB:
			SetFlag(BOOL_DIFFERENCEA); break;
		case BOOLOP_SUB_BA:
			SetFlag(BOOL_DIFFERENCEB); break;
		}
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}

BOOL BoolObject::GetDisplayResult()
	{
	return TestFlag(BOOL_DISPRESULT);
	}

void BoolObject::SetDisplayResult(BOOL onOff)
	{
	if (onOff) 
		 SetFlag(BOOL_DISPRESULT);
	else ClearFlag(BOOL_DISPRESULT);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}

BOOL BoolObject::GetShowHiddenOps()
	{
	return TestFlag(BOOL_DISPHIDDEN);
	}

void BoolObject::SetShowHiddenOps(BOOL onOff)
	{
	if (onOff) 
		 SetFlag(BOOL_DISPHIDDEN);
	else ClearFlag(BOOL_DISPHIDDEN);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}


int BoolObject::GetUpdateMode()
	{
	if (TestFlag(BOOL_UPDATEALWAYS)) return BOOLUPDATE_ALWAYS;
	if (TestFlag(BOOL_UPDATERENDER)) return BOOLUPDATE_RENDER;
	if (TestFlag(BOOL_UPDATEMANUAL)) return BOOLUPDATE_MANUAL;
	if (TestFlag(BOOL_UPDATESELECT)) return BOOLUPDATE_SELECTED;
	return 0;
	}
 
void BoolObject::SetUpdateMode(int mode)
	{
	ClearFlag(BOOL_UPDATEALWAYS);
	ClearFlag(BOOL_UPDATERENDER);
	ClearFlag(BOOL_UPDATEMANUAL);
	ClearFlag(BOOL_UPDATESELECT);
	switch (mode) {
		case BOOLUPDATE_ALWAYS:
			SetFlag(BOOL_UPDATEALWAYS); break;
		case BOOLUPDATE_SELECTED:
			SetFlag(BOOL_UPDATESELECT); break;
		case BOOLUPDATE_RENDER:
			SetFlag(BOOL_UPDATERENDER); break;
		case BOOLUPDATE_MANUAL:
			SetFlag(BOOL_UPDATEMANUAL); break;
		}
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}

BOOL BoolObject::GetOptimize()
	{
	return TestFlag(BOOL_OPTIMIZE);
	}

void BoolObject::SetOptimize(BOOL onOff)
	{
	if (onOff) 
		 SetFlag(BOOL_OPTIMIZE);
	else ClearFlag(BOOL_OPTIMIZE);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}

int BoolObject::NumSubObjTypes() 
{ 
	return 1;
}

ISubObjType *BoolObject::GetSubObjType(int i) 
{

	static bool initialized = false;
	if(!initialized)
	{
		initialized = true;
		SOT_Operands.SetName(GetString(IDS_RB_OPERANDS));
	}

	switch(i)
	{
	case 0:
		return &SOT_Operands;
	}
	return NULL;
}

#endif // NO_OBJECT_BOOL
