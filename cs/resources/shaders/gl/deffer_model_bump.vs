#include	"common.h"
#include	"skin.h"

#if defined(USE_PARALLAX) || defined(USE_STEEPPARALLAX)
	uniform half3x4	    m_invW;
#endif 	//	defined(USE_PARALLAX) || defined(USE_STEEPPARALLAX)

out gl_PerVertex { vec4 gl_Position; };

layout(location = 0) in vec4 P;		// (float,float,float,1)
#ifdef 	SKIN_0
layout(location = 1) in vec3 norm;	// (nx,ny,nz)
#else
layout(location = 1) in vec4 norm;	// (nx,ny,nz)
#endif
#if defined(SKIN_3) || defined(SKIN_4)
layout(location = 2) in vec4 tan;	// (nx,ny,nz)
layout(location = 3) in vec4 bnorm;	// (nx,ny,nz)
#else
layout(location = 2) in vec3 tan;	// (nx,ny,nz)
layout(location = 3) in vec3 bnorm;	// (nx,ny,nz)
#endif
#if defined(SKIN_2) || defined(SKIN_3)
layout(location = 4) in vec4 tc;	// (u,v)
#else
layout(location = 4) in vec2 tc;	// (u,v)
#endif
#ifdef 	SKIN_4
layout(location = 5) in vec4 ind;
#endif

#if defined(USE_R2_STATIC_SUN) && !defined(USE_LM_HEMI)
layout(location = 0) out vec4	tcdh;		// Texture coordinates,         w=sun_occlusion
#else
layout(location = 0) out vec2	tcdh;		// Texture coordinates
#endif
layout(location = 1) out vec4	position;	// position + hemi
layout(location = 2) out vec3	M1;			// nmap 2 eye - 1
layout(location = 3) out vec3	M2;			// nmap 2 eye - 2
layout(location = 4) out vec3	M3;	// nmap 2 eye - 3
#if defined(USE_PARALLAX) || defined(USE_STEEPPARALLAX)
layout(location = 5) out vec3	eye;		// vector to point in tangent space
  #ifdef USE_TDETAIL
layout(location = 6) out vec2	tcdbump;	// d-bump
    #ifdef USE_LM_HEMI
layout(location = 7) out vec2	lmh;		// lm-hemi
    #endif
  #else
    #ifdef USE_LM_HEMI
layout(location = 6) out vec2	lmh;		// lm-hemi
    #endif
  #endif
#else
  #ifdef USE_TDETAIL
layout(location = 5) out vec2	tcdbump;	// d-bump
    #ifdef USE_LM_HEMI
layout(location = 6) out vec2	lmh;		// lm-hemi
    #endif
  #else
    #ifdef USE_LM_HEMI
layout(location = 5) out vec2	lmh;		// lm-hemi
    #endif
  #endif
#endif

void _main( v_model I )
{
	float4	w_pos	= I.P;

	// Eye-space pos/normal
	gl_Position	= mul( m_WVP, w_pos	);
	float2 	tc 	= I.tc;
	float3	Pe	= mul( m_WV, w_pos.xyz ).xyz;
	position	= float4( Pe, L_material.x );

#if defined(USE_R2_STATIC_SUN) && !defined(USE_LM_HEMI)
	tcdh 		= float4( tc.xyyy );
	tcdh.w		= L_material.y;					// (,,,dir-occlusion)
#else
	tcdh 		= float2( tc.xy );
#endif

	// Calculate the 3x3 transform from tangent space to eye-space
	// TangentToEyeSpace = object2eye * tangent2object
	//		     = object2eye * transpose(object2tangent) (since the inverse of a rotation is its transpose)
	float3 	N 	= I.N;		// just scale (assume normal in the -.5f, .5f)
	float3 	T 	= I.T;		// 
	float3 	B 	= I.B;		// 
	float3x3 xform	= mul	(mat3(m_WV), float3x3(
						2*T.x,2*B.x,2*N.x,
						2*T.y,2*B.y,2*N.y,
						2*T.z,2*B.z,2*N.z
				));
	// The pixel shader operates on the bump-map in [0..1] range
	// Remap this range in the matrix, anyway we are pixel-shader limited :)
	// ...... [ 2  0  0  0]
	// ...... [ 0  2  0  0]
	// ...... [ 0  0  2  0]
	// ...... [-1 -1 -1  1]
	// issue: strange, but it's slower :(
	// issue: interpolators? dp4? VS limited? black magic? 

	// Feed this transform to pixel shader
	M1 			= xform	[0]; 
	M2 			= xform	[1]; 
	M3 			= xform	[2]; 

#if defined(USE_PARALLAX) || defined(USE_STEEPPARALLAX)
	float3  WrldEye	= -(mul(m_W,w_pos) - eye_position);
	float3	ObjEye	= mul( m_invW,  WrldEye);
	eye 			= mul( float3x3(T,B,N),  ObjEye);	//	Local eye
#endif

#ifdef 	USE_TDETAIL
	tcdbump			= tcdh * dt_params;		// dt tc
#endif
}

/////////////////////////////////////////////////////////////////////////
void	main()
{
#ifdef 	SKIN_NONE
	v_model v;
#endif
#ifdef 	SKIN_0
	v_model_skinned_0 v;
#endif
#ifdef	SKIN_1
	v_model_skinned_1 v;
#endif
#ifdef	SKIN_2
	v_model_skinned_2 v;
#endif
#ifdef	SKIN_3
	v_model_skinned_3 v;
#endif
#ifdef	SKIN_4
	v_model_skinned_4 v;
	v.ind = ind;
#endif

	v.P = P;
	v.N = norm;
	v.T = tan;
	v.B = bnorm;
	v.tc = tc;

#ifdef 	SKIN_NONE
	_main(v);
#endif
#ifdef 	SKIN_0
	_main(skinning_0(v));
#endif
#ifdef	SKIN_1
	_main(skinning_1(v));
#endif
#ifdef	SKIN_2
	_main(skinning_2(v));
#endif
#ifdef	SKIN_3
	_main(skinning_3(v));
#endif
#ifdef	SKIN_4
	_main(skinning_4(v));
#endif
}
