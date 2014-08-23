//-----------------------------------------------------------------------------
// --------------------
// File ....: tif.cpp
// --------------------
// Author...: Tom Hudson
// Date ....: Feb. 20, 1996
// Descr....: TIF File I/O Module
//
// History .: Feb. 20 1996 - Started
//			  March 2001, Rewrite for 6.0.  Gord Lueck

//-- Includes -----------------------------------------------------------------

#include <Max.h>
#include <iparamb2.h>
#include <bmmlib.h>
#include "pixelbuf.h"
#include "tif.h"
#include "tifrc.h"
#include "color.h"
#include "tif_luv.c"

#include <iFnPub.h>//class MaxException

//-----------------------------------------------------------------------------
//-- File Class

class File {
     public:
        FILE *stream;
        File(const TCHAR *name, const TCHAR *mode) { stream = _tfopen(name,mode); }
        ~File() { Close(); }
        void Close() { if(stream) fclose(stream); stream = NULL; }
     };

//-- Globals ------------------------------------------------------------------

HINSTANCE hInst = NULL;
static TIFFErrorHandler _MAXTIFFWarningHandler = MAXWarningHandler;
static TIFFErrorHandler _MAXTIFFErrorHandler = MAXErrorHandler;

static void tiff_error_on () {
	TIFFSetWarningHandler(_MAXTIFFWarningHandler);
	TIFFSetErrorHandler(_MAXTIFFErrorHandler);
}

static void tiff_error_off () {
	TIFFSetWarningHandler(0);
	TIFFSetErrorHandler(0);
}

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

//Number of simultaneous live strings.
//Libtiff uses a large number of strings simultaneously in a dialog, so this method 
//was modified so that it iterates through a different number of buffers.

#define TIF_MAX_STRINGS			4
#define TIF_MAX_STRING_SIZE		256

//Declare this function as extern "C" so that the libtiff stuff can hook into it.

extern "C" TCHAR *GetString(int id)
	{
	static int bufPos=0;
	static TCHAR buf[TIF_MAX_STRING_SIZE*TIF_MAX_STRINGS];
	TCHAR* ret;

	if (hInst)
		if (LoadString(hInst, id, buf+bufPos, TIF_MAX_STRING_SIZE))
			{
			ret = buf+bufPos;
			bufPos+=TIF_MAX_STRING_SIZE;
			if (bufPos >= TIF_MAX_STRINGS*TIF_MAX_STRING_SIZE) bufPos = 0;
			return ret;
			}
	return NULL;
	}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// TIF Class Description

class TIFClassDesc:public ClassDesc2 {
     public:
        int           IsPublic     ( ) { return 1; }	
        void         *Create       ( BOOL loading=FALSE) { return new BitmapIO_TIF; }
        const TCHAR  *ClassName    ( ) { return GetString(IDS_TIF); }
        SClass_ID     SuperClassID ( ) { return BMM_IO_CLASS_ID; }
        Class_ID      ClassID      ( ) { return Class_ID(TIFCLASSID,0); }
        const TCHAR  *Category     ( ) { return GetString(IDS_BITMAP_IO);  }
		const TCHAR  *InternalName ( ) { return _T("tifio"); }
};

static TIFClassDesc TIFDesc;

//-----------------------------------------------------------------------------
// Interface

DLLEXPORT const TCHAR * LibDescription ( )  { 
     return GetString(IDS_TIF_DESC); 
}

DLLEXPORT int LibNumberClasses ( ) { 
     return 1; 
}


DLLEXPORT ClassDesc *LibClassDesc(int i) {
     switch(i) {
        case  0: return &TIFDesc; break;
        default: return 0;        break;
     }
}

DLLEXPORT ULONG LibVersion ( )  { 
     return ( VERSION_3DSMAX ); 
}

// Let the plug-in register itself for deferred loading
DLLEXPORT ULONG CanAutoDefer()
{
	return 1;
}

TIFUSERDATA CreateDefaults()
	{
	TIFUSERDATA ret;
	ret.version = TIFVERSION;
	ret.saved = TRUE;//pretend we're saved
	ret.writeType = tif_write_color;
	ret.compressionType = tif_compress_none;
#ifdef GEOREFSYS_UVW_MAPPING 
	ret.geoInfo = FALSE;
	ret.matrix.IdentityMatrix();
	
#endif
	ret.dpi = 300.0f;
	ret.writeAlpha = FALSE;
	//Lighting analysis stuff - when creating this, assume no lighting analysis.
	ret.disableControl = FALSE;
	ret.lightActive = FALSE;
	ret.lumStonits = NO_STONITS;	
	return ret;
	}

/************************************************************************
/*	
/*	Compare UserData's.  Everything but if saved.
/*	
/************************************************************************/

BOOL TIFUSERDATA::operator==(const _tifuserdata& other)
	{
	return (version == other.version &&
		writeType == other.writeType &&
		compressionType == other.compressionType &&
		writeAlpha == other.writeAlpha &&
		disableControl == other.disableControl &&
		lightActive == other.lightActive &&
		lumStonits == other.lumStonits &&
		dpi == other.dpi
#ifdef GEOREFSYS_UVW_MAPPING 
		&& geoInfo == other.geoInfo &&
		matrix == other.matrix 		
#endif
		) ? 1 : 0;
	}

static TIFUSERDATA tifDefaults = CreateDefaults();

/************************************************************************
/*	
/*	Implementation of the TiffInterface - TiffInterfaceImp
/*	
/************************************************************************/
/************************************************************************
/*	
/*	The methods of this class access the tifDefaults global struct.
/*	
/************************************************************************/

//	 PROP_FNS(tif_get_dpi, GetDPI, tif_set_dpi, SetDPI, TYPE_INT);
//FN_0(tif_get_dpi, TYPE_INT, GetDPI)
//	 VFN_1(tif_set_dpi, SetDPI, TYPE_INT)

class TiffInterfaceImp : public TiffInterface
{

	DECLARE_DESCRIPTOR(TiffInterfaceImp);

	BEGIN_FUNCTION_MAP	 
	 PROP_FNS(tif_get_photometric, GetPhotometric, tif_set_photometric, SetPhotometric, TYPE_ENUM);
	 PROP_FNS(tif_get_compression, GetCompression, tif_set_compression, SetCompression, TYPE_ENUM);
	 PROP_FNS(tif_get_show_control, GetShowControl, tif_set_show_control, SetShowControl, TYPE_ENUM);
	 PROP_FNS(tif_get_alpha, GetAlpha, tif_set_alpha, SetAlpha, TYPE_ENUM);
	 PROP_FNS(tif_get_dpi, GetDPI, tif_set_dpi, SetDPI, TYPE_FLOAT);	 
	END_FUNCTION_MAP

	photometric GetPhotometric(){ return (photometric)tifDefaults.writeType; 	}
	void SetPhotometric(int newPhotometric){ tifDefaults.writeType = newPhotometric; tifDefaults.saved = FALSE;}

	BOOL GetShowControl(){return !tifDefaults.disableControl; }
	void SetShowControl(BOOL show = TRUE){ tifDefaults.disableControl = !show; tifDefaults.saved = FALSE;}

	compression GetCompression(){ return (compression)tifDefaults.compressionType; }
	void SetCompression(int newCompress) { tifDefaults.compressionType = newCompress; tifDefaults.saved = FALSE;}

	BOOL GetAlpha(){return tifDefaults.writeAlpha; }
	void SetAlpha(BOOL onOff){ tifDefaults.writeAlpha = onOff; tifDefaults.saved = FALSE;}

	// added dpi resolution functions to the interface
	// Dave Cunningham, September 11, 2001
	double	GetDPI()	{ return tifDefaults.dpi;  }

	void		SetDPI(double newDPI) { 
		if(newDPI >= 0.0f)	{
			tifDefaults.dpi = newDPI; 
			tifDefaults.saved = FALSE; 
		}
	}
}; 


static TiffInterfaceImp tifi (
	TIFF_OPTIONS_INTERFACE, _T("itifio"), IDS_TIF_DESC, &TIFDesc, 0,
	tif_get_photometric, _T("getType"), IDS_TIFIO_GETTYPE, TYPE_ENUM, tif_photo, 0, 0,
	tif_set_photometric, _T("setType"), IDS_TIFIO_SETTYPE, TYPE_VOID, 0, 1, 
	_T("type"), 0, TYPE_ENUM, tif_photo,

	tif_get_show_control, _T("getShowControl"), IDS_TIFIO_GETSHOWCONTROL, TYPE_ENUM, tif_show_control, 0, 0,
	tif_set_show_control, _T("setShowControl"), IDS_TIFIO_SETSHOWCONTROL, TYPE_VOID, 0, 1, 
	_T("showControl"), 0, TYPE_ENUM, tif_show_control,

	tif_get_compression, _T("getCompression"), IDS_TIFIO_GETCOMPRESSION, TYPE_ENUM, tif_compression, 0, 0,
	tif_set_compression, _T("setCompression"), IDS_TIFIO_SETCOMPRESSION, TYPE_VOID, 0, 1, 
	_T("compression"), 0, TYPE_ENUM, tif_compression,

	tif_get_alpha, _T("getAlpha"), IDS_TIFIO_GETALPHA, TYPE_ENUM, tif_alpha, 0, 0,
	tif_set_alpha, _T("setAlpha"), IDS_TIFIO_SETALPHA, TYPE_VOID, 0, 1, 
	_T("onOff"), 0, TYPE_ENUM, tif_alpha,

	tif_get_dpi, _T("getDPI"), IDS_TIFIO_GETDPI, TYPE_FLOAT, 0, 0,
	tif_set_dpi, _T("setDPI"), IDS_TIFIO_SETDPI, TYPE_VOID, 0, 1, 
					 _T("dpi"), 0, TYPE_FLOAT, 

	enums,
		tif_photo, 4,
			_T("mono"), tif_write_mono,
			_T("color"), tif_write_color,
			_T("logL"), tif_write_logl,
			_T("logLUV"), tif_write_logluv,
		tif_show_control, 2,
			_T("true"), 1,
			_T("false"), 0,
		tif_compression, 2,
			_T("none"), tif_compress_none,
			_T("packBits"), tif_compress_packbits,
		tif_alpha, 2,
			_T("true"), 1,
			_T("false"), 0,
	
	end
);

//	tif_test, 2,
//		   _T("true"), 1,
//			_T("false"), 0,

BitmapIO_TIF::BitmapIO_TIF() {
	tif = NULL;
	loadStorage = saveStorage = NULL;
	inStream = NULL;
	
	// Init user data
	UserData = tifDefaults;	

	TIFFSetWarningHandler(_MAXTIFFWarningHandler);
	TIFFSetErrorHandler(_MAXTIFFErrorHandler);
	}


BitmapIO_TIF::~BitmapIO_TIF()
	{
	}
//-----------------------------------------------------------------------------
// #> BitmapIO_TIF::LongDesc()

const TCHAR *BitmapIO_TIF::LongDesc()  {
     return GetString(IDS_TIF_FILE);
}
     
//-----------------------------------------------------------------------------
// #> BitmapIO_TIF::ShortDesc()

const TCHAR *BitmapIO_TIF::ShortDesc() {
     return GetString(IDS_TIF);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_TIF::LoadConfigure()
//Sets our internal defaults to an already saved ptr parameter

BOOL BitmapIO_TIF::LoadConfigure ( void *ptr ) {
     TIFUSERDATA *buf = (TIFUSERDATA *)ptr;
     if (buf->version == TIFVERSION) {
		 if (!tifDefaults.saved && !UserData.saved)//special case.  what the scripter says takes prescendence
			 return TRUE;
        memcpy((void *)&UserData,ptr,sizeof(TIFUSERDATA));
		if (tifDefaults.saved)
			tifDefaults = UserData;
        return (TRUE);
     } else
        return (FALSE);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_TIF::SaveConfigure()

BOOL BitmapIO_TIF::SaveConfigure ( void *ptr ) {
     if (ptr) 
		{
		if (!tifDefaults.saved)//the global tifDefaults takes precedence over the local copy.
			{
			tifDefaults.saved = TRUE;
			memcpy(ptr, (void*)&tifDefaults, sizeof(TIFUSERDATA));
			UserData = tifDefaults;
			}
		else
			{
			UserData.saved = TRUE;
			memcpy(ptr,(void *)&UserData,sizeof(TIFUSERDATA));
			}
		return (TRUE);
		} 
	 else
        return (FALSE);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_TIF::EvaluateConfigure()

DWORD BitmapIO_TIF::EvaluateConfigure ( ) {
      return (sizeof(TIFUSERDATA));
}

//-----------------------------------------------------------------------------
// *> AboutCtrlDlgProc()
//

INT_PTR CALLBACK AboutCtrlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {

     switch (message) {
        
        case WM_INITDIALOG: 
             CenterWindow(hWnd,GetParent(hWnd));
             return 1;
        

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
// #> BitmapIO_TIF::ShowAbout()

void BitmapIO_TIF::ShowAbout(HWND hWnd) {
     DialogBoxParam(
        hInst,
        MAKEINTRESOURCE(IDD_ABOUT),
        hWnd,
        (DLGPROC)AboutCtrlDlgProc,
        (LPARAM)0);
}


//-----------------------------------------------------------------------------
// *> ControlCtrlDlgProc()
//

static ISpinnerControl* dpiSpinner = NULL;

BOOL BitmapIO_TIF::Control(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {

     static BOOL forced = FALSE;

     switch (message) {
        
        case WM_INITDIALOG: {
      
             CenterWindow(hWnd,GetParent(hWnd));
			 SetCursor(LoadCursor(NULL,IDC_ARROW));

             InitCommonControls();
             
             //if (!UserData.saved)
                //ReadCfg();

			 //Set initial compression state.
			 CheckDlgButton(hWnd,IDC_COMPRESSION_NONE,UserData.compressionType == tif_compress_none ? TRUE : FALSE); 
			 CheckDlgButton(hWnd,IDC_COMPRESSION_PACKBITS,UserData.compressionType == tif_compress_packbits ? TRUE : FALSE); 
#ifdef ALLOW_LZW_COMPRESSION
			 CheckDlgButton(hWnd,IDC_COMPRESSION_LZW,UserData.compressionType == tif_compress_lzw ? TRUE : FALSE ); 
#endif //ALLOW_LZW_COMPRESSION

			 //Writing type checking on init.
			 //We need to disable compressions based on the type....do this by sending another message.
			 if (UserData.writeType == tif_write_mono)
				 {
				 CheckDlgButton(hWnd,IDC_GREYSCALE, TRUE); 
				 SendMessage(hWnd, WM_COMMAND, IDC_GREYSCALE, 0);
				 }
			 else if (UserData.writeType == tif_write_color)
				 {
				 CheckDlgButton(hWnd, IDC_COLOR, TRUE);
				 SendMessage(hWnd, WM_COMMAND, IDC_COLOR, 0);
				 }
			 else if (UserData.writeType == tif_write_logl)
				 {
				 CheckDlgButton(hWnd, IDC_LOGL, TRUE);
				 SendMessage(hWnd, WM_COMMAND, IDC_LOGL, 0);
				 }
			 else if (UserData.writeType == tif_write_logluv)
				 {
				 CheckDlgButton(hWnd, IDC_LOGLUV, TRUE);
				 SendMessage(hWnd, WM_COMMAND, IDC_LOGLUV, 0);
				 }

			 CheckDlgButton(hWnd,IDC_SAVE_ALPHA, UserData.writeAlpha ? TRUE : FALSE);

			 dpiSpinner = SetupFloatSpinner(hWnd, IDC_DPI_SPIN, IDC_DPI, 0.0f, 12000.0f,
				 float(UserData.dpi), 10.0f);
             return 1;
             
        }

		case WM_DESTROY:
			 if (dpiSpinner != NULL) {
				 ReleaseISpinner(dpiSpinner);
				 dpiSpinner = NULL;
			 }
			 break;

        case WM_COMMAND:

             switch (LOWORD(wParam)) {
                
				 //We need to enable/disable compression types based on the type.
				 //if a radio button we are going to disable is checked, uncheck it.

				 case IDC_GREYSCALE:
					 {
					 EnableWindow( GetDlgItem(hWnd,IDC_COMPRESSION_NONE), TRUE); 
					 EnableWindow( GetDlgItem(hWnd,IDC_COMPRESSION_PACKBITS), TRUE ); 
#ifdef ALLOW_LZW_COMPRESSION
					 EnableWindow( GetDlgItem(hWnd,IDC_COMPRESSION_LZW), TRUE ); 
#endif //ALLOW_LZW_COMPRESSION

					 EnableWindow( GetDlgItem(hWnd,IDC_SAVE_ALPHA), FALSE);
					 }
					 break;
					 
				 case IDC_COLOR:
					 {

					 EnableWindow( GetDlgItem(hWnd,IDC_COMPRESSION_NONE), TRUE); 
					 EnableWindow( GetDlgItem(hWnd,IDC_COMPRESSION_PACKBITS), TRUE ); 
#ifdef ALLOW_LZW_COMPRESSION
					 EnableWindow( GetDlgItem(hWnd,IDC_COMPRESSION_LZW),TRUE ); 
#endif //ALLOW_LZW_COMPRESSION

					 EnableWindow( GetDlgItem(hWnd,IDC_SAVE_ALPHA), TRUE);
					 }
					 break;

				 case IDC_LOGL:
					 {
					 //Just check the "no compression" radio button
					 CheckDlgButton(hWnd, IDC_COMPRESSION_NONE, TRUE);
					 CheckDlgButton(hWnd, IDC_COMPRESSION_PACKBITS, FALSE);					 

					 EnableWindow( GetDlgItem(hWnd,IDC_COMPRESSION_NONE), FALSE ) ;
					 EnableWindow( GetDlgItem(hWnd,IDC_COMPRESSION_PACKBITS), FALSE ); 
#ifdef ALLOW_LZW_COMPRESSION
					 EnableWindow( GetDlgItem(hWnd,IDC_COMPRESSION_LZW),FALSE ); 
#endif //ALLOW_LZW_COMPRESSION

					 EnableWindow( GetDlgItem(hWnd,IDC_SAVE_ALPHA), FALSE);
					 }
					 break;
				 case IDC_LOGLUV:
					 {
					 //Just check the "no compression" radio button
					 CheckDlgButton(hWnd, IDC_COMPRESSION_NONE, TRUE);
					 CheckDlgButton(hWnd, IDC_COMPRESSION_PACKBITS, FALSE);

					 EnableWindow( GetDlgItem(hWnd,IDC_COMPRESSION_NONE), FALSE );
					 EnableWindow( GetDlgItem(hWnd,IDC_COMPRESSION_PACKBITS), FALSE ); 
#ifdef ALLOW_LZW_COMPRESSION
					 EnableWindow( GetDlgItem(hWnd,IDC_COMPRESSION_LZW), FALSE ); 
#endif //ALLOW_LZW_COMPRESSION

					 EnableWindow( GetDlgItem(hWnd,IDC_SAVE_ALPHA), FALSE);
					 }
					 break;
                case IDOK: {
					//Type takes priority over compression, so set it without checking compression type.

					 if(IsDlgButtonChecked(hWnd,IDC_GREYSCALE))
						UserData.writeType = tif_write_mono;
					 else if(IsDlgButtonChecked(hWnd,IDC_COLOR))
						UserData.writeType = tif_write_color;
					 else if (IsDlgButtonChecked(hWnd,IDC_LOGL))
						 UserData.writeType = tif_write_logl;
					 else if (IsDlgButtonChecked(hWnd, IDC_LOGLUV))
						 UserData.writeType = tif_write_logluv;

					 BYTE type = UserData.writeType;
					 //Only set alpha channel if we are writing color mode.
					 if (IsDlgButtonChecked(hWnd, IDC_SAVE_ALPHA)
						 && type == tif_write_color)
						 UserData.writeAlpha = TRUE;
					 else
						 UserData.writeAlpha = FALSE;

					 if (IsDlgButtonChecked(hWnd, IDC_COMPRESSION_NONE))//No problem
						UserData.compressionType = tif_compress_none;

					 else if(IsDlgButtonChecked(hWnd,IDC_COMPRESSION_PACKBITS))
						UserData.compressionType = tif_compress_packbits;

#ifdef ALLOW_LZW_COMPRESSION
					 else if (IsDlgButtonChecked(hWnd, IDC_COMPRESSION_LZW))
						 UserData.compressionType = tif_compress_lzw;
#endif //ALLOW_LZW_COMPRESSION

					 if (dpiSpinner != NULL)
						 UserData.dpi = dpiSpinner->GetFVal();

					tifDefaults = UserData;
  					WriteCfg();
                    EndDialog(hWnd,1);
					 }
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
// *> ControlDlgProc
//

static INT_PTR CALLBACK ControlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
     static BitmapIO_TIF *bm = NULL;
     if (message == WM_INITDIALOG) 
        bm = (BitmapIO_TIF *)lParam;
     if (bm) 
        return (bm->Control(hWnd,message,wParam,lParam));
     else
        return(FALSE);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_TIF::Control()

BOOL BitmapIO_TIF::ShowControl(HWND hWnd, DWORD flag ) 
	{
	if (!UserData.disableControl)
     return (
        DialogBoxParam(
        hInst,
        MAKEINTRESOURCE(IDD_TIF_CONTROL),
        hWnd,
        (DLGPROC)ControlDlgProc,
        (LPARAM)this)
     );
	else
		{
		return TRUE;
		}
}


//-----------------------------------------------------------------------------
// #> BitmapIO_TIF::GetImageInfo()

BMMRES BitmapIO_TIF::GetImageInfo ( BitmapInfo *fbi ) {
     
     //-- Get File Header
	try
		{
		tif=TIFFOpen(fbi->Name(), _T("r"));
		}
	catch(MAXException exc)
		{
		if (tif)
			TIFFClose(tif);
		tif = NULL;
		return ProcessImageIOError(fbi, BMMRES_BADFILEHEADER);
		}

     //tif=TIFFReadHdr(inStream);
     if(tif==NULL) {
		DebugPrint("TIFFOpen failed\n");
		return (ProcessImageIOError(fbi,BMMRES_BADFILEHEADER));
		}
     td = &tif->tif_dir;

     /* Fill in appropriate info slots */

     nsamp = td->td_samplesperpixel;
     fbi->SetWidth(td->td_imagewidth);
     fbi->SetHeight(td->td_imagelength);
     fbi->SetAspect(1.0f);
     //fbi->SetGamma(1.0f);
     fbi->SetFirstFrame(0);
	 fbi->SetStartFrame(0);
	
	tiff_error_off();
	try {
		fbi->SetLastFrame(TIFFNumberOfDirectories(tif)-1);
		fbi->SetEndFrame(TIFFNumberOfDirectories(tif)-1);
	} catch(...) {
		fbi->SetLastFrame(0);
		fbi->SetEndFrame(0);
	}
	tiff_error_on();
	 
	 int type = BMM_NO_TYPE;

     switch (td->td_photometric) {
     	case PHOTOMETRIC_MINISWHITE:  
		case PHOTOMETRIC_MINISBLACK:
			if (td->td_bitspersample == 1)
	     		type = BMM_LINE_ART;
			else if(td->td_bitspersample==16)
     			type = BMM_GRAY_16;
			else
     			type = BMM_GRAY_8;
     		break;
		//-- This thing was ignoring 16bit images and assuming a single alpha channel 
		// GG: 08/12/02
     	case PHOTOMETRIC_RGB:
			if(td->td_bitspersample==8)
				type = (td->td_samplesperpixel > 3) ? BMM_TRUE_32 : BMM_TRUE_24;
			else if(td->td_bitspersample==16)
				type = (td->td_samplesperpixel > 3) ? BMM_TRUE_64 : BMM_TRUE_48;
			else if(td->td_bitspersample==32)
				type = BMM_REALPIX_32;
			else
				type = BMM_NO_TYPE;
     		break;
     	case PHOTOMETRIC_PALETTE:
			type = BMM_PALETTED;
     		break;
		case PHOTOMETRIC_LOGL:/* High dynamic range luminance */
			type = BMM_LOGLUV_32;//We don't have a type for LogL, but this is a subset of LogLUV
			break;
		case PHOTOMETRIC_LOGLUV:
			type = BMM_LOGLUV_32;
			break;
     	}

     if(type == BMM_NO_TYPE) {
		DebugPrint("Unknown TIF type\n");
		return (ProcessImageIOError(fbi,GetString(IDS_UNKNOWN)));
		}

	 fbi->SetType(type);

     if (tif)
		 {
         TIFFClose(tif);
		 tif=NULL;
		 }
		
     return BMMRES_SUCCESS;
}


//-----------------------------------------------------------------------------
// #> BitmapIO_TIF::Load()

BitmapStorage *BitmapIO_TIF::Load(BitmapInfo *fbi, Bitmap *map, BMMRES *status) {

   //-- Initialize Status Optimistically

   *status = BMMRES_SUCCESS;

   //-- Make sure nothing weird is going on

   if(openMode != BMM_NOT_OPEN) {
      *status = ProcessImageIOError(fbi,BMMRES_INTERNALERROR);
      return NULL;
   }

   openMode = BMM_OPEN_R;
   loadMap  = map;
   BitmapStorage* s = NULL;
   try
   {
      s = ReadTIFFile(fbi, map->Manager(), status);
   }
   catch (MAXException exp)
   {
      if (tif)
        TIFFClose(tif);
      tif = NULL;
      *status = ProcessImageIOError(fbi, BMMRES_INTERNALERROR);
      return NULL;
   }

   if(!s)
      return NULL;

   // GG: 10/02/02 - Correction, this was bug: 312453. I'm leaving this
   // here because I'm not sure what the intent was. However, saving this
   // on load is not that meaningful unless the consumer has some hardcoded
   // knowledge about this structure.

   // bug fix 312452
   // need to set the UserData in the bitmap info so that it can be 
   // extracted on the other side

   fbi->AllocPiData(sizeof(UserData));
   if( fbi->GetPiData() != NULL)
      memcpy( fbi->GetPiData(), &UserData, sizeof(UserData));

   //-- Set the storage's BitmapInfo

   s->bi.CopyImageInfo(fbi);

   return s;
}

//-----------------------------------------------------------------------------
// *> BitmapIO_TIF::ReadTIFFile()
//
//    Load a TIF file, returning the storage location

BitmapStorage *BitmapIO_TIF::ReadTIFFile(BitmapInfo *fbi, BitmapManager *manager, BMMRES *status) {

     BitmapStorage *s = NULL;

	int res=1;

	tif=TIFFOpen(fbi->Name(), _T("r"));
	if (tif==NULL) {
		*status = ProcessImageIOError(fbi,BMMRES_BADFILEHEADER);
		return(0);
	}

	 //Multiple frames access...
	 fbi->SetFirstFrame(0);

	int nDirs;
	tiff_error_off();
	try {
		nDirs = TIFFNumberOfDirectories(tif);
	} catch (...) {
		nDirs = 1;
	}
	tiff_error_on();
	 
	 fbi->SetLastFrame(nDirs-1);

	 int dirOff = fbi->CurrentFrame();
	 if (TIFFSetDirectory(tif, dirOff))
		fbi->SetCurrentFrame(dirOff);
	 else
		 {
		 *status = ProcessImageIOError(fbi,BMMRES_BADFRAME);
		 return NULL;
		 }
	 td = &tif->tif_dir;

#ifdef GEOREFSYS_UVW_MAPPING
	 tif->tif_name = const_cast<char *>(fbi->Name());
	 GTIFF * gtif = GeoTIFFRead(tif, td);
	 if (gtif)
	 {
		GeoTableItem * data = new GeoTableItem;
		
		data->m_geoInfo = true;
//		GeoTIFFCoordSysName(gtif, UserData.name);
		data->m_matrix = GeoTIFFModelTransform(gtif);
		data->m_names.AddName(const_cast<TCHAR *>(fbi->Name()));
		GeoTIFFExtents(data, gtif);
		TheManager->Execute(0, (ULONG)(fbi->Name()), (ULONG)(data), 0);
	 }
#endif

 
	width = td->td_imagewidth;
	height = td->td_imagelength;
	fbi->SetWidth(width);
	fbi->SetHeight(height);
	fbi->SetAspect(1.0f);
	//fbi->SetGamma(1.0f);

	// if we're reading in logluv data, set the data format before allocating the 
	//decompression buffer to make sure it's the correct size.
	//use TIFFSetField to update dependencies...

	if (td->td_photometric == PHOTOMETRIC_LOGLUV || td->td_photometric == PHOTOMETRIC_LOGL)
		{
		//We have to reset samplesperpixel here if requesting the raw data - the Tiff 
		//library manipulates this value when we set the above line, so that it is 
		//expecting to give 32 bits as 16, 8, and 8, but really, we just want the raw data.
		TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1L);
		if (td->td_photometric == PHOTOMETRIC_LOGL)
			TIFFSetField(tif, TIFFTAG_SGILOGDATAFMT, SGILOGDATAFMT_FLOAT);//request a decompressed array of floats
		else
			TIFFSetField(tif, TIFFTAG_SGILOGDATAFMT, SGILOGDATAFMT_RAW);//this is significantly faster than requesting
																//data using *float* to avoid converting luv to xyz space.
		}

	//refresh dependent scanlinesize.
	//tif->tif_scanlinesize = TIFFScanlineSize(tif);

	nsamp = td->td_samplesperpixel;
	//-- Bad Assumption (GG: 08/12/02)
	//if (nsamp>4)
	//	nsamp = 4;		

	load_alpha = (nsamp > 3) ? TRUE : FALSE;


	/* allocate decompression buffer memory */ 
	PixelBuf8 lbuf(tif->tif_scanlinesize);
	loadbuf = lbuf.Ptr();

	 switch (td->td_photometric) {
     	case PHOTOMETRIC_MINISWHITE:
		case PHOTOMETRIC_MINISBLACK:
			if(td->td_bitspersample == 1) {
	     		fbi->SetType(BMM_LINE_ART); 	
		 		s = TifReadLineArt(fbi, manager); 
			} else {
				//-- Assumed if it ain't 1 it must be 8. What about 16? GG:08/12/02
				if (td->td_bitspersample == 16) {
     				fbi->SetType(BMM_GRAY_16);
					s = TifReadGrayScale16(fbi,manager); 
				} else {
					fbi->SetType(BMM_GRAY_8);
     				s = TifReadGrayScale(fbi, manager); 
				}
			}
     		break;
     	case PHOTOMETRIC_RGB:
			if (td->td_bitspersample == 8)
     			fbi->SetType(BMM_TRUE_32);
			else if (td->td_bitspersample == 16)
				fbi->SetType(BMM_TRUE_64);
			else
				fbi->SetType(BMM_NO_TYPE);
     		if (td->td_planarconfig==PLANARCONFIG_SEPARATE)
     			s = TifReadPlanarRGB(fbi, manager);
     		else 
     			s = TifReadChunkyRGB(fbi, manager);
     		break;
     	case PHOTOMETRIC_PALETTE:
			fbi->SetType(BMM_PALETTED);
     		s = TifReadColPal(fbi, manager); 
     		break;
		case PHOTOMETRIC_LOGL:
			fbi->SetType(BMM_LOGLUV_32);
			s = TifReadLogLUV(fbi, manager);
			break;
		case PHOTOMETRIC_LOGLUV:
			fbi->SetType(BMM_LOGLUV_32);
			s = TifReadLogLUV(fbi, manager);
			break;
     	}

#ifdef GEOREFSYS_UVW_MAPPING 
		 GeoTIFFClose(gtif);
#endif

	if (tif)
		{
		TIFFClose(tif);
		tif=NULL;
		}
     if(s)
        return s;

     //-- If we get here, something went wrong!

	*status = ProcessImageIOError(fbi,GetString(IDS_READ_ERROR));
     return NULL;

}

// Our TIF loader functions:

BitmapStorage *BitmapIO_TIF::TifReadLineArt(BitmapInfo *fbi, BitmapManager *manager) {
 
     //-- Create a storage for this bitmap ------------------------------------

     BitmapStorage *s = BMMCreateStorage(manager,BMM_LINE_ART);

     if(!s)
		 return NULL;

     if (s->Allocate(fbi,manager,BMM_OPEN_R)==0) {
        bail_out:
        if(s) {
           delete s;
           s = NULL;
        }
        return NULL;
     }

	uchar byte;
	int nbyte,bytesperrow,x,y,photo,bps,nmax;
	
	bytesperrow = tif->tif_scanlinesize;
	photo = td->td_photometric;
	bps = td->td_bitspersample;
	if (bps!=1) {
		DebugPrint("Bad bits per sample\n");
		goto bail_out;
		}

	nmax = (1<<bps)-1;

    PixelBuf line64(width);

	for (y = 0; y<height; y++)  {
		if (TIFFReadScanline(tif, loadbuf, y, 0)<0) 
			goto bail_out;
        BMM_Color_64 *l64=line64.Ptr();
		x = 0;
		for (nbyte=0; nbyte<bytesperrow; nbyte++) {
			unsigned char mask;
			unsigned int c;
			int nsh;
			byte = loadbuf[nbyte];
			mask = nmax<<(8-bps);
			for (nsh=8-bps; nsh>=0; nsh-=bps, mask>>=bps, ++l64) {
				c = (byte&mask)>>nsh;  
				c = (c*255)/nmax;
				if (photo==0) c = 255-c;
				l64->r = l64->g = l64->b = c ? 0xffff : 0;
				l64->a = 0;
				if (++x == width) break;
				}
			}
        if(s->PutPixels(0,y,width,line64.Ptr())!=1)
            goto bail_out;
		}
	return s;
	}

BitmapStorage *BitmapIO_TIF::TifReadGrayScale(BitmapInfo *fbi, BitmapManager *manager) {
     
	 //-- Create a storage for this bitmap ------------------------------------

     BitmapStorage *s = BMMCreateStorage(manager,BMM_GRAY_8);

     if(!s)
		 return NULL;

     if (s->Allocate(fbi,manager,BMM_OPEN_R)==0) {
        bail_out:
        if(s) {
           delete s;
           s = NULL;
        }
        return NULL;
     }

	uchar byte;
	int nbyte,bytesperrow,x,y,photo,bps,nmax;
	
	bytesperrow = tif->tif_scanlinesize;
	photo = td->td_photometric;
	bps = td->td_bitspersample;
	if (bps>8||bps==3||bps==5||bps==7) {
		DebugPrint("Bad bits per sample\n");
		goto bail_out;
		}

	nmax = (1<<bps)-1;

    PixelBuf line64(width);

	int accum = 0;
	int acc_ix = 0;

	for (y = 0; y<height; y++)  {
		if (TIFFReadScanline(tif, loadbuf, y, 0)<0) 
			goto bail_out;
        BMM_Color_64 *l64=line64.Ptr();
		x = 0;
		for (nbyte=0; nbyte<bytesperrow; nbyte++) {
			unsigned char mask;
			unsigned int c;
			int nsh;
			byte = loadbuf[nbyte];

			mask = nmax<<(8-bps);
			for (nsh=8-bps; nsh>=0; nsh-=bps, mask>>=bps) {
				c = (byte&mask)>>nsh;  
				c = (c*255)/nmax;
				if (photo==0) c = 255-c;
				accum += c;
				acc_ix++;
				if(acc_ix == td->td_samplesperpixel) {
					l64->r = l64->g = l64->b = (WORD)accum << 8;
					l64->a = 0;
					l64++;
					x++;
					accum = acc_ix = 0;
					}
				if (x >= width) break;
				}
			if (x >= width) break;
			}
        if(s->PutPixels(0,y,width,line64.Ptr())!=1)
            goto bail_out;
		}
	return s;
	}

//-----------------------------------------------------------------------------
// BitmapIO_TIF::TifReadGrayScale16()
//
// Reads 16bit Grayscale Images. GG: 08/12/02

BitmapStorage *BitmapIO_TIF::TifReadGrayScale16(BitmapInfo *fbi, BitmapManager *manager) {
	BitmapStorage *s = BMMCreateStorage(manager,BMM_GRAY_16);
	if(!s)
		return NULL;
	if (s->Allocate(fbi,manager,BMM_OPEN_R)==0) {
		bail_out:
		if(s) {
			delete s;
			s = NULL;
		}
		return NULL;
	}
	for (int y = 0; y < height; y++)  {
		if (TIFFReadScanline(tif,loadbuf,y,0)<0) 
			goto bail_out;
		if(s->Put16Gray(0,y,width,(WORD*)loadbuf)!=1)
			goto bail_out;
	}
	return s;
}


static void PutColorComponent(BMM_Color_64 *ptr, int component, int width, BYTE *data) {
	UWORD *work = ((UWORD *)ptr) + component;
	for(int i = 0; i < width; ++i, work+=4, data++)
		*work = *data << 8;
	}

BitmapStorage *BitmapIO_TIF::TifReadPlanarRGB(BitmapInfo *fbi, BitmapManager *manager) {

	 //-- Create a storage for this bitmap ------------------------------------

     BitmapStorage *s = BMMCreateStorage(manager, BMM_TRUE_32);

     if(!s)
        return NULL;

     if(nsamp > 3)
        fbi->SetFlags(MAP_HAS_ALPHA);

     if (s->Allocate(fbi,manager,BMM_OPEN_R)==0) {
        bail_out:
        if(s) {
           delete s;
           s = NULL;
        }
        return NULL;
     }

    PixelBuf line64(width);

	int y,isamp;

	//-- Very explicitly... GG: 08/12/02
	int clamp_alpha = nsamp;
	if (clamp_alpha > 4)
		clamp_alpha = 4;

	//-- Could this be even more inefficient?
	for (isamp=0; isamp<clamp_alpha; isamp++) {
		for (y = 0; y<height; y++)  {
			if (TIFFReadScanline(tif, loadbuf, y, isamp)<0)
				goto bail_out;
			s->GetPixels(0,y,width,line64.Ptr());			
			PutColorComponent(line64.Ptr(), isamp, width, loadbuf);
			if(s->PutPixels(0,y,width,line64.Ptr())!=1)
				goto bail_out;
			}
		}
	return s;
	}

BitmapStorage *BitmapIO_TIF::TifReadChunkyRGB(BitmapInfo *fbi, BitmapManager *manager) {

	 //-- Create a storage for this bitmap ------------------------------------

	BitmapStorage *s;
	 
	int bps = td->td_bitspersample;

	if(bps==32)
		s = BMMCreateStorage(manager, BMM_LOGLUV_32);
	else if(bps==16)
		s = BMMCreateStorage(manager, BMM_TRUE_64);
	else
		s = BMMCreateStorage(manager, BMM_TRUE_32);

	if(!s)
		return NULL;

	if(nsamp > 3)
		fbi->SetFlags(MAP_HAS_ALPHA);

	if (s->Allocate(fbi,manager,BMM_OPEN_R)==0) {
		bail_out:
		if(s) {
			delete s;
			s = NULL;
		}
		return NULL;
	}


	//-- Loading 32 bit float RGB values
    if(bps==32)	{
		BMM_Color_fl *line = new BMM_Color_fl[width];
		float *scanptr;
		int x,y;
		for (y = 0; y<height; y++)  {
			if (TIFFReadScanline(tif, loadbuf, y, 0)<0)
			{
				delete [] line;
				goto bail_out;
			}
			BMM_Color_fl *l=line;
			scanptr = (float*)loadbuf;
			for(x = 0; x < width; ++x, ++l) {
				l->r = *scanptr++;
				l->g = *scanptr++;
				l->b = *scanptr++;
				if(nsamp > 3) {
					//-- Load First Alpha channel (assumption here again) GG: 08/12/02
					l->a = *scanptr;
					scanptr += nsamp - 3;
				} else
					l->a = 0;
			}

			if(s->PutPixels(0,y,width,line)!=1) {
				delete [] line;
				goto bail_out;
			}

		}
		delete [] line;
	//-- Loading 16 bit RGB values GG: 08/12/02
	} else if (bps==16)	{
		PixelBuf line64(width);
		WORD *scanptr;
		int x,y;
		for (y = 0; y<height; y++)  {
			if (TIFFReadScanline(tif, loadbuf, y, 0)<0)
				goto bail_out;
			BMM_Color_64 *l64=line64.Ptr();
			scanptr = (WORD*)loadbuf;
			for(x = 0; x < width; ++x, ++l64) {
				l64->r = *scanptr++;
				l64->g = *scanptr++;
				l64->b = *scanptr++;
				if(nsamp > 3) {
					l64->a = *scanptr;
					scanptr += nsamp - 3;
				} else
					l64->a = 0;
				}
			if(s->PutPixels(0,y,width,line64.Ptr())!=1)
				goto bail_out;
			}
	//-- Loading 8 bit RGB values
	} else {
		PixelBuf line64(width);
		uchar *scanptr;
		int x,y;
		for (y = 0; y<height; y++)  {
			if (TIFFReadScanline(tif, loadbuf, y, 0)<0)
				goto bail_out;
			BMM_Color_64 *l64=line64.Ptr();
			scanptr = loadbuf;
			for(x = 0; x < width; ++x, ++l64) {
				l64->r = *scanptr++ << 8;
				l64->g = *scanptr++ << 8;
				l64->b = *scanptr++ << 8;
				if(nsamp > 3) {
					l64->a = *scanptr << 8;
					scanptr += nsamp - 3;
				} else
					l64->a = 0;
				}
			if(s->PutPixels(0,y,width,line64.Ptr())!=1)
				goto bail_out;
			}
	}

	return s;
	}

void BitmapIO_TIF::ScrunchColorMap(BMM_Color_48 *colpal) {
	int npal,i;
	npal = 1<<td->td_bitspersample;
	for (i=0; i<npal; i++) {
		colpal[i].r = td->td_colormap[0][i];
		colpal[i].g = td->td_colormap[1][i];
		colpal[i].b = td->td_colormap[2][i];
		}
	}

BitmapStorage *BitmapIO_TIF::TifReadColPal(BitmapInfo *fbi, BitmapManager *manager) {
	 //-- Create a storage for this bitmap ------------------------------------

     BitmapStorage *s = BMMCreateStorage(manager, BMM_PALETTED);

     if(!s)
        return NULL;

     if (s->Allocate(fbi,manager,BMM_OPEN_R)==0) {
        bail_out:
        if(s) {
           delete s;
           s = NULL;
        }
        return NULL;
     }

    PixelBuf8 line8(width);

	int x,y,bps,shft;
	BMM_Color_48 cmap[256];
	uchar byte,mask,*pbuf;

	bps = td->td_bitspersample;
	mask = (1<<bps)-1;
	ScrunchColorMap(cmap);
	s->SetPalette(0, 256, cmap);
	for (y = 0; y<height; y++)  {
		if (TIFFReadScanline(tif, loadbuf, y, 0)<0)
			goto bail_out;
		if (bps==4||bps==2||bps==1) {
			shft = -1;
			pbuf = loadbuf;
			for (x=0; x<width; x++) {
				if (shft<0) {
					byte = *pbuf++;
					shft = 8-bps;
					}
				line8[x] = (byte>>shft)&mask;
				shft-=bps;
				if (shft<0) {
					byte = *pbuf++;
					shft = 8-bps;
					}
				}
			if(s->PutIndexPixels(0,y,width,line8.Ptr())!=1)
				goto bail_out;
			}
		else {
			if(s->PutIndexPixels(0,y,width,loadbuf)!=1)	// 8 bits -- just use load buffer
				goto bail_out;
			}
		}
	return s;
	}


BitmapStorage* BitmapIO_TIF::TifReadLogLUV(BitmapInfo* fbi, BitmapManager* manager)
	{
	//-- Create a storage for this bitmap ------------------------------------

     BitmapStorage *s = BMMCreateStorage(manager, BMM_LOGLUV_32);
	 
     if(!s)
        return NULL;

     if (s->Allocate(fbi,manager,BMM_OPEN_R)==0) {
        bail_out:
        if(s) {
           delete s;
           s = NULL;
        }
        return NULL;
     }

	 //'factor' is the physically - based units scale to scale these luminance values to
	 //physically based units - candelas per metre squared
	 //We still have to do something with this number...
	 double factor = 0;
	 if (!TIFFGetField(tif, TIFFTAG_STONITS, &factor) == 1)
		 {
		 factor = 1.0;
		 }

    // add the read stonits to the UserData
    // bug 312453 - David Cunningham Nov. 7, 2001
    UserData.lumStonits = factor;

	int y,type=0;

	for (y = 0; y<height; y++)  
		{
		if (TIFFReadScanline(tif, loadbuf, y, 0)<0)
			goto bail_out;

		
		if (td->td_photometric == PHOTOMETRIC_LOGL)
			{
			if (s && s->GetStoragePtr(&type) && type == BMM_LOGLUV_32)//this should always 
				//be true, after all, we allocated it!

				//luv = (float*)line.Ptr();
				//we could play with exposure here - because of the high dynamic range, images
				//often look washed out because of clamping at white.  We can adjust this to display 
				//images in different ranges.
				if(s->Put16Gray(0,y,width,(float*)loadbuf)!=1)
					goto bail_out;
			}
		else if (td->td_photometric == PHOTOMETRIC_LOGLUV)
			{
			if (s && s->GetStoragePtr(&type) && type == BMM_LOGLUV_32)
				{
				LogLUV32Pixel* pixel = (LogLUV32Pixel*)s->GetStoragePtr(&type);
				pixel += y*width;
				LogLUV32Pixel* iter = (LogLUV32Pixel*)loadbuf;
				memcpy(pixel, iter, width*sizeof(LogLUV32Pixel));//32 bits per pixel here.  4 bytes.
				}
			else
				goto bail_out;
			}
		else
			TIFFError(GetString(IDS_TIF_READ_ERR), GetString(IDS_TIFF_FORMAT_NO_SUPPORT));
		}
	return s;
	}
//-----------------------------------------------------------------------------
// #> BitmapIO_TIF::OpenOutput()

BMMRES BitmapIO_TIF::OpenOutput(BitmapInfo *fbi, Bitmap *map) {

	if (openMode != BMM_NOT_OPEN)
		return (ProcessImageIOError(fbi,BMMRES_INTERNALERROR));
		
	if (!map)
		return (ProcessImageIOError(fbi,BMMRES_INTERNALERROR));
		
	//-- Check for Default Configuration -----------------
	
	if (!UserData.saved)
		UserData = tifDefaults;
	
    //-- Save Image Info Data

    bi.CopyImageInfo(fbi);    

    this->map   = map;
    openMode    = BMM_OPEN_W;

    return BMMRES_SUCCESS;

}

//-----------------------------------------------------------------------------
// #> BitmapIO_TIF::Write()
//

BMMRES BitmapIO_TIF::Write(int frame) {
     
	//-- If we haven't gone through an OpenOutput(), leave

	if (openMode != BMM_OPEN_W)
		return (ProcessImageIOError(&bi,BMMRES_INTERNALERROR));

	//-- Resolve Filename --------------------------------

	// Write a multiframe TIF file if UserData.lightActive has been set,
	// to support the Lighting Data Export utility. CA - 312242
	bool multiFrame = UserData.lightActive && frame != BMM_SINGLEFRAME;	// CA - 312242
	if (frame == BMM_SINGLEFRAME || UserData.lightActive) {
		_tcscpy(fileName,bi.Name());
	} else {	// CA - 312242
		// If lightActive is not set, then we will not do multi-frame
		// tiff files.
		if (!BMMCreateNumberedFilename(bi.Name(),frame,fileName))
			return (ProcessImageIOError(&bi,BMMRES_NUMBEREDFILENAMEERROR));
	}

    //-- Create Image File -------------------------------
	if (!multiFrame && tif != NULL) {		// CA - 312242
		// If we aren't using multiframe, then close any open file
		TIFFClose(tif);
		tif = NULL;
	}

	// Open the file, if it isn't open
	if (!tif) {			// CA - 312242
		try {
			tif = TIFFOpen(fileName, _T("w"));
		} catch (MAXException exp) {
			if (tif)
				TIFFClose(tif);
			tif=NULL;
			return ProcessImageIOError(&bi, BMMRES_CANTSTORAGE);
		}
		if (!tif)
			return ProcessImageIOError(&bi, BMMRES_IOERROR);
	}

	if (tif) {			// CA - 312242
		if (tif->tif_name == NULL) {
			tif->tif_name = fileName;
		}

		// Make sure the frames are sequential
		if (multiFrame) {
			tdir_t curDir = TIFFCurrentDirectory(tif);
			if (curDir == (unsigned short)(frame - 1) )//sequential.  This is fine.
				//flush out previous frames
				TIFFFlush(tif);
			else if (curDir != 65535)
				return ProcessImageIOError(&bi, BMMRES_NUMBEREDFILENAMEERROR);
		}
	}

	int result;
	try {
		result = SaveTIF();
		if (!multiFrame && tif != NULL) {		// CA - 312242
			TIFFClose(tif);
			tif = NULL;
		}
	} catch (MAXException exp) {
        if (tif)
		    TIFFClose(tif);
		tif=NULL;
		return ProcessImageIOError(&bi, BMMRES_INTERNALERROR);
	}
	

	switch(result) {
		case TIF_SAVE_OK:
			return BMMRES_SUCCESS;
		case TIF_SAVE_WRITE_ERROR:
		default:
			return (ProcessImageIOError(&bi,GetString(IDS_WRITE_ERROR)));
	}
}

/** If this is 1, does horizontal differencing before LZW encode **/
int  hordif = 0; 


/* fill in the blanks in the tiff header */

void
BitmapIO_TIF::MakeTiffhead() {

	if (!tif)
		return;

	TIFFCreateDirectory(tif);

	td = &tif->tif_dir;
    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, (uint32)width);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, (uint32)height);

	TIFFSetField(tif, TIFFTAG_XRESOLUTION, (dblparam_t) UserData.dpi);
	TIFFSetField(tif, TIFFTAG_YRESOLUTION, (dblparam_t) UserData.dpi);
	TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, (uint32) RESUNIT_INCH);

    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, rps);

	//No tiles
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    // Bit Fill Order => MSB to LSB. DO NOT CHANGE for the sake of COMPATIBILITY with other software.
    TIFFSetField(tif, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);    
    TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	//TIFFSetField(tif, TIFFTAG_SUBFILETYPE, 0x00);

	//This might be somewhat useful to have - Flag the file with the software that wrote it.
	if (UserData.lightActive)
		{
		TIFFSetField(tif, TIFFTAG_SOFTWARE, _T("Shine Lighting Analysis"));
		UserData.compressionType = COMPRESSION_NONE;
		UserData.writeType = tif_write_logluv;
		}
	else
#ifdef GEOREFSYS_UVW_MAPPING 
		TIFFSetField(tif, TIFFTAG_SOFTWARE, _T("Autodesk VIZ"));
#else
		TIFFSetField(tif, TIFFTAG_SOFTWARE, _T("3ds MAX"));
#endif

	//Set the compression tag... will be this unless LogL or LogLUV is chosen
	//The dialog should have prevented incompatible entries here.
	int compressionTag = COMPRESSION_NONE;
	switch (UserData.compressionType)
		{
		case tif_compress_none:
			compressionTag = COMPRESSION_NONE;
			break;
#ifdef ALLOW_LZW_COMPRESSION
		case tif_compress_lzw:
			compressionTag = COMPRESSION_LZW;
			break;
#endif
		case tif_compress_packbits:
			compressionTag = COMPRESSION_PACKBITS;
			break;
		default:
			compressionTag = COMPRESSION_NONE;
			break;
		}
	
	TIFFSetField(tif, TIFFTAG_COMPRESSION, compressionTag);

	if (UserData.writeType == tif_write_color)				/* color tiff */
		{
        if (write_alpha)
			{
            TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8L, 8L, 8L, 8L);
			TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 4L);
			}
        else
			{
            TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8L, 8L, 8L);
			TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3L);
			}
        TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
		}
	else if (UserData.writeType == tif_write_mono)			/* greyscale tiff */
		{
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8L);
        TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
		TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1L);
		}
	else if (UserData.writeType == tif_write_logl)			/* sgi log luminance */
		{
		TIFFSetField(tif,TIFFTAG_COMPRESSION, COMPRESSION_SGILOG);
		TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_LOGL);
		TIFFSetField(tif, TIFFTAG_SGILOGDATAFMT, SGILOGDATAFMT_FLOAT);
		TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1L);
		//And take care of lighting analysis properties.
		if (UserData.lightActive)
			{
			if (UserData.lumStonits >= 0)/* if known */
				TIFFSetField(tif, TIFFTAG_STONITS, (double)UserData.lumStonits);   
			}
		}
	else if (UserData.writeType == tif_write_logluv)		/* sgi log luminance + chromaticity */
		{
		TIFFSetField(tif,TIFFTAG_COMPRESSION, COMPRESSION_SGILOG);
		TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_LOGLUV);
		TIFFSetField(tif, TIFFTAG_SGILOGDATAFMT, SGILOGDATAFMT_FLOAT);
		TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3L);
		if (UserData.lightActive)
			{
			if (UserData.lumStonits > 0)/* if known */
				TIFFSetField(tif, TIFFTAG_STONITS, (double)UserData.lumStonits);   
			}
		}
	return;
	}

/*-----------------------------------------------------------
 * LZW Encoding.
-------------------------------------------------------------*/

#ifdef ALLOW_LZW_COMPRESSION

#define	 MAXCODE(n)			((1 << (n)) - 1)
#define	SetMaxCode(v) {	lzw_state.lzw_maxcode = (v)-1; }

/* Reset encoding state at the start of a strip. */
int BitmapIO_TIF::LZWPreEncode(void) {
	lzw_state.lzw_flags = 0;
	lzw_state.lzw_hordiff = hordif?LZW_HORDIFF8:0;
	lzw_state.lzw_stride = spp;
	lzw_state.enc_ratio = 0;
	lzw_state.enc_checkpoint = CHECK_GAP;
	lzw_state.lzw_nbits = BITS_MIN;
	SetMaxCode(MAXCODE(BITS_MIN)+1);
	lzw_state.lzw_free_ent = CODE_FIRST;
	lzw_state.lzw_bitoff = 0;
	lzw_state.lzw_bitsize = (BLOCKSIZE << 3) - (BITS_MAX-1);
	ClearHash();		/* clear hash table */
	lzw_state.lzw_oldcode = -1;	/* generates CODE_CLEAR in LZWEncode */
	return (1);
	}

#define REPEAT4(n, op)		\
    switch (n) {		\
    default: { int i; for (i = n-4; i > 0; i--) { op; } } \
    case 4:  op;		\
    case 3:  op;		\
    case 2:  op;		\
    case 1:  op;		\
    case 0:  ;			\
    }


static void
horizontalDifference8(register char *cp, register int cc, register int stride)
	{
	if (cc > stride) {
		cc -= stride;
		cp += cc - 1;
		do {
			REPEAT4(stride, cp[stride] -= cp[0]; cp--)
			cc -= stride;
			} while (cc > 0);
		}
	}

static void
horizontalDifference16(register short *wp, register int wc, register int stride)
	{
	if (wc > stride) {
		wc -= stride;
		wp += wc - 1;
		do {
			REPEAT4(stride, wp[stride] -= wp[0]; wp--)
			wc -= stride;
			} while (wc > 0);
		}
	}

/* Encode a scanline of pixels.
 * Uses an open addressing double hashing (no chaining) on the 
 * prefix code/next character combination.  We do a variant of
 * Knuth's algorithm D (vol. 3, sec. 6.4) along with G. Knott's
 * relatively-prime secondary probe.  Here, the modular division
 * first probe is gives way to a faster exclusive-or manipulation. 
 * Also do block compression with an adaptive reset, whereby the
 * code table is cleared when the compression ratio decreases,
 * but after the table fills.  The variable-length output codes
 * are re-sized at this point, and a CODE_CLEAR is generated
 * for the decoder. 
 */
int BitmapIO_TIF::LZWEncode(uchar *bp,	int cc )	{
	register long fcode;
	register int h, c, ent, disp;

	/* XXX horizontal differencing alters user's data XXX */
	switch (lzw_state.lzw_hordiff) {
		case LZW_HORDIFF8:
			horizontalDifference8((char *)bp, cc, (int)lzw_state.lzw_stride);
			break;
		case LZW_HORDIFF16:
			horizontalDifference16((short *)bp, cc/2, (int)lzw_state.lzw_stride);
			break;
		}

	ent = lzw_state.lzw_oldcode;
	if (ent == -1 && cc > 0) {
		PutNextCode( CODE_CLEAR);
		ent = *bp++; cc--; lzw_state.enc_incount++;
		}
	while (cc > 0) {
		c = *bp++; cc--; lzw_state.enc_incount++;
		fcode = ((long)c << BITS_MAX) + ent;
		h = (c << HSHIFT) ^ ent;	/* xor hashing */
		if (lzw_state.enc_htab[h] == fcode) {
			ent = lzw_state.enc_codetab[h];
			continue;
			}
		if (lzw_state.enc_htab[h] >= 0) {
			/*
			 * Primary hash failed, check secondary hash.
			 */
			disp = HSIZE - h;
			if (h == 0)
				disp = 1;
			do {
				if ((h -= disp) < 0)
					h += HSIZE;
				if (lzw_state.enc_htab[h] == fcode) {
					ent = lzw_state.enc_codetab[h];
					goto hit;
					}
				} while (lzw_state.enc_htab[h] >= 0);
			}
		/*
		 * New entry, emit code and add to table.
		 */
		PutNextCode(ent);
		ent = c;
		
		lzw_state.enc_codetab[h] = lzw_state.lzw_free_ent++;
		lzw_state.enc_htab[h] = fcode;
		if (lzw_state.lzw_free_ent == CODE_MAX-1) {
			/* table is full, emit clear code and reset */
			lzw_state.enc_ratio = 0;
			ClearHash();
			lzw_state.lzw_free_ent = CODE_FIRST;
			PutNextCode(CODE_CLEAR);
			SetMaxCode(MAXCODE(lzw_state.lzw_nbits = BITS_MIN)+1);
		} else {
			if (lzw_state.enc_incount >= lzw_state.enc_checkpoint)
				ClearBlock();
			/*
			 * If the next entry is going to be too big for
			 * the code size, then increase it, if possible.
			 */
			if (lzw_state.lzw_free_ent > lzw_state.lzw_maxcode) {
				lzw_state.lzw_nbits++;
				SetMaxCode(MAXCODE(lzw_state.lzw_nbits)+1);
				}
			}
		hit:
			;
		}
	lzw_state.lzw_oldcode = ent;
	return (1);
	}

/*
 * Finish off an encoded strip by flushing the last
 * string and tacking on an End Of Information code.
 */
int BitmapIO_TIF::LZWPostEncode(void){
	if (lzw_state.lzw_oldcode != -1)
		PutNextCode(lzw_state.lzw_oldcode);
	PutNextCode(CODE_EOI);
	return (1);
	}

void BitmapIO_TIF::PutNextCode(int c)	{
	register long r_off;
	register int bits, code = c;
	register uchar *bp;

	r_off = lzw_state.lzw_bitoff;
	bits = lzw_state.lzw_nbits;
 	/* Get to the first byte. */
	bp = comp_buf + (r_off >> 3);
	r_off &= 7;
	/*
	 * Note that lzw_bitoff is maintained as the bit offset
	 * into the buffer w/ a right-to-left orientation (i.e.
	 * lsb-to-msb).  The bits, however, go in the file in
	 * an msb-to-lsb order.
	 */
	bits -= (8 - r_off);
	*bp = (*bp & lmask[r_off]) | (code >> bits);
	bp++;
	if (bits >= 8) {
		bits -= 8;
		*bp++ = code >> bits;
		}
	if (bits)
		*bp = (code & rmask[bits]) << (8 - bits);
	/*
	 * enc_outcount is used by the compression analysis machinery
	 * which resets the compression tables when the compression
	 * ratio goes up.  lzw_bitoff is used here (in PutNextCode) for
	 * inserting codes into the output buffer.  tif_rawcc must
	 * be updated for the mainline write code in TIFFWriteScanline()
	 * so that data is flushed when the end of a strip is reached.
	 * Note that the latter is rounded up to ensure that a non-zero
	 * byte count is present. 
	 */
	lzw_state.enc_outcount += lzw_state.lzw_nbits;
	lzw_state.lzw_bitoff += lzw_state.lzw_nbits;
	rawcc = (lzw_state.lzw_bitoff + 7) >> 3;
	}

/*
 * Check compression ratio and, if things seem to
 * be slipping, clear the hash table and reset state.
 */

void BitmapIO_TIF::ClearBlock(void) {
	register long rat;

	lzw_state.enc_checkpoint = lzw_state.enc_incount + CHECK_GAP;
	if (lzw_state.enc_incount > 0x007fffff) {	/* shift will overflow */
		rat = lzw_state.enc_outcount >> 8;
		rat = (rat == 0 ? 0x7fffffff : lzw_state.enc_incount / rat);
		} 
	else
		rat = (lzw_state.enc_incount << 8) / lzw_state.enc_outcount; /* 8 fract bits */
	if (rat <= lzw_state.enc_ratio) {
		lzw_state.enc_ratio = 0;
		ClearHash();
		lzw_state.lzw_free_ent = CODE_FIRST;
		PutNextCode(CODE_CLEAR);
		SetMaxCode(MAXCODE(lzw_state.lzw_nbits = BITS_MIN)+1);
		} 
	else
		lzw_state.enc_ratio = rat;
	}

/*  Reset code table. */

void BitmapIO_TIF::ClearHash() {
	register int *htab_p = lzw_state.enc_htab+HSIZE;
	register long i, m1 = -1;

	i = HSIZE - 16;
 	do {
		*(htab_p-16) = m1;
		*(htab_p-15) = m1;
		*(htab_p-14) = m1;
		*(htab_p-13) = m1;
		*(htab_p-12) = m1;
		*(htab_p-11) = m1;
		*(htab_p-10) = m1;
		*(htab_p-9) = m1;
		*(htab_p-8) = m1;
		*(htab_p-7) = m1;
		*(htab_p-6) = m1;
		*(htab_p-5) = m1;
		*(htab_p-4) = m1;
		*(htab_p-3) = m1;
		*(htab_p-2) = m1;
		*(htab_p-1) = m1;
		htab_p -= 16;
		} while ((i -= 16) >= 0);
   for (i += 16; i > 0; i--)
		*--htab_p = m1;
	}
#endif //ALLOW_LZW_COMPRESSION

BOOL BitmapIO_TIF::WriteTIF(FILE *stream) {
	
	long packw;

    WORD y, j, k, l;
	unsigned long grey;
	int nstrip = 0;

	y = 0;

	do {
		rawcc = 0;
#ifdef ALLOW_LZW_COMPRESSION
		if (UserData.compressed)
			LZWPreEncode();
#endif //ALLOW_LZW_COMPRESSION
		for (j=0; j<rps; j++) {

			/* remove the desired sample */
			if (UserData.writeType == tif_write_color) {				/* color tiff */
				/* get a row and prepare for encoding */
				GetOutputPixels(0,y,width,scanline); // Get gamma-corrected pixels
				int index = 0;
				scanptr = scanline;
				for(k = 0; k < width; k++, ++scanptr) {
					WORD *pixel = (WORD *)scanptr;
					for(l = 0; l < spp; ++l)
						shortstrip[index++] = (BYTE)(pixel[l] >> 8);
					}
				packw = width*spp;
				}
			else if (UserData.writeType == tif_write_mono)
				{									/* grey scale tiff */
				/* get a row and prepare for encoding */
				GetOutputPixels(0,y,width,scanline); // Get gamma-corrected pixels
				scanptr = scanline;
				for (k=0; k<width; k++, ++scanptr) {
					grey = (unsigned long)scanptr->r * 30L;		/* 30% red */
					grey+= (unsigned long)scanptr->g * 59L;		/* 59% green */
					grey+= (unsigned long)scanptr->b * 11L;		/* 11% blue */
					grey/= 100L;
					shortstrip[k] = (BYTE)(grey >> 8);
					}
				packw = width;
				}
			else if (UserData.writeType == tif_write_logl)		/*Log-Luminance type tiff*/
				{
				//as long as we have set the tag TIFFTAG_SGILOGDATAFMT to realize float format - 
				//and the tiff writer knows we're using logL, so we feed it an array of floats
				//for each row, calculated from the BMM_Color_fl we have from GetLinearPixels().
				if (Storage()->IsHighDynamicRange())
					{
					int type = 0;
					if (Storage() && Storage()->GetStoragePtr(&type) && type == BMM_LOGLUV_32)
						{
						LogLUV32Pixel* pixel = (LogLUV32Pixel*)Storage()->GetStoragePtr(&type);
						pixel += y*width;
						//we're writing in float format, so we have to convert each pixel's luminance to float.
						float* lum = (float*)shortstrip;
						for (k=0; k<width; k++, pixel++)
							{
							lum[k] = (float)pix16toY((int)pixel->value >> 16);
							}
						}
					}
				else
					{
					/* get a row and prepare for encoding */
					GetOutputPixels(0,y,width,scanline); // Get gamma-corrected pixels
					scanptr = scanline;
					float* lum = (float*)shortstrip;//cast to float
					for (k=0; k<width; k++, ++scanptr) 
						{
						static LogLUV24Pixel converter;
						BMM_Color_64 rgb = *scanptr;
						Color rgbColor (rgb);//construct float color struct from BMM_Color_64
						Color xyz;
						converter.RGBtoXYZ(rgbColor, xyz);//convert rgb values to adjust for human sensitivity.
						lum[k] = (xyz.r + xyz.g + xyz.b)/3;//uh... integrate? to get grey value from xyz coordinates
						}
					}
				}
			else if (UserData.writeType == tif_write_logluv)
				{
				
				if (Storage()->IsHighDynamicRange())
					//the header should have already been set to SGILOGDATAFMT_RAW, so that we can just 
					//grab the data storage of the bitmap, and feed it to the tiffwriter, row by row.
				
					{
					Storage()->GetPixels(0, y, width, (BMM_Color_fl*)scanline); //no gamma correction, HDR from LogLUV storage.
					BMM_Color_fl* scanPtrHDR = (BMM_Color_fl*)scanline;
					Color* lum = (Color*)shortstrip;
					for (k=0; k<width; k++, scanPtrHDR++)
						{
						static LogLUV32Pixel converter;
						Color xyz;
						converter.RGBtoXYZ(*scanPtrHDR, xyz);
						lum[k] = xyz;
						}
					}
				else//conversion from native type.
					{
					/* get a row and prepare for encoding */
					GetOutputPixels(0,y,width,scanline); // Get gamma-corrected pixels
					scanptr = scanline;
					Color* lum = (Color*)shortstrip;
					for (k=0; k<width; k++, ++scanptr)
						{
						static LogLUV32Pixel converter;
						BMM_Color_64 rgb = *scanptr;
						Color rgbColor (rgb);
						Color xyz;
						converter.RGBtoXYZ(rgbColor, xyz);//Adjust for human sensitivity.
						lum[k] = xyz;
						}
					}
				
				}
			else
				TIFFError("tif.cpp", GetString(IDS_NO_WRITE_ALGORITHM));
#ifdef ALLOW_LZW_COMPRESSION
			if (UserData.compressionType == tif_compress_lzw)
				LZWEncode(shortstrip, packw);
			else {
#endif //ALLOW_LZW_COMPRESSION
				TIFFWriteScanline(tif, shortstrip, y);//copy the scanline to the tif output buffer.
				rawcc+= packw;
#ifdef ALLOW_LZW_COMPRESSION
				}
#endif //ALLOW_LZW_COMPRESSION
			y++;
			if (y == height) break;
			}

#ifdef ALLOW_LZW_COMPRESSION
		if (UserData.compressionType == tif_compress_lzw)
			LZWPostEncode();
#endif //ALLOW_LZW_COMPRESSION

		}	while (y != height);

	return TRUE;
	}

int
BitmapIO_TIF::SaveTIF() {
	unsigned short ret=1;

	width = map->Width();
	height = map->Height();

	write_alpha = (UserData.writeType == tif_write_color && UserData.writeAlpha && map->HasAlpha()) ? TRUE : FALSE;

	spp =(UserData.writeType == tif_write_color)?(write_alpha?4:3):1;  

	TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, spp);

	//refresh scanlinesize.
	tif->tif_scanlinesize = TIFFScanlineSize(tif);

	/* allocate memory for work areas */

	long scanlineSize = width;
	long sStripSize = spp * width;
	if (UserData.writeType == tif_write_logl)
		{
		//we're writing an array of floats (luminance, so we need a bigger buffer.
		sStripSize = spp * width* sizeof(float);
		scanlineSize = width*2;//3 floats + 1 float for alpha channel is twice the size of a 64 bit color which is 
									//the buffer type allocated.
		}
	else if (UserData.writeType == tif_write_logluv)
		{
		sStripSize = spp*width*sizeof(float)*3;//we convert from XYZ floats to SGILog
		scanlineSize = width*2;
		}

	
	PixelBuf64 sline(scanlineSize);//memory for the internal storage buffer.
	scanline = sline.Ptr();
	PixelBuf8 sstrip(sStripSize);
	shortstrip = sstrip.Ptr();

#ifdef ALLOW_LZW_COMPRESSION
	PixelBuf8 cbuf((BLOCKSIZE*3)/2);
	comp_buf = cbuf.Ptr();
#endif //ALLOW_LZW_COMPRESSION

	/*
	 * rows per strip must be carefully chosen - especially in light
	 * of RGB images. Assuming a worse case scenario wrt/packing bits,
	 * (ie: a single plane receives NO compression due to the complexity
	 * of the image) there might be 512 * RPS bytes in a strip; assuming
	 * the use of a targa or vision16 sized image (512 X 486). The
	 * value of the image width (512) times the rows per strip must
	 * NOT exceed the BLOCKSIZE definition. If this happens, the
	 * potential exists to corrupt memory.
	 */

	rps = BLOCKSIZE/(width*spp);						/* nice default value */

	//The tiff 6.0 library allows only 32767 for a working buffer size when using sgilog.
	//We have to make sure that our rps is not too big to avoid exceeding the maximum
	//value for a short.

	while((short)(width*rps) <= 0)
		rps /= 2;

	if (rps==0)
		return TIF_SAVE_WRITE_ERROR;
	
	/* construct tiff file header */
	MakeTiffhead();

    if (!WriteTIF((FILE*)tif->tif_clientdata))
        ret = TIF_SAVE_WRITE_ERROR;

	TIFFFlush(tif);

	return(ret);
	}

//-----------------------------------------------------------------------------
// #> BitmapIO_TIF::GetCfgFilename()	
//

void BitmapIO_TIF::GetCfgFilename( TCHAR *filename ) {
	_tcscpy(filename,TheManager->GetDir(APP_PLUGCFG_DIR));
	int len = _tcslen(filename);
	if (len) {
		if (_tcscmp(&filename[len-1],_T("\\")))
			_tcscat(filename,_T("\\"));
	}   
	_tcscat(filename,TIFCONFIGNAME);   
}

//-----------------------------------------------------------------------------
// #> BitmapIO_TIF::ReadCfg()
//

BOOL BitmapIO_TIF::ReadCfg() {
	
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
		
		LocalFree(buf);
	
		return (res);
	
	}
	
	return (FALSE);
}
	
//-----------------------------------------------------------------------------
// #> BitmapIO_TIF::WriteCfg()
//

void BitmapIO_TIF::WriteCfg() {
 
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


//-----------------------------------------------------------------------------
// #> BitmapIO_TIF::Close()
//

int  BitmapIO_TIF::Close( int flag ) {
    if ( openMode != BMM_OPEN_W )
		return 0;
	if (tif)
		{
		TIFFClose(tif);
		tif = NULL;
		}
    return 1;
}


void MAXWarningHandler(const char* module, const char* fmt, va_list ap)
{
#ifndef TIF_PLATFORM_CONSOLE
	LPTSTR szTitle;
	LPTSTR szTmp;

	LPCTSTR szTitleText = GetString(IDS_MAX_TIF_WARNING);
	LPCTSTR szDefaultModule = GetString(IDS_LIBTIFF_MODULE);

	szTmp = (module == NULL) ? (LPTSTR)szDefaultModule : (LPTSTR)module;
	if ((szTitle = (LPTSTR)LocalAlloc(LMEM_FIXED, (lstrlen(szTmp) +
			lstrlen(szTitleText) + lstrlen(fmt) + 128)*sizeof(TCHAR))) == NULL)
		return;
	wsprintf(szTitle, szTitleText, szTmp);
	szTmp = szTitle + (lstrlen(szTitle)+2)*sizeof(TCHAR);
	wvsprintf(szTmp, fmt, ap);

	//this is just a warning - display message box.
	//April 17, 2001.  Disable annoying warnings for images that load fine.
	//MessageBox(GetFocus(), szTmp, szTitle, MB_OK);

	LocalFree(szTitle);
	return;        
#else
	if (module != NULL)
		fprintf(stderr, "%s: ", module);
	fprintf(stderr, "Warning, ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, ".\n");
#endif
}

void MAXErrorHandler(const char* module, const char* fmt, va_list ap)
	{
#ifndef TIF_PLATFORM_CONSOLE

	LPTSTR szTitle;
	LPTSTR szTmp;
	LPCTSTR szTitleText = GetString(IDS_MAX_TIF_ERROR);
	LPCTSTR szDefaultModule = GetString(IDS_LIBTIFF_MODULE);
	szTmp = (module == NULL) ? (LPTSTR)szDefaultModule : (LPTSTR)module;
	if ((szTitle = (LPTSTR)LocalAlloc(LMEM_FIXED, (lstrlen(szTmp) +
			lstrlen(szTitleText) + lstrlen(fmt) + 128)*sizeof(TCHAR))) == NULL)
		return;
	wsprintf(szTitle, szTitleText, szTmp);
	szTmp = szTitle + (lstrlen(szTitle)+2)*sizeof(TCHAR);
	wvsprintf(szTmp, fmt, ap);

	//-- GG: 06/26/02
	// I'm not sure what this is all about but I do know it cannot be done
	// as is. You cannot just throw a dialogue with no regards to the
	// current environment. Chances are there will be no one to interact with
	// the dialogue and max will be stuck. Errors should be handled by
	// BitmapIO::ProcessImageIOError(). This is out of scope and I don't
	// have the time to figure it out. For now, if running in server mode
	// I will simply log and ignore the whole thing.

	if (!TheManager->SilentMode()) {
		if (MessageBox(GetFocus(), szTmp, szTitle, MB_RETRYCANCEL | MB_ICONINFORMATION) == IDCANCEL)
			throw MAXException(szTmp);
	} else
		TheManager->Max()->Log()->LogEntry(SYSLOG_ERROR,NO_DIALOG,NULL,_T("%s - %s"),szTitle,szTmp);

	LocalFree(szTitle);
	return;


#else
	if (module != NULL)
		fprintf(stderr, "%s: ", module);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, ".\n");
#endif        
	}
//-- EOF: TIF.cpp -----------------------------------------------------------
