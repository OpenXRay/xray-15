//***************************************************************************
//* Audio Amplitude Controller for 3D Studio MAX.
//* Dialogue box class for "3 point" controllers
//* 
//* Code & design by Christer Janson
//* Autodesk European Developer Support - Neuchatel Switzerland
//* December 1995
//*
#include "auctrl.h"
#include "aup3base.h"
#include "aup3dlg.h"

extern HINSTANCE hInstance;
extern BOOL GetSoundFileName(HWND hWnd,TSTR &name,TSTR &dir); // CoreExport

AudioP3Dlg::AudioP3Dlg(AudioP3Control *cont, ParamDimensionBase *dim,
	IObjParam *ip, HWND hParent)
{
	this->cont = cont;
	this->ip   = ip;
	this->dim  = dim;

	valid = FALSE;
	// Create a reference to the controller
	MakeRefByID(FOREVER,0,cont);
	// Create the dialog box
	hWnd = CreateDialogParam(
		hInstance,
		MAKEINTRESOURCE(IDD_AUDIOPOINT3PARAMS),
		hParent,
		AudioP3DlgProc,
		(LPARAM)this);
}

AudioP3Dlg::~AudioP3Dlg()
{
	DeleteAllRefsFromMe();
	ReleaseISpinner(iBaseX);
	ReleaseISpinner(iBaseY);
	ReleaseISpinner(iBaseZ);
	ReleaseISpinner(iTargetX);
	ReleaseISpinner(iTargetY);
	ReleaseISpinner(iTargetZ);
	ReleaseISpinner(iSamples);
	ReleaseISpinner(iThreshold);
}

// Helper for user interface. Enable channel for stereo sound
void AudioP3Dlg::EnableChannelUI(BOOL state)
{
	EnableWindow(GetDlgItem(hWnd, IDC_AUDAMP_LEFT), state);
	EnableWindow(GetDlgItem(hWnd, IDC_AUDAMP_RIGHT), state);
	EnableWindow(GetDlgItem(hWnd, IDC_AUDAMP_MIX), state);
}

void AudioP3Dlg::SetActive()
{
	SetActiveWindow(hWnd);
}

void AudioP3Dlg::Update()
{
	if (!valid && hWnd) {
		iBaseX->SetValue(dim->Convert(cont->basePoint.x),FALSE);
		iBaseY->SetValue(dim->Convert(cont->basePoint.y),FALSE);
		iBaseZ->SetValue(dim->Convert(cont->basePoint.z),FALSE);
		iTargetX->SetValue(dim->Convert(cont->targetPoint.x),FALSE);
		iTargetY->SetValue(dim->Convert(cont->targetPoint.y),FALSE);
		iTargetZ->SetValue(dim->Convert(cont->targetPoint.z),FALSE);
		iSamples->SetValue(cont->numsamples,FALSE);
		iThreshold->SetValue(cont->threshold,FALSE);
		if (!cont->szFilename.isNull())
			SendMessage(GetDlgItem(hWnd, IDC_AUDAMP_FILENAME), WM_SETTEXT, 0, (LPARAM)(char *)cont->szFilename);
		else
			SendMessage(GetDlgItem(hWnd, IDC_AUDAMP_FILENAME), WM_SETTEXT, 0, (LPARAM)GetString(IDS_CJ_NOFILE));
		CheckDlgButton(hWnd, IDC_AUDAMP_ABSOLUTE, cont->absolute == TRUE ? 1 : 0);
		CheckDlgButton(hWnd, IDC_QUICKDRAW, cont->quickdraw == TRUE ? 1 : 0);
		// Enable channel UI if we are stereo
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

void AudioP3Dlg::SetupUI(HWND hWnd)
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

	// Change label if we are Scale or Rotation controllers
	if (cont->type == AUDIO_SCALE_CONTROL_CLASS_ID1) {
		SendMessage(GetDlgItem(hWnd, IDC_BASEFRAME), WM_SETTEXT, 0, (LPARAM)GetString(IDS_CJ_BASESCALE));
		SendMessage(GetDlgItem(hWnd, IDC_TARGETFRAME), WM_SETTEXT, 0, (LPARAM)GetString(IDS_CJ_TARGETSCALE));
	}
	else if (cont->type == AUDIO_ROTATION_CONTROL_CLASS_ID1) {
		SendMessage(GetDlgItem(hWnd, IDC_BASEFRAME), WM_SETTEXT, 0, (LPARAM)GetString(IDS_CJ_BASEANGLE));
		SendMessage(GetDlgItem(hWnd, IDC_TARGETFRAME), WM_SETTEXT, 0, (LPARAM)GetString(IDS_CJ_TARGETANGLE));
	}

	// Setup limits and type of the spinners.
	iBaseX = GetISpinner(GetDlgItem(hWnd,IDC_AUDAMP_BASEXSPIN));
	iBaseX->SetLimits(-9999999,9999999,FALSE);
	iBaseX->SetAutoScale();
	iBaseX->LinkToEdit(GetDlgItem(hWnd,IDC_AUDAMP_BASEX),EDITTYPE_FLOAT);		

	iBaseY = GetISpinner(GetDlgItem(hWnd,IDC_AUDAMP_BASEYSPIN));
	iBaseY->SetLimits(-9999999,9999999,FALSE);
	iBaseY->SetAutoScale();
	iBaseY->LinkToEdit(GetDlgItem(hWnd,IDC_AUDAMP_BASEY),EDITTYPE_FLOAT);		

	iBaseZ = GetISpinner(GetDlgItem(hWnd,IDC_AUDAMP_BASEZSPIN));
	iBaseZ->SetLimits(-9999999,9999999,FALSE);
	iBaseZ->SetAutoScale();
	iBaseZ->LinkToEdit(GetDlgItem(hWnd,IDC_AUDAMP_BASEZ),EDITTYPE_FLOAT);		

	iTargetX = GetISpinner(GetDlgItem(hWnd,IDC_AUDAMP_TARGETXSPIN));
	iTargetX->SetLimits(-9999999,9999999,FALSE);
	iTargetX->SetAutoScale();
	iTargetX->LinkToEdit(GetDlgItem(hWnd,IDC_AUDAMP_TARGETX),EDITTYPE_FLOAT);		

	iTargetY = GetISpinner(GetDlgItem(hWnd,IDC_AUDAMP_TARGETYSPIN));
	iTargetY->SetLimits(-9999999,9999999,FALSE);
	iTargetY->SetAutoScale();
	iTargetY->LinkToEdit(GetDlgItem(hWnd,IDC_AUDAMP_TARGETY),EDITTYPE_FLOAT);		

	iTargetZ = GetISpinner(GetDlgItem(hWnd,IDC_AUDAMP_TARGETZSPIN));
	iTargetZ->SetLimits(-9999999,9999999,FALSE);
	iTargetZ->SetAutoScale();
	iTargetZ->LinkToEdit(GetDlgItem(hWnd,IDC_AUDAMP_TARGETZ),EDITTYPE_FLOAT);		

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

// Filter for the TrackView picker
class VectorFilter : public TrackViewFilter {
public:
	BOOL proc(Animatable *anim, Animatable *client,int subNum)
	{
		return anim->SuperClassID() == CTRL_POSITION_CLASS_ID ||
				anim->SuperClassID() == CTRL_POINT3_CLASS_ID; 
	}
};

// Handle WM_COMMAND messages separately
void AudioP3Dlg::WMCommand(int id, int notify, HWND hCtrl)
{
	int mmerr;
	AudioP3Control* af = (AudioP3Control *)GetWindowLongPtr(hWnd, DWLP_USER);

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
			// We haven't actually changed anything, but the switch from file to record
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
void AudioP3Dlg::SpinnerChange(int id,BOOL drag)
{
	switch (id) {
		case IDC_AUDAMP_BASEXSPIN:
			cont->basePoint.x = dim->UnConvert(iBaseX->GetFVal());
			Change();
			break;
		case IDC_AUDAMP_BASEYSPIN:
			cont->basePoint.y = dim->UnConvert(iBaseY->GetFVal());
			Change();
			break;
		case IDC_AUDAMP_BASEZSPIN:
			cont->basePoint.z = dim->UnConvert(iBaseZ->GetFVal());
			Change();
			break;
		case IDC_AUDAMP_TARGETXSPIN:
			cont->targetPoint.x = dim->UnConvert(iTargetX->GetFVal());
			Change();
			break;
		case IDC_AUDAMP_TARGETYSPIN:
			cont->targetPoint.y = dim->UnConvert(iTargetY->GetFVal());
			Change();
			break;
		case IDC_AUDAMP_TARGETZSPIN:
			cont->targetPoint.z = dim->UnConvert(iTargetZ->GetFVal());
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
 
void AudioP3Dlg::SpinnerStart(int id)
{
}

void AudioP3Dlg::SpinnerEnd(int id,BOOL cancel)
{
	ip->RedrawViews(ip->GetTime());
}

// Update viewports after parameters have changed
void AudioP3Dlg::Change()
{
	cont->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	UpdateWindow(GetParent(hWnd));	
}

RefResult AudioP3Dlg::NotifyRefChanged(Interval changeInt, 
	RefTargetHandle hTarget, PartID& partID, RefMessage message)
{
	switch (message) {
		case REFMSG_CHANGE:
			break;
	}
	return REF_SUCCEED;
}

// Dialog box procedure for user interface dialog.
static INT_PTR CALLBACK AudioP3DlgProc(HWND hWnd, UINT msg, WPARAM wParam,
	LPARAM lParam)
{
	AudioP3Dlg *dlg = (AudioP3Dlg*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch (msg) {
		case WM_INITDIALOG:
			dlg = (AudioP3Dlg*)lParam;
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
			// The controller needs to know when the dialogue is deleted
			dlg->cont->pDlg = NULL;						
			delete dlg;
			break;
		
		default:
			return FALSE;
	}
	return TRUE;
}

void AudioP3Control::EditTrackParams(TimeValue t, ParamDimensionBase *dim,
	TCHAR *pname, HWND hParent, IObjParam *ip, DWORD flags)
{
	// Create and open the dialog box
	if (!pDlg)
		pDlg = new AudioP3Dlg(this,dim,ip,hParent);
	else
		pDlg->SetActive();
}	

INT_PTR CALLBACK AboutBoxDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
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

void AudioP3Dlg::ShowAbout(HWND hWnd)	{
	DialogBoxParam(
		hInstance,
		MAKEINTRESOURCE(IDD_ABOUTBOX),
		hWnd,
		(DLGPROC)AboutBoxDlgProc,
		(LPARAM)this);
}