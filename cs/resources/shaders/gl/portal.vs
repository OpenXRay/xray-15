#include "common.h"

out gl_PerVertex { vec4 gl_Position; };

layout(location = POSITION) in vec4 	pos;	// (float,float,float,1)
layout(location = COLOR0) 	in vec4 	c;		// (r,g,b,dir-occlusion)

layout(location = COLOR0)	out vec4	c0;
layout(location = FOG)		out float	fog;

void main ()
{
	float4 color;

	float4 hpos	= mul(m_VP, pos);				// xform, input in world coords
	color		= c;
	fog 		= calc_fogging(pos);			// fog, input in world coords
	fog 		= saturate(fog);
	color.rgb 	= lerp(fog_color, color, fog).rgb;

	float scale = tex2Dlod	(s_tonemap,float4(.5,.5,.5,.5)).x ;
	color.rgb	= color.rgb*scale;      		// copy color, pre-scale by tonemap //float4 ( v.c.rgb*scale*2, v.c.a );

	c0			= color;
	gl_Position = hpos;
}
