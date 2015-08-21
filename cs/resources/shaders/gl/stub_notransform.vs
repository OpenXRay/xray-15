#include "common.h"

layout(location = 0) in vec4 iPos;
layout(location = 1) in vec4 iColor;
layout(location = 2) in vec2 iTex0;

out vec2 vTex0;
out vec4 vColor;

//////////////////////////////////////////////////////////////////////////////////////////
// Vertex
void main ()
{
	gl_Position = iPos;
	vTex0 = iTex0;
	//	Some shaders that use this stub don't need Color at all
	vColor = iColor.bgra;	//	swizzle vertex colour
}