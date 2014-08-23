//***************************************************************************
//* Audio Amplitude Float Controller for 3D Studio MAX.
//* 
//* Code & design by Christer Janson
//* Autodesk European Developer Support - Neuchatel Switzerland
//* December 1995
//*

#include "auctrl.h"

extern HINSTANCE hInstance;
extern BOOL GetSoundFileName(HWND hWnd,TSTR &name,TSTR &dir); // CoreExport
extern INT_PTR CALLBACK AboutBoxDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);

class AudioFloatDlg;

class AudioFloatControl : public AudioBaseControl {
public:
	Class_ID ClassID() { return Class_ID(AUDIO_FLOAT_CONTROL_CLASS_ID1, AUDIO_FLOAT_CONTROL_CLASS_ID2); }  
	SClass_ID SuperClassID() { return CTRL_FLOAT_CLASS_ID; } 
	void GetClassName(TSTR& s) {s = AUDIO_FLOAT_CONTROL_CNAME;}

	// Remember the dialog object, we may need to close it
	AudioFloatDlg* pDlg;

	AudioFloatControl();
	~AudioFloatControl();

	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);

	// Reference methods
	RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage) {return REF_SUCCEED;}
	void RefDeleted();
	RefTargetHandle Clone(RemapDir& remap);
	void Copy(Control *from);

	void EditTrackParams(TimeValue t, ParamDimensionBase *dim, TCHAR *pname,
		HWND hParent, IObjParam *ip, DWORD flags);

	void *CreateTempValue() {return new float;}
	void DeleteTempValue(void *val) {delete (float*)val;}
	void ApplyValue(void *val, void *delta) {*((float*)val) += *((float*)delta);}
	void MultiplyValue(void *val, float m) {*((float*)val) *= m;}

	void GetValueLocalTime(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
	void SetValueLocalTime(TimeValue t, void *val, int commit, GetSetMethod method);
};

class AudioFloatClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new AudioFloatControl(); }
	const TCHAR *	ClassName() { return AUDIO_FLOAT_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_FLOAT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(AUDIO_FLOAT_CONTROL_CLASS_ID1,AUDIO_FLOAT_CONTROL_CLASS_ID2); }
	const TCHAR* 	Category() { return _T("");  }
};

static AudioFloatClassDesc floatAudioCD;

ClassDesc* GetAudioFloatDesc()
{
	return &floatAudioCD;
}

static INT_PTR CALLBACK AudioFloatDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class AudioFloatDlg : public ReferenceMaker {
	public:
		AudioFloatControl *cont;
		ParamDimensionBase *dim;
		IObjParam *ip;
		HWND hWnd;
		BOOL valid;

		ISpinnerControl *iMin;
		ISpinnerControl *iMax;
		ISpinnerControl *iSamples;
		ISpinnerControl *iThreshold;

		AudioFloatDlg(AudioFloatControl *cont, ParamDimensionBase *dim, IObjParam *ip, HWND hParent);
		~AudioFloatDlg();

		void Update();
		void SetupUI(HWND hWnd);
		void Change();
		void WMCommand(int id, int notify, HWND hCtrl);
		void SpinnerChange(int id,BOOL drag);
		void SpinnerStart(int id);
		void SpinnerEnd(int id,BOOL cancel);
		void EnableChannelUI(BOOL state);

		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);
		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i) {return cont;}
		void SetReference(int i, RefTargetHandle rtarg) {cont=(AudioFloatControl*)rtarg;}

		void SetActive();
		void ShowAbout(HWND hWnd);
};

AudioFloatDlg::AudioFloatDlg(AudioFloatControl *cont, ParamDimensionBase *dim,
	IObjParam *ip, HWND hParent)
{
	this->cont = cont;
	this->ip   = ip;
	this->dim  = dim;
	valid = FALSE;
	MakeRefByID(FOREVER,0,cont);
	hWnd = CreateDialogParam(
		hInstance,
		MAKEINTRESOURCE(IDD_AUDIOFLOATPARAMS),
		hParent,
		AudioFloatDlgProc,
		(LPARAM)this);
}

AudioFloatDlg::~AudioFloatDlg()
{
	DeleteAllRefsFromMe();
	ReleaseISpinner(iMin);
	ReleaseISpinner(iMax);
	ReleaseISpinner(iSamples);
	ReleaseISpinner(iThreshold);
}

void AudioFloatDlg::SetActive()
{
	SetActiveWindow(hWnd);
}

void AudioFloatDlg::EnableChannelUI(BOOL state)
{
	EnableWindow(GetDlgItem(hWnd, IDC_AUDAMP_LEFT), state);
	EnableWindow(GetDlgItem(hWnd, IDC_AUDAMP_RIGHT), state);
	EnableWindow(GetDlgItem(hWnd, IDC_AUDAMP_MIX), state);
}

void AudioFloatDlg::Update()
{
	if (!valid && hWnd) {
		iMin->SetValue(dim->Convert(cont->min),FALSE);
		iMax->SetValue(dim->Convert(cont->max),FALSE);
		iSamples->SetValue(cont->numsamples,FALSE);
		iThreshold->SetValue(cont->threshold,FALSE);
		if (!cont->szFilename.isNull())
			SendMessage(GetDlgItem(hWnd, IDC_AUDAMP_FILENAME), WM_SETTEXT, 0, (LPARAM)(char *)cont->szFilename);
		else
			SendMessage(GetDlgItem(hWnd, IDC_AUDAMP_FILENAME), WM_SETTEXT, 0, (LPARAM)GetString(IDS_CJ_NOFILE));
		CheckDlgButton(hWnd, IDC_AUDAMP_ABSOLUTE, cont->absolute == TRUE ? 1 : 0);
		CheckDlgButton(hWnd, IDC_QUICKDRAW, cont->quickdraw == TRUE ? 1 : 0);
		EnableChannelUI(cont->wave->GetNumChannels() == 2);

		switch (cont->channel) {
			case 0:
				CheckRadioButton(hWnd, IDC_AUDAMP_LEFT, IDC_AUDAMP_MIX, IDC_AUDAMP_LEFT);
				break;
			case 1:
				CheckRadioButton(hWnd, IDC_AUDAMP_LEFT, IDC_AUDAMP_MIX, IDC_AUDAMP_RIGHT);
				break;
			case 2:
				CheckRadioButton(hWnd, IDC_AUDAMP_LEFT, IDC_AUDAMP_MIX, IDC_AUDAMP_MIX);
				break;
		}
		valid = TRUE;
	}
}

// Called to initialize the dialog box
void AudioFloatDlg::SetupUI(HWND hWnd)
{
	this->hWnd = hWnd;

	// Get hold of, and setup, any available recording device.
	if (cont->rtwave->m_nNumDevices > 0) {
		for (int i = 0; i< cont->rtwave->m_nNumDevices; i++) {
			SendMessage(GetDlgItem(hWnd, IDC_AUDAMP_RECDEVICE), CB_ADDSTRING, 0, (LPARAM)cont->rtwave->m_pDevCaps[i].szPname);
		}
	}
	else {
		SendMessage(GetDlgItem(hWnd, IDC_AUDAMP_RECDEVICE), CB_ADDSTRING, 0, (LPARAM)GetString(IDS_CJ_NODEVICE));
		EnableWindow(GetDlgItem(hWnd, IDC_AUDAMP_RECDEVICE), 0);
		EnableWindow(GetDlgItem(hWnd, IDC_AUDAMP_RECENABLE), 0);
	}

	// Set recording device to first item (Will be "none" if no device)
	SendMessage(GetDlgItem(hWnd, IDC_AUDAMP_RECDEVICE), CB_SETCURSEL, 0, 0);
	cont->rtwave->SetDevice(0);

	// If we are currently recording we check the box
	if (cont->rtwave->IsRecording()) {
		SendMessage(GetDlgItem(hWnd, IDC_AUDAMP_RECDEVICE), CB_SETCURSEL, cont->rtwave->GetDevice(), 0);
		EnableWindow(GetDlgItem(hWnd, IDC_AUDAMP_RECDEVICE), 0);
		CheckDlgButton(hWnd, IDC_AUDAMP_RECENABLE, 1);
	}

	// Setup limits and type of the spinners.
	iMin = GetISpinner(GetDlgItem(hWnd,IDC_AUDAMP_MINSPIN));
	iMin->SetLimits(-9999999,9999999,FALSE);
	iMin->SetAutoScale();
	iMin->LinkToEdit(GetDlgItem(hWnd,IDC_AUDAMP_MIN),EDITTYPE_FLOAT);		

	iMax = GetISpinner(GetDlgItem(hWnd,IDC_AUDAMP_MAXSPIN));
	iMax->SetLimits(-9999999,9999999,FALSE);
	iMax->SetAutoScale();
	iMax->LinkToEdit(GetDlgItem(hWnd,IDC_AUDAMP_MAX),EDITTYPE_FLOAT);		

	iSamples = GetISpinner(GetDlgItem(hWnd,IDC_AUDAMP_NUMSAMPLESSPIN));
	iSamples->SetLimits(1,1000,FALSE);
	iSamples->SetAutoScale();
	iSamples->LinkToEdit(GetDlgItem(hWnd,IDC_AUDAMP_NUMSAMPLES),EDITTYPE_INT);

	iThreshold = GetISpinner(GetDlgItem(hWnd,IDC_AUDAMP_THRESHOLDSPIN));
	iThreshold->SetLimits(0,1,FALSE);
	iThreshold->SetAutoScale();
	iThreshold->LinkToEdit(GetDlgItem(hWnd,IDC_AUDAMP_THRESHOLD),EDITTYPE_FLOAT);

	valid = FALSE;
	Update();
}

// Handle WM_COMMAND messages separately
void AudioFloatDlg::WMCommand(int id, int notify, HWND hCtrl)
{
	int mmerr;

	switch (id) {
		case IDC_AUDAMP_ABSOLUTE:
			cont->absolute = IsDlgButtonChecked(hWnd,id);
			Change();
			break;
		case IDC_QUICKDRAW:
			cont->quickdraw = IsDlgButtonChecked(hWnd,id);
			Change();
			break;
		case IDC_AUDAMP_RECDEVICE:
			if (notify == CBN_SELCHANGE) {
				int devIdx = SendMessage(GetDlgItem(hWnd, IDC_AUDAMP_RECDEVICE), CB_GETCURSEL, 0, 0);
				cont->rtwave->SetDevice(devIdx);
				Change();
			}
			break;
		case IDC_AUDAMP_RECENABLE:
			cont->enableRuntime = IsDlgButtonChecked(hWnd,id);
			// Startup or terminate runtime recording
			if (cont->enableRuntime) {
				if ((mmerr = cont->rtwave->StartRecording()) == 0) {
					EnableWindow(GetDlgItem(hWnd, IDC_AUDAMP_RECDEVICE), 0);
				}
				else {
					CheckDlgButton(hWnd, id, 0);
					MessageBox(hWnd, GetString(IDS_CJ_DEVICE_BUSY), GetString(IDS_CJ_PROGNAME), MB_OK);
				}
			} 
			else {
				cont->rtwave->StopRecording();
				EnableWindow(GetDlgItem(hWnd, IDC_AUDAMP_RECDEVICE), 1);
			}

			// Here we are just telling trackview to repaint its hierarchy.
			// We haven't actually changed cnything, but the switch from file to record
			// will cause a change in TrackView line height - and we need to tell that
			// to TrackView
			cont->NotifyDependents(FOREVER,PART_ALL,REFMSG_SUBANIM_STRUCTURE_CHANGED);

			Change();
			break;
		case IDC_BTN_BROWSE: {
				// Changed to open in the directory of the file. //CJ
				TSTR dir = cont->szFilename;
				if (dir.Length() == 0) {
					// Use sound dir if no file.
					dir = TSTR(ip->GetDir(APP_SOUND_DIR));
				}
				else {
					int lc = dir.last((int)'\\');
					if (lc == -1) {
						lc = dir.last((int)'/');
					}
					if (lc != -1) {
						dir[lc] = '\0';
					}
				}
				// Brings up file dialog box with sound "preview".
				GetSoundFileName(hCtrl, cont->szFilename, dir);

				if (!cont->szFilename.isNull()) {
					cont->wave->InitOpen(cont->szFilename);
					SendMessage(GetDlgItem(hWnd, IDC_AUDAMP_FILENAME), WM_SETTEXT, 0, (LPARAM)((char *)cont->szFilename));
					EnableChannelUI(cont->wave->GetNumChannels() == 2);
				}
				Change();
			}
			break;
		case IDC_BTN_REMOVESOUND:
			cont->wave->FreeSample();
			cont->szFilename = "";
			SendMessage(GetDlgItem(hWnd, IDC_AUDAMP_FILENAME), WM_SETTEXT, 0, (LPARAM)"");
			EnableChannelUI(FALSE);
			Change();
			break;
		case IDC_AUDAMP_LEFT:
			cont->channel = 0;
			Change();
			break;
		case IDC_AUDAMP_RIGHT:
			cont->channel = 1;
			Change();
			break;
		case IDC_AUDAMP_MIX:
			cont->channel = 2;
			Change();
			break;
		case IDC_ABOUT:
			ShowAbout(hWnd);
			break;
		case IDC_CLOSE:
			DestroyWindow(hWnd);
			break;
	}
}

// Handle spinners in the dialog box
void AudioFloatDlg::SpinnerChange(int id,BOOL drag)
{
	switch (id) {
		case IDC_AUDAMP_MINSPIN:
			cont->min = dim->UnConvert(iMin->GetFVal());
			Change();
			break;
		case IDC_AUDAMP_MAXSPIN:
			cont->max = dim->UnConvert(iMax->GetFVal());
			Change();
			break;
		case IDC_AUDAMP_NUMSAMPLESSPIN:
			cont->numsamples = iSamples->GetIVal();
			Change();
			break;
		case IDC_AUDAMP_THRESHOLDSPIN:
			cont->threshold = iThreshold->GetFVal();
			Change();
			break;
		}
	ip->RedrawViews(ip->GetTime());
	}
 
void AudioFloatDlg::SpinnerStart(int id)
{
}

void AudioFloatDlg::SpinnerEnd(int id,BOOL cancel)
{
	ip->RedrawViews(ip->GetTime());
}

void AudioFloatDlg::Change()
{
	cont->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	UpdateWindow(GetParent(hWnd));	
}

RefResult AudioFloatDlg::NotifyRefChanged(Interval changeInt, 
	RefTargetHandle hTarget, PartID& partID, RefMessage message)
{
	switch (message) {
		case REFMSG_CHANGE:
			break;
	}
	return REF_SUCCEED;
}

// Dialog box procedure for user interface dialog.
static INT_PTR CALLBACK AudioFloatDlgProc(HWND hWnd, UINT msg, WPARAM wParam,
	LPARAM lParam)
{
	AudioFloatDlg *dlg = (AudioFloatDlg*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch (msg) {
		case WM_INITDIALOG:
			dlg = (AudioFloatDlg*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			// Initialize spinners etc.
			dlg->SetupUI(hWnd);
			break;

		case CC_SPINNER_BUTTONDOWN:
			dlg->SpinnerStart(LOWORD(wParam));
			break;

		case CC_SPINNER_CHANGE:
			dlg->SpinnerChange(LOWORD(wParam),HIWORD(wParam));
			break;

		case CC_SPINNER_BUTTONUP:
			dlg->SpinnerEnd(LOWORD(wParam),!HIWORD(wParam));
			break;

		case WM_COMMAND:
			// Handle these separately
			dlg->WMCommand(LOWORD(wParam),HIWORD(wParam),(HWND)lParam);						
			break;

		case WM_PAINT:
			dlg->Update();
			return 0;			
		
		case WM_CLOSE:
			DestroyWindow(hWnd);			
			break;

		case WM_DESTROY:
			// The controller needs to know when the dialog object is deleted
			dlg->cont->pDlg = NULL;						
			delete dlg;
			break;
		
		default:
			return FALSE;
	}
	return TRUE;
}

void AudioFloatDlg::ShowAbout(HWND hWnd)	{
	DialogBoxParam(
		hInstance,
		MAKEINTRESOURCE(IDD_ABOUTBOX),
		hWnd,
		(DLGPROC)AboutBoxDlgProc,
		(LPARAM)this);
}

void AudioFloatControl::EditTrackParams(TimeValue t, ParamDimensionBase *dim,
	TCHAR *pname, HWND hParent, IObjParam *ip, DWORD flags)
{
	// Create and open the dialog box
	if (!pDlg)
		pDlg = new AudioFloatDlg(this,dim,ip,hParent);
	else 
		pDlg->SetActive();
}

AudioFloatControl::AudioFloatControl() 
{
	pDlg = NULL;
	type = AUDIO_FLOAT_CONTROL_CLASS_ID1;
	range = Interval(GetAnimStart(),GetAnimEnd());
	channel = 2;
	absolute = 0;
	min = 0.0f;
	max = 1.0f;
	numsamples = 1;
	threshold = 0.0f;
	quickdraw = 0;
} 

AudioFloatControl::~AudioFloatControl()
{
}


void AudioFloatControl::Copy(Control* from)
{
	float fval;

	if (from->ClassID() == ClassID()) {
		min = ((AudioFloatControl*)from)->min;
		max = ((AudioFloatControl*)from)->max;
	}
	else {
		from->GetValue(0, &fval, Interval(0,0));
		min = fval;
		max = fval;
	}
}


RefTargetHandle AudioFloatControl::Clone(RemapDir& remap)
{
	// make a new AudioFloat controller and give it our param values.
	// TBD: Param values!?
	AudioFloatControl *cont = new AudioFloatControl;
	// *cont = *this;
	cont->type = type;
	cont->range = range;
	cont->channel = channel;
	cont->absolute = absolute;
	cont->numsamples = numsamples;
	cont->enableRuntime = enableRuntime;
	cont->szFilename = szFilename;
	cont->quickdraw = quickdraw;
	cont->min = min;
	cont->max = max;


	BaseClone(this, cont, remap);
	return cont;
}

// When the last reference to a controller is
// deleted we need to close the realtime recording device and 
// its parameter dialog needs to be closed
void AudioFloatControl::RefDeleted()
{
	int c=0;
	RefListItem  *ptr = GetRefList().first;
	while (ptr) {
		if (ptr->maker!=NULL) {
			if (ptr->maker->SuperClassID()) c++;
		}
		ptr = ptr->next;
	}	
	if (!c) {
		// Stop the real-time recording is the object is deleted.
		if (rtwave->IsRecording())
			rtwave->StopRecording();

		if (pDlg != NULL)
			DestroyWindow(pDlg->hWnd);
	}
}


#define MIN_CHUNK			0x0100
#define MAX_CHUNK			0x0101
#define ABSOLUTE_CHUNK		0x0103
#define FILENAME_CHUNK		0x0104
#define NUMSAMPLES_CHUNK	0x0105
#define CHANNEL_CHUNK		0x0106
#define RANGE_CHUNK			0x0107
#define THRESHOLD_CHUNK		0x010A
#define QUICKDRAW_CHUNK		0x010C

// Save the controller data
IOResult AudioFloatControl::Save(ISave *isave)
{
	ULONG nb;

	Control::Save(isave);	// Handle ORT's

	isave->BeginChunk(MIN_CHUNK);
	isave->Write(&min,sizeof(min),&nb);
	isave->EndChunk();

	isave->BeginChunk(MAX_CHUNK);
	isave->Write(&max,sizeof(max),&nb);
	isave->EndChunk();

	isave->BeginChunk(ABSOLUTE_CHUNK);
	isave->Write(&absolute,sizeof(absolute),&nb);
	isave->EndChunk();

	isave->BeginChunk(CHANNEL_CHUNK);
	isave->Write(&channel,sizeof(channel),&nb);
	isave->EndChunk();

	isave->BeginChunk(FILENAME_CHUNK);
	isave->WriteWString((TCHAR*)szFilename);
	isave->EndChunk();

	isave->BeginChunk(NUMSAMPLES_CHUNK);
	isave->Write(&numsamples,sizeof(numsamples),&nb);
	isave->EndChunk();

	isave->BeginChunk(RANGE_CHUNK);
	isave->Write(&range,sizeof(range),&nb);
	isave->EndChunk();

	isave->BeginChunk(THRESHOLD_CHUNK);
	isave->Write(&threshold,sizeof(threshold),&nb);
	isave->EndChunk();

	isave->BeginChunk(QUICKDRAW_CHUNK);
	isave->Write(&quickdraw,sizeof(quickdraw),&nb);
	isave->EndChunk();

	return IO_OK;
}

// Load controller data
IOResult AudioFloatControl::Load(ILoad *iload)
{
	ULONG nb;
	IOResult res = IO_OK;

	Control::Load(iload);	// Handle ORT's

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case MIN_CHUNK:
				res=iload->Read(&min,sizeof(min),&nb);
				break;

			case MAX_CHUNK:
				res=iload->Read(&max,sizeof(max),&nb);
				break;

			case ABSOLUTE_CHUNK:
				res=iload->Read(&absolute,sizeof(absolute),&nb);
				break;

			case CHANNEL_CHUNK:
				res=iload->Read(&channel,sizeof(channel),&nb);
				break;

			case FILENAME_CHUNK: {
				wchar_t *buf = NULL;
				res=iload->ReadWStringChunk(&buf);
				szFilename = buf;
				if (!szFilename.isNull()) {
					if (FixupFilename(szFilename, iload->GetDir(APP_SOUND_DIR))) {
						// Initialize the WaveForm and load the audio stream
						wave->InitOpen(szFilename);
					}
				}
				break;
				}
			case NUMSAMPLES_CHUNK:
				res=iload->Read(&numsamples,sizeof(numsamples),&nb);
				break;

			case RANGE_CHUNK:
				res=iload->Read(&range,sizeof(range),&nb);
				break;
			case THRESHOLD_CHUNK:
				res=iload->Read(&threshold,sizeof(threshold),&nb);
				break;
			case QUICKDRAW_CHUNK:
				res=iload->Read(&quickdraw,sizeof(quickdraw),&nb);
				break;
		}

		iload->CloseChunk();
		if (res!=IO_OK)  return res;
	}
	return IO_OK;
}

// Return the value at a specific instant in time
void AudioFloatControl::GetValueLocalTime(TimeValue t, void *val, Interval &valid,
	GetSetMethod method)
{
	valid.SetInstant(t); // This controller is always changing.

	// Subtract start of range from this time to get the 'local' wave time.
	*((float*)val) = SampleAtTime(t - range.Start(), 0, FALSE);
}

// TBD: Not appropriate for this controller.
void AudioFloatControl::SetValueLocalTime(TimeValue t, void *val, int commit, GetSetMethod method)
{
}

BOOL CALLBACK AboutDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)	{
	switch (message) {
		case WM_INITDIALOG:
			CenterWindow(hWnd,GetParent(hWnd));
			SetCursor(LoadCursor(NULL,IDC_ARROW));
			return 1;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:				  
				case IDCANCEL:
					EndDialog(hWnd,1);
					break;
			}
			return 1;
	}
	return 0;
}
