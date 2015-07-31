#ifndef	dx10R_Backend_Runtime_included
#define	dx10R_Backend_Runtime_included
#pragma once

#include "StateManager/dx10StateManager.h"
#include "StateManager/dx10ShaderResourceStateCache.h"

IC void CBackend::set_xform( u32 ID, const Fmatrix& M )
{
	stat.xforms			++;
	//	TODO: DX10: Implement CBackend::set_xform
	//VERIFY(!"Implement CBackend::set_xform");
}

IC void CBackend::set_RT(ID3DRenderTargetView* RT, u32 ID)
{
	if (RT!=pRT[ID])
	{
		PGO(Msg("PGO:setRT"));
		stat.target_rt	++;
		pRT[ID]			= RT;
		//	Mark RT array dirty
		//HW.pDevice->OMSetRenderTargets(sizeof(pRT)/sizeof(pRT[0]), pRT, 0);
		//HW.pDevice->OMSetRenderTargets(sizeof(pRT)/sizeof(pRT[0]), pRT, pZB);
		//	Reset all RT's here to allow RT to be bounded as input
		if (!m_bChangedRTorZB)
			HW.pDevice->OMSetRenderTargets(0, 0, 0);

		m_bChangedRTorZB = true;
	}
}

IC void	CBackend::set_ZB(ID3DDepthStencilView* ZB)
{
	if (ZB!=pZB)
	{
		PGO				(Msg("PGO:setZB"));
		stat.target_zb	++;
		pZB				= ZB;
		//HW.pDevice->OMSetRenderTargets(0, 0, pZB);
		//HW.pDevice->OMSetRenderTargets(sizeof(pRT)/sizeof(pRT[0]), pRT, pZB);
		//	Reset all RT's here to allow RT to be bounded as input
		if (!m_bChangedRTorZB)
			HW.pDevice->OMSetRenderTargets(0, 0, 0);
		m_bChangedRTorZB = true;
	}
}

ICF void CBackend::set_Format(SDeclaration* _decl)
{
	if (decl!=_decl)
	{
		PGO				(Msg("PGO:v_format:%x",_decl));
#ifdef DEBUG
		stat.decl		++;
#endif
		decl			= _decl;
	}
}

ICF void CBackend::set_PS(ID3DPixelShader* _ps, LPCSTR _n)
{
	if (ps!=_ps)
	{
		PGO				(Msg("PGO:Pshader:%x",_ps));
		stat.ps			++;
		ps				= _ps;
		HW.pDevice->PSSetShader(ps);
#ifdef DEBUG
		ps_name			= _n;
#endif
	}
}

ICF void CBackend::set_GS(ID3DGeometryShader* _gs, LPCSTR _n)
{
	if (gs!=_gs)
	{
		PGO				(Msg("PGO:Gshader:%x",_ps));
		//	TODO: DX10: Get statistics for G Shader change
		//stat.gs			++;
		gs				= _gs;
		HW.pDevice->GSSetShader(gs);
#ifdef DEBUG
		gs_name			= _n;
#endif
	}
}

ICF void CBackend::set_VS(ID3DVertexShader* _vs, LPCSTR _n)
{
	if (vs!=_vs)
	{
		PGO				(Msg("PGO:Vshader:%x",_vs));
		stat.vs			++;
		vs				= _vs;
		HW.pDevice->VSSetShader(vs);
#ifdef DEBUG
		vs_name			= _n;
#endif
	}
}

ICF void CBackend::set_Vertices(ID3DVertexBuffer* _vb, u32 _vb_stride)
{
	if ((vb!=_vb) || (vb_stride!=_vb_stride))
	{
		PGO				(Msg("PGO:VB:%x,%d",_vb,_vb_stride));
#ifdef DEBUG
		stat.vb			++;
#endif
		vb				= _vb;
		vb_stride		= _vb_stride;
		//CHK_DX			(HW.pDevice->SetStreamSource(0,vb,0,vb_stride));
		//UINT StreamNumber,
		//IDirect3DVertexBuffer9 * pStreamData,
		//UINT OffsetInBytes,
		//UINT Stride

		//UINT StartSlot,
		//UINT NumBuffers,
		//ID3D10Buffer *const *ppVertexBuffers,
		//const UINT *pStrides,
		//const UINT *pOffsets
		u32	iOffset = 0;
		HW.pDevice->IASetVertexBuffers( 0, 1, &vb, &_vb_stride, &iOffset);
	}
}

ICF void CBackend::set_Indices(ID3DIndexBuffer* _ib)
{
	if (ib!=_ib)
	{
		PGO				(Msg("PGO:IB:%x",_ib));
#ifdef DEBUG
		stat.ib			++;
#endif
		ib				= _ib;
		HW.pDevice->IASetIndexBuffer(ib, DXGI_FORMAT_R16_UINT, 0);
	}
}

IC D3D10_PRIMITIVE_TOPOLOGY TranslateTopology(D3DPRIMITIVETYPE T)
{
	static	D3D10_PRIMITIVE_TOPOLOGY translateTable[] =
	{
		D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED,		//	None
		D3D10_PRIMITIVE_TOPOLOGY_POINTLIST,		//	D3DPT_POINTLIST = 1,
		D3D10_PRIMITIVE_TOPOLOGY_LINELIST,		//	D3DPT_LINELIST = 2,
		D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP,		//	D3DPT_LINESTRIP = 3,
		D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST,	//	D3DPT_TRIANGLELIST = 4,
		D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,	//	D3DPT_TRIANGLESTRIP = 5,
		D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED,		//	D3DPT_TRIANGLEFAN = 6,
	};

	VERIFY(T<sizeof(translateTable)/sizeof(translateTable[0]));
	VERIFY(T>=0);

	D3D10_PRIMITIVE_TOPOLOGY	result = translateTable[T];

	VERIFY( result != D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED );

	return result;
}

IC u32 GetIndexCount(D3DPRIMITIVETYPE T, u32 iPrimitiveCount)
{
	switch (T)
	{
	case D3DPT_POINTLIST:
		return iPrimitiveCount;
	case D3DPT_LINELIST:
		return iPrimitiveCount*2;
	case D3DPT_LINESTRIP:
		return iPrimitiveCount+1;
	case D3DPT_TRIANGLELIST:
		return iPrimitiveCount*3;
	case D3DPT_TRIANGLESTRIP:
		return iPrimitiveCount+2;
	default: NODEFAULT;
#ifdef DEBUG
		return 0;
#endif // #ifdef DEBUG
	}
}

IC void CBackend::ApplyPrimitieTopology( D3D10_PRIMITIVE_TOPOLOGY Topology )
{
	if ( m_PrimitiveTopology != Topology )
	{
		m_PrimitiveTopology = Topology;
		HW.pDevice->IASetPrimitiveTopology(m_PrimitiveTopology);
	}
}

IC void CBackend::Render(D3DPRIMITIVETYPE T, u32 baseV, u32 startV, u32 countV, u32 startI, u32 PC)
{
	//VERIFY(vs);
	//HW.pDevice->VSSetShader(vs);
	//HW.pDevice->GSSetShader(0);

	D3D10_PRIMITIVE_TOPOLOGY Topology = TranslateTopology(T);
	u32	iIndexCount = GetIndexCount(T, PC);

	stat.calls++;
	stat.verts += countV;
	stat.polys += PC;

	ApplyPrimitieTopology(Topology);
	
	//CHK_DX(HW.pDevice->DrawIndexedPrimitive(T,baseV, startV, countV,startI,PC));
	//D3DPRIMITIVETYPE Type,
	//INT BaseVertexIndex,
	//UINT MinIndex,
	//UINT NumVertices,
	//UINT StartIndex,
	//UINT PriResmitiveCount

	//UINT IndexCount,
	//UINT StartIndexLocation,
	//INT BaseVertexLocation
	SRVSManager.Apply();
	ApplyRTandZB();
	ApplyVertexLayout();
	StateManager.Apply();
	//	State manager may alter constants
	constants.flush();
//	Msg("DrawIndexed: Start");
//	Msg("iIndexCount=%d, startI=%d, baseV=%d", iIndexCount, startI, baseV);
	HW.pDevice->DrawIndexed(iIndexCount, startI, baseV);
//	Msg("DrawIndexed: End\n");

	PGO					(Msg("PGO:DIP:%dv/%df",countV,PC));
}

IC void CBackend::Render(D3DPRIMITIVETYPE T, u32 startV, u32 PC)
{
	//	TODO: DX10: Remove triangle fan usage from the engine
	if (T == D3DPT_TRIANGLEFAN)
		return;

	//VERIFY(vs);
	//HW.pDevice->VSSetShader(vs);

	D3D10_PRIMITIVE_TOPOLOGY Topology = TranslateTopology(T);
	u32	iVertexCount = GetIndexCount(T, PC);

	stat.calls++;
	stat.verts += 3*PC;
	stat.polys += PC;

	ApplyPrimitieTopology(Topology);
	SRVSManager.Apply();
	ApplyRTandZB();
	ApplyVertexLayout();
	StateManager.Apply();
	//	State manager may alter constants
	constants.flush();
//	Msg("Draw: Start");
//	Msg("iVertexCount=%d, startV=%d", iVertexCount, startV);
	//CHK_DX				(HW.pDevice->DrawPrimitive(T, startV, PC));
	HW.pDevice->Draw(iVertexCount, startV);
//	Msg("Draw: End\n");
	PGO					(Msg("PGO:DIP:%dv/%df",3*PC,PC));
}

IC void CBackend::set_Geometry(SGeometry* _geom)
{
	set_Format			(&*_geom->dcl);
	
	set_Vertices		(_geom->vb, _geom->vb_stride);
	set_Indices			(_geom->ib);
}

IC void	CBackend::set_Scissor(Irect*	R)
{
	if (R)			
	{
		//CHK_DX		(HW.pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE,TRUE));		
		StateManager.EnableScissoring();
		RECT	*	clip	= (RECT	*)R;
		HW.pDevice->RSSetScissorRects(1, clip);
	} 
	else
	{
		//CHK_DX		(HW.pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE,FALSE));
		StateManager.EnableScissoring(FALSE);
		HW.pDevice->RSSetScissorRects(0, 0);
	}
}

IC void CBackend::set_Stencil(u32 _enable, u32 _func, u32 _ref, u32 _mask, u32 _writemask, u32 _fail, u32 _pass, u32 _zfail)
{
	StateManager.SetStencil(_enable, _func, _ref, _mask, _writemask, _fail, _pass, _zfail);
	// Simple filter
	//if (stencil_enable		!= _enable)		{ stencil_enable=_enable;		CHK_DX(HW.pDevice->SetRenderState	( D3DRS_STENCILENABLE,		_enable				)); }
	//if (!stencil_enable)					return;
	//if (stencil_func		!= _func)		{ stencil_func=_func;			CHK_DX(HW.pDevice->SetRenderState	( D3DRS_STENCILFUNC,		_func				)); }
	//if (stencil_ref			!= _ref)		{ stencil_ref=_ref;				CHK_DX(HW.pDevice->SetRenderState	( D3DRS_STENCILREF,			_ref				)); }
	//if (stencil_mask		!= _mask)		{ stencil_mask=_mask;			CHK_DX(HW.pDevice->SetRenderState	( D3DRS_STENCILMASK,		_mask				)); }
	//if (stencil_writemask	!= _writemask)	{ stencil_writemask=_writemask;	CHK_DX(HW.pDevice->SetRenderState	( D3DRS_STENCILWRITEMASK,	_writemask			)); }
	//if (stencil_fail		!= _fail)		{ stencil_fail=_fail;			CHK_DX(HW.pDevice->SetRenderState	( D3DRS_STENCILFAIL,		_fail				)); }
	//if (stencil_pass		!= _pass)		{ stencil_pass=_pass;			CHK_DX(HW.pDevice->SetRenderState	( D3DRS_STENCILPASS,		_pass				)); }
	//if (stencil_zfail		!= _zfail)		{ stencil_zfail=_zfail;			CHK_DX(HW.pDevice->SetRenderState	( D3DRS_STENCILZFAIL,		_zfail				)); }
}

IC  void CBackend::set_Z(u32 _enable)
{
	StateManager.SetDepthEnable(_enable);
	//if (z_enable != _enable)
	//{ 
	//	z_enable=_enable;
	//	CHK_DX(HW.pDevice->SetRenderState	( D3DRS_ZENABLE, _enable )); 
	//}
}

IC  void CBackend::set_ZFunc(u32 _func)
{
	StateManager.SetDepthFunc(_func);
	//if (z_func!=_func)
	//{
	//	z_func = _func;
	//	CHK_DX(HW.pDevice->SetRenderState( D3DRS_ZFUNC, _func));
	//}
}

IC  void CBackend::set_AlphaRef(u32 _value)
{
	//	TODO: DX10: Implement rasterizer state update to support alpha ref
	VERIFY(!"Not implemented.");
	//if (alpha_ref != _value)
	//{ 
	//	alpha_ref = _value;
	//	CHK_DX(HW.pDevice->SetRenderState(D3DRS_ALPHAREF,_value));
	//}
}

IC void	CBackend::set_ColorWriteEnable(u32 _mask )
{
	StateManager.SetColorWriteEnable(_mask);
	//if (colorwrite_mask		!= _mask)		{ 
	//	colorwrite_mask=_mask;		
	//	CHK_DX(HW.pDevice->SetRenderState	( D3DRS_COLORWRITEENABLE,	_mask	));	
	//	CHK_DX(HW.pDevice->SetRenderState	( D3DRS_COLORWRITEENABLE1,	_mask	));	
	//	CHK_DX(HW.pDevice->SetRenderState	( D3DRS_COLORWRITEENABLE2,	_mask	));	
	//	CHK_DX(HW.pDevice->SetRenderState	( D3DRS_COLORWRITEENABLE3,	_mask	));	
	//}
}
ICF void CBackend::set_CullMode(u32 _mode)
{
	StateManager.SetCullMode(_mode);
	//if (cull_mode		!= _mode)		{ cull_mode = _mode;			CHK_DX(HW.pDevice->SetRenderState	( D3DRS_CULLMODE,			_mode				)); }
}

IC void CBackend::ApplyVertexLayout()
{
	VERIFY(vs);
	VERIFY(decl);
	VERIFY(m_pInputSignature);

	xr_map<ID3DBlob*, ID3D10InputLayout*>::iterator	it;

	it = decl->vs_to_layout.find(m_pInputSignature);

	if (it==decl->vs_to_layout.end())
	{
		ID3D10InputLayout* pLayout;

		CHK_DX(HW.pDevice->CreateInputLayout(
			&decl->dx10_dcl_code[0],
			decl->dx10_dcl_code.size()-1,
			m_pInputSignature->GetBufferPointer(),
			m_pInputSignature->GetBufferSize(),
			&pLayout
			));

		it = decl->vs_to_layout.insert(
			std::pair<ID3DBlob*, ID3D10InputLayout*>(m_pInputSignature, pLayout)).first;
	}

	if ( m_pInputLayout != it->second)
	{
		m_pInputLayout = it->second;
		HW.pDevice->IASetInputLayout(m_pInputLayout);
	}	
}

ICF void CBackend::set_VS(ref_vs& _vs)
{
	m_pInputSignature = _vs->signature->signature;
	set_VS(_vs->vs,_vs->cName.c_str());
}

ICF void CBackend::set_VS(SVS* _vs)
{
	m_pInputSignature = _vs->signature->signature;
	set_VS(_vs->vs,_vs->cName.c_str());
}

IC bool CBackend::CBuffersNeedUpdate( ref_cbuffer buf1[MaxCBuffers], ref_cbuffer buf2[MaxCBuffers], u32 &uiMin, u32 &uiMax)
{
	bool	bRes = false;
	int i=0;
	while ( (i<MaxCBuffers) && (buf1[i]==buf2[i]))
		++i;

	uiMin = i;

	for ( ; i<MaxCBuffers; ++i)
	{
		if (buf1[i]!=buf2[i])
		{
			bRes = true;
			uiMax = i;
		}
	}

	return bRes;
}

IC void CBackend::set_Constants			(R_constant_table* C)
{
	// caching
	if (ctable==C)	return;
	ctable			= C;
	xforms.unmap	();
	hemi.unmap		();
	tree.unmap		();
	StateManager.UnmapConstants();
	if (0==C)		return;

	PGO				(Msg("PGO:c-table"));


	//	Setup constant tables
	{
		ref_cbuffer	aPixelConstants[MaxCBuffers];
		ref_cbuffer	aVertexConstants[MaxCBuffers];
		ref_cbuffer	aGeometryConstants[MaxCBuffers];

		for (int i=0; i<MaxCBuffers; ++i)
		{
			aPixelConstants[i] = m_aPixelConstants[i];
			aVertexConstants[i] = m_aVertexConstants[i];
			aGeometryConstants[i] = m_aGeometryConstants[i];

			m_aPixelConstants[i] = 0;
			m_aVertexConstants[i] = 0;
			m_aGeometryConstants[i] = 0;
		}
		R_constant_table::cb_table::iterator	it	= C->m_CBTable.begin();
		R_constant_table::cb_table::iterator	end	= C->m_CBTable.end	();
		for (; it!=end; ++it)
		{
			//ID3D10Buffer*	pBuffer = (it->second)->GetBuffer();
			u32				uiBufferIndex = it->first; 

			if ( (uiBufferIndex&CB_BufferTypeMask) == CB_BufferPixelShader)
			{
				VERIFY((uiBufferIndex&CB_BufferIndexMask)<MaxCBuffers);
				m_aPixelConstants[uiBufferIndex&CB_BufferIndexMask] = it->second;
			}
			else if ( (uiBufferIndex&CB_BufferTypeMask) == CB_BufferVertexShader)
			{
				VERIFY((uiBufferIndex&CB_BufferIndexMask)<MaxCBuffers);
				m_aVertexConstants[uiBufferIndex&CB_BufferIndexMask] = it->second;
			}
			else //if ( (uiBufferIndex&CB_BufferTypeMask) == CB_BufferGeometryShader)
			{
				VERIFY((uiBufferIndex&CB_BufferIndexMask)<MaxCBuffers);
				m_aGeometryConstants[uiBufferIndex&CB_BufferIndexMask] = it->second;
			}
		}

		ID3D10Buffer*	tempBuffer[MaxCBuffers];

		u32 uiMin;
		u32 uiMax;

		if (CBuffersNeedUpdate(m_aPixelConstants, aPixelConstants, uiMin, uiMax))
		{
			++uiMax;

			for (u32 i=uiMin; i<uiMax; ++i)
			{
				if (m_aPixelConstants[i])
					tempBuffer[i] = m_aPixelConstants[i]->GetBuffer();
				else
					tempBuffer[i] = 0;
			}

			HW.pDevice->PSSetConstantBuffers(uiMin, uiMax-uiMin, &tempBuffer[uiMin]);
		}
		

		if (CBuffersNeedUpdate(m_aVertexConstants, aVertexConstants, uiMin, uiMax))
		{
			++uiMax;

			for (u32 i=uiMin; i<uiMax; ++i)
			{
				if (m_aVertexConstants[i])
					tempBuffer[i] = m_aVertexConstants[i]->GetBuffer();
				else
					tempBuffer[i] = 0;
			}
			HW.pDevice->VSSetConstantBuffers(uiMin, uiMax-uiMin, &tempBuffer[uiMin]);
		}

			
		if (CBuffersNeedUpdate(m_aGeometryConstants, aGeometryConstants, uiMin, uiMax))
		{
			++uiMax;

			for (u32 i=uiMin; i<uiMax; ++i)
			{
				if (m_aGeometryConstants[i])
					tempBuffer[i] = m_aGeometryConstants[i]->GetBuffer();
				else
					tempBuffer[i] = 0;
			}
			HW.pDevice->GSSetConstantBuffers(uiMin, uiMax-uiMin, &tempBuffer[uiMin]);
		}

		/*
		for (int i=0; i<MaxCBuffers; ++i)
		{
			if (m_aPixelConstants[i])
				tempBuffer[i] = m_aPixelConstants[i]->GetBuffer();
			else
				tempBuffer[i] = 0;
		}
		HW.pDevice->PSSetConstantBuffers(0, MaxCBuffers, tempBuffer);

		for (int i=0; i<MaxCBuffers; ++i)
		{
			if (m_aVertexConstants[i])
				tempBuffer[i] = m_aVertexConstants[i]->GetBuffer();
			else
				tempBuffer[i] = 0;
		}
		HW.pDevice->VSSetConstantBuffers(0, MaxCBuffers, tempBuffer);

		for (int i=0; i<MaxCBuffers; ++i)
		{
			if (m_aGeometryConstants[i])
				tempBuffer[i] = m_aGeometryConstants[i]->GetBuffer();
			else
				tempBuffer[i] = 0;
		}
		HW.pDevice->GSSetConstantBuffers(0, MaxCBuffers, tempBuffer);
		*/
	}

	// process constant-loaders
	R_constant_table::c_table::iterator	it	= C->table.begin();
	R_constant_table::c_table::iterator	end	= C->table.end	();
	for (; it!=end; it++)	
	{
		R_constant*		Cs	= &**it;
		if (Cs->handler)	Cs->handler->setup(Cs);
	}
}

ICF void CBackend::ApplyRTandZB()
{
	if (m_bChangedRTorZB)
	{
		m_bChangedRTorZB = false;
		HW.pDevice->OMSetRenderTargets(sizeof(pRT)/sizeof(pRT[0]), pRT, pZB);
	}
}

IC	void CBackend::get_ConstantDirect(shared_str& n, u32 DataSize, void** pVData, void** pGData, void** pPData)
{
	ref_constant C = get_c(n);

	if (C)
		constants.access_direct(&*C, DataSize, pVData, pGData, pPData);
	else
	{
		if (pVData)	*pVData = 0;
		if (pGData)	*pGData = 0;
		if (pPData)	*pPData = 0;
	}
}

#endif	//	dx10R_Backend_Runtime_included