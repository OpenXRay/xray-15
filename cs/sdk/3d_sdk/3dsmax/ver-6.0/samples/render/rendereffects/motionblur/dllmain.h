/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: dllMain.h

	 DESCRIPTION: DLL entry header

	 CREATED BY: michael malone (mjm)

	 HISTORY: created October 2, 1998

   	 Copyright (c) 1998, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */
#ifndef __DLL_MAIN__H
#define __DLL_MAIN__H


#include "resource.h"
#include "max.h"
#include "imblur.h"

extern HINSTANCE hInstance;

TCHAR *GetString(int id);
extern ClassDesc* GetMotBlurDesc();

class IMBOpsImp : public IMBOps	{
	DECLARE_DESCRIPTOR(IMBOpsImp)
	NO_FUNCTION_MAP
	ULONG ChannelsRequired(ULONG flags);
	int ApplyMotionBlur(Bitmap *bm, CheckAbortCallback *progCallback, float duration, ULONG flags, Bitmap *extraBM);
	};


#endif

