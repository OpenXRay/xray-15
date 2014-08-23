/**********************************************************************
 *<
	FILE: motion.cpp

	DESCRIPTION: Motion capture controllers

	CREATED BY: Rolf Berteig

	HISTORY: October 30, 1996

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#include "motion.h"
#include "buildver.h"

HINSTANCE hInstance;
int controlsInit = FALSE;

/** public functions **/
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
	hInstance = hinstDLL;
	if ( !controlsInit ) {
		controlsInit = TRUE;
				
		InitCustomControls(hInstance);
		InitCommonControls();
		}
	return(TRUE);
	}

__declspec( dllexport ) const TCHAR *
LibDescription() { return
 GetString(IDS_LIB_DESCRIPTION); }


__declspec( dllexport ) int LibNumberClasses() {return 15;}

__declspec( dllexport ) ClassDesc*
LibClassDesc(int i) {
	switch(i) {
		case 0:  return	GetMotionManDesc();
		case 1:  return GetPosMotionDesc();
		case 2:  return	GetRotMotionDesc();
		case 3:  return	GetScaleMotionDesc();
		case 4:  return	GetFloatMotionDesc();
		case 5:  return	GetPoint3MotionDesc();		
		case 6:  return GetMouseDeviceClassDescDesc();
		case 7:  return GetMidiDeviceClassDescDesc();
		case 8:  return GetJoyDeviceClassDescDesc();
		case 9:  return GetMouseDeviceClassDescDescOld();
		case 10: return GetMidiDeviceClassDescDescOld();
		case 11: return GetJoyDeviceClassDescDescOld();
		case 12: return GetTheJoyDeviceClassDescDesc();
		case 13: return GetTheMouseDeviceClassDescDesc();
		case 14: return GetTheMidiDeviceClassDescDesc();
		default: return 0;
		}
	}


__declspec( dllexport ) ULONG 
LibVersion() { return VERSION_3DSMAX; }

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if(hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

