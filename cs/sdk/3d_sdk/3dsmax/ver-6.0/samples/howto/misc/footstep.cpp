/**********************************************************************
 *<
	FILE: footstep.cpp

	DESCRIPTION:  A sample controller that draws foot step apparatuses

	CREATED BY: Rolf Berteig

	HISTORY: created 5/10/94

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "core.h"
#include "object.h"
#include "splshape.h"
#include "resource.h"
#include "custcont.h"
#include "units.h"
#include "mouseman.h"
#include "control.h"
#include "objmode.h"
#include "cmdmode.h"
#include "Maxapi.h"
#include "decomp.h"
#include "coremain.h"

#define FOOTSAMPLE_CLASS_ID		0x3001
#define FOOTSAMPLE_CNAME		GetResString(IDS_RB_FOOTSTEPSAMPLE)

#define NUM_FOOTSTEPS	10

class FootStepControl : public Control {
		friend class FootStepContRest;
		friend INT_PTR CALLBACK FootStepParamDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
			// WIN64 Cleanup: Shuler
	private:
		BOOL showSteps;
		Matrix3 tm;
		Matrix3 fs[NUM_FOOTSTEPS];
		BitArray sel;

		static FootStepControl *editCtrl;
		static HWND hParams;
		static IObjParam *ip;
		static MoveCtrlApparatusCMode *moveMode;
		static RotateCtrlApparatusCMode *rotMode;
		static UScaleCtrlApparatusCMode *uscaleMode;
		static NUScaleCtrlApparatusCMode *nuscaleMode;
		static SquashCtrlApparatusCMode *squashMode;
		static SelectCtrlApparatusCMode *selectMode;

		void Move(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& v, BOOL localOrigin, int commit=1); 
		void Rotate( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& q, BOOL localOrigin, int commit=1);
		void Scale(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin, int commit=1);
		void SetAbsValue(TimeValue t, const Matrix3 &val,const Matrix3 &parent, int commit=1);

	public:		
		static HWND hFootParams;
		static IObjParam *iObjParams;

		FootStepControl(BOOL loading=FALSE);
		FootStepControl(const FootStepControl& ctrl);		
		FootStepControl& operator=(const FootStepControl& ctrl);
		void HoldTrack();

		// From Control
		void Copy(Control *from);
		void CommitValue(TimeValue t) {}
		void RestoreValue(TimeValue t) {}
		virtual BOOL IsLeaf() {return FALSE;}
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void SetValue(TimeValue t, void *val, int commit=1, GetSetMethod method=CTRL_ABSOLUTE);
		
		// From control -- for apparatus manipulation
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box);
		int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
		void ActivateSubobjSel(int level, XFormModes& modes );
		void SelectSubComponent(CtrlHitRecord *hitRec, BOOL selected, BOOL all);
		void ClearSelection(int selLevel);
		int SubObjectIndex(CtrlHitRecord *hitRec);
		
		//int NumSubObjects(TimeValue t,INode *node);
		//void GetSubObjectTM(TimeValue t,INode *node,int subIndex,Matrix3& tm);
		//Point3 GetSubObjectCenter(TimeValue t,INode *node,int subIndex,int type);
		void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node);
		void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node);

		void SubMove( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE );
		void SubRotate( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin=FALSE );
		void SubScale( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE );
		
		RefTargetHandle Clone(RemapDir& remap = NoRemap()) {
			FootStepControl *fsctrl = new FootStepControl(* this); 
			BaseClone(this, fsctrl, remap);
			return fsctrl; 
		}

		// From Animatable
		Class_ID ClassID() { return Class_ID(FOOTSAMPLE_CLASS_ID,0); }  
		SClass_ID SuperClassID() { return CTRL_MATRIX3_CLASS_ID; }  
		void GetClassName(TSTR& s) { s = FOOTSAMPLE_CNAME; }
		void DeleteThis() { delete this; }		
		int IsKeyable(){ return 0;}
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev); 
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next); 		
		RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage) {return REF_SUCCEED;}
	};

HWND                       FootStepControl::hParams     = NULL;
IObjParam*                 FootStepControl::ip          = NULL;
FootStepControl*           FootStepControl::editCtrl    = NULL;
MoveCtrlApparatusCMode*    FootStepControl::moveMode    = NULL;
RotateCtrlApparatusCMode*  FootStepControl::rotMode     = NULL;
UScaleCtrlApparatusCMode*  FootStepControl::uscaleMode  = NULL;
NUScaleCtrlApparatusCMode* FootStepControl::nuscaleMode = NULL;
SquashCtrlApparatusCMode*  FootStepControl::squashMode  = NULL;
SelectCtrlApparatusCMode*  FootStepControl::selectMode  = NULL;


//********************************************************
// FOOTSTEP CONTROL
//********************************************************
static Class_ID footControlClassID(FOOTSAMPLE_CLASS_ID,0); 
class FootStepClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new FootStepControl(loading); }
	const TCHAR *	ClassName() { return FOOTSAMPLE_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_MATRIX3_CLASS_ID; }
	Class_ID		ClassID() { return footControlClassID; }
	const TCHAR* 	Category() { return _T("");  }
	};
static FootStepClassDesc footStepCD;

ClassDesc *GetFootStepDescriptor()
	{
	return &footStepCD;
	}

class FootStepContRest: public RestoreObj {
	public:		
		FootStepControl *ctrl;
		Matrix3 undo,redo,undofs[NUM_FOOTSTEPS],redofs[NUM_FOOTSTEPS];
		BitArray undoSel,redoSel;
		FootStepContRest(FootStepControl *c) {
			ctrl = c;
			undo = c->tm;
			undoSel = c->sel;
			for (int i=0; i<NUM_FOOTSTEPS; i++) {
				undofs[i] = ctrl->fs[i];
				}
			}					
		void Restore(int isUndo) {			
			redo = ctrl->tm;			
			ctrl->tm = undo;
			redoSel = ctrl->sel;
			ctrl->sel = undoSel;
			for (int i=0; i<NUM_FOOTSTEPS; i++) {
				redofs[i]   = ctrl->fs[i];
				ctrl->fs[i] = undofs[i];
				}
			ctrl->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
		void Redo() {
			ctrl->tm = redo;
			ctrl->sel = redoSel;
			for (int i=0; i<NUM_FOOTSTEPS; i++) {
				ctrl->fs[i] = redofs[i];				
				}
			ctrl->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
		int Size() {return sizeof(FootStepContRest);}
		void EndHold() { ctrl->ClearAFlag(A_HELD); }		
	};


FootStepControl::FootStepControl(BOOL loading)
	{
	tm.IdentityMatrix();
	showSteps = TRUE;
	sel.SetSize(NUM_FOOTSTEPS);
	for (int i=0; i<NUM_FOOTSTEPS; i++) {
		fs[i].IdentityMatrix();
		Point3 t;
		t.x = i&1 ? -5.0f : 5.0f;
		t.y = float(i) * 15.0f;
		t.z = 0.0f;
		fs[i].SetTrans(t);
		}
	}

FootStepControl::FootStepControl(const FootStepControl& ctrl)
	{
	*this = ctrl;	
	}

FootStepControl& FootStepControl::operator=(const FootStepControl& ctrl)
	{
	tm = ctrl.tm;
	showSteps = ctrl.showSteps;
	sel = ctrl.sel;
	for (int i=0; i<NUM_FOOTSTEPS; i++) {
		fs[i] = ctrl.fs[i];
		}
	return (*this);
	}

void FootStepControl::Copy(Control *from)
	{
	if (from->ClassID()==ClassID()) {
	 	FootStepControl *fctrl = (FootStepControl*)from;
		*this = *fctrl;
	} else {
		Interval valid;
		tm.IdentityMatrix();
		from->GetValue(0,&tm,valid,CTRL_RELATIVE);
		}
	}

void FootStepControl::HoldTrack()
	{
	if (theHold.Holding()&&!TestAFlag(A_HELD)) {		
		theHold.Put(new FootStepContRest(this));
		SetAFlag(A_HELD);
		}
	}

void FootStepControl::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
	if (method == CTRL_ABSOLUTE) {
		*((Matrix3*)val) = tm;
	} else {
		*((Matrix3*)val) = tm * *((Matrix3*)val);
		}
	}

void FootStepControl::SetValue(
		TimeValue t, void *val, int commit, GetSetMethod method)
	{
	SetXFormPacket *ptr = (SetXFormPacket*)val;
	
	HoldTrack();

	switch (ptr->command) {
		case XFORM_MOVE:
			Move(t,ptr->tmParent,ptr->tmAxis,ptr->p,ptr->localOrigin,commit);
			break;
		case XFORM_ROTATE:
			Rotate(t,ptr->tmParent,ptr->tmAxis,ptr->q,ptr->localOrigin,commit);
			break;
		case XFORM_SCALE:
			Scale(t,ptr->tmParent,ptr->tmAxis,ptr->p,ptr->localOrigin,commit);
			break;
		
		case XFORM_SET:
			SetAbsValue(t,ptr->tmAxis,ptr->tmParent,commit);
			break;
		}
	}
		

void FootStepControl::Move(
		TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& v, 
		BOOL localOrigin, int commit)
	{
	Point3 p = VectorTransform(tmAxis*Inverse(partm),v);
	tm.Translate(p);	
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void FootStepControl::Rotate( 
		TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& q, 
		BOOL localOrigin, int commit)
	{
	Matrix3 mat;
	q.MakeMatrix(mat);
	mat = partm*Inverse(tmAxis)*mat*tmAxis*Inverse(partm);
	tm  = tm * mat;	
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void FootStepControl::Scale(
		TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, 
		BOOL localOrigin, int commit)
	{
	Matrix3 mat = ScaleMatrix(val);
	mat = partm*Inverse(tmAxis)*mat*tmAxis*Inverse(partm);
	tm  = tm * mat;	
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void FootStepControl::SetAbsValue(
		TimeValue t, const Matrix3 &val,const Matrix3 &parent, int commit)
	{
	tm = val*Inverse(parent);
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}


// A foot print
#define FOOT_POINTS		14
static Point3 footPts[15] = { // NOTE: I'm pretty sure an extra point is needed in the array for GFX.
		Point3(-2,0,0),Point3(-2,-1,0),Point3(-1,-2,0),
		Point3(1,-2,0),Point3(2,-1,0),Point3(2,0,0),
		Point3(1,1,0),Point3(2,4,0),Point3(2,6,0),
		Point3(1,7,0),Point3(-1,7,0),Point3(-2,6,0),
		Point3(-2,4,0),Point3(-1,1,0)};	


int FootStepControl::Display(
		TimeValue t, INode* inode, ViewExp *vpt, int flags)
	{
	if (showSteps || editCtrl==this) {
		GraphicsWindow *gw = vpt->getGW();
		Matrix3 ntm = inode->GetNodeTM(t);		
		for (int i=0; i<NUM_FOOTSTEPS; i++) {
			if (sel[i] && editCtrl==this) {
				gw->setColor(LINE_COLOR,1.0f,0.0f,0.0f);
			} else {
				gw->setColor(LINE_COLOR,1.0f,1.0f,0.0f);
				}
			gw->setTransform(fs[i]*ntm);
			gw->polyline(FOOT_POINTS,footPts,NULL,NULL,TRUE,NULL);
			}
		}
	return 0;
	}

void FootStepControl::GetWorldBoundBox(
		TimeValue t,INode* inode, ViewExp *vpt, Box3& box)
	{
	if (showSteps || editCtrl==this) {
		Matrix3 ntm = inode->GetNodeTM(t);
		box.Init();
		for (int i=0; i<NUM_FOOTSTEPS; i++) {
			for (int j=0; j<FOOT_POINTS; j++) {
				box += fs[i]*ntm * footPts[j];
				}
			}
		}
	}

int FootStepControl::HitTest(
		TimeValue t, INode* inode, int type, int crossing, int flags, 
		IPoint2 *p, ViewExp *vpt)
	{
	int savedLimits, res = 0;
	GraphicsWindow *gw = vpt->getGW();
	Matrix3 ntm = inode->GetNodeTM(t);
	HitRegion hr;
	MakeHitRegion(hr,type,crossing,4,p);
	gw->setHitRegion(&hr);
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
	gw->clearHitCode();
	
	BOOL abortOnHit = flags&SUBHIT_ABORTONHIT?TRUE:FALSE; 
	BOOL selOnly    = flags&SUBHIT_SELONLY?TRUE:FALSE; 
	BOOL unselOnly  = flags&SUBHIT_UNSELONLY?TRUE:FALSE; 

	for (int i=0; i<NUM_FOOTSTEPS; i++) {
		if (selOnly && !sel[i]) continue;
		if (unselOnly && sel[i]) continue;
		gw->setTransform(fs[i]*ntm);
		gw->polyline(FOOT_POINTS,footPts,NULL,NULL,TRUE,NULL);
		if (gw->checkHitCode()) {
			res = TRUE;
			vpt->CtrlLogHit(inode,gw->getHitDistance(),i,0);
			if (abortOnHit) {
				break;
				}
			gw->clearHitCode();
			}
		}

	gw->setRndLimits(savedLimits);
	return res;
	}

void FootStepControl::ActivateSubobjSel(int level, XFormModes& modes )
	{
	if (level) {
		modes = XFormModes(moveMode,rotMode,nuscaleMode,uscaleMode,squashMode,selectMode);
		}
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);	
	}

void FootStepControl::SelectSubComponent(
		CtrlHitRecord *hitRec, BOOL selected, BOOL all)
	{
	HoldTrack();
	while (hitRec) {
		if (selected) {
			sel.Set(hitRec->hitInfo);
		} else {
			sel.Clear(hitRec->hitInfo);
			}
		if (all) hitRec = hitRec->Next();
		else break;
		}
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void FootStepControl::ClearSelection(int selLevel)	
	{
	HoldTrack();
	sel.ClearAll();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}


int FootStepControl::SubObjectIndex(CtrlHitRecord *hitRec)
	{
	int count = 0;
	for (ulong i=0; i<hitRec->hitInfo; i++) {
		if (sel[i]) count++;
		}
	return count;
	}

void FootStepControl::GetSubObjectCenters(
		SubObjAxisCallback *cb,TimeValue t,INode *node)
	{
	Matrix3 mat(1);
	if (cb->Type()==SO_CENTER_SELECTION) {
		Point3 center(0,0,0);
		Matrix3 ntm = node->GetNodeTM(t);
		for (int i=0; i<NUM_FOOTSTEPS; i++) {
			mat = fs[i] * ntm;
			center += mat.GetTrans();
			}
		center /= float(NUM_FOOTSTEPS);
		cb->Center(center,0);
	} else {		
		for (int i=0; i<NUM_FOOTSTEPS; i++) {
			if (sel[i]) {
				mat = fs[i] * node->GetNodeTM(t);
				cb->Center(mat.GetTrans(),i);
				}			
			}		
		}
	}

void FootStepControl::GetSubObjectTMs(
		SubObjAxisCallback *cb,TimeValue t,INode *node)
	{
	Matrix3 mat;	
	for (int i=0; i<NUM_FOOTSTEPS; i++) {
		if (sel[i]) {
			mat = fs[i]*node->GetNodeTM(t);
			cb->TM(mat,i);			
			}		
		}
	}

/*
int FootStepControl::NumSubObjects(TimeValue t,INode *node)
	{
	int count = 0;
	for (int i=0; i<NUM_FOOTSTEPS; i++) {
		if (sel[i]) count++;
		}
	return count;
	}

void FootStepControl::GetSubObjectTM(TimeValue t,INode *node,int subIndex,Matrix3& tm)
	{	
	int count = 0;
	for (int i=0; i<NUM_FOOTSTEPS; i++) {
		if (count == subIndex) {
			tm = fs[i]*node->GetNodeTM(t);
			return;
			}
		if (sel[i]) count++;
		}
	tm.IdentityMatrix();
	}

Point3 FootStepControl::GetSubObjectCenter(TimeValue t,INode *node,int subIndex,int type)
	{
	Matrix3 tm(1);
	if (type==SO_CENTER_SELECTION) {
		Point3 center(0,0,0);
		Matrix3 ntm = node->GetNodeTM(t);
		for (int i=0; i<NUM_FOOTSTEPS; i++) {
			tm = fs[i] * ntm;
			center += tm[3];
			}
		center /= float(NUM_FOOTSTEPS);
		return center;
	} else {
		int count = 0;
		for (int i=0; i<NUM_FOOTSTEPS; i++) {
			if (count == subIndex) {
				tm = fs[i] * node->GetNodeTM(t);
				}
			if (sel[i]) count++;
			}
		return tm[3];
		}
	}
*/

void FootStepControl::SubMove( 
		TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, 
		BOOL localOrigin)
	{	
	Matrix3 mat(1);
	Point3 p = VectorTransform(tmAxis*Inverse(partm),val);
	mat.Translate(p);		
	HoldTrack();
	for (int i=0; i<NUM_FOOTSTEPS; i++) {
		if (sel[i]) fs[i] = fs[i] * mat;
		}
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void FootStepControl::SubRotate( 
		TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, 
		BOOL localOrigin)
	{
	Matrix3 mat;
	val.MakeMatrix(mat);
	mat = partm*Inverse(tmAxis)*mat*tmAxis*Inverse(partm);
	HoldTrack();
	for (int i=0; i<NUM_FOOTSTEPS; i++) {
		if (sel[i]) fs[i] = fs[i] * mat;
		}
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void FootStepControl::SubScale( 
		TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, 
		BOOL localOrigin)
	{
	Matrix3 mat = ScaleMatrix(val);
	mat = partm*Inverse(tmAxis)*mat*tmAxis*Inverse(partm);
	HoldTrack();
	for (int i=0; i<NUM_FOOTSTEPS; i++) {
		if (sel[i]) fs[i] = fs[i] * mat;
		}
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}


INT_PTR CALLBACK FootStepParamDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
	{
		// WIN64 Cleanup: Shuler
	static BOOL xEnabled=TRUE;
	static BOOL yEnabled=TRUE;
	static BOOL zEnabled=TRUE;
	static BOOL xyEnabled=TRUE;
	static BOOL centerEnabled=TRUE;
	static BOOL refEnabled=TRUE;

	FootStepControl *co = (FootStepControl*)GetWindowLongPtr(hDlg,GWLP_USERDATA);
		// WIN64 Cleanup: Shuler
	if ( !co && message != WM_INITDIALOG ) return FALSE;
	switch ( message ) {
		case WM_INITDIALOG:
			co = (FootStepControl*)lParam;
			SetWindowLongPtr(hDlg,GWLP_USERDATA,(LONG_PTR)co);	
				// WIN64 Cleanup: Shuler
			CheckDlgButton(hDlg,IDC_SHOWFOOTPRINTS,co->showSteps);
			return TRUE;

		case WM_LBUTTONDOWN:case WM_LBUTTONUP:	case WM_MOUSEMOVE:
			co->ip->RollupMouseMessage(hDlg,message,wParam,lParam);
			return FALSE;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {				
				case IDC_ACTIVE_X:
					co->ip->SetAxisConstraints(AXIS_X); break;
				case IDC_ACTIVE_Y:
					co->ip->SetAxisConstraints(AXIS_Y); break;
				case IDC_ACTIVE_Z:
					co->ip->SetAxisConstraints(AXIS_Z); break;
				case IDC_ACTIVE_XY:
					co->ip->SetAxisConstraints(AXIS_XY); break;
				case IDC_ACTIVE_YZ:
					co->ip->SetAxisConstraints(AXIS_YZ); break;
				case IDC_ACTIVE_ZX:
					co->ip->SetAxisConstraints(AXIS_ZX); break;

				case IDC_ACTIVE_SYS:
					co->ip->SetCoordCenter(ORIGIN_SYSTEM); break;
				case IDC_ACTIVE_SEL:
					co->ip->SetCoordCenter(ORIGIN_SELECTION); break;
				case IDC_ACTIVE_LOCAL:
					co->ip->SetCoordCenter(ORIGIN_LOCAL); break;

				case IDC_ACTIVE_VIEW:
					co->ip->SetRefCoordSys(COORDS_HYBRID); break;
				case IDC_ACTIVE_SCREEN:
					co->ip->SetRefCoordSys(COORDS_SCREEN); break;
				case IDC_ACTIVE_WORLD:
					co->ip->SetRefCoordSys(COORDS_WORLD); break;
				case IDC_ACTIVE_PARENT:
					co->ip->SetRefCoordSys(COORDS_PARENT); break;
				case IDC_ACTIVE_LOCALSYS:
					co->ip->SetRefCoordSys(COORDS_LOCAL); break;
				case IDC_ACTIVE_OBJECT:
					co->ip->SetRefCoordSys(COORDS_OBJECT); break;


				case IDC_ENABLE_X:
					xEnabled = !xEnabled;
					co->ip->EnableAxisConstraints(AXIS_X,xEnabled);
					break;
				case IDC_ENABLE_Y:
					yEnabled = !yEnabled;
					co->ip->EnableAxisConstraints(AXIS_Y,yEnabled); 
					break;
				case IDC_ENABLE_Z:
					zEnabled = !zEnabled;
					co->ip->EnableAxisConstraints(AXIS_Z,zEnabled); 
					break;
				case IDC_ENABLE_XY:					
				case IDC_ENABLE_YZ:					
				case IDC_ENABLE_ZX:
					xyEnabled = !xyEnabled;
					co->ip->EnableAxisConstraints(AXIS_XY,xyEnabled); 
					break;

				case IDC_ENABLE_SYS:
				case IDC_ENABLE_SEL:
				case IDC_ENABLE_LOCAL:
					centerEnabled = !centerEnabled;
					co->ip->EnableCoordCenter(centerEnabled); 
					break;
					
				case IDC_ENABLE_VIEW:					
				case IDC_ENABLE_SCREEN:					
				case IDC_ENABLE_WORLD:					
				case IDC_ENABLE_PARENT:					
				case IDC_ENABLE_LOCALSYS:
				case IDC_ENABLE_OBJECT:
					refEnabled = !refEnabled;
					co->ip->EnableRefCoordSys(refEnabled);
					break;


				case IDC_SHOWFOOTPRINTS:
					if (IsDlgButtonChecked(hDlg,IDC_SHOWFOOTPRINTS)) {
						co->showSteps = TRUE;
					} else {
						co->showSteps = FALSE;
						}
					break;
				}
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}

void FootStepControl::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
	{
	editCtrl = this;
	this->ip = ip;	
	hParams  = ip->AddRollupPage( 
		getResMgr().getHInst(RES_ID_RB), 
		MAKEINTRESOURCE(IDD_FOOTSTEPPARAMS),
		FootStepParamDialogProc,
		GetResString(IDS_RB_FOOTSTEPPARAMS), 
		(LPARAM)this );		
	ip->RegisterDlgWnd(hParams);	
	
	moveMode    = new MoveCtrlApparatusCMode(this,ip);
	rotMode     = new RotateCtrlApparatusCMode(this,ip);
	uscaleMode  = new UScaleCtrlApparatusCMode(this,ip);
	nuscaleMode = new NUScaleCtrlApparatusCMode(this,ip);
	squashMode  = new SquashCtrlApparatusCMode(this,ip);
	selectMode  = new SelectCtrlApparatusCMode(this,ip);

	TSTR type(GetResString(IDS_RB_FOOTPRINT));	
	TCHAR *ptype = type;
	ip->RegisterSubObjectTypes((const TCHAR**)&ptype,1);
	
	if (!showSteps) {
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		ip->RedrawViews(ip->GetTime());
		}
	}

void FootStepControl::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
	{
	editCtrl = NULL;
	if (!showSteps) {
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		ip->RedrawViews(ip->GetTime());
		}	
	ip->UnRegisterDlgWnd(hParams);
	ip->DeleteRollupPage(hParams);
	this->ip=NULL;
	hParams=NULL;

	// This ensures that the mode isn't left around after editing is
	// completed.
	ip->DeleteMode(moveMode);
	ip->DeleteMode(rotMode);
	ip->DeleteMode(uscaleMode);
	ip->DeleteMode(nuscaleMode);
	ip->DeleteMode(squashMode);
	ip->DeleteMode(selectMode);
	delete moveMode; moveMode = NULL;
	delete rotMode; rotMode = NULL;
	delete uscaleMode; uscaleMode = NULL;
	delete nuscaleMode; nuscaleMode = NULL;
	delete squashMode; squashMode = NULL;
	delete selectMode; selectMode = NULL;	
	}




