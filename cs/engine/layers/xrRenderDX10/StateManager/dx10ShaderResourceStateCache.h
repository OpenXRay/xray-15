#ifndef	dx10ShaderResourceStateCache_included
#define	dx10ShaderResourceStateCache_included
#pragma once

class dx10ShaderResourceStateCache
{
public:
	dx10ShaderResourceStateCache();

	void	ResetDeviceState();

	void	Apply();

	void	SetPSResource( u32 uiSlot, ID3D10ShaderResourceView	*pRes );
	void	SetGSResource( u32 uiSlot, ID3D10ShaderResourceView	*pRes );
	void	SetVSResource( u32 uiSlot, ID3D10ShaderResourceView	*pRes );

private:
	ID3D10ShaderResourceView	*m_PSViews[CBackend::mtMaxPixelShaderTextures];
	ID3D10ShaderResourceView	*m_GSViews[CBackend::mtMaxGeometryShaderTextures];
	ID3D10ShaderResourceView	*m_VSViews[CBackend::mtMaxVertexShaderTextures];

	u32		m_uiMinPSView;
	u32		m_uiMaxPSView;

	u32		m_uiMinGSView;
	u32		m_uiMaxGSView;

	u32		m_uiMinVSView;
	u32		m_uiMaxVSView;

	bool	m_bUpdatePSViews;
	bool	m_bUpdateGSViews;
	bool	m_bUpdateVSViews;
};

extern	dx10ShaderResourceStateCache	SRVSManager;

#endif	//	dx10ShaderResourceStateCache_included