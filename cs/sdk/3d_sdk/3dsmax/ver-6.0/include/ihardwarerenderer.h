/**********************************************************************
 *<
	FILE: IHardwareRenderer.h

	DESCRIPTION: Hardware Renderer Extension Interface class

	CREATED BY: Norbert Jeske

	HISTORY:

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#ifndef _HARDWARE_RENDERER_H_
#define _HARDWARE_RENDERER_H_

#define HARDWARE_RENDERER_INTERFACE_ID Interface_ID(0x7f78405c, 0x43894b27)

class IHardwareMesh;
class IHardwareMaterial;
class HardwareMaterial;

class IHardwareRenderer : public IHardwareShader
{
public:
	virtual Interface_ID	GetID() { return HARDWARE_RENDERER_INTERFACE_ID; }

	// Interface Lifetime
	virtual LifetimeType	LifetimeControl() { return noRelease; }

	// Device Information
	virtual int		GetMaxTexStages() = 0;

	// Active Viewport Windows
	virtual int		GetNumActiveWindows() = 0;

	// Screen Update
	virtual bool	UpdateScreen(GFX_ESCAPE_FN fn) = 0;

	// Depth Peeling for Transparency
	virtual bool	CanDepthPeel() = 0;
	virtual void	CreateLayer(int layerNum) = 0;
	virtual void	BlendLayers() = 0;

	// Start and End of Drawing a Node
	virtual void	StartNode() = 0;
	virtual void	EndNode(GFX_ESCAPE_FN fn) = 0;

	// Material Settings
	virtual void	SetColor(ColorType t, D3DXCOLOR *pClr) = 0;
	virtual void	SetMaterial(HardwareMaterial *pMtl) = 0;

	virtual D3DXCOLOR	*GetDiffuseColor() = 0;

	// Display All Triangle Edges?
	virtual bool	DisplayAllTriangleEdges() = 0;

	// Cached Mesh Data
	virtual bool	CanCacheMeshData() = 0;

	// Draw a 3ds max Mesh
	//
	//	type = D3DPRIMITIVETYPE type of primitive
	//	nPrim = number of primitives
	//	pAttr = array of length nPrim indicating which primitives share
	//		attributes, e.g. color, material, effect
	//	nComp = number of separate vertex data arrays
	//	pUse = array of length nComp indicating D3DDECLUSAGE of each data array
	//	ppInd = array of nComp arrays each the same length and long enough
	//		to handle nPrim, the connectivity data for the IndexBuffer
	//	fComp = bit flags for number of float components in each data array,
	//		e.g. 0x00 = 4 floats, 0x01 = 1 float, 0x02 = 2 floats, and
	//		0x03 = 3 floats, enough space for 16 flags
	//	pLen = array of nComp values giving lengths of data arrays
	//	ppData = array of nComp float arrays with data, each being 1X, 2X, 3X,
	//		or 4X the length specified in pLen depending on the fComp bit flag
	//	attr = which attribute set to draw

	virtual void	DrawMesh(DWORD type, DWORD nPrim, DWORD *pAttr,
		DWORD nComp, DWORD *pUse, DWORD **ppInd, DWORD fComp, DWORD *pLen,
		float **ppData,	DWORD attr, GFX_ESCAPE_FN fn) = 0;

	// Build a D3DXMesh from a 3ds max Mesh
	//
	//	Returns a pointer to a GFX D3DXMesh class that wraps the D3DXMesh
	//	object

	virtual DWORD_PTR	BuildMesh(DWORD nFaces, DWORD *pAttr, DWORD nComp,
		DWORD *pUse, DWORD **ppInd, DWORD fComp, DWORD *pLen, float **ppData,
		GFX_ESCAPE_FN fn) = 0;

	virtual DWORD_PTR	BuildMesh(IHardwareMesh *pHWMesh,
		GFX_ESCAPE_FN fn) = 0;

	// Draw a D3DXMesh or part of a D3DXMesh
	//
	//	pMesh = pointer to a GFX D3DMesh class
	//	attr = attribute value to match with primitive attributes

	virtual void	DrawMesh(DWORD_PTR pMesh, DWORD attr,
		GFX_ESCAPE_FN fn) = 0;

	// Load a D3DXMesh from a .X file
	//
	//	Returns a pointer to a GFX D3DXMesh class that wraps the D3DXMesh
	//	object

	virtual DWORD_PTR	LoadMesh(LPCSTR filename) = 0;

	// Save a D3DXMesh to a .X file
	//
	//	pMesh = pointer to a GFX D3DMesh class

	virtual void	SaveMesh(LPCSTR filename, DWORD_PTR pMesh) = 0;

	// Free a D3DXMesh
	//
	//	pMesh = pointer to a GFX D3DMesh class

	virtual void	FreeMesh(DWORD_PTR pMesh) = 0;

	// Build a GFX IndexedBuffer, a VertexBuffer and IndexBuffer pair
	//
	//	Returns a pointer to a GFX D3DIndexedBuffer class

	virtual DWORD_PTR	BuildIndexedBuffer(DWORD type, DWORD nPrim,
		DWORD *pAttr, DWORD nComp, DWORD *pUse, DWORD **ppInd, DWORD fComp,
		DWORD *pLen, float **ppData, GFX_ESCAPE_FN fn) = 0;

	// Draw an IndexedBuffer or part of an IndexedBuffer
	virtual void	DrawIndexedBuffer(DWORD_PTR pBuffer, DWORD attr,
		GFX_ESCAPE_FN fn) = 0;

	// Free an IndexedBuffer
	virtual void	FreeIndexedBuffer(DWORD_PTR pBuffer) = 0;

	// Build a D3DTexture from a 3ds max texture
	//
	//	bmi = 3ds max bitmap texture
	//	mipLevels = number of MipMap levels to build
	//	usage = type, e.g. Texture, RenderTarget, DepthStencil
	//	format = pixel format, e.g. A8R8G8B8
	//
	//	Returns a pointer to a GFX D3DTexture class

	virtual DWORD_PTR	BuildTexture(BITMAPINFO *bmi, UINT mipLevels,
		DWORD usage, DWORD format) = 0;

	// Load a D3DTexture from a file, includes CubeMaps
	//
	//	Returns a pointer to a GFX D3DTexture class

	virtual DWORD_PTR	LoadTexture(LPCSTR filename) = 0;

	// Save a D3DTexture to a file, includes CubeMaps
	virtual void	SaveTexture(LPCSTR filename, DWORD_PTR pTex) = 0;

	// Free a D3DTexture, includes CubeMaps
	virtual void	FreeTexture(DWORD_PTR pTex) = 0;

	// Build a D3DCubeMap from a set of D3DTextures
	//
	//	size = edge size, e.g. width and height
	//	mipLevels = number of MipMap levels to build
	//	usage = type, e.g. Texture, RenderTarget, DepthStencil
	//	format = pixel format, e.g. A8R8G8B8
	//	pTex = array of six D3DTextures to use for the faces
	//
	//	Returns a pointer to a GFX D3DCubeMap class

	virtual DWORD_PTR	BuildCubeMap(UINT size, UINT mipLevels, DWORD usage,
		DWORD format, DWORD_PTR *pTex) = 0;

	// Set a face of a D3DCubeMap
	//
	//	pCubeMap = pointer to a GFX D3DCubeMap class
	//	face = face to set, e.g. positive X
	//	mipLevel = number of MipMap levels to build
	//	pTex = D3DTexture to use for the face

	virtual void	SetCubeMapFace(DWORD_PTR pCubeMap, DWORD face,
		UINT mipLevel, DWORD_PTR pTex) = 0;

	// Build a D3DVertexDeclaration
	//
	//	nComp = number of separate vertex data components
	//	pUse = array of length nComp indicating D3DDECLUSAGE of each data array
	//	fComp = bit flags for number of float components in each data array,
	//		e.g. 0x00 = 4 floats, 0x01 = 1 float, 0x02 = 2 floats, and
	//		0x03 = 3 floats, enough space for 16 flags
	//
	//	Returns a pointer to a GFX D3DVertexDecl class

	virtual DWORD_PTR	BuildVertexDecl(DWORD nComp, DWORD *pUse,
		DWORD fComp) = 0;

	// Set a D3DVertexDeclaration
	virtual void	SetVertexDecl(DWORD_PTR pVertexDecl) = 0;

	// Free a D3DVertexDeclaration
	virtual void	FreeVertexDecl(DWORD_PTR pVertexDecl) = 0;

	// Build a D3DVertexShader
	//
	//	code = string with Shader instructions
	//
	//	Returns a pointer to a GFX D3DVertexShader class

	virtual DWORD_PTR	BuildVertexShader(LPCSTR code) = 0;

	// Load a D3DVertexShader from a file
	//
	//	Returns a pointer to a GFX D3DVertexShader class

	virtual DWORD_PTR	LoadVertexShader(LPCSTR filename) = 0;

	// Save a D3DVertexShader to a file
	virtual void	SaveVertexShader(LPCSTR filename, DWORD_PTR pVertexShader) = 0;

	// Set a D3DVertexShader
	virtual void	SetVertexShader(DWORD_PTR pVertexShader) = 0;

	// Set D3DVertexShader Constants
	//
	//	nReg is the number of the register where the data loading starts,
	//	type is the D3DXPARAMETERTYPE of the parameter, pData is the actual
	//	data and nData is the number of four 32 bit sets of data
	//
	virtual void	SetVertexShaderConstant(DWORD_PTR pVertexShader,
		DWORD nReg, DWORD type, void *pData, DWORD nData) = 0;

	// Free a D3DVertexShader
	virtual void	FreeVertexShader(DWORD_PTR pVertexShader) = 0;

	// Build a D3DPixelShader
	//
	//	code = string with Shader instructions
	//
	//	Returns a pointer to a GFX D3DPixelShader class

	virtual DWORD_PTR	BuildPixelShader(LPCSTR code) = 0;

	// Load a D3DPixelShader
	//
	//	Returns a pointer to a GFX D3DPixelShader class

	virtual DWORD_PTR	LoadPixelShader(LPCSTR filename) = 0;

	// Save a D3DPixelShader to a file
	virtual void	SavePixelShader(LPCSTR filename, DWORD_PTR pPixelShader) = 0;

	// Set a D3DPixelShader
	virtual void	SetPixelShader(DWORD_PTR pPixelShader) = 0;

	// Set D3DPixelShader Constants
	//
	//	nReg is the number of the register where the data loading starts,
	//	type is the D3DXPARAMETERTYPE of the parameter, pData is the actual
	//	data and nData is the number of four 32 bit sets of data
	//
	virtual void	SetPixelShaderConstant(DWORD_PTR pPixelShader,
		DWORD nReg, DWORD type, void *pData, DWORD nData) = 0;

	// Free a D3DPixelShader
	virtual void	FreePixelShader(DWORD_PTR pPixelShader) = 0;

	// Build a D3DXEffect
	//
	//	Returns a pointer to a GFX D3DEffect class

	virtual	DWORD_PTR	BuildEffect(LPCSTR code) = 0;

	// Load a D3DXEffect from a file
	virtual DWORD_PTR	LoadEffect(LPCSTR filename) = 0;

	// Save a D3DXEffect to a file
	virtual void	SaveEffect(LPCSTR filename, DWORD_PTR pEffect) = 0;

	// Set D3DXEffect Parameters
	//
	//	name is the name of the parameter, type is the D3DXPARAMETERTYPE of the
	//	parameter, pData is the actual data
	//
	virtual void	SetEffectParameter(DWORD_PTR pEffect, LPCSTR name,
		DWORD type, void *pData) = 0;

	// Set D3DXEffect Technique
	//
	//	name is the name of the technique
	//
	virtual void	SetEffectTechnique(DWORD_PTR pEffect, LPCSTR name) = 0;

	// Begin a D3DXEffect
	virtual void	BeginEffect(DWORD_PTR pEffect) = 0;

	// Set a D3DXEffect Pass
	virtual void	SetEffectPass(DWORD_PTR pEffect, DWORD nPass) = 0;

	// End a D3DXEffect
	virtual void	EndEffect(DWORD_PTR pEffect) = 0;

	// Free a D3DXEffect
	virtual void	FreeEffect(DWORD_PTR pEffect) = 0;

	// Begin D3DXEffect access
	//
	//	The D3DXEffect associated with the GFX D3DEffect is returned
	//
	virtual DWORD_PTR	BeginEffectAccess(DWORD_PTR pEffect, GFX_ESCAPE_FN fn) = 0;

	// End D3DXEffect access
	virtual void	EndEffectAccess(GFX_ESCAPE_FN fn) = 0;

	// Begin Direct3D Device access
	//
	//	The Direct3DDevice is returned
	//
	virtual DWORD_PTR	BeginDeviceAccess(GFX_ESCAPE_FN fn) = 0;

	// End Direct3D Device access
	virtual void	EndDeviceAccess(GFX_ESCAPE_FN fn) = 0;

	// This tells the mesh / mnmesh classes whther a shader has performed the drawing already
	// so it can simply ignore the draw call.  This prevents double draws.
	virtual bool ObjectDrawnByShader() = 0;
};

#endif
