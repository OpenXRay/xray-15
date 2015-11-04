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
	gl_Position = iPos;
	vTex0 = iTex0;
	//	Some shaders that use this stub don't need Color at all
	vColor = iColor.bgra;	//	swizzle vertex colour
}