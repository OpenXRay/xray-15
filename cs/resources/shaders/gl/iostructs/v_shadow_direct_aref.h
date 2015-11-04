
out gl_PerVertex { vec4 gl_Position; };

layout(location = 0) in vec4	P;			// (float,float,float,1)
layout(location = 1) in vec4	Nh;			// (nx,ny,nz,hemi occlusion)
layout(location = 2) in vec4	T;			// tangent
layout(location = 3) in vec4	B;			// binormal
layout(location = 4) in vec4	color;		// (r,g,b,dir-occlusion)	//	Swizzle before use!!!
layout(location = 5) in vec2	tc;			// (u,v)
#if	defined(USE_LM_HEMI)
layout(location = 6) in vec2	lm;			// (lmu,lmv)
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
	I.color = color;
//	I.color.rgb 	= I.color.bgr;	//	Swizzle to compensate DX9/OGL format mismatch
	I.tc	= tc;
#if	defined(USE_LM_HEMI)
	I.lmh	= lm;
#endif

	v_shadow_direct_aref O = _main (I);

	tc0			= O.tc0;
#ifndef  USE_HWSMAP
	depth		= O.depth;
#endif
	gl_Position = O.hpos;
}
