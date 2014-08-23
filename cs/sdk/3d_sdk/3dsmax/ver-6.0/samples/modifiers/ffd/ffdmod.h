/**********************************************************************
 *<
	FILE: ffdmod.h

	DESCRIPTION:

	CREATED BY: Rolf Berteig

	HISTORY: created 7/22/96

 *>	Copyright (c) 1996 Rolf Berteig, All Rights Reserved.
 **********************************************************************/

#ifndef __INFERNO__H
#define __INFERNO__H

#include "Max.h"
#include "resource.h"

extern ClassDesc* GetFFDDesc44();
extern ClassDesc* GetFFDDesc33();
extern ClassDesc* GetFFDDesc22();
extern ClassDesc* GetFFDNMSquareOSDesc();
extern ClassDesc* GetFFDNMSquareWSDesc();
extern ClassDesc* GetFFDNMSquareWSModDesc();
extern ClassDesc* GetFFDNMCylOSDesc();
extern ClassDesc* GetFFDNMCylWSDesc();
extern ClassDesc* GetFFDSelModDesc();

extern HINSTANCE hInstance;

TCHAR *GetString(int id);

#endif
