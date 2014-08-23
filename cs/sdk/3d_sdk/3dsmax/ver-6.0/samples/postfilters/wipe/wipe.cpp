//-----------------------------------------------------------------------------
// -------------------
// File ....: Wipe.cpp
// -------------------
// Author...: Gus J Grubba
// Date ....: February 1996
// Descr....: Wipe Transition
//
// History .: Feb, 18 1996 - Started
//            
//-----------------------------------------------------------------------------
        
//-- Include files

#include <Max.h>
#include <bmmlib.h>
#include <fltlib.h>
#include "Wipe.h"
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
// Wipe Class Description

class WIPEClassDesc:public ClassDesc {
     
     public:

        int             IsPublic     ( )                   { return 1;                }
        void           *Create       ( BOOL loading=FALSE) { return new ImageFilter_Wipe; }
        const TCHAR    *ClassName    ( )                   { return GetString(IDS_DB_WIPE);     }
        SClass_ID       SuperClassID ( )                   { return FLT_CLASS_ID;  }
        Class_ID        ClassID      ( )                   { return Class_ID(WIPECLASSID,0);    }
        const TCHAR    *Category     ( )                   { return GetString(IDS_DB_IMAGE_FILTER); }

};

static WIPEClassDesc WIPEDesc;

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
        case  0: return &WIPEDesc; break;
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

const TCHAR *ImageFilter_Wipe::Description( ) 
{
	return GetString(IDS_DB_SIMPLE_WIPE);
}

//-----------------------------------------------------------------------------
// *> ControlDlgProc()
//

INT_PTR	CALLBACK	ControlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)	{
     static ImageFilter_Wipe *flt = NULL;
     if (message == WM_INITDIALOG) 
        flt = (ImageFilter_Wipe *)lParam;
     if (flt) 
        return (flt->Control(hWnd,message,wParam,lParam));
     else
        return(FALSE);
}

//-----------------------------------------------------------------------------
// *> AboutCtrlDlgProc()
//

BOOL CALLBACK AboutCtrlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
	switch (message) {
        case WM_INITDIALOG:
			CenterWindow(hWnd,GetParent(hWnd));
			SetCursor(LoadCursor(NULL,IDC_ARROW));
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
// #> ImageFilter_Wipe::ImageFilter_Wipe()
//

ImageFilter_Wipe::ImageFilter_Wipe() {
	data.version	= WIPEVERSION;
	data.type		= IDC_RIGHT;
	data.reverse	= FALSE;
	data.overlap	= TRUE;
}

//-----------------------------------------------------------------------------
// *> ToggleItem()
//

void ToggleItem ( HWND hWnd, int item, BOOL flag ) {
	HWND hDlg = GetDlgItem(hWnd,item);
	if (flag)
		ShowWindow(hDlg,SW_SHOW);
	else
		ShowWindow(hDlg,SW_HIDE);
}

//-----------------------------------------------------------------------------
// *> HandleIcons()
//

void HandleIcons( HWND hWnd, BOOL reverse ) {
	ToggleItem(hWnd,IDC_RIGHT_POS,reverse);
	ToggleItem(hWnd,IDC_LEFT_POS,	reverse);
	ToggleItem(hWnd,IDC_RIGHT_NEG,!reverse);
	ToggleItem(hWnd,IDC_LEFT_NEG,	!reverse);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Wipe::Control()
//

BOOL	ImageFilter_Wipe::Control(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)	{

	static WIPEDATA tempdata;

	switch (message) {

		case WM_INITDIALOG:

			CenterWindow(hWnd,GetParent(hWnd));
			SetCursor(LoadCursor(NULL,IDC_ARROW));

			tempdata = data;

			//-- Handle Direction ----------------

			CheckRadioButton(
				hWnd,
				IDC_RIGHT,
				IDC_DOWN,
				tempdata.type
			);

			//-- Options -------------------------

			CheckRadioButton(
				hWnd,
				IDC_REVERSE,
				IDC_NORMAL,
				(tempdata.reverse ? IDC_REVERSE : IDC_NORMAL)
			);

			HandleIcons(hWnd,tempdata.reverse);

			return 1;

		case WM_COMMAND:

			switch (LOWORD(wParam)) {

				case IDC_RIGHT:
				case IDC_LEFT:
					tempdata.type = LOWORD(wParam);
					break;

				case IDC_REVERSE:
				case IDC_NORMAL:
					tempdata.reverse = IsDlgButtonChecked(hWnd,IDC_REVERSE);
					HandleIcons(hWnd,tempdata.reverse);
					break;

				case IDOK:
					data = tempdata;
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
// #> ImageFilter_Wipe::ShowAbout()

void ImageFilter_Wipe::ShowAbout(HWND hWnd) {
     DialogBoxParam(
        hInst,
        MAKEINTRESOURCE(IDD_WIPE_ABOUT),
        hWnd,
        (DLGPROC)AboutCtrlDlgProc,
        (LPARAM)this);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Wipe::ShowControl()

BOOL ImageFilter_Wipe::ShowControl(HWND hWnd) {
     return (DialogBoxParam(
        hInst,
        MAKEINTRESOURCE(IDD_WIPE_CONTROL),
        hWnd,
        (DLGPROC)ControlDlgProc,
        (LPARAM)this));
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Wipe::Render()
//
//	 This filter act as both a filter and as a layer. If the foreground map is
//	 present, we are a layer and we use it. Otherwise, we simply use a pseudo
//	 (black) map.
//

BOOL ImageFilter_Wipe::Render(HWND hWnd) {

	BMM_Color_64 *l64,*line64 = NULL;
	BMM_Color_64 *f64,*fgnd64 = NULL;

	int iy,x0,x1;

	BOOL abort  = FALSE;
	BOOL result = FALSE;

	//-- Calculate Lerp value

	float count = (float)(max(ifi->FilterRange.Count()-1,1));
	float lerp 	= (float)((ifi->QueueRange.Current() - ifi->FilterRange.First())) / count;
     
     if (!srcmap)
        goto done;

     //-- Allocate Background Line
     
     if ((line64=(BMM_Color_64 *)calloc(srcmap->Width(),sizeof(BMM_Color_64)))==NULL)
        goto done;

     //-- Allocate Foreground Line
     
     if ((fgnd64=(BMM_Color_64 *)calloc(srcmap->Width(),sizeof(BMM_Color_64)))==NULL)
        goto done;

	//------------------------------------------------------------------------
	//-- Process Image -------------------------------------------------------

	for (iy = 0; iy < srcmap->Height(); iy++) {

        //-- Progress Report
        
        SendMessage(hWnd,FLT_PROGRESS,iy,srcmap->Height()-1);

        //-- Check for Abort
        
        SendMessage(hWnd,FLT_CHECKABORT,0,(LPARAM)(BOOL *)&abort);

        if (abort)
        	goto done;
        
        l64 = line64;
        f64 = fgnd64;

        //-- Get Source line
        
		if (!data.overlap) {
        	if (srcmap->GetLinearPixels(0,iy,srcmap->Width(),line64)!=1)
        		goto done;
		}

		//-- Push Image Into Background - Left to Right (Layer) ------------

		if (data.type == IDC_RIGHT && data.reverse && frgmap) {
     	   
			if (frgmap->GetLinearPixels(0,iy,srcmap->Width(),fgnd64)!=1)
				goto done;
			x1 = (int)(lerp * (float)srcmap->Width());
			x0 = max(srcmap->Width() - x1 - 1,0);
			if (x1) srcmap->PutPixels(0,iy,x1,&fgnd64[x0]);

		//-- Push Image Out of Background - Left to Right (Layer) ----------

		} else if (data.type == IDC_RIGHT && !data.reverse && frgmap) {

			if (frgmap->GetLinearPixels(0,iy,srcmap->Width(),fgnd64)!=1)
				goto done;
			x1 = srcmap->Width() - (int)(lerp * (float)srcmap->Width());
			x0 = max(srcmap->Width() - x1 - 1,0);
			if (x1) srcmap->PutPixels(x0,iy,x1,fgnd64);

		//-- Push Image Into Background - Right to Left (Layer) ------------

		} else if (data.type == IDC_LEFT && data.reverse && frgmap) {

			if (frgmap->GetLinearPixels(0,iy,srcmap->Width(),fgnd64)!=1)
				goto done;
		  	x1 = (int)(lerp * (float)srcmap->Width());
		  	x0 = max(srcmap->Width() - x1 - 1,0);
		  	if (x1) srcmap->PutPixels(x0,iy,x1,fgnd64);

		//-- Push Image Out of Background - Right to Left (Layer) ----------

		} else if (data.type == IDC_LEFT && !data.reverse && frgmap) {
	
			if (frgmap->GetLinearPixels(0,iy,srcmap->Width(),fgnd64)!=1)
				goto done;
			x1 = srcmap->Width() - (int)(lerp * (float)srcmap->Width());
			x0 = max(srcmap->Width() - x1 - 1,0);
			if (x1) srcmap->PutPixels(0,iy,x1,&fgnd64[x0]);

		//-- Push Black Into Background - Left to Right (Filter) -----------

		} else if (data.type == IDC_RIGHT && !data.reverse && !frgmap) {
     	   
			x1 = (int)(lerp * (float)srcmap->Width());
			x0 = max(srcmap->Width() - x1 - 1,0);
			if (x1) srcmap->PutPixels(0,iy,x1,&fgnd64[x0]);

		//-- Push Black Out of Background - Left to Right (Filter) ---------

		} else if (data.type == IDC_RIGHT && data.reverse && !frgmap) {

			x1 = srcmap->Width() - (int)(lerp * (float)srcmap->Width());
			x0 = max(srcmap->Width() - x1 - 1,0);
			if (x1) srcmap->PutPixels(x0,iy,x1,fgnd64);

		//-- Push Black Into Background - Right to Left (Filter) -----------

		} else if (data.type == IDC_LEFT && !data.reverse && !frgmap) {

		  	x1 = (int)(lerp * (float)srcmap->Width());
		  	x0 = max(srcmap->Width() - x1 - 1,0);
		  	if (x1) srcmap->PutPixels(x0,iy,x1,fgnd64);

		//-- Push Black Out of Background - Right to Left (Filter) ---------

		} else if (data.type == IDC_LEFT && data.reverse && !frgmap) {
	
			x1 = srcmap->Width() - (int)(lerp * (float)srcmap->Width());
			x0 = max(srcmap->Width() - x1 - 1,0);
			if (x1) srcmap->PutPixels(0,iy,x1,&fgnd64[x0]);
		
		}

	}
		
   	result = TRUE;

   	done:
     
   	if (line64)
   		free(line64);
        
   	if (fgnd64)
   		free(fgnd64);
        
   	return(result);

}

//-----------------------------------------------------------------------------
// #> ImageFilter_Wipe::LoadConfigure()

BOOL ImageFilter_Wipe::LoadConfigure ( void *ptr ) {
     WIPEDATA *buf = (WIPEDATA *)ptr;
     if (buf->version == WIPEVERSION) {
        memcpy((void *)&data,ptr,sizeof(WIPEDATA));
        return (TRUE);
     } else
        return (FALSE);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Wipe::SaveConfigure()

BOOL ImageFilter_Wipe::SaveConfigure ( void *ptr ) {
     if (ptr) {
        memcpy(ptr,(void *)&data,sizeof(WIPEDATA));
        return (TRUE);
     } else
        return (FALSE);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Wipe::EvaluateConfigure()

DWORD ImageFilter_Wipe::EvaluateConfigure ( ) {
      return (sizeof(WIPEDATA));
}

//-- EOF: Wipe.cpp ------------------------------------------------------------
