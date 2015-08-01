#include "stdafx.h"

CBackend			RCache;

// Igor: is used to test bug with rain, particles corruption
void CBackend::RestoreQuadIBData()
{
	// Igor: never seen this corruption for DX10
	;
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

// Device dependance
void CBackend::OnDeviceCreate()
{
	CreateQuadIB();

	// invalidate caching
	Invalidate();
}

void CBackend::OnDeviceDestroy()
{
	// Quad
	glDeleteBuffers(1, &QuadIB);
}
