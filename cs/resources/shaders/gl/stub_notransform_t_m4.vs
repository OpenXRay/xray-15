#include "common.h"

out gl_PerVertex { vec4 gl_Position; };

uniform half4		screen_res;		// Screen resolution (x-Width,y-Height, zw - 1/resolution)

layout(location = 0) in vec4 iPos;
layout(location = 1) in vec4 iColor;
layout(location = 2) in vec2 iTex0;

layout(location = 0) out vec2 vTex0;
layout(location = 1) out vec4 vColor;

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

	vTex0 = iTex0;
	vColor = float4(iColor.bgr*4, 1.0f);	//	swizzle vertex colour

	gl_Position = vHPos;
}