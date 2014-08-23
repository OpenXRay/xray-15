
/**********************************************************************
 *
	FILE: position_cnstrnt.cpp

	DESCRIPTION: A controller that controls the position of a source object
				based on the position of one or many weighted target objects

	CREATED BY: Ambarish Goswami 4/2000
	            

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "ctrl.h"
#include "units.h"
#include "interpik.h"
#include "istdplug.h"
#include "iparamm2.h"


#define POS_CONTROL_CNAME		GetString(IDS_AG_POSITION)

enum { position_target_weight, position_target_list, position_relative,};


class PosConstPosition; 
class PosConstTimeChangeCallback : public TimeChangeCallback
{
	public:
		PosConstPosition* pcontroller;
		void TimeChanged(TimeValue t);
};

class posConstValidatorClass : public PBValidator
{
	public:
		PosConstPosition *mod;
	private:
		BOOL Validate(PB2Value &v);
};


class PosConstPosition : public	IPosConstPosition 
{
	public:

		posConstValidatorClass validator;							
		void RedrawListbox(TimeValue t, int sel = -1);			// AG added
		HWND hWnd;
		int last_selection, oldTargetNumber;
		
		PosConstTimeChangeCallback posConstTimeChangeCallback;	
		IParamBlock2* pblock;

		DWORD flags;
		Point3 curval, basePointWorld, basePointLocal;
		Interval ivalid;
		int initialPositionFlag;
		Point3 InitialPosition;
		
		static PosConstPosition *editCont;
		static IObjParam *ip;

		PosConstPosition(const PosConstPosition &ctrl);
		PosConstPosition(BOOL loading=FALSE);

		~PosConstPosition();

		int GetNumTargets();									
		int GetnonNULLNumPosTargets();
		INode* GetNode(int targetNumber);						
		float GetTargetWeight(int targetNumber);	
		BOOL SetTargetWeight(int targetNumber, float weight);	
		BOOL AppendTarget(INode *target, float weight=50.0);		
		BOOL DeleteTarget(int selection);					
		
		BOOL GetRelative() {return Relative();}	
		void SetRelative(BOOL rel);								
		BOOL Relative();	

		void Update(TimeValue t);

		// Animatable methods
		Class_ID ClassID() { return Class_ID(POSITION_CONSTRAINT_CLASS_ID,0); }  
		SClass_ID SuperClassID() { return CTRL_POSITION_CLASS_ID; }  		
		
		void GetClassName(TSTR& s) {s = POS_CONTROL_CNAME;}
		void DeleteThis() {delete this;} // intellgent destructor
		int IsKeyable() {return 0;}		// if the controller is keyable 

		int NumSubs()  {return 1;} //because it uses the paramblock
		Animatable* SubAnim(int i) {return pblock;}
		TSTR SubAnimName(int i) { return GetString(IDS_AG_POSITIONPARAMS); }
		int SubNumToRefNum(int subNum) {if (subNum==0) return POSPOS_PBLOCK_REF; else return -1;}

		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev ); //to display controller params on the motion panel
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next ); // to undisplay

		int SetProperty(ULONG id, void *data);
		void *GetProperty(ULONG id);

		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock

		// Animatable's Schematic View methods
		SvGraphNodeReference SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags);
		TSTR SvGetRelTip(IGraphObjectManager *gom, IGraphNode *gNodeTarger, int id, IGraphNode *gNodeMaker);
		bool SvCanDetachRel(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker);
		bool SvDetachRel(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker);
		bool SvLinkChild(IGraphObjectManager *gom, IGraphNode *gNodeThis, IGraphNode *gNodeChild);
		bool SvCanConcludeLink(IGraphObjectManager *gom, IGraphNode *gNode, IGraphNode *gNodeChild);

		// Reference methods
		int NumRefs() { return 1; };	
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
		RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage);

		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		void RescaleWorldUnits(float f) {}

		// Control methods
		void Copy(Control *from);
		RefTargetHandle Clone(RemapDir& remap);
		BOOL IsLeaf() {return FALSE;}
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method);
		void SetValue(TimeValue t, void *val, int commit, GetSetMethod method){}
		void CommitValue(TimeValue t) { }
		void RestoreValue(TimeValue t) { }

		//Function Publishing method (Mixin Interface)
		//Added by Adam Felt (5-16-00)
		//******************************
		BaseInterface* GetInterface(Interface_ID id)  
		{ 
			if (id == POS_CONSTRAINT_INTERFACE) 
				return (PosConstPosition*)this; 
			else 
				return FPMixinInterface::GetInterface(id);
		} 
		//******************************

	};

	BOOL posConstValidatorClass::Validate(PB2Value &v)
	{
		INode *node = (INode*) v.r;

		for (int i = 0; i < mod->pblock->Count(position_target_list); i++){
			if (node == mod->pblock->GetINode(position_target_list, 0, i))
				return FALSE;
		}
		return TRUE;
	}

void PosConstTimeChangeCallback::TimeChanged(TimeValue t){

	int selection = SendDlgItemMessage(pcontroller->hWnd, IDC_POS_TARG_LIST, LB_GETCURSEL, 0, 0);
	ISpinnerControl* spin = GetISpinner(GetDlgItem(pcontroller->hWnd, IDC_POS_CONS_WEIGHT_SPINNER));
	spin = GetISpinner(GetDlgItem(pcontroller->hWnd, IDC_POS_CONS_WEIGHT_SPINNER));

	Control *cont = pcontroller->pblock->GetController(position_target_weight, selection); 
	// until it is animated, paramblock doesn't have a controller assigned in it yet.
	if (spin && (cont !=NULL)) {
		if (cont->IsKeyAtTime(t,KEYAT_POSITION)) {
			spin->SetKeyBrackets(TRUE);
		} 
		else {
			spin->SetKeyBrackets(FALSE);
		}
	}
	ReleaseISpinner(spin);
	
	pcontroller->RedrawListbox(t);
}


IObjParam *PosConstPosition::ip					= NULL;
PosConstPosition *PosConstPosition::editCont    = NULL;

//********************************************************
// POSITION CONSTRAINT
//********************************************************
static Class_ID posConstControlClassID(POSITION_CONSTRAINT_CLASS_ID,0); 
class PosConstClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new PosConstPosition(loading); }
	const TCHAR *	ClassName() { return POS_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_POSITION_CLASS_ID; }
	Class_ID		ClassID() { return posConstControlClassID; }
	const TCHAR* 	Category() { return _T("");  }
	const TCHAR*	InternalName() { return _T("Position_Constraint"); }// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
	};

static PosConstClassDesc posCD;
ClassDesc* GetPosConstDesc() {return &posCD;}


//--- CustMod dlg proc ---------DOWN DOWN DOWN----from Modifiers/clstmode.cpp---------------


class PickControlNode : public PickModeCallback,
		public PickNodeCallback {
	public:			
		PosConstPosition *p;
		PickControlNode() {p = NULL;}
		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);		
		BOOL Pick(IObjParam *ip,ViewExp *vpt);		
		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);		
		BOOL Filter(INode *node);
		PickNodeCallback *GetFilter() {return this;}
		BOOL RightClick(IObjParam *ip,ViewExp *vpt) {return TRUE;}
	};
static PickControlNode thePickMode;

BOOL PickControlNode::Filter(INode *node){

	for (int i = 0; i < p->pblock->Count(position_target_list); i++){
		if (node == p->pblock->GetINode(position_target_list, 0, i)) 
			return FALSE;
	}

	if (node->TestForLoop(FOREVER,(ReferenceMaker *) p)!=REF_SUCCEED) 
	{
		return FALSE;
	}

	return TRUE;
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
			p->AppendTarget(node);
			p->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
			p->ip->RedrawViews(GetCOREInterface()->GetTime());
		}
		return FALSE;
	}

void PickControlNode::EnterMode(IObjParam *ip)
	{
		ICustButton *iBut = GetICustButton(GetDlgItem(p->hWnd,IDC_POS_TARG_PICKNODE));
		if (iBut) iBut->SetCheck(TRUE);
		ReleaseICustButton(iBut);
		GetCOREInterface()->PushPrompt("Select Position Target Object");
	}

void PickControlNode::ExitMode(IObjParam *ip)
	{
		ICustButton *iBut = GetICustButton(GetDlgItem(p->hWnd,IDC_POS_TARG_PICKNODE));
		if (iBut) iBut->SetCheck(FALSE);
		ReleaseICustButton(iBut);
		GetCOREInterface()->PopPrompt();
	}

//--- CustMod dlg proc -----------UP UP UP -------------------

//Function Publishing descriptor for Mixin interface
// Adam Felt (5-16-00)
//*****************************************************

static FPInterfaceDesc pos_constraint_interface(
    POS_CONSTRAINT_INTERFACE, _T("constraints"), 0, &posCD, 0,
		IPosConstPosition::get_num_targets,		_T("getNumTargets"), 0, TYPE_INDEX, 0, 0,
		IPosConstPosition::get_node,			_T("getNode"),	0, TYPE_INODE, 0, 1,
			_T("nodeNumber"),	0, TYPE_INDEX,
		IPosConstPosition::get_target_weight,	_T("getWeight"), 0, TYPE_FLOAT, 0, 1,
			_T("targetNumber"),	0, TYPE_INDEX,
		IPosConstPosition::set_target_weight,	_T("setWeight"), 0, TYPE_BOOL, 0, 2,
			_T("targetNumber"),	0, TYPE_INDEX,
			_T("weight"),	0, TYPE_FLOAT,
		IPosConstPosition::append_target,		_T("appendTarget"), 0, TYPE_BOOL, 0, 2,
			_T("target"),	0, TYPE_INODE,
			_T("weight"),	0, TYPE_FLOAT,
		IPosConstPosition::delete_target,		_T("deleteTarget"), 0, TYPE_BOOL, 0, 1,
			_T("targetNumber"), 0, TYPE_INDEX,

		end
		);

//  Get Descriptor method for Mixin Interface
//  *****************************************
FPInterfaceDesc* IPosConstPosition::GetDesc()
{
	return &pos_constraint_interface;
}

//*********************************************
// End of Function Publishing Code


enum { pos_const_params};


class PosConstPBAccessor : public PBAccessor
{ 
public:

	void TabChanged(tab_changes changeCode, Tab<PB2Value>* tab, 
                  ReferenceMaker* owner, ParamID id, int tabIndex, int count) 
		{ 

		if ((id == position_target_weight) || (id == position_target_list)){

			PosConstPosition* p = (PosConstPosition*)owner;
			if (changeCode == tab_ref_deleted){		  
				int j = p->pblock->Delete(position_target_list, tabIndex, 1); // deletes the node from nodelist;
				int k = p->pblock->Delete(position_target_weight, tabIndex, 1); // deletes corresponding weight;
			}
			// CAL-09/13/02: move these two lines after pblock->Delete()
			int ct = p->pblock->Count(position_target_list);
			int ctf = p->pblock->Count(position_target_weight);
			if (ct == ctf){

				switch(changeCode)
				{
					case tab_append:
					{
						p->last_selection = ct - 1;
					}
					break;

					case tab_delete:
					{
						p->last_selection = p->last_selection - 1;
					}
					break;

					case tab_setcount:
					{
						if (p->last_selection > ct - 1)
							p->last_selection = ct - 1;
						// CAL-09/10/02: set to 0 if there's any target in the list
						if (p->last_selection < 0 && ct > 0)
							p->last_selection = 0;
					}
					break;
				}

				
				p->RedrawListbox(GetCOREInterface()->GetTime(), p->last_selection);


//				int selection = SendDlgItemMessage(p->hWnd, IDC_POS_TARG_LIST, LB_GETCURSEL, 0, 0);
				
				if (p->last_selection < 0 || p->GetnonNULLNumPosTargets() < 1){ 
//					nothing is selected OR there are no targets
//					all buttons - target_delete, weight_edit, weight_spinner -- are disabled
					EnableWindow(GetDlgItem(p->hWnd, IDC_POS_CONS_WEIGHT_EDIT), FALSE);
					EnableWindow(GetDlgItem(p->hWnd, IDC_POS_CONS_WEIGHT_SPINNER), FALSE);
					ICustButton *iPickOb1;
					iPickOb1= GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_POS_TARG));
					if (iPickOb1 != NULL){
						iPickOb1->Enable(FALSE);		
						ReleaseICustButton(iPickOb1);
					}
				}
				else if (p->last_selection >= 0) {	
//					there is a valid selection 
//					automatically means there is AT LEAST one target
					if (p->GetnonNULLNumPosTargets() == 1){ // there is only one target
					// the "delete" button should be enabled
						ICustButton *iPickOb1;
						iPickOb1 = GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_POS_TARG));
						if (iPickOb1 != NULL){
							iPickOb1->Enable(TRUE);		
							ReleaseICustButton(iPickOb1);
						}
						// the weight_edit, weight_spinner buttons are disabled
						EnableWindow(GetDlgItem(p->hWnd, IDC_POS_CONS_WEIGHT_EDIT), FALSE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_POS_CONS_WEIGHT_SPINNER), FALSE);
					}
					
					else if (p->GetnonNULLNumPosTargets() > 1){ // there are more than one targets
						
//						all buttons - target_delete, weight_edit, weight_spinner -- are enabled
						EnableWindow(GetDlgItem(p->hWnd, IDC_POS_CONS_WEIGHT_EDIT), TRUE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_POS_CONS_WEIGHT_SPINNER), TRUE);
						ICustButton *iPickOb1;
						iPickOb1 = GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_POS_TARG));
						if (iPickOb1 != NULL){
							iPickOb1->Enable(TRUE);		
							ReleaseICustButton(iPickOb1);
						}
					}
				}

			}
				
			}
			
		}
				
	


	void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid)
	{
		PosConstPosition* p = (PosConstPosition*)owner;
		if (id == position_target_weight){
			if ((v.f) < 0.0f) v.f = 0.0f; 
		}
	}

	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		PosConstPosition* p = (PosConstPosition*)owner;
		if (id == position_target_list){
			IParamMap2* pmap = p->pblock->GetMap();	
		}	
	}
};



static PosConstPBAccessor pos_const_accessor;
class PosConstDlgProc : public ParamMap2UserDlgProc 
{
	public:

		void UpdatePosName(PosConstPosition* p){
			IParamMap2* pmap = p->pblock->GetMap();
		}

		BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{	
			PosConstPosition* p = (PosConstPosition*)map->GetParamBlock()->GetOwner();
			UpdatePosName(p);
			p->hWnd = hWnd;
			int ct = p->pblock->Count(position_target_list);
			int selection;
			ICustButton *iBut;
			ISpinnerControl* spin;
//			int ctf = p->pblock->Count(position_target_weight);

			switch (msg) {

				case WM_INITDIALOG:

					selection = p->last_selection;

					iBut = GetICustButton(GetDlgItem(hWnd,IDC_POS_TARG_PICKNODE));
					iBut->SetType(CBT_CHECK);
					iBut->SetHighlightColor(GREEN_WASH);
					ReleaseICustButton(iBut);
						
					spin = SetupFloatSpinner(hWnd, IDC_POS_CONS_WEIGHT_SPINNER, IDC_POS_CONS_WEIGHT_EDIT, 0.0f, 100.0f, 50.0f, 1.0f);
					spin = GetISpinner(GetDlgItem(hWnd, IDC_POS_CONS_WEIGHT_SPINNER));

					// CAL-09/10/02: automatically set last_selection to 0 when there're targets
					if (p->GetnonNULLNumPosTargets() < 1) { 
						// nothing is selected OR there are no targets
						// all buttons - target_delete, weight_edit, weight_spinner -- are disabled
						EnableWindow(GetDlgItem(p->hWnd, IDC_POS_CONS_WEIGHT_EDIT), FALSE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_POS_CONS_WEIGHT_SPINNER), FALSE);
						ICustButton *iPickOb1;
						iPickOb1	= GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_POS_TARG));
						if (iPickOb1 != NULL){
							iPickOb1->Enable(FALSE);		
							ReleaseICustButton(iPickOb1);
						}
						spin->SetValue(0.0f, FALSE);
					} else {
						if (selection < 0)
							p->last_selection = selection = 0;

						// there is a valid selection 
						// automatically means there is AT LEAST one target
						Control *cont = p->pblock->GetController(position_target_weight, selection); 
						// until it is animated, paramblock doesn't have a controller assigned in it yet.
						if (spin) {
							if ((cont != NULL) && cont->IsKeyAtTime(t,KEYAT_POSITION))
								spin->SetKeyBrackets(TRUE);
							else
								spin->SetKeyBrackets(FALSE);
						}
						ReleaseISpinner(spin);
						
						if (p->GetnonNULLNumPosTargets() == 1){ // there is only one target
//							the "delete" button should be enabled
							ICustButton *iPickOb1;
							iPickOb1 = GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_POS_TARG));
							if (iPickOb1 != NULL){
								iPickOb1->Enable(TRUE);		
								ReleaseICustButton(iPickOb1);
							}
//							the rest are disabled
							EnableWindow(GetDlgItem(p->hWnd, IDC_POS_CONS_WEIGHT_EDIT), FALSE);
							EnableWindow(GetDlgItem(p->hWnd, IDC_POS_CONS_WEIGHT_SPINNER), FALSE);
						}

						else if (p->GetnonNULLNumPosTargets() > 1){ 
//							there are more than one targets
//							all buttons - delete, weight_edit, weight_spinner -- are enabled
							EnableWindow(GetDlgItem(p->hWnd, IDC_POS_CONS_WEIGHT_EDIT), TRUE);
							EnableWindow(GetDlgItem(p->hWnd, IDC_POS_CONS_WEIGHT_SPINNER), TRUE);
							ICustButton *iPickOb1;
							iPickOb1 = GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_POS_TARG));
							if (iPickOb1 != NULL){
								iPickOb1->Enable(TRUE);		
								ReleaseICustButton(iPickOb1);
							}
						}
					}

					p->RedrawListbox(GetCOREInterface()->GetTime(), selection);
					return TRUE;
			
				break;


				case WM_COMMAND:

					if (LOWORD(wParam) == IDC_POS_TARG_LIST && HIWORD(wParam) == LBN_SELCHANGE)
					{
						
						selection = SendDlgItemMessage(p->hWnd, IDC_POS_TARG_LIST, LB_GETCURSEL, 0, 0);
						p->last_selection = selection;
						if (selection >= 0){
//							there is a valid selection 
//							automatically means there is AT LEAST one target

							if (p->GetnonNULLNumPosTargets() == 1){  // there is only one target
//								the "delete" button should be enabled
								ICustButton *iPickOb1;
								iPickOb1 = GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_POS_TARG));
								if (iPickOb1 != NULL){
									iPickOb1->Enable(TRUE);		
									ReleaseICustButton(iPickOb1);
								}
//								the rest are disabled
								EnableWindow(GetDlgItem(p->hWnd, IDC_POS_CONS_WEIGHT_EDIT), FALSE);
								EnableWindow(GetDlgItem(p->hWnd, IDC_POS_CONS_WEIGHT_SPINNER), FALSE);
							}


							else  if (p->GetnonNULLNumPosTargets() > 1){ 
//								there are more than one targets
//								all buttons - delete, weight_edit, weight_spinner -- are enabled
								EnableWindow(GetDlgItem(p->hWnd, IDC_POS_CONS_WEIGHT_EDIT), TRUE);
								EnableWindow(GetDlgItem(p->hWnd, IDC_POS_CONS_WEIGHT_SPINNER), TRUE);
								ICustButton *iPickOb1;
								iPickOb1 = GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_POS_TARG));
								if (iPickOb1 != NULL){
									iPickOb1->Enable(TRUE);		
									ReleaseICustButton(iPickOb1);
								}

								spin = GetISpinner(GetDlgItem(hWnd, IDC_POS_CONS_WEIGHT_SPINNER));
								float value = map->GetParamBlock()->GetFloat(position_target_weight, t, selection);
								spin->SetValue(value, FALSE);
								Control *cont = p->pblock->GetController(position_target_weight, selection); // until it is animated, paramblock doesn't have a controller assigned in it yet.
								if (spin && (cont != NULL)) {
									if (cont->IsKeyAtTime(t,KEYAT_POSITION)){
										spin->SetKeyBrackets(TRUE);
									}
									else {
										spin->SetKeyBrackets(FALSE);
									}
								}
								
								ReleaseISpinner(spin);
							}	
							p->RedrawListbox(GetCOREInterface()->GetTime(), selection);

						}
						

						else if (selection < 0 || p->GetnonNULLNumPosTargets() < 1){ 
//							nothing is selected OR there are no targets
//							all buttons - target_delete, weight_edit, weight_spinner -- are disabled
							EnableWindow(GetDlgItem(p->hWnd, IDC_POS_CONS_WEIGHT_EDIT), FALSE);
							EnableWindow(GetDlgItem(p->hWnd, IDC_POS_CONS_WEIGHT_SPINNER), FALSE);
							ICustButton *iPickOb1;
							iPickOb1	= GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_POS_TARG));
							if (iPickOb1 != NULL){
								iPickOb1->Enable(FALSE);		
								ReleaseICustButton(iPickOb1);
							}
						}
					}

					else if (LOWORD(wParam) == IDC_POS_TARG_PICKNODE){
						thePickMode.p  = p;					
						p->ip->SetPickMode(&thePickMode);
					}

					else if (LOWORD(wParam) == IDC_REMOVE_POS_TARG){
						selection = SendDlgItemMessage(p->hWnd, IDC_POS_TARG_LIST, LB_GETCURSEL, 0, 0);
						p->last_selection = selection;
						if (selection >= 0){
							theHold.Begin();
							p->DeleteTarget(selection);
							theHold.Accept(GetString(IDS_AG_POSITION_LIST));
						}
					}

				
				break;


				case CC_SPINNER_CHANGE:
				
					if (LOWORD(wParam) == IDC_POS_CONS_WEIGHT_SPINNER) {
						selection = SendDlgItemMessage(p->hWnd, IDC_POS_TARG_LIST, LB_GETCURSEL, 0, 0);

						// CAL-09/10/02: need to start a hold if it's not holding to handle type-in values
						BOOL isHold = theHold.Holding();
						if (!isHold) theHold.Begin();

						spin = GetISpinner(GetDlgItem(hWnd, IDC_POS_CONS_WEIGHT_SPINNER));

						if (selection >= 0) {
							float value = ((ISpinnerControl *)lParam)->GetFVal();
							map->GetParamBlock()->SetValue(position_target_weight, t, value, selection);

							Control *cont = p->pblock->GetController(position_target_weight, selection); // until it is animated, paramblock doesn't have a controller assigned in it yet.
							if (spin)
								spin->SetKeyBrackets(((cont != NULL) && cont->IsKeyAtTime(t,KEYAT_POSITION)) ? TRUE : FALSE);
							
							p->RedrawListbox(GetCOREInterface()->GetTime(), selection);
						} else {
							if (spin) spin->SetValue(0.0f, FALSE);
						}

						if (spin) ReleaseISpinner(spin);

						if (!isHold) theHold.Accept(GetString(IDS_RB_PARAMETERCHANGE));
					}
				
				break;
			}
			return FALSE;
		}

		void SetParamBlock(IParamBlock2* pb) 
		{ 
			UpdatePosName((PosConstPosition*)pb->GetOwner());
		}

		void DeleteThis() { }
};

	void PosConstPosition::RedrawListbox(TimeValue t, int sel)
		{
			if (hWnd == NULL) return;
			if (!ip || editCont != this) return;
			if(!posCD.NumParamMaps()) return;
			int selection = SendDlgItemMessage(hWnd, IDC_POS_TARG_LIST, LB_GETCURSEL, 0, 0);
			SendDlgItemMessage(hWnd, IDC_POS_TARG_LIST, LB_RESETCONTENT, 0, 0);
			int ts = 64;  
			SendDlgItemMessage(hWnd, IDC_POS_TARG_LIST, LB_SETTABSTOPS, 1, (LPARAM)&ts);
			int ct = pblock->Count(position_target_list);
			int ctf = pblock->Count(position_target_weight);
			if (ct != ctf) return;		// CAL-09/10/02: In the middle of changing table size.

			for (int i = 0; i < ct; i++){

				if (pblock->GetINode(position_target_list, t, i) != NULL){	
					TSTR str;
					str.printf(_T("%-s\t%-d"), 
						pblock->GetINode(position_target_list, t, i)->GetName(),
						(int)pblock->GetFloat(position_target_weight, t, i));
					SendDlgItemMessage(hWnd, IDC_POS_TARG_LIST, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR) str.data());			
				}
			}

			if (ct > 0){
			
				if (sel >= 0){
					SendDlgItemMessage(hWnd, IDC_POS_TARG_LIST, LB_SETCURSEL, sel, 0);
				}
				else {
					if (selection >= 0){
						SendDlgItemMessage(hWnd, IDC_POS_TARG_LIST, LB_SETCURSEL, selection, 0);
						last_selection = selection;
					}

					else {
						SendDlgItemMessage(hWnd, IDC_POS_TARG_LIST, LB_SETCURSEL, 0, 0);
						last_selection = 0;
					}
				}
	

				ISpinnerControl* spin = GetISpinner(GetDlgItem(hWnd, IDC_POS_CONS_WEIGHT_SPINNER));
				if (last_selection >=0 && spin != NULL){
					float value = pblock->GetFloat(position_target_weight, GetCOREInterface()->GetTime(), last_selection);
					spin->SetValue(value, FALSE);
				}
			}
			HWND hListBox = GetDlgItem(hWnd, IDC_POS_TARG_LIST);
			int extent = computeHorizontalExtent(hListBox, TRUE, 1, &ts);
			SendMessage(hListBox, LB_SETHORIZONTALEXTENT, extent, 0);
			UpdateWindow(hWnd);
		}


static PosConstDlgProc posConstDlgProc;



static ParamBlockDesc2 pos_const_paramblk (pos_const_params, _T("PosConsParameters"),  0, &posCD, P_AUTO_CONSTRUCT + P_AUTO_UI, POSPOS_PBLOCK_REF, 
	//rollout
	IDD_POS_CONST_PARAMS, IDS_AG_POSITIONPARAMS, BEGIN_EDIT_MOTION, 0, &posConstDlgProc,
	// params

	position_target_weight, _T("weight"), 	TYPE_FLOAT_TAB, 0,  P_ANIMATABLE + P_VARIABLE_SIZE + P_TV_SHOW_ALL, 	IDS_AG_POSITION_WEIGHT_LIST, 
		p_default, 		50.0f, 
		p_range, 		0.0f, 100.0f, 
		p_accessor,		&pos_const_accessor,	
		end, 

	position_target_list,		_T(""),		TYPE_INODE_TAB, 0,	P_VARIABLE_SIZE,	IDS_AG_POSITION_LIST,
		p_accessor,		&pos_const_accessor,	
		end,

	position_relative, _T("relative"),		TYPE_BOOL,		P_RESET_DEFAULT,	IDS_AG_POS_CONS_RELATIVE,
		p_default, 		FALSE, 
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_POS_CONS_RELATIVE, 
		p_accessor,		&pos_const_accessor,
		end,

	end
	);



PosConstPosition::PosConstPosition(BOOL loading) 
	{
	Point3 zero(0.0f, 0.0f, 0.0f);
	curval   = Point3(0,0,0);
	oldTargetNumber = 0;
	InitialPosition = zero; 
	flags      = 0;
	basePointWorld = zero;
	basePointLocal = zero;
	last_selection =  -1;
	hWnd = NULL;
	
	// make the paramblock
	posCD.MakeAutoParamBlocks(this);
	pblock->CallSets();
	ivalid.SetEmpty();
	validator.mod = this; 
	initialPositionFlag = 0;

}

PosConstPosition::~PosConstPosition()
	{
	DeleteAllRefsFromMe();
	}

RefTargetHandle PosConstPosition::Clone(RemapDir& remap) // gets called when the controller gets copied
	{
	PosConstPosition *p = new PosConstPosition(TRUE);

    p->ReplaceReference(POSPOS_PBLOCK_REF, pblock->Clone(remap));

	p->flags      = flags;
	p->curval     = curval;
	p->basePointWorld  = basePointWorld;
	p->basePointLocal  = basePointLocal;
	p->InitialPosition  = InitialPosition;
	p->oldTargetNumber = oldTargetNumber;
	p->ivalid.SetEmpty(); // forcing the cloned object to re-evaluate
	BaseClone(this, p, remap);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	return p;
	}


void PosConstPosition::Copy(Control *from) // gets called when my controller is applied over the top of another controller
	{
	Point3 fval;
	if (from->ClassID()==ClassID()) {
		PosConstPosition *ctrl = (PosConstPosition*)from;
		// a copy will construct its own pblock to keep the pblock-to-owner 1-to-1.
		RemapDir *remap = NewRemapDir(); 
		ReplaceReference(POSPOS_PBLOCK_REF, ctrl->pblock->Clone(*remap));
		remap->DeleteThis();
		curval   = ctrl->curval;
		flags    = ctrl->flags;
	} else {
		from->GetValue(GetCOREInterface()->GetTime(), &fval, FOREVER);	// to know the object position before the
		initialPositionFlag = 1;
		basePointLocal = fval;
		}
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}


void PosConstPosition::Update(TimeValue t){
	ivalid = FOREVER;
	Matrix3 otm;
	
	int ct = pblock->Count(position_target_list);
	int ctf = pblock->Count(position_target_weight);
	if (ctf != ct){
		pblock->SetCount(position_target_weight, ct);
		RedrawListbox(t);
	}

	curval = Point3(0.0f, 0.0f, 0.0f);
	float total_pos_target_weight = 0.0f;

	for (int i = 0; i < ct; i++) {

		INode *pos_target;
		float pos_targ_Wt = 0.0f;
		pblock->GetValue(position_target_list, t, pos_target, FOREVER, i);

		if (pos_target == NULL) continue; // skip the for loop if the selected object is null  

		pblock->GetValue(position_target_weight, t, pos_targ_Wt, ivalid, i);
		total_pos_target_weight += pos_targ_Wt;

		curval += pos_targ_Wt * pos_target->GetNodeTM(t, &ivalid).GetTrans();
		
	} 

	if (total_pos_target_weight > 0.0){
		curval =  curval/total_pos_target_weight;
		if (oldTargetNumber != ct) {
			InitialPosition = curval;
		}
		if (Relative()){
			curval = curval + (basePointWorld - InitialPosition);
		}
	}
	else {
		curval = basePointWorld;
	}

	oldTargetNumber = ct;
	if (ivalid.Empty()) ivalid.SetInstant(t);
}


void PosConstPosition::GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
	if (initialPositionFlag) {
		Matrix3 tempMat(1);
		// CAL-9/26/2002: in absolute mode the value could be un-initialized (random value).
		if (method == CTRL_RELATIVE) tempMat = *(Matrix3*)val;
		basePointWorld =  basePointLocal * tempMat;
		initialPositionFlag = 0;
	}
	if (!ivalid.InInterval(t)) 
		Update(t);
	valid &= ivalid;
			 
	if (method==CTRL_RELATIVE) {
  		Matrix3 *mat = (Matrix3*)val;
		mat->SetTrans(curval);		
	} else {
		*((Point3*)val) = curval;
		}

	}

RefTargetHandle PosConstPosition::GetReference(int i)
	{
		switch (i)
		{
			case POSPOS_PBLOCK_REF:
				return pblock;
		}
		return NULL;
	}

void PosConstPosition::SetReference(int i, RefTargetHandle rtarg)
	{
		switch (i)
		{
			case POSPOS_PBLOCK_REF:
				pblock = (IParamBlock2*)rtarg; break;
		}
	}

RefResult PosConstPosition::NotifyRefChanged(
		Interval iv, 
		RefTargetHandle hTarg, 
		PartID& partID, 
		RefMessage msg) 
	{
	switch (msg) {
		case REFMSG_CHANGE:

			if (hTarg == pblock) {
				ParamID changing_param = pblock->LastNotifyParamID();
				switch (changing_param)
				{
					// CAL-09/10/02: need to redraw the list box specifically.
					// TODO: Most of the other calls to RedrawListbox() should be removed with this addition.
					case position_target_weight:
						RedrawListbox(GetCOREInterface()->GetTime());
						break;
					default:
						pos_const_paramblk.InvalidateUI(changing_param);
						break;
				}
				ivalid.SetEmpty();
			}
		break;

		case REFMSG_OBJECT_CACHE_DUMPED:
			
			return REF_STOP;

		break;
		}
	return REF_SUCCEED;
	}


class PickPathRestore : public RestoreObj {
	public:
		PosConstPosition *cont;
		PickPathRestore(PosConstPosition *c) {cont=c;}
		void Restore(int isUndo) {
			if (cont->editCont == cont) {
				pos_const_paramblk.InvalidateUI();
				}									
			}
		void Redo() {
		}
	};

int PosConstPosition::GetNumTargets(){
	return pblock->Count(position_target_list);
}

int PosConstPosition::GetnonNULLNumPosTargets(){
	int no_of_nonNULL_nodes = 0;
	for (int i = 0; i < pblock->Count(position_target_list); ++i){
		if (pblock->GetINode(position_target_list, GetCOREInterface()->GetTime(), i) != NULL){
			no_of_nonNULL_nodes = no_of_nonNULL_nodes + 1;
		}			
	}
	return no_of_nonNULL_nodes;
}

INode* PosConstPosition :: GetNode(int nodeNumber){
	if (nodeNumber >= 0 && nodeNumber < pblock->Count(position_target_list)){
		INode *pos_targ;
		pblock->GetValue(position_target_list, 0, pos_targ, FOREVER, nodeNumber);
		return pos_targ;
	}
	return NULL;
}

BOOL PosConstPosition::AppendTarget(INode *target, float weight){

	if (!(target->TestForLoop(FOREVER,(ReferenceMaker *) this)!=REF_SUCCEED)){

		for (int i = 0; i < pblock->Count(position_target_list); ++i){
			if (target == pblock->GetINode(position_target_list, GetCOREInterface()->GetTime(), i)){
				return FALSE;
			}
		}

		theHold.Begin();
		pblock->Append(position_target_list, 1, &target, 1);
		pblock->Append(position_target_weight, 1, &weight, 1);
		theHold.Accept(GetString(IDS_AG_POSITION_LIST));
		return TRUE;
	}
	return FALSE;

}

BOOL PosConstPosition::DeleteTarget(int selection){
	if (selection >= 0 && selection < pblock->Count(position_target_list)){
//		theHold.Begin();
		pblock->Delete(position_target_weight, selection, 1);
		pblock->Delete(position_target_list, selection, 1);
//		theHold.Accept(GetString(IDS_AG_POSITION_LIST));
		return TRUE;
	}
	else
		return FALSE;
}


float PosConstPosition::GetTargetWeight(int targetNumber){
	int ct = pblock->Count(position_target_list);
	if (targetNumber >= 0 && targetNumber < ct){
		float posWt;
		pblock->GetValue(position_target_weight, GetCOREInterface()->GetTime(), posWt, FOREVER, targetNumber);
		return posWt;
	}
	else{
		return 0.0f;
	}
}

BOOL PosConstPosition::SetTargetWeight(int targetNumber, float weight){
	int ct = pblock->Count(position_target_list);
	if (targetNumber >= 0 && targetNumber < ct){
		pblock->SetValue(position_target_weight, GetCOREInterface()->GetTime(), weight, targetNumber);
		return TRUE;
	}
	else{
		return FALSE;
	}
}

void PosConstPosition::SetRelative(BOOL rel)
	{
	pblock->SetValue(position_relative, GetCOREInterface()->GetTime(), rel);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}

BOOL PosConstPosition::Relative(){
	Interval iv;
	BOOL isRelative;
	pblock->GetValue(position_relative, GetCOREInterface()->GetTime(), isRelative, iv);
	return isRelative;
}


/*--------------------------------------------------------------------*/
// PosConstPosition UI

void PosConstPosition::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
	{	
	this->ip = ip;
	editCont = this;
	if (flags & BEGIN_EDIT_HIERARCHY) {
		InterpCtrlUI *ui;
		ui = new InterpCtrlUI(NULL,ip,this);
		DWORD f=0;	
		SetProperty(PROPID_INTERPUI,ui);		
	} 
	else {
		posCD.BeginEditParams(ip, this, flags, prev);
	}

	ip->RegisterTimeChangeCallback(&posConstTimeChangeCallback);
	posConstTimeChangeCallback.pcontroller = this;

	IParamMap2* pmap = pblock->GetMap();
	if (pmap) hWnd = pmap->GetHWnd();

	int ct = pblock->Count(position_target_list);
	if (last_selection < 0){
		RedrawListbox(GetCOREInterface()->GetTime());
	}
	else {
		RedrawListbox(GetCOREInterface()->GetTime(), last_selection);
		ISpinnerControl* spin = GetISpinner(GetDlgItem(hWnd, IDC_POS_CONS_WEIGHT_SPINNER));
		if (spin != NULL){
			float value = pblock->GetFloat(position_target_weight, GetCOREInterface()->GetTime(), last_selection);
			spin->SetValue(value, FALSE);
		}
		ReleaseISpinner(spin);
	}


}

void PosConstPosition::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
{
	editCont = NULL;
	IParamMap2* pmap = pblock->GetMap();
	if (pmap != NULL)
	{
		if (next && next->ClassID() == ClassID() && ((PosConstPosition*)next)->pblock)
		{
			pmap->SetParamBlock(((PosConstPosition*)next)->pblock);
			ip->ClearPickMode();
		}
		else
			posCD.EndEditParams(ip, this, flags | END_EDIT_REMOVEUI, next);
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
		ip->UnRegisterTimeChangeCallback(&posConstTimeChangeCallback);
		ip->ClearPickMode(); // need this, otherwise will crash on undo, while pickmode is active.
		this->ip = NULL;
		hWnd = NULL;
}

TSTR PosConstPosition::SvGetRelTip(IGraphObjectManager *gom, IGraphNode *gNodeTarger, int id, IGraphNode *gNodeMaker)
{
	return SvGetName(gom, gNodeMaker, false) + " -> " + gNodeTarger->GetAnim()->SvGetName(gom, gNodeTarger, false);
}

bool PosConstPosition::SvCanDetachRel(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker)
{
	return true;
}

bool PosConstPosition::SvDetachRel(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker)
{
	for( int i=0; i<GetNumTargets(); i++ ) {
		if( GetNode(i) == gNodeTarget->GetAnim() ) {
			DeleteTarget(i);
			return true;
		}
	}
	return false;
}

bool PosConstPosition::SvCanConcludeLink(IGraphObjectManager *gom, IGraphNode *gNode, IGraphNode *gNodeChild)
{
	Animatable *pChildAnim = gNodeChild->GetAnim();
	if( pChildAnim->SuperClassID() == BASENODE_CLASS_ID )
		return true;

	return Control::SvCanConcludeLink(gom, gNode, gNodeChild);
}

bool PosConstPosition::SvLinkChild(IGraphObjectManager *gom, IGraphNode *gNodeThis, IGraphNode *gNodeChild)
{
	Animatable *pChildAnim = gNodeChild->GetAnim();
	if( pChildAnim->SuperClassID() == BASENODE_CLASS_ID ) {
		if( AppendTarget( (INode*)pChildAnim ) ) {
			NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
			return true;
		}
	}
	return Control::SvLinkChild(gom, gNodeThis, gNodeChild);
}

SvGraphNodeReference PosConstPosition::SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags)
{
	SvGraphNodeReference nodeRef = Control::SvTraverseAnimGraph( gom, owner, id, flags );

	if( nodeRef.stat == SVT_PROCEED ) {
		for( int i=0; i<GetNumTargets(); i++ ) {
			gom->AddRelationship( nodeRef.gNode, GetNode(i), i, RELTYPE_CONSTRAINT );
		}
	}

	return nodeRef;
}

int PosConstPosition::SetProperty(ULONG id, void *data)
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
	} else {
		return Animatable::SetProperty(id,data);
		}
	}

void* PosConstPosition::GetProperty(ULONG id)
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

#define OFFSET_POS_LOCAL_CHUNK		0x1001
#define OFFSET_POS_WORLD_CHUNK		0x1002
#define INITIAL_POSITION_CHUNK		0x1003
#define OLD_TARGET_NUMBER_CHUNK		0x1004


IOResult PosConstPosition::Save(ISave *isave)
	{	

		ULONG nb;

		// Save basePointLocal
		isave->BeginChunk(OFFSET_POS_LOCAL_CHUNK);
		isave->Write(&basePointLocal, sizeof(basePointLocal), &nb);
		isave->EndChunk();

		// Save basePointWorld
		isave->BeginChunk(OFFSET_POS_WORLD_CHUNK);
		isave->Write(&basePointWorld, sizeof(basePointWorld), &nb);
		isave->EndChunk();

		// Save InitialPosition
		isave->BeginChunk(INITIAL_POSITION_CHUNK);
		isave->Write(&InitialPosition,sizeof(InitialPosition),&nb);	
		isave->EndChunk();

		// Save oldTargetNumber
		isave->BeginChunk(OLD_TARGET_NUMBER_CHUNK);
		isave->Write(&oldTargetNumber,sizeof(oldTargetNumber),&nb);	
		isave->EndChunk();
	
	return IO_OK;
	}

IOResult PosConstPosition::Load(ILoad *iload)
{
	
	ULONG nb;
	IOResult res = IO_OK;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			
			case OFFSET_POS_LOCAL_CHUNK:
				res=iload->Read(&basePointLocal, sizeof(basePointLocal), &nb);
			break;

			case OFFSET_POS_WORLD_CHUNK:
				res=iload->Read(&basePointWorld, sizeof(basePointWorld), &nb);
			break;

			case INITIAL_POSITION_CHUNK:
				res=iload->Read(&InitialPosition, sizeof(InitialPosition), &nb);
			break;

			case OLD_TARGET_NUMBER_CHUNK:
				res=iload->Read(&oldTargetNumber, sizeof(oldTargetNumber), &nb);
			break;
			
		}		
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
	}
	
	return IO_OK;
}
