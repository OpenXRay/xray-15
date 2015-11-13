
out gl_PerVertex { vec4 gl_Position; };

struct vv
{
	float4 P;
	float2 tc;
	float4 c;
};
struct vf
{
	float4 hpos;
	float2 tc;
	float4 c;

//	Igor: for additional depth dest
#ifdef	USE_SOFT_PARTICLES
	float4 tctexgen;
#endif	//	USE_SOFT_PARTICLES
};

layout(location = POSITION)		in float4 P;
layout(location = TEXCOORD0) 	in float2 tc;
layout(location = COLOR0)		in float4 c;

layout(location = TEXCOORD0)	out vec2	tc0;		// base
layout(location = COLOR0)		out vec4	c0;			// color
#ifdef	USE_SOFT_PARTICLES
layout(location = TEXCOORD1)	out vec4	tctexgen;
#endif	//	USE_SOFT_PARTICLES
layout(location = FOG) 			out float	fog;

vf _main( vv v );

void	main()
{
	vv I;
	I.P	 = P;
	I.tc = tc;
	I.c	 = c;

	vf O;
	O = _main(I);

	tc0			= O.tc;
	c0			= O.c;
#ifdef	USE_SOFT_PARTICLES
	tctexgen	= O.tctexgen;
#endif	//	USE_SOFT_PARTICLES
	gl_Position = O.hpos;
}
