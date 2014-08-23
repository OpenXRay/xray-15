/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: selMaps.cpp

	 DESCRIPTION: maps selection type - class definitions

	 CREATED BY: michael malone (mjm)

	 HISTORY: created November 4, 1998

	 Copyright (c) 1998, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */

// precompiled header
#include "pch.h"

// local includes
#include "selMaps.h"
#include "..\blurMgr.h"

SelMaps::~SelMaps()
{
	// mp_radMap delete by ~SelLum()
}

void SelMaps::notifyPrmChanged(int prmID)
{
	switch (prmID)
	{
		case prmMaskMap:
		case prmMaskChannel:
		case prmMaskMin:
		case prmMaskMax:
			m_selectValid = m_featherValid = m_compValid = false;
			break;
		case prmMaskFeathRad:
		{
			for (int i=0; i<m_imageSz; i++)
			{
				if ( mp_radMap[i] > 0.0f )
					mp_radMap[i] = 1.0f;
			}
			m_featherValid = m_compValid = false;
			break;
		}
		case prmMaskBrighten:
		case prmMaskBlend:
			m_compValid = false;
			break;
	}
}

bool SelMaps::testPixel(int index)
{
	int x = index % m_imageW;
	int y = index / m_imageW;									// > 5/16/01 - 10:45pm --mm-- was m_imageH, providing wrong y position

	m_shadeContext.uvw.x = (x+0.5f) * m_shadeContext.duvw.x;
	m_shadeContext.uvw.y = (m_imageH-y-0.5f) * m_shadeContext.duvw.y;
	m_shadeContext.pt = m_shadeContext.uvw;
	m_shadeContext.scrPos.x = x;
	m_shadeContext.scrPos.y = y;

	AColor c = m_brightenMap->EvalColor(m_shadeContext);

	float val;
	switch (m_channel)
	{
	case red:
		val = c.r;
		break;
	case green:
		val = c.g;
		break;
	case blue:
		val = c.b;
		break;
	case alpha:
		val = c.a;
		break;
	case luminance:
		val = luminanceNormFloat(c.r, c.g, c.b);
		break; 
	}
	if ( val > m_min && val <= m_max )
		return true;
	else
		return false;
}

bool SelMaps::doSelect()
{
	mp_nameCurProcess = GetString(IDS_PROCESS_SELECT);
	int interval = 3*m_imageW;
	for (int mapIndex=0; mapIndex<m_imageSz; mapIndex++)
	{
		mp_radMap[mapIndex] = ( testPixel(mapIndex) ) ? 0.0f : 1.0f;
		if ( ( (mapIndex%interval) == 0 ) && mp_blurMgr->progress(mp_nameCurProcess, mapIndex, m_imageSz) )
			return false;
	}

	return true;
}

void SelMaps::select(TimeValue t, CompMap &compMap, Bitmap *bm, RenderGlobalContext *gc)
{
	int type;

	mp_srcAlpha = (WORD*)bm->GetAlphaPtr(&type);
	mp_srcMap = (WORD*)bm->GetStoragePtr(&type);
	assert(type == BMM_TRUE_48);
	m_shadeContext.globContext = gc;

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

	float fTemp;
	if (!m_selectValid)
	{
		m_brightenMap = NULL;
		mp_blurMgr->getSelValue(prmMaskMap, 0, m_brightenMap, FOREVER);
		if (!m_brightenMap)
			return;
		m_shadeContext.scale = 1.0f;
		m_shadeContext.duvw  = Point3(1.0f/float(m_imageW), 1.0f/float(m_imageH), 0.0f);
		m_shadeContext.uvw.z = 0.0f;
		m_shadeContext.filterMaps = TRUE;
		m_brightenMap->Update(t, Interval());
		m_brightenMap->LoadMapFiles(t);

		mp_blurMgr->getSelValue(prmMaskChannel, t, m_channel, FOREVER);
		mp_blurMgr->getSelValue(prmMaskMin, t, fTemp, FOREVER);
		LimitValue(fTemp, 0.0f, 100.0f); // mjm - 9.30.99
		m_min = fTemp*PERCENT2DEC;

		mp_blurMgr->getSelValue(prmMaskMax, t, fTemp, FOREVER);
		LimitValue(fTemp, 0.0f, 100.0f); // mjm - 9.30.99
		m_max = fTemp*PERCENT2DEC;

		m_selectValid = doSelect();
	}

	if (!m_featherValid)
	{
		mp_blurMgr->getSelValue(prmMaskFeathRad, t, fTemp, FOREVER);
		LimitValue(fTemp, 0.0f, 1000.0f); // mjm - 9.30.99
		m_feathRad = (int)floor(max(m_imageW, m_imageH)*(fTemp*PERCENT2DEC));
		m_featherValid = doFeather();
	}

	mp_blurMgr->getSelValue(prmMaskBrighten, t, fTemp, FOREVER);
	LimitValue(fTemp, 0.0f, 1000.0f); // mjm - 9.30.99
	m_brighten = fTemp*PERCENT2DEC;

	mp_blurMgr->getSelValue(prmMaskBlend, t, fTemp, FOREVER);
	LimitValue(fTemp, 0.0f, 100.0f); // mjm - 9.30.99
	m_blend = fTemp*PERCENT2DEC;

	m_compValid = doComposite(t, compMap);
}
