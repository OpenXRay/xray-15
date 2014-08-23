/**********************************************************************
 *<
	FILE: Convert.cpp

	DESCRIPTION:   Conversion modifiers

	CREATED BY: Steve Anderson

	HISTORY: created February 2000

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/
#include "buildver.h"
#include "Convert.h"

HINSTANCE hInstance;
int controlsInit = FALSE;

/** public functions **/
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
	hInstance = hinstDLL;
	if ( !controlsInit ) {
		controlsInit = TRUE;
		// jaguar controls
		InitCustomControls(hInstance);
		// initialize Chicago controls
		InitCommonControls();
	}
	return(TRUE);
}


//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_LIBDESCRIPTION); }

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses() {
#ifndef NO_MODIFIER_CONVERTTOPATCH
	return 3;
#else
	return 2;
#endif // NO_MODIFIER_CONVERTTOPATCH
}


__declspec( dllexport ) ClassDesc*
LibClassDesc(int i) {
	switch(i) {
	case 0: return GetConvertToMeshDesc();
	case 1: return GetConvertToPolyDesc();

#ifndef NO_MODIFIER_CONVERTTOPATCH
	case 2: return GetConvertToPatchDesc();
#endif // NO_MODIFIER_CONVERTTOPATCH

	default: return 0;
	}
}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG 
LibVersion() { return VERSION_3DSMAX; }

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}

TCHAR *GetString(int id) {
	static TCHAR buf[256];
	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}
