/**********************************************************************
 *<
	FILE: indepos.cpp

	DESCRIPTION: An independent X, Y, Z position controller

	CREATED BY: Rolf Berteig

	HISTORY: created 13 June 1995

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "ctrl.h"

#define IPOS_CONTROL_CLASS_ID	Class_ID(0x118f7e02,0xffee238a)
#define IPOS_CONTROL_CNAME		GetString(IDS_RB_IPOS)

#define IPOINT3_CONTROL_CLASS_ID	Class_ID(0x118f7e02,0xfeee238b)
#define IPOINT3_CONTROL_CNAME		GetString(IDS_RB_IPOINT3)

#define ICOLOR_CONTROL_CLASS_ID		Class_ID(0x118f7c01,0xfeee238a)
#define ICOLOR_CONTROL_CNAME		GetString(IDS_RB_ICOLOR)

#define IPOINT4_CONTROL_CLASS_ID	Class_ID(0x118f7e02,0xfeee238c)
#define IPOINT4_CONTROL_CNAME		GetString(IDS_RB_IPOINT4)

#define IACOLOR_CONTROL_CLASS_ID	Class_ID(0x118f7c01,0xfeee238b)
#define IACOLOR_CONTROL_CNAME		GetString(IDS_RB_IACOLOR)

#define IPOS_X_REF		0
#define IPOS_Y_REF		1
#define IPOS_Z_REF		2
#define IPOS_W_REF		3

class IPosDlg;

static DWORD subColor[] = {PAINTCURVE_XCOLOR, PAINTCURVE_YCOLOR, PAINTCURVE_ZCOLOR, PAINTCURVE_WCOLOR};

class IndePosition : public Control {
	public:
		Control *posX;
		Control *posY;
		Control *posZ;
		Point3 curval;
		Interval ivalid;
		BOOL blockUpdate;
		// To support trajectory path.
		int xkSkip;
		int ykSkip;
		int zkSkip;
		BitArray xKeys;
		BitArray yKeys;
		BitArray zKeys;
		Tab<TimeValue> keyTimes;

		static IPosDlg *dlg;
		static IObjParam *ip;
		static ULONG beginFlags;
		static IndePosition *editControl; // The one being edited.
		
		IndePosition(BOOL loading=FALSE);
		IndePosition(const IndePosition &ctrl);
		~IndePosition();
		void Update(TimeValue t);

		// Animatable methods
		Class_ID ClassID() { return IPOS_CONTROL_CLASS_ID;} 
		SClass_ID SuperClassID() {return CTRL_POSITION_CLASS_ID;} 
		
		void GetClassName(TSTR& s) {s = IPOS_CONTROL_CNAME;}
		void DeleteThis() {delete this;}		
		int IsKeyable() {return 1;}		

		int NumSubs()  {return 3;}
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int subNum) {return subNum;}

		DWORD GetSubAnimCurveColor(int subNum) {return subColor[subNum];}

		ParamDimension* GetParamDimension(int i) {return stdWorldDim;}
		BOOL AssignController(Animatable *control,int subAnim);
		void AddNewKey(TimeValue t,DWORD flags);
		int NumKeys();
		TimeValue GetKeyTime(int index);
		int GetKeyTimes(Tab<TimeValue>&, Interval, DWORD flags);
		int GetKeySelState(BitArray&, Interval, DWORD flags);
		void SelectKeys(TrackHitTab& sel, DWORD flags);
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
		
		// Reference methods
		int NumRefs() { return 3; };	
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
		RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage);
		
		// Control methods
		Control *GetXController() {return posX;}
		Control *GetYController() {return posY;}
		Control *GetZController() {return posZ;}
		void Copy(Control *from);
		RefTargetHandle Clone(RemapDir& remap);
		BOOL IsLeaf() {return FALSE;}
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method);	
		void SetValue(TimeValue t, void *val, int commit, GetSetMethod method);
		void CommitValue(TimeValue t);
		void RestoreValue(TimeValue t);	

		// AF (09/20/02) Pass these methods onto the subAnims
		int GetORT(int type);
		void SetORT(int ort, int type);
		void EnableORTs(BOOL enable);
		
		BOOL CreateLockKey(TimeValue t, int which);

	};

IPosDlg       *IndePosition::dlg         = NULL;
IObjParam     *IndePosition::ip          = NULL;
ULONG          IndePosition::beginFlags  = 0;
IndePosition  *IndePosition::editControl = NULL;

class IndePoint3 : public IndePosition {
	public:
		IndePoint3(BOOL loading=FALSE) : IndePosition(loading) {}
		SClass_ID SuperClassID() {return CTRL_POINT3_CLASS_ID;} 
		Class_ID ClassID() { return IPOINT3_CONTROL_CLASS_ID;} 
		void GetClassName(TSTR& s) {s = IPOINT3_CONTROL_CNAME;}
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method);
		RefTargetHandle Clone(RemapDir& remap);
	};

class IndeColor : public IndePoint3 {
	public:
		IndeColor(BOOL loading=FALSE) : IndePoint3(loading) {}
		SClass_ID SuperClassID() {return CTRL_POINT3_CLASS_ID;} 
		Class_ID ClassID() { return ICOLOR_CONTROL_CLASS_ID;} 
		void GetClassName(TSTR& s) {s = ICOLOR_CONTROL_CNAME;}		
		ParamDimension* GetParamDimension(int i) {return stdColor255Dim;}
		RefTargetHandle Clone(RemapDir& remap);
	};

class IndePoint4 : public Control {
public:
	Control *posX;
	Control *posY;
	Control *posZ;
	Control *posW;
	Point4 curval;
	Interval ivalid;
	BOOL blockUpdate;
	// To support trajectory path.
	int xkSkip;
	int ykSkip;
	int zkSkip;
	int wkSkip;
	BitArray xKeys;
	BitArray yKeys;
	BitArray zKeys;
	BitArray wKeys;
	Tab<TimeValue> keyTimes;

	IndePoint4(BOOL loading=FALSE);
	IndePoint4(const IndePoint4 &ctrl);
	~IndePoint4();
	void Update(TimeValue t);

	// Animatable methods
	Class_ID ClassID() { return IPOINT4_CONTROL_CLASS_ID;} 
	SClass_ID SuperClassID() {return CTRL_POINT4_CLASS_ID;} 

	void GetClassName(TSTR& s) {s = IPOINT4_CONTROL_CNAME;}
	void DeleteThis() {delete this;}		
	int IsKeyable() {return 1;}		

	int NumSubs()  {return 4;}
	Animatable* SubAnim(int i);
	TSTR SubAnimName(int i);
	int SubNumToRefNum(int subNum) {return subNum;}

	DWORD GetSubAnimCurveColor(int subNum) {return subColor[subNum];}

	ParamDimension* GetParamDimension(int i) {return stdWorldDim;}
	BOOL AssignController(Animatable *control,int subAnim);
	void AddNewKey(TimeValue t,DWORD flags);
	int NumKeys();
	TimeValue GetKeyTime(int index);
	int GetKeyTimes(Tab<TimeValue>&, Interval, DWORD flags);
	int GetKeySelState(BitArray&, Interval, DWORD flags);
	void SelectKeys(TrackHitTab& sel, DWORD flags);
	void SelectKeyByIndex(int i,BOOL sel);
	BOOL IsKeySelected(int i);
	void CopyKeysFromTime(TimeValue src,TimeValue dst,DWORD flags);
	BOOL IsKeyAtTime(TimeValue t,DWORD flags);
	BOOL GetNextKeyTime(TimeValue t,DWORD flags,TimeValue &nt);
	void DeleteKeyAtTime(TimeValue t);
	void DeleteKeys(DWORD flags);
	void DeleteKeyByIndex(int index);

	// Reference methods
	int NumRefs() { return 4; };	
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);
	RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage);

	// Control methods
	Control *GetXController() {return posX;}
	Control *GetYController() {return posY;}
	Control *GetZController() {return posZ;}
	Control *GetWController() {return posW;}
	void Copy(Control *from);
	RefTargetHandle Clone(RemapDir& remap);
	BOOL IsLeaf() {return FALSE;}
	void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method);	
	void SetValue(TimeValue t, void *val, int commit, GetSetMethod method);
	void CommitValue(TimeValue t);
	void RestoreValue(TimeValue t);	

	// AF (09/20/02) Pass these methods onto the subAnims
	int GetORT(int type);
	void SetORT(int ort, int type);
	void EnableORTs(BOOL enable);

	BOOL CreateLockKey(TimeValue t, int which);

};

class IndeAColor : public IndePoint4 {
public:
	IndeAColor(BOOL loading=FALSE);
	SClass_ID SuperClassID() {return CTRL_POINT4_CLASS_ID;} 
	Class_ID ClassID() { return IACOLOR_CONTROL_CLASS_ID;} 
	void GetClassName(TSTR& s) {s = IACOLOR_CONTROL_CNAME;}		
	ParamDimension* GetParamDimension(int i) {return stdColorDim;}
	RefTargetHandle Clone(RemapDir& remap);
};

class IPosClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading) {return new IndePosition(loading);}
	const TCHAR *	ClassName() {return IPOS_CONTROL_CNAME;}
	SClass_ID		SuperClassID() {return CTRL_POSITION_CLASS_ID;}
	Class_ID		ClassID() {return IPOS_CONTROL_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
	};
static IPosClassDesc iposCD;
ClassDesc* GetIPosCtrlDesc() {return &iposCD;}

class IPoint3ClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading) {return new IndePoint3(loading);}
	const TCHAR *	ClassName() {return IPOINT3_CONTROL_CNAME;}
	SClass_ID		SuperClassID() {return CTRL_POINT3_CLASS_ID;}
	Class_ID		ClassID() {return IPOINT3_CONTROL_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
	};
static IPoint3ClassDesc ipoint3CD;
ClassDesc* GetIPoint3CtrlDesc() {return &ipoint3CD;}

class IColorClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading) {return new IndeColor(loading);}
	const TCHAR *	ClassName() {return ICOLOR_CONTROL_CNAME;}
	SClass_ID		SuperClassID() {return CTRL_POINT3_CLASS_ID;}
	Class_ID		ClassID() {return ICOLOR_CONTROL_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
	};
static IColorClassDesc icolorCD;
ClassDesc* GetIColorCtrlDesc() {return &icolorCD;}

class IPoint4ClassDesc:public ClassDesc {
public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading) {return new IndePoint4(loading);}
	const TCHAR *	ClassName() {return IPOINT4_CONTROL_CNAME;}
	SClass_ID		SuperClassID() {return CTRL_POINT4_CLASS_ID;}
	Class_ID		ClassID() {return IPOINT4_CONTROL_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
};
static IPoint4ClassDesc ipoint4CD;
ClassDesc* GetIPoint4CtrlDesc() {return &ipoint4CD;}

class IAColorClassDesc:public ClassDesc {
public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading) {return new IndeAColor(loading);}
	const TCHAR *	ClassName() {return IACOLOR_CONTROL_CNAME;}
	SClass_ID		SuperClassID() {return CTRL_POINT4_CLASS_ID;}
	Class_ID		ClassID() {return IACOLOR_CONTROL_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
};
static IAColorClassDesc iacolorCD;
ClassDesc* GetIAColorCtrlDesc() {return &iacolorCD;}


static const int editButs[] = {IDC_IPOS_X,IDC_IPOS_Y,IDC_IPOS_Z};

#define EDIT_X	0
#define EDIT_Y	1
#define EDIT_Z	2

#define IPOS_BEGIN		1
#define IPOS_MIDDLE		2
#define IPOS_END		3

static INT_PTR CALLBACK IPosParamDialogProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);

class IPosDlg {
	public:
		IndePosition *cont;
		HWND hWnd;
		IObjParam *ip;
		ICustButton *iEdit[3];
		static int cur;
		
		IPosDlg(IndePosition *cont,IObjParam *ip);
		~IPosDlg();

		void Init();
		void EndingEdit(IndePosition *next);
		void BeginingEdit(IndePosition *cont,IObjParam *ip,IndePosition *prev);
		void SetCur(int c,int code=IPOS_MIDDLE);
		void WMCommand(int id, int notify, HWND hCtrl);
	};

int IPosDlg::cur = EDIT_X;

IPosDlg::IPosDlg(IndePosition *cont,IObjParam *ip)
	{
	this->ip   = ip;
	this->cont = cont;
	for (int i=0; i<3; i++) {
		iEdit[i] = NULL;
		}
	
	TCHAR *name;
	if (cont->ClassID()==IPOS_CONTROL_CLASS_ID) 
		 name = GetString(IDS_RB_IPOSPARAMS);
	else 
	if (cont->ClassID()==IPOINT3_CONTROL_CLASS_ID) 
		 name = GetString(IDS_RB_IPOINT3PARAMS);
	else name = GetString(IDS_RB_ICOLORPARAMS);

	hWnd = ip->AddRollupPage( 
		hInstance,
		MAKEINTRESOURCE(IDD_IPOS_PARAMS),
		IPosParamDialogProc,
		name, 
		(LPARAM)this);
	ip->RegisterDlgWnd(hWnd);	
	
	SetCur(cur,IPOS_BEGIN);	
	UpdateWindow(hWnd);
	}

IPosDlg::~IPosDlg()
	{
	SetCur(cur,IPOS_END);
	for (int i=0; i<3; i++) {
		ReleaseICustButton(iEdit[i]);		
		}
	ip->UnRegisterDlgWnd(hWnd);
	ip->DeleteRollupPage(hWnd);
	hWnd = NULL;
	}

void IPosDlg::EndingEdit(IndePosition *next)
	{
	switch (cur) {
		case EDIT_X:
			cont->posX->EndEditParams(ip,0,next->posX);
			break;
		case EDIT_Y:
			cont->posY->EndEditParams(ip,0,next->posY);
			break;
		case EDIT_Z:
			cont->posZ->EndEditParams(ip,0,next->posZ);
			break;
		}
	cont = NULL;
	ip   = NULL;
	}

void IPosDlg::BeginingEdit(IndePosition *cont,IObjParam *ip,IndePosition *prev)
	{
	this->ip   = ip;
	this->cont = cont;
	switch (cur) {
		case EDIT_X:
			cont->posX->BeginEditParams(ip,BEGIN_EDIT_MOTION,prev->posX);
			break;
		case EDIT_Y:
			cont->posY->BeginEditParams(ip,BEGIN_EDIT_MOTION,prev->posY);
			break;
		case EDIT_Z:
			cont->posZ->BeginEditParams(ip,BEGIN_EDIT_MOTION,prev->posZ);
			break;
		}	
	UpdateWindow(hWnd);
	}

void IPosDlg::Init()
	{	
	for (int i=0; i<3; i++) {
		iEdit[i] = GetICustButton(GetDlgItem(hWnd,editButs[i]));		
		iEdit[i]->SetType(CBT_CHECK);
		}
	iEdit[cur]->SetCheck(TRUE);	
	}

void IPosDlg::SetCur(int c,int code)
	{
	if (c==cur && code==IPOS_MIDDLE) return;
	Control *prev = NULL, *next = NULL;

	if (code!=IPOS_END) {
		switch (c) {
			case EDIT_X:
				next = cont->posX;
				break;
			case EDIT_Y:
				next = cont->posY;
				break;
			case EDIT_Z:
				next = cont->posZ;
				break;
			}
		}

	if (code!=IPOS_BEGIN) {
		switch (cur) {
			case EDIT_X:
				cont->posX->EndEditParams(ip,END_EDIT_REMOVEUI,next);
				prev = cont->posX;
				break;
			case EDIT_Y:
				cont->posY->EndEditParams(ip,END_EDIT_REMOVEUI,next);
				prev = cont->posY;
				break;
			case EDIT_Z:
				cont->posZ->EndEditParams(ip,END_EDIT_REMOVEUI,next);
				prev = cont->posZ;
				break;
			}
		}

	cur = c;

	if (code!=IPOS_END) {
		switch (cur) {
			case EDIT_X:
				cont->posX->BeginEditParams(ip,BEGIN_EDIT_MOTION,prev);
				break;
			case EDIT_Y:
				cont->posY->BeginEditParams(ip,BEGIN_EDIT_MOTION,prev);
				break;
			case EDIT_Z:
				cont->posZ->BeginEditParams(ip,BEGIN_EDIT_MOTION,prev);
				break;
			}
		}
	}

void IPosDlg::WMCommand(int id, int notify, HWND hCtrl)
	{
	switch (id) {
		case IDC_IPOS_X:
			SetCur(0);
			iEdit[0]->SetCheck(TRUE);
			iEdit[1]->SetCheck(FALSE);
			iEdit[2]->SetCheck(FALSE);
			break;
		case IDC_IPOS_Y:
			SetCur(1);
			iEdit[0]->SetCheck(FALSE);
			iEdit[1]->SetCheck(TRUE);
			iEdit[2]->SetCheck(FALSE);
			break;
		case IDC_IPOS_Z:
			SetCur(2);
			iEdit[0]->SetCheck(FALSE);
			iEdit[1]->SetCheck(FALSE);
			iEdit[2]->SetCheck(TRUE);
			break;
		}
	}

static INT_PTR CALLBACK IPosParamDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
	{
	IPosDlg *dlg = (IPosDlg*)GetWindowLongPtr(hDlg,GWLP_USERDATA);

	switch (message) {
		case WM_INITDIALOG:
			dlg = (IPosDlg*)lParam;			
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



IndePosition::IndePosition(const IndePosition &ctrl)
	: xkSkip(0)
	, ykSkip(0)
	, zkSkip(0)
	{
	blockUpdate = FALSE;

	posX = NULL;
	posY = NULL;
	posZ = NULL;

	if (ctrl.posX) {
		ReplaceReference(IPOS_X_REF,ctrl.posX);
	} else {
		ReplaceReference(IPOS_X_REF,NewDefaultFloatController());
		}
	if (ctrl.posY) {
		ReplaceReference(IPOS_Y_REF,ctrl.posY);
	} else {
		ReplaceReference(IPOS_Y_REF,NewDefaultFloatController());
		}
	if (ctrl.posZ) {
		ReplaceReference(IPOS_Z_REF,ctrl.posZ);
	} else {
		ReplaceReference(IPOS_Z_REF,NewDefaultFloatController());
		}
	
	curval = ctrl.curval;
	ivalid = ctrl.ivalid;
	}

IndePosition::IndePosition(BOOL loading) 
	: xkSkip(0)
	, ykSkip(0)
	, zkSkip(0)
	{
	blockUpdate = FALSE;

	posX = NULL;
	posY = NULL;
	posZ = NULL;
	if (!loading) {
		ReplaceReference(IPOS_X_REF,NewDefaultFloatController());
		ReplaceReference(IPOS_Y_REF,NewDefaultFloatController());
		ReplaceReference(IPOS_Z_REF,NewDefaultFloatController());
		ivalid = FOREVER;
		curval = Point3(0,0,0);
	} else {
		ivalid.SetEmpty();
		}	
	}

RefTargetHandle IndePoint3::Clone(RemapDir& remap) 
	{
	IndePoint3 *pos = new IndePoint3(TRUE);	
	pos->ReplaceReference(IPOS_X_REF, remap.CloneRef(posX));
	pos->ReplaceReference(IPOS_Y_REF, remap.CloneRef(posY));
	pos->ReplaceReference(IPOS_Z_REF, remap.CloneRef(posZ));
	BaseClone(this, pos, remap);
	return pos;
	}

RefTargetHandle IndeColor::Clone(RemapDir& remap) 
	{
	IndeColor *pos = new IndeColor(TRUE);	
	pos->ReplaceReference(IPOS_X_REF, remap.CloneRef(posX));
	pos->ReplaceReference(IPOS_Y_REF, remap.CloneRef(posY));
	pos->ReplaceReference(IPOS_Z_REF, remap.CloneRef(posZ));
	BaseClone(this, pos, remap);
	return pos;
	}

BOOL IndePosition::CreateLockKey(TimeValue t, int which)
{
if (posX) posX->CreateLockKey(t,which);
if (posY) posY->CreateLockKey(t,which);
if (posZ) posZ->CreateLockKey(t,which);
return TRUE;
}

RefTargetHandle IndePosition::Clone(RemapDir& remap) 
	{
	IndePosition *pos = new IndePosition(TRUE);	
	pos->ReplaceReference(IPOS_X_REF, remap.CloneRef(posX));
	pos->ReplaceReference(IPOS_Y_REF, remap.CloneRef(posY));
	pos->ReplaceReference(IPOS_Z_REF, remap.CloneRef(posZ));
	BaseClone(this, pos, remap);
	return pos;
	}


IndePosition::~IndePosition()
	{
	DeleteAllRefsFromMe();
	}

void IndePosition::Copy(Control *from)
	{
	if (from->ClassID()==ClassID()) {
		IndePosition *ctrl = (IndePosition*)from;
		ReplaceReference(IPOS_X_REF,ctrl->posX);
		ReplaceReference(IPOS_Y_REF,ctrl->posY);
		ReplaceReference(IPOS_Z_REF,ctrl->posZ);
		curval = ctrl->curval;
		ivalid = ctrl->ivalid;
	} else {		
		Point3 v;
		Interval iv;
		int num;		
		if ((num=from->NumKeys())!=NOT_KEYFRAMEABLE && num>0) {
			// CAL-09/11/02: check if the old controller is keyed at time 0
			bool keyAtZero = false;

			SuspendAnimate();
			AnimateOn();
			for (int i=0; i<num; i++) {
				TimeValue t = from->GetKeyTime(i);
				if (t == 0) keyAtZero = true;
				from->GetValue(t,&v,iv);
				SetValue(t,&v,TRUE,CTRL_ABSOLUTE);	
				}
			ResumeAnimate();
			// RB 2/10/99: A key at frame 0 may have been created
			if (num>0 && !keyAtZero) {
				posX->DeleteKeyAtTime(0);
				posY->DeleteKeyAtTime(0);
				posZ->DeleteKeyAtTime(0);
				}
		} else {
			from->GetValue(0,&v,ivalid);
			SetValue(0,&v,TRUE,CTRL_ABSOLUTE);
			}
		}
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}

// added 020819  --prs.
void IndePosition::SelectKeyByIndex(int i, BOOL sel)
	{
	int nk = NumKeys();				// sets up keyTimes table
	if (i < nk) {
		TimeValue tv = GetKeyTime(i);
		if (posX) {
			int xkey = posX->GetKeyIndex(tv);
			if (xkey >= 0)
				posX->SelectKeyByIndex(xkey, sel);
			}
		if (posY) {
			int ykey = posY->GetKeyIndex(tv);
			if (ykey >= 0)
				posY->SelectKeyByIndex(ykey, sel);
			}
		if (posZ) {
			int zkey = posZ->GetKeyIndex(tv);
			if (zkey >= 0)
				posZ->SelectKeyByIndex(zkey, sel);
			}
		}
	}

// added 020819  --prs.
BOOL IndePosition::IsKeySelected(int i)
	{
	if (i < keyTimes.Count()) {
		TimeValue tv = GetKeyTime(i);
		if (posX) {
			int xkey = posX->GetKeyIndex(tv);
			if (xkey >= 0 && posX->IsKeySelected(xkey))
				return TRUE;
			}
		if (posY) {
			int ykey = posY->GetKeyIndex(tv);
			if (ykey >= 0 && posY->IsKeySelected(ykey))
				return TRUE;
			}
		if (posZ) {
			int zkey = posZ->GetKeyIndex(tv);
			if (zkey >= 0 && posZ->IsKeySelected(zkey))
				return TRUE;
			}
		}
	return FALSE;
	}

void IndePosition::Update(TimeValue t)
	{
	if (!ivalid.InInterval(t)) {
		ivalid = FOREVER;		
		if (posX) posX->GetValue(t,&curval.x,ivalid);
		if (posY) posY->GetValue(t,&curval.y,ivalid);
		if (posZ) posZ->GetValue(t,&curval.z,ivalid);		
		}
	}

void IndePosition::SetValue(TimeValue t, void *val, int commit, GetSetMethod method)
	{
	Point3 *v = (Point3*)val;
	
	// RB 5/5/99: SetValue() calls NotifyDependents() which ultimately calls GetValue().
	// This isn't supposed to happen since reference makers are supposed to only invalidate
	// on a call to NotifyRefChanged(), not re-evaluate. However this is what happens
	// on around line 644 of stdShaders.cpp. 
	curval = *v;
	blockUpdate = TRUE;
	if (posX) posX->SetValue(t,&v->x,commit,method);
	if (posY) posY->SetValue(t,&v->y,commit,method);
	if (posZ) posZ->SetValue(t,&v->z,commit,method);
	blockUpdate = FALSE;

	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}

void IndePosition::GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{		
	Update(t);
	valid &= ivalid;			 
	if (method==CTRL_RELATIVE) {
  		Matrix3 *mat = (Matrix3*)val;		
		mat->PreTranslate(curval);
	} else {
		*((Point3*)val) = curval;
		}
	}

void IndePoint3::GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{		
	if (!blockUpdate) Update(t);
	valid &= ivalid;			 
	if (method==CTRL_RELATIVE) {
  		*((Point3*)val) += curval;
	} else {			    
		*((Point3*)val)  = curval;
		}
	}

void IndePosition::CommitValue(TimeValue t)
	{
	if (posX) posX->CommitValue(t);
	if (posY) posY->CommitValue(t);
	if (posZ) posZ->CommitValue(t);
	}

void IndePosition::RestoreValue(TimeValue t)
	{
	if (posX) posX->RestoreValue(t);
	if (posY) posY->RestoreValue(t);
	if (posZ) posZ->RestoreValue(t);
	}

RefTargetHandle IndePosition::GetReference(int i)
	{
	switch (i) {
		case IPOS_X_REF: return posX;
		case IPOS_Y_REF: return posY;
		case IPOS_Z_REF: return posZ;
		default: return NULL;
		}
	}

void IndePosition::SetReference(int i, RefTargetHandle rtarg)
	{
	switch (i) {
		case IPOS_X_REF: posX = (Control*)rtarg; break;
		case IPOS_Y_REF: posY = (Control*)rtarg; break;
		case IPOS_Z_REF: posZ = (Control*)rtarg; break;
		}
	}

Animatable* IndePosition::SubAnim(int i)
	{
	return GetReference(i);
	}

TSTR IndePosition::SubAnimName(int i)
	{
	if (ClassID()==IPOS_CONTROL_CLASS_ID) {
		switch (i) {
			case IPOS_X_REF: return GetString(IDS_RB_XPOSITION);
			case IPOS_Y_REF: return GetString(IDS_RB_YPOSITION);
			case IPOS_Z_REF: return GetString(IDS_RB_ZPOSITION);
			default: return _T("");
			}
	} else
	if (ClassID()==IPOINT3_CONTROL_CLASS_ID) {
		switch (i) {
			case IPOS_X_REF: return _T("X");
			case IPOS_Y_REF: return _T("Y");
			case IPOS_Z_REF: return _T("Z");
			default: return _T("");
			}
	} else {
		switch (i) {
			case IPOS_X_REF: return _T("R");
			case IPOS_Y_REF: return _T("G");
			case IPOS_Z_REF: return _T("B");
			default: return _T("");
			}
		}
	}

RefResult IndePosition::NotifyRefChanged(
		Interval iv, 
		RefTargetHandle hTarg, 
		PartID& partID, 
		RefMessage msg) 
	{
	switch (msg) {
		case REFMSG_CHANGE:
			ivalid.SetEmpty();
			break;
		case REFMSG_TARGET_DELETED:
			if (posX == hTarg) posX = NULL;
			if (posY == hTarg) posY = NULL;
			if (posZ == hTarg) posZ = NULL; 
			break;
		case REFMSG_GET_CONTROL_DIM: {
			ParamDimension **dim = (ParamDimension **)partID;
			assert(dim);
			*dim = stdWorldDim;
			}
		}
	return REF_SUCCEED;
	}

BOOL IndePosition::AssignController(Animatable *control,int subAnim)
	{	
	switch (subAnim) {
		case IPOS_X_REF:
			ReplaceReference(IPOS_X_REF,(RefTargetHandle)control);
			break;
		case IPOS_Y_REF:
			ReplaceReference(IPOS_Y_REF,(RefTargetHandle)control);
			break;
		case IPOS_Z_REF:
			ReplaceReference(IPOS_Z_REF,(RefTargetHandle)control);
			break;
		}

	// mjm 9.28.98
	// bugfix -- validity interval needs to be invalidated so curval
	// can be properly updated in next call to IndePosition::Update()
	ivalid.SetEmpty();

	NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE,TREE_VIEW_CLASS_ID,FALSE);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);	
	return TRUE;
	}

void IndePosition::AddNewKey(TimeValue t,DWORD flags)
	{
	if (posX) posX->AddNewKey(t,flags);
	if (posY) posY->AddNewKey(t,flags);
	if (posZ) posZ->AddNewKey(t,flags);
	}

int IndePosition::NumKeys()
	{
	int xnum = posX->NumKeys();
	int ynum = posY->NumKeys();
	int znum = posZ->NumKeys();
	keyTimes.ZeroCount();
	for (int i = 0, j = 0, k = 0;
		 i < xnum || j < ynum || k < znum; ) {
		TimeValue k1 = i < xnum ? posX->GetKeyTime(i) : TIME_PosInfinity;
		TimeValue k2 = j < ynum ? posY->GetKeyTime(j) : TIME_PosInfinity;
		TimeValue k3 = k < znum ? posZ->GetKeyTime(k) : TIME_PosInfinity;
		TimeValue kmin = k1 < k2 ? (k3 < k1 ? k3 : k1) : (k3 < k2 ? k3 : k2);
		keyTimes.Append(1, &kmin, 10);
		if (k1 == kmin) ++i;
		if (k2 == kmin) ++j;
		if (k3 == kmin) ++k;
	}
	return keyTimes.Count();
	}

TimeValue IndePosition::GetKeyTime(int index)
	{
	return keyTimes[index];
	}

int IndePosition::GetKeyTimes(
	Tab<TimeValue>& times, Interval range, DWORD flags)
{
	Tab<TimeValue> xtimes;
	Tab<TimeValue> ytimes;
	Tab<TimeValue> ztimes;
	xkSkip = posX != NULL ? posX->GetKeyTimes(xtimes, range, flags) : 0;
	ykSkip = posY != NULL ? posY->GetKeyTimes(ytimes, range, flags) : 0;
	zkSkip = posZ != NULL ? posZ->GetKeyTimes(ztimes, range, flags) : 0;
	size_t i = 0, xc = xtimes.Count();
	size_t j = 0, yc = ytimes.Count();
	size_t k = 0, zc = ztimes.Count();
	while (i < xc || j < yc || k < zc) {
		TimeValue k1 = i < xc ? xtimes[i] : TIME_PosInfinity;
		TimeValue k2 = j < yc ? ytimes[j] : TIME_PosInfinity;
		TimeValue k3 = k < zc ? ztimes[k] : TIME_PosInfinity;
		TimeValue kmin = k1 < k2 ? (k3 < k1 ? k3 : k1) : (k3 < k2 ? k3 : k2);
		times.Append(1, &kmin, 10);
		if (k1 == kmin) ++i;
		if (k2 == kmin) ++j;
		if (k3 == kmin) ++k;
	}
	xKeys.SetSize(times.Count());
	yKeys.SetSize(times.Count());
	zKeys.SetSize(times.Count());
	xKeys.ClearAll();
	yKeys.ClearAll();
	zKeys.ClearAll();
	i = j = k = 0;
	for (size_t n = 0; n < times.Count(); ++n) {
		if (xtimes.Count() > 0 && xtimes[i] == times[n]) { //RK: 06/18/02, 430812, check if count > 0
			xKeys.Set(n);
			++i;
		}
		if (ytimes.Count() > 0 && ytimes[j] == times[n]) { //RK: 06/18/02, 430812, check if count > 0
			yKeys.Set(n);
			++j;
		}
		if (ztimes.Count() > 0 && ztimes[k] == times[n]) { //RK: 06/18/02, 430812, check if count > 0
			zKeys.Set(n);
			++k;
		}
	}
	return 0;
}

int IndePosition::GetKeySelState(
	BitArray& sel, Interval range, DWORD flags)
{
	BitArray xsel;
	BitArray ysel;
	BitArray zsel;
	int c = xKeys.GetSize();
	xsel.SetSize(c);
	xsel.ClearAll();
	ysel.SetSize(c);
	ysel.ClearAll();
	zsel.SetSize(c);
	zsel.ClearAll();
	xkSkip = posX != NULL ? posX->GetKeySelState(xsel, range, flags) : 0;
	ykSkip = posY != NULL ? posY->GetKeySelState(ysel, range, flags) : 0;
	zkSkip = posZ != NULL ? posZ->GetKeySelState(zsel, range, flags) : 0;
	size_t i = 0;
	size_t j = 0;
	size_t k = 0;
	for (size_t n = 0; n < c; ++n) {
		BOOL selected = FALSE;
		if (xKeys[n]) {
			if (xsel[i]) selected = true;
			++i;
		}
		if (yKeys[n]) {
			if (ysel[j]) selected = true;
			++j;
		}
		if (zKeys[n]) {
			if (zsel[k]) selected = true;
			++k;
		}
		sel.Set(n, selected);
	}
	return 0;
}

void IndePosition::SelectKeys(TrackHitTab& sel, DWORD flags)
{
	if (posX) {
		TrackHitTab adjusted_sel = sel;
		int c = sel.Count();
		int i = 0;
		while (i < c) {
			unsigned j = adjusted_sel[i].hit;
			if (xKeys[j]) {
				unsigned k = 0;
				for (size_t n = 0; n < j; ++n)
					if (xKeys[n]) ++k;
				adjusted_sel[i].hit = xkSkip + k;
				++i;
			} else {
				adjusted_sel.Delete(i, 1);
				--c;
			}
		}
		posX->SelectKeys(adjusted_sel, flags);
	}
	if (posY) {
		TrackHitTab adjusted_sel = sel;
		int c = adjusted_sel.Count();
		int i = 0;
		while (i < c) {
			unsigned j = adjusted_sel[i].hit;
			if (yKeys[j]) {
				unsigned k = 0;
				for (size_t n = 0; n < j; ++n)
					if (yKeys[n]) ++k;
				adjusted_sel[i].hit = ykSkip + k;
				++i;
			} else {
				adjusted_sel.Delete(i, 1);
				--c;
			}
		}
		posY->SelectKeys(adjusted_sel, flags);
	}
	if (posZ) {
		TrackHitTab adjusted_sel = sel;
		int c = adjusted_sel.Count();
		int i = 0;
		while (i < c) {
			unsigned j = adjusted_sel[i].hit;
			if (zKeys[j]) {
				unsigned k = 0;
				for (size_t n = 0; n < j; ++n)
					if (zKeys[n]) ++k;
				adjusted_sel[i].hit = zkSkip + k;
				++i;
			} else {
				adjusted_sel.Delete(i, 1);
				--c;
			}
		}
		posZ->SelectKeys(adjusted_sel, flags);
	}
}

void IndePosition::CopyKeysFromTime(TimeValue src,TimeValue dst,DWORD flags)
	{
	if (posX) posX->CopyKeysFromTime(src,dst,flags);
	if (posY) posY->CopyKeysFromTime(src,dst,flags);
	if (posZ) posZ->CopyKeysFromTime(src,dst,flags);
	}

BOOL IndePosition::IsKeyAtTime(TimeValue t,DWORD flags)
	{
	if (posX && posX->IsKeyAtTime(t,flags)) return TRUE;
	if (posY && posY->IsKeyAtTime(t,flags)) return TRUE;
	if (posZ && posZ->IsKeyAtTime(t,flags)) return TRUE;
	return FALSE;
	}

void IndePosition::DeleteKeyAtTime(TimeValue t)
	{
	if (posX) posX->DeleteKeyAtTime(t);
	if (posY) posY->DeleteKeyAtTime(t);
	if (posZ) posZ->DeleteKeyAtTime(t);
	}

// added 020823  --prs.
void IndePosition::DeleteKeys(DWORD flags)
	{
	if (posX) posX->DeleteKeys(flags);
	if (posY) posY->DeleteKeys(flags);
	if (posZ) posZ->DeleteKeys(flags);
	}

// added 020813  --prs.
void IndePosition::DeleteKeyByIndex(int index)
	{
	int nk = NumKeys();	// sets up keyTimes table

	if (index < nk)
		DeleteKeyAtTime(keyTimes[index]);
	}

BOOL IndePosition::GetNextKeyTime(TimeValue t,DWORD flags,TimeValue &nt)
	{
	TimeValue at,tnear = 0;
	BOOL tnearInit = FALSE;
	
	if (posX && posX->GetNextKeyTime(t,flags,at)) {
		if (!tnearInit) {
			tnear = at;
			tnearInit = TRUE;
		} else 
		if (ABS(at-t) < ABS(tnear-t)) tnear = at;
		}

	if (posY && posY->GetNextKeyTime(t,flags,at)) {
		if (!tnearInit) {
			tnear = at;
			tnearInit = TRUE;
		} else 
		if (ABS(at-t) < ABS(tnear-t)) tnear = at;
		}

	if (posZ && posZ->GetNextKeyTime(t,flags,at)) {
		if (!tnearInit) {
			tnear = at;
			tnearInit = TRUE;
		} else 
		if (ABS(at-t) < ABS(tnear-t)) tnear = at;
		}
	
	if (tnearInit) {
		nt = tnear;
		return TRUE;
	} else {
		return FALSE;
		}
	}
		

void IndePosition::BeginEditParams(
		IObjParam *ip, ULONG flags,Animatable *prev)
	{	
	this->ip = ip;

	if (dlg) {
		dlg->BeginingEdit(this,ip,(IndePosition*)prev);
	} else {
		dlg = new IPosDlg(this,ip);
		}
	}

void IndePosition::EndEditParams(
		IObjParam *ip, ULONG flags,Animatable *next)
	{	
	IndePosition *cont=NULL;
	if (next && next->ClassID()==ClassID()) {
		cont = (IndePosition*)next;
		}

	if (dlg) {
		if (cont) {
			dlg->EndingEdit(cont);
		} else {
			delete dlg;
			dlg = NULL;
			}
		}
	}

int IndePosition::GetORT(int type)
	{
	if (posX) return posX->GetORT(type);  // AF -- you got to pick something...
	return 0;
	}

void IndePosition::SetORT(int ort,int type)
	{
	if (posX) posX->SetORT(ort, type);
	if (posY) posY->SetORT(ort, type);
	if (posZ) posZ->SetORT(ort, type);
	}

void IndePosition::EnableORTs(BOOL enable)
	{
	if (posX) posX->EnableORTs(enable);
	if (posY) posY->EnableORTs(enable);
	if (posZ) posZ->EnableORTs(enable);
	}

//================================================================================

IndePoint4::IndePoint4(const IndePoint4 &ctrl)
: xkSkip(0)
, ykSkip(0)
, zkSkip(0)
, wkSkip(0)
{
	blockUpdate = FALSE;

	posX = NULL;
	posY = NULL;
	posZ = NULL;
	posW = NULL;

	if (ctrl.posX) {
		ReplaceReference(IPOS_X_REF,ctrl.posX);
	} else {
		ReplaceReference(IPOS_X_REF,NewDefaultFloatController());
	}
	if (ctrl.posY) {
		ReplaceReference(IPOS_Y_REF,ctrl.posY);
	} else {
		ReplaceReference(IPOS_Y_REF,NewDefaultFloatController());
	}
	if (ctrl.posZ) {
		ReplaceReference(IPOS_Z_REF,ctrl.posZ);
	} else {
		ReplaceReference(IPOS_Z_REF,NewDefaultFloatController());
	}
	if (ctrl.posW) {
		ReplaceReference(IPOS_W_REF,ctrl.posW);
	} else {
		ReplaceReference(IPOS_W_REF,NewDefaultFloatController());
	}

	curval = ctrl.curval;
	ivalid = ctrl.ivalid;
}

IndePoint4::IndePoint4(BOOL loading) 
: xkSkip(0)
, ykSkip(0)
, zkSkip(0)
, wkSkip(0)
{
	blockUpdate = FALSE;

	posX = NULL;
	posY = NULL;
	posZ = NULL;
	posW = NULL;
	if (!loading) {
		ReplaceReference(IPOS_X_REF,NewDefaultFloatController());
		ReplaceReference(IPOS_Y_REF,NewDefaultFloatController());
		ReplaceReference(IPOS_Z_REF,NewDefaultFloatController());
		ReplaceReference(IPOS_W_REF,NewDefaultFloatController());
		ivalid = FOREVER;
		curval = Point4(0,0,0,0);
	} else {
		ivalid.SetEmpty();
	}	
}

IndeAColor::IndeAColor(BOOL loading) 
{
	xkSkip = ykSkip = zkSkip = wkSkip = 0;
	blockUpdate = FALSE;

	posX = NULL;
	posY = NULL;
	posZ = NULL;
	posW = NULL;
	if (!loading) {
		ReplaceReference(IPOS_X_REF,NewDefaultFloatController());
		ReplaceReference(IPOS_Y_REF,NewDefaultFloatController());
		ReplaceReference(IPOS_Z_REF,NewDefaultFloatController());
		ReplaceReference(IPOS_W_REF,NewDefaultFloatController());
		float v = 1.0f;
		posW->SetValue(0,&v,TRUE,CTRL_ABSOLUTE);
		ivalid = FOREVER;
		curval = Point4(0.f,0.f,0.f,1.f);
	} else {
		ivalid.SetEmpty();
	}	
}

RefTargetHandle IndePoint4::Clone(RemapDir& remap) 
{
	IndePoint4 *pos = new IndePoint4(TRUE);	
	pos->ReplaceReference(IPOS_X_REF, remap.CloneRef(posX));
	pos->ReplaceReference(IPOS_Y_REF, remap.CloneRef(posY));
	pos->ReplaceReference(IPOS_Z_REF, remap.CloneRef(posZ));
	pos->ReplaceReference(IPOS_W_REF, remap.CloneRef(posW));
	BaseClone(this, pos, remap);
	return pos;
}

RefTargetHandle IndeAColor::Clone(RemapDir& remap) 
{
	IndeColor *pos = new IndeColor(TRUE);	
	pos->ReplaceReference(IPOS_X_REF, remap.CloneRef(posX));
	pos->ReplaceReference(IPOS_Y_REF, remap.CloneRef(posY));
	pos->ReplaceReference(IPOS_Z_REF, remap.CloneRef(posZ));
	pos->ReplaceReference(IPOS_W_REF, remap.CloneRef(posW));
	BaseClone(this, pos, remap);
	return pos;
}

BOOL IndePoint4::CreateLockKey(TimeValue t, int which)
{
	if (posX) posX->CreateLockKey(t,which);
	if (posY) posY->CreateLockKey(t,which);
	if (posZ) posZ->CreateLockKey(t,which);
	if (posW) posW->CreateLockKey(t,which);
	return TRUE;
}

IndePoint4::~IndePoint4()
{
	DeleteAllRefsFromMe();
}

void IndePoint4::Copy(Control *from)
{
	if (from->ClassID()==ClassID()) {
		IndePoint4 *ctrl = (IndePoint4*)from;
		ReplaceReference(IPOS_X_REF,ctrl->posX);
		ReplaceReference(IPOS_Y_REF,ctrl->posY);
		ReplaceReference(IPOS_Z_REF,ctrl->posZ);
		ReplaceReference(IPOS_W_REF,ctrl->posW);
		curval = ctrl->curval;
		ivalid = ctrl->ivalid;
	} else {		
		Point4 v;
		Interval iv;
		int num;		
		if ((num=from->NumKeys())!=NOT_KEYFRAMEABLE && num>0) {
			// CAL-09/11/02: check if the old controller is keyed at time 0
			bool keyAtZero = false;

			SuspendAnimate();
			AnimateOn();
			for (int i=0; i<num; i++) {
				TimeValue t = from->GetKeyTime(i);
				if (t == 0) keyAtZero = true;
				from->GetValue(t,&v,iv);
				SetValue(t,&v,TRUE,CTRL_ABSOLUTE);	
			}
			ResumeAnimate();
			// RB 2/10/99: A key at frame 0 may have been created
			if (num>0 && !keyAtZero) {
				posX->DeleteKeyAtTime(0);
				posY->DeleteKeyAtTime(0);
				posZ->DeleteKeyAtTime(0);
				posW->DeleteKeyAtTime(0);
			}
		} else {
			from->GetValue(0,&v,ivalid);
			SetValue(0,&v,TRUE,CTRL_ABSOLUTE);
		}
	}
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
}

// added 020819  --prs.
void IndePoint4::SelectKeyByIndex(int i, BOOL sel)
{
	int nk = NumKeys();				// sets up keyTimes table
	if (i < nk) {
		TimeValue tv = GetKeyTime(i);
		if (posX) {
			int xkey = posX->GetKeyIndex(tv);
			if (xkey >= 0)
				posX->SelectKeyByIndex(xkey, sel);
		}
		if (posY) {
			int ykey = posY->GetKeyIndex(tv);
			if (ykey >= 0)
				posY->SelectKeyByIndex(ykey, sel);
		}
		if (posZ) {
			int zkey = posZ->GetKeyIndex(tv);
			if (zkey >= 0)
				posZ->SelectKeyByIndex(zkey, sel);
		}
		if (posW) {
			int wkey = posW->GetKeyIndex(tv);
			if (wkey >= 0)
				posW->SelectKeyByIndex(wkey, sel);
		}
	}
}

// added 020819  --prs.
BOOL IndePoint4::IsKeySelected(int i)
{
	if (i < keyTimes.Count()) {
		TimeValue tv = GetKeyTime(i);
		if (posX) {
			int xkey = posX->GetKeyIndex(tv);
			if (xkey >= 0 && posX->IsKeySelected(xkey))
				return TRUE;
		}
		if (posY) {
			int ykey = posY->GetKeyIndex(tv);
			if (ykey >= 0 && posY->IsKeySelected(ykey))
				return TRUE;
		}
		if (posZ) {
			int zkey = posZ->GetKeyIndex(tv);
			if (zkey >= 0 && posZ->IsKeySelected(zkey))
				return TRUE;
		}
		if (posW) {
			int wkey = posW->GetKeyIndex(tv);
			if (wkey >= 0 && posW->IsKeySelected(wkey))
				return TRUE;
		}
	}
	return FALSE;
}

void IndePoint4::Update(TimeValue t)
{
	if (!ivalid.InInterval(t)) {
		ivalid = FOREVER;		
		if (posX) posX->GetValue(t,&curval.x,ivalid);
		if (posY) posY->GetValue(t,&curval.y,ivalid);
		if (posZ) posZ->GetValue(t,&curval.z,ivalid);		
		if (posW) posW->GetValue(t,&curval.w,ivalid);		
	}
}

void IndePoint4::SetValue(TimeValue t, void *val, int commit, GetSetMethod method)
{
	Point4 *v = (Point4*)val;

	// RB 5/5/99: SetValue() calls NotifyDependents() which ultimately calls GetValue().
	// This isn't supposed to happen since reference makers are supposed to only invalidate
	// on a call to NotifyRefChanged(), not re-evaluate. However this is what happens
	// on around line 644 of stdShaders.cpp. 
	curval = *v;
	blockUpdate = TRUE;
	if (posX) posX->SetValue(t,&v->x,commit,method);
	if (posY) posY->SetValue(t,&v->y,commit,method);
	if (posZ) posZ->SetValue(t,&v->z,commit,method);
	if (posW) posW->SetValue(t,&v->w,commit,method);
	blockUpdate = FALSE;

	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
}

void IndePoint4::GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method)
{		
	if (!blockUpdate) Update(t);
	valid &= ivalid;			 
	if (method==CTRL_RELATIVE) {
		*((Point4*)val) += curval;
	} else {			    
		*((Point4*)val)  = curval;
	}
}

void IndePoint4::CommitValue(TimeValue t)
{
	if (posX) posX->CommitValue(t);
	if (posY) posY->CommitValue(t);
	if (posZ) posZ->CommitValue(t);
	if (posW) posW->CommitValue(t);
}

void IndePoint4::RestoreValue(TimeValue t)
{
	if (posX) posX->RestoreValue(t);
	if (posY) posY->RestoreValue(t);
	if (posZ) posZ->RestoreValue(t);
	if (posW) posW->RestoreValue(t);
}

RefTargetHandle IndePoint4::GetReference(int i)
{
	switch (i) {
		case IPOS_X_REF: return posX;
		case IPOS_Y_REF: return posY;
		case IPOS_Z_REF: return posZ;
		case IPOS_W_REF: return posW;
		default: return NULL;
	}
}

void IndePoint4::SetReference(int i, RefTargetHandle rtarg)
{
	switch (i) {
		case IPOS_X_REF: posX = (Control*)rtarg; break;
		case IPOS_Y_REF: posY = (Control*)rtarg; break;
		case IPOS_Z_REF: posZ = (Control*)rtarg; break;
		case IPOS_W_REF: posW = (Control*)rtarg; break;
	}
}

Animatable* IndePoint4::SubAnim(int i)
{
	return GetReference(i);
}

TSTR IndePoint4::SubAnimName(int i)
{
	if (ClassID()==IPOINT4_CONTROL_CLASS_ID) {
		switch (i) {
			case IPOS_X_REF: return _T("X");
			case IPOS_Y_REF: return _T("Y");
			case IPOS_Z_REF: return _T("Z");
			case IPOS_W_REF: return _T("W");
			default: return _T("");
		}
	} else {
		switch (i) {
			case IPOS_X_REF: return _T("R");
			case IPOS_Y_REF: return _T("G");
			case IPOS_Z_REF: return _T("B");
			case IPOS_W_REF: return _T("A");
			default: return _T("");
		}
	}
}

RefResult IndePoint4::NotifyRefChanged(
	Interval iv, 
	RefTargetHandle hTarg, 
	PartID& partID, 
	RefMessage msg) 
{
	switch (msg) {
		case REFMSG_CHANGE:
			ivalid.SetEmpty();
			break;
		case REFMSG_TARGET_DELETED:
			if (posX == hTarg) posX = NULL;
			if (posY == hTarg) posY = NULL;
			if (posZ == hTarg) posZ = NULL; 
			if (posW == hTarg) posW = NULL; 
			break;
		case REFMSG_GET_CONTROL_DIM: {
			ParamDimension **dim = (ParamDimension **)partID;
			assert(dim);
			*dim = stdWorldDim;
			 }
	}
	return REF_SUCCEED;
}

BOOL IndePoint4::AssignController(Animatable *control,int subAnim)
{	
	switch (subAnim) {
		case IPOS_X_REF:
			ReplaceReference(IPOS_X_REF,(RefTargetHandle)control);
			break;
		case IPOS_Y_REF:
			ReplaceReference(IPOS_Y_REF,(RefTargetHandle)control);
			break;
		case IPOS_Z_REF:
			ReplaceReference(IPOS_Z_REF,(RefTargetHandle)control);
			break;
		case IPOS_W_REF:
			ReplaceReference(IPOS_W_REF,(RefTargetHandle)control);
			break;
	}

	// mjm 9.28.98
	// bugfix -- validity interval needs to be invalidated so curval
	// can be properly updated in next call to IndePoint4::Update()
	ivalid.SetEmpty();

	NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE,TREE_VIEW_CLASS_ID,FALSE);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);	
	return TRUE;
}

void IndePoint4::AddNewKey(TimeValue t,DWORD flags)
{
	if (posX) posX->AddNewKey(t,flags);
	if (posY) posY->AddNewKey(t,flags);
	if (posZ) posZ->AddNewKey(t,flags);
	if (posW) posW->AddNewKey(t,flags);
}

int IndePoint4::NumKeys()
{
	int xnum = posX->NumKeys();
	int ynum = posY->NumKeys();
	int znum = posZ->NumKeys();
	int wnum = posW->NumKeys();
	keyTimes.ZeroCount();
	for (int i = 0, j = 0, k = 0, m = 0;
		i < xnum || j < ynum || k < znum || m < wnum; ) {
			TimeValue k1 = i < xnum ? posX->GetKeyTime(i) : TIME_PosInfinity;
			TimeValue k2 = j < ynum ? posY->GetKeyTime(j) : TIME_PosInfinity;
			TimeValue k3 = k < znum ? posZ->GetKeyTime(k) : TIME_PosInfinity;
			TimeValue k4 = m < wnum ? posW->GetKeyTime(k) : TIME_PosInfinity;
			TimeValue kmin12 = k1 < k2 ? k1 : k2;
			TimeValue kmin34 = k3 < k4 ? k3 : k4;
			TimeValue kmin = kmin12 < kmin34 ? kmin12 : kmin34;
			keyTimes.Append(1, &kmin, 10);
			if (k1 == kmin) ++i;
			if (k2 == kmin) ++j;
			if (k3 == kmin) ++k;
			if (k4 == kmin) ++m;
		}
		return keyTimes.Count();
}

TimeValue IndePoint4::GetKeyTime(int index)
{
	return keyTimes[index];
}

int IndePoint4::GetKeyTimes(
	Tab<TimeValue>& times, Interval range, DWORD flags)
{
	Tab<TimeValue> xtimes;
	Tab<TimeValue> ytimes;
	Tab<TimeValue> ztimes;
	Tab<TimeValue> wtimes;
	xkSkip = posX != NULL ? posX->GetKeyTimes(xtimes, range, flags) : 0;
	ykSkip = posY != NULL ? posY->GetKeyTimes(ytimes, range, flags) : 0;
	zkSkip = posZ != NULL ? posZ->GetKeyTimes(ztimes, range, flags) : 0;
	wkSkip = posW != NULL ? posW->GetKeyTimes(wtimes, range, flags) : 0;
	size_t i = 0, xc = xtimes.Count();
	size_t j = 0, yc = ytimes.Count();
	size_t k = 0, zc = ztimes.Count();
	size_t m = 0, wc = wtimes.Count();
	while (i < xc || j < yc || k < zc || m < wc) {
		TimeValue k1 = i < xc ? xtimes[i] : TIME_PosInfinity;
		TimeValue k2 = j < yc ? ytimes[j] : TIME_PosInfinity;
		TimeValue k3 = k < zc ? ztimes[k] : TIME_PosInfinity;
		TimeValue k4 = m < wc ? wtimes[k] : TIME_PosInfinity;
		TimeValue kmin12 = k1 < k2 ? k1 : k2;
		TimeValue kmin34 = k3 < k4 ? k3 : k4;
		TimeValue kmin = kmin12 < kmin34 ? kmin12 : kmin34;
		times.Append(1, &kmin, 10);
		if (k1 == kmin) ++i;
		if (k2 == kmin) ++j;
		if (k3 == kmin) ++k;
		if (k4 == kmin) ++m;
	}
	xKeys.SetSize(times.Count());
	yKeys.SetSize(times.Count());
	zKeys.SetSize(times.Count());
	wKeys.SetSize(times.Count());
	xKeys.ClearAll();
	yKeys.ClearAll();
	zKeys.ClearAll();
	wKeys.ClearAll();
	i = j = k = m = 0;
	for (size_t n = 0; n < times.Count(); ++n) {
		if (xtimes.Count() > 0 && xtimes[i] == times[n]) { //RK: 06/18/02, 430812, check if count > 0
			xKeys.Set(n);
			++i;
		}
		if (ytimes.Count() > 0 && ytimes[j] == times[n]) { //RK: 06/18/02, 430812, check if count > 0
			yKeys.Set(n);
			++j;
		}
		if (ztimes.Count() > 0 && ztimes[k] == times[n]) { //RK: 06/18/02, 430812, check if count > 0
			zKeys.Set(n);
			++k;
		}
		if (wtimes.Count() > 0 && wtimes[m] == times[n]) { //RK: 06/18/02, 430812, check if count > 0
			wKeys.Set(n);
			++m;
		}
	}
	return 0;
}

int IndePoint4::GetKeySelState(
	BitArray& sel, Interval range, DWORD flags)
{
	BitArray xsel;
	BitArray ysel;
	BitArray zsel;
	BitArray wsel;
	int c = xKeys.GetSize();
	xsel.SetSize(c);
	xsel.ClearAll();
	ysel.SetSize(c);
	ysel.ClearAll();
	zsel.SetSize(c);
	zsel.ClearAll();
	wsel.SetSize(c);
	wsel.ClearAll();
	xkSkip = posX != NULL ? posX->GetKeySelState(xsel, range, flags) : 0;
	ykSkip = posY != NULL ? posY->GetKeySelState(ysel, range, flags) : 0;
	zkSkip = posZ != NULL ? posZ->GetKeySelState(zsel, range, flags) : 0;
	wkSkip = posW != NULL ? posW->GetKeySelState(wsel, range, flags) : 0;
	size_t i = 0;
	size_t j = 0;
	size_t k = 0;
	size_t m = 0;
	for (size_t n = 0; n < c; ++n) {
		BOOL selected = FALSE;
		if (xKeys[n]) {
			if (xsel[i]) selected = true;
			++i;
		}
		if (yKeys[n]) {
			if (ysel[j]) selected = true;
			++j;
		}
		if (zKeys[n]) {
			if (zsel[k]) selected = true;
			++k;
		}
		if (wKeys[n]) {
			if (wsel[m]) selected = true;
			++m;
		}
		sel.Set(n, selected);
	}
	return 0;
}

void IndePoint4::SelectKeys(TrackHitTab& sel, DWORD flags)
{
	if (posX) {
		TrackHitTab adjusted_sel = sel;
		int c = sel.Count();
		int i = 0;
		while (i < c) {
			unsigned j = adjusted_sel[i].hit;
			if (xKeys[j]) {
				unsigned k = 0;
				for (size_t n = 0; n < j; ++n)
					if (xKeys[n]) ++k;
				adjusted_sel[i].hit = xkSkip + k;
				++i;
			} else {
				adjusted_sel.Delete(i, 1);
				--c;
			}
		}
		posX->SelectKeys(adjusted_sel, flags);
	}
	if (posY) {
		TrackHitTab adjusted_sel = sel;
		int c = adjusted_sel.Count();
		int i = 0;
		while (i < c) {
			unsigned j = adjusted_sel[i].hit;
			if (yKeys[j]) {
				unsigned k = 0;
				for (size_t n = 0; n < j; ++n)
					if (yKeys[n]) ++k;
				adjusted_sel[i].hit = ykSkip + k;
				++i;
			} else {
				adjusted_sel.Delete(i, 1);
				--c;
			}
		}
		posY->SelectKeys(adjusted_sel, flags);
	}
	if (posZ) {
		TrackHitTab adjusted_sel = sel;
		int c = adjusted_sel.Count();
		int i = 0;
		while (i < c) {
			unsigned j = adjusted_sel[i].hit;
			if (zKeys[j]) {
				unsigned k = 0;
				for (size_t n = 0; n < j; ++n)
					if (zKeys[n]) ++k;
				adjusted_sel[i].hit = zkSkip + k;
				++i;
			} else {
				adjusted_sel.Delete(i, 1);
				--c;
			}
		}
		posZ->SelectKeys(adjusted_sel, flags);
	}
	if (posW) {
		TrackHitTab adjusted_sel = sel;
		int c = adjusted_sel.Count();
		int i = 0;
		while (i < c) {
			unsigned j = adjusted_sel[i].hit;
			if (wKeys[j]) {
				unsigned k = 0;
				for (size_t n = 0; n < j; ++n)
					if (wKeys[n]) ++k;
				adjusted_sel[i].hit = wkSkip + k;
				++i;
			} else {
				adjusted_sel.Delete(i, 1);
				--c;
			}
		}
		posW->SelectKeys(adjusted_sel, flags);
	}
}

void IndePoint4::CopyKeysFromTime(TimeValue src,TimeValue dst,DWORD flags)
{
	if (posX) posX->CopyKeysFromTime(src,dst,flags);
	if (posY) posY->CopyKeysFromTime(src,dst,flags);
	if (posZ) posZ->CopyKeysFromTime(src,dst,flags);
	if (posW) posW->CopyKeysFromTime(src,dst,flags);
}

BOOL IndePoint4::IsKeyAtTime(TimeValue t,DWORD flags)
{
	if (posX && posX->IsKeyAtTime(t,flags)) return TRUE;
	if (posY && posY->IsKeyAtTime(t,flags)) return TRUE;
	if (posZ && posZ->IsKeyAtTime(t,flags)) return TRUE;
	if (posW && posW->IsKeyAtTime(t,flags)) return TRUE;
	return FALSE;
}

void IndePoint4::DeleteKeyAtTime(TimeValue t)
{
	if (posX) posX->DeleteKeyAtTime(t);
	if (posY) posY->DeleteKeyAtTime(t);
	if (posZ) posZ->DeleteKeyAtTime(t);
	if (posW) posW->DeleteKeyAtTime(t);
}

// added 020823  --prs.
void IndePoint4::DeleteKeys(DWORD flags)
{
	if (posX) posX->DeleteKeys(flags);
	if (posY) posY->DeleteKeys(flags);
	if (posZ) posZ->DeleteKeys(flags);
	if (posW) posW->DeleteKeys(flags);
}

// added 020813  --prs.
void IndePoint4::DeleteKeyByIndex(int index)
{
	int nk = NumKeys();	// sets up keyTimes table

	if (index < nk)
		DeleteKeyAtTime(keyTimes[index]);
}

BOOL IndePoint4::GetNextKeyTime(TimeValue t,DWORD flags,TimeValue &nt)
{
	TimeValue at,tnear = 0;
	BOOL tnearInit = FALSE;

	if (posX && posX->GetNextKeyTime(t,flags,at)) {
		if (!tnearInit) {
			tnear = at;
			tnearInit = TRUE;
		} else 
			if (ABS(at-t) < ABS(tnear-t)) tnear = at;
	}

	if (posY && posY->GetNextKeyTime(t,flags,at)) {
		if (!tnearInit) {
			tnear = at;
			tnearInit = TRUE;
		} else 
			if (ABS(at-t) < ABS(tnear-t)) tnear = at;
	}

	if (posZ && posZ->GetNextKeyTime(t,flags,at)) {
		if (!tnearInit) {
			tnear = at;
			tnearInit = TRUE;
		} else 
			if (ABS(at-t) < ABS(tnear-t)) tnear = at;
	}

	if (posW && posW->GetNextKeyTime(t,flags,at)) {
		if (!tnearInit) {
			tnear = at;
			tnearInit = TRUE;
		} else 
			if (ABS(at-t) < ABS(tnear-t)) tnear = at;
	}

	if (tnearInit) {
		nt = tnear;
		return TRUE;
	} else {
		return FALSE;
	}
}


int IndePoint4::GetORT(int type)
{
	if (posX) return posX->GetORT(type);  // AF -- you got to pick something...
	return 0;
}

void IndePoint4::SetORT(int ort,int type)
{
	if (posX) posX->SetORT(ort, type);
	if (posY) posY->SetORT(ort, type);
	if (posZ) posZ->SetORT(ort, type);
	if (posW) posW->SetORT(ort, type);
}

void IndePoint4::EnableORTs(BOOL enable)
{
	if (posX) posX->EnableORTs(enable);
	if (posY) posY->EnableORTs(enable);
	if (posZ) posZ->EnableORTs(enable);
	if (posW) posW->EnableORTs(enable);
}

