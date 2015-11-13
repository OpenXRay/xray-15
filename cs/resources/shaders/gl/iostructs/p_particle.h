
struct 	v2p
{
	float2 tc0;		// base
	half4 c;		// color
//	Igor: for additional depth dest
#ifdef	USE_SOFT_PARTICLES
	float4 tctexgen;
#endif	//	USE_SOFT_PARTICLES
	float  fog;
};

layout(location = TEXCOORD0) 	in vec2	tc0;		// base
layout(location = COLOR0)		in vec4	c0;			// color
#ifdef	USE_SOFT_PARTICLES
layout(location = TEXCOORD1) 	in vec4	tctexgen;
#endif	//	USE_SOFT_PARTICLES
layout(location = FOG) 			in float	fog;

layout(location = COLOR0) 		out vec4	C;

half4 _main( v2p I );

void	main()
{
	v2p I;
	I.tc0		= tc0;
	I.c			= c0;
	I.tctexgen	= tctexgen;
	I.fog		= fog;

	C = _main(I);
}
