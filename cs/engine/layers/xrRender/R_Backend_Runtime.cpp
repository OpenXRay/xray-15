#include "stdafx.h"
#pragma hdrstop

#include "../../xrEngine/frustum.h"

#ifdef	USE_DX10
#include "../xrRenderDX10/StateManager/dx10StateManager.h"
#include "../xrRenderDX10/StateManager/dx10ShaderResourceStateCache.h"
#endif	USE_DX10

void CBackend::OnFrameEnd	()
{
//#ifndef DEDICATED_SERVER
#ifndef _EDITOR
	if (!g_dedicated_server)
#endif    
	{
#if defined(USE_DX10) || defined(USE_OGL)
#ifndef USE_OGL
		HW.pDevice->ClearState();
#endif // !USE_OGL
		Invalidate			();
#else	//	USE_DX10 || USE_OGL

		for (u32 stage=0; stage<HW.Caps.raster.dwStages; stage++)
			CHK_DX(HW.pDevice->SetTexture(0,0));
		CHK_DX				(HW.pDevice->SetStreamSource	(0,0,0,0));
		CHK_DX				(HW.pDevice->SetIndices			(0));
		CHK_DX				(HW.pDevice->SetVertexShader	(0));
		CHK_DX				(HW.pDevice->SetPixelShader		(0));
		Invalidate			();
#endif	//	USE_DX10 || USE_OGL
	}
//#endif
}

void CBackend::OnFrameBegin	()
{
//#ifndef DEDICATED_SERVER
#ifndef _EDITOR
	if (!g_dedicated_server)
#endif    
	{
		PGO					(Msg("PGO:*****frame[%d]*****",Device.dwFrame));
#ifdef	USE_DX10
		Invalidate();
		//	DX9 sets base rt nd base zb by default
		RImplementation.rmNormal();
		set_RT				(HW.pBaseRT);
		set_ZB				(HW.pBaseZB);
#endif	//	USE_DX10
		Memory.mem_fill		(&stat,0,sizeof(stat));
		Vertex.Flush		();
		Index.Flush			();
		set_Stencil			(FALSE);
	}
//#endif
}

void CBackend::Invalidate	()
{
	pRT[0]						= NULL;
	pRT[1]						= NULL;
	pRT[2]						= NULL;
	pRT[3]						= NULL;
	pZB							= NULL;

	decl						= NULL;
	vb							= NULL;
	ib							= NULL;
	vb_stride					= 0;

#ifndef USE_OGL
	state						= NULL;
#endif // !USE_OGL
	ps							= NULL;
	vs							= NULL;
#ifdef USE_DX10
	gs							= NULL;
#endif // USE_DX10
	ctable						= NULL;

	T							= NULL;
	M							= NULL;
	C							= NULL;

	stencil_enable=u32(-1);
	stencil_func=u32(-1);
	stencil_ref=u32(-1);
	stencil_mask=u32(-1);
	stencil_writemask=u32(-1);
	stencil_fail=u32(-1);
	stencil_pass=u32(-1);
	stencil_zfail=u32(-1);
	cull_mode=u32(-1);
	z_enable=u32(-1);
	z_func=u32(-1);
	alpha_ref=u32(-1);
	colorwrite_mask				= u32(-1);

	//	Since constant buffers are unmapped (for DirecX 10)
	//	transform setting handlers should be unmapped too.
	xforms.unmap	();

#ifdef	USE_DX10
	m_pInputLayout				= NULL;
	m_PrimitiveTopology			= D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED;
	m_bChangedRTorZB			= false;
	m_pInputSignature			= NULL;
	for (int i=0; i<MaxCBuffers; ++i)
	{
		m_aPixelConstants[i] = 0;
		m_aVertexConstants[i] = 0;
		m_aGeometryConstants[i] = 0;
	}
	StateManager.Reset();
	//	Redundant call. Just no note that we need to unmap const
	//	if we create dedicated class.
	StateManager.UnmapConstants();
	SSManager.ResetDeviceState();
	SRVSManager.ResetDeviceState();

	for (u32 gs_it =0; gs_it < CTexture::mtMaxGeometryShaderTextures;)	textures_gs	[gs_it++]	= 0;
#endif	//	USE_DX10

	for (u32 ps_it =0; ps_it < CTexture::mtMaxPixelShaderTextures;)	textures_ps	[ps_it++]	= 0;
	for (u32 vs_it =0; vs_it < CTexture::mtMaxVertexShaderTextures;)	textures_vs	[vs_it++]	= 0;
#ifdef _EDITOR
	for (u32 m_it =0; m_it< 8;)		matrices	[m_it++]	= 0;
#endif
}

void	CBackend::set_ClipPlanes	(u32 _enable, Fplane*	_planes /*=NULL */, u32 count/* =0*/)
{
#if defined(USE_DX10) || defined(USE_OGL)
	//	TODO: DX10: Implement in the corresponding vertex shaders
	//	Use this to set up location, were shader setup code will get data
	//VERIFY(!"CBackend::set_ClipPlanes not implemented!");
	return;
#else	//	USE_DX10 || USE_OGL
	if (0==HW.Caps.geometry.dwClipPlanes)	return;
	if (!_enable)	{
		CHK_DX	(HW.pDevice->SetRenderState(D3DRS_CLIPPLANEENABLE,FALSE));
		return;
	}

	// Enable and setup planes
	VERIFY	(_planes && count);
	if		(count>HW.Caps.geometry.dwClipPlanes)	count=HW.Caps.geometry.dwClipPlanes;

	D3DXMATRIX			worldToClipMatrixIT;
	D3DXMatrixInverse	(&worldToClipMatrixIT,NULL,(D3DXMATRIX*)&Device.mFullTransform);
	D3DXMatrixTranspose	(&worldToClipMatrixIT,&worldToClipMatrixIT);
	for		(u32 it=0; it<count; it++)		{
		Fplane&		P			= _planes	[it];
		D3DXPLANE	planeWorld	(-P.n.x,-P.n.y,-P.n.z,-P.d), planeClip;
		D3DXPlaneNormalize		(&planeWorld,	&planeWorld);
		D3DXPlaneTransform		(&planeClip,	&planeWorld, &worldToClipMatrixIT);
		CHK_DX					(HW.pDevice->SetClipPlane(it,planeClip));
	}

	// Enable them
	u32		e_mask	= (1<<count)-1;
	CHK_DX	(HW.pDevice->SetRenderState(D3DRS_CLIPPLANEENABLE,e_mask));
#endif	//	USE_DX10 || USE_OGL
}

#ifndef DEDICATED_SREVER
void	CBackend::set_ClipPlanes	(u32 _enable, Fmatrix*	_xform  /*=NULL */, u32 fmask/* =0xff */)
{
#ifndef USE_OGL
	if (0==HW.Caps.geometry.dwClipPlanes)	return;
#endif // !USE_OGL
	if (!_enable)	{
#if defined(USE_DX10) || defined(USE_OGL)
		//	TODO: DX10: Implement in the corresponding vertex shaders
		//	Use this to set up location, were shader setup code will get data
		//VERIFY(!"CBackend::set_ClipPlanes not implemented!");
#else	//	USE_DX10 || USE_OGL
		CHK_DX	(HW.pDevice->SetRenderState(D3DRS_CLIPPLANEENABLE,FALSE));
#endif	//	USE_DX10 || USE_OGL
		return;
	}
	VERIFY		(_xform && fmask);
	CFrustum	F;
	F.CreateFromMatrix	(*_xform,fmask);
	set_ClipPlanes		(_enable,F.planes,F.p_count);
}

void CBackend::set_Textures			(STextureList* _T)
{
	if (T == _T)	return;
	T				= _T;
	//	If resources weren't set at all we should clear from resource #0.
	int _last_ps	= -1;
	int _last_vs	= -1;
#ifdef	USE_DX10
	int _last_gs	= -1;
#endif	//	USE_DX10
	STextureList::iterator	_it		= _T->begin	();
	STextureList::iterator	_end	= _T->end	();

	for (; _it!=_end; _it++)
	{
		std::pair<u32,ref_texture>&		loader	=	*_it;
		u32			load_id		= loader.first		;
		CTexture*	load_surf	= &*loader.second	;
//		if (load_id < 256)		{
		if (load_id < CTexture::rstVertex)
		{
			//	Set up pixel shader resources
			VERIFY(load_id<CTexture::mtMaxPixelShaderTextures);
			// ordinary pixel surface
			if ((int)load_id>_last_ps)		_last_ps	=	load_id;
			if (textures_ps[load_id]!=load_surf)	
			{
				textures_ps[load_id]	= load_surf			;
#ifdef DEBUG
				stat.textures			++;
#endif
				if (load_surf)			
				{
					PGO					(Msg("PGO:tex%d:%s",load_id,load_surf->cName.c_str()));
					load_surf->bind		(load_id);
//					load_surf->Apply	(load_id);
				}
			}
		} else 
#ifdef	USE_DX10
		if (load_id < CTexture::rstGeometry)
#endif	//	USE_DX10
		{
			//	Set up pixel shader resources
			VERIFY(load_id < CTexture::rstVertex+CTexture::mtMaxVertexShaderTextures);

			// vertex only //d-map or vertex	
			u32		load_id_remapped	= load_id - CTexture::rstVertex;
			if ((int)load_id_remapped>_last_vs)	_last_vs	=	load_id_remapped;
			if (textures_vs[load_id_remapped]!=load_surf)	
			{
				textures_vs[load_id_remapped]	= load_surf;
#ifdef DEBUG
				stat.textures	++;
#endif
				if (load_surf)
				{
					PGO					(Msg("PGO:tex%d:%s",load_id,load_surf->cName.c_str()));
					load_surf->bind		(load_id);
//					load_surf->Apply	(load_id);
				}
			}
		}
#ifdef	USE_DX10
		else
		{
			//	Set up pixel shader resources
			VERIFY(load_id < CTexture::rstGeometry + CTexture::mtMaxGeometryShaderTextures);

			// vertex only //d-map or vertex	
			u32		load_id_remapped = load_id - CTexture::rstGeometry;
			if ((int)load_id_remapped>_last_gs)	_last_gs = load_id_remapped;
			if (textures_gs[load_id_remapped] != load_surf)
			{
				textures_gs[load_id_remapped] = load_surf;
#ifdef DEBUG
				stat.textures++;
#endif
				if (load_surf)
				{
					PGO(Msg("PGO:tex%d:%s", load_id, load_surf->cName.c_str()));
					load_surf->bind(load_id);
					//					load_surf->Apply	(load_id);
				}
			}
		}
#endif	//	USE_DX10
	}

	// TODO: OGL: Do we actually need to clear the remaining stages?
	// clear remaining stages (PS)
	for (++_last_ps; _last_ps<CTexture::mtMaxPixelShaderTextures; _last_ps++)
	{
		if (!textures_ps[_last_ps])
			continue;

		textures_ps[_last_ps]			= 0;
#ifdef	USE_DX10
		//	TODO: DX10: Optimise: set all resources at once
		ID3D10ShaderResourceView	*pRes = 0;
		//HW.pDevice->PSSetShaderResources(_last_ps, 1, &pRes);
		SRVSManager.SetPSResource(_last_ps, pRes);
#else	//	USE_DX10
#ifdef	USE_OGL
		CHK_GL							(glActiveTexture(GL_TEXTURE0 + _last_ps));
		CHK_GL							(glBindTexture(GL_TEXTURE_2D, 0));
		CHK_GL							(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
#else
		CHK_DX							(HW.pDevice->SetTexture(_last_ps,NULL));
#endif	//	USE_OGL
#endif	//	USE_DX10
	}
	// clear remaining stages (VS)
	for (++_last_vs; _last_vs<CTexture::mtMaxVertexShaderTextures; _last_vs++)
	{
		if (!textures_vs[_last_vs])
			continue;

		textures_vs[_last_vs]			= 0;
#ifdef	USE_DX10
		//	TODO: DX10: Optimise: set all resources at once
		ID3D10ShaderResourceView	*pRes = 0;
		//HW.pDevice->VSSetShaderResources(_last_vs, 1, &pRes);
		SRVSManager.SetVSResource(_last_vs, pRes);
#else	//	USE_DX10
#ifdef	USE_OGL
		CHK_GL							(glActiveTexture(GL_TEXTURE0 + CTexture::rstVertex + _last_vs));
		CHK_GL							(glBindTexture(GL_TEXTURE_2D, 0));
		CHK_GL							(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
#else
		CHK_DX							(HW.pDevice->SetTexture(_last_vs+CTexture::rstVertex,NULL));
#endif	//	USE_OGL
#endif	//	USE_DX10
	}

#ifdef	USE_DX10
	// clear remaining stages (VS)
	for (++_last_gs; _last_gs<CTexture::mtMaxGeometryShaderTextures; _last_gs++)
	{
		if (!textures_gs[_last_gs])
			continue;

		textures_gs[_last_gs]			= 0;

#ifdef	USE_OGL
		CHK_GL							(glActiveTexture(GL_TEXTURE20 + _last_gs));
		CHK_GL							(glBindTexture(GL_TEXTURE_2D, 0));
		CHK_GL							(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
#else
		//	TODO: DX10: Optimise: set all resources at once
		ID3D10ShaderResourceView	*pRes = 0;
		//HW.pDevice->GSSetShaderResources(_last_gs, 1, &pRes);
		SRVSManager.SetGSResource(_last_gs, pRes);
#endif	//	USE_OGL
	}
#endif	//	USE_DX10
}
#else

void	CBackend::set_ClipPlanes	(u32 _enable, Fmatrix*	_xform  /*=NULL */, u32 fmask/* =0xff */) {}
void	CBackend::set_Textures			(STextureList* _T) {}

#endif