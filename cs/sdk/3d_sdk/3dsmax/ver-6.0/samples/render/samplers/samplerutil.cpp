/////////////////////////////////////////////////////////////////////////
// 
//	FILE: SamplerUtil.cpp	
//
//	DESCRIPTION:Standard sampler plugin utilities
//
//	Created Kells Elmquist, 8 dec 1998
//

#include "samplersHdr.h"
#include "samplerUtil.h"
#include "stdSamplers.h"

// from old rendpost code
// #define mul 0x00004000 
// #define mur 0x00000200 
// #define mll 0x00400000
// #define mlr 0x00020000
//			if (ab->mask[1]&mlr) { // Lower Right (+X +Y)
//			if (ab->mask[0]&mur) { // Upper Right (+X -Y)
//			if (ab->mask[0]&mul) { // Upper Left (-X -Y)
//			if (ab->mask[1]&mll) { // Lower Left (-X +Y)

BOOL sampleInMask( Point2& sample,  MASK m, BOOL fieldRendering )
{
	int x = int( sample.x * 8.0f );	// cd be 7.999
	x = bound( x, 0, 7 );
	
	float yIn = sample.y;
	if( fieldRendering )
		yIn *= 0.5f;
	int y = int( yIn * 8.0f );
	y = bound( y, 0, 7 );

	BYTE * pMask = (BYTE*)m;
	BYTE b = pMask[ y ];		// >>>>< polarity? 8-y ?

	BYTE in = b & (0x80 >> x) ;		// >>>>< polarity?
	return in > 0;
}


RefResult intNotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	GetParamName * gpn;

	switch (message) {
		case REFMSG_CHANGE:
			;
			break;

		case REFMSG_GET_PARAM_DIM: {
			GetParamDim * gpd = (GetParamDim*)partID;
			switch (gpd->index) {
				case PB_ENABLE: gpd->dim = defaultDim; break; //>>>>>< BOOL ??
				case PB_QUALITY: gpd->dim = defaultDim; break;
				case PB_SUBSAMP_TEX: gpd->dim = defaultDim; break; //>>>>>< BOOL ??
				default: 	     gpd->dim = defaultDim; break;
			}
			return REF_STOP; 
		}

		case REFMSG_GET_PARAM_NAME: {
			gpn = (GetParamName*)partID;
			switch (gpn->index) {
				case PB_ENABLE : gpn->name = _T( GetString(IDS_KE_ENABLE) ); break;
				case PB_QUALITY : gpn->name = _T( GetString(IDS_KE_QUALITY) ); break;
				case PB_SUBSAMP_TEX : gpn->name = _T( GetString(IDS_KE_SUBSAMP_TEX) ); break;
				default:		  gpn->name = _T(""); break;
			}
			return REF_STOP; 
		}
	}
	return REF_SUCCEED;
}


RefResult adaptNotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	GetParamName * gpn;

	switch (message) {
		case REFMSG_CHANGE:
			;
			break;

		case REFMSG_GET_PARAM_DIM: {
			GetParamDim * gpd = (GetParamDim*)partID;
			switch (gpd->index) {
				case PB_ENABLE: gpd->dim = defaultDim; break; //>>>>>< BOOL ??
				case PB_QUALITY: gpd->dim = defaultDim; break;
				case PB_ADAPT_ENABLE: gpd->dim = defaultDim; break; //>>>>>< BOOL ??
				case PB_ADAPT_THRESHOLD: gpd->dim = defaultDim; break;
				case PB_SUBSAMP_TEX_ADAPT: gpd->dim = defaultDim; break; //>>>>>< BOOL ??
				default: 	     gpd->dim = defaultDim; break;
			}
			return REF_STOP; 
		}

		case REFMSG_GET_PARAM_NAME: {
			gpn = (GetParamName*)partID;
			switch (gpn->index) {
				case PB_ENABLE : gpn->name = _T( GetString(IDS_KE_ENABLE) ); break;
				case PB_QUALITY : gpn->name = _T( GetString(IDS_KE_QUALITY) ); break;
				case PB_SUBSAMP_TEX_ADAPT : gpn->name = _T( GetString(IDS_KE_SUBSAMP_TEX) ); break;
				case PB_ADAPT_ENABLE : gpn->name = _T( GetString(IDS_KE_ADAPT_ENABLE) ); break;
				case PB_ADAPT_THRESHOLD : gpn->name = _T( GetString(IDS_KE_ADAPT_THRESHOLD) ); break;
				default:		  gpn->name = _T(""); break;
			}
			return REF_STOP; 
		}
	}
	return REF_SUCCEED;
}

