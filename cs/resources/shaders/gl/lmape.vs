//	TODO: OGL: Check r2 path. If we always get hemi here
#define	USE_LM_HEMI
#include "common.h"

out gl_PerVertex { vec4 gl_Position; };

layout(location = 0) in vec4 	P;		// (float,float,float,1)
layout(location = 1) in vec4	Nh;		// (nx,ny,nz,hemi occlusion)
layout(location = 2) in vec4	T;		// tangent
layout(location = 3) in vec4	B;		// binormal
layout(location = 4) in vec2	tc;		// (u,v)
#ifdef	USE_LM_HEMI
layout(location = 5) in vec2	lmh;	// (lmu,lmv)
#endif
//layout(location = 6) in vec4	color;	// (r,g,b,dir-occlusion)	//	Swizzle before use!!!

layout(location = 0) out vec2 tc0;
layout(location = 1) out vec2 tc1;
layout(location = 2) out vec2 tch;
layout(location = 3) out vec2 tc2;
layout(location = 4) out vec2 c0;		// c0=hemi+v-lights, 	c0.a = dt*
layout(location = 5) out vec2 c1;		// c1=sun, 		c1.a = dt+
layout(location = 6) out float fog;

vf main(v_static v)
{
	float3 	pos_w	= P;
	float3 	norm_w	= normalize(unpack_normal(Nh));
	
	gl_Position = mul				(m_VP, P);			// xform, input in world coords
	tc0			= unpack_tc_base	(tc,T.w,B.w);		// copy tc
	tc1			= unpack_tc_lmap	(lmh);				// copy tc 
	tch 		= tc1;
	tc2			= calc_reflection	(pos_w, norm_w);
	c0			= v_hemi(norm_w);						// just hemisphere
	c1 			= v_sun	(norm_w);						// sun
	fog.x 		= saturate(calc_fogging (P));			// fog, input in world coords

	return o;
}
