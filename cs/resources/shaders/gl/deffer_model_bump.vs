#include	"common.h"
#include	"skin.h"
#include	"iostructs/v_model_bumped.h"

#if defined(USE_PARALLAX) || defined(USE_STEEPPARALLAX)
	uniform half3x4	    m_invW;
#endif 	//	defined(USE_PARALLAX) || defined(USE_STEEPPARALLAX)

p_bumped _main( v_model I )
{
	float4	w_pos	= I.P;

	// Eye-space pos/normal
	p_bumped	O;
	O.hpos		= mul( m_WVP, w_pos	);
	float2 	tc 	= I.tc;
	float3	Pe	= mul( m_WV, w_pos.xyz ).xyz;
	O.position	= float4( Pe, L_material.x );

#if defined(USE_R2_STATIC_SUN) && !defined(USE_LM_HEMI)
	O.tcdh 		= float4( tc.xyyy );
	O.tcdh.w	= L_material.y;					// (,,,dir-occlusion)
#else
	O.tcdh 		= float2( tc.xy );
#endif

	// Calculate the 3x3 transform from tangent space to eye-space
	// TangentToEyeSpace = object2eye * tangent2object
	//		     = object2eye * transpose(object2tangent) (since the inverse of a rotation is its transpose)
	float3 	N 	= I.N;		// just scale (assume normal in the -.5f, .5f)
	float3 	T 	= I.T;		// 
	float3 	B 	= I.B;		// 
	float3x3 xform	= mul	(float3x3(
						2*T.x,2*B.x,2*N.x,
						2*T.y,2*B.y,2*N.y,
						2*T.z,2*B.z,2*N.z
				), float3x3(m_WV));
	// The pixel shader operates on the bump-map in [0..1] range
	// Remap this range in the matrix, anyway we are pixel-shader limited :)
	// ...... [ 2  0  0  0]
	// ...... [ 0  2  0  0]
	// ...... [ 0  0  2  0]
	// ...... [-1 -1 -1  1]
	// issue: strange, but it's slower :(
	// issue: interpolators? dp4? VS limited? black magic? 

	// Feed this transform to pixel shader
	O.M1		= xform	[0]; 
	O.M2		= xform	[1]; 
	O.M3		= xform	[2]; 

#if defined(USE_PARALLAX) || defined(USE_STEEPPARALLAX)
	float3  WrldEye	= -(mul(m_W,w_pos.xyz).xyz - eye_position);
	float3	ObjEye	= mul( m_invW,  WrldEye).xyz;
	O.eye 			= mul( ObjEye,  float3x3(T,B,N));	//	Local eye
#endif

#ifdef 	USE_TDETAIL
	O.tcdbump			= tcdh * dt_params;		// dt tc
#endif

	return	O;
}
