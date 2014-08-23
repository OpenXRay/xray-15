/**********************************************************************
 *<
	FILE: DllEntry.cpp

	DESCRIPTION: Contains the Dll Entry stuff

	CREATED BY: Neil Hazzard

	HISTORY: 02/15/02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/
#include "ViewportManager.h"

extern ClassDesc2* GetViewportManagerControlDesc();
extern ClassDesc2* GetViewportLoaderDesc();

HINSTANCE hInstance;
int controlsInit = FALSE;


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

__declspec( dllexport ) const TCHAR* LibDescription()
{
	return GetString(IDS_LIBDESCRIPTION);
}

//TODO: Must change this number when adding a new class
__declspec( dllexport ) int LibNumberClasses()
{
	return 2;
}

__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
{
	switch(i) {
		case 0: return GetViewportManagerControlDesc();
		case 1: return GetViewportLoaderDesc();
		default: return 0;
	}
}

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

BOOL CheckForDX()
{
	ViewExp *vpt;
	vpt = GetCOREInterface()->GetActiveViewport();	
	GraphicsWindow *gw = vpt->getGW();
	if(gw->querySupport(GW_SPT_GEOM_ACCEL))
	{
		IHardwareShader * phs = (IHardwareShader*)gw->GetInterface(HARDWARE_SHADER_INTERFACE_ID);
		if(phs)
			return true;
		else
			return false;
	}
	else
		return false;

}