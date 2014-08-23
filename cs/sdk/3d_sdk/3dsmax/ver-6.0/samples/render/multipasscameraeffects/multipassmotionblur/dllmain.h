/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: dllMain.h

	 DESCRIPTION: DLL entry header

	 CREATED BY: michael malone (mjm)

	 HISTORY: created July 17, 2000

   	 Copyright (c) 2000, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */
#ifndef __DLL_MAIN__H
#define __DLL_MAIN__H


#include "resource.h"
#include "max.h"

extern HINSTANCE hInstance;

TCHAR *GetString(int id);
extern ClassDesc* GetMultiPassMotionBlurClassDesc();


#endif

// ---------------------
// end of file dllmain.h
// ---------------------
