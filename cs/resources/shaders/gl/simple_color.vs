#include "common.h"

out gl_PerVertex { vec4 gl_Position; };

layout(location = POSITION)	in float4 P;

layout(location = COLOR0) 	out float4 C;

uniform float4 		tfactor;
void main ()
{
	gl_Position	= mul			(m_WVP, P);			// xform, input in world coords
	C 			= tfactor;
}
