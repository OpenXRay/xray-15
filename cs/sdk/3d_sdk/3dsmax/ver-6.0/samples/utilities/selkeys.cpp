/**********************************************************************																  /**********************************************************************
 *<
	FILE: selkeys.cpp

	DESCRIPTION: A Track View Utility that selects keys in the current time selection

	CREATED BY: Rolf Berteig

	HISTORY: created 12/18/96

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/
#include "util.h"
#include "tvutil.h"
#include "istdplug.h"

#define SEL_KEYS_NAME		GetString(IDS_RB_SELKEYS)
#define SEL_KEYS_CLASS_ID	Class_ID(0x18be63a6,0xcf32a8d3)

class SelKeysUtil : public TrackViewUtility {
	public:				
		Interface *ip;
		ITVUtility *iu;
		HWND hWnd;
		int clear;
		SelKeysUtil() {ip=NULL;iu=NULL;hWnd=NULL;clear=TRUE;}
		void DeleteThis() {if (hWnd) DestroyWindow(hWnd);} 
		void BeginEditParams(Interface *ip,ITVUtility *iu);		
		void		GetClassName(TSTR& s)	{ s = SEL_KEYS_NAME; }  
		Class_ID	ClassID()				{ return SEL_KEYS_CLASS_ID; }

		void SetupWindow(HWND hWnd);
		void SelectKeys(HWND hWnd);

		void Destroy(){hWnd = NULL; if (iu) iu->TVUtilClosing(this);}

	};
SelKeysUtil theSelKeysUtil;

class SelKeysClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return &theSelKeysUtil;}
	const TCHAR *	ClassName() {return SEL_KEYS_NAME;}
	SClass_ID		SuperClassID() {return TRACKVIEW_UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return SEL_KEYS_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
	};

static SelKeysClassDesc selKeysDesc;
ClassDesc* GetSelKeysDesc() {return &selKeysDesc;}


void SelKeysUtil::SetupWindow(HWND hWnd)
	{
	Interval iv = iu->GetTimeSelection();
	ISpinnerControl *spin;
	spin = GetISpinner(GetDlgItem(hWnd,IDC_SELKEYS_STARTSPIN));
	spin->SetLimits(TIME_NegInfinity, TIME_PosInfinity, FALSE);
	spin->SetScale(10.0f);
	spin->LinkToEdit(GetDlgItem(hWnd,IDC_SELKEYS_START), EDITTYPE_TIME);
	spin->SetValue(iv.Start(),FALSE);
	ReleaseISpinner(spin);

	spin = GetISpinner(GetDlgItem(hWnd,IDC_SELKEYS_ENDSPIN));
	spin->SetLimits(TIME_NegInfinity, TIME_PosInfinity, FALSE);
	spin->SetScale(10.0f);
	spin->LinkToEdit(GetDlgItem(hWnd,IDC_SELKEYS_END), EDITTYPE_TIME);
	spin->SetValue(iv.End(),FALSE);
	ReleaseISpinner(spin);

	CheckDlgButton(hWnd,IDC_SELKEYS_CLEAR,clear);
	}

class ClearWorkFlagEnumProc : public AnimEnum {		
	public:
		ClearWorkFlagEnumProc() : AnimEnum(SCOPE_ALL) {}
		int proc(Animatable *anim, Animatable *client, int subNum) {
			anim->ClearAFlag(A_WORK1);
			return ANIM_ENUM_PROCEED;
			}
	};

class SelKeysEnumProc : public AnimEnum {
	public:
		TimeValue start, end;
		BOOL clear;
		SelKeysEnumProc(TimeValue s, TimeValue e, BOOL c) : AnimEnum(SCOPE_ALL)
			{start=s; end=e; clear=c;}
		int proc(Animatable *anim, Animatable *client, int subNum) {
			if (anim->TestAFlag(A_WORK1)) return ANIM_ENUM_PROCEED;
			anim->SetAFlag(A_WORK1);
			if (clear) {
				TrackHitTab none;
				anim->SelectKeys(none, SELKEYS_CLEARKEYS);
			} else {			
				for (int j=0; j<anim->NumKeys(); j++) {				
					TimeValue t = anim->GetKeyTime(j);
					if (t>=start && t<=end) {
						anim->SelectKeyByIndex(j,TRUE);
						}
					}
				}
			return ANIM_ENUM_PROCEED;
			}
	};

void SelKeysUtil::SelectKeys(HWND hWnd)
	{
	clear = IsDlgButtonChecked(hWnd,IDC_SELKEYS_CLEAR);
	ISpinnerControl *spin;
	spin = GetISpinner(GetDlgItem(hWnd,IDC_SELKEYS_STARTSPIN));
	TimeValue start = spin->GetIVal();
	ReleaseISpinner(spin);
	spin = GetISpinner(GetDlgItem(hWnd,IDC_SELKEYS_ENDSPIN));
	TimeValue end = spin->GetIVal();
	ReleaseISpinner(spin);
	if (start>end) {
		TimeValue temp = end;
		end   = start;
		start = temp;
		}
	theHold.Begin();
	
	if (iu->SubTreeMode()) {
		ClearWorkFlagEnumProc proc;
		iu->GetTVRoot()->EnumAnimTree(&proc,NULL,0);
		}

	if (clear) {
		for (int i=0; i<iu->GetNumTracks(); i++) {			
			Animatable *anim = iu->GetAnim(i);
			if (iu->SubTreeMode()) {
				SelKeysEnumProc proc(start,end,TRUE);
				anim->EnumAnimTree(&proc,NULL,0);
			} else {
				if (clear) {
					TrackHitTab none;
					anim->SelectKeys(none, SELKEYS_CLEARKEYS);
					}				
				}				
			}

		if (iu->SubTreeMode()) {
			ClearWorkFlagEnumProc proc;
			iu->GetTVRoot()->EnumAnimTree(&proc,NULL,0);
			}
		}

	for (int i=0; i<iu->GetNumTracks(); i++) {
		if (iu->IsSelected(i)) {
			Animatable *anim = iu->GetAnim(i);
			if (iu->SubTreeMode()) {
				SelKeysEnumProc proc(start,end,FALSE);
				anim->EnumAnimTree(&proc,NULL,0);
			} else {							
				for (int j=0; j<anim->NumKeys(); j++) {				
					TimeValue t = anim->GetKeyTime(j);
					if (t>=start && t<=end) {
						anim->SelectKeyByIndex(j,TRUE);
						}					
					}
				}
			}
		}
	theHold.Accept(SEL_KEYS_NAME);
	}

static INT_PTR CALLBACK SelKeysDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG:
			CenterWindow(hWnd,GetParent(hWnd));
			theSelKeysUtil.SetupWindow(hWnd);
			break;

		case WM_COMMAND:			
			switch (LOWORD(wParam)) {				
				case IDOK:
					theSelKeysUtil.SelectKeys(hWnd);
					EndDialog(hWnd, 1);
					break;

				case IDCANCEL:
					EndDialog(hWnd,0);
					break;
				}
			break;

		case WM_DESTROY:
			theSelKeysUtil.Destroy();			
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}

void SelKeysUtil::BeginEditParams(Interface *ip,ITVUtility *iu)
	{
	this->ip = ip;
	this->iu = iu;
	DialogBox(
		hInstance,
		MAKEINTRESOURCE(IDD_SEL_KEYS),
		iu->GetTrackViewHWnd(),
		SelKeysDlgProc);
	}
