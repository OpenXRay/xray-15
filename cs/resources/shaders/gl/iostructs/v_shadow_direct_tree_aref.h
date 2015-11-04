
out gl_PerVertex { vec4 gl_Position; };

#ifdef  USE_HWSMAP
layout(location = TEXCOORD0) in vec2	tc0;	// Diffuse map for aref
#else
layout(location = TEXCOORD0) in float	depth;	// Depth
layout(location = TEXCOORD1) in vec2	tc0;	// Diffuse map for aref
#endif

#ifdef  USE_HWSMAP
layout(location = TEXCOORD0) out vec2	tc0;	// Diffuse map for aref
#else
layout(location = TEXCOORD0) out float	depth;	// Depth
layout(location = TEXCOORD1) out vec2	tc0;	// Diffuse map for aref
#endif

v_shadow_direct_aref main ( v_shadow_direct_aref I )

void	main()
{
	v_shadow_direct_aref I;
	I.tc0	= tc0;
#ifndef  USE_HWSMAP
	I.depth	= depth;
#endif

	v_shadow_direct_aref O = _main (I);

	tc0			= O.tc0;
#ifndef  USE_HWSMAP
	depth		= O.depth;
#endif
	gl_Position = O.hpos;
}
