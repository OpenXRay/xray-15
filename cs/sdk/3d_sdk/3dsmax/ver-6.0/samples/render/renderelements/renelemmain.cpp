/**********************************************************************
 *<
	FILE: renElemMain.cpp

	DESCRIPTION:   	DLL main for render elements

	CREATED BY: 	Kells Elmquist

	HISTORY: 	created 15 apr 2000

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "renElemPch.h"
#include "stdRenElems.h"
#include "stdBakeElem.h"

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

__declspec( dllexport ) const TCHAR * LibDescription() { return GetString(IDS_KE_RENDER_ELEMENT_DESC); }

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses() 
{
	return 22;
}

__declspec( dllexport ) ClassDesc* LibClassDesc(int i) 
{
	switch(i) {
		// Both versions
		case 0: return GetBeautyElementDesc();
		case 1: return GetSpecularElementDesc();
		case 2: return GetDiffuseElementDesc();
		case 3: return GetEmissionElementDesc();
		case 4: return GetReflectionElementDesc();
		case 5: return GetRefractionElementDesc();
		case 6: return GetShadowElementDesc();
		case 7: return GetAtmosphereElementDesc();
		case 8: return GetBlendElementDesc();
		case 9: return GetZElementDesc();
		case 10: return GetAlphaElementDesc();
		case 11: return GetBgndElementDesc();
		case 12: return GetLightingElementDesc();
		case 13: return GetMatteElementDesc();

		case 14: return GetDiffuseBakeElementDesc();
		case 15: return GetSpecularBakeElementDesc();
		case 16: return GetCompleteBakeElementDesc();
//		case 15: return GetReflectRefractBakeElementDesc();
		case 17: return GetAlphaBakeElementDesc();
		case 18: return GetLightBakeElementDesc();
		case 19: return GetShadowBakeElementDesc();
		case 20: return GetNormalsBakeElementDesc();
		case 21: return GetBlendBakeElementDesc();

#ifndef DESIGN_VER	// Not Design version

#else  // DESIGN_VER

#endif // DESIGN_VER

		default: return NULL;
	}// end switch
}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG LibVersion() {
	return VERSION_3DSMAX;
}

TCHAR *GetString(int id)
{
	static TCHAR buf[256];
	if(hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;

	return NULL;
}
