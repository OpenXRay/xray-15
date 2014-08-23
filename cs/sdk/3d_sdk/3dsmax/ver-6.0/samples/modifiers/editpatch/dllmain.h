/**********************************************************************
 *<
	FILE:			dllmain.h

	DESCRIPTION:	Project header for Edit Patch

	CREATED BY:		Christer Janson

	HISTORY:

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#ifndef __DLLMAIN__H
#define __DLLMAIN__H

#include "Max.h"
#include "resource.h"

#define EDITSPLINE_CLASS_ID			0x00060
#define EDITPATCH_CLASS_ID			0x00070

extern ClassDesc* GetEditPatchModDesc();
extern ClassDesc* GetEditSplineModDesc();

TCHAR *GetString(int id);

extern HINSTANCE hInstance;

#define BIGFLOAT	float(999999)

#define NEWSWMCAT	_T("Modifiers")

#endif

