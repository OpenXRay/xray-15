	/**********************************************************************
	 *<
		FILE: ffdmod.cpp

		DESCRIPTION: DllMain is in here

		CREATED BY: Rolf Berteig

		HISTORY: created 7/22/96

	 *>	Copyright (c) 1996 Rolf Berteig, All Rights Reserved.
	 **********************************************************************/

////////////////////////////////////////////////////////////////////
//
// Free Form Deformation Patent #4,821,214 licensed 
// from Viewpoint DataLabs Int'l, Inc., Orem, UT
// www.viewpoint.com
// 
////////////////////////////////////////////////////////////////////

#include "ffdmod.h"

HINSTANCE hInstance;
static int controlsInit = FALSE;

// russom - 10/12/01
// The classDesc array was created because it was way to difficult
// to maintain the switch statement return the Class Descriptors
// with so many different #define used in this module.

#define MAX_FFD_OBJECTS 9
static ClassDesc *classDescArray[MAX_FFD_OBJECTS];
static int classDescCount = 0;

void initClassDescArray(void)
{
#ifndef NO_MODIFIER_FFD_4X4	// russom - 10/12/01
	classDescArray[classDescCount++] = GetFFDDesc44();
#endif
#ifndef NO_MODIFIER_FFD_3X3
	classDescArray[classDescCount++] = GetFFDDesc33();
#endif
#ifndef NO_MODIFIER_FFD_2X2
	classDescArray[classDescCount++] = GetFFDDesc22();
#endif
	classDescArray[classDescCount++] = GetFFDNMSquareOSDesc();
#ifndef NO_SPACEWARPS
	classDescArray[classDescCount++] = GetFFDNMSquareWSDesc();
	classDescArray[classDescCount++] = GetFFDNMSquareWSModDesc();
#endif
	classDescArray[classDescCount++] = GetFFDNMCylOSDesc();
#ifndef NO_SPACEWARPS
	classDescArray[classDescCount++] = GetFFDNMCylWSDesc();
	classDescArray[classDescCount++] = GetFFDSelModDesc();
#endif
}

/** public functions **/
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
	hInstance = hinstDLL;

	if (!controlsInit) {
		controlsInit = TRUE;		
		
		initClassDescArray();

		// MAXontrols
		InitCustomControls(hInstance);
		
		// initialize Chicago controls
		InitCommonControls();		
		}	

	return(TRUE);
	}


//------------------------------------------------------
// This is the interface to MAX
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_RB_FFDMOD); }


// This function returns the number of plug-in classes this DLL implements
__declspec( dllexport ) int 
LibNumberClasses() { return classDescCount; }


// This function return the ith class descriptor. We have one.
__declspec( dllexport ) ClassDesc* 
LibClassDesc(int i) {

	if( i < classDescCount )
		return classDescArray[i];
	else
		return NULL;
 }


// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG 
LibVersion() { return VERSION_3DSMAX; }


// Loads a string from the resource into a static buffer.
TCHAR *GetString(int id)
	{
	static TCHAR buf[256];
	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
	}

INT_PTR CALLBACK DefaultSOTProc(
		HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	IObjParam *ip = (IObjParam*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch (msg) {
		case WM_INITDIALOG:
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			if (ip) ip->RollupMouseMessage(hWnd,msg,wParam,lParam);
			return FALSE;

		default:
			return FALSE;
		}
	return TRUE;
	}
