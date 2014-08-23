/**********************************************************************
 *<
	FILE: 		shadersMain.cpp

	DESCRIPTION:   	DLL main for shaders

	CREATED BY: 	Kells Elmquist

	HISTORY: 	created 2/6/1999

 *>	Copyright (c) 1999, All Rights Reserved.
 **********************************************************************/

#include "shadersPch.h"
#include "shadersRc.h"
#include "shadersMain.h"

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
// This is the interface to Max:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR * LibDescription() { return GetString(IDS_KE_SHADERS_DESC); }

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses() 
{
#ifndef DESIGN_VER
	return 5;
#else
	return 5;
#endif // !DESIGN_VER
}

__declspec( dllexport ) ClassDesc* LibClassDesc(int i) 
{
	switch(i) {
		// Both versions
//		case 0: return GetPhongShaderCD();
//		case 1: return GetMetalShaderCD();
//		case 2: return GetBlinnShaderCD();
		case 0: return GetOrenNayarBlinnShaderCD();
		case 1: return GetAnisoShaderCD();
		case 2: return GetMultiLayerShaderCD();
		case 3: return GetStraussShaderCD();
		case 4: return GetTranslucentShaderCD();
//		case 7: return GetOldBlinnShaderCD();

//		case 8: return GetWardShaderCD();
//		case 9: return GetConstantShaderCD();

#ifndef DESIGN_VER	// Not Design version

#else  // DESIGN_VER

#endif // DESIGN_VER

		default: return 0;
	}
}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG LibVersion() { return VERSION_3DSMAX; }

TCHAR *GetString(int id)
{
	static TCHAR buf[256];
	if(hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;

	return NULL;
}
