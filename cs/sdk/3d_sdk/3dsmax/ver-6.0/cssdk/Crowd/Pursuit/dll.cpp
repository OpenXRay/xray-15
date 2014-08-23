//*********************************************************************
//	Crowd / Unreal Pictures / dll.cpp
//	Copyright (c) 2000, All Rights Reserved.
//*********************************************************************

#include "dll.h"
#include "resource.h"

HINSTANCE hInstance = NULL;
HINSTANCE hResource = NULL;
int resourcesLoaded = FALSE;
int controlsInit = FALSE;
int messagesent = FALSE;
#define MAX_PATH_LENGTH 257
#define MAX_STRING_LENGTH 256
int triedToLoad = FALSE;
;

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if (hResource)
		return LoadString(hResource, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

/** public functions **/
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
	hInstance = hinstDLL;

	if ( !controlsInit ) {
		controlsInit = TRUE;
		
		// initialize Chicago controls
		InitCommonControls();
		}

    // If we have already tried to load the resource
    // file and failed just give up.
    if (triedToLoad && ! resourcesLoaded)
        return FALSE;
    // Load our resources.  We look for the file in the
    // same directory where this DLL was found

    if (! resourcesLoaded) {
        // Where this DLL resides
        char dirName[MAX_PATH_LENGTH];
        // Full path name to resource DLL
        char dllName[MAX_PATH_LENGTH];
        char *chPtr;
        GetModuleFileName (hinstDLL, dirName, MAX_PATH_LENGTH);
        // Strip off the file name
        chPtr = dirName + strlen (dirName);
        while (*(--chPtr) != '\\')
            ;
        *(chPtr+1) = 0;
        // Add in "epsres.dll"
        strcpy (dllName, dirName);

        strcat (dllName, "pursuit.str");
        // Load resource DLL
        // Turn off error reporting
        int errorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX);
        hResource = LoadLibraryEx(dllName, NULL, 0);
        SetErrorMode(errorMode);
        // Be sure to check to see if we succeeded
        // loading resource DLL
        if (hResource) {
            resourcesLoaded = TRUE;
            InitCustomControls (hResource);
        } else {
            triedToLoad = TRUE;

            MessageBox (NULL,"Crowd Pursuit Behavior Plugin failed to load due to missing resource file pursuit.str",
            "CROWD", MB_ICONINFORMATION);
            return FALSE;
        }
    }

	switch(fdwReason) {
		case DLL_PROCESS_ATTACH:
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			if (hResource) 
			{
				FreeLibrary(hResource);
				hResource = NULL;
			}
			break;
		}
	return(TRUE);
	}


//------------------------------------------------------
// Dll Library Functions
//------------------------------------------------------

__declspec( dllexport ) const TCHAR* LibDescription() { return GetString(IDS_LIBDESC); }

__declspec( dllexport ) int LibNumberClasses()
{
	return 1;
}

__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
{
	switch(i) {
		case 0:  return GetPursuitBhvrDesc();
		default: return 0;
	}
}

__declspec( dllexport ) ULONG LibVersion()
{
	return VERSION_3DSMAX;
}



