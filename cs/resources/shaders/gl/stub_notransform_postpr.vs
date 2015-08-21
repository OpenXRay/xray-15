#include "common.h"

uniform half4		screen_res;		// Screen resolution (x-Width,y-Height, zw - 1/resolution)

layout(location = 0) in vec4 iPos;
layout(location = 1) in vec4 iColor;
layout(location = 2) in vec4 iGray;
layout(location = 3) in vec2 iTex0;
layout(location = 4) in vec2 iTex1;
layout(location = 5) in vec2 iTex2;

out vec2 vTex0;
out vec2 vTex1;
out vec2 vTex2;
out vec4 vColor;
out vec4 vGray;

//////////////////////////////////////////////////////////////////////////////////////////
// Vertex
void main ()
{
	vec4 vHPos, iP = iPos;

	{
		iP.xy += 0.5f;
//		vHPos.x = iP.x/1024 * 2 - 1;
//		vHPos.y = (iP.y/768 * 2 - 1)*-1;
		vHPos.x = iP.x * screen_res.z * 2 - 1;
		vHPos.y = (iP.y * screen_res.w * 2 - 1)*-1;
		vHPos.zw = iP.zw;
	}


	vTex0	= iTex0;
	vTex1	= iTex1;
	vTex2	= iTex2;
	
	vColor	= iColor.bgra;	//	swizzle vertex colour
	vGray	= iGray.bgra;	//	swizzle vertex colour

	gl_Position = vHPos;
}