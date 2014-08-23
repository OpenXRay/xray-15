/////////////////////////////////////////////////////////////////////////
// 
//	FILE: SamplerUtil.h	
//
//	DESCRIPTION:Standard sampler plugin utilities
//
//	Created Kells Elmquist, 8 dec 1998
//

#ifndef SAMPLER_UTIL_H
#define SAMPLER_UTIL_H


#define ALL_ONES	0xffffffffL

inline void setMask( MASK m, ULONG v ) { m[0] = m[1] = v; }
inline void copyMask( MASK to, MASK from ) { to[0] = from[0]; to[1]=from[1];}
inline BOOL centerInMask( MASK m ){ return TRUE; } //>>>>>><

inline float bound( float v, float min=0.0f, float max=1.0f )
	{ return (v < min)? min : (v > max)? max : v; }
inline int bound( int v, int min=0, int max=1 )
	{ return (v < min)? min : (v > max)? max : v; }


BOOL sampleInMask( Point2& sample,  MASK m, BOOL fieldRendering );


RefResult intNotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message);

RefResult adaptNotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message); 

// fixed, base 2....tests show this one works the best
inline float radicalInverse2( int n ) {
	float out = 0.0f;
	float digitVal = 0.5;
	for( int i = n; i > 0; i >>=1 ) {
		if ( i & 0x1 )
			out += digitVal;
		digitVal *= 0.5f;
	}
	return out;
}

inline float radicalInverse( int n, int base ) {
	float out = 0.0f;
	float digitMult = 1.0f / float(base);
	float digitVal = digitMult;
	for( int i = n; i > 0; i /= base ) {
		int d = i % base;
		if ( d )
			out += d * digitVal;
		digitVal *= digitMult;
	}
	return out;
}



#endif