
#ifndef __MTLHDR__H
#define __MTLHDR__H

#ifdef BLD_MTL
#define MtlExport __declspec( dllexport )
#else
#define MtlExport __declspec( dllimport )
#endif

#include "max.h"
#include "imtl.h"
#include "texutil.h"
#include "resource.h"
#include <bmmlib.h>

extern ClassDesc* GetSwirlDesc();

TCHAR *GetString(int id);

#endif
