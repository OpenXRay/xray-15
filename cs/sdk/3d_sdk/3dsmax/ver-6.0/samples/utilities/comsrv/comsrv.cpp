//-----------------------------------------------------------------------------
// ---------------------
// File ....: comsrv.cpp
// ---------------------
// Author...: Gus J Grubba
// Date ....: October 1998
// Descr....: COM/DCOM Server Control Utility
//
// Shows how to control a "GUP" utility (which has no UI)
//            
//-----------------------------------------------------------------------------

#include "comsrv.h"

//-- Globals ------------------------------------------------------------------

HINSTANCE hInst = NULL;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-- DLL Declaration

BOOL WINAPI DllMain ( HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved ) {

	static BOOL controlsInit = FALSE;

	switch (fdwReason) {

		case DLL_PROCESS_ATTACH:
			if (hInst)
				return(FALSE);
			hInst = hinstDLL;
			if (!controlsInit) {
				controlsInit = TRUE;
				InitCustomControls(hInst);
				InitCommonControls();
			}
			break;

		case DLL_PROCESS_DETACH:
			hInst  = NULL;
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;

	}
			
	return (TRUE);
}

//-----------------------------------------------------------------------------
//-- Helper

TCHAR *GetString(int id) {
	static TCHAR buf[256];
	if (hInst)
		return LoadString(hInst, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

//-----------------------------------------------------------------------------
//-- The Utility Object

static COMsrv comsrvManager;

//-----------------------------------------------------------------------------
//-- Interface Class Description

class COMsrvClassDesc : public ClassDesc {
	public:
		int 			IsPublic		( ) {return 1;}
		void*			Create			( BOOL loading = FALSE) {return &comsrvManager;}
		const TCHAR*	ClassName		( ) {return GetString(IDS_CLASS_DESC);}
		SClass_ID		SuperClassID	( ) {return UTILITY_CLASS_ID;}
		Class_ID		ClassID			( ) {return Class_ID(0x129AD7F2,10);}
		const TCHAR* 	Category		( ) {return _T("");}
};

static COMsrvClassDesc iflClassDesc;

//-----------------------------------------------------------------------------
// Max Interface

DLLEXPORT const TCHAR * LibDescription ( )  { 
	return GetString(IDS_LIBDESCRIPTION); 
}

DLLEXPORT int LibNumberClasses ( ) { 
	return 1; 
}

DLLEXPORT ClassDesc *LibClassDesc(int i) {
	  switch(i) {
		 case  0: return &iflClassDesc;	break;
		 default: return 0;				break;
	  }
}

DLLEXPORT ULONG LibVersion ( )  { 
	  return ( VERSION_3DSMAX ); 
}

//-----------------------------------------------------------------------------
// Dialogue Handler

INT_PTR CALLBACK iflDlgProc(	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {

	switch (msg) {
		
		case WM_INITDIALOG:
			comsrvManager.Init(hWnd);			
			break;
		
		case WM_DESTROY:
			comsrvManager.Destroy(hWnd);
			break;
		
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_REGISTER:
					comsrvManager.Register(hWnd);
					break;
				case IDOK:
					comsrvManager.iu->CloseUtility();
					break;
			}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			comsrvManager.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
	}

	return TRUE; 

}

//-----------------------------------------------------------------------------
// #> COMsrv::COMsrv()

COMsrv::COMsrv() {
	iu		= NULL;
	ip		= NULL;	
	comgup	= NULL;
	hPanel	= NULL;	
}

//-----------------------------------------------------------------------------
// #> COMsrv::BeginEditParams()

void COMsrv::BeginEditParams( Interface *ip, IUtil *iu ) {
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(
		hInst,
		MAKEINTRESOURCE(IDC_COMSRV_DLG),
		iflDlgProc,
		GetString(IDS_CLASS_DESC),
		0);
}
	
//-----------------------------------------------------------------------------
// #> COMsrv::EndEditParams()

void COMsrv::EndEditParams( Interface *ip,IUtil *iu ) {	
	this->iu	= NULL;
	this->ip	= NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel		= NULL;
}

//-----------------------------------------------------------------------------
// #> COMsrv::UpdateButton()

void COMsrv::UpdateButton( HWND hWnd ) {
	if (ISCOMREGISTERED)
		SetDlgItemText(hWnd,IDC_REGISTER,GetString(IDS_UNREGISTER));
	else
		SetDlgItemText(hWnd,IDC_REGISTER,GetString(IDS_REGISTER));
}

//-----------------------------------------------------------------------------
// #> COMsrv::Init()

void COMsrv::Init ( HWND hWnd ) {
	comgup = OpenGupPlugIn(Class_ID(470000002,0));
	if (!comgup) {
		TCHAR title[128];
		_tcscpy(title,GetString(IDS_DLG_TITLE));
		MessageBox(hWnd,GetString(IDS_CANT_FIND_GUP),title,MB_OK);
		return;
	}
	HWND hDlg = GetDlgItem(hWnd,IDC_REGISTER);
	EnableWindow(hDlg,TRUE);
	UpdateButton(hWnd);
}

//-----------------------------------------------------------------------------
// #> COMsrv::Destroy()

void COMsrv::Destroy ( HWND hWnd ) {
	if (comgup) {
		//delete comgup;
		comgup->DeleteThis();
		comgup = NULL;
	}
}

//-----------------------------------------------------------------------------
// #> COMsrv::Register()

void COMsrv::Register( HWND hWnd ) {
	HCURSOR oldcur = SetCursor(LoadCursor(NULL,IDC_WAIT));
	if (ISCOMREGISTERED)
		UNREGISTERCOM;
	else
		REGISTERCOM;
	UpdateButton(hWnd);
	SetCursor(oldcur);
}

//-- EOF: comsrv.cpp ----------------------------------------------------------
