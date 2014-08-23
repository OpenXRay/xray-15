/**********************************************************************
 *<
	FILE: listctrl.cpp

	DESCRIPTION:  Implements built-in list controllers

	CREATED BY: Rolf Berteig

	HISTORY: created 9/16/95
	EDITED:  Moved out of Core and function published -- Adam Felt (08/30/00) 
	EDITED:  Added weights and pose to pose blending  -- Adam Felt (02/21/02)

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "ctrl.h"
#include "decomp.h"
#include "iparamb2.h"
#include "istdplug.h"

extern HINSTANCE hInstance;

#define FLOATLIST_CNAME		GetString(IDS_RB_FLOATLIST)
#define POINT3LIST_CNAME	GetString(IDS_RB_POINT3LIST)
#define POSLIST_CNAME		GetString(IDS_RB_POSITIONLIST)
#define ROTLIST_CNAME		GetString(IDS_RB_ROTATIONLIST)
#define SCALELIST_CNAME		GetString(IDS_RB_SCALELIST)
#define MASTERLIST_CNAME	GetString(IDS_PW_MASTERBLOCK)
#define POINT4LIST_CNAME	GetString(IDS_RB_POINT4LIST)

#define PBLOCK_INIT_REF_ID 2

static void UnRegisterListCtrlWindow(HWND hWnd);

class ListControl;

typedef Tab<Control*> ControlTab;

class NameList : public Tab<TSTR*> {
	public:
		void Free() {
			for (int i=0; i<Count(); i++) {
				delete (*this)[i];
				(*this)[i] = NULL;
				}
			}
		void Duplicate() {
			for (int i=0; i<Count(); i++) {
				if ((*this)[i]) (*this)[i] = new TSTR(*(*this)[i]);
				}
			}
	};

class ListDummyEntry : public Control {
	public:
		ListControl *lc;
		void Init(ListControl *l);
		Class_ID ClassID() {return Class_ID(DUMMY_CONTROL_CLASS_ID,0);}
		SClass_ID SuperClassID();
		void GetClassName(TSTR& s) {s=_T("");}
		RefResult AutoDelete() {return REF_SUCCEED;}
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	         PartID& partID,  RefMessage message) {return REF_SUCCEED;}
		RefTargetHandle Clone(RemapDir &remap) {BaseClone(this,this,remap); return this;}

		void Copy(Control *from) {}
		void CommitValue(TimeValue t) {}
		void RestoreValue(TimeValue t) {}
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void SetValue(TimeValue t, void *val, int commit=1, GetSetMethod method=CTRL_ABSOLUTE) {}

		BOOL CanCopyAnim() {return FALSE;}
		int IsKeyable() {return FALSE;}
	};


class ListControlMotionDlg;

enum { kListCtrlWeightParams };					// paramblock IDs
enum { kListCtrlWeight, kListCtrlAverage };		// parameter IDs

class ListControl : public IListControl {		
	public:
		ListDummyEntry dummy;
		Tab<Control*> conts;	//ControlTab 
		Control *clip;			//ListControl
		NameList names;
		TSTR nameClip;
		float weightClip;
		int active;	
		
		IParamBlock2* pblock;
	
		ListControlMotionDlg *dlg;
		DWORD paramFlags;

		ListControl(BOOL loading=FALSE);
		ListControl(const ListControl& ctrl);
		virtual ~ListControl();	
		
		ListControl& operator=(const ListControl& ctrl);
		void Resize(int c);
		float AverageWeight(float weight);	
		
		//virtual ClassDesc2 *GetListDesc()=0;
		virtual ListControl *DerivedClone()=0;

		//From IListControl
		int	 GetListCount() {return conts.Count();}
		void SetActive(int index);
		int  GetActive() { return active; }
		void DeleteItem(int index);
		void CutItem(int index);
		void PasteItem(int index);
		void SetName(int index, TSTR name);
		TSTR GetName(int index);
		
		//From FPMixinInterface
		BaseInterface* GetInterface(Interface_ID id) 
		{ 
			if (id == LIST_CONTROLLER_INTERFACE) 
				return (IListControl*)this; 
			else 
				return FPMixinInterface::GetInterface(id);
		} 


		// From Control
		void Copy(Control *from);
		void CommitValue(TimeValue t);
		void RestoreValue(TimeValue t);
		BOOL IsLeaf() {return FALSE;}		
		int IsKeyable();
		void SetValue(TimeValue t, void *val, int commit=1, GetSetMethod method=CTRL_ABSOLUTE);
		void EnumIKParams(IKEnumCallback &callback);
		BOOL CompDeriv(TimeValue t,Matrix3& ptm,IKDeriv& derivs,DWORD flags);
		void MouseCycleCompleted(TimeValue t);
		BOOL InheritsParentTransform();

		// From Animatable
		virtual int NumSubs()  {return conts.Count()+2;} //numControllers+dummyController+pblock
		virtual Animatable* SubAnim(int i);
		virtual TSTR SubAnimName(int i);		
		void DeleteThis() {delete this;}		
		void AddNewKey(TimeValue t,DWORD flags);
		void CloneSelectedKeys(BOOL offset);
		void DeleteKeys(DWORD flags);
		void SelectKeys(TrackHitTab& sel, DWORD flags);
		BOOL IsKeySelected(int index);
		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);
		ParamDimension* GetParamDimension(int i);
		BOOL AssignController(Animatable *control,int subAnim);
		void CopyKeysFromTime(TimeValue src,TimeValue dst,DWORD flags);
		void DeleteKeyAtTime(TimeValue t);
		BOOL IsKeyAtTime(TimeValue t,DWORD flags);
		BOOL GetNextKeyTime(TimeValue t,DWORD flags,TimeValue &nt);
		int GetKeyTimes(Tab<TimeValue> &times,Interval range,DWORD flags);
		int GetKeySelState(BitArray &sel,Interval range,DWORD flags);
		void EditTrackParams(
			TimeValue t,
			ParamDimensionBase *dim,
			TCHAR *pname,
			HWND hParent,
			IObjParam *ip,
			DWORD flags);
		int TrackParamsType() {return TRACKPARAMS_WHOLE;}		
		int SubNumToRefNum(int subNum);
		Interval GetTimeRange(DWORD flags);

		// From ReferenceTarget
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock

		int NumRefs() {return conts.Count()+3;};	//numControllers+dummyController+controllerInBuffer+pblock
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage);
		void NotifyForeground(TimeValue t);
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

	};

ClassDesc* GetFloatListDesc();

//Control* ListControl::clip = NULL;

#define LISTDLG_CONTREF	0
//#define LISTDLG_CLIPREF	1

#define LISTDLG_CLASS_ID	0x773dd61a

class ListControlDlg : public ReferenceMaker, public TimeChangeCallback {
	public:
		IObjParam *ip;
		ListControl *cont;
		HWND hWnd;
		BOOL valid; 

		ListControlDlg(IObjParam *i,ListControl *c);
		~ListControlDlg();

		Class_ID ClassID() {return Class_ID(LISTDLG_CLASS_ID,0);}
		SClass_ID SuperClassID() {return REF_MAKER_CLASS_ID;}

		void StartEdit(int a);
		void EndEdit(int a);
		//void SetActive(int a);
		//void DeleteItem(int a);
		//void CutItem(int a);
		//void PasteItem(int a);
		void SetButtonStates();
		void Init(HWND hParent);
		void Reset(IObjParam *i,ListControl *c);
		void Invalidate();
		void Update();
		void SetupUI();
		void SetupList();
		void UpdateList();

		void TimeChanged(TimeValue t) {Invalidate(); Update();}
		
		virtual HWND CreateWin(HWND hParent)=0;
		virtual void MouseMessage(UINT message,WPARAM wParam,LPARAM lParam) {};	
		virtual void MaybeCloseWindow() {}

		void WMCommand(int id, int notify, HWND hCtrl);
		
		RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage);
		int NumRefs();
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
	};

class ListControlMotionDlg : public ListControlDlg {
	public:
		
		ListControlMotionDlg(IObjParam *i,ListControl *c)
			: ListControlDlg(i,c) {}
					
		HWND CreateWin(HWND hParent);
		void MouseMessage(UINT message,WPARAM wParam,LPARAM lParam) 
			{ip->RollupMouseMessage(hWnd,message,wParam,lParam);}
	};

class ListControlTrackDlg : public ListControlDlg {
	public:
		
		ListControlTrackDlg(IObjParam *i,ListControl *c)
			: ListControlDlg(i,c) {}
					
		HWND CreateWin(HWND hParent);
		void MouseMessage(UINT message,WPARAM wParam,LPARAM lParam) {};
		void MaybeCloseWindow();
	};

//ListControlMotionDlg *ListControl::dlg = NULL;
//DWORD ListControl::paramFlags = 0;

class ListControlRestore : public RestoreObj {
	public:
		ListControl *cont;
		ControlTab list, rlist;
		NameList unames, rnames;
		int active, ractive;

		ListControlRestore(ListControl *c) 
			{
			cont   = c;
			list   = c->conts;
			active = c->active;
			unames = c->names;
			unames.Duplicate();
			}   		
		void Restore(int isUndo) 
			{	   
			// RB 3-26-96: Handled by RemoveItemRestore now

			//if (cont->dlg) {
			//	cont->dlg->EndEdit(cont->active);
			//	}
			rlist   = cont->conts;
			ractive = cont->active;
			rnames  = cont->names;
			rnames.Duplicate();

			cont->conts  = list;
			cont->active = active;
			cont->names.Free();
			cont->names = unames;
			cont->names.Duplicate();

			//if (cont->dlg) {
			//	cont->dlg->StartEdit(cont->active);
			//	}
			cont->NotifyDependents(FOREVER,0,REFMSG_CHANGE);			
			cont->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
			}
		void Redo() 
			{			
			//if (cont->dlg) {
			//	cont->dlg->EndEdit(cont->active);
			//	}
			cont->conts  = rlist;
			cont->active = ractive;
			cont->names.Free();
			cont->names = rnames;
			cont->names.Duplicate();

			//if (cont->dlg) {
			//	cont->dlg->StartEdit(cont->active);
			//	}
			cont->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
			cont->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
			}				
		TSTR Description() {return TSTR(_T("List Control"));}
	};

// The restore object handles stopping and starting editing
// durring an undo. One of these is placed before and after
// a cut copy/paste/delete operation.
class RemoveItemRestore : public RestoreObj {
	public:   		
		ListControl *cont;
		BOOL parity;

		RemoveItemRestore(ListControl *c, BOOL p)
			{cont=c; parity=p;}
		void Restore(int isUndo) {
			if (cont->dlg) {
				if (parity) cont->dlg->EndEdit(cont->active);
				else cont->dlg->StartEdit(cont->active);
				}
			}
		void Redo() {
			if (cont->dlg) {
				if (!parity) cont->dlg->EndEdit(cont->active);
				else cont->dlg->StartEdit(cont->active);
				}
			}
		TSTR Description() {return TSTR(_T("List control remove item"));}
	};

class ListSizeRestore : public RestoreObj {
	public:
		ListControl *cont;		

		ListSizeRestore(ListControl *c) {
			cont = c;			
			}
		void Restore(int isUndo) {			
			cont->conts.Resize(cont->conts.Count()-1);
			cont->names.Resize(cont->names.Count()-1);
			if (cont->active>=cont->conts.Count())
				cont->active = cont->conts.Count()-1;
			if (cont->active < 0) cont->active = 0;
			cont->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
			}
		void Redo() {
			cont->conts.SetCount(cont->conts.Count()+1);
			cont->conts[cont->conts.Count()-1] = NULL;
			cont->names.SetCount(cont->names.Count()+1);
			cont->names[cont->names.Count()-1] = NULL;
			if (cont->active>=cont->conts.Count())
				cont->active = cont->conts.Count()-1;
			if (cont->active < 0) cont->active = 0;
			cont->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
			}
		TSTR Description() {return TSTR(_T("List size"));}
	};


// This restore object doesn't do anything except keep the clipboard
// item from being deleted.
class CutRestore : public RestoreObj, public ReferenceMaker {	
	public:	
		Control *cont;
		CutRestore(Control *c) {
			cont = NULL;
			theHold.Suspend();
			ReplaceReference(0,c);
			theHold.Resume();
			}
		~CutRestore() {
			theHold.Suspend();
			DeleteReference(0);
			theHold.Resume();
			}
		void Restore(int isUndo) {}
		void Redo() {}
		TSTR Description() {return TSTR(_T("List Control Cut"));}
		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i) {return cont;}
		void SetReference(int i, RefTargetHandle rtarg) {cont=(Control*)rtarg;}
		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message) {
			if (message==REFMSG_TARGET_DELETED && hTarget==cont) {
				cont=NULL;
				}
			return REF_SUCCEED;
			}
	};


// CAL-06/06/02: TODO: this should really go to core\decomp.cpp, and it should be optimized.
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


void ListDummyEntry::Init(ListControl *l) {lc=l;}
SClass_ID ListDummyEntry::SuperClassID() {return lc->SuperClassID();}

//added by AF -- some controllers don't initialize values before calling GetValue.  
//This does the initializing for them
void ListDummyEntry::GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method) 
{
	if (method == CTRL_ABSOLUTE)
	{
		switch(SuperClassID())
		{		
			case CTRL_POSITION_CLASS_ID:
			case CTRL_POINT3_CLASS_ID:
				*((Point3*)val) = Point3(0,0,0);
				break;
			case CTRL_POINT4_CLASS_ID:
				*((Point4*)val) = Point4(0,0,0,0);
				break;
			case CTRL_FLOAT_CLASS_ID:
				*((float*)val) = 0.0f;
				break;
			case CTRL_SCALE_CLASS_ID:
				*((ScaleValue*)val) = ScaleValue(Point3(1,1,1));
				break;
			case CTRL_ROTATION_CLASS_ID:
				*((Quat*)val) = Quat(0.0f,0.0f,0.0f,1.0f);
				break;
			default:
				break;
		}
	}
}


ListControl::ListControl(BOOL loading)
	{
	active = 0;
	dummy.Init(this);
	paramFlags = 0;
	dlg = 0;
	clip = NULL;
	weightClip = 0.0f;
	}

ListControl::ListControl(const ListControl& ctrl)
	{
	DeleteAllRefsFromMe();
	Resize(ctrl.conts.Count());	
	int i=0;
	for (i=0; i<ctrl.conts.Count(); i++) {		
		MakeRefByID(FOREVER,i,ctrl.conts[i]);
		}
	MakeRefByID(FOREVER,i,NULL);
	MakeRefByID(FOREVER,i+1,ctrl.clip);
	MakeRefByID(FOREVER,i+2,ctrl.pblock);
	names = ctrl.names;
	names.Duplicate();
	weightClip = ctrl.weightClip;
	active = ctrl.active;
	dummy.Init(this);
	paramFlags = 0;
	dlg = 0;
	}

ListControl::~ListControl()
	{
	DeleteAllRefsFromMe();
	//RK:12/15/00, for maxscript to get notified when the list controller is deleted
	dummy.NotifyDependents(FOREVER,0,REFMSG_TARGET_DELETED,NOTIFY_ALL,TRUE,&dummy);
	names.Free();
	}

void ListControl::Resize(int c)
	{
	int pc = conts.Count();	
	conts.SetCount(c);
	names.SetCount(c);
	pblock->EnableNotifications(FALSE);
	pblock->SetCount(kListCtrlWeight, c);
	// pblock->EnableNotifications(TRUE);		// CAL-10/18/2002: keep it disabled here (459225)
	for (int i=pc; i<c; i++) {
		conts[i] = NULL;
		names[i] = NULL;
		pblock->SetValue(kListCtrlWeight, 0, 1.0f, i);
		}
	pblock->EnableNotifications(TRUE);			// CAL-10/18/2002: enable it here
	}

ListControl& ListControl::operator=(const ListControl& ctrl)
	{	
	for (int i=0; i<ctrl.conts.Count(); i++) {
		ReplaceReference(i,ctrl.conts[i]->Clone());
		}
	ReplaceReference(i+2, ctrl.pblock->Clone());
	names = ctrl.names;
	names.Duplicate();
	weightClip = ctrl.weightClip;
	active = ctrl.active;
	return *this;
	}

Interval ListControl::GetTimeRange(DWORD flags)
	{
	Interval range = NEVER;
	for (int i=0; i<conts.Count(); i++) {
		if (!i) range = conts[i]->GetTimeRange(flags);
		else {
			Interval iv = conts[i]->GetTimeRange(flags);
			if (!iv.Empty()) {
				if (!range.Empty()) {
					range += iv.Start();
					range += iv.End();
				} else {
					range = iv;
					}
				}
			}
		}
	return range;
	}

RefTargetHandle ListControl::Clone(RemapDir& remap)
	{
	ListControl *ctrl = DerivedClone();

	ctrl->Resize(conts.Count());
	for (int i=0; i<conts.Count(); i++) {
		ctrl->ReplaceReference(i,remap.CloneRef(conts[i]));
		}
	ctrl->ReplaceReference(i+1,remap.CloneRef(clip));
	ctrl->ReplaceReference(i+2, pblock->Clone(remap));
	ctrl->active = active;
	ctrl->names  = names;
	ctrl->names.Duplicate();
	ctrl->weightClip = weightClip;
	BaseClone(this, ctrl, remap);
	return ctrl;
	}

void ListControl::Copy(Control *from)
	{
	if (from->ClassID() != Class_ID(DUMMY_CONTROL_CLASS_ID,0)) {
		Resize(conts.Count()+1);
		MakeRefByID(FOREVER,conts.Count()-1,from);
		}
	}

void ListControl::CommitValue(TimeValue t)
	{	
	if (!conts.Count()) return;
	assert(active>=0);
	conts[active]->CommitValue(t);
	}

void ListControl::RestoreValue(TimeValue t)
	{
	if (!conts.Count()) return;
	assert(active>=0);
	conts[active]->RestoreValue(t);
	}

int ListControl::IsKeyable()
	{
	if (!conts.Count()) return 0;
	assert(active>=0);
	// CAL-10/18/2002: conts[active] could be null while undo. (459225)
	return conts[active] && conts[active]->IsKeyable();
	}

void ListControl::SetValue(TimeValue t, void *val, int commit, GetSetMethod method)
	{
	if (!conts.Count()) return;
	// Note: if method is CTRL_ABSOLUTE this will not do the right thing.
	// need to pass in: Inverse(beforeControllers) * val * Inverse(afterControllers)
	//
	// RB 11/28/2000: Actually to update the above comment... the transformation should be:
	// Inverse(afterControllers) * val * Inverse(beforeControllers)
	assert(active>=0);
	conts[active]->SetValue(t,val,commit,method);
	}

void ListControl::EnumIKParams(IKEnumCallback &callback)
	{
	if (!conts.Count()) return;
	assert(active>=0);
	conts[active]->EnumIKParams(callback);
	}

BOOL ListControl::CompDeriv(TimeValue t,Matrix3& ptm,IKDeriv& derivs,DWORD flags)
	{
	if (!conts.Count()) return FALSE;
	// Note: ptm is not correct if there are controllers before
	assert(active>=0);
	return conts[active]->CompDeriv(t,ptm,derivs,flags);
	}

void ListControl::MouseCycleCompleted(TimeValue t)
	{
	if (!conts.Count()) return;
	assert(active>=0);
	conts[active]->MouseCycleCompleted(t);
	}

// ambarish added this method on 11/28/2000
// the purpose is to ensure that the controller that doesn't want to inherit parent transform doesn't do so
BOOL ListControl::InheritsParentTransform(){
	if (!conts.Count()) return FALSE;
//	int activeControlNumber =  GetActive();
	for (int i=0; i <= GetActive(); ++i){ // loop through each item in the list up to (and including) the active item
		if (conts[i]->InheritsParentTransform() == FALSE){
			return FALSE;
		}
	}
	return TRUE;

}

void ListControl::AddNewKey(TimeValue t,DWORD flags)
	{
	for (int i=0; i<conts.Count(); i++) {
		conts[i]->AddNewKey(t,flags);
		}
	}

void ListControl::CloneSelectedKeys(BOOL offset)
	{
	if (!conts.Count()) return;
	assert(active>=0);
	conts[active]->CloneSelectedKeys(offset);
	}

void ListControl::DeleteKeys(DWORD flags)
	{
	if (!conts.Count()) return;
	assert(active>=0);
	conts[active]->DeleteKeys(flags);
	}

void ListControl::SelectKeys(TrackHitTab& sel, DWORD flags)
	{
	if (!conts.Count()) return;
	assert(active>=0);
	conts[active]->SelectKeys(sel,flags);
	}

BOOL ListControl::IsKeySelected(int index)
	{
	if (!conts.Count()) return FALSE;
	assert(active>=0);
	return conts[active]->IsKeySelected(index);
	}

void ListControl::CopyKeysFromTime(TimeValue src,TimeValue dst,DWORD flags)
	{
	if (!conts.Count()) return;
	assert(active>=0);
	conts[active]->CopyKeysFromTime(src,dst,flags);
	}

void ListControl::DeleteKeyAtTime(TimeValue t)
	{
	if (!conts.Count()) return;
	assert(active>=0);
	conts[active]->DeleteKeyAtTime(t);
	}

BOOL ListControl::IsKeyAtTime(TimeValue t,DWORD flags)
	{
	if (!conts.Count()) return FALSE;
	assert(active>=0);
	return conts[active]? conts[active]->IsKeyAtTime(t,flags) : FALSE;
	}

BOOL ListControl::GetNextKeyTime(TimeValue t,DWORD flags,TimeValue &nt)
	{
	if (!conts.Count()) return FALSE;
	assert(active>=0);
	return conts[active]->GetNextKeyTime(t,flags,nt);
	}

int ListControl::GetKeyTimes(Tab<TimeValue> &times,Interval range,DWORD flags)
	{
	if (!conts.Count()) return 0;
	assert(active>=0);
	return conts[active]->GetKeyTimes(times,range,flags);
	}

int ListControl::GetKeySelState(BitArray &sel,Interval range,DWORD flags)
	{
	if (!conts.Count()) return 0;
	assert(active>=0);
	return conts[active]->GetKeySelState(sel,range,flags);
	}

RefResult ListControl::NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message )
	{
	switch (message) {
		case REFMSG_WANT_SHOWPARAMLEVEL:// show the paramblock2 entry in trackview
			{
			BOOL *pb = (BOOL *)(partID);
			*pb = TRUE;
			return REF_STOP;
			}
		}

	return REF_SUCCEED;
	}

void ListControl::NotifyForeground(TimeValue t)
	{
	if (!conts.Count()) return;
	assert(active>=0);
	conts[active]->NotifyForeground(t);
	}

Animatable* ListControl::SubAnim(int i)
	{
	if (i==conts.Count()+1)
		return pblock;
	if (i==conts.Count()) {
		return &dummy;
	} else {
		return conts[i];
		}
	}

TSTR ListControl::SubAnimName(int i)	
	{
	TSTR name;
	if (i==conts.Count()+1) {
		return GetString(IDS_AF_LIST_WEIGHTS); // the parameter block
		} 
	if (i==conts.Count()) {
		return GetString(IDS_RB_AVAILABLE);
		} 
	else {
		if (names[i] && names[i]->length()) {
			name = *names[i];
		} else if (conts[i]) {
			conts[i]->GetClassName(name);
			} 
		else {
			name = GetString(IDS_RB_AVAILABLE);
			}
		}
	return name;
	}

int ListControl::SubNumToRefNum(int subNum)
	{
	if (subNum<=conts.Count()) return subNum;
	if (subNum == conts.Count()+1) return subNum+1; // the parameter block
	return -1;
	}

RefTargetHandle ListControl::GetReference(int i)
	{
	if (i<conts.Count()) return conts[i];	//controllers in the list
	if (i==conts.Count()) return NULL;		//available dummy controller
	if (i==conts.Count()+1) return clip;	//controller in the copy buffer
	if (i==conts.Count()+2) return pblock;	//parameter block
	return NULL;
	}

void ListControl::SetReference(int i, RefTargetHandle rtarg)
	{
	if (i == conts.Count()+2 || 
	 (rtarg && rtarg->ClassID() == Class_ID(PARAMETER_BLOCK2_CLASS_ID,0)))
		{
		//set the parameter block
		pblock = (IParamBlock2*)rtarg;
		return;
		}

	if (i == conts.Count()+1)	//copying a controller to the buffer
		{
		clip=(Control*)rtarg;
		return;
		}
	
	if (i==conts.Count())		//pasting onto the available slot, grow the list by one
		{
		if (theHold.Holding()) {
			theHold.Put(new ListSizeRestore(this));
			}
		Resize(conts.Count()+1);
		conts[conts.Count()-1] = (Control*)rtarg;
		if (!rtarg) names[conts.Count()-1] = NULL;
		return;
		}

	if (i>=conts.Count()) return;  //if you make it here it is out of bounds...
/*
	if (!rtarg) {
		conts.Delete(i,1);
		names.Delete(i,1);
		pblock->Delete(kListCtrlWeight, i, 1);
		}
	else
*/
	conts[i] = (Control*)rtarg;	   //replacing a sub-controller
	if (!rtarg) names[i] = NULL;
	return;
	}

ParamDimension* ListControl::GetParamDimension(int i)
	{
	ParamDimension *dim = defaultDim;
	NotifyDependents(FOREVER, (PartID)&dim, REFMSG_GET_CONTROL_DIM);
	return dim;
	}

BOOL ListControl::AssignController(Animatable *control,int subAnim)
	{
	// AF (04/17/02) the available controller used to be last sub 
	// so some existing controllers may be making that assumption.
	// never want assign controller to be called on the pblock track 
	// so just remap the subAnim index
	if (subAnim > conts.Count()) 
		subAnim = conts.Count(); 

	if (control==&dummy) return TRUE;
	if (subAnim==conts.Count()) {
		if (theHold.Holding()) {
			theHold.Put(new ListSizeRestore(this));
			}
		Resize(conts.Count()+1);
		}		
	if (active<0) active = 0;
	ReplaceReference(SubNumToRefNum(subAnim),(RefTargetHandle)control);
	NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	return TRUE;
	}


#define LISTCOUNT_CHUNK		0x01010
#define LISTACTIVE_CHUNK	0x01020
#define ITEMNAME_CHUNK		0x01030
#define NONAME_CHUNK		0x01040

IOResult ListControl::Save(ISave *isave)
	{
	ULONG nb;
	int count = conts.Count();
	isave->BeginChunk(LISTCOUNT_CHUNK);
	isave->Write(&count,sizeof(int),&nb);			
	isave->EndChunk();

	isave->BeginChunk(LISTACTIVE_CHUNK);
	isave->Write(&active,sizeof(active),&nb);			
	isave->EndChunk();

	for (int i=0; i<count; i++) {
		if (names[i]) {
			isave->BeginChunk(ITEMNAME_CHUNK);
			isave->WriteWString(*names[i]);
			isave->EndChunk();
		} else {
			isave->BeginChunk(NONAME_CHUNK);
			isave->EndChunk();
			}
		}

	return IO_OK;
	}

class ListCtrlPostLoad : public PostLoadCallback 
	{
	public:
		ListControl *listCtrl;
	
		ListCtrlPostLoad(ListControl *lc) 
			{
			listCtrl = lc;
			}
		void proc(ILoad *iload) 
			{
			// fill in the weight list when bringing in old files
			int listCount = listCtrl->pblock->Count(kListCtrlWeight);
			listCtrl->pblock->EnableNotifications(FALSE);
			listCtrl->pblock->SetCount(kListCtrlWeight, listCtrl->GetListCount());
			for(int i=listCount;i<listCtrl->GetListCount();i++)
				{
				listCtrl->pblock->SetValue(kListCtrlWeight, 0, 1.0f, i);
				}
			listCtrl->pblock->EnableNotifications(TRUE);
			delete this; 
			}
	};


IOResult ListControl::Load(ILoad *iload)
	{
	ULONG nb;
	IOResult res = IO_OK;
	int ix=0;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case LISTCOUNT_CHUNK: {
				int count;
				res = iload->Read(&count,sizeof(count),&nb);
				Resize(count);
				break;
				}

			case LISTACTIVE_CHUNK:
				res = iload->Read(&active,sizeof(active),&nb);				
				break;

			case ITEMNAME_CHUNK: {
				TCHAR *buf;
				iload->ReadWStringChunk(&buf);
				names[ix++] = new TSTR(buf);
				break;
				}

			case NONAME_CHUNK:
				ix++;
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}
	iload->RegisterPostLoadCallback(new ListCtrlPostLoad(this));
	return IO_OK;
	}

//Added by AF (08/29/00)
//************************************************
void ListControl::SetActive(int a)
	{
	if (a < 0 || a >= conts.Count()) {
		throw MAXException(GetString(IDS_AF_INDEX_ERROR));
		return;
	}

	if (dlg) {
		if (conts.Count()) {
			conts[active]->EndEditParams(
				dlg->ip,END_EDIT_REMOVEUI,conts[a]);
			}
		if (conts.Count()) {
			conts[a]->BeginEditParams(
				dlg->ip,paramFlags,conts[active]);
			}
		}
	active = a;
	NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	}


void ListControl::DeleteItem(int a)
	{	
	if (a < 0 || a >= conts.Count()) {
		throw MAXException(GetString(IDS_AF_INDEX_ERROR));
		return;
	}

	if (dlg) dlg->EndEdit(active);
	ListControlRestore *rest = new ListControlRestore(this);
	theHold.Begin();
	theHold.Put(new RemoveItemRestore(this,0));
	DeleteReference(a);
	conts.Delete(a,1);
	names.Delete(a,1);
	pblock->Delete(kListCtrlWeight, a, 1);
	if (active > a) active--;
	if (active > conts.Count()-1) active = conts.Count()-1;
	theHold.Put(rest);
	NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	theHold.Put(new RemoveItemRestore(this,1));
	theHold.Accept(GetString(IDS_RB_DELETECONTROLLER));
	if (dlg) dlg->StartEdit(active);
	}

void ListControl::CutItem(int a)
	{
	if (a < 0 || a >= conts.Count()) {
		throw MAXException(GetString(IDS_AF_INDEX_ERROR));
		return;
	}

	if (dlg) dlg->EndEdit(active);
	ListControlRestore *rest = new ListControlRestore(this);	
	ReplaceReference(conts.Count()+1,conts[a]);
	if (names[a]) nameClip = *names[a];
	else nameClip = _T("");
	pblock->GetValue(kListCtrlWeight, GetCOREInterface()->GetTime(), weightClip, FOREVER, a);
	theHold.Begin();
	theHold.Put(new RemoveItemRestore(this,0));
	theHold.Put(new CutRestore(conts[a]));
	DeleteReference(a);
	conts.Delete(a,1);
	names.Delete(a,1);
	pblock->Delete(kListCtrlWeight, a, 1);
	if (active > a) active--;
	if (active > conts.Count()-1) active = conts.Count()-1;	
	theHold.Put(rest);
	NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	theHold.Put(new RemoveItemRestore(this,1));
	theHold.Accept(GetString(IDS_RB_CUTCONTROLLER));
	if (dlg) dlg->StartEdit(active);
	}

void ListControl::PasteItem(int a)
	{	
	if (a < 0 || a > conts.Count()) {
		throw MAXException(GetString(IDS_AF_INDEX_ERROR));
		return;
	}
	
	if (!clip) return;

	if (dlg) dlg->EndEdit(active);
	theHold.Begin();
	theHold.Put(new RemoveItemRestore(this,0));
	theHold.Put(new ListControlRestore(this));
	if (a == conts.Count())
		conts.Append(1, (Control**)&clip);
	else 
		conts.Insert(a,1,(Control**)&clip);	
	TSTR *ptr = new TSTR(nameClip);
	names.Insert(a,1,&ptr);	
	pblock->Insert(kListCtrlWeight, a, 1, &weightClip);
	MakeRefByID(FOREVER,a,clip);	
	if (active >= a) active++;	
	NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	theHold.Put(new RemoveItemRestore(this,1));
	theHold.Accept(GetString(IDS_RB_PASTECONTROLLER));
	DeleteReference(conts.Count()+1);
	clip = NULL;	
	if (active<0) active = 0;
	if (dlg) dlg->StartEdit(active);
	}

void ListControl::SetName(int index, TSTR name)
	{
	if (index < 0 || index >= names.Count()) {
		throw MAXException(GetString(IDS_AF_INDEX_ERROR));
		return;
		}

	if (!(names[index])) 
		names[index] = new TSTR;
	*(names[index]) = name;
	NotifyDependents(FOREVER,0,REFMSG_NODE_NAMECHANGE);

	if (dlg) dlg->UpdateList();
	}


TSTR ListControl::GetName(int index)
	{
	TSTR name;
	if (index < 0 || index >= conts.Count()) {
		throw MAXException(GetString(IDS_AF_INDEX_ERROR));
		return _T("");
		}
	if (names[index]) return *(names[index]);
	else {
		name = SubAnimName(index);
		return name;
		}
	}

//***************************************************************


//-------------------------------------------------------------------------
// UI stuff

ListControlDlg::ListControlDlg(IObjParam *i,ListControl *c)
	{
	// CAL-9/23/2002: don't want to put a MakeRefRestore into the undo stack. (436460)
	theHold.Suspend();
	MakeRefByID(FOREVER,0,c);
	theHold.Resume();
	
	ip = i;
	valid = FALSE;
	hWnd  = NULL;
	GetCOREInterface()->RegisterTimeChangeCallback(this);
	}

ListControlDlg::~ListControlDlg()
	{
	UnRegisterListCtrlWindow(hWnd);
	if (cont->dlg==this) {
		if (cont->conts.Count()) {
			cont->conts[cont->active]->EndEditParams(
				ip,END_EDIT_REMOVEUI,NULL);
			}
		}
	DeleteAllRefsFromMe();
	GetCOREInterface()->UnRegisterTimeChangeCallback(this);
	}
		
void ListControlDlg::Reset(IObjParam *i,ListControl *c)
	{
	if (cont->dlg==this) {
		if (cont->conts.Count()) {
			cont->conts[cont->active]->EndEditParams(
				i,END_EDIT_REMOVEUI,NULL);
			}
		if (c->conts.Count()) {
			c->conts[cont->active]->BeginEditParams(
				i,c->paramFlags,NULL);
			}
		}
	ReplaceReference(0,c);
	ip = i;
	Invalidate();
	}

void ListControlDlg::Init(HWND hParent)
	{
	hWnd = CreateWin(hParent);
	SetupList();
	SetButtonStates();
	if (cont->dlg==this) {
		if (cont->conts.Count()) {
			if (cont->active < 0) cont->active = 0;
			cont->conts[cont->active]->BeginEditParams(
				ip,cont->paramFlags,NULL);
			}
		}
	}

void ListControlDlg::Invalidate()
	{
	valid = FALSE;
	Rect rect(IPoint2(0,0),IPoint2(10,10));
	InvalidateRect(hWnd,&rect,FALSE);
	}

void ListControlDlg::Update()
	{
	if (!valid && hWnd) {
		// CAL-10/7/2002: Check if the weight count and list count are consistent. When undoing
		// list controller assignment they will go out of sync and it's time to close the window.
		if (!cont->pblock || cont->pblock->Count(cont->pblock->IndextoID(kListCtrlWeight)) != cont->conts.Count()) {
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			return;
		}
		SetupUI();
		valid = TRUE;		
		}
	}

void ListControlDlg::UpdateList()
	{
	
	HWND hList = GetDlgItem(hWnd, IDC_CONTROLLER_WEIGHT_LIST);

	for (int i=0; i<cont->conts.Count(); i++) {
		TSTR name = cont->SubAnimName(i);
		if (i==cont->active) {
			name = TSTR(_T("->")) + name;
		} else {
			name = TSTR(_T("  ")) + name;
			}

		if (ListView_GetItemCount(hList) <= i)
			{
			LV_ITEM item;
			item.mask = LVIF_TEXT;
			item.iItem = i;
			item.iSubItem = 0;
			item.pszText = name;
			item.cchTextMax = _tcslen(name);
			ListView_InsertItem(hList,&item);

			item.iSubItem = 1;
			name.printf(_T("%.1f"), cont->pblock->GetFloat(kListCtrlWeight, GetCOREInterface()->GetTime(), i)*100.0f);
			item.pszText = name;
			item.cchTextMax = _tcslen(name);
			ListView_SetItemText(hList, i, 1, name);
			}
		else {
			ListView_SetItemText(hList, i, 0, name);
			name.printf(_T("%.1f"), cont->pblock->GetFloat(kListCtrlWeight, GetCOREInterface()->GetTime(), i)*100.0f);
			ListView_SetItemText(hList, i, 1, name);
			}
		}
	int itemCt = ListView_GetItemCount(hList);	for (int x = itemCt; x > i; x--)
		ListView_DeleteItem(hList, x-1);
  }

void ListControlDlg::SetupList()
	{
	
	HWND hList = GetDlgItem(hWnd, IDC_CONTROLLER_WEIGHT_LIST);
	int sel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);

	LV_COLUMN column;
	column.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;

	column.fmt = LVCFMT_LEFT;
	column.pszText = GetString(IDS_AF_LIST_ENTRY);
	column.cx = 90;
	ListView_InsertColumn(hList, 2, &column);

	column.pszText = GetString(IDS_AF_LIST_WEIGHT);
	column.cx = 50;
	ListView_InsertColumn(hList, 1, &column);

	UpdateList();

	/*

	LRESULT sel = SendMessage(GetDlgItem(hWnd,IDC_CONTROLLER_LIST),
				LB_GETCURSEL,0,0);
	SendMessage(GetDlgItem(hWnd,IDC_CONTROLLER_LIST),
				LB_RESETCONTENT,0,0);
	for (int i=0; i<cont->conts.Count(); i++) {
		TSTR name = cont->SubAnimName(i);
		if (i==cont->active) {
			name = TSTR(_T("->")) + name;
		} else {
			name = TSTR(_T("  ")) + name;
			}
		SendMessage(GetDlgItem(hWnd,IDC_CONTROLLER_LIST),
			LB_ADDSTRING,0,(LPARAM)(TCHAR*)name);
		}
	if (sel!=LB_ERR) {
		SendMessage(GetDlgItem(hWnd,IDC_CONTROLLER_LIST),
			LB_SETCURSEL,(WPARAM)sel,0);
	} else {
		SendMessage(GetDlgItem(hWnd,IDC_CONTROLLER_LIST),
				LB_SETCURSEL,(WPARAM)-1,0);
		}
*/
  }

void ListControlDlg::SetupUI()
	{
	UpdateList();
	SetButtonStates();
	}

void ListControlDlg::StartEdit(int a)
	{
	if (cont->dlg) {
		if (cont->conts.Count()) {
			cont->conts[a]->BeginEditParams(
				ip,cont->paramFlags,NULL);
			}
		}
	}

void ListControlDlg::EndEdit(int a)
	{
	if (cont->dlg) {
		if (cont->conts.Count()) {			
			cont->conts[a]->EndEditParams(
				ip,END_EDIT_REMOVEUI,NULL);				
			}
		}
	}
/*
void ListControlDlg::SetActive(int a)
	{
	if (cont->dlg) {
		if (cont->conts.Count()) {
			cont->conts[cont->active]->EndEditParams(
				ip,END_EDIT_REMOVEUI,cont->conts[a]);
			}
		if (cont->conts.Count()) {
			cont->conts[a]->BeginEditParams(
				ip,cont->paramFlags,cont->conts[cont->active]);
			}
		}
	cont->active = a;
	cont->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	}


void ListControlDlg::DeleteItem(int a)
	{	
	EndEdit(cont->active);
	ListControlRestore *rest = new ListControlRestore(cont);
	theHold.Begin();
	theHold.Put(new RemoveItemRestore(cont,0));
	cont->DeleteReference(a);
	cont->conts.Delete(a,1);
	cont->names.Delete(a,1);
	if (cont->active > a) cont->active--;
	if (cont->active > cont->conts.Count()-1) cont->active = cont->conts.Count()-1;
	theHold.Put(rest);
	cont->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	cont->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	theHold.Put(new RemoveItemRestore(cont,1));
	theHold.Accept(IDS_RB_DELETECONTROLLER);
	StartEdit(cont->active);
	}

void ListControlDlg::CutItem(int a)
	{
	EndEdit(cont->active);
	ListControlRestore *rest = new ListControlRestore(cont);	
	cont->ReplaceReference(cont->conts.Count(),cont->conts[a]);
	if (cont->names[a]) cont->nameClip = *cont->names[a];
	else cont->nameClip = _T("");
	theHold.Begin();
	theHold.Put(new RemoveItemRestore(cont,0));
	theHold.Put(new CutRestore(cont->conts[a]));
	cont->DeleteReference(a);
	cont->conts.Delete(a,1);
	if (cont->active > a) cont->active--;
	if (cont->active > cont->conts.Count()-1) cont->active = cont->conts.Count()-1;	
	theHold.Put(rest);
	cont->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	cont->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	theHold.Put(new RemoveItemRestore(cont,1));
	theHold.Accept(IDS_RB_CUTCONTROLLER);
	StartEdit(cont->active);
	}

void ListControlDlg::PasteItem(int a)
	{	
	EndEdit(cont->active);
	theHold.Begin();
	theHold.Put(new RemoveItemRestore(cont,0));
	theHold.Put(new ListControlRestore(cont));
	cont->conts.Insert(a,1,(Control**)&(cont->clip));	
	TSTR *ptr = new TSTR(cont->nameClip);
	cont->names.Insert(a,1,&ptr);	
	cont->MakeRefByID(FOREVER,a,cont->clip);	
	if (cont->active >= a) cont->active++;	
	cont->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	cont->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	theHold.Put(new RemoveItemRestore(cont,1));
	theHold.Accept(IDS_RB_PASTECONTROLLER);
	cont->DeleteReference(cont->conts.Count());
	cont->clip = NULL;	
	if (cont->active<0) cont->active = 0;
	StartEdit(cont->active);
	}
*/
void ListControlDlg::SetButtonStates()
	{
	HWND hList = GetDlgItem(hWnd, IDC_CONTROLLER_WEIGHT_LIST);
	int sel = ListView_GetNextItem(hList, -1, LVNI_SELECTED); //ListView_GetItemState(hList, m_hit.x, LVIS_SELECTED);
//	LRESULT sel = SendMessage(GetDlgItem(hWnd,IDC_CONTROLLER_LIST),
//		LB_GETCURSEL,0,0);
	ICustEdit *iName = GetICustEdit(GetDlgItem(hWnd,IDC_LIST_NAME));
	ISpinnerControl *iWeight = GetISpinner(GetDlgItem(hWnd,IDC_LIST_WEIGHTSPIN));
	iWeight->SetLimits(-10000.0f,10000.0f,FALSE);
	iWeight->SetScale(1.0f);
	iWeight->LinkToEdit(GetDlgItem(hWnd,IDC_LIST_WEIGHT),EDITTYPE_FLOAT);
	
	if (sel >= cont->conts.Count())
		sel = cont->conts.Count()-1;

	if (sel!=LB_ERR) {
		EnableWindow(GetDlgItem(hWnd,IDC_LIST_SETACTIVE),TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_LIST_DELETE),TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_LIST_CUT),TRUE);		
		iName->Enable();
		if (cont->names[sel]) iName->SetText(*cont->names[sel]);
		else iName->SetText(_T(""));
		
		float weight = 0.0f;
		cont->pblock->GetValue(kListCtrlWeight, GetCOREInterface()->GetTime(), weight, FOREVER, sel);
		iWeight->Enable();
		iWeight->SetValue(weight*100.0f, 0);
		iWeight->SetKeyBrackets(cont->pblock->KeyFrameAtTime(kListCtrlWeight, GetCOREInterface()->GetTime(), sel));
	} else {
		EnableWindow(GetDlgItem(hWnd,IDC_LIST_SETACTIVE),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_LIST_DELETE),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_LIST_CUT),FALSE);		
		iName->SetText(_T(""));
		iName->Disable();
		iWeight->SetValue(0.0f, 0);
		iWeight->Disable();
		iWeight->SetKeyBrackets(FALSE);
		}
	if (cont->clip) {
		EnableWindow(GetDlgItem(hWnd,IDC_LIST_PASTE),TRUE);
	} else {
		EnableWindow(GetDlgItem(hWnd,IDC_LIST_PASTE),FALSE);
		}
	ReleaseISpinner(iWeight);
	ReleaseICustEdit(iName);

	BOOL average = FALSE;
	cont->pblock->GetValue(kListCtrlAverage, 0, average, FOREVER);
	CheckDlgButton(hWnd, IDC_AVERAGE_WEIGHTS, average);
}

void ListControlDlg::WMCommand(int id, int notify, HWND hCtrl)
	{
	HWND hList = GetDlgItem(hWnd, IDC_CONTROLLER_WEIGHT_LIST);
	int sel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
	if (sel >= cont->conts.Count()) 
		sel = - 1;
	switch (id) {
		case IDC_LIST_NAME: {
			//LRESULT sel = SendMessage(GetDlgItem(hWnd,IDC_CONTROLLER_LIST),
			//	LB_GETCURSEL,0,0);
			if (sel>=0) {
				TCHAR buf[256];
				ICustEdit *iName = GetICustEdit(GetDlgItem(hWnd,IDC_LIST_NAME));
				if (iName) {
					iName->GetText(buf,256);
					cont->SetName(sel, buf);
					}
				UpdateList();
				ReleaseICustEdit(iName);
				}
			break;
			}

		case IDC_LIST_SETACTIVE: {			
			//int sel = SendMessage(GetDlgItem(hWnd,IDC_CONTROLLER_LIST),
			//	LB_GETCURSEL,0,0);
			if (sel!=LB_ERR) {
				cont->SetActive(sel);
				}
			break;
			}

		case IDC_LIST_DELETE: {
			//int sel = SendMessage(GetDlgItem(hWnd,IDC_CONTROLLER_LIST),
			//	LB_GETCURSEL,0,0);
			if (sel!=LB_ERR) {
				cont->DeleteItem(sel);	
				ip->RedrawViews(ip->GetTime());
				}
			break;
			}

		case IDC_LIST_CUT: {
			if (sel!=LB_ERR) {
				cont->CutItem(sel);
				ip->RedrawViews(ip->GetTime());
				}
			break;
			}

		case IDC_LIST_PASTE: {
			if (sel!=LB_ERR) {
				cont->PasteItem(sel);
			} else {
				cont->PasteItem(cont->conts.Count());
				}
			ip->RedrawViews(ip->GetTime());
			break;
			}
		case IDC_AVERAGE_WEIGHTS: {
			BOOL average = IsDlgButtonChecked(hWnd, IDC_AVERAGE_WEIGHTS);
			cont->pblock->SetValue(kListCtrlAverage, 0, average);
			cont->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
			break;
			}
		}
	}


int ListControlDlg::NumRefs() 
	{
	return 1;
	}

RefTargetHandle ListControlDlg::GetReference(int i) 
	{
	switch (i) {
		case LISTDLG_CONTREF:
			return cont;
		//case LISTDLG_CLIPREF:
		//	return clip;
		default:
			return NULL;
		}
	}

void ListControlDlg::SetReference(int i, RefTargetHandle rtarg) 
	{
	switch (i) {
		case LISTDLG_CONTREF:
			cont=(ListControl*)rtarg;
			break;
	//	case LISTDLG_CLIPREF:
	//		clip=(ListControl*)rtarg;
	//		break;
		}
	}

RefResult ListControlDlg::NotifyRefChanged(
		Interval iv, 
		RefTargetHandle rtarg, 
		PartID& partID, 
		RefMessage message)
	{
	switch (message) {
		case REFMSG_CHANGE:
			Invalidate();
			break;
				
		case REFMSG_REF_DELETED:
			MaybeCloseWindow();
			break;
		
		}
	return REF_SUCCEED;
	}


static INT_PTR CALLBACK ListControlDlgProc(
		HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
	{
			// WIN64 Cleanup: Shuler
	ListControlDlg *ld = (ListControlDlg*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
			// WIN64 Cleanup: Shuler
	
	static undoHeldHere = FALSE; //AF -- this flag is used to trigger an undo begin in the even of a WM_CUSTEDIT_ENTER
	
	switch (message) {
		case WM_INITDIALOG: { 			
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
					// WIN64 Cleanup: Shuler
			ld = (ListControlDlg*)lParam;
			if (ld->cont->SuperClassID() == CTRL_ROTATION_CLASS_ID)
				SetDlgItemText(hWnd, IDC_AVERAGE_WEIGHTS, GetString(IDS_AF_LIST_POSE_TO_POSE));
			break;
			}
		case WM_NOTIFY:
		{
			int idCtrl = (int)wParam;
			LPNMHDR pnmh = (LPNMHDR)lParam;

			switch( idCtrl )
			{
				case IDC_CONTROLLER_WEIGHT_LIST:
				{
					switch(pnmh->code)
					{
						case LVN_ITEMCHANGED:
							{
							if ( !(((NMLISTVIEW *)lParam)->uChanged & LVIF_STATE) )
								return 0;

							if ( (((NMLISTVIEW *)lParam)->uNewState & LVIS_SELECTED) || (((NMLISTVIEW *)lParam)->uOldState & LVIS_SELECTED) )
								ld->SetButtonStates();				
							}
							return 0;
						case NM_DBLCLK:
							if (((LPNMITEMACTIVATE)lParam)->iItem >= 0 && ((LPNMITEMACTIVATE)lParam)->iItem < ld->cont->conts.Count())
							{
								ld->cont->SetActive(((LPNMITEMACTIVATE)lParam)->iItem);
							}
							return 1;
					}
				}
			}
			break;
		}

		case WM_COMMAND:
			ld->WMCommand(LOWORD(wParam),HIWORD(wParam),(HWND)lParam);						
			break;
		case CC_SPINNER_BUTTONDOWN:
			theHold.Begin();
			undoHeldHere = TRUE;
			break;
		case CC_SPINNER_CHANGE: {
			if (!undoHeldHere) {theHold.Begin();undoHeldHere = TRUE;}
			TimeValue t = GetCOREInterface()->GetTime();
			switch (LOWORD(wParam)) 
			{
				case IDC_LIST_WEIGHTSPIN: {
					HWND hList = GetDlgItem(hWnd, IDC_CONTROLLER_WEIGHT_LIST);
					int sel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
					ISpinnerControl* iWeight = GetISpinner(GetDlgItem(hWnd,IDC_LIST_WEIGHTSPIN));
					if (sel!=LB_ERR)
						ld->cont->pblock->SetValue(kListCtrlWeight, t, iWeight->GetFVal()/100.0f, sel);
					ReleaseISpinner(iWeight);
					break;
					}
				default:
					break;
			}
			ld->cont->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(t);
			break;
			}
		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP:
			if (HIWORD(wParam) || message==WM_CUSTEDIT_ENTER)
				theHold.Accept(GetString(IDS_AF_LIST_WEIGHT_UNDO));
			else theHold.Cancel();
			undoHeldHere = FALSE;
			break;
		case WM_PAINT:
			ld->Update();
			return 0;			
		
		case WM_CLOSE:
			DestroyWindow(hWnd);			
			break;

		case WM_DESTROY:						
			delete ld;
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:			
			ld->MouseMessage(message,wParam,lParam);
			return FALSE;

		default:
			return 0;
		}
	return 1;
	}


HWND ListControlMotionDlg::CreateWin(HWND hParent)
	{
	TSTR name;
	cont->GetClassName(name);
	return ip->AddRollupPage( 
			hInstance, 
			MAKEINTRESOURCE(IDD_LISTPARAMS_MOTION),
			ListControlDlgProc,
			name,
			(LPARAM)this);		
	}

HWND ListControlTrackDlg::CreateWin(HWND hParent)
	{
	return CreateDialogParam(
		hInstance,
		MAKEINTRESOURCE(IDD_LISTPARAMS_TRACK),
		hParent,
		ListControlDlgProc,
		(LPARAM)this);
	}

class CheckForNonListDlg : public DependentEnumProc {
	public:		
		BOOL non;
		ReferenceMaker *me;
		CheckForNonListDlg(ReferenceMaker *m) {non = FALSE;me = m;}
		int proc(ReferenceMaker *rmaker) {
			if (rmaker==me) return 0;
			if (rmaker->SuperClassID()!=REF_MAKER_CLASS_ID &&
				rmaker->ClassID()!=Class_ID(LISTDLG_CLASS_ID,0)) {
				non = TRUE;
				return 1;
				}
			return 0;
			}
	};

void ListControlTrackDlg::MaybeCloseWindow()
	{
	CheckForNonListDlg check(cont);
	cont->EnumDependents(&check);
	if (!check.non) {
		PostMessage(hWnd,WM_CLOSE,0,0);
		}
	}


class ListCtrlWindow {
	public:
		HWND hWnd;
		HWND hParent;
		Control *cont;
		ListCtrlWindow() {assert(0);}
		ListCtrlWindow(HWND hWnd,HWND hParent,Control *cont)
			{this->hWnd=hWnd; this->hParent=hParent; this->cont=cont;}
	};
static Tab<ListCtrlWindow> listCtrlWindows;

static void RegisterListCtrl(HWND hWnd, HWND hParent, Control *cont)
	{
	ListCtrlWindow rec(hWnd,hParent,cont);
	listCtrlWindows.Append(1,&rec);
	}

static void UnRegisterListCtrlWindow(HWND hWnd)
	{	
	for (int i=0; i<listCtrlWindows.Count(); i++) {
		if (hWnd==listCtrlWindows[i].hWnd) {
			listCtrlWindows.Delete(i,1);
			return;
			}
		}	
	}

static HWND FindOpenListCtrl(HWND hParent,Control *cont)
	{	
	for (int i=0; i<listCtrlWindows.Count(); i++) {
		if (hParent == listCtrlWindows[i].hParent &&
			cont    == listCtrlWindows[i].cont) {
			return listCtrlWindows[i].hWnd;
			}
		}
	return NULL;
	}

void ListControl::EditTrackParams(
		TimeValue t,
		ParamDimensionBase *dim,
		TCHAR *pname,
		HWND hParent,		
		IObjParam *ip,
		DWORD flags)
	{
	HWND hCur = FindOpenListCtrl(hParent,this);
	if (hCur) {
		SetForegroundWindow(hCur);
		return;
		}

	ListControlTrackDlg *dlg = 
		new ListControlTrackDlg(ip,this);
	dlg->Init(hParent);
	RegisterListCtrl(dlg->hWnd,hParent,this);
	}

void ListControl::BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev)
	{
	if (0) {
	//if (dlg && prev && prev->ClassID()==ClassID()) {
		paramFlags = flags;
		dlg->Reset(ip,this);
	} else {
		paramFlags = flags;
		dlg = new ListControlMotionDlg(ip,this);
		dlg->Init(NULL);
		}
	}

void ListControl::EndEditParams(IObjParam *ip, ULONG flags,Animatable *next)
	{
	//if (!next || next->ClassID()!=ClassID()) {	
		if (dlg) {
			ip->DeleteRollupPage(dlg->hWnd);
			dlg        = NULL;
			}
		paramFlags = 0;
	//	}
	}

float ListControl::AverageWeight(float weight)
	{
	BOOL average = FALSE;
	pblock->GetValue(kListCtrlAverage, 0, average, FOREVER);
	if (average) {
		float tempWeight = 0.0f;
		float totalWeight = 0.0f;;
		for (int i=0;i<conts.Count();i++) {
			pblock->GetValue(kListCtrlWeight, 0, tempWeight, FOREVER, i);
			totalWeight += tempWeight;
			}
		return totalWeight!=0.0f?(average?(weight/totalWeight):weight):0.0f;
		}
	return weight;
	}


//-------------------------------------------------------------------------
// Float list control
//		
		
class FloatListControl : public ListControl {
	public:
		FloatListControl(BOOL loading) : ListControl(loading) {Init();} 
		FloatListControl() {Init();}

		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void SetValue(TimeValue t, void *val, int commit, GetSetMethod method);
		ListControl *DerivedClone();
		void Init();

		Class_ID ClassID() { return Class_ID(FLOATLIST_CONTROL_CLASS_ID,0); }  
		SClass_ID SuperClassID() { return CTRL_FLOAT_CLASS_ID; }  
		void GetClassName(TSTR& s) {s = FLOATLIST_CNAME;}
	};

void FloatListControl::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
	float *v = (float*)val;
	float weight;

	if (method==CTRL_ABSOLUTE) {
		*v = 0.0f;
		}

	// CAL-10/18/2002: conts.Count() and pblock->Count(pblock->IndextoID(kListCtrlWeight)) might go out of sync while undo. (459225)
	if (conts.Count() != pblock->Count(pblock->IndextoID(kListCtrlWeight)))
		return;

	float prevVal;
	for (int i=0; i<conts.Count(); i++) {
		pblock->GetValue(kListCtrlWeight, t, weight, valid, i);
		if (weight != 0.0f) {
			prevVal = *v;
			if (conts[i]) conts[i]->GetValue(t,v,valid,CTRL_RELATIVE);
			*v = prevVal + ((*v) - prevVal)*AverageWeight(weight);
			}
		}
	}

void FloatListControl::SetValue(
		TimeValue t, void *val, int commit, GetSetMethod method)
	{
	if (!conts.Count()) return;	
	float weight;

	if (method==CTRL_ABSOLUTE) {
		float v = *((float*)val);
		pblock->GetValue(kListCtrlWeight, t, weight, FOREVER, active);
		v *= weight;
		float before = 0.0f, after = 0.0f;
		Interval valid;
		float prevVal;
		for (int i=0; i<active; i++) {
			prevVal = before;
			conts[i]->GetValue(t,&before,valid,CTRL_RELATIVE);
			pblock->GetValue(kListCtrlWeight, t, weight, FOREVER, i);
			before = prevVal + (before - prevVal)*AverageWeight(weight);
		//	conts[i]->GetValue(t,&before,valid,CTRL_RELATIVE);
			}
		for (i=active+1; i<conts.Count(); i++) {
			prevVal = after;
			conts[i]->GetValue(t,&after,valid,CTRL_RELATIVE);
			pblock->GetValue(kListCtrlWeight, t, weight, FOREVER, i);
			after = prevVal + (after - prevVal)*AverageWeight(weight);
		//	conts[i]->GetValue(t,&after,valid,CTRL_RELATIVE);
			}
		v = -before + v + -after;
		assert(active>=0);
		conts[active]->SetValue(t,&v,commit,method);
	} else {
		assert(active>=0);
		pblock->GetValue(kListCtrlWeight, t, weight, FOREVER, active);
		*((float*)val) *= weight;
		conts[active]->SetValue(t,val,commit,method);
		}
	}

ListControl *FloatListControl::DerivedClone()
	{
	return new FloatListControl;
	}

class FloatListClassDesc : public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	const TCHAR *	ClassName() {return FLOATLIST_CNAME;}
    SClass_ID		SuperClassID() {return CTRL_FLOAT_CLASS_ID;}
	Class_ID		ClassID() {return Class_ID(FLOATLIST_CONTROL_CLASS_ID,0);}
	const TCHAR* 	Category() {return _T("");}
	void *	Create(BOOL loading) { 
		return new FloatListControl(loading);
		}

	const TCHAR*	InternalName() { return _T("FloatList"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }					// returns owning module handle
	};
static FloatListClassDesc floatListCD;
ClassDesc* GetFloatListDesc() {return &floatListCD;}

void FloatListControl::Init()
	{
	// make the paramblock
	floatListCD.MakeAutoParamBlocks(this);
	}

// CAL-10/7/2002: add a PBAccessor for List Controller to return a local name for the weight tracks
// list controller PBAccessor
class ListCtrlPBAccessor : public PBAccessor
{ 
	public:
		TSTR GetLocalName(ReferenceMaker* owner, ParamID id, int tabIndex) {
			ListControl *ctrl = (ListControl*) owner;
			TSTR name;
			if (id == kListCtrlWeight)
				// CAL-11/12/2002: Check if the ctrl(owner) is NULL. This is possible for scenes created
				//		before A019, because of the problem reported in defect #463222. (467256)
				if (ctrl)
					name.printf("%s: %s", GetString(IDS_AF_LIST_WEIGHT), ctrl->SubAnimName(tabIndex));
				else
					name.printf("%s", GetString(IDS_AF_LIST_WEIGHT));
			return name;
		}
};

static ListCtrlPBAccessor theListCtrlPBAccessor;

// per instance list controller block
static ParamBlockDesc2 list_paramblk (kListCtrlWeightParams, _T("Parameters"),  0, &floatListCD, P_AUTO_CONSTRUCT, PBLOCK_INIT_REF_ID, 
	// params
	kListCtrlWeight, _T("weight"), TYPE_FLOAT_TAB, 0, P_ANIMATABLE+P_VARIABLE_SIZE+P_TV_SHOW_ALL+P_COMPUTED_NAME,  IDS_AF_LIST_WEIGHT, 
		p_default, 		1.0, 
		p_range, 		-10000.0, 10000.0, 
		p_dim,			stdPercentDim,
		p_accessor,		&theListCtrlPBAccessor,
		end,
		
	kListCtrlAverage, _T("average"), TYPE_BOOL, 0, IDS_AF_LIST_AVERAGE, 
		p_default, 		FALSE, 
		end, 
	end
	);

static FPInterfaceDesc listControlInterface ( LIST_CONTROLLER_INTERFACE, _T("list"), 0, &floatListCD, FP_MIXIN,
		IListControl::list_getNumItems,		_T("getCount"),		0, TYPE_INT,	0,	0,
		IListControl::list_setActive,		_T("setActive"),	0, TYPE_VOID,	0,	1,
			_T("listIndex"), 0, TYPE_INDEX,
		IListControl::list_getActive,		_T("getActive"),	0, TYPE_INT,	0,	0,
		IListControl::list_deleteItem,		_T("delete"),		0, TYPE_VOID,	0,	1,
			_T("listIndex"), 0, TYPE_INDEX,
		IListControl::list_cutItem,			_T("cut"),			0, TYPE_VOID,	0,	1,
			_T("listIndex"), 0, TYPE_INDEX,
		IListControl::list_pasteItem,		_T("paste"),		0, TYPE_VOID,	0,	1,
			_T("listIndex"), 0, TYPE_INDEX,
		IListControl::list_getName,			_T("getName"),		0, TYPE_TSTR_BV, 0,  1,
			_T("listIndex"), 0, TYPE_INDEX,
		IListControl::list_setName,			_T("setName"),		0, TYPE_VOID,   0,  2,
			_T("listIndex"), 0, TYPE_INDEX,
			_T("name"),		 0, TYPE_STRING,
		properties,
		IListControl::list_count, FP_NO_FUNCTION, _T("count"), 0, TYPE_INT,
		IListControl::list_getActive_prop, IListControl::list_setActive_prop, _T("active"), 0, TYPE_INDEX,

		end
	);


FPInterfaceDesc* IListControl::GetDesc()
{
	return &listControlInterface;
	//return NULL;
}

//-------------------------------------------------------------------------
// Point3 list control
//		
		
class Point3ListControl : public ListControl {
	public:
		Point3ListControl(BOOL loading) : ListControl(loading) {Init();} 
		Point3ListControl() {Init();}

		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void SetValue(TimeValue t, void *val, int commit, GetSetMethod method);
		ListControl *DerivedClone();
		void Init();

		Class_ID ClassID() { return Class_ID(POINT3LIST_CONTROL_CLASS_ID,0); }  
		SClass_ID SuperClassID() { return CTRL_POINT3_CLASS_ID; }  
		void GetClassName(TSTR& s) {s = POINT3LIST_CNAME;}
	};

void Point3ListControl::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
	Point3 *v = (Point3*)val;
	float weight;

	if (method==CTRL_ABSOLUTE) {
		*v = Point3(0,0,0);
		}
	Point3 prevVal;
	for (int i=0; i<conts.Count(); i++) {
		pblock->GetValue(kListCtrlWeight, t, weight, valid, i);
		if (weight != 0.0f) {
			prevVal = *v;
			if (conts[i]) conts[i]->GetValue(t,v,valid,CTRL_RELATIVE);
			*v = prevVal + ((*v) - prevVal)*AverageWeight(weight);
			}
		}
	}

void Point3ListControl::SetValue(
		TimeValue t, void *val, int commit, GetSetMethod method)
	{
	if (!conts.Count()) return;	
	float weight;

	if (method==CTRL_ABSOLUTE) {
		Point3 v = *((Point3*)val);
//		pblock->GetValue(kListCtrlWeight, t, weight, FOREVER, active);
//		v = v*weight;
		Point3 before(0,0,0), after(0,0,0);
		Interval valid;
		Point3 prevVal;
		for (int i=0; i<active; i++) {
			prevVal = before;
			if (conts[i]) conts[i]->GetValue(t,&before,valid,CTRL_RELATIVE);
			pblock->GetValue(kListCtrlWeight, t, weight, FOREVER, i);
			before = prevVal + (before - prevVal)*AverageWeight(weight);
			//conts[i]->GetValue(t,&before,valid,CTRL_RELATIVE);
			}
		for (i=active+1; i<conts.Count(); i++) {
			prevVal = after;
			if (conts[i]) conts[i]->GetValue(t,&after,valid,CTRL_RELATIVE);
			pblock->GetValue(kListCtrlWeight, t, weight, FOREVER, i);
			after = prevVal + (after - prevVal)*AverageWeight(weight);
			//conts[i]->GetValue(t,&after,valid,CTRL_RELATIVE);
			}
		v = -before + v + -after;
		assert(active>=0);
		conts[active]->SetValue(t,&v,commit,method);
	} else {
		assert(active>=0);
		pblock->GetValue(kListCtrlWeight, t, weight, FOREVER, active);
		*((Point3*)val) = *((Point3*)val) * AverageWeight(weight);
		conts[active]->SetValue(t,val,commit,method);
		}
	}

ListControl *Point3ListControl::DerivedClone()
	{
	return new Point3ListControl;
	}

//keeps track of whether an FP interface desc has been added to the ClassDesc
static bool p3ListInterfaceLoaded = false;

class Point3ListClassDesc : public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	const TCHAR *	ClassName() {return POINT3LIST_CNAME;}
    SClass_ID		SuperClassID() {return CTRL_POINT3_CLASS_ID;}
	Class_ID		ClassID() {return Class_ID(POINT3LIST_CONTROL_CLASS_ID,0);}
	const TCHAR* 	Category() {return _T("");}
	void *	Create(BOOL loading) { 	
		if (!p3ListInterfaceLoaded)
			{
			AddInterface(&listControlInterface);
			// AddParamBlockDesc(&list_paramblk);
			p3ListInterfaceLoaded = true;
			}
		return new Point3ListControl(loading);
		}
	
	const TCHAR*	InternalName() { return _T("Point3List"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }					// returns owning module handle
	};
static Point3ListClassDesc point3ListCD;
ClassDesc* GetPoint3ListDesc() {return &point3ListCD;}

void Point3ListControl::Init()
	{
	// make the paramblock
	point3ListCD.MakeAutoParamBlocks(this);
	}

// CAL-10/30/2002: Add individual ParamBlockDesc2 for each list controller (463222)
//		When the pblock is cloned in ListControl::Clone(), the owner of the new pblock isn't set to the
//		new list controller after ctrl->ReplaceReference() is called.
//		Normally when ReplaceReference() is called with pblock, the owner of the pblock will be set
//		by ParamBlock2::RefAdded() if the rmaker's ClassID and the ParamBlockDesc2->ClassDescriptor's
//		ClassID are the same. However, all list controllers use one ParamBlockDesc2, list_paramblk,
//		which has a class descriptor, floatListCD. Therefore, the owner of the new pblock will remain
//		NULL if 'ctrl' is not a FloatListControl. In max, it is assumed that a ParamBlockDesc2 belongs
//		to a single ClassDesc. The list controllers share one ParamBlockDesc2 in many Classes and
//		unfortunately max's architecture isn't quite ready for that yet.
//		Fix it by adding individual ParamBlockDesc2 for each list controller and remove the call of
//		AddParamBlockDesc() from the ClassDesc2::Create() method of these classes.

// per instance list controller block
static ParamBlockDesc2 point3_list_paramblk (kListCtrlWeightParams, _T("Parameters"),  0, &point3ListCD, P_AUTO_CONSTRUCT, PBLOCK_INIT_REF_ID, 
	// params
	kListCtrlWeight, _T("weight"), TYPE_FLOAT_TAB, 0, P_ANIMATABLE+P_VARIABLE_SIZE+P_TV_SHOW_ALL+P_COMPUTED_NAME,  IDS_AF_LIST_WEIGHT, 
		p_default, 		1.0, 
		p_range, 		-10000.0, 10000.0, 
		p_dim,			stdPercentDim,
		p_accessor,		&theListCtrlPBAccessor,
		end,
		
	kListCtrlAverage, _T("average"), TYPE_BOOL, 0, IDS_AF_LIST_AVERAGE, 
		p_default, 		FALSE, 
		end, 
	end
	);

//-------------------------------------------------------------------------
// Point4 list control
//		
		
class Point4ListControl : public ListControl {
	public:
		Point4ListControl(BOOL loading) : ListControl(loading) {Init();} 
		Point4ListControl() {Init();}

		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void SetValue(TimeValue t, void *val, int commit, GetSetMethod method);
		ListControl *DerivedClone();
		void Init();

		Class_ID ClassID() { return Class_ID(POINT4LIST_CONTROL_CLASS_ID,0); }  
		SClass_ID SuperClassID() { return CTRL_POINT4_CLASS_ID; }  
		void GetClassName(TSTR& s) {s = POINT4LIST_CNAME;}
	};

void Point4ListControl::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
	Point4 *v = (Point4*)val;
	float weight;

	if (method==CTRL_ABSOLUTE) {
		*v = Point4(0,0,0,0);
		}
	Point4 prevVal;
	for (int i=0; i<conts.Count(); i++) {
		pblock->GetValue(kListCtrlWeight, t, weight, valid, i);
		if (weight != 0.0f) {
			prevVal = *v;
			if (conts[i]) conts[i]->GetValue(t,v,valid,CTRL_RELATIVE);
			*v = prevVal + ((*v) - prevVal)*AverageWeight(weight);
			}
		}
	}

void Point4ListControl::SetValue(
		TimeValue t, void *val, int commit, GetSetMethod method)
	{
	if (!conts.Count()) return;	
	float weight;

	if (method==CTRL_ABSOLUTE) {
		Point4 v = *((Point4*)val);
//		pblock->GetValue(kListCtrlWeight, t, weight, FOREVER, active);
//		v = v*weight;
		Point4 before(0,0,0,0), after(0,0,0,0);
		Interval valid;
		Point4 prevVal;
		for (int i=0; i<active; i++) {
			prevVal = before;
			if (conts[i]) conts[i]->GetValue(t,&before,valid,CTRL_RELATIVE);
			pblock->GetValue(kListCtrlWeight, t, weight, FOREVER, i);
			before = prevVal + (before - prevVal)*AverageWeight(weight);
			//conts[i]->GetValue(t,&before,valid,CTRL_RELATIVE);
			}
		for (i=active+1; i<conts.Count(); i++) {
			prevVal = after;
			if (conts[i]) conts[i]->GetValue(t,&after,valid,CTRL_RELATIVE);
			pblock->GetValue(kListCtrlWeight, t, weight, FOREVER, i);
			after = prevVal + (after - prevVal)*AverageWeight(weight);
			//conts[i]->GetValue(t,&after,valid,CTRL_RELATIVE);
			}
		v = -before + v + -after;
		assert(active>=0);
		conts[active]->SetValue(t,&v,commit,method);
	} else {
		assert(active>=0);
		pblock->GetValue(kListCtrlWeight, t, weight, FOREVER, active);
		*((Point4*)val) = *((Point4*)val) * AverageWeight(weight);
		conts[active]->SetValue(t,val,commit,method);
		}
	}

ListControl *Point4ListControl::DerivedClone()
	{
	return new Point4ListControl;
	}

//keeps track of whether an FP interface desc has been added to the ClassDesc
static bool p4ListInterfaceLoaded = false;

class Point4ListClassDesc : public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	const TCHAR *	ClassName() {return POINT4LIST_CNAME;}
    SClass_ID		SuperClassID() {return CTRL_POINT4_CLASS_ID;}
	Class_ID		ClassID() {return Class_ID(POINT4LIST_CONTROL_CLASS_ID,0);}
	const TCHAR* 	Category() {return _T("");}
	void *	Create(BOOL loading) { 	
		if (!p4ListInterfaceLoaded)
			{
			AddInterface(&listControlInterface);
			// AddParamBlockDesc(&list_paramblk);
			p4ListInterfaceLoaded = true;
			}
		return new Point4ListControl(loading);
		}
	
	const TCHAR*	InternalName() { return _T("Point4List"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }					// returns owning module handle
	};
static Point4ListClassDesc point4ListCD;
ClassDesc* GetPoint4ListDesc() {return &point4ListCD;}

void Point4ListControl::Init()
	{
	// make the paramblock
	point4ListCD.MakeAutoParamBlocks(this);
	}

// per instance list controller block
static ParamBlockDesc2 point4_list_paramblk (kListCtrlWeightParams, _T("Parameters"),  0, &point4ListCD, P_AUTO_CONSTRUCT, PBLOCK_INIT_REF_ID, 
	// params
	kListCtrlWeight, _T("weight"), TYPE_FLOAT_TAB, 0, P_ANIMATABLE+P_VARIABLE_SIZE+P_TV_SHOW_ALL+P_COMPUTED_NAME,  IDS_AF_LIST_WEIGHT, 
		p_default, 		1.0, 
		p_range, 		-10000.0, 10000.0, 
		p_dim,			stdPercentDim,
		p_accessor,		&theListCtrlPBAccessor,
		end,
		
	kListCtrlAverage, _T("average"), TYPE_BOOL, 0, IDS_AF_LIST_AVERAGE, 
		p_default, 		FALSE, 
		end, 
	end
	);


//-------------------------------------------------------------------------
// Position list control
//		
		
class PositionListControl : public ListControl {
	public:
		PositionListControl(BOOL loading) : ListControl(loading) {Init();} 
		PositionListControl() {Init();}

		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void SetValue(TimeValue t, void *val, int commit, GetSetMethod method);
		ListControl *DerivedClone();
		void Init();

		Class_ID ClassID() { return Class_ID(POSLIST_CONTROL_CLASS_ID,0); }  
		SClass_ID SuperClassID() { return CTRL_POSITION_CLASS_ID; }  
		void GetClassName(TSTR& s) {s = POSLIST_CNAME;}
	};


// #define ANDY_ADDATIVE_LIST			// Use additive approach to sum the values in the list

#ifdef ANDY_ADDATIVE_LIST

#if 1
// CAL-08/16/02: Relative to identity matrix.
//		Good:	SetValue() can be done without the parent's transformation
//		Bad:	constraints will be affected by the parent's transformation if it's not identity matrix.
void PositionListControl::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
{
	Matrix3 tm(1);
	float weight;

	if (method == CTRL_RELATIVE)
		tm = *(Matrix3*)val;

	for (int i=0; i<conts.Count(); i++) {
		if (conts[i]) {
			Matrix3 deltaTM(1);
			conts[i]->GetValue(t, &deltaTM, valid, CTRL_RELATIVE);
			pblock->GetValue(kListCtrlWeight, t, weight, valid, i);
			deltaTM.SetTrans(deltaTM.GetTrans() * AverageWeight(weight));
			tm = deltaTM * tm;
		}
	}

	if (method == CTRL_ABSOLUTE)
		*(Point3*)val = tm.GetTrans();
	else
		*(Matrix3*)val = tm;
}

#else

// CAL-08/16/02: Relative to the parent's transformation
//		Good:	constraints will not be affected by the parent's transformation.
//		Bad:	SetValue() need to know the parent's transformation to do it correctly.
void PositionListControl::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
{
	Matrix3 tm(1), inTM(1), invTM(1);
	float weight;

	if (method == CTRL_RELATIVE) {
		tm = *(Matrix3*)val;
		inTM = tm;
		invTM = Inverse(inTM);
	}

	for (int i=0; i<conts.Count(); i++) {
		if (conts[i]) {
			Matrix3 deltaTM = inTM;
			conts[i]->GetValue(t, &deltaTM, valid, CTRL_RELATIVE);
			deltaTM = deltaTM * invTM;
			pblock->GetValue(kListCtrlWeight, t, weight, valid, i);
			deltaTM.SetTrans(deltaTM.GetTrans() * AverageWeight(weight));
			tm = deltaTM * tm;
		}
	}

	if (method == CTRL_ABSOLUTE)
		*(Point3*)val = tm.GetTrans();
	else
		*(Matrix3*)val = tm;
}

#endif

#else

void PositionListControl::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
	float weight;

	if (method==CTRL_ABSOLUTE) {
		Point3 prevPos;
		Matrix3 tm(1);

		for (int i=0; i<conts.Count(); i++) {
			pblock->GetValue(kListCtrlWeight, t, weight, valid, i);
			if (weight != 0.0f) {
				prevPos = tm.GetTrans();
				if (conts[i]) conts[i]->GetValue(t,&tm,valid,CTRL_RELATIVE);
				tm.SetTrans(prevPos + (tm.GetTrans() - prevPos)*AverageWeight(weight));
				}
			}
		*(Point3*)val = tm.GetTrans();
	} else {
		Matrix3* tm = (Matrix3*)val;
		Point3 prevPos;
		for (int i=0; i<conts.Count(); i++) {
			if (conts[i]) {
				pblock->GetValue(kListCtrlWeight, t, weight, valid, i);
				if (weight != 0.0f) {
					prevPos = tm->GetTrans();
					conts[i]->GetValue(t,tm,valid,CTRL_RELATIVE);
					tm->SetTrans(prevPos + (tm->GetTrans() - prevPos)*AverageWeight(weight));
					}
				}
			}
		}
	}

#endif

#ifdef ANDY_ADDATIVE_LIST

void PositionListControl::SetValue(
		TimeValue t, void *val, int commit, GetSetMethod method)
{
	if (!conts.Count()) return;
	assert(active>=0);

	float weight;
	Point3 v = *((Point3*)val);

	Matrix3 before(1), after(1);
	Interval valid;

	for (int i=0; i<conts.Count(); i++) {
		if (i != active && conts[i]) {
			Matrix3 deltaTM(1);
			conts[i]->GetValue(t, &deltaTM, valid, CTRL_RELATIVE);
			pblock->GetValue(kListCtrlWeight, t, weight, valid, i);
			deltaTM.SetTrans(deltaTM.GetTrans() * AverageWeight(weight));
			if (i < active)
				before = deltaTM * before;
			else
				after = deltaTM * after;
		}
	}

	if (method==CTRL_ABSOLUTE) {		
		// CAL-06/17/02: It should also consider the transformation of the active controller 
		//	Let assume that Tg is the input translation, Mg is the total transformation of all
		//	controllers in the list, M1 is the ‘before’ matrix, M2 is the active matrix to be set,
		//	and M3 is the ‘after’ matrix.
		//	Each transformation matrix can be represented by its 3x3 rotation matrix R and 3x1
		//	translation vector T, ie M = (R)(T).
		//	So Mg = (Rg)(Tg), M1 = (R1)(T1), M2 = (R2)(T2), M3 = (R3)(T3).
		//	So, Mg = M3 * M2 * M1 (also we know that Rg = R3 * R2 * R1)
		//	M2 = Inverse(M3) * Mg * Inverse(M1)
		//	   = (Inverse(R3))(-T3*Inverse(R3)) * (R3*R2*R1)(Tg) * (Inverse(R1))(-T1*Inverse(R1))
		//	   = (Inverse(R3))(-T3*Inverse(R3)) * (R3*R2)((Tg-T1)*Inverse(R1))
		//	   = (R2)((Tg-T1)*Inverse(R1) - T3*R2)  -> This is equal to (R2)(T2)
		//	So, T2 = (Tg - T1) * Inverse(R1) - T3 * R2

		Matrix3 mat(1);
		conts[active]->GetValue(t,&mat,valid,CTRL_RELATIVE);
		v = v * Inverse(before) - VectorTransform(mat,after.GetTrans());
	} else {
		// CAL-06/17/02: relative translation vector need only to be inverse transformed by
		//		the before matrix.
		//	Assume that Tv is the input relative translation, and Td is the relative translation
		//	that should be applied to M2.
		//	So, Mg * (I)(Tv) = M3 * M2 * (I)(Td) * M1
		//	(I)(Td) = Inverse(M2) * Inverse(M3) * Mg * (I)(Tv) * Inverse(M1)
		//		    = Inverse(M2) * Inverse(M3) * M3 * M2 * M1 * (I)(Tv) * Inverse(M1)
		//		    = M1 * (I)(Tv) * Inverse(M1)
		//		    = (R1)(T1) * (I)(Tv) * (Inverse(R1))(-T1*Inverse(R1))
		//		    = (R1)(T1+Tv) * (Inverse(R1))(-T1*Inverse(R1))
		//		    = (I)((T1+Tv)*Inverse(R1) - T1*Inverse(R1))
		//		    = (I)(Tv * Inverse(R1))
		//	So, Td = Tv * Inverse(R1)  -> This is equal to VectorTransform(Inverse(M1), Tv)

		v = VectorTransform(Inverse(before),v);
	}

	// the value needs to be adjusted by the weight of the active controller too.
	pblock->GetValue(kListCtrlWeight, t, weight, FOREVER, active);
	v = v / AverageWeight(weight);
	
	conts[active]->SetValue(t,&v,commit,method);
}

#else

void PositionListControl::SetValue(
		TimeValue t, void *val, int commit, GetSetMethod method)
	{
	if (!conts.Count()) return;	
	float weight;
	Point3 v = *((Point3*)val);
//	pblock->GetValue(kListCtrlWeight, t, weight, FOREVER, active);
//	v = v*weight;

	Matrix3 before(1), after(1);
	Interval valid;
	Point3 prevPos;

	for (int i=0; i<active; i++) {
		prevPos = before.GetTrans();
		if (conts[i]) conts[i]->GetValue(t,&before,valid,CTRL_RELATIVE);
		pblock->GetValue(kListCtrlWeight, t, weight, FOREVER, i);
		before.SetTrans(prevPos + (before.GetTrans() - prevPos)*AverageWeight(weight));
		//conts[i]->GetValue(t,&before,valid,CTRL_RELATIVE);
		}
	for (i=active+1; i<conts.Count(); i++) {
		prevPos = after.GetTrans();
		if (conts[i]) conts[i]->GetValue(t,&after,valid,CTRL_RELATIVE);
		pblock->GetValue(kListCtrlWeight, t, weight, FOREVER, i);
		after.SetTrans(prevPos + (after.GetTrans() - prevPos)*AverageWeight(weight));
		//conts[i]->GetValue(t,&after,valid,CTRL_RELATIVE);
		}

	if (method==CTRL_ABSOLUTE) {
		//v = -before.GetTrans() + v + -after.GetTrans();
		// RB 11/28/2000:
		// This was incorrect. It should be:
		// Inverse(afterControllers) * val * Inverse(beforeControllers)

		//v = Inverse(before) * v * Inverse(after);
		v = Inverse(after) * v * Inverse(before);

		assert(active>=0);
		conts[active]->SetValue(t,&v,commit,method);
	} else {		
		assert(active>=0);

		v = VectorTransform(Inverse(before),v);
		v = VectorTransform(Inverse(after),v);

		conts[active]->SetValue(t,&v,commit,method);
		}
	}

#endif

ListControl *PositionListControl::DerivedClone()
	{
	return new PositionListControl;
	}

//keeps track of whether an FP interface desc has been added to the ClassDesc
static bool posListInterfaceLoaded = false;

class PositionListClassDesc : public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	const TCHAR *	ClassName() {return POSLIST_CNAME;}
    SClass_ID		SuperClassID() {return CTRL_POSITION_CLASS_ID;}
	Class_ID		ClassID() {return Class_ID(POSLIST_CONTROL_CLASS_ID,0);}
	const TCHAR* 	Category() {return _T("");}
	void *	Create(BOOL loading) { 		
		if (!posListInterfaceLoaded)
			{
			AddInterface(&listControlInterface);
			// AddParamBlockDesc(&list_paramblk);
			posListInterfaceLoaded = true;
			}
		return new PositionListControl(loading);
		}
	
	const TCHAR*	InternalName() { return _T("PositionList"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }					// returns owning module handle
	};
static PositionListClassDesc posListCD;
ClassDesc* GetPositionListDesc() {return &posListCD;}

void PositionListControl::Init()
	{
	// make the paramblock
	posListCD.MakeAutoParamBlocks(this);
	}

// CAL-10/30/2002: Add individual ParamBlockDesc2 for each list controller - see point3_list_paramblk.
// per instance list controller block
static ParamBlockDesc2 pos_list_paramblk (kListCtrlWeightParams, _T("Parameters"),  0, &posListCD, P_AUTO_CONSTRUCT, PBLOCK_INIT_REF_ID, 
	// params
	kListCtrlWeight, _T("weight"), TYPE_FLOAT_TAB, 0, P_ANIMATABLE+P_VARIABLE_SIZE+P_TV_SHOW_ALL+P_COMPUTED_NAME,  IDS_AF_LIST_WEIGHT, 
		p_default, 		1.0, 
		p_range, 		-10000.0, 10000.0, 
		p_dim,			stdPercentDim,
		p_accessor,		&theListCtrlPBAccessor,
		end,
		
	kListCtrlAverage, _T("average"), TYPE_BOOL, 0, IDS_AF_LIST_AVERAGE, 
		p_default, 		FALSE, 
		end, 
	end
	);

//-------------------------------------------------------------------------
// Rotation list control
//		
		
class RotationListControl : public ListControl {
	public:
		RotationListControl(BOOL loading) : ListControl(loading) {Init();} 
		RotationListControl() {Init();}

		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void SetValue(TimeValue t, void *val, int commit, GetSetMethod method);
		ListControl *DerivedClone();
		void Init();

		Class_ID ClassID() { return Class_ID(ROTLIST_CONTROL_CLASS_ID,0); }  
		SClass_ID SuperClassID() { return CTRL_ROTATION_CLASS_ID; }  
		void GetClassName(TSTR& s) {s = ROTLIST_CNAME;}
	};

/*

void RotationListControl::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
	float weight;
	BOOL average = FALSE;
	pblock->GetValue(kListCtrlAverage, 0, average, FOREVER);

	if (!average) // Addative method
		{	
		Quat localRot(IdentQuat());
		Quat lastRot(IdentQuat());
		Matrix3 tm;
		if (method==CTRL_ABSOLUTE) {
			tm.IdentityMatrix();
			}
		else {
			tm = *((Matrix3*)val);
			}

		for (int i=0; i<conts.Count(); i++) {
			conts[i]->GetValue(t,&localRot,valid,CTRL_ABSOLUTE);
			pblock->GetValue(kListCtrlWeight, t, weight, valid, i);
			Quat weightedRot = Slerp(lastRot, localRot, min(max(AverageWeight(weight), 0.0f), 1.0f));
			lastRot = weightedRot;
			PreRotateMatrix(tm, weightedRot);
			}
		if (method==CTRL_ABSOLUTE) {
			*((Quat*)val) = Quat(tm);
			} 
		else {
			*((Matrix3*)val) = tm;
			}
		}
	else  // pose to pose blending
		{
		Quat localRot(IdentQuat());
		Quat lastRot(IdentQuat());
		Matrix3 tm;
		if (method==CTRL_ABSOLUTE) {
			tm.IdentityMatrix();
		} else {
			tm = *((Matrix3*)val);
			}
		Matrix3 initTM = tm;

		for (int i=0; i<conts.Count(); i++) {
			conts[i]->GetValue(t,&localRot,valid,CTRL_ABSOLUTE);
			pblock->GetValue(kListCtrlWeight, t, weight, valid, i);
			Quat weightedRot = Slerp(lastRot, localRot, min(max(weight, 0.0f), 1.0f));
			lastRot = weightedRot;
			weightedRot = weightedRot * Quat(initTM);
			Point3 trans = tm.GetTrans();
			tm.SetRotate(weightedRot);
			tm.SetTrans(trans);
			}

		if (method==CTRL_ABSOLUTE) {
			*((Quat*)val) = Quat(tm);
			}
		else {
			*((Matrix3*)val) = tm;
			}
		}
	}

*/

void RotationListControl::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
	float weight;
	BOOL average = FALSE;
	pblock->GetValue(kListCtrlAverage, 0, average, FOREVER);

	Matrix3 tm, localTM;
	if (method==CTRL_ABSOLUTE) {
		tm.IdentityMatrix();
		}
	else {
		tm = *((Matrix3*)val);
		}
		
	if (!average) { // Addative method
		for (int i=0; i<conts.Count(); i++) {
			pblock->GetValue(kListCtrlWeight, t, weight, valid, i);
			if (weight != 0.0f) {
				localTM = tm;
				conts[i]->GetValue(t,&localTM,valid,CTRL_RELATIVE);
				localTM = localTM * Inverse(tm);

				Quat weightedRot = Quat(localTM);
				weightedRot.Normalize();
				weightedRot.MakeClosest(IdentQuat());	// CAL-10/15/2002: find the smallest rotation
				weightedRot = Slerp(IdentQuat(), weightedRot, min(max(weight, 0.0f), 1.0f));
				weightedRot.Normalize();
				PreRotateMatrix(tm, weightedRot);
				}
			}
		}
	else { // pose to pose blending
		Matrix3 initTM = tm;
		Quat lastRot = IdentQuat();
		for (int i=0; i<conts.Count(); i++) {
			pblock->GetValue(kListCtrlWeight, t, weight, valid, i);
			if (weight != 0.0f) {
				localTM = tm;
				conts[i]->GetValue(t,&localTM,valid,CTRL_RELATIVE);
				localTM = localTM * Inverse(tm);

				Quat weightedRot = Quat(localTM);
				weightedRot.Normalize();
				weightedRot.MakeClosest(lastRot);
				weightedRot = Slerp(lastRot, weightedRot, min(max(weight, 0.0f), 1.0f));
				weightedRot.Normalize();

				lastRot = weightedRot;
				weightedRot = weightedRot * Quat(initTM);

				Point3 trans = tm.GetTrans();
				tm.SetRotate(weightedRot);
				tm.SetTrans(trans);
				}
			}
		}
	if (method==CTRL_ABSOLUTE) {
		*((Quat*)val) = Quat(tm);
		}
	else {
		*((Matrix3*)val) = tm;
		}
	}


static bool IsGimbalAxis(const Point3& a)
{
	int i = a.MaxComponent();
	if (a[i] != 1.0f) return false;
	int j = (i + 1) % 3, k = (i + 2) % 3;
	if (a[j] != 0.0f ||
		a[k] != FLT_MIN)
		return false;
	return true;
}
void RotationListControl::SetValue(
		TimeValue t, void *val, int commit, GetSetMethod method)
	{
	if (!conts.Count()) return;	
	AngAxis *aa = (AngAxis*)val;

	bool gimbal_axis = false;
	if (GetCOREInterface()->GetRefCoordSys() == COORDS_GIMBAL
		&& method == CTRL_RELATIVE
		&& IsGimbalAxis(aa->axis))
		gimbal_axis = true;

	Interval valid;
	Matrix3 before(1);
	Matrix3 localTM;
	float weight;
	BOOL average = FALSE;
	pblock->GetValue(kListCtrlAverage, 0, average, FOREVER);

	if (!average) { // Addative method
		if (!gimbal_axis)
			for (int i=0; i<active; i++) {
				localTM = before;
				conts[i]->GetValue(t,&localTM,valid,CTRL_RELATIVE);
				localTM = localTM * Inverse(before);

				pblock->GetValue(kListCtrlWeight, t, weight, valid, i);
				Quat weightedRot = Slerp(IdentQuat(), Quat(localTM), min(max(weight, 0.0f), 1.0f));
				PreRotateMatrix(before, weightedRot);
				}

		if (method==CTRL_ABSOLUTE) {
			Quat v = *((Quat*)val);
			Matrix3 after(1);
			for (int i=active+1; i<conts.Count(); i++) {
				localTM = after;
				conts[i]->GetValue(t,&localTM,valid,CTRL_RELATIVE);
				localTM = localTM * Inverse(after);

				pblock->GetValue(kListCtrlWeight, t, weight, valid, i);
				Quat weightedRot = Slerp(IdentQuat(), Quat(localTM), min(max(weight, 0.0f), 1.0f));
				PreRotateMatrix(after, weightedRot);
				}

			v = Inverse(Quat(after)) * v * Inverse(Quat(before));		
			conts[active]->SetValue(t,&v,commit,method);		
		} else {
			AngAxis na = *aa;
			if (!(conts[active]->ClassID()== Class_ID(LOOKAT_CONSTRAINT_CLASS_ID,0))
				&& !gimbal_axis)
				na.axis = VectorTransform(Inverse(before),na.axis);
			conts[active]->SetValue(t,&na,commit,method);
			}
		}
	else {  // pose to pose blending
		Matrix3 initTM = before;
		Quat lastRot = IdentQuat();
		if (!gimbal_axis)
			for (int i=0; i<active; i++) {
				localTM = before;
				conts[i]->GetValue(t,&localTM,valid,CTRL_RELATIVE);
				localTM = localTM * Inverse(before);
				pblock->GetValue(kListCtrlWeight, t, weight, valid, i);
				Quat weightedRot = Slerp(lastRot, Quat(localTM), min(max(weight, 0.0f), 1.0f));
				lastRot = weightedRot;
				weightedRot = weightedRot * Quat(initTM);
				Point3 trans = before.GetTrans();
				before.SetRotate(weightedRot);
				before.SetTrans(trans);
				}

		if (method==CTRL_ABSOLUTE) {
			Quat v = *((Quat*)val);
			Matrix3 after(1);
			initTM = after;
			lastRot = IdentQuat();
			for (int i=active+1; i<conts.Count(); i++) {
				localTM = after;
				conts[i]->GetValue(t,&localTM,valid,CTRL_RELATIVE);
				localTM = localTM * Inverse(after);
				pblock->GetValue(kListCtrlWeight, t, weight, valid, i);
				Quat weightedRot = Slerp(lastRot, Quat(localTM), min(max(weight, 0.0f), 1.0f));
				lastRot = weightedRot;
				weightedRot = weightedRot * Quat(initTM);
				Point3 trans = after.GetTrans();
				after.SetRotate(weightedRot);
				after.SetTrans(trans);
				}
			
			v = Inverse(Quat(after)) * v * Inverse(Quat(before));		
			
			assert(active>=0);
			conts[active]->SetValue(t,&v,commit,method);		
		} else {
			AngAxis na = *aa;
			if (!(conts[active]->ClassID()== Class_ID(LOOKAT_CONSTRAINT_CLASS_ID,0))
				&& !gimbal_axis)
				na.axis = VectorTransform(Inverse(before),na.axis);
			assert(active>=0);

			conts[active]->SetValue(t,&na,commit,method);
			}

		}
	}
/*
void RotationListControl::SetValue(
		TimeValue t, void *val, int commit, GetSetMethod method)
	{
	if (!conts.Count()) return;	
	AngAxis *aa = (AngAxis*)val;

	bool gimbal_axis = false;
	if (GetCOREInterface()->GetRefCoordSys() == COORDS_GIMBAL
		&& method == CTRL_RELATIVE
		&& IsGimbalAxis(aa->axis))
		gimbal_axis = true;

	Matrix3 before(1);
	Interval valid;
	if (!gimbal_axis)
		for (int i=0; i<active; i++) {
			conts[i]->GetValue(t,&before,valid,CTRL_RELATIVE);
			}

	if (method==CTRL_ABSOLUTE) {
		Quat v = *((Quat*)val);
		Matrix3 after(1);		
		for (int i=active+1; i<conts.Count(); i++) {
			conts[i]->GetValue(t,&after,valid,CTRL_RELATIVE);
			}	
		// RB 11/28/2000:
		// This was incorrect. It should be:
		// Inverse(afterControllers) * val * Inverse(beforeControllers)

		//v = Inverse(Quat(before)) * v * Inverse(Quat(after));		
		v = Inverse(Quat(after)) * v * Inverse(Quat(before));		
		
		assert(active>=0);
		conts[active]->SetValue(t,&v,commit,method);		
	} else {
		AngAxis na = *aa;
		if (!(conts[active]->ClassID()== Class_ID(LOOKAT_CONSTRAINT_CLASS_ID,0))
			&& !gimbal_axis)
			na.axis = VectorTransform(Inverse(before),na.axis);
		assert(active>=0);
		conts[active]->SetValue(t,&na,commit,method);
		}
	}
*/
ListControl *RotationListControl::DerivedClone()
	{
	return new RotationListControl;
	}

//keeps track of whether an FP interface desc has been added to the ClassDesc
static bool rotListInterfaceLoaded = false;

class RotationListClassDesc : public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	const TCHAR *	ClassName() {return ROTLIST_CNAME;}
    SClass_ID		SuperClassID() {return CTRL_ROTATION_CLASS_ID;}
	Class_ID		ClassID() {return Class_ID(ROTLIST_CONTROL_CLASS_ID,0);}
	const TCHAR* 	Category() {return _T("");}
	void *	Create(BOOL loading) { 		
		if (!rotListInterfaceLoaded)
			{
			AddInterface(&listControlInterface);
			// AddParamBlockDesc(&list_paramblk);
			rotListInterfaceLoaded = true;
			}
		return new RotationListControl(loading);
		}
	
	const TCHAR*	InternalName() { return _T("RotationList"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }					// returns owning module handle
	};
static RotationListClassDesc rotListCD;
ClassDesc* GetRotationListDesc() {return &rotListCD;}

void RotationListControl::Init()
	{
	// make the paramblock
	rotListCD.MakeAutoParamBlocks(this);
	}

// CAL-10/30/2002: Add individual ParamBlockDesc2 for each list controller - see point3_list_paramblk.
// per instance list controller block
static ParamBlockDesc2 rot_list_paramblk (kListCtrlWeightParams, _T("Parameters"),  0, &rotListCD, P_AUTO_CONSTRUCT, PBLOCK_INIT_REF_ID, 
	// params
	kListCtrlWeight, _T("weight"), TYPE_FLOAT_TAB, 0, P_ANIMATABLE+P_VARIABLE_SIZE+P_TV_SHOW_ALL+P_COMPUTED_NAME,  IDS_AF_LIST_WEIGHT, 
		p_default, 		1.0, 
		p_range, 		-10000.0, 10000.0, 
		p_dim,			stdPercentDim,
		p_accessor,		&theListCtrlPBAccessor,
		end,
		
	kListCtrlAverage, _T("average"), TYPE_BOOL, 0, IDS_AF_LIST_AVERAGE, 
		p_default, 		FALSE, 
		end, 
	end
	);

//-------------------------------------------------------------------------
// Scale list control
//		
		
class ScaleListControl : public ListControl {
	public:
		ScaleListControl(BOOL loading) : ListControl(loading) {Init();} 
		ScaleListControl() {Init();}

		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void SetValue(TimeValue t, void *val, int commit, GetSetMethod method);
		ListControl *DerivedClone();
		void Init();

		Class_ID ClassID() { return Class_ID(SCALELIST_CONTROL_CLASS_ID,0); }  
		SClass_ID SuperClassID() { return CTRL_SCALE_CLASS_ID; }  
		void GetClassName(TSTR& s) {s = SCALELIST_CNAME;}
	};

void ScaleListControl::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
	float weight;
	AffineParts parts;

	if (method==CTRL_ABSOLUTE) {
		ScaleValue totalScale(Point3(1,1,1));
		ScaleValue tempScale(Point3(0,0,0));

		for (int i=0; i<conts.Count(); i++) {
			pblock->GetValue(kListCtrlWeight, t, weight, valid, i);
			if (weight != 0.0f) {
				// CAL-06/06/02: why do we need to zero the scale here? doesn't make sense to me??
				// if (i==0) totalScale = Point3(0,0,0);
				if (conts[i]) conts[i]->GetValue(t,&tempScale,valid,CTRL_ABSOLUTE);
				totalScale = totalScale + tempScale*AverageWeight(weight);
				}
			}
		(*(ScaleValue*)val) = totalScale;
	} else {
		Matrix3* tm = (Matrix3*)val;
		decomp_affine(*tm,&parts);
		Point3 tempVal = parts.k;
		for (int i=0; i<conts.Count(); i++) {
			pblock->GetValue(kListCtrlWeight, t, weight, valid, i);
			if (weight != 0.0f) {
				conts[i]->GetValue(t,val,valid,CTRL_RELATIVE);
				decomp_affine(*tm,&parts);
				tempVal = tempVal + (parts.k - tempVal)*AverageWeight(weight);
				// CAL-06/06/02: Use comp_affine to reconstruct the matrix better.
				//		NOTE: SetRotate() will erase the scale set by SetScale().
				parts.k = tempVal;
				comp_affine(parts, *tm);
				// Quat rot = Quat(*tm);
				// Point3 trans =tm->GetTrans();
				// tm->SetScale(tempVal);
				// tm->SetRotate(rot);
				// tm->SetTrans(trans);
				}
			}
		}
	}


void ScaleListControl::SetValue(
		TimeValue t, void *val, int commit, GetSetMethod method)
	{
	if (!conts.Count()) return;	

	if (method==CTRL_ABSOLUTE) {
		ScaleValue v = *((Point3*)val);
		Matrix3 before(1), after(1);
		Interval valid;
		for (int i=0; i<active; i++) {
			conts[i]->GetValue(t,&before,valid,CTRL_RELATIVE);
			}
		for (i=active+1; i<conts.Count(); i++) {
			conts[i]->GetValue(t,&after,valid,CTRL_RELATIVE);
			}
		
		AffineParts bparts, aparts;
		decomp_affine(Inverse(before),&bparts);
		decomp_affine(Inverse(after),&aparts);
		v.q = Inverse(bparts.u) * v.q * Inverse(aparts.u);
		// CAL-06/06/02: scale should be computed by scaling (not subtracting) out others scales.
		//		NOTE: probably need to check divide-by-zero exception.
		// v.s = -bparts.k + v.s + -aparts.k;
		DbgAssert((bparts.k * aparts.k) != Point3::Origin);
		v.s = v.s / (bparts.k * aparts.k);
		assert(active>=0);
		conts[active]->SetValue(t,&v,commit,method);
	} else {
		assert(active>=0);
		conts[active]->SetValue(t,val,commit,method);
		}
	}

ListControl *ScaleListControl::DerivedClone()
	{
	return new ScaleListControl;
	}

//keeps track of whether an FP interface desc has been added to the ClassDesc
static bool scaleListInterfaceLoaded = false;

class ScaleListClassDesc : public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	const TCHAR *	ClassName() {return SCALELIST_CNAME;}
    SClass_ID		SuperClassID() {return CTRL_SCALE_CLASS_ID;}
	Class_ID		ClassID() {return Class_ID(SCALELIST_CONTROL_CLASS_ID,0);}
	const TCHAR* 	Category() {return _T("");}
	void *	Create(BOOL loading) { 		
		if (!scaleListInterfaceLoaded)
			{
			AddInterface(&listControlInterface);
			// AddParamBlockDesc(&list_paramblk);
			scaleListInterfaceLoaded = true;
			}
		return new ScaleListControl(loading);
		}
	
	const TCHAR*	InternalName() { return _T("ScaleList"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }					// returns owning module handle
	};
static ScaleListClassDesc scaleListCD;
ClassDesc* GetScaleListDesc() {return &scaleListCD;}

void ScaleListControl::Init()
	{
	// make the paramblock
	scaleListCD.MakeAutoParamBlocks(this);
	}

// CAL-10/30/2002: Add individual ParamBlockDesc2 for each list controller - see point3_list_paramblk.
// per instance list controller block
static ParamBlockDesc2 scale_list_paramblk (kListCtrlWeightParams, _T("Parameters"),  0, &scaleListCD, P_AUTO_CONSTRUCT, PBLOCK_INIT_REF_ID, 
	// params
	kListCtrlWeight, _T("weight"), TYPE_FLOAT_TAB, 0, P_ANIMATABLE+P_VARIABLE_SIZE+P_TV_SHOW_ALL+P_COMPUTED_NAME,  IDS_AF_LIST_WEIGHT, 
		p_default, 		1.0, 
		p_range, 		-10000.0, 10000.0, 
		p_dim,			stdPercentDim,
		p_accessor,		&theListCtrlPBAccessor,
		end,
		
	kListCtrlAverage, _T("average"), TYPE_BOOL, 0, IDS_AF_LIST_AVERAGE, 
		p_default, 		FALSE, 
		end, 
	end
	);

//-------------------------------------------------------------------------
//Master Control List Watje

class MasterListControl : public ListControl {
	public:
		MasterListControl(BOOL loading) : ListControl(loading) {Init();} 
		MasterListControl() {Init();}

		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void SetValue(TimeValue t, void *val, int commit, GetSetMethod method);
		ListControl *DerivedClone();
		void Init();

		Class_ID ClassID() { return Class_ID(MASTERLIST_CONTROL_CLASS_ID,0); }  
		SClass_ID SuperClassID() { return MASTERBLOCK_SUPER_CLASS_ID; }  
		void GetClassName(TSTR& s) {s = MASTERLIST_CNAME;}
		BOOL IsReplaceable() {return FALSE;}

		BOOL CanCopyAnim() {return FALSE;}
		BOOL CanApplyEaseMultCurves() { return FALSE;}

		int PaintTrack(ParamDimensionBase *dim,HDC hdc,Rect& rcTrack,
			Rect& rcPaint,float zoom,int scroll,DWORD flags );

		int NumSubs()  {return conts.Count()+1;} //numControllers+dummyController (no pblock)
	};

void MasterListControl::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
	}

void MasterListControl::SetValue(
		TimeValue t, void *val, int commit, GetSetMethod method)
	{
	}

ListControl *MasterListControl::DerivedClone()
	{
	return new MasterListControl;
	}

int MasterListControl::PaintTrack(ParamDimensionBase *dim,HDC hdc,Rect& rcTrack,
			Rect& rcPaint,float zoom,int scroll,DWORD flags ) 
	{
	if (flags&PAINTTRACK_SUBTREEMODE)
		return TRACK_DONE; // don't paint subtrack keys for this controlller
	return TRACK_DORANGE; 
	}

//keeps track of whether an FP interface desc has been added to the ClassDesc
static bool masterListInterfaceLoaded = false;

class MasterListClassDesc : public ClassDesc2 {
	public:
	int 			IsPublic() {return 0;}
	const TCHAR *	ClassName() {return MASTERLIST_CNAME;}
    SClass_ID		SuperClassID() {return MASTERBLOCK_SUPER_CLASS_ID;}
	Class_ID		ClassID() {return Class_ID(MASTERLIST_CONTROL_CLASS_ID,0);}
	const TCHAR* 	Category() {return _T("");}
	void *	Create(BOOL loading) { 		
		if (!masterListInterfaceLoaded)
			{
			AddInterface(&listControlInterface);
			// AddParamBlockDesc(&list_paramblk);
			masterListInterfaceLoaded = true;
			}
		return new MasterListControl(loading);
		}
	
	const TCHAR*	InternalName() { return _T("MasterList"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }					// returns owning module handle
	};
static MasterListClassDesc masterListCD;
ClassDesc* GetMasterListDesc() {return &masterListCD;}

void MasterListControl::Init()
	{
	// make the paramblock
	masterListCD.MakeAutoParamBlocks(this);
	}

// CAL-10/30/2002: Add individual ParamBlockDesc2 for each list controller - see point3_list_paramblk.
// per instance list controller block
static ParamBlockDesc2 master_list_paramblk (kListCtrlWeightParams, _T("Parameters"),  0, &masterListCD, P_AUTO_CONSTRUCT, PBLOCK_INIT_REF_ID, 
	// params
	kListCtrlWeight, _T("weight"), TYPE_FLOAT_TAB, 0, P_ANIMATABLE+P_VARIABLE_SIZE+P_TV_SHOW_ALL+P_COMPUTED_NAME,  IDS_AF_LIST_WEIGHT, 
		p_default, 		1.0, 
		p_range, 		-10000.0, 10000.0, 
		p_dim,			stdPercentDim,
		p_accessor,		&theListCtrlPBAccessor,
		end,
		
	kListCtrlAverage, _T("average"), TYPE_BOOL, 0, IDS_AF_LIST_AVERAGE, 
		p_default, 		FALSE, 
		end, 
	end
	);

//-------------------------------------------------------------------------

