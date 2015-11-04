
#if defined(USE_R2_STATIC_SUN) && !defined(USE_LM_HEMI)
layout(location = TEXCOORD0) in vec4	tcdh;	// Texture coordinates,         w=sun_occlusion
#else
layout(location = TEXCOORD0) in vec2	tcdh;	// Texture coordinates
#endif
layout(location = TEXCOORD1) in vec4	position;	// position + hemi
layout(location = TEXCOORD2) in vec3	M1;	// nmap 2 eye - 1
layout(location = TEXCOORD3) in vec3	M2;	// nmap 2 eye - 2
layout(location = TEXCOORD4) in vec3	M3;	// nmap 2 eye - 3
#if defined(USE_PARALLAX) || defined(USE_STEEPPARALLAX)
layout(location = TEXCOORD5) in vec3	eye;	// vector to point in tangent space
  #ifdef USE_TDETAIL
layout(location = TEXCOORD6) in vec2	tcdbump;	// d-bump
    #ifdef USE_LM_HEMI
layout(location = TEXCOORD7) in vec2	lmh;		// lm-hemi
    #endif
  #else
    #ifdef USE_LM_HEMI
layout(location = TEXCOORD6) in vec2	lmh;		// lm-hemi
    #endif
  #endif
#else
  #ifdef USE_TDETAIL
layout(location = TEXCOORD5) in vec2	tcdbump;	// d-bump
    #ifdef USE_LM_HEMI
layout(location = TEXCOORD6) in vec2	lmh;		// lm-hemi
    #endif
  #else
    #ifdef USE_LM_HEMI
layout(location = TEXCOORD5) in vec2	lmh;		// lm-hemi
    #endif
  #endif
#endif

layout(location = COLOR0) out vec4	P;			// px,py,pz, m-id
layout(location = COLOR1) out vec4	N;			// nx,ny,nz, hemi
layout(location = COLOR2) out vec4	C;			// r, g, b,  gloss

f_deffer _main ( p_bumped I );

void main()
{
	p_bumped I;
	I.tcdh = tcdh;
	I.position = position;
	I.M1 = M1;
	I.M2 = M2;
	I.M3 = M3;
#if defined(USE_PARALLAX) || defined(USE_STEEPPARALLAX)
	I.eye = eye;
#endif
#ifdef USE_TDETAIL
	I.tcdbump = tcdbump;
#endif
#ifdef USE_LM_HEMI
	I.lmh = lmh;
#endif

	f_deffer O = _main(I);

	P = O.position;
	N = O.Ne;
	C = O.C;
}
