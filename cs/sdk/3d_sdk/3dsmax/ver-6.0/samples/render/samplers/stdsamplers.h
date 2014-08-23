///////////////////////////////////////////////////////////////
//
//	Standard Samplers functions
//

#ifndef STDSAMPLERS_H
#define STDSAMPLERS_H

// Common parameter block ids
#define	PB_QUALITY		0
#define	PB_ENABLE		1
#define	PB_SUBSAMP_TEX	2

#define	PB_ADAPT_ENABLE	3
#define	PB_ADAPT_THRESHOLD	4
#define	PB_SUBSAMP_TEX_ADAPT	5


// Class ids

#define SINGLE_SAMPLER_CLASS_ID		0x25773210
//#define R25_SAMPLER_CLASS_ID		0x25773211
#define UNIFORM_SAMPLER_CLASS_ID	0x25773212
#define CMJ_SAMPLER_CLASS_ID		0x25773213
#define HAMMERSLEY_SAMPLER_CLASS_ID	0x25773214
#define HALTON_SAMPLER_CLASS_ID		0x25773215
#define AHALTON_SAMPLER_CLASS_ID	0x25773216
#define ACMJ_SAMPLER_CLASS_ID		0x25773217


// Sampler Class Descriptors
extern ClassDesc* GetSingleSamplerDesc();
extern ClassDesc* GetR25SamplerDesc();
extern ClassDesc* GetUniformSamplerDesc();
extern ClassDesc* GetCMJSamplerDesc();
extern ClassDesc* GetHammersleySamplerDesc();
extern ClassDesc* GetHaltonSamplerDesc();
extern ClassDesc* GetAHaltonSamplerDesc();
extern ClassDesc* GetACMJSamplerDesc();




#endif

