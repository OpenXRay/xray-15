
out gl_PerVertex { vec4 gl_Position; };

layout(location = 0) in vec4	P;		// (float,float,float,1)
layout(location = 1) in vec4	Nh;		// (nx,ny,nz,hemi occlusion)
layout(location = 2) in vec4	T;		// tangent
layout(location = 3) in vec4	B;		// binormal
layout(location = 4) in vec2	tc;		// (u,v)
#if	defined(USE_LM_HEMI)
layout(location = 5) in vec2	lm;		// (lmu,lmv)
#elif defined(USE_R2_STATIC_SUN)
layout(location = 5) in vec4	color;	// (r,g,b,dir-occlusion)	//	Swizzle before use!!!
#endif

#ifdef  USE_HWSMAP
layout(location = 0) out vec2	tc0;	// Diffuse map for aref
#else
layout(location = 0) out float	depth;	// Depth
layout(location = 1) out vec2	tc0;	// Diffuse map for aref
#endif

v_shadow_direct_aref _main ( v_static I );

void	main()
{
	v_static I;
	I.P		= P;
	I.Nh	= Nh;
	I.T		= T;
	I.B		= B;
	I.tc	= tc;

	v_shadow_direct_aref O = _main (I);

	tc0			= O.tc0;
#ifndef  USE_HWSMAP
	depth		= O.depth;
#endif
	gl_Position = O.hpos;
}
