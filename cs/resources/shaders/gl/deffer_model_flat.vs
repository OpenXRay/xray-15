#include "common.h"
#include "skin.h"
#include "iostructs/v_model_flat.h"

p_flat _main( v_model I )
{
	// world-space  N
	float3 	N_w	= mul( m_W, I.N );

	// Eye-space pos/normal
	p_flat 		O;
	float3	Pe	= mul( m_WV, I.P );
	O.hpos		= mul( m_WVP, I.P );
	O.N	 		= mul( float3x3(m_WV), I.N );

	//  Hemi cube lighting
	float3	Nw	= mul		(float3x3(m_W), float3(I.N));
	half3   hc_pos	= half3(hemi_cube_pos_faces);
	half3	hc_neg	= half3(hemi_cube_neg_faces);
	half3   hc_mixed= all(lessThan(Nw, float3(0))) ? hc_neg : hc_pos;
	float	hemi_val= dot( hc_mixed, abs(Nw) );
	hemi_val	= saturate(hemi_val);

	O.position	= float4	(Pe, hemi_val);		//use L_material.x for old behaviour

#if defined(USE_R2_STATIC_SUN) && !defined(USE_LM_HEMI)
	O.tcdh 		= float4	(I.tc.xyyy);
	O.tcdh.w	= L_material.y;							// (,,,dir-occlusion)
#else
	O.tcdh 		= I.tc.xy;
#endif

#ifdef USE_TDETAIL
	O.tcdbump	= O.tcdh*dt_params.xy;					// dt tc
#endif

	return	O;
}
