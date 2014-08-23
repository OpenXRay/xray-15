/**********************************************************************
 *<
	FILE: simpwave.cpp

	DESCRIPTION: A simple waveform controller

	CREATED BY: Tom Hudson

	HISTORY: created 25 Sept. 1996

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/
#include "wavectrl.h"
#include "units.h"
#include "iColorMan.h"

#define FLOATWAVE_CONTROL_CNAME	GetString(IDS_TH_WAVEFLOAT)
#define FLOATWAVE_CONTROL_CLASS_ID	0x930Abc78

#define TYPE_SQUARE 0
#define TYPE_SINE 1
#define TYPE_SAWTOOTH 2
#define TYPE_TRIANGLE 3
#define TYPE_HALFSINE 4

#define BIAS_CENTERED 0
#define BIAS_POSITIVE 1
#define BIAS_NEGATIVE 2
#define BIAS_MANUAL 3

#define EFFECT_ADD 0
#define EFFECT_MULTIPLY 1
#define EFFECT_CLAMP_ABOVE 2
#define EFFECT_CLAMP_BELOW 3

#define DISP_THIS 0		// Display this curve only
#define DISP_THISOUT 1	// Display output from this wave w/previous
#define DISP_FINALOUT 2	// Display final output of all waves combined

// Optional index parameters for GetValue
#define GETVAL_ALL -1

class BaseWaveControl : public StdControl {
	public:
		int waves;		// Number of waveforms making us up
		int index;		// Index of currently-displayed waveform
		TSTR *name;
		float *period;
		float *duty;	// Square waves only
		float *amplitude;
		float *phase;
		float *bias;
		int *type;
		int *biasType;
		int *effect;
		BOOL *inverted;
		BOOL *flipped;
		BOOL *enabled;
		int display;
		Interval range;

		BaseWaveControl();
		BaseWaveControl& operator=(const BaseWaveControl& from);
		float WaveAtTime(TimeValue t,int ix);	// -1 = all
		float GetValue(float where, int ix);	// -1 = all

		virtual int Elems()=0;

		// Animatable methods		
		void DeleteThis() {delete this;}		
		int IsKeyable() {return 0;}		
		BOOL IsAnimated() {return TRUE;}

		void EditTrackParams(
			TimeValue t,
			ParamDimensionBase *dim,
			TCHAR *pname,
			HWND hParent,
			IObjParam *ip,
			DWORD flags);
		
		int TrackParamsType() {return TRACKPARAMS_WHOLE;}

		// Reference methods
		RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage) {return REF_SUCCEED;}

		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		// Control methods				
		void Copy(Control *from) {}
		BOOL IsLeaf() {return TRUE;}
		void CommitValue(TimeValue t) {}
		void RestoreValue(TimeValue t) {}		
		
		void HoldRange();
		Interval GetTimeRange(DWORD flags) {return range;}
		void EditTimeRange(Interval range,DWORD flags);
		void MapKeys(TimeMap *map,DWORD flags );

		// Waveform slot management...
		void Insert(int ix);
		void Append();
		void Delete(int ix);
		void Move(int ix,int direction);

		// Info calls
		void GetRange(float *low, float *high, int ix);
		void GetDisplayRanges(float *lo, float *hi);
	};

//---------------------------------------------------------------------------

class FloatWaveControl : public BaseWaveControl {
	public:
		int Elems() {return 1;}

		Class_ID ClassID() { return Class_ID(FLOATWAVE_CONTROL_CLASS_ID,0); }  
		SClass_ID SuperClassID() { return CTRL_FLOAT_CLASS_ID; } 
		void GetClassName(TSTR& s) {s = FLOATWAVE_CONTROL_CNAME;}

		// Control methods
		RefTargetHandle Clone(RemapDir& remap);

		// StdControl methods
		void GetValueLocalTime(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void SetValueLocalTime(TimeValue t, void *val, int commit, GetSetMethod method) {}		
		void Extrapolate(Interval range,TimeValue t,void *val,Interval &valid,int type);		
		void *CreateTempValue() {return new float;}
		void DeleteTempValue(void *val) {delete (float*)val;}
		void ApplyValue(void *val, void *delta) {*((float*)val) += *((float*)delta);}
		void MultiplyValue(void *val, float m) {*((float*)val) *= m;}
	};


class FloatWaveClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new FloatWaveControl(); }
	const TCHAR *	ClassName() { return FLOATWAVE_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_FLOAT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(FLOATWAVE_CONTROL_CLASS_ID,0); }
	const TCHAR* 	Category() { return _T("");  }
	};
static FloatWaveClassDesc floatWaveCD;
ClassDesc* GetFloatWaveDesc() {return &floatWaveCD;}

//-----------------------------------------------------------------

class RangeRestore : public RestoreObj {
	public:
		BaseWaveControl *cont;
		Interval ur, rr;
		RangeRestore(BaseWaveControl *c) 
			{
			cont = c;
			ur   = cont->range;
			}   		
		void Restore(int isUndo) 
			{
			rr = cont->range;
			cont->range = ur;
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
		void Redo()
			{
			cont->range = rr;
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}		
		void EndHold() 
			{ 
			cont->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T("Wave control range")); }
	};


//-----------------------------------------------------------------
//
// UI

#define STEPSIZE			2
#define GRAPH_TIMESCALE		20

static void SetBiasRadio(HWND hWnd, int type) {
	CheckDlgButton(hWnd, IDC_CENTERED, type == BIAS_CENTERED);
	CheckDlgButton(hWnd, IDC_POSITIVE, type == BIAS_POSITIVE);
	CheckDlgButton(hWnd, IDC_NEGATIVE, type == BIAS_NEGATIVE);
	CheckDlgButton(hWnd, IDC_MANUAL, type == BIAS_MANUAL);
	}

static void SetEffectRadio(HWND hWnd, int effect) {
	CheckDlgButton(hWnd, IDC_ADDFUNC, effect == EFFECT_ADD);
	CheckDlgButton(hWnd, IDC_MULT, effect == EFFECT_MULTIPLY);
	CheckDlgButton(hWnd, IDC_CLAMPABOVE, effect == EFFECT_CLAMP_ABOVE);
	CheckDlgButton(hWnd, IDC_CLAMPBELOW, effect == EFFECT_CLAMP_BELOW);
	}

static void SetDisplayRadio(HWND hWnd, int display) {
	CheckDlgButton(hWnd, IDC_THIS, display == DISP_THIS);
	CheckDlgButton(hWnd, IDC_THISOUT, display == DISP_THISOUT);
	CheckDlgButton(hWnd, IDC_FINALOUT, display == DISP_FINALOUT);
	}

static HIMAGELIST hWaveButtons = NULL;

class DeleteWaveButtonResources {
	public:
		~DeleteWaveButtonResources() {
			ImageList_Destroy(hWaveButtons);
			}
	};
static DeleteWaveButtonResources	theWBDelete;

static void LoadWaveButtonResources()
	{
	static BOOL loaded=FALSE;
	if (loaded) return;
	loaded = TRUE;	
	HBITMAP hBitmap, hMask;

	InitCustomControls(hInstance);
	hWaveButtons = ImageList_Create(16, 16, TRUE, 6, 0);
	hBitmap = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_WAVE_BUTTONS));
	hMask   = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_WAVE_MASKBUTTONS));
	ImageList_Add(hWaveButtons,hBitmap,hMask);
	DeleteObject(hBitmap);
	DeleteObject(hMask);
	}

static INT_PTR CALLBACK AboutDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{	
	switch (msg) {
		case WM_INITDIALOG:			
			CenterWindow(hWnd, GetParent(hWnd));
			break;
			
		case WM_CLOSE:
			EndDialog(hWnd,0);			
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					EndDialog(hWnd,0);
					break;
				}
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}

//-----------------------------------------------------------------
//
// UI

static LRESULT CALLBACK WaveGraphWinProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static void InitWaveGraph(HINSTANCE hInst)
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
    wc.lpfnWndProc   = WaveGraphWinProc;
    wc.lpszClassName = _T("WaveGraph");
		
	RegisterClass(&wc);	
	}


#define STEPSIZE			2
#define GRAPH_TIMESCALE		20

static void DrawWaveGraph(BaseWaveControl *cont,HWND hWnd,HDC hdc)
	{
	Rect rect, orect;
	GetClientRect(hWnd,&rect);
	orect = rect;
	float fy;
	int mid = rect.h()/2;
	rect.top    += 5;
	rect.bottom -= 5;
	TimeValue end = rect.right * GRAPH_TIMESCALE;
	float bottomValue, topValue;

	switch(cont->display) {
		case DISP_THIS: {
			int index = cont->index;
			// Autoscale the graph...
			// Width is 2x period
			float viewWidth = 2.0f;
			float leftValue = 0.0f;
			float rightValue = 2.0f;
			// Establish bottom value
			cont->GetRange(&bottomValue, &topValue, index);
			// Height is 1.2x range
			float viewHeight = (topValue - bottomValue);
			float viewVScale = viewHeight / (float)rect.h();
			float viewHScale = viewWidth / (float)rect.w();

			// Draw zero line
			SelectObject(hdc,CreatePen(PS_DOT,0,GetCustSysColor(COLOR_BTNFACE)));
			int zeroPos = rect.bottom - (int)(-bottomValue / viewVScale);
			MoveToEx(hdc,0,zeroPos,NULL);
			LineTo(hdc,rect.right,zeroPos);

			DeleteObject(SelectObject(hdc,GetStockObject(BLACK_PEN)));

			for (int x = 0; x < rect.right + STEPSIZE; x += STEPSIZE) {
				float sample = (float)x * viewHScale;
				fy = (cont->GetValue(sample, index) - bottomValue) / viewVScale;
				if (!x) {
					MoveToEx(hdc,0,rect.bottom - int(fy),NULL);
				} else {
					LineTo(hdc,x,rect.bottom - int(fy));
					}
				}
			}
			break;
		case DISP_THISOUT: {
			float maxPeriod = -9999.0f;
			int index = cont->index;
			for(int ix = 0; ix <= index; ++ix) {
				if(cont->period[ix] > maxPeriod)
					maxPeriod = cont->period[ix];
				}
			// Autoscale the graph...
			cont->GetDisplayRanges(&bottomValue, &topValue);
			// Width is 2x period
			float viewWidth = 2.0f;
			float leftValue = 0.0f;
			float rightValue = 2.0f;
			// Height is 1.2x amplitude
			float viewHeight = (topValue - bottomValue);
			float viewVScale = viewHeight / (float)rect.h();
			float viewHScale = viewWidth / (float)rect.w();

			// Draw zero line
			SelectObject(hdc,CreatePen(PS_DOT,0,GetCustSysColor(COLOR_BTNFACE)));
			int zeroPos = rect.bottom - (int)(-bottomValue / viewVScale);
			MoveToEx(hdc,0,zeroPos,NULL);
			LineTo(hdc,rect.right,zeroPos);

			DeleteObject(SelectObject(hdc,GetStockObject(BLACK_PEN)));

			for (int x = 0; x < rect.right + STEPSIZE; x += STEPSIZE) {
				float yValue = 0.0f;
				for(ix = 0; ix <= index; ++ix) {
					float sample = (float)x * viewHScale * (maxPeriod / cont->period[ix]);
					fy = cont->GetValue(sample, ix);
					switch(cont->effect[ix]) {
						case EFFECT_ADD:
							yValue += fy;
							break;
						case EFFECT_MULTIPLY:
							yValue *= fy;
							break;
						case EFFECT_CLAMP_ABOVE:
							if(yValue < fy)
								yValue = fy;
							break;
						case EFFECT_CLAMP_BELOW:
							if(yValue > fy)
								yValue = fy;
							break;
						}
					}
				yValue = (yValue - bottomValue) / viewVScale;
				if (!x) {
					MoveToEx(hdc,0,rect.bottom - int(yValue),NULL);
				} else {
					LineTo(hdc,x,rect.bottom - int(yValue));
					}
				}
			}
			break;
		case DISP_FINALOUT: {
			float maxPeriod = -9999.0f;
			int index = cont->index;
			for(int ix = 0; ix < cont->waves; ++ix) {
				if(cont->period[ix] > maxPeriod)
					maxPeriod = cont->period[ix];
				}
			// Autoscale the graph...
			cont->GetDisplayRanges(&bottomValue, &topValue);
			// Width is 2x period
			float viewWidth = 2.0f;
			float leftValue = 0.0f;
			float rightValue = 2.0f;
			// Height is 1.2x amplitude
			float viewHeight = (topValue - bottomValue);
			float viewVScale = viewHeight / (float)rect.h();
			float viewHScale = viewWidth / (float)rect.w();

			// Draw zero line
			SelectObject(hdc,CreatePen(PS_DOT,0,GetCustSysColor(COLOR_BTNFACE)));
			int zeroPos = rect.bottom - (int)(-bottomValue / viewVScale);
			MoveToEx(hdc,0,zeroPos,NULL);
			LineTo(hdc,rect.right,zeroPos);

			DeleteObject(SelectObject(hdc,GetStockObject(BLACK_PEN)));

			for (int x = 0; x < rect.right + STEPSIZE; x += STEPSIZE) {
				float yValue = 0.0f;
				for(ix = 0; ix < cont->waves; ++ix) {
					float sample = (float)x * viewHScale * (maxPeriod / cont->period[ix]);
					fy = cont->GetValue(sample, ix);
					switch(cont->effect[ix]) {
						case EFFECT_ADD:
							yValue += fy;
							break;
						case EFFECT_MULTIPLY:
							yValue *= fy;
							break;
						case EFFECT_CLAMP_ABOVE:
							if(yValue < fy)
								yValue = fy;
							break;
						case EFFECT_CLAMP_BELOW:
							if(yValue > fy)
								yValue = fy;
							break;
						}
					}
				yValue = (yValue - bottomValue) / viewVScale;
				if (!x) {
					MoveToEx(hdc,0,rect.bottom - int(yValue),NULL);
				} else {
					LineTo(hdc,x,rect.bottom - int(yValue));
					}
				}
			}
			break;
		}
	WhiteRect3D(hdc,orect,TRUE);
	}

static LRESULT CALLBACK WaveGraphWinProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	BaseWaveControl *cont = (BaseWaveControl*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	if (!cont) return DefWindowProc(hWnd,msg,wParam,lParam);

	switch (msg) {		
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd,&ps);
			DrawWaveGraph(cont,hWnd,hdc);
			EndPaint(hWnd,&ps);
			break;
			}

		default:
			return DefWindowProc(hWnd,msg,wParam,lParam);
		}
	
	return 0;
	}

//--------------------------------------------------------------

static INT_PTR CALLBACK WaveDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#define WAVEDLG_CLASS_ID	0x8234Fc4e

class WaveCtrlWindow {
	public:
		HWND hWnd;
		HWND hParent;
		Control *cont;
		WaveCtrlWindow() {assert(0);}
		WaveCtrlWindow(HWND hWnd,HWND hParent,Control *cont)
			{this->hWnd=hWnd; this->hParent=hParent; this->cont=cont;}
	};
static Tab<WaveCtrlWindow> waveCtrlWindows;

static void RegisterWaveCtrlWindow(HWND hWnd, HWND hParent, Control *cont)
	{
	WaveCtrlWindow rec(hWnd,hParent,cont);
	waveCtrlWindows.Append(1,&rec);
	}

static void UnRegisterWaveCtrlWindow(HWND hWnd)
	{	
	for (int i=0; i<waveCtrlWindows.Count(); i++) {
		if (hWnd==waveCtrlWindows[i].hWnd) {
			waveCtrlWindows.Delete(i,1);
			return;
			}
		}	
	}

static HWND FindOpenWaveCtrlWindow(HWND hParent,Control *cont)
	{	
	for (int i=0; i<waveCtrlWindows.Count(); i++) {
		if (hParent == waveCtrlWindows[i].hParent &&
			cont    == waveCtrlWindows[i].cont) {
			return waveCtrlWindows[i].hWnd;
			}
		}
	return NULL;
	}

class WaveDlg : public ReferenceMaker {
	public:
		BaseWaveControl *cont;	
		ParamDimensionBase *dim;
		IObjParam *ip;
		HWND hWnd;
		BOOL valid;
		ISpinnerControl *iPeriod, *iDuty, *iAmplitude, *iPhase, *iBias;

		WaveDlg(
			BaseWaveControl *cont,
			ParamDimensionBase *dim,
			TCHAR *pname,
			IObjParam *ip,
			HWND hParent);
		~WaveDlg();

		Class_ID ClassID() {return Class_ID(WAVEDLG_CLASS_ID,0);}
		SClass_ID SuperClassID() {return REF_MAKER_CLASS_ID;}

		void MaybeCloseWindow();

		void Invalidate();
		void Update();
		void SetupUI(HWND hWnd);
		void Change(BOOL redraw=FALSE);
		void WMCommand(int id, int notify, HWND hCtrl);
		void SpinnerChange(int id,BOOL drag);
		void SpinnerStart(int id);
		void SpinnerEnd(int id,BOOL cancel);

		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	         PartID& partID,  RefMessage message);
		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i) {return cont;}
		void SetReference(int i, RefTargetHandle rtarg) {cont=(BaseWaveControl*)rtarg;}
	};


WaveDlg::WaveDlg(
		BaseWaveControl *cont,
		ParamDimensionBase *dim,
		TCHAR *pname,
		IObjParam *ip,
		HWND hParent)
	{
	InitWaveGraph(hInstance);
	this->cont = cont;
	this->ip   = ip;
	this->dim  = dim;
	valid = FALSE;
	MakeRefByID(FOREVER,0,cont);
	hWnd = CreateDialogParam(
		hInstance,
		MAKEINTRESOURCE(IDD_WAVEPARAMS),
		hParent,
		WaveDlgProc,
		(LPARAM)this);	
	TSTR title = TSTR(GetString(IDS_TH_WAVECONTROLTITLE)) + TSTR(pname);
	SetWindowText(hWnd,title);
	}

WaveDlg::~WaveDlg()
	{
	UnRegisterWaveCtrlWindow(hWnd);
	DeleteAllRefsFromMe();
	ReleaseISpinner(iPeriod);
	ReleaseISpinner(iDuty);
	ReleaseISpinner(iAmplitude);
	ReleaseISpinner(iPhase);
	ReleaseISpinner(iBias);
	}

void WaveDlg::Invalidate()
	{
	valid = FALSE;
	Rect rect(IPoint2(0,0),IPoint2(10,10));
	InvalidateRect(hWnd,&rect,FALSE);	
	}

// strip off all but one trailing zero --DS
static void RemoveTrailingZeros(char* buf) {
	int n = strlen(buf);
	for (int i=0; i<n; i++) {
		if (buf[i] == '.' || buf[i] == ',') {
			for (int j = n-1; j>i+1; j--) {
				if (buf[j]=='0') 
					buf[j] = 0;
				else 
					break;
				}
			break;
			}
		}
	}

void WaveDlg::Update()
	{
	ICustButton *but;
	if (!valid && hWnd) {
		int index = cont->index;
		SendMessage(GetDlgItem(hWnd,IDC_WAVELIST), LB_RESETCONTENT, 0, 0);
		for(int i = 0; i < cont->waves; ++i)
			SendMessage(GetDlgItem(hWnd,IDC_WAVELIST), LB_ADDSTRING, 0, (LPARAM)cont->name[i].data());
		SendMessage(GetDlgItem(hWnd,IDC_WAVELIST), LB_SETCURSEL, cont->index, 0);
		SendMessage(GetDlgItem(hWnd,IDC_WAVENAME), WM_SETTEXT, 0, (LPARAM)cont->name[index].data());
		iPeriod->SetValue(cont->period[index],FALSE);
		iDuty->SetValue(cont->duty[index],FALSE);
		if(cont->type[index] == TYPE_SQUARE)
			iDuty->Enable();
		else 
			iDuty->Disable();
		iAmplitude->SetValue(dim->Convert(cont->amplitude[index]),FALSE);
		iPhase->SetValue(cont->phase[index],FALSE);
		iBias->SetValue(dim->Convert(cont->bias[index]),FALSE);
		but = GetICustButton(GetDlgItem(hWnd, IDC_SQUARE));
		but->SetCheck((cont->type[index] == TYPE_SQUARE) ? TRUE : FALSE);
		ReleaseICustButton(but);
		but = GetICustButton(GetDlgItem(hWnd, IDC_SINE));
		but->SetCheck((cont->type[index] == TYPE_SINE) ? TRUE : FALSE);
		ReleaseICustButton(but);
		but = GetICustButton(GetDlgItem(hWnd, IDC_TRIANGLE));
		but->SetCheck((cont->type[index] == TYPE_TRIANGLE) ? TRUE : FALSE);
		ReleaseICustButton(but);
		but = GetICustButton(GetDlgItem(hWnd, IDC_SAWTOOTH));
		but->SetCheck((cont->type[index] == TYPE_SAWTOOTH) ? TRUE : FALSE);
		ReleaseICustButton(but);
		but = GetICustButton(GetDlgItem(hWnd, IDC_HALFSINE));
		but->SetCheck((cont->type[index] == TYPE_HALFSINE) ? TRUE : FALSE);
		ReleaseICustButton(but);
		CheckDlgButton(hWnd,IDC_INVERTED,cont->inverted[index]);
		CheckDlgButton(hWnd,IDC_FLIPPED,cont->flipped[index]);
		CheckDlgButton(hWnd,IDC_DISABLE,cont->enabled[index] ? FALSE : TRUE);
		SetBiasRadio(hWnd, cont->biasType[index]);
		SetEffectRadio(hWnd, cont->effect[index]);
		SetDisplayRadio(hWnd, cont->display);

		EnableWindow(GetDlgItem(hWnd,IDC_REMOVE), (cont->waves > 1) ? TRUE : FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_MOVEUP), (index > 0) ? TRUE : FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_MOVEDOWN), (index < (cont->waves - 1)) ? TRUE : FALSE);

		float lorange, hirange;
		cont->GetDisplayRanges(&lorange, &hirange);
		char buf[256];
		sprintf(buf,"%.2f",lorange);
		RemoveTrailingZeros(buf);
		SetDlgItemText(hWnd, IDC_LOVALUE, buf);
		sprintf(buf,"%.2f",hirange);
		RemoveTrailingZeros(buf);
		SetDlgItemText(hWnd, IDC_HIVALUE, buf);

		valid = TRUE;
		}
	}

void WaveDlg::SetupUI(HWND hWnd)
	{
	ICustButton *but;	
	this->hWnd = hWnd;

	iPeriod = GetISpinner(GetDlgItem(hWnd,IDC_WAVE_PERIODSPIN));
	iPeriod->SetLimits(0.01f,9999999.0f,FALSE);
	iPeriod->SetScale(0.1f);
	iPeriod->LinkToEdit(GetDlgItem(hWnd,IDC_WAVE_PERIOD),EDITTYPE_FLOAT);	

	iDuty = GetISpinner(GetDlgItem(hWnd,IDC_WAVE_DUTYSPIN));
	iDuty->SetLimits(0,100,FALSE);
	iDuty->SetScale(0.1f);
	iDuty->LinkToEdit(GetDlgItem(hWnd,IDC_WAVE_DUTY),EDITTYPE_FLOAT);	

	iAmplitude = GetISpinner(GetDlgItem(hWnd,IDC_WAVE_AMPLITUDESPIN));
	iAmplitude->SetLimits(0,9999999,FALSE);
	iAmplitude->SetScale(0.1f);
	iAmplitude->LinkToEdit(GetDlgItem(hWnd,IDC_WAVE_AMPLITUDE),EDITTYPE_FLOAT);	

	iPhase = GetISpinner(GetDlgItem(hWnd,IDC_WAVE_PHASESPIN));
	iPhase->SetLimits(0,1,FALSE);
	iPhase->SetScale(0.01f);
	iPhase->LinkToEdit(GetDlgItem(hWnd,IDC_WAVE_PHASE),EDITTYPE_FLOAT);	
	
	iBias = GetISpinner(GetDlgItem(hWnd,IDC_WAVE_BIASSPIN));
	iBias->SetLimits(-9999999,9999999,FALSE);
	iBias->SetScale(0.1f);
	iBias->LinkToEdit(GetDlgItem(hWnd,IDC_WAVE_BIAS),EDITTYPE_FLOAT);	
	
	LoadWaveButtonResources();
	but = GetICustButton(GetDlgItem(hWnd,IDC_SQUARE));
	but->SetType(CBT_CHECK);
	but->SetImage( hWaveButtons, 0,0,0,0, 16, 16);
	but->SetTooltip(TRUE, GetString(IDS_TH_SQUARE));
	ReleaseICustButton(but);
	but = GetICustButton(GetDlgItem(hWnd,IDC_SINE));
	but->SetType(CBT_CHECK);
	but->SetImage( hWaveButtons, 1,1,1,1, 16, 16);
	but->SetTooltip(TRUE, GetString(IDS_TH_SINE));
	ReleaseICustButton(but);
	but = GetICustButton(GetDlgItem(hWnd,IDC_TRIANGLE));
	but->SetType(CBT_CHECK);
	but->SetImage( hWaveButtons, 2,2,2,2, 16, 16);
	but->SetTooltip(TRUE, GetString(IDS_TH_TRIANGLE));
	ReleaseICustButton(but);
	but = GetICustButton(GetDlgItem(hWnd,IDC_SAWTOOTH));
	but->SetType(CBT_CHECK);
	but->SetImage( hWaveButtons, 3,3,3,3, 16, 16);
	but->SetTooltip(TRUE, GetString(IDS_TH_SAWTOOTH));
	ReleaseICustButton(but);
	but = GetICustButton(GetDlgItem(hWnd,IDC_HALFSINE));
	but->SetType(CBT_CHECK);
	but->SetImage( hWaveButtons, 4,4,4,4, 16, 16);
	but->SetTooltip(TRUE, GetString(IDS_TH_HALFSINE));
	ReleaseICustButton(but);

	SetWindowLongPtr(GetDlgItem(hWnd,IDC_WAVE_GRAPH),GWLP_USERDATA,(LONG_PTR)cont);
	valid = FALSE;
	Update();
	}

void WaveDlg::WMCommand(int id, int notify, HWND hCtrl)
	{
	int index = cont->index;
	TCHAR buf[256];
	switch (id) {
		case IDC_WAVELIST:
			switch(notify) {
				case LBN_SELCHANGE: {
					int newIndex = SendMessage(GetDlgItem(hWnd,IDC_WAVELIST), LB_GETCURSEL, 0, 0);
					cont->index = newIndex;
					Change(TRUE);
					}
					break;					
				}
			break;
		case IDC_WAVENAME:
			switch(notify) {
				case EN_SETFOCUS:
					DisableAccelerators();
					break;
				case EN_KILLFOCUS:
					EnableAccelerators();
					GetDlgItemText(hWnd, IDC_WAVENAME, buf, 256);
					cont->name[index] = buf;
					Change(TRUE);
					break;
				case EN_CHANGE:
					GetDlgItemText(hWnd, IDC_WAVENAME, buf, 256);
					cont->name[index] = buf;
					SendMessage(GetDlgItem(hWnd,IDC_WAVELIST), LB_DELETESTRING, index, 0);
					SendMessage(GetDlgItem(hWnd,IDC_WAVELIST), LB_INSERTSTRING, index, (LPARAM)cont->name[index].data());
					SendMessage(GetDlgItem(hWnd,IDC_WAVELIST), LB_SETCURSEL, index, 0);
					break;
				}					
			break;
		case IDC_SQUARE:
			cont->type[index] = TYPE_SQUARE;
			Change(TRUE);
			break;
		case IDC_SINE:
			cont->type[index] = TYPE_SINE;
			Change(TRUE);
			break;
		case IDC_TRIANGLE:
			cont->type[index] = TYPE_TRIANGLE;
			Change(TRUE);
			break;
		case IDC_SAWTOOTH:
			cont->type[index] = TYPE_SAWTOOTH;
			Change(TRUE);
			break;
		case IDC_HALFSINE:
			cont->type[index] = TYPE_HALFSINE;
			Change(TRUE);
			break;
		case IDC_CENTERED:
			cont->biasType[index] = BIAS_CENTERED;
			Change(TRUE);
			break;
		case IDC_POSITIVE:
			cont->biasType[index] = BIAS_POSITIVE;
			Change(TRUE);
			break;
		case IDC_NEGATIVE:
			cont->biasType[index] = BIAS_NEGATIVE;
			Change(TRUE);
			break;
		case IDC_MANUAL:
			cont->biasType[index] = BIAS_MANUAL;
			Change(TRUE);
			break;
		case IDC_INVERTED:
			cont->inverted[index] = IsDlgButtonChecked(hWnd,id);
			Change(TRUE);
			break;
		case IDC_FLIPPED:
			cont->flipped[index] = IsDlgButtonChecked(hWnd,id);
			Change(TRUE);
			break;
		case IDC_DISABLE:
			cont->enabled[index] = IsDlgButtonChecked(hWnd,id) ? FALSE : TRUE;
			Change(TRUE);
			break;
		case IDC_ADDFUNC:
			cont->effect[index] = EFFECT_ADD;
			Change(TRUE);
			break;
		case IDC_MULT:
			cont->effect[index] = EFFECT_MULTIPLY;
			Change(TRUE);
			break;
		case IDC_CLAMPABOVE:
			cont->effect[index] = EFFECT_CLAMP_ABOVE;
			Change(TRUE);
			break;
		case IDC_CLAMPBELOW:
			cont->effect[index] = EFFECT_CLAMP_BELOW;
			Change(TRUE);
			break;
		case IDC_THIS:
			cont->display = DISP_THIS;
			Change(TRUE);
			break;
		case IDC_THISOUT:
			cont->display = DISP_THISOUT;
			Change(TRUE);
			break;
		case IDC_FINALOUT:
			cont->display = DISP_FINALOUT;
			Change(TRUE);
			break;
		case IDC_APPEND:
			cont->Append();
			Change(TRUE);
			break;
		case IDC_INSERT:
			cont->Insert(index);
			Change(TRUE);
			break;
		case IDC_REMOVE:
			cont->Delete(index);
			Change(TRUE);
			break;
		case IDC_MOVEUP:
			cont->Move(index, -1);
			Change(TRUE);
			break;
		case IDC_MOVEDOWN:
			cont->Move(index, 1);
			Change(TRUE);
			break;
		case IDC_ABOUT:
			DialogBox(
				hInstance,
				MAKEINTRESOURCE(IDD_ABOUT),
				hWnd,
				AboutDlgProc);	
			break;
		}
	}

void WaveDlg::SpinnerChange(int id,BOOL drag)
	{
	int index = cont->index;
	switch (id) {
		case IDC_WAVE_PERIODSPIN:
			cont->period[index] = iPeriod->GetFVal();
			Change(1);
			break;

		case IDC_WAVE_DUTYSPIN:
			cont->duty[index] = iDuty->GetFVal();
			Change(1);
			break;

		case IDC_WAVE_AMPLITUDESPIN:
			cont->amplitude[index] = dim->UnConvert(iAmplitude->GetFVal());
			Change(1);
			break;

		case IDC_WAVE_PHASESPIN:
			cont->phase[index] = iPhase->GetFVal();
			Change(1);
			break;

		case IDC_WAVE_BIASSPIN:
			cont->bias[index] = dim->UnConvert(iBias->GetFVal());
			Change(1);
			break;
		}
	}

void WaveDlg::SpinnerStart(int id)
	{
	}

void WaveDlg::SpinnerEnd(int id,BOOL cancel)
	{
	ip->RedrawViews(ip->GetTime());
	}

void WaveDlg::Change(BOOL redraw)
	{
	InvalidateRect(GetDlgItem(hWnd,IDC_WAVE_GRAPH),NULL,TRUE);
	cont->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	UpdateWindow(GetDlgItem(hWnd,IDC_WAVE_GRAPH));
	UpdateWindow(GetParent(hWnd));	
	if (redraw) ip->RedrawViews(ip->GetTime());
	}


class CheckForNonWaveDlg : public DependentEnumProc {
	public:		
		BOOL non;
		ReferenceMaker *me;
		CheckForNonWaveDlg(ReferenceMaker *m) {non = FALSE;me = m;}
		int proc(ReferenceMaker *rmaker) {
			if (rmaker==me) return 0;
			if (rmaker->SuperClassID()!=REF_MAKER_CLASS_ID &&
				rmaker->ClassID()!=Class_ID(WAVEDLG_CLASS_ID,0)) {
				non = TRUE;
				return 1;
				}
			return 0;
			}
	};

void WaveDlg::MaybeCloseWindow()
	{
	CheckForNonWaveDlg check(cont);
	cont->EnumDependents(&check);
	if (!check.non) {
		PostMessage(hWnd,WM_CLOSE,0,0);
		}
	}

RefResult WaveDlg::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
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


static INT_PTR CALLBACK WaveDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	WaveDlg *dlg = (WaveDlg*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch (msg) {
		case WM_INITDIALOG:
			dlg = (WaveDlg*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
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
			dlg->WMCommand(LOWORD(wParam),HIWORD(wParam),(HWND)lParam);						
			break;

		case WM_PAINT:
			dlg->Update();
			return 0;			
		
		case WM_CLOSE:
			DestroyWindow(hWnd);			
			break;

		case WM_DESTROY:						
			delete dlg;
			break;
		
		default:
			return FALSE;
		}
	return TRUE;
	}

void BaseWaveControl::EditTrackParams(
		TimeValue t,
		ParamDimensionBase *dim,
		TCHAR *pname,
		HWND hParent,
		IObjParam *ip,
		DWORD flags)
	{
	HWND hCur = FindOpenWaveCtrlWindow(hParent,this);
	if (hCur) {
		SetForegroundWindow(hCur);
		return;
		}
	WaveDlg *dlg = new WaveDlg(this,dim,pname,ip,hParent);
	RegisterWaveCtrlWindow(dlg->hWnd,hParent,this);
	}

//--------------------------------------------------------------------------------------

BaseWaveControl::BaseWaveControl() 
	{
	waves = 0;
	Insert(0);	// Create our initial slot
	index = 0;

	display = DISP_THIS;
	range     = Interval(GetAnimStart(),GetAnimEnd());
	} 

BaseWaveControl& BaseWaveControl::operator=(const BaseWaveControl& from)
	{
	// Flush any old data
	while(waves)
		Delete(waves-1);
	// Make space for new data
	while(waves < from.waves)
		Insert(0);
	index = from.index;
	for(int i = 0; i < waves; ++i) {	
		name[i]      = from.name[i];
		period[i]    = from.period[i];
		duty[i]      = from.duty[i];
		amplitude[i] = from.amplitude[i];
		phase[i]     = from.phase[i];
		bias[i]      = from.bias[i];
		type[i]      = from.type[i];
		biasType[i]  = from.biasType[i];
		effect[i]    = from.effect[i];
		inverted[i]  = from.inverted[i];
		flipped[i]   = from.flipped[i];	
		enabled[i]   = from.enabled[i];	
		}
	display = from.display;
	range     = from.range;
	return *this;
	}

float BaseWaveControl::GetValue(float where, int ix) {
	float baseValue = 0.0f;
	if(ix < 0) {
		for(int i = 0; i < waves; ++i) {
			if(enabled[i]) {
				float tValue = GetValue(where, i);
				switch(effect[i]) {
					case EFFECT_ADD:
						baseValue += tValue;
						break;
					case EFFECT_MULTIPLY:
						baseValue *= tValue;
						break;
					case EFFECT_CLAMP_ABOVE:
						if(baseValue < tValue)
							baseValue = tValue;
						break;
					case EFFECT_CLAMP_BELOW:
						if(baseValue > tValue)
							baseValue = tValue;
						break;
					}
				}
			}
		}
	else {
		if(!enabled[ix]) {
			switch(effect[ix]) {
				case EFFECT_ADD:
					return 0.0f;
				case EFFECT_MULTIPLY:
					return 1.0f;
				case EFFECT_CLAMP_ABOVE:
					return -9999999.0f;
				case EFFECT_CLAMP_BELOW:
					return 9999999.0f;
				}
			}
		where = (float)fmod(where + phase[ix], 1.0f);
		if(flipped[ix])
			where = 1.0f - where;
		switch(type[ix]) {
			case TYPE_SQUARE:
				baseValue = (where < (duty[ix] / 100.0f)) ? amplitude[ix] : -amplitude[ix];
				break;
			case TYPE_SINE:
				baseValue = (float)sin(where * 2.0f * PI) * amplitude[ix];
				break;
			case TYPE_SAWTOOTH:
				baseValue = -amplitude[ix] + (amplitude[ix] * 2.0f * where);
				break;
			case TYPE_TRIANGLE:
				baseValue = (where < 0.5f) ? (-amplitude[ix] + (amplitude[ix] * 4.0f * where)) :
					(amplitude[ix] - (amplitude[ix] * 4.0f * (where - 0.5f)));
				break;
			case TYPE_HALFSINE:
				baseValue = -amplitude[ix] + (float)sin(where * PI) * amplitude[ix] * 2.0f;
				break;
			default:
				assert(0);
				baseValue = 0.0f;
				break;
			}

		if(inverted[ix])
			baseValue = -baseValue;

		switch(biasType[ix]) {
			case BIAS_CENTERED:
				break;
			case BIAS_POSITIVE:
				baseValue += amplitude[ix];
				break;
			case BIAS_NEGATIVE:
				baseValue -= amplitude[ix];
				break;
			case BIAS_MANUAL:
				baseValue += bias[ix];
				break;
			default:
				assert(0);
				break;
			}
		}

	return baseValue;
	}

float BaseWaveControl::WaveAtTime(TimeValue t,int ix)
	{
	if(ix < 0) {
		float theValue = 0.0f;
		for(int i = 0; i < waves; ++i) {
			if(enabled[i]) {
				float tValue = GetValue((float)fmod((float)t / 160.0f, period[i]) / period[i], i);
				switch(effect[i]) {
					case EFFECT_ADD:
						theValue += tValue;
						break;
					case EFFECT_MULTIPLY:
						theValue *= tValue;
						break;
					case EFFECT_CLAMP_ABOVE:
						if(theValue < tValue)
							theValue = tValue;
						break;
					case EFFECT_CLAMP_BELOW:
						if(theValue > tValue)
							theValue = tValue;
						break;
					}
				}
			}
		return theValue;
		}
	else {
		// Convert time to frame number and then mod it with our period
		return GetValue((float)fmod((float)t / 160.0f, period[ix]) / period[ix], ix);
		}
	}

void BaseWaveControl::HoldRange()
	{
	if (theHold.Holding() && !TestAFlag(A_HELD)) {
		SetAFlag(A_HELD);
		theHold.Put(new RangeRestore(this));
		}
	}

void BaseWaveControl::EditTimeRange(Interval range,DWORD flags)
	{
	if (!(flags&EDITRANGE_LINKTOKEYS)) {
		HoldRange();
		this->range = range;
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
	}

void BaseWaveControl::MapKeys(TimeMap *map,DWORD flags)
	{
	if (flags&TRACK_MAPRANGE) {
		HoldRange();
		TimeValue t0 = map->map(range.Start());
		TimeValue t1 = map->map(range.End());
		range.Set(t0,t1);
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
	}

void BaseWaveControl::Insert(int ix) {
	int newWaves = waves + 1;
	TSTR *newName = new TSTR [newWaves];
	float *newPeriod = new float[newWaves];
	float *newDuty = new float[newWaves];
	float *newAmplitude = new float[newWaves];
	float *newPhase = new float[newWaves];
	float *newBias = new float[newWaves];
	int *newType = new int[newWaves];
	int *newBiasType = new int[newWaves];
	int *newEffect = new int[newWaves];
	int *newInverted = new BOOL[newWaves];
	int *newFlipped = new BOOL[newWaves];
	int *newEnabled = new BOOL[newWaves];
	// Pop the old ones into the new slots
	int d = 0;
	for(int s = 0; s < ix; ++s,++d) {
		newName[d] = name[s];
		newPeriod[d] = period[s];
		newDuty[d] = duty[s];
		newAmplitude[d] = amplitude[s];
		newPhase[d] = phase[s];
		newBias[d] = bias[s];
		newType[d] = type[s];
		newBiasType[d] = biasType[s];
		newEffect[d] = effect[s];
		newInverted[d] = inverted[s];
		newFlipped[d] = flipped[s];
		newEnabled[d] = enabled[s];
		}
	// Load the new slot with default waveform
	newName[d] = TSTR("Waveform");
	newPeriod[d]    = 10.0f;
	newDuty[d]      = 50.0f;
	newAmplitude[d] = 100.0f;
	newPhase[d]     = 0.0f;
	newBias[d]      = 0.0f;
	newType[d]      = TYPE_SINE;
	newBiasType[d]  = BIAS_CENTERED;
	newEffect[d]    = EFFECT_ADD;
	newInverted[d]  = FALSE;
	newFlipped[d]   = FALSE;
	newEnabled[d]   = TRUE;
	d++;
	// Copy the following slots
	for(s = ix; s < waves; ++s,++d) {
		newName[d] = name[s];
		newPeriod[d] = period[s];
		newDuty[d] = duty[s];
		newAmplitude[d] = amplitude[s];
		newPhase[d] = phase[s];
		newBias[d] = bias[s];
		newType[d] = type[s];
		newBiasType[d] = biasType[s];
		newEffect[d] = effect[s];
		newInverted[d] = inverted[s];
		newFlipped[d] = flipped[s];
		newEnabled[d] = enabled[s];
		}
	if(waves) {
		delete [] name;
		delete [] period;
		delete [] duty;
		delete [] amplitude;
		delete [] phase;
		delete [] bias;
		delete [] type;
		delete [] biasType;
		delete [] effect;
		delete [] inverted;
		delete [] flipped;
		delete [] enabled;
		}
	name = newName;
	period = newPeriod;
	duty = newDuty;
	amplitude = newAmplitude;
	phase = newPhase;
	bias = newBias;
	type = newType;
	biasType = newBiasType;
	effect = newEffect;
	inverted = newInverted;
	flipped = newFlipped;
	enabled = newEnabled;
	waves = newWaves;
	index = ix;
	}

void BaseWaveControl::Append() {
	int newWaves = waves + 1;
	TSTR *newName = new TSTR [newWaves];
	float *newPeriod = new float[newWaves];
	float *newDuty = new float[newWaves];
	float *newAmplitude = new float[newWaves];
	float *newPhase = new float[newWaves];
	float *newBias = new float[newWaves];
	int *newType = new int[newWaves];
	int *newBiasType = new int[newWaves];
	int *newEffect = new int[newWaves];
	int *newInverted = new BOOL[newWaves];
	int *newFlipped = new BOOL[newWaves];
	int *newEnabled = new BOOL[newWaves];
	// Pop the old ones into the new slots
	int d = 0;
	for(int s = 0; s < waves; ++s,++d) {
		newName[d] = name[s];
		newPeriod[d] = period[s];
		newDuty[d] = duty[s];
		newAmplitude[d] = amplitude[s];
		newPhase[d] = phase[s];
		newBias[d] = bias[s];
		newType[d] = type[s];
		newBiasType[d] = biasType[s];
		newEffect[d] = effect[s];
		newInverted[d] = inverted[s];
		newFlipped[d] = flipped[s];
		newEnabled[d] = enabled[s];
		}
	// Load the new slot with default waveform
	newName[d] = TSTR("Waveform");
	newPeriod[d]    = 10.0f;
	newDuty[d]      = 50.0f;
	newAmplitude[d] = 100.0f;
	newPhase[d]     = 0.0f;
	newBias[d]      = 0.0f;
	newType[d]      = TYPE_SINE;
	newBiasType[d]  = BIAS_CENTERED;
	newEffect[d]    = EFFECT_ADD;
	newInverted[d]  = FALSE;
	newFlipped[d]   = FALSE;
	newEnabled[d]   = TRUE;
	if(waves) {
		delete [] name;
		delete [] period;
		delete [] duty;
		delete [] amplitude;
		delete [] phase;
		delete [] bias;
		delete [] type;
		delete [] biasType;
		delete [] effect;
		delete [] inverted;
		delete [] flipped;
		delete [] enabled;
		}
	name = newName;
	period = newPeriod;
	duty = newDuty;
	amplitude = newAmplitude;
	phase = newPhase;
	bias = newBias;
	type = newType;
	biasType = newBiasType;
	effect = newEffect;
	inverted = newInverted;
	flipped = newFlipped;
	enabled = newEnabled;
	waves = newWaves;
	index = waves - 1;
	}

void BaseWaveControl::Delete(int ix) {
	assert(ix >= 0 && ix < waves);
	int newWaves = waves - 1;
	TSTR *newName = NULL;
	float *newPeriod = NULL;
	float *newDuty = NULL;
	float *newAmplitude = NULL;
	float *newPhase = NULL;
	float *newBias = NULL;
	int *newType = NULL;
	int *newBiasType = NULL;
	int *newEffect = NULL;
	int *newInverted = NULL;
	int *newFlipped = NULL;
	int *newEnabled = NULL;
	if(newWaves) {
		newName = new TSTR [newWaves];
		newPeriod = new float[newWaves];
		newDuty = new float[newWaves];
		newAmplitude = new float[newWaves];
		newPhase = new float[newWaves];
		newBias = new float[newWaves];
		newType = new int[newWaves];
		newBiasType = new int[newWaves];
		newEffect = new int[newWaves];
		newInverted = new BOOL[newWaves];
		newFlipped = new BOOL[newWaves];
		newEnabled = new BOOL[newWaves];
		// Pop the old ones into the new slots
		int d = 0;
		for(int s = 0; s < waves; ++s) {
			if(s != ix) {
				newName[d] = name[s];
				newPeriod[d] = period[s];
				newDuty[d] = duty[s];
				newAmplitude[d] = amplitude[s];
				newPhase[d] = phase[s];
				newBias[d] = bias[s];
				newType[d] = type[s];
				newBiasType[d] = biasType[s];
				newEffect[d] = effect[s];
				newInverted[d] = inverted[s];
				newFlipped[d] = flipped[s];
				newEnabled[d] = enabled[s];
				d++;
				}
			}
		}
	delete [] name;
	delete [] period;
	delete [] duty;
	delete [] amplitude;
	delete [] phase;
	delete [] bias;
	delete [] type;
	delete [] biasType;
	delete [] effect;
	delete [] inverted;
	delete [] flipped;
	delete [] enabled;
	name = newName;
	period = newPeriod;
	duty = newDuty;
	amplitude = newAmplitude;
	phase = newPhase;
	bias = newBias;
	type = newType;
	biasType = newBiasType;
	effect = newEffect;
	inverted = newInverted;
	flipped = newFlipped;
	enabled = newEnabled;
	waves = newWaves;
	if(index >= waves)
		index = waves - 1;
	}

void BaseWaveControl::Move(int ix,int direction) {
	int dest = ix + direction;
	TSTR holdName = name[dest];
	float holdPeriod = period[dest];
	float holdDuty = duty[dest];
	float holdAmplitude = amplitude[dest];
	float holdPhase = phase[dest];
	float holdBias = bias[dest];
	int holdType = type[dest];
	int holdBiasType = biasType[dest];
	int holdEffect = effect[dest];
	int holdInverted = inverted[dest];
	int holdFlipped = flipped[dest];
	int holdEnabled = enabled[dest];

	name[dest] = name[ix];
	period[dest] = period[ix];
	duty[dest] = duty[ix];
	amplitude[dest] = amplitude[ix];
	phase[dest] = phase[ix];
	bias[dest] = bias[ix];
	type[dest] = type[ix];
	biasType[dest] = biasType[ix];
	effect[dest] = effect[ix];
	inverted[dest] = inverted[ix];
	flipped[dest] = flipped[ix];
	enabled[dest] = enabled[ix];

	name[ix] = holdName;
	period[ix] = holdPeriod;
	duty[ix] = holdDuty;
	amplitude[ix] = holdAmplitude;
	phase[ix] = holdPhase;
	bias[ix] = holdBias;
	type[ix] = holdType;
	biasType[ix] = holdBiasType;
	effect[ix] = holdEffect;
	inverted[ix] = holdInverted;
	flipped[ix] = holdFlipped;
	enabled[ix] = holdEnabled;

	index = dest;
	}

static float highestOf(float a, float b, float c, float d) {
	float maxab = (a > b) ? a : b;
	float maxcd = (c > d) ? c : d;
	return (maxab > maxcd) ? maxab : maxcd;
	}

static float lowestOf(float a, float b, float c, float d) {
	float minab = (a < b) ? a : b;
	float mincd = (c < d) ? c : d;
	return (minab < mincd) ? minab : mincd;
	}

void BaseWaveControl::GetRange(float *low, float *high, int index) {
	assert(index >= 0 && index < waves);
	switch(biasType[index]) {
		case BIAS_CENTERED:
			if(low)
				*low = -amplitude[index];
			if(high)
				*high = amplitude[index];
			break;
		case BIAS_POSITIVE:
			if(low)
				*low = 0.0f;
			if(high)
				*high = 2.0f * amplitude[index];
			break;
		case BIAS_NEGATIVE:
			if(low)
				*low = -2.0f * amplitude[index];
			if(high)
				*high = 0.0f;
			break;
		case BIAS_MANUAL:
		default:
			if(low)
				*low = -amplitude[index] + bias[index];
			if(high)
				*high = amplitude[index] + bias[index];
			break;
		}
	}

void BaseWaveControl::GetDisplayRanges(float *lo, float *hi) {
	int i;
	float low = 0.0f, high = 0.0f;
	switch(display) {
		case DISP_THIS:
			GetRange(lo, hi, index);
			return;
		case DISP_THISOUT:
			for(i = 0; i <= index; ++i) {
				float l,h;
				if(enabled[i]) {
					GetRange(&l, &h, i);
					switch(effect[i]) {
						case EFFECT_ADD:
							high += h;
							low += l;
							break;
						case EFFECT_MULTIPLY: {
							float hh = high * h;
							float hl = high * l;
							float ll = low * l;
							float lh = low * h;
							high = highestOf(hh, hl, ll, lh);
							low = lowestOf(hh, hl, ll, lh);
							}
							break;
						case EFFECT_CLAMP_ABOVE:
							if(low < l)
								low = l;
							if(high < l)
								high = l;
							break;
						case EFFECT_CLAMP_BELOW:
							if(low > h)
								low = h;
							if(high > h)
								high = h;
							break;
						}
					}
				}
			break;
		case DISP_FINALOUT:
			for(i = 0; i < waves; ++i) {
				float l,h;
				if(enabled[i]) {
					GetRange(&l, &h, i);
					switch(effect[i]) {
						case EFFECT_ADD:
							high += h;
							low += l;
							break;
						case EFFECT_MULTIPLY: {
							float hh = high * h;
							float hl = high * l;
							float ll = low * l;
							float lh = low * h;
							high = highestOf(hh, hl, ll, lh);
							low = lowestOf(hh, hl, ll, lh);
							}
							break;
						case EFFECT_CLAMP_ABOVE:
							if(low < l)
								low = l;
							if(high < l)
								high = l;
							break;
						case EFFECT_CLAMP_BELOW:
							if(low > h)
								low = h;
							if(high > h)
								high = h;
							break;
						}
					}
				}
			break;
		}
	*lo = low;
	*hi = high;
	}

#define WC_WAVES_CHUNK		0x0100
#define WC_INDEX_CHUNK		0x0110
#define WC_NAME_CHUNK		0x0120
#define WC_PERIOD_CHUNK		0x0130
#define WC_DUTY_CHUNK		0x0135
#define WC_AMPLITUDE_CHUNK	0x0140
#define WC_PHASE_CHUNK		0x0150
#define WC_BIAS_CHUNK		0x0160
#define WC_TYPE_CHUNK		0x0170
#define WC_BIASTYPE_CHUNK	0x0180
#define WC_EFFECT_CHUNK		0x0190
#define WC_INVERTED_CHUNK	0x01A0
#define WC_FLIPPED_CHUNK	0x01B0
#define WC_ENABLED_CHUNK	0x01B5
#define WC_DISPLAY_CHUNK	0x01C0
#define WC_RANGE_CHUNK		0x01D0
#define WC_WAVE_ENTRY0		0x1000
#define WC_WAVE_ENTRYN		0x1FFF

IOResult BaseWaveControl::Save(ISave *isave)
	{		
	ULONG nb;	
	int i;

	isave->BeginChunk(WC_WAVES_CHUNK);
	isave->Write(&waves,sizeof(waves),&nb);
	isave->EndChunk();
	
	isave->BeginChunk(WC_INDEX_CHUNK);
	isave->Write(&index,sizeof(index),&nb);
	isave->EndChunk();
	
	for(i = 0; i < waves; ++i) {
		isave->BeginChunk(WC_WAVE_ENTRY0 + i);

		isave->BeginChunk(WC_NAME_CHUNK);
		isave->WriteCString(name[i]);
		isave->EndChunk();

		isave->BeginChunk(WC_PERIOD_CHUNK);
		isave->Write(&period[i],sizeof(float),&nb);
		isave->EndChunk();
	
		isave->BeginChunk(WC_DUTY_CHUNK);
		isave->Write(&duty[i],sizeof(float),&nb);
		isave->EndChunk();
	
		isave->BeginChunk(WC_AMPLITUDE_CHUNK);
		isave->Write(&amplitude[i],sizeof(float),&nb);
		isave->EndChunk();
	
		isave->BeginChunk(WC_PHASE_CHUNK);
		isave->Write(&phase[i],sizeof(float),&nb);
		isave->EndChunk();
	
		isave->BeginChunk(WC_BIAS_CHUNK);
		isave->Write(&bias[i],sizeof(float),&nb);
		isave->EndChunk();
	
		isave->BeginChunk(WC_TYPE_CHUNK);
		isave->Write(&type[i],sizeof(int),&nb);
		isave->EndChunk();
	
		isave->BeginChunk(WC_BIASTYPE_CHUNK);
		isave->Write(&biasType[i],sizeof(int),&nb);
		isave->EndChunk();
	
		isave->BeginChunk(WC_EFFECT_CHUNK);
		isave->Write(&effect[i],sizeof(int),&nb);
		isave->EndChunk();
	
		isave->BeginChunk(WC_INVERTED_CHUNK);
		isave->Write(&inverted[i],sizeof(BOOL),&nb);
		isave->EndChunk();
	
		isave->BeginChunk(WC_FLIPPED_CHUNK);
		isave->Write(&flipped[i],sizeof(BOOL),&nb);
		isave->EndChunk();

		isave->BeginChunk(WC_ENABLED_CHUNK);
		isave->Write(&enabled[i],sizeof(BOOL),&nb);
		isave->EndChunk();

		isave->EndChunk();
		}

	isave->BeginChunk(WC_DISPLAY_CHUNK);
	isave->Write(&display,sizeof(int),&nb);
	isave->EndChunk();

	isave->BeginChunk(WC_RANGE_CHUNK);
	isave->Write(&range,sizeof(range),&nb);
	isave->EndChunk();
	
	return IO_OK;
	}

IOResult BaseWaveControl::Load(ILoad *iload)
	{
	ULONG nb;
	int id, i;
	TCHAR *cp;
	IOResult res = IO_OK;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (id = iload->CurChunkID()) {
			case WC_WAVES_CHUNK: {
				int newWaves;
				while(waves)
					Delete(waves-1);
				res=iload->Read(&newWaves,sizeof(int),&nb);
				assert(newWaves);
				while(waves < newWaves)
					Insert(0);
				}
				break;

			case WC_INDEX_CHUNK:
				res=iload->Read(&index,sizeof(index),&nb);
				break;

			case WC_DISPLAY_CHUNK:
				res=iload->Read(&display,sizeof(int),&nb);
				break;
			case WC_RANGE_CHUNK:
				res=iload->Read(&range,sizeof(range),&nb);
				break;
			}
		if(id >= WC_WAVE_ENTRY0 && id <= WC_WAVE_ENTRYN) {
			i = id - WC_WAVE_ENTRY0;
			assert(i < waves);
			while (IO_OK == iload->OpenChunk()) {
				switch (iload->CurChunkID()) {
				case WC_NAME_CHUNK:
					iload->ReadCStringChunk(&cp);
					name[i] = cp;
					break;

				case WC_PERIOD_CHUNK:
					res=iload->Read(&period[i],sizeof(float),&nb);
					break;

				case WC_DUTY_CHUNK:
					res=iload->Read(&duty[i],sizeof(float),&nb);
					break;

				case WC_AMPLITUDE_CHUNK:
					res=iload->Read(&amplitude[i],sizeof(float),&nb);
					break;

				case WC_PHASE_CHUNK:
					res=iload->Read(&phase[i],sizeof(float),&nb);
					break;
				case WC_BIAS_CHUNK:
					res=iload->Read(&bias[i],sizeof(float),&nb);
					break;
				case WC_TYPE_CHUNK:
					res=iload->Read(&type[i],sizeof(int),&nb);
					break;
				case WC_BIASTYPE_CHUNK:
					res=iload->Read(&biasType[i],sizeof(int),&nb);
					break;
				case WC_EFFECT_CHUNK:
					res=iload->Read(&effect[i],sizeof(int),&nb);
					break;
				case WC_INVERTED_CHUNK:
					res=iload->Read(&inverted[i],sizeof(BOOL),&nb);
					break;
				case WC_FLIPPED_CHUNK:
					res=iload->Read(&flipped[i],sizeof(BOOL),&nb);
					break;
				case WC_ENABLED_CHUNK:
					res=iload->Read(&enabled[i],sizeof(int),&nb);
					break;
				}	
				iload->CloseChunk();
			}
		}

		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}
	return IO_OK;
	}


//--------------------------------------------------------------------

RefTargetHandle FloatWaveControl::Clone(RemapDir& remap)
	{
	// make a new controller and give it our param values.
	FloatWaveControl *cont = new FloatWaveControl;
	*cont = *this;	
	BaseClone(this, cont, remap);
	return cont;
	}

void FloatWaveControl::GetValueLocalTime(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
	// This controller is always changing.
	valid.SetInstant(t);
	
	*((float*)val) = WaveAtTime(t,-1);
	}

void FloatWaveControl::Extrapolate(
		Interval range,TimeValue t,void *val,Interval &valid,int type)
	{
	float val0, val1, val2, res;
	switch (type) {
		case ORT_LINEAR:			
			if (t<range.Start()) {
				GetValueLocalTime(range.Start(),&val0,valid);
				GetValueLocalTime(range.Start()+1,&val1,valid);
				res = LinearExtrapolate(range.Start(),t,val0,val1,val0);				
			} else {
				GetValueLocalTime(range.End()-1,&val0,valid);
				GetValueLocalTime(range.End(),&val1,valid);
				res = LinearExtrapolate(range.End(),t,val0,val1,val1);
				}
			break;

		case ORT_IDENTITY:
			if (t<range.Start()) {
				GetValueLocalTime(range.Start(),&val0,valid);
				res = IdentityExtrapolate(range.Start(),t,val0);
			} else {
				GetValueLocalTime(range.End(),&val0,valid);
				res = IdentityExtrapolate(range.End(),t,val0);
				}
			break;

		case ORT_RELATIVE_REPEAT:
			GetValueLocalTime(range.Start(),&val0,valid);
			GetValueLocalTime(range.End(),&val1,valid);
			GetValueLocalTime(CycleTime(range,t),&val2,valid);
			res = RepeatExtrapolate(range,t,val0,val1,val2);			
			break;
		}
	valid.Set(t,t);
	*((float*)val) = res;
	}

