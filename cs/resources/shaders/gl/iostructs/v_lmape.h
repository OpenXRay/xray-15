
out gl_PerVertex { vec4 gl_Position; };

struct 	v_lmap
{
	float4 	pos;	// (float,float,float,1)
	float4	norm;	// (nx,ny,nz,hemi occlusion)
	float2 	tc0;	// (base)
	float2	tc1;	// (lmap/compressed)
};
struct vf
{
	float4 hpos;
	float2 tc0;
	float2 tc1;
	float2 tch;
	float3 tc2;
	float3 c0;		// c0=hemi+v-lights, 	c0.a = dt*
	float3 c1;		// c1=sun, 		c1.a = dt+
	float  fog;
};

layout(location = POSITION)		in vec4	P;	// (float,float,float,1)
layout(location = NORMAL)		in vec4	Nh;	// (nx,ny,nz,hemi occlusion)
layout(location = TANGENT)		in vec4	T;	// tangent
layout(location = BINORMAL)		in vec4	B;	// binormal
layout(location = TEXCOORD0)	in vec2	tc;	// (u,v)
layout(location = TEXCOORD1)	in vec2	lm;	// (lmu,lmv)
layout(location = COLOR0)		in vec4	color;	// (r,g,b,dir-occlusion)	//	Swizzle before use!!!

layout(location = TEXCOORD0)	out float2 tc0;
layout(location = TEXCOORD1)	out float2 tc1;
layout(location = TEXCOORD2)	out float2 tch;
layout(location = TEXCOORD3)	out float3 tc2;
layout(location = COLOR0)		out float3 c0;		// c0=hemi+v-lights, 	c0.a = dt*
layout(location = COLOR1)		out float3 c1;		// c1=sun, 		c1.a = dt+
layout(location = FOG)			out float  fog;

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
	tc1			= O.tc1;
	tch			= O.tch;
	tc2			= O.tc2;
	c0			= O.c0;
	c1			= O.c1;
	fog			= O.fog;
	gl_Position = O.hpos;
}
