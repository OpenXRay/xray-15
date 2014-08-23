/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: blurRadial.h

	 DESCRIPTION: radial blur type -  class declaration

	 CREATED BY: michael malone (mjm)

	 HISTORY: created November 4, 1998

	 Copyright (c) 1998, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */

#if !defined(_BLUR_RADIAL_H_INCLUDED_)
#define _BLUR_RADIAL_H_INCLUDED_

class BlurBase;
#include "blurBase.h"

class BlurRadial : public BlurBase
{
protected:
	BOOL m_useNode;
	INode *m_node;
	Point2 m_orig;
	int m_pixRad, m_inRad, m_outRad, m_inRot, m_outRot, m_szFilter, m_szRotPix, m_numRotPix;
	float m_trail;
	float *mp_Filter;
	IPoint2 *mp_RotPix;
	Interval m_validInterval;

	bool calcRotPixOffsets(double theta);
	virtual bool doBlur(CompMap *pCompMap, WORD *mapFrom, WORD *mapTo, WORD *alphaFrom, WORD *alphaTo);
public:
	BlurRadial() : m_szFilter(0), m_szRotPix(0), m_numRotPix(0), mp_Filter(NULL), mp_RotPix(NULL), m_validInterval(FOREVER) { }
	BlurRadial(BlurMgr *const mgr) : m_szFilter(0), m_szRotPix(0), m_numRotPix(0), mp_Filter(NULL), mp_RotPix(NULL), m_validInterval(FOREVER), BlurBase(mgr) { }
	~BlurRadial();
	virtual void notifyPrmChanged(int prmID);
	virtual void blur(TimeValue t, CompMap *compositeMap, Bitmap *bm, RenderGlobalContext *gc);
};

#endif // !defined(_BLUR_RADIAL_H_INCLUDED_)
