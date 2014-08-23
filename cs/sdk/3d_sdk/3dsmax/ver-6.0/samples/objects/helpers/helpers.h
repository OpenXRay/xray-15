/**********************************************************************
 *<
	FILE: helpers.h

	DESCRIPTION: Helper object header file

	CREATED BY: Tom Hudson

	HISTORY: Created 31 January 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#ifndef __HELPERS__H
#define __HELPERS__H

#include "Max.h"
#include "resource.h"

TCHAR *GetString(int id);

#define LINEHELP_CLASS_ID 0x02012


extern ClassDesc* GetGridHelpDesc();
extern ClassDesc* GetTapeHelpDesc();
extern ClassDesc* GetLineHelpDesc();
extern ClassDesc* GetPointHelpDesc();
extern ClassDesc* GetVIZGridHelpDesc();

#endif // __HELPERS__H
