
#if defined(USE_R2_STATIC_SUN) && !defined(USE_LM_HEMI)
layout(location = 0) in vec4	tcdh;		// Texture coordinates,         w=sun_occlusion
#else
layout(location = 0) in vec2	tcdh;		// Texture coordinates
#endif
layout(location = 1) in vec4	position;	// position + hemi
layout(location = 2) in vec3	N;			// Eye-space normal        (for lighting)
#ifdef USE_TDETAIL
layout(location = 3) in vec2	tcdbump;	// d-bump
    #ifdef USE_LM_HEMI
layout(location = 4) in vec2	lmh;		// lm-hemi
    #endif
#else
    #ifdef USE_LM_HEMI
layout(location = 3) in vec2	lmh;		// lm-hemi
    #endif
#endif

layout(location = 0) out vec4	P;			// px,py,pz, m-id
layout(location = 1) out vec4	Ne;			// nx,ny,nz, hemi
layout(location = 2) out vec4	C;			// r, g, b,  gloss

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
