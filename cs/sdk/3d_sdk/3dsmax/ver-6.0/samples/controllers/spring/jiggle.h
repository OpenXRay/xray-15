/**********************************************************************
 *<
	FILE: jiggle.h

	DESCRIPTION:	A procedural Mass/Spring Position controller 
					Main header

	CREATED BY:		Adam Felt

	HISTORY: 

 *>	Copyright (c) 1999-2000, All Rights Reserved.
 **********************************************************************/

#ifndef __JIGGLE__H
#define __JIGGLE__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamm2.h"
#include "custcont.h"
#include "SpringSys.h"
#include "ISpringCtrl.h"
#include "Notify.h"

extern TCHAR *GetString(int id);
extern HINSTANCE hInstance;

#define HAS_NO_PARENT  (1<<0)

class Jiggle;
class JiggleDlg;

//dialog stuff 
class DynMapDlgProc : public ParamMap2UserDlgProc {
	public:
		Jiggle *cont;
		IParamMap2 *paramMap;
		ICustButton		*iAddBone;
		ICustButton		*iDeleteBone;
		ISpinnerControl	*iTension;
		ISpinnerControl	*iDampening;
		ISpinnerControl *iMass;
		ISpinnerControl *iDrag;

		DynMapDlgProc(Jiggle *c) {cont = c; paramMap = NULL;}	
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {delete this;}
		
		void InitParams();
		void DestroyParams();
		void Update(TimeValue t);
	};

class ForceMapDlgProc : public ParamMap2UserDlgProc {
	public:
		Jiggle *cont;		
		ForceMapDlgProc(Jiggle *c) {cont = c;}		
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {delete this;}
	};
	
//About dialog handler
BOOL CALLBACK AboutRollupDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );

//validator classes for the pick node paramblocks
class JiggleValidatorClass : public PBValidator
{
public:
Jiggle *cont;
private:
BOOL Validate(PB2Value &v) 
	{
	INode *node = (INode*) v.r;

	if (node->TestForLoop(FOREVER,(ReferenceMaker *) cont)!=REF_SUCCEED) return FALSE;

	const ObjectState& os = node->EvalWorldState(0);
	Object* ob = os.obj;
	if (ob!=NULL) 
			{	
			int id=(os.obj?os.obj->SuperClassID():SClass_ID(0));
			if (id==WSM_OBJECT_CLASS_ID)
				{
				WSMObject *obref=(WSMObject*)node->GetObjectRef();
				BOOL ShouldBeHere=obref->SupportsDynamics();

				return ShouldBeHere;
				}
			else return FALSE;
				  
			}
	return FALSE;

	};
};

class JiggleNodeValidatorClass : public PBValidator
{
public:
Jiggle *cont;
private:
BOOL Validate(PB2Value &v) 
	{
	INode *node = (INode*) v.r;

	if (node->TestForLoop(FOREVER,(ReferenceMaker *) cont)!=REF_SUCCEED) return FALSE;
	return TRUE;
	};
};

class MyEnumProc : public DependentEnumProc 
	{
      public :
      virtual int proc(ReferenceMaker *rmaker); 
	  INodeTab nodes;
	};


class CacheClass
{
	public:
		Point3 vel, pos;
};

class PickForceMode : 
		public PickModeCallback,
		public PickNodeCallback {
	public:		
		JiggleDlg *dlg;
		
		PickForceMode(JiggleDlg *d) {dlg=d;}
		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);
		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);
		BOOL RightClick(IObjParam *ip,ViewExp *vpt)	{return TRUE;}
		BOOL Filter(INode *node);		
		PickNodeCallback *GetFilter() {return this;}
		BOOL AllowMultiSelect(){ return true; }
	};

class PickNodeMode : 
		public PickModeCallback,
		public PickNodeCallback {
	public:		
		Jiggle *cont;
		DynMapDlgProc *map;
		
		PickNodeMode(DynMapDlgProc *m) {map = m; cont = map->cont;}
		PickNodeMode(Jiggle *c) {map = NULL; cont = c;}

		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);
		BOOL PickAnimatable(Animatable* anim);
		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);
		BOOL RightClick(IObjParam *ip,ViewExp *vpt)	{return TRUE;}
		BOOL Filter(INode *node);		
		PickNodeCallback *GetFilter() {return this;}
		BOOL AllowMultiSelect(){ return true; }
	};


static void UnRegisterJiggleWindow(HWND hWnd);


//Function Publishing stuff
#define SPRING_CONTROLLER_INTERFACE Interface_ID(0x4c212b2e, 0x680828ab)
#define GetSpringControllerInterface(cd) \
			(Jiggle *)(cd)->GetInterface(SPRING_CONTROLLER_INTERFACE)

class Jiggle : public Control, public IJiggle, public SpringSysClient {
	public:
		
		int type;						//the type of controller it is
		bool validStart;
		INode *selfNode;
		BOOL ctrlValid;

		Tab<Matrix3> initState;

		Interval	ivalid;
		Interval	range;
		IParamBlock2 *dyn_pb;
		IParamBlock2 *force_pb;

		Control *posCtrl;			// ref 0
		byte flags;

		JiggleDlg* dlg;
		DynMapDlgProc* pmap;

		HWND	hParams1;  //dynamics dialog handle
		HWND	hParams2;  //force dialog handle
		PickNodeMode *pickNodeMode;
		
		static IObjParam *ip;
		JiggleValidatorClass validator;
		JiggleNodeValidatorClass node_validator;

		Jiggle(int t, Jiggle &ctrl){};
		Jiggle(int type, BOOL loading);
		~Jiggle();

		Jiggle& operator=(const Jiggle& from);

		int	NumParamBlocks() { return 2; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) {if (i==0) return dyn_pb; 
						else if (i==1) return force_pb; 
						else return NULL;} // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { if (dyn_pb->ID() == id) return dyn_pb;  
						else if (force_pb->ID() == id) return force_pb; 
						else return NULL; } // return id'd ParamBlock
		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev); 
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next); 

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

		void DeleteThis() {delete this;}		
		int IsKeyable() {return 0;}		
		BOOL IsAnimated() {return TRUE;}  //this could be done better

		int NumSubs()  {return 3;}
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int subNum);
		BOOL ChangeParents(TimeValue t,const Matrix3& oldP,const Matrix3& newP,const Matrix3& tm);

		//From Animatable
		void CopyKeysFromTime(TimeValue src,TimeValue dst,DWORD flags) {posCtrl->CopyKeysFromTime(src,dst,flags);}
		BOOL IsKeyAtTime(TimeValue t,DWORD flags) {return posCtrl->IsKeyAtTime(t,flags);}
		BOOL GetNextKeyTime(TimeValue t,DWORD flags,TimeValue &nt) {return posCtrl->GetNextKeyTime(t,flags,nt);}
		int GetKeyTimes(Tab<TimeValue> &times,Interval range,DWORD flags) {return posCtrl->GetKeyTimes(times,range,flags);}
		int GetKeySelState(BitArray &sel,Interval range,DWORD flags) {return posCtrl->GetKeySelState(sel,range,flags);}

		// Reference methods
		int NumRefs() {return 3;}		
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
		RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage);
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
		BOOL AssignController(Animatable *control,int subAnim);

		// Control methods				
		void Copy(Control *from);
		BOOL IsLeaf() {return FALSE;}
		void SetValue(TimeValue t, void *val, int commit, GetSetMethod method);
		void CommitValue(TimeValue t) {posCtrl->CommitValue(t); ctrlValid = true; }
		void RestoreValue(TimeValue t) {posCtrl->RestoreValue(t);}	
		
		//FPMixinInterface methods
		BaseInterface* GetInterface(Interface_ID id) 
		{ 
			if (id == SPRING_CONTROLLER_INTERFACE) 
				return (Jiggle*)this; 
			else 
				return FPMixinInterface::GetInterface(id);
		} 

		//Local Methods
		Matrix3 GetCurrentTM(TimeValue t);
		BOOL SetSelfReference();
		Point3 ComputeValue(TimeValue t, Interval &valid);
		void UpdateNodeList(bool updateSpinners = true);		
	
		//From IJiggle
		virtual float	GetMass();
		virtual float	GetDrag();
		virtual float	GetTension(int index);
		virtual float	GetDampening(int index);
		virtual void	SetMass(float mass, bool update=true);
		virtual void	SetDrag(float drag, bool update=true);
		virtual void	SetTension(int index, float tension, int absolute=1, bool update=true);
		virtual void	SetDampening(int index, float dampening, int absolute=1, bool update=true);
		virtual BOOL	AddSpring(INode *node);	
		virtual INT		GetSpringCount();
		virtual void	RemoveSpring(int which);
		virtual void	RemoveSpring(INode *node);
		virtual SpringSys* GetSpringSystem() { return partsys; }
		
		//From SpringSysClient
		Tab<Matrix3> GetForceMatrices(TimeValue t);
		Point3 GetDynamicsForces(TimeValue t, Point3 pos, Point3 vel);

		//Undo methods
		void HoldAll();
};

		
class PosJiggle : public Jiggle {
	public:
		
		PosJiggle(BOOL loading) : Jiggle(JIGGLEPOS, loading) {}
		~PosJiggle() {}

		Class_ID ClassID() { return JIGGLE_POS_CLASS_ID; }  
		SClass_ID SuperClassID() { return CTRL_POSITION_CLASS_ID; } 
		void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}

		// Control methods
		RefTargetHandle Clone(RemapDir& remap);
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method);
	};


class Point3Jiggle : public Jiggle {
	public:

		Point3Jiggle(BOOL loading) : Jiggle(JIGGLEP3, loading) {}
		~Point3Jiggle() {}

		Class_ID ClassID() { return JIGGLE_P3_CLASS_ID; }  
		SClass_ID SuperClassID() { return CTRL_POINT3_CLASS_ID; } 
		void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}

		// Control methods
		RefTargetHandle Clone(RemapDir& remap);
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method);
	};

class PosJiggleClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading) {return new PosJiggle(loading);}
	const TCHAR *	ClassName() {return GetString(IDS_CLASS_NAME);}
	SClass_ID		SuperClassID() {return CTRL_POSITION_CLASS_ID;}
	Class_ID		ClassID() {return JIGGLE_POS_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_CATEGORY);}
	void			ResetClassParams (BOOL fileReset);

// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("PositionSpring"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle

};
static PosJiggleClassDesc PosJiggleDesc;

//Mixin descripter
static FPInterfaceDesc spring_controller_interface(
    SPRING_CONTROLLER_INTERFACE, _T("Spring"), 0, &PosJiggleDesc, FP_MIXIN,
		IJiggle::get_mass, _T("getMass"), 0, TYPE_FLOAT, 0, 0,
		IJiggle::set_mass, _T("setMass"), 0, TYPE_VOID, 0, 1,
			_T("mass"), 0, TYPE_FLOAT,
		
		IJiggle::get_drag, _T("getDrag"), 0, TYPE_FLOAT, 0, 0,
		IJiggle::set_drag, _T("setDrag"), 0, TYPE_VOID, 0, 1,
			_T("drag"), 0, TYPE_FLOAT,
		
		IJiggle::get_tension, _T("getTension"), 0, TYPE_FLOAT, 0, 1,
			_T("springIndex"), 0, TYPE_INDEX,
		IJiggle::set_tension, _T("setTension"), 0, TYPE_VOID, 0, 2,
			_T("springIndex"), 0, TYPE_INDEX,
			_T("tension"), 0, TYPE_FLOAT,
		
		IJiggle::get_dampening, _T("getDampening"), 0, TYPE_FLOAT, 0, 1,
			_T("springIndex"), 0, TYPE_INDEX,
		IJiggle::set_dampening, _T("setDampening"), 0, TYPE_VOID, 0, 2,
			_T("springIndex"), 0, TYPE_INDEX,
			_T("dampening"), 0, TYPE_FLOAT,

		IJiggle::add_spring, _T("addSpring"), 0, TYPE_BOOL, 0, 1, 
			_T("node"), 0, TYPE_INODE,
		
		IJiggle::get_spring_count, _T("getSpringCount"), 0, TYPE_INT, 0, 0,

		IJiggle::remove_spring_by_index, _T("removeSpringByIndex"), 0, TYPE_VOID, 0, 1,
			_T("springIndex"), 0, TYPE_INDEX,
		
		IJiggle::remove_spring, _T("removeSpring"), 0, TYPE_VOID, 0, 1,
			_T("node"), 0, TYPE_INODE,

		//IJiggle::get_spring_system,  _T("getSpringSys"), 0, TYPE_INTERFACE, 0, 0,
		end
);

// block IDs
enum { jig_dynamics_params, jig_force_params};

// jig param IDs
enum { jig_control_node, jig_how };
enum { jig_tolerence, jig_xeffect, jig_yeffect, jig_zeffect, jig_start, jig_force_node };

class DynamicsPBAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t);
	void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval& valid);
};


class ForcesPBAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t);
	void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval& valid);
	void TabChanged(tab_changes changeCode, Tab<PB2Value>* tab, ReferenceMaker* owner, ParamID id, int tabIndex, int count); 
};


static DynamicsPBAccessor dynPBAccessor;
static ForcesPBAccessor forcesPBAccessor;

static ParamBlockDesc2 jig_param_blk ( jig_dynamics_params, _T("JiggleDynamicsParameters"),  0, &PosJiggleDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, JIGGLE_PBLOCK_REF1, 
	//rollout
	IDD_DYNAMICS_PANEL, IDS_DYN_PARAMS, BEGIN_EDIT_MOTION, 0, NULL,
	// params
	jig_control_node,    _T(""),  TYPE_INODE_TAB,		0,	0,	IDS_JIG_CONTROL_NODES,
		end,
	jig_how,	_T("effectHow"),	TYPE_RADIOBTN_INDEX,	0,	IDS_EFFECT_HOW,
		p_default,		SET_PARAMS_ABSOLUTE,
		p_range,		SET_PARAMS_RELATIVE,	SET_PARAMS_ABSOLUTE,
		p_ui,			TYPE_RADIO,	2,	IDC_RELATIVE_RB, IDC_ABSOLUTE_RB,
		p_enabled,		TRUE,
		p_accessor,		&dynPBAccessor,
		end, 
	end
	);


static ParamBlockDesc2 jig_force_param_blk ( jig_force_params, _T("ForceAndLimits"),  0, &PosJiggleDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, JIGGLE_PBLOCK_REF2, 
	//rollout
	IDD_FORCES_PANEL, IDS_FORCE_PARAMS, 0, APPENDROLL_CLOSED, NULL,
	// params
	jig_force_node,    _T("forceNode"),  TYPE_INODE_TAB,		0,	P_AUTO_UI,	IDS_JIG_FORCE_NODES,
		p_ui,			TYPE_NODELISTBOX, IDC_LIST1,IDC_FORCE_PICKNODE,0,IDC_REMOVE_FORCE,
		p_accessor,		&forcesPBAccessor,
		end,
	jig_tolerence,  _T("steps"), 	TYPE_INT, 	0, 	IDS_JIGGLE_STEPS, 
		p_default, 		2,	
		p_range, 		0, 4, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_POS_INT, IDC_JIGGLE_STEPS,IDC_JIGGLE_STEPS_SPIN, 1.0, 
		p_accessor,		&forcesPBAccessor,
		end, 
	jig_xeffect,  _T("x_effect"), 			TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_JIGGLE_X, 
		p_default, 		100.0f,	
		p_range, 		0.0f, 1000.0f, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_JIGGLE_X,IDC_JIGGLE_X_SPIN, .01f, 
		p_accessor,		&forcesPBAccessor,
		end, 
	jig_yeffect,  _T("y_effect"), 			TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_JIGGLE_Y, 
		p_default, 		100.0f,	
		p_range, 		0.0f, 1000.0f, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_JIGGLE_Y,IDC_JIGGLE_Y_SPIN, .01f, 
		p_accessor,		&forcesPBAccessor,
		end, 
	jig_zeffect,  _T("z_effect"), 			TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_JIGGLE_Z, 
		p_default, 		100.0f,	
		p_range, 		0.0f, 1000.0f, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_JIGGLE_Z,IDC_JIGGLE_Z_SPIN, .01f, 
		p_accessor,		&forcesPBAccessor,
		end, 
	jig_start,  _T("start"), 	TYPE_INT, 	0, 	IDS_JIGGLE_START, 
		p_default, 		0,	
		p_range, 		-999999, 999999, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_INT, IDC_JIGGLE_START,IDC_JIGGLE_START_SPIN, 1.0, 
		p_accessor,		&forcesPBAccessor,
		end, 
	end
	);

static bool p3JiggleInterfaceAdded = false;
class Point3JiggleClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { 
						if (!p3JiggleInterfaceAdded){
							AddInterface(&spring_controller_interface);
							AddParamBlockDesc(&jig_param_blk);
							AddParamBlockDesc(&jig_force_param_blk);
							p3JiggleInterfaceAdded = true;
						}
						return new Point3Jiggle(loading); 
					}
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return CTRL_POINT3_CLASS_ID; }
	Class_ID		ClassID() { return JIGGLE_P3_CLASS_ID; }
	const TCHAR* 	Category() {return GetString(IDS_CATEGORY);  }
	void			ResetClassParams (BOOL fileReset);

// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("Point3Spring"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle

};
static Point3JiggleClassDesc point3JiggleDesc;


class SpringRestore: public RestoreObj 
{
	public:		
		Jiggle *cont;
		SpringSys *sav;
		SpringSys *redo;

		SpringRestore(Jiggle *ctrl, SpringSys* partsys) 
		{
			theHold.Suspend();
			cont = ctrl;
			sav = new SpringSys(((SpringSysClient*)cont), 1);
			sav->Copy(cont->partsys);
			redo = NULL;
			theHold.Resume();
		}
		~SpringRestore() 
		{
			if (sav) delete sav;
			if (redo) delete redo;
		}		
		
		void Restore(int isUndo) {
			assert(cont);
			theHold.Suspend();
			if (isUndo) {
				if (!redo) redo = new SpringSys(((SpringSysClient*)cont), 1);
				redo->Copy(cont->partsys);

				}
			cont->partsys->Copy(sav);
			theHold.Resume();
			cont->partsys->Invalidate();
			cont->validStart = false;
			cont->UpdateNodeList(false);
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
		void Redo() {
			assert(cont); 
			if (redo) 
				cont->partsys->Copy(redo);
			cont->partsys->Invalidate();
			cont->validStart = false;
			cont->UpdateNodeList(false);
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
		void EndHold() {}
		TSTR Description() { return GetString(IDS_FULL_UNDO); }
};

class SpringSpinnerRestore: public RestoreObj 
{
	public:		
		Jiggle *cont;
		float save_tension, redo_tension;
		float save_dampening, redo_dampening;


		SpringSpinnerRestore(Jiggle *ctrl) 
		{
			theHold.Suspend();
			cont = ctrl;
			theHold.Resume();
		}
		~SpringSpinnerRestore() 
		{
		}		
		
		void Restore(int isUndo) {
			assert(cont); 
			theHold.Suspend();
			if (isUndo) {

				}
			cont->partsys->Invalidate();
			cont->validStart = false;
			cont->UpdateNodeList();
			}
		void Redo() {
			assert(cont); 
			cont->partsys->Invalidate();
			cont->validStart = false;
			cont->UpdateNodeList();
			}
		void EndHold() {}
		TSTR Description() { return _T("SpringSpinnerRestore"); }
};



//Dialog Proc stuff
//*********************************************
#define JIGGLEDLG_CONTREF	0
#define JIGGLEDLG_CLASS_ID	0x1c353f9f
class JiggleDlgTimeChangeCallback;

class JiggleDlg : public ReferenceMaker {
	public:
		static IObjParam *ip;
		Jiggle *cont;
		HWND hWnd, dynDlg, forceDlg;
		BOOL valid, blockRedraw;
		JiggleDlgTimeChangeCallback *timeChangeCallback;

		ICustButton		*iAddBone;
		ICustButton		*iDeleteBone;
		ISpinnerControl	*iTension;
		ISpinnerControl	*iDampening;
		ISpinnerControl *iMass;
		ISpinnerControl *iDrag;

		ISpinnerControl *xSpin;
		ISpinnerControl *ySpin;
		ISpinnerControl *zSpin;
		ISpinnerControl *startSpin;
		ISpinnerControl *stepsSpin;
		ICustButton		*iPickForce;
		
		PickForceMode *pickForceMode;

		JiggleDlg(IObjParam *i,Jiggle *c);
		~JiggleDlg();

		Class_ID ClassID() {return Class_ID(JIGGLEDLG_CLASS_ID,0);}
		SClass_ID SuperClassID() {return REF_MAKER_CLASS_ID;}

		void Init(HWND hParent);
		void InitDynamicsParams();
		void DestroyDynamicsParams();
		void Reset(IObjParam *i,Jiggle *c);
		void Invalidate();
		void Update();
		void UpdateForceList();
		void UpdateForceSpinners();

		void SetupUI();
		void SetupList();
		static IRollupWindow *TVRollUp;

		virtual HWND CreateWin(HWND hParent)=0;
		virtual void MouseMessage(UINT message,WPARAM wParam,LPARAM lParam) {};	
		virtual void MaybeCloseWindow() {}

		RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage);
		int NumRefs();
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
	};


class JiggleTrackDlg : public JiggleDlg {
	public:
		
		JiggleTrackDlg(IObjParam *i,Jiggle *c)
			: JiggleDlg(i,c) {}
					
		HWND CreateWin(HWND hParent);
		void MouseMessage(UINT message,WPARAM wParam,LPARAM lParam) {};
		void MaybeCloseWindow();
	};

class JiggleDlgTimeChangeCallback : public TimeChangeCallback
{
	JiggleDlg* dlg;
	public:
		JiggleDlgTimeChangeCallback(JiggleDlg* d){dlg = d;}
		void TimeChanged(TimeValue t);
};

class CheckForNonJiggleDlg : public DependentEnumProc {
	public:		
		BOOL non;
		ReferenceMaker *me;
		CheckForNonJiggleDlg(ReferenceMaker *m) {non = FALSE;me = m;}
		int proc(ReferenceMaker *rmaker) {
			if (rmaker==me) return 0;
			if (rmaker->SuperClassID()!=REF_MAKER_CLASS_ID &&
				rmaker->ClassID()!=Class_ID(JIGGLEDLG_CLASS_ID,0)) {
				non = TRUE;
				return 1;
				}
			return 0;
			}
	};

//Window mananger class
class JiggleWindow 
{
	public:
		HWND hWnd;
		HWND hParent;
		Control *cont;
		JiggleWindow() {assert(0);}
		JiggleWindow(HWND hWnd,HWND hParent,Control *cont)
		{this->hWnd=hWnd; this->hParent=hParent; this->cont=cont; }
};
static Tab<JiggleWindow> jiggleWindows;

static void RegisterJiggleWindow(HWND hWnd, HWND hParent, Control *cont)
{
	JiggleWindow rec(hWnd,hParent,cont);
	jiggleWindows.Append(1,&rec);
}

static void UnRegisterJiggleWindow(HWND hWnd)
{	
	for (int i=0; i<jiggleWindows.Count(); i++) 
	{
		if (hWnd==jiggleWindows[i].hWnd) 
		{
			jiggleWindows.Delete(i,1);
			return;
		}
	}	
}

static HWND FindOpenJiggleWindow(HWND hParent,Control *cont)
{	
	for (int i=0; i<jiggleWindows.Count(); i++) 
	{
		if (hParent == jiggleWindows[i].hParent &&
			cont    == jiggleWindows[i].cont) 
		{
			return jiggleWindows[i].hWnd;
		}
	}
	return NULL;
}



#endif // __JIGGLE__H
