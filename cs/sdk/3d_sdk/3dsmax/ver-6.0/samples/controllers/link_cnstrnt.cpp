/**********************************************************************
 *
	FILE: Link_cnstrnt.cpp

	DESCRIPTION: A transform constraint controller to control animated links 
				
	CREATED BY: Rolf Berteig 1/25/97
	Added features and Transformed for paramblock2 compatibility 
			: Ambarish Goswami 7/2000
			MacrRecorder related code: Larry Minton 5/2001

	            

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "ctrl.h"
//#include "units.h"
#include "interpik.h"
#include "istdplug.h"
#include "iparamm2.h"
#include "notify.h"
#include "macrorec.h"



#define LINKCTRL_NAME		GetString(IDS_RB_LINKCTRL)

enum {	link_target_list,		link_key_mode,				link_start_time, 
		link_key_nodes_mode,	link_key_hierarchy_mode, };


class LinkConstTransform; 
class LinkConstTimeChangeCallback : public TimeChangeCallback
{
	public:
		LinkConstTransform* link_controller;
		void TimeChanged(TimeValue t);
};

//class LinkConstValidatorClass : public PBValidator
//{
//	public:
//		LinkConstTransform *mod;
//	private:
//		BOOL Validate(PB2Value &v);
//};

/*
class TimeMapp :  public TimeMap{
	public:
		LinkConstTransform *p;
		TimeValue Mapp(TimeValue time);
};
*/

class LinkConstTransform : public ILinkCtrl  
{
	public:

		Control *tmControl;		// ref 0

		static HWND hWnd;
		static IObjParam *ip;
		static LinkConstTransform *editCont;
		static BOOL valid;
		static ISpinnerControl *iTime;
		static ICustButton *iPickOb, *iDelOb;
		int spDownVal, spUpVal;
		int spDownSel, spUpSel;
		int linkConstraintVersion;

		Tab<INode*> nodes;		// ref 1-n -- Rolf's old variable
		Tab<TimeValue> times;  // Rolf's old variable

		int state;	// LAM - for controlling macroRecorder output line emittor

		LinkConstTransform(BOOL loading=FALSE);


//		LinkConstValidatorClass validator;							
		void RedrawListbox(TimeValue t, int sel = -1);
		int last_selection;
		int frameNoSpinnerUp;
		int previous_value;
		LinkConstTimeChangeCallback linkConstTimeChangeCallback;	
		IParamBlock2* pblock;

		LinkConstTransform(const LinkConstTransform &ctrl);

		~LinkConstTransform();

		// Animatable methods
		void DeleteThis() {delete this;}		
		Class_ID ClassID() {return Class_ID(LINKCTRL_CLASSID);}
		SClass_ID SuperClassID() {return CTRL_MATRIX3_CLASS_ID;}
		void GetClassName(TSTR& s) {s = LINKCTRL_NAME;}
		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev); 
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next); 
		int NumSubs()  {return 1;}
		Animatable* SubAnim(int i) {return tmControl;}
		TSTR SubAnimName(int i) {return GetString(IDS_AG_LINKPARAMS);}				
		int IsKeyable() {return 1;}	// do I need this? Inherited from Link constraint

		// Animatable's Schematic View methods
		SvGraphNodeReference SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags);
		TSTR SvGetRelTip(IGraphObjectManager *gom, IGraphNode *gNodeTarger, int id, IGraphNode *gNodeMaker);
		bool SvCanDetachRel(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker);
		bool SvDetachRel(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker);
		bool SvLinkChild(IGraphObjectManager *gom, IGraphNode *gNodeThis, IGraphNode *gNodeChild);
		bool SvCanConcludeLink(IGraphObjectManager *gom, IGraphNode *gNode, IGraphNode *gNodeChild);

		void CopyKeysFromTime(TimeValue src,TimeValue dst,DWORD flags) {tmControl->CopyKeysFromTime(src,dst,flags);}
		BOOL IsKeyAtTime(TimeValue t,DWORD flags) {return tmControl->IsKeyAtTime(t,flags);}
		BOOL GetNextKeyTime(TimeValue t,DWORD flags,TimeValue &nt) {return tmControl->GetNextKeyTime(t,flags,nt);}
		int GetKeyTimes(Tab<TimeValue> &times,Interval range,DWORD flags) {return tmControl->GetKeyTimes(times,range,flags);}
		int GetKeySelState(BitArray &sel,Interval range,DWORD flags) {return tmControl->GetKeySelState(sel,range,flags);}
		Control *GetPositionController() {return tmControl->GetPositionController();}
		Control *GetRotationController() {return tmControl->GetRotationController();}
		Control *GetScaleController() {return tmControl->GetScaleController();}
		BOOL SetPositionController(Control *c) {return tmControl->SetPositionController(c);}
		BOOL SetRotationController(Control *c) {return tmControl->SetRotationController(c);}
		BOOL SetScaleController(Control *c) {return tmControl->SetScaleController(c);}
//		void MapKeys(TimeMap *map,DWORD flags);
		int SubNumToRefNum(int subNum) {if (subNum==0) return LINKCTRL_PBLOCK_REF; else return -1;} 

		int RemapRefOnLoad(int iref);

		// Reference methods
		int NumRefs() {return 3 + nodes.Count();} 
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
		RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage);
		
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		RefTargetHandle Clone(RemapDir &remap = NoRemap());
		BOOL AssignController(Animatable *control,int subAnim);
		void RescaleWorldUnits(float f) {} // do I need this? Inherited from Link constraint

		// Control methods
		void Copy(Control *from);
		BOOL IsLeaf() {return FALSE;}
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void SetValue(TimeValue t, void *val, int commit=1, GetSetMethod method=CTRL_ABSOLUTE);
		void CommitValue(TimeValue t) { }
		void RestoreValue(TimeValue t) { }
		BOOL InheritsParentTransform() {return FALSE;}
		BOOL CanInstanceController() {return FALSE;}
//		void DeleteKeysFrom(INode *toDeleteNode, TimeValue toDeleteTime);

		// From ILinkCtrl
		int GetNumTargets();
		TimeValue GetLinkTime(int i) {
			return pblock->GetTimeValue(link_start_time, 0, i);
		}
		void SetLinkTime(int i, TimeValue linkTime) {SetTime(linkTime,i);}
		void LinkTimeChanged() {
			SortNodes(-1);
		}

		static void NotifyPreDeleteNode(void* param, NotifyInfo*);

		
		// From where?
		int SetProperty(ULONG id, void *data);
		void *GetProperty(ULONG id);
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock


		// Local methods
		Matrix3 GetParentTM(TimeValue t,Interval *valid=NULL);
		Matrix3 CompTM(TimeValue t, int i);
		void AddNewLink(INode *node,TimeValue t);
		BOOL DeleteTarget(int i);
		void SetTime(TimeValue linkTime, int i);
		void SortNodes(int sel);

//		void SetupDialog(HWND hWnd);
//		void DestroyDialog();
//		void SetupList(int sel=-1);
//		void ListSelChanged();
//		void Invalidate();
		int Getlink_key_mode();
		int Getlink_key_nodes_mode();
		int Getlink_key_hierarchy_mode();
		BOOL Setlink_key_mode(int keymode);
		BOOL Setlink_key_nodes_mode(int keynodes_mode);
		BOOL Setlink_key_hierarchy_mode(int keyhier_mode);


		void ChangeKeys(int sell);
		int GetFrameNumber(int targetNumber);
		BOOL SetFrameNumber(int targetNumber, int frameNumber);
		BOOL AddTarget(INode *target, int frameNo = GetCOREInterface()->GetTime()/GetTicksPerFrame());
		int InternalAddWorldMethod(int frameNo = GetCOREInterface()->GetTime()/GetTicksPerFrame());
		INode* GetNode(int targetNumber);


//		Function Publishing method (Mixin Interface)
//		Added by Ambarish Goswami (7/20/2000)
//		Adapted from Adam Felt (5-16-00)

		BaseInterface* GetInterface(Interface_ID id) 
		{ 
			if (id == LINK_CONSTRAINT_INTERFACE) 
				return (LinkConstTransform*)this; 
			else 
				return FPMixinInterface::GetInterface(id);
		}

	};


void LinkConstTimeChangeCallback :: TimeChanged(TimeValue t){
	int selection = SendDlgItemMessage(link_controller->hWnd, IDC_LINK_LIST, LB_GETCURSEL, 0, 0);
	ISpinnerControl* spin = GetISpinner(GetDlgItem(link_controller->hWnd, IDC_LINK_TIME_SPINNER));
	spin = GetISpinner(GetDlgItem(link_controller->hWnd, IDC_LINK_TIME_SPINNER));

	Control *cont = link_controller->pblock->GetController(link_start_time, selection); 
	// until it is animated, paramblock doesn't have a controller assigned in it yet.
	if (spin && (cont !=NULL)) {
		if (cont->IsKeyAtTime(t,KEYAT_POSITION) &&
			cont->IsKeyAtTime(t,KEYAT_ROTATION) &&
			cont->IsKeyAtTime(t,KEYAT_SCALE)) {
			spin->SetKeyBrackets(TRUE);
		} 
		else {
			spin->SetKeyBrackets(FALSE);
		}
	}
	link_controller->RedrawListbox(t);
}


	IObjParam			*LinkConstTransform::ip				= NULL;
	LinkConstTransform	*LinkConstTransform::editCont		= NULL;


	HWND				LinkConstTransform::hWnd			= NULL;
	ISpinnerControl		*LinkConstTransform::iTime			= NULL;
	ICustButton			*LinkConstTransform::iPickOb		= NULL;
	ICustButton			*LinkConstTransform::iDelOb			= NULL;
	//PickLinkNode		*LinkConstTransform::pickLinkNode= NULL;
	BOOL				LinkConstTransform::valid			= FALSE;

//********************************************************
// LINK CONSTRAINT
//********************************************************
static Class_ID linkConstControlClassID(LINKCTRL_CLASSID); 
class LinkConstClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new LinkConstTransform(loading); }
	const TCHAR *	ClassName() { return LINKCTRL_NAME; }
	SClass_ID		SuperClassID() { return CTRL_MATRIX3_CLASS_ID; }
	Class_ID		ClassID() { return linkConstControlClassID; }
	const TCHAR* 	Category() { return _T("");  }
	const TCHAR*	InternalName() { return _T("Link"); }// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
	};

static LinkConstClassDesc linkCD;
ClassDesc* GetLinkCtrlDesc() {return &linkCD;}

//--- CustMod dlg proc ---------DOWN DOWN DOWN----from Modifiers/clstmode.cpp---------------


class PickLinkNode : public PickModeCallback,
		public PickNodeCallback {

	public:			

		LinkConstTransform *p;
		PickLinkNode() {p = NULL;}
//		PickLinkNode(LinkConstTransform *c) {p = c;}
		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);		
		BOOL Pick(IObjParam *ip,ViewExp *vpt);		
		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);		
		BOOL Filter(INode *node);
		PickNodeCallback *GetFilter() {return this;}
		BOOL RightClick(IObjParam *ip,ViewExp *vpt) {return TRUE;}
	};

static PickLinkNode thePickMode;

BOOL PickLinkNode::Filter(INode *node){
	if (node) {
		return (node->TestForLoop(FOREVER, p)==REF_SUCCEED);
		}
	return FALSE;
}

BOOL PickLinkNode::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
	{	
	INode *node = p->ip->PickNode(hWnd,m);	
	return node?TRUE:FALSE;
	}

BOOL PickLinkNode::Pick(IObjParam *ip,ViewExp *vpt)
	{
	INode *node = vpt->GetClosestHit();
	if (node) {
		if (node->TestForLoop(FOREVER, p)==REF_SUCCEED) {
			p->AddTarget(node);
			p->ip->RedrawViews(p->ip->GetTime());
		}
	}
	return FALSE;
	}

void PickLinkNode::EnterMode(IObjParam *ip)
	{
	ICustButton *iBut = GetICustButton(GetDlgItem(p->hWnd,IDC_LINK_TARG_PICKNODE));
	if (iBut) iBut->SetCheck(TRUE);
	ReleaseICustButton(iBut);
	GetCOREInterface()->PushPrompt("Select Link Target Object");
	}

void PickLinkNode::ExitMode(IObjParam *ip)
	{
	ICustButton *iBut = GetICustButton(GetDlgItem(p->hWnd,IDC_LINK_TARG_PICKNODE));
	if (iBut) iBut->SetCheck(FALSE);
	ReleaseICustButton(iBut);
	GetCOREInterface()->PopPrompt();
	}


//--- CustMod dlg proc -----------UP UP UP -------------------


//Function Publishing descriptor for Mixin interface
//Added by Ambarish Goswami (5/18/2000)
//*****************************************************


  static FPInterfaceDesc link_constraint_interface(
    LINK_CONSTRAINT_INTERFACE, _T("constraints"), 0, &linkCD, 0,
		ILinkCtrl::get_node, _T("getNode"), 0, TYPE_INODE, 0, 1,
			_T("nodeNumber"),	0,		TYPE_INDEX,
		ILinkCtrl::get_frame_no, _T("getFrameNo"), 0, TYPE_INT, 0, 1,
			_T("targetNumber"),	0,		TYPE_INDEX,
		ILinkCtrl::set_frame_no, _T("setFrameNo"), 0, TYPE_BOOL, 0, 2,
			_T("targetNumber"),	0,	TYPE_INDEX,
			_T("frameNo"),	0,		TYPE_INT,
		ILinkCtrl::get_num_targets, _T("getNumTargets"), 0, TYPE_INT, 0,0,
		ILinkCtrl::add_target, _T("addTarget"), 0, TYPE_BOOL, 0, 2,
			_T("target"),	0,		TYPE_INODE,
			_T("frameNo"),	0,		TYPE_INT,
		ILinkCtrl::delete_target, _T("DeleteTarget"), 0,	TYPE_BOOL,	0, 1,
			_T("targetNumber"), 0, TYPE_INDEX,
		ILinkCtrl::add_world, _T("addWorld"), 0, TYPE_INT, 0, 1,
			_T("frameNo"),	0,		TYPE_INT, f_keyArgDefault, -99999,
			
		end
		);



//  Get Descriptor method for Mixin Interface
//  *****************************************
FPInterfaceDesc* ILinkCtrl::GetDesc()
{
     return &link_constraint_interface;
}



//*********************************************
// End of Function Publishing Code



enum { link_const_params, };

// parameter setter callback, reflect any ParamBlock-mediated param setting in instance data members.

class LinkConstPBAccessor : public PBAccessor
{ 
	public:

		void TabChanged(tab_changes changeCode, Tab<PB2Value>* tab, 
                  ReferenceMaker* owner, ParamID id, int tabIndex, int count) 
		{ 

		LinkConstTransform* p = (LinkConstTransform*)owner;
		if (((id == link_target_list) || (id == link_start_time)) &&
			(p->pblock->Count(link_target_list) == p->pblock->Count(link_start_time))){
			// make sure that the lists are of same size

// the tab_ref_deleted case is removed in favor of implementing a RegisterNotification
// so that the source object doesn't jum when the target is viewport-deleted.
// Ref: bug# 286503
// Ambarish : 4/4/2001
//			if (changeCode == tab_ref_deleted){

//				p->DeleteTarget(tabIndex);
//			}

				switch(changeCode)
				{
					case tab_append:
					{
						p->last_selection = p->pblock->Count(link_target_list) -1;
					}
					break;

					case tab_delete:
					{
						p->last_selection = p->last_selection - 1;
					}
					break;
					case tab_setcount:
					{
						if (p->last_selection > p->pblock->Count(link_target_list) -1)
						p->last_selection = p->pblock->Count(link_target_list) -1;
					}
					break;
				}

				if (p->pblock->Count(link_target_list) > 0 && p->last_selection < 0) 
					p->last_selection = 0;

				p->RedrawListbox(GetCOREInterface()->GetTime(), p->last_selection);
//			}
	
				if (p->last_selection < 0 || p->pblock->Count(link_target_list) < 1){ 
				// nothing is selected OR there are no targets
					// all buttons - target_delete, weight_edit, weight_spinner -- are disabled
					EnableWindow(GetDlgItem(p->hWnd, IDC_LINK_TIME_EDIT), FALSE);
					EnableWindow(GetDlgItem(p->hWnd, IDC_LINK_TIME_SPINNER), FALSE);
					ICustButton *iPickOb1;
					iPickOb1= GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_LINK_TARG));
					if (iPickOb1 != NULL){
						iPickOb1->Enable(FALSE);		
					}
					ReleaseICustButton(iPickOb1);
				}
				else if (p->last_selection >= 0) {	
				// there is a valid selection 
					// automatically means there is AT LEAST one target
					if (p->pblock->Count(link_target_list) >= 1){ // there is only one target
					// the "delete" button should be enabled
						ICustButton *iPickOb1;
						iPickOb1 = GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_LINK_TARG));
						if (iPickOb1 != NULL){
							iPickOb1->Enable(TRUE);		
						}
						ReleaseICustButton(iPickOb1);
						// the rest are disabled
						EnableWindow(GetDlgItem(p->hWnd, IDC_LINK_TIME_EDIT), TRUE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_LINK_TIME_SPINNER), TRUE);
					}
				}
			
		}
		}


	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		LinkConstTransform* p = (LinkConstTransform*)owner;
		switch (id){
			case link_target_list:{
				IParamMap2* pmap = p->pblock->GetMap();
				break;
			}

			case link_key_mode:
				switch (v.i){
					case 0: 
						EnableWindow(GetDlgItem(p->hWnd, IDC_KEY_NODES_CHILD), FALSE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_KEY_NODES_PARENTS), FALSE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_KEY_HIERARCHY_CHILD), FALSE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_KEY_HIERARCHY_PARENTS), FALSE);
					break;

					case 1: 
						EnableWindow(GetDlgItem(p->hWnd, IDC_KEY_NODES_CHILD), TRUE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_KEY_NODES_PARENTS), TRUE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_KEY_HIERARCHY_CHILD), FALSE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_KEY_HIERARCHY_PARENTS), FALSE);
						if (p->Getlink_key_nodes_mode() == -1 || p->Getlink_key_nodes_mode() == 0) 
							p->Setlink_key_nodes_mode(0);
						else
							p->Setlink_key_nodes_mode(1);
						
					break;
					case 2: 
						EnableWindow(GetDlgItem(p->hWnd, IDC_KEY_NODES_CHILD), FALSE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_KEY_NODES_PARENTS), FALSE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_KEY_HIERARCHY_CHILD), TRUE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_KEY_HIERARCHY_PARENTS), TRUE);
						if (p->Getlink_key_hierarchy_mode() == -1 || p->Getlink_key_hierarchy_mode() == 0) 
							p->Setlink_key_hierarchy_mode(0);
						else
							p->Setlink_key_hierarchy_mode(1);
					break;
				}
			break;
			UpdateWindow(p->hWnd);


		}
	}
};


static LinkConstPBAccessor link_const_accessor;

class LinkConstDlgProc : public ParamMap2UserDlgProc 
{
	public:
		void UpdateLinkName(LinkConstTransform* p)
		{
			IParamMap2* pmap = p->pblock->GetMap();
		}
						

// In below, I'd need to take control off of the ParamBlock2 and rewrite the listbox 
// myself. This is what the "RedrawListbox" code does. John Wainwright wrote the initial 
// code for the following three switch messages.

		BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			LinkConstTransform* p = (LinkConstTransform*)map->GetParamBlock()->GetOwner();	
			UpdateLinkName(p);
			p->hWnd = hWnd;
			int ct = p->pblock->Count(link_target_list);

			switch (msg) 
			{
				case WM_INITDIALOG:
				{
					int selection = p->last_selection;

					if (selection < 0 && p->pblock->Count(link_target_list) > 0) selection = 0;

					ICustButton *iBut = GetICustButton(GetDlgItem(hWnd,IDC_LINK_TARG_PICKNODE));
					iBut->SetType(CBT_CHECK);
					iBut->SetHighlightColor(GREEN_WASH);
					ReleaseICustButton(iBut);

					ISpinnerControl* spin = SetupIntSpinner(hWnd, IDC_LINK_TIME_SPINNER, IDC_LINK_TIME_EDIT, -99999999, 99999999, 0);
					spin = GetISpinner(GetDlgItem(hWnd, IDC_LINK_TIME_SPINNER));
					

					switch (p->Getlink_key_mode()){
						case 0:  // no key
						EnableWindow(GetDlgItem(p->hWnd, IDC_KEY_NODES_CHILD), FALSE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_KEY_NODES_PARENTS), FALSE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_KEY_HIERARCHY_CHILD), FALSE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_KEY_HIERARCHY_PARENTS), FALSE);
						break;

						case 1:  // key nodes
						EnableWindow(GetDlgItem(p->hWnd, IDC_KEY_NODES_CHILD), TRUE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_KEY_NODES_PARENTS), TRUE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_KEY_HIERARCHY_CHILD), FALSE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_KEY_HIERARCHY_PARENTS), FALSE);
						if (p->Getlink_key_nodes_mode() == -1 || p->Getlink_key_nodes_mode() == 0) 
							p->Setlink_key_nodes_mode(0);
						else
							p->Setlink_key_nodes_mode(1);
							
						break;
						case 2: //key entire hierarchy
						EnableWindow(GetDlgItem(p->hWnd, IDC_KEY_NODES_CHILD), FALSE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_KEY_NODES_PARENTS), FALSE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_KEY_HIERARCHY_CHILD), TRUE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_KEY_HIERARCHY_PARENTS), TRUE);
						if (p->Getlink_key_hierarchy_mode() == -1 || p->Getlink_key_hierarchy_mode() == 0) 
							p->Setlink_key_hierarchy_mode(0);
						else
							p->Setlink_key_hierarchy_mode(1);
						break;
					}

					UpdateWindow(p->hWnd);
					
					if (selection < 0 || p->pblock->Count(link_target_list) < 1){ 
					// nothing is selected OR there are no targets
						// all buttons - target_delete, weight_edit, weight_spinner -- are disabled
						EnableWindow(GetDlgItem(p->hWnd, IDC_LINK_TIME_EDIT), FALSE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_LINK_TIME_SPINNER), FALSE);
						ICustButton *iPickOb1;
						iPickOb1	= GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_LINK_TARG));
						if (iPickOb1 != NULL){
							iPickOb1->Enable(FALSE);		
						}
						spin->SetValue(0, FALSE);
						ReleaseICustButton(iPickOb1);
					}

					else if (selection >= 0) {	
					// there is a valid selection 
					// automatically means there is AT LEAST one target
						if (p->pblock->Count(link_target_list) >= 1){// there is only one target
							// the "delete" button should be enabled
							ICustButton *iPickOb1;
							iPickOb1 = GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_LINK_TARG));
							if (iPickOb1 != NULL){
								iPickOb1->Enable(TRUE);		
							}
							ReleaseICustButton(iPickOb1);
							// the rest are disabled
							EnableWindow(GetDlgItem(p->hWnd, IDC_LINK_TIME_EDIT), TRUE);
							EnableWindow(GetDlgItem(p->hWnd, IDC_LINK_TIME_SPINNER), TRUE);
						}
					}
					ReleaseISpinner(spin);
					p->RedrawListbox(GetCOREInterface()->GetTime(), selection);
					p->state = -1;
					return TRUE;	
				}
				break;

				case WM_COMMAND:
				{
					int selection;

					if (HIWORD(wParam) == LBN_SELCHANGE  && LOWORD(wParam) == IDC_LINK_LIST){
						
						selection = SendDlgItemMessage(p->hWnd, IDC_LINK_LIST, LB_GETCURSEL, 0, 0);
						p->last_selection = selection;

						if (selection >= 0){

							if (p->pblock->Count(link_target_list) >= 1){ // there is only one target
								// the "delete" button should be enabled
								EnableWindow(GetDlgItem(p->hWnd, IDC_LINK_TIME_EDIT), TRUE);
								EnableWindow(GetDlgItem(p->hWnd, IDC_LINK_TIME_SPINNER), TRUE);
								ICustButton *iPickOb1;
								iPickOb1 = GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_LINK_TARG));
								if (iPickOb1 != NULL){
									iPickOb1->Enable(TRUE);		
								}
								ReleaseICustButton(iPickOb1);

								ISpinnerControl* spin = GetISpinner(GetDlgItem(hWnd, IDC_LINK_TIME_SPINNER));
								int linkTime = (int) map->GetParamBlock()->GetTimeValue(link_start_time, t, selection);
								spin->SetValue((int)linkTime/GetTicksPerFrame(), FALSE);
								p->previous_value = linkTime;

								ReleaseISpinner(spin);

							}
							p->RedrawListbox(GetCOREInterface()->GetTime(), selection);							
						}
						
						else if (selection < 0 || p->pblock->Count(link_target_list) < 1){ 
						// nothing is selected OR there are no targets
							// all buttons - target_delete, weight_edit, weight_spinner -- are disabled
							EnableWindow(GetDlgItem(p->hWnd, IDC_LINK_TIME_EDIT), FALSE);
							EnableWindow(GetDlgItem(p->hWnd, IDC_LINK_TIME_SPINNER), FALSE);
							ICustButton *iPickOb1;
							iPickOb1	= GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_LINK_TARG));
							if (iPickOb1 != NULL){
								iPickOb1->Enable(FALSE);	
							}
							ReleaseICustButton(iPickOb1);
						}

					}
					else if (LOWORD(wParam) == IDC_LINK_TO_WORLD){
						INode *nullNode = NULL;
						p->AddTarget(nullNode);
						p->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE); // so that the change is updated
						p->RedrawListbox(GetCOREInterface()->GetTime());

					}
					else if (LOWORD(wParam) == IDC_LINK_TARG_PICKNODE){
						thePickMode.p = p;			
						p->ip->SetPickMode(&thePickMode);
					}
				
					else if (LOWORD(wParam) == IDC_REMOVE_LINK_TARG){
						selection = SendDlgItemMessage(p->hWnd, IDC_LINK_LIST, LB_GETCURSEL, 0, 0);
						p->last_selection = selection;
						if (selection >= 0){
							theHold.Begin();
							p->DeleteTarget(selection);
							theHold.Accept(GetString(IDS_AG_LINK_LIST));
						}
					}
				}
				break;

				case CC_SPINNER_CHANGE:
				{
					int selection = SendDlgItemMessage(p->hWnd, IDC_LINK_LIST, LB_GETCURSEL, 0, 0);
				

					if (LOWORD(wParam) == IDC_LINK_TIME_SPINNER)
					{
						if (selection >= 0)
						{
							int value = ((ISpinnerControl *)lParam)->GetIVal(); // value is in frameNo.
							macroRec->Disable();
							map->GetParamBlock()->SetValue(link_start_time, t, value * GetTicksPerFrame(), selection);
							macroRec->Enable();
							if (p->state != 3+selection)
							{	macroRec->EmitScript();
								p->state = 3+selection;
							}
							macroRec->OperandSequence(3, mr_prop, _T("setFrameNo"),mr_reftarg, p, mr_int, selection+1, mr_int, value);
							ISpinnerControl* spin = GetISpinner(GetDlgItem(hWnd, IDC_LINK_TIME_SPINNER));							
							p->RedrawListbox(p->ip->GetTime());
						}
						
						else{
							ISpinnerControl* spin = GetISpinner(GetDlgItem(hWnd, IDC_LINK_TIME_SPINNER));
							spin->SetValue(0, FALSE);
						}
					
					}
				}
				break;

				case CC_SPINNER_BUTTONDOWN:
				{
					int selection = SendDlgItemMessage(p->hWnd, IDC_LINK_LIST, LB_GETCURSEL, 0, 0);
					if (LOWORD(wParam) == IDC_LINK_TIME_SPINNER){
						if (selection >= 0 ){
//							p->spDownVal = p->pblock->GetInt(link_start_time, GetCOREInterface()->GetTime(), selection);
							p->spDownVal = ((ISpinnerControl *)lParam)->GetIVal(); // frameNo
							p->spDownSel = selection;
						}
					}

				}
				break;

				
				case CC_SPINNER_BUTTONUP:
				{
					int selection = SendDlgItemMessage(p->hWnd, IDC_LINK_LIST, LB_GETCURSEL, 0, 0);
					if (LOWORD(wParam) == IDC_LINK_TIME_SPINNER){
						if (selection >= 0 )
						{
							int value = ((ISpinnerControl *)lParam)->GetIVal(); // value is in frameNo
							p->spUpVal = value;
							if (value > p->previous_value){
								p->frameNoSpinnerUp = 1;
							}
							else {
								p->frameNoSpinnerUp = 0;
							}
							p->SetTime(value * GetTicksPerFrame(), selection);
							p->spUpSel = selection;
							p->ChangeKeys(value);
						}
						p->RedrawListbox(p->ip->GetTime());
					}
				}
				break;
			}
			return FALSE;
		}

		void SetParamBlock(IParamBlock2* pb) 
		{ 
			UpdateLinkName((LinkConstTransform*)pb->GetOwner());
		}
		void DeleteThis() { }
};

	void LinkConstTransform::RedrawListbox(TimeValue t,  int sel)
		{
			if (hWnd == NULL) return;
			if (!ip || editCont != this) return;
			if(!linkCD.NumParamMaps()) return;
			int selection = SendDlgItemMessage(hWnd, IDC_LINK_LIST, LB_GETCURSEL, 0, 0);
			SendDlgItemMessage(hWnd, IDC_LINK_LIST, LB_RESETCONTENT, 0, 0);
			int ts = 64;  
			SendDlgItemMessage(hWnd, IDC_LINK_LIST, LB_SETTABSTOPS, 1, (LPARAM)&ts);
			int ct = pblock->Count(link_target_list);

//			if (selection == LB_ERR && last_selection == LB_ERR && ct > 0) 
//				selection = ct-1;
//			else if (selection == LB_ERR && last_selection != LB_ERR && ct > 0) 
//				selection = last_selection;

//			SortNodes(selection);

//			SendDlgItemMessage(hWnd, IDC_LINK_LIST, LB_SETCURSEL, selection, 0);
			for (int i = 0; i < ct; i++){

				TimeValue linkTime = pblock->GetTimeValue(link_start_time, t, i);

				TSTR str;
				if (pblock->GetINode(link_target_list, t, i) == NULL){
					str.printf( _T("%-s\t%-d"), // NOTE: for tab "\t" to use, check "use tabstops" in the resource listbox properties
					"World", linkTime / GetTicksPerFrame());
				}
				else{
					str.printf( _T("%-s\t%-d"), // NOTE: for tab "\t" to use, check "use tabstops" in the resource listbox properties
					pblock->GetINode(link_target_list, t, i)->GetName(), linkTime / GetTicksPerFrame());
				}

				SendDlgItemMessage(hWnd, IDC_LINK_LIST, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR) str.data());			
			}



			if (ct > 0){
				if (sel >= 0){
					SendDlgItemMessage(hWnd, IDC_LINK_LIST, LB_SETCURSEL, sel, 0);
				}
				else{
					if (selection >= 0){
						SendDlgItemMessage(hWnd, IDC_LINK_LIST, LB_SETCURSEL, selection, 0);
						last_selection = selection;
					}
					else if (ct == 1) {
						SendDlgItemMessage(hWnd, IDC_LINK_LIST, LB_SETCURSEL, 0, 0);
						last_selection = 0;
					}
				}


				EnableWindow(GetDlgItem(hWnd, IDC_LINK_TIME_EDIT), TRUE);
				EnableWindow(GetDlgItem(hWnd, IDC_LINK_TIME_SPINNER), TRUE);
				ICustButton *iPickOb1;
				iPickOb1	= GetICustButton(GetDlgItem(hWnd, IDC_REMOVE_LINK_TARG));
				if (iPickOb1 != NULL){
					iPickOb1->Enable(TRUE);	
				}
				ReleaseICustButton(iPickOb1);
				
			
				ISpinnerControl* spin = GetISpinner(GetDlgItem(hWnd, IDC_LINK_TIME_SPINNER));
				if (last_selection >= 0 && spin != NULL){
					int linkTime = (int)pblock->GetTimeValue(link_start_time, t, last_selection);
					spin->SetValue((int)linkTime/GetTicksPerFrame(), FALSE);
				}
			}
			else{

				EnableWindow(GetDlgItem(hWnd, IDC_LINK_TIME_EDIT), FALSE);
				EnableWindow(GetDlgItem(hWnd, IDC_LINK_TIME_SPINNER), FALSE);
				ICustButton *iPickOb1;
				iPickOb1	= GetICustButton(GetDlgItem(hWnd, IDC_REMOVE_LINK_TARG));
				if (iPickOb1 != NULL){
					iPickOb1->Enable(FALSE);	
				}
				ReleaseICustButton(iPickOb1);
			}
			HWND hListBox = GetDlgItem(hWnd, IDC_LINK_LIST);
			int extent = computeHorizontalExtent(hListBox, TRUE, 1, &ts);
			SendMessage(hListBox, LB_SETHORIZONTALEXTENT, extent, 0);
			UpdateWindow(hWnd);

		}



//static HWND hParamGizmos;

static LinkConstDlgProc linkConstDlgProc;

static ParamBlockDesc2 link_const_paramblk (link_const_params, _T("LinkConsParameters"),  0, &linkCD, P_AUTO_CONSTRUCT + P_AUTO_UI, LINKCTRL_PBLOCK_REF, 
	//rollout
	IDD_LINK_PARAMS, IDS_AG_LINKPARAMS, BEGIN_EDIT_MOTION, 0, &linkConstDlgProc,
	// params

	link_target_list,  _T(""),			TYPE_INODE_TAB, 0,	P_VARIABLE_SIZE,	IDS_AG_LINK_LIST,
		p_accessor,		&link_const_accessor,
		end,

	link_key_mode, 		_T("key_mode"),	TYPE_INT, 0, IDS_AG_LINK_KEY_MODE,
		p_default, 		0, 
		p_range, 		0, 2, 
		p_ui, 			TYPE_RADIO, 3, IDC_NO_KEY, IDC_KEY_NODES, IDC_KEY_HIERARCHY, 
		p_accessor,		&link_const_accessor,
		end,

	link_start_time, _T(""), 	TYPE_TIMEVALUE_TAB, 0,  P_VARIABLE_SIZE, IDS_AG_LINK_START_FRAME_LIST, 
		p_accessor,		&link_const_accessor,	
		end, 

	link_key_nodes_mode, _T("key_nodes_mode"),	TYPE_INT, 0, IDS_AG_LINK_KEY_NODES_MODE,
		p_range, 		0, 1, 
		p_ui, 			TYPE_RADIO, 2, IDC_KEY_NODES_CHILD, IDC_KEY_NODES_PARENTS, 
		p_accessor,		&link_const_accessor,
		end,

	link_key_hierarchy_mode, 	_T("key_hierarchy_mode"),	TYPE_INT, 0, IDS_AG_LINK_KEY_HIERARCHY_MODE,
		p_range, 		0, 1, 
		p_ui, 			TYPE_RADIO, 2, IDC_KEY_HIERARCHY_CHILD, IDC_KEY_HIERARCHY_PARENTS, 
		p_accessor,		&link_const_accessor,
		end,

	end
	);


LinkConstTransform::LinkConstTransform(BOOL loading)
	{

	linkConstraintVersion = 400; // after 10/27
	linkCD.MakeAutoParamBlocks(this);
	pblock->CallSets();
	last_selection =  -1;
	frameNoSpinnerUp = 0;
	previous_value = 2000000;
	hWnd = NULL;

	RegisterNotification(NotifyPreDeleteNode, this, NOTIFY_SCENE_PRE_DELETED_NODE);
	tmControl = NULL;
	if (!loading) {
		Control *cont;
		ClassDesc *desc = GetDefaultController(CTRL_MATRIX3_CLASS_ID);
		if (desc && desc->ClassID()==ClassID()) {
			cont = (Control*)CreateInstance(CTRL_MATRIX3_CLASS_ID, Class_ID(PRS_CONTROL_CLASS_ID,0));			
		} else {
			cont = NewDefaultMatrix3Controller();
			}
		ReplaceReference(0,cont);
		}
	// make the paramblock

	}

LinkConstTransform::~LinkConstTransform()
	{
	UnRegisterNotification(NotifyPreDeleteNode, this,	NOTIFY_SCENE_PRE_DELETED_NODE);
	DeleteAllRefsFromMe();
	}


void LinkConstTransform::NotifyPreDeleteNode(void* parm, NotifyInfo* arg){
	LinkConstTransform* linkC = (LinkConstTransform*)parm;
	if(linkC == NULL) return;
	Tab<int> deleted_nodes;
	if((arg != NULL) && (linkC->pblock != NULL)){
		INode* deleted_node = (INode*)arg->callParam;
		int ct = linkC->pblock->Count(link_target_list);
		int deleted_nodes_number = 0;
		for(int i=0; i <ct; i++ ){
			INode* someNode;
			linkC->pblock->GetValue(link_target_list, GetCOREInterface()->GetTime(), someNode, FOREVER, i);
			if (deleted_node == someNode){
				deleted_nodes.SetCount(deleted_nodes_number+1);
				deleted_nodes[deleted_nodes_number] = i;
				deleted_nodes_number++;
			}
		}
	}

	for (int i=0; i <deleted_nodes.Count(); i++ ){

		deleted_nodes[i] = deleted_nodes[i]-i;
	}

	for(int j=0; j <deleted_nodes.Count(); j++ ){

		linkC->DeleteTarget(deleted_nodes[j]);
	}

}


RefTargetHandle LinkConstTransform::Clone(RemapDir& remap)
	{
	LinkConstTransform *p = new LinkConstTransform(TRUE);
	p->ReplaceReference(0,remap.CloneRef(tmControl));
    p->ReplaceReference(LINKCTRL_PBLOCK_REF, pblock->Clone(remap));
	BaseClone(this, p, remap);
	return p;
	}

BOOL LinkConstTransform::AssignController(Animatable *control,int subAnim)
	{
	if (control->ClassID()==ClassID()) return FALSE;
	ReplaceReference(0,(ReferenceTarget*)control);
	NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	return TRUE;
	}


void LinkConstTransform::Copy(Control *from){
	if (from->ClassID()==ClassID()) {
		LinkConstTransform *lc = (LinkConstTransform*)from;
		ReplaceReference(0,(ReferenceTarget*)lc->tmControl->Clone());
	} 
	else {
		ReplaceReference(0,(ReferenceTarget*)from->Clone());
	}
	NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	}



void LinkConstTransform::GetValue( TimeValue t, void *val, Interval &valid, GetSetMethod method){

	Matrix3 *ptm = (Matrix3*)val;
	*ptm = GetParentTM(t,&valid);
	tmControl->GetValue(t, val, valid, method);
	
}

void LinkConstTransform::SetValue(TimeValue t, void *val, int commit, GetSetMethod method){
	
	SetXFormPacket *pckt = (SetXFormPacket*)val;
	pckt->tmParent = GetParentTM(t);
	tmControl->SetValue(t, val, commit, method);

}

Matrix3 LinkConstTransform::CompTM(TimeValue t, int i){
	Matrix3 rtm(1);
	INode *sNode;
	
	TimeValue sTime = pblock->GetTimeValue(link_start_time, t, i);
	pblock->GetValue(link_target_list, t, sNode, FOREVER, i); 

	if (i) {

		// Evaluate the tm of the previous node the instant of the switch
		Matrix3 pptm = CompTM(sTime, i-1);
		 
		// Evaluate the tm of this node the instant of the switch

		Matrix3 ptm;
		if (sNode == NULL){
			ptm.IdentityMatrix();  // check for "Link To World"
		}
		else {
			ptm = sNode->GetNodeTM(sTime);
		}
		
		rtm = pptm*Inverse(ptm);
	}

	Matrix3 ttm;
	if (sNode == NULL){
		ttm.IdentityMatrix();   // check for "Link To World"
	}
	else {
		ttm = sNode->GetNodeTM(t);
	}
	return rtm*ttm;
}


Matrix3 LinkConstTransform::GetParentTM(TimeValue t, Interval *valid)
	{
	int ct = pblock->Count(link_target_list);
	int ctf = pblock->Count(link_start_time);
	if (ct != ctf) return Matrix3(1);
	if (ct < 1) return Matrix3(1);

	for (int i = 0; i < ct; i++) { 

		TimeValue sTime = pblock->GetTimeValue(link_start_time, t, i);

		if (sTime > t) {
			if (i) i--;
			break;
		}
	}
	if (i > ct-1) i = ct-1;
	INode *sNode;

	TimeValue sTime = pblock->GetTimeValue(link_start_time, t, i);
	pblock->GetValue(link_target_list, t, sNode, FOREVER, i);

	// Compute a matrix that adjusts the transitions between links so
	// as to maintain continuity
	Matrix3 rtm(1);
	if (i) {

		Matrix3 pptm = CompTM(sTime,i-1);
			
		// Evaluate the tm of this node the instant of the switch
		Matrix3 ptm;
		if (sNode == NULL){
			ptm.IdentityMatrix();
		}
		else {
			ptm = sNode->GetNodeTM(sTime, valid);
		}
		
		rtm = pptm*Inverse(ptm);
	}
	
	// Validity is limited to the duration of this link
	if (valid) {
		Interval iv = FOREVER;
		if (i) iv.SetStart(sTime);
		if (i< pblock->Count(link_start_time)-1) {
			TimeValue sTimeP1 = pblock->GetTimeValue(link_start_time, t, i+1);
			iv.SetEnd(sTimeP1);
		}
		*valid &= iv;
	}

	Matrix3 ttm;
	if (sNode == NULL){
		ttm.IdentityMatrix();
	}
	else {
		ttm = sNode->GetNodeTM(t, valid);
	}
	return rtm * ttm;
	}


void LinkConstTransform::AddNewLink(INode *node,TimeValue t)
	{
	
	theHold.Begin();
	int newNodeFlag = 1;
	INode *nullNode = NULL;
	int ct = pblock->Count(link_target_list);
//	int ctf = pblock->Count(link_start_time);
	if (ct == 0) {		
		Matrix3 tm(1), ntm, itm(1);
		if (node != NULL){
			ntm = node->GetNodeTM(t);
		}
		else{
			ntm.IdentityMatrix();
		}
		tmControl->GetValue(t,&tm,FOREVER,CTRL_RELATIVE);
		tmControl->ChangeParents(t,itm,ntm,tm);
	}

	for (int i = 0; i < ct; i++) {
		TimeValue sTime = pblock->GetTimeValue(link_start_time, 0, i);
		if (t != sTime){
			if (sTime > t) break;
		}
		else{
			newNodeFlag = 0;
			break;
		}
	}

	macroRec->Disable();
	if (newNodeFlag){ // we are adding a node at a new frame#
		pblock->Insert(link_target_list, i, 1, &node);
		pblock->Insert(link_start_time, i, 1, &t);

	}
	else { // we are replacing a node with a new one at the same frame#
		pblock->SetValue(link_target_list, t, node, i);
	}
	SendDlgItemMessage(hWnd, IDC_LINK_LIST, LB_SETCURSEL, i, 0);

	SortNodes(i);
	macroRec->Enable();
	state = 1;
	macroRec->EmitScript();
	if (node)
		macroRec->OperandSequence(3, mr_prop, _T("addTarget"),mr_reftarg, this, mr_reftarg, node, mr_int, int(t/GetTicksPerFrame()));
	else
		macroRec->OperandSequence(3, mr_prop, _T("addWorld"),mr_reftarg, this, mr_varname, _T("frameNo:"), mr_int, int(t/GetTicksPerFrame()));
	Matrix3 matTM(1);
	DWORD flags = 0;
	flags |= COPYKEY_POS;
	flags |= COPYKEY_ROT;
	flags |= COPYKEY_SCALE;

	// Note: When I add a new link target, I have to add new keys for the newly added node
	// in the target list, and delete the keys that has no meaning anymore. I have 
	// to do the following things:
	//	1. Add a key to the target(selection) at frame#(selection)
	//	2. Add a key to the target(selection) at frame#(selection + 1), if it exists
	//	3. Add a key to the target(selection -1) at frame#(selection)
	//	4. Add the targe(selection) node itself.
	//	5. Add a key to the source at frame#(selection)
	//	6. Delete the key for the target(selection) at previous frame#(selection)
	//	7. Delete the key for the target (selection+1) at previous frame#(selection+1)

	// There are special cases when selection = 0 and selection = ct - 1

	// In the "Key Hierarchy" mode, I have to delete all the keys up the hirearchy
	// of the target(selection) and target(selection - 1) at the keyframes described above


	switch (Getlink_key_mode())
	{

//		case 0: No key, so we don't do anything here.
		case 1: // Key_nodes_only // Key the source object, the fromNode target object and the toNode target object

			tmControl->CopyKeysFromTime(t, t, flags); // Key the source object
			theHold.Begin();
			CopyKeysFromTime(t, t, flags);
			theHold.Accept(GetString(IDS_AG_LINK_LIST));

			if (Getlink_key_nodes_mode() == 0) break;

			if (i == 0 ){	// only one node to key -- the fromNode
				INode *fromNode;
				fromNode = pblock->GetINode(link_target_list, t, i);
				if (fromNode != NULL) {
//					theHold.Begin();
					fromNode->GetTMController()->CopyKeysFromTime(t, t, flags);
//					theHold.Accept(GetString(IDS_AG_LINK_LIST));
				}
			}
			if (ct > 1 ){

				INode *fromNode;
				fromNode = pblock->GetINode(link_target_list, t, ct-2);
				if (fromNode != NULL) {
//					theHold.Begin();
					fromNode->GetTMController()->CopyKeysFromTime(t, t, flags);
//					theHold.Accept(GetString(IDS_AG_LINK_LIST));
				}

				INode *toNode;
				toNode = pblock->GetINode(link_target_list, t, ct-1);
				if (toNode != NULL) {
//					theHold.Begin();
					toNode->GetTMController()->CopyKeysFromTime(t, t, flags);
//					theHold.Accept(GetString(IDS_AG_LINK_LIST));
				}
			}
		break;

		case 2: // Key_entire_hierarchy  // Key the source object, the fromNode target object and the toNode target object AND
				// all their parents

			tmControl->CopyKeysFromTime(t, t, flags); // Key the source object
//			theHold.Begin();
			CopyKeysFromTime(t, t, flags);
//			theHold.Accept(GetString(IDS_AG_LINK_LIST));


			// now key the parents of the source object

			ULONG handle;
			NotifyDependents(FOREVER, (PartID)&handle, REFMSG_GET_NODE_HANDLE); 
			INode *sourceNode;
			sourceNode = GetCOREInterface()->GetINodeByHandle(handle);
			INode *sourceParentNode;
			sourceParentNode = sourceNode->GetParentNode();
			while (sourceParentNode != GetCOREInterface()->GetRootNode()){
//				theHold.Begin();
				sourceParentNode->GetTMController()->CopyKeysFromTime(t, t, flags);
//				theHold.Accept(GetString(IDS_AG_LINK_LIST));
				sourceParentNode = sourceParentNode->GetParentNode();
			}


			if (Getlink_key_hierarchy_mode() == 0) break;

			if (ct == 1){		// there is only one target
				INode *fromNode;
				fromNode = pblock->GetINode(link_target_list, t, ct-1);
				if (fromNode != NULL) {
//					theHold.Begin();
					fromNode->GetTMController()->CopyKeysFromTime(t, t, flags);
//					theHold.Accept(GetString(IDS_AG_LINK_LIST));
				}
				INode *fromParentNode;
				fromParentNode = fromNode->GetParentNode();

				while (fromParentNode != GetCOREInterface()->GetRootNode()){
//					theHold.Begin();
					fromParentNode->GetTMController()->CopyKeysFromTime(t, t, flags);
//					theHold.Accept(GetString(IDS_AG_LINK_LIST));
					fromParentNode = fromParentNode->GetParentNode();
				}
			}
			
			if (ct > 1 ){ // there are more than one nodes 
				INode *fromNode;
				fromNode = pblock->GetINode(link_target_list, t, ct-2);
				if (fromNode != NULL) {
//					theHold.Begin();
					fromNode->GetTMController()->CopyKeysFromTime(t, t, flags);
//					theHold.Accept(GetString(IDS_AG_LINK_LIST));
				}

				INode *fromParentNode;
				if (fromNode != NULL) {
					fromParentNode = fromNode->GetParentNode();

					while (fromParentNode != GetCOREInterface()->GetRootNode()){
//						theHold.Begin();
						fromParentNode->GetTMController()->CopyKeysFromTime(t, t, flags);
//						theHold.Accept(GetString(IDS_AG_LINK_LIST));
						fromParentNode = fromParentNode->GetParentNode();
					}
				}

				INode *toNode;
				toNode = pblock->GetINode(link_target_list, t, ct-1);
				if (toNode != NULL) {
//					theHold.Begin();
					toNode->GetTMController()->CopyKeysFromTime(t, t, flags);
//					theHold.Accept(GetString(IDS_AG_LINK_LIST));
				}

				INode *toParentNode;
				if (toNode != NULL) {
					toParentNode = toNode->GetParentNode();

					while (toParentNode != GetCOREInterface()->GetRootNode()){
//						theHold.Begin();
						toParentNode->GetTMController()->CopyKeysFromTime(t, t, flags);
//						theHold.Accept(GetString(IDS_AG_LINK_LIST));
						toParentNode = toParentNode->GetParentNode();
					}
				}
			}
		break;

	}

	theHold.Accept(GetString(IDS_AG_LINK_LIST));
}

	/*
void LinkCtrl::DeleteLink(int i)
	{
	if (nodes.Count()==1) {
		TimeValue t = GetCOREInterface()->GetTime();
		Matrix3 ptm = GetParentTM(t);
		Matrix3 itm(1), tm = ptm;
		tmControl->GetValue(t,&tm,FOREVER,CTRL_RELATIVE);
		tmControl->ChangeParents(t,ptm,itm,tm);
		}

	DeleteReference(i+1);
	if (theHold.Holding() && !TestAFlag(A_HELD)) 
		theHold.Put(new LinksRestore(this));
	nodes.Delete(i,1);
	times.Delete(i,1);
	NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	if (hWnd && editCont==this) SetupList();
	}
  */

BOOL LinkConstTransform::DeleteTarget(int i)
	{

	int ct = pblock->Count(link_target_list);
	if (i < 0 || i >= ct) return FALSE;

	// This method doesn't handle the case when a node is deleted from the viewport
	// John has added some methods to handle this case -- try it!

	TimeValue t = GetCOREInterface()->GetTime();

	if (ct == 1) {
		Matrix3 ptm = GetParentTM(t);
		Matrix3 itm(1), tm = ptm;
		tmControl->GetValue(t,&tm,FOREVER,CTRL_RELATIVE);
		tmControl->ChangeParents(t,ptm,itm,tm);
	}


	macroRec->Disable();
	pblock->Delete(link_start_time, i, 1); //deleting the frame# entry of the deleted target node
	pblock->Delete(link_target_list, i, 1); // deletes the node from nodelist;
	macroRec->Enable();
	state = 2;
	macroRec->EmitScript();
	macroRec->OperandSequence(2, mr_prop, _T("deleteTarget"),mr_reftarg, this, mr_int, i+1);

	NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	return TRUE;
	}

void LinkConstTransform::SetTime(TimeValue linkTime, int i)
	{
	macroRec->Disable();
	pblock->SetValue(link_start_time, GetCOREInterface()->GetTime(), linkTime, i);
	SortNodes(i);
	macroRec->Enable();
	if (state != 3+i)
	{	macroRec->EmitScript();
		state = 3+i;
	}
	macroRec->OperandSequence(3, mr_prop, _T("setFrameNo"),mr_reftarg, this, mr_int, i+1, mr_int, int(linkTime/GetTicksPerFrame()));
	NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	}

/*
void LinkConstTransform::MapKeys(TimeMap *map, DWORD flags)
	{	

	for (int i=0; i<pblock->Count(link_start_time); i++) {

		Interval iv;
//		TimeValue sTime;
		int frameNo;		
		pblock->SetValue(link_start_time, GetCOREInterface()->GetTime(), map->map(pblock->GetValue(link_start_time, 0, frameNo, iv, i)), i);
	}
	
//	if (hWnd && editCont==this) SetupList();
	NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	}
*/

static LinkConstTransform *theSortCtrl = NULL;
static int __cdecl CompareNodes(const void *a, const void *b)
	{
	int *aa = (int*)a;
	int *bb = (int*)b;
	assert(theSortCtrl);
	
	TimeValue time_diff = theSortCtrl->pblock->GetTimeValue(link_start_time, 0, *aa) - theSortCtrl->pblock->GetTimeValue(link_start_time, 0, *bb);
	return time_diff;

	}

void LinkConstTransform::SortNodes(int sel)
	{
	// Setup a map for a shell sort
	TimeValue t = GetCOREInterface()->GetTime();
	Tab<int> map;
	if (pblock->Count(link_target_list) != pblock->Count(link_start_time)) return;
	int cnt = pblock->Count(link_target_list);
	map.SetCount(cnt);	
	for (int i = 0; i < map.Count(); i++) 
	{
		map[i] = i;
	}

	// Sort
	theSortCtrl = this;
	map.Sort(CompareNodes);

	// Put everything in the right place

	Tab<INode*> onodes;
	onodes.SetCount(cnt);
	for (i = 0; i < cnt; i++){
		INode *sNode;
		pblock->GetValue(link_target_list, 0, sNode, FOREVER, i);
		onodes[i] =  sNode;
	}
	
	Tab<TimeValue> otimes;
	otimes.SetCount(cnt);
	for (i = 0; i < cnt; i++){

		TimeValue sTime = pblock->GetTimeValue(link_start_time, t, i);
		otimes[i] =  sTime;
	}

	INode *selINode1;
	pblock->GetValue(link_target_list, 0, selINode1, FOREVER, sel);

	for (i = 0; i < cnt; i++) {

		INode *sNode;
		TimeValue sTime;
		sNode = onodes[map[i]];
		pblock->SetValue(link_target_list, 0, sNode, i);
		sTime = otimes[map[i]];
		pblock->SetValue(link_start_time, 0, sTime, i);

	}

	INode *selINode2;
	pblock->GetValue(link_target_list, 0, selINode2, FOREVER, sel);

// compare the sel INode with one after and one before. 
// if they are unequal, the list has been re-organized.
	// in that case, see if the Frame# spinner has gone 
	// up (down the list) or down (up the list).
		// if spinner is up: set LB_SETCURSEL to sel+1 (ie down the list)
		// else spinner is down: set LB_SETCURSEL to sel-1 (ie up the list)
	int curCurSel = sel;
	if (selINode1 != selINode2){
		if (frameNoSpinnerUp) {
			while (curCurSel < pblock->Count(link_start_time)-1){
				
				TimeValue time1, time2;

				time1 = pblock->GetTimeValue(link_start_time, t, curCurSel+1);
				time2 = pblock->GetTimeValue(link_start_time, t, curCurSel);
				if (time1 == time2){
					time1 = time1 + GetTicksPerFrame();
					pblock->SetValue(link_start_time, t, time1, curCurSel+1);
					SortNodes(curCurSel+1);
				}
				SendDlgItemMessage(hWnd, IDC_LINK_LIST, LB_SETCURSEL, curCurSel+1, 0);
				curCurSel = curCurSel+1;
			}
		}
		else {
//			int curCurSel = sel;
			while (curCurSel > 0){

				TimeValue time1, time2;

				time1 = pblock->GetTimeValue(link_start_time, t, curCurSel-1);
				time2 = pblock->GetTimeValue(link_start_time, t, curCurSel);
				if (time1 == time2){
					time1 = time1 - GetTicksPerFrame();
					pblock->SetValue(link_start_time, t, time1, curCurSel-1);
					SortNodes(curCurSel-1);
				}
				SendDlgItemMessage(hWnd, IDC_LINK_LIST, LB_SETCURSEL, curCurSel-1, 0);
				curCurSel = curCurSel-1;
			}
		}

	}
//	last_selection = curCurSel;
	ISpinnerControl* spin = GetISpinner(GetDlgItem(hWnd, IDC_LINK_TIME_SPINNER));				

	int linkTime = (int)pblock->GetTimeValue(link_start_time, t, curCurSel);

	if (spin != NULL) spin->SetValue((int)linkTime/GetTicksPerFrame(), FALSE);
	ReleaseISpinner(spin);
	NotifyDependents(FOREVER,0,REFMSG_CHANGE);


	// Redo the list selecting the node in its new place
	if (sel>=0) {
		for (i=0; i<map.Count(); i++) {
			if (map[i]==sel) {
				RedrawListbox(GetCOREInterface()->GetTime()); // why was SetupList here?
				break;
			}
		}
	}
}

int LinkConstTransform::RemapRefOnLoad(int iref){
	int cc = iref;
	if (linkConstraintVersion <= 310){
		if (iref >= 1){

			return iref + 2;
		}
	}
	return iref;

}

RefTargetHandle LinkConstTransform::GetReference(int i)
	{
	if (i==0){							// this is LINKCTRL_CONTROL_REF
		return tmControl;
	}
	if (i == LINKCTRL_PBLOCK_REF){		// this is i = 2
		return pblock;
	}
	int bb = nodes.Count();
	i = i - 3;
	if (i >= 0 && i < nodes.Count()) {
		return nodes[i];
	}
	return NULL;
	}

void LinkConstTransform::SetReference(int i, RefTargetHandle rtarg)
	{

		switch (i)
		{
			case LINKCTRL_CONTROL_REF:
				tmControl = (Control*)rtarg;
			break;

			case LINKCTRL_PBLOCK_REF:
				pblock = (IParamBlock2*)rtarg; 
			break;
		}

		if (i > 2){
			nodes[i-3] = (INode*)rtarg;
		}

	}

RefResult LinkConstTransform::NotifyRefChanged (Interval iv, RefTargetHandle hTarg, PartID& partID, RefMessage msg) 
	{

	switch (msg) {
		case REFMSG_CHANGE:
			{
				if (hTarg == pblock){
					ParamID changing_param = pblock->LastNotifyParamID();
					link_const_paramblk.InvalidateUI(changing_param);
				}
			}
			break;

//		case REFMSG_TARGET_DELETED:
//			{
//				for (int i=nodes.Count()-1; i>=0; i--) {
//				if (nodes[i]==hTarg) DeleteTarget(i);}

//				int ct = pblock->Count(link_target_list);
//				for (int i =  ct-1; i >=0; i--){
//					if (hTarg == pblock->GetINode(link_target_list, 0, i)){
//						DeleteTarget(i);
//					}
//				}
//				
//			}
//			break;
			

	}
	return REF_SUCCEED;
}

class PickTargRestore : public RestoreObj {
	public:
		LinkConstTransform *cont;
		PickTargRestore(LinkConstTransform *c) {cont=c;}
		void Restore(int isUndo) {
			if (cont->editCont == cont) {
				link_const_paramblk.InvalidateUI();
				}									
			}
		void Redo() {
		}
	};


/*
void LinkConstTransform::SetupList(int sel)
	{
	if (!hWnd || editCont!=this) return;
	int csel = SendDlgItemMessage(hWnd,IDC_LINK_LIST,LB_GETCURSEL,0,0);
	if (sel>=0) csel = sel;
	SendDlgItemMessage(hWnd,IDC_LINK_LIST,LB_RESETCONTENT,0,0);
	for (int i=0; i<pblock->Count(link_target_list); i++) {
		INode *sNode;
		Interval iv;
		pblock->GetValue(link_target_list, 0, sNode, iv, i);
		SendDlgItemMessage(hWnd,IDC_LINK_LIST,LB_ADDSTRING,0,
			(LPARAM)sNode->GetName());	// 	replaces nodes[i]->GetName());
		}
	if (csel!=LB_ERR && csel<pblock->Count(link_target_list)) {
		SendDlgItemMessage(hWnd,IDC_LINK_LIST,LB_SETCURSEL,csel,0);
		}
	ListSelChanged();
	}


void LinkConstTransform::ListSelChanged()
	{
	int sel = SendDlgItemMessage(hWnd,IDC_LINK_LIST,LB_GETCURSEL,0,0);
	if (sel!=LB_ERR) {
		iDelOb->Enable();
		iTime->Enable();
		iTime->SetValue(times[sel],FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_LINK_TIMELABEL),TRUE);
	} else {
		iDelOb->Disable();
		iTime->Disable();		
		EnableWindow(GetDlgItem(hWnd,IDC_LINK_TIMELABEL),FALSE);
		}
	}

void LinkConstTransform::Invalidate()
	{	
	if (hWnd && editCont==this) {
		valid = FALSE;
		InvalidateRect(hWnd,NULL,FALSE);		
		}
	}

*/

int LinkConstTransform::GetNumTargets(){
	return pblock->Count(link_target_list);
} 

	

INode* LinkConstTransform :: GetNode(int nodeNumber){
	if (nodeNumber >= 0 && nodeNumber < pblock->Count(link_target_list)){
		INode *link_targ;
		pblock->GetValue(link_target_list, 0, link_targ, FOREVER, nodeNumber);
		return link_targ;
	}
	return NULL;
}



BOOL LinkConstTransform::AddTarget(INode *target, int frameNo){
	if(target){
		if (target->TestForLoop(FOREVER,(ReferenceMaker *) this) == REF_SUCCEED){
			TimeValue sTime =  frameNo * GetTicksPerFrame();
			AddNewLink(target, sTime);
			return TRUE;
		}
	}

	else{
		TimeValue sTime =  frameNo * GetTicksPerFrame();
		AddNewLink(target, sTime);
		return TRUE;

	}
	return FALSE;
}

int LinkConstTransform::InternalAddWorldMethod(int frameNo){
		TimeValue sTime =  frameNo * GetTicksPerFrame();
		INode *nullNode=NULL;
		AddNewLink(nullNode, sTime);
		return 1;
}


int LinkConstTransform::Getlink_key_mode(){
	int key_mode;
	pblock->GetValue(link_key_mode, GetCOREInterface()->GetTime(), key_mode, FOREVER);
	if (key_mode == 0 || key_mode == 1 || key_mode == 2) return key_mode;
	else return -1;
}

int LinkConstTransform::Getlink_key_nodes_mode(){
	int key_n_mode;
	pblock->GetValue(link_key_nodes_mode, GetCOREInterface()->GetTime(), key_n_mode, FOREVER);
	if (key_n_mode == 0 || key_n_mode == 1) return key_n_mode;
	else return -1;
}

int LinkConstTransform::Getlink_key_hierarchy_mode(){
	int key_h_mode;
	pblock->GetValue(link_key_hierarchy_mode, GetCOREInterface()->GetTime(), key_h_mode, FOREVER);
	if (key_h_mode == 0 || key_h_mode == 1) return key_h_mode;
	else return -1;
}

BOOL LinkConstTransform::Setlink_key_mode(int keymode){
	if(keymode >= 0 && keymode <= 2){
		pblock->SetValue(link_key_mode, GetCOREInterface()->GetTime(), keymode);
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		return TRUE;
	}
	else
		return FALSE;
}

BOOL LinkConstTransform::Setlink_key_nodes_mode(int keynodes_mode){
	if(keynodes_mode == 0 || keynodes_mode == 1){
		pblock->SetValue(link_key_nodes_mode, GetCOREInterface()->GetTime(), keynodes_mode);
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		return TRUE;
	}
	else
		return FALSE;
}

BOOL LinkConstTransform::Setlink_key_hierarchy_mode(int keyhier_mode){
	if(keyhier_mode == 0 || keyhier_mode == 1){
		pblock->SetValue(link_key_hierarchy_mode, GetCOREInterface()->GetTime(), keyhier_mode);
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		return TRUE;
	}
	else
		return FALSE;
}


int LinkConstTransform::GetFrameNumber(int targetNumber){
	if (targetNumber >= 0 && targetNumber < pblock->Count(link_target_list)){
		
		TimeValue linkTime = pblock->GetTimeValue(link_start_time, GetCOREInterface()->GetTime(), targetNumber);
		return linkTime/GetTicksPerFrame();
	}
	else{
		return -1000000;
	}
}




BOOL LinkConstTransform::SetFrameNumber(int targetNumber, int frameNumber){
	if (targetNumber >= 0 && targetNumber < pblock->Count(link_target_list)){
		theHold.Begin();
		TimeValue timeNumber = frameNumber * GetTicksPerFrame();
		pblock->SetValue(link_start_time, GetCOREInterface()->GetTime(), timeNumber, targetNumber);
		SortNodes(targetNumber);
		RedrawListbox(GetCOREInterface()->GetTime(), targetNumber);
		theHold.Accept(GetString(IDS_AG_LINK_LIST));
		return TRUE;
	}
	else{
		return FALSE;
	}
}






/*--------------------------------------------------------------------*/
// LinkConstTransform UI


void LinkConstTransform::BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev){

	if (flags&BEGIN_EDIT_MOTION) {
		this->ip = ip;
		editCont = this;	
		linkCD.BeginEditParams(ip, this, flags, prev);		
		tmControl->BeginEditParams(ip,flags,NULL);
	} 
	else if (flags&BEGIN_EDIT_LINKINFO) {
		this->ip = ip;
		editCont = this;
		tmControl->BeginEditParams(ip,flags,NULL);
	}


	ip->RegisterTimeChangeCallback(&linkConstTimeChangeCallback);
	linkConstTimeChangeCallback.link_controller = this;
	
	IParamMap2* pmap = pblock->GetMap();
	if(pmap) hWnd = pmap->GetHWnd();

	if (last_selection < 0){
		RedrawListbox(GetCOREInterface()->GetTime());
	}
	else{
		RedrawListbox(GetCOREInterface()->GetTime(), last_selection);
		ISpinnerControl* spin = GetISpinner(GetDlgItem(hWnd, IDC_LINK_TIME_SPINNER));
		if (spin != NULL){
			
			int linkTime = (int)pblock->GetTimeValue(link_start_time, GetCOREInterface()->GetTime(), last_selection);
			spin->SetValue(linkTime/GetTicksPerFrame(), FALSE);
		}

	}

}


void LinkConstTransform::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
{
	if(editCont)tmControl->EndEditParams(ip,flags,NULL);
	this->ip = NULL;
	editCont = NULL;
	IParamMap2* pmap = pblock->GetMap();
	ip->ClearPickMode(); // need this, otherwise will crash on undo, while pickmode is active.


	
	if (pmap != NULL)
	{
		if (next && next->ClassID() == ClassID() && ((LinkConstTransform*)next)->pblock)
		{
			pmap->SetParamBlock(((LinkConstTransform*)next)->pblock);
			ip->ClearPickMode();
			//pmap->UpdateUI(GetCOREInterface()->GetTime());
		}
		else
			linkCD.EndEditParams(ip, this, flags | END_EDIT_REMOVEUI, next);
	}
	else
	{
		int index = aprops.FindProperty(PROPID_INTERPUI);
		if (index>=0) {
			InterpCtrlUI *ui = (InterpCtrlUI*)aprops[index];
			if (ui->hParams) {
				ip->UnRegisterDlgWnd(ui->hParams);
				ip->DeleteRollupPage(ui->hParams);			
				}
			index = aprops.FindProperty(PROPID_INTERPUI);
			if (index>=0) {
				delete aprops[index];
				aprops.Delete(index,1);
				}
			}	
	}
		ip->UnRegisterTimeChangeCallback(&linkConstTimeChangeCallback);		
		hWnd = NULL;

}

TSTR LinkConstTransform::SvGetRelTip(IGraphObjectManager *gom, IGraphNode *gNodeTarger, int id, IGraphNode *gNodeMaker)
{
	return SvGetName(gom, gNodeMaker, false) + " -> " + gNodeTarger->GetAnim()->SvGetName(gom, gNodeTarger, false);
}

bool LinkConstTransform::SvCanDetachRel(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker)
{
	return true;
}

bool LinkConstTransform::SvDetachRel(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker)
{
	for( int i=0; i<GetNumTargets(); i++ ) {
		if( GetNode(i) == gNodeTarget->GetAnim() ) {
			DeleteTarget(i);
			return true;
		}
	}
	return false;
}

bool LinkConstTransform::SvCanConcludeLink(IGraphObjectManager *gom, IGraphNode *gNode, IGraphNode *gNodeChild)
{
	Animatable *pChildAnim = gNodeChild->GetAnim();
	if( pChildAnim->SuperClassID() == BASENODE_CLASS_ID )
		return true;

	return Control::SvCanConcludeLink(gom, gNode, gNodeChild);
}

bool LinkConstTransform::SvLinkChild(IGraphObjectManager *gom, IGraphNode *gNodeThis, IGraphNode *gNodeChild)
{
	Animatable *pChildAnim = gNodeChild->GetAnim();
	if( pChildAnim->SuperClassID() == BASENODE_CLASS_ID ) {
		if( AddTarget( (INode*)pChildAnim ) ) {
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
			return true;
		}
	}
	return Control::SvLinkChild(gom, gNodeThis, gNodeChild);
}

SvGraphNodeReference LinkConstTransform::SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags)
{
	SvGraphNodeReference nodeRef = Control::SvTraverseAnimGraph( gom, owner, id, flags );

	if( nodeRef.stat == SVT_PROCEED ) {
		for( int i=0; i<GetNumTargets(); i++ ) {
			gom->AddRelationship( nodeRef.gNode, GetNode(i), i, RELTYPE_CONSTRAINT );
		}
	}

	return nodeRef;
}

int LinkConstTransform::SetProperty(ULONG id, void *data)
	{
	if (id==PROPID_JOINTPARAMS) {		
		if (!data) {
			int index = aprops.FindProperty(id);
			if (index>=0) {
				aprops.Delete(index,1);
				}
		} else {				
			}
		return 1;
	} else
	if (id==PROPID_INTERPUI) {		
		if (!data) {
			int index = aprops.FindProperty(id);
			if (index>=0) {
				aprops.Delete(index,1);
				}
		} else {
			InterpCtrlUI *ui = (InterpCtrlUI*)GetProperty(id);
			if (ui) {
				*ui = *((InterpCtrlUI*)data);
			} else {
				aprops.Append(1,(AnimProperty**)&data);
				}					
			}
		return 1;
	} 
	else if (id==ADD_WORLD_LINK){

		return InternalAddWorldMethod( *(int*)(data) );
	}
	
	else {
		return Animatable::SetProperty(id,data);
		}
	}

void* LinkConstTransform::GetProperty(ULONG id)
	{
	if (id==PROPID_INTERPUI || id==PROPID_JOINTPARAMS) {
		int index = aprops.FindProperty(id);
		if (index>=0) {
			return aprops[index];
		} else {
			return NULL;
			}
	} else {
		return Animatable::GetProperty(id);
		}
	}


#define LINKCOUNT_CHUNK		0x0100
#define TIMES_CHUNK			0x0110
#define VERSION_CHUNK		0x0111


IOResult LinkConstTransform::Save(ISave *isave)
	{	
		ULONG nb;
	linkConstraintVersion = 400; // from R3.1 to 10/27
	isave->BeginChunk(VERSION_CHUNK);
	isave->Write(&linkConstraintVersion,sizeof(int),&nb);
	isave->EndChunk();
	
	return IO_OK;
	}


// provide a post-load callback so old-version path controller data can be loaded into the ParamBlock2
class LinkPLCB : public PostLoadCallback 
{
public:
	LinkConstTransform*	p;
	LinkPLCB(LinkConstTransform* pth){ p = pth;}
	void proc(ILoad *iload)
	{
		if (p->linkConstraintVersion <= 310)
			{
			int nc = p->nodes.Count();
			int tc = p->times.Count();
			if (nc!= tc) return;
			p->pblock->SetCount(link_target_list, nc);
			p->pblock->SetCount(link_start_time, nc);
			for (int i = 0; i < nc; i++){
				TCHAR *str1 = p->nodes[i]->GetName();
//				p->AddNewLink(p->nodes[i],p->times[i]);

				p->pblock->SetValue(link_target_list, TimeValue(0), p->nodes[i], i);	// copy nodes to paramblock
				p->pblock->SetValue(link_start_time, TimeValue(0), p->times[i], i); 	// copy times to paramblock
				p->ReplaceReference(i+3, NULL);	// nodes to NULL, times aren't references
			}		
			
			p->nodes.ZeroCount();			// gets rid of nodes
			p->times.ZeroCount();			// gets rid of times
		}

		delete this;

	}
};

IOResult LinkConstTransform::Load(ILoad *iload)
	{

// don't put any paramblock stuff in "load" since paramblock is not set up yet
	ULONG nb;
	int ct;	
	IOResult res;
	LinkPLCB* plcb = new LinkPLCB(this);
	iload->RegisterPostLoadCallback(plcb);
	linkConstraintVersion = 390; // from R3.1 to 10/27
		
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case VERSION_CHUNK:
				iload->Read(&linkConstraintVersion,sizeof(int), &nb);
				break;

			case LINKCOUNT_CHUNK: {
				iload->Read(&ct,sizeof(ct),&nb);
				nodes.SetCount(ct);
				times.SetCount(ct);
				for (int i=0; i<ct; i++) nodes[i] = NULL; // creates placeholders for the nodes
				break;
				}

			case TIMES_CHUNK:
				linkConstraintVersion = 310;
				iload->Read(times.Addr(0),sizeof(TimeValue)*ct,&nb);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}

	return IO_OK;
}

/*
void LinkConstTransform::DeleteKeysFrom(INode *toDeleteNode, TimeValue toDeleteTime){
	if (toDeleteNode != NULL) {
		toDeleteNode->GetTMController()->GetPositionController()->DeleteKeyAtTime(toDeleteTime);
		toDeleteNode->GetTMController()->GetRotationController()->DeleteKeyAtTime(toDeleteTime);
		toDeleteNode->GetTMController()->GetScaleController()->DeleteKeyAtTime(toDeleteTime);
	}
}
*/
void LinkConstTransform::ChangeKeys(int sell)
	{

	ULONG handle;
	DWORD flags=0;
	flags|= TRACK_DOALL;
	NotifyDependents(FOREVER, (PartID)&handle, REFMSG_GET_NODE_HANDLE);
	INode *sourceNode;
	sourceNode = GetCOREInterface()->GetINodeByHandle(handle);
	TimeValue spDownTime = spDownVal * GetTicksPerFrame();
	TimeValue spUpTime = spUpVal * GetTicksPerFrame();


	switch (Getlink_key_mode())
	{
		case 1: 

		// delete the previous source node keys
//		tmControl->GetPositionController()->DeleteKeyAtTime(spDownTime);
//		tmControl->GetRotationController()->DeleteKeyAtTime(spDownTime);
//		tmControl->GetScaleController()->DeleteKeyAtTime(spDownTime);

		// add the new source node keys
		CopyKeysFromTime(spUpTime, spUpTime, flags);

		//delete the previous target node keys at spDownTime

// BE VERY CAREFUL HERE: YOU DON'T WANT TO TOUCH THE EXISTING ANIMATION OF THE TARGET OBJECTS
//-------------------------------------------------------------------------------------------
		if (spDownSel == 0 ){	// delete key from only one node -- the fromNode
			INode *fromNode;
			fromNode = pblock->GetINode(link_target_list, GetCOREInterface()->GetTime(), spDownSel); // i can only be = 0
//			DeleteKeysFrom(fromNode, spDownTime);

			if (pblock->Count(link_target_list) >1 ){
				INode *toNode;
				toNode = pblock->GetINode(link_target_list, GetCOREInterface()->GetTime(), spDownSel+1);
				// deleting the key of the toNode at current frame#
//				DeleteKeysFrom(toNode, spDownTime);
			}
		}

		if (spDownSel > 0 ){

			// deleting the key of the fromNode at current frame#
			INode *fromNode;
			fromNode = pblock->GetINode(link_target_list, GetCOREInterface()->GetTime(), spDownSel-1);
//			TSTR str1 = fromNode->GetName();
//			DeleteKeysFrom(fromNode, spDownTime);
			INode *toNode;
			toNode = pblock->GetINode(link_target_list, GetCOREInterface()->GetTime(), spDownSel);
//			TSTR str2 = toNode->GetName();
			// deleting the key of the toNode at current frame#
//			DeleteKeysFrom(toNode, spDownTime);
			if (spDownSel < pblock->Count(link_target_list) - 1){
				INode *toNode;
				toNode = pblock->GetINode(link_target_list, GetCOREInterface()->GetTime(), spDownSel);
				//TSTR str3 = toNode->GetName();
				// deleting the key of the toNode at the next frame#
//				DeleteKeysFrom(toNode, spDownTime);
			}
		}
//-------------------------------------------------------------------------------------------
		//Add the previous target node keys at spDownTime
//-------------------------------------------------------------------------------------------
		if (spUpSel == 0 ){	// only one node to key -- the fromNode
			INode *fromNode;
			fromNode = pblock->GetINode(link_target_list, GetCOREInterface()->GetTime(), spUpSel);
			if (fromNode != NULL) {
				fromNode->GetTMController()->CopyKeysFromTime(spUpTime, spUpTime, flags);
			}
		}
		if (spUpSel > 0 ){

			INode *fromNode;
			fromNode = pblock->GetINode(link_target_list, GetCOREInterface()->GetTime(), spUpSel-1);
			if (fromNode != NULL) {
				fromNode->GetTMController()->CopyKeysFromTime(spUpTime, spUpTime, flags);
			}

			INode *toNode;
			toNode = pblock->GetINode(link_target_list, GetCOREInterface()->GetTime(), spUpSel);
			if (toNode != NULL) {
				toNode->GetTMController()->CopyKeysFromTime(spUpTime, spUpTime, flags);
			}
		}

  
		break;

		case 2:

		// delete the previous source node PARENT'S keys
		INode *sourceParentNode;
		sourceParentNode = sourceNode->GetParentNode();
		while (sourceParentNode != GetCOREInterface()->GetRootNode()){
//			sourceParentNode->GetTMController()->GetPositionController()->DeleteKeyAtTime(spDownTime);
			sourceParentNode = sourceParentNode->GetParentNode();
		}

		// add the new source node PARENT'S keys
		while (sourceParentNode != GetCOREInterface()->GetRootNode()){
			sourceParentNode->GetTMController()->CopyKeysFromTime(spUpTime, spUpTime, flags);
			sourceParentNode = sourceParentNode->GetParentNode();
		}

		break;
	}
}

/*
TimeValue TimeMapp::Mapp(TimeValue fromTime)
{
	
//	TimeValue toTime;
	if (fromTime == p->spDownVal)
		return p->spUpVal; // need to convert this to a frame number
	return p->spDownVal;

//	return 0;
}
*/
