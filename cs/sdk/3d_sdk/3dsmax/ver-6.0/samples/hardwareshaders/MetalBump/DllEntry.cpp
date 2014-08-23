/**********************************************************************
 *<
	FILE: DllEntry.cpp

	DESCRIPTION: Contains the Dll Entry stuff

	CREATED BY: 

	HISTORY: 

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/
#include "MetalBump.h"
#include "MSEmulator.h"

extern ClassDesc2* GetDefaultShaderDesc();
extern ClassDesc2* GetMSEmulatorDesc();

HINSTANCE	hInstance;
int			controlsInit = FALSE;

//_____________________________________________________________________________
//
//	Functions	
//
//_____________________________________________________________________________


BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
//	char	Buff[256];
//	DWORD	Version;

	hInstance = hinstDLL;				

/*	Version = Get3DSMAXVersion();

	if(GET_MAX_RELEASE(Version) < MAX_RELEASE)
	{
		sprintf(Buff,"Need Max version:%d\n",MAX_RELEASE);
		MessageBox(NULL,Buff,"Error",MB_OK | MB_ICONWARNING | MB_SETFOREGROUND | MB_APPLMODAL);
		return(false);
	} 
*/

	if(!controlsInit) 
	{
		controlsInit = TRUE;
		InitCustomControls(hInstance);	
		InitCommonControls();			

	}

	if(fdwReason == DLL_PROCESS_ATTACH || fdwReason == DLL_THREAD_ATTACH)
	{	
		AVIFileInit();
	}
	else if(fdwReason == DLL_PROCESS_DETACH || fdwReason == DLL_THREAD_DETACH)
	{
		AVIFileExit();
	}


	CoInitialize(NULL);


	return(true);
}


//_____________________________________
//
//	LibDescription 
//
//_____________________________________

__declspec( dllexport ) const TCHAR* LibDescription()
{
	return GetString(IDS_LIBDESC);
}

//_____________________________________
//
//	LibNumberClasses 
//
//_____________________________________

__declspec( dllexport ) int LibNumberClasses()
{
	return 2;
}

//_____________________________________
//
//	LibClassDesc 
//
//_____________________________________

__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
{
	switch(i) 
	{
		case 0: 
					return GetDefaultShaderDesc();
		case 1:
					return GetMSEmulatorDesc();

		default:
					return 0;
	}
}

//_____________________________________
//
//	LibVersion
//
//_____________________________________

__declspec( dllexport ) ULONG LibVersion()
{
	return VERSION_3DSMAX;
}

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
    return 1;
}

//_____________________________________
//
//	GetString 
//
//_____________________________________

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
	{
		return(LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL);
	}

	return(NULL);
}

