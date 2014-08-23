/**********************************************************************
 *<
	FILE: $$root$$.h

	DESCRIPTION:	Includes for Plugins

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#ifndef __$$ROOT$$__H
#define __$$ROOT$$__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
$$//
$$IF(SIMPLE_TYPE)
$$//
$$IF(PROCEDURAL_OBJECT_TYPE)
#include "Simpobj.h"
$$ELIF(MODIFIER_TYPE)
#include "Simpmod.h"
#include "Simpobj.h"
$$ENDIF
$$//
$$ENDIF //SIMPLE_TYPE

$$IF(SPACE_WARP_TYPE)
#include "simpmod.h"
#include "simpobj.h"
$$ENDIF
$$IF(COLPICK_TYPE)
#include "hsv.h"
$$ELIF(FILE_IMPORT_TYPE)
#include <direct.h>
#include <commdlg.h>
$$ELIF(GUP_TYPE)
#include <guplib.h>
$$ELIF(FRONT_END_CONTROLLER_TYPE) 
#include "frontend.h"
$$ELIF(IMAGE_FILTER_COMPOSITOR_TYPE)
#include "tvnode.h"
#include "bmmlib.h"
#include "fltlib.h"
$$ELIF(MODIFIER_TYPE)
#include "meshadj.h"
#include "XTCObject.h"
$$ELIF(SAMPLER_TYPE)
#include "samplers.h"
$$ELIF(SHADER_TYPE)
#include "texutil.h"
#include "shaders.h"
#include "macrorec.h"
#include "gport.h"
$$ELIF(SHADOW_TYPE)
#include "shadgen.h"
$$ELIF(TEX_TYPE)
#include "stdmat.h"
#include "imtl.h"
#include "macrorec.h"
$$ELIF(TRACK_VIEW_UTILITY_TYPE)
#include "tvutil.h"
$$ELIF(UTILITY_TYPE)
#include "utilapi.h"
$$ELIF(IK_TYPE)
#include "IKSolver.h"
$$ELIF(SKIN_GIZMO_TYPE)
#include "ISkin.h"
#include "ISkinCodes.h"
#include "icurvctl.h"
$$ELIF(MANIP_TYPE)
#include "Manipulator.h"
$$ELIF(ATMOSPHERIC_TYPE)
#include "gizmo.h"
#include "gizmoimp.h"
$$ELIF(EXTENSION)
#include "XTCObject.h"
$$ENDIF


extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

#endif // __$$ROOT$$__H
