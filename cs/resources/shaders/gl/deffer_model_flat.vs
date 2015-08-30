#include "common.h"
#include "skin.h"

out gl_PerVertex { vec4 gl_Position; };

layout(location = 0) in ivec4 P;		// (float,float,float,1)
#ifdef 	SKIN_0
layout(location = 1) in vec3 norm;	// (nx,ny,nz)
#else
layout(location = 1) in vec4 norm;	// (nx,ny,nz)
#endif
#if defined(SKIN_3) || defined(SKIN_4)
layout(location = 2) in vec4 tan;	// (nx,ny,nz)
layout(location = 3) in vec4 bnorm;	// (nx,ny,nz)
#else
layout(location = 2) in vec3 tan;	// (nx,ny,nz)
layout(location = 3) in vec3 bnorm;	// (nx,ny,nz)
#endif
#if defined(SKIN_2) || defined(SKIN_3)
layout(location = 4) in ivec4 tc;	// (u,v)
#else
layout(location = 4) in ivec2 tc;	// (u,v)
#endif
#ifdef 	SKIN_4
layout(location = 5) in vec4 ind;
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

void _main( v_model I )
{
	// world-space  N
	float3 	N_w	= mul( m_W, I.N ).xyz;

	// Eye-space pos/normal
	float3	Pe	= mul( m_WV, I.P.xyz ).xyz;
	gl_Position	= mul( m_WVP, I.P );
	N	 		= mul( m_WV, I.N ).xyz;

	position	= float4( Pe, L_material.x );

#if defined(USE_R2_STATIC_SUN) && !defined(USE_LM_HEMI)
	tcdh 		= float4( I.tc.xyyy	);
	tcdh.w		= L_material.y;							// (,,,dir-occlusion)
#else
	tcdh 		= I.tc.xy;
#endif

#ifdef USE_TDETAIL
	tcdbump		= tcdh*dt_params;					// dt tc
#endif
}

/////////////////////////////////////////////////////////////////////////
void	main()
{
#ifdef 	SKIN_NONE
	v_model v;
#endif
#ifdef 	SKIN_0
	v_model_skinned_0 v;
#endif
#ifdef	SKIN_1
	v_model_skinned_1 v;
#endif
#ifdef	SKIN_2
	v_model_skinned_2 v;
#endif
#ifdef	SKIN_3
	v_model_skinned_3 v;
#endif
#ifdef	SKIN_4
	v_model_skinned_4 v;
	v.ind = ind;
#endif

	v.P = P;
	v.N = norm;
	v.T = tan;
	v.B = bnorm;
	v.tc = tc;

#ifdef 	SKIN_NONE
	_main(v);
#endif

#ifdef 	SKIN_0
	_main(skinning_0(v));
#endif

#ifdef	SKIN_1
	_main(skinning_1(v));
#endif

#ifdef	SKIN_2
	_main(skinning_2(v));
#endif

#ifdef	SKIN_3
	_main(skinning_3(v));
#endif

#ifdef	SKIN_4
	_main(skinning_4(v));
#endif
}
