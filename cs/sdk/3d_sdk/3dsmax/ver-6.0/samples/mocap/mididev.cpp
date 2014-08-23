/**********************************************************************
 *<
	FILE: mididev.cpp

	DESCRIPTION: A midi device for motion capture

	CREATED BY: Rolf Berteig

	HISTORY: November 6, 1996

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#include "motion.h"
//#include "mmanager.h"
#include "mcapdev.h"
#include "midiman.h"

static void MIDIViewerDialog(HWND hParent);
static MCInputDevice *GetMidiDevice();

#define MIDI_DEVICE_CLASS_ID	0x84d63b45

#define NUM_MIDI_CHANNELS	16
#define NUM_MIDI_NOTES		132

// Macros to pull out peices of param1
#define MIDI_CHANNEL(a)				((a)&0x0f)
#define MIDI_EVENT(a)				((a)&0xf0)
#define MIDI_NOTENUMBER(a)			(((a)&0xff00)>>8)
#define MIDI_VELOCITY(a)			(((a)&0xff0000)>>16)
#define MIDI_PITCHBEND(a)			(((a)&0xff0000)>>16)
#define MIDI_NOTEFLOAT(a,low,high)	(float((a)-(low))/float((high)-(low)))
#define MIDI_VELFLOAT(a)            (float(a)/127.0f)
#define MIDI_BENDFLOAT(a)			(float(a)/127.0f)

// MIDI events
#define MIDI_NOTE_ON		0x90
#define MIDI_NOTE_OFF		0x80
#define MIDI_PITCH_BEND		0xe0
#define MIDI_CONTROLCHANGE	0xb0

#define SPEED_UI_FACT 0.005f


class MidiDeviceBinding : public MCDeviceBinding {
	public:
		int channel, trigger, lowNote, highNote, sustain, variSustain;
		int controlNum;
		float min, max, speed;
		float curVal;		

		MidiDeviceBinding();
		MCInputDevice *GetDevice() {return GetMidiDevice();}
		TSTR BindingName() {return GetString(IDS_RB_MIDI);}
		float Eval(TimeValue t);
		void Accumulate(TimeValue t);
		void DeleteThis() {delete this;}
		Class_ID ClassID() {return Class_ID(MIDI_DEVICE_CLASS_ID,0);}
		RefTargetHandle Clone(RemapDir& remap);

		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		void AddRollup(IMCParamDlg *dlg);
		void UpdateRollup(IRollupWindow *iRoll);
		void BeginActivate(BOOL reset);
		void EndActivate();

		void InitDlg(HWND hWnd);
		void SetupNote(HWND hCombo);
		void SetupOctave(HWND hCombo);
		void SetEnabledStates(HWND hWnd);

		void SetTrigger(int t) {trigger=t;}
		void SetChannel(int c) {channel=c;}
		void SetLowNote(int n) {lowNote=n;}
		void SetHighNote(int n) {highNote=n;}
		void SetMin(float m) {min=m;}
		void SetMax(float m) {max=m;}
		void SetSpeed(float s) {speed=s;}
		void SetSustain(int s) {sustain=s;}
		void SetVariSustain(int v) {variSustain=v;}
		void SetControlNum(int n) {controlNum=n;}

		int GetNoteVal(HWND hWnd, int idNote, int idOctave);
	};

class ChannelData {
	public:
		ChannelData();
		BOOL pressed[NUM_MIDI_NOTES];
		TimeValue time[NUM_MIDI_NOTES];
		int vel[NUM_MIDI_NOTES];
		int bend;
	};

class MidiDevice : public MCInputDevice {
	public:
		ChannelData data[NUM_MIDI_CHANNELS];
		BOOL active;
		//HMIDIIN hMidiIn;		
		IMCapManager *theMM;
		BOOL noDevice;

		MidiDevice();

		TSTR DeviceName() {return GetString(IDS_RB_MIDI);}
		MCDeviceBinding *CreateBinding() {return new MidiDeviceBinding;}

		void MidiEvent(DWORD param);
		void Open();
		void Close();
		void Start();
		void Stop();

		void UtilityStarted(IMCapManager *im);
		void UtilityStopped(IMCapManager *im);

		int FirstKeyDown(int channel, int low, int high);
		int NewestKeyDown(int channel, int low, int high);
	};

//--- Class Descriptor -----------------------------------------------

// This is only here to support old files. Before the motion capture
// device interface was made plug-able the general REF_TARGET_CLASS_ID was used.
class MidiDeviceClassDescOld:public ClassDesc {
	public:
	int 			IsPublic() {return 0;}
	void *			Create(BOOL loading) {return new MidiDeviceBinding;}
	const TCHAR *	ClassName() {return GetString(IDS_RB_MIDI);}
	SClass_ID		SuperClassID() {return REF_TARGET_CLASS_ID;}
	Class_ID		ClassID() {return Class_ID(MIDI_DEVICE_CLASS_ID,0);}
	const TCHAR* 	Category() {return _T("");}
	};
static MidiDeviceClassDescOld midiCDOld;
ClassDesc* GetMidiDeviceClassDescDescOld() {return &midiCDOld;}

class MidiDeviceClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 0;}
	void *			Create(BOOL loading) {return new MidiDeviceBinding;}
	const TCHAR *	ClassName() {return GetString(IDS_RB_MIDI);}
	SClass_ID		SuperClassID() {return MOT_CAP_DEVBINDING_CLASS_ID;}
	Class_ID		ClassID() {return Class_ID(MIDI_DEVICE_CLASS_ID,0);}
	const TCHAR* 	Category() {return _T("");}
	};
static MidiDeviceClassDesc midiCD;
ClassDesc* GetMidiDeviceClassDescDesc() {return &midiCD;}

class TheMidiDeviceClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading) {return GetMidiDevice();}
	const TCHAR *	ClassName() {return GetString(IDS_RB_MIDI);}
	SClass_ID		SuperClassID() {return MOT_CAP_DEV_CLASS_ID;}
	Class_ID		ClassID() {return Class_ID(MIDI_DEVICE_CLASS_ID,1);}
	const TCHAR* 	Category() {return _T("");}
	};
static TheMidiDeviceClassDesc theMidiCD;
ClassDesc* GetTheMidiDeviceClassDescDesc() {return &theMidiCD;}


//--- Midi device binding --------------------------------------------

static MidiDevice theMidiDevice;
static MCInputDevice *GetMidiDevice() {return &theMidiDevice;}


static INT_PTR CALLBACK MidiDeviceDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


MidiDeviceBinding::MidiDeviceBinding()
	{	
	channel     = 0;
	trigger     = IDC_MIDI_NOTE;
	lowNote     = 0;
	highNote    = NUM_MIDI_NOTES-1;
	min         = 0.0f;
	max         = 1.0f;
	speed       = 10.0f * SPEED_UI_FACT;
	sustain     = 0;
	variSustain = TRUE;	
	controlNum  = 7;
	}

#define CHANNEL_CHUNK_ID		0x0100
#define TRIGGER_CHUNK_ID		0x0110
#define LOWNOTE_CHUNK_ID		0x0120
#define HIGHNOTE_CHUNK_ID		0x0130
#define MIN_CHUNK_ID			0x0140
#define MAX_CHUNK_ID			0x0150
#define SPEED_CHUNK_ID			0x0160
#define SUSTAIN_CHUNK_ID		0x0170
#define VARISUSTAIN_CHUNK_ID	0x0180
#define CONTROLNUM_CHUNK_ID		0x0190

IOResult MidiDeviceBinding::Save(ISave *isave)
	{
	ULONG nb;

	isave->BeginChunk(CHANNEL_CHUNK_ID);
	isave->Write(&channel,sizeof(channel),&nb);
	isave->EndChunk();

	isave->BeginChunk(TRIGGER_CHUNK_ID);
	isave->Write(&trigger,sizeof(trigger),&nb);
	isave->EndChunk();
	
	isave->BeginChunk(LOWNOTE_CHUNK_ID);
	isave->Write(&lowNote,sizeof(lowNote),&nb);
	isave->EndChunk();
	
	isave->BeginChunk(HIGHNOTE_CHUNK_ID);
	isave->Write(&highNote,sizeof(highNote),&nb);
	isave->EndChunk();
	
	isave->BeginChunk(MIN_CHUNK_ID);
	isave->Write(&min,sizeof(min),&nb);
	isave->EndChunk();
	
	isave->BeginChunk(MAX_CHUNK_ID);
	isave->Write(&max,sizeof(max),&nb);
	isave->EndChunk();

	isave->BeginChunk(SPEED_CHUNK_ID);
	isave->Write(&speed,sizeof(speed),&nb);
	isave->EndChunk();

	isave->BeginChunk(SUSTAIN_CHUNK_ID);
	isave->Write(&sustain,sizeof(sustain),&nb);
	isave->EndChunk();

	isave->BeginChunk(VARISUSTAIN_CHUNK_ID);
	isave->Write(&variSustain,sizeof(variSustain),&nb);
	isave->EndChunk();

	isave->BeginChunk(CONTROLNUM_CHUNK_ID);
	isave->Write(&controlNum, sizeof(controlNum),&nb);
	isave->EndChunk();

	return IO_OK;
	}

IOResult MidiDeviceBinding::Load(ILoad *iload)
	{
	ULONG nb;
	IOResult res = IO_OK;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case CHANNEL_CHUNK_ID:
				res=iload->Read(&channel,sizeof(channel),&nb);
				break;

			case TRIGGER_CHUNK_ID:
				res=iload->Read(&trigger,sizeof(trigger),&nb);
				break;

			case LOWNOTE_CHUNK_ID:
				res=iload->Read(&lowNote,sizeof(lowNote),&nb);
				break;

			case HIGHNOTE_CHUNK_ID:
				res=iload->Read(&highNote,sizeof(highNote),&nb);
				break;

			case MIN_CHUNK_ID:
				res=iload->Read(&min,sizeof(min),&nb);
				break;

			case MAX_CHUNK_ID:
				res=iload->Read(&max,sizeof(max),&nb);
				break;

			case SPEED_CHUNK_ID:
				res=iload->Read(&speed,sizeof(speed),&nb);
				break;

			case SUSTAIN_CHUNK_ID:
				res=iload->Read(&sustain,sizeof(sustain),&nb);
				break;

			case VARISUSTAIN_CHUNK_ID:
				res=iload->Read(&variSustain,sizeof(variSustain),&nb);
				break;

			case CONTROLNUM_CHUNK_ID:
				res=iload->Read(&controlNum,sizeof(controlNum),&nb);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}
	return IO_OK;
	}

RefTargetHandle MidiDeviceBinding::Clone(RemapDir& remap)
	{
	MidiDeviceBinding *b = new MidiDeviceBinding;
	b->channel     = channel;
	b->trigger     = trigger;
	b->lowNote     = lowNote;
	b->highNote    = highNote;
	b->sustain	   = sustain;
	b->variSustain = variSustain;
	b->min		   = min;
	b->max		   = max;
	b->speed	   = speed;
	b->curVal	   = curVal;
	BaseClone(this, b, remap);
	return b;
	}

void MidiDeviceBinding::BeginActivate(BOOL reset)
	{	
	//theMidiDevice.Start();
	if (reset) curVal = 0.0f;
	}

void MidiDeviceBinding::Accumulate(TimeValue t)
	{
	switch (trigger) {
		case IDC_MIDI_NOTE: {
			float newVal;
			int note = theMidiDevice.NewestKeyDown(channel, lowNote, highNote);			
			if (note>=0) {
				float u  = MIDI_NOTEFLOAT(note,lowNote,highNote);
				float v  = MIDI_VELFLOAT(theMidiDevice.data[channel].vel[note]);
				float dv = v * speed * (float)fabs(max-min);
				newVal = min*(1.0f-u) + max*u;
				if (newVal<curVal) {
					curVal -= dv;
					if (curVal<newVal) curVal=newVal;
					}
				if (newVal>curVal) {
					curVal += dv;
					if (curVal>newVal) curVal=newVal;
					}				
				}
			break;
			}
		}
	}

float MidiDeviceBinding::Eval(TimeValue t)
	{
	assert(theMidiDevice.theMM);

	switch (trigger) {
		case IDC_MIDI_NOTE: {
			return curVal;			
			}

		case IDC_MIDI_USER: {			
			float u = MIDI_VELFLOAT(theMidiDevice.data[channel].vel[controlNum]);
			float target = min*(1.0f-u) + max*u;
			if (sustain>0) {
				float ss = variSustain ? float(sustain) * u : sustain;
				float t0 = float(theMidiDevice.data[channel].time[controlNum]);
				float t1 = float(theMidiDevice.theMM->GetTime());
				if (t1>t0+ss) target = min;
				else {
					float v = 2.0f * (t1-t0)/float(ss) - 1.0f;
					target = (1.0f-v*v) * target;
					}
				}
			return target;
			}

		case IDC_MIDI_VEL: {						
			int note = theMidiDevice.NewestKeyDown(channel, lowNote, highNote);			
			if (note>=0) {				
				float u = MIDI_VELFLOAT(theMidiDevice.data[channel].vel[note]);				
				float target = min*(1.0f-u) + max*u;
				if (sustain>0) {
					float ss = variSustain ? float(sustain) * u : sustain;
					float t0 = float(theMidiDevice.data[channel].time[note]);
					float t1 = float(theMidiDevice.theMM->GetTime());
					if (t1>t0+ss) target = min;
					else {
						float v = 2.0f * (t1-t0)/float(ss) - 1.0f;
						target = (1.0f-v*v) * target;
						}
					}
				return target;
			} else {
				return min;
				}
			}

		case IDC_MIDI_BEND: {
			float u = MIDI_BENDFLOAT(theMidiDevice.data[channel].bend);
			return min*(1.0f-u) + max*u;
			break;
			}
		}
	return 0.0f;
	}

void MidiDeviceBinding::EndActivate()
	{		
	//theMidiDevice.Stop();
	}


void MidiDeviceBinding::AddRollup(IMCParamDlg *dlg)
	{
	dlg->iRoll->AppendRollup(
			hInstance, 
			MAKEINTRESOURCE(IDD_MC_MIDI), 
			MidiDeviceDlgProc, 
			GetString(IDS_RB_MIDI), 
			(LPARAM)dlg);
	}

void MidiDeviceBinding::UpdateRollup(IRollupWindow *iRoll)
	{
	if (iRoll->GetNumPanels()>1) {
		HWND hWnd = iRoll->GetPanelDlg(1);
		SendDlgItemMessage(hWnd, IDC_MIDI_LOWNOTE,    CB_SETCURSEL, lowNote %12, 0);
		SendDlgItemMessage(hWnd, IDC_MIDI_HIGHNOTE,   CB_SETCURSEL, highNote%12, 0);
		SendDlgItemMessage(hWnd, IDC_MIDI_LOWOCTAVE,  CB_SETCURSEL, lowNote /12, 0);
		SendDlgItemMessage(hWnd, IDC_MIDI_HIGHOCTAVE, CB_SETCURSEL, highNote/12, 0);
		
		for (int i=0; i<16; i++) {
			CheckDlgButton(hWnd, IDC_MIDI_CHAN1+i, FALSE);
			}
		CheckDlgButton(hWnd, IDC_MIDI_CHAN1+channel, TRUE);

		CheckDlgButton(hWnd, trigger, TRUE);
		CheckDlgButton(hWnd, IDC_SUSTAIN_VARIABLE, variSustain);

		ISpinnerControl *spin = GetISpinner(GetDlgItem(hWnd,IDC_MIDI_PARAMMINSPIN));
		spin->SetValue(min,FALSE);
		ReleaseISpinner(spin);

		spin = GetISpinner(GetDlgItem(hWnd,IDC_MIDI_PARAMMAXSPIN));	
		spin->SetValue(max,FALSE);
		ReleaseISpinner(spin);

		spin = GetISpinner(GetDlgItem(hWnd,IDC_MIDI_SPEEDSPIN));	
		spin->SetValue(speed/SPEED_UI_FACT,FALSE);
		ReleaseISpinner(spin);

		spin = GetISpinner(GetDlgItem(hWnd,IDC_MIDI_SUSTAINSPIN));	
		spin->SetValue(sustain,FALSE);
		ReleaseISpinner(spin);

		SetEnabledStates(hWnd);
		}
	}

void MidiDeviceBinding::SetupNote(HWND hCombo)
	{
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("C"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("C#"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("D"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("D#"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("E"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("F"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("F#"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("G"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("G#"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("A"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("A#"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("B"));
	}

void MidiDeviceBinding::SetupOctave(HWND hCombo)
	{
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("-2"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("-1"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("0"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("1"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("2"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("3"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("4"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("5"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("6"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("7"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("8"));	
	}

int MidiDeviceBinding::GetNoteVal(HWND hWnd, int idNote, int idOctave)
	{
	int note = SendDlgItemMessage(hWnd,idNote,CB_GETCURSEL,0,0);
	int octave = SendDlgItemMessage(hWnd,idOctave,CB_GETCURSEL,0,0);
	if (note<0) note = 0;
	if (octave<0) octave = 0;
	return octave * 12 + note;
	}

void MidiDeviceBinding::SetEnabledStates(HWND hWnd)
	{
	ISpinnerControl *speedSpin = GetISpinner(GetDlgItem(hWnd,IDC_MIDI_SPEEDSPIN));
	ISpinnerControl *sustSpin  = GetISpinner(GetDlgItem(hWnd,IDC_MIDI_SUSTAINSPIN));	
	ISpinnerControl *contSpin  = GetISpinner(GetDlgItem(hWnd,IDC_MIDI_USERNUMSPIN));	
	
	if (trigger==IDC_MIDI_NOTE || trigger==IDC_MIDI_BEND) {
		sustSpin->Disable();
		EnableWindow(GetDlgItem(hWnd, IDC_SUSTAIN_VARIABLE), FALSE);
	} else {
		sustSpin->Enable();
		EnableWindow(GetDlgItem(hWnd, IDC_SUSTAIN_VARIABLE), TRUE);
		}
	if (trigger==IDC_MIDI_VEL || trigger==IDC_MIDI_BEND || trigger==IDC_MIDI_USER) 
		 speedSpin->Disable();
	else speedSpin->Enable();
	
	if (trigger==IDC_MIDI_USER) {
		contSpin->Enable();
		EnableWindow(GetDlgItem(hWnd, IDC_MIDI_USERNUM_LABEL), TRUE);
	} else {
		contSpin->Disable();
		EnableWindow(GetDlgItem(hWnd, IDC_MIDI_USERNUM_LABEL), FALSE);
		}

	ReleaseISpinner(sustSpin);
	ReleaseISpinner(speedSpin);
	ReleaseISpinner(contSpin);

	if (trigger!=IDC_MIDI_BEND && trigger!=IDC_MIDI_USER) {
		EnableWindow(GetDlgItem(hWnd, IDC_NOTERANGE_LABEL1), TRUE);
		EnableWindow(GetDlgItem(hWnd, IDC_NOTERANGE_LABEL2), TRUE);
		EnableWindow(GetDlgItem(hWnd, IDC_NOTERANGE_LABEL3), TRUE);
		EnableWindow(GetDlgItem(hWnd, IDC_NOTERANGE_LABEL4), TRUE);
		EnableWindow(GetDlgItem(hWnd, IDC_NOTERANGE_LABEL5), TRUE);
		EnableWindow(GetDlgItem(hWnd, IDC_NOTERANGE_LABEL6), TRUE);
		EnableWindow(GetDlgItem(hWnd, IDC_NOTERANGE_LABEL7), TRUE);
		EnableWindow(GetDlgItem(hWnd, IDC_MIDI_LOWNOTE), TRUE);
		EnableWindow(GetDlgItem(hWnd, IDC_MIDI_LOWOCTAVE), TRUE);
		EnableWindow(GetDlgItem(hWnd, IDC_MIDI_HIGHNOTE), TRUE);
		EnableWindow(GetDlgItem(hWnd, IDC_MIDI_HIGHOCTAVE), TRUE);
	} else {
		EnableWindow(GetDlgItem(hWnd, IDC_NOTERANGE_LABEL1), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_NOTERANGE_LABEL2), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_NOTERANGE_LABEL3), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_NOTERANGE_LABEL4), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_NOTERANGE_LABEL5), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_NOTERANGE_LABEL6), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_NOTERANGE_LABEL7), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_MIDI_LOWNOTE), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_MIDI_LOWOCTAVE), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_MIDI_HIGHNOTE), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_MIDI_HIGHOCTAVE), FALSE);
		}
	}

void MidiDeviceBinding::InitDlg(HWND hWnd)
	{
	ISpinnerControl *spin = GetISpinner(GetDlgItem(hWnd,IDC_MIDI_PARAMMINSPIN));
	spin->SetLimits(-float(999999), float(999999), FALSE);
	spin->SetAutoScale();
	spin->LinkToEdit(GetDlgItem(hWnd,IDC_MIDI_PARAMMIN), EDITTYPE_FLOAT);
	spin->SetValue(min,FALSE);
	ReleaseISpinner(spin);

	spin = GetISpinner(GetDlgItem(hWnd,IDC_MIDI_PARAMMAXSPIN));
	spin->SetLimits(-float(999999), float(999999), FALSE);
	spin->SetAutoScale();
	spin->LinkToEdit(GetDlgItem(hWnd,IDC_MIDI_PARAMMAX), EDITTYPE_FLOAT);
	spin->SetValue(max,FALSE);
	ReleaseISpinner(spin);

	spin = GetISpinner(GetDlgItem(hWnd,IDC_MIDI_SPEEDSPIN));
	spin->SetLimits(0.0f, float(100.0f), FALSE);
	spin->SetAutoScale();
	spin->LinkToEdit(GetDlgItem(hWnd,IDC_MIDI_SPEED), EDITTYPE_FLOAT);
	spin->SetValue(speed/SPEED_UI_FACT,FALSE);
	ReleaseISpinner(spin);

	spin = GetISpinner(GetDlgItem(hWnd,IDC_MIDI_SUSTAINSPIN));
	spin->SetLimits(0, TIME_PosInfinity, FALSE);
	spin->SetScale(10.0f);
	spin->LinkToEdit(GetDlgItem(hWnd,IDC_MIDI_SUSTAIN), EDITTYPE_TIME);
	spin->SetValue(sustain,FALSE);
	ReleaseISpinner(spin);

	spin = GetISpinner(GetDlgItem(hWnd,IDC_MIDI_USERNUMSPIN));
	spin->SetLimits(0, 127, FALSE);
	spin->SetScale(1.0f);
	spin->LinkToEdit(GetDlgItem(hWnd,IDC_MIDI_USERNUM), EDITTYPE_INT);
	spin->SetValue(controlNum,FALSE);
	ReleaseISpinner(spin);

	SetupNote(GetDlgItem(hWnd,IDC_MIDI_LOWNOTE));
	SetupNote(GetDlgItem(hWnd,IDC_MIDI_HIGHNOTE));
	SetupOctave(GetDlgItem(hWnd,IDC_MIDI_LOWOCTAVE));
	SetupOctave(GetDlgItem(hWnd,IDC_MIDI_HIGHOCTAVE));	
	}


static INT_PTR CALLBACK MidiDeviceDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	IMCParamDlg *dlg = (IMCParamDlg*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	MidiDeviceBinding *b = NULL;
	if (!dlg && msg!=WM_INITDIALOG) return FALSE;
	if (dlg) b = (MidiDeviceBinding*)dlg->binding;

	switch (msg) {
		case WM_INITDIALOG:
			dlg = (IMCParamDlg*)lParam;
			((MidiDeviceBinding*)dlg->binding)->InitDlg(hWnd);			
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_MIDI_VIEWER:
					MIDIViewerDialog(hWnd);
					break;

				case IDC_MIDI_CHAN1: case IDC_MIDI_CHAN9:
				case IDC_MIDI_CHAN2: case IDC_MIDI_CHAN10:
				case IDC_MIDI_CHAN3: case IDC_MIDI_CHAN11:
				case IDC_MIDI_CHAN4: case IDC_MIDI_CHAN12:
				case IDC_MIDI_CHAN5: case IDC_MIDI_CHAN13:
				case IDC_MIDI_CHAN6: case IDC_MIDI_CHAN14:
				case IDC_MIDI_CHAN7: case IDC_MIDI_CHAN15:
				case IDC_MIDI_CHAN8: case IDC_MIDI_CHAN16:
					b->SetChannel(LOWORD(wParam)-IDC_MIDI_CHAN1);
					b->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
					break;

				case IDC_MIDI_USER:
				case IDC_MIDI_NOTE:
				case IDC_MIDI_VEL:
				case IDC_MIDI_BEND:
					b->SetTrigger(LOWORD(wParam));
					b->SetEnabledStates(hWnd);
					b->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
					break;

				case IDC_MIDI_LOWNOTE:
				case IDC_MIDI_LOWOCTAVE:
					if (HIWORD(wParam)==CBN_SELCHANGE) {
						b->SetLowNote(
							b->GetNoteVal(hWnd,IDC_MIDI_LOWNOTE,IDC_MIDI_LOWOCTAVE));
						b->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
						}
					break;

				case IDC_MIDI_HIGHNOTE:
				case IDC_MIDI_HIGHOCTAVE:
					if (HIWORD(wParam)==CBN_SELCHANGE) {
						b->SetHighNote(
							b->GetNoteVal(hWnd,IDC_MIDI_HIGHNOTE,IDC_MIDI_HIGHOCTAVE));
						b->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
						}
					break;

				case IDC_SUSTAIN_VARIABLE:
					b->SetVariSustain(
						IsDlgButtonChecked(hWnd,IDC_SUSTAIN_VARIABLE));
					b->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
					break;
				}
			break;		

		case CC_SPINNER_CHANGE: {
			ISpinnerControl *spin = (ISpinnerControl *)lParam;			
			switch (LOWORD(wParam)) {
				case IDC_MIDI_PARAMMINSPIN: b->SetMin(spin->GetFVal()); break;
				case IDC_MIDI_PARAMMAXSPIN: b->SetMax(spin->GetFVal()); break;
				case IDC_MIDI_SPEEDSPIN:    b->SetSpeed(spin->GetFVal()*SPEED_UI_FACT); break;
				case IDC_MIDI_SUSTAINSPIN:  b->SetSustain(spin->GetIVal()); break;
				case IDC_MIDI_USERNUMSPIN:  b->SetControlNum(spin->GetIVal()); break;
				}
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



//--- Midi device ----------------------------------------------------


ChannelData::ChannelData()
	{
	for (int i=0; i<NUM_MIDI_NOTES; i++) {
		pressed[i] = FALSE;
		time[i]    = TIME_NegInfinity;
		vel[i]     = 0;
		}
	bend = 0;
	}

static DWORD MidiInFunc(
		HMIDIIN hMidi, UINT msg, DWORD Instance, 
		DWORD param1, DWORD param2)
	{
	switch (msg) {
		case MIM_DATA:
			theMidiDevice.MidiEvent(param1);
			break;
		}

	return 0;
	}

MidiDevice::MidiDevice()
	{		
	//hMidiIn = NULL;
	active   = FALSE;	
	theMM    = NULL;
	noDevice = FALSE;
	}

void MidiDevice::Open()
	{
	int res = MIDIMan_Open(MidiInFunc,0,20);
	if (!noDevice && res!=MMSYSERR_NOERROR) {
		TSTR buf1 = GetString(IDS_RB_NODEVICE);
		TSTR buf2 = GetString(IDS_RB_MIDI);
		MessageBox(GetCOREInterface()->GetMAXHWnd(), buf1, buf2, MB_ICONSTOP|MB_OK|MB_APPLMODAL);
		noDevice = TRUE;
		}
#if 0
	if (!hMidiIn) {
		if (midiInGetNumDevs()) {
			res = midiInOpen(&hMidiIn, 0,(DWORD)MidiInFunc, 0,CALLBACK_FUNCTION);
			if (res!=MMSYSERR_NOERROR) {
				TSTR buf1 = GetString(IDS_RB_NODEVICE);
				TSTR buf2 = GetString(IDS_RB_MIDI);
				MessageBox(GetCOREInterface()->GetMAXHWnd(), buf1, buf2, MB_ICONSTOP|MB_OK|MB_APPLMODAL);
				}			
			}
		}
#endif
	}

void MidiDevice::Close()
	{
	int res = MIDIMan_Close(MidiInFunc,0);
	if (!noDevice && res!=MMSYSERR_NOERROR) {
		TSTR buf1 = GetString(IDS_RB_MIDIDEVICE_ERROR);
		TSTR buf2 = GetString(IDS_RB_MIDI);
		MessageBox(GetCOREInterface()->GetMAXHWnd(), buf1, buf2, MB_ICONSTOP|MB_OK|MB_APPLMODAL);
		}
#if 0
	if (hMidiIn) {
		res = midiInClose(hMidiIn);
		hMidiIn = NULL;
		if (res!=MMSYSERR_NOERROR) {
			TSTR buf1 = GetString(IDS_RB_MIDIDEVICE_ERROR);
			TSTR buf2 = GetString(IDS_RB_MIDI);
			MessageBox(GetCOREInterface()->GetMAXHWnd(), buf1, buf2, MB_ICONSTOP|MB_OK);
			}
		}
#endif
	}

void MidiDevice::Start()
	{
	if (!active && MIDIMan_IsOpened()) {
		if (MIDIMan_Start() != MMSYSERR_NOERROR) {
			TSTR buf1 = GetString(IDS_RB_MIDIDEVICE_ERROR);
			TSTR buf2 = GetString(IDS_RB_MIDI);
			MessageBox(GetCOREInterface()->GetMAXHWnd(), buf1, buf2, MB_ICONSTOP|MB_OK|MB_APPLMODAL);
			return;
			}
		active = TRUE;
		}
#if 0
	if (!active && hMidiIn) {		
		if (midiInStart(hMidiIn) != MMSYSERR_NOERROR) {
			TSTR buf1 = GetString(IDS_RB_MIDIDEVICE_ERROR);
			TSTR buf2 = GetString(IDS_RB_MIDI);
			MessageBox(GetCOREInterface()->GetMAXHWnd(), buf1, buf2, MB_ICONSTOP|MB_OK|MB_APPLMODAL);			
			return;
			}
		active = TRUE;
		}
#endif
	
	// Clear the keyboard.
	for (int i=0; i<NUM_MIDI_CHANNELS; i++) {
		data[i] = ChannelData();
		}
	}

void MidiDevice::Stop()
	{
	if (active && MIDIMan_IsOpened()) {
		if (MIDIMan_Stop() != MMSYSERR_NOERROR) {
			TSTR buf1 = GetString(IDS_RB_MIDIDEVICE_ERROR);
			TSTR buf2 = GetString(IDS_RB_MIDI);
			MessageBox(GetCOREInterface()->GetMAXHWnd(), buf1, buf2, MB_ICONSTOP|MB_OK|MB_APPLMODAL);
			return;
			}
		active = FALSE;
		}
#if 0
	if (active && hMidiIn) {
		if (midiInStop(hMidiIn) != MMSYSERR_NOERROR) {
			TSTR buf1 = GetString(IDS_RB_MIDIDEVICE_ERROR);
			TSTR buf2 = GetString(IDS_RB_MIDI);
			MessageBox(GetCOREInterface()->GetMAXHWnd(), buf1, buf2, MB_ICONSTOP|MB_OK|MB_APPLMODAL);
			return;
			}
		active = FALSE;
		}
#endif
	}

void MidiDevice::UtilityStarted(IMCapManager *im) 
	{
	Open();
	Start();
	theMM = im;
	}

void MidiDevice::UtilityStopped(IMCapManager *im) 
	{
	Stop();
	Close();
	theMM = NULL;
	}

void MidiDevice::MidiEvent(DWORD param)
	{
	assert(theMM);
	int channel = MIDI_CHANNEL(param);
	int event = MIDI_EVENT(param);
	int vel = MIDI_VELOCITY(param);
	int nn  = MIDI_NOTENUMBER(param);

	switch (event) {		
		case MIDI_NOTE_ON: {			
			data[channel].pressed[nn] = TRUE;
			data[channel].time[nn]    = theMM->GetTime();
			if (vel) data[channel].vel[nn] = vel;
			theMM->MidiNote(channel, nn);
			break;
			}

		case MIDI_NOTE_OFF: {			
			data[channel].pressed[nn] = FALSE;
			data[channel].time[nn]    = TIME_NegInfinity;
			data[channel].vel    [nn] = 0;
			break;
			}

		case MIDI_PITCH_BEND: {
			int pb = MIDI_PITCHBEND(param);
			data[channel].bend = pb;
			break;
			}
		
		case MIDI_CONTROLCHANGE:
			data[channel].pressed[nn] = TRUE;
			data[channel].time[nn]    = theMM->GetTime();
			data[channel].vel[nn] = vel;
			break;		
		}
	}

int MidiDevice::FirstKeyDown(int channel, int low, int high)
	{
	for (int i=low; i<=high; i++) {
		if (data[channel].pressed[i]) return i;
		}
	return -1;
	}

int MidiDevice::NewestKeyDown(int channel, int low, int high)
	{
	int res = -1;
	TimeValue t = TIME_NegInfinity;
	for (int i=low; i<=high; i++) {
		if (data[channel].pressed[i] && 
			data[channel].time[i] > t) {
			res = i;
			t   = data[channel].time[i];
			}
		}
	return res;
	}


//------------------------------------------------------------
// MIDI Viewer Dialog

static int chanButs[] = {
	IDC_MV_CHANBUT1,IDC_MV_CHANBUT2,IDC_MV_CHANBUT3,IDC_MV_CHANBUT4,
	IDC_MV_CHANBUT5,IDC_MV_CHANBUT6,IDC_MV_CHANBUT7,IDC_MV_CHANBUT8,
	IDC_MV_CHANBUT9,IDC_MV_CHANBUT10,IDC_MV_CHANBUT11,IDC_MV_CHANBUT12,
	IDC_MV_CHANBUT13,IDC_MV_CHANBUT14,IDC_MV_CHANBUT15,IDC_MV_CHANBUT16};

static int chanProg[] = {
	IDC_MV_CHANPROG1,IDC_MV_CHANPROG2,IDC_MV_CHANPROG3,IDC_MV_CHANPROG4,
	IDC_MV_CHANPROG5,IDC_MV_CHANPROG6,IDC_MV_CHANPROG7,IDC_MV_CHANPROG8,
	IDC_MV_CHANPROG9,IDC_MV_CHANPROG10,IDC_MV_CHANPROG11,IDC_MV_CHANPROG12,
	IDC_MV_CHANPROG13,IDC_MV_CHANPROG14,IDC_MV_CHANPROG15,IDC_MV_CHANPROG16};

static int octaves[] = {
	IDC_MV_OCTAVEBUT1,IDC_MV_OCTAVEBUT2,IDC_MV_OCTAVEBUT3,
	IDC_MV_OCTAVEBUT4,IDC_MV_OCTAVEBUT5,IDC_MV_OCTAVEBUT6,
	IDC_MV_OCTAVEBUT7,IDC_MV_OCTAVEBUT8,IDC_MV_OCTAVEBUT9,
	IDC_MV_OCTAVEBUT10,IDC_MV_OCTAVEBUT11};

static int notes[] = {
	IDC_MV_NOTEPROG1, IDC_MV_NOTEPROG2, IDC_MV_NOTEPROG3,
	IDC_MV_NOTEPROG4, IDC_MV_NOTEPROG5, IDC_MV_NOTEPROG6,
	IDC_MV_NOTEPROG7, IDC_MV_NOTEPROG8, IDC_MV_NOTEPROG9,
	IDC_MV_NOTEPROG10,IDC_MV_NOTEPROG11,IDC_MV_NOTEPROG12};

static HWND hMIDIViewer=NULL;
//static HMIDIIN hMidiIn=NULL;
static int channel=0, octave=0;	

static int channelVars[16];
static int noteVars[12];
static int controllerVar=0;
static int controlNum = 7;

static void ClearChannelVars() {
	for (int i=0; i<16; i++) channelVars[i] = 0;
	}

static void ClearNoteVars() {
	for (int i=0; i<12; i++) noteVars[i] = 0;	
	}

static INT_PTR CALLBACK MidiViewerDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG: {
			CheckDlgButton(hWnd,chanButs[channel],TRUE);
			CheckDlgButton(hWnd,octaves[octave],TRUE);
			hMIDIViewer = hWnd;
			ClearChannelVars();
			ClearNoteVars();
			for (int i=0; i<12; i++) {
				SendDlgItemMessage(hWnd,notes[i],
					PBM_SETRANGE,0,MAKELPARAM(0,127));
				}
			SendDlgItemMessage(hWnd,IDC_MV_NOTEUSER,
					PBM_SETRANGE,0,MAKELPARAM(0,127));
			SetTimer(hWnd,1,100,NULL);

			ISpinnerControl *spin = GetISpinner(GetDlgItem(hWnd,IDC_MIDI_USERNUMSPIN));
			spin->SetLimits(0, 127, FALSE);
			spin->SetScale(1.0f);
			spin->LinkToEdit(GetDlgItem(hWnd,IDC_MIDI_USERNUM), EDITTYPE_INT);
			spin->SetValue(controlNum,FALSE);
			ReleaseISpinner(spin);

			break;
			}
		
		case WM_TIMER: {
			for (int i=0; i<16; i++) {
				SendDlgItemMessage(hWnd,chanProg[i],
					PBM_SETPOS,channelVars[i],0);
				channelVars[i] -= 20;
				if (channelVars[i]<0) channelVars[i] = 0;
				}
			for (i=0; i<12; i++) {
				SendDlgItemMessage(hWnd,notes[i],
					PBM_SETPOS,noteVars[i],0);
				noteVars[i] -= 10;
				if (noteVars[i]<0) noteVars[i] = 0;
				}
			
			SendDlgItemMessage(hWnd,IDC_MV_NOTEUSER,
					PBM_SETPOS,controllerVar,0);
			//controllerVar -= 10;
			//if (controllerVar<0) controllerVar = 0;
			break;
			}


		case WM_DESTROY:
			hMIDIViewer = NULL;
			KillTimer(hWnd,1);
			break;

		case CC_SPINNER_CHANGE: {
			ISpinnerControl *iSpin = (ISpinnerControl*)lParam;
			controlNum = iSpin->GetIVal();
			controllerVar = 0;
			break;
			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_MV_CHANBUT1:  channel = 0; ClearNoteVars(); break;
				case IDC_MV_CHANBUT2:  channel = 1; ClearNoteVars(); break;
				case IDC_MV_CHANBUT3:  channel = 2; ClearNoteVars(); break;
				case IDC_MV_CHANBUT4:  channel = 3; ClearNoteVars(); break;
				case IDC_MV_CHANBUT5:  channel = 4; ClearNoteVars(); break;
				case IDC_MV_CHANBUT6:  channel = 5; ClearNoteVars(); break;
				case IDC_MV_CHANBUT7:  channel = 6; ClearNoteVars(); break;
				case IDC_MV_CHANBUT8:  channel = 7; ClearNoteVars(); break;
				case IDC_MV_CHANBUT9:  channel = 8; ClearNoteVars(); break;
				case IDC_MV_CHANBUT10: channel = 9; ClearNoteVars(); break;
				case IDC_MV_CHANBUT11: channel = 10; ClearNoteVars(); break;
				case IDC_MV_CHANBUT12: channel = 11; ClearNoteVars(); break;
				case IDC_MV_CHANBUT13: channel = 12; ClearNoteVars(); break;
				case IDC_MV_CHANBUT14: channel = 13; ClearNoteVars(); break;
				case IDC_MV_CHANBUT15: channel = 14; ClearNoteVars(); break;
				case IDC_MV_CHANBUT16: channel = 15; ClearNoteVars(); break;		

				case IDC_MV_OCTAVEBUT1:  octave = 0; break;
				case IDC_MV_OCTAVEBUT2:  octave = 1; break;
				case IDC_MV_OCTAVEBUT3:  octave = 2; break;
				case IDC_MV_OCTAVEBUT4:  octave = 3; break;
				case IDC_MV_OCTAVEBUT5:  octave = 4; break;
				case IDC_MV_OCTAVEBUT6:  octave = 5; break;
				case IDC_MV_OCTAVEBUT7:  octave = 6; break;
				case IDC_MV_OCTAVEBUT8:  octave = 7; break;
				case IDC_MV_OCTAVEBUT9:  octave = 8; break;
				case IDC_MV_OCTAVEBUT10: octave = 9; break;
				case IDC_MV_OCTAVEBUT11: octave = 10; break;

				case IDCANCEL:
				case IDOK:
					EndDialog(hWnd,1);
					break;
				}
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}

static DWORD MidiViewerInFunc(
		HMIDIIN hMidi, UINT msg, DWORD Instance, 
		DWORD param1, DWORD param2)
	{
	switch (msg) {
		case MIM_DATA: {
			int ch = MIDI_CHANNEL(param1);
			int nn = MIDI_NOTENUMBER(param1);			
			int event = MIDI_EVENT(param1);
			int vel = MIDI_VELOCITY(param1);			

			channelVars[ch] = 100;
			
			SetDlgItemInt(hMIDIViewer,IDC_MV_CHAN,ch+1,TRUE);
			SetDlgItemInt(hMIDIViewer,IDC_MV_EVENT,event>>4,TRUE);
			SetDlgItemInt(hMIDIViewer,IDC_MV_VEL,vel,TRUE);
			SetDlgItemInt(hMIDIViewer,IDC_MV_NOTE,nn,TRUE);

			switch (event) {
				case MIDI_CONTROLCHANGE:
					if (controlNum==nn && ch==channel) {
						controllerVar = vel;
						}
					break;

				case MIDI_NOTE_ON: {
					int vel = MIDI_VELOCITY(param1);
					if (nn/12==octave && ch==channel) {
						noteVars[nn%12] = vel;
						}
					break;
					}

				case MIDI_NOTE_OFF: {
					if (nn/12==octave && ch==channel) {
						noteVars[nn%12] = 0;
						}
					break;
					}
				}
			break;
			}
		}

	return 0;
	}


static void MIDIViewerDialog(HWND hParent)
	{
	int res = MMSYSERR_NOERROR;	

	if (hMIDIViewer) {
		SetForegroundWindow(hMIDIViewer);
		return;
		}

	res = MIDIMan_Open(MidiViewerInFunc,0);
	if (res != MMSYSERR_NOERROR) {
		TSTR buf1 = GetString(IDS_RB_NODEVICE);
		TSTR buf2 = GetString(IDS_RB_MIDI);
		MessageBox(GetCOREInterface()->GetMAXHWnd(), buf1, buf2, MB_ICONSTOP|MB_OK|MB_APPLMODAL);
		return;
		}

	res = MIDIMan_Start();
	if (res != MMSYSERR_NOERROR) {
		TSTR buf1 = GetString(IDS_RB_MIDIDEVICE_ERROR);
		TSTR buf2 = GetString(IDS_RB_MIDI);
		MessageBox(GetCOREInterface()->GetMAXHWnd(), buf1, buf2, MB_ICONSTOP|MB_OK|MB_APPLMODAL);
		return;
		}	

	DialogBox(
		hInstance,
		MAKEINTRESOURCE(IDD_MIDI_VIEWER),
		hParent,
		MidiViewerDlgProc);

	MIDIMan_Stop();
	res = MIDIMan_Close(MidiViewerInFunc,0);
	if (res != MMSYSERR_NOERROR) {
		TSTR buf1 = GetString(IDS_RB_MIDIDEVICE_ERROR);
		TSTR buf2 = GetString(IDS_RB_MIDI);
		MessageBox(GetCOREInterface()->GetMAXHWnd(), buf1, buf2, MB_ICONSTOP|MB_OK|MB_APPLMODAL);
		}	
#if 0
	BOOL opened = FALSE;
	if (theMidiDevice.hMidiIn) {
		opened = TRUE;
		theMidiDevice.Stop();
		theMidiDevice.Close();
		}

	if (!midiInGetNumDevs()) {
		TSTR buf1 = GetString(IDS_RB_NOMIDIDEVICES);
		TSTR buf2 = GetString(IDS_RB_MIDIVIEWER);
		MessageBox(hParent,buf1,buf2,MB_ICONEXCLAMATION|MB_OK|MB_APPLMODAL);
		goto done;
		}

	if (!hMidiIn) {
		res = midiInOpen(&hMidiIn, 0,(DWORD)MidiViewerInFunc, 0,
			CALLBACK_FUNCTION);
		}
	if (res!=MMSYSERR_NOERROR) {
		TSTR buf1 = GetString(IDS_RB_NODEVICE);
		TSTR buf2 = GetString(IDS_RB_MIDI);
		MessageBox(GetCOREInterface()->GetMAXHWnd(), buf1, buf2, MB_ICONSTOP|MB_OK|MB_APPLMODAL);
		goto done;
		}
	
	if (midiInStart(hMidiIn) != MMSYSERR_NOERROR) {
		TSTR buf1 = GetString(IDS_RB_MIDIDEVICE_ERROR);
		TSTR buf2 = GetString(IDS_RB_MIDI);
		MessageBox(GetCOREInterface()->GetMAXHWnd(), buf1, buf2, MB_ICONSTOP|MB_OK|MB_APPLMODAL);
		goto done;
		}	
	
	DialogBox(
		hInstance,
		MAKEINTRESOURCE(IDD_MIDI_VIEWER),
		hParent,
		MidiViewerDlgProc);

	if (midiInStop(hMidiIn) != MMSYSERR_NOERROR) {
		TSTR buf1 = GetString(IDS_RB_MIDIDEVICE_ERROR);
		TSTR buf2 = GetString(IDS_RB_MIDI);
		MessageBox(GetCOREInterface()->GetMAXHWnd(), buf1, buf2, MB_ICONSTOP|MB_OK|MB_APPLMODAL);
		}

	if (midiInClose(hMidiIn) != MMSYSERR_NOERROR) {
		TSTR buf1 = GetString(IDS_RB_MIDIDEVICE_ERROR);
		TSTR buf2 = GetString(IDS_RB_MIDI);
		MessageBox(GetCOREInterface()->GetMAXHWnd(), buf1, buf2, MB_ICONSTOP|MB_OK|MB_APPLMODAL);
		}	
	hMidiIn = NULL;

done:	
	if (opened) {		
		theMidiDevice.Open();
		theMidiDevice.Start();
		}
#endif
	}
