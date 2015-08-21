#include "common.h"

uniform half4		screen_res;		// Screen resolution (x-Width,y-Height, zw - 1/resolution)

layout(location = 0) in vec4 iPos;
layout(location = 1) in vec4 iColor;
layout(location = 2) in vec2 iTex0;

out vec2 vTex0;
out vec4 vColor;

//////////////////////////////////////////////////////////////////////////////////////////
// Vertex
void main ()
{
	vec4 OHPos, IP = iPos;

	{
		IP.xy += 0.5f;
//		OHPos.x = IP.x/1024 * 2 - 1;
//		OHPos.y = (IP.y/768 * 2 - 1)*-1;
		OHPos.x = IP.x * screen_res.z * 2 - 1;
		OHPos.y = (IP.y * screen_res.w * 2 - 1)*-1;
		OHPos.zw = IP.zw;
	}

	vTex0 = iTex0;
	vColor = float4(iColor.bgr*2, 1.0f);	//	swizzle vertex colour
}