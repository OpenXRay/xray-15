/**********************************************************************
 *<
	FILE: IHardwareMaterial.h

	DESCRIPTION: Hardware Material Extension Interface class

	CREATED BY: Norbert Jeske

	HISTORY:

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#ifndef _IHARDWARE_MATERIAL_H_
#define _IHARDWARE_MATERIAL_H_

#define IHARDWARE_MATERIAL_INTERFACE_ID Interface_ID(0x40c926b7, 0x7b3a66b7)

class IHardwareMaterial : public BaseInterface
{
public:
	virtual Interface_ID	GetID() { return IHARDWARE_MATERIAL_INTERFACE_ID; }

	// Interface Lifetime
	virtual LifetimeType	LifetimeControl() { return noRelease; }

	// Information provided from a Custom Attribute?
	virtual void	SetCustomFlag(bool bVal) = 0;

	// Render States
	virtual void	SetFillMode(DWORD mode) = 0;
	virtual void	SetShadeMode(DWORD mode) = 0;

	// Material Colors
	virtual void	SetMaterial(LPD3DXMATERIAL pMtl) = 0;

	virtual void	SetDiffuseColor(LPD3DXCOLOR pClr) = 0;
	virtual void	SetDiffuseColor(Color c, float alpha = 1.0f) = 0;
	virtual void	SetDiffuseColor(Point3 c, float alpha = 1.0f) = 0;

	virtual void	SetAmbientColor(LPD3DXCOLOR pClr) = 0;
	virtual void	SetAmbientColor(Color c, float alpha = 1.0f) = 0;
	virtual void	SetAmbientColor(Point3 c, float alpha = 1.0f) = 0;

	virtual void	SetSpecularColor(LPD3DXCOLOR pClr) = 0;
	virtual void	SetSpecularColor(Color c, float alpha = 1.0f) = 0;
	virtual void	SetSpecularColor(Point3 c, float alpha = 1.0f) = 0;

	virtual void	SetEmissiveColor(LPD3DXCOLOR pClr) = 0;
	virtual void	SetEmissiveColor(Color c, float alpha = 1.0f) = 0;
	virtual void	SetEmissiveColor(Point3 c, float alpha = 1.0f) = 0;

	virtual void	SetSpecularPower(float power) = 0;

	// Texture States
	virtual bool	SetNumTexStages(DWORD numStages) = 0;
	virtual bool	SetTexture(DWORD stage, DWORD_PTR pTexture) = 0;
	virtual bool	SetTexture(DWORD stage, LPCSTR filename) = 0;
	virtual bool	SetTextureUVWSource(DWORD stage, DWORD type) = 0;
	virtual bool	SetTextureMapChannel(DWORD stage, DWORD numChan) = 0;
	virtual bool	SetTextureCoordIndex(DWORD stage, DWORD index) = 0;

	// Texture Transforms
	virtual bool	SetTextureTransformFlag(DWORD stage, DWORD flag) = 0;
	virtual bool	SetTextureTransform(DWORD stage, LPD3DXMATRIX pTransform) = 0;

	// Texture Stage States
	virtual bool	SetTextureColorOp(DWORD stage, DWORD colorOp) = 0;
	virtual bool	SetTextureColorArg(DWORD stage, DWORD argNum, DWORD colorArg) = 0;
	virtual bool	SetTextureAlphaOp(DWORD stage, DWORD alphaArg) = 0;
	virtual bool	SetTextureAlphaArg(DWORD stage, DWORD argNum, DWORD alphaArg) = 0;
	virtual bool	SetTextureAddressMode(DWORD stage, DWORD coordNum, DWORD mode) = 0;

	// Shaders
	virtual bool	SetVertexShader(DWORD_PTR pVertexShader) = 0;
	virtual bool	SetPixelShader(DWORD_PTR pPixelShader) = 0;

	// Effects
	virtual bool	SetEffect(DWORD_PTR pEffect) = 0;

	// User Plugin
	virtual bool	SetPlugin(BaseInterface *pPlugin) = 0;

	// Current Associated INode
	virtual bool	SetINode(INode *pINode) = 0;
};

#endif
