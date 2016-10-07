
out gl_PerVertex { vec4 gl_Position; };

layout(location = POSITION)		in vec4	P;	// (float,float,float,1)
layout(location = NORMAL)		in vec4	Nh;	// (nx,ny,nz,hemi occlusion)
layout(location = TANGENT)		in vec4	T;	// tangent
layout(location = BINORMAL)		in vec4	B;	// binormal
layout(location = TEXCOORD0)	in vec2	tc;	// (u,v)
layout(location = TEXCOORD1)	in vec2	lm;	// (lmu,lmv)
layout(location = COLOR0)		in vec4	color;	// (r,g,b,dir-occlusion)	//	Swizzle before use!!!

#ifdef  USE_HWSMAP
layout(location = TEXCOORD0) out vec2	tc0;	// Diffuse map for aref
#else
layout(location = TEXCOORD0) out float	depth;	// Depth
layout(location = TEXCOORD1) out vec2	tc0;	// Diffuse map for aref
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
	I.lmh	= lm;

	v_shadow_direct_aref O = _main (I);

	tc0			= O.tc0;
#ifndef  USE_HWSMAP
	depth		= O.depth;
#endif
	gl_Position = O.hpos;
}
