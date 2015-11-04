#include "common.h"

out gl_PerVertex { vec4 gl_Position; };

layout(location = POSITION) in vec4 P;
layout(location = TEXCOORD0) in vec2 uv;

layout(location = TEXCOORD0) out vec2 tc0;
layout(location = TEXCOORD1) out vec2 tcJ;

//////////////////////////////////////////////////////////////////////////////////////////
// Vertex
void main ()
{
	gl_Position	= float4	(P.x, -P.y, 0, 1);
	float  	scale 	= tex2D(s_tonemap,vec2(0,0)).x;
	tc0			= float4	(P.zw, scale, scale).rg;
	tcJ			= uv;
}
