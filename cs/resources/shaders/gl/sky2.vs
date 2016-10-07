#include "common.h"

out gl_PerVertex { vec4 gl_Position; };

layout(location = POSITION) in vec4 p;
layout(location = COLOR0) in vec4 c;
layout(location = TEXCOORD0) in vec3 itc0;
layout(location = TEXCOORD1) in vec3 itc1;

layout(location = TEXCOORD0) out vec4 factor;
layout(location = TEXCOORD1) out vec3 tc0;
layout(location = TEXCOORD2) out vec3 tc1;

void main (void)
{
	float4	hpos	= mul       (m_WVP, p * 1000);							// xform, input in world coords, 1000 - magic number
	hpos.z	    	= hpos.w;
	tc0				= itc0;                        							// copy tc
	tc1				= itc1;                        							// copy tc
	float	scale	= tex2Dlod	(s_tonemap,float4(.5)).x ;
//	float	scale	= s_tonemap.Load( int3(0,0,0) ).x;
//	float	scale	= s_tonemap.Load( int3(1,1,0) ).x;
//	factor			= float4	( c.rgb*(scale*1.7), c.a );      		// copy color, pre-scale by tonemap //float4 ( c.rgb*scale*2, c.a );
    factor			= float4	( c.rgb*(scale*2.0), c.a );      		// copy color, pre-scale by tonemap //float4 ( c.rgb*scale*2, c.a );
	gl_Position		= hpos;
}
