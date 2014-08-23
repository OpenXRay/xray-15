//***************************************************************************
// CJRender - [maxincl.h] Sample Plugin Renderer for 3D Studio MAX.
// 
// By Christer Janson - Kinetix
//
// May     11, 1996	CCJ Initial coding
// October 28, 1996	CCJ Texture mapping, raycasting etc.
// December    1996	CCJ General cleanup
//
// Description:
// Here we include the Max SDK headers for use with precompiled headers
//
//***************************************************************************

#include "Max.h"
#include "bmmlib.h"
#include "meshadj.h"
#include "modstack.h"
#include "stdmat.h"
#include "templt.h"
#include "render.h"
#include <float.h>		// Include these guys, otherwise sqrt() doesn't work!
#include <math.h>
#include <notify.h>


// Prototype for a utility function used for string table resources
TCHAR *GetString(int id);
