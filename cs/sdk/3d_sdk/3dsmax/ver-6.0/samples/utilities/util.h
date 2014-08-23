/**********************************************************************
 *<
	FILE: util.h

	DESCRIPTION:

	CREATED BY: Rolf Berteig

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#ifndef __UTIL__H
#define __UTIL__H

#include "Max.h"
#include "resource.h"

TCHAR *GetString(int id);

#define NO_UTILITY_ASCIIOUTPUT	// for Luna, 020617  --prs.

extern ClassDesc* GetColorClipDesc();
#ifndef NO_UTILITY_ASCIIOUTPUT	// for Luna, 020617  --prs.
extern ClassDesc* GetAsciiOutDesc();
#endif	// NO_UTILITY_ASCIIOUTPUT
extern ClassDesc* GetUtilTestDesc();
extern ClassDesc* GetAppDataTestDesc();
extern ClassDesc* GetTestSoundObjDescriptor();
extern ClassDesc* GetCollapseUtilDesc();
extern ClassDesc* GetRandKeysDesc();
extern ClassDesc* GetORTKeysDesc();
extern ClassDesc* GetSelKeysDesc();
extern ClassDesc* GetRefObjUtilDesc();
extern ClassDesc* GetLinkInfoUtilDesc();
extern ClassDesc* GetCellTexDesc();
/*extern ClassDesc* GetPipeMakerDesc();*/	//RK: 07/02/99 Removing this from Shiva
extern ClassDesc* GetRescaleDesc();
extern ClassDesc* GetShapeCheckDesc();
extern ClassDesc* GetSetKeyUtilDesc();

extern HINSTANCE hInstance;

#endif
