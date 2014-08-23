#include "stdafx.h"
#include "dx10ShaderResourceStateCache.h"

dx10ShaderResourceStateCache	SRVSManager;

dx10ShaderResourceStateCache::dx10ShaderResourceStateCache()
{
	ResetDeviceState();
}

void dx10ShaderResourceStateCache::ResetDeviceState()
{
	ZeroMemory(m_PSViews, sizeof(m_PSViews));
	ZeroMemory(m_GSViews, sizeof(m_GSViews));
	ZeroMemory(m_VSViews, sizeof(m_VSViews));

	m_uiMinPSView = 0xFFFFFFFF;
	m_uiMaxPSView = 0xFFFFFFFF;

	m_uiMinGSView = 0xFFFFFFFF;
	m_uiMaxGSView = 0xFFFFFFFF;

	m_uiMinVSView = 0xFFFFFFFF;
	m_uiMaxVSView = 0xFFFFFFFF;

	m_bUpdatePSViews = false;
	m_bUpdateGSViews = false;
	m_bUpdateVSViews = false;
}

void dx10ShaderResourceStateCache::Apply()
{
	if (m_bUpdatePSViews)
	{
		HW.pDevice->PSSetShaderResources( m_uiMinPSView, m_uiMaxPSView-m_uiMinPSView+1, &m_PSViews[m_uiMinPSView]);
		m_uiMinPSView = 0xFFFFFFFF;
		m_uiMaxPSView = 0xFFFFFFFF;
		m_bUpdatePSViews = false;
	}

	if (m_bUpdateGSViews)
	{
		HW.pDevice->GSSetShaderResources( m_uiMinGSView, m_uiMaxGSView-m_uiMinGSView+1, &m_GSViews[m_uiMinGSView]);
		m_uiMinGSView = 0xFFFFFFFF;
		m_uiMaxGSView = 0xFFFFFFFF;
		m_bUpdateGSViews = false;
	}

	if (m_bUpdateVSViews)
	{
		HW.pDevice->VSSetShaderResources( m_uiMinVSView, m_uiMaxVSView-m_uiMinVSView+1, &m_VSViews[m_uiMinVSView]);
		m_uiMinVSView = 0xFFFFFFFF;
		m_uiMaxVSView = 0xFFFFFFFF;
		m_bUpdateVSViews = false;
	}
}

void dx10ShaderResourceStateCache::SetPSResource( u32 uiSlot, ID3D10ShaderResourceView	*pRes )
{
	VERIFY(uiSlot<CBackend::mtMaxPixelShaderTextures);

	if ( m_PSViews[uiSlot] != pRes)
	{
		m_PSViews[uiSlot] = pRes;
		if (m_bUpdatePSViews)
		{
			m_uiMinPSView = _min( m_uiMinPSView, uiSlot );
			m_uiMaxPSView = _max( m_uiMaxPSView, uiSlot );
		}
		else
		{
			m_bUpdatePSViews = true;
			m_uiMinPSView = uiSlot;
			m_uiMaxPSView = uiSlot;
		}
	}
}

void dx10ShaderResourceStateCache::SetGSResource( u32 uiSlot, ID3D10ShaderResourceView	*pRes )
{
	VERIFY(uiSlot<CBackend::mtMaxGeometryShaderTextures);

	if ( m_GSViews[uiSlot] != pRes)
	{
		m_GSViews[uiSlot] = pRes;
		if (m_bUpdateGSViews)
		{
			m_uiMinGSView = _min( m_uiMinGSView, uiSlot );
			m_uiMaxGSView = _max( m_uiMaxGSView, uiSlot );
		}
		else
		{
			m_bUpdateGSViews = true;
			m_uiMinGSView = uiSlot;
			m_uiMaxGSView = uiSlot;
		}
	}
}

void dx10ShaderResourceStateCache::SetVSResource( u32 uiSlot, ID3D10ShaderResourceView	*pRes )
{
	VERIFY(uiSlot<CBackend::mtMaxVertexShaderTextures);

	if ( m_VSViews[uiSlot] != pRes)
	{
		m_VSViews[uiSlot] = pRes;
		if (m_bUpdateVSViews)
		{
			m_uiMinVSView = _min( m_uiMinVSView, uiSlot );
			m_uiMaxVSView = _max( m_uiMaxVSView, uiSlot );
		}
		else
		{
			m_bUpdateVSViews = true;
			m_uiMinVSView = uiSlot;
			m_uiMaxVSView = uiSlot;
		}
	}
}