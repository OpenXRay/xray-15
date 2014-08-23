/**********************************************************************
 *<
	FILE: EPoly.cpp

	DESCRIPTION: Editable PolyMesh

	CREATED BY: Steve Anderson

	HISTORY: created 4 March 1996

 *>	Copyright (c) Discreet 1999, All Rights Reserved.
 **********************************************************************/

#include "EPoly.h"

HINSTANCE hInstance;
static int controlsInit = FALSE;
int enabled = FALSE;

#define MAX_MOD_OBJECTS	51
ClassDesc *classDescArray[MAX_MOD_OBJECTS];
int classDescCount = 0;

void initClassDescArray(void)
{
#ifndef NO_MODIFIER_POLY_SELECT // JP Morel - July 23rd 2002
    classDescArray[classDescCount++] = GetPolySelectDesc  ();
#endif
	classDescArray[classDescCount++] = GetEditablePolyDesc();
#ifndef NO_MODIFIER_EDIT_NORMAL
	classDescArray[classDescCount++] = GetEditNormalsDesc ();
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
		// Register us as the editable poly object?
		RegisterEditPolyObjDesc (GetEditablePolyDesc());
	}

	return(TRUE);
}


//------------------------------------------------------
// This is the interface to MAX
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_LIB_DESCRIPTION); }

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses() {return classDescCount;}


__declspec( dllexport ) ClassDesc*LibClassDesc(int i) {
	if( i < classDescCount )
		return classDescArray[i];
	else
		return NULL;
}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG LibVersion() { return VERSION_3DSMAX; }

TCHAR *GetString(int id) {
	static TCHAR buf[256];
	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}
