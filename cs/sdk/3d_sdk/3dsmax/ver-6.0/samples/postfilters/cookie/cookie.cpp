//-----------------------------------------------------------------------------
// ---------------------
// File	....:	Cookie.cpp
// ---------------------
// Author...:	Gus J	Grubba
// Date	....:	February 1996
// Descr....:	Cookie Cutter Image Filter
//
// History	.:	Feb, 18 1996 -	Started
//				
//-----------------------------------------------------------------------------
		
//--	Include files

#include <Max.h>
#include <bmmlib.h>
#include <fltlib.h>
#include "Cookie.h"
#include "resource.h"

//--	Globals ------------------------------------------------------------------

HINSTANCE hInst = NULL;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//--	DLL Declaration

BOOL	WINAPI DllMain(HINSTANCE hDLLInst, DWORD fdwReason, LPVOID lpvReserved)	{
	switch (fdwReason) {
		 case	DLL_PROCESS_ATTACH:
				if	(hInst)
					return(FALSE);
				hInst	= hDLLInst;
				break;
		 case	DLL_PROCESS_DETACH:
				hInst	 =	NULL;
				break;
		 case	DLL_THREAD_ATTACH:
				break;
		 case	DLL_THREAD_DETACH:
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
// Cookie	Class	Description

class COOKIEClassDesc:public ClassDesc {
	
	public:

		int				 IsPublic	  ( )							{ return	1;						}
		void				*Create		  ( BOOL	loading=FALSE)	{ return	new ImageFilter_Cookie; }
		const	TCHAR		*ClassName	  ( )							{ return	GetString(IDS_DB_COOKIE);		 }
		SClass_ID		 SuperClassID ( )							{ return	FLT_CLASS_ID;	}
		Class_ID			 ClassID		  ( )							{ return	Class_ID(COOKIECLASSID,0);	 }
		const	TCHAR		*Category	  ( )							{ return	GetString(IDS_DB_IMAGE_FILTER); }

};

static COOKIEClassDesc COOKIEDesc;

//-----------------------------------------------------------------------------
// Interface

DLLEXPORT const TCHAR * LibDescription (	)	{ 
 	return GetString(IDS_LIBDESCRIPTION); 
}

DLLEXPORT int	LibNumberClasses ( )	{ 
	return 1; 
}

DLLEXPORT ClassDesc	*LibClassDesc(int	i)	{
	switch(i) {
		case	0:	return &COOKIEDesc;	break;
		default:	return 0;		  break;
	}
}

DLLEXPORT ULONG LibVersion (	)	{ 
	return (	VERSION_3DSMAX	);	
}

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}

const TCHAR *ImageFilter_Cookie::Description( ) 
{
	return GetString(IDS_DB_IMAGE_ALPHA);
}

//-----------------------------------------------------------------------------
// *> AboutCtrlDlgProc()
//

BOOL	CALLBACK	AboutCtrlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)	{
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
// #> ImageFilter_Cookie::ShowAbout()

void	ImageFilter_Cookie::ShowAbout(HWND hWnd) {
	DialogBoxParam(
		hInst,
		MAKEINTRESOURCE(IDD_COOKIE_ABOUT),
		hWnd,
		(DLGPROC)AboutCtrlDlgProc,
		(LPARAM)this);
}

//-----------------------------------------------------------------------------
// #> ImageFilter_Cookie::Render()
//

BOOL	ImageFilter_Cookie::Render(HWND	hWnd)	{

	BMM_Color_64 *l64,*line64 = NULL;
	WORD			 *m16,*mask16 = NULL;
	BOOL result	= FALSE;
	BOOL abort	= FALSE;
	int iy,ix;

	if	(!srcmap)
		return (FALSE);

	if	(!mskmap)
		return (FALSE);

	//-- Buffers -----------------------------------------

	line64 = (BMM_Color_64 *)calloc(srcmap->Width(),sizeof(BMM_Color_64));

	if	(!line64)
		goto done;

	mask16 =	(WORD	*)calloc(srcmap->Width(),sizeof(WORD));

	if	(!mask16)
		goto done;

	//-- Render Loop -------------------------------------

	for ( iy	= 0; iy < srcmap->Height(); iy++) {

		//-- Progress Report
		
		SendMessage(hWnd,FLT_PROGRESS,iy,srcmap->Height()-1);

		//-- Check for	Abort
		
		SendMessage(hWnd,FLT_CHECKABORT,0,(LPARAM)(BOOL	*)&abort);

		if	(abort)
			goto done;
		
		//-- Get	lines
		
		if	(srcmap->GetLinearPixels(0,iy,srcmap->Width(),line64)!=1)
			goto done;

		if	(mskmap->Get16Gray(0,iy,srcmap->Width(),mask16)!=1)
			goto done;
			
		//-- Apply Mask -----------------------------------
		
		l64 =	line64;
	  	m16 =	mask16;

		for ( ix	= 0; ix < srcmap->Width();	ix++,l64++,m16++)	{
			l64->a =	*m16;
		}

		//-- Output	Line
		
		if	(srcmap->PutPixels(0,iy,srcmap->Width(),line64)!=1)
			goto done;

	}

	result = TRUE;
	
	//-- Kaput -------------------------------------------

	done:

	if	(mask16)
		free(mask16);
		
	if	(line64)
		free(line64);
		
	return(result);

}

//--	EOF: Cookie.cpp --------------------------------------------------------


































