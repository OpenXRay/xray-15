/**********************************************************************
 *
	FILE: lookat_cnstrnt.cpp

	DESCRIPTION: A rotation constraint controller that controls the lookat 
				of an object based on the lookat of the target objects

	CREATED BY: Ambarish Goswami 5/2000
	            

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/




#include "ctrl.h"
#include "units.h"
#include "interpik.h"
#include "istdplug.h"
#include "iparamm2.h"
#include "decomp.h"


#define LOOKAT_CONSTRAINT_CNAME		GetString(IDS_AG_LOOKAT_CONS)
#define more_than_and_very_close_to_zero 0.0000001f
#define less_than_and_very_close_to_one  0.9999999f


enum {	lookat_target_weight,		lookat_target_list,		lookat_relative, 
		lookat_vector_line_length,	set_orientation,		lookat_target_axis, 
		lookat_target_axis_flip,	lookat_upnode_axis,		lookat_upnode_world, 
		lookat_upnode_pick,			lookat_StoUP_axis,		lookat_StoUP_axis_flip, 
		viewline_length_abs,		lookat_upnode_control, };


class LookAtConstRotation; 
class LookAtConstTimeChangeCallback : public TimeChangeCallback
{
	public:
		LookAtConstRotation* lookAt_controller;
		void TimeChanged(TimeValue t);
};

class lookAtConstValidatorClass : public PBValidator
{
	public:
		LookAtConstRotation *mod;
	private:
		BOOL Validate(PB2Value &v);
};

static inline void dLine2Pts(GraphicsWindow *gw, Point3& r, Point3& q) {
	Point3 s[3];
	s[0] = r; s[1] = q; 
 	gw->polyline( 2, s, NULL, NULL, TRUE, NULL );
	}


class LookAtConstRotation : public	ILookAtConstRotation  
{
	public:
		Box3 bbox;
		lookAtConstValidatorClass validator;							
		void RedrawListbox(TimeValue t, int sel = -1);
		HWND hWnd;
		int last_selection, oldTargetNumber;
		
		Matrix3 sourceTM;
		float total_lookat_target_weight;
		
		LookAtConstTimeChangeCallback lookAtConstTimeChangeCallback;	
		IParamBlock2* pblock;


		DWORD flags;
		Quat curRot, baseRotQuatLocal, baseRotQuatWorld;
		Interval ivalid; 
		Quat InitialLookAtQuat;
		int initialLookAtFlag;
		Point3 baseEuler;
		Quat userRotQuat;

		static LookAtConstRotation *editCont;
		static IObjParam *ip;

		LookAtConstRotation(const LookAtConstRotation &ctrl);
		LookAtConstRotation(BOOL loading=FALSE);

		~LookAtConstRotation();

		int GetNumTargets();									
		int GetnonNULLNumLookAtTargets();
		INode* GetNode(int targetNumber);	
		float GetTargetWeight(int targetNumber);			
		BOOL SetTargetWeight(int targetNumber, float weight);
		BOOL AppendTarget(INode *target, float weight=50.0);
		BOOL DeleteTarget(int selection);	


		BOOL GetRelative() {return Relative();}
		BOOL GetTargetAxisFlip() {return TFlip();}
		BOOL GetStoUPAxisFlip() {return SUFlip();}
		BOOL Get_SetOrientation() {return SetOrientation();}
		BOOL GetUpnodeWorld() {return UpnodeWorld();}
		BOOL GetVLisAbs() {return VLAbs();}
		int GetTargetAxis();
		int GetUpNodeAxis();
		int Get_StoUPAxis();
		int Get_upnode_control();

		void SetRelative(BOOL rel);
		void SetVLisAbs(BOOL rel);
		void SetUpnodeWorld(BOOL uw);
		void SetTargetAxisFlip(BOOL rel);
		void SetStoUPAxisFlip(BOOL rel);
		void Set_SetOrientation(BOOL rel);
		void Set_Reset_Orientation();
		void SetTargetAxis(int axis);
		void SetUpNodeAxis(int axis);
		void Set_StoUPAxis(int axis);
		void Set_upnode_control(int ucontrol);
		Quat ComputeLookAtQuat(TimeValue t);

		BOOL Relative();
		BOOL UpnodeWorld();
		BOOL TFlip();
		BOOL SUFlip();
		BOOL SetOrientation();
		BOOL VLAbs();

		void HoldQuat();
		Point3 GetTargetPosition(TimeValue t);
		Point3 ProjectionOnPerpPlane(Point3 Vector_ProjectionOf, Point3 Vector_OnPlanePerpTo);

		void Update(TimeValue t);


		// Animatable methods
		Class_ID ClassID() { return Class_ID(LOOKAT_CONSTRAINT_CLASS_ID, 0); }  
		SClass_ID SuperClassID() { return CTRL_ROTATION_CLASS_ID; }  		
		
		void GetClassName(TSTR& s) {s = LOOKAT_CONSTRAINT_CNAME;}
		void DeleteThis() {delete this;} // intellgent destructor
		int IsKeyable() {return 0;}		// if the controller is keyable (for trackview)?

		int NumSubs()  {return 1;} //because it uses the paramblock
		Animatable* SubAnim(int i) {return pblock;}
		TSTR SubAnimName(int i) { return GetString(IDS_AG_LOOKATPARAMS); }
		int SubNumToRefNum(int subNum) {if (subNum==0) return LOOKAT_ROT_PBLOCK_REF; else return -1;}

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
		void SetValue(TimeValue t, void *val, int commit, GetSetMethod method);
		void CommitValue(TimeValue t) { }
		void RestoreValue(TimeValue t) { }
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		void GetWorldBoundBox(TimeValue t,INode* inode,ViewExp *vpt, Box3& box);
		BOOL InheritsParentTransform() {return FALSE;}

// FUNCTION_PUBLISHING

		BaseInterface* GetInterface(Interface_ID id) 
		{ 
			if (id == LOOKAT_CONSTRAINT_INTERFACE) 
				return (LookAtConstRotation*)this; 
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


BOOL lookAtConstValidatorClass::Validate(PB2Value &v)
{
	INode *node = (INode*) v.r;

	for (int i = 0; i < mod->pblock->Count(lookat_target_list); i++)
	{
		if (node == mod->pblock->GetINode(lookat_target_list, 0, i))
			return FALSE;
	}
	return TRUE;
}

void LookAtConstTimeChangeCallback :: TimeChanged(TimeValue t){

	int selection = SendDlgItemMessage(lookAt_controller->hWnd, IDC_LOOKAT_TARG_LIST, LB_GETCURSEL, 0, 0);
	ISpinnerControl* spin = GetISpinner(GetDlgItem(lookAt_controller->hWnd, IDC_LOOKAT_CONS_WEIGHT_SPINNER));
	spin = GetISpinner(GetDlgItem(lookAt_controller->hWnd, IDC_LOOKAT_CONS_WEIGHT_SPINNER));

	Control *cont = lookAt_controller->pblock->GetController(lookat_target_weight, selection); 
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
	lookAt_controller->RedrawListbox(t);
}


IObjParam *LookAtConstRotation::ip						= NULL;
LookAtConstRotation *LookAtConstRotation::editCont		= NULL;

//********************************************************
// LOOKAT CONSTRAINT
//********************************************************
static Class_ID lookAtConstControlClassID(LOOKAT_CONSTRAINT_CLASS_ID,0); 
class LookAtConstClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new LookAtConstRotation(loading); }
	const TCHAR *	ClassName() { return LOOKAT_CONSTRAINT_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_ROTATION_CLASS_ID; }
	Class_ID		ClassID() { return lookAtConstControlClassID; }
	const TCHAR* 	Category() { return _T("");  }
	const TCHAR*	InternalName() { return _T("Lookat_Constraint"); }// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
	};

static LookAtConstClassDesc lookAtCD;
ClassDesc* GetLookAtConstDesc() {return &lookAtCD;}



//--- CustMod dlg proc ---------DOWN DOWN DOWN----from Modifiers/clstmode.cpp---------------

class PickLookAtNode : public PickModeCallback,
		public PickNodeCallback {
	public:			
		LookAtConstRotation *p;
		PickLookAtNode() {p = NULL;}
		BOOL HitTest(IObjParam *ip, HWND hWnd, ViewExp *vpt, IPoint2 m, int flags);		
		BOOL Pick(IObjParam *ip,ViewExp *vpt);		
		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);		
		BOOL Filter(INode *node);
		PickNodeCallback *GetFilter() {return this;}
		BOOL RightClick(IObjParam *ip,ViewExp *vpt) {return TRUE;}
	};
static PickLookAtNode thePickMode;

BOOL PickLookAtNode::Filter(INode *node){

	for (int i = 0; i < p->pblock->Count(lookat_target_list); i++){
		if (node == p->pblock->GetINode(lookat_target_list, 0, i)) 
			return FALSE;
	}

	if (node->TestForLoop(FOREVER,(ReferenceMaker *) p)!=REF_SUCCEED) 
	{
		return FALSE;
	}

	return TRUE;
}

BOOL PickLookAtNode::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
	{	
	if (ip->PickNode(hWnd,m,this)) {
		return TRUE;
	} else {
		return FALSE;
		}
	}

BOOL PickLookAtNode::Pick(IObjParam *ip,ViewExp *vpt)
	{

		INode *node = vpt->GetClosestHit();
		if (node) {
			p->AppendTarget(node);
			p->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
			p->ip->RedrawViews(GetCOREInterface()->GetTime());
		}
		return FALSE;
	}

void PickLookAtNode::EnterMode(IObjParam *ip)
	{
		ICustButton *iBut = GetICustButton(GetDlgItem(p->hWnd,IDC_LOOKAT_TARG_PICKNODE));
		if (iBut) iBut->SetCheck(TRUE);
		ReleaseICustButton(iBut);

		ICustButton *iPickOb;
		iPickOb	= GetICustButton(GetDlgItem(p->hWnd, IDC_LOOKAT_SET_ORIENTATION_PICKNODE));
		if (iPickOb != NULL){
			iPickOb->Enable(FALSE);	
		}
		ReleaseICustButton(iPickOb);

		GetCOREInterface()->PushPrompt("Select LookAt Target Object");
	}

void PickLookAtNode::ExitMode(IObjParam *ip)
	{
		ICustButton *iBut = GetICustButton(GetDlgItem(p->hWnd,IDC_LOOKAT_TARG_PICKNODE));
		if (iBut) iBut->SetCheck(FALSE);
		ReleaseICustButton(iBut);

		ICustButton *iPickOb;
		iPickOb	= GetICustButton(GetDlgItem(p->hWnd, IDC_LOOKAT_SET_ORIENTATION_PICKNODE));
		if (iPickOb != NULL){
			iPickOb->Enable(TRUE);	
		}
		ReleaseICustButton(iPickOb);


		GetCOREInterface()->PopPrompt();
	}

//--- CustMod dlg proc -----------UP UP UP -------------------


// FUNCTION_PUBLISHING
//Function Publishing descriptor for Mixin interface
//Added by Ambarish Goswami (5/18/2000)
//*****************************************************


  static FPInterfaceDesc lookAt_constraint_interface(
    LOOKAT_CONSTRAINT_INTERFACE, _T("constraints"), 0, &lookAtCD, 0,

		ILookAtConstRotation::get_num_targets,		_T("getNumTargets"),0, TYPE_INDEX, 0, 0,
		ILookAtConstRotation::get_node,				_T("getNode"),		0, TYPE_INODE, 0, 1,
			_T("targetNumber"),	0,	TYPE_INDEX,
		ILookAtConstRotation::get_target_weight,	_T("getWeight"),	0, TYPE_FLOAT, 0, 1,
			_T("targetNumber"),	0,	TYPE_INDEX,
		ILookAtConstRotation::set_target_weight,	_T("setWeight"),	0, TYPE_BOOL,  0, 2,
			_T("targetNumber"),	0, TYPE_INDEX,
			_T("weight"),	0, TYPE_FLOAT,
		ILookAtConstRotation::append_target,		_T("appendTarget"), 0, TYPE_BOOL,  0, 2,
			_T("target"),	0,	TYPE_INODE,
			_T("weight"),	0,	TYPE_FLOAT,
		ILookAtConstRotation::delete_target,		_T("deleteTarget"), 0, TYPE_BOOL,  0, 1,
			_T("targetNumber"), 0, TYPE_INDEX,

		end
		);


//  Get Descriptor method for Mixin Interface
//  *****************************************
FPInterfaceDesc* ILookAtConstRotation::GetDesc()
{
     return &lookAt_constraint_interface;
}

//*********************************************
// End of Function Publishing Code


enum { lookAt_const_params, };

// parameter setter callback, reflect any ParamBlock-mediated param setting in instance data members.

class LookAtConstPBAccessor : public PBAccessor
{ 
	public:

		void TabChanged(tab_changes changeCode, Tab<PB2Value>* tab, 
                  ReferenceMaker* owner, ParamID id, int tabIndex, int count) 
		{ 
		if ((id == lookat_target_weight) || (id == lookat_target_list)){

			LookAtConstRotation* p = (LookAtConstRotation*)owner;
			int ct = p->pblock->Count(lookat_target_list);
			int ctf = p->pblock->Count(lookat_target_weight);
			if (changeCode == tab_ref_deleted){	
				p->DeleteTarget(tabIndex);
				ct = p->pblock->Count(lookat_target_list);
				ctf = p->pblock->Count(lookat_target_weight);
//				p->pblock->SetCount(lookat_target_list, ct-1);
//				p->pblock->SetCount(lookat_target_weight, ctf-1);
//				int j = p->pblock->Delete(lookat_target_list, tabIndex, 1); // deletes the node from nodelist;
//				int k = p->pblock->Delete(lookat_target_weight, tabIndex, 1); // deletes corresponding weight;
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

				if (ct > 0 && p->last_selection < 0) p->last_selection = 0;
	
				p->RedrawListbox(GetCOREInterface()->GetTime(), p->last_selection);

				if (p->last_selection < 0 || p->GetnonNULLNumLookAtTargets() < 1){ 
				// nothing is selected OR there are no targets
					// all buttons - target_delete, weight_edit, weight_spinner -- are disabled
					EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_WEIGHT_EDIT), FALSE);
					EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_WEIGHT_SPINNER), FALSE);
					ICustButton *iPickOb1;
					iPickOb1= GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_LOOKAT_TARG));
					if (iPickOb1 != NULL){
						iPickOb1->Enable(FALSE);		
						ReleaseICustButton(iPickOb1);
					}
				}
				else if (p->last_selection >= 0) {	
				// there is a valid selection 
					// automatically means there is AT LEAST one target
					if (p->GetnonNULLNumLookAtTargets() == 1){
					// the "delete" button should be enabled
						ICustButton *iPickOb1;
						iPickOb1 = GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_LOOKAT_TARG));
						if (iPickOb1 != NULL){
							iPickOb1->Enable(TRUE);		
							ReleaseICustButton(iPickOb1);
						}
						// the rest are disabled
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_WEIGHT_EDIT), FALSE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_WEIGHT_SPINNER), FALSE);
					}
					
					else if (p->GetnonNULLNumLookAtTargets() > 1){
						// all buttons - target_delete, weight_edit, weight_spinner -- are enabled
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_WEIGHT_EDIT), TRUE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_WEIGHT_SPINNER), TRUE);
						ICustButton *iPickOb1;
						iPickOb1 = GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_LOOKAT_TARG));
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
		LookAtConstRotation* p = (LookAtConstRotation*)owner;
		switch (id)
		{
			case lookat_target_weight:
				if ((v.f) < 0.0f) 
					v.f = 0.0f; 
			break;
		}
	}

	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		LookAtConstRotation* p = (LookAtConstRotation*)owner;

		switch (id)
		{

			case lookat_target_axis: // selects default choice for  source/upnode axis combination
				switch (v.i)
				{
					case 0: 
						
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_X), FALSE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_Y), TRUE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_Z), TRUE);
						p->pblock->SetValue(lookat_upnode_axis, t, 2);
						p->pblock->SetValue(lookat_StoUP_axis, t, 2);

					break;			// X
					case 1: 
						
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_X), TRUE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_Y), FALSE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_Z), TRUE);
						p->pblock->SetValue(lookat_upnode_axis, t, 0);
						p->pblock->SetValue(lookat_StoUP_axis, t, 0);
						
					break;	// Y
					case 2: 
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_X), TRUE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_Y), TRUE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_Z), FALSE);
						p->pblock->SetValue(lookat_upnode_axis, t, 1);
						p->pblock->SetValue(lookat_StoUP_axis, t, 1);

					break;	// Z
				}

			break;

//			case lookat_target_axis_flip:
//						
//				if (v.i) EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_FLIP), FALSE);
//				else EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_FLIP), TRUE);
//							
//			break;

//			case lookat_StoUP_axis_flip:
//				if (v.i) EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_TARGET_FLIP), FALSE);
//				else EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_TARGET_FLIP), TRUE);
//
//			break;

			


			case lookat_upnode_control: // selects default choice for  source/upnode axis combination
				int status;
				switch (v.i)
				{
					case 0: // upnode controls sourcenode secondary lookat direction 
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_X), FALSE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Y), FALSE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Z), FALSE);

						ICustButton *iPickOb;
						iPickOb	= GetICustButton(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_PICKNODE));
						status = GetCheckBox(p->hWnd,IDC_LOOKAT_CONS_UPNODE_W);
						if (iPickOb != NULL){
							if (p->GetNode(0) == p->pblock->GetINode(lookat_upnode_pick, 0, 0) ||
								(status == 1)){
								    iPickOb->Enable(FALSE);
							}
							else iPickOb->Enable(TRUE);
							
						}
						ReleaseICustButton(iPickOb);

					break;	
					case 1: // upnode controls sourcenode orientation
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_X), TRUE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Y), TRUE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Z), TRUE);

					break;
				}
			break;

			case lookat_upnode_world:
				ICustButton *iPickOb;
				iPickOb	= GetICustButton(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_PICKNODE));
				if (iPickOb){  // this is always valid if there is a valid window
					if (v.i) {
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_CTRL_LA), TRUE);
						iPickOb->SetCheck(FALSE);
						iPickOb->Enable(FALSE);
//						iPickOb->SetText("None");
//						INode *nullNode = NULL;
//						theHold.Begin();
//						p->pblock->SetValue(lookat_upnode_pick, t, nullNode, 0);
//						theHold.Accept(GetString(IDS_AG_LOOKAT_UPNODE_PICKNODE));

						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_X), TRUE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_Y), TRUE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_Z), TRUE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_FLIP), TRUE);


						if (p->Get_upnode_control() ==  1){
							EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_X), TRUE);
							EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Y), TRUE);
							EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Z), TRUE);

						}
						else if (p->Get_upnode_control() ==  0){
							EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_X), FALSE);
							EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Y), FALSE);
							EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Z), FALSE);
							EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_PICKNODE), TRUE);


						}
					}
					else{ // World is unchecked

						if (!(p->pblock->Count(lookat_target_list) == 1 && p->Get_upnode_control() == 0 &&
							p->GetNode(0) == p->pblock->GetINode(lookat_upnode_pick, 0, 0) )){
							iPickOb->Enable(TRUE);
						}
						if (p->pblock->Count(lookat_target_list) == 1 
								&& (p->GetNode(0) == p->pblock->GetINode(lookat_upnode_pick, 0, 0))){
							EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_CTRL_LA), FALSE);
						}

						if (p->pblock->GetINode(lookat_upnode_pick, 0, 0) == NULL){
							iPickOb->Enable(TRUE);
							EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_X), FALSE);
							EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Y), FALSE);
							EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Z), FALSE);
							EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_X), FALSE);
							EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_Y), FALSE);
							EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_Z), FALSE);
							EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_FLIP), FALSE);

						}
						else if (p->pblock->GetINode(lookat_upnode_pick, 0, 0)){
							if (p->Get_upnode_control() ==  1){
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_X), TRUE);
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Y), TRUE);
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Z), TRUE);
							}
							else if (p->Get_upnode_control() ==  0){

								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_X), FALSE);
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Y), FALSE);
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Z), FALSE);

								if (p->pblock->Count(lookat_target_list) == 1){
									if (p->GetNode(0) == p->pblock->GetINode(lookat_upnode_pick, 0, 0)){
										iPickOb->Enable(TRUE);
										iPickOb->SetText("None");
										INode *nullNode = NULL;
										p->pblock->SetValue(lookat_upnode_pick, t, nullNode, 0);
									}
								}

							}
						}




					}
					// EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_PICKNODE), TRUE); 
					// won't work well.
					// grey windows in the resource need to be controlled by max methods
					// ex: EnableWindow worked well with IDC_LOOKAT_CONS_S_UP_Y radio button above.
					ReleaseICustButton(iPickOb);
				}
			break;

			case lookat_upnode_pick:


				if ((p->pblock->Count(lookat_target_list) == 1) && (p->Get_upnode_control() == 0)){
					if (v.r == p->GetNode(0) && !GetCheckBox(p->hWnd,IDC_LOOKAT_CONS_UPNODE_W)) {
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_CTRL_AA), TRUE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_CTRL_LA), FALSE);
						p->Set_upnode_control(1);
					}
					else{
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_CTRL_AA), TRUE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_CTRL_LA), TRUE);
					}
				}
				if (v.r){
					EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_X), TRUE);
					EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_Y), TRUE);
					EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_Z), TRUE);
					EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_FLIP), TRUE);

					if (p->Get_upnode_control() ==  1){
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_X), TRUE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Y), TRUE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Z), TRUE);
						if (p->pblock->Count(lookat_target_list) == 1){
							if(v.r == p->GetNode(0))
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_CTRL_LA), FALSE);
							else
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_CTRL_LA), TRUE);
						}

					}
					else if (p->Get_upnode_control() ==  0){
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_X), FALSE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Y), FALSE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Z), FALSE);

					}
				}
				
				
			break;


			case lookat_relative:
				iPickOb	= GetICustButton(GetDlgItem(p->hWnd, IDC_LOOKAT_SET_ORIENTATION_PICKNODE));
				if (iPickOb != NULL){
					if (v.i) iPickOb->Enable(FALSE);	
					else iPickOb->Enable(TRUE);
					ReleaseICustButton(iPickOb);
				}
			break;

			case set_orientation:
				iPickOb	= GetICustButton(GetDlgItem(p->hWnd, IDC_LOOKAT_TARG_PICKNODE));
				if (iPickOb != NULL){
					if (v.i) {
						iPickOb->Enable(FALSE);	
					}
					else {
						iPickOb->Enable(TRUE);
					}
					ReleaseICustButton(iPickOb);
				}
			break;

//			case lookat_StoUP_axis:
//
//				EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_FLIP), TRUE);
//
//			break;



/*			case set_orientation:
//				ICustButton *iPickOb;
				iPickOb	= GetICustButton(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_RELATIVE));
				if (iPickOb != NULL){
					if (v.i) iPickOb->Enable(FALSE);	
					else iPickOb->Enable(TRUE);
					ReleaseICustButton(iPickOb);
				}
				
//			break;
*/
		
		}
	}
	
};	


static LookAtConstPBAccessor lookAt_const_accessor;

class LookAtConstDlgProc : public ParamMap2UserDlgProc 
{
	public:
		
		void UpdateLookAtName(LookAtConstRotation* p){
			IParamMap2* pmap = p->pblock->GetMap();
		}
						

		BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			LookAtConstRotation* p = (LookAtConstRotation*)map->GetParamBlock()->GetOwner();
			UpdateLookAtName(p);
			p->hWnd = hWnd;
			int ct = p->pblock->Count(lookat_target_list);
			int selection;
			ISpinnerControl* spin;
			ICustButton *iBut1, *iBut2;

			switch (msg) 
			{
				case WM_INITDIALOG:

						selection = p->last_selection;
						if (selection < 0 && p->GetnonNULLNumLookAtTargets() > 0) selection = 0;

						iBut1 = GetICustButton(GetDlgItem(hWnd,IDC_LOOKAT_TARG_PICKNODE));
						iBut1->SetType(CBT_CHECK);
						iBut1->SetHighlightColor(GREEN_WASH);
						ReleaseICustButton(iBut1);

						iBut2 = GetICustButton(GetDlgItem(hWnd,IDC_LOOKAT_SET_ORIENTATION_PICKNODE));
						iBut2->SetType(CBT_CHECK);
						iBut2->SetHighlightColor(GREEN_WASH);
						ReleaseICustButton(iBut2);

/*						ICustButton *iBut3 = GetICustButton(GetDlgItem(hWnd,IDC_LOOKAT_RESET_ORIENTATION_PICKNODE));
						iBut3->SetType(CBT_CHECK);
						iBut3->SetHighlightColor(GREEN_WASH);
						ReleaseICustButton(iBut3);
*/
						
						spin = SetupFloatSpinner(hWnd, IDC_LOOKAT_CONS_WEIGHT_SPINNER, IDC_LOOKAT_CONS_WEIGHT_EDIT, 0.0f, 100.0f, 50.0f, 1.0f);
						spin = GetISpinner(GetDlgItem(hWnd, IDC_LOOKAT_CONS_WEIGHT_SPINNER));
						
						
						switch (p->Get_upnode_control()){
							case 0: // upnode controls sourcenode secondary lookat direction 
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_X), FALSE);
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Y), FALSE);
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Z), FALSE);

								if (p->pblock->Count(lookat_target_list) == 1){
									if (p->GetNode(0) == p->pblock->GetINode(lookat_upnode_pick, 0, 0)){

										ICustButton *iPickOb;
										iPickOb	= GetICustButton(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_PICKNODE));
										if (iPickOb != NULL){
											iPickOb->Enable(FALSE);
										}
										ReleaseICustButton(iPickOb);

//										EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_PICKNODE), FALSE);
									}
									else{
//										EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_PICKNODE), TRUE);
										ICustButton *iPickOb;
										iPickOb	= GetICustButton(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_PICKNODE));
										if (iPickOb != NULL){
											iPickOb->Enable(TRUE);
										}
										ReleaseICustButton(iPickOb);
									}

								}

							break;	
							case 1: // upnode controls sourcenode orientation
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_X), TRUE);
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Y), TRUE);
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Z), TRUE);

								if (p->pblock->Count(lookat_target_list) == 1){
									if (p->GetNode(0) == p->pblock->GetINode(lookat_upnode_pick, 0, 0)){
										EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_CTRL_LA), FALSE);
									}
									else{
										EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_CTRL_LA), TRUE);
									}

								}

							break;
						}

						// CAL-09/10/02: automatically set last_selection to 0 when there're targets
						if (p->GetnonNULLNumLookAtTargets() < 1) { 
							// nothing is selected OR there are no targets
							// all buttons - target_delete, weight_edit, weight_spinner -- are disabled
							EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_WEIGHT_EDIT), FALSE);
							EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_WEIGHT_SPINNER), FALSE);
							ICustButton *iPickOb1;
							iPickOb1	= GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_LOOKAT_TARG));
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
							Control *cont = p->pblock->GetController(lookat_target_weight, selection); 
							// until it is animated, paramblock doesn't have a controller assigned in it yet.
							if (spin) {
								if ((cont != NULL) && cont->IsKeyAtTime(t, KEYAT_ROTATION))
									spin->SetKeyBrackets(TRUE);
								else
									spin->SetKeyBrackets(FALSE);
							}

							if (p->GetnonNULLNumLookAtTargets() == 1){
							// the "delete" button should be enabled
								ICustButton *iPickOb1;
								iPickOb1 = GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_LOOKAT_TARG));
								if (iPickOb1 != NULL){
									iPickOb1->Enable(TRUE);		
									ReleaseICustButton(iPickOb1);
								}
							// the rest are disabled
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_WEIGHT_EDIT), FALSE);
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_WEIGHT_SPINNER), FALSE);
							}
							
							else if (p->GetnonNULLNumLookAtTargets() > 1){ 
								// there are more than one targets
								// all buttons - delete, weight_edit, weight_spinner -- are enabled
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_WEIGHT_EDIT), TRUE);
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_WEIGHT_SPINNER), TRUE);
								ICustButton *iPickOb1;
								iPickOb1 = GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_LOOKAT_TARG));
								if (iPickOb1 != NULL){
									iPickOb1->Enable(TRUE);		
									ReleaseICustButton(iPickOb1);
								}
							}
						}

					
						ICustButton *iPickOb;
						iPickOb	= GetICustButton(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_PICKNODE));
						if (iPickOb != NULL){
							if (p->UpnodeWorld()) {
								iPickOb->Enable(FALSE);
//								iPickOb->SetText("None");
								if (p->pblock->Count(lookat_target_list) == 1) {
									EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_CTRL_LA), TRUE);
								}
							}
							else if (p->GetNode(0) == p->pblock->GetINode(lookat_upnode_pick, 0, 0) 
								&& iPickOb->IsChecked()){
								iPickOb->Enable(FALSE);
							}
							else {
								iPickOb->Enable(TRUE);
							}
							
						}
						ReleaseICustButton(iPickOb);

						iPickOb	= GetICustButton(GetDlgItem(p->hWnd, IDC_LOOKAT_SET_ORIENTATION_PICKNODE));
						ICustButton *iBBut;
						iBBut	= GetICustButton(GetDlgItem(p->hWnd, IDC_LOOKAT_TARG_PICKNODE));						
							if (iPickOb != NULL){
								if (iPickOb->IsChecked()){
									iBBut->Enable(FALSE);
								}
								else {
									iBBut->Enable(TRUE);
								}
								if (p->Relative()) {
									iPickOb->Enable(FALSE);	
								}
								else {
									iPickOb->Enable(TRUE);
								}
								ReleaseICustButton(iBBut);
								ReleaseICustButton(iPickOb);
						}

						if (p->pblock->GetInt(lookat_upnode_world) == 0){

							if (p->pblock->GetINode(lookat_upnode_pick) == NULL){

								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_X), FALSE);
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Y), FALSE);
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Z), FALSE);
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_X), FALSE);
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_Y), FALSE);
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_Z), FALSE);
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_FLIP), FALSE);

							}
							else if (p->pblock->GetINode(lookat_upnode_pick)){
								if (p->Get_upnode_control() ==  1){
									EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_X), TRUE);
									EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Y), TRUE);
									EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Z), TRUE);
								}
								else if (p->Get_upnode_control() ==  0){
									EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_X), FALSE);
									EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Y), FALSE);
									EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Z), FALSE);

								}
							}
						}
						else if (p->pblock->GetInt(lookat_upnode_world) == 1){

							switch (p->GetTargetAxis()){
								case 0: 
									EnableWindow(GetDlgItem(hWnd, IDC_LOOKAT_CONS_S_UP_X), FALSE);
									EnableWindow(GetDlgItem(hWnd, IDC_LOOKAT_CONS_S_UP_Y), TRUE);
									EnableWindow(GetDlgItem(hWnd, IDC_LOOKAT_CONS_S_UP_Z), TRUE);
								break;			// X

								case 1: 
									EnableWindow(GetDlgItem(hWnd, IDC_LOOKAT_CONS_S_UP_X), TRUE);
									EnableWindow(GetDlgItem(hWnd, IDC_LOOKAT_CONS_S_UP_Y), FALSE);
									EnableWindow(GetDlgItem(hWnd, IDC_LOOKAT_CONS_S_UP_Z), TRUE);
								break;	// Y

								case 2: 
									EnableWindow(GetDlgItem(hWnd, IDC_LOOKAT_CONS_S_UP_X), TRUE);
									EnableWindow(GetDlgItem(hWnd, IDC_LOOKAT_CONS_S_UP_Y), TRUE);
									EnableWindow(GetDlgItem(hWnd, IDC_LOOKAT_CONS_S_UP_Z), FALSE);
								break;	// Z
							}

							EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_FLIP), TRUE);

							if (p->Get_upnode_control() ==  1){
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_X), TRUE);
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Y), TRUE);
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Z), TRUE);
							}
							else if (p->Get_upnode_control() ==  0){
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_X), FALSE);
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Y), FALSE);
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_UPNODE_Z), FALSE);

							}
						}
//
//						if(p->TFlip()) EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_FLIP), FALSE);
//						else EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_S_UP_FLIP), TRUE);
//
//						if(p->SUFlip()) EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_TARGET_FLIP), FALSE);
//						else EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_TARGET_FLIP), TRUE);
						

	
						ReleaseISpinner(spin);
						p->RedrawListbox(GetCOREInterface()->GetTime(), selection);
						return TRUE;
							
				break;

				case WM_COMMAND:

					switch (LOWORD(wParam))
					{
						case IDC_LOOKAT_TARG_LIST:

							if (HIWORD(wParam) == LBN_SELCHANGE){
						
								selection = SendDlgItemMessage(hWnd, IDC_LOOKAT_TARG_LIST, LB_GETCURSEL, 0, 0);
								p->last_selection = selection;

								if (selection >= 0){
								// there is a valid selection 
								// automatically means there is AT LEAST one target
									if (p->GetnonNULLNumLookAtTargets() == 1){ 
									// the "delete" button should be enabled
										ICustButton *iPickOb1;
										iPickOb1 = GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_LOOKAT_TARG));
										if (iPickOb1 != NULL){
											iPickOb1->Enable(TRUE);		
											ReleaseICustButton(iPickOb1);
										}
									// the rest are disabled
									EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_WEIGHT_EDIT), FALSE);
									EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_WEIGHT_SPINNER), FALSE);
								}

								else  if (p->GetnonNULLNumLookAtTargets() > 1){ 
								// there are more than one targets
									// all buttons - delete, weight_edit, weight_spinner -- are enabled
									EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_WEIGHT_EDIT), TRUE);
									EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_WEIGHT_SPINNER), TRUE);
									ICustButton *iPickOb1;
									iPickOb1 = GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_LOOKAT_TARG));
									if (iPickOb1 != NULL){
										iPickOb1->Enable(TRUE);		
										ReleaseICustButton(iPickOb1);
									}

									ISpinnerControl* spin = GetISpinner(GetDlgItem(hWnd, IDC_LOOKAT_CONS_WEIGHT_SPINNER));
									float value = map->GetParamBlock()->GetFloat(lookat_target_weight, t, selection);
									spin->SetValue(value, FALSE);
									Control *cont = p->pblock->GetController(lookat_target_weight, selection); // until it is animated, paramblock doesn't have a controller assigned in it yet.
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
						
							else if (selection < 0 || p->GetnonNULLNumLookAtTargets() < 1){ 
							// nothing is selected OR there are no targets
								// all buttons - target_delete, weight_edit, weight_spinner -- are disabled
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_WEIGHT_EDIT), FALSE);
								EnableWindow(GetDlgItem(p->hWnd, IDC_LOOKAT_CONS_WEIGHT_SPINNER), FALSE);
								ICustButton *iPickOb1;
								iPickOb1	= GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_LOOKAT_TARG));
								if (iPickOb1 != NULL){
									iPickOb1->Enable(FALSE);		
									ReleaseICustButton(iPickOb1);
								}
							}
						}
						break;
					
						case IDC_LOOKAT_TARG_PICKNODE:

							thePickMode.p  = p;					
							p->ip->SetPickMode(&thePickMode);

						break;


						case IDC_REMOVE_LOOKAT_TARG:

							selection = SendDlgItemMessage(p->hWnd, IDC_LOOKAT_TARG_LIST, LB_GETCURSEL, 0, 0);
							p->last_selection = selection;
							if (selection >= 0){
								theHold.Begin();
								p->DeleteTarget(selection);
								theHold.Accept(GetString(IDS_AG_LOOKAT_LIST));
							}

						break;

						case IDC_LOOKAT_RESET_ORIENTATION_PICKNODE:

							p->userRotQuat.Identity();
							p->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE); // so that the change is updated

						break;


						case IDC_LOOKAT_SET_ORIENTATION_PICKNODE:

							ICustButton *iPickOb;
							iPickOb	= GetICustButton(GetDlgItem(p->hWnd, IDC_LOOKAT_SET_ORIENTATION_PICKNODE));
							if (iPickOb != NULL){
								if (p->Relative()) iPickOb->Enable(FALSE);	
								else iPickOb->Enable(TRUE);
								ReleaseICustButton(iPickOb);
							}
						break;

						}

				break;


				case CC_SPINNER_CHANGE:

					if (LOWORD(wParam) == IDC_LOOKAT_CONS_WEIGHT_SPINNER) {
						selection = SendDlgItemMessage(p->hWnd, IDC_LOOKAT_TARG_LIST, LB_GETCURSEL, 0, 0);

						// CAL-09/09/02: need to start a hold if it's not holding to handle type-in values
						BOOL isHold = theHold.Holding();
						if (!isHold) theHold.Begin();
						
						ISpinnerControl* spin = GetISpinner(GetDlgItem(hWnd, IDC_LOOKAT_CONS_WEIGHT_SPINNER));

						if (selection >= 0){
							float value = ((ISpinnerControl *)lParam)->GetFVal();
							map->GetParamBlock()->SetValue(lookat_target_weight, t, value, selection);

							Control *cont = p->pblock->GetController(lookat_target_weight, selection); // until it is animated, paramblock doesn't have a controller assigned in it yet.
							if (spin)
								spin->SetKeyBrackets(((cont != NULL) && cont->IsKeyAtTime(t,KEYAT_ROTATION)) ? TRUE : FALSE);
							
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
			UpdateLookAtName((LookAtConstRotation*)pb->GetOwner());
		}

		void DeleteThis() { }
		
		
};



	void LookAtConstRotation::RedrawListbox(TimeValue t, int sel){

		if (hWnd == NULL) return;
		if (!ip || editCont != this) return;
		if(!lookAtCD.NumParamMaps()) return;
		int selection = SendDlgItemMessage(hWnd, IDC_LOOKAT_TARG_LIST, LB_GETCURSEL, 0, 0);
		SendDlgItemMessage(hWnd, IDC_LOOKAT_TARG_LIST, LB_RESETCONTENT, 0, 0);
		int ts = 64;
		SendDlgItemMessage(hWnd, IDC_LOOKAT_TARG_LIST, LB_SETTABSTOPS, 1, (LPARAM)&ts);
		int ct = pblock->Count(lookat_target_list);
		int ctf = pblock->Count(lookat_target_weight);
		if (ct != ctf) return;		// CAL-09/10/02: In the middle of changing table size.

		for (int i = 0; i < ct; i++){
			if (pblock->GetINode(lookat_target_list, t, i) != NULL){					
				TSTR str;
				str.printf(_T("%-s\t%-d"), 
					pblock->GetINode(lookat_target_list, t, i)->GetName(),
					(int)pblock->GetFloat(lookat_target_weight, t, i));
				SendDlgItemMessage(hWnd, IDC_LOOKAT_TARG_LIST, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR) str.data());			
			}
		}

		if (ct > 0){
			
			if (sel >= 0){
				SendDlgItemMessage(hWnd, IDC_LOOKAT_TARG_LIST, LB_SETCURSEL, sel, 0);
			}
			else if (selection >= 0){
				SendDlgItemMessage(hWnd, IDC_LOOKAT_TARG_LIST, LB_SETCURSEL, selection, 0);
				last_selection = selection;
			}
			else {
				SendDlgItemMessage(hWnd, IDC_LOOKAT_TARG_LIST, LB_SETCURSEL, 0, 0);
				last_selection = 0;
			}

			ISpinnerControl* spin = GetISpinner(GetDlgItem(hWnd, IDC_LOOKAT_CONS_WEIGHT_SPINNER));
			if (last_selection >=0 && spin != NULL){
				float value = pblock->GetFloat(lookat_target_weight, GetCOREInterface()->GetTime(), last_selection);
				spin->SetValue(value, FALSE);
			}
			ReleaseISpinner(spin);
		}
		HWND hListBox = GetDlgItem(hWnd, IDC_LOOKAT_TARG_LIST);
		int extent = computeHorizontalExtent(hListBox, TRUE, 1, &ts);
		SendMessage(hListBox, LB_SETHORIZONTALEXTENT, extent, 0);
		UpdateWindow(hWnd);

	}

//static HWND hParamGizmos;

static LookAtConstDlgProc lookAtConstDlgProc;


static ParamBlockDesc2 lookAt_const_paramblk (lookAt_const_params, _T("LookAtConsParameters"),  0, &lookAtCD, P_AUTO_CONSTRUCT + P_AUTO_UI, LOOKAT_ROT_PBLOCK_REF, 
	//rollout
	IDD_LOOKAT_CONST_PARAMS, IDS_AG_LOOKATPARAMS, BEGIN_EDIT_MOTION, 0, &lookAtConstDlgProc,
	// params

	lookat_target_weight, 	_T("weight"), 	TYPE_FLOAT_TAB, 0,  P_ANIMATABLE + P_VARIABLE_SIZE+ P_TV_SHOW_ALL, 	IDS_AG_LOOKAT_WEIGHT_LIST, 
		p_default, 		50.0f, 
		p_range, 		0.0f, 100.0f, 
		p_accessor,		&lookAt_const_accessor,	
		p_enabled,		FALSE,
		end, 

	lookat_target_list,  _T(""),		TYPE_INODE_TAB, 0,	P_VARIABLE_SIZE,	IDS_AG_LOOKAT_LIST,
		p_accessor,		&lookAt_const_accessor,
		p_enable_ctrls,	1, lookat_target_weight,
		end,

	lookat_relative,	_T("relative"),			TYPE_BOOL,	P_RESET_DEFAULT,	IDS_AG_LOOKAT_CONS_RELATIVE,
		p_default, 		FALSE, 
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_LOOKAT_CONS_RELATIVE, 
		p_accessor,		&lookAt_const_accessor,
		end,

	lookat_vector_line_length, 	_T("lookat_vector_length"), TYPE_FLOAT, P_ANIMATABLE + P_RESET_DEFAULT, IDS_AG_LOOKAT_VECTOR_LENGTH,
		p_default, 		100.0f,	
		p_range, 		-100000.0f, 100000.0f, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_LOOKAT_CONS_LENGTH_EDIT, IDC_LOOKAT_CONS_LENGTH_SPINNER, 1.0f, 
		p_accessor,		&lookAt_const_accessor,	
		end,

	set_orientation, 	_T("set_orientation"), 	TYPE_BOOL, 	P_RESET_DEFAULT,	IDS_AG_LOOKAT_SET_ORIENTATION,
		p_default, 		FALSE, 
		p_ui, 			TYPE_CHECKBUTTON, IDC_LOOKAT_SET_ORIENTATION_PICKNODE, 
		p_accessor,		&lookAt_const_accessor,
		end, 

	lookat_target_axis, _T("target_axis"),		TYPE_INT, 	P_RESET_DEFAULT,	IDS_AG_LOOKAT_TARGET_AXIS,
		p_default, 		0, 
		p_range, 		0, 2, 
		p_ui, 			TYPE_RADIO, 	3, IDC_LOOKAT_CONS_TARGET_X, IDC_LOOKAT_CONS_TARGET_Y, IDC_LOOKAT_CONS_TARGET_Z, 
		p_accessor,		&lookAt_const_accessor,
		end, 

	lookat_target_axis_flip, _T("target_axisFlip"),	TYPE_BOOL,	P_RESET_DEFAULT,	IDS_AG_LOOKAT_TARGET_AXISFLIP,
		p_default, 		FALSE, 
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_LOOKAT_CONS_TARGET_FLIP, 
		p_accessor,		&lookAt_const_accessor,
		p_enabled,		TRUE,
		end, 

	lookat_upnode_axis, _T("upnode_axis"),		TYPE_INT, 	P_RESET_DEFAULT,	IDS_AG_LOOKAT_UPNODE_AXIS,
		p_default, 		2, 
		p_range, 		0, 2, 
		p_ui, 			TYPE_RADIO, 	3, IDC_LOOKAT_CONS_UPNODE_X, IDC_LOOKAT_CONS_UPNODE_Y, IDC_LOOKAT_CONS_UPNODE_Z, 
		p_accessor,		&lookAt_const_accessor,
		end, 

	lookat_upnode_world,	_T("upnode_world"), TYPE_BOOL,	P_RESET_DEFAULT,	IDS_AG_LOOKAT_UPNODE_W,
		p_default, 		TRUE, 
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_LOOKAT_CONS_UPNODE_W, 
		p_accessor,		&lookAt_const_accessor,
		end,

	lookat_upnode_pick,  _T("pickUpNode"), 		TYPE_INODE,  0,					IDS_AG_LOOKAT_UPNODE_PICKNODE,
		p_ui, 			TYPE_PICKNODEBUTTON, IDC_LOOKAT_CONS_UPNODE_PICKNODE,
		p_accessor,		&lookAt_const_accessor,
		p_prompt,		IDS_PICK_LOOKAT_UPNODE_PRMPT,
		end,
		
	lookat_StoUP_axis, 	_T("StoUP_axis"),		TYPE_INT, 	P_RESET_DEFAULT,	IDS_AG_LOOKAT_S_UP_AXIS,
		p_default, 		2, 
		p_range, 		0, 2, 
		p_ui, 			TYPE_RADIO, 3, IDC_LOOKAT_CONS_S_UP_X, IDC_LOOKAT_CONS_S_UP_Y, IDC_LOOKAT_CONS_S_UP_Z, 
		p_accessor,		&lookAt_const_accessor,
		end, 

	lookat_StoUP_axis_flip, _T("StoUP_axisFlip"),	TYPE_BOOL,	P_RESET_DEFAULT,	IDS_AG_LOOKAT_S_UP_AXISFLIP,
		p_default, 		FALSE, 
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_LOOKAT_CONS_S_UP_FLIP, 
		p_accessor,		&lookAt_const_accessor,
		end, 

	viewline_length_abs,	_T("viewline_length_abs"), TYPE_BOOL,	P_RESET_DEFAULT,	IDS_AG_LOOKAT_CONS_VL_ABS,
		p_default, 		TRUE, 
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_AG_LOOKAT_CONS_VL_ABS, 
		p_accessor,		&lookAt_const_accessor,
		end,

	lookat_upnode_control, 	_T("upnode_ctrl"),	TYPE_INT, 	P_RESET_DEFAULT,	IDS_AG_LOOKAT_CONS_UPNODE_CTRL,
		p_default, 		1, 
		p_range, 		0, 1, 
		p_ui, 			TYPE_RADIO, 2, IDC_LOOKAT_CONS_UPNODE_CTRL_LA, IDC_LOOKAT_CONS_UPNODE_CTRL_AA, 
		p_accessor,		&lookAt_const_accessor,
		end, 

	end
	);



LookAtConstRotation::LookAtConstRotation(BOOL loading) 
	{
	curRot.Identity();
	
	// make the paramblock
	lookAtCD.MakeAutoParamBlocks(this);
	pblock->CallSets();
	ivalid.SetEmpty();
	validator.mod = this; 
	initialLookAtFlag = 0;

	last_selection =  -1;
	baseRotQuatLocal.Identity();
	baseRotQuatWorld.Identity();
	userRotQuat.Identity();
	InitialLookAtQuat.Identity();
	oldTargetNumber = 0;
	sourceTM.IdentityMatrix();
	flags      = 0;
	hWnd = NULL;
}

LookAtConstRotation::~LookAtConstRotation()
	{
	DeleteAllRefsFromMe();
	}

RefTargetHandle LookAtConstRotation::Clone(RemapDir& remap)
	{
	LookAtConstRotation *p = new LookAtConstRotation(TRUE);

    p->ReplaceReference(LOOKAT_ROT_PBLOCK_REF, pblock->Clone(remap));


	p->curRot     = curRot;
	p->baseRotQuatLocal  = baseRotQuatLocal;
	p->baseRotQuatWorld  = baseRotQuatWorld;
	p->userRotQuat = userRotQuat;
	p->oldTargetNumber = oldTargetNumber;
	p->InitialLookAtQuat = InitialLookAtQuat; // LAM - 9/8/03 - defect 508329
	p->ivalid.SetEmpty();
	BaseClone(this, p, remap);
	return p;
	}


void LookAtConstRotation::Copy(Control *from)
	{
	Quat fvalRot;
	if (from->ClassID()==ClassID()) {
		LookAtConstRotation *ctrl = (LookAtConstRotation*)from;
		// a copy will construct its own pblock to keep the pblock-to-owner 1-to-1.
		RemapDir *remap = NewRemapDir(); 
		ReplaceReference(LOOKAT_ROT_PBLOCK_REF, ctrl->pblock->Clone(*remap));
		remap->DeleteThis();
		curRot   = ctrl->curRot;
	} else {
		from->GetValue(GetCOREInterface()->GetTime(), &fvalRot, Interval(0, 0));	//  to know the object lookat before the
		initialLookAtFlag = 1;
		baseRotQuatLocal = fvalRot;
		baseRotQuatLocal.Normalize(); // current controller was active
	}
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}




Point3 LookAtConstRotation::GetTargetPosition(TimeValue t)
	{
	Point3 average_target_position(0.0f, 0.0f, 0.0f);
	
	int ct = pblock->Count(lookat_target_list);

	total_lookat_target_weight = 0.0f;

	for (int i = 0; i < ct; i++) {
		INode *lookat_target;
		float targetWt = 0.0f;
		Point3 lookat_target_position;
		pblock->GetValue(lookat_target_list, t, lookat_target, FOREVER, i);
		if (lookat_target == NULL) continue; // skip the for loop if the selected object is null  

		Matrix3 lookat_targetTM;
		lookat_targetTM = lookat_target->GetNodeTM(t, &ivalid);
		lookat_target_position = lookat_targetTM.GetTrans();
		pblock->GetValue(lookat_target_weight, t, targetWt, ivalid, i);
		total_lookat_target_weight += targetWt;

		average_target_position += targetWt * lookat_target_position;
	}

	if (total_lookat_target_weight >= 0.0){
		average_target_position =  average_target_position/total_lookat_target_weight;
	}

	return average_target_position;
}

Quat LookAtConstRotation::ComputeLookAtQuat(TimeValue t){

	Matrix3 temp_mat(true);
	Point3 xdir, ydir, zdir;
	Point3 source_to_upnode_vector;

	temp_mat.IdentityMatrix();
	if(pblock->Count(lookat_target_list)>0){
		Point3 source_pos = sourceTM.GetTrans();
		Point3 target_pos = GetTargetPosition(t);
		Point3 source_target_unit_vector = target_pos - source_pos;
		source_target_unit_vector = Normalize(source_target_unit_vector);
		if(TFlip())	source_target_unit_vector = -source_target_unit_vector;

		INode *upn;
		Matrix3 upTM;
		upTM.IdentityMatrix();
		source_to_upnode_vector = Normalize(upTM.GetTrans() - sourceTM.GetTrans());
	
		pblock->GetValue(lookat_upnode_pick, 0, upn, FOREVER);


		if (GetUpnodeWorld()) {
			upTM.IdentityMatrix();
			source_to_upnode_vector = Normalize(upTM.GetTrans() - sourceTM.GetTrans());
		}
		
		else if (upn != NULL){
			upTM = upn->GetNodeTM(t, &ivalid);
			source_to_upnode_vector = Normalize(upTM.GetTrans() - sourceTM.GetTrans());
			if (pblock->Count(lookat_target_list) == 1 && Get_upnode_control() == 0){
				INode *the_only_target;
				pblock->GetValue(lookat_target_list, t, the_only_target, FOREVER, 0);
				if(upn == the_only_target){
					upn = NULL;
					upTM.IdentityMatrix();
					source_to_upnode_vector = Normalize(upTM.GetTrans() - sourceTM.GetTrans());
				}
			}
		}
		else {
			upTM.IdentityMatrix();
			source_to_upnode_vector = Normalize(upTM.GetTrans() - sourceTM.GetTrans());
		}
//		else if (upn != NULL){
//			upTM = upn->GetNodeTM(t, &ivalid);
//			source_to_upnode_vector = Normalize(upTM.GetTrans() - sourceTM.GetTrans());
//		}
//		else{	// to cover all cases
//			upTM.IdentityMatrix();
//			source_to_upnode_vector = Normalize(upTM.GetTrans() - sourceTM.GetTrans());
//		}


		switch (GetTargetAxis()) { //	GetTargetAxis() == GetUpNodeAxis() represents an incorrect choice
			case 0:  // x-axis coincident with the source-target line
				xdir = source_target_unit_vector;
					if (Get_StoUPAxis()== 0){ // an incorrect case
						ydir = Normalize(CrossProd(Point3(1,0,0), xdir));
						zdir = Normalize(CrossProd(xdir, ydir));
					}
//					else 
					if (Get_StoUPAxis()== 1){
						if (Get_upnode_control() == 1){
							ydir = Normalize(ProjectionOnPerpPlane(upTM.GetRow(GetUpNodeAxis()),xdir));
						}
						else if (Get_upnode_control() == 0){
							ydir = Normalize(ProjectionOnPerpPlane(source_to_upnode_vector,xdir));
						}
						if(SUFlip()) ydir = -ydir;
						zdir = Normalize(CrossProd(xdir, ydir));
					}
					else if (Get_StoUPAxis()== 2) {
						if (Get_upnode_control() == 1){
							zdir = Normalize(ProjectionOnPerpPlane(upTM.GetRow(GetUpNodeAxis()),xdir));
						}
						else if (Get_upnode_control() == 0){
							zdir = Normalize(ProjectionOnPerpPlane(source_to_upnode_vector,xdir));
						}
						if(SUFlip()) zdir = -zdir;
						ydir = Normalize(CrossProd(zdir, xdir));
					}
				
			break;

			case 1: // y-axis coincident with the source-target line
				ydir = source_target_unit_vector;
				
					if (Get_StoUPAxis()== 0){
						if (Get_upnode_control() == 1){
							xdir = Normalize(ProjectionOnPerpPlane(upTM.GetRow(GetUpNodeAxis()), ydir));
						}
						else if (Get_upnode_control() == 0){
							xdir = Normalize(ProjectionOnPerpPlane(source_to_upnode_vector,ydir));
						}
						if(SUFlip()) xdir = -xdir;
						zdir = Normalize(CrossProd(xdir, ydir));
					}
					else if (Get_StoUPAxis()== 1){ // an incorrect case
						xdir = Normalize(CrossProd(Point3(0,1,0), ydir));
						zdir = Normalize(CrossProd(xdir, ydir));
					}
					else if (Get_StoUPAxis()== 2){
						if (Get_upnode_control() == 1){
							zdir = Normalize(ProjectionOnPerpPlane(upTM.GetRow(GetUpNodeAxis()), ydir));
						}
						else if (Get_upnode_control() == 0){
							zdir = Normalize(ProjectionOnPerpPlane(source_to_upnode_vector, ydir));
						}
						if(SUFlip()) zdir = -zdir;
						xdir = Normalize(CrossProd(ydir, zdir));
					}
				
			break;

			case 2: // z-axis coincident with the source-target line
				zdir = source_target_unit_vector;
					if (Get_StoUPAxis()== 0){
						if (Get_upnode_control() == 1){
							xdir = Normalize(ProjectionOnPerpPlane(upTM.GetRow(GetUpNodeAxis()), zdir));
						}
						else if (Get_upnode_control() == 0){
							xdir = Normalize(ProjectionOnPerpPlane(source_to_upnode_vector, zdir));
						}
						if(SUFlip()) xdir = -xdir;
						ydir = Normalize(CrossProd(zdir, xdir));
					}
					else if (Get_StoUPAxis()== 1){
						if (Get_upnode_control() == 1){
							ydir = Normalize(ProjectionOnPerpPlane(upTM.GetRow(GetUpNodeAxis()), zdir));
						}
						else if (Get_upnode_control() == 0){
							ydir = Normalize(ProjectionOnPerpPlane(source_to_upnode_vector, zdir));
						}
						if(SUFlip()) ydir = -ydir;
						xdir = Normalize(CrossProd(ydir, zdir));
					}
					else { // an incorrect case
						xdir = Normalize(CrossProd(Point3(0,0,1), zdir));
						ydir = Normalize(CrossProd(zdir, xdir));
				}
			break;
		}
	}


	temp_mat.SetRow(0, xdir);
	temp_mat.SetRow(1, ydir);
	temp_mat.SetRow(2, zdir);
	Quat temp_quat;
	temp_quat.Set(temp_mat);
	temp_quat.MakeClosest(IdentQuat());
	temp_quat.Normalize();
	return temp_quat;

}


void LookAtConstRotation::Update(TimeValue t){

	ivalid = FOREVER;
	int ct = pblock->Count(lookat_target_list);
	int ctf = pblock->Count(lookat_target_weight);
	if (ctf  !=  ct){
		pblock->SetCount(lookat_target_weight, ct);
		RedrawListbox(t);
	}

	if (ct != oldTargetNumber) {
		InitialLookAtQuat = ComputeLookAtQuat(t);
	}

	if(ct > 0){

		curRot =   ComputeLookAtQuat(t) * userRotQuat;
		if (Relative()){
			
			curRot =   baseRotQuatWorld * (ComputeLookAtQuat(t) / InitialLookAtQuat);
		}
	}
	else{

		curRot = baseRotQuatWorld;
	}

	curRot.Normalize();
	oldTargetNumber = ct;
//	if (ivalid.Empty()) ivalid.SetInstant(t);
}

void LookAtConstRotation::SetValue(TimeValue t, void *val, int commit, GetSetMethod method)
{
	HoldQuat();			// this is where the LookAtConstRotationRestore is called
						// to properly manage the the object rotation in the set
						// orientation mode. Works in concert with RestoreObj 
						// and HoldQuat functions
	Quat deltaRotQuat;
	deltaRotQuat.Identity();
	if (Get_SetOrientation()){
		if (method==CTRL_RELATIVE) {
			AngAxis *AngAx = (AngAxis*)val;
			deltaRotQuat = QFromAngAxis(AngAx->angle, AngAx->axis);
			userRotQuat *= deltaRotQuat;
			ivalid.SetEmpty();
			NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);	
		} 
		else //CTRL_ABSOLUTE
		{
  			*((Quat*)val) = curRot;
		}
	}
}


void LookAtConstRotation::GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{


	 	sourceTM = *(Matrix3*)val;		// has only the relevant position part. The rotation part 
								// will be set by this constraint controller

		if(initialLookAtFlag){
			Matrix3 tempMat(1);
			// CAL-9/26/2002: in absolute mode the value could be un-initialized (random value).
			if (method == CTRL_RELATIVE) tempMat = *(Matrix3*)val;
			AffineParts comps;		// Requires header decomp.h
			decomp_affine(tempMat, &comps);
			baseRotQuatWorld = baseRotQuatLocal * comps.q;
			baseRotQuatWorld.Normalize();
			initialLookAtFlag = 0;
		}

//		if (!ivalid.InInterval(t))
			Update(t);
//		}
		valid &= ivalid;	 
		if (method==CTRL_RELATIVE) {
			int ct = pblock->Count(lookat_target_list);
//			if (ct > 1){
//				Matrix3 *mat = (Matrix3*)val;		// this is source_Parent_TM
//				Quat q = baseRotQuatLocal;			// this is target_world_TM
//				PreRotateMatrix(*mat, q);
//			}
//			else{
				Matrix3 *mat = (Matrix3*)val;
				// CAL-06/24/02: preserve all components and replace with the new orientation.
				AffineParts ap;
				decomp_affine( *mat, &ap );
				ap.q = curRot;				// target world rotation
				comp_affine( ap, *mat );
				// CAL-06/24/02: this only preserve the translation component
				// Point3 tr; 
				// tr = mat->GetTrans();
				// Quat q = curRot;			
				// q.MakeMatrix(*mat);
				// mat->SetTrans(tr);	
//			}
		} else {
			*((Quat*)val) = curRot;
		}

	}

int LookAtConstRotation::Display(
		TimeValue t, INode* inode, ViewExp *vpt, int flags)
	{

		Interval iv;
		GraphicsWindow *gw = vpt->getGW();	// This line is here because I don't know how to initialize 
											// a *gw. I will change it in the next line
		gw->setTransform(Matrix3(1));		// sets the graphicsWindow to world

		bbox.Init();
		int ct = pblock->Count(lookat_target_list);
		if (ct <= 0) return 0;

		// draw dotted lines from the source to each of the target positions

		
		for (int i = 0; i < ct; i++) {
			Point3 sourcePosition = sourceTM.GetTrans();
			INode *lookat_target;
			DWORD targColor;
			float targetWt;
//watje don't need to set the ivalid in the display loop
			pblock->GetValue(lookat_target_weight, t, targetWt, FOREVER, i);

			Point3 targetPosition;
			pblock->GetValue(lookat_target_list, t, lookat_target, FOREVER, i);
			if (lookat_target == NULL) continue; // skip the for loop if the selected object is null  
			Matrix3 lookat_targetTM;
			lookat_targetTM = lookat_target->GetNodeTM(t, &iv);
			targetPosition = lookat_targetTM.GetTrans();


			Point3 source_target_vector = (targetPosition - sourcePosition);
			Point3 source_target_unit_vector = Normalize(source_target_vector);
			float source_target_dist = FLength(source_target_vector);
			float vector_length;
			pblock->GetValue(lookat_vector_line_length, t, vector_length, iv);				
			if (VLAbs()){
				targetPosition = sourcePosition + ct * source_target_unit_vector * (vector_length/total_lookat_target_weight) * targetWt;
			}
			else{
				targetPosition = sourcePosition + source_target_dist* source_target_unit_vector* (vector_length/100.0f);
			}
			targColor =  lookat_target->GetWireColor();

			float targ_r = (float) GetRValue(targColor) / 255.0f;;

			float targ_g = (float) GetGValue(targColor) / 255.0f;

			float targ_b = (float) GetBValue(targColor) / 255.0f;
			gw->setColor(LINE_COLOR, targ_r, targ_g, targ_b);


			dLine2Pts(gw,sourcePosition, targetPosition);
			bbox += targetPosition;
			IParamMap2* pmap = pblock->GetMap();
			if (pmap!=NULL) pmap->RedrawViews(GetCOREInterface()->GetTime());
		}

		// draw a solid line from the source to the resultant target position

		if(pblock->Count(lookat_target_list)>0){
			Point3 r = sourceTM.GetTrans();
			Point3 q = GetTargetPosition(t);
			Point3 source_target_vector = (q-r);
			Point3 source_target_unit_vector = Normalize(source_target_vector);
			float source_target_dist = FLength(source_target_vector);
			float vector_length;
			pblock->GetValue(lookat_vector_line_length, t, vector_length, iv);			
			if (VLAbs()){
				q = r + source_target_unit_vector * vector_length;
			}
			else{
				q = r + source_target_dist* source_target_unit_vector* (vector_length/100.0f);
			}

			gw->setColor(LINE_COLOR, GetUIColor(COLOR_TARGET_LINE));

			dLine2Pts(gw,r,q);
			bbox += q;
			UpdateWindow(hWnd);

		}




		
		return 1;
	}

void LookAtConstRotation::GetWorldBoundBox(TimeValue t, INode *inode, ViewExp* vpt, Box3& box)
	{ 
//		box.Init();
//		Matrix3 m = inode->GetNodeTM(t);
//		box += m.GetRow(3);
		box += bbox;
//		box = bbox * m;

	}


Point3 LookAtConstRotation::ProjectionOnPerpPlane(Point3 Vector_ProjectionOf, Point3 Vector_OnPlanePerpTo){
	Point3 xAxis;
	xAxis = CrossProd(Vector_OnPlanePerpTo, Vector_ProjectionOf);

	if (FLength(xAxis) < more_than_and_very_close_to_zero){

		int ix = Vector_OnPlanePerpTo.MinComponent();
		xAxis = Point3(0,0,0); 
		xAxis[ix] = 1;
		Point3 cp = CrossProd(xAxis, Vector_OnPlanePerpTo);
		xAxis = Normalize(CrossProd(Vector_OnPlanePerpTo, cp));
	}

	Point3 yAxis = CrossProd(xAxis, Vector_OnPlanePerpTo);
	return Normalize(yAxis);




//	float var = DotProd(Vector_ProjectionOf, Vector_OnPlanePerpTo);
//	if ((float)fabs(var) < more_than_and_very_close_to_zero || (float)fabs(var) > less_than_and_very_close_to_one){
//		Vector_ProjectionOf = Vector_ProjectionOf + Normalize(Point3(more_than_and_very_close_to_zero, more_than_and_very_close_to_zero, more_than_and_very_close_to_zero));
//	}
//	Point3 vec = Vector_ProjectionOf - Vector_OnPlanePerpTo * DotProd(Vector_ProjectionOf, Vector_OnPlanePerpTo);
//	Point3 vec = CrossProd(Vector_ProjectionOf,Vector_OnPlanePerpTo); 
//	return  Normalize(vec);
}


RefTargetHandle LookAtConstRotation::GetReference(int i)
	{
		switch (i)
		{
			case LOOKAT_ROT_PBLOCK_REF:
				return pblock;
		}
		return NULL;
	}

void LookAtConstRotation::SetReference(int i, RefTargetHandle rtarg)
	{
		switch (i)
		{
			case LOOKAT_ROT_PBLOCK_REF:
				pblock = (IParamBlock2*)rtarg; break;
		}
	}

RefResult LookAtConstRotation::NotifyRefChanged (Interval iv, RefTargetHandle hTarg, PartID& partID, RefMessage msg) 
	{

	switch (msg) {
		case REFMSG_CHANGE:
		{
			if (hTarg == pblock){
				ParamID changing_param = pblock->LastNotifyParamID();
				switch (changing_param)
				{
					// CAL-09/09/02: need to redraw the list box specifically.
					// TODO: Most of the other calls to RedrawListbox() should be removed with this addition.
					case lookat_target_weight:
						RedrawListbox(GetCOREInterface()->GetTime());
						break;
					default:
						lookAt_const_paramblk.InvalidateUI(changing_param);
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

class LookAtConstRotationRestore : public RestoreObj {
	public:
		Quat undo, redo;
		LookAtConstRotation *cont;
		LookAtConstRotationRestore(LookAtConstRotation *c) {
			cont = c;
			undo   = c->userRotQuat;
		}
		void Restore(int isUndo) {
//			if (cont->editCont == cont) {
//				lookAt_const_paramblk.InvalidateUI();
//			}
			redo   = cont->userRotQuat;
			cont->userRotQuat  = undo;
			cont->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		}
		void Redo() {
			cont->userRotQuat = redo;
			cont->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		}

		void EndHold() {cont->ClearAFlag(A_HELD);}
	};

void LookAtConstRotation::HoldQuat(){
	if (theHold.Holding()&&!TestAFlag(A_HELD)) {		
		theHold.Put(new LookAtConstRotationRestore(this));
		SetAFlag(A_HELD);
		}
	}


int LookAtConstRotation::GetNumTargets(){
	return pblock->Count(lookat_target_list);
}

int LookAtConstRotation::GetnonNULLNumLookAtTargets(){
	int no_of_nonNULL_nodes = 0;
	for (int i = 0; i < pblock->Count(lookat_target_list); ++i){
		if (pblock->GetINode(lookat_target_list, GetCOREInterface()->GetTime(), i) != NULL){
			no_of_nonNULL_nodes = no_of_nonNULL_nodes + 1;
		}			
	}
	return no_of_nonNULL_nodes;
}


INode* LookAtConstRotation::GetNode(int nodeNumber){
	if (nodeNumber >= 0 && nodeNumber < pblock->Count(lookat_target_list)){
		INode *lookAt_targ;
		pblock->GetValue(lookat_target_list, 0, lookAt_targ, FOREVER, nodeNumber);
		return lookAt_targ;
	}
	return NULL;
}

BOOL LookAtConstRotation::AppendTarget(INode *target, float weight){

	if (target->TestForLoop(FOREVER,(ReferenceMaker *) this)== REF_SUCCEED){
		for (int i = 0; i < pblock->Count(lookat_target_list); ++i){
			if (target == pblock->GetINode(lookat_target_list, GetCOREInterface()->GetTime(), i)){
				return FALSE;
			}
		}
		theHold.Begin();
		pblock->Append(lookat_target_list, 1, &target, 1);
		pblock->Append(lookat_target_weight, 1, &weight, 1);
		theHold.Accept(GetString(IDS_AG_LOOKAT_LIST));
		return TRUE;
	}
	return FALSE;

}

BOOL LookAtConstRotation::DeleteTarget(int selection){
	int ct = pblock->Count(lookat_target_list);
 	if (selection >= 0 && selection < pblock->Count(lookat_target_list)){
//		theHold.Begin();
		pblock->Delete(lookat_target_weight, selection, 1);
		pblock->Delete(lookat_target_list, selection, 1);
//		theHold.Accept(GetString(IDS_AG_LOOKAT_LIST));
		return TRUE;
	}
	else
		return FALSE;
}

/*
BOOL LookAtConstRotation::SetTarget(INode *target, int targetNumber){
	if (target->TestForLoop(FOREVER,this)==REF_SUCCEED) {

		if (targetNumber >= 0 && targetNumber <= pblock->Count(lookat_target_list)) {
			pblock->SetValue(lookat_target_list, 0, target, targetNumber);
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

float LookAtConstRotation::GetTargetWeight(int targetNumber){
	if (targetNumber >= 0 && targetNumber < pblock->Count(lookat_target_list)){
		float targetWt;
		pblock->GetValue(lookat_target_weight, GetCOREInterface()->GetTime(), targetWt, FOREVER, targetNumber);
		return targetWt;
	}
	else{
		return 0.0f;
	}
}

BOOL LookAtConstRotation::SetTargetWeight(int targetNumber, float weight){
	if (targetNumber >= 0 && targetNumber < pblock->Count(lookat_target_list)){
		pblock->SetValue(lookat_target_weight, GetCOREInterface()->GetTime(), weight, targetNumber);
		return TRUE;
	}
	else{
		return FALSE;
	}
}

void LookAtConstRotation::SetRelative(BOOL rel)
	{
	pblock->SetValue(lookat_relative, GetCOREInterface()->GetTime(), rel);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}

void LookAtConstRotation::SetVLisAbs(BOOL rel)
	{
	pblock->SetValue(viewline_length_abs, GetCOREInterface()->GetTime(), rel);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}

void LookAtConstRotation::SetUpnodeWorld(BOOL uw)
	{
	pblock->SetValue(lookat_upnode_world, GetCOREInterface()->GetTime(), uw);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}

void LookAtConstRotation::SetTargetAxisFlip(BOOL rel)
	{
	pblock->SetValue(lookat_target_axis_flip, GetCOREInterface()->GetTime(), rel);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}

void LookAtConstRotation::SetStoUPAxisFlip(BOOL rel)
	{
	pblock->SetValue(lookat_StoUP_axis_flip, GetCOREInterface()->GetTime(), rel);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}


void LookAtConstRotation::Set_SetOrientation(BOOL rel)
	{
	pblock->SetValue(set_orientation, GetCOREInterface()->GetTime(), rel);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}

BOOL LookAtConstRotation::SetOrientation(){
	Interval iv;
	BOOL isSetOrient;
	pblock->GetValue(set_orientation, GetCOREInterface()->GetTime(), isSetOrient, iv);
	return isSetOrient;
}

BOOL LookAtConstRotation::SUFlip(){
	Interval iv;
	BOOL isSUFlip;
	pblock->GetValue(lookat_StoUP_axis_flip, GetCOREInterface()->GetTime(), isSUFlip, iv);
	return isSUFlip;	
}

BOOL LookAtConstRotation::TFlip(){
	Interval iv;
	BOOL isTFlip;
	pblock->GetValue(lookat_target_axis_flip, GetCOREInterface()->GetTime(), isTFlip, iv);
	return isTFlip;	
}

BOOL LookAtConstRotation::Relative(){
	Interval iv;
	BOOL isRelative;
	pblock->GetValue(lookat_relative, GetCOREInterface()->GetTime(), isRelative, iv);
	return isRelative;
}

BOOL LookAtConstRotation::UpnodeWorld(){
	Interval iv;
	BOOL isUpnode;
	pblock->GetValue(lookat_upnode_world, GetCOREInterface()->GetTime(), isUpnode, iv);
	return isUpnode;
}

BOOL LookAtConstRotation::VLAbs(){
	Interval iv;
	BOOL isAbs;
	pblock->GetValue(viewline_length_abs, GetCOREInterface()->GetTime(), isAbs, iv);
	return isAbs;
}

void LookAtConstRotation::Set_Reset_Orientation()
	{
	userRotQuat.Identity();	
	Update(GetCOREInterface()->GetTime());
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}

int LookAtConstRotation::GetTargetAxis()
	{
	Interval iv;
	int targeAxisSel;
	pblock->GetValue(lookat_target_axis, GetCOREInterface()->GetTime(), targeAxisSel, iv);
	return targeAxisSel;
	}

int LookAtConstRotation::GetUpNodeAxis()
	{
	Interval iv;
	int upnodeAxisSel;
	pblock->GetValue(lookat_upnode_axis, GetCOREInterface()->GetTime(), upnodeAxisSel, iv);
	return upnodeAxisSel;
	}

int LookAtConstRotation::Get_StoUPAxis()
	{ 
	Interval iv;
	int StoUPAxisSel;
	pblock->GetValue(lookat_StoUP_axis, GetCOREInterface()->GetTime(), StoUPAxisSel, iv);
	return StoUPAxisSel;
	}

int LookAtConstRotation::Get_upnode_control()
	{
	Interval iv;
	int ucontrol;
	pblock->GetValue(lookat_upnode_control, GetCOREInterface()->GetTime(), ucontrol, iv);
	return ucontrol;
	}

void LookAtConstRotation::SetTargetAxis(int axis)
	{
	pblock->SetValue(lookat_target_axis, GetCOREInterface()->GetTime(), axis);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}

void LookAtConstRotation::SetUpNodeAxis(int axis)
	{
	pblock->SetValue(lookat_upnode_axis, GetCOREInterface()->GetTime(), axis);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}

void LookAtConstRotation::Set_StoUPAxis(int axis)
	{
	pblock->SetValue(lookat_StoUP_axis, GetCOREInterface()->GetTime(), axis);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}

void LookAtConstRotation::Set_upnode_control(int ucontrol)
	{
	pblock->SetValue(lookat_upnode_control, GetCOREInterface()->GetTime(), ucontrol);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}


/*--------------------------------------------------------------------*/
// LookAtConstRotation UI

void LookAtConstRotation::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
	{	
	this->ip = ip;
	editCont = this;
	if (flags & BEGIN_EDIT_HIERARCHY) {
// JBW: JointParam stuff not handled by ParamBlock2 yet		
		// No IK if follow is on
			InterpCtrlUI *ui;
			ui = new InterpCtrlUI(NULL,ip,this);
			DWORD f=0;	
			
		SetProperty(PROPID_INTERPUI,ui);		
	} else {
		lookAtCD.BeginEditParams(ip, this, flags, prev);
		}

	ip->RegisterTimeChangeCallback(&lookAtConstTimeChangeCallback);
	lookAtConstTimeChangeCallback.lookAt_controller = this;


	IParamMap2* pmap = pblock->GetMap();
	if (pmap) hWnd = pmap->GetHWnd();

	int ct = pblock->Count(lookat_target_list);
	if (last_selection < 0){
		RedrawListbox(GetCOREInterface()->GetTime());
	}
	else {
		RedrawListbox(GetCOREInterface()->GetTime(), last_selection);
		ISpinnerControl* spin = GetISpinner(GetDlgItem(hWnd, IDC_LOOKAT_CONS_WEIGHT_SPINNER));
		if (spin != NULL && ct > 0){
			float value = pblock->GetFloat(lookat_target_weight, GetCOREInterface()->GetTime(), last_selection);
			spin->SetValue(value, FALSE);
		}
		ReleaseISpinner(spin);
	}
	}

void LookAtConstRotation::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
{
	this->ip = NULL;
	editCont = NULL;
	IParamMap2* pmap = pblock->GetMap();
	ip->ClearPickMode(); // need this, otherwise will crash on undo, while pickmode is active.
	
	if (pmap != NULL)
	{
		if (next && next->ClassID() == ClassID() && ((LookAtConstRotation*)next)->pblock)
		{
			pmap->SetParamBlock(((LookAtConstRotation*)next)->pblock);
			ip->ClearPickMode();
			//pmap->UpdateUI(GetCOREInterface()->GetTime());
			RedrawListbox(GetCOREInterface()->GetTime());
		}
		else 
			lookAtCD.EndEditParams(ip, this, flags | END_EDIT_REMOVEUI, next);
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
		ip->UnRegisterTimeChangeCallback(&lookAtConstTimeChangeCallback);
		hWnd = NULL;
		
	
}

TSTR LookAtConstRotation::SvGetRelTip(IGraphObjectManager *gom, IGraphNode *gNodeTarger, int id, IGraphNode *gNodeMaker)
{
	return SvGetName(gom, gNodeMaker, false) + " -> " + gNodeTarger->GetAnim()->SvGetName(gom, gNodeTarger, false);
}

bool LookAtConstRotation::SvCanDetachRel(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker)
{
	return true;
}

bool LookAtConstRotation::SvDetachRel(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker)
{
	for( int i=0; i<GetNumTargets(); i++ ) {
		if( GetNode(i) == gNodeTarget->GetAnim() ) {
			DeleteTarget(i);
			return true;
		}
	}
	return false;
}

SvGraphNodeReference LookAtConstRotation::SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags)
{
	SvGraphNodeReference nodeRef = Control::SvTraverseAnimGraph( gom, owner, id, flags );

	if( nodeRef.stat == SVT_PROCEED ) {
		for( int i=0; i<GetNumTargets(); i++ ) {
			gom->AddRelationship( nodeRef.gNode, GetNode(i), i, RELTYPE_CONSTRAINT );
		}
	}

	return nodeRef;
}

bool LookAtConstRotation::SvCanConcludeLink(IGraphObjectManager *gom, IGraphNode *gNode, IGraphNode *gNodeChild)
{
	Animatable *pChildAnim = gNodeChild->GetAnim();
	if( pChildAnim->SuperClassID() == BASENODE_CLASS_ID )
		return true;

	return Control::SvCanConcludeLink(gom, gNode, gNodeChild);
}

bool LookAtConstRotation::SvLinkChild(IGraphObjectManager *gom, IGraphNode *gNodeThis, IGraphNode *gNodeChild)
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

int LookAtConstRotation::SetProperty(ULONG id, void *data)
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

void* LookAtConstRotation::GetProperty(ULONG id)
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

#define OFFSETQUAT_LOCAL_CHUNK		0x1001
#define OFFSETQUAT_WORLD_CHUNK		0x1002
#define INITIAL_QUAT_CHUNK			0x1003
#define USEROTQUAT_CHUNK			0x1004
#define OLD_TARGET_NUMBER_CHUNK		0x1005


IOResult LookAtConstRotation::Save(ISave *isave)
	{	
		ULONG nb;

		// Save baseRotQuat
		isave->BeginChunk(OFFSETQUAT_LOCAL_CHUNK);
		isave->Write(&baseRotQuatLocal, sizeof(baseRotQuatLocal), &nb);
		isave->EndChunk();

		// Save baseRotQuat
		isave->BeginChunk(OFFSETQUAT_WORLD_CHUNK);
		isave->Write(&baseRotQuatWorld, sizeof(baseRotQuatWorld), &nb);
		isave->EndChunk();

		// Save InitialLookAtQuat
		isave->BeginChunk(INITIAL_QUAT_CHUNK);
		isave->Write(&InitialLookAtQuat,sizeof(InitialLookAtQuat),&nb);	
		isave->EndChunk();

		// Save userRotQuat
		isave->BeginChunk(USEROTQUAT_CHUNK);
		isave->Write(&userRotQuat,sizeof(userRotQuat),&nb);	
		isave->EndChunk();

		// Save oldTargetNumber
		isave->BeginChunk(OLD_TARGET_NUMBER_CHUNK);
		isave->Write(&oldTargetNumber,sizeof(oldTargetNumber),&nb);	
		isave->EndChunk();
	
	return IO_OK;
	}

IOResult LookAtConstRotation::Load(ILoad *iload)
	{
	ULONG nb;
	IOResult res = IO_OK;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case OFFSETQUAT_LOCAL_CHUNK:
				// Read a block of bytes from the output stream.
				res=iload->Read(&baseRotQuatLocal, sizeof(baseRotQuatLocal), &nb);
			break;

			case OFFSETQUAT_WORLD_CHUNK:
				// Read a block of bytes from the output stream.
				res=iload->Read(&baseRotQuatWorld, sizeof(baseRotQuatWorld), &nb);
			break;

			case INITIAL_QUAT_CHUNK:
				// Read a block of bytes from the output stream.
				res=iload->Read(&InitialLookAtQuat, sizeof(InitialLookAtQuat), &nb);
			break;

			case USEROTQUAT_CHUNK:
				// Read a block of bytes from the output stream.
				res=iload->Read(&userRotQuat, sizeof(userRotQuat), &nb);
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
