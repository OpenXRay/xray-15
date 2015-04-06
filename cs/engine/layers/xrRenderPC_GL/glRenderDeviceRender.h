#pragma once

#include "..\..\Include\xrRender\RenderDeviceRender.h"

class glRenderDeviceRender :
	public IRenderDeviceRender
{
public:
	glRenderDeviceRender();

	virtual void	Copy(IRenderDeviceRender &_in) { };

	//	Gamma correction functions
	virtual void	setGamma(float fGamma) { };
	virtual void	setBrightness(float fGamma) { };
	virtual void	setContrast(float fGamma) { };
	virtual void	updateGamma() { };

	//	Destroy
	virtual void	OnDeviceDestroy(BOOL bKeepTextures) { };
	virtual void	ValidateHW() { };
	virtual void	DestroyHW() { };
	virtual void	Reset(HWND hWnd, u32 &dwWidth, u32 &dwHeight, float &fWidth_2, float &fHeight_2) { };
	//	Init
	virtual void	SetupStates() { };
	virtual void	OnDeviceCreate(LPCSTR shName) { };
	virtual bool	Create(HWND hWnd, u32 &dwWidth, u32 &dwHeight, float &fWidth_2, float &fHeight_2, bool move_window);
	virtual void	SetupGPU(BOOL bForceGPU_SW, BOOL bForceGPU_NonPure, BOOL bForceGPU_REF) { };
	//	Overdraw
	virtual void	overdrawBegin() { };
	virtual void	overdrawEnd() { };

	//	Resources control
	virtual void	DeferredLoad(BOOL E) { };
	virtual void	ResourcesDeferredUpload() { };
	virtual void	ResourcesGetMemoryUsage(u32& m_base, u32& c_base, u32& m_lmaps, u32& c_lmaps) { };
	virtual void	ResourcesDestroyNecessaryTextures() { };
	virtual void	ResourcesStoreNecessaryTextures() { };
	virtual void	ResourcesDumpMemoryUsage() { };

	//	HWSupport
	virtual bool	HWSupportsShaderYUV2RGB() { };

	//	Device state
	virtual DeviceState GetDeviceState() { return dsOK; };
	virtual BOOL	GetForceGPU_REF() { return false; };
	virtual u32		GetCacheStatPolys() { return 0; };
	virtual void	Begin() { };
	virtual void	Clear() { };
	virtual void	End() { };
	virtual void	ClearTarget() { };
	virtual void	SetCacheXform(Fmatrix &mView, Fmatrix &mProject) { };
	virtual void	OnAssetsChanged() { };

private:
	HDC				m_hDC;
	HGLRC			m_hRC;
};
