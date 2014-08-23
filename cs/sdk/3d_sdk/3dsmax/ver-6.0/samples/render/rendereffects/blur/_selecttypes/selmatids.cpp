/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: selMatIds.cpp

	 DESCRIPTION: object ids selection type - class definitions

	 CREATED BY: michael malone (mjm)

	 HISTORY: created July 29, 1999

	 Copyright (c) 1999, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */

// precompiled header
#include "pch.h"

// local includes
#include "selMatIds.h"
#include "..\blurMgr.h"

// ------------------
// initialize statics
// ------------------
const int SelMatIds::maxID = 255;	// maximum 8 bit object id value

// ------------------
// member definitions
// ------------------

SelMatIds::~SelMatIds()
{
	// mp_radMap delete by ~SelLum()
	if (mp_idList)
		delete[] mp_idList;
}

void SelMatIds::notifyPrmChanged(int prmID)
{
	switch (prmID)
	{
		case prmMatIdsIds:
		case prmMatIdsLumMin:
		case prmMatIdsLumMax:
			m_selectValid = m_featherValid = m_compValid = false;
			break;
		case prmMatIdsFeathRad:
		{
			for (int i=0; i<m_imageSz; i++)
			{
				if ( mp_radMap[i] > 0.0f )
					mp_radMap[i] = 1.0f;
			}
			m_featherValid = m_compValid = false;
			break;
		}
		case prmMatIdsBrighten:
		case prmMatIdsBlend:
			m_compValid = false;
			break;
	}
}

bool SelMatIds::doSelect()
{
	float lum;
	int mapIndex;
	mp_nameCurProcess = GetString(IDS_PROCESS_SELECT);

	int interval = 3*m_imageW;
	for (mapIndex=0; mapIndex<m_imageSz; mapIndex++)
	{
		if ( IdInList(mp_srcMatId[mapIndex]) && (lum = luminanceNormFloat(mp_srcColor[mapIndex]) >= m_min) && (lum <= m_max) )
			mp_radMap[mapIndex] = 0.0f;
		else
			mp_radMap[mapIndex] = 1.0f;

		if ( ( (mapIndex%interval) == 0 ) && mp_blurMgr->progress(mp_nameCurProcess, mapIndex, m_imageSz) )
			return false;
	}
	return true;

}

void SelMatIds::select(TimeValue t, CompMap &compMap, Bitmap *bm, RenderGlobalContext *gc)
{
	float fTemp;
	ULONG chanType;

	mp_srcMatId = (UBYTE*)bm->GetChannel(BMM_CHAN_MTL_ID, chanType);
	if (chanType != BMM_CHAN_TYPE_8)
		mp_blurMgr->blurError(IDS_ERR_GBUF_INVALID);
	mp_srcColor = (Color24*)bm->GetChannel(BMM_CHAN_COLOR, chanType);
	if (chanType != BMM_CHAN_TYPE_24)
		mp_blurMgr->blurError(IDS_ERR_GBUF_INVALID);

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
		// make list of desired ids
		m_idCount = mp_blurMgr->getSelTabCount(prmMatIdsIds);
		if (m_idCount > m_idListSz)
		{
			if (mp_idList)
				delete[] mp_idList;
			mp_idList = new int[m_idCount];
			m_idListSz = m_idCount;
		}
		for (int i=0; i<m_idCount; i++)
			mp_blurMgr->getSelValue(prmMatIdsIds, t, mp_idList[i], FOREVER, i);

		// get luminance parameters
		mp_blurMgr->getSelValue(prmMatIdsLumMin, t, fTemp, FOREVER);
		LimitValue(fTemp, 0.0f, 100.0f); // mjm - 9.30.99
		m_min = fTemp*PERCENT2DEC;

		mp_blurMgr->getSelValue(prmMatIdsLumMax, t, fTemp, FOREVER);
		LimitValue(fTemp, 0.0f, 100.0f); // mjm - 9.30.99
		m_max = fTemp*PERCENT2DEC;

		// select the appropriate pixels
		m_selectValid = doSelect();
	}
	
	if (!m_featherValid)
	{
		mp_blurMgr->getSelValue(prmMatIdsFeathRad, t, fTemp, FOREVER);
		LimitValue(fTemp, 0.0f, 1000.0f); // mjm - 9.30.99
		m_feathRad = (int)floor(max(m_imageW, m_imageH)*(fTemp*PERCENT2DEC));
		m_featherValid = doFeather();
	}

	mp_blurMgr->getSelValue(prmMatIdsBrighten, t, fTemp, FOREVER);
	LimitValue(fTemp, 0.0f, 1000.0f); // mjm - 9.30.99
	m_brighten = fTemp*PERCENT2DEC;

	mp_blurMgr->getSelValue(prmMatIdsBlend, t, fTemp, FOREVER);
	LimitValue(fTemp, 0.0f, 100.0f); // mjm - 9.30.99
	m_blend = fTemp*PERCENT2DEC;

	m_compValid = doComposite(t, compMap);
}
