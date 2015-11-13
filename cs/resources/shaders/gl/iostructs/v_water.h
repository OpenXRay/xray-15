
out gl_PerVertex { vec4 gl_Position; };

struct	v_vert
{
	float4 	P;		// (float,float,float,1)
	float4	N;		// (nx,ny,nz,hemi occlusion)
	float4 	T;
	float4 	B;
	float4	color;	// (r,g,b,dir-occlusion)
	float2 	uv;		// (u0,v0)
};
struct   vf
{
    float4      hpos;
    float2  	tbase;	// base
    float2      tnorm0;	// nm0
    float2      tnorm1;	// nm1
    half3       M1;
    half3       M2;
    half3       M3;
    half3       v2point;
#ifdef	USE_SOFT_WATER
#ifdef	NEED_SOFT_WATER
	float4      tctexgen;
#endif	//	USE_SOFT_WATER
#endif	//	NEED_SOFT_WATER	
    half4       c0;
    float       fog;
};

layout(location = POSITION)		in float4	P;	// (float,float,float,1)
layout(location = NORMAL)		in float4	N;	// (nx,ny,nz,hemi occlusion)
layout(location = TANGENT)		in float4	T;	// tangent
layout(location = BINORMAL)		in float4	B;	// binormal
layout(location = COLOR0)		in float4	color;	// (r,g,b,dir-occlusion)	//	Swizzle before use!!!
layout(location = TEXCOORD0)	in float2	uv;	// (u,v)

layout(location = TEXCOORD0)	out float2	tbase;  // base
layout(location = TEXCOORD1)	out float2	tnorm0;  // nm0
layout(location = TEXCOORD2)	out float2	tnorm1;  // nm1
layout(location = TEXCOORD3)	out half3 	M1;
layout(location = TEXCOORD4)	out half3	M2;
layout(location = TEXCOORD5)	out half3	M3;
layout(location = TEXCOORD6)	out half3	v2point;
#ifdef	USE_SOFT_WATER
#ifdef	NEED_SOFT_WATER
layout(location = TEXCOORD7)	out float4	tctexgen;
#endif	//	USE_SOFT_WATER
#endif	//	NEED_SOFT_WATER	
layout(location = COLOR0)		out half4	c0;
layout(location = FOG)			out float	fog;

vf _main ( v_vert I );

void main()
{
	v_vert I;
	I.P		= P;
	I.N		= N;
	I.T		= T;
	I.B		= B;
	I.color	= color;
	I.uv	= uv;

	vf O 	= _main (I);

	tbase		= O.tbase;
	tnorm0		= O.tnorm0;
	tnorm1		= O.tnorm1;
	M1			= O.M1;
	M2			= O.M2;
	M3			= O.M3;
	v2point		= O.v2point;
#ifdef	USE_SOFT_WATER
#ifdef	NEED_SOFT_WATER
	tctexgen	= O.tctexgen;
#endif	//	USE_SOFT_WATER
#endif	//	NEED_SOFT_WATER	
	c0			= O.c0;
	fog			= O.fog;
	gl_Position = O.hpos;
}
