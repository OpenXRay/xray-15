/**********************************************************************
 *<
	FILE: DllEntry.cpp

	DESCRIPTION: Contains the Dll Entry stuff

	CREATED BY: 

	HISTORY: 

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/
#include "$$root$$.h"

extern ClassDesc2* Get$$CLASS_NAME$$Desc();
$$IF(SPACE_WARP_TYPE)
extern ClassDesc2* Get$$CLASS_NAME$$ObjDesc();
$$ENDIF

HINSTANCE hInstance;
int controlsInit = FALSE;

$$IF(ADD_COMMENTS)
// This function is called by Windows when the DLL is loaded.  This 
// function may also be called many times during time critical operations
// like rendering.  Therefore developers need to be careful what they
// do inside this function.  In the code below, note how after the DLL is
// loaded the first time only a few statements are executed.
$$ENDIF //ADD_COMMENTS

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
	hInstance = hinstDLL;				// Hang on to this DLL's instance handle.

	if (!controlsInit) {
		controlsInit = TRUE;
		InitCustomControls(hInstance);	// Initialize MAX's custom controls
		InitCommonControls();			// Initialize Win95 controls
	}
			
	return (TRUE);
}

$$IF(ADD_COMMENTS)
// This function returns a string that describes the DLL and where the user
// could purchase the DLL if they don't have it.
$$ENDIF //ADD_COMMENTS 
__declspec( dllexport ) const TCHAR* LibDescription()
{
	return GetString(IDS_LIBDESCRIPTION);
}

$$IF(ADD_COMMENTS)
// This function returns the number of plug-in classes this DLL
$$ENDIF //ADD_COMMENTS 
//TODO: Must change this number when adding a new class
__declspec( dllexport ) int LibNumberClasses()
{
$$IF(!SPACE_WARP_TYPE)
	return 1;
$$ELSE
	return 2;
$$ENDIF
}

$$IF(ADD_COMMENTS)
// This function returns the number of plug-in classes this DLL
$$ENDIF //ADD_COMMENTS 
__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
{
	switch(i) {
		case 0: return Get$$CLASS_NAME$$Desc();
$$IF(SPACE_WARP_TYPE)
		case 1: return Get$$CLASS_NAME$$ObjDesc();
$$ENDIF
		default: return 0;
	}
}

$$IF(ADD_COMMENTS)
// This function returns a pre-defined constant indicating the version of 
// the system under which it was compiled.  It is used to allow the system
// to catch obsolete DLLs.
$$ENDIF //ADD_COMMENTS 
__declspec( dllexport ) ULONG LibVersion()
{
	return VERSION_3DSMAX;
}

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

