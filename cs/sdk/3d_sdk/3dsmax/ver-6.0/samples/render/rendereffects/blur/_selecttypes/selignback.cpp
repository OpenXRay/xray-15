/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: selIgnBack.cpp

	 DESCRIPTION: ignore background selection type - class definitions

	 CREATED BY: michael malone (mjm)

	 HISTORY: created November 4, 1998

	 Copyright (c) 1998, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */

// precompiled header
#include "pch.h"

// local includes
#include "selIgnBack.h"
#include "..\blurMgr.h"

SelIgnBack::~SelIgnBack()
{
	// mp_radMap delete by ~SelLum()
}

void SelIgnBack::notifyPrmChanged(int prmID)
{
	switch (prmID)
	{
		case prmIBackFeathRad:
		{
			for (int i=0; i<m_imageSz; i++)
			{
				if ( mp_radMap[i] > 0.0f )
					mp_radMap[i] = 1.0f;
			}
			m_featherValid = m_compValid = false;
			break;
		}
		case prmIBackBrighten:
		case prmIBackBlend:
			m_compValid = false;
			break;
	}
}

bool SelIgnBack::doSelect()
{
	mp_nameCurProcess = GetString(IDS_PROCESS_SELECT);
	int interval = 3*m_imageW;
	for (int mapIndex=0; mapIndex<m_imageSz; mapIndex++)
	{
		if ( mp_srcAlpha[mapIndex] )
			mp_radMap[mapIndex] = 0.0f;
		else
			mp_radMap[mapIndex] = 1.0f;
		if ( ( (mapIndex%interval) == 0 ) && mp_blurMgr->progress(mp_nameCurProcess, mapIndex, m_imageSz) )
			return false;
	}
	return true;
}

void SelIgnBack::select(TimeValue t, CompMap &compMap, Bitmap *bm, RenderGlobalContext *gc)
{
	float fTemp;
	int type;

	mp_srcAlpha = (WORD*)bm->GetAlphaPtr(&type);
	mp_srcMap = (WORD*)bm->GetStoragePtr(&type);
	assert(type == BMM_TRUE_48);

	// if source bitmap has changed since last call
	if ( m_lastBMModifyID != bm->GetModifyID() )
	{
		m_lastBMModifyID = bm->GetModifyID();
		m_selectValid = m_featherValid = m_compValid = false;
		if ( (bm->Width() != m_imageW) || (bm->Height() != m_imageH) )
		{
			m_imageW = bm->Width();
			m_imageH = bm->Height();
			m_imageSz = m_imageW * m_imageH;
			if (m_imageSz > m_mapSz)
			{
				if (mp_radMap)
					delete[] mp_radMap;
				mp_radMap = new float[m_imageSz];
				m_mapSz = m_imageSz;
			}
		}
	}

	if (!m_selectValid)
	{
		m_selectValid = doSelect();
	}
	
	if (!m_featherValid)
	{
		mp_blurMgr->getSelValue(prmIBackFeathRad, t, fTemp, FOREVER);
		LimitValue(fTemp, 0.0f, 1000.0f); // mjm - 9.30.99
		m_feathRad = (int)floor(max(m_imageW, m_imageH)*(fTemp*PERCENT2DEC));
		m_featherValid = doFeather();
	}

	mp_blurMgr->getSelValue(prmIBackBrighten, t, fTemp, FOREVER);
	LimitValue(fTemp, 0.0f, 1000.0f); // mjm - 9.30.99
	m_brighten = fTemp*PERCENT2DEC;

	mp_blurMgr->getSelValue(prmIBackBlend, t, fTemp, FOREVER);
	LimitValue(fTemp, 0.0f, 100.0f); // mjm - 9.30.99
	m_blend = fTemp*PERCENT2DEC;

	m_compValid = doComposite(t, compMap);
}
