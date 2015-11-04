
out gl_PerVertex { vec4 gl_Position; };

layout(location = POSITION)	in vec4	hpos;	// (float,float,float,1)
#ifndef  USE_HWSMAP
layout(location = TEXCOORD1) out float	depth;	// Depth
#endif

#ifndef  USE_HWSMAP
layout(location = TEXCOORD0) out float	depth;	// Depth
#endif

v_shadow_direct _main( v_shadow_direct I );

void	main()
{
	v_shadow_direct I;
	I.hpos		= hpos;
#ifndef  USE_HWSMAP
	I.depth		= depth;
#endif

	v_shadow_direct O = _main (I);

#ifndef  USE_HWSMAP
	depth		= O.depth;
#endif
	gl_Position = O.hpos;
}
