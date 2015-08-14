#include "common.h"

in vec3 pos;
in vec2 iTex0;
in vec4 oColor;

out vec2 oTex0;
out vec4 oColor;

//////////////////////////////////////////////////////////////////////////////////////////
// Vertex
void main ()
{
	gl_Position = m_WVP * pos;
	oTex0 = iTex0;
	oColor = iColor.bgra;	//	swizzle vertex colour
}
