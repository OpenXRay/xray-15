#ifndef	SHADOW_H
#define SHADOW_H

#include "common.h"

//	Used for RGBA texture too ?!
uniform sampler2D	s_smap;		// 2D/cube shadowmap

#define	KERNEL	.6f

//////////////////////////////////////////////////////////////////////////////////////////
// hardware + PCF
//////////////////////////////////////////////////////////////////////////////////////////
half sample_hw_pcf (float4 tc,float4 shift)
{
	const float 	ts = KERNEL / float(SMAP_size);
#ifndef SUNSHAFTS_DYNAMIC
	float	s	= tex2Dproj( s_smap, tc + tc.w * shift * ts ).x;
	return (s < tc.z / tc.w) ? 0.f : 1.f;
#else	//	SUNSHAFTS_DYNAMIC
	float4 tc2 = tc / tc.w + shift * ts;
	tc2.w = 0;
	return tex2Dlod(s_smap, tc2).x;
#endif	//	SUNSHAFTS_DYNAMIC
}

half shadow_hw( float4 tc )
{
  	half	s0		= sample_hw_pcf( tc, float4( -1, -1, 0, 0) );
  	half	s1		= sample_hw_pcf( tc, float4( +1, -1, 0, 0) );
  	half	s2		= sample_hw_pcf( tc, float4( -1, +1, 0, 0) );
  	half	s3		= sample_hw_pcf( tc, float4( +1, +1, 0, 0) );

	return	(s0+s1+s2+s3)/4.f;
}
//////////////////////////////////////////////////////////////////////////////////////////
//	D24X8+PCF
//////////////////////////////////////////////////////////////////////////////////////////
half shadow( float4 tc ) 
{
	return shadow_hw( tc ); 
}

//////////////////////////////////////////////////////////////////////////////////////////
// testbed

uniform sampler2D	jitter0;
uniform sampler2D	jitter1;
//uniform sampler2D	jitter2;
//uniform sampler2D	jitter3;
//uniform half4 	jitterS;

//Texture2D			jitterMipped;

half4 	test 		(float4 tc, half2 offset)
{
	float4	tcx	= float4 (tc.xy + tc.w*offset, tc.zw);
	bvec4	s	= lessThan(tex2Dproj (s_smap,tcx), float4(tc.z / tc.w));
	return	half4(s);
}

half 	shadowtest 	(float4 tc, float4 tcJ)				// jittered sampling
{
	half4	r;

	const 	float 	scale 	= (2.7f/float(SMAP_size));

	half4	J0 	= tex2Dproj	(jitter0,tcJ)*scale;
	half4	J1 	= tex2Dproj	(jitter1,tcJ)*scale;

		r.x 	= test 	(tc,J0.xy).x;
		r.y 	= test 	(tc,J0.wz).y;
		r.z		= test	(tc,J1.xy).z;
		r.w		= test	(tc,J1.wz).x;

	return	dot(r,vec4(1.f/4.f));
}

half 	shadowtest_sun 	(float4 tc, float2 tcJ)			// jittered sampling
{
	half4	r;

//	const 	float 	scale 	= (2.0f/float(SMAP_size));
	const 	float 	scale 	= (0.7f/float(SMAP_size));
	half4	J0 	= tex2D	(jitter0,tcJ)*scale;
	half4	J1 	= tex2D	(jitter1,tcJ)*scale;

		r.x 	= test 	(tc,J0.xy).x;
		r.y 	= test 	(tc,J0.wz).y;
		r.z		= test	(tc,J1.xy).z;
		r.w		= test	(tc,J1.wz).x;

	return	dot(r,vec4(1.f/4.f));
}

half 	shadow_rain 	(float4 tc, float2 tcJ)			// jittered sampling
{
	half4	r;

	const 	float 	scale 	= (4.0f/float(SMAP_size));
	half4	J0 	= tex2D	(jitter0,tcJ)*scale;
	half4	J1 	= tex2D	(jitter1,tcJ)*scale;

	r.x 	= test 	(tc,J0.xy).x;
	r.y 	= test 	(tc,J0.wz).y;
	r.z		= test	(tc,J1.xy).z;
	r.w		= test	(tc,J1.wz).x;

//	half4	J0 	= jitterMipped.Sample( smp_base, tcJ )*scale;

//	r.x 	= test 	(tc,J0.xy).x;
//	r.y 	= test 	(tc,J0.wz).y;
//	r.z		= test	(tc,J0.yz).z;
//	r.w		= test	(tc,J0.xw).x;

	return	dot(r,vec4(1.f/4.f));
}

//////////////////////////////////////////////////////////////////////////////////////////
#ifdef  USE_SUNMASK	
float3x4 m_sunmask;	// ortho-projection
half sunmask( float4 P )
{
	float2 		tc	= mul( m_sunmask, P );		//
	return 		tex2D( s_lmap, tc ).w;			// A8 
}
#else
half sunmask( float4 P ) { return 1.f; }		// 
#endif
//////////////////////////////////////////////////////////////////////////////////////////
uniform float4x4	m_shadow;

#endif
