
out gl_PerVertex { vec4 gl_Position; };

struct vf
{
	float4 hpos;
	float2 tc0;		// base
	float4 c0;		// color
};

layout(location = POSITION)		in vec4	P;	// (float,float,float,1)
layout(location = NORMAL)		in vec4	Nh;	// (nx,ny,nz,hemi occlusion)
layout(location = TANGENT)		in vec4	T;	// tangent
layout(location = BINORMAL)		in vec4	B;	// binormal
layout(location = TEXCOORD0)	in vec2	tc;	// (u,v)
layout(location = TEXCOORD1)	in vec2	lm;	// (lmu,lmv)
layout(location = COLOR0)		in vec4	color;	// (r,g,b,dir-occlusion)	//	Swizzle before use!!!

layout(location = TEXCOORD0) out vec2	tc0;		// base
layout(location = COLOR0) 	out vec4	c0;			// color

vf _main( v_static v );

void	main()
{
	v_static I;
	I.P		= P;
	I.Nh	= Nh;
	I.T		= T;
	I.B		= B;
	I.tc	= tc;
	I.lmh	= lm;
	I.color = color;

	vf O = _main(I);

	tc0			= O.tc0;
	c0			= O.c0;
	gl_Position = O.hpos;
}
