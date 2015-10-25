#include "common.h"

out gl_PerVertex { vec4 gl_Position; };

layout(location = 0) in vec4 	P;
layout(location = 1) in vec2 	tc;
layout(location = 2) in vec4 	c;

layout(location = 0) out vec2	tc0;
layout(location = 1) out vec4	c0;
//	Igor: for additional depth dest
#ifdef	USE_SOFT_PARTICLES
layout(location = 2) out vec4	tctexgen;
#endif	//	USE_SOFT_PARTICLES

//uniform float4x4 	mVPTexgen;

void main ()
{
	float4 hpos = mul	(m_WVP, P);		// xform, input in world coords
//	float4 hpos	= mul	(m_VP, P);		// xform, input in world coords
	tc0			= tc;				// copy tc
	c0			= unpack_D3DCOLOR(c);				// copy color

//	Igor: for additional depth dest
#ifdef	USE_SOFT_PARTICLES
	tctexgen 	= mul( mVPTexgen, P);
	tctexgen.z	= hpos.z;
#endif	//	USE_SOFT_PARTICLES

	gl_Position = hpos;
}
