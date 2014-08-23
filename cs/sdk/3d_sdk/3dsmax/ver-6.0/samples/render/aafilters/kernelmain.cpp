/**********************************************************************
 *<
	FILE: kernelmain.cpp

	DESCRIPTION:   DLL implementation prefilter kernels

	CREATED BY: 	Kells Elmquist

	HISTORY: 	created 29 june 1998

 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/

#include "kernelhdr.h"
#include "kernelres.h"

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

		// register SXP readers
		// RegisterSXPReader(_T("MARBLE_I.SXP"), Class_ID(MARBLE_CLASS_ID,0));
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

__declspec( dllexport ) const TCHAR * LibDescription() { return GetString(IDS_KE_KERNELDESC); }

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses() 
{
#ifdef INCLUDE_PIXELSIZE
	return 12;
#else
	return 11;
#endif 
}

__declspec( dllexport ) ClassDesc* LibClassDesc(int i) 
{
	switch(i) {
		case 0: return GetAreaKernelDesc();
		case 1: return GetMax1KernelDesc();
		case 2: return GetQuadraticKernelDesc();
		case 3: return GetCubicKernelDesc();
		case 4: return GetNTSCKernelDesc();
		case 5: return GetGaussVarKernelDesc();
		case 6: return GetCookVarKernelDesc();
		case 7: return GetBlendKernelDesc();
		case 8: return GetBlackman474KernelDesc();
		case 9: return GetMitNetVarKernelDesc();
		case 10: return GetCatRomKernelDesc();
#ifdef INCLUDE_PIXELSIZE
		case 11: return GetPixelSizeKernelDesc();
#endif
//		case 6: return GetCylinderVarKernelDesc();

//		case 10: return GetStockingKernelDesc();
//		case 11: return GetHanningKernelDesc();
//		case 12: return GetSampKernelDesc();
//		case 13: return GetConeKernelDesc();
//		case 14: return GetCubicKernelDesc();
//		case 15: return GetGaussNarrowKernelDesc();
//		case 16: return GetGaussMediumKernelDesc();
//		case 17: return GetGaussWideKernelDesc();
//		case 18: return GetPavicUnitVolKernelDesc();
//		case 19: return GetPavicOpKernelDesc();
//		case 20: return GetHammingKernelDesc();
//		case 21: return GetBlackmanKernelDesc();
//		case 22: return GetBlackman361KernelDesc();
//		case 23: return GetBlackman367KernelDesc();
//		case 24: return GetBlackman492KernelDesc();
//		case 25: return GetMax2KernelDesc();
//		case 26: return GetMax3KernelDesc();
//		case 27: return GetMitNetNotchKernelDesc();
//		case 28: return GetCookOpKernelDesc();
//		case 29: return GetCylinderVarKernelDesc();
//		case 30: return GetQuadraticVarKernelDesc();
//		case 31: return GetCubicVarKernelDesc();


#ifndef DESIGN_VER

#else

#endif // !DESIGN_VER

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
