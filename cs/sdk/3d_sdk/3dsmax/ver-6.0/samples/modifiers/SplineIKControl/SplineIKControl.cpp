/**********************************************************************
 *<
	FILE: SplineIKControl.cpp

	DESCRIPTION:	
	
		This is a modifier and can be used independently of the SplineIK. When applied 
		to a spline, this modifier creates one point helper for each knot of the spline. 
		The user can then animate the spline by simply animnating (position and rotation) 
		the point helpers. Thus to animate the spline, the user wouldn’t need to get 
		into the sub-object level. The modifier comes integrated with all the options of a 
		point helper and they function identically. The UI controls are uniformly active 
		on all the point helpers created by the modifier. The controls are: 

			- central marker (checkbox) - default OFF
			- axis tripod (checkbox) - default OFF
			- cross (checkbox) - default OFF
			- box (checkbox) - default ON
			- constant screen size (checkbox) - default OFF
			- draw on top (checkbox) - default ON
			- helper size (edit box and spinner) - deafult 20.0
	
	  In addition, there are three options, presented as a set of 3 radiobuttons:
		- Link All in Hierarchy (default):

			Makes each helper a child to its immediately previous helper. So Helper#2 is 
			child to Helper#1, Helper#3  is child to Helper#2, and so on. Helper#1 is 
			still child to the world. Translation and rotation of a helper then "solidly" 
			moves/rotates part of the spline _subsequent_ to the selected helper. The part 
			of the spline previous to the helper is unaffected.

		- Link All to Root
			
			Makes all helpers children to Helper#1, i.e., knot#1. Helper#1 can be 
			position constrained or linked to another object, like it is possible above.
			Additionally individual helpers can be moved and rotated without any other
			helper being affect.

		- No Linking
			
			All helpers are independent -- not linked to any other helper -- so that 
			they can be moved and rotated without any other	helper being affect.

		
		"Create Helpers" button:
		
			 Helpers are not automatically added to the spline on the assignment of the 
			 modifier. To do that the user need to press the "Create Helpers" button. 

		If the user adds ("insert") a knot to the spline, a new helper object 
		is automatically created at that knotpoint.


	CREATED BY: Ambarish Goswami

	HISTORY: 

 *>	Copyright (c) 2001-2002, All Rights Reserved.
 **********************************************************************/

#include "SplineIKControl.h"
#include "splshape.h"
#include "modstack.h"
#include "surf_api.h"
#include "notify.h"


#define MIN_NODECOUNT 2
#define MAX_NODECOUNT 100

#define DEFAULTDUMMYSIZE  20.0f    // dummies will be 10 units on each edge

class SplineIKControlModData;

//TODO: Add enums for various parameters
//enum {  
//		sm_point_node_list,		sm_helpersize,	sm_helper_centermarker,		sm_helper_axistripod, 
//		sm_helper_cross,		sm_helper_box,	sm_helper_screensize,	sm_helper_drawontop,
//		sm_link_types,
//
//};


class SMNodeNotify;
class SplineIKControl : public ISplineIKControl {
	public:
	
		BOOL CreateHelpers(int knotCt);
		Tab<int> deleted_nodes;
		static void NotifyPreDeleteNode(void* param, NotifyInfo*);
		static void NotifyPreNotifyDep(void*, NotifyInfo*);
		static void NotifyPostNotifyDep(void*, NotifyInfo*);
		BOOL DeleteHelper(int i);
		BOOL notASplineOrNURBS;
		
	
		
		int noHelperNearThisKnot(Point3 kPos);
		void RedrawListbox(TimeValue t, int sel = -1);			// AG added
		HWND hWnd;
		bool assignmentFlag;
//		int helpCreationFlag;
		int nKnots;
		INode *firstNode; // the helper node attached to the first knot point of the spline
		// Parameter block
		IParamBlock2	*pblock2;	//ref 0
		SMNodeNotify *notify;
	
		INode *splNode;
		Matrix3 splNodeTM;
		Class_ID cid;
		SClass_ID sid;


		int GetHelperCount();
		int GetKnotCount();
		float GetHelperSize(); // All Helpers Have Same size for now
		BOOL SetHelperSize(float hsize);
		BOOL LinkInHierarchy();
		BOOL LinkToRoot();
		BOOL UnLink();

		static IObjParam *ip;			//Access to the interface
		static SplineIKControl* curMod;
		
		// From Animatable
		TCHAR *GetObjectName() { return GetString(IDS_CLASS_NAME); }


		//From Modifier
		ChannelMask ChannelsUsed()  { return GEOM_CHANNEL|TOPO_CHANNEL; }
		//TODO: Add the channels that the modifier actually modifies
		ChannelMask ChannelsChanged() { return GEOM_CHANNEL|TOPO_CHANNEL; }
		//TODO: Return the ClassID of the object that the modifier can modify
		Class_ID InputType() {return genericShapeClassID;}

		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc);

		void NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index);
		void NotifyPostCollapse(INode *node,Object *obj, IDerivedObject *derObj, int index);


		Interval LocalValidity(TimeValue t);

		// From BaseObject
		//TODO: Return true if the modifier changes topology
		BOOL ChangeTopology() {return TRUE;}
		
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}

//		BOOL HasUVW();
//		void SetGenUVW(BOOL sw);


		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);


		Interval GetValidity(TimeValue t);

		// Automatic texture support
		
		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		//From Animatable
		Class_ID ClassID() {return SPLINEIKCONTROL_CLASS_ID;}		
		SClass_ID SuperClassID() { return OSM_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}

		RefTargetHandle Clone( RemapDir &remap );
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
			PartID& partID,  RefMessage message);


		int NumSubs() { return 1; }
		TSTR SubAnimName(int i) { return GetString(IDS_PARAMS); }				
		Animatable* SubAnim(int i) { return pblock2; }
		int SubNumToRefNum(int subNum) {if (subNum==0) return SPLINEIKCONTROL_PBLOCK_REF; else return -1;}

		// TODO: Maintain the number or references here
		int NumRefs() { return 1; }
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock2; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock2->ID() == id) ? pblock2 : NULL; } // return id'd ParamBlock

		void DeleteThis() { delete this; }	
		
		void InvalidateUI();

		INode* GetNodeFromModContext(SplineIKControlModData *smd, int &which);
		//Constructor/Destructor

		SplineIKControl();
		~SplineIKControl();	
		
		//		Function Publishing method (Mixin Interface)

		BaseInterface* GetInterface(Interface_ID id) 
		{ 
			if (id == SPLINEIK_CONTROL_INTERFACE) 
				return (SplineIKControl*)this; 
			else 
				return FPMixinInterface::GetInterface(id);
		}
		BitArray mNotifyStack;

		// Temporary flags:
		byte mSetLinkTypes : 1;
};



class sMyEnumProc : public DependentEnumProc 
	{
      public :
      virtual int proc(ReferenceMaker *rmaker); 
      INodeTab Nodes;              
	};

int sMyEnumProc::proc(ReferenceMaker *rmaker) 
	{ 
	if (rmaker->SuperClassID()==BASENODE_CLASS_ID)    
			{
            Nodes.Append(1, (INode **)&rmaker);                 
			}
     return 0;              
	}

class SplineIKControlModData : public LocalModData {
	public:
		int id;
		INode *selfNode;
		Interval ivalid;
		SplineIKControlModData()
			{
			ivalid.SetEmpty ();
			selfNode = NULL;
			}
		SplineIKControlModData(int i)
			{
			id = i;
			selfNode = NULL;
			}
		~SplineIKControlModData()
			{
			}	
		LocalModData*	Clone()
			{
			SplineIKControlModData* d = new SplineIKControlModData();
			d->id = -1;
			d->selfNode = NULL;
			d->ivalid = NEVER;

			return d;

			}	


	};

BOOL RecursePipeAndMatch(SplineIKControlModData *smd, Object *obj)
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
				if (mc->localData == smd)
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

INode* SplineIKControl::GetNodeFromModContext(SplineIKControlModData *smd, int &which)
	{

	int	i;

    sMyEnumProc dep;              
	EnumDependents(&dep);
	for ( i = 0; i < dep.Nodes.Count(); i++)
		{
		INode *node = dep.Nodes[i];
		BOOL found = FALSE;

		if (node)
			{
			Object* obj = node->GetObjectRef();
			if (obj->ClassID() == Class_ID(XREFOBJ_CLASS_ID,0)) {
				obj = (Object*)obj->GetReference(0);
			}
	
			if ( RecursePipeAndMatch(smd,obj) )
				{
				which = i;
				return node;
				}
			}
		}
	return NULL;
	}




/*===========================================================================*\
 |	SplineIKControl
\*===========================================================================*/


class SplineIKControlClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return new SplineIKControl(); }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return SPLINEIKCONTROL_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }
	const TCHAR*	InternalName() { return _T("Spline_IK_Control"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};



static SplineIKControlClassDesc SplineIKControlDesc;
ClassDesc2* GetSplineIKControlDesc() { return &SplineIKControlDesc; }

//Function Publishing descriptor for Mixin interface
//*****************************************************


  static FPInterfaceDesc spline_ik_control_interface(
    SPLINEIK_CONTROL_INTERFACE, _T("modifiers"), 0, &SplineIKControlDesc, 0,
		ISplineIKControl::getHelperCount,		_T("getHelperCount"),	0,		TYPE_INT,	0,0,
		ISplineIKControl::getKnotCount,			_T("getKnotCount"),		0,		TYPE_INT,	0,0,
		ISplineIKControl::link_allToRoot,		_T("linkToRoot"),		0,		TYPE_BOOL,	0,0,
		ISplineIKControl::link_allinHierarchy,	_T("linkInHierarchy"),	0,		TYPE_BOOL,	0,0,
		ISplineIKControl::link_none,			_T("noLinking"),		0,		TYPE_BOOL,	0,0,
		ISplineIKControl::create_hlpr,			_T("createHelper"),		0,		TYPE_BOOL,	0,1,
							_T("knotCount"), 0, TYPE_INT,
		end
		);



//  Get Descriptor method for Mixin Interface
//  *****************************************
FPInterfaceDesc* ISplineIKControl::GetDesc()
{
     return &spline_ik_control_interface;
}


//*********************************************
// End of Function Publishing Code




//enum { SplineIKControl_params };


class SplineIKControlPBAccessor : public PBAccessor
{ 
public:

	void TabChanged(tab_changes changeCode, Tab<PB2Value>* tab, 
                  ReferenceMaker* owner, ParamID id, int tabIndex, int count) 
		{ 

		if (id == ISplineIKControl::sm_point_node_list){

			SplineIKControl* sm = (SplineIKControl*)owner;
			int ct = sm->pblock2->Count(ISplineIKControl::sm_point_node_list);
			if (changeCode == tab_ref_deleted){	
//				MessageBox(GetCOREInterface()->GetMAXHWnd(), "All animation for this knot will be lost", "Warning!", MB_ICONSTOP|MB_OK);
//				int j = sm->pblock2->Delete(sm_point_node_list, tabIndex, 1); // deletes the node from nodelist;
				sm->RedrawListbox(GetCOREInterface()->GetTime());
			}


		}
	}
		
	void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid)
	{
		SplineIKControl* sikc = (SplineIKControl*)owner;
//		switch (id)
//		{
//			case sm_nodecount_spin:
//				if ((v.i) < 2) v.i = 2;; 
//			break;
//		}
	}

	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		SplineIKControl* sikc = (SplineIKControl*)owner;
		int i, ct;
		switch (id)
		{
			case ISplineIKControl::sm_point_node_list:
				if (sikc->nKnots != sikc->pblock2->Count(ISplineIKControl::sm_point_node_list))
					EnableWindow(GetDlgItem(sikc->hWnd, IDC_SIK_CREATE_HELPERS_PICKBTN), TRUE);
				else
					EnableWindow(GetDlgItem(sikc->hWnd, IDC_SIK_CREATE_HELPERS_PICKBTN), FALSE);
			break;

			case ISplineIKControl::sm_helpersize:
				float size;
				sikc->pblock2->GetValue(ISplineIKControl::sm_helpersize, t, size, FOREVER);
				ct = sikc->pblock2->Count(ISplineIKControl::sm_point_node_list);
	
				for (i = 0; i < ct; i++){
					INode *sm_point_node;
					sikc->pblock2->GetValue(ISplineIKControl::sm_point_node_list, t, sm_point_node, FOREVER, i);

					if (sm_point_node == NULL) continue;

					Object *hObj;
					hObj = sm_point_node->GetObjectRef();
					hObj->GetParamBlockByID(pointobj_params)->SetValue(pointobj_size, t, size);
				}

			break;

			case ISplineIKControl::sm_helper_centermarker:
				BOOL cmrkr;
				sikc->pblock2->GetValue(ISplineIKControl::sm_helper_centermarker, t, cmrkr, FOREVER);
				ct = sikc->pblock2->Count(ISplineIKControl::sm_point_node_list);
	
				for (i = 0; i < ct; i++){
					INode *sm_point_node;
					sikc->pblock2->GetValue(ISplineIKControl::sm_point_node_list, t, sm_point_node, FOREVER, i);

					if (sm_point_node == NULL) continue;

					Object *hObj;
					hObj = sm_point_node->GetObjectRef();
					hObj->GetParamBlockByID(pointobj_params)->SetValue(pointobj_centermarker, t, cmrkr);
				}
			break;

			case ISplineIKControl::sm_helper_axistripod:
				BOOL aTripod;
				sikc->pblock2->GetValue(ISplineIKControl::sm_helper_axistripod, t, aTripod, FOREVER);
				ct = sikc->pblock2->Count(ISplineIKControl::sm_point_node_list);
	
				for (i = 0; i < ct; i++){
					INode *sm_point_node;
					sikc->pblock2->GetValue(ISplineIKControl::sm_point_node_list, t, sm_point_node, FOREVER, i);

					if (sm_point_node == NULL) continue;

					Object *hObj;
					hObj = sm_point_node->GetObjectRef();
					hObj->GetParamBlockByID(pointobj_params)->SetValue(pointobj_axistripod, t, aTripod);
				}
			break;

			case ISplineIKControl::sm_helper_cross:
				BOOL crs;
				sikc->pblock2->GetValue(ISplineIKControl::sm_helper_cross, t, crs, FOREVER);
				ct = sikc->pblock2->Count(ISplineIKControl::sm_point_node_list);
	
				for (i = 0; i < ct; i++){
					INode *sm_point_node;
					sikc->pblock2->GetValue(ISplineIKControl::sm_point_node_list, t, sm_point_node, FOREVER, i);

					if (sm_point_node == NULL) continue;

					Object *hObj;
					hObj = sm_point_node->GetObjectRef();
					hObj->GetParamBlockByID(pointobj_params)->SetValue(pointobj_cross, t, crs);
				}
			break;

			case ISplineIKControl::sm_helper_box:
				BOOL bx;
				sikc->pblock2->GetValue(ISplineIKControl::sm_helper_box, t, bx, FOREVER);
				ct = sikc->pblock2->Count(ISplineIKControl::sm_point_node_list);
	
				for (i = 0; i < ct; i++){
					INode *sm_point_node;
					sikc->pblock2->GetValue(ISplineIKControl::sm_point_node_list, t, sm_point_node, FOREVER, i);

					if (sm_point_node == NULL) continue;

					Object *hObj;
					hObj = sm_point_node->GetObjectRef();
					hObj->GetParamBlockByID(pointobj_params)->SetValue(pointobj_box, t, bx);
				}
			break;

			case ISplineIKControl::sm_helper_screensize:
				BOOL cnScrnSz;
				sikc->pblock2->GetValue(ISplineIKControl::sm_helper_screensize, t, cnScrnSz, FOREVER);
				ct = sikc->pblock2->Count(ISplineIKControl::sm_point_node_list);
	
				for (i = 0; i < ct; i++){
					INode *sm_point_node;
					sikc->pblock2->GetValue(ISplineIKControl::sm_point_node_list, t, sm_point_node, FOREVER, i);

					if (sm_point_node == NULL) continue;

					Object *hObj;
					hObj = sm_point_node->GetObjectRef();
					hObj->GetParamBlockByID(pointobj_params)->SetValue(pointobj_screensize, t, cnScrnSz);
				}
			break;

			case ISplineIKControl::sm_helper_drawontop:
				BOOL dOnTop;
				sikc->pblock2->GetValue(ISplineIKControl::sm_helper_drawontop, t, dOnTop, FOREVER);
				ct = sikc->pblock2->Count(ISplineIKControl::sm_point_node_list);
	
				for (i = 0; i < ct; i++){
					INode *sm_point_node;
					sikc->pblock2->GetValue(ISplineIKControl::sm_point_node_list, t, sm_point_node, FOREVER, i);

					if (sm_point_node == NULL) continue;

					Object *hObj;
					hObj = sm_point_node->GetObjectRef();
					hObj->GetParamBlockByID(pointobj_params)->SetValue(pointobj_drawontop, t, dOnTop);
				}
			break;

			case ISplineIKControl::sm_link_types:
				sikc->mSetLinkTypes = 1;
				switch (v.i)
					{
					case 0: 						
						sikc->LinkInHierarchy();
					break;
					case 1: 						
						sikc->LinkToRoot();						
					break;
					case 2: 
						sikc->UnLink();	
					break;
				}
				sikc->mSetLinkTypes = 0;
			break;

		}
	}
};

static SplineIKControlPBAccessor sm_accessor;


static ParamBlockDesc2 SplineIKControl_param_blk ( ISplineIKControl::SplineIKControl_params, _T("params"),  0, &SplineIKControlDesc, 
	P_AUTO_CONSTRUCT + P_AUTO_UI, SPLINEIKCONTROL_PBLOCK_REF, 
	//rollout
	IDD_PANEL, IDS_PARAMS, 0, 0, NULL,
	// params

	ISplineIKControl::sm_point_node_list,  _T("helper_list"),  TYPE_INODE_TAB, 0, P_VARIABLE_SIZE,	NULL,	
		p_accessor,		&sm_accessor,
	end,

	ISplineIKControl::sm_helpersize, 	_T("helper_size"), TYPE_FLOAT, P_ANIMATABLE + P_RESET_DEFAULT, IDS_AG_SM_HELPERSIZE,
		p_default, 		20.0f,	
		p_range, 		0.0f, 100000.0f, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_SIK_HELPERSIZE_EDIT, IDC_SIK_HELPERSIZE_SPINNER, 1.0f, 
		p_accessor,		&sm_accessor,	
	end,

	ISplineIKControl::sm_helper_centermarker, 	_T("helper_centermarker"), TYPE_BOOL, P_ANIMATABLE+P_RESET_DEFAULT, IDS_AG_CENTERMARKER,
		p_default, 			FALSE,
		p_ui, 				TYPE_SINGLECHEKBOX, 	IDC_CENTERMARKER_CHECK, 
		p_accessor,		&sm_accessor,	
	end, 

	ISplineIKControl::sm_helper_axistripod, 	_T("helper_axistripod"), TYPE_BOOL, P_ANIMATABLE+P_RESET_DEFAULT, IDS_AG_AXISTRIPOD,
		p_default, 			FALSE,
		p_ui, 				TYPE_SINGLECHEKBOX, 	IDC_AXISTRIPOD_CHECK, 
		p_accessor,		&sm_accessor,	
	end, 

	ISplineIKControl::sm_helper_cross, 		_T("helper_cross"), TYPE_BOOL, P_ANIMATABLE+P_RESET_DEFAULT, IDS_AG_CROSS,
		p_default, 			FALSE,
		p_ui, 				TYPE_SINGLECHEKBOX, 	IDC_CROSS_CHECK, 
		p_accessor,		&sm_accessor,
	end, 

	ISplineIKControl::sm_helper_box, 			_T("box"), TYPE_BOOL, P_ANIMATABLE+P_RESET_DEFAULT, IDS_AG_BOX,
		p_default, 			TRUE,
		p_ui, 				TYPE_SINGLECHEKBOX, 	IDC_BOX_CHECK, 
		p_accessor,		&sm_accessor,
	end, 

	ISplineIKControl::sm_helper_screensize,	_T("constantscreensize"), TYPE_BOOL, P_ANIMATABLE+P_RESET_DEFAULT, IDS_AG_CNSTSCREENSIZE,
		p_default, 			FALSE,
		p_ui, 				TYPE_SINGLECHEKBOX, 	IDC_CNSTSCREENSIZE_CHECK,
		p_accessor,		&sm_accessor,
	end, 

	ISplineIKControl::sm_helper_drawontop,	    _T("drawontop"),       TYPE_BOOL, P_ANIMATABLE+P_RESET_DEFAULT, IDS_AG_DRAWONTOP,
		p_default, 			TRUE,
		p_ui, 				TYPE_SINGLECHEKBOX, 	IDC_DRAWONTOP_CHECK,
		p_accessor,		&sm_accessor,
	end, 

	ISplineIKControl::sm_link_types,		_T("linkTypes"),		TYPE_INT, 		0,		IDS_AG_LINK_TYPES,
		p_default, 		0, 
		p_range, 		0, 2, 
		p_ui, 			TYPE_RADIO, 	3, IDC_LINK_IN_HIERARCHY, IDC_LINK_TO_ROOT, IDC_LINK_TO_NONE, 
		p_accessor,		&sm_accessor,
	end, 

	end
	);



IObjParam*					SplineIKControl::ip				= NULL;
SplineIKControl*			SplineIKControl::curMod			= NULL;


//--- SplineIKControl -------------------------------------------------------
SplineIKControl::SplineIKControl()
{
	assignmentFlag = false;
	SplineIKControlDesc.MakeAutoParamBlocks(this);
	assert(pblock2);
	pblock2->CallSets();
//	helpCreationFlag = 1;
	splNode = NULL;
	splNodeTM.IdentityMatrix();
	cid = Class_ID();
	sid = SClass_ID();
	nKnots = 2;
	firstNode =  NULL;
	notASplineOrNURBS = 0;

	RegisterNotification(NotifyPreDeleteNode, this, NOTIFY_SCENE_PRE_DELETED_NODE);
	RegisterNotification(NotifyPreNotifyDep, this, NOTIFY_PRE_NOTIFYDEPENDENTS);
	RegisterNotification(NotifyPostNotifyDep, this, NOTIFY_POST_NOTIFYDEPENDENTS);
	
//	ivalid.SetEmpty();
}

SplineIKControl::~SplineIKControl()
{

		DeleteAllRefsFromMe();
		UnRegisterNotification(NotifyPreDeleteNode, this,	NOTIFY_SCENE_PRE_DELETED_NODE);
		UnRegisterNotification(NotifyPreNotifyDep, this, NOTIFY_PRE_NOTIFYDEPENDENTS);
		UnRegisterNotification(NotifyPostNotifyDep, this, NOTIFY_POST_NOTIFYDEPENDENTS);
		pblock2 = NULL;
}


void SplineIKControl::NotifyPreDeleteNode(void* parm, NotifyInfo* arg){

	SplineIKControl* sik = (SplineIKControl*)parm;
	if(sik == NULL) return;
//	Tab<int> deleted_nodes;

	if((arg != NULL) && (sik->pblock2 != NULL)){
		INode* deleted_node = (INode*)arg->callParam;
		int ct = sik->pblock2->Count(sm_point_node_list);
		int deleted_nodes_number = 0;
		for(int i=0; i <ct; i++ ){
			INode* someNode;
			sik->pblock2->GetValue(sm_point_node_list, GetCOREInterface()->GetTime(), someNode, FOREVER, i);
			if (deleted_node == someNode){
				sik->deleted_nodes.SetCount(deleted_nodes_number+1);
				sik->deleted_nodes[deleted_nodes_number] = i;
				deleted_nodes_number++;
			}
		}
		if(deleted_nodes_number){
//			MessageBox(GetCOREInterface()->GetMAXHWnd(), "All animation for this knot will be lost. If this was not intended, hit OK and then CTRL+Z", "Warning!", MB_ICONSTOP|MB_OK);

			for (i=0; i <sik->deleted_nodes.Count(); i++ ){
				sik->deleted_nodes[i] = sik->deleted_nodes[i]-i;
			}
			for(int j=0; j <sik->deleted_nodes.Count(); j++ ){
				sik->DeleteHelper(sik->deleted_nodes[j]);
			}
		}
	}

}

void SplineIKControl::NotifyPreNotifyDep(void* parm, NotifyInfo* arg)
{
	SplineIKControl* me = (SplineIKControl*)parm;
	if (me == NULL) return;
	me->mNotifyStack.SetSize(me->mNotifyStack.GetSize() + 1, 1);
}

void SplineIKControl::NotifyPostNotifyDep(void* parm, NotifyInfo* arg)
{
	SplineIKControl* me = (SplineIKControl*)parm;
	if (me == NULL) return;
	int sz = me->mNotifyStack.GetSize();
	if (--sz >= 0) {
		me->mNotifyStack.SetSize(sz, 1);
	}
}

BOOL SplineIKControl::DeleteHelper(int i)
	{

	if(pblock2){

		int ct = pblock2->Count(sm_point_node_list);
		if (i < 0 || i >= ct) return FALSE;

	// This method doesn't handle the case when a node is deleted from the viewport
	// John has added some methods to handle this case -- try it!

		//TimeValue t = GetCOREInterface()->GetTime();

		// Should not suspend the Hold. This caused the reference from the
		// PB2 to the helper to be lost after undo, and therefore no longer
		// control the spline. (425288) -J.Zhao, 11/9/02.
		// 
		//theHold.Suspend();
		pblock2->Delete(sm_point_node_list, i, 1); //deleting the frame# entry of the deleted target node
		//theHold.Resume();
		
		NotifyDependents(FOREVER,0,REFMSG_CHANGE);
		return TRUE;
	}
	return FALSE;
	}



class SplineIKControlDlgProc : public ParamMap2UserDlgProc 
{
	public:
		

		SplineIKControl *sm;
		SplineIKControlDlgProc() {}
		SplineIKControlDlgProc(SplineIKControl *sm_in) { sm = sm_in; }

		void DeleteThis() { delete this;}
		void UpdateSMName(SplineIKControl* p){
			IParamMap2* pmap = p->pblock2->GetMap();
		}

		void SetThing(ReferenceTarget *m) {
			sm = (SplineIKControl*)m;
			}

		BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			SplineIKControl *sikc = (SplineIKControl*)map->GetParamBlock()->GetOwner();
			UpdateSMName(sm);
			sikc->hWnd = hWnd;
			
			int id = LOWORD(wParam);
			ICustButton *iBut1;
			switch (msg) 
			{
				case WM_INITDIALOG:

					iBut1 = GetICustButton(GetDlgItem(hWnd,IDC_SIK_CREATE_HELPERS_PICKBTN));
					iBut1->SetType(CBT_CHECK);
					iBut1->SetHighlightColor(GREEN_WASH);
					ReleaseICustButton(iBut1);

					if ((sikc->nKnots != sikc->pblock2->Count(ISplineIKControl::sm_point_node_list)) && (!sikc->notASplineOrNURBS))
						EnableWindow(GetDlgItem(hWnd, IDC_SIK_CREATE_HELPERS_PICKBTN), TRUE);
					else
						EnableWindow(GetDlgItem(hWnd, IDC_SIK_CREATE_HELPERS_PICKBTN), FALSE);
					break;

				case WM_DESTROY:
					break;
				case WM_COMMAND:
					if (LOWORD(wParam) == IDC_SIK_CREATE_HELPERS_PICKBTN){

						// putting theHold here crashes max when you undo this modifier and try to create a 
						// new spline. Need to fix this.=
//						if(!theHold.Holding()){
							theHold.Begin();
//						}
						sikc->CreateHelpers(sikc->nKnots);
						theHold.Accept(_T("Helper Creation"));
					}
				 break;			

			}
			return FALSE;
		}
		void SetParamBlock(IParamBlock2* pb) 
		{ 
			UpdateSMName((SplineIKControl*)pb->GetOwner());
		}
};


void SplineIKControl::RedrawListbox(TimeValue t, int sel)
{
	if (hWnd == NULL) return;
	if(!SplineIKControlDesc.NumParamMaps()) return;
	int selection = SendDlgItemMessage(hWnd, IDC_CTRL_OBJ_LIST, LB_GETCURSEL, 0, 0);
	SendDlgItemMessage(hWnd, IDC_CTRL_OBJ_LIST, LB_RESETCONTENT, 0, 0);
	int ct = pblock2->Count(sm_point_node_list);
	for (int i = 0; i < ct; i++){
		TSTR str;
		if (pblock2->GetINode(sm_point_node_list, t, i) != NULL){	
			str.printf(_T("%-s %-d   %-s"), 
				_T("Knot# "),
				(int)i+1, 
				pblock2->GetINode(sm_point_node_list, t, i)->GetName());
		}
		else{
			str.printf(_T("%-s %-d   %-s"), 
				_T("Knot# "), 
				(int)i+1,
				_T("No Helper"));
		}
		SendDlgItemMessage(hWnd, IDC_CTRL_OBJ_LIST, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR) str.data());	
	}

}




Interval SplineIKControl::LocalValidity(TimeValue t)
{
	// if being edited, return NEVER forces a cache to be built 
	// after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;  
	
	//TODO: Return the validity interval of the modifier
	Interval valid = GetValidity(t);
	return valid;

}



void SplineIKControl::NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index)
{
	//TODO:  Perform any Pre Stack Collapse methods here
}



void SplineIKControl::NotifyPostCollapse(INode *node,Object *obj, IDerivedObject *derObj, int index)
{
	//TODO: Perform any Post Stack collapse methods here.

}


void SplineIKControl::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) 
{


        // Get our personal validity interval...
	Interval valid = GetValidity(t);
	// and intersect it with the channels we use as input (see ChannelsUsed)
	
	cid = os->obj->ClassID();
	sid = os->obj->SuperClassID();


	if (cid == EDITABLE_SURF_CLASS_ID){  //it's a NURBS point or CV curve
#ifndef NO_NURBS		
		notASplineOrNURBS = 0;
		NURBSSet getSet;
		BOOL okay = GetNURBSSet(os->obj, GetCOREInterface()->GetTime(), getSet, TRUE);
		if (okay) {
			int obCount = getSet.GetNumObjects();
			for(int j = 0; j < obCount; j++){
				if (getSet.GetNURBSObject(j)->GetType() == kNPointCurve || 
					getSet.GetNURBSObject(j)->GetType() == kNCVCurve){	
					nKnots = os->obj->NumPoints();
					break;
				}

			} 
			
			if (!os->obj) return;

			int ct = pblock2->Count(sm_point_node_list); //should be = nKnots
			if(ct <= 0 || ct != nKnots)return;
			SplineIKControlModData *d  = new SplineIKControlModData(0);    
			mc.localData = d;

			int id;
			splNode = GetNodeFromModContext(d,id);
			if (splNode == NULL) {
				DbgAssert(0);
				return;
			}
			splNodeTM = splNode->GetObjectTM(t);

			if (assignmentFlag == false){  // enters loop only for the first time
				assignmentFlag = true;
				for (int i = 0; i < ct; i++){
					INode *sm_point_node;
					pblock2->GetValue(sm_point_node_list, t, sm_point_node, valid, i);

					if (sm_point_node == NULL) continue;

					Matrix3 xfm(1);
					Point3 knotPosition;
					Point3 nPoint = os->obj->GetPoint(i);
					knotPosition = nPoint * splNodeTM;
					if (noHelperNearThisKnot(knotPosition)){
						xfm.SetTrans (knotPosition);  //setting helper position is done inside ModifyObject
						sm_point_node->SetNodeTM (0, xfm);
					}
				}
			}
			else { //assignmentFlag = true -- executed when the helpers are moved
				
				for (int i = 0; i < pblock2->Count(sm_point_node_list); i++){
					INode *sm_point_node;
					pblock2->GetValue(sm_point_node_list, t, sm_point_node, valid, i);
					Matrix3 aTM;
					
					if(sm_point_node!=NULL) {

						aTM = sm_point_node->GetNodeTM(t, &valid);
						Point3 curHlprNodePosW = aTM.GetTrans();
						os->obj->SetPoint(i, curHlprNodePosW*Inverse(splNodeTM));
					}
					else
						continue;
					}
				}
				os->obj->PointsWereChanged();

			}
#endif // NO_NURBS

	}
	else if (sid == SHAPE_CLASS_ID){ //only if the node is a spline && cid == Class_ID(SPLINESHAPE_CLASS_ID, 0)
//		splineShapeClassID = SPLINESHAPE_CLASS_ID

		if (cid == Class_ID(LINEARSHAPE_CLASS_ID,0)){
			return;
		}

	    SplineShape *sObj;
		sObj = (SplineShape *) os->obj;
		valid &= sObj->ChannelValidity(t,TOPO_CHAN_NUM);
		valid &= sObj->ChannelValidity(t,GEOM_CHAN_NUM);
        notASplineOrNURBS = 0;
		BezierShape *bShape;
		bShape = &sObj->shape;
		Spline3D* manipSpline;
		int sCount = bShape->SplineCount();
		if (sCount < 1) return;
		manipSpline = bShape->GetSpline(0);
		if (!manipSpline) return;
		nKnots = manipSpline->KnotCount();
		if (nKnots <2) return;

		int ct = pblock2->Count(sm_point_node_list); //should be = nKnots
		if(ct <= 0 || ct != nKnots)return;
//		if(ct <= 0) return;
		SplineIKControlModData *d  = new SplineIKControlModData(0);    
		mc.localData = d;

		int id;
		splNode = GetNodeFromModContext(d,id);
		if (splNode == NULL) {
			DbgAssert(0);
			return;
		}
		splNodeTM = splNode->GetObjectTM(t);

//orignial place
		
		if (assignmentFlag == false){  // enters loop only for the first time
			assignmentFlag = true;
			int ct = pblock2->Count(sm_point_node_list); //should be = nKnots
			for (int i = 0; i < ct; i++){
//					Interval ivalid;
					INode *sm_point_node;
					pblock2->GetValue(sm_point_node_list, t, sm_point_node, valid, i);

					if (sm_point_node == NULL) continue;

					Matrix3 xfm(1);
					Point3 knotPosition = manipSpline->GetKnotPoint(i) * splNodeTM;
				if (noHelperNearThisKnot(knotPosition)){
					xfm.SetTrans (knotPosition);  //setting helper position is done inside ModifyObject
					sm_point_node->SetNodeTM (0, xfm);
				}
			}
		}

		else { //assignmentFlag = true -- executed when the helpers are moved
				
			for (int i = 0; i < pblock2->Count(sm_point_node_list); i++){
//				Interval ivalid;
				INode *sm_point_node;
				pblock2->GetValue(sm_point_node_list, t, sm_point_node, valid, i);
				Matrix3 aTM;
				
				if(sm_point_node!=NULL) {
					aTM = sm_point_node->GetNodeTM(t, &valid);
					Point3 curHlprNodePosW = aTM.GetTrans();
					Point3 aTMScale;
					Quat aTMRotQuat;
					Matrix3 rot3;
					DecomposeMatrix(aTM, curHlprNodePosW, aTMRotQuat, aTMScale);
					rot3.SetRotate(aTMRotQuat);
					Point3 prevKnotPosL = manipSpline->GetKnotPoint(i);
					Point3 inRelVecPtL = manipSpline->GetRelInVec(i);  //get it before the knotPoint has been reset
					Point3 outRelVecPtL = manipSpline->GetRelOutVec(i);  //get it before the knotPoint has been reset
					Point3 inVecPtL = manipSpline->GetInVec(i);  //get it before the knotPoint has been reset
					Point3 outVecPtL = manipSpline->GetOutVec(i);  //get it before the knotPoint has been reset
					manipSpline->SetKnotPoint(i, curHlprNodePosW * Inverse(splNodeTM));  //need to convert curKnotPos to local space
					
					manipSpline->SetRelInVec(i,  (splNodeTM * rot3 * Inverse(splNodeTM)).VectorTransform(inRelVecPtL));
					manipSpline->SetRelOutVec(i,  (splNodeTM * rot3 * Inverse(splNodeTM)).VectorTransform(outRelVecPtL));
				}
				else
					continue;
			}

			manipSpline->ComputeBezPoints();
			manipSpline->InvalidateGeomCache();
			sObj->InvalidateGeomCache();
			bShape->UpdateSels();
//			DebugPrint("%f  %f  %f \n", kPosition_in_World.x, kPosition_in_World.y, kPosition_in_World.z);
//		}

		}

	}
	else {
		notASplineOrNURBS = 1;
		MessageBox(GetCOREInterface()->GetMAXHWnd(), "SplineIKControl modifier cannot be assigned to the selected non-shape object!", "Warning!", MB_ICONSTOP|MB_OK);				
		if (ip) ip->RedrawViews(t);
		return;
	}

		os->obj->SetChannelValidity(TOPO_CHAN_NUM, valid);
		os->obj->SetChannelValidity(GEOM_CHAN_NUM, valid);
		os->obj->SetChannelValidity(TEXMAP_CHAN_NUM, valid);
		os->obj->SetChannelValidity(MTL_CHAN_NUM, valid);
		os->obj->SetChannelValidity(SELECT_CHAN_NUM, valid);
		os->obj->SetChannelValidity(SUBSEL_TYPE_CHAN_NUM, valid);
		os->obj->SetChannelValidity(DISP_ATTRIB_CHAN_NUM, valid);

		os->obj->UpdateValidity(GEOM_CHAN_NUM,valid); // Have to do this to get it to evaluate
		os->obj->UpdateValidity(TOPO_CHAN_NUM,valid); // Have to do this to get it to evaluate
	
}

// ref enumerator to find a scene base object's node
class FindSceneObjNodeDEP : public DependentEnumProc 
{
public:		
	INode*			node;
	ReferenceTarget* dobj;
	ReferenceTarget* cur_obj;
	FindSceneObjNodeDEP(ReferenceTarget* obj) { node = NULL; dobj = NULL; cur_obj = obj; }
	int proc(ReferenceMaker* rmaker)
	{
		if (rmaker == cur_obj)
			return 0;
		// hunt for either a node or derived object
		SClass_ID sid = rmaker->SuperClassID();
		if (sid == BASENODE_CLASS_ID)
		{
			node = (INode*)rmaker;
			return 1;
		}
		else if (sid == GEN_DERIVOB_CLASS_ID ||  
				 sid == DERIVOB_CLASS_ID ||   
				 sid == WSM_DERIVOB_CLASS_ID ||
				 sid == MODAPP_CLASS_ID ||
				 sid == OBREF_MODAPP_CLASS_ID)    
		{
//			dobj = (ReferenceTarget*)rmaker;
//			return 1;
			return 0;
		}
		else 
			return DEP_ENUM_SKIP;
	}
};

static INode*
find_scene_obj_node(ReferenceTarget* obj)
{
	// given a scene base object or modifier, look for a referencing node via successive 
	// reference enumerations up through any intervening mod stack entries
	FindSceneObjNodeDEP fsno (obj);
	obj->EnumDependents(&fsno);
	while (fsno.dobj != NULL)
	{
		// found a mod stack, wind up through any derived objs
		fsno.cur_obj = fsno.dobj;
		fsno.dobj = NULL;
		fsno.cur_obj->EnumDependents(&fsno);
	}
	return fsno.node;
}



BOOL SplineIKControl::CreateHelpers(int knotCt){
	
	TimeValue t = GetCOREInterface()->GetTime();

	ModContextList mcList;
	INodeTab nodes;
	ObjectState os;

	if (ip==NULL){ // if ip != NULL, up in modify panel
		splNode = find_scene_obj_node((ReferenceTarget*)this); 
		if (!splNode) 
			return FALSE;
	}
	else{
		ip->GetModContexts(mcList,nodes);
		GetCOREInterface()->GetModContexts(mcList,nodes);
		assert(nodes.Count());
		splNode = nodes[0];
		if (splNode == NULL) return FALSE;
	}

		os = splNode->EvalWorldState(t);
	

		cid = os.obj->ClassID();
		sid = os.obj->SuperClassID();

	if (knotCt == 0) // this is for the case when CreateHelpers is called from maxscript or outside
						//without specifying any knotCt. We have to count the knots right here.

 //-----------------------------------------------------------------
 // I don't think I need to know anything about the curve here. All I need is to create
 // a certain(= nKnots) number of helpers, that's all.

	{

		if (cid == EDITABLE_SURF_CLASS_ID){
#ifndef NO_NURBS		
			NURBSSet getSet;
			BOOL okay = GetNURBSSet(os.obj, GetCOREInterface()->GetTime(), getSet, TRUE);
			if (okay) {
				int obCount = getSet.GetNumObjects();
				for(int j = 0; j < obCount; j++){
					if (getSet.GetNURBSObject(j)->GetType() == kNPointCurve || 
						getSet.GetNURBSObject(j)->GetType() == kNCVCurve){	
						knotCt = os.obj->NumPoints();
						break;
					}
				}
			}
#endif // NO_NURBS
		}
		else if (sid == SHAPE_CLASS_ID){

			SplineShape *sObj;
			sObj = (SplineShape *) os.obj;
			BezierShape *bShape;
			bShape = &sObj->shape;
			Spline3D* manipSpline;
			int sCount = bShape->SplineCount();
			if (sCount != 1) return FALSE;
			manipSpline = bShape->GetSpline(0);
			if (!manipSpline) return FALSE;
			knotCt = manipSpline->KnotCount();

		}

		
//-----------------------------------------------------------------

	}

	if (notASplineOrNURBS){
		return FALSE;
	}
	else if (cid == EDITABLE_SURF_CLASS_ID){ // it's a NURBS curve -- treat it differently
		int ct = pblock2->Count(sm_point_node_list);
		Object  *hObj;
		INode *ptHelperNode;
		DWORD hlprColor;

		if (knotCt > pblock2->Count(sm_point_node_list)){	// One or more knots have been added
			for (int i = 0; i < knotCt - ct; i++){ //loop through for the # of knots
				//create the i-th point helper
				hObj = (Object *)GetCOREInterface()->CreateInstance(HELPER_CLASS_ID, Class_ID(POINTHELP_CLASS_ID,0));
				ptHelperNode = GetCOREInterface()->CreateObjectNode (hObj);
				if(i == 0) hlprColor = ptHelperNode->GetWireColor();
				ptHelperNode->SetWireColor(hlprColor);


				// update the pblock that stores the point helper nodes
				theHold.Suspend();
				pblock2->Append(sm_point_node_list, 1, &ptHelperNode, 1);
				theHold.Resume();
			}
			assignmentFlag = false;

		}
		else if (knotCt < pblock2->Count(sm_point_node_list)){ // one or more knots have been deleted. Delete same number of helpers

			for (int i = 0; i < ct - knotCt; i++){ //loop through for the # of knots
				//create the i-th point helper
				hObj = (Object *)GetCOREInterface()->CreateInstance(HELPER_CLASS_ID, Class_ID(POINTHELP_CLASS_ID,0));	
				ptHelperNode = GetCOREInterface()->CreateObjectNode (hObj);
				if(i == 0) hlprColor = ptHelperNode->GetWireColor();

				ptHelperNode->SetWireColor(hlprColor);

				// update the pblock that stores the point helper nodes
				theHold.Suspend();
				pblock2->Append(sm_point_node_list, 1, &ptHelperNode, 1);
				theHold.Resume();
			}
			assignmentFlag = false;
		}

		
	}
	else if (sid == SHAPE_CLASS_ID){

		if (cid == Class_ID(LINEARSHAPE_CLASS_ID,0))
			return FALSE;
		int ct = pblock2->Count(sm_point_node_list);
		Object  *hObj;
		INode *ptHelperNode;
		DWORD hlprColor;

		if (knotCt == pblock2->Count(sm_point_node_list)){ // this condition should never be satisfied
			for (int i = 0; i < knotCt; i++){ //loop through for the # of knots
				hObj = (Object *)GetCOREInterface()->CreateInstance(HELPER_CLASS_ID, Class_ID(POINTHELP_CLASS_ID,0));
				ptHelperNode = GetCOREInterface()->CreateObjectNode (hObj);
				if(i == 0) hlprColor = ptHelperNode->GetWireColor();
				ptHelperNode->SetWireColor(hlprColor);

                // update the pblock that stores the point helper nodes
				theHold.Suspend();
				pblock2->Append(sm_point_node_list, 1, &ptHelperNode, 1);
				theHold.Resume();

			}
		}

		else if (knotCt > pblock2->Count(sm_point_node_list)){	// One or more knots have been added
																//  -- OR -- Some helpers are deleted

//			int response;
//			if (!helpCreationFlag)
//				response = MessageBox(GetCOREInterface()->GetMAXHWnd(), "You Have More Knots than Helpers! Want to Create Additional Helpers?", "Warning!", MB_ICONQUESTION|MB_YESNO);

//			if (helpCreationFlag){
//				|| response == IDYES){ 	// Add some of helpers
				for (int i = 0; i < knotCt - ct; i++){ //loop through for the # of knots
//					if (noHelperNearThisKnot(knotPosition)){
					
					//create the i-th point helper
						hObj = (Object *)GetCOREInterface()->CreateInstance(HELPER_CLASS_ID, Class_ID(POINTHELP_CLASS_ID,0));
						ptHelperNode = GetCOREInterface()->CreateObjectNode (hObj);
						if(i == 0) hlprColor = ptHelperNode->GetWireColor();
						ptHelperNode->SetWireColor(hlprColor);


					// update the pblock that stores the point helper nodes
						theHold.Suspend();
						pblock2->Append(sm_point_node_list, 1, &ptHelperNode, 1);
						theHold.Resume();
//					}
				}
//				helpCreationFlag = 0;
				assignmentFlag = false;
//			}

//			else {
//				assignmentFlag = true;
//			}

		}
		else if (knotCt < pblock2->Count(sm_point_node_list)){ // one or more knots have been deleted. Delete same number of helpers

			for (int i = 0; i < ct - knotCt; i++){ //loop through for the # of knots
//				if (noHelperNearThisKnot(knotPosition)){
					
					//create the i-th point helper
					hObj = (Object *)GetCOREInterface()->CreateInstance(HELPER_CLASS_ID, Class_ID(POINTHELP_CLASS_ID,0));	
					ptHelperNode = GetCOREInterface()->CreateObjectNode (hObj);
					if(i == 0) hlprColor = ptHelperNode->GetWireColor();
					ptHelperNode->SetWireColor(hlprColor);


					// update the pblock that stores the point helper nodes
					theHold.Suspend();
					pblock2->Append(sm_point_node_list, 1, &ptHelperNode, 1);
					theHold.Resume();
//				}
			}
			assignmentFlag = false;
		}

	}

//	helpCreationFlag = 0;
	pblock2->CallSets();

	if (hWnd != NULL) {
		EnableWindow(GetDlgItem(hWnd, IDC_SIK_CREATE_HELPERS_PICKBTN), FALSE);
		ICustButton* iBut1 = GetICustButton(GetDlgItem(hWnd,IDC_SIK_CREATE_HELPERS_PICKBTN));
		if (iBut1) {
			iBut1->SetCheck(FALSE);
			ReleaseICustButton(iBut1);
		}
	}

	RedrawListbox(t);

//	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
//	SetAFlag(A_MOD_BEING_EDITED);

	return TRUE;

}


void SplineIKControl::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;
	curMod = this;
//	Modifier::BeginEditParams(ip,flags,prev);
	SplineIKControlDesc.BeginEditParams(ip, this, flags, prev);


	ModContextList mcList;
	INodeTab nodes;
	if (ip) ip->GetModContexts(mcList,nodes);
	assert(nodes.Count());
	splNode = nodes[0];	
	splNodeTM = splNode->GetObjectTM(ip->GetTime());
	if (splNode == NULL) 
		return;

	// get the objectstate and form the spline
//	Object *sObj = splNode->GetObjectRef();
	ObjectState os = splNode->EvalWorldState(ip->GetTime());
//	SplineShape *sObj;
//	sObj = (SplineShape *) os.obj;
	Object *sObj = os.obj;

	if (sid == SHAPE_CLASS_ID || cid == EDITABLE_SURF_CLASS_ID){
		if (cid == Class_ID(LINEARSHAPE_CLASS_ID,0))
			notASplineOrNURBS =  1;
		else
			notASplineOrNURBS =  0;
	}
	else
		notASplineOrNURBS =  1;
	

	SplineIKControl_param_blk.SetUserDlgProc(new SplineIKControlDlgProc(this));

	IParamMap2* pmap = pblock2->GetMap();
	if (pmap) hWnd = pmap->GetHWnd();


	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);
	

	RedrawListbox(GetCOREInterface()->GetTime());

}

void SplineIKControl::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	this->ip = NULL; 
	curMod = NULL;


	IParamMap2* pmap = pblock2->GetMap();
	
	if (pmap != NULL)
	{
		if (next && next->ClassID() == ClassID() && ((SplineIKControl*)next)->pblock2){
			pmap->SetParamBlock(((SplineIKControl*)next)->pblock2);
			pmap->UpdateUI(GetCOREInterface()->GetTime());
		}
		else 
			SplineIKControlDesc.EndEditParams(ip, this, flags | END_EDIT_REMOVEUI, next);	
	}

	ClearAFlag(A_MOD_BEING_EDITED);
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);


//	Modifier::EndEditParams(ip,flags,next);
	SplineIKControlDesc.EndEditParams(ip, this, flags, next);
	hWnd = NULL;

}


int SplineIKControl::noHelperNearThisKnot(Point3 kPos){
	int ct = pblock2->Count(sm_point_node_list);
	double delta = 0.001;
	Point3 helperPos;
	int answer = 1;
	Matrix3 aTM;
	Interval ivalid;
	
	for (int i = 0; i < ct; i++){
		INode *sm_point_node;
		pblock2->GetValue(sm_point_node_list, GetCOREInterface()->GetTime(), sm_point_node, FOREVER, i);

		if (sm_point_node == NULL) continue;

		aTM = sm_point_node->GetNodeTM(GetCOREInterface()->GetTime(), &ivalid);
		helperPos = aTM.GetTrans();
		if (	fabs(kPos.x - helperPos.x) < delta &&
				fabs(kPos.y - helperPos.y) < delta &&
				fabs(kPos.z - helperPos.z) < delta ){
			answer = 0;
			break;
		}
		
	}

	if (answer){
		return 1;
	}
	else
		return 0;
}




 Interval SplineIKControl::GetValidity(TimeValue t)
	{
	float f;
	Interval valid = FOREVER;
	
		BOOL b;	
	// Start our interval at forever...
	// Intersect each parameters interval to narrow it down.
	for (int i = 0; i < pblock2->Count(ISplineIKControl::sm_point_node_list); i= i + 1){
		INode* locNode;
		pblock2->GetValue(ISplineIKControl::sm_point_node_list,t,locNode,valid, i);	
	}
	pblock2->GetValue(ISplineIKControl::sm_helpersize,t,f,valid);
	pblock2->GetValue(ISplineIKControl::sm_helper_centermarker,t,b,valid);
	pblock2->GetValue(ISplineIKControl::sm_helper_axistripod,t,b,valid);	
	pblock2->GetValue(ISplineIKControl::sm_helper_cross,t,b,valid);
	pblock2->GetValue(ISplineIKControl::sm_helper_box,t,b,valid);
	pblock2->GetValue(ISplineIKControl::sm_helper_screensize,t,b,valid);
	pblock2->GetValue(ISplineIKControl::sm_helper_drawontop,t,b,valid);
	pblock2->GetValue(ISplineIKControl::sm_link_types,t,b,valid);	

	return valid;
	}




RefTargetHandle SplineIKControl::Clone(RemapDir& remap)
{
	SplineIKControl* newmod = new SplineIKControl();	
	//TODO: Add the cloning code here
	newmod->ReplaceReference(SPLINEIKCONTROL_PBLOCK_REF,pblock2->Clone(remap));
	BaseClone(this, newmod, remap);
	return(newmod);
}


RefTargetHandle SplineIKControl::GetReference(int i)
	{
		switch (i)
		{
			case SPLINEIKCONTROL_PBLOCK_REF:
				return pblock2;
			break;
		}
		return NULL;
	}

void SplineIKControl::SetReference(int i, RefTargetHandle rtarg)
	{
		switch (i)
		{
			case SPLINEIKCONTROL_PBLOCK_REF:
				pblock2 = (IParamBlock2*)rtarg; 
			break;
		}
	}

RefResult SplineIKControl::NotifyRefChanged (Interval iv, RefTargetHandle hTarg, PartID& partID, RefMessage msg) 
	{
	int top = mNotifyStack.GetSize();
	if (--top >= 0) {
		if (mNotifyStack[top]) {
			return REF_STOP;
		} else {
			mNotifyStack.Set(top);
		}
	}

	switch (msg) {
		case REFMSG_CHANGE:
		{
			if (hTarg == pblock2){
				ParamID changing_param = pblock2->LastNotifyParamID();
				SplineIKControl_param_blk.InvalidateUI(changing_param);
//				NotifyDependents(iv, partID, msg);
			}
		}
		break;

		case REFMSG_OBJECT_CACHE_DUMPED:
			
			return REF_STOP;

		break;

	}
	return REF_SUCCEED;
}



void SplineIKControl::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc)
{
	if (!mc->localData) return;
	SplineIKControlModData *smd = (SplineIKControlModData *) mc->localData;
	smd->ivalid = NEVER;

}



//From Object
//BOOL SplineIKControl::HasUVW() 
//{ 
	//TODO: Return whether the object has UVW coordinates or not
//	return TRUE; 
//}

//void SplineIKControl::SetGenUVW(BOOL sw) 
//{  
//	if (sw==HasUVW()) return;
	//TODO: Set the plugin internal value to sw				
//}

IOResult SplineIKControl::Load(ILoad *iload)
{
	Modifier::Load(iload);
	assignmentFlag = true;
	return IO_OK;

}

IOResult SplineIKControl::Save(ISave *isave)
{
	//TODO: Add code to allow plugin to save its data
	
	return IO_OK;
}

void SplineIKControl::InvalidateUI()
{
	SplineIKControl_param_blk.InvalidateUI(pblock2->LastNotifyParamID());
}


int SplineIKControl::GetHelperCount(){
	return pblock2->Count(sm_point_node_list);
} 

int SplineIKControl::GetKnotCount(){
	return nKnots;
} 

float SplineIKControl::GetHelperSize(){ // All Helpers Have Same size for now
	float hsize;
	pblock2->GetValue(sm_helpersize, GetCOREInterface()->GetTime(), hsize, FOREVER);
	return hsize;
} 

BOOL SplineIKControl::SetHelperSize(float hsize){ // All Helpers Have Same size for now
	pblock2->SetValue(sm_helpersize, GetCOREInterface()->GetTime(), hsize);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	return TRUE;;
} 




BOOL SplineIKControl::LinkToRoot(){
	int ct = pblock2->Count(sm_point_node_list);
	if (mSetLinkTypes) {
		if (ct <= 0) return FALSE;

		TimeValue t = GetCOREInterface()->GetTime();
		for (int i = 0; i < ct; i++){ 
			if (i > 0 && pblock2->GetINode(sm_point_node_list, t, 0) && pblock2->GetINode(sm_point_node_list, t, i))
				pblock2->GetINode(sm_point_node_list, t, 0)->AttachChild(pblock2->GetINode(sm_point_node_list, t, i), 1);
		}
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE); // so that the change is updated

		return TRUE;
	} else {
		pblock2->SetValue(sm_link_types, 0, 1);
		return ct > 0;
	}
} 


BOOL SplineIKControl::LinkInHierarchy(){
	int ct = pblock2->Count(sm_point_node_list);
	if (mSetLinkTypes){
		if (ct <= 0) return FALSE;

		TimeValue t = GetCOREInterface()->GetTime();
		for (int i = 0; i < ct; i++){ 
			if (i > 0 && pblock2->GetINode(sm_point_node_list, t, i-1) && pblock2->GetINode(sm_point_node_list, t, i)) 
				pblock2->GetINode(sm_point_node_list, t, i-1)->AttachChild(pblock2->GetINode(sm_point_node_list, t, i), 1);
							
		}
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE); // so that the change is updated

		return TRUE;
	} else {
		pblock2->SetValue(sm_link_types, 0, 0);
		return ct > 0;
	}
}

BOOL SplineIKControl::UnLink(){
	int ct = pblock2->Count(sm_point_node_list);
	if (mSetLinkTypes) {
		if (ct <= 0) return FALSE;

		TimeValue t = GetCOREInterface()->GetTime();
		for (int i = 1; i < ct; i++){ 
			if (pblock2->GetINode(sm_point_node_list, t, i))
				pblock2->GetINode(sm_point_node_list, t, i)->Detach(t, 1);
							
		}
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE); // so that the change is updated

		return TRUE;
	} else {
		pblock2->SetValue(sm_link_types, 0, 2);
		return ct > 0;
	}
}
