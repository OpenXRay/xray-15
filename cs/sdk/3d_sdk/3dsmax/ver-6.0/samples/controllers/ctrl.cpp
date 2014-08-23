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

#include "ctrl.h"
#include "buildver.h"

HINSTANCE hInstance;
int controlsInit = FALSE;

// russom - 12/04/01
// The classDesc array was created because it was way to difficult
// to maintain the switch statement return the Class Descriptors
// with so many different #define used in this module.

#define MAX_CTRL_OBJECTS 35
static ClassDesc *classDescArray[MAX_CTRL_OBJECTS];
static int classDescCount = 0;

#ifndef DESIGN_VER	// JP Morel - July 25th 2002
void initClassDescArray(void)
{
	classDescArray[classDescCount++] = GetPathCtrlDesc();
	classDescArray[classDescCount++] = GetEulerCtrlDesc();
	classDescArray[classDescCount++] = GetLocalEulerCtrlDesc();
	classDescArray[classDescCount++] = GetFloatNoiseDesc();
	classDescArray[classDescCount++] = GetExprPosCtrlDesc();
	classDescArray[classDescCount++] = GetExprP3CtrlDesc();
	classDescArray[classDescCount++] = GetExprFloatCtrlDesc();
	classDescArray[classDescCount++] = GetExprScaleCtrlDesc();
	classDescArray[classDescCount++] = GetPositionNoiseDesc();
	classDescArray[classDescCount++] = GetPoint3NoiseDesc();
	classDescArray[classDescCount++] = GetRotationNoiseDesc();
	classDescArray[classDescCount++] = GetScaleNoiseDesc();
	classDescArray[classDescCount++] = GetBoolControlDesc();
	classDescArray[classDescCount++] = GetIPosCtrlDesc();
	classDescArray[classDescCount++] = GetAttachControlDesc();
	classDescArray[classDescCount++] = GetIPoint3CtrlDesc();
	classDescArray[classDescCount++] = GetIColorCtrlDesc();
	classDescArray[classDescCount++] = GetLinkCtrlDesc();
#ifndef NO_UTILITY_FOLLOWBANK	// russom - 12/04/01
	classDescArray[classDescCount++] = GetFollowUtilDesc();
#endif
#ifndef NO_CONTROLLER_SURFACE
	classDescArray[classDescCount++] = GetSurfCtrlDesc();
#endif
	classDescArray[classDescCount++] = GetIScaleCtrlDesc(); // mjm 9.15.98
	classDescArray[classDescCount++] = GetPosConstDesc(); // AG added (Position Constraint) 4/21/2000
	classDescArray[classDescCount++] = GetOrientConstDesc(); // AG added (Orientation Constraint) 5/04/2000
	classDescArray[classDescCount++] = GetLookAtConstDesc(); // AG added (LookAt Constraint) 6/26/2000
	classDescArray[classDescCount++] = GetFloatListDesc();
	classDescArray[classDescCount++] = GetPoint3ListDesc();
	classDescArray[classDescCount++] = GetPositionListDesc();
	classDescArray[classDescCount++] = GetRotationListDesc();
	classDescArray[classDescCount++] = GetScaleListDesc();
	classDescArray[classDescCount++] = GetMasterListDesc();
#ifndef NO_OUTPUTRENDERER	// russom - 05/24/01
	classDescArray[classDescCount++] = GetLODControlDesc();
 #ifndef NO_UTILITY_LOD	// russom - 12/04/01
	classDescArray[classDescCount++] = GetLODUtilDesc();
 #endif
#endif
//	classDescArray[classDescCount++] = GetAdditiveEulerCtrlDesc(); // removed 001018  --prs.
	classDescArray[classDescCount++] = GetIPoint4CtrlDesc();
	classDescArray[classDescCount++] = GetIAColorCtrlDesc();
	classDescArray[classDescCount++] = GetPoint4ListDesc();

	assert(classDescCount <= MAX_CTRL_OBJECTS);
}
#else
void initClassDescArray(void)
{
	classDescArray[classDescCount++] = GetPathCtrlDesc();
	classDescArray[classDescCount++] = GetEulerCtrlDesc();
	classDescArray[classDescCount++] = GetLocalEulerCtrlDesc();
	classDescArray[classDescCount++] = GetFloatNoiseDesc();
	classDescArray[classDescCount++] = GetPositionNoiseDesc();

	classDescArray[classDescCount++] = GetPoint3NoiseDesc();
	classDescArray[classDescCount++] = GetRotationNoiseDesc();
	classDescArray[classDescCount++] = GetScaleNoiseDesc();
	classDescArray[classDescCount++] = GetBoolControlDesc();
	classDescArray[classDescCount++] = GetIPosCtrlDesc();

	classDescArray[classDescCount++] = GetAttachControlDesc();
	classDescArray[classDescCount++] = GetIPoint3CtrlDesc();
	classDescArray[classDescCount++] = GetIColorCtrlDesc();
	classDescArray[classDescCount++] = GetLinkCtrlDesc();
#ifndef NO_UTILITY_FOLLOWBANK	// russom - 12/04/01
	classDescArray[classDescCount++] = GetFollowUtilDesc();
#endif
#ifndef NO_CONTROLLER_SURFACE
	classDescArray[classDescCount++] = GetSurfCtrlDesc();
#endif
	classDescArray[classDescCount++] = GetIScaleCtrlDesc(); // mjm 9.15.98
	classDescArray[classDescCount++] = GetPosConstDesc(); // AG added (Position Constraint) 4/21/2000
	classDescArray[classDescCount++] = GetOrientConstDesc(); // AG added (Orientation Constraint) 5/04/2000
	classDescArray[classDescCount++] = GetLookAtConstDesc(); // AG added (LookAt Constraint) 6/26/2000

	classDescArray[classDescCount++] = GetFloatListDesc();
	classDescArray[classDescCount++] = GetPoint3ListDesc();
	classDescArray[classDescCount++] = GetPositionListDesc();
	classDescArray[classDescCount++] = GetRotationListDesc();
	classDescArray[classDescCount++] = GetScaleListDesc();
	classDescArray[classDescCount++] = GetMasterListDesc();

	classDescArray[classDescCount++] = GetIPoint4CtrlDesc();
	classDescArray[classDescCount++] = GetIAColorCtrlDesc();
	classDescArray[classDescCount++] = GetPoint4ListDesc();

	assert(classDescCount <= MAX_CTRL_OBJECTS);
}
#endif // DESIGN_VER

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

		// CAL-9/17/2002: change default bool from on/off controller to boolean controller
        // SetDefaultBoolController(GetBoolControlDesc());

// xavier robitaille | 03.02.05 | bezier default position controller
#ifndef BEZIER_DEFAULT_POS_CTRL
        SetDefaultController(CTRL_POSITION_CLASS_ID, GetIPosCtrlDesc());
#endif
        SetDefaultController(CTRL_ROTATION_CLASS_ID, GetEulerCtrlDesc());
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
 GetString(IDS_RB_DEFCONTROLLERS); }



/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses() {
	return classDescCount;
} 

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

// The method is a copy of the method in "maxsdk\Samples\Objects\Particles\suprprts.cpp
// Please duplicate any changes there as well
int computeHorizontalExtent(HWND hListBox, BOOL useTabs, int cTabs, LPINT lpnTabs)
{
	int i;
	int count = SendMessage(hListBox, LB_GETCOUNT, 0, 0);
	HFONT font;
	HDC dc;
	int width=0;
	LPSTR Buffer;
	int saved;
	SIZE margin;

	// first, we must make sure we have a buffer
	// large enough to hold the longest string
	for (i=0; i < count; i++)
	{	/* compute buffer size */
		int len = SendMessage(hListBox, LB_GETTEXTLEN, i, 0);
		width = max(width, len);
	} 

	// if the list box is empty, jsut return 0
	if (width ==0)
		return 0;

	// allocate a buffer to hold the string
	// including the terminating NULL character
	Buffer = (TCHAR*)malloc ((width+1) * sizeof(TCHAR));
	if (Buffer == NULL)
		return 0;

	// we will need a DC for string length computation
	dc = GetDC(hListBox);

	// save the DC so we can restore it later
	saved = SaveDC(dc);

	font = GetWindowFont (hListBox);

	// if our font is other then the system font select it into the DC
	if (font != NULL)
		SelectFont (dc, font);

	// we now compute the longest actual string length
	width = 0;
	for(i=0; i < count; i++)
	{ /* compute the buffer size */
		int cx=0;
		SendMessage(hListBox, LB_GETTEXT, i, (LPARAM) (LPCTSTR) Buffer);
		if (useTabs) {
			DWORD sz = GetTabbedTextExtent(dc, Buffer, _tcslen(Buffer), cTabs, lpnTabs );
			cx = LOWORD(sz);
		}
		else {
			SIZE sz;
			GetTextExtentPoint32 (dc, Buffer, _tcslen(Buffer), &sz);
			cx = sz.cx;
		}		
		width = max (width, cx );
	}	
	GetTextExtentPoint32 (dc, _T(" "), _tcslen(_T(" ")), &margin);

	// we no longer need the buffer or DC; free them
	free (Buffer);
	RestoreDC(dc, saved);
	ReleaseDC(hListBox, dc);

	// deal with the (possible) presence of a scroll bar
	width += GetSystemMetrics(SM_CXVSCROLL) + 2*margin.cx;
	return width;
}
