#include "common.h"

out gl_PerVertex { vec4 gl_Position; };

uniform half4		screen_res;		// Screen resolution (x-Width,y-Height, zw - 1/resolution)

layout(location = POSITION) in vec4 iPos;
layout(location = TEXCOORD0) in vec2 iTex0;
layout(location = TEXCOORD1) in vec2 iTex1;
layout(location = TEXCOORD2) in vec2 iTex2;
layout(location = TEXCOORD3) in vec2 iTex3;
layout(location = TEXCOORD4) in vec2 iTex4;
layout(location = TEXCOORD5) in vec4 iTex5;
layout(location = TEXCOORD6) in vec4 iTex6;

layout(location = TEXCOORD0) out vec2 vTex0;
layout(location = TEXCOORD1) out vec2 vTex1;
layout(location = TEXCOORD2) out vec2 vTex2;
layout(location = TEXCOORD3) out vec2 vTex3;
layout(location = TEXCOORD4) out vec2 vTex4;
layout(location = TEXCOORD5) out vec4 vTex5;
layout(location = TEXCOORD6) out vec4 vTex6;

//////////////////////////////////////////////////////////////////////////////////////////
// Vertex
void main ()
{
	vec4 HPos, P = iPos;

	{
		P.xy += 0.5f;
//		HPos.x = P.x/1024 * 2 - 1;
//		HPos.y = P.y/768 * 2 - 1;
		HPos.x = P.x * screen_res.z * 2 - 1;
		HPos.y = P.y * screen_res.w * 2 - 1;
		HPos.zw = P.zw;
	}


	vTex0 = iTex0;
	vTex1 = iTex1;
	vTex2 = iTex2;
	vTex3 = iTex3;
	vTex4 = iTex4;
	vTex5 = iTex5;
	vTex6 = iTex6;

	gl_Position = HPos;
}