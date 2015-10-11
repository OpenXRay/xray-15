#include 	"common.h"

out gl_PerVertex { vec4 gl_Position; };

layout(location = 0) in vec4	P;
layout(location = 1) in vec2	tc;
layout(location = 2) in vec4	c;

layout(location = 0) out vec4 	color;
#if defined(USE_R2_STATIC_SUN) && !defined(USE_LM_HEMI)
layout(location = 1) out vec4	tcdh;		// Texture coordinates,         w=sun_occlusion
#else
layout(location = 1) out vec2	tcdh;		// Texture coordinates
#endif
layout(location = 2) out vec4	position;	// position + hemi
layout(location = 3) out vec3	N;			// Eye-space normal        (for lighting)
#ifdef USE_TDETAIL
layout(location = 4) out vec2	tcdbump;	// d-bump
	#ifdef USE_LM_HEMI
layout(location = 5) out vec2	lmh;		// lm-hemi
	#endif
#else
	#ifdef USE_LM_HEMI
layout(location = 4) out vec2	lmh;		// lm-hemi
	#endif
#endif

void main()
{
	float4 	w_pos 	= P;

	// Eye-space pos/normal
	gl_Position	= mul		(m_WVP,		w_pos	);
	N 			= normalize (eye_position-w_pos.xyz	);
	float3	Pe	= mul		(m_WV, 		P		);
#if defined(USE_R2_STATIC_SUN) && !defined(USE_LM_HEMI)
	tcdh 		= float4	(tc.xyyy			);
#else
	tcdh 		= tc.xy;
#endif
	position	= float4	(Pe, 		.2f		);

#ifdef 	USE_TDETAIL
	tcdbump		= tcdh * dt_params.xy;			// dt tc
#endif

	color = c;
}
