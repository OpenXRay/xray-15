//-----------------------------------------------------------------------------
// -----------------------
// File ....: Contrast.cpp
// -----------------------
// Author...: Gus J Grubba
// Date ....: September 1995
// Descr....: Contrast Image Filter
//
// History .: Sep, 07 1995 - Started
//            
//-----------------------------------------------------------------------------
		 
//-- Include files

#include <Max.h>
#include <tvnode.h>
#include <bmmlib.h>
#include <fltlib.h>
#include "Contrast.h"
#include "resource.h"

//-- Globals ------------------------------------------------------------------

HINSTANCE hInst = NULL;
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
//-- Helper

TCHAR *GetString(int id) {
	static TCHAR buf[256];
	if (hInst)
		return LoadString(hInst, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Contrast Class Description

class CONClassDesc:public ClassDesc {
	  
	  public:

		 int             IsPublic     ( )                   { return 1;                }
		 void           *Create       ( BOOL loading=FALSE) { return new ImageFilter_Contrast; }
		 const TCHAR    *ClassName    ( )                   { return GetString(IDS_DB_CONTRAST);     }
		 SClass_ID       SuperClassID ( )                   { return FLT_CLASS_ID;  }
		 Class_ID        ClassID      ( )                   { return Class_ID(CONTRASTCLASSID,0);    }
		 const TCHAR    *Category     ( )                   { return GetString(IDS_DB_IMAGE_FILTER); }

};

static CONClassDesc CONDesc;

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
		 case  0: return &CONDesc; break;
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

const TCHAR *ImageFilter_Contrast::Description( ) 
{
	return GetString(IDS_DB_CONTRAST);
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
// *> ControlCtrlDlgProc()
//

INT_PTR CALLBACK ControlCtrlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
	static ImageFilter_Contrast *f = NULL;
	if (message == WM_INITDIALOG) 
		f = (ImageFilter_Contrast *)lParam;
	if (f) 
		return (f->Control(hWnd,message,wParam,lParam));
	else
		return(FALSE);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Contrast::ImageFilter_Contrast()

ImageFilter_Contrast::ImageFilter_Contrast() {

	lut				= NULL;
	data.absolute	= FALSE;
	data.contrast	= 0.0f;
	data.brightness	= 0.0f;

}

//-----------------------------------------------------------------------------
// #> ImageFilter_Contrast::ShowAbout()

void ImageFilter_Contrast::ShowAbout(HWND hWnd) {
	  DialogBoxParam(
		 hInst,
		 MAKEINTRESOURCE(IDD_CON_ABOUT),
		 hWnd,
		 (DLGPROC)AboutCtrlDlgProc,
		 (LPARAM)this);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Contrast::ShowControl()

BOOL ImageFilter_Contrast::ShowControl(HWND hWnd) {
	return (DialogBoxParam(
		hInst,
		MAKEINTRESOURCE(IDD_CONTROL),
		hWnd,
		(DLGPROC)ControlCtrlDlgProc,
		(LPARAM)this));
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Contrast::Control()

BOOL ImageFilter_Contrast::Control(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {

	switch (message) {
		
		//------------------------------------------------------------
		//-- Init ----------------------------------------------------

		case WM_INITDIALOG:

			CenterWindow(hWnd,GetParent(hWnd));
			CheckRadioButton(hWnd,IDC_ABSOLUTE,IDC_DERIVED,data.absolute?IDC_ABSOLUTE:IDC_DERIVED);

			//-- Contrast Spinner

			conspin = GetISpinner(GetDlgItem(hWnd, IDC_CONTRAST_SP));
			conspin->LinkToEdit(  GetDlgItem(hWnd, IDC_CONTRAST_ED), EDITTYPE_FLOAT );
			conspin->SetLimits( -1.0f,1.0f, FALSE);
			conspin->SetValue(data.contrast,FALSE);
			conspin->SetScale((float)0.01);

			//-- Brightness Spinner

			brispin = GetISpinner(GetDlgItem(hWnd, IDC_BRIGHT_SP));
			brispin->LinkToEdit(  GetDlgItem(hWnd, IDC_BRIGHT_ED), EDITTYPE_FLOAT );
			brispin->SetLimits( -1.0f,1.0f, FALSE);
			brispin->SetValue(data.brightness,FALSE);
			brispin->SetScale((float)0.01);

			return 1;

		//------------------------------------------------------------
		//-- Buttons -------------------------------------------------
				  
		case WM_COMMAND:

			switch (LOWORD(wParam)) {
				  
				case IDOK:              
					data.absolute	= IsDlgButtonChecked(hWnd,IDC_ABSOLUTE);
					data.contrast	= conspin->GetFVal();
					data.brightness	= brispin->GetFVal();
					EndDialog(hWnd,1);
					break;

				case IDCANCEL:
				   	EndDialog(hWnd,0);
				   	break;
		
			}
			return 1;

		//------------------------------------------------------------
		//-- Kaput ---------------------------------------------------
				  
		case WM_DESTROY:
			ReleaseISpinner(conspin);
			ReleaseISpinner(brispin);
			break;

	}
	
	return 0;

}

//-----------------------------------------------------------------------------
// #> ImageFilter_Contrast::Render()
//

BOOL ImageFilter_Contrast::Render(HWND hWnd) {

	BMM_Color_64 *l64,*line64 = NULL;
	
	if (!srcmap)
		return (FALSE);

	if (!BuildTable())
		return (FALSE);

	if (data.contrast == 0.0f && data.brightness == 0.0f)
		return(TRUE);

	if ((line64=(BMM_Color_64 *)calloc(srcmap->Width(),sizeof(BMM_Color_64)))==NULL)
		return (FALSE);

	BOOL result = TRUE;
	BOOL abort  = FALSE;

	int	bright = (int)(data.brightness * 65535.0f);
	  
	//-- Mess up the image

	for (int iy = 0; iy < srcmap->Height(); iy++) {

		//-- Progress Report
		 
		SendMessage(hWnd,FLT_PROGRESS,iy,srcmap->Height()-1);

		//-- Check for Abort
		 
		SendMessage(hWnd,FLT_CHECKABORT,0,(LPARAM)(BOOL *)&abort);

		if (abort) {
			result = FALSE;
			break;
		}   
		 
		//-- Get line
		 
		l64 = line64;

		if (srcmap->GetLinearPixels(0,iy,srcmap->Width(),line64)!=1) {
			result = FALSE;
			break;
		}

		for (int ix = 0; ix < srcmap->Width(); ix++,l64++) {

			int gray;

			int r = l64->r;
			int g = l64->g;
			int b = l64->b;

			if (data.absolute) {
			    gray = max(r,g);
			    gray = max(b,gray);
			} else 
				gray = (r+g+b) / 3;

			int k = lut[gray]-gray;

			l64->r = max( min( r + k + bright, 65535),0);
			l64->g = max( min( g + k + bright, 65535),0);
			l64->b = max( min( b + k + bright, 65535),0);

		}

		//-- Output Line
		 
		if (srcmap->PutPixels(0,iy,srcmap->Width(),line64)!=1) {
			result = FALSE;
			break;
		}

	}
	  
	if (line64)
		free(line64);
		 
	return(result);

}

//-----------------------------------------------------------------------------
// #> ImageFilter_Contrast::BuildTable()
//

BOOL ImageFilter_Contrast::BuildTable ( void ) {
	
	lut = (WORD *)LocalAlloc(LPTR,sizeof(WORD)*65536);

	if (!lut)
		return (FALSE);
	
	int i;

	for (i=0; i < 65536; i++) lut[i]=i;

	if ( data.contrast > 0.0f ) {

		int MinBin	= (int)(data.contrast * 65535.0f);
		int MaxBin	= 65535 - MinBin;
		float step	= (float)sqrt((double)data.contrast)/data.contrast;
		float stepv	= 0.0f;

		for (i=0; i<MinBin; i++) {
			lut[i] = (WORD)stepv;
			stepv += step;
		}

		step = 65536.0f / (float)(MaxBin-MinBin);

		for (i=MinBin; i<=MaxBin; i++) {
			if (stepv > 65535.0f) {
				stepv = 65535.0f;
				step = 0.0f;
			}
			lut[i] = (WORD)stepv;
			stepv += step;
		}

		for (i=MaxBin+1; i < 65536; i++) lut[i] = 65535;

	} else  {

		if (data.contrast < 0.0f ) {
			float step	= (65536.0f + data.contrast * 2.0f * 65535.0f ) / 65536.0f;
			float stepv	= data.contrast * -1.0f * 65535.0f;
			for (i = 0; i < 65536; i++) {
				lut[i] = (WORD)stepv;
				stepv += step;
			}
		}
	}

	return (TRUE);

}

//-----------------------------------------------------------------------------
// #> ImageFilter_Contrast::LoadConfigure()

BOOL ImageFilter_Contrast::LoadConfigure ( void *ptr ) {
	CONTRASTDATA *buf = (CONTRASTDATA *)ptr;
	if (buf->version == CONTRASTVERSION) {
		memcpy((void *)&data,ptr,sizeof(CONTRASTDATA));
		return (TRUE);
	} else
		return (FALSE);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Contrast::SaveConfigure()

BOOL ImageFilter_Contrast::SaveConfigure ( void *ptr ) {
	if (ptr) {
		data.version = CONTRASTVERSION;
		memcpy(ptr,(void *)&data,sizeof(CONTRASTDATA));
		return (TRUE);
	} else
		return (FALSE);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Contrast::EvaluateConfigure()

DWORD ImageFilter_Contrast::EvaluateConfigure ( ) {
      return (sizeof(CONTRASTDATA));
}

//-- EOF: contrast.cpp --------------------------------------------------------


































