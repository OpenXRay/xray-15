#include "common.h"
#include "shared\wmark.h"

out gl_PerVertex { vec4 gl_Position; };

layout(location = 0) in vec4	P;		// (float,float,float,1)
layout(location = 1) in vec4	Nh;		// (nx,ny,nz,hemi occlusion)
layout(location = 2) in vec4	T;		// tangent
layout(location = 3) in vec4	B;		// binormal
layout(location = 4) in vec4	color;	// (r,g,b,dir-occlusion)	//	Swizzle before use!!!
layout(location = 5) in vec2	tc;		// (u,v)

layout(location = 0) out vec2	tc0;
layout(location = 1) out vec3	c0;		// c0=all lighting
layout(location = 2) out float	fog;

void main ()
{
	float3 norm =	unpack_normal(Nh.xyz);
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
