#include "common.h"

out gl_PerVertex { vec4 gl_Position; };

layout(location = 0) in vec4	P;		// (float,float,float,1)
layout(location = 1) in vec4	norm;	// (nx,ny,nz,hemi occlusion)
layout(location = 2) in vec4	tan;	// tangent
layout(location = 3) in vec4	bnorm;	// binormal
layout(location = 4) in vec2	tc;		// (u,v)
#if	defined(USE_LM_HEMI)
layout(location = 5) in vec2	lm;		// (lmu,lmv)
#elif defined(USE_R2_STATIC_SUN)
layout(location = 5) in vec4	color;	// (r,g,b,dir-occlusion)	//	Swizzle before use!!!
#endif

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
	vec4 Nh			= unpack_D3DCOLOR(norm);
	vec4 T			= unpack_D3DCOLOR(tan);
	vec4 B			= unpack_D3DCOLOR(bnorm);

	// Eye-space pos/normal
	float4	Pp 	= mul( m_WVP, P );
	gl_Position	= Pp;
	N 			= mul( m_WV, unpack_bx2(Nh.rgb) ).xyz;
	float3	Pe	= mul( m_WV, P.xyz ).xyz;

	float2	tc 	= unpack_tc_base( tc, T.w, B.w);	// copy tc
	position	= float4( Pe, Nh.w );

#if defined(USE_R2_STATIC_SUN) && !defined(USE_LM_HEMI)
	float 	s	= color.w	;							// (r,g,b,dir-occlusion)
	tcdh		= float4( tc.xyyy );
	tcdh.w		= s;
#else
	tcdh		= tc.xy;
#endif

#ifdef	USE_TDETAIL
	tcdbump		= tcdh * dt_params;					// dt tc
#endif

#ifdef	USE_LM_HEMI
	lmh 		= unpack_tc_lmap( lm );
#endif
}
