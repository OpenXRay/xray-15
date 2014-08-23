/**********************************************************************
 *<
	FILE: dllmain.cpp

	DESCRIPTION:   DLL implementation of primitives

	CREATED BY: Charles Thaeler

        BASED on helpers.cpp

	HISTORY: created 12 February 1996

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "max.h"
#include "resource.h"

extern ClassDesc* GetMrBlueDesc();
extern ClassDesc* GetLODDesc();
extern ClassDesc* GetVRBLDesc();
extern ClassDesc* GetVRMLInsertDesc();
//extern ClassDesc* GetVRMLMtlDesc();
//extern ClassDesc *GetOmniLightDesc();
//extern ClassDesc *GetTSpotLightDesc();
//extern ClassDesc *GetDirLightDesc();
//extern ClassDesc *GetFSpotLightDesc();
extern ClassDesc* GetPolyCounterDesc();
extern ClassDesc* GetCppOutDesc();

HINSTANCE hInstance;
int controlsInit = FALSE;

TCHAR
*GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}


/** public functions **/
BOOL WINAPI
DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
	hInstance = hinstDLL;

	if ( !controlsInit ) {
		controlsInit = TRUE;
		
		// jaguar controls
		InitCustomControls(hInstance);

		// initialize Chicago controls
		InitCommonControls();
		}

	switch(fdwReason) {
		case DLL_PROCESS_ATTACH:
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
		}
	return(TRUE);
}

//------------------------------------------------------
// This is the interface to MAX:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() {
	return GetString(IDS_DLL_INFO);
}

#define NUM_BASE_CLASSES 4
#ifdef _DEBUG
#define NUM_CLASSES (NUM_BASE_CLASSES+1)
#else
#define NUM_CLASSES NUM_BASE_CLASSES
#endif

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses()
{
	return NUM_CLASSES;
}

__declspec( dllexport ) ClassDesc*
LibClassDesc(int i)
{
    switch(i) {
    case 0: return GetLODDesc();
    case 1: return GetVRBLDesc();
    case 2: return GetVRMLInsertDesc();
    case 3: return GetMrBlueDesc();
#ifdef _DEBUG
    case NUM_BASE_CLASSES: return GetCppOutDesc();
#endif
    default: return 0;
    }
    
}

// Return version so can detect obsolete DLLs -- NOTE THIS IS THE API VERSION NUMBER
//                                               NOT THE VERSION OF THE DLL.
__declspec( dllexport ) ULONG 
LibVersion() { return VERSION_3DSMAX; }

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}
