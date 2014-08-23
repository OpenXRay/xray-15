/**********************************************************************
 *<
	FILE: Colliders.h

	DESCRIPTION:	Template Utility

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#ifndef __COLLIDERS__H
#define __COLLIDERS__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"

extern ClassDesc* GetCollisionPlaneDesc();
extern ClassDesc* GetCollisionSphereDesc();
extern ClassDesc* GetCollisionMeshDesc();

extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

#endif // __COLLIDERS__H
