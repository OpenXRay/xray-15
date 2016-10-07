#include "common.h"
#include "skin.h"
#include "iostructs/v_model.h"

vf _main(v_model v)
{
	vf		o;

	float4 	pos 	= v.P;
	float3  pos_w 	= mul( m_W, pos );
	float3 	norm_w 	= normalize( mul( m_W, v.N ) );

	o.hpos 		= mul( m_WVP, pos );		// xform, input in world coords
	o.tc0		= v.tc.xy;					// copy tc
	o.c0 		= calc_model_lq_lighting( norm_w );
	o.fog 		= saturate(calc_fogging( float4( pos_w, 1 ) ));	// fog, input in world coords

	return o;
}
