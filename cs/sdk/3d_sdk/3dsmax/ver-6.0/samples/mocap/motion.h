/**********************************************************************
 *<
	FILE: motion.h

	DESCRIPTION:

	CREATED BY: Rolf Berteig

	HISTORY:

 *>	Copyright (c) 1999, All Rights Reserved.
 **********************************************************************/

#ifndef __MOTION__H
#define __MOTION__H

#include "Max.h"
#include "utilapi.h"
#include "resource.h"


#define MOTION_MANAGER_CLASS_ID		0x76ef52a6

extern ClassDesc* GetPosMotionDesc();
extern ClassDesc* GetRotMotionDesc();
extern ClassDesc* GetScaleMotionDesc();
extern ClassDesc* GetFloatMotionDesc();
extern ClassDesc* GetPoint3MotionDesc();
extern ClassDesc* GetMotionManDesc();
extern ClassDesc* GetMouseDeviceClassDescDesc();
extern ClassDesc* GetMidiDeviceClassDescDesc();
extern ClassDesc* GetJoyDeviceClassDescDesc();
extern ClassDesc* GetMouseDeviceClassDescDescOld();
extern ClassDesc* GetMidiDeviceClassDescDescOld();
extern ClassDesc* GetJoyDeviceClassDescDescOld();
extern ClassDesc* GetTheJoyDeviceClassDescDesc();
extern ClassDesc* GetTheMouseDeviceClassDescDesc();
extern ClassDesc* GetTheMidiDeviceClassDescDesc();

TCHAR *GetString(int id);
extern HINSTANCE hInstance;

#endif

