/**********************************************************************
 *
	FILE: Path_cnstrnt.cpp

	DESCRIPTION: A controller that moves an object along a spline path
				 Ed. 2 re-coded using ParamBlock2/ParamMap2

	CREATED BY: Rolf Berteig
	            Ed. 2 John Wainwright

  MODIFIED AND ENHANCED BY: Ambarish Goswami 2/2000
		FEATURES ADDED:
				Multiple path with individual animatable weights
				Viewport manipulation capability (Thanks to Peter Watje's help)
				Loop (/no_loop)checkbox
				Relative (/Absolute) option for the object


	HISTORY:	created 13 June 1995
				Ed. 2 re-coded 9/22/98
				Ed. 3 re-coded 3/6/2000

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "ctrl.h"
#include "units.h"
#include "interpik.h"
#include "spline3d.h"
#include "istdplug.h"
#include "iparamm2.h"
#include "decomp.h"


#define PATH_CONTROL_CNAME		GetString(IDS_RB_PATH) // Class name, MaxScript uses this

#define InterpType() ((ConstVel())?SPLINE_INTERP_NORMALIZED:SPLINE_INTERP_SIMPLE)

// flag values
#define PATHFLAG_FOLLOW		(1<<0)
#define PATHFLAG_BANK		(1<<1)
#define PATHFLAG_CLOSED		(1<<2)
#define PATHFLAG_ALLOWFLIP	(1<<3)
#define PATHFLAG_CONSTVEL	(1<<4)
#define PATHFLAG_FLIP		(1<<5)
#define PATHFLAG_YAXIS		(1<<6)
#define PATHFLAG_ZAXIS		(1<<7)

// AG added
#define PATHFLAG_LOOP		(1<<8)
#define PATHFLAG_RELATIVE	(1<<9)


// path_params param IDs
//enum {	path_percent,			path_path,			path_follow,  
//		path_bank,				path_bank_amount,	path_smoothness, 
//		path_allow_upsidedown,	path_constant_vel,	path_axis, 
//		path_axis_flip,			path_path_weight,	path_path_list, 
//		path_loop,				path_relative,};
	   
class PathPosition; 
class PathTimeChangeCallback : public TimeChangeCallback
{
	public:
		PathPosition* pcontroller;
		void TimeChanged(TimeValue t);
};

 class pathValidatorClass : public PBValidator
{
	public:
		PathPosition *mod;
	private:
		BOOL Validate(PB2Value &v);
};


class PathPosition : public	IPathPosition 
{
	public:
		int incFlag;
		int PathOpenFlag;					// TRUE if at least one path is open
											// FALSE otherwise
		pathValidatorClass validator;								
		void RedrawListbox(TimeValue t, int sel = -1);						// AG added
		HWND hWnd;
		int last_selection, oldTargetNumber;

// for viewport manipulation down
		Point3 manip_curval;
		Spline3D newSpline;
		void MouseCycleStarted(TimeValue t);
		void MouseCycleCompleted(TimeValue t);
		Matrix3 objectTM;
// for viewport manipulation up

		PathTimeChangeCallback pathTimeChangeCallback;
		IParamBlock2* pblock;
		INode *patht;

		DWORD flags;
		Point3 curval, basePointLocal, basePointWorld;
		Interval ivalid;
		Quat curRot;
		float bankAmount, tracking;
		Control* old_percent;  // holds pre-PB2 percent controller during loading
		int initialPositionFlag, reDrawFlag;
		Point3 InitialPosition;
		
		static PathPosition *editCont;
		static IObjParam *ip;

		PathPosition(const PathPosition &ctrl);
		PathPosition(BOOL loading=FALSE);

		~PathPosition();

		int GetNumTargets();									// AG added
		int GetnonNULLNumPathTargets();
		INode* GetNode(int targetNumber);						// AG added
		float GetTargetWeight(int targetNumber);				// AG added
		BOOL SetTargetWeight(int targetNumber, float weight);	// AG added
		BOOL AppendTarget(INode *target, float weight=50.0);	// AG added
		BOOL DeleteTarget(int selection);						// AG added
		BOOL SetTarget(INode *target, int targetNumber);


		void SetBankAmount(float a);
		void SetTracking(float t);
		void SetFollow(BOOL f);
		void SetBank(BOOL b);
		void SetAllowFlip(BOOL f);
		void SetConstVel(BOOL cv);
		void SetFlip(BOOL onOff);
		void SetAxis(int axis);
		void SetLoop(BOOL l);									// AG added
		void SetRelative(BOOL rel);								// AG added
//		BOOL stopReferencing;
		

		float GetBankAmount();
		float GetTracking();
		BOOL GetFollow() {return Follow();}
		BOOL GetBank() {return Bank();}
		BOOL GetAllowFlip() {return AllowFlip();}
		BOOL GetConstVel() {return ConstVel();}
		BOOL GetFlip() {return flags&PATHFLAG_FLIP?TRUE:FALSE;}
		int GetAxis();
		BOOL GetLoop() {return Loop();}							// AG added
		BOOL GetRelative() {return Relative();}					// AG added

		BOOL Follow() {return flags&PATHFLAG_FOLLOW?TRUE:FALSE;}
		BOOL Bank() {return flags&PATHFLAG_BANK?TRUE:FALSE;}
		BOOL AllowFlip() {return flags&PATHFLAG_ALLOWFLIP?TRUE:FALSE;}
		BOOL ConstVel() {return flags&PATHFLAG_CONSTVEL?TRUE:FALSE;}
		
		BOOL Loop() {return flags&PATHFLAG_LOOP?TRUE:FALSE;}			// AG added
		BOOL Relative() {return flags&PATHFLAG_RELATIVE?TRUE:FALSE;}	// AG added


		void Update(TimeValue t);
		float GetPercent(TimeValue t, BOOL noClip=FALSE);
		Point3 PointOnPath(TimeValue t, ShapeObject *pathOb, INode* path);									// AG added
		Point3 PointOnPath_manip(float u, TimeValue anim, ShapeObject *pathOb, INode* path);						// AG added

		Point3 CalculateTangentOnPath(TimeValue t, ShapeObject *pathOb, INode* path);	// AG added
		Matrix3 CalcRefFrame(Point3 PathNormal, Point3 av_tan, Point3 av_pos, 
					TimeValue t, ShapeObject *pathOb, INode* path);						// AG added
		float SplineToPoint(TimeValue t, Point3 p1, Spline3D *s, float &finalu, int &cid, 
					int &sid, Matrix3 worldToScreenTm);					// AG added (Peter Watje's code)
		void PointToPiece(TimeValue t, float &tempu, Spline3D *s, int Curve, int Piece, 
				int depth, Point3 fp, Matrix3 worldToScreenTm);			// AG added (Peter Watje's code)
		void RecurseDepth(TimeValue t, float u1, float u2, float &fu, Spline3D *s, 
					int Curve, int Piece, int &depth, Point3 fp, Matrix3 worldToScreenTm);
		void SplineFromPathPoints(TimeValue t);							// AG added (Peter Watje's code)
		Point3 SampleSplineCurves(float percentage, TimeValue anim_time, TimeValue t);	// AG added


		// Animatable methods
		Class_ID ClassID() { return Class_ID(PATH_CONTROL_CLASS_ID,0); }  
		SClass_ID SuperClassID() { return CTRL_POSITION_CLASS_ID; }  		
		
		void GetClassName(TSTR& s) {s = PATH_CONTROL_CNAME;}
		void DeleteThis() {delete this;} // intellgent destructor
		int IsKeyable() {return 0;}		// if the controller is keyable (for trackview)?

		int NumSubs()  {return 1;} //because it uses the paramblock
		Animatable* SubAnim(int i) {return pblock;}
		TSTR SubAnimName(int i) { return GetString(IDS_RB_PATHPARAMS); }
		int SubNumToRefNum(int subNum) {if (subNum==0) return PATHPOS_PBLOCK_REF; else return -1;}

		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev ); //to display controller params on the motion panel
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next ); // to undisplay

		int SetProperty(ULONG id, void *data);
		void *GetProperty(ULONG id);

// JBW: direct ParamBlock access is added
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
		int NumRefs() { return 3; };	
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

		// IK Methods
		void EnumIKParams(IKEnumCallback &callback);
		BOOL CompDeriv(TimeValue t, Matrix3& ptm, IKDeriv& derivs, DWORD flags);
		float IncIKParam(TimeValue t, int index, float delta);
		void ClearIKParam(Interval iv,int index);

//watje is a bool to determine if any of the curves used is a NURBS curve to allow for faster tv display
		BOOL isNurbs;
//watje these are some optimizations for the trackview display
//normally the path control evals really fast, but when a nurbs
//curve is used it becomes slower. The isNurbs and GetDrawPixelStep
//and GetExtentTimeStep allows the controller to work with TrackView faster
		int GetDrawPixelStep() {if (isNurbs) return 10 ;
								  else return 5;}
		int GetExtentTimeStep() {if (isNurbs) return 320;
								else return 40;}

		BaseInterface* GetInterface(Interface_ID id) 
		{ 
			if (id == PATH_CONSTRAINT_INTERFACE) 
				return (PathPosition*)this; 
			else 
				return FPMixinInterface::GetInterface(id);
		} 

	};


	BOOL pathValidatorClass::Validate(PB2Value &v)
	{
		INode *node = (INode*) v.r;

		for (int i = 0; i < mod->pblock->Count(path_path_list); i++)
		{
			if (node == mod->pblock->GetINode(path_path_list, 0, i))
				return FALSE;
		}

// self-dependency loop check  
		if (node->TestForLoop(FOREVER,(ReferenceMaker *) mod)!=REF_SUCCEED) return FALSE;

		const ObjectState& os = node->EvalWorldState(0);
		Object* ob = os.obj;
		if (ob!=NULL) 
		{	
			if (os.obj->SuperClassID() == SHAPE_CLASS_ID)
				return TRUE;
			else
				return FALSE;
		}
		return FALSE;
	}
	

void PathTimeChangeCallback::TimeChanged(TimeValue t){
	int selection = SendDlgItemMessage(pcontroller->hWnd, IDC_PATH_LIST, LB_GETCURSEL, 0, 0);
	ISpinnerControl* spin = GetISpinner(GetDlgItem(pcontroller->hWnd, IDC_PATH_WEIGHT_SPINNER));
	spin = GetISpinner(GetDlgItem(pcontroller->hWnd, IDC_PATH_WEIGHT_SPINNER));

	Control *cont = pcontroller->pblock->GetController(path_path_weight, selection); 
	// until it is animated, paramblock doesn't have a controller assigned in it yet.
	if (spin && (cont !=NULL)) {
		if (cont->IsKeyAtTime(t,KEYAT_POSITION)) {
			spin->SetKeyBrackets(TRUE);
		} 
		else {
			spin->SetKeyBrackets(FALSE);
		}
	}
	pcontroller->RedrawListbox(t);
}

class JointParamsPath : public JointParams {
	public:			 	
		JointParamsPath() : JointParams((DWORD)JNT_POS,1,100.0f) {}
		void SpinnerChange(InterpCtrlUI *ui,WORD id,ISpinnerControl *spin,BOOL interactive);
	};

IObjParam *PathPosition::ip             = NULL;
PathPosition *PathPosition::editCont    = NULL;

//********************************************************
// PATH CONTROL
//********************************************************
static Class_ID pathControlClassID(PATH_CONTROL_CLASS_ID,0); 
class PathClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new PathPosition(loading); }
	const TCHAR *	ClassName() { return PATH_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_POSITION_CLASS_ID; }
	Class_ID		ClassID() { return pathControlClassID; }
	const TCHAR* 	Category() { return _T("");  }
// JBW: new descriptor data accessors added.
	const TCHAR*	InternalName() { return _T("Path"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }		// returns owning module handle
	};

static PathClassDesc pathCD;
ClassDesc* GetPathCtrlDesc() {return &pathCD;}

//--- CustMod dlg proc ---------DOWN DOWN DOWN----from Modifiers/clstmode.cpp---------------


class PickPathNode : public PickModeCallback,
		public PickNodeCallback {
	public:			
		PathPosition *p;
		PickPathNode() {p = NULL;}
		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);		
		BOOL Pick(IObjParam *ip,ViewExp *vpt);		
		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);		
		BOOL Filter(INode *node);
		PickNodeCallback *GetFilter() {return this;}
		BOOL RightClick(IObjParam *ip,ViewExp *vpt) {return TRUE;}
	};
static PickPathNode thePickMode;


BOOL PickPathNode::Filter(INode *node){

	for (int i = 0; i < p->pblock->Count(path_path_list); i++){
		if (node == p->pblock->GetINode(path_path_list, 0, i)) 
			return FALSE;
	}

	if (node->TestForLoop(FOREVER,(ReferenceMaker *) p)!=REF_SUCCEED)
		return FALSE;


	const ObjectState& os = node->EvalWorldState(0);
	Object* ob = os.obj;
	if (ob!=NULL) 
		{	
			if (os.obj->SuperClassID() == SHAPE_CLASS_ID)
				return TRUE;
			else
				return FALSE;
		}

	return TRUE;
}

BOOL PickPathNode::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
	{	
		if (ip->PickNode(hWnd,m,this)) {
			return TRUE;
		} 
		else {
			return FALSE;
		}
	}

BOOL PickPathNode::Pick(IObjParam *ip,ViewExp *vpt)
	{
		INode *node = vpt->GetClosestHit();
		if (node) {
			p->AppendTarget(node);
			p->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE); 
			p->ip->RedrawViews(GetCOREInterface()->GetTime());
		}
		return FALSE;
	}

void PickPathNode::EnterMode(IObjParam *ip)
	{
		ICustButton *iBut = GetICustButton(GetDlgItem(p->hWnd,IDC_PATH_PICKNODE));
		if (iBut) iBut->SetCheck(TRUE);
		ReleaseICustButton(iBut);
		GetCOREInterface()->PushPrompt("Select Path Target");
	}

void PickPathNode::ExitMode(IObjParam *ip)
	{
		ICustButton *iBut = GetICustButton(GetDlgItem(p->hWnd,IDC_PATH_PICKNODE));
		if (iBut) iBut->SetCheck(FALSE);
		ReleaseICustButton(iBut);
		GetCOREInterface()->PopPrompt();
	}


//--- CustMod dlg proc -----------UP UP UP -------------------


//Function Publishing descriptor for Mixin interface

static FPInterfaceDesc path_constraint_interface(
    PATH_CONSTRAINT_INTERFACE, _T("constraints"), 0, &pathCD, 0,

//		enum_name				maxscript_name				??	return_type	??	no_of_argu

		IPathPosition::get_num_targets,		_T("getNumTargets"), 0, TYPE_INDEX, 0, 0,
		IPathPosition::get_node,			_T("getNode"),	0, TYPE_INODE, 0, 1,
			_T("targetNumber"), 0, TYPE_INDEX,
		IPathPosition::get_target_weight,	_T("getWeight"), 0, TYPE_FLOAT, 0, 1,
			_T("targetNumber"), 0, TYPE_INDEX,
		IPathPosition::set_target_weight,	_T("setWeight"), 0, TYPE_BOOL, 0, 2,
			_T("targetNumber"), 0, TYPE_INDEX,
			_T("weight"), 0, TYPE_FLOAT,
		IPathPosition::append_target,		_T("appendTarget"), 0, TYPE_BOOL, 0, 2,
			_T("target"), 0, TYPE_INODE,
			_T("weight"), 0, TYPE_FLOAT,
		IPathPosition::delete_target,		_T("deleteTarget"), 0, TYPE_BOOL, 0, 1,
			_T("targetNumber"), 0, TYPE_INDEX,

		end
		);


//  Get Descriptor method for Mixin Interface
//  *****************************************
FPInterfaceDesc* IPathPosition::GetDesc()
{
     return &path_constraint_interface;
}

//*********************************************
// End of Function Publishing Code

// enum { path_params, path_joint_params };


// parameter setter callback, reflect any ParamBlock-mediated param setting in instance data members.
// JBW: since the old path controller kept all parameters as instance data members, this setter callback
// is implemented to to reduce changes to existing code 
class PathPBAccessor : public PBAccessor
{ 
public:

	void TabChanged(tab_changes changeCode, Tab<PB2Value>* tab, 
                  ReferenceMaker* owner, ParamID id, int tabIndex, int count) 
	{ 
		if (id == path_path_weight || id == path_path_list){

			PathPosition* p = (PathPosition*)owner;
			int ct = p->pblock->Count(path_path_list);
			int ctf = p->pblock->Count(path_path_weight);
			if (changeCode == tab_ref_deleted){		  
				int j = p->pblock->Delete(path_path_list, tabIndex, 1); // deletes the node from nodelist;
				int k = p->pblock->Delete(path_path_weight, tabIndex, 1); // deletes corresponding weight;
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

				if (p->last_selection < 0  || p->GetnonNULLNumPathTargets() < 1){ 
				// nothing is selected OR there are no targets
					// all buttons - target_delete, weight_edit, weight_spinner -- are disabled
					EnableWindow(GetDlgItem(p->hWnd, IDC_PATH_WEIGHT_EDIT), FALSE);
					EnableWindow(GetDlgItem(p->hWnd, IDC_PATH_WEIGHT_SPINNER), FALSE);
					ICustButton *iPickOb1;
					iPickOb1= GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_PATH));
					if (iPickOb1 != NULL){
						iPickOb1->Enable(FALSE);		
						ReleaseICustButton(iPickOb1);
					}
				}
				else if (p->last_selection >= 0) {	
				// there is a valid selection 
					// automatically means there is AT LEAST one target
					if (p->GetnonNULLNumPathTargets() == 1){
					// the "delete" button should be enabled
						ICustButton *iPickOb1;
						iPickOb1 = GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_PATH));
						if (iPickOb1 != NULL){
							iPickOb1->Enable(TRUE);		
							ReleaseICustButton(iPickOb1);
						}
						// the rest are disabled
						EnableWindow(GetDlgItem(p->hWnd, IDC_PATH_WEIGHT_EDIT), FALSE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_PATH_WEIGHT_SPINNER), FALSE);
					}
					
					else if (p->GetnonNULLNumPathTargets() > 1){
						
						// all buttons - target_delete, weight_edit, weight_spinner -- are enabled
						EnableWindow(GetDlgItem(p->hWnd, IDC_PATH_WEIGHT_EDIT), TRUE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_PATH_WEIGHT_SPINNER), TRUE);
						ICustButton *iPickOb1;
						iPickOb1 = GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_PATH));
						if (iPickOb1 != NULL){
							iPickOb1->Enable(TRUE);		
							ReleaseICustButton(iPickOb1);
						}
					}		// else if (p->GetnonNULLNumPathTargets() > 1){
				}			//	else if (p->last_selection != LB_ERR) {			
			} // if (ct == ctf){
		}				// if (id == path_path_weight || id == path_path_list)

	}

	void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid)
	{
		PathPosition* p = (PathPosition*)owner;

		switch(id)
		{
			case path_path_weight:
				if ((v.f) < 0.0f) v.f = 0.0f; 
			break;

			case path_path:
				INode* aNode;
				int ct = p->pblock->Count(path_path_list);
				if ( ct > 0){
					p->pblock->GetValue(path_path_list, t, aNode, FOREVER, 0);
					if ( aNode != NULL){
						v.r = aNode;
					}
					else{
						v.r = NULL;
					}
				}
				else v.r = NULL;
				break;

		}
	}



	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		PathPosition* p = (PathPosition*)owner;
		switch (id)
		{
//			case path_path_list:
//			{
//				IParamMap2* pmap = p->pblock->GetMap();
//				if (pmap)
//					if (v.r == NULL)
//						SetWindowText(GetDlgItem(pmap->GetHWnd(), IDC_PATHNAME), GetString(IDS_RB_NONE)); 
//					else
//						SetWindowText(GetDlgItem(pmap->GetHWnd(), IDC_PATHNAME), ((INode*)(v.r))->GetName()); 
//					
//				break;
//
//			}

			case path_follow:
				if (v.i) p->flags |= PATHFLAG_FOLLOW; else p->flags &= ~PATHFLAG_FOLLOW; break;
			case path_bank:
				if (v.i) p->flags |= PATHFLAG_BANK; else p->flags &= ~PATHFLAG_BANK; break;
			case path_bank_amount:
				p->bankAmount = FromBankUI(v.f); break;
			case path_smoothness:
				p->tracking = FromTrackUI(v.f); break;
			case path_allow_upsidedown:
				if (v.i) p->flags |= PATHFLAG_ALLOWFLIP; else p->flags &= ~PATHFLAG_ALLOWFLIP; break;
			case path_constant_vel:
				if (v.i) p->flags |= PATHFLAG_CONSTVEL; else p->flags &= ~PATHFLAG_CONSTVEL; break;
			case path_axis:
				switch (v.i)
				{
					case 0: p->flags &= ~(PATHFLAG_YAXIS | PATHFLAG_ZAXIS); break;			// X
					case 1: p->flags |= PATHFLAG_YAXIS; p->flags &= ~PATHFLAG_ZAXIS; break;	// Y
					case 2: p->flags |= PATHFLAG_ZAXIS; p->flags &= ~PATHFLAG_YAXIS; break;	// Y
				}
				break;
			case path_axis_flip:
				if (v.i) p->flags |= PATHFLAG_FLIP; else p->flags &= ~PATHFLAG_FLIP; break;

// AG Added
			case path_loop:
				if (v.i) p->flags |= PATHFLAG_LOOP; else p->flags &= ~PATHFLAG_LOOP; break;
			case path_relative:
				if (v.i) p->flags |= PATHFLAG_RELATIVE; else p->flags &= ~PATHFLAG_RELATIVE; break;	

			case path_path:
				if (v.r != NULL){
					p->SetTarget((INode*)(v.r), 0);
//					v.r = NULL;

				}

			break;
		}
	}
};


static PathPBAccessor path_accessor;
class PathDlgProc : public ParamMap2UserDlgProc 
{
	public:
		void UpdatePathName(PathPosition* p)
		{
			IParamMap2* pmap = p->pblock->GetMap();
		}

		BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			PathPosition* p = (PathPosition*)map->GetParamBlock()->GetOwner();			
			UpdatePathName(p);
			p->hWnd = hWnd;
			int ct = p->pblock->Count(path_path_list);

			switch (msg) 
			{
				case WM_INITDIALOG:
				{
					int selection = p->last_selection;

					ICustButton *iBut = GetICustButton(GetDlgItem(hWnd,IDC_PATH_PICKNODE));
					iBut->SetType(CBT_CHECK);
					iBut->SetHighlightColor(GREEN_WASH);
					ReleaseICustButton(iBut);

					ISpinnerControl* spin = SetupFloatSpinner(hWnd, IDC_PATH_WEIGHT_SPINNER, IDC_PATH_WEIGHT_EDIT, 0.0f, 100.0f, 50.0f, 1.0f);
					spin = GetISpinner(GetDlgItem(hWnd, IDC_PATH_WEIGHT_SPINNER));

					// CAL-09/10/02: automatically set last_selection to 0 when there're targets
					if (p->GetnonNULLNumPathTargets() < 1) { 
						// nothing is selected OR there are no targets
						// all buttons - target_delete, weight_edit, weight_spinner -- are disabled
						EnableWindow(GetDlgItem(p->hWnd, IDC_PATH_WEIGHT_EDIT), FALSE);
						EnableWindow(GetDlgItem(p->hWnd, IDC_PATH_WEIGHT_SPINNER), FALSE);
						ICustButton *iPickOb1;
						iPickOb1	= GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_PATH));
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
						Control *cont = p->pblock->GetController(path_path_weight, selection); 
						// until it is animated, paramblock doesn't have a controller assigned in it yet.
						if (spin) {
							if ((cont != NULL) && cont->IsKeyAtTime(t,KEYAT_POSITION))
								spin->SetKeyBrackets(TRUE);
							else
								spin->SetKeyBrackets(FALSE);
						}

						if (p->GetnonNULLNumPathTargets() == 1){
						// the "delete" button should be enabled
							ICustButton *iPickOb1;
							iPickOb1 = GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_PATH));
							if (iPickOb1 != NULL){
								iPickOb1->Enable(TRUE);		
								ReleaseICustButton(iPickOb1);
							}
							// the rest are disabled
							EnableWindow(GetDlgItem(p->hWnd, IDC_PATH_WEIGHT_EDIT), FALSE);
							EnableWindow(GetDlgItem(p->hWnd, IDC_PATH_WEIGHT_SPINNER), FALSE);
						}
						else if (p->GetnonNULLNumPathTargets() > 1){ 
						// there are more than one targets
						// all buttons - delete, weight_edit, weight_spinner -- are enabled
							EnableWindow(GetDlgItem(p->hWnd, IDC_PATH_WEIGHT_EDIT), TRUE);
							EnableWindow(GetDlgItem(p->hWnd, IDC_PATH_WEIGHT_SPINNER), TRUE);
							ICustButton *iPickOb1;
							iPickOb1 = GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_PATH));
							if (iPickOb1 != NULL){
								iPickOb1->Enable(TRUE);		
								ReleaseICustButton(iPickOb1);
							}
						}
					}
					ReleaseISpinner(spin);
					p->RedrawListbox(GetCOREInterface()->GetTime(),selection);
					return TRUE;
						
				}
				break;

				case WM_COMMAND:
				{
					int selection;

					if (LOWORD(wParam) == IDC_PATH_LIST && HIWORD(wParam) == LBN_SELCHANGE)
					{
						selection = SendDlgItemMessage(p->hWnd, IDC_PATH_LIST, LB_GETCURSEL, 0, 0);
						
						p->last_selection = selection;
						if (selection >= 0){
						// there is a valid selection 
							// automatically means there is AT LEAST one target
							if (p->GetnonNULLNumPathTargets() == 1){ 
								// the "delete" button should be enabled
								ICustButton *iPickOb1;
								iPickOb1 = GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_PATH));
								if (iPickOb1 != NULL){
									iPickOb1->Enable(TRUE);		
									ReleaseICustButton(iPickOb1);
								}
								// the rest are disabled
								EnableWindow(GetDlgItem(p->hWnd, IDC_PATH_WEIGHT_EDIT), FALSE);
								EnableWindow(GetDlgItem(p->hWnd, IDC_PATH_WEIGHT_SPINNER), FALSE);
							}

							else  if (p->GetnonNULLNumPathTargets() > 1){ 
							// there are more than one targets
								// all buttons - delete, weight_edit, weight_spinner -- are enabled
								EnableWindow(GetDlgItem(p->hWnd, IDC_PATH_WEIGHT_EDIT), TRUE);
								EnableWindow(GetDlgItem(p->hWnd, IDC_PATH_WEIGHT_SPINNER), TRUE);
								ICustButton *iPickOb1;
								iPickOb1 = GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_PATH));
								if (iPickOb1 != NULL){
									iPickOb1->Enable(TRUE);		
									ReleaseICustButton(iPickOb1);
								}

								ISpinnerControl* spin = GetISpinner(GetDlgItem(hWnd, IDC_PATH_WEIGHT_SPINNER));
								float value = map->GetParamBlock()->GetFloat(path_path_weight, t, selection);
								spin->SetValue(value, FALSE);
								Control *cont = p->pblock->GetController(path_path_weight, selection); // until it is animated, paramblock doesn't have a controller assigned in it yet.
								if (spin && (cont != NULL)) {
									if (cont->IsKeyAtTime(t,KEYAT_POSITION)){
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
						
						else if (selection < 0 || p->GetnonNULLNumPathTargets() < 1){ 
						// nothing is selected OR there are no targets
							// all buttons - target_delete, weight_edit, weight_spinner -- are disabled
							EnableWindow(GetDlgItem(p->hWnd, IDC_PATH_WEIGHT_EDIT), FALSE);
							EnableWindow(GetDlgItem(p->hWnd, IDC_PATH_WEIGHT_SPINNER), FALSE);
							ICustButton *iPickOb1;
							iPickOb1	= GetICustButton(GetDlgItem(p->hWnd, IDC_REMOVE_PATH));
							if (iPickOb1 != NULL){
								iPickOb1->Enable(FALSE);		
								ReleaseICustButton(iPickOb1);
							}
						}
					}

					else if (LOWORD(wParam) == IDC_PATH_PICKNODE){
						thePickMode.p  = p;					
						p->ip->SetPickMode(&thePickMode);
					}

					else if (LOWORD(wParam) == IDC_REMOVE_PATH){

						selection = SendDlgItemMessage(p->hWnd, IDC_PATH_LIST, LB_GETCURSEL, 0, 0);
						p->last_selection = selection;
						if (selection >= 0){
							theHold.Begin();
							p->DeleteTarget(selection);
							theHold.Accept(GetString(IDS_AG_PATH_PATH_LIST));
						}
					}
				}
				break;


				case CC_SPINNER_CHANGE:
				{
					if (LOWORD(wParam) == IDC_PATH_WEIGHT_SPINNER) {
						int selection = SendDlgItemMessage(p->hWnd, IDC_PATH_LIST, LB_GETCURSEL, 0, 0);

						// CAL-09/10/02: need to start a hold if it's not holding to handle type-in values
						BOOL isHold = theHold.Holding();
						if (!isHold) theHold.Begin();
						
						ISpinnerControl* spin = GetISpinner(GetDlgItem(hWnd, IDC_PATH_WEIGHT_SPINNER));

						if (selection >= 0) {
							float value = ((ISpinnerControl *)lParam)->GetFVal();
							map->GetParamBlock()->SetValue(path_path_weight, t, value, selection);

							Control *cont = p->pblock->GetController(path_path_weight, selection); // until it is animated, paramblock doesn't have a controller assigned in it yet.
							if (spin)
								spin->SetKeyBrackets(((cont != NULL) && cont->IsKeyAtTime(t,KEYAT_POSITION)) ? TRUE : FALSE);

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
			UpdatePathName((PathPosition*)pb->GetOwner());
		}
		void DeleteThis() { }
};

	void PathPosition::RedrawListbox(TimeValue t, int sel)
		{
			if (hWnd == NULL) return;
			if (!ip || editCont != this) return;
			if(!pathCD.NumParamMaps()) return;
			int selection = SendDlgItemMessage(hWnd, IDC_PATH_LIST, LB_GETCURSEL, 0, 0);
			SendDlgItemMessage(hWnd, IDC_PATH_LIST, LB_RESETCONTENT, 0, 0);
			int ts = 62;
			SendDlgItemMessage(hWnd, IDC_PATH_LIST, LB_SETTABSTOPS, 1, (LPARAM)&ts);
			int ct = pblock->Count(path_path_list);
			int ctf = pblock->Count(path_path_weight);
			if (ct != ctf) return;		// CAL-09/10/02: In the middle of changing table size.

			for (int i = 0; i < ct; i++){
				if (pblock->GetINode(path_path_list, t, i) != NULL){					
					TSTR str;
					str.printf(_T("%-s\t%-d"), 
						pblock->GetINode(path_path_list, t, i)->GetName(),
						(int)pblock->GetFloat(path_path_weight, t, i));
					SendDlgItemMessage(hWnd, IDC_PATH_LIST, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR) str.data());			
				}
			}

			if (ct > 0){
			
				if (sel >= 0){
					SendDlgItemMessage(hWnd, IDC_PATH_LIST, LB_SETCURSEL, sel, 0);
				}
				else if (selection >= 0){
					SendDlgItemMessage(hWnd, IDC_PATH_LIST, LB_SETCURSEL, selection, 0);
					last_selection = selection;
				}
				else if (ct == 1){
					SendDlgItemMessage(hWnd, IDC_PATH_LIST, LB_SETCURSEL, 0, 0);
					last_selection = 0;
				}

				ISpinnerControl* spin = GetISpinner(GetDlgItem(hWnd, IDC_PATH_WEIGHT_SPINNER));
				if (last_selection >=0 && spin != NULL){
					float value = pblock->GetFloat(path_path_weight, GetCOREInterface()->GetTime(), last_selection);
					spin->SetValue(value, FALSE);
				}
			}
			HWND hListBox = GetDlgItem(hWnd, IDC_PATH_LIST);
			int extent = computeHorizontalExtent(hListBox, TRUE, 1, &ts);
			SendMessage(hListBox, LB_SETHORIZONTALEXTENT, extent, 0);
			UpdateWindow(hWnd);
		}


static PathDlgProc pathDlgProc;

// per instance path controller block
static ParamBlockDesc2 path_paramblk (path_params, _T("PathParameters"),  0, &pathCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PATHPOS_PBLOCK_REF, 
	//rollout
	IDD_PATHPARAMS, IDS_RB_PATHPARAMS, BEGIN_EDIT_MOTION, 0, &pathDlgProc,
	// params
	path_percent,		_T("percent"),		TYPE_PCNT_FRAC,		P_ANIMATABLE|P_RESET_DEFAULT, 	IDS_RB_PERCENT, 
		p_default, 		0.0,	
		p_range, 		float(-999999999), float(999999999), 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_PATHPERCENT, IDC_PATHPERCENTSPIN, 1.0f, 
		p_accessor,		&path_accessor,
		end, 

	path_follow,		_T("follow"),		TYPE_BOOL, 			P_RESET_DEFAULT,		IDS_JW_FOLLOW,
		p_default, 		FALSE, 
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_PATH_FOLLOW, 
		p_enable_ctrls,	4, path_bank, path_allow_upsidedown, path_axis, path_axis_flip,
		p_accessor,		&path_accessor,
		end, 

	path_path, 			_T("path"), 		TYPE_INODE, 		0,						IDS_RB_PATH,
		p_accessor,		&path_accessor,
		end, 

	path_bank,			_T("bank"),			TYPE_BOOL, 			P_RESET_DEFAULT,		IDS_JW_BANK,
		p_default, 		FALSE, 
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_PATH_BANK, 
		p_enabled,		FALSE,
		p_enable_ctrls,	2, path_bank_amount, path_smoothness,
		p_accessor,		&path_accessor,
		end, 

	path_bank_amount,	_T("bankAmount"),	TYPE_FLOAT,			P_ANIMATABLE|P_RESET_DEFAULT, 	IDS_JW_BANKAMOUNT, 
		p_default, 		0.5, 
		p_range, 		-999999.0, 999999.0, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOLLOW_BANK, IDC_FOLLOW_BANKSPIN, 0.01f,
		p_accessor,		&path_accessor,
		p_enabled,		FALSE,
		end, 

	path_smoothness,	_T("smoothness"),	TYPE_FLOAT,			P_ANIMATABLE|P_RESET_DEFAULT, 	IDS_JW_SMOOTHNESS, 
		p_default, 		0.5f, 
		p_range, 		0.0f, 10.0f, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FOLLOW_TRACK, IDC_FOLLOW_TRACKSPIN, 0.01f,
		p_enabled,		FALSE,
		p_accessor,		&path_accessor,
		end, 

	path_allow_upsidedown, _T("allowUpsideDown"), TYPE_BOOL,	P_RESET_DEFAULT,		IDS_JW_ALLOWUPSIDEDOWN,
		p_default, 		FALSE, 
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_PATH_ALLOWFLIP, 
		p_enabled,		FALSE,
		p_accessor,		&path_accessor,
		end, 

	path_constant_vel, _T("constantVel"),	TYPE_BOOL,			P_RESET_DEFAULT,		IDS_JW_CONSTANTVEL,
		p_default, 		TRUE,
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_PATH_CONSTVEL, 
		p_accessor,		&path_accessor,
		end, 

	path_axis, 			_T("axis"),			TYPE_INT, 			P_RESET_DEFAULT,		IDS_JW_AXIS,
		p_default, 		0, 
		p_range, 		0, 2, 
		p_ui, 			TYPE_RADIO, 	3, IDC_PATH_X, IDC_PATH_Y, IDC_PATH_Z, 
		p_enabled,		FALSE,
		p_accessor,		&path_accessor,
		end, 

	path_axis_flip,		_T("axisFlip"),		TYPE_BOOL,			P_RESET_DEFAULT,		IDS_JW_AXISFLIP,
		p_default, 		FALSE, 
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_PATH_FLIP, 
		p_enabled,		FALSE,
		p_accessor,		&path_accessor,
		end, 

	path_path_weight, 	_T("weight"), 	TYPE_FLOAT_TAB, 0,			P_ANIMATABLE + P_VARIABLE_SIZE + P_TV_SHOW_ALL, 	IDS_AG_PATH_WEIGHT_LIST, 
		p_default, 		50.0f, 
		p_range, 		0.0f, 100.0f, 
		p_accessor,		&path_accessor,	
		end, 

	path_path_list,		_T(""),		TYPE_INODE_TAB,	0,			P_VARIABLE_SIZE,		IDS_AG_PATH_PATH_LIST,
		p_accessor,		&path_accessor,
		end,

	path_loop,			_T("loop"),			TYPE_BOOL,			P_RESET_DEFAULT,		IDS_AG_LOOP,
		p_default, 		TRUE, 
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_PATH_LOOP, 
		p_accessor,		&path_accessor,
		end, 

	path_relative,		_T("relative"),		TYPE_BOOL,			P_RESET_DEFAULT,		IDS_AG_RELATIVE,
		p_default, 		FALSE, 
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_PATH_RELATIVE, 
		p_accessor,		&path_accessor,
		end,

	end
	);



PathPosition::PathPosition(BOOL loading) 
	{
//watje
	isNurbs = FALSE;

	Point3 zero(0.0f, 0.0f, 0.0f);
	patht = NULL;
	old_percent = NULL;
	curval   = Point3(0,0,0);
	oldTargetNumber = 0;
	curRot.Identity();
	flags      = 0;
	bankAmount = FromBankUI(0.5f);
	tracking   = FromTrackUI(0.5f);
	incFlag = 0; // to switch over control from path constraint viewport manip to IK viewport manip
	basePointWorld = zero;
	basePointLocal = zero;
	last_selection =  -1;
	reDrawFlag = 0;
	InitialPosition = zero;
	hWnd = NULL;
	initialPositionFlag = 0;
	manip_curval = zero;

	// make the paramblock
	pathCD.MakeAutoParamBlocks(this);
	// pre-animate percent param
	
	// CAL-08/05/02: Use linear float controller
	Control *ctrl = (Control *) CreateInstance(CTRL_FLOAT_CLASS_ID, Class_ID(LININTERP_FLOAT_CLASS_ID, 0));
	pblock->SetController(path_percent, 0, ctrl);
	
	SuspendAnimate();
	AnimateOn();
	float val =  1.0f;
	pblock->SetValue(path_percent, GetAnimEnd(), val);
	ResumeAnimate();
	
	pblock->CallSets();
	ivalid.SetEmpty(); 
	validator.mod = this;   // to access data inside the current instantiation of the pathposition 
}

PathPosition::~PathPosition()
	{
		DeleteAllRefsFromMe();
	}

RefTargetHandle PathPosition::Clone(RemapDir& remap)
	{
		PathPosition *p = new PathPosition(TRUE);
	
		p->ReplaceReference(PATHPOS_PBLOCK_REF, pblock->Clone(remap));
		p->flags      = flags;
		p->bankAmount = bankAmount;
		p->tracking   = tracking;
		p->curval     = curval;
		p->curRot     = curRot;
		p->basePointLocal  = basePointLocal;
		p->basePointWorld  = basePointWorld;
		p->InitialPosition  = InitialPosition;
		p->oldTargetNumber = oldTargetNumber;
		p->ivalid.SetEmpty();
		BaseClone(this, p, remap);
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		return p;
	}


void PathPosition::Copy(Control *from)
	{
	Point3 fval;
	if (from->ClassID()==ClassID()) {
		PathPosition *ctrl = (PathPosition*)from;
		// a copy will construct its own pblock to keep the pblock-to-owner 1-to-1.
		RemapDir *remap = NewRemapDir(); 
		ReplaceReference(PATHPOS_PBLOCK_REF, ctrl->pblock->Clone(*remap));
		remap->DeleteThis();
		curval   = ctrl->curval;
		curRot   = ctrl->curRot;
		flags    = ctrl->flags;
		bankAmount = ctrl->bankAmount;
		tracking = ctrl->tracking;
	} 
	else {
		from->GetValue(GetCOREInterface()->GetTime(), &fval, FOREVER);	// to know the object position before the
		initialPositionFlag = 1;
		basePointLocal = fval;							// current controller was active
	}
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}


float PathPosition::GetPercent(TimeValue t, BOOL noClip)
	{	
	float per;
//	DebugPrint("in GetPercent1: start_valid = %d  end_valid = %d\n", ivalid.Start(), ivalid.End());
	pblock->GetValue(path_percent, t, per, ivalid);
//	DebugPrint("in GetPercent3: start_valid = %d  end_valid = %d\n", ivalid.Start(), ivalid.End());
	if (noClip) return per;
	if (Loop()){
// if per > 1, fold it to within 0 and 1.
// if per = 1.0, 2.0, 3.0 etc, per = 1.0
		if (per > 1.0f) {
			per = per - floor(per);
			if (!per) per = 1.0;
		}

// if per < 0, fold it to within 0 and 1.
// if per = -1.0, -2.0, -3.0 etc, per = 0.0
		if (per < 0.0f) {
			per = per - floor(per);
			if (!per) per = 0.0;
		}
	}
	else{
		if (per < 0.0f) per = 0.0f;
		if (per > 1.0f) per = 1.0f;
	}
	return per;
	}

Point3 PathPosition::PointOnPath(TimeValue t, ShapeObject *pathOb, INode* path) 
	{
	path->GetObjectTM(t,&ivalid);	
	return pathOb->InterpCurve3D(t, 0, GetPercent(t), InterpType()) * 
		path->GetObjTMAfterWSM(t);

	}


Point3 PathPosition::PointOnPath_manip(float u, TimeValue anim, ShapeObject *pathOb, INode* path)
	{
	Interval iv = FOREVER;
	return pathOb->InterpCurve3D(anim, 0, u, InterpType()) * path->GetObjTMAfterWSM(anim,&iv);

	}


// Reference frame is calculated as the following:
//
// X axis = tangent(t)
// Y axis = WorldZ cross X
// Z axis = X cross Y
//
// or if the 'Allow Upside Down' option us selected:
//
// X axis = tangent(t)
// Z axis = path normal cross X
// Y axis = Z cross X
//
// If roll is on then the reference frame is rotated about
// the tangent by rollamount * curviture.

#define DU			0.001f
#define BDU			0.01f
#define NUMSTEPS	5



Point3 PathPosition::CalculateTangentOnPath(TimeValue t, ShapeObject *pathOb, INode* path)
	{
		Point3 tangent(0.0f,0.0f,0.0f);
		Matrix3 otm = path->GetObjTMAfterWSM(t);
		float u = GetPercent(t);
//		DebugPrint("in CalculateTangentOnPath: start_valid = %d  end_valid = %d\n", ivalid.Start(), ivalid.End());
		
		Point3 pt0, pt1;
		pt0 = pathOb->InterpCurve3D(t, 0, u-DU, InterpType()) * otm;
		pt1 = pathOb->InterpCurve3D(t, 0, u+DU, InterpType()) * otm;

		tangent = pt1 - pt0;
		int ct = pblock->Count(path_path_list);
		return tangent;
}


Matrix3 PathPosition::CalcRefFrame(Point3 PathNormal, Point3 av_tan, Point3 av_pos, TimeValue t, ShapeObject *pathOb, INode* path)
	{
		Matrix3 otm = path->GetObjTMAfterWSM(t);
		Matrix3 tm(1);
		float u = GetPercent(t);
//		DebugPrint("4: start_valid = %d  end_valid = %d\n", ivalid.Start(), ivalid.End());

		bankAmount = FromBankUI(pblock->GetFloat(path_bank_amount, t));
		tracking = FromTrackUI(pblock->GetFloat(path_smoothness, t));

	// X
	tm.SetRow(0, Normalize(av_tan));

//AG CUT 	tm.SetRow(0,Normalize(pt1-pt0));

	if (GetFlip()) tm.SetRow(0,-tm.GetRow(0));

	if (AllowFlip()) {
		// Choose Z in the plane of the path		
		// Z
		tm.SetRow(2,Normalize(tm.GetRow(0)^PathNormal));

		// Y
		tm.SetRow(1,tm.GetRow(2)^tm.GetRow(0));} 
	else {
		// Choose Y in the world XY plane
		// Y
		tm.SetRow(1,Normalize(Point3(0,0,1)^tm.GetRow(0)));

		// Z
		tm.SetRow(2,tm.GetRow(0)^tm.GetRow(1));
		}


	// swap axis around	
	Point3 tmp;
	switch (GetAxis()) {
		case 1: 
			tmp = tm.GetRow(0);
			tm.SetRow(0,-tm.GetRow(1));
			tm.SetRow(1,tmp);
			break;
		case 2: 
			tmp = tm.GetRow(0);
			tm.SetRow(0,-tm.GetRow(2));
			tm.SetRow(2,tmp);  
			break;
		}		
	
	if (Bank()) {
	// Average over NUMSTEPS samples
		Point3 pt0, pt1, pt2, v0, v1;
		float cv = 0.0f;
		u -= float(NUMSTEPS/2+1)*tracking;
				
		if (!pathOb->CurveClosed(t,0)) {
			if (u+(NUMSTEPS+2)*tracking > 1.0f) 
				u = 1.0f - (NUMSTEPS+2)*tracking;
			if (u<0.0f) 
				u=0.0f;
		}

		pt1 = pathOb->InterpCurve3D(t, 0, u, InterpType()) * otm;
		u += tracking;		
		pt2 = pathOb->InterpCurve3D(t, 0, u, InterpType()) * otm;
		u += tracking;		
		for (int i = 0; i < NUMSTEPS; i++) {			
			pt0 = pt1;
			pt1 = pt2;
			if (!pathOb->CurveClosed(t,0) && u>1.0f) {
				//pt2 += pt2 - pt1;
				break;
			} 
			else {
				u   = (float)fmod(u,1.0f);
				pt2 = pathOb->InterpCurve3D(t, 0, u, InterpType()) * otm;
			}
			v0 = Normalize(pt2-pt1);
			v1 = Normalize(pt1-pt0);			
			v0.z = v1.z = 0.0f; // remove Z component.
			cv += (v0^v1).z * bankAmount / Length(pt1-pt0);
			u  += tracking;			
		}	
		if (i) {
			if (GetFlip()) cv = -cv;
			switch (GetAxis()) {
				case 0: tm.PreRotateX(cv/float(i)); break;
				case 1: tm.PreRotateY(cv/float(i)); break;
				case 2: tm.PreRotateZ(cv/float(i)); break;
			}
		}
	}
	return tm;
}


void PathPosition::Update(TimeValue t){

	ShapeObject *pathOb = NULL;
	ivalid = FOREVER;
	Point3 pathNorm(0.0f, 0.0f, 0.0f), curval_initial(0.0f, 0.0f, 0.0f);
	Matrix3 otm;
	
	int ct = pblock->Count(path_path_list);
	int ctf = pblock->Count(path_path_weight);
	if (ctf != ct){
		pblock->SetCount(path_path_weight, ct);
		RedrawListbox(t);
	}

	curval = Point3(0.0f, 0.0f, 0.0f);
	Point3 average_tangent(0.0f, 0.0f, 0.0f);
	float total_path_weight = 0.0f;
//watje
	isNurbs = FALSE;
	for (int i = 0; i < ct; i++) {

		INode *path;
		float pathWt = 0.0;
		pblock->GetValue(path_path_list, t, path, FOREVER, i);

		if (path == NULL) continue; // skip the for loop if the selected object is null  

		pblock->GetValue(path_path_weight, t, pathWt, ivalid, i);
//		DebugPrint("1: start_valid = %d  end_valid = %d\n", ivalid.Start(), ivalid.End());
		total_path_weight += pathWt;


		ObjectState os = path->EvalWorldState(t);
		if (os.obj->IsShapeObject()) {
			pathOb = (ShapeObject*)os.obj;
			if (!pathOb->NumberOfCurves()) {
				pathOb = NULL;
			}
//checks against the NURBS class id
			if (os.obj->ClassID() == Class_ID(0x76a11646, 0x12a822fb))
				isNurbs = TRUE;

		}
		Interval objstvalid = os.Validity(t);
//		DebugPrint("66: start_valid = %d  end_valid = %d\n", objstvalid.Start(), objstvalid.End());

		ivalid &= os.Validity(t);

//		DebugPrint("2: start_valid = %d  end_valid = %d\n", ivalid.Start(), ivalid.End());
		if (!pathOb) continue;
		curval += pathWt * PointOnPath(t, pathOb, path);

// Compute the normal to the plane of path#1 by sampling points on the path
		if(i == 0 && AllowFlip() ){
			otm = path->GetObjTMAfterWSM(t); //validity interval needs to be included
			#define NUM_SAMPLES 20
			Point3 v[NUM_SAMPLES], center(0.0f, 0.0f, 0.0f);
			for (int j=0; j<NUM_SAMPLES; j++) {
				v[j] = pathOb->InterpCurve3D(t, 0, float(j)/float(NUM_SAMPLES), InterpType()) * otm;
				center += v[j];
			}		
			center /= float(NUM_SAMPLES);
			for (j=1; j<NUM_SAMPLES; j++) {
				pathNorm += Normalize((v[j]-center)^(v[j-1]-center));
			}
			pathNorm = Normalize(pathNorm);
		}

		average_tangent += pathWt * CalculateTangentOnPath(t, pathOb, path);


		if (pathOb->CurveClosed(t,0)) {
			flags |= PATHFLAG_CLOSED;
		} 
		else{
			flags &= ~PATHFLAG_CLOSED;
		}

		if (Follow()) {
			curRot = Quat(CalcRefFrame(pathNorm, average_tangent, curval, t, pathOb, path));
		}
		else {
			curRot.Identity();
		}
	} //for (int i = 0; i < ct; i++)

	
	if (total_path_weight > 0.0){
		curval =  curval/total_path_weight;
		average_tangent = Normalize(average_tangent/total_path_weight);
		if (oldTargetNumber != ct) {
			InitialPosition = curval;
		}	
		if (Relative()){
			curval = curval + (basePointWorld - InitialPosition);
		}
	}
	else {
		curval = basePointWorld;
//		curRot.Identity();
	}

	oldTargetNumber = ct;
//	DebugPrint("3: start_valid = %d  end_valid = %d\n", ivalid.Start(), ivalid.End());
	if (ivalid.Empty()) ivalid.SetInstant(t);

}



void PathPosition::GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method){
	
	if (initialPositionFlag) {
		Matrix3 tempMat(1);
		// Point3 tempPt = Point3(0.0f, 0.0f, 0.0f);
		
		if (method==CTRL_RELATIVE)
			tempMat = *(Matrix3*)val;
		// CAL-9/26/2002: Actually it's not necessary to do this. Use identity matrix for absolute mode.
		/*
		else {
			// CAL-9/26/2002: the val passed in when in CTRL_ABSOLUTE mode could be un-initialized.
			//		It's dangerous to use the input val in CTRL_ABSOLUTE mode.
			// tempPt = *(Point3*)val;
			tempMat.SetTrans(tempPt);
		}
		*/

		basePointWorld =  basePointLocal * tempMat;
		initialPositionFlag = 0;
	}
	if (!ivalid.InInterval(t)) {
		Update(t); 
		}
	valid &= ivalid;

			 
	if (method==CTRL_RELATIVE) {
  		Matrix3 *mat = (Matrix3*)val;
		if (Follow()) {
			curRot.MakeMatrix(*mat);
		}
		mat->SetTrans(curval);		
	} 
	else {
		*((Point3*)val) = curval;
	}
//	if (!reDrawFlag) RedrawListbox(GetCOREInterface()->GetTime());
	
}


Point3 PathPosition::SampleSplineCurves(float percentage, TimeValue anim_time, TimeValue t)
	{
	ShapeObject *pathOb = NULL;
	Matrix3 otm;
	int ct = pblock->Count(path_path_list);

	float total_path_weight = 0.0f;

	for (int i = 0; i < ct; i++) {
		INode *path;
		float pathWt = 0.0;
		pblock->GetValue(path_path_list, t, path, FOREVER, i);
		if (path == NULL) continue; // skip the for loop if the selected object is null  

		pblock->GetValue(path_path_weight, anim_time, pathWt, FOREVER, i);
		total_path_weight += pathWt;

		ObjectState os = path->EvalWorldState(t);
		if (os.obj->IsShapeObject()) {
			pathOb = (ShapeObject*)os.obj;
			if (!pathOb->NumberOfCurves()) {
				pathOb = NULL;
			}
			else{
				manip_curval += pathWt*	PointOnPath_manip(percentage, t, pathOb, path);
			}
		}

	}
	if (pathOb){
		manip_curval =  manip_curval/total_path_weight;


		if (Relative()){
			manip_curval = manip_curval + (basePointWorld - InitialPosition);
		}
	}
	else {
		manip_curval= basePointWorld;
	}
	
		return manip_curval;

}
/*
void PathPosition::SplineFromPathPoints(TimeValue t)
	{
	newSpline.NewSpline();  // This method clears out the spline.  It frees the knots attributes array and the bezier points array.
	newSpline.SetOpen();

	int samples = 160;

	//this samples a spline which we can do mouse interaction tests agains
	for (int j = 0; j < samples; j++) 
		{
		Point3 aPoint(0.0f, 0.0f, 0.0f);
		aPoint = SampleSplineCurves((float)j/(float)(samples-1), t, t);
		SplineKnot kn(KTYPE_CORNER, LTYPE_LINE, aPoint, aPoint, aPoint);
		newSpline.AddKnot(kn);
		}

	newSpline.ComputeBezPoints();
	for (int poly = 0; poly < newSpline.KnotCount(); poly++)
		newSpline.SetKnotType(poly, KTYPE_CORNER);
	newSpline.ComputeBezPoints();
	}
*/
void PathPosition::SplineFromPathPoints(TimeValue t)
  {
	Interval AnimRange = GetCOREInterface()->GetAnimRange(); // time values stored here are in ticks
															// here I obtain the total animation time in ticks
// keep frame rate constant = 160

	int no_of_frames = (AnimRange.End() - AnimRange.Start())/GetTicksPerFrame(); // GetTicksPerFrame is a globally declared function

	newSpline.NewSpline();		// This method clears out the spline.  It frees the knots attributes array and the bezier points array.
	newSpline.SetOpen();

	float totalDist = 0.0f;
	Point3 prevPoint(0.0f,0.0f,0.0f);
	int numberOfCurves = pblock->Count(path_path_list);

	int no_of_Samples = 100;

	float inc = no_of_frames/100 * GetTicksPerFrame();

	for (int j = 0; j < no_of_Samples; j++) 
		{
		Point3 aPoint(0.0f, 0.0f, 0.0f);
		if (numberOfCurves == 1)
			aPoint = SampleSplineCurves((float)j/(float)(no_of_Samples), t, t);
		else 
			{
			TimeValue aTime = AnimRange.Start();
			aTime += j * inc;
			aPoint = SampleSplineCurves((float)j/(float)(no_of_Samples), aTime, t);
			}
		SplineKnot kn(KTYPE_CORNER, LTYPE_LINE, aPoint, aPoint, aPoint);
		newSpline.AddKnot(kn);
		if (j !=0)
			totalDist += Length(aPoint-prevPoint);
		prevPoint = aPoint;
		}

	newSpline.ComputeBezPoints();
	for (int poly = 0; poly < newSpline.KnotCount(); poly++)
		newSpline.SetKnotType(poly, KTYPE_CORNER);
	newSpline.ComputeBezPoints();
}


// For viewport manipulation -- DOWN

class sMyEnumProc : public DependentEnumProc 
	{
      public :
      virtual int proc(ReferenceMaker *rmaker); 
      INodeTab Nodes;              
	};

int sMyEnumProc::proc(ReferenceMaker *rmaker) 
	{ 
	if (rmaker->SuperClassID()==BASENODE_CLASS_ID){
		Nodes.Append(1, (INode **)&rmaker);                 
	}
     return 0;              
	}

void PathPosition::MouseCycleStarted(TimeValue t)
{

	sMyEnumProc dep;              
	EnumDependents(&dep);
	Interval valid = FOREVER;
	objectTM = dep.Nodes[0]->GetObjTMAfterWSM(t,&valid);
	SplineFromPathPoints(t);
	reDrawFlag = 1;

}

void PathPosition::MouseCycleCompleted(TimeValue t)
{

	incFlag = 0;
	reDrawFlag = 0;


}


#define BIGFLOAT (float(1.0e30))

// Viewport manip code from Peter's MaxSDK/Samples/Modifier/BonesDef/DistanceStuff.cpp - DOWN

void PathPosition::RecurseDepth(TimeValue t, float u1, float u2, float &fu,  Spline3D *s,int Curve,int Piece, int &depth, 
					Point3 fp, Matrix3 worldToScreenTm)
{
for (int i = 0; i < depth; i++)
	{
	float u = (u1+u2)*.5f;
	float midu = (u2-u1)*.25f;
	float tu1 = u - midu; 
	float tu2 = u + midu;
	Point3 p1, p2;
	p1 = s->InterpBezier3D(Piece, tu1);
	p2 = s->InterpBezier3D(Piece, tu2);

	p1 = p1 * worldToScreenTm;
	p1.z = 0.0f;

	p2 = p2 * worldToScreenTm;
	p2.z = 0.0f;

	if ( LengthSquared(fp-p1) < LengthSquared(fp-p2) )
		{
		u1 = u1;
		u2 = u;
		}
	else
		{
		u1 = u;
		u2 = u2;
		}

	}
fu = (u2+u1)*0.5f;
}


void PathPosition::PointToPiece(TimeValue t, float &tempu, Spline3D *s, int Curve, int Piece, int depth, Point3 fp,
					Matrix3 worldToScreenTm)

{
//float tu1,tu2,tu3,tu4;
float tu;
float su,eu;
int depth1;

depth1 = depth;

su = 0.0f;
eu = 0.25f;

float fdist = BIGFLOAT;
float fu = 0.0f;

for (int i = 0; i < 4; i++)
	{
	tu = 0.0f;
	depth = depth1;
	RecurseDepth(t, su, eu, tu, s, Curve, Piece, depth, fp, worldToScreenTm);
	su += 0.25f;
	eu += 0.25f;
	Point3 dp = s->InterpBezier3D(Piece, tu);

	dp = dp * worldToScreenTm;
	dp.z = 0.0f;

	float dist = LengthSquared(fp-dp);
	if (dist<fdist)
		{
		fdist = dist;
		fu = tu;
		}
	}


tempu = fu;
//return fu;
}


float PathPosition::SplineToPoint(TimeValue t, Point3 p1, Spline3D *s, float &finalu, int &cid, 
								  int &sid, Matrix3 worldToScreenTm)
{
//brute force for now
p1 = p1 * worldToScreenTm;

int rec_depth = 5;

int piece_count = 0;
float fdist = BIGFLOAT;
int i = 0;
//for (int i = 0; i < s->NumberOfCurves(); i++)
	{
	for (int j = 0; j < s->Segments(); j++)
		{
		float u;
		PointToPiece(t, u, s, i, j, rec_depth, p1, worldToScreenTm);
		Point3 dp = s->InterpBezier3D( j, u);

		dp = dp * worldToScreenTm;
		dp.z = 0.0f;

		float dist = LengthSquared(p1-dp);
		if (dist<fdist)
			{
			fdist = dist;
			finalu = u;
			cid = i;
			sid = j;
			}
		}
	}
return finalu;
//return (float)sqrt(fdist);
}

// Viewport manipulation code from Peter's MaxSDK/Samples/Modifier/BonesDef/DistanceStuff.cpp - UP


void PathPosition::SetValue(TimeValue t, void *val, int commit, GetSetMethod method)
{
	if(incFlag)	return;
	int ct = pblock->Count(path_path_list);
	if (ct<1) return;
	if (method==CTRL_RELATIVE) {

		Point3 *inVal = ((Point3 *) val);
		Point3 inputVec = *inVal;

		INode *path;
		ct = pblock->Count(path_path_list);
		pblock->GetValue(path_path_list, t, path, FOREVER, 0);
		
		if (path){		// to make sure that a path has been selected
						// do something more efficient as looking at the path_path_list
				Point3 initialPosition(0.0f,0.0f,0.0f);
				initialPosition = initialPosition * objectTM;
				Point3 worldPosition = initialPosition + inputVec;
				float u, dist;
				int cid, sid;
//need to put everything in screen space then ignore the z component
				ViewExp *vExp = GetCOREInterface()->GetActiveViewport();
				Matrix3 worldToScreenTm;
				vExp->GetAffineTM(worldToScreenTm);
				GetCOREInterface()->ReleaseViewport(vExp);
				dist = SplineToPoint(t, worldPosition, &newSpline, u, cid, sid, 
								  worldToScreenTm);
	
				if (!newSpline.KnotCount()) {
					initialPosition =Point3(0,0,0);
					sMyEnumProc dep;              
					EnumDependents(&dep);
					Interval valid = FOREVER;
					if (dep.Nodes.Count() && dep.Nodes[0])
					{
						objectTM = dep.Nodes[0]->GetObjTMAfterWSM(t,&valid);
						initialPosition = initialPosition * objectTM;
						worldPosition = initialPosition + inputVec;
						SplineFromPathPoints(t);
						dist = SplineToPoint(t, worldPosition, &newSpline, u, cid, sid, 
								  worldToScreenTm);
					}
				}
				int pieceCount = newSpline.Segments();
				if (sid != pieceCount)
					u = (float)(sid)/(float)pieceCount + u/(float)pieceCount;

				if (Loop()){
					u = (float)fmod(u, 1.0f);
					if (u < 0.0f) u = 1.0f + u;
					pblock->SetValue(path_percent, t, u);
				}
				else{
					if (u > 1.0) u = 1.0;
					if (u < 0.0) u = 0.0;
					pblock->SetValue(path_percent, t, u);		
				} 
				NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		}	
	} 
	else //CTRL_ABSOLUTE
	{
  		Point3 *p = (Point3*)val;
	}
}

  

// for viewport manipulation  -- UP



RefTargetHandle PathPosition::GetReference(int i)
	{
		switch (i)
		{
			case 0:
				return old_percent;		// ref 0 is now obsolete, percent stored in ParamBlock.
			case PATHPOS_PATH_REF:		// patht is inside PATHPOS_PBLOCK_REF
				return patht;
			case PATHPOS_PBLOCK_REF:	// path_path = patht is inside PATHPOS_PBLOCK_REF also - to FIX
				return pblock;
		}
		return NULL;
	}

void PathPosition::SetReference(int i, RefTargetHandle rtarg)
	{
		switch (i)
		{
			case 0:
				old_percent = (Control*)rtarg;  // reference set by pre-PB2 versions loading, fix up in PLCB
				break;
			case PATHPOS_PATH_REF:
				patht = (INode*)rtarg; break;
			case PATHPOS_PBLOCK_REF:
				pblock = (IParamBlock2*)rtarg; break;
		}
	}

RefResult PathPosition::NotifyRefChanged(
		Interval iv,  
		RefTargetHandle hTarg, 
		PartID& partID, 
		RefMessage msg) 
	{
//	if (stopReferencing) return REF_STOP;
	switch (msg) {
		case REFMSG_CHANGE:

// if i'm in edit mode ie if ip != null, then check 
// if nodelistcount != no of strings in the listbox, then refill the listbox

			if (hTarg == pblock) {
				ParamID changing_param = pblock->LastNotifyParamID();
				switch (changing_param)
				{
					// CAL-09/10/02: need to redraw the list box specifically.
					// TODO: Most of the other calls to RedrawListbox() should be removed with this addition.
					case path_path_weight:
						RedrawListbox(GetCOREInterface()->GetTime());
						break;
					default:
						path_paramblk.InvalidateUI(changing_param);
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
		PathPosition *cont;
		PickPathRestore(PathPosition *c) {cont=c;}
		void Restore(int isUndo) {
			if (cont->editCont == cont) {
				path_paramblk.InvalidateUI();
				}									
			}
		void Redo() {
		}
	};


int PathPosition::GetNumTargets(){
	return pblock->Count(path_path_list);
}

int PathPosition::GetnonNULLNumPathTargets(){
	int no_of_nonNULL_nodes = 0;
	for (int i = 0; i < pblock->Count(path_path_list); ++i){
		if (pblock->GetINode(path_path_list, GetCOREInterface()->GetTime(), i) != NULL){
			no_of_nonNULL_nodes = no_of_nonNULL_nodes + 1;
		}			
	}
	return no_of_nonNULL_nodes;
}

INode* PathPosition::GetNode(int nodeNumber){
	if (nodeNumber >= 0 && nodeNumber < pblock->Count(path_path_list)){
		INode *path;
		pblock->GetValue(path_path_list, 0, path, FOREVER, nodeNumber);
		return path;
	}
	return NULL;
}

BOOL PathPosition::AppendTarget(INode *target, float weight){
	if (target->TestForLoop(FOREVER,this)==REF_SUCCEED) {
	
		for (int i = 0; i < pblock->Count(path_path_list); ++i){
			if (target == pblock->GetINode(path_path_list, GetCOREInterface()->GetTime(), i)){
			return FALSE;	
			}
		}

		theHold.Begin();
		pblock->Append(path_path_list, 1, &target, 1);
		pblock->Append(path_path_weight, 1, &weight, 1);
		theHold.Accept(GetString(IDS_AG_PATH_PATH_LIST));
		return TRUE;
	}
	return FALSE;
}

BOOL PathPosition::DeleteTarget(int selection){
 	if (selection >= 0 && selection < pblock->Count(path_path_list)){
//		theHold.Begin();
		pblock->Delete(path_path_weight, selection, 1);
		pblock->Delete(path_path_list, selection, 1);
//		theHold.Accept(GetString(IDS_AG_PATH_PATH_LIST));
		return TRUE;
	}
	else
		return FALSE;
}


BOOL PathPosition::SetTarget(INode *target, int targetNumber){

	if (target->TestForLoop(FOREVER,this)==REF_SUCCEED) {
		int ct = pblock->Count(path_path_list);
//		int ctf = pblock->Count(path_path_weight);
		if (targetNumber >= 0 && targetNumber <= ct) {
			float weight = 50.0;
			theHold.Begin();
			pblock->SetCount(path_path_list, targetNumber+1);
			pblock->SetValue(path_path_list, GetCOREInterface()->GetTime(), target, targetNumber);
			pblock->SetCount(path_path_weight, targetNumber+1);
			pblock->SetValue(path_path_weight, GetCOREInterface()->GetTime(), weight, targetNumber);
			theHold.Accept(GetString(IDS_AG_PATH_PATH_LIST));
			RedrawListbox(GetCOREInterface()->GetTime());
		}
		else{
			return FALSE;   // force the programmer to use  the append function
		}
		return TRUE;
	} 
	else {
		return FALSE;
	}
	}

  

float PathPosition::GetTargetWeight(int targetNumber){
	int ct = pblock->Count(path_path_list);
	if (targetNumber >= 0 && targetNumber < ct){
		
		float pathWt;
		pblock->GetValue(path_path_weight, GetCOREInterface()->GetTime(), pathWt, FOREVER, targetNumber);
		return pathWt;
	}
	else{
		return 0.0f;
	}
}

BOOL PathPosition::SetTargetWeight(int targetNumber, float weight){
	int ct = pblock->Count(path_path_list);
	if (targetNumber >= 0 && targetNumber < ct){
		pblock->SetValue(path_path_weight, GetCOREInterface()->GetTime(), weight, targetNumber);
		return TRUE;
	}
	else{
		return FALSE;
	}
}


void PathPosition::SetTracking(float t)
	{
	float ut = ToTrackUI(t);
	pblock->SetValue(path_smoothness, GetCOREInterface()->GetTime(), ut);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}

float PathPosition::GetTracking()
	{
	return FromTrackUI(pblock->GetFloat(path_smoothness, GetCOREInterface()->GetTime()));
	}

void PathPosition::SetBankAmount(float a)
	{
	float ua = ToBankUI(a);
	pblock->SetValue(path_bank_amount, GetCOREInterface()->GetTime(), ua);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);		
	}

float PathPosition::GetBankAmount()
	{
	return FromBankUI(pblock->GetFloat(path_bank_amount, GetCOREInterface()->GetTime()));
	}

void PathPosition::SetFollow(BOOL f)
	{
	pblock->SetValue(path_follow, GetCOREInterface()->GetTime(), f);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}

void PathPosition::SetAllowFlip(BOOL f)
	{
	pblock->SetValue(path_allow_upsidedown, GetCOREInterface()->GetTime(), f);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}

void PathPosition::SetConstVel(BOOL cv)
	{
	pblock->SetValue(path_constant_vel, GetCOREInterface()->GetTime(), cv);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}

void PathPosition::SetBank(BOOL b)
	{
	pblock->SetValue(path_bank, GetCOREInterface()->GetTime(), b);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);	
	}

void PathPosition::SetFlip(BOOL onOff)
	{
	pblock->SetValue(path_axis_flip, GetCOREInterface()->GetTime(), onOff);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);	
	}

void PathPosition::SetAxis(int axis)
	{
	pblock->SetValue(path_axis, GetCOREInterface()->GetTime(), axis);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}

// Next three AG added 2/11/2000

void PathPosition::SetLoop(BOOL l)
	{
	pblock->SetValue(path_loop, GetCOREInterface()->GetTime(), l);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}

void PathPosition::SetRelative(BOOL rel)
	{
	pblock->SetValue(path_relative, GetCOREInterface()->GetTime(), rel);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}

int PathPosition::GetAxis()
	{
	if (flags&PATHFLAG_YAXIS) return 1;
	if (flags&PATHFLAG_ZAXIS) return 2;
	return 0;
	}

/*--------------------------------------------------------------------*/
// PathPosition UI

void PathPosition::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
	{	
	this->ip = ip;
	editCont = this;
	if (flags & BEGIN_EDIT_HIERARCHY) {
// JBW: JointParam stuff not handled by ParamBlock2 yet		
		// No IK if follow is on
		if (Follow()) return;

		JointParamsPath *jp = (JointParamsPath*)GetProperty(PROPID_JOINTPARAMS);
		InterpCtrlUI *ui;	

		if (!jp) {
			jp = new JointParamsPath();
			SetProperty(PROPID_JOINTPARAMS,jp);
			}

		if (prev &&
			prev->ClassID()==ClassID() && 
		    (ui = (InterpCtrlUI*)prev->GetProperty(PROPID_INTERPUI))) {
			JointParams *prevjp = (JointParams*)prev->GetProperty(PROPID_JOINTPARAMS);
			prevjp->EndDialog(ui);
			ui->cont = this;
			ui->ip   = ip;
			prev->SetProperty(PROPID_INTERPUI,NULL);
			JointDlgData *jd = (JointDlgData*)GetWindowLongPtr(ui->hParams,GWLP_USERDATA);
			jd->jp = jp;
			jp->InitDialog(ui);
		} else {
			ui = new InterpCtrlUI(NULL,ip,this);
			DWORD f=0;
			if (!jp || !jp->RollupOpen()) f = APPENDROLL_CLOSED;	

			ui->hParams = ip->AddRollupPage( 
				hInstance, 
				MAKEINTRESOURCE(IDD_PATHJOINTPARAMS),
				JointParamDlgProc,
				GetString(IDS_RB_PATHJOINTPARAMS), 
				(LPARAM)new JointDlgData(ui,jp),f);	
			}
	
		SetProperty(PROPID_INTERPUI,ui);		
	} else {
		pathCD.BeginEditParams(ip, this, flags, prev);
		}

	ip->RegisterTimeChangeCallback(&pathTimeChangeCallback);
	pathTimeChangeCallback.pcontroller = this;

	IParamMap2* pmap = pblock->GetMap();
	if (pmap) hWnd = pmap->GetHWnd();

	int ct = pblock->Count(path_path_list);
	if (last_selection < 0){
		RedrawListbox(GetCOREInterface()->GetTime());
	}
	else {
		RedrawListbox(GetCOREInterface()->GetTime(), last_selection);
		ISpinnerControl* spin = GetISpinner(GetDlgItem(hWnd, IDC_PATH_WEIGHT_SPINNER));
		if (spin != NULL){
			float value = pblock->GetFloat(path_path_weight, GetCOREInterface()->GetTime(), last_selection);
			spin->SetValue(value, FALSE);
		}
	}
	path_paramblk.ParamOption(path_path_list, p_validator, &validator);

}

void PathPosition::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
{
	this->ip = NULL;
	ip->ClearPickMode();
	editCont = NULL;
	IParamMap2* pmap = pblock->GetMap();
	if (pmap != NULL)
	{
		if (next && next->ClassID() == ClassID() && ((PathPosition*)next)->pblock)
		{
			pmap->SetParamBlock(((PathPosition*)next)->pblock);
			ip->ClearPickMode();
		}
		else
			pathCD.EndEditParams(ip, this, flags | END_EDIT_REMOVEUI, next);
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

		ip->UnRegisterTimeChangeCallback(&pathTimeChangeCallback);
		path_paramblk.ParamOption(path_path_list, p_validator, NULL);
		hWnd = NULL;
}

TSTR PathPosition::SvGetRelTip(IGraphObjectManager *gom, IGraphNode *gNodeTarger, int id, IGraphNode *gNodeMaker)
{
	return SvGetName(gom, gNodeMaker, false) + " -> " + gNodeTarger->GetAnim()->SvGetName(gom, gNodeTarger, false);
}

bool PathPosition::SvCanDetachRel(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker)
{
	return true;
}

bool PathPosition::SvDetachRel(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker)
{
	for( int i=0; i<GetNumTargets(); i++ ) {
		if( GetNode(i) == gNodeTarget->GetAnim() ) {
			DeleteTarget(i);
			return true;
		}
	}
	return false;
}

bool PathPosition::SvCanConcludeLink(IGraphObjectManager *gom, IGraphNode *gNode, IGraphNode *gNodeChild)
{
	Animatable *pChildAnim = gNodeChild->GetAnim();
	if( pChildAnim->SuperClassID() == BASENODE_CLASS_ID ) {
		const ObjectState& os = ((INode*)pChildAnim)->EvalWorldState(0);
		Object* ob = os.obj;
		if( ob && (ob->SuperClassID() == SHAPE_CLASS_ID) )
			return true;
		return false;
	}

	return Control::SvCanConcludeLink(gom, gNode, gNodeChild);
}

bool PathPosition::SvLinkChild(IGraphObjectManager *gom, IGraphNode *gNodeThis, IGraphNode *gNodeChild)
{
	Animatable *pChildAnim = gNodeChild->GetAnim();
	if( pChildAnim->SuperClassID() == BASENODE_CLASS_ID ) {
		const ObjectState& os = ((INode*)pChildAnim)->EvalWorldState(0);
		Object* ob = os.obj;
		if( ob && (ob->SuperClassID() == SHAPE_CLASS_ID) ) {
			if( AppendTarget( (INode*)pChildAnim ) ) {
				NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
				GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
				return true;
			}
		}
	}

	return Control::SvLinkChild(gom, gNodeThis, gNodeChild);
}

SvGraphNodeReference PathPosition::SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags)
{
	SvGraphNodeReference nodeRef = Control::SvTraverseAnimGraph( gom, owner, id, flags );

	if( nodeRef.stat == SVT_PROCEED ) {
		for( int i=0; i<GetNumTargets(); i++ ) {
			gom->AddRelationship( nodeRef.gNode, GetNode(i), i, RELTYPE_CONSTRAINT );
		}
	}

	return nodeRef;
}

int PathPosition::SetProperty(ULONG id, void *data)
	{
	if (id==PROPID_JOINTPARAMS) {		
		if (!data) {
			int index = aprops.FindProperty(id);
			if (index>=0) {
				aprops.Delete(index,1);
				}
		} else {
			JointParamsPath *jp = (JointParamsPath*)GetProperty(id);
			if (jp) {
				*jp = *((JointParamsPath*)data);
				delete (JointParamsPath*)data;
			} else {
				aprops.Append(1,(AnimProperty**)&data);
				}					
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

void* PathPosition::GetProperty(ULONG id)
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


#define JOINTPARAMPATH_CHUNK		0x1001
#define FOLLOW_CHUNK				0x1002
#define BANK_CHUNK					0x1003
#define BANKAMOUNT_CHUNK			0x1004
#define	TRACKING_CHUNK				0x1005
#define ALLOWFLIP_CHUNK				0x1006
#define CONSTVEL_CHUNK				0x1007
#define AXIS_CHUNK					0x1008
#define FLIP_CHUNK					0x1009
#define OFFSET_POS_LOCAL_CHUNK		0x1011
#define OFFSET_POS_WORLD_CHUNK		0x1012
#define OLD_TARGET_NUMBER_CHUNK		0x1013

IOResult PathPosition::Save(ISave *isave)
	{	
	ULONG nb;
	JointParamsPath *jp = (JointParamsPath*)GetProperty(PROPID_JOINTPARAMS);

	if (jp) {
		isave->BeginChunk(JOINTPARAMPATH_CHUNK);
		jp->Save(isave);
		isave->EndChunk();
	}

		// Save basePointLocal
	isave->BeginChunk(OFFSET_POS_LOCAL_CHUNK);
	isave->Write(&basePointLocal, sizeof(basePointLocal), &nb);
	isave->EndChunk();

		// Save basePointWorld
	isave->BeginChunk(OFFSET_POS_WORLD_CHUNK);
	isave->Write(&basePointWorld, sizeof(basePointWorld), &nb);
	isave->EndChunk();

		// Save oldTargetNumber
	isave->BeginChunk(OLD_TARGET_NUMBER_CHUNK);
	isave->Write(&oldTargetNumber,sizeof(oldTargetNumber),&nb);	
	isave->EndChunk();
	
	
	return IO_OK;
	}


// provide a post-load callback so old-version path controller data can be loaded into the ParamBlock2
class PathPLCB : public PostLoadCallback 
{
public:
	PathPosition*	p;
	BOOL			old_version;
	BOOL			flip;
	int				axis;
	BOOL			constvel;
	BOOL			follow;	
	BOOL			bank;	
	BOOL			allowFlip;
	float			bankAmt;	
	float			tracking;

	PathPLCB(PathPosition* pth)
	{ 
		p = pth;
		old_version = FALSE; flip = FALSE; axis = 0; constvel = FALSE;
	    follow = FALSE; bank = FALSE; allowFlip = FALSE; bankAmt = 0.5; tracking = 0.5;
	}
	void proc(ILoad *iload)
	{
		if (old_version)
		{
			// loading an old version, set the param values
			p->pblock->SetValue(path_axis_flip, 0, flip);
			p->pblock->SetValue(path_axis, 0, axis);
			p->pblock->SetValue(path_constant_vel, 0, constvel);
			p->pblock->SetValue(path_follow, 0, follow);
			p->pblock->SetValue(path_bank, 0, bank);
			p->pblock->SetValue(path_allow_upsidedown, 0, allowFlip);
			bankAmt = ToBankUI(bankAmt);
			p->pblock->SetValue(path_bank_amount, 0, bankAmt);
			tracking = ToTrackUI(tracking);
			p->pblock->SetValue(path_smoothness, 0, tracking);
			// copy across old percent controller
			if (p->old_percent != NULL)
				p->pblock->SetController(path_percent, 0, p->old_percent, FALSE);
		}
		else
			// make all params call their PBAccessor::Set() fns to set up flags from just-loaded pblock values
			p->pblock->CallSets();

		// compatibility between R3 and R4

//		INode* compat_path;
		float wt = 50.0;
		if(p->patht != NULL){										// that is there is an old-style node
			p->pblock->Append(path_path_list, 1, &p->patht, 1);		// param mapping. add the single path to the new path list
			p->pblock->Append(path_path_weight, 1, &wt, 1);				// parameter mapping. assign wt=50 to the single path
			p->ReplaceReference(PATHPOS_PATH_REF, NULL);
		}
		delete this;

	}
};

IOResult PathPosition::Load(ILoad *iload)
	{
	ULONG nb;
	IOResult res = IO_OK;
	PathPLCB* plcb = new PathPLCB(this);
	iload->RegisterPostLoadCallback(plcb);

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case FLIP_CHUNK:
				plcb->old_version = TRUE;
				plcb->flip = TRUE;
			break;

			case AXIS_CHUNK:
				plcb->old_version = TRUE;
				res=iload->Read(&plcb->axis,sizeof(int),&nb);
			break;

			case CONSTVEL_CHUNK:
				plcb->old_version = TRUE;
				plcb->constvel = TRUE;
			break;

			case FOLLOW_CHUNK:
				plcb->old_version = TRUE;
				plcb->follow = TRUE;
			break;

			case BANK_CHUNK:
				plcb->old_version = TRUE;
				plcb->bank = TRUE;
			break;

			case ALLOWFLIP_CHUNK:
				plcb->old_version = TRUE;
				plcb->allowFlip = TRUE;
			break;

			case BANKAMOUNT_CHUNK:
				plcb->old_version = TRUE;
				res=iload->Read(&plcb->bankAmt,sizeof(bankAmount),&nb);
			break;

			case TRACKING_CHUNK:
				plcb->old_version = TRUE;
				res=iload->Read(&plcb->tracking,sizeof(tracking),&nb);
			break;

			case JOINTPARAMPATH_CHUNK: {
				JointParamsPath *jp = new JointParamsPath;
				jp->Load(iload);
				SetProperty(PROPID_JOINTPARAMS,jp);
			break;
			}

			case OFFSET_POS_LOCAL_CHUNK:
				res=iload->Read(&basePointLocal, sizeof(basePointLocal), &nb);
			break;

			case OFFSET_POS_WORLD_CHUNK:
				res=iload->Read(&basePointWorld, sizeof(basePointWorld), &nb);
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



//-------------------------------------------------------------
//
// IK
//


void PathPosition::EnumIKParams(IKEnumCallback &callback)
	{
	JointParamsPath *jp = (JointParamsPath*)GetProperty(PROPID_JOINTPARAMS);
	if (jp && jp->Active(0) && !Follow()) {
		callback.proc(this,0);
		}
	}


BOOL PathPosition::CompDeriv(TimeValue t, Matrix3& ptm, IKDeriv& derivs, DWORD flags)
	{
	JointParamsPath *jp = (JointParamsPath*)GetProperty(PROPID_JOINTPARAMS);
	if (!jp || !jp->Active(0) || Follow()) return FALSE;
	Point3 zero(0,0,0);

/* TO REMOVE SOON 1/24/00 */	

		INode *path;
		pblock->GetValue(path_path_list, t, path, FOREVER, 0);

  if (path) {
		ObjectState os = path->EvalWorldState(t);
		if (os.obj->IsShapeObject()) {
			ShapeObject *pathOb = (ShapeObject*)os.obj;
			if (pathOb->NumberOfCurves()) {
				Point3 p0, p1, d;
				float per, dt = 0.01f;
				pblock->GetValue(path_percent, t, per, FOREVER);
				per = (float)fmod(per,1.0f);
				if (per < 0.0f) per = 1.0f + per;
				if (dt + per > 1.0f) dt = -dt;
				p0 = pathOb->InterpCurve3D(t, 0, per, InterpType());
				p1 = pathOb->InterpCurve3D(t, 0, per + dt, InterpType());
				if (os.GetTM()) {
					p0 = p0 * (*os.GetTM());
					p1 = p1 * (*os.GetTM());
					}
				d = (p1-p0)/dt;
				for (int j = 0; j<derivs.NumEndEffectors(); j++) {
					if (flags&POSITION_DERIV) {
						derivs.DP(d,j);
						}
					if (flags&ROTATION_DERIV) {
						derivs.DR(zero,j);
						}
					}
				derivs.NextDOF();
				ptm.SetTrans(p0);
				return TRUE;
				}
			}	
		}
	
	for (int j=0; j<derivs.NumEndEffectors(); j++) {
		if (flags&POSITION_DERIV) {
			derivs.DP(zero,j);
			}
		if (flags&ROTATION_DERIV) {
			derivs.DR(zero,j);
			}
		}
	derivs.NextDOF();

// were removed upto this

	return FALSE;
	}

float PathPosition::IncIKParam(TimeValue t, int index, float delta)
	{
	incFlag = 1;
	JointParamsPath *jp = (JointParamsPath*)GetProperty(PROPID_JOINTPARAMS);
	float v = 0.0f;
	BOOL gotV = FALSE;
	if (fabs(delta) > 0.01f) {
		if (delta < 0) delta = -0.01f;
		else delta = 0.01f;
		}
	if (jp) {		
		if (jp->Limited(0)) {			
			pblock->GetValue(path_percent, t, v, FOREVER);
			gotV = TRUE;
			}
		delta = jp->ConstrainInc(0,v,delta);
		}	
	// If the path is not closed, do not let it go off the end.
	if (!(flags&PATHFLAG_CLOSED)) {
		if (!gotV) {
			pblock->GetValue(path_percent, t, v, FOREVER);
			}
		if (v + delta < 0.0f) delta = -v;
		if (v + delta > 1.0f) delta = 1.0f-v;
		}
	pblock->GetController(path_percent)->SetValue(t,&delta,FALSE,CTRL_RELATIVE); //3rd parameter true
	return delta;
	}

void PathPosition::ClearIKParam(Interval iv,int index) 
	{
	pblock->GetController(path_percent)->DeleteTime(iv,TIME_INCRIGHT|TIME_NOSLIDE);
	}

void JointParamsPath::SpinnerChange(
		InterpCtrlUI *ui,WORD id,ISpinnerControl *spin,BOOL interactive)
	{
	float val;
	BOOL set = FALSE;

	switch (id) {
		case IDC_XFROMSPIN:
			val = min[0] = spin->GetFVal()/scale; 
			set = TRUE;
			break;
		case IDC_XTOSPIN:
			val = max[0] = spin->GetFVal()/scale;
			set = TRUE;
			break;
		
		case IDC_XDAMPINGSPIN:
			damping[0] = spin->GetFVal(); break;
		}
	
	if (set && interactive) {
		PathPosition *c = (PathPosition*)ui->cont;
 		c->pblock->GetController(path_percent)->SetValue(ui->ip->GetTime(),&val,TRUE,CTRL_ABSOLUTE);
		ui->ip->RedrawViews(ui->ip->GetTime(),REDRAW_INTERACTIVE);
		}
	}


