/**********************************************************************
 *<
	FILE: reactor.cpp

	DESCRIPTION: A Controller plugin that reacts to changes in other controllers

	CREATED BY: Adam Felt

	HISTORY: 

 *>	Copyright (c) 1998, All Rights Reserved.
***********************************************************************/
//-----------------------------------------------------------------------------
#include "reactor.h"
#include "iparamm.h"
#include "ReactAPI.h"
#include "macrorec.h"


//#include "..\..\..\Include\Maxscrpt\maxscrpt.h"
//-----------------------------------------------------------------------
float Distance(Point3 p1, Point3 p2)
{
	p1 = p2-p1;
	return (float)sqrt((p1.x*p1.x)+(p1.y*p1.y)+(p1.z*p1.z));
}

//-----------------------------------------------------------------------

#define REACTOR_DONT_SHOW_CREATE_MSG    (1<<1)  //don't display the Create reaction warning message
#define REACTOR_DONT_CREATE_SIMILAR     (1<<2)  //don't create a reaction if it has the same value as an existing reaction
#define REACTOR_BLOCK_CURVE_UPDATE      (1<<3)  //don't update the Curves
#define REACTOR_BLOCK_REACTION_DELETE   (1<<4)  //only delete one reaction if multiple keys are selected

#define REACTORDLG_LIST_CHANGED_SIZE		(2<<1)  //refresh the listbox and everything depending on selection
#define REACTORDLG_LIST_SELECTION_CHANGED   (2<<2)  //update the fields depending on the list selection
#define REACTORDLG_REACTTO_NAME_CHANGED		(2<<3)  
//#define REACTORDLG_REACTION_NAME_CHANGED	(2<<4)

#define REFMSG_REACTOR_SELECTION_CHANGED 0x00022000
#define REFMSG_REACTION_COUNT_CHANGED	 0x00023000
#define REFMSG_REACTTO_OBJ_NAME_CHANGED	 0x00024000
#define REFMSG_USE_CURVE_CHANGED		 0x00025000
#define REFMSG_REACTION_NAME_CHANGED	 0x00026000

class ReactorDlg;
class Reactor;


//Storage class for the animatable right-click style menu
//*******************************************************
class AnimEntry
{ 
public:
	TCHAR*		pname;			// param name
	int			level;			// used during pop-up building, 0->same, 1->new level, -1->back level
	ReferenceTarget* root;		// display root
	ReferenceTarget* parent;	// anim/subanim num pair
	int			subnum; 

	AnimEntry() : 
		pname(NULL), parent(NULL), root(NULL) { }

	AnimEntry(TCHAR* n, int l, ReferenceTarget* p, int s, ReferenceTarget* r=NULL) :
		pname(n ? save_string(n) : n), level(l), parent(p), subnum(s), root(r) { }

	AnimEntry& operator=(const AnimEntry& from)
	{
		Clear();
		//pname = from.pname;  // ? save_string(from.pname) : NULL
		pname = from.pname ? save_string(from.pname) : NULL;
		level = from.level;
		root = from.root;
		parent = from.parent;
		subnum = from.subnum;
		return *this;
	}

	void Clear()
	{
		if (pname) free(pname);
		pname = NULL;
		root = parent = NULL;
	}

	Animatable* Anim() { return parent ? parent->SubAnim(subnum) : NULL; }
};


/* -----  viewport right-click menu ----------------- */

class AnimPopupMenu
{
private:
	Reactor*	reactor;
	INode*		node;				// source node & param...
	AnimEntry	entry;
	Tab<AnimEntry> AnimEntries;		// currently-showing menu data
	Tab<HMENU>  menus;

	void		add_hmenu_items(Tab<AnimEntry>& wparams, HMENU menu, int& i);
	bool		wireable(Animatable* parent, int subnum);
	void		build_params(Tab<AnimEntry>& wparams, ReferenceTarget* root);
	bool		add_subanim_params(Tab<AnimEntry>& wparams, TCHAR* name, ReferenceTarget* parent, int level, ReferenceTarget* root);
	AnimEntry*	find_param(Tab<AnimEntry>& params, Animatable* anim, Animatable* parent, int subNum);
	bool		Filter(Animatable* anim, bool includeChildren=false);

public:
				AnimPopupMenu() { }
	void		DoPopupMenu(Reactor *cont, INode *node);
	Control*	AssignDefaultController(Animatable* parent, int subnum);
};

static AnimPopupMenu theAnimPopupMenu;


//Storage class for the track being referenced
//*********************************************
class VarRef {
public:
	INode* client;
	int	refCt;
	int subnum;

	VarRef()	{ client = NULL; refCt = 0; }
	VarRef(INode* c)	{ client = c; refCt = 1; }
	
	VarRef& operator=(const VarRef& from){
		client = ((VarRef)from).client;
		refCt = ((VarRef)from).refCt;
		subnum = ((VarRef)from).subnum;
		return *this;
	}

};

// Reaction variables
//*****************************************
class SVar {
public:
	TSTR	name;
	int		subNum;
	int		regNum;	// register number variable is assigned to
	int		refID;	// < 0 means constant
	float	influence, multiplier; 
	float strength;
	float falloff;

	//The type used here is the same as the controller type
	float	fstate;		//reaction state if it's a float
	Quat	qstate;		//reaction state if it's a quat
	Point3	pstate;		//reaction state if it's a point3
	
	//The type used here is the same as the client track
	Point3	pvalue;		//current value if it is a point3
	float	fvalue;		//current value if it's a float
	Quat	qvalue;		//current value if it's a quat

	SVar& operator=(const SVar& from){
		name = from.name;
		subNum = from.subNum;
		regNum = from.regNum;
		refID = from.refID;
		multiplier = from.multiplier;
		influence = from.influence;
		strength = from.strength;
		falloff = from.falloff;
		fstate = from.fstate;
		qstate = from.qstate;
		pstate = from.pstate;
		pvalue = from.pvalue;
		fvalue = from.fvalue;
		qvalue = from.qvalue;
		return *this;
	}
};
	
MakeTab(SVar);



//Pick Mode Stuff
//******************************************
class PickReactToMode : 
		public PickModeCallback,
		public PickNodeCallback 
{
	public:		
		Reactor *cont;
		
		PickReactToMode(Reactor *c) {cont = c;}

		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);
		BOOL PickAnimatable(Animatable* anim);
		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);
		BOOL RightClick(IObjParam *ip,ViewExp *vpt)	{return TRUE;}
		BOOL Filter(INode *node);		
		PickNodeCallback *GetFilter() {return this;}
		BOOL FilterAnimatable(Animatable *anim);
		BOOL AllowMultiSelect(){ return false; }
};


//Enumeration Proc
//Used to determine the proper reference object....
//******************************************
class MyEnumProc : public DependentEnumProc 
	{
      public :
      virtual int proc(ReferenceMaker *rmaker); 
	  INodeTab nodes;
	};


int MyEnumProc::proc(ReferenceMaker *rmaker) 
{ 
		nodes.Append(1, (INode**)&rmaker);
		return 0;
}

//Reference Maker class to handle the Curve Control References
//****************************************************************
/*
class DummyRefMaker : public ReferenceMaker
{
	void DeleteThis() {}
	virtual void* GetInterface(ULONG id);
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,RefMessage message){return REF_DONTCARE;}
};
static DummyRefMaker theDummyRefMaker;
*/

class Reactor : public IReactor, public ResourceMakerCallback {

	public:

		int			type, selected, count, rtype;
		int			isCurveControlled;
		BOOL		editing;  //editing the reaction state
		BOOL		isBiped;
		SVarTab		reaction;
		Interval	ivalid;
		Interval	range;
//		HWND		hParams;
		NameMaker*	nmaker;
		ICurveCtl*	iCCtrl;
		UINT		flags;

		VarRef vrefs;
		Point3 curpval;
		Point3 upVector;
		float curfval;
		Quat curqval;
		BOOL blockGetNodeName; // RB 3/23/99: See imp of getNodeName()

		ReactorActionCB<Reactor >	*reactorActionCB;		// Actions handler 		
		
		PickReactToMode *pickReactToMode;

		virtual int Elems(){return 0;}
		IObjParam *ip;
		ReactorDlg *dlg;	
		
		//Reactor(int t, Reactor &ctrl);
		Reactor();
		Reactor(int t, BOOL loading);
		Reactor& operator=(const Reactor& from);
		~Reactor();
		void Init();

		BOOL	assignReactObj(INode* client, int subnum);
		void	reactTo(ReferenceTarget* anim, TimeValue t = GetCOREInterface()->GetTime());
		void	updReactionCt(int val);
		SVar*	CreateReactionAndReturn(BOOL setDefaults = TRUE, TCHAR *buf=NULL, TimeValue t = GetCOREInterface()->GetTime());
		BOOL	CreateReaction(TCHAR *buf=NULL, TimeValue t = GetCOREInterface()->GetTime());
		BOOL	DeleteReaction(int i=-1);
		int		getReactionCount() { return reaction.Count(); }
		void	deleteAllVars();
		int		getSelected() {return selected;}
		void	setSelected(int i); 
		void	setReactionType(int i){ rtype = i; }
		int 	getReactionType(){ return rtype; }
		int 	getType(){ return type; }
		TCHAR*	getReactionName(int i);
		void	setReactionName(int i, TSTR name);
		void*	getReactionValue(int i);
		BOOL	setReactionValue(int i=-1, TimeValue t=NULL);
		BOOL	setReactionValue(int i, float val);
		BOOL	setReactionValue(int i, Point3 val);
		BOOL	setReactionValue(int i, Quat val);
		float	getCurFloatValue(TimeValue t);
		Point3	getCurPoint3Value(TimeValue t);
		ScaleValue	getCurScaleValue(TimeValue t);
		Quat	getCurQuatValue(TimeValue t);
		BOOL	setInfluence(int num, float inf);
		float	getInfluence(int num);
		void	setMinInfluence(int x=-1);
		void	setMaxInfluence(int x=-1);
		BOOL	setStrength(int num, float inf);
		float	getStrength(int num);
		BOOL	setFalloff(int num, float inf);
		float	getFalloff(int num);
		void	setEditReactionMode(BOOL edit);
		BOOL	getEditReactionMode(){return editing;}
		void*	getState(int num);
		BOOL	setState(int num=-1, TimeValue t=NULL);
		BOOL	setState(int num, float val);
		BOOL	setState(int num, Point3 val);
		BOOL	setState(int num, Quat val);
		void	getNodeName(ReferenceTarget *client, TSTR &name);
		Point3	getUpVector() {return upVector;}
		void	setUpVector(Point3 up) {upVector = up;}
		BOOL	useCurve(){return isCurveControlled;}
		void	useCurve(BOOL use) {isCurveControlled = use; NotifyDependents(FOREVER,PART_ALL,REFMSG_USE_CURVE_CHANGED);}
		ICurveCtl*	getCurveControl() {return iCCtrl;}
		void	SortReactions();

		void	Update(TimeValue t);
		void	ComputeMultiplier(TimeValue t);
		void	GetAbsoluteControlValue(INode *node,TimeValue t,void *pt,Interval &iv);
		BOOL	ChangeParents(TimeValue t,const Matrix3& oldP,const Matrix3& newP,const Matrix3& tm);
		void	isABiped(BOOL bip) { isBiped = bip; }
		void	ShowCurveControl();
		void	BuildCurves();
		void	RebuildFloatCurves();
		void	UpdateCurves(bool doZoom = true);
		void	GetCurveValue(TimeValue t, float* val, Interval &valid);

		FPValue fpGetReactionValue(int index);
		FPValue fpGetState(int index);


		// Animatable methods		
		void DeleteThis() {delete this;}		
		int IsKeyable() {return 0;}		
		BOOL IsAnimated() {if (reaction.Count() > 1) return true; else return false;}
		Interval GetTimeRange(DWORD flags) { return range; } 
		void EditTimeRange(Interval range,DWORD flags);
		void MapKeys(TimeMap *map,DWORD flags);

		void HoldTrack();
		void HoldAll();
		void HoldParams();
		void HoldRange();

		int NumSubs();
		BOOL AssignController(Animatable *control,int subAnim) {return false;}
		Animatable* SubAnim(int i){return NULL;}
		TSTR SubAnimName(int i){ return "";}

		int NumRefs();
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
		
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev );
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next );

		// Animatable's Schematic View methods
		SvGraphNodeReference SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags);
		TSTR SvGetRelTip(IGraphObjectManager *gom, IGraphNode *gNodeTarger, int id, IGraphNode *gNodeMaker);
		bool SvHandleRelDoubleClick(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker);

		void EditTrackParams(
			TimeValue t,
			ParamDimensionBase *dim,
			TCHAR *pname,
			HWND hParent,
			IObjParam *ip,
			DWORD flags);
		int TrackParamsType() {return TRACKPARAMS_WHOLE;}

		// Reference methods
		RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage);
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		// Control methods				
		void Copy(Control *from);
		BOOL IsLeaf() {return TRUE;}

		//These three default implementation are shared by Position, Point3 and Scale controllers
		void SetValue(TimeValue t, void *val, int commit, GetSetMethod method);
		void CommitValue(TimeValue t);
		void RestoreValue(TimeValue t);		

		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method){} //overwritten by everyone


		//From ResourceMakerCallback
		void ResetCallback(int curvenum, ICurveCtl *pCCtl);
		void NewCurveCreatedCallback(int curvenum, ICurveCtl *pCCtl);
		void* GetInterface(ULONG id)
		{
			if(id == I_RESMAKER_INTERFACE)
				return (void *) (ResourceMakerCallback *) this;
			else
				return (void *) Control::GetInterface(id);
		}

		//Function Publishing method (Mixin Interface)
		//******************************
		BaseInterface* GetInterface(Interface_ID id) 
		{ 
			if (id == REACTOR_INTERFACE) 
				return (IReactor*)this; 
			else 
				return FPMixinInterface::GetInterface(id);
		} 
		//******************************

};

//static Reactor theReactor;

//-----------------------------------------------------------------------------

class PositionReactor : public Reactor {
	public:
		int Elems() {return 3;}

		//PositionReactor(PositionReactor &ctrl) : Reactor(REACTORPOS, ctrl) {}
		PositionReactor(BOOL loading) : Reactor(REACTORPOS, loading) {}
		~PositionReactor() {}

		Class_ID ClassID() { return REACTORPOS_CLASS_ID; }  
		SClass_ID SuperClassID() { return CTRL_POSITION_CLASS_ID; } 
		void GetClassName(TSTR& s) {s = GetString(IDS_AF_REACTORPOS);}

		// Control methods
		RefTargetHandle Clone(RemapDir& remap);
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method); 
	};


class PositionReactorClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new PositionReactor(loading); }
	const TCHAR *	ClassName() { return GetString(IDS_AF_REACTORPOS); }
	SClass_ID		SuperClassID() { return CTRL_POSITION_CLASS_ID; }
	Class_ID		ClassID() { return REACTORPOS_CLASS_ID; }
	const TCHAR* 	Category() { return _T("");  }

	const TCHAR*	InternalName() { return _T("PositionReactor"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }					// returns owning module handle
	};

static PositionReactorClassDesc positionReactorCD;
ClassDesc* GetPositionReactorDesc() {return &positionReactorCD;}


//Function Publishing descriptor for Mixin interface
//*****************************************************
static FPInterfaceDesc reactor_interface(
    REACTOR_INTERFACE, _T("reactions"), 0, &positionReactorCD, FP_MIXIN,
		IReactor::react_to, _T("reactTo"), 0, TYPE_VOID, 0, 1,
			_T("object"), 0, TYPE_REFTARG,
		IReactor::create_reaction,  _T("create"), 0, TYPE_BOOL,   0, 1,
			_T("name"), 0, TYPE_STRING, f_keyArgDefault, _T("scriptCreated"),			//uses keyword arguments
		IReactor::delete_reaction, _T("delete"), 0, TYPE_BOOL, 0, 1,
			_T("index"), 0, TYPE_INDEX,
		IReactor::get_reaction_count, _T("getCount"), 0, TYPE_INT, 0, 0,
		IReactor::select_reaction, _T("select"), 0, TYPE_VOID, 0, 1,
			_T("index"), 0, TYPE_INDEX,
		IReactor::get_selected_reaction, _T("getSelected"), 0, TYPE_INDEX, 0, 0,

		IReactor::set_reaction_state, _T("setStateToCurrent"), 0, TYPE_BOOL, 0, 1,
			_T("index"), 0, TYPE_INDEX,
		IReactor::set_float_reaction_state, _T("setFloatState"), 0, TYPE_BOOL, 0, 2,
			_T("index"), 0, TYPE_INDEX,
			_T("state"), 0, TYPE_FLOAT,
		IReactor::set_p3_reaction_state, _T("setVectorState"), 0, TYPE_BOOL, 0, 2,
			_T("index"), 0, TYPE_INDEX,
			_T("state"), 0, TYPE_POINT3,
		IReactor::set_quat_reaction_state, _T("setRotationState"), 0, TYPE_BOOL, 0, 2,
			_T("index"), 0, TYPE_INDEX,
			_T("state"), 0, TYPE_QUAT,

		IReactor::set_reaction_value, _T("setValueToCurrent"), 0, TYPE_BOOL, 0, 1,
			_T("index"), 0, TYPE_INDEX,
		IReactor::set_float_reaction_value, _T("setValueAsFloat"), 0, TYPE_BOOL, 0, 2,
			_T("index"), 0, TYPE_INDEX,
			_T("value"), 0, TYPE_FLOAT,
		IReactor::set_p3_reaction_value, _T("setValueAsVector"), 0, TYPE_BOOL, 0, 2,
			_T("index"), 0, TYPE_INDEX,
			_T("value"), 0, TYPE_POINT3,
		IReactor::set_quat_reaction_value, _T("setValueAsQuat"), 0, TYPE_BOOL, 0, 2,
			_T("index"), 0, TYPE_INDEX,
			_T("value"), 0, TYPE_QUAT,

		IReactor::set_reaction_influence, _T("setInfluence"), 0, TYPE_BOOL, 0, 2,
			_T("index"), 0, TYPE_INDEX,
			_T("influence"), 0, TYPE_FLOAT,
		IReactor::set_reaction_strength, _T("setStrength"), 0, TYPE_BOOL, 0, 2,
			_T("index"), 0, TYPE_INDEX,
			_T("strength"), 0, TYPE_FLOAT,
		IReactor::set_reaction_falloff, _T("setFalloff"), 0, TYPE_BOOL, 0, 2,
			_T("index"), 0, TYPE_INDEX,
			_T("influence"), 0, TYPE_FLOAT,
		IReactor::set_reaction_name, _T("setName"), 0, TYPE_VOID, 0, 2,
			_T("index"), 0, TYPE_INDEX,	
			_T("name"), 0, TYPE_STRING,

		IReactor::get_reaction_name, _T("getName"), 0, TYPE_STRING, 0, 1,
			_T("index"), 0, TYPE_INDEX,	
		IReactor::get_reaction_influence, _T("getInfluence"), 0, TYPE_FLOAT, 0, 1,
			_T("index"), 0, TYPE_INDEX,	
		IReactor::get_reaction_strength, _T("getStrength"), 0, TYPE_FLOAT, 0, 1,
			_T("index"), 0, TYPE_INDEX,	
		IReactor::get_reaction_falloff, _T("getFalloff"), 0, TYPE_FLOAT, 0, 1,
			_T("index"), 0, TYPE_INDEX,

		IReactor::get_reaction_type, _T("getType"), 0, TYPE_ENUM, IReactor::reaction_type, 0, 0, 
		
		IReactor::get_reaction_state, _T("getState"), 0, TYPE_FPVALUE_BV, 0, 1,
			_T("index"), 0, TYPE_INDEX,
		IReactor::get_reaction_value, _T("getValue"), 0, TYPE_FPVALUE_BV, 0, 1,
			_T("index"), 0, TYPE_INDEX,
		//IReactor::get_curve, _T("getCurveControl"), 0, TYPE_REFTARG, 0, 0,
		
		properties,
		IReactor::get_editing_state, IReactor::set_editing_state, _T("editStateMode"), 0, TYPE_BOOL,
		IReactor::get_use_curve, IReactor::set_use_curve, _T("useCurve"), 0, TYPE_BOOL,
		
		enums,
		IReactor::reaction_type, 4,
				_T("floatReaction"),		FLOAT_VAR,
				_T("positionalReaction"),	VECTOR_VAR,
				_T("rotationalReaction"),	QUAT_VAR,
				_T("scaleReaction"),		SCALE_VAR,
      end
      );

//  Get Descriptor method for Mixin Interface
//  *****************************************
FPInterfaceDesc* IReactor::GetDesc()
  {
     return &reactor_interface;
  }

// End of Function Publishing Code

//---------------------------------------------------------------------------

static int CompareReactions( const void *elem1, const void *elem2 ) {
	float a = ((SVar *)elem1)->fvalue;
	float b = ((SVar *)elem2)->fvalue;
	return (a<b)?-1:(a==b?0:1);
}


class Point3Reactor : public Reactor {
	public:
		int Elems() {return 3;}

		//Point3Reactor(Point3Reactor &ctrl) : Reactor(REACTORP3, ctrl) {}
		Point3Reactor(BOOL loading) : Reactor(REACTORP3, loading) {}
		~Point3Reactor() {}

		Class_ID ClassID() { return REACTORP3_CLASS_ID; }  
		SClass_ID SuperClassID() { return CTRL_POINT3_CLASS_ID; } 
		void GetClassName(TSTR& s) {s = GetString(IDS_AF_REACTORP3);}

		// Control methods
		RefTargetHandle Clone(RemapDir& remap);
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method);
	};

static bool point3ReactorInterfaceAdded = false;
class Point3ReactorClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) 
					{ 
						if (!point3ReactorInterfaceAdded)
						{
							AddInterface(&reactor_interface);
							point3ReactorInterfaceAdded = true;
						}
						return new Point3Reactor(loading); 
					}
 
	const TCHAR *	ClassName() { return GetString(IDS_AF_REACTORP3); }
	SClass_ID		SuperClassID() { return CTRL_POINT3_CLASS_ID; }
	Class_ID		ClassID() { return REACTORP3_CLASS_ID; }
	const TCHAR* 	Category() { return _T("");  }

	const TCHAR*	InternalName() { return _T("Point3Reactor"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};
static Point3ReactorClassDesc point3ReactorCD;
ClassDesc* GetPoint3ReactorDesc() {return &point3ReactorCD;}


//-----------------------------------------------------------------------------


class ScaleReactor : public Reactor {
	public:
		int Elems() {return 3;}

		//ScaleReactor(ScaleReactor &ctrl) : Reactor(REACTORSCALE, ctrl) {}
		ScaleReactor(BOOL loading) : Reactor(REACTORSCALE, loading) {}
		~ScaleReactor() {}

		Class_ID ClassID() { return REACTORSCALE_CLASS_ID; }  
		SClass_ID SuperClassID() { return CTRL_SCALE_CLASS_ID; } 
		void GetClassName(TSTR& s) {s = GetString(IDS_AF_REACTORSCALE);}

		// Control methods
		RefTargetHandle Clone(RemapDir& remap);
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method);
		void SetValue(TimeValue t, void *val, int commit, GetSetMethod method);		
	};

static bool scaleReactorInterfaceAdded = false;
class ScaleReactorClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading)
					{ 
						if (!scaleReactorInterfaceAdded)
						{
							AddInterface(&reactor_interface);
							scaleReactorInterfaceAdded = true;
						}
						return new ScaleReactor(loading); 
					}
	const TCHAR *	ClassName() { return GetString(IDS_AF_REACTORSCALE); }
	SClass_ID		SuperClassID() { return CTRL_SCALE_CLASS_ID; }
	Class_ID		ClassID() { return REACTORSCALE_CLASS_ID;}
	const TCHAR* 	Category() { return _T("");  }

	const TCHAR*	InternalName() { return _T("ScaleReactor"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
	};

static ScaleReactorClassDesc scaleReactorCD;
ClassDesc* GetScaleReactorDesc() {return &scaleReactorCD;}

//-------------------------------------------------------------------

class RotationReactor : public Reactor {
	public:
		int Elems() {return 3;}

		//RotationReactor(RotationReactor &ctrl) : Reactor(REACTORROT, ctrl) {}
		RotationReactor(BOOL loading) : Reactor(REACTORROT, loading) {}
		~RotationReactor() {}

		Class_ID ClassID() { return REACTORROT_CLASS_ID; }  
		SClass_ID SuperClassID() { return CTRL_ROTATION_CLASS_ID; } 
		void GetClassName(TSTR& s) {s = GetString(IDS_AF_REACTORROT);}

		// Control methods
		RefTargetHandle Clone(RemapDir& remap);
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method);
		void SetValue(TimeValue t, void *val, int commit, GetSetMethod method);		
		void CommitValue(TimeValue t);
		void RestoreValue(TimeValue t);
	};

static bool rotationReactorInterfaceAdded = false;
class RotationReactorClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 3; }
	void *			Create(BOOL loading) 
					{ 
						if (!rotationReactorInterfaceAdded)
						{
							AddInterface(&reactor_interface);
							rotationReactorInterfaceAdded = true;
						}
						return new RotationReactor(loading); 
					}
	const TCHAR *	ClassName() { return GetString(IDS_AF_REACTORROT); }
	SClass_ID		SuperClassID() { return CTRL_ROTATION_CLASS_ID; }
	Class_ID		ClassID() { return REACTORROT_CLASS_ID; }
	const TCHAR* 	Category() { return _T("");  }

	const TCHAR*	InternalName() { return _T("RotationReactor"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }					// returns owning module handle
};	

static RotationReactorClassDesc rotationReactorCD;
ClassDesc* GetRotationReactorDesc() {return &rotationReactorCD;}

//-----------------------------------------------------------------------------


class FloatReactor : public Reactor {
	public:
		int Elems() {return 1;}
		
		//FloatReactor(FloatReactor &ctrl) : Reactor(REACTORFLOAT, ctrl) {}
		FloatReactor(BOOL loading) : Reactor(REACTORFLOAT, loading) {}
		~FloatReactor() {}

		Class_ID ClassID() { return REACTORFLOAT_CLASS_ID; }  
		SClass_ID SuperClassID() { return CTRL_FLOAT_CLASS_ID; } 
		void GetClassName(TSTR& s) {s = GetString(IDS_AF_REACTORFLOAT);}

		// Control methods
		RefTargetHandle Clone(RemapDir& remap);
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method);
		void SetValue(TimeValue t, void *val, int commit, GetSetMethod method){}		
		void CommitValue(TimeValue t){}
		void RestoreValue(TimeValue t){}		
	};

static bool floatReactorInterfaceAdded = false;
class FloatReactorClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) 
					{ 
						if (!floatReactorInterfaceAdded)
						{
							AddInterface(&reactor_interface);
							floatReactorInterfaceAdded = true;
						}
						return new FloatReactor(loading);
					}

	const TCHAR *	ClassName() { return GetString(IDS_AF_REACTORFLOAT); }
	SClass_ID		SuperClassID() { return CTRL_FLOAT_CLASS_ID; }
	Class_ID		ClassID() { return REACTORFLOAT_CLASS_ID; }
	const TCHAR* 	Category() { return _T("");  }

	const TCHAR*	InternalName() { return _T("FloatReactor"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

	//You only need to add the action stuff to one Class Desc
	int             NumActionTables() { return 1; }
	ActionTable*  GetActionTable(int i) { return GetActions(); }

	};
static FloatReactorClassDesc floatReactorCD;
ClassDesc* GetFloatReactorDesc() {return &floatReactorCD;}

//-----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////
//************************************************************

class ReactorDlg : public ReferenceMaker, public TimeChangeCallback {
	public:
		Reactor *cont;	
		ParamDimensionBase *dim;
		IObjParam *ip;
		HWND hWnd;
		BOOL valid;
		DWORD updateFlags;

		int elems;  //This is reserved in case I have a variable # of spinners
		ISpinnerControl *iFloatState;		
		ISpinnerControl *iInfluence;
		ISpinnerControl *iStrength;
		ISpinnerControl *iFalloff[1];  //Variable number of spinners (reserved)
		ICustButton *iReactToBut;
		ICustButton *iCreateBut;
		ICustButton *iDeleteBut;
		ICustButton *iSetBut;
		ICustButton *iEditBut;
		ICustEdit	*iNameEdit;
		ICustEdit	*iValueStatus;


		ReactorActionCB<Reactor >	*reactorActionCB;		// Actions handler 		
		
		ReactorDlg(
			Reactor *cont,
			ParamDimensionBase *dim,
			TCHAR *pname,
			IObjParam *ip,
			HWND hParent);
		~ReactorDlg();

		Class_ID ClassID() {return Class_ID(REACTORDLG_CLASS_ID,0x67053d10);}
		SClass_ID SuperClassID() {return REF_MAKER_CLASS_ID;}

		void MaybeCloseWindow();
		void TimeChanged(TimeValue t) {Invalidate();}
		void Invalidate();
		void Update();
		void UpdateNodeName();
		void UpdateVarList();
		void UpdateReactionValue();
		void SetupUI(HWND hWnd);
		void Change(BOOL redraw=FALSE);
		void WMCommand(int id, int notify, HWND hCtrl);
		void SpinnerChange(int id,BOOL drag);
		void SpinnerStart(int id);
		void SpinnerEnd(int id,BOOL cancel);
		void SetupFalloffUI();

		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	         PartID& partID,  RefMessage message);
		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i) {return cont;}
		void SetReference(int i, RefTargetHandle rtarg) {cont=(Reactor*)rtarg;}
	};



//-----------------------------------------------------------------------

class FullRestore: public RestoreObj {
	public:		
		Reactor *sav;
		Reactor *cur;
		Reactor *redo; 
		FullRestore() { sav = cur = redo =  NULL; }
		FullRestore(Reactor *cont) {
			cur = cont;
			//theHold.Suspend();
			//sav = (Control*)cont->Clone();
			//theHold.Resume();
			sav = new Reactor();
			*sav = *cur;
			redo = NULL;
			}
		~FullRestore() {
			if (sav) delete sav;
			if (redo) delete redo;
			}		
		
		void Restore(int isUndo) {
			assert(cur); assert(sav);
			if (isUndo) {
				//theHold.Suspend();
				//redo = (Control *)cur->Clone();
				//theHold.Resume();
				redo = new Reactor();
				*redo = *cur;
				}
			//cur->Copy(sav);
			*cur = *sav;
			}
		void Redo() {
			assert(cur); 
			if (redo) 
				//cur->Copy(redo);
				*cur = *redo;
			}
		void EndHold() {}
		TSTR Description() { return TSTR(_T("FullReactorRestore")); }
	};


void Reactor::HoldAll()
	{
	if (theHold.Holding()) { 	
		theHold.Put(new FullRestore(this));
		}
	}


// A restore object to save the influence, strength, and falloff.
class SpinnerRestore : public RestoreObj {
	public:		
		Reactor *cont;
		Tab<SVar> ureaction, rreaction;
		float uselected, rselected;
		UINT flags;

		SpinnerRestore(Reactor *c) {
			cont=c;
			//ureaction = cont->reaction;
			ureaction.SetCount(cont->reaction.Count());
			for(int i = 0;i < cont->reaction.Count();i++)
			{
				memset(&ureaction[i], 0, sizeof(SVar));
				ureaction[i] = cont->reaction[i];
			}


			uselected = cont->selected;
			flags = cont->flags;
		}
		void Restore(int isUndo) {
			// if we're undoing, save a redo state
			if (isUndo) {
				//rreaction = cont->reaction;
				rreaction.SetCount(cont->reaction.Count());
				for(int i = 0;i < cont->reaction.Count();i++)
				{
					memset(&rreaction[i], 0, sizeof(SVar));
					rreaction[i] = cont->reaction[i];
				}

				rselected = cont->selected;
			}
			//cont->reaction = ureaction;
			cont->reaction.SetCount(ureaction.Count());
			for(int i = 0;i < ureaction.Count();i++)
			{
				memset(&cont->reaction[i], 0, sizeof(SVar));
				cont->reaction[i] = ureaction[i];
			}
			cont->selected = uselected;
			if (isUndo && !(flags&REACTOR_BLOCK_CURVE_UPDATE)) {
				cont->RebuildFloatCurves();
			}
			cont->count = cont->reaction.Count();
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			cont->NotifyDependents(FOREVER,PART_ALL,REFMSG_REACTION_COUNT_CHANGED);
		}
		void Redo() {
			//cont->reaction = rreaction;
			cont->reaction.SetCount(rreaction.Count());
			for(int i = 0;i < rreaction.Count();i++)
			{
				memset(&cont->reaction[i], 0, sizeof(SVar));
				cont->reaction[i] = rreaction[i];
			}
			cont->selected = rselected;
			cont->count = cont->reaction.Count();
			if (!(flags&REACTOR_BLOCK_CURVE_UPDATE))
				cont->RebuildFloatCurves();
			cont->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
			cont->NotifyDependents(FOREVER,PART_ALL,REFMSG_REACTION_COUNT_CHANGED);
		}
		void EndHold()
		{
		}
		int Size()
		{
			return sizeof(cont->reaction) + sizeof(float);
		}

};


void Reactor::HoldParams()
{
	if (theHold.Holding()) {
		theHold.Put(new SpinnerRestore(this));
	}
}


class StateRestore : public RestoreObj {
	public:
		Reactor *cont;
		Point3 ucurpval, rcurpval;
		float ucurfval, rcurfval;
		Quat ucurqval, rcurqval;
		Tab<SVar> ureaction, rreaction;

		StateRestore(Reactor *c) 
		{
			cont = c;
			ucurpval = cont->curpval;
			ucurqval = cont->curqval;
			ucurfval = cont->curfval;
			//ureaction = cont->reaction;
			ureaction.SetCount(cont->reaction.Count());
			for(int i = 0;i < cont->reaction.Count();i++)
			{
				memset(&ureaction[i], 0, sizeof(SVar));
				ureaction[i] = cont->reaction[i];
			}

		}   		
		void Restore(int isUndo) 
			{
			if (isUndo)
			{
				rcurpval = cont->curpval;
				rcurqval = cont->curqval;
				rcurfval = cont->curfval;
				//rreaction = cont->reaction;
				rreaction.SetCount(cont->reaction.Count());
				for(int i = 0;i < cont->reaction.Count();i++)
				{
					memset(&rreaction[i], 0, sizeof(SVar));
					rreaction[i] = cont->reaction[i];
				}
			}
			cont->curpval = ucurpval;
			cont->curqval = ucurqval;
			cont->curfval = ucurfval;
			//cont->reaction = ureaction;
			cont->reaction.SetCount(ureaction.Count());
			for(int i = 0;i < ureaction.Count();i++)
			{
				memset(&cont->reaction[i], 0, sizeof(SVar));
				cont->reaction[i] = ureaction[i];
			}
			if (isUndo) cont->UpdateCurves();
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
		void Redo()
			{
			cont->curpval = rcurpval;
			cont->curqval = rcurqval;
			cont->curfval = rcurfval;
			//cont->reaction = rreaction;
			cont->reaction.SetCount(rreaction.Count());
			for(int i = 0;i < rreaction.Count();i++)
			{
				memset(&cont->reaction[i], 0, sizeof(SVar));
				cont->reaction[i] = rreaction[i];
			}

			cont->UpdateCurves();
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}		
		void EndHold() 
			{ 
			cont->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T("Reactor State")); }
};

void Reactor::HoldTrack()
	{
	if (theHold.Holding()&&!TestAFlag(A_HELD)) {		
		theHold.Put(new StateRestore(this));
		SetAFlag(A_HELD);
		}
	}

class RangeRestore : public RestoreObj {
	public:
		Reactor *cont;
		Interval ur, rr;
		RangeRestore(Reactor *c) 
			{
			cont = c;
			ur   = cont->range;
			}   		
		void Restore(int isUndo) 
			{
			rr = cont->range;
			cont->range = ur;
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
		void Redo()
			{
			cont->range = rr;
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}		
		void EndHold() 
			{ 
			cont->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T("Reactor control range")); }
	};


void Reactor::HoldRange()
{
	if (theHold.Holding() && !TestAFlag(A_HELD)) {
		SetAFlag(A_HELD);
		theHold.Put(new RangeRestore(this));
		}
}
		

//IObjParam		*Reactor::ip = NULL;
//ReactorDlg		*Reactor::dlg = NULL;
/*
Reactor::Reactor(int t, Reactor &ctrl)
{
	type = t;
	DeleteAllRefsFromMe();
	MakeRefByID(FOREVER,0,vrefs.client);

	ip = ctrl.ip;
	//hParams = ctrl.hParams;
	dlg = NULL;
	range = ctrl.range;
	ivalid = ctrl.ivalid;
	selected = ctrl.selected;
	count = ctrl.count;
	editing = ctrl.editing;
	isBiped = ctrl.isBiped;
	curpval = ctrl.curpval;
	curfval = ctrl.curfval;
	curqval = ctrl.curqval;
	rtype = ctrl.rtype;
	type = ctrl.type;
	blockGetNodeName = FALSE;
	isCurveControlled = ctrl.isCurveControlled;
	iCCtrl = ctrl.iCCtrl;
	flags = ctrl.flags;
	pickReactToMode = NULL;
	upVector = ctrl.upVector;

}
*/

Reactor::Reactor() 
{
	Init();
}

Reactor::Reactor(int t, BOOL loading) 
{
	Init();
	type = t;
	if (loading) { isCurveControlled = 0; }  

}

void Reactor::Init()
{
	type = 0;
	range.Set(GetAnimStart(), GetAnimEnd());
	count = 0;
	selected = 0;
	editing = FALSE;
	isBiped = FALSE;
	nmaker = NULL;
	curpval = Point3(1.0f,1.0f,1.0f);
	curfval = 0.0f;
	curqval.Identity();
	ivalid.SetEmpty();
	blockGetNodeName = FALSE;
	isCurveControlled = 1;
	iCCtrl = NULL;
	flags = 0;
	upVector = Point3(0,0,-1);
	dlg = NULL;
	ip = NULL;
	pickReactToMode = NULL;
}


Reactor::~Reactor()
{
	deleteAllVars();
	if(nmaker) delete nmaker;
	if (dlg)
	{
		DestroyWindow(dlg->hWnd);
	}
	DeleteAllRefsFromMe();
}

void Reactor::deleteAllVars()
{
	reaction.SetCount(0);
	count = 0;
	selected = 0;
}

Reactor& Reactor::operator=(const Reactor& from)
{
	int i;
	type = from.type;
	rtype = from.rtype;	
	
	//reaction = from.reaction;
	reaction.SetCount(from.reaction.Count());
	for(i = 0;i < from.reaction.Count();i++)
	{
		memset(&reaction[i], 0, sizeof(SVar));
		reaction[i] = from.reaction[i];
	}
	
	count = from.count;	
	selected = from.selected;
	editing = from.editing;
	isBiped = from.isBiped;
	isCurveControlled = from.isCurveControlled;

	curfval = from.curfval;
	curpval = from.curpval;
	curqval = from.curqval;

	ivalid = from.ivalid;
	range = from.range;

	upVector = from.upVector;

	if (nmaker) delete nmaker;
	nmaker = GetCOREInterface()->NewNameMaker(FALSE);
	for (i=0;i<reaction.Count();i++)
	{
		nmaker->AddName(reaction[i].name);
	}

	return *this;
}

int Reactor::NumSubs() 
	{
	return 0;
	}

void Reactor::EditTimeRange(Interval range,DWORD flags)
{
	if(!(flags&EDITRANGE_LINKTOKEYS)){
		HoldRange();
		this->range = range;
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}
}

void Reactor::MapKeys(TimeMap *map,DWORD flags)
	{
	if (flags&TRACK_MAPRANGE) {
		HoldRange();
		TimeValue t0 = map->map(range.Start());
		TimeValue t1 = map->map(range.End());
		range.Set(t0,t1);
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
	}

int Reactor::NumRefs() 
{
		return 2;
}

RefTargetHandle Reactor::GetReference(int i) 
{
	switch (i)
	{
		case 0:
			return vrefs.client;
		case 1:
			return iCCtrl;
		default:
			return NULL;
	}
}

void Reactor::SetReference(int i, RefTargetHandle rtarg) 
{
	switch (i)
	{
		case 0:
			vrefs.client = (INode*)rtarg;
			break;
		case 1:
			iCCtrl = (ICurveCtl*)rtarg;
			break;
		default:
			break;
	}
}


RefResult Reactor::NotifyRefChanged(
		Interval iv, 
		RefTargetHandle hTarg, 
		PartID& partID, 
		RefMessage msg) 
{
	switch (msg) {
		case REFMSG_CHANGE:
			ivalid.SetEmpty();
			break;
		case REFMSG_GET_NODE_NAME:
		// RB 3/23/99: See comment at imp of getNodeName().
		if (blockGetNodeName) return REF_STOP;
		break;
		case REFMSG_TARGET_DELETED:
			if (hTarg==vrefs.client && vrefs.subnum < 0 )  //If it's a special case reference delete everything
			{
				vrefs.client = NULL;
				HoldParams();
				count = 0;
				reaction.ZeroCount();
				setSelected(-1);
				NotifyDependents(FOREVER,PART_ALL,REFMSG_REACTION_COUNT_CHANGED);
			}
			break;
		case REFMSG_REF_DELETED:
			break;
		case REFMSG_GET_CONTROL_DIM: 
			break;
		case REFMSG_NODE_NAMECHANGE:
			NotifyDependents(FOREVER,PART_ALL,REFMSG_REACTTO_OBJ_NAME_CHANGED);
			break;
		}
	return REF_SUCCEED;
}
 
void Reactor::Copy(Control *from)
{
	Point3 pointval;
	float floatval, f;
	Quat quatval;
	ScaleValue sv;
	
	if (from->ClassID() == ClassID() && ((Reactor*)from)->dlg) PostMessage(((Reactor*)from)->dlg->hWnd,WM_CLOSE,0,0);
	
	//if (from->ClassID() == ClassID() ) (*this) = *((Reactor*)from);
	//else {
		switch (type)
		{
			case REACTORPOS:
			case REACTORP3:
				from->GetValue(GetCOREInterface()->GetTime(), &pointval, ivalid);
				curpval = pointval;
				break;
			case REACTORROT:
				from->GetValue(GetCOREInterface()->GetTime(), &quatval, ivalid);
				curqval = quatval;
				break;
			case REACTORFLOAT:
				from->GetValue(GetCOREInterface()->GetTime(), &floatval, ivalid);
				f = floatval;
				curfval = f;
				break;
			case REACTORSCALE:
				from->GetValue(GetCOREInterface()->GetTime(), &sv, ivalid);
				curpval = sv.s;
				break;
			default: break;
		}
//	}
}

class ReactorPLCB : public PostLoadCallback
{
public:

	Reactor* cont;
	ReactorPLCB(Reactor* r) { cont = r; }
	virtual void proc(ILoad *iload){
		if (!cont->iCCtrl) cont->BuildCurves();
	}
};

#define REACTOR_VAR_RQUAT		0x5000
#define REACTOR_VAR_RVECTOR		0x5001
#define REACTOR_RTYPE_CHUNK		0x5002
#define REACTOR_VAR_STRENGTH	0x5003
#define REACTOR_VAR_FALLOFF		0x5004
#define REACTOR_ISBIPED_CHUNK	0x5005
#define REACTOR_UPVECTOR_CHUNK	0x5006
#define REACTOR_RANGE_CHUNK		0x6001
#define REACTOR_VREFS_REFCT		0x6002
#define REACTOR_VREFS_SUBNUM	0x6003
#define REACTOR_SVAR_TABSIZE	0x6004
#define REACTOR_VVAR_TABSIZE	0x6005
#define REACTOR_VAR_NAME		0x6006
#define REACTOR_VAR_VAL			0x6007
#define REACTOR_VAR_INF			0x6008
#define REACTOR_VAR_MULT		0x6009
#define REACTOR_VAR_FNUM		0x7000
#define REACTOR_VAR_POS			0x7300
#define REACTOR_VAR_QVAL		0x7600
#define REACTOR_USE_CURVE		0x7700
#define REACTOR_SVAR_ENTRY0		0x8000
#define REACTOR_SVAR_ENTRYN		0x8fff
#define REACTOR_VVAR_ENTRY0		0x9000
#define REACTOR_VVAR_ENTRYN		0x9fff


IOResult Reactor::Save(ISave *isave)
{		
	ULONG 	nb;
	int		i, ct, intVar;
 
	isave->BeginChunk(REACTOR_RTYPE_CHUNK);
	isave->Write(&rtype, sizeof(int), &nb);
 	isave->EndChunk();

	isave->BeginChunk(REACTOR_RANGE_CHUNK);
	isave->Write(&range, sizeof(range), &nb);
 	isave->EndChunk();

	isave->BeginChunk(REACTOR_ISBIPED_CHUNK);
	isave->Write(&isBiped, sizeof(BOOL), &nb);
 	isave->EndChunk();

	isave->BeginChunk(REACTOR_UPVECTOR_CHUNK);
	isave->Write(&upVector, sizeof(Point3), &nb);
 	isave->EndChunk();

	isave->BeginChunk(REACTOR_VREFS_REFCT);
	isave->Write(&vrefs.refCt, sizeof(int), &nb);
	isave->EndChunk();

	isave->BeginChunk(REACTOR_VREFS_SUBNUM);
	isave->Write(&vrefs.subnum, sizeof(int), &nb);
	isave->EndChunk();

	isave->BeginChunk(REACTOR_USE_CURVE);
	isave->Write(&isCurveControlled, sizeof(int), &nb);
 	isave->EndChunk();

	isave->BeginChunk(REACTOR_SVAR_TABSIZE);
	intVar = count;
	isave->Write(&intVar, sizeof(intVar), &nb);
 	isave->EndChunk();

	
	ct = count;
	for(i = 0; i < ct; i++) {
	 	isave->BeginChunk(REACTOR_SVAR_ENTRY0+i);
	 	 isave->BeginChunk(REACTOR_VAR_POS);
		 isave->Write(&reaction[i].pvalue, sizeof(Point3), &nb);
 		 isave->EndChunk();
	 	 isave->BeginChunk(REACTOR_VAR_NAME);
		 isave->WriteCString(reaction[i].name);
 		 isave->EndChunk();
	 	 isave->BeginChunk(REACTOR_VAR_VAL);
		 isave->Write(&reaction[i].fstate, sizeof(float), &nb);
 		 isave->EndChunk();
	 	 isave->BeginChunk(REACTOR_VAR_RQUAT);
		 isave->Write(&reaction[i].qstate, sizeof(Quat), &nb);
 		 isave->EndChunk();
	 	 isave->BeginChunk(REACTOR_VAR_RVECTOR);
		 isave->Write(&reaction[i].pstate, sizeof(Point3), &nb);
 		 isave->EndChunk();
	 	 isave->BeginChunk(REACTOR_VAR_INF);
		 isave->Write(&reaction[i].influence, sizeof(float), &nb);
 		 isave->EndChunk();
	 	 isave->BeginChunk(REACTOR_VAR_STRENGTH);
		 isave->Write(&reaction[i].strength, sizeof(float), &nb);
 		 isave->EndChunk();
	 	 isave->BeginChunk(REACTOR_VAR_FALLOFF);
		 isave->Write(&reaction[i].falloff, sizeof(float), &nb);
 		 isave->EndChunk();
	 	 isave->BeginChunk(REACTOR_VAR_MULT);
		 isave->Write(&reaction[i].multiplier, sizeof(float), &nb);
 		 isave->EndChunk();
	 	 isave->BeginChunk(REACTOR_VAR_FNUM);
		 isave->Write(&reaction[i].fvalue, sizeof(float), &nb);
 		 isave->EndChunk();
	 	 isave->BeginChunk(REACTOR_VAR_QVAL);
		 isave->Write(&reaction[i].qvalue, sizeof(Quat), &nb);
 		 isave->EndChunk();
	 	isave->EndChunk();
	}
	return IO_OK;
}

IOResult Reactor::Load(ILoad *iload)
	{
	ULONG 	nb;
	TCHAR	*cp;
	int		id, i, varIndex, intVar = 0;
	IOResult res;
	VarRef	dummyVarRef;
	ReactorPLCB* plcb = new ReactorPLCB(this);
	iload->RegisterPostLoadCallback(plcb);

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (id = iload->CurChunkID()) {
	
		case REACTOR_RTYPE_CHUNK:
			iload->Read(&rtype, sizeof(int), &nb);
			break;
		case REACTOR_RANGE_CHUNK:
			iload->Read(&range, sizeof(range), &nb);
			break;
		case REACTOR_ISBIPED_CHUNK:
			iload->Read(&isBiped, sizeof(BOOL), &nb);
			break;
		case REACTOR_UPVECTOR_CHUNK:
			iload->Read(&upVector, sizeof(Point3), &nb);
			break;
		case REACTOR_VREFS_REFCT:
			iload->Read(&vrefs.refCt, sizeof(int), &nb);
			break;
		case REACTOR_VREFS_SUBNUM:
			iload->Read(&vrefs.subnum, sizeof(int), &nb);
			break;
		case REACTOR_USE_CURVE:
			iload->Read(&isCurveControlled, sizeof(int), &nb);
			break;
		case REACTOR_SVAR_TABSIZE:
			iload->Read(&intVar, sizeof(intVar), &nb);
			reaction.SetCount(intVar);
			for(i = 0; i < intVar; i++)
				memset(&reaction[i], 0, sizeof(SVar));
			break;
		}	
		if(id >= REACTOR_SVAR_ENTRY0 && id <= REACTOR_SVAR_ENTRYN) {
			varIndex = id - REACTOR_SVAR_ENTRY0;
			assert(varIndex < reaction.Count());
			while (IO_OK == iload->OpenChunk()) {
				switch (iload->CurChunkID()) {
				case REACTOR_VAR_NAME:
					iload->ReadCStringChunk(&cp);
					reaction[varIndex].name = cp;
					break;
				case REACTOR_VAR_VAL:
					iload->Read(&reaction[varIndex].fstate, sizeof(float), &nb);
					break;
				case REACTOR_VAR_RQUAT:
					iload->Read(&reaction[varIndex].qstate, sizeof(Quat), &nb);
					break;
				case REACTOR_VAR_RVECTOR:
					iload->Read(&reaction[varIndex].pstate, sizeof(Point3), &nb);
					break;
				case REACTOR_VAR_INF:
					iload->Read(&reaction[varIndex].influence, sizeof(float), &nb);
					break;
				case REACTOR_VAR_STRENGTH:
					iload->Read(&reaction[varIndex].strength, sizeof(float), &nb);
					break;
				case REACTOR_VAR_FALLOFF:
					iload->Read(&reaction[varIndex].falloff, sizeof(float), &nb);
					break;
				case REACTOR_VAR_MULT:
					iload->Read(&reaction[varIndex].multiplier, sizeof(float), &nb);
					break;
				case REACTOR_VAR_FNUM:
					iload->Read(&reaction[varIndex].fvalue, sizeof(float), &nb);
					break;
				case REACTOR_VAR_POS:
					iload->Read(&reaction[varIndex].pvalue, sizeof(Point3), &nb);
					break;
				case REACTOR_VAR_QVAL:
					iload->Read(&reaction[varIndex].qvalue, sizeof(Quat), &nb);
					break;
				}	
				iload->CloseChunk();
			}
		}
		iload->CloseChunk();
	}
	updReactionCt(intVar);
	return IO_OK;
}

// RB 3/23/99: To solve 75139 (the problem where a node name is found for variables that 
// are not associated with nodes such as globabl tracks) we need to block the propogation
// of this message through our reference to the client of the variable we're referencing.
// In the expression controller's imp of NotifyRefChanged() we're going to block the get
// node name message if the blockGetNodeName variable is TRUE.
void Reactor::getNodeName(ReferenceTarget *client, TSTR &name)
{
	blockGetNodeName = TRUE;
	if (client) client->NotifyDependents(FOREVER,(PartID)&name,REFMSG_GET_NODE_NAME);
	blockGetNodeName = FALSE;
}

void Reactor::setSelected(int i) 
{
	selected = i; 
	NotifyDependents(FOREVER,PART_ALL,REFMSG_REACTOR_SELECTION_CHANGED);
	if (editing) {
		curpval = reaction[i].pstate;
		curqval = reaction[i].qstate;
		curfval = reaction[i].fstate;
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE); 
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
	}
	return;
}

void Reactor::ComputeMultiplier(TimeValue t)
{
	if (!iCCtrl) BuildCurves();

	float m, mtemp, normval, total;
	int i, j;
	Tab<float> ftab, inftab;
	Point3 axis;

	ftab.ZeroCount();
	inftab.ZeroCount();
	total = 0.0f;
	if (!editing)
	{
		//if(iCCtrl && getReactionType() != FLOAT_VAR && iCCtrl->GetNumCurves() != reaction.Count()) { assert(0); return; }
		
		float mult;

		// Limit to in range
		if (t<range.Start()) t = range.Start();
		if (t>range.End()) t = range.End();	

		//Make sure there is always an influentual reaction
		//If not create a temp influence value that is large enough
		//First sum up all multiplier values
		for(i=0;i<count;i++){
			inftab.Append(1, &(reaction[i].influence)); 
			switch (rtype)
			{
				case FLOAT_VAR:
					mult = 1.0f-((float)fabs(getCurFloatValue(t)-reaction[i].fvalue)/(reaction[i].influence));
					break;
				case VECTOR_VAR: 
					mult = 1.0f-(Distance(reaction[i].pvalue, getCurPoint3Value(t))/(reaction[i].influence));
					break;
				case SCALE_VAR:
					mult = 1.0f-(Distance(reaction[i].pvalue, (getCurScaleValue(t)).s)/(reaction[i].influence));
					break;
				case QUAT_VAR: 
					mult = 1.0f-(QangAxis(reaction[i].qvalue, getCurQuatValue(t), axis)/reaction[i].influence);
					break;

				default: assert(0);
			}
			if (mult<0) mult = 0.0f;
			total += mult;
		}
		//Check to see if any are influencial, total > 0 if any influence
		if(total <= 0.0f) {
			//find the closest reaction
			int which;  
			float closest, closesttemp;
			closest = 10000000.0f;
			which = 0;
			for(i=0;i<count;i++)
			{
				switch (rtype)
				{
					case FLOAT_VAR:
						closesttemp = ((float)fabs(getCurFloatValue(t)-reaction[i].fvalue)<closest ? (float)fabs(getCurFloatValue(t)-reaction[i].fvalue) : closest);
						break;
					case VECTOR_VAR: 
						closesttemp = (Distance(reaction[i].pvalue, getCurPoint3Value(t))<closest ? Distance(reaction[i].pvalue, getCurPoint3Value(t)) : closest);
						break;
					case SCALE_VAR: 
						closesttemp = (Distance(reaction[i].pvalue, (getCurScaleValue(t)).s)<closest ? Distance(reaction[i].pvalue, (getCurScaleValue(t)).s) : closest);
						break;
					case QUAT_VAR: 
						closesttemp = (QangAxis(reaction[i].qvalue, getCurQuatValue(t), axis)<closest ? QangAxis(reaction[i].qvalue, getCurQuatValue(t), axis) : closest);
						break;
					default: closesttemp = 10000000.0f;
				}
				if (closesttemp < closest) 
				{
					which = i;	
					closest = closesttemp;
				}
			}
			if(count&&closest) 
			{
				inftab[which] = closest + 1.0f;  //make the influence a little more than the closest reaction
			}
		}


		//Get the initial multiplier by determining it's influence
		for(i=0;i<count;i++)
		{
			switch (rtype)
			{
				case FLOAT_VAR:
					m = 1.0f-((float)fabs(getCurFloatValue(t)-reaction[i].fvalue)/inftab[i]);
					break;
				case VECTOR_VAR: 
					m = 1.0f-(Distance(reaction[i].pvalue, getCurPoint3Value(t))/inftab[i]);
					break;
				case SCALE_VAR: 
					m = 1.0f-(Distance(reaction[i].pvalue, (getCurScaleValue(t)).s)/inftab[i]);
					break;
				case QUAT_VAR: 
					m = 1.0f-(QangAxis(reaction[i].qvalue, getCurQuatValue(t), axis)/inftab[i]);
					break;

				default: assert(0);
			}
			if(m<0) m=0;
			reaction[i].multiplier = m;
		}
		
		for(i=0;i<count;i++)
		{
			//add the strength
			reaction[i].multiplier  *= reaction[i].strength;
		}

		//Make an adjustment so that when a value is reached 
		//the state is also reached reguardless of the other influentual reactions 
		for(i=0;i<count;i++)
		{
			mtemp = 1.0f;
			for(j=0;j<count;j++)
			{
				BOOL is_same = false;
				switch (rtype)
				{
					case SCALE_VAR:
					case VECTOR_VAR:
						if((*((Point3*)getReactionValue(j))) == (*((Point3*)getReactionValue(i)))) 
							is_same = true;
						break;
					case QUAT_VAR:
						if((*((Quat*)getReactionValue(j))) == (*((Quat*)getReactionValue(i)))) 
							is_same = true;
						break;
					case FLOAT_VAR:
						if((*((float*)getReactionValue(j))) == (*((float*)getReactionValue(i)))) 
							is_same = true;
						break;
					default : is_same = false;
				}
				if (is_same ) mtemp *= reaction[j].multiplier; 
					else mtemp *= (1.0f - reaction[j].multiplier);
			}
			if(mtemp<0) mtemp = 0.0f;
			ftab.Append(1, &mtemp);
		}

		//update the Reaction multipliers
		for(i=0;i<count;i++)
		{
			reaction[i].multiplier = ftab[i];
			//compute the falloff
			if (!isCurveControlled) reaction[i].multiplier = (float)pow(reaction[i].multiplier, (1/reaction[i].falloff));
			else 
			{
				ICurve* curve = iCCtrl->GetControlCurve(i);
				if (curve) reaction[i].multiplier = curve->GetValue(0, reaction[i].multiplier, FOREVER, FALSE);
			}
		}
		//make sure they always add up to 1.0
		int valcount = 0;
		total = 0.0f;
		for(i=0;i<count;i++)
			total +=reaction[i].multiplier;
		if (!total) total = 1.0f; 
		normval = 1.0f/total;
		for(i=0;i<count;i++)
		reaction[i].multiplier *= normval;
	}
}


void Reactor::reactTo(ReferenceTarget *anim, TimeValue t)
{
	Animatable* nd;
	if (nmaker) delete nmaker;
	nmaker = NULL;

	theHold.Begin();
	HoldAll();

	switch ( anim->SuperClassID() )
	{
		case CTRL_FLOAT_CLASS_ID:
			setReactionType(FLOAT_VAR); break;
		case CTRL_POINT3_CLASS_ID:
		case CTRL_POSITION_CLASS_ID:
			setReactionType(VECTOR_VAR); break;
		case CTRL_SCALE_CLASS_ID:
			setReactionType(SCALE_VAR); break;
		case CTRL_ROTATION_CLASS_ID:
			setReactionType(QUAT_VAR); break;
		default: setReactionType(VECTOR_VAR); 
	}

	if (anim->SuperClassID()==BASENODE_CLASS_ID)
	{
		nd = (INode*)anim;

		Control *c = ((INode*)nd)->GetTMController();
		if (c->ClassID() == BIPSLAVE_CONTROL_CLASS_ID || c->ClassID() == IKSLAVE_CLASSID)
		{
			isABiped(TRUE);
			setReactionType(QUAT_VAR);
			if (!(assignReactObj((INode*)nd, -2))) return;
		}else {
			setReactionType(VECTOR_VAR);
			if (!(assignReactObj((INode*)nd, -1))) return;
		}
	} 
	else {
		MyEnumProc dep;             
		((ReferenceTarget*)anim)->EnumDependents(&dep);

		for(int x=0; x<dep.nodes.Count(); x++)
		{
			for(int i=0; i<dep.nodes[x]->NumSubs(); i++)
			{
				Animatable* n = dep.nodes[x]->SubAnim(i);
				if ((Control*)n == (Control*)anim)
				{
					if (!(assignReactObj((INode*)dep.nodes[x], i))) return;
				}
			}
		}
	}
	iCCtrl = NULL;
	theHold.Suspend();
	BuildCurves();
	theHold.Resume();

	CreateReaction();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_REACTTO_OBJ_NAME_CHANGED);

	if (theHold.Holding()) theHold.Accept(GetString(IDS_ASSIGN_TO));
}


/*
void Reactor::assignTo(TrackViewPick res)
{
	theHold.Begin();
	HoldAll();

	switch (res.anim->SuperClassID())
	{
		case CTRL_FLOAT_CLASS_ID:
			setrType(FLOAT_VAR); break;
		case CTRL_POINT3_CLASS_ID:
		case CTRL_POSITION_CLASS_ID:
			setrType(VECTOR_VAR); break;
		case CTRL_SCALE_CLASS_ID:
			setrType(SCALE_VAR); break;
		case CTRL_ROTATION_CLASS_ID:
			setrType(QUAT_VAR); break;
		default: setrType(VECTOR_VAR); //assert(0);
	}

	
	if (res.anim->SuperClassID()==BASENODE_CLASS_ID) {

		Control *c = ((INode*)res.anim)->GetTMController();
		if (c->ClassID() == BIPSLAVE_CONTROL_CLASS_ID || c->ClassID() == IKSLAVE_CLASSID)
		{
			isBiped = TRUE;
			setrType(QUAT_VAR);
			assignReactObj((INode*)res.anim, -2);
		}else {
			setrType(VECTOR_VAR);
			assignReactObj((INode*)res.anim, -1);
		}
	} 
	else {
		assignReactObj((INode*)res.client, res.subNum);
		}

	CreateReaction();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);

	theHold.Accept(GetString(IDS_ASSIGN_TO));

}

*/ 

BOOL Reactor::assignReactObj(INode* client, int subNum)
{
	if (!client) return FALSE;

	reaction.ZeroCount();
	count = 0;
	selected = 0;

	if(ReplaceReference(0, client) != REF_SUCCEED) {
		vrefs = NULL;
		count = 0;
		if (dlg)
		{
			TSTR s = GetString(IDS_AF_CIRCULAR_DEPENDENCY);
			MessageBox(dlg->hWnd, s, GetString(IDS_AF_CANT_ASSIGN), 
				MB_ICONEXCLAMATION | MB_SYSTEMMODAL | MB_OK);
		}
		theHold.Cancel();
		return FALSE;
	}
	vrefs.subnum = subNum;

	return TRUE;
}

void Reactor::updReactionCt(int val)
{
	count += val;
	if (rtype == FLOAT_VAR) SortReactions();
}


SVar* Reactor::CreateReactionAndReturn(BOOL setDefaults, TCHAR *buf, TimeValue t)
{
	ivalid = FOREVER;
	theHold.Begin();
	HoldParams();  
	
	int i;
	SVar sv;
	sv.refID = -1;

	sv.strength = 1.0f;
	sv.falloff = 2.0f;
	sv.influence = 100.0f;
	
	i = reaction.Append(1, &sv);
	//SVar curReaction = reaction[i];

	TSTR mname(GetString(IDS_AF_VARNAME));
	if (buf == NULL)
	{
		if (!nmaker) nmaker = GetCOREInterface()->NewNameMaker(FALSE);
		nmaker->MakeUniqueName(mname);
	}else mname = buf;
	setReactionName(i, mname);
	selected = i;

	if (setDefaults)
	{
		setState(i, t);
		setReactionValue(i, t);
		
		//Scheme to set influence to nearest reaction automatically (better defaults)
		if (i!=0)		//if its not the first reaction
		{
			if(i == 1)   //and if it is the second one update the first while your at it
				setMinInfluence(0);
			setMinInfluence(i);
		}
	}

	theHold.Suspend();

	if (iCCtrl) 
	{ 
		if (rtype == FLOAT_VAR)
		{
			if (!(flags&REACTOR_BLOCK_CURVE_UPDATE))
			{
				iCCtrl->SetNumCurves(Elems(), TRUE); 
				RebuildFloatCurves();
			}
		}	
		else 
		{
			iCCtrl->SetNumCurves(reaction.Count(), TRUE); 
			NewCurveCreatedCallback(reaction.Count()-1, iCCtrl); 
		}
	}
	
	BOOL isSame = FALSE;
	int retVal = ( !(flags & REACTOR_DONT_CREATE_SIMILAR) );

	if (setDefaults)
	{
		for (int x=0; x < i; x++)
		{
			void *iVal = getReactionValue(i);
			void *xVal = getReactionValue(x);	
			switch (rtype)
			{
				case FLOAT_VAR:
					if ( *((float*)iVal) == *((float*)xVal) )
						isSame = TRUE;
					break;

				case VECTOR_VAR:
				case SCALE_VAR:
					if ( *((Point3*)iVal) == *((Point3*)xVal) )
						isSame = TRUE;
					break;
				case QUAT_VAR:
					if ( *((Point3*)iVal) == *((Point3*)xVal) )
						isSame = TRUE;
					break;
				default: break;	
			}	
			if ( isSame )
			{
				//check to see if the reaction value is the same as that of an existing reaction.  If it is, post a message
				if ( !(flags&REACTOR_DONT_SHOW_CREATE_MSG) )
				{
					DWORD extRetVal;
					HWND hWnd;

					if (dlg) hWnd = dlg->hWnd;
						else hWnd = GetCOREInterface()->GetMAXHWnd();

					TSTR *question = new TSTR(GetString(IDS_SAME_REACTION));
					retVal = ( MaxMsgBox(hWnd, question->data(), GetString(IDS_AF_REACTOR),
								MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON1, //|MB_HELP, 
								MAX_MB_DONTSHOWAGAIN, &extRetVal) == IDYES );
					delete question;
					
					//don't show the dialog again (for this session anyway...)
					if ( extRetVal == MAX_MB_DONTSHOWAGAIN ) 
					{
						flags = (flags | REACTOR_DONT_SHOW_CREATE_MSG);
						if (retVal == 0)
						{
							flags = (flags|REACTOR_DONT_CREATE_SIMILAR);
						}
					}
				}
				break;
			}
		}
	}
	theHold.Resume();

	if (isSame && retVal == 0)
	{
		theHold.Cancel();
		return NULL;
	}

	updReactionCt(1);

	if (theHold.Holding()) theHold.Accept(GetString(IDS_CREATE_REACTION));
	NotifyDependents(FOREVER,PART_ALL,REFMSG_REACTION_COUNT_CHANGED);	
	return &(reaction[selected]);
}

BOOL Reactor::CreateReaction(TCHAR *buf, TimeValue t)
{
	if ( CreateReactionAndReturn(TRUE, buf, t) ) return TRUE;
	else return FALSE;
}

BOOL Reactor::DeleteReaction(int i)
{
	if(vrefs.client != NULL)
	{		
		if (i == -1) i = selected;
		if (getReactionCount()>1)  //can't delete the last reaction
		{
			theHold.Begin();
			HoldParams(); 

			if (getReactionType() != FLOAT_VAR && iCCtrl->GetNumCurves() >= reaction.Count())
			{
				iCCtrl->DeleteCurve(i);
			}

			reaction.Delete(i, 1);
			count = getReactionCount(); 
			if (selected >= getReactionCount() ) selected -=1;

			if (!(flags&REACTOR_BLOCK_CURVE_UPDATE))
				RebuildFloatCurves();

			ivalid.SetEmpty();
			NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
			NotifyDependents(FOREVER,PART_ALL,REFMSG_REACTION_COUNT_CHANGED);	
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
			if (theHold.Holding()) theHold.Accept(GetString(IDS_DELETE_REACTION));
			return TRUE;
		}

	}return FALSE;
}



TCHAR *Reactor::getReactionName(int i)
{
	if(i>=0&&i<count&&count>0)
		return reaction[i].name;
	return "error";
}

void Reactor::setReactionName(int i, TSTR name)
{
	if (i>=0&&i<reaction.Count())
	{
		reaction[i].name = name;
		NotifyDependents(FOREVER,i,REFMSG_REACTION_NAME_CHANGED);
	}
}

void* Reactor::getReactionValue(int i)
{
	if(getReactionCount() > 0 && i >= 0 && i < getReactionCount())
	{
		switch (rtype)
		{
			case FLOAT_VAR:
				return &reaction[i].fvalue;
			case QUAT_VAR:
				return &reaction[i].qvalue;
			case VECTOR_VAR:
			case SCALE_VAR:
				return &reaction[i].pvalue;
			default: assert(0);
		}
	}
	return NULL;
}

FPValue Reactor::fpGetReactionValue(int index)
{
	FPValue val = FPValue();
	if(getReactionCount() > 0 && index >= 0 && index < getReactionCount())
	{
		switch (rtype)
		{
			case FLOAT_VAR:
				val.type = (ParamType2)TYPE_FLOAT;
				val.f =  reaction[index].fvalue;
				break;
			case QUAT_VAR:
				val.type = (ParamType2)TYPE_QUAT;
				val.q =  &reaction[index].qvalue;
				break;
			case VECTOR_VAR:
			case SCALE_VAR:
				val.type = (ParamType2)TYPE_POINT3;
				val.p = &reaction[index].pvalue;
				break;
			default: assert(0);
		}
	} else throw MAXException("the index is not within the valid range");  // globalize!
	return val;
}


/*
BOOL Reactor::setReactionValue(int i, void *val, TimeValue t)
{
	float f;
	Quat q;
	Point3 p;
	ScaleValue s;
	Control *c;
	
	if (t == NULL) t = GetCOREInterface()->GetTime();

	if (i == -1) i = selected;

	if (vrefs.client != NULL) {

		theHold.Begin();
		HoldParams();
		
		reaction[i].fvalue = 0.0f;
		reaction[i].pvalue = Point3(0,0,0);
		reaction[i].qvalue.Identity();

		if ( val == NULL )
		{
			if (vrefs.subnum < 0 )
			{
				if (isBiped)	
				{
					GetAbsoluteControlValue(vrefs.client, t, &(reaction[i].qvalue), FOREVER);
				}
				else {
					GetAbsoluteControlValue(vrefs.client, t, &(reaction[i].pvalue), FOREVER);
				}
			}
			else {
				c = (Control *)vrefs.client->SubAnim(vrefs.subnum);
				switch (rtype)
				{
					case FLOAT_VAR:
						c->GetValue(t, &f, FOREVER);
						reaction[i].fvalue = f;
						break;
					case VECTOR_VAR:
						c->GetValue(t, &p, FOREVER);
						reaction[i].pvalue = p;
						break;
					case SCALE_VAR:
						c->GetValue(t, &s, FOREVER);
						reaction[i].pvalue = s.s;
						break;
					case QUAT_VAR:
						c->GetValue(t, &q, FOREVER);
						reaction[i].qvalue = q;
						break;
				}
			}
		} else {
			switch (rtype)
			{
				case FLOAT_VAR:
					reaction[i].fvalue = (*(float*)val);
					break;
				case SCALE_VAR:
				case VECTOR_VAR:
					reaction[i].pvalue = (*(Point3*)val);
					break;
				case QUAT_VAR:
					reaction[i].qvalue = (*(Quat*)val);
					break;
			}
		}
		if( ip ) dlg->UpdateReactionValue();   //Updates the value field
		ivalid.SetEmpty();
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
		theHold.Accept(GetString(IDS_UNSET_VALUE));
		return TRUE;
	}
	return FALSE;
}
*/

BOOL Reactor::setReactionValue(int i, TimeValue t)
{
	float f;
	Quat q;
	Point3 p;
	ScaleValue s;
	Control *c;
	
	if (t == NULL) t = GetCOREInterface()->GetTime();
	if(getReactionCount() == 0 || i < 0 || i >= getReactionCount()) 
	{
		if (i == -1) i = selected;
			else return false;
	}

	if (vrefs.client != NULL) {

		theHold.Begin();
		HoldParams();
		
		reaction[i].fvalue = 0.0f;
		reaction[i].pvalue = Point3(0,0,0);
		reaction[i].qvalue.Identity();

		if (vrefs.subnum < 0 )
		{
			if (isBiped)	
			{
				GetAbsoluteControlValue(vrefs.client, t, &(reaction[i].qvalue), FOREVER);
			}
			else {
				GetAbsoluteControlValue(vrefs.client, t, &(reaction[i].pvalue), FOREVER);
			}
		}
		else {
			c = (Control *)vrefs.client->SubAnim(vrefs.subnum);
			switch (rtype)
			{
				case FLOAT_VAR:
					c->GetValue(t, &f, FOREVER);
					reaction[i].fvalue = f;
					SortReactions();
					UpdateCurves();
					break;
				case VECTOR_VAR:
					c->GetValue(t, &p, FOREVER);
					reaction[i].pvalue = p;
					break;
				case SCALE_VAR:
					c->GetValue(t, &s, FOREVER);
					reaction[i].pvalue = s.s;
					break;
				case QUAT_VAR:
					c->GetValue(t, &q, FOREVER);
					reaction[i].qvalue = q;
					break;
			}
		}

		if( ip ) dlg->UpdateReactionValue();   //Updates the value field
		ivalid.SetEmpty();
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
		if (theHold.Holding()) theHold.Accept(GetString(IDS_UNSET_VALUE));
		return TRUE;
	}
	return FALSE;
}


BOOL Reactor::setReactionValue(int i, float val)
{
	if (getReactionType() != FLOAT_VAR ) return false;
	if(getReactionCount() == 0 || i < 0 || i >= getReactionCount()) 
	{
		if (i == -1) i = selected;
			else return false;
	}

	if (vrefs.client != NULL) {

		theHold.Begin();
		HoldParams();
		
		reaction[i].fvalue = val;
		SortReactions();

		if( ip ) dlg->UpdateReactionValue();   //Updates the value field
		ivalid.SetEmpty();
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
		if (theHold.Holding()) theHold.Accept(GetString(IDS_UNSET_VALUE));
		UpdateCurves();
		return TRUE;
	}
	return FALSE;
}

BOOL Reactor::setReactionValue(int i, Point3 val)
{
	if (getReactionType() != VECTOR_VAR && getReactionType() != SCALE_VAR ) return false;
	if(getReactionCount() == 0 || i < 0 || i >= getReactionCount()) 
	{
		if (i == -1) i = selected;
			else return false;
	}

	if (vrefs.client != NULL) {

		theHold.Begin();
		HoldParams();
		
		reaction[i].pvalue = val;
		
		if( ip ) dlg->UpdateReactionValue();   //Updates the value field
		ivalid.SetEmpty();
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
		if (theHold.Holding()) theHold.Accept(GetString(IDS_UNSET_VALUE));
		return TRUE;
	}
	return FALSE;
}

BOOL Reactor::setReactionValue(int i, Quat val)
{
	if (getReactionType() != QUAT_VAR ) return false;
	if(getReactionCount() == 0 || i < 0 || i >= getReactionCount()) 
	{
		if (i == -1) i = selected;
			else return false;
	}

	if (vrefs.client != NULL) {

		theHold.Begin();
		HoldParams();
		
		reaction[i].qvalue = val;
		
		if( ip ) dlg->UpdateReactionValue();   //Updates the value field
		ivalid.SetEmpty();
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
		if (theHold.Holding()) theHold.Accept(GetString(IDS_UNSET_VALUE));
		return TRUE;
	}
	return FALSE;
}

float Reactor::getCurFloatValue(TimeValue t)
{
	float f;
	Control *c;

	if (vrefs.client != NULL) {
		if (c = GetControlInterface(vrefs.client->SubAnim(vrefs.subnum)))
			{
			c->GetValue(t, &f, FOREVER);
			return f;
			}
	}
	return 0.0f;
}

Point3 Reactor::getCurPoint3Value(TimeValue t)
{
	Point3 p = Point3(0,0,0);
	Control *c;

	if (vrefs.client != NULL) {
		if (vrefs.subnum < 0 )
		{
			GetAbsoluteControlValue(vrefs.client, t, &p, FOREVER);
		}else {
			if (c = GetControlInterface(vrefs.client->SubAnim(vrefs.subnum)))
				c->GetValue(t, &p, FOREVER);
		}
	}
	return p;
}

ScaleValue Reactor::getCurScaleValue(TimeValue t)
{
	ScaleValue ss;
	Control *c;

	if (vrefs.client != NULL) {
		if (c = GetControlInterface(vrefs.client->SubAnim(vrefs.subnum)))
			c->GetValue(t, &ss, FOREVER);
	}
	return ss;
}

Quat Reactor::getCurQuatValue(TimeValue t)
{
	Quat q;
	q.Identity();
	Control *c;

	if (vrefs.client != NULL) {
		if (vrefs.subnum < 0 )
		{
			GetAbsoluteControlValue(vrefs.client, t, &q, FOREVER);
		}else {
			if (c = GetControlInterface(vrefs.client->SubAnim(vrefs.subnum)))
				c->GetValue(t, &q, FOREVER);
		}
	}
	return q;
}



float Reactor::getInfluence(int num)
{
	if (num >= 0 && num <reaction.Count())
		return reaction[num].influence;
	return 0;
}

BOOL Reactor::setInfluence(int num, float inf)
{
	BOOL hold_here = false;

	if (!theHold.Holding()) { hold_here=true; theHold.Begin(); HoldParams();}

	reaction[num].influence = inf;

	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
	if (hold_here && theHold.Holding() ) theHold.Accept(GetString(IDS_AF_CHANGEINFLUENCE));
	return TRUE;
}

void Reactor::setMinInfluence(int x)
{
	BOOL hold_here = false;
	if (!theHold.Holding()) 
	{ 
		hold_here=true; theHold.Begin(); HoldParams();
	}
	
	if ( x == -1 ) x = selected;
	float dist = 1000000.0f;
	float disttemp = 100.0f;
	Point3 axis;

	if ( reaction.Count() && x >= 0 )
	{
		for(int i=0;i<reaction.Count();i++)
		{
			if (i != x)
			{
				switch (rtype)
				{
					case FLOAT_VAR:
						disttemp = (float)fabs(reaction[x].fvalue - reaction[i].fvalue);
						break;
					case VECTOR_VAR: 
					case SCALE_VAR: 
						disttemp = Distance(reaction[i].pvalue, reaction[x].pvalue);
						break;
					case QUAT_VAR: 
						disttemp = QangAxis(reaction[i].qvalue, reaction[x].qvalue, axis);
						break;
					default: disttemp = 100.0f; break;
				} 
			}
			if (disttemp != 0.0f) 
			{
				dist = (dist <= disttemp && dist != 100 ? dist : disttemp);
			} else if (dist == 1000000.0f && i == reaction.Count()-1) dist = 100.0f;
		}
		reaction[x].influence = dist;
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		if (theHold.Holding() && hold_here) theHold.Accept(GetString(IDS_AF_CHANGEINFLUENCE));
	}
}

void Reactor::setMaxInfluence(int x)
{
	BOOL hold_here = false;
	if (!theHold.Holding()) { hold_here=true; theHold.Begin(); HoldParams();}
	
	if ( x == -1 ) x = selected;

	float dist;
	float disttemp;
	Point3 axis;

	if ( count && x >= 0 )
	{
		for(int i=0;i<count;i++)
		{
			switch (rtype)
			{
				case FLOAT_VAR:
					disttemp = (float)fabs(reaction[x].fvalue - reaction[i].fvalue);
					break;
				case VECTOR_VAR: 
				case SCALE_VAR: 
					disttemp = Distance(reaction[i].pvalue, reaction[x].pvalue);
					break;
				case QUAT_VAR: 
					disttemp = QangAxis(reaction[i].qvalue, reaction[x].qvalue, axis);
					break;
				default: disttemp = 100.0f; break;
			}
			if (i == 0) dist = disttemp;
			else dist = (dist >= disttemp ? dist : disttemp);
		}
		reaction[x].influence = dist;
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		if (theHold.Holding() && hold_here) theHold.Accept(GetString(IDS_AF_CHANGEINFLUENCE));
	}
}



float Reactor::getStrength(int num)
{
	return reaction[num].strength;
}

BOOL Reactor::setStrength(int num, float inf)
{
	BOOL hold_here = false;

	if (!theHold.Holding()) { hold_here=true; theHold.Begin(); HoldParams();}

	reaction[num].strength = inf;

	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
	if(theHold.Holding() && hold_here) theHold.Accept(GetString(IDS_AF_CHANGESTRENGTH));
	return TRUE;
}

float Reactor::getFalloff(int num)
{
	return reaction[num].falloff;
}

BOOL Reactor::setFalloff(int num, float inf)
{
	BOOL hold_here = false;

	if (!theHold.Holding()) { hold_here=true; theHold.Begin(); HoldParams();}

	reaction[num].falloff = inf;

	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
	if (theHold.Holding() && hold_here) theHold.Accept(GetString(IDS_AF_CHANGEFALLOFF));
	return TRUE;
}

void Reactor::setEditReactionMode(BOOL ed)
{
	editing = ed;
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
}

BOOL Reactor::setState(int num, TimeValue t)
{
	if(getReactionCount() == 0 || num < 0 || num >= getReactionCount()) 
		if ( num == -1 ) num = selected;
			else return false;

	if (t == NULL) t = GetCOREInterface()->GetTime();
	switch (type)
	{
		case REACTORFLOAT: 
			this->GetValue(t, &(reaction[num].fstate), FOREVER, CTRL_ABSOLUTE);
			break; 
		case REACTORROT: 
			this->GetValue(t, &(reaction[num].qstate), FOREVER, CTRL_ABSOLUTE);
			break; 
		case REACTORP3: 
		case REACTORSCALE: 
		case REACTORPOS: 
			this->GetValue(t, &(reaction[num].pstate), FOREVER, CTRL_ABSOLUTE);
			break; 
		default: return false;
	}
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
	return true; 
}


//Overload used by Float reactors
BOOL Reactor::setState(int num, float val)
{
	if (type != REACTORFLOAT) return false;
	if(getReactionCount() == 0 || num < 0 || num >= getReactionCount()) 
		if ( num == -1 ) num = selected;
		else return false;
	
	reaction[num].fstate = val;

	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
	UpdateCurves();
	return true; 
}

//Overload used by Point3 reactors
BOOL Reactor::setState(int num, Point3 val)
{
	if (type != REACTORP3 && type != REACTORSCALE && type != REACTORPOS) return false;
	if(getReactionCount() == 0 || num < 0 || num >= getReactionCount()) 
		if ( num == -1 ) num = selected;
		else return false;
	
	reaction[num].pstate = val;

	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
	UpdateCurves();
	return true; 
}
		
	

//Overload used by Rotation reactors
BOOL Reactor::setState(int num, Quat val)
{
	if (type != REACTORROT) return false;
	if(getReactionCount() == 0 || num < 0 || num >= getReactionCount()) 
		if ( num == -1 ) num = selected;
		else return false;
	
	reaction[num].qstate = val;

	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
	return true; 
}

void* Reactor::getState(int num)
{
	if(getReactionCount() > 0 && num >= 0 && num < getReactionCount())
		switch (type)
		{
			case REACTORFLOAT: 
				return &reaction[num].fstate; 
			case REACTORROT: 
				return &reaction[num].qstate;
			case REACTORP3: 
			case REACTORSCALE: 
			case REACTORPOS: 
				return &reaction[num].pstate;

		}
	return NULL;
}

FPValue Reactor::fpGetState(int index)
{
	FPValue val = FPValue();

	if(getReactionCount() > 0 && index >= 0 && index < getReactionCount())
	{
		switch (type)
		{
			case REACTORFLOAT: 
				val.type = (ParamType2)TYPE_FLOAT;
				val.f = reaction[index].fstate;
				break;
			case REACTORROT: 
				val.type = (ParamType2)TYPE_QUAT;
				val.q = &reaction[index].qstate;
				break;
			case REACTORP3: 
			case REACTORSCALE: 
			case REACTORPOS: 
				val.type = (ParamType2)TYPE_POINT3;
				val.p = &reaction[index].pstate;
				break;
			default: assert(0);
		}
	}else throw MAXException("the index is not within the valid range");
	return val;
}

void Reactor::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
}

void Reactor::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
{
}

TSTR Reactor::SvGetRelTip(IGraphObjectManager *gom, IGraphNode *gNodeTarger, int id, IGraphNode *gNodeMaker)
{
	return SvGetName(gom, gNodeMaker, false) + " -> " + gNodeTarger->GetAnim()->SvGetName(gom, gNodeTarger, false);
}

bool Reactor::SvHandleRelDoubleClick(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker)
{
	return Control::SvHandleDoubleClick(gom, gNodeMaker);
}

SvGraphNodeReference Reactor::SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags)
{
	SvGraphNodeReference nodeRef = Control::SvTraverseAnimGraph( gom, owner, id, flags );

	if( nodeRef.stat == SVT_PROCEED ) {
		if( vrefs.client != NULL )
			gom->AddRelationship( nodeRef.gNode, vrefs.client, 0, RELTYPE_CONTROLLER );
	}

	return nodeRef;
}

void Reactor::GetAbsoluteControlValue(
		INode *node,TimeValue t,void *pt,Interval &iv)
{
	if (node != NULL)
	{
		if (isBiped)
		{
			Matrix3 cur_mat = node->GetNodeTM(t,&iv);
			Matrix3 par_mat =  node->GetParentTM(t);
			Matrix3 relative_matrix = cur_mat * Inverse( par_mat);
			Quat q = Quat(relative_matrix);
			*(Quat*)pt = q;
		}else {	
			Matrix3 tm = node->GetNodeTM(t,&iv);
			*(Point3*)pt = tm.GetTrans();
		}
	}
}

void Reactor::SortReactions()
{
	if (reaction.Count())
	{
		SVar curReaction = reaction[selected];
		reaction.Sort(CompareReactions);
		for (int i = 0; i<reaction.Count();i++)
		{
			if (reaction[i].name == curReaction.name && reaction[i].fvalue == curReaction.fvalue)
			{	
				if (getSelected() != i) setSelected(i);
				return;
			}
		}
	}
}

//--------------------------------------------------------------------

BOOL Reactor::ChangeParents(TimeValue t,const Matrix3& oldP,const Matrix3& newP,const Matrix3& tm)
	{
		HoldAll();
		// Position and rotation controllers need their path counter rotated to
		// account for the new parent.
		Matrix3 rel = oldP * Inverse(newP);
		// Modify the controllers current value (the controllers cache)
		*((Point3*)(&curpval)) = *((Point3*)(&curpval)) * rel;
		*((Quat*)(&curqval)) = *((Quat*)(&curqval)) * rel;

		//Modify each reaction state 
		for (int i=0;i<count;i++)
		{
			*((Point3*)(&reaction[i].pstate)) = *((Point3*)(&reaction[i].pstate)) * rel;
			*((Quat*)(&reaction[i].qstate)) = *((Quat*)(&reaction[i].qstate)) * rel;
		}
		ivalid.SetEmpty();
		return TRUE;
	}

void Reactor::Update(TimeValue t)
{
	if (!ivalid.InInterval(t))
	{
		ivalid = FOREVER;		
		if (vrefs.client!=NULL)
		{
			float f;
			Quat q;
			Point3 p;
			ScaleValue s;
			//update the validity interval
			if (vrefs.subnum < 0 )
			{
				if (isBiped) GetAbsoluteControlValue(vrefs.client, t, &q, ivalid);
					else GetAbsoluteControlValue(vrefs.client, t, &p, ivalid);
			}else if (GetControlInterface(vrefs.client->SubAnim(vrefs.subnum))) {
				switch (rtype)
				{
				case FLOAT_VAR:
					GetControlInterface(vrefs.client->SubAnim(vrefs.subnum))->GetValue(t, &f, ivalid);
					break;
				case VECTOR_VAR:
					GetControlInterface(vrefs.client->SubAnim(vrefs.subnum))->GetValue(t, &p, ivalid);
					break;
				case SCALE_VAR:
					GetControlInterface(vrefs.client->SubAnim(vrefs.subnum))->GetValue(t, &s, ivalid);
					break;
				case QUAT_VAR:
					GetControlInterface(vrefs.client->SubAnim(vrefs.subnum))->GetValue(t, &q, ivalid);
					break;
				}
			}
			curfval = reaction[selected].fstate;
			curpval = reaction[selected].pstate;
			curqval = reaction[selected].qstate;
		}
	}
}


void Reactor::SetValue(TimeValue t, void *val, int commit, GetSetMethod method)
{
	if (editing && count) {
		if (!TestAFlag(A_SET)) {				
			HoldTrack();
			tmpStore.PutBytes(sizeof(Point3),&curpval,this);
			SetAFlag(A_SET);
			}
		if (method == CTRL_RELATIVE) curpval += *((Point3*)val);
		else curpval = *((Point3*)val);

		ivalid.SetInstant(t);	
		if (commit) CommitValue(t);
		if (!commit) NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		//UpdateCurve();
	}
}


void Reactor::CommitValue(TimeValue t) {
	if (TestAFlag(A_SET)) {		
		if (ivalid.InInterval(t)) {

			Point3 old;
			tmpStore.GetBytes(sizeof(Point3),&old,this);					
			reaction[selected].pstate = curpval;
			if (rtype == FLOAT_VAR) UpdateCurves();

			tmpStore.Clear(this);
			ivalid.SetEmpty();
			NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
		ClearAFlag(A_SET);
	}
}

void Reactor::RestoreValue(TimeValue t) 
	{
	if (TestAFlag(A_SET)) {
		if (count) {
			tmpStore.GetBytes(sizeof(Point3),&curpval,this);
			reaction[selected].pstate = curpval;
			tmpStore.Clear(this);
			ivalid.SetInstant(t);
			}
		ClearAFlag(A_SET);
		}
	}



//------------------------------------------------------------

RefTargetHandle FloatReactor::Clone(RemapDir& remap)
	{
	// make a new reactor controller and give it our param values.
	FloatReactor *cont = new FloatReactor(FALSE);
	cont->assignReactObj(vrefs.client, vrefs.subnum);
	*cont = *this;
	if (!iCCtrl)
		cont->BuildCurves();	
	else 	
		cont->ReplaceReference(1, iCCtrl->Clone(remap));
	CloneControl(cont,remap);
	BaseClone(this, cont, remap);
	return cont;
	}

void FloatReactor::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
{
	float f[3];
	Update(t);
	valid &= ivalid;  
	if (!editing)
	{
		if (rtype == FLOAT_VAR && isCurveControlled)
		{
			if (reaction.Count() > 1)
			{
				GetCurveValue(t, f, valid);
				curfval = f[0];
			}
		}
		else{
			ComputeMultiplier(t);
			//sum up all the weighted states
			float ray = 0.0f;
			for(int i=0;i<count;i++)
				ray +=((reaction[i].fstate)*reaction[i].multiplier);
			if (count) curfval = ray; 
		}
	}

	if (method==CTRL_RELATIVE) {
		*((float*)val) += curfval;
	} else {
		*((float*)val) = curfval;
	}
		
}


//--------------------------------------------------------------------------

RefTargetHandle PositionReactor::Clone(RemapDir& remap)
{
	// make a new reactor controller and give it our param values.
	PositionReactor *cont = new PositionReactor(FALSE);
	cont->assignReactObj(vrefs.client, vrefs.subnum);
	*cont = *this;
	if (!iCCtrl)
		cont->BuildCurves();	
	else 	
		cont->ReplaceReference(1, iCCtrl->Clone(remap));
	CloneControl(cont,remap);
	BaseClone(this, cont, remap);
	return cont;
}

void PositionReactor::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
{
	Point3 ray = Point3(0,0,0);
	float f[3];
	Update(t);
	valid &= ivalid;  
	if (!editing && count)
	{
		if (rtype == FLOAT_VAR && isCurveControlled)
		{
			GetCurveValue(t, f, valid);
			for (int x=0;x<3;x++)
			{
				curpval[x] = f[x];
			}
		}
		else{
			ComputeMultiplier(t);
			//sum up all the weighted states
			for(int i=0;i<count;i++)
				ray +=((reaction[i].pstate)*reaction[i].multiplier);
			curpval = ray;  
		}
	}

	if (method==CTRL_RELATIVE) {
  		Matrix3 *mat = (Matrix3*)val;	
		mat->PreTranslate(curpval);
	} else {
		*((Point3*)val) = curpval;
	}
}
//---------------------------------------------------------

RefTargetHandle Point3Reactor::Clone(RemapDir& remap)
	{
	// make a new reactor controller and give it our param values.
	Point3Reactor *cont = new Point3Reactor(FALSE);
	cont->assignReactObj(vrefs.client, vrefs.subnum);
	*cont = *this;	
	if (!iCCtrl)
		cont->BuildCurves();	
	else 	
		cont->ReplaceReference(1, iCCtrl->Clone(remap));
	CloneControl(cont,remap);
	BaseClone(this, cont, remap);
	return cont;
	}

void Point3Reactor::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
	float f[3];
	Update(t);
	valid &= ivalid;  
	if (!editing && count)
	{
		if (rtype == FLOAT_VAR && isCurveControlled)
		{
			GetCurveValue(t, f, valid);
			for (int x=0;x<3;x++)
			{
				curpval[x] = f[x];
			}
		}
		else{
			ComputeMultiplier(t);
			//sum up all the weighted states
			Point3 ray = Point3(0,0,0);
			for(int i=0;i<count;i++)
				ray +=((reaction[i].pstate)*reaction[i].multiplier);
			curpval = ray;  
		}
	}

	if (method==CTRL_RELATIVE) {
		*((Point3*)val) += curpval;
	} else {
		*((Point3*)val) = curpval;
	}
}



//--------------------------------------------------------------------------

RefTargetHandle ScaleReactor::Clone(RemapDir& remap)
	{
	// make a new reactor controller and give it our param values.
	ScaleReactor *cont = new ScaleReactor(FALSE);
	cont->assignReactObj(vrefs.client, vrefs.subnum);
	*cont = *this;
	if (!iCCtrl)
		cont->BuildCurves();	
	else 	
		cont->ReplaceReference(1, iCCtrl->Clone(remap));
	CloneControl(cont,remap);
	BaseClone(this, cont, remap);
	return cont;
	}

void ScaleReactor::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
{
	float f[3];
	Update(t);
	valid &= ivalid;  
	if (!editing && count)
	{
		if (rtype == FLOAT_VAR && isCurveControlled)
		{
			GetCurveValue(t, f, valid);
			for (int x=0;x<3;x++)
			{
				curpval[x] = f[x];
			}
		}
		else{
			ComputeMultiplier(t);
			//sum up all the weighted states
			Point3 ray = Point3(0,0,0);
			for(int i=0;i<count;i++)
				ray +=((reaction[i].pstate)*reaction[i].multiplier);
			curpval = ray;  
		}
	}

	if (method==CTRL_RELATIVE) {
  		Matrix3 *mat = (Matrix3*)val;
		ApplyScaling(*mat, curpval);
	} else {
		*((Point3*)val) = curpval;
	}
}


void ScaleReactor::SetValue(TimeValue t, void *val, int commit, GetSetMethod method)
{
	if (editing && count) {
		if (!TestAFlag(A_SET)) {				
			HoldTrack();
			tmpStore.PutBytes(sizeof(Point3),&curpval,this);
			SetAFlag(A_SET);
			}
		if (method == CTRL_RELATIVE) curpval *= *((Point3*)val);
		else curpval = *((Point3*)val);

		ivalid.SetInstant(t);	
		if (commit) CommitValue(t);
		if (!commit) NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}
}


//------------------------------------------------------------------

RefTargetHandle RotationReactor::Clone(RemapDir& remap)
	{
	// make a new reactor controller and give it our param values.
	RotationReactor *cont = new RotationReactor(FALSE);
	cont->assignReactObj(vrefs.client, vrefs.subnum);
	*cont = *this;	
	if (!iCCtrl)
		cont->BuildCurves();	
	else 	
		cont->ReplaceReference(1, iCCtrl->Clone(remap));
	CloneControl(cont,remap);
	BaseClone(this, cont, remap);
	return cont;
	}

void RotationReactor::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
{
	float eulr[3];
	Quat ray;
	Update(t);
	valid &= ivalid;  
	if (!editing && count)
	{
		if (rtype == FLOAT_VAR && isCurveControlled)
		{
			GetCurveValue(t, eulr, valid);
			for (int p = 0;p<3;p++)
			{
				eulr[p] = DegToRad(eulr[p]);
			}
			EulerToQuat(eulr, curqval);
		}
		else{
			ComputeMultiplier(t);
			//sum up all the weighted states
			curqval.Identity();
			
			for(int i=0;i<count;i++)
			{
				ray = reaction[i].qstate;
				QuatToEuler(ray, eulr);
				eulr[0] *= reaction[i].multiplier;
				eulr[1] *= reaction[i].multiplier;
				eulr[2] *= reaction[i].multiplier;
				EulerToQuat(eulr, ray);

				curqval += ray;
			}
		}
	}

	if (method==CTRL_RELATIVE) {
  	Matrix3 *mat = (Matrix3*)val;	
	PreRotateMatrix(*mat, curqval);
	} else {
		*((Quat*)val) = curqval;
	}
		
}

void RotationReactor::SetValue(TimeValue t, void *val, int commit, GetSetMethod method)
{
	if (editing && count) {
		if (!TestAFlag(A_SET)) {				
			HoldTrack();
			tmpStore.PutBytes(sizeof(Quat),&curqval,this);
			SetAFlag(A_SET);
			}

		if (method == CTRL_RELATIVE) curqval *= Quat(*((AngAxis*)val));
		else curqval = Quat(*((AngAxis*)val));

		ivalid.SetInstant(t);	
		if (commit) CommitValue(t);
		if (!commit) NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}
}


void RotationReactor::CommitValue(TimeValue t) {
	if (TestAFlag(A_SET)) {		
		if (ivalid.InInterval(t)) {

			Quat old;
			tmpStore.GetBytes(sizeof(Quat),&old,this);					
			reaction[selected].qstate = curqval;
			if (rtype == FLOAT_VAR) UpdateCurves();

			tmpStore.Clear(this);
			ivalid.SetEmpty();
			NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
		ClearAFlag(A_SET);
	}
}

void RotationReactor::RestoreValue(TimeValue t) 
	{
	if (TestAFlag(A_SET)) {
		if (count) {
			tmpStore.GetBytes(sizeof(Quat),&curqval,this);
			reaction[selected].qstate = curqval;
			tmpStore.Clear(this);
			ivalid.SetInstant(t);
			}
		ClearAFlag(A_SET);
		}
	}


//--------------------------------------------------------------------------

class ReactionFilter : public TrackViewFilter {
public:
	BOOL proc(Animatable *anim, Animatable *client, int subNum)
	{ 
		
		if (anim->SuperClassID() == BASENODE_CLASS_ID) {
			INode *node = (INode*)anim;
			return !node->IsRootNode();
			}

		return anim->SuperClassID() == CTRL_FLOAT_CLASS_ID ||
			anim->SuperClassID() == CTRL_POSITION_CLASS_ID ||
			anim->SuperClassID() == CTRL_POINT3_CLASS_ID ||
			anim->SuperClassID() == CTRL_SCALE_CLASS_ID ||
			anim->SuperClassID() == CTRL_ROTATION_CLASS_ID || 
			anim->SuperClassID() == BASENODE_CLASS_ID; 
	}

};

//**************************************************************
//--------------------------------------------------------------
// UI Stuff
//--------------------------------------------------------------
//**************************************************************
//This should be called when the contents of the reaction list changes
void ReactorDlg::UpdateVarList()
{
	int i, ct;

	if (updateFlags & REACTORDLG_LIST_CHANGED_SIZE)
			
	{
		//SendMessage(hWndList, WM_SETREDRAW, FALSE, 0L);
		SendDlgItemMessage(hWnd, IDC_REACTION_LIST, WM_SETREDRAW, FALSE, 0L);
		SendDlgItemMessage(hWnd, IDC_REACTION_LIST, LB_SETCURSEL, cont->selected, 0);
		if (cont->selected >= cont->getReactionCount() ) cont->selected -=1;
		ct = cont->getReactionCount();
		SendDlgItemMessage(hWnd, IDC_REACTION_LIST, LB_RESETCONTENT, 0, 0);
		for(i = 0; i < ct; i++)
		{
			SendDlgItemMessage(hWnd, IDC_REACTION_LIST, LB_ADDSTRING, 0, (LPARAM)cont->getReactionName(i));
		}
		SendDlgItemMessage(hWnd, IDC_REACTION_LIST, LB_SETCURSEL, cont->selected, 0);
		SendDlgItemMessage(hWnd, IDC_REACTION_LIST, WM_SETREDRAW, TRUE, 0L);
		InvalidateRect(GetDlgItem(hWnd, IDC_REACTION_LIST), NULL, TRUE);

		if (cont->count > 1) iDeleteBut->Enable(TRUE);
			else iDeleteBut->Enable(FALSE);
	}


	if (updateFlags & (REACTORDLG_LIST_SELECTION_CHANGED|REACTORDLG_LIST_CHANGED_SIZE))
	{
		i = cont->getSelected();
		if(i >=0 && i < cont->getReactionCount()) 
		{
			TCHAR buf[256];
			iNameEdit->GetText(buf, 256);
			if (_tcsicmp(buf, cont->getReactionName(i)) !=0 ) 
				iNameEdit->SetText(cont->reaction[i].name);
			UpdateReactionValue();
			
			if (cont->rtype != FLOAT_VAR && cont->iCCtrl)
			{
				BitArray ba;
				ba.SetSize(cont->reaction.Count());
				ba.ClearAll();
				ba.Set(cont->selected);
				//cont->iCCtrl->SetDisplayMode(ba);	
			}
			iFloatState->SetValue(cont->reaction[cont->selected].fstate, FALSE);
			iStrength->SetValue(cont->reaction[cont->selected].strength, FALSE);
			iFalloff[0]->SetValue(cont->reaction[cont->selected].falloff, FALSE);
			float inf = cont->reaction[cont->selected].influence;
			if (cont->rtype == QUAT_VAR)
				inf = RadToDeg(inf);
			iInfluence->SetValue(inf, FALSE);

		}
	}
	updateFlags &= ~REACTORDLG_LIST_CHANGED_SIZE;
	updateFlags &= ~REACTORDLG_LIST_SELECTION_CHANGED;

	//cont->ivalid.SetEmpty();
	//cont->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	//if (ip) ip->RedrawViews(ip->GetTime());
}

void ReactorDlg::UpdateReactionValue()
{
	AngAxis aa;
	float f;
	TCHAR buf[256];
	_stprintf(buf, _T(""));
	int i = cont->selected;

	if (cont->vrefs.client)
	{
		switch (cont->rtype)
		{
			case FLOAT_VAR:
				assert(SetDlgItemFloat(hWnd, IDC_VALUE_STATUS, cont->reaction[i].fvalue));
				break;
			case VECTOR_VAR:
			case SCALE_VAR:
				_stprintf(buf, _T("( %g; %g; %g )"), cont->reaction[i].pvalue[0], cont->reaction[i].pvalue[1], cont->reaction[i].pvalue[2]);
				SetDlgItemText(hWnd, IDC_VALUE_STATUS, buf);	
				break;
			case QUAT_VAR:
				aa = cont->reaction[i].qvalue;
				f = RadToDeg(aa.angle);
				_stprintf(buf, _T("%g ( %g; %g; %g )"), f, aa.axis.x, aa.axis.y, aa.axis.z);
				SetDlgItemText(hWnd, IDC_VALUE_STATUS, buf);	
				break;
		}
	}
	else SetDlgItemText(hWnd, IDC_VALUE_STATUS, buf);	
}

//------------------------------------------------------------
/*
void *DummyRefMaker::GetInterface(ULONG id)
{

	if(id == I_RESMAKER_INTERFACE)
		return (void *) (ResourceMakerCallback *) &theReactor;
	else
		return (void *) NULL;

}
*/

static INT_PTR CALLBACK ReactorDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
/*
class ReactorCtrlWindow {
	public:
		HWND hWnd;
		HWND hParent;
		Control *cont;
		ReactorCtrlWindow() {assert(0);}
		ReactorCtrlWindow(HWND hWnd,HWND hParent,Control *cont)
			{this->hWnd=hWnd; this->hParent=hParent; this->cont=cont;}
	};
static Tab<ReactorCtrlWindow> reactorCtrlWindows;

static void RegisterReactorCtrlWindow(HWND hWnd, HWND hParent, Control *cont)
	{
	ReactorCtrlWindow rec(hWnd,hParent,cont);
	reactorCtrlWindows.Append(1,&rec);
	}

static void UnRegisterReactorCtrlWindow(HWND hWnd)
	{	
	for (int i=0; i<reactorCtrlWindows.Count(); i++) {
		if (hWnd==reactorCtrlWindows[i].hWnd) {
			reactorCtrlWindows.Delete(i,1);
			return;
			}
		}	
	}

static HWND FindOpenReactorCtrlWindow(HWND hParent,Control *cont)
	{	
	for (int i=0; i<reactorCtrlWindows.Count(); i++) {
		if (hParent == reactorCtrlWindows[i].hParent &&
			cont    == reactorCtrlWindows[i].cont) {
			return reactorCtrlWindows[i].hWnd;
			}
		}
	return NULL;
	}

*/

ReactorDlg::ReactorDlg(Reactor *cont, ParamDimensionBase *dim, TCHAR *pname,
						IObjParam *ip, HWND hParent)
{
	this->cont = cont;
	this->ip   = ip;
	this->dim  = dim;
	valid = FALSE;
	elems = cont->Elems();
	MakeRefByID(FOREVER,0,cont);
	updateFlags = (REACTORDLG_LIST_CHANGED_SIZE|REACTORDLG_LIST_SELECTION_CHANGED);

	hWnd = CreateDialogParam(
		hInstance,
		MAKEINTRESOURCE(IDD_REACTOR_PARAMS),
		hParent,
		ReactorDlgProc,
		(LPARAM)this);	
	TSTR title = TSTR(GetString(IDS_AF_REACTORTITLE)) + TSTR(pname);
	SetWindowText(hWnd,title);
	ip->RegisterDlgWnd(hWnd);
	ip->RegisterTimeChangeCallback(this);
	reactorActionCB = new ReactorActionCB<Reactor>(cont);
	ip->GetActionManager()->ActivateActionTable(reactorActionCB, kReactorActions);
}

ReactorDlg::~ReactorDlg()
	{
	ip->GetActionManager()->DeactivateActionTable(reactorActionCB, kReactorActions);
	delete reactorActionCB;

	ip->UnRegisterDlgWnd(hWnd);
	//UnRegisterReactorCtrlWindow(hWnd);
	ip->UnRegisterTimeChangeCallback(this);
	//cont->hParams = NULL;
	cont->ip = NULL;
	cont->dlg = NULL;
	DeleteAllRefsFromMe();

	ReleaseISpinner(iFloatState);
	ReleaseISpinner(iInfluence);
	ReleaseISpinner(iStrength);
	ReleaseISpinner(iFalloff[0]);
	ReleaseICustEdit(iNameEdit);
	ReleaseICustEdit(iValueStatus);
	ReleaseICustButton(iReactToBut);
	ReleaseICustButton(iCreateBut);
	ReleaseICustButton(iDeleteBut);
	ReleaseICustButton(iSetBut);
	ReleaseICustButton(iEditBut);
	//iCCtrl = NULL;
	}

void ReactorDlg::Invalidate()
	{
	valid = FALSE;
	InvalidateRect(hWnd,NULL,FALSE);
	}

void ReactorDlg::UpdateNodeName()
	{
		TSTR nname, pname;
		pname = "";
		
		if (cont->vrefs.client)
		{

			if(cont->vrefs.subnum < 0)	// special case: we're referencing a node
				pname = ((INode *)cont->vrefs.client)->GetName();
			else {
				cont->getNodeName(cont->vrefs.client,nname);
				if (nname.Length())
					pname = nname + TSTR(_T("\\")) + cont->vrefs.client->SubAnimName(cont->vrefs.subnum);
				else 
					pname = cont->vrefs.client->SubAnimName(cont->vrefs.subnum);
			}
			assert(SetDlgItemText(hWnd, IDC_TRACK_NAME, pname));
		} else{
			assert(SetDlgItemText(hWnd, IDC_TRACK_NAME, pname));
		}
	}

void ReactorDlg::Update()
{
	TCHAR buf[256];
	valid = TRUE;

	if (cont->vrefs.client && cont->count) {

		float fval, f;
		Point3 pval;
		Quat qval;
		AngAxis aa;
		ScaleValue sval;
		Control *c;

		if (!iCreateBut->IsEnabled())
		{
			EnableWindow(GetDlgItem(hWnd, IDC_USE_CURVE), true);
			ICustButton *iCurveBut = GetICustButton(GetDlgItem(hWnd,IDC_CURVE_BUTTON));
			iCurveBut->Enable(TRUE);
			ReleaseICustButton(iCurveBut);

			iFloatState->Enable(TRUE);
			if (!iInfluence->IsEnabled()) iInfluence->Enable(TRUE);
			iFalloff[0]->Enable(TRUE);
			if (!iStrength->IsEnabled()) iStrength->Enable(TRUE);
			iCreateBut->Enable(TRUE);
			//if (cont->count > 1) iDeleteBut->Enable(TRUE);
				//else iDeleteBut->Enable(FALSE);
			iSetBut->Enable(TRUE);
			iEditBut->Enable(TRUE);
		}

		iEditBut->SetCheck(cont->editing);

		//Update the current React To object value field
		if (cont->vrefs.subnum < 0 )
		{
			if (cont->isBiped)	
			{
				cont->GetAbsoluteControlValue(cont->vrefs.client, ip->GetTime(), &qval, FOREVER);
					aa = qval;
					f = RadToDeg(aa.angle);
					_stprintf(buf, _T("%g ( %g; %g; %g )"), f, aa.axis.x, aa.axis.y, aa.axis.z);
			}
			else {
				cont->GetAbsoluteControlValue(cont->vrefs.client, ip->GetTime(), &pval, FOREVER);
				_stprintf(buf, _T("( %g; %g; %g )"), pval.x, pval.y, pval.z);
			}
		}
		else {
			if (c = GetControlInterface(cont->vrefs.client->SubAnim(cont->vrefs.subnum))) {
				switch (cont->rtype)
				{
					case FLOAT_VAR:
						c->GetValue(ip->GetTime(), &fval, FOREVER);
						_stprintf(buf, _T("%g"), fval);
						cont->iCCtrl->SetCurrentXValue(fval);
						//assert(SetDlgItemFloat(hWnd, IDC_TRACK_VALUE, fval));
						break;
					case VECTOR_VAR:
						c->GetValue(ip->GetTime(), &pval, FOREVER);
						_stprintf(buf, _T("( %g; %g; %g )"), pval.x, pval.y, pval.z);
						break;
					case SCALE_VAR:
						c->GetValue(ip->GetTime(), &sval, FOREVER);
						_stprintf(buf, _T("( %g; %g; %g )"), sval.s.x, sval.s.y, sval.s.z);
						break;
					case QUAT_VAR:
						c->GetValue(ip->GetTime(), &qval, FOREVER);
						aa = qval;
						f = RadToDeg(aa.angle);
						_stprintf(buf, _T("%g ( %g; %g; %g )"), f, aa.axis.x, aa.axis.y, aa.axis.z);
						break;
					default: _stprintf(buf, _T(""));
				}
			}
		}
		SetDlgItemText(hWnd, IDC_TRACK_VALUE, buf);	
		
		//update the current output field
		switch (cont->type)
		{
			case REACTORFLOAT:
				assert(SetDlgItemFloat(hWnd, IDC_OUTPUT_STATUS, cont->curfval));
				break;
			case REACTORPOS:
			case REACTORP3:
			case REACTORSCALE:
				_stprintf(buf, _T("( %g; %g; %g )"), cont->curpval.x, cont->curpval.y, cont->curpval.z);
				SetDlgItemText(hWnd, IDC_OUTPUT_STATUS, buf);	
				break;
			case REACTORROT:
				aa = cont->curqval;
				f = RadToDeg(aa.angle);
				_stprintf(buf, _T("%g ( %g; %g; %g )"), f, aa.axis.x, aa.axis.y, aa.axis.z);
				SetDlgItemText(hWnd, IDC_OUTPUT_STATUS, buf);	
				break;
		}		
	} 	
	else{
		//disable all buttons if count = 0
		_stprintf(buf, _T(""));
		iFloatState->Enable(FALSE);
		iInfluence->Enable(FALSE);
		iFalloff[0]->Enable(FALSE);
		iStrength->Enable(FALSE);
		iCreateBut->Enable(FALSE);
		iDeleteBut->Enable(FALSE);
		iSetBut->Enable(FALSE);
		iEditBut->Enable(FALSE);
		UpdateNodeName();
		UpdateVarList();
		UpdateReactionValue();
		SetDlgItemText(hWnd, IDC_TRACK_VALUE, buf);	
		SetDlgItemText(hWnd, IDC_OUTPUT_STATUS, buf);
		iNameEdit->SetText("");
		
		EnableWindow(GetDlgItem(hWnd, IDC_USE_CURVE), false);
		ICustButton *iCurveBut = GetICustButton(GetDlgItem(hWnd,IDC_CURVE_BUTTON));
		iCurveBut->Enable(FALSE);
		ReleaseICustButton(iCurveBut);

		SendDlgItemMessage(hWnd, IDC_REACTION_LIST, LB_RESETCONTENT, 0, 0);

	}
	if (updateFlags & (REACTORDLG_LIST_SELECTION_CHANGED|REACTORDLG_LIST_CHANGED_SIZE)) 
	{
		UpdateVarList();
		if(!(cont->flags&REACTOR_BLOCK_CURVE_UPDATE))
		{
			cont->UpdateCurves(false);
		}
	}

	UpdateNodeName();
	updateFlags &= ~REACTORDLG_LIST_SELECTION_CHANGED;
	updateFlags &= ~REACTORDLG_LIST_CHANGED_SIZE;
}


void ReactorDlg::SetupUI(HWND hWnd)
{
	this->hWnd = hWnd;

	iFloatState = GetISpinner(GetDlgItem(hWnd,IDC_FLOATSTATE_SPIN));
	iFloatState ->SetLimits(-99999,99999,FALSE);
	iFloatState ->SetAutoScale();
	iFloatState ->LinkToEdit(GetDlgItem(hWnd,IDC_FLOATSTATE_EDIT),EDITTYPE_FLOAT);	

	iStrength = GetISpinner(GetDlgItem(hWnd,IDC_STRENGTH_SPIN));
	iStrength ->SetLimits(0,99999,FALSE);
	iStrength ->SetAutoScale();
	iStrength ->LinkToEdit(GetDlgItem(hWnd,IDC_STRENGTH_EDIT),EDITTYPE_FLOAT);	

	iFalloff[0] = GetISpinner(GetDlgItem(hWnd,IDC_FALLOFF_SPIN));
	iFalloff[0] ->SetLimits(0.001f,10.0f,FALSE);
	iFalloff[0] ->SetAutoScale();
	iFalloff[0] ->LinkToEdit(GetDlgItem(hWnd,IDC_FALLOFF_EDIT),EDITTYPE_FLOAT);	

	iInfluence = GetISpinner(GetDlgItem(hWnd,IDC_INFLUENCE_SPIN));
	iInfluence ->SetLimits(0.0f,99999.0f,FALSE);
	iInfluence ->SetAutoScale();
	iInfluence ->LinkToEdit(GetDlgItem(hWnd,IDC_INFLUENCE_EDIT),EDITTYPE_FLOAT);	

	iNameEdit = GetICustEdit(GetDlgItem(hWnd,IDC_NAME_EDIT));
	iValueStatus = GetICustEdit(GetDlgItem(hWnd,IDC_VALUE_STATUS));
	
	iReactToBut = GetICustButton(GetDlgItem(hWnd,IDC_PICK_BUTTON));
	iReactToBut->SetType(CBT_CHECK);
	iReactToBut->SetHighlightColor(GREEN_WASH);
	iReactToBut->SetTooltip(TRUE, GetString(IDS_ASSIGN_TO));

	iCreateBut = GetICustButton(GetDlgItem(hWnd,IDC_CREATE_BUTTON));
	iCreateBut->SetType(CBT_PUSH);

	iDeleteBut = GetICustButton(GetDlgItem(hWnd,IDC_DELETE_BUTTON));
	iDeleteBut->SetType(CBT_PUSH);

	iSetBut = GetICustButton(GetDlgItem(hWnd,IDC_SET_BUTTON));
	iSetBut->SetType(CBT_PUSH);

	iEditBut = GetICustButton(GetDlgItem(hWnd,IDC_EDIT_BUTTON));
	iEditBut->SetType(CBT_CHECK);
	iEditBut->SetHighlightColor(GREEN_WASH);

	if (cont->ClassID() == REACTORFLOAT_CLASS_ID){
		ShowWindow(GetDlgItem(hWnd, IDC_EDIT_BUTTON), SW_HIDE);
	}
	else{
		ShowWindow(GetDlgItem(hWnd, IDC_FLOATSTATE_SPIN), SW_HIDE);
		ShowWindow(GetDlgItem(hWnd, IDC_FLOATSTATE_EDIT), SW_HIDE);
		ShowWindow(GetDlgItem(hWnd, IDC_EDITSTATE_STATIC), SW_HIDE);
	}
	
	SendDlgItemMessage(hWnd, IDC_USE_CURVE, BM_SETCHECK, cont->isCurveControlled,0);
	SetupFalloffUI();

	
	if (!cont->reaction.Count())
	{
		iFloatState->Enable(FALSE);
		iInfluence->Enable(FALSE);
		iFalloff[0]->Enable(FALSE);
		iStrength->Enable(FALSE);
		iCreateBut->Enable(FALSE);
		iDeleteBut->Enable(FALSE);
		iSetBut->Enable(FALSE);
		iEditBut->Enable(FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_USE_CURVE), false);
		
		ICustButton *iCurveBut = GetICustButton(GetDlgItem(hWnd,IDC_CURVE_BUTTON));
		iCurveBut->Enable(FALSE);
		ReleaseICustButton(iCurveBut);
	}
	valid = FALSE;
}

void ReactorDlg::SetupFalloffUI()
{
	if (cont->isCurveControlled)
	{
		//show the button and hide the spinner
		ShowWindow(GetDlgItem(hWnd, IDC_CURVE_BUTTON), SW_SHOWNORMAL);
		ShowWindow(GetDlgItem(hWnd, IDC_FALLOFF_SPIN), SW_HIDE);
		ShowWindow(GetDlgItem(hWnd, IDC_FALLOFF_EDIT), SW_HIDE);
		
		if (cont->rtype == FLOAT_VAR && iInfluence->IsEnabled()){
			iInfluence->Disable();
			iStrength->Disable();
		}
	}
	else{
		//hide the button and show the spinner
		ShowWindow(GetDlgItem(hWnd, IDC_CURVE_BUTTON), SW_HIDE);
		ShowWindow(GetDlgItem(hWnd, IDC_FALLOFF_SPIN), SW_SHOWNORMAL);
		ShowWindow(GetDlgItem(hWnd, IDC_FALLOFF_EDIT), SW_SHOWNORMAL);

		if (cont->rtype == FLOAT_VAR && !iInfluence->IsEnabled()){
			iInfluence->Enable(TRUE);
			iStrength->Enable(TRUE);
		}
	}
}

void ReactorDlg::WMCommand(int id, int notify, HWND hCtrl)
	{
		Point3 pt;
		int i;
		TCHAR buf[256];
		//TrackViewPick res;
		//ReactionFilter rf;

		switch (id) {
			case IDC_PICK_BUTTON:
				//if(cont->ip->TrackViewPickDlg(hWnd, &res, &rf)) {
				//cont->reactTo(res.anim);
				ip->ClearPickMode();
				if (iReactToBut->IsChecked())
				{
					if (!cont->pickReactToMode) cont->pickReactToMode = new PickReactToMode(cont);
					ip->SetPickMode(cont->pickReactToMode);
				}
				break;
			case IDC_CREATE_BUTTON:
					if(cont->vrefs.client != NULL)
						cont->CreateReaction();
				break;
			case IDC_REACTION_LIST:
				if(notify == LBN_SELCHANGE) {
					cont->setSelected(SendDlgItemMessage(hWnd, IDC_REACTION_LIST, LB_GETCURSEL, 0, 0));
					if (cont->rtype != FLOAT_VAR && cont->iCCtrl)
					{
						BitArray ba;
						ba.SetSize(cont->reaction.Count());
						ba.ClearAll();
						ba.Set(cont->selected);
						cont->iCCtrl->SetDisplayMode(ba);	
					}

				}break;
			case IDC_DELETE_BUTTON:
				cont->DeleteReaction();
				break;
			case IDC_SET_BUTTON:
				if(cont->vrefs.client != NULL)
				{
					cont->setReactionValue(cont->selected);
					//experimental code to see if people like this better
					cont->setMinInfluence(cont->selected);
				}
				break;
			case IDC_EDIT_BUTTON:
				cont->setEditReactionMode(iEditBut->IsChecked());
				if (cont->editing)
				{
					switch (cont->type)
					{
						case REACTORPOS:
						case REACTORP3:
							cont->ip->SetStdCommandMode(CID_OBJMOVE);
							break;
						case REACTORROT:
							cont->ip->SetStdCommandMode(CID_OBJROTATE);
							break;
						case REACTORSCALE:
							cont->ip->SetStdCommandMode(CID_OBJSCALE);
							break;
						default:
							break;
					}
				}
				break;
			case IDC_CURVE_BUTTON:
				cont->ShowCurveControl();
				break;
			case IDC_USE_CURVE:
				cont->isCurveControlled = SendDlgItemMessage(hWnd, IDC_USE_CURVE, BM_GETCHECK, 0,0);
				SetupFalloffUI();
				cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
				cont->ip->RedrawViews(cont->ip->GetTime());
				break;
			case IDC_NAME_EDIT:
				i = SendDlgItemMessage(hWnd, IDC_REACTION_LIST, LB_GETCURSEL, 0, 0);
				if (i>=0)
				{
					iNameEdit->GetText(buf, 256);
					cont->setReactionName(cont->selected, buf);
				}
				break;

			default: break;
		}
	}

void ReactorDlg::SpinnerChange(int id,BOOL drag)
	{

	float f;

	if (!drag)
		if (!theHold.Holding()) {
		SpinnerStart(id);
		}
	
	switch (id) {
		case IDC_FALLOFF_SPIN:
			cont->setFalloff(cont->selected, (float)iFalloff[0]->GetFVal());
			Change(FALSE);
			break;
		case IDC_INFLUENCE_SPIN:

			f = iInfluence->GetFVal();
			if (cont->rtype == QUAT_VAR) { f = DegToRad(f); } 
			
			cont->setInfluence(cont->selected, f );
			Change(FALSE);
			break;


		case IDC_STRENGTH_SPIN:
			cont->setStrength(cont->selected, (float)iStrength->GetFVal());
			Change(FALSE);
			break;

		case IDC_FLOATSTATE_SPIN:
			cont->HoldParams();
			cont->reaction[cont->selected].fstate = (float)iFloatState->GetFVal();
			cont->UpdateCurves();
			Change(TRUE);
			break;
		}
		
	}

void ReactorDlg::SpinnerStart(int id)
	{
	switch (id) {
		case IDC_FLOATSTATE_SPIN:
		case IDC_STRENGTH_SPIN:
		case IDC_INFLUENCE_SPIN:
		case IDC_FALLOFF_SPIN:
			theHold.Begin();
			cont->HoldParams();
			break;
		}
	}

void ReactorDlg::SpinnerEnd(int id,BOOL cancel)
{
	if (cancel) {
		theHold.Cancel();
	} else {
	switch (id) {
		case IDC_FLOATSTATE_SPIN:
		case IDC_FLOATSTATE_EDIT:
			theHold.Accept(GetString(IDS_AF_CHANGESTATE));
			break;
		case IDC_STRENGTH_SPIN:
		case IDC_STRENGTH_EDIT:
			theHold.Accept(GetString(IDS_AF_CHANGESTRENGTH));
			break;
		case IDC_INFLUENCE_SPIN:
		case IDC_INFLUENCE_EDIT:
			theHold.Accept(GetString(IDS_AF_CHANGEINFLUENCE));
			break;
		case IDC_FALLOFF_SPIN:
		case IDC_FALLOFF_EDIT:
			theHold.Accept(GetString(IDS_AF_CHANGEFALLOFF));
			break;
		}
	}
	ip->RedrawViews(ip->GetTime());
}

void ReactorDlg::Change(BOOL redraw)
	{
	cont->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	UpdateWindow(GetParent(hWnd));	
	if (redraw) ip->RedrawViews(ip->GetTime());
	}


class CheckForNonReactorDlg : public DependentEnumProc {
	public:		
		BOOL non;
		ReferenceMaker *me;
		CheckForNonReactorDlg(ReferenceMaker *m) {non = FALSE;me = m;}
		int proc(ReferenceMaker *rmaker) {
			if (rmaker==me) return 0;
			if (rmaker->SuperClassID()!=REF_MAKER_CLASS_ID &&
				rmaker->ClassID()!=Class_ID(REACTORDLG_CLASS_ID,0x67053d10)) {
				non = TRUE;
				return 1;
				}
			return 0;
			}
	};
void ReactorDlg::MaybeCloseWindow()
	{
	CheckForNonReactorDlg check(cont);
	cont->EnumDependents(&check);
	if (!check.non) {
		PostMessage(hWnd,WM_CLOSE,0,0);
		}
	}

RefResult ReactorDlg::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
     	PartID& partID,  
     	RefMessage message)
	{
//	TCHAR buf[256];
//	int msg;
	switch (message) {
		case REFMSG_CHANGE:
			Invalidate();			
			break;
		case REFMSG_REACTION_COUNT_CHANGED:
			updateFlags |= REACTORDLG_LIST_CHANGED_SIZE;
		case REFMSG_REACTOR_SELECTION_CHANGED:
			updateFlags |= REACTORDLG_LIST_SELECTION_CHANGED;
			Invalidate();
			break;
		case REFMSG_NODE_NAMECHANGE:
		case REFMSG_REACTTO_OBJ_NAME_CHANGED:
			//flags &= REACTORDLG_REACTTO_NAME_CHANGED;
			UpdateNodeName();
			break;
		case REFMSG_USE_CURVE_CHANGED:
			SendDlgItemMessage(hWnd, IDC_USE_CURVE, BM_SETCHECK, cont->isCurveControlled, 0);
			SetupFalloffUI();
			break;
		case REFMSG_REACTION_NAME_CHANGED:
			/*
			msg = SendMessage(GetDlgItem(hWnd, IDC_REACTION_LIST), LB_SETITEMDATA, partID, (LPARAM)cont->getReactionName(partID));

			iNameEdit->GetText(buf, 256);
			if (_tcsicmp(buf, cont->getReactionName(cont->getSelected())) !=0 ) 
				iNameEdit->SetText( cont->getReactionName(cont->getSelected()) );
			*/
			updateFlags |= REACTORDLG_LIST_CHANGED_SIZE;
			Invalidate();
			break;
		case REFMSG_REF_DELETED:
			MaybeCloseWindow();
			break;
		}
	return REF_SUCCEED;
	}


static INT_PTR CALLBACK ReactorDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	ReactorDlg *dlg = (ReactorDlg*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	int i;
	int selCount;
//	TCHAR buf[256];
	ICurve *curve, *otherCurve;
	float eulAng[3];
	CurvePoint pt;
	int ptNum;
	switch (msg) {
		case WM_INITDIALOG:
			dlg = (ReactorDlg*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			dlg->SetupUI(hWnd);
			if (dlg->cont->vrefs.client){
				dlg->updateFlags |= REACTORDLG_LIST_CHANGED_SIZE;
				dlg->UpdateNodeName();
				dlg->Invalidate();
			}
			break;

		case CC_SPINNER_BUTTONDOWN:
			dlg->SpinnerStart(LOWORD(wParam));
			break;

		case CC_SPINNER_CHANGE:
			dlg->SpinnerChange(LOWORD(wParam),HIWORD(wParam));
			break;

		case WM_CUSTEDIT_ENTER:
			switch (LOWORD(wParam)) 
			{
				case IDC_NAME_EDIT:
					break;
				default: 
					dlg->SpinnerEnd(LOWORD(wParam),FALSE); 
					break;
			}
			break;

		case CC_SPINNER_BUTTONUP:
			dlg->SpinnerEnd(LOWORD(wParam),!HIWORD(wParam));
			break;

		case WM_COMMAND:
			dlg->WMCommand(LOWORD(wParam),HIWORD(wParam),(HWND)lParam);						
			break;
		case WM_CC_CHANGE_CURVEPT:
		case WM_CC_CHANGE_CURVETANGENT:
			
			if (dlg->cont->flags&REACTOR_BLOCK_CURVE_UPDATE) break;
			dlg->cont->flags |= REACTOR_BLOCK_CURVE_UPDATE;


			dlg->cont->iCCtrl->EnableDraw(FALSE);
			curve = ((ICurve*)lParam);
			ptNum = LOWORD(wParam);
			pt = curve->GetPoint(0,ptNum, FOREVER);
			
			theHold.Suspend();

			if ( dlg->cont->rtype == FLOAT_VAR) 
			{
				dlg->cont->reaction[ptNum].fvalue = pt.p.x;
				
				int curveNum;
				for (curveNum=0;curveNum<dlg->cont->iCCtrl->GetNumCurves();curveNum++)
				{
					if (curve == dlg->cont->iCCtrl->GetControlCurve(curveNum))
						break;
				}

				switch (dlg->cont->type)
				{
					case REACTORFLOAT:
						dlg->cont->reaction[ptNum].fstate = pt.p.y;
						dlg->cont->UpdateCurves(false);
						break;
					case REACTORPOS:
					case REACTORP3:
					case REACTORSCALE:
						dlg->cont->reaction[ptNum].pstate[curveNum] = pt.p.y;
						dlg->cont->UpdateCurves(false);
						break;
					case REACTORROT:
						QuatToEuler(dlg->cont->reaction[ptNum].qstate, eulAng);
						eulAng[curveNum] = DegToRad(pt.p.y);
						EulerToQuat(eulAng, dlg->cont->reaction[ptNum].qstate);
						dlg->cont->UpdateCurves(false);

						break;
					default:
						break;
				}
				
				float xMax = dlg->cont->iCCtrl->GetXRange().y;
				float xMin = dlg->cont->iCCtrl->GetXRange().x;
				float oldWidth = xMax - xMin;

				float oldMax = xMax;
				float oldMin = xMin;
				Point2 p,pin,pout;

				if (curve->GetNumPts() > ptNum+1 && pt.p.x >= curve->GetPoint(0,ptNum+1, FOREVER).p.x - pt.out.x - curve->GetPoint(0,ptNum+1, FOREVER).in.x)//{}
				{
					pt.p.x = curve->GetPoint(0,ptNum+1, FOREVER).p.x - pt.out.x - curve->GetPoint(0,ptNum+1, FOREVER).in.x - 0.001f;
				}
				else 
					if (ptNum && pt.p.x <= curve->GetPoint(0,ptNum-1, FOREVER).p.x + pt.in.x + curve->GetPoint(0,ptNum+1, FOREVER).out.x)//{}
					{
						pt.p.x = curve->GetPoint(0,ptNum-1, FOREVER).p.x + pt.in.x + curve->GetPoint(0,ptNum+1, FOREVER).out.x + 0.001f;
					}
				
				if (ptNum == 0) 
				{
					if (pt.p.x > xMax) 
						xMax = pt.p.x;
					xMin = pt.p.x;
					dlg->cont->iCCtrl->SetXRange(pt.p.x, xMax, FALSE);
				}
				else if ( ptNum == curve->GetNumPts()-1 )
				{	
					if (pt.p.x < xMin) 
						xMin = pt.p.x;
					dlg->cont->iCCtrl->SetXRange(xMin, pt.p.x, FALSE);
					xMax = pt.p.x;
				}

				//keep the rest of them in sync
				for (int i=0;i<dlg->cont->iCCtrl->GetNumCurves();i++)
				{
					otherCurve = dlg->cont->iCCtrl->GetControlCurve(i);
					if (curve != otherCurve)
					{
						pt = otherCurve->GetPoint(0,ptNum, FOREVER);
						pt.p.x = dlg->cont->reaction[ptNum].fvalue;
						otherCurve->SetPoint(0,ptNum,&pt,FALSE, FALSE);
					}
				}
			}
			theHold.Resume();
			dlg->cont->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
			dlg->ip->RedrawViews(dlg->ip->GetTime());
			dlg->cont->iCCtrl->EnableDraw(TRUE);
			
			dlg->cont->flags &= ~REACTOR_BLOCK_CURVE_UPDATE;

			break;

		case WM_CC_SEL_CURVEPT:
			
			if (dlg->cont->getReactionType() == FLOAT_VAR)
			{
				curve = ((ICurve*)lParam);
				selCount = (LOWORD(wParam));
				if (selCount == 1 )
				{
					BitArray selPts = curve->GetSelectedPts();
					for (i=0;i<selPts.GetSize();i++)
					{
						if (selPts[i])
						{
							SendDlgItemMessage(dlg->hWnd, IDC_REACTION_LIST, LB_SETCURSEL, i, 0);
							dlg->cont->setSelected(i);
							break;
						}
					}
				}
			}
			break;
		
		case WM_CC_DEL_CURVEPT:
			
			if (dlg->cont->rtype == FLOAT_VAR)
			{
				curve = ((ICurve*)lParam);
				ptNum = LOWORD(wParam);
				dlg->cont->flags |= REACTOR_BLOCK_CURVE_UPDATE;

				//delete the points on the other curves
				int curveNum;
				for (curveNum=0;curveNum<dlg->cont->iCCtrl->GetNumCurves();curveNum++)
				{
					otherCurve = dlg->cont->iCCtrl->GetControlCurve(curveNum);
					if (curve != otherCurve)
					{
						otherCurve->Delete(ptNum);
						//dlg->cont->UpdateCurves(false);
					}
				}

				dlg->cont->DeleteReaction(ptNum);
				dlg->cont->flags &= ~REACTOR_BLOCK_CURVE_UPDATE;
				dlg->cont->iCCtrl->Redraw();

			}

			break;
		case WM_CC_INSERT_CURVEPT:
			if (dlg->cont->rtype == FLOAT_VAR && !theHold.RestoreOrRedoing())
			{
				curve = ((ICurve*)lParam);
				if (curve->GetNumPts() <= dlg->cont->reaction.Count()) return 0;
				ptNum = LOWORD(wParam);
				pt = curve->GetPoint(0,ptNum,FOREVER);
				//update the flags for the newly added point
				pt.flags |= CURVEP_NO_X_CONSTRAINT;
				curve->SetPoint(0,ptNum,&pt,FALSE,FALSE);

				int curNum;
				ICurve* tempCurve;

				dlg->cont->flags |= REACTOR_BLOCK_CURVE_UPDATE;
				SVar* reaction = dlg->cont->CreateReactionAndReturn(FALSE);
				dlg->cont->flags &= ~REACTOR_BLOCK_CURVE_UPDATE;

				if (reaction)
				{
					reaction->fvalue = pt.p.x;

					switch ( dlg->cont->type )
					{
						case REACTORFLOAT:
							reaction->fstate = pt.p.y;
							break;
						case REACTORROT:
							float ang[3];
							for (curNum=0;curNum<dlg->cont->iCCtrl->GetNumCurves();curNum++)
							{
								tempCurve = dlg->cont->iCCtrl->GetControlCurve(curNum);
								ang[curNum] = DegToRad(tempCurve->GetValue(0, reaction->fvalue, FOREVER, FALSE));
							}
							EulerToQuat(ang, reaction->qstate);
							break;
						case REACTORSCALE:
						case REACTORPOS:
						case REACTORP3:
							for (curNum=0;curNum<dlg->cont->iCCtrl->GetNumCurves();curNum++)
							{
								tempCurve = dlg->cont->iCCtrl->GetControlCurve(curNum);
								reaction->pstate[curNum] = tempCurve->GetValue(0, reaction->fvalue, FOREVER, FALSE);
							}
							break;
					}
					//AF (3/09/01)
					//don't rebuild just insert
					//dlg->cont->RebuildFloatCurves();
					//theHold.Suspend();
					for (curNum=0;curNum<dlg->cont->iCCtrl->GetNumCurves();curNum++)
					{
						tempCurve = dlg->cont->iCCtrl->GetControlCurve(curNum);
						if (tempCurve != curve)
						{
							CurvePoint *newPt = new CurvePoint();
							newPt->p.x = reaction->fvalue;
							newPt->p.y = tempCurve->GetValue(0, reaction->fvalue);
							newPt->in = newPt->out = Point2(0.0f,0.0f);// newPt->p;
							newPt->flags = pt.flags;
							//newPt->flags |= CURVEP_NO_X_CONSTRAINT;

							tempCurve->Insert(ptNum, *newPt);
						}
					}					
					//theHold.Resume();

					if (dlg->cont->reaction.Count() > 1)		//if its not the first reaction
					{
						dlg->cont->setMinInfluence(ptNum);
					}
					dlg->cont->SortReactions();
				}
			}
			break;
		case WM_PAINT:
			if (!dlg->valid) dlg->Update();
			return 0;			
		
		case WM_CLOSE:
			if (dlg->ip) dlg->ip->ClearPickMode();
			if (dlg->cont->pickReactToMode)
			{
				delete dlg->cont->pickReactToMode; 
				dlg->cont->pickReactToMode = NULL;
			}
			DestroyWindow(hWnd);

			dlg = NULL;
			break;

		case WM_DESTROY:
			delete dlg;
			break;
		
		default:
			return FALSE;
		}
	return TRUE;
	}

//***************************************************************************
//**
//** ResourceMakerCallback implementation
//**
//***************************************************************************

void Reactor::ResetCallback(int curvenum,ICurveCtl *pCCtl)
{
	if (rtype != FLOAT_VAR)
	{
		ICurve *pCurve = NULL;
		pCurve = pCCtl->GetControlCurve(curvenum);
		if(pCurve)
		{
			pCurve->SetNumPts(2);
			NewCurveCreatedCallback(curvenum, pCCtl);
		}
	}
}

void Reactor::NewCurveCreatedCallback(int curvenum,ICurveCtl *pCCtl)
{
	ICurve *newCurve = NULL;
	
	iCCtrl = pCCtl;
	newCurve = iCCtrl->GetControlCurve(curvenum);
	
	if(newCurve)
	{
		newCurve->SetCanBeAnimated(FALSE);
		CurvePoint pt;
		if (rtype != FLOAT_VAR)
		{
			newCurve->SetNumPts(2);
			
			pt = newCurve->GetPoint(0,0);
			pt.p = Point2(0.0f,0.0f);
			pt.flags = (CURVEP_CORNER|CURVEP_BEZIER|CURVEP_LOCKED_Y|CURVEP_LOCKED_X);
			pt.out = Point2(0.25f, 0.25f);
			newCurve->SetPoint(0,0,&pt);
			
			pt = newCurve->GetPoint(0,1);
			pt.p = Point2(1.0f,1.0f);
			pt.flags = (CURVEP_CORNER|CURVEP_BEZIER|CURVEP_LOCKED_Y|CURVEP_LOCKED_X);
			pt.in = Point2(-0.25f, -0.25f);
			newCurve->SetPoint(0,1,&pt);
			newCurve->SetPenProperty( RGB(0,0,0));
			newCurve->SetDisabledPenProperty( RGB(128,128,128));
		}else
		{
			RebuildFloatCurves();
			switch (curvenum) 
			{
			case 0: newCurve->SetPenProperty( RGB(255,0,0)); break;
			case 1: newCurve->SetPenProperty( RGB(0,255,0)); break;
			case 2: newCurve->SetPenProperty( RGB(0,0,255)); break;
			default: break;
			}
			newCurve->SetDisabledPenProperty( RGB(128,128,128));
		}
	}
}

void Reactor::BuildCurves()
{
	int numCurves = 1;
	ICurve *pCurve = NULL;
	CurvePoint point;

	if (rtype == FLOAT_VAR)
	{
		numCurves = Elems();
	}
	else numCurves = reaction.Count();

	if(!iCCtrl)
	{
		MakeRefByID(FOREVER, 1, (RefTargetHandle)CreateInstance(REF_MAKER_CLASS_ID,CURVE_CONTROL_CLASS_ID));
		if(!iCCtrl)
			return;
		
		//bug #271818
//		theHold.Suspend();

		BitArray ba;
		//iCCtrl->SetNumCurves(0, FALSE);
		iCCtrl->SetNumCurves(numCurves, FALSE);
		
		if (rtype != FLOAT_VAR)
		{
			iCCtrl->SetXRange(0.0f,1.0f);
			iCCtrl->SetYRange(0.0f,1.0f);
			iCCtrl->SetZoomValues(492.727f, 137.857f);  
			iCCtrl->SetScrollValues(-24, -35);
		
			//LoadCurveControlResources();
			iCCtrl->SetTitle(GetString(IDS_FALLOFF_CURVE));
			ba.SetSize(1);
		}
		else
		{
			iCCtrl->SetTitle(GetString(IDS_REACTION_CURVE));
			iCCtrl->SetCCFlags(CC_SINGLESELECT|CC_DRAWBG|CC_DRAWGRID|CC_DRAWUTOOLBAR|CC_SHOWRESET|CC_DRAWLTOOLBAR|CC_DRAWSCROLLBARS|CC_AUTOSCROLL|
								CC_DRAWRULER|CC_ASPOPUP|CC_RCMENU_MOVE_XY|CC_RCMENU_MOVE_X|CC_RCMENU_MOVE_Y|CC_RCMENU_SCALE|
								CC_RCMENU_INSERT_CORNER|CC_RCMENU_INSERT_BEZIER|CC_RCMENU_DELETE|CC_SHOW_CURRENTXVAL);
			iCCtrl->SetXRange(0.0f,0.01f);
			iCCtrl->SetYRange(9999999.0f,9999999.0f);
			ba.SetSize(numCurves);
		}
		for (int i=0;i<numCurves;i++)
			NewCurveCreatedCallback(i, iCCtrl);
		
		ba.SetAll();
		iCCtrl->SetDisplayMode(ba);	
//		theHold.Resume();
	}
}

void Reactor::ShowCurveControl()
{
	if (!iCCtrl) BuildCurves();
	
	if(iCCtrl->IsActive())
		iCCtrl->SetActive(FALSE);
	else
	{
		//iCCtrl->RegisterResourceMaker(&theDummyRefMaker);
		iCCtrl->RegisterResourceMaker(this);
		iCCtrl->SetCustomParentWnd(GetDlgItem(dlg->hWnd, IDC_CURVE_BUTTON));
		iCCtrl->SetMessageSink(dlg->hWnd);
		iCCtrl->SetActive(TRUE);
		iCCtrl->ZoomExtents();
	}
}

void Reactor::RebuildFloatCurves()
{
	if (!iCCtrl) return;

	if (rtype == FLOAT_VAR)
	{
		SortReactions();
		theHold.Suspend(); //leave this in.  Not suspending the hold results in an array deletion error in the curve control...
		for (int c =0;c<iCCtrl->GetNumCurves();c++)
		{
			ICurve * curve = iCCtrl->GetControlCurve(c);  //didn't return a valid curve here...
			curve->SetNumPts(reaction.Count());
			curve->SetOutOfRangeType(CURVE_EXTRAPOLATE_CONSTANT);
			curve->SetCanBeAnimated(FALSE);
			for(int i=0;i<reaction.Count();i++)
			{
				if (c == 0 && i == 0)
				{
					iCCtrl->SetXRange(reaction[i].fvalue, reaction[i].fvalue+1.0f, FALSE);
				}
				else{
					if (reaction[i].fvalue < iCCtrl->GetXRange().x) 
						iCCtrl->SetXRange(reaction[i].fvalue, iCCtrl->GetXRange().y, FALSE);
					if (reaction[i].fvalue > iCCtrl->GetXRange().y) 
						iCCtrl->SetXRange(iCCtrl->GetXRange().x, reaction[i].fvalue, FALSE);
				}
				CurvePoint pt = curve->GetPoint(0,i);
				//pt.flags = 0;
				pt.flags |= (CURVEP_NO_X_CONSTRAINT);  //CURVEP_CORNER|
				//pt.p.x = reaction[i].fvalue;
				switch (type) 
				{
					case REACTORFLOAT:
						pt.p = Point2(reaction[i].fvalue, reaction[i].fstate);
						break;
					case REACTORROT:
						float ang[3];
						QuatToEuler(reaction[i].qstate, ang);
						pt.p = Point2(reaction[i].fvalue, RadToDeg(ang[c]));

						break;
					case REACTORSCALE:
					case REACTORPOS:
					case REACTORP3:
						pt.p = Point2(reaction[i].fvalue, reaction[i].pstate[c]);
						//pt.in = Point2(0.0f,0.0f);
						//pt.in = Point2(0.0f,0.0f);
						break;
				}
				curve->SetPoint(0,i, &pt, TRUE, FALSE);
			}
		}
		theHold.Resume();
	}
	iCCtrl->ZoomExtents();
}

void Reactor::UpdateCurves(bool doZoom)
{
	if (flags&REACTOR_BLOCK_CURVE_UPDATE) return;
	int c = 0;
	ICurve* curve;
	if (getReactionType() != FLOAT_VAR)
	{
		//check for NULL curves and delete them
		for (c = 0;c<iCCtrl->GetNumCurves();c++)
		{
			curve = iCCtrl->GetControlCurve(c);
			if (curve == NULL)
			{
				iCCtrl->DeleteCurve(c);
				continue;
			}
		}
		return;
	}

	CurvePoint pt;
	SortReactions();
	if (!iCCtrl) return;
	for (c = 0;c<iCCtrl->GetNumCurves();c++)
	{
		curve = iCCtrl->GetControlCurve(c);
		for(int i=0;i<reaction.Count();i++)
		{
			theHold.Suspend();
			if (c == 0 && i == 0)
			{
				iCCtrl->SetXRange(reaction[i].fvalue-0.001f, reaction[i].fvalue, FALSE);
			}
			else{
				if (reaction[i].fvalue < iCCtrl->GetXRange().x) 
					iCCtrl->SetXRange(reaction[i].fvalue, iCCtrl->GetXRange().y, FALSE);
				if (reaction[i].fvalue > iCCtrl->GetXRange().y) 
					iCCtrl->SetXRange(iCCtrl->GetXRange().x, reaction[i].fvalue, FALSE);
			}
			CurvePoint pt = curve->GetPoint(0,i);
			//pt.p.x = reaction[i].fvalue;
			switch (type) 
			{
				case REACTORFLOAT:
					pt.p = Point2(reaction[i].fvalue, reaction[i].fstate);
					break;
				case REACTORROT:
					float ang[3];
					QuatToEuler(reaction[i].qstate, ang);
					pt.p = Point2(reaction[i].fvalue, RadToDeg(ang[c]));
					break;
				case REACTORSCALE:
				case REACTORPOS:
				case REACTORP3:
					pt.p = Point2(reaction[i].fvalue, reaction[i].pstate[c]);
					//pt.in = Point2(0.0f,0.0f);
					//pt.in = Point2(0.0f,0.0f);
					break;
			}
			curve->SetPoint(0,i, &pt, FALSE, FALSE);
			//DebugPrint("CurveUpdated\n");
			theHold.Resume();
		}
	}
	if (doZoom) iCCtrl->ZoomExtents();

}

void Reactor::GetCurveValue(TimeValue t, float* val, Interval &valid)
{
	if (!iCCtrl) BuildCurves();
	if (!iCCtrl) return;

	int numCurves = iCCtrl->GetNumCurves();
	
	ICurve* curve;
	float f;
	for (int i=0;i<numCurves;i++,val++)
	{
		curve = iCCtrl->GetControlCurve(i);
		f = curve->GetValue(0, getCurFloatValue(t), FOREVER);
		*val = f;
	}
}
void Reactor::EditTrackParams(TimeValue t,ParamDimensionBase *dim,TCHAR *pname,
							  HWND hParent,IObjParam *ip,DWORD flags)
{

	if (!dlg || !dlg->hWnd) {
		dlg = new ReactorDlg(this,dim,pname,ip,hParent);
	} 
	else {
		SetActiveWindow(dlg->hWnd);
	}
	this->ip = ip;


	/*
	this->ip = ip;
	HWND hCur = FindOpenReactorCtrlWindow(hParent,this);
	if (hCur) 
	{
		SetForegroundWindow(hCur);
		return;
	}

	dlg = new ReactorDlg(this,dim,pname,ip,hParent);
	ip->RegisterDlgWnd(dlg->hWnd);
	RegisterReactorCtrlWindow(dlg->hWnd,hParent,this);

  */
}


//Pick Mode Implementation
//*******************************************************

BOOL PickReactToMode::Filter(INode *node)
{
	return FilterAnimatable( ((Animatable*)node) );
}

BOOL PickReactToMode::FilterAnimatable(Animatable *anim)
{
	if ( ((ReferenceTarget*)anim)->TestForLoop(FOREVER,(ReferenceMaker *) cont)!=REF_SUCCEED ) 
		return FALSE;
	if (anim->SuperClassID() == BASENODE_CLASS_ID && ((INode*)anim)->IsRootNode() ) 
		return FALSE;
	
	return anim->SuperClassID() == CTRL_FLOAT_CLASS_ID ||
		anim->SuperClassID() == CTRL_POSITION_CLASS_ID ||
		anim->SuperClassID() == CTRL_POINT3_CLASS_ID ||
		anim->SuperClassID() == CTRL_SCALE_CLASS_ID ||
		anim->SuperClassID() == CTRL_ROTATION_CLASS_ID || 
		anim->SuperClassID() == BASENODE_CLASS_ID; 
}

BOOL PickReactToMode::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
{
	INode *node = ip->PickNode(hWnd,m);	
	if (!node) return false;
	return true; //Filter(node);
}

BOOL PickReactToMode::Pick(IObjParam *ip,ViewExp *vpt)
{
	INode *node = vpt->GetClosestHit();
	theAnimPopupMenu.DoPopupMenu(cont, node); 
	//cont->reactTo(node);
	return TRUE;
}

BOOL PickReactToMode::PickAnimatable(Animatable* anim)
{
	if ( FilterAnimatable((Animatable*)anim) )
	{
		cont->reactTo((ReferenceTarget*)anim);
		GetCOREInterface()->ClearPickMode();
		return TRUE;
	}
	return FALSE;
}


void PickReactToMode::EnterMode(IObjParam *ip)
{
	if (cont->dlg) cont->dlg->iReactToBut->SetCheck(TRUE);
}

void PickReactToMode::ExitMode(IObjParam *ip)
{
	if (cont->dlg) cont->dlg->iReactToBut->SetCheck(FALSE);
}


void AnimPopupMenu::add_hmenu_items(Tab<AnimEntry>& wparams, HMENU menu, int& i)
{
	assert(menu != NULL);

	menus.Append(1, &menu);
	// build the menu
	while (i < wparams.Count())
	{
		AnimEntry& wp = wparams[i];
		i += 1;
		if (!wp.parent) continue;
		if (wp.level > 0)
		{
			// new submenu
			HMENU submenu = CreatePopupMenu();
			AppendMenu(menu, MF_POPUP, (UINT_PTR)submenu, wp.pname);
			add_hmenu_items(wparams, submenu, i);
		}
		else if (wp.level < 0)
			// end submenu
			return;
		else
			// add menu item
			AppendMenu(menu, MF_STRING, i, wp.pname);
	}
}

bool AnimPopupMenu::wireable(Animatable* parent, int subnum)
{
	Control* c;
	Animatable* anim = parent->SubAnim(subnum);
	if (anim != NULL && (c = GetControlInterface(anim)) != NULL )
		return true;
	else if (parent->SuperClassID() == PARAMETER_BLOCK_CLASS_ID)
	{
		IParamBlock* pb = (IParamBlock*)parent;
		if (pb->GetAnimParamControlType(subnum) != 0)
			return true;
	}
	else if (parent->SuperClassID() == PARAMETER_BLOCK2_CLASS_ID)
	{
		IParamBlock2* pb = (IParamBlock2*)parent;
		if (pb->GetAnimParamControlType(subnum) != 0)
			return true;
	}
	return false;
}

bool AnimPopupMenu::Filter(Animatable *anim, bool includeChildren)
{
	bool valid = true;
	if ( ((ReferenceTarget*)anim)->TestForLoop(FOREVER,(ReferenceMaker *) reactor)!=REF_SUCCEED ) 
		valid = false;
	if (anim == reactor)
		valid = false;
	if (anim->SuperClassID() == BASENODE_CLASS_ID && ((INode*)anim)->IsRootNode() ) 
		valid = false;
	if (!includeChildren) return valid;
	for (int i = 0; i< anim->NumSubs(); i++)
	{
		if (anim->SubAnim(i))
		{
			valid = Filter(anim->SubAnim(i));
			if (valid == true) return valid;
		}
	}
	return valid;
}

bool AnimPopupMenu::add_subanim_params(Tab<AnimEntry>& wparams, TCHAR* name, ReferenceTarget* parent, int level, ReferenceTarget* root)
{
	
	bool subAdded = false;

	// add the subanim params for given parent
	if (parent->NumSubs() > 0)
	{
		// first add entry for this level
		if (name != NULL)
		{

		}
		for (int i = 0; i < parent->NumSubs(); i++)
		{
			// add assignable leaf subanims or descend 
			Animatable* s = parent->SubAnim(i);
			if (s == NULL)
				continue;
			if (s->BypassTreeView() || s->BypassPropertyLevel())
			{	
				if (add_subanim_params(wparams, NULL, (ReferenceTarget*)s, 0, root))
					subAdded = true;
			}
			else if (s->NumSubs() > 0 || s->GetCustAttribContainer() != NULL)
			{
				//if (Filter(s))
				//{
					AnimEntry wp (parent->SubAnimName(i), 1, parent, 0, root);
					wparams.Append(1, &wp);
					//subAdded = true;
				//}
				if(!add_subanim_params(wparams, parent->SubAnimName(i), (ReferenceTarget*)s, 1, root))
					wparams.Delete(wparams.Count()-1, 1);
				else subAdded = true;
			}
			else if (wireable(parent, i)) 
			{
				if (Filter(parent))
				{
					AnimEntry wp (parent->SubAnimName(i), 0, parent, i, root);
					wparams.Append(1, &wp);
					subAdded = true;
				}
			}
		}
	}
	// check for custom attributes
	ICustAttribContainer* cc = parent->GetCustAttribContainer();
	if (cc != NULL)
		add_subanim_params(wparams, NULL, (ReferenceTarget*)cc, 0, root);

	if (level == 1)
	{
		AnimEntry wp;
		if (subAdded)
		{
			wp = AnimEntry(name, -1, parent, 0, root);
			wparams.Append(1, &wp);
		}
	}

	return subAdded;
}

void AnimPopupMenu::build_params(Tab<AnimEntry>& wparams, ReferenceTarget* root)
{
	// build the param list for given node
	for (int j = 0; j < wparams.Count(); j++)
		if (wparams[j].pname != NULL)
			free(wparams[j].pname);
	wparams.ZeroCount();

	// params from node subanim tree
	if (Filter(root))
	{
		AnimEntry wp (((INode*)root)->GetName(), 0, root, 0, root);
		wparams.Append(1, &wp);
	}
	add_subanim_params(wparams, NULL, root, 1, root);
}

void AnimPopupMenu::DoPopupMenu(Reactor *cont, INode *node)
{
	Interface* ip = GetCOREInterface();
	POINT mp;
	reactor = cont;

	IPoint3 np;
	Point3 zero(0.0,0.0,0.0);
	ViewExp* view = ip->GetActiveViewport();
	HWND hwnd = view->GetHWnd(); // ip->GetMAXHWnd();
	GraphicsWindow*	gw = view->getGW();
	Matrix3 tm = node->GetNodeTM(ip->GetTime());		
	view->getGW()->setTransform(tm);
	view->getGW()->wTransPoint(&zero, &np);
	ip->ReleaseViewport(view);
	mp.x = np.x; mp.y = np.y;
	ClientToScreen(hwnd, &mp);
	SetCursorPos(mp.x, mp.y);

	HMENU menu;

	// create menu
	if ((menu = CreatePopupMenu()) != NULL)
	{
		// load source nodes param tree into wparams
		build_params(AnimEntries, node);

		// build the popup menu
		int i = 0;
		add_hmenu_items(AnimEntries, menu, i);

		// pop the menu
		int id = TrackPopupMenu(menu, TPM_CENTERALIGN + TPM_NONOTIFY + TPM_RETURNCMD, mp.x, mp.y, 0, hwnd, NULL);
		// destroy the menus
		for (i = 0; i < menus.Count(); i++)
			DestroyMenu(menus[i]);
		menus.ZeroCount();
		if (id > 1)
		{
			if (AnimEntries[id-1].parent->SubAnim(AnimEntries[id-1].subnum)->SuperClassID() == CTRL_USERTYPE_CLASS_ID )
			{
				if (!AssignDefaultController(AnimEntries[id-1].parent, AnimEntries[id-1].subnum))
					return;
			}
			
			cont->reactTo(((ReferenceTarget*)AnimEntries[id-1].parent->SubAnim(AnimEntries[id-1].subnum)));
		}
		else 
			if (id == 1) cont->reactTo( (ReferenceTarget*)AnimEntries[id-1].parent);
	}
}


Control* AnimPopupMenu::AssignDefaultController(Animatable* parent, int subnum)
{
	Control* ctrl = NULL;
	if (parent->SuperClassID() == PARAMETER_BLOCK_CLASS_ID)
	{
		IParamBlock* pb = (IParamBlock*)parent;
		if (pb->GetAnimParamControlType(subnum) == CTRL_FLOAT_CLASS_ID )
			ctrl = NewDefaultFloatController();
		else if (pb->GetAnimParamControlType(subnum) == CTRL_POINT3_CLASS_ID )
			ctrl = NewDefaultPoint3Controller();
	}
	else if (parent->SuperClassID() == PARAMETER_BLOCK2_CLASS_ID)
	{
		IParamBlock2* pb = (IParamBlock2*)parent;
		if (pb->GetAnimParamControlType(subnum) == CTRL_FLOAT_CLASS_ID )
			ctrl = NewDefaultFloatController();
		else if (pb->GetAnimParamControlType(subnum) == CTRL_POINT3_CLASS_ID )
			ctrl = NewDefaultPoint3Controller();
	}
	
	if (ctrl != NULL) 
	{
		macroRecorder->Disable();
		parent->AssignController(ctrl, subnum);
		macroRecorder->Enable();
	}
	return ctrl;
}


