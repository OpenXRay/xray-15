/**********************************************************************
 *<
	FILE: ortkeys.cpp

	DESCRIPTION: A Track View Utility that creates key for out of range animation

	CREATED BY: Rolf Berteig

	HISTORY: created 12/18/96

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/
#include "util.h"
#include "tvutil.h"
#include "istdplug.h"

#define ORT_KEYS_NAME		GetString(IDS_RB_ORTKEYS)
#define ORT_KEYS_CLASS_ID	Class_ID(0x811ff4ba,0x20dd06ab)

class ORTKeysUtil : public TrackViewUtility {
	public:
		Interface *ip;
		ITVUtility *iu;
		HWND hWnd;
		ISpinnerControl *iBefore, *iAfter, *iSamp;
		TimeValue before, after;				
		int samples;		

		ORTKeysUtil();
		void DeleteThis() {if (hWnd) DestroyWindow(hWnd); } // this is deleted when the dialog closes
		void BeginEditParams(Interface *ip,ITVUtility *iu);
		void EndEditParams(Interface *ip,ITVUtility *iu);				
		void		GetClassName(TSTR& s)	{ s = ORT_KEYS_NAME; }  
		Class_ID	ClassID()				{ return ORT_KEYS_CLASS_ID; }

		void SetupWindow(HWND hWnd);
		void Destroy();
		void SpinnerChange(int id);		
		void Apply();
		void SampleORT(void *tab,Control *cont, TimeValue start, TimeValue end);
		void MakeORTKeys(void *tab,Control *cont, TimeValue start, TimeValue end);
	};

class ORTKeysClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new ORTKeysUtil;}
	const TCHAR *	ClassName() {return ORT_KEYS_NAME;}
	SClass_ID		SuperClassID() {return TRACKVIEW_UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return ORT_KEYS_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
	};

static ORTKeysClassDesc ortKeysDesc;
ClassDesc* GetORTKeysDesc() {return &ortKeysDesc;}

ORTKeysUtil::ORTKeysUtil()
	{
	ip = NULL;
	iu = NULL;
	hWnd = NULL;
	before = after = 1600;	
	samples = 10;
	}

static INT_PTR CALLBACK ORTKeysDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	ORTKeysUtil *util = (ORTKeysUtil*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch (msg) {
		case WM_INITDIALOG:
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			util = (ORTKeysUtil*)lParam;
			util->SetupWindow(hWnd);
			CenterWindow(hWnd,GetParent(hWnd));
			break;

		case CC_SPINNER_CHANGE:
			util->SpinnerChange(LOWORD(wParam));			
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {				
				case IDC_ORTKEYS_APPLY:
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

void ORTKeysUtil::SpinnerChange(int id) 
	{
	switch (id) {
		case IDC_ORTKEYS_BEFORESPIN:
			before = iBefore->GetIVal(); break;
		case IDC_ORTKEYS_AFTERSPIN:
			after = iAfter->GetIVal(); break;		
		case IDC_ORTKEYS_SAMPLESSPIN:
			samples = iSamp->GetIVal(); break;
		}
	}

void ORTKeysUtil::SetupWindow(HWND hWnd)
	{	
	this->hWnd = hWnd;
	
	iBefore = GetISpinner(GetDlgItem(hWnd,IDC_ORTKEYS_BEFORESPIN));
	iBefore->SetLimits(0, TIME_PosInfinity, FALSE);
	iBefore->SetScale(10.0f);
	iBefore->LinkToEdit(GetDlgItem(hWnd,IDC_ORTKEYS_BEFORE), EDITTYPE_TIME);
	iBefore->SetValue(before,FALSE);

	iAfter = GetISpinner(GetDlgItem(hWnd,IDC_ORTKEYS_AFTERSPIN));
	iAfter->SetLimits(0, TIME_PosInfinity, FALSE);
	iAfter->SetScale(10.0f);
	iAfter->LinkToEdit(GetDlgItem(hWnd,IDC_ORTKEYS_AFTER), EDITTYPE_TIME);
	iAfter->SetValue(before,FALSE);

	iSamp = GetISpinner(GetDlgItem(hWnd,IDC_ORTKEYS_SAMPLESSPIN));
	iSamp->SetLimits(1, 999999999, FALSE);
	iSamp->SetScale(0.1f);
	iSamp->LinkToEdit(GetDlgItem(hWnd,IDC_ORTKEYS_SAMPLES), EDITTYPE_INT);
	iSamp->SetValue(samples,FALSE);
	}

void ORTKeysUtil::Destroy()
	{
	ReleaseISpinner(iBefore);
	ReleaseISpinner(iAfter);	
	ReleaseISpinner(iSamp);
	iu->TVUtilClosing(this);
	}

void ORTKeysUtil::BeginEditParams(Interface *ip,ITVUtility *iu)
	{
	this->ip = ip;
	this->iu = iu;	
	hWnd = CreateDialogParam(
		hInstance,
		MAKEINTRESOURCE(IDD_ORT_KEYS),
		iu->GetTrackViewHWnd(),
		ORTKeysDlgProc,
		(LONG_PTR)this);
	}

void ORTKeysUtil::EndEditParams(Interface *ip,ITVUtility *iu)
	{
	}

void ORTKeysUtil::SampleORT(
		void *tab,Control *cont, TimeValue start, TimeValue end)
	{
	Tab<float> *ftab = (Tab<float>*)tab;
	Tab<Point3> *pttab = (Tab<Point3>*)tab;
	Tab<Point4> *p4tab = (Tab<Point4>*)tab;
	Tab<ScaleValue> *stab = (Tab<ScaleValue>*)tab;
	Tab<Quat> *qtab = (Tab<Quat>*)tab;

	switch (cont->SuperClassID()) {
		case CTRL_FLOAT_CLASS_ID:			
			ftab->SetCount(samples);
			break;

		case CTRL_POSITION_CLASS_ID:
		case CTRL_POINT3_CLASS_ID:
			pttab->SetCount(samples);
			break;
		
		case CTRL_POINT4_CLASS_ID:
			p4tab->SetCount(samples);
			break;

		case CTRL_ROTATION_CLASS_ID:
			qtab->SetCount(samples);
			break;

		case CTRL_SCALE_CLASS_ID:
			stab->SetCount(samples);
			break;
		}

	// First fill a table full of values
	for (int i=0; i< samples; i++) {
		float u = float(i+1)/float(samples);
		TimeValue t = int(u*end) + int((1.0f-u)*start);

		switch (cont->SuperClassID()) {
			case CTRL_FLOAT_CLASS_ID:			
				cont->GetValue(t,&(*ftab)[i],FOREVER);
				break;

			case CTRL_POSITION_CLASS_ID:
			case CTRL_POINT3_CLASS_ID:
				cont->GetValue(t,&(*pttab)[i],FOREVER);				
				break;
			
			case CTRL_POINT4_CLASS_ID:
				cont->GetValue(t,&(*p4tab)[i],FOREVER);				
				break;

			case CTRL_ROTATION_CLASS_ID:
				cont->GetValue(t,&(*qtab)[i],FOREVER);
				break;

			case CTRL_SCALE_CLASS_ID:
				cont->GetValue(t,&(*stab)[i],FOREVER);
				break;
			}
		}
	}

void ORTKeysUtil::MakeORTKeys(
		void *tab,Control *cont, TimeValue start, TimeValue end)
	{
	Tab<float> *ftab = (Tab<float>*)tab;
	Tab<Point3> *pttab = (Tab<Point3>*)tab;
	Tab<Point4> *p4tab = (Tab<Point4>*)tab;
	Tab<ScaleValue> *stab = (Tab<ScaleValue>*)tab;
	Tab<Quat> *qtab = (Tab<Quat>*)tab;

	// Temporarily suspend ORTs
	cont->EnableORTs(FALSE);

	// Now set the keys
	for (int i=0; i< samples; i++) {
		float u = float(i+1)/float(samples);
		TimeValue t = int(u*end) + int((1.0f-u)*start);

		switch (cont->SuperClassID()) {
			case CTRL_FLOAT_CLASS_ID:			
				cont->SetValue(t,&(*ftab)[i]);
				break;

			case CTRL_POSITION_CLASS_ID:
			case CTRL_POINT3_CLASS_ID:
				cont->SetValue(t,&(*pttab)[i]);
				break;
			
			case CTRL_POINT4_CLASS_ID:
				cont->SetValue(t,&(*p4tab)[i]);
				break;

			case CTRL_ROTATION_CLASS_ID:
				cont->SetValue(t,&(*qtab)[i]);
				break;

			case CTRL_SCALE_CLASS_ID:
				cont->SetValue(t,&(*stab)[i]);
				break;
			}
		}

	// Turn ORTs back on.
	cont->EnableORTs(TRUE);
	}

void ORTKeysUtil::Apply()
	{
	Tab<float> ftabB, ftabA;
	Tab<Point3> pttabB, pttabA;
	Tab<Point4> p4tabB, p4tabA;
	Tab<ScaleValue> stabB, stabA;
	Tab<Quat> qtabB, qtabA;
	void *tabB, *tabA;

	BOOL fcurveMode = iu->GetMajorMode()==TVMODE_EDITFCURVE;

	theHold.Begin();

	// Turn animation on
	SuspendAnimate();
	AnimateOn();

	for (int i=0; i<iu->GetNumTracks(); i++) {
		if (!iu->IsSelected(i)) continue;

		// Get the control interface
		Control *cont = GetControlInterface(iu->GetAnim(i));
		if (!cont || !cont->IsLeaf() || !cont->IsKeyable()) continue;

		// Curve has to be selected when in fcurve mode.
		if (fcurveMode && !cont->IsCurveSelected()) continue;

		// Get the time range
		Interval range = cont->GetTimeRange(TIMERANGE_ALL);

		// Set the appropriate table pointers
		switch (cont->SuperClassID()) {
			case CTRL_FLOAT_CLASS_ID:			
				tabB = &ftabB; tabA = &ftabA;
				break;

			case CTRL_POSITION_CLASS_ID:
			case CTRL_POINT3_CLASS_ID:
				tabB = &pttabB; tabA = &pttabA;
				break;
			
			case CTRL_POINT4_CLASS_ID:
				tabB = &p4tabB; tabA = &p4tabA;
				break;

			case CTRL_ROTATION_CLASS_ID:
				tabB = &qtabB; tabA = &qtabA;
				break;

			case CTRL_SCALE_CLASS_ID:
				tabB = &stabB; tabA = &stabA;
				break;

			default: return;
			}

		// Samples the leys
		if (cont->GetORT(ORT_BEFORE)!=ORT_CONSTANT && before) {			
			SampleORT(tabB, cont, range.Start(), range.Start()-before);
			}		
		if (cont->GetORT(ORT_AFTER)!=ORT_CONSTANT && after) {
			SampleORT(tabA, cont, range.End(), range.End()+after);
			}		

		// Make new keys
		if (cont->GetORT(ORT_BEFORE)!=ORT_CONSTANT && before) {			
			MakeORTKeys(tabB, cont, range.Start(), range.Start()-before);
			}		
		if (cont->GetORT(ORT_AFTER)!=ORT_CONSTANT && after) {
			MakeORTKeys(tabA, cont, range.End(), range.End()+after);
			}		
		}

	ResumeAnimate();

	theHold.Accept(GetString(IDS_RB_CREATEORTKEYS));
	ip->RedrawViews(ip->GetTime());
	}
