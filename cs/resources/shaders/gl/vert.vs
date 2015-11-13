#include "common.h"

out gl_PerVertex { vec4 gl_Position; };

layout(location = POSITION)		in vec4 P;		// (float,float,float,1)
layout(location = NORMAL)		in vec4	Nh;		// (nx,ny,nz,hemi occlusion)
layout(location = TANGENT)		in vec4 T;
layout(location = BINORMAL)		in vec4 B;
layout(location = COLOR0)		in vec4	color;	// (r,g,b,dir-occlusion)
layout(location = TEXCOORD0)	in vec2 tc;		// (u0,v0)

layout(location = TEXCOORD0)	out vec2 tc0;
layout(location = COLOR0)		out vec3 c0;		// c0=all lighting
layout(location = FOG)			out float fog;

void main ()
{
	float3 	N 	= unpack_normal		(Nh);
	gl_Position	= mul				(m_VP, P);			// xform, input in world coords
	tc0			= unpack_tc_base	(tc,T.w,B.w);		// copy tc
//	tc0			= unpack_tc_base	(tc);				// copy tc

	float3 	L_rgb 	= color.zyx;						// precalculated RGB lighting
	float3 	L_hemi 	= v_hemi(N)*Nh.w;					// hemisphere
	float3 	L_sun 	= v_sun(N)*color.w;					// sun
	float3 	L_final	= L_rgb + L_hemi + L_sun + L_ambient.rgb;

	c0			= L_final;
	fog 		= saturate(calc_fogging 		(P));			// fog, input in world coords
}
