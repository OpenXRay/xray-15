
out gl_PerVertex { vec4 gl_Position; };

layout(location = POSITION)		in vec4	P;		// (float,float,float,1)
layout(location = NORMAL)		in vec4	Nh;		// (nx,ny,nz)
layout(location = TANGENT)		in vec3	T;		// tangent
layout(location = BINORMAL)		in vec3	B;		// binormal
layout(location = TEXCOORD0)	in vec4	tc;		// (u,v,frac,???)

#if defined(USE_R2_STATIC_SUN) && !defined(USE_LM_HEMI)
layout(location = TEXCOORD0) out vec4	tcdh;		// Texture coordinates,         w=sun_occlusion
#else
layout(location = TEXCOORD0) out vec2	tcdh;		// Texture coordinates
#endif
layout(location = TEXCOORD1) out vec4	position;	// position + hemi
layout(location = TEXCOORD2) out vec3	N;			// Eye-space normal        (for lighting)
#ifdef USE_TDETAIL
layout(location = TEXCOORD3) out vec2	tcdbump;	// d-bump
	#ifdef USE_LM_HEMI
layout(location = TEXCOORD4) out vec2	lmh;		// lm-hemi
	#endif
#else
	#ifdef USE_LM_HEMI
layout(location = TEXCOORD3) out vec2	lmh;		// lm-hemi
	#endif
#endif

p_flat _main ( v_tree I );

void main()
{
	v_tree I;
	I.P		= P;
	I.Nh	= Nh;
	I.T		= T;
	I.B		= B;
	I.tc	= tc;

	p_flat O 	= _main (I);

	tcdh		= O.tcdh;
	position	= O.position;
	N			= O.N;
#ifdef USE_TDETAIL
	tcdbump		= O.tcdbump;
#endif
#ifdef USE_LM_HEMI
	lmh			= O.lmh;
#endif
	gl_Position = O.hpos;
}
