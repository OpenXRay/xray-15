#include "stdafx.h"

#include "xrface.h"
//#include "build.h"
#include "xrdeflector.h"
#include "xrLC_globaldata.h"
#include "serialize.h"
#include "lightmap.h"
volatile u32					dwInvalidFaces;//= 0;
u32		InvalideFaces()
{
	return dwInvalidFaces;
}

const Shader_xrLC&	base_Face::Shader		()const
{
	VERIFY( inlc_global_data() );
	return shader( dwMaterial, inlc_global_data()->shaders(), inlc_global_data()->materials() );
}
void			base_Face::CacheOpacity	()
{
	flags.bOpaque				= true;
	VERIFY ( inlc_global_data() );

	b_material& M		= inlc_global_data()->materials()		[dwMaterial];
	b_texture&	T		= inlc_global_data()->textures()		[M.surfidx];
	if (T.bHasAlpha)	flags.bOpaque = false;
	else				flags.bOpaque = true;
	if (!flags.bOpaque && (0==T.pSurface))		{
		flags.bOpaque	= true;
		clMsg			("Strange face detected... Has alpha without texture...");
	}
}

Face*	Face::read_create( )
{
	return inlc_global_data()->create_face();
}



//
//const int	edge2idx	[3][2]	= { {0,1},		{1,2},		{2,0}	};
//const int	edge2idx3	[3][3]	= { {0,1,2},	{1,2,0},	{2,0,1}	};
//const int	idx2edge	[3][3]  = {
//	{-1,  0,  2},
//	{ 0, -1,  1},
//	{ 2,  1, -1}
//};



//extern CBuild*	pBuild;

bool			g_bUnregister = true;

//template<>
void destroy_vertex( Vertex* &v, bool unregister )
{
	g_bUnregister = unregister;
	inlc_global_data()->destroy_vertex( v );
	g_bUnregister = true;
}
//vecVertex		g_vertices;
//vecFace			g_faces;

//poolVertices	VertexPool;
//poolFaces		FacePool;

Tvertex<DataVertex>::Tvertex()
{
	VERIFY( inlc_global_data() );
	if( !inlc_global_data()->b_r_vertices() )
		inlc_global_data()->g_vertices().push_back(this);
	m_adjacents.reserve	(4);
}

template <>
Tvertex<DataVertex>::~Tvertex()
{
	if (g_bUnregister) {
		vecVertexIt F = std::find(inlc_global_data()->g_vertices().begin(), inlc_global_data()->g_vertices().end(), this);
		if (F!=inlc_global_data()->g_vertices().end()) inlc_global_data()->g_vertices().erase(F);
		else clMsg("* ERROR: Unregistered VERTEX destroyed");
	}
}

Vertex*	Vertex::CreateCopy_NOADJ( vecVertex& vertises_storage ) const
{
	VERIFY( &vertises_storage == &inlc_global_data()->g_vertices() );
	Vertex* V	= inlc_global_data()->create_vertex();
	V->P.set	(P);
	V->N.set	(N);
	V->C		= C;
	return		V;
}

Vertex*	Vertex::read_create( )
{
	return inlc_global_data()->create_vertex();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////





template<>
Tface<DataVertex>::Tface()
{
	pDeflector				= 0;
	flags.bSplitted			= false;
	VERIFY( inlc_global_data() );
	if( !inlc_global_data()->b_r_faces() )
		inlc_global_data()->g_faces().push_back		(this);
	sm_group				= u32(-1);
	lmap_layer				= NULL;
}

template<>
Tface<DataVertex>::~Tface()
{
	if (g_bUnregister) 
	{
		vecFaceIt F = std::find(inlc_global_data()->g_faces().begin(), inlc_global_data()->g_faces().end(), this);
		if (F!=inlc_global_data()->g_faces().end()) inlc_global_data()->g_faces().erase(F);
		else clMsg("* ERROR: Unregistered FACE destroyed");
	}
	// Remove 'this' from adjacency info in vertices
	for (int i=0; i<3; ++i)
		v[i]->prep_remove(this);

	lmap_layer				= NULL;
}

#define VPUSH(a) a.x,a.y,a.z
template<>
void Face::	Failure		()
{
	dwInvalidFaces			++;

	clMsg		("* ERROR: Invalid face. (A=%f,e0=%f,e1=%f,e2=%f)",
		CalcArea(),
		v[0]->P.distance_to(v[1]->P),
		v[0]->P.distance_to(v[2]->P),
		v[1]->P.distance_to(v[2]->P)
		);
	clMsg		("*        v0[%f,%f,%f], v1[%f,%f,%f], v2[%f,%f,%f]",
		VPUSH(v[0]->P),
		VPUSH(v[1]->P),
		VPUSH(v[2]->P)
		);
	inlc_global_data()->err_invalid().w_fvector3	(v[0]->P);
	inlc_global_data()->err_invalid().w_fvector3	(v[1]->P);
	inlc_global_data()->err_invalid().w_fvector3	(v[2]->P);
}

void	Face::Verify		()
{
	// 1st :: area
	float	_a	= CalcArea();
	if		(!_valid(_a) || (_a<EPS))		{ Failure(); return; }

	// 2nd :: TC0
	Fvector2*	tc			= getTC0();
	float	eps				= .5f / 4096.f;		// half pixel from 4096 texture (0.0001220703125)
	float	e0				= tc[0].distance_to(tc[1]);	
	float	e1				= tc[1].distance_to(tc[2]);
	float	e2				= tc[2].distance_to(tc[0]);
	float	p				= e0+e1+e2;
	if		(!_valid(_a) || (p<eps))		{ Failure(); return; }

	// 3rd :: possibility to calc normal
	CalcNormal				();
	if (!_valid(N))			{ Failure(); return; }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int			affected	= 0;
void		start_unwarp_recursion()
{
	affected				= 1;
}
void Face::OA_Unwarp()
{
	if (pDeflector)					return;
	if (!Deflector->OA_Place(this))	return;
	
	// now iterate on all our neigbours
	for (int i=0; i<3; ++i) 
		for (vecFaceIt it=v[i]->m_adjacents.begin(); it!=v[i]->m_adjacents.end(); ++it) 
		{
			affected		+= 1;
			(*it)->OA_Unwarp();
		}
}


BOOL	DataFace::RenderEqualTo	(Face *F)
{
	if (F->dwMaterial	!= dwMaterial		)	return FALSE;
	//if (F->tc.size()	!= F->tc.size()		)	return FALSE;	// redundant???
	return TRUE;
}



void	DataFace::AddChannel	(Fvector2 &p1, Fvector2 &p2, Fvector2 &p3) 
{
	_TCF	TC;
	TC.uv[0] = p1;	TC.uv[1] = p2;	TC.uv[2] = p3;
	tc.push_back(TC);
}

BOOL	DataFace::hasImplicitLighting()
{
	if (0==this)								return FALSE;
	if (!Shader().flags.bRendering)				return FALSE;
	VERIFY( inlc_global_data() );
	b_material& M		= inlc_global_data()->materials()		[dwMaterial];
	b_BuildTexture&	T	= inlc_global_data()->textures()		[M.surfidx];
	return (T.THM.flags.test(STextureParams::flImplicitLighted));
}


/*	
	Fvector					N;				// face normal

	svector<_TCF,2>			tc;				// TC

	void*					pDeflector;		// does the face has LM-UV map?
	CLightmap*				lmap_layer;
	u32						sm_group;
*/


/*
BOOL	exact_normalize	(Fvector3& a)	{	return exact_normalize(&a.x);	}
BOOL	exact_normalize (float* a)
{
	double	sqr_magnitude	= a[0]*a[0] + a[1]*a[1] + a[2]*a[2];
	double	epsilon			= 1.192092896e-05F;
	if		(sqr_magnitude > epsilon)
	{
		double	l	=	rsqrt(sqr_magnitude);
		a[0]		*=	l;
		a[1]		*=	l;
		a[2]		*=	l;
		return		TRUE;
	}

	double a0,a1,a2,aa0,aa1,aa2,l;
	a0 = a[0];
	a1 = a[1];
	a2 = a[2];
	aa0 = _abs(a0);
	aa1 = _abs(a1);
	aa2 = _abs(a2);
	if (aa1 > aa0) {
		if (aa2 > aa1) {
			goto aa2_largest;
		}
		else {		// aa1 is largest
			a0 /= aa1;
			a2 /= aa1;
			l = rsqrt (a0*a0 + a2*a2 + 1);
			a[0] = a0*l;
			a[1] = (double)_copysign(l,a1);
			a[2] = a2*l;
		}
	}
	else {
		if (aa2 > aa0) {
aa2_largest:	// aa2 is largest
			a0 /= aa2;
			a1 /= aa2;
			l = rsqrt (a0*a0 + a1*a1 + 1);
			a[0] = a0*l;
			a[1] = a1*l;
			a[2] = (double)_copysign(l,a2);
		}
		else {		// aa0 is largest
			if (aa0 <= 0) {
				// dDEBUGMSG ("vector has zero size"); ... this messace is annoying
				a[0] = 0;	// if all a's are zero, this is where we'll end up.
				a[1] = 1;	// return a default unit length vector.
				a[2] = 0;
				return	FALSE;
			}
			a1 /= aa0;
			a2 /= aa0;
			l = rsqrt (a1*a1 + a2*a2 + 1);
			a[0] = (double)_copysign(l,a0);
			a[1] = a1*l;
			a[2] = a2*l;
		}
	}
	return	TRUE;
}
*/


void	DataFace::	read	(INetReader	&r )
{
	base_Face::read( r );	

	r.r_fvector3( N );			
	r_vector ( r, tc ) ;			
	pDeflector =0 ;
	VERIFY( read_lightmaps );
	read_lightmaps->read( r, lmap_layer );
	sm_group = r.r_u32();

}
void	DataFace::	write	(IWriter	&w )const
{
	base_Face::write( w );
	w.w_fvector3( N );			
	w_vector ( w, tc ) ;			
	VERIFY( write_lightmaps );
	write_lightmaps->write( w, lmap_layer );
	w.w_u32( sm_group );
}
void	DataVertex::	read	(INetReader	&r )
{
	base_Vertex::read( r );

}
void	DataVertex::	write	(IWriter	&w )const
{
	base_Vertex::write( w );
}

void	Face::	read	( INetReader	&r )
{
	DataFace::read( r );
	VERIFY( read_vertices );
	read_vertices->read( r, v[0] );
	read_vertices->read( r, v[1] );
	read_vertices->read( r, v[2] );
}

void	Face::	write	( IWriter	&w )const
{
	DataFace::write( w );
	VERIFY( write_vertices );
	write_vertices->write( w, v[0] );
	write_vertices->write( w, v[1] );
	write_vertices->write( w, v[2] );
}

void	Vertex::read		( INetReader	&r )
{
	//	v_faces							m_adjacents; !
	DataVertex::read( r );
}
void	Vertex::write		( IWriter	&w )const
{
	//	v_faces							m_adjacents; !
	DataVertex::write( w );
}
