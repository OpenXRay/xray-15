#include "shared/common.h"

uniform half4		screen_res;		// Screen resolution (x-Width,y-Height, zw - 1/resolution)

out gl_PerVertex { vec4 gl_Position; };

layout(location = POSITION) in vec4 iPos;
layout(location = COLOR0) in vec4 iColor;

layout(location = COLOR0) out vec4 vColor;

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

	vColor = iColor.bgra;	//	swizzle vertex colour
}