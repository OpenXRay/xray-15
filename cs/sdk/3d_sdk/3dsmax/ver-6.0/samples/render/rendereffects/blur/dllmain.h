/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: dllMain.h

	 DESCRIPTION: DLL entry header

	 CREATED BY: michael malone (mjm)

	 HISTORY: created November 4, 1998

   	 Copyright (c) 1998, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */


#if !defined(_DLL_MAIN_H_INCLUDED_)
#define _DLL_MAIN_H_INCLUDED_

// maxsdk includes
#include "max.h"

extern HINSTANCE hInstance;

TCHAR *GetString(int id);
extern ClassDesc* GetBlurMgrDesc();

#endif // !defined(_DLL_MAIN_H_INCLUDED_)

