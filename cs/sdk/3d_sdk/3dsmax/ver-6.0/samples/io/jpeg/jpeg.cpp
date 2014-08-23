//-----------------------------------------------------------------------------
// -------------------
// File ....: jpeg.cpp
// -------------------
// Author...: Gus J Grubba
// Date ....: October 1995
// Descr....: JPEG File I/O Module
//
// History .: Oct, 24 1995 - Started
//            
//-----------------------------------------------------------------------------
		
//-- Include files

#include <Max.h>
#include <bmmlib.h>
#include <istdplug.h>
#include <IParamb2.h>
#include "jpeg.h"
#include "jpegrc.h"

extern "C" {
#include "interfce.h"
}

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

HINSTANCE	hInst = NULL;
JPEGDATA		data;

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

TCHAR *GetString(int id)
	{
	static TCHAR buf[256];

	if (hInst)
		return LoadString(hInst, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
	}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// JPEG Class Description

class JPEGClassDesc:public ClassDesc2 {
	
	public:

		int             IsPublic     ( )                   { return 1;                }
		void           *Create       ( BOOL loading=FALSE) { return new BitmapIO_JPEG; }
		const TCHAR    *ClassName    ( )                   { return GetString(IDS_JPEG);     }
		SClass_ID       SuperClassID ( )                   { return BMM_IO_CLASS_ID;  }
		Class_ID        ClassID      ( )                   { return Class_ID(JPEGCLASSID,0);    }
		const TCHAR    *Category     ( )                   { return GetString(IDS_BITMAP_IO); }
		const TCHAR    *InternalName ( )                   { return _T("jpegio"); }
		HINSTANCE		HInstance    ( )                   { return hInst; }
};

static JPEGClassDesc JPEGDesc;

//-----------------------------------------------------------------------------
// Interface

DLLEXPORT const TCHAR * LibDescription ( )  { 
	return GetString(IDS_JPEG_DESC); 
}

DLLEXPORT int LibNumberClasses ( ) { 
	return 1; 
}

DLLEXPORT ClassDesc *LibClassDesc(int i) {
	switch(i) {
		case  0: return &JPEGDesc; break;
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


// Function published July 2000 - CCJ
// FP Interface
class BitmapIO_Jpeg_Imp : public IBitmapIO_Jpeg {
public:
	int		GetQuality();
	void	SetQuality(int quality);
	int		GetSmoothing();
	void	SetSmoothing(int smoothing);

	// function IDs 
	enum { jpegio_getQuality, jpegio_setQuality, jpegio_getSmoothing, jpegio_setSmoothing }; 

	DECLARE_DESCRIPTOR(BitmapIO_Jpeg_Imp) 

	// dispatch map
	BEGIN_FUNCTION_MAP
		FN_0(jpegio_getQuality, TYPE_INT, GetQuality);
		VFN_1(jpegio_setQuality, SetQuality, TYPE_INT);

		FN_0(jpegio_getSmoothing, TYPE_INT, GetSmoothing);
		VFN_1(jpegio_setSmoothing, SetSmoothing, TYPE_INT);
	END_FUNCTION_MAP 
	};

static BitmapIO_Jpeg_Imp pngIOInterface(
		BMPIO_INTERFACE, _T("ijpegio"), IDS_JPEGIO_INTERFACE, &JPEGDesc, 0,
			BitmapIO_Jpeg_Imp::jpegio_getQuality, _T("getQuality"), IDS_JPEGIO_GETQUALITY, TYPE_INT, 0, 0, 
			BitmapIO_Jpeg_Imp::jpegio_setQuality, _T("setQuality"), IDS_JPEGIO_SETQUALITY, TYPE_VOID, 0, 1, 
			_T("quality"), 0, TYPE_INT,

			BitmapIO_Jpeg_Imp::jpegio_getSmoothing, _T("getSmoothing"), IDS_JPEGIO_GETSMOOTHING, TYPE_INT, 0, 0, 
			BitmapIO_Jpeg_Imp::jpegio_setSmoothing, _T("setSmoothing"), IDS_JPEGIO_SETSMOOTHING, TYPE_VOID, 0, 1, 
			_T("smoothing"), 0, TYPE_INT,
		end); 


int BitmapIO_Jpeg_Imp::GetQuality()
	{
	int quality = 75;

	BitmapIO_JPEG* p = new BitmapIO_JPEG;
	if (p) {
		p->ReadCfg();
		quality = p->GetConfiguration()->qFactor;
		delete p;
		}
	return quality;
	}

void BitmapIO_Jpeg_Imp::SetQuality(int quality)
	{
	BitmapIO_JPEG* p = new BitmapIO_JPEG;
	if (p) {
		p->ReadCfg();
		p->GetConfiguration()->qFactor = quality;
		p->WriteCfg();
		delete p;
		}
	}

int BitmapIO_Jpeg_Imp::GetSmoothing()
	{
	int smoothing = 75;

	BitmapIO_JPEG* p = new BitmapIO_JPEG;
	if (p) {
		p->ReadCfg();
		smoothing = p->GetConfiguration()->Smooth;
		delete p;
		}
	return smoothing;
	}

void BitmapIO_Jpeg_Imp::SetSmoothing(int smoothing)
	{
	BitmapIO_JPEG* p = new BitmapIO_JPEG;
	if (p) {
		p->ReadCfg();
		p->GetConfiguration()->Smooth = smoothing;
		p->WriteCfg();
		delete p;
		}
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
// *> UpdateText()
//

static void UpdateText(HWND hWnd) {

	int  pos;
	char buf[64];
	HWND hDlg;

	hDlg = GetDlgItem(hWnd,IDC_QUALITY);
	pos  = SendMessage(hDlg, TBM_GETPOS,0,0);
	sprintf(buf,"%d",pos);
	SetDlgItemText(hWnd, IDC_QUALITY_TEXT, buf);

	/*
	hDlg = GetDlgItem(hWnd,IDC_FILE_SIZE);
	pos  = SendMessage(hDlg, TBM_GETPOS,0,0);
	sprintf(buf,"%d",pos);
	SetDlgItemText(hWnd, IDC_SIZE_TEXT, buf);
	*/

	hDlg = GetDlgItem(hWnd,IDC_SMOOTH);
	pos  = SendMessage(hDlg, TBM_GETPOS,0,0);
	sprintf(buf,"%d",pos);
	SetDlgItemText(hWnd, IDC_SMOOTH_TEXT, buf);
	
}

//-----------------------------------------------------------------------------
// *> ControlCtrlDlgProc()
//

BOOL BitmapIO_JPEG::Control(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {

	int   pos;
	HWND  hDlg;

	static BOOL forced = FALSE;

	switch (message) {
		
		case WM_INITDIALOG: {
	 
			  CenterWindow(hWnd,GetParent(hWnd));

			  InitCommonControls();
			  
			if (!UserData.userDataSaved)
				ReadCfg();

			  hDlg = GetDlgItem(hWnd,IDC_QUALITY);
			  SendMessage(hDlg, TBM_SETRANGE,    TRUE, (LPARAM) MAKELONG(1, 100));
			  SendMessage(hDlg, TBM_SETPAGESIZE, 0,    (LPARAM) 5);
			  SendMessage(hDlg, TBM_SETTICFREQ,  5,    (LPARAM) UserData.qFactor); 
			  SendMessage(hDlg, TBM_SETPOS,      TRUE, (LPARAM) UserData.qFactor); 
			
			  hDlg = GetDlgItem(hWnd,IDC_FILE_SIZE);
			  SendMessage(hDlg, TBM_SETRANGE,    TRUE, (LPARAM) MAKELONG(1, 100));
			  SendMessage(hDlg, TBM_SETPAGESIZE, 0,    (LPARAM) 5);
			  SendMessage(hDlg, TBM_SETTICFREQ,  5,    (LPARAM) 101 - UserData.qFactor); 
			  SendMessage(hDlg, TBM_SETPOS,      TRUE, (LPARAM) 101 - UserData.qFactor); 
			
			  hDlg = GetDlgItem(hWnd,IDC_SMOOTH);
			  SendMessage(hDlg, TBM_SETRANGE,    TRUE, (LPARAM) MAKELONG(0, 100));
			  SendMessage(hDlg, TBM_SETPAGESIZE, 0,    (LPARAM) 5);
			  SendMessage(hDlg, TBM_SETTICFREQ,  5,    (LPARAM) UserData.Smooth); 
			  SendMessage(hDlg, TBM_SETPOS,      TRUE, (LPARAM) UserData.Smooth); 
			
			  UpdateText(hWnd);
			  
			  return 1;
			  
		}

		case WM_HSCROLL:
			  
			  hDlg = GetDlgItem(hWnd,IDC_QUALITY);
			  if ((HWND)lParam == hDlg) {
				  if (!forced) {
					  forced = TRUE;
					  pos    = SendMessage(hDlg, TBM_GETPOS,0,0);
					  hDlg   = GetDlgItem(hWnd,IDC_FILE_SIZE);
					  SendMessage(hDlg, TBM_SETPOS, TRUE, 101 - pos); 
					  UpdateText(hWnd);
				  } else
					  forced = FALSE;
				  return 1;
			  }   
			  
			  hDlg = GetDlgItem(hWnd,IDC_FILE_SIZE);
			  if ((HWND)lParam == hDlg) {
				  if (!forced) {
					  forced = TRUE;
					  hDlg   = GetDlgItem(hWnd,IDC_FILE_SIZE);
					  pos    = SendMessage(hDlg, TBM_GETPOS,0,0);
					  hDlg   = GetDlgItem(hWnd,IDC_QUALITY);
					  SendMessage(hDlg, TBM_SETPOS, TRUE, 101 - pos); 
					  UpdateText(hWnd);
				  } else
					 forced = FALSE;
				  return 1;   
			  }
			  
			  hDlg = GetDlgItem(hWnd,IDC_SMOOTH);
			  if ((HWND)lParam == hDlg) {
				  UpdateText(hWnd);
				  return 1;   
			  }
    
			  break;

		case WM_COMMAND:

			  switch (LOWORD(wParam)) {
				  
				  case IDOK:              
						 hDlg = GetDlgItem(hWnd,IDC_QUALITY);
						 UserData.qFactor = (BYTE)SendMessage(hDlg, TBM_GETPOS,0,0);
						 hDlg = GetDlgItem(hWnd,IDC_SMOOTH);
						 UserData.Smooth  = (BYTE)SendMessage(hDlg, TBM_GETPOS,0,0);

					 WriteCfg();

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

static void InitUserData(JPEGUSERDATA *p) {
	p->version = JPEGVERSION;
	p->qFactor = 75; 
	p->Smooth  = 0;
	p->userDataSaved = FALSE;
}

//-----------------------------------------------------------------------------
// #> BitmapIO_JPEG::BitmapIO_JPEG()

BitmapIO_JPEG::BitmapIO_JPEG  ( ) { 
	InitUserData(&UserData);
	}

BitmapIO_JPEG::~BitmapIO_JPEG ( ) { }

//-----------------------------------------------------------------------------
// #> BitmapIO_JPEG::LongDesc()

const TCHAR *BitmapIO_JPEG::LongDesc() {
	return GetString(IDS_JPEG_FILE);
}
	
//-----------------------------------------------------------------------------
// #> BitmapIO_JPEG::ShortDesc()

const TCHAR *BitmapIO_JPEG::ShortDesc() {
	return GetString(IDS_JPEG);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_JPEG::LoadConfigure()

BOOL BitmapIO_JPEG::LoadConfigure ( void *ptr ) {
	JPEGUSERDATA *buf = (JPEGUSERDATA *)ptr;
	if (buf->version == JPEGVERSION) {
		memcpy((void *)&UserData,ptr,sizeof(JPEGUSERDATA));
		return (TRUE);
	} else
		return (FALSE);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_JPEG::SaveConfigure()

BOOL BitmapIO_JPEG::SaveConfigure ( void *ptr ) {
	if (ptr) {
		UserData.userDataSaved = TRUE;
		memcpy(ptr,(void *)&UserData,sizeof(JPEGUSERDATA));
		return (TRUE);
	} else
		return (FALSE);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_JPEG::EvaluateConfigure()

DWORD BitmapIO_JPEG::EvaluateConfigure ( ) {
	 return (sizeof(JPEGUSERDATA));
}

//-----------------------------------------------------------------------------
// #> BitmapIO_JPEG::ShowAbout()

void BitmapIO_JPEG::ShowAbout(HWND hWnd) {
	DialogBoxParam(
		hInst,
		MAKEINTRESOURCE(IDD_JPEG_ABOUT),
		hWnd,
		(DLGPROC)AboutCtrlDlgProc,
		(LPARAM)0);
}

//-----------------------------------------------------------------------------
// *> ControlDlgProc
//

static INT_PTR CALLBACK ControlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
	static BitmapIO_JPEG *bm = NULL;
	if (message == WM_INITDIALOG) 
		bm = (BitmapIO_JPEG *)lParam;
	if (bm) 
		return (bm->Control(hWnd,message,wParam,lParam));
	else
		return(FALSE);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_JPEG::Control()

BOOL BitmapIO_JPEG::ShowControl(HWND hWnd, DWORD flag ) {
	return (
		DialogBoxParam(
		hInst,
		MAKEINTRESOURCE(IDD_JPEG_CONTROL),
		hWnd,
		(DLGPROC)ControlDlgProc,
		(LPARAM)this)
	);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_JPEG::GetImageInfo()

BMMRES BitmapIO_JPEG::GetImageInfo ( BitmapInfo *fbi ) {

	//-- Open BMP File -----------------------------------
	
	File file(fbi->Name(), _T("rb"));

	if (!file.stream)
		return (ProcessImageIOError(fbi));

	//-- Read File Header --------------------------------

	memset(&data,0,sizeof(JPEGDATA));
	data.input_file = file.stream;
	JpegInfo(&data);

	if (data.status)
		return (ProcessImageIOError(fbi,BMMRES_BADFILEHEADER));

	//-- Update Bitmap Info ------------------------------
	
	fbi->SetWidth(data.width);
	fbi->SetHeight(data.height);
	
	switch (data.components) {
		case 1:
			fbi->SetType(BMM_GRAY_8);
			break;
		case 3:
			fbi->SetType(BMM_TRUE_24);
			break;
		case 4:
			fbi->SetType(BMM_TRUE_32);
			break;
		default:
			fbi->SetType(BMM_NO_TYPE);
		}
	
	//fbi->SetGamma(1.0f);  //DS: 6/21/96
	fbi->SetAspect(1.0f);
	fbi->SetFirstFrame(0);
	fbi->SetLastFrame(0);
	fbi->ResetFlags(MAP_HAS_ALPHA);

	return BMMRES_SUCCESS;

	}

//-----------------------------------------------------------------------------
//-- BitmapIO_JPEG::Load()

BitmapStorage *BitmapIO_JPEG::Load(BitmapInfo *fbi, Bitmap *map, BMMRES *status) {

	BitmapStorage *s = NULL;
	int   mType;

	//-- Initialize Status Optimistically

	*status = BMMRES_SUCCESS;

	//-- Make sure nothing weird is going on

	if(openMode != BMM_NOT_OPEN) {
		*status = ProcessImageIOError(fbi,BMMRES_INTERNALERROR);
		return NULL;
	}

	//-- Update Bitmap Info (and fill out JPEGDATA structure 'data')
	
	*status = GetImageInfo(fbi);
	
	if (*status != BMMRES_SUCCESS)
		return(NULL);

	//-- Allocate Image buffer ---------------------------
	
	if (data.components != 1 && data.components != 3 && data.components != 4) {
		*status = ProcessImageIOError(fbi,BMMRES_BADFILEHEADER);
		return(NULL);
	}
	  
	//-- Open JPEG File ----------------------------------
	
	File file(fbi->Name(), _T("rb"));

	if (!file.stream) {
		*status = ProcessImageIOError(fbi);
		return NULL;
	}

	int numbytes = fbi->Width() * fbi->Height() * data.components;



	//--- PRE-ALLOCATE storage, so can (possibly) use it as input buffer:

	ULONG stype = data.components==1?BMM_GRAY_8: BMM_TRUE_32;

	//-- Create Image Storage ---------------------------- 
	
	s = BMMCreateStorage(map->Manager(),stype);
	if(!s) {
		*status = ProcessImageIOError(fbi,BMMRES_INTERNALERROR);
		return NULL;
		}

	//-- Allocate Image Storage --------------------------
	
	if (s->Allocate(fbi,map->Manager(),BMM_OPEN_R)==0) {
		memory_error:
		*status = ProcessImageIOError(fbi,BMMRES_MEMORYERROR);
		delete s;
		return NULL;
		}

	BOOL useStorage = FALSE;
	BYTE *Buffer = (BYTE *)s->GetStoragePtr(&mType);
	BYTE *inBuffer = NULL;

	if (Buffer) {
		if (mType == BMM_TRUE_24 && data.components == 3) {
			inBuffer = Buffer;
			useStorage = TRUE;
			}
//		else if (mType == BMM_GRAY_8 && data.components == 1)  {
//			inBuffer = Buffer;
//			useStorage = TRUE;
//			}
		}

	if (!useStorage) {
		inBuffer = (BYTE *)LocalAlloc(LPTR,numbytes);
		if (!inBuffer) {
			*status = ProcessImageIOError(fbi,BMMRES_MEMORYERROR);
			return 0;
			}
		}
	

	//-- Read Image File ---------------------------------
	data.ptr             = inBuffer;
	data.input_file      = file.stream;
	data.hWnd            = fbi->GetUpdateWindow();
	data.ProgressMsg     = BMM_PROGRESS;

	JpegRead(&data);

	if (data.status) {
		*status = ProcessImageIOError(fbi,GetString(IDS_READ_ERROR));
		if (!useStorage)
			LocalFree(inBuffer);
		return (NULL);
		}
	

	if (!useStorage) {
		BMM_Color_64 *line = (BMM_Color_64 *)LocalAlloc(LPTR,fbi->Width()*sizeof(BMM_Color_64));
		BYTE *bf;

		if (!line) {
			LocalFree(inBuffer);
			goto memory_error;
			}

		bf = inBuffer;
		
		for (int y=0; y<fbi->Height(); y++) {

			//-- 24 bit buffer

			if (data.components == 3) {
				for (int x=0;x<fbi->Width();x++) {
					line[x].r = (*bf++) << 8;
					line[x].g = (*bf++) << 8;
					line[x].b = (*bf++) << 8;
				}   

			//-- 32 bit buffer

			} else if (data.components == 4) {

				for (int x=0;x<fbi->Width();x++) {
					line[x].r = (*bf++) << 8;
					line[x].g = (*bf++) << 8;
					line[x].b = (*bf++) << 8;
					line[x].a = (*bf++) << 8;
				}   

			//-- 8 bit buffer

			} else {

				for (int x=0;x<fbi->Width();x++) {
					line[x].r = (*bf)   << 8;
					line[x].g = (*bf)   << 8;
					line[x].b = (*bf++) << 8;
				}   

			}

			if (s->PutPixels(0,y,fbi->Width(),line)!=1) {
				LocalFree(line);
				LocalFree(inBuffer);
				goto memory_error;
			}
		}
		
		LocalFree(line);
		LocalFree(inBuffer);

	} else {
		// Data's already in storage.
		//	memcpy(Buffer,inBuffer,numbytes);
	
		}

	
	//-- Set the storage's BitmapInfo

	s->bi.CopyImageInfo(fbi);

	return  s;


}

//-----------------------------------------------------------------------------
// #> BitmapIO_JPEG::OpenOutput()
//

BMMRES BitmapIO_JPEG::OpenOutput(BitmapInfo *fbi, Bitmap *map) {

	if (openMode != BMM_NOT_OPEN)
		return (ProcessImageIOError(fbi,BMMRES_INTERNALERROR));
		
	if (!map)
		return (ProcessImageIOError(fbi,BMMRES_INTERNALERROR));
		
	//-- Check for Default Configuration -----------------
	
	if (!UserData.userDataSaved)
		ReadCfg();

	//-- Save Image Info Data

	bi.CopyImageInfo(fbi);    
	bi.SetUpdateWindow(fbi->GetUpdateWindow());
	
	this->map   = map;
	openMode    = BMM_OPEN_W;

	return BMMRES_SUCCESS;

}

//-----------------------------------------------------------------------------
// #> BitmapIO_JPEG::Write()
//
//

BMMRES BitmapIO_JPEG::Write(int frame) {
	
	//-- If we haven't gone through an OpenOutput(), leave

	if (openMode != BMM_OPEN_W)
		return (ProcessImageIOError(&bi,BMMRES_INTERNALERROR));

	//-- Resolve Filename --------------------------------

	TCHAR filename[MAX_PATH];

	if (frame == BMM_SINGLEFRAME) {
		_tcscpy(filename,bi.Name());
	} else {
		if (!BMMCreateNumberedFilename(bi.Name(),frame,filename)) {
			return (ProcessImageIOError(&bi,BMMRES_NUMBEREDFILENAMEERROR));
		}
	}
	
	//-- Allocate 24 bit buffer --------------------------
	
	int numbytes = bi.Width() * bi.Height() * 3;
	
	BYTE *rgbBuffer = (BYTE *)LocalAlloc(LPTR,numbytes);

	if (!rgbBuffer) {
		return (ProcessImageIOError(&bi,BMMRES_MEMORYERROR));
	}

	//-- Convert 48 to 24 --------------------------------
	
//   int mType;
//   WORD *Buffer = (WORD *)map->GetStoragePtr(&mType);
	
    //-- Use one general method   -- DS 1-6-96

	int w = bi.Width();
	int h = bi.Height();
	BMM_Color_32 *pbuf= (BMM_Color_32 *)LocalAlloc(LPTR,w*sizeof(BMM_Color_32));
	if (!pbuf) {
		//-- Check for silent mode
		 //-- Display Error Dialog Box
		//-- Log Error
		LocalFree(rgbBuffer);
		return (ProcessImageIOError(&bi,BMMRES_MEMORYERROR));
		 }
	BYTE *dst = rgbBuffer;
	for (int y=0; y<h; y++) { 
		// Get gamma corrected and (possibly) dithered output pixels
		GetDitheredOutputPixels(0,y,w,pbuf); 
		for (int x = 0; x < w; x++)  {
			(*dst++) = (BYTE)(pbuf[x].r);
			(*dst++) = (BYTE)(pbuf[x].g);
			(*dst++) = (BYTE)(pbuf[x].b);
			}
		}
	LocalFree(pbuf);

	//-- Create Image File -------------------------------
	
	File file(filename, _T("wb"));
	
	if (!file.stream) {
		return (ProcessImageIOError(&bi));
	}
	
	//-- Setup JPEG Structure ----------------------------
	
	memset(&data,0,sizeof(JPEGDATA));

	data.ptr             = rgbBuffer;
	data.width           = bi.Width();
	data.height          = bi.Height();
	data.output_file     = file.stream;
	data.aritcoding      = HUFFMAN_CODING;
	data.CCIR601sampling = FALSE;
	data.smoothingfactor = UserData.Smooth;
	data.quality         = UserData.qFactor;
	data.hWnd            = bi.GetUpdateWindow();
	data.ProgressMsg     = BMM_PROGRESS;

	//-- Write Image File --------------------------------
	
	JpegWrite(&data);
	LocalFree(rgbBuffer);

	if (data.status)
		return (ProcessImageIOError(&bi,GetString(IDS_WRITE_ERROR)));
	else   
		return (BMMRES_SUCCESS);

}

//-----------------------------------------------------------------------------
// #> BitmapIO_JPEG::Close()
//

int  BitmapIO_JPEG::Close( int flag ) {

	return 1;

}


//-----------------------------------------------------------------------------
// #> BitmapIO_JPEG::GetCfgFilename()
//

void BitmapIO_JPEG::GetCfgFilename( TCHAR *filename ) {
	_tcscpy(filename,TheManager->GetDir(APP_PLUGCFG_DIR));
	int len = _tcslen(filename);
	if (len) {
		if (_tcscmp(&filename[len-1],_T("\\")))
			_tcscat(filename,_T("\\"));
		}   
	_tcscat(filename,JPEGCONFIGNAME);   
	}



//-----------------------------------------------------------------------------
// #> BitmapIO_JPEG::ReadCfg()
//

BOOL BitmapIO_JPEG::ReadCfg() {
	
	TCHAR filename[MAX_PATH];
	GetCfgFilename(filename);

	//-- Open Configuration File
	
	File file(filename, _T("rb"));
	
	if (!file.stream)
		return (FALSE);
	
	fseek(file.stream,0,SEEK_END);
	DWORD size = (DWORD)ftell(file.stream);
	
	if (size) {

		fseek(file.stream,0,SEEK_SET);
		
		//-- Allocate Temp Buffer
		
		BYTE *buf = (BYTE *)LocalAlloc(LPTR,size);
		
		if (!buf)
			return (FALSE);
		
		//-- Read Data Block and Set it
		
		BOOL res = FALSE;
		
		if (fread(buf,1,size,file.stream) == size)
			res = LoadConfigure(buf);

		if(!res) {
			InitUserData(&UserData);
			}
					
		LocalFree(buf);
	
		return (res);
	
	}
	
	return (FALSE);
}
	
//-----------------------------------------------------------------------------
// #> BitmapIO_JPEG::WriteCfg()
//

void BitmapIO_JPEG::WriteCfg() {
 
	TCHAR filename[MAX_PATH];
	GetCfgFilename(filename);
	
	//-- Find out buffer size
	
	DWORD size = EvaluateConfigure();
	
	if (!size)
		return;
	
	//-- Allocate Temp Buffer
	
	BYTE *buf = (BYTE *)LocalAlloc(LPTR,size);
	
	if (!buf)
		return;
	
	//-- Get Data Block and Write it
	
	if (SaveConfigure(buf)) {   
		File file(filename, _T("wb"));
		if (file.stream) {
			fwrite(buf,1,size,file.stream);
		}
	}
	
	LocalFree(buf);
	
}

//-- EOF: jpeg.cpp -------------------------------------------------------------
