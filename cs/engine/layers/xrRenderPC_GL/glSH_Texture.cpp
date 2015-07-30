#include "stdafx.h"
#pragma hdrstop

#include "../xrRender/ResourceManager.h"

#ifndef _EDITOR
#include "../../xrEngine/render.h"
#endif

#include "../../xrEngine/tntQAVI.h"
#include "../../xrEngine/xrTheora_Surface.h"

#include "glRenderDeviceRender.h"

#define		PRIORITY_HIGH	12
#define		PRIORITY_NORMAL	8
#define		PRIORITY_LOW	4

void resptrcode_texture::create(LPCSTR _name)
{
	_set(DEV->_CreateTexture(_name));
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CTexture::CTexture		()
{
	pSurface			= NULL;
	pAVI				= NULL;
	pTheora				= NULL;
	desc_cache			= 0;
	seqMSPF				= 0;
	flags.MemoryUsage	= 0;
	flags.bLoaded		= false;
	flags.bUser			= false;
	flags.seqCycles		= FALSE;
	m_material			= 1.0f;
	bind				= fastdelegate::FastDelegate1<u32>(this,&CTexture::apply_load);
}

CTexture::~CTexture()
{
	Unload				();

	// release external reference
	DEV->_DeleteTexture	(this);
}

void					CTexture::surface_set	(GLuint surf )
{
	pSurface			= surf;
}

GLuint					CTexture::surface_get	()
{
	return pSurface;
}

void CTexture::PostLoad	()
{
	if (pTheora)				bind		= fastdelegate::FastDelegate1<u32>(this,&CTexture::apply_theora);
	else if (pAVI)				bind		= fastdelegate::FastDelegate1<u32>(this,&CTexture::apply_avi);
	else if (!seqDATA.empty())	bind		= fastdelegate::FastDelegate1<u32>(this,&CTexture::apply_seq);
	else						bind		= fastdelegate::FastDelegate1<u32>(this,&CTexture::apply_normal);
}

void CTexture::apply_load	(u32 dwStage)	{
	if (!flags.bLoaded)		Load			()	;
	else					PostLoad		()	;
	bind					(dwStage)			;
};

void CTexture::apply_theora	(u32 dwStage)	{
	VERIFY(!"CTexture::apply_theora not implemented.");
};
void CTexture::apply_avi	(u32 dwStage)	{
	VERIFY(!"CTexture::apply_avi not implemented.");
};
void CTexture::apply_seq	(u32 dwStage)	{
	VERIFY(!"CTexture::apply_seq not implemented.");
};
void CTexture::apply_normal	(u32 dwStage)	{
	VERIFY(!"CTexture::apply_normal not implemented.");
};

void CTexture::Preload	()
{
	m_bumpmap = DEV->m_textures_description.GetBumpName(cName);
	m_material = DEV->m_textures_description.GetMaterial(cName);
}

void CTexture::Load		()
{
	VERIFY(!"CTexture::Load not implemented.");
}

void CTexture::Unload	()
{
	VERIFY(!"CTexture::Unload not implemented.");
}

void CTexture::desc_update	()
{
	VERIFY(!"CTexture::desc_update not implemented.");
}

void CTexture::video_Play		(BOOL looped, u32 _time)	
{ 
	if (pTheora) pTheora->Play	(looped,(_time!=0xFFFFFFFF)?(m_play_time=_time):Device.dwTimeContinual); 
}

void CTexture::video_Pause		(BOOL state)
{
	if (pTheora) pTheora->Pause	(state); 
}

void CTexture::video_Stop			()				
{ 
	if (pTheora) pTheora->Stop(); 
}

BOOL CTexture::video_IsPlaying	()				
{ 
	return (pTheora)?pTheora->IsPlaying():FALSE; 
}
