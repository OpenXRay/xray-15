#include "common.h"

out gl_PerVertex { vec4 gl_Position; };

uniform half4		screen_res;		// Screen resolution (x-Width,y-Height, zw - 1/resolution)

layout(location = 0) in vec4 iPos;
layout(location = 1) in vec4 iColor;
layout(location = 2) in vec4 iGray;
layout(location = 3) in vec2 iTex0;
layout(location = 4) in vec2 iTex1;
layout(location = 5) in vec2 iTex2;

layout(location = 0) out vec2 vTex0;
layout(location = 1) out vec2 vTex1;
layout(location = 2) out vec2 vTex2;
layout(location = 3) out vec4 vColor;
layout(location = 4) out vec4 vGray;

//////////////////////////////////////////////////////////////////////////////////////////
// Vertex
void main ()
{
	vec4 HPos, P = iPos;

	{
		P.xy += 0.5f;
//		HPos.x = P.x/1024 * 2 - 1;
//		HPos.y = (P.y/768 * 2 - 1)*-1;
		HPos.x = P.x * screen_res.z * 2 - 1;
		HPos.y = (P.y * screen_res.w * 2 - 1)*-1;
		HPos.zw = P.zw;
	}


	vTex0	= iTex0;
	vTex1	= iTex1;
	vTex2	= iTex2;
	
	vColor	= iColor.bgra;	//	swizzle vertex colour
	vGray	= iGray.bgra;	//	swizzle vertex colour

	gl_Position = HPos;
}