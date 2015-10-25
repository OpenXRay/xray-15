#include "common.h"

out gl_PerVertex { vec4 gl_Position; };

layout(location = 0) in vec4 	P;
layout(location = 1) in vec2 	tc;
layout(location = 2) in vec4 	c;

layout(location = 0) out vec2	tc0;
layout(location = 1) out vec4	c0;

void main ()
{
	float4 hpos	= mul	(m_WVP, P);		// xform, input in world coords
	hpos.z		= abs	(hpos.z);
	hpos.w		= abs	(hpos.w);
	tc0			= tc;				// copy tc
	c0			= c;				// copy color

	gl_Position = hpos;
}
