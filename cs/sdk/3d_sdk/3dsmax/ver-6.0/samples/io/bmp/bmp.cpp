//-----------------------------------------------------------------------------
// ------------------
// File ....: bmp.cpp
// ------------------
// Author...: Gus J Grubba
// Date ....: October 1995
// Descr....: BMP File I/O Module
//
// History .: Oct, 26 1995 - Started
//            
//-----------------------------------------------------------------------------
		
//-- Include files

#include <Max.h>
#include <istdplug.h>
#include <bmmlib.h>
#include <IParamb2.h>
#include "bmp.h"
#include "bmprc.h"
#include "pixelbuf.h"

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
// BMP Class Description

class BMPClassDesc:public ClassDesc2 {
	
	public:

		int             IsPublic     ( )                   { return 1;                }
		void           *Create       ( BOOL loading=FALSE) { return new BitmapIO_BMP; }
		const TCHAR    *ClassName    ( )                   { return GetString(IDS_BMP); }
		SClass_ID       SuperClassID ( )                   { return BMM_IO_CLASS_ID;  }
		Class_ID        ClassID      ( )                   { return Class_ID(BMPCLASSID,0);    }
		const TCHAR    *Category     ( )                   { return GetString(IDS_BITMAP_IO); }
		const TCHAR    *InternalName ( )                   { return _T("bmpio"); }
		HINSTANCE		HInstance    ( )                   { return hInst; }

};

static	BMPClassDesc BMPDesc;

// FP Interface
class BitmapIO_Bmp_Imp : public IBitmapIO_Bmp {	
public:
	int		GetType();
	void	SetType(int type);

	// function IDs 
	enum { bmpio_getType, bmpio_setType }; 
	// enum IDs 
	enum { bmpio_type }; 

	DECLARE_DESCRIPTOR(BitmapIO_Bmp_Imp) 

	// dispatch map
	BEGIN_FUNCTION_MAP
		FN_0(bmpio_getType, TYPE_ENUM, GetType);
		VFN_1(bmpio_setType, SetType, TYPE_ENUM); 
	END_FUNCTION_MAP 
	};

static BitmapIO_Bmp_Imp bmpIOInterface(
		BMPIO_INTERFACE, _T("ibmpio"), IDS_BMPIO_INTERFACE, &BMPDesc, 0,
			BitmapIO_Bmp_Imp::bmpio_getType, _T("getType"), IDS_BMPIO_GETBITMAPTYPE, TYPE_ENUM, BitmapIO_Bmp_Imp::bmpio_type, 0, 0, 
			BitmapIO_Bmp_Imp::bmpio_setType, _T("setType"), IDS_BMPIO_SETBITMAPTYPE, TYPE_VOID, 0, 1, 
			_T("type"), 0, TYPE_ENUM, BitmapIO_Bmp_Imp::bmpio_type,
		enums,
			BitmapIO_Bmp_Imp::bmpio_type, 3,
				"noType",	BMM_NO_TYPE,
				"paletted",	BMM_PALETTED,
				"true24",	BMM_TRUE_24,
		end); 

int BitmapIO_Bmp_Imp::GetType() {
	int type = BMM_TRUE_24;
	BitmapIO_BMP* p = new BitmapIO_BMP;
	if (p) {
		p->ReadCfg();
		type = p->mParams.outDepth;
		delete p;
	}
	return type;
}

void BitmapIO_Bmp_Imp::SetType(int type) {
	BitmapIO_BMP* p = new BitmapIO_BMP;
	if (p) {
		p->ReadCfg();
		p->mParams.outDepth = type;
		p->WriteCfg();
		delete p;
	}
}

//-----------------------------------------------------------------------------
// Interface

DLLEXPORT const TCHAR * LibDescription ( )  { 
	return GetString(IDS_BMP_DESC); 
}

DLLEXPORT int LibNumberClasses ( ) { 
	return 1; 
}

DLLEXPORT ClassDesc *LibClassDesc(int i) {
	switch(i) {
		case  0: return &BMPDesc; break;
		default: return 0;        break;
	}
}

DLLEXPORT ULONG LibVersion ( )  { 
	return ( VERSION_3DSMAX ); 
}

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 0;
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
// #> BitmapIO_BMP::BitmapIO_BMP()

BitmapIO_BMP::BitmapIO_BMP( ) { 
	memset(&mParams,0,sizeof(BMPParams));
	mParams.outDepth	= BMM_TRUE_24;
	mParams.saved		= false;
}

BitmapIO_BMP::~BitmapIO_BMP ( ) { }

//-----------------------------------------------------------------------------
// #> BitmapIO_BMP::LongDesc()

const TCHAR *BitmapIO_BMP::LongDesc() {
	return GetString(IDS_BMP_FILE);
}
	
//-----------------------------------------------------------------------------
// #> BitmapIO_BMP::ShortDesc()

const TCHAR *BitmapIO_BMP::ShortDesc() {
	return GetString(IDS_BMP);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_BMP::LoadConfigure()

BOOL BitmapIO_BMP::LoadConfigure ( void *ptr ) {
    BMPParams *buf = (BMPParams*) ptr;
    memcpy(&mParams, ptr, sizeof(BMPParams));
    return TRUE;
}

//-----------------------------------------------------------------------------
// #> BitmapIO_BMP::SaveConfigure()

BOOL BitmapIO_BMP::SaveConfigure(void *ptr) {
    if (ptr) {
		mParams.saved = true;
		memcpy(ptr, &mParams, sizeof(BMPParams));
		return TRUE;
    } 
	return FALSE;
}

//-----------------------------------------------------------------------------
// #> BitmapIO_BMP::ShowAbout()

void BitmapIO_BMP::ShowAbout(HWND hWnd) {
	DialogBoxParam(
		hInst,
		MAKEINTRESOURCE(IDD_BMP_ABOUT),
		hWnd,
		(DLGPROC)AboutCtrlDlgProc,
		(LPARAM)0);
}


//-----------------------------------------------------------------------------
// #> BitmapIO_BMP::ConfigCtrlDlgProc

BOOL BitmapIO_BMP::ConfigCtrlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
    switch (message) {
		case WM_INITDIALOG:
			if (!mParams.saved)
				ReadCfg();
			switch (mParams.outDepth) {
				case BMM_PALETTED:
					CheckDlgButton( hWnd, IDC_BMP_PALETTE, TRUE );
					break;
				case BMM_NO_TYPE:
				case BMM_TRUE_24:
					CheckDlgButton( hWnd, IDC_BMP_RGB24, TRUE );
					break;
			}
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK: 
					WriteCfg();
					EndDialog(hWnd,1);
					break;
				case IDCANCEL:
					EndDialog(hWnd,0);
					break;
				case IDC_BMP_PALETTE:
					mParams.outDepth  = BMM_PALETTED;
					break;
				case IDC_BMP_RGB24:
					mParams.outDepth  = BMM_TRUE_24;
					break;
			}
			return TRUE;
	}
	return FALSE;
}

//-----------------------------------------------------------------------------
// #> StaticDialogProc

INT_PTR CALLBACK StaticDialogProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l) {
	BitmapIO_BMP* p;
	if (msg==WM_INITDIALOG) {
		p = (BitmapIO_BMP*)l;
        SetWindowLongPtr(hwnd, GWLP_USERDATA,l);
	} else  {
	    if ( (p = (BitmapIO_BMP*)GetWindowLongPtr(hwnd, GWLP_USERDATA) ) == NULL )
			return FALSE; 
	}
	return p->ConfigCtrlDlgProc(hwnd,msg,w,l);	
}

//-----------------------------------------------------------------------------
// #> BitmapIO_BMP::ShowControl()

BOOL BitmapIO_BMP::ShowControl(HWND hWnd, DWORD flag ) {
    return DialogBoxParam(
		hInst, 
		MAKEINTRESOURCE (IDD_BMP_CONFIG),
		hWnd, 
		(DLGPROC) StaticDialogProc, 
		(LPARAM) this);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_BMP::GetImageInfo()

BMMRES BitmapIO_BMP::GetImageInfo ( BitmapInfo *fbi ) {

	//-- Open BMP File -----------------------------------
	
	File file(fbi->Name(), _T("rb"));

	if (!file.stream)
		return (ProcessImageIOError(fbi));

	//-- Read File Header --------------------------------
	
	if (!ReadBimpHeader(file.stream))
		return (ProcessImageIOError(fbi,BMMRES_BADFILEHEADER));
	
	//-- Update Bitmap Info ------------------------------
	
	fbi->SetWidth( (WORD)bmi.biWidth );
	fbi->SetHeight((WORD)bmi.biHeight);
	
	switch (bmi.biBitCount) {
		case  1: fbi->SetType(BMM_LINE_ART); break;
		case  4: fbi->SetType(BMM_BMP_4);    break;
		case  8: fbi->SetType(BMM_PALETTED); break;
		case 16: fbi->SetType(BMM_TRUE_16);  break;
		case 24: fbi->SetType(BMM_TRUE_24);  break;
		case 32: fbi->SetType(BMM_PAD_24);   break;
		default: fbi->SetType(BMM_NO_TYPE);  break;
	}

//	fbi->SetGamma(1.0f);
	fbi->SetAspect(1.0f);
	fbi->SetFirstFrame(0);
	fbi->SetLastFrame(0);

	return BMMRES_SUCCESS;

}

//-----------------------------------------------------------------------------
//-- BitmapIO_BMP::Load()

BitmapStorage *BitmapIO_BMP::Load(BitmapInfo *fbi, Bitmap *map, BMMRES *status) 
{

	RGBQUAD      *rgb = NULL;
	BMM_Color_48 *pal = NULL;
	BitmapStorage  *s = NULL;
	BMM_Color_64   *b = NULL;
	BYTE           *p = NULL;
	BYTE		  *b8 = NULL;
	BYTE		  *b4 = NULL;


	int pixels = 0;
	int rows   = 0;
	int w      = 0;
	int wb     = 0;
	int h      = 0;
	int	j;

	//-- Initialize Status Optimistically

	*status = BMMRES_SUCCESS;

	//-- Make sure nothing weird is going on

	if(openMode != BMM_NOT_OPEN) {
		*status = ProcessImageIOError(fbi,BMMRES_INTERNALERROR);
		return NULL;
	}

	//-- Open BMP File -----------------------------------
	
	File file(fbi->Name(), _T("rb"));

	if (!file.stream) {
		*status = ProcessImageIOError(fbi);
		return(NULL);
	}

	//-- Read File Header --------------------------------
	
	if (!ReadBimpHeader(file.stream)) {
		*status = ProcessImageIOError(fbi,BMMRES_BADFILEHEADER);
		return (NULL);
	}
	
	//-- Update Bitmap Info ------------------------------
	
	fbi->SetWidth( (WORD)bmi.biWidth );
	fbi->SetHeight((WORD)bmi.biHeight);
	
	if ((bmi.biBitCount != 24 && bmi.biBitCount != 8 && bmi.biBitCount != 4) ||
		bmi.biCompression != BI_RGB) {
		*status = ProcessImageIOError(fbi,GetString(IDS_UNSUPPORTED));
		return(NULL);
	}

//	fbi->SetGamma(1.0f);
	fbi->SetAspect(1.0f);

	switch(bmi.biBitCount)
	{
		case 24:
			fbi->SetType(BMM_TRUE_24);
			break;
		
		case 8:
		case 4:

			//-- We don't have a 4 bit bitmap storage anyway. 
			//-- So force 4 bit to 8 bit

			fbi->SetType(BMM_PALETTED);
			break;
	}

	fbi->SetFirstFrame(0);
	fbi->SetLastFrame(0);
	
	//-- Create Image Storage ---------------------------- 
	
	switch(bmi.biBitCount)
	{
		case 24:
			s = BMMCreateStorage(map->Manager(),BMM_TRUE_32);
			break;

		case 8:
		case 4:

			//-- We don't have a 4 bit bitmap storage anyway. 
			//-- So force 4 bit storage to 8 bit storage

			s = BMMCreateStorage(map->Manager(),BMM_PALETTED);
			break;
	}

	if(!s) {
		*status = ProcessImageIOError(fbi,BMMRES_CANTSTORAGE);
		return NULL;
	}

	//-- Allocate Image Storage --------------------------
	
	if (s->Allocate(fbi,map->Manager(),BMM_OPEN_R)==0) {
		
		memory_error_out:
		*status = ProcessImageIOError(fbi,BMMRES_MEMORYERROR);
		goto bail_out;
	
		io_error_out:
		*status = ProcessImageIOError(fbi);
		bail_out:

		if (s) delete s;
		if (b) free(b);
		if (p) free(p);
		if (rgb) free(rgb);
		if (pal) free(pal);

		return NULL;

	}

	
	switch(bmi.biBitCount) 
	{
		case 4:	
			
			//-- Read 4 bit Palette ------------------------------------

			if (!bmi.biClrUsed) 
				bmi.biClrUsed = 16;
			rgb = (RGBQUAD *)malloc(bmi.biClrUsed * sizeof(RGBQUAD));
			if (!rgb)
				goto memory_error_out;
			pal = (BMM_Color_48 *)malloc(bmi.biClrUsed * sizeof(BMM_Color_48));
			if (!pal)
				goto memory_error_out;
			if (fread(rgb,sizeof(RGBQUAD),bmi.biClrUsed,file.stream) != bmi.biClrUsed)   
				goto io_error_out;
			for (j = 0; j < (int)bmi.biClrUsed; j++) 
			{
				pal[j].r = rgb[j].rgbRed   << 8;
				pal[j].g = rgb[j].rgbGreen << 8;
				pal[j].b = rgb[j].rgbBlue  << 8;
			}
	 
			s->SetPalette(0,bmi.biClrUsed,pal);
	 
			free(pal);
			free(rgb);
			pal = NULL;
			rgb = NULL;

			//-- Read Image (4 Bits) -----------------------------

			w	= fbi->Width();
			wb  = ( ((fbi->Width()+1)/2)+ 3) & ~3;	// width must be multiple of 4
			h   = fbi->Height() - 1;

			
			p   = (BYTE *)malloc(wb);
			b4  = (BYTE *)malloc(w);
			if (!p || !b4)
				goto memory_error_out;

			do 
			{
				pixels = fread(p,1,wb,file.stream);
	
				if (pixels != wb && pixels != 0)
					goto io_error_out;
				
				if (pixels)  
				{
					// -- the 4bit buffer p has two pixels per byte.
					// -- convert it to 8 bit buffer b8 that has one pixel per byte
					for(j=0;j<w;j++)
					{
						b4[j] = (j%2) ?  (p[j/2] & 0x0f) : (p[j/2] >> 4);
					}
					s->PutIndexPixels(0,(h - rows),w,b4);
					rows++;
					if (rows>h) break;
				}

				//-- Progress Report
			
				if (fbi->GetUpdateWindow())
					SendMessage(fbi->GetUpdateWindow(),BMM_PROGRESS,rows,h);

			} while (pixels);
			break;
		
		case 8:	
			
			//-- Read 8 bitPalette ------------------------------------

			if (!bmi.biClrUsed) 
				bmi.biClrUsed = 256;
			rgb = (RGBQUAD *)malloc(bmi.biClrUsed * sizeof(RGBQUAD));
			if (!rgb)
				goto memory_error_out;
			pal = (BMM_Color_48 *)malloc(bmi.biClrUsed * sizeof(BMM_Color_48));
			if (!pal)
				goto memory_error_out;
			if (fread(rgb,sizeof(RGBQUAD),bmi.biClrUsed,file.stream) != bmi.biClrUsed)   
				goto io_error_out;
			for ( j = 0; j < (int)bmi.biClrUsed; j++) 
			{
				pal[j].r = rgb[j].rgbRed   << 8;
				pal[j].g = rgb[j].rgbGreen << 8;
				pal[j].b = rgb[j].rgbBlue  << 8;
			}
	 
			s->SetPalette(0,bmi.biClrUsed,pal);
	 
			free(pal);
			free(rgb);
			pal = NULL;
			rgb = NULL;

			//-- Read Image (8 Bits) -----------------------------

			w    = (fbi->Width() + 3) & ~3;	// width must be multiple of 4
			h    = fbi->Height() - 1;
	
			p = (BYTE *)malloc(w);

			if (!p)
				goto memory_error_out;

			do 
			{
				pixels = fread(p,1,w,file.stream);
	
				if (pixels != w && pixels != 0)
					goto io_error_out;
				
				if (pixels)  
				{
					s->PutIndexPixels(0,(h - rows),fbi->Width(),p);
					rows++;
					if (rows>h) break;
				}

				//-- Progress Report
			
				if (fbi->GetUpdateWindow())
					SendMessage(fbi->GetUpdateWindow(),BMM_PROGRESS,rows,h);

			} while (pixels);
			break;

		case 24:
	
			//-- Read Image (24 Bits) ----------------------------

			w    = fbi->Width();
			wb   = (fbi->Width() * 3 + 3) & ~3;	// width bytes must be multiple of 4
			h    = fbi->Height() - 1;
	
			b = (BMM_Color_64  *)malloc(fbi->Width()*sizeof(BMM_Color_64));
			p = (BYTE          *)malloc(wb);

			if(!b || !p)
				goto memory_error_out;

			BYTE *ptr;
	
			do 
			{

				pixels = fread(p,1,wb,file.stream);

				if (pixels != wb && pixels != 0)
					goto io_error_out;
					
				if (pixels)  
				{
					ptr = p;
					for (int x = 0; x < w; x++) 
					{
						b[x].b = (WORD)((*ptr++) << 8);
						b[x].g = (WORD)((*ptr++) << 8);
						b[x].r = (WORD)((*ptr++) << 8);
					}
					if (s->PutPixels(0,(h - rows),w,b)!=1)
						goto io_error_out;
					rows++;
					if (rows>h) break;
				}

				//-- Progress Report
			
				if (fbi->GetUpdateWindow())
					SendMessage(fbi->GetUpdateWindow(),BMM_PROGRESS,rows,h);

			} while (pixels);
			break;
	}
	
	//-- Clean Up ----------------------------------------
	
	if (b) free(b);
	if (p) free(p);
	if (b8)free(b8);
	if (b4)free(b4);

	//-- Set the storage's BitmapInfo

	s->bi.CopyImageInfo(fbi);

	return  s;
	
}

//-----------------------------------------------------------------------------
// #> BitmapIO_BMP::OpenOutput()
//

BMMRES BitmapIO_BMP::OpenOutput(BitmapInfo *fbi, Bitmap *map) {
	if (openMode != BMM_NOT_OPEN)
		return (ProcessImageIOError(&bi,BMMRES_INTERNALERROR));
	if (!map)
		return (ProcessImageIOError(&bi,BMMRES_INTERNALERROR));
	//-- Save Image Info Data
	if (!mParams.saved)
		ReadCfg();
	bi.CopyImageInfo(fbi);    
	bi.SetUpdateWindow(fbi->GetUpdateWindow());
	this->map	= map;
	openMode	= BMM_OPEN_W;
	return (BMMRES_SUCCESS);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_BMP::Write()
//
//

BMMRES BitmapIO_BMP::Write(int frame) 
{

	BMMRES result = BMMRES_SUCCESS;
	
	//-- If we haven't gone through an OpenOutput(), leave

	if (openMode != BMM_OPEN_W)
		return (ProcessImageIOError(&bi,BMMRES_INTERNALERROR));

	//-- Resolve Filename --------------------------------

	TCHAR filename[MAX_PATH];

	if (frame == BMM_SINGLEFRAME) 
	{
		_tcscpy(filename,bi.Name());
	} 
	else 
	{
		if (!BMMCreateNumberedFilename(bi.Name(),frame,filename))
			return (ProcessImageIOError(&bi,BMMRES_NUMBEREDFILENAMEERROR));
	}
	
	//-- Create Image File -------------------------------
	
	File file(filename, _T("wb"));
	
	if (!file.stream)
		return (ProcessImageIOError(&bi));
	

	//-- Create File Header-------------------------------

	BITMAPFILEHEADER	hdr;
	PBITMAPINFO			pbmi;
	int					lx,y;
	
	memset(&hdr,0,sizeof(BITMAPFILEHEADER));
	hdr.bfType    = 0x4d42;
	hdr.bfOffBits = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER);


	//-- Pallette buffer and pixel buffer for 8bit output
	PixelBuf8			*pixBuf;
	BYTE				*pix;
	PixelBuf48			*palBuf;			
	BMM_Color_48		*pal;
	
	int w  = map->Width();
	int wb = (map->Width() + 3) & ~3;	// width must be multiple of 4
	int h  = map->Height();


	switch(mParams.outDepth)
	{

		//-- Paletted BMP required

		case BMM_PALETTED:
			pixBuf = new PixelBuf8(wb*h);
			palBuf = new PixelBuf48(256);
			
			if( (!pixBuf) || (!palBuf) )
				ProcessImageIOError(&bi,BMMRES_MEMORYERROR);
	
			pix = pixBuf->Ptr();
			pal = palBuf->Ptr();
			
			if( (!pix) || (!pal) )
				ProcessImageIOError(&bi,BMMRES_MEMORYERROR);
	
			if( Storage()->Paletted()) 
			{
				//-- Existing map is palletted...so get the pallete and 
				//-- the look up table..we are done..
				
				Storage()->GetPalette(0, 256, pal);
				for(y = 0; y < h; y++)	
					Storage()->GetIndexPixels(0,(h-y-1),w,pix+wb*y);		
			}
			else 
			{
				
				//-- Caluculate the pallete for the image..
				//-- Then create the look up table

				if(CalcOutputPalette(256,pal) == 0)
					ProcessImageIOError(&bi);
		
				PixelBuf64		line(w);
				ColorPacker*	cPack = BMMNewColorPacker(w,pal,256);
				for(y=0; y<h; y++) 
				{
					if(!GetOutputPixels(0,(h-y-1),w,line.Ptr()))
						ProcessImageIOError(&bi);
					cPack->PackLine(line.Ptr(),pix+y*wb,w);
				}
				cPack->DeleteThis();
			}
	
			//-- Fill in the BITMAPINFO structure---------------------

			lx = sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD) + wb*h;
			
			pbmi = (PBITMAPINFO)LocalAlloc(LPTR,lx);
			if (!pbmi)
				return (ProcessImageIOError(&bi,GetString(IDS_CONVERT_ERROR)));

			memset(pbmi,0,lx);

			pbmi->bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
			pbmi->bmiHeader.biWidth       = w;
			pbmi->bmiHeader.biHeight      = h;
			pbmi->bmiHeader.biPlanes      = 1;
			pbmi->bmiHeader.biBitCount    = 8;
			pbmi->bmiHeader.biCompression = BI_RGB;
			pbmi->bmiHeader.biSizeImage   = w*h; //256*sizeof(RGBQUAD) + wb*h;
			pbmi->bmiHeader.biXPelsPerMeter = 2834;
			pbmi->bmiHeader.biYPelsPerMeter = 2834;
			
			hdr.bfOffBits += 256*sizeof(RGBQUAD);  // DS 2/16/98

			RGBQUAD *rgb;

			//-- Fill in the palette

			rgb = (RGBQUAD*) &(pbmi->bmiColors[0]);
			for(y=0; y<256; y++)
			{
				rgb->rgbRed	  =  pal[y].r >> 8;
				rgb->rgbGreen =  pal[y].g >> 8;
				rgb->rgbBlue  =  pal[y].b >> 8;
				rgb++;
			}

			//-- Fill in the look up table

			memcpy((LPBYTE)rgb, pix,wb*h);

			break;

		//-- RGB24 requested--------------------------------------
		case BMM_NO_TYPE:
		case BMM_TRUE_24:
			{
			//-- Convert Bitmap to DIB ---------------------------
	
			pbmi = GetDitheredOutputDib();
			if (!pbmi)
				return (ProcessImageIOError(&bi,GetString(IDS_CONVERT_ERROR)));
	
	
			//-- Prepare Header ------------------------
	
			if (bi.GetUpdateWindow())
				SendMessage(bi.GetUpdateWindow(),BMM_PROGRESS,25,100);

			int rb  = (map->Width() * 3 + 3) & ~3;	// must be multiple of 4 bytes
			lx  = sizeof(BITMAPINFOHEADER) + (rb * map->Height());
			}
			break;
		default:
			assert(0);
			return BMMRES_IOERROR;
			break;

	}
		
	hdr.bfSize    = lx + sizeof(BITMAPFILEHEADER);
	
	//-- Write Header ------------------------

	int res = fwrite(&hdr,1,sizeof(BITMAPFILEHEADER),file.stream);

	if (res != sizeof(BITMAPFILEHEADER)) 
	{
		io_error:
		result = ProcessImageIOError(&bi);
		LocalFree(pbmi);
		return (result);
	}

	//-- Write Image File --------------------------------
	
	if (bi.GetUpdateWindow())
		SendMessage(bi.GetUpdateWindow(),BMM_PROGRESS,50,100);

	res = fwrite(pbmi,1,lx,file.stream);
	
	if (res != lx)
		goto io_error;
	
	LocalFree(pbmi);
	
	if (bi.GetUpdateWindow())
		SendMessage(bi.GetUpdateWindow(),BMM_PROGRESS,100,100);

	return (result);

}

//-----------------------------------------------------------------------------
// #> BitmapIO_BMP::Close()
//

int  BitmapIO_BMP::Close( int flag ) {

	return 1;

}

//-----------------------------------------------------------------------------
// #> BitmapIO_BMP::ReadBimpHeader()
//

int  BitmapIO_BMP::ReadBimpHeader( FILE *stream ) {

	//-- Read File Header --------------------------------

	int res = fread(&hdr,1,sizeof(BITMAPFILEHEADER),stream);

	if (res != sizeof(BITMAPFILEHEADER))
		return 0;

	//-- Read Bitmap Info Header -------------------------

	res = fread(&bmi,1,sizeof(BITMAPINFOHEADER),stream);

	if (res != sizeof(BITMAPINFOHEADER))
		return 0;

	//-- Validate ----------------------------------------

	if (hdr.bfType != 0x4d42)
		return 0;

	//-- Done
	
	return (1);

}

//-----------------------------------------------------------------------------
// #> BitmapIO_BMP::GetCfgFilename()
//

void BitmapIO_BMP::GetCfgFilename( TCHAR *filename ) {
	_tcscpy(filename,TheManager->GetDir(APP_PLUGCFG_DIR));
	int len = _tcslen(filename);
	if (len) {
		if (_tcscmp(&filename[len-1],_T("\\")))
			_tcscat(filename,_T("\\"));
	}   
	_tcscat(filename,BMPCONFIGNAME);   
}

//-----------------------------------------------------------------------------
// #> BitmapIO_BMP::ReadCfg()
//

BOOL BitmapIO_BMP::ReadCfg() {
	TCHAR filename[MAX_PATH];
	GetCfgFilename(filename);
	//-- Open Configuration File
	File file(filename, _T("rb"));
	if (!file.stream)
		return (FALSE);
	fseek(file.stream,0,SEEK_END);
	DWORD size = (DWORD)ftell(file.stream);
	if (size != EvaluateConfigure())
		return FALSE;
	fseek(file.stream,0,SEEK_SET);
	//-- Allocate Temp Buffer
	BYTE *buf = (BYTE *)LocalAlloc(LPTR,size);
	if (!buf)
		return FALSE;
	//-- Read Data Block and Set it
	BOOL res = FALSE;
	if (fread(buf,1,size,file.stream) == size)
		res = LoadConfigure(buf);
	LocalFree(buf);
	return (res);
}
	
//-----------------------------------------------------------------------------
// #> BitmapIO_BMP::WriteCfg()
//

void BitmapIO_BMP::WriteCfg() {
	TCHAR filename[MAX_PATH];
	GetCfgFilename(filename);
	BMPParams tbuf;
	tbuf = mParams;
	tbuf.saved = false;
	File file(filename, _T("wb"));
	if (file.stream) {
		fwrite(&tbuf,1,sizeof(BMPParams),file.stream);
	}
}

//-- EOF: bmp.cpp -------------------------------------------------------------
