/*==============================================================================

  file:	        plugin.cpp

  author:       Daniel Levesque
  
  created:      13 August 2001
  
  description:  Entry point for this plugin
  
  modified:	


© 2001 Autodesk
==============================================================================*/

#include <max.h>
#include "PseudoMap.h"
#include <iparamb2.h>
#include "resource.h"
#include "RenElem.h"

HINSTANCE hInstance;


TCHAR *GetString( int id );

//------------------------------------------------------------------------------
//

BOOL WINAPI DllMain(
                    HINSTANCE   hinstDLL,
                    ULONG       fdwReason,
                    LPVOID      lpvReserved
                    )
{
    
    static bool controlsInit = false;
    
    hInstance = hinstDLL;
    
    switch(fdwReason) {
    case DLL_PROCESS_ATTACH:
        //MessageBox(NULL,_T("PRIM.DLL: DllMain)",_T("Prim"),MB_OK);
        if ( !controlsInit ) {
            controlsInit = true;
            InitCustomControls(hInstance);
            InitCommonControls();
        }
        
        DisableThreadLibraryCalls( hinstDLL );
        
        break;
    }
    
    return true;
}


// This is the interface to Jaguar:

//------------------------------------------------------------------------------
//

extern "C" __declspec( dllexport ) const TCHAR* LibDescription()
{
    return GetString(IDS_EXPCTL_LIBRARY);
}


//------------------------------------------------------------------------------
//

extern "C" __declspec( dllexport ) int LibNumberClasses()
{
    return 3;
}

//------------------------------------------------------------------------------
//

extern "C" __declspec( dllexport ) ClassDesc* LibClassDesc( int i )
{
    switch(i) {
    case 0:
        return PseudoMap::GetClassDesc();
    case 1:
        return GetLuminationClassDesc();
    case 2:
        return GetIlluminationClassDesc();
    default:
        return NULL;
    }
}

//------------------------------------------------------------------------------
//

// Return version so can detect obsolete DLLs
extern "C" __declspec( dllexport ) ULONG  LibVersion()
{
    return VERSION_3DSMAX;
}

//------------------------------------------------------------------------------
//

TCHAR *GetString( int id )
{
    static TCHAR buf[256];
    
    if (hInstance) {
        return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
    }
    
    return NULL;
}
