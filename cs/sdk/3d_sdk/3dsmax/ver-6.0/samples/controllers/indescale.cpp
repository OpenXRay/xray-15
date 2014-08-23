/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: indescale.cpp

	 DESCRIPTION: An independent X, Y, Z scale controller
	              Based upon: class IndePosition

	 CREATED BY: Michael Malone (mjm)

	 HISTORY: created September 15, 1998

   	 Copyright (c) 1998, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */

#include "ctrl.h"

#define ISCALE_CONTROL_CLASS_ID		Class_ID(0x118f7c01,0xfeee238b)
#define ISCALE_CONTROL_CNAME		GetString(IDS_MM_ISCALE)

#define ISCALE_X_REF 0
#define ISCALE_Y_REF 1
#define ISCALE_Z_REF 2

class IScaleDlg;

static DWORD subColor[] = { PAINTCURVE_XCOLOR, PAINTCURVE_YCOLOR, PAINTCURVE_ZCOLOR };

class IndeScale : public Control
{
	public:
		Control *scaleX;
		Control *scaleY;
		Control *scaleZ;
		ScaleValue curval;
		Interval ivalid;

		static IScaleDlg *dlg;
		static IObjParam *ip;
		static ULONG beginFlags;
		static IndeScale *editControl; // The one being edited.
		Tab<TimeValue> keyTimes;
		
		IndeScale(BOOL loading=FALSE);
		IndeScale(const IndeScale &ctrl);
		~IndeScale();
		void Update(TimeValue t);

		// from class Animatable:
		Class_ID ClassID()
			{ return ISCALE_CONTROL_CLASS_ID; } 
		SClass_ID SuperClassID()
			{ return CTRL_SCALE_CLASS_ID; } 
		void GetClassName(TSTR& s)
			{ s = ISCALE_CONTROL_CNAME; }
		void DeleteThis()
			{ delete this; }		
		int IsKeyable()
			{ return 1;	}		
		int NumSubs()
			{ return 3; }
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int subNum)
			{ return subNum; }
		DWORD GetSubAnimCurveColor(int subNum)
			{ return subColor[subNum]; }
		ParamDimension* GetParamDimension(int i)
			{ return stdPercentDim; }
		BOOL AssignController(Animatable *control,int subAnim);
		void AddNewKey(TimeValue t,DWORD flags);
		int NumKeys();
		TimeValue GetKeyTime(int index);
		void SelectKeyByIndex(int i,BOOL sel);
		BOOL IsKeySelected(int i);
		void CopyKeysFromTime(TimeValue src,TimeValue dst,DWORD flags);
		BOOL IsKeyAtTime(TimeValue t,DWORD flags);
		BOOL GetNextKeyTime(TimeValue t,DWORD flags,TimeValue &nt);
		void DeleteKeyAtTime(TimeValue t);
		void DeleteKeys(DWORD flags);
		void DeleteKeyByIndex(int index);

		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev );
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next );
		
		// from class ReferenceMaker:
		int NumRefs()
			{ return 3; };	
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
		RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage);
		
		// from class Control:
		Control *GetXController()
			{ return scaleX; }
		Control *GetYController()
			{ return scaleY; }
		Control *GetZController()
			{ return scaleZ; }
		void Copy(Control *from);
		RefTargetHandle Clone(RemapDir& remap);
		BOOL IsLeaf()
			{ return FALSE; }
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method);	
		void SetValue(TimeValue t, void *val, int commit, GetSetMethod method);
		void CommitValue(TimeValue t);
		void RestoreValue(TimeValue t);

		// AF (09/20/02) Pass these methods onto the subAnims
		int GetORT(int type);
		void SetORT(int ort, int type);
		void EnableORTs(BOOL enable);
};

IScaleDlg *IndeScale::dlg		  = NULL;
IObjParam *IndeScale::ip		  = NULL;
ULONG	   IndeScale::beginFlags  = 0;
IndeScale *IndeScale::editControl = NULL;

class IScaleClassDesc:public ClassDesc
{
	public:
		int IsPublic()
			{ return 1; }
		void* Create(BOOL loading)
			{ return new IndeScale(loading); }
		const TCHAR* ClassName()
			{ return ISCALE_CONTROL_CNAME; }
		SClass_ID SuperClassID()
			{ return CTRL_SCALE_CLASS_ID; }
		Class_ID ClassID()
			{ return ISCALE_CONTROL_CLASS_ID; }
		const TCHAR* Category()
			{ return _T(""); }
};

static IScaleClassDesc iscaleCD;
ClassDesc* GetIScaleCtrlDesc()
	{ return &iscaleCD; }

static INT_PTR CALLBACK IScaleParamDialogProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);

static const int editButs[] = { IDC_ISCALE_X, IDC_ISCALE_Y, IDC_ISCALE_Z };

#define EDIT_X 0
#define EDIT_Y 1
#define EDIT_Z 2

#define ISCALE_BEGIN 1
#define ISCALE_MIDDLE 2
#define ISCALE_END 3

class IScaleDlg
{
	public:
		IndeScale *cont;
		HWND hWnd;
		IObjParam *ip;
		ICustButton *iEdit[3];
		static int cur;
		
		IScaleDlg(IndeScale *cont,IObjParam *ip);
		~IScaleDlg();

		void Init();
		void EndingEdit(IndeScale *next);
		void BeginingEdit(IndeScale *cont,IObjParam *ip,IndeScale *prev);
		void SetCur(int c,int code=ISCALE_MIDDLE);
		void WMCommand(int id, int notify, HWND hCtrl);
};

int IScaleDlg::cur = EDIT_X;

IScaleDlg::IScaleDlg(IndeScale *cont,IObjParam *ip)
{
	this->ip = ip;
	this->cont = cont;
	for (int i=0; i<3; i++)
		iEdit[i] = NULL;
	
	TCHAR *name = GetString(IDS_MM_ISCALEPARAMS);

	hWnd = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_ISCALE_PARAMS),
							 IScaleParamDialogProc, name, (LPARAM)this);

	ip->RegisterDlgWnd(hWnd);	
	
	SetCur(cur,ISCALE_BEGIN);	
	UpdateWindow(hWnd);
}

IScaleDlg::~IScaleDlg()
{
	SetCur(cur,ISCALE_END);
	for (int i=0; i<3; i++)
		ReleaseICustButton(iEdit[i]);
	ip->UnRegisterDlgWnd(hWnd);
	ip->DeleteRollupPage(hWnd);
	hWnd = NULL;
}

void IScaleDlg::EndingEdit(IndeScale *next)
{
	switch (cur)
	{
	case EDIT_X:
		cont->scaleX->EndEditParams(ip,0,next->scaleX);
		break;
	case EDIT_Y:
		cont->scaleY->EndEditParams(ip,0,next->scaleY);
		break;
	case EDIT_Z:
		cont->scaleZ->EndEditParams(ip,0,next->scaleZ);
		break;
	}
	cont = NULL;
	ip   = NULL;
}

void IScaleDlg::BeginingEdit(IndeScale *cont,IObjParam *ip,IndeScale *prev)
{
	this->ip   = ip;
	this->cont = cont;
	switch (cur)
	{
	case EDIT_X:
		cont->scaleX->BeginEditParams(ip,BEGIN_EDIT_MOTION,prev->scaleX);
		break;
	case EDIT_Y:
		cont->scaleY->BeginEditParams(ip,BEGIN_EDIT_MOTION,prev->scaleY);
		break;
	case EDIT_Z:
		cont->scaleZ->BeginEditParams(ip,BEGIN_EDIT_MOTION,prev->scaleZ);
		break;
	}	
	UpdateWindow(hWnd);
}

void IScaleDlg::Init()
{	
	for (int i=0; i<3; i++)
	{
		iEdit[i] = GetICustButton(GetDlgItem(hWnd,editButs[i]));		
		iEdit[i]->SetType(CBT_CHECK);
	}
	iEdit[cur]->SetCheck(TRUE);
}

void IScaleDlg::SetCur(int c,int code)
{
	if (c==cur && code==ISCALE_MIDDLE) return;
	Control *prev = NULL, *next = NULL;

	if (code!=ISCALE_END)
	{
		switch (c)
		{
		case EDIT_X:
			next = cont->scaleX;
			break;
		case EDIT_Y:
			next = cont->scaleY;
			break;
		case EDIT_Z:
			next = cont->scaleZ;
			break;
		}
	}

	if (code != ISCALE_BEGIN)
	{
		switch (cur)
		{
		case EDIT_X:
			cont->scaleX->EndEditParams(ip, END_EDIT_REMOVEUI, next);
			prev = cont->scaleX;
			break;
		case EDIT_Y:
			cont->scaleY->EndEditParams(ip, END_EDIT_REMOVEUI, next);
			prev = cont->scaleY;
			break;
		case EDIT_Z:
			cont->scaleZ->EndEditParams(ip, END_EDIT_REMOVEUI, next);
			prev = cont->scaleZ;
			break;
		}
	}

	cur = c;

	if (code != ISCALE_END)
	{
		switch (cur)
		{
		case EDIT_X:
			cont->scaleX->BeginEditParams(ip, BEGIN_EDIT_MOTION, prev);
			break;
		case EDIT_Y:
			cont->scaleY->BeginEditParams(ip, BEGIN_EDIT_MOTION, prev);
			break;
		case EDIT_Z:
			cont->scaleZ->BeginEditParams(ip, BEGIN_EDIT_MOTION, prev);
			break;
		}
	}
}

void IScaleDlg::WMCommand(int id, int notify, HWND hCtrl)
{
	switch (id)
	{
	case IDC_ISCALE_X:
		SetCur(0);
		iEdit[0]->SetCheck(TRUE);
		iEdit[1]->SetCheck(FALSE);
		iEdit[2]->SetCheck(FALSE);
		break;
	case IDC_ISCALE_Y:
		SetCur(1);
		iEdit[0]->SetCheck(FALSE);
		iEdit[1]->SetCheck(TRUE);
		iEdit[2]->SetCheck(FALSE);
		break;
	case IDC_ISCALE_Z:
		SetCur(2);
		iEdit[0]->SetCheck(FALSE);
		iEdit[1]->SetCheck(FALSE);
		iEdit[2]->SetCheck(TRUE);
		break;
	}
}

static INT_PTR CALLBACK IScaleParamDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	IScaleDlg *dlg = (IScaleDlg*)GetWindowLongPtr(hDlg,GWLP_USERDATA);

	switch (message)
	{
	case WM_INITDIALOG:
		dlg = (IScaleDlg*)lParam;			
		SetWindowLongPtr(hDlg,GWLP_USERDATA,lParam);
		dlg->hWnd = hDlg;
		dlg->Init();
		break;
	
	case WM_COMMAND:
		dlg->WMCommand(LOWORD(wParam),HIWORD(wParam),(HWND)lParam);
		break;

	case WM_LBUTTONDOWN:case WM_LBUTTONUP:	case WM_MOUSEMOVE:
		dlg->ip->RollupMouseMessage(hDlg,message,wParam,lParam);
		break;
			
	default:
		return FALSE;
	}
	return TRUE;
}



IndeScale::IndeScale(const IndeScale &ctrl)
{
	scaleX = NULL;
	scaleY = NULL;
	scaleZ = NULL;

	if (ctrl.scaleX)
		ReplaceReference(ISCALE_X_REF, ctrl.scaleX);
	else
		ReplaceReference(ISCALE_X_REF, NewDefaultFloatController());

	if (ctrl.scaleY)
		ReplaceReference(ISCALE_Y_REF, ctrl.scaleY);
	else
		ReplaceReference(ISCALE_Y_REF, NewDefaultFloatController());

	if (ctrl.scaleZ)
		ReplaceReference(ISCALE_Z_REF, ctrl.scaleZ);
	else
		ReplaceReference(ISCALE_Z_REF, NewDefaultFloatController());
	
	curval = ctrl.curval;
	ivalid = ctrl.ivalid;
}

IndeScale::IndeScale(BOOL loading) 
{
	scaleX = NULL;
	scaleY = NULL;
	scaleZ = NULL;
	if (!loading)
	{
		ReplaceReference(ISCALE_X_REF, NewDefaultFloatController());
		ReplaceReference(ISCALE_Y_REF, NewDefaultFloatController());
		ReplaceReference(ISCALE_Z_REF, NewDefaultFloatController());
		ivalid = FOREVER;
		curval = ScaleValue(Point3(1,1,1));
	} else {
		ivalid.SetEmpty();
	}	
}

RefTargetHandle IndeScale::Clone(RemapDir& remap) 
{
	IndeScale *scale = new IndeScale(TRUE);	
	scale->ReplaceReference(ISCALE_X_REF, remap.CloneRef(scaleX));
	scale->ReplaceReference(ISCALE_Y_REF, remap.CloneRef(scaleY));
	scale->ReplaceReference(ISCALE_Z_REF, remap.CloneRef(scaleZ));
	BaseClone(this, scale, remap);
	return scale;
}

IndeScale::~IndeScale()
{
	DeleteAllRefsFromMe();
}

void IndeScale::Copy(Control *from)
{
	if (from->ClassID()==ClassID())
	{
		IndeScale *ctrl = (IndeScale*)from;
		ReplaceReference(ISCALE_X_REF, ctrl->scaleX);
		ReplaceReference(ISCALE_Y_REF, ctrl->scaleY);
		ReplaceReference(ISCALE_Z_REF, ctrl->scaleZ);
		curval = ctrl->curval;
		ivalid = ctrl->ivalid;
	} else {		
		ScaleValue v;
		Interval iv;
		int num;		
		if ((num=from->NumKeys())!=NOT_KEYFRAMEABLE && num>0)
		{
			SuspendAnimate();
			AnimateOn();
			for (int i=0; i<num; i++)
			{
				TimeValue t = from->GetKeyTime(i);
				from->GetValue(t,&v,iv);
				SetValue(t,&v,TRUE,CTRL_ABSOLUTE);
			}
			// RB 2/10/99: A key at frame 0 may have been created
			if (num>0 && from->GetKeyTime(0)!=0) {
				scaleX->DeleteKeyAtTime(0);
				scaleY->DeleteKeyAtTime(0);
				scaleZ->DeleteKeyAtTime(0);
			}
			ResumeAnimate();
		} else {
			from->GetValue(0,&v,ivalid);
			SetValue(0,&v,TRUE,CTRL_ABSOLUTE);
		}
	}
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
}

// added 020819  --prs.
#if 1
void IndeScale::SelectKeyByIndex(int i, BOOL sel)
	{
	int nk = NumKeys();				// sets up keyTimes table
	if (i < nk) {
		TimeValue tv = GetKeyTime(i);
		if (scaleX) {
			int xkey = scaleX->GetKeyIndex(tv);
			if (xkey >= 0)
				scaleX->SelectKeyByIndex(xkey, sel);
			}
		if (scaleY) {
			int ykey = scaleY->GetKeyIndex(tv);
			if (ykey >= 0)
				scaleY->SelectKeyByIndex(ykey, sel);
			}
		if (scaleZ) {
			int zkey = scaleZ->GetKeyIndex(tv);
			if (zkey >= 0)
				scaleZ->SelectKeyByIndex(zkey, sel);
			}
		}
	}

BOOL IndeScale::IsKeySelected(int i)
	{
	if (i < keyTimes.Count()) {
		TimeValue tv = GetKeyTime(i);
		if (scaleX) {
			int xkey = scaleX->GetKeyIndex(tv);
			if (xkey >= 0 && scaleX->IsKeySelected(xkey))
				return TRUE;
			}
		if (scaleY) {
			int ykey = scaleY->GetKeyIndex(tv);
			if (ykey >= 0 && scaleY->IsKeySelected(ykey))
				return TRUE;
			}
		if (scaleZ) {
			int zkey = scaleZ->GetKeyIndex(tv);
			if (zkey >= 0 && scaleZ->IsKeySelected(zkey))
				return TRUE;
			}
		}
	return FALSE;
	}
#endif

void IndeScale::Update(TimeValue t)
{
	if (!ivalid.InInterval(t))
	{
		ivalid = FOREVER;		
		if (scaleX)
			scaleX->GetValue(t,&curval.s.x,ivalid);
		if (scaleY)
			scaleY->GetValue(t,&curval.s.y,ivalid);
		if (scaleZ)
			scaleZ->GetValue(t,&curval.s.z,ivalid);
		// ------------------------------------------------------------------------
		// ignoring the quaternion here... can't find anyplace in MAX that uses it,
		// could later add a rotation leaf controller for off-axis scaling
		//   -- mjm
		// ------------------------------------------------------------------------
	}
}

void IndeScale::SetValue(TimeValue t, void *val, int commit, GetSetMethod method)
{
	ScaleValue *v = (ScaleValue*)val;

	if (method==CTRL_RELATIVE)
	{
		// ------------------------------------------------------------------------
		// By default, a relative setValue() on an interp float controller does a
		// += instead of *=. Therefore, we have to first get their existing value,
		// manually update (*=), then do an absolute setValue() with the result.
		//   -- mjm
		// ------------------------------------------------------------------------
		float childScale;
		if (scaleX)
		{
			scaleX->GetValue(t,&childScale,FOREVER);
			v->s.x *= childScale;
		}
		if (scaleY)
		{
			scaleY->GetValue(t,&childScale,FOREVER);
			v->s.y *= childScale;
		}
		if (scaleZ)
		{
			scaleZ->GetValue(t,&childScale,FOREVER);
			v->s.z *= childScale;
		}
		method = CTRL_ABSOLUTE;
	}

	if (scaleX)
		scaleX->SetValue(t,&v->s.x,commit,method);
	if (scaleY)
		scaleY->SetValue(t,&v->s.y,commit,method);
	if (scaleZ)
		scaleZ->SetValue(t,&v->s.z,commit,method);
	// ------------------------------------------------------------------------
	// ignoring the quaternion here... can't find anyplace in MAX that uses it,
	// could later add a rotation leaf controller for off-axis scaling
	//   -- mjm
	// ------------------------------------------------------------------------

	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
}

void IndeScale::GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method)
{		
	Update(t);
	valid &= ivalid;			 
	if (method==CTRL_RELATIVE)
	{
  		Matrix3 *mat = (Matrix3*)val;
		ApplyScaling(*mat,curval);
	} else {
		*((ScaleValue*)val) = curval;
	}
}

void IndeScale::CommitValue(TimeValue t)
{
	if (scaleX)
		scaleX->CommitValue(t);
	if (scaleY)
		scaleY->CommitValue(t);
	if (scaleZ)
		scaleZ->CommitValue(t);
}

void IndeScale::RestoreValue(TimeValue t)
{
	if (scaleX)
		scaleX->RestoreValue(t);
	if (scaleY)
		scaleY->RestoreValue(t);
	if (scaleZ)
		scaleZ->RestoreValue(t);
}

RefTargetHandle IndeScale::GetReference(int i)
{
	switch (i)
	{
	case ISCALE_X_REF:
		return scaleX;
	case ISCALE_Y_REF:
		return scaleY;
	case ISCALE_Z_REF:
		return scaleZ;
	default:
		return NULL;
	}
}

void IndeScale::SetReference(int i, RefTargetHandle rtarg)
{
	switch (i)
	{
	case ISCALE_X_REF:
		scaleX = (Control*)rtarg;
		break;
	case ISCALE_Y_REF:
		scaleY = (Control*)rtarg;
		break;
	case ISCALE_Z_REF:
		scaleZ = (Control*)rtarg;
		break;
	}
}

Animatable* IndeScale::SubAnim(int i)
{
	return GetReference(i);
}

TSTR IndeScale::SubAnimName(int i)
{
	switch (i)
	{
	case ISCALE_X_REF:
		return GetString(IDS_MM_XSCALE);
	case ISCALE_Y_REF:
		return GetString(IDS_MM_YSCALE);
	case ISCALE_Z_REF:
		return GetString(IDS_MM_ZSCALE);
	default:
		return _T("");
	}
}

RefResult IndeScale::NotifyRefChanged( Interval iv, RefTargetHandle hTarg, PartID& partID, RefMessage msg)
{
	switch (msg)
	{
	case REFMSG_CHANGE:
		ivalid.SetEmpty();
		break;
	case REFMSG_TARGET_DELETED:
		if (scaleX == hTarg)
			scaleX = NULL;
		if (scaleY == hTarg)
			scaleY = NULL;
		if (scaleZ == hTarg)
			scaleZ = NULL; 
		break;
	case REFMSG_GET_CONTROL_DIM:
		ParamDimension **dim = (ParamDimension **)partID;
		assert(dim);
		*dim = stdPercentDim;
		break;
	}
	return REF_SUCCEED;
}

BOOL IndeScale::AssignController(Animatable *control,int subAnim)
{
	// ------------------------------------------------------------------------
	// Interp float controllers initialize to 0.
	// For scaling, we must initialize them to 1.
	//   -- mjm
	// ------------------------------------------------------------------------
	if ( control && !((Control *)control)->NumKeys() )
	{
		float ctrlScale;
		((Control *)control)->GetValue(0,&ctrlScale,FOREVER);
		if (ctrlScale == 0.0f)
		{
			ctrlScale = 1.0f;
			((Control *)control)->SetValue(0,&ctrlScale);
		}
	}
	
	switch (subAnim)
	{
	case ISCALE_X_REF:
		ReplaceReference(ISCALE_X_REF,(RefTargetHandle)control);
		break;
	case ISCALE_Y_REF:
		ReplaceReference(ISCALE_Y_REF,(RefTargetHandle)control);
		break;
	case ISCALE_Z_REF:
		ReplaceReference(ISCALE_Z_REF,(RefTargetHandle)control);
		break;
	}

	ivalid.SetEmpty();

	NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE,TREE_VIEW_CLASS_ID,FALSE);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);	
	return TRUE;
}

void IndeScale::AddNewKey(TimeValue t,DWORD flags)
{
	if (scaleX)
		scaleX->AddNewKey(t,flags);
	if (scaleY)
		scaleY->AddNewKey(t,flags);
	if (scaleZ)
		scaleZ->AddNewKey(t,flags);
}

// changed 020819  --prs.
int IndeScale::NumKeys()
{
#if 1
	int xnum = scaleX->NumKeys();
	int ynum = scaleY->NumKeys();
	int znum = scaleZ->NumKeys();
	keyTimes.ZeroCount();
	for (int i = 0, j = 0, k = 0;
		 i < xnum || j < ynum || k < znum; ) {
		TimeValue k1 = i < xnum ? scaleX->GetKeyTime(i) : TIME_PosInfinity;
		TimeValue k2 = j < ynum ? scaleY->GetKeyTime(j) : TIME_PosInfinity;
		TimeValue k3 = k < znum ? scaleZ->GetKeyTime(k) : TIME_PosInfinity;
		TimeValue kmin = k1 < k2 ? (k3 < k1 ? k3 : k1) : (k3 < k2 ? k3 : k2);
		keyTimes.Append(1, &kmin, 10);
		if (k1 == kmin) ++i;
		if (k2 == kmin) ++j;
		if (k3 == kmin) ++k;
	}
	return keyTimes.Count();
#else
	int num = 0;
	if (scaleX)
		num += scaleX->NumKeys(); 
	if (scaleY)
		num += scaleY->NumKeys();
	if (scaleZ)
		num += scaleZ->NumKeys();
	return num;
#endif
}

TimeValue IndeScale::GetKeyTime(int index)
{
#if 1	// 020819  --prs.
	return keyTimes[index];
#else
	int onum,num = 0;
	if (scaleX)
		num += scaleX->NumKeys(); 
	if (index < num)
		return scaleX->GetKeyTime(index);

	onum = num;
	if (scaleY)
		num += scaleY->NumKeys(); 
	if (index < num)
		return scaleY->GetKeyTime(index-onum);

	onum = num;
	if (scaleZ)
		num += scaleZ->NumKeys(); 
	if (index < num)
		return scaleZ->GetKeyTime(index-onum);

	return 0;
#endif
}

void IndeScale::CopyKeysFromTime(TimeValue src,TimeValue dst,DWORD flags)
{
	if (scaleX)
		scaleX->CopyKeysFromTime(src,dst,flags);
	if (scaleY)
		scaleY->CopyKeysFromTime(src,dst,flags);
	if (scaleZ)
		scaleZ->CopyKeysFromTime(src,dst,flags);
}

BOOL IndeScale::IsKeyAtTime(TimeValue t,DWORD flags)
{
	if (scaleX && scaleX->IsKeyAtTime(t,flags))
		return TRUE;
	if (scaleY && scaleY->IsKeyAtTime(t,flags))
		return TRUE;
	if (scaleZ && scaleZ->IsKeyAtTime(t,flags))
		return TRUE;
	return FALSE;
}

void IndeScale::DeleteKeyAtTime(TimeValue t)
{
	if (scaleX)
		scaleX->DeleteKeyAtTime(t);
	if (scaleY)
		scaleY->DeleteKeyAtTime(t);
	if (scaleZ)
		scaleZ->DeleteKeyAtTime(t);
}

// added 020823  --prs.
void IndeScale::DeleteKeys(DWORD flags)
	{
	if (scaleX) scaleX->DeleteKeys(flags);
	if (scaleY) scaleY->DeleteKeys(flags);
	if (scaleZ) scaleZ->DeleteKeys(flags);
	}

// added 020813  --prs.
#if 1
void IndeScale::DeleteKeyByIndex(int index)
	{
	int nk = NumKeys();	// sets up keyTimes table

	if (index < nk)
		DeleteKeyAtTime(keyTimes[index]);
	}
#endif

BOOL IndeScale::GetNextKeyTime(TimeValue t,DWORD flags,TimeValue &nt)
{
	TimeValue at,tnear = 0;
	BOOL tnearInit = FALSE;
	
	if (scaleX && scaleX->GetNextKeyTime(t,flags,at))
	{
		if (!tnearInit)
		{
			tnear = at;
			tnearInit = TRUE;
		} else if ( ABS(at-t) < ABS(tnear-t) ) {
			tnear = at;
		}
	}

	if (scaleY && scaleY->GetNextKeyTime(t,flags,at))
	{
		if (!tnearInit)
		{
			tnear = at;
			tnearInit = TRUE;
		} else if ( ABS(at-t) < ABS(tnear-t) ) {
			tnear = at;
		}
	}

	if (scaleZ && scaleZ->GetNextKeyTime(t,flags,at))
	{
		if (!tnearInit)
		{
			tnear = at;
			tnearInit = TRUE;
		} else if ( ABS(at-t) < ABS(tnear-t) ) {
			tnear = at;
		}
	}
	
	if (tnearInit)
	{
		nt = tnear;
		return TRUE;
	} else {
		return FALSE;
	}
}
		

void IndeScale::BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev)
{	
	this->ip = ip;

	if (dlg)
		dlg->BeginingEdit(this,ip,(IndeScale*)prev);
	else
		dlg = new IScaleDlg(this,ip);
}

void IndeScale::EndEditParams(IObjParam *ip, ULONG flags,Animatable *next)
{	
	IndeScale *cont=NULL;
	if ( next && next->ClassID()==ClassID() )
		cont = (IndeScale*)next;

	if (dlg)
	{
		if (cont)
		{
			dlg->EndingEdit(cont);
		} else {
			delete dlg;
			dlg = NULL;
		}
	}
}

int IndeScale::GetORT(int type)
	{
	if (scaleX) return scaleX->GetORT(type);  // AF -- you got to pick something...
	return 0;
	}

void IndeScale::SetORT(int ort,int type)
	{
	if (scaleX) scaleX->SetORT(ort, type);
	if (scaleY) scaleY->SetORT(ort, type);
	if (scaleZ) scaleZ->SetORT(ort, type);
	}

void IndeScale::EnableORTs(BOOL enable)
	{
	if (scaleX) scaleX->EnableORTs(enable);
	if (scaleY) scaleY->EnableORTs(enable);
	if (scaleZ) scaleZ->EnableORTs(enable);
	}
