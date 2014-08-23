/**********************************************************************
 *<
	FILE: CubeMap.h

	DESCRIPTION:	Includes for Plugins

	CREATED BY:		Neil Hazzard

	HISTORY: created 04/30/02
	Based on the code by Nikolai Sander from the r4 samples

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#ifndef __CUBEMAP__H
#define __CUBEMAP__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"

#include "IDX8PixelShader.h"
#include "IDX8VertexShader.h"
#include "ID3DGraphicsWindow.h"
#include "IStdDualVS.h"
#include "reflect.h"

extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

#define SAFE_DELETE(p)			{ if (p) { delete (p);		(p)=NULL; } }
#define SAFE_DELETE_ARRAY(p)	{ if (p) { delete[] (p);	(p)=NULL; } }
#define SAFE_RELEASE(p)			{ if (p) { (p)->Release();	(p)=NULL; } }

class CubeMap;

class IDX8PixelShaderImp : public IDX8PixelShader 
{
	bool	initDone;

	LPDIRECT3DDEVICE8 pd3dDevice;

	LPDIRECT3DCUBETEXTURE8	pCubeTexture;

	DWORD	dwPixelShader;
	
	CubeMap *map;

public :
	IDX8PixelShaderImp(CubeMap *map);
	~IDX8PixelShaderImp();

	HRESULT ConfirmDevice(ID3DGraphicsWindow *d3dgw);

	HRESULT	ConfirmVertexShader(IDX8VertexShader *pvs);

	HRESULT Initialize(Material *mtl, INode *node);

	int GetNumMultiPass();

	DWORD GetPixelShaderHandle(int numPass) { return dwPixelShader; }

	HRESULT SetPixelShader(ID3DGraphicsWindow *d3dgw, int numPass);
};

class IDX8VertexShaderCache : public VertexShaderCache
{
public:
		
};

class IDX8PixelVertexShaderImp : public IDX8VertexShader, public IStdDualVSCallback
{
	bool initDone;

	LPDIRECT3DDEVICE8 pd3dDevice;
	
	LPDIRECT3DCUBETEXTURE8	pCubeTexture;
	// VertexShader Declarations
	Tab<DWORD> Declaration;

	// VertexShader Instructions
	LPD3DXBUFFER pCode;

	// VertexShader Constants
	Tab<D3DXVECTOR4> Constants;

		// VertexShader Handle
	DWORD	dwVertexShader;
	
	IStdDualVS *stdDualVS;
	CubeMap *map;

public:
	IDX8PixelVertexShaderImp(CubeMap *map);
	~IDX8PixelVertexShaderImp();
	
	virtual HRESULT Initialize(Mesh *mesh, INode *node);
	virtual HRESULT Initialize(MNMesh *mnmesh, INode *node);
	// From FPInterface
	virtual LifetimeType	LifetimeControl() { return noRelease; }

	// From IVertexShader
	HRESULT ConfirmDevice(ID3DGraphicsWindow *d3dgw);
	
	HRESULT ConfirmPixelShader(IDX8PixelShader *pps);

	bool CanTryStrips();

	int GetNumMultiPass();

	DWORD GetVertexShaderHandle(int numPass) { return dwVertexShader; }

	HRESULT SetVertexShader(ID3DGraphicsWindow *d3dgw, int numPass);
	
	// Draw 3D mesh as TriStrips
	bool	DrawMeshStrips(ID3DGraphicsWindow *d3dgw, MeshData *data);

	// Draw 3D mesh as wireframe
	bool	DrawWireMesh(ID3DGraphicsWindow *d3dgw, WireMeshData *data);

	// Draw 3D lines
	void	StartLines(ID3DGraphicsWindow *d3dgw, WireMeshData *data);
	void	AddLine(ID3DGraphicsWindow *d3dgw, DWORD *vert, int vis);
	bool	DrawLines(ID3DGraphicsWindow *d3dgw);
	void	EndLines(ID3DGraphicsWindow *d3dgw, GFX_ESCAPE_FN fn);

	// Draw 3D triangles
	void	StartTriangles(ID3DGraphicsWindow *d3dgw, MeshFaceData *data);
	void	AddTriangle(ID3DGraphicsWindow *d3dgw, DWORD index, int *edgeVis);
	bool	DrawTriangles(ID3DGraphicsWindow *d3dgw);
	void	EndTriangles(ID3DGraphicsWindow *d3dgw, GFX_ESCAPE_FN fn);

	// from IStdDualVSCallback
	virtual ReferenceTarget *GetRefTarg();
	virtual VertexShaderCache *CreateVertexShaderCache();
	virtual HRESULT  InitValid(Mesh* mesh, INode *node);
	virtual HRESULT  InitValid(MNMesh* mnmesh, INode *node);
	HRESULT LoadAndCompileShader();
};

#endif // __CUBEMAP__H
