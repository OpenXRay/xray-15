
#if defined(USE_R2_STATIC_SUN) && !defined(USE_LM_HEMI)
layout(location = TEXCOORD0) in vec4	tcdh;		// Texture coordinates,         w=sun_occlusion
#else
layout(location = TEXCOORD0) in vec2	tcdh;		// Texture coordinates
#endif
layout(location = TEXCOORD1) in vec4	position;	// position + hemi
layout(location = TEXCOORD2) in vec3	N;			// Eye-space normal        (for lighting)
#ifdef USE_TDETAIL
layout(location = TEXCOORD3) in vec2	tcdbump;	// d-bump
    #ifdef USE_LM_HEMI
layout(location = TEXCOORD4) in vec2	lmh;		// lm-hemi
    #endif
#else
    #ifdef USE_LM_HEMI
layout(location = TEXCOORD3) in vec2	lmh;		// lm-hemi
    #endif
#endif

layout(location = COLOR0) out vec4	P;			// px,py,pz, m-id
layout(location = COLOR1) out vec4	Ne;			// nx,ny,nz, hemi
layout(location = COLOR2) out vec4	C;			// r, g, b,  gloss

f_deffer _main( p_flat I );

void main()
{
	p_flat I;
	I.tcdh		= tcdh;
	I.position	= position;
	I.N			= N;
#ifdef USE_TDETAIL
	I.tcdbump	= tcdbump;
#endif
#ifdef USE_LM_HEMI
	I.lmh		= lmh;
#endif

	f_deffer O	= _main(I);

	P 	= O.position;
	Ne	= O.Ne;
	C	= O.C;
}
