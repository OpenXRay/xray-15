#include "stdafx.h"

CBackend			RCache;

// Igor: is used to test bug with rain, particles corruption
void CBackend::RestoreQuadIBData()
{
	const u32 dwTriCount = 4 * 1024;
	u16		*Indices = 0;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, QuadIB);
	Indices = (u16*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	{
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
	}
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


void CBackend::CreateQuadIB()
{
	const u32 dwTriCount = 4 * 1024;
	const u32 dwIdxCount = dwTriCount * 2 * 3;
	u16		*Indices = 0;
	GLenum	dwUsage = GL_DYNAMIC_DRAW;

	glGenBuffers(1, &QuadIB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, QuadIB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, dwIdxCount * 2, 0, dwUsage);
	Indices = (u16*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);

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

	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
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
