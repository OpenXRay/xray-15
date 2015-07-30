#pragma once

#include "..\..\Include\xrRender\RenderDeviceRender.h"

#ifndef _EDITOR
	#define DEV glRenderDeviceRender::Instance().Resources
#else
	#define DEV Device.Resources
#endif

class CResourceManager;

class glRenderDeviceRender :
	public IRenderDeviceRender
{
public:
	static glRenderDeviceRender& Instance() { return *((glRenderDeviceRender*)(&*Device.m_pRender)); }

	glRenderDeviceRender();

	virtual void	Copy(IRenderDeviceRender &_in) { VERIFY(!"glRenderDeviceRender::Copy not implemented."); };

	//	Gamma correction functions
	virtual void	setGamma(float fGamma) { VERIFY(!"glRenderDeviceRender::setGamma not implemented."); };
	virtual void	setBrightness(float fGamma) { VERIFY(!"glRenderDeviceRender::setBrightness not implemented."); };
	virtual void	setContrast(float fGamma) { VERIFY(!"glRenderDeviceRender::setContrast not implemented."); };
	virtual void	updateGamma() { VERIFY(!"glRenderDeviceRender::updateGamma not implemented."); };

	//	Destroy
	virtual void	OnDeviceDestroy(BOOL bKeepTextures) { VERIFY(!"glRenderDeviceRender::OnDeviceDestroy not implemented."); };
	virtual void	ValidateHW() { VERIFY(!"glRenderDeviceRender::ValidateHW not implemented."); };
	virtual void	DestroyHW();
	virtual void	Reset(HWND hWnd, u32 &dwWidth, u32 &dwHeight, float &fWidth_2, float &fHeight_2) { VERIFY(!"glRenderDeviceRender::Reset not implemented."); };
	//	Init
	virtual void	SetupStates() { VERIFY(!"glRenderDeviceRender::SetupStates not implemented."); };
	virtual void	OnDeviceCreate(LPCSTR shName) { VERIFY(!"glRenderDeviceRender::OnDeviceCreate not implemented."); };
	virtual bool	Create(HWND hWnd, u32 &dwWidth, u32 &dwHeight, float &fWidth_2, float &fHeight_2, bool move_window);
	virtual void	SetupGPU(BOOL bForceGPU_SW, BOOL bForceGPU_NonPure, BOOL bForceGPU_REF) { VERIFY(!"glRenderDeviceRender::SetupGPU not implemented."); };
	//	Overdraw
	virtual void	overdrawBegin() { VERIFY(!"glRenderDeviceRender::overdrawBegin not implemented."); };
	virtual void	overdrawEnd() { VERIFY(!"glRenderDeviceRender::overdrawEnd not implemented."); };

	//	Resources control
	virtual void	DeferredLoad(BOOL E) { VERIFY(!"glRenderDeviceRender::DeferredLoad not implemented."); };
	virtual void	ResourcesDeferredUpload() { VERIFY(!"glRenderDeviceRender::ResourcesDeferredUpload not implemented."); };
	virtual void	ResourcesGetMemoryUsage(u32& m_base, u32& c_base, u32& m_lmaps, u32& c_lmaps) { VERIFY(!"glRenderDeviceRender::ResourcesGetMemoryUsage not implemented."); };
	virtual void	ResourcesDestroyNecessaryTextures() { VERIFY(!"glRenderDeviceRender::ResourcesDestroyNecessaryTextures not implemented."); };
	virtual void	ResourcesStoreNecessaryTextures() { VERIFY(!"glRenderDeviceRender::ResourcesStoreNecessaryTextures not implemented."); };
	virtual void	ResourcesDumpMemoryUsage() { VERIFY(!"glRenderDeviceRender::ResourcesDumpMemoryUsage not implemented."); };

	//	HWSupport
	virtual bool	HWSupportsShaderYUV2RGB() { VERIFY(!"glRenderDeviceRender::HWSupportsShaderYUV2RGB not implemented."); return false; };

	//	Device state
	virtual DeviceState GetDeviceState() { VERIFY(!"glRenderDeviceRender::GetDeviceState not implemented."); return dsOK; };
	virtual BOOL	GetForceGPU_REF() { VERIFY(!"glRenderDeviceRender::GetForceGPU_REF not implemented."); return false; };
	virtual u32		GetCacheStatPolys() { VERIFY(!"glRenderDeviceRender::GetCacheStatPolys not implemented."); return 0; };
	virtual void	Begin() { VERIFY(!"glRenderDeviceRender::Begin not implemented."); };
	virtual void	Clear() { VERIFY(!"glRenderDeviceRender::Clear not implemented."); };
	virtual void	End() { VERIFY(!"glRenderDeviceRender::End not implemented."); };
	virtual void	ClearTarget() { VERIFY(!"glRenderDeviceRender::ClearTarget not implemented."); };
	virtual void	SetCacheXform(Fmatrix &mView, Fmatrix &mProject) { VERIFY(!"glRenderDeviceRender::SetCacheXform not implemented."); };
	virtual void	OnAssetsChanged() { VERIFY(!"glRenderDeviceRender::OnAssetsChanged not implemented."); };

public:
	CResourceManager*	Resources;

private:
	HWND			m_hWnd;
	HDC				m_hDC;
	HGLRC			m_hRC;
};
