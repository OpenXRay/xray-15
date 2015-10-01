#include "common.h"

out gl_PerVertex { vec4 gl_Position; };

uniform float3x4	m_xform		;
uniform float3x4	m_xform_v	;
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
layout(location = 2) out vec3	M1;			// nmap 2 eye - 1
layout(location = 3) out vec3	M2;			// nmap 2 eye - 2
layout(location = 4) out vec3	M3;			// nmap 2 eye - 3
#if defined(USE_PARALLAX) || defined(USE_STEEPPARALLAX)
layout(location = 5) out vec3	eye;		// vector to point in tangent space
  #ifdef USE_TDETAIL
layout(location = 6) out vec2	tcdbump;	// d-bump
    #ifdef USE_LM_HEMI
layout(location = 7) out vec2	lmh;		// lm-hemi
    #endif
  #else
    #ifdef USE_LM_HEMI
layout(location = 6) out vec2	lmh;		// lm-hemi
    #endif
  #endif
#else
  #ifdef USE_TDETAIL
layout(location = 5) out vec2	tcdbump;	// d-bump
    #ifdef USE_LM_HEMI
layout(location = 6) out vec2	lmh;		// lm-hemi
    #endif
  #else
    #ifdef USE_LM_HEMI
layout(location = 5) out vec2	lmh;		// lm-hemi
    #endif
  #endif
#endif

void 	main 	()
{
	vec4 Nh	=	unpack_D3DCOLOR(norm);
	vec4 T	=	unpack_D3DCOLOR(tan);
	vec4 B	=	unpack_D3DCOLOR(bnorm);

	// Transform to world coords
	float3 	pos		= mul			(m_xform, P.xyz).xyz;

	//
	float 	base 	= m_xform._24	;		// take base height from matrix
	float 	dp		= calc_cyclic  	(wave.w+dot(pos,wave.xyz));
	float 	H 		= pos.y - base	;		// height of vertex (scaled, rotated, etc.)
	float 	frac 	= tc.z*consts.x;		// fractional (or rigidity)
	float 	inten 	= H * dp;				// intensity
	float2 	result	= calc_xz_wave	(wind.xz*inten, frac);
#ifdef		USE_TREEWAVE
			result	= 0;
#endif
	float4 	w_pos 	= float4(pos.x+result.x, pos.y, pos.z+result.y, 1);
	float2 	tc 		= (tc * consts).xy;
	float 	hemi 	= Nh.w * c_scale.w + c_bias.w;
//	float 	hemi 	= Nh.w;

	// Eye-space pos/normal
	float3	Pe		= mul		(m_V,  	w_pos.xyz	).xyz;
	gl_Position		= mul		(m_VP,	w_pos		);
	position		= float4	(Pe, 	hemi		);

#if defined(USE_R2_STATIC_SUN) && !defined(USE_LM_HEMI)
	float 	suno 	= Nh.w * c_sun.x + c_sun.y	;
	tcdh 			= tc.xyyy;
	tcdh.w			= suno;					// (,,,dir-occlusion)
#else
	tcdh 			= tc.xy;
#endif

	// Calculate the 3x3 transform from tangent space to eye-space
	// TangentToEyeSpace = object2eye * tangent2object
	//		     = object2eye * transpose(object2tangent) (since the inverse of a rotation is its transpose)
	float3 	N 		= unpack_bx4(Nh);	// just scale (assume normal in the -.5f, .5f)
	float3 	xT 		= unpack_bx4(T);	//
	float3 	xB 		= unpack_bx4(B);	//
	float3x3 xform	= mul	(float3x3(
						xT.x,xB.x,N.x,
						xT.y,xB.y,N.y,
						xT.z,xB.z,N.z
					), mat3(m_xform_v));

	// The pixel shader operates on the bump-map in [0..1] range
	// Remap this range in the matrix, anyway we are pixel-shader limited :)
	// ...... [ 2  0  0  0]
	// ...... [ 0  2  0  0]
	// ...... [ 0  0  2  0]
	// ...... [-1 -1 -1  1]
	// issue: strange, but it's slower :(
	// issue: interpolators? dp4? VS limited? black magic?

	// Feed this transform to pixel shader
	M1 			= xform[0];
	M2 			= xform[1];
	M3 			= xform[2];

#ifdef 	USE_PARALLAX
	eye 		= mul		(-(w_pos - eye_position), float3x3(T,B,N));
#endif

#ifdef 	USE_TDETAIL
	tcdbump		= tcdh * dt_params;		// dt tc
#endif
}
