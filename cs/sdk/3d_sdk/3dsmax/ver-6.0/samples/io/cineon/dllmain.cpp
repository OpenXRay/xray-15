/**********************************************************************
 *
 * FILE:        dllmain.cpp
 * AUTHOR:      greg finch
 *
 * DESCRIPTION: SMPTE Digital Picture Exchange Format,
 *              SMPTE CIN, Kodak Cineon
 *
 * CREATED:     20 june, 1997
 *
 *
 * 
 * Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/
 
#include "max.h"
#include "resource.h"
#define DLLExport __declspec(dllexport) 

HINSTANCE   hInst       = NULL;
BOOL        ctrlsInit   = FALSE;

extern ClassDesc*   Get_CIN_Desc();

BOOL WINAPI
DllMain(HINSTANCE hDLLInst, ULONG reason, LPVOID reserved)
{
    if ( !ctrlsInit ) {
        ctrlsInit = TRUE;
        InitCustomControls(hDLLInst);
	    //InitCommonControls();
    }

    switch(reason) {
        case DLL_PROCESS_ATTACH:
            if (hInst)
                return FALSE;
            hInst = hDLLInst;
            break;
        case DLL_PROCESS_DETACH:
            hInst = NULL;
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
    }
    return(TRUE);
}

DLLExport const TCHAR*
LibDescription()
{
    static TCHAR buf[128];
    if (hInst)  return LoadString(hInst, IDS_CIN_DLLDesc, buf, sizeof(buf)) ? buf : NULL;
    else        return NULL;
}

DLLExport int
LibNumberClasses()
{
    return 1;
}

DLLExport ClassDesc*
LibClassDesc(int i)
{
    switch(i) {
        case  0:
            return Get_CIN_Desc();
        default:
            return NULL;
    }
}

DLLExport ULONG 
LibVersion()
{
    return VERSION_3DSMAX;
}

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}
