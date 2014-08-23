/**********************************************************************
 *<
	FILE: vuedlg.cpp

	DESCRIPTION: .VUE file selector

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "rvuepch.h"
#include "resource.h"
#include "rendvue.h"

class RendVueParamDlg : public RendParamDlg {
public:
	VueRenderer *rend;
	IRendParams *ir;
	HWND hPanel;
	BOOL prog;
	HFONT hFont;
	TSTR workFileName;
	RendVueParamDlg(VueRenderer *r,IRendParams *i,BOOL prog);
	~RendVueParamDlg();
	void AcceptParams();
	void DeleteThis() {delete this;}
	void InitParamDialog(HWND hWnd);
	void InitProgDialog(HWND hWnd);
	void ReleaseControls() {}
	BOOL FileBrowse();
	BOOL WndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
	};

RendVueParamDlg::~RendVueParamDlg()
	{
	DeleteObject(hFont);
	ir->DeleteRollupPage(hPanel);
	}

BOOL RendVueParamDlg::WndProc(
		HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG:
			if (prog) InitProgDialog(hWnd);
			else InitParamDialog(hWnd);
			break;
		
		case WM_DESTROY:
			if (!prog) ReleaseControls();
			break;
		default:
			return FALSE;
		}
	return TRUE;
	}

static INT_PTR CALLBACK RendVueParamDlgProc(
		HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	RendVueParamDlg *dlg = (RendVueParamDlg*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	switch (msg) {
		case WM_INITDIALOG:
			dlg = (RendVueParamDlg*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			break;
		case WM_LBUTTONDOWN:
		case WM_MOUSEMOVE:
		case WM_LBUTTONUP:
			dlg->ir->RollupMouseMessage(hWnd,msg,wParam,lParam);
			break;
		case WM_COMMAND:
			case IDC_RENDVUE_FILE:
				if (dlg->FileBrowse()) {
					SetDlgItemText(hWnd,IDC_RENDVUE_FILENAME, dlg->workFileName.data());
					}
				break;
		case CC_SPINNER_CHANGE:   
			{
			}
			break;
		}	
	if (dlg) return dlg->WndProc(hWnd,msg,wParam,lParam);
	else return FALSE;
	}

RendVueParamDlg::RendVueParamDlg(
		VueRenderer *r,IRendParams *i,BOOL prog)
	{
	hFont      = hFont = CreateFont(14,0,0,0,FW_BOLD,0,0,0,0,0,0,0, VARIABLE_PITCH | FF_SWISS, _T(""));
	rend       = r;
	ir         = i;
	this->prog = prog;
	if (prog) {		
		hPanel = ir->AddRollupPage(
			hInstance, 
			MAKEINTRESOURCE(IDD_RENDVUE_PROG),
			RendVueParamDlgProc,
			GetString(IDS_VRENDTITLE),
			(LPARAM)this);
	} else {
		hPanel = ir->AddRollupPage(
			hInstance, 
			MAKEINTRESOURCE(IDD_RENDVUE_PARAMS),
			RendVueParamDlgProc,
			GetString(IDS_VRENDTITLE),
			(LPARAM)this);
		}
	}

void RendVueParamDlg::InitParamDialog(HWND hWnd) {
	workFileName = rend->vueFileName;
	SetDlgItemText(hWnd,IDC_RENDVUE_FILENAME, workFileName);
	}

void RendVueParamDlg::InitProgDialog(HWND hWnd) {
	SetDlgItemText(hWnd,IDC_RENDVUE_FILENAME, rend->vueFileName.data());
	}

void RendVueParamDlg::AcceptParams() {
	rend->vueFileName = workFileName;
	}

RendParamDlg * VueRenderer::CreateParamDialog(IRendParams *ir,BOOL prog) {
	return new RendVueParamDlg(this,ir,prog);
	}



// File Browse ------------------------------------------------------------
BOOL FileExists(TCHAR *filename) {
     HANDLE findhandle;
     WIN32_FIND_DATA file;
     findhandle = FindFirstFile(filename,&file);
     FindClose(findhandle);
     if (findhandle == INVALID_HANDLE_VALUE)
        return(FALSE);
     else
        return(TRUE);
	}

BOOL RunningNewShell() 
	{
	OSVERSIONINFO os;
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&os);
	if (os.dwPlatformId == VER_PLATFORM_WIN32_NT && os.dwMajorVersion < 4)
		return FALSE;
	return TRUE;
	}
	

#define VUEEXT _T(".vue")
#define VUEFILTER _T("*.vue")

void FixFileExt(OPENFILENAME &ofn, TCHAR* ext = VUEEXT) {
	int l = _tcslen(ofn.lpstrFile);
	int e = _tcslen(ext);
	if (_tcsicmp(ofn.lpstrFile+l-e, ext)) {
		_tcscat(ofn.lpstrFile,ext);	
		}
	}

#if 0
BOOL WINAPI FileHook( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
	switch (message) {
		case WM_INITDIALOG:
			SetDlgItemText(hDlg, IDOK, _T("OK"));
			break;
		case WM_COMMAND:{
			}
			
			break;
		}
	return FALSE;
	}  

BOOL PMFileHook(HWND hWnd,UINT message,WPARAM wParam,LPARAM	lParam) 
	{
	switch (message) {
		case WM_INITDIALOG:
			SetDlgItemText(hWnd, IDOK, _T("OK"));
			break;
		case WM_COMMAND:{
			}
			
			break;
		}
	return 0;
	}
#endif

BOOL RendVueParamDlg::FileBrowse() {
	int tried = 0;
	FilterList filterList;
	HWND hWnd = hPanel;
	static int filterIndex = 1;
    OPENFILENAME  ofn;
	TSTR filename;
    TCHAR fname[512];
	TCHAR saveDir[1024];
		{
		TSTR dir;
		SplitFilename(workFileName, &dir, &filename,NULL);
		_tcscpy(saveDir,dir.data());
		}
	_tcscpy(fname,filename.data());
	_tcscat(fname, VUEEXT);

	filterList.Append(GetString(IDS_VUE_FILE));
	filterList.Append(VUEFILTER);

    memset(&ofn, 0, sizeof(OPENFILENAME));

    ofn.lStructSize      = sizeof(OPENFILENAME);
    ofn.hwndOwner        = hWnd;
	ofn.hInstance        = hInstance;	


	ofn.nFilterIndex = filterIndex;
    ofn.lpstrFilter  = filterList;

    ofn.lpstrTitle   = GetString(IDS_WRITE_VUEFILE); 
    ofn.lpstrFile    = fname;
    ofn.nMaxFile     = sizeof(fname) / sizeof(TCHAR);      

	Interface *iface = GetCOREInterface();
	
	if(saveDir[0])
	   	ofn.lpstrInitialDir = saveDir;
	else
  	 	ofn.lpstrInitialDir = iface->GetDir(APP_SCENE_DIR);

	if(RunningNewShell()) {
		ofn.Flags = OFN_HIDEREADONLY | OFN_EXPLORER /* | OFN_ENABLEHOOK | OFN_ENABLETEMPLATE*/;  // OFN_OVERWRITEPROMPT;
		ofn.lpfnHook = NULL;// (LPOFNHOOKPROC)FileHook;
		ofn.lCustData = 0;		// 0 for save, 1 for open
//		ofn.lpTemplateName = MAKEINTRESOURCE(IDD_EXT_FILE);
	}
	else {
		ofn.Flags			  =	OFN_HIDEREADONLY | OFN_PATHMUSTEXIST /* |OFN_ENABLEHOOK  | OFN_ENABLETEMPLATE */;
		ofn.lpfnHook		  =	NULL; // (LPOFNHOOKPROC)PMFileHook;
		ofn.lCustData		  =	0;
//		ofn.lpTemplateName	  =	MAKEINTRESOURCE(IDD_PM_EXT_FILE);
	}

	FixFileExt(ofn,VUEEXT); // add ".vue" if absent

	while (GetSaveFileName(&ofn)) 	{
		FixFileExt(ofn,VUEEXT); // add ".vue" if absent

		workFileName = ofn.lpstrFile;
		return TRUE;
		}
	return FALSE;
	}
