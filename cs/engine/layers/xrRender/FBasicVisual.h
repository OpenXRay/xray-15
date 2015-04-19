#ifndef FBasicVisualH
#define FBasicVisualH
#pragma once

#include "../../xrEngine/vis_common.h"

#ifdef USE_OGL
#include "glRenderVisual.h"
#else
#include "dxRenderVisual.h"
#endif // USE_OGL

struct					IRender_Mesh	
{
	// format
	ref_geom					rm_geom;

	// verts
#ifdef USE_OGL
	GLuint						p_rm_Vertices;
#else
	ID3DVertexBuffer*			p_rm_Vertices;
#endif // USE_OGL
	u32							vBase;
	u32							vCount;

	// indices
#ifdef USE_OGL
	GLuint						p_rm_Indices;
#else
	ID3DIndexBuffer*			p_rm_Indices;
#endif // USE_OGL
	u32							iBase;
	u32							iCount;
	u32							dwPrimitives;

	IRender_Mesh				()				{ p_rm_Vertices=0; p_rm_Indices=0;						}
	virtual ~IRender_Mesh		();
private:
	IRender_Mesh				(const IRender_Mesh& other);
	void	operator=			( const IRender_Mesh& other);
};

#endif // !FBasicVisualH
