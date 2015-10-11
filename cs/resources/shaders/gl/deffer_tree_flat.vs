#include "common.h"

out gl_PerVertex { vec4 gl_Position; };

uniform float3x4	m_xform;
uniform float3x4	m_xform_v;
uniform float4 		consts; 	// {1/quant,1/quant,???,???}
uniform float4 		c_scale,c_bias,wind,wave;
uniform float2 		c_sun;		// x=*, y=+

layout(location = 0) in vec4	P;		// (float,float,float,1)
layout(location = 1) in vec4	norm;	// (nx,ny,nz)
layout(location = 2) in vec4	tan;	// tangent
layout(location = 3) in vec4	bnorm;	// binormal
layout(location = 4) in vec4	tc;		// (u,v,frac,???)

#if defined(USE_R2_STATIC_SUN) && !defined(USE_LM_HEMI)
layout(location = 0) out vec4	tcdh;		// Texture coordinates,         w=sun_occlusion
#else
layout(location = 0) out vec2	tcdh;		// Texture coordinates
#endif
layout(location = 1) out vec4	position;	// position + hemi
layout(location = 2) out vec3	N;			// Eye-space normal        (for lighting)
#ifdef USE_TDETAIL
layout(location = 3) out vec2	tcdbump;	// d-bump
	#ifdef USE_LM_HEMI
layout(location = 4) out vec2	lmh;		// lm-hemi
	#endif
#else
	#ifdef USE_LM_HEMI
layout(location = 3) out vec2	lmh;		// lm-hemi
	#endif
#endif

void main ()
{
	vec4 Nh	=	unpack_D3DCOLOR(norm);
	vec4 T	=	unpack_D3DCOLOR(tan);
	vec4 B	=	unpack_D3DCOLOR(bnorm);

	// Transform to world coords
	float3 	pos		= mul		(m_xform, P);

	//
	float 	base 	= m_xform._24;			// take base height from matrix
	float 	dp		= calc_cyclic  (wave.w+dot(pos,wave.xyz));
	float 	H 		= pos.y - base;			// height of vertex (scaled, rotated, etc.)
	float 	frac 	= tc.z*consts.x;		// fractional (or rigidity)
	float 	inten 	= H * dp;			// intensity
	float2 	result	= calc_xz_wave	(wind.xz*inten, frac);
#ifdef		USE_TREEWAVE
			result	= 0;
#endif
	float4 	f_pos 	= float4(pos.x+result.x, pos.y, pos.z+result.y, 1);

	// Final xform(s)
	// Final xform
	float3	Pe		= mul		(m_V,  f_pos				);
	float 	hemi 	= Nh.w*c_scale.w + c_bias.w;
    //float 	hemi 	= Nh.w;
	gl_Position		= mul		(m_VP, f_pos				);
	N 				= mul		(m_xform_v, unpack_bx2(Nh.rgb)	);
	position		= float4	(Pe, hemi					);

#if defined(USE_R2_STATIC_SUN) && !defined(USE_LM_HEMI)
	float 	suno 	= Nh.w * c_sun.x + c_sun.y	;
	tcdh 			= (tc * consts).xyyy;
	tcdh.w			= suno;					// (,,,dir-occlusion)
#else
	tcdh 			= (tc * consts).xy;
#endif

#ifdef USE_TDETAIL
	tcdbump	= tcdh*dt_params.xy;					// dt tc
#endif
}
