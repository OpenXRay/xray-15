#ifndef r_constantsH
#define r_constantsH
#pragma once

#include "../../xrcore/xr_resource.h"


#ifdef	USE_DX10
#include "../xrRenderDX10/dx10ConstantBuffer.h"
#endif	//	USE_DX10


class  ECORE_API	R_constant_setup;

enum
{
	RC_float		= 0,
	RC_int			= 1,
	RC_bool			= 2,
	RC_sampler		= 99,	//	DX9 shares index for sampler and texture
	RC_dx10texture	= 100	//	For DX10 sampler and texture are different resources
};
enum
{
	RC_1x1		= 0,					// vector1, or scalar
	RC_1x4,								// vector4
	RC_1x3,								// vector3
	RC_1x2,								// vector2
	RC_2x4,								// 4x2 matrix, transpose
	RC_3x4,								// 4x3 matrix, transpose
	RC_4x4,								// 4x4 matrix, transpose
	RC_1x4a,							// array: vector4
	RC_3x4a,							// array: 4x3 matrix, transpose
	RC_4x4a								// array: 4x4 matrix, transpose
};
enum
{
	//	Don't change this since some code relies on magic numbers
	RC_dest_pixel					= (1<<0),
	RC_dest_vertex					= (1<<1),
	RC_dest_sampler					= (1<<2),	//	For DX10 it's either sampler or texture
	RC_dest_geometry				= (1<<3),	//	DX10 only
	RC_dest_pixel_cb_index_mask		= 0xF000,	//	Buffer index == 0..14
	RC_dest_pixel_cb_index_shift	= 12,
	RC_dest_vertex_cb_index_mask	= 0x0F00,	//	Buffer index == 0..14
	RC_dest_vertex_cb_index_shift	= 8,
	RC_dest_geometry_cb_index_mask	= 0x00F0,	//	Buffer index == 0..14
	RC_dest_geometry_cb_index_shift	= 4,
};

enum	//	Constant buffer index masks
{
	CB_BufferIndexMask		= 0xF,	//	Buffer index == 0..14

	CB_BufferTypeMask		= 0x30,
	CB_BufferPixelShader	= 0x10,
	CB_BufferVertexShader	= 0x20,
	CB_BufferGeometryShader	= 0x30
};

struct ECORE_API	R_constant_load
{
	u16						index;		// linear index (pixel)
	u16						cls;		// element class
#ifdef USE_OGL
	GLuint					location;
	GLuint					program;

	R_constant_load() : index(u16(-1)), cls(u16(-1)), location(0), program(0) {};
#else
	R_constant_load() : index(u16(-1)), cls(u16(-1)) {};
#endif // USE_OGL

	IC BOOL					equal		(R_constant_load& C)
	{
#ifdef USE_OGL
		return (index==C.index) && (cls == C.cls) && (location == C.location) && (program == C.program);
#else
		return (index==C.index) && (cls == C.cls);
#endif // USE_OGL
	}
};

struct ECORE_API	R_constant			:public xr_resource
{
	shared_str				name;		// HLSL-name
	u16						type;		// float=0/integer=1/boolean=2
	u16						destination;// pixel/vertex/(or both)/sampler

	R_constant_load			ps;
	R_constant_load			vs;
#ifdef	USE_DX10
	R_constant_load			gs;
#endif	//	USE_DX10
	R_constant_load			samp;
	R_constant_setup*		handler;

	R_constant() : type(u16(-1)), destination(0), handler(NULL) { };

	IC BOOL					equal		(R_constant& C)
	{
		return (0 == xr_strcmp(name, C.name)) && (type == C.type) && (destination == C.destination) && ps.equal(C.ps) && vs.equal(C.vs) && samp.equal(C.samp) && handler == C.handler;
	}
	IC BOOL					equal		(R_constant* C)
	{
		return equal(*C);
	}
};
typedef	resptr_core<R_constant,resptr_base<R_constant> > ref_constant;

// Automatic constant setup
class	 ECORE_API			R_constant_setup
{
public:
	virtual void			setup		(R_constant* C)	= 0;
	virtual ~R_constant_setup () {}
};

#ifdef USE_OGL
class cl_sampler : public R_constant_setup		{ virtual void setup(R_constant* C); };
#endif // USE_OGL

class	 ECORE_API			R_constant_table	: public xr_resource_flagged	{
public:
	typedef xr_vector<ref_constant>		c_table;
	c_table					table;
#ifdef USE_OGL
	cl_sampler				sampler_binder;
#endif // USE_OGL

#ifdef	USE_DX10
	typedef std::pair<u32,ref_cbuffer>	cb_table_record;
	typedef xr_vector<cb_table_record>	cb_table;
	cb_table							m_CBTable;
#endif	//	USE_DX10
private:
	void					fatal		(LPCSTR s);

#ifdef	USE_DX10
	BOOL					parseConstants(ID3D10ShaderReflectionConstantBuffer* pTable, u16 destination);
	BOOL					parseResources(ID3D10ShaderReflection* pReflection, int ResNum, u16 destination);
#endif	//	USE_DX10

public:
	~R_constant_table					();

	void					clear		();
	BOOL					parse		(void* desc, u16 destination);
	void					merge		(R_constant_table* C);
	ref_constant			get			(LPCSTR		name);		// slow search
	ref_constant			get			(shared_str&	name);		// fast search

	BOOL					equal		(R_constant_table& C);
	BOOL					equal		(R_constant_table* C)	{	return equal(*C);		}
	BOOL					empty		()						{	return 0==table.size();	}
private:

};
typedef	resptr_core<R_constant_table,resptr_base<R_constant_table> >				ref_ctable;

#ifdef	USE_DX10
#include "../xrRenderDX10/dx10ConstantBuffer_impl.h"
#endif	//	USE_DX10

#endif
