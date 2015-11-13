#include "common.h"
#include "shared\wmark.h"

out gl_PerVertex { vec4 gl_Position; };

layout(location = POSITION)		in vec4	P;	// (float,float,float,1)
layout(location = NORMAL)		in vec4	Nh;	// (nx,ny,nz,hemi occlusion)
layout(location = TANGENT)		in vec4	T;	// tangent
layout(location = BINORMAL)		in vec4	B;	// binormal
layout(location = TEXCOORD0)	in vec2	tc;	// (u,v)
layout(location = TEXCOORD1)	in vec2	lm;	// (lmu,lmv)
layout(location = COLOR0)		in vec4	color;	// (r,g,b,dir-occlusion)	//	Swizzle before use!!!

layout(location = TEXCOORD0)	out vec2	tc0;
layout(location = COLOR0)		out vec3	c0;		// c0=all lighting
layout(location = FOG)			out float	fog;

void main ()
{
	float3 norm =	unpack_normal(Nh);
	float4 	pos = 	wmark_shift	(P.xyz,norm);
	gl_Position	= 	mul		(m_VP, pos);					// xform, input in world coords
	tc0			= 	unpack_tc_base	(tc,T.w,B.w);		// copy tc

	//float3 	L_rgb 	= color.xyz;					// precalculated RGB lighting
	//float3 	L_hemi 	= v_hemi(norm)*Nh.w;				// hemisphere
	//float3 	L_sun 	= v_sun(norm)*color.w;				// sun
	//float3 	L_final	= L_rgb + L_hemi + L_sun + L_ambient		;

	c0			= float3(0);	//L_final;
	fog 		= saturate(calc_fogging 		(P));	// fog, input in world coords
}
