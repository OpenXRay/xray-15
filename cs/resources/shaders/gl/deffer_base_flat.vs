#include "common.h"
#include "iostructs/v_flat.h"

p_flat _main ( v_static I )
{
	I.Nh			= unpack_D3DCOLOR(I.Nh);
	I.T				= unpack_D3DCOLOR(I.T);
	I.B				= unpack_D3DCOLOR(I.B);

	// Eye-space pos/normal
	p_flat 		O;
	float4	Pp 	= mul( m_WVP, I.P );
	O.hpos		= Pp;
	O.N			= mul( m_WV, unpack_bx2(I.Nh.rgb) ).xyz;
	float3	Pe	= mul( m_WV, I.P.xyz ).xyz;

	float2	tc 	= unpack_tc_base( I.tc, I.T.w, I.B.w);	// copy tc
	position	= float4( Pe, I.Nh.w );

#if defined(USE_R2_STATIC_SUN) && !defined(USE_LM_HEMI)
	float 	s	= color.w	;							// (r,g,b,dir-occlusion)
	O.tcdh		= float4( tc.xyyy );
	O.tcdh.w	= s;
#else
	O.tcdh		= tc.xy;
#endif

#ifdef	USE_TDETAIL
	O.tcdbump	= O.tcdh * dt_params;					// dt tc
#endif

#ifdef	USE_LM_HEMI
	O.lmh 		= unpack_tc_lmap( I.lmh );
#endif

	return	O;
}
