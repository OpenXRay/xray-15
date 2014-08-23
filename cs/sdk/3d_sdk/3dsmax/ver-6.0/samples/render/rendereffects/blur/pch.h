/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: pch.h

	 DESCRIPTION: precompiled header

	 CREATED BY: michael malone (mjm)

	 HISTORY: created March 17, 1999

   	 Copyright (c) 1998, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */


#if !defined(_PCH_H_INCLUDED_)
#define _PCH_H_INCLUDED_

// system includes
#include <math.h>
#include <assert.h>

// maxsdk includes
#include <max.h>
#include <iparamm2.h>
#include <iparamb2.h>
#include <bmmlib.h>
#include <icurvctl.h>

// local includes
#include "resource.h"
#include "dllMain.h"
#include "blurMgr.h"
#include "dlgProcs.h"
#include ".\_selectTypes\selImage.h"
#include ".\_selectTypes\selIgnBack.h"
#include ".\_selectTypes\selLum.h"
#include ".\_selectTypes\selMaps.h"
#include ".\_selectTypes\selObjIds.h"
#include ".\_selectTypes\selMatIds.h"
#include ".\_selectTypes\sctex.h"
#include ".\_blurTypes\blurBase.h"
#include ".\_blurTypes\blurUniform.h"
#include ".\_blurTypes\blurDirectional.h"
#include ".\_blurTypes\blurRadial.h"


#endif // !defined(_PCH_H_INCLUDED_)

