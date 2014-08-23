/**********************************************************************
 *<
	FILE: Convert.h

	DESCRIPTION: Convert To type modifiers

	CREATED BY: Steve Anderson

	HISTORY:

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#ifndef __CONVERT__H
#define __CONVERT__H

#include "Max.h"
#include "MNMath.h"
#include "PolyObj.h"
#include "iparamm2.h"
#include "resource.h"
#include "surf_api.h"

#define CONVERT_TO_MESH_ID	Class_ID(0x5c5b50f7,0x60397ca1)
#define CONVERT_TO_POLY_ID	Class_ID(0x2f494e50,0x5c376942)
#define CONVERT_TO_PATCH_ID	Class_ID(0x3c4b70d1,0x232d27c5)

#define REF_PBLOCK 0

TCHAR *GetString(int id);

extern ClassDesc *GetConvertToMeshDesc(), *GetConvertToPolyDesc(),
		*GetConvertToPatchDesc();

extern HINSTANCE hInstance;


#endif
