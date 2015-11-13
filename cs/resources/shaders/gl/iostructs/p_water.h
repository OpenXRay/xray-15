
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
//    float       fog;
};

layout(location = TEXCOORD0)	in float2	tbase;  // base
layout(location = TEXCOORD1)	in float2	tnorm0;  // nm0
layout(location = TEXCOORD2)	in float2	tnorm1;  // nm1
layout(location = TEXCOORD3)	in half3 	M1;
layout(location = TEXCOORD4)	in half3	M2;
layout(location = TEXCOORD5)	in half3	M3;
layout(location = TEXCOORD6)	in half3	v2point;
#ifdef	USE_SOFT_WATER
#ifdef	NEED_SOFT_WATER
layout(location = TEXCOORD7)	in float4	tctexgen;
#endif	//	USE_SOFT_WATER
#endif	//	NEED_SOFT_WATER	
layout(location = COLOR0)		in half4	c0;
//layout(location = FOG)			in float	fog;

layout(location = COLOR0) 		out half4	C;

half4 _main( vf I );

void main()
{
	vf I;
	I.tbase		= tbase;
	I.tnorm0	= tnorm0;
	I.tnorm1	= tnorm1;
	I.M1		= M1;
	I.M2		= M2;
	I.M3		= M3;
	I.v2point	= v2point;
#ifdef	USE_SOFT_WATER
#ifdef	NEED_SOFT_WATER
	I.tctexgen	= tctexgen;
#endif	//	USE_SOFT_WATER
#endif	//	NEED_SOFT_WATER
	I.c0		= c0;
//	I.fog		= fog;

	C	= _main(I);
}
