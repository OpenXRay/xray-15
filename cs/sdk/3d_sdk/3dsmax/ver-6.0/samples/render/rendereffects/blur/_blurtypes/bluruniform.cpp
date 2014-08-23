/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: blurUniform.cpp

	 DESCRIPTION: uniform blur type - class definitions

	 CREATED BY: michael malone (mjm)

	 HISTORY: created November 4, 1998

	 Copyright (c) 1998, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */

// precompiled header
#include "pch.h"

// local includes
#include "blurUniform.h"
#include "..\blurMgr.h"


BlurUniform::~BlurUniform()
{
	// mp_blurCache delete by ~BlurDirectional
}

void BlurUniform::notifyPrmChanged(int prmID)
{
	switch (prmID)
	{
		case prmUnifPixRad:
			m_blurValid = false;
		case prmUnifAlpha:
			break;
	}
}

bool BlurUniform::doBlur(CompMap *pCompMap, WORD *mapFrom, WORD *mapTo, WORD *alphaFrom, WORD *alphaTo)
{
	// since we're subclassed from BlurDirectional, do a non-rotated directional blur
	m_uPixRad = m_vPixRad = m_pixRad;
	m_uTrail = m_vTrail = 0.0f;
	return BlurDirectional::doBlur(pCompMap, mapFrom, mapTo, alphaFrom, alphaTo);
}

void BlurUniform::blur(TimeValue t, CompMap *pCompMap, Bitmap *bm, RenderGlobalContext *gc)
{
	// get source bitmap data
	int type;
	mp_srcAlpha = (WORD*)bm->GetAlphaPtr(&type);
	mp_srcMap = (WORD*)bm->GetStoragePtr(&type);
	assert(type == BMM_TRUE_48);

	// if source bitmap has changed since last call
	if ( m_lastBMModifyID != bm->GetModifyID() )
	{
		m_lastBMModifyID = bm->GetModifyID();
		m_blurValid = false;
		if ( (bm->Width() != m_imageW) || (bm->Height() != m_imageH) )
		{
			m_imageW = bm->Width();
			m_imageH = bm->Height();
			m_imageSz = m_imageW * m_imageH;
		}
	}

	// get ui parameters
	float fTemp;
	mp_blurMgr->getBlurValue(prmUnifPixRad, t, fTemp, FOREVER);
	LimitValue(fTemp, 0.0f, 1000.0f); // mjm - 9.30.99
	m_pixRad = (int)( 0.5*floor( max(m_imageW, m_imageH)*(fTemp*PERCENT2DEC) ) );
	mp_blurMgr->getBlurValue(prmUnifAlpha, t, m_affectAlpha, FOREVER);
	mp_blurMgr->getSelValue(prmGenBrightType, t, m_brightenType, FOREVER);

	// setup buffers
	if (m_imageSz > m_mapSz)
	{
		if (mp_scratchMap)
			delete[] mp_scratchMap;
		mp_scratchMap = new AColor[m_imageSz];

		if (mp_blurCache)
			delete[] mp_blurCache;
		mp_blurCache = new AColor[m_imageSz];

		m_mapSz = m_imageSz;
	}

	m_blurValid = doBlur(pCompMap, mp_srcMap, mp_srcMap, mp_srcAlpha, (m_affectAlpha) ? mp_srcAlpha : NULL);
};
