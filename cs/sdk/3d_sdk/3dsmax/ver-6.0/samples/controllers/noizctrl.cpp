/**********************************************************************
 *<
	FILE: noizctrl.cpp

	DESCRIPTION: A simple noise controller

	CREATED BY: Rolf Berteig

	HISTORY: created 26 August 1995

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "ctrl.h"
#include "units.h"
#include "noise.h"
#include "istdplug.h"
#include "iColorMan.h"

#include "notify.h"

#define FLOATNOISE_CONTROL_CNAME	GetString(IDS_RB_NOISEFLOAT)
#define POSITIONNOISE_CONTROL_CNAME	GetString(IDS_RB_NOISEPOSITION)
#define POINT3NOISE_CONTROL_CNAME	GetString(IDS_RB_NOISEPOINT3)
#define ROTATIONNOISE_CONTROL_CNAME	GetString(IDS_RB_NOISEROTATION)
#define SCALENOISE_CONTROL_CNAME	GetString(IDS_RB_NOISESCALE)



#define MAX_ELEMS	3
										   
class BaseNoiseControl : public INoiseControl {
	public:		
		//float strength[MAX_ELEMS];
		Control *cont; // make strength animatable.
		float frequency;
		float roughness;
		int seed;
		BOOL fractal;
		BOOL lim[MAX_ELEMS];
		Interval range;
		TimeValue rampin, rampout;

		BaseNoiseControl();
		BaseNoiseControl& operator=(const BaseNoiseControl& from);
		float NoiseAtTime(TimeValue t,int s,int index);
		
		virtual int Elems()=0;

		// Animatable methods		
		void DeleteThis() {delete this;}		
		int IsKeyable() {return 0;}		
		BOOL IsAnimated() {return TRUE;}

		int NumSubs();
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		BOOL AssignController(Animatable *control,int subAnim);
		int SubNumToRefNum(int subNum);

		int NumRefs();
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

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
		
		// From INoiseControl
		void SetSeed(int seed);		
		void SetFrequency(float f);		
		void SetFractal(BOOL f);		
		void SetRoughness(float f);		
		void SetRampIn(TimeValue in);
		void SetRampOut(TimeValue out);		
		void SetPositiveOnly(int which,BOOL onOff);		
		void SetStrengthController(Control *c);

		float GetRoughness() {return roughness;}
		int GetSeed() {return seed;}
		float GetFrequency() {return frequency;}
		BOOL GetFractal() {return fractal;}
		TimeValue GetRampIn() {return rampin;}
		TimeValue GetRampOut() {return rampout;}
		BOOL GetPositiveOnly(int which) {return lim[which];}
		Control *GetStrengthController() {return cont;}

		void HoldRange();
		Interval GetTimeRange(DWORD flags) {return range;}
		void EditTimeRange(Interval range,DWORD flags);
		void MapKeys(TimeMap *map,DWORD flags );

		void GetStrength(TimeValue t,float *strength) {
			if (cont) cont->GetValue(t,strength,FOREVER);
			}
	};

//---------------------------------------------------------------------------

class FloatNoiseControl : public BaseNoiseControl {
	public:
		int Elems() {return 1;}

		FloatNoiseControl() {
			cont=NULL;
			ReplaceReference(Control::NumRefs(),NewDefaultFloatController());
			float v = 5.0f;
			cont->SetValue(0,&v);
			}

		Class_ID ClassID() { return Class_ID(FLOATNOISE_CONTROL_CLASS_ID,0); }  
		SClass_ID SuperClassID() { return CTRL_FLOAT_CLASS_ID; } 
		void GetClassName(TSTR& s) {s = FLOATNOISE_CONTROL_CNAME;}

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


class FloatNoiseClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new FloatNoiseControl(); }
	const TCHAR *	ClassName() { return FLOATNOISE_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_FLOAT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(FLOATNOISE_CONTROL_CLASS_ID,0); }
	const TCHAR* 	Category() { return _T("");  }
	};
static FloatNoiseClassDesc floatNoiseCD;
ClassDesc* GetFloatNoiseDesc() {return &floatNoiseCD;}

//-----------------------------------------------------------------------------

class PositionNoiseControl : public BaseNoiseControl {
	public:
		int Elems() {return 3;}

		PositionNoiseControl() {
			cont=NULL;
			ReplaceReference(Control::NumRefs(),NewDefaultPoint3Controller());
			Point3 v(50.0f,50.0f,50.0f);			
			cont->SetValue(0,&v);
			}

		Class_ID ClassID() { return Class_ID(POSITIONNOISE_CONTROL_CLASS_ID,0); }  
		SClass_ID SuperClassID() { return CTRL_POSITION_CLASS_ID; } 
		void GetClassName(TSTR& s) {s = POSITIONNOISE_CONTROL_CNAME;}

		// Control methods
		RefTargetHandle Clone(RemapDir& remap);

		// StdControl methods
		void GetValueLocalTime(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void SetValueLocalTime(TimeValue t, void *val, int commit, GetSetMethod method) {}		
		void Extrapolate(Interval range,TimeValue t,void *val,Interval &valid,int type);
		void *CreateTempValue() {return new Point3;}
		void DeleteTempValue(void *val) {delete (Point3*)val;}
		void ApplyValue(void *val, void *delta) {((Matrix3*)val)->PreTranslate(*((Point3*)delta));}
		void MultiplyValue(void *val, float m) {*((Point3*)val) *= m;}
	};


class PositionNoiseClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new PositionNoiseControl(); }
	const TCHAR *	ClassName() { return POSITIONNOISE_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_POSITION_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(POSITIONNOISE_CONTROL_CLASS_ID,0); }
	const TCHAR* 	Category() { return _T("");  }
	};
static PositionNoiseClassDesc positionNoiseCD;
ClassDesc* GetPositionNoiseDesc() {return &positionNoiseCD;}

//-----------------------------------------------------------------------------

class Point3NoiseControl : public BaseNoiseControl {
	public:
		int Elems() {return 3;}

		Point3NoiseControl() {
			cont=NULL;
			ReplaceReference(Control::NumRefs(),NewDefaultPoint3Controller());
			Point3 v(50.0f,50.0f,50.0f);			
			cont->SetValue(0,&v);
			}

		Class_ID ClassID() { return Class_ID(POINT3NOISE_CONTROL_CLASS_ID,0); }  
		SClass_ID SuperClassID() { return CTRL_POINT3_CLASS_ID; } 
		void GetClassName(TSTR& s) {s = POINT3NOISE_CONTROL_CNAME;}

		// Control methods
		RefTargetHandle Clone(RemapDir& remap);

		// StdControl methods
		void GetValueLocalTime(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void SetValueLocalTime(TimeValue t, void *val, int commit, GetSetMethod method) {}		
		void Extrapolate(Interval range,TimeValue t,void *val,Interval &valid,int type);
		void *CreateTempValue() {return new Point3;}
		void DeleteTempValue(void *val) {delete (Point3*)val;}
		void ApplyValue(void *val, void *delta) {*((Point3*)val) += *((Point3*)delta);}
		void MultiplyValue(void *val, float m) {*((Point3*)val) *= m;}
	};


class Point3NoiseClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new Point3NoiseControl(); }
	const TCHAR *	ClassName() { return POINT3NOISE_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_POINT3_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(POINT3NOISE_CONTROL_CLASS_ID,0); }
	const TCHAR* 	Category() { return _T("");  }
	};
static Point3NoiseClassDesc point3NoiseCD;
ClassDesc* GetPoint3NoiseDesc() {return &point3NoiseCD;}

//-----------------------------------------------------------------------------


class RotationNoiseControl : public BaseNoiseControl {
	public:
		int Elems() {return 3;}
		
		RotationNoiseControl() {
			Point3 strength;
			strength[0] = strength[1] = strength[2] = DegToRad(45);
			cont=NULL;
			ReplaceReference(Control::NumRefs(),NewDefaultPoint3Controller());
			cont->SetValue(0,&strength); 
			}

		Class_ID ClassID() { return Class_ID(ROTATIONNOISE_CONTROL_CLASS_ID,0); }  
		SClass_ID SuperClassID() { return CTRL_ROTATION_CLASS_ID; } 
		void GetClassName(TSTR& s) {s = ROTATIONNOISE_CONTROL_CNAME;}

		// Control methods
		RefTargetHandle Clone(RemapDir& remap);

		// StdControl methods
		void GetValueLocalTime(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void SetValueLocalTime(TimeValue t, void *val, int commit, GetSetMethod method) {}		
		void Extrapolate(Interval range,TimeValue t,void *val,Interval &valid,int type);
		void *CreateTempValue() {return new Quat;}
		void DeleteTempValue(void *val) {delete (Quat*)val;}
		void ApplyValue(void *val, void *delta) {PreRotateMatrix( *((Matrix3*)val), *((Quat*)delta) );}
		void MultiplyValue(void *val, float m) {*((Quat*)val) *= m;}
	};


class RotationNoiseClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new RotationNoiseControl(); }
	const TCHAR *	ClassName() { return ROTATIONNOISE_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_ROTATION_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(ROTATIONNOISE_CONTROL_CLASS_ID,0); }
	const TCHAR* 	Category() { return _T("");  }
	};
static RotationNoiseClassDesc rotationNoiseCD;
ClassDesc* GetRotationNoiseDesc() {return &rotationNoiseCD;}

//-----------------------------------------------------------------------------

class ScaleNoiseControl : public BaseNoiseControl {
	public:
		int Elems() {return 3;}

		ScaleNoiseControl() {
			Point3 strength;
			strength[0] = strength[1] = strength[2] = 0.5f;
			cont=NULL;
			ReplaceReference(Control::NumRefs(),NewDefaultPoint3Controller());
			cont->SetValue(0,&strength); 
			}

		Class_ID ClassID() { return Class_ID(SCALENOISE_CONTROL_CLASS_ID,0); }  
		SClass_ID SuperClassID() { return CTRL_SCALE_CLASS_ID; } 
		void GetClassName(TSTR& s) {s = SCALENOISE_CONTROL_CNAME;}

		// Control methods
		RefTargetHandle Clone(RemapDir& remap);

		// StdControl methods
		void GetValueLocalTime(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void SetValueLocalTime(TimeValue t, void *val, int commit, GetSetMethod method) {}		
		void Extrapolate(Interval range,TimeValue t,void *val,Interval &valid,int type);
		void *CreateTempValue() {return new ScaleValue;}
		void DeleteTempValue(void *val) {delete (ScaleValue*)val;}
		void ApplyValue(void *val, void *delta) {ApplyScaling( *((Matrix3*)val), *((ScaleValue*)delta) );}
		void MultiplyValue(void *val, float m) {*((ScaleValue*)val) *= m;}
	};


class ScaleNoiseClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new ScaleNoiseControl(); }
	const TCHAR *	ClassName() { return SCALENOISE_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_SCALE_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(SCALENOISE_CONTROL_CLASS_ID,0);}
	const TCHAR* 	Category() { return _T("");  }
	};
static ScaleNoiseClassDesc scaleNoiseCD;
ClassDesc* GetScaleNoiseDesc() {return &scaleNoiseCD;}


//-----------------------------------------------------------------------------


class RangeRestore : public RestoreObj {
	public:
		BaseNoiseControl *cont;
		Interval ur, rr;
		RangeRestore(BaseNoiseControl *c) 
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
		TSTR Description() { return TSTR(_T("Noise control range")); }
	};


//-----------------------------------------------------------------
//
// UI

static LRESULT CALLBACK NoiseGraphWinProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static void InitNoiseGraph(HINSTANCE hInst)
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
    wc.lpfnWndProc   = NoiseGraphWinProc;
    wc.lpszClassName = _T("NoiseGraph");
		
	RegisterClass(&wc);	
	}


#define STEPSIZE			4
#define GRAPH_TIMESCALE		20

static void DrawNoiseGraph(BaseNoiseControl *cont,HWND hWnd,HDC hdc)
	{
	Rect rect, orect;
	GetClientRect(hWnd,&rect);
	orect = rect;
	float v, fy;
	int mid = rect.h()/2;
	rect.top    += 5;
	rect.bottom -= 5;
	TimeValue end = rect.right * GRAPH_TIMESCALE;

	SelectObject(hdc,CreatePen(PS_DOT,0,GetCustSysColor(COLOR_BTNFACE)));
	
	MoveToEx(hdc,0,mid,NULL);
	LineTo(hdc,rect.right,mid);
	MoveToEx(hdc,cont->rampin / GRAPH_TIMESCALE,0,NULL);
	LineTo(hdc,cont->rampin / GRAPH_TIMESCALE,rect.bottom);
	MoveToEx(hdc,rect.right - cont->rampout / GRAPH_TIMESCALE,0,NULL);
	LineTo(hdc,rect.right - cont->rampout / GRAPH_TIMESCALE,rect.bottom);

	DeleteObject(SelectObject(hdc,GetStockObject(BLACK_PEN)));

	for (int x = 0; x < rect.right + STEPSIZE; x += STEPSIZE) {
		int t = x*GRAPH_TIMESCALE;
		float ramp = 1.0f;
		
		// mjm 9.28.98
		// add a check for div by zero on cont->rampin
		// since x>=0, then t>=0, so t should never be < cont->rampin when cont->rampin is 0
		// but fix is added for symmetry with fix below
		if ( (cont->rampin != 0) && (t<cont->rampin) )
		{
			float u = float(t)/float(cont->rampin);
			ramp *= u*u * (3.0f-2.0f*u);
		}
		// mjm 9.28.98
		// bugfix -- add a check for div by zero on cont->rampout
		// because of STEPSIZE and GRAPH_TIMESCALE, t jumps from being < end to being > end.
		// when cont->rampout equals 0, (its default setting), then (t>end-cont->rampout) is TRUE,
		// leading to the div by zero
		if ( (cont->rampout != 0) && (t>end-cont->rampout) )
		{
			float u = float(end-t)/float(cont->rampout);
			ramp *= u*u * (3.0f-2.0f*u);
		}

		v = float(t) * float(0.005) * cont->frequency + Perm(cont->seed);
		if (cont->fractal) {
			fy = (float)fBm1(v, 1.0f-cont->roughness, 2.0f, 6 /*octaves*/);
		} else {
			fy = noise1(v);
			}
		fy *= ramp;

		if (!x) {
			MoveToEx(hdc,0,mid + int((rect.h()) * fy),NULL);
		} else {
			LineTo(hdc,x,mid + int((rect.h()) * fy));
			}
		}

	WhiteRect3D(hdc,orect,TRUE);
	}

static LRESULT CALLBACK NoiseGraphWinProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	BaseNoiseControl *cont = (BaseNoiseControl*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	if (!cont) return DefWindowProc(hWnd,msg,wParam,lParam);

	switch (msg) {		
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd,&ps);
			DrawNoiseGraph(cont,hWnd,hdc);
			EndPaint(hWnd,&ps);
			break;
			}

		default:
			return DefWindowProc(hWnd,msg,wParam,lParam);
		}
	
	return 0;
	}

//--------------------------------------------------------------

static INT_PTR CALLBACK NoiseDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static int strengthSpinID[] = {IDC_NOISE_XSTRENGTHSPIN,IDC_NOISE_YSTRENGTHSPIN,IDC_NOISE_ZSTRENGTHSPIN};
static int strengthEditID[] = {IDC_NOISE_XSTRENGTH,IDC_NOISE_YSTRENGTH,IDC_NOISE_ZSTRENGTH};
static int strengthLabelID[] = {IDC_NOISE_XSTRENGTHLABEL,IDC_NOISE_YSTRENGTHLABEL,IDC_NOISE_ZSTRENGTHLABEL};
static int limID[] = {IDC_NOISE_XLIM0,IDC_NOISE_YLIM0,IDC_NOISE_ZLIM0};

#define NOISEDLG_CLASS_ID	0xaab659c4


class NoiseCtrlWindow {
	public:
		HWND hWnd;
		HWND hParent;
		Control *cont;
		NoiseCtrlWindow() {assert(0);}
		NoiseCtrlWindow(HWND hWnd,HWND hParent,Control *cont)
			{this->hWnd=hWnd; this->hParent=hParent; this->cont=cont;}
	};
static Tab<NoiseCtrlWindow> noiseCtrlWindows;

static void RegisterNoiseCtrlWindow(HWND hWnd, HWND hParent, Control *cont)
	{
	NoiseCtrlWindow rec(hWnd,hParent,cont);
	noiseCtrlWindows.Append(1,&rec);
	}

static void UnRegisterNoiseCtrlWindow(HWND hWnd)
	{	
	for (int i=0; i<noiseCtrlWindows.Count(); i++) {
		if (hWnd==noiseCtrlWindows[i].hWnd) {
			noiseCtrlWindows.Delete(i,1);
			return;
			}
		}	
	}

static HWND FindOpenNoiseCtrlWindow(HWND hParent,Control *cont)
	{	
	for (int i=0; i<noiseCtrlWindows.Count(); i++) {
		if (hParent == noiseCtrlWindows[i].hParent &&
			cont    == noiseCtrlWindows[i].cont) {
			return noiseCtrlWindows[i].hWnd;
			}
		}
	return NULL;
	}

class NoiseDlg : public ReferenceMaker, public TimeChangeCallback {
	public:
		BaseNoiseControl *cont;	
		ParamDimensionBase *dim;
		IObjParam *ip;
		HWND hWnd;
		BOOL valid;
		int elems;
		ISpinnerControl *iStrength[MAX_ELEMS];
		ISpinnerControl *iSeed, *iFreq, *iRough, *iRampIn, *iRampOut;

		NoiseDlg(
			BaseNoiseControl *cont,
			ParamDimensionBase *dim,
			TCHAR *pname,
			IObjParam *ip,
			HWND hParent);
		~NoiseDlg();

		Class_ID ClassID() {return Class_ID(NOISEDLG_CLASS_ID,0);}
		SClass_ID SuperClassID() {return REF_MAKER_CLASS_ID;}

		void MaybeCloseWindow();

		void TimeChanged(TimeValue t) {Invalidate();}

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
		void SetReference(int i, RefTargetHandle rtarg) {cont=(BaseNoiseControl*)rtarg;}

		static void NotifyPreReset(void* param, NotifyInfo*);
	};


NoiseDlg::NoiseDlg(
		BaseNoiseControl *cont,
		ParamDimensionBase *dim,
		TCHAR *pname,
		IObjParam *ip,
		HWND hParent)
	{
	InitNoiseGraph(hInstance);
	this->cont = cont;
	this->ip   = ip;
	this->dim  = dim;
	valid = FALSE;
	elems = cont->Elems();

	theHold.Suspend();
	MakeRefByID(FOREVER,0,cont);
	theHold.Resume();

	hWnd = CreateDialogParam(
		hInstance,
		MAKEINTRESOURCE(IDD_NOISEPARAMS),
		hParent,
		NoiseDlgProc,
		(LPARAM)this);	
	TSTR title = TSTR(GetString(IDS_RB_NOISECONTROLTITLE)) + TSTR(pname);
	SetWindowText(hWnd,title);
	ip->RegisterTimeChangeCallback(this);

	RegisterNotification(NotifyPreReset, this, NOTIFY_SYSTEM_PRE_RESET);

	}

NoiseDlg::~NoiseDlg()
	{
	UnRegisterNoiseCtrlWindow(hWnd);
	ip->UnRegisterTimeChangeCallback(this);
	UnRegisterNotification(NotifyPreReset, this,	NOTIFY_SYSTEM_PRE_RESET);

	theHold.Suspend();
	DeleteAllRefsFromMe();
	theHold.Resume();

	ReleaseISpinner(iSeed);
	ReleaseISpinner(iFreq);
	ReleaseISpinner(iRough);
	ReleaseISpinner(iRampIn);
	ReleaseISpinner(iRampOut);
	for (int i=0; i<elems; i++) {
		ReleaseISpinner(iStrength[i]);
		}
	}

void NoiseDlg::NotifyPreReset(void* param, NotifyInfo*)
	{
	NoiseDlg* dlg = (NoiseDlg*)param;
	if (dlg)
		PostMessage(dlg->hWnd,WM_CLOSE,0,0);
	}

void NoiseDlg::Invalidate()
	{
	valid = FALSE;
	InvalidateRect(hWnd,NULL,FALSE);	
	InvalidateRect(GetDlgItem(hWnd,IDC_NOISE_GRAPH),NULL,FALSE);
	}

void NoiseDlg::Update()
	{
	if (!valid && hWnd) {
		if (cont->cont == NULL) 
			{
			PostMessage(hWnd,WM_CLOSE,0,0);
			return;
			}
		float strength[MAX_ELEMS];
		cont->GetStrength(ip->GetTime(),strength);

		for (int i=0; i<elems; i++) {			
			iStrength[i]->SetValue(dim->Convert(strength[i]),FALSE);
			iStrength[i]->SetKeyBrackets( cont->cont->IsKeyAtTime(ip->GetTime(), 0) ); // mjm - 3.1.99
			CheckDlgButton(hWnd,limID[i],cont->lim[i]);
			}
		iSeed->SetValue(cont->seed,FALSE);
		iFreq->SetValue(cont->frequency,FALSE);
		iRough->SetValue(cont->roughness,FALSE);
		iRampIn->SetValue(cont->rampin,FALSE);
		iRampOut->SetValue(cont->rampout,FALSE);
		if (cont->fractal) {
			CheckDlgButton(hWnd,IDC_NOISE_FRACTAL,TRUE);
			EnableWindow(GetDlgItem(hWnd,IDC_NOISE_ROUGHLABEL),TRUE);
			EnableWindow(GetDlgItem(hWnd,IDC_NOISE_ROUGHSPIN),TRUE);
		} else {
			CheckDlgButton(hWnd,IDC_NOISE_FRACTAL,FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_NOISE_ROUGHLABEL),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_NOISE_ROUGHSPIN),FALSE);
			}
		valid = TRUE;
		}
	}

void NoiseDlg::SetupUI(HWND hWnd)
	{
	this->hWnd = hWnd;

	for (int i=0; i<elems; i++) {
		iStrength[i] = GetISpinner(GetDlgItem(hWnd,strengthSpinID[i]));
		iStrength[i]->SetLimits(-9999999,9999999,FALSE);
		iStrength[i]->SetAutoScale();
		iStrength[i]->LinkToEdit(GetDlgItem(hWnd,strengthEditID[i]),EDITTYPE_FLOAT);		
		}
	for ( ; i< MAX_ELEMS; i++) {
		ShowWindow(GetDlgItem(hWnd,strengthSpinID[i]),SW_HIDE);
		ShowWindow(GetDlgItem(hWnd,strengthEditID[i]),SW_HIDE);
		ShowWindow(GetDlgItem(hWnd,strengthLabelID[i]),SW_HIDE);
		ShowWindow(GetDlgItem(hWnd,limID[i]),SW_HIDE);
		}

	iSeed = GetISpinner(GetDlgItem(hWnd,IDC_NOISE_SEEDSPIN));
	iSeed->SetLimits(0,9999999,FALSE);
	iSeed->SetScale(0.1f);
	iSeed->LinkToEdit(GetDlgItem(hWnd,IDC_NOISE_SEED),EDITTYPE_INT);	

	iFreq = GetISpinner(GetDlgItem(hWnd,IDC_NOISE_FREQSPIN));
	iFreq->SetLimits(0.001f,10.0f,FALSE);
	iFreq->SetScale(0.005f);
	iFreq->LinkToEdit(GetDlgItem(hWnd,IDC_NOISE_FREQ),EDITTYPE_FLOAT);	

	iRough = GetISpinner(GetDlgItem(hWnd,IDC_NOISE_ROUGHSPIN));
	iRough->SetLimits(0.0f,1.0f,FALSE);
	iRough->SetScale(0.005f);
	iRough->LinkToEdit(GetDlgItem(hWnd,IDC_NOISE_ROUGH),EDITTYPE_FLOAT);	
	
	iRampIn = GetISpinner(GetDlgItem(hWnd,IDC_NOISE_RAMPINSPIN));
	iRampIn->SetLimits(0,TIME_PosInfinity,FALSE);
	iRampIn->SetScale(10.0f);
	iRampIn->LinkToEdit(GetDlgItem(hWnd,IDC_NOISE_RAMPIN),EDITTYPE_TIME);	
	
	iRampOut = GetISpinner(GetDlgItem(hWnd,IDC_NOISE_RAMPOUTSPIN));
	iRampOut->SetLimits(0,TIME_PosInfinity,FALSE);
	iRampOut->SetScale(10.0f);
	iRampOut->LinkToEdit(GetDlgItem(hWnd,IDC_NOISE_RAMPOUT),EDITTYPE_TIME);	
	
	if (elems==1) {		
		SetWindowText(GetDlgItem(hWnd,IDC_NOISE_XSTRENGTHLABEL),
			GetString(IDS_RB_STRENGTH));
		}
	
	SetWindowLongPtr(GetDlgItem(hWnd,IDC_NOISE_GRAPH),GWLP_USERDATA,(LONG_PTR)cont);
	valid = FALSE;
	Update();
	}

void NoiseDlg::WMCommand(int id, int notify, HWND hCtrl)
	{
	switch (id) {
		case IDC_NOISE_FRACTAL:
			cont->fractal = IsDlgButtonChecked(hWnd,id);
			Change(TRUE);
			break;

		case IDC_NOISE_XLIM0:
			cont->lim[0] = IsDlgButtonChecked(hWnd,id);
			Change(TRUE);
			break;
		case IDC_NOISE_YLIM0:
			cont->lim[1] = IsDlgButtonChecked(hWnd,id);
			Change(TRUE);
			break;
		case IDC_NOISE_ZLIM0:
			cont->lim[2] = IsDlgButtonChecked(hWnd,id);
			Change(TRUE);
			break;
		}
	}

void NoiseDlg::SpinnerChange(int id,BOOL drag)
	{
	float strength[MAX_ELEMS];
	BOOL strengthChange = FALSE;
	
	// RB 2/5/99: Added test to see if theHold is already holding. A right click on the
	// spinner causes both a mouse down and up (but drag is FALSE) so there is no need to
	// turn the hold on again.
	if (!drag && !theHold.Holding()) {
		SpinnerStart(id);
		}

	if (id==IDC_NOISE_XSTRENGTHSPIN ||
		id==IDC_NOISE_YSTRENGTHSPIN ||
		id==IDC_NOISE_ZSTRENGTHSPIN) {
		cont->GetStrength(ip->GetTime(),strength);
		}

	switch (id) {
		case IDC_NOISE_XSTRENGTHSPIN:
			strength[0] = dim->UnConvert(iStrength[0]->GetFVal());
			strengthChange = TRUE;
			Change(1);
			break;

		case IDC_NOISE_YSTRENGTHSPIN:
			strength[1] = dim->UnConvert(iStrength[1]->GetFVal());
			strengthChange = TRUE;
			Change(1);
			break;

		case IDC_NOISE_ZSTRENGTHSPIN:
			strength[2] = dim->UnConvert(iStrength[2]->GetFVal());
			strengthChange = TRUE;
			Change(1);
			break;

		case IDC_NOISE_SEEDSPIN:
			cont->seed = iSeed->GetIVal();
			Change(1);
			break;

		case IDC_NOISE_FREQSPIN:
			cont->frequency = iFreq->GetFVal();
			Change(1);
			break;	
		
		case IDC_NOISE_ROUGHSPIN:
			cont->roughness = iRough->GetFVal();
			Change(1);
			break;			

		case IDC_NOISE_RAMPINSPIN:
			cont->rampin = iRampIn->GetIVal();
			Change(1);
			break;

		case IDC_NOISE_RAMPOUTSPIN:
			cont->rampout = iRampOut->GetIVal();
			Change(1);
			break;
		}
	
	if (strengthChange) {
		cont->cont->SetValue(ip->GetTime(),strength);
		}
	}

void NoiseDlg::SpinnerStart(int id)
	{
	switch (id) {
		case IDC_NOISE_XSTRENGTHSPIN:
		case IDC_NOISE_YSTRENGTHSPIN:
		case IDC_NOISE_ZSTRENGTHSPIN:
			theHold.Begin();
			break;
		}
	}

void NoiseDlg::SpinnerEnd(int id,BOOL cancel)
	{
	switch (id) {
		case IDC_NOISE_XSTRENGTH:
		case IDC_NOISE_YSTRENGTH:
		case IDC_NOISE_ZSTRENGTH:
		case IDC_NOISE_XSTRENGTHSPIN:
		case IDC_NOISE_YSTRENGTHSPIN:
		case IDC_NOISE_ZSTRENGTHSPIN:
			if (cancel) {
				theHold.Cancel();
			} else {
				theHold.Accept(GetString(IDS_RB_CHANGESTRENGTH));
				}
			break;
		}
	ip->RedrawViews(ip->GetTime());
	}

void NoiseDlg::Change(BOOL redraw)
	{
	InvalidateRect(GetDlgItem(hWnd,IDC_NOISE_GRAPH),NULL,TRUE);
	cont->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	UpdateWindow(GetDlgItem(hWnd,IDC_NOISE_GRAPH));
	UpdateWindow(GetParent(hWnd));	
	if (redraw) ip->RedrawViews(ip->GetTime());
	}


class CheckForNonNoiseDlg : public DependentEnumProc {
	public:		
		BOOL non;
		ReferenceMaker *me;
		CheckForNonNoiseDlg(ReferenceMaker *m) {non = FALSE;me = m;}
		int proc(ReferenceMaker *rmaker) {
			if (rmaker==me) return 0;
			if (rmaker->SuperClassID()!=REF_MAKER_CLASS_ID &&
				rmaker->ClassID()!=Class_ID(NOISEDLG_CLASS_ID,0)) {
				non = TRUE;
				return 1;
				}
			return 0;
			}
	};
void NoiseDlg::MaybeCloseWindow()
	{
	CheckForNonNoiseDlg check(cont);
	cont->EnumDependents(&check);
	if (!check.non) {
		PostMessage(hWnd,WM_CLOSE,0,0);
		}
	}



RefResult NoiseDlg::NotifyRefChanged(
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


static INT_PTR CALLBACK NoiseDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	NoiseDlg *dlg = (NoiseDlg*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch (msg) {
		case WM_INITDIALOG:
			dlg = (NoiseDlg*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			dlg->SetupUI(hWnd);
			break;

		case CC_SPINNER_BUTTONDOWN:
			dlg->SpinnerStart(LOWORD(wParam));
			break;

		case CC_SPINNER_CHANGE:
			dlg->SpinnerChange(LOWORD(wParam),HIWORD(wParam));
			break;

		case WM_CUSTEDIT_ENTER:
			dlg->SpinnerEnd(LOWORD(wParam),FALSE);
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

void BaseNoiseControl::EditTrackParams(
		TimeValue t,
		ParamDimensionBase *dim,
		TCHAR *pname,
		HWND hParent,
		IObjParam *ip,
		DWORD flags)
	{
	HWND hCur = FindOpenNoiseCtrlWindow(hParent,this);
	if (hCur) {
		SetForegroundWindow(hCur);
		return;
		}

	NoiseDlg *dlg = new NoiseDlg(this,dim,pname,ip,hParent);
	RegisterNoiseCtrlWindow(dlg->hWnd,hParent,this);
	}

//------------------------------------------------------------

BaseNoiseControl::BaseNoiseControl() 
	{	
	for (int i=0; i<MAX_ELEMS; i++) {
		//strength[i] = 50.0f;
		lim[i] = FALSE;
		}
	fractal   = TRUE;
	roughness = 0.0f;
	frequency = 0.5f;	
	seed      = 0;	
	rampin = rampout = 0;
	range     = Interval(GetAnimStart(),GetAnimEnd());
	} 

BaseNoiseControl& BaseNoiseControl::operator=(const BaseNoiseControl& from)
	{
	ReplaceReference(Control::NumRefs(),from.cont);
	for (int i=0; i<MAX_ELEMS; i++) {
		//strength[i]  = from.strength[i];
		lim[i] = from.lim[i];
		}
	fractal   = from.fractal;
	roughness = from.roughness;
	frequency = from.frequency;
	seed      = from.seed;
	range     = from.range;
	rampin    = from.rampin;
	rampout   = from.rampout;
	return *this;
	}

int BaseNoiseControl::NumSubs() 
	{
	return Control::NumSubs()+1;
	}

Animatable* BaseNoiseControl::SubAnim(int i) 
	{
	if (i<Control::NumSubs()) return Control::SubAnim(i);
	return cont;
	}

TSTR BaseNoiseControl::SubAnimName(int i) 
	{
	if (i<Control::NumSubs()) 
		return Control::SubAnimName(i);
	return GetString(IDS_RB_NOISESTRENGTH);
	}

BOOL BaseNoiseControl::AssignController(Animatable *control,int subAnim) 
	{
	ReplaceReference(Control::NumRefs(),(Control*)control);
	return TRUE;
	}

int BaseNoiseControl::SubNumToRefNum(int subNum) 
	{	
	return Control::NumRefs();
	}

int BaseNoiseControl::NumRefs() 
	{
	return Control::NumRefs()+1;
	}

RefTargetHandle BaseNoiseControl::GetReference(int i) 
	{
	if (i<Control::NumRefs()) return Control::GetReference(i);
	return cont;
	}

void BaseNoiseControl::SetReference(int i, RefTargetHandle rtarg) 
	{
	if (i<Control::NumRefs()) Control::SetReference(i,rtarg);
	else cont = (Control*)rtarg;
	}

extern float noise1(float arg);

float BaseNoiseControl::NoiseAtTime(TimeValue t,int s,int index)
	{
	float ramp = 1.0f, res;
	
	// Limit to in range
	if (t<range.Start()) t = range.Start();
	if (t>range.End()) t = range.End();	
	
	// Compute ramping
	if (t<range.Start()+rampin) {
		float u = float(t-range.Start())/float(rampin);
		ramp *= u*u * (3.0f-2.0f*u);
		}
	if (t>range.End()-rampout) {
		float u = float(range.End()-t)/float(rampout);
		ramp *= u*u * (3.0f-2.0f*u);
		}

	// Compute noise
	float v = float(t) * float(0.005) * frequency + Perm(s);	
	if (fractal) {
		res = (float)fBm1(v, 1.0f-roughness, 2.0f, 6 /*octaves*/);
	} else {
		res = noise1(v);
		}
	if (lim[index]) {
		res = res+0.5f;
		}
	return res * ramp;
	}

void BaseNoiseControl::HoldRange()
	{
	if (theHold.Holding() && !TestAFlag(A_HELD)) {
		SetAFlag(A_HELD);
		theHold.Put(new RangeRestore(this));
		}
	}

void BaseNoiseControl::EditTimeRange(Interval range,DWORD flags)
	{
	if (!(flags&EDITRANGE_LINKTOKEYS)) {
		HoldRange();
		this->range = range;
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
	}

void BaseNoiseControl::MapKeys(TimeMap *map,DWORD flags)
	{
	// CAL-08/27/03: Need to call the default function Animatable::MapKeys() when overriding it. (Defect #468018)
	Animatable::MapKeys(map,flags);
	if (flags&TRACK_MAPRANGE) {
		HoldRange();
		TimeValue t0 = map->map(range.Start());
		TimeValue t1 = map->map(range.End());
		range.Set(t0,t1);
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
	}


#define XSTRENGTH_CHUNK		0x0100
#define YSTRENGTH_CHUNK		0x0101
#define ZSTRENGTH_CHUNK		0x0102
#define FREQUENCY_CHUNK		0x0103
#define ROUGHNESS_CHUNK		0x0104
#define SEED_CHUNK			0x0105
#define FRACTAL_CHUNK		0x0106
#define RANGE_CHUNK			0x0107
#define RAMPIN_CHUNK		0x0108
#define RAMPOUT_CHUNK		0x0109
#define XLIM_CHUNK			0x0110
#define YLIM_CHUNK			0x0111
#define ZLIM_CHUNK			0x0112

IOResult BaseNoiseControl::Save(ISave *isave)
	{		
	ULONG nb;	

	for (int i=0; i<MAX_ELEMS; i++) {
		//isave->BeginChunk(XSTRENGTH_CHUNK+i);
		//isave->Write(&strength[i],sizeof(strength[i]),&nb);
		//isave->EndChunk();

		isave->BeginChunk(XLIM_CHUNK+i);
		isave->Write(&lim[i],sizeof(lim[i]),&nb);
		isave->EndChunk();
		}

	isave->BeginChunk(FREQUENCY_CHUNK);
	isave->Write(&frequency,sizeof(frequency),&nb);
	isave->EndChunk();
	
	isave->BeginChunk(ROUGHNESS_CHUNK);
	isave->Write(&roughness,sizeof(roughness),&nb);
	isave->EndChunk();
	
	isave->BeginChunk(SEED_CHUNK);
	isave->Write(&seed,sizeof(seed),&nb);
	isave->EndChunk();
	
	isave->BeginChunk(FRACTAL_CHUNK);
	isave->Write(&fractal,sizeof(fractal),&nb);
	isave->EndChunk();
	
	isave->BeginChunk(RANGE_CHUNK);
	isave->Write(&range,sizeof(range),&nb);
	isave->EndChunk();
	
	isave->BeginChunk(RAMPIN_CHUNK);
	isave->Write(&rampin,sizeof(rampin),&nb);
	isave->EndChunk();
	
	isave->BeginChunk(RAMPOUT_CHUNK);
	isave->Write(&rampout,sizeof(rampout),&nb);
	isave->EndChunk();
	
	return IO_OK;
	}

IOResult BaseNoiseControl::Load(ILoad *iload)
	{
	float strength[MAX_ELEMS];
	BOOL strengthRead=FALSE;
	ULONG nb;
	IOResult res = IO_OK;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case XSTRENGTH_CHUNK:
				res=iload->Read(&strength[0],sizeof(strength[0]),&nb);
				strengthRead = TRUE;
				break;

			case YSTRENGTH_CHUNK:
				res=iload->Read(&strength[1],sizeof(strength[1]),&nb);
				strengthRead = TRUE;
				break;

			case ZSTRENGTH_CHUNK:
				res=iload->Read(&strength[2],sizeof(strength[2]),&nb);
				strengthRead = TRUE;
				break;

			case XLIM_CHUNK:
				res=iload->Read(&lim[0],sizeof(lim[0]),&nb);
				break;
			case YLIM_CHUNK:
				res=iload->Read(&lim[1],sizeof(lim[1]),&nb);
				break;
			case ZLIM_CHUNK:
				res=iload->Read(&lim[2],sizeof(lim[2]),&nb);
				break;

			case FREQUENCY_CHUNK:
				res=iload->Read(&frequency,sizeof(frequency),&nb);
				break;

			case ROUGHNESS_CHUNK:
				res=iload->Read(&roughness,sizeof(roughness),&nb);
				break;

			case SEED_CHUNK:
				res=iload->Read(&seed,sizeof(seed),&nb);
				break;

			case FRACTAL_CHUNK:
				res=iload->Read(&fractal,sizeof(fractal),&nb);
				break;

			case RANGE_CHUNK:
				res=iload->Read(&range,sizeof(range),&nb);
				break;

			case RAMPIN_CHUNK:
				res=iload->Read(&rampin,sizeof(rampin),&nb);
				break;

			case RAMPOUT_CHUNK:
				res=iload->Read(&rampout,sizeof(rampout),&nb);
				break;
			}

		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}
	
	if (strengthRead) {
		cont->SetValue(0,strength);
		}
	return IO_OK;
	}


void BaseNoiseControl::SetSeed(int seed)
	{
	this->seed = seed;
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void BaseNoiseControl::SetFrequency(float f)
	{
	frequency = f;
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void BaseNoiseControl::SetFractal(BOOL f)
	{
	fractal = f;
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void BaseNoiseControl::SetRoughness(float f)
	{
	roughness = f;
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void BaseNoiseControl::SetRampIn(TimeValue in)
	{
	rampin = in;
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void BaseNoiseControl::SetRampOut(TimeValue out)
	{
	rampout = out;
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void BaseNoiseControl::SetPositiveOnly(int which,BOOL onOff)
	{
	lim[which] = onOff;
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void BaseNoiseControl::SetStrengthController(Control *c)
	{
	ReplaceReference(Control::NumRefs(),c);
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}



//--------------------------------------------------------------------

RefTargetHandle FloatNoiseControl::Clone(RemapDir& remap)
	{
	// make a new noise controller and give it our param values.
	FloatNoiseControl *cont = new FloatNoiseControl;
	*cont = *this;
	// Clone the strength controller
	cont->ReplaceReference(Control::NumRefs(),remap.CloneRef(cont->cont));
	CloneControl(cont,remap);
	BaseClone(this, cont, remap);
	return cont;
	}

void FloatNoiseControl::GetValueLocalTime(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
	float strength;
	GetStrength(t,&strength);

	// This controller is always changing.
	valid.SetInstant(t);
	
	*((float*)val) = NoiseAtTime(t,seed,0) * strength;
	}

void FloatNoiseControl::Extrapolate(
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

//--------------------------------------------------------------------------

RefTargetHandle PositionNoiseControl::Clone(RemapDir& remap)
	{
	// make a new noise controller and give it our param values.
	PositionNoiseControl *cont = new PositionNoiseControl;
	*cont = *this;
	// Clone the strength controller
	cont->ReplaceReference(Control::NumRefs(),remap.CloneRef(cont->cont));
	CloneControl(cont,remap);
	BaseClone(this, cont, remap);
	return cont;
	}

void PositionNoiseControl::GetValueLocalTime(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
	float strength[3];
	GetStrength(t,strength);

	// This controller is always changing.
	valid.SetInstant(t);
	
	*((Point3*)val) = Point3(
		NoiseAtTime(t,seed,0) * strength[0],
		NoiseAtTime(t,seed+1,1) * strength[1],
		NoiseAtTime(t,seed+2,2) * strength[2] );
	}

void PositionNoiseControl::Extrapolate(
		Interval range,TimeValue t,void *val,Interval &valid,int type)
	{
	Point3 val0, val1, val2, res;
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
	*((Point3*)val) = res;
	}

//--------------------------------------------------------------------------


RefTargetHandle Point3NoiseControl::Clone(RemapDir& remap)
	{
	// make a new noise controller and give it our param values.
	Point3NoiseControl *cont = new Point3NoiseControl;
	*cont = *this;	
	// Clone the strength controller
	cont->ReplaceReference(Control::NumRefs(),remap.CloneRef(cont->cont));
	CloneControl(cont,remap);
	BaseClone(this, cont, remap);
	return cont;
	}

void Point3NoiseControl::GetValueLocalTime(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
	float strength[3];
	GetStrength(t,strength);

	// This controller is always changing.
	valid.SetInstant(t);
	
	*((Point3*)val) = Point3(
		NoiseAtTime(t,seed,0) * strength[0],
		NoiseAtTime(t,seed+1,1) * strength[1],
		NoiseAtTime(t,seed+2,2) * strength[2] );
	}

void Point3NoiseControl::Extrapolate(
		Interval range,TimeValue t,void *val,Interval &valid,int type)
	{
	Point3 val0, val1, val2, res;
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
	*((Point3*)val) = res;
	}

//--------------------------------------------------------------------------



RefTargetHandle RotationNoiseControl::Clone(RemapDir& remap)
	{
	// make a new noise controller and give it our param values.
	RotationNoiseControl *cont = new RotationNoiseControl;
	*cont = *this;	
	// Clone the strength controller
	cont->ReplaceReference(Control::NumRefs(),remap.CloneRef(cont->cont));
	CloneControl(cont,remap);
	BaseClone(this, cont, remap);
	return cont;
	}

void RotationNoiseControl::GetValueLocalTime(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
	float strength[3];
	GetStrength(t,strength);

	// This controller is always changing.
	valid.SetInstant(t);
	float ang[3];

	ang[0] = NoiseAtTime(t,seed,0) * strength[0];
	ang[1] = NoiseAtTime(t,seed+1,1) * strength[1];
	ang[2] = NoiseAtTime(t,seed+2,2) * strength[2];
	
	EulerToQuat(ang,*((Quat*)val));		
	}

void RotationNoiseControl::Extrapolate(
		Interval range,TimeValue t,void *val,Interval &valid,int type)
	{
	Quat val0, val1, val2, res;
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
	*((Quat*)val) = res;
	}

//--------------------------------------------------------------------------



RefTargetHandle ScaleNoiseControl::Clone(RemapDir& remap)
	{
	// make a new noise controller and give it our param values.
	ScaleNoiseControl *cont = new ScaleNoiseControl;
	*cont = *this;
	// Clone the strength controller
	cont->ReplaceReference(Control::NumRefs(),remap.CloneRef(cont->cont));
	CloneControl(cont,remap);
	BaseClone(this, cont, remap);
	return cont;
	}

void ScaleNoiseControl::GetValueLocalTime(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
	float strength[3];
	GetStrength(t,strength);

	// This controller is always changing.
	valid.SetInstant(t);
	
	*((ScaleValue*)val) = ScaleValue(
		Point3(
			1.0f + NoiseAtTime(t,seed,0) * strength[0],
			1.0f + NoiseAtTime(t,seed+1,1) * strength[1],
			1.0f + NoiseAtTime(t,seed+2,2) * strength[2] ),
		Quat(0.0,0.0,0.0,1.0));
	}

void ScaleNoiseControl::Extrapolate(
		Interval range,TimeValue t,void *val,Interval &valid,int type)
	{
	ScaleValue val0, val1, val2, res;
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
	*((ScaleValue*)val) = res;
	}





