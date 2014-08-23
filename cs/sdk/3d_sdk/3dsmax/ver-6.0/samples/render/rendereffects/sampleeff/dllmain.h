/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: dllMain.h

	 DESCRIPTION: DLL entry header

	 CREATED BY: michael malone (mjm)

	 HISTORY: created January 27, 1999

   	 Copyright (c) 1998, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */
#ifndef __DLL_MAIN__H
#define __DLL_MAIN__H


#include "resource.h"
#include "max.h"

extern HINSTANCE hInstance;

TCHAR *GetString(int id);
extern ClassDesc* GetSampleEffDesc();


#endif

