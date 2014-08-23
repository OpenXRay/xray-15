/**********************************************************************
 *<
	FILE: appdata.cpp

	DESCRIPTION:  A utility to test appdata

	CREATED BY: Rolf Berteig

	HISTORY: created 3/10/96

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "util.h"
#include "utilapi.h"

#define APPDATA_TEST_CLASS_ID		0xf6277abd

class AppDataTest : public UtilityObj {
	public:
		IUtil *iu;
		Interface *ip;
		HWND hPanel;		
		ISpinnerControl *spin;

		AppDataTest();
		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);
		void DeleteThis() {}

		void Init(HWND hWnd);
		void Destroy(HWND hWnd);

		Animatable *PickAnim();
		void GetAppData();
		void PutAppData();
	};
static AppDataTest theAppDataTest;

class AppDataTestClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return &theAppDataTest;}
	const TCHAR *	ClassName() {return GetString(IDS_RB_APPDATATEST);}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return Class_ID(APPDATA_TEST_CLASS_ID,0);}
	const TCHAR* 	Category() {return _T("");}
	};

static AppDataTestClassDesc appDataTestDesc;
ClassDesc* GetAppDataTestDesc() {return &appDataTestDesc;}

static bool s_through_xref = 0;	// 020416  --prs.

static INT_PTR CALLBACK AppDataTestDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG:
			theAppDataTest.Init(hWnd);
			CheckDlgButton(hWnd, IDC_APPDATA_XREF, s_through_xref);
			break;
		
		case WM_DESTROY:
			theAppDataTest.Destroy(hWnd);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					theAppDataTest.iu->CloseUtility();
					break;

				case IDC_APPDATA_GET:
					theAppDataTest.GetAppData();
					break;

				case IDC_APPDATA_PUT:
					theAppDataTest.PutAppData();
					break;

				case IDC_APPDATA_EDIT:
					switch (HIWORD(wParam)) {
						case EN_SETFOCUS:
							DisableAccelerators();
							break;
						case EN_KILLFOCUS:
							EnableAccelerators();
							break;
						}
					break;
				case IDC_APPDATA_XREF:
					s_through_xref = IsDlgButtonChecked(hWnd, IDC_APPDATA_XREF)?true:false;
					break;
				}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			theAppDataTest.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
		}
	return TRUE; 
	}

AppDataTest::AppDataTest()
	{
	iu = NULL;
	ip = NULL;	
	hPanel = NULL;	
	spin = NULL;
	}

void AppDataTest::BeginEditParams(Interface *ip,IUtil *iu) 
	{
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_APPDATA_PANEL),
		AppDataTestDlgProc,
		GetString(IDS_RB_APPDATATEST),
		0);
	}
	
void AppDataTest::EndEditParams(Interface *ip,IUtil *iu) 
	{	
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
	}

void AppDataTest::Init(HWND hWnd)
	{
	spin = SetupIntSpinner(hWnd,IDC_APPDATA_SLOTSPIN,IDC_APPDATA_SLOT,0,99999999,0);
	}

void AppDataTest::Destroy(HWND hWnd)
	{
	ReleaseISpinner(spin);
	}

Animatable *AppDataTest::PickAnim()
	{
	TrackViewPick pick;
	if (ip->TrackViewPickDlg(hPanel,&pick)) {
		return pick.anim;
	} else {
		return NULL;
		}
	}

void AppDataTest::GetAppData()
	{
	Animatable *anim = PickAnim();
	if (!anim) return;

	// XRef stuff 020416  --prs.
	if (s_through_xref) {
		RefTargetHandle rth = (RefTargetHandle)anim;
		while (rth != NULL && rth->ClassID() == Class_ID(XREFOBJ_CLASS_ID,0))
			rth = rth->GetReference(0);
		if (rth != NULL)
			anim = rth;
	}

	// Grab the app data chunk from the anim
	AppDataChunk *ad = 
		anim->GetAppDataChunk(
			Class_ID(APPDATA_TEST_CLASS_ID,0), 
			UTILITY_CLASS_ID, 
			spin->GetIVal());
	if (ad) {
		// Fill the edit field with the text.
		SetDlgItemText(hPanel,IDC_APPDATA_EDIT,(TCHAR*)ad->data);
	} else {
		// Not found!
		MessageBox(hPanel,_T("No app data found"),_T("App Data Tester"),MB_OK);
		}
	}

void AppDataTest::PutAppData()
	{
	Animatable *anim = PickAnim();
	if (!anim) return;

	// XRef stuff 020416  --prs.
	if (s_through_xref) {
		RefTargetHandle rth = (RefTargetHandle)anim;
		while (rth != NULL && rth->ClassID() == Class_ID(XREFOBJ_CLASS_ID,0))
			rth = rth->GetReference(0);
		if (rth != NULL)
			anim = rth;
	}

	// Load the text out of the edit field into a buffer
	int len = GetWindowTextLength(GetDlgItem(hPanel,IDC_APPDATA_EDIT))+1;
	TCHAR *buf = (TCHAR*)malloc(len*sizeof(TCHAR));
	GetWindowText(
		GetDlgItem(hPanel,IDC_APPDATA_EDIT),
		buf, len);
	buf[len-1] = '\0';

	// Remove the chunk if it exists
	anim->RemoveAppDataChunk(
		Class_ID(APPDATA_TEST_CLASS_ID,0), 
		UTILITY_CLASS_ID, 
		spin->GetIVal());

	// Add a chunk
	anim->AddAppDataChunk(
		Class_ID(APPDATA_TEST_CLASS_ID,0), 
		UTILITY_CLASS_ID, 
		spin->GetIVal(),
		(DWORD)len, buf);
	}