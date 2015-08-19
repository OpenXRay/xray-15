#include "stdafx.h"
#pragma hdrstop

#include "../xrRender/ResourceManager.h"

#include "glRenderDeviceRender.h"

CRT::CRT			()
{
	pSurface		= NULL;
	pRT				= NULL;
	dwWidth			= 0;
	dwHeight		= 0;
	fmt				= 0;
}
CRT::~CRT			()
{
	destroy			();

	// release external reference
	DEV->_DeleteRT	(this);
}

void CRT::create	(LPCSTR Name, u32 w, u32 h,	GLenum f, u32 SampleCount )
{
	VERIFY(!"CRT::create not implemented");
}

void CRT::destroy		()
{
	VERIFY(!"CRT::destroy not implemented");
}
void CRT::reset_begin	()
{
	destroy		();
}
void CRT::reset_end		()
{
	create		(*cName,dwWidth,dwHeight,fmt);
}
void resptrcode_crt::create(LPCSTR Name, u32 w, u32 h, GLenum f, u32 SampleCount)
{
	_set			(DEV->_CreateRT(Name,w,h,f));
}
