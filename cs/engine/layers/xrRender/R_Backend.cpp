#include "stdafx.h"
#pragma hdrstop

#include "../xrRenderDX10/dx10BufferUtils.h"

CBackend			RCache;
#ifdef USE_OGL

void CBackend::RestoreQuadIBData()
{
}

void CBackend::CreateQuadIB()
{
	const u32 dwTriCount = 4 * 1024;
	const u32 dwIdxCount = dwTriCount * 2 * 3;
	u16		IndexBuffer[dwIdxCount];
	u16		*Indices = IndexBuffer;
	GLenum	dwUsage = GL_STATIC_DRAW;

	int		Cnt = 0;
	int		ICnt = 0;
	for (int i = 0; i<dwTriCount; i++)
	{
		Indices[ICnt++] = u16(Cnt + 0);
		Indices[ICnt++] = u16(Cnt + 1);
		Indices[ICnt++] = u16(Cnt + 2);

		Indices[ICnt++] = u16(Cnt + 3);
		Indices[ICnt++] = u16(Cnt + 2);
		Indices[ICnt++] = u16(Cnt + 1);

		Cnt += 4;
	}

	glGenBuffers(1, &QuadIB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, QuadIB);
	CHK_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, dwIdxCount * 2, Indices, dwUsage));
}

#else

// Create Quad-IB
#ifdef	USE_DX10

// Igor: is used to test bug with rain, particles corruption
void CBackend::RestoreQuadIBData()
{
	// Igor: never seen this corruption for DX10
	;
}

void CBackend::CreateQuadIB		()
{
	static const u32 dwTriCount	= 4*1024;
	static const u32 dwIdxCount	= dwTriCount*2*3;
	u16		IndexBuffer[dwIdxCount];
	u16		*Indices		= IndexBuffer;
	//u32		dwUsage			= D3DUSAGE_WRITEONLY;
	//if (HW.Caps.geometry.bSoftware)	dwUsage|=D3DUSAGE_SOFTWAREPROCESSING;
	//R_CHK(HW.pDevice->CreateIndexBuffer(dwIdxCount*2,dwUsage,D3DFMT_INDEX16,D3DPOOL_MANAGED,&QuadIB,NULL));

	D3D10_BUFFER_DESC desc;
	desc.ByteWidth = dwIdxCount*2;
	//desc.Usage = D3D10_USAGE_IMMUTABLE;
	desc.Usage = D3D10_USAGE_DEFAULT;
	desc.BindFlags = D3D10_BIND_INDEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D10_SUBRESOURCE_DATA subData;
	subData.pSysMem = IndexBuffer;

	//R_CHK(QuadIB->Lock(0,0,(void**)&Indices,0));
	{
		int		Cnt = 0;
		int		ICnt= 0;
		for (int i=0; i<dwTriCount; i++)
		{
			Indices[ICnt++]=u16(Cnt+0);
			Indices[ICnt++]=u16(Cnt+1);
			Indices[ICnt++]=u16(Cnt+2);

			Indices[ICnt++]=u16(Cnt+3);
			Indices[ICnt++]=u16(Cnt+2);
			Indices[ICnt++]=u16(Cnt+1);

			Cnt+=4;
		}
	}
	//R_CHK(QuadIB->Unlock());

	//R_CHK(HW.pDevice->CreateIndexBuffer(dwIdxCount*2,dwUsage,D3DFMT_INDEX16,D3DPOOL_MANAGED,&QuadIB,NULL));
	R_CHK(HW.pDevice->CreateBuffer( &desc, &subData, &QuadIB));
}

#else	//	USE_DX10

// Igor: is used to test bug with rain, particles corruption
void CBackend::RestoreQuadIBData()
{
	const u32 dwTriCount	= 4*1024;
	u16		*Indices		= 0;
	R_CHK(QuadIB->Lock(0,0,(void**)&Indices,0));
	{
		int		Cnt = 0;
		int		ICnt= 0;
		for (int i=0; i<dwTriCount; i++)
		{
			Indices[ICnt++]=u16(Cnt+0);
			Indices[ICnt++]=u16(Cnt+1);
			Indices[ICnt++]=u16(Cnt+2);

			Indices[ICnt++]=u16(Cnt+3);
			Indices[ICnt++]=u16(Cnt+2);
			Indices[ICnt++]=u16(Cnt+1);

			Cnt+=4;
		}
	}
	R_CHK(QuadIB->Unlock());
}


void CBackend::CreateQuadIB		()
{
	const u32 dwTriCount	= 4*1024;
	const u32 dwIdxCount	= dwTriCount*2*3;
	u16		*Indices		= 0;
	u32		dwUsage			= D3DUSAGE_WRITEONLY;
	if (HW.Caps.geometry.bSoftware)	dwUsage|=D3DUSAGE_SOFTWAREPROCESSING;
	R_CHK(HW.pDevice->CreateIndexBuffer(dwIdxCount*2,dwUsage,D3DFMT_INDEX16,D3DPOOL_MANAGED,&QuadIB,NULL));
	R_CHK(QuadIB->Lock(0,0,(void**)&Indices,0));
	{
		int		Cnt = 0;
		int		ICnt= 0;
		for (int i=0; i<dwTriCount; i++)
		{
			Indices[ICnt++]=u16(Cnt+0);
			Indices[ICnt++]=u16(Cnt+1);
			Indices[ICnt++]=u16(Cnt+2);

			Indices[ICnt++]=u16(Cnt+3);
			Indices[ICnt++]=u16(Cnt+2);
			Indices[ICnt++]=u16(Cnt+1);

			Cnt+=4;
		}
	}
	R_CHK(QuadIB->Unlock());
}

#endif	//	USE_DX10

#endif // USE_OGL

// Device dependance
void CBackend::OnDeviceCreate	()
{
#ifdef	USE_DX10
	//CreateConstantBuffers();
#endif	//	USE_DX10
#ifdef USE_OGL
	// Create the program pipeline used for rendering with shaders
	glGenProgramPipelines(1, &pp);
#endif // USE_OGL


	CreateQuadIB		();

	// streams
	Vertex.Create		();
	Index.Create		();

	// invalidate caching
	Invalidate			();
}

void CBackend::OnDeviceDestroy()
{
	// streams
	Index.Destroy		();
	Vertex.Destroy		();

	// Quad
#ifdef	USE_OGL
	glDeleteBuffers(1, &QuadIB);
#else
	_RELEASE			(QuadIB);
#endif

#ifdef	USE_DX10
	//DestroyConstantBuffers();
#endif	//	USE_DX10
#ifdef USE_OGL
	glDeleteProgramPipelines(1, &pp);
#endif // USE_OGL
}

#ifdef	USE_DX10
/*
void CBackend::CreateConstantBuffers()
{
	const int iVectorElements = 4;
	const int iVectorNumber = 256;
	dx10BufferUtils::CreateConstantBuffer(&m_pPixelConstants, iVectorNumber*iVectorElements*sizeof(float));
	dx10BufferUtils::CreateConstantBuffer(&m_pVertexConstants, iVectorNumber*iVectorElements*sizeof(float));
}

void CBackend::DestroyConstantBuffers()
{
	_RELEASE(m_pVertexConstants);
	_RELEASE(m_pPixelConstants);
}
*/
#endif	//	USE_DX10
