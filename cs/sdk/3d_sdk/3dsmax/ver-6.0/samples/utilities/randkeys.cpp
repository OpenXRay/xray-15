/**********************************************************************
 *<
	FILE: randkeys.cpp

	DESCRIPTION: A Track View Utility that randomizes keys

	CREATED BY: Rolf Berteig

	HISTORY: created 12/18/96

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/
#include "util.h"
#include "tvutil.h"
#include "istdplug.h"

#define RAND_KEYS_NAME		GetString(IDS_RB_RANDKEYS)
#define RAND_KEYS_CLASS_ID	Class_ID(0x811ff4ba,0x20dd06ac)

class RandKeysUtil : public TrackViewUtility {
	public:
		Interface *ip;
		ITVUtility *iu;
		HWND hWnd;
		ISpinnerControl *iPosTime, *iNegTime, *iPosVal, *iNegVal;
		TimeValue posTime, negTime;
		float posVal, negVal;
		BOOL doTime, doVal;

		RandKeysUtil();
		void DeleteThis() {if (hWnd) DestroyWindow(hWnd); } // this is deleted when the dialog closes
		void BeginEditParams(Interface *ip,ITVUtility *iu);
		void EndEditParams(Interface *ip,ITVUtility *iu);
		void MajorModeChanged() {SetStates();}
		void TimeSelectionChanged() {SetStates();}
		void		GetClassName(TSTR& s)	{ s = RAND_KEYS_NAME; }  
		Class_ID	ClassID()				{ return RAND_KEYS_CLASS_ID; }

		void SetupWindow(HWND hWnd);
		void Destroy();
		void SpinnerChange(int id);
		void SetStates();
		void Apply();
	};

class RandKeysClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new RandKeysUtil;}
	const TCHAR *	ClassName() {return RAND_KEYS_NAME;}
	SClass_ID		SuperClassID() {return TRACKVIEW_UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return RAND_KEYS_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
	};

static RandKeysClassDesc randKeysDesc;
ClassDesc* GetRandKeysDesc() {return &randKeysDesc;}

RandKeysUtil::RandKeysUtil()
	{
	ip = NULL;
	iu = NULL;
	hWnd = NULL;
	posTime = negTime = 160;
	posVal = negVal = 10.0f;
	doTime = TRUE;
	doVal = TRUE;
	}

static INT_PTR CALLBACK RandKeysDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	RandKeysUtil *util = (RandKeysUtil*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch (msg) {
		case WM_INITDIALOG:
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			util = (RandKeysUtil*)lParam;
			util->SetupWindow(hWnd);
			CenterWindow(hWnd,GetParent(hWnd));
			break;

		case CC_SPINNER_CHANGE:
			util->SpinnerChange(LOWORD(wParam));			
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {				
				case IDC_RANDKEYS_TIME:
					util->doTime = IsDlgButtonChecked(hWnd,LOWORD(wParam));
					util->SetStates();
					break;

				case IDC_RANDKEYS_VAL:
					util->doVal = IsDlgButtonChecked(hWnd,LOWORD(wParam));
					util->SetStates();
					break;

				case IDC_RANDKEYS_APPLY:
					util->Apply();
					break;
				}
			break;

		case WM_CLOSE:
			DestroyWindow(hWnd);
			break;

		case WM_DESTROY:
			util->Destroy();			
			delete util;
			break;
		
		default:
			return FALSE;
		}
	return TRUE;
	}

void RandKeysUtil::SpinnerChange(int id) 
	{
	switch (id) {
		case IDC_RANDKEYS_POSTIMESPIN:
			posTime = iPosTime->GetIVal(); break;
		case IDC_RANDKEYS_NEGTIMESPIN:
			negTime = iNegTime->GetIVal(); break;
		case IDC_RANDKEYS_POSVALSPIN:
			posVal = iPosVal->GetFVal(); break;
		case IDC_RANDKEYS_NEGVALSPIN:
			negVal = iNegVal->GetFVal(); break;
		}
	}

void RandKeysUtil::SetStates()
	{
	if (doTime) {
		iPosTime->Enable();
		EnableWindow(GetDlgItem(hWnd,IDC_RANDKEYS_POSTIMELABEL),TRUE);
		iNegTime->Enable();
		EnableWindow(GetDlgItem(hWnd,IDC_RANDKEYS_NEGTIMELABEL),TRUE);
	} else {
		iPosTime->Disable();
		EnableWindow(GetDlgItem(hWnd,IDC_RANDKEYS_POSTIMELABEL),FALSE);
		iNegTime->Disable();
		EnableWindow(GetDlgItem(hWnd,IDC_RANDKEYS_NEGTIMELABEL),FALSE);
		}

	if (doVal) {
		iPosVal->Enable();
		EnableWindow(GetDlgItem(hWnd,IDC_RANDKEYS_POSVALLABEL),TRUE);
		iNegVal->Enable();
		EnableWindow(GetDlgItem(hWnd,IDC_RANDKEYS_NEGVALLABEL),TRUE);
	} else {
		iPosVal->Disable();
		EnableWindow(GetDlgItem(hWnd,IDC_RANDKEYS_POSVALLABEL),FALSE);
		iNegVal->Disable();
		EnableWindow(GetDlgItem(hWnd,IDC_RANDKEYS_NEGVALLABEL),FALSE);
		}

	switch (iu->GetMajorMode()) {
		case TVMODE_EDITKEYS:
		case TVMODE_EDITFCURVE:
			SetDlgItemText(hWnd,IDC_RANDKEYS_TEXT,
				GetString(IDS_RANDKEYS_KEYTEXT));
			EnableWindow(GetDlgItem(hWnd,IDC_RANDKEYS_APPLY),TRUE);
			break;

		case TVMODE_EDITTIME: {
			Interval iv = iu->GetTimeSelection();
			TSTR buf, start, end;
			TimeToString(iv.Start(),start);
			TimeToString(iv.End(),end);
			buf.printf(GetString(IDS_RANDKEYS_TIMETEXT),start,end);
			SetDlgItemText(hWnd,IDC_RANDKEYS_TEXT,buf);
			EnableWindow(GetDlgItem(hWnd,IDC_RANDKEYS_APPLY),TRUE);
			break;
			}

		case TVMODE_EDITRANGES:
		case TVMODE_POSRANGES:
			SetDlgItemText(hWnd,IDC_RANDKEYS_TEXT,_T(""));
			EnableWindow(GetDlgItem(hWnd,IDC_RANDKEYS_APPLY),FALSE);
			break;
		
		}
	}

void RandKeysUtil::SetupWindow(HWND hWnd)
	{	
	this->hWnd = hWnd;

	CheckDlgButton(hWnd,IDC_RANDKEYS_TIME,doTime);
	CheckDlgButton(hWnd,IDC_RANDKEYS_VAL,doVal);

	iPosTime = GetISpinner(GetDlgItem(hWnd,IDC_RANDKEYS_POSTIMESPIN));
	iPosTime->SetLimits(0, TIME_PosInfinity, FALSE);
	iPosTime->SetScale(10.0f);
	iPosTime->LinkToEdit(GetDlgItem(hWnd,IDC_RANDKEYS_POSTIME), EDITTYPE_TIME);
	iPosTime->SetValue(posTime,FALSE);

	iNegTime = GetISpinner(GetDlgItem(hWnd,IDC_RANDKEYS_NEGTIMESPIN));
	iNegTime->SetLimits(0, TIME_PosInfinity, FALSE);
	iNegTime->SetScale(10.0f);
	iNegTime->LinkToEdit(GetDlgItem(hWnd,IDC_RANDKEYS_NEGTIME), EDITTYPE_TIME);
	iNegTime->SetValue(negTime,FALSE);

	iPosVal = GetISpinner(GetDlgItem(hWnd,IDC_RANDKEYS_POSVALSPIN));
	iPosVal->SetLimits(0.0f, 999999999.9f, FALSE);
	iPosVal->SetScale(0.1f);
	iPosVal->LinkToEdit(GetDlgItem(hWnd,IDC_RANDKEYS_POSVAL), EDITTYPE_FLOAT);
	iPosVal->SetValue(posVal,FALSE);

	iNegVal = GetISpinner(GetDlgItem(hWnd,IDC_RANDKEYS_NEGVALSPIN));
	iNegVal->SetLimits(0.0f, 999999999.0f, FALSE);
	iNegVal->SetScale(0.1f);
	iNegVal->LinkToEdit(GetDlgItem(hWnd,IDC_RANDKEYS_NEGVAL), EDITTYPE_FLOAT);
	iNegVal->SetValue(negVal,FALSE);

	SetStates();
	}

void RandKeysUtil::Destroy()
	{
	ReleaseISpinner(iPosTime);
	ReleaseISpinner(iNegTime);
	ReleaseISpinner(iPosVal);
	ReleaseISpinner(iNegVal);
	if (iu) iu->TVUtilClosing(this);
	}

void RandKeysUtil::BeginEditParams(Interface *ip,ITVUtility *iu)
	{
	this->ip = ip;
	this->iu = iu;	
	hWnd = CreateDialogParam(
		hInstance,
		MAKEINTRESOURCE(IDD_RAND_KEYS),
		iu->GetTrackViewHWnd(),
		RandKeysDlgProc,
		(LONG_PTR)this);
	}

void RandKeysUtil::EndEditParams(Interface *ip,ITVUtility *iu)
	{
	}

static IKey *GetKeyPointer(SClass_ID sid, Class_ID cid)
	{
	static ITCBFloatKey  tcbfkey;
	static ITCBPoint3Key tcbpt3key;
	static ITCBPoint4Key tcbpt4key;
	static ITCBRotKey    tcbrkey;
	static ITCBScaleKey  tcbskey;
	static IBezFloatKey  bezfkey;
	static IBezPoint3Key bezpt3key;
	static IBezPoint4Key bezpt4key;
	static IBezQuatKey   bezrkey;
	static IBezScaleKey  bezskey;
	static ILinFloatKey  linfkey;
	static ILinPoint3Key linpt3key;
	static ILinRotKey    linrkey;
	static ILinScaleKey  linskey;
	static IBoolFloatKey boolfkey;


	switch (sid) {
		case CTRL_FLOAT_CLASS_ID:
			if (cid==Class_ID(HYBRIDINTERP_FLOAT_CLASS_ID,0)) return &bezfkey;
			if (cid==Class_ID(LININTERP_FLOAT_CLASS_ID,0)) return &linfkey;
			if (cid==Class_ID(TCBINTERP_FLOAT_CLASS_ID,0)) return &tcbfkey;
			if (cid==Class_ID(BOOLCNTRL_CLASS_ID)) return &boolfkey;

			break;

		case CTRL_POINT3_CLASS_ID:
			if (cid==Class_ID(HYBRIDINTERP_POINT3_CLASS_ID,0)) return &bezpt3key;
			if (cid==Class_ID(HYBRIDINTERP_COLOR_CLASS_ID,0)) return &bezpt3key;
			if (cid==Class_ID(TCBINTERP_POINT3_CLASS_ID,0)) return &tcbpt3key;
			break;

		case CTRL_POINT4_CLASS_ID:
			if (cid==Class_ID(HYBRIDINTERP_POINT4_CLASS_ID,0)) return &bezpt4key;
			if (cid==Class_ID(HYBRIDINTERP_FRGBA_CLASS_ID,0)) return &bezpt4key;
			if (cid==Class_ID(TCBINTERP_POINT4_CLASS_ID,0)) return &tcbpt4key;
			break;

		case CTRL_POSITION_CLASS_ID:
			if (cid==Class_ID(LININTERP_POSITION_CLASS_ID,0)) return &linpt3key;
			if (cid==Class_ID(HYBRIDINTERP_POSITION_CLASS_ID,0)) return &bezpt3key;
			if (cid==Class_ID(TCBINTERP_POSITION_CLASS_ID,0)) return &tcbpt3key;
			break;

		case CTRL_ROTATION_CLASS_ID:
			if (cid==Class_ID(HYBRIDINTERP_ROTATION_CLASS_ID,0)) return &bezrkey;
			if (cid==Class_ID(LININTERP_ROTATION_CLASS_ID,0)) return &linrkey;
			if (cid==Class_ID(TCBINTERP_ROTATION_CLASS_ID,0)) return &tcbrkey;
			break;

		case CTRL_SCALE_CLASS_ID:
			if (cid==Class_ID(LININTERP_SCALE_CLASS_ID,0)) return &linskey;
			if (cid==Class_ID(HYBRIDINTERP_SCALE_CLASS_ID,0)) return &bezskey;
			if (cid==Class_ID(TCBINTERP_SCALE_CLASS_ID,0)) return &tcbskey;
			break;
		}
	return NULL;
	}

static float CompRand(float min, float max)
	{
	float u = float(rand())/float(RAND_MAX);
	return (1.0f-u)*min + u*max;
	}


void RandKeysUtil::Apply()
	{
	BOOL timeMode   = iu->GetMajorMode()==TVMODE_EDITTIME;
	BOOL fcurveMode = iu->GetMajorMode()==TVMODE_EDITFCURVE;
	Interval iv = iu->GetTimeSelection();
	if (!doTime && !doVal) return;

	theHold.Begin();

	// Turn animation on
	SuspendAnimate();
	AnimateOn();

	for (int i=0; i<iu->GetNumTracks(); i++) {
		if ((timeMode||fcurveMode) && !iu->IsSelected(i)) continue;
		
		// Get Interfaces
		Animatable *anim   = iu->GetAnim(i);
		Animatable *client = iu->GetClient(i);
		int subNum         = iu->GetSubNum(i);
		Control *cont      = GetControlInterface(anim);
		IKeyControl *ikc   = GetKeyControlInterface(anim);
		IKey *key          = GetKeyPointer(anim->SuperClassID(),anim->ClassID());				
		if (!ikc || !cont || !key) continue;						
		if (fcurveMode && !anim->IsCurveSelected()) continue;

		// Get the param dim
		float min = negVal, max = posVal;
		ParamDimension *dim = client->GetParamDimension(subNum);
		if (dim) {
			min = dim->UnConvert(min);
			max = dim->UnConvert(max);
			}

		for (int j=0; j<ikc->GetNumKeys(); j++) {
			// Get the key data
			ikc->GetKey(j,key);
			
			// Check if it's selected
			if (timeMode && !iv.InInterval(key->time)) continue;
			if (!timeMode && !(key->flags&IKEY_SELECTED)) continue;			

			// Randomize time
			if (doTime) {
				key->time = (int)CompRand(
					float(key->time-negTime),
					float(key->time+posTime));
				ikc->SetKey(j,key);
				}
			}

		if (doTime) ikc->SortKeys();

		for (j=0; j<ikc->GetNumKeys(); j++) {
			// Get the key data
			ikc->GetKey(j,key);
			
			// Check if it's selected
			if (timeMode && !iv.InInterval(key->time)) continue;
			if (!timeMode && !(key->flags&IKEY_SELECTED)) continue;			

			// Randomize value
			if (doVal) {
				Point3 pt, ang;
				Point4 p4;
				float f;
				Quat q;
				ScaleValue s;				
				BOOL doX, doY, doZ, doW;
				doX = doY = doZ = doW = TRUE;
				if (!fcurveMode) {
					if (!(key->flags&IKEY_XSEL)) doX = FALSE;
					if (!(key->flags&IKEY_YSEL)) doY = FALSE;
					if (!(key->flags&IKEY_ZSEL)) doZ = FALSE;
					if (!(key->flags&IKEY_WSEL)) doW = FALSE;
					}

				switch (anim->SuperClassID()) {
					case CTRL_FLOAT_CLASS_ID:			
						cont->GetValue(key->time,&f,FOREVER);
						f = CompRand(f-min,f+max);
						cont->SetValue(key->time,&f);
						break;

					case CTRL_POSITION_CLASS_ID:
					case CTRL_POINT3_CLASS_ID:
						cont->GetValue(key->time,&pt,FOREVER);
						if (doX) pt.x = CompRand(pt.x-min,pt.x+max);
						if (doY) pt.y = CompRand(pt.y-min,pt.y+max);
						if (doZ) pt.z = CompRand(pt.z-min,pt.z+max);
						cont->SetValue(key->time,&pt);
						break;
					
					case CTRL_POINT4_CLASS_ID:
						cont->GetValue(key->time,&p4,FOREVER);
						if (doX) p4.x = CompRand(p4.x-min,p4.x+max);
						if (doY) p4.y = CompRand(p4.y-min,p4.y+max);
						if (doZ) p4.z = CompRand(p4.z-min,p4.z+max);
						if (doW) p4.w = CompRand(p4.w-min,p4.w+max);
						cont->SetValue(key->time,&p4);
						break;

					case CTRL_ROTATION_CLASS_ID:
						cont->GetValue(key->time,&q,FOREVER);
						QuatToEuler(q, ang);
						ang.x = CompRand(ang.x-min,ang.x+max);
						ang.y = CompRand(ang.y-min,ang.y+max);
						ang.z = CompRand(ang.z-min,ang.z+max);
						EulerToQuat(ang,q);
						cont->SetValue(key->time,&q);
						break;

					case CTRL_SCALE_CLASS_ID:
						cont->GetValue(key->time,&s,FOREVER);
						if (doX) s.s.x = CompRand(s.s.x-min,s.s.x+max);
						if (doY) s.s.y = CompRand(s.s.y-min,s.s.y+max);
						if (doZ) s.s.z = CompRand(s.s.z-min,s.s.z+max);
						cont->SetValue(key->time,&s);
						break;
					}
				}
			}

		
		}

	ResumeAnimate();

	theHold.Accept(GetString(IDS_RB_RANDOMIZEKEYS));
	ip->RedrawViews(ip->GetTime());
	}
