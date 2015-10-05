#include	"common.h"

out gl_PerVertex { vec4 gl_Position; };

layout(location = 0) in vec4	P;		// (float,float,float,1)
layout(location = 1) in vec4	norm;	// (nx,ny,nz,hemi occlusion)
layout(location = 2) in vec4	tan;	// tangent
layout(location = 3) in vec4	bnorm;	// binormal
layout(location = 4) in vec2	tc;		// (u,v)
#if	defined(USE_LM_HEMI)
layout(location = 5) in vec2	lm;		// (lmu,lmv)
#elif defined(USE_R2_STATIC_SUN)
layout(location = 5) in vec4	color;	// (r,g,b,dir-occlusion)	//	Swizzle before use!!!
#endif

#if defined(USE_R2_STATIC_SUN) && !defined(USE_LM_HEMI)
layout(location = 0) out vec4	tcdh;		// Texture coordinates,         w=sun_occlusion
#else
layout(location = 0) out vec2	tcdh;		// Texture coordinates
#endif
layout(location = 1) out vec4	position;	// position + hemi
layout(location = 2) out vec3	M1;			// nmap 2 eye - 1
layout(location = 3) out vec3	M2;			// nmap 2 eye - 2
layout(location = 4) out vec3	M3;			// nmap 2 eye - 3
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

void main()
{
//	color.rgb 		= color.bgr;	//	Swizzle to compensate DX9/OGL format mismatch
	float4	w_pos	= P				;
	float2 	tc		= unpack_tc_base	(tc,tan.w,bnorm.w);	// copy tc
	float 	hemi 	= norm.w			;

	// Eye-space pos/normal
	float3	Pe	= mul		(m_WV,  w_pos.xyz	).xyz;
	gl_Position	= mul		(m_WVP,	w_pos		);
	position	= float4	(Pe, hemi			);
//	position	= float4	(gl_Position.xyz, hemi	);

#if defined(USE_R2_STATIC_SUN) && !defined(USE_LM_HEMI)
	tcdh 	= float4	(tc.xyyy);
	tcdh.w	= color.w;				// (r,g,b,dir-occlusion)
#else
	tcdh 	= tc.xy;
#endif

	// Calculate the 3x3 transform from tangent space to eye-space
	// TangentToEyeSpace = object2eye * tangent2object
	//		     = object2eye * transpose(object2tangent) (since the inverse of a rotation is its transpose)
	vec4 Nh		= unpack_D3DCOLOR(norm);
	vec4 T		= unpack_D3DCOLOR(tan);
	vec4 B		= unpack_D3DCOLOR(bnorm);
	float3 	N 	= unpack_bx4(norm);	// just scale (assume normal in the -.5f, .5f)
	float3 	xT 	= unpack_bx4(tan);	// 
	float3 	xB 	= unpack_bx4(bnorm);// 
	float3x3 xform	= mul	(float3x3(
						xT.x,xB.x,N.x,
						xT.y,xB.y,N.y,
						xT.z,xB.z,N.z
				), mat3(m_WV));
	// The pixel shader operates on the bump-map in [0..1] range
	// Remap this range in the matrix, anyway we are pixel-shader limited :)
	// ...... [ 2  0  0  0]
	// ...... [ 0  2  0  0]
	// ...... [ 0  0  2  0]
	// ...... [-1 -1 -1  1]
	// issue: strange, but it's slower :(
	// issue: interpolators? dp4? VS limited? black magic? 

	// Feed this transform to pixel shader
	M1 			= xform[0]; 
	M2 			= xform[1]; 
	M3 			= xform[2]; 

#if defined(USE_PARALLAX) || defined(USE_STEEPPARALLAX)
	eye 		= mul		(-(w_pos.xyz - eye_position), float3x3(T,B,N));
#endif

#ifdef 	USE_TDETAIL
	tcdbump		= tcdh * dt_params.xy;		// dt tc
#endif

#ifdef	USE_LM_HEMI
	lmh 		= unpack_tc_lmap	(lm);
#endif
}
