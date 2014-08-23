/**********************************************************************
 *
	FILE: orientation_cnstrnt.cpp

	DESCRIPTION: A rotation constraint controller that controls the orientation 
				of an object based on the orientation of the target objects

	CREATED BY: Ambarish Goswami 5/2000
	            

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "ctrl.h"
#include "units.h"
#include "interpik.h"
#include "istdplug.h"
#include "iparamm2.h"
#include "decomp.h"


#define ORIENT_CONTROL_CNAME		GetString(IDS_AG_ORIENTATION)
#define PB_LENGTH 0


enum {	orientation_target_weight,	orientation_target_list,	orientation_relative, orientation_lw, 
};


class OrientConstRotation; 

class OrientConstTimeChangeCallback : public TimeChangeCallback
{
	public:
		OrientConstRotation* orient_controller;
		void TimeChanged(TimeValue t);
};

class orientConstValidatorClass : public PBValidator
{
	public:
		OrientConstRotation *mod;
	private:
		BOOL Validate(PB2Value &v);
};


class OrientConstRotation : public	IOrientConstRotation  
{
	public:
		orientConstValidatorClass validator;							
		void RedrawListbox(TimeValue t, int sel = -1);
		HWND hWnd;
		int last_selection, oldTargetNumber;
		
		OrientConstTimeChangeCallback orientConstTimeChangeCallback;	

		IParamBlock2* pblock;
		INode *orient_t;

		DWORD flags;
		Interval ivalid;
		Quat curRot, baseRotQuatLocal, baseRotQuatWorld;
		Point3 baseEuler;
		int pickWorldFlag, startFlag, firstTimeFlag;
		Quat InitialOrientQuat;


		static OrientConstRotation *editCont;
		static IObjParam *ip;

		OrientConstRotation(const OrientConstRotation &ctrl);
		OrientConstRotation(BOOL loading=FALSE);

		~OrientConstRotation();

		int GetNumTargets();
		INode* GetNode(int targetNumber);
		float GetTargetWeight(int targetNumber);
		BOOL SetTargetWeight(int targetNumber, float weight);	
		BOOL AppendTarget(INode *target, float weight=50.0);
		BOOL DeleteTarget(int selection);
		BOOL GetRelative() {return Relative();}	
		void SetRelative(BOOL rel);	
		BOOL Relative();
		int IsLocal();

		void Update(TimeValue t);

		// Animatable methods
		Class_ID ClassID() { return Class_ID(ORIENTATION_CONSTRAINT_CLASS_ID, 0); }  
		SClass_ID SuperClassID() { return CTRL_ROTATION_CLASS_ID; }  		
		
		void GetClassName(TSTR& s) {s = ORIENT_CONTROL_CNAME;}
		void DeleteThis() {delete this;} // intellgent destructor
		int IsKeyable() {return 0;}		// if the controller is keyable (for trackview)?

		int NumSubs()  {return 1;} //because it uses the paramblock
		Animatable* SubAnim(int i) {return pblock;}
		TSTR SubAnimName(int i) { return GetString(IDS_AG_ORIENTATIONPARAMS); }
		int SubNumToRefNum(int subNum) {if (subNum==0) return ORIENT_ROT_PBLOCK_REF; else return -1;}

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

		//	Function Publishing method (Mixin Interface)
		BaseInterface* GetInterface(Interface_ID id) 
		{ 
			if (id == ORIENT_CONSTRAINT_INTERFACE) 
				return (OrientConstRotation*)this; 
			else 
				return FPMixinInterface::GetInterface(id);
		}
};

// CAL-05/24/02: TODO: this should really go to core\decomp.cpp, and it should be optimized.
//		For now, it's defined locally in individual files in the ctrl project.
static void comp_affine( const AffineParts &ap, Matrix3 &mat )
{
	Matrix3 tm;
	
	mat.IdentityMatrix();
	mat.SetTrans( ap.t );

	if ( ap.f != 1.0f ) {				// has f component
		tm.SetScale( Point3( ap.f, ap.f, ap.f ) );
		mat = tm * mat;
	}

	if ( !ap.q.IsIdentity() ) {			// has q rotation component
		ap.q.MakeMatrix( tm );
		mat = tm * mat;
	}
	
	if ( ap.k.x != 1.0f || ap.k.y != 1.0f || ap.k.z != 1.0f ) {		// has k scale component
		tm.SetScale( ap.k );
		if ( !ap.u.IsIdentity() ) {			// has u rotation component
			Matrix3 utm;
			ap.u.MakeMatrix( utm );
			mat = Inverse( utm ) * tm * utm * mat;
		} else {
			mat = tm * mat;
		}
	}
}


BOOL orientConstValidatorClass::Validate(PB2Value &v)
{
	INode *node = (INode*) v.r;

	for (int i = 0; i < mod->pblock->Count(orientation_target_list); i++){
		if (node == mod->pblock->GetINode(orientation_target_list, 0, i))
			return FALSE;
	}

	if (node->TestForLoop(FOREVER,(ReferenceMaker *) mod)!=REF_SUCCEED) 
	{
		return FALSE;
	}
	return TRUE;
}

void OrientConstTimeChangeCallback :: TimeChanged(TimeValue t){
	int selection = SendDlgItemMessage(orient_controller->hWnd, IDC_ORIENT_TARG_LIST, LB_GETCURSEL, 0, 0);
	ISpinnerControl* spin = GetISpinner(GetDlgItem(orient_controller->hWnd, IDC_ORIENT_CONS_WEIGHT_SPINNER));
	spin = GetISpinner(GetDlgItem(orient_controller->hWnd, IDC_ORIENT_CONS_WEIGHT_SPINNER));

	Control *cont = orient_controller->pblock->GetController(orientation_target_weight, selection); 
	// until it is animated, paramblock doesn't have a controller assigned in it yet.
	if (spin && (cont !=NULL)) {
		if (cont->IsKeyAtTime(t,KEYAT_ROTATION)) {
			spin->SetKeyBrackets(TRUE);
		} 
		else {
			spin->SetKeyBrackets(FALSE);
		}
	}
	ReleaseISpinner(spin);

	orient_controller->RedrawListbox(t);
}

IObjParam *OrientConstRotation::ip						= NULL;
OrientConstRotation *OrientConstRotation::editCont		= NULL;

//********************************************************
// ORIENTATION CONSTRAINT
//********************************************************
static Class_ID orientConstControlClassID(ORIENTATION_CONSTRAINT_CLASS_ID,0); 
class OrientConstClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new OrientConstRotation(loading); }
	const TCHAR *	ClassName() { return ORIENT_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_ROTATION_CLASS_ID; }
	Class_ID		ClassID() { return orientConstControlClassID; }
	const TCHAR* 	Category() { return _T("");  }
	const TCHAR*	InternalName() { return _T("Orientation_Constraint"); }// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
	};

static OrientConstClassDesc orientCD;
ClassDesc* GetOrientConstDesc() {return &orientCD;}

//--- CustMod dlg proc ---------DOWN DOWN DOWN----from Modifiers/clstmode.cpp---------------


class PickOrientNode : public PickModeCallback,
		public PickNodeCallback {
	public:			
		OrientConstRotation *p;
		PickOrientNode() {p = NULL;}
		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);		
		BOOL Pick(IObjParam *ip,ViewExp *vpt);		
		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);		
		BOOL Filter(INode *node);
		PickNodeCallback *GetFilter() {return this;}
		BOOL RightClick(IObjParam *ip,ViewExp *vpt) {return TRUE;}
	};
static PickOrientNode thePickMode;

BOOL PickOrientNode::Filter(INode *node){

	for (int i = 0; i < p->pblock->Count(orientation_target_list); i++){
		if (node == p->pblock->GetINode(orientation_target_list, 0, i)) 
			return FALSE;
	}

	if (node->TestForLoop(FOREVER,(ReferenceMaker *) p)!=REF_SUCCEED) 
	{
		return FALSE;
	}

	return TRUE;
}

BOOL PickOrientNode::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
	{	
	if (ip->PickNode(hWnd,m,this)) {
		return TRUE;
	} else {
		return FALSE;
		}
	}

BOOL PickOrientNode::Pick(IObjParam *ip,ViewExp *vpt)
	{

	INode *node = vpt->GetClosestHit();
	if (node) {
		p->AppendTarget(node);
		p->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		p->ip->RedrawViews(GetCOREInterface()->GetTime());
	}
	return FALSE;
	}

void PickOrientNode::EnterMode(IObjParam *ip)
	{
	ICustButton *iBut = GetICustButton(GetDlgItem(p->hWnd,IDC_ORIENT_TARG_PICKNODE));
	if (iBut) iBut->SetCheck(TRUE);
	ReleaseICustButton(iBut);
	GetCOREInterface()->PushPrompt("Select Orientation Target Object");
	}

void PickOrientNode::ExitMode(IObjParam *ip)
	{
	ICustButton *iBut = GetICustButton(GetDlgItem(p->hWnd,IDC_ORIENT_TARG_PICKNODE));
	if (iBut) iBut->SetCheck(FALSE);
	ReleaseICustButton(iBut);
	GetCOREInterface()->PopPrompt();
	}


//--- CustMod dlg proc -----------UP UP UP -------------------


//Function Publishing descriptor for Mixin interface
//Added by Ambarish Goswami (5/18/2000)
//*****************************************************


  static FPInterfaceDesc orient_constraint_interface(
    ORIENT_CONSTRAINT_INTERFACE, _T("constraints"), 0, &orientCD, 0,
		IOrientConstRotation::get_num_targets, _T("getNumTargets"), 0, TYPE_INDEX, 0,0,
		IOrientConstRotation::get_node, _T("getNode"), 0, TYPE_INODE, 0, 1,
			_T("targetNumber"), 0, TYPE_INDEX,
		IOrientConstRotation::get_target_weight, _T("getWeight"), 0, TYPE_FLOAT, 0, 1,
			_T("targetNumber"), 0, TYPE_INDEX,
		IOrientConstRotation::set_target_weight, _T("setWeight"), 0, TYPE_BOOL, 0, 2,
			_T("targetNumber"), 0, TYPE_INDEX,
			_T("weight"), 0, TYPE_FLOAT,
		IOrientConstRotation::append_target, _T("appendTarget"), 0, TYPE_VOID, 0, 2,
			_T("target"), 0, TYPE_INODE,
			_T("weight"), 0, TYPE_FLOAT,
		IOrientConstRotation::delete_target, _T("deleteTarget"), 0, TYPE_BOOL, 0, 1,
			_T("targetNumber"), 0, TYPE_INDEX,
		end
		);



//  Get Descriptor method for Mixin Interface
//  *****************************************
FPInterfaceDesc* IOrientConstRotation::GetDesc()
{
     return &orient_constraint_interface;
}

//*********************************************
// End of Function Publishing Code

// ParamBlock IDs
// main block
enum { orient_const_params, };

// parameter setter callback, reflect any ParamBlock-mediated param setting in instance data members.

class OrientConstPBAccessor : public PBAccessor
{ 
public:

	void TabChanged(tab_changes changeCode, Tab<PB2Value>* tab, 
                  ReferenceMaker* owner, ParamID id, int tabIndex, int count) 
		{ 

		if ((id == orientation_target_weight) || (id == orientation_target_list)){

			OrientConstRotation* p = (OrientConstRotation*)owner;
			int ct = p->pblock->Count(orientation_target_list);
			int ctf = p->pblock->Count(orientation_target_weight);
			if (changeCode == tab_ref_deleted){		  
				int j = p->pblock->Delete(orientation_target_list, tabIndex, 1); // deletes the node from nodelist;
				int k = p->pblock->Delete(orientation_target_weight, tabIndex, 1); // deletes corresponding weight;
			}
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
				
//				int selection = SendDlgItemMessage(p->hWnd, IDC_ORIENT_TARG_LIST, LB_GETCURSEL, 0, 0);
	
				if (p->last_selection < 0 || p->pblock->Count(orientation_target_list) < 1){ 
//					nothing is selected OR there are no targets
//					all buttons - target_delete, weight_edit, weight_spinner -- are disabled
					EnableWindow(GetDlgItem(p->hWnd, IDC_ORIENT_CONS_WEIGHT_EDIT), FALSE);
					EnableWindow(GetDlgItem(p->hWnd, IDC_ORIENT_CONS_WEIGHT_SPINNER), FALSE);
					ICustButton *iPickOb1;
					iPickOb1= GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_ORIENT_TARG));
					if (iPickOb1 != NULL){
						iPickOb1->Enable(FALSE);		
						ReleaseICustButton(iPickOb1);
					}
				}
				else if (p->last_selection >= 0) {	
//					there is a valid selection 
//					automatically means there is AT LEAST one target
					if (p->pblock->Count(orientation_target_list) == 1){ // there is only one target
//						the "delete" button should be enabled
						ICustButton *iPickOb1;
						iPickOb1 = GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_ORIENT_TARG));
						if (iPickOb1 != NULL){
							iPickOb1->Enable(TRUE);		
							ReleaseICustButton(iPickOb1);
						}
//						the weight_edit, weight_spinner buttons are disabled
						EnableWindow(GetDlgItem(p->hWnd, IDC_ORIENT_CONS_WEIGHT_EDIT), FALSE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_ORIENT_CONS_WEIGHT_SPINNER), FALSE);
					}
					else if (p->pblock->Count(orientation_target_list) > 1){ // there are more than one targets
						
//						all buttons - target_delete, weight_edit, weight_spinner -- are enabled
						EnableWindow(GetDlgItem(p->hWnd, IDC_ORIENT_CONS_WEIGHT_EDIT), TRUE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_ORIENT_CONS_WEIGHT_SPINNER), TRUE);
						ICustButton *iPickOb1;
						iPickOb1 = GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_ORIENT_TARG));
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
		OrientConstRotation* p = (OrientConstRotation*)owner;
		if (id == orientation_target_weight){
			if ((v.f) < 0.0f) v.f = 0.0f; 
		}
	}

	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		OrientConstRotation* p = (OrientConstRotation*)owner;
		if (id == orientation_target_list){
			IParamMap2* pmap = p->pblock->GetMap();	
		}	
	}
};

static OrientConstPBAccessor orient_const_accessor;
class OrientConstDlgProc : public ParamMap2UserDlgProc 
{
	public:
		void UpdateOrientName(OrientConstRotation* p)
		{
			IParamMap2* pmap = p->pblock->GetMap();
		}

		BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{	

			OrientConstRotation* p = (OrientConstRotation*)map->GetParamBlock()->GetOwner();
			UpdateOrientName(p);
			p->hWnd = hWnd;
			int ct = p->pblock->Count(orientation_target_list);
//			int ctf = p->pblock->Count(orientation_target_weight);

			switch (msg) 
			{
				case WM_INITDIALOG:
				{
					int selection = p->last_selection;

					ICustButton *iBut = GetICustButton(GetDlgItem(hWnd,IDC_ORIENT_TARG_PICKNODE));
					iBut->SetType(CBT_CHECK);
					iBut->SetHighlightColor(GREEN_WASH);
					ReleaseICustButton(iBut);
					
					ISpinnerControl* spin = SetupFloatSpinner(hWnd, IDC_ORIENT_CONS_WEIGHT_SPINNER, IDC_ORIENT_CONS_WEIGHT_EDIT, 0.0f, 100.0f, 50.0f, 1.0f);
					spin = GetISpinner(GetDlgItem(hWnd, IDC_ORIENT_CONS_WEIGHT_SPINNER));
					
					// CAL-09/10/02: automatically set last_selection to 0 when there're targets
					if (p->pblock->Count(orientation_target_list) < 1) { 
						// nothing is selected OR there are no targets
						// all buttons - target_delete, weight_edit, weight_spinner -- are disabled
						EnableWindow(GetDlgItem(p->hWnd, IDC_ORIENT_CONS_WEIGHT_EDIT), FALSE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_ORIENT_CONS_WEIGHT_SPINNER), FALSE);
						ICustButton *iPickOb1;
						iPickOb1	= GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_ORIENT_TARG));
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
						Control *cont = p->pblock->GetController(orientation_target_weight, selection); 
						// until it is animated, paramblock doesn't have a controller assigned in it yet.
						if (spin) {
							if ((cont != NULL) && cont->IsKeyAtTime(t,KEYAT_ROTATION))
								spin->SetKeyBrackets(TRUE);
							else
								spin->SetKeyBrackets(FALSE);
						}

						if(p->pickWorldFlag){
							ICustButton *iPickOb;
							iPickOb	= GetICustButton(GetDlgItem(p->hWnd, IDC_ORIENT_PICK_WORLD));
							if (iPickOb != NULL){
								iPickOb->Enable(FALSE);
								ReleaseICustButton(iPickOb);
							}

						}

						if (p->pblock->Count(orientation_target_list) == 1){// there is only one target
							// the "delete" button should be enabled
							ICustButton *iPickOb1;
							iPickOb1 = GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_ORIENT_TARG));
							if (iPickOb1 != NULL){
								iPickOb1->Enable(TRUE);		
								ReleaseICustButton(iPickOb1);
							}
							// the rest are disabled
//							EnableWindow(GetDlgItem(p->hWnd, IDC_ORIENT_CONS_WEIGHT_EDIT), FALSE);
//							EnableWindow(GetDlgItem(p->hWnd, IDC_ORIENT_CONS_WEIGHT_SPINNER), FALSE);
						}
						else if (p->pblock->Count(orientation_target_list) > 1){ 
							// there are more than one targets
							// all buttons - delete, weight_edit, weight_spinner -- are enabled
							EnableWindow(GetDlgItem(p->hWnd, IDC_ORIENT_CONS_WEIGHT_EDIT), TRUE);
							EnableWindow(GetDlgItem(p->hWnd, IDC_ORIENT_CONS_WEIGHT_SPINNER), TRUE);
							ICustButton *iPickOb1;
							iPickOb1 = GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_ORIENT_TARG));
							if (iPickOb1 != NULL){
								iPickOb1->Enable(TRUE);		
								ReleaseICustButton(iPickOb1);
							}
						}
					}
					ReleaseISpinner(spin);
					p->RedrawListbox(GetCOREInterface()->GetTime(), selection);
					return TRUE;	
				}
				break;


				case WM_COMMAND:
				{
					
					int selection;
					
					if (LOWORD(wParam) == IDC_ORIENT_TARG_LIST && HIWORD(wParam) == LBN_SELCHANGE)
					{
						selection = SendDlgItemMessage(p->hWnd, IDC_ORIENT_TARG_LIST, LB_GETCURSEL, 0, 0);
						p->last_selection = selection;
						if (selection >= 0){
						// there is a valid selection 
							// automatically means there is AT LEAST one target
							if (p->pblock->Count(orientation_target_list) == 1){ // there is only one target
								// the "delete" button should be enabled
								ICustButton *iPickOb1;
								iPickOb1 = GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_ORIENT_TARG));
								if (iPickOb1 != NULL){
									iPickOb1->Enable(TRUE);		
									ReleaseICustButton(iPickOb1);
								}
//								// the rest are disabled
//								EnableWindow(GetDlgItem(p->hWnd, IDC_ORIENT_CONS_WEIGHT_EDIT), FALSE);
//								EnableWindow(GetDlgItem(p->hWnd, IDC_ORIENT_CONS_WEIGHT_SPINNER), FALSE);
							}

							else  if (p->pblock->Count(orientation_target_list) > 1){ 
							// there are more than one targets
								// all buttons - delete, weight_edit, weight_spinner -- are enabled
								EnableWindow(GetDlgItem(p->hWnd, IDC_ORIENT_CONS_WEIGHT_EDIT), TRUE);
								EnableWindow(GetDlgItem(p->hWnd, IDC_ORIENT_CONS_WEIGHT_SPINNER), TRUE);
								ICustButton *iPickOb1;
								iPickOb1 = GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_ORIENT_TARG));
								if (iPickOb1 != NULL){
									iPickOb1->Enable(TRUE);		
									ReleaseICustButton(iPickOb1);
								}

								ISpinnerControl* spin = GetISpinner(GetDlgItem(hWnd, IDC_ORIENT_CONS_WEIGHT_SPINNER));
								float value = map->GetParamBlock()->GetFloat(orientation_target_weight, t, selection);
								spin->SetValue(value, FALSE);

								Control *cont = p->pblock->GetController(orientation_target_weight, selection); // until it is animated, paramblock doesn't have a controller assigned in it yet.
								if (spin && (cont != NULL)) {
									if (cont->IsKeyAtTime(t,KEYAT_ROTATION)){
										spin->SetKeyBrackets(TRUE);
									} 
								}
								else {
									spin->SetKeyBrackets(FALSE);
								}
								ReleaseISpinner(spin);
							}
							p->RedrawListbox(GetCOREInterface()->GetTime(), selection);							
						}
						
						else if (selection < 0 || p->pblock->Count(orientation_target_list) < 1){ 
						// nothing is selected OR there are no targets
							// all buttons - target_delete, weight_edit, weight_spinner -- are disabled
							EnableWindow(GetDlgItem(p->hWnd, IDC_ORIENT_CONS_WEIGHT_EDIT), FALSE);
							EnableWindow(GetDlgItem(p->hWnd, IDC_ORIENT_CONS_WEIGHT_SPINNER), FALSE);
							ICustButton *iPickOb1;
							iPickOb1	= GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_ORIENT_TARG));
							if (iPickOb1 != NULL){
								iPickOb1->Enable(FALSE);		
								ReleaseICustButton(iPickOb1);
							}
						}
					}

					else if (LOWORD(wParam) == IDC_ORIENT_TARG_PICKNODE){
						thePickMode.p  = p;					
						p->ip->SetPickMode(&thePickMode);
					}

					else if (LOWORD(wParam) == IDC_REMOVE_ORIENT_TARG){
						selection = SendDlgItemMessage(p->hWnd, IDC_ORIENT_TARG_LIST, LB_GETCURSEL, 0, 0);
						p->last_selection = selection;
						if (selection >= 0){
							theHold.Begin();
							p->DeleteTarget(selection);
							theHold.Accept(GetString(IDS_AG_ORIENTATION_LIST));
						}
//						p->RedrawListbox(GetCOREInterface()->GetTime(), selection-1);
					}

					else if (LOWORD(wParam) == IDC_ORIENT_PICK_WORLD){
						INode *nullNode = NULL;
						p->AppendTarget(nullNode);
						
						p->pickWorldFlag = 1;

						p->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE); // so that the change is updated
						ICustButton *iPickOb;
						iPickOb	= GetICustButton(GetDlgItem(p->hWnd, IDC_ORIENT_PICK_WORLD));
						if (iPickOb != NULL){
							iPickOb->Enable(FALSE);
							ReleaseICustButton(iPickOb);
						}
						p->RedrawListbox(GetCOREInterface()->GetTime());
					}

				
				}
				break;


				case CC_SPINNER_CHANGE:
				{
					if (LOWORD(wParam) == IDC_ORIENT_CONS_WEIGHT_SPINNER) {
						int selection = SendDlgItemMessage(p->hWnd, IDC_ORIENT_TARG_LIST, LB_GETCURSEL, 0, 0);

						// CAL-09/10/02: need to start a hold if it's not holding to handle type-in values
						BOOL isHold = theHold.Holding();
						if (!isHold) theHold.Begin();

						ISpinnerControl* spin = GetISpinner(GetDlgItem(hWnd, IDC_ORIENT_CONS_WEIGHT_SPINNER));

						if (selection >= 0) {
							float value = ((ISpinnerControl *)lParam)->GetFVal();
							map->GetParamBlock()->SetValue(orientation_target_weight, t, value, selection);

							Control *cont = p->pblock->GetController(orientation_target_weight, selection); // until it is animated, paramblock doesn't have a controller assigned in it yet.
							if (spin)
								spin->SetKeyBrackets(((cont != NULL) && cont->IsKeyAtTime(t,KEYAT_ROTATION)) ? TRUE : FALSE);

							p->RedrawListbox(GetCOREInterface()->GetTime(), selection);
						} else {
							if (spin) spin->SetValue(0.0f, FALSE);
						}

						if (spin) ReleaseISpinner(spin);

						if (!isHold) theHold.Accept(GetString(IDS_RB_PARAMETERCHANGE));
					}
				}			
				break;

			}
			return FALSE;
		}

		void SetParamBlock(IParamBlock2* pb) 
		{ 
			UpdateOrientName((OrientConstRotation*)pb->GetOwner());
		}
		void DeleteThis() { }
};

	void OrientConstRotation::RedrawListbox(TimeValue t,  int sel)
	{
		if (hWnd == NULL) return;
		if (!ip || editCont != this) return;
		int isWorldinListboxFlag = 0;
		if(!orientCD.NumParamMaps()) return;
		int selection = SendDlgItemMessage(hWnd, IDC_ORIENT_TARG_LIST, LB_GETCURSEL, 0, 0);
//		IParamBlock2* pb = orientCD.GetParamMap(0)->GetParamBlock();
		SendDlgItemMessage(hWnd, IDC_ORIENT_TARG_LIST, LB_RESETCONTENT, 0, 0);
		int ts = 64;  // appears smaller than necessary since for a large no of targets the vertical scroll bar needs space
		SendDlgItemMessage(hWnd, IDC_ORIENT_TARG_LIST, LB_SETTABSTOPS, 1, (LPARAM)&ts);
		int ct = pblock->Count(orientation_target_list);
		int ctf = pblock->Count(orientation_target_weight);
		if (ct != ctf) return;		// CAL-09/10/02: In the middle of changing table size.

		for (int i = 0; i < ct; i++){				
			TSTR str;
			INode *testNode;
			testNode = pblock->GetINode(orientation_target_list, t, i);
			if (testNode == NULL){
				str.printf( _T("%-s\t%-d"), // NOTE: for tab "\t" to use, check "use tabstops" in the resource listbox properties
				"World", (int)pblock->GetFloat(orientation_target_weight, t, i));
				isWorldinListboxFlag = 1;
				ICustButton *iPickOb;
				iPickOb	= GetICustButton(GetDlgItem(hWnd, IDC_ORIENT_PICK_WORLD));
				if (iPickOb != NULL){
					iPickOb->Enable(FALSE);
					ReleaseICustButton(iPickOb);
				}
			}
			else{
				float wwt = pblock->GetFloat(orientation_target_weight, t, i);
				TCHAR * nname = testNode->GetName();
				str.printf(_T("%-s\t%-d"), 
				testNode->GetName(),
				(int)pblock->GetFloat(orientation_target_weight, t, i));
			}
			SendDlgItemMessage(hWnd, IDC_ORIENT_TARG_LIST, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR) str.data());			
		}

		if(!isWorldinListboxFlag){
				ICustButton *iPickOb;
				iPickOb	= GetICustButton(GetDlgItem(hWnd, IDC_ORIENT_PICK_WORLD));
				if (iPickOb != NULL){
					iPickOb->Enable(TRUE);
					ReleaseICustButton(iPickOb);
				}
		}

		if (ct > 0){
			if (sel >= 0){
				SendDlgItemMessage(hWnd, IDC_ORIENT_TARG_LIST, LB_SETCURSEL, sel, 0);
			}
			else{
//		int selection = SendDlgItemMessage(hWnd, IDC_POS_TARG_LIST, LB_GETCURSEL, 0, 0);
				if (selection >= 0){
					SendDlgItemMessage(hWnd, IDC_ORIENT_TARG_LIST, LB_SETCURSEL, selection, 0);
					last_selection = selection;
				}
				else if (ct == 1){
					SendDlgItemMessage(hWnd, IDC_ORIENT_TARG_LIST, LB_SETCURSEL, 0, 0);
					last_selection = 0;
				}
			}
			
			ISpinnerControl* spin = GetISpinner(GetDlgItem(hWnd, IDC_ORIENT_CONS_WEIGHT_SPINNER));
			if (last_selection >= 0 && spin != NULL){
				float value = pblock->GetFloat(orientation_target_weight, GetCOREInterface()->GetTime(), last_selection);
				spin->SetValue(value, FALSE);
			}
			ReleaseISpinner(spin);
		}
		HWND hListBox = GetDlgItem(hWnd, IDC_ORIENT_TARG_LIST);
		int extent = computeHorizontalExtent(hListBox, TRUE, 1, &ts);
		SendMessage(hListBox, LB_SETHORIZONTALEXTENT, extent, 0);
		UpdateWindow(hWnd);
	}


//static HWND hParamGizmos;

static OrientConstDlgProc orientConstDlgProc;


static ParamBlockDesc2 orient_const_paramblk (orient_const_params, _T("OrientConsParameters"),  0, &orientCD, P_AUTO_CONSTRUCT + P_AUTO_UI, ORIENT_ROT_PBLOCK_REF, 
	//rollout
	IDD_ORIENT_CONST_PARAMS, IDS_AG_ORIENTATIONPARAMS, BEGIN_EDIT_MOTION, 0, &orientConstDlgProc,
	// params

	orientation_target_weight, 	_T("weight"), 	TYPE_FLOAT_TAB, 0,  P_ANIMATABLE + P_VARIABLE_SIZE + P_TV_SHOW_ALL, 	IDS_AG_ORIENTATION_WEIGHT_LIST, 
		p_default, 		50.0f, 
		p_range, 		0.0f, 100.0f, 
		p_accessor,		&orient_const_accessor,	
		end, 

	orientation_target_list,  _T(""),		TYPE_INODE_TAB, 0, P_VARIABLE_SIZE,	IDS_AG_ORIENTATION_LIST,
		p_accessor,		&orient_const_accessor,	
		end,

	orientation_relative, _T("relative"),		TYPE_BOOL,		P_RESET_DEFAULT,	IDS_AG_ORIENT_CONS_RELATIVE,
		p_default, 		FALSE, 
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_ORIENT_CONS_RELATIVE, 
		p_accessor,		&orient_const_accessor,
		end,

	orientation_lw, 	_T("local_world"),		TYPE_INT, 		P_RESET_DEFAULT,	IDS_AG_ORIENT_CONS_LW,
		p_default, 		0, 
		p_range, 		0, 1, 
		p_ui, 			TYPE_RADIO, 2, IDC_ORIENT_CONST_WORLD, IDC_ORIENT_CONST_LOCAL,  
		p_accessor,		&orient_const_accessor,
		end, 

	end
	);



OrientConstRotation::OrientConstRotation(BOOL loading) 
	{
	curRot.Identity();
	flags      = 0;
	baseRotQuatLocal.Identity();
	baseRotQuatWorld.Identity();
	last_selection =  -1;
	pickWorldFlag = 0;
	InitialOrientQuat.Identity();
	oldTargetNumber = 0;
	startFlag = 1;
	firstTimeFlag = 0;

	// make the paramblock
	orientCD.MakeAutoParamBlocks(this);

	pblock->CallSets();
	ivalid.SetEmpty();
	validator.mod = this;
	hWnd =  NULL;

}

OrientConstRotation::~OrientConstRotation()
	{
	DeleteAllRefsFromMe();
	}

RefTargetHandle OrientConstRotation::Clone(RemapDir& remap)
	{
	OrientConstRotation *p = new OrientConstRotation(TRUE);

    p->ReplaceReference(ORIENT_ROT_PBLOCK_REF, pblock->Clone(remap));

	p->flags      = flags;
	p->curRot     = curRot;
	p->baseRotQuatLocal = baseRotQuatLocal;
	p->baseRotQuatWorld = baseRotQuatWorld;
	p->InitialOrientQuat = InitialOrientQuat;
	p->oldTargetNumber = oldTargetNumber;
	p->ivalid.SetEmpty();
	BaseClone(this, p, remap);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	return p;
	}


void OrientConstRotation::Copy(Control *from)
	{
	Quat fvalRot;
	if (from->ClassID()==ClassID()) {
		OrientConstRotation *ctrl = (OrientConstRotation*)from;
		// a copy will construct its own pblock to keep the pblock-to-owner 1-to-1.
		RemapDir *remap = NewRemapDir(); 
		ReplaceReference(ORIENT_ROT_PBLOCK_REF, ctrl->pblock->Clone(*remap));
		remap->DeleteThis();
		curRot   =			ctrl->curRot;
		baseRotQuatLocal =  ctrl->baseRotQuatLocal;
		baseRotQuatWorld =  ctrl->baseRotQuatWorld;
		flags    = ctrl->flags;
	} else {
		from->GetValue(GetCOREInterface()->GetTime(), &fvalRot, Interval(0, 0));	// to know the object orientation before the
		firstTimeFlag = 1;
		baseRotQuatLocal = fvalRot;
		baseRotQuatLocal.Normalize(); // current controller was active
		}
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}


void OrientConstRotation::Update(TimeValue t){
	Interval iv = FOREVER;
	ivalid = FOREVER;
	
	int ct = pblock->Count(orientation_target_list);
	int ctf = pblock->Count(orientation_target_weight);

	float total_orient_target_weight_prev = 0.0f, total_orient_target_weight_current = 0.0f;
	float orient_targ_Wt_current = 0.0f;
	Quat quat_prev, quat_current, lCurRot;
	INode *orient_target_prev= NULL, *orient_target_current= NULL;
	Matrix3 targetTM(1);
	Point3 trans, scaleP;

	quat_prev.Identity();
	quat_current.Identity();
	lCurRot.Identity();

		if (ct == 1){
			pblock->GetValue(orientation_target_list, t, orient_target_prev, iv, 0);
			pblock->GetValue(orientation_target_weight, t, orient_targ_Wt_current, iv, 0);
				ivalid &= iv;

				if (orient_target_prev == NULL){
					targetTM.IdentityMatrix();
				}
				else {
					targetTM = orient_target_prev->GetNodeTM(t, &ivalid);
				}

				if (IsLocal() && orient_target_prev != NULL) {
					targetTM = targetTM * Inverse(orient_target_prev->GetParentTM(t));
				}
				AffineParts comps;		// Requires header decomp.h
				decomp_affine(targetTM, &comps);
				quat_current = comps.q;
				quat_current.Normalize();
				quat_current.MakeClosest(quat_prev);

				lCurRot = quat_current;

				quat_prev = lCurRot;
				total_orient_target_weight_prev += orient_targ_Wt_current;
//			}
		}
		else if (ct > 1){

			pblock->GetValue(orientation_target_list, t, orient_target_prev, iv, 0);
			pblock->GetValue(orientation_target_weight, t, orient_targ_Wt_current, iv, 0);
//			if (orient_target_prev != NULL)
//			{
				ivalid &= iv;

				if (orient_target_prev == NULL){
					targetTM.IdentityMatrix();
				}
				else {
					targetTM = orient_target_prev->GetNodeTM(t, &ivalid);
				}

				if (IsLocal() && orient_target_prev != NULL) {
					targetTM = targetTM * Inverse(orient_target_prev->GetParentTM(t));
				}
				AffineParts comps;		// Requires header decomp.h
				decomp_affine(targetTM, &comps);
				quat_current = comps.q;
				quat_current.Normalize();
				quat_current.MakeClosest(quat_prev);

				lCurRot = quat_current;

				quat_prev = lCurRot;
				total_orient_target_weight_prev += orient_targ_Wt_current;
//			}

			for (int i = 0; i < ct -1; i++) {
//				ct = pblock->Count(orientation_target_list);
				pblock->GetValue(orientation_target_list, t, orient_target_current, iv, i+1);
				pblock->GetValue(orientation_target_weight, t, orient_targ_Wt_current, iv, i+1);
//				if (orient_target_current != NULL){
					ivalid &= iv;

					if (orient_target_current == NULL){
						targetTM.IdentityMatrix();
					}
					else {
						targetTM = orient_target_current->GetNodeTM(t, &ivalid);
					}

//					Matrix3 targetTM = orient_target_current->GetNodeTM(t, &ivalid);
					if (IsLocal() && orient_target_current != NULL) {
						targetTM = targetTM * Inverse(orient_target_current->GetParentTM(t));
					}
					AffineParts comps;		// Requires header decomp.h
					decomp_affine(targetTM, &comps);
					quat_current = comps.q;
					quat_current.Normalize();
					quat_current.MakeClosest(quat_prev);

					float slerp_wt = 0.0f;
					if ((total_orient_target_weight_prev + orient_targ_Wt_current) != 0.0){
						slerp_wt = orient_targ_Wt_current / (total_orient_target_weight_prev + orient_targ_Wt_current);
					}

					lCurRot = Slerp(quat_prev, quat_current, slerp_wt);
					lCurRot.Normalize();

					quat_prev = lCurRot;
					total_orient_target_weight_prev += orient_targ_Wt_current;
//				}
			}			//for (int i = 0; i < ct -1; i++)
		}				//else if (ct > 1)

		if (oldTargetNumber != ct) {
			InitialOrientQuat = lCurRot;
		}

		curRot = lCurRot;
		if (total_orient_target_weight_prev > 0.0){
			if (Relative()){
				if(IsLocal()){
					curRot =   baseRotQuatLocal * (lCurRot /InitialOrientQuat);
				}
				else{
					curRot =   baseRotQuatWorld * (lCurRot /InitialOrientQuat);
				}
			}
		}
		else {
			curRot = baseRotQuatLocal;
		}
		curRot.MakeClosest(IdentQuat());
		curRot.Normalize();
		oldTargetNumber = ct;
//	if (ivalid.Empty()) ivalid.SetInstant(t);
}


void OrientConstRotation::GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method)
{
	if (firstTimeFlag) {
		Point3 trans, scaleP;
		Quat quat;
		Matrix3 tempMat(1);
		// CAL-9/26/2002: in absolute mode the value could be un-initialized (random value).
		if (method == CTRL_RELATIVE) tempMat = *(Matrix3*)val;
		DecomposeMatrix(tempMat, trans, quat, scaleP);
		baseRotQuatWorld = baseRotQuatLocal * quat;
		baseRotQuatWorld.Normalize();
		firstTimeFlag = 0;
	}
	if (!ivalid.InInterval(t)) {
		DbgAssert(val != NULL);
		Update(t);
	}
	valid &= ivalid;

			 
	if (method==CTRL_RELATIVE) {
		Interval iv;

		if (IsLocal()){			// From Update, I'm getting target_local_TM in curRot
			Matrix3 *mat = (Matrix3*)val; // this is source_Parent_TM
			Quat q = curRot;			// this is target_local_TM
			PreRotateMatrix(*mat, q);	// source_world_TM = source_Parent_TM * target_local_TM 
		}
		
		else { 		// i.e., WorldToWorld: from Update, I'm getting target_world_TM in curRot
			int ct = pblock->Count(orientation_target_list);;
			if (ct < 1){
				Matrix3 *mat = (Matrix3*)val;
				Quat q = curRot;
				PreRotateMatrix(*mat, q);
			}
			else{
				Matrix3 *mat = (Matrix3*)val; // this is source_Parent_TM
				// CAL-06/24/02: preserve all components and replace with the new orientation.
				AffineParts ap;
				decomp_affine( *mat, &ap );
				ap.q = curRot;				// target world rotation
				comp_affine( ap, *mat );
				// CAL-06/24/02: this only preserve the translation component
				// Point3 tr;
				// tr = mat->GetTrans();
				// mat->IdentityMatrix();
				// Quat q = curRot;			// this is target_world_TM
				// q.MakeMatrix(*mat);
				// mat->SetTrans(tr);
			}
		}
	} 
	else {
		*((Quat*)val) = curRot;
	}
//		RedrawListbox(GetCOREInterface()->GetTime());
}

RefTargetHandle OrientConstRotation::GetReference(int i)
	{
		switch (i)
		{
			case ORIENT_ROT_PBLOCK_REF:
				return pblock;
		}
		return NULL;
	}

void OrientConstRotation::SetReference(int i, RefTargetHandle rtarg)
	{
		switch (i)
		{
			case ORIENT_ROT_PBLOCK_REF:
				pblock = (IParamBlock2*)rtarg; break;
		}
	}

RefResult OrientConstRotation::NotifyRefChanged (
		Interval iv, 
		RefTargetHandle hTarg, 
		PartID& partID, 
		RefMessage msg) 
	{

//	INode* orient_targ;

	switch (msg) {
		case REFMSG_CHANGE:
		{
			if (hTarg == pblock){
				ParamID changing_param = pblock->LastNotifyParamID();
				switch (changing_param)
				{
					// CAL-09/10/02: need to redraw the list box specifically.
					// TODO: Most of the other calls to RedrawListbox() should be removed with this addition.
					case orientation_target_weight:
						RedrawListbox(GetCOREInterface()->GetTime());
						break;
					default:
						orient_const_paramblk.InvalidateUI(changing_param);
						break;
				}
				ivalid.SetEmpty();
			}
		}
		break;

		case REFMSG_OBJECT_CACHE_DUMPED:
			
			return REF_STOP;

		break;
	}
	return REF_SUCCEED;
}


class PickTargRestore : public RestoreObj {
	public:
		OrientConstRotation *cont;
		PickTargRestore(OrientConstRotation *c) {cont=c;}
		void Restore(int isUndo) {
			if (cont->editCont == cont) {
				orient_const_paramblk.InvalidateUI();
				}									
			}
		void Redo() {
		}
};
	
int OrientConstRotation::GetNumTargets(){
	return pblock->Count(orientation_target_list);
}

INode* OrientConstRotation::GetNode(int targetNumber){
	if (targetNumber >= 0 && targetNumber < pblock->Count(orientation_target_list)){
		INode *orient_targ;
		pblock->GetValue(orientation_target_list, 0, orient_targ, FOREVER, targetNumber);
		return orient_targ;
	}
	return NULL;
}

BOOL OrientConstRotation::AppendTarget(INode *target, float weight){

	if (target == NULL){
		float var = 50.0f;
		int ct = pblock->Count(orientation_target_list);
		int ctf = pblock->Count(orientation_target_weight);
		theHold.Begin();
		pblock->SetCount(orientation_target_list, ct + 1);
		pblock->SetValue(orientation_target_list, GetCOREInterface()->GetTime(), target, ct);
		pblock->Append(orientation_target_weight, 1, &var, 1);
		theHold.Accept(GetString(IDS_AG_ORIENTATION_LIST));
		return TRUE;
	}
	else if (!(target->TestForLoop(FOREVER,(ReferenceMaker *) this)!=REF_SUCCEED)) 
	{
		for (int i = 0; i < pblock->Count(orientation_target_list); i++){
			if (target == pblock->GetINode(orientation_target_list, GetCOREInterface()->GetTime(), i)){
				return FALSE; // the target is already in the targetlist
			}
		}
			
		theHold.Begin();
		pblock->Append(orientation_target_list, 1, &target, 1);
		pblock->Append(orientation_target_weight, 1, &weight, 1);
		theHold.Accept(GetString(IDS_AG_ORIENTATION_LIST));
		return TRUE;
	}
	return FALSE;


}

BOOL OrientConstRotation::DeleteTarget(int selection){
	int ct = pblock->Count(orientation_target_list);
	if (selection >= 0 && selection < pblock->Count(orientation_target_list)){
//		theHold.Begin();
		pblock->Delete(orientation_target_weight, selection, 1);
		pblock->Delete(orientation_target_list, selection, 1);
//		theHold.Accept(GetString(IDS_AG_ORIENTATION_LIST));
		return TRUE;
	}
	else
		return FALSE;

}

/*
BOOL OrientConstRotation::SetTarget(INode *target, int targetNumber){
	if (target->TestForLoop(FOREVER,this)==REF_SUCCEED) {

		if (targetNumber >= 0 && targetNumber <= pblock->Count(orientation_target_list)) {
			float orientWt = 50.0;
			theHold.Begin();
			pblock->SetValue(orientation_target_list, 0, target, targetNumber);
			pblock->SetValue(orientation_target_weight, GetCOREInterface()->GetTime(), orientWt, targetNumber);
			theHold.Accept(GetString(IDS_AG_ORIENTATION_LIST));
			RedrawListbox(GetCOREInterface()->GetTime());
		}
		else{
			return FALSE;   // force the programmer to use  the append function
		}
		return TRUE;
	} else {
		return FALSE;
		}
	}
*/


float OrientConstRotation::GetTargetWeight(int targetNumber){
	if (targetNumber >= 0 && targetNumber < pblock->Count(orientation_target_list)){
		float targetWt;
		pblock->GetValue(orientation_target_weight, GetCOREInterface()->GetTime(), targetWt, FOREVER, targetNumber);
		return targetWt;
	}
	else{
		return 0.0f;
	}
}

BOOL OrientConstRotation::SetTargetWeight(int targetNumber, float weight){
	if (targetNumber >= 0 && targetNumber < pblock->Count(orientation_target_list)){
		pblock->SetValue(orientation_target_weight, GetCOREInterface()->GetTime(), weight, targetNumber);
		return TRUE;
	}
	else{
		return FALSE;
	}
}

void OrientConstRotation::SetRelative(BOOL rel)
	{
	pblock->SetValue(orientation_relative, GetCOREInterface()->GetTime(), rel);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}

int OrientConstRotation::IsLocal(){
	Interval iv;
	BOOL isitLocal;
	pblock->GetValue(orientation_lw, GetCOREInterface()->GetTime(), isitLocal, iv);
	return isitLocal;
	}

BOOL OrientConstRotation::Relative(){
	Interval iv;
	BOOL isRelative;
	pblock->GetValue(orientation_relative, GetCOREInterface()->GetTime(), isRelative, iv);
	return isRelative;
}

/*--------------------------------------------------------------------*/
// OrientConstRotation UI

void OrientConstRotation::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
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
		orientCD.BeginEditParams(ip, this, flags, prev);
	}

	ip->RegisterTimeChangeCallback(&orientConstTimeChangeCallback);
	orientConstTimeChangeCallback.orient_controller = this;
	
	IParamMap2* pmap = pblock->GetMap();
	if (pmap) hWnd = pmap->GetHWnd();

	if (last_selection < 0){
		RedrawListbox(GetCOREInterface()->GetTime());
	}
	else {
		RedrawListbox(GetCOREInterface()->GetTime(), last_selection);
		ISpinnerControl* spin = GetISpinner(GetDlgItem(hWnd, IDC_ORIENT_CONS_WEIGHT_SPINNER));
		if (spin != NULL){
			float value = pblock->GetFloat(orientation_target_weight, GetCOREInterface()->GetTime(), last_selection);
			spin->SetValue(value, FALSE);
		}
		ReleaseISpinner(spin);

	}
}

void OrientConstRotation::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
{
	editCont = NULL;
	IParamMap2* pmap = pblock->GetMap();
	if (pmap != NULL)
	{
		if (next && next->ClassID() == ClassID() && ((OrientConstRotation*)next)->pblock)
		{
			pmap->SetParamBlock(((OrientConstRotation*)next)->pblock);
			ip->ClearPickMode();
		}
		else
			orientCD.EndEditParams(ip, this, flags | END_EDIT_REMOVEUI, next);
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
		ip->UnRegisterTimeChangeCallback(&orientConstTimeChangeCallback);
		ip->ClearPickMode(); // need this, otherwise will crash on undo, while pickmode is active.
		this->ip = NULL;
		hWnd = NULL;
}

TSTR OrientConstRotation::SvGetRelTip(IGraphObjectManager *gom, IGraphNode *gNodeTarger, int id, IGraphNode *gNodeMaker)
{
	return SvGetName(gom, gNodeMaker, false) + " -> " + gNodeTarger->GetAnim()->SvGetName(gom, gNodeTarger, false);
}

bool OrientConstRotation::SvCanDetachRel(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker)
{
	return true;
}

bool OrientConstRotation::SvDetachRel(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker)
{
	for( int i=0; i<GetNumTargets(); i++ ) {
		if( GetNode(i) == gNodeTarget->GetAnim() ) {
			DeleteTarget(i);
			return true;
		}
	}
	return false;
}

bool OrientConstRotation::SvCanConcludeLink(IGraphObjectManager *gom, IGraphNode *gNode, IGraphNode *gNodeChild)
{
	Animatable *pChildAnim = gNodeChild->GetAnim();
	if( pChildAnim->SuperClassID() == BASENODE_CLASS_ID )
		return true;

	return Control::SvCanConcludeLink(gom, gNode, gNodeChild);
}

bool OrientConstRotation::SvLinkChild(IGraphObjectManager *gom, IGraphNode *gNodeThis, IGraphNode *gNodeChild)
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

SvGraphNodeReference OrientConstRotation::SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags)
{
	SvGraphNodeReference nodeRef = Control::SvTraverseAnimGraph( gom, owner, id, flags );

	if( nodeRef.stat == SVT_PROCEED ) {
		for( int i=0; i<GetNumTargets(); i++ ) {
			gom->AddRelationship( nodeRef.gNode, GetNode(i), i, RELTYPE_CONSTRAINT );
		}
	}

	return nodeRef;
}

int OrientConstRotation::SetProperty(ULONG id, void *data)
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

void* OrientConstRotation::GetProperty(ULONG id)
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


#define OFFSETQUAT_LOCAL_CHUNK	0x1001
#define OFFSETQUAT_WORLD_CHUNK	0x1002
#define INITIAL_QUAT_CHUNK		0x1003
#define OLD_TARGET_NUMBER_CHUNK		0x1004


IOResult OrientConstRotation::Save(ISave *isave)
	{	

		ULONG nb;
		// Save baseRotQuatLocal
		isave->BeginChunk(OFFSETQUAT_LOCAL_CHUNK);
		isave->Write(&baseRotQuatLocal, sizeof(Quat), &nb);
		isave->EndChunk();

		// Save baseRotQuatWorld
		isave->BeginChunk(OFFSETQUAT_WORLD_CHUNK);
		isave->Write(&baseRotQuatWorld, sizeof(Quat), &nb);
		isave->EndChunk();

		// Save InitialOrientQuat
		isave->BeginChunk(INITIAL_QUAT_CHUNK);
		isave->Write(&InitialOrientQuat,sizeof(InitialOrientQuat),&nb);	
		isave->EndChunk();

		// Save oldTargetNumber
		isave->BeginChunk(OLD_TARGET_NUMBER_CHUNK);
		isave->Write(&oldTargetNumber,sizeof(oldTargetNumber),&nb);	
		isave->EndChunk();
	
	return IO_OK;
	}

IOResult OrientConstRotation::Load(ILoad *iload)
{
	
	ULONG nb;
	IOResult res = IO_OK;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			
			case OFFSETQUAT_LOCAL_CHUNK:
				res=iload->Read(&baseRotQuatLocal, sizeof(Quat), &nb);
			break;

			case OFFSETQUAT_WORLD_CHUNK:
				res=iload->Read(&baseRotQuatWorld, sizeof(Quat), &nb);
			break;

			case INITIAL_QUAT_CHUNK:
				res=iload->Read(&InitialOrientQuat, sizeof(InitialOrientQuat), &nb);
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
