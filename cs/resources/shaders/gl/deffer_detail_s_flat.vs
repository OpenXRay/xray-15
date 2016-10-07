#include "common.h"

uniform float4 		consts; // {1/quant,1/quant,diffusescale,ambient}
uniform float4 		array[61*4];

out gl_PerVertex { vec4 gl_Position; };

layout(location = POSITION) in vec4 P;	// (float,float,float,1)
layout(location = TEXCOORD0) in vec4 misc;	// (u(Q),v(Q),frac,matrix-id)

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

void 	main ()
{
	// index
	int 	i 	= int(misc.w);
	float4  m0 	= array[i+0];
	float4  m1 	= array[i+1];
	float4  m2 	= array[i+2];
	float4  c0 	= array[i+3];

	// Transform pos to world coords
	float4 	pos;
 	pos.x 		= dot	(m0, P);
 	pos.y 		= dot	(m1, P);
 	pos.z 		= dot	(m2, P);
	pos.w 		= 1;

	// Normal in world coords
	float3 	norm;	
		norm.x 	= pos.x - m0.w	;
		norm.y 	= pos.y - m1.w	+ .75f;	// avoid zero
		norm.z	= pos.z - m2.w	;

	// Final out
	float4	Pp 	= mul		(m_WVP,	pos				);
	gl_Position	= Pp;
	N	 		= mul		(m_WV,  normalize(norm)	);
	float3	Pe	= mul		(m_WV,  pos				);

# if defined(USE_R2_STATIC_SUN)
	tcdh 		= (misc * consts).xyyy;
	tcdh.w	= c0.x;								// (,,,dir-occlusion)
# else
	tcdh 		= (misc * consts).xy;
# endif

	position	= float4	(Pe, 		c0.w		);
}
