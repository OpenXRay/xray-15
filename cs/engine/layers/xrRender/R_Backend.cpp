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
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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
#ifdef	USE_OGL
	LoadShaderIncludes();
#endif	//	USE_OGL

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

#ifdef USE_OGL
void CBackend::LoadShaderIncludes()
{
	// Open file list
	typedef xr_vector<LPSTR>		file_list_type;
	file_list_type*					file_list = FS.file_list_open("$game_shaders$", ::Render->getShaderPath());
	VERIFY(file_list);

	file_list_type::const_iterator	i = file_list->begin();
	file_list_type::const_iterator	e = file_list->end();
	for (; i != e; ++i) {
		// Open file
		string_path					cname, fn;
		strcpy_s(fn, *i);
		if (0 == strext(fn) || 0 != xr_strcmp(strext(fn), ".h"))	continue;
		strconcat(sizeof(cname), cname, ::Render->getShaderPath(), fn);
		FS.update_path(cname, "$game_shaders$", cname);

		// Convert to OGL path
		string_path					glname;
		while (char* sep = strchr(fn, '\\')) *sep = '/';
		strconcat(sizeof(glname), glname, "/", fn);

		// Load the include file
		IReader*		R = FS.r_open(cname);
		R_ASSERT2(R, cname);
		CHK_GL(glNamedStringARB(GL_SHADER_INCLUDE_ARB, xr_strlen(glname), glname, R->length(), (GLchar*)R->pointer()));
		FS.r_close(R);
	}

	FS.file_list_close(file_list);
}
#endif // USE_OGL
