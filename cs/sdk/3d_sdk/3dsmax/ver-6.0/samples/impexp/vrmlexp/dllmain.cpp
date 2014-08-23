/**********************************************************************
 *<
	FILE: dllmain.cpp

	DESCRIPTION:   DLL implementation of primitives

	CREATED BY: Charles Thaeler

        BASED on helpers.cpp

	HISTORY: created 12 February 1996

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "vrml.h"

//extern ClassDesc* GetMrBlueDesc();
extern ClassDesc* GetLODDesc();
extern ClassDesc* GetVRBLDesc();
extern ClassDesc* GetVRMLInsertDesc();
extern ClassDesc* GetVRMLMtlDesc();
extern ClassDesc *GetOmniLightDesc();
extern ClassDesc *GetTSpotLightDesc();
extern ClassDesc *GetDirLightDesc();
extern ClassDesc *GetFSpotLightDesc();
extern ClassDesc* GetPolyCounterDesc();
extern ClassDesc* GetTimeSensorDesc();
extern ClassDesc* GetNavInfoDesc();
extern ClassDesc* GetBackgroundDesc();
#ifndef NO_HELPER_FOG
extern ClassDesc* GetFogDesc();
#endif // NO_HELPER_FOG
extern ClassDesc* GetAudioClipDesc();
extern ClassDesc* GetSoundDesc();
extern ClassDesc* GetTouchSensorDesc();
extern ClassDesc* GetProxSensorDesc();
extern ClassDesc* GetAnchorDesc();
extern ClassDesc* GetBillboardDesc();
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

#define MAX_PRIM_OBJECTS 14

#ifdef _DEBUG
#define NUM_CLASSES (MAX_PRIM_OBJECTS + 1)
#else
#define NUM_CLASSES MAX_PRIM_OBJECTS
#endif

ClassDesc *classDescArray[NUM_CLASSES];
int classDescCount = 0;

void initClassDescArray()
{
	classDescArray[classDescCount++] = GetAnchorDesc();
	classDescArray[classDescCount++] = GetTouchSensorDesc();
	classDescArray[classDescCount++] = GetProxSensorDesc();
	classDescArray[classDescCount++] = GetTimeSensorDesc();
	classDescArray[classDescCount++] = GetNavInfoDesc();
	classDescArray[classDescCount++] = GetBackgroundDesc();
#ifndef NO_HELPER_FOG
	classDescArray[classDescCount++] = GetFogDesc();
#endif // NO_HELPER_FOG
	classDescArray[classDescCount++] = GetAudioClipDesc();
	classDescArray[classDescCount++] = GetSoundDesc();
	classDescArray[classDescCount++] = GetBillboardDesc();
	classDescArray[classDescCount++] = GetLODDesc();
	classDescArray[classDescCount++] = GetVRBLDesc();
	classDescArray[classDescCount++] = GetVRMLInsertDesc();
#ifndef NO_UTILITY_POLYGONCOUNTER	// russom - 12/04/01
	classDescArray[classDescCount++] = GetPolyCounterDesc();
#endif
#ifdef _DEBUG
	classDescArray[classDescCount++] = GetCppOutDesc();
#endif
}

/** public functions **/
BOOL WINAPI
DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
	hInstance = hinstDLL;

	if ( !controlsInit ) {
		controlsInit = TRUE;
		
		initClassDescArray();

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

__declspec( dllexport ) int LibNumberClasses()
{
	return classDescCount;
}

__declspec( dllexport ) ClassDesc*
LibClassDesc(int i)
{
   	if( i < classDescCount )
		return classDescArray[i];
	else
		return 0;
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