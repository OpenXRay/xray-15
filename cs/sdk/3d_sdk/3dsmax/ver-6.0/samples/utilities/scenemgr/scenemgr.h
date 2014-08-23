/*----------------------------------------------------------------------*
 |
 |	FILE: scenemgr.h
 |	AUTH: Harry Denholm, Kinetix
 |		  Copyright (c) 1998, All Rights Reserved.
 |
 *----------------------------------------------------------------------*/

#ifndef __SCENEMGR__H
#define __SCENEMGR__H

#include "Max.h"

#include "resource.h"
#include "utilapi.h"
#include "stdmat.h"
#include "bmmlib.h"
#include "cjapiext.h"

TCHAR *GetString(int id);

extern ClassDesc* GetSManagerDesc();

extern HINSTANCE hInstance;

#endif
