#include "stdafx.h"
#pragma hdrstop

#include "../xrRender/ResourceManager.h"

#include "glRenderDeviceRender.h"

#include "glTextureUtils.h"

CRT::CRT			()
{
	pSurface		= NULL;
	dwWidth			= 0;
	dwHeight		= 0;
	fmt				= D3DFMT_UNKNOWN;
}
CRT::~CRT			()
{
	destroy			();

	// release external reference
	DEV->_DeleteRT	(this);
}

void CRT::create	(LPCSTR Name, u32 w, u32 h,	D3DFORMAT f, u32 SampleCount )
{
	if (pSurface)	return;

	R_ASSERT(Name && Name[0] && w && h);
	_order = CPU::GetCLK();	//Device.GetTimerGlobal()->GetElapsed_clk();

	//HRESULT		_hr;

	dwWidth		= w;
	dwHeight	= h;
	fmt			= f;

	// Get caps
	GLint max_width, max_height;
	CHK_GL(glGetIntegerv(GL_MAX_FRAMEBUFFER_WIDTH, &max_width));
	CHK_GL(glGetIntegerv(GL_MAX_FRAMEBUFFER_HEIGHT, &max_height));

	// Check width-and-height of render target surface
	if (w>max_width)		return;
	if (h>max_height)		return;

	// Select usage
	GLenum usage	= 0;
	if (D3DFMT_D24X8			==fmt)						usage = GL_DEPTH24_STENCIL8;
	else if (D3DFMT_D24S8		==fmt)						usage = GL_DEPTH24_STENCIL8;
	else if (D3DFMT_D15S1		==fmt)						usage = GL_DEPTH24_STENCIL8;
	else if (D3DFMT_D16			==fmt)						usage = GL_DEPTH_COMPONENT16;
	else if (D3DFMT_D16_LOCKABLE==fmt)						usage = GL_DEPTH_COMPONENT16;
	else if (D3DFMT_D32F_LOCKABLE==fmt)						usage = GL_DEPTH_COMPONENT32F;
	else if ((D3DFORMAT)MAKEFOURCC('D','F','2','4') == fmt)	usage = GL_DEPTH_COMPONENT24;
	else													usage = glTextureUtils::ConvertTextureFormat(fmt);

	DEV->Evict();

	glGenTextures(1, &pSurface);
	CHK_GL(glBindTexture(GL_TEXTURE_2D, pSurface));
	CHK_GL(glTextureStorage2D(GL_TEXTURE_2D, 1, usage, w, h));

	pTexture	= DEV->_CreateTexture	(Name);
	pTexture->surface_set(pSurface);
}

void CRT::destroy		()
{
	if (pTexture._get())	{
		pTexture->surface_set(0);
		pTexture = NULL;
	}
	CHK_GL(glDeleteTextures(1, &pSurface));
}
void CRT::reset_begin	()
{
	destroy		();
}
void CRT::reset_end		()
{
	create		(*cName,dwWidth,dwHeight,fmt);
}
void resptrcode_crt::create(LPCSTR Name, u32 w, u32 h, D3DFORMAT f, u32 SampleCount)
{
	_set			(DEV->_CreateRT(Name,w,h,f));
}
