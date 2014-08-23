/**********************************************************************
 *<
	FILE: mods.h

	DESCRIPTION:

	CREATED BY: Rolf Berteig (based on prim.h)

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#ifndef __MODS__H
#define __MODS__H

#include "Max.h"
//#include "reslib.h"
#include "modsres.h"


TCHAR *GetString(int id);

#define EDITMESH_CLASS_ID			0x00050
#define EDITSPLINE_CLASS_ID			0x00060
#define EDITPATCH_CLASS_ID			0x00070
#define EDITLOFT_CLASS_ID			0x00080

#define CLUSTOSM_CLASS_ID			0x25215824

#define RESET_XFORM_CLASS_ID		0x8d562b81
#define CLUSTNODEOSM_CLASS_ID		0xc4d33

extern ClassDesc* GetLagModDesc();
// in mods.cpp
extern HINSTANCE hInstance;

// For 'Supports Object of Type' rollups
extern INT_PTR CALLBACK DefaultSOTProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);

#define BIGFLOAT	float(999999)

#define NEWSWMCAT	_T("Modifiers")

#endif

