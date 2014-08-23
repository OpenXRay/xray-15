#ifndef	xrD3DDefs_included
#define	xrD3DDefs_included
#pragma once

#ifdef	USE_DX10

class	dx10State;

typedef	ID3D10VertexShader		ID3DVertexShader;
typedef	ID3D10GeometryShader	ID3DGeometryShader;
typedef	ID3D10PixelShader		ID3DPixelShader;
typedef	ID3D10Blob				ID3DBlob;
typedef	D3D10_SHADER_MACRO		D3D_SHADER_MACRO;
typedef	ID3D10Query				ID3DQuery;
typedef	D3D10_VIEWPORT			D3D_VIEWPORT;
typedef	ID3D10Include			ID3DInclude;
typedef	ID3D10Texture2D			ID3DTexture2D;
typedef	ID3D10RenderTargetView	ID3DRenderTargetView;
typedef	ID3D10DepthStencilView	ID3DDepthStencilView;
typedef	ID3D10Resource			ID3DBaseTexture;
typedef	D3D10_TEXTURE2D_DESC	D3D_TEXTURE2D_DESC;
typedef ID3D10Buffer			ID3DVertexBuffer;
typedef ID3D10Buffer			ID3DIndexBuffer;
typedef	ID3D10Texture3D			ID3DTexture3D;
typedef	dx10State				ID3DState;

#define DX10_ONLY(expr)			expr

#else	//	USE_DX10

typedef	IDirect3DVertexShader9	ID3DVertexShader;
typedef	IDirect3DPixelShader9	ID3DPixelShader;
typedef	ID3DXBuffer				ID3DBlob;
typedef	D3DXMACRO				D3D_SHADER_MACRO;
typedef	IDirect3DQuery9			ID3DQuery;
typedef	D3DVIEWPORT9			D3D_VIEWPORT;
typedef	ID3DXInclude			ID3DInclude;
typedef	IDirect3DTexture9		ID3DTexture2D;
typedef	IDirect3DSurface9		ID3DRenderTargetView;
typedef	IDirect3DSurface9		ID3DDepthStencilView;
typedef	IDirect3DBaseTexture9	ID3DBaseTexture;
typedef	D3DSURFACE_DESC			D3D_TEXTURE2D_DESC;
typedef IDirect3DVertexBuffer9	ID3DVertexBuffer;
typedef IDirect3DIndexBuffer9	ID3DIndexBuffer;
typedef	IDirect3DVolumeTexture9	ID3DTexture3D;
typedef	IDirect3DStateBlock9	ID3DState;

#define DX10_ONLY(expr)			do {} while (0)

#endif	//	USE_DX10


#endif	//	xrD3DDefs_included