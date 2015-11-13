#include "common.h"

out gl_PerVertex { vec4 gl_Position; };

layout(location = POSITION)		in float4 P;
layout(location = COLOR0)		in float4 C;

layout(location = COLOR0)		out float4 c0;

uniform float4 		tfactor;
void main ()
{
	gl_Position	= mul			(m_WVP, P);			// xform, input in world coords
	c0	 		= tfactor*C;
}
