
struct   v2p
{
    half2        tbase;  // base
    half2        tdist0;  // d0
    half2        tdist1;  // d1
	float4      tctexgen;
};

layout(location = TEXCOORD0)	in float2	tbase;  // base
layout(location = TEXCOORD1)	in float2	tdist0;  // d0
layout(location = TEXCOORD2)	in float2	tdist1;  // d1
#ifdef	USE_SOFT_WATER
#ifdef	NEED_SOFT_WATER
layout(location = TEXCOORD7)	in float4	tctexgen;
#endif	//	USE_SOFT_WATER
#endif	//	NEED_SOFT_WATER

layout(location = COLOR0) 		out half4	C;

half4 _main( v2p I );

void main()
{
	v2p I;
	I.tbase		= tbase;
	I.tdist0	= tdist0;
	I.tdist1	= tdist1;
#ifdef	USE_SOFT_WATER
#ifdef	NEED_SOFT_WATER
	I.tctexgen	= tctexgen;
#endif	//	USE_SOFT_WATER
#endif	//	NEED_SOFT_WATER

	C	= _main(I);
}
