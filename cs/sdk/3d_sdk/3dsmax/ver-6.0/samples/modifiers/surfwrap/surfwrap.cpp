/**********************************************************************
 *<
	FILE: surfwrap.cpp

	DESCRIPTION: Wrap one surface over another space warp

	CREATED BY: Audrey Peterson

	HISTORY: 1/97

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/
#include "wrap.h"
#include "iparamm.h"
#include "simpmod.h"
#include "simpobj.h"
#include "shape.h"
#include "spline3d.h"
#include "splshape.h"
#include "texutil.h"
#include "surf_api.h"

#define PBLK		0
#define CUSTNODE 		1
#define DU 0.001f
static Point3 Zero=Point3(0.0f,0.0f,0.0f);
static Class_ID SWRAP_CLASS_ID(0x3de109fc, 0x40371016);
static Class_ID SWRAPMOD_CLASS_ID(0x20dc0b59, 0x437a5d7c);

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

class SWrapPickOperand;

class SWrapObject : public SimpleWSMObject {	
	public:		
		static IParamMap *pmapParam;
		static IObjParam *ip;
		static HWND hSot;
		static HWND hParams;
		Interval cmValid;
		Mesh *cmesh;
		Matrix3 tm;
		Tab<VNormal> vnorms;
		Tab<Point3> fnorms;

		INode *custnode;
		TSTR custname;
		SWrapObject();
		~SWrapObject();
		static BOOL creating;
		static SWrapPickOperand pickCB;

		void ShowName();
		// From Animatable		
		void DeleteThis() {delete this;}		
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);		
		Class_ID ClassID() {return SWRAP_CLASS_ID;}		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_AP_SWRAPOBJ);}
				
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
	};
class SWrapPickOperand : 
		public PickModeCallback,
		public PickNodeCallback {
	public:		
		SWrapObject *po;
		
		SWrapPickOperand() {po=NULL;}

		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);

		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);

		BOOL RightClick(IObjParam *ip, ViewExp *vpt) { return TRUE; }
		BOOL Filter(INode *node);
		
		PickNodeCallback *GetFilter() {return this;}
	};

//--- ClassDescriptor and class vars ---------------------------------

IObjParam *SWrapObject::ip        = NULL;
IParamMap *SWrapObject::pmapParam = NULL;
HWND       SWrapObject::hSot      = NULL;
HWND       SWrapObject::hParams      = NULL;
BOOL SWrapObject::creating    = FALSE;
SWrapPickOperand SWrapObject::pickCB;

class SWrapClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) { return new SWrapObject;}
	const TCHAR *	ClassName() {return GetString(IDS_AP_SWRAP);}
	SClass_ID		SuperClassID() {return WSM_OBJECT_CLASS_ID; }
	Class_ID		ClassID() {return SWRAP_CLASS_ID;}
	const TCHAR* 	Category() {return GetSpaceWarpCatString(SPACEWARP_CAT_GEOMDEF);}
	int 			BeginCreate(Interface *i);
	int 			EndCreate(Interface *i);
	};
static SWrapClassDesc SWrapDesc;
ClassDesc* GetSWrapDesc() {return &SWrapDesc;}

//--- DeflectMod -----------------------------------------------------

class SWrapMod : public SimpleWSMMod {
	public:				

		SWrapMod() {}
		SWrapMod(INode *node,SWrapObject *obj);	

		// From Animatable
		void GetClassName(TSTR& s) {s= GetString(IDS_AP_SWRAPMOD);}
		SClass_ID SuperClassID() {return WSM_CLASS_ID;}
		void DeleteThis() {delete this;}
		Class_ID ClassID() { return SWRAPMOD_CLASS_ID;}
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_AP_SWRAPBINDING);}

		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);

		// From SimpleWSMMod		
		Interval GetValidity(TimeValue t);		
		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);
	};

//--- ClassDescriptor and class vars ---------------------------------

class SWrapModClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 0;}
	void *			Create(BOOL loading = FALSE) { return new SWrapMod;}
	const TCHAR *	ClassName() {return GetString(IDS_AP_SWRAPMOD);}
	SClass_ID		SuperClassID() {return WSM_CLASS_ID; }
	Class_ID		ClassID() {return SWRAPMOD_CLASS_ID;}
	const TCHAR* 	Category() {return GetSpaceWarpCatString(SPACEWARP_CAT_GEOMDEF);}
	};

static SWrapModClassDesc SWrapModDesc;
ClassDesc* GetSWrapModDesc() {return &SWrapModDesc;}
//--- Parameter map/block descriptors -------------------------------

#define PB_KIDEFAULT		0
#define PB_USESELVERTS		1
#define PB_ICONSIZE			2
#define PB_STANDOFF			3

//
//
// Parameters

static ParamUIDesc descParam[] = {
	// Default Projection Distance Ki
	ParamUIDesc(
		PB_KIDEFAULT,
		EDITTYPE_FLOAT,
		IDC_AP_KIDEFAULT,IDC_AP_KIDEFAULTSPIN,
		-BIGFLOAT,BIGFLOAT,
		SPIN_AUTOSCALE),

	// Use Selected Vertices ONLY
	ParamUIDesc(PB_USESELVERTS,TYPE_SINGLECHEKBOX,IDC_AP_USESELVERTS),

	// Icon Size
	ParamUIDesc(
		PB_ICONSIZE,
		EDITTYPE_FLOAT,
		IDC_AP_ICONSIZE,IDC_AP_ICONSIZESPIN,
		0.0f,BIGFLOAT,
		SPIN_AUTOSCALE),

	// Standoff Distance
	ParamUIDesc(
		PB_STANDOFF,
		EDITTYPE_FLOAT,
		IDC_AP_STANDOFF,IDC_AP_STANDOFFSPIN,
		-BIGFLOAT,BIGFLOAT,
		SPIN_AUTOSCALE),
	};
#define PARAMDESC_LENGTH 4


static ParamBlockDescID SWrapdescVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_INT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, FALSE, 2 }, //ICON size
	{ TYPE_FLOAT, NULL, TRUE, 3 },
	};

#define PBLOCK_LENGTH	4

#define NUM_OLDVERSIONS	0

// Current version
#define CURRENT_VERSION	0

static ParamVersionDesc curVersion(SWrapdescVer0,PBLOCK_LENGTH,CURRENT_VERSION);

class CreateSWrapObjectProc : public MouseCallBack,ReferenceMaker {
	private:
		IObjParam *ip;
		void Init(IObjParam *i) {ip=i;}
		CreateMouseCallBack *createCB;	
		INode *SWrapNode;
		SWrapObject *SWrapObj;
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
		RefTargetHandle GetReference(int i) { return (RefTargetHandle)SWrapNode; } 
		void SetReference(int i, RefTargetHandle rtarg) { SWrapNode = (INode *)rtarg; }

		// StdNotifyRefChanged calls this, which can change the partID to new value 
		// If it doesnt depend on the particular message& partID, it should return
		// REF_DONTCARE
		BOOL SupportAutoGrid(){return TRUE;}
	    RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	    	PartID& partID,  RefMessage message);
	public:
		void Begin( IObjCreate *ioc, ClassDesc *desc );
		void End();
		
		CreateSWrapObjectProc()
			{
			ignoreSelectionChange = FALSE;
			}
		int createmethod(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
		int proc( HWND hwnd, int msg, int point, int flag, IPoint2 m );
	};


#define CID_CREATESWRAPOBJECTMODE	CID_USER + 25

class CreateSWrapObjectMode : public CommandMode {		
	public:		
		CreateSWrapObjectProc proc;
		IObjParam *ip;
		SWrapObject *obj;
		void Begin( IObjCreate *ioc, ClassDesc *desc ) { proc.Begin( ioc, desc ); }
		void End() { proc.End(); }
		void JumpStart(IObjParam *i,SWrapObject*o);

		int Class() {return CREATE_COMMAND;}
		int ID() { return CID_CREATESWRAPOBJECTMODE; }
		MouseCallBack *MouseProc(int *numPoints) {*numPoints = 10000; return &proc;}
		ChangeForegroundCallback *ChangeFGProc() {return CHANGE_FG_SELECTED;}
		BOOL ChangeFG( CommandMode *oldMode ) { return (oldMode->ChangeFGProc() != CHANGE_FG_SELECTED); }
		void EnterMode() {/*MakeRefByID(FOREVER,0,svNode);*/}
		void ExitMode() {/*DeleteAllRefsFromMe();*/}
	};
static CreateSWrapObjectMode theCreateSWrapObjectMode;

RefResult CreateSWrapObjectProc::NotifyRefChanged(
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
		 	if ( SWrapObj && SWrapNode==hTarget ) {
				// this will set camNode== NULL;
				theHold.Suspend();
				DeleteReference(0);
				theHold.Resume();
				goto endEdit;
				}
			// fall through

		case REFMSG_TARGET_DELETED:		
			if (SWrapObj && SWrapNode==hTarget ) {
				endEdit:
				if (createInterface->GetCommandMode()->ID() == CID_STDPICK) 
				{ if (SWrapObj->creating) 
						{  theCreateSWrapObjectMode.JumpStart(SWrapObj->ip,SWrapObj);
							createInterface->SetCommandMode(&theCreateSWrapObjectMode);
					    } 
				  else {createInterface->SetStdCommandMode(CID_OBJMOVE);}
				}
#ifdef _OSNAP
				SWrapObj->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
				SWrapObj->EndEditParams( (IObjParam*)createInterface, 0, NULL);
				SWrapObj  = NULL;				
				SWrapNode    = NULL;
				CreateNewObject();	
				attachedToNode = FALSE;
				}
			break;		
		}
	return REF_SUCCEED;
	}

void CreateSWrapObjectProc::Begin( IObjCreate *ioc, ClassDesc *desc )
	{
	createInterface = ioc;
	cDesc           = desc;
	attachedToNode  = FALSE;
	createCB        = NULL;
	SWrapNode         = NULL;
	SWrapObj       = NULL;
	dostuff=0;
	CreateNewObject();
	}
void CreateSWrapObjectProc::CreateNewObject()
	{
	SWrapObj = (SWrapObject*)cDesc->Create();
	lastPutCount  = theHold.GetGlobalPutCount();
	
	// Start the edit params process
	if ( SWrapObj ) {
#ifndef NO_CREATE_TASK	// russom - 12/04/01
		SWrapObj->BeginEditParams( (IObjParam*)createInterface, BEGIN_EDIT_CREATE, NULL );
#endif
#ifdef _OSNAP
		SWrapObj->SetAFlag(A_OBJ_LONG_CREATE);
#endif
		}	
	}

//LACamCreationManager::~LACamCreationManager
void CreateSWrapObjectProc::End()
{ if ( SWrapObj ) { 
#ifdef _OSNAP
		SWrapObj->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
#ifndef NO_CREATE_TASK	// russom - 12/04/01
	SWrapObj->EndEditParams( (IObjParam*)createInterface, 
	                    	          END_EDIT_REMOVEUI, NULL);
#endif
		if ( !attachedToNode ) 
		{	// RB 4-9-96: Normally the hold isn't holding when this 
			// happens, but it can be in certain situations (like a track view paste)
			// Things get confused if it ends up with undo...
			theHold.Suspend(); 
			delete SWrapObj;
			SWrapObj = NULL;
			theHold.Resume();
			if (theHold.GetGlobalPutCount()!=lastPutCount) 
				GetSystemSetting(SYSSET_CLEAR_UNDO);
		} 
 else if ( SWrapNode ) {
			 // Get rid of the reference.
			theHold.Suspend();
			DeleteReference(0);  // sets camNode = NULL
			theHold.Resume();
			}
	}
}

void CreateSWrapObjectMode::JumpStart(IObjParam *i,SWrapObject *o)
	{
	ip  = i;
	obj = o;
	//MakeRefByID(FOREVER,0,svNode);
#ifndef NO_CREATE_TASK	// russom - 12/04/01
	obj->BeginEditParams(i,BEGIN_EDIT_CREATE,NULL);
#endif
	}


int SWrapClassDesc::BeginCreate(Interface *i)
	{	
	IObjCreate *iob = i->GetIObjCreate();
	theCreateSWrapObjectMode.Begin(iob,this);
	iob->PushCommandMode(&theCreateSWrapObjectMode);
	return TRUE;
	}

int SWrapClassDesc::EndCreate(Interface *i)
	{
	theCreateSWrapObjectMode.End();
	i->RemoveMode(&theCreateSWrapObjectMode);
	return TRUE;
	}

int CreateSWrapObjectProc::createmethod(
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
				SWrapObj->pblock->SetValue(PB_ICONSIZE,0,0.01f);
				SWrapObj->pmapParam->Invalidate();
				break;

			case 1: {
				mat.IdentityMatrix();
				sp1 = m;
				p1  = vpt->SnapPoint(m,m,NULL,snapdim);
				Point3 center = (p0+p1)/float(2);
				mat.SetTrans(center);
				SWrapObj->pblock->SetValue(PB_ICONSIZE,0,Length(p1-p0));
				SWrapObj->pmapParam->Invalidate();

				if (msg==MOUSE_POINT) {
					if (Length(m-sp0)<3) {						
						return CREATE_ABORT;
					} else {
					ICustButton *iBut = GetICustButton(GetDlgItem(SWrapObj->hParams,IDC_AP_WRAPBUTTON));
					iBut->Enable();
					ReleaseICustButton(iBut);
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

int CreateSWrapObjectProc::proc(HWND hwnd,int msg,int point,int flag,
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
					assert( SWrapObj );					
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
                    SWrapObj->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
				   		SWrapObj->EndEditParams( (IObjParam*)createInterface, 0, NULL);
						if (SWrapNode) {
							theHold.Suspend();
							DeleteReference(0);
							theHold.Resume();
							}

						// new object
						CreateNewObject();   // creates SWrapObj
						}

				   	theHold.Begin();	 // begin hold for undo
					mat.IdentityMatrix();

					// link it up
					SWrapNode = createInterface->CreateObjectNode( SWrapObj);
					attachedToNode = TRUE;
					assert( SWrapNode );					
					createCB = NULL;
					createInterface->SelectNode( SWrapNode );
					
					// Reference the new node so we'll get notifications.
					theHold.Suspend();
					MakeRefByID( FOREVER, 0, SWrapNode);
					theHold.Resume();
					mat.IdentityMatrix();
				default:				
					res = createmethod(vpx,msg,point,flag,m,mat);
					createInterface->SetNodeTMRelConstPlane(SWrapNode, mat);
					if (res==CREATE_ABORT)
						goto abort;
					if (res==CREATE_STOP){
#ifdef _OSNAP
                        SWrapObj->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
						theHold.Accept(GetString(IDS_AP_CREATE));	
					}
					createInterface->RedrawViews(createInterface->GetTime()); 
					break;
				}			
			break;

		case MOUSE_MOVE:
			res = createmethod(vpx,msg,point,flag,m,mat);
			createInterface->SetNodeTMRelConstPlane(SWrapNode, mat);
			if (res==CREATE_ABORT)
				goto abort;
			if (res==CREATE_STOP){
#ifdef _OSNAP
               SWrapObj->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
			   theHold.Accept(GetString(IDS_AP_CREATE));	
			}
			createInterface->RedrawViews(createInterface->GetTime()); 
		break;

/*			res = createmethod(vpx,msg,point,flag,m,mat);
			createInterface->SetNodeTMRelConstPlane(SWrapNode, mat);
			if (res==CREATE_ABORT)
				goto abort;
			if (res==CREATE_STOP)
				theHold.Accept(GetString(IDS_AP_CREATE));	
			createInterface->RedrawViews(createInterface->GetTime()); 
		break;*/
	case MOUSE_PROPCLICK:
		createInterface->SetStdCommandMode(CID_OBJMOVE);
		break;
	case MOUSE_ABORT: 
	abort:
		assert( SWrapObj );
#ifdef _OSNAP
			SWrapObj->ClearAFlag(A_OBJ_LONG_CREATE);
#endif
		SWrapObj->EndEditParams( (IObjParam*)createInterface,0,NULL);
		theHold.Cancel();	 // deletes both the object and target.
		if (theHold.GetGlobalPutCount()!=lastPutCount) 
					GetSystemSetting(SYSSET_CLEAR_UNDO);
		SWrapNode = NULL;			
		createInterface->RedrawViews(createInterface->GetTime()); 
		CreateNewObject();	
		attachedToNode = FALSE;
		res = FALSE;
		break;
	case MOUSE_FREEMOVE:
			SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR)));
#ifdef _OSNAP  //PREVIEW SNAP
			res = createmethod(vpx,msg,point,flag,m,mat);
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

BOOL SWrapPickOperand::Filter(INode *node)
	{
	if (node)
	{ ObjectState os = node->GetObjectRef()->Eval(po->ip->GetTime());
	  if (!IsGEOM(os.obj)) 
	  {		node = NULL;
			return FALSE;
	  }
	node->BeginDependencyTest();
	po->NotifyDependents (FOREVER, 0, REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) return FALSE;

	}
	return node ? TRUE : FALSE;
	}

BOOL SWrapPickOperand::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
	{	
	INode *node = ip->PickNode(hWnd,m,this);
	
	if (node) 
	{	ObjectState os = node->GetObjectRef()->Eval(ip->GetTime());
	  if (!IsGEOM(os.obj)) 
		{	node = NULL;
			return FALSE;
			}
	node->BeginDependencyTest();
	po->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) return FALSE;
	}
	return node ? TRUE : FALSE;
	}

void SWrapObject::ShowName()
{TSTR name=TSTR(GetString(IDS_AP_OBJECTSTR)) + (custnode ? custname : TSTR(GetString(IDS_AP_NONE)));
SetWindowText(GetDlgItem(hParams, IDC_AP_WRAPPICKOBJ), name);
}

BOOL SWrapPickOperand::Pick(IObjParam *ip,ViewExp *vpt)
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
	po->cmValid.SetEmpty();
	po->pmapParam->Invalidate();
	po->ShowName();	
	po->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	if (po->creating) {
		theCreateSWrapObjectMode.JumpStart(ip,po);
		ip->SetCommandMode(&theCreateSWrapObjectMode);
		ip->RedrawViews(ip->GetTime());
		return FALSE;
	} else {
		return TRUE;
		}
	}

void SWrapPickOperand::EnterMode(IObjParam *ip)
	{
	ICustButton *iBut;
	iBut=GetICustButton(GetDlgItem(po->hParams,IDC_AP_WRAPBUTTON));
	if (iBut) iBut->SetCheck(TRUE);
	ReleaseICustButton(iBut);
	GetCOREInterface()->PushPrompt(GetString(IDS_AP_PICKMODE));
	}

void SWrapPickOperand::ExitMode(IObjParam *ip)
	{
	ICustButton *iBut;
	iBut=GetICustButton(GetDlgItem(po->hParams,IDC_AP_WRAPBUTTON));
	if (iBut) iBut->SetCheck(FALSE);
	ReleaseICustButton(iBut);
    GetCOREInterface()->PopPrompt();
	}
class SWrapObjectDlgProc : public ParamMapUserDlgProc {
	public:
		SWrapObject *po;

		SWrapObjectDlgProc(SWrapObject *p) {po=p;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void Update(TimeValue t);
		void DeleteThis() {delete this;}
	};

void SWrapObjectDlgProc::Update(TimeValue t)
{ float size;
	po->pblock->GetValue(PB_ICONSIZE,0,size,FOREVER);
	ICustButton *iBut = GetICustButton(GetDlgItem(po->hParams,IDC_AP_WRAPBUTTON));
	if (size<0.01f) iBut->Disable(); else iBut->Enable();
	ReleaseICustButton(iBut);
	po->ShowName();
}

BOOL SWrapObjectDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{int FToTick=(int)((float)TIME_TICKSPERSEC/(float)GetFrameRate());
	switch (msg) {
		case WM_INITDIALOG: {
			ICustButton *iBut = GetICustButton(GetDlgItem(hWnd,IDC_AP_WRAPBUTTON));
			iBut->SetType(CBT_CHECK);
			iBut->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(iBut);
			po->hParams=hWnd;
			Update(t);
			return FALSE;	// stop default keyboard focus - DB 2/27  
			}
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{    case IDC_AP_WRAPBUTTON:
				   { if (po->ip->GetCommandMode()->ID() == CID_STDPICK) 
					{ if (po->creating) 
						{  theCreateSWrapObjectMode.JumpStart(po->ip,po);
							po->ip->SetCommandMode(&theCreateSWrapObjectMode);
						} else {po->ip->SetStdCommandMode(CID_OBJMOVE);}
					} else 
						{ po->pickCB.po = po;						
						  po->ip->SetPickMode(&po->pickCB);
						}
					break;
				}

			}
			break;	
		}
	return TRUE;
	}

SWrapObject::SWrapObject()
{ TimeValue tpf=GetTicksPerFrame();
	MakeRefByID(FOREVER, 0, 
		CreateParameterBlock(SWrapdescVer0, PBLOCK_LENGTH, CURRENT_VERSION));
	assert(pblock);	

	cmValid.SetEmpty();
	srand(12345);
	custname=TSTR(_T(" "));
	custnode=NULL;
	cmesh=NULL;
	tm.IdentityMatrix();
	vnorms.ZeroCount();
	fnorms.ZeroCount();
	pblock->SetValue(PB_KIDEFAULT,0,0.0f);
	pblock->SetValue(PB_USESELVERTS,0,0);
	pblock->SetValue(PB_STANDOFF,0,1.0f);
}

SWrapObject::~SWrapObject()
{ pblock=NULL;
  DeleteAllRefsFromMe();
  if (cmesh) delete cmesh;
  vnorms.SetCount(0);vnorms.Shrink();
  fnorms.SetCount(0);fnorms.Shrink();
}

Modifier *SWrapObject::CreateWSMMod(INode *node)
	{
	return new SWrapMod(node,this);
	}

RefTargetHandle SWrapObject::Clone(RemapDir& remap) 
	{
	SWrapObject* newob = new SWrapObject();	
	newob->ReplaceReference(0,pblock->Clone(remap));
	if (custnode) newob->ReplaceReference(CUSTNODE,custnode);
	BaseClone(this, newob, remap);
	return newob;
	}

void SWrapObject::BeginEditParams(
		IObjParam *ip,ULONG flags,Animatable *prev)
	{
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
			GetString(IDS_AP_TOP), 
			(LPARAM)ip,APPENDROLL_CLOSED);

		// Gotta make a new one.
		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_AP_SURFWRAP),
			GetString(IDS_AP_PARAMETERS),
			0);
		}
		if (pmapParam)
			pmapParam->SetUserDlgProc(new SWrapObjectDlgProc(this));
	}

void SWrapObject::EndEditParams(
		IObjParam *ip, ULONG flags,Animatable *next)
	{		
//	SimpleWSMObject::EndEditParams(ip,flags,next);
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

void SWrapObject::BuildMesh(TimeValue t)
{	ivalid = FOREVER;
	float width;
	pblock->GetValue(PB_ICONSIZE,t,width,ivalid);

	float hwidth;
	width  *= 0.5f;
	hwidth =width*0.5f;

	mesh.setNumVerts(10);
	mesh.setNumFaces(5);
	mesh.setVert(0, Point3(width,width, width));
	mesh.setVert(1, Point3(-width, width, width));
	mesh.setVert(2, Point3(-width, -width, width));
	mesh.setVert(3, Point3( width, -width,width));
	mesh.setVert(4, Point3( 0.0f, 0.0f,width));
	mesh.setVert(5, Point3( 0.0f, 0.0f,-width));
	mesh.setVert(6, Point3(hwidth, 0.0f,-hwidth));
	mesh.setVert(7, Point3(-hwidth, 0.0f,-hwidth));
	mesh.setVert(8, Point3(0.0f,hwidth,-hwidth));
	mesh.setVert(9, Point3(0.0f,-hwidth,-hwidth));

	mesh.faces[0].setVerts(3,0,1);
	mesh.faces[0].setEdgeVisFlags(1,1,0);
	mesh.faces[0].setSmGroup(0);

	mesh.faces[1].setEdgeVisFlags(1,1,0);
	mesh.faces[1].setSmGroup(0);
	mesh.faces[1].setVerts(1,2,3);

	mesh.faces[2].setEdgeVisFlags(1,1,1);
	mesh.faces[2].setSmGroup(0);
	mesh.faces[2].setVerts(5,6,7);

	mesh.faces[3].setEdgeVisFlags(1,1,1);
	mesh.faces[3].setSmGroup(0);
	mesh.faces[3].setVerts(5,8,9);

	mesh.faces[4].setEdgeVisFlags(0,1,0);
	mesh.faces[4].setSmGroup(0);
	mesh.faces[4].setVerts(3,4,5);

	mesh.InvalidateGeomCache();
}

void SWrapObject::InvalidateUI() 
	{
	if (pmapParam) pmapParam->Invalidate();
	}

ParamDimension *SWrapObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {		
		case PB_ICONSIZE:
		case PB_STANDOFF:
		case PB_KIDEFAULT:	 return stdWorldDim;
		default: return defaultDim;
		}
	}

TSTR SWrapObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {				
		case PB_KIDEFAULT: 	return GetString(IDS_AP_KIDEFAULT);
		case PB_USESELVERTS: 	return GetString(IDS_AP_SELVERTS);
		case PB_ICONSIZE: 	return GetString(IDS_AP_ICONSIZE);
		case PB_STANDOFF: 	return GetString(IDS_AP_STANDOFF);
		default: 			return TSTR(_T(""));
		}
	}

//--- DeflectMod methods -----------------------------------------------

SWrapMod::SWrapMod(INode *node,SWrapObject *obj)
	{	
//	MakeRefByID(FOREVER,SIMPWSMMOD_OBREF,obj);
	MakeRefByID(FOREVER,SIMPWSMMOD_NODEREF,node);	
	pblock = NULL;
	obRef=NULL;
	}

Interval SWrapMod::GetValidity(TimeValue t) 
{ 	if (obRef && nodeRef)
	{	Interval valid = FOREVER;
		Matrix3 tm;
		float f;		
		SWrapObject *obj = (SWrapObject*)GetWSMObject(t);
		if (obj->custnode) 
		{ valid=obj->cmValid; 	}
		obj->pblock->GetValue(PB_KIDEFAULT,t,f,valid);
		obj->pblock->GetValue(PB_STANDOFF,t,f,valid);
		tm = nodeRef->GetObjectTM(t,&valid);

		return valid;
	} else {return FOREVER;	}
}

class SWrapDeformer : public Deformer {
	public:		
		Point3 Map(int i, Point3 p) {return p;}
	};
static SWrapDeformer SWrapdeformer;

Deformer& SWrapMod::GetDeformer(
		TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
	{
	return SWrapdeformer;
	}

RefTargetHandle SWrapMod::Clone(RemapDir& remap) 
	{
	SWrapMod *newob = new SWrapMod(nodeRef,(SWrapObject*)obRef);	
	newob->SimpleWSMModClone(this);
	BaseClone(this, newob, remap);
	return newob;
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
 static BOOL IsShape(Object *pobj,TimeValue t,BezierShape *bshape)
{ 
  return FALSE;
}

 #define EPSILON	0.0001f

int RayIntersect(Ray& ray, float& at, Point3& norm,Mesh *amesh,Tab<VNormal> vnorms,Tab<Point3> fnorms)
{	Face *face = amesh->faces;	
	Point3 n, p, bry,bary;
	float d, rn, a;
	BOOL first = FALSE;

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
		if (first) {if (a > at) continue;}
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
//		bary.x  = bry.z;
//		bary.y  = bry.x;
//		bary.z  = bry.y;
		bary = bry; // DS 3/8/97
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

Point3 DoIntersect(Point3 vert,Ray ray,Mesh *mesh,float kdef,float standoff,int *kfound,Point3 v,Tab<VNormal> vnorms,Tab<Point3> fnorms) 
{ ray.p=vert;
  float at;Point3 norm;
  (*kfound)=RayIntersect(ray,at,norm,mesh,vnorms,fnorms);
  if (!(*kfound)) vert+=kdef*v;
  else 
  { Point3 intersect=ray.p+ray.dir*at;
    intersect=intersect;
    intersect=vert-intersect;
    vert=vert+v*(Length(intersect)-standoff);
  }
  return vert;
}

void GetVFLst(Mesh* dmesh,Tab<VNormal>* vnorms,Tab<Point3>* fnorms)	 
{ int nv=dmesh->getNumVerts();	
  int nf=dmesh->getNumFaces();	
  (*fnorms).Resize(nf);
  (*fnorms).SetCount(nf);
  (*vnorms).Resize(nv);
  (*vnorms).SetCount(nv);
  Face *face = dmesh->faces;
  for (int i=0; i<nv; i++) 
    (*vnorms)[i] = VNormal();
  Point3 v0, v1, v2;
  for (i=0; i<dmesh->getNumFaces(); i++,face++) 
  {	// Calculate the surface normal
	v0 = dmesh->verts[face->v[0]];
	v1 = dmesh->verts[face->v[1]];
	v2 = dmesh->verts[face->v[2]];
	(*fnorms)[i] = (v1-v0)^(v2-v1);
	for (int j=0; j<3; j++) 
	   (*vnorms)[face->v[j]].AddNormal((*fnorms)[i],face->smGroup);
    (*fnorms)[i] = Normalize((*fnorms)[i]);
  }
  for (i=0; i<nv; i++) 
	(*vnorms)[i].Normalize();
}
/*#define EDITABLE_SURF_CLASS_ID Class_ID(0x76a11646, 0x12a821fa)
#define FITPOINT_PLANE_CLASS_ID Class_ID(0x76a11646, 0xbadbeef)
#define EDITABLE_CVCURVE_CLASS_ID Class_ID(0x76a11646, 0x12a82144)
#define EDITABLE_FPCURVE_CLASS_ID Class_ID(0x76a11646, 0x12a82142)*/


void SWrapMod::ModifyObject(
		TimeValue t, ModContext &mc, ObjectState *os, INode *node)
{	SWrapObject *obj = (SWrapObject *)GetWSMObject(t);
	INode *pnode;
	TriObject *towrapOb=NULL;Object *pobj=NULL;
	if (obj) pnode=obj->custnode;
	if (obj && nodeRef && pnode) 
	{	Interval valid = FOREVER;
		if (!obj->cmValid.InInterval(t))
		{   pobj = pnode->EvalWorldState(t).obj;
			obj->cmValid=pobj->ObjectValidity(t);
			Matrix3 tm=pnode->GetObjectTM(t,&(obj->cmValid));
			TriObject *wrapOb=IsUseable(pobj,t);
			if (wrapOb)
			{ if (obj->cmesh) delete obj->cmesh; 
			  obj->cmesh=new Mesh;
			   obj->cmesh->DeepCopy(&wrapOb->GetMesh(),
			     PART_GEOM|SELECT_CHANNEL|PART_SUBSEL_TYPE|PART_TOPO|TM_CHANNEL);
			   for (int ic=0;ic<obj->cmesh->getNumVerts();ic++)
			     obj->cmesh->verts[ic]=obj->cmesh->verts[ic]*tm;
			  GetVFLst(obj->cmesh,&obj->vnorms,&obj->fnorms);
			  if (wrapOb!=pobj) wrapOb->DeleteThis();
			}
 		}
		if (!obj->cmesh) return;
		if ((obj->cmesh->getNumVerts()==0)||(obj->cmesh->getNumFaces()==0)) 
			return;		 
//		Matrix3 invtm=Inverse(obj->tm);
		valid=obj->cmValid;
		Matrix3 ctm;
		ctm = nodeRef->GetNodeTM(t,&valid);
		Ray ray;
		Point3 v=-ctm.GetRow(2);
//		Matrix3 nooff=invtm;nooff.NoTrans();
		ray.dir=v;//*nooff;
		int selverts;
		float kdef,standoff;
		obj->pblock->GetValue(PB_USESELVERTS,t,selverts,valid);
		obj->pblock->GetValue(PB_KIDEFAULT,t,kdef,valid);
		obj->pblock->GetValue(PB_STANDOFF,t,standoff,valid);
		BezierShape stowrapOb;
		int found=0;
		Matrix3 towtm(1);
		if (os->GetTM()) 
			towtm=*(os->GetTM());
		Matrix3 invtowtm=Inverse(towtm);
		Point3 vert;
		Class_ID cid=os->obj->ClassID(),es=EDITABLE_SURF_CLASS_ID,efp=FITPOINT_PLANE_CLASS_ID,ecv=EDITABLE_CVCURVE_CLASS_ID,ecfp=EDITABLE_FPCURVE_CLASS_ID;
		if (((cid==EDITABLE_SURF_CLASS_ID)||(cid==FITPOINT_PLANE_CLASS_ID))||((cid==EDITABLE_CVCURVE_CLASS_ID)||(cid==EDITABLE_FPCURVE_CLASS_ID)))
		{ Object* nurbobj=os->obj;
		  int num=nurbobj->NumPoints();
		  for (int i=0;i<num;i++)
		  {	vert=DoIntersect(nurbobj->GetPoint(i)*towtm,ray,obj->cmesh,kdef,standoff,&found,v,obj->vnorms,obj->fnorms); 
		    nurbobj->SetPoint(i,(vert*invtowtm));
		  }
		}
#ifndef NO_PATCHES
		else if (os->obj->IsSubClassOf(patchObjectClassID))
		{ PatchObject* patchob=(PatchObject *)os->obj;
		  PatchMesh *pm=&(patchob->patch);
		  int nv=pm->getNumVerts();
		  BitArray sel = pm->VertSel();
		  for (int i=0;i<nv;i++)
		  { if (!selverts||sel[i])
			{ vert=DoIntersect(pm->getVert(i).p*towtm,ray,obj->cmesh,kdef,standoff,&found,v,obj->vnorms,obj->fnorms); 
		      vert=vert*invtowtm;
		      pm->setVert(i,vert);
			}
		  }
/*		  pm->buildLinkages();
		  pm->computeInteriors();
		  pm->InvalidateGeomCache();*/
		}
#endif // NO_PATCHES
		else if (towrapOb=IsUseable(os->obj,t))
		{	Point3 tvector;
			float dist;
			float *vssel = NULL;
		  if (selverts) 
			  vssel = towrapOb->GetMesh().getVSelectionWeights();
		  for (int i=0;i<towrapOb->GetMesh().getNumVerts();i++)
		  { 
				if ((!selverts)||(towrapOb->GetMesh().vertSel[i])
							   ||(vssel&&vssel[i]))
				{
					vert = DoIntersect(towrapOb->GetMesh().verts[i]*towtm,ray,obj->cmesh,kdef,standoff,&found,v,obj->vnorms,obj->fnorms); 
					vert = vert*invtowtm;
					if (vssel&&vssel[i])
					{
						tvector = vert - towrapOb->GetMesh().verts[i];
						dist = Length(tvector);
						if ((float)fabs(dist) > EPSILON)
							tvector = tvector/dist;
						else
							tvector = Zero;
						vert = towrapOb->GetMesh().verts[i] + dist*vssel[i]*tvector;
					}
					towrapOb->GetMesh().verts[i] = vert;
				}
		  }
		  if (towrapOb!=os->obj) 
			  towrapOb->DeleteThis();
		}
		else if((os->obj->IsSubClassOf(splineShapeClassID))||(os->obj->CanConvertToType(splineShapeClassID))) 
		{ SplineShape *attSplShape = (SplineShape *)os->obj->ConvertToType(t,splineShapeClassID);
		if (attSplShape) 
		{ stowrapOb=attSplShape->shape;
		  for (int poly=0; poly<stowrapOb.splineCount; ++poly)
		  { Spline3D *spline = stowrapOb.GetSpline(poly);
			int verts = spline->Verts();
			int knots = spline->KnotCount();
			BitArray sel = stowrapOb.VertexTempSel(poly);
			Point3 cknot,cknot2;
			{ for(int k=0; k<knots; ++k) 
			  {	int vert = k * 3 + 1;
				if (!selverts||sel[vert])  
				{ cknot=DoIntersect(spline->GetKnotPoint(k)*towtm,ray,obj->cmesh,kdef,standoff,&found,v,obj->vnorms,obj->fnorms);
				  attSplShape->shape.SetVert(poly,vert,cknot*invtowtm);
				  if (found)
				  { int knotType = spline->GetKnotType(k);
				    if(knotType & KTYPE_BEZIER) 
				    { cknot2= DoIntersect(spline->GetInVec(k)*towtm,ray,obj->cmesh,kdef,standoff,&found,v,obj->vnorms,obj->fnorms);
					attSplShape->shape.SetVert(poly,vert-1,(found?cknot2:cknot)*invtowtm);
				  	  cknot2= DoIntersect(spline->GetOutVec(k)*towtm,ray,obj->cmesh,kdef,standoff,&found,v,obj->vnorms,obj->fnorms);
					  attSplShape->shape.SetVert(poly,vert+1,(found?cknot2:cknot)*invtowtm);
					}
				  }
				}
			  }
			}
		  }
		  if (attSplShape!=os->obj) attSplShape->DeleteThis();
		}
	  }
//	os->obj->PointsWereChanged();
	os->obj->UpdateValidity(GEOM_CHAN_NUM,valid);	
//	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);

  }
} 

RefTargetHandle SWrapObject::GetReference(int i)
{	switch(i) {
		case PBLK: return(RefTargetHandle)pblock;
		case CUSTNODE: return (RefTargetHandle)custnode;
		default: return NULL;
		}
	}

void SWrapObject::SetReference(int i, RefTargetHandle rtarg) { 
	switch(i) {
		case PBLK: pblock=(IParamBlock*)rtarg; return;
		case CUSTNODE: custnode = (INode *)rtarg; return;
		}
	}

RefResult SWrapObject::NotifyRefChanged( 
		Interval changeInt,
		RefTargetHandle hTarget, 
		PartID& partID, 
		RefMessage message )
	{				
	switch (message) {		
		case REFMSG_CHANGE:	
			{ if (hTarget==custnode) cmValid.SetEmpty();
			  if (hTarget==pblock) InvalidateUI();
			SimpleWSMObject::NotifyRefChanged(changeInt,hTarget,partID,message);
			}
			break;
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
#define SWRAP_CUSTNAME_CHUNK	0x0100

IOResult SWrapObject::Save(ISave *isave)
	{
	isave->BeginChunk(SWRAP_CUSTNAME_CHUNK);		
	isave->WriteWString(custname);
	isave->EndChunk();
	return IO_OK;
	}

IOResult SWrapObject::Load(ILoad *iload)
	{
	IOResult res = IO_OK;
	
	// Default names
	custname = TSTR(_T(" "));

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case SWRAP_CUSTNAME_CHUNK: {
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
