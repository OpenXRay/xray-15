//-----------------------------------------------------------------------------
// -------------------
// File ....: flic.cpp
// -------------------
// Author...: Tom Hudson
// Date ....: November 22 1995
// Descr....: FLI/FLC File I/O Module
//
// History .: Nov, 22 1994 - Started
//            Oct, 20 1995 - Taken over the code and changed to new API (GG)
//            
//-----------------------------------------------------------------------------

//-- Include files

#include <Max.h>
#include <bmmlib.h>
#include "flic.h"
#include "pixelbuf.h"
#include "resource.h"


// Uncomment the following for debug prints
//#define DBGFLIC
		
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-- DLL Declaration

HINSTANCE hInst = NULL;
BOOL controlsInit = FALSE;

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
	 switch(fdwReason) {
		 case DLL_PROCESS_ATTACH:
				if (hInst)
					return(FALSE);
				hInst = hinstDLL;
				if ( !controlsInit ) {
					controlsInit = TRUE;
					// jaguar controls
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
	return(TRUE);
	}

TCHAR *GetString(int id)
	{
	static TCHAR buf[256];

	if (hInst)
		return LoadString(hInst, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
	}

//-----------------------------------------------------------------------------
// Uniform palette
#define RED_LVLS 6
#define GRN_LVLS 7
#define BLU_LVLS 6
static void makePalette(BMM_Color_48* pPal )	{
	int		i, j, k, n;
	double	red_ct   = RED_LVLS - 1.0;
	double	green_ct = GRN_LVLS - 1.0;
	double	blue_ct  = BLU_LVLS - 1.0;
	n = 0;
	for ( i=0; i < RED_LVLS; i++) {
		for ( j=0; j < GRN_LVLS; j++) {
			for ( k=0; k < BLU_LVLS; k++) {
				pPal[n].r  = (WORD)((double(i) / red_ct) * 65535.0);
				pPal[n].g  = (WORD)((double(j) / green_ct) * 65535.0);
				pPal[n].b  = (WORD)((double(k) / blue_ct) * 65535.0);
				n++;
				}
			}
		}
	}

//-----------------------------------------------------------------------------
// FLIC Class Description

class FLICClassDesc:public ClassDesc {
	 public:
	 int            IsPublic     ( ) { return 1; }
	 void          *Create       ( BOOL loading=FALSE) { return new BitmapIO_FLIC; }
	 const TCHAR   *ClassName    ( ) { return GetString(IDS_FLIC_CLASS); }
	 SClass_ID      SuperClassID ( ) { return BMM_IO_CLASS_ID; }
	 Class_ID       ClassID      ( ) { return Class_ID(FLICCLASSID,0); }
	 const TCHAR   *Category     ( ) { return GetString(IDS_BITMAP_IO);  }
	 };

static FLICClassDesc FLICDesc;

//-----------------------------------------------------------------------------
// Interface

DLLEXPORT const TCHAR * LibDescription ( )  { 
	 return GetString(IDS_FLIC_DESC); 
}

DLLEXPORT int LibNumberClasses ( ) { 
	 return 1; 
}

DLLEXPORT ClassDesc *LibClassDesc(int i) {
	 switch(i) {
		case  0: return &FLICDesc; break;
		default: return 0;         break;
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

/*
static void MessageBox(int s1, int s2= IDS_FLIC_WRITE) {
	TSTR str1(GetString(s1));
	TSTR str2(GetString(s2));
	MessageBox(GetActiveWindow(), str1.data(), str2.data(), MB_OK|MB_TASKMODAL);
	}
*/

//-----------------------------------------------------------------------------
// #> BitmapIO_FLIC Constructors/Destructor

BitmapIO_FLIC::BitmapIO_FLIC() : BitmapIO() {
	prast = thisRast = lastRast = NULL;
	fileptr = 0;
	inStream = NULL;
	isOldCEL = FALSE;
	hiMakeName = TRUE;
	config.Init(); 	
	}

BitmapIO_FLIC::~BitmapIO_FLIC() {
	if(prast) {
		pj_raster_free_ram(&prast);
		prast = NULL;
		}
	Close(BMM_CLOSE_ABANDON);
	}

//-----------------------------------------------------------------------------
// #> BitmapIO_FLIC::ExtCount()

int  BitmapIO_FLIC::ExtCount() {
	return 3;
	}

//-----------------------------------------------------------------------------
// #> BitmapIO_FLIC::Ext()

const TCHAR * BitmapIO_FLIC::Ext(int n) {
	 switch(n) {
		case 0:  return _T("flc");
		case 1:  return _T("fli");
		case 2:  return _T("cel");
		}
	return _T("");
	}

//-----------------------------------------------------------------------------
// #> BitmapIO_FLIC BMI Data

const TCHAR *BitmapIO_FLIC::LongDesc() {
	 return GetString(IDS_FLIC_FILE);
	}
	 
const TCHAR *BitmapIO_FLIC::ShortDesc() {
	 return GetString(IDS_FLIC);
	}

const TCHAR *BitmapIO_FLIC::AuthorName() {
	 return _T("Tom Hudson");
	}

const TCHAR *BitmapIO_FLIC::CopyrightMessage() {
	 return _T("Copyright 1995 Yost Group");
	}

unsigned int BitmapIO_FLIC::Version() {
	 return 100;
	}

int  BitmapIO_FLIC::Capability() {
	 return BMMIO_READER | 
			BMMIO_WRITER | 
			BMMIO_CONTROLWRITE |
			BMMIO_MULTIFRAME | 
			BMMIO_OWN_VIEWER |
		    BMMIO_NON_CONCURRENT_ACCESS |
			BMMIO_UNINTERRUPTIBLE |
			BMMIO_EXTENSION;
	}

//-----------------------------------------------------------------------------
// #> BitmapIO_FLIC::LoadConfigure()

BOOL BitmapIO_FLIC::LoadConfigure( void *ptr ) {
	FlicConfigData *buf = (FlicConfigData *)ptr;
	if (buf->version == FLIC_VERSION) {
		config = *buf;
        return (TRUE);
		} 
	else
        return (FALSE);
	}

//-----------------------------------------------------------------------------
// #> BitmapIO_FLIC::SaveConfigure()

BOOL BitmapIO_FLIC::SaveConfigure ( void *ptr ) {
	if (ptr) {
		config.saved = TRUE;
		memcpy(ptr,(void *)&config,sizeof(FlicConfigData));
		return (TRUE);
		} 
	else
		return (FALSE);
	}

//-----------------------------------------------------------------------------
// *> AboutCtrlDlgProc()
//

BOOL CALLBACK AboutCtrlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
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
// #> BitmapIO_FLIC::ShowAbout()

void BitmapIO_FLIC::ShowAbout(HWND hWnd) {
	DialogBoxParam(
		hInst,
		MAKEINTRESOURCE(IDD_ABOUT),
		hWnd,
		(DLGPROC)AboutCtrlDlgProc,
		(LPARAM)0);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_FLIC::GetImageInfo()

BMMRES BitmapIO_FLIC::GetImageInfo ( BitmapInfo *fbi ) {

	 //-- Open Flic File

	 TSTR wname(fbi->Name());

	 if (pj_flic_open_info(wname, &inflic, &finfo) >= Success) {
	
		pj_flic_close(&inflic);

		fbi->SetWidth(finfo.width);
		fbi->SetHeight(finfo.height);
		//fbi->SetGamma (1.0f);
		fbi->SetAspect (1.0f);
		fbi->SetFirstFrame(0);
		fbi->SetLastFrame(finfo.num_frames-1);
		fbi->SetType(BMM_PALETTED);

		return BMMRES_SUCCESS;
	 }

	 // Watch out -- it may be an old Autodesk Animator .CEL file!

	 File file(fbi->Name(), _T("rb"));

	 if(!(inStream = file.stream))
		return (ProcessImageIOError(fbi));

	 //-- Read Header -------------------------------------

	 if (ReadCELHeader() <= 0)
		return (ProcessImageIOError(fbi,BMMRES_BADFILEHEADER));

	 //-- Update BitmapInfo
	 fbi->SetWidth(CELhdr.width);
	 fbi->SetHeight(CELhdr.height);
	 //fbi->SetGamma(1.0f);
	 fbi->SetAspect(1.0f);
	 fbi->SetType(BMM_PALETTED);
	 fbi->SetFirstFrame(0);
	 fbi->SetLastFrame(0);

	 return BMMRES_SUCCESS;

	}

//-----------------------------------------------------------------------------
// *> GetPaletteFromFile()
//
BOOL BitmapIO_FLIC::GetPaletteFromFile(HWND hwnd, TCHAR *name, BOOL testOnly) {
	BMMRES status = BMMRES_SUCCESS;
	BitmapInfo pbi;
	if (name[0]==0) return 0;
	pbi.SetName(name);
	Bitmap *thebm = TheManager->Load(&pbi,&status);
	if (thebm==NULL) {
		if (!SilentMode())
			MessageBox(hwnd,GetString(IDS_COULDNT_OPEN),GetString(IDS_PALFILE_ERROR),MB_OK|MB_TASKMODAL);
		return 0;
		}
//	if (!thebm->Paletted()) {
//		MessageBox(hwnd,_T("Bitmap Not Paletted"),_T("Load Palette"),MB_OK);
//		return 0;
//		}
	int res;
	BMM_Color_48 pal[256];
	res = thebm->GetPalette(0,256,pal);
	if (res) {
		memcpy(config.custPal, pal, 256*sizeof(BMM_Color_48));
		}
	else {
		if (!SilentMode())
			MessageBox(hwnd,GetString(IDS_NO_PALETTE),GetString(IDS_PALFILE_ERROR),MB_OK|MB_TASKMODAL);
		}
	thebm->DeleteThis();
	return res;
	}

int  BrowsePaletteFile(HWND hWnd, TCHAR *name) {
	int tried = 0;
	FilterList filterList;
	static int filterIndex = 1;
    OPENFILENAME	ofn;
	TCHAR 			fname[256];

	_tcscpy(fname,name);
	fname[0] = 0;

	filterList.Append( _T("FLC (.flc,*.fli,*.cel)"));
	filterList.Append( _T("*.flc;*.fli;*.cel"));

	filterList.Append( _T("BMP (*.bmp)"));
	filterList.Append( _T("*.bmp"));

	filterList.Append( _T("GIF (*.gif)"));
	filterList.Append( _T("*.gif"));

	filterList.Append( _T("All files (*.*)"));
	filterList.Append( _T("*.*"));

    memset(&ofn, 0, sizeof(OPENFILENAME));

    ofn.lStructSize      = sizeof(OPENFILENAME);
    ofn.hwndOwner        = hWnd;
	ofn.hInstance        = hInst;	

	ofn.nFilterIndex = filterIndex;
    ofn.lpstrFilter  = filterList;
	ofn.Flags		= OFN_HIDEREADONLY;
    ofn.lpstrTitle   = GetString(IDS_LOAD_PALETTE); 
    ofn.lpstrFile    = fname;
    ofn.nMaxFile     = sizeof(fname) / sizeof(TCHAR);      
	
//   	ofn.lpstrInitialDir = initDir;

	if (GetOpenFileName(&ofn)) {
		_tcscpy(name,ofn.lpstrFile);
		return 1;
		}
	else return 0;
	}

//-----------------------------------------------------------------------------
// *> ControlDlg()
//

BOOL BitmapIO_FLIC::ControlDlg(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
	static BOOL forced = FALSE;
	static load_pal_clicked = FALSE;

	switch (message) {
		case WM_INITDIALOG: {
			if (!config.saved)
                ReadCfg();
			InitCommonControls();
			CenterWindow(hWnd,GetParent(hWnd));
			SetCursor(LoadCursor(NULL,IDC_ARROW));
			iPalCols = SetupIntSpinner(hWnd, IDC_PALCOLS_SPIN, IDC_PALCOLS, 1,256,config.nPalCols);
			CheckRadioButton(hWnd, IDC_PAL_LOW, IDC_PAL_UNIFORM, IDC_PAL_LOW+config.palType);			 
			SetDlgItemText(hWnd,IDC_CUST_PALFILE,config.palFileName);
			load_pal_clicked = FALSE;
			return 1;
	        }

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_LOAD_PAL:	{
					TCHAR buf[MAXPALNAME];
					GetDlgItemText(hWnd,IDC_CUST_PALFILE,buf,MAXPALNAME);
					if (BrowsePaletteFile(hWnd, buf)) 
						SetDlgItemText(hWnd,IDC_CUST_PALFILE,buf);
					GetDlgItemText(hWnd,IDC_CUSTOM_PAL,buf,MAXPALNAME);
					GetPaletteFromFile( hWnd, buf, TRUE); 
					load_pal_clicked = TRUE;
					CheckRadioButton(hWnd, IDC_PAL_LOW, IDC_PAL_UNIFORM, IDC_PAL_CUSTOM);			 
					}
					break;
				case IDC_PAL_LOW:
				case IDC_PAL_MED:
//				case IDC_PAL_HI:
				case IDC_PAL_CUSTOM:
				case IDC_PAL_UNIFORM:
					CheckRadioButton(hWnd, IDC_PAL_LOW, IDC_PAL_UNIFORM, LOWORD(wParam));
					break;			 
				case IDOK: 
					config.nPalCols = iPalCols->GetIVal();					
					
					if (IsDlgButtonChecked(hWnd,IDC_PAL_LOW)) config.palType = PAL_LOW;
					if (IsDlgButtonChecked(hWnd,IDC_PAL_MED)) config.palType = PAL_MED;
//					if (IsDlgButtonChecked(hWnd,IDC_PAL_HI)) config.palType = PAL_HI;
					if (IsDlgButtonChecked(hWnd,IDC_PAL_CUSTOM)) config.palType = PAL_CUSTOM;
					if (IsDlgButtonChecked(hWnd,IDC_PAL_UNIFORM)) config.palType = PAL_UNIFORM;

					if (load_pal_clicked) {
						GetDlgItemText(hWnd,IDC_CUST_PALFILE,config.palFileName,MAXPALNAME);
						GetPaletteFromFile( hWnd, config.palFileName); 
						}

					WriteCfg();
					
					EndDialog(hWnd,1);
					break;

				case IDCANCEL:
					EndDialog(hWnd,0);
					break;
					}
				return 1;
		case WM_DESTROY:
			ReleaseISpinner(iPalCols);
			break;
		}
	return 0;
	}


//-----------------------------------------------------------------------------
// *> ControlDlgProc
//

static INT_PTR CALLBACK ControlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
	 static BitmapIO_FLIC *bm = NULL;
	 if (message == WM_INITDIALOG) 
		bm = (BitmapIO_FLIC *)lParam;
	 if (bm) 
		return (bm->ControlDlg(hWnd,message,wParam,lParam));
	 else
		return FALSE;
	}


//-----------------------------------------------------------------------------
// #> BitmapIO_FLIC::ShowControl()
//

BOOL BitmapIO_FLIC::ShowControl(HWND hWnd, DWORD flag ) {
	 return (
		DialogBoxParam(
		hInst,
		MAKEINTRESOURCE(IDD_FLC_CONTROL),
		hWnd,
		(DLGPROC)ControlDlgProc,
		(LPARAM)this)
		 );
	}


//-----------------------------------------------------------------------------
//-- BitmapIO_FLIC::Load()
//

BitmapStorage *BitmapIO_FLIC::Load(BitmapInfo *fbi, Bitmap *map, BMMRES *status) {

	 BitmapStorage *s = NULL;

	//-- Initialize Status Optimistically

	*status = BMMRES_SUCCESS;

	//-- Make sure nothing weird is going on

	if(openMode != BMM_NOT_OPEN) {
		*status = ProcessImageIOError(fbi,BMMRES_INTERNALERROR);
		return NULL;
	}

	 TSTR wname(fbi->Name());

	 Errcode err = Success;

	 //-- Open Flic File

	 // First, see if it's an old-style CEL file
	 
	 File file(fbi->Name(), _T("rb"));

	 if(!(inStream = file.stream)) {
		*status = ProcessImageIOError(fbi);
		return (BitmapStorage *)NULL;
	 }

	 //-- Read Header -------------------------------------

	 if (ReadCELHeader() == 1) {
		
		//-- Create a storage for this bitmap...

		s = BMMCreateStorage(map->Manager(),BMM_PALETTED);

		if (!s) {
		   storage_error:
			*status = ProcessImageIOError(fbi,BMMRES_INTERNALERROR);
		   CEL_bail_out:
		   if (s) {
			  delete s;
			  s = NULL;
		   }
		   return (BitmapStorage *)NULL;
		}
		
		//-- Update BitmapInfo

		fbi->SetWidth(CELhdr.width);
		fbi->SetHeight(CELhdr.height);
		//fbi->SetGamma(1.0f);
		fbi->SetAspect(1.0f);
		fbi->SetType(BMM_PALETTED);
		fbi->SetFirstFrame(0);
		fbi->SetLastFrame(0);
		fbi->SetFlags(0);

		//-- Allocate Storage

		if (s->Allocate(fbi,map->Manager(),BMM_OPEN_R)==0)
		   goto storage_error;
	 
		// Stuff the palette into the storage
		s->SetPalette(0, 256, CELpalette);

		//-- Transfer Image

		PixelBuf8 buf(CELhdr.width);
		BYTE *ptr = buf.Ptr();

		for(int y = 0; y < CELhdr.height; ++y) {
		   if(!fread(ptr, CELhdr.width, 1, inStream))
			  goto CEL_bail_out;
		   if(s->PutIndexPixels(0, y, CELhdr.width, ptr)!=1)
			  goto storage_error;
		   }

		//-- Set the storage's BitmapInfo

		s->bi.CopyImageInfo(fbi);

		// Remind ourselves that this is not an animated file!
		isOldCEL = TRUE;

		return s;
	 }

	 // Not an old CEL file -- Close it and see if it's a regular flic...
	 
	 file.Close();
	 inStream = NULL;

	 if ((err = pj_flic_open_info(wname, &inflic, &finfo)) < Success) {
		*status = ProcessImageIOError(fbi,GetString(IDS_BAD_FILE));
		return (BitmapStorage *)NULL;
	 }

	 //-- Allocate Memory
	 
	 if ((err = pj_raster_make_ram(&prast,finfo.width,finfo.height)) < Success ) {
		*status = ProcessImageIOError(fbi,BMMRES_MEMORYERROR);
		bail_out:
		pj_flic_close(&inflic);
		if (s) {
		   delete s;
		   s = NULL;
		}
		return (BitmapStorage *)NULL;
	 }

	 //-- Update BitmapInfo

	 fbi->SetWidth(finfo.width);
	 fbi->SetHeight(finfo.height);
	 //fbi->SetGamma(1.0f);
	 fbi->SetAspect(1.0f);
	 fbi->SetType(BMM_TRUE_32);
	 fbi->SetFirstFrame(0);
	 fbi->SetLastFrame(finfo.num_frames - 1);
	 
	 //-- Handle Given Frame Number
	 
	 int fframe;

	*status = GetFrame(fbi,&fframe);

	if (*status != BMMRES_SUCCESS)
		goto bail_out;
		  
	 //-- Cue given frame

	 err = CueFlic(fframe);
	 if (err < Success) {
		*status = ProcessImageIOError(fbi,GetString(IDS_FILE_READ_ERROR));
		goto bail_out;
	 }

	 //-- Create a storage for this bitmap...

	 fbi->SetType(BMM_PALETTED);
	 s = BMMCreateStorage(map->Manager(),BMM_PALETTED);

	 if(!s)
		goto bail_out;

	 if(s->Allocate(fbi,map->Manager(),BMM_OPEN_R)==0)
		goto bail_out;

	 if(!StoreFrame(fframe, s))
		goto bail_out;

	 //-- Set the storage's BitmapInfo

	 s->bi.CopyImageInfo(fbi);
	 pj_flic_close(&inflic);	// TH 4/9/99

	 return s;

}

//-----------------------------------------------------------------------------
// #> BitmapIO_FLIC::FreeOutputRasters()
//
void BitmapIO_FLIC::FreeOutputRasters() {
	if(thisRast)
	   pj_raster_free_ram(&thisRast);
	if(lastRast)
	   pj_raster_free_ram(&lastRast);
	thisRast = lastRast = NULL;
	}

//-----------------------------------------------------------------------------
// #> BitmapIO_FLIC::InitOutputRasters()
//
int BitmapIO_FLIC::InitOutputRasters(int width, int height) {
	if ( pj_raster_make_ram(&thisRast,width,height) < Success ) {
		bail_out:
		FreeOutputRasters();
		return 0;
		}
	if ( pj_raster_make_ram(&lastRast,width,height) < Success ) 
		goto bail_out;
	thisRast->cmap->num_colors = 256;
	lastRast->cmap->num_colors = 256;
	return 1;
	}

//-----------------------------------------------------------------------------

void make_tmp_name(const TCHAR* fname, TSTR& tmpName) {
	TSTR fn(fname);
	TSTR path,ext;
	SplitFilename(fn, &path,NULL,&ext);
	TSTR name;
	tmpName.printf(_T("%s\\$rtemp$%s"), path.data(),ext.data());
	}

void make_tmp_hi_basename(const TCHAR* fname, const TCHAR* ext, TSTR& tmpName) {
	TSTR fn(fname);
	TSTR path, name;
	SplitFilename(fn, &path, &name, NULL);
	tmpName.printf(_T("%s\\$FLI-%s$.%s"), path.data(), name.data(), ext);
	}

//-----------------------------------------------------------------------------
// #> BitmapIO_FLIC:OpenOutput()
//

BMMRES BitmapIO_FLIC::OpenOutput(BitmapInfo *fbi, Bitmap *map) {

#ifdef DBGFLIC
DebugPrint("Opening flic for output\n");
#endif     
	if (openMode != BMM_NOT_OPEN)
		return (ProcessImageIOError(fbi,BMMRES_INTERNALERROR));
		
	if (!map)
		return (ProcessImageIOError(fbi,BMMRES_INTERNALERROR));
		
	//-- Init flic stuff

	pj_animinfo_init(&finfo);

	finfo.width  = map->Width();
	finfo.height = map->Height();
	rasterBytes  = finfo.width * finfo.height;

	 //-- Allocate work buffers
	if (!InitOutputRasters(finfo.width, finfo.height)) {
		return (ProcessImageIOError(fbi, BMMRES_INTERNALERROR));
		}

	TSTR wname;
	switch (config.palType) {
	case PAL_MED:
		make_tmp_name(fbi->Name(),wname);
		break;
	case PAL_HI:
		hiMakeName = TRUE;
		break;
	default:
		wname = fbi->Name();
	}

	if (config.palType != PAL_HI) {
		Errcode err = pj_flic_create(wname, &outflic, &finfo);
		if (err<Success) {
			FreeOutputRasters();
			return ProcessImageIOError(fbi,GetString(IDS_CANT_CREATE));
		}
	}

	// --- Init Image Info Data ---

	bi.CopyImageInfo(fbi);

	this->map   = map;
	openMode    = BMM_OPEN_W;

	frame = 0;
	switch (config.palType) {
		case PAL_CUSTOM:
			memcpy(outpal, config.custPal, 256*sizeof(BMM_Color_48));
			break;
		case PAL_LOW:
		case PAL_UNIFORM:
			makePalette(outpal);
			break;
		}
#ifdef DBGFLIC
DebugPrint("Flic opened OK\n");
#endif     

	return (BMMRES_SUCCESS);
	}

class PixelSource {
	public:
	virtual int Width()=0;
	virtual int Height()=0;
	virtual	int GetPixels(int x,int y,int pixels,BMM_Color_64  *ptr)=0;
	};

class MyPixSrc: public PixelSource {
	public:
	int w,h;
	BitmapIO *bi;
	MyPixSrc(BitmapIO *b, int ww, int hh) { bi = b; w = ww; h = hh; } 
	virtual int Width() { return w;}
	virtual int Height() { return h; }
	virtual	int GetPixels(int x,int y,int pixels,BMM_Color_64  *ptr) {
		return bi->GetOutputPixels(x,y,pixels,ptr);
		}
	};

BMMRES ColorCut(PixelSource *src, BMM_Color_48 *pal, int nSlots, BYTE *pixel, BYTE *remap=NULL)  {
	int y, w = src->Width();
	PixelBuf line(w);
	if(!line.Ptr())
		return BMMRES_INTERNALERROR;
	
	// Create a color packer to reduce to 256 colors
	ColorPacker* cpack = BMMNewColorPacker(w,pal,nSlots,remap);
	cpack->PropogateErrorBetweenLines(FALSE); // better for animation 

	for (y=0; y<src->Height(); ++y) {
		if (!src->GetPixels(0, y, w, line.Ptr()))
			return BMMRES_INTERNALERROR;
		cpack->PackLine(line.Ptr(),pixel,w);
		pixel += w;
		}

	cpack->DeleteThis();
	return BMMRES_SUCCESS;
	}

static void color48_to_PjRgb(PjRgb *rgb, BMM_Color_48 *c48, int n=256) {
	int i;
	if (rgb==NULL) 
		return;
	for (i=0; i<n; i++) {
		rgb[i].r = c48[i].r>>8;
		rgb[i].g = c48[i].g>>8;	
		rgb[i].b = c48[i].b>>8;
		}
	}

static void PjRgb_to_color48( BMM_Color_48 *c48, PjRgb *rgb, int n=256) {
	int i;
	if (rgb==NULL) 
		return;
	for (i=0; i<n; i++) {
		c48[i].r = rgb[i].r<<8;
		c48[i].g = rgb[i].g<<8;
		c48[i].b = rgb[i].b<<8;
		}
	}


//-----------------------------------------------------------------------------
// #> BitmapIO_FLIC::Write()
//

BMMRES BitmapIO_FLIC::Write(int frameNum) {
	// Gotta convert this!
	if(frameNum == BMM_SINGLEFRAME)
	    frameNum = 0;

	// If (frameNum != frame) warning(out of sync);

#ifdef DBGFLIC
DebugPrint("Writing frame %d\n",frame);
#endif     
	 
	//-- If we haven't gone through an OpenOutput(), leave
	if (openMode != BMM_OPEN_W)
		return (ProcessImageIOError(&bi,BMMRES_INTERNALERROR));

	if (frame==0) {
		remap = NULL;
		memcpy(cutpal,outpal,256*sizeof(BMM_Color_48));
	}
	
	// NOTE: Currently, this only outputs LOW color flics (1st frame palette used for all)
	// NOTE: Frame number is ignored!  Must be sequential output

	// Use palette from image if paletted bitmap
	if ((palSlots=map->Paletted()) != 0) {
		map->GetPalette(0, palSlots, outpal);
	} else {
		BOOL calcPal;
		palSlots = config.nPalCols;
		switch (config.palType) {
			case PAL_LOW:		calcPal = (frame==0)?TRUE:FALSE; break;
			case PAL_MED:		calcPal = TRUE; palSlots = 256; break;
			case PAL_HI:
			case PAL_CUSTOM:
			case PAL_UNIFORM:
			default:			calcPal = FALSE;
		}
		if (calcPal) {
		  	if ( CalcOutputPalette(palSlots, cutpal) == 0)
				return ProcessImageIOError(&bi,GetString(IDS_PAL_ERROR));
			if ( config.palType==PAL_LOW ) {
				FixPaletteForWindows(cutpal, outpal, palSlots, fixmap);
				remap = fixmap;
			}
			else memcpy(outpal,cutpal,256*sizeof(BMM_Color_48));
		}
	}
	
	// stuff palette into output raster
	color48_to_PjRgb(thisRast->cmap->ctab, outpal);

	// Transfer image to output raster
	if (map->Paletted()) {
		int y;
		int w = map->Width();
		int h = map->Height();
		BYTE *out = (BYTE *)thisRast->hw.bytemap_data[3];
		for (y=0; y<h; ++y, out += w) {
		    if (!map->GetIndexPixels(0,y,w,out))
				return ProcessImageIOError(&bi,BMMRES_INTERNALERROR);
		}
	} else {
		MyPixSrc pixsrc(this,map->Width(), map->Height());
		if (frame==0) {
			// save background color so we can force it to be in the palette.
			GetOutputPixels(0,0,1,&bgColor);
			}
		BMMRES r = ColorCut(&pixsrc, cutpal, palSlots, (BYTE *)thisRast->hw.bytemap_data[3],remap);
		if (r!=BMMRES_SUCCESS) 
			return ProcessImageIOError(&bi,r);
	}

	// Write frame
	Errcode err;
	if (config.palType != PAL_HI) {		// Write fli raster
		if (frame == 0)
			err = pj_flic_write_first(&outflic,thisRast);
		else
			err = pj_flic_write_next(&outflic,thisRast,lastRast);
	} else {							// Write True Color image file
		BitmapInfo imgbi;
		make_tmp_hi_basename(bi.Name(), PAL_HI_EXT, hiTempFile);
		imgbi.SetName(hiTempFile);
		int idx  = TheManager->ioList.FindDeviceFromFilename(imgbi.Name());
		if (idx >= 0)
		   imgbi.SetDevice(TheManager->ioList[idx].LongDescription());
		map->OpenOutput(&imgbi);
		map->Write(&imgbi, frame);
		map->Close(&imgbi);
		err = Success;
	}

	if(err < Success)
		return ProcessImageIOError(&bi,GetString(IDS_WRITE_ERROR));

	// Copy thisRast image area and palette to lastRast
	pj_raster_copy(thisRast,lastRast);

	// Increment to next frame
	frame++;

#ifdef DBGFLIC
DebugPrint("Wrote frame OK\n");
#endif     
	return BMMRES_SUCCESS;

	}


//-----------------------------------------------------------------------------
//-- BitmapIO_FLIC::Close()
//

int BitmapIO_FLIC::Close(int option) {

	#ifdef DBGFLIC
	DebugPrint("Closing flic / %d\n",option);
	#endif     
	
	bool killfile = false;

	Errcode err;
	switch(openMode) {

		case BMM_OPEN_R:
			if(isOldCEL)				// Already closed if it's an old-style CEL
				break;
			if(!prevIO && !nextIO)
				pj_flic_close(&inflic);
			else
				pj_free_flic(&inflic);  // Free the memory but leave file open!
			pj_raster_free_ram(&prast);
			break;
		
		case BMM_OPEN_W:
			if (option == BMM_CLOSE_COMPLETE) {
				if (config.palType != PAL_HI) {
					err = pj_flic_write_finish(&outflic,thisRast);
					if(err < Success) {
						if (!SilentMode())
							MessageBox(NULL,GetString(IDS_FINISH_ERROR),_T("FLIC"),MB_OK|MB_TASKMODAL);
						killfile = true;
					}
			   		err = pj_flic_close(&outflic);
					if(err < Success) {
						if (!SilentMode())
							MessageBox(NULL,GetString(IDS_CLOSE_ERROR),_T("FLIC"),MB_OK|MB_TASKMODAL);
						killfile = true;
					}
				}
				FreeOutputRasters();
				// Post processing MED/HI palette methods
				if (killfile == false)
					switch (config.palType) {
					case PAL_MED:
						PostMed();
						break;
					case PAL_HI:
						PostHi();
						break;
					}
			} else
				killfile = true;
			break;
	}

	//-- If the file is invalid, kill it
	//   GG: 09/14/99

	if (killfile) {
		if (config.palType == PAL_HI) {		// Delete all temporary image files
			TCHAR cname[MAX_PATH];
			make_tmp_hi_basename(bi.Name(), PAL_HI_EXT, hiTempFile);
			for(int ix=0; ix<frame; ++ix) {
				BMMCreateNumberedFilename(hiTempFile, ix, cname);
				remove(cname);
			}
		} else {							// Delete flic file
			TSTR wname;
			if (config.palType==PAL_MED) 
				make_tmp_name(bi.Name(), wname);
			else wname = bi.Name();
			DeleteFile(wname);
		}
	}

	//-- Reset open mode, just in case...

	openMode = BMM_NOT_OPEN;

	#ifdef DBGFLIC
	DebugPrint("Flic closed OK!\n");
	#endif
	 
	return 1;

}

//-----------------------------------------------------------------------------
// #> BitmapIO_FLIC::StoreFrame()
//

int  BitmapIO_FLIC::StoreFrame(int f, BitmapStorage *storage) {
	 
	 PixelBuf8 line(finfo.width);
	 BYTE *lineptr = line.Ptr();
	 if(!lineptr)
		return 0;

	 PixelBuf48 pal(256);
	 BMM_Color_48 *palptr = pal.Ptr();
	 if(!palptr)
		return 0;

	 int ix,iy;
	 BYTE *pixelIn = (BYTE *)prast->hw.bytemap_data[3];

	 // Convert the flic palette

	 PjRgb *cmap = prast->cmap->ctab;
	 for(ix=0; ix<256; ++ix) {
		palptr[ix].r = cmap[ix].r << 8;
		palptr[ix].g = cmap[ix].g << 8;
		palptr[ix].b = cmap[ix].b << 8;
		}

	 storage->SetPalette(0,256,palptr);  // Now give it the palette

	 for(iy=0; iy<finfo.height; ++iy,pixelIn+=finfo.width) {
		memcpy(lineptr,pixelIn,finfo.width);
		if(!storage->PutIndexPixels(0,iy,finfo.width,lineptr))
		   return 0;
		}
	 
	 return 1;
	}

//-----------------------------------------------------------------------------
// #> BitmapIO_FLIC::CueFlic()
//
//

Errcode BitmapIO_FLIC::CueFlic(int f) {
#ifdef DBGFLIC
DebugPrint("Cueing flic to frame %d\n",f);
#endif     
	 
	 long result;
	 result = fli_seek(&inflic,fileptr); // Point it to where we left off

	 // Look where the flic frame pointer is, then command it appropriately
	 f++;
	 int flicframe = pj_flic_frame(&inflic);
	 if(f == flicframe) {
#ifdef DBGFLIC
DebugPrint("Flic already cued there (OK)!\n");
#endif     
		return Success;      // Already there!
		}
	 if(flicframe == 0 || f < flicframe)
		f = -f;
	 else
		f -= flicframe;
	 Errcode err = pj_flic_play_frames(&inflic,prast,f);
	 fileptr = fli_tell(&inflic);
#ifdef DBGFLIC
DebugPrint("Flic cue returned %d\n",err);
#endif     
	 return err;

}

int BitmapIO_FLIC::ReadCELHeader() {

	if(!fread(&CELhdr, sizeof(CelHead), 1, inStream))
		return CELERR_READ_ERROR;

	if(CELhdr.magic!=CEL_MAGIC)
		return CELERR_BAD_MAGIC;

	if(CELhdr.compression!=0 || CELhdr.bits!=8)
		return(CELERR_BAD_FORMAT);

	BMM_Color_24 rawPalette[256];
	if(!fread(rawPalette, 768, 1, inStream))
		return CELERR_BAD_FORMAT;

	// Convert the palette

	for(int i = 0; i < 256; ++i) {
		CELpalette[i].r = (WORD)rawPalette[i].r << 10;
		CELpalette[i].g = (WORD)rawPalette[i].g << 10;
		CELpalette[i].b = (WORD)rawPalette[i].b << 10;
		}

	return 1;
	}

void KillProcess(HANDLE hProcess)
{
	DWORD exitCode;

	if(hProcess) {
		GetExitCodeProcess(hProcess, &exitCode);
		if(exitCode == STILL_ACTIVE) {
			TerminateProcess(hProcess, 0);
			CloseHandle(hProcess);
		}
	}
}

static HANDLE playProcess = NULL;

//-----------------------------------------------------------------------------
// *> PlayShellFile()
//
// Play using explorer's default viewer
BOOL PlayShellFile(const TCHAR *filename) {
	SHELLEXECUTEINFO pShellExecInfo;

	if(playProcess)
		KillProcess(playProcess);
	
	playProcess = NULL;

	pShellExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	pShellExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	pShellExecInfo.hwnd = NULL;
	pShellExecInfo.lpVerb = "play";
	pShellExecInfo.lpFile = filename;
	pShellExecInfo.lpParameters = NULL;
	pShellExecInfo.lpDirectory = NULL;
	pShellExecInfo.nShow = SW_SHOWNORMAL;

	BOOL bRetVal = ShellExecuteEx(&pShellExecInfo);
	if (bRetVal) {
		playProcess = pShellExecInfo.hProcess;
		}

	return bRetVal;
}

//-----------------------------------------------------------------------------
// *> PlayFile()
//
BOOL PlayFile ( TCHAR *cmd ) {

	PROCESS_INFORMATION process;
	STARTUPINFO	si;
	BOOL			res =	FALSE;

	//-- Startup Info	Structure --------------------------

	memset(&si,0,sizeof(STARTUPINFO));
	
	si.cb			= sizeof(STARTUPINFO);
	si.dwFlags		= STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow	= SW_SHOWDEFAULT;
	si.hStdError	= NULL;

	if(playProcess)
		KillProcess(playProcess);
	
	playProcess = NULL;
	
	//-- Start the	Thing	---------------------------------

	res =	CreateProcess(
		  (LPCTSTR)	NULL,						//-- Pointer	to	name of executable module 
		  (LPTSTR)	cmd,  						//-- Pointer	to	command line string
		  (LPSECURITY_ATTRIBUTES)	NULL,		//-- Pointer	to	process security attributes 
		  (LPSECURITY_ATTRIBUTES)	NULL,		//-- Pointer	to	thread security attributes	
		  FALSE,								//-- Handle inheritance flag 
		  (DWORD)0,								//-- Creation flags 
		  (LPVOID)NULL,							//-- Pointer	to	new environment block 
		  (LPCTSTR)NULL,						//-- Pointer	to	current directory	name 
		  (LPSTARTUPINFO)&si,					//-- Pointer	to	STARTUPINFO	
		  (LPPROCESS_INFORMATION)&process		//-- Pointer	to	PROCESS_INFORMATION	
	);		 
	
	if	(!res)
		return (FALSE);
	
	CloseHandle(process.hThread);

	// NO, Don't Wait!!! - DB
	// MAX won't be able to process any messages while this is waiting!!
#if 0
	//-- Wait for process to finish ----------------------
	DWORD			exitcode;

	 exitcode	= WaitForSingleObject(process.hProcess,INFINITE);
	CloseHandle(process.hProcess);
#else
	playProcess = process.hProcess;
#endif
		
	 return (TRUE);

}


//-----------------------------------------------------------------------------
// #> BitmapIO_AVI::ShowImage()
//

BOOL BitmapIO_FLIC::ShowImage( HWND hWnd, BitmapInfo *lbi ) {

	// First, see if it's an old-style CEL file -- They can't be played by the media player
	 
	File file(lbi->Name(), _T("rb"));

	if(!(inStream = file.stream))
	   return FALSE;

	if (ReadCELHeader() == 1)
	   return FALSE;			// Can't play it!

	file.Close();
	inStream = NULL;

	if (!BMMGetFullFilename(lbi))
	   return FALSE;

	TCHAR	cmd[512];
	// CCJ 4.29.99
	// The Shell in NT4SP3 uses amovie.ocx, which can't playback Flic files.
	// Disable the shell playback for now.
	//if (!PlayShellFile(lbi->Name())) {
		wsprintf(cmd,_T("mplay32.exe \"%s\""),lbi->Name());
		if (!PlayFile(cmd)) {
			wsprintf(cmd,_T("mplayer.exe \"%s\""),lbi->Name());
			if(!PlayFile(cmd))
				return FALSE;
			}
	//	}

	return TRUE;
	}




//-----------------------------------------------------------------------------
// #> BitmapIO_FLIC::GetCfgFilename()
//

void BitmapIO_FLIC::GetCfgFilename( TCHAR *filename ) {
	_tcscpy(filename,TheManager->GetDir(APP_PLUGCFG_DIR));
	int len = _tcslen(filename);
	if (len) {
		if (_tcscmp(&filename[len-1],_T("\\")))
			_tcscat(filename,_T("\\"));
		}   
	_tcscat(filename,FLICCONFIGNAME);   
	}


//-----------------------------------------------------------------------------
// #> BitmapIO_FLIC::ReadCfg()
//
BOOL BitmapIO_FLIC::ReadCfg() {
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
		if(!res)
			config.Init();
		LocalFree(buf);
		return (res);
		}
	
	return (FALSE);
	}
	
//-----------------------------------------------------------------------------
// #> BitmapIO_FLIC::WriteCfg()
//

void BitmapIO_FLIC::WriteCfg() {
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


void FlicConfigData::Init()  { 	
   	saved = FALSE;
	version = FLIC_VERSION; 
	palType = PAL_LOW; 
	nPalCols =	256; 
	palFileName[0]=0;
	makePalette(custPal);
	}


int outflicOpen=0;

int OpenOutfli(char *name, Flic &outflic, int w, int h) {
	Errcode err;
	AnimInfo info;
	pj_animinfo_init(&info);
	info.width = w;
	info.height = h;
	err = pj_flic_create(name,&outflic,&info);
	return(err);
	}


//-----------------------------------------------------------------------------
// #> BitmapIO_FLIC::PostMed()
//
//  Post process "Medium" level palettes.
//	Accumulate a count of the colors in all the palettes of all the frames
//  of the flic.  Then make a palette by cutting it down, and
//  remap the flic images to the new palette.
//

int BitmapIO_FLIC::PostMed() {
	int ix,jx,ncols;
	Errcode err;
	PjRgb *curpal;
	FlicRaster  *inRast=NULL;
	long pixels;
	BMM_Color_48 workpal[256];
	BYTE fixmap[256];
	BYTE mappal[256];
	ColorPacker* cpack = NULL;
	Flic tmpflic = {0};
	Flic outflic = {0};
	BOOL outFlicOpen = FALSE;	
	BOOL tmpFlicOpen = FALSE;	
	TSTR tname;
	
	int res = 0;
	int w = map->Width();
	int h = map->Height();

	pixels = map->Width()*map->Height();

	if ((err = pj_raster_make_ram(&inRast,w,h)) < Success ) {
		return (ProcessImageIOError(&bi,GetString(IDS_FLIC_NORAM)));
		}

	Quantizer *quant = BMMNewQuantizer();
	if (!quant->AllocHistogram()) {
		ProcessImageIOError(&bi,GetString(IDS_FLIC_NORAM));
		pj_raster_free_ram(&inRast);
		quant->DeleteThis();
		return 0;
		}

	SetCursor(LoadCursor(NULL,IDC_WAIT));
	
	// open the temporary fli file 
	make_tmp_name(bi.Name(),tname);
	if ((err = pj_flic_open(tname, &tmpflic)) < Success)	
		goto error;

	tmpFlicOpen = TRUE;	
	
	/* Go thru and tally the colors used in all frames */
	for(ix=0; ix<frame; ++ix)	{
		if ((err = pj_flic_play_frames(&tmpflic,inRast,1))<Success) 
			goto error;
		curpal = inRast->cmap->ctab;
		PjRgb_to_color48( workpal, curpal);
		quant->AddToHistogram(workpal,256);
		}

	ncols = quant->Partition(cutpal, config.nPalCols, &bgColor);
	quant->DeleteThis();  // we're done with the Quantizer
	quant = NULL;

	FixPaletteForWindows(cutpal, outpal, ncols, fixmap); 

	// --- Now go back thru frames, convert to RGB and cut their colors --- 
	pj_flic_rewind(&tmpflic);

	// create output rasters
	if (!InitOutputRasters(w, h)) {
		err = Success-1;
		goto error;
		}

	color48_to_PjRgb(thisRast->cmap->ctab,outpal);
	color48_to_PjRgb(lastRast->cmap->ctab,outpal);
	
	// --- Open actual output FLI --- 
	if ((err = OpenOutfli(TSTR(bi.Name()), outflic, w, h))<Success) 
		goto error;
	outFlicOpen = TRUE;	

	// create a color packer to use to remap the palettes.
	cpack = BMMNewColorPacker(256, cutpal, config.nPalCols, fixmap);
	cpack->PropogateErrorBetweenLines(FALSE); 
	cpack->EnableDither(FALSE);

	for(ix=0; ix<frame; ++ix)	{
		// step input flic to the next frame */
		if ((err = pj_flic_play_frames(&tmpflic,inRast,1)) < Success) 
			goto werror;
														  
		pj_raster_copy(inRast,thisRast);   // thisRast = inRast;

		curpal = thisRast->cmap->ctab;
		PjRgb_to_color48( workpal, curpal);
		
		cpack->PackLine(workpal, mappal, 256);

		// now remap image 
		BYTE *cp = (BYTE *)thisRast->hw.bytemap_data[3];
		for(jx=0; jx<pixels; jx++,cp++)	*cp = mappal[*cp];

		color48_to_PjRgb(thisRast->cmap->ctab,outpal);
		color48_to_PjRgb(lastRast->cmap->ctab,outpal);

		// Write the fli frame 
		if (ix==0) 
			err = pj_flic_write_first(&outflic, thisRast);
		else  
			err = pj_flic_write_next(&outflic,  thisRast, lastRast);

		if(err<Success)	
			goto werror;

		pj_raster_copy(thisRast, lastRast);  // lastRast = thisRast;
		}
	

	color48_to_PjRgb(lastRast->cmap->ctab,outpal);
	err = pj_flic_write_finish(&outflic, thisRast);
	if (err<Success) {
		werror:
		pj_flic_close(&outflic);
		outFlicOpen = FALSE;
		remove(bi.Name());
		}

	error:
	if (err<Success) {	
		ProcessImageIOError(&bi,GetString(IDS_WRITE_ERROR));
		res = 0;
		}
	if (tmpFlicOpen) {
		pj_flic_close(&tmpflic);
		remove(tname);
		}
	if (outFlicOpen) {
		pj_flic_close(&outflic);
		}
	FreeOutputRasters();
	if (cpack) cpack->DeleteThis();
	if(inRast)  
		pj_raster_free_ram(&inRast);
	if (quant) 
		quant->DeleteThis();
	SetCursor(LoadCursor(NULL,IDC_ARROW));

	return(res);
	}


//-----------------------------------------------------------------------------
// #> BitmapIO_FLIC::PostHi()
//
//  Post process "High" level palettes.
//	Accumulate a count of the colors in all frames.
//  Based on this histogram, a palette is computed.
//  All the frames are read in, and reduced to byte-per-pixel images
//  using the palette, and written into the flic.
//

#define PAL_HI_SAMPLES			1000000	// number of samples when using HIGH palette method
#define MIN_SAMPLES_PER_FRAME	10		// Minimum samples per frames

int BitmapIO_FLIC::PostHi()	{
	int ix, ncols;
	long qinc;				// sample increment
	ColorPacker* cpack = NULL;
	BOOL outFlicOpen = FALSE;
	Bitmap *imgbmap;		// Bitmap of the True Color Image
	BitmapInfo imgbi;		// BitmapInfo of the True Color Image
	TCHAR cname[MAX_PATH];	// Temporary file name with number
	BMMRES lstatus;			// Status of loading bitmap
	Errcode err;			// Error code

	int res = 0;			// returned result
	int w = map->Width();	// image width
	int h = map->Height();	// image height
	long pixels = w * h;	// number of pixels in the image

	// Create a quantizer for histogram accumulation
	Quantizer *quant = BMMNewQuantizer();
	if (!quant->AllocHistogram()) {
		ProcessImageIOError(&bi,GetString(IDS_FLIC_NORAM));
		quant->DeleteThis();
		return 0;
	}

	// Turn on waiting cursor
	SetCursor(LoadCursor(NULL,IDC_WAIT));

	// Compute the sample rate of histogram computation
	qinc = frame*pixels/PAL_HI_SAMPLES;
	if(pixels < qinc*MIN_SAMPLES_PER_FRAME) qinc = pixels/MIN_SAMPLES_PER_FRAME;
	if(qinc==0) qinc = 1;

	// Go thru and tally the colors used in all frames
	BMM_Color_64 pixelValue;	// Pixel value
	PixelBuf pixelLine(w);		// Pixel line
	if (pixelLine.Ptr() == NULL) goto error;	// internal error

	if (hiMakeName == TRUE) make_tmp_hi_basename(bi.Name(), PAL_HI_EXT, hiTempFile);
	for(ix=0; ix<frame; ++ix) {
		// Load one frame from temp True Color image file
		BMMCreateNumberedFilename(hiTempFile, ix, cname);
		imgbi.SetName(cname);
		int idx  = TheManager->ioList.FindDeviceFromFilename(imgbi.Name());
		if (idx >= 0)
		   imgbi.SetDevice(TheManager->ioList[idx].LongDescription());
		imgbmap = TheManager->Load(&imgbi, &lstatus);
		if(lstatus != BMMRES_SUCCESS) goto error;

		// Take a sample every qinc pixels
		for (long i=(rand()%qinc); i<pixels; i+=qinc) {
			int px = i/w;
			int py = i%w;
			imgbmap->GetPixels(px, py, 1, &pixelValue);
			quant->AddToHistogram(&pixelValue, 1);
		}

		// Free temp bitmap resource
		if (imgbmap) imgbmap->DeleteThis();
	}

	// Now pack this color map
	ncols = quant->Partition(cutpal, config.nPalCols, &bgColor);
	quant->DeleteThis();	// we're done with the Quantizer
	quant = NULL;

	// Rearrange palette order
	FixPaletteForWindows(cutpal, outpal, ncols, fixmap);
	
	// --- Now go back thru frames and cut their colors ---

	// create output rasters
	if (!InitOutputRasters(w, h)) goto error;

	// Are these necessary?
	color48_to_PjRgb(thisRast->cmap->ctab, outpal);
	color48_to_PjRgb(lastRast->cmap->ctab, outpal);
	
	// --- Open actual output FLI ---
	if ((err = OpenOutfli(TSTR(bi.Name()), outflic, w, h)) < Success)
		goto error;
	outFlicOpen = TRUE;

	// create a color packer to use to remap the palettes.
	cpack = BMMNewColorPacker(w, cutpal, config.nPalCols, fixmap);
	cpack->PropogateErrorBetweenLines(FALSE); 
	cpack->EnableDither(FALSE);

	for(ix=0; ix<frame; ++ix) {
		// Load one frame from temp True Color file
		BMMCreateNumberedFilename(hiTempFile, ix, cname);
		imgbi.SetName(cname);
		imgbmap = TheManager->Load(&imgbi, &lstatus);
		if(lstatus != BMMRES_SUCCESS) goto werror;
		// Done with temp frame, nuke it!
		remove(cname);

		// Initialize thisRast, maybe?

		// Fit the frame to the color palette
		BYTE *pMap = (BYTE*) thisRast->hw.bytemap_data[3];
		for (int iy=0; iy<h; iy++, pMap+=w) {
			imgbmap->GetPixels(0, iy, w, pixelLine.Ptr());
			cpack->PackLine(pixelLine.Ptr(), pMap, w);
		}

		// use this color map
		color48_to_PjRgb(thisRast->cmap->ctab, outpal);
		color48_to_PjRgb(lastRast->cmap->ctab, outpal);

		// Write the fli frame
		if (ix==0) 
			err = pj_flic_write_first(&outflic, thisRast);
		else
			err = pj_flic_write_next(&outflic, thisRast, lastRast);
		if (err < Success) goto werror;
	
		// Free temp bitmap resource
		if (imgbmap) imgbmap->DeleteThis();

		pj_raster_copy(thisRast, lastRast);		// lastRast = thisRast;
	}
	
	color48_to_PjRgb(lastRast->cmap->ctab, outpal);
	err = pj_flic_write_finish(&outflic, thisRast);
	if (err < Success) goto werror;
	res = 1;			// success

	// Writing Error. Close out flic file.
	werror:
	pj_flic_close(&outflic);
	outFlicOpen = FALSE;
	if (res == 0) remove(bi.Name());
	
	// Error message
	error:
	if (res == 0) ProcessImageIOError(&bi,GetString(IDS_WRITE_ERROR));

	// Free resource
	FreeOutputRasters();
	if (quant) quant->DeleteThis();

	// Set cursor back to normal
	SetCursor(LoadCursor(NULL,IDC_ARROW));

	return(res);
}


//-----------------------------------------------------------------------------
// #> BitmapIO_FLIC::PostHi()
//
//  Convert a sequence of True Color images into paletted flic file
//  A BitmapIO_FLIC need to be created with the following data set properly
//  before calling BitmapIO_FLIC::PostHi()
//		BitmapIO::map			- set width, height
//		BitmapIO::bi			- set output flic filename
//		BitmapIO_FLIC::config	- set number of palette colors
//
//	The input sequence of images must be at the same folder as the output flic file.
//		path			- directory of the image files
//		basename		- the base name of the input file names, eg "testimage"
//		ext				- extension of the input file names, eg "TGA"
//		nimages			- number of input images
//
//	The input filenames will then be in the format of "basenameXXXX.ext", where XXXX
//	started from 0000 to (nimages-1).

int BitmapIO_FLIC::TrueColorToPalette(const TCHAR* path, const TCHAR* basename, const TCHAR* ext, int nimages) {
	// Set number of frames
	frame = nimages;
	// Set the temp filenames for PostHi() input
	hiTempFile.printf(_T("%s\\%s.%s"), path, basename, ext);
	// Disable the automatic generation of temp filenames in PostHi()
	hiMakeName = FALSE;
	int res = PostHi();
	hiMakeName = TRUE;
	return res;
}