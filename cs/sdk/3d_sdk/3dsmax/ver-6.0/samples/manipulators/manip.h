/**********************************************************************
 *<
	FILE: manip.h

	DESCRIPTION:

	CREATED BY: 

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#ifndef __MANIP__H
#define __MANIP__H

#include "Max.h"
#include "resource.h"

TCHAR *GetString(int id);

extern ClassDesc* GetConeAngleDesc();
extern ClassDesc* GetHotsotManipDesc();
extern ClassDesc* GetFalloffManipDesc();

extern ClassDesc* GetPlaneAngleDesc();
extern ClassDesc* GetIKSwivelManipDesc();
extern ClassDesc* GetReactorAngleManipDesc();
extern ClassDesc* GetSliderManipDesc();
extern ClassDesc* GetPositionManipDesc();
extern ClassDesc* GetSphericalManipDesc();
extern ClassDesc* GetReactorPosValueManipDesc();
extern ClassDesc* GetIKStartSpTwistManipDesc();
extern ClassDesc* GetIKEndSpTwistManipDesc();
extern HINSTANCE hInstance;


#endif  //__MANIP__H

