
out gl_PerVertex { vec4 gl_Position; };

layout(location = 0) in vec4 P;		// (float,float,float,1)
#ifdef 	SKIN_0
layout(location = 1) in vec3 norm;	// (nx,ny,nz)
#else
layout(location = 1) in vec4 norm;	// (nx,ny,nz)
#endif
#if defined(SKIN_3) || defined(SKIN_4)
layout(location = 2) in vec4 tan;	// (nx,ny,nz)
layout(location = 3) in vec4 bnorm;	// (nx,ny,nz)
#else
layout(location = 2) in vec3 tan;	// (nx,ny,nz)
layout(location = 3) in vec3 bnorm;	// (nx,ny,nz)
#endif
#if defined(SKIN_2) || defined(SKIN_3)
layout(location = 4) in vec4 tc;	// (u,v)
#else
layout(location = 4) in vec2 tc;	// (u,v)
#endif
#ifdef 	SKIN_4
layout(location = 5) in vec4 ind;
#endif

#ifdef  USE_HWSMAP
layout(location = 0) out vec2	tc0;	// Diffuse map for aref
#else
layout(location = 0) out float	depth;	// Depth
layout(location = 1) out vec2	tc0;	// Diffuse map for aref
#endif

v_shadow_direct_aref _main( v_model	I );

void	main()
{
#ifdef 	SKIN_NONE
	v_model I;
#endif
#ifdef 	SKIN_0
	v_model_skinned_0 I;
#endif
#ifdef	SKIN_1
	v_model_skinned_1 I;
#endif
#ifdef	SKIN_2
	v_model_skinned_2 I;
#endif
#ifdef	SKIN_3
	v_model_skinned_3 I;
#endif
#ifdef	SKIN_4
	v_model_skinned_4 I;
	I.ind = ind;
#endif

	I.P = P;
	I.N = norm;
	I.T = tan;
	I.B = bnorm;
	I.tc = tc;

	v_shadow_direct_aref O;
#ifdef 	SKIN_NONE
	O = _main(I);
#endif
#ifdef 	SKIN_0
	O = _main(skinning_0(I));
#endif
#ifdef	SKIN_1
	O = _main(skinning_1(I));
#endif
#ifdef	SKIN_2
	O = _main(skinning_2(I));
#endif
#ifdef	SKIN_3
	O = _main(skinning_3(I));
#endif
#ifdef	SKIN_4
	O = _main(skinning_4(I));
#endif

	tc0			= O.tc0;
#ifndef  USE_HWSMAP
	depth		= O.depth;
#endif
	gl_Position = O.hpos;
}
