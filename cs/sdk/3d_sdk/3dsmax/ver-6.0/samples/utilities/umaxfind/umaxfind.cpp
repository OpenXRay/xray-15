/**********************************************************************
 *<
	FILE:			UMaxFind.cpp

	DESCRIPTION:	Utility to startup external MAXFinder executable.

	CREATED BY:		Christer Janson

	HISTORY:		Created Monday, October 05, 1998

 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/

#include "UMaxFind.h"
#include "buildver.h"

HINSTANCE hInstance;

#define UMAXFIND_CLASS_ID	Class_ID(0x72bb49f5, 0xa7a1cb4f)

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
	switch (fdwReason) {
		case DLL_PROCESS_ATTACH:
			hInstance = hinstDLL;
			InitCustomControls(hInstance);
			InitCommonControls();
			break;
		case DLL_PROCESS_DETACH:
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
	}

	return (TRUE);
}

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

__declspec( dllexport ) const TCHAR* LibDescription()
{
	return GetString(IDS_LIBDESC);
}

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses()
{
	return 1;
}


__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
{
	switch(i) {
		case 0: return GetUMaxFindDesc();
		default: return 0;
	}
}

__declspec( dllexport ) ULONG LibVersion()
{
	return VERSION_3DSMAX;
}

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}

class UMaxFind : public UtilityObj {
	public:
		IUtil *iu;
		Interface *ip;
		HWND hPanel;

		UMaxFind();
		~UMaxFind();

		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);
		void DeleteThis() {}

		void Init(HWND hWnd);
		void Destroy(HWND hWnd);
		void StartFileFinder();
};

static UMaxFind theUMaxFind;

class UMaxFindClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return &theUMaxFind;}
	const TCHAR *	ClassName() {return GetString(IDS_CLASSNAME);}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return UMAXFIND_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
};

static UMaxFindClassDesc UMaxFindDesc;
ClassDesc* GetUMaxFindDesc() {return &UMaxFindDesc;}


static INT_PTR CALLBACK UMaxFindDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_INITDIALOG:
			theUMaxFind.Init(hWnd);
			break;

		case WM_DESTROY:
			theUMaxFind.Destroy(hWnd);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_CLOSEBUTTON:
					theUMaxFind.iu->CloseUtility();
					break;
				case IDC_START:
					theUMaxFind.StartFileFinder();
					break;
			}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			theUMaxFind.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
	}
	return TRUE;
}	

UMaxFind::UMaxFind()
{
	iu = NULL;
	ip = NULL;	
	hPanel = NULL;
}

UMaxFind::~UMaxFind()
{
}

void UMaxFind::BeginEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_PANEL),
		UMaxFindDlgProc,
		GetString(IDS_TITLE),
		0);
}
	
void UMaxFind::EndEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
}

void UMaxFind::Init(HWND hWnd)
{
}

void UMaxFind::Destroy(HWND hWnd)
{
}

void UMaxFind::StartFileFinder()
{
	TCHAR				szExeFile[_MAX_PATH];
	TSTR				szPath;
	PROCESS_INFORMATION	process;
	STARTUPINFO			si;
	BOOL				res;

	memset(&si, 0, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow = SW_SHOWDEFAULT;
	si.hStdError = NULL;


	GetModuleFileName(NULL, szExeFile, _MAX_PATH);
	SplitFilename(TSTR(szExeFile), &szPath, NULL, NULL);
	szPath = szPath+GetString(IDS_EXE_FILENAME);	

	res = CreateProcess(NULL, szPath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &process);
	if (!res) {
		// Error
		TSTR msg = GetString(IDS_ERROR);
		MessageBox(hPanel, GetString(IDS_NOEXE), msg, MB_OK | MB_ICONEXCLAMATION);
		return;
		}

	CloseHandle(process.hThread);
}
