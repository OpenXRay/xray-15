/**********************************************************************
 *<
	FILE: morphobj.cpp

	DESCRIPTION:  Morph object

	CREATED BY: Rolf Berteig

	HISTORY: created 21 August 1995

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "prim.h"

#ifndef NO_OBJECT_MORPH	// russom - 10/15/01

Control *newDefaultMorphControl();

class MorphObject;

#define TARG_COPY		0
#define TARG_REF		1
#define TARG_INSTANCE	2
#define TARG_MOVE		3


class PickTarget;

class MorphObject: public GeomObject {			   
		
		RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message);
				
	public:
		MorphControl *morphCont;
		int sel;

		static IObjParam *ip;
		static HWND hParams1;
		static HWND hParams2;
		static PickTarget pickCB;
		static int addTargMethod;
		static MorphObject *editOb;
		static BOOL creating;

		MorphObject(BOOL loading=FALSE);
		~MorphObject();
		void SetupTargetList();
		int AddTargMethod();
		void SetTargMethod(int m);
		void AddNewTarget(INode *node,TimeValue t,int m);

		// From BaseObject
		CreateMouseCallBack* GetCreateMouseCallBack();		
		TCHAR *GetObjectName() { return GetString(IDS_RB_MORPH); }

		// From Object		
		void InitNodeName(TSTR& s) {s = GetString(IDS_RB_MORPH);}
		Interval ObjectValidity(TimeValue t);
		int CanConvertToType(Class_ID obtype);
		Object* ConvertToType(TimeValue t, Class_ID obtype);
        BOOL PolygonCount(TimeValue t, int& numFaces, int& numVerts);
		ObjectState Eval(TimeValue time);
		int NumPipeBranches(bool selected);
		Object *GetPipeBranch(int i, bool selected);
		BOOL HasUVW() {return morphCont->HasUVW();}
		void SetGenUVW(BOOL sw) {morphCont->SetGenUVW(sw);}

		// From GeomObject		
		ObjectHandle CreateTriObjRep(TimeValue t);  // for rendering, also for deformation
		int IntersectRay(TimeValue t, Ray& r, float& at) {return 0;}

		// Animatable methods
		Class_ID ClassID() {return Class_ID(MORPHOBJ_CLASS_ID,0);}  
		void GetClassName(TSTR& s) {s = GetString(IDS_RB_MORPHOBJECT_CLASS);}
		void DeleteThis() {delete this;}				
		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);
		BOOL AssignController(Animatable *control,int subAnim);

		int NumSubs();
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);

		// From ref
		RefTargetHandle Clone(RemapDir& remap);
		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i) {return morphCont;}
		void SetReference(int i, RefTargetHandle rtarg) {morphCont = (MorphControl*)rtarg;}
	};				

class PickTarget : 
		public PickModeCallback,
		public PickNodeCallback {
	public:		
		MorphObject *mo;				

		PickTarget() {mo=NULL;}

		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);

		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);

		BOOL RightClick(IObjParam *ip,ViewExp *vpt)	{return TRUE;}

		BOOL Filter(INode *node);
		
		PickNodeCallback *GetFilter() {return this;}
		BOOL AllowMultiSelect() {return TRUE;}
	};


IObjParam *MorphObject::ip                 = NULL;
HWND MorphObject::hParams1                 = NULL;
HWND MorphObject::hParams2                 = NULL;
int MorphObject::addTargMethod             = TARG_INSTANCE;
MorphObject *MorphObject::editOb           = NULL;
BOOL MorphObject::creating                 = FALSE;
PickTarget MorphObject::pickCB;

class MorphObjClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new MorphObject(loading);}
	const TCHAR *	ClassName() { return GetString(IDS_RB_MORPH_CLASS); }
	SClass_ID		SuperClassID() { return GEOMOBJECT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(MORPHOBJ_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_COMPOUNDOBJECTS);}
	BOOL			OkToCreate(Interface *i);
	int 			BeginCreate(Interface *i);
	int 			EndCreate(Interface *i);
	void			ResetClassParams(BOOL fileReset);
	};

void MorphObjClassDesc::ResetClassParams(BOOL fileReset)
	{
	MorphObject::addTargMethod = TARG_INSTANCE;
	}

BOOL MorphObjClassDesc::OkToCreate(Interface *i)
	{
	if (i->GetSelNodeCount()!=1) return FALSE;	
	ObjectState os = i->GetSelNode(0)->GetObjectRef()->Eval(i->GetTime());	
	if (os.obj->IsParticleSystem()) return FALSE;
	return os.obj->IsDeformable() || os.obj->CanConvertToType(defObjectClassID);
	}

static MorphObjClassDesc morphObjDesc;

ClassDesc* GetMorphObjDesc() { return &morphObjDesc; }

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

//----------------------------------------------------------------------

#define CID_CREATEMORPHMODE		0x8F525AB2

class CreateMorphProc : public MouseCallBack {
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

int CreateMorphProc::proc( 
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

class CreateMorphMode : public CommandMode, ReferenceMaker {		
	public:		
		CreateMorphProc proc;
		INode *node, *svNode;
		IObjParam *ip;
		MorphObject *obj;

		void Begin(INode *n,IObjParam *i);
		void End(IObjParam *i);		
		void JumpStart(IObjParam *i,MorphObject *o);

		int Class() { return CREATE_COMMAND; }
		int ID() { return CID_CREATEMORPHMODE; }
		MouseCallBack *MouseProc(int *numPoints) {*numPoints = 1; return &proc;}
		ChangeForegroundCallback *ChangeFGProc() {return CHANGE_FG_SELECTED;}
		BOOL ChangeFG(CommandMode *oldMode) {return TRUE;}
		void EnterMode() {}
		void ExitMode() {}
		
		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i) {return node;}
		void SetReference(int i, RefTargetHandle rtarg) {node = (INode*)rtarg;}
	    RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	    	PartID& partID,  RefMessage message);		
	};
static CreateMorphMode theCreateMorphMode;

RefResult CreateMorphMode::NotifyRefChanged(
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

class CreateMorphRestore : public RestoreObj {
	public:   		
		void Restore(int isUndo) {
			if (theCreateMorphMode.ip) {
				// Jump out of boolean create mode.
				theCreateMorphMode.ip->SetStdCommandMode(CID_OBJMOVE);
				}
			}	
		void Redo() {}
		TSTR Description() {return TSTR(_T("Create Morph"));}
	};

// Sending the REFMSG_NOTIFY_PASTE message notifies the modify
// panel that the Node's object reference has changed when
// undoing or redoing.
class CreateMorphNotify : public RestoreObj {
	public:   		
		MorphObject *obj;
		BOOL which;
		CreateMorphNotify(MorphObject *o, BOOL w) {
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
		TSTR Description() {return TSTR(_T("Create Morph Notify"));}
	};

void CreateMorphMode::Begin(INode *n,IObjParam *i) 
	{
	MakeRefByID(FOREVER,0,n);
	svNode = node;
	assert(node);
	ip = i;
	proc.Init(ip);

	theHold.Begin();
	theHold.Put(new CreateMorphRestore);

	obj = new MorphObject;
	
	theHold.Put(new CreateMorphNotify(obj,1));

	TSTR name = TSTR(_T("M_")) + node->GetName();
	SetMorphTargetPacket pckt(node->GetObjectRef(),name);
	obj->morphCont->SetValue(0,&pckt);
	
	node->SetObjectRef(obj);
	
	theHold.Put(new CreateMorphNotify(obj,0));

	theHold.Accept(IDS_DS_CREATE);

	obj->BeginEditParams(i,BEGIN_EDIT_CREATE,NULL);
	}

void CreateMorphMode::End(IObjParam *i)
	{
	svNode = node;
	if (obj) obj->EndEditParams(i,END_EDIT_REMOVEUI,NULL);
	DeleteAllRefsFromMe();
	ip  = NULL;
	obj = NULL;
	}

void CreateMorphMode::JumpStart(IObjParam *i,MorphObject *o)
	{
	ip  = i;
	obj = o;
	//MakeRefByID(FOREVER,0,svNode);
	obj->BeginEditParams(i,BEGIN_EDIT_CREATE,NULL);
	}

int MorphObjClassDesc::BeginCreate(Interface *i)
	{	
	SuspendSetKeyMode();
	assert(i->GetSelNodeCount()==1);

	theCreateMorphMode.Begin(i->GetSelNode(0),(IObjParam*)i);
	i->PushCommandMode(&theCreateMorphMode);
	return TRUE;
	}

int MorphObjClassDesc::EndCreate(Interface *i)
	{
	ResumeSetKeyMode();
	theCreateMorphMode.End((IObjParam*)i);
	i->RemoveMode(&theCreateMorphMode);
	return TRUE;
	}

//----------------------------------------------------------------------

BOOL PickTarget::Filter(INode *node)
	{
	if (node) {
		if (mo->morphCont->ValidTarget(
				mo->ip->GetTime(),
				node->GetObjectRef())) {			
			return TRUE;
			}
		}

	return FALSE;
	}

BOOL PickTarget::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
	{
	INode *node = ip->PickNode(hWnd,m,this);
	
	if (node) {
		if (mo->morphCont->ValidTarget(
				mo->ip->GetTime(),
				node->GetObjectRef())) {			
			return TRUE;
			}
		}

	return FALSE;
	}

BOOL PickTarget::Pick(IObjParam *ip,ViewExp *vpt)
	{
	INode *node = vpt->GetClosestHit();
	assert(node);
	if (mo->morphCont->
			ValidTarget(
				mo->ip->GetTime(),
				node->GetObjectRef())) {
		mo->AddNewTarget(node,mo->ip->GetTime(),mo->AddTargMethod());
		mo->SetupTargetList();
		ip->RedrawViews(ip->GetTime());
		}
	return FALSE;
	}

void PickTarget::EnterMode(IObjParam *ip)
	{
	ICustButton *iBut = GetICustButton(GetDlgItem(mo->hParams1,IDC_PICK_MORPHTARG));
	iBut->SetCheck(TRUE);
	ReleaseICustButton(iBut);
	}

void PickTarget::ExitMode(IObjParam *ip)
	{
	ICustButton *iBut = GetICustButton(GetDlgItem(mo->hParams1,IDC_PICK_MORPHTARG));
	iBut->SetCheck(FALSE);
	ReleaseICustButton(iBut);
	}



//----------------------------------------------------------------------

MorphObject::MorphObject(BOOL loading)
	{
	sel = -1;
	morphCont = NULL;
	if (!loading) 
		MakeRefByID(FOREVER,0,newDefaultMorphControl());	
	}

MorphObject::~MorphObject()
	{
	DeleteAllRefsFromMe();
	}
		

class MorphObjCreateCallBack: public CreateMouseCallBack {	
	MorphObject *ob;	
	public:
		int proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
		void SetObj(MorphObject *obj) {ob = obj;}
	};

int MorphObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
	Point3 pt;

	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {
			case 0:  // only happens with MOUSE_POINT msg
				pt = vpt->GetPointOnCP(m);
				mat.SetTrans(pt);
				break;
			case 1:								
				pt = vpt->GetPointOnCP(m);
				mat.SetTrans(pt);
				if (msg==MOUSE_POINT) 
					return CREATE_STOP;
				break;
			}
		}
	else
	if (msg == MOUSE_ABORT)
		return CREATE_ABORT;
	return TRUE;
	}

static MorphObjCreateCallBack morphCreateCB;

CreateMouseCallBack* MorphObject::GetCreateMouseCallBack()
	{
	morphCreateCB.SetObj(this);
	return &morphCreateCB;
	}

Interval MorphObject::ObjectValidity(TimeValue t)
	{	
	return Interval(t,t);
	}

int MorphObject::CanConvertToType(Class_ID obtype)
	{
	ObjectState os = Eval(0);
	return os.obj->CanConvertToType(obtype);
	}

Object* MorphObject::ConvertToType(TimeValue t, Class_ID obtype)
	{
	ObjectState os = Eval(t);
	Object *obj = os.obj->ConvertToType(t,obtype);
	if (!obj) return NULL;
	return (Object*)obj->Clone();
	}

BOOL MorphObject::PolygonCount(TimeValue t, int& faceCount, int& vertCount)
	{
	ObjectState os = Eval(t);
	Object *obj = os.obj;
	if (!obj) return FALSE;
	return obj->PolygonCount(t, faceCount, vertCount);
	}

ObjectState MorphObject::Eval(TimeValue time)
	{
	ObjectState os;
	Interval valid;
	morphCont->GetValue(time,&os,valid);	
	return os;
	}

void MorphObject::AddNewTarget(INode *node,TimeValue t,int m)
	{
	Object *obj = NULL;
	BOOL delnode = FALSE;

	switch (m) {
		case TARG_REF:
			obj = MakeObjectDerivedObject(node->GetObjectRef());
			break;
		
		case TARG_INSTANCE:
			obj = node->GetObjectRef();
			break;

		case TARG_COPY:
			obj = (Object*)node->GetObjectRef()->Clone();
			break;

		case TARG_MOVE:
			obj = node->GetObjectRef();
			delnode = TRUE;
			break;

		default:
			return;
		}

	theHold.Begin();

	TSTR name = TSTR(_T("M_")) + node->GetName();
	Matrix3 tm = node->GetObjectTM(t)*Inverse(node->GetNodeTM(t));
	SetMorphTargetPacket pckt(obj,name,tm);
	morphCont->SetValue(t,&pckt);

	if (delnode && ip) {
		ip->DeleteNode(node);
		}

	theHold.Accept(GetString(IDS_RB_PICKMORPHTARGET));
	}

void MorphObject::SetupTargetList()
	{
	if (!morphCont) return;
	SendMessage(GetDlgItem(hParams2,IDC_MORPHTARG_LIST),LB_RESETCONTENT,0,0);
	for (int i=0; i<morphCont->NumMorphTargs(); i++) {
		SendMessage(GetDlgItem(hParams2,IDC_MORPHTARG_LIST),
			LB_ADDSTRING,0,(LPARAM)(TCHAR*)SubAnimName(i));
		}
	if (sel>=0) {
		SendMessage(GetDlgItem(hParams2,IDC_MORPHTARG_LIST),
			LB_SETCURSEL,sel,0);
		EnableWindow(GetDlgItem(hParams2,IDC_CREATE_MORPHKEY),TRUE);
		EnableWindow(GetDlgItem(hParams2,IDC_DELETE_MORPHTARG),TRUE);

		ICustEdit *edit = GetICustEdit(GetDlgItem(hParams2,IDC_MORPHTARG_NAME));
		edit->Enable();
		edit->SetText(SubAnimName(sel));
		ReleaseICustEdit(edit);
	} else {
		EnableWindow(GetDlgItem(hParams2,IDC_CREATE_MORPHKEY),FALSE);
		EnableWindow(GetDlgItem(hParams2,IDC_DELETE_MORPHTARG),FALSE);
		
		ICustEdit *edit = GetICustEdit(GetDlgItem(hParams2,IDC_MORPHTARG_NAME));
		edit->Disable();
		edit->SetText(_T(""));
		ReleaseICustEdit(edit);
		}
	}

int MorphObject::AddTargMethod()
	{
	if (IsDlgButtonChecked(hParams1,IDC_TARG_REFERENCE)) {
		return TARG_REF;
		}
	if (IsDlgButtonChecked(hParams1,IDC_TARG_COPY)) {
		return TARG_COPY;
		}
	if (IsDlgButtonChecked(hParams1,IDC_TARG_INSTANCE)) {
		return TARG_INSTANCE;
		}
	if (IsDlgButtonChecked(hParams1,IDC_TARG_MOVE)) {
		return TARG_MOVE;
		}
	return TARG_INSTANCE;	
	}

void MorphObject::SetTargMethod(int m)
	{
	CheckDlgButton(hParams1,IDC_TARG_REFERENCE,FALSE);
	CheckDlgButton(hParams1,IDC_TARG_COPY,FALSE);
	CheckDlgButton(hParams1,IDC_TARG_INSTANCE,FALSE);
	CheckDlgButton(hParams1,IDC_TARG_MOVE,FALSE);
	switch (m) {
		case TARG_REF:
			CheckDlgButton(hParams1,IDC_TARG_REFERENCE,TRUE);
			break;
		case TARG_COPY:
			CheckDlgButton(hParams1,IDC_TARG_COPY,TRUE);
			break;
		case TARG_INSTANCE:
			CheckDlgButton(hParams1,IDC_TARG_INSTANCE,TRUE);
			break;
		case TARG_MOVE:
			CheckDlgButton(hParams1,IDC_TARG_MOVE,TRUE);
			break;
		}
	}

static INT_PTR CALLBACK MorphParamDlgProc1( 
		HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
	MorphObject *mo = (MorphObject*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	if (!mo && message!=WM_INITDIALOG) return FALSE;

	switch (message) {
		case WM_INITDIALOG: {
			mo = (MorphObject*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			mo->hParams1 = hWnd;			
			
			ICustButton *iBut = GetICustButton(GetDlgItem(hWnd,IDC_PICK_MORPHTARG));
			iBut->SetType(CBT_CHECK);
			iBut->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(iBut);
			
			mo->SetTargMethod(mo->addTargMethod);
			break;
			}

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_PICK_MORPHTARG:
					if (mo->ip->GetCommandMode()->ID() == CID_STDPICK) {
						if (mo->creating) {
							theCreateMorphMode.JumpStart(mo->ip,mo);
							mo->ip->SetCommandMode(&theCreateMorphMode);
						} else {
							mo->ip->SetStdCommandMode(CID_OBJMOVE);
							}
					} else {
						mo->pickCB.mo = mo;
						mo->ip->SetPickMode(&mo->pickCB);						
						}
					break;				
				}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			mo->ip->RollupMouseMessage(hWnd,message,wParam,lParam);
			return FALSE;
		
		default:
			return FALSE;
		}
	return TRUE;
	}

static INT_PTR CALLBACK MorphParamDlgProc2( 
		HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
	MorphObject *mo = (MorphObject*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	if (!mo && message!=WM_INITDIALOG) return FALSE;

	switch (message) {
		case WM_INITDIALOG: {
			mo = (MorphObject*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			mo->hParams2 = hWnd;
			mo->SetupTargetList();			
			break;
			}

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_CREATE_MORPHKEY: {
					TSTR name;
					Matrix3 tm = mo->morphCont->GetMorphTargTM(mo->sel);
					SetMorphTargetPacket pckt(
						mo->morphCont->GetMorphTarg(mo->sel),name,
						tm,TRUE);
					theHold.Begin();
					mo->morphCont->SetValue(mo->ip->GetTime(),&pckt);
					theHold.Accept(GetString(IDS_RB_CREATEMORPHKEY));
					mo->ip->RedrawViews(mo->ip->GetTime());
					break;
					}

				case IDC_MORPHTARG_LIST:
					if (HIWORD(wParam)==LBN_SELCHANGE) {
						mo->sel = SendMessage(
							GetDlgItem(mo->hParams2,IDC_MORPHTARG_LIST),
							LB_GETCURSEL,0,0);						
						if (mo->sel < 0) {
							EnableWindow(GetDlgItem(hWnd,IDC_CREATE_MORPHKEY),FALSE);
							EnableWindow(GetDlgItem(hWnd,IDC_DELETE_MORPHTARG),FALSE);
							ICustEdit *edit = GetICustEdit(GetDlgItem(hWnd,IDC_MORPHTARG_NAME));
							edit->Disable();
							edit->SetText(_T(""));
							ReleaseICustEdit(edit);
						} else {
							EnableWindow(GetDlgItem(hWnd,IDC_CREATE_MORPHKEY),TRUE);
							EnableWindow(GetDlgItem(hWnd,IDC_DELETE_MORPHTARG),TRUE);
							ICustEdit *edit = GetICustEdit(GetDlgItem(hWnd,IDC_MORPHTARG_NAME));
							edit->Enable();
							edit->SetText(mo->SubAnimName(mo->sel));
							ReleaseICustEdit(edit);
							}
						mo->NotifyDependents(FOREVER,(PartID) mo,REFMSG_BRANCHED_HISTORY_CHANGED);
						}
					break;

				case IDC_DELETE_MORPHTARG:
					if (mo->sel>=0) {
						theHold.Begin();
						mo->morphCont->DeleteMorphTarg(mo->sel);
						theHold.Accept(GetString(IDS_RB_DELETEMORPHTARG));
						mo->sel = -1;
						mo->SetupTargetList();
						mo->ip->RedrawViews(mo->ip->GetTime());
						//GetSystemSetting(SYSSET_CLEAR_UNDO);
						}
					break;					
				}
			break;

		case WM_CUSTEDIT_ENTER:
			if (LOWORD(wParam)==IDC_MORPHTARG_NAME && mo->sel >= 0) {
				ICustEdit *edit = GetICustEdit(GetDlgItem(hWnd,IDC_MORPHTARG_NAME));
				TCHAR buf[256];
				edit->GetText(buf,256);
				mo->morphCont->SetMorphTargName(mo->sel,TSTR(buf));
				mo->SetupTargetList();
				mo->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
				ReleaseICustEdit(edit);				
				}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			mo->ip->RollupMouseMessage(hWnd,message,wParam,lParam);
			return FALSE;
		
		default:
			return FALSE;
		}
	return TRUE;
	}

void MorphObject::BeginEditParams(
		IObjParam *ip, ULONG flags,Animatable *prev)
	{	
	this->ip = ip;
	editOb   = this;
	//pickMode = new PickTargCommandMode(this,ip);	
	
	if (flags&BEGIN_EDIT_CREATE) {
		creating = TRUE;
	} else {
		creating = FALSE;
		}
	if (!hParams1) {
		hParams1 = ip->AddRollupPage( 
				hInstance, 
				MAKEINTRESOURCE(IDD_MORPHPARAM1),
				MorphParamDlgProc1, 
				GetString(IDS_RB_PICKTARGETS), 
				(LPARAM)this);		
		ip->RegisterDlgWnd(hParams1);
		hParams2 = ip->AddRollupPage( 
				hInstance, 
				MAKEINTRESOURCE(IDD_MORPHPARAM2),
				MorphParamDlgProc2, 
				GetString(IDS_RB_CURRENTTARGETS), 
				(LPARAM)this);		
		ip->RegisterDlgWnd(hParams1);
	} else {
		SetWindowLongPtr(hParams1,GWLP_USERDATA,(LONG_PTR)this);
		SetWindowLongPtr(hParams2,GWLP_USERDATA,(LONG_PTR)this);
		SetupTargetList();
		SetTargMethod(addTargMethod);
		}
	}

void MorphObject::EndEditParams(
		IObjParam *ip, ULONG flags,Animatable *next)
	{
	//ip->DeleteMode(pickMode);
	//delete pickMode;
	//pickMode = NULL;
	ip->ClearPickMode();
	editOb   = NULL;
	creating = FALSE;

	addTargMethod = AddTargMethod();

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
	ip = NULL;
	}

int MorphObject::NumPipeBranches(bool selected) 
	{
	if(!selected)
		return morphCont->NumMorphTargs();

	if (sel>=0) return 1;
	else return 0;
	}

Object *MorphObject::GetPipeBranch(int i, bool selected) 
	{
	if(!selected) {
		assert(i>=0&&i<morphCont->NumMorphTargs());
		return morphCont->GetMorphTarg(i);
	}

	assert(sel>=0&&sel<morphCont->NumMorphTargs());
	return morphCont->GetMorphTarg(sel);
	}

int MorphObject::NumSubs()
	{
	return morphCont->NumMorphTargs()+1;
	}

Animatable* MorphObject::SubAnim(int i)
	{
	if (i==morphCont->NumMorphTargs()) {
		return morphCont;
	} else {
		return morphCont->GetMorphTarg(i);
		}
	}

TSTR MorphObject::SubAnimName(int i)
	{	
	if (i>=morphCont->NumMorphTargs()) {
		return GetString(IDS_RB_MORPH);
	} else {		
		TSTR name;
		morphCont->GetMorphTargName(i,name);		
		return name;
		}
	}

BOOL MorphObject::AssignController(Animatable *control,int subAnim)
	{
	if (subAnim==morphCont->NumMorphTargs()) {
		ReplaceReference(0,(ReferenceTarget*)control);
		NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
		if (editOb==this) SetupTargetList();
		return TRUE;
		}
	return FALSE;
	}

RefTargetHandle MorphObject::Clone(RemapDir& remap)
	{
	MorphObject *obj = new MorphObject(TRUE);
	obj->ReplaceReference(0,morphCont->Clone(remap));
	obj->sel = sel;
	BaseClone(this, obj, remap);
	return obj;
	}

RefResult MorphObject::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
		PartID& partID, 
		RefMessage message) 
	{
	switch (message) {
		case REFMSG_SELECT_BRANCH: {
			sel = morphCont->GetFlaggedTarget();
			break;
			}

		case REFMSG_SUBANIM_STRUCTURE_CHANGED:
			if (editOb==this) SetupTargetList();
			break;
		}
	return REF_SUCCEED;
	}

#endif // NO_OBJECT_MORPH