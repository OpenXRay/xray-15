
out gl_PerVertex { vec4 gl_Position; };

layout(location = POSITION)		in vec4	P;		// (float,float,float,1)
layout(location = NORMAL)		in vec4	Nh;		// (nx,ny,nz)
layout(location = TANGENT)		in vec3	T;		// tangent
layout(location = BINORMAL)		in vec3	B;		// binormal
layout(location = TEXCOORD0)	in vec4	tc;		// (u,v,frac,???)

#ifdef  USE_HWSMAP
layout(location = TEXCOORD0) out vec2	tc0;	// Diffuse map for aref
#else
layout(location = TEXCOORD0) out float	depth;	// Depth
layout(location = TEXCOORD1) out vec2	tc0;	// Diffuse map for aref
#endif

v_shadow_direct_aref _main( v_tree I );

void	main()
{
	v_tree I;
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
