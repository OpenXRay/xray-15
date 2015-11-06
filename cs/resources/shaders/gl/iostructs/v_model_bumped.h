
out gl_PerVertex { vec4 gl_Position; };

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
layout(location = TEXCOORD0) in vec4 tc0;	// (u,v)
#else
layout(location = TEXCOORD0) in vec2 tc0;	// (u,v)
#endif
#ifdef 	SKIN_4
layout(location = TEXCOORD1) in vec4 ind;
#endif

#if defined(USE_R2_STATIC_SUN) && !defined(USE_LM_HEMI)
layout(location = TEXCOORD0) out vec4	tcdh;		// Texture coordinates,         w=sun_occlusion
#else
layout(location = TEXCOORD0) out vec2	tcdh;		// Texture coordinates
#endif
layout(location = TEXCOORD1) out vec4	position;	// position + hemi
layout(location = TEXCOORD2) out vec3	M1;			// nmap 2 eye - 1
layout(location = TEXCOORD3) out vec3	M2;			// nmap 2 eye - 2
layout(location = TEXCOORD4) out vec3	M3;	// nmap 2 eye - 3
#if defined(USE_PARALLAX) || defined(USE_STEEPPARALLAX)
layout(location = TEXCOORD5) out vec3	eye;		// vector to point in tangent space
  #ifdef USE_TDETAIL
layout(location = TEXCOORD6) out vec2	tcdbump;	// d-bump
    #ifdef USE_LM_HEMI
layout(location = TEXCOORD7) out vec2	lmh;		// lm-hemi
    #endif
  #else
    #ifdef USE_LM_HEMI
layout(location = TEXCOORD6) out vec2	lmh;		// lm-hemi
    #endif
  #endif
#else
  #ifdef USE_TDETAIL
layout(location = TEXCOORD5) out vec2	tcdbump;	// d-bump
    #ifdef USE_LM_HEMI
layout(location = TEXCOORD6) out vec2	lmh;		// lm-hemi
    #endif
  #else
    #ifdef USE_LM_HEMI
layout(location = TEXCOORD5) out vec2	lmh;		// lm-hemi
    #endif
  #endif
#endif

p_bumped _main( v_model I );

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
	I.tc = tc0;

	p_bumped O;
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

	tcdh		= O.tcdh;
	position	= O.position;
	M1			= O.M1;
	M2			= O.M2;
	M3			= O.M3;
#if defined(USE_PARALLAX) || defined(USE_STEEPPARALLAX)
	eye			= O.eye;
#endif
#ifdef USE_TDETAIL
	tcdbump		= O.tcdbump;
#endif
#ifdef USE_LM_HEMI
	lmh			= O.lmh;
#endif
	gl_Position = O.hpos;
}
