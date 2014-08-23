/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: selImage.cpp

	 DESCRIPTION: whole image selection type - class definitions

	 CREATED BY: michael malone (mjm)

	 HISTORY: created November 4, 1998

	 Copyright (c) 1998, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */

// precompiled header
#include "pch.h"

// local includes
#include "selImage.h"
#include "..\blurMgr.h"

float SelImage::getBrighten(TimeValue t)
{
	float fTemp;
	mp_blurMgr->getSelValue(prmImageBrighten, t, fTemp, FOREVER);
	LimitValue(fTemp, 0.0f, 1000.0f); // mjm - 9.30.99
	m_brighten = fTemp*PERCENT2DEC;

	return m_brighten;
};

float SelImage::getBlend(TimeValue t)
{
	float fTemp;
	mp_blurMgr->getSelValue(prmImageBlend, t, fTemp, FOREVER);
	LimitValue(fTemp, 0.0f, 100.0f); // mjm - 9.30.99
	m_blend = fTemp*PERCENT2DEC;

	return m_blend;
};
