/**********************************************************************
 *<
	FILE: mcdevice.cpp

	DESCRIPTION: Devices for the motion manager

	CREATED BY: Rolf Berteig

	HISTORY: October 30, 1996

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#include "motion.h"
#include "mcapdev.h"
//#include "mmanager.h"

#define MC_MOUSE_X	0
#define MC_MOUSE_Y	1

#define MOUSE_DEVICE_CLASS_ID	0xbb387ff1

static MCInputDevice *GetMouseDevice();

class MouseDeviceBinding : public MCDeviceBinding {
	public:
		int which, invert;
		float scale;
		MouseDeviceBinding() {which=MC_MOUSE_X; scale=1.0f; invert=FALSE;}
		MCInputDevice *GetDevice() {return GetMouseDevice();}
		TSTR BindingName() {return GetString(which==MC_MOUSE_X?IDS_RB_MOUSEX:IDS_RB_MOUSEY);}
		float Eval(TimeValue t);
		void DeleteThis() {delete this;}
		Class_ID ClassID() {return Class_ID(MOUSE_DEVICE_CLASS_ID,0);}
		RefTargetHandle Clone(RemapDir& remap);

		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		void AddRollup(IMCParamDlg *dlg);
		void UpdateRollup(IRollupWindow *iRoll);
		void BeginActivate(BOOL reset);
	};

class MouseDevice : public MCInputDevice {
	public:
		int xbase, ybase;

		TSTR DeviceName() {return GetString(IDS_RB_MOUSEDEVICE);}
		MCDeviceBinding *CreateBinding() {return new MouseDeviceBinding;}

		void ZeroMouse();
		float DX();
		float DY();
		void CheckForWrap();
	};

//--- Class Descriptor -----------------------------------------------

// This is only here to support old files. Before the motion capture
// device interface was made plug-able the general REF_TARGET_CLASS_ID was used.
class MouseDeviceClassDescOld:public ClassDesc {
	public:
	int 			IsPublic() {return 0;}
	void *			Create(BOOL loading) {return new MouseDeviceBinding;}
	const TCHAR *	ClassName() {return GetString(IDS_RB_MOUSEDEVICE);}
	SClass_ID		SuperClassID() {return REF_TARGET_CLASS_ID;}
	Class_ID		ClassID() {return Class_ID(MOUSE_DEVICE_CLASS_ID,0);}
	const TCHAR* 	Category() {return _T("");}
	};
static MouseDeviceClassDescOld mouseCDOld;
ClassDesc* GetMouseDeviceClassDescDescOld() {return &mouseCDOld;}

class MouseDeviceClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 0;}
	void *			Create(BOOL loading) {return new MouseDeviceBinding;}
	const TCHAR *	ClassName() {return GetString(IDS_RB_MOUSEDEVICE);}
	SClass_ID		SuperClassID() {return MOT_CAP_DEVBINDING_CLASS_ID;}
	Class_ID		ClassID() {return Class_ID(MOUSE_DEVICE_CLASS_ID,0);}
	const TCHAR* 	Category() {return _T("");}
	};
static MouseDeviceClassDesc mouseCD;
ClassDesc* GetMouseDeviceClassDescDesc() {return &mouseCD;}

class TheMouseDeviceClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading) {return GetMouseDevice();}
	const TCHAR *	ClassName() {return GetString(IDS_RB_MOUSEDEVICE);}
	SClass_ID		SuperClassID() {return MOT_CAP_DEV_CLASS_ID;}
	Class_ID		ClassID() {return Class_ID(MOUSE_DEVICE_CLASS_ID,1);}
	const TCHAR* 	Category() {return _T("");}
	};
static TheMouseDeviceClassDesc theMouseCD;
ClassDesc* GetTheMouseDeviceClassDescDesc() {return &theMouseCD;}


//--- Mouse device binding ---------------------------------------------------

static INT_PTR CALLBACK MouseDeviceDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static MouseDevice theMouseDevice;
static MCInputDevice *GetMouseDevice() {return &theMouseDevice;}

float MouseDeviceBinding::Eval(TimeValue t)
	{
	float val = 0.0f;
	if (which==MC_MOUSE_X) {
		return theMouseDevice.DX() * scale * (invert?-1.0f:1.0f);
	} else {
		return theMouseDevice.DY() * scale * (invert?-1.0f:1.0f);
		}
	}

RefTargetHandle MouseDeviceBinding::Clone(RemapDir& remap)
	{
	MouseDeviceBinding *b = new MouseDeviceBinding;
	b->which  = which;
	b->invert = invert;
	b->scale  = scale;
	BaseClone(this, b, remap);
	return b;
	}

void MouseDeviceBinding::BeginActivate(BOOL reset)
	{
	if (reset) theMouseDevice.ZeroMouse();
	}

void MouseDeviceBinding::AddRollup(IMCParamDlg *dlg)
	{	
	dlg->iRoll->AppendRollup(
			hInstance, 
			MAKEINTRESOURCE(IDD_MC_MOUSE), 
			MouseDeviceDlgProc, 
			GetString(IDS_RB_MOUSEDEVICE), 
			(LPARAM)dlg);
	}

void MouseDeviceBinding::UpdateRollup(IRollupWindow *iRoll)
	{
	if (iRoll->GetNumPanels()>1) {
		HWND hWnd = iRoll->GetPanelDlg(1);
		CheckDlgButton(hWnd,IDC_MOUSE_X,which==MC_MOUSE_X);
		CheckDlgButton(hWnd,IDC_MOUSE_Y,which==MC_MOUSE_Y);
		CheckDlgButton(hWnd,IDC_MOUSE_FLIP,invert);

		ISpinnerControl *spin = GetISpinner(GetDlgItem(hWnd,IDC_MOUSE_SCALESPIN));
		spin->SetLimits(0.0f, float(999999), FALSE);
		spin->SetScale(0.01f);
		spin->LinkToEdit(GetDlgItem(hWnd,IDC_MOUSE_SCALE), EDITTYPE_FLOAT);
		spin->SetValue(scale,FALSE);
		ReleaseISpinner(spin);
		}
	}

static INT_PTR CALLBACK MouseDeviceDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	IMCParamDlg *dlg = (IMCParamDlg*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	if (!dlg && msg!=WM_INITDIALOG) return FALSE;

	switch (msg) {
		case WM_INITDIALOG:
			dlg = (IMCParamDlg*)lParam;
			dlg->binding->UpdateRollup(dlg->iRoll);
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_MOUSE_X: {
					MouseDeviceBinding *b = (MouseDeviceBinding*)dlg->binding;
					b->which = MC_MOUSE_X;
					b->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
					break;
					}

				case IDC_MOUSE_Y: {
					MouseDeviceBinding *b = (MouseDeviceBinding*)dlg->binding;
					b->which = MC_MOUSE_Y;
					b->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
					break;
					}

				case IDC_MOUSE_FLIP: {
					MouseDeviceBinding *b = (MouseDeviceBinding*)dlg->binding;
					b->invert = IsDlgButtonChecked(hWnd,IDC_MOUSE_FLIP);
					b->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
					break;
					}
				}
			break;

		case CC_SPINNER_CHANGE: {
			ISpinnerControl *spin = (ISpinnerControl *)lParam;
			MouseDeviceBinding *b = (MouseDeviceBinding*)dlg->binding;
			b->scale = spin->GetFVal();
			b->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
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

IOResult MouseDeviceBinding::Save(ISave *isave)
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
	
	return IO_OK;
	}

IOResult MouseDeviceBinding::Load(ILoad *iload)
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
			}
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}
	return IO_OK;
	}




//--- MouseDevice --------------------------------------------------

void MouseDevice::ZeroMouse()
	{
	POINT pt;
	GetCursorPos(&pt);
	xbase = pt.x;
	ybase = pt.y;
	}

void MouseDevice::CheckForWrap()
	{
	#define	WRAP_MARGIN 10
	//int sy = GetSystemMetrics(SM_CYSCREEN);
	//int sx = GetSystemMetrics(SM_CXSCREEN);
	// CCJ 6.6.00 These new maxutil functions supports multiple monitors
	int sx = GetScreenWidth();
	int sy = GetScreenHeight();

	POINT pt;
	GetCursorPos(&pt);
	if (pt.y<WRAP_MARGIN) {
		ybase += sy-WRAP_MARGIN-pt.y;
		pt.y   = sy-WRAP_MARGIN;
		SetCursorPos(pt.x, pt.y);		
	} else 
	if (pt.y>sy-WRAP_MARGIN) {			
		ybase  += WRAP_MARGIN-pt.y;
		pt.y    = WRAP_MARGIN;			
		SetCursorPos(pt.x, pt.y);		
		}
	if (pt.x<WRAP_MARGIN) {
		xbase += sx-WRAP_MARGIN-pt.x;
		pt.x   = sx-WRAP_MARGIN;
		SetCursorPos(pt.x, pt.y);		
	} else 
	if (pt.x>sx-WRAP_MARGIN) {			
		xbase  += WRAP_MARGIN-pt.x;
		pt.x    = WRAP_MARGIN;			
		SetCursorPos(pt.x, pt.y);		
		}
	}

float MouseDevice::DX()
	{
	CheckForWrap();
	POINT pt;
	GetCursorPos(&pt);
	return float(pt.x - xbase);
	}

float MouseDevice::DY()
	{
	CheckForWrap();
	POINT pt;
	GetCursorPos(&pt);
	return -float(pt.y - ybase);
	}

