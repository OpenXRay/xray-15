#include "common.h"

out gl_PerVertex { vec4 gl_Position; };

layout(location = POSITION)		in vec4 iPos;
layout(location = COLOR0)		in vec4 iColor;
layout(location = TEXCOORD0)	in vec2 iTex0;

layout(location = TEXCOORD0)	out vec2 vTex0;
layout(location = COLOR0)		out vec4 vColor;

//////////////////////////////////////////////////////////////////////////////////////////
// Vertex
void main ()
{
	vec4 HPos, P = iPos;

	{
		P.xy += 0.5f;
		HPos.x = P.x/1024 * 2 - 1;
		HPos.y = (P.y/768 * 2 - 1)*-1;
		HPos.zw = P.zw;
	}

	vTex0 = iTex0;
	vColor = iColor.aaaa;	//	swizzle vertex colour

	gl_Position = HPos;
}