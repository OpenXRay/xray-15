/**********************************************************************
 *<
	FILE: util.cpp

	DESCRIPTION:   Sample utilities

	CREATED BY: Rolf Berteig

	HISTORY: created 23 December 1995

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "util.h"

HINSTANCE hInstance;
int controlsInit = FALSE;

// russom - 10/16/01
// The classDesc array was created because it was way to difficult
// to maintain the switch statement return the Class Descriptors
// with so many different #define used in this module.

#define MAX_UTIL_OBJECTS 14
static ClassDesc *classDescArray[MAX_UTIL_OBJECTS];
static int classDescCount = 0;
static int classDescCountOrig = 0;	// this is unique to the util.cpp implementation

void initClassDescArray(void)
{
#ifndef NO_UTILITY_COLORCLIPBOARD	// russom - 12/04/01
	classDescArray[classDescCount++] = GetColorClipDesc();
#endif
#ifndef NO_UTILITY_ASCIIOUTPUT	// russom - 12/04/01
 #ifndef DESIGN_VER
	classDescArray[classDescCount++] = GetAsciiOutDesc();
 #endif
#endif
#ifndef NO_UTILITY_COLLAPSE	// russom - 10/16/01
	classDescArray[classDescCount++] = GetCollapseUtilDesc();
#endif
	classDescArray[classDescCount++] = GetRandKeysDesc();
	classDescArray[classDescCount++] = GetORTKeysDesc();
	classDescArray[classDescCount++] = GetSelKeysDesc();
#ifndef NO_UTILITY_LINKINFO	// russom - 10/16/01
	classDescArray[classDescCount++] = GetLinkInfoUtilDesc();
#endif
	classDescArray[classDescCount++] = GetCellTexDesc();
#ifndef NO_UTILITY_RESCALE	// russom - 10/16/01
	classDescArray[classDescCount++] = GetRescaleDesc();
#endif
#ifndef NO_UTILITY_SHAPECHECK	// russom - 12/04/01
	classDescArray[classDescCount++] = GetShapeCheckDesc();
#endif
#ifdef _DEBUG
	classDescArray[classDescCount++] = GetUtilTestDesc();
	classDescArray[classDescCount++] = GetAppDataTestDesc();
	classDescArray[classDescCount++] = GetTestSoundObjDescriptor();
#endif

	classDescCountOrig = classDescCount;
}

/** public functions **/
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
	hInstance = hinstDLL;

	if (!controlsInit) {
		controlsInit = TRUE;
		
		initClassDescArray();

		// jaguar controls
		InitCustomControls(hInstance);
		
		// initialize Chicago controls
		InitCommonControls();
		}
			
	return (TRUE);
	}


//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_RB_DEFUTIL); }

__declspec( dllexport ) int LibNumberClasses() {

	classDescCount = classDescCountOrig;

	// RB 11/17/2000: Only provide set-key mode plug-in if the feature is enabled.
	if (IsSetKeyModeFeatureEnabled())
		classDescArray[classDescCount++] = GetSetKeyUtilDesc();

	return classDescCount;
	}


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




// russom - 10/16/01
// This is the original LibNumberClasses and LibClassDesc - since it is a
// little unique, I kept it around.

/*
__declspec( dllexport ) int LibNumberClasses() {
#if !defined(DESIGN_VER)
	int numClasses = 10; /// <- MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
#else
	int numClasses = 9; //aszabo|Sep.14.01 - removed ASCII Object Output
#endif // DESIGN_VER

	if (IsSetKeyModeFeatureEnabled()) numClasses++;	

#ifdef _DEBUG
	return numClasses + 3;
#else
	return numClasses;
#endif
	}


__declspec( dllexport ) ClassDesc*
LibClassDesc(int i) {
	switch(i) {
#if !defined(DESIGN_VER)
		case 0: return GetColorClipDesc();
		case 1: return GetAsciiOutDesc();
		case 2: return GetCollapseUtilDesc();
		case 3: return GetRandKeysDesc();
		case 4: return GetORTKeysDesc();
		case 5: return GetSelKeysDesc();
		case 6: return GetLinkInfoUtilDesc();
		case 7: return GetCellTexDesc();
		// case 8: return GetPipeMakerDesc(); //RK: 07/02/99 Removing this from Shiva
		case 8: return GetRescaleDesc();
		case 9: return GetShapeCheckDesc();					
#ifdef _DEBUG
		case 10: return GetUtilTestDesc();
		case 11: return GetAppDataTestDesc();
		case 12: return GetTestSoundObjDescriptor();
#endif

#else
		case 0: return GetColorClipDesc();
		//case 1: return GetAsciiOutDesc(); //aszabo|Sep.14.01 - removed ASCII Object Output
		case 1: return GetCollapseUtilDesc();
		case 2: return GetRandKeysDesc();
		case 3: return GetORTKeysDesc();
		case 4: return GetSelKeysDesc();
		case 5: return GetLinkInfoUtilDesc();
		case 6: return GetCellTexDesc();
		//case 8: return GetPipeMakerDesc();// //RK: 07/02/99 Removing this from Shiva
		case 7: return GetRescaleDesc();
		case 8: return GetShapeCheckDesc();					
#ifdef _DEBUG
		case 9: return GetUtilTestDesc();
		case 10: return GetAppDataTestDesc();
		case 11: return GetTestSoundObjDescriptor();
#endif

#endif // DESIGN_VER
		// RB 11/20/2000
		// Note that returning NULL when i<LibNumberClasses() is not supported!!!!
		// So to handle the set-key utility (which may or not be enable in either
		// the debug or release build) I've put it in the default case.
		// It should be case 10 in a release build and case 13 in a debug build.
		default:
			// RB 11/17/2000: Only provide set-key mode plug-in if the feature is enabled.
			if (IsSetKeyModeFeatureEnabled()) {
				return GetSetKeyUtilDesc();
				}
			return NULL; // Acts as if plug-in is not present.

		//default: return 0;
		}
			
	}
*/
