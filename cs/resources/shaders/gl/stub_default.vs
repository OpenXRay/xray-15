#include "common.h"

layout(location = 0) in vec4 iPos;
layout(location = 1) in vec4 iColor;
layout(location = 2) in vec2 iTex0;

out vec4 vColor;
out vec2 vTex0;

//////////////////////////////////////////////////////////////////////////////////////////
// Vertex
void main ()
{
	gl_Position = m_WVP * iPos;
	vTex0 = iTex0;
	vColor = iColor.bgra;	//	swizzle vertex colour
}
