#include "common.h"
#include "iostructs\v_lmape.h"

vf _main 	(v_static v)
{
	vf 		o;

	float3 	pos_w	= v.P;
	float3 	norm_w	= normalize(unpack_normal(v.Nh));
	
	o.hpos 		= mul				(m_VP, v.P);			// xform, input in world coords
	o.tc0		= unpack_tc_base	(v.tc,v.T.w,v.B.w);		// copy tc
	o.tc1		= unpack_tc_lmap	(v.lmh);			// copy tc 
	o.tch 		= o.tc1;
	o.tc2		= calc_reflection	(pos_w, norm_w);
	o.fog 		= saturate(calc_fogging 		(v.P));			// fog, input in world coords
	o.c0		= half4(v_hemi(norm_w),o.fog);	// just hemisphere
	o.c1 		= v_sun	(norm_w);  	// sun

	return o;
}
