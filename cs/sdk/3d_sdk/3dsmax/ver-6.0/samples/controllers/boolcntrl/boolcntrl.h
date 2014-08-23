/**********************************************************************
 *<
	FILE: boolcntrl.h

	DESCRIPTION:	Includes for Plugins

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#ifndef __BOOLCNTRL__H
#define __BOOLCNTRL__H

#include "Max.h"
#include "resource.h"
#include "iparamb2.h"
#include "iparamm2.h"

#include "control.h"
#include "interpik.h"
#include "hsv.h"
#include "exprlib.h"


// Key flags
#define KEY_SELECTED		IKEY_SELECTED
#define KEY_XSEL			IKEY_XSEL
#define KEY_YSEL			IKEY_YSEL
#define KEY_ZSEL			IKEY_ZSEL
#define KEY_WSEL			IKEY_WSEL
#define KEY_ALLSEL			IKEY_ALLSEL
#define KEY_FLAGGED			IKEY_FLAGGED
#define KEY_TIME_LOCK		IKEY_TIME_LOCK



// Track flags
#define CURVE_SELECTED		(1<<0)
#define RANGE_UNLOCKED		(1<<1)
#define TRACK_LOOPEDIN		(1<<2)
#define TRACK_LOOPEDOUT		(1<<3)


#define F_COLOR			(GetColorManager()->GetColor(kFunctionCurveFloat))


extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;
extern ClassDesc2* GetBoolCntrlDesc();

#endif // __BOOLCNTRL__H
