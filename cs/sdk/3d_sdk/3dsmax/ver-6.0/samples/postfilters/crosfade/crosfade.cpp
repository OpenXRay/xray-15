//-----------------------------------------------------------------------------
// -----------------------
// File ....: CrosFade.cpp
// -----------------------
// Author...: Gus J Grubba
// Date ....: February 1996
// Descr....: CrosFade Transition
//
// History .: Feb, 08 1996 - Started
//            
//-----------------------------------------------------------------------------
		
//-- Include files

#include <Max.h>
#include <bmmlib.h>
#include <fltlib.h>
#include "CrosFade.h"
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
// CrosFade Class Description

class CROSFADEClassDesc:public ClassDesc {
	
	public:

		int             IsPublic     ( )                   { return 1;                }
		void           *Create       ( BOOL loading=FALSE) { return new ImageFilter_CrosFade; }
		const TCHAR    *ClassName    ( )                   { return GetString(IDS_DB_CROSSFADE);     }
		SClass_ID       SuperClassID ( )                   { return FLT_CLASS_ID;  }
		Class_ID        ClassID      ( )                   { return Class_ID(CROSFADECLASSID,0);    }
		const TCHAR    *Category     ( )                   { return GetString(IDS_DB_IMAGE_FILTER); }

};

static CROSFADEClassDesc CROSFADEDesc;

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
		case  0: return &CROSFADEDesc; break;
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

const TCHAR *ImageFilter_CrosFade::Description( ) 
{
	return GetString(IDS_DB_CROSS_FADE);
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
// #> ImageFilter_CrosFade::ShowAbout()

void ImageFilter_CrosFade::ShowAbout(HWND hWnd) {
	DialogBoxParam(
		hInst,
		MAKEINTRESOURCE(IDD_CROSFADE_ABOUT),
		hWnd,
		(DLGPROC)AboutCtrlDlgProc,
		(LPARAM)this);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_CrosFade::Render()
//
//    Private member Bitmap *srcmap has source bitmap
//    Private member Bitmap *frgmap has foreground bitmap
//    Private member Bitmap *mskmap has mask (if any)
//

BOOL ImageFilter_CrosFade::Render(HWND hWnd) {

	BMM_Color_64 *l64,*line64 = NULL;
	BMM_Color_64 *f64,*fgnd64 = NULL;
	WORD         *m16,*mask16 = NULL;

	if (!srcmap)
		return (FALSE);

	if (!frgmap)
		return (FALSE);

	//-- Allocate Background Line
	
	if ((line64=(BMM_Color_64 *)calloc(srcmap->Width(),sizeof(BMM_Color_64)))==NULL)
		return (FALSE);

	//-- Allocate Foreground Line
	
	if ((fgnd64=(BMM_Color_64 *)calloc(srcmap->Width(),sizeof(BMM_Color_64)))==NULL)
		return (FALSE);

	//-- Allocate Mask Line
	
	if (mskmap) {
		mask16 = (WORD *)calloc(srcmap->Width(),sizeof(WORD));
	}  

	BOOL result = TRUE;
	BOOL abort  = FALSE;
	
	//-- Calculate Lerp value

	float count = (float)ifi->FilterRange.Count();
	float lerp 	= (float)((ifi->QueueRange.Current() - ifi->FilterRange.First())) / count;
	
	//-- Process Image

	for (int iy = 0; iy < srcmap->Height(); iy++) {

		//-- Progress Report
		
		SendMessage(hWnd,FLT_PROGRESS,iy,srcmap->Height()-1);

		//-- Check for Abort
		
		SendMessage(hWnd,FLT_CHECKABORT,0,(LPARAM)(BOOL *)&abort);

		if (abort) {
			result = FALSE;
			break;
		}   
		
		l64 = line64;
		f64 = fgnd64;

		//-- Get Source line
		
		if (srcmap->GetLinearPixels(0,iy,srcmap->Width(),line64)!=1) {
			result = FALSE;
			break;
		}

		//-- Get Foreground line
		
		if (frgmap->GetLinearPixels(0,iy,srcmap->Width(),fgnd64)!=1) {
			result = FALSE;
			break;
		}

		//-- Handle Mask ----------------------------------
		
		if (mask16) {
			int r,g,b,a;
			
			if (mskmap->Get16Gray(0,iy,srcmap->Width(),mask16)!=1) {
				result = FALSE;
				break;
			}
			
			m16 = mask16;
			for (int ix = 0; ix < srcmap->Width(); ix++,m16++,l64++,f64++) {
				
				r = Lerp(l64->r, f64->r, lerp);
				g = Lerp(l64->g, f64->g, lerp);
				b = Lerp(l64->b, f64->b, lerp);
				a = Lerp(l64->a, f64->a, lerp);
				
				if (*m16 < 65530) {
					if (*m16) {
						l64->r = Lerp(r,(int)l64->r,(int)*m16);
						l64->g = Lerp(g,(int)l64->g,(int)*m16);
						l64->b = Lerp(b,(int)l64->b,(int)*m16);
						l64->a = Lerp(a,(int)l64->a,(int)*m16);
					} else {
						l64->r = r;
						l64->g = g;
						l64->b = b;
						l64->a = a;
					}
				}
			
			}
			
		//-- Unmasked -------------------------------------
		
		} else {
		
			for (int ix = 0; ix < srcmap->Width(); ix++,l64++,f64++) {

				l64->r = Lerp(l64->r, f64->r, lerp);
				l64->g = Lerp(l64->g, f64->g, lerp);
				l64->b = Lerp(l64->b, f64->b, lerp);
				l64->a = Lerp(l64->a, f64->a, lerp);

			}
			
		}

		//-- Output Line ----------------------------------
		
		if (srcmap->PutPixels(0,iy,srcmap->Width(),line64)!=1) {
			result = FALSE;
			break;
		}

	}
	
	if (mask16)
		free(mask16);
		
	if (line64)
		free(line64);
		
	if (fgnd64)
		free(fgnd64);
		
	return(result);

}

//-- EOF: CrosFade.cpp --------------------------------------------------------


































