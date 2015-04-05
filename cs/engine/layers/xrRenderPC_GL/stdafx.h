// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

#pragma warning(disable:4995)
#include "../../xrEngine/stdafx.h"
#pragma warning(default:4995)
#pragma warning(disable:4714)
#pragma warning( 4 : 4018 )
#pragma warning( 4 : 4244 )
#pragma warning(disable:4237)

#include <gl\glew.h>
#include <gl\GL.h>
#include <gl\GLU.h>
#include <gl\glext.h>
#include <gl\wglext.h>

#define		R_R1	1
#define		R_R2	2
#define		R_R3	3
#define		R_GL	4
#define		RENDER	R_GL


#include "../../xrEngine/psystem.h"

#include "../../xrEngine/vis_common.h"
#include "../../xrEngine/render.h"
#include "../../xrEngine/igame_level.h"
//#include "../xrRender/blenders\blender.h"
//#include "../xrRender/blenders\blender_clsid.h"
#include "../xrRender/xrRender_console.h"
#include "GLUtils.h"
#include "r4.h"
