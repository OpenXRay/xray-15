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
	vec4 vHPos, iP = iPos;

	{
		iP.xy += 0.5f;
		vHPos.x = iP.x/1024 * 2 - 1;
		vHPos.y = (iP.y/768 * 2 - 1)*-1;
		vHPos.zw = iP.zw;
	}

	vTex0 = iTex0;
	vColor = iColor.aaaa;	//	swizzle vertex colour

	gl_Position = vHPos;
}