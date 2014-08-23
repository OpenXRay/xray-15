//-----------------------------------------------------------------------------
// ------------------
// File ....: ifl.cpp
// ------------------
// Author...: Gus J Grubba
// Date ....: July 1995
// Descr....: IFL File I/O Module
//
// History .: Oct, 20 1995 - Started
//            
//-----------------------------------------------------------------------------
		
//-- Include files

#include <Max.h>
#include <bmmlib.h>
#include "ifl.h"
#include "iflrc.h"

//-----------------------------------------------------------------------------
//-- File Class

class File {
	public:
		FILE  *stream;
		File  ( const TCHAR *name, const TCHAR *mode) { stream = _tfopen(name,mode); }
		~File ( ) { Close(); }
		void Close() { if(stream) fclose(stream); stream = NULL; }
};

//-- Globals ------------------------------------------------------------------

HINSTANCE hInst = NULL;

typedef struct tag_FileList {
	TCHAR file[MAX_PATH];
} FileList;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-- DLL Declaration

BOOL WINAPI DllMain(HINSTANCE hDLLInst, DWORD fdwReason, LPVOID lpvReserved) {
	switch (fdwReason) {
		 case DLL_PROCESS_ATTACH:
				if (hInst)
					return(FALSE);
				hInst = hDLLInst;
				break;
		 case DLL_PROCESS_DETACH:
				hInst  = NULL;
				break;
		 case DLL_THREAD_ATTACH:
				break;
		 case DLL_THREAD_DETACH:
				break;
	}
	return TRUE;
}

//-- Helpers

//-----------------------------------------------------------------------------
// *> get_string()
//

TCHAR *get_string(int id) {
	static TCHAR buf[256];
	if (hInst)
		return LoadString(hInst, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

inline bool is_number(TCHAR c) {
	if (c >= '0' && c <= '9')
		return true;
	return false;
}

//-----------------------------------------------------------------------------
// *> extract_filename()
//
// GG: 09/14/99
//
// This is to work around image filenames with spaces. The original code would
// simply (and conveniently) sscanf the line looking for a string and an 
// integer. Having a space in the filename blows that off the water. 
//

static char *extract_filename(TCHAR *s, int *i) {
	static TCHAR line[_MAX_PATH];
	_tcscpy(line,s);
	TCHAR *ptr = _tcsrchr(line,'\n');
	if (ptr) *ptr = 0;
	int l = _tcslen(line);
	while (line[--l] == ' ');
	if (is_number(line[l])) {
		while (is_number(line[--l]) && l);
		if (i)
			sscanf(&line[l],"%d",i);
		line[l] = 0;
	} else {
		if (i)
			*i = 0;
	}
	return line;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// IFL Class Description

class IFLClassDesc:public ClassDesc {
	
	public:

		int             IsPublic     ( )                   { return 1;                }
		void           *Create       ( BOOL loading=FALSE) { return new BitmapIO_IFL; }
		const TCHAR    *ClassName    ( )                   { return get_string(IDS_IFL);     }
		SClass_ID       SuperClassID ( )                   { return BMM_IO_CLASS_ID;  }
		Class_ID        ClassID      ( )                   { return Class_ID(IFLCLASSID,0);    }
		const TCHAR    *Category     ( )                   { return get_string(IDS_BITMAP_IO); }

};

static IFLClassDesc IFLDesc;

//-----------------------------------------------------------------------------
// Interface

DLLEXPORT const TCHAR * LibDescription ( )  { 
	return get_string(IDS_LIBDESCRIPTION); 
}

DLLEXPORT int LibNumberClasses ( ) { 
	return 1; 
}

DLLEXPORT ClassDesc *LibClassDesc(int i) {
	switch(i) {
		case  0: return &IFLDesc; break;
		default: return 0;        break;
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
// *> AboutCtrlDlgProc()
//

INT_PTR CALLBACK AboutCtrlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {

	switch (message) {
		
		case WM_INITDIALOG: {
			  CenterWindow(hWnd,GetParent(hWnd));
			  return 1;
		}

		case WM_COMMAND:

			  switch (LOWORD(wParam)) {
				  
				  case IDOK:              
						 EndDialog(hWnd,1);
						 break;

				  case IDCANCEL:
						 EndDialog(hWnd,0);
						 break;
		
			  }
			  return 1;

	}
	
	return 0;

}

//-----------------------------------------------------------------------------
// *> ControlCtrlDlgProc()
//

INT_PTR CALLBACK ControlCtrlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
	static BitmapIO_IFL *bm = NULL;
	if (message == WM_INITDIALOG) 
		bm = (BitmapIO_IFL *)lParam;
	if (bm) 
		return (bm->Control(hWnd,message,wParam,lParam));
	else
		return(FALSE);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_IFL::BitmapIO_IFL()

BOOL BitmapIO_IFL::Control(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {

	switch (message) {
		
		case WM_INITDIALOG:
			CenterWindow(hWnd,GetParent(hWnd));
			SetDlgItemText(hWnd,IDC_EDITOR,data.editor);
			//ReadCfg();
			return 1;

		case WM_COMMAND:

			switch (LOWORD(wParam)) {
	
				case IDC_EDIT_EXEC:
					//EditIFL(hWnd,bi);
					break;
				  
				case IDOK:              
					GetDlgItemText(hWnd,IDC_EDITOR,data.editor,MAX_PATH);
					//WriteCfg();
					EndDialog(hWnd,1);
					break;

				case IDCANCEL:
					EndDialog(hWnd,0);
					break;
		
			}
			return 1;

	}
	
	return 0;

}

//-----------------------------------------------------------------------------
// #> BitmapIO_IFL::BitmapIO_IFL()

BitmapIO_IFL::BitmapIO_IFL  ( ) {
	edtProcess = NULL;
	_tcscpy(data.editor,_T("notepad.exe"));
}

BitmapIO_IFL::~BitmapIO_IFL ( ) {
//	if(edtProcess)
//		KillProcess();
//	edtProcess = NULL;
}

//-----------------------------------------------------------------------------
// #> BitmapIO_IFL::LongDesc()

const TCHAR *BitmapIO_IFL::LongDesc() {
	return get_string(IDS_IFL_FILE);
}
	
//-----------------------------------------------------------------------------
// #> BitmapIO_IFL::ShortDesc()

const TCHAR *BitmapIO_IFL::ShortDesc() {
	return get_string(IDS_IFL);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_IFL::LoadConfigure()

BOOL BitmapIO_IFL::LoadConfigure ( void *ptr ) {
	return 1;
}

//-----------------------------------------------------------------------------
// #> BitmapIO_IFL::SaveConfigure()

BOOL BitmapIO_IFL::SaveConfigure ( void *ptr ) {
	return 1;
}

//-----------------------------------------------------------------------------
// #> BitmapIO_IFL::ShowAbout()

void BitmapIO_IFL::ShowAbout(HWND hWnd) {
	DialogBoxParam(
		hInst,
		MAKEINTRESOURCE(IDD_IFL_ABOUT),
		hWnd,
		(DLGPROC)AboutCtrlDlgProc,
		(LPARAM)0);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_IFL::Control()

BOOL BitmapIO_IFL::ShowControl(HWND hWnd, DWORD flag ) {
	if (storage)
		EditIFL(hWnd,&storage->bi);
	/*
	//InitCommonControls();
	InitCustomControls(hInst);
	DialogBoxParam(
		hInst,
		MAKEINTRESOURCE(IDD_IFL_CONTROL),
		hWnd,
		(DLGPROC)ControlCtrlDlgProc,
		(LPARAM)this);
	*/
	return (FALSE);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_IFL::GetImageInfoDlg()

#if 0
BMMRES BitmapIO_IFL::GetImageInfoDlg ( HWND hWnd, BitmapInfo *fbi, const TCHAR *filename ) {
	if (filename)
		fbi->SetName(filename);
	EditIFL(hWnd,fbi);
	return BMMRES_SUCCESS;
}
#endif

//-----------------------------------------------------------------------------
// #> BitmapIO_IFL::GetImageInfo()

BMMRES BitmapIO_IFL::GetImageInfo ( BitmapInfo *fbi ) {

	//---------------------------------------------------------------
	//-- Check for File Enumeration
	//
	// GG: 09/21/99 Added to speed up the process of file enumeration

	if (fbi->TestCustomFlags(BMM_CUSTOM_IFLENUMFILES)) {
		File ifile(fbi->Name(),_T("rt"));
		if (!ifile.stream)
			return BMMRES_FILENOTFOUND;
		Tab<FileList> list;
		char	line[MAX_PATH + MAX_PATH];   
		char*	filename;
		while (fgets(line,MAX_PATH,ifile.stream)) {
			int i = FirstNonBlank(line);
			if ( i != -1) {
				if (line[i] != ';' && line[i] != '#') {
					filename = extract_filename(line,NULL);
					FileList l;
					
					//-- Resolve Filename

					TCHAR tfile[MAX_PATH];
					TCHAR textn[MAX_PATH];
					TCHAR tpath[MAX_PATH];
					TCHAR ipath[MAX_PATH];

					//-- Get IFL file's path

					BMMSplitFilename(fbi->Name(),tpath,NULL,NULL);
					BMMAppendSlash(tpath);
					BMMSplitFilename(filename,ipath,tfile,textn);

					//-- Decide which path to use

					if (ipath[0])
						wsprintf(l.file,_T("%s%s%s"),ipath,tfile,textn);
					else
						wsprintf(l.file,_T("%s%s%s"),tpath,tfile,textn);

					list.Append(1,&l);

				}
			}
		}

		if (list.Count()) {
			DWORD size = 0;
			for (int i = 0; i < list.Count(); i++)
				size += _tcslen(list[i].file) + 1;
			size+=2;
			if (fbi->AllocPiData(size)) {
				TCHAR *ptr = (TCHAR *)fbi->GetPiData();
				memset(ptr,0,size);
				if (ptr) {
					for (i = 0; i < list.Count(); i++) {
						_tcscpy(ptr,list[i].file);
						ptr += _tcslen(ptr) + 1;
					}
					return BMMRES_SUCCESS;
				}
			}
		}

		return BMMRES_FILENOTFOUND;

	}

	//---------------------------------------------------------------
	//-- Define Number of Frames
	
	TSTR wname = fbi->Name();
	int frameCount = NumberOfFrames(wname);

	if (!frameCount)
		return (ProcessImageIOError(fbi,get_string(IDS_EMPTY_FILE)));
	
	//-- Get First Image from List
	
	TCHAR filenameOut[MAX_PATH];
	BitmapInfo tbi;
	tbi.SetName(fbi->Name());
	BMMRES res = GetImageName(&tbi,filenameOut);
	if (res != BMMRES_SUCCESS)
		return (res);
	res = TheManager->GetImageInfo(&tbi,filenameOut);
	if (res != BMMRES_SUCCESS)
		return (res);
	
	//-- Set Image Info
	
	fbi->SetWidth(tbi.Width());
	fbi->SetHeight(tbi.Height());
	//fbi->SetGamma(tbi.Gamma());
	fbi->SetAspect(tbi.Aspect());
	fbi->SetType(tbi.Type());
	fbi->SetFirstFrame(0);
	fbi->SetLastFrame(frameCount-1);

	return BMMRES_SUCCESS;

}

//-----------------------------------------------------------------------------
// #> BitmapIO_IFL::Load()
//
//    We don't load anything...
//

BitmapStorage *BitmapIO_IFL::Load(BitmapInfo *fbi, Bitmap *map, BMMRES *status ) {
	*status = ProcessImageIOError(fbi,BMMRES_INTERNALERROR);
	return NULL;
}

//-----------------------------------------------------------------------------
// #> BitmapIO_IFL::GetImageName()
//

BMMRES BitmapIO_IFL::GetImageName(BitmapInfo *fbi, TCHAR *filename) {

	//-- Define Number of Frames
	
	TSTR wname = fbi->Name();
	int frameCount = NumberOfFrames(wname);
	
	if (!frameCount) {
	
		//-- Check for silent mode
		//-- Display Error Dialog Box
		//-- Log Error
	
		return(BMMRES_ERRORTAKENCARE);
	}
	
	//-- Handle Given Frame Number
	
	int frame = fbi->CurrentFrame();
	
	if (frame >= frameCount) {
		int dif = frame %= frameCount;
		if (fbi->SequenceOutBound() == BMM_SEQ_WRAP) {
			frame = dif;
		} else {
			//-- Log Entry (GG 01/15/98)
			TheManager->Max()->Log()->LogEntry(SYSLOG_ERROR,NO_DIALOG,NULL,get_string(IDS_IFL_IFLREADERROR),fbi->Name());
			return(BMMRES_ERRORTAKENCARE);
		}        
	}
		  
	//-- Update Number of Frames

	fbi->SetFirstFrame(0);
	fbi->SetLastFrame(frameCount - 1);
	
	//-- Get file from IFL file
	
	GetIFLFile(fbi->Name(), frame, filename);

	//-- Check for Recoursive Files (GG 01/15/98)

	const TCHAR *thisfile = fbi->Name();
	int l0 = _tcslen(filename);
	int l1 = _tcslen(thisfile);
	while (--l0 && --l1) {
		if (filename[l0] == _T('\\') || thisfile[l1] == _T('\\') ||
			filename[l0] == _T(':')  || thisfile[l1] == _T(':'))
			break;
		if (filename[l0] != thisfile[l1])
			return (BMMRES_SUCCESS);
	}

	//-- Log Entry 

	TheManager->Max()->Log()->LogEntry(SYSLOG_ERROR,NO_DIALOG,NULL,get_string(IDS_IFL_RECOURSEERROR),thisfile);
	
	return(BMMRES_ERRORTAKENCARE);

}

//-----------------------------------------------------------------------------
// #> BitmapIO_IFL::OpenOutput()
//
//    Not supported
//

BMMRES BitmapIO_IFL::OpenOutput(BitmapInfo *fbi, Bitmap *map) {
	return BMMRES_INTERNALERROR;
}

//-----------------------------------------------------------------------------
// #> BitmapIO_IFL::Write()
//
//    Not supported
//

BMMRES BitmapIO_IFL::Write(int frame) {
	return BMMRES_INTERNALERROR;
}

//-----------------------------------------------------------------------------
// #> BitmapIO_IFL::Close()
//

int  BitmapIO_IFL::Close( int flag ) {

	return 1;

}

//-----------------------------------------------------------------------------
// #> BitmapIO_IFL::GetIFLFile()
//

void BitmapIO_IFL::GetIFLFile(const TCHAR *file, int frame, TCHAR *filenameOut) {

	filenameOut[0] = 0;

	//-- Open IFL file -----------------------------------
	
	File ifile(file,_T("rt"));
	
	if (!ifile.stream)
		return;

	//-- Skip frame frames -------------------------------
	//
	//   ";" or "#" is used as comments and if found as 
	//   the first character won't be counted)

	int		nframes = 0;
	char	line[MAX_PATH + MAX_PATH];   
	char*	filename;
	int		loop;
	
	while (fgets(line,MAX_PATH,ifile.stream)) {
		int i = FirstNonBlank(line);
		if ( i != -1) {
			loop = 0;
			if (line[i] != ';' && line[i] != '#') {
				filename = extract_filename(line,&loop);
				if (loop)
					nframes+=loop;
				else   
					nframes++;
			}
		}
		if (nframes > frame)
			break;
	}

	//-- Resolve Filename
	
	TCHAR tfile[MAX_PATH];
	TCHAR textn[MAX_PATH];
	TCHAR tpath[MAX_PATH];
	TCHAR ipath[MAX_PATH];

	//-- Get IFL file's path

	BMMSplitFilename(file,tpath,NULL,NULL);
	BMMAppendSlash(tpath);
	
	BMMSplitFilename(filename,ipath,tfile,textn);

	//-- Decide which path to use

	if (ipath[0])
		wsprintf(filenameOut,_T("%s%s%s"),ipath,tfile,textn);
	else
		wsprintf(filenameOut,_T("%s%s%s"),tpath,tfile,textn);

}

//-----------------------------------------------------------------------------
// #> BitmapIO_IFL::FirstNonBlank()
//
// Find the first non blank character. If the line is empty, this function will
// return a -1.
//

int BitmapIO_IFL::FirstNonBlank(TCHAR *s) {
	int i=0;
	while (!_tcscmp(&s[i],_T(" "))) i++;
	if (!_tcscmp(&s[i],_T("\n")) || !_tcscmp(&s[i],_T("\r")))
		return(-1);
	else   
		return (i);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_IFL::NumberOfFrames()
//
// Opens an IFL file and count number of frames
//

int BitmapIO_IFL::NumberOfFrames(TCHAR *file) {

	int frames = 0;

	//-- Open IFL file -----------------------------------
	
	File ifile(file,_T("rt"));
	
	if (!ifile.stream)
		return 0;

	char line[MAX_PATH + MAX_PATH];   
	int  loop;
	
	//-- Count number of frames --------------------------
	//
	//   ";" or "#" is used as comments and if found as 
	//   the first character won't be counted)

	while (fgets(line,MAX_PATH,ifile.stream)) {
		int i = FirstNonBlank(line);
		if ( i != -1) {
			loop = 0;
			if (line[i] != ';' && line[i] != '#') {
				extract_filename(line,&loop);
				if (loop)
					frames+=loop;
				else   
					frames++;
			}
		}
	}

	return (frames);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_IFL::KillProcess()
//

void BitmapIO_IFL::KillProcess() {
	DWORD exitCode;
	if(edtProcess) {
		GetExitCodeProcess(edtProcess, &exitCode);
		if(exitCode == STILL_ACTIVE) {
			TerminateProcess(edtProcess, 0);
			CloseHandle(edtProcess);
		}
	}
}

//-----------------------------------------------------------------------------
// #> BitmapIO_IFL::EditIFL()
//

BOOL BitmapIO_IFL::EditIFL( HWND hWnd, BitmapInfo *lbi ) {

	//-- Define Command Line Command -----------------------

	if (!BMMGetFullFilename(lbi))
		return FALSE;

	TCHAR cmd[512];
	wsprintf(cmd,_T("%s \"%s\""),data.editor,lbi->Name());

	//-- Startup Info	Structure --------------------------

	PROCESS_INFORMATION process;
	STARTUPINFO	si;
	BOOL res = FALSE;

	memset(&si,0,sizeof(STARTUPINFO));
	
	si.cb  			= sizeof(STARTUPINFO);
	si.dwFlags		= STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow	= SW_SHOWDEFAULT;
	si.hStdError	= NULL;

	if(edtProcess)
		KillProcess();
	
	edtProcess = NULL;
	
	//-- Start the	Thing	---------------------------------

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
	edtProcess = process.hProcess;

	return (TRUE);

}

//-----------------------------------------------------------------------------
// #> BitmapIO_IFL::GetCfgFilename()
//

void BitmapIO_IFL::GetCfgFilename( TCHAR *filename ) {
	_tcscpy(filename,TheManager->GetDir(APP_PLUGCFG_DIR));
	int len = _tcslen(filename);
	if (len) {
		if (_tcscmp(&filename[len-1],_T("\\")))
			_tcscat(filename,_T("\\"));
		}   
	_tcscat(filename,IFLCONFIGNAME);   
}

//-----------------------------------------------------------------------------
// #> BitmapIO_IFL::ReadCfg()
//

BOOL BitmapIO_IFL::ReadCfg() {
	
	TCHAR filename[MAX_PATH];
	GetCfgFilename(filename);
	IFLDATA tdata;

	//-- Open Configuration File
	
	File file(filename, _T("rb"));
	
	if (!file.stream)
		return (FALSE);
	
	if (fread(&tdata,1,sizeof(IFLDATA),file.stream) != sizeof(IFLDATA))
		return (FALSE);

	if (tdata.version != IFLVERSION)
		return (FALSE);


	data = tdata;
	return(TRUE);
	
}
	
//-----------------------------------------------------------------------------
// #> BitmapIO_IFL::WriteCfg()
//

void BitmapIO_IFL::WriteCfg() {
	TCHAR filename[MAX_PATH];
	GetCfgFilename(filename);
	File file(filename, _T("wb"));
 	if (file.stream) {
 		fwrite(&data,1,sizeof(IFLDATA),file.stream);
 	}
}

//-- EOF: ifl.cpp -------------------------------------------------------------
