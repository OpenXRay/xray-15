
out gl_PerVertex { vec4 gl_Position; };

struct 	a2v
{
// 	float4 tc0;	// Texture coordinates
	float4 P;	// Object-space position
};

layout(location = 0) in vec4	P;		// (float,float,float,1)

#ifndef  USE_HWSMAP
layout(location = 0) out float	depth;	// Depth
#endif

v_shadow_direct _main( a2v I );

void	main()
{
	a2v I;
	I.P		= P;

	v_shadow_direct O = _main (I);

#ifndef  USE_HWSMAP
	depth		= O.depth;
#endif
	gl_Position = O.hpos;
}
