/**********************************************************************
 *<
	FILE: Manip.cpp

	DESCRIPTION:   DLL implementation of standard Manipulators

	CREATED BY: Scott Morrison

	HISTORY: created 5/18/00

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "Manip.h"

HINSTANCE hInstance;
int controlsInit = FALSE;

// russom - 10/16/01
// The classDesc array was created because it was way to difficult
// to maintain the switch statement return the Class Descriptors
// with so many different #define used in this module.

#define MAX_MANIP_OBJECTS 11
static ClassDesc *classDescArray[MAX_MANIP_OBJECTS];
static int classDescCount = 0;

void initClassDescArray(void)
{
#ifndef NO_MANIP_CONE	// russom - 10/16/01
	classDescArray[classDescCount++] = GetConeAngleDesc();
#endif
	classDescArray[classDescCount++] = GetHotsotManipDesc();
	classDescArray[classDescCount++] = GetFalloffManipDesc();
#ifndef NO_MANIP_PLANEANGLE	// russom - 10/16/01
	classDescArray[classDescCount++] = GetPlaneAngleDesc();
#endif
	classDescArray[classDescCount++] = GetIKSwivelManipDesc();
	classDescArray[classDescCount++] = GetReactorAngleManipDesc();
	classDescArray[classDescCount++] = GetSliderManipDesc();
	classDescArray[classDescCount++] = GetPositionManipDesc();
	classDescArray[classDescCount++] = GetReactorPosValueManipDesc();
	classDescArray[classDescCount++] = GetIKStartSpTwistManipDesc();
	classDescArray[classDescCount++] = GetIKEndSpTwistManipDesc();

}

/** public functions **/
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
	hInstance = hinstDLL;

	if ( !controlsInit ) {
		controlsInit = TRUE;
		
		initClassDescArray();

		// jaguar controls
		InitCustomControls(hInstance);

		// initialize Chicago controls
		InitCommonControls();
	}

	return(TRUE);
}


__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_LIBDESCRIPTION); }

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses() {return classDescCount;}


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

TCHAR *GetString(int id)
	{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
	}
