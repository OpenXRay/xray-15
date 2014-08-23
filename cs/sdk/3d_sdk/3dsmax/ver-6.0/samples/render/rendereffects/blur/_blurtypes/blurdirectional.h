/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: blurDirectional.h

	 DESCRIPTION: directional blur type -  class declaration

	 CREATED BY: michael malone (mjm)

	 HISTORY: created November 4, 1998

	 Copyright (c) 1998, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */

#if !defined(_BLUR_DIRECTIONAL_H_INCLUDED_)
#define _BLUR_DIRECTIONAL_H_INCLUDED_

#include "blurBase.h"

class BlurDirectional : public BlurBase
{
protected:
	bool m_rotValid;
	int m_uPixRad, m_vPixRad, m_numURotPix, m_numVRotPix, m_szURotPix, m_szVRotPix, m_szUFilter, m_szVFilter,
		m_leftRad, m_rightRad, m_upRad, m_downRad, m_leftRot, m_rightRot, m_upRot, m_downRot;
	float m_uTrail, m_vTrail, m_rot;
	float *mp_uFilter, *mp_vFilter;
	IPoint2 *mp_uRotPix, *mp_vRotPix;


	bool calcRotUPixOffsets();
	bool calcRotVPixOffsets();
	bool do1DBoxRot(CompMap *pCompMap, WORD *mapFrom, WORD *mapTo, WORD *alphaFrom, WORD *alphaTo);
	bool do1DBoxNoRot(CompMap *pCompMap, WORD *mapFrom, WORD *mapTo, WORD *alphaFrom, WORD *alphaTo);
	bool do2DBoxRot(CompMap *pCompMap, WORD *mapFrom, WORD *mapTo, WORD *alphaFrom, WORD *alphaTo);
	bool do2DBoxNoRot(CompMap *pCompMap, WORD *mapFrom, WORD *mapTo, WORD *alphaFrom, WORD *alphaTo);
	virtual bool doBlur(CompMap *pCompMap, WORD *mapFrom, WORD *mapTo, WORD *alphaFrom, WORD *alphaTo);

public:
	BlurDirectional() : m_rotValid(false), m_uPixRad(0), m_vPixRad(0), m_numURotPix(0), m_numVRotPix(0), m_szURotPix(0), m_szVRotPix(0),
		m_szUFilter(0), m_szVFilter(0), m_leftRad(-1), m_rightRad(-1), m_upRad(-1), m_downRad(-1), // mjm - 10.18.99
		m_leftRot(0), m_rightRot(0), m_upRot(0), m_downRot(0), m_uTrail(0.0f), m_vTrail(0.0f), m_rot(0.0f),
		mp_uFilter(NULL), mp_vFilter(NULL), mp_uRotPix(NULL), mp_vRotPix(NULL) { }
	BlurDirectional(BlurMgr *const mgr) : m_rotValid(false), m_uPixRad(0), m_vPixRad(0), m_numURotPix(0), m_numVRotPix(0), m_szURotPix(0), m_szVRotPix(0),
		m_szUFilter(0), m_szVFilter(0), m_leftRad(-1), m_rightRad(-1), m_upRad(-1), m_downRad(-1), // mjm - 10.18.99
		m_leftRot(0), m_rightRot(0), m_upRot(0), m_downRot(0), m_uTrail(0.0f), m_vTrail(0.0f), m_rot(0.0f),
		mp_uFilter(NULL), mp_vFilter(NULL), mp_uRotPix(NULL), mp_vRotPix(NULL), BlurBase(mgr) { }
	~BlurDirectional();
	virtual void notifyPrmChanged(int prmID);
	virtual void blur(TimeValue t, CompMap *compositeMap, Bitmap *bm, RenderGlobalContext *gc);
};

#endif // !defined(_BLUR_DIRECTIONAL_H_INCLUDED_)
