/**********************************************************************
 *<
	FILE: kernelhdr.h

	DESCRIPTION: Hearder for Prefilter Kernels DLL

	CREATED BY: Kells Elmquist

	HISTORY:

 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/

#ifndef __KERNELHDR__H
#define __KERNELHDR__H

#ifdef BLD_KERNEL
#define KernelExport __declspec( dllexport )
#else
#define KernelExport __declspec( dllimport )
#endif

#include "max.h"
#include "render.h"
#include "stdKernels.h"

extern ClassDesc* GetSampKernelDesc();




TCHAR *GetString(int id);

#endif
