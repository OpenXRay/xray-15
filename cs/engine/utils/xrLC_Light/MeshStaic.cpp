#include "stdafx.h"

#include "xrLC_GlobalData.h"

#include "xrface.h"


typedef poolSS<Vertex,8*1024>	poolVertices;
typedef poolSS<Face,8*1024>		poolFaces;
static poolVertices	_VertexPool;
static poolFaces	_FacePool;

Face* xrLC_GlobalData	::create_face	()		
{
	return _FacePool.create();
}
void xrLC_GlobalData	::destroy_face	(Face* &f)
{
	_FacePool.destroy( f );
}

Vertex* xrLC_GlobalData	::create_vertex	()		
{
	return _VertexPool.create();
}
void xrLC_GlobalData	::destroy_vertex	(Vertex* &f)
{
	_VertexPool.destroy( f );
}

void xrLC_GlobalData	::gl_mesh_clear	()
{
	_g_vertices.clear();
	_g_faces.clear();
	_VertexPool.clear();
	_FacePool.clear();
}






