//-----------------------------------------------------------------------------
// ---------------------
// File ....: iflmgr.cpp
// ---------------------
// Author...: Gus J Grubba
// Date ....: March 1997
// Descr....: IFL Manager Utility
//
// History .: Mar, 18 1997 - Started
//            
//-----------------------------------------------------------------------------

#include "iflmgr.h"

//-- Globals ------------------------------------------------------------------

HINSTANCE hInst = NULL;

#define NUMCONTROLS 9

static int ControlArray[] = {
	IDC_START_ED,
	IDC_START_SP,
	IDC_END_ED,
	IDC_END_SP,
	IDC_NTH_ED,
	IDC_NTH_SP,
	IDC_MULT_ED,
	IDC_MULT_SP,
	IDC_CREATE
};

//-----------------------------------------------------------------------------
// *> Sort IFL list

static BOOL backward = FALSE;

static int CompTable( const void *elem1, const void *elem2 ) {
	TCHAR *a = (TCHAR *)elem1;
	TCHAR *b = (TCHAR *)elem2;
	//if (backward)
	//	return(_tcscmp(b,a));
	//else
		return(_tcscmp(a,b));
}

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

static IFLMgr iflManager;

//-----------------------------------------------------------------------------
//-- Interface Class Description

class IFLMgrClassDesc : public ClassDesc {
	public:
		int 			IsPublic		( ) {return 1;}
		void*			Create			( BOOL loading = FALSE) {return &iflManager;}
		const TCHAR*	ClassName		( ) {return GetString(IDS_CLASS_DESC);}
		SClass_ID		SuperClassID	( ) {return UTILITY_CLASS_ID;}
		Class_ID		ClassID			( ) {return Class_ID(0x129AD7F1,0);}
		const TCHAR* 	Category		( ) {return _T("");}
};

static IFLMgrClassDesc iflClassDesc;

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

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}

//-----------------------------------------------------------------------------
// Dialogue Handler

INT_PTR CALLBACK iflDlgProc(	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {

	switch (msg) {
		
		case WM_INITDIALOG:
			iflManager.Init(hWnd);			
			break;
		
		case WM_DESTROY:
			iflManager.Destroy(hWnd);
			break;
		
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_CREATE:
					iflManager.Create(hWnd);
					break;
				case IDC_EDIT:
					iflManager.Edit(hWnd);
					break;
				case IDC_SELECT:
					iflManager.Select(hWnd);
					break;
				case IDOK:
					iflManager.iu->CloseUtility();
					break;
			}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			iflManager.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
	}

	return TRUE; 

}

//-----------------------------------------------------------------------------
// #> IFLMgr::IFLMgr()

IFLMgr::IFLMgr() {
	iu			= NULL;
	ip			= NULL;	
	hPanel		= NULL;	
	workpath[0] = 0;
}

//-----------------------------------------------------------------------------
// #> IFLMgr::BeginEditParams()

void IFLMgr::BeginEditParams( Interface *ip, IUtil *iu ) {
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(
		hInst,
		MAKEINTRESOURCE(IDC_IFLMGR_DLG),
		iflDlgProc,
		GetString(IDS_CLASS_DESC),
		0);
}
	
//-----------------------------------------------------------------------------
// #> IFLMgr::EndEditParams()

void IFLMgr::EndEditParams( Interface *ip,IUtil *iu ) {	
	this->iu	= NULL;
	this->ip	= NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel		= NULL;
}

//-----------------------------------------------------------------------------
// #> IFLMgr::Init()

void IFLMgr::Init ( HWND hWnd ) {
	for (int i = 0; i < NUMCONTROLS; i++) 
		ToggleControl(hWnd,ControlArray[i],FALSE);
}

//-----------------------------------------------------------------------------
// #> IFLMgr::Destroy()

void IFLMgr::Destroy ( HWND hWnd ) {
	DestroySpinner(hWnd,IDC_START_SP);
	DestroySpinner(hWnd,IDC_END_SP);
	DestroySpinner(hWnd,IDC_NTH_SP);
	DestroySpinner(hWnd,IDC_MULT_SP);
}

//-----------------------------------------------------------------------------
// #> IFLMgr::Create()

void IFLMgr::Create ( HWND hWnd ) {

	if (table.Count() < 2)
		return;

	FilterList filterList;
	filterList.Append( GetString(IDS_IFL_FILES));
	filterList.Append( _T("*.ifl"));
	filterList.Append( _T(""));
	filterList.Append( _T(""));

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(OPENFILENAME));

	TCHAR filename[MAX_PATH];
	filename[0] = 0;
	ofn.lpstrFile		= filename;
	ofn.nMaxFile		= MAX_PATH;

	ofn.lpstrFilter		= filterList;
	ofn.lpstrTitle		= GetString(IDS_CREATE_TITLE);
	ofn.Flags			= OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
	ofn.lStructSize		= sizeof(OPENFILENAME);
	ofn.hwndOwner		= hWnd;

	if (workpath[0])
		ofn.lpstrInitialDir	= workpath;
	else
		ofn.lpstrInitialDir	= ip->GetMapDir(0);

	while (1) {
		
		if (GetSaveFileName(&ofn)) {

			//-- Make sure there is an extension ----------

			int l = _tcslen(ofn.lpstrFile);
			if (!l)
				return;
			if (l==ofn.nFileExtension || !ofn.nFileExtension) 
			_tcscat(ofn.lpstrFile,_T(".ifl"));  
			
			//-- Check for file overwrite -----------------

			if (BMMIsFile(ofn.lpstrFile)) {
				TCHAR text[MAX_PATH];
				wsprintf(text,GetString(IDS_OVERWRITE_TXT),ofn.lpstrFile);
				TSTR tit = GetString(IDS_OVERWRITE_TIT);
				if (MessageBox(hWnd,text,tit,MB_APPLMODAL | MB_ICONQUESTION | MB_YESNO) != IDYES)
					continue;
			}
			
			//-- Create IFL file --------------------------

			FILE *f = _tfopen(ofn.lpstrFile,"wt");
			if (f) {

				//-- Get Arguments ------------------------

				int start	= max(GetSpinnerValue(hWnd,IDC_START_SP),0);
				int end		= min(GetSpinnerValue(hWnd,IDC_END_SP),table.Count()-1);
				int nth		= GetSpinnerValue(hWnd,IDC_NTH_SP);
				int mult	= GetSpinnerValue(hWnd,IDC_MULT_SP);

				if (start > end) {
					int t = start;
					start = end;
					end = t;
					backward = TRUE;
				} else
					backward = FALSE;

				table.Sort(CompTable);

				//-- Append list of image files -----------

				if (backward) {
					for (int i = end; i >= start; i -= nth ) {
						if (mult > 1)
							fprintf(f,"%s %d\n",table[i].file,mult);
						else
							fprintf(f,"%s\n",table[i].file);
					}
				} else {
					for (int i = start; i <= end; i += nth ) {
						if (mult > 1)
							fprintf(f,"%s %d\n",table[i].file,mult);
						else
							fprintf(f,"%s\n",table[i].file);
					}
				}

				fclose(f);
				BMMSplitFilename(ofn.lpstrFile,workpath,NULL,NULL);

			}

			//-- Kaput ------------------------------------
			
			//iflManager.iu->CloseUtility();

		}

		break;
	
	}

}

//-----------------------------------------------------------------------------
// #> IFLMgr::Select()

void IFLMgr::Select ( HWND hWnd ) {

#if 0

	FilterList filterList;
	filterList.Append( GetString(IDS_ALL_FILES));
	filterList.Append( _T("*.*"));
	filterList.Append( _T(""));
	filterList.Append( _T(""));

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(OPENFILENAME));
	TCHAR filename[MAX_PATH];
	filename[0] = 0;

	ofn.lpstrFilter		= filterList;
	ofn.lpstrTitle		= GetString(IDS_IMAGE_FILES);
	ofn.Flags			= OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
	ofn.lStructSize		= sizeof(OPENFILENAME);
	ofn.hwndOwner		= hWnd;
	ofn.lpstrFile		= filename;
	ofn.nMaxFile		= MAX_PATH;

	if (workpath[0])
		ofn.lpstrInitialDir	= workpath;
	else
		ofn.lpstrInitialDir	= ip->GetMapDir(0);

	if (GetOpenFileName(&ofn)) {
		
		if (!BuildFileList(hWnd,ofn.lpstrFile)) 
			return;
		
		TCHAR title[MAX_PATH];
		wsprintf(title,_T(" %s "),root);
		SetDlgItemText(hWnd,IDC_FRAME,title);

		for (int i = 0; i < NUMCONTROLS; i++) 
			ToggleControl(hWnd,ControlArray[i],TRUE);
		
		int c = table.Count();
		InitSpinner(hWnd,IDC_START_ED,	IDC_START_SP,	0,0,c-1);
		InitSpinner(hWnd,IDC_END_ED,	IDC_END_SP,		c-1,0,c-1);
		InitSpinner(hWnd,IDC_NTH_ED,	IDC_NTH_SP,		1,1,c-1);
		InitSpinner(hWnd,IDC_MULT_ED,	IDC_MULT_SP,	1,1,9999);

		BMMSplitFilename(ofn.lpstrFile,workpath,NULL,NULL);
		
	}

#endif

	BitmapInfo fbi;
	TCHAR path[MAX_PATH];

	if (workpath[0])
		strcpy(path,workpath);
	else
		strcpy(path,ip->GetMapDir(0));

	BMMAppendSlash(path);
	fbi.SetName(path);
	
	if (TheManager->SelectFileInput(&fbi,hWnd)) {

		//-- Check for Valid File

		int idx = TheManager->ioList.FindDeviceFromFilename(fbi.Name());
		if (idx == -1) {
			TCHAR text[MAX_PATH];
			wsprintf(text,GetString(IDS_BAD_FILETYPE),fbi.Name());
			TSTR tit = GetString(IDS_CLASS_DESC);
			MessageBox(hWnd,text,tit,MB_APPLMODAL | MB_ICONSTOP | MB_OK);
			return;
		}

		if (TheManager->ioList[idx].TestCapabilities(BMMIO_MULTIFRAME) ||
			TheManager->ioList[idx].TestCapabilities(BMMIO_IFL)) {
			TCHAR text[MAX_PATH];
			wsprintf(text,GetString(IDS_BAD_FILETYPE),fbi.Name());
			TSTR tit = GetString(IDS_CLASS_DESC);
			MessageBox(hWnd,text,tit,MB_APPLMODAL | MB_ICONSTOP | MB_OK);
			return;
		}

		if (!BuildFileList(hWnd,fbi.Name()) || table.Count() == 1) {
			if (table.Count())
				table.Delete(0,table.Count());
			TSTR text	= GetString(IDS_NOLISTCREATED);
			TSTR tit	= GetString(IDS_CLASS_DESC);
			MessageBox(hWnd,text,tit,MB_APPLMODAL | MB_ICONSTOP | MB_OK);
			return;
		}

		TCHAR title[MAX_PATH];
		wsprintf(title,_T(" %s "),root);
		SetDlgItemText(hWnd,IDC_FRAME,title);

		for (int i = 0; i < NUMCONTROLS; i++) 
			ToggleControl(hWnd,ControlArray[i],TRUE);
		
		int c = table.Count();
		InitSpinner(hWnd,IDC_START_ED,	IDC_START_SP,	0,0,c-1);
		InitSpinner(hWnd,IDC_END_ED,	IDC_END_SP,		c-1,0,c-1);
		InitSpinner(hWnd,IDC_NTH_ED,	IDC_NTH_SP,		1,1,c-1);
		InitSpinner(hWnd,IDC_MULT_ED,	IDC_MULT_SP,	1,1,9999);

		BMMSplitFilename(fbi.Name(),workpath,NULL,NULL);
		return;
	}

}

//-----------------------------------------------------------------------------
// #> IFLMgr::Edit()

void IFLMgr::Edit ( HWND hWnd ) {

	FilterList filterList;
	filterList.Append( GetString(IDS_IFL_FILES));
	filterList.Append( _T("*.ifl"));
	filterList.Append( GetString(IDS_ALL_FILES));
	filterList.Append( _T("*.*"));
	filterList.Append( _T(""));
	filterList.Append( _T(""));

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(OPENFILENAME));
	TCHAR filename[MAX_PATH];
	filename[0] = 0;

	ofn.lpstrFilter		= filterList;
	ofn.lpstrTitle		= GetString(IDS_EDIT_IFL);
	ofn.Flags			= OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.lStructSize		= sizeof(OPENFILENAME);
	ofn.hwndOwner		= hWnd;
	ofn.lpstrFile		= filename;
	ofn.nMaxFile		= MAX_PATH;

	if (workpath[0])
		ofn.lpstrInitialDir	= workpath;
	else
		ofn.lpstrInitialDir	= ip->GetMapDir(0);

	if (GetOpenFileName(&ofn)) {
		EditIFL( hWnd, ofn.lpstrFile );
		BMMSplitFilename(ofn.lpstrFile,workpath,NULL,NULL);
	}

}

//-----------------------------------------------------------------------------
// #> IFLMgr::ToggleControl()

void IFLMgr::ToggleControl( HWND hWnd, int control, BOOL state ) {
	HWND hDlg = GetDlgItem(hWnd,control);
	EnableWindow(hDlg,state);
}
	
//-----------------------------------------------------------------------------
// #> IFLMgr::EditIFL()

BOOL IFLMgr::EditIFL( HWND hWnd, TCHAR *filename ) {

	//-- Define Command Line ------------------------------

	// Use default editor
	if ((unsigned int)ShellExecute(NULL, "open", filename, NULL, NULL, SW_SHOWNORMAL) > 32) {
		return TRUE; 
		}

	// Default to notepad if no default viewer.
	TCHAR cmd[512];
	wsprintf(cmd,_T("notepad.exe \"%s\""),filename);

	//-- Startup Info Structure ---------------------------

	PROCESS_INFORMATION process;
	STARTUPINFO	si;
	BOOL res = FALSE;

	memset(&si,0,sizeof(STARTUPINFO));
	
	si.cb  			= sizeof(STARTUPINFO);
	si.dwFlags		= STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow	= SW_SHOWDEFAULT;
	si.hStdError	= NULL;

	//-- Start the Thing ----------------------------------

	res = CreateProcess(
		  (LPCTSTR)	NULL,						  //-- Pointer	to	name of executable module 
		  (LPTSTR)	cmd,  						  //-- Pointer	to	command line string
		  (LPSECURITY_ATTRIBUTES)	NULL,		  //-- Pointer	to	process security attributes 
		  (LPSECURITY_ATTRIBUTES)	NULL,		  //-- Pointer	to	thread security attributes	
		  FALSE,								  //-- Handle inheritance flag 
		  (DWORD)0,								  //-- Creation flags 
		  (LPVOID)NULL,							  //-- Pointer	to	new environment block 
		  (LPCTSTR)NULL,						  //-- Pointer	to	current directory	name 
		  (LPSTARTUPINFO)&si,					  //-- Pointer	to	STARTUPINFO	
		  (LPPROCESS_INFORMATION)&process		  //-- Pointer	to	PROCESS_INFORMATION	
	);		 
	
	if	(!res)
		return (FALSE);
	
	CloseHandle(process.hThread);
	return (TRUE);

}

//-----------------------------------------------------------------------------
// *> IsNumeric()

static BOOL IsNumeric( TCHAR *filename ) {
	TCHAR *p = filename;
	 while (*(p++)) {
		if (*p >= _T('0') && *p <= _T('9'))
			return TRUE;
		}
	return FALSE;
}

//-----------------------------------------------------------------------------
// #> IFLMgr::BuildFileList()

int	IFLMgr::BuildFileList(HWND hWnd, const TCHAR *masktext ) {

	TCHAR mask[MAX_PATH];
	strcpy(mask,masktext);

	if (table.Count())
		table.Delete(0,table.Count());

	//-- Check for Wild Cards

	TCHAR tpath[MAX_PATH],tfile[MAX_PATH],ext[_MAX_EXT];
	BMMSplitFilename(mask,tpath,tfile,ext);
	BMMAppendSlash(tpath);

	int i = 0;
	while (tfile[i++]) {
		if (tfile[i] <= _T('9') && tfile[i] >= _T('0'))
			tfile[i] = _T('?');
	}
	wsprintf(mask,_T("%s%s%s"),tpath,tfile,ext);
	
	if (!_tcsstr(mask,_T("*")) && !_tcsstr(mask,_T("?")))
		return 0;

	//-- Look for matches --------------------------------

	HANDLE findhandle;
	WIN32_FIND_DATA file;
	findhandle = FindFirstFile(mask,&file);
	FindClose(findhandle);

	if (findhandle == INVALID_HANDLE_VALUE)
		return 0;

	//-- Load and sort list of files ----------------------

	findhandle = FindFirstFile(mask,&file);
	do {
		if (!IsNumeric(file.cFileName))
			continue;
		table.Append(1,(SortTable *)(void *)&file.cFileName);
	} while (FindNextFile(findhandle,&file));

	if (table.Count() < 2)
		return 0;

	//-- Build Filename -----------------------------------

	ExtractRoot(table[0].file);
	wsprintf(iflfile,_T("%s%s.ifl"),tpath,root);

	return (table.Count());
	
}

//-----------------------------------------------------------------------------
// #> IFLMgr::ExtractRoot()

void IFLMgr::ExtractRoot( TCHAR *filename ) {
	TCHAR dr[_MAX_DRIVE],di[_MAX_DIR],fn[_MAX_FNAME],ex[_MAX_EXT];
	_splitpath(filename,dr,di,fn,ex);
	int i = 0;
	while (fn[i++]) {
		if (fn[i] >= _T('0') && fn[i] <= _T('9')) {
			fn[i]=0;
			break;
		}
	}
	_makepath(root,dr,di,fn,NULL);
	_tcslwr(root);
}

//-----------------------------------------------------------------------------
// #> IFLMgr::InitSpinner()

void IFLMgr::InitSpinner( HWND hWnd, int ed, int sp, int v, int minv, int maxv ) {
	ISpinnerControl	*blendspin = GetISpinner(GetDlgItem(hWnd,sp));
	if (blendspin) {
		blendspin->LinkToEdit(GetDlgItem(hWnd,ed),EDITTYPE_INT);
		blendspin->SetLimits(minv,maxv,FALSE);
		blendspin->SetValue(v,FALSE);
	}
}

//-----------------------------------------------------------------------------
// #> IFLMgr::DestroySpinner()

void IFLMgr::DestroySpinner( HWND hWnd, int sp ) {
	ISpinnerControl	*blendspin = GetISpinner(GetDlgItem(hWnd,sp));
	if (blendspin)
		ReleaseISpinner(blendspin);
}

//-----------------------------------------------------------------------------
// #> IFLMgr::GetSpinnerValue()

int IFLMgr::GetSpinnerValue( HWND hWnd, int sp ) {
	ISpinnerControl	*blendspin = GetISpinner(GetDlgItem(hWnd,sp));
	if (blendspin)
		return (blendspin->GetIVal());
	else
		return 0;
}

//-- EOF: iflmgr.cpp ----------------------------------------------------------
