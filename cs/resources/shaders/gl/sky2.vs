#include "common.h"

out gl_PerVertex { vec4 gl_Position; };

layout(location = 0) in vec4 p;
layout(location = 1) in vec4 c;
layout(location = 2) in vec3 itc0;
layout(location = 3) in vec3 itc1;

layout(location = 0) out vec4 factor;
layout(location = 1) out vec3 tc0;
layout(location = 2) out vec3 tc1;

void main (void)
{
	gl_Position		= mul       (m_WVP, p);									// xform, input in world coords
	tc0				= itc0;                        							// copy tc
	tc1				= itc1;                        							// copy tc
	float	scale	= tex2Dlod	(s_tonemap,float2(.5),.5).x ;
//	float	scale	= s_tonemap.Load( int3(0,0,0) ).x;
//	float	scale	= s_tonemap.Load( int3(1,1,0) ).x;
//	factor			= float4	( c.rgb*(scale*1.7), c.a );      		// copy color, pre-scale by tonemap //float4 ( c.rgb*scale*2, c.a );
    factor			= float4	( c.rgb*(scale*2.0), c.a );      		// copy color, pre-scale by tonemap //float4 ( c.rgb*scale*2, c.a );
}
