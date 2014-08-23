//-----------------------------------------------------------------------------
// ------------------
// File ....: wsd.cpp
// ------------------
// Author...: Gus J Grubba
// Date ....: September 1995
// Descr....: WSD Image I/O Module
//
// History .: Sep, 20 1995 - Started (GG)
//            Jan, 17 1997 - Version 2 - Chroma Filtering Added (GG)
//            
//-----------------------------------------------------------------------------
		
//-- Include files

#include <Max.h>
#include <bmmlib.h>
#include <gcommlib.h>
#include "wsd.h"
#include "wsdrc.h"

//-- Handy macros

#define limit(x) {                      \
    if(x > 0xFFFFFF) x = 0xFFFFFF;      \
    if(x <=  0xFFFF) x = 0;             \
    x  &=  0xFF0000;                    \
}

//-- Globals ------------------------------------------------------------------

HINSTANCE hInst = NULL;
TCPcomm  *tcp   = NULL; 
char  szTemp[80];
static BOOL controlsInit = FALSE;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-- DLL Declaration

BOOL WINAPI DllMain(HINSTANCE hDLLInst, DWORD fdwReason, LPVOID lpvReserved) {
	switch (fdwReason) {
		case DLL_PROCESS_ATTACH:
			if (hInst)
				return(FALSE);
				hInst = hDLLInst;
			if ( !controlsInit ) {
				controlsInit = TRUE;
				InitCustomControls(hInst);
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
	return TRUE;
}

//-----------------------------------------------------------------------------
// *> GetString()
//

TCHAR *GetString(int id) {
	static TCHAR buf[256];
	if (hInst)
		return LoadString(hInst, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

//-----------------------------------------------------------------------------
// *> psrand()
//

DWORD randomseed = 1234;

int psrand() {
	 randomseed = randomseed * 1103515245 + 12345;
	 return (int)((randomseed>>16)&0x7FFF);
}

//-----------------------------------------------------------------------------
// *> ErrorHandler()
//
//  Error Handler for the TCP/IP Library
//
//	 ERR_FATAL
//	 ERR_WARN 
//	 ERR_INFO 
//	 ERR_DEBUG
//

void WINAPI ErrorHandler( int ErrorCode, const TCHAR *ErrorMessage) {
	
	int LogType = 0;

	switch (ErrorCode) {
		case ERR_FATAL:
			LogType = SYSLOG_ERROR;
			break;
		case ERR_WARN:
			LogType = SYSLOG_WARN;
			break;
		case ERR_INFO:
			LogType = SYSLOG_INFO;
			break;
		case ERR_DEBUG:
			LogType = SYSLOG_DEBUG;
			break;
	}

	if (LogType) {
		TCHAR txt[MAX_PATH];
		wsprintf(txt,_T("TCP/IP - %s"),ErrorMessage);
		TheManager->Max()->Log()->LogEntry(LogType,NO_DIALOG,NULL,txt);
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// WSD Class Description

class WSDClassDesc:public ClassDesc {
	
	public:

		int             IsPublic     ( )                   { return 1;                }
		void           *Create       ( BOOL loading=FALSE) { return new BitmapIO_WSD; }
		const TCHAR    *ClassName    ( )                   { return GetString(IDS_WSD);     }
		SClass_ID       SuperClassID ( )                   { return BMM_IO_CLASS_ID;  }
		Class_ID        ClassID      ( )                   { return Class_ID(WSDCLASSID,0); }
		const TCHAR    *Category     ( )                   { return GetString(IDS_BITMAP_IO); }

};

static WSDClassDesc WSDDesc;

//-----------------------------------------------------------------------------
// Interface

DLLEXPORT const TCHAR * LibDescription ( )  { 
	return GetString(IDS_LIBDSCRIPTION); 
}

DLLEXPORT int LibNumberClasses ( ) { 
	return 1; 
}

DLLEXPORT ClassDesc *LibClassDesc(int i) {
	switch(i) {
		case  0: return &WSDDesc; break;
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
// *> GetCfgFilename()
//

void BitmapIO_WSD::GetCfgFilename( TCHAR *filename ) {
	_tcscpy(filename,TheManager->GetDir(APP_PLUGCFG_DIR));
	int len = _tcslen(filename);
	if (len) {
		if (_tcscmp(&filename[len-1],_T("\\")))
			_tcscat(filename,_T("\\"));
	}   
	_tcscat(filename,WSDCONFIGNAME);   
}

//-----------------------------------------------------------------------------
// *> ReadCfg()
//

int BitmapIO_WSD::ReadCfg() {
	
	TCHAR filename[MAX_PATH];
	TCHAR system[64];
	
	GetCfgFilename(filename);
	
	if (!BMMIsFile(filename)) {
		//-- Log Entry (GG 02/23/97)
		TheManager->Max()->Log()->LogEntry(SYSLOG_WARN,NO_DIALOG,NULL,GetString(IDS_LOG_NOCFG));
		_tcscpy(system,WSDDEFSYS);
		return (FALSE);
	}
	
	TCHAR temp[64];
	wsprintf(temp,_T("%d"),maxframes);

	GetPrivateProfileString(WSDSECTION,WSDHOSTKEY,WSDDEFAULT,hostname,MAX_PATH,filename);
	GetPrivateProfileString(WSDSECTION,WSDSYSKEY,WSDDEFSYS,system,64,filename);
	GetPrivateProfileString(WSDSECTION,WSDMXFRAMEKEY,temp,temp,64,filename);
	maxframes = atoi(temp);
	
	if (!_tcscmp(system,WSDDEFSYS)) {
		ntsc   = TRUE;
		height = 486;
	} else {
		ntsc   = FALSE;
		height = 576;
	}
	
	return (TRUE);

}

//-----------------------------------------------------------------------------
// *> WriteCfg()
//

void BitmapIO_WSD::WriteCfg() {
	
	TCHAR filename[MAX_PATH];
	TCHAR system[64];
	
	if (ntsc)
		_tcscpy(system,WSDDEFSYS);
	else   
		_tcscpy(system,_T("pal"));
	
	GetCfgFilename(filename);
	WritePrivateProfileString(WSDSECTION,WSDHOSTKEY,hostname,filename);
	WritePrivateProfileString(WSDSECTION,WSDSYSKEY,system,filename);

	TCHAR temp[64];
	wsprintf(temp,_T("%d"),maxframes);
	WritePrivateProfileString(WSDSECTION,WSDMXFRAMEKEY,temp,filename);
}

//-----------------------------------------------------------------------------
// *> Setup()
//

BOOL BitmapIO_WSD::Setup(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {

	switch (message) {
		
		case WM_INITDIALOG: {
			  
			CenterWindow(hWnd,GetParent(hWnd));
			SetCursor(LoadCursor(NULL,IDC_ARROW));

			//-- Set Hostname Text -----------------------

			SetDlgItemText(hWnd,IDC_HOSTNAME,hostname);

			//-- Handle System Type ----------------------

			CheckRadioButton(hWnd,IDC_NTSC,IDC_PAL,ntsc?IDC_NTSC:IDC_PAL);

			ISpinnerControl	*mxframespin = GetISpinner(GetDlgItem(hWnd,IDC_MAX_FRAMES_SP));
			if (mxframespin) {
				mxframespin->LinkToEdit(GetDlgItem(hWnd,IDC_MAX_FRAMES_ED),EDITTYPE_INT);
				mxframespin->SetLimits(1,99999,FALSE);
				mxframespin->SetValue(maxframes,FALSE);
			}

			return 1;
		}

		case WM_COMMAND:

			switch (LOWORD(wParam)) {
				  
				//-- Changed Accepted ---------------------
				  
				case IDOK:
				  
					//-- Handle System Type --------------

					ntsc = IsDlgButtonChecked(hWnd,IDC_NTSC);

					//-- Handle Host Name ----------------

					GetDlgItemText(hWnd,IDC_HOSTNAME,hostname,MAX_PATH);
				  
					{
						ISpinnerControl	*mxframespin = GetISpinner(GetDlgItem(hWnd,IDC_MAX_FRAMES_SP));
						if (mxframespin)
							maxframes = mxframespin->GetIVal();
					}

					//-- Update Configuration File -------

					WriteCfg();
				  
					EndDialog(hWnd,1);
					break;

				//-- Changed Aborted ----------------------
				  
				case IDCANCEL:
					EndDialog(hWnd,0);
					break;
		
			}
			return 1;

		case WM_DESTROY: {
				ISpinnerControl	*mxframespin = GetISpinner(GetDlgItem(hWnd,IDC_MAX_FRAMES_SP));
				if (mxframespin)
					ReleaseISpinner(mxframespin);
			}
			break;

	}
	
	return 0;

}

//-----------------------------------------------------------------------------
// *> SetupCtrlDlgProc()
//

INT_PTR CALLBACK SetupCtrlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
	static BitmapIO_WSD *bio = NULL;
	if (message == WM_INITDIALOG) 
		bio = (BitmapIO_WSD *)lParam;
	if (bio) 
		return (bio->Setup(hWnd,message,wParam,lParam));
	else
		return(FALSE);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_WSD::ShowSetup()

BOOL BitmapIO_WSD::ShowSetup(HWND hWnd) {
	return (
		DialogBoxParam(
			hInst,
			MAKEINTRESOURCE(IDD_WSD_SETUP),
			hWnd,
			(DLGPROC)SetupCtrlDlgProc,
			(LPARAM)this)
	);        
}

//-----------------------------------------------------------------------------
// #> BitmapIO_WSD::Where()

int BitmapIO_WSD::Where() {
	tcp->Send(&ci,"w\n",2);
	tcp->Receive(&ci,szTemp,80,0.0f);
	tcp->Receive(&ci,szTemp,80,0.333f);
	char *ptr = strstr(szTemp,"#");
	if (ptr)
		return(atoi(++ptr));
	else
		return (0);
}

//-----------------------------------------------------------------------------
// #> ControlConnect()
//

BOOL BitmapIO_WSD::ControlConnect(HWND hWnd) {

	SetCursor(LoadCursor(NULL,IDC_WAIT));

	ci.Reset();
	ci.SetServerName(hostname);

	if (tcp->rlogin(&ci) != GCRES_SUCCESS) {
		
		//-- Log Entry (GG 02/23/97)

		TCHAR title[MAX_PATH];
		_tcscpy(title,GetString(IDS_TITLE_LOG));

		TheManager->Max()->Log()->LogEntry(SYSLOG_ERROR,DISPLAY_DIALOG,
			title,
			GetString(IDS_LOG_NOHOST),hostname);
		
		HWND hDlg = GetDlgItem(hWnd,IDC_PLAY);
		EnableWindow(hDlg,FALSE);
		hDlg = GetDlgItem(hWnd,IDC_REW);
		EnableWindow(hDlg,FALSE);
		hDlg = GetDlgItem(hWnd,IDC_STOP);
		EnableWindow(hDlg,FALSE);
		SetCursor(LoadCursor(NULL,IDC_ARROW));
		return FALSE;
	} else {

		//-- Log Entry (GG 02/23/97)

		TheManager->Max()->Log()->LogEntry(SYSLOG_DEBUG,NO_DIALOG,
			NULL, GetString(IDS_LOG_CONNECTED),hostname);
		
		HWND hDlg = GetDlgItem(hWnd,IDC_PLAY);
		EnableWindow(hDlg,TRUE);
		hDlg = GetDlgItem(hWnd,IDC_REW);
		EnableWindow(hDlg,TRUE);
		hDlg = GetDlgItem(hWnd,IDC_STOP);
		EnableWindow(hDlg,TRUE);
		SetCursor(LoadCursor(NULL,IDC_ARROW));
		return TRUE;
	}

}

//-----------------------------------------------------------------------------
// #> ControlCtrlDlgProc()
//

BOOL BitmapIO_WSD::Control(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {

	static connected = FALSE;
	
	switch (message) {
		
		//-- Initialization -------------------------------
		
		case WM_INITDIALOG: {
			  
			  CenterWindow(hWnd,GetParent(hWnd));
			  SetCursor(LoadCursor(NULL,IDC_ARROW));
			  
			  //-- Read Configuration ----------------------
			  
			  if (!ReadCfg()) {
				  if (!ShowSetup(hWnd)) {
					  EndDialog(hWnd,0);
					  return 1;
				  }
			  }
			  
			  //-- Attempt to Connect ----------------------
    
			  tcp = (TCPcomm *)gcommCreate(gcTCP);
			  if (!tcp) {
				  EndDialog(hWnd,0);
				  return 1;
			  }

			  //-- Set Error handlers ----------------------
	
			  tcp->SetSilentMode(TRUE);
			  tcp->RegisterErrorHandler((PERROR_HANDLER)ErrorHandler);
	
			  //-- Init ------------------------------------
	
			  if (!tcp->Init(hWnd)) {
				  EndDialog(hWnd,0);
				  return 1;
			  }
			  
			  connected = ControlConnect(hWnd);
	
			  //-- Handle Starting Frame -------------------
			  
			  sprintf(szTemp,"%d",data.startframe);
			  SetDlgItemText(hWnd,IDC_STARTFRAME,szTemp);

			  //-- Handle Chroma Filtering

			  CheckDlgButton(hWnd,IDC_CHROMA,data.chromadither);

			  return 1;
		}

		case WM_COMMAND:

			  switch (LOWORD(wParam)) {
				  
				  case IDC_PLAY:
						 tcp->Send(&ci,"play\n",5);
						 tcp->Receive(&ci,szTemp,80,0.0f);
						 break;

				  case IDC_REW:              
						 tcp->Send(&ci,"g 0\n",4);
						 tcp->Receive(&ci,szTemp,80,0.0f);
						 break;

				  case IDC_STOP:              
						 tcp->Send(&ci,"stop\n",5);
						 tcp->Receive(&ci,szTemp,80,0.0f);
						 break;

				  case IDC_CURRENT:              
						 data.startframe = Where();
						 sprintf(szTemp,"%d",data.startframe);
						 SetDlgItemText(hWnd,IDC_STARTFRAME,szTemp);
						 break;

				  case IDC_SETUP:              
						 if (ShowSetup(hWnd)) {
							 if (connected) {
								 tcp->Disconnect(&ci);
								 tcp->Close();
							 }
							 connected = ControlConnect(hWnd);
						 }
						 break;

				  case IDOK:
						 
						 //-- Get Starting Frame
						 
						 char buf[64];
						 GetDlgItemText(hWnd,IDC_STARTFRAME,buf,64);
						 data.startframe = atoi(buf);
						 data.chromadither = IsDlgButtonChecked(hWnd,IDC_CHROMA);
						 EndDialog(hWnd,1);
						 break;

				  case IDCANCEL:
						 EndDialog(hWnd,0);
						 break;
		
			  }
			  return 1;

		case WM_DESTROY:
			  if (connected) {
				  tcp->Disconnect(&ci);
				  tcp->Close();
			  }
			  connected = FALSE;  
			  delete tcp;
			  tcp = NULL;
			  return 1;

	}
	
	return 0;

}

//-----------------------------------------------------------------------------
// *> ControlDlgProc()
//

INT_PTR CALLBACK ControlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
	static BitmapIO_WSD *bio = NULL;
	if (message == WM_INITDIALOG) 
		bio = (BitmapIO_WSD *)lParam;
	if (bio) 
		return (bio->Control(hWnd,message,wParam,lParam));
	else
		return(FALSE);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_WSD::ShowControl()

BOOL BitmapIO_WSD::ShowControl(HWND hWnd, DWORD flag) {
	return (
		DialogBoxParam(
		hInst,
		MAKEINTRESOURCE(IDD_WSD_CONTROL),
		hWnd,
		(DLGPROC)ControlDlgProc,
		(LPARAM)this)
	);
}

//-----------------------------------------------------------------------------
// *> AboutCtrlDlgProc()
//

BOOL CALLBACK AboutCtrlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
	switch (message) {
		case WM_INITDIALOG: {
			  CenterWindow(hWnd,GetParent(hWnd));
			  SetCursor(LoadCursor(NULL,IDC_ARROW));
			  return 1;
		}
		case WM_COMMAND:
			  switch (LOWORD(wParam)) {
				  case IDOK:              
				  case IDCANCEL:
						 EndDialog(hWnd,1);
						 break;
			  }
			  return 1;
	}
	return 0;
}

//-----------------------------------------------------------------------------
// #> BitmapIO_WSD::BitmapIO_WSD()

BitmapIO_WSD::BitmapIO_WSD  ( ) { 
	width   = 720;
	height  = 486;
	ntsc    = TRUE;
	rgbbuf  = NULL;
	yuvbuf  = NULL;
	line    = NULL;
	
	maxframes = MAX_FRAMES;

	data.version    	= WSDVERSION;
	data.startframe 	= 0;
	data.chromadither	= TRUE;

	_tcscpy(hostname,WSDDEFAULT);
}

BitmapIO_WSD::~BitmapIO_WSD ( ) { }

//-----------------------------------------------------------------------------
// #> BitmapIO_WSD::LongDesc()

const TCHAR *BitmapIO_WSD::LongDesc() {
	return GetString(IDS_WSD_FILE);
}
	
//-----------------------------------------------------------------------------
// #> BitmapIO_WSD::ShortDesc()

const TCHAR *BitmapIO_WSD::ShortDesc() {
	return GetString(IDS_ACCOM);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_WSD::LoadConfigure()

BOOL BitmapIO_WSD::LoadConfigure ( void *ptr ) {
	WSDDATA *buf = (WSDDATA *)ptr;
	if (buf->version == WSDVERSION) {
		memcpy((void *)&data,ptr,sizeof(WSDDATA));
		height	= data.height;
		ntsc	= data.ntsc;
		strcpy(hostname,data.hostname);
		return (TRUE);
	} else
		return (FALSE);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_WSD::SaveConfigure()

BOOL BitmapIO_WSD::SaveConfigure ( void *ptr ) {
	if (ptr) {
		data.height	= height;
		data.ntsc	= ntsc;
		strcpy(data.hostname,hostname);
		memcpy(ptr,(void *)&data,sizeof(WSDDATA));
		return (TRUE);
	} else
		return (FALSE);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_WSD::EvaluateConfigure()

DWORD BitmapIO_WSD::EvaluateConfigure ( ) {
	 return (sizeof(WSDDATA));
}

//-----------------------------------------------------------------------------
// #> BitmapIO_WSD::ShowAbout()

void BitmapIO_WSD::ShowAbout(HWND hWnd) {
	DialogBoxParam(
		hInst,
		MAKEINTRESOURCE(IDD_WSD_ABOUT),
		hWnd,
		(DLGPROC)AboutCtrlDlgProc,
		(LPARAM)0);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_WSD::GetImageInfo()

BMMRES BitmapIO_WSD::GetImageInfo ( BitmapInfo *fbi ) {

	//-- Note that I'm hardcoding some bogus length. Eventually, I will get
	//   the number of frames straight from the Accom (disk size).

	//-- Update Bitmap Info ------------------------------
	
	fbi->SetWidth(720);
	fbi->SetHeight(486);
	fbi->SetType(BMM_TRUE_24);
	//fbi->SetGamma(1.0f);
	fbi->SetAspect(1.3333333f / 720.0f / 486.0f);
	fbi->SetFirstFrame(0);
	fbi->SetLastFrame(999999);

	return BMMRES_SUCCESS;

}

//-----------------------------------------------------------------------------
// #> BitmapIO_WSD::Load()

BitmapStorage *BitmapIO_WSD::Load(BitmapInfo *fbi, Bitmap *map, BMMRES *status) {

	BitmapStorage *s = NULL;

	int   y;
	GCRES res;

	//-- Initialize Status Optimistically

	*status = BMMRES_SUCCESS;

	//-- Make sure nothing weird is going on

	if(openMode != BMM_NOT_OPEN) {
		*status = ProcessImageIOError(fbi,BMMRES_INTERNALERROR);
		return NULL;
	}

	//-- Allocate receive Buffer
	
	yuvbuf = (BYTE *)LocalAlloc(LPTR, width * 2);

	if (!yuvbuf) {
		memory_error:
		*status = ProcessImageIOError(fbi,BMMRES_MEMORYERROR);
		CleanUp();
		return NULL;
	}

	line = (BMM_Color_64 *)LocalAlloc(LPTR, width * sizeof(BMM_Color_64));

	if (!line)
		goto memory_error;

	//-- Update Bitmap Info ------------------------------
	
	fbi->SetWidth(  width  );
	fbi->SetHeight( height );
	fbi->SetAspect( 1.3333333f / (float)width / (float)height);
	fbi->SetType(BMM_YUV_422);
	fbi->ResetFlags(MAP_HAS_ALPHA);
	fbi->SetFirstFrame(0);
	fbi->SetLastFrame(999999);
	
	//-- Create Image Storage ---------------------------- 
	
	s = BMMCreateStorage(map->Manager(),BMM_TRUE_32);

	if (!s) {
		storage_error:
		*status = ProcessImageIOError(fbi,BMMRES_INTERNALERROR);
		goto bail_out;
	}
	
	if (s->Allocate(fbi,map->Manager(),BMM_OPEN_R)==0)
		goto storage_error;

	//-- Init login

	ReadCfg();
	
	tcp = (TCPcomm *)gcommCreate(gcTCP);

	if (!tcp) {
		*status = ProcessImageIOError(fbi,GetString(IDS_ERR_TCPERROR));
		goto bail_out;
	}

	//-- Set Error handlers ----------------------
	
	tcp->SetSilentMode(TRUE);
	tcp->RegisterErrorHandler((PERROR_HANDLER)ErrorHandler);
	
	//-- Init ------------------------------------
	
	if (!tcp->Init(fbi->GetUpdateWindow())) {
		*status = ProcessImageIOError(fbi,GetString(IDS_ERR_TCPINITERROR));
		goto bail_out;
	}
		
	ci.Reset();
	ci.SetServerName(hostname);
	*status = BMMRES_ERRORTAKENCARE;

	//-- Login
	
	if (tcp->rlogin(&ci) != GCRES_SUCCESS) {

		//-- Log Entry (GG 02/23/97)

		TheManager->Max()->Log()->LogEntry(SYSLOG_ERROR,NO_DIALOG,
			NULL,GetString(IDS_LOG_NOCONNECT),hostname);

		goto bail_out;

	}

	//-- Send "Receive" Command --------------------------
	
	char cmdstring[MAX_PATH];
	sprintf(cmdstring, "recv %d\n", (fbi->CurrentFrame() * fbi->GetCustomStep()) + data.startframe);
	res = tcp->Send(&ci,cmdstring,strlen(cmdstring));
	if (res != GCRES_SUCCESS)
		goto io_error;

	//-- Check for Ack
	
	res = tcp->Receive(&ci,szTemp,1);
	if (res != GCRES_SUCCESS)
		goto io_error;

	//-- Loop through rows
	
	for (y = 0; y < height; y++) {

		WORD *bf = rgbbuf;
		res = tcp->Receive(&ci,yuvbuf,width * 2,LINE_TIMEOUT);
		if (res != GCRES_SUCCESS)
			 goto io_error;
	
		YUVtoRGB(line,yuvbuf,width);
		
		if (s->PutPixels(0,y,width,line)!=1)
			goto io_error;

		//-- Progress Report
		
		if (fbi->GetUpdateWindow())
			SendMessage(fbi->GetUpdateWindow(),BMM_PROGRESS,y,height);

	}
	
	//-- Wait for ACK ------------------------------------

	res = tcp->Receive(&ci,szTemp,1);

	if (res != GCRES_SUCCESS) {

		io_error:
		
		//-- Log Entry (GG 02/23/97)

		TheManager->Max()->Log()->LogEntry(SYSLOG_ERROR,NO_DIALOG,
			NULL,GetString(IDS_LOG_IOERROR),hostname);
	
		bail_out:

		if (s)
			delete s;
		if (tcp) {
			tcp->Disconnect(&ci);
			tcp->Close();
			delete tcp;
			tcp = NULL;
		}
		CleanUp();
		return NULL;
	
	}
 
	//-- Set the storage's BitmapInfo

	tcp->Disconnect(&ci);
	tcp->Close();
	delete tcp;
	tcp = NULL;

	s->bi.CopyImageInfo(fbi);
	*status = BMMRES_SUCCESS;

	return  s;
	

}

//-----------------------------------------------------------------------------
// #> BitmapIO_WSD::OpenOutput()
//

BMMRES  BitmapIO_WSD::OpenOutput(BitmapInfo *fbi, Bitmap *map) {

	if (openMode != BMM_NOT_OPEN)
		return (ProcessImageIOError(fbi,BMMRES_INTERNALERROR));
		
	if (!map)
		return (ProcessImageIOError(fbi,BMMRES_INTERNALERROR));
		
	ReadCfg();
	
	//-- Attempt login

	tcp = (TCPcomm *)gcommCreate(gcTCP);

	if (!tcp)
		return BMMRES_ERRORTAKENCARE;

	//-- Set Error handlers ----------------------
	
	tcp->SetSilentMode(TRUE);
	tcp->RegisterErrorHandler((PERROR_HANDLER)ErrorHandler);
	
	//-- Init ------------------------------------
	
	if (!tcp->Init(fbi->GetUpdateWindow()))
		return BMMRES_ERRORTAKENCARE;
	
	ci.Reset();
	ci.SetServerName(hostname);
	
	if (tcp->rlogin(&ci) != GCRES_SUCCESS) {

		//-- Log Entry (GG 02/23/97)

		TheManager->Max()->Log()->LogEntry(SYSLOG_ERROR,NO_DIALOG,
			NULL,GetString(IDS_LOG_NOCONNECT),hostname);

		return BMMRES_ERRORTAKENCARE;
	}

	//-- Save Image Info Data

	bi.CopyImageInfo(fbi);    
	bi.SetUpdateWindow(fbi->GetUpdateWindow());

	this->map   = map;
	openMode    = BMM_OPEN_W;

	return BMMRES_SUCCESS;

}

//-----------------------------------------------------------------------------
// #> BitmapIO_WSD::Write()
//

BMMRES BitmapIO_WSD::Write(int frame) {

	GCRES res;

	//-- If we haven't gone through an OpenOutput(), leave

	if (openMode != BMM_OPEN_W)
		return (ProcessImageIOError(&bi,BMMRES_INTERNALERROR));

	if (!tcp)
		return (ProcessImageIOError(&bi,BMMRES_INTERNALERROR));
	
	//-- Check for a valid frame number

	if ((frame + data.startframe) > maxframes) {
		TCHAR title[MAX_PATH];
		_tcscpy(title,GetString(IDS_TITLE_LOG));
		TheManager->Max()->Log()->LogEntry(SYSLOG_ERROR,DISPLAY_DIALOG,
			title,
			GetString(IDS_LOG_INVALIDFRAMEERROR),(frame + data.startframe));
		return BMMRES_ERRORTAKENCARE;
	}
	
	//-- Allocate Buffers --------------------------------

	yuvbuf = (BYTE *)LocalAlloc(LPTR, width * 2);

	if (!yuvbuf) {
		memory_error:
		CleanUp();
		return (ProcessImageIOError(&bi,BMMRES_MEMORYERROR));
	}


	line = (BMM_Color_64 *)LocalAlloc(LPTR, width * sizeof(BMM_Color_64));

	if (!line)
		goto memory_error;
	
	//-- Convert Image and Transfer to Accom -------------
	
	int w = min(width,bi.Width());
	int h = bi.Height();
	int y;
    
	//-- Send "Send" Command
	
	char cmdstring[MAX_PATH];
	if (frame == BMM_SINGLEFRAME)
		frame = 0;
	sprintf(cmdstring, "send %d\n", (frame + data.startframe));

	res = tcp->Send(&ci,cmdstring,strlen(cmdstring));

	if (res != GCRES_SUCCESS)
		goto io_error;

	//-- Check for Ack
    
	res = tcp->Receive(&ci,szTemp,1);
	
	if (res != GCRES_SUCCESS)
		goto io_error;

	//-- Loop through rows
	
	for (y = 0; y < height; y++) {

		if (y < h) {
			GetOutputPixels(0,y,w,line);   // Get gamma-corrected, dithered pixels
			RGBtoYUV(line,yuvbuf,width);
		} else {
			if (y == h) {
				memset(line,0,width*sizeof(BMM_Color_64));
				RGBtoYUV(line,yuvbuf,width);
			}
		}
	
		res = tcp->Send(&ci,yuvbuf,(width * 2),LINE_TIMEOUT);
		if (res != GCRES_SUCCESS)
			 goto io_error;

		//-- Progress Report
		
		if (bi.GetUpdateWindow())
			SendMessage(bi.GetUpdateWindow(),BMM_PROGRESS,y,height);

	}
	
	//-- Wait for ACK ------------------------------------

	res = tcp->Receive(&ci,szTemp,1);
	
	if (res != GCRES_SUCCESS) {
		io_error:

		//-- Log Entry (GG 02/23/97)

		TheManager->Max()->Log()->LogEntry(SYSLOG_ERROR,NO_DIALOG,
			NULL,GetString(IDS_LOG_IOERROR),hostname);
	
		CleanUp();
		return BMMRES_ERRORTAKENCARE;
	}

	//-- Done --------------------------------------------
	
	CleanUp();   
	return (BMMRES_SUCCESS);

}

//-----------------------------------------------------------------------------
// #> BitmapIO_WSD::Close()
//

int  BitmapIO_WSD::Close(int flag ) {

	if (openMode != BMM_OPEN_W)
		return 0;

	if (tcp) {   
		tcp->Disconnect(&ci);
		tcp->Close();   
		delete tcp;
		tcp = NULL;
	}

	CleanUp();

	return 1;
}

//-----------------------------------------------------------------------------
// #> BitmapIO_WSD::RGBtoYUV()
//

#define DITHER(to, from) { int d = (from+e)>>8;  to = (d>255)?255:d; }

void BitmapIO_WSD::RGBtoYUV (BMM_Color_64 *rgb, BYTE *yuv, int len) {

	int i, r, g, b, e;
	int y1, y2, u, v, u0, u1, u2, v0, v1, v2;

	u = v = y1 = y2 = u0 = u1 = v0 = v1 = v2 = 0;

	//-- Old Method

	if (!data.chromadither) {
				
		for (i = (len>>1); i>0; i--) {

			//-- first pixel

			e = psrand()&0xff; 
			DITHER(r,rgb->r);
			DITHER(g,rgb->g);
			DITHER(b,rgb->b);
				
			rgb++;

			y1  = 16840*r +  33060*g +  6420*b;
			u   = -9668*r + -18987*g + 28654*b;
				
			//-- second pixel

			e = psrand()&0xff; 
			DITHER(r,rgb->r);
			DITHER(g,rgb->g);
			DITHER(b,rgb->b);

			rgb++;

			y2  = 16840*r +  33060*g +  6420*b;
			v   = 28661*r + -23998*g + -4662*b;

			*yuv++  = (u >>16) +128;
			*yuv++  = (y1>>16) + 16;
			*yuv++  = (v >>16) +128;
			*yuv++  = (y2>>16) + 16;

		}

	//-- Chroma Dithering

	} else {

		for (i = (len>>1); i>0; i--) {

			//-- first pixel

			e = psrand()&0xff; 
			DITHER(r,rgb->r);
			DITHER(g,rgb->g);
			DITHER(b,rgb->b);
				
			rgb++;

			y1  = 16829*r +  33039*g +  6416*b;
			u1  = -4853*r +  -9530*g + 14383*b;
			v1  = 14386*r + -12046*g + -2340*b;
				
			//-- second pixel

			e = psrand()&0xff; 
			DITHER(r,rgb->r);
			DITHER(g,rgb->g);
			DITHER(b,rgb->b);

			rgb++;

			y2  = 16829*r +  33039*g +  6416*b + (0xFFFF & y1);
			u2  = -2426*r +  -4765*g +  7191*b;
			v2  =  7193*r +  -6023*g + -1170*b;

			//-- Filter the chroma 

			u = u0 + u1 + u2 + (0xFFFF & u);
			v = v0 + v1 + v2 + (0xFFFF & v);
			u0 = u2;
			v0 = v2;

			*yuv++  = (u >>16) +128;
			*yuv++  = (y1>>16) + 16;
			*yuv++  = (v >>16) +128;
			*yuv++  = (y2>>16) + 16;

		}

	}
}

//-----------------------------------------------------------------------------
// #> BitmapIO_WSD::YUVtoRGB()
//

void BitmapIO_WSD::YUVtoRGB (BMM_Color_64 *rgb, BYTE *yuv, int len) {

	int j;  
	long y, u, v, y1, r, g, b;

	for(j=0;j<(len/2);j++) {
		
		u = *yuv++;
		u -= 128;
		y = *yuv++;
		y -= 16;
		
		if(y<0) y = 0;

		v  = *yuv++;
		v -= 128;
		y1 = *yuv++;
		y1-= 16;
		
		if(y1<0) y1 = 0;

		y *= 76310;
		r  = y + 104635*v;
		g  = y + -25690*u + -53294*v;
		b  = y + 132278*u;

		limit(r);
		limit(g);
		limit(b);

		rgb->r = (WORD)(r>>8);
		rgb->g = (WORD)(g>>8);
		rgb->b = (WORD)(b>>8);
		rgb->a = 0;
		rgb++;

		y1 *= 76310;
		r  =    y1 + 104635*v;
		g  =    y1 + -25690*u + -53294*v;
		b  =    y1 + 132278*u;

		limit(r);
		limit(g);
		limit(b);
		
		rgb->r = (WORD)(r>>8);
		rgb->g = (WORD)(g>>8);
		rgb->b = (WORD)(b>>8);
		rgb->a = 0;
		rgb++;

    }

}

//-----------------------------------------------------------------------------
// #> BitmapIO_WSD::CleanUp()
//

void BitmapIO_WSD::CleanUp () {
	if (rgbbuf)
		LocalFree(rgbbuf);
	if (yuvbuf)   
		LocalFree(yuvbuf);
	if (line)   
		LocalFree(line);
	rgbbuf= NULL;
	yuvbuf= NULL;
	line  = NULL;
}

//-----------------------------------------------------------------------------
// #> BitmapIO_WSD::EvalMatch()
//

void BitmapIO_WSD::EvalMatch( TCHAR *matchString ) {
	wsprintf(matchString,_T("%d"),data.startframe);
}
//-- EOF: wsd.cpp -------------------------------------------------------------
