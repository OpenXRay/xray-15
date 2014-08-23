#include "stdafx.h"

#include "xrLC_GlobalData.h"
#include "xrface.h"
#include "xrdeflector.h"
#include "lightmap.h"
#include "serialize.h"
#include "mu_model_face.h"
#include "xrmu_model.h"
#include "xrmu_model_reference.h"
#include "../../xrcdb/xrcdb.h"

bool g_using_smooth_groups = true;
bool g_smooth_groups_by_faces = false;

xrLC_GlobalData* data =0;

tread_lightmaps		*read_lightmaps		= 0;
twrite_lightmaps	*write_lightmaps	= 0;

twrite_faces		*write_faces		= 0;
tread_faces			*read_faces			= 0;
tread_vertices		*read_vertices		= 0;
twrite_vertices		*write_vertices		= 0;
tread_deflectors	*read_deflectors	= 0;
twrite_deflectors	*write_deflectors	= 0;

tread_models		*read_models		= 0;
twrite_models		*write_models		= 0;

 xrLC_GlobalData*	lc_global_data()
 {
	 return data;
 }
void	create_global_data()
{
	VERIFY( !inlc_global_data() );
	data = xr_new<xrLC_GlobalData>();
}
void	destroy_global_data()
{
	VERIFY( inlc_global_data() );
	if(data)
		data->clear();
	xr_delete(data);
}

void xrLC_GlobalData	::create_write_faces()
{
	VERIFY(!write_faces);
	write_faces = xr_new< twrite_faces	>( &_g_faces );
}
void xrLC_GlobalData::destroy_write_faces()
{
	
	xr_delete(write_faces);
}
void xrLC_GlobalData	::create_read_faces()
{
	VERIFY(!read_faces);
	read_faces = xr_new< tread_faces	>( &_g_faces );
}
void xrLC_GlobalData::destroy_read_faces()
{
	
	xr_delete(read_faces);
}
/*
poolVertices &xrLC_GlobalData	::VertexPool	()		
{
	return	_VertexPool; 
}
poolFaces &xrLC_GlobalData	::FacePool			()		
{
	return	_FacePool;
}
*/




void	xrLC_GlobalData	::destroy_rcmodel	()
{
	xr_delete		(_RCAST_Model);
}
void	xrLC_GlobalData	::create_rcmodel	(CDB::CollectorPacked& CL)
{
	VERIFY(!_RCAST_Model);
	_RCAST_Model				= xr_new<CDB::MODEL> ();
	_RCAST_Model->build		(CL.getV(),(int)CL.getVS(),CL.getT(),(int)CL.getTS());
}

void		xrLC_GlobalData	::				initialize		()
{
	if (strstr(Core.Params,"-att"))	_gl_linear	= true;
}


/*
		xr_vector<b_BuildTexture>		_textures;
		xr_vector<b_material>			_materials;
		Shader_xrLC_LIB					_shaders;				
		CMemoryWriter					_err_invalid;
		b_params						_g_params;
		vecVertex						_g_vertices;
		vecFace							_g_faces;
		vecDefl							_g_deflectors;
		base_lighting					_L_static;
		CDB::MODEL*						_RCAST_Model;
		bool							_b_nosun;
		bool							_gl_linear;
*/

//void			xrLC_GlobalData	::				cdb_read_create	() 
//{
//	VERIFY(!_RCAST_Model);
//	_RCAST_Model = xr_new<CDB::MODEL> ();
//	_RCAST_Model->build( &*verts.begin(), (int)verts.size(), &*tris.begin(), (int)tris.size() );
//}

//base_Face* F		= (base_Face*)(*((void**)&T.dummy));

//*((u32*)&F)

base_Face* convert_nax( u32 dummy )
{
	return (base_Face*)(*((void**)&dummy));
}

u32 convert_nax( base_Face* F )
{
	return *((u32*)&F);
}

void write( IWriter	&w, const CDB::TRI &tri )
{
	w.w_u32( tri.verts[ 0 ] );
	w.w_u32( tri.verts[ 1 ] );
	w.w_u32( tri.verts[ 2 ] );
	const base_Face* F=  convert_nax( tri.dummy );
	VERIFY( inlc_global_data() );
	inlc_global_data()->write( w, F );
}

void read( INetReader	&r, CDB::TRI &tri )
{
	tri.verts[ 0 ]  = r.r_u32( );
	tri.verts[ 1 ]  = r.r_u32( );
	tri.verts[ 2 ]  = r.r_u32( );
	VERIFY( inlc_global_data() );
	base_Face* F = 0;
	inlc_global_data()->read( r, F );
	tri.dummy = convert_nax( F );
}

static xr_vector<Fvector> verts;
static xr_vector<CDB::TRI> tris;

void read( INetReader	&r, CDB::MODEL* &m )
{
	
	verts.clear();
	tris.clear();
	r_pod_vector( r, verts );

	u32 tris_count = r.r_u32();
	tris.resize( tris_count );
	for( u32 i = 0; i < tris_count; ++i)
		read( r, tris[i] );

	VERIFY(!m);
	m = xr_new<CDB::MODEL> ();
	m->build( &*verts.begin(), (int)verts.size(), &*tris.begin(), (int)tris.size() );
	verts.clear();
	tris.clear();
}

void write( IWriter	&w,  CDB::MODEL &m )
{
	w.w_u32( (u32)m.get_verts_count() );
	w.w( m.get_verts(), m.get_verts_count() * sizeof(Fvector) );
	
	u32 tris_count = (u32) m.get_tris_count() ;
	w.w_u32( tris_count );
	for( u32 i = 0; i < tris_count; ++i)
		write( w, m.get_tris()[i] );

//	w.w( m.get_tris(), m.get_tris_count() * sizeof(CDB::TRI) );
}

void		xrLC_GlobalData	::read			( INetReader	&r )
{
	

	_b_nosun = !!r.r_u8();
	_gl_linear = !!r.r_u8();
	r_pod( r, _g_params );
	
	_L_static.read( r );

	
	r_vector( r, _textures );
	r_pod_vector( r, _materials );
	r_pod_vector( r, _shaders.Library	() );	
	//	CMemoryWriter					_err_invalid;


	read_lightmaps= xr_new< tread_lightmaps >( &_g_lightmaps );
	read_lightmaps->read( r );

	read_vertices = xr_new< tread_vertices	>( &_g_vertices );
	read_vertices->read( r );

	read_faces = xr_new< tread_faces	>( &_g_faces );
	read_faces->read( r );

	read_deflectors = xr_new< tread_deflectors	>( &_g_deflectors );
	read_deflectors->read( r );

	read_models =  xr_new< tread_models	>( &_mu_models );
	read_models->read( r );
	
	::read( r, _RCAST_Model );

	close_models_read( );

	xr_delete( read_lightmaps );
	xr_delete( read_vertices );
	xr_delete( read_faces );
	xr_delete( read_deflectors );

}

bool			xrLC_GlobalData	::			b_r_vertices	()		
{
	return !!read_vertices;
}

bool			xrLC_GlobalData	::			b_r_faces		()		
{
	return !!read_faces;
}

void		xrLC_GlobalData	::				write			( IWriter	&w ) const
{
	

	w.w_u8(_b_nosun);
	w.w_u8(_gl_linear);
	w_pod( w, _g_params );

	_L_static.write( w );

	
	
	w_vector( w, _textures );
	w_pod_vector( w, _materials );
	w_pod_vector( w, _shaders.Library	() );	
	//	CMemoryWriter					_err_invalid;


	write_lightmaps= xr_new< twrite_lightmaps >( &_g_lightmaps );
	write_lightmaps->write( w );

	write_vertices = xr_new< twrite_vertices	>( &(_g_vertices) );
	write_vertices->write( w );

	write_faces = xr_new< twrite_faces	>( &_g_faces );
	write_faces->write( w );

	write_deflectors = xr_new< twrite_deflectors	>( &_g_deflectors );
	write_deflectors->write( w );

	write_models =  xr_new< twrite_models	>( &_mu_models );
	write_models ->write( w );

	::write( w, *_RCAST_Model);

	close_models_write( );
	xr_delete( write_lightmaps );
	xr_delete( write_vertices );
	xr_delete( write_faces );
	xr_delete( write_deflectors );

}
void			xrLC_GlobalData	::			close_models_read		()
{
	xr_vector<xrMU_Model*> :: iterator i = _mu_models.begin() , e = _mu_models.end();
	for( ; e!= i; ++i )
		(*i)->reading_close();
}
void			xrLC_GlobalData	::			close_models_write		()const
{
	xr_vector<xrMU_Model*> :: const_iterator i = _mu_models.begin() , e = _mu_models.end();
	for( ; e!= i; ++i )
		(*i)->writting_close();
}

template<typename T>
std::pair<u32,u32>	get_id( const xr_vector<xrMU_Model*>& mu_models, const T * v )
{


	u32 face_id = u32(-1);
	struct find
	{
		const T * _v;
		u32& _id;
		find( const T * v, u32& id) : _v(v), _id( id )
		{}
		bool operator () ( const xrMU_Model * m )
		{	
			VERIFY(m);
			u32 id = m->find( _v );
			if( id == u32(-1) )
				return false;
			_id = id;
			return true;
		}
	} f( v, face_id );

	xr_vector<xrMU_Model*> :: const_iterator ii =std::find_if( mu_models.begin(), mu_models.end(), f );
	if( ii == mu_models.end() )
		return std::pair<u32,u32>(u32(-1), u32(-1));
	return std::pair<u32,u32>(u32(ii-mu_models.begin()), face_id );
}

//std::pair<u32,u32>			xrLC_GlobalData	::		get_id		( const _face * v ) const
//{
//	return ::get_id( _mu_models, v );
//}
//
//std::pair<u32,u32>			xrLC_GlobalData	::		get_id		( const _vertex * v ) const
//{
//	return ::get_id( _mu_models, v );
//}
enum serialize_mesh_item_type
{
	smit_plain = u8(0),
	smit_model = u8(1),
	smit_null  = u8(-1)
};
void			xrLC_GlobalData	::	read			( INetReader &r, base_Face* &f )
{
	VERIFY(!f);
	u8 type  = r.r_u8( );

	switch( type  )
	{
		case smit_plain:
			{
				VERIFY(read_faces);
				Face * face = 0;
				read_faces->read( r, face );
				f = face;
				return;
			}
		case smit_model:
			{
				u32 model_id = r.r_u32();
				_face *model_face = 0;
				_mu_models[ model_id ]->read( r, model_face );
				f = model_face;
				return;
			}
		case smit_null: 
			return;
	}
}

void xrLC_GlobalData	::	write( IWriter &w, const base_Face *f ) const 
{
	
	if(!f)
	{
		w.w_u8( smit_null );
		return;
	}

	const Face *face = dynamic_cast<const Face*>( f );
	if(face)
	{
		VERIFY( write_faces );
		w.w_u8( smit_plain );
		write_faces->write( w, face );
		return;
	}

	const _face *model_face = dynamic_cast<const _face*>( f );
	VERIFY(model_face);

	w.w_u8( smit_model );

	std::pair<u32,u32> id = get_id( _mu_models, model_face );
	
	w.w_u32( id.first );

	_mu_models[ id.first ]->write( w, id.second, model_face  );

}

xrLC_GlobalData::~xrLC_GlobalData()
{
	//u32 i;
	//i++;
}

template<typename T>
void vec_clear( xr_vector<T*> &v )
{
	typename xr_vector<T*>::iterator i = v.begin(), e = v.end();
	for(;i!=e;++i)
		xr_delete(*i);
	v.clear();
}
template<typename T>
void vec_spetial_clear( xr_vector<T> &v )
{
	typename xr_vector<T>::iterator i = v.begin(), e = v.end();
	for(;i!=e;++i)
		clear(*i);
	v.clear();
}

void mu_mesh_clear();
void		xrLC_GlobalData::				clear			()
{
		vec_spetial_clear(_textures );
		_materials.clear();
		_shaders.Unload();
	//	CMemoryWriter					_err_invalid;
	//	b_params						_g_params;
		vec_clear(_g_lightmaps);
		vec_clear(_mu_models);//mem leak
		vec_clear(_mu_refs);
		mu_mesh_clear();
		gl_mesh_clear();
		//VertexPool;
		//FacePool;

	

	//	vecVertex						_g_vertices;
	//	vecFace							_g_faces;
		gl_mesh_clear	();
	    vec_clear		(_g_deflectors);

		//base_lighting					_L_static;
		xr_delete(_RCAST_Model);

//		bool							_b_nosun;
//		bool							_gl_linear;
}