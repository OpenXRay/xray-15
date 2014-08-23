/**********************************************************************
 *<
	FILE: ID3D9GraphicsWindow.h

	DESCRIPTION: Direct3D 9.0 Graphics Window Extension Interface class

	CREATED BY: Norbert Jeske

	HISTORY:

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#ifndef _D3D9_GRAPHICS_WINDOW_H_
#define _D3D9_GRAPHICS_WINDOW_H_

#include <d3dx9.h>

#define D3D9_GRAPHICS_WINDOW_INTERFACE_ID Interface_ID(0x56424386, 0x2151b83)

class Mesh;

class ID3D9GraphicsWindow : public BaseInterface
{
public:
	virtual Interface_ID	GetID() { return D3D9_GRAPHICS_WINDOW_INTERFACE_ID; }

	// Interface Lifetime
	virtual LifetimeType	LifetimeControl() { return noRelease; }

	// Get Direct3D Device from GFX
	virtual LPDIRECT3DDEVICE9	GetDevice() = 0;

	// Get VertexBuffer from GFX.  Unless older Flexible Vertex Formats are in
	// use, FVF should be zero.
	virtual LPDIRECT3DVERTEXBUFFER9	GetVertexBuffer(UINT length, DWORD FVF) = 0;

	// Get IndexBuffer from GFX
	virtual LPDIRECT3DINDEXBUFFER9	GetIndexBuffer(UINT length, D3DFORMAT format) = 0;

	// Get Transforms from GFX
	virtual D3DXMATRIX	GetWorldXform() = 0;
	virtual D3DXMATRIX	GetViewXform() = 0;
	virtual D3DXMATRIX	GetProjXform() = 0;

	// Get Constant Color of specified type from GFX
	virtual D3DCOLOR	GetColor(ColorType t) = 0;

	// Get a pointer to a 'Tab' table array of pointers to enabled Direct3D
	// Lights from GFX
	virtual Tab<D3DLIGHT9 *> *GetLights() = 0;

	// Get Material from GFX
	virtual D3DMATERIAL9	GetMaterial() = 0;

	// Get Texture Tiling for specified texStage and texCoord from GFX
	virtual DWORD	GetTextureTiling(int texStage, int coord) = 0;

	// Get Texture Transfrom for specified texStage from GFX
	virtual D3DXMATRIX	GetTexXform(int texStage) = 0;

	// Get the current viewport dimensions. This is useful for doing multi pass rendering and the off screen buffer needs
	// to be created the same size as the viewport currently being drawn.
	virtual void GetWindowDimension(int &width, int &height) = 0;
};

#endif
