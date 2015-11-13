#include "common.h"
#include "iostructs/v_static.h"

vf _main (v_static v)
{
	vf 		o;

	o.hpos 		= mul			(m_WVP, v.P);		// xform, input in world coords
	o.tc0		= unpack_tc_base	(v.tc,v.T.w,v.B.w);	// copy tc

	// calculate fade
	float3  dir_v 	= normalize		(mul(m_WV,v.P));
	float3 	norm_v 	= normalize 		(mul(m_WV,unpack_normal(v.Nh)));
	float 	fade 	= abs			(dot(dir_v,norm_v));
	o.c0		= float4(fade);

	return o;
}
