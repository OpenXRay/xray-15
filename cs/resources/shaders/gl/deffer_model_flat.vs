#include "common.h"
#include "skin.h"
#include "iostructs/v_model_flat.h"

p_flat _main( v_model I )
{
	// world-space  N
	float3 	N_w	= mul( m_W, I.N ).xyz;

	// Eye-space pos/normal
	p_flat 		O;
	float3	Pe	= mul( m_WV, I.P.xyz ).xyz;
	O.hpos		= mul( m_WVP, I.P );
	O.N	 		= mul( m_WV, I.N ).xyz;

	O.position	= float4( Pe, L_material.x );

#if defined(USE_R2_STATIC_SUN) && !defined(USE_LM_HEMI)
	O.tcdh 		= float4( I.tc.xyyy	);
	O.tcdh.w	= L_material.y;							// (,,,dir-occlusion)
#else
	O.tcdh 		= I.tc.xy;
#endif

#ifdef USE_TDETAIL
	O.tcdbump	= O.tcdh*dt_params;					// dt tc
#endif

	return	O;
}
