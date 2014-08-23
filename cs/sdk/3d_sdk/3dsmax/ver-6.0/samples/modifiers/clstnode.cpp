/**********************************************************************
 *<
	FILE: clstnode.cpp   

	DESCRIPTION:  Vertex cluster animating modifier that uses nodes

	CREATED BY: Rolf Berteig

	HISTORY: created 27 October, 1995
	         Aug 29, 2000 Param Block2 the node data and added the undo

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "mods.h"
#include "iparamm2.h"
#include "clstnodeapi.h"
#include "modstack.h"
#include "tvnode.h"
#include "notify.h"

#define PBLOCK_REF 1

#define CLUSTNODECONTAINERMASTER_TVNODE_CLASS_ID Class_ID(0xe27e0f2b, 0x74fad505)
#define CLUSTNODECONTAINER_TVNODE_CLASS_ID Class_ID(0xe27e0f2b, 0x74fad594)
#define CLUSTNODE_TVNODE_CLASS_ID Class_ID(0xe27e0f2b, 0x74fad594)


enum { lxform_params};

enum { lxform_node, lxform_backtm };

class ClusterMyEnumProc : public DependentEnumProc 
	{
      public :
      virtual int proc(ReferenceMaker *rmaker); 
      INodeTab Nodes;              
	};

int ClusterMyEnumProc::proc(ReferenceMaker *rmaker) 
	{ 
	if (rmaker->SuperClassID()==BASENODE_CLASS_ID)    
			{
            Nodes.Append(1, (INode **)&rmaker);                 
			}
     return 0;              
	}

class ClustNodeMod;

class ClustNodeNotify  : public TVNodeNotify 
{
public:
ClustNodeMod *s;
ClustNodeNotify(ClustNodeMod *smod);

RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);

};


class ClustNodeModData : public LocalModData {
	public:
		INode *selfNode;

		Matrix3 tm,invtm;
		BOOL reloadLocalTMs;
		
		ClustNodeModData()
			{
			selfNode = NULL;
			tm.IdentityMatrix();
			invtm.IdentityMatrix();
			reloadLocalTMs = FALSE;
			}
		~ClustNodeModData()
			{
			}	
		LocalModData*	Clone()
			{
			ClustNodeModData* d = new ClustNodeModData();
			d->selfNode = NULL;
//			d->reloadLocalTMs = TRUE;
			d->tm = tm;
			d->invtm = invtm;			
			return d;
			}	


	};



class ClustNodeMod : public IClustNodeMod {	
	public:

		//watje new paramblock2
		IParamBlock2 *pblock;


		INode *tempNode;
		static IObjParam *ip;
		static HWND hParams;
		Matrix3 tm, invtm;

		ClustNodeMod();
		~ClustNodeMod();

		// From Animatable
		void DeleteThis() {delete this;}
		void GetClassName(TSTR& s) { s= GetString(IDS_RB_CLUSTNODEMOD); }  
		virtual Class_ID ClassID() { return Class_ID(CLUSTNODEOSM_CLASS_ID,0);}
		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip,ULONG flags,Animatable *next);		
		TCHAR *GetObjectName() {return GetString(IDS_RB_NODEXFORM);}
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 

		ChannelMask ChannelsUsed()  {return PART_GEOM|PART_TOPO|PART_SELECT|PART_SUBSEL_TYPE;}
		ChannelMask ChannelsChanged() {return PART_GEOM;}
		Class_ID InputType() {return defObjectClassID;}
		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		Interval LocalValidity(TimeValue t);

		int NumRefs() {return 2;}
		RefTargetHandle GetReference(int i) 
			{
			if (i == 0) return tempNode;
			else if (i == 1) return pblock;
			else return NULL;
			}

		void SetReference(int i, RefTargetHandle rtarg) 
			{
			if (i==0)
				tempNode=(INode*)rtarg;
			else if (i==1) pblock = (IParamBlock2*)rtarg;
			}
		
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);
				
		IOResult SaveLocalData(ISave *isave, LocalModData *pld);
		IOResult LoadLocalData(ILoad *iload, LocalModData **pld);
				
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);

// JBW: direct ParamBlock access is added
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { if (i == 0) return pblock; 
											else return NULL;
												} // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) {if (pblock->ID() == id) return pblock;
													 else return  NULL; } // return id'd ParamBlock

		//******************************
		// --- Methods for FPMixinInterface --- 
		INode*	getControlNode(TimeValue t);
		void	setControlNode(INode* node, TimeValue t);

		//Function Publishing method (Mixin Interface)
		BaseInterface* GetInterface(Interface_ID id) 
		{ 
			if (id == CLUSTNODE_MOD_INTERFACE) 
				return (IClustNodeMod*)this; 
			else 
				return FPMixinInterface::GetInterface(id);
		} 
		//******************************
		
		BOOL redoBaseTM;
		Matrix3 initialOffset;
		Matrix3 initialBaseTM;
		INode* GetNodeFromModContext(ModContext *smd, int &which);
		
		int ver;
		
		ITrackViewNode *container;
		ClustNodeNotify *notify;
		INode *CheckForXRefs(TimeValue t);
		
		
	};


ClustNodeNotify::ClustNodeNotify(ClustNodeMod *smod)
	{
	s = smod;
	}

RefResult ClustNodeNotify::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message)
	{
	if (message == REFMSG_CHANGE) 
		{
		if (partID & PART_TM)
			{
//			if (s->ip)
				{
				BOOL backTM;
				s->pblock->GetValue(lxform_backtm,0,backTM,FOREVER);
				if (backTM)
					s->NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
//					s->pblock->SetValue(lxform_backtm,0,backTM);
				}
			}
		}
	return REF_SUCCEED ;
	}

//-----------------------------------------------------------------

class LXNodeRestore : public RestoreObj {
	public:

		ClustNodeMod *mod;
		INode *unode;
		Matrix3 uTm,uInvTm;

		INode *rnode;
		Matrix3 rTm,rInvTm;

		LXNodeRestore(ClustNodeMod *mod, INode *rnode) 
			{
			this->mod = mod;
			INode *node;
			mod->pblock->GetValue(lxform_node,0,node,FOREVER);
			unode = node;
			uTm = mod->tm;
			uInvTm = mod->invtm;

			this->rnode = rnode;
			}   		
		void Restore(int isUndo) 
			{
			if (isUndo)
				{
				rTm = mod->tm;
				rInvTm = mod->invtm;
				}

			mod->pblock->SetValue(lxform_node,0,unode);
			mod->tm = uTm;
			mod->invtm = uInvTm;

			if (unode)
				{
				SetWindowText(GetDlgItem(mod->hParams,IDC_CLUST_NODENAME),
					unode->GetName());
				}
			else 
				{
				SetWindowText(GetDlgItem(mod->hParams,IDC_CLUST_NODENAME),
					GetString(IDS_RB_NONE));
				}

			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
		void Redo()
			{

			mod->pblock->SetValue(lxform_node,0,rnode);
			mod->tm = rTm;
			mod->invtm = rInvTm;

			if (rnode)
				{
				SetWindowText(GetDlgItem(mod->hParams,IDC_CLUST_NODENAME),
					rnode->GetName());
				}
			else 
				{
				SetWindowText(GetDlgItem(mod->hParams,IDC_CLUST_NODENAME),
					GetString(IDS_RB_NONE));
				}

			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}		
		void EndHold() 
			{ 
			mod->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T("Linked XForm Add Node")); }
	};




//--- ClassDescriptor and class vars ---------------------------------

IObjParam* ClustNodeMod::ip = NULL;
HWND ClustNodeMod::hParams  = NULL;

class ClustNodeClassDesc:public ClassDesc2 {
	public:
#ifndef DESIGN_VER
	int 			IsPublic() { return 1; }
#else 
	int 			IsPublic() { return 0; }
#endif // !DESIGN_VER
	void *			Create(BOOL loading = FALSE) {return new ClustNodeMod;}
	const TCHAR *	ClassName() {return GetString(IDS_RB_NODEXFORM_CLASS);}
	SClass_ID		SuperClassID() {return OSM_CLASS_ID;}
	Class_ID		ClassID() {return Class_ID(CLUSTNODEOSM_CLASS_ID,0); }
	const TCHAR* 	Category() {return GetString(IDS_RB_DEFDEFORMATIONS);}

// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("LinkedXForm"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle

	};

static ClustNodeClassDesc clustNodeDesc;
extern ClassDesc* GetClustNodeModDesc() {return &clustNodeDesc;}

// per instance geosphere block
static ParamBlockDesc2 lxform_param_blk ( lxform_params, _T("Parameters"),  0, &clustNodeDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//rollout
	IDD_CLUSTNODEPARAM, IDS_RB_PARAMETERS, 0, 0, NULL,
	// params
	// LAM 11/25/00 - changed param name from node to control to match old MXS name

	lxform_node, 	_T(""),		TYPE_INODE, 		0,				IDS_PW_NODE,
		end, 
		
	lxform_backtm, 	_T("backTransform"),		TYPE_BOOL, 		P_RESET_DEFAULT,	IDS_PW_BACKTM,
		p_default, 		TRUE, 
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_BACKTM, 
		end, 		
	end
	);

// LAM - defect 172334
//Function Publishing descriptor for Mixin interface
//*****************************************************
static FPInterfaceDesc ClustNodeMod_interface(
    CLUSTNODE_MOD_INTERFACE, _T("interface"), 0, &clustNodeDesc, FP_MIXIN,
		properties,
		ClustNodeMod::get_control_node, ClustNodeMod::set_control_node, _T("control"), 0, TYPE_INODE,
      end
      );

//  Get Descriptor method for Mixin Interface
//  *****************************************
FPInterfaceDesc* IClustNodeMod::GetDesc()
  {
     return &ClustNodeMod_interface;
  }

// End of Function Publishing Code


//--- CustMod dlg proc ------------------------------

class PickControlNode : 
		public PickModeCallback,
		public PickNodeCallback {
	public:				
		ClustNodeMod *mod;
		PickControlNode() {mod=NULL;}
		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);		
		BOOL Pick(IObjParam *ip,ViewExp *vpt);		
		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);		
		BOOL Filter(INode *node);
		PickNodeCallback *GetFilter() {return this;}
		BOOL RightClick(IObjParam *ip,ViewExp *vpt) {return TRUE;}
	};
static PickControlNode thePickMode;

BOOL PickControlNode::Filter(INode *node)
	{
	node->BeginDependencyTest();
	mod->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) {		
		return FALSE;
	} else {
		return TRUE;
		}
	}

BOOL PickControlNode::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
	{	
	if (ip->PickNode(hWnd,m,this)) {
		return TRUE;
	} else {
		return FALSE;
		}
	}

BOOL PickControlNode::Pick(IObjParam *ip,ViewExp *vpt)
	{
	INode *node = vpt->GetClosestHit();
	if (node) {
		// RB 3/1/99: This should use the node tm not the object TM. See ModifyObject() imp.
		Matrix3 ourTM,ntm = node->GetNodeTM(ip->GetTime()); //node->GetObjectTM(ip->GetTime());	

		ModContextList mcList;
		INodeTab nodes;
		ip->GetModContexts(mcList,nodes);
		assert(nodes.Count());
		ourTM = nodes[0]->GetObjectTM(ip->GetTime());
		


		theHold.Begin();
		theHold.Put(new LXNodeRestore(mod, node));

		mod->initialBaseTM = ourTM;
		
		mod->tm    = ourTM * Inverse(ntm);
		mod->invtm = Inverse(ourTM);

		mod->pblock->SetValue(lxform_node,0,node);
		theHold.Accept(GetString(IDS_RB_NODEXFORM));

		SetWindowText(GetDlgItem(mod->hParams,IDC_CLUST_NODENAME),
					node->GetName());
		nodes.DisposeTemporary();
		}
	return TRUE;
	}

void PickControlNode::EnterMode(IObjParam *ip)
	{
	ICustButton *iBut = GetICustButton(GetDlgItem(mod->hParams,IDC_CLUST_PICKNODE));
	if (iBut) iBut->SetCheck(TRUE);
	ReleaseICustButton(iBut);
	}

void PickControlNode::ExitMode(IObjParam *ip)
	{
	ICustButton *iBut = GetICustButton(GetDlgItem(mod->hParams,IDC_CLUST_PICKNODE));
	if (iBut) iBut->SetCheck(FALSE);
	ReleaseICustButton(iBut);
	}



class LXFormProc : public ParamMap2UserDlgProc {
public:
	ClustNodeMod *mod;
	HWND hWnd;
	LXFormProc () { mod = NULL; }
	BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
	void DeleteThis() {  }


};

static LXFormProc theLXFormProc;


BOOL LXFormProc::DlgProc (TimeValue t, IParamMap2 *map,
										HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (!mod) return FALSE;

	switch (msg) {
		case WM_INITDIALOG: {
			mod->hParams = hWnd;
			ICustButton *iBut = GetICustButton(GetDlgItem(hWnd,IDC_CLUST_PICKNODE));
			iBut->SetType(CBT_CHECK);
			iBut->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(iBut);

			INode *node = NULL;
			mod->pblock->GetValue(lxform_node,t,node,FOREVER);
			if (node) {
				SetWindowText(GetDlgItem(hWnd,IDC_CLUST_NODENAME),
					node->GetName());
			} else {
				SetWindowText(GetDlgItem(hWnd,IDC_CLUST_NODENAME),
					GetString(IDS_RB_NONE));
				}
			break;
			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_CLUST_PICKNODE:
					thePickMode.mod  = mod;					
					mod->ip->SetPickMode(&thePickMode);
					break;
				}
			break;
		
		default:
			return FALSE;
		}
	return TRUE;
}



//--- ClustMod methods -------------------------------

ClustNodeMod::ClustNodeMod()
	{
	initialBaseTM.IdentityMatrix();

	tempNode  = NULL;
	tm    = Matrix3(1);
	invtm = Matrix3(1);

	redoBaseTM = FALSE;
	ver = 600;
	GetClustNodeModDesc()->MakeAutoParamBlocks(this);

	container = NULL;
	notify = new ClustNodeNotify(this);

	}

ClustNodeMod::~ClustNodeMod()
	{
	DeleteAllRefsFromMe();
	
	if (container && notify)
		{
		ITrackViewNode *tvr = GetCOREInterface()->GetTrackViewRootNode();
		ITrackViewNode *global = tvr->GetNode(Class_ID(0xb27e9f2a, 0x73fad370));
		ITrackViewNode *tvroot = global->GetNode(CLUSTNODECONTAINERMASTER_TVNODE_CLASS_ID);
		if (tvroot) 
			{
			int ct = tvroot->NumItems();
			for (int i = 0; i <ct; i++)
				{
				ITrackViewNode *n = tvroot->GetNode(i);
				if (container == n)
					container->UnRegisterTVNodeNotify(notify);
				}
			}
		else
			{
			int ct = tvr->NumItems();
			for (int i = 0; i <ct; i++)
				{
				ITrackViewNode *n = tvr->GetNode(i);
				if (container == n)
					container->UnRegisterTVNodeNotify(notify);
				}


			}	
		}
	if (notify)
		delete notify;	
	
	}

#define CLUSTNODE_TM_CHUNK		0x0100
#define CLUSTNODE_INVTM_CHUNK	0x0110
#define CLUSTNODE_VER_CHUNK		0x0120
#define CLUSTNODE_BACKTM_CHUNK	0x0130
#define BACKPATCH_CHUNK			0x0140

class LXFormPostLoad : public PostLoadCallback 
	{
	public:
		ClustNodeMod *n;
		LXFormPostLoad(ClustNodeMod *ns) {n = ns;}
		void proc(ILoad *iload) {  
			if (n->tempNode != NULL)
				{
				n->pblock->SetValue(lxform_node,0,n->tempNode);
				//n->tempNode = NULL;
				n->ReplaceReference(0,NULL);
				}

			if (n->container != NULL)
				{
				n->container->RegisterTVNodeNotify(n->notify);
				n->container->HideChildren(TRUE);
				}
				
			delete this; 


			} 
	};


IOResult ClustNodeMod::Load(ILoad *iload)
	{
	Modifier::Load(iload);
	IOResult res = IO_OK;
	ver = 500;
	ULONG		nb;	

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case CLUSTNODE_TM_CHUNK:
				tm.Load(iload);
				break;

			case CLUSTNODE_INVTM_CHUNK:
				invtm.Load(iload);
				break;
			case CLUSTNODE_VER_CHUNK:
				iload->Read(&ver,sizeof(ver), &nb);
				break;
			case CLUSTNODE_BACKTM_CHUNK:
				initialBaseTM.Load(iload);
				break;
				
			case BACKPATCH_CHUNK:
				ULONG id;
				iload->Read(&id,sizeof(ULONG), &nb);
				if (id!=0xffffffff)
					{
					iload->RecordBackpatch(id,(void**)&container);
					}

				break;
				
				
			}
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}

	iload->RegisterPostLoadCallback(new LXFormPostLoad(this));
	return IO_OK;
	}

IOResult ClustNodeMod::Save(ISave *isave)
	{
	IOResult	res;
	ULONG		nb;	

	Modifier::Save(isave);
	isave->BeginChunk(CLUSTNODE_TM_CHUNK);
	tm.Save(isave);
	isave->EndChunk();

	isave->BeginChunk(CLUSTNODE_INVTM_CHUNK);
	invtm.Save(isave);
	isave->EndChunk();
	
	if (ver >= 600)
	{
		isave->BeginChunk(CLUSTNODE_VER_CHUNK);
		res = isave->Write(&ver, sizeof(ver), &nb);
		isave->EndChunk();	
	}
	
	isave->BeginChunk(CLUSTNODE_BACKTM_CHUNK);
	initialBaseTM.Save(isave);
	isave->EndChunk();
	
	ULONG id = isave->GetRefID(container);

	isave->BeginChunk(BACKPATCH_CHUNK);
	isave->Write(&id,sizeof(ULONG),&nb);
	isave->EndChunk();
	
	
	return IO_OK;
	}

void ClustNodeMod::BeginEditParams(
		IObjParam *ip, ULONG flags,Animatable *prev)
	{
	this->ip = ip;

	clustNodeDesc.BeginEditParams(ip, this, flags, prev);
	theLXFormProc.mod = this;

	lxform_param_blk.SetUserDlgProc(&theLXFormProc);


	}

void ClustNodeMod::EndEditParams(
		IObjParam *ip,ULONG flags,Animatable *next)
	{
	ip->ClearPickMode();
	this->ip = NULL;
	clustNodeDesc.EndEditParams(ip, this, flags, next);

	}



class ClustNodeDeformer: public Deformer {
	public:
		Matrix3 tm,invtm;
		ClustNodeDeformer(Matrix3 m,Matrix3 mi) {tm=m;invtm=mi;}
		Point3 Map(int i, Point3 p) {return (p*tm)*invtm;}
	};


class XRefEnumProc : public DependentEnumProc 
	{
      public :
      virtual int proc(ReferenceMaker *rmaker); 
      INodeTab Nodes;              
	  BOOL nukeME;
	};

int XRefEnumProc::proc(ReferenceMaker *rmaker) 
	{ 
	if (rmaker->SuperClassID()==BASENODE_CLASS_ID)    
			{
            Nodes.Append(1, (INode **)&rmaker);            
			nukeME = TRUE;
			}
     return 0;              
	}	

BOOL RecurseXRefTree(INode *n, INode *target)
{
for (int i = 0; i < n->NumberOfChildren(); i++)
	{

	INode *child = n->GetChildNode(i);

	if (child == target) 
		{
		return TRUE;
		}
	else RecurseXRefTree(child,target);
	
	}
return FALSE;
}

INode *ClustNodeMod::CheckForXRefs(TimeValue t)
	{

	INode *rootNode = GetCOREInterface()->GetRootNode();
	int xct = rootNode->GetXRefFileCount();

	INode *tempBindNode = NULL;
	
	if (xct > 0)
		{

		XRefEnumProc dep;              
		dep.nukeME = FALSE;
		EnumDependents(&dep);
	
		INode *XRefNode = dep.Nodes[0];


		for (int xid = 0; xid < xct; xid++)
			{
			INode *xroot = rootNode->GetXRefTree(xid);
			BOOL amIanXRef = FALSE;
			if (xroot)
				amIanXRef = RecurseXRefTree(xroot,XRefNode);
			if (amIanXRef)
				{
				tempBindNode = rootNode->GetXRefParent(xid);
				}
			}
		}
		
	return tempBindNode;
	
	}


void ClustNodeMod::ModifyObject(
		TimeValue t, ModContext &mc, ObjectState *os, INode *node)
	{


	
	if (mc.localData == NULL)
	{
		ClustNodeModData *d  = new ClustNodeModData();
		mc.localData = d;
		d->reloadLocalTMs = TRUE;
	}

	ClustNodeModData *d = (ClustNodeModData *) mc.localData;
	
	INode *tnode = NULL;
	pblock->GetValue(lxform_node,t,tnode,FOREVER);
	
	BOOL backtm;
	pblock->GetValue(lxform_backtm,t,backtm,FOREVER);
	
	Matrix3 ntm(1);
	Interval valid = FOREVER;
	if (tnode) 
	{
		
		ntm = tnode->GetNodeTM(t,&valid);
	}

	
	if (redoBaseTM && tnode)
	{
		redoBaseTM = FALSE;
		int which;
		INode *baseNode = GetNodeFromModContext(&mc, which);

		if (baseNode)
		{
			Matrix3 ourTM = baseNode->GetObjectTM(t);
			invtm = Inverse(ourTM);		
			initialBaseTM = ourTM;
		}
	}
	
	
	if (tnode)
		{
		if ((ver >= 600) && (backtm))
		{
			int which;
//			if (d->selfNode == NULL)

			INode *nodeTest = GetNodeFromModContext(&mc, which);
			if (nodeTest == NULL)
			{
				XRefEnumProc dep;              
				dep.nukeME = FALSE;
				EnumDependents(&dep);
					
				if (dep.Nodes.Count() > 0)
					nodeTest = dep.Nodes[0];
			}			
			
			if ((d->selfNode == NULL) || (nodeTest != d->selfNode))
			{
				INode *pnode = d->selfNode;
				d->selfNode = GetNodeFromModContext(&mc, which);
				
				if (d->selfNode == NULL)
				{
					XRefEnumProc dep;              
					dep.nukeME = FALSE;
					EnumDependents(&dep);
					
					if (dep.Nodes.Count() > 0)
						d->selfNode = dep.Nodes[0];
				}
					
				if ((container) && (nodeTest != pnode))
				{
					container->AddController(d->selfNode->GetTMController(),d->selfNode->GetName(),CLUSTNODE_TVNODE_CLASS_ID);
				}
			}
			
			INode *baseNode = d->selfNode;
			
			
			if ((container == NULL) && (baseNode))
				{
		//add a new inode to trackview
				ITrackViewNode *tvr = GetCOREInterface()->GetTrackViewRootNode();
				ITrackViewNode *global = tvr->GetNode(Class_ID(0xb27e9f2a, 0x73fad370));
				ITrackViewNode *tvroot = global->GetNode(CLUSTNODECONTAINERMASTER_TVNODE_CLASS_ID);

				if (!tvroot) 
					{
					ITrackViewNode *mcontainer =  CreateITrackViewNode(TRUE);
					global->AddNode(mcontainer,GetString(IDS_RB_CLUSTNODEMOD),CLUSTNODECONTAINERMASTER_TVNODE_CLASS_ID);
					tvroot = mcontainer;
					}

		//add a new a container
				container =  CreateITrackViewNode(TRUE);
				container->HideChildren(TRUE);
				tvroot->AddNode(container,GetString(IDS_RB_CLUSTNODEMOD),CLUSTNODECONTAINER_TVNODE_CLASS_ID);

				container->AddController(baseNode->GetTMController(),baseNode->GetName(),CLUSTNODE_TVNODE_CLASS_ID);
					
				container->RegisterTVNodeNotify(notify);
				}	
				
			if (tnode && d->selfNode && (d->reloadLocalTMs))
			{
				Matrix3 ourTM,ntm = tnode->GetNodeTM(t); 

				ourTM = d->selfNode->GetObjectTM(t);
				
				d->tm    = ourTM * Inverse(ntm);
				d->invtm = Inverse(ourTM);
				d->reloadLocalTMs = FALSE;
				
			}
									
			
			Matrix3 iBackTM = Inverse(baseNode->GetNodeTM(t));
			Matrix3 ibaseTM  = Inverse(initialBaseTM);
//			Matrix3 itm = initialBaseTM * iBackTM * invtm;
//			Matrix3 itm = iBackTM * initialBaseTM * invtm;
			
			Matrix3 ident(1);
			ClustNodeDeformer deformer(d->tm*ntm *iBackTM,ident);
			os->obj->Deform(&deformer, TRUE);
			os->obj->UpdateValidity(GEOM_CHAN_NUM,valid);	

		}
		else 
		{
			ClustNodeDeformer deformer(tm*ntm,invtm);
			os->obj->Deform(&deformer, TRUE);
			os->obj->UpdateValidity(GEOM_CHAN_NUM,valid);	
		}
		}
	}

Interval ClustNodeMod::LocalValidity(TimeValue t)
	{
	Interval valid = FOREVER;
	INode *tnode = NULL;
	pblock->GetValue(lxform_node,t,tnode,FOREVER);
	if (tnode) {
		tnode->GetNodeTM(t,&valid);
		}
	return valid;
	}

RefTargetHandle ClustNodeMod::Clone(RemapDir& remap)
	{
	ClustNodeMod *newmod = new ClustNodeMod;
	newmod->ver = ver;
	newmod->ReplaceReference(0,tempNode);
	newmod->ReplaceReference(PBLOCK_REF,pblock->Clone(remap));
	newmod->tm    = tm;
	newmod->invtm = invtm;
	
	INode *snode = NULL;
	pblock->GetValue(lxform_node,0,snode,FOREVER);
	if (snode)
	{
		if( remap.FindMapping( snode )  ) 
		{
//			newmod->redoBaseTM = TRUE;
//			Matrix3 currentTM = snode->GetObjectTM(GetCOREInterface()->GetTime());
//			Matrix3 initialTM = 
			newmod->pblock->SetValue(lxform_node,0,(INode*) remap.CloneRef(snode));
		}		
	}
	
	BaseClone(this, newmod, remap);
	return newmod;
	}

RefResult ClustNodeMod::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
		PartID& partID, 
		RefMessage message)
	{
	switch (message) {
		case REFMSG_CHANGE:	
			{ 
			if (hTarget == pblock)
				{
				ParamID changing_param = pblock->LastNotifyParamID();

				lxform_param_blk.InvalidateUI(changing_param);
				}

			break;
			}


		}
	return REF_SUCCEED;
	}


INode* ClustNodeMod::getControlNode(TimeValue t)
{	INode *node;
	pblock->GetValue(lxform_node,0,node,FOREVER);
	return node;

}
void ClustNodeMod::setControlNode(INode* node, TimeValue t)
{
	ModContextList mcList;
	INodeTab nodeTab;

	if (node && node->TestForLoop(FOREVER,this)==REF_SUCCEED) {
		Matrix3 ourTM,ntm = node->GetNodeTM(t);	

		INode* myNode=NULL;
		if (ip==NULL) // if ip != NULL, up in modify panel
		{	myNode = find_scene_obj_node((ReferenceTarget*)this); 
			if (!myNode) return;
		}
		else
		{	ip->GetModContexts(mcList, nodeTab);
			assert(nodeTab.Count());
			myNode = nodeTab[0];
		}
		ourTM = myNode->GetObjectTM(t);
		
		initialBaseTM = ourTM;
		
		theHold.Begin();
		theHold.Put(new LXNodeRestore(this, node));

		tm    = ourTM * Inverse(ntm);
		invtm = Inverse(ourTM);

		pblock->SetValue(lxform_node,0,node);
		theHold.Accept(GetString(IDS_RB_NODEXFORM));

		// We have just changed -- notify our dependents
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);

		if (hParams)
			SetWindowText(GetDlgItem(hParams,IDC_CLUST_NODENAME), node->GetName());

		nodeTab.DisposeTemporary();

	} 
}




BOOL RecursePipeAndMatch(ModContext *smd, Object *obj)
	{
	SClass_ID		sc;
	IDerivedObject* dobj;
	Object *currentObject = obj;

	if ((sc = obj->SuperClassID()) == GEN_DERIVOB_CLASS_ID)
		{
		dobj = (IDerivedObject*)obj;
		while (sc == GEN_DERIVOB_CLASS_ID)
			{
			for (int j = 0; j < dobj->NumModifiers(); j++)
				{
				ModContext *mc = dobj->GetModContext(j);
				if (mc == smd)
					{
					return TRUE;
					}

				}
			dobj = (IDerivedObject*)dobj->GetObjRef();
			currentObject = (Object*) dobj;
			sc = dobj->SuperClassID();
			}
		}

	int bct = currentObject->NumPipeBranches(FALSE);
	if (bct > 0)
		{
		for (int bi = 0; bi < bct; bi++)
			{
			Object* bobj = currentObject->GetPipeBranch(bi,FALSE);
			if (RecursePipeAndMatch(smd, bobj)) return TRUE;
			}

		}

	return FALSE;
}

INode* ClustNodeMod::GetNodeFromModContext(ModContext *smd, int &which)
	{

	int	i;

    ClusterMyEnumProc dep;              
	EnumDependents(&dep);
	for ( i = 0; i < dep.Nodes.Count(); i++)
		{
		INode *node = dep.Nodes[i];
		BOOL found = FALSE;

		if (node)
			{
			Object* obj = node->GetObjectRef();
	
			if ( RecursePipeAndMatch(smd,obj) )
				{
				which = i;
				return node;
				}
			}
		}
	return NULL;
	}




#define TM_CHUNK 0x1000
#define INVTM_CHUNK 0x1010

IOResult ClustNodeMod::SaveLocalData(ISave *isave, LocalModData *pld)
{
ClustNodeModData *p;
IOResult	res;

p = (ClustNodeModData*)pld;

isave->BeginChunk(TM_CHUNK);
res = p->tm.Save(isave);
isave->EndChunk();



isave->BeginChunk(INVTM_CHUNK);
res = p->invtm.Save(isave);
isave->EndChunk();


return IO_OK;
}

IOResult ClustNodeMod::LoadLocalData(ILoad *iload, LocalModData **pld)

{
	IOResult	res;

	ClustNodeModData *p= new ClustNodeModData();
	Matrix3 tm;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case TM_CHUNK:
				tm.Load(iload);
				p->tm = tm;
				break;
			case INVTM_CHUNK:
				tm.Load(iload);
				p->invtm = tm;
				break;

			}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
		}
	*pld = p;
return IO_OK;

}
