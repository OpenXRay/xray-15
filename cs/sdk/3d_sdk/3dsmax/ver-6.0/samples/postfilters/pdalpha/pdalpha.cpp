//-----------------------------------------------------------------------------
// ----------------------
// File ....: PdAlpha.cpp
// ----------------------
// Author...: Gus J Grubba
// Date ....: February 1996
// Descr....: Pseudo Alpha Image Filter
//
// History .: Feb, 23 1995 - Started
//            
//-----------------------------------------------------------------------------
        
//-- Include files

#include <Max.h>
#include <bmmlib.h>
#include <fltlib.h>
#include "PdAlpha.h"
#include "resource.h"

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
// PdAlpha Class Description

class PDALPHAClassDesc:public ClassDesc {
     
     public:

        int             IsPublic     ( )                   { return 1;                }
        void           *Create       ( BOOL loading=FALSE) { return new ImageFilter_PdAlpha; }
        const TCHAR    *ClassName    ( )                   { return GetString(IDS_DB_PDALPHA);     }
        SClass_ID       SuperClassID ( )                   { return FLT_CLASS_ID;  }
        Class_ID        ClassID      ( )                   { return Class_ID(PDALPHACLASSID,0);    }
        const TCHAR    *Category     ( )                   { return GetString(IDS_DB_IMAGE_FILTER); }

};

static PDALPHAClassDesc PDALPHADesc;

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
        case  0: return &PDALPHADesc; break;
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

const TCHAR *ImageFilter_PdAlpha::Description( ) 
{
	return GetString(IDS_DB_PSEUDO_ALPHA);
}

//-----------------------------------------------------------------------------
// *> AboutCtrlDlgProc()
//

INT_PTR CALLBACK AboutCtrlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {

     switch (message) {
        
        case WM_INITDIALOG: {
             CenterWindow(hWnd,GetParent(hWnd));
             SetCursor(LoadCursor(NULL,IDC_ARROW));
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
// #> ImageFilter_PdAlpha::ShowAbout()

void ImageFilter_PdAlpha::ShowAbout(HWND hWnd) {
     DialogBoxParam(
        hInst,
        MAKEINTRESOURCE(IDD_PDALPHA_ABOUT),
        hWnd,
        (DLGPROC)AboutCtrlDlgProc,
        (LPARAM)this);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_PdAlpha::Render()
//

BOOL ImageFilter_PdAlpha::Render(HWND hWnd) {

     BMM_Color_64 pixel;
     BMM_Color_64 *l64,*line64 = NULL;
     BMM_Color_64 *f64,*fgnd64 = NULL;

     BOOL result = TRUE;
     BOOL abort  = FALSE;
     
	  int iy,ix;

     if (!srcmap)
        goto done;

	//-- Allocate Line Buffers -------------------------------------

     if ((line64=(BMM_Color_64 *)calloc(srcmap->Width(),sizeof(BMM_Color_64)))==NULL)
        goto done;

     if (frgmap) {
     	if ((fgnd64=(BMM_Color_64 *)calloc(srcmap->Width(),sizeof(BMM_Color_64)))==NULL)
        	goto done;
	}

	//-- Get Key ---------------------------------------------------

	if (!frgmap) {
		if (srcmap->GetLinearPixels(0,0,1,&pixel)!=1)
			goto done;
	} else {
		if (frgmap->GetLinearPixels(0,0,1,&pixel)!=1)
			goto done;
	}

	//-- Process ---------------------------------------------------

     for (iy = 0; iy < srcmap->Height(); iy++) {

        //-- Progress Report
        
        SendMessage(hWnd,FLT_PROGRESS,iy,srcmap->Height()-1);

        //-- Check for Abort
        
        SendMessage(hWnd,FLT_CHECKABORT,0,(LPARAM)(BOOL *)&abort);

        if (abort)
			goto done;
        
        //-- Get line
        
        l64 = line64;

        if (srcmap->GetLinearPixels(0,iy,srcmap->Width(),line64)!=1)
			goto done;

		//-- Working as a filter ------------------------------------

		if (!frgmap) {

        	for (ix = 0; ix < srcmap->Width(); ix++,l64++) {
        	   if (l64->r == pixel.r && l64->g == pixel.g && l64->b == pixel.b)
        	   	l64->a = 0;
				else
        	   	l64->a = 0xFFFF;
        	}

		//-- Working as a layer -------------------------------------

		} else {
		
        	f64 = fgnd64;

			if (frgmap->GetLinearPixels(0,iy,srcmap->Width(),fgnd64)!=1)
				goto done;

        	for (ix = 0; ix < srcmap->Width(); ix++,l64++,f64++) {
        	   if (f64->r != pixel.r && f64->g != pixel.g && f64->b != pixel.b)
        	   	*l64 = *f64;
        	}

		}
           
        //-- Output Line
        
        if (srcmap->PutPixels(0,iy,srcmap->Width(),line64)!=1)
			goto done;

     }

	result = TRUE;
     
	done:

	if (line64)
		free(line64);
        
	if (fgnd64)
		free(fgnd64);
		
     return(result);

}

//-- EOF: PdAlpha.cpp ------------------------------------------------------------
