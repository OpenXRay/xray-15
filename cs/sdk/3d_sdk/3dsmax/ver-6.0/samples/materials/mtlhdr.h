/**********************************************************************
 *<
	FILE: mtlhdr.h

	DESCRIPTION:

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#ifndef __MTLHDR__H
#define __MTLHDR__H

#define USE_STDMTL2_AS_STDMTL

#ifdef BLD_MTL
#define MtlExport __declspec( dllexport )
#else
#define MtlExport __declspec( dllimport )
#endif

#include "max.h"
#include "imtl.h"
#include "texutil.h"
#include "buildver.h"

extern ClassDesc* GetStdMtlDesc();
extern ClassDesc* GetStdMtl2Desc();
extern ClassDesc* GetBMTexDesc();
extern ClassDesc* GetTexmapsDesc();
extern ClassDesc* GetTexmaps2Desc();
extern ClassDesc* GetOldTexmapsDesc();
extern ClassDesc* GetOldTexmaps2Desc();

#ifndef NO_MTL_TOPBOTTOM
extern ClassDesc* GetCMtlDesc();
#endif // NO_MTL_TOPBOTTOM

extern ClassDesc* GetCheckerDesc();
extern ClassDesc* GetMixDesc();
extern ClassDesc* GetMarbleDesc();
extern ClassDesc* GetMaskDesc();

#ifndef NO_MAPTYPE_RGBTINT
extern ClassDesc* GetTintDesc();
#endif //NO_MAPTYPE_RGBTINT

extern ClassDesc* GetNoiseDesc();
extern ClassDesc* GetMultiDesc();
extern ClassDesc* GetDoubleSidedDesc();
extern ClassDesc* GetMixMatDesc();

#ifndef NO_MAPTYPE_REFLECTREFRACT
extern ClassDesc* GetACubicDesc();
#endif // NO_MAPTYPE_REFLECTREFRACT

#ifndef NO_MAPTYPE_FLATMIRROR
extern ClassDesc* GetMirrorDesc();
#endif // NO_MAPTYPE_FLATMIRROR

#ifndef NO_MAPTYPE_GRADIENT
extern ClassDesc* GetGradientDesc();
#endif // NO_MAPTYPE_GRADIENT

extern ClassDesc* GetCompositeDesc();

#ifndef NO_MTL_MATTESHADOW
extern ClassDesc* GetMatteDesc();
#endif // NO_MTL_MATTESHADOW

#ifndef NO_MAPTYPE_RGBMULT
extern ClassDesc* GetRGBMultDesc();
#endif // NO_MAPTYPE_RGBMULT

#ifndef NO_MAPTYPE_OUTPUT
extern ClassDesc* GetOutputDesc();
#endif // NO_MAPTYPE_OUTPUT

extern ClassDesc* GetFalloffDesc();

#ifndef NO_MAPTYPE_VERTCOLOR
extern ClassDesc* GetVColDesc();
#endif // NO_MAPTYPE_VERTCOLOR

#ifndef NO_MAPTYPE_THINWALL
extern ClassDesc* GetPlateDesc();
#endif //NO_MAPTYPE_THINWALL

#ifndef NO_PARTICLES // orb 07-11-01

#ifndef NO_MAPTYPE_PARTICLEAGE
extern ClassDesc* GetPartAgeDesc();
#endif // NO_MAPTYPE_PARTICLEAGE

#ifndef NO_MAPTYPE_PARTICLEMBLUR
extern ClassDesc* GetPartBlurDesc();
#endif // NO_MAPTYPE_PARTICLEMBLUR

#endif // NO_PARTICLES

#ifndef NO_MTL_COMPOSITE
extern ClassDesc* GetCompositeMatDesc();
#endif // NO_MTL_COMPOSITE

extern ClassDesc* GetCellTexDesc();

// old shaders are here, mostly to guarantee the existance of the default shader
extern ClassDesc* GetConstantShaderCD();
#ifndef USE_LIMITED_STDMTL 
extern ClassDesc* GetPhongShaderCD();
extern ClassDesc* GetMetalShaderCD();
#endif

extern ClassDesc* GetBlinnShaderCD();
extern ClassDesc* GetOldBlinnShaderCD();
extern ClassDesc* GetBakeShellDesc();

TCHAR *GetString(int id);

#endif
