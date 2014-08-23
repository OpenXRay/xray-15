#ifndef D3DViewportRenderer_h_
#define D3DViewportRenderer_h_

#include <maya/MViewportRenderer.h>
#include <maya/MImage.h>
#include <maya/MDagPath.h>
#include <maya/MObjectHandle.h>
#include <maya/MMessage.h> // For monitoring geometry list nodes
#include <maya/MStringArray.h>
#include <list>

// Resources
#include <D3DResourceManager.h>

//#define D3D9_SUPPORTED
#if defined(D3D9_SUPPORTED)
	#define WIN32_LEAN_AND_MEAN
	#include <d3d9.h>
	#include <d3dx9.h>
#endif

#if defined(D3D9_SUPPORTED)
// Vertex with just position attribute
struct PlainVertex
{
	float x, y, z;

	enum FVF
	{
		FVF_Flags = D3DFVF_XYZ
	};
};
#endif

class MMatrix;
class MBoundingBox;
class MDagPath;
class MObject;

//
// Sample plugin viewport renderer using the Direct3D api.
//
class D3DViewportRenderer : public MViewportRenderer
{
public:
	D3DViewportRenderer();
	virtual ~D3DViewportRenderer();

	// Required virtual overrides from MViewportRenderer
	//
	virtual	MStatus	initialize();
	virtual	MStatus	uninitialize();
	virtual MStatus	render( const MRenderingInfo &renderInfo );
	virtual bool	nativelySupports( MViewportRenderer::RenderingAPI api, 
										  float version );
	virtual bool	override( MViewportRenderer::RenderingOverride override );

protected:

	RenderingAPI	m_API;		// Rendering API
	float			m_Version;	// Direct3D version number as float.

	// Last / current render dimensions
	unsigned int			m_renderWidth;
	unsigned int			m_renderHeight;

#if defined(D3D9_SUPPORTED)
public:
	bool					buildRenderTargets(unsigned int width, unsigned int height);

	bool					translateCamera( const MRenderingInfo &renderInfo );
	bool					setupMatrices();

	// Main entry point to render
	bool					renderToTarget( const MRenderingInfo &renderInfo );

	// Regular scene drawing routines
	bool					drawScene( const MRenderingInfo &renderInfo );
	bool					drawBounds(const MMatrix &matrix, const MBoundingBox &box, bool filled, bool useDummyGeometry,
										float color[3],
										LPDIRECT3DVERTEXBUFFER9 buffer = NULL);
	bool					drawCube(float minVal[3], float maxVal[3], bool filled, bool useDummyGeometry,
										float color[3],
										LPDIRECT3DVERTEXBUFFER9 buffer = NULL);
	bool					drawSurface( const MDagPath &dagPath, bool active, bool templated);
	bool					setSurfaceMaterial( const MDagPath &dagPath );

	// Post-processing
	bool					initializePostEffects(LPDIRECT3DDEVICE9 D3D);
	void					drawFullScreenQuad(float leftU, float topV, 
											 float rightU, float bottomV,
											 float targetWidth, float targetHeight,
											 LPDIRECT3DDEVICE9 D3D);
	bool					postRenderToTarget();

	// Readback to system memory from target
	bool					readFromTargetToSystemMemory();

	// Resource management
	const D3DResourceManager&	resourceManager() const { return m_resourceManager; }
	void					clearResources(bool onlyInvalidItems, bool clearShaders);

protected:
	// Basics to setup D3D
	//
	HWND					m_hWnd;			// Handle to window
	LPDIRECT3D9				m_pD3D;			// D3D
	LPDIRECT3DDEVICE9		m_pD3DDevice;	// Reference to a device 

	// Off screen render targets
	//
	bool					m_wantFloatingPointTargets; // Do we want floating point render targets (intermediates)
	D3DFORMAT				m_intermediateTargetFormat; // Format used for intermediate render targets.
	D3DFORMAT				m_outputTargetFormat;  // Output color buffer format.
	LPDIRECT3DTEXTURE9		m_pTextureInterm;	// Intermediate texture render target
	LPDIRECT3DSURFACE9		m_pTextureIntermSurface; // Intermediate surface of the texture.
	LPDIRECT3DTEXTURE9		m_pTextureOutput;	// Output texture render target
	LPDIRECT3DSURFACE9		m_pTextureOutputSurface; // Output surface of the texture.
	LPDIRECT3DTEXTURE9		m_pTexturePost;	// Intermediate Post-processing render target

	// Offscreen depth / stencil. We create one for general purpose usage
	D3DFORMAT				m_depthStencilFormat; // Depth+stencil format. Same for all targets.
	bool					m_requireDepthStencilReadback;
	LPDIRECT3DSURFACE9		m_pDepthStencilSurface;

	LPDIRECT3DSURFACE9		m_SystemMemorySurface; // System memory surface for readback.

	// Camera and model matrices
	//
    D3DXMATRIXA16			m_matWorld;		// Object to world matrix

	// System memory for colour buffer readback
	MImage					m_readBackBuffer;

	LPDIRECT3DVERTEXBUFFER9 m_pBoundsBuffer;
	LPD3DXMESH				m_pGeometry;

	// D3D Resources. Geometry, textures, lights, shaders etc.
	//
	D3DResourceManager		m_resourceManager;
#endif
};

#endif /* D3DViewportRenderer_h_ */

//-
// ==========================================================================
// Copyright (C) 1995 - 2006 Autodesk, Inc. and/or its licensors.  All 
// rights reserved.
//
// The coded instructions, statements, computer programs, and/or related 
// material (collectively the "Data") in these files contain unpublished 
// information proprietary to Autodesk, Inc. ("Autodesk") and/or its 
// licensors, which is protected by U.S. and Canadian federal copyright 
// law and by international treaties.
//
// The Data is provided for use exclusively by You. You have the right 
// to use, modify, and incorporate this Data into other products for 
// purposes authorized by the Autodesk software license agreement, 
// without fee.
//
// The copyright notices in the Software and this entire statement, 
// including the above license grant, this restriction and the 
// following disclaimer, must be included in all copies of the 
// Software, in whole or in part, and all derivative works of 
// the Software, unless such copies or derivative works are solely 
// in the form of machine-executable object code generated by a 
// source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND. 
// AUTODESK DOES NOT MAKE AND HEREBY DISCLAIMS ANY EXPRESS OR IMPLIED 
// WARRANTIES INCLUDING, BUT NOT LIMITED TO, THE WARRANTIES OF 
// NON-INFRINGEMENT, MERCHANTABILITY OR FITNESS FOR A PARTICULAR 
// PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE, OR 
// TRADE PRACTICE. IN NO EVENT WILL AUTODESK AND/OR ITS LICENSORS 
// BE LIABLE FOR ANY LOST REVENUES, DATA, OR PROFITS, OR SPECIAL, 
// DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES, EVEN IF AUTODESK 
// AND/OR ITS LICENSORS HAS BEEN ADVISED OF THE POSSIBILITY 
// OR PROBABILITY OF SUCH DAMAGES.
//
// ==========================================================================
//+

