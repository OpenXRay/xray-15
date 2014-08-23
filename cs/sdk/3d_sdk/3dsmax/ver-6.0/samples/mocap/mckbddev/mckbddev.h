/**********************************************************************
 *<
	FILE: mckbddev.h

	DESCRIPTION:

	CREATED BY: Christer Janson

	HISTORY:

 *>	Copyright (c) 1999, All Rights Reserved.
 **********************************************************************/

#ifndef __MCKBDDEV__H
#define __MCKBDDEV__H

#include "Max.h"
#include "utilapi.h"
#include "resource.h"

extern ClassDesc* GetKbdDeviceClassDesc();
extern ClassDesc* GetKbdBindingClassDesc();

TCHAR *GetString(int id);
extern HINSTANCE hInstance;

#endif // __MCKBDDEV__H