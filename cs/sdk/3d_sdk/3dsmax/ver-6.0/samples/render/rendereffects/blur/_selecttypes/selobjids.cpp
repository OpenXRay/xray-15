/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: selObjIds.cpp

	 DESCRIPTION: object ids selection type - class definitions

	 CREATED BY: michael malone (mjm)

	 HISTORY: created July 29, 1999

	 Copyright (c) 1999, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */

// precompiled header
#include "pch.h"

// local includes
#include "selObjIds.h"
#include "..\blurMgr.h"

// ------------------
// initialize statics
// ------------------
const int SelObjIds::maxID = 65535;	// maximum 16 bit object id value

// ------------------
// member definitions
// ------------------

SelObjIds::~SelObjIds()
{
	// mp_radMap delete by ~SelLum()
	if (mp_idList)
		delete[] mp_idList;
}

void SelObjIds::notifyPrmChanged(int prmID)
{
	switch (prmID)
	{
		case prmObjIdsIds:
		case prmObjIdsLumMin:
		case prmObjIdsLumMax:
			m_selectValid = m_featherValid = m_compValid = false;
			break;
		case prmObjIdsFeathRad:
		{
			for (int i=0; i<m_imageSz; i++)
			{
				if ( mp_radMap[i] > 0.0f )
					mp_radMap[i] = 1.0f;
			}
			m_featherValid = m_compValid = false;
			break;
		}
		case prmObjIdsBrighten:
		case prmObjIdsBlend:
			m_compValid = false;
			break;
	}
}

bool SelObjIds::doSelect()
{
	float lum;
	mp_nameCurProcess = GetString(IDS_PROCESS_SELECT);

	int interval = 3*m_imageW;
	for (int mapIndex=0; mapIndex<m_imageSz; mapIndex++)
	{
		if ( IdInList(mp_srcObjId[mapIndex]) && (lum = luminanceNormFloat(mp_srcColor[mapIndex]) >= m_min) && (lum <= m_max) )
			mp_radMap[mapIndex] = 0.0f;
		else
			mp_radMap[mapIndex] = 1.0f;

		if ( ( (mapIndex%interval) == 0 ) && mp_blurMgr->progress(mp_nameCurProcess, mapIndex, m_imageSz) )
			return false;
	}
	return true;
}

void SelObjIds::select(TimeValue t, CompMap &compMap, Bitmap *bm, RenderGlobalContext *gc)
{
	float fTemp;
	ULONG chanType;

	mp_srcObjId = (WORD *)bm->GetChannel(BMM_CHAN_NODE_ID, chanType);
	if (chanType != BMM_CHAN_TYPE_16)
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
		m_idCount = mp_blurMgr->getSelTabCount(prmObjIdsIds);
		if (m_idCount > m_idListSz)
		{
			if (mp_idList)
				delete[] mp_idList;
			mp_idList = new int[m_idCount];
			m_idListSz = m_idCount;
		}
		for (int i=0; i<m_idCount; i++)
			mp_blurMgr->getSelValue(prmObjIdsIds, t, mp_idList[i], FOREVER, i);

		// get luminance parameters
		mp_blurMgr->getSelValue(prmObjIdsLumMin, t, fTemp, FOREVER);
		LimitValue(fTemp, 0.0f, 100.0f); // mjm - 9.30.99
		m_min = fTemp*PERCENT2DEC;

		mp_blurMgr->getSelValue(prmObjIdsLumMax, t, fTemp, FOREVER);
		LimitValue(fTemp, 0.0f, 100.0f); // mjm - 9.30.99
		m_max = fTemp*PERCENT2DEC;

		// select the appropriate pixels
		m_selectValid = doSelect();
	}
	
	if (!m_featherValid)
	{
		mp_blurMgr->getSelValue(prmObjIdsFeathRad, t, fTemp, FOREVER);
		LimitValue(fTemp, 0.0f, 1000.0f); // mjm - 9.30.99
		m_feathRad = (int)floor(max(m_imageW, m_imageH)*(fTemp*PERCENT2DEC));
		m_featherValid = doFeather();
	}

	mp_blurMgr->getSelValue(prmObjIdsBrighten, t, fTemp, FOREVER);
	LimitValue(fTemp, 0.0f, 1000.0f); // mjm - 9.30.99
	m_brighten = fTemp*PERCENT2DEC;

	mp_blurMgr->getSelValue(prmObjIdsBlend, t, fTemp, FOREVER);
	LimitValue(fTemp, 0.0f, 100.0f); // mjm - 9.30.99
	m_blend = fTemp*PERCENT2DEC;

	m_compValid = doComposite(t, compMap);
}
