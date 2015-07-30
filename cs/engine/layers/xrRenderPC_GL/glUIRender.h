#pragma once

#include "..\..\Include\xrRender\UIRender.h"

class glUIRender :
	public IUIRender
{
public:
	glUIRender();
	~glUIRender();

	virtual void CreateUIGeom() { VERIFY(!"glUIRender::CreateUIGeom not implemented."); };
	virtual void DestroyUIGeom() { VERIFY(!"glUIRender::DestroyUIGeom not implemented."); };

	virtual void SetShader(IUIShader &shader) { VERIFY(!"glUIRender::SetShader not implemented."); };
	virtual void SetAlphaRef(int aref) { VERIFY(!"glUIRender::SetAlphaRef not implemented."); };
	//.	virtual void StartTriList(u32 iMaxVerts) { };
	//.	virtual void FlushTriList() { };
	//.	virtual void StartTriFan(u32 iMaxVerts) { };
	//.	virtual void FlushTriFan() { };

	//virtual void StartTriStrip(u32 iMaxVerts) { };
	//virtual void FlushTriStrip() { };
	//.	virtual void StartLineStrip(u32 iMaxVerts) { };
	//.	virtual void FlushLineStrip() { };
	//.	virtual void StartLineList(u32 iMaxVerts) { };
	//.	virtual void FlushLineList() { };
	virtual void SetScissor(Irect* rect = NULL) { VERIFY(!"glUIRender::SetScissor not implemented."); };
	virtual void GetActiveTextureResolution(Fvector2 &res) { VERIFY(!"glUIRender::GetActiveTextureResolution not implemented."); };

	//.	virtual void PushPoint(float x, float y, u32 c, float u, float v) { };
	//.	virtual void PushPoint(int x, int y, u32 c, float u, float v) { };
	virtual void PushPoint(float x, float y, float z, u32 C, float u, float v) { VERIFY(!"glUIRender::PushPoint not implemented."); };

	virtual void StartPrimitive(u32 iMaxVerts, ePrimitiveType primType, ePointType pointType) { VERIFY(!"glUIRender::StartPrimitive not implemented."); };
	virtual void FlushPrimitive() { VERIFY(!"glUIRender::FlushPrimitive not implemented."); };

	virtual LPCSTR	UpdateShaderName(LPCSTR tex_name, LPCSTR sh_name) { VERIFY(!"glUIRender::UpdateShaderName not implemented."); return ""; };

	virtual void	CacheSetXformWorld(const Fmatrix& M) { VERIFY(!"glUIRender::CacheSetXformWorld not implemented."); };
	virtual void	CacheSetCullMode(CullMode) { VERIFY(!"glUIRender::CacheSetCullMode not implemented."); };
};

extern glUIRender	UIRenderImpl;
