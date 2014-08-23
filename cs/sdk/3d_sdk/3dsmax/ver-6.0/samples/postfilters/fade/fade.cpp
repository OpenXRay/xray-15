//-----------------------------------------------------------------------------
// -------------------
// File ....: Fade.cpp
// -------------------
// Author...: Gus J Grubba
// Date ....: February 1996
// Descr....: Fade Image Filter
//
// History .: Feb, 21 1995 - Started
//            
//-----------------------------------------------------------------------------
        
//-- Include files

#include <Max.h>
#include <bmmlib.h>
#include <fltlib.h>
#include "Fade.h"
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
// Fade Class Description

class FADEClassDesc:public ClassDesc {
     
     public:

        int             IsPublic     ( )                   { return 1;                }
        void           *Create       ( BOOL loading=FALSE) { return new ImageFilter_Fade; }
        const TCHAR    *ClassName    ( )                   { return GetString(IDS_DB_FADE);     }
        SClass_ID       SuperClassID ( )                   { return FLT_CLASS_ID;  }
        Class_ID        ClassID      ( )                   { return Class_ID(FADECLASSID,0);    }
        const TCHAR    *Category     ( )                   { return GetString(IDS_DB_IMAGE_FILTER); }

};

static FADEClassDesc FADEDesc;

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
        case  0: return &FADEDesc; break;
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

const TCHAR *ImageFilter_Fade::Description( ) 
{
	return GetString(IDS_DB_FADE);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Fade::ImageFilter_Fade()
//

ImageFilter_Fade::ImageFilter_Fade()	{
	data.version= FADEVERSION; 
	data.type	= IDC_OUT;
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Fade::Control()
//

BOOL	ImageFilter_Fade::Control(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)	{

	switch (message) {

		case WM_INITDIALOG:

			CenterWindow(hWnd,GetParent(hWnd));
			SetCursor(LoadCursor(NULL,IDC_ARROW));

			CheckRadioButton(
				hWnd,
				IDC_IN,
				IDC_OUT,
				data.type
			);

			return 1;

		case WM_COMMAND:

			switch (LOWORD(wParam)) {

				case IDOK:
					data.type = (IsDlgButtonChecked(hWnd,IDC_IN) ? IDC_IN : IDC_OUT);
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
// *> ControlDlgProc()
//

INT_PTR	CALLBACK	ControlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)	{
     static ImageFilter_Fade *flt = NULL;
     if (message == WM_INITDIALOG) 
        flt = (ImageFilter_Fade *)lParam;
     if (flt) 
        return (flt->Control(hWnd,message,wParam,lParam));
     else
        return(FALSE);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Fade::ShowControl()

BOOL ImageFilter_Fade::ShowControl(HWND hWnd) {
     return (DialogBoxParam(
        hInst,
        MAKEINTRESOURCE(IDD_FADE_CONTROL),
        hWnd,
        (DLGPROC)ControlDlgProc,
        (LPARAM)this));
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
// #> ImageFilter_Fade::ShowAbout()

void ImageFilter_Fade::ShowAbout(HWND hWnd) {
     DialogBoxParam(
        hInst,
        MAKEINTRESOURCE(IDD_FADE_ABOUT),
        hWnd,
        (DLGPROC)AboutCtrlDlgProc,
        (LPARAM)this);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Fade::Render()
//

BOOL ImageFilter_Fade::Render(HWND hWnd) {

     BMM_Color_64 *l64,*line64 = NULL;
     WORD         *m16,*mask16 = NULL;

     BOOL result = TRUE;
     BOOL abort  = FALSE;
     
     if (!srcmap)
        return (FALSE);

     if ((line64=(BMM_Color_64 *)calloc(srcmap->Width(),sizeof(BMM_Color_64)))==NULL)
        return (FALSE);

     if (mskmap)
        mask16 = (WORD *)calloc(srcmap->Width(),sizeof(WORD));

	//-- Calculate Lerp value

	float count = (float)(max(ifi->FilterRange.Count()-1,1));
	float lerp 	= (float)((ifi->QueueRange.Current() - ifi->FilterRange.First())) / count;

	if (data.type == IDC_OUT)
		lerp = 1.0f - lerp;
     
     for (int iy = 0; iy < srcmap->Height(); iy++) {

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

        //-- Handle Mask
        
        if (mask16) {
           int r,g,b,a;
           
           if (mskmap->Get16Gray(0,iy,srcmap->Width(),mask16)!=1)
				goto done;
           
           m16 = mask16;
           for (int ix = 0; ix < srcmap->Width(); ix++,l64++,m16++) {
              r = (int)((float)l64->r * lerp);
              g = (int)((float)l64->g * lerp);
              b = (int)((float)l64->b * lerp);
              a = (int)((float)l64->a * lerp);
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
           
        //-- Unmasked
        
        } else {
        
           for (int ix = 0; ix < srcmap->Width(); ix++,l64++) {
              l64->r = (int)((float)l64->r * lerp);
              l64->g = (int)((float)l64->g * lerp);
              l64->b = (int)((float)l64->b * lerp);
              l64->a = (int)((float)l64->a * lerp);
           }
           
        }

        //-- Output Line
        
        if (srcmap->PutPixels(0,iy,srcmap->Width(),line64)!=1)
			goto done;

     }

	result = TRUE;
     
	done:

     if (mask16)
        free(mask16);
        
     if (line64)
        free(line64);
        
     return(result);

}

//-----------------------------------------------------------------------------
// #> ImageFilter_Fade::LoadConfigure()

BOOL ImageFilter_Fade::LoadConfigure ( void *ptr ) {
     FADEDATA *buf = (FADEDATA *)ptr;
     if (buf->version == FADEVERSION) {
        memcpy((void *)&data,ptr,sizeof(FADEDATA));
        return (TRUE);
     } else
        return (FALSE);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Fade::SaveConfigure()

BOOL ImageFilter_Fade::SaveConfigure ( void *ptr ) {
     if (ptr) {
        memcpy(ptr,(void *)&data,sizeof(FADEDATA));
        return (TRUE);
     } else
        return (FALSE);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Fade::EvaluateConfigure()

DWORD ImageFilter_Fade::EvaluateConfigure ( ) {
      return (sizeof(FADEDATA));
}

//-- EOF: Fade.cpp ------------------------------------------------------------
