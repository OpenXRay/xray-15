/**********************************************************************
 *<
	FILE: LightMap.h

	DESCRIPTION:	Includes for Plugins

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#ifndef __LIGHTMAP__H
#define __LIGHTMAP__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"

#include "gamma.h"


extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

// Some utilities to give us the correct texture format for BuildTexture

extern BITMAPINFO *BMToDIB(Bitmap* newBM, int extraFlags);	
extern BITMAPINFO * ConvertBitmap(Bitmap * bm);

#endif // __LIGHTMAP__H
