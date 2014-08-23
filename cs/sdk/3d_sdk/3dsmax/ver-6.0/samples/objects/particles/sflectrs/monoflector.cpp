#include "MonoflectDialog.h"
#include "sflectr.h"

#define PLANAR	0
#define SPHERE	1
#define	MESH	2

#define MONODEF_CUSTNAME_CHUNK	0x0100

static TriObject *IsUseable(Object *pobj,TimeValue t)
{ 
	if (pobj->SuperClassID()==GEOMOBJECT_CLASS_ID)
	{	if (pobj->IsSubClassOf(triObjectClassID)) 
			return (TriObject*)pobj;
		else 
		{	if (pobj->CanConvertToType(triObjectClassID)) 
	  		return (TriObject*)pobj->ConvertToType(t,triObjectClassID);			
		}
	}
	return NULL;
}

//--- ClassDescriptor and class vars ---------------------------------

class BasicFlectorModClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 0;}
	void *			Create(BOOL loading = FALSE) { return new BasicFlectorMod();}
	const TCHAR *	ClassName() {return GetString(IDS_EP_BASICDEFLECTORMOD);}
	SClass_ID		SuperClassID() {return WSM_CLASS_ID; }
	Class_ID		ClassID() {return BASICFLECTORMOD_CLASSID;}
	const TCHAR* 	Category() {return _T("");}
	};

static BasicFlectorModClassDesc BasicFlectorModDesc;
ClassDesc* GetBasicFlectorModDesc() {return &BasicFlectorModDesc;}

IObjParam* BasicFlectorObj::ip = NULL;

class FlectorPickOperand : 
		public PickModeCallback,
		public PickNodeCallback {
	public:		
		BasicFlectorObj *po;
		
		FlectorPickOperand() {po=NULL;}

		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);

		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);

		BOOL RightClick(IObjParam *ip, ViewExp *vpt) { return TRUE; }
		BOOL Filter(INode *node);
		
		PickNodeCallback *GetFilter() {return this;}
	};

class CreateFlectorPickNode : public RestoreObj {
	public:   		
		BasicFlectorObj *obj;
		INode *oldn;
		CreateFlectorPickNode(BasicFlectorObj *o, INode *n) {
			obj = o; oldn=n;
			}
		void Restore(int isUndo) {
			INode* cptr=obj->st->pblock2->GetINode(PB_MESHNODE);
			TSTR custname;
			if (cptr) 
			{ custname = TSTR(cptr->GetName());
			}
			else 
			{ custname=TSTR(_T(""));
			}
			obj->st->ShowName();
			}
		void Redo() 
		{ int type;
		  obj->pblock2->GetValue(PB_TYPE,0,type,FOREVER);
		  if ((type==2)&&(obj->st->pmap[pbType_subani]))
			obj->st->ShowName(oldn);
		}
		TSTR Description() {return GetString(IDS_AP_FPICK);}
	};
#define CID_CREATEBasicFlectorMODE	CID_USER +23

FlectorPickOperand BasicFlectorObj::pickCB;

IParamMap2Ptr BasicFlectorObj::pmap[numpblocks]={NULL,NULL};

class CreateBasicFlectorProc : public MouseCallBack,ReferenceMaker 
{
	private:
		IObjParam *ip;
		void Init(IObjParam *i) {ip=i;}
		CreateMouseCallBack *createCB;	
		INode *CloudNode;
		BasicFlectorObj *BasicFlectorObject;
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
		void SetIgnore(BOOL sw) { ignoreSelectionChange = sw; }
		
		CreateBasicFlectorProc()
		{
			ignoreSelectionChange = FALSE;
		}
		int createmethod(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
		int proc( HWND hwnd, int msg, int point, int flag, IPoint2 m );
	};

class CreateBasicFlectorMode : public CommandMode 
{		
	public:		
		CreateBasicFlectorProc proc;
		IObjParam *ip;
		BasicFlectorObj *obj;
		void Begin( IObjCreate *ioc, ClassDesc *desc ) { proc.Begin( ioc, desc ); }
		void End() { proc.End(); }
		void JumpStart(IObjParam *i,BasicFlectorObj*o);

		int Class() {return CREATE_COMMAND;}
		int ID() { return CID_CREATEBasicFlectorMODE; }
		MouseCallBack *MouseProc(int *numPoints) {*numPoints = 10000; return &proc;}
		ChangeForegroundCallback *ChangeFGProc() {return CHANGE_FG_SELECTED;}
		BOOL ChangeFG( CommandMode *oldMode ) { return (oldMode->ChangeFGProc() != CHANGE_FG_SELECTED); }
		void EnterMode() 
		{ GetCOREInterface()->PushPrompt(GetString(IDS_AP_CREATEMODE));
		  SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR)));
		}
		void ExitMode() {GetCOREInterface()->PopPrompt();SetCursor(LoadCursor(NULL, IDC_ARROW));}
	};
static CreateBasicFlectorMode theCreateBasicFlectorMode;
IParamMap2Ptr BasicFlectorType::pmap[numpblocks]={NULL,NULL};
MonoFlectorParamPtr BasicFlectorType::theParam[numpblocks]={NULL,NULL};

class BasicFlectorTypeObjDlgProc : public BasicFlectorDlgProc 
{
	public:
		HWND hw;
		int dtype,which;
		IParamBlock2* pblk;
		BasicFlectorTypeObjDlgProc(BasicFlectorObj *sso_in) { sso = sso_in;st=NULL;dtype=pbType_subani;which=PB_TYPE; }
		void Update(TimeValue t);

		BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

class BasicFlectorComplexDlgProc : public BasicFlectorDlgProc 
{	public:
		HWND hw;
		int dtype,which;
		IParamBlock2* pblk;
		BasicFlectorComplexDlgProc(BasicFlectorObj *sso_in) { sso = sso_in;st=NULL;dtype=pbComplex_subani;which=PB_COMPLEX; }
		void Update(TimeValue t);
		BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

// this classdesc2 for the object
class BasicFlectorObjClassDesc:public ClassDesc2 
{
	public:
	int 			IsPublic()					{ return TRUE; }
	void *			Create( BOOL loading )		{ return new BasicFlectorObj(); }
	const TCHAR *	ClassName()					{ return GetString(IDS_AP_MONONAME); }
	SClass_ID		SuperClassID()				{ return WSM_OBJECT_CLASS_ID; }
	Class_ID 		ClassID()					{ return BASICFLECTOR_CLASSID; }
	const TCHAR* 	Category()					{ return GetString(IDS_EP_SW_DEFLECTORS);  }
	int				BeginCreate(Interface *i);
	int				EndCreate(Interface *i);
	// Hardwired name, used by MAX Script as unique identifier
	const TCHAR*	InternalName()				{ return _T("BasicFlectorObj"); }
	HINSTANCE		HInstance()					{ return hInstance; }
};

static BasicFlectorObjClassDesc BasicFlectorOCD;
ClassDesc* GetBasicFlectorObjDesc() {return &BasicFlectorOCD;}

BOOL BasicFlectorObj::creating    = FALSE;

void BasicFlectorTypeObjDlgProc::Update(TimeValue t)
{	sdlgs = GetDlgItem(hw, IDC_FLECTTYPELIST);
	SetUpList(sdlgs,hw,sso->st->GetNameList(dtype));
	int oldval;pblk->GetValue(which,0,oldval,FOREVER);
	SendMessage(sdlgs, CB_SETCURSEL, oldval, 0);
}

//each one of these takes input from a listbox and creates/destroys the subrollups
BOOL BasicFlectorTypeObjDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int id = LOWORD(wParam);
	pblk=sso->pblock2;
	hw=hWnd;
	switch (msg) 
	{
		case WM_INITDIALOG:
			Update(t);			
			break;
		case WM_DESTROY:
			if (sso->st->theParam[dtype]) 
			{	sso->st->theParam[dtype]->DeleteThis();
				sso->st->theParam[dtype]=NULL;
			}
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_FLECTTYPELIST:
					int curSel = SendMessage(sdlgs, CB_GETCURSEL, 0, 0);
					if (curSel<0) 
						return TRUE; 
					int oldval;	
					pblk->GetValue(which,0,oldval,FOREVER);
					if (oldval!=curSel)
					{	pblk->SetValue(which,0,curSel);
						sso->st->CreateMonoFlectorParamDlg(GetCOREInterface(),curSel,dtype,sso->st->pmap[dtype]->GetHWnd());
					}
					return TRUE;
			}
			break;
	}
	return FALSE;
}
void BasicFlectorComplexDlgProc::Update(TimeValue t)
{ sdlgs = GetDlgItem(hw, IDC_FLECTCOMPLEXITYLIST);
  SetUpList(sdlgs,hw,sso->st->GetNameList(dtype));
  int oldval;pblk->GetValue(which,0,oldval,FOREVER);
  SendMessage(sdlgs, CB_SETCURSEL, oldval, 0);
}

BOOL BasicFlectorComplexDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int id = LOWORD(wParam);
	pblk=sso->pbComplex;hw=hWnd;
	switch (msg) 
	{
		case WM_INITDIALOG:
			  Update(t);
			break;
		case WM_DESTROY:
			if (sso->st->theParam[dtype]) 
			{	sso->st->theParam[dtype]->DeleteThis();sso->st->theParam[dtype]=NULL;
			}
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_FLECTCOMPLEXITYLIST:
					int curSel = SendMessage(sdlgs, CB_GETCURSEL, 0, 0);
					if (curSel<0) 
						return TRUE; 
					int oldval;	pblk->GetValue(which,0,oldval,FOREVER);
					if (oldval!=curSel)
					{	pblk->SetValue(which,0,curSel);
						sso->st->CreateMonoFlectorParamDlg(GetCOREInterface(),curSel,dtype,sso->st->pmap[dtype]->GetHWnd());
					}
					return TRUE;
			}
			break;
	}
	return FALSE;
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

BOOL FlectorPickOperand::Filter(INode *node)
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

BOOL FlectorPickOperand::HitTest(
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

BOOL FlectorPickOperand::Pick(IObjParam *ip,ViewExp *vpt)
{	
	BOOL groupflag=0;
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
	theHold.Put(new CreateFlectorPickNode(po,node));
    po->st->pblock2->SetValue(PB_MESHNODE,ip->GetTime(),node);
//    po->st->pblock2->SetINode(PB_MESHNODE,node);
/*
	if (po->custnode) 
		po->ReplaceReference(CUSTNODE,node,TRUE);
	else 
		po->MakeRefByID(FOREVER,CUSTNODE,node);	
*/
	theHold.Accept(GetString(IDS_AP_FPICK));
	po->st->ShowName(node);
	if (po->creating) 
	{
		theCreateBasicFlectorMode.JumpStart(ip,po);
		ip->SetCommandMode(&theCreateBasicFlectorMode);
		ip->RedrawViews(ip->GetTime());
		return FALSE;
	} 
	else 
	{
		return TRUE;
	}
}

void FlectorPickOperand::EnterMode(IObjParam *ip)
{
	ICustButton *iBut;
	iBut=GetICustButton(GetDlgItem(po->st->hParams,IDC_EP_PICKBUTTON));
	if (iBut) iBut->SetCheck(TRUE);
	ReleaseICustButton(iBut);
	GetCOREInterface()->PushPrompt(GetString(IDS_AP_PICKMODE));
}

void FlectorPickOperand::ExitMode(IObjParam *ip)
	{
	ICustButton *iBut;
	iBut=GetICustButton(GetDlgItem(po->st->hParams,IDC_EP_PICKBUTTON));
	if (iBut) iBut->SetCheck(FALSE);
	ReleaseICustButton(iBut);
    GetCOREInterface()->PopPrompt();
	}

//pb2s for each static dialog
static ParamBlockDesc2 BasicFlectorPB 
(	pbType_subani,
	_T("BasicFlectorParams"),
	0, 	
	&BasicFlectorOCD, 
	P_AUTO_CONSTRUCT + P_AUTO_UI, 	
	pbType_subani, 

	//rollout
	IDD_MF_0100_FLECTTYPESELECT, IDS_DLG_FTYPE, 0, 0, NULL, 

	// params
	PB_TYPE, _T("FlectorType"), TYPE_INT, 0, IDS_FLECTORTYPETBLE,
		end,

//watje ref to hold the collision engine
	monoflect_colliderp,  _T("colliderplane"),		TYPE_REFTARG, 	0, 	IDS_FLECTORTYPETBLE, 
		end, 

	monoflect_colliders,  _T("collidersphere"),	TYPE_REFTARG, 	0, 	IDS_FLECTORTYPETBLE, 
		end, 

	monoflect_colliderm,  _T("collidermesh"),		TYPE_REFTARG, 	0, 	IDS_FLECTORTYPETBLE, 
		end, 

	end
);

static ParamBlockDesc2 BasicFComplexPB 
(	pbComplex_subani,
	_T("FlectorComplexityParams"),
	0, 
	&BasicFlectorOCD, 
	P_AUTO_CONSTRUCT + P_AUTO_UI, 	
	pbComplex_subani, 

	//rollout
	IDD_MF_0200_FLECTCOMPLEXITYSELECT, IDS_DLG_FCOMPLEX, 0, 0, NULL, 

	// params
	PB_COMPLEX, _T("FlectorComplex"), TYPE_INT, 0, IDS_FLECTORCOMPTBLE,
		end,

	end
);


RefResult CreateBasicFlectorProc::NotifyRefChanged(
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
		 	if ( BasicFlectorObject && CloudNode==hTarget ) {
				// this will set camNode== NULL;
				theHold.Suspend();
				DeleteReference(0);
				theHold.Resume();
				goto endEdit;
				}
			// fall through

		case REFMSG_TARGET_DELETED:		
			if (BasicFlectorObject && CloudNode==hTarget ) {
				endEdit:
				if (createInterface->GetCommandMode()->ID() == CID_STDPICK) 
				{ if (BasicFlectorObject->creating) 
						{  theCreateBasicFlectorMode.JumpStart(BasicFlectorObject->ip,BasicFlectorObject);
							createInterface->SetCommandMode(&theCreateBasicFlectorMode);
					    } 
				  else {createInterface->SetStdCommandMode(CID_OBJMOVE);}
				}
#ifdef _OSNAP
				BasicFlectorObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
#ifndef NO_CREATE_TASK	// russom - 12/04/01
				BasicFlectorObject->EndEditParams( (IObjParam*)createInterface, 0, NULL);
#endif
				BasicFlectorObject  = NULL;				
				CloudNode    = NULL;
				CreateNewObject();	
				attachedToNode = FALSE;
				}
			break;		
		}
	return REF_SUCCEED;
	}

void AddMesh(BasicFlectorObj *obj, TriObject *triOb, Matrix3 tm, BOOL nottop)
{	int lastv = obj->nv,
	    lastf = obj->nf;
	obj->nv += triOb->GetMesh().getNumVerts();
	obj->nf += triOb->GetMesh().getNumFaces();
	if (!nottop)
		obj->dmesh->DeepCopy(&triOb->GetMesh(),PART_GEOM|PART_TOPO);
	else
	{	
		obj->dmesh->setNumFaces(obj->nf,obj->dmesh->getNumFaces());
		obj->dmesh->setNumVerts(obj->nv,obj->dmesh->getNumVerts());
		tm = tm*obj->invtm;
		for (int vc=0;vc<triOb->GetMesh().getNumFaces();vc++)
		{	obj->dmesh->faces[lastf]=triOb->GetMesh().faces[vc];
			for (int vs=0;vs<3;vs++) 
				obj->dmesh->faces[lastf].v[vs]+=lastv;
			lastf++;
		}
	}
	for (int vc=0;vc<triOb->GetMesh().getNumVerts();vc++)
	{	if (nottop) 
			obj->dmesh->verts[lastv]=triOb->GetMesh().verts[vc]*tm;
		else 
			obj->dmesh->verts[lastv]=triOb->GetMesh().verts[vc];
		lastv++;
	}
}  

void CreateBasicFlectorProc::Begin( IObjCreate *ioc, ClassDesc *desc )
	{					   
	ip=(IObjParam*)ioc;
	createInterface = ioc;
	cDesc           = desc;
	attachedToNode  = FALSE;
	createCB        = NULL;
	CloudNode         = NULL;
	BasicFlectorObject       = NULL;
	CreateNewObject();
	}
void CreateBasicFlectorProc::CreateNewObject()
	{
	SuspendSetKeyMode();
    createInterface->GetMacroRecorder()->BeginCreate(cDesc);
	BasicFlectorObject = (BasicFlectorObj*)cDesc->Create();
	lastPutCount  = theHold.GetGlobalPutCount();
	
	// Start the edit params process
	if ( BasicFlectorObject ) {
#ifndef NO_CREATE_TASK	// russom - 12/04/01
		BasicFlectorObject->BeginEditParams( (IObjParam*)createInterface, BEGIN_EDIT_CREATE, NULL );
#endif
		BasicFlectorObject->SetAFlag(A_OBJ_CREATING);
#ifdef _OSNAP
		BasicFlectorObject->SetAFlag(A_OBJ_LONG_CREATE);
#endif
			}
	ResumeSetKeyMode();
	}

//LACamCreationManager::~LACamCreationManager
void CreateBasicFlectorProc::End()
{ if ( BasicFlectorObject ) 
	{ 
#ifdef _OSNAP
		BasicFlectorObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
#ifndef NO_CREATE_TASK	// russom - 12/04/01
	BasicFlectorObject->EndEditParams( (IObjParam*)createInterface, 
	                    	          END_EDIT_REMOVEUI, NULL);
#endif
		if ( !attachedToNode ) 
		{	// RB 4-9-96: Normally the hold isn't holding when this 
			// happens, but it can be in certain situations (like a track view paste)
			// Things get confused if it ends up with undo...
			theHold.Suspend(); 
			BasicFlectorObject->DeleteAllRefsFromMe();
			BasicFlectorObject->DeleteAllRefsToMe();
			theHold.Resume();
			BasicFlectorObject->DeleteThis();
			BasicFlectorObject = NULL;
			createInterface->GetMacroRecorder()->Cancel();
			if (theHold.GetGlobalPutCount()!=lastPutCount) 
				GetSystemSetting(SYSSET_CLEAR_UNDO);
		}  else if ( CloudNode ) 
		{	theHold.Suspend();
			DeleteReference(0);  // sets cloudNode = NULL
			theHold.Resume();  }
	}
}

void CreateBasicFlectorMode::JumpStart(IObjParam *i,BasicFlectorObj *o)
	{
	ip  = i;
	obj = o;
	//MakeRefByID(FOREVER,0,svNode);
#ifndef NO_CREATE_TASK	// russom - 12/04/01
	obj->BeginEditParams(i,BEGIN_EDIT_CREATE,NULL);
#endif
	}


int BasicFlectorObjClassDesc::BeginCreate(Interface *i)
	{
	SuspendSetKeyMode();
	IObjCreate *iob = i->GetIObjCreate();
	theCreateBasicFlectorMode.Begin(iob,this);
	iob->PushCommandMode(&theCreateBasicFlectorMode);
	return TRUE;
	}

int BasicFlectorObjClassDesc::EndCreate(Interface *i)
	{
	ResumeSetKeyMode();
	theCreateBasicFlectorMode.End();
	i->RemoveMode(&theCreateBasicFlectorMode);
	macroRec->EmitScript();  // 10/00
	return TRUE;
	}
int CreateBasicFlectorProc::proc(HWND hwnd,int msg,int point,int flag,
				IPoint2 m )
{	int res=TRUE;	
	ViewExp *vpx = createInterface->GetViewport(hwnd); 
	assert( vpx );

#ifdef _3D_CREATE
	DWORD snapdim = SNAP_IN_3D;
#else
	DWORD snapdim = SNAP_IN_PLANE;
#endif

	switch ( msg ) {
		case MOUSE_POINT:
			switch ( point ) {
				case 0:
					assert( BasicFlectorObject );					
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
						BasicFlectorObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
#ifndef NO_CREATE_TASK	// russom - 12/04/01
				   		BasicFlectorObject->EndEditParams( (IObjParam*)createInterface, 0, NULL);
#endif
              createInterface->GetMacroRecorder()->EmitScript();
						// Get rid of the reference.
						if (CloudNode) {
							theHold.Suspend();
							DeleteReference(0);
							theHold.Resume();
							}
						// new object
						CreateNewObject();   // creates BasicFlectorObject
						}

				   	theHold.Begin();	 // begin hold for undo
					mat.IdentityMatrix();

					// link it up
					CloudNode = createInterface->CreateObjectNode( BasicFlectorObject);
					attachedToNode = TRUE;
          BasicFlectorObject->ClearAFlag(A_OBJ_CREATING);
 					assert( CloudNode );					
					createCB = NULL;
					createInterface->SelectNode( CloudNode );
					
					// Reference the new node so we'll get notifications.
					theHold.Suspend();
					MakeRefByID( FOREVER, 0, CloudNode);
					theHold.Resume();

					mat.SetTrans(vpx->SnapPoint(m,m,NULL,snapdim));

					macroRec->Disable();   // 10/00
					createInterface->SetNodeTMRelConstPlane(CloudNode, mat);
					macroRec->Enable();
				default:				
						res = createmethod(vpx,msg,point,flag,m,mat);
						createInterface->SetNodeTMRelConstPlane(CloudNode, mat);

						if (res==CREATE_ABORT)
							goto abort;
						if (res==CREATE_STOP)
						{
#ifdef _OSNAP
                         BasicFlectorObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
						 theHold.Accept(GetString(IDS_EP_CREATE));	 
						}
						createInterface->RedrawViews(createInterface->GetTime());   //DS

					break;
					
				}			
			break;

		case MOUSE_MOVE:
				res = createmethod(vpx,msg,point,flag,m,mat);
				macroRec->Disable();   // 10/2/00
				createInterface->SetNodeTMRelConstPlane(CloudNode, mat);
				macroRec->Enable();
				if (res==CREATE_ABORT) 
					goto abort;
				if (res==CREATE_STOP)
				{
#ifdef _OSNAP
         BasicFlectorObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
					theHold.Accept(GetString(IDS_EP_CREATE));	// TH
				}
				createInterface->RedrawViews(createInterface->GetTime(),REDRAW_INTERACTIVE);		//DS		
//				macroRec->SetProperty(BasicFlectorObject, _T("target"),   // JBW 4/23/99
//				mr_create, Class_ID(TARGET_CLASS_ID, 0), GEOMOBJECT_CLASS_ID, 1, _T("transform"), mr_matrix3, &mat);
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
	  case MOUSE_ABORT: 
		 abort:
		assert( BasicFlectorObject );
#ifdef _OSNAP
		BasicFlectorObject->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
#ifndef NO_CREATE_TASK	// russom - 12/04/01
		BasicFlectorObject->EndEditParams( (IObjParam*)createInterface,0,NULL);
#endif
		theHold.Cancel();	 // deletes both the Cloudera and target.
		if (theHold.GetGlobalPutCount()!=lastPutCount) 
			GetSystemSetting(SYSSET_CLEAR_UNDO);
		BasicFlectorObject=NULL;
		createInterface->RedrawViews(createInterface->GetTime());
		CreateNewObject();	
		attachedToNode = FALSE;
		res = FALSE;
	}

	done:
	if ((res == CREATE_STOP)||(res==CREATE_ABORT))
		vpx->ReleaseImplicitGrid();
	createInterface->ReleaseViewport(vpx); 
	return res;
}


void BasicFlectorDlgProc::SetUpList(HWND cwnd,HWND hWnd,dlglist ilist)
{ SendMessage(cwnd,CB_RESETCONTENT,0,0);
  for (int i=0; i<ilist.cnt; i++) {
	SendMessage(cwnd,CB_ADDSTRING,0,(LPARAM)(TCHAR*)GetString(ilist.namelst[i]));
	}
}

BasicFlectorObj::BasicFlectorObj()
{
	gf = NULL;
	mf = NULL;
	BasicFlectorOCD.MakeAutoParamBlocks(this);
	assert(pblock2);
	assert(pbComplex);
	MakeRefByID(FOREVER,monoflecdlg,new MonoFlector(this));

	int tpf=GetTicksPerFrame();
	int timeoff=100*tpf;

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
	pblock2->SetValue(PB_TYPE,0,0);
	macroRec->Disable();

//watje create a new ref to our collision engine
	CollisionPlane *colp = (CollisionPlane*)CreateInstance(REF_MAKER_CLASS_ID, PLANAR_COLLISION_ID);
	if (colp)
	{
		pblock2->SetValue(monoflect_colliderp,0,(ReferenceTarget*)colp);
	}
	CollisionSphere *cols = (CollisionSphere*)CreateInstance(REF_MAKER_CLASS_ID, SPHERICAL_COLLISION_ID);
	if (cols)
	{
		pblock2->SetValue(monoflect_colliders,0,(ReferenceTarget*)cols);
	}
	CollisionMesh *colm = (CollisionMesh*)CreateInstance(REF_MAKER_CLASS_ID, MESH_COLLISION_ID);
	if (colm)
	{
		pblock2->SetValue(monoflect_colliderm,0,(ReferenceTarget*)colm);
	}
	macroRec->Enable();
}

BasicFlectorObj::~BasicFlectorObj()
{ 
	DeleteAllRefsFromMe();
	if (gf)
		delete gf;
	if (mf)
		delete mf;
	if (vnorms) 
		delete[] vnorms;
	if (fnorms) 
		delete[] fnorms;
	if (dmesh) 
		delete dmesh;
	ReferenceTarget *rt;
	if (pblock2)
	{
		pblock2->GetValue(monoflect_colliderp,0,rt,FOREVER);
		if (rt) 
			delete rt;
		pblock2->GetValue(monoflect_colliderm,0,rt,FOREVER);
		if (rt) 
			delete rt;
		pblock2->GetValue(monoflect_colliders,0,rt,FOREVER);
		if (rt) 
			delete rt;
	}
	pblock2 = NULL;
}

void BasicFlectorObj::MapKeys(TimeMap *map,DWORD flags)
{	Animatable::MapKeys(map,flags);
	TimeValue TempTime;
	float ftemp,tpf=GetTicksPerFrame();

	pbComplex->GetValue(PB_TIMEON,0,ftemp,FOREVER);
	TempTime=ftemp*tpf;
	TempTime = map->map(TempTime);
	pbComplex->SetValue(PB_TIMEON,0,((float)ftemp)/tpf);

	pbComplex->GetValue(PB_TIMEOFF,0,ftemp,FOREVER);
	TempTime=ftemp*tpf;
	TempTime = map->map(TempTime);
	pbComplex->SetValue(PB_TIMEOFF,0,((float)ftemp)/tpf);
}  

Modifier *BasicFlectorObj::CreateWSMMod(INode *node)
{
	return new BasicFlectorMod(node,this);
}

void BasicFlectorObj::IntoPickMode()
{  
  if (ip->GetCommandMode()->ID() == CID_STDPICK) 
  { if (creating) 
	{ theCreateBasicFlectorMode.JumpStart(ip,this);
	  ip->SetCommandMode(&theCreateBasicFlectorMode);
	} else {ip->SetStdCommandMode(CID_OBJMOVE);}
  }
  else 
  { pickCB.po = this;						
    ip->SetPickMode(&pickCB);
  }
}

Object *BasicFlectorField::GetSWObject()
{ 
	return obj;
}

BOOL BasicFlectorField::CheckCollision(TimeValue t,Point3 &inp,Point3 &vel,float dt,int index,float *ct, BOOL UpdatePastCollide)
{	
	BOOL donewithparticle = FALSE;
	float K=(float)GetMasterScale(UNITS_CENTIMETERS);
	float stepsize = dt;
	Point3 InVel, SaveVel = vel;
	int typeofdeflector;
	obj->pblock2->GetValue(PB_TYPE,t,typeofdeflector,FOREVER);
	int enableAdvanced, enableDynamics;
	obj->pbComplex->GetValue(PB_COMPLEX,t,enableAdvanced,FOREVER);
	enableDynamics = (enableAdvanced>1?1:0);
	enableAdvanced = (enableAdvanced>0?1:0);
	
	switch(typeofdeflector)
	{	
		case PLANAR:
// PLANAR COLLISION CODE BLOCK BEGINS HERE
		{

	ReferenceTarget *rt;
	obj->pblock2->GetValue(monoflect_colliderp,t,rt,obj->tmValid);
	colp = (CollisionPlane *) rt;
	
	if (!((obj->mValid.InInterval(t))&&(obj->tmValid.InInterval(t))))
	{
		obj->tmValid = FOREVER;

		obj->st->pblock2->GetValue(PB_WIDTH,t,width,obj->tmValid);
		obj->st->pblock2->GetValue(PB_LENGTH,t,height,obj->tmValid);
		obj->st->pblock2->GetValue(PB_QUALITY,t,quality,obj->tmValid);

		if (colp)
		{	colp->SetWidth(t,width);
			colp->SetHeight(t,height);
			colp->SetQuality(t,quality);
			colp->SetNode(t,node);
		}
		if (colp) colp->PreFrame(t,(TimeValue) dt);

		obj->st->pbComplex->GetValue(PB_BOUNCE,t,bounce,obj->tmValid);
		obj->st->pbComplex->GetValue(PB_BVAR,t,bvar,obj->tmValid);
		obj->st->pbComplex->GetValue(PB_CHAOS,t,chaos,obj->tmValid);
		obj->st->pbComplex->GetValue(PB_INHERVEL,t,vinher,obj->tmValid);
		obj->st->pbComplex->GetValue(PB_FRICTION,t,friction,obj->tmValid);

//		vinher *= 0.01f;
//		bvar *= 0.01f;
//		chaos *= 0.01f;
//		friction *= 0.01f;

		obj->st->pbComplex->GetValue(PB_DISTORTION,t,refvol,obj->tmValid);
//		refvol *= 0.01f;
		obj->st->pbComplex->GetValue(PB_DISTORTIONVAR,t,refvar,FOREVER);
//		refvar *= 0.01f;
		obj->st->pbComplex->GetValue(PB_PASSVEL,t,decel,FOREVER);
		obj->st->pbComplex->GetValue(PB_PASSVELVAR,t,decelvar,FOREVER);
//		decelvar *= 0.01f;

		width  *= 0.5f;
		height *= 0.5f;
		Interval tmpValid = FOREVER;
	}

    if ((curtime!=t)&&(enableDynamics))
	{	totalforce = Zero;
		applyat = Zero;
		totalnumber = 0;
		curtime = t;
	}
	float fstartt,fendt;
	TimeValue startt,endt;
	obj->st->pbComplex->GetValue(PB_TIMEON,t,fstartt,FOREVER);
	obj->st->pbComplex->GetValue(PB_TIMEOFF,t,fendt,FOREVER);
	startt=fstartt*GetTicksPerFrame();endt=fendt*GetTicksPerFrame();
	if ((t<startt)||(t>endt))
	{	
		obj->lastrnd=rand();
		obj->ReturnThreeStateValue = DONTCARE;
		return FALSE;
	}
	
	if (!colp)
	{	
		obj->lastrnd=rand();
		obj->ReturnThreeStateValue = DONTCARE;
		return FALSE;
	}
	else
	{
	    srand(obj->lastrnd);
		float reflects;
		obj->st->pbComplex->GetValue(PB_REFLECTS,t,reflects,FOREVER);
//		reflects *= 0.01f;

		if (RND01()<reflects)
		{	
			donewithparticle = TRUE;
			Point3 hitpoint,bnorm,frict,inheritedVel;
			float at;
			BOOL hit = colp->CheckCollision(t,inp,vel,dt,at,hitpoint,bnorm,frict,inheritedVel);

			if (!hit) 
			{
				obj->lastrnd=rand();
				obj->ReturnThreeStateValue = 0;
				return FALSE;
			}

			float holddt = dt;
			dt -= at;
		
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

			vel = bnorm*(bounce*rvariation) + frict*(1.0f-(friction*rchaos)) + (inheritedVel * vinher);
		
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

			vel +=  (inheritedVel * vinher);
			InVel = vel;
			applyat = hitpoint;
		}

// particle was not reflected and not tested for refraction!
		float refracts;
		obj->st->pbComplex->GetValue(PB_REFRACTS,t,refracts,FOREVER);
//		refracts *= 0.01f;
		if ((RND01()<refracts)&&(!donewithparticle)&&(enableAdvanced))
		{
			donewithparticle = TRUE;
			InVel = vel;

			Point3 hitpoint,bnorm,frict,inheritedVel;
			float at;
			BOOL hit = colp->CheckCollision(t,inp,vel,dt,at,hitpoint,bnorm,frict,inheritedVel);

			if (!hit) 
			{
				obj->lastrnd=rand();
				obj->ReturnThreeStateValue = 0;
				return FALSE;
			}

			float holddt = dt;
			dt -= at;

			float dirapproach,theta;
			Point3 ZVec = bnorm;
			dirapproach = (DotProd(InVel,ZVec)<0.0f?1.0f:-1.0f);
			Point3 MZVec = -bnorm; 

			InVel *= decel*(1.0f-decelvar*RND01());

			float maxref,refangle,maxvarref;
			refangle = 0.0f;
			if (!FloatEQ0(refvol))
			{	if (dirapproach>0.0f)
					theta = (float)acos(DotProd(Normalize(-InVel),ZVec));
				else
					theta = (float)acos(DotProd(Normalize(-InVel),MZVec));
				if ((refvol>0.0f)==(dirapproach>0.0f))
					maxref = -theta;
				else 
					maxref = HalfPI-theta;
				refangle = maxref*(float)fabs(refvol);
				float frefangle = (float)fabs(refangle);
				if ((refvol>0.0f)==(dirapproach>0.0f))
					maxvarref = HalfPI-theta-frefangle;
				else
					maxvarref = theta-frefangle;
				refangle += maxvarref*RND11()*refvar;
				Point3 c,d;
				if (theta<0.01f)
				{
					// Martell 4/14/01: Fix for order of ops bug.
					float ztmp=RND11(); float ytmp=RND11(); float xtmp=RND11();
					d = Point3(xtmp,ytmp,ztmp);
					c = Normalize(InVel^d);
				}
				else
				{	if (dirapproach>0.0f)
					 	c = Normalize(ZVec^(-InVel));
					else
				 		c = Normalize(MZVec^(-InVel));
				}
				RotateOnePoint(InVel,&Zero.x,&c.x,refangle);
			}

			float maxdiff,diffuse,diffvar,diffangle;
			obj->st->pbComplex->GetValue(PB_DIFFUSION,t,diffuse,FOREVER);
//			diffuse *= 0.01f;
			obj->st->pbComplex->GetValue(PB_DIFFUSIONVAR,t,diffvar,FOREVER);
//			diffvar *= 0.01f;
			maxdiff = HalfPI-theta-refangle;

			if (!FloatEQ0(diffuse))
			{	
				// Martell 4/14/01: Fix for order of ops bug.
				float ztmp=RND11(); float ytmp=RND11(); float xtmp=RND11();
				Point3 d = Point3(xtmp,ytmp,ztmp);
				Point3 c = Normalize(InVel^d);
				diffangle = 0.5f*maxdiff*diffuse*(1.0f+RND11()*diffvar);
				RotateOnePoint(InVel,&Zero.x,&c.x,diffangle);
			}

			if (UpdatePastCollide)
			{	inp += InVel * dt;  //uses up the rest of the time with the new velocity
				if (ct) 
					(*ct) = holddt;
			}
			else
			{	if (ct) 
					(*ct) = at;
			}

			InVel +=  (inheritedVel * vinher);
			vel = InVel;
			applyat = hitpoint;
		}

// particle was neither reflected nor refracted nor tested for either!
		float spawnonly;
		obj->st->pbComplex->GetValue(PB_COLAFFECTS,t,spawnonly,FOREVER);
//		spawnonly *= 0.01f;
		if ((RND01()<spawnonly)&&(!donewithparticle)&&(enableAdvanced))
		{
			donewithparticle = TRUE;
			InVel = vel;

			Point3 hitpoint,bnorm,frict,inheritedVel;
			float at;
			BOOL hit = colp->CheckCollision(t,inp,vel,dt,at,hitpoint,bnorm,frict,inheritedVel);

			if (!hit) 
			{
				obj->lastrnd=rand();
				obj->ReturnThreeStateValue = 0;
				return FALSE;
			}

			float passvel,passvelvar;
			obj->st->pbComplex->GetValue(PB_COLPASSVEL,t,passvel,FOREVER);
			obj->st->pbComplex->GetValue(PB_COLPASSVELVAR,t,passvelvar,FOREVER);
//			passvelvar *= 0.01f;
			InVel *= passvel*(1.0f+passvelvar*RND11());

			float holddt = dt;
			dt -= at;

			if (UpdatePastCollide)
			{	inp += InVel * dt;  //uses up the rest of the time with the new velocity
				if (ct) 
					(*ct) = holddt;
			}
			else
			{	if (ct) 
					(*ct) = at;
			}

			InVel +=  (inheritedVel * vinher);
			vel = InVel;
			applyat = hitpoint;
		}
	}

		}
// PLANAR COLLISION CODE BLOCK ENDS HERE
			break;

		case SPHERE:
// SPHERE COLLISION CODE BLOCK BEGINS HERE
		{

	ReferenceTarget *rt;
	obj->pblock2->GetValue(monoflect_colliders,t,rt,obj->tmValid);
	cols = (CollisionSphere *) rt;

	if (!((obj->mValid.InInterval(t))&&(obj->tmValid.InInterval(t))))
	{
		obj->tmValid = FOREVER;

		obj->st->pblock2->GetValue(PB_WIDTH,t,width,obj->tmValid);

		if (cols)
		{	
			cols->SetRadius(t,width);
			cols->SetNode(t,node);
			cols->PreFrame(t,(TimeValue) dt);
		}
		
		obj->st->pbComplex->GetValue(PB_BOUNCE,t,bounce,obj->tmValid);
		obj->st->pbComplex->GetValue(PB_BVAR,t,bvar,obj->tmValid);
		obj->st->pbComplex->GetValue(PB_CHAOS,t,chaos,obj->tmValid);
		obj->st->pbComplex->GetValue(PB_INHERVEL,t,vinher,obj->tmValid);
		obj->st->pbComplex->GetValue(PB_FRICTION,t,friction,obj->tmValid);

//		vinher *= 0.01f;
//		bvar *= 0.01f;
//		chaos *= 0.01f;
//		friction *= 0.01f;

		obj->st->pbComplex->GetValue(PB_DISTORTION,t,refvol,obj->tmValid);
//		refvol *= 0.01f;
		obj->st->pbComplex->GetValue(PB_DISTORTIONVAR,t,refvar,FOREVER);
//		refvar *= 0.01f;
		obj->st->pbComplex->GetValue(PB_PASSVEL,t,decel,FOREVER);
		obj->st->pbComplex->GetValue(PB_PASSVELVAR,t,decelvar,FOREVER);
//		decelvar *= 0.01f;

		Interval tmpValid = FOREVER;
	}

    if ((curtime!=t)&&(enableDynamics))
	{	totalforce = Zero;
		applyat = Zero;
		totalnumber = 0;
		curtime = t;
	}
	float fstartt,fendt;
	TimeValue startt,endt;
	obj->st->pbComplex->GetValue(PB_TIMEON,t,fstartt,FOREVER);
	obj->st->pbComplex->GetValue(PB_TIMEOFF,t,fendt,FOREVER);
	startt=fstartt*GetTicksPerFrame();endt=fendt*GetTicksPerFrame();
	if ((t<startt)||(t>endt))
	{	
		obj->lastrnd=rand();
		obj->ReturnThreeStateValue = DONTCARE;
		return FALSE;
	}

	if (!cols)
	{	
		obj->lastrnd=rand();
		obj->ReturnThreeStateValue = DONTCARE;
		return FALSE;
	}
	else
	{
	    srand(obj->lastrnd);
		float reflects;
		obj->st->pbComplex->GetValue(PB_REFLECTS,t,reflects,FOREVER);
//		reflects *= 0.01f;

		if (RND01()<reflects)
		{	
			donewithparticle = TRUE;
			Point3 hitpoint,bnorm,frict,inheritedVel;
			float at;
			BOOL hit = cols->CheckCollision(t,inp,vel,dt,at,hitpoint,bnorm,frict,inheritedVel);
	
			if (!hit) 
			{
				obj->lastrnd=rand();
				obj->ReturnThreeStateValue = 0;
				return FALSE;
			}

			float holddt = dt;
			dt -= at;
		
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

			vel = bnorm*(bounce*rvariation) + frict*(1.0f-(friction*rchaos)) + (inheritedVel * vinher);

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
			vel +=  (inheritedVel * vinher);
			InVel = vel;
			applyat = hitpoint;
		}

// particle was not reflected and not tested for refraction!
		float refracts;
		obj->st->pbComplex->GetValue(PB_REFRACTS,t,refracts,FOREVER);
//		refracts *= 0.01f;
		if ((RND01()<refracts)&&(!donewithparticle)&&(enableAdvanced))
		{
			donewithparticle = TRUE;
			InVel = vel;
	
			Point3 hitpoint,bnorm,frict,inheritedVel;
			float at;
			BOOL hit = cols->CheckCollision(t,inp,vel,dt,at,hitpoint,bnorm,frict,inheritedVel);

			if (!hit) 
			{
				obj->lastrnd=rand();
				obj->ReturnThreeStateValue = 0;
				return FALSE;
			}

			float holddt = dt;
			dt -= at;

			float q1 = DotProd(-InVel,bnorm);
			float theta = (float)acos(q1);
			if (theta>=HalfPI) 
				theta -= PI;
			InVel *= decel*(1.0f-decelvar*RND01());

			float maxref,refangle,maxvarref;
			refangle = 0.0f;
			if (!FloatEQ0(refvol))
			{	if (refvol>0.0f)
					maxref = -theta;
				else 
					maxref = HalfPI-theta;
				refangle = maxref*(float)fabs(refvol);
				float frefangle = (float)fabs(refangle);
				if (refvol>0.0f)
					maxvarref = HalfPI-theta-frefangle;
				else
					maxvarref = theta-frefangle;
				refangle += maxvarref*RND11()*refvar;
				Point3 c,d;
				if (theta<0.01f)
				{	
					// Martell 4/14/01: Fix for order of ops bug.
					float ztmp=RND11(); float ytmp=RND11(); float xtmp=RND11();
					d = Point3(xtmp,ytmp,ztmp);
					c = Normalize(InVel^d);
				}
				else
					c = Normalize(bnorm^(-InVel));
				RotateOnePoint(InVel,&Zero.x,&c.x,refangle);
			}
			float maxdiff,diffuse,diffvar,diffangle;
			obj->st->pbComplex->GetValue(PB_DIFFUSION,t,diffuse,FOREVER);
//			diffuse *= 0.01f;
			obj->st->pbComplex->GetValue(PB_DIFFUSIONVAR,t,diffvar,FOREVER);
//			diffvar *= 0.01f;
			maxdiff = HalfPI-theta-refangle;
			if (!FloatEQ0(diffuse))
			{	
				// Martell 4/14/01: Fix for order of ops bug.
				float ztmp=RND11(); float ytmp=RND11(); float xtmp=RND11();
				Point3 d = Point3(xtmp,ytmp,ztmp);
				Point3 c = Normalize(InVel^d);
				diffangle = 0.5f*maxdiff*diffuse*(1.0f+RND11()*diffvar);
				RotateOnePoint(InVel,&Zero.x,&c.x,diffangle);
			}

			if (UpdatePastCollide)
			{	inp += InVel * dt;  //uses up the rest of the time with the new velocity
				if (ct) 
					(*ct) = holddt;
			}
			else
			{	if (ct) 
					(*ct) = at;
			}

			InVel +=  (inheritedVel * vinher);
			vel = InVel;
			applyat = hitpoint;
		
		
		}

// particle was neither reflected nor refracted nor tested for either!
		float spawnonly;
		obj->st->pbComplex->GetValue(PB_COLAFFECTS,t,spawnonly,FOREVER);
//		spawnonly *= 0.01f;
		if ((RND01()<spawnonly)&&(!donewithparticle)&&(enableAdvanced))
		{
			donewithparticle = TRUE;
			InVel = vel;

			Point3 hitpoint,bnorm,frict,inheritedVel;
			float at;
			BOOL hit = cols->CheckCollision(t,inp,vel,dt,at,hitpoint,bnorm,frict,inheritedVel);

			if (!hit) 
			{
				obj->lastrnd=rand();
				obj->ReturnThreeStateValue = 0;
				return FALSE;
			}

			float passvel,passvelvar;
			obj->st->pbComplex->GetValue(PB_COLPASSVEL,t,passvel,FOREVER);
			obj->st->pbComplex->GetValue(PB_COLPASSVELVAR,t,passvelvar,FOREVER);
//			passvelvar *= 0.01f;
			InVel *= passvel*(1.0f+passvelvar*RND11());

			float holddt = dt;
			dt -= at;

			if (UpdatePastCollide)
			{	inp += InVel * dt;  //uses up the rest of the time with the new velocity
				if (ct) 
					(*ct) = holddt;
			}
			else
			{	if (ct) 
					(*ct) = at;
			}

			InVel +=  (inheritedVel * vinher);
			vel = InVel;
			applyat = hitpoint;
		}
	}

		}
// SPHERE COLLISION CODE BLOCK ENDS HERE
			break;

		case MESH:
// MESH COLLISION CODE BLOCK BEGINS HERE
		{

	if (badmesh) 
	{
		obj->ReturnThreeStateValue = DONTCARE;
		return(0);
	}

	Point3 iw;

	ReferenceTarget *rt;
	obj->pblock2->GetValue(monoflect_colliderm,t,rt,obj->tmValid);
	colm = (CollisionMesh *) rt;

	if (!((obj->mValid.InInterval(t))&&(obj->tmValid.InInterval(t))))
	{
		obj->tmValid = FOREVER;

		if (colm) colm->SetNode(t,obj->custnode);
		if (colm) colm->PreFrame(t,(TimeValue) dt);

		obj->tm = obj->custnode->GetObjectTM(t,&obj->tmValid);
		obj->tmNoTrans = obj->tm;
		obj->tmNoTrans.NoTrans();
		obj->invtm = Inverse(obj->tm);
		obj->invtmNoTrans = Inverse(obj->tmNoTrans);

		obj->st->pbComplex->GetValue(PB_BOUNCE,t,bounce,obj->tmValid);
		obj->st->pbComplex->GetValue(PB_BVAR,t,bvar,obj->tmValid);
		obj->st->pbComplex->GetValue(PB_CHAOS,t,chaos,obj->tmValid);
		obj->st->pbComplex->GetValue(PB_INHERVEL,t,vinher,obj->tmValid);
		obj->st->pbComplex->GetValue(PB_FRICTION,t,friction,obj->tmValid);

		obj->st->pbComplex->GetValue(PB_DISTORTION,t,refvol,obj->tmValid);
		obj->st->pbComplex->GetValue(PB_DISTORTIONVAR,t,refvar,obj->tmValid);
		obj->st->pbComplex->GetValue(PB_PASSVEL,t,decel,obj->tmValid);
		obj->st->pbComplex->GetValue(PB_PASSVELVAR,t,decelvar,obj->tmValid);

//		vinher *= 0.01f;
//		bvar *= 0.01f;
//		chaos *= 0.01f;
//		friction *= 0.01f;

		if (obj->dmesh) 
			delete obj->dmesh;
		obj->dmesh = new Mesh;
		obj->dmesh->setNumFaces(0);
		if (obj->vnorms) 
		{	delete[] obj->vnorms;
			obj->vnorms=NULL;
		}
		if (obj->fnorms) 
		{	delete[] obj->fnorms;
			obj->fnorms=NULL;
		}
		obj->nv = (obj->nf=0);
		Interval tmpValid=FOREVER;
		obj->ptm = obj->custnode->GetObjectTM(t+(TimeValue)dt,&tmpValid);
		obj->dvel = (Zero*obj->ptm-Zero*obj->tm)/dt;
		Object *pobj; 
		pobj = obj->custnode->EvalWorldState(t).obj;
		obj->mValid = pobj->ObjectValidity(t);
		TriObject *triOb=NULL;
		badmesh = TRUE;
		if ((triOb=IsUseable(pobj,t))!=NULL) 
			AddMesh(obj,triOb,obj->tm,FALSE);
		if (obj->custnode->IsGroupHead())
		{	for (int ch=0;ch<obj->custnode->NumberOfChildren();ch++)
			{   INode *cnode=obj->custnode->GetChildNode(ch);
				if (cnode->IsGroupMember())
				{	pobj = cnode->EvalWorldState(t).obj;
					if ((triOb=IsUseable(pobj,t))!=NULL)
					{	Matrix3 tm=cnode->GetObjectTM(t,&obj->tmValid);
						obj->mValid=obj->mValid & pobj->ObjectValidity(t);
						AddMesh(obj,triOb,tm,TRUE);
					}
				}
			}
		}
		if (obj->nf>0)
		{	obj->vnorms=new VNormal[obj->nv];
			obj->fnorms=new Point3[obj->nf];
			GetVFLst(obj->dmesh,obj->vnorms,obj->fnorms);
			badmesh=FALSE;
		}
		if ((triOb)&&(triOb!=pobj)) 
			triOb->DeleteThis();
 	}

	if (badmesh) 
	{
		obj->ReturnThreeStateValue = DONTCARE;
		return 0;
	}

    if ((curtime!=t)&&(enableDynamics))
	{	totalforce = Zero;
		applyat = Zero;
		totalnumber = 0;
		curtime = t;
	}

	float fstartt,fendt;
	TimeValue startt,endt;
	obj->st->pbComplex->GetValue(PB_TIMEON,t,fstartt,FOREVER);
	obj->st->pbComplex->GetValue(PB_TIMEOFF,t,fendt,FOREVER);
	startt=fstartt*GetTicksPerFrame();endt=fendt*GetTicksPerFrame();
	if ((t<startt)||(t>endt))
	{	
		obj->lastrnd=rand();
		obj->ReturnThreeStateValue = DONTCARE;
		return FALSE;
	}

	if (!colm)
	{	
		obj->lastrnd=rand();
		obj->ReturnThreeStateValue = DONTCARE;
		return FALSE;
	}
	else
	{
	    srand(obj->lastrnd);

		float TempDP;
		float reflects;
		obj->st->pbComplex->GetValue(PB_REFLECTS,t,reflects,FOREVER);
//		reflects *= 0.01f;

		if (RND01()<reflects)
		{	
			donewithparticle = TRUE;
			Point3 hitpoint,bnorm,frict,inheritedVel;
			float at;
			BOOL hit = colm->CheckCollision(t,inp,vel,dt,at,hitpoint,bnorm,frict,inheritedVel);

			if (!hit) 
			{
				obj->lastrnd=rand();
				obj->ReturnThreeStateValue = 0;
				return FALSE;
			}
			float holddt = dt;
			dt -= at;
			float rvariation = 1.0f;
			float rchaos = 1.0f;
			if (bvar != 0.0f)
			{
				rvariation = 1.0f - (bvar * randomFloat[index%500]);
			}
			if (chaos != 0.0f)
			{
				rchaos = 1.0f - (chaos * randomFloat[index%500]);
			}
			vel = bnorm*(bounce*rvariation) + frict*(1.0f-(friction*rchaos)) ;
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
			vel +=  (inheritedVel * vinher);
			InVel = vel;
			applyat = hitpoint;
		}

// particle was not reflected and not tested for refraction!
		float refracts;
		obj->st->pbComplex->GetValue(PB_REFRACTS,t,refracts,FOREVER);
//		refracts *= 0.01f;
		if ((RND01()<refracts)&&(!donewithparticle)&&(enableAdvanced))
		{
			donewithparticle = TRUE;
			InVel = vel;
			Point3 hitpoint,bnorm,frict,inheritedVel;
			float at;
			BOOL hit = colm->CheckCollision(t,inp,vel,dt,at,hitpoint,bnorm,frict,inheritedVel);

			if (!hit) 
			{
				obj->lastrnd=rand();
				obj->ReturnThreeStateValue = 0;
				return FALSE;
			}
			
			Point3 c2,c1;
			Point3 Vdirbase = Normalize(InVel);

			float q1 = DotProd(-Vdirbase,bnorm);
			float theta=(float)acos(q1);
			if (theta>=HalfPI) 
				theta-=PI;
			c1 = Normalize((-InVel)^bnorm);
			c2=Normalize(bnorm^c1);
			Point3 Drag = friction*c2*DotProd(c2,-InVel);
			InVel *= decel*(1.0f-decelvar*RND01());

// rotate velocity vector
			float maxref,refangle,maxvarref;
			refangle = 0.0f;
			if (!FloatEQ0(refvol))
			{	if (refvol>0.0f)
					maxref = -theta;
				else 
					maxref = HalfPI-theta;
				refangle = maxref*(float)fabs(refvol);
				float frefangle = (float)fabs(refangle);
				if (refvol>0.0f)
					maxvarref = HalfPI-theta-frefangle;
				else
					maxvarref = theta-frefangle;
				refangle += maxvarref*RND11()*refvar;
				Point3 c,d;
				if (theta<0.01f)
				{	
					// Martell 4/14/01: Fix for order of ops bug.
					float ztmp=RND11(); float ytmp=RND11(); float xtmp=RND11();
					d = Point3(xtmp,ytmp,ztmp);
					c = Normalize(InVel^d);
				}
				else
				{	c = Normalize(bnorm^(-InVel));
				}
				RotateOnePoint(InVel,&Zero.x,&c.x,refangle);
				TempDP = DotProd(InVel,bnorm);
				if (TempDP>0.0f) 
					InVel = InVel - TempDP*bnorm;
			}
			float maxdiff,diffuse,diffvar,diffangle;
			obj->st->pbComplex->GetValue(PB_DIFFUSION,t,diffuse,FOREVER);
//			diffuse *= 0.01f;
			obj->st->pbComplex->GetValue(PB_DIFFUSIONVAR,t,diffvar,FOREVER);
//			diffvar *= 0.01f;
			maxdiff = HalfPI-theta-refangle;
			if (!FloatEQ0(diffuse))
			{	
				// Martell 4/14/01: Fix for order of ops bug.
				float ztmp=RND11(); float ytmp=RND11(); float xtmp=RND11();
				Point3 d = Point3(xtmp,ytmp,ztmp);
				Point3 c = Normalize(InVel^d);
				diffangle = 0.5f*maxdiff*diffuse*(1.0f+RND11()*diffvar);
				RotateOnePoint(InVel,&Zero.x,&c.x,diffangle);
				TempDP = DotProd(InVel,bnorm);
				if (TempDP>0.0f) 
					InVel = InVel - TempDP*bnorm;
			}

			float holddt = dt;
			dt -= at;
			inp = hitpoint;
			if (UpdatePastCollide)
			{	inp += InVel * dt;  //uses up the rest of the time with the new velocity
				if (ct) 
					(*ct) = holddt;
			}
			else
			{	if (ct) 
					(*ct) = at;
			}
			InVel += (inheritedVel * vinher);
			vel = InVel;
			applyat = hitpoint;
		}

// particle was neither reflected nor refracted nor tested for either!
		float spawnonly;
		obj->st->pbComplex->GetValue(PB_COLAFFECTS,t,spawnonly,FOREVER);
//		spawnonly *= 0.01f;
		if ((RND01()<spawnonly)&&(!donewithparticle)&&(enableAdvanced))
		{
			donewithparticle = TRUE;
			InVel = vel;
			Point3 hitpoint,bnorm,frict,inheritedVel;
			float at;
			BOOL hit = colm->CheckCollision(t,inp,vel,dt,at,hitpoint,bnorm,frict,inheritedVel);

			if (!hit) 
			{
				obj->lastrnd=rand();
				obj->ReturnThreeStateValue = 0;
				return FALSE;
			}

			float passvel,passvelvar;
			obj->st->pbComplex->GetValue(PB_COLPASSVEL,t,passvel,FOREVER);
			obj->st->pbComplex->GetValue(PB_COLPASSVELVAR,t,passvelvar,FOREVER);
//			passvelvar *= 0.01f;
			InVel *= passvel*(1.0f+passvelvar*RND11());

			float holddt = dt;
			dt -= at;
			inp = hitpoint;
			if (UpdatePastCollide)
			{	inp += InVel * dt;  //uses up the rest of the time with the new velocity
				if (ct) 
					(*ct) = holddt;
			}
			else
			{	if (ct) 
					(*ct) = at;
			}
			InVel +=  (inheritedVel * vinher);
			vel = InVel;
			applyat = hitpoint;
		}
	}

		}
// MESH COLLISION CODE BLOCK ENDS HERE
			break;
	}

	if (donewithparticle)
	{
		if (enableDynamics)
		{
			float mass = 0.001f;
			if (t==obj->ctime)
			{	
				totalnumber += 1;
				totalforce += (SaveVel-InVel)*K*mass/stepsize;
				obj->ffdata.FlectForce += totalforce;
				obj->ffdata.ApplyAt = applyat;
				obj->ffdata.Num = totalnumber;
			}
		}
		obj->ReturnThreeStateValue = 1;
		obj->lastrnd = rand();
		return TRUE;
	}
	else
	{
		obj->ReturnThreeStateValue = DONTCARE;
		obj->lastrnd=rand();
		return FALSE;
	}
}

void BasicFlectorObj::InvalidateUI()
{
	BasicFlectorPB.InvalidateUI(pblock2->LastNotifyParamID());
	BasicFComplexPB.InvalidateUI(pbComplex->LastNotifyParamID());
}

void BasicFlectorObj::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;
	if (flags&BEGIN_EDIT_CREATE) 
	{
		creating = TRUE;
	} 
	else 
	{ 
		creating = FALSE; 
	}
	SimpleWSMObject2::BeginEditParams(ip,flags,prev);
	if (!pmap[pbType_subani])
	{	pmap[pbType_subani] = CreateCPParamMap2(pblock2,ip,hInstance,MAKEINTRESOURCE(IDD_MF_0100_FLECTTYPESELECT),GetString(IDS_DLG_FTYPE),0);
		pmap[pbType_subani]->SetUserDlgProc(new BasicFlectorTypeObjDlgProc(this));
	} 
	else 
		pmap[pbType_subani]->SetParamBlock(GetParamBlockByID(pbType_subani));
	int oldval;
	pblock2->GetValue(PB_TYPE,pbType_subani,oldval,FOREVER);
	if (!st->theParam[pbType_subani])
		st->CreateMonoFlectorParamDlg(ip,oldval,pbType_subani);
	else 
		st->pmap[pbType_subani]->SetParamBlock(st->GetParamBlockByID(pbType_subani));   
	if (!pmap[pbComplex_subani]) 
	{	pmap[pbComplex_subani] = CreateCPParamMap2(pbComplex,ip,hInstance,MAKEINTRESOURCE(IDD_MF_0200_FLECTCOMPLEXITYSELECT),GetString(IDS_DLG_FCOMPLEX),0);
		pmap[pbComplex_subani]->SetUserDlgProc(new BasicFlectorComplexDlgProc(this));
	}
	else 
		pmap[pbComplex_subani]->SetParamBlock(GetParamBlockByID(pbComplex_subani)); 
	pbComplex->GetValue(PB_COMPLEX,pbComplex_subani,oldval,FOREVER);
	if (!st->theParam[pbComplex_subani])
		st->CreateMonoFlectorParamDlg(ip,oldval,pbComplex_subani);
	else 
		st->pmap[pbComplex_subani]->SetParamBlock(st->GetParamBlockByID(pbComplex_subani)); 
}

static TCHAR*

SaveString(TCHAR* str)
{
	// save a copy of string in heap
	return _tcscpy((TCHAR*)malloc(strlen(str) + sizeof(TCHAR)), str);
}
		
void Reinit(IParamMap2Ptr *pm,TimeValue t)
{
  ParamBlockDesc2* pbd = (*pm)->GetDesc();
  if (!(pbd->flags & P_CLASS_PARAMS))
    for (int i = 0; i < pbd->count; i++)
	{ ParamDef& pd = pbd->paramdefs[i];
	  if (!(pd.flags & P_RESET_DEFAULT))
		switch (pd.type)
		{
		  case TYPE_ANGLE:
		  case TYPE_PCNT_FRAC:
		  case TYPE_WORLD:
		  case TYPE_COLOR_CHANNEL:
		  case TYPE_FLOAT:
				pd.cur_def.f = (*pm)->GetParamBlock()->GetFloat(pd.ID, t); 
				pd.flags |= P_HAS_CUR_DEFAULT;
			break;
		  case TYPE_BOOL:
		  case TYPE_TIMEVALUE:
		  case TYPE_RADIOBTN_INDEX:
		  case TYPE_INT:		
				pd.cur_def.i = (*pm)->GetParamBlock()->GetInt(pd.ID, t); 
				pd.flags |= P_HAS_CUR_DEFAULT;
			break;
		  case TYPE_HSV:
		  case TYPE_RGBA:
		  case TYPE_POINT3:
				{
					if (pd.cur_def.p != NULL)
						delete pd.cur_def.p;
					pd.cur_def.p = new Point3((*pm)->GetParamBlock()->GetPoint3(pd.ID, t)); 
					pd.flags |= P_HAS_CUR_DEFAULT;
					break;
				}
		  case TYPE_FILENAME:
		  case TYPE_STRING:
				{
					TCHAR* s = (*pm)->GetParamBlock()->GetStr(pd.ID, t);
					if (s != NULL)
					{
						if (pd.cur_def.s != NULL)
							free(pd.cur_def.s);
						pd.cur_def.s = SaveString(s); 
						pd.flags |= P_HAS_CUR_DEFAULT;
					}
					break;
				}
		}
	}
}
void BasicFlectorObj::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
{
	SimpleWSMObject2::EndEditParams(ip,flags,next);
	ip->ClearPickMode();
	this->ip = NULL;
	if (flags&END_EDIT_REMOVEUI )
	{		
		if (pmap[pbType_subani])
		{	
			DestroyCPParamMap2(pmap[pbType_subani]);
			pmap[pbType_subani]=NULL;
		}
		if (pmap[pbComplex_subani])
		{	DestroyCPParamMap2(pmap[pbComplex_subani]);
			pmap[pbComplex_subani]=NULL;
		}
	}
	creating=FALSE;
}

IOResult BasicFlectorObj::Save(ISave *isave)
{
	isave->BeginChunk(MONODEF_CUSTNAME_CHUNK);		
	isave->WriteWString(custname);
	isave->EndChunk();
	return IO_OK;
}

class BasicFlectorObjLoad : public PostLoadCallback 
{
	public:
		BasicFlectorObj *n;
		BasicFlectorObjLoad(BasicFlectorObj *ns) {n = ns;}
		void proc(ILoad *iload) 
		{  
			ReferenceTarget *rt;
			Interval iv;

			n->pblock2->GetValue(monoflect_colliderp,0,rt,iv);
			if (rt == NULL)
			{
				CollisionPlane *colp = (CollisionPlane*)CreateInstance(REF_MAKER_CLASS_ID, PLANAR_COLLISION_ID);
				if (colp)
					n->pblock2->SetValue(monoflect_colliderp,0,(ReferenceTarget*)colp);
			}

			n->pblock2->GetValue(monoflect_colliders,0,rt,iv);
			if (rt == NULL)
			{
				CollisionSphere *cols = (CollisionSphere*)CreateInstance(REF_MAKER_CLASS_ID, SPHERICAL_COLLISION_ID);
				if (cols)
					n->pblock2->SetValue(monoflect_colliders,0,(ReferenceTarget*)cols);
			}

			n->pblock2->GetValue(monoflect_colliderm,0,rt,iv);
			if (rt == NULL)
			{
				CollisionMesh *colm = (CollisionMesh*)CreateInstance(REF_MAKER_CLASS_ID, MESH_COLLISION_ID);
				if (colm)
					n->pblock2->SetValue(monoflect_colliderm,0,(ReferenceTarget*)colm);
			}
			delete this; 
		} 
};

IOResult BasicFlectorObj::Load(ILoad *iload)
{
	IOResult res = IO_OK;
	
	// Default names
	custname = TSTR(_T(" "));

	while (IO_OK==(res=iload->OpenChunk())) 
	{
		switch (iload->CurChunkID()) 
		{
			case MONODEF_CUSTNAME_CHUNK: 
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
	iload->RegisterPostLoadCallback(new BasicFlectorObjLoad(this));
	return IO_OK;
}

FlectForces BasicFlectorObj::ForceData(TimeValue t)
{
	float ft1,ft2;
	pbComplex->GetValue(PB_TIMEON,t,ft1,FOREVER);
	pbComplex->GetValue(PB_TIMEOFF,t,ft2,FOREVER);
	ffdata.t1=ft1*GetTicksPerFrame();ffdata.t2=ft2*GetTicksPerFrame();
	return ffdata;
}

RefTargetHandle BasicFlectorObj::Clone(RemapDir& remap) 
{
	BasicFlectorObj* newob = new BasicFlectorObj();	
	if (pblock2) newob->ReplaceReference(pbType_subani, pblock2->Clone(remap));
	if (pbComplex) newob->ReplaceReference(pbComplex_subani, pbComplex->Clone(remap));
	if (st) newob->ReplaceReference(monoflecdlg, st->Clone(remap));

//	if (custnode) 
//		newob->ReplaceReference(CUSTNODE,custnode);
	newob->custname=custname;
	newob->dmesh=NULL;
	newob->vnorms=NULL;
	newob->fnorms=NULL;
	newob->ivalid.SetEmpty();	
	BaseClone(this, newob, remap);
	return(newob);
}

BOOL BasicFlectorObj::OKtoDisplay(TimeValue t) 
{
		float size;
		st->pblock2->GetValue(PB_WIDTH,t,size,FOREVER);
		if (size==0.0f) 
			return FALSE;
		else 
			return TRUE;
}

/*int BasicFlectorObj::IntersectRay(TimeValue t, Ray& ray, float& at, Point3& norm)
{
	// pass to SimpleObject to do this
	return SimpleWSMObject2::IntersectRay(t, ray, at, norm);
}*/

int BasicFlectorObj::CanConvertToType(Class_ID obtype)
{
	return FALSE;
}

int CreateBasicFlectorProc::createmethod(
		ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat)
{
	Point3 p1, center;

#ifdef _3D_CREATE
	DWORD snapdim = SNAP_IN_3D;
#else
	DWORD snapdim = SNAP_IN_PLANE;
#endif

	if (msg == MOUSE_FREEMOVE)
	{
		vpt->SnapPreview(m, m, NULL, snapdim);
	}
	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) 
	{
		switch(point) 
		{

		// point one - where we measure from
			case 0:
				GetCOREInterface()->SetHideByCategoryFlags(
						GetCOREInterface()->GetHideByCategoryFlags() & ~(HIDE_OBJECTS|HIDE_PARTICLES));
				sp0 = m;
				p0 = vpt->SnapPoint(m, m, NULL, snapdim);
				BasicFlectorObject->st->pblock2->SetValue(PB_WIDTH,0,0.01f);
				BasicFlectorObject->st->pblock2->SetValue(PB_LENGTH,0,0.01f);
				p1 = p0 + Point3(.01,.01,.01);
				mat.SetTrans(float(.5)*(p0+p1));				
				BasicFlectorObject->st->pmap[pbType_subani]->Invalidate();
				break;

		// point two - where we measure to in worldspace
			case 1:		
				sp1 = m;
				p1 = vpt->SnapPoint(m, m, NULL, snapdim);
				p1.z = p0.z +(float).01; 
//				if(flags&MOUSE_CTRL) 
//				{	mat.SetTrans(p0);	
//				} 
//				else 
					mat.SetTrans(float(.5)*(p0+p1));
				Point3 d = p1-p0;
				float len;
				if (fabs(d.x) > fabs(d.y)) 
					len = d.x;
				else 
					len = d.y;
				d.x = d.y = 2.0f * len;
				BasicFlectorObject->st->pblock2->SetValue(PB_WIDTH,0,(float)fabs(p1.x-p0.x));
				BasicFlectorObject->st->pblock2->SetValue(PB_LENGTH,0,(float)fabs(p1.y-p0.y));
				BasicFlectorObject->st->pmap[pbType_subani]->Invalidate();

				if (msg==MOUSE_POINT)
				{ if (Length(sp1-sp0)<3 || Length(d)<0.1f)  return CREATE_ABORT;	
				  else
				  {
				   return CREATE_STOP;	
				  }
				}
				break;					   
			}
		} 
		else 
		{
			if (msg == MOUSE_ABORT) 
				return CREATE_ABORT;
		}

	return TRUE;
}

BOOL BasicFlectorObj::SupportsDynamics()
{
	int supportsdynamics;
	pbComplex->GetValue(PB_COMPLEX,0,supportsdynamics,ivalid);
	return (supportsdynamics>1);
}

void BasicFlectorObj::BuildMesh(TimeValue t)
{
	int typeofdeflector;
	pblock2->GetValue(PB_TYPE,0,typeofdeflector,ivalid);

	float sz0,sz1;
	ivalid = FOREVER;
	st->pblock2->GetValue(PB_WIDTH,t,sz0,ivalid);
	sz0 *= 0.5f;
	st->pblock2->GetValue(PB_LENGTH,t,sz1,ivalid);
	sz1 *= 0.5f;

	switch(typeofdeflector)
	{
		case PLANAR:
			{
	float w, h;
	float w2,h2,h3,h4;
	ivalid = FOREVER;
	w = sz0;
	w2=w*0.5f;
	h = sz1;
	h2=h*0.5f;
	h3=h2*0.15f;
	h4=h2*0.25f;

	mesh.setNumVerts(19);
	mesh.setNumFaces(11);
	mesh.setVert(0, Point3(-w,-h, 0.0f));
	mesh.setVert(1, Point3( w,-h, 0.0f));
	mesh.setVert(2, Point3( w, h, 0.0f));
	mesh.setVert(3, Point3(-w, h, 0.0f));

	mesh.setVert( 4, Point3(0.0f,0.0f,0.0f));
	mesh.setVert( 5, Point3(0.0f,  h2,  h2));
	mesh.setVert( 6, Point3(0.0f, -h2,  h2));
	mesh.setVert( 7, Point3(0.0f,  h2+h3,  h2));
	mesh.setVert( 8, Point3(0.0f,  h2,  h2+h3));
	mesh.setVert( 9, Point3(0.0f, -h2,  h2-h3));
	mesh.setVert(10, Point3(0.0f, -h2+h3,  h2));

	mesh.setVert(11, Point3(0.0f, h4, 0.0f));
	mesh.setVert(12, Point3(0.0f, h4, -h2));
	mesh.setVert(13, Point3(0.0f, h4+h3, -h2));
	mesh.setVert(14, Point3(0.0f, 0.0f, -h2-h3-h3));
	mesh.setVert(15, Point3(0.0f,-h4-h3, -h2));
	mesh.setVert(16, Point3(0.0f,-h4, -h2));
	mesh.setVert(17, Point3(0.0f,-h4, 0.0f));
	mesh.setVert(18, Point3(0.0f,0.0f,-h4));
	
	mesh.faces[0].setEdgeVisFlags(1,1,0);
	mesh.faces[0].setSmGroup(1);
	mesh.faces[0].setVerts(0,1,2);
	mesh.faces[1].setEdgeVisFlags(1,1,0);
	mesh.faces[1].setSmGroup(1);
	mesh.faces[1].setVerts(2,3,0);	

	mesh.faces[2].setEdgeVisFlags(1,0,1);
	mesh.faces[2].setSmGroup(1);
	mesh.faces[2].setVerts(4,6,5);	
	mesh.faces[3].setEdgeVisFlags(1,0,1);
	mesh.faces[3].setSmGroup(1);
	mesh.faces[3].setVerts(6,9,10);	
	mesh.faces[4].setEdgeVisFlags(1,0,1);
	mesh.faces[4].setSmGroup(1);
	mesh.faces[4].setVerts(5,8,7);
	mesh.faces[5].setEdgeVisFlags(1,0,1);
	mesh.faces[5].setSmGroup(1);
	mesh.faces[5].setVerts(11,12,18);	
	mesh.faces[6].setEdgeVisFlags(0,0,0);
	mesh.faces[6].setSmGroup(1);
	mesh.faces[6].setVerts(12,16,18);	
	mesh.faces[7].setEdgeVisFlags(1,1,0);
	mesh.faces[7].setSmGroup(1);
	mesh.faces[7].setVerts(16,17,18);	
	mesh.faces[8].setEdgeVisFlags(1,1,0);
	mesh.faces[8].setSmGroup(1);
	mesh.faces[8].setVerts(12,13,14);	
	mesh.faces[9].setEdgeVisFlags(0,0,0);
	mesh.faces[9].setSmGroup(1);
	mesh.faces[9].setVerts(12,14,16);	
	mesh.faces[10].setEdgeVisFlags(1,1,0);
	mesh.faces[10].setSmGroup(1);
	mesh.faces[10].setVerts(14,15,16);	

	mesh.InvalidateGeomCache();	
	return;
			}

		case SPHERE:
			{
	float r,r2,r3,r4,u;
	#define NUM_SEGS	24
	r = 2.0f * sz0;
	r2=0.5f*r;
	r3=0.15f*r2;
	r4=0.25f*r2;

	mesh.setNumVerts(3*NUM_SEGS+16);
	mesh.setNumFaces(3*NUM_SEGS+9);

	for (int i=0; i<NUM_SEGS; i++)
	{	u=float(i)/float(NUM_SEGS) * TWOPI;
		mesh.setVert(i, Point3((float)cos(u) * r, (float)sin(u) * r, 0.0f));
	}
	for (i=0; i<NUM_SEGS; i++)
	{	u=float(i)/float(NUM_SEGS) * TWOPI;
		mesh.setVert(i+NUM_SEGS, Point3(0.0f, (float)cos(u) * r, (float)sin(u) * r));
	}
	for (i=0; i<NUM_SEGS; i++)
	{	u=float(i)/float(NUM_SEGS) * TWOPI;
		mesh.setVert(i+2*NUM_SEGS, Point3((float)cos(u) * r, 0.0f, (float)sin(u) * r));
	}		

	mesh.setVert(72, Point3(0.0f,0.0f,0.0f));

	mesh.setVert(73, Point3(0.0f,0.0f,  r ));
	mesh.setVert(74, Point3(0.0f, r2 ,r+r2));
	mesh.setVert(75, Point3(0.0f,-r2 ,r+r2));
	mesh.setVert(76, Point3(0.0f, r2+r3,r+r2));
	mesh.setVert(77, Point3(0.0f, r2,r+r2+r3));
	mesh.setVert(78, Point3(0.0f,-r2,r+r2-r3));
	mesh.setVert(79, Point3(0.0f,-r2+r3,r+r2));

	mesh.setVert(80, Point3(0.0f, r4   ,-r ));
	mesh.setVert(81, Point3(0.0f, r4   ,-r-r2));
	mesh.setVert(82, Point3(0.0f, r4+r3,-r-r2));
	mesh.setVert(83, Point3(0.0f,0.0f  ,-r-r2-r3-r3));
	mesh.setVert(84, Point3(0.0f,-r4-r3,-r-r2));
	mesh.setVert(85, Point3(0.0f,-r4   ,-r-r2));
	mesh.setVert(86, Point3(0.0f,-r4   ,-r));
	mesh.setVert(87, Point3(0.0f,0.0f  ,-r-r4));
	
	for (i=0; i<3*NUM_SEGS; i++)
	{	int i1 = i+1;
		if (i1%NUM_SEGS==0) i1 -= NUM_SEGS;
		mesh.faces[i].setEdgeVisFlags(1,0,0);
		mesh.faces[i].setSmGroup(1);
		mesh.faces[i].setVerts(i,i1,3*NUM_SEGS);
	}

	mesh.faces[72].setEdgeVisFlags(1,0,1);
	mesh.faces[72].setSmGroup(1);
	mesh.faces[72].setVerts(73,75,74);	
	mesh.faces[73].setEdgeVisFlags(1,0,1);
	mesh.faces[73].setSmGroup(1);
	mesh.faces[73].setVerts(75,78,79);	
	mesh.faces[74].setEdgeVisFlags(1,0,1);
	mesh.faces[74].setSmGroup(1);
	mesh.faces[74].setVerts(74,77,76);

	mesh.faces[75].setEdgeVisFlags(1,0,1);
	mesh.faces[75].setSmGroup(1);
	mesh.faces[75].setVerts(80,81,87);	
	mesh.faces[76].setEdgeVisFlags(0,0,0);
	mesh.faces[76].setSmGroup(1);
	mesh.faces[76].setVerts(81,85,87);	
	mesh.faces[77].setEdgeVisFlags(1,1,0);
	mesh.faces[77].setSmGroup(1);
	mesh.faces[77].setVerts(85,86,87);	
	mesh.faces[78].setEdgeVisFlags(1,1,0);
	mesh.faces[78].setSmGroup(1);
	mesh.faces[78].setVerts(81,82,83);	
	mesh.faces[79].setEdgeVisFlags(0,0,0);
	mesh.faces[79].setSmGroup(1);
	mesh.faces[79].setVerts(81,83,85);	
	mesh.faces[80].setEdgeVisFlags(1,1,0);
	mesh.faces[80].setSmGroup(1);
	mesh.faces[80].setVerts(83,84,85);	

	mesh.InvalidateGeomCache();
	return;
			}
		case MESH:
			{
				int shouldIhide;
				st->pblock2->GetValue(PB_HIDEICON,0,shouldIhide,ivalid);
				if (shouldIhide)
				{
					mesh.setNumVerts(0);
					mesh.setNumFaces(0);
					mesh.InvalidateGeomCache();
					return;
				}
				else
				{

	float l,h2,h3,h4;
	l = sz0;
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
	return;
				}
			}
	}
}

BOOL BasicFlectorObj::HasUVW() 
{ 
	BOOL genUVs = FALSE;
//	pblock2->GetValue(particlepodobj_genuv, 0, genUVs, FOREVER);
	return genUVs; 
}

void BasicFlectorObj::SetGenUVW(BOOL sw) 
{  
	if (sw==HasUVW()) return;
//	pblock2->SetValue(particlepodobj_genuv, 0, sw);				
}
Animatable* BasicFlectorObj::SubAnim(int i)
{
	switch(i) {
		// paramblock2s
		case pbType_subani:		return pblock2;
		case pbComplex_subani:	return pbComplex;
		case monoflecdlg: return st;

		default: return 0;
	}
}
void BasicFlectorObj::SetReference(int i, RefTargetHandle rtarg)
{
	switch(i) {
		case pbType_subani:		SimpleWSMObject2::SetReference(i, rtarg); break;
		case pbComplex_subani:	pbComplex=(IParamBlock2*)rtarg; break;
		case monoflecdlg: st=(MonoFlector*)rtarg; break;
	}
}
RefTargetHandle BasicFlectorObj::GetReference(int i)
{
	switch(i) {
		// paramblock2s
		case pbType_subani:		return SimpleWSMObject2::GetReference(i);
		case pbComplex_subani:	return pbComplex;
		case monoflecdlg:		return st; 
		default: return 0;
	}
}

TSTR BasicFlectorObj::SubAnimName(int i)
{
	switch(i) {
		case pbType_subani:		return GetString(IDS_DLG_FTYPE);
		case pbComplex_subani:	return GetString(IDS_DLG_FCOMPLEX);
		case monoflecdlg:	return GetString(IDS_DLG_MONOF);
		default: return _T("");
	}
}
IParamBlock2* BasicFlectorObj::GetParamBlock(int i)
{
	switch(i) {
		case pbType_subani:		return pblock2;
		case pbComplex_subani:	return pbComplex;
		default:				return NULL;
	}
}

IParamBlock2* BasicFlectorObj::GetParamBlockByID(BlockID id)
{
	if(pblock2->ID() == id)
		return pblock2;
	else if(pbComplex->ID() == id)
		return pbComplex;
	else
		return NULL;
}

RefResult BasicFlectorObj::NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message )
{				
//	switch (message) 
//	{			default: 
			SimpleWSMObject2::NotifyRefChanged(changeInt,hTarget,partID,message);
//	}
	return REF_SUCCEED;
}
BasicFlectorMod::BasicFlectorMod(INode *node,BasicFlectorObj *obj)
{	
	MakeRefByID(FOREVER,SIMPWSMMOD_NODEREF,node);	
	pblock = NULL;
	obRef=NULL;
}

Interval BasicFlectorMod::GetValidity(TimeValue t) 
{
	if (obRef && nodeRef) 
	{
		Interval valid = FOREVER;
		Matrix3 tm;
		BasicFlectorObj *obj = (BasicFlectorObj*)GetWSMObject(t);
		tm = nodeRef->GetObjectTM(t,&valid);
		
		float TempT;
		obj->st->pbComplex->GetValue(PB_TIMEON,t,TempT,valid);
		obj->st->pbComplex->GetValue(PB_TIMEOFF,t,TempT,valid);

		float f;		
		obj->st->pbComplex->GetValue(PB_REFLECTS,t,f,valid);
		obj->st->pbComplex->GetValue(PB_BOUNCE,t,f,valid);
		obj->st->pbComplex->GetValue(PB_BVAR,t,f,valid);
		obj->st->pbComplex->GetValue(PB_CHAOS,t,f,valid);
		obj->st->pbComplex->GetValue(PB_FRICTION,t,f,valid);
		obj->st->pbComplex->GetValue(PB_INHERVEL,t,f,valid);
		obj->st->pbComplex->GetValue(PB_REFRACTS,t,f,valid);
		obj->st->pbComplex->GetValue(PB_PASSVEL,t,f,valid);
		obj->st->pbComplex->GetValue(PB_PASSVELVAR,t,f,valid);
		obj->st->pbComplex->GetValue(PB_DISTORTION,t,f,valid);
		obj->st->pbComplex->GetValue(PB_DISTORTIONVAR,t,f,valid);
		obj->st->pbComplex->GetValue(PB_DIFFUSION,t,f,valid);
		obj->st->pbComplex->GetValue(PB_DIFFUSIONVAR,t,f,valid);
		obj->st->pbComplex->GetValue(PB_COLAFFECTS,t,f,valid);
		obj->st->pbComplex->GetValue(PB_COLPASSVEL,t,f,valid);
		obj->st->pbComplex->GetValue(PB_COLPASSVELVAR,t,f,valid);

		obj->st->pblock2->GetValue(PB_WIDTH,t,f,valid);
		obj->st->pblock2->GetValue(PB_LENGTH,t,f,valid);

		return valid;
	} 
	else 
	{
		return FOREVER;
	}
}

class BasicFlectorDeformer : public Deformer {
	public:		
		Point3 Map(int i, Point3 p) {return p;}
	};
static BasicFlectorDeformer BasicFlectordeformer;

Deformer& BasicFlectorMod::GetDeformer(
		TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
	{
	return BasicFlectordeformer;
	}

RefTargetHandle BasicFlectorMod::Clone(RemapDir& remap) 
	{
	BasicFlectorMod *newob = new BasicFlectorMod(nodeRef,(BasicFlectorObj*)obRef);	
	newob->SimpleWSMModClone(this);
	BaseClone(this, newob, remap);
	return newob;
	}

void BasicFlectorMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node)
{
	ParticleObject *obj = GetParticleInterface(os->obj);
	if (obj) 
	{
		deflect.obj  = (BasicFlectorObj*)GetWSMObject(t);
		deflect.obj->custnode = deflect.obj->st->pblock2->GetINode(PB_MESHNODE);
		deflect.node = nodeRef;

		deflect.obj->tmValid.SetEmpty();		
		deflect.obj->mValid.SetEmpty();
		deflect.badmesh = (deflect.obj->custnode==NULL);
		if (t<=deflect.obj->t) 
			deflect.obj->lastrnd = 12345;
		deflect.obj->t=t;
		
		deflect.obj->dvel = Zero;
		deflect.totalforce = Zero;
		deflect.applyat = Zero;
		deflect.totalnumber = 0;
		TimeValue tmpt = GetCOREInterface()->GetTime();
		if (deflect.obj->ctime != tmpt)
		{	deflect.obj->ctime = tmpt;
			deflect.obj->ffdata.FlectForce = deflect.totalforce;
			deflect.obj->ffdata.ApplyAt = deflect.applyat;
			deflect.obj->ffdata.Num = deflect.totalnumber;
		}
		obj->ApplyCollisionObject(&deflect);
	}
}

CollisionObject *BasicFlectorObj::GetCollisionObject(INode *node)
{
	BasicFlectorField *gf = new BasicFlectorField;	
	gf->obj  = this;
	gf->node = node;
	gf->obj->tmValid.SetEmpty();
	return gf;
}

/*  // Bayboro 9/18/01
void* BasicFlectorObj::GetInterface(ULONG id) 
{
	switch (id)
	{ 
		case I_NEWPARTTEST:		return (ITestInterface*)this;
	}
	return Object::GetInterface(id);
}
*/  // Bayboro 9/18/01

void BasicFlectorObj::SetUpModifier(TimeValue t,INode *node)
{
	custnode = st->pblock2->GetINode(PB_MESHNODE);

	mf->deflect.obj  = (BasicFlectorObj*)(mf->GetWSMObject(t));
	mf->deflect.node = mf->nodeRef;
//	mf->deflect.obj->tmValid.SetEmpty();		
//	mf->deflect.obj->mValid.SetEmpty();
	tmValid.SetEmpty();		
	mValid.SetEmpty();
	mf->deflect.badmesh = (custnode==NULL);
//	if (t <= mf->deflect.obj->t) 
//		mf->deflect.obj->lastrnd = 12345;

	mf->deflect.obj->t = t;
	mf->deflect.obj->dvel = Zero;
	mf->deflect.totalforce = Zero;
	mf->deflect.applyat = Zero;
	mf->deflect.totalnumber = 0;
	TimeValue tmpt = GetCOREInterface()->GetTime();
	if (mf->deflect.obj->ctime != tmpt)
	{	
		mf->deflect.obj->ctime = tmpt;
		mf->deflect.obj->ffdata.FlectForce = mf->deflect.totalforce;
		mf->deflect.obj->ffdata.ApplyAt = mf->deflect.applyat;
		mf->deflect.obj->ffdata.Num = mf->deflect.totalnumber;
	}
}

/*  // Bayboro 9/18/01
int BasicFlectorObj::NPTestInterface(TimeValue t,BOOL UpdatePastCollide,ParticleData *part,float dt,INode *node,int index)
{
	ReturnThreeStateValue = DONTCARE;

	if (!mf)
		mf = (BasicFlectorMod *)CreateWSMMod(node);
	SetUpModifier(t,node);
	
	float ct = 0;
	UpdatePastCollide = TRUE;
	mf->deflect.CheckCollision(t,part->position,part->velocity,dt,index,&ct,UpdatePastCollide);
	return (ReturnThreeStateValue);
}
*/  // Bayboro 9/18/01