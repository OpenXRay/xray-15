/**********************************************************************
 *<
	FILE: joydev.cpp

	DESCRIPTION: Joystick device for motion capture system

	CREATED BY: Rolf Berteig

	HISTORY: November 8, 1996

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#include "motion.h"
#include "mcapdev.h"
//#include "mmanager.h"

static MCInputDevice *GetJoyDevice();

#define JOY_DEVICE_CLASS_ID	0xf52b57d1

#define SPEED_UI_FACT 1.0f

class JoyDeviceBinding : public MCDeviceBinding {
	public:
		int which, invert, type, btype;	 // For joystick, type=1 is accum. mode. For buttons, type=0 is inc/dec, type=1 is inc, and type=2 is absolute mode
		float scale, speed, curVal;				
		IMCControl *cont; // Ref #0
		int dir, comp;

		JoyDeviceBinding();
		MCInputDevice *GetDevice() {return GetJoyDevice();}
		TSTR BindingName();
		float Eval(TimeValue t);
		void Accumulate(TimeValue t);
		void DeleteThis() {delete this;}
		Class_ID ClassID() {return Class_ID(JOY_DEVICE_CLASS_ID,0);}
		RefTargetHandle Clone(RemapDir& remap);

		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i) {return cont;}
		void SetReference(int i, RefTargetHandle rtarg) {cont = (IMCControl*)rtarg;}

		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		void AddRollup(IMCParamDlg *dlg);
		void UpdateRollup(IRollupWindow *iRoll);
		void BeginActivate(BOOL reset);
		void EndActivate();
		void SetStates(HWND hWnd);
		void UpdateContName(HWND hWnd);
		void DoPickCont(HWND hWnd);
		BOOL Accumulating();
	};


#define FILTER_SIZE		100

class JoyDevice : public MCInputDevice {
	public:
		BOOL active, error;
		DWORD xbase, ybase, zbase, tbase;
		DWORD xsamp[FILTER_SIZE];
		DWORD ysamp[FILTER_SIZE];
		DWORD zsamp[FILTER_SIZE];
		DWORD tsamp[FILTER_SIZE];

		DWORD filterPos, count;
		CRITICAL_SECTION csect;

		JoyDevice();
		~JoyDevice();

		TSTR DeviceName() {return GetString(IDS_RB_JOYDEVICE);}
		MCDeviceBinding *CreateBinding() {return new JoyDeviceBinding;}

		void Cycle(UINT tick);
		void ZeroJoy();
		float DX();
		float DY();
		float DZ();
		float DT();
		float POVH();
		float POVV();
		float But1();
		float But2();
		float But3();
		float But4();
		DWORD Filter(DWORD *f);

		DWORD ReadButtons();
		DWORD ReadPOV();
	};

//--- Class Descriptor -----------------------------------------------

// This is only here to support old files. Before the motion capture
// device interface was made plug-able the general REF_TARGET_CLASS_ID was used.
class JoyDeviceClassDescOld:public ClassDesc {
	public:
	int 			IsPublic() {return 0;}
	void *			Create(BOOL loading) {return new JoyDeviceBinding;}
	const TCHAR *	ClassName() {return GetString(IDS_RB_JOYDEVICE);}
	SClass_ID		SuperClassID() {return REF_TARGET_CLASS_ID;}
	Class_ID		ClassID() {return Class_ID(JOY_DEVICE_CLASS_ID,0);}
	const TCHAR* 	Category() {return _T("");}
	};
static JoyDeviceClassDescOld joyCDOld;
ClassDesc* GetJoyDeviceClassDescDescOld() {return &joyCDOld;}

class JoyDeviceClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 0;}
	void *			Create(BOOL loading) {return new JoyDeviceBinding;}
	const TCHAR *	ClassName() {return GetString(IDS_RB_JOYDEVICE);}
	SClass_ID		SuperClassID() {return MOT_CAP_DEVBINDING_CLASS_ID;}
	Class_ID		ClassID() {return Class_ID(JOY_DEVICE_CLASS_ID,0);}
	const TCHAR* 	Category() {return _T("");}
	};
static JoyDeviceClassDesc joyCD;
ClassDesc* GetJoyDeviceClassDescDesc() {return &joyCD;}

class TheJoyDeviceClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading) {return GetJoyDevice();}
	const TCHAR *	ClassName() {return GetString(IDS_RB_JOYDEVICE);}
	SClass_ID		SuperClassID() {return MOT_CAP_DEV_CLASS_ID;}
	Class_ID		ClassID() {return Class_ID(JOY_DEVICE_CLASS_ID,1);}
	const TCHAR* 	Category() {return _T("");}
	};
static TheJoyDeviceClassDesc theJoyCD;
ClassDesc* GetTheJoyDeviceClassDescDesc() {return &theJoyCD;}



//--- Joy device binding ---------------------------------------------------

static INT_PTR CALLBACK JoyDeviceDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static JoyDevice theJoyDevice;
static MCInputDevice *GetJoyDevice() {return &theJoyDevice;}

JoyDeviceBinding::JoyDeviceBinding() 
	{
	which  = IDC_JOY_X; 
	scale  = 1.0f; 
	speed  = 10.0f * SPEED_UI_FACT;
	curVal = 0.0f;
	invert = FALSE;
	type   = 0;
	btype  = 0;
	cont   = NULL;
	dir    = 2;
	comp   = 0;
	}

TSTR JoyDeviceBinding::BindingName() 
	{
	return GetString(which);
	}

BOOL JoyDeviceBinding::Accumulating()
	{
	switch (which) {
		case IDC_JOY_X:
		case IDC_JOY_Y:
		case IDC_JOY_Z:
			if (!type) return FALSE;
			break;
		case IDC_JOY_BUT1:
		case IDC_JOY_BUT2:
		case IDC_JOY_BUT3:
		case IDC_JOY_BUT4:
			if (btype==2) return FALSE;
			break;
		}
	return TRUE;
	}

void JoyDeviceBinding::Accumulate(TimeValue t)
	{
	float fact = 1.0f, bval;
	
	if (!Accumulating()) return;

	if (cont) {
		Matrix3 tm(1);
		cont->GetValueLive(t,&tm,CTRL_RELATIVE);
		fact = tm.GetRow(dir)[comp];
		}

	switch (which) {
		case IDC_JOY_X:
			curVal += theJoyDevice.DX() * 0.05f * fact * scale * (invert?-1.0f:1.0f);			
			break;
		case IDC_JOY_Y:
			curVal += theJoyDevice.DY() * 0.05f * fact * scale * (invert?-1.0f:1.0f);			
			break;
		case IDC_JOY_Z:
			curVal += theJoyDevice.DZ() * 0.05f * fact * scale * (invert?-1.0f:1.0f);			
			break;
		case IDC_JOY_THROTTLE:
			curVal += theJoyDevice.DT() * 0.05f * fact * scale * (invert?-1.0f:1.0f);			
			break;

		case IDC_JOY_POVH:
			curVal += speed * fact * theJoyDevice.POVH();
			break;
		case IDC_JOY_POVV:
			curVal += speed * fact * theJoyDevice.POVV();
			break;

		case IDC_JOY_BUT1:
			bval = theJoyDevice.But1();
			switch (btype) {
				case 0: curVal += speed * fact * bval; break;
				case 1: if (bval>0) curVal += speed * fact * bval; break;
				}
			if (speed>0.0f && curVal<0.0f) curVal = 0.0f;
			if (speed<0.0f && curVal>0.0f) curVal = 0.0f;
			break;
		case IDC_JOY_BUT2:
			bval = theJoyDevice.But2();
			switch (btype) {
				case 0: curVal += speed * fact * bval; break;
				case 1: if (bval>0) curVal += speed * fact * bval; break;				
				}			
			if (speed>0.0f && curVal<0.0f) curVal = 0.0f;
			if (speed<0.0f && curVal>0.0f) curVal = 0.0f;
			break;
		case IDC_JOY_BUT3:
			bval = theJoyDevice.But3();
			switch (btype) {
				case 0: curVal += speed * fact * bval; break;
				case 1: if (bval>0) curVal += speed * fact * bval; break;
				}			
			if (speed>0.0f && curVal<0.0f) curVal = 0.0f;
			if (speed<0.0f && curVal>0.0f) curVal = 0.0f;
			break;
		case IDC_JOY_BUT4:
			bval = theJoyDevice.But4();
			switch (btype) {
				case 0: curVal += speed * fact * bval; break;
				case 1: if (bval>0) curVal += speed * fact * bval; break;
				}			
			if (speed>0.0f && curVal<0.0f) curVal = 0.0f;
			if (speed<0.0f && curVal>0.0f) curVal = 0.0f;
			break;			
		}
	}

float JoyDeviceBinding::Eval(TimeValue t)
	{		
	float bval;
	switch (which) {
		case IDC_JOY_X:			
			if (!type) curVal = theJoyDevice.DX() * scale * (invert?-1.0f:1.0f);
			break;
		case IDC_JOY_Y:			
			if (!type) curVal = theJoyDevice.DY() * scale * (invert?-1.0f:1.0f);
			break;
		case IDC_JOY_Z:			
			if (!type) curVal = theJoyDevice.DZ() * scale * (invert?-1.0f:1.0f);
			break;
		case IDC_JOY_THROTTLE:			
			if (!type) curVal = theJoyDevice.DT() * scale * (invert?-1.0f:1.0f);
			break;

		case IDC_JOY_BUT1:						
			if (btype==2) {
				bval = theJoyDevice.But1();
				if (bval>0) curVal = speed; else curVal = 0.0f; break;
				}			
			break;
		case IDC_JOY_BUT2:			
			if (btype==2) {
				bval = theJoyDevice.But2();
				if (bval>0) curVal = speed; else curVal = 0.0f; break;
				}
			break;
		case IDC_JOY_BUT3:
			if (btype==2) {
				bval = theJoyDevice.But3();
				if (bval>0) curVal = speed; else curVal = 0.0f; break;
				}			
			break;
		case IDC_JOY_BUT4:			
			if (btype==2) {
				bval = theJoyDevice.But4();
				if (bval>0) curVal = speed; else curVal = 0.0f; break;
				}			
			break;			
		}
	
	return curVal;
	}

RefTargetHandle JoyDeviceBinding::Clone(RemapDir& remap)
	{
	JoyDeviceBinding *b = new JoyDeviceBinding;
	b->which  = which;
	b->invert = invert;
	b->scale  = scale;
	b->speed  = speed;
	b->type   = type;
	b->btype  = btype;
	b->dir    = dir;
	b->comp   = comp;
	if (cont) b->ReplaceReference(0,cont);
	BaseClone(this, b, remap);
	return b;
	}

void JoyDeviceBinding::BeginActivate(BOOL reset)
	{
	BOOL wasActive = theJoyDevice.active;
	theJoyDevice.active = TRUE;
	if (reset) {
		if (!wasActive) theJoyDevice.ZeroJoy();
		curVal = 0.0f;
		}
	}

void JoyDeviceBinding::EndActivate()
	{
	theJoyDevice.active = FALSE;
	}


void JoyDeviceBinding::AddRollup(IMCParamDlg *dlg)
	{	
	dlg->iRoll->AppendRollup(
			hInstance, 
			MAKEINTRESOURCE(IDD_MC_JOY), 
			JoyDeviceDlgProc, 
			GetString(IDS_RB_JOYDEVICE), 
			(LPARAM)dlg);
	}

void JoyDeviceBinding::UpdateRollup(IRollupWindow *iRoll)
	{
	if (iRoll->GetNumPanels()>1) {
		HWND hWnd = iRoll->GetPanelDlg(1);
		
		CheckDlgButton(hWnd,IDC_JOY_X,IDC_JOY_X==which);
		CheckDlgButton(hWnd,IDC_JOY_Y,IDC_JOY_Y==which);
		CheckDlgButton(hWnd,IDC_JOY_Z,IDC_JOY_Z==which);
		CheckDlgButton(hWnd,IDC_JOY_THROTTLE,IDC_JOY_THROTTLE==which);
		CheckDlgButton(hWnd,IDC_JOY_POVH,IDC_JOY_POVH==which);
		CheckDlgButton(hWnd,IDC_JOY_POVV,IDC_JOY_POVV==which);
		CheckDlgButton(hWnd,IDC_JOY_BUT1,IDC_JOY_BUT1==which);
		CheckDlgButton(hWnd,IDC_JOY_BUT2,IDC_JOY_BUT2==which);
		CheckDlgButton(hWnd,IDC_JOY_BUT3,IDC_JOY_BUT3==which);
		CheckDlgButton(hWnd,IDC_JOY_BUT4,IDC_JOY_BUT4==which);

		CheckDlgButton(hWnd,IDC_JOY_FLIP,invert);
		CheckDlgButton(hWnd,IDC_JOY_ACCUM,type);

		CheckDlgButton(hWnd,IDC_JOY_TYPEINCDEC  ,btype==0);
		CheckDlgButton(hWnd,IDC_JOY_TYPEINC     ,btype==1);
		CheckDlgButton(hWnd,IDC_JOY_TYPEABSOLUTE,btype==2);

		CheckDlgButton(hWnd,IDC_JOY_DIRX,dir==0);
		CheckDlgButton(hWnd,IDC_JOY_DIRY,dir==1);
		CheckDlgButton(hWnd,IDC_JOY_DIRZ,dir==2);

		CheckDlgButton(hWnd,IDC_JOY_COMPX,comp==0);
		CheckDlgButton(hWnd,IDC_JOY_COMPY,comp==1);
		CheckDlgButton(hWnd,IDC_JOY_COMPZ,comp==2);

		ISpinnerControl *spin = GetISpinner(GetDlgItem(hWnd,IDC_JOY_SCALESPIN));
		spin->SetLimits(0.0f, float(999999), FALSE);
		spin->SetScale(0.01f);
		spin->LinkToEdit(GetDlgItem(hWnd,IDC_JOY_SCALE), EDITTYPE_FLOAT);
		spin->SetValue(scale,FALSE);
		ReleaseISpinner(spin);

		spin = GetISpinner(GetDlgItem(hWnd,IDC_JOY_SPEEDSPIN));
		spin->SetLimits(-float(999999), float(999999), FALSE);
		spin->SetScale(0.01f);
		spin->LinkToEdit(GetDlgItem(hWnd,IDC_JOY_SPEED), EDITTYPE_FLOAT);
		spin->SetValue(speed/SPEED_UI_FACT,FALSE);
		ReleaseISpinner(spin);

		SetStates(hWnd);
		UpdateContName(hWnd);
		}
	}

void JoyDeviceBinding::UpdateContName(HWND hWnd)
	{
	if (cont) {
		TSTR name;
		cont->NotifyDependents(FOREVER,(PartID)&name,REFMSG_GET_NODE_NAME);
		if (name.length()) name = name + TSTR(_T("\\")) + TSTR(_T("Rotation"));
		else name = _T("Rotation");
		SetDlgItemText(hWnd,IDC_INC_BINDING, name);
	} else {
		SetDlgItemText(hWnd,IDC_INC_BINDING,GetString(IDS_RB_NONE));
		}
	}

void JoyDeviceBinding::SetStates(HWND hWnd)
	{
	switch (which) {
		case IDC_JOY_BUT1:
		case IDC_JOY_BUT2:
		case IDC_JOY_BUT3:
		case IDC_JOY_BUT4:
			EnableWindow(GetDlgItem(hWnd,IDC_JOY_TYPEINCDEC  ),TRUE);
			EnableWindow(GetDlgItem(hWnd,IDC_JOY_TYPEINC     ),TRUE);
			EnableWindow(GetDlgItem(hWnd,IDC_JOY_TYPEABSOLUTE),TRUE);
			break;

		default:
			EnableWindow(GetDlgItem(hWnd,IDC_JOY_TYPEINCDEC  ),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_JOY_TYPEINC     ),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_JOY_TYPEABSOLUTE),FALSE);
			break;
		}

	if (Accumulating()) {
		if (cont) EnableWindow(GetDlgItem(hWnd,IDC_CLEAR_INC_BINDING),TRUE);
		else EnableWindow(GetDlgItem(hWnd,IDC_CLEAR_INC_BINDING),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_INC_BINDING),TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_JOY_DIRX),TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_JOY_DIRY),TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_JOY_DIRZ),TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_JOY_COMPX),TRUE);		
		EnableWindow(GetDlgItem(hWnd,IDC_JOY_COMPY),TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_JOY_COMPZ),TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_BASED_ON_DIR_LABEL),TRUE);		
		EnableWindow(GetDlgItem(hWnd,IDC_JOY_DIRLABEL),TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_JOY_COMPLABEL),TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_JOY_CONTLABEL),TRUE);
	} else {
		EnableWindow(GetDlgItem(hWnd,IDC_INC_BINDING),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_CLEAR_INC_BINDING),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_JOY_DIRX),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_JOY_DIRY),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_JOY_DIRZ),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_JOY_COMPX),FALSE);		
		EnableWindow(GetDlgItem(hWnd,IDC_JOY_COMPY),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_JOY_COMPZ),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_BASED_ON_DIR_LABEL),FALSE);		
		EnableWindow(GetDlgItem(hWnd,IDC_JOY_DIRLABEL),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_JOY_COMPLABEL),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_JOY_CONTLABEL),FALSE);
		}	
	}

class MCTrackViewFilter : public TrackViewFilter {
	public:		
		BOOL proc(Animatable *anim, Animatable *client,int subNum)
			{
			if (anim->SuperClassID()==CTRL_ROTATION_CLASS_ID &&
				anim->ClassID()==Class_ID(ROT_MOTION_CLASS_ID,0)) {
				return TRUE;
			} else {
				return FALSE;
				}
			}
	};

void JoyDeviceBinding::DoPickCont(HWND hWnd)
	{
	TrackViewPick res;
	MCTrackViewFilter filt;
	if (GetCOREInterface()->TrackViewPickDlg(hWnd,&res, &filt)) {
		if (ReplaceReference(0,res.anim)!=REF_SUCCEED) {
			MessageBox(hWnd,
				_T("Can't create circular reference."),
				GetString(IDS_RB_JOYDEVICE),
				MB_OK|MB_ICONEXCLAMATION);
			}
		UpdateContName(hWnd);
		SetStates(hWnd);
		}
	}

static INT_PTR CALLBACK JoyDeviceDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	IMCParamDlg *dlg = (IMCParamDlg*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	if (!dlg && msg!=WM_INITDIALOG) return FALSE;
	JoyDeviceBinding *b;
	if (dlg) b = (JoyDeviceBinding*)dlg->binding;

	switch (msg) {
		case WM_INITDIALOG:
			dlg = (IMCParamDlg*)lParam;			
			dlg->binding->UpdateRollup(dlg->iRoll);
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);			
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_JOY_X:
				case IDC_JOY_Y:
				case IDC_JOY_Z:
				case IDC_JOY_THROTTLE:
				case IDC_JOY_POVH:
				case IDC_JOY_POVV:
				case IDC_JOY_BUT1:
				case IDC_JOY_BUT2:
				case IDC_JOY_BUT3:
				case IDC_JOY_BUT4:					
					b->which = LOWORD(wParam);
					b->SetStates(hWnd);
					b->NotifyDependents(FOREVER,0,REFMSG_CHANGE);					
					break;								
					
				case IDC_JOY_FLIP:					
					b->invert = IsDlgButtonChecked(hWnd,IDC_JOY_FLIP);
					b->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
					break;

				case IDC_JOY_ACCUM:
					b->type = IsDlgButtonChecked(hWnd,IDC_JOY_ACCUM);
					b->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
					b->SetStates(hWnd);
					break;

				case IDC_JOY_TYPEINCDEC:
					b->btype = 0;
					b->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
					break;
				case IDC_JOY_TYPEINC:
					b->btype = 1;
					b->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
					break;
				case IDC_JOY_TYPEABSOLUTE:
					b->btype = 2;
					b->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
					break;

				case IDC_JOY_DIRX:
					b->dir = 0;
					b->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
					break;
				case IDC_JOY_DIRY:
					b->dir = 1;
					b->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
					break;
				case IDC_JOY_DIRZ:
					b->dir = 2;
					b->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
					break;

				case IDC_JOY_COMPX:
					b->comp = 0;
					b->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
					break;
				case IDC_JOY_COMPY:
					b->comp = 1;
					b->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
					break;
				case IDC_JOY_COMPZ:
					b->comp = 2;
					b->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
					break;

				case IDC_INC_BINDING:
					b->DoPickCont(hWnd);
					break;

				case IDC_CLEAR_INC_BINDING:
					b->ReplaceReference(0,NULL);
					b->UpdateContName(hWnd);
					b->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
					break;
				}
			break;

		case CC_SPINNER_CHANGE: {
			ISpinnerControl *spin = (ISpinnerControl *)lParam;			
			switch (LOWORD(wParam)) {
				case IDC_JOY_SCALESPIN:
					b->scale = spin->GetFVal();
					b->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
					break;

				case IDC_JOY_SPEEDSPIN:
					b->speed = spin->GetFVal()*SPEED_UI_FACT;
					b->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
					break;
				}

			break;
			}

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:   			
   			dlg->iRoll->DlgMouseMessage(hWnd,msg,wParam,lParam);
			break; 

		default:
			return FALSE;
		}
	return TRUE;
	}


#define WHICH_CHUNK_ID		0x0100
#define INVERT_CHUNK_ID		0x0200
#define SCALE_CHUNK_ID		0x0300
#define SPEED_CHUNK_ID		0x0400
#define TYPE_CHUNK_ID		0x0410
#define BTYPE_CHUNK_ID		0x0420
#define DIR_CHUNK_ID		0x0430
#define COMP_CHUNK_ID		0x0440

IOResult JoyDeviceBinding::Save(ISave *isave)
	{
	ULONG nb;	

	isave->BeginChunk(WHICH_CHUNK_ID);
	isave->Write(&which,sizeof(which),&nb);
	isave->EndChunk();
	
	isave->BeginChunk(INVERT_CHUNK_ID);
	isave->Write(&invert,sizeof(invert),&nb);
	isave->EndChunk();
	
	isave->BeginChunk(SCALE_CHUNK_ID);
	isave->Write(&scale,sizeof(scale),&nb);
	isave->EndChunk();

	isave->BeginChunk(SPEED_CHUNK_ID);
	isave->Write(&speed,sizeof(speed),&nb);
	isave->EndChunk();

	isave->BeginChunk(TYPE_CHUNK_ID);
	isave->Write(&type,sizeof(type),&nb);
	isave->EndChunk();

	isave->BeginChunk(BTYPE_CHUNK_ID);
	isave->Write(&btype,sizeof(btype),&nb);
	isave->EndChunk();

	isave->BeginChunk(DIR_CHUNK_ID);
	isave->Write(&dir,sizeof(dir),&nb);
	isave->EndChunk();

	isave->BeginChunk(COMP_CHUNK_ID);
	isave->Write(&comp,sizeof(comp),&nb);
	isave->EndChunk();
	
	return IO_OK;
	}

IOResult JoyDeviceBinding::Load(ILoad *iload)
	{
	ULONG nb;
	IOResult res = IO_OK;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case WHICH_CHUNK_ID:
				res=iload->Read(&which,sizeof(which),&nb);
				break;

			case INVERT_CHUNK_ID:
				res=iload->Read(&invert,sizeof(invert),&nb);
				break;

			case SCALE_CHUNK_ID:
				res=iload->Read(&scale,sizeof(scale),&nb);
				break;

			case SPEED_CHUNK_ID:
				res=iload->Read(&speed,sizeof(speed),&nb);
				break;

			case TYPE_CHUNK_ID:
				res=iload->Read(&type,sizeof(type),&nb);
				break;

			case BTYPE_CHUNK_ID:
				res=iload->Read(&btype,sizeof(btype),&nb);
				break;

			case DIR_CHUNK_ID:
				res=iload->Read(&dir,sizeof(dir),&nb);
				break;

			case COMP_CHUNK_ID:
				res=iload->Read(&comp,sizeof(comp),&nb);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}
	return IO_OK;
	}




//--- JoyDevice --------------------------------------------------

JoyDevice::JoyDevice()
	{
	InitializeCriticalSection(&csect);
	active = FALSE;
	error  = FALSE;
	}

JoyDevice::~JoyDevice()
	{
	DeleteCriticalSection(&csect);
	}

void JoyDevice::ZeroJoy()
	{
	EnterCriticalSection(&csect);

	int dev = joyGetNumDevs();
	JOYINFOEX ji;
	memset(&ji,0,sizeof(ji));
	ji.dwSize = sizeof(ji);
	ji.dwFlags = JOY_RETURNALL;
	MMRESULT res = joyGetPosEx(JOYSTICKID1,&ji);
	error = FALSE;
	switch (res) {
		case JOYERR_UNPLUGGED:
			MessageBox(NULL,_T("Joystick is unplugged"), _T("Joy Stick"), MB_OK);
			error = TRUE;
			break;

		case MMSYSERR_NODRIVER:
			MessageBox(NULL,_T("No joystick driver present."), _T("Joy Stick"), MB_OK);
			error = TRUE;
			break;

		case MMSYSERR_INVALPARAM:
			MessageBox(NULL,_T("Invalid parameter."), _T("Joy Stick"), MB_OK);
			error = TRUE;
			break;
		}
	xbase = ji.dwXpos;
	ybase = ji.dwYpos;	
	zbase = ji.dwRpos;	
	tbase = ji.dwZpos;

	count = 0;
	filterPos = 0;
	Cycle(0);

	LeaveCriticalSection(&csect);
	}

float JoyDevice::DX()
	{
	DWORD val = Filter(xsamp);
	return 0.005f * (float(val) - float(xbase));
	}

float JoyDevice::DY()
	{		
	DWORD val = Filter(ysamp);
	return 0.005f * (float(val) - float(ybase));
	}

float JoyDevice::DZ()
	{		
	DWORD val = Filter(zsamp);
	return 0.005f * (float(val) - float(zbase));
	}

float JoyDevice::DT()
	{		
	DWORD val = Filter(tsamp);
	return 0.005f * (float(val) - float(tbase));
	}

void JoyDevice::Cycle(UINT tick)
	{
	if (!active) return;
	if (error) return;

	JOYINFOEX ji;
	memset(&ji,0,sizeof(ji));
	ji.dwSize = sizeof(ji);
	ji.dwFlags = JOY_RETURNALL;
	
	EnterCriticalSection(&csect);
	joyGetPosEx(JOYSTICKID1,&ji);	
	
	xsamp[filterPos] = ji.dwXpos;
	ysamp[filterPos] = ji.dwYpos;
	zsamp[filterPos] = ji.dwRpos;
	tsamp[filterPos] = ji.dwZpos;
	if (count<FILTER_SIZE) count++;
	filterPos++;
	if (filterPos>=FILTER_SIZE) filterPos = 0;
	LeaveCriticalSection(&csect);	
	}

DWORD JoyDevice::Filter(DWORD *f)
	{
	if (!count) return 0;
	DWORD val = 0;
	EnterCriticalSection(&csect);	
	for (DWORD i=0; i<count; i++) {
		int index = filterPos-i-1;
		if (index<0) index += FILTER_SIZE;
		assert(index>=0 && index<FILTER_SIZE);
		val += f[index];
		}
	LeaveCriticalSection(&csect);	
	if (!count) return 0;
	return val/count;
	}

float JoyDevice::POVH()
	{
	DWORD pov = ReadPOV();
	float val = 0.0f;
	if (pov==JOY_POVLEFT)  val = -1.0f;
	if (pov==JOY_POVRIGHT) val = 1.0f;
	return val;
	}

float JoyDevice::POVV()
	{
	DWORD pov = ReadPOV();
	float val = 0.0f;
	if (pov==JOY_POVFORWARD)   val = 1.0f;
	if (pov==JOY_POVBACKWARD)  val = -1.0f;
	return val;
	}

float JoyDevice::But1()
	{
	return ReadButtons()&JOY_BUTTON1 ? 1.0f : -1.0f;
	}

float JoyDevice::But2()
	{
	return ReadButtons()&JOY_BUTTON2 ? 1.0f : -1.0f;
	}

float JoyDevice::But3()
	{
	return ReadButtons()&JOY_BUTTON3 ? 1.0f : -1.0f;
	}

float JoyDevice::But4()
	{
	return ReadButtons()&JOY_BUTTON4 ? 1.0f : -1.0f;
	}

DWORD JoyDevice::ReadButtons()
	{
	if (error) return 0;

	JOYINFOEX ji;
	memset(&ji,0,sizeof(ji));
	ji.dwSize = sizeof(ji);
	ji.dwFlags = JOY_RETURNALL;
	
	EnterCriticalSection(&csect);
	joyGetPosEx(JOYSTICKID1,&ji);	
	LeaveCriticalSection(&csect);	

	return ji.dwButtons;
	}

DWORD JoyDevice::ReadPOV()
	{
	if (error) return 0;

	JOYINFOEX ji;
	memset(&ji,0,sizeof(ji));
	ji.dwSize = sizeof(ji);
	ji.dwFlags = JOY_RETURNPOVCTS;
	
	EnterCriticalSection(&csect);
	joyGetPosEx(JOYSTICKID1,&ji);
	LeaveCriticalSection(&csect);	

	return ji.dwPOV;
	}
