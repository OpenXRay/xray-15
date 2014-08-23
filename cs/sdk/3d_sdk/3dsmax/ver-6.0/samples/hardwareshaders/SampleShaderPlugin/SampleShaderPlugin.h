/**********************************************************************
 *<
	FILE: SampleShaderPlugin.h

	DESCRIPTION:	Includes for Plugins

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#ifndef __SAMPLESHADERPLUGIN__H
#define __SAMPLESHADERPLUGIN__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"

#include "stdmat.h"
#include "imtl.h"
#include "macrorec.h"

#include "IDX8PixelShader.h"
#include "IDX8VertexShader.h"
#include "ID3DGraphicsWindow.h"
#include "IStdDualVS.h"

#define SAFE_DELETE(p)			{ if (p) { delete (p);		(p)=NULL; } }
#define SAFE_DELETE_ARRAY(p)	{ if (p) { delete[] (p);	(p)=NULL; } }
#define SAFE_RELEASE(p)			{ if (p) { (p)->Release();	(p)=NULL; } }

extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

#endif // __SAMPLESHADERPLUGIN__H
