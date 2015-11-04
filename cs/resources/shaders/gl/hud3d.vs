#include "common.h"

out gl_PerVertex { vec4 gl_Position; };

layout(location = POSITION) in vec4 iPos;
layout(location = COLOR0) in vec4 color;
layout(location = TEXCOORD0) in vec2 uv;

layout(location = TEXCOORD0) out vec2 tc0;

void main ()
{
	tc0			= uv;
	float4 P	= iPos;
	P.w			= 1;
	gl_Position = mul( m_WVP, P );
}
