
out gl_PerVertex { vec4 gl_Position; };

struct 	vf
{
	float2 tc0;		// base
	float3 c0;		// color
//	Igor: for additional depth dest
#ifdef	USE_SOFT_PARTICLES
	float4 tctexgen;
#endif	//	USE_SOFT_PARTICLES
	float  fog;
	float4 hpos;
};

layout(location = POSITION)	in vec4 P;		// (float,float,float,1)
#ifdef 	SKIN_0
layout(location = NORMAL)	in vec3 norm;	// (nx,ny,nz)
#else
layout(location = NORMAL)	in vec4 norm;	// (nx,ny,nz)
#endif
#if defined(SKIN_3) || defined(SKIN_4)
layout(location = TANGENT)	in vec4 tan;	// (nx,ny,nz)
layout(location = BINORMAL)	in vec4 bnorm;	// (nx,ny,nz)
#else
layout(location = TANGENT)	in vec3 tan;	// (nx,ny,nz)
layout(location = BINORMAL)	in vec3 bnorm;	// (nx,ny,nz)
#endif
#if defined(SKIN_2) || defined(SKIN_3)
layout(location = TEXCOORD0) in vec4 uv;	// (u,v)
#else
layout(location = TEXCOORD0) in vec2 uv;	// (u,v)
#endif
#ifdef 	SKIN_4
layout(location = TEXCOORD1) in vec4 ind;
#endif

layout(location = TEXCOORD0) out vec2	tc0;		// base
layout(location = COLOR0) 	out vec3	c0;			// color
#ifdef	USE_SOFT_PARTICLES
layout(location = TEXCOORD1) out vec4	tctexgen;
#endif	//	USE_SOFT_PARTICLES
layout(location = FOG) 		out float	fog;

vf _main( v_model v );

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
	I.tc = uv;

	vf O;
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
	c0			= O.c0;
#ifdef	USE_SOFT_PARTICLES
	tctexgen	= O.tctexgen;
#endif	//	USE_SOFT_PARTICLES
	fog			= O.fog;
	gl_Position = O.hpos;
}
