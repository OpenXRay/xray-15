/**********************************************************************
 *<
	FILE: kbddev.cpp

	DESCRIPTION: Keyboard device driver for motion capture system

	CREATED BY: Christer Janson

	HISTORY: May 21, 1997

 *>     Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#include "mckbddev.h"
#include "mcapdev.h"

HINSTANCE	hInstance;
int			controlsInit = FALSE;

#define KBD_DEVICE_CLASS_ID			Class_ID(0x3b5228b8, 0x18377f5b)
#define KBD_DEVICEBINDING_CLASS_ID	Class_ID(0x4cd34d1d, 0x4d50722f)

static MCInputDevice *GetKbdDevice();

static void InitEnvelopeImage(HINSTANCE hInst);

class KbdDeviceBinding : public MCDeviceBinding {
	public:
		float	attack;
		float	decay;
		float	sustain;
		float	release;
		float	scale;
		float	timeScale;
		USHORT	nVKey;

		int		pressStatus;
		float	valueWhenStatusChanged;
		float	curValue;

		KbdDeviceBinding();
		MCInputDevice	*GetDevice() {return GetKbdDevice();}
		TSTR	BindingName();
		float	Eval(int time);
		void	DeleteThis() {delete this;}
		Class_ID	ClassID() {return KBD_DEVICEBINDING_CLASS_ID;}
		RefTargetHandle	Clone(RemapDir& remap);

		IOResult	Save(ISave *isave);
		IOResult	Load(ILoad *iload);

		void	AddRollup(IMCParamDlg *dlg);
		void	UpdateRollup(IRollupWindow *iRoll);
		void	BeginActivate(BOOL reset);
		void	EndActivate();

		void	AssignKey(HWND hParent);
		TCHAR*	GetVKeyName(USHORT vKey);
};

#define ELEMENTS(array) (sizeof(array)/sizeof((array)[0]))

struct VKeyStatus {
	USHORT	vKey;
	BOOL	bPressed;
	UINT	nActionTime;
	UINT	nLastSample;
};

struct VKey {
	USHORT	vkey;
	char*	name;
	int		nameRes;
};

VKey KeyList[] = {
	186,        _T(";"),	0,
	187,        _T("="),	0,
	188,        _T(","),	0,
	189,        _T("-"),	0,
	190,        _T("."),	0,
	191,        _T("/"),	0,
	192,        _T("`"),	0,
	219,        _T("["),	0,
	220,        _T("\\"),	0,
	221,        _T("]"),	0,
	222,        _T("\'"),	0,
	VK_BACK,	NULL,	IDS_KBD_KEYNAME_BACKSPACE,
	VK_SPACE,	NULL,	IDS_KBD_KEYNAME_SPACE,
	VK_PRIOR,	NULL,	IDS_KBD_KEYNAME_PGUP,
	VK_NEXT,	NULL,	IDS_KBD_KEYNAME_PGDN,
	VK_END,		NULL,	IDS_KBD_KEYNAME_END,
	VK_HOME,	NULL,	IDS_KBD_KEYNAME_HOME,
	VK_LEFT,	NULL,	IDS_KBD_KEYNAME_LEFTARROW,
	VK_UP,		NULL,	IDS_KBD_KEYNAME_UPARROW,
	VK_RIGHT,	NULL,	IDS_KBD_KEYNAME_RIGHTARROW,
	VK_DOWN,	NULL,	IDS_KBD_KEYNAME_DOWNARROW,
	VK_INSERT,	NULL,	IDS_KBD_KEYNAME_INSERT,
	VK_DELETE,	NULL,	IDS_KBD_KEYNAME_DELETE,
	VK_NUMPAD0,	NULL,	IDS_KBD_KEYNAME_NUMPAD0,
	VK_NUMPAD1,	NULL,	IDS_KBD_KEYNAME_NUMPAD1,
	VK_NUMPAD2,	NULL,	IDS_KBD_KEYNAME_NUMPAD2,
	VK_NUMPAD3,	NULL,	IDS_KBD_KEYNAME_NUMPAD3,
	VK_NUMPAD4,	NULL,	IDS_KBD_KEYNAME_NUMPAD4,
	VK_NUMPAD5,	NULL,	IDS_KBD_KEYNAME_NUMPAD5,
	VK_NUMPAD6,	NULL,	IDS_KBD_KEYNAME_NUMPAD6,
	VK_NUMPAD7,	NULL,	IDS_KBD_KEYNAME_NUMPAD7,
	VK_NUMPAD8,	NULL,	IDS_KBD_KEYNAME_NUMPAD8,
	VK_NUMPAD9,	NULL,	IDS_KBD_KEYNAME_NUMPAD9,
	VK_MULTIPLY,NULL,	IDS_KBD_KEYNAME_GREYSTAR,
	VK_ADD,		NULL,	IDS_KBD_KEYNAME_GREYPLUS,
	VK_SUBTRACT,NULL,	IDS_KBD_KEYNAME_GREYMINUS,
	VK_DECIMAL,	NULL,	IDS_KBD_KEYNAME_GREYDOT,
	VK_DIVIDE,	NULL,	IDS_KBD_KEYNAME_GREYSLASH,
	//VK_F1,		NULL,	IDS_KBD_KEYNAME_F1,
	VK_F2,		NULL,	IDS_KBD_KEYNAME_F2,
	VK_F3,		NULL,	IDS_KBD_KEYNAME_F3,
	VK_F4,		NULL,	IDS_KBD_KEYNAME_F4,
	VK_F5,		NULL,	IDS_KBD_KEYNAME_F5,
	VK_F6,		NULL,	IDS_KBD_KEYNAME_F6,
	VK_F7,		NULL,	IDS_KBD_KEYNAME_F7,
	VK_F8,		NULL,	IDS_KBD_KEYNAME_F8,
	VK_F9,		NULL,	IDS_KBD_KEYNAME_F9,
	VK_F10,		NULL,	IDS_KBD_KEYNAME_F10,
	VK_F11,		NULL,	IDS_KBD_KEYNAME_F11,
	VK_F12,		NULL,	IDS_KBD_KEYNAME_F12,
};

typedef Tab<VKeyStatus*> VKeyStatusTab;

class KbdDevice : public MCInputDevice {
	public:
		BOOL	active;
		CRITICAL_SECTION	csect;

		KbdDevice();
		~KbdDevice();

		TSTR DeviceName()	{return GetString(IDS_KBD_DEVICENAME);}
		MCDeviceBinding	*CreateBinding() {return new KbdDeviceBinding;}

		void	Cycle(UINT tick);

		void	AppendVKey(USHORT vKey);
		void	RemoveVKey(USHORT vKey);
		VKeyStatus*	QueryVKey(USHORT vKey);

	private:
		VKeyStatusTab	vKeyTab;
};

//--- Class Descriptors -----------------------------------------------

// ClassDesc for Device Binding
class KbdDeviceBindingClassDesc:public ClassDesc {
	public:
	int				IsPublic() {return 0;}
	void*			Create(BOOL loading) {return new KbdDeviceBinding;}
	const TCHAR*	ClassName() {return GetString(IDS_KBD_DEVICENAME);}
	SClass_ID		SuperClassID() {return MOT_CAP_DEVBINDING_CLASS_ID;}
	Class_ID		ClassID() {return KBD_DEVICEBINDING_CLASS_ID;}
	const TCHAR*	Category() {return _T("");}
};

static KbdDeviceBindingClassDesc kbdBindCD;
ClassDesc* GetKbdBindingClassDesc() {return &kbdBindCD;}

// ClassDesc for Device
class KbdDeviceClassDesc:public ClassDesc {
	public:
	int				IsPublic() {return 1;}
	void*			Create(BOOL loading) {return GetKbdDevice();}
	const TCHAR*	ClassName() {return GetString(IDS_KBD_DEVICENAME);}
	SClass_ID		SuperClassID() {return MOT_CAP_DEV_CLASS_ID;}
	Class_ID		ClassID() {return KBD_DEVICE_CLASS_ID;}
	const TCHAR*	Category() {return _T("");}
};

static KbdDeviceClassDesc kbdCD;
ClassDesc* GetKbdDeviceClassDesc() {return &kbdCD;}


//--- Kbd device binding ---------------------------------------------------

static INT_PTR CALLBACK KbdDeviceDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static KbdDevice theKbdDevice;
MCInputDevice *GetKbdDevice() {return &theKbdDevice;}

KbdDeviceBinding::KbdDeviceBinding() 
{
	// Setup default values
	attack = 0.2f;
	decay = 0.0f;
	sustain = 1.0f;
	release = 0.2f;
	scale = 10.0f;
	timeScale = 1.0f;
	nVKey = 0;
}

TSTR KbdDeviceBinding::BindingName() 
{
	TSTR bindStr;
	TSTR keyStr = GetString(IDS_KBD_KEY);

	if (nVKey == 0) {
		bindStr.printf("%s: %s", keyStr, GetString(IDS_KBD_UNASSIGNED));
	}
	else {
		bindStr.printf("%s: %s", keyStr, GetVKeyName(nVKey));
	}

	return bindStr;
}


float KbdDeviceBinding::Eval(int time)
{
	// Query the device for the status of this vkey
	VKeyStatus* pVks = theKbdDevice.QueryVKey(nVKey);

	if (!pVks)
		return 0.0f;

	// Get the time since the last press/release
	float deltaT = (pVks->nLastSample - pVks->nActionTime)/(timeScale*1000.0f);
	
	// When the press/release status changes, we save off the value.
	// This lets us start attach/release slopes from the current value
	// instead of always starting from 0 or sustain.
	if (pVks->bPressed != pressStatus) {
		pressStatus = pVks->bPressed;
		valueWhenStatusChanged = curValue;
	}

	// Here we're bringing down attack with the time it [would have] taken
	// to get to the current value.
	float usedAttack = attack*(1-valueWhenStatusChanged);

	if (pVks->bPressed) {
		if (deltaT < usedAttack) {
			// Attack in progress
			curValue = deltaT/(usedAttack)*(1-valueWhenStatusChanged)+valueWhenStatusChanged;
		}
		else if (deltaT < (usedAttack+decay)) {
			// Decay in progress
			curValue = 1.0f-(deltaT-usedAttack)/decay*(1-sustain);
		}
		else {
			// Sustain ongoing
			curValue = sustain;
		}
	}
	else {
		if (deltaT < release && pVks->nActionTime > 0) {
			// releasing...
			curValue = valueWhenStatusChanged-deltaT/release;
		}
		else {
			curValue = 0.0f;
		}
	}

	// This shouldn't really be needed, but if somethinmg goes terribly wrong,
	// it shouldn't affect the user too much.
	if (curValue<0.0f)
		curValue = 0.0f;

	if (curValue>1.0f)
		curValue = 1.0f;

	return curValue*scale;
}

RefTargetHandle KbdDeviceBinding::Clone(RemapDir& remap)
{
	KbdDeviceBinding *b = new KbdDeviceBinding;
	b->attack = attack;
	b->decay = decay;
	b->sustain = sustain;
	b->release = release;
	b->scale = scale;
	b->timeScale = timeScale;
	b->nVKey = nVKey;
	BaseClone(this, b, remap);
	return b;
}

void KbdDeviceBinding::BeginActivate(BOOL reset)
{
	// Initialize at startup.
	pressStatus = -1;
	valueWhenStatusChanged = 0.0f;
	curValue = 0.0f;

	theKbdDevice.active = TRUE;
	theKbdDevice.AppendVKey(nVKey);
	DisableAccelerators();
}

void KbdDeviceBinding::EndActivate()
{
	theKbdDevice.active = FALSE;
	theKbdDevice.RemoveVKey(nVKey);
	EnableAccelerators();
}


void KbdDeviceBinding::AddRollup(IMCParamDlg *dlg)
{
	InitEnvelopeImage(hInstance);

	dlg->iRoll->AppendRollup(
			hInstance, 
			MAKEINTRESOURCE(IDD_MC_KBD), 
			KbdDeviceDlgProc, 
			GetString(IDS_KBD_DEVICENAME),
			(LPARAM)dlg);
}

void KbdDeviceBinding::UpdateRollup(IRollupWindow *iRoll)
{
	if (iRoll->GetNumPanels()>1) {
		HWND hWnd = iRoll->GetPanelDlg(1);
		
		if (hWnd) {
			ISpinnerControl *spin;
			spin = GetISpinner(GetDlgItem(hWnd,IDC_KBD_ATTACKSPIN));
			spin->SetLimits(0.0f, 1.0f, FALSE);
			spin->SetScale(0.01f);
			spin->LinkToEdit(GetDlgItem(hWnd,IDC_KBD_ATTACK), EDITTYPE_FLOAT);
			spin->SetValue(attack,FALSE);
			ReleaseISpinner(spin);

			spin = GetISpinner(GetDlgItem(hWnd,IDC_KBD_DECAYSPIN));
			spin->SetLimits(0.0f, 1.0f, FALSE);
			spin->SetScale(0.01f);
			spin->LinkToEdit(GetDlgItem(hWnd,IDC_KBD_DECAY), EDITTYPE_FLOAT);
			spin->SetValue(decay,FALSE);
			ReleaseISpinner(spin);

			spin = GetISpinner(GetDlgItem(hWnd,IDC_KBD_SUSTAINSPIN));
			spin->SetLimits(0.0f, 1.0f, FALSE);
			spin->SetScale(0.01f);
			spin->LinkToEdit(GetDlgItem(hWnd,IDC_KBD_SUSTAIN), EDITTYPE_FLOAT);
			spin->SetValue(sustain,FALSE);
			ReleaseISpinner(spin);

			spin = GetISpinner(GetDlgItem(hWnd,IDC_KBD_RELEASESPIN));
			spin->SetLimits(0.0f, 1.0f, FALSE);
			spin->SetScale(0.01f);
			spin->LinkToEdit(GetDlgItem(hWnd,IDC_KBD_RELEASE), EDITTYPE_FLOAT);
			spin->SetValue(release,FALSE);
			ReleaseISpinner(spin);

			spin = GetISpinner(GetDlgItem(hWnd,IDC_KBD_SCALESPIN));
			spin->SetLimits(float(-999999), float(999999), FALSE);
			spin->SetAutoScale(TRUE);
			spin->LinkToEdit(GetDlgItem(hWnd,IDC_KBD_SCALE), EDITTYPE_FLOAT);
			spin->SetValue(scale,FALSE);
			ReleaseISpinner(spin);

			spin = GetISpinner(GetDlgItem(hWnd,IDC_KBD_TIMESCALESPIN));
			spin->SetLimits(float(0.01), float(999999), FALSE);
			spin->SetAutoScale(TRUE);
			spin->LinkToEdit(GetDlgItem(hWnd,IDC_KBD_TIMESCALE), EDITTYPE_FLOAT);
			spin->SetValue(timeScale,FALSE);
			ReleaseISpinner(spin);

			InvalidateRect(GetDlgItem(hWnd, IDC_KBD_ENVGRAPH), NULL, FALSE);

			int numElems = ELEMENTS(KeyList);
			int idx;
			SendMessage(GetDlgItem(hWnd, IDC_KBD_KEYEDIT), CB_RESETCONTENT, 0, 0);
			for (int i=0; i<numElems; i++) {
				// A little kludgy, but so what. I only want keys that has a resource in the drop down.
				// The rest of the entries in the KeyList are for displaying the key after it's been pressed.
				// By doing it this way I can do it with one list.
				if (KeyList[i].nameRes != 0) {
					idx = SendMessage(GetDlgItem(hWnd, IDC_KBD_KEYEDIT), CB_ADDSTRING, 0, (LPARAM)GetVKeyName(KeyList[i].vkey));
					SendMessage(GetDlgItem(hWnd, IDC_KBD_KEYEDIT), CB_SETITEMDATA, idx, (LPARAM)KeyList[i].vkey);
				}
			}

			// Display the configured key
			SendMessage(GetDlgItem(hWnd, IDC_KBD_KEYEDIT), WM_SETTEXT, 0, (LPARAM)GetVKeyName(nVKey));
		}
	}
}

TCHAR* KbdDeviceBinding::GetVKeyName(USHORT vKey)
{
	static TCHAR keyName[10];

	if (vKey == 0) {
		return GetString(IDS_KBD_UNASSIGNED);
	}

	int numElems = ELEMENTS(KeyList);
	for (int i=0; i<numElems; i++) {
		if (vKey == KeyList[i].vkey) {
			if (KeyList[i].name) {
				// A special key < 32
				return KeyList[i].name;
			}
			else {
				// A named key
				return GetString(KeyList[i].nameRes);
			}
		}
	}

	// If it is none of the above, it's a letter or character.
	// Just display the character as is.

	_stprintf(keyName, _T("%c"), vKey);
	return keyName;
}


static INT_PTR CALLBACK KbdDeviceDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	IMCParamDlg *dlg = (IMCParamDlg*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	if (!dlg && msg!=WM_INITDIALOG) return FALSE;
	KbdDeviceBinding *b;
	if (dlg) b = (KbdDeviceBinding*)dlg->binding;

	switch (msg) {
		case WM_INITDIALOG:
			dlg = (IMCParamDlg*)lParam;
			b = (KbdDeviceBinding*)dlg->binding;
			b->UpdateRollup(dlg->iRoll);
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			SetWindowLongPtr(GetDlgItem(hWnd,IDC_KBD_ENVGRAPH),GWLP_USERDATA,lParam);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_KBD_KEYBTN:
					b->AssignKey(hWnd);
					// Need to tell Manager that key has changed
					// so that the label can be updated
					b->NotifyDependents(FOREVER,0,REFMSG_CHANGE);                                   
					break;                                                          
				case IDC_KBD_KEYEDIT:
					switch(HIWORD(wParam)) {
						case CBN_SETFOCUS:
							DisableAccelerators();
							break;
						case CBN_KILLFOCUS:
							EnableAccelerators();
							break;
						case CBN_SELENDOK: {
							int idx = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
							b->nVKey = (USHORT)SendMessage((HWND)lParam, CB_GETITEMDATA, idx, 0);
							// Need to tell Manager that key has changed
							// so that the label can be updated
							b->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
							break;
						}
						case CBN_EDITCHANGE:
							SendMessage(GetDlgItem(hWnd, IDC_KBD_KEYEDIT), WM_SETTEXT, 0, (LPARAM)b->GetVKeyName(b->nVKey));
							break;
					}
				}
			break;
		case CC_SPINNER_CHANGE: {
			ISpinnerControl *spin = (ISpinnerControl *)lParam;
			switch (LOWORD(wParam)) {
				case IDC_KBD_ATTACKSPIN:
					b->attack = spin->GetFVal();
					break;
				case IDC_KBD_DECAYSPIN:
					b->decay = spin->GetFVal();
					break;
				case IDC_KBD_SUSTAINSPIN:
					b->sustain = spin->GetFVal();
					break;
				case IDC_KBD_RELEASESPIN:
					b->release = spin->GetFVal();
					break;
				case IDC_KBD_SCALESPIN:
					b->scale = spin->GetFVal();
					break;
				case IDC_KBD_TIMESCALESPIN:
					b->timeScale = spin->GetFVal();
					break;
				}
				// Since we don't register for undo, we need this:
				SetSaveRequiredFlag(TRUE);

				InvalidateRect(GetDlgItem(hWnd, IDC_KBD_ENVGRAPH), NULL, TRUE);
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


#define ATTACK_CHUNK_ID		0x0100
#define DECAY_CHUNK_ID		0x0200
#define SUSTAIN_CHUNK_ID	0x0300
#define RELEASE_CHUNK_ID	0x0400
#define SCALE_CHUNK_ID		0x0500
#define TIMESCALE_CHUNK_ID	0x0550
#define VKEY_CHUNK_ID		0x0600

IOResult KbdDeviceBinding::Save(ISave *isave)
{
	ULONG nb;       

	isave->BeginChunk(ATTACK_CHUNK_ID);
	isave->Write(&attack,sizeof(attack),&nb);
	isave->EndChunk();
	
	isave->BeginChunk(DECAY_CHUNK_ID);
	isave->Write(&decay,sizeof(decay),&nb);
	isave->EndChunk();
	
	isave->BeginChunk(SUSTAIN_CHUNK_ID);
	isave->Write(&sustain,sizeof(sustain),&nb);
	isave->EndChunk();

	isave->BeginChunk(RELEASE_CHUNK_ID);
	isave->Write(&release,sizeof(release),&nb);
	isave->EndChunk();

	isave->BeginChunk(SCALE_CHUNK_ID);
	isave->Write(&scale,sizeof(scale),&nb);
	isave->EndChunk();

	isave->BeginChunk(TIMESCALE_CHUNK_ID);
	isave->Write(&timeScale,sizeof(timeScale),&nb);
	isave->EndChunk();

	isave->BeginChunk(VKEY_CHUNK_ID);
	isave->Write(&nVKey,sizeof(nVKey),&nb);
	isave->EndChunk();

	return IO_OK;
}

IOResult KbdDeviceBinding::Load(ILoad *iload)
	{
	ULONG nb;
	IOResult res = IO_OK;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case ATTACK_CHUNK_ID:
				res=iload->Read(&attack,sizeof(attack),&nb);
				break;

			case DECAY_CHUNK_ID:
				res=iload->Read(&decay,sizeof(decay),&nb);
				break;

			case SUSTAIN_CHUNK_ID:
				res=iload->Read(&sustain,sizeof(sustain),&nb);
				break;

			case RELEASE_CHUNK_ID:
				res=iload->Read(&release,sizeof(release),&nb);
				break;

			case SCALE_CHUNK_ID:
				res=iload->Read(&scale,sizeof(scale),&nb);
				break;

			case TIMESCALE_CHUNK_ID:
				res=iload->Read(&timeScale,sizeof(timeScale),&nb);
				break;

			case VKEY_CHUNK_ID:
				res=iload->Read(&nVKey,sizeof(nVKey),&nb);
				break;
		}
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
	}
	return IO_OK;
}




//--- KbdDevice --------------------------------------------------

KbdDevice::KbdDevice()
{
	InitializeCriticalSection(&csect);
	active = FALSE;
}

KbdDevice::~KbdDevice()
{
	DeleteCriticalSection(&csect);
}

void KbdDevice::AppendVKey(USHORT vKey)
{
	EnterCriticalSection(&csect);
	
	// No need to append the same key twice.
	// If one binding adds the device, many bindings can read
	// the key afterwards.
	BOOL bKeyExist = FALSE;
	for (int i=0; i<vKeyTab.Count(); i++) {
		if (vKeyTab[i]->vKey == vKey) {
			bKeyExist = TRUE;
			break;
		}
	}
		
	if (!bKeyExist) {
		VKeyStatus* pVks = new VKeyStatus;
		pVks->vKey = vKey;
		pVks->bPressed = FALSE;
		pVks->nActionTime = 0;
		pVks->nLastSample = 0;
		vKeyTab.Append(1, &pVks, 10);
	}

	LeaveCriticalSection(&csect);
}

void KbdDevice::RemoveVKey(USHORT nVKey)
{
	EnterCriticalSection(&csect);

	// Step through and delete the specified key entry.
	// Abort when we find one since we don't add duplicates.
	// Also, make sure not to complain if it's not found,
	// since this will happen when multiple device bindings
	// requests (thus deletes) the same key.
	for (int i=0; i<vKeyTab.Count(); i++) {
		VKeyStatus* pVks = vKeyTab[i];
		if (pVks->vKey == nVKey) {
			vKeyTab.Delete(i, 1);
			delete pVks;
			break;
		}
	}
	
	LeaveCriticalSection(&csect);
}

VKeyStatus* KbdDevice::QueryVKey(USHORT vKey)
{
	VKeyStatus* pVks = NULL;
	
	EnterCriticalSection(&csect);
	
	for (int i=0; i<vKeyTab.Count(); i++) {
		if (vKeyTab[i]->vKey == vKey) {
			pVks = vKeyTab[i];
			break;
		}
	}

	LeaveCriticalSection(&csect);
	
	return pVks;
}

void KbdDevice::Cycle(UINT tick)
{
	int i, numKeys;
	VKeyStatus* pVks;
	USHORT nStatus;

	if (!active)
		return;

	numKeys = vKeyTab.Count();
		
	EnterCriticalSection(&csect);
	for (i=0; i<numKeys; i++) {
		pVks = vKeyTab[i];
	
		pVks->nLastSample = tick;

		nStatus = GetAsyncKeyState(pVks->vKey);
		if (nStatus & 0x8000) {
			if (!pVks->bPressed) {
				pVks->bPressed = TRUE;
				pVks->nActionTime = tick;
			}
		}
		else {
			if (pVks->bPressed) {
				pVks->bPressed = FALSE;
				pVks->nActionTime = tick;
			}
		}
	}
	LeaveCriticalSection(&csect);	
}


//--- Envelope Graph ---------------------------------------------

static LRESULT CALLBACK EnvelopeImageWinProc( 
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static void InitEnvelopeImage(HINSTANCE hInst)
{
	static BOOL init = FALSE;
	if (init) return;
	else init = TRUE;

	WNDCLASS wc;
	wc.style         = 0;
    wc.hInstance     = hInst;
    wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName  = NULL;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.lpfnWndProc   = EnvelopeImageWinProc;
    wc.lpszClassName = _T("EnvelopeImage");

	RegisterClass(&wc);	
}

static void DrawEnvelopeImage(KbdDeviceBinding* b,HWND hWnd,HDC hdc)
{
	Rect rect, orect;

	if (!b) return;

	GetClientRect(hWnd,&rect);

	orect = rect;

	rect.top += 4;
	rect.left += 4;
	rect.right -= 4;
	rect.bottom -= 4;

	float ySize = (float)(rect.bottom-rect.top);
	float MaxLevel = (rect.right-rect.left) / 4.0f;

	int Attack = (int)(MaxLevel*b->attack) + rect.left;
	int Decay = (int)(MaxLevel*b->decay + Attack);
	int Sustain = (int)(ySize - ySize * b->sustain + rect.top);
	int Release = (int)(rect.right - MaxLevel*b->release);

	SelectObject(hdc,CreatePen(PS_SOLID,0,RGB(10, 10, 10)));
	MoveToEx(hdc, rect.left, rect.bottom, NULL);
	LineTo(hdc, Attack, rect.top);
	LineTo(hdc, Decay, Sustain);
	LineTo(hdc, Release, Sustain);
	LineTo(hdc, rect.right, rect.bottom);

	DeleteObject(SelectObject(hdc,GetStockObject(BLACK_PEN)));

	WhiteRect3D(hdc,orect,TRUE);
}

static LRESULT CALLBACK EnvelopeImageWinProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	IMCParamDlg *dlg = (IMCParamDlg*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	KbdDeviceBinding *b;
	if (dlg) {
		b = (KbdDeviceBinding*)dlg->binding;
	}

	switch (msg) {
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd,&ps);
			DrawEnvelopeImage(b,hWnd,hdc);
			EndPaint(hWnd,&ps);
			break;
			}

		default:
			return DefWindowProc(hWnd,msg,wParam,lParam);
	}
	
	return 0;
}


//--- Key Assignment ---------------------------------------------

static int vKey, kFlags;
static WNDPROC oldButtonProc;

BOOL CALLBACK newButtonProc(HWND hButton, UINT iMsg, UINT wParam, LONG lParam)
{
	switch(iMsg) {
	case WM_CREATE:
		kFlags = vKey = 0;
		break;
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		switch(wParam) {
		case VK_NUMLOCK:
		case VK_CAPITAL:
		case VK_CLEAR:
		case VK_SCROLL:
		case VK_PAUSE:
		case VK_F1:
		case VK_BACK:
		case VK_DELETE:
			return 0;
		case VK_MENU:
			kFlags |= FALT;
			return 0;
		case VK_SHIFT:
			kFlags |= FSHIFT;
			return 0;
		case VK_CONTROL:
			kFlags |= FCONTROL;
			return 0;
		default:
			if(GetKeyState(VK_MENU) & 0x8000)
				kFlags |= FALT;
			vKey = wParam;
			SendMessage(GetParent(hButton), WM_COMMAND, IDOK, 0);
		}
		return 0;
	case WM_SYSKEYUP:
	case WM_KEYUP:
		switch(wParam) {
		case VK_MENU:
			kFlags &= ~FALT;
			return 0;
		case VK_SHIFT:
			kFlags &= ~FSHIFT;
			return 0;
		case VK_CONTROL:
			kFlags &= ~FCONTROL;
			return 0;
		}
		return 0;
	case WM_SYSCHAR:
	case WM_SYSCOMMAND:
		return 0;
	}
	return CallWindowProc(oldButtonProc, hButton, iMsg, wParam, lParam);
}

INT_PTR CALLBACK KeyPressProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    switch (iMsg)  {
		case WM_INITDIALOG:
			CenterWindow(hDlg, GetWindow(hDlg, GW_OWNER));
			oldButtonProc = (WNDPROC)GetWindowLongPtr(GetDlgItem(hDlg, ID_FILE_EXIT), GWLP_WNDPROC);
			SetWindowLongPtr(GetDlgItem(hDlg, ID_FILE_EXIT), GWLP_WNDPROC, (LONG_PTR)newButtonProc);
			SetFocus(GetDlgItem(hDlg, ID_FILE_EXIT));
			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam))  {
			case IDOK:
				EndDialog(hDlg, TRUE);
				break;
			case ID_FILE_EXIT:
				EndDialog(hDlg, FALSE);
				break;
			case IDCANCEL:
				EndDialog(hDlg, FALSE);
				break;
			}
			break;
    }
    return FALSE;
}



void KbdDeviceBinding::AssignKey(HWND hParent)
{
	if (DialogBox(hInstance, MAKEINTRESOURCE(IDD_KBD_KEYPRESSDLG), hParent , KeyPressProc)) {
		nVKey = vKey;
		SendMessage(GetDlgItem(hParent, IDC_KBD_KEYEDIT), WM_SETTEXT, 0, (LPARAM)(char*)BindingName());
	}
}


//--- Public functions ---------------------------------------------


BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
	hInstance = hinstDLL;
	if ( !controlsInit ) {
		controlsInit = TRUE;
		InitCustomControls(hInstance);
		InitCommonControls();
	}
	return(TRUE);
}

__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_LIBDESCRIPTION); }

__declspec( dllexport ) int LibNumberClasses() {return 2;}

__declspec( dllexport ) ClassDesc*
LibClassDesc(int i) {
	switch(i) {
		case 0: return GetKbdDeviceClassDesc();
		case 1: return GetKbdBindingClassDesc();
		default: return 0;
	}
}


__declspec( dllexport ) ULONG 
LibVersion() { return VERSION_3DSMAX; }

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if(hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}
