/**********************************************************************
 *<
	FILE: shadersMain.h

	DESCRIPTION:

	CREATED BY: Kells Elmquist

	HISTORY:

 *>	Copyright (c) 1999, All Rights Reserved.
 **********************************************************************/

#ifndef __SHADERS_MAIN__H
#define __SHADERS_MAIN__H
#include "buildver.h"

// moved to mtls dll to guarantee a default shader
//extern ClassDesc* GetConstantShaderCD();
//extern ClassDesc* GetPhongShaderCD();
//extern ClassDesc* GetBlinnShaderCD();
//extern ClassDesc* GetOldBlinnShaderCD();
//extern ClassDesc* GetMetalShaderCD();

extern ClassDesc* GetOrenNayarShaderCD();
extern ClassDesc* GetOrenNayarBlinnShaderCD();
extern ClassDesc* GetWardShaderCD();
extern ClassDesc* GetAnisoShaderCD();
extern ClassDesc* GetLaFortuneShaderCD();
extern ClassDesc* GetLayeredBlinnShaderCD();
extern ClassDesc* GetPhysicalBlinnShaderCD();
extern ClassDesc* GetCookShaderCD();
extern ClassDesc* GetStraussShaderCD();

#ifndef NO_MTL_COMPOSITE // orb 01-07-2001
extern ClassDesc* GetCompositeMatDesc();
#endif // NO_MTL_COMPOSITE

extern ClassDesc * GetSchlickShaderCD();
extern ClassDesc * GetMultiLayerShaderCD();
extern ClassDesc * GetTranslucentShaderCD();

TCHAR *GetString(int id);

#endif
