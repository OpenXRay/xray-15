#include "common.h"

uniform float4 		consts; // {1/quant,1/quant,diffusescale,ambient}
uniform float4 		wave; 	// cx,cy,cz,tm
uniform float4 		dir2D; 
//uniform float4 		array	[200] : register(c12);
//tbuffer DetailsData
//{
	uniform float4 		array[61*4];
//}

out gl_PerVertex { vec4 gl_Position; };

layout(location = 0) in vec4 P;				// (float,float,float,1)
layout(location = 1) in ivec4 misc;			// (u(Q),v(Q),frac,matrix-id)

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

void 	main ()
{
	// index
	int 	i 	= misc.w;
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

	// 
	float 	base 	= m1.w;
	float 	dp		= calc_cyclic   (dot(pos,wave));
	float 	H 		= pos.y - base;			// height of vertex (scaled)
	float 	frac 	= misc.z*consts.x;	// fractional
	float 	inten 	= H * dp;
	float2 	result	= calc_xz_wave	(dir2D.xz*inten,frac);
	pos				= float4(pos.x+result.x, pos.y, pos.z+result.y, 1);

	// Normal in world coords
	float3 	norm;	//	= float3(0,1,0);
		norm.x 	= pos.x - m0.w	;
		norm.y 	= pos.y - m1.w	+ .75f;	// avoid zero
		norm.z	= pos.z - m2.w	;

	// Final out
	float4	Pp 	= mul		(m_WVP,	pos				);
	gl_Position	= Pp;
	N 			= mul		(m_WV,  normalize(norm)	).xyz;
	float3	Pe	= mul		(m_WV,  pos.xyz			).xyz;
//	tcdh 		= float4	((misc * consts).xy	);

# if defined(USE_R2_STATIC_SUN)
	tcdh 		= (misc * consts).xyyy;
	tcdh.w		= c0.x;								// (,,,dir-occlusion)
#else
	tcdh 		= (misc * consts).xy;
# endif
	position	= float4	(Pe, 		c0.w		);
}
