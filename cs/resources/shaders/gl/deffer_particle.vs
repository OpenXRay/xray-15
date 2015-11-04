#include 	"common.h"

out gl_PerVertex { vec4 gl_Position; };

layout(location = POSITION) in vec4	P;
layout(location = TEXCOORD0) in vec2	tc;
layout(location = COLOR0) in vec4	c;

#if defined(USE_R2_STATIC_SUN) && !defined(USE_LM_HEMI)
layout(location = TEXCOORD0) out vec4	tcdh;		// Texture coordinates,         w=sun_occlusion
#else
layout(location = TEXCOORD0) out vec2	tcdh;		// Texture coordinates
#endif
layout(location = TEXCOORD1) out vec4	position;	// position + hemi
layout(location = TEXCOORD2) out vec3	N;			// Eye-space normal        (for lighting)
#ifdef USE_TDETAIL
layout(location = TEXCOORD3) out vec2	tcdbump;	// d-bump
	#ifdef USE_LM_HEMI
layout(location = TEXCOORD4) out vec2	lmh;		// lm-hemi
	#endif
#else
	#ifdef USE_LM_HEMI
layout(location = TEXCOORD3) out vec2	lmh;		// lm-hemi
	#endif
#endif
layout(location = COLOR0) out vec4 	color;

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
