//-----------------------------------------------------------------------------
// ------------------
// File ....: yuv.cpp
// ------------------
// Author...: Gus J Grubba
// Date ....: July 1995
// Descr....: YUV File I/O Module
//
// History .: Jul, 27 1995 - Started
//            
//-----------------------------------------------------------------------------
        
//-- Include files

#include <Max.h>
#include <bmmlib.h>
#include "yuv.h"
#include "yuvrc.h"

//-- Prototypes

//static BOOL CenterWindow(HWND hWndChild, HWND hWndParent);

//-- Handy macros

#define limit(x) {                      \
    if(x > 0xFFFFFF) x = 0xFFFFFF;      \
    if(x <=  0xFFFF) x = 0;             \
    x  &=  0xFF0000;                    \
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
// YUV Class Description

class YUVClassDesc:public ClassDesc {
     
     public:

        int             IsPublic     ( )                   { return 1;                }
        void           *Create       ( BOOL loading=FALSE) { return new BitmapIO_YUV; }
        const TCHAR    *ClassName    ( )                   { return GetString(IDS_YUV);     }
        SClass_ID       SuperClassID ( )                   { return BMM_IO_CLASS_ID;  }
        Class_ID        ClassID      ( )                   { return Class_ID(YUVCLASSID,0);    }
        const TCHAR    *Category     ( )                   { return GetString(IDS_BITMAP_IO); }

};

static YUVClassDesc YUVDesc;

//-----------------------------------------------------------------------------
// Interface

DLLEXPORT const TCHAR * LibDescription ( )  { 
     return GetString(IDS_LIBDESCRIPTION); 
}

DLLEXPORT int LibNumberClasses ( ) { 
     return 1; 
}

DLLEXPORT ClassDesc *LibClassDesc(int i) {
     switch(i) {
        case  0: return &YUVDesc; break;
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
// #> BitmapIO_YUV::BitmapIO_YUV()

BitmapIO_YUV::BitmapIO_YUV  ( ) { }

BitmapIO_YUV::~BitmapIO_YUV ( ) { }

//BitmapIO_YUV::BitmapIO_YUV(BitmapStorage *s,BitmapIO *previous,int frame) : BitmapIO(s,previous,frame) { }

//-----------------------------------------------------------------------------
// #> BitmapIO_YUV::LongDesc()

const TCHAR *BitmapIO_YUV::LongDesc() {
     return GetString(IDS_YUV_FILE);
}
     
//-----------------------------------------------------------------------------
// #> BitmapIO_YUV::ShortDesc()

const TCHAR *BitmapIO_YUV::ShortDesc() {
     return GetString(IDS_YUV);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_YUV::LoadConfigure()

BOOL BitmapIO_YUV::LoadConfigure ( void *ptr ) {
     return 1;
}

//-----------------------------------------------------------------------------
// #> BitmapIO_YUV::SaveConfigure()

BOOL BitmapIO_YUV::SaveConfigure ( void *ptr ) {
     return 1;
}

//-----------------------------------------------------------------------------
// #> BitmapIO_YUV::ShowAbout()

void BitmapIO_YUV::ShowAbout(HWND hWnd) {
     DialogBoxParam(
        hInst,
        MAKEINTRESOURCE(IDD_YUV_ABOUT),
        hWnd,
        (DLGPROC)AboutCtrlDlgProc,
        (LPARAM)0);
}

//-----------------------------------------------------------------------------
// #> BitmapIO_YUV::Control()

BOOL BitmapIO_YUV::ShowControl(HWND hWnd, DWORD flag ) {
     return (
        DialogBoxParam(
        hInst,
        MAKEINTRESOURCE(IDD_YUV_CONTROL),
        hWnd,
        (DLGPROC)ControlCtrlDlgProc,
        (LPARAM)0)
     );
}

//-----------------------------------------------------------------------------
// #> BitmapIO_YUV::GetImageInfo()

BMMRES BitmapIO_YUV::GetImageInfo ( BitmapInfo *fbi ) {

	//-- Get File size

     HANDLE findhandle;
     WIN32_FIND_DATA file;
     findhandle = FindFirstFile(fbi->Name(),&file);
     if (findhandle == INVALID_HANDLE_VALUE)
        return BMMRES_FILENOTFOUND;

     //-- Fill up BitmapInfo 

     if (file.nFileSizeLow == WIDTH * NHEIGHT * 2) {
        fbi->SetWidth(WIDTH);
        fbi->SetHeight(NHEIGHT);
     } else if (file.nFileSizeLow == WIDTH * PHEIGHT * 2) {
        fbi->SetWidth(WIDTH);
        fbi->SetHeight(PHEIGHT);
     } else {
        fbi->SetWidth(WIDTH);
        fbi->SetHeight((WORD)(file.nFileSizeLow / (WIDTH * 2)));
     }
     
     //fbi->SetGamma(1.8f);
     fbi->SetAspect( 1.3333333f / (float)(fbi->Width()) / (float)(fbi->Height()));
     fbi->SetType(BMM_YUV_422);
     fbi->SetFirstFrame(0);
     fbi->SetLastFrame(0);

     FindClose(findhandle);

     return BMMRES_SUCCESS;

}

//-----------------------------------------------------------------------------
//-- BitmapIO_YUV::Load()

BitmapStorage *BitmapIO_YUV::Load(BitmapInfo *fbi, Bitmap *map, BMMRES *status) {

     unsigned char *yuvbuf = NULL;
     BMM_Color_64  *rgbbuf = NULL;
     BitmapStorage *s      = NULL;

	//-- Initialize Status Optimistically

	*status = BMMRES_SUCCESS;

	//-- Make sure nothing weird is going on

	if(openMode != BMM_NOT_OPEN) {
		*status = ProcessImageIOError(fbi,BMMRES_INTERNALERROR);
		return NULL;
	}

     //-- Update Bitmap Info
     
     *status = GetImageInfo(fbi);
     
     if (*status != BMMRES_SUCCESS)
        return(NULL);

     //-- Open YUV File -----------------------------------
     
     File file(fbi->Name(), _T("rb"));

     if (!(inStream = file.stream)) {
		*status = ProcessImageIOError(fbi);
        return NULL;
     }

     //-- Create Image Storage ---------------------------- 
     
     s = BMMCreateStorage(map->Manager(),BMM_TRUE_32);

     if(!s) {
		*status = ProcessImageIOError(fbi,BMMRES_INTERNALERROR);
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
        if (s) 
           delete s;
        if (yuvbuf)
           free(yuvbuf);
        if (rgbbuf)
           free(rgbbuf);
        return NULL;
     }

     //-- Allocate Buffers --------------------------------
     
     yuvbuf=(unsigned char *)malloc(fbi->Width()*2);
     rgbbuf=(BMM_Color_64  *)malloc(fbi->Width()*sizeof(BMM_Color_64));

     if(!yuvbuf || !rgbbuf)
        goto memory_error_out;
     
     //-- Read Image

     int pixels = fbi->Width() * fbi->Height();
     int rows   = 0;
     
     while (pixels) {
        pixels = fread(yuvbuf,2,fbi->Width(),inStream);
        if (pixels != fbi->Width() && pixels != 0)  {
           goto io_error_out;
        }
        if (pixels)  {
           YUVtoRGB(rgbbuf,yuvbuf,fbi->Width());
           if (s->PutPixels(0,rows,fbi->Width(),rgbbuf)!=1)
              goto io_error_out;
           rows++;
           if (rows>fbi->Height()) break;
        }   

        //-- Progress Report
        
        if (fbi->GetUpdateWindow())
           SendMessage(fbi->GetUpdateWindow(),BMM_PROGRESS,rows,fbi->Height());

     }
     
     if (yuvbuf)
        free(yuvbuf);
     if (rgbbuf)
        free(rgbbuf);

     //-- Set the storage's BitmapInfo

     s->bi.CopyImageInfo(fbi);
     return  s;

}

//-----------------------------------------------------------------------------
// #> BitmapIO_YUV::OpenOutput()
//

BMMRES BitmapIO_YUV::OpenOutput(BitmapInfo *fbi, Bitmap *map) {

	if (openMode != BMM_NOT_OPEN)
		return (ProcessImageIOError(fbi,BMMRES_INTERNALERROR));
		
	if (!map)
		return (ProcessImageIOError(fbi,BMMRES_INTERNALERROR));
		
     //-- Save Image Info Data

     bi.CopyImageInfo(fbi);    

     this->map   = map;
     openMode    = BMM_OPEN_W;

     return BMMRES_SUCCESS;

}

//-----------------------------------------------------------------------------
// #> BitmapIO_YUV::Write()
//
//

BMMRES BitmapIO_YUV::Write(int frame) {

	//-- If we haven't gone through an OpenOutput(), leave

	if (openMode != BMM_OPEN_W)
		return (ProcessImageIOError(&bi,BMMRES_INTERNALERROR));


	return (ProcessImageIOError(&bi,BMMRES_INTERNALERROR));
	
}

//-----------------------------------------------------------------------------
// #> BitmapIO_YUV::Close()
//

int  BitmapIO_YUV::Close( int flag ) {

     return 1;

}

//-----------------------------------------------------------------------------
// #> BitmapIO_YUV::YUVtoRGB()
//

void BitmapIO_YUV::YUVtoRGB (BMM_Color_64 *rgb, unsigned char *yuv, int len) {

     int j;  
     long y, u, v, y1, r, g, b;

     for(j=0;j<(WIDTH/2);j++) {
        
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

//-- EOF: yuv.cpp -------------------------------------------------------------
