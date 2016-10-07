// dxRender_Visual.cpp: implementation of the dxRender_Visual class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#pragma hdrstop

#include "fbasicvisual.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IRender_Mesh::~IRender_Mesh()		
{ 
#ifdef USE_OGL
	GLuint buffers[] = { p_rm_Vertices, p_rm_Indices };
	glDeleteBuffers(2, buffers);
#else
	_RELEASE(p_rm_Vertices);
	_RELEASE(p_rm_Indices);
#endif // USE_OGL
}
