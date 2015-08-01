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
	LoadShaderIncludes();
	CreateQuadIB();

	// streams
	Vertex.Create();
	Index.Create();

	// invalidate caching
	Invalidate();
}

void CBackend::OnDeviceDestroy()
{
	// Quad
	glDeleteBuffers(1, &QuadIB);
}

void CBackend::LoadShaderIncludes()
{
	// Open file list
	typedef xr_vector<LPSTR>		file_list_type;
	string_path						dirname;
	strconcat(sizeof(dirname), dirname, ::Render->getShaderPath(), "shared\\");
	file_list_type*					file_list = FS.file_list_open("$game_shaders$", dirname);
	VERIFY(file_list);

	file_list_type::const_iterator	i = file_list->begin();
	file_list_type::const_iterator	e = file_list->end();
	for (; i != e; ++i) {
		// Open file
		string_path					cname;
		strconcat(sizeof(cname), cname, ::Render->getShaderPath(), "shared\\", *i);
		FS.update_path(cname, "$game_shaders$", cname);

		// Prefix root path
		string_path					glname;
		strconcat(sizeof(glname), glname, "/", *i);

		// Load the include file
		IReader*		R = FS.r_open(cname);
		R_ASSERT2(R, cname);
		CHK_GL(glNamedStringARB(GL_SHADER_INCLUDE_ARB, xr_strlen(glname), glname, R->length(), (GLchar*)R->pointer()));
		FS.r_close(R);
	}

	FS.file_list_close(file_list);
}
