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
	glActiveTexture(dwStage);
	if (!flags.bLoaded)		Load			()	;
	else					PostLoad		()	;
	bind					(dwStage)			;
};

void CTexture::apply_theora(u32 dwStage)	{
	CHK_GL(glBindTexture(GL_TEXTURE_2D, pSurface));

	if (pTheora->Update(m_play_time!=0xFFFFFFFF?m_play_time:Device.dwTimeContinual)) {
		u32 width	= pTheora->Width(true);
		u32 height	= pTheora->Height(true);
		u32* pBits	= xr_alloc<u32>(pTheora->Width(false)*pTheora->Height(false) * 4);

		int _pos = 0;
		pTheora->DecompressFrame(pBits, pTheora->Width(false) - width, _pos);
		CHK_GL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
			GL_RGBA, GL_UNSIGNED_BYTE, pBits));
	}
};
void CTexture::apply_avi(u32 dwStage)	{
	CHK_GL(glBindTexture(GL_TEXTURE_2D, pSurface));

	if (pAVI->NeedUpdate())		{
		// AVI
		BYTE* ptr; pAVI->GetFrame(&ptr);
		CHK_GL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pAVI->m_dwWidth, pAVI->m_dwHeight,
			GL_RGBA, GL_UNSIGNED_BYTE, ptr));
	}
};
void CTexture::apply_seq(u32 dwStage)	{
	CHK_GL(glBindTexture(GL_TEXTURE_2D, pSurface));

	// SEQ
	u32	frame		= Device.dwTimeContinual/seqMSPF; //Device.dwTimeGlobal
	u32	frame_data	= seqDATA.size();
	if (flags.seqCycles)		{
		u32	frame_id	= frame%(frame_data*2);
		if (frame_id>=frame_data)	frame_id = (frame_data-1) - (frame_id%frame_data);
		pSurface 			= seqDATA[frame_id];
	} else {
		u32	frame_id	= frame%frame_data;
		pSurface 			= seqDATA[frame_id];
	}
};
void CTexture::apply_normal	(u32 dwStage)	{
	CHK_GL(glBindTexture(GL_TEXTURE_2D, pSurface));
};

void CTexture::Preload	()
{
	m_bumpmap = DEV->m_textures_description.GetBumpName(cName);
	m_material = DEV->m_textures_description.GetMaterial(cName);
}

void CTexture::Load		()
{
	flags.bLoaded = true;
	desc_cache = 0;
	if (pSurface)					return;

	flags.bUser = false;
	flags.MemoryUsage = 0;
	if (0 == stricmp(*cName, "$null"))	return;
	if (0 != strstr(*cName, "$user$"))	{
		flags.bUser = true;
		return;
	}

	Preload();

	bool	bCreateView = true;

	// Check for OGM
	string_path			fn;
	if (FS.exist(fn, "$game_textures$", *cName, ".ogm")) {
		// AVI
		pTheora = new CTheoraSurface();
		m_play_time = 0xFFFFFFFF;

		if (!pTheora->Load(fn)) {
			xr_delete(pTheora);
			FATAL("Can't open video stream");
		}
		else {
			flags.MemoryUsage = pTheora->Width(true)*pTheora->Height(true) * 4;
			pTheora->Play(TRUE, Device.dwTimeContinual);

			// Now create texture
			GLuint	pTexture = 0;
			u32 _w = pTheora->Width(false);
			u32 _h = pTheora->Height(false);

			glGenTextures(1, &pTexture);
			glBindTexture(GL_TEXTURE_2D, pTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _w, _h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

			pSurface = pTexture;
			if (glGetError() != GL_NO_ERROR)
			{
				FATAL("Invalid video stream");
				xr_delete(pTheora);
				pSurface = 0;
			}
		}
	}
	else if (FS.exist(fn, "$game_textures$", *cName, ".avi")) {
		// AVI
		pAVI = new CAviPlayerCustom();

		if (!pAVI->Load(fn)) {
			xr_delete(pAVI);
			FATAL("Can't open video stream");
		}
		else {
			flags.MemoryUsage = pAVI->m_dwWidth*pAVI->m_dwHeight * 4;

			// Now create texture
			GLuint	pTexture = 0;
			glGenTextures(1, &pTexture);
			glBindTexture(GL_TEXTURE_2D, pTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pAVI->m_dwWidth, pAVI->m_dwHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

			pSurface = pTexture;
			if (glGetError() != GL_NO_ERROR)
			{
				FATAL("Invalid video stream");
				xr_delete(pAVI);
				pSurface = 0;
			}
		}
	}
	else if (FS.exist(fn, "$game_textures$", *cName, ".seq")) {
		// Sequence
		string256 buffer;
		IReader* _fs = FS.r_open(fn);

		flags.seqCycles = FALSE;
		_fs->r_string(buffer, sizeof(buffer));
		if (0 == stricmp(buffer, "cycled"))
		{
			flags.seqCycles = TRUE;
			_fs->r_string(buffer, sizeof(buffer));
		}
		u32 fps = atoi(buffer);
		seqMSPF = 1000 / fps;

		while (!_fs->eof())
		{
			_fs->r_string(buffer, sizeof(buffer));
			_Trim(buffer);
			if (buffer[0])
			{
				// Load another texture
				u32	mem = 0;
				pSurface = ::RImplementation.texture_load(buffer, mem);
				if (pSurface)
				{
					// pSurface->SetPriority	(PRIORITY_LOW);
					seqDATA.push_back(pSurface);
					flags.MemoryUsage += mem;
				}
			}
		}
		pSurface = 0;
		FS.r_close(_fs);
	}
	else {
		// Normal texture
		u32	mem = 0;
		pSurface = ::RImplementation.texture_load(*cName, mem);

		// Calc memory usage and preload into vid-mem
		if (pSurface) {
			// pSurface->SetPriority	(PRIORITY_NORMAL);
			flags.MemoryUsage = mem;
		}
	}

	PostLoad();
}

void CTexture::Unload	()
{
#ifdef DEBUG
	string_path				msg_buff;
	sprintf_s(msg_buff, sizeof(msg_buff), "* Unloading texture [%s] pSurface ID=%d", cName.c_str(), pSurface);
#endif // DEBUG

	//.	if (flags.bLoaded)		Msg		("* Unloaded: %s",cName.c_str());

	flags.bLoaded = FALSE;
	if (!seqDATA.empty())	{
		glDeleteTextures(seqDATA.size(), seqDATA.data());
		seqDATA.clear();
		pSurface = 0;
	}

	glDeleteTextures(1, &pSurface);

	xr_delete(pAVI);
	xr_delete(pTheora);

	bind = fastdelegate::FastDelegate1<u32>(this, &CTexture::apply_load);
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
