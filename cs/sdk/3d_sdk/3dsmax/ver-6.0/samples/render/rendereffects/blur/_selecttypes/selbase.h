/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: selBase.h

	 DESCRIPTION: base selection type -  class declaration

	 CREATED BY: michael malone (mjm)

	 HISTORY: created November 4, 1998

	 Copyright (c) 1998, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */

#if !defined(_SEL_BASE_H_INCLUDED_)
#define _SEL_BASE_H_INCLUDED_

#include "max.h"
class BlurMgr;

// ----------------------------------------
// base selection type - class declaration
// ----------------------------------------
class SelBase
{
protected:
	BlurMgr *const mp_blurMgr;
	WORD *mp_srcMap, *mp_srcAlpha;
	float *mp_radMap, *m_normRadMask;
	float m_min, m_max, m_brighten, m_blend;
	int m_feathRad, m_mapSz, m_imageSz, m_imageW, m_imageH, m_normRadMaskW, m_normRadMaskH;
	bool m_selectValid, m_featherValid, m_compValid;
	const TCHAR *mp_nameCurProcess;
	DWORD m_lastBMModifyID;

	virtual float calcBrighten(const float &normRadius) { return 0.0f; }
	virtual float calcBlend(const float &normRadius) { return 0.0f; }
	virtual bool doSelect() { return false; }
	virtual bool doFeather() { return false; }
	virtual bool doComposite(TimeValue t, CompMap &compMap) { return false; }

public:
	SelBase() :
		mp_blurMgr(NULL), mp_srcMap(NULL), mp_srcAlpha(NULL), mp_radMap(NULL), m_mapSz(0),
		m_imageSz(0), m_imageW(0), m_imageH(0), m_selectValid(false), m_featherValid(false), m_compValid(false),
		mp_nameCurProcess(NULL), m_lastBMModifyID(0xFFFFFFFF), m_normRadMask(NULL), m_normRadMaskW(0), m_normRadMaskH(0) { }

	SelBase(BlurMgr *const mgr) :
		mp_blurMgr(mgr), mp_srcMap(NULL), mp_srcAlpha(NULL), mp_radMap(NULL), m_mapSz(0),
		m_imageSz(0), m_imageW(0), m_imageH(0), m_selectValid(false), m_featherValid(false), m_compValid(false),
		mp_nameCurProcess(NULL), m_lastBMModifyID(0xFFFFFFFF), m_normRadMask(NULL), m_normRadMaskW(0), m_normRadMaskH(0) { }

	virtual ~SelBase() { }
	virtual void notifyPrmChanged(int prmID) { }
	bool checkValid(DWORD bmID) { return (m_selectValid && m_featherValid && m_compValid && (m_lastBMModifyID == bmID)); }
	virtual float getBrighten(TimeValue t) { return m_brighten; }
	virtual float getBlend(TimeValue t) { return m_blend; }
	virtual void select(TimeValue t, CompMap &compMap, Bitmap *bm, RenderGlobalContext *gc) { }
};

#endif // !defined(_SEL_BASE_H_INCLUDED_)
