/*----------------------------------------------------------------------*
 |
 |	FILE: uvremove.h
 |	AUTH: Harry Denholm, Kinetix
 |		  Copyright (c) 1998, All Rights Reserved.
 |
 *----------------------------------------------------------------------*/

#ifndef __UVREMOVE__H
#define __UVREMOVE__H

#include "Max.h"

#include "resource.h"
#include "utilapi.h"

TCHAR *GetString(int id);

extern ClassDesc* GetUVStripDesc();

extern HINSTANCE hInstance;

#endif
