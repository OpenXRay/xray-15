/**********************************************************************
 *<
	FILE: mods.cpp

	DESCRIPTION:   DLL implementation of modifiers

	CREATED BY: Rolf Berteig (based on prim.cpp)

	HISTORY: created 30 January 1995

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "mods.h"
#include "buildver.h"

HINSTANCE hInstance;
int controlsInit = FALSE;

#define MAX_MOD_OBJECTS	52 // LAM - 2/3/03 - bounced up from 51
ClassDesc *classDescArray[MAX_MOD_OBJECTS];
int classDescCount = 0;

#if !defined DESIGN_VER

// russom - 05/24/01
// The classDesc array was created because it was way to difficult
// to maintain the switch statement return the Class Descriptors
// with so many different #define used in this module.

void initClassDescArray(void)
{

	classDescArray[classDescCount++] = GetBendModDesc();
	classDescArray[classDescCount++] = GetTaperModDesc();
#ifndef NO_SPACEWARPS
	classDescArray[classDescCount++] = GetSinWaveObjDesc();
	classDescArray[classDescCount++] = GetSinWaveModDesc();
#endif
#ifndef NO_MODIFIER_DELETE_MESH	// russom - 10/11/01
	classDescArray[classDescCount++] = GetEditMeshModDesc();
#endif
	classDescArray[classDescCount++] = GetTwistModDesc();
	classDescArray[classDescCount++] = GetExtrudeModDesc();
#ifndef NO_SPACEWARPS
	classDescArray[classDescCount++] = GetBombObjDesc();
	classDescArray[classDescCount++] = GetBombModDesc();		
#endif
	classDescArray[classDescCount++] = GetClustModDesc();
	classDescArray[classDescCount++] = GetSkewModDesc();
	classDescArray[classDescCount++] = GetNoiseModDesc();
	classDescArray[classDescCount++] = GetSinWaveOModDesc();
#ifndef NO_SPACEWARPS
	classDescArray[classDescCount++] = GetLinWaveObjDesc();
	classDescArray[classDescCount++] = GetLinWaveModDesc();
#endif
	classDescArray[classDescCount++] = GetLinWaveOModDesc();
	classDescArray[classDescCount++] = GetOptModDesc();
	classDescArray[classDescCount++] = GetDispModDesc();
	classDescArray[classDescCount++] = GetClustNodeModDesc();
#ifndef NO_SPACEWARPS
	classDescArray[classDescCount++] = GetGravityObjDesc();
	classDescArray[classDescCount++] = GetGravityModDesc();
	classDescArray[classDescCount++] = GetWindObjDesc();
	classDescArray[classDescCount++] = GetWindModDesc();
	classDescArray[classDescCount++] = GetDispObjDesc();
	classDescArray[classDescCount++] = GetDispWSModDesc();
	classDescArray[classDescCount++] = GetDeflectObjDesc();
	classDescArray[classDescCount++] = GetDeflectModDesc();
#endif
	classDescArray[classDescCount++] = GetUVWMapModDesc();
	classDescArray[classDescCount++] = GetSelModDesc();
	classDescArray[classDescCount++] = GetSmoothModDesc();
#ifndef NO_MODIFIER_MATERIAL	// russom - 12/10/01
	classDescArray[classDescCount++] = GetMatModDesc();
#endif
	classDescArray[classDescCount++] = GetNormalModDesc();
	classDescArray[classDescCount++] = GetSurfrevModDesc();
#ifndef NO_UTILITY_RESETXFORM	// russom - 12/04/01
	classDescArray[classDescCount++] = GetResetXFormDesc();
#endif
	classDescArray[classDescCount++] = GetAFRModDesc();
	classDescArray[classDescCount++] = GetTessModDesc();
#ifndef NO_MODIFIER_DELETE_MESH	// russom - 10/11/01
	classDescArray[classDescCount++] = GetDeleteModDesc();
#endif
	classDescArray[classDescCount++] = GetMeshSelModDesc();
	classDescArray[classDescCount++] = GetFaceExtrudeModDesc();
#ifndef NO_MODIFIER_UVW_XFORM 	// russom - 10/11/01
	classDescArray[classDescCount++] = GetUVWXFormModDesc();
	classDescArray[classDescCount++] = GetUVWXFormMod2Desc();
#endif
	classDescArray[classDescCount++] = GetMirrorModDesc();
#ifndef NO_SPACEWARPS
	classDescArray[classDescCount++] = GetBendWSMDesc();
	classDescArray[classDescCount++] = GetTwistWSMDesc();
	classDescArray[classDescCount++] = GetTaperWSMDesc();
	classDescArray[classDescCount++] = GetSkewWSMDesc();
	classDescArray[classDescCount++] = GetNoiseWSMDesc();
#endif
#ifndef NO_MODIFIER_DELETE_SPLINE	// russom - 10/25/01
	classDescArray[classDescCount++] = GetSDeleteModDesc();
#endif
#if !defined(NO_OUTPUTRENDERER) && !defined(NO_MODIFIER_DISP_APPROX)
	classDescArray[classDescCount++] = GetDispApproxModDesc();
#endif
#if !defined(NO_SPACEWARPS) && !defined(NO_MODIFIER_DISPLACEMESH)
	classDescArray[classDescCount++] = GetMeshMesherWSMDesc();
#endif
	classDescArray[classDescCount++] = GetNormalizeSplineDesc();
#ifndef NO_PATCHES
	classDescArray[classDescCount++] = GetDeletePatchModDesc();
#endif
	DbgAssert (classDescCount <= MAX_MOD_OBJECTS);
}

#else // if defined DESIGN_VER

void initClassDescArray(void)
{

#ifndef NO_MODIFIER_BEND 	// JP Morel - June 28th 2002
    classDescArray[classDescCount++] = GetBendModDesc();
#endif //NO_MODIFIER_BEND
#ifndef NO_MODIFIER_TAPER // JP Morel - June 28th 2002
    classDescArray[classDescCount++] = GetTaperModDesc();
#endif // NO_MODIFIER_TAPER 
    classDescArray[classDescCount++] = GetEditMeshModDesc();
#ifndef NO_MODIFIER_TWIST // JP Morel - June 28th 2002
    classDescArray[classDescCount++] = GetTwistModDesc();
#endif 
    classDescArray[classDescCount++] = GetExtrudeModDesc();
#ifndef NO_MODIFIER_XFORM // JP Morel - June 28th 2002
    classDescArray[classDescCount++] = GetClustModDesc();
#endif	
    classDescArray[classDescCount++] = GetSkewModDesc();
#ifndef NO_MODIFIER_NOISE // JP Morel - June 28th 2002
    classDescArray[classDescCount++] = GetNoiseModDesc();
#endif           
#ifndef NO_MODIFIER_RIPPLE // JP Morel - June 28th 2002
    classDescArray[classDescCount++] = GetSinWaveOModDesc();
    classDescArray[classDescCount++] = GetLinWaveOModDesc();
#endif 
#ifndef NO_MODIFIER_OPTIMIZE // JP Morel - June 28th 2002
    classDescArray[classDescCount++] = GetOptModDesc();
#endif 
#ifndef NO_MODIFIER_DISPLACE // JP Morel - July 23th 2002
    classDescArray[classDescCount++] = GetDispModDesc();
#endif
	classDescArray[classDescCount++] = GetClustNodeModDesc();
	classDescArray[classDescCount++] = GetUVWMapModDesc();
	classDescArray[classDescCount++] = GetSmoothModDesc();
#ifndef NO_MODIFIER_MATERIAL
	classDescArray[classDescCount++] = GetMatModDesc();
#endif
	classDescArray[classDescCount++] = GetNormalModDesc();
	classDescArray[classDescCount++] = GetMeshSelModDesc();

#ifndef NO_MODIFIER_LATHE // JP Morel - July 23th 2002
	classDescArray[classDescCount++] = GetSurfrevModDesc();
#endif
				
#ifndef NO_UTILITY_RESETXFORM	// JP Morel - July 25th 2002
	classDescArray[classDescCount++] = GetResetXFormDesc();
#endif

#ifndef NO_MODIFIER_AFFECTREGION // JP Morel - June 28th 2002
	classDescArray[classDescCount++] = GetAFRModDesc();         
#endif // NO_MODIFIER_AFFECTREGION

#ifndef NO_MODIFIER_TESSELATE // JP Morel - June 28th 2002
	classDescArray[classDescCount++] = GetTessModDesc();
#endif // NO_MODIFIER_TESSELATE 

#ifndef NO_MODIFIER_DELETE_MESH		// JP Morel - June 28th 2002
	classDescArray[classDescCount++] = GetDeleteModDesc();
#endif // NO_MODIFIER_DELETE_MESH

#ifndef NO_MODIFIER_VOLUME_SELECT // JP Morel - July 25th 2002 
	classDescArray[classDescCount++] = GetSelModDesc();
#endif

#ifndef NO_MODIFIER_FACE_EXTRUDE // JP Morel - July 23th 2002
	classDescArray[classDescCount++] = GetFaceExtrudeModDesc();
#endif

	classDescArray[classDescCount++] = GetUVWXFormModDesc();
	classDescArray[classDescCount++] = GetUVWXFormMod2Desc();

#ifndef NO_MODIFIER_MIRROR  // JP Morel - June 28th 2002
	classDescArray[classDescCount++] = GetMirrorModDesc();
#endif // NO_MODIFIER_MIRROR 

#ifndef NO_MODIFIER_BEND 	// JP Morel - June 28th 2002
	classDescArray[classDescCount++] = GetBendWSMDesc();
#endif //NO_MODIFIER_BEND

#ifndef NO_MODIFIER_TWIST // JP Morel - June 28th 2002
	classDescArray[classDescCount++] = GetTwistWSMDesc();
#endif // NO_MODIFIER_TWIST 

#ifndef NO_MODIFIER_TAPER // JP Morel - June 28th 2002
	classDescArray[classDescCount++] = GetTaperWSMDesc();
#endif // NO_MODIFIER_TAPER 

	classDescArray[classDescCount++] = GetSkewWSMDesc();

#ifndef NO_MODIFIER_NOISE // JP Morel - June 28th 2002
	classDescArray[classDescCount++] = GetNoiseWSMDesc();
#endif // NO_MODIFIER_NOISE 

#ifndef NO_MODIFIER_DELETE_SPLINE	// JP Morel - July 23th 2002
	classDescArray[classDescCount++] = GetSDeleteModDesc();
#endif
#ifndef NO_MODIFIER_DISP_APPROX // JP Morel - July 23th 2002
	classDescArray[classDescCount++] = GetDispApproxModDesc();
	classDescArray[classDescCount++] = GetMeshMesherWSMDesc();
#endif
#ifndef NO_MODIFIER_NORMALIZE_SPLINE // JP Morel - July 23th 2002
	classDescArray[classDescCount++] = GetNormalizeSplineDesc();
#endif
#ifndef NO_PATCHES // JP Morel - July 22th 2002
	classDescArray[classDescCount++] = GetDeletePatchModDesc();
#endif
	DbgAssert (classDescCount <= MAX_MOD_OBJECTS);
}

#endif // ifndef DESIGN_VER


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
LibDescription() { return
 GetString(IDS_RB_DEFMODS); }


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

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}

INT_PTR CALLBACK DefaultSOTProc(
		HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	IObjParam *ip = (IObjParam*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch (msg) {
		case WM_INITDIALOG:
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			if (ip) ip->RollupMouseMessage(hWnd,msg,wParam,lParam);
			return FALSE;

		default:
			return FALSE;
		}
	return TRUE;
	}

TCHAR *GetString(int id)
	{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
	}
