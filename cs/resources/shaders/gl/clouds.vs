#include "common.h"
#include "shared\cloudconfig.h"

out gl_PerVertex { vec4 gl_Position; };

layout(location = 0) in vec4 p;
layout(location = 1) in vec4 dir;
layout(location = 2) in vec4 iColor;

layout(location = 0) out vec4 vColor;
layout(location = 1) out vec2 tc0;
layout(location = 2) out vec2 tc1;

void main (void)
{
	gl_Position 		= mul		(m_WVP, p);	// xform, input in world coords
	
//	if (length(float3(v.p.x,0,v.p.z))>CLOUD_FADE)	vColor.w = 0	;

	// generate tcs
	float2  d0	= dir.xy*2-1;
	float2  d1	= dir.wz*2-1;
	float2 	_0	= p.xz * CLOUD_TILE0 + d0*timers.z*CLOUD_SPEED0;
	float2 	_1	= p.xz * CLOUD_TILE1 + d1*timers.z*CLOUD_SPEED1;
	tc0			= _0;					// copy tc
	tc1			= _1;					// copy tc

	vColor		=	iColor	;			// copy color, low precision, cannot prescale even by 2
	vColor.w	*= 	pow		(p.y,25);

	float  	scale 	= 	tex2Dlod (s_tonemap,float4(.5)).x ;
//	float	scale	= s_tonemap.Load( int3(0,0,0) ).x;
//	float	scale	= s_tonemap.Load( int3(1,1,0) ).x;
	vColor.rgb 	*= 	scale	;		// high precision
}
