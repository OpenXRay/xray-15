/**********************************************************************
 *<
    FILE: ctrl.cpp

    DESCRIPTION:   DLL implementation of some controllers

    CREATED BY: Rolf Berteig

    HISTORY: created 13 June 1995

	         added independent scale controller (ScaleXYZ)
			 see file "./indescale.cpp"
			   mjm 9.15.98

 *> Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "block.h"
#include "buildver.h"

#define MAX_CONTROLLERS	8
ClassDesc *classDescArray[MAX_CONTROLLERS];
int classDescCount = 0;

void initClassDescArray(void)
{
	classDescArray[classDescCount++] = GetMasterBlockDesc();
	classDescArray[classDescCount++] = GetBlockControlDesc();
	classDescArray[classDescCount++] = GetSlaveFloatDesc();
#ifndef NO_CONTROLLER_SLAVE_POSITION
	classDescArray[classDescCount++] = GetSlavePosDesc();
#endif
	classDescArray[classDescCount++] = GetControlContainerDesc();
#ifndef NO_CONTROLLER_SLAVE_ROTATION
	classDescArray[classDescCount++] = GetSlaveRotationDesc();
#endif
#ifndef NO_CONTROLLER_SLAVE_SCALE
	classDescArray[classDescCount++] = GetSlaveScaleDesc();
#endif
	classDescArray[classDescCount++] = GetSlavePoint3Desc();
}

HINSTANCE hInstance;
int controlsInit = FALSE;

/** public functions **/
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
    hInstance = hinstDLL;

    if ( !controlsInit ) {
        controlsInit = TRUE;
        
        initClassDescArray();

        // jaguar controls
        InitCustomControls(hInstance);

#ifdef OLD3DCONTROLS
        // initialize 3D controls
        Ctl3dRegister(hinstDLL);
        Ctl3dAutoSubclass(hinstDLL);
#endif
        
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
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() { return _T("Block controller (Discreet)"); }


/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses() { return classDescCount; } // mjm 9.15.98

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

    if(hInstance)
        return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
    return NULL;
}

