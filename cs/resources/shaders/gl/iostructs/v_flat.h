
out gl_PerVertex { vec4 gl_Position; };

layout(location = 0) in vec4	P;			// (float,float,float,1)
layout(location = 1) in vec4	Nh;			// (nx,ny,nz,hemi occlusion)
layout(location = 2) in vec4	T;			// tangent
layout(location = 3) in vec4	B;			// binormal
layout(location = 4) in vec4	color;		// (r,g,b,dir-occlusion)	//	Swizzle before use!!!
layout(location = 5) in vec2	tc;			// (u,v)
#if	defined(USE_LM_HEMI)
layout(location = 6) in vec2	lm;			// (lmu,lmv)
#endif

#if defined(USE_R2_STATIC_SUN) && !defined(USE_LM_HEMI)
layout(location = 0) out vec4	tcdh;		// Texture coordinates,         w=sun_occlusion
#else
layout(location = 0) out vec2	tcdh;		// Texture coordinates
#endif
layout(location = 1) out vec4	position;	// position + hemi
layout(location = 2) out vec3	N;			// Eye-space normal        (for lighting)
#ifdef USE_TDETAIL
layout(location = 3) out vec2	tcdbump;	// d-bump
	#ifdef USE_LM_HEMI
layout(location = 4) out vec2	lmh;		// lm-hemi
	#endif
#else
	#ifdef USE_LM_HEMI
layout(location = 3) out vec2	lmh;		// lm-hemi
	#endif
#endif

p_flat _main ( v_static I );

void main()
{
	v_static I;
	I.P		= P;
	I.Nh	= Nh;
	I.T		= T;
	I.B		= B;
	I.color = color;
//	I.color.rgb 	= I.color.bgr;	//	Swizzle to compensate DX9/OGL format mismatch
	I.tc	= tc;
#if	defined(USE_LM_HEMI)
	I.lmh	= lm;
#endif

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
