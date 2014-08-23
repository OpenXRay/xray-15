/**********************************************************************
 *<
	FILE: prim.cpp

	DESCRIPTION:   DLL implementation of primitives

	CREATED BY: Dan Silva

	HISTORY: created 12 December 1994

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "buildver.h"
#include "prim.h"
#include "3dsurfer.h"

HINSTANCE hInstance;
int controlsInit = FALSE;
SurferPatchDataReaderCallback patchReader;
SurferSplineDataReaderCallback splineReader;

// russom - 05/07/01
// The classDesc array was created because it was way to difficult
// to maintain the switch statement return the Class Descriptors
// with so many different #define used in this module.
#define MAX_PRIM_OBJECTS	41
ClassDesc *classDescArray[MAX_PRIM_OBJECTS];
int classDescCount = 0;

void initClassDescArray(void)
{
	classDescArray[classDescCount++] = GetBoxobjDesc();
	classDescArray[classDescCount++] = GetSphereDesc();
	classDescArray[classDescCount++] = GetCylinderDesc();
	classDescArray[classDescCount++] = GetLookatCamDesc();
	classDescArray[classDescCount++] = GetSimpleCamDesc();
	classDescArray[classDescCount++] = GetTargetObjDesc();
#ifndef NO_OBJECT_SPOT_LIGHT
	classDescArray[classDescCount++] = GetTSpotLightDesc();
	classDescArray[classDescCount++] = GetFSpotLightDesc();
#endif
#ifndef NO_OBJECT_DIRECT_LIGHT
	classDescArray[classDescCount++] = GetTDirLightDesc();
	classDescArray[classDescCount++] = GetDirLightDesc();
#endif
#ifndef NO_OBJECT_OMNI_LIGHT
	classDescArray[classDescCount++] = GetOmniLightDesc();
#endif
#ifndef NO_OBJECT_SHAPES_SPLINES
	classDescArray[classDescCount++] = GetSplineDesc();
	classDescArray[classDescCount++] = GetNGonDesc();
	classDescArray[classDescCount++] = GetDonutDesc();
#endif
#ifndef NO_OBJECT_BONE
	classDescArray[classDescCount++] = GetBonesDesc();
#endif // NO_OBJECT_BONE
#ifndef NO_OBJECT_RINGARRAY	// russom - 10/12/01
	classDescArray[classDescCount++] = GetRingMasterDesc();
	classDescArray[classDescCount++] = GetSlaveControlDesc();
#endif
#ifndef NO_PATCHES
	classDescArray[classDescCount++] = GetQuadPatchDesc();
	classDescArray[classDescCount++] = GetTriPatchDesc();
#endif
#ifndef NO_OBJECT_STANDARD_PRIMITIVES
	classDescArray[classDescCount++] = GetTorusDesc();
#endif
#ifndef NO_OBJECT_MORPH	// russom - 10/15/01
	classDescArray[classDescCount++] = GetMorphObjDesc();
#endif
#ifndef NO_CONTROLLER_MORPH	// russom - 10/15/01
	classDescArray[classDescCount++] = GetCubicMorphContDesc();
#endif
#ifndef NO_OBJECT_SHAPES_SPLINES
	classDescArray[classDescCount++] = GetRectangleDesc();
#endif
#ifndef NO_OBJECT_BOOL
	classDescArray[classDescCount++] = GetBoolObjDesc();
#endif
#ifndef NO_HELPER_TAPE // russom 10/16/01
	classDescArray[classDescCount++] = GetTapeHelpDesc();
#endif
#ifndef NO_OBJECT_STANDARD_PRIMITIVES
	classDescArray[classDescCount++] = GetTubeDesc();
	classDescArray[classDescCount++] = GetConeDesc();
#endif
#if !defined(NO_EXTENDED_PRIMITIVES) && !defined(NO_OBJECT_HEDRA)
	classDescArray[classDescCount++] = GetHedraDesc();
#endif
#ifndef NO_OBJECT_SHAPES_SPLINES
	classDescArray[classDescCount++] = GetCircleDesc();
	classDescArray[classDescCount++] = GetEllipseDesc();
	classDescArray[classDescCount++] = GetArcDesc();
	classDescArray[classDescCount++] = GetStarDesc();
	classDescArray[classDescCount++] = GetHelixDesc();
#endif
#ifndef NO_PARTICLES
	classDescArray[classDescCount++] = GetRainDesc();
	classDescArray[classDescCount++] = GetSnowDesc();
#endif // NO_PARTICLES
#ifndef NO_OBJECT_SHAPES_SPLINES
	classDescArray[classDescCount++] = GetTextDesc();
#endif
#ifndef NO_OBJECT_TEAPOT
	classDescArray[classDescCount++] = GetTeapotDesc();
#endif // NO_OBJECT_TEAPOT
#ifndef NO_CONTROLLER_MORPH	// russom - 10/15/01
	classDescArray[classDescCount++] = GetBaryMorphContDesc();
#endif
#ifndef NO_HELPER_PROTRACTOR // russom 10/16/01
	classDescArray[classDescCount++] = GetProtHelpDesc();
#endif
// xavier robitaille | 03.02.12 | had been left out...
#ifndef NO_OBJECT_STANDARD_PRIMITIVES
	classDescArray[classDescCount++] = GetGridobjDesc();
#endif
#ifndef NO_OBJECT_BONE
	classDescArray[classDescCount++] = GetNewBonesDesc();
#endif // NO_OBJECT_BONE
}

/** public functions **/
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
	hInstance = hinstDLL;
	static call=0;

	if ( !controlsInit ) {
		controlsInit = TRUE;


		// russom - 05/07/01
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

	if (call==0) {
		RegisterObjectAppDataReader(&patchReader);
		RegisterObjectAppDataReader(&splineReader);
		call++;
		}

	switch(fdwReason) {
		case DLL_PROCESS_ATTACH:
			//MessageBox(NULL,_T("PRIM.DLL: DllMain)",_T("Prim"),MB_OK);
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
LibDescription() { return GetString(IDS_RB_PRIMLIB); }

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses() 
{
	return classDescCount;
}

// russom - 05/07/01 - changed to use classDescArray
__declspec( dllexport ) ClassDesc*
LibClassDesc(int i) 
{

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

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
	}
