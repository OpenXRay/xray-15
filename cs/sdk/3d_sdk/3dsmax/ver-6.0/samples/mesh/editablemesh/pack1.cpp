/**********************************************************************
 *<
	FILE: pack1.cpp

	DESCRIPTION:   Some extra goodies

	CREATED BY: Rolf Berteig

	HISTORY: created 4 March 1996

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#include "pack1.h"

HINSTANCE hInstance;
static int controlsInit = FALSE;
int enabled = FALSE;

/** public functions **/
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
	hInstance = hinstDLL;

	if (!controlsInit) {
		controlsInit = TRUE;
		
		// See if the editable mesh is enabled
		enabled = GetSystemSetting(SYSSET_ENABLE_EDITABLEMESH);

		// MAXontrols
		InitCustomControls(hInstance);
		
		// initialize Chicago controls
		InitCommonControls();

		// Register us as the editable tri object.
		if (enabled) {
			RegisterEditTriObjDesc(GetEditTriObjectDesc());
			RegisterStaticEditTri (statob.GetEditTriOb ());
			}
		}	

	return(TRUE);
	}


//------------------------------------------------------
// This is the interface to MAX
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_LIB_DESCRIPTION); }

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses() {return 1;}


__declspec( dllexport ) ClassDesc*
LibClassDesc(int i) {
	switch(i) {
		case 0: return GetEditTriObjectDesc();
		default: return 0;
		}

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

Object *StaticObject::GetEditTriOb () {
	if (!ob) ob = (Object *) GetEditTriObjDesc()->Create ();
	return ob;
}

static StaticObject statob;
