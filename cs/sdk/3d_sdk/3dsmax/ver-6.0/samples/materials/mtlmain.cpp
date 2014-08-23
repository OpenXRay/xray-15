/**********************************************************************
 *<
	FILE: mtl.cpp

	DESCRIPTION:   DLL implementation of material and textures

	CREATED BY: Dan Silva

	HISTORY: created 12 December 1994

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "mtlhdr.h"
#include "stdmat.h"
#include "mtlres.h"
#include "mtlresOverride.h"

HINSTANCE hInstance;
int controlsInit = FALSE;

// orb 01-03-2001 Removing map types
static void initClassDescArray(void);  // forward declaration

/** public functions **/
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
	hInstance = hinstDLL;

	if ( !controlsInit ) {
		controlsInit = TRUE;
		
		initClassDescArray();
		
		// jaguar controls
		InitCustomControls(hInstance);

		// initialize Chicago controls
		InitCommonControls();

		// register SXP readers
		RegisterSXPReader(_T("MARBLE_I.SXP"), Class_ID(MARBLE_CLASS_ID,0));
		RegisterSXPReader(_T("NOISE_I.SXP"),  Class_ID(NOISE_CLASS_ID,0));
		RegisterSXPReader(_T("NOISE2_I.SXP"), Class_ID(NOISE_CLASS_ID,0));
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
// This is the interface to Max:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_DS_MTLDESC); }

// orb - 01/03/01
// The classDesc array was created because it was way to difficult
// to maintain the switch statement return the Class Descriptors
// with so many different #define used in this module.

#define MAX_MTLTEX_OBJECTS 32
static ClassDesc *classDescArray[MAX_MTLTEX_OBJECTS];
static int classDescCount = 0;

void initClassDescArray(void)
{
#ifdef USE_STDMTL2_AS_STDMTL
		classDescArray[classDescCount++] =  GetStdMtl2Desc();
#else
		classDescArray[classDescCount++] = GetStdMtlDesc();
#endif
		classDescArray[classDescCount++] = GetMultiDesc();

#ifndef NO_MTL_TOPBOTTOM		
		classDescArray[classDescCount++] = GetCMtlDesc();
#endif // NO_MTL_TOPBOTTOM

		classDescArray[classDescCount++] = GetBMTexDesc();
		classDescArray[classDescCount++] = GetMaskDesc();
		
#ifndef NO_MAPTYPE_RGBTINT
		classDescArray[classDescCount++] = GetTintDesc();
#endif // NO_MAPTYPE_RGBTINT

		
		classDescArray[classDescCount++] = GetCheckerDesc();
		classDescArray[classDescCount++] = GetMixDesc();
		classDescArray[classDescCount++] = GetMarbleDesc();
		classDescArray[classDescCount++] = GetNoiseDesc();
		classDescArray[classDescCount++] = GetTexmapsDesc();
		classDescArray[classDescCount++] = GetDoubleSidedDesc();
		classDescArray[classDescCount++] = GetMixMatDesc();

#ifndef NO_MAPTYPE_REFLECTREFRACT
		classDescArray[classDescCount++] = GetACubicDesc();
#endif // NO_MAPTYPE_REFLECTREFRACT

#ifndef NO_MAPTYPE_FLATMIRROR 
		classDescArray[classDescCount++] = GetMirrorDesc();
#endif // NO_MAPTYPE_FLATMIRROR

#ifndef NO_MAPTYPE_GRADIENT		
		classDescArray[classDescCount++] = GetGradientDesc();
#endif // NO_MAPTYPE_GRADIENT


		classDescArray[classDescCount++] = GetCompositeDesc();

#ifndef NO_MTL_MATTESHADOW
		classDescArray[classDescCount++] = GetMatteDesc();
#endif // NO_MTL_MATTESHADOW

#ifndef NO_MAPTYPE_RGBMULT
		classDescArray[classDescCount++] = GetRGBMultDesc();
#endif // NO_MAPTYPE_RGBMULT
		
#ifndef NO_MAPTYPE_OUTPUT
		classDescArray[classDescCount++] = GetOutputDesc();
#endif // NO_MAPTYPE_OUTPUT
		
		classDescArray[classDescCount++] = GetFalloffDesc();

#ifndef	NO_MAPTYPE_VERTCOLOR
		classDescArray[classDescCount++] = GetVColDesc();
#endif // NO_MAPTYPE_VERTCOLOR

#ifndef USE_LIMITED_STDMTL 
		classDescArray[classDescCount++] = GetPhongShaderCD();
		classDescArray[classDescCount++] = GetMetalShaderCD();
#endif
		classDescArray[classDescCount++] = GetBlinnShaderCD();
		classDescArray[classDescCount++] = GetOldBlinnShaderCD();

#ifndef NO_MAPTYPE_THINWALL
		classDescArray[classDescCount++] = GetPlateDesc();
#endif // NO_MAPTYPE_THINWALL

		classDescArray[classDescCount++] = GetOldTexmapsDesc();

#ifndef NO_MTL_COMPOSITE
		classDescArray[classDescCount++] = GetCompositeMatDesc();
#endif // NO_MTL_COMPOSITE

#ifndef DESIGN_VER
#ifndef NO_PARTICLES // orb 07-11-01

#ifndef NO_MAPTYPE_PARTICLEMBLUR
		classDescArray[classDescCount++] = GetPartBlurDesc();
#endif  // NO_MAPTYPE_PARTICLEMBLUR

#ifndef NO_MAPTYPE_PARTICLEAGE
		classDescArray[classDescCount++] = GetPartAgeDesc();
#endif // NO_MAPTYPE_PARTICLEAGE


#endif // NO_PARTICLES
#endif // DESIGN_VER

		classDescArray[classDescCount++] = GetBakeShellDesc();



		}

__declspec( dllexport ) int LibNumberClasses() { return classDescCount; }

// This function return the ith class descriptor.
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
